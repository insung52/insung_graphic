// SoldierLab — L1 SQUAD 본체 구현
// 초안 (2026-09-09). ★ 컴파일 검증하지 않았다. 엔진 API 이름은 UE5.8에서 대조할 것.
//
// 이 파일은 "예산 안에 드는 단순한 안"을 우선한다 (사용자 지시).
// 우아하지만 무거운 대안은 주석으로 남기되 구현하지 않았다.

#include "SquadComponent.h"

#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "NavigationSystem.h"

const FSquadAssignment USquadComponent::InvalidAssignment = FSquadAssignment();

// ===========================================================================
// ASquadCoordinator
// ===========================================================================

ASquadCoordinator::ASquadCoordinator()
{
	PrimaryActorTick.bCanEverTick = false;   // 틱은 USquadSubsystem 이 라운드로빈으로 돌린다

	// 설계 4.3.2절: L1은 서버 전용, 복제하지 않는다(파생 상태)
	bReplicates = false;
	SetHidden(true);
	SetCanBeDamaged(false);

	SquadComponent = CreateDefaultSubobject<USquadComponent>(TEXT("SquadComponent"));
}

// ===========================================================================
// USquadComponent — 편제
// ===========================================================================

USquadComponent::USquadComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USquadComponent::InitializeSquad(const FSquadDefinition& Definition)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())   // CLAUDE.md P5
	{
		return;
	}

	SquadId = Definition.SquadId;
	Faction = Definition.Faction;
	Tuning = Definition.Tuning;
	PlanRules = Definition.PlanRules;

	Members.Reset();
	Assignments.Reset();
	Members.Reserve(Definition.Members.Num());
	Assignments.Reserve(Definition.Members.Num());

	Blackboard = FSquadBlackboard();
	Blackboard.Morale = GetTuning() ? GetTuning()->MoraleInitial : 0.8f;

	// 조는 항상 2개다 (PLAN.md 2.3절). 3개 이상이 필요해지면 [D13].
	Blackboard.Elements.SetNum(2);
	Blackboard.Elements[0].ElementId = TEXT("Alpha");
	Blackboard.Elements[1].ElementId = TEXT("Bravo");

	bPlanEvalPending = true;
	bSpatialPrepPending = true;
}

int32 USquadComponent::RegisterMember(AActor* Pawn, ESquadRole Role, int32 SuccessionOrder)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !Pawn)
	{
		return INDEX_NONE;
	}
	if (Members.Num() >= SOLDIERLAB_MAX_SQUAD_MEMBERS)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Squad %s] 정원 초과 — %s 등록 실패"),
			*SquadId.ToString(), *Pawn->GetName());
		return INDEX_NONE;
	}

	FSquadMemberEntry& Entry = Members.AddDefaulted_GetRef();
	Entry.Pawn = Pawn;
	Entry.Role = Role;
	Entry.SuccessionOrder = SuccessionOrder;

	FSquadAssignment& Assignment = Assignments.AddDefaulted_GetRef();
	Assignment.SquadId = SquadId;
	Assignment.MemberIndex = Members.Num() - 1;
	Assignment.Role = Role;

	bPlanEvalPending = true;
	return Assignment.MemberIndex;
}

void USquadComponent::NotifyMemberDied(int32 MemberIndex)
{
	if (!Members.IsValidIndex(MemberIndex))
	{
		return;
	}

	// ★ 배열에서 제거하지 않는다 (PLAN.md 2.1절). 제거하면 인덱스가 밀려
	//   개인이 캐시한 FSquadHandle 이 다른 사람을 가리킨다.
	Members[MemberIndex].Status.Health = ESquadMemberHealth::Dead;
	++CasualtyCount;

	// 자원 회수 — 죽은 자가 토큰·슬롯·사격선을 붙잡고 있으면 분대가 마비된다.
	ReleaseToken(MemberIndex, ESquadTokenType::Movement);
	ReleaseToken(MemberIndex, ESquadTokenType::Suppression);
	ReleaseSlotClaim(MemberIndex);
	ClearFireLane(MemberIndex);

	if (Members[MemberIndex].Role == ESquadRole::Leader)
	{
		PromoteNewLeader();
	}

	// 사상자는 페이즈 B 를 깨우는 이벤트다 (PLAN.md 7.4절)
	bPlanEvalPending = true;
}

int32 USquadComponent::GetAliveCount() const
{
	int32 Count = 0;
	for (const FSquadMemberEntry& M : Members)
	{
		if (M.IsAlive())
		{
			++Count;
		}
	}
	return Count;
}

void USquadComponent::PromoteNewLeader()
{
	// 설계 9.5절: 사망 시 차순위 승계.
	int32 BestIndex = INDEX_NONE;
	int32 BestOrder = MAX_int32;
	for (int32 i = 0; i < Members.Num(); ++i)
	{
		if (Members[i].IsAlive() && Members[i].Role != ESquadRole::Leader
			&& Members[i].SuccessionOrder < BestOrder)
		{
			BestOrder = Members[i].SuccessionOrder;
			BestIndex = i;
		}
	}
	if (BestIndex != INDEX_NONE)
	{
		Members[BestIndex].Role = ESquadRole::Leader;
		Assignments[BestIndex].Role = ESquadRole::Leader;
		// [D12] 승계 시 사기 페널티를 줄 것인가 — 미결정. 지금은 주지 않는다.
	}
	bPlanEvalPending = true;
}

int32 USquadComponent::FindMemberIndex(const AActor* Pawn) const
{
	for (int32 i = 0; i < Members.Num(); ++i)
	{
		if (Members[i].Pawn.Get() == Pawn)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

const USquadTuningData* USquadComponent::GetTuning() const
{
	return Tuning.Get();
}

const FSquadAssignment& USquadComponent::GetAssignment(int32 MemberIndex) const
{
	return Assignments.IsValidIndex(MemberIndex) ? Assignments[MemberIndex] : InvalidAssignment;
}

// ===========================================================================
// 명령 수신 (PLAN.md 3.3절)
// ===========================================================================

void USquadComponent::ReceiveOrder(const FSoldierOrder& Order)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !Order.IsValidOrder())
	{
		return;
	}

	// ★ HoldFire 는 교전규칙만 바꾼다. 플랜을 건드리지 않는다 (PLAN.md 3.3절).
	//   이걸 플랜으로 취급하면 "사격 금지" 한 마디에 분대가 진형을 푼다.
	if (Order.Verb == EOrderVerb::HoldFire)
	{
		CurrentOrder.Constraints.ROE = EEngagementRule::HoldFire;
		PushAssignmentsRevision();
		return;
	}

	// 우선순위가 낮으면 무시 (설계 11.2절 Priority 주석)
	if (CurrentOrder.IsValidOrder() && Order.Priority < CurrentOrder.Priority)
	{
		return;
	}

	CurrentOrder = Order;
	bPlanEvalPending = true;   // 명령 변경은 페이즈 B 를 깨우는 이벤트
}

// ===========================================================================
// 틱
// ===========================================================================

float USquadComponent::GetDesiredTickHz() const
{
	const USquadTuningData* T = GetTuning();
	if (!T)
	{
		return 3.f;
	}
	if (GetAliveCount() == 0)
	{
		return 0.f;
	}

	// TODO: SignificanceManager 로 멤버 최고 티어를 얻는다 (설계 12.3절).
	//       지금은 T1 고정. LOD 연동은 S8(성능 단계)에서.
	return T->SquadTickHzT1;
}

