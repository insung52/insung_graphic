// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/EngineTypes.h"		// ECollisionChannel
#include "GameplayTagContainer.h"
#include "SoldierAITypes.h"
#include "SoldierAIConfig.generated.h"

class UCurveFloat;

/**
 * Tuning data for one soldier archetype.
 *
 * P6 in CLAUDE.md: everything tunable is data. Nothing in this file has a hard-coded counterpart
 * in the components - if a number matters, it lives here. Defaults are the design document's
 * numbers where it gave one, and a stated guess where it did not; the guesses are marked [B].
 */


// ---------------------------------------------------------------------------------------------
// Perception
// ---------------------------------------------------------------------------------------------

UCLASS(BlueprintType)
class SOLDIERLAB_API USoldierPerceptionConfig : public UDataAsset
{
	GENERATED_BODY()

public:

	// --- vision cone (design 7.2) ---

	/** Foveal half-angle, degrees. Full perception rate inside this. [A] design 7.2 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sight", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float FovealHalfAngleDeg = 15.f;

	/** Peripheral half-angle, degrees. Rate falls from PeripheralRateScale to MaxAngleRateScale across this band. [A] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sight", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float PeripheralHalfAngleDeg = 60.f;

	/** Outer limit of vision, degrees. Nothing is seen beyond this. [A] design 7.2 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sight", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float MaxHalfAngleDeg = 110.f;

	/** Rate multiplier at the edge of the peripheral band. design 7.2 gives 0.3~0.7. [A] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sight", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PeripheralRateScale = 0.3f;

	/** Rate multiplier at MaxHalfAngleDeg. design 7.2 gives ~0.1. [A] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sight", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxAngleRateScale = 0.1f;

	/** Optional override of the whole angular falloff. X = angle/MaxHalfAngleDeg (0..1), Y = multiplier. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sight")
	TObjectPtr<UCurveFloat> AngleFalloffCurve = nullptr;

	/** Sight range, cm. design 7.2 says 100 m. [A] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sight", meta = (ClampMin = "1.0"))
	float MaxSightRangeCm = 10000.f;

	/**
	 * Distance at which the perception rate halves, cm. Produces 1/(1+(d/h)^2), which is the
	 * "close to inverse square" the design asks for while staying finite at d=0. [B] - the design
	 * states the shape, not this parameterisation.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sight", meta = (ClampMin = "1.0"))
	float HalfRateDistanceCm = 3000.f;

	/** Optional override of distance falloff. X = distance/MaxSightRangeCm (0..1), Y = multiplier. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sight")
	TObjectPtr<UCurveFloat> DistanceFalloffCurve = nullptr;

	// --- occlusion sampling (design 7.2: "머리/가슴/어깨 3~5개 샘플점", partial visibility 0..1) ---

	/**
	 * Sockets or bones traced on the target. [B] - names assume the UE5 mannequin hierarchy that
	 * SK_UEFN_Mannequin uses; verify against the actual skeleton before trusting them. A name that
	 * does not resolve is skipped, and if none resolve the actor's bounds centre is used.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sight")
	TArray<FName> VisibilitySampleBones = { FName("head"), FName("spine_03"), FName("upperarm_l"), FName("upperarm_r"), FName("pelvis") };

	/** Bone used as our own eye position. Falls back to the actor's view point when it does not resolve. [B] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sight")
	FName EyeBoneName = FName("head");

	/** Channel for the occlusion trace. Not the cover channel - this one must be blocked by anything opaque. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sight")
	TEnumAsByte<ECollisionChannel> SightTraceChannel = ECC_Visibility;

	/** Hard cap on traces per perception update, for the 0.7 ms budget in design 12.2. [B] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sight", meta = (ClampMin = "1"))
	int32 MaxSightTracesPerUpdate = 12;

	// --- awareness accumulator (design 7.1) ---

	/**
	 * Awareness gained per second under ideal conditions (dead ahead, close, fully visible, moving).
	 * 1.0 means "about a second to go from nothing to confirmed at point blank". [B]
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Awareness", meta = (ClampMin = "0.0"))
	float BaseAwarenessRatePerSec = 1.0f;

	/** Awareness lost per second while the target is not visible. Deliberately much slower than the gain. [B] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Awareness", meta = (ClampMin = "0.0"))
	float AwarenessDecayPerSec = 0.25f;

	/** [A] design 7.1 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Awareness", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SuspiciousThreshold = 0.3f;

	/** [A] design 7.1 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Awareness", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ConfirmedThreshold = 0.7f;

	/**
	 * Band hysteresis. A memory sitting exactly on a threshold would otherwise flip level every
	 * update and spam the squad share / intent change. Not in the design; added for the same
	 * reason the design added intent hysteresis in 8.3. [B]
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Awareness", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float AwarenessBandHysteresis = 0.05f;

	/** Target speed (cm/s) at which the movement bonus is fully applied. [B] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Awareness", meta = (ClampMin = "1.0"))
	float TargetSpeedForFullBonusCm = 300.f;

	/** Rate multiplier for a fully moving target. 1.0 disables the movement term. [B] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Awareness", meta = (ClampMin = "1.0"))
	float MaxTargetSpeedRateMultiplier = 1.5f;

	/**
	 * Global multiplier for lighting / weather / smoke (design 7.1 f(조명/기상)).
	 * Left as one scalar until there is a lighting model to read; a level or volume can drive it.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Awareness", meta = (ClampMin = "0.0"))
	float EnvironmentRateScale = 1.f;

	// --- memory (design 7.4) ---

	/** Confidence lost per second once the target is out of sight. [B] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Memory", meta = (ClampMin = "0.0"))
	float ConfidenceDecayPerSec = 0.1f;

	/** Seconds after which a memory with no reinforcement is dropped entirely. [B] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Memory", meta = (ClampMin = "0.0"))
	float MemoryLifetimeSec = 30.f;

	/** Confidence a squad-shared contact starts at. design 9.3 / upper_layer_plan 6.5 give 0.6. [A] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Memory", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SquadSharedConfidence = 0.6f;

	/** Awareness a squad-shared contact starts at. Below ConfirmedThreshold on purpose: heard it, have not seen it. [B] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Memory", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SquadSharedAwareness = 0.5f;

	/** Most memories kept at once. Beyond this the lowest-threat ones are dropped. [B] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Memory", meta = (ClampMin = "1"))
	int32 MaxTrackedThreats = 12;

	// --- hearing (design 7.3) ---

	/** Range at which a unit-loudness noise is still heard, cm. Scaled by the reported loudness. [B] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hearing", meta = (ClampMin = "0.0"))
	float HearingRangeCm = 5000.f;

	/** Effective range multiplier when a wall sits between the noise and the ear (Tarkov model, design 7.3). [B] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hearing", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HearingOcclusionRangeScale = 0.4f;

	/** Awareness added by one heard noise at point blank, falling off with distance. [B] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hearing", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HearingAwarenessImpulse = 0.35f;

	/**
	 * Positional uncertainty a heard contact is recorded with, cm. design 7.3 is explicit that
	 * hearing gives direction, not position, and that this inaccuracy is what produces the
	 * "advance warily toward roughly there" behaviour. Do not set it small. [A] on the intent,
	 * [B] on the number.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hearing", meta = (ClampMin = "0.0"))
	float HearingLocationUncertaintyCm = 800.f;

	// --- exposure, the reverse model (design 7.5) ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Exposure", meta = (ClampMin = "0.0"))
	float ExposureStanding = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Exposure", meta = (ClampMin = "0.0"))
	float ExposureCrouched = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Exposure", meta = (ClampMin = "0.0"))
	float ExposureProne = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Exposure", meta = (ClampMin = "0.0"))
	float ExposureStationary = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Exposure", meta = (ClampMin = "0.0"))
	float ExposureWalking = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Exposure", meta = (ClampMin = "0.0"))
	float ExposureRunning = 1.4f;

	/** Muzzle flash and noise (design 7.5). Applied for FiringExposureDecaySec after the last shot. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Exposure", meta = (ClampMin = "0.0"))
	float ExposureFiring = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Exposure", meta = (ClampMin = "0.0"))
	float FiringExposureDecaySec = 1.5f;

	/** Speed (cm/s) at or above which the running exposure term applies fully. [B] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Exposure", meta = (ClampMin = "1.0"))
	float RunningSpeedCm = 400.f;

	// --- scheduling (design 12.2 / 12.3) ---

	/** Perception updates per second at LOD tier T0. design 12.3 gives 10 Hz. [A] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scheduling", meta = (ClampMin = "0.1"))
	float UpdateHzTier0 = 10.f;

	/** design 12.3 gives 5 Hz. [A] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scheduling", meta = (ClampMin = "0.1"))
	float UpdateHzTier1 = 5.f;

	/** design 12.3 gives 2 Hz. [A] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scheduling", meta = (ClampMin = "0.1"))
	float UpdateHzTier2 = 2.f;
};


// ---------------------------------------------------------------------------------------------
// Threat assessment
// ---------------------------------------------------------------------------------------------

UCLASS(BlueprintType)
class SOLDIERLAB_API USoldierThreatConfig : public UDataAsset
{
	GENERATED_BODY()

public:

	// --- how dangerous is this target to me ---

	/** Distance at which danger halves, cm. Threat falls off as 1/(1+(d/h)^2). [B] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Threat", meta = (ClampMin = "1.0"))
	float DangerHalfDistanceCm = 2000.f;

	/** Optional override. X = distance/MaxConsideredRangeCm, Y = 0..1 danger. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Threat")
	TObjectPtr<UCurveFloat> DangerDistanceCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Threat", meta = (ClampMin = "1.0"))
	float MaxConsideredRangeCm = 15000.f;

	/** Applied when the target is aiming at us. The single largest term in the design's intent - it is what makes a soldier react to being singled out. [B] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Threat", meta = (ClampMin = "1.0"))
	float AimingAtMeMultiplier = 2.0f;

	/** Seconds after taking a hit from a target that the recency multiplier still applies. [B] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Threat", meta = (ClampMin = "0.0"))
	float RecentDamageWindowSec = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Threat", meta = (ClampMin = "1.0"))
	float RecentDamageMultiplier = 2.0f;

	/**
	 * Per weapon-class threat multiplier, keyed on FThreatMemory::TargetWeaponTag.
	 * A tag with no entry scores 1.0. Keeps "an MG is worse than a pistol" out of code (P6).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Threat")
	TMap<FGameplayTag, float> WeaponThreatMultipliers;

	/**
	 * A target we are not sure about is not a smaller threat by the same amount that it is less
	 * certain - the design's 8.3 AimedFire axis multiplies by confidence, but threat should not go
	 * to zero for a contact we heard. This is the floor. [B]
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Threat", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinConfidenceThreatScale = 0.25f;

	// --- is it worth shooting ---

	/** Below this confidence we do not open aimed fire at all. Stops shooting at rumours. [B] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Engagement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinConfidenceToEngage = 0.5f;

	/** Below this visible fraction the target is not worth an aimed shot; suppression is the answer instead. [B] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Engagement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinVisibilityToEngage = 0.2f;

	/** Beyond the weapon's effective range engagement value falls off; this is how far past it still counts. [B] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Engagement", meta = (ClampMin = "1.0"))
	float RangeFalloffSlackCm = 3000.f;

	// --- target switching (design 6.2: "표적 전환 시 수렴 타이머 리셋") ---

	/**
	 * A challenger must beat the current primary target's score by this fraction before we switch.
	 * Without it the soldier ping-pongs between two equal targets and never finishes settling his
	 * aim, which by design 6.2 means he never fires. Same medicine as the intent hysteresis in 8.3. [B]
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Engagement", meta = (ClampMin = "0.0"))
	float TargetSwitchMargin = 0.2f;

	/** Minimum seconds to keep a primary target before any switch is allowed, barring it dying or vanishing. [B] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Engagement", meta = (ClampMin = "0.0"))
	float MinTargetHoldSec = 1.0f;

	/**
	 * How the primary target is picked: pure danger, pure payoff, or a blend.
	 * 0 = most dangerous to me, 1 = most worth shooting. The design does not settle this - see
	 * OPEN_QUESTIONS Q12.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Engagement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ThreatVsValueBias = 0.5f;
};


// ---------------------------------------------------------------------------------------------
// Profile - one asset per archetype
// ---------------------------------------------------------------------------------------------

/**
 * design 5.3 of the upper layer plan: Profile_Recruit / Profile_Veteran / Profile_Leader are the
 * same code with different curves. This asset is the whole of that difference, which is also why
 * ally and enemy need no separate class (CLAUDE.md P4, design 3.7.1).
 */
UCLASS(BlueprintType)
class SOLDIERLAB_API USoldierProfile : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Profile")
	TObjectPtr<USoldierPerceptionConfig> Perception = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Profile")
	TObjectPtr<USoldierThreatConfig> Threat = nullptr;

	/** Candidate intents. Order is irrelevant; the scorer sorts by score. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Profile")
	TArray<FIntentDefinition> Intents;

	/** Utility re-evaluations per second at LOD tier T0. design 4.2 gives 4~10 Hz. [A] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Profile", meta = (ClampMin = "0.1"))
	float DecisionHzTier0 = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Profile", meta = (ClampMin = "0.1"))
	float DecisionHzTier1 = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Profile", meta = (ClampMin = "0.1"))
	float DecisionHzTier2 = 2.f;

	/** Bonus added to the currently running intent's score. design 8.3 gives 10~15%. [A] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Profile", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CurrentIntentBonus = 0.12f;
};
