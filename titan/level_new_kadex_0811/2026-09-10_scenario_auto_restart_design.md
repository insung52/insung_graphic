# 시나리오 자동 재시작 (데모 모드) — 설계

2026-09-10 / 설계단계 / 데모 모드에서 적 섬멸 10초 뒤 자동 재시작. 레벨 재오픈 대신 **인플레이스 소프트 리셋**(액터 부활+원위치)을 권장하는 근거와 리셋 대상 전수 목록.

관련: `scenario_three_stage_combat.md`(구현 현황), `2026-09-01_scenario_run_modes_demo_fullsystem.md`(데모 모드),
`2026-09-02_scenario_double_eval_travel_bug.md`(레벨 트래블이 시나리오 상태에 남긴 사고),
`scenario_authoring_guide.md`(DT 저작).

---

## 0. 요구사항

전시 데모(`AScenarioConfig::RunMode == Demo`)에서 **적군 전멸 → 약 10초 뒤 → 시나리오가 처음부터 다시**.
무인 방치로 하루 종일 반복 재생되는 것이 목적.

풀 시스템(통제기·상위체계 연동) 구성에서는 **절대 자동 재시작하면 안 된다** — 통제기가 상황을 쥐고 있는데
언리얼이 혼자 판을 갈아엎는 꼴이 된다. 게이트 필수.

---

## 1. 트리거는 DT 한 행이면 끝난다

`DT_ScenarioSteps_ThreeStage`에는 이미 종료 행이 있다:

| RowName | Prereq | Trigger | Effect |
|---|---|---|---|
| `ScenarioComplete` | `EnemyEngage` | `AllEnemiesEliminated` | `ShowUIMessage` |

여기에 한 행만 매단다:

| RowName | Prereq | Trigger | 값 | Effect |
|---|---|---|---|---|
| `ScenarioRestart` | `ScenarioComplete` | `TimerOnly` | **10s** | **`RestartScenario`** (신설) |

- `AllEnemiesEliminated`는 `UDetectableTargetSubsystem`의 Enemy 등록이 0이 되는 순간 참이라
  **"적이 아직 안 뜬 시작 직후"에도 참이 된다**(`ScenarioStepTypes.h` 주석의 함정). 설계표
  (`scenario_three_stage_combat.md` §4)는 `ScenarioComplete`의 Prereq를 `EnemyEngage`로 두어 이를 피한다 —
  **DT에서 실제로 그렇게 들어가 있는지 확인할 것**(uasset 바이너리로는 행 이름만 확인됨).
  재시작 후에도 이 Prereq가 매 사이클 다시 걸려야 무한 재시작 루프가 안 생긴다.
- 10초는 DT의 `TriggerDelaySeconds`로 저작 — 전시장에서 페이싱만 바꾸고 싶으면 빌드 없이 조정 가능.
- 데모 게이트는 **이펙트 구현부**에서: `IsDemoMode()`가 거짓이거나 `ScenarioConfig::bDemoAutoRestart`가
  꺼져 있으면 로그만 남기고 no-op. 이러면 같은 DT를 두 구성이 공유해도 안전하다(`bEnabled`를 껐다 켰다 하는
  운용은 실수하기 쉬움).

`AScenarioConfig`에 추가할 값(기존 `Scenario|Run Mode` 카테고리 옆):

| 필드 | 기본 | 의미 |
|---|---|---|
| `bDemoAutoRestart` | true | 데모에서 자동 재시작 사용 |
| `RestartFadeOutSeconds` / `RestartFadeInSeconds` | 0.3 / 0.5 | 리셋 순간을 가리는 페이드 |
| `HardReloadEveryNCycles` | 0(끔) | N회마다 소프트 대신 레벨 재오픈(§5 하이브리드) |

---

## 2. 재시작 방식 3안

### A. 레벨 재오픈 (`OpenLevel` / `ServerTravel`)

구현 5줄, 초기화 100% 보장. 하지만 이 프로젝트에서는 비용이 크다:

1. **`New_kadex_0811.umap`이 187MB 모놀리식**(World Partition/OFPA 아님 — `Content/__ExternalActors__`에
   이 레벨 항목이 없다). 여기에 PCG 숲까지 얹혀 있어 매 사이클 수 초~수십 초 암전. 관람객 앞에서 반복된다.
   → 실측하려면 로그의 `LogLoad: Took N seconds to LoadMap`을 보면 된다(현재 로그엔 에디터 기동분만 있음).
