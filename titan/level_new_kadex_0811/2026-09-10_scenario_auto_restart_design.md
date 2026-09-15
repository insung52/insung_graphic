# 시나리오 자동 재시작 (데모 모드) — 설계

2026-09-10 작성 / 2026-09-11 개정(에디터 실측 반영) / 설계확정·구현대기 / 적 섬멸 10초 뒤 인플레이스 소프트 리셋 → 3초 뒤 "적 특작부대 침투 상황 발생"으로 재시작. 방식은 **부활(revive)+원위치**로 확정.

관련: `scenario_three_stage_combat.md`(구현 현황), `2026-09-01_scenario_run_modes_demo_fullsystem.md`(데모 모드),
`2026-09-02_scenario_double_eval_travel_bug.md`(타이머/이중 평가 사고), `scenario_authoring_guide.md`(DT 저작).

---

## 0. 목표 타임라인

```
[적 15명 전멸]
   │  ScenarioComplete 행 발동 → "시나리오 완료" 토스트
   │  (+10초)  ← 새 행 ScenarioRestart, TriggerDelaySeconds=10
   ▼
[재시작 실행]  페이드 아웃 0.3s → 인플레이스 리셋(1프레임) → 페이드 인 0.5s
   │  (+3초)   ← ScenarioConfig.DemoAutoStartDelaySeconds = 3 (실측 확인됨)
   ▼
[BeginEnemyContactScenario()]
   │  "적 특작부대 침투 상황 발생" 토스트 7초 (WBP_Notify_EnemyContact)
   │  스텝 평가 시작 → +1s EnemyApproach, +3s UAVMission …
   ▼
[1사이클 반복]
```

사용자 질문 "3초 맞나" → **맞다.** `ScenarioConfig_1.DemoAutoStartDelaySeconds = 3`이고,
그 타이머가 끝나면 `DemoAutoStartScenario()` → `BeginEnemyContactScenario()`가 불리며,
그 함수가 직접 `ShowScenarioNotification("EnemyContact", 7초)`를 띄운다. 재시작도 **정확히 같은
경로를 재사용**하면 되므로 알림을 따로 만들 필요가 없다.

---

## 1. 에디터에서 실측 확인한 사실 (2026-09-11, MCP)

| 항목 | 실측값 | 설계에 주는 의미 |
|---|---|---|
| `ScenarioConfig_1.RunMode` | **Demo** | 데모 게이트가 바로 동작 |
| `DemoAutoStartDelaySeconds` | **3.0** | 위 타임라인의 3초 |
| `bDemoAutoStartScenario` / `bDemoForceUGVAutoFire` / `bDemoForceCommandPostAutoFire` | 전부 true | 재시작 후 `ApplyDemoRunModeSetup()` 재실행만 하면 RCWS 복구 (⚠️ 2026-09-15 이후엔 **지휘소만** — 아래 메모) |

