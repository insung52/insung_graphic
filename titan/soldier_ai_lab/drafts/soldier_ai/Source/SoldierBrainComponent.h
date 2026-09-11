// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoldierAITypes.h"
#include "SoldierBrainComponent.generated.h"

class USoldierProfile;
class USoldierPerceptionComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSoldierIntentChanged, FGameplayTag, NewIntent, FGameplayTag, PreviousIntent);


/**
 * L2 BRAIN - the individual soldier's judgement layer (design 8, upper_layer_plan 5).
 *
 * Scores every candidate intent from the profile, picks one, and publishes it. It does not carry
 * the intent out; that is L3's job (StateTree), and the split is the design's central decision
 * (design 4.1, 8.2): mixing "choose what to do" with "carry it out" is what hard-codes priority
 * into a tree shape and produces the behaviour this project is replacing.
 *
 * Three things keep it from oscillating, all from design 8.3:
 *   - the running intent gets a bonus (hysteresis)
 *   - each intent declares a minimum duration
 *   - intents tagged Intent.Trait.Survival may pre-empt that minimum
 *
 * Every selection keeps its per-axis breakdown. upper_layer_plan 2.3 makes that a required output
 * of the layer rather than a debug extra: a utility system nobody can interrogate is the failure
 * mode this design set out to avoid, and A3's acceptance criterion depends on the glass-box UI.
 */
UCLASS(ClassGroup = (SoldierAI), meta = (BlueprintSpawnableComponent), Blueprintable)
class SOLDIERLAB_API USoldierBrainComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	USoldierBrainComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soldier|Config")
	TObjectPtr<USoldierProfile> Profile = nullptr;

	/** LOD tier, 0..2 (design 12.3). Drives the re-evaluation rate. Set by SignificanceManager. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soldier|Config", meta = (ClampMin = "0", ClampMax = "2"))
	int32 LODTier = 0;

	/** Keep the per-axis breakdown of every candidate, not just the winner. Costs a little memory, buys the glass box. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soldier|Debug")
	bool bRecordAllScores = true;

	UPROPERTY(BlueprintAssignable, Category = "Soldier|Brain")
	FSoldierIntentChanged OnIntentChanged;

	// --- output: the L2 -> L3 contract ---

	/** By value: a UFUNCTION cannot return a const reference. Use GetCurrentIntentRef in C++. */
	UFUNCTION(BlueprintPure, Category = "Soldier|Brain")
	FIntentSelection GetCurrentIntent() const { return CurrentIntent; }

	const FIntentSelection& GetCurrentIntentRef() const { return CurrentIntent; }

	UFUNCTION(BlueprintPure, Category = "Soldier|Brain")
	FGameplayTag GetCurrentIntentTag() const { return CurrentIntent.IntentTag; }

	/** All candidate scores from the last evaluation, highest first. For the glass-box UI. */
	UFUNCTION(BlueprintPure, Category = "Soldier|Brain")
	TArray<FIntentScoreBreakdown> GetLastScores() const { return LastScores; }

	// --- input ---

	/**
	 * Set a scoring input the individual layer does not own.
	 *
	 * The squad and cover layers push their values in through here (SoldierScoringInputs::Squad*,
	 * BestCoverSlotScore, ...). An input nobody sets stays at 0, which drives the intents that
	 * depend on it to a score of 0 - the intended behaviour for a soldier with no squad.
	 */
	UFUNCTION(BlueprintCallable, Category = "Soldier|Brain")
	void SetScoringInput(FName InputName, float Value);

	UFUNCTION(BlueprintPure, Category = "Soldier|Brain")
	float GetScoringInput(FName InputName) const;

	/** Force a re-evaluation on the next tick, ignoring the remaining minimum duration. For events that cannot wait. */
	UFUNCTION(BlueprintCallable, Category = "Soldier|Brain")
	void RequestImmediateReevaluation();

protected:

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Fills the input map from perception and own state. Squad-supplied keys are left untouched. */
	void GatherInputs();

	/** design 8.5: multiply the axes, then apply the IAUS make-up value. */
	float ScoreIntent(const FIntentDefinition& Definition, FIntentScoreBreakdown& OutBreakdown) const;

	void EvaluateAndSelect();

	float GetDecisionInterval() const;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Soldier|Brain")
	FIntentSelection CurrentIntent;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Soldier|Brain")
	TArray<FIntentScoreBreakdown> LastScores;

	UPROPERTY(Transient)
	TObjectPtr<USoldierPerceptionComponent> Perception = nullptr;

	/** Keyed by SoldierScoringInputs names. */
	TMap<FName, float> ScoringInputs;

	float TimeUntilNextDecision = 0.f;
	bool bImmediateReevaluationRequested = false;
};
