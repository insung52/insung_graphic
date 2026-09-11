// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"
#include "SoldierAITypes.generated.h"

class UCurveFloat;
class UStateTree;

/**
 * Shared vocabulary for the individual soldier's perception / threat assessment / decision layers.
 *
 * Names follow design/2026-09-01_architecture.md 7.4 and 8.5 verbatim where the design already
 * named a type (FThreatMemory, FConsideration, FIntentDefinition). Fields the design did not name
 * are grouped under an "extension" comment so a later reader can see what came from the document
 * and what was added here.
 */

// ---------------------------------------------------------------------------------------------
// Intent tags (design 8.3 / ai/2026-09-02_upper_layer_plan.md 4.1)
//
// Native tags rather than a UENUM: the design's L2->L3 contract passes an FGameplayTag
// (FIntentSelection), and DataAssets need to reference intents that Blueprint content can add to.
// Squad-owned intents are declared here too so the tag namespace stays in one place, but the
// individual layer scores them to 0 unless the squad layer supplies its inputs (see
// SoldierScoringInputs below).
// ---------------------------------------------------------------------------------------------
SOLDIERLAB_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Intent_Idle);
SOLDIERLAB_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Intent_Overwatch);
SOLDIERLAB_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Intent_TakeCover);
SOLDIERLAB_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Intent_AimedFire);
SOLDIERLAB_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Intent_SuppressiveFire);
SOLDIERLAB_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Intent_Reposition);
SOLDIERLAB_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Intent_Advance);
SOLDIERLAB_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Intent_Retreat);
SOLDIERLAB_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Intent_Reload);
SOLDIERLAB_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Intent_PeekCheck);
SOLDIERLAB_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Intent_Regroup);

/** Intents that may pre-empt a running intent's minimum duration (design 8.3, "우선순위 인터럽트"). */
SOLDIERLAB_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Intent_Trait_Survival);


// ---------------------------------------------------------------------------------------------
// Enums
// ---------------------------------------------------------------------------------------------

/** design 7.1 - awareness is a 0..1 accumulator read as three bands, not a boolean. */
UENUM(BlueprintType)
enum class ESoldierAwareness : uint8
{
	Unaware		UMETA(DisplayName = "Unaware"),
	Suspicious	UMETA(DisplayName = "Suspicious"),
	Confirmed	UMETA(DisplayName = "Confirmed")
};

/** Which sense last wrote to a memory. Drives location uncertainty (design 7.3: hearing gives direction only). */
UENUM(BlueprintType)
enum class ESoldierPerceptionSource : uint8
{
	None,
	Sight,
	Hearing,
	Damage,
	Squad
};

/** design 6.5.2. Kept as an explicit state, not a health scalar. */
UENUM(BlueprintType)
enum class ESoldierHealthState : uint8
{
	Healthy,
	Injured,
	Downed,
	Dead
};

/** design 7.5 exposure stance term. Matches GASP's `stance` plus Prone, which GASP does not have yet. */
UENUM(BlueprintType)
enum class ESoldierStance : uint8
{
	Stand,
	Crouch,
	Prone
};

/** IAUS response curves (design 8.5). Parameter meanings follow Dave Mark's four-parameter form. */
UENUM(BlueprintType)
enum class EResponseCurve : uint8
{
	Linear,
	Quadratic,
	Logistic,
	Logit,
	/** Use CustomCurve instead of the four parameters. Escape hatch for shapes the four do not reach. */
	Custom
};


// ---------------------------------------------------------------------------------------------
// FThreatMemory - design 7.4
// ---------------------------------------------------------------------------------------------

/**
 * What one soldier remembers about one other actor.
 *
 * The first block is the design document's field list, unchanged. The extension block holds what
 * the perception implementation turned out to need; if any of it proves unnecessary it should be
 * deleted rather than left as dead weight.
 */
USTRUCT(BlueprintType)
struct SOLDIERLAB_API FThreatMemory
{
	GENERATED_BODY()

	// --- design 7.4 ---

	UPROPERTY(BlueprintReadOnly, Category = "Threat")
	TWeakObjectPtr<AActor> Target;

	/** Where the target was last actually observed. Not extrapolated - see GetEstimatedLocation. */
	UPROPERTY(BlueprintReadOnly, Category = "Threat")
	FVector LastKnownLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Threat")
	FVector LastKnownVelocity = FVector::ZeroVector;

