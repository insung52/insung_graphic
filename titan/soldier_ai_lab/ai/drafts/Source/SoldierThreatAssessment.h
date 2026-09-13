// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SoldierAITypes.h"
#include "UObject/Object.h"
#include "SoldierThreatAssessment.generated.h"

class USoldierThreatConfig;

/**
 * Turns a list of remembered contacts into two numbers per contact and one primary target.
 *
 * The two numbers are deliberately not one:
 *
 *   ThreatLevel     how much this contact endangers ME       -> drives TakeCover / Retreat
 *   EngagementValue how much shooting it is worth to me      -> drives AimedFire / SuppressiveFire
 *
 * design 8.3 already splits its intent axes this way (TakeCover reads exposure and incoming fire,
 * AimedFire reads target confidence and target exposure), so collapsing both into a single
 * "threat" scalar would force the intents to share an axis that means two different things. The
 * machine gunner behind a wall is the case that separates them: high threat, low engagement value,
 * and the right answer is cover plus suppression, not an aimed shot.
 *
 * This is a plain UObject rather than a component: it holds no per-frame state (the state is in
 * FThreatMemory) and one instance per soldier is enough. Instanced + Blueprintable so a profile
 * can subclass the scoring without touching the perception component.
 *
 * Everything tunable lives in USoldierThreatConfig (P6). This class holds the shape, not the numbers.
 */
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class SOLDIERLAB_API USoldierThreatAssessment : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * How dangerous this contact is to us right now, 0..1-ish (it is a product of multipliers and
	 * may exceed 1 when several aggravating terms stack; callers compare it, they do not display it).
	 *
	 * Terms, all from design 7.4's "거리·무기·조준여부" plus the recency the design's Intent axes need:
	 *   - proximity            close is worse
	 *   - confidence           a contact we are unsure about is discounted, but not to zero
	 *   - weapon class         data-driven multiplier
	 *   - aiming at me         the biggest single multiplier
	 *   - recently shot me     decays over RecentDamageWindowSec
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Soldier|Threat")
	float ScoreThreat(const FThreatMemory& Memory, const FSoldierSelfState& Self, const USoldierThreatConfig* Config, float WorldTimeSeconds) const;
	virtual float ScoreThreat_Implementation(const FThreatMemory& Memory, const FSoldierSelfState& Self, const USoldierThreatConfig* Config, float WorldTimeSeconds) const;

	/**
	 * How worthwhile it is to shoot this contact, 0..1. Zero means "do not open aimed fire":
	 * not confident enough, not visible enough, or out of useful range.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Soldier|Threat")
	float ScoreEngagementValue(const FThreatMemory& Memory, const FSoldierSelfState& Self, const USoldierThreatConfig* Config, float WorldTimeSeconds) const;
	virtual float ScoreEngagementValue_Implementation(const FThreatMemory& Memory, const FSoldierSelfState& Self, const USoldierThreatConfig* Config, float WorldTimeSeconds) const;

	/**
	 * Pick the primary target, with hysteresis.
	 *
	 * A challenger must beat the incumbent by Config->TargetSwitchMargin, and the incumbent must
	 * have been held at least Config->MinTargetHoldSec. design 6.2 makes this structural rather
	 * than cosmetic: switching targets resets the aim convergence timer, so a soldier who reselects
	 * every update never settles and therefore never fires. Ping-ponging does not look indecisive,
	 * it looks broken.
	 *
	 * Returns nullptr when nothing is worth tracking.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Soldier|Threat")
	AActor* SelectPrimaryTarget(const TArray<FThreatMemory>& Memories, const FSoldierSelfState& Self, const USoldierThreatConfig* Config, AActor* CurrentTarget, float CurrentHeldForSec) const;
	virtual AActor* SelectPrimaryTarget_Implementation(const TArray<FThreatMemory>& Memories, const FSoldierSelfState& Self, const USoldierThreatConfig* Config, AActor* CurrentTarget, float CurrentHeldForSec) const;

	/**
	 * Blend of danger and payoff used to rank candidates for primary target.
	 * Config->ThreatVsValueBias picks the mix; the design does not settle it (OPEN_QUESTIONS Q12).
	 */
	UFUNCTION(BlueprintPure, Category = "Soldier|Threat")
	static float ScoreTargetPriority(const FThreatMemory& Memory, const USoldierThreatConfig* Config);
};
