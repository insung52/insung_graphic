# 시나리오 재시작 (확인창 + 자동 재시작) — 설계 v3

2026-09-10 작성 / 2026-09-22 전면 개정(SoldierLab 이관 반영, 요구사항 v2) / **구현 완료(2026-09-22) — 구현 기록은 `2026-09-22_scenario_restart_implementation.md`** / 완료 5초 뒤 "재시작 하시겠습니까?" 확인창(예·아니요·자동 재시작 체크박스, 값 유지, 체크 시 10초 카운트다운). 인플레이스 소프트 리셋 유지 — **병사 40명은 스냅샷 기반 재스폰, 차량·드론은 부활(revive)**. 목표 품질 = PIE 새로 시작과 동일.

> **구현에서 달라진 점(2026-09-22, **2026-09-23 항목 5~9 + 11 추가**(2-PC 5~7 · 짐벌 배율 8 · 낙하산 9 · 상태 패널 11) — 이 문서 본문은 설계 시점 그대로 둠, 최신 사실은 구현 문서 참고)**
> 1. **상황 필드 지오메트리 캐시도 비운다** — §4a·§10 의 "`Horizons`/`CoarseHorizons` 유지"는 **폐기**. `USoldierSituationFieldSubsystem::ResetForRestart` 가 Built* 를 무효화해 `EnsureLevels` 가 첫 실행과 같은 경로로 재구축("PIE 와 동일" 기준 우선, 비용은 첫 실행과 같은 점진 베이크).
> 2. **확인창이 뜨는 화면 규칙**(§4h "호스트 화면에만"은 폐기): UGV 호스트 + 자체방호 클라이언트 → **자체방호(클라이언트)에만** / 자체방호 단독 → 자체방호 / UGV 단독 → UGV / 데모·풀 시스템 무관. 서버가 `Atitan_exampleGameState::Multicast_ShowScenarioRestartPrompt(Countdown, bShowOnUGVAxis)` 로 뿌리고 각 프로세스가 자기 축을 보고 띄우며, 클라이언트의 [예]/카운트다운/체크값은 `Atitan_examplePlayerController::Server_RequestScenarioRestart / Server_SetScenarioAutoRestart` RPC 로 서버에 올라간다(체크값은 GameState `bScenarioAutoRestart` 리플리케이트).
> 3. **재스폰본 AI 빙의** — `BP_Soldier_*` 가 `AutoPossessAI=PlacedInWorld` 라 스폰된 폰엔 컨트롤러가 안 붙는다. 재스폰은 `bDeferConstruction` 스폰 → `AutoPossessAI=PlacedInWorldOrSpawned` 로 바꾼 뒤 `FinishSpawning`(BeginPlay 전 빙의, 레벨 배치본과 같은 순서).
> 4. **딸린 액터 동반 파괴** — 병사만 Destroy 하면 병사가 스폰해 손에 붙인 소총(`BP_AR4Rifle`)이 남아 매 틱 죽은 주인을 참조하고 사이클마다 두 배로 는다. 재스폰 전 `GetAttachedActors` 재귀 + Owner 기준으로 같이 파괴(풀링 투사체 제외).
> 5. **(2026-09-23, 2-PC) 정리·리셋은 "각 프로세스가 자기 것을" 한다** — 설계는 재시작을 전부 서버 처리로 봤으나, 카메라 페이드(로컬 `PlayerCameraManager`)·병사 손의 소총(`BP_AR4Rifle` `bReplicates=false`)·코스메틱 투사체 풀은 프로세스마다 따로다. `Atitan_exampleGameState` 에 **멀티캐스트 3종**(`Multicast_ScenarioRestartBegin/Apply/End`)을 두고 각 프로세스가 `UScenarioStateSubsystem::RunLocalRestartBegin/Apply/End` 를 실행한다. **전제 조건**: 레벨 GameMode 가 titan 계열(`BP_KadexTestGameMode`)이어야 한다 — `GM_SoldierLab` 오버라이드면 titan GameState 가 없어 전부 서버-로컬 폴백이 되고 **2-PC 에서만** 클라가 아무것도 못 받는다(구현 문서 §5).
> 6. **(2026-09-23) 드론 리셋은 권한별로 나눈다** — 드론만 **시뮬 주체가 서버가 아니다**(풀 시스템에서 자체방호 클라). 서버에서만 `Flight->ResetTo` 를 불러도 다음 `Server_ReportState` 에 덮인다. 그래서 `DroneFollowPath` 와 같은 규약으로 서버 = 복제되는 것(프레이밍·탐지 단계·경로 명령), 시뮬 주체 = 물리·입력·짐벌·정찰(`ResetTo`·Autopilot Disengage·`bParachuteObserved`). 차량은 반대로 `if (!HasAuthority()) return;`. §4b 의 "드론은 `Flight->ResetTo` 하나면 된다"는 단일 프로세스 한정.
> 7. **(2026-09-23) 소총 액터 누수의 근본 수정 위치는 SoldierLab 이다** — 위 4번(재시작 때 딸린 액터 동반 파괴)은 증상 대응이었고, 진짜 원인은 평상시 사망 경로에도 있던 누수였다(`BP_SoldierCharacter` 에 `K2_DestroyActor`/`EndPlay` 0개 + 엔진은 소유 액터를 안 따라 지움). `USoldierHealthComponent::EndPlay` + `bDestroyCarriedActorsOnDestroy`(기본 켬)로 해결([W116]), titan 쪽 `DestroySoldierAttachments`/`DestroyOrphanedChildActors` 는 **안전망**으로 남았다(낙하산 등 병사가 아닌 재스폰 대상용).
> 8. **(2026-09-23) 드론 짐벌 배율(`ZoomLevel`)도 리셋 대상이다** — §4b 의 드론 리셋 목록에 없던 항목. 저작값(기본 1.0)이지만 자동 정찰/교전 프레이밍이 런타임에 계속 바꾸므로, 안 되돌리면 2회차가 지난 사이클이 끝난 배율에서 시작한다. `InitialZoomLevel` 을 BeginPlay 에 스냅샷 → **시뮬 주체 구간**에서 `SetZoomLevel(InitialZoomLevel)`. **아직 빌드 전(다음 빌드 반영 예정)**. 같이 한 전수 대조에서 드론·UGV 컨트롤러의 나머지 런타임 상태 중 빠진 것은 없었고, `ViewMode`(로컬 카메라 모드)는 **리셋하지 않기로 결정**했다.
> 9. **(2026-09-23) 낙하산은 재스폰 대상이 아니다** — 설계가 "병사 + 낙하산 재스폰"으로 잡았던 것 중 낙하산은 **폐기**(사용자 확정: 정적 액터라 초기화할 필요 없음). `ScenarioConfig.RespawnActors` 는 New_kadex_0811 에서 **빈 채로 둔다**. 프로퍼티는 병사가 아닌 재스폰 대상이 생길 때를 위해 남긴다.
> 10. 소소한 것: 구 액터 Rename 안 함(§5 의 이름 재사용 옵션 미채택, `NameMode=Requested` 만), 델타 ImportText 는 FinishSpawning **뒤**(SCS 컴포넌트가 그 안에서 생김), 재스폰 스냅샷 시점은 `OnWorldBeginPlay` 인데 UE5.8 에선 액터 BeginPlay **이전**에 불림(§5 "이후" 서술은 부정확).
> 11. **(2026-09-23) 상태 패널 누적값(배터리·비행시간·주행거리)도 리셋 대상이다 — 그리고 호출 위치가 컴포넌트마다 다르다** — 설계의 리셋 목록에 HUD 상태 컴포넌트가 없었다. 드론·트럭 `UStatusHUDComponent`(배터리 = `100 − ElapsedTime×0.05` · 비행시간 · 그래프 히스토리)와 UGV `UUGVStatusComponent`(배터리 = `100 − ElapsedTime×0.03` · 누적 주행거리, 원본은 `AUGVAIController::TankTotalDistanceTraveledCm`)에 `ResetForScenarioRestart()` 를 신설했다. **드론 패널은 `CurrentData` 가 비복제 + 틱 권한 게이트가 없어 모든 프로세스가 각자 되돌려야 하고**(시뮬 주체 판정보다 **먼저** 호출), **UGV 패널은 `CurrentData` 가 복제되고 서버에서만 생성되므로 `HasAuthority` 게이트 안**이다 — 같은 재시작 안에서 정반대 배치. RCWS 탄약은 이미 되돌려지고 있었다(확인 완료). **아직 빌드 전(다음 빌드 반영 예정)**. 상세는 구현 문서 §5 끝 · `../replication/replication_audit.md` §9.

