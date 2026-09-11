// 초안 — 컴파일 검증 전. UE 5.8 기준.
// 배치 위치: Source/SoldierLab/Tactical/EnvQueryContext_CoverThreats.h
//
// ★ 이 파일이 인지(설계 7절)와의 접점이다. 위협의 "신뢰도 × 위험도" 계산은 인지 담당 범위이고,
//   엄폐 시스템은 아래 인터페이스가 주는 결과만 소비한다 (PLAN.md 5.3 / 12절).
//
// TODO: ISoldierThreatProvider 의 최종 위치는 Source/SoldierLab/Perception/ 이 맞다.
//       인지 담당이 그쪽에 만들면 이 파일에서는 include 만 남긴다.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "SoldierCoverTypes.h"
#include "EnvQueryContext_CoverThreats.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class USoldierThreatProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * 위협 목록 공급자. USoldierPerceptionComponent(설계 7절) 또는 AIController 가 구현한다.
 *
 * ★ 분대 단위로 한 벌만 만들어 공유하는 것을 권장한다 (PLAN.md 9.5절) —
 *   5명이 각자 위협 목록을 만들면 Exposure 계산의 앞단이 5배가 된다.
 *   FSquadBlackboard::KnownThreats(설계 9.3절)가 이미 그 자리다.
 */
class ISoldierThreatProvider
{
	GENERATED_BODY()

public:
	/** 주 위협 하나. 없으면 false. */
	virtual bool GetPrimaryThreat(FCoverThreatInfo& OutThreat) const = 0;

	/** 알려진 위협 전부. 주 위협을 포함한다. */
	virtual void GetKnownThreats(TArray<FCoverThreatInfo>& OutThreats) const = 0;
};

/**
 * 주 위협 1개의 위치.
 * UEnvQueryTest_CoverQuality(차폐도, 가중 3.0)와 UEnvQueryTest_CoverFiringPosition 이 쓴다.
 */
UCLASS()
class SOLDIERLAB_API UEnvQueryContext_PrimaryThreat : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};

/**
 * 알려진 위협 전부의 위치.
 * "다른 위협에 대한 노출"(가중 2.0, ScoringEquation=InverseLinear) 테스트가 쓴다.
 *
 * ⚠ 가중치(Weight)는 EQS 컨텍스트로 전달할 수 없다 — 컨텍스트는 위치 배열만 준다.
 *    가중을 반영해야 하면 UEnvQueryTest_CoverQuality 가 컨텍스트 대신
 *    ISoldierThreatProvider 를 직접 조회하도록 bUseWeightedThreats 를 켠다.
 */
UCLASS()
class SOLDIERLAB_API UEnvQueryContext_KnownThreats : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};

namespace SoldierCover
{
	/** QueryInstance 의 Owner 에서 ISoldierThreatProvider 를 찾는다. Owner → Controller → Pawn 순. */
	SOLDIERLAB_API const ISoldierThreatProvider* FindThreatProvider(const UObject* QueryOwner);
}
