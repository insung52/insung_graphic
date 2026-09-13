// 초안 — 컴파일 검증 전. UE 5.8 기준.
// 배치 위치: Source/SoldierLab/Tactical/SoldierCoverSubsystem.cpp

#include "SoldierCoverSubsystem.h"

#include "Engine/World.h"
#include "SmartObjectSubsystem.h"
#include "SmartObjectRuntime.h"		// FConstSmartObjectSlotView

// ---------------------------------------------------------------------------
// UWorldSubsystem
// ---------------------------------------------------------------------------

void USoldierCoverSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// SmartObjectSubsystem 이 먼저 준비되도록 의존성을 건다.
	Collection.InitializeDependency<USmartObjectSubsystem>();
	Super::Initialize(Collection);
}

void USoldierCoverSubsystem::Deinitialize()
{
	CoverInfoCache.Empty();
	RecentOccupancy.Empty();
	InFlightQueries.Empty();
	Super::Deinitialize();
}

bool USoldierCoverSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game
		|| WorldType == EWorldType::PIE
		|| WorldType == EWorldType::Editor;	// 베이크 툴 / EQS 테스팅 폰에서 쓴다
}

// ---------------------------------------------------------------------------
// 캐시
// ---------------------------------------------------------------------------

bool USoldierCoverSubsystem::BuildRuntimeInfo(USmartObjectSubsystem& SmartObjects,
                                              const FSmartObjectSlotHandle& SlotHandle,
                                              FSoldierCoverRuntimeInfo& OutInfo) const
{
	const TOptional<FTransform> SlotTransform = SmartObjects.GetSlotTransform(SlotHandle);
	if (!SlotTransform.IsSet())
	{
		return false;
	}

	bool bFound = false;

	// 락을 잡는 경로다. 등록/갱신 시점에만 부른다 — EQS 테스트 안에서는 절대 부르지 않는다.
	SmartObjects.ReadSlotData(SlotHandle,
		[&](FConstSmartObjectSlotView SlotView)
		{
			const FSoldierCoverSlotData* CoverData = SlotView.GetDefinitionDataPtr<FSoldierCoverSlotData>();
			if (CoverData == nullptr)
			{
				return;	// 엄폐 슬롯이 아니다 (벤치 등)
			}

			const FTransform& Xf = SlotTransform.GetValue();

			OutInfo.SlotLocation = Xf.GetLocation();

			// CoverFacing = 슬롯 +X 의 수평 성분.
			FVector Facing = Xf.GetUnitAxis(EAxis::X);
			Facing.Z = 0.0;
			if (!Facing.Normalize())
			{
				return;	// 위/아래를 보는 슬롯은 무효
			}
			OutInfo.CoverFacing = Facing;

			const float InnerDeg = FMath::Clamp(CoverData->ProtectionHalfAngleInnerDeg, 0.0f, 180.0f);
			const float OuterDeg = FMath::Clamp(CoverData->ProtectionHalfAngleOuterDeg, InnerDeg, 180.0f);
			OutInfo.CosInner = FMath::Cos(FMath::DegreesToRadians(InnerDeg));
			OutInfo.CosOuter = FMath::Cos(FMath::DegreesToRadians(OuterDeg));

			OutInfo.CoverHeight      = CoverData->CoverHeight;
			OutInfo.FireModes        = CoverData->FireModes;
			OutInfo.CoverTopHeightCm = CoverData->CoverTopHeightCm;

			OutInfo.FiringPointLocations.Reset(CoverData->FiringPoints.Num());
			OutInfo.FiringPointViability.Reset(CoverData->FiringPoints.Num());
			for (const FSoldierCoverFiringPoint& Point : CoverData->FiringPoints)
			{
				const FVector LocalOffset(Point.LocalMuzzleOffset);
				OutInfo.FiringPointLocations.Add(Xf.TransformPosition(LocalOffset));
				OutInfo.FiringPointViability.Add(Point.Viability);
			}

			bFound = true;
		});

	return bFound;
}

void USoldierCoverSubsystem::RegisterCoverSlot(const FSmartObjectSlotHandle& SlotHandle)
{
	USmartObjectSubsystem* SmartObjects = UWorld::GetSubsystem<USmartObjectSubsystem>(GetWorld());
	if (SmartObjects == nullptr)
	{
		return;
	}

	FSoldierCoverRuntimeInfo Info;
	if (BuildRuntimeInfo(*SmartObjects, SlotHandle, Info))
	{
		CoverInfoCache.Add(SlotHandle, MoveTemp(Info));
	}
}