관련: `2026-09-21_soldierlab_migration_new_kadex_0811.md`(현재 레벨 구성 — 이 문서의 전제),
`../soldier_ai_lab/squad/2026-09-21_break_contact_and_targeting_exclusion.md`,
`2026-09-15_demo_ugv_autofire_on_zone1_arrival.md`, `2026-09-02_scenario_double_eval_travel_bug.md`,
`scenario_authoring_guide.md`. v2(2026-09-11, 구 BP 병사 기준)의 §4b "적군 부활" 항목은 **폐기**.

> 방식 확인(2026-09-22, 사용자): 레벨 리로드는 검토 대상이 아니다. "PIE 새로 시작과 거의 똑같게"는
> **결과의 깨끗함 기준**이지 방식 변경 요청이 아니다.

---

## 0. 요구사항 (2026-09-22 확정) 과 타임라인

1. 시나리오 완료 후 **5초** 뒤 확인창: "재시작 하시겠습니까?" + [예] [아니요] + [☐ 자동 재시작]
2. 자동 재시작 체크 시 **10초** 뒤 자동 재시작
3. 체크박스 값은 **계속 유지**(재시작해도, 다음 완료 때도 그대로)
4. 재시작 결과는 **PIE 새로 시작과 거의 동일**

```
[적 15명 전멸]  ScenarioComplete 행 → "시나리오 완료" 토스트
   │ +5s   ← 새 행 ScenarioRestartPrompt (TimerOnly 5, 이펙트 ShowRestartPrompt)
   ▼
[확인창]  예 → 즉시 재시작 / 아니요 → 닫고 대기(체크값은 유지) / 체크 ON → 10초 카운트다운("예 (10)") → 0에 재시작
   ▼
[재시작]  페이드 아웃 0.3s → 리셋(병사 재스폰·서브시스템 초기화·차량 부활, 2~5프레임) → 페이드 인 0.5s
   │ +3s   ← ScenarioConfig.DemoAutoStartDelaySeconds = 3 (실측)
   ▼
[BeginEnemyContactScenario()]  "적 특작부대 침투 상황 발생" 토스트 7s → 스텝 평가 시작 → +1s EnemyApproach/AllyDefend, +3s UAVMission …
```

