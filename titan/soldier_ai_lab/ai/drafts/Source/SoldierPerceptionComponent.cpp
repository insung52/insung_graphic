// Fill out your copyright notice in the Description page of Project Settings.

#include "SoldierPerceptionComponent.h"

#include "SoldierAIConfig.h"
#include "SoldierThreatAssessment.h"

#include "Components/SkeletalMeshComponent.h"
#include "Curves/CurveFloat.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Hearing.h"

namespace
{
	/** 1/(1 + (x/half)^2). Finite at 0, close to inverse square far out. design 7.2's stated shape. */
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


USoldierPerceptionComponent::USoldierPerceptionComponent()
{
	// The subsystem drives updates so they can be spread across frames under one budget.
	// Component ticking would give every soldier the same frame, which is the shape design 12.2 rules out.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);
}

void USoldierPerceptionComponent::BeginPlay()
{
	Super::BeginPlay();

	// P5 / design 4.3.2: perception is server only. The gate goes in now, while it is cheap.
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (USoldierPerceptionSubsystem* Subsystem = World->GetSubsystem<USoldierPerceptionSubsystem>())
		{
			Subsystem->RegisterSoldier(this);
		}
	}

	BindToAIPerception();
}

void USoldierPerceptionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (USoldierPerceptionSubsystem* Subsystem = World->GetSubsystem<USoldierPerceptionSubsystem>())
		{
			Subsystem->UnregisterSoldier(this);
		}
	}

	if (BoundAIPerception)
	{
		BoundAIPerception->OnTargetPerceptionUpdated.RemoveDynamic(this, &USoldierPerceptionComponent::HandleAIPerceptionUpdated);
		BoundAIPerception = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}


// ---------------------------------------------------------------------------------------------
// Config access
// ---------------------------------------------------------------------------------------------

const USoldierPerceptionConfig* USoldierPerceptionComponent::GetPerceptionConfig() const
{
	if (Profile && Profile->Perception)
	{
		return Profile->Perception;
	}
	return PerceptionConfigOverride;
}

const USoldierThreatConfig* USoldierPerceptionComponent::GetThreatConfig() const
{
	if (Profile && Profile->Threat)
	{
		return Profile->Threat;
	}
	return ThreatConfigOverride;
}

float USoldierPerceptionComponent::GetUpdateInterval() const
{
	const USoldierPerceptionConfig* Config = GetPerceptionConfig();
	if (!Config)
	{
		return 0.1f;
	}

	const float Hz = (LODTier <= 0) ? Config->UpdateHzTier0
		: (LODTier == 1) ? Config->UpdateHzTier1
		: Config->UpdateHzTier2;

	return (Hz > KINDA_SMALL_NUMBER) ? (1.f / Hz) : 0.1f;
}


// ---------------------------------------------------------------------------------------------
// Geometry helpers
// ---------------------------------------------------------------------------------------------

FVector USoldierPerceptionComponent::GetEyeLocation() const
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return FVector::ZeroVector;
	}

	if (const USoldierPerceptionConfig* Config = GetPerceptionConfig())
	{
		if (const USkeletalMeshComponent* Mesh = Owner->FindComponentByClass<USkeletalMeshComponent>())
		{
			if (Config->EyeBoneName != NAME_None && Mesh->DoesSocketExist(Config->EyeBoneName))
			{
				return Mesh->GetSocketLocation(Config->EyeBoneName);
			}
		}
	}

	// GetActorEyesViewPoint is the engine's own fallback and already accounts for pawn eye height.
	FVector Location = FVector::ZeroVector;
	FRotator Rotation = FRotator::ZeroRotator;
	Owner->GetActorEyesViewPoint(Location, Rotation);
	return Location;
}

FVector USoldierPerceptionComponent::GetGazeDirection() const
{
	if (bGazeOverrideActive)
	{
		return GazeOverrideDirection;
	}

	// design 7.2 wants head look-at, separate from the aim vector. Until a look-at channel exists
	// (pose spec 3.2 FGazeIntent), the controller rotation is the closest honest approximation:
	// it is what GASP drives aimingRotation from, so at least gaze and aim disagree only when L3
	// deliberately overrides. [B]
	if (const APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (const AController* Controller = Pawn->GetController())
		{
			return Controller->GetControlRotation().Vector();
		}
	}

	return GetOwner() ? GetOwner()->GetActorForwardVector() : FVector::ForwardVector;
}