> ⚠️ **2026-09-15 메모** — UGV RCWS 자동사격 강제가 `ApplyDemoRunModeSetup()`에서 빠지고 DT 행
> `UGVArriveZone1`(1차 목적지 도착 시, 이펙트 `SetDemoUGVAutoFire`)로 옮겨졌다
> (`2026-09-15_demo_ugv_autofire_on_zone1_arrival.md`). 재시작 관점에서:
> - 지휘소 절반은 여전히 §6-9의 재실행으로 복구된다.
> - UGV 절반은 `FiredScenarioSteps` 리셋으로 다음 사이클에 `UGVArriveZone1`이 다시 발동하므로
>   따로 할 일은 없다. **단, §4(d) UGV 리셋에 "RCWS 모드를 `Remote`로 되돌림"을 반드시 넣을 것** —
>   안 넣으면 이전 사이클의 AutoFire가 남아 2회차엔 출발 전부터 스윕한다(1회차와 다른 연출).
| `DemoFireMode` | Burst | 재시작 시 같이 재적용 |
| `ScenarioStepTable` | `DT_ScenarioSteps_ThreeStage` | 재시작 행을 여기에 추가 |
| `UGVZone3Destination` | **None** (+ `UGVMoveZone3` 행 `bEnabled=false`) | 3차에서 UGV는 안 움직임 — 리셋 대상에서 빠짐 |
| 적 | `BP_Enemy_kadex_C_1..15` = **15명** | |
| 아군 | `BP_Ally_kadex_C_1..25` = **25명** | |
| 적 인스턴스 초기값 | `Health=100`, `IsDead=false`, **`IsParachuting=true`**, `IsHoldingWeapon?=false`, `CurrentRifle=None` | 초기 상태가 "무기 없이 낙하 상태" — 클래스 기본값이 아니라 **인스턴스 스냅샷**을 떠야 하는 근거 |
| 적 사망 처리(BP_Enemy_Base EventGraph) | `… → Delay(5.0) → DestroyActor(CurrentRifle) → DestroyActor(self)` | **self 파괴 노드 1개만 지우면** 부활 방식이 성립 |
| 소총 | 런타임에 `EquipRifle` 커스텀 이벤트가 `SpawnActorFromClass`로 생성 | 부활 시 `EquipRifle` 재호출로 복구 |
| 아군 사망 처리 | `IsDead` 변수를 가진 BP는 적군 계열뿐. `UAllyFormationComponent`에도 체력 없음 | **아군은 현재 무적** — 부활 불필요, 위치/상태만 리셋 |
| RCWS 탄약 | `URCWSComponent::CurrentData`가 **private**, `ConsumeAmmo`만 존재(리로드 없음) | 리필 함수 신설 필요 |
| UGV 컴포넌트 | `RtspBridge` 존재 확인 | 레벨 재오픈 시 RTSP가 끊기는 근거 |
| 확인창 인프라 | `UNotificationSubsystem::ShowConfirmDialog(FText)` + `OnConfirmed`/`OnCancelled` 이미 존재 | 향후 "재시작 하시겠습니까?"는 거의 공짜(§7) |
| 알림 DT 행 | `EnemyContact`, `RoadExitWarning`, `ScenarioComplete` | 재시작 전용 알림을 원하면 행 1개 추가 |
| 레벨 | `New_kadex_0811.umap` 187MB 모놀리식(WP/OFPA 아님), `GameDefaultMap=/Game/kadex_lobby` | 재오픈은 비싸다. 단, 로비 맵이 있어 하드 리셋 폴백 경로는 존재 |

---

## 2. 방식 확정 — 부활(revive) + 원위치

재스폰이 아니라 부활로 간다. 근거:

1. 적/아군은 레벨 배치 인스턴스이고 인스턴스별 저작 데이터가 많다(`CombatZones[0..2]` 마커 6개 +
   자세, `SquadId`, `AmbushMarker`, 감지 반경 …). 재스폰하면 전부 복제해야 하고 하나 빠지면
   2회차 연출이 달라진다. 위 표의 `IsParachuting=true`처럼 **클래스 기본값과 다른 인스턴스 값**이
   실제로 존재하므로, 재스폰은 "클래스에서 새로 만들면 된다"가 성립하지 않는다.
2. 스켈레탈메시 40개 동시 스폰은 프레임 스파이크다.
3. 걸림돌이었던 "죽으면 액터가 사라진다"는 **노드 1개 삭제로 해결**된다(§8-1).
4. 시체를 남겨도 기존 로직이 안 깨지는 것을 코드로 확인했다:
   - 생존/잔여 수 판정은 액터 유효성이 아니라 `UDetectableTargetComponent::IsIncapacitated()`
     (`CollectEnemyCombatComponents`).
   - 도주 정원·빈 역할 승계도 `AliveEnemies.Contains(Original)` 기준(`BeginEnemyFleeToZone`).
   - 사격/타겟팅은 감지 레지스트리 기반이라 시체를 쏘지 않는다.

**초기 상태는 각 컴포넌트가 자기 `BeginPlay`에서 스스로 스냅샷**을 뜬다(액터 트랜스폼, 메시 상대
트랜스폼, BP 변수 초기값, MaxWalkSpeed, 포탑 기본각). 서브시스템이 남의 초기값을 알 필요가 없다.

---

## 3. 무한 재시작 분석 (사용자 요청 항목)

### 3.1 구조적으로 왜 안 도는가

재시작이 `FiredScenarioSteps`를 비우므로, **재시작 직후 `Prereq=None`인 행들이 다시 평가 대상**이
된다. DT에서 `Prereq=None`인 행은 4개뿐이고 각각을 따져보면:

