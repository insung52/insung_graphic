// 초안 — 컴파일 검증 전. UE 5.8 기준.
// 배치 위치: Source/SoldierLab/Tactical/SoldierCoverActors.h
//
// 층 1(슬롯 생성)의 세 경로 (PLAN.md 3절):
//   ASoldierCoverPoint     — 수동 저작. P0-3 은 이것만으로 간다.
//   ASoldierCoverVolume    — 베이크 대상 영역 지정 + 에디터 베이크 진입점.
//   ASoldierCoverManager   — UCoverBakeData 를 런타임에 실체화.
//   UCoverProviderComponent— 동적 엄폐물(UGV·잔해)이 자기 슬롯을 등록/해제.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "SmartObjectTypes.h"
#include "SoldierCoverTypes.h"
#include "SoldierCoverActors.generated.h"

class USmartObjectComponent;
class UBoxComponent;

/**
 * 수동 저작 엄폐 지점.
 *
 * 설계 10.2절 "수동 저작" 행에 대응하며, titan_example 의 기존
 * FiringPose.Marker / CoverPose.Marker 가 이관될 자리이기도 하다.
 *
 * ★ 재베이크가 이 액터를 절대 건드리지 않는다 — 베이크 산출물은 UCoverBakeData 라는
 *   별개 파일이기 때문이다 (PLAN.md 4절 = 설계 백로그 D2 의 답).
 */
UCLASS(Blueprintable)
class SOLDIERLAB_API ASoldierCoverPoint : public AActor
{
	GENERATED_BODY()

public:
	ASoldierCoverPoint();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR
	/** CoverFacing 부채꼴 · 사격점 · 엄폐 높이를 그린다. 좌표 규약(PLAN.md 2절) 검증용. */
	virtual void OnConstruction(const FTransform& Transform) override;
#endif

protected:
	/** 슬롯 정의는 SmartObjectComponent 가 들고 있고, 엄폐 메타데이터는 그 슬롯의 DefinitionData 에 있다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cover")
	TObjectPtr<USmartObjectComponent> SmartObjectComponent;

	/**
	 * 손으로 놓을 때의 편의값. BeginPlay 에서 슬롯 정의의 FSoldierCoverSlotData 와 대조해
	 * 불일치하면 경고를 띄운다 (저작 실수 방지).
	 */
	UPROPERTY(EditAnywhere, Category = "Cover")
	FSoldierCoverSlotData AuthoringPreview;
};

/**
 * 베이크 대상 영역.
 *
 * 영역을 비워두면 NavMesh 경계 전체를 대상으로 한다. 실무에서는 전장 구역별로 나눠 두는 편이
 * 재베이크 시간과 Perforce 충돌을 줄인다.
 */
UCLASS(Blueprintable)
class SOLDIERLAB_API ASoldierCoverVolume : public AActor
{
	GENERATED_BODY()

public:
	ASoldierCoverVolume();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cover|Bake")
	TObjectPtr<UBoxComponent> Bounds;

	/** 이 영역의 베이크 산출물. 레벨이 아니라 이 DataAsset 에 쓰인다. */
	UPROPERTY(EditAnywhere, Category = "Cover|Bake")
	TObjectPtr<UCoverBakeData> BakeData;

	// ---- 베이크 파라미터 (PLAN.md 3.2절) --------------------------------

	/** NavMesh 경계를 따라 후보점을 뽑는 간격(cm). */
	UPROPERTY(EditAnywhere, Category = "Cover|Bake", meta = (ClampMin = "25.0"))
	float SampleSpacingCm = 75.0f;

	/** 경계에서 안쪽으로 밀어 넣는 거리(cm). 캡슐 반경 + 여유. */
	UPROPERTY(EditAnywhere, Category = "Cover|Bake", meta = (ClampMin = "0.0"))
	float InsetFromEdgeCm = 52.0f;

	/** 차폐 판정 높이(cm). 40(Prone)은 현 범위 밖이라 기본에서 뺐다 — PLAN.md 8.2절 A7. */
	UPROPERTY(EditAnywhere, Category = "Cover|Bake")
	TArray<float> ProbeHeightsCm = { 180.0f, 100.0f };