void USoldierPerceptionComponent::SetGazeOverride(const FVector& WorldDirection, bool bEnabled)
{
	bGazeOverrideActive = bEnabled;
	GazeOverrideDirection = WorldDirection.GetSafeNormal(KINDA_SMALL_NUMBER, FVector::ForwardVector);
}

bool USoldierPerceptionComponent::IsHostileTo(const AActor* Other) const
{
	if (!Other || Other == GetOwner())
	{
		return false;
	}

	const USoldierPerceptionComponent* OtherPerception = Other->FindComponentByClass<USoldierPerceptionComponent>();
	if (!OtherPerception)
	{
		return false;
	}

	return HostileFactions.HasTag(OtherPerception->Faction);
}


// ---------------------------------------------------------------------------------------------
// Update
// ---------------------------------------------------------------------------------------------

void USoldierPerceptionComponent::UpdatePerception(float DeltaTime)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())		// P5
	{
		return;
	}

	const USoldierPerceptionConfig* Config = GetPerceptionConfig();
	if (!Config)
	{
		return;
	}

	SelfState.EyeLocation = GetEyeLocation();
	UpdateExposure();

	// Candidate pre-filter: registry, range, hostility. design 12.2 puts the trace count behind
	// this filter on purpose - the traces are the expensive part, not the bookkeeping.
	TArray<AActor*> Candidates;
	if (UWorld* World = GetWorld())
	{
		if (USoldierPerceptionSubsystem* Subsystem = World->GetSubsystem<USoldierPerceptionSubsystem>())
		{
			Subsystem->GatherHostileCandidates(this, Config->MaxSightRangeCm, Candidates);
		}
	}

	int32 TraceBudget = Config->MaxSightTracesPerUpdate;

	// Mark everything as unseen first; the sight pass re-marks what it finds.
	for (FThreatMemory& Memory : Memories)
	{
		Memory.bCurrentlyVisible = false;
	}

	const float WorldTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	for (AActor* Candidate : Candidates)
	{
		float VisibilityRatio = 0.f;
		const float Rate = EvaluateSight(Candidate, VisibilityRatio, TraceBudget);
		if (Rate <= 0.f)
		{
			continue;
		}

		FThreatMemory& Memory = FindOrAddMemory(Candidate);
		Memory.Awareness = FMath::Clamp(Memory.Awareness + Rate * DeltaTime, 0.f, 1.f);
		Memory.bCurrentlyVisible = true;
		Memory.VisibilityRatio = VisibilityRatio;
		Memory.LastSource = ESoldierPerceptionSource::Sight;
		Memory.LastKnownLocation = Candidate->GetActorLocation();
		Memory.LastKnownVelocity = Candidate->GetVelocity();
		Memory.LastSeenWorldTime = WorldTime;
		Memory.TimeSinceLastSeen = 0.f;
		Memory.LocationUncertaintyCm = 0.f;
		Memory.bSharedBySquad = false;			// seeing it myself outranks the radio (design 9.3)
		Memory.Confidence = FMath::Max(Memory.Confidence, Memory.Awareness);

		if (const USoldierPerceptionComponent* TargetPerception = Candidate->FindComponentByClass<USoldierPerceptionComponent>())
		{
			Memory.TargetWeaponTag = TargetPerception->SelfState.WeaponTag;
		}
	}

	DecayMemories(DeltaTime);
	ApplyAwarenessBands();
	UpdateThreatLevels();
	UpdatePrimaryTarget();
}