| 행 | 트리거 | 재시작 직후 즉시 참이 되는가 | 판정 |
|---|---|---|---|
| `UAVMission` | TimerOnly 3s | 3초 뒤 정상 발동 | ✅ 의도대로 |
| `EnemyApproach` | TimerOnly 1s | 1초 뒤 정상 발동 | ✅ 의도대로 |
| `EnemyEngage` | `UGVFiredNearEnemy` ≤100m | **아니오** — `UGVFireWatch`를 리셋하면 `LastShotsFired=INDEX_NONE`이 되고, `UpdateRCWSFireWatch`는 첫 관측 틱에서 **기준선만 잡고 리턴**한다(코드 확인). 이전 사이클 누적 발사수는 영향을 못 준다 | ✅ 안전 |
| `EnemyFleeToZone2` | `EnemyCasualtyCountAtLeast` 3 | **위험** — `Casualty = ScenarioEnemyCountBaseline − Alive`인데, 부활 전에 스텝 평가가 돌면 Alive=0이라 즉시 발동한다 | ⚠ §3.2로 차단 |

그리고 `ScenarioComplete`는 **`Prereq=EnemyEngage`**(DT 실측 확인)이므로, 새 사이클에서 UGV가
100m 이내에서 다시 쏘기 전까지는 절대 발동할 수 없다. 즉 **"섬멸 → 재시작 → 즉시 섬멸 판정 →
재시작"의 폭주 루프는 구조적으로 불가능**하다. `EnemyEngage`가 루프 브레이커다.

### 3.2 그래도 지켜야 하는 3가지 (안 지키면 조용히 깨진다)

1. **평가 루프를 먼저 완전히 멈춘다.** `FTimerHandle::Invalidate()`가 아니라
   `ClearTimer(ScenarioStepTickTimerHandle)`. 재시작은 **같은 월드**라 `ResetForNewWorldIfNeeded`의
   월드 비교가 걸리지 않으므로 그 안전망이 없다. 게다가 `BeginScenarioSteps`는
   `bScenarioStepsRunning || IsTimerActive(...)`이면 **조용히 리턴**한다 — 안 멈추고 다시 부르면
   "재시작 버튼을 눌렀는데 아무 일도 안 일어남"이 된다(2026-09-02 사고의 쌍둥이).
2. **부활 → (3초 공백) → 스텝 시작** 순서를 지킨다. 리셋 시점에 루프가 죽어 있고, 3초 뒤
   `BeginScenarioSteps`가 `ScenarioEnemyCountBaseline = CountAliveEnemies()`를 다시 잡는다.
   그 시점엔 이미 15명이 살아 있으므로 `EnemyFleeToZone2`가 오발동하지 않는다.
   → **가드 추가**: `BeginScenarioSteps` 직전에 살아있는 적 수를 로그로 남기고, 0이면 재시작을
   중단하고 에러 로그(부활 실패를 조용히 넘기지 않기 위함).
3. **재진입 잠금.** `bRestartInProgress` 플래그 — 페이드/틱 대기 사이에 두 번째 재시작이
   들어오지 못하게. 향후 수동 재시작 버튼(§7)이 생기면 자동 타이머와 겹칠 수 있어 필수.

### 3.3 폭주 대신 "정지"가 날 수 있는 경우

부활이 실패해 적이 0명이면: RCWS가 쏠 대상이 없음 → `EnemyEngage` 미발동 → `ScenarioComplete`
미발동 → **재시작도 안 걸리고 데모가 멈춘다.** 폭주보다는 낫지만 무인 전시에선 똑같이 치명적이다.
그래서 §3.2-2의 "부활 검증 로그 + 중단"과, 옵션으로 §8-6의 하드 리셋 폴백을 둔다.

---

## 4. 리셋 대상 전수 목록

빠뜨리면 2회차부터 조용히 어긋나는 것들. **굵은 항목은 실측으로 확인된 함정.**

### (a) 시나리오 서브시스템 — `ResetScenarioRuntimeState()`

기존 `ResetForNewWorldIfNeeded()` 본문이 곧 이 목록이다. **그 본문을 함수로 뽑아내서 월드 변경
경로와 재시작 경로가 같은 함수를 쓰게 한다**(두 벌로 복사하면 한쪽만 갱신되는 사고가 난다).

- 타이머 3종 `ScenarioStepTickTimerHandle` / `DemoAutoFireTimerHandle` / `DemoAutoStartTimerHandle`
  → **ClearTimer**
