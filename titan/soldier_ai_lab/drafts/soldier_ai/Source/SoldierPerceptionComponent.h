// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "SoldierAITypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "SoldierPerceptionComponent.generated.h"

class USoldierPerceptionConfig;
class USoldierThreatConfig;
class USoldierProfile;
class USoldierThreatAssessment;
class UAIPerceptionComponent;
struct FAIStimulus;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSoldierAwarenessChanged, AActor*, Target, ESoldierAwareness, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSoldierPrimaryTargetChanged, AActor*, NewTarget);


/**
 * One soldier's senses and memory (design 7).
 *
 * What this replaces: the current project's "sphere overlap == target acquired", which design 7.1
 * names as the single largest cause of the AI reading as mechanical. Here, detection is a process
 * that takes time, degrades with angle / distance / occlusion, and fades rather than snapping off.
 *
 * What this is NOT: a replacement for UAIPerceptionComponent. Engine perception stays as an event
 * source for hearing and damage (design 7.1); sight is done here because the engine's sight sense
 * is a boolean per target and the accumulator needs the continuous inputs behind that boolean.
 *
 * Authority: every update path is gated on the owner having authority (CLAUDE.md P5, design 4.3.6).
 * Nothing here replicates - the design replicates only the L2 result (design 4.3.3).
 */