void USquadComponent::TickSquad(float NowSeconds)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())   // CLAUDE.md P5
	{
		return;
	}

	const float DeltaSeconds = (LastSquadTickSeconds < 0.f) ? 0.f : (NowSeconds - LastSquadTickSeconds);
	LastSquadTickSeconds = NowSeconds;

	// --- 페이즈 A : 매 분대 틱 ---
	Phase_CollectMemberStatus(NowSeconds);
	Phase_UpdateSpatialSummary();
	Phase_UpdateMorale(DeltaSeconds);
	Phase_DeliverPendingReports(NowSeconds);
	Phase_UpdateTokens(NowSeconds);
	Phase_UpdateTargetArbitration();

	// --- 페이즈 B : 0.5Hz 또는 이벤트 ---
	const USquadTuningData* T = GetTuning();
	const float PlanInterval = T ? T->PlanEvalIntervalSec : 2.f;
	if (bPlanEvalPending || LastPlanEvalSeconds < 0.f || (NowSeconds - LastPlanEvalSeconds) >= PlanInterval)
	{
		bPlanEvalPending = false;
		LastPlanEvalSeconds = NowSeconds;

		const ESquadPlan PreviousPlan = Blackboard.CurrentPlan;
		Phase_SelectPlan(NowSeconds);

		if (Blackboard.CurrentPlan != PreviousPlan)
		{
			PlanEnteredAtSeconds = NowSeconds;
			bSpatialPrepPending = true;
			Phase_FormElements();       // 조 편성은 플랜 전환 시에만 (PLAN.md 2.3절)
		}
		Phase_AssignTasks();
	}

	// --- 페이즈 C : 플랜 전환 시 1회 ---
	if (bSpatialPrepPending)
	{
		bSpatialPrepPending = false;
		Phase_PrepareSpatial();
	}
}

// ===========================================================================
// 페이즈 A
// ===========================================================================

void USquadComponent::Phase_CollectMemberStatus(float NowSeconds)
{
	// ★ pull 이다 (PLAN.md 7.3절). 구현 측 getter 는 캐시된 값을 반환만 해야 한다 —
	//   여기서 트레이스가 돌면 분대 예산이 그 자리에서 터진다.
	for (FSquadMemberEntry& M : Members)
	{
		AActor* Pawn = M.Pawn.Get();
		if (!Pawn)
		{
			M.Status.Health = ESquadMemberHealth::Dead;
			continue;
		}
		if (ISquadMemberInterface* Iface = Cast<ISquadMemberInterface>(Pawn))
		{
			Iface->GetSquadStatus(M.Status);
			M.StatusUpdatedAtSeconds = NowSeconds;
		}
	}
}

void USquadComponent::Phase_UpdateSpatialSummary()
{
	// O(N) 벡터 합만. 허용된 4가지 연산 중 하나 (PLAN.md 7.3절).
	FVector Sum = FVector::ZeroVector;
	int32 Count = 0;
	for (const FSquadMemberEntry& M : Members)
	{
		if (M.IsAlive())
		{
			Sum += M.Status.Location;
			++Count;
		}
	}
	if (Count == 0)
	{
		Blackboard.Dispersion = 0.f;
		return;
	}

	Blackboard.FriendlyCentroid = Sum / static_cast<float>(Count);

	float SumSq = 0.f;
	for (const FSquadMemberEntry& M : Members)
	{
		if (M.IsAlive())
		{
			SumSq += FVector::DistSquared(M.Status.Location, Blackboard.FriendlyCentroid);
		}
	}
	Blackboard.Dispersion = FMath::Sqrt(SumSq / static_cast<float>(Count));

	// 적 centroid 는 공유 위협 목록(최대 8개)으로만 낸다 — 인지를 다시 돌리지 않는다.
	FVector EnemySum = FVector::ZeroVector;
	int32 EnemyCount = 0;
	for (const FSharedThreat& Threat : Blackboard.KnownThreats)
	{
		if (Threat.Target.IsValid())
		{
			EnemySum += Threat.ReportedLocation;
			++EnemyCount;
		}
	}
	if (EnemyCount > 0)
	{
		Blackboard.EnemyCentroid = EnemySum / static_cast<float>(EnemyCount);
	}
}

void USquadComponent::Phase_UpdateMorale(float DeltaSeconds)
{
	const USquadTuningData* T = GetTuning();
	if (!T)
	{
		return;
	}

	const int32 Alive = GetAliveCount();
	const int32 Total = FMath::Max(1, Members.Num());

	float AvgSuppression = 0.f;
	bool bLeaderAlive = false;
	for (const FSquadMemberEntry& M : Members)
	{
		if (M.IsAlive())
		{
			AvgSuppression += M.Status.SuppressionLevel;
			bLeaderAlive |= (M.Role == ESquadRole::Leader);
		}
	}
	AvgSuppression /= static_cast<float>(FMath::Max(1, Alive));

	// 수적 열세: 확인된 적 수 vs 생존자
	int32 ConfirmedEnemies = 0;
	for (const FSharedThreat& Threat : Blackboard.KnownThreats)
	{
		if (Threat.Confidence >= T->ConfirmedThreatConfidence)
		{
			++ConfirmedEnemies;
		}
	}
	const float Outnumbered = (Alive > 0)
		? FMath::Clamp((static_cast<float>(ConfirmedEnemies) - Alive) / static_cast<float>(Alive), 0.f, 1.f)
		: 1.f;

	// 설계 9.4절 공식
	float Target = T->MoraleInitial
		+ CurrentOrder.Constraints.Aggression * 0.2f                     // 명령의 공세성
		- T->W_Casualty * static_cast<float>(CasualtyCount)
		- T->W_Suppression * AvgSuppression
		- T->W_Outnumbered * Outnumbered
		+ T->W_EnemyCasualty * static_cast<float>(EnemyCasualtyCount)
		+ (bLeaderAlive ? T->W_LeaderAlive : 0.f);
	// + T->W_SupportNearby * ...   ← UGV/화력 지원. SoldierLab 엔 UGV 가 없다 [D8]

	Target = FMath::Clamp(Target, 0.f, 1.f);

	// EMA 평활화 — 없으면 사기가 튀고 플랜이 요동친다 (PLAN.md 3.4절 [C-61])
	const float Alpha = FMath::Clamp(T->MoraleSmoothing, 0.01f, 1.f);
	Blackboard.Morale = FMath::Lerp(Blackboard.Morale, Target, Alpha);

	// 설계 11.2절 MoraleFloor — "결사 항전"의 파라미터화 (설계 9.4절)
	Blackboard.Morale = FMath::Max(Blackboard.Morale, CurrentOrder.Constraints.MoraleFloor);

	(void)DeltaSeconds;   // 주기가 고정이 아니면 Alpha 를 dt 보정할 것 [B]
	(void)Total;
}

// ===========================================================================
// 페이즈 A — 정보 공유 (PLAN.md 5절)
// ===========================================================================