float USoldierPerceptionComponent::EvaluateSight(AActor* Candidate, float& OutVisibilityRatio, int32& InOutTraceBudget) const
{
	OutVisibilityRatio = 0.f;

	const USoldierPerceptionConfig* Config = GetPerceptionConfig();
	if (!Config || !Candidate)
	{
		return 0.f;
	}

	const FVector EyeLocation = SelfState.EyeLocation;
	const FVector ToTarget = Candidate->GetActorLocation() - EyeLocation;
	const float Distance = ToTarget.Size();

	if (Distance > Config->MaxSightRangeCm || Distance <= KINDA_SMALL_NUMBER)
	{
		return 0.f;
	}

	const FVector GazeDirection = GetGazeDirection();
	const float CosAngle = FVector::DotProduct(GazeDirection, ToTarget / Distance);
	const float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(CosAngle, -1.f, 1.f)));

	if (AngleDeg > Config->MaxHalfAngleDeg)
	{
		return 0.f;
	}

	// Angular falloff: flat inside the foveal cone, then two linear ramps. The design gives the
	// three anchor points and not the shape between them, so a curve override is provided. [B]
	float AngleScale = 1.f;
	if (Config->AngleFalloffCurve)
	{
		AngleScale = Config->AngleFalloffCurve->GetFloatValue(AngleDeg / FMath::Max(Config->MaxHalfAngleDeg, 1.f));
	}
	else if (AngleDeg > Config->PeripheralHalfAngleDeg)
	{
		const float Span = FMath::Max(Config->MaxHalfAngleDeg - Config->PeripheralHalfAngleDeg, 1.f);
		const float Alpha = (AngleDeg - Config->PeripheralHalfAngleDeg) / Span;
		AngleScale = FMath::Lerp(Config->PeripheralRateScale, Config->MaxAngleRateScale, FMath::Clamp(Alpha, 0.f, 1.f));
	}
	else if (AngleDeg > Config->FovealHalfAngleDeg)
	{
		const float Span = FMath::Max(Config->PeripheralHalfAngleDeg - Config->FovealHalfAngleDeg, 1.f);
		const float Alpha = (AngleDeg - Config->FovealHalfAngleDeg) / Span;
		AngleScale = FMath::Lerp(1.f, Config->PeripheralRateScale, FMath::Clamp(Alpha, 0.f, 1.f));
	}

	float DistanceScale = 1.f;
	if (Config->DistanceFalloffCurve)
	{
		DistanceScale = Config->DistanceFalloffCurve->GetFloatValue(Distance / FMath::Max(Config->MaxSightRangeCm, 1.f));
	}
	else
	{
		DistanceScale = InverseSquareFalloff(Distance, Config->HalfRateDistanceCm);
	}

	// Occlusion last, because it is the only part that costs traces.
	OutVisibilityRatio = ComputeVisibilityRatio(Candidate, InOutTraceBudget);
	if (OutVisibilityRatio <= 0.f)
	{
		return 0.f;
	}

	// A moving target is easier to spot than a still one.
	const float TargetSpeed = Candidate->GetVelocity().Size();
	const float SpeedAlpha = FMath::Clamp(TargetSpeed / FMath::Max(Config->TargetSpeedForFullBonusCm, 1.f), 0.f, 1.f);
	const float SpeedScale = FMath::Lerp(1.f, Config->MaxTargetSpeedRateMultiplier, SpeedAlpha);

	// design 7.5: the target's own exposure multiplies our detection rate. Same number, two systems.
	float TargetExposure = 1.f;
	if (const USoldierPerceptionComponent* TargetPerception = Candidate->FindComponentByClass<USoldierPerceptionComponent>())
	{
		TargetExposure = TargetPerception->GetExposure();
	}

	return Config->BaseAwarenessRatePerSec
		* AngleScale
		* DistanceScale
		* OutVisibilityRatio
		* SpeedScale
		* TargetExposure
		* Config->EnvironmentRateScale;
}

