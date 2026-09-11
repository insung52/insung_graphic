// 초안 — 컴파일 검증 전. UE 5.8 기준.
// 배치 위치: Source/SoldierLab/Tactical/SoldierCoverActors.cpp
//
// 이 초안은 ASoldierCoverManager 만 구현한다. 여기가 이 설계에서 유일하게
// "엔진 API 를 새로 쓰는" 지점이라 먼저 확정해 두는 것이 낫다.
// 나머지(수동 슬롯 액터 · 베이크 볼륨 · 동적 컴포넌트)는 P0-3/P1 착수 시 채운다.

#include "SoldierCoverActors.h"

#include "Engine/World.h"
#include "SmartObjectSubsystem.h"
#include "SmartObjectDefinition.h"
#include "SoldierCoverSubsystem.h"

ASoldierCoverManager::ASoldierCoverManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;	// 엄폐 슬롯은 서버 권위 데이터다 (설계 4.3절)
}

void ASoldierCoverManager::BeginPlay()
{
	Super::BeginPlay();

	// 설계 P5 — 모든 L0~L3 진입점에 권위 게이트.
	if (!HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	USmartObjectSubsystem* SmartObjects = UWorld::GetSubsystem<USmartObjectSubsystem>(World);
	USoldierCoverSubsystem* CoverSubsystem = UWorld::GetSubsystem<USoldierCoverSubsystem>(World);
	if (SmartObjects == nullptr || CoverSubsystem == nullptr)
	{
		return;
	}

	for (const UCoverBakeData* BakeData : BakeDataSets)
	{
		if (BakeData == nullptr)
		{
			continue;
		}

		// TODO(verify): 동기 로드. 슬롯 수가 많으면 비동기 로드 + 지연 실체화로 바꾼다.
		USmartObjectDefinition* Definition = BakeData->SlotDefinition.LoadSynchronous();
		if (Definition == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Cover] %s: SlotDefinition 이 없다. 실체화를 건너뛴다."),
				*GetNameSafe(BakeData));
			continue;
		}

		SpawnedObjects.Reserve(SpawnedObjects.Num() + BakeData->Points.Num());

		for (const FSoldierCoverBakePoint& Point : BakeData->Points)
		{
			// ★ 컴포넌트 없이 SmartObject 런타임 인스턴스를 만든다.
			//    이것이 "베이크 산출물을 레벨 액터로 두지 않아도 되는" 근거다 (PLAN.md 4절).
			const FSmartObjectHandle Handle =
				SmartObjects->CreateSmartObject(*Definition, Point.SlotTransform, FConstStructView());

			if (!Handle.IsValid())
			{
				continue;
			}

			SpawnedObjects.Add(Handle);

			// 슬롯 메타데이터를 우리 평면 캐시에 옮긴다.
			// ⚠ 여기서 한 번만 읽는다. EQS 테스트 안에서는 ReadSlotData 를 부르지 않는다
			//    (락 비용 — PLAN.md 6.3절).
			TArray<FSmartObjectSlotHandle> Slots;
			SmartObjects->GetAllSlots(Handle, Slots);
			for (const FSmartObjectSlotHandle& SlotHandle : Slots)
			{
				CoverSubsystem->RegisterCoverSlot(SlotHandle);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[Cover] %d 개 엄폐 SmartObject 실체화 완료."), SpawnedObjects.Num());
}

void ASoldierCoverManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		USmartObjectSubsystem* SmartObjects = UWorld::GetSubsystem<USmartObjectSubsystem>(World);
		USoldierCoverSubsystem* CoverSubsystem = UWorld::GetSubsystem<USoldierCoverSubsystem>(World);

		if (SmartObjects != nullptr)
		{
			for (const FSmartObjectHandle& Handle : SpawnedObjects)
			{
				if (CoverSubsystem != nullptr)
				{
					TArray<FSmartObjectSlotHandle> Slots;
					SmartObjects->GetAllSlots(Handle, Slots);
					for (const FSmartObjectSlotHandle& SlotHandle : Slots)
					{
						CoverSubsystem->UnregisterCoverSlot(SlotHandle);
					}
				}

				SmartObjects->DestroySmartObject(Handle);
			}
		}
	}

	SpawnedObjects.Reset();
	Super::EndPlay(EndPlayReason);
}