	/** 0..1, decays with time since last observation. Distinct from Awareness. */
	UPROPERTY(BlueprintReadOnly, Category = "Threat")
	float Confidence = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Threat")
	float TimeSinceLastSeen = 0.f;

	/** 0..1 accumulator from design 7.1. Rises while visible, decays while not. */
	UPROPERTY(BlueprintReadOnly, Category = "Threat")
	float Awareness = 0.f;

	/** True when this came over the squad radio rather than from our own senses (design 9.3). */
	UPROPERTY(BlueprintReadOnly, Category = "Threat")
	bool bSharedBySquad = false;

	/** How dangerous this target is to me. Written by USoldierThreatAssessment. */
	UPROPERTY(BlueprintReadOnly, Category = "Threat")
	float ThreatLevel = 0.f;

	// --- extension (not in design 7.4) ---

	/** Banded reading of Awareness, with hysteresis applied so it does not flicker at a threshold. */
	UPROPERTY(BlueprintReadOnly, Category = "Threat")
	ESoldierAwareness AwarenessLevel = ESoldierAwareness::Unaware;

	/**
	 * How much shooting this target is worth, as opposed to how much it endangers me.
	 * Separate axis on purpose - a sniper behind full cover is high threat and low engagement value,
	 * and collapsing the two into one number loses that.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Threat")
	float EngagementValue = 0.f;

	/** 0..1 fraction of the target's sample points that were unoccluded on the last sight test (design 7.2). */
	UPROPERTY(BlueprintReadOnly, Category = "Threat")
	float VisibilityRatio = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Threat")
	bool bCurrentlyVisible = false;

	UPROPERTY(BlueprintReadOnly, Category = "Threat")
	ESoldierPerceptionSource LastSource = ESoldierPerceptionSource::None;

	/** World seconds. -1000 means "never". */
	UPROPERTY(BlueprintReadOnly, Category = "Threat")
	float LastSeenWorldTime = -1000.f;

	/** World seconds at which this target last damaged us. Feeds the "recently shot at me" threat term. */
	UPROPERTY(BlueprintReadOnly, Category = "Threat")
	float LastDamagedMeWorldTime = -1000.f;

	/** Is this target's aim pointed roughly at me right now. [B] - needs a real aim source, see OPEN_QUESTIONS Q13. */
	UPROPERTY(BlueprintReadOnly, Category = "Threat")
	bool bTargetAimingAtMe = false;

	/**
	 * Radius (cm) inside which the target actually is. Sight gives a small value, hearing a large one
	 * (design 7.3: sound gives direction, not position). Search behaviour should use this, not a point.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Threat")
	float LocationUncertaintyCm = 0.f;

	/** Copied from the target so the threat config can weight rifle vs. MG vs. unarmed without a cast. */
	UPROPERTY(BlueprintReadOnly, Category = "Threat")
	FGameplayTag TargetWeaponTag;

	/**
	 * Observation time of the most recent squad report merged into this memory (world seconds).
	 * Exists to satisfy merge rule R2 agreed with the squad draft: a report older than one already
	 * merged is discarded. Without it, out-of-order radio traffic walks a contact backwards.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Threat")
	float LastSharedReportObservedTime = -1000.f;

	/**
	 * Weight for consumers that need one number per contact - the cover layer's EQS threat
	 * contexts want (confidence x danger). Kept here so both sides use the same product.
	 */
	float GetThreatWeight() const { return FMath::Max(Confidence * ThreatLevel, 0.f); }

	/** Dead-reckoned position: last seen position plus last seen velocity times elapsed time (design 7.4). */
	FVector GetEstimatedLocation(float WorldTimeSeconds) const;

	bool IsValidMemory() const { return Target.IsValid(); }
};


// ---------------------------------------------------------------------------------------------
// FSoldierSelfState - what the decision layers need to know about their own body
// ---------------------------------------------------------------------------------------------

/**
 * Own-body state, pushed in by whoever owns each system (weapon, health, movement).
 *
 * This exists so perception and the brain do not have to know about a weapon class or a health
 * component that does not exist yet. Every field has a neutral default, so an unwired field
 * produces a defensible (usually conservative) score rather than a crash.
 */
