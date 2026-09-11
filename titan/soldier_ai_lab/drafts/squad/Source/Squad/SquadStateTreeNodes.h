// SoldierLab — L1 ↔ L3 접점: StateTree 노드
// 초안 (2026-09-09). ★ 컴파일 검증하지 않았다.
//
// ⚠ [C-63] 베이스 클래스명·헤더 경로·스키마 허용 규칙을 UE5.8 엔진 소스에서 반드시 대조할 것.
//   이 초안은 UE5.3~5.5 계열 API 를 가정했다:
//     FStateTreeEvaluatorCommonBase / FStateTreeTaskCommonBase / FStateTreeConditionCommonBase
//     + using FInstanceDataType = ... + GetInstanceDataType() override
//   또한 이 노드들이 UStateTreeAIComponentSchema 에서 보이려면 스키마의 IsStructAllowed 를
//   통과해야 한다 → [R6].
//
// 근거: ai/2026-09-02_upper_layer_plan.md 3.2절(GASP 태스크 라이브러리) ·
//       4.2절(태스크 어휘 — STT_AcquireToken/STT_ReleaseToken 이 이미 예고돼 있다) ·
//       14.3·14.5절(★ 자원은 그것을 보유하는 상태가 부모여야 한다)
//       PLAN.md 8절

#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "StateTreeTaskBase.h"
#include "StateTreeConditionBase.h"
#include "StateTreeExecutionContext.h"
#include "SquadTypes.h"
#include "SquadStateTreeNodes.generated.h"

class AAIController;
class USquadComponent;

// ===========================================================================
// Evaluator — 분대 배정을 StateTree 에 노출한다
// ===========================================================================

USTRUCT()
struct FSTE_SquadContextInstanceData
{
	GENERATED_BODY()

	/** GASP AI StateTree 와 동일하게 AIController 를 컨텍스트로 받는다 */
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AAIController> AIController = nullptr;

	UPROPERTY(EditAnywhere, Category = "Output")
	FSquadAssignment Assignment;

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bHasSquad = false;

	/** 캐시된 리비전. 같으면 복사를 건너뛴다 (배정은 3Hz 로만 바뀐다) */
	UPROPERTY()
	uint32 CachedRevision = 0;
};

/**
 * 상위계획 R4 — GASP `STE_GetAIData` 패턴의 승계.
 *
 * ★ 분대 룩업은 이 Evaluator 하나로 통일한다 (PLAN.md 8.2절).
 *   태스크마다 각자 분대를 찾으면 룩업이 태스크 수만큼 늘고,
 *   같은 틱 안에서 배정이 달라 보이는 사고가 난다.
 */
USTRUCT(meta = (DisplayName = "Squad Context", Category = "SoldierLab|Squad"))
struct SOLDIERLAB_API FSTE_SquadContext : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTE_SquadContextInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};

// ===========================================================================
// Task — 토큰 획득/반납
// ===========================================================================

USTRUCT()
struct FSTT_AcquireSquadTokenInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AAIController> AIController = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	ESquadTokenType TokenType = ESquadTokenType::Movement;

	/**
	 * true  : 획득 실패 시 즉시 Failed 를 반환한다 (형제 상태로 전이 → 자동 엄호)
	 * false : Running 을 유지하며 계속 대기한다 (starvation 보너스가 쌓인다)
	 */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bFailIfUnavailable = true;

	UPROPERTY()
	int32 MemberIndex = INDEX_NONE;

	UPROPERTY()
	bool bHolding = false;
};

/**
 * 상위계획 4.2절 `STT_AcquireToken` 의 구현.
 *
 * ★★ 이 태스크는 반드시 **자원을 쓰는 상태의 부모 상태**에 둔다 (상위계획 14.3·14.5절).
 *
 *     AcquireMovementToken       ← 이 태스크. 부모가 토큰을 보유
 *       └ MoveToBoundLine        ← 자식이 실제 일을 한다
 *
 *     이렇게 두면 사망·중단·상위 인터럽트 어느 경로로 상태를 벗어나도
 *     ExitState 가 반드시 불려 토큰이 반납된다. GASP 가 SmartObject 예약 누수를
 *     구조로 해결한 것과 정확히 같은 장치를 토큰에 재사용하는 것이다.
 *
 *     태스크를 한 상태에 나열하는 방식으로 짜면 이 성질이 사라진다.
 */
USTRUCT(meta = (DisplayName = "Acquire Squad Token", Category = "SoldierLab|Squad"))
struct SOLDIERLAB_API FSTT_AcquireSquadToken : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTT_AcquireSquadTokenInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

	virtual void ExitState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context,
		const float DeltaTime) const override;
};

// ===========================================================================
// Task — 엄폐 슬롯 사전 예고 (PLAN.md 6.1절)
// ===========================================================================

USTRUCT()
struct FSTT_ReportSlotClaimInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AAIController> AIController = nullptr;

	/** EQS 결과 1위 슬롯 */
	UPROPERTY(EditAnywhere, Category = "Input")
	FGuid SlotId;

	UPROPERTY(EditAnywhere, Category = "Input")
	float Score = 0.f;

	UPROPERTY()
	int32 MemberIndex = INDEX_NONE;
};

/**
 * soft-claim 등록/해제.
 * ★ 최적화이지 정확성 장치가 아니다 — 중복 점유를 막는 것은 SmartObject 예약이다.
 *   이 태스크도 부모 상태(QueryTacticalSlot)에 두어 ExitState 로 자동 해제시킨다.
 */
USTRUCT(meta = (DisplayName = "Report Cover Slot Claim", Category = "SoldierLab|Squad"))
struct SOLDIERLAB_API FSTT_ReportSlotClaim : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTT_ReportSlotClaimInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

	virtual void ExitState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};

// ===========================================================================
// Conditions
// ===========================================================================

USTRUCT()
struct FSTC_SquadTaskIsInstanceData
{
	GENERATED_BODY()

	/** Evaluator 의 Output 에 바인딩한다 */
	UPROPERTY(EditAnywhere, Category = "Input")
	FSquadAssignment Assignment;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	ESquadTask RequiredTask = ESquadTask::None;
};

USTRUCT(meta = (DisplayName = "Squad Task Is", Category = "SoldierLab|Squad"))
struct SOLDIERLAB_API FSTC_SquadTaskIs : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTC_SquadTaskIsInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};

USTRUCT()
struct FSTC_SquadHasTokenInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input")
	FSquadAssignment Assignment;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	ESquadTokenType TokenType = ESquadTokenType::Movement;
};

USTRUCT(meta = (DisplayName = "Squad Has Token", Category = "SoldierLab|Squad"))
struct SOLDIERLAB_API FSTC_SquadHasToken : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTC_SquadHasTokenInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};

USTRUCT()
struct FSTC_IsInsideCorridorInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AAIController> AIController = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input")
	FSquadAssignment Assignment;

	/** 회랑 경계에서 이만큼 안쪽까지만 유효로 본다(경계에서 깜빡이지 않게) */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float MarginCm = 100.f;
};

/** 회랑이 비어 있으면(IsValid==false) 항상 true — "제약 없음"이 기본값이다. */
USTRUCT(meta = (DisplayName = "Is Inside Squad Corridor", Category = "SoldierLab|Squad"))
struct SOLDIERLAB_API FSTC_IsInsideCorridor : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTC_IsInsideCorridorInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};
