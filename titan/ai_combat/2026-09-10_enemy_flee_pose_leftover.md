# 적군 도주 중 이전 전투 자세가 남는 버그 — 기울어진 채로 도망감

2026-09-10 / 완료(코드 수정 완료, PIE 검증 대기) / 도주(Flee) 상태엔 자세를 쓰는 코드가 아예 없어서 도주 직전 프레임의 애니메이션 플래그가 그대로 굳던 문제. `CommitPendingFlee()`에서 도주 자세를 한 번 확정하도록 수정.

선행 문서: `2026-09-03_enemy_combat_fixes.md`(자세 사이클/사격선), `enemy_scenario_combat_expansion.md`(자세 사이클 설계), `2026-09-01_animation_asset_inventory.md`(4-1절 Lean 설명).

---

## 1. 증상

사용자 리포트 — **적군 병사가 2차·3차 전투지로 도망갈 때 몸이 좌/우로 기울어진(Lean) 채로
달린다.** "사격 중에 도망이 시작돼서 이전 상태가 남은 것 같다"는 추측이 그대로 맞았다.

## 2. 원인

`LeanAlpha`(좌우 기울임, -1~+1)를 쓰는 곳은 **`UEnemyCombatComponent::ApplyCombatPose()`
단 한 곳**이다(`EnemyCombatComponent.cpp:1515`):

```cpp
ECC_SetFloatPropertyByName(Owner, FName("LeanAlpha"),
    Pose.Lean == EEnemyLean::Left ? -1.0 : (Pose.Lean == EEnemyLean::Right ? 1.0 : 0.0));
```

그런데 `ApplyCombatPose()`는 **`TickCombatPoseCycle()`(= `Combat` 상태)에서만** 매 틱 호출된다.
최상위 상태가 `Flee`로 넘어가면:

- `TickFlee()`는 이동(`TickNavPathMovement`) / 재타겟 / 조준(`SetDesiredFacingYaw` +
  `TickWeaponAimPitchCorrection`) / 단발 견제 사격만 한다. **자세 플래그는 하나도 안 건드린다.**
- 진입 지점인 `CommitPendingFlee()`도 상태 변수(`CurrentState`/`CombatPoseState`/
  `CombatPoseTimer`/`bFiringBurstTriggered`)만 리셋하고 애니메이션 플래그는 그대로 뒀다.

즉 **도주가 커밋되는 프레임 직전의 자세가 도주 내내 그대로 굳는다.** 도주 커밋은
`MinFleeCommitDelaySeconds~Max` 랜덤 지연 뒤에 일어나므로 그 시점의 자세 사이클 상태는 개체마다
제각각이고, 하필 `Firing`/`TransitioningToFiring`에서 걸린 개체는 그 엄폐 사격 자세의
`Lean=Left/Right`(±1)를 안고 뛰게 된다. 1차→2차 도주만 실측 55~60초라 그동안 계속 보인다.

레벨 데이터에도 lean이 실제로 쓰이고 있음을 확인했다 — `Content/New_kadex_0811.umap`의
직렬화 데이터에 `EEnemyLean::Left` / `EEnemyLean::Right`가 모두 들어 있다.

### 같은 원인으로 남는 것들

`LeanAlpha`만의 문제가 아니라 `ApplyCombatPose`가 쓰는 플래그 전부가 같은 구조로 남는다:

| 플래그 | 도주 중 남았을 때의 그림 |
|---|---|
| `LeanAlpha` | **리포트된 증상** — 기울어진 채 달림 |
| `IsHoldingWeapon?` | Cover 포즈(`bIsFiringPose=false`)에서 커밋되면 `false`인 채로 남는데, 정작 `TickFlee`는 단발 사격을 한다 → **총 내린 채 발사** |
| `IsProne` | Prone 포즈에서 커밋되면 **엎드린 자세로 `FleeMoveSpeed`(400) 질주** |
| `IsKneeling` | 직전 포즈 그대로(들쭉날쭉). `TickMove` 주석이 정의한 도주 스타일은 "빠른 숙인 뜀박질"이므로 `true`가 맞다 |

## 3. 수정

`CommitPendingFlee()`(`EnemyCombatComponent.cpp:533`)에서 도주 자세를 한 번 확정한다. 도주 중엔
아무도 이 값들을 덮어쓰지 않으므로 **1회 세팅으로 충분**하고, 새 전투지에 도착하면 `TickFlee`가
`Combat`으로 넘기면서 `ApplyCombatPose`가 다시 매 틱 관리한다.

```cpp
if (AActor* Owner = GetOwner())
{
    ECC_SetFloatPropertyByName(Owner, FName("LeanAlpha"), 0.0);         // 엄폐물 기울임 해제
    ECC_SetBoolPropertyByName(Owner, FName("IsProne"), false);          // 엎드린 채로는 못 뛴다
    ECC_SetBoolPropertyByName(Owner, FName("IsKneeling"), true);        // 숙인 뜀박질(도주 스타일)
    ECC_SetBoolPropertyByName(Owner, FName("IsHoldingWeapon?"), true);  // 도주 중에도 단발 견제 사격
}
```

`BeginMove()`도 구조는 똑같이 "자세를 안 되돌린다"지만 **호출이 항상 `BeginMove(0)`뿐이라**
(시나리오 1차 전투지 스텝 + `bAutoBeginMoveForTesting`) 그 시점엔 `LeanAlpha`가 기본값 0이다.
손대지 않았다. 나중에 `BeginMove(1)`/`BeginMove(2)` 같은 직접 이동이 생기면 여기도 같은 처리가
필요해진다.

## 4. 검증

- [ ] PIE에서 2차 도주 트리거 → 기울어진 채 뛰는 개체가 없는지
- [ ] 3차 도주(누적 7명 사망)에서도 동일 확인
- [ ] 도주 중 단발 견제 사격 시 총을 든 자세인지(`IsHoldingWeapon?` 수정분)

## 5. 교훈 — 같은 구조의 버그가 더 있을 수 있는 곳

**"매 틱 쓰는 상태값"과 "상태 전환 시 한 번만 쓰는 상태값"이 섞여 있고, 어떤 최상위 상태에서는
그 매 틱 갱신이 아예 안 도는 구조**가 원인이다. 같은 패턴을 이미 두 번 밟았다:

- `AimPitch` — 경계 이동(patrol) 중 상체를 들어올려 놓고 교전 시작 시 아무도 안 되돌려서 위를 본
  채 굳던 문제. `BeginEngageAtCurrentZone()`과 `TickMove`의 도착 분기에서 각각 0으로 되돌리는
  코드가 이미 들어가 있다(`EnemyCombatComponent.cpp:469`, `:1016`).
- 이번 `LeanAlpha`/`IsProne`/`IsHoldingWeapon?` — 같은 것의 도주 버전.

새 최상위 상태를 추가하거나 상태 전환 경로를 늘릴 때는 **"이 상태에서 매 틱 갱신되지 않는
애니메이션 플래그가 뭐가 남는가"** 를 한 번 훑을 것.
