# 로딩 화면 + 워밍업 + 호스트 시나리오 게이트 구현

2026-10-06 / 구현 완료(검증 대기) / 설계(`2026-10-06_loading_screen_design.md`) A~D 단계와 §4 호스트 시나리오 게이트를 `UTitanLoadingScreenSubsystem` + `UScenarioStateSubsystem` 게이트로 구현; 빌드 완료·단독 PIE 동작 확인(워밍업 원점 스폰 버그 수정 포함, §8), 콘솔 거부·재시작·2-PC·-game 검증 대기.

> 정본 설계: [`2026-10-06_loading_screen_design.md`](2026-10-06_loading_screen_design.md). 이 문서는 "실제로 무엇을 어떻게 넣었고 설계와 어디가 다른가" + 설계 §6 미결 2건의 조사 결과 + 검증 절차.
> 빌드는 사용자가 한다(이 세션은 빌드 안 함). 아래 엔진 file:line은 UE5.8 엔진 소스 기준.

---

## 1. 파일

| 파일 | 내용 |
|---|---|
| `Source/titan_example/UI/TitanLoadingScreenSubsystem.h/.cpp` (신규) | `UGameInstanceSubsystem`. A~D 단계 전부, cvar, Slate 오버레이, 워밍업. 코어 티커(`FTSTicker`)로 자체 틱 |
| `Source/titan_example/UI/TitanLoadingScreenSettings.h` (신규) | `UDeveloperSettings`(config=Game, Project Settings > Game > Titan Loading Screen): `WarmupNiagaraSystems`, `WarmupDecalMaterials`, `LoadingLogo`(선택), `LoadingText`(기본 "Loading", 점 애니메이션으로 "Loading..."), `LoadingFont`(`TSoftObjectPtr<UFont>`), `LoadingFontSize`(int32, 기본 28) — 폰트 2개는 빌드 후 추가(§8-2) |
| `Source/titan_example/UI/ScenarioStateSubsystem.h/.cpp` | `HostReadyWorld`/`DemoSetupPendingWorld`(둘 다 `TWeakObjectPtr<UWorld>`), `IsHostWorldReady()`, `PassHostReadyGate(EntryName)`, `NotifyHostWorldReady(World)`; 진입 함수마다 게이트 1줄; `ApplyDemoRunModeSetup` 연기 |
| `Source/titan_example/titan_examplePlayerController.cpp` | 익명 네임스페이스 헬퍼 `ExecuteLobbyTravel(PC, Command)`(`:2404`) — `HostListenServer`/`StartSoloAxis`/`ConnectToHost` 세 곳(`:2454, 2484, 2498`)이 `ConsoleCommand(open…)` 대신 이걸 부른다. 서브시스템이 없으면 예전처럼 즉시 `ConsoleCommand`. **PC 헤더는 안 바꿈** |
| `Source/titan_example/titan_example.Build.cs` | `PrivateDependencyModuleNames`에 `"MoviePlayer"`, `"EngineSettings"`(`UGameMapsSettings::GetGameDefaultMap` — 로비 판정) 추가 |
| `Config/DefaultGame.ini` | `[/Script/titan_example.TitanLoadingScreenSettings]` — Niagara 23개 + 탄흔 데칼 1개 + `LoadingText=Loading` + `LoadingFont=/Game/Font/Pretendard-Medium_Font.Pretendard-Medium_Font` + `LoadingFontSize=28` |

## 2. 단계별 구현 (설계 §3 대응)

### A. 로비 페이드아웃
- `BeginLobbyTravel` → 로컬 카메라 `StartCameraFade(0→1, LobbyFadeOutSeconds, 검정, bFadeAudio=true, hold)` → 실시간 경과 후 `ExecutePendingTravel`이 요청 PC의 `ConsoleCommand(open…)`.
- `ExecutePendingTravel`이 open 직전에 뷰포트 오버레이를 먼저 띄움 → `ConnectToHost` 접속 대기 동안에도 "Loading..."이 보임.
- `OnNetworkFailure`/`OnTravelFailure` 구독. 로비 페이드/트래블/클라 로딩 중 실패면 `AbortLoading` → 오버레이 제거, 로비 월드가 그대로면 0.3 s 페이드로 로비 화면 복구. **호스트 워밍업 중 네트워크 실패는 중단하지 않음**(§4 참고).

