// Fill out your copyright notice in the Description page of Project Settings.

#include "SoldierThreatAssessment.h"

#include "SoldierAIConfig.h"

#include "Curves/CurveFloat.h"
#include "GameFramework/Actor.h"

namespace
{
	float InverseSquareFalloff(float Distance, float HalfDistance)
	{
		if (HalfDistance <= KINDA_SMALL_NUMBER)
		{
			return 0.f;
		}
		const float Ratio = Distance / HalfDistance;
		return 1.f / (1.f + Ratio * Ratio);
	}
}


float USoldierThreatAssessment::ScoreThreat_Implementation(const FThreatMemory& Memory, const FSoldierSelfState& Self, const USoldierThreatConfig* Config, float WorldTimeSeconds) const
{
	if (!Config || !Memory.Target.IsValid())
	{
		return 0.f;
	}

	const float Distance = FVector::Dist(Self.EyeLocation, Memory.GetEstimatedLocation(WorldTimeSeconds));

	if (Distance > Config->MaxConsideredRangeCm)
	{
		return 0.f;
	}

	// --- proximity ---
	float Score = Config->DangerDistanceCurve
		? FMath::Clamp(Config->DangerDistanceCurve->GetFloatValue(Distance / FMath::Max(Config->MaxConsideredRangeCm, 1.f)), 0.f, 1.f)
		: InverseSquareFalloff(Distance, Config->DangerHalfDistanceCm);

	// --- confidence ---
	// Not a plain multiply. A contact we only heard is still dangerous - discounting it to near
	// zero would make a soldier ignore the man who just shot at him from cover.
	const float ConfidenceScale = FMath::Lerp(Config->MinConfidenceThreatScale, 1.f, FMath::Clamp(Memory.Confidence, 0.f, 1.f));
	Score *= ConfidenceScale;

	// --- weapon class ---
	if (const float* Multiplier = Config->WeaponThreatMultipliers.Find(Memory.TargetWeaponTag))
	{
		Score *= *Multiplier;
	}

	// --- aiming at me ---
	if (Memory.bTargetAimingAtMe)
	{
		Score *= Config->AimingAtMeMultiplier;
	}

	// --- recently shot me ---
	const float SinceDamage = WorldTimeSeconds - Memory.LastDamagedMeWorldTime;
	if (SinceDamage >= 0.f && SinceDamage < Config->RecentDamageWindowSec)
	{
		const float Freshness = 1.f - (SinceDamage / FMath::Max(Config->RecentDamageWindowSec, KINDA_SMALL_NUMBER));
		Score *= FMath::Lerp(1.f, Config->RecentDamageMultiplier, FMath::Clamp(Freshness, 0.f, 1.f));
	}

	// TODO: a Dead or Downed contact threatens nobody, and should score 0 here. Deliberately not
	// implemented: there is no health component in this project yet, and guessing at its interface
	// would put a wrong assumption in the one place that is hard to notice. Until then the target's
	// memory simply stops being reinforced when it dies, and ForgetTarget() removes it (design 6.5.3).

	return FMath::Max(Score, 0.f);
}


float USoldierThreatAssessment::ScoreEngagementValue_Implementation(const FThreatMemory& Memory, const FSoldierSelfState& Self, const USoldierThreatConfig* Config, float WorldTimeSeconds) const
{
	if (!Config || !Memory.Target.IsValid())
	{
		return 0.f;
	}

	// Gates first. These are hard zeros, not small numbers: design 8.3 multiplies utility axes, so
	// a zero here removes AimedFire from consideration entirely rather than making it merely unlikely.
	if (Memory.Confidence < Config->MinConfidenceToEngage)
	{
		return 0.f;
	}

	if (Memory.VisibilityRatio < Config->MinVisibilityToEngage)
	{
		return 0.f;
	}

	const float Distance = FVector::Dist(Self.EyeLocation, Memory.GetEstimatedLocation(WorldTimeSeconds));

	// Inside effective range, full value. Past it, falls off linearly over RangeFalloffSlackCm.
	float RangeScale = 1.f;
	if (Distance > Self.WeaponEffectiveRangeCm)
	{
		const float Over = Distance - Self.WeaponEffectiveRangeCm;
		RangeScale = FMath::Clamp(1.f - (Over / FMath::Max(Config->RangeFalloffSlackCm, 1.f)), 0.f, 1.f);
	}

	// How much of him is actually shootable. This is the term that says "he is behind a wall,
	// suppress instead of aiming", and it is the reason VisibilityRatio is a fraction and not a bool.
	const float ExposedScale = FMath::Clamp(Memory.VisibilityRatio, 0.f, 1.f);

	// Our own ability to deliver the shot. Suppressed and low on ammunition both reduce the payoff
	// of choosing an aimed engagement over taking cover first (design 6.2).
	const float AmmoGate = (Self.MagazineRatio > 0.f) ? 1.f : 0.f;
	const float ReadinessScale = FMath::Clamp(1.f - Self.SuppressionLevel, 0.f, 1.f) * AmmoGate;

	return FMath::Clamp(Memory.Confidence * ExposedScale * RangeScale * ReadinessScale, 0.f, 1.f);
}


float USoldierThreatAssessment::ScoreTargetPriority(const FThreatMemory& Memory, const USoldierThreatConfig* Config)
{
	if (!Config)
	{
		return Memory.ThreatLevel;
	}

	const float Bias = FMath::Clamp(Config->ThreatVsValueBias, 0.f, 1.f);
	return FMath::Lerp(Memory.ThreatLevel, Memory.EngagementValue, Bias);
}


AActor* USoldierThreatAssessment::SelectPrimaryTarget_Implementation(const TArray<FThreatMemory>& Memories, const FSoldierSelfState& Self, const USoldierThreatConfig* Config, AActor* CurrentTarget, float CurrentHeldForSec) const
{
	if (!Config)
	{
		return CurrentTarget;
	}

	AActor* Best = nullptr;
	float BestScore = 0.f;
	float IncumbentScore = 0.f;
	bool bIncumbentStillValid = false;

	for (const FThreatMemory& Memory : Memories)
	{
		if (!Memory.Target.IsValid())
		{
			continue;
		}

		// Only contacts we are at least suspicious of can be a primary target. Below that we do
		// not know anyone is there, and aiming at an Unaware contact is the aimbot behaviour
		// design 7.1 exists to remove.
		if (Memory.AwarenessLevel == ESoldierAwareness::Unaware)
		{
			continue;
		}

		const float Score = ScoreTargetPriority(Memory, Config);

		if (Memory.Target.Get() == CurrentTarget)
		{
			bIncumbentStillValid = true;
			IncumbentScore = Score;
		}

		if (Score > BestScore)
		{
			BestScore = Score;
			Best = Memory.Target.Get();
		}
	}

	// Incumbent gone (dead, forgotten, out of range): take the best available with no margin.
	if (!bIncumbentStillValid)
	{
		return Best;
	}

	// Held too briefly to reconsider.
	if (CurrentHeldForSec < Config->MinTargetHoldSec)
	{
		return CurrentTarget;
	}

	// Switch only on a clear win. Compare against an absolute margin as well as a relative one so
	// that two near-zero scores cannot trade places forever.
	const float Required = IncumbentScore * (1.f + Config->TargetSwitchMargin) + KINDA_SMALL_NUMBER;
	if (Best && Best != CurrentTarget && BestScore > Required)
	{
		return Best;
	}

	return CurrentTarget;
}