2. **RTSP 12스트림이 전부 끊긴다.** `URtspStreamComponent`는 차량의 **액터 컴포넌트**이고
   `UVehicleRtspBridgeComponent`가 BeginPlay 시점에 RenderTarget 크기로 NVENC 세션/SDP를 확정한다
   (`VehicleRtspBridgeComponent.h` 주석). 월드가 새로 뜨면 마운트가 통째로 재생성 → VLC/통제기/전시 모니터가
   전부 재접속해야 한다. **무인 데모에서 이건 사실상 실격 사유.**
3. 2대 PC 구성이면 비-seamless 트래블로 **클라이언트(자체방호 축)도 같이 암전 + 재접속**.
4. 하루 8시간 × 수십 사이클의 로드/GC 반복 — 안정성은 오히려 검증이 더 필요하다.

`bUseSeamlessTravel` + 전환 맵을 써도 2번(액터가 새로 생기므로 스트림 재수립)은 그대로다.

### B. 인플레이스 소프트 리셋 — **권장**

액터를 파괴/재생성하지 않고 "레벨 시작 직후 상태"로 되돌린다.

- 레벨 로드 0, **RTSP 무중단**, 클라이언트 재접속 없음, 전환 비용은 한 프레임 + 물리 안정화 1~2틱.
- 유일한 리스크는 **리셋 대상을 빠뜨리는 것** — 빠뜨리면 2회차부터 조용히 어긋난다(예: 탄약).
  그래서 §4에 전수 목록을 만들고, §7의 10사이클 연속 검증으로 잡는다.

### C. 하이브리드

소프트 리셋을 기본으로 하되 `HardReloadEveryNCycles`(예: 20)마다 한 번은 레벨 재오픈으로 완전 세탁.
누적 드리프트·누수 보험. 카운터는 GameInstance 서브시스템에 둔다(레벨을 넘어 살아남아야 하므로).

**결론: B로 만들고 C를 옵션으로 남긴다.**

---

## 3. "재스폰이냐 위치 이동이냐" — 부활(revive)이 답

### 재스폰이 나쁜 이유

적/아군은 **레벨에 배치된 인스턴스**이고, 인스턴스별 저작 데이터가 매우 많다:
`CombatZones[0..2]`의 FiringPose/CoverPose 마커 6개 + BodyPose + Lean, `SquadId`, `bIsSquadLeader`,
`DefaultWatchYawDeg`, 수신호 몽타주, `AmbushMarker`, 감지 스피어 반경…
(`scenario_three_stage_combat.md` §7 마커 인벤토리 전체가 인스턴스 값이다.)

재스폰하면 이걸 전부 스냅샷 떠서 복제해야 하고, 하나라도 빠지면 2회차 연출이 달라진다.
게다가 스켈레탈메시+ABP+피직스애셋 40개 동시 스폰은 그 자체로 프레임 스파이크다.
**재스폰은 더 비싸고 더 위험하다.**

### 그래서 "부활 + 원위치"

같은 액터를 그대로 두고 상태만 초기화하면 저작 데이터는 손댈 일이 없다. 걸림돌은 하나:

> ⚠️ **`BP_Enemy_Base`가 사망 시 `K2_DestroyActor`를 호출한다**(uasset에서 확인).
> 그러면 부활시킬 액터가 없다.

**해결: BP에서 Destroy를 빼거나 `bDestroyOnDeath` 변수로 게이트한다.** 시체는 랙돌로 누워 있게 두면 된다.

이게 안전한 이유(코드로 확인함):

- 사망 판정과 "몇 명 남았나"는 **액터 유효성이 아니라 `UDetectableTargetComponent::IsIncapacitated()`**로
  한다(`UScenarioStateSubsystem::CollectEnemyCombatComponents`). 시체가 남아도 생존 수는 정확하다.
- 도주 정원/빈 역할 승계도 `AliveEnemies.Contains(Original)` 기준이라 영향 없다
  (`BeginEnemyFleeToZone`). 오히려 `CaptureEnemyZoneRoles` 스냅샷이 필요했던 원인
  ("죽은 적 액터가 파괴되면서 마커째 사라져 정원이 깎인다")이 사라진다 — 스냅샷은 그대로 둬도 무해하다.
- 사격/타겟팅도 레지스트리 기반이라 시체를 쏘지 않는다(`EnemyCombatComponent.h:978` 주석).
- 랙돌 40구의 물리 비용이 걱정되면 사망 N초 뒤 `SetSimulatePhysics(false)`로 굳히면 된다.