### B. LoadMap 동안 MoviePlayer
- `FCoreUObjectDelegates::PreLoadMapWithContext`에서 `GetMoviePlayer()->SetupLoadingScreen` (`bAutoCompleteWhenLoadingCompletes=true`, `bWaitForManualStop=false`, 위젯 = 같은 Slate 로딩 위젯).
- 조건: `titan.Loading.UseMoviePlayer != 0 && IsMoviePlayerEnabled()`. 에디터/PIE는 엔진이 끔(`MoviePlayer.cpp:123-132`) → 로그 "MoviePlayer 비활성(에디터/PIE 또는 cvar), 뷰포트 오버레이만".
- 워밍업 단계까지 끌지 않음(설계대로). LoadMap 끝나면 C로.

### C. 워밍업 오버레이
- 훅: `OnPostWorldInitialization` → `World->OnWorldBeginPlay` → `HandleWorldBeginPlay`. 대상 = 자기 GameInstance의 게임 월드 중 `GameDefaultMap`(로비)이 아닌 것 전부. 판정 결과를 항상 로그(`대상 월드 판정: …`).
- `BeginWarmup`: 뷰포트 오버레이 표시 + 로컬 카메라 검정 고정(`SetManualCameraFade(1, 검정, bFadeAudio=true)`, 매 틱 재확인) + `PrecachePSOsBoostToHighestPriority(true)` + 워밍업 에셋 동기 로드(에디터면 `UNiagaraSystem::WaitForCompilationComplete`).
- 로컬 PC+카메라가 생기고 **카메라 위치가 원점 근처가 아니게 된 뒤** 2프레임 뒤 카메라 앞 `WarmupDistanceCm`에 Niagara 전부 스폰, 데칼은 카메라 앞 지면 트레이스 지점에 1회(지면 못 찾으면 생략). `WarmupFrames` 프레임 뒤 제거. 카메라가 계속 원점이면 `NoCameraSkipSeconds`(10 s) 뒤 그 위치로 폴백 스폰, 로컬 카메라 자체가 없으면 이펙트 워밍업 생략(경고 로그). 스폰 직전 진단 로그 `[LoadingScreen] 워밍업 카메라 — PC=…, 뷰타겟=…, 카메라=…`.
- **"로컬 PC" 판정 = `TitanLoading::IsViewingPlayerController(PC)` = `PC->GetLocalPlayer() != nullptr`** — `IsLocalController()`가 아님(§8-3). 카메라 PC 찾기·카메라 검정 고정·카메라 페이드·Monitor1 창 찾기 4곳 모두 이 판정.
- **준비 조건**(모두 동시 충족이 `StableFrames` 연속 + `MinHoldSeconds` 경과): 워밍업 완료 · **로컬 PC(카메라 있음) 존재** · `!IsAsyncLoading()` · `IStreamingManager::GetNumWantingResources() <= MaxStreamingWanting` · `PipelineStateCache::NumActivePrecacheRequests() <= MaxPSOPrecache` · `FApp::GetDeltaTime() <= MaxFrameMs` · (`ForceUnmet=1`이면 강제 미충족).
- 하드 타임아웃 `TimeoutSeconds`(단 `MinHoldSeconds+1`보다 작으면 그 값으로 올림). 5 s마다 진행 로그(미충족 목록), 종료 시 `사유=정상/타임아웃` + 마지막 미충족 목록.

### D. 페이드인 → 준비 신호
- `FinishWarmup`: 워밍업 이펙트·PSO 부스트 해제 → 카메라 페이드 1→0(`FadeInSeconds`) **시작과 동시에** NM_Client가 아니면 `UScenarioStateSubsystem::NotifyHostWorldReady(World)`. 클라는 "클라이언트 — 자기 화면만 정리(서버로 보내는 신호 없음)" 로그만.
- 페이드인 동안 오버레이(텍스트 포함) 알파도 같이 1→0, 끝나면 제거 → "페이드인 완료".
- `titan.Loading.Enabled 0`이면 대상 월드 BeginPlay에서 오버레이·워밍업 없이 곧바로 준비 신호(게이트 자체는 그대로 동작).