"10초"의 기준은 **확인창이 뜬 시점**으로 둔다(완료 기준 15초). 완료 기준 10초가 의도면 `TriggerDelaySeconds`
5 → 0, 카운트다운 10 그대로 — 둘 다 데이터 값이라 빌드 없이 바꿀 수 있다. [가정, 확인 필요]

---

## 1. 전제 변경 — 병사는 재스폰, 차량·드론은 부활

v2는 "구 BP 병사(인스턴스 저작 데이터 多, 런타임 상태 少)"를 전제로 **부활**을 골랐다. 2026-09-21 이관으로
전제가 뒤집혔다:

| | 구 `BP_Enemy_kadex` (v2 전제) | 현 `BP_Soldier_Hostile/Friendly` (SoldierLab) |
|---|---|---|
| 인스턴스 저작 데이터 | `CombatZones` 마커 6개·자세·`AmbushMarker`… **많음** | **`SquadId`·`bSquadLeader`·트랜스폼뿐**(실측 §2) — 전투지는 `ASoldierZone` 8개 + DT 가 든다 |
| 런타임 상태 | BP 변수 ~10개 + 컴포넌트 1개 | GASP 캐릭터(MotionWarping·Traversal·GameplayCamera…) + **SoldierLab 컴포넌트 13개**(Identity/Perception/Sight/Comms/Suppression/Engagement/Cover/Health/HeadAim/ScanTurn/GaitBridge/PoseSmoother/AIBridge) + 브리지가 런타임 부착하는 `DetectableTarget` |
| 상태 소유 | 한 세션이 다 앎 | **다른 세션들이 활발히 개발 중**(헤더 7,500줄, [C] 미측정 항목 다수) |
| 결론 | 부활(필드 복원) | **재스폰** — 필드 하나씩 되돌리는 방식은 SoldierLab 이 바뀔 때마다 깨진다. 액터를 새로 만들면 BeginPlay 가 전부 다시 돌아 "PIE 와 동일"이 정의상 성립한다 |

재스폰이 안전한 근거(코드 확인):
- 브리지(`Soldiers/SoldierLabBridgeSubsystem`)가 **주기 스캔으로 새 병사를 자동 발견**해 `DetectableTarget` 을 붙이고
  (`bSoldierLabHostilesStartHidden` 이면 hidden), `USoldierRegistrySubsystem` 은 컴포넌트 BeginPlay/EndPlay 에서 자동 등록/해제.
  즉 "병사가 추가돼도 자동으로 따라간다"가 설계 의도다(헤더 주석).
- `ReinforceSquads` 가 `Identity->SquadId` 를 **런타임에 바꾼다**(`SoldierSquadSubsystem.cpp:204`) — 부활이면 되돌릴 게 하나 더 있는 셈이고, 재스폰이면 저작값으로 자연 복원.
- 리플레이(Chronicle)가 이미 "체크포인트 로드 때 레벨 액터를 클래스에서 재스폰"하며, 그때 **인스턴스 오버라이드가 CDO 로 되돌아가는** 함정이 문서화돼 있다 → 그래서 §5 의 "저작 델타 스냅샷"이 필요하다.

차량(UGV·트럭)·드론은 그대로 **부활**: RTSP 인코더 세션(`RtspBridge`, NVENC)이 액터 컴포넌트에 살아 있어 재스폰하면 스트림이 끊긴다. 드론은 `Flight->ResetTo` 가 이미 있다.
낙하산(`BP_Parachute_C_3`)은 단순 액터라 **재스폰** 쪽에 넣는다(1회성 강하 타임라인을 되돌릴 필요가 없어짐).

---

## 2. 실측·코드 확인 사실 (2026-09-22)

