// Fill out your copyright notice in the Description page of Project Settings.

#include "SoldierBrainComponent.h"

#include "SoldierAIConfig.h"
#include "SoldierPerceptionComponent.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"

namespace
{
	/**
	 * Stand-in for "no threat anywhere near". Used as the raw value of distance inputs when there
	 * is no target, so that a distance consideration clamps to the far end of its InputRange rather
	 * than reading 0 - which would mean "the enemy is on top of me" and invert the intent.
	 */
	constexpr float BIG_DISTANCE_CM = 1.0e6f;
}


USoldierBrainComponent::USoldierBrainComponent()
{
	// Ticks, but only to count down to the next decision. The decision itself runs at 2~10 Hz
	// (design 4.2); running the scorer every frame would spend the whole 0.3 ms budget on
	// re-deriving numbers that have not meaningfully changed.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	SetIsReplicatedByDefault(false);
}

void USoldierBrainComponent::BeginPlay()
{
	Super::BeginPlay();

	// P5 / design 4.3.2: judgement is server only. On a client the component sits idle and the
	// replicated intent tag (design 4.3.3) is what drives the local animation.
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		SetComponentTickEnabled(false);
		return;
	}

	Perception = GetOwner()->FindComponentByClass<USoldierPerceptionComponent>();

	// Stagger the first decision so soldiers spawned together do not all evaluate on one frame.
	TimeUntilNextDecision = FMath::FRand() * GetDecisionInterval();
}

float USoldierBrainComponent::GetDecisionInterval() const
{
	if (!Profile)
	{
		return 0.2f;
	}

	const float Hz = (LODTier <= 0) ? Profile->DecisionHzTier0
		: (LODTier == 1) ? Profile->DecisionHzTier1
		: Profile->DecisionHzTier2;

	return (Hz > KINDA_SMALL_NUMBER) ? (1.f / Hz) : 0.2f;
}

void USoldierBrainComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	TimeUntilNextDecision -= DeltaTime;

	if (TimeUntilNextDecision > 0.f && !bImmediateReevaluationRequested)
	{
		return;
	}

	bImmediateReevaluationRequested = false;
	TimeUntilNextDecision = GetDecisionInterval();

	GatherInputs();
	EvaluateAndSelect();
}

void USoldierBrainComponent::RequestImmediateReevaluation()
{
	bImmediateReevaluationRequested = true;
}

void USoldierBrainComponent::SetScoringInput(FName InputName, float Value)
{
	ScoringInputs.Add(InputName, Value);
}

float USoldierBrainComponent::GetScoringInput(FName InputName) const
{
	if (const float* Found = ScoringInputs.Find(InputName))
	{
		return *Found;
	}
	return 0.f;
}


// ---------------------------------------------------------------------------------------------
// Inputs
// ---------------------------------------------------------------------------------------------

