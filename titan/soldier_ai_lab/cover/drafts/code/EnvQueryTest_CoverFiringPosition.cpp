// 초안 — 컴파일 검증 전. UE 5.8 기준.
// 배치 위치: Source/SoldierLab/Tactical/EnvQueryTest_CoverFiringPosition.cpp

#include "EnvQueryTest_CoverFiringPosition.h"

#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvQueryItemType_SmartObject.h"
#include "EnvQueryContext_CoverThreats.h"
#include "SoldierCoverSubsystem.h"

#define LOCTEXT_NAMESPACE "SoldierCover"

UEnvQueryTest_CoverFiringPosition::UEnvQueryTest_CoverFiringPosition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Cost = EEnvTestCost::High;					// ★ 질의의 마지막에 두라는 신호
	ValidItemType = UEnvQueryItemType_SmartObject::StaticClass();
	SetWorkOnFloatValues(true);

	TargetContext = UEnvQueryContext_PrimaryThreat::StaticClass();
}

void UEnvQueryTest_CoverFiringPosition::RunTest(FEnvQueryInstance& QueryInstance) const
{
	UObject* QueryOwner = QueryInstance.Owner.Get();
	UWorld* World = QueryInstance.World;
	if (QueryOwner == nullptr || World == nullptr)
	{
		return;
	}

	const USoldierCoverSubsystem* CoverSubsystem = UWorld::GetSubsystem<USoldierCoverSubsystem>(World);
	if (CoverSubsystem == nullptr)
	{
		return;
	}

	TArray<FVector> TargetLocations;
	if (!QueryInstance.PrepareContext(TargetContext, TargetLocations) || TargetLocations.IsEmpty())
	{
		return;	// 표적이 없으면 이 축은 판단 불가 — 테스트를 건너뛴다
	}

	for (FVector& Location : TargetLocations)
	{
		Location.Z += TargetHeightOffset;
	}

	FloatValueMin.BindData(QueryOwner, QueryInstance.QueryID);
	FloatValueMax.BindData(QueryOwner, QueryInstance.QueryID);
	const float MinThresholdValue = FloatValueMin.GetValue();
	const float MaxThresholdValue = FloatValueMax.GetValue();

	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(CoverFiringPosition), /*bTraceComplex*/ false);
	if (const AActor* OwnerActor = Cast<AActor>(QueryOwner))
	{
		TraceParams.AddIgnoredActor(OwnerActor);
	}

	const float MaxRangeSq = MaxEngagementRange * MaxEngagementRange;

	// 사격점 후보를 "표적 방향과의 정렬도" 로 정렬하기 위한 임시 버퍼.
	// 아이템 루프 밖에서 한 번만 잡아 재할당을 피한다.
	TArray<TPair<float, int32>, TInlineAllocator<8>> Ranked;

	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		const FSmartObjectSlotEQSItem& Item = UEnvQueryItemType_SmartObject::GetValue(It.GetItemData());
		const FSoldierCoverRuntimeInfo* Info = CoverSubsystem->FindCoverInfo(Item.SlotHandle);

		for (const FVector& TargetLocation : TargetLocations)
		{
			if (Info == nullptr || Info->FiringPointLocations.IsEmpty())
			{
				It.SetScore(TestPurpose, FilterType, 0.0f, MinThresholdValue, MaxThresholdValue);
				continue;
			}

			if (FVector::DistSquared(Info->SlotLocation, TargetLocation) > MaxRangeSq)
			{
				It.SetScore(TestPurpose, FilterType, 0.0f, MinThresholdValue, MaxThresholdValue);
				continue;
			}

			// 1) 기하 정렬도로 사격점 순위를 매긴다 (트레이스 전).
			Ranked.Reset();
			for (int32 PointIndex = 0; PointIndex < Info->FiringPointLocations.Num(); ++PointIndex)
			{
				const FVector& MuzzleLocation = Info->FiringPointLocations[PointIndex];

				FVector ToTarget = TargetLocation - MuzzleLocation;
				if (!ToTarget.Normalize())
				{
					continue;
				}

				// 사격점이 슬롯 중심에서 표적 쪽으로 나와 있을수록 유망하다.
				const FVector Offset = MuzzleLocation - Info->SlotLocation;
				const float Alignment = static_cast<float>(FVector::DotProduct(Offset.GetSafeNormal(), ToTarget));

				const float Viability = Info->FiringPointViability.IsValidIndex(PointIndex)
					? Info->FiringPointViability[PointIndex]
					: 1.0f;

				Ranked.Emplace(Alignment * Viability, PointIndex);
			}

			if (Ranked.IsEmpty())
			{
				It.SetScore(TestPurpose, FilterType, 0.0f, MinThresholdValue, MaxThresholdValue);
				continue;
			}

			Ranked.Sort([](const TPair<float, int32>& A, const TPair<float, int32>& B)
			{
				return A.Key > B.Key;
			});

			// 2) 상위 MaxTracesPerItem 개만 실제로 트레이스한다.
			float BestScore = 0.0f;

			if (MaxTracesPerItem <= 0)
			{
				// 폴백: 트레이스 없이 기하 판정만. C-66 에서 예산 초과가 확인되면 이 경로로 내린다.
				const int32 BestIndex = Ranked[0].Value;
				BestScore = Info->FiringPointViability.IsValidIndex(BestIndex)
					? Info->FiringPointViability[BestIndex]
					: 1.0f;
			}
			else
			{
				const int32 TraceCount = FMath::Min(MaxTracesPerItem, Ranked.Num());
				for (int32 RankIndex = 0; RankIndex < TraceCount; ++RankIndex)
				{
					const int32 PointIndex = Ranked[RankIndex].Value;
					const FVector& MuzzleLocation = Info->FiringPointLocations[PointIndex];

					// 사격점 → 표적. 막히지 않으면(=Hit 없음) 쏠 수 있다.
					const bool bBlocked = World->LineTraceTestByChannel(
						MuzzleLocation, TargetLocation, TraceChannel, TraceParams);

					if (!bBlocked)
					{
						BestScore = Info->FiringPointViability.IsValidIndex(PointIndex)
							? Info->FiringPointViability[PointIndex]
							: 1.0f;
						break;	// 하나만 뚫리면 충분하다
					}
				}
			}

			It.SetScore(TestPurpose, FilterType, BestScore, MinThresholdValue, MaxThresholdValue);
		}
	}
}

FText UEnvQueryTest_CoverFiringPosition::GetDescriptionTitle() const
{
	return FText::Format(LOCTEXT("CoverFiringTitle", "Can Engage {0} From Cover"),
		FText::FromString(GetNameSafe(TargetContext)));
}

FText UEnvQueryTest_CoverFiringPosition::GetDescriptionDetails() const
{
	return DescribeFloatTestParams();
}

#undef LOCTEXT_NAMESPACE
