// 초안 — 컴파일 검증 전. UE 5.8 기준.
// 배치 위치: Source/SoldierLab/Tactical/EnvQueryContext_CoverThreats.cpp

#include "EnvQueryContext_CoverThreats.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"

namespace SoldierCover
{
	const ISoldierThreatProvider* FindThreatProvider(const UObject* QueryOwner)
	{
		if (QueryOwner == nullptr)
		{
			return nullptr;
		}

		// 1) Owner 자신
		if (const ISoldierThreatProvider* Direct = Cast<const ISoldierThreatProvider>(QueryOwner))
		{
			return Direct;
		}

		const AActor* OwnerActor = Cast<const AActor>(QueryOwner);
		if (OwnerActor == nullptr)
		{
			return nullptr;
		}

		// 2) 컴포넌트
		TArray<UActorComponent*> Components;
		OwnerActor->GetComponents(Components);
		for (const UActorComponent* Component : Components)
		{
			if (const ISoldierThreatProvider* FromComponent = Cast<const ISoldierThreatProvider>(Component))
			{
				return FromComponent;
			}
		}

		// 3) Controller ↔ Pawn 반대편
		const AActor* Other = nullptr;
		if (const APawn* Pawn = Cast<const APawn>(OwnerActor))
		{
			Other = Pawn->GetController();
		}
		else if (const AController* Controller = Cast<const AController>(OwnerActor))
		{
			Other = Controller->GetPawn();
		}

		if (Other != nullptr && Other != OwnerActor)
		{
			return FindThreatProvider(Other);
		}

		return nullptr;
	}
}

void UEnvQueryContext_PrimaryThreat::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	const ISoldierThreatProvider* Provider = SoldierCover::FindThreatProvider(QueryInstance.Owner.Get());
	if (Provider == nullptr)
	{
		return;	// 컨텍스트가 비면 EQS 가 해당 테스트를 건너뛴다
	}

	FCoverThreatInfo Threat;
	if (!Provider->GetPrimaryThreat(Threat))
	{
		return;
	}

	UEnvQueryItemType_Point::SetContextHelper(ContextData, Threat.Location);
}

void UEnvQueryContext_KnownThreats::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	const ISoldierThreatProvider* Provider = SoldierCover::FindThreatProvider(QueryInstance.Owner.Get());
	if (Provider == nullptr)
	{
		return;
	}

	TArray<FCoverThreatInfo> Threats;
	Provider->GetKnownThreats(Threats);
	if (Threats.IsEmpty())
	{
		return;
	}

	TArray<FVector> Locations;
	Locations.Reserve(Threats.Num());
	for (const FCoverThreatInfo& Threat : Threats)
	{
		Locations.Add(Threat.Location);
	}

	UEnvQueryItemType_Point::SetContextHelper(ContextData, Locations);
}