## 3. 호스트 시나리오 게이트 (설계 §4 대응)

- 준비 상태 = `HostReadyWorld == GetWorld()`(월드 키). `ResetScenarioRuntimeState`에는 **안 넣음**(재시작 공유 함수) → 재시작은 같은 월드라 준비 상태 유지, 재시작 흐름 불변.
- `PassHostReadyGate(EntryName)`: 미준비면 경고 로그 `"[ScenarioStateSubsystem] <EntryName> 거부 — 호스트 로딩 중(완료 후 다시 입력). 대기열에 넣지 않음."` 후 false. 대기열 없음.
- **게이트를 건 함수**(전부 `UScenarioStateSubsystem`):

| 함수 | 비고 |
|---|---|
| `BeginEnemyContactScenario` | 명령어 `BeginScenarioEnemyContact`, 데모 자동 시작 |
| `StartUGVAdvance` | |
| `BeginScenarioSteps` | 최종 방어선, **호스트(NM_Client 아님)일 때만** 검사 |
| `RequestScenarioRestart` | 확인창/카운트다운/`titan.ScenarioRestart`/fail-safe |
| `ApplyDemoRCWSAutoFire` | 로그 이름 "데모 지휘소 RCWS 자동사격" |
| `BeginAllyFormUp` / `BeginAllyFollowing` / `BeginAllyApproach` / `BeginAllyAmbush` | |
| `BeginEnemyFleeToZone` | `BeginEnemyFlee`도 이걸 거침 |
| `MoveUGVToScenarioDestination` | |
| `FireScenarioStep` | |
| `IssueSquadOrderSpec` | 콘솔 `titan.SquadOrder` 포함 |
| `RetargetAllEnemies` / `BeginEnemyEngage` | |

  제외: 값만 저장하는 세터(`SetUGVStandbyDestination`, `SetUGVEnemyDistanceSpeedLimit` 등).
- **데모 자동 시작은 연기**: `ApplyDemoRunModeSetup`이 미준비면 `DemoSetupPendingWorld = World` + 로그 "데모 세팅(지휘소 자동사격·자동 시작) 연기 — 호스트 로딩 완료(페이드인) 후 적용." 후 타이머를 안 건다. `NotifyHostWorldReady`가 pending 월드가 같고 설정 액터가 등록돼 있으면 "연기했던 데모 세팅 적용" 후 다시 호출 → 자동사격 1 s·자동 시작 3 s가 **페이드인 시작부터** 세어진다. pending도 월드 키라 월드가 바뀌면 자동 무효.
- 준비 신호는 호스트/솔로만, 클라는 아무것도 안 보냄. `NotifyHostWorldReady`도 NM_Client면 무시(이중 방어), 현재 월드가 아니면 무시(경고).
- fail-closed: 로딩 시스템이 대상 월드를 못 잡으면 게이트가 안 풀린다 — 그래서 대상 판정은 "로비가 아닌 모든 게임 월드"로 넓게 잡았다.

## 4. 설계 대비 결정/차이 (이유)