bool USquadComponent::SubmitContactReport(int32 ReporterMemberIndex, AActor* Target,
	const FVector& ObservedLocation, const FVector& ObservedVelocity,
	float ThreatLevel, float ObserverConfidence)
{
	const USquadTuningData* T = GetTuning();
	if (!T || !Target || !Members.IsValidIndex(ReporterMemberIndex))
	{
		return false;
	}

	FSquadMemberEntry& Reporter = Members[ReporterMemberIndex];
	if (!Reporter.IsAlive())
	{
		return false;   // 죽은 자는 무전을 못 친다
	}

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	// --- [1] 발신 게이트 ---

	// 분대 전체 무전 상한 (초당). 초과분은 폐기 = "무전이 겹쳤다"
	if (ReportBudgetWindowStart < 0.f || (Now - ReportBudgetWindowStart) >= 1.f)
	{
		ReportBudgetWindowStart = Now;
		ReportsThisWindow = 0;
	}
	if (static_cast<float>(ReportsThisWindow) >= T->MaxReportsPerSecond)
	{
		return false;
	}

	// 동일 표적 재보고 쿨다운. 단 적이 크게 움직였으면 무시하고 갱신 보고를 낸다.
	if (const float* LastTime = Reporter.LastReportTimeByTarget.Find(Target))
	{
		const bool bCooling = (Now - *LastTime) < T->ReportCooldownSec;
		bool bMovedFar = false;
		for (const FSharedThreat& Known : Blackboard.KnownThreats)
		{
			if (Known.Target.Get() == Target)
			{
				bMovedFar = FVector::Dist(Known.ReportedLocation, ObservedLocation) > T->ReportMoveThresholdCm;
				break;
			}
		}
		if (bCooling && !bMovedFar)
		{
			return false;
		}
	}
	Reporter.LastReportTimeByTarget.Add(Target, Now);
	++ReportsThisWindow;

	// --- 보고 본문 ---
	FSharedThreat Report;
	Report.Target = Target;
	Report.ReportedLocation = ObservedLocation;
	Report.ReportedVelocity = ObservedVelocity;
	Report.ObservedAtSeconds = Now;
	Report.ThreatLevel = ThreatLevel;
	Report.Confidence = FMath::Min(ObserverConfidence, T->SharedConfidenceCap);   // 설계 9.3절 상한
	Report.ReporterMemberIndex = ReporterMemberIndex;
	Report.bReporterIsLeader = (Reporter.Role == ESquadRole::Leader);
	Report.LocationSigmaCm = 0.f;   // 수신자별로 배달 시점에 계산한다

	// 분대 blackboard 는 즉시 갱신한다(분대의 "장부"). 개인 기억은 지연 배달로만 바뀐다.
	MergeIntoKnownThreats(Report);
	EnqueueContactDeliveries(Report, ReporterMemberIndex, Now);
	return true;
}

void USquadComponent::MergeIntoKnownThreats(const FSharedThreat& Report)
{
	const USquadTuningData* T = GetTuning();
	const int32 MaxThreats = T ? T->MaxKnownThreats : 8;

	for (FSharedThreat& Known : Blackboard.KnownThreats)
	{
		if (Known.Target.Get() == Report.Target.Get())
		{
			if (Report.ObservedAtSeconds >= Known.ObservedAtSeconds)
			{
				Known = Report;
			}
			return;
		}
	}

	if (Blackboard.KnownThreats.Num() < MaxThreats)
	{
		Blackboard.KnownThreats.Add(Report);
		return;
	}

	// 상한 초과 — ThreatLevel × Confidence 최하위를 축출 (PLAN.md 2.4절)
	int32 WorstIndex = 0;
	float WorstScore = MAX_flt;
	for (int32 i = 0; i < Blackboard.KnownThreats.Num(); ++i)
	{
		const FSharedThreat& K = Blackboard.KnownThreats[i];
		const float Score = K.ThreatLevel * K.Confidence;
		if (Score < WorstScore)
		{
			WorstScore = Score;
			WorstIndex = i;
		}
	}
	if (Report.ThreatLevel * Report.Confidence > WorstScore)
	{
		Blackboard.KnownThreats[WorstIndex] = Report;
	}
}

void USquadComponent::EnqueueContactDeliveries(const FSharedThreat& Report, int32 ReporterIndex, float NowSeconds)
{
	const USquadTuningData* T = GetTuning();
	if (!T)
	{
		return;
	}

	const float ReporterToTargetDist = Members.IsValidIndex(ReporterIndex)
		? FVector::Dist(Members[ReporterIndex].Status.Location, Report.ReportedLocation)
		: 0.f;

	for (int32 i = 0; i < Members.Num(); ++i)
	{
		if (i == ReporterIndex || !Members[i].IsAlive())
		{
			continue;
		}
		if (Blackboard.ReportQueue.Num() >= T->MaxReportQueueDepth)
		{
			// 큐 폭주 — 가장 오래된 것부터 버린다. 무전이 밀렸다는 뜻이고, 그래도 된다.
			Blackboard.ReportQueue.RemoveAt(0);
		}

		// --- [2] 수신자별 지연. ★ 동시에 5명이 고개를 돌리면 그 자체가 치팅 신호다 ---
		float Delay = T->BaseRadioDelaySec
			+ FMath::FRandRange(-T->RadioJitterSec, T->RadioJitterSec)
			+ (Report.bReporterIsLeader ? T->LeaderDelayBonusSec : 0.f)
			+ (Members[i].Status.CurrentTarget.IsValid() ? T->BusyRecipientDelayPenaltySec : 0.f);
		Delay = FMath::Clamp(Delay, T->RadioDelayClampSec.X, T->RadioDelayClampSec.Y);

		FPendingContactDelivery Delivery;
		Delivery.Payload = Report;
		Delivery.RecipientMemberIndex = i;
		Delivery.DeliverAtSeconds = NowSeconds + Delay;

		// --- [3] 위치 불확실성. ★ 이 한 줄이 "치팅처럼 보이지 않게"의 실체다 ---
		//     수신자는 좌표가 아니라 (추정 위치, σ) 를 받는다.
		//     σ 가 임계를 넘으면 정조준 대상이 될 수 없고, 확인하러 가거나 대략 제압할 뿐이다.
		const float ElapsedAtDelivery = Delay;
		float Sigma = T->BaseSigmaCm
			+ T->PerSecondSigmaCm * ElapsedAtDelivery
			+ T->DistanceSigmaCoef * ReporterToTargetDist;
		Delivery.Payload.LocationSigmaCm = FMath::Min(Sigma, T->MaxSigmaCm);

		Blackboard.ReportQueue.Add(Delivery);
	}
}

void USquadComponent::Phase_DeliverPendingReports(float NowSeconds)
{
	// 큐 pop — 허용된 4가지 연산 중 하나.
	// 3Hz 로 pop 하므로 최대 0.33초의 양자화 오차가 생기지만, 지연 자체가 0.8±0.3초라
	// 오차가 지연 안에 묻힌다. 오히려 지터가 공짜로 생긴다 (PLAN.md 5.4절).
	for (int32 i = Blackboard.ReportQueue.Num() - 1; i >= 0; --i)
	{
		const FPendingContactDelivery& Delivery = Blackboard.ReportQueue[i];
		if (Delivery.DeliverAtSeconds > NowSeconds)
		{
			continue;
		}

		if (Members.IsValidIndex(Delivery.RecipientMemberIndex))
		{
			AActor* Pawn = Members[Delivery.RecipientMemberIndex].Pawn.Get();
			if (Pawn && Members[Delivery.RecipientMemberIndex].IsAlive())
			{
				// ★ L1은 병합하지 않는다 (PLAN.md 5.3절). 배달만 하고, 병합 규칙 R1~R3 은
				//   수신자의 개인 기억이 판정한다 — 내 기억을 어떻게 갱신할지는 나만 안다.
				if (ISoldierThreatMemorySink* Sink = Cast<ISoldierThreatMemorySink>(Pawn))
				{
					Sink->ReceiveSharedThreat(Delivery.Payload);
				}
			}
		}
		Blackboard.ReportQueue.RemoveAtSwap(i);
	}
}