- 아군 신호·집결 타이머 `ApproachSignalTimerHandle` / `AmbushSignalTimerHandle` /
  `FormUpAdvanceTimerHandle` / `FormUpStaggerTimerHandle` → ClearTimer (기존 월드 리셋 함수에는
  빠져 있다 — 재시작 경로에서는 반드시 필요)
- `FiredScenarioSteps`, `ScenarioStepFireTimes`, `ScenarioStepsStartTime`
- `ScenarioEnemyCountBaseline`(0으로 — Max로만 올라가므로 반드시 내려야 함), `ScenarioAllyFireCountBaseline`
- `LoggedEnemyDeaths`, `TrackedEnemies`, `LastEnemyRosterRefreshTime`
- `ScenarioZoneRoles`, `ScenarioSquadTotals`
- **`UGVFireWatch`, `CommandPostFireWatch`** → 구조체 새로 대입. RCWS의 `ShotsFiredCount` 자체는
  **리셋 불필요**(§3.1 근거)
- `FormUpLeader`, `FormUpDestination`, `bUGVAdvanceTriggered`, `AlliesReachedFormation`,
  `bAllAlliesReachedFormation`, `FormUpStaggerQueue`, `bHasUGVStandbyDestination`
- `bHasEnemyPredictedLocation`, `EnemyPredictedLocation`
- `RemoveUGVNavObstacle()`
- GameState: `EScenarioPhase` 초기값, 미니맵 적 예상 위치 마커

### (b) 적군 15 — 부활

- **랙돌 해제**: `Mesh->SetSimulatePhysics(false)` → 캡슐에 재부착 → 저장해둔 메시 상대 트랜스폼
  복원 → 콜리전 프로파일 복원 → `StopAllMontages`
- **BP 변수 복원(리플렉션)**: `Health`, `IsDead`, **`IsParachuting`**, `IsHoldingWeapon?`,
  `DeathIndex`, `HasTarget`, `IsProne`/`IsKneeling`/`IsLookingAround`/`IsSprinting`,
  `BurstShotsRemaining`, `IsReloading?`, `LastHitLocation`/`LastHitDirection`/`LastHitBoneName`/
  `LastHitVelocity`, `MoveTarget`, `TargetLocation`, `CurrentAlly`
  → 전부 **인스턴스 초기 스냅샷 값**으로(클래스 기본값이 아님)
- **소총 재장착**: 사망 5초 뒤 `CurrentRifle`이 Destroy되므로 `EquipRifle` 커스텀 이벤트를
  리플렉션으로 재호출(프로젝트에 이미 있는 관용구)
- **`UDetectableTargetComponent`**: `SetIncapacitated(false)` — 단 **`IsDead`를 먼저 false로**
  내린 뒤에 호출할 것. `PollBlueprintDeathState`는 `bIsIncapacitated==false`일 때만 도는데,
  `IsDead`가 아직 true면 다음 틱에 다시 죽은 것으로 되돌린다.
- **`SetRevealed(false)`** — 다시 숨긴다. **안 하면**: 데모 RCWS가 이미 AutoFire 상태라
  재시작 직후 UGV가 제자리의 적을 바로 쏘기 시작하고, 드론 정찰·낙하산 발견·UGV 출발 순서가
  통째로 건너뛰어진다. 2회차부터 데모가 다른 물건이 된다.
- `SetTargetableByAlliesAndUGV(true)`, `SetFireHold(false)` (`ExcludeFleeingEnemies` /
  `HoldFleeingFire` 이펙트가 꺼둔 것)
- 컴포넌트 상태: `CurrentState=Standby`, `bEngaged=false`, `CurrentZoneIndex=0`, 포즈 사이클
  타이머, 타겟 해제, 단발 사격 쿨다운, `SetDetectionEnabled(false)`
- 이동: 경로 취소, `CharacterMovement` 속도 0, `MaxWalkSpeed` 초기값
- 액터 트랜스폼: 초기 스냅샷으로 `TeleportTo`

### (c) 아군 25 — 위치/상태만

위치·회전 복원, 매복/집결 상태 초기화, `SetStopsignRaised(false)`, 타겟 해제, 이동 중단,
자세 사이클 초기화.
**`FireTriggerCount`는 건드리지 않는다** — `BeginScenarioSteps`가 그 합으로 baseline을 다시
잡으므로 카운터를 0으로 만들면 오히려 어긋난다.

### (d) UGV (`BP_UGV_0901_C_1`)

