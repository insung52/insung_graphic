# 적군 미끄러짐 / 배속 / 사격 정지 — 원인 3건 규명

2026-09-11 / 완료 / 사격→엄폐 이동 시 애니메이션 없이 미끄러지던 원인은 **`FireRecoil` 슬롯이 AnimGraph 메인 사슬에 전신으로 물려 있던 것**. 배속은 `IsSprinting` 오판정, 그 수정의 부작용으로 드러난 사격 정지는 `PoseArrivalToleranceCm` 10cm가 너무 빡빡했던 것.

선행 문서: `2026-09-11_ally_skeleton_migration_prep.md`(같은 날 스켈레톤 이관).

---

## 1. 증상

- **사격 자세(서서 조준)에서 엄폐 위치로 출발할 때, 약 0.5초간 그 포즈 그대로 아무 이동 애니메이션 없이 미끄러진다.** 0.5초 뒤 앉으면서 그제야 정상 이동.
- **엄폐 → 사격 방향은 정상.** 서서 제대로 걸어간다.
- **아군은 완전히 정상.** 자세 구성이 같은 개체(`C_1`: 서서 사격 → 숙여 엄폐)도 문제없음.

## 2. 원인 — `FireRecoil` 슬롯이 전신

`Slot_1(FireRecoil)`이 AnimGraph 메인 포즈 사슬 한가운데에 **감싸는 블렌드 없이** 직결돼 있었다.

```
LayeredBoneBlend_4 → Slot_1(FireRecoil) → LocalToComponentSpace_1 → TwoBoneIK → Root
```

몽타주가 재생 중이면 스테이트머신 출력이 **통째로** 무시된다. 그리고 `Firing` 상태는 버스트
마지막 탄을 쏘자마자 즉시 엄폐로 전환하는데, 반동 몽타주(`AS_Enemy/Firing_Rifle_Anim_2`,
**1.167초**)는 한참 남아 있다. 그 잔여 시간 동안 "서서 조준한 사격 포즈"가 이동 포즈를 덮었다.

**결정적 증거** — 미끄러지는 구간의 로그:

```
자세=3(엄폐로 이동)  상태=AimLocomotion  Speed=600  Kneel=Y
  ||  슬롯 Fire=1.00  몽타주=AnimMontage_59
```

스테이트머신은 `Speed=600`짜리 이동 포즈를 **정상 생성 중**이었고, 슬롯 가중치가 1.00으로
그 위를 100% 덮고 있었다.

> **같은 그래프의 `ReloadSlot`은 원래부터 제대로 돼 있었다** — `Slot_2 → LayeredBoneBlend_1`
> (`Spine2` 브랜치 필터, 가중치 1.0)로 상체에만 얹힌다. **사격 슬롯만 그 블렌드가 빠져 있었다.**

### 2.1 수정

`ReloadSlot`과 동일한 구조로 맞췄다. 반동을 중간에 끊지 않으면서 다리는 이동 애니메이션을 그린다.

```
LayeredBoneBlend_4 ──┬──> Slot_1 (FireRecoil)
                     └──> [신규 LayeredBoneBlend].BasePose
Slot_1 ──> [신규].BlendPoses_0   (가중치 1.0, 브랜치 필터 / 깊이 0)
[신규] ──> LocalToComponentSpace_1
```

| ABP | 신규 노드 | 브랜치 필터 |
|---|---|---|
| `ABP_Enemy_kadex2_New` | `LayeredBoneBlend_8` | `Spine1` |
| `ABP_Ally_kadex_T` | `LayeredBoneBlend_0` | `spine_03` |

필터 본이 다른 건 스켈레톤이 달라서다(적군 Mixamo / 아군 UE5 마네킹). 반동이 과하면 한 칸 위
(`Spine2` / `spine_04`), 약하면 한 칸 아래로 조정하면 된다.

> 아군도 구조는 동일했다 — 증상이 눈에 안 띄었을 뿐이라 같이 고쳤다.

## 3. 배속 — `IsSprinting` 오판정

이동은 200~300인데 발이 2배로 빨랐다.

`ApplyGaitForDesiredSpeed`가 `bSprinting = DesiredSpeed > WalkTop`으로 판정하는데,
`PoseMoveSpeed = 300` > `BaseWalkSpeed/CrouchWalkSpeed = 200`이라 **자세 전환 이동이 전부
"뛰기"로 잡혔다.** ABP `Speed`가 뛰기 앵커 600에 고정되고, 600용 뛰기 애니메이션이 300 이동에
얹혀 2배로 보였다.

**수정: `PoseMoveSpeed` 300 → 200.** `Sprint=N` → `Speed=300`(걷기 앵커) → 실제 이동 200과 일치.
아군이 쓰는 구성과 같아졌다.

## 4. 부작용 — 사격 정지, 그리고 `PoseArrivalToleranceCm`

3번 수정 직후 적군이 **타겟을 쳐다본 채 한 발도 안 쏘고** 5~6초 뒤 엄폐로 돌아갔다.

상시 활성인 `FiringStuckGraceSeconds` 경고가 답을 그대로 찍어줬다.

```
Firing 상태에서 5.4초간 빠져나오지 못해 강제로 엄폐 복귀
  — 남은탄=0 버스트트리거=0 마커도착=0 조준정렬=1 타겟=BP_Ally_kadex_C_3
```