float USoldierPerceptionComponent::ComputeVisibilityRatio(const AActor* Target, int32& InOutTraceBudget) const
{
	const USoldierPerceptionConfig* Config = GetPerceptionConfig();
	UWorld* World = GetWorld();
	if (!Config || !World || !Target || InOutTraceBudget <= 0)
	{
		return 0.f;
	}

	TArray<FVector, TInlineAllocator<8>> SamplePoints;

	if (const USkeletalMeshComponent* Mesh = Target->FindComponentByClass<USkeletalMeshComponent>())
	{
		for (const FName& BoneName : Config->VisibilitySampleBones)
		{
			if (BoneName != NAME_None && Mesh->DoesSocketExist(BoneName))
			{
				SamplePoints.Add(Mesh->GetSocketLocation(BoneName));
			}
		}
	}

	if (SamplePoints.Num() == 0)
	{
		// No skeletal mesh or no name resolved. One trace to the centre is better than declaring
		// the target invisible - a silent "nobody can see anything" is the worst failure here.
		SamplePoints.Add(Target->GetActorLocation());
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(SoldierSight), /*bTraceComplex=*/false);
	Params.AddIgnoredActor(GetOwner());
	Params.AddIgnoredActor(Target);

	int32 Hits = 0;
	int32 Tested = 0;

	for (const FVector& Point : SamplePoints)
	{
		if (InOutTraceBudget <= 0)
		{
			break;
		}
		--InOutTraceBudget;
		++Tested;

		// LineTraceTestByChannel returns true when something BLOCKED the trace.
		const bool bBlocked = World->LineTraceTestByChannel(SelfState.EyeLocation, Point, Config->SightTraceChannel, Params);
		if (!bBlocked)
		{
			++Hits;
		}
	}

	return (Tested > 0) ? (static_cast<float>(Hits) / static_cast<float>(Tested)) : 0.f;
}

void USoldierPerceptionComponent::UpdateExposure()
{
	const USoldierPerceptionConfig* Config = GetPerceptionConfig();
	const AActor* Owner = GetOwner();
	if (!Config || !Owner)
	{
		return;
	}

	float Exposure = 1.f;

	switch (SelfState.Stance)
	{
	case ESoldierStance::Crouch:	Exposure *= Config->ExposureCrouched;	break;
	case ESoldierStance::Prone:		Exposure *= Config->ExposureProne;		break;
	default:						Exposure *= Config->ExposureStanding;	break;
	}

	// Cover quality is authored 0..1 by the cover layer; 1 means fully covered.
	Exposure *= FMath::Clamp(1.f - SelfState.CurrentCoverQuality, 0.f, 1.f);

	const float Speed = Owner->GetVelocity().Size();
	if (Speed <= KINDA_SMALL_NUMBER)
	{
		Exposure *= Config->ExposureStationary;
	}
	else
	{
		const float Alpha = FMath::Clamp(Speed / FMath::Max(Config->RunningSpeedCm, 1.f), 0.f, 1.f);
		Exposure *= FMath::Lerp(Config->ExposureWalking, Config->ExposureRunning, Alpha);
	}

	const float WorldTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	if (WorldTime - LastFiredWorldTime < Config->FiringExposureDecaySec)
	{
		Exposure *= Config->ExposureFiring;
	}

	SelfState.Exposure = FMath::Max(Exposure, 0.f);
}

void USoldierPerceptionComponent::DecayMemories(float DeltaTime)
{
	const USoldierPerceptionConfig* Config = GetPerceptionConfig();
	if (!Config)
	{
		return;
	}

	for (int32 Index = Memories.Num() - 1; Index >= 0; --Index)
	{
		FThreatMemory& Memory = Memories[Index];

		if (!Memory.Target.IsValid())
		{
			Memories.RemoveAtSwap(Index);
			continue;
		}

		if (Memory.bCurrentlyVisible)
		{
			continue;
		}

		Memory.TimeSinceLastSeen += DeltaTime;

		// design 7.1: awareness fades, it does not snap to zero when line of sight breaks.
		Memory.Awareness = FMath::Max(Memory.Awareness - Config->AwarenessDecayPerSec * DeltaTime, 0.f);
		Memory.Confidence = FMath::Max(Memory.Confidence - Config->ConfidenceDecayPerSec * DeltaTime, 0.f);

		// Uncertainty grows with time: the target could have moved. This is what makes PeekCheck
		// search an area rather than walk to a stale point (design 7.4).
		Memory.LocationUncertaintyCm += Memory.LastKnownVelocity.Size() * DeltaTime * 0.5f;

		if (Memory.TimeSinceLastSeen > Config->MemoryLifetimeSec && Memory.Awareness <= 0.f)
		{
			Memories.RemoveAtSwap(Index);
		}
	}

	// Capacity trim: keep the most dangerous. A soldier who is tracking twelve people is already
	// past the point where a thirteenth changes any decision.
	if (Memories.Num() > Config->MaxTrackedThreats)
	{
		Memories.Sort([](const FThreatMemory& A, const FThreatMemory& B)
		{
			return A.ThreatLevel > B.ThreatLevel;
		});
		Memories.SetNum(Config->MaxTrackedThreats);
	}
}

