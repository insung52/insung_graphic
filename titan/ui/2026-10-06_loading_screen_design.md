# 로딩 화면 + 워밍업 + 호스트 시나리오 게이트 설계

2026-10-06 / 구현 완료(검증 대기) / 로비→New_kadex_0811 전환에 페이드·로딩 화면을 넣고, 그 아래서 Niagara/PSO 워밍업과 안정화 대기를 한 뒤 페이드인; 호스트 시나리오는 호스트 로딩 완료 전 어떤 경로로도 시작 불가, 클라 로딩은 자기 화면 전용.

> **구현(2026-10-06)**: [`2026-10-06_loading_screen_impl.md`](2026-10-06_loading_screen_impl.md) — 실제 구현, 설계 대비 결정/차이, §6 미결 조사 결과, 검증 절차.
> 출처: S6 감사(`infra_architecture/2026-09-29_s6_code_audit.md` §8)의 후속. 근거 표기 [A] = 소스/엔진 file:line·로그 확인, [B] = 추정(구현 시 확인).
> 목표 범위: **품질은 그대로, 비용을 로딩 시점으로 옮긴다.** 시나리오 내용(스텝·타이밍·AI 동작)은 바꾸지 않는다.

---

## 1. 요구사항 (사용자 확정, 2026-10-06)

1. 로비 → 게임맵 전환 시 페이드아웃 → 로딩 화면 → (안정화) → 페이드인.
2. **클라이언트 로딩은 자기 화면에만.** 호스트에 RPC·신호 일절 없음, 호스트 시나리오 시스템에 관여 0. 클라는 지금처럼 IP 접속해 복제로 합류.
3. **호스트 시나리오는 호스트 로딩 화면이 끝나야만 시작.** 준비 전 시작은 어떤 경로로도 절대 금지.
4. 시나리오의 기본 시작은 **명령어**(`BeginScenarioEnemyContact`), 데모 자동 시작은 편의 기능. 둘 다 게이트 대상.

## 2. 현재 구조 [A]

- **하드 트래블**만 사용(시임리스·TransitionMap 없음). `GameDefaultMap=/Game/kadex_lobby`(`Config/DefaultEngine.ini:69`).
  - 호스트: `HostListenServer` → `open <map>?Listen?Axis=UGV?Demo=%d`(`titan_examplePlayerController.cpp:2433`), 솔로 `StartSoloAxis`(`:2462`), 클라 `ConnectToHost` → `open <IP>?Axis=SelfDefense`(`:2478`). 클라는 호스트와 **다른 시점에 따로** LoadMap.
- `UEngine::LoadMap`: `PreLoadMap`(엔진 `UnrealEngine.cpp:16109`) → … → 액터 BeginPlay(`:16661`) → `PostLoadMapWithWorld`(`:16665`). 즉 `AScenarioConfig::BeginPlay`가 PostLoadMap보다 먼저.
- `AScenarioConfig::BeginPlay` → `UScenarioStateSubsystem::RegisterScenarioConfig`(`UI/ScenarioStateSubsystem.cpp:565`) → `ApplyDemoRunModeSetup`(`:609`, 호출 `:590`): 서버+데모면 **BeginPlay 시점에** 자동사격 타이머 1 s(`:645`)와 자동 시작 타이머 `DemoAutoStartDelaySeconds`(기본 3 s, `ScenarioConfig.h:126`; `:648-653`)를 무장. 재시작도 같은 함수를 다시 부른다(`:1036`).
- `UScenarioStateSubsystem`은 **GameInstance 서브시스템**(트래블을 넘어 생존). 월드 교체 감지 `ResetForNewWorldIfNeeded`(`:1669`) → `ResetScenarioRuntimeState`(`:1705`) — **이 함수는 재시작 경로와 공유**된다(헤더 주석).
- 재활용할 것: `FadeLocalCameras`(`:1190-1203`, private) · `UTitanGraphicsSubsystem`의 `OnPostWorldInitialization → World->OnWorldBeginPlay` 패턴(`Settings/TitanGraphicsSubsystem.h:40-50`, PIE에서도 발동) · `UTitanHealthRecorderSubsystem`의 PreLoadMap/PostLoadMap 바인딩(`Diagnostics/…/TitanHealthRecorderSubsystem.cpp:1011-1012`; PIE 시작은 PreLoadMap 안 탐 `:1144`).