| 항목 | 값 | 의미 |
|---|---|---|
| `ScenarioConfig_1` | RunMode **Demo**, `DemoAutoStartDelaySeconds` **3**, `bDemoAutoStartScenario` true, `bDemoForceCommandPostAutoFire` true, `ScenarioStepTable = DT_ScenarioSteps_ThreeStage_SoldierLab`, `bSoldierLabHostilesStartHidden` true | 재시작 뒤 3초 자동 시작 경로 재사용 |
| 병사 | `BP_Soldier_Hostile_C_1..15`, `BP_Soldier_Friendly_C_1..25` | 40 |
| 병사 인스턴스 저작값 | `AC_SoldierIdentity`: `Faction`(클래스), **`SquadId="1"`, `bSquadLeader=true`**(인스턴스). `AC_SoldierHealth`: `bInvincible`(아군 true, 사용자 체크), `MaxHealth 100` | §5 스냅샷이 잡아야 할 것 |
| 병사 컴포넌트 | 37개(GASP 계열 + SoldierLab 13 + 브리지 부착 1) | 필드 복원 불가 → 재스폰 |
| 사망 처리 | `USoldierHealthComponent`: Death 몽타주 → 래그돌, `DestroyAfterSeconds>0` 이면 타이머로 Destroy(weak 람다), 사망 시 `Registry->Unregister` | 재시작이 먼저 Destroy 해도 안전 |
| SoldierLab 월드 서브시스템 | `USoldierRegistrySubsystem`(등록부, 자동), `USoldierSquadSubsystem`(`Squads`/`Members` 맵 + `NextOrderSerial`), `USoldierSituationFieldSubsystem`(`Scopes`=분대별 셀·라이트·앵커·퇴거큐 **동적** / `Horizons`·`CoarseHorizons` **지오메트리 캐시**), `USoldierProjectilePool`(`Rounds`) | §4a 리셋 계약 |
| titan 서브시스템 | `USoldierLabBridgeSubsystem`(`Soldiers` 배열, `ScanIntervalSeconds` 주기 스캔), `UDetectableTargetSubsystem`(EndPlay 자동 해제), `UScenarioStateSubsystem`(GameInstance) | |
| UGV RCWS 데모 자동사격 | **레벨 시작이 아니라 `UGVArriveZone1`(ActorStopped, 이펙트 `SetDemoUGVAutoFire`) 에서 켜짐**(09-15). `ApplyDemoRCWSAutoFire()` 는 지휘소만 | UGV 리셋 = RCWS `Remote` 로 되돌리기 |
| 확인창 인프라 | `UNotificationSubsystem::ShowConfirmDialog` + `OnConfirmed/OnCancelled`(체크박스 없음) | 새 위젯 클래스 필요(§6) |
| DT `ScenarioComplete` | Prereq **`EnemyEngage`**, `AllEnemiesEliminated`, `ShowUIMessage(ScenarioComplete)` | 루프 브레이커 유지 |
| DT `Prereq=None` 행 | `UAVMission`(3s) `EnemyApproach`(1s) `AllyDefend`(1s) `EnemyEngage`(UGVFiredNearEnemy 100m) `EnemyFleeToZone2`(사망≥3) `AllyEngage`(EnemyNearFriendlySoldiers 80m) | §3 |
| 이 레벨에서 죽은 코드 | `UEnemyCombatComponent`/`UAllyFormationComponent` 경로 전부(대상 0명, 이관 문서 §7) | v2 §4b/c 폐기. `BP_Enemy_Base` 노드 삭제도 불필요 |

---

## 3. 무한 재시작 · 오발동 분석 (새 DT 기준)

재시작이 `FiredScenarioSteps` 를 비우므로 `Prereq=None` 6행이 즉시 재평가 대상이 된다:

| 행 | 재시작 직후 | 판정 |
|---|---|---|
| `UAVMission`/`EnemyApproach`/`AllyDefend` | 타이머 — 3s/1s/1s 뒤 정상 | ✅ |
| `EnemyEngage` (UGV 100m 내 사격) | `UGVFireWatch` 리셋 → `LastShotsFired=INDEX_NONE` → 첫 틱은 기준선만(`UpdateRCWSFireWatch`) | ✅ RCWS `ShotsFiredCount` 는 리셋 불필요 |
| `AllyEngage` (적 보병이 아군 80m 내) | 재스폰 위치: 적 x≈−30500, 아군 x≈23000+ → 500 m 이상 | ✅ 단, **시체가 남아 있으면** 판정에 들어갈 수 있음 → 재스폰 전에 구 액터 전부 Destroy |
| `EnemyFleeToZone2` (사망≥3) | `사망 = Baseline − Alive`. 재스폰 전에 평가가 돌면 Alive=0 → 즉시 발동 | ⚠ **평가 루프를 먼저 끄고, 재스폰 완료 + 3초 뒤에만 다시 켠다**(§7 순서). `BeginScenarioSteps` 가 그때 `Baseline=CountAliveEnemies()=15` 로 다시 잡는다 |

`ScenarioComplete` 는 Prereq `EnemyEngage` 라 새 사이클에서 UGV 가 다시 쏘기 전엔 발동 불가 → **완료→재시작→즉시 완료 폭주는 구조적으로 불가능**. 확인창 행 `ScenarioRestartPrompt` 도 Prereq `ScenarioComplete` 라 같은 보호를 받는다.

지켜야 할 것 세 가지(v2 와 동일, 이유는 `2026-09-02_scenario_double_eval_travel_bug.md`):
1. **`ClearTimer(ScenarioStepTickTimerHandle)` 로 루프를 실제로 끈다.** 재시작은 같은 월드라 `ResetForNewWorldIfNeeded` 의 월드 비교가 안 걸리고, `BeginScenarioSteps` 는 타이머가 살아 있으면 조용히 리턴한다("눌렀는데 아무 일도 안 남").
2. **재스폰 → 3초 → 스텝 시작** 순서.
3. **`bRestartInProgress` 재진입 잠금** — 확인창의 [예]와 카운트다운, 콘솔 명령이 겹칠 수 있다.

