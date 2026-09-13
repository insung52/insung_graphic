// SoldierLab — L1 SQUAD 튜닝 데이터
// 초안 (2026-09-09). 컴파일 검증 없음.
//
// CLAUDE.md P6: 튜닝 대상은 전부 데이터. 코드에 상수를 박지 않는다.
// PLAN.md 10절. 아래 기본값은 전부 [B] 잠정치이며 계측·관찰로 확정한다.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "SquadTypes.h"
#include "../Command/SoldierOrder.h"
#include "SquadTuningData.generated.h"

// ---------------------------------------------------------------------------
// 플랜 규칙 (PLAN.md 3.3~3.4절)
// ---------------------------------------------------------------------------

USTRUCT(BlueprintType)
struct FSquadPlanRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Plan")
	ESquadPlan Plan = ESquadPlan::None;

	/** 이 Verb 들 중 하나일 때만 후보. 비우면 모든 Verb 에서 후보 */
	UPROPERTY(EditAnywhere, Category = "Plan|Gate")
	TArray<EOrderVerb> AllowedVerbs;

	/** true = 접촉 중일 때만, false = 접촉 없을 때만, 미설정 = 무관 */
	UPROPERTY(EditAnywhere, Category = "Plan|Gate", meta = (InlineEditConditionToggle))
	bool bUseContactGate = false;

	UPROPERTY(EditAnywhere, Category = "Plan|Gate", meta = (EditCondition = "bUseContactGate"))
	bool bRequiresContact = true;

	UPROPERTY(EditAnywhere, Category = "Plan|Gate", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinMorale = 0.f;

	UPROPERTY(EditAnywhere, Category = "Plan|Gate", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxMorale = 1.f;

	/** Flank 전용 — Constraints.bAllowFlanking 과 회랑 검증을 함께 요구한다 */
	UPROPERTY(EditAnywhere, Category = "Plan|Gate")
	bool bRequiresFlankCorridor = false;

	UPROPERTY(EditAnywhere, Category = "Plan|Gate")
	int32 MinAliveMembers = 1;

	/** 게이트를 통과한 후보들끼리의 비교 가중 */
	UPROPERTY(EditAnywhere, Category = "Plan")
	float RuleWeight = 1.f;

	/** 이 플랜일 때 개인에게 내려보내는 공세성 보정 (명령 Aggression 에 곱한다) */
	UPROPERTY(EditAnywhere, Category = "Plan", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float AggressionScale = 1.f;

	/** 이동 토큰을 조 단위로 발급하는가 — ★ 바운딩 오버워치의 실체 (PLAN.md 4.1절) */
	UPROPERTY(EditAnywhere, Category = "Plan|Token")
	bool bMovementTokenPerElement = false;

	/** 생존자 수 N 에 대한 토큰 상한: ceil(N * Ratio) + Offset, 최소 0 */
	UPROPERTY(EditAnywhere, Category = "Plan|Token", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SuppressionTokenRatio = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Plan|Token")
	int32 SuppressionTokenOffset = 0;

	UPROPERTY(EditAnywhere, Category = "Plan|Token", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MovementTokenRatio = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Plan|Token")
	int32 MovementTokenOffset = 0;
};

UCLASS(BlueprintType)
class SOLDIERLAB_API USquadPlanRuleSet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Plan")
	TArray<FSquadPlanRule> Rules;

	const FSquadPlanRule* FindRule(ESquadPlan Plan) const
	{
		return Rules.FindByPredicate([Plan](const FSquadPlanRule& R) { return R.Plan == Plan; });
	}
};

// ---------------------------------------------------------------------------
// 대형 (PLAN.md 4.6절)
// ---------------------------------------------------------------------------

USTRUCT(BlueprintType)
struct FSquadFormationSlots
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Formation")
	EFormationType Formation = EFormationType::Wedge;

	/** 진행 방향 기준 로컬 오프셋(cm). X=전방, Y=우측. 인덱스 = 대형 내 순번 */
	UPROPERTY(EditAnywhere, Category = "Formation")
	TArray<FVector2D> LocalOffsets;
};

UCLASS(BlueprintType)
class SOLDIERLAB_API USquadFormationData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Formation")
	TArray<FSquadFormationSlots> Formations;
};

// ---------------------------------------------------------------------------
// 전체 튜닝
// ---------------------------------------------------------------------------

UCLASS(BlueprintType)
class SOLDIERLAB_API USquadTuningData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// --- 틱 스케줄 (PLAN.md 7.2·7.6절) ---

	/** 멤버 최고 티어가 T0 / T1 / T2 일 때의 분대 틱 주기 */
	UPROPERTY(EditAnywhere, Category = "Tick")
	float SquadTickHzT0 = 4.f;

	UPROPERTY(EditAnywhere, Category = "Tick")
	float SquadTickHzT1 = 3.f;

	UPROPERTY(EditAnywhere, Category = "Tick")
	float SquadTickHzT2 = 1.f;

	/** ★ 스파이크 방어선. 이게 없으면 힛치 복귀 프레임에서 전 분대가 한꺼번에 돈다 */
	UPROPERTY(EditAnywhere, Category = "Tick", meta = (ClampMin = "1"))
	int32 MaxSquadsPerFrame = 1;

	/** 페이즈 B(플랜 재평가) 주기. 이벤트로도 깨어나므로 느려도 굼뜨지 않다 */
	UPROPERTY(EditAnywhere, Category = "Tick")
	float PlanEvalIntervalSec = 2.f;

	// --- 사기 (설계 9.4절) ---

	UPROPERTY(EditAnywhere, Category = "Morale")
	float MoraleInitial = 0.8f;

	UPROPERTY(EditAnywhere, Category = "Morale")
	float W_Casualty = 0.18f;          // 사상자 1명당

	UPROPERTY(EditAnywhere, Category = "Morale")
	float W_Suppression = 0.25f;       // 분대 평균 피제압도

	UPROPERTY(EditAnywhere, Category = "Morale")
	float W_Outnumbered = 0.20f;       // 수적 열세

	UPROPERTY(EditAnywhere, Category = "Morale")
	float W_EnemyCasualty = 0.10f;     // 적 사상

	UPROPERTY(EditAnywhere, Category = "Morale")
	float W_LeaderAlive = 0.15f;       // 분대장 생존 보너스

	UPROPERTY(EditAnywhere, Category = "Morale")
	float W_SupportNearby = 0.10f;     // 아군 지원 근접

	/** EMA 평활화 계수(0~1). 낮을수록 사기가 천천히 움직인다 */
	UPROPERTY(EditAnywhere, Category = "Morale", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float MoraleSmoothing = 0.25f;

	/** 이 아래로 떨어지면 Withdraw 강제 (단 Constraints.MoraleFloor 가 더 우선) */
	UPROPERTY(EditAnywhere, Category = "Morale")
	float MoraleCollapseThreshold = 0.25f;

	// --- 플랜 (PLAN.md 3.4절) ---

	UPROPERTY(EditAnywhere, Category = "Plan")
	float PlanHysteresis = 1.15f;

	UPROPERTY(EditAnywhere, Category = "Plan")
	float PlanMinDurationSec = 3.f;

	// --- 토큰 (PLAN.md 6.3절) ---

	UPROPERTY(EditAnywhere, Category = "Token")
	float TokenMinHoldSec = 1.5f;

	/** ★ 최대 보유시간이 없으면 교대가 아예 안 난다 (설계 9.5절 "제압 토큰 순환") */
	UPROPERTY(EditAnywhere, Category = "Token")
	float SuppressionTokenMaxHoldSec = 6.f;

	UPROPERTY(EditAnywhere, Category = "Token")
	float MovementTokenMaxHoldSec = 8.f;

	/** ★ 없으면 특정 병사가 영구히 토큰을 못 받는다 */
	UPROPERTY(EditAnywhere, Category = "Token")
	float StarvationBonusPerSec = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Token")
	float RoleWeightAutomaticRiflemanSuppression = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Token")
	float RoleWeightMarksmanSuppression = 1.2f;

	// --- 바운딩 오버워치 (PLAN.md 4.1절) ---

	UPROPERTY(EditAnywhere, Category = "Bounding")
	float BoundLengthCm = 1500.f;      // [C-62]

	UPROPERTY(EditAnywhere, Category = "Bounding")
	float BoundArrivalRatio = 0.7f;

	UPROPERTY(EditAnywhere, Category = "Bounding")
	float BoundMaxDurationSec = 8.f;

	UPROPERTY(EditAnywhere, Category = "Bounding")
	float BoundMinDurationSec = 2.f;

	/** 교대 시 토큰 겹침. 0 이면 "아무도 안 쏘는 한 틱"이 생긴다 */
	UPROPERTY(EditAnywhere, Category = "Bounding")
	float BoundHandoverOverlapSec = 0.5f;

	// --- 측면기동 (PLAN.md 4.2절) ---

	UPROPERTY(EditAnywhere, Category = "Flank")
	float FlankOffsetCm = 2000.f;

	UPROPERTY(EditAnywhere, Category = "Flank")
	FVector FlankHalfExtentCm = FVector(1200.f, 1200.f, 400.f);

	// --- 재편성 (PLAN.md 4.5절) ---

	UPROPERTY(EditAnywhere, Category = "Regroup")
	float DispersionRegroupCm = 2500.f;

	UPROPERTY(EditAnywhere, Category = "Regroup")
	float DispersionReleaseCm = 1500.f;

	// --- 정보 공유 (PLAN.md 5절) — ★ 치팅 인상 튜닝의 전부가 여기 있다 ---

	UPROPERTY(EditAnywhere, Category = "Sharing")
	float ReportCooldownSec = 4.f;

	UPROPERTY(EditAnywhere, Category = "Sharing")
	float ReportMoveThresholdCm = 500.f;

	UPROPERTY(EditAnywhere, Category = "Sharing")
	float MaxReportsPerSecond = 2.f;

	UPROPERTY(EditAnywhere, Category = "Sharing")
	int32 MaxReportQueueDepth = 32;

	UPROPERTY(EditAnywhere, Category = "Sharing")
	float BaseRadioDelaySec = 0.8f;

	UPROPERTY(EditAnywhere, Category = "Sharing")
	float RadioJitterSec = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Sharing")
	float LeaderDelayBonusSec = -0.3f;

	UPROPERTY(EditAnywhere, Category = "Sharing")
	float BusyRecipientDelayPenaltySec = 0.4f;

	UPROPERTY(EditAnywhere, Category = "Sharing")
	FVector2D RadioDelayClampSec = FVector2D(0.2f, 3.0f);

	/** ★ 위치 불확실성 — 남이 알려준 적을 정조준하지 못하게 만드는 값 */
	UPROPERTY(EditAnywhere, Category = "Sharing")
	float BaseSigmaCm = 200.f;

	UPROPERTY(EditAnywhere, Category = "Sharing")
	float PerSecondSigmaCm = 100.f;

	UPROPERTY(EditAnywhere, Category = "Sharing")
	float DistanceSigmaCoef = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Sharing")
	float MaxSigmaCm = 1500.f;

	/** 설계 9.3절: 공유본 신뢰도 상한 */
	UPROPERTY(EditAnywhere, Category = "Sharing", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SharedConfidenceCap = 0.6f;

	UPROPERTY(EditAnywhere, Category = "Sharing")
	int32 MaxKnownThreats = 8;

	// --- 자원 중재 (PLAN.md 6.1~6.2절) ---

	/** 0 으로 죽이지 않는다 — 설계 10.5절 "완벽한 엄폐만 찾지 않는다"와 정합 */
	UPROPERTY(EditAnywhere, Category = "Arbitration", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ContestedSlotPenalty = 0.35f;

	UPROPERTY(EditAnywhere, Category = "Arbitration")
	float SlotSoftClaimLifetimeSec = 3.f;

	/** 동점 판정 폭 — 이 안이면 결정론적 tie-break 로 간다 */
	UPROPERTY(EditAnywhere, Category = "Arbitration")
	float SlotScoreTieEpsilon = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Arbitration")
	float TargetFocusDecay = 0.6f;

	UPROPERTY(EditAnywhere, Category = "Arbitration")
	float TargetFocusBonus = 2.f;

	/** 1위가 2위의 이 배를 넘을 때만 FocusTarget 을 지정한다 */
	UPROPERTY(EditAnywhere, Category = "Arbitration")
	float FocusTargetDominanceRatio = 1.4f;

	UPROPERTY(EditAnywhere, Category = "Arbitration")
	float FireLaneHalfWidthCm = 150.f;

	// --- 상태 보고 (PLAN.md 3.5절) ---

	UPROPERTY(EditAnywhere, Category = "Report")
	float OrderStatusReportIntervalSec = 2.f;

	UPROPERTY(EditAnywhere, Category = "Report")
	float ConfirmedThreatConfidence = 0.7f;
};

// ---------------------------------------------------------------------------
// 초기 편제 (DataTable)
// ---------------------------------------------------------------------------

USTRUCT(BlueprintType)
struct FSquadMemberDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Squad")
	ESquadRole Role = ESquadRole::Rifleman;

	UPROPERTY(EditAnywhere, Category = "Squad")
	int32 SuccessionOrder = 100;

	/** 레벨에 배치된 병사를 찾기 위한 태그. 저작 방식은 [D11]에서 확정 */
	UPROPERTY(EditAnywhere, Category = "Squad")
	FName SpawnTag = NAME_None;
};

/** 설계 3.7.1절 / CLAUDE.md P4: 진영은 데이터다. 진영별 파생 클래스를 만들지 않는다. */
USTRUCT(BlueprintType)
struct FSquadDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Squad")
	FName SquadId = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Squad")
	FGameplayTag Faction;

	UPROPERTY(EditAnywhere, Category = "Squad")
	TArray<FSquadMemberDefinition> Members;

	UPROPERTY(EditAnywhere, Category = "Squad")
	TObjectPtr<USquadTuningData> Tuning = nullptr;

	UPROPERTY(EditAnywhere, Category = "Squad")
	TObjectPtr<USquadPlanRuleSet> PlanRules = nullptr;
};
