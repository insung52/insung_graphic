// SoldierLab — StateTree 노드 구현
// 초안 (2026-09-09). ★ 컴파일 검증하지 않았다. [C-63] 참고.

#include "SquadStateTreeNodes.h"

#include "SquadComponent.h"
#include "SquadSubsystem.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

namespace SoldierLabSquadNodes
{
	/** AIController → 분대 컴포넌트 + 멤버 인덱스. 실패하면 둘 다 무효 */
	static USquadComponent* ResolveSquad(const AAIController* Controller, int32& OutMemberIndex)
	{
		OutMemberIndex = INDEX_NONE;
		if (!Controller)
		{
			return nullptr;
		}
		const APawn* Pawn = Controller->GetPawn();
		const UWorld* World = Controller->GetWorld();
		if (!Pawn || !World)
		{
			return nullptr;
		}
		USquadSubsystem* Subsystem = World->GetSubsystem<USquadSubsystem>();
		if (!Subsystem)
		{
			return nullptr;
		}
		FSquadHandle Handle;
		if (!Subsystem->FindSquadHandleFor(Pawn, Handle))
		{
			return nullptr;
		}
		OutMemberIndex = Handle.MemberIndex;
		return Subsystem->FindSquad(Handle.SquadId);
	}
}

// ===========================================================================
// FSTE_SquadContext
// ===========================================================================

void FSTE_SquadContext::TreeStart(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);
	Data.bHasSquad = false;
	Data.CachedRevision = 0;
	Data.Assignment = FSquadAssignment();
}

void FSTE_SquadContext::Tick(FStateTreeExecutionContext& Context, const float /*DeltaTime*/) const
{
	FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);

	int32 MemberIndex = INDEX_NONE;
	USquadComponent* Squad = SoldierLabSquadNodes::ResolveSquad(Data.AIController, MemberIndex);

	if (!Squad || MemberIndex == INDEX_NONE)
	{
		// 분대가 없는 병사도 정상이다 — 단독 병사는 제약 없이 개인 유틸리티만으로 움직인다.
		Data.bHasSquad = false;
		return;
	}

	const FSquadAssignment& Live = Squad->GetAssignment(MemberIndex);
	Data.bHasSquad = true;

	// 배정은 3Hz 로만 바뀐다. 리비전이 같으면 복사를 건너뛴다 (PLAN.md 8.2절).
	if (Live.Revision != Data.CachedRevision)
	{
		Data.Assignment = Live;
		Data.CachedRevision = Live.Revision;
	}
	else
	{
		// 토큰 플래그만은 리비전과 무관하게 바뀔 수 있다 (매 분대 틱의 회수·발급).
		Data.Assignment.bHasMovementToken = Live.bHasMovementToken;
		Data.Assignment.bHasSuppressionToken = Live.bHasSuppressionToken;
		Data.Assignment.FocusTarget = Live.FocusTarget;
	}
}

// ===========================================================================
// FSTT_AcquireSquadToken
// ===========================================================================

EStateTreeRunStatus FSTT_AcquireSquadToken::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& /*Transition*/) const
{
	FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);

	int32 MemberIndex = INDEX_NONE;
	USquadComponent* Squad = SoldierLabSquadNodes::ResolveSquad(Data.AIController, MemberIndex);

	Data.MemberIndex = MemberIndex;
	Data.bHolding = false;

	if (!Squad || MemberIndex == INDEX_NONE)
	{
		// 분대가 없으면 토큰 제약도 없다 — 통과시킨다.
		// (분대 없는 단독 병사가 영영 못 움직이는 것이 더 나쁜 실패다)
		return EStateTreeRunStatus::Running;
	}

	if (Squad->RequestToken(MemberIndex, Data.TokenType))
	{
		Data.bHolding = true;
		return EStateTreeRunStatus::Running;
	}

	// 획득 실패 → 형제 상태(Overwatch)로 전이한다.
	// ★ 이것이 "토큰만으로 엄호가 창발한다"의 실제 배선이다. 분기 로직을 쓰지 않았다.
	return Data.bFailIfUnavailable ? EStateTreeRunStatus::Failed : EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSTT_AcquireSquadToken::Tick(FStateTreeExecutionContext& Context,
	const float /*DeltaTime*/) const
{
	FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);

	if (Data.MemberIndex == INDEX_NONE)
	{
		return EStateTreeRunStatus::Running;
	}

	int32 MemberIndex = INDEX_NONE;
	USquadComponent* Squad = SoldierLabSquadNodes::ResolveSquad(Data.AIController, MemberIndex);
	if (!Squad)
	{
		return EStateTreeRunStatus::Running;
	}

	if (!Data.bHolding)
	{
		// 대기 중 — starvation 보너스가 쌓이고 있다.
		if (Squad->RequestToken(Data.MemberIndex, Data.TokenType))
		{
			Data.bHolding = true;
		}
		return EStateTreeRunStatus::Running;
	}

	// 보유 중이었는데 분대가 회수했다(최대 보유시간 초과 = 교대 시점).
	// ★ 여기서 Failed 를 내야 상태를 벗어나고 ExitState 가 정리한다.
	if (!Squad->HasToken(Data.MemberIndex, Data.TokenType))
	{
		Data.bHolding = false;
		return EStateTreeRunStatus::Failed;
	}

	return EStateTreeRunStatus::Running;
}