- AI: `AUGVAIController` 경로 취소, 목적지 상태 초기화
- `SetUGVEnemyDistanceSpeedLimit(false)` (`UGVSpeedLimitOn` 이펙트가 켠 것)
- **`FireControl->bRespectEnemyTargetingExclusion = false`** — `ExcludeFleeingEnemies` 이펙트가
  UGV RCWS에 켜두는 플래그. 안 되돌리면 2회차부터 UGV가 일부 적을 영영 안 쏜다
- Chaos 차량: `SetActorTransform(..., ETeleportType::TeleportPhysics)`, 선/각속도 0,
  스로틀·브레이크 입력 0
- RCWS: 포탑 방위/고각 원위치, 락온 게이지·배럴 스핀·줌 램프 리셋, **모드를 `Remote`로**
  (2026-09-15 — AutoFire는 재시작이 아니라 `UGVArriveZone1` 행이 도착 시 다시 켠다)
- **탄약**: `CurrentData.AmmoCurrent = AmmoMax`. 600발이라 2~3사이클이면 마르고, 마르면 UGV가
  못 쏴서 `EnemyEngage`가 영영 안 걸린다(=§3.3의 정지). `CurrentData`가 private이므로
  `URCWSComponent`에 리필 함수 신설 필요
- `UGVAvoidanceProxyComponent` 상태

### (e) 이동형지휘소 (`BP_TitanTruck_C_4`)

RCWS 동일(모드/탄약/포탑/게이지). 위치는 안 움직이므로 트랜스폼 복원 불필요.

### (f) 드론 (`ADronePawn`)

- `Flight->ResetTo(SpawnLocation + (0,0,200), SpawnYawDegrees)` — `OnResetPressed`가 쓰는 기존 경로
- **`bParachuteObserved = false`** — 안 되돌리면 `UAVSpotted`(트리거 `UAVParachuteObserved`)가
  재시작 직후 즉시 참이 되어 UGV가 바로 출발하고 정찰 연출이 통째로 날아간다
- 자율비행/스플라인 진행도 초기화, `RecenterGimbal()`, 프레이밍 세트 `None`,
  단계별 탐지(아군만 → +낙하산 → +적군) 초기 단계로

### (g) 낙하산 (`BP_Parachute_C_3`)

초기 위치(z=1030)/타임라인 복원. BP 액터이므로 §5 인터페이스를 BP에서 구현.

### (h) 월드 잔재

살아있는 `ARCWSProjectile` 전부 Destroy, 루프 사운드 정지, 탄흔 데칼 정리(수명 무한이면 하루
누적 시 성능 영향), 떠 있는 토스트 숨김, 대시보드 카운터 갱신.

### (i) 리플리케이션

전부 서버 권위(스텝 평가가 이미 서버 전용). 클라 전용 캐시는 멀티캐스트 1발로 같이 리셋.

---

## 5. 코드 설계

### 5.1 인터페이스

```cpp
// UI/ScenarioResettable.h
UINTERFACE(MinimalAPI, Blueprintable)
class UScenarioResettable : public UInterface { GENERATED_BODY() };

class IScenarioResettable
{
    GENERATED_BODY()
public:
    // 시나리오 재시작 — "레벨 시작 직후" 상태로 되돌린다. 서버에서만 호출된다.
    UFUNCTION(BlueprintNativeEvent, Category = "Scenario")
    void ResetForScenarioRestart();
};
```

서브시스템은 월드의 구현체를 순회해 한 번씩 호출한다. 액터가 늘어나도 서브시스템을 안 고친다.

**적/아군은 BP 그래프를 건드리지 않는다** — C++ 컴포넌트(`UEnemyCombatComponent` /
`UAllyFormationComponent`)가 자기 소유 액터의 BP 변수를 리플렉션으로 복원하고 `EquipRifle`도
리플렉션으로 호출한다. 프로젝트에 이미 같은 패턴이 있다(`PollBlueprintDeathState`,
`SetBoolPropertyByName`류, 구 `BeginEngagementApproach` 호출). BP 작업은 §8-1의 **노드 1개 삭제**뿐.

### 5.2 공개 진입점 (자동/수동 공용)

```cpp
// UScenarioStateSubsystem
UFUNCTION(BlueprintCallable, Category = "Scenario")
bool RequestScenarioRestart(bool bForce = false);   // 유일한 진입점
```