void USoldierPerceptionComponent::ApplyAwarenessBands()
{
	const USoldierPerceptionConfig* Config = GetPerceptionConfig();
	if (!Config)
	{
		return;
	}

	const float H = Config->AwarenessBandHysteresis;

	for (FThreatMemory& Memory : Memories)
	{
		const ESoldierAwareness Previous = Memory.AwarenessLevel;

		// Raising a band needs to clear the threshold; dropping needs to fall below it by H.
		ESoldierAwareness NewLevel = Previous;
		switch (Previous)
		{
		case ESoldierAwareness::Unaware:
			if (Memory.Awareness >= Config->ConfirmedThreshold)		{ NewLevel = ESoldierAwareness::Confirmed; }
			else if (Memory.Awareness >= Config->SuspiciousThreshold) { NewLevel = ESoldierAwareness::Suspicious; }
			break;

		case ESoldierAwareness::Suspicious:
			if (Memory.Awareness >= Config->ConfirmedThreshold)			{ NewLevel = ESoldierAwareness::Confirmed; }
			else if (Memory.Awareness < Config->SuspiciousThreshold - H)	{ NewLevel = ESoldierAwareness::Unaware; }
			break;

		case ESoldierAwareness::Confirmed:
			if (Memory.Awareness < Config->SuspiciousThreshold - H)		{ NewLevel = ESoldierAwareness::Unaware; }
			else if (Memory.Awareness < Config->ConfirmedThreshold - H)	{ NewLevel = ESoldierAwareness::Suspicious; }
			break;
		}

		if (NewLevel != Previous)
		{
			Memory.AwarenessLevel = NewLevel;
			OnAwarenessChanged.Broadcast(Memory.Target.Get(), NewLevel);
		}
	}
}

void USoldierPerceptionComponent::UpdateThreatLevels()
{
	if (!ThreatAssessment)
	{
		return;
	}

	const USoldierThreatConfig* Config = GetThreatConfig();
	if (!Config)
	{
		return;
	}

	const float WorldTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	for (FThreatMemory& Memory : Memories)
	{
		Memory.ThreatLevel = ThreatAssessment->ScoreThreat(Memory, SelfState, Config, WorldTime);
		Memory.EngagementValue = ThreatAssessment->ScoreEngagementValue(Memory, SelfState, Config, WorldTime);
	}
}

void USoldierPerceptionComponent::UpdatePrimaryTarget()
{
	const USoldierThreatConfig* Config = GetThreatConfig();
	if (!ThreatAssessment || !Config)
	{
		return;
	}

	const float WorldTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	AActor* Current = PrimaryTarget.Get();
	const float HeldFor = WorldTime - PrimaryTargetSelectedWorldTime;

	AActor* Selected = ThreatAssessment->SelectPrimaryTarget(Memories, SelfState, Config, Current, HeldFor);

	if (Selected != Current)
	{
		PrimaryTarget = Selected;
		PrimaryTargetSelectedWorldTime = WorldTime;
		OnPrimaryTargetChanged.Broadcast(Selected);
	}
}


// ---------------------------------------------------------------------------------------------
// Memory access
// ---------------------------------------------------------------------------------------------

FThreatMemory& USoldierPerceptionComponent::FindOrAddMemory(AActor* Target)
{
	for (FThreatMemory& Memory : Memories)
	{
		if (Memory.Target.Get() == Target)
		{
			return Memory;
		}
	}

	FThreatMemory NewMemory;
	NewMemory.Target = Target;
	NewMemory.LastKnownLocation = Target ? Target->GetActorLocation() : FVector::ZeroVector;
	const int32 Index = Memories.Add(NewMemory);
	return Memories[Index];
}

bool USoldierPerceptionComponent::GetThreatMemory(AActor* Target, FThreatMemory& OutMemory) const
{
	for (const FThreatMemory& Memory : Memories)
	{
		if (Memory.Target.Get() == Target)
		{
			OutMemory = Memory;
			return true;
		}
	}
	return false;
}

