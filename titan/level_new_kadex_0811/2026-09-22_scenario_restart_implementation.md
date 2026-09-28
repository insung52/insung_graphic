# 시나리오 재시작(확인창 + 자동 재시작) 구현

2026-09-22(2026-09-23 2-PC 절 추가) / **완료(L_SoldierScenario 단일 프로세스 ✅ · New_kadex_0811 2-PC 검증 완료 — 사용자 "이제 잘됨")** / 설계 v3(`2026-09-10_scenario_auto_restart_design.md`)대로 구현: 병사 스냅샷 재스폰(낙하산은 09-23 에 대상에서 빠짐) + 차량·드론 제자리 부활 + SoldierLab 리셋 계약 + 확인창/체크박스(ini 유지). 정식 빌드 통과, DT 2개에 `ScenarioRestartPrompt` 행(이펙트 `ShowRestartPrompt`) 저장, `WBP_RestartPrompt` 제작 완료. 2026-09-23 2-PC 테스트에서 **레벨 GameMode 오버라이드가 `GM_SoldierLab` 로 바뀌어 있던 것**이 클라 페이드/드론 미복귀의 최종 원인(§5), 그 과정에서 멀티캐스트 3종 · 드론 권한 분리 · 소총 누수 근본 수정이 추가됐다. 그 뒤 **드론 짐벌 배율(`ZoomLevel`) 리셋 누락**(§5)과 **상태 패널 누적값(배터리·비행시간·주행거리) 미초기화**(§5 끝)를 잡았는데, 이 둘은 **아직 빌드 전 — 다음 빌드 반영 예정**이다. 낙하산 `RespawnActors` 연결은 **불필요로 종결**(정적 액터, §3).

설계 근거·무한 재시작 분석·리셋 목록은 설계 문서에 있고, 이 문서는 **무엇을 어디에 만들었고 다음에 뭘 해야 하는가**만 적는다.

---

## 1. 흐름 (구현된 대로)

```
ScenarioComplete(적 전멸) → DT 행 ScenarioRestartPrompt(+5s, 이펙트 ShowRestartPrompt)
  → UScenarioStateSubsystem::ShowRestartPrompt() → GameState Multicast_ShowScenarioRestartPrompt(10s, bShowOnUGVAxis)
  → (자기 축이면) UNotificationSubsystem::ShowRestartPrompt(10s)
  → WBP_RestartPrompt(부모 URestartPromptWidget): [예] / [아니요] / [☐ 자동 재시작]
       체크 ON → "예 (10)" 카운트다운 → 0 에 재시작 요청
       [예] → 즉시 / [아니요] → 닫기(체크값 유지)
       위젯 → 서브시스템 RequestScenarioRestartFromLocalUI / RequestSetAutoRestartFromLocalUI
         → PlayerController RequestScenarioRestart / SetScenarioAutoRestart(서버면 로컬, 클라면 Server_* RPC)
  → (서버) UScenarioStateSubsystem::RequestScenarioRestart()   ← 유일 진입점(확인창·카운트다운·콘솔·향후 HUD 버튼)
       1 확인창 닫기 + 스텝 루프 ClearTimer                         (이 프레임)
       2 Multicast_ScenarioRestartBegin(0.3) → 각 프로세스 RunLocalRestartBegin:
         페이드 아웃, 확인창/토스트 닫기, 로컬 탄 주차, 로컬 딸린 액터 파괴 (§5)
       3 ExecuteScenarioReset: UScenarioRespawnSubsystem::BeginRespawnAll(병사 40 + 낙하산 Destroy),
         SoldierLab SquadSubsystem/SituationField ResetForRestart, titan ResetScenarioRuntimeState(true),
         GameState 적 예상 위치/페이즈 초기화,
         Multicast_ScenarioRestartApply → 각 프로세스가 자기가 시뮬하는 IScenarioResettable 리셋
         (UGV·트럭 = 서버 / 드론 = 시뮬 주체, §5)
       4 다음 프레임부터 RestartRespawnPerFrame(8)씩 재스폰 + 저작 델타 복원
       5 ContinueScenarioRestartAfterRespawn: 브리지 ForceRescan(적 hidden), 검증(등록부/탐지 레지스트리),
         ApplyDemoRunModeSetup()(지휘소 ARM+AutoFire, 3s 뒤 BeginEnemyContactScenario),
         Multicast_ScenarioRestartEnd(0.5) → 각 프로세스 RunLocalRestartEnd: 고아 청소 + 페이드 인
       6 FinishScenarioRestart: 사이클 카운터 +1, 소요 ms 로그. 실패 시 페이드 복구 + 확인창 재표시
```

콘솔: `titan.ScenarioRestart`(확인창 없이 즉시), `titan.ScenarioAutoRestart <0|1>`(체크값), `SoldierLab.ResetWorld`(SoldierLab 셋만 — 시험 레벨 단독 검증용). 커맨드라인 `-autorestart`.

---

## 2. 변경 파일