	/** CoverFacing 을 찾기 위한 방향 스윕 간격(도). */
	UPROPERTY(EditAnywhere, Category = "Cover|Bake", meta = (ClampMin = "5.0", ClampMax = "45.0"))
	float FacingSweepStepDeg = 15.0f;

	/** 슬롯 병합 임계 — 이보다 가깝고 facing 차가 작으면 하나로 합친다. */
	UPROPERTY(EditAnywhere, Category = "Cover|Bake", meta = (ClampMin = "0.0"))
	float MergeDistanceCm = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Cover|Bake", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float MergeFacingToleranceDeg = 20.0f;

	/** 설계 3.6절이 요구한 전용 Cover 트레이스 채널. */
	UPROPERTY(EditAnywhere, Category = "Cover|Bake")
	TEnumAsByte<ECollisionChannel> CoverTraceChannel = ECC_WorldStatic;

#if WITH_EDITOR
	/**
	 * 베이크 실행. Details 패널 버튼 또는 에디터 유틸리티에서 부른다.
	 * ⚠ 자동 재베이크는 하지 않는다 — 비결정적 결과를 조용히 밀어넣게 된다.
	 *    지오메트리 해시 불일치는 "경고만" 띄운다 (PLAN.md 4절).
	 */
	UFUNCTION(CallInEditor, Category = "Cover|Bake")
	void BakeCoverPoints();

	/** 베이크 시점 지오메트리의 해시. 볼륨 내 스태틱 액터의 트랜스폼 + 메시 에셋 GUID 집합. */
	uint64 ComputeGeometryHash() const;
#endif
};

/**
 * UCoverBakeData 를 런타임에 실체화한다. 레벨당 1개.
 *
 * ★ 액터를 슬롯 수만큼 스폰하지 않는다.
 *   USmartObjectSubsystem::CreateSmartObject(Definition, Transform, OwnerData) 가
 *   컴포넌트 없이 런타임 인스턴스를 만들어 준다 (엔진 API 확인 — SmartObjectSubsystem.h:398).
 */
UCLASS(Blueprintable)
class SOLDIERLAB_API ASoldierCoverManager : public AActor
{
	GENERATED_BODY()

public:
	ASoldierCoverManager();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Cover")
	TArray<TObjectPtr<UCoverBakeData>> BakeDataSets;

	/** 실체화한 SmartObject 핸들. EndPlay 에서 전부 DestroySmartObject 한다. */
	TArray<FSmartObjectHandle> SpawnedObjects;
};

/**
 * 동적 엄폐물. UGV·차량·파괴 잔해에 붙인다.
 *
 * 설계 10.2절 "동적 장애물" 행. SoldierLab 에는 아직 UGV 가 없으므로(D8)
 * P1 후반 이후 항목이다.
 */
UCLASS(ClassGroup = (SoldierLab), meta = (BlueprintSpawnableComponent))
class SOLDIERLAB_API UCoverProviderComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCoverProviderComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 슬롯을 즉시 무효화한다(파괴·전복). 점유 중인 병사에게 CoverInvalidated 가 간다. */
	UFUNCTION(BlueprintCallable, Category = "Cover")
	void InvalidateCover();

	/** 액터가 움직였을 때 슬롯 월드 값을 다시 푼다. 이동 중 예약 유지 정책은 C-62. */
	UFUNCTION(BlueprintCallable, Category = "Cover")
	void RefreshCoverTransforms();

protected:
	/** 이 액터가 제공하는 엄폐 지점들. 액터 로컬 공간. */
	UPROPERTY(EditAnywhere, Category = "Cover")
	TArray<FSoldierCoverBakePoint> LocalCoverPoints;

	UPROPERTY(EditAnywhere, Category = "Cover")
	TSoftObjectPtr<class USmartObjectDefinition> SlotDefinition;

	TArray<FSmartObjectHandle> SpawnedObjects;
};