// ===========================================================================
// 페이즈 A — 토큰 (PLAN.md 6.3절)
// ===========================================================================

bool USquadComponent::HasToken(int32 MemberIndex, ESquadTokenType Type) const
{
	for (const FSquadTokenGrant& G : Blackboard.Grants)
	{
		if (G.Type != Type)
		{
			continue;
		}
		if (G.MemberIndex == MemberIndex)
		{
			return true;
		}
		// 조 단위 발급이면 그 조 전원이 보유한 것으로 본다 (PLAN.md 4.1절)
		if (G.ElementId != NAME_None)
		{
			for (const FSquadElement& E : Blackboard.Elements)
			{
				if (E.ElementId == G.ElementId && E.Contains(MemberIndex))
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool USquadComponent::RequestToken(int32 MemberIndex, ESquadTokenType Type)
{
	if (!Members.IsValidIndex(MemberIndex) || !Members[MemberIndex].IsAlive())
	{
		return false;
	}
	if (HasToken(MemberIndex, Type))
	{
		return true;
	}

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	// 요청 등록 — 다음 분대 틱에서 starvation 보너스를 받는다.
	// ★ 이 보너스가 없으면 특정 병사가 영구히 토큰을 못 받는다 (점수 배분의 고전적 실패).
	if (Members[MemberIndex].TokenRequestedAtSeconds < 0.f)
	{
		Members[MemberIndex].TokenRequestedAtSeconds = Now;
		Members[MemberIndex].RequestedTokenType = Type;
	}

	// 즉시 여유가 있으면 바로 발급한다 (StateTree EnterState 에서 한 틱 기다리지 않도록)
	const int32 Cap = (Type == ESquadTokenType::Suppression)
		? Blackboard.SuppressionTokens : Blackboard.MovementTokens;

	int32 Held = 0;
	for (const FSquadTokenGrant& G : Blackboard.Grants)
	{
		if (G.Type == Type) { ++Held; }
	}
	if (Held < Cap)
	{
		FSquadTokenGrant Grant;
		Grant.Type = Type;
		Grant.MemberIndex = MemberIndex;
		Grant.GrantedAtSeconds = Now;
		Blackboard.Grants.Add(Grant);
		Members[MemberIndex].TokenRequestedAtSeconds = -1.f;
		Assignments[MemberIndex].bHasSuppressionToken = HasToken(MemberIndex, ESquadTokenType::Suppression);
		Assignments[MemberIndex].bHasMovementToken = HasToken(MemberIndex, ESquadTokenType::Movement);
		return true;
	}
	return false;
}

void USquadComponent::ReleaseToken(int32 MemberIndex, ESquadTokenType Type)
{
	// ★ StateTree 의 ExitState 가 무조건 이걸 부른다 (상위계획 14.3절).
	//   사망·중단·상위 인터럽트 어느 경로로 나가도 누수가 없다.
	Blackboard.Grants.RemoveAll([MemberIndex, Type](const FSquadTokenGrant& G)
	{
		return G.Type == Type && G.MemberIndex == MemberIndex;
	});
	if (Members.IsValidIndex(MemberIndex))
	{
		Members[MemberIndex].TokenRequestedAtSeconds = -1.f;
	}
	if (Assignments.IsValidIndex(MemberIndex))
	{
		Assignments[MemberIndex].bHasSuppressionToken = HasToken(MemberIndex, ESquadTokenType::Suppression);
		Assignments[MemberIndex].bHasMovementToken = HasToken(MemberIndex, ESquadTokenType::Movement);
	}
}

void USquadComponent::Phase_UpdateTokens(float NowSeconds)
{
	const USquadTuningData* T = GetTuning();
	if (!T)
	{
		return;
	}

	// --- 상한 갱신 (플랜별. PLAN.md 6.3절 표) ---
	const int32 Alive = GetAliveCount();
	const FSquadPlanRule* Rule = PlanRules ? PlanRules->FindRule(Blackboard.CurrentPlan) : nullptr;
	if (Rule)
	{
		Blackboard.SuppressionTokens = FMath::Max(0,
			FMath::CeilToInt(Alive * Rule->SuppressionTokenRatio) + Rule->SuppressionTokenOffset);
		Blackboard.MovementTokens = FMath::Max(0,
			FMath::CeilToInt(Alive * Rule->MovementTokenRatio) + Rule->MovementTokenOffset);
	}

	// --- 회수 ---
	for (int32 i = Blackboard.Grants.Num() - 1; i >= 0; --i)
	{
		const FSquadTokenGrant& G = Blackboard.Grants[i];
		const float Held = NowSeconds - G.GrantedAtSeconds;

		// 최소 보유시간 — 받자마자 뺏기면 아무 일도 못 한다
		if (Held < T->TokenMinHoldSec)
		{
			continue;
		}

		// ★ 최대 보유시간. 이게 없으면 교대가 아예 안 난다 —
		//   한 명이 계속 쥐고 있으면 바운딩도 제압 순환도 멈춘다 (설계 9.5절 "토큰 순환").
		const float MaxHold = (G.Type == ESquadTokenType::Suppression)
			? T->SuppressionTokenMaxHoldSec : T->MovementTokenMaxHoldSec;

		bool bRevoke = (Held >= MaxHold);

		// 즉시 회수 조건
		if (!bRevoke && G.MemberIndex != INDEX_NONE && Members.IsValidIndex(G.MemberIndex))
		{
			const FSquadMemberEntry& M = Members[G.MemberIndex];
			bRevoke = !M.IsAlive() || (G.Type == ESquadTokenType::Suppression && M.Status.AmmoRatio <= 0.f);
		}

		if (bRevoke)
		{
			const int32 Owner = G.MemberIndex;
			Blackboard.Grants.RemoveAt(i);
			if (Members.IsValidIndex(Owner))
			{
				Assignments[Owner].bHasSuppressionToken = HasToken(Owner, ESquadTokenType::Suppression);
				Assignments[Owner].bHasMovementToken = HasToken(Owner, ESquadTokenType::Movement);
			}
		}
	}

	// --- 발급: 대기 큐를 점수순으로 ---
	for (int32 TypeIdx = 0; TypeIdx < 2; ++TypeIdx)
	{
		const ESquadTokenType Type = (TypeIdx == 0) ? ESquadTokenType::Suppression : ESquadTokenType::Movement;

		// 조 단위 발급 플랜에서는 개인 발급을 하지 않는다 (PLAN.md 4.1절).
		// 조 교대는 Phase_AssignTasks 가 관리한다.
		if (Type == ESquadTokenType::Movement && Rule && Rule->bMovementTokenPerElement)
		{
			continue;
		}

		const int32 Cap = (Type == ESquadTokenType::Suppression)
			? Blackboard.SuppressionTokens : Blackboard.MovementTokens;

		int32 Held = 0;
		for (const FSquadTokenGrant& G : Blackboard.Grants)
		{
			if (G.Type == Type) { ++Held; }
		}

		while (Held < Cap)
		{
			int32 BestIndex = INDEX_NONE;
			float BestPriority = -1.f;

			for (int32 i = 0; i < Members.Num(); ++i)
			{
				const FSquadMemberEntry& M = Members[i];
				if (!M.IsAlive() || M.TokenRequestedAtSeconds < 0.f || M.RequestedTokenType != Type)
				{
					continue;
				}
				if (HasToken(i, Type))
				{
					continue;
				}

				float RoleWeight = 1.f;
				if (Type == ESquadTokenType::Suppression)
				{
					if (M.Role == ESquadRole::AutomaticRifleman) { RoleWeight = T->RoleWeightAutomaticRiflemanSuppression; }
					else if (M.Role == ESquadRole::Marksman)      { RoleWeight = T->RoleWeightMarksmanSuppression; }
					if (M.Status.Health == ESquadMemberHealth::Injured) { RoleWeight *= 0.6f; }
				}
				else
				{
					if (M.Status.Health == ESquadMemberHealth::Injured) { RoleWeight *= 1.4f; }  // 부상자 이탈 우선
				}

				const float Waited = NowSeconds - M.TokenRequestedAtSeconds;
				const float Priority = RoleWeight + T->StarvationBonusPerSec * Waited;

				if (Priority > BestPriority)
				{
					BestPriority = Priority;
					BestIndex = i;
				}
			}

			if (BestIndex == INDEX_NONE)
			{
				break;
			}

			FSquadTokenGrant Grant;
			Grant.Type = Type;
			Grant.MemberIndex = BestIndex;
			Grant.GrantedAtSeconds = NowSeconds;
			Blackboard.Grants.Add(Grant);
			Members[BestIndex].TokenRequestedAtSeconds = -1.f;
			Assignments[BestIndex].bHasSuppressionToken = HasToken(BestIndex, ESquadTokenType::Suppression);
			Assignments[BestIndex].bHasMovementToken = HasToken(BestIndex, ESquadTokenType::Movement);
			++Held;
		}
	}
}

// ===========================================================================
// 페이즈 A — 표적·슬롯 중재 (PLAN.md 6.1~6.2절)
// ===========================================================================

void USquadComponent::Phase_UpdateTargetArbitration()
{
	const USquadTuningData* T = GetTuning();
	if (!T)
	{
		return;
	}

	// 표적별 교전 인원 집계 — pull 이므로 개인이 표적을 바꿀 때마다 통보할 필요가 없다.
	Blackboard.TargetEngagedCount.Reset();
	for (const FSquadMemberEntry& M : Members)
	{
		if (M.IsAlive() && M.Status.CurrentTarget.IsValid())
		{
			uint8& Count = Blackboard.TargetEngagedCount.FindOrAdd(M.Status.CurrentTarget);
			Count = static_cast<uint8>(FMath::Min<int32>(Count + 1, 255));
		}
	}

	// 고가치 표적 지정 (PLAN.md 4.3절). 1위가 2위의 Dominance 배를 넘을 때만.
	AActor* Best = nullptr;
	float BestScore = 0.f;
	float SecondScore = 0.f;
	for (const FSharedThreat& Threat : Blackboard.KnownThreats)
	{
		AActor* Target = Threat.Target.Get();
		if (!Target)
		{
			continue;
		}
		const float Score = Threat.ThreatLevel * Threat.Confidence;
		if (Score > BestScore)
		{
			SecondScore = BestScore;
			BestScore = Score;
			Best = Target;
		}
		else if (Score > SecondScore)
		{
			SecondScore = Score;
		}
	}

	AActor* Focus = nullptr;
	if (Best && (SecondScore <= KINDA_SMALL_NUMBER || BestScore >= SecondScore * T->FocusTargetDominanceRatio))
	{
		Focus = Best;
	}
	// 명령이 특정 액터를 Suppress 하라고 했으면 그것이 이긴다
	if (CurrentOrder.Verb == EOrderVerb::Suppress && CurrentOrder.Target.Type == EOrderTargetType::Actor)
	{
		Focus = CurrentOrder.Target.Actor.Get();
	}

	for (FSquadAssignment& A : Assignments)
	{
		A.FocusTarget = Focus;
	}

	// soft-claim 만료 정리
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	Blackboard.SlotClaims.RemoveAll([Now](const FSlotSoftClaim& C)
	{
		return C.ExpiresAtSeconds <= Now;
	});
}

void USquadComponent::RegisterSlotClaim(int32 MemberIndex, const FGuid& SlotId, float Score)
{
	const USquadTuningData* T = GetTuning();
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	// 멤버당 1개 — 최신이 이전 것을 덮어쓴다
	ReleaseSlotClaim(MemberIndex);

	FSlotSoftClaim Claim;
	Claim.SlotId = SlotId;
	Claim.MemberIndex = MemberIndex;
	Claim.Score = Score;
	Claim.ExpiresAtSeconds = Now + (T ? T->SlotSoftClaimLifetimeSec : 3.f);
	Blackboard.SlotClaims.Add(Claim);
}

void USquadComponent::ReleaseSlotClaim(int32 MemberIndex)
{
	Blackboard.SlotClaims.RemoveAll([MemberIndex](const FSlotSoftClaim& C)
	{
		return C.MemberIndex == MemberIndex;
	});
}

float USquadComponent::GetSlotContentionMultiplier(int32 RequestingMemberIndex, const FGuid& SlotId) const
{
	const USquadTuningData* T = GetTuning();
	const float Penalty = T ? T->ContestedSlotPenalty : 0.35f;

	for (const FSlotSoftClaim& Claim : Blackboard.SlotClaims)
	{
		if (Claim.SlotId != SlotId || Claim.MemberIndex == RequestingMemberIndex)
		{
			continue;
		}
		// ★ 0 을 반환하지 않는다 — 설계 10.5절("완벽한 엄폐만 찾지 않는다")과 정합하고,
		//   남은 슬롯이 그것뿐일 때 아무도 못 가는 사태를 막는다.
		return Penalty;
	}
	return 1.f;
}

float USquadComponent::GetTargetPreferenceMultiplier(AActor* Target) const
{
	const USquadTuningData* T = GetTuning();
	if (!T || !Target)
	{
		return 1.f;
	}

	const uint8* CountPtr = Blackboard.TargetEngagedCount.Find(Target);
	const int32 Count = CountPtr ? *CountPtr : 0;

	float Multiplier = 1.f / (1.f + T->TargetFocusDecay * static_cast<float>(Count));

	// FocusTarget 은 억제를 이긴다 — "지시가 있으면 집중" (PLAN.md 4.3절)
	if (Assignments.Num() > 0 && Assignments[0].FocusTarget.Get() == Target)
	{
		Multiplier *= T->TargetFocusBonus;
	}
	return Multiplier;
}

void USquadComponent::PublishFireLane(int32 MemberIndex, const FVector& Origin, const FVector& Direction)
{
	const USquadTuningData* T = GetTuning();
	ClearFireLane(MemberIndex);

	if (Blackboard.ActiveFireLanes.Num() >= Blackboard.SuppressionTokens)
	{
		return;   // 토큰 없이는 lane 도 없다 (PLAN.md 2.4절 상한)
	}

	FSquadFireLane Lane;
	Lane.Origin = Origin;
	Lane.Direction = Direction.GetSafeNormal();
	Lane.HalfWidthCm = T ? T->FireLaneHalfWidthCm : 150.f;
	Blackboard.ActiveFireLanes.Add(Lane);
}

void USquadComponent::ClearFireLane(int32 /*MemberIndex*/)
{
	// TODO: lane 에 MemberIndex 를 달아 개별 제거. 지금은 상한이 3~4개라
	//       Phase_UpdateTokens 에서 통째로 재구성해도 무해하다.
}

// ===========================================================================
// 페이즈 B — 플랜 선택 (PLAN.md 3.3~3.4절)
// ===========================================================================

bool USquadComponent::IsInContact() const
{
	const USquadTuningData* T = GetTuning();
	const float Threshold = T ? T->ConfirmedThreatConfidence : 0.7f;
	for (const FSharedThreat& Threat : Blackboard.KnownThreats)
	{
		if (Threat.Target.IsValid() && Threat.Confidence >= Threshold)
		{
			return true;
		}
	}
	return false;
}

float USquadComponent::ScorePlan(const FSquadPlanRule& Rule, float /*NowSeconds*/) const
{
	// --- 게이트: 하나라도 걸리면 0 ---
	if (Rule.AllowedVerbs.Num() > 0 && !Rule.AllowedVerbs.Contains(CurrentOrder.Verb))
	{
		return 0.f;
	}
	if (Rule.bUseContactGate && (IsInContact() != Rule.bRequiresContact))
	{
		return 0.f;
	}
	if (Blackboard.Morale < Rule.MinMorale || Blackboard.Morale > Rule.MaxMorale)
	{
		return 0.f;
	}
	if (GetAliveCount() < Rule.MinAliveMembers)
	{
		return 0.f;
	}
	if (Rule.bRequiresFlankCorridor)
	{
		// 설계 11.2절 bAllowFlanking — false 면 회랑 계산 자체를 하지 않는다
		if (!CurrentOrder.Constraints.bAllowFlanking)
		{
			return 0.f;
		}
		FBox Unused;
		if (!BuildFlankCorridor(Unused))
		{
			return 0.f;
		}
	}

	float Score = Rule.RuleWeight;

	// 히스테리시스 — 설계 8.3절이 개인 유틸리티에 준 것과 같은 값·같은 이유.
	// 분대는 개인보다 느려야 하므로 최소 유지시간을 추가로 건다 (아래 Phase_SelectPlan).
	const USquadTuningData* T = GetTuning();
	if (Rule.Plan == Blackboard.CurrentPlan)
	{
		Score *= (T ? T->PlanHysteresis : 1.15f);
	}
	return Score;
}

void USquadComponent::Phase_SelectPlan(float NowSeconds)
{
	const USquadTuningData* T = GetTuning();
	if (!T || !PlanRules)
	{
		return;
	}

	LastPlanScores.Reset();

	// --- 무조건 오버라이드 두 개 (PLAN.md 3.3절). 플랜 스코어링보다 먼저 판정한다 ---

	// ① 사기 붕괴 → Withdraw 강제. 단 MoraleFloor 가 높으면 여기 오지 않는다(결사 항전).
	if (Blackboard.Morale < T->MoraleCollapseThreshold
		&& Blackboard.Morale > CurrentOrder.Constraints.MoraleFloor)
	{
		if (Blackboard.CurrentPlan != ESquadPlan::Withdraw)
		{
			Blackboard.CurrentPlan = ESquadPlan::Withdraw;
			PushAssignmentsRevision();
		}
		return;
	}

	// ② 분산도 초과 → Regroup 강제 (해제는 히스테리시스로)
	if (Blackboard.CurrentPlan != ESquadPlan::Regroup && Blackboard.Dispersion > T->DispersionRegroupCm)
	{
		Blackboard.CurrentPlan = ESquadPlan::Regroup;
		PushAssignmentsRevision();
		return;
	}
	if (Blackboard.CurrentPlan == ESquadPlan::Regroup && Blackboard.Dispersion > T->DispersionReleaseCm)
	{
		return;   // 아직 모이는 중
	}

	// --- 최소 유지시간 ---
	const bool bMinDurationSatisfied =
		(PlanEnteredAtSeconds < 0.f) || ((NowSeconds - PlanEnteredAtSeconds) >= T->PlanMinDurationSec);

	// --- 스코어 선택 (계획기 없음. 설계 9.2절) ---
	ESquadPlan Best = Blackboard.CurrentPlan;
	float BestScore = -1.f;
	for (const FSquadPlanRule& Rule : PlanRules->Rules)
	{
		const float Score = ScorePlan(Rule, NowSeconds);
		LastPlanScores.Add(Rule.Plan, Score);   // 설계 2.3절 제3규칙: 왜 그랬는지 답할 수 있어야 한다
		if (Score > BestScore)
		{
			BestScore = Score;
			Best = Rule.Plan;
		}
	}

	if (Best != Blackboard.CurrentPlan && bMinDurationSatisfied && BestScore > 0.f)
	{
		Blackboard.CurrentPlan = Best;
		PushAssignmentsRevision();
	}
}

void USquadComponent::Phase_FormElements()
{
	// PLAN.md 2.3절 — 플랜 전환 시에만. 매 틱 재편성하면 분대원이 두 조를 왕복한다.
	if (Blackboard.Elements.Num() < 2)
	{
		Blackboard.Elements.SetNum(2);
		Blackboard.Elements[0].ElementId = TEXT("Alpha");
		Blackboard.Elements[1].ElementId = TEXT("Bravo");
	}
	Blackboard.Elements[0].MemberMask = 0;
	Blackboard.Elements[1].MemberMask = 0;

	// ① 분대장과 승계 1순위를 서로 다른 조에 — 한 방에 지휘부가 날아가지 않게
	int32 LeaderIndex = INDEX_NONE;
	int32 SecondIndex = INDEX_NONE;
	int32 BestOrder = MAX_int32;
	for (int32 i = 0; i < Members.Num(); ++i)
	{
		if (!Members[i].IsAlive()) { continue; }
		if (Members[i].Role == ESquadRole::Leader) { LeaderIndex = i; continue; }
		if (Members[i].SuccessionOrder < BestOrder) { BestOrder = Members[i].SuccessionOrder; SecondIndex = i; }
	}
	if (LeaderIndex != INDEX_NONE) { Blackboard.Elements[0].MemberMask |= (1u << LeaderIndex); }
	if (SecondIndex != INDEX_NONE) { Blackboard.Elements[1].MemberMask |= (1u << SecondIndex); }

	// ② 지원화기수를 갈라 각 조가 화력을 갖게 한다
	int32 ArNext = 0;
	for (int32 i = 0; i < Members.Num(); ++i)
	{
		if (!Members[i].IsAlive() || i == LeaderIndex || i == SecondIndex) { continue; }
		if (Members[i].Role != ESquadRole::AutomaticRifleman) { continue; }
		Blackboard.Elements[ArNext % 2].MemberMask |= (1u << i);
		++ArNext;
	}

	// ③ 나머지는 균등 분배 (위치 기준 분할은 S6 에서 — 지금은 단순하게)
	int32 Next = 0;
	for (int32 i = 0; i < Members.Num(); ++i)
	{
		if (!Members[i].IsAlive() || i == LeaderIndex || i == SecondIndex) { continue; }
		if (Members[i].Role == ESquadRole::AutomaticRifleman) { continue; }
		Blackboard.Elements[Next % 2].MemberMask |= (1u << i);
		++Next;
	}
}

void USquadComponent::Phase_AssignTasks()
{
	const USquadTuningData* T = GetTuning();
	const FSquadPlanRule* Rule = PlanRules ? PlanRules->FindRule(Blackboard.CurrentPlan) : nullptr;
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	// --- 조 단위 이동 토큰 교대 (PLAN.md 4.1절) ---
	//     ★ 개인 단위 발급으로는 바운딩 오버워치가 나오지 않는다 —
	//       매 틱 다른 2명이 토큰을 잡아 "산개 전진"이 되고, 조 교대가 절대 안 생긴다.
	if (Rule && Rule->bMovementTokenPerElement && Blackboard.Elements.Num() == 2)
	{
		const bool bBoundTooLong = (LastBoundStartSeconds >= 0.f)
			&& ((Now - LastBoundStartSeconds) >= (T ? T->BoundMaxDurationSec : 8.f));
		const bool bBoundMinElapsed = (LastBoundStartSeconds < 0.f)
			|| ((Now - LastBoundStartSeconds) >= (T ? T->BoundMinDurationSec : 2.f));

		bool bHandover = (MovementTokenElementId == NAME_None);

		if (!bHandover && bBoundMinElapsed)
		{
			// 종료 조건 ①: 이동조 생존자의 BoundArrivalRatio 이상이 bound line 도달
			// 종료 조건 ②: 최대 지속 초과   ③: 이동조 사상자   ④: 엄호조 제압 능력 상실
			// TODO(S3): ①③④ 판정. 지금은 ② 만 구현 — 이것만으로도 교대는 일어난다.
			bHandover = bBoundTooLong;
		}

		if (bHandover)
		{
			const FName Previous = MovementTokenElementId;
			MovementTokenElementId = (Previous == Blackboard.Elements[0].ElementId)
				? Blackboard.Elements[1].ElementId
				: Blackboard.Elements[0].ElementId;
			LastBoundStartSeconds = Now;

			// 교대 시 0.5초 겹침 — 겹치지 않으면 "아무도 안 쏘는" 한 틱이 생긴다.
			// TODO(S3): BoundHandoverOverlapSec 만큼 이전 조의 grant 를 유지.
			Blackboard.Grants.RemoveAll([](const FSquadTokenGrant& G)
			{
				return G.Type == ESquadTokenType::Movement;
			});

			FSquadTokenGrant Grant;
			Grant.Type = ESquadTokenType::Movement;
			Grant.ElementId = MovementTokenElementId;
			Grant.GrantedAtSeconds = Now;
			Blackboard.Grants.Add(Grant);
		}
	}
	else
	{
		MovementTokenElementId = NAME_None;
	}

	// --- 배역 배분 ---
	for (int32 i = 0; i < Members.Num(); ++i)
	{
		FSquadAssignment& A = Assignments[i];
		A.SquadId = SquadId;
		A.MemberIndex = i;
		A.Plan = Blackboard.CurrentPlan;
		A.Role = Members[i].Role;
		A.Corridor = Blackboard.AssignedCorridor;
		A.PlanAggression = CurrentOrder.Constraints.Aggression * (Rule ? Rule->AggressionScale : 1.f);
		A.bHasSuppressionToken = HasToken(i, ESquadTokenType::Suppression);
		A.bHasMovementToken = HasToken(i, ESquadTokenType::Movement);

		// 소속 조
		A.ElementId = NAME_None;
		for (const FSquadElement& E : Blackboard.Elements)
		{
			if (E.Contains(i)) { A.ElementId = E.ElementId; break; }
		}

		switch (Blackboard.CurrentPlan)
		{
		case ESquadPlan::BoundingOverwatch:
			A.Task = (A.ElementId == MovementTokenElementId) ? ESquadTask::Bound : ESquadTask::Overwatch;
			break;

		case ESquadPlan::Withdraw:
			// 바운딩의 부호 반전 — 같은 코드를 쓴다 (PLAN.md 4.4절)
			A.Task = (A.ElementId == MovementTokenElementId) ? ESquadTask::Withdraw : ESquadTask::RearGuard;
			A.Anchor = Blackboard.RallyPoint;
			break;

		case ESquadPlan::Flank:
			// 기동조는 회랑 안에서 자유. 고정조는 이동 토큰 0 이라 자동으로 고정된다.
			A.Task = (A.ElementId == Blackboard.Elements[1].ElementId)
				? ESquadTask::Maneuver : ESquadTask::Suppress;
			break;

		case ESquadPlan::BaseOfFire:
			A.Task = A.bHasSuppressionToken ? ESquadTask::Suppress : ESquadTask::Overwatch;
			break;

		case ESquadPlan::Regroup:
			A.Task = ESquadTask::Regroup;
			A.Anchor = Blackboard.RallyPoint;
			break;

		case ESquadPlan::Hold:
		case ESquadPlan::FormationMove:
		default:
			A.Task = ESquadTask::Overwatch;
			break;
		}

		A.Revision = AssignmentRevision;
	}

	// Hold 의 부채꼴 시야 분담 (설계 9.2절). 위협 방향에 2배 밀도로 가중 분할하는 것은 S3.
	if (Blackboard.CurrentPlan == ESquadPlan::Hold)
	{
		const int32 Alive = FMath::Max(1, GetAliveCount());
		const float Span = 360.f / static_cast<float>(Alive);
		int32 Slot = 0;
		for (int32 i = 0; i < Members.Num(); ++i)
		{
			if (!Members[i].IsAlive()) { continue; }
			const float Min = -180.f + Span * Slot;
			Assignments[i].SectorYawRange = FVector2D(Min, Min + Span);
			++Slot;
		}
	}
}

void USquadComponent::PushAssignmentsRevision()
{
	++AssignmentRevision;
	for (FSquadAssignment& A : Assignments)
	{
		A.Revision = AssignmentRevision;
	}
	for (int32 i = 0; i < Members.Num(); ++i)
	{
		if (AActor* Pawn = Members[i].Pawn.Get())
		{
			if (ISquadMemberInterface* Iface = Cast<ISquadMemberInterface>(Pawn))
			{
				Iface->OnSquadAssignmentChanged(AssignmentRevision);
			}
		}
	}
}

// ===========================================================================
// 페이즈 C — 공간 준비 (플랜 전환 시 1회. PLAN.md 4.2·4.5·7.4절)
// ===========================================================================

void USquadComponent::Phase_PrepareSpatial()
{
	// ★ 분대 틱에서 도는 유일한 공간 계산이며, 플랜 전환 시에만 돈다 (PLAN.md 7.3절 금지 규칙).
	switch (Blackboard.CurrentPlan)
	{
	case ESquadPlan::Flank:
	{
		FBox Corridor;
		if (BuildFlankCorridor(Corridor))
		{
			Blackboard.AssignedCorridor = Corridor;
		}
		else
		{
			// 회랑을 못 만들면 Flank 게이트를 닫고 BaseOfFire 로 폴백
			Blackboard.CurrentPlan = ESquadPlan::BaseOfFire;
			Blackboard.AssignedCorridor = FBox(ForceInit);
			PushAssignmentsRevision();
		}
		break;
	}

	case ESquadPlan::BoundingOverwatch:
		for (FSquadElement& E : Blackboard.Elements)
		{
			E.Anchor = ComputeBoundLine(E);
		}
		LastBoundStartSeconds = -1.f;   // 다음 Phase_AssignTasks 가 즉시 첫 조에 토큰을 준다
		break;

	case ESquadPlan::Withdraw:
	case ESquadPlan::Regroup:
		Blackboard.RallyPoint = ComputeRallyPoint();
		break;

	default:
		Blackboard.AssignedCorridor = FBox(ForceInit);
		break;
	}
}

bool USquadComponent::BuildFlankCorridor(FBox& OutCorridor) const
{
	const USquadTuningData* T = GetTuning();
	if (!T)
	{
		return false;
	}

	// 1순위: 저작 회랑. 저작자 의도가 런타임 추정을 이긴다 (PLAN.md 4.2절).
	if (CurrentOrder.Constraints.CorridorId != NAME_None)
	{
		// TODO(S6): 레벨의 CorridorId 볼륨을 조회해 FBox 를 얻는다.
		//           볼륨 조회는 캐시할 것 — 매번 액터 순회를 하면 안 된다.
	}

	const FVector Axis = (Blackboard.EnemyCentroid - Blackboard.FriendlyCentroid).GetSafeNormal2D();
	if (Axis.IsNearlyZero())
	{
		return false;
	}
	const FVector Perp = FVector::CrossProduct(Axis, FVector::UpVector).GetSafeNormal();
	const float Distance = FVector::Dist2D(Blackboard.EnemyCentroid, Blackboard.FriendlyCentroid);

	UNavigationSystemV1* Nav = GetWorld() ? UNavigationSystemV1::GetCurrent(GetWorld()) : nullptr;
	if (!Nav)
	{
		return false;
	}

	for (int32 SideIdx = 0; SideIdx < 2; ++SideIdx)
	{
		const float Side = (SideIdx == 0) ? 1.f : -1.f;
		const FVector Center = Blackboard.FriendlyCentroid
			+ Perp * Side * T->FlankOffsetCm
			+ Axis * (Distance * 0.5f);

		FNavLocation Projected;
		if (Nav->ProjectPointToNavigation(Center, Projected, FVector(500.f, 500.f, 500.f)))
		{
			OutCorridor = FBox::BuildAABB(Projected.Location, T->FlankHalfExtentCm);
			// TODO(S6): 기동조 대표 1명 → 회랑 중심의 경로 존재를 비동기로 확인.
			//           동기 FindPathSync 를 쓰지 말 것 — 분대 예산이 그 자리에서 터진다.
			return true;
		}
	}
	return false;
}

FVector USquadComponent::ComputeBoundLine(const FSquadElement& /*MovingElement*/) const
{
	const USquadTuningData* T = GetTuning();
	const float BoundLength = T ? T->BoundLengthCm : 1500.f;   // [C-62]

	FVector GoalLocation = Blackboard.EnemyCentroid;
	if (CurrentOrder.Target.Type == EOrderTargetType::Location)
	{
		GoalLocation = CurrentOrder.Target.Location;
	}
	else if (CurrentOrder.Target.Type == EOrderTargetType::Actor && CurrentOrder.Target.Actor.IsValid())
	{
		GoalLocation = CurrentOrder.Target.Actor->GetActorLocation();
	}

	const FVector ToGoal = GoalLocation - Blackboard.FriendlyCentroid;
	const float Remaining = ToGoal.Size2D();
	const FVector Dir = ToGoal.GetSafeNormal2D();

	return Blackboard.FriendlyCentroid + Dir * FMath::Min(BoundLength, Remaining);
}

FVector USquadComponent::ComputeRallyPoint() const
{
	// ★ EQS 를 쓰지 않는다 (PLAN.md 4.5절) — 분대 틱에서 질의를 발행하면 7.3절 금지 규칙을 깨고
	//   설계 12.2절의 EQS 0.7ms 예산을 분대 수만큼 잠식한다.
	FVector Candidate = Blackboard.FriendlyCentroid;

	if (CurrentOrder.Target.Type == EOrderTargetType::Location)
	{
		Candidate = CurrentOrder.Target.Location;
	}
	else
	{
		for (const FSquadMemberEntry& M : Members)
		{
			if (M.IsAlive() && M.Role == ESquadRole::Leader)
			{
				Candidate = M.Status.Location;
				break;
			}
		}
	}

	if (UNavigationSystemV1* Nav = GetWorld() ? UNavigationSystemV1::GetCurrent(GetWorld()) : nullptr)
	{
		FNavLocation Projected;
		if (Nav->ProjectPointToNavigation(Candidate, Projected, FVector(500.f, 500.f, 500.f)))
		{
			return Projected.Location;
		}
	}
	return Candidate;
}

// ===========================================================================
// 상태 보고 (설계 11.5절)
// ===========================================================================

void USquadComponent::BuildStatusReport(FOrderStatusReport& OutReport) const
{
	const USquadTuningData* T = GetTuning();

	OutReport.OrderId = CurrentOrder.OrderId;
	OutReport.SquadId = SquadId;
	OutReport.Casualties = CasualtyCount;
	OutReport.SquadMorale = Blackboard.Morale;
	OutReport.SquadCentroid = Blackboard.FriendlyCentroid;

	int32 Confirmed = 0;
	const float Threshold = T ? T->ConfirmedThreatConfidence : 0.7f;
	for (const FSharedThreat& Threat : Blackboard.KnownThreats)
	{
		if (Threat.Target.IsValid() && Threat.Confidence >= Threshold) { ++Confirmed; }
	}
	OutReport.ConfirmedEnemyCount = Confirmed;

	if (GetAliveCount() == 0)
	{
		OutReport.Status = EOrderStatus::Failed;
		OutReport.Progress = 0.f;
		return;
	}

	// Verb 별 Progress 술어 (PLAN.md 3.5절). enum 분기를 늘리지 말고 술어를 데이터화할 것.
	switch (CurrentOrder.Verb)
	{
	case EOrderVerb::Advance:
	case EOrderVerb::MoveTo:
	{
		// 시작점 → 목표 축 위의 진척. TODO(S7): 시작 centroid 를 명령 수신 시 캐시.
		OutReport.Progress = 0.f;
		break;
	}
	case EOrderVerb::Engage:
		OutReport.Progress = (InitialConfirmedEnemyCount > 0)
			? 1.f - static_cast<float>(Confirmed) / static_cast<float>(InitialConfirmedEnemyCount)
			: 0.f;
		break;
	default:
		OutReport.Progress = 0.f;
		break;
	}

	OutReport.Status = (OutReport.Progress >= 1.f) ? EOrderStatus::Achieved : EOrderStatus::InProgress;
}