### 아군은 애초에 죽지 않는다

`IsDead`를 가진 BP는 적군 계열뿐이고(`BP_Enemy_Base`/`ABP_Enemy_*`), `BP_Ally_kadex`에도
`UAllyFormationComponent`에도 체력/사망 처리가 없다. **현재 빌드에서 아군은 무적**이다.
→ 아군 리셋은 "부활"이 아니라 위치/자세/상태 초기화만 하면 된다(§4c). 나중에 아군 사망이 들어오면
적군과 같은 축으로 확장.

---

## 4. 리셋 대상 전수 목록

빠뜨리면 2회차부터 깨지는 것들. 레이어별로.

### (a) 시나리오 서브시스템

**기존 `ResetForNewWorldIfNeeded()`의 본문이 곧 정답 목록이다.** 이걸
`ResetScenarioRuntimeState()`로 뽑아내서 **월드 변경 경로와 재시작 경로가 같은 함수를 쓰게 한다.**
두 벌로 복사하면 한쪽만 갱신되는 사고가 반드시 난다.

- 타이머 3종(`ScenarioStepTickTimerHandle`/`DemoAutoFireTimerHandle`/`DemoAutoStartTimerHandle`)은
  **반드시 `ClearTimer`** — `Invalidate()`는 핸들만 버리고 타이머는 계속 돈다
  (2026-09-02 이중 평가 사고의 직접 원인). 재시작은 **같은 월드**라 `ResetForNewWorldIfNeeded`의
  월드 비교가 걸리지 않으므로, 이 정리를 재시작 경로가 스스로 해야 한다.
- `FiredScenarioSteps`, `ScenarioStepFireTimes`, `ScenarioStepsStartTime`
- `ScenarioEnemyCountBaseline`, `ScenarioAllyFireCountBaseline`
- `LoggedEnemyDeaths`, `TrackedEnemies`, `LastEnemyRosterRefreshTime`
- `ScenarioZoneRoles`, `ScenarioSquadTotals` (재시작 후 `CaptureEnemyZoneRoles`가 다시 뜬다)
- `UGVFireWatch`, `CommandPostFireWatch`
  → **RCWS의 `ShotsFiredCount` 자체는 리셋 불필요**. Watch의 `LastShotsFired`가 `INDEX_NONE`으로
    돌아가면 다음 관측에서 기준선을 다시 잡는 구조다.
- `FormUpLeader`, `bUGVAdvanceTriggered`, `AlliesReachedFormation`, `bAllAlliesReachedFormation`,
  `FormUpStaggerQueue`, `bHasEnemyPredictedLocation`/`EnemyPredictedLocation`, standby destination
- 아군 신호/집결 타이머(`ApproachSignalTimerHandle`, `AmbushSignalTimerHandle`,
  `FormUpAdvanceTimerHandle`, `FormUpStaggerTimerHandle`) 전부 ClearTimer
- `RemoveUGVNavObstacle()` — 집결 때 붙인 NavModifier 정리
- GameState: `EScenarioPhase` 초기값, 미니맵 "적 예상 위치" 마커 클리어

### (b) 적군 15 — `UEnemyCombatComponent::ResetForScenarioRestart()`

- **액터**: BeginPlay에서 떠둔 초기 트랜스폼으로 `TeleportTo`, 랙돌 해제
  (`SetSimulatePhysics(false)` → 메시를 캡슐에 재부착 + 초기 상대 트랜스폼 복원), 콜리전 프로파일 복원,
  `StopAllMontages` + 애님 인스턴스 재초기화
- **BP 변수**: `Health` = Max, `IsDead` = false — 프로젝트에 이미 있는 리플렉션 세터 관용구
  (`SetBoolPropertyByName`류) 사용. `IsDead`를 내리면 `UDetectableTargetComponent::PollBlueprintDeathState`가
  다시 죽었다고 오인하지 않는다(그 폴링은 `bIsIncapacitated`가 false일 때만 돈다 → 순서 주의:
  **`IsDead`를 먼저 내리고 그 다음 `SetIncapacitated(false)`**)
- **`UDetectableTargetComponent`**: `SetIncapacitated(false)`(레지스트리 재등록),
  **`SetRevealed(false)`**(다시 숨김 — `RevealEnemies` 스텝 전 상태로), `bLowPriorityForEnemyTargeting` 원복
