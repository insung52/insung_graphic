// SoldierLab — L1 SQUAD 공통 타입
// 초안 (2026-09-09). 컴파일 검증 없음. UE5.8 가정.
//
// 근거: design/2026-09-01_architecture.md 9절 (9.1~9.5)
//       ai/2026-09-02_upper_layer_plan.md 6·8절
//       PLAN.md (같은 폴더)
//
// 배치 예정: Source/SoldierLab/Squad/SquadTypes.h   (설계 3.7절 모듈 레이아웃)
//
// Build.cs 에 추가해야 하는 의존 (PublicDependencyModuleNames):
//   "GameplayTags", "AIModule", "NavigationSystem", "StateTreeModule",
//   "GameplayStateTreeModule", "SmartObjectsModule"
//   ※ FInstancedStruct 등은 UE5.5+ 에서 CoreUObject 로 이동했다. 5.8에서 재확인할 것.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "SquadTypes.generated.h"

/** 분대 최대 인원. 비트마스크(uint32)와 고정 배열 상한의 근거. */
#define SOLDIERLAB_MAX_SQUAD_MEMBERS 12

// ---------------------------------------------------------------------------
// 열거형
// ---------------------------------------------------------------------------

/**
 * 편제 역할 — 정적. 스폰 시 데이터로 정해지고 분대장 승계 때만 바뀐다.
 * PLAN.md 2.2절. 역할은 "가중치"로만 작동한다 — 역할별 코드 분기를 만들지 말 것
 * (설계 3.7.1절 / CLAUDE.md P4와 같은 논리).
 */
UENUM(BlueprintType)
enum class ESquadRole : uint8
{
	Rifleman,
	Leader,
	AutomaticRifleman,   // 지원화기 — 제압 토큰 우선
	Grenadier,
	Marksman,            // 저격 — Overwatch 배역 선호
	Medic
};

/**
 * 전술 배역 — 동적. 플랜 전환·토큰 회수 때마다 바뀐다.
 * PLAN.md 2.2절. 설계 9.2절 "역할 배분"이 가리키던 것이 이쪽이다.
 */
UENUM(BlueprintType)
enum class ESquadTask : uint8
{
	None,
	Overwatch,    // 정지·감시·엄호사격
	Bound,        // 전진 (이동 토큰 보유 조)
	Suppress,     // 고정·제압
	Maneuver,     // 측면기동 (회랑 내)
	RearGuard,    // 후위 엄호
	Withdraw,     // 이탈
	Regroup
};

/** 설계 9.2절 6종 + FormationMove 1종 (PLAN.md 3.3절). */
UENUM(BlueprintType)
enum class ESquadPlan : uint8
{
	None,
	Hold,
	BaseOfFire,
	BoundingOverwatch,
	Flank,
	Withdraw,
	Regroup,
	FormationMove
};

UENUM(BlueprintType)
enum class ESquadTokenType : uint8
{
	Suppression,
	Movement
};

/** 설계 6.5.2절 EHealthState 와 대응. L1은 이 4단계만 알면 된다. */
UENUM(BlueprintType)
enum class ESquadMemberHealth : uint8
{
	Healthy,
	Injured,
	Downed,
	Dead
};

// ---------------------------------------------------------------------------
// 핸들
// ---------------------------------------------------------------------------

/**
 * 개인이 캐시하는 자기 분대 좌표.
 * ★ MemberIndex 는 재사용하지 않는다 (PLAN.md 2.1절) — 전사자는 배열에서 제거하지 않고
 *   HealthState = Dead 로 표시만 한다. 제거하면 인덱스가 밀려 다른 사람을 가리킨다.
 */
USTRUCT(BlueprintType)
struct FSquadHandle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	FName SquadId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	int32 MemberIndex = INDEX_NONE;

	bool IsValid() const { return SquadId != NAME_None && MemberIndex != INDEX_NONE; }
};

// ---------------------------------------------------------------------------
// L2 → L1 : 개인 상태 (상위계획 8절 FSoldierStatus)
// ---------------------------------------------------------------------------

/**
 * 분대가 알아야 하는 개인 상태의 전부.
 *
 * ★ pull 이다 (PLAN.md 7.3절) — 분대 틱이 3Hz로 읽어간다. 개인이 push 하지 않는다.
 *   push 로 만들면 45명이 상태를 바꿀 때마다 분대가 깨어난다.
 * ★ 여기 있는 값은 전부 "개인이 이미 계산해 둔 캐시된 스칼라"여야 한다.
 *   getter 안에서 트레이스나 질의를 돌리면 분대 예산이 그 자리에서 터진다.
 */