"정지" 위험(폭주 아님): 재스폰이 실패해 적이 0명이면 `EnemyEngage` 가 영영 안 걸려 완료도 확인창도 안 뜬다.
→ §7-8 의 재스폰 검증(등록부 Hostile 15 / Friendly 25 / Detectable Enemy 15)이 어긋나면 **에러 로그 + 확인창을 즉시 다시 띄운다**(사람이 [예]로 재시도 가능). 시간 기반 강제 재시작은 넣지 않는다(요구에 없음).

---

## 4. 리셋 대상 — 층별

### (a) SoldierLab 월드 서브시스템 — **SoldierLab 모듈 안에 리셋 계약을 둔다**

의존 방향은 titan → SoldierLab 한쪽뿐(분대 층 결정). 그러므로 리셋 API 는 SoldierLab 이 소유하고 titan 이 호출만 한다.
SoldierLab 이 바뀌면 그쪽 세션이 자기 리셋을 같이 고치는 구조. 시험 레벨 `L_SoldierScenario` 에서 콘솔 `SoldierLab.ResetWorld` 로 단독 검증 가능하게.

| 서브시스템 | 리셋 | 근거 |
|---|---|---|
| `USoldierSquadSubsystem::ResetForRestart()` | `Squads.Reset()`, `Members.Reset()`, `NextOrderSerial=1`, `LastStatusLogSeconds=-1` | 명령·정원 편입 이력이 다음 사이클에 남으면 `SquadOrderAchieved`·정원 판정이 어긋남 |
| `USoldierSituationFieldSubsystem::ResetForRestart()` | **`Scopes` 전부 비움**(셀·라이트·앵커·퇴거큐), `AllAnchors`, `PendingBakes`, `NextHorizonSnapshot`. **`Horizons`/`CoarseHorizons` 는 유지**(지오메트리 캐시 — 새 PIE 가 다시 구울 값과 같으므로 남겨도 동일, 오히려 트레이스 절약) | "안 본 곳 = 적이 있다" 모델이라 이전 사이클의 목격 라이트가 남으면 2회차 침투 거동이 달라진다 |
| `USoldierProjectilePool::RecallAll()` | 날아가는 탄 전부 풀로 회수(비활성) | |
| `USoldierRegistrySubsystem` | 없음 — Unregister 는 EndPlay 자동. 단 재스폰 뒤 **카운트 검증**(Hostile 15 / Friendly 25) | |

### (b) 병사 40 + 낙하산 — 재스폰 (§5)

### (c) titan 서브시스템

- `UScenarioStateSubsystem::ResetScenarioRuntimeState()` — `ResetForNewWorldIfNeeded()` 본문을 함수로 추출(두 경로가 같은 함수). 타이머 3종 ClearTimer, `FiredScenarioSteps`/`ScenarioStepFireTimes`/`ScenarioStepsStartTime`, `ScenarioEnemyCountBaseline=0`(Max 로만 오르므로 반드시), `ScenarioAllyFireCountBaseline`, `LoggedEnemyDeaths`/`TrackedEnemies`/`LastEnemyRosterRefreshTime`, `UGVFireWatch`/`CommandPostFireWatch`, `FormUpLeader`/`bUGVAdvanceTriggered`/집결 상태, `bHasEnemyPredictedLocation`, `RemoveUGVNavObstacle()`. (구 `ScenarioZoneRoles`도 같이 — 죽은 코드지만 무해)
- `USoldierLabBridgeSubsystem` — `Soldiers` 배열의 무효 항목 정리 + **재스폰 직후 즉시 재스캔**(`ForceRescan()` 신설). 안 하면 `ScanIntervalSeconds` 동안 적군이 hidden 이 아닌 채 보인다(PIE 시작에도 같은 창이 있지만 페이드로 가리려면 명시 호출이 맞다).
- `UDetectableTargetSubsystem` — EndPlay 자동. 재스폰 뒤 Enemy 15(hidden) 검증.
- GameState: `EScenarioPhase` 초기값, 미니맵 적 예상 위치.
- `UNotificationSubsystem` — 떠 있는 토스트/확인창 정리.

### (d) UGV (`BP_UGV_0901_C_1`) — 부활