- **컴포넌트 상태**: `CurrentState = Standby`, `bEngaged = false`, `CurrentZoneIndex = 0`,
  포즈 사이클 상태/타이머, 타겟 해제, 단발 사격 쿨다운, `SetDetectionEnabled(false)`,
  `SetTargetableByAlliesAndUGV(true)`, `SetFireHold(false)`
- **이동**: 경로 취소 + `CharacterMovement` 속도 0 + `MaxWalkSpeed` 초기값 복원

### (c) 아군 25 — `UAllyFormationComponent::ResetForScenarioRestart()`

위치/자세 복원, 매복·집결 상태 초기화, `SetStopsignRaised(false)`, 타겟 해제, 이동 중단.
**`FireTriggerCount`는 건드리지 말 것** — `BeginScenarioSteps`가 재시작 시 그 합으로 baseline을
다시 잡으므로, 카운터를 0으로 되돌리는 쪽이 오히려 위험하다(둘 다 리셋하면 무해하지만, 한쪽만 하면 깨진다).

### (d) UGV(`BP_UGV_0901` / `AUGV0901Pawn`) — 가장 성가신 축

- **AI**: `AUGVAIController` 경로 취소 + 목적지 상태 초기화, `SetUGVEnemyDistanceSpeedLimit(false)`
- **Chaos 차량**: `SetActorTransform(..., ETeleportType::TeleportPhysics)` +
  선/각속도 0 + 스로틀/브레이크 입력 0. 텔레포트 직후 1~2틱은 서스펜션이 튈 수 있다 → §6의 페이드로 가린다.
- **RCWS**: 포탑 yaw/pitch 원위치, 락온 게이지·배럴 스핀·줌 램프 리셋, `SetControlMode` 초기값,
  데모면 `ApplyDemoRCWSAutoFire()` 재실행
- **탄약**: `URCWSComponent::CurrentData.AmmoCurrent = AmmoMax`.
  현재 **리로드 플로우가 아예 없다**(`ConsumeAmmo`만 있고 반대 방향이 없음, `RCWSComponent.h:303` 주석).
  세터를 하나 신설해야 한다. **소프트 리셋에서 가장 놓치기 쉬운 항목** — 600발이라 2~3사이클이면 마르고,
  그 뒤로는 아무도 안 쏴서 시나리오가 1차 교전에서 영구 정지한다.
- `UGVAvoidanceProxyComponent` 상태

### (e) 이동형지휘소(`BP_TitanTruck0`)

RCWS 동일(모드/탄약/포탑/게이지). 위치는 안 움직이므로 트랜스폼 복원은 생략 가능.

### (f) 드론(`BP_Drone` / `ADronePawn`)

`Flight->ResetTo(SpawnLocation + (0,0,200), SpawnYawDegrees)` — `OnResetPressed`가 이미 쓰는 경로를
그대로 재사용. 여기에 자율비행/스플라인 진행도 초기화, `RecenterGimbal()`, 프레이밍 세트 `None`,
`bHasObservedParachute = false`, 단계별 탐지(아군만 → +낙하산 → +적군) 초기 단계로.

### (g) 낙하산 액터

강하 연출이 1회성이면 초기 위치/타임라인으로. BP 액터이므로 §5의 인터페이스를 BP에서 구현하는 게 맞다.

### (h) 월드 잔재

살아있는 `ARCWSProjectile` 전부 Destroy, 총구/피격 나이아가라 원샷은 자연 소멸(수명 확인),
루프 사운드 정지, 탄흔 데칼(수명 무한이면 정리 — 하루 종일 누적되면 성능에 영향),
알림 위젯 숨김, 대시보드의 사상자/탄약 표시 갱신.

### (i) 리플리케이션

전부 **서버 권위**로 실행하고 클라는 리플리케이트 결과를 따른다(스텝 평가가 이미 서버 전용).
클라 전용 캐시(위젯 누적 로그 등)는 멀티캐스트 1발로 같이 리셋. 소프트 리셋의 큰 장점이
"클라이언트 재접속이 없다"는 점이므로, 여기서 클라 상태를 놓치면 장점이 반감된다.

---

## 5. 구현 형태 — 인터페이스 하나로

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

- C++ 컴포넌트(적/아군/UGV/드론/RCWS)는 네이티브 구현, BP 전용 액터(낙하산 등)는 BP 이벤트로 구현.
- 서브시스템은 **월드에서 이 인터페이스 구현체를 전부 순회해 한 번씩 호출**하면 끝 →
  나중에 액터가 추가돼도 서브시스템 코드를 안 고친다.
