// 초안 — 컴파일 검증 전. UE 5.8 기준.
// 배치 위치: Source/SoldierLab/Tactical/EnvQueryTest_CoverQuality.cpp

#include "EnvQueryTest_CoverQuality.h"

#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvQueryItemType_SmartObject.h"			// SmartObjectsModule/Public (플랫 경로)
#include "EnvQueryContext_CoverThreats.h"
#include "SoldierCoverSubsystem.h"

#define LOCTEXT_NAMESPACE "SoldierCover"

// ---------------------------------------------------------------------------
// UEnvQueryTest_CoverQuality
// ---------------------------------------------------------------------------

UEnvQueryTest_CoverQuality::UEnvQueryTest_CoverQuality(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Cost = EEnvTestCost::Low;					// 트레이스 없음. 값싼 필터라 파이프라인 위쪽에 둔다
	ValidItemType = UEnvQueryItemType_SmartObject::StaticClass();
	SetWorkOnFloatValues(true);

	ThreatContext = UEnvQueryContext_PrimaryThreat::StaticClass();
}

void UEnvQueryTest_CoverQuality::RunTest(FEnvQueryInstance& QueryInstance) const
{
	UObject* QueryOwner = QueryInstance.Owner.Get();
	if (QueryOwner == nullptr || QueryInstance.World == nullptr)
	{
		return;
	}

	FloatValueMin.BindData(QueryOwner, QueryInstance.QueryID);
	FloatValueMax.BindData(QueryOwner, QueryInstance.QueryID);
	const float MinThresholdValue = FloatValueMin.GetValue();
	const float MaxThresholdValue = FloatValueMax.GetValue();

	const USoldierCoverSubsystem* CoverSubsystem = UWorld::GetSubsystem<USoldierCoverSubsystem>(QueryInstance.World);
	if (CoverSubsystem == nullptr)
	{
		return;
	}

	TArray<FVector> ThreatLocations;
	if (!QueryInstance.PrepareContext(ThreatContext, ThreatLocations) || ThreatLocations.IsEmpty())
	{
		// 위협을 모르면 이 축은 판단 불가. 테스트를 건너뛴다(모든 아이템 유지).
		return;
	}

	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		const FSmartObjectSlotEQSItem& Item = UEnvQueryItemType_SmartObject::GetValue(It.GetItemData());
		const FSoldierCoverRuntimeInfo* Info = CoverSubsystem->FindCoverInfo(Item.SlotHandle);

		if (Info == nullptr || !Info->IsValid())
		{
			// 엄폐 메타데이터가 없는 슬롯. 컨텍스트 개수만큼 동일 점수를 넣어 준다.
			for (int32 Index = 0; Index < ThreatLocations.Num(); ++Index)
			{
				It.SetScore(TestPurpose, FilterType, ScoreForNonCoverSlot, MinThresholdValue, MaxThresholdValue);
			}
			continue;
		}

		for (const FVector& ThreatLocation : ThreatLocations)
		{
			float Protection = 0.0f;
			switch (StanceMode)
			{
			case ECoverQualityStanceMode::Crouched:
				Protection = USoldierCoverSubsystem::ComputeProtection(*Info, ThreatLocation, /*bStanding*/ false);
				break;

			case ECoverQualityStanceMode::Standing:
				Protection = USoldierCoverSubsystem::ComputeProtection(*Info, ThreatLocation, /*bStanding*/ true);
				break;

			case ECoverQualityStanceMode::Best:
			default:
				Protection = FMath::Max(
					USoldierCoverSubsystem::ComputeProtection(*Info, ThreatLocation, /*bStanding*/ false),
					USoldierCoverSubsystem::ComputeProtection(*Info, ThreatLocation, /*bStanding*/ true));
				break;
			}

			// 여러 위협이면 여기서 여러 번 호출된다.
			// 집계는 MultipleContextScoreOp / MultipleContextFilterOp 가 한다 — 우리가 합산하지 않는다.
			It.SetScore(TestPurpose, FilterType, Protection, MinThresholdValue, MaxThresholdValue);
		}
	}
}

FText UEnvQueryTest_CoverQuality::GetDescriptionTitle() const
{
	return FText::Format(LOCTEXT("CoverQualityTitle", "Cover Quality vs {0}"),
		FText::FromString(GetNameSafe(ThreatContext)));
}

FText UEnvQueryTest_CoverQuality::GetDescriptionDetails() const
{
	return DescribeFloatTestParams();
}

// ---------------------------------------------------------------------------
// UEnvQueryTest_CoverRecency
// ---------------------------------------------------------------------------

UEnvQueryTest_CoverRecency::UEnvQueryTest_CoverRecency(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Cost = EEnvTestCost::Low;
	ValidItemType = UEnvQueryItemType_SmartObject::StaticClass();
	SetWorkOnFloatValues(true);
	TestPurpose = EEnvTestPurpose::Score;
}

void UEnvQueryTest_CoverRecency::RunTest(FEnvQueryInstance& QueryInstance) const
{
	UObject* QueryOwner = QueryInstance.Owner.Get();
	if (QueryOwner == nullptr || QueryInstance.World == nullptr)
	{
		return;
	}

	const USoldierCoverSubsystem* CoverSubsystem = UWorld::GetSubsystem<USoldierCoverSubsystem>(QueryInstance.World);
	if (CoverSubsystem == nullptr)
	{
		return;
	}

	// 이력은 "병사(액터)" 기준으로 기록된다. Owner 가 컨트롤러면 Pawn 으로 내려간다.
	const AActor* Soldier = Cast<AActor>(QueryOwner);
	if (const AController* Controller = Cast<AController>(QueryOwner))
	{
		Soldier = Controller->GetPawn() ? static_cast<const AActor*>(Controller->GetPawn()) : Soldier;
	}

	FloatValueMin.BindData(QueryOwner, QueryInstance.QueryID);
	FloatValueMax.BindData(QueryOwner, QueryInstance.QueryID);
	const float MinThresholdValue = FloatValueMin.GetValue();
	const float MaxThresholdValue = FloatValueMax.GetValue();

	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		const FSmartObjectSlotEQSItem& Item = UEnvQueryItemType_SmartObject::GetValue(It.GetItemData());
		const float Score = CoverSubsystem->GetRecencyScore(Soldier, Item.SlotHandle);
		It.SetScore(TestPurpose, FilterType, Score, MinThresholdValue, MaxThresholdValue);
	}
}

FText UEnvQueryTest_CoverRecency::GetDescriptionTitle() const
{
	return LOCTEXT("CoverRecencyTitle", "Cover Recency (avoid last position)");
}

#undef LOCTEXT_NAMESPACE