- `AUGVAIController`: 경로/코리도/**도착 정렬 페이즈(`EUGVArrivalPhase`)** 초기화, `SetUGVEnemyDistanceSpeedLimit(false)`. 리셋 뒤 `IsMoving()==false` 여야 `UGVArriveZone1`(ActorStopped) 이 1회차와 같은 타이밍에 걸린다.
- **RCWS 모드 `Remote` 로**(09-15: 1차 도착 전 UGV 는 Remote, 스윕 없음). 안 되돌리면 2회차엔 출발 전부터 스윕한다.
- **`FireControl->bRespectEnemyTargetingExclusion=false`**(`IssueSquadOrderSpec SetTargetable(false)` 가 켠 것 — 안 끄면 2회차부터 UGV 가 일부 적을 영영 안 쏨).
- **탄약 `CurrentData.AmmoCurrent = AmmoMax`** — `CurrentData` private, 리로드 없음 → `URCWSComponent::RefillAmmo()` 신설. 600발이라 2~3사이클이면 마르고 그러면 `EnemyEngage` 가 영영 안 걸린다.
- 포탑 방위/고각 원위치, 락온 게이지·배럴 스핀·줌 램프, 스티키 표적(W106) 해제.
- Chaos: `SetActorTransform(TeleportPhysics)` + 선/각속도 0 + 입력 0. 텔레포트 직후 1~2틱 튐 → 페이드 아래.
- `UGVAvoidanceProxyComponent`, 브리지가 붙인 `USoldierIdentityComponent`(Friendly, 차량) 는 그대로.

### (e) 이동형지휘소 (`BP_TitanTruck_C_4`) — 부활

RCWS 탄약/모드/포탑/게이지. 위치 불변. 이후 `ApplyDemoRunModeSetup()` 재실행이 ARM+AutoFire+Burst 를 다시 건다.

### (f) 드론 — 부활

`Flight->ResetTo(SpawnLocation+(0,0,200), SpawnYawDegrees)`(기존), **`bParachuteObserved=false`**(안 하면 `UAVSpotted` 즉시 참 → UGV 즉시 출발), 오토파일럿 상태/스플라인 진행도, `RecenterGimbal()`, 프레이밍 `None`, `EDroneDetectionPhase` 초기, `EDroneGimbalReconPhase` 초기.

### (g) 월드 잔재

`ARCWSProjectile` 전부 Destroy, 루프 사운드 정지, 탄흔 데칼(수명 확인), 토스트 숨김, 대시보드 카운터.

### (h) 리플리케이션

전부 서버 권위. 재스폰 액터는 새 채널로 클라에 생기고 브리지 `DetectableTarget` 도 복제된다(09-21). 확인창은 호스트(서버) 화면에만.

---

## 5. 스냅샷 · 재스폰 메커니즘

새 월드 서브시스템 `UScenarioRespawnSubsystem`(titan, `UI/` 또는 `Soldiers/`):

**대상 판정**: `USoldierHealthComponent` 를 가진 액터(병사 40) + `AScenarioConfig::RespawnActors`(명시 목록: 낙하산). 병사 BP 는 건드리지 않는다.

**스냅샷 시점**: `OnWorldBeginPlay`(액터 BeginPlay 이후, 첫 틱 전) — `ReinforceSquads` 가 `SquadId` 를 바꾸기 전(+210s)이라 저작값 그대로.

**스냅샷 내용**(`FScenarioActorSnapshot`, 월드 오브젝트 강참조 금지 — [[feedback_strong_object_ptr_pie_world_leak]]):
- `TSubclassOf<AActor> Class`, `FTransform`, `FName OriginalName`, `Tags`, `bHidden`, 폴더(로그용)
- **저작 델타**: 액터 + 각 컴포넌트(이름 매칭)에 대해 `CPF_Edit && !CPF_Transient && !CPF_DisableEditOnInstance` 프로퍼티 중 **아키타입(CDO/컴포넌트 템플릿)과 다른 것만** `ExportText` 로 `TMap<FName Component, TMap<FName Property, FString Text>>`. 실측 기대치: `AC_SoldierIdentity.SquadId/bSquadLeader`, 아군 `AC_SoldierHealth.bInvincible`, 그 외 0~2개. 로그에 델타 목록을 찍어 첫 실행에서 눈으로 확인.

**재스폰**: 구 액터 `Destroy()` → (이름 재사용 원하면 구 액터를 `Rename(nullptr, GetTransientPackage())` 로 비켜 두고 `SpawnParams.Name=OriginalName` — 로그의 `BP_Soldier_Hostile_C_4` 같은 이름이 1회차와 같아져 비교가 쉬움, 선택) → `SpawnActorDeferred` → 델타 `ImportText` → `FinishSpawning` → 생성자·CS·BeginPlay 전부 새로. **프레임 분산**: 8구/프레임(GASP 캐릭터 40 동시 스폰 스파이크 회피, 페이드 0.3s 안에 끝남).

**검증**: 재스폰 뒤 `Registry` Hostile/Friendly 수, `DetectableTarget` Enemy 수·hidden 여부, 델타 적용 수를 한 줄 로그. 어긋나면 §3 정책.

---

## 6. 확인창 + 자동 재시작 체크박스

### 6.1 위젯 — C++ `URestartPromptWidget : UUserWidget` + 사용자가 만드는 `WBP_RestartPrompt`

MCP 로는 UMG 이벤트 배선이 안 되므로(Chronicle 결론) **BindWidget 이름 고정** 방식. WBP 는 사용자가 만든다([[feedback_dont_create_assets_via_mcp]]). 위젯 이름 규격:

| 위젯 이름 | 타입 | 필수 |
|---|---|---|
| `YesButton` | `UButton` | ✓ |
| `NoButton` | `UButton` | ✓ |
| `AutoRestartCheckBox` | `UCheckBox` | ✓ |
| `YesLabel` | `UTextBlock` | 선택 — 카운트다운 "예 (7)" 표기 |
| `MessageText` | `UTextBlock` | 선택 — 기본 "시나리오를 재시작하시겠습니까?" |

동작(C++):
- 열릴 때 `bAutoRestartEnabled` 를 읽어 체크박스 초기화. 체크면 즉시 카운트다운 시작.
- `AutoRestartCheckBox` 토글 → 값 즉시 저장(§6.3) + 켜지면 카운트다운 시작 / 꺼지면 취소.
- [예] → `RequestScenarioRestart(bForce=true)`. [아니요] → 닫기 + 카운트다운 취소(**체크값은 그대로**).
- 카운트다운 0 → `RequestScenarioRestart()`. 길이 = `AScenarioConfig::AutoRestartCountdownSeconds`(기본 10).
- 모달(입력 잠금)은 기존 `UConfirmDialogWidget` 과 같은 방식. 한 번에 하나만.

소유: `UNotificationSubsystem::ShowRestartPrompt()` / `HideRestartPrompt()` — 확인창·토스트와 같은 자리(GC 강참조, 뷰포트 배치, 모니터 위치 규칙 재사용). `RestartPromptWidgetClass` 는 `WBP_ConfirmDialog` 와 같은 ConstructorHelpers 기본값 패턴.

⚠ 기존 `OnConfirmed/OnCancelled` 는 **서브시스템 단위 멀티캐스트**라 다른 확인창(종료 등)과 공유된다. 재시작창은 그 델리게이트를 쓰지 않고 자기 버튼을 직접 바인딩한다.

### 6.2 트리거 — DT 행

| RowName | Prereq | Trigger | 값 | Effect |
|---|---|---|---|---|
| `ScenarioRestartPrompt` | `ScenarioComplete` | `TimerOnly` | **5** | `ShowRestartPrompt`(신설) |

이펙트 구현부: 서버 + 로컬 플레이어가 있을 때만(클라/헤드리스 no-op). 데모/풀 시스템 구분 없이 뜬다(요구사항에 구분 없음). 풀 시스템에서 끄고 싶으면 `bEnabled=false` 또는 `ScenarioConfig.bRestartPromptEnabled`.

### 6.3 체크값 유지

- 런타임: `UScenarioStateSubsystem::bAutoRestartEnabled`(GameInstance 서브시스템 → 재시작·레벨 트래블을 넘어 산다).
- **앱 재실행에도 유지**: `GConfig` `GGameUserSettingsIni` `[Scenario] bAutoRestart=` 로 저장/로드(`Initialize` 에서 읽음). 전시장에서 한 번만 체크하면 재부팅 뒤에도 무인 반복. 커맨드라인 `-autorestart` 로도 켤 수 있게(선택).

### 6.4 진입점 통일

```cpp
UFUNCTION(BlueprintCallable, Exec) bool RequestScenarioRestart(bool bForce = false);
```
확인창 [예]·카운트다운·콘솔 `RequestScenarioRestart`·향후 HUD 버튼이 전부 이걸 탄다. `bRestartInProgress` 면 false.

---

## 7. 실행 시퀀스 (`RequestScenarioRestart`)

| # | 동작 | 이유 |
|---|---|---|
| 1 | 잠금 + `HideRestartPrompt()` + `ClearTimer(ScenarioStepTickTimerHandle)`, `bScenarioStepsRunning=false` | §3-1 |
| 2 | 페이드 아웃 0.3s | 텔레포트·스폰 팝 가림 |
| 3 | 잔재 정리: `SoldierProjectilePool::RecallAll`, `ARCWSProjectile` Destroy, 사운드, 토스트 | |
| 4 | 병사 40 + 낙하산 `Destroy()` (시체 포함) | `AllyEngage` 오판정 차단 |
| 5 | SoldierLab 서브시스템 리셋(§4a) → titan 서브시스템 리셋(§4c, `ResetScenarioRuntimeState`) | 순서: 액터가 사라진 뒤 |
| 6 | 차량·드론 부활(§4d~f) | |
| 7 | **다음 프레임부터** 재스폰 8구/프레임(§5) → 끝나면 브리지 `ForceRescan()` | 물리·스폰 분산 |
| 8 | 검증 로그: 등록부 15/25, Detectable Enemy 15 hidden, 델타 적용 수, UGV/트럭 탄 600/600 | 어긋나면 에러 + 확인창 재표시 |
| 9 | `ApplyDemoRunModeSetup()`(지휘소 ARM+AutoFire+Burst) | 레벨 시작과 동일 |
| 10 | 페이드 인 0.5s + `DemoAutoStartDelaySeconds`(3s) 타이머 | |
| 11 | `BeginEnemyContactScenario()` → EnemyContact 토스트 → `BeginScenarioSteps`(Baseline=15) | 기존 경로 |
| 12 | 잠금 해제, 사이클 카운터 +1, 요약 로그 1줄 | |

---

## 8. 변경 목록 (파일별)

**SoldierLab 모듈** (리셋 계약 — 이 모듈 세션과 조율)
1. `Squad/SoldierSquadSubsystem.*` — `ResetForRestart()`
2. `AI/SoldierSituationField.*` — `ResetForRestart()`(Scopes 만, 지오메트리 유지)
3. `Weapons/SoldierProjectilePool.*` — `RecallAll()`
4. 콘솔 `SoldierLab.ResetWorld`(위 셋 호출) — 시험 레벨 단독 검증용

**titan_example**
5. `UI/ScenarioRespawnSubsystem.*` 신설 — 스냅샷/재스폰/검증(§5)
6. `UI/ScenarioStateSubsystem.*` — `ResetScenarioRuntimeState()` 추출, `RequestScenarioRestart()`, `bRestartInProgress`, `bAutoRestartEnabled`(+ini 저장), 사이클 카운터, 시퀀스(§7)
7. `UI/ScenarioStepTypes.h` — `EScenarioEffectType::ShowRestartPrompt`; `ExecuteScenarioEffect` 케이스
8. `UI/ScenarioConfig.h` — `AutoRestartCountdownSeconds=10`, `RestartFadeOutSeconds=0.3`/`RestartFadeInSeconds=0.5`, `RespawnActors`(낙하산), `bRestartPromptEnabled=true`
9. `UI/RestartPromptWidget.*` 신설 + `UI/NotificationSubsystem.*` — `ShowRestartPrompt/HideRestartPrompt`, `RestartPromptWidgetClass`
10. `Soldiers/SoldierLabBridgeSubsystem.*` — `ForceRescan()`, `Soldiers` 무효 항목 정리
11. `Vehicles/RCWSComponent.*` — `RefillAmmo()`; `Vehicles/RCWSFireControlComponent.*` — `ResetForScenarioRestart()`(모드 Remote·게이지·스티키 표적·`bRespectEnemyTargetingExclusion=false`)
12. `Vehicles/UGVAIController.*` / `UGV0901Pawn.*` / `TitanTruck.*` / `Drone/DronePawn.*` — `IScenarioResettable::ResetForScenarioRestart()`(§4d~f). `UI/ScenarioResettable.h` 인터페이스 신설
13. 페이드(플레이어 카메라 매니저 `StartCameraFade` 로 충분)

**에셋/데이터** (사용자·MCP)
14. `WBP_RestartPrompt` — 사용자 생성(§6.1 이름 규격), 부모 `URestartPromptWidget`
15. DT `DT_ScenarioSteps_ThreeStage_SoldierLab` 행 `ScenarioRestartPrompt` 추가(MCP `set_rows` — `squadOrder` 구조체는 통째로 써야 함, [[feedback_mcp_datatable_struct_partial_write]])
16. `ScenarioConfig_1.RespawnActors = [BP_Parachute_C_3]`

**빌드 주의**: 헤더에 UPROPERTY/UFUNCTION 이 늘어나므로 Live Coding 이 아니라 정식 빌드([[feedback_live_coding_uproperty_missing_property]]). 빌드는 사용자가 한다.

**순서 제안**: 5→6→7→8→10→15(콘솔 `RequestScenarioRestart` 로 자동 재시작 없이 1차 검증) → 1~4 → 11~12 → 9·14(확인창) → 13 → 검증.

---

## 9. 검증 — "PIE 와 동일"의 판정법

1회차(PIE 직후)와 2회차(재시작 후)의 로그를 나란히 놓는다:

| 항목 | 같아야 하는 것 |
|---|---|
| 스텝 발동 순서·간격 | `[ScenarioStateSubsystem] 시나리오 스텝 발동:` 26행 순서, 각 +s 가 ±수 초 |
| 분대 | `[Squad] reinforce … quota=10 living=…` 형태·`[Squad] order … BreakContact` 5명 |
| 병사 수 | 재스폰 직후 등록부 15/25, `DetectableTarget` Enemy 15(hidden) — 1회차 BeginPlay 직후와 동일 |
| 저작 델타 | 스냅샷 로그의 델타 목록이 매 사이클 동일하게 적용 |
| 차량 | UGV 탄 600, RCWS Remote → `UGVArriveZone1` 에서 AutoFire, 트럭 600 |
| 드론 | `UAVSpotted` 가 1회차와 비슷한 +s 에(즉시 발동하면 `bParachuteObserved` 리셋 누락) |
| 성능 | 재시작 프레임 스파이크(페이드 안), 10사이클 뒤 `stat memory`·fps 1회차 대비 |
| 스트림 | RTSP 재접속 없음 |

10사이클 연속 자동(체크박스 ON) 뒤 위 표를 다시 본다. **2회차**가 핵심 — 이 설계의 함정은 전부 "1회차는 멀쩡, 2회차부터 어긋남" 유형이다.

---

## 10. PIE 와 남는 차이 · 리스크 · 미결

- 남는 차이(의도적): 상황 필드 지오메트리 캐시(`Horizons`) 유지, RCWS `ShotsFiredCount` 누적, 월드 시간 연속(SoldierLab 은 `NowSeconds` 차분만 씀 — 절대시간 의존 코드가 생기면 깨질 수 있어 검증 항목에 둠), 브리지가 차량에 붙인 Identity 유지.
- 재스폰 프레임 비용 [C] — 8구/프레임이 부족하면 4구/프레임 + 페이드 연장.
- 클라이언트(자체방호 PC)의 위젯·HUD 캐시 — 2-PC 검증 때 확인.
- "10초"의 기준(확인창 기준 vs 완료 기준) — §0 가정.
- 풀 시스템에서 확인창을 띄울지 — 기본 켬, `bRestartPromptEnabled` 로 끌 수 있게.
- SoldierLab 리셋 API 는 그 모듈 세션이 자기 필드를 아는 채로 구현하는 게 맞다 — 이 문서는 계약(무엇을 비우고 무엇을 남기는가)만 정한다.