### 시나리오 시작 진입점 전수 [A]

모든 스텝은 `BeginScenarioSteps()`(`:1489`) 하나로 시작하고 호출자는 `BeginEnemyContactScenario`(`:1241`)와 `StartUGVAdvance`(`:1486`) 둘뿐.

| 경로 | 위치 |
|---|---|
| **명령어 `BeginScenarioEnemyContact`**(기본 시작 방식) → 서버면 로컬, 클라면 `Server_BeginScenarioEnemyContact` RPC → `BeginEnemyContactScenario` | `titan_examplePlayerController.cpp:2004-2027`, `.h:944` |
| 데모 자동 시작 타이머 → `DemoAutoStartScenario`(`:716-727`) → `BeginEnemyContactScenario` | `ScenarioStateSubsystem.cpp:648-653` |
| 데모 RCWS 자동사격(지휘소 ARM+AutoFire) 타이머 | `:645` → `ApplyDemoRCWSAutoFire` |
| `BeginAllyFormUpAndAdvance` 등 시나리오 Exec(→ Server RPC → 서브시스템) → 지연 타이머 `StartUGVAdvance` | `PlayerController.h:611-651`, `ScenarioStateSubsystem.cpp:1422-1451` |
| 재시작: 확인창/카운트다운/`titan.ScenarioRestart`/ScenarioMonitor fail-safe → `RequestScenarioRestart`(`:825`) | `:138, 799-807`, `ScenarioMonitorSubsystem.cpp:713`, `PlayerController.cpp:2030-2060` |
| 풀시스템 모드 LIG 통제기 명령 | `Network/UGVRemoteControlSubsystem.cpp` — 위 함수들로 들어오는지 **구현 시 확인 [B]** |

## 3. 설계

새 `UTitanLoadingScreenSubsystem : UGameInstanceSubsystem`(트래블을 넘어 생존) 하나가 A~D 단계를 맡고, 시나리오 쪽은 게이트(§4)만 추가한다.

### A. 로비 페이드아웃
- `HostListenServer` / `StartSoloAxis` / `ConnectToHost`에서 즉시 `ConsoleCommand(open…)` 대신: 로컬 카메라 `StartCameraFade(0→1, T, 검정, hold)` → T초 타이머 → `open`. (`open`이 게임 스레드를 막아서 즉시 열면 페이드가 화면에 안 나간다.)
- 접속/트래블 실패(`GEngine->OnNetworkFailure()/OnTravelFailure()`) 시 로딩 상태 해제(엔진이 로비로 되돌리며 새 맵이라 페이드는 자연 해제).

### B. LoadMap 동안 — MoviePlayer Slate 화면
- `PreLoadMap`에서 `GetMoviePlayer()->SetupLoadingScreen(FLoadingScreenAttributes)`: Slate 위젯(검정 + 로고 + "로딩중"), `bAutoCompleteWhenLoadingCompletes=true`, `bWaitForManualStop=false`.
- **PIE/에디터에선 비활성**: `IsMoviePlayerEnabled() = !GIsEditor && !IsRunningDedicatedServer() && … && GUseThreadedRendering`(엔진 `Runtime/MoviePlayer/Private/MoviePlayer.cpp:123-132`) [A]. `-game` Standalone·패키지(Windows/Linux)에서 동작.
- `titan_example.Build.cs`에 `"MoviePlayer"` 추가.
- **워밍업 단계까지 끌지 않는다** — MoviePlayer가 떠 있는 동안엔 게임 뷰가 그려지지 않아 PSO/스트리밍이 실제 뷰로 데워지지 않는다 [B: 엔진 `DefaultGameMoviePlayer.cpp` WaitForMovieToFinish 경로]. LoadMap이 끝나면 바로 C로 넘긴다.