USTRUCT(BlueprintType)
struct FSoldierSquadStatus
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	ESquadMemberHealth Health = ESquadMemberHealth::Healthy;

	/** 0~1. 설계 7.5절 Exposure — 인지와 유틸리티가 공유하는 그 값 */
	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	float Exposure = 0.f;

	/** 0~1. 탄창 잔량 */
	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	float AmmoRatio = 1.f;

	/** 0~1. 피제압도 (설계 9.4절 사기 공식의 입력) */
	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	float SuppressionLevel = 0.f;

	/** L2가 지금 고른 Intent. 사기·EngagedCount 집계에 쓴다 */
	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	FGameplayTag CurrentIntent;

	/** 지금 교전 중인 표적 — 표적 과집중 억제(PLAN.md 6.2절)의 집계 대상 */
	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	TWeakObjectPtr<AActor> CurrentTarget;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	bool bIsInCover = false;
};

/** 병사가 구현하는 인터페이스. 분대는 이것만 알고 병사 클래스를 모른다. */
UINTERFACE(MinimalAPI, BlueprintType)
class USquadMemberInterface : public UInterface
{
	GENERATED_BODY()
};

class ISquadMemberInterface
{
	GENERATED_BODY()

public:
	/** 캐시된 값을 반환하기만 할 것. 여기서 계산하지 말 것. */
	virtual void GetSquadStatus(FSoldierSquadStatus& OutStatus) const = 0;

	/** 분대가 배정을 갱신했음을 통보(선택). 개인은 폴링해도 된다. */
	virtual void OnSquadAssignmentChanged(uint32 NewRevision) {}
};

// ---------------------------------------------------------------------------
// 정보 공유 (PLAN.md 5절)
// ---------------------------------------------------------------------------

/**
 * 분대가 공유하는 위협 1건.
 *
 * ★ LocationSigmaCm 이 이 구조체의 핵심이다 (PLAN.md 5.2절).
 *   수신자는 좌표가 아니라 (추정 위치, 불확실성 반경)을 받는다.
 *   σ 가 임계를 넘으면 정조준 대상이 될 수 없다 — 이것이 "치팅처럼 보이지 않게"의 실체다.
 */
USTRUCT(BlueprintType)
struct FSharedThreat
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	TWeakObjectPtr<AActor> Target;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	FVector ReportedLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	FVector ReportedVelocity = FVector::ZeroVector;

	/** ★ 위치 불확실성 반경(cm). 0 이면 "직접 목격"이라는 뜻이므로 공유본에는 0을 넣지 말 것 */
	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	float LocationSigmaCm = 0.f;

	/** 목격 시각(월드 초). 배달 시각이 아니다 — 병합 규칙 R2가 이 값을 본다 */
	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	float ObservedAtSeconds = 0.f;

	/** 설계 9.3절: 공유본은 0.6 상한 */
	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	float Confidence = 0.6f;

	/** 나에게 얼마나 위험한가 (설계 7.4절). 발신자 기준값을 그대로 옮긴다 */
	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	float ThreatLevel = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	int32 ReporterMemberIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	bool bReporterIsLeader = false;
};

/** 지연 배달 1건. 수신자마다 지연이 다르다 — 동시에 반응하면 그 자체가 치팅 신호다. */
USTRUCT()
struct FPendingContactDelivery
{
	GENERATED_BODY()

	UPROPERTY()
	FSharedThreat Payload;

	UPROPERTY()
	int32 RecipientMemberIndex = INDEX_NONE;

	UPROPERTY()
	float DeliverAtSeconds = 0.f;
};

/**
 * 공유 위협을 받는 쪽(개인 기억)이 구현하는 인터페이스.
 *
 * ★ L1은 병합하지 않는다 (PLAN.md 5.3절). 배달만 하고, 병합은 수신자가 판정한다 —
 *   내 기억을 어떻게 갱신할지는 나만 아는 정보이기 때문(설계 2.1절 제1규칙).
 *
 * 구현 측이 지켜야 하는 규칙 3개 → 인지 담당자와 합의 필요 [Q-12]:
 *   R1. 직접 목격(bSharedBySquad==false)이 더 최신이면 이 보고를 버린다. 절대 덮어쓰지 않는다
 *   R2. 이미 공유로 알던 것보다 오래된(ObservedAtSeconds) 보고는 버린다
 *   R3. 병합 결과 Confidence 는 min(Report.Confidence, 0.6) 을 넘지 않는다 (설계 9.3절)
 */