UCLASS(ClassGroup = (SoldierAI), meta = (BlueprintSpawnableComponent), Blueprintable)
class SOLDIERLAB_API USoldierPerceptionComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	USoldierPerceptionComponent();

	// --- identity (design 3.7.1: faction is data, not a subclass) ---

	/** Faction.Ally / Faction.Enemy / Faction.Neutral. Compared, never branched on. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soldier|Identity")
	FGameplayTag Faction;

	/** Factions this soldier treats as hostile. Data, so ally and enemy run the same code path (P4). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soldier|Identity")
	FGameplayTagContainer HostileFactions;

	// --- configuration ---

	/** Archetype asset. Perception/Threat configs are read from here when set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soldier|Config")
	TObjectPtr<USoldierProfile> Profile = nullptr;

	/** Direct override, used when Profile is null. Handy for a one-off test actor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soldier|Config")
	TObjectPtr<USoldierPerceptionConfig> PerceptionConfigOverride = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soldier|Config")
	TObjectPtr<USoldierThreatConfig> ThreatConfigOverride = nullptr;

	/**
	 * Scores danger and engagement value for each memory. Instanced so a profile can subclass it in
	 * Blueprint without touching this component.
	 */
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly, Category = "Soldier|Config")
	TObjectPtr<USoldierThreatAssessment> ThreatAssessment = nullptr;

	/** LOD tier, 0..2 (design 12.3). Set by SignificanceManager; defaults to the most expensive tier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soldier|Config", meta = (ClampMin = "0", ClampMax = "2"))
	int32 LODTier = 0;

	// --- own-body state, pushed in by the weapon / health / movement owners ---

	/** Read by threat assessment and by the brain. Exposure and EyeLocation are overwritten here. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soldier|State")
	FSoldierSelfState SelfState;

	// --- events ---

	UPROPERTY(BlueprintAssignable, Category = "Soldier|Perception")
	FSoldierAwarenessChanged OnAwarenessChanged;

	UPROPERTY(BlueprintAssignable, Category = "Soldier|Perception")
	FSoldierPrimaryTargetChanged OnPrimaryTargetChanged;

	// --- queries (StateTree conditions and the brain read these) ---

	UFUNCTION(BlueprintPure, Category = "Soldier|Perception")
	AActor* GetPrimaryTarget() const { return PrimaryTarget.Get(); }

	UFUNCTION(BlueprintPure, Category = "Soldier|Perception")
	bool GetThreatMemory(AActor* Target, FThreatMemory& OutMemory) const;

	/** By value: a UFUNCTION cannot return a const reference to an array. Use GetMemoriesRef in C++. */
	UFUNCTION(BlueprintPure, Category = "Soldier|Perception")
	TArray<FThreatMemory> GetThreatMemories() const { return Memories; }

	const TArray<FThreatMemory>& GetMemoriesRef() const { return Memories; }

	/** Best estimate of where the primary target is, dead-reckoned. False when there is no primary target. */
	UFUNCTION(BlueprintPure, Category = "Soldier|Perception")
	bool GetPrimaryTargetEstimatedLocation(FVector& OutLocation, float& OutUncertaintyCm) const;

	UFUNCTION(BlueprintPure, Category = "Soldier|Perception")
	int32 CountThreatsAtLeast(ESoldierAwareness MinLevel) const;

	/** design 7.5. How visible this soldier is to others right now. Recomputed every update. */
	UFUNCTION(BlueprintPure, Category = "Soldier|Perception")
	float GetExposure() const { return SelfState.Exposure; }

	UFUNCTION(BlueprintPure, Category = "Soldier|Perception")
	FVector GetEyeLocation() const;

	/**
	 * Direction the head is looking. design 7.2 keeps this separate from the aim direction so that
	 * a scan behaviour actually changes what gets seen. Defaults to the control rotation; L3 can
	 * override it while scanning.
	 */
	UFUNCTION(BlueprintPure, Category = "Soldier|Perception")
	FVector GetGazeDirection() const;

	UFUNCTION(BlueprintCallable, Category = "Soldier|Perception")
	void SetGazeOverride(const FVector& WorldDirection, bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Soldier|Perception")
	bool IsHostileTo(const AActor* Other) const;

	// --- external stimulus entry points ---

	/**
	 * Called by whatever owns damage. Kept as an explicit call rather than binding OnTakeAnyDamage,
	 * because the damage pipeline for this project is not decided yet and a wrong binding would be
	 * silent. design 7.1 lists damage as a detection source.
	 */
	UFUNCTION(BlueprintCallable, Category = "Soldier|Perception")
	void ReportDamageFrom(AActor* Instigator, float Amount, const FVector& SourceLocation);

	/** Loudness 1.0 == a rifle shot at HearingRangeCm. Occlusion is tested here, per design 7.3. */
	UFUNCTION(BlueprintCallable, Category = "Soldier|Perception")
	void ReportNoise(const FVector& NoiseLocation, float Loudness, AActor* Instigator);

	/**
	 * Squad layer entry point (design 9.3 / upper_layer_plan 6.5). A shared contact arrives at
	 * lower confidence than a seen one, and is flagged so "go and check it yourself" has something
	 * to key on. Owned by the squad module - this is only the receiving end.
	 *
	 * Implements the three merge rules the squad draft asks the receiver to guarantee:
	 *   R1  a first-hand observation that is fresher than the report wins; never overwrite it
	 *   R2  a report older than one already merged is discarded
	 *   R3  merged confidence never exceeds SquadSharedConfidence (design 9.3 gives 0.6)
	 *
	 * The squad module's ISoldierThreatMemorySink::ReceiveSharedThreat(const FSharedThreat&) is a
	 * one-line adapter onto this. Kept as loose parameters so this component does not have to
	 * include the squad module's header - the two are being drafted in parallel.
	 */
	UFUNCTION(BlueprintCallable, Category = "Soldier|Perception")
	void ReceiveSharedThreat(AActor* Target, const FVector& ReportedLocation, const FVector& ReportedVelocity,
		float LocationSigmaCm, float ObservedAtSeconds, float ReportedConfidence);

	/**
	 * Threat list for the cover layer's EQS contexts (ISoldierThreatProvider).
	 * Weight is confidence x danger, per FThreatMemory::GetThreatWeight.
	 * Returns only contacts we are at least suspicious of.
	 */
	UFUNCTION(BlueprintCallable, Category = "Soldier|Perception")
	void GetWeightedThreats(TArray<FVector>& OutLocations, TArray<float>& OutWeights) const;

	UFUNCTION(BlueprintCallable, Category = "Soldier|Perception")
	bool GetPrimaryThreatWeighted(FVector& OutLocation, float& OutWeight) const;

	/** Call when firing. Raises exposure for FiringExposureDecaySec (design 7.5). */
	UFUNCTION(BlueprintCallable, Category = "Soldier|Perception")
	void NotifyFired();

	/** design 6.5.3 death cleanup: other soldiers' memories of this actor must be invalidated. */
	UFUNCTION(BlueprintCallable, Category = "Soldier|Perception")
	void ForgetTarget(AActor* Target);

	// --- driven by USoldierPerceptionSubsystem ---

	/** One perception step. DeltaTime is the real elapsed time since this component last updated. */
	void UpdatePerception(float DeltaTime);

	float GetUpdateInterval() const;

	/** Next world time this component wants to be updated. Managed by the subsystem. */
	float NextUpdateWorldTime = 0.f;

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	const USoldierPerceptionConfig* GetPerceptionConfig() const;
	const USoldierThreatConfig* GetThreatConfig() const;

	/** Sight pass for one candidate. Returns the awareness rate, 0 when not visible. */
	float EvaluateSight(AActor* Candidate, float& OutVisibilityRatio, int32& InOutTraceBudget) const;

	/** Fraction of the target's sample points reachable from our eye. design 7.2 partial visibility. */
	float ComputeVisibilityRatio(const AActor* Target, int32& InOutTraceBudget) const;

	void UpdateExposure();
	void DecayMemories(float DeltaTime);
	void UpdateThreatLevels();
	void UpdatePrimaryTarget();

	FThreatMemory& FindOrAddMemory(AActor* Target);
	void ApplyAwarenessBands();

	/** The only place engine perception API is touched. Hearing / damage stimuli only. */
	void BindToAIPerception();

	UFUNCTION()
	void HandleAIPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	/** design 7.4. One entry per remembered actor; capacity limited by MaxTrackedThreats. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Soldier|Perception")
	TArray<FThreatMemory> Memories;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Soldier|Perception")
	TWeakObjectPtr<AActor> PrimaryTarget;

	float PrimaryTargetSelectedWorldTime = 0.f;
	float LastFiredWorldTime = -1000.f;

	bool bGazeOverrideActive = false;
	FVector GazeOverrideDirection = FVector::ForwardVector;

	/** Cached so the subsystem does not have to walk the owner chain every frame. */
	UPROPERTY(Transient)
	TObjectPtr<UAIPerceptionComponent> BoundAIPerception = nullptr;
};