1. **준비 신호는 페이드인 "끝"이 아니라 "시작"** — 설계 §4-3 "3초는 페이드인부터". 페이드인 1 s가 끝나고 세면 데모 자동 시작이 4 s 뒤가 된다.
2. **로딩 중 소리 차단 = 카메라 페이드의 `bFadeAudio=true`** — 엔진이 페이드 중 오디오 장치 transient primary volume을 `1-FadeAmount`로 둔다(`PlayerCameraManager.cpp:488-507, 994-996`). 워밍업 Niagara가 사운드를 내더라도 안 들린다(설계 "소리는 내지 않는다"의 보장 수단).
3. **워밍업 Niagara 컴포넌트에 `SetHiddenInSceneCapture(true)`** — RCWS 조준경·드론·환경 카메라 RTSP는 씬 캡쳐라 카메라 페이드가 안 덮는다. "메인 카메라 앞에만 두면 캡쳐에 안 잡힌다"(설계)는 캡쳐 시야에 따라 보장이 안 되므로 명시적으로 뺐다. **데칼은 예외** — `UDecalComponent`는 `USceneComponent` 파생(엔진 `DecalComponent.h:24`)이라 이 함수가 없다(첫 빌드 C2039로 확인, 제거함). 지면 탄흔 1개가 `WarmupFrames`(기본 3프레임)만 살아 있으므로 캡쳐에 잠깐 잡힐 수 있는 것은 감수. 메인 뷰 RTSP는 카메라 페이드로 검정(설계 §3-C 근거 그대로).
4. **준비 조건에 "로컬 PC(카메라 있음) 존재" 추가** — 클라는 PC가 복제로 늦게 생겨서, 이게 없으면 카메라 검정 고정·워밍업 스폰 전에 조건이 다 맞아 버릴 수 있다.
5. **오버레이 = 순수 Slate 위젯을 `UGameViewportClient::AddViewportWidgetContent`로 ZOrder 10000** — 엔진은 LoadMap에서 뷰포트 위젯을 안 지운다(뷰포트 파괴 때만 `RemoveAllViewportWidgets`, `GameViewportClient.cpp:472,480`) → PIE에서 트래블을 넘어 남아 새 월드 첫 프레임부터 덮는다. 로딩 중엔 `Visible`(마우스 차단), 페이드인 중엔 `HitTestInvisible`. 텍스트/로고도 페이드인과 함께 투명해진다.
6. **자체방호 Monitor1(별도 `SWindow`)** — PC의 기존 public `GetSelfDefenseMonitor1WidgetInstance()->GetCachedWidget()` → `FSlateApplication::FindWidgetWindow`로 창을 찾아 `SWindow::AddOverlaySlot(10000)`에 같은 위젯을 얹는다. 창이 늦게 생겨도 매 틱 재시도. **PC 헤더 무변경.**
7. **호스트 워밍업 중 네트워크 실패는 로딩을 중단하지 않음** — 중단하면 준비 신호가 영영 안 나가 게이트가 닫힌 채로 남는다(fail-closed). 클라/트래블 단계 실패만 중단 + 로비 화면 복구.
8. **PIE 첫 월드도 잡힌다** — `UGameInstance::Init`(`GameInstance.cpp:364`, GI 서브시스템 Initialize)이 `PostCreatePIEWorld → InitWorld`(`PlayLevel.cpp:2422`)보다 먼저라 PostWorldInitialization 구독이 첫 PIE 월드에 걸린다.
9. **MoviePlayer 설정은 `PreLoadMapWithContext`** — 같은 지점에서 `PreLoadMap`보다 먼저 브로드캐스트되고(`UnrealEngine.cpp:16108-16109`), MoviePlayer는 `PreLoadMap`에서 재생을 시작한다(`DefaultGameMoviePlayer.cpp:928-939`) → 순서상 Setup이 재생보다 앞선다.
10. **워밍업 대상 목록**
    - Niagara 23개(설계 §3-C 목록 그대로): 23개 모두 Content에 존재하고 다른 에셋(BP/레벨)이 참조함을 확인.
    - 데칼 초기값은 `/Game/Gun_effect/Decal_Bullet/Demo/Materials/M_Decal_Bullet` 하나 — `BP_RifleProjectile`(SoldierLab·Soldiers 2개)·`BP_RCWSProjectile`이 참조. **혈흔 데칼 재질은 경로 미확인이라 비워둠**(확인되면 ini에 한 줄 추가).
    - ini의 소프트 참조는 그 자체로 쿡 대상이 되지 않는다 — 실사용 BP가 참조하므로 쿡에 들어감. 사용처가 사라지면 목록에서 빼거나 쿡 대상에 추가할 것(ini 주석에도 기재).

## 5. cvar

