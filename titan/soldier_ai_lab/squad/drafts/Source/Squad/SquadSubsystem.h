// SoldierLab — 분대 등록·조회·틱 스케줄러
// 초안 (2026-09-09). 컴파일 검증 없음.
//
// 근거: design/2026-09-01_architecture.md 12.1~12.2절 (45명 / 분대 조율 0.1ms / 2~4Hz)
//       PLAN.md 7절
//
// 배치 예정: Source/SoldierLab/Squad/SquadSubsystem.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SquadTypes.h"
#include "SquadSubsystem.generated.h"

class USquadComponent;
class ASquadCoordinator;
class USquadTuningData;
struct FSquadDefinition;

/**
 * 분대 스케줄러.
 *
 * ★ 예산 산술 (PLAN.md 7.1~7.2절):
 *     45명 / 4~6명 = 8~11 분대. 11분대 × 3Hz = 33 분대틱/초.
 *     60fps 이면 프레임당 0.55회. 예산 0.1ms 는 곧 "분대 틱 1회당 0.18ms 상한"이다.
 *
 * ★ MaxSquadsPerFrame = 1 이 스파이크 방어선이다.
 *   이게 없으면 힛치 복귀 프레임에서 11개 분대가 한꺼번에 돈다.
 *
 * ★ 밀린 분대 틱은 몰아치지 않고 버린다.
 *   분대 결정은 2~4Hz 면 충분하므로 한두 번 건너뛰어도 무해하다.
 */
UCLASS()
class SOLDIERLAB_API USquadSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	// --- UWorldSubsystem ---
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- FTickableGameObject ---
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;

	// --- 분대 관리 ---

	/** 서버에서만 유효. ASquadCoordinator 를 스폰하고 편제를 채운다 */
	UFUNCTION(BlueprintCallable, Category = "SoldierLab|Squad")
	USquadComponent* CreateSquad(const FSquadDefinition& Definition);

	UFUNCTION(BlueprintPure, Category = "SoldierLab|Squad")
	USquadComponent* FindSquad(FName SquadId) const;

	/**
	 * 액터 → 분대 역인덱스.
	 * 개인이 자기 분대를 찾는 유일한 경로. StateTree Evaluator 가 이것만 쓴다
	 * (태스크마다 각자 찾으면 룩업이 태스크 수만큼 늘고, 같은 틱 안에서 배정이
	 *  달라 보이는 사고가 난다 — PLAN.md 8.2절).
	 */
	UFUNCTION(BlueprintPure, Category = "SoldierLab|Squad")
	bool FindSquadHandleFor(const AActor* Pawn, FSquadHandle& OutHandle) const;

	USquadComponent* GetSquadForActor(const AActor* Pawn) const;

	void RegisterMemberLookup(const AActor* Pawn, const FSquadHandle& Handle);
	void UnregisterMemberLookup(const AActor* Pawn);

	/** 디버그 HUD 용 */
	const TArray<TObjectPtr<USquadComponent>>& GetAllSquads() const { return Squads; }

private:
	UPROPERTY()
	TArray<TObjectPtr<USquadComponent>> Squads;

	UPROPERTY()
	TMap<TWeakObjectPtr<const AActor>, FSquadHandle> MemberLookup;

	UPROPERTY()
	TMap<FName, TObjectPtr<USquadComponent>> SquadsById;

	/** 라운드로빈 커서 — 분대 간 스태거링이 공짜로 나온다 */
	int32 Cursor = 0;

	/** 누적 틱 크레딧. MaxAccumulator 로 클램프해 몰아치기를 막는다 */
	float Accumulator = 0.f;

	static constexpr float MaxAccumulator = 1.f;

	/** 튜닝 에셋이 없을 때의 폴백. 정상 경로에서는 분대별 USquadTuningData 를 쓴다 */
	static constexpr float FallbackSquadTickHz = 3.f;
	static constexpr int32 FallbackMaxSquadsPerFrame = 1;
};