/**
 * Registry and scheduler for every USoldierPerceptionComponent in the world.
 *
 * Two jobs, both from design 12.2:
 *   1. Candidate lists. Without a registry each soldier would have to sweep the world for targets;
 *      45 soldiers doing that is the shape of the cost the budget cannot absorb.
 *   2. Round-robin scheduling. Components declare a target rate (LOD tier) and the subsystem
 *      spreads their updates across frames under a per-frame cap, so the 0.7 ms perception budget
 *      is a cap rather than an average.
 *
 * Server only - RegisterSoldier is a no-op on a client (design 4.3.2).
 */
UCLASS()
class SOLDIERLAB_API USoldierPerceptionSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	void RegisterSoldier(USoldierPerceptionComponent* Component);
	void UnregisterSoldier(USoldierPerceptionComponent* Component);

	/** Everyone registered. Callers filter by faction themselves. */
	const TArray<TWeakObjectPtr<USoldierPerceptionComponent>>& GetAllSoldiers() const { return Registered; }

	/** Registered soldiers hostile to Asker, within RangeCm. Does no line of sight work. */
	void GatherHostileCandidates(const USoldierPerceptionComponent* Asker, float RangeCm, TArray<AActor*>& OutCandidates) const;

	/** Most component updates allowed in one frame. Raise only with a measurement (design 12.2). [B] */
	UPROPERTY(EditAnywhere, Category = "Soldier|Perception", meta = (ClampMin = "1"))
	int32 MaxUpdatesPerFrame = 8;

private:

	UPROPERTY()
	TArray<TWeakObjectPtr<USoldierPerceptionComponent>> Registered;

	int32 RoundRobinCursor = 0;
};
