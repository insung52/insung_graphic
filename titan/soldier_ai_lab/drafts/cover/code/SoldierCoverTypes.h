// Copyright ... SoldierLab
// 초안 — 컴파일 검증 전. UE 5.8 기준.
// 배치 위치: Source/SoldierLab/Tactical/SoldierCoverTypes.h
//
// 좌표 규약 (PLAN.md 2절):
//   슬롯 트랜스폼의 +X = CoverFacing = "이 슬롯이 막아주는 방향의 중심"(엄폐물이 있는 쪽).
//   병사는 +X 를 바라보고 서며, 그 앞에 엄폐물이 있다.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "SmartObjectTypes.h"          // FSmartObjectDefinitionData / FSmartObjectSlotHandle
#include "SoldierCoverTypes.generated.h"

/** 엄폐물 높이. 40cm(Prone)은 현 범위 밖 — PLAN.md 2절 / 8.2절 A7. */
UENUM(BlueprintType)
enum class ESoldierCoverHeight : uint8
{
	/** 약 100cm. 웅크려야 가려진다. 서면 상체가 노출된다. */
	Low		UMETA(DisplayName = "Low (crouch)"),
	/** 약 180cm. 서서도 가려진다. */
	High	UMETA(DisplayName = "High (stand)")
};

/** 이 슬롯에서 가능한 사격 방식. 비트마스크. */
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ESoldierCoverFireMode : uint8
{
	None		= 0			UMETA(Hidden),
	LeanLeft	= 1 << 0,
	LeanRight	= 1 << 1,
	/** 낮은 엄폐물 위로 넘겨쏘기. */
	OverTop		= 1 << 2,
	/** 맹목사격 — 시야 없이 총만 내민다. 조준사격은 불가. */
	Blind		= 1 << 3
};
ENUM_CLASS_FLAGS(ESoldierCoverFireMode);

/** L3 → L4 계약. FSoldierPoseIntent 에 얹힐 축 (PLAN.md 7.5절). */
UENUM(BlueprintType)
enum class ESoldierCoverPosture : uint8
{
	None,
	/** 엄폐물 뒤. 사격하지 않음. */
	Behind,
	PeekLeft,
	PeekRight,
	OverTop,
	/** 완전 은폐. 총을 내리고 조준 오프셋을 끈다. */
	Hunker
};

/** 하나의 사격점. 오프셋은 전부 슬롯 로컬 공간. */
USTRUCT(BlueprintType)
struct FSoldierCoverFiringPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cover")
	ESoldierCoverFireMode Mode = ESoldierCoverFireMode::OverTop;

	/** 사격 시 총구가 놓이는 위치 (슬롯 로컬). 베이크 3.2절 6단계 산출. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cover")
	FVector3f LocalMuzzleOffset = FVector3f::ZeroVector;

	/** 이 사격점을 쓰려면 필요한 자세. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cover")
	ESoldierCoverPosture RequiredPosture = ESoldierCoverPosture::OverTop;

	/** 점수 가중. PLAN.md 5.4절 ModeWeight. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cover", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Viability = 1.0f;
};

/**
 * 엄폐 메타데이터. SmartObject 슬롯 정의에 직접 얹힌다.
 *
 * FSmartObjectSlotDefinition::DefinitionData (TArray<FSmartObjectDefinitionDataProxy>) 에
 * 담기며, 런타임에는
 *     FConstSmartObjectSlotView::GetDefinitionDataPtr<FSoldierCoverSlotData>()
 * 로 읽는다. (SmartObjectRuntime.h:663 / SmartObjectDefinition.h:143 확인)
 *
 * ⚠ 단 EQS 테스트 안에서는 이 경로를 쓰지 않는다 — ReadSlotData 가 락을 잡기 때문.
 *    등록 시 1회만 읽어 USoldierCoverSubsystem 의 평면 캐시로 옮긴다 (PLAN.md 6.3절).
 */