UINTERFACE(MinimalAPI, BlueprintType)
class USoldierThreatMemorySink : public UInterface
{
	GENERATED_BODY()
};

class ISoldierThreatMemorySink
{
	GENERATED_BODY()

public:
	virtual void ReceiveSharedThreat(const FSharedThreat& Report) = 0;
};

// ---------------------------------------------------------------------------
// 자원 중재 (PLAN.md 6절)
// ---------------------------------------------------------------------------

/**
 * 엄폐 슬롯 사전 예고.
 *
 * ★ 최적화이지 정확성 장치가 아니다 (PLAN.md 6.1절).
 *   중복 점유를 막는 것은 SmartObject 예약(설계 10.3절)이고, soft-claim 은
 *   "둘이 같은 슬롯으로 개활지를 가로질러 뛰는" 낭비만 줄인다.
 *   실패해도 시스템은 옳게 동작해야 한다.
 */
USTRUCT()
struct FSlotSoftClaim
{
	GENERATED_BODY()

	/** SmartObject 슬롯 식별자. 실제 타입은 엄폐 담당자의 스키마에 맞춘다 [Q-12 인접] */
	UPROPERTY()
	FGuid SlotId;

	UPROPERTY()
	int32 MemberIndex = INDEX_NONE;

	UPROPERTY()
	float Score = 0.f;

	UPROPERTY()
	float ExpiresAtSeconds = 0.f;
};

/** 제압 사격선. 기동조 경로의 "비용"이지 "금지선"이 아니다 (PLAN.md 6.4절). */
USTRUCT()
struct FSquadFireLane
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Origin = FVector::ZeroVector;

	UPROPERTY()
	FVector Direction = FVector::ForwardVector;

	UPROPERTY()
	float HalfWidthCm = 150.f;

	UPROPERTY()
	float LengthCm = 5000.f;
};

/** 토큰 1개의 보유 기록. */
USTRUCT()
struct FSquadTokenGrant
{
	GENERATED_BODY()

	UPROPERTY()
	ESquadTokenType Type = ESquadTokenType::Movement;

	/** 개인 단위 발급이면 멤버 인덱스, 조 단위면 INDEX_NONE */
	UPROPERTY()
	int32 MemberIndex = INDEX_NONE;

	/** 조 단위 발급이면 조 이름, 개인 단위면 NAME_None (PLAN.md 4.1절) */
	UPROPERTY()
	FName ElementId = NAME_None;

	UPROPERTY()
	float GrantedAtSeconds = 0.f;
};

// ---------------------------------------------------------------------------
// 편제
// ---------------------------------------------------------------------------

/**
 * 조(Element). 바운딩 오버워치·측면기동·교대 후퇴는 전부 "두 덩어리"를 요구한다.
 * PLAN.md 2.3절. 설계 11.2절 EOrderRecipientType::Element 의 L1 대응물.
 */
USTRUCT(BlueprintType)
struct FSquadElement
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	FName ElementId = NAME_None;

	/** 멤버 인덱스 비트마스크 (최대 12명이므로 uint32 로 충분) */
	UPROPERTY()
	uint32 MemberMask = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	ESquadTask Task = ESquadTask::None;

	/** 이 조의 공간 기준점 — bound line / 경계 구역 중심 / 집결점 */
	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	FVector Anchor = FVector::ZeroVector;

	bool Contains(int32 MemberIndex) const
	{
		return MemberIndex >= 0 && MemberIndex < 32 && (MemberMask & (1u << MemberIndex)) != 0;
	}
};

USTRUCT(BlueprintType)
struct FSquadMemberEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	TWeakObjectPtr<AActor> Pawn;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	ESquadRole Role = ESquadRole::Rifleman;

	/** 분대장 승계 순위. 낮을수록 먼저 (설계 9.5절 "사망 시 차순위 승계") */
	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	int32 SuccessionOrder = 100;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	FSoldierSquadStatus Status;

	UPROPERTY()
	float StatusUpdatedAtSeconds = 0.f;

	/** 토큰 요청 대기 시작 시각 — starvation 방지 보너스의 기준 (PLAN.md 6.3절) */
	UPROPERTY()
	float TokenRequestedAtSeconds = -1.f;

	UPROPERTY()
	ESquadTokenType RequestedTokenType = ESquadTokenType::Movement;

	/** 접촉 보고 쿨다운 — 표적별 마지막 보고 시각 */
	TMap<TWeakObjectPtr<AActor>, float> LastReportTimeByTarget;

	bool IsAlive() const
	{
		return Pawn.IsValid()
			&& Status.Health != ESquadMemberHealth::Dead
			&& Status.Health != ESquadMemberHealth::Downed;
	}
};