### C. 워밍업 오버레이 — "진입 후 몇 초 불안정"을 실제로 없애는 단계
- 훅: `OnPostWorldInitialization → World->OnWorldBeginPlay`(PIE 포함). 대상 월드: 게임 월드이면서 `GameDefaultMap`(로비)이 아닌 것.
- 표시: `GameViewport->AddViewportWidgetContent(오버레이, 높은 ZOrder)` — PC 위젯이 아니라 뷰포트에 붙여 첫 프레임부터 덮고, `Client_OnAxisResolved`/BeginPlay 순서 경합(`PlayerController.cpp:111-121`)과 무관하게.
- **아래 월드는 계속 렌더**(`SetWorldRenderingEnabled(false)` 금지 — 과거 버그 `:123-126`, 그리고 워밍업은 실제로 그려져야 함).
- **카메라 페이드도 검정 유지**: 카메라 페이드(`PlayerCameraManager` FadeAmount → `ViewInitOptions.OverlayColor`, 엔진 `LocalPlayer.cpp:813-814`)는 CombineLUTs에서 톤매핑 LUT에 구워지고(엔진 `PostProcessCombineLUTs.cpp:227,414`), RTSP 메인뷰는 톤매핑 이후(`VisualizeDepthOfField` 지점, `Plugins/RtspEncoder/…/MainViewFrameSource.cpp:46-59`)를 떠 간다 → **페이드를 걸어두면 RTSP 메인뷰 송출도 검정**, 워밍업 이펙트가 LIG 수신 화면에 안 나간다 [A]. UMG 오버레이만으로는 RTSP를 못 가린다. 씬은 여전히 렌더되므로 PSO는 컴파일된다.
- 자체방호 클라 Monitor1은 별도 네이티브 `SWindow`(`PlayerController.cpp:312-341`)라 뷰포트 오버레이가 안 덮는다 → 같은 오버레이를 그 창에도 얹거나, 준비 완료까지 내용 숨김.
- 씬 캡쳐 기반 RTSP(RCWS 조준경·드론·환경)는 카메라 페이드 대상이 아니다 — 로딩 중 원본 영상이 나간다(현재도 동일, 변경 없음). 워밍업 스폰을 메인 카메라 앞에만 두면 캡쳐에 안 잡힌다.

**워밍업 작업**(전부 로컬·비복제·결과 동일):
1. 대상 Niagara 시스템 동기 로드 → 에디터 빌드면 `UNiagaraSystem::WaitForCompilationComplete()`(PIE는 컴파일 ID가 무효인 에셋을 첫 활성화 때 컴파일함, 엔진 `Plugins/FX/Niagara/…/NiagaraSystem.cpp:198-203,966-970`, `NiagaraComponent.cpp:1352-1360`).
2. 각 시스템을 메인 카메라 앞 ≈5 m에 `SpawnSystemAtLocation`(bAutoDestroy) 1~2프레임 활성 후 비활성 — **에디터는 컴포넌트 PSO 프리캐싱이 꺼져 있어**(`PSOPrecache.cpp:141` `&& !GIsEditor`) 실제로 그려야 PSO가 컴파일된다. 쿡 빌드는 InitializeSystem 시 프리캐시(`NiagaraComponent.cpp:1253`).
3. 탄흔/혈흔 데칼 재질도 같은 방식으로 1회.
4. **소리는 내지 않는다**(워밍업 중 사운드 재생 금지). MetaSound는 재생 없이 `UMetaSoundCacheSubsystem::PrecacheMetaSound`(선택, 별도 작업).
5. 대상 목록은 **`UDeveloperSettings`(Project Settings, `DefaultGame.ini`)** 에 둔다(에셋을 MCP로 만들지 않는다는 프로젝트 규칙). 초기값 = 로그에서 런타임 컴파일이 확인된 시스템:
   `/Game/SoldierLab/Effects/Muzzle/NS_MuzzleFlash`, `/Game/SoldierLab/Effects/Tracer/NS_Rifle_Tracer`, `/Game/SoldierLab/Effects/Impact/NS_Rifle_{Dirt,Metal,Wood,Hard,Glass}`, `/Game/SoldierLab/Effects/Impact/hit_enemy/ParticleEffects/NS_Blood`, `/Game/VFX/Rifle/NS_Rifle_{Dirt,Metal,Wood,Hard,Glass}`, `/Game/VFX/RCWS/NS_RCWS_{Dirt,Wood,Hard}`, `/Game/VFX/Blood/NS_Blood_RCWS`, `/Game/NiagaraExamples/FX_Weapons/MuzzleFlashes/NS_MuzzleFlash_UGV`, `/Game/NiagaraExamples/FX_Weapons/MuzzleFlashes/NS_MuzzleFlash`, `/Game/ResidentHorrorV1/ParticleEffects/NS_Blood`, `/Game/Vehicles/UGV/Effects/NS_BarrelSmoke`, `/Game/Effects/NS_FallingLeaves`, `/Game/Effects/NS_AmbientDust`(로그 `Saved/Logs/*.log` "Compiling System" 집계). 실제 사용 여부는 구현 시 각 BP 참조로 재확인.