void USoldierCoverSubsystem::UnregisterCoverSlot(const FSmartObjectSlotHandle& SlotHandle)
{
	CoverInfoCache.Remove(SlotHandle);
}

void USoldierCoverSubsystem::RefreshCoverSlot(const FSmartObjectSlotHandle& SlotHandle)
{
	RegisterCoverSlot(SlotHandle);	// 덮어쓰기
}

const FSoldierCoverRuntimeInfo* USoldierCoverSubsystem::FindCoverInfo(const FSmartObjectSlotHandle& SlotHandle) const
{
	return CoverInfoCache.Find(SlotHandle);
}

// ---------------------------------------------------------------------------
// 평가 (PLAN.md 5.2 / 5.3절)
// ---------------------------------------------------------------------------

float USoldierCoverSubsystem::ComputeAngularFactor(const FSoldierCoverRuntimeInfo& Info, const FVector& ThreatLocation)
{
	FVector ToThreat = ThreatLocation - Info.SlotLocation;
	ToThreat.Z = 0.0;	// 수평 성분만 — 고저차 보정은 C-64
	if (!ToThreat.Normalize())
	{
		return 0.0f;	// 위협이 슬롯 바로 위/아래
	}

	const float Cos = static_cast<float>(FVector::DotProduct(Info.CoverFacing, ToThreat));

	if (Cos >= Info.CosInner)
	{
		return 1.0f;
	}
	if (Cos <= Info.CosOuter)
	{
		return 0.0f;
	}

	// Inner → Outer 구간을 부드럽게 1 → 0.
	// 계단 함수면 위협이 경계를 오갈 때 점수가 튀고 병사가 두 슬롯 사이를 왕복한다.
	const float Denom = Info.CosInner - Info.CosOuter;
	const float Alpha = (Denom > KINDA_SMALL_NUMBER) ? ((Cos - Info.CosOuter) / Denom) : 0.0f;
	return FMath::SmoothStep(0.0f, 1.0f, Alpha);
}

float USoldierCoverSubsystem::ComputeHeightFactor(ESoldierCoverHeight CoverHeight, bool bStanding)
{
	if (CoverHeight == ESoldierCoverHeight::High)
	{
		return 1.0f;
	}
	// Low 엄폐에서 서 있으면 상체가 노출된다. 0.25 는 C-63 튜닝 대상 — 데이터로 뺄 것(P6).
	return bStanding ? 0.25f : 1.0f;
}

float USoldierCoverSubsystem::ComputeProtection(const FSoldierCoverRuntimeInfo& Info,
                                                const FVector& ThreatLocation,
                                                bool bStanding)
{
	return ComputeAngularFactor(Info, ThreatLocation) * ComputeHeightFactor(Info.CoverHeight, bStanding);
}

float USoldierCoverSubsystem::ComputeExposure(const FSoldierCoverRuntimeInfo& Info,
                                              TConstArrayView<FCoverThreatInfo> Threats,
                                              bool bStanding)
{
	if (Threats.IsEmpty())
	{
		return 0.0f;
	}

	float TotalWeight = 0.0f;
	for (const FCoverThreatInfo& Threat : Threats)
	{
		TotalWeight += FMath::Max(0.0f, Threat.Weight);
	}
	if (TotalWeight <= KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}

	float WeightedProtection = 0.0f;
	for (const FCoverThreatInfo& Threat : Threats)
	{
		const float NormalizedWeight = FMath::Max(0.0f, Threat.Weight) / TotalWeight;
		WeightedProtection += NormalizedWeight * ComputeProtection(Info, Threat.Location, bStanding);
	}

	return FMath::Clamp(1.0f - WeightedProtection, 0.0f, 1.0f);
}

// ---------------------------------------------------------------------------
// 질의 없이 답하는 값 (PLAN.md 6.4절)
// ---------------------------------------------------------------------------