bool USoldierPerceptionComponent::GetPrimaryTargetEstimatedLocation(FVector& OutLocation, float& OutUncertaintyCm) const
{
	const AActor* Target = PrimaryTarget.Get();
	if (!Target)
	{
		return false;
	}

	const float WorldTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	for (const FThreatMemory& Memory : Memories)
	{
		if (Memory.Target.Get() == Target)
		{
			OutLocation = Memory.GetEstimatedLocation(WorldTime);
			OutUncertaintyCm = Memory.LocationUncertaintyCm;
			return true;
		}
	}

	return false;
}

int32 USoldierPerceptionComponent::CountThreatsAtLeast(ESoldierAwareness MinLevel) const
{
	int32 Count = 0;
	for (const FThreatMemory& Memory : Memories)
	{
		if (Memory.AwarenessLevel >= MinLevel && Memory.Target.IsValid())
		{
			++Count;
		}
	}
	return Count;
}

void USoldierPerceptionComponent::ForgetTarget(AActor* Target)
{
	for (int32 Index = Memories.Num() - 1; Index >= 0; --Index)
	{
		if (Memories[Index].Target.Get() == Target)
		{
			Memories.RemoveAtSwap(Index);
		}
	}

	if (PrimaryTarget.Get() == Target)
	{
		PrimaryTarget = nullptr;
		OnPrimaryTargetChanged.Broadcast(nullptr);
	}
}


// ---------------------------------------------------------------------------------------------
// External stimuli
// ---------------------------------------------------------------------------------------------

void USoldierPerceptionComponent::ReportDamageFrom(AActor* Instigator, float Amount, const FVector& SourceLocation)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !Instigator)
	{
		return;
	}

	const float WorldTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	SelfState.LastDamagedWorldTime = WorldTime;

	FThreatMemory& Memory = FindOrAddMemory(Instigator);

	// Being hit is the strongest possible evidence that someone is there, even if we cannot see
	// them. It confirms presence, not position - hence the uncertainty (design 7.3's argument).
	Memory.Awareness = 1.f;
	Memory.Confidence = FMath::Max(Memory.Confidence, 0.8f);
	Memory.LastSource = ESoldierPerceptionSource::Damage;
	Memory.LastDamagedMeWorldTime = WorldTime;
	Memory.LastKnownLocation = SourceLocation;
	Memory.LocationUncertaintyCm = FMath::Max(Memory.LocationUncertaintyCm, 300.f);
	Memory.TimeSinceLastSeen = 0.f;
}