### 신규 (P4 add)
| 파일 | 내용 |
|---|---|
| `Source/titan_example/UI/ScenarioResettable.h` | `IScenarioResettable`(BlueprintNativeEvent `ResetForScenarioRestart`) — 제자리 부활 계약 |
| `Source/titan_example/UI/ScenarioRespawnSubsystem.h/.cpp` | 월드 서브시스템. `OnWorldBeginPlay`(UE5.8 은 액터 BeginPlay **이전**에 불린다 — `World.cpp` 확인)에서 `USoldierHealthComponent` 보유 액터 + `ScenarioConfig::RespawnActors` 의 클래스/트랜스폼/**저작 델타**(액터·컴포넌트별 EditAnywhere 중 아키타입과 다른 값을 ExportText — 실측 `AC_SoldierIdentity.SquadId/bSquadLeader`, 아군 `AC_SoldierHealth.bInvincible`) 스냅샷. `BeginRespawnAll(PerFrame, OnComplete)` 가 병사 + **딸린 액터**(`GetAttachedActors` 재귀 + Owner 기준, 풀링 투사체 제외 — 소총 `BP_AR4Rifle` 등) Destroy → 다음 프레임부터 `RestartRespawnPerFrame`(8)씩 `bDeferConstruction` 스폰 → **`AutoPossessAI=PlacedInWorldOrSpawned`** 로 바꿔 `FinishSpawning`(BeginPlay 전 AI 빙의, 없으면 `SpawnDefaultController` 안전망) → ImportText 델타 복원. 델타 목록은 Verbose 로그. **(2026-09-23 추가)** ① `DestroySoldierAttachments()` / `DestroyOrphanedChildActors()` — 각 프로세스가 **자기가 스폰한 것만**(`ROLE_Authority`) 치우는 로컬 정리(§5, 소총 근본 수정 뒤에는 **안전망**). ② 스냅샷에서 **에디터 시각화용 임시 컴포넌트 제외** — `CameraProxyMeshComponent_N`/`DrawFrustumComponent_N`/`OutputCameraComponent` 는 PIE 에서 카메라 컴포넌트가 스스로 만들고 스폰마다 번호가 달라 "재스폰본에 없음" 경고가 났고, `RF_TextExportTransient` 라 ExportText 가 빈 문자열이어서 적용도 실패했다. 판별: 아키타입이 자기 클래스 CDO(`RF_ClassDefaultObject`) 이거나 `RF_Transient｜RF_TextExportTransient`, `IsEditorOnly()` |
| `Source/titan_example/UI/RestartPromptWidget.h/.cpp` | 확인창 C++ 부모(§4). 버튼/체크박스는 서브시스템의 `RequestScenarioRestartFromLocalUI` / `RequestSetAutoRestartFromLocalUI` 를 부른다(클라이언트에서도 동작) |

### 수정
| 파일 | 내용 |
|---|---|
| `UI/ScenarioStateSubsystem.h/.cpp` | **(2026-09-23 추가)** `RunLocalRestartBegin(FadeOutSeconds)` / `RunLocalRestartApply()` / `RunLocalRestartEnd(FadeInSeconds)` — 각 프로세스가 로컬로 실행하는 3단(§5). GameState 멀티캐스트의 수신부이고, titan GameState 가 없는 월드에선 서버가 직접 부르는 **로컬 폴백**. 재시작 시작 시 **GameState 클래스 진단 경고**(`Atitan_exampleGameState` 가 아니면 경고, Standalone 제외) |
| `UI/ScenarioStateSubsystem.h/.cpp` | `RequestScenarioRestart`(유일 진입점) / `ShowRestartPrompt` / `Is·SetAutoRestartEnabled`(ini `[Scenario] bAutoRestart`; `Is…` 는 클라이언트에선 GameState 리플리케이트 값) / **`RequestScenarioRestartFromLocalUI` / `RequestSetAutoRestartFromLocalUI`**(위젯 → PlayerController 경유, 클라이언트면 RPC) / `GetScenarioCycleCount` / `Initialize`(ini·`-autorestart`); `ResetForNewWorldIfNeeded` 본문을 **`ResetScenarioRuntimeState(bClearTimersOnCurrentWorld)`** 로 추출(월드 교체=false, 재시작=true — 새 월드에 옛 핸들로 ClearTimer 치면 남의 타이머를 끈다); 시퀀스 `ExecuteScenarioReset` → `ContinueScenarioRestartAfterRespawn` → `FinishScenarioRestart`; 콘솔 `titan.ScenarioRestart` / `titan.ScenarioAutoRestart`; 이펙트 케이스 `ShowRestartPrompt` |
| `UI/ScenarioStepTypes.h` | `EScenarioEffectType::ShowRestartPrompt` |
| `UI/ScenarioConfig.h` | `bRestartPromptEnabled`, `AutoRestartCountdownSeconds=10`, `RestartFadeOutSeconds=0.3`, `RestartFadeInSeconds=0.5`, `RestartRespawnPerFrame=8`, `RespawnActors` |
| `UI/NotificationSubsystem.h/.cpp` | `ShowRestartPrompt/HideRestartPrompt/IsRestartPromptOpen`, `HideAllToasts`, `RestartPromptWidgetClass`(기본 `/Game/widget/notify/WBP_RestartPrompt`), 열린 동안 커서 표시 |
| `titan_exampleGameState.h/.cpp` | `ClearEnemyPredictedLocation`, `Multicast_ShowScenarioRestartPrompt(Countdown, bShowOnUGVAxis)` / `Multicast_HideScenarioRestartPrompt`(서버가 자체방호 플레이어 접속 여부를 판정해 인자로, 각 프로세스가 자기 축을 보고 띄움 — 축 없는 PIE/시험 레벨은 무조건), `bScenarioAutoRestart`(Replicated, 클라 확인창 초기값) + `SetScenarioAutoRestart`; **(2026-09-23)** `Multicast_ScenarioRestartBegin(FadeOutSeconds)` / `Multicast_ScenarioRestartApply()` / `Multicast_ScenarioRestartEnd(FadeInSeconds)` 3종 — 각각 서브시스템의 `RunLocalRestartBegin/Apply/End` 를 부른다(§5) |
| `titan_examplePlayerController.h/.cpp` | `RequestScenarioRestart` / `SetScenarioAutoRestart`(Exec, 서버면 로컬·클라면 RPC) + `Server_RequestScenarioRestart` / `Server_SetScenarioAutoRestart`(Server RPC) — 클라이언트 [예]/카운트다운/체크값이 서버로 올라가는 경로. **(2026-09-23)** 앞의 둘은 **`public` 구역**이어야 한다 — 서브시스템(`RequestScenarioRestartFromLocalUI`)이 부르는데 `protected` 라 빌드 에러 **C2248** 이 났다 |
| `Soldiers/SoldierLabBridgeSubsystem.h/.cpp` | `ForceRescan()` |
| `Vehicles/RCWSComponent.h/.cpp` | `ResetForScenarioRestart`(탄약 리필·마운트 원위치(AddPanTiltInput 델타)·줌·카메라/발사 모드·장전) + BeginPlay 초기값 스냅샷 |
| `Vehicles/RCWSFireControlComponent.h/.cpp` | `ResetForScenarioRestart`(모드 초기값·표적/락온/기억/총성조사/배럴스핀/발사사이클·`bRespectEnemyTargetingExclusion` 초기값·ARM 초기값; `ShotsFiredCount` 는 유지) |
| `Vehicles/RCWSProjectile.h/.cpp` | `Park()` |
| `Vehicles/UGVAIController.h/.cpp` | `ResetForScenarioRestart`(경로/도착 정렬/적분기/속도 제한/드라이브 모드·시동 초기값). **(2026-09-23)** `TankTotalDistanceTraveledCm = 0.f` 추가 — UGV 상태 패널의 누적 주행거리는 이 값에서 파생된다(§5 끝) |
| `UI/StatusHUDComponent.h/.cpp`(드론·트럭 상태 패널) | **(2026-09-23 신규)** `ResetForScenarioRestart()` — `ElapsedTime`/`TimeSinceLastSample`/`TimeSinceLastUIUpdate` 0, `CurrentData = FUAVStatusData()`(배터리 100 · 비행시간 0 · 고도/속도 그래프 히스토리 비움; GPS·링크·임무 등 나머지는 다음 틱의 `GenerateDummyData` 가 다시 채움), `AltitudeGraphFilter`/`SpeedGraphFilter` 새 구조체로, 실데이터 오버라이드 플래그(`bHasRealFlightData`/`bHasRealWaypoint`) 내림 |
| `UI/UGVStatusComponent.h/.cpp`(UGV 상태 패널) | **(2026-09-23 신규)** `ResetForScenarioRestart()` — `ElapsedTime`·`RealDistanceTraveledKm` 0, `CurrentData.BatteryPercent = 100`, `CurrentData.DistanceTraveledKm = 0`. **구조체를 통째로 밀지 않는다** — 기어 라벨(`SetGearData`)처럼 매 틱 다시 안 채워지는 필드가 있어서. 온도·속도·경사는 매 틱 파생되므로 손댈 필요 없음 |
| `Vehicles/UGV0901Pawn.h/.cpp` | `IScenarioResettable` 구현: 컨트롤러 → 텔레포트 + Chaos `ResetVehicleState` → RCWS → 사격 통제. **(2026-09-23)** `ResetForScenarioRestart_Implementation` 맨 앞에서 **`if (!HasAuthority()) return;`** — 차량은 서버가 시뮬하므로 클라가 복제 액터를 로컬로 건드려 봐야 다음 복제에 덮인다. **(2026-09-23 후속)** 그 게이트 **안**에서 `FindComponentByClass<UUGVStatusComponent>()->ResetForScenarioRestart()`(§5 끝) |
| `Vehicles/TitanTruck.h/.cpp` | `IScenarioResettable` 구현: RCWS + 사격 통제. **(2026-09-23)** 같은 `HasAuthority` 게이트. **(2026-09-23 후속)** 트럭도 `UStatusHUDComponent`(드론과 같은 컴포넌트)를 들고 있어 `StatusHUD->ResetForScenarioRestart()` 를 부르는데, 트럭 자체를 서버가 시뮬하므로 지금은 게이트 **안**이다 — 클라 화면의 트럭 패널을 쓰게 되면 게이트 밖으로 옮겨야 한다(코드 주석에 적어 둠) |
| `Drone/DronePawn.h/.cpp` | `IScenarioResettable` 구현. **(2026-09-23 권한별 분리 — 이 프로젝트에서 유일하게 서버가 아닌 쪽이 물리를 돌리는 액터라서)**: 서버(`HasAuthority`)는 **복제되는 것만**(`FramingSet`, `SetDetectionPhase(InitialDetectionPhase)`, `CommandedPathId=None` + `++CommandedPathCounter`, `RepEngagementFocusLocations` + 카운터), 시뮬 주체(`bSimulationAuthority`)는 **물리·입력·짐벌·정찰 상태**(수동 해제, Autopilot `Disengage`, `Flight->ResetTo(SpawnLocation, SpawnYawDegrees)`, 짐벌 원위치, 정찰 단계, `bParachuteObserved`). 시뮬 주체가 아니면 위치는 `TickRemoteInterpolation` 이 따라오므로 안 건드린다. `DroneFollowPath` 와 같은 규약(§5). **(2026-09-23 후속)** 상태 패널 `StatusHUD->ResetForScenarioRestart()` 는 **권한 분기 어느 쪽에도 넣지 않고 시뮬 주체 판정보다 먼저** 부른다 — 모든 프로세스가 각자 되돌려야 한다(§5 끝) |
| `SoldierLab/AI/SoldierHealth.h/.cpp`(`USoldierHealthComponent`) | **(2026-09-23, 재시작 밖의 근본 수정 — [W116])** `bDestroyCarriedActorsOnDestroy`(EditAnywhere, 기본 true) + `EndPlay` 오버라이드: `EEndPlayReason::Destroyed` 일 때 자기가 어태치(`GetAttachedActors` 재귀)하거나 소유(`Children`)한 액터를 같이 파괴. 예외 = 풀링 투사체(`ASoldierProjectile`) · 클라이언트의 복제 액터(`ROLE_Authority` 인 것만). BP 수정 0건 |
| `SoldierLab/Squad/SoldierSquadSubsystem.h/.cpp` | `ResetForRestart` + 콘솔 `SoldierLab.ResetWorld` |
| `SoldierLab/AI/SoldierSituationField.h/.cpp` | `ResetForRestart`(Built* 무효화 → `EnsureLevels` 가 첫 실행과 같은 경로로 재구축, 지오메트리 캐시까지 비움 — "PIE 와 동일" 기준) |
| `SoldierLab/Weapons/SoldierProjectilePool.h/.cpp`, `SoldierProjectile.h/.cpp` | `RecallAll`, `Park` |
| `Content/Scenario/DT_ScenarioSteps_ThreeStage_SoldierLab`(New_kadex_0811용) · `DT_ScenarioSteps_SquadThreeStage`(L_SoldierScenario용) | 둘 다 행 `ScenarioRestartPrompt`(Prereq `ScenarioComplete`, TimerOnly 5, 이펙트 **`ShowRestartPrompt`**, bEnabled true) 추가·저장 |
| `Content/widget/notify/WBP_RestartPrompt`(사용자 제작) | 부모 `URestartPromptWidget`, 규격 §4 |

Perforce: 워크스페이스 `user4_DESKTOP-81S78B2_4340`, 소스 ~35파일 edit + 5파일 add(위 신규 표) + DT 2개 edit. 일부 파일은 user2 도 동시 체크아웃 중 — 제출 전 조율.

---

## 3. 남은 것 (순서대로)

**끝난 것(2026-09-22)**: 정식 빌드 통과 → DT 2개에 `ScenarioRestartPrompt`(이펙트 `ShowRestartPrompt`) 저장 → `WBP_RestartPrompt` 제작 → **`L_SoldierScenario` PIE 검증 완료**(1차 테스트 결함 2건(§5) 수정 후 사용자 판정 "완벽하다").

**끝난 것(2026-09-23)**: **New_kadex_0811 2-PC 검증 완료**(UGV 호스트 + 자체방호 클라이언트 — 사용자 "이제 잘됨"). 그 과정에서 레벨 GameMode 오버라이드 복구(`BP_KadexTestGameMode`) · 멀티캐스트 3종 · 드론 권한 분리 · 소총 누수 근본 수정([W116]) · 스냅샷 임시 컴포넌트 제외 — 전부 §5.

**현재 상태(2026-09-23 기준)**: 사용자 판정 "일단 지금 다 잘 작동하는 상태임". 아래는 **검증·운영 항목만** 남았다.

1. **빌드 대기 2건 — 드론 짐벌 배율 + 상태 패널 누적값** — 코드 수정은 둘 다 끝났지만(§5 짐벌 배율 문단 · §5 끝 상태 패널 문단) **아직 빌드하지 않았다**. 빌드 후 확인할 것: ① 2회차 첫 몇 초의 짐벌 그림이 1회차와 같은가 ② 2회차 시작 시 **드론 배터리 100 % · 비행시간 00:00 · 고도/속도 그래프 빈 상태**, **UGV 배터리 100 % · 주행거리 0** 인가(드론은 호스트·클라 **양쪽 화면**에서, UGV 는 서버 값이 복제되므로 아무 화면에서나). **"검증 완료"가 아니다.**
2. 10사이클 자동(체크 ON) 뒤 `stat memory`/fps 추이(1회차 대비).
3. 장시간 무인 반복(자동 재시작 ON + `-autorestart`) — 사이클 로그는 `ScenarioMonitorSubsystem` 의 `cycles.csv`(2.8절 / `2026-09-22_scenario_flow_log_and_failsafe.md`).
4. **P4 제출 순서 조율** — 이 세션 워크스페이스(`user4_DESKTOP-81S78B2_4340`)에 소스 다수 + DT 2개가 열려 있고, `Soldiers/SoldierLabBridgeSubsystem.cpp` 처럼 **다른 세션이 동시에 편집 중인 파일**이 있다. 어느 쪽이 먼저 제출할지 맞추고 나서 낼 것.
5. [W116] `obj list class=BP_AR4Rifle_C` **실측 카운트는 아직 없다**(2-PC 실기에서 증상 소멸은 확인됨 — `soldier_ai_lab/OPEN_ITEMS.md` [W116]).

~~`New_kadex_0811` 의 `ScenarioConfig_1.RespawnActors = [BP_Parachute_C_3]` 연결~~ → **불필요로 종결(2026-09-23, 사용자 확정)**. 낙하산은 정적 액터라 재시작 때 초기화할 것이 없다 — `RespawnActors` 는 **빈 채로 둔다**. 프로퍼티 자체는 남긴다(병사가 아닌 재스폰 대상이 나중에 생기면 쓰는 자리).

~~평상시 사망 뒤 소총 잔존 확인~~ → 2026-09-23 원인 확정 + `USoldierHealthComponent` 수정으로 해결([W116], §5).

---

## 4. `WBP_RestartPrompt` 규격 (사용자 제작)

- 경로/이름: **`/Game/widget/notify/WBP_RestartPrompt`**, 부모 클래스 **`RestartPromptWidget`**(C++ `URestartPromptWidget`). 경로가 다르면 `UNotificationSubsystem` 의 `RestartPromptWidgetClass` 를 손으로 지정.
- 루트: 화면 전체를 덮는 **CanvasPanel**(WBP_ConfirmDialog 와 같은 구조 — 반투명 배경 + 중앙 박스). 매니저가 붙일 때 앵커를 0~1 로 늘린다.
- 위젯 이름(대소문자 정확히):

| 이름 | 타입 | 필수 | 역할 |
|---|---|---|---|
| `YesButton` | Button | ✓ | 즉시 재시작 |
| `NoButton` | Button | ✓ | 닫기(카운트다운 취소, 체크값은 유지) |
| `AutoRestartCheckBox` | CheckBox | ✓ | 값 즉시 저장(ini), 켜면 카운트다운 시작/끄면 취소 |
| `YesLabel` | TextBlock | 선택 | [예] 버튼 안의 글자. 카운트다운 중 `예 (7)` 로 바뀜(원문은 열릴 때 기억) |
| `MessageText` | TextBlock | 선택 | "시나리오를 재시작하시겠습니까?" — 코드가 안 건드리므로 WBP 에 직접 적음 |

- 그래프 배선은 **없음** — 클릭/체크 이벤트는 C++ 이 이름으로 잡아 바인딩한다. 체크박스 초기값도 코드가 `Open()` 에서 맞춘다.
- 커서: 열릴 때 `bShowMouseCursor` 를 켜고 닫을 때 원래대로 되돌린다. 입력 모드(UI Only 등)는 건드리지 않는다 — 필요하면 나중에.
- 화면 크기: 루트 CanvasPanel 은 매니저가 붙일 때 호스트(Monitor1 패널 또는 뷰포트)에 앵커 (0,0)~(1,1) 로 늘리므로 해상도를 몰라도 된다. **안의 박스는 앵커 (0.5,0.5) + Alignment (0.5,0.5) + Size To Content(또는 SizeBox 고정 크기)** 로 중앙에 두면 어떤 해상도/듀얼 모니터 폭에서도 가운데다. 절대 좌표(Position X/Y 고정)만 피하면 된다. 루트를 Overlay 로 만들어도 동작한다(슬롯 캐스트가 실패하면 그냥 건너뛰고, 뷰포트 추가는 원래 전체 화면).
- **어느 화면에 뜨는가**(2026-09-22 사용자 확정, `Atitan_exampleGameState::Multicast_ShowScenarioRestartPrompt`):
  UGV 호스트 + 자체방호 클라이언트 → **자체방호(클라이언트)** / 자체방호 단독 → 자체방호 / UGV 단독 → UGV / 데모·풀 시스템 무관.
  서버가 "자체방호 플레이어 접속 여부"를 판정해 멀티캐스트 인자로 넘기고, 각 프로세스가 자기 축을 보고 띄운다.
  [예]·카운트다운·체크값은 클라이언트에서 `Atitan_examplePlayerController::Server_RequestScenarioRestart / Server_SetScenarioAutoRestart` 로 서버에 올라가고, 체크값 정본은 서버(ini + GameState `bScenarioAutoRestart` 리플리케이트 → 클라 확인창 초기값). 재시작이 시작되면 `Multicast_HideScenarioRestartPrompt` 로 모든 화면의 확인창을 닫는다.

---

## 5. 설계와 달라진 점 / 주의

**2-PC 리플리케이션 테스트(2026-09-23) — 최종 원인은 레벨 GameMode 오버라이드였다 ★**

증상(UGV축 호스트 + 자체방호축 클라이언트): 호스트는 정상인데 **클라이언트만** ① 카메라 페이드가 전혀 안 되고 ② 재시작 후 드론이 원위치로 안 돌아왔다(+ 1차 테스트와 같은 소총 에러 스팸).

**최종 원인**: `New_kadex_0811` 의 **World Settings GameMode 오버라이드가 `GM_SoldierLab` 로 바뀌어 있었다**(2026-09-21 SoldierLab 이관 때 들어옴). `GM_SoldierLab` 의 부모는 순정 `AGameModeBase` 이고 `GameStateClass = AGameStateBase`, `PlayerControllerClass = APlayerController` 다(실측). 그래서:

| 없어진 클래스 | 결과 |
|---|---|
| `Atitan_exampleGameState` | 재시작의 **모든 멀티캐스트가 서버-로컬 폴백으로** 빠진다 → 클라이언트는 페이드·정리·리셋을 아무것도 못 받는다. 단일 프로세스 Standalone 에서는 그 폴백이 곧 정답이라 `L_SoldierScenario` 테스트는 멀쩡했다 — **2-PC 에서만 깨진다** |
| `Atitan_examplePlayerController` | `PlayerAxis` 가 안 정해지고 데모 플래그(GameState 리플리케이트)도 클라에 안 간다 → `ADronePawn::ResolveShouldSimulateDrone` 에서 서버(데모→리슨서버)와 클라(Unspecified→true)가 **둘 다 시뮬 주체**가 되어 각자 물리를 돌린다. 재시작 전에 이미 동기화가 아니었던 것 |

**해결**: 레벨 GameMode 오버라이드를 원래대로 **`BP_KadexTestGameMode`**(부모 `Atitan_exampleGameMode`)로 되돌림 → 사용자 확인 **"이제 잘됨"**. 근거: P4 리비전 `Content/New_kadex_0811.umap#43`(2026-09-18, 이관 직전)을 직접 열어 보니 GameMode 오버라이드가 `BP_KadexTestGameMode` 하나뿐이었고 `GM_SoldierLab` 은 #44(09-21) 이후 들어왔다.

| 레벨 | GameMode 오버라이드 | 부모 |
|---|---|---|
| `New_kadex_0811` | **`BP_KadexTestGameMode`** | `Atitan_exampleGameMode` |
| `kadex_test` | `BP_KadexTestGameMode` | 〃 |
| `kadex_lobby` | `BP_TestGameMode` | `AGameModeBase`(대기실이라 무관) |

**운용 규칙**: 병사 거동을 자유 관전으로 볼 때만 `GM_SoldierLab` 로 바꿔 **단일 프로세스**로 돌리고, **2-PC·전시 구성에서는 반드시 `BP_KadexTestGameMode`**. (이관 문서 `2026-09-21_soldierlab_migration_new_kadex_0811.md` §6 의 "`GM_SoldierLab` 에서도 시나리오는 돈다"는 **단일 프로세스 한정**이다 — 잃는 것 목록에 "클라이언트 쪽 재시작 전파"가 빠져 있었다.)

**진단 경고(코드 추가)**: 재시작 시 월드의 GameState 가 `Atitan_exampleGameState` 가 아니면
`[ScenarioStateSubsystem] 재시작: 이 월드의 GameState 가 Atitan_exampleGameState 가 아님(게임 모드=…) — 클라이언트 쪽 페이드/정리/리셋이 전달되지 않는다`
경고를 남긴다(Standalone 은 제외 — 거기선 폴백이 정상 경로).

---

**같은 테스트에서 잡은 것 (1) — "각 프로세스가 자기 것을 치운다"**

재시작 처리가 전부 서버에서만 돌아 클라이언트에만 따로 존재하는 것이 남았다. 그런 것이 셋이다:

| 대상 | 왜 서버 처리로는 안 되나 |
|---|---|
| 카메라 페이드 | 로컬 `PlayerCameraManager` 의 일 — 서버에서 걸면 서버 화면만 어두워진다 |
| 병사 손의 소총 `BP_AR4Rifle` | **`bReplicates=false`**(실측) — 프로세스마다 BeginPlay 에서 자기 것을 스폰한다. 서버가 자기 것을 지워도 클라 것은 남고, 클라의 병사 복제본이 (복제된 파괴로) 사라지면 고아가 되어 매 틱 죽은 주인을 건드린다. 엔진은 파괴 시 **자기 Owner 만** 비우고(`LevelActor.cpp` `ThisActor->SetOwner(NULL)`) 자기가 소유한 액터의 Owner 는 그대로 둔다(어태치만 끊음) |
| 코스메틱 투사체 풀 | SoldierLab/RCWS 풀이 프로세스마다 따로 |

**근본 수정(2026-09-23, [W116] 해결)** — 소총 누수는 재시작만의 문제가 아니라 **평상시 사망에도 있던 것**이었다(확정: `BP_SoldierCharacter` 가 BeginPlay 에서 소총을 `SpawnActor` 하고 `Rifle` 에 들고 있지만 그 BP 에 `K2_DestroyActor`·`EndPlay` 가 **0개**, `USoldierHealthComponent` 는 `DestroyAfterSeconds` 뒤 `Owner->Destroy()` 만, 엔진은 소유 액터를 안 따라 지움). 그래서 `USoldierHealthComponent::EndPlay` 에 `bDestroyCarriedActorsOnDestroy`(기본 켬)를 넣어 **병사가 파괴될 때 자기가 들고 있는 것을 같이 파괴**한다 — BP 수정 없이 모든 파괴 경로·모든 프로세스를 덮고, 클라이언트는 자기가 스폰한 것만(`ROLE_Authority`), 풀링 투사체는 제외. 엔진 순서상 안전(`DestroyActor` → `Destroyed()` → `RouteEndPlay` → 컴포넌트 `EndPlay` 가 어태치 해제·`SetOwner(NULL)` 보다 먼저). 아래 멀티캐스트 정리는 그대로 두되 이제 **안전망**이다(낙하산처럼 병사가 아닌 재스폰 대상도 덮으므로).

수정: `Atitan_exampleGameState` 에 **멀티캐스트 3종** 신설 → 각 프로세스가 `UScenarioStateSubsystem` 의 로컬 함수를 실행한다.

| 멀티캐스트 | 로컬 함수 | 언제 / 무엇 |
|---|---|---|
| `Multicast_ScenarioRestartBegin(FadeOutSeconds)` | `RunLocalRestartBegin` | **병사 파괴보다 먼저** 보낸다(클라가 자기 병사가 살아있는 동안 소총을 치우도록). 카메라 페이드 아웃(로컬 `PlayerCameraManager` 의 일), 확인창/토스트 닫기, 로컬 코스메틱 투사체 주차(SoldierLab 풀 `RecallAll` + `ARCWSProjectile::Park`), 병사가 로컬로 스폰한 딸린 액터 파괴(`UScenarioRespawnSubsystem::DestroySoldierAttachments`) |
| `Multicast_ScenarioRestartApply()` | `RunLocalRestartApply` | **화면이 검은 순간**에 보낸다. 이 프로세스가 시뮬하는 `IScenarioResettable` 액터 리셋 — 드론처럼 시뮬 주체가 서버가 아닌 액터가 있어서 필요하다(아래) |
| `Multicast_ScenarioRestartEnd(FadeInSeconds)` | `RunLocalRestartEnd` | 고아 액터 청소(`DestroyOrphanedChildActors` — 판정은 "Owner 포인터는 있는데 유효하지 않음", 패킷 순서로 Begin 이 늦게 처리돼 놓친 것) + 페이드 인 |

- 클라이언트는 **자기가 스폰한 것만**(`GetLocalRole()==ROLE_Authority`) 파괴/주차한다 — 복제된 액터를 로컬로 지우면 서버와 갈린다. 서버에선 전부 Authority 라 동작이 같다. 풀링 투사체는 항상 제외(풀이 포인터를 쥔다).
- **titan GameState 가 없는 월드**(= `GM_SoldierLab` 등)에서는 세 함수가 각각 **로컬 폴백**으로 실행된다 — 단일 프로세스면 그게 정답이고, 2-PC 면 위 경고가 뜬다.
- `ExecuteScenarioReset` 의 투사체/토스트 정리는 Begin 으로 옮겨 중복 제거.

**같은 테스트에서 잡은 것 (2) — 드론 리셋을 권한별로 분리 [설계상 중요]**

드론은 이 프로젝트에서 **유일하게 서버가 아닌 쪽이 물리를 돌리는 액터**다: 풀 시스템에서는 자체방호 클라이언트가 시뮬 주체(`ResolveShouldSimulateDrone` ③)이고, 서버는 물리를 안 돌린 채 `Server_ReportState` 로 받은 위치로 자기 복제본을 `SetActorLocationAndRotation` 할 뿐이다. 그래서 **서버에서만 `Flight->ResetTo` 를 불러 봤자 다음 보고에 즉시 덮이고 클라 드론은 날던 자리에 남는다.**

수정: `ADronePawn::ResetForScenarioRestart_Implementation` 을 `DroneFollowPath` 와 **같은 규약**으로 나눴다.

| 누가 | 무엇을 되돌리나 |
|---|---|
| 서버(`HasAuthority`) | **복제되는 것만** — `FramingSet`, `SetDetectionPhase(InitialDetectionPhase)`, `CommandedPathId=None` + `++CommandedPathCounter`, `RepEngagementFocusLocations` + 카운터 |
| 시뮬 주체(`bSimulationAuthority`) | **물리·입력·짐벌·정찰 상태** — 수동 해제, Autopilot `Disengage`, `Flight->ResetTo(SpawnLocation, SpawnYawDegrees)`, 짐벌 각도 원위치, **짐벌 배율 `SetZoomLevel(InitialZoomLevel)`**, 정찰 단계, `bParachuteObserved` |

> **짐벌 배율(2026-09-23 추가, 사용자 지적)** — `ZoomLevel` 은 `EditAnywhere` 저작값(기본 1.0)인데 자동 정찰/교전 프레이밍이 런타임에 계속 바꾼다(`GimbalSearchZoomLevel` ↔ `GimbalZoomInLevel` 램프, 광각 `RequiredZoom`, 수동 전환). 처음엔 전환 플래그(`bManualZoomTransitionActive`)만 끄고 값은 안 되돌려서, 2회차가 지난 사이클의 배율에서 시작했다(정찰 램프가 천천히 수렴하긴 하지만 첫 몇 초 그림이 1회차와 다름). `InitialZoomLevel` 을 BeginPlay 에 스냅샷(`InitialDetectionPhase` 와 같은 방식)했다가 `SetZoomLevel(InitialZoomLevel)` 로 되돌린다(그 함수가 `SyncGimbalLensFromCineCamera` 까지 해서 FOV 즉시 반영). `ManualZoomStartLevel`/`ManualZoomTargetLevel` 도 같이 초기 배율로. 서버의 `RepZoomLevel` 은 시뮬 주체의 다음 `Server_ReportState` 로 따라온다.
> **⚠ 이 수정은 아직 빌드에 안 들어갔다 — 다음 빌드 반영 예정이고, 실기 확인은 그 뒤다(§3-1).**
>
> **`ViewMode` 는 리셋하지 않는다(2026-09-23 결정, 사용자 확인)** — `EDroneViewMode`(Chase/Onboard/Gimbal)는 사람이 드론을 **직접 조종할 때** 입력 액션 `OnCameraTogglePressed` 로 바꾸는 **로컬 카메라 모드**다(`L_DroneTest` 류 워크플로). 전시·2-PC 구성에서는 아무도 드론을 빙의하지 않아 값이 바뀌지 않으므로 재시작 리셋 대상이 아니다.
>
> 같이 점검한 결과 드론·UGV 컨트롤러의 나머지 런타임 상태 중 빠진 것은 없다 — 미리셋으로 남은 것은 진단 누적값(`Diag*`/`*LogAccum`/`StateReports*`), 복제 미러(`Rep*` — 시뮬 주체 값이 오면 덮임), BeginPlay 1회 세팅(`bAxisResolved`/`bMappingContextApplied`/도로 캐시 — **되돌리면 오히려 깨진다**), `bSnapInProgress=false` 면 무의미해지는 값(`SnapStart*`)뿐이다.

- 시뮬 주체가 **아니면** 위치는 `TickRemoteInterpolation` 이 따라오므로 건드리지 않는다.
- **`bParachuteObserved` 는 복제되지 않고 시뮬 주체에서만 갱신된다**(드론 Tick 의 권한 게이트 때문) — 그래서 그 초기화도 시뮬 주체 몫이다.
- 단일 프로세스(단독 실행 · 데모 리슨서버)는 둘 다 자신이라 예전과 동일하게 돈다.
- 반대로 차량(`AUGV0901Pawn`, `ATitanTruck`)은 서버가 시뮬하므로 `ResetForScenarioRestart_Implementation` 맨 앞에서 `if (!HasAuthority()) return;` 한다.

**같은 테스트에서 잡은 것 (3) — 재스폰 스냅샷에서 에디터 시각화용 임시 컴포넌트 제외**

`CameraProxyMeshComponent_N` / `DrawFrustumComponent_N` / `OutputCameraComponent` — PIE 에서 카메라 컴포넌트가 스스로 만들고 스폰마다 번호가 달라 "재스폰본에 없음" 경고가 났고, `RF_TextExportTransient` 라 ExportText 가 빈 문자열이어서 적용도 실패했다. 판별: 아키타입이 자기 클래스 CDO(`RF_ClassDefaultObject`) 이거나 `RF_Transient｜RF_TextExportTransient`, `IsEditorOnly()`.

**L_SoldierScenario 1차 테스트(2026-09-22)에서 잡은 것 2건** — 페이드/UGV 재주행은 됐지만:
- **재스폰 병사가 안 움직임** → `BP_Soldier_*` 가 `AutoPossessAI=PlacedInWorld` 라 스폰된 폰엔 AI 컨트롤러가 안 붙었다(`APawn::PostInitializeComponents` 는 월드 시작 중에만 빙의). 재스폰을 `bDeferConstruction` 으로 미루고 `AutoPossessAI=PlacedInWorldOrSpawned` 로 바꾼 뒤 `FinishSpawning` → 레벨 배치본과 같은 순서(BeginPlay 전)로 빙의. 안전망으로 컨트롤러 없으면 `SpawnDefaultController`.
- **GT 지연이 계속 증가 + `BP_AR4Rifle ... OwningCharacter is not valid` 스팸** → 병사만 Destroy 하니 병사가 스폰해 손에 붙인 소총 액터가 남아 매 틱 죽은 주인을 참조했고, 사이클마다 소총이 두 배로 늘었다. `DestroyChildActors`: 병사에 붙어 있거나(GetAttachedActors 재귀) 병사가 Owner 인 액터를 같이 파괴(풀링된 투사체는 제외).

- 재스폰 이름: 구 액터를 Rename 하지 않는다(리슨서버에서 bNetStartup 액터 이름 변경이 클라 파괴 동기화를 깨뜨릴 수 있어서). `NameMode=Requested` 로 원래 이름을 청하되, GC 전이면 접미사가 붙을 수 있다 — 로그 대조 시 이름 접미사는 무시.
- 상황 필드 지오메트리 캐시(`Horizons`)도 비운다 — 설계 v3 는 "유지"였지만 "PIE 와 동일" 기준에 맞춰 첫 실행과 같은 경로(`EnsureLevels` 재구축)로 통일. 비용은 첫 실행과 같은 점진 베이크.
- 델타는 FinishSpawning(=BeginPlay) 뒤에 들어간다(SCS 컴포넌트가 그 안에서 생김). 현재 저작값(SquadId/bSquadLeader/bInvincible)은 전부 명령·피격 시점에 읽혀 문제 없음. `Health = MaxHealth` 는 BeginPlay 가 읽지만 MaxHealth 는 클래스 기본값이라 델타에 안 잡힌다.
- 확인창/재시작은 `RunMode` 를 가리지 않는다(요구에 구분 없음). 풀 시스템에서 끄려면 `ScenarioConfig.bRestartPromptEnabled=false`.
- `UGVArriveZone1` 은 `ActorStopped` 트리거라 리셋 뒤 `IsMoving()==false` 여야 한다 — `AUGVAIController::ResetForScenarioRestart` 가 보장.

---

**상태 패널 누적값(배터리·비행시간·주행거리)도 리셋 대상이다 (2026-09-23 후속, 사용자 지적) ★ 호출 위치가 두 컴포넌트에서 정반대**

**발견** — 재시작해도 **드론 배터리와 비행시간이 초기화되지 않았다**(사용자 지적). 확인해 보니 **UGV 에도 같은 문제**가 있었다(배터리 + 누적 주행거리). 배터리가 0 이 돼도 비행이 멈추는 로직은 없으므로 기능적 장애는 아니지만, **"재시작 = 레벨 시작과 같은 상태"** 기준에 어긋난다.

**원인** — 두 상태 컴포넌트가 월드 시작부터 계속 누적하는 값을 갖고 있고, 재시작 리셋 계약에 둘 다 빠져 있었다.

| 대상 | 누적 구조 |
|---|---|
| 드론·트럭 `UStatusHUDComponent`(`UI/StatusHUDComponent.cpp`) | `ElapsedTime` 이 영원히 누적 → `BatteryPercent = 100 − ElapsedTime × 0.05`, 그리고 `CurrentData.FlightTimeSeconds += DeltaTime`. 고도/속도 그래프 히스토리·평활 필터·샘플 타이머도 계속 이어짐 |
| UGV `UUGVStatusComponent`(`UI/UGVStatusComponent.cpp`) | `ElapsedTime` → `BatteryPercent = 100 − ElapsedTime × 0.03`, 온도도 여기서 파생 |
| UGV 누적 주행거리 | 원본은 `AUGVAIController::TankTotalDistanceTraveledCm`, `UUGVStatusComponent::CurrentData.DistanceTraveledKm` 이 매 틱 거기서 파생 |

**수정** — 컴포넌트마다 `ResetForScenarioRestart()` 신설(내용은 §2 표) + `AUGVAIController::ResetForScenarioRestart` 에 `TankTotalDistanceTraveledCm = 0.f` + 호출부 셋(`ADronePawn` · `AUGV0901Pawn` · `ATitanTruck`).

**★ 호출 위치가 두 컴포넌트에서 정반대다 — 리플리케이션이 다르기 때문**

| 패널 | 어디서 부르나 | 왜 |
|---|---|---|
| 드론(`UStatusHUDComponent`) | **모든 프로세스** — `ADronePawn::ResetForScenarioRestart_Implementation` 의 **시뮬 주체 판정보다 먼저**(`HasAuthority` 분기 어느 쪽에도 안 넣음) | `UStatusHUDComponent::CurrentData` 는 **리플리케이트되지 않고** 틱에 권한 게이트도 없다 → 프로세스마다 자기 값을 따로 누적한다. 서버에서만 되돌리면 클라 화면의 드론 패널은 그대로 이어진다 |
| UGV(`UUGVStatusComponent`) | **서버에서만** — `AUGV0901Pawn` 의 `HasAuthority` 게이트 **안** | `UUGVStatusComponent::TickComponent` 가 `bUseDummyData && GetOwner()->HasAuthority()` 일 때만 값을 만들고, `CurrentData` 가 `ReplicatedUsing = OnRep_CurrentData` 로 복제된다 → 서버 한 곳만 되돌리면 클라 대시보드 숫자도 따라온다 |
| 트럭(`UStatusHUDComponent`) | 지금은 **서버에서만**(`ATitanTruck` 의 `HasAuthority` 게이트 안) | 드론과 **같은 컴포넌트**지만 트럭 자체를 서버가 시뮬한다. 클라 화면의 트럭 패널을 쓰게 되면 **게이트 밖으로 옮겨야 한다** — 코드 주석에 그렇게 적어 뒀다 |

즉 §5 의 "복제되는 것은 서버가, 비복제 로컬 상태는 각 프로세스가 되돌린다"는 규칙이 **같은 재시작 안에서 정반대 배치로 나타난 사례**다. 판단 기준은 "이 액터를 누가 시뮬하는가"가 아니라 **"이 값이 복제되는가 / 어느 프로세스가 이 값을 만드는가"** 다(리플리케이션 관점 정리는 `../replication/replication_audit.md` §9).

**같은 계열 전수 검색 — 추가 누락 없음.** 시간·거리 누적으로 표시되는 값을 전부 훑었고 나머지는 문제없다:

- `UDroneAutopilotComponent::SpoolElapsedSeconds` — 다음 `BeginPathFollowing` 이 0 으로 만든다(자가 복구).
- `AUGVPawn::TotalDistanceTraveledCm` · `AUAVPawn::MissionElapsedSeconds` — 이 레벨에서 **안 쓰는 구 변형**(현재는 `BP_UGV_0901` / `ADronePawn`).
- `AUGVAIController` 의 `AlignElapsedSeconds`/`SnapElapsedSeconds`, 드론의 `ObservationHoldSeconds`/`ManualZoomElapsedSeconds` — 이미 각자 리셋에 들어 있다.

**RCWS 탄약은 이미 초기화되고 있다(사용자 질문 확인 결과).** "트럭·UGV RCWS 총알도 비슷한 것 아니냐"에 대해 코드로 확인함 — `URCWSComponent::ResetForScenarioRestart()` 첫 두 줄이 `CurrentData.AmmoMax = AmmoMax; CurrentData.AmmoCurrent = AmmoMax;` 이고 호출부가 `Vehicles/UGV0901Pawn.cpp`(`CachedRCWS->ResetForScenarioRestart()`)와 `Vehicles/TitanTruck.cpp`(`RCWS->ResetForScenarioRestart()`) 양쪽에 있다. `CurrentData` 가 복제되므로 클라 대시보드 숫자도 따라온다.

⚠ **이 수정은 아직 빌드에 안 들어갔다 — 다음 빌드 반영 예정이고, 2회차 확인은 그 뒤다(§3-1).**