| cvar | 기본 | 의미 |
|---|---|---|
| `titan.Loading.Enabled` | 1 | 0=오버레이/워밍업 끔, 대상 월드 BeginPlay에서 즉시 준비 신호(게이트는 동작) |
| `titan.Loading.UseMoviePlayer` | 1 | LoadMap 동안 MoviePlayer(-game/패키지 전용) |
| `titan.Loading.LobbyFadeOutSeconds` | 0.5 | 로비 open 전 페이드아웃 |
| `titan.Loading.FadeInSeconds` | 1.0 | 워밍업 후 페이드인(준비 신호는 시작 시점) |
| `titan.Loading.MinHoldSeconds` | 1.0 | 오버레이 최소 유지 |
| `titan.Loading.StableFrames` | 30 | 조건 연속 충족 프레임 수 |
| `titan.Loading.TimeoutSeconds` | 30 | 하드 타임아웃, `MinHoldSeconds+1`보다 작으면 그 값으로 올림 |
| `titan.Loading.MaxFrameMs` | 60 | 프레임 안정 임계(ms) |
| `titan.Loading.MaxStreamingWanting` | 0 | 스트리밍 대기 리소스 허용치 |
| `titan.Loading.MaxPSOPrecache` | 0 | 진행 중 PSO 프리캐시 요청 허용치 |
| `titan.Loading.WarmupFrames` | 3 | 워밍업 이펙트 유지 프레임 |
| `titan.Loading.WarmupDistanceCm` | 500 | 카메라 앞 스폰 거리 |
| `titan.Loading.ForceUnmet` | 0 | 검증용, 1=강제 미충족(타임아웃 경로) |

참고: 비-Shipping 빌드에서 `-NoLoadingScreen` 커맨드라인은 엔진이 MoviePlayer를 끈다. `titan.Loading.UseMoviePlayer 0`도 같은 효과(뷰포트 오버레이·워밍업은 그대로).

## 6. 조사 결과 — 설계 §6 미결 2건 (보고만, 코드 변경 없음)

### 6-1. SoldierLab 분대가 시나리오와 무관하게 스스로 움직이는가
- 분대에 초기 명령은 없지만, 기본 `FSoldierAssignment`의 ROE가 **`Free`**, `EngageRangeCm=0`(`SoldierOrderTypes.h:150,161`)이고, 교전 틱은 배정이 없어도(`NoAssignment`) 돈다(`SoldierEngagement.cpp:996-997`). 적대 병력을 숨기는 브리지 `SetActorHiddenInGame`은 SoldierLab 인지를 끄지 않는다.
- → **BeginPlay 시점에 적을 볼 수 있는 병사는 시나리오 스텝과 무관하게 스스로 사격/엄폐한다.** `SoldierObjective`가 없으면 그 외엔 제자리에서 스캔 회전만 한다. 구 titan `EnemyCombat`/`AllyFormation`은 Standby 유지.
- 결론: 로딩 시간이 시작 전 상태를 바꿀 수 있는 경우는 **스폰 시점에 대립 SoldierLab 병사끼리(또는 적대 병력 ↔ UGV/차량) 시선이 닿을 때뿐** — 배치 의존, 실측 안 함. 사용자 결정 대기.

### 6-2. 풀시스템 모드 LIG 명령이 §2 진입점을 거치는가
- `Network/UGVRemoteControlSubsystem.cpp`는 시나리오 서브시스템을 **`IsDemoMode()` 확인에만** 쓴다(`:185-190`). **시나리오를 시작하는 LIG 명령은 없다.**
- `RC_OperationMode`/`RC_RemoteDriving`/`RC_Movement`/`RC_FireWeapon`/`RC_ActivateFire`/`RC_ChargeWeapon`/`RC_FireMode`와 확장 `HQ_MissionMoveToEngage`(`:731-767`, `MoveToDestination` 직접 호출)는 UGV/RCWS를 직접 구동 → **로딩 게이트 대상 아님**.
- 이 서브시스템은 `Initialize`부터 코어 티커(0.02 s)로 돌고, 활성 조건은 `클라 아님 && !IsDemoMode() && PC0 축 UGV` — **로딩을 기다리지 않는다.** 즉 풀시스템 모드에선 로딩 오버레이 중에도 LIG 명령으로 UGV가 움직일 수 있다.
- 엣지: `IsDemoMode()`는 `AScenarioConfig`가 등록되기 전엔 false(`-demo`/URL 오버라이드 제외) → 설정 액터로만 데모를 켠 레벨에선 등록 전 잠깐 소켓이 열릴 수 있다.

