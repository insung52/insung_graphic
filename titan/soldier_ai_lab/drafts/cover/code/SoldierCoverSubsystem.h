// 초안 — 컴파일 검증 전. UE 5.8 기준.
// 배치 위치: Source/SoldierLab/Tactical/SoldierCoverSubsystem.h
//
// 책임 (PLAN.md 6.3 / 9.3 / 9.5절):
//   1) 슬롯 엄폐 메타데이터의 월드 공간 평면 캐시   ← EQS 테스트가 유일하게 읽는 곳
//   2) UCoverBakeData 실체화 / 해제
//   3) 최근 점유 이력 (같은 자리 반복 방지)
//   4) 질의 동시 상한 토큰 (45명이 동시에 질의하는 피크를 막는다)

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SmartObjectTypes.h"
#include "SoldierCoverTypes.h"
#include "SoldierCoverSubsystem.generated.h"

class USmartObjectSubsystem;

UCLASS()
class SOLDIERLAB_API USoldierCoverSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// ---- UWorldSubsystem ---------------------------------------------------
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	// ---- 캐시 --------------------------------------------------------------

	/**
	 * 슬롯 하나를 캐시에 등록한다. 슬롯 정의에서 FSoldierCoverSlotData 를 1회만 읽는다.
	 * 등록 경로: ASoldierCoverPoint::BeginPlay / ASoldierCoverManager(베이크) / UCoverProviderComponent
	 */
	void RegisterCoverSlot(const FSmartObjectSlotHandle& SlotHandle);

	void UnregisterCoverSlot(const FSmartObjectSlotHandle& SlotHandle);

	/** 슬롯이 움직였을 때 (동적 엄폐물). 월드 공간 값을 다시 푼다. */
	void RefreshCoverSlot(const FSmartObjectSlotHandle& SlotHandle);

	/** EQS 테스트가 부르는 유일한 조회 함수. 없으면 nullptr. 락 없음. */
	const FSoldierCoverRuntimeInfo* FindCoverInfo(const FSmartObjectSlotHandle& SlotHandle) const;

	// ---- 평가 헬퍼 (EQS 테스트와 디버그 HUD 가 공유한다) --------------------

	/** PLAN.md 5.2절 AngularFactor. 수평 성분만 사용 — 고저차 보정은 C-64. */
	static float ComputeAngularFactor(const FSoldierCoverRuntimeInfo& Info, const FVector& ThreatLocation);

	/** PLAN.md 5.2절 HeightFactor. bStanding=false 는 웅크린 자세. */
	static float ComputeHeightFactor(ESoldierCoverHeight CoverHeight, bool bStanding);

	/** PLAN.md 5.2절 Protection = Angular * Height. */
	static float ComputeProtection(const FSoldierCoverRuntimeInfo& Info, const FVector& ThreatLocation, bool bStanding);

	/**
	 * PLAN.md 5.3절 Exposure = 1 - Σ(w_i 정규화 * Protection_i).
	 * ★ 이 정의는 설계 7.5절 인지의 Exposure 와 반드시 같아야 한다 (계약).
	 */
	static float ComputeExposure(const FSoldierCoverRuntimeInfo& Info,
	                             TConstArrayView<FCoverThreatInfo> Threats,
	                             bool bStanding);

	// ---- 질의 없이 답하는 값 (PLAN.md 6.4절) -------------------------------
	//
	// L2 유틸리티가 TakeCover 를 고르려면 "쓸 만한 엄폐가 있는가"를 먼저 알아야 하는데,
	// EQS 질의는 TakeCover 가 선택된 뒤에 발행된다. 질의 결과로 질의 여부를 정할 수는 없다.
	// → 트레이스도 경로탐색도 없이, 기하 Protection 만으로 낙관적으로 답한다.
	//
	// 이 두 값은 drafts/soldier_ai 의 SoldierScoringInputs::CoverSlotAvailable /
	// ::BestCoverSlotScore 로 그대로 들어간다 (그쪽 모듈이 "cover / EQS layer 가 채운다"고
	// 명시해 둔 자리다).

	/**
	 * @param OutBestScore  최고 기하 Protection (0~1). 트레이스·경로탐색 없음.
	 * @return              반경 내에 예약 안 된 엄폐 슬롯이 하나라도 있으면 true.
	 */
	bool QueryCoverAvailability(const FVector& FromLocation,
	                            float SearchRadiusCm,
	                            TConstArrayView<FCoverThreatInfo> Threats,
	                            float& OutBestScore) const;

	/**
	 * SoldierScoringInputs::CurrentCoverQuality.
	 * 슬롯을 점유 중이면 그 슬롯의 Protection, 아니면 0.
	 *
	 * ★ 인지의 Exposure 와의 계약 (PLAN.md 12.2절):
	 *      PerceivedExposure = 인지의 Exposure × (1 - CurrentCoverQuality)
	 *   인지 쪽 Exposure 는 자세·이동·사격 배수만 곱한 값이고 엄폐 항이 없다.
	 *   두 팀이 각자 "엄폐를 반영한 Exposure"를 만들면 엄폐가 두 번 곱해진다.
	 */
	float GetCurrentCoverQuality(const FSmartObjectSlotHandle& OccupiedSlot,
	                             TConstArrayView<FCoverThreatInfo> Threats,
	                             bool bStanding) const;

	// ---- 최근 점유 이력 ----------------------------------------------------

	void NotifySlotOccupied(const AActor* Soldier, const FSmartObjectSlotHandle& SlotHandle);

	/** 0(최근에 있었음) ~ 1(최근 이력 없음). UEnvQueryTest_CoverRecency 가 쓴다. */
	float GetRecencyScore(const AActor* Soldier, const FSmartObjectSlotHandle& SlotHandle) const;

	// ---- 질의 동시 상한 (PLAN.md 9.3절 ③) ----------------------------------

	/**
	 * 질의 토큰을 요청한다. 실패하면 이번 틱에 질의를 발행하지 않는다.
	 * EQS 매니저의 타임슬라이스는 질의를 "늦출" 뿐 거절하지 않으므로,
	 * 폭주는 프레임이 아니라 "3초 뒤에 움직이는 병사"로 나타난다.
	 */
	bool TryAcquireQueryToken(const AActor* Soldier);
	void ReleaseQueryToken(const AActor* Soldier);

	/** 디버그 HUD 용. */
	int32 GetInFlightQueryCount() const { return InFlightQueries.Num(); }