bool USoldierCoverSubsystem::QueryCoverAvailability(const FVector& FromLocation,
                                                    float SearchRadiusCm,
                                                    TConstArrayView<FCoverThreatInfo> Threats,
                                                    float& OutBestScore) const
{
	OutBestScore = 0.0f;

	USmartObjectSubsystem* SmartObjects = UWorld::GetSubsystem<USmartObjectSubsystem>(GetWorld());
	if (SmartObjects == nullptr)
	{
		return false;
	}

	const float RadiusSq = SearchRadiusCm * SearchRadiusCm;
	bool bAnyAvailable = false;

	// TODO(perf): 지금은 캐시 전체를 훑는다. 슬롯이 수백 개를 넘으면
	//             USmartObjectSubsystem 의 공간 조회(FindSmartObjects, QueryBox)로 후보를 먼저 줄일 것.
	//             이 함수는 2Hz 로만 돌므로 P0-3 규모에서는 이대로 충분하다.
	for (const TPair<FSmartObjectSlotHandle, FSoldierCoverRuntimeInfo>& Pair : CoverInfoCache)
	{
		const FSoldierCoverRuntimeInfo& Info = Pair.Value;

		if (FVector::DistSquared(FromLocation, Info.SlotLocation) > RadiusSq)
		{
			continue;
		}

		// 예약된 슬롯은 후보가 아니다. EQS 쪽은 Generator 의 bOnlyClaimable 이 같은 일을 한다.
		bool bClaimable = false;
		SmartObjects->ReadSlotData(Pair.Key,
			[&bClaimable](FConstSmartObjectSlotView SlotView)
			{
				bClaimable = SlotView.CanBeClaimed();	// TODO(verify): 5.8 의 정확한 함수명
			});

		if (!bClaimable)
		{
			continue;
		}

		bAnyAvailable = true;

		// ★ 트레이스도 경로탐색도 하지 않는다. 낙관적으로 답하고, 틀리면 실제 질의가 걸러낸다.
		const float Score = 1.0f - ComputeExposure(Info, Threats, /*bStanding*/ false);
		OutBestScore = FMath::Max(OutBestScore, Score);
	}

	return bAnyAvailable;
}

float USoldierCoverSubsystem::GetCurrentCoverQuality(const FSmartObjectSlotHandle& OccupiedSlot,
                                                     TConstArrayView<FCoverThreatInfo> Threats,
                                                     bool bStanding) const
{
	const FSoldierCoverRuntimeInfo* Info = FindCoverInfo(OccupiedSlot);
	if (Info == nullptr || !Info->IsValid())
	{
		return 0.0f;	// 슬롯 밖 = 엄폐하지 않은 것으로 친다
	}

	return 1.0f - ComputeExposure(*Info, Threats, bStanding);
}

// ---------------------------------------------------------------------------
// 최근 점유 이력
// ---------------------------------------------------------------------------

void USoldierCoverSubsystem::NotifySlotOccupied(const AActor* Soldier, const FSmartObjectSlotHandle& SlotHandle)
{
	if (Soldier == nullptr || GetWorld() == nullptr)
	{
		return;
	}

	TArray<FRecentOccupancy>& History = RecentOccupancy.FindOrAdd(Soldier);

	FRecentOccupancy Entry;
	Entry.SlotHandle  = SlotHandle;
	Entry.TimeSeconds = GetWorld()->GetTimeSeconds();

	History.Insert(MoveTemp(Entry), 0);
	if (History.Num() > RecencyHistoryDepth)
	{
		History.SetNum(RecencyHistoryDepth, EAllowShrinking::No);
	}
}

float USoldierCoverSubsystem::GetRecencyScore(const AActor* Soldier, const FSmartObjectSlotHandle& SlotHandle) const
{
	if (Soldier == nullptr || GetWorld() == nullptr)
	{
		return 1.0f;
	}

	const TArray<FRecentOccupancy>* History = RecentOccupancy.Find(Soldier);
	if (History == nullptr)
	{
		return 1.0f;
	}

	const double Now = GetWorld()->GetTimeSeconds();
	for (const FRecentOccupancy& Entry : *History)
	{
		if (Entry.SlotHandle == SlotHandle)
		{
			const float Elapsed = static_cast<float>(Now - Entry.TimeSeconds);
			if (Elapsed >= RecencyWindowSeconds)
			{
				return 1.0f;
			}
			// 방금 있던 자리 = 0, 창(窓) 끝 = 1 로 선형 회복.
			return FMath::Clamp(Elapsed / FMath::Max(RecencyWindowSeconds, KINDA_SMALL_NUMBER), 0.0f, 1.0f);
		}
	}

	return 1.0f;
}

// ---------------------------------------------------------------------------
// 질의 동시 상한
// ---------------------------------------------------------------------------

bool USoldierCoverSubsystem::TryAcquireQueryToken(const AActor* Soldier)
{
	if (Soldier == nullptr)
	{
		return false;
	}

	// 죽은 참조 정리
	for (auto It = InFlightQueries.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrent();
		}
	}

	if (InFlightQueries.Contains(Soldier))
	{
		return true;	// 이미 보유
	}
	if (InFlightQueries.Num() >= MaxConcurrentQueries)
	{
		return false;
	}

	InFlightQueries.Add(Soldier);
	return true;
}

void USoldierCoverSubsystem::ReleaseQueryToken(const AActor* Soldier)
{
	InFlightQueries.Remove(Soldier);
}