## 7. 검증 절차 (설계 §5 대응 — 진행 상태는 §8-4 표)

> 오버레이는 월드 BeginPlay에서 시작하므로 cvar는 **월드 로드 전에** 넣어야 한다: `DefaultEngine.ini [ConsoleVariables]` 또는 `-ExecCmds="titan.Loading.MinHoldSeconds 30,titan.Loading.TimeoutSeconds 60"`. PIE는 PIE 시작 전 에디터 콘솔에서 설정.

1. `titan.Loading.MinHoldSeconds 30` + `titan.Loading.TimeoutSeconds 60`:
   - ① 데모: 로그 순서 `[ScenarioStateSubsystem] 데모 세팅 … 연기` → `호스트 로딩 완료 — 시나리오 게이트 해제` → `연기했던 데모 세팅 적용`, 그로부터 ~3 s 뒤 `데모 자동 시작`. 스텝 로그·"적 특작부대 침투" 알림이 페이드인 전에 안 찍힐 것.
   - ② 오버레이 중 콘솔 `BeginScenarioEnemyContact` / `titan.ScenarioRestart` / `BeginAllyFormUpAndAdvance` / `BeginEnemyFlee` / `titan.SquadOrder Hostile * MoveTo 0` → 각각 `… 거부 — 호스트 로딩 중` 로그, 아무 일 없음. 페이드인 후 같은 명령 → 정상. 주의: `BeginAllyFormUpAndAdvance`는 게이트 걸린 `BeginAllyFormUp`이 거부하기 전에 UGV 대기 목적지/헤딩 값(값 저장 세터, 게이트 제외)을 먼저 저장할 수 있다 — 유닛은 안 움직임.
   - ③ `데모 자동사격: 이동형지휘소` 로그는 게이트 해제 로그 **뒤에만** 나올 것.
2. 로비 없이 New_kadex_0811에서 PIE 직접 시작: `[LoadingScreen] 대상 월드 판정: … 로딩 오버레이 + 워밍업 시작` → `로딩 종료 — 사유=정상`.
3. 준비 후 `titan.ScenarioRestart` 3회 연속 — `거부` 로그 0.
4. 2-PC: 클라 로그 `[LoadingScreen] 클라이언트 — 자기 화면만 정리(서버로 보내는 신호 없음)`; 클라 접속 시 호스트 로그에 `[LoadingScreen]` 줄 없음, 호스트 시나리오 영향 0.
5. RTSP 메인뷰 수신 화면이 오버레이 동안 검정.
6. 첫 발사·첫 피격 히치: `titan.Loading.Enabled 0` vs `1`을 PIE와 `-game` 각각 비교(`stat unit` 또는 로그 프레임 간격).
7. `titan.Loading.ForceUnmet 1` → `TimeoutSeconds` 후 `로딩 종료 — 사유=타임아웃 (… 마지막 미충족=[강제미충족…])`, 이후 시나리오 정상 시작.
- 추가: `-game`에서 `-NoLoadingScreen`(비-Shipping) 또는 `titan.Loading.UseMoviePlayer 0` → LoadMap 동안 MoviePlayer 없이 진행되는지.

## 8. 2026-10-06 빌드 후 수정·PIE 확인

### 8-1. 첫 빌드 C2039
- `UDecalComponent`는 `USceneComponent` 파생(엔진 `DecalComponent.h:24`)이라 `SetHiddenInSceneCapture`가 없음 → 데칼 쪽 호출만 제거. Niagara는 그대로 캡쳐 제외. 결과 정리는 §4-3.