- **초기 상태 스냅샷은 각 컴포넌트가 자기 BeginPlay에서 스스로 뜬다**(액터 트랜스폼, 메시 상대 트랜스폼,
  MaxWalkSpeed, 포탑 기본각 등). 서브시스템이 남의 초기값을 알 필요가 없어 결합도가 최소가 된다.

### 실행 순서 (`UScenarioStateSubsystem::RestartScenarioInPlace()`)

1. 스텝 평가 루프 정지(ClearTimer) + 재진입 잠금
2. 페이드 아웃 시작(`RestartFadeOutSeconds`)
3. 월드 잔재 정리(투사체/이펙트/사운드)
4. 인터페이스 구현체 전부 `ResetForScenarioRestart()` 호출
5. **한 틱 대기** — 텔레포트한 Chaos 차량/캐릭터가 같은 프레임에 이동 명령을 받으면 물리가 튄다
6. `ResetScenarioRuntimeState()` (§4a)
7. `ApplyDemoRunModeSetup()` 재실행(RCWS ARM+AutoFire, 발사 모드)
8. 페이드 인 + `DemoAutoStartDelaySeconds` 뒤 `BeginEnemyContactScenario()`
9. 로그 1줄: `재시작 #N — 적 15/15 복원, 아군 25/25, 탄약 600/600, 소요 12.3ms`

---

## 6. 연출

리셋 순간 0.3초 페이드 아웃 → 리셋 → 0.5초 페이드 인. 관람객에겐 "장면 전환"으로 보이고,
텔레포트 팝과 물리 튐을 전부 가린다. 종료~재시작 10초 구간에는 `ShowUIMessage`로 안내 문구
(`DT_NotificationWidgets`에 행 추가). 카운트다운까지 넣을지는 취향.

---

## 7. 검증

**10사이클 연속 자동 실행 후 1회차와 비교**:

| 항목 | 기대 |
|---|---|
| 적 생존 수 / 아군 수 | 15 / 25 |
| UGV·트럭 탄약 | 600 / 600 |
| 사상자 로그 누적 | 사이클마다 15에서 초기화 |
| UGV 최종 위치 | 3차 목적지 부근(1회차와 동일) |
| fps / `stat memory` | 1회차와 유의미한 차이 없음 |
| RTSP 스트림 | 재접속 없이 계속 살아 있음 |

각 사이클의 스텝 발동 순서(`[ScenarioStateSubsystem] 시나리오 스텝 발동:`)가 1회차와 같은지도 로그로 대조.

---

## 8. 작업 순서

1. **`BP_Enemy_Base` 사망 시 `K2_DestroyActor` 제거(또는 `bDestroyOnDeath` 게이트).**
   이게 없으면 나머지가 전부 무의미하다.
2. `IScenarioResettable` 신설 + 서브시스템에 `ResetScenarioRuntimeState()` 추출 +
   `RestartScenarioInPlace()` 구현.
3. 적군 컴포넌트 Reset 구현(가장 큰 덩어리) → 이 단계에서 "적만 부활" 상태로 1차 검증 가능.
4. RCWS 탄약 리필 세터 + 포탑/게이지 리셋.
5. UGV / 드론 / 트럭 / 아군 Reset.
6. 이펙트 `RestartScenario` + DT 행 `ScenarioRestart` + `ScenarioConfig` 필드 3종.
7. 페이드 + 알림 연출.
8. 10사이클 검증 → 필요시 하이브리드(`HardReloadEveryNCycles`) 활성화.

---

## 9. 미결 / 판단 필요

- **전멸이 영영 안 오는 경우**(적 1명이 지형에 끼는 등) 데모가 멈춘다. "시나리오 시작 후 N분 경과 시
  강제 재시작" 실패 안전장치를 넣을지는 사용자 판단 — 요구에 없는 안전로직을 임의로 넣지 않는다는
  프로젝트 원칙이 있으므로 지시가 있을 때만 추가.
- 랙돌 시체를 남기는 기간(사망 후 즉시 굳힐지, 재시작까지 물리 유지할지) — 연출 취향 + 성능 실측.
- 하이브리드 주기(`HardReloadEveryNCycles`)의 적정값은 §7 검증에서 메모리 추이를 보고 정한다.