USTRUCT(BlueprintType)
struct SOLDIERLAB_API FSoldierSelfState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Self")
	ESoldierHealthState HealthState = ESoldierHealthState::Healthy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Self", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HealthRatio = 1.f;

	/** Rounds left in the magazine, 0..1. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Self", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MagazineRatio = 1.f;

	/** Total reserve ammunition, 0..1. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Self", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ReserveAmmoRatio = 1.f;

	/** 0..1. Rises with rounds passing nearby, decays over time. Widens the aim cone (design 6.2). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Self", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SuppressionLevel = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Self")
	ESoldierStance Stance = ESoldierStance::Stand;

	/** 0..1, how well the currently occupied position covers us. 0 when standing in the open. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Self", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CurrentCoverQuality = 0.f;

	/** Muzzle is on target and has settled (design 6.2). Set by the weapon layer, read by AimedFire. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Self", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AimConvergence = 0.f;

	/** Effective engagement range of the carried weapon, cm. Used to normalise distance considerations. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Self", meta = (ClampMin = "1.0"))
	float WeaponEffectiveRangeCm = 6000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Self")
	FGameplayTag WeaponTag;

	/** World seconds at which we last took damage from anyone. -1000 means "never". */
	UPROPERTY(BlueprintReadWrite, Category = "Self")
	float LastDamagedWorldTime = -1000.f;

	/** Filled in by USoldierPerceptionComponent every update (design 7.5). Do not set by hand. */
	UPROPERTY(BlueprintReadOnly, Category = "Self", meta = (ClampMin = "0.0"))
	float Exposure = 1.f;

	/** Eye position used for sight tests and as the origin of the line of fire. Filled by perception. */
	UPROPERTY(BlueprintReadOnly, Category = "Self")
	FVector EyeLocation = FVector::ZeroVector;
};


// ---------------------------------------------------------------------------------------------
// Utility scoring - design 8.5
// ---------------------------------------------------------------------------------------------

/**
 * One scoring axis: a named input, normalised to 0..1, run through a response curve.
 *
 * design 8.5 gives InputName / Curve / Slope / Exponent / XShift / YShift. InputRange and bInvert
 * are added here because raw inputs are not in 0..1 (distance is centimetres, time is seconds) and
 * every axis would otherwise need its own bespoke normalisation in code, which P6 forbids.
 */
USTRUCT(BlueprintType)
struct SOLDIERLAB_API FConsideration
{
	GENERATED_BODY()

	/** Key into the brain's scoring input map. Use the SoldierScoringInputs names. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consideration")
	FName InputName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consideration")
	EResponseCurve Curve = EResponseCurve::Linear;

	/** Raw input range mapped onto 0..1 before the curve runs. Values outside are clamped. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consideration")
	FVector2D InputRange = FVector2D(0.f, 1.f);

	/** m - vertical scale. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consideration")
	float Slope = 1.f;

	/** k - exponent (Quadratic) or steepness (Logistic). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consideration")
	float Exponent = 1.f;

	/** c - horizontal shift. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consideration")
	float XShift = 0.f;

	/** b - vertical shift. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consideration")
	float YShift = 0.f;

	/** Mirror the result about 0.5. Cheaper than authoring the inverse of every curve. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consideration")
	bool bInvert = false;

	/** Used only when Curve == Custom. Sampled over 0..1 on X. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consideration")
	TObjectPtr<UCurveFloat> CustomCurve = nullptr;

	/** Raw input -> 0..1. Never returns NaN; out-of-domain inputs clamp rather than throw. */
	float Evaluate(float RawInput) const;
};

/** design 8.5. One candidate action, its axes, and the subtree that carries it out. */
USTRUCT(BlueprintType)
struct SOLDIERLAB_API FIntentDefinition
{
	GENERATED_BODY()

	/** Intent.TakeCover etc. See the TAG_Intent_* declarations above. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intent")
	FGameplayTag IntentTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intent")
	TArray<FConsideration> Considerations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intent", meta = (ClampMin = "0.0"))
	float BaseWeight = 1.f;

	/** Seconds this intent must run before another may replace it (design 8.3), 0.5~2.0 in the design. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intent", meta = (ClampMin = "0.0"))
	float MinDurationSec = 0.5f;

	/**
	 * Tags describing this intent. Intent.Trait.Survival lets it pre-empt another intent's
	 * MinDurationSec, which is how TakeCover / Retreat interrupt (design 8.3).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intent")
	FGameplayTagContainer Traits;

	/**
	 * L3 subtree for this intent. Referenced, not run, by the brain - the StateTree component
	 * enters it. [B] - whether the root tree links these as LinkedAsset states or the brain
	 * swaps the running tree is a StateTree wiring decision, see PLAN.md 6.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intent")
	TObjectPtr<UStateTree> ExecutionTree = nullptr;

	/** Intents below this score are never selected, no matter that they won. Prevents "least bad" flailing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intent", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinViableScore = 0.02f;
};

/**
 * Per-axis record of how one intent scored. This is not a debug feature: upper_layer_plan 2.3
 * makes "why did I pick this" a required output of the layer.
 */
USTRUCT(BlueprintType)
struct SOLDIERLAB_API FIntentScoreBreakdown
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	FGameplayTag IntentTag;

	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	float FinalScore = 0.f;

