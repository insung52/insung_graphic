// 초안 — 컴파일 검증 전. UE 5.8 기준.
// 배치 위치: Source/SoldierLab/Tactical/EnvQueryTest_CoverFiringPosition.h
//
// ★ 이 파이프라인에서 트레이스를 하는 유일한 테스트다. 반드시 질의의 **마지막**에 둔다.
//   (PLAN.md 6.1절 표 #8 / 9.2절 비용 산정)

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "Engine/EngineTypes.h"
#include "EnvQueryTest_CoverFiringPosition.generated.h"

/**
 * "이 엄폐 지점에서 표적을 실제로 쏠 수 있는가."
 *
 * 설계 10.4절의 핵심 트레이드오프 — 완벽히 숨으면 못 쏜다 — 의 반대편 축이다.
 * 이 테스트의 가중치와 UEnvQueryTest_CoverQuality 의 가중치 비율이 병사 성향
 * (공세적 / 방어적)을 결정하며, FOrderConstraints::Aggression 이 여기에 매핑된다.
 *
 * 점수: 0(어느 사격점에서도 안 보인다) ~ 1(OverTop 으로 깨끗하게 보인다)
 *       사격점별 ModeWeight — OverTop 1.0 / Lean 0.9 / Blind 0.3 (PLAN.md 5.4절)
 *
 * 비용 억제:
 *   - 아이템당 트레이스를 MaxTracesPerItem(기본 1)회로 제한한다.
 *   - 후보 사격점은 "표적 방향과의 내적"으로 미리 정렬해 가장 유망한 것부터 본다.
 *   - 트레이스는 사격점 → 표적 방향으로 쏜다. 반대로 쏘면 사격점 주변 지오메트리에
 *     파묻히는 오탐이 는다.
 */
UCLASS(MinimalAPI, meta = (DisplayName = "Cover Firing Position"))
class UEnvQueryTest_CoverFiringPosition : public UEnvQueryTest
{
	GENERATED_BODY()

public:
	UEnvQueryTest_CoverFiringPosition(const FObjectInitializer& ObjectInitializer);

	SOLDIERLAB_API virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
	SOLDIERLAB_API virtual FText GetDescriptionTitle() const override;
	SOLDIERLAB_API virtual FText GetDescriptionDetails() const override;

protected:
	/** 사격 대상. 기본값 UEnvQueryContext_PrimaryThreat. */
	UPROPERTY(EditDefaultsOnly, Category = Cover)
	TSubclassOf<UEnvQueryContext> TargetContext;

	/**
	 * 가시성 트레이스 채널.
	 * 설계 3.6절이 요구한 전용 Cover 채널을 프로젝트 설정에 추가한 뒤 여기에 지정한다.
	 * (엄폐물은 막지만 풀·난간 같은 것은 통과시키기 위해 Visibility 와 분리한다)
	 */
	UPROPERTY(EditDefaultsOnly, Category = Cover)
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	/** 표적 위치에서 위로 올려 잡는 오프셋(cm). 표적의 흉부를 겨냥한다. */
	UPROPERTY(EditDefaultsOnly, Category = Cover)
	float TargetHeightOffset = 60.0f;

	/** 아이템당 트레이스 상한. 0 이면 트레이스 없이 기하 판정만 한다(C-66 초과 시 폴백). */
	UPROPERTY(EditDefaultsOnly, Category = Cover, meta = (ClampMin = "0", ClampMax = "4"))
	int32 MaxTracesPerItem = 1;

	/** 표적까지의 사거리 상한(cm). 넘으면 사격 가능성을 0 으로 본다. */
	UPROPERTY(EditDefaultsOnly, Category = Cover, meta = (ClampMin = "0.0"))
	float MaxEngagementRange = 6000.0f;
};