void USoldierBrainComponent::GatherInputs()
{
	using namespace SoldierScoringInputs;

	const UWorld* World = GetWorld();
	const float WorldTime = World ? World->GetTimeSeconds() : 0.f;

	if (!Perception)
	{
		return;
	}

	const FSoldierSelfState& Self = Perception->SelfState;

	// --- self ---
	ScoringInputs.Add(SelfExposure,			Self.Exposure);
	ScoringInputs.Add(SelfHealthRatio,		Self.HealthRatio);
	ScoringInputs.Add(SelfIsInjured,		(Self.HealthState == ESoldierHealthState::Injured) ? 1.f : 0.f);
	ScoringInputs.Add(MagazineRatio,		Self.MagazineRatio);
	ScoringInputs.Add(ReserveAmmoRatio,		Self.ReserveAmmoRatio);
	ScoringInputs.Add(SuppressionLevel,		Self.SuppressionLevel);
	ScoringInputs.Add(CurrentCoverQuality,	Self.CurrentCoverQuality);
	ScoringInputs.Add(AimConvergence,		Self.AimConvergence);
	ScoringInputs.Add(TimeSinceDamagedSec,	WorldTime - Self.LastDamagedWorldTime);

	// --- threat picture ---
	const TArray<FThreatMemory>& Memories = Perception->GetMemoriesRef();
	AActor* Primary = Perception->GetPrimaryTarget();

	float NearestDistance = BIG_DISTANCE_CM;
	float TotalThreat = 0.f;
	int32 VisibleCount = 0;

	for (const FThreatMemory& Memory : Memories)
	{
		if (!Memory.Target.IsValid())
		{
			continue;
		}

		TotalThreat += Memory.ThreatLevel;

		if (Memory.AwarenessLevel == ESoldierAwareness::Confirmed && Memory.bCurrentlyVisible)
		{
			++VisibleCount;
		}

		const float Distance = FVector::Dist(Self.EyeLocation, Memory.GetEstimatedLocation(WorldTime));
		NearestDistance = FMath::Min(NearestDistance, Distance);

		if (Memory.Target.Get() == Primary)
		{
			ScoringInputs.Add(PrimaryTargetConfidence,	Memory.Confidence);
			ScoringInputs.Add(PrimaryTargetAwareness,	Memory.Awareness);
			ScoringInputs.Add(PrimaryTargetVisibility,	Memory.VisibilityRatio);
			ScoringInputs.Add(PrimaryThreatLevel,		Memory.ThreatLevel);
			ScoringInputs.Add(PrimaryEngagementValue,	Memory.EngagementValue);
			ScoringInputs.Add(PrimaryTargetDistanceCm,	Distance);
			ScoringInputs.Add(PrimaryTimeSinceSeenSec,	Memory.TimeSinceLastSeen);

			// Exposure of the target, from the target's own perception component (design 7.5).
			float TargetExposure = 1.f;
			if (const USoldierPerceptionComponent* TargetPerception = Memory.Target->FindComponentByClass<USoldierPerceptionComponent>())
			{
				TargetExposure = TargetPerception->GetExposure();
			}
			ScoringInputs.Add(PrimaryTargetExposure, TargetExposure);
		}
	}

	if (!Primary)
	{
		// Explicit zeroes. Leaving stale values from the previous target would let a soldier keep
		// scoring AimedFire highly against someone who is no longer there.
		ScoringInputs.Add(PrimaryTargetConfidence,	0.f);
		ScoringInputs.Add(PrimaryTargetAwareness,	0.f);
		ScoringInputs.Add(PrimaryTargetVisibility,	0.f);
		ScoringInputs.Add(PrimaryThreatLevel,		0.f);
		ScoringInputs.Add(PrimaryEngagementValue,	0.f);
		ScoringInputs.Add(PrimaryTargetExposure,	0.f);
		ScoringInputs.Add(PrimaryTargetDistanceCm,	BIG_DISTANCE_CM);
		ScoringInputs.Add(PrimaryTimeSinceSeenSec,	0.f);
	}

	ScoringInputs.Add(VisibleThreatCount,		static_cast<float>(VisibleCount));
	ScoringInputs.Add(NearestThreatDistanceCm,	NearestDistance);
	ScoringInputs.Add(TotalIncomingThreat,		TotalThreat);

	// LineOfFireClear is left to whoever owns the weapon: it needs the muzzle transform and the
	// friendly positions, neither of which belongs to perception. design 9.5 lists it as an
	// AimedFire axis; until it is wired the input stays 0, which zeroes AimedFire. That is the
	// wrong default for a solo test, so the weapon layer must set it - see PLAN.md 8.

	// SoldierScoringInputs::Squad* and BestCoverSlotScore / CoverSlotAvailable are NOT set here.
	// They belong to the squad and cover layers, which push them in via SetScoringInput.
}


// ---------------------------------------------------------------------------------------------
// Scoring - design 8.5
// ---------------------------------------------------------------------------------------------

float USoldierBrainComponent::ScoreIntent(const FIntentDefinition& Definition, FIntentScoreBreakdown& OutBreakdown) const
{
	OutBreakdown.IntentTag = Definition.IntentTag;
	OutBreakdown.AxisNames.Reset();
	OutBreakdown.AxisInputs.Reset();
	OutBreakdown.AxisFactors.Reset();

	const int32 AxisCount = Definition.Considerations.Num();
	if (AxisCount == 0)
	{
		OutBreakdown.RawScore = Definition.BaseWeight;
		OutBreakdown.FinalScore = Definition.BaseWeight;
		return Definition.BaseWeight;
	}

	float Score = Definition.BaseWeight;

	for (const FConsideration& Consideration : Definition.Considerations)
	{
		const float RawInput = GetScoringInput(Consideration.InputName);
		const float Factor = Consideration.Evaluate(RawInput);

		Score *= Factor;

		if (bRecordAllScores)
		{
			OutBreakdown.AxisNames.Add(Consideration.InputName);
			OutBreakdown.AxisInputs.Add(RawInput);
			OutBreakdown.AxisFactors.Add(Factor);
		}

		// A zero axis kills the intent outright (design 8.3: multiplication, so one zero is a veto).
		// Bail out early - but only after recording, or the breakdown would not show which axis vetoed.
		if (Score <= 0.f)
		{
			break;
		}
	}

	// IAUS make-up value: without it an intent with six axes can never beat one with two, because
	// each extra multiplicand below 1 drags the product down. design 8.5 spells out this exact form.
	const float Modification = 1.f - (1.f / static_cast<float>(AxisCount));
	Score = Score + (1.f - Score) * Modification * Score;

	OutBreakdown.RawScore = Score;
	OutBreakdown.FinalScore = Score;
	return Score;
}