- DT 이펙트 `RestartScenario`, 콘솔 exec 명령, 향후 UI 버튼/확인창이 **전부 이걸 호출**한다.
- `bForce=false`면 데모 게이트(`IsDemoMode() && Config->bDemoAutoRestart`)를 확인하고,
  수동 호출(버튼)은 `bForce=true`로 풀 시스템에서도 쓸 수 있게 둔다.
- `bRestartInProgress`면 false 반환.

### 5.3 ScenarioConfig 추가 필드

| 필드 | 기본 | 의미 |
|---|---|---|
| `bDemoAutoRestart` | true | 데모 자동 재시작 사용 |
| `RestartFadeOutSeconds` / `RestartFadeInSeconds` | 0.3 / 0.5 | 리셋 순간을 가림 |
| `HardReloadEveryNCycles` | 0(끔) | N회마다 레벨 재오픈으로 완전 세탁(§8-6) |

### 5.4 DT 행

| RowName | Prereq | Trigger | 값 | Effect | bEnabled |
|---|---|---|---|---|---|
| `ScenarioRestart` | `ScenarioComplete` | `TimerOnly` | **10** | `RestartScenario` | true |

10초는 DT 값이라 빌드 없이 전시장에서 조절 가능. 데모 게이트는 이펙트 구현부에 있으므로
풀 시스템과 같은 DT를 공유해도 안전하다.

---

## 6. 실행 시퀀스

`RequestScenarioRestart()` 내부:

| # | 동작 | 이유 |
|---|---|---|
| 1 | 게이트 확인 + `bRestartInProgress = true` | 재진입 차단 |
| 2 | `ClearTimer(ScenarioStepTickTimerHandle)` + `bScenarioStepsRunning = false` | §3.2-1 |
| 3 | 페이드 아웃 시작 | 텔레포트 팝·물리 튐을 가림 |
| 4 | 월드 잔재 정리(투사체/사운드/토스트) | |
| 5 | `IScenarioResettable` 구현체 전부 `ResetForScenarioRestart()` | 부활 + 원위치 |
| 6 | **한 틱 대기** | 텔레포트한 Chaos 차량/캐릭터가 같은 프레임에 명령을 받으면 물리가 튄다 |
| 7 | `ResetScenarioRuntimeState()` | §4(a) |
| 8 | 부활 검증: `CountAliveEnemies() == 기대치`인지 로그, 0이면 중단+에러 | §3.2-2 |
| 9 | `ApplyDemoRunModeSetup()` 재실행 | **지휘소** RCWS ARM+AutoFire+Burst 복구 (UGV는 2026-09-15부터 `UGVArriveZone1` 행 담당 — §1 메모) |
| 10 | 페이드 인 + `DemoAutoStartDelaySeconds`(3초) 타이머 | |
| 11 | `BeginEnemyContactScenario()` → EnemyContact 토스트 + 스텝 평가 시작 | 기존 경로 재사용 |
| 12 | `bRestartInProgress = false`, 사이클 카운터 +1, 요약 로그 | |

로그 1줄 예시:
`[ScenarioStateSubsystem] 재시작 #7 완료 — 적 15/15 부활, 아군 25/25, UGV탄 600/600, 트럭탄 600/600, 소요 14.2ms`

---

## 7. 수동 재시작 / "재시작 하시겠습니까?" (향후)

인프라가 이미 있어서 추가 작업이 거의 없다:

- `UNotificationSubsystem::ShowConfirmDialog(FText)` + `OnConfirmed` / `OnCancelled` (기존)
- 버튼/단축키 → `ShowConfirmDialog("시나리오를 재시작하시겠습니까?")` →
  `OnConfirmed` → `RequestScenarioRestart(true)`

⚠ **주의**: `OnConfirmed`/`OnCancelled`는 서브시스템 단위 멀티캐스트 델리게이트라 **모든 확인창이
공유**한다. 재시작 핸들러를 상시 바인딩해두면 "종료하시겠습니까?" 같은 다른 확인창에서 확인을
눌러도 재시작이 걸린다. **띄울 때 바인딩하고 결과가 오면 즉시 언바인딩**할 것.

§5.2에서 `RequestScenarioRestart`를 처음부터 공개 API로 만들어 두면, 자동 타이머·콘솔·버튼·확인창이
전부 같은 경로를 타므로 나중에 UI를 붙일 때 C++을 다시 안 건드린다.