USTRUCT(BlueprintType)
struct FSoldierCoverSlotData : public FSmartObjectDefinitionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cover")
	ESoldierCoverHeight CoverHeight = ESoldierCoverHeight::High;

	/** CoverFacing(+X) 기준 이 각도 안쪽은 완전 차폐. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cover", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float ProtectionHalfAngleInnerDeg = 45.0f;

	/** 이 각도에서 차폐가 0 이 된다. Inner 이상이어야 한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cover", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float ProtectionHalfAngleOuterDeg = 80.0f;

	/** 가능한 사격 방식 (ESoldierCoverFireMode 비트마스크). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cover", meta = (Bitmask, BitmaskEnum = "/Script/SoldierLab.ESoldierCoverFireMode"))
	uint8 FireModes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cover")
	TArray<FSoldierCoverFiringPoint> FiringPoints;

	/** 엄폐물 상단 높이 (슬롯 원점 기준 cm). 총구 클리어런스 판정용 — PLAN.md 7.2절. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cover")
	float CoverTopHeightCm = 100.0f;

	/** 저작 출처. 재베이크가 수동 저작을 건드리지 않는지 검증할 때 쓴다 (D2). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cover")
	bool bAuthoredByHand = false;
};

/**
 * 런타임 평면 캐시 1행. 월드 공간으로 미리 풀어 둔다.
 * EQS 테스트는 오직 이것만 읽는다.
 */
USTRUCT()
struct FSoldierCoverRuntimeInfo
{
	GENERATED_BODY()

	/** 슬롯 월드 위치 (캡슐 중심). */
	UPROPERTY()
	FVector SlotLocation = FVector::ZeroVector;

	/** 월드 공간 CoverFacing. 수평 정규화되어 있다. */
	UPROPERTY()
	FVector CoverFacing = FVector::ForwardVector;

	UPROPERTY()
	float CosInner = 0.707f;	// cos(InnerDeg)

	UPROPERTY()
	float CosOuter = 0.174f;	// cos(OuterDeg)

	UPROPERTY()
	ESoldierCoverHeight CoverHeight = ESoldierCoverHeight::High;

	UPROPERTY()
	uint8 FireModes = 0;

	UPROPERTY()
	float CoverTopHeightCm = 100.0f;

	/** 월드 공간으로 푼 사격점 위치 + 모드 가중. */
	UPROPERTY()
	TArray<FVector> FiringPointLocations;

	UPROPERTY()
	TArray<float> FiringPointViability;

	bool IsValid() const { return !CoverFacing.IsNearlyZero(); }
};

/** 하나의 위협. 인지(설계 7절)가 채워 준다 — PLAN.md 5.3절 계약. */
USTRUCT(BlueprintType)
struct FCoverThreatInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Cover")
	FVector Location = FVector::ZeroVector;

	/** (신뢰도 × 위험도). 소비 측에서 Σ=1 로 정규화한다. */
	UPROPERTY(BlueprintReadWrite, Category = "Cover", meta = (ClampMin = "0.0"))
	float Weight = 1.0f;
};

// ---------------------------------------------------------------------------
// 베이크 산출물 (PLAN.md 4절 = 설계 백로그 D2 의 답)
// ---------------------------------------------------------------------------

/** 베이크된 엄폐 지점 1개. 레벨 액터가 아니라 이 구조체로 저장된다. */
USTRUCT()
struct FSoldierCoverBakePoint
{
	GENERATED_BODY()

	UPROPERTY()
	FTransform SlotTransform = FTransform::Identity;

	UPROPERTY()
	FSoldierCoverSlotData Data;
};

/**
 * 레벨 1개당 DataAsset 1개.
 *
 * 왜 레벨 액터가 아닌가 (PLAN.md 4절):
 *   - 재베이크가 수동 저작(ASoldierCoverPoint 액터)을 절대 못 지운다 — 파일이 다르다.
 *     보장을 "정책"이 아니라 "구조"로 만든다.
 *   - Perforce 체크아웃이 에셋 1개. 레벨을 건드리지 않는다.
 *   - World Partition 로딩 / 액터 초기화 비용이 없다.
 */
UCLASS(BlueprintType)
class SOLDIERLAB_API UCoverBakeData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 이 베이크가 대상으로 삼은 레벨. */
	UPROPERTY(VisibleAnywhere, Category = "Cover|Bake")
	FName LevelName;

	/** 베이크 시점의 지오메트리 해시. 불일치 시 에디터 경고만 띄우고 자동 재베이크는 하지 않는다. */
	UPROPERTY(VisibleAnywhere, Category = "Cover|Bake")
	uint64 GeometryHash = 0;

	UPROPERTY(VisibleAnywhere, Category = "Cover|Bake")
	FDateTime BakeTimestamp;

	/** 실체화에 쓸 SmartObject 정의. C-60 — 슬롯 1개짜리 정의를 N번 인스턴스화하는 방식. */
	UPROPERTY(EditAnywhere, Category = "Cover|Bake")
	TSoftObjectPtr<class USmartObjectDefinition> SlotDefinition;

	UPROPERTY(VisibleAnywhere, Category = "Cover|Bake")
	TArray<FSoldierCoverBakePoint> Points;
};