void USoldierBrainComponent::EvaluateAndSelect()
{
	if (!Profile || Profile->Intents.Num() == 0)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const float WorldTime = World ? World->GetTimeSeconds() : 0.f;
	const float RunningFor = WorldTime - CurrentIntent.SelectedWorldTime;

	LastScores.Reset(Profile->Intents.Num());

	const FIntentDefinition* WinnerDefinition = nullptr;
	FIntentScoreBreakdown WinnerBreakdown;
	float WinnerScore = 0.f;

	// Is the running intent still allowed to hold the floor?
	const FIntentDefinition* CurrentDefinition = Profile->Intents.FindByPredicate(
		[this](const FIntentDefinition& Definition)
		{
			return Definition.IntentTag == CurrentIntent.IntentTag;
		});

	const bool bCurrentLocked = CurrentDefinition && (RunningFor < CurrentDefinition->MinDurationSec);

	for (const FIntentDefinition& Definition : Profile->Intents)
	{
		FIntentScoreBreakdown Breakdown;
		float Score = ScoreIntent(Definition, Breakdown);

		const bool bIsCurrent = Definition.IntentTag == CurrentIntent.IntentTag;

		// Hysteresis (design 8.3): the incumbent gets a bonus, otherwise two intents with nearly
		// equal scores swap every evaluation and the soldier stutters.
		if (bIsCurrent && Score > 0.f)
		{
			Score *= (1.f + Profile->CurrentIntentBonus);
			Breakdown.FinalScore = Score;
		}

		// Minimum duration, with the survival override. design 8.3: TakeCover / Retreat may cut in.
		if (bCurrentLocked && !bIsCurrent && !Definition.Traits.HasTag(TAG_Intent_Trait_Survival))
		{
			Breakdown.FinalScore = 0.f;
			if (bRecordAllScores)
			{
				LastScores.Add(Breakdown);
			}
			continue;
		}

		if (Score < Definition.MinViableScore)
		{
			Breakdown.FinalScore = Score;
			if (bRecordAllScores)
			{
				LastScores.Add(Breakdown);
			}
			continue;
		}

		if (bRecordAllScores)
		{
			LastScores.Add(Breakdown);
		}

		if (Score > WinnerScore)
		{
			WinnerScore = Score;
			WinnerDefinition = &Definition;
			WinnerBreakdown = Breakdown;
		}
	}

	LastScores.Sort([](const FIntentScoreBreakdown& A, const FIntentScoreBreakdown& B)
	{
		return A.FinalScore > B.FinalScore;
	});

	if (!WinnerDefinition)
	{
		// Nothing cleared its viability floor. Keep doing what we were doing rather than falling
		// into an undefined state; if that is also nothing, the profile is missing an Idle intent
		// with no considerations, which should always score BaseWeight.
		return;
	}

	if (WinnerDefinition->IntentTag == CurrentIntent.IntentTag)
	{
		// Same intent - refresh the target and the rationale, but do not restart the clock.
		CurrentIntent.Score = WinnerScore;
		CurrentIntent.Breakdown = WinnerBreakdown;
	}
	else
	{
		const FGameplayTag Previous = CurrentIntent.IntentTag;

		CurrentIntent.IntentTag = WinnerDefinition->IntentTag;
		CurrentIntent.Score = WinnerScore;
		CurrentIntent.SelectedWorldTime = WorldTime;
		CurrentIntent.Breakdown = WinnerBreakdown;

		OnIntentChanged.Broadcast(CurrentIntent.IntentTag, Previous);
	}

	// The target rides along with the selection so L3 does not have to re-query and possibly get a
	// different answer than the one the score was based on.
	if (Perception)
	{
		CurrentIntent.TargetActor = Perception->GetPrimaryTarget();

		FVector EstimatedLocation = FVector::ZeroVector;
		float Uncertainty = 0.f;
		if (Perception->GetPrimaryTargetEstimatedLocation(EstimatedLocation, Uncertainty))
		{
			CurrentIntent.TargetLocation = EstimatedLocation;
		}
	}
}