void USoldierPerceptionComponent::ReportNoise(const FVector& NoiseLocation, float Loudness, AActor* Instigator)
{
	const USoldierPerceptionConfig* Config = GetPerceptionConfig();
	UWorld* World = GetWorld();
	if (!Config || !World || !GetOwner() || !GetOwner()->HasAuthority() || !Instigator)
	{
		return;
	}

	const FVector EarLocation = GetEyeLocation();
	const float Distance = FVector::Dist(EarLocation, NoiseLocation);

	float EffectiveRange = Config->HearingRangeCm * FMath::Max(Loudness, 0.f);

	// design 7.3: a wall between the noise and the ear shortens the effective range.
	FCollisionQueryParams Params(SCENE_QUERY_STAT(SoldierHearing), /*bTraceComplex=*/false);
	Params.AddIgnoredActor(GetOwner());
	Params.AddIgnoredActor(Instigator);
	if (World->LineTraceTestByChannel(EarLocation, NoiseLocation, Config->SightTraceChannel, Params))
	{
		EffectiveRange *= Config->HearingOcclusionRangeScale;
	}

	if (Distance > EffectiveRange || EffectiveRange <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const float Falloff = FMath::Clamp(1.f - (Distance / EffectiveRange), 0.f, 1.f);

	FThreatMemory& Memory = FindOrAddMemory(Instigator);
	Memory.Awareness = FMath::Clamp(Memory.Awareness + Config->HearingAwarenessImpulse * Falloff, 0.f, 1.f);
	Memory.LastSource = ESoldierPerceptionSource::Hearing;

	// Only write the position if we have nothing better. A heard contact must not overwrite a seen
	// one with a worse estimate - that would make a soldier lose a target he is looking at.
	if (!Memory.bCurrentlyVisible)
	{
		Memory.LastKnownLocation = NoiseLocation;
		Memory.LocationUncertaintyCm = Config->HearingLocationUncertaintyCm;
		Memory.Confidence = FMath::Max(Memory.Confidence, 0.3f);
	}
}

void USoldierPerceptionComponent::ReceiveSharedThreat(AActor* Target, const FVector& ReportedLocation, const FVector& ReportedVelocity,
	float LocationSigmaCm, float ObservedAtSeconds, float ReportedConfidence)
{
	const USoldierPerceptionConfig* Config = GetPerceptionConfig();
	if (!Config || !Target || !GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	FThreatMemory& Memory = FindOrAddMemory(Target);

	// R1 - a first-hand observation that is at least as fresh as the report wins outright.
	// Seeing him myself beats being told, and being told must never move a contact I am looking at.
	if (Memory.bCurrentlyVisible || Memory.LastSeenWorldTime >= ObservedAtSeconds)
	{
		return;
	}

	// R2 - out-of-order radio traffic must not walk the contact backwards.
	if (ObservedAtSeconds <= Memory.LastSharedReportObservedTime)
	{
		return;
	}

	// R3 - design 9.3 caps a shared contact's confidence. Hearsay does not become certainty by
	// being repeated, and this cap is what keeps "직접 확인하러 간다" reachable.
	const float Confidence = FMath::Min(
		(ReportedConfidence > 0.f) ? ReportedConfidence : Config->SquadSharedConfidence,
		Config->SquadSharedConfidence);

	Memory.bSharedBySquad = true;
	Memory.LastSource = ESoldierPerceptionSource::Squad;
	Memory.LastSharedReportObservedTime = ObservedAtSeconds;
	Memory.Confidence = FMath::Max(Memory.Confidence, Confidence);
	Memory.Awareness = FMath::Max(Memory.Awareness, Config->SquadSharedAwareness);
	Memory.LastKnownLocation = ReportedLocation;
	Memory.LastKnownVelocity = ReportedVelocity;

	// The report's own sigma, never zero: a zero radius would claim first-hand accuracy for
	// something we were only told about, and PeekCheck would walk to a point instead of searching.
	Memory.LocationUncertaintyCm = FMath::Max(LocationSigmaCm, 200.f);
}


void USoldierPerceptionComponent::GetWeightedThreats(TArray<FVector>& OutLocations, TArray<float>& OutWeights) const
{
	OutLocations.Reset();
	OutWeights.Reset();

	const float WorldTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	for (const FThreatMemory& Memory : Memories)
	{
		if (!Memory.Target.IsValid() || Memory.AwarenessLevel == ESoldierAwareness::Unaware)
		{
			continue;
		}

		OutLocations.Add(Memory.GetEstimatedLocation(WorldTime));
		OutWeights.Add(Memory.GetThreatWeight());
	}
}

bool USoldierPerceptionComponent::GetPrimaryThreatWeighted(FVector& OutLocation, float& OutWeight) const
{
	const AActor* Target = PrimaryTarget.Get();
	if (!Target)
	{
		return false;
	}

	const float WorldTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	for (const FThreatMemory& Memory : Memories)
	{
		if (Memory.Target.Get() == Target)
		{
			OutLocation = Memory.GetEstimatedLocation(WorldTime);
			OutWeight = Memory.GetThreatWeight();
			return true;
		}
	}

	return false;
}

void USoldierPerceptionComponent::NotifyFired()
{
	LastFiredWorldTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
}

void USoldierPerceptionComponent::BindToAIPerception()
{
	// UAIPerceptionComponent lives on the controller in the usual setup, but a pawn-mounted one is
	// legal too, so try both. This is optional wiring: with no engine perception component the
	// hearing path simply relies on explicit ReportNoise calls.
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	UAIPerceptionComponent* Found = Owner->FindComponentByClass<UAIPerceptionComponent>();
	if (!Found)
	{
		if (const APawn* Pawn = Cast<APawn>(Owner))
		{
			if (AController* Controller = Pawn->GetController())
			{
				Found = Controller->FindComponentByClass<UAIPerceptionComponent>();
			}
		}
	}

	if (Found)
	{
		BoundAIPerception = Found;
		Found->OnTargetPerceptionUpdated.AddDynamic(this, &USoldierPerceptionComponent::HandleAIPerceptionUpdated);
	}
}

void USoldierPerceptionComponent::HandleAIPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor || !Stimulus.WasSuccessfullySensed())
	{
		return;
	}

	// Sight stimuli are deliberately ignored: this component runs its own sight model, and taking
	// the engine's boolean would reintroduce exactly the step-function detection design 7.1 removes.
	// [B] - the sense id comparison below is the documented API but has not been compiled here.
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		ReportNoise(Stimulus.StimulusLocation, Stimulus.Strength, Actor);
	}
}


