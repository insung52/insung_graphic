// 초안 — 컴파일 검증 전. UE 5.8 기준.
// 배치 위치: Source/SoldierLab/Tactical/EnvQueryTest_CoverQuality.h
//
// 트레이스를 하지 않는다. 베이크가 미리 구한 CoverFacing / HalfAngle 로 각도 계산만 한다.
// (PLAN.md 5.1절 — "어느 방향을 막는가"는 베이크가 답하고, 트레이스는 "쏠 수 있는가"에만 쓴다)

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "SoldierCoverTypes.h"
#include "EnvQueryTest_CoverQuality.generated.h"

/** 이 테스트가 어떤 자세를 가정하고 차폐를 계산하는가. */
UENUM()
enum class ECoverQualityStanceMode : uint8
{
	/** 웅크린 자세 기준. Low 엄폐도 100% 로 쳐 준다. */
	Crouched,
	/** 선 자세 기준. Low 엄폐는 크게 감점된다. */
	Standing,
	/** 둘 중 좋은 쪽. "웅크릴 수 있으면 웅크린다"를 표현한다 — 기본값. */
	Best
};

/**
 * 엄폐 차폐도 테스트.
 *
 * 아이템 타입: UEnvQueryItemType_SmartObject (엔진 UEnvQueryGenerator_SmartObjects 가 생성)
 * 점수: PLAN.md 5.2절 Protection = AngularFactor * HeightFactor,  [0,1]
 *
 * ★ 컨텍스트가 여러 위협을 주면 SetScore 가 위협마다 호출된다.
 *   집계 방식은 엔진의 MultipleContextScoreOp 가 정한다 — 우리가 합산 코드를 쓰지 않는다.
 *     - 주 위협 테스트(가중 3.0)  : 컨텍스트 = PrimaryThreat, 항목 1개
 *     - 다중 위협 테스트(가중 2.0): 컨텍스트 = KnownThreats,
 *                                   MultipleContextScoreOp = MinScore
 *                                   → "가장 안 막아주는 위협" 기준이 되어 설계 10.4절의
 *                                     "다른 위협에 대한 노출 감점"과 같은 효과를 낸다.
 *                                     (별도의 InverseLinear 설정이 필요 없다)
 *
 * ⚠ 위협별 가중치(신뢰도×위험도)는 EQS 컨텍스트로 전달되지 않는다.
 *    가중이 꼭 필요하면 EnvQueryContext_CoverThreats.h 의 주석을 볼 것.
 */
UCLASS(MinimalAPI, meta = (DisplayName = "Cover Quality"))
class UEnvQueryTest_CoverQuality : public UEnvQueryTest
{
	GENERATED_BODY()

public:
	UEnvQueryTest_CoverQuality(const FObjectInitializer& ObjectInitializer);

	SOLDIERLAB_API virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
	SOLDIERLAB_API virtual FText GetDescriptionTitle() const override;
	SOLDIERLAB_API virtual FText GetDescriptionDetails() const override;

protected:
	/** 위협 위치를 주는 컨텍스트. 기본값 UEnvQueryContext_PrimaryThreat. */
	UPROPERTY(EditDefaultsOnly, Category = Cover)
	TSubclassOf<UEnvQueryContext> ThreatContext;

	UPROPERTY(EditDefaultsOnly, Category = Cover)
	ECoverQualityStanceMode StanceMode = ECoverQualityStanceMode::Best;

	/**
	 * 엄폐 메타데이터가 없는 슬롯(벤치 등)의 점수.
	 * 필터로 쓰면 0 이 걸러진다. 기본 0.
	 */
	UPROPERTY(EditDefaultsOnly, Category = Cover)
	float ScoreForNonCoverSlot = 0.0f;
};

/**
 * "최근에 있던 자리" 감점 (설계 10.4절 마지막 항, 가중 0.5).
 * 점수 0(방금 있던 자리) ~ 1(최근 이력 없음).
 * 설계 10.5절 "전원이 같은 곳으로 몰리는" 실패 모드의 세 대책 중 하나.
 */
UCLASS(MinimalAPI, meta = (DisplayName = "Cover Recency"))
class UEnvQueryTest_CoverRecency : public UEnvQueryTest
{
	GENERATED_BODY()

public:
	UEnvQueryTest_CoverRecency(const FObjectInitializer& ObjectInitializer);

	SOLDIERLAB_API virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
	SOLDIERLAB_API virtual FText GetDescriptionTitle() const override;
};