protected:
	/** 동시 in-flight 질의 상한. 데이터로 뺄 것 (P6) — 지금은 초안 기본값. */
	UPROPERTY(EditAnywhere, Category = "Cover|Perf", meta = (ClampMin = "1"))
	int32 MaxConcurrentQueries = 4;

	/** 최근 점유 이력의 유효 시간(초). */
	UPROPERTY(EditAnywhere, Category = "Cover|Perf")
	float RecencyWindowSeconds = 30.0f;

	/** 병사당 기억하는 최근 슬롯 수. */
	UPROPERTY(EditAnywhere, Category = "Cover|Perf")
	int32 RecencyHistoryDepth = 4;

private:
	struct FRecentOccupancy
	{
		FSmartObjectSlotHandle SlotHandle;
		double TimeSeconds = 0.0;
	};

	/**
	 * 평면 캐시. EQS 테스트는 이것만 읽는다.
	 * TODO(verify): FSmartObjectSlotHandle 에 GetTypeHash 가 정의돼 있는지 확인.
	 *               없으면 FSmartObjectSlotHandle 대신 그 내부 인덱스를 키로 쓴다.
	 */
	TMap<FSmartObjectSlotHandle, FSoldierCoverRuntimeInfo> CoverInfoCache;

	TMap<TWeakObjectPtr<const AActor>, TArray<FRecentOccupancy>> RecentOccupancy;

	TSet<TWeakObjectPtr<const AActor>> InFlightQueries;

	/** 슬롯 정의에서 FSoldierCoverSlotData 를 읽어 월드 공간으로 푼다. 등록/갱신 시에만 호출. */
	bool BuildRuntimeInfo(USmartObjectSubsystem& SmartObjects,
	                      const FSmartObjectSlotHandle& SlotHandle,
	                      FSoldierCoverRuntimeInfo& OutInfo) const;
};