### 8-2. 텍스트/폰트 교체 (사용자 요청 — 한글 "로딩중"이 싸 보임)
- `LoadingText` 기본값 "로딩중" → **"Loading"**(점 애니메이션으로 "Loading...").
- 설정 추가: `UTitanLoadingScreenSettings::LoadingFont`(`TSoftObjectPtr<UFont>`), `LoadingFontSize`(int32, 기본 28).
- `DefaultGame.ini`: `LoadingText=Loading`, `LoadingFont=/Game/Font/Pretendard-Medium_Font.Pretendard-Medium_Font`, `LoadingFontSize=28`.
  - 주의: `UFont` 에셋은 **`_Font`가 붙은 쪽**. `/Game/Font/Pretendard-Medium` 자체는 FontFace라 이 슬롯에 안 맞음.
  - 쿡: 이 폰트는 `WBP_kadex`·`WBP_ConfirmDialog`·`WBP_RestartPrompt` 등이 참조하므로 쿡에 들어감(ini 소프트 참조만으로는 쿡 대상 아님 — §4-10과 같은 원칙).
- 서브시스템이 로드한 폰트를 `UPROPERTY() TObjectPtr<UFont> LoadingFont`로 붙잡음 — MoviePlayer 로딩 스레드도 같은 위젯으로 그리므로 GC 안전용. 로드 실패 시 엔진 기본 bold 폰트로 폴백 + 경고 로그 "로딩 폰트 로드 실패".
- 향후(사용자 계획): 디자이너가 만든 전체 화면 이미지로 교체 예정 → 배경 이미지 옵션 + 텍스트 숨김 옵션 추가하면 됨. **WBP로 안 간 이유**: MoviePlayer 단계(LoadMap)는 로딩 스레드에서 렌더하므로 UObject 위젯(UMG)은 위험 — 순수 Slate 유지.

### 8-3. 버그 — 워밍업 이펙트가 월드 원점에 스폰
- 증상: 로그 `워밍업 이펙트 스폰 … @ V(X=500.00)`(원점 + 앞 500 cm), 데칼은 "카메라 앞 지면을 못 찾아 생략".
- 1차 수정(불충분): 카메라 위치가 원점 근처가 아닐 때까지 대기, `NoCameraSkipSeconds`=10 s 뒤 폴백. → 2차 PIE에서 정확히 10 s 폴백 시점에 여전히 원점 스폰. 대기만으로는 해결 안 됨.
- **진짜 원인**: 로컬 PC를 `APlayerController::IsLocalController()`로 찾았는데, 이 함수는 Standalone/Client에서 NetDriver가 있으면 **모든 PC에 true**(엔진 `PlayerController.cpp:343-349`) → `ULocalPlayer` 없는 PlayerController 파생 `AUGVAIController`(카메라가 원점)를 집었다. 프로젝트 기존 함정 "AUGVAIController는 사실 PlayerController"의 재발.
- 수정: `TitanLoading::IsViewingPlayerController(PC)` = `PC->GetLocalPlayer() != nullptr`. 사용처 4곳 전부 교체 — `FindLocalPlayerControllerWithCamera`, `HoldLocalCamerasBlack`, `StartLocalCameraFade`, Monitor1 창 찾기. 2-PC 리슨 호스트에서 UGV 컨트롤러가 "로컬 PC"로 잡히는 것도 같이 막힘.
- 1차 수정의 "카메라가 원점이 아닐 때까지 대기"는 추가 안전장치로 유지. 진단 로그 `[LoadingScreen] 워밍업 카메라 — PC=…, 뷰타겟=…, 카메라=…` 추가.
- 미변경 참고: `UScenarioStateSubsystem::FadeLocalCameras`(기존 코드)도 `IsLocalController`를 쓰지만 무해 — UGV 컨트롤러의 안 쓰는 카메라까지 페이드할 뿐. 손대지 않음.

### 8-4. PIE 결과 (단독 PIE, New_kadex_0811, standalone, 데모)