void FSTT_AcquireSquadToken::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& /*Transition*/) const
{
	FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);

	// ★★ 상위계획 14.3절의 핵심: 상태를 벗어나면 부모가 자원을 정리한다.
	//     사망·중단·상위 인터럽트 어느 경로로 나가도 여기가 불린다 → 토큰 누수 없음.
	if (Data.MemberIndex != INDEX_NONE)
	{
		int32 MemberIndex = INDEX_NONE;
		if (USquadComponent* Squad = SoldierLabSquadNodes::ResolveSquad(Data.AIController, MemberIndex))
		{
			Squad->ReleaseToken(Data.MemberIndex, Data.TokenType);
		}
	}
	Data.bHolding = false;
	Data.MemberIndex = INDEX_NONE;
}

// ===========================================================================
// FSTT_ReportSlotClaim
// ===========================================================================

EStateTreeRunStatus FSTT_ReportSlotClaim::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& /*Transition*/) const
{
	FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);

	int32 MemberIndex = INDEX_NONE;
	USquadComponent* Squad = SoldierLabSquadNodes::ResolveSquad(Data.AIController, MemberIndex);
	Data.MemberIndex = MemberIndex;

	if (Squad && MemberIndex != INDEX_NONE && Data.SlotId.IsValid())
	{
		Squad->RegisterSlotClaim(MemberIndex, Data.SlotId, Data.Score);
	}
	// soft-claim 은 실패해도 시스템이 옳게 동작해야 한다 → 항상 Running.
	return EStateTreeRunStatus::Running;
}

void FSTT_ReportSlotClaim::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& /*Transition*/) const
{
	FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);

	if (Data.MemberIndex != INDEX_NONE)
	{
		int32 MemberIndex = INDEX_NONE;
		if (USquadComponent* Squad = SoldierLabSquadNodes::ResolveSquad(Data.AIController, MemberIndex))
		{
			Squad->ReleaseSlotClaim(Data.MemberIndex);
		}
	}
	Data.MemberIndex = INDEX_NONE;
}

// ===========================================================================
// Conditions
// ===========================================================================

bool FSTC_SquadTaskIs::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);
	return Data.Assignment.Task == Data.RequiredTask;
}

bool FSTC_SquadHasToken::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);
	return (Data.TokenType == ESquadTokenType::Movement)
		? Data.Assignment.bHasMovementToken
		: Data.Assignment.bHasSuppressionToken;
}

bool FSTC_IsInsideCorridor::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);

	// 회랑이 없으면 "제약 없음"이 기본값이다 (설계 11.2절: CorridorId 를 비우면 자유).
	if (!Data.Assignment.Corridor.IsValid)
	{
		return true;
	}
	const AAIController* Controller = Data.AIController;
	const APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	if (!Pawn)
	{
		return true;
	}

	const FBox Shrunk = Data.Assignment.Corridor.ExpandBy(-Data.MarginCm);
	return Shrunk.IsInsideXY(Pawn->GetActorLocation());
}