	/** Score before hysteresis / min-duration adjustment. */
	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	float RawScore = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	TArray<FName> AxisNames;

	/** Raw input per axis, parallel to AxisNames. */
	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	TArray<float> AxisInputs;

	/** Curve output per axis, parallel to AxisNames. The multiplicands of the product. */
	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	TArray<float> AxisFactors;
};

/** L2 -> L3 contract (upper_layer_plan 8: FIntentSelection). */
USTRUCT(BlueprintType)
struct SOLDIERLAB_API FIntentSelection
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Intent")
	FGameplayTag IntentTag;

	/** Primary target at the moment of selection, when the intent has one. */
	UPROPERTY(BlueprintReadOnly, Category = "Intent")
	TWeakObjectPtr<AActor> TargetActor;

	/** Best known / estimated position of TargetActor, or a world point for location intents. */
	UPROPERTY(BlueprintReadOnly, Category = "Intent")
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Intent")
	float Score = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Intent")
	float SelectedWorldTime = 0.f;

	/** Selection rationale, kept alongside the result (upper_layer_plan 2.3). */
	UPROPERTY(BlueprintReadOnly, Category = "Intent")
	FIntentScoreBreakdown Breakdown;
};


// ---------------------------------------------------------------------------------------------
// Scoring input names
// ---------------------------------------------------------------------------------------------

/**
 * Well-known keys for FConsideration::InputName.
 *
 * Anything the individual layer can compute for itself is filled by USoldierBrainComponent.
 * The Squad* keys are filled by the squad layer (someone else's module); when nothing fills them
 * they stay at their neutral default, which drives squad-gated intents to a score of 0. That is the
 * intended failure mode - a soldier with no squad does not lay down suppressive fire on his own.
 */
namespace SoldierScoringInputs
{
	// Self
	SOLDIERLAB_API extern const FName SelfExposure;
	SOLDIERLAB_API extern const FName SelfHealthRatio;
	SOLDIERLAB_API extern const FName SelfIsInjured;
	SOLDIERLAB_API extern const FName MagazineRatio;
	SOLDIERLAB_API extern const FName ReserveAmmoRatio;
	SOLDIERLAB_API extern const FName SuppressionLevel;
	SOLDIERLAB_API extern const FName CurrentCoverQuality;
	SOLDIERLAB_API extern const FName TimeSinceDamagedSec;
	SOLDIERLAB_API extern const FName AimConvergence;

	// Primary target / threat picture
	SOLDIERLAB_API extern const FName PrimaryTargetConfidence;
	SOLDIERLAB_API extern const FName PrimaryTargetAwareness;
	SOLDIERLAB_API extern const FName PrimaryTargetExposure;
	SOLDIERLAB_API extern const FName PrimaryTargetDistanceCm;
	SOLDIERLAB_API extern const FName PrimaryTargetVisibility;
	SOLDIERLAB_API extern const FName PrimaryThreatLevel;
	SOLDIERLAB_API extern const FName PrimaryEngagementValue;
	SOLDIERLAB_API extern const FName PrimaryTimeSinceSeenSec;
	SOLDIERLAB_API extern const FName LineOfFireClear;
	SOLDIERLAB_API extern const FName VisibleThreatCount;
	SOLDIERLAB_API extern const FName NearestThreatDistanceCm;
	SOLDIERLAB_API extern const FName TotalIncomingThreat;

	// Supplied by the cover / EQS layer (not owned by this module)
	SOLDIERLAB_API extern const FName BestCoverSlotScore;
	SOLDIERLAB_API extern const FName CoverSlotAvailable;

	// Supplied by the squad layer (not owned by this module)
	SOLDIERLAB_API extern const FName SquadHasSuppressionToken;
	SOLDIERLAB_API extern const FName SquadHasMovementToken;
	SOLDIERLAB_API extern const FName SquadMorale;
	SOLDIERLAB_API extern const FName SquadDistanceCm;
	SOLDIERLAB_API extern const FName OrderAggression;
	SOLDIERLAB_API extern const FName AlliesManoeuvring;
}