| 회차 | 결과 |
|---|---|
| 1 (22:38) | 정상 종료 20.3 s. 원점 스폰(버그). 순서는 정상: 데모 세팅 연기 → 로딩 종료 정상 → 게이트 해제 → 연기했던 데모 세팅 적용 → 지휘소 자동사격 +1.0 s → 데모 자동 시작 +3.0 s → 스텝 평가 시작 |
| 2 (23:00) | 정상 종료 12.8 s. 1차 수정 후에도 정확히 10 s 폴백에서 원점 스폰(불충분 확인). 순서 정상(동일) |
| 3 (23:07, 최종) | `PC=BP_TestPlayerController_C_0`, `뷰타겟=BP_UGV_0901_C_1`, 카메라 `V(X=59267,Y=13471,Z=-3723)` — 2프레임째. Niagara 23 + 데칼 1을 ~5 m 앞에 스폰, 3프레임 뒤 제거. 정상 종료 11.3 s(안정 직전 마지막 미충족 = `프레임=62.4ms`). 게이트 해제 → 데모 세팅 → 자동사격 +1.0 s → 자동 시작 +3.0 s → 스텝 시작 |

- 로딩 시간 구성: 워밍업 에셋 동기 로드 ~3.3 s(PIE는 Niagara 컴파일 대기 포함, 로그상 ~6.2 s짜리 한 프레임으로 보임) + 이후 프레임 시간 안정화 대기. `-game`/패키지는 미측정.
- 로그의 에러는 무관: `UGVChaosPawn` CDO의 `SK_UGVChaos` 누락, CommonUI 뷰포트 경고.

§7 검증 항목 상태:

| §7 항목 | 상태 |
|---|---|
| 1-① 데모 로그 순서 | **부분 확인** — 기본 cvar(MinHold 1 s)로 3회 모두 순서 정상. `MinHoldSeconds 30` 조건은 미실시 → **검증됨(2026-10-06, -game MinHold 30/Timeout 60, [s6_verification](../infra_architecture/2026-10-06_s6_verification.md) L1)** |
| 1-② 로딩 중 콘솔 명령 거부 | 미실시 → **검증됨(2026-10-06, -game 5종 모두 거부, [s6_verification](../infra_architecture/2026-10-06_s6_verification.md) L2)** — `?Demo=0` 회차는 미실시 |
| 1-③ 자동사격이 게이트 해제 뒤 | 확인(로그 순서) → MinHold 30 -game에서도 검증됨([s6_verification](../infra_architecture/2026-10-06_s6_verification.md) L1) |
| 2 PIE 직접 시작 | **확인** → -game·`Enabled=0` PIE까지 검증됨([s6_verification](../infra_architecture/2026-10-06_s6_verification.md) L3) |
| 3 재시작 3회 | 미실시 → **검증됨(2026-10-06, 거부 0, [s6_verification](../infra_architecture/2026-10-06_s6_verification.md) L4)** |
| 4 2-PC | 미실시 → 여전히 미실시(수동 목록, [s6_verification](../infra_architecture/2026-10-06_s6_verification.md) L5·§4) |
| 5 RTSP 메인뷰 검정 | 미실시 → **미통과·의문(2026-10-06)** — 오버레이 중 5.65 s 뒤 송출이 검정에서 풀림, 원인 미확정 → [s6_verification](../infra_architecture/2026-10-06_s6_verification.md) L8 |
| 6 첫 발사 히치 비교 | 미실시 → 부분(PIE 1샘플씩, 차이 미확정, -game 미실시) → [s6_verification](../infra_architecture/2026-10-06_s6_verification.md) L9 |
| 7 `ForceUnmet` 타임아웃 | 미실시 → 여전히 미실시([s6_verification](../infra_architecture/2026-10-06_s6_verification.md) L6). 단 실제 타임아웃 2회 관측 — 원인은 프레임 조건(MaxFrameMs 60), L7 |
| 추가 `-game` MoviePlayer 화면 | 미실시 → 부분(로그 `MoviePlayer 로딩 화면.` 확인, `UseMoviePlayer 0` 비교 미실시) → [s6_verification](../infra_architecture/2026-10-06_s6_verification.md) L10 |
| 추가 `-game` 워밍업 이펙트 | **발견(2026-10-06)** — R01에서 첫 프레임 전에 `NoCameraSkipSeconds` 10 s 만료로 워밍업 생략(R02는 성공) → [s6_verification](../infra_architecture/2026-10-06_s6_verification.md) F1 |
