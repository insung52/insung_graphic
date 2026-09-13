// SoldierLab — L1 SQUAD 본체
// 초안 (2026-09-09). 컴파일 검증 없음.
//
// 근거: design/2026-09-01_architecture.md 9절 · 4.3절(권위)
//       ai/2026-09-02_upper_layer_plan.md 6절
//       PLAN.md 2·3·4·5·6·7절
//
// 배치 예정: Source/SoldierLab/Squad/SquadComponent.h
//
// ★ 서버 전용 (설계 4.3.2절). 모든 진입점에 HasAuthority() 게이트 (CLAUDE.md P5).
// ★ 분대 틱이 절대 하지 않는 것 — PLAN.md 7.3절:
//     트레이스 / EQS 발행 / 경로 재계산(플랜 전환당 비동기 1회 예외) /
//     멤버별 LoS / 개인 기억 순회 / 매 틱 조 재편성
//   허용되는 것은 사실상 넷뿐이다:
//     캐시된 스칼라 읽기, O(N) 벡터 합, O(8) 위협 정렬, 큐 pop.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Info.h"
#include "SquadTypes.h"
#include "SquadTuningData.h"
#include "../Command/SoldierOrder.h"
#include "SquadComponent.generated.h"

class UNavigationSystemV1;

/**
 * 분대 호스트 액터.
 *
 * PLAN.md 2.1절 — 왜 분대장 폰에 붙이지 않는가:
 *   분대장이 죽으면 분대 상태(토큰·blackboard·플랜)가 함께 파괴된다. 설계 9.5절이 요구하는
 *   "차순위 승계"를 하려면 상태를 새 액터로 이전해야 하고, 이전 중 한 틱이라도 비면 전원이
 *   토큰을 잃는다. 액터 1개/분대 × 최대 11개는 무시할 비용이다.
 */
UCLASS()
class SOLDIERLAB_API ASquadCoordinator : public AInfo
{
	GENERATED_BODY()

public:
	ASquadCoordinator();

	UPROPERTY(VisibleAnywhere, Category = "Squad")
	TObjectPtr<class USquadComponent> SquadComponent;
};

/**
 * L1 SQUAD — 설계 3.7절이 지정한 이름 그대로.
 *
 * 하는 일 (설계 9.1절 하이브리드): 분대는 ①역할/배역 ②회랑 ③토큰만 배분하고,
 * 개인은 그 제약 안에서 유틸리티로 자유 판단한다.
 * 분대는 "누구를 언제 쏴라"를 절대 말하지 않는다 (설계 2.2절 제2규칙).
 */