// ---------------------------------------------------------------------------
// L1 → L2 : 배정 (상위계획 8절 FSquadAssignment)
// ---------------------------------------------------------------------------

/**
 * ★ 이 구조체의 모든 필드는 "제약"이어야 한다. "명령"을 넣으면 안 된다 (설계 2.2절 제2규칙).
 *
 *   허용: 너는 엄호 쪽이다 / 이 영역 밖으로 나가지 마라 / 이 방향을 맡아라 / 토큰이 없다
 *   금지: 저 적을 쏴라 / 지금 쏴라 / 정확히 이 좌표에 서라
 *
 *   필드를 추가할 때는 PLAN.md 3.2절 표에 "제약인가 명령인가"를 함께 채울 것.
 *   이 규칙을 놓치면 titan_example 의 원격조종 상태로 돌아간다.
 */
USTRUCT(BlueprintType)
struct FSquadAssignment
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	FName SquadId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	int32 MemberIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	ESquadPlan Plan = ESquadPlan::None;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	ESquadRole Role = ESquadRole::Rifleman;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	FName ElementId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	ESquadTask Task = ESquadTask::None;

	/** 이동 허용 영역. IsValid()==false 면 자유 */
	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	FBox Corridor = FBox(ForceInit);

	/** 배역의 공간 기준점. "여기로 가라"가 아니라 "이 근처가 네 자리다" */
	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	FVector Anchor = FVector::ZeroVector;

	/** 담당 부채꼴 (X=최소 yaw, Y=최대 yaw, 월드 도) — 설계 9.2절 Hold 의 "부채꼴 시야 분담" */
	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	FVector2D SectorYawRange = FVector2D(-180.f, 180.f);

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	bool bHasSuppressionToken = false;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	bool bHasMovementToken = false;

	/** 권고일 뿐 강제가 아니다 (PLAN.md 4.3절) */
	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	TWeakObjectPtr<AActor> FocusTarget;

	/** 명령 Aggression × 플랜 보정. EQS 가중치(설계 10.4)와 개인 축의 입력 */
	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	float PlanAggression = 0.5f;

	/** 변경 감지. 값이 같으면 StateTree Evaluator 가 복사를 건너뛴다 */
	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	uint32 Revision = 0;
};

// ---------------------------------------------------------------------------
// 분대 blackboard (설계 9.3절 + PLAN.md 2.4절 추가 필드)
// ---------------------------------------------------------------------------

USTRUCT()
struct FSquadBlackboard
{
	GENERATED_BODY()

	// --- 설계 9.3절 원본 ---

	UPROPERTY()
	TArray<FSharedThreat> KnownThreats;      // 상한 8 (PLAN.md 2.4절)

	UPROPERTY()
	float Morale = 1.f;                      // 0~1 (설계 9.4절)

	UPROPERTY()
	int32 SuppressionTokens = 0;

	UPROPERTY()
	int32 MovementTokens = 0;

	UPROPERTY()
	FVector FriendlyCentroid = FVector::ZeroVector;

	UPROPERTY()
	FVector EnemyCentroid = FVector::ZeroVector;

	UPROPERTY()
	FBox AssignedCorridor = FBox(ForceInit);

	UPROPERTY()
	ESquadPlan CurrentPlan = ESquadPlan::None;

	// --- PLAN.md 2.4절 추가 ---

	UPROPERTY()
	TArray<FSquadElement> Elements;          // 항상 2개 (PLAN.md 2.3절)

	UPROPERTY()
	TArray<FPendingContactDelivery> ReportQueue;   // 상한 32

	UPROPERTY()
	TArray<FSlotSoftClaim> SlotClaims;       // 멤버당 1개

	UPROPERTY()
	TArray<FSquadFireLane> ActiveFireLanes;  // 상한 = SuppressionTokens

	UPROPERTY()
	TArray<FSquadTokenGrant> Grants;

	UPROPERTY()
	FVector RallyPoint = FVector::ZeroVector;

	/** 생존자 centroid 로부터의 RMS 거리(cm) */
	UPROPERTY()
	float Dispersion = 0.f;

	/** 표적별 교전 인원 수 — 과집중 억제 (PLAN.md 6.2절) */
	TMap<TWeakObjectPtr<AActor>, uint8> TargetEngagedCount;
};