// ---------------------------------------------------------------------------------------------
// USoldierPerceptionSubsystem
// ---------------------------------------------------------------------------------------------

TStatId USoldierPerceptionSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USoldierPerceptionSubsystem, STATGROUP_Tickables);
}

bool USoldierPerceptionSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void USoldierPerceptionSubsystem::RegisterSoldier(USoldierPerceptionComponent* Component)
{
	if (!Component)
	{
		return;
	}

	Registered.AddUnique(Component);

	// Stagger the first update so 45 soldiers spawned on the same frame do not all come due together.
	const UWorld* World = GetWorld();
	const float Now = World ? World->GetTimeSeconds() : 0.f;
	const float Interval = Component->GetUpdateInterval();
	Component->NextUpdateWorldTime = Now + FMath::FRand() * Interval;
}

void USoldierPerceptionSubsystem::UnregisterSoldier(USoldierPerceptionComponent* Component)
{
	Registered.RemoveAllSwap([Component](const TWeakObjectPtr<USoldierPerceptionComponent>& Entry)
	{
		return !Entry.IsValid() || Entry.Get() == Component;
	});
}

void USoldierPerceptionSubsystem::GatherHostileCandidates(const USoldierPerceptionComponent* Asker, float RangeCm, TArray<AActor*>& OutCandidates) const
{
	OutCandidates.Reset();

	if (!Asker || !Asker->GetOwner())
	{
		return;
	}

	const FVector AskerLocation = Asker->GetOwner()->GetActorLocation();
	const float RangeSq = RangeCm * RangeCm;

	for (const TWeakObjectPtr<USoldierPerceptionComponent>& Entry : Registered)
	{
		const USoldierPerceptionComponent* Other = Entry.Get();
		if (!Other || Other == Asker)
		{
			continue;
		}

		AActor* OtherOwner = Other->GetOwner();
		if (!OtherOwner)
		{
			continue;
		}

		// Faction comparison, not a class check. design 3.7.1 / P4.
		if (!Asker->HostileFactions.HasTag(Other->Faction))
		{
			continue;
		}

		if (FVector::DistSquared(AskerLocation, OtherOwner->GetActorLocation()) > RangeSq)
		{
			continue;
		}

		OutCandidates.Add(OtherOwner);
	}
}

void USoldierPerceptionSubsystem::Tick(float DeltaTime)
{
	// No Super::Tick - UTickableWorldSubsystem declares it PURE_VIRTUAL.

	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float Now = World->GetTimeSeconds();
	const int32 Count = Registered.Num();
	if (Count == 0)
	{
		return;
	}

	// Round robin from where the last frame stopped, so a component near the end of the array is
	// not starved when the per-frame cap bites.
	int32 Updated = 0;
	for (int32 Step = 0; Step < Count && Updated < MaxUpdatesPerFrame; ++Step)
	{
		const int32 Index = (RoundRobinCursor + Step) % Count;
		USoldierPerceptionComponent* Component = Registered[Index].Get();
		if (!Component)
		{
			continue;
		}

		if (Now < Component->NextUpdateWorldTime)
		{
			continue;
		}

		const float Interval = Component->GetUpdateInterval();

		// Real elapsed time, not the nominal interval: when the cap delays a component its
		// accumulator must still integrate the time that actually passed, or awareness rises
		// slower for crowded frames and the AI gets duller as the fight gets bigger.
		const float Elapsed = FMath::Min(Now - (Component->NextUpdateWorldTime - Interval), Interval * 4.f);

		Component->UpdatePerception(FMath::Max(Elapsed, KINDA_SMALL_NUMBER));
		Component->NextUpdateWorldTime = Now + Interval;
		++Updated;

		RoundRobinCursor = Index + 1;
	}

	if (Count > 0)
	{
		RoundRobinCursor %= Count;
	}
}