`bAtCombatPoseMarker = 0`. 사격 게이트가 이걸 요구한다.

```cpp
bCoverAllowsFire = bAtCombatPoseMarker && IsFacingSettled(...) && !bFiringBurstTriggered && !bLaneBlocked;
```

**`PoseArrivalToleranceCm`이 10cm로 너무 빡빡했다.** `PoseMoveSpeed`를 200으로 낮추자 마지막
접근이 느려져 10cm 안으로 들어가기 전에 멈춰버렸다(300일 때는 관성으로 들어갔다).

> 함정: `MoveToward`가 재는 건 **내비 경로 웨이포인트까지 거리**이고 `bAtCombatPoseMarker`는
> **마커까지의 직선거리**다. `SmoothedMaxWalkSpeed`가 0이 됐다고 해서 도착 판정이 떨어진 게 아니다
> (실제로 이걸 근거로 도착했다고 오판했다).

**수정: `PoseArrivalToleranceCm` 10 → 50.** 이 값은 정지 거리로도 같이 쓰이므로 마커 50cm 앞에
선다. 엄폐/사격 마커 간격이 1~1.5m라 문제없다.

## 5. 최종 값 (`test_soldiers_2` 적군 3명)

| 값 | 최종 | 비고 |
|---|---|---|
| `PoseMoveSpeed` | **200** | 걷기 앵커와 일치 |
| `PoseArrivalToleranceCm` | **50** | 10은 너무 빡빡 |
| `PoseApproachSlowdownDistanceCm` | 60 | 기본값 |
| `MaxWalkSpeedAccelerationCmPerSec2` | 800 | 기본값 |
| `FacingTurnRateDegPerSec` | 360 | 기본값 |

## 6. 폐기한 가설들 (재조사 방지)

여러 번 틀렸다. 같은 길을 다시 파지 않도록 남긴다.

| 가설 | 폐기 근거 |
|---|---|
| 가속 램프(`MaxWalkSpeedAccelerationCmPerSec2`) | 로그상 `Speed`가 0.13초 만에 600 도달 |
| `GaitTopSpeed`가 고정 gait top이라서 | 고쳤는데 증상 그대로 |
| 스테이트머신이 `Knee`로 못 넘어감 | `AimLocomotion`도 `Speed`를 받으므로 애니메이션은 나와야 함 |
| 숙임 블렌드스페이스 대각선 샘플 부족 | 미끄러지는 구간은 **서 있는** 상태 |
| `BS_enemy_kadex` 미리빌드 | 엄폐→사격에선 서서 정상 이동 |
| 자세 전환(Standing↔Crouched) 자체 | 아군 `C_1`도 같은 구성인데 정상 |
| 타겟 상실 / 재장전 | 로그 전 구간 `HasTarget=Y 재장전=N` |

> **가속도 3000으로 올렸을 때 미끄러짐이 사라진 건 착시였다.** 그 설정에선 적군이 사격을 못 해서
> 반동 몽타주가 아예 없었고, 그래서 `Firing→엄폐` 구간 자체가 안 생겼다.

## 7. 진단 로그

`EnemyCombatComponent.cpp`에 두 블록이 `#if 0`으로 남아 있다. 되살리면 프레임 단위 추적이 된다.

- **`[속도진단]`** (0.05초 주기) — 자세/Kneel/Hold/HasTarget/타겟/재장전, 실제속도·MaxWalkSpeed·
  Smoothed·GaitTop·Sprint, **ABP 실제값**(스테이트 이름/Speed/Dir/Kneel), **슬롯 가중치**
  (Fire/Reload)와 재생 중 몽타주 이름
- **`[상태전환]`** — `SetCombatPoseState` 호출 시점 1줄

**교훈**: 주기 샘플만으로는 전환 순간을 특정할 수 없다(그 때문에 중간 구간을 전환 직후로
오독했다). 이벤트 로그와 짝지어야 한다. 그리고 **C++이 넘기는 값과 ABP가 실제로 받는 값,
그 아래 슬롯 가중치까지** 봐야 어느 층이 범인인지 갈린다.

## 8. 같은 날 함께 수정한 것

- **`KS_RifleAim`/`KS_RifleIdle` 의미 확정** — `Aim`=조준, `Idle`=총 내림. 구 규칙과 반대라
  아군·적군 블렌드스페이스 속도 0 줄과 ABP 시퀀스 노드 배정을 전부 뒤집었다. 적군도 동일
  (`AS_Enemy/AS_RifleAim`=조준).
- **`Enemy_Rifle_Socket` 회전 교정** — 소켓 프리뷰 애셋이 AK였는데 런타임엔 M4(`SK_AR4_X`)가
  붙어서 90° 틀어져 보였다. 프리뷰 애셋을 실물로 교체하고 뷰포트에서 재정렬.
- **`Gun` 컴포넌트는 더미가 아니다** — 사망 시 `DetachFromComponent` + `SetSimulatePhysics(true)`로
  **총을 떨어뜨리는 본체**다. 가시성 호출만 보고 "역할 없는 껍데기"로 오판해 삭제했다가
  노드 3개가 끊어졌고, 사용자가 재생성해 복구했다. **컴포넌트 삭제 전에는 참조를 전수 추적할 것.**