**준비 조건** — 아래가 N프레임(예: 30) 연속 충족 + 최소 유지 시간(예: 1 s), **하드 타임아웃(예: 30 s)**:
1. 워밍업 작업 완료.
2. `!IsAsyncLoading()`.
3. `IStreamingManager::Get().GetNumWantingResources() == 0`(엔진 `Runtime/Engine/Public/ContentStreaming.h:321`) — 숲 씬에선 0이 안 될 수 있어 임계값 또는 타임아웃 의존.
4. `PipelineStateCache::NumActivePrecacheRequests() == 0`(엔진 `Runtime/RHI/Public/PipelineStateCache.h:286`); 오버레이 동안 `PrecachePSOsBoostToHighestPriority(true)`(`:289`), 해제 시 false.
5. 프레임 시간 안정(`FApp::GetDeltaTime()`이 임계 이하 M프레임 연속).
- 해제 시 어떤 조건으로 끝났는지(정상/타임아웃 + 미충족 조건) 로그. 수치(임계·N·타임아웃)는 cvar로.

### D. 페이드인 → 준비 신호
- 오버레이 제거 + 카메라 페이드 1→0.
- **NM_Client가 아닌 프로세스(호스트·솔로)만** `UScenarioStateSubsystem::NotifyHostWorldReady(World)` 호출. 클라는 오버레이만 내리고 끝(서버로 아무것도 안 보냄).

## 4. 호스트 시나리오 게이트

1. **준비 상태 = 월드 단위로 키잉**: `TWeakObjectPtr<UWorld> HostReadyWorld`, `IsHostWorldReady() = (HostReadyWorld == GetWorld())`. 월드가 바뀌면 자동으로 미준비가 된다.
   - ⚠ **`ResetScenarioRuntimeState`에 넣지 말 것** — 재시작 경로와 공유하는 함수라, 넣으면 재시작이 준비 상태를 지우고 로딩은 다시 안 돌므로 **재시작 후 시나리오가 영영 안 시작된다.**
2. **게이트는 `UScenarioStateSubsystem` 진입 함수에** 둔다(PC·RPC·콘솔 쪽이 아니라 — 모든 경로가 여기로 모이므로 한 곳만 지키면 빠짐이 없다): `BeginEnemyContactScenario`, `StartUGVAdvance`, `BeginScenarioSteps`(최종 방어선), `RequestScenarioRestart`, `ApplyDemoRCWSAutoFire`, 그리고 유닛을 움직이는 시나리오 명령 위임 함수(`BeginAllyFormUp`/`Following`/`Approach`/`Ambush`, `BeginEnemyFlee`, `MoveUGVToScenarioDestination` 등). 값만 저장하는 명령(`SetUGVStandbyDestination`, `SetUGVEnemyDistanceSpeedLimit`)은 제외.
   - 미준비 시 **명령어는 거부 + 경고 로그**("로딩 중 — 완료 후 다시 입력"). 대기열에 넣었다가 나중에 실행하지 않는다(예상 못 한 시점 시작 방지).
