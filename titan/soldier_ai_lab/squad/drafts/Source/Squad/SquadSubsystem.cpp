// SoldierLab — 분대 스케줄러 구현
// 초안 (2026-09-09). ★ 컴파일 검증하지 않았다.

#include "SquadSubsystem.h"

#include "SquadComponent.h"
#include "SquadTuningData.h"
#include "../Command/SoldierOrder.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

DECLARE_STATS_GROUP(TEXT("SoldierAI"), STATGROUP_SoldierAI, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Squad Tick"), STAT_SquadTick, STATGROUP_SoldierAI);

void USquadSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Squads.Reset();
	SquadsById.Reset();
	MemberLookup.Reset();
	Cursor = 0;
	Accumulator = 0.f;
}

void USquadSubsystem::Deinitialize()
{
	Squads.Reset();
	SquadsById.Reset();
	MemberLookup.Reset();
	Super::Deinitialize();
}

TStatId USquadSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USquadSubsystem, STATGROUP_Tickables);
}

bool USquadSubsystem::IsTickable() const
{
	// 설계 4.3.2절: L1 은 서버 전용. 클라이언트에서는 아예 돌지 않는다 (CLAUDE.md P5).
	const UWorld* World = GetWorld();
	return World
		&& World->IsGameWorld()
		&& World->GetNetMode() != NM_Client
		&& Squads.Num() > 0;
}

void USquadSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UWorld* World = GetWorld();
	if (!World || Squads.Num() == 0)
	{
		return;
	}

	const float Now = World->GetTimeSeconds();

	// 죽은 참조 정리 (드물게)
	Squads.RemoveAll([](const TObjectPtr<USquadComponent>& S) { return S == nullptr; });
	if (Squads.Num() == 0)
	{
		return;
	}

	// --- 라운드로빈 크레딧 ---
	// 분대 수 × 희망 주파수만큼의 "틱 크레딧"을 매 프레임 적립하고, 1 이상이면 하나를 돌린다.
	float AggregateHz = 0.f;
	for (const TObjectPtr<USquadComponent>& S : Squads)
	{
		AggregateHz += S ? S->GetDesiredTickHz() : 0.f;
	}
	if (AggregateHz <= 0.f)
	{
		Accumulator = 0.f;
		return;
	}

	Accumulator += DeltaTime * AggregateHz;

	// ★ 몰아치기 방지. 밀린 것은 버린다 — 분대 결정은 한두 번 건너뛰어도 무해하다.
	Accumulator = FMath::Min(Accumulator, MaxAccumulator + 1.f);

	int32 ProcessedThisFrame = 0;
	const int32 MaxPerFrame = FallbackMaxSquadsPerFrame;   // ★ 스파이크 방어선

	while (Accumulator >= 1.f && ProcessedThisFrame < MaxPerFrame)
	{
		USquadComponent* Squad = Squads[Cursor % Squads.Num()];
		++Cursor;

		if (Squad)
		{
			SCOPE_CYCLE_COUNTER(STAT_SquadTick);   // [C-60] 판정 기준: 1회 ≤ 0.18ms
			Squad->TickSquad(Now);
		}

		Accumulator -= 1.f;
		++ProcessedThisFrame;
	}

	// 남은 크레딧이 상한을 넘으면 잘라낸다 (다음 프레임에 몰리지 않게)
	Accumulator = FMath::Min(Accumulator, MaxAccumulator);

	// 명령 만료 처리는 전용 틱을 하나 더 두지 않고 여기에 얹는다
	if (USoldierCommandSubsystem* Command = World->GetSubsystem<USoldierCommandSubsystem>())
	{
		Command->PruneExpiredOrders(Now);
	}
}

USquadComponent* USquadSubsystem::CreateSquad(const FSquadDefinition& Definition)
{
	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_Client)
	{
		return nullptr;
	}
	if (Definition.SquadId == NAME_None)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SquadSubsystem] SquadId 가 비어 있다 — 생성 거부"));
		return nullptr;
	}
	if (SquadsById.Contains(Definition.SquadId))
	{
		return SquadsById[Definition.SquadId];
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.Name = MakeUniqueObjectName(World, ASquadCoordinator::StaticClass(), Definition.SquadId);

	ASquadCoordinator* Coordinator = World->SpawnActor<ASquadCoordinator>(
		ASquadCoordinator::StaticClass(), FTransform::Identity, Params);

	if (!Coordinator || !Coordinator->SquadComponent)
	{
		return nullptr;
	}

	Coordinator->SquadComponent->InitializeSquad(Definition);
	Squads.Add(Coordinator->SquadComponent);
	SquadsById.Add(Definition.SquadId, Coordinator->SquadComponent);
	return Coordinator->SquadComponent;
}

USquadComponent* USquadSubsystem::FindSquad(FName SquadId) const
{
	const TObjectPtr<USquadComponent>* Found = SquadsById.Find(SquadId);
	return Found ? Found->Get() : nullptr;
}

bool USquadSubsystem::FindSquadHandleFor(const AActor* Pawn, FSquadHandle& OutHandle) const
{
	if (!Pawn)
	{
		return false;
	}
	if (const FSquadHandle* Handle = MemberLookup.Find(Pawn))
	{
		OutHandle = *Handle;
		return OutHandle.IsValid();
	}
	return false;
}

USquadComponent* USquadSubsystem::GetSquadForActor(const AActor* Pawn) const
{
	FSquadHandle Handle;
	if (FindSquadHandleFor(Pawn, Handle))
	{
		return FindSquad(Handle.SquadId);
	}
	return nullptr;
}

void USquadSubsystem::RegisterMemberLookup(const AActor* Pawn, const FSquadHandle& Handle)
{
	if (Pawn && Handle.IsValid())
	{
		MemberLookup.Add(Pawn, Handle);
	}
}

void USquadSubsystem::UnregisterMemberLookup(const AActor* Pawn)
{
	// ★ 죽어도 지우지 않는 것이 원칙이다 (PLAN.md 2.1절 — 인덱스 안정성).
	//   실제 액터 파괴 시에만 호출할 것.
	if (Pawn)
	{
		MemberLookup.Remove(Pawn);
	}
}