UCLASS(ClassGroup = (SoldierLab), meta = (BlueprintSpawnableComponent))
class SOLDIERLAB_API USquadComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USquadComponent();

	// -----------------------------------------------------------------------
	// 편제
	// -----------------------------------------------------------------------

	void InitializeSquad(const FSquadDefinition& Definition);

	/** 반환값이 이 병사의 MemberIndex. 실패 시 INDEX_NONE */
	int32 RegisterMember(AActor* Pawn, ESquadRole Role, int32 SuccessionOrder);

	/** 배열에서 제거하지 않는다 — Dead 표시만. PLAN.md 2.1절(인덱스 안정성) */
	void NotifyMemberDied(int32 MemberIndex);

	UFUNCTION(BlueprintPure, Category = "SoldierLab|Squad")
	FName GetSquadId() const { return SquadId; }

	UFUNCTION(BlueprintPure, Category = "SoldierLab|Squad")
	int32 GetAliveCount() const;

	/** 분대장 사망 시 SuccessionOrder 가 가장 낮은 생존자로 승계 (설계 9.5절) */
	void PromoteNewLeader();

	// -----------------------------------------------------------------------
	// 계층 계약
	// -----------------------------------------------------------------------

	/** L0 → L1. 우선순위 비교 후 채택. HoldFire 는 ROE 만 갱신한다 (PLAN.md 3.3절) */
	void ReceiveOrder(const FSoldierOrder& Order);

	/** L1 → L2. StateTree Evaluator 와 개인 유틸리티가 읽는 유일한 창구 */
	UFUNCTION(BlueprintPure, Category = "SoldierLab|Squad")
	const FSquadAssignment& GetAssignment(int32 MemberIndex) const;

	/** L1 → L0 (설계 11.5절) */
	void BuildStatusReport(FOrderStatusReport& OutReport) const;

	// -----------------------------------------------------------------------
	// 정보 공유 (PLAN.md 5절)
	// -----------------------------------------------------------------------

	/**
	 * 개인이 Awareness == Confirmed 에 도달했을 때 호출 (설계 9.3절).
	 * L1은 발신 게이트 통과 여부만 판정하고 지연 큐에 넣는다. 즉시 배달하지 않는다.
	 * @return 보고가 접수되었으면 true (쿨다운/상한에 걸리면 false)
	 */
	bool SubmitContactReport(int32 ReporterMemberIndex, AActor* Target,
		const FVector& ObservedLocation, const FVector& ObservedVelocity,
		float ThreatLevel, float ObserverConfidence);

	/** 분대가 아는 위협 목록 (상한 8). 개인이 폴링한다 */
	const TArray<FSharedThreat>& GetKnownThreats() const { return Blackboard.KnownThreats; }

	// -----------------------------------------------------------------------
	// 자원 중재 (PLAN.md 6절)
	// -----------------------------------------------------------------------

	/** 엄폐 슬롯 사전 예고. L1은 보관만 한다 — 점수 계산도 EQS 발행도 하지 않는다 */
	void RegisterSlotClaim(int32 MemberIndex, const FGuid& SlotId, float Score);
	void ReleaseSlotClaim(int32 MemberIndex);

	/**
	 * 엄폐 담당자의 EQS 테스트가 호출하는 중재 훅.
	 * @return 이 슬롯에 곱할 배수. 경쟁이 없으면 1.0, 있으면 ContestedSlotPenalty.
	 *         ★ 0을 반환하지 않는다 — 남은 슬롯이 그것뿐일 때 아무도 못 가는 사태를 막는다
	 */
	UFUNCTION(BlueprintPure, Category = "SoldierLab|Squad")
	float GetSlotContentionMultiplier(int32 RequestingMemberIndex, const FGuid& SlotId) const;

	/**
	 * 표적 선택 배수 (PLAN.md 6.2절).
	 * 기본은 분산(과집중 억제), FocusTarget 이 지정되면 그쪽에 보너스.
	 */
	UFUNCTION(BlueprintPure, Category = "SoldierLab|Squad")
	float GetTargetPreferenceMultiplier(AActor* Target) const;

	/**
	 * 토큰 요청. 실패하면 요청 큐에 등록되어 다음 분대 틱에서 starvation 보너스를 받는다.
	 * ★ StateTree 에서는 자원을 보유하는 상태가 부모여야 한다 (상위계획 14.3절) —
	 *   FSTT_AcquireSquadToken 의 EnterState 가 이것을, ExitState 가 Release 를 호출한다.
	 */
	bool RequestToken(int32 MemberIndex, ESquadTokenType Type);
	void ReleaseToken(int32 MemberIndex, ESquadTokenType Type);

	UFUNCTION(BlueprintPure, Category = "SoldierLab|Squad")
	bool HasToken(int32 MemberIndex, ESquadTokenType Type) const;

	/** 제압 중인 사격선 게시/해제 (PLAN.md 6.4절). 금지선이 아니라 경로 비용이다 */
	void PublishFireLane(int32 MemberIndex, const FVector& Origin, const FVector& Direction);
	void ClearFireLane(int32 MemberIndex);
	const TArray<FSquadFireLane>& GetActiveFireLanes() const { return Blackboard.ActiveFireLanes; }

	// -----------------------------------------------------------------------
	// 틱 — USquadSubsystem 만 호출한다 (PLAN.md 7.2절 라운드로빈)
	// -----------------------------------------------------------------------

	/** 페이즈 A + (주기 도달 시) B + (플랜 전환 시) C */
	void TickSquad(float NowSeconds);

	/** 현재 LOD에 따른 희망 틱 주파수 (PLAN.md 7.6절) */
	float GetDesiredTickHz() const;

	/** 페이즈 B 강제 실행 요청 — 명령 변경·사상자·접촉 변화·분대장 사망 */
	void RequestPlanReevaluation() { bPlanEvalPending = true; }

	// -----------------------------------------------------------------------
	// 디버그 (상위계획 10절 — 1급 시민. CLAUDE.md P7)
	// -----------------------------------------------------------------------

	const FSquadBlackboard& GetBlackboard() const { return Blackboard; }
	const TArray<FSquadMemberEntry>& GetMembers() const { return Members; }

	/** 마지막 플랜 스코어링 결과 — "왜 이 플랜이 이겼는가" (설계 2.3절 제3규칙) */
	const TMap<ESquadPlan, float>& GetLastPlanScores() const { return LastPlanScores; }