3. **데모 자동 시작은 거부가 아니라 연기**: `ApplyDemoRunModeSetup`이 미준비면 타이머를 걸지 않고 `bDemoSetupPending=true`; `NotifyHostWorldReady`가 (설정 액터 등록돼 있고 pending이면) 그때 `ApplyDemoRunModeSetup` 실행 → 3 s가 **페이드인부터** 세어진다. 순서가 반대(준비 신호가 Register보다 먼저)여도 `RegisterScenarioConfig`에서 준비 상태를 보고 바로 무장.
4. 재시작은 레벨 리로드가 아니므로 로딩을 다시 안 거치고, 같은 월드는 계속 준비 상태 → 재시작 흐름은 지금과 동일.
5. "준비 전 시작 금지"는 구조로 보장되고, **타임아웃은 로딩 종료 조건**(타임아웃 → 로딩 끝 → 그다음 시나리오)이다. 12 h 무인 운용에서 영구 대기 방지를 위해 필수.
6. 로딩 시스템이 아예 안 돈 경우(대상 월드 판정 실패 등)엔 게이트가 안 풀린다(fail-closed). 대상 판정은 "로비가 아닌 모든 게임 월드"로 넓게 잡고, 판정 결과를 로그로 남긴다.

## 5. 검증 시나리오 (구현 후)

1. 최소 유지 시간을 cvar로 30 s로 늘린 상태에서:
   - ① 데모: 스텝 로그·"적 특작부대 침투" 알림이 페이드인 전에 안 찍힘, 페이드인 후 3 s에 시작.
   - ② 로딩 중 콘솔 `BeginScenarioEnemyContact`·`titan.ScenarioRestart`·`BeginAllyFormUpAndAdvance` → 거부 로그, 아무 일 없음. 페이드인 후 같은 명령 → 정상 시작.
   - ③ 지휘소 RCWS 자동사격도 페이드인 후에만 켜짐.
2. PIE 직접 시작(로비 거치지 않음)에서도 오버레이가 뜨고 게이트가 풀림.
3. 재시작 3회 연속 정상(준비 상태 유지).
4. 2-PC: 호스트 시나리오 진행 중 클라 접속 → 클라만 자기 오버레이, 호스트 로그에 클라 관련 신호 0, 호스트 시나리오 영향 0.
5. RTSP 메인뷰 수신 화면이 로딩 동안 검정.
6. 첫 발사·첫 피격 히치: 워밍업 전/후 PIE와 `-game`에서 비교(`stat unit` 또는 로그 프레임 간격).
7. 타임아웃 강제(조건 하나를 일부러 못 맞추게) → 30 s 후 로딩 종료 + 사유 로그 + 시나리오 정상 시작.

## 6. 미결 / 구현 시 확인

- SoldierLab 분대가 시나리오 스텝과 무관하게 BeginPlay부터 스스로 이동·교전하는지 [B]. 그렇다면 로딩이 길어진 만큼 시작 전 상태가 달라진다 — **바꾸지 말고 보고**(사용자 결정).
- 풀시스템 모드 LIG 시작 명령이 §2 진입점을 거치는지 [B].
- 오버레이 디자인: 검정 + 로고 + "로딩중" 텍스트(기본안).

**조사 결과(2026-10-06, 구현 시 — 상세 `2026-10-06_loading_screen_impl.md` §6, 코드 변경 없음):**
- SoldierLab: 초기 명령은 없지만 기본 배정 ROE=`Free`(`EngageRangeCm=0`)이고 교전 틱은 배정 없이도 돈다 → 스폰 시점에 적이 보이는 병사는 시나리오와 무관하게 스스로 사격/엄폐한다. 그 외엔 제자리 스캔만. 로딩 시간이 시작 전 상태를 바꾸는 건 스폰 시 시선이 닿는 배치일 때뿐(배치 의존, 미측정) — 사용자 결정 대기.
- LIG 풀시스템: 시나리오를 시작하는 LIG 명령은 없음(`UGVRemoteControlSubsystem`은 `IsDemoMode`만 참조). 다만 RC_*/`HQ_MissionMoveToEngage`는 UGV/RCWS를 직접 구동하고 로딩을 기다리지 않음 → 로딩 게이트 대상 아님.