---

## 8. 작업 순서 (파일별)

1. **`Content/Soldiers/BP_Enemy_Base`** — EventGraph의 `DestroyActor(self)` 노드 하나 삭제
   (`K2Node_CallFunction_171`). 앞의 `Delay(5.0) → DestroyActor(CurrentRifle)`는 그대로 둔다.
   → 이것 없이는 나머지가 전부 무의미. **P4 체크아웃 먼저.**
2. **`UI/ScenarioResettable.h`** 신설 (§5.1).
3. **`UI/ScenarioStateSubsystem.*`** — `ResetForNewWorldIfNeeded` 본문을
   `ResetScenarioRuntimeState()`로 추출, `RequestScenarioRestart()` + 시퀀스(§6),
   `bRestartInProgress`, 사이클 카운터, 콘솔 exec 명령.
4. **`UI/ScenarioStepTypes.h`** — `EScenarioEffectType::RestartScenario` 추가 +
   `ExecuteScenarioEffect`에 케이스 추가(내부는 `RequestScenarioRestart()` 호출 한 줄).
5. **`Soldiers/EnemyCombatComponent.*`** — BeginPlay 스냅샷 + `ResetForScenarioRestart()` 구현
   (§4b). 가장 큰 덩어리. 이 단계까지만 해도 "적만 부활"로 1차 검증이 가능하다.
6. **`Soldiers/AllyFormationComponent.*`** — 동일 축, 훨씬 작음(§4c).
7. **`Vehicles/RCWSComponent.*`** — 탄약 리필 + 포탑/게이지 리셋 함수.
   **`Vehicles/RCWSFireControlComponent.*`** — `bRespectEnemyTargetingExclusion` 원복 포함 리셋.
8. **`Vehicles/UGV0901Pawn.*` / `UGVAIController.*` / `TitanTruck.*` / `Drone/DronePawn.*`** —
   각자 `ResetForScenarioRestart()` 구현(§4d~f).
9. **`UI/ScenarioConfig.h`** — 필드 3종(§5.3).
10. **DT 행 `ScenarioRestart` 추가**(§5.4) + 필요하면 알림 DT에 재시작 예고 행.
11. 페이드 연출.
12. **선택**: 하드 리셋 폴백 — `HardReloadEveryNCycles`마다, 또는 §3.3의 부활 실패 시
    `OpenLevel(New_kadex_0811)`. 로비 맵(`kadex_lobby`)이 이미 있으므로 경유 경로도 가능.
13. 10사이클 연속 검증(§9).

---

## 9. 검증

10사이클 연속 자동 실행 후 1회차와 비교:

| 항목 | 기대 |
|---|---|
| 적 부활 수 / 아군 수 | 15 / 25 |
| UGV·트럭 탄약 | 600 / 600 |
| 사상자 로그 누적 | 사이클마다 0에서 다시 셈 |
| 스텝 발동 순서 | `[ScenarioStateSubsystem] 시나리오 스텝 발동:` 로그가 1회차와 동일 순서 |
| 적 최종 위치 | 3차 전투지 부근(1회차와 동일) |
| fps / `stat memory` | 1회차와 유의미한 차이 없음 |
| RTSP 스트림 | 재접속 없이 계속 살아 있음 |

특히 **2회차**를 집중해서 본다 — 위 §4의 함정(재은폐, 낙하산 관측 플래그, 탄약,
`bRespectEnemyTargetingExclusion`)은 전부 "1회차는 멀쩡하고 2회차부터 깨지는" 종류다.

---

## 10. 리스크 / 미결

- **부활 실패 시 정지**(§3.3). 검증 로그 + 하드 리셋 폴백으로 대응. "시나리오 시작 후 N분 경과 시
  강제 재시작" 같은 시간 기반 안전장치를 넣을지는 사용자 판단 — 요구에 없는 안전로직을 임의로
  넣지 않는다는 프로젝트 원칙이 있으므로 지시가 있을 때만.
- 랙돌 시체를 언제 굳힐지(사망 직후 vs 재시작까지 유지) — 연출 취향 + 성능 실측.
- 낙하산 강하 연출이 1회성 타임라인이면 BP 쪽 리셋 구현이 필요(§4g) — BP 구조 확인 후 확정.
- `HardReloadEveryNCycles` 적정값은 §9의 메모리 추이를 보고 결정.