protected:
	// --- 페이즈 A: 수집·유지 (매 분대 틱) ---
	void Phase_CollectMemberStatus(float NowSeconds);
	void Phase_UpdateSpatialSummary();
	void Phase_UpdateMorale(float DeltaSeconds);
	void Phase_DeliverPendingReports(float NowSeconds);
	void Phase_UpdateTokens(float NowSeconds);
	void Phase_UpdateTargetArbitration();

	// --- 페이즈 B: 플랜 재평가 (0.5Hz 또는 이벤트) ---
	void Phase_SelectPlan(float NowSeconds);
	void Phase_FormElements();
	void Phase_AssignTasks();

	// --- 페이즈 C: 공간 준비 (플랜 전환 시 1회) ---
	void Phase_PrepareSpatial();
	bool BuildFlankCorridor(FBox& OutCorridor) const;
	FVector ComputeBoundLine(const FSquadElement& MovingElement) const;
	FVector ComputeRallyPoint() const;

	// --- 보조 ---
	float ScorePlan(const FSquadPlanRule& Rule, float NowSeconds) const;
	bool IsInContact() const;
	void PushAssignmentsRevision();
	void EnqueueContactDeliveries(const FSharedThreat& Report, int32 ReporterIndex, float NowSeconds);
	void MergeIntoKnownThreats(const FSharedThreat& Report);
	int32 FindMemberIndex(const AActor* Pawn) const;

	const USquadTuningData* GetTuning() const;

private:
	UPROPERTY()
	FName SquadId = NAME_None;

	UPROPERTY()
	FGameplayTag Faction;

	UPROPERTY()
	TArray<FSquadMemberEntry> Members;

	UPROPERTY()
	TArray<FSquadAssignment> Assignments;   // Members 와 인덱스 일치

	UPROPERTY()
	FSquadBlackboard Blackboard;

	UPROPERTY()
	FSoldierOrder CurrentOrder;

	UPROPERTY()
	TObjectPtr<const USquadTuningData> Tuning = nullptr;

	UPROPERTY()
	TObjectPtr<const USquadPlanRuleSet> PlanRules = nullptr;

	// --- 타이밍 ---
	float LastSquadTickSeconds = -1.f;
	float LastPlanEvalSeconds = -1.f;
	float PlanEnteredAtSeconds = -1.f;
	float LastBoundStartSeconds = -1.f;
	float LastStatusReportSeconds = -1.f;
	float ReportBudgetWindowStart = -1.f;
	int32 ReportsThisWindow = 0;

	bool bPlanEvalPending = true;
	bool bSpatialPrepPending = true;

	uint32 AssignmentRevision = 0;

	/** 조 단위 이동 토큰의 현재 보유 조 (PLAN.md 4.1절) */
	FName MovementTokenElementId = NAME_None;

	/** 사상자 카운트 — 사기 공식과 상태 보고에 쓴다 */
	int32 CasualtyCount = 0;
	int32 EnemyCasualtyCount = 0;
	int32 InitialConfirmedEnemyCount = 0;

	/** 디버그: 마지막 플랜 스코어 (설계 2.3절 제3규칙 — 왜 그랬는지 답할 수 있어야 한다) */
	TMap<ESquadPlan, float> LastPlanScores;

	static const FSquadAssignment InvalidAssignment;
};
