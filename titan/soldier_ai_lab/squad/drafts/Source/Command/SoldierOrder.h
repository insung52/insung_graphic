// SoldierLab — L0 COMMAND
// 초안 (2026-09-09). 컴파일 검증 없음.
//
// 근거: design/2026-09-01_architecture.md 11.2 / 11.5절.
//       스키마는 설계 문서의 것을 그대로 옮겼다. 필드를 바꾸지 말 것 —
//       바꾸면 상위계획 8절의 계층 계약이 깨진다.
//
// 배치 예정: Source/SoldierLab/Command/SoldierOrder.h
// Build.cs 추가 의존: "GameplayTags"

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SoldierOrder.generated.h"

class USquadComponent;

/** 설계 11.2절. 동사 × 대상 × 제약의 조합으로 표현을 폭발시킨다 — enum을 늘리지 않는다. */
UENUM(BlueprintType)
enum class EOrderVerb : uint8
{
	None,
	MoveTo,     // 지정 위치/구역으로 이동 (교전은 부차적)
	Occupy,     // 구역을 점령·확보하고 방어 태세
	Engage,     // 지정 대상/구역과 교전 개시
	Suppress,   // 지정 구역/대상을 제압 (명중보다 억제 우선)
	Advance,    // 교전하면서 전진
	Withdraw,   // 이탈 — 교대 엄호 포함
	HoldFire,   // ★ 교전규칙만 바꾼다. 플랜을 바꾸지 않는다 (PLAN.md 3.3절)
	Regroup,    // 집결
	Overwatch,  // 지정 방향/구역 감시, 접촉 시 교전
	Follow      // 지정 액터 동행
};

UENUM(BlueprintType)
enum class EOrderRecipientType : uint8
{
	Squad,
	Element,      // 분대 내 조 (PLAN.md 2.3절)
	Individual,   // 의미는 설계 백로그 D7에서 확정
	AllOfFaction
};

UENUM(BlueprintType)
enum class EOrderTargetType : uint8
{
	None,
	Location,
	Actor,
	Zone,
	Direction
};

UENUM(BlueprintType)
enum class EEngagementRule : uint8
{
	Free,
	ReturnFireOnly,
	HoldFire,
	PositiveIDRequired
};

UENUM(BlueprintType)
enum class EFormationType : uint8
{
	Column,
	Wedge,
	Line,
	Diamond
};

UENUM(BlueprintType)
enum class EOrderStatus : uint8
{
	None,
	Received,
	InProgress,
	Achieved,
	Failed,
	Aborted
};

/** 설계 11.2절 — 목표는 4가지 중 하나. */
USTRUCT(BlueprintType)
struct FOrderTarget
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order")
	EOrderTargetType Type = EOrderTargetType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order")
	TWeakObjectPtr<AActor> Actor;

	/** 레벨의 전투구역 볼륨 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order")
	FName ZoneId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order")
	FVector Direction = FVector::ForwardVector;
};

/**
 * 설계 11.2절 — "어떻게"가 아니라 "어떤 범위 안에서".
 * 여기에 "어떻게"에 해당하는 필드를 추가하려는 순간이 오면 설계 2.2절 제2규칙 위반이다.
 */
USTRUCT(BlueprintType)
struct FOrderConstraints
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order")
	EEngagementRule ROE = EEngagementRule::Free;

	/** 0 = 최대한 안전하게, 1 = 속도 우선. 설계 10.4절 EQS 가중치에 직접 영향 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Aggression = 0.5f;

	/** 이 이하로는 사기가 안 무너진다. 설계 9.4절 "결사 항전"의 파라미터화 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MoraleFloor = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order")
	EFormationType Formation = EFormationType::Wedge;

	/** 이동 회랑. 비우면 자유 (PLAN.md 4.2절: 저작 회랑이 런타임 생성을 이긴다) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order")
	FName CorridorId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order")
	float StandoffDistance = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order")
	bool bAllowFlanking = true;
};

/** 설계 11.2절. */
USTRUCT(BlueprintType)
struct FSoldierOrder
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order")
	FGuid OrderId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order")
	EOrderRecipientType RecipientType = EOrderRecipientType::Squad;

	/** SquadId / ElementId / 액터 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order")
	FName RecipientId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order")
	EOrderVerb Verb = EOrderVerb::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order")
	FOrderTarget Target;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order")
	FOrderConstraints Constraints;

	/** 높을수록 기존 명령을 덮어쓴다 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order")
	int32 Priority = 0;

	/** 0 = 무기한 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order")
	float ExpiresAfterSec = 0.f;

	/** 시나리오 스텝 추적용(디버깅) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order")
	FName SourceStepId = NAME_None;

	bool IsValidOrder() const { return Verb != EOrderVerb::None; }
};

/** 설계 11.5절 — 역방향 피드백. 이게 없으면 시나리오를 못 짠다. */
USTRUCT(BlueprintType)
struct FOrderStatusReport
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Order")
	FGuid OrderId;

	UPROPERTY(BlueprintReadOnly, Category = "Order")
	EOrderStatus Status = EOrderStatus::None;

	/** 0~1. Verb별 술어로 계산 (PLAN.md 3.5절) */
	UPROPERTY(BlueprintReadOnly, Category = "Order")
	float Progress = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Order")
	int32 Casualties = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Order")
	float SquadMorale = 1.f;

	UPROPERTY(BlueprintReadOnly, Category = "Order")
	FVector SquadCentroid = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Order")
	int32 ConfirmedEnemyCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Order")
	FName SquadId = NAME_None;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOrderStatusReported, const FOrderStatusReport&, Report);

/**
 * 설계 11.3절 — 명령의 수신·우선순위·만료 관리와 배달.
 *
 * 서버 전용 (설계 4.3.2절). 모든 진입점에 권위 게이트 (CLAUDE.md P5).
 */
UCLASS()
class SOLDIERLAB_API USoldierCommandSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** 명령 발행. 서버에서만 유효. 수신자 해석 후 해당 분대(들)에 배달한다. */
	UFUNCTION(BlueprintCallable, Category = "SoldierLab|Command")
	bool IssueOrder(const FSoldierOrder& Order);

	UFUNCTION(BlueprintCallable, Category = "SoldierLab|Command")
	bool CancelOrder(const FGuid& OrderId);

	/** 분대가 진척을 보고하는 창구. 상태 변화 시 + 2초 주기 (PLAN.md 3.5절) */
	void ReportOrderStatus(const FOrderStatusReport& Report);

	UPROPERTY(BlueprintAssignable, Category = "SoldierLab|Command")
	FOnOrderStatusReported OnOrderStatusReported;

	/** 만료 처리. USquadSubsystem 틱에서 호출한다(전용 틱을 하나 더 두지 않는다). */
	void PruneExpiredOrders(float NowSeconds);

private:
	struct FActiveOrder
	{
		FSoldierOrder Order;
		float IssuedAtSeconds = 0.f;
	};

	/** 활성 명령. 분대 수 × 소수라 배열로 충분하다. */
	TArray<FActiveOrder> ActiveOrders;

	/** 마지막 보고 캐시 — L0가 폴링할 수 있게 */
	TMap<FGuid, FOrderStatusReport> LastReports;
};
