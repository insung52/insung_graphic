# S1 — 카메라 파이프라인(SceneCapture · RTSP/NVENC · 메인 뷰) 비용 원인 분석

2026-09-29 / 완료(분석 전용, 코드 변경 없음) / S0 "발견 A"(CaptureScene 플러시 3.4 ms)는 숲 ISKM 비용을 앞당긴 것이지 추가 비용이 아니며, 실제 S1 몫은 ① NVENC 출력 Lock의 렌더 스레드 동기 대기 ② 320x180 캡쳐에도 도는 해상도 무관 고정비(VSM·Nanite·Lumen, ≈2.4 ms/캡쳐)다.

> 신뢰도: **[A]** = 프로젝트/엔진 소스 file:line 또는 S0 로그(`Saved/Logs/titan_example-backup-2026.09.29-02.34.40.log`) 직접 파싱. **[B]** = 추정 — 확정 방법을 같이 적음.
> 엔진 경로는 `C:\Program Files\Epic Games\UE_5.8\Engine\Source\Runtime\...` 기준 상대 경로. 출발점: `infra_architecture/2026-09-29_perf_baseline_s0.md`.

---

## 0. 한 장 요약

| # | 결론 | 신뢰도 |
|---|---|---|
| 1 | QuadCam 틱 안의 플러시 3.4 ms 중 **≈96%가 숲 `InstancedSkinnedMeshComponent` 16개의 CalcBounds+UpdatePrimitiveTransform**. 병사 메시는 거기 없다 | [A] 로그 |
| 2 | 그 비용은 **이동(앞당김)이지 이중 지불이 아니다** — 같은 프레임 프레임끝 플러시(0.585 ms)에 ISKM 항목이 없음. `CaptureSceneDeferred()`로 바꿔도 GT 절감 ≈ 0 | [A] 로그+엔진 |
| 3 | S0는 에디터 바이너리(`-game`)라 플러시가 **게임 스레드 직렬**(`AllowAsyncRenderThreadUpdatesEditorGameWorld=0`). 패키지는 ParallelFor라 3.4 ms는 과대 측정 | [A] 소스 / 감소폭 [B] |
| 4 | ISKM은 **프레임마다 전 인스턴스 바운드를 두 번** 계산(틱 1회 + 플러시 1회) — S2 몫 | [A] 소스 |
| 5 | `RtspStreamEncodeFrame`의 CPU 블록 지점 = `nvEncLockBitstream(doNotWait=false)` — 스트림의 **직전(N-1) 프레임** 인코드 완료를 렌더 스레드가 동기 대기 | [A] 구조 / "12 ms = GPU 따라잡기 대기" [B] |
| 6 | **수신 클라이언트가 없어도 전부 인코드**하고 결과만 버린다 | [A] |
| 7 | UGV 호스트: 캡쳐 1회/프레임(320x180), 인코드 3.8~5회/프레임(CCTV 4×30fps + RCWS 1080p×60fps) — S0 "4~5회"와 일치 | [A] |
| 8 | 320x180 캡쳐 GPU 3.7 ms 중 **해상도 무관 고정비 ≈2.4 ms**(VSM 1.37 · Nanite VisBuffer 0.49 · GlobalDF 0.27 · 포그/DOF/TSR). Lumen GI는 parity 코드가 캡쳐마다 **강제로 켠다** | [A] 로그+소스 |
| 9 | 쓰이지 않는데 렌더되는 캡쳐: 정상 상태엔 **없음**. RCWS 이중 렌더 제거됨, 3D 뷰 패밀리 1개 | [A] |
| 10 | "`bCaptureEveryFrame`이 동적 생성 캡쳐에서 신뢰성 없다" 주석: 엔진에 그런 실패 경로 **없음** — 옛 07-23 문구 복사본(stale) | [A] 경로 부재 / 런타임 재확인 [B] |

---

## 1. Q1 — `CaptureScene()` 플러시의 정체

### 1.1 호출 사슬 [A]

- `USceneCaptureComponent2D::CaptureScene()` → `World->SendAllEndOfFrameUpdates()`(`Engine/Private/Components/SceneCaptureComponent.cpp:832`) → `UpdateSceneCaptureContents` + `SceneRenderBuilder->Execute()`(`:834-836`), 전부 게임 스레드.
- `UWorld::SendAllEndOfFrameUpdates()`(`Engine/Private/LevelTick.cpp:1371`) → `SendAllEndOfFrameUpdatesInternal(Immediate)`(`:1120`):
  1. `OnWorldPreSendAllEndOfFrameUpdates` 브로드캐스트(`:1139`) — Niagara가 구독(`NiagaraWorldManager.cpp:527` → `:1551-1573`)해서 async 틱 중인 시뮬 전부 `WaitForInstancesTickComplete()` → **틱 중간 호출이면 GT가 Niagara 워커를 기다릴 수 있음**(교전 중 비용 [B]).
  2. 더티 컴포넌트 배열을 컨텍스트로 옮기고 **원본을 비운다**(`:1194-1203`). 비어 있으면 조기 반환(`:1143`).
  3. `bInTick`이면 각 컴포넌트 `OnEndOfFrameUpdateDuringTick()`(`:1209-1228`).
  4. `DoDeferredRenderUpdates_Concurrent()`(`ActorComponent.cpp:2645-2690`): 렌더 상태 더티면 Recreate, 아니면 `STAT_PostTickComponentLW`("Transform or RenderData") 아래 `SendRenderTransform_Concurrent` / `SendRenderDynamicData_Concurrent` / `SendRenderInstanceData_Concurrent`.
  5. `Scene->UpdatePrimitiveTransforms`(`:1303`), 스켈레탈 메시 업데이터 태스크(`:1378-1385`).
- **같은 프레임 두 번째 호출은 거의 공짜** [A] — 배열이 비어 조기 반환. Niagara 대기 브로드캐스트만 매번 돈다.

### 1.2 플러시 안에 실제로 뭐가 들었나 [A 로그]

S0 M1 `stat dumpframe`(로그 18409행, QuadCam `Post Tick Component Update` 3.418 ms):

| 자식 | ms |
|---|---|
| `InstancedSkinnedMeshComponent` ×16(BP_SplineForest_tree, Megaplants 자작나무 A~D 등) → Transform or RenderData | **3.295** |
| └ Component CalcBounds | 1.642 |
| └ UpdatePrimitiveTransform (GT) | 1.601 |
| OtherChildren(66) | 0.123 |

29808행(3.723 ms)도 동일 패턴(ISKM 3.21 + Niagara 0.26). **병사 메시는 이 플러시에 없다.**

### 1.3 추가 비용인가, 앞당긴 비용인가 → **앞당긴 것** [A]

- 같은 프레임 프레임끝 플러시(`RedrawViewports` → `FRendererModule::BeginRenderingViewFamilies` → `SendAllEndOfFrameUpdates`, `Renderer/Private/SceneRendering.cpp:5380`, `GameEngine.cpp:2033`) 0.585 ms(로그 18852행)의 내용은 **병사 `CharacterMesh0` ×25(0.155 ms) + 기타 375개** — ISKM 항목 **없음**. 30493행(0.784 ms)도 동일.
- 메커니즘: 나무는 PrePhysics에 한 번 틱해서 한 번 더티 마킹 → QuadCam 플러시가 처리하고 배열을 비움(1.1절 2) → 프레임끝엔 남은 게 없음.
- 병사는 프레임끝에만 나온다: `CharacterMesh0`은 `TG_PrePhysics`(`SkeletalMeshComponent.cpp:441`)지만 `tick.AnimationDelaysEndGroup=1`(`:2027`)로 끝 그룹이 `TG_PostPhysics`로 밀리고(`:2084-2089`), CMC PostPhysics도 메시를 움직인다(`CharacterMovementComponent.cpp:661`) → QuadCam 플러시 이후에 더티가 된다. **병사가 두 번 갱신되는 일은 이 로그에선 없다.**
- ⚠ **S0 발견 A 정정**: "순수 추가 비용 3.6 ms"가 아니라 "숲 렌더 상태 갱신 비용이 QuadCam 스코프로 집계된 것". `CaptureSceneDeferred()`로 바꾸면 같은 3.4 ms가 `RedrawViewports` 아래로 돌아갈 뿐이다.
- 틱 순서는 보장 안 됨: 캡쳐 틱은 전부 TickGroup 미지정 = **`TG_PrePhysics`**(브리프의 DuringPhysics 아님; 기본값 `TickTaskManager.cpp:2362`, 생성자 `QuadCamComponent.cpp:15`·`RCWSComponent.cpp:17`·`TitanTruck.cpp:70`·`DronePawn.cpp:54`는 `bCanEverTick`만 설정). QuadCam↔ISKM 사이 prerequisite 없음 → QuadCam이 먼저 돌면 비용이 그냥 프레임끝으로 간다 [B].

### 1.4 S0 숫자가 에디터 바이너리라 과대 [A 소스 / B 폭]

- `WITH_EDITOR` + 게임 월드면 모드 = `AllowAsyncRenderThreadUpdatesEditorGameWorld`, 기본 **0 = GameThread 직렬**(`LevelTick.cpp:801-804`, `:829-835`). 게임 스레드 모드에선 `_OnGameThread` 목록으로 강제되어 `Update_GameThread`(`:1085-1117`)에서 직렬 처리(`:1000-1006`).
- 패키지(`!WITH_EDITOR`)는 `AllowAsyncRenderThreadUpdates=1` = ParallelFor(`:793-799`). 프로젝트 `Config/`에 override 없음(grep 0건).
- 즉 S0의 3.4 ms(max 6.9)는 **비쿡 `-game` 전용 직렬 수치**. 패키지에선 워커로 분산돼 줄어든다 [B] → 측정 요청 M-1.

### 1.5 ISKM 바운드 이중 계산 (S2 인계) [A]

- `UInstancedSkinnedMeshComponent` 틱 → `UpdateBounds(); MarkRenderTransformDirty(); MarkRenderDynamicDataDirty();` **매 프레임**(`InstancedSkinnedMeshComponent.cpp:1281-1283`).
- 플러시의 `UPrimitiveComponent::SendRenderTransform_Concurrent`가 `UpdateBounds()`를 **또** 호출(`PrimitiveComponent.cpp:657`) → `Scene->UpdatePrimitiveTransform`(`:664`).
- `FInstancedSkinnedMeshComponentHelper::CalcBounds`는 **전 인스턴스 루프**(박스 변환 + 애니메이션 바운드 조회; `InstancedSkinnedMeshComponentHelper.h:189-251`, `r.InstancedSkinnedMeshes.AnimationBounds=1` 기본 `InstancedSkinnedMeshComponent.cpp:61-63`, `bFastBuild=false` 하드코딩 `:203`).
- S0의 숲 ISKM 틱 1.64 ms ≈ 플러시 CalcBounds 1.64 ms → "같은 일 두 번" 정황 [B]. **S2가 판정**(숲이 정적이면 바운드를 틱마다 다시 잴 이유가 없음).

### 1.6 프레임당 수동 `CaptureScene()` 호출 수 [A]

| 호출 지점 | 게이트 | UGV 호스트 | 자체방호 |
|---|---|---|---|
| UGV QuadCam `QuadCamComponent.cpp:309` | `GFrameCounter % CaptureEveryNTicks`(=1, `:273-277`) + 4대 라운드로빈(`:288-290`) | **1** | 꺼짐(`VehicleRtspBridgeComponent.cpp:45-48`) |
| UGV RCWS Sight `RCWSComponent.cpp:366` | `bDisableSightCapture`(`:350`) | 꺼짐(메인 뷰 경로) | — |
| 트럭 QuadCam | `TitanTruck.cpp:211-221` | 꺼짐 | 1 |
| 트럭 Battlefield `TitanTruck.cpp:376` | 슬롯 0/2(`:367-368`) | 꺼짐(`:222`) | 0.5 |
| 드론 Gimbal `DronePawn.cpp:1018` | 슬롯 1/2(`:1013-1014`) | 꺼짐(`:276-281`) | 0.5 |

UGV 호스트 = **1회/프레임**(값싼 두 번째 호출이 없음). 자체방호 = 2회(두 번째는 플러시 거의 공짜). 예외: BeginPlay 1회씩(`RCWSComponent.cpp:166`, `TitanTruck.cpp:195`, `DronePawn.cpp:361`), QuadCam 팝업 열 때 4대 동시(`QuadCamComponent.cpp:329-334`), 축 판정 전 폴링 구간(최대 5초, `RtspAxisGate.cpp:40-58`)엔 전부 캡쳐.

### 1.7 `CaptureSceneDeferred()`로 바꾸면 [A]

- 엔진: `SceneCapturesToUpdateMap`에 넣기만 함(`SceneCaptureComponent.cpp:811-821`) → `BeginRenderingViewFamilies`에서 프레임끝 플러시 **뒤**, 메인 뷰 렌더러 생성 전, 같은 `FSceneRenderBuilder` 안에서 렌더(`SceneRendering.cpp:5486-5494` → `SceneCaptureComponent.cpp:520-600`). 히트프록시 캔버스면 스킵(`SceneRendering.cpp:5490`). `MainViewFamily` 연결은 이 경로에서만(`SceneCaptureComponent.cpp:588-597`).
- **GT 절감 ≈ 0**(1.3절). 대신:
  - **`FScopedCaptureViewShake`가 깨진다**: 캡쳐 트랜스폼은 렌더 시점에 GT에서 읽히는데(`SceneCaptureRendering.cpp:1180`, `:765`), RAII가 틱 스코프 끝에서 원복(`SceneCaptureViewParity.cpp:165-169`) → 피격 흔들림 소실. 헤더 주석도 동기 호출 전제(`SceneCaptureViewParity.h:118-123`).
  - **RTSP +1프레임 고정**: `RtspStreamComponent` 틱도 PrePhysics(`RtspStreamComponent.cpp:150`)라 `ENQUEUE`가 항상 지연 캡쳐 렌더보다 먼저 들어감. 지금은 순서 비보장이라 같은 프레임/+1이 섞임 [B].
  - `ApplyMainViewReflectionParity`는 무사(PP 값은 틱~렌더 사이 안 바뀜). 텍스처 스트리밍 무관(엔진 캡쳐 경로에 `AddViewInformation` 없음, 프로젝트가 틱마다 수동 등록 `QuadCamComponent.cpp:295-301`). ViewState는 `bAlwaysPersistRenderingState=true`라 어느 쪽이든 유지(`SceneCaptureComponent.cpp:421-430`).
- **결론: 성능 목적으로는 바꿀 이유 없음.**

### 1.8 "`bCaptureEveryFrame` 신뢰성 없음" 주석의 진짜 원인 [A 경로 / B 런타임]

- BeginPlay 중 `RegisterComponent` → `HandleRegisterComponentWithWorld`(`ActorComponent.cpp:2049`) → 소유자가 BeginPlay 중이면 `RegisterAllComponentTickFunctions(true)` + `BeginPlay()`(`Actor.cpp:6429-6450`), 틱은 `bStartWithTickEnabled`로 켜짐(`ActorComponent.cpp:1747`) → 캡쳐 틱이 `bCaptureEveryFrame`이면 `CaptureSceneDeferred()`(`SceneCaptureComponent.cpp:743-746`). **"동적 생성이라 첫 프레임에 언다"는 실패 경로가 엔진에 없다.**
- 프로젝트 자체 후속 주석이 이미 무관한 디버그 드로우 버그를 원인으로 지목(`RCWSComponent.cpp:116-126`, `TargetDetectionComponent.cpp:481-492`의 `#if 0`). `TitanTruck.cpp:181-183`, `DronePawn.cpp:348-350`은 07-23 문구 복사본 → **stale**.
- 지연 캡쳐가 실제로 스킵되는 조건은 숨김/디테일 모드(`:814`), 히트프록시 뷰포트, 메인 뷰가 안 그려지는 프레임뿐.
- 확정 [B]: `ATestSceneCapture`(`TestSceneCapture.cpp:48`)를 New_kadex_0811에 놓고 `bCaptureEveryFrame=true`로 갱신 확인(레벨 수정이라 이번 세션 범위 밖).

---

## 2. Q2 — RTSP 인코드가 렌더 스레드를 막는 구조

### 2.1 경로와 블록 지점 (Windows D3D12)

- GT: `URtspStreamComponent::TickComponent`(`Plugins/RtspEncoder/.../RtspStreamComponent.cpp:150`) → TargetFps 누적기(`:180-188`, 뺄셈식이라 목표 간격보다 프레임이 길면 매 틱 인코드) → `ENQUEUE_RENDER_COMMAND(RtspStreamEncodeFrame)`(`:216`).
- RT 람다(`:216-286`, **이 전체가 stat 스코프**): 중간 RT가 있으면 스트림별 `FRDGBuilder` + `AddDrawTexturePass` + `Execute`(`:236-248`) → `EncoderRef->EncodeFrame`(`:271`) → `PushEncodedFrame`(`RtspServerSubsystem.cpp:467-470` g_malloc+memcpy+appsrc).
- `FNvencD3D12Encoder::EncodeFrame`(`Private/Windows/NvencD3D12Encoder.cpp:145`): Transition(`:217`, `:267`), `EnqueueLambda`로 네이티브 `CopyResource` + `RHISignalManualFence`(`:238-262`) — **기록만, CPU 대기 없음**(엔진 `D3D12CommandContext.cpp:265-273`: 커맨드리스트를 닫고 펜스를 붙일 뿐, 즉시 제출 없음. 부작용으로 스트림마다 커맨드리스트가 쪼개짐 [B 소비용]).
- `NvEncoderD3D12::EncodeFrame`(`ThirdParty/NvCodec/NvEncoder/Windows/NvEncoderD3D12.cpp:352`): `MapResources`(`:362`) → `nvEncEncodePicture`(`NvEncoder.cpp:613`, 제출만; 입력 펜스는 GPU 측 대기 `NvEncoderD3D12.cpp:371-372`) → **`GetEncodedPacket(..., bOutputDelay=true)`(`:379`)**:
  - `:406` `WaitForCompletionEvent` — async 모드에서만 대기(`NvEncoder.cpp:842-853`, `WaitForSingleObject(...,20000)` `:850`). D3D12 경로는 `nvEncRegisterAsyncEvent` 미등록(`NvEncoder.cpp:369`)이라 실질 대기는 아래 Lock일 가능성 [B].
  - **`:409-410` `lockBitstreamData.doNotWait = false; nvEncLockBitstream(...)` — 확정 블록 지점.** 헤더 정의 "If not set, the call will block until operation completes"(`Interface/nvEncodeAPI.h:2675`).
- 버퍼 깊이: `m_nEncoderBuffer = 1 + 0 + nExtraOutputDelay(1) = 2`(`NvEncoder.cpp:346`, `NvencD3D12Encoder.cpp:70,92`), `m_nOutputDelay = 1`(`NvEncoder.cpp:358`), `iEnd = m_iToSend - 1`(`NvEncoderD3D12.cpp:403`) → **매 호출에서 그 스트림이 직전에 제출한 N-1 프레임의 완료를 동기 대기**.
- N-1의 복사+펜스는 그 프레임 GPU 스트림의 씬 렌더 뒤에 있음 → GPU가 1~2프레임 밀린 GPU 바운드 상황이면 Lock 대기 ≈ "GPU가 N-1 복사 지점까지 따라잡는 시간 + NVENC 인코드". S0 M1 12 ms(GPU 바운드) vs M2 2.2 ms(GT 바운드)와 맞는 구조, 인과는 **[B]**.
- 헤더 주석 "no CPU stall"(`NvencD3D12Encoder.h:6-7`, `.cpp:263-265`)은 입력 복사에만 해당 — 출력 Lock은 동기.
- 여러 스트림이면 첫 스트림이 대기를 대부분 떠안고 나머지는 싸다 [B].
- 스케일 복사는 즉시 플러시 안 함 [A]: `GraphBuilder.Execute()`는 기록만, 같은 포맷·크기면 하드웨어 복사(`Renderer/Private/ScreenPass.cpp:336-339`). D3D12 경로에 `ImmediateFlush`/`SubmitAndBlockUntilGPUIdle`/`FlushRHIThread` 없음.

### 2.2 렌더 스레드 → 게임 스레드 전파 [A 구조]

- GT는 매 프레임 `FFrameEndSync::Sync`(`Launch/Private/LaunchEngineLoop.cpp:6084`; 구현 `RenderCore/Private/RenderingThread.cpp:2477-2584`)에서 렌더 스레드 N-1 펜스를 기다림(`r.OneFrameThreadLag=1` 기본 `:2435-2439`, `:2505-2509`). RHI는 `r.GTSyncType=0`으로 최대 2프레임 겹침(`:2536-2543`). 대기 시간은 `STAT_FrameSyncTime`(`:2452`). 프로젝트 ini override 없음.
- M1: RT 23.9(인코드 12 + SceneRenderBuilder 11.35) > GT 작업 ≈20 → GT idle 3.9 ms가 이 동기화.
- **단, GPU가 21.8 ms라 인코드 대기를 없애도 프레임은 GPU 역압으로 비슷할 가능성이 크다 [B]** — 인코드 대기는 "원인"이라기보다 GPU 바운드의 증상이 RT에 드러난 것. 판정은 M-2.

### 2.3 스트림 구성 [A]

| 축 | 스트림 | 해상도 | fps | 소스 / 복사 |
|---|---|---|---|---|
| UGV | CCTV ×4 | 320x180 | 30(`CctvTargetFps`, `StreamResolutionSubsystem.h:69`) | QuadCam RT를 같은 크기로 `ResizeTarget`(`VehicleRtspBridgeComponent.cpp:98-103`) → 중간 RT 없이 CopyResource |
| UGV | RCWS | 1920x1080 | 60(`RcwsTargetFps`, `.h:63`) | 메인 뷰 톤맵 결과 복사(`bStreamRcwsFromMainView=true` `.h:70`, `MainViewFrameSource.cpp`) |
| 자체방호 | env | 640x360 | 30(기본값 `RtspStreamComponent.h:34`) | 중간 RT 스케일 블릿(`RtspStreamComponent.cpp:65-78`) |
| 자체방호 | CCTV ×4 | 320x180 | 30 | 트럭 QuadCam(위젯 크기) → 스케일(`TitanTruck.cpp:264-268`) |
| 자체방호 | uav_gimbal | 640x360 | 30 | 스케일 |
| 자체방호 | rcws | 1280x720 | 30 | 메인 뷰, `bPinViewportResolution=false`(`TitanTruck.cpp:286-289`) |

- M1(≈42 fps)에서 RCWS 매 프레임 + CCTV 각 ≈70% → **3.8~5 인코드/프레임** = S0 "4~5회" 일치.
- CCTV 카메라 내용은 틱/4(≈10 fps)로만 바뀌는데 30 fps로 송출 → **같은 그림을 ≈3번씩 인코드**. 의도된 설계(SDP fps보다 적게 보내면 클라이언트가 멈추는 이력, `StreamResolutionSubsystem.h:51-69`, `RtspStreamComponent.cpp:176-178`).
- 스케일 블릿 GPU 비용은 작음 [B] — `ProfileGPU`의 `RtspScaleSource`/`RtspMainViewDestination`로 확인.

### 2.4 Linux Vulkan 경로 — 같은 구조가 아니라 더 나쁨 [A 구조]

- "~50 ms" 이력(`Public/FNvencVulkanEncoder.h:45-65`): 예전엔 `SubmitAndBlockUntilGPUIdle()`로 GPU 큐 전체 대기 → 복사 전용 펜스로 교체.
- 현재(`Private/FNvencVulkanEncoder.cpp:490-516`): `CopyTexture` → `WriteGPUFence` → **`ImmediateFlush(DispatchToRHIThread)`(`:506`)** → **`CopyFence->Wait()` CPU 대기(`:516`)** → `cuMemcpy2D` 동기(`:553`, 스트림 NULL) → `NvEncoderCuda::EncodeFrame`.
- D3D12와 차이: 스트림마다 RHI 디스패치 강제 / **이번 프레임** 복사를 기다림 → GPU 큐 순서상 앞에 쌓인 작업 전부를 기다림 [B] / `NvEncoderCuda`를 `nExtraOutputDelay` 없이 생성해 기본 3(`FNvencVulkanEncoder.cpp:324`, `NvEncoderCuda.h:64`) → Lock 자체는 오래된 프레임이라 짧음 [B] / 비Windows는 동기 모드(`NvEncoder.cpp:140-145`).
- 리눅스 실측은 없음 → LIG 리눅스 PC 측정이 필요하면 별도(이번 요청 목록 M-6).

### 2.5 클라이언트 없이도 인코드 [A]

- `TickComponent`에 클라이언트 수 게이트 없음(`RtspStreamComponent.cpp:150-286`). `PushEncodedFrame`에서 `AppSrc == nullptr`이면 버림(`RtspServerSubsystem.cpp:455-460`, 주석 "Normal/expected: no RTSP client connected").
- 캡쳐 쪽도 축만 보고 접속 여부는 안 봄(`VehicleRtspBridgeComponent.cpp:32-49`).
- **S0 측정 때 수신 클라이언트가 붙어 있었는지는 기록 없음** — 전시 상태(수신기 연결)와 개발 상태의 비용은 같다(어차피 인코드).

---

## 3. Q3 — 캡쳐 수와 뷰당 비용

### 3.1 축별 활성 캡쳐 [A 소스 · BP 템플릿/CDO 기준, 레벨 인스턴스 override는 B]

레벨(`New_kadex_0811.umap`)엔 BP_UGV_0901·BP_TitanTruck·BP_Drone·MinimapCaptureActor만. `MinimapCaptureActor`는 07-20부터 정적 이미지(캡쳐 없음, `UI/MinimapCaptureActor.h:4-12`). `TargetDetectionComponent`는 캡쳐 트랜스폼·FOV로 UV만 계산(렌더/리드백 0건). UAVPawn·RCWSPreviewActor·TestSceneCapture는 레벨에 없음.

| 축 | 캡쳐 | 해상도 | 주기 | 소비처 |
|---|---|---|---|---|
| UGV 호스트/데모 | UGV QuadCam ×4 | 240x135(템플릿) → **320x180**(브리지 리사이즈) | 틱마다 1대(카메라당 틱/4) | RTSP 4 + UGVTestDashboard(`UGVTestDashboardWidget.cpp:143-148`) |
| | UGV RCWS Sight | 1226x928 | **꺼짐**(`bDisableSightCapture`, `VehicleRtspBridgeComponent.cpp:142-147,213`) — 브리지 배선 전 1~2틱만 돎(`:63`) | — |
| | 트럭·드론 캡쳐 전부 | — | 축 판정 후 꺼짐 | — |
| 자체방호 | 트럭 QuadCam ×4 | 위젯 픽셀 크기(`SelfDefenseMonitor1Widget.cpp:507-520`) | 틱마다 1대 | RTSP 4 + Monitor1 |
| | 트럭 Battlefield | 위젯 크기(`:500`) | 2틱에 1번 | RTSP env + Monitor1 |
| | 드론 Gimbal | 위젯 크기(`:526`) | 2틱에 1번 | RTSP uav_gimbal + Monitor1 |
| | 트럭 RCWS Sight | — | 꺼짐(메인 뷰 경로, `TitanTruck.cpp:282-300`) | — |
| 축 미지정 | 전부 | — | 5초 폴링 후 Unspecified → 전부 꺼짐(`RtspAxisGate.cpp:40-58`) | — |

**RTSP로도 위젯으로도 안 쓰이는데 렌더되는 캡쳐는 정상 상태에서 없다.** 일회성만 있음(BeginPlay 1회, 브리지 배선 전 1~2틱, 축 판정 전 최대 5초).

### 3.2 캡쳐 렌더 설정 — 320x180에도 풀 파이프라인 [A]

- 공통(QuadCam `:137-157`, RCWS `:109-141,329`, 트럭 `:179-187`, 드론 `:346-357`): `SCS_FinalColorLDR`, `bCaptureEveryFrame=false`, `bAlwaysPersistRenderingState=true`, `ShowFlags.TemporalAA=true`, `MotionBlur=false`. 그 외 ShowFlags는 엔진 기본(`ESFIM_Game`에서 MotionBlur·SeparateTranslucency만 빠짐, `SceneCaptureComponent.cpp:169,186-189`) → **그림자(VSM)·Nanite·AO·볼류메트릭 포그·DOF·포스트 전부 켜짐**.
- `LODDistanceFactor`·`MaxViewDistanceOverride`·`PrimitiveRenderMode`·`HiddenActors/ShowOnly` 프로젝트 설정 0건 → LOD 1.0, 원거리 클립 없음(`SceneCaptureComponent.cpp:179-180`).
- **Lumen GI를 parity 코드가 캡쳐마다 강제로 켠다**: `ApplyMainViewReflectionParity`가 PP override가 없으면 `r.DynamicGlobalIlluminationMethod`(=1 Lumen, `DefaultEngine.ini:96`)와 `r.ReflectionMethod`(→SSR로 클램프)를 PP에 박음(`Plugins/QuadCamModule/.../SceneCaptureViewParity.cpp:80-109`). UGV FrontCineCamera PP의 `bOverride_*`는 전부 false(MCP 읽기 확인).
- **캡쳐 뷰마다 뷰 전용 Lumen 씬 데이터**: ViewState 유지 + Lumen이면 `AddLumenSceneData`(`SceneCaptureRendering.cpp:1470-1478`) → `bViewSpecific=true` `FLumenSceneData` 신규 할당, `Scene->PerViewOrGPULumenSceneData`에 등록(`SceneViewState.cpp:525-553`). UGV 호스트 기준 표면 캐시 6벌(CCTV 4 + RCWS 초기 1 + 메인). 갱신 비용·메모리 [B].
- 화면 비율 100%(`bMainViewResolution=false` → `FLegacyScreenPercentageDriver(1.0)`, `SceneCaptureRendering.cpp:951-955`) + TSR(`r.AntiAliasingMethod=4`, `DefaultEngine.ini:46`). 지터 끔은 Battlefield만(`TitanTruck.cpp:38-68,189`).

### 3.3 320x180 캡쳐 한 장의 GPU 해부 [A 로그]

S0 `ProfileGPU` Frame 4869, `WorldTick → BP_UGV_0901 → SceneRender` 3.711 ms(로그 27674행~):

| 패스 | ms | 해상도 비례? |
|---|---|---|
| ShadowDepths(VSM Nanite 0.857 · Non-Nanite 0.085 · BuildPageAllocation 0.124) | **1.079** | 거의 무관 |
| VirtualShadowMapMarkPages(PageTable 8192x384 클리어 0.191 포함) | 0.290 | 무관 |
| Nanite::VisBuffer(MainPass 0.271 · PostPass 0.177) | 0.490 | 대부분 무관(컬링) |
| GlobalDistanceFieldUpdate(Update Movable) | 0.270 | 무관 |
| PostProcessing(TSR 0.097 · DOF 0.056 …) | 0.282 | 비례 |
| RenderDeferredLighting | 0.169 | 비례 |
| BasePass | 0.164 | 비례 |
| LumenSceneUpdate(카드 캡쳐 4) · Lumen 프로브 | ≈0.1~0.2 | 무관 |
| VolumetricFog | 0.072 | 무관(그리드) |

→ **해상도 무관 고정비 ≈ 2.3~2.5 ms / 캡쳐**. 메인 뷰(1400x788, 17.4 ms)의 픽셀 비례로 치면 캡쳐는 ≈0.9 ms여야 함. S0 "캡쳐 뷰당 ≈3.7 ms × 활성 수"는 해상도를 줄여서는 안 줄어든다는 뜻.
- 이 프레임엔 캡쳐 1개(UGV 호스트 1회/프레임과 일치).

### 3.4 매 틱 PostProcessSettings 복사 — 렌더 상태 재생성 없음 [A]

- `USceneCaptureComponent2D`는 프리미티브가 아니고 `PostProcessSettings`는 평범한 UPROPERTY → 대입해도 `MarkRenderStateDirty` 없음, 렌더 시 포인터로 읽힘(`SceneCaptureRendering.cpp:1455-1456`).
- ViewState 파괴 경로는 `!bCaptureEveryFrame && !bAlwaysPersistRenderingState`뿐(`SceneCaptureComponent.cpp:427-430`) — 전부 true라 해당 없음. Lumen 품질 변경(`ChangeLumenSceneDataQuality`)은 SurfaceCacheResolution 변화 때만(항상 0.5).
- 비용: UGV QuadCam 4개 `SyncLens`(`QuadCamComponent.cpp:98-125`, 게이트 전 매 틱) + RCWS 2회(`RCWSComponent.cpp:284,326`) 구조체 복사 — µs급 [B]. S0 QuadCam 틱 3.6 ms의 비Post-Tick 몫은 0.17 ms(3.588-3.418)라 상한도 그 이하.

### 3.5 `UTitanGraphicsSettings` 캡쳐 주기 3종 [A]

`UTitanGraphicsSubsystem::HandleWorldBeginPlay`(`Settings/TitanGraphicsSubsystem.cpp:64-68`) → `ApplyCaptureRates`(`Settings/TitanGraphicsSettings.cpp:314-367`), 값은 ini 사용자값 우선 → CDO, 1~8 클램프(`:131-150`). 현재 ini 키 없음 → C++ 기본 1/2/2(S0 로그 2591행 "QuadCam 2개=x1, Drone 1개=2틱, Truck 1개=2틱" 일치).

| 설정 | 대상 |
|---|---|
| `CctvCaptureEveryNTicks` | 모든 `QuadCam->CaptureEveryNTicks`(`:340-344`) → `QuadCamComponent.cpp:273-277` |
| `DroneGimbalCaptureEveryNTicks` | `ADronePawn::GimbalRoundRobinCount`, Slot 1(`:346-352`) |
| `BattlefieldCaptureEveryNTicks` | `ATitanTruck::BattlefieldRoundRobinCount`, Slot 0(`:354-359`) |

RCWS `CaptureRoundRobinCount`는 대상 아님(두 축 모두 꺼져 있어 현재 무영향). 월드 BeginPlay 이후 스폰된 드론/트럭은 미적용 [B](기본값이 같아 현재 무해).

---

## 4. Q4 — 메인 뷰 · 듀얼 모니터 [A]

- 가상 데스크톱 스팬(보더리스 2배 폭) 방식 **폐기** — 호출 주석 처리(`titan_exampleViewportClient.cpp:15-22`). 로컬 플레이어 1명 → **3D 뷰 패밀리 1개**.
- UGV 축: `UMainViewStreamComponent`가 `SetFixedViewportSize(1920,1080)`로 고정(`MainViewStreamComponent.cpp:80,95`, `bPinViewportResolution=true` `.h:79`, 별도 RT는 `titan_exampleGameEngine` `DefaultEngine.ini:204`). → **S0의 "메인 뷰 1080p 한 개"는 설계대로**(듀얼 모니터와 무관). TSR 입력 1400x788 = 화면비율 ≈73%.
  - ⚠ 메모리 노트 "RTSP 고정 해상도: SetFixedViewportSize 쓰지 말 것"과 충돌해 보임 — 범위 밖, 기록만.
- 자체방호 축: 3D는 메인 창 1개(창 크기 추종, `TitanTruck.cpp:286-289`). Monitor1은 게임 뷰포트 없는 Slate 전용 두 번째 SWindow(`titan_examplePlayerController.cpp:293-341`) → 추가 씬 렌더 없음.
- **RCWS 이중 렌더(메인 뷰 + 조준경 캡쳐) 제거 상태** — 메인 뷰 RT 생성 실패 시에만 폴백(`VehicleRtspBridgeComponent.cpp:157-162`, `TitanTruck.cpp:303-309`, Warning 로그).
- `FMainViewFrameSource`는 추가 씬 렌더 없음. 다만 `VisualizeDepthOfField` 포스트 패스를 구독해서 톤매퍼가 체인 끝이 아니게 되고 **1080p 풀스크린 블릿 2회/프레임**(RTSP RT `MainViewFrameSource.cpp:129`, 백버퍼 OverrideOutput `:150-153`) + 중간 텍스처 쓰기(`:135-137`). 비용 0.1 ms급 [B].

---

## 5. 수정 후보 (제안만 — 이 세션은 수정하지 않음)

| # | 후보 | 예상 효과 | 위험 |
|---|---|---|---|
| F1 | **CCTV급 캡쳐의 고정비 깎기**: parity의 Lumen GI 강제를 소형 캡쳐에선 끄거나(GI None/SSGI), ShowFlags로 DynamicShadows/VolumetricFog/DOF 끄기, `MaxViewDistanceOverride`·`LODDistanceFactor` 적용 | 캡쳐당 GPU 최대 ≈2 ms(VSM 1.37 + Lumen/GDF 일부). UGV 호스트는 프레임마다 캡쳐 1장이라 **GPU 바운드인 M1에 직결** | 08-20 parity 작업(메인 뷰와 같은 그림)을 되돌리는 방향 — 사용자 판단 필요. 그림자 없는 CCTV는 티가 남 → 먼저 A/B 스크린샷 |
| F2 | **NVENC 출력 Lock 비동기화**: Lock을 별도 스레드로 옮기거나 `nExtraOutputDelay` 늘려 N-2를 Lock | RT의 인코드 대기(M1 12 ms, max 18.6) 제거 | 지연 +1프레임(≈16~25 ms). GPU 바운드라 **프레임 시간은 거의 안 줄 수 있음**[B] — M-2로 먼저 판정 |
| F3 | 클라이언트 없으면 인코드 스킵 + 접속 시 IDR(`ConsumeNewConnectionFlag` → `bForceKeyframe` 경로 재사용) | 수신기 없는 스트림의 RT 대기·NVENC 부하 0 | 전시 중엔 수신기가 붙어 있으므로 **전시 효과는 ≈0**, 개발/테스트 효과만. `bWaitingForRequestedKeyframe` 재검증 |
| F4 | ISKM 바운드 이중 계산 제거(정적 숲이면 틱 끄기/AnimationBounds 재검토) | GT ≈1.6 ms(+ 에디터에선 플러시 직렬분) | **S2 몫**, 숲 애니 바운드 컬링 부작용 |
| F5 | `CaptureSceneDeferred()` 전환 | GT ≈0(비용 이동일 뿐) | 뷰 셰이크 소실 + RTSP +1프레임 — **비권장** |
| F6 | stale 주석 정리(`TitanTruck.cpp:181-183`, `DronePawn.cpp:348-350`) | 오해 방지 | 없음 |
| F7 | CCTV 인코드 중복(내용 10fps, 송출 30fps) 줄이기 | 인코드 횟수 ≈2/3 감소 | SDP fps 미달 시 클라이언트 멈춤 이력 — 비트스트림 재전송 등 별도 설계 필요 |

**우선순위 판단**: M1은 GPU 바운드 → F1이 프레임에 가장 직접적. F2는 M-2 결과(GPU 부하를 줄이면 인코드 대기도 주는가)를 보고 결정. 플러시는 S1 몫이 아님(F4 → S2).

---

## 6. 측정 요청 (사용자 실행, 결과는 `Saved/Logs/*.log` 파싱)

조건은 S0와 동일(`UnrealEditor.exe titan_example.uproject -game -log -noailogging`, `New_kadex_0811?Listen?Axis=UGV?Demo=1`, M1 숲 정면·교전 전 같은 시점).

| ID | 목적 | 명령(순서대로) |
|---|---|---|
| M-1 | 패키지 모드 플러시 비용(1.4절) | `stat dumpframe -ms=0.05` → `AllowAsyncRenderThreadUpdatesEditorGameWorld 1` → 3초 뒤 `stat dumpframe -ms=0.05` → `stat dumpave -num=120 -ms=0.1` → `AllowAsyncRenderThreadUpdatesEditorGameWorld 0` |
| M-2 | 인코드 = GPU 대기 가설(2.1·2.2절) | `stat dumpave -num=120 -ms=0.1` → `r.ScreenPercentage 50` → 3초 뒤 `stat dumpave -num=120 -ms=0.1` → `r.ScreenPercentage 100`. `RtspStreamEncodeFrame`과 `Frame Sync Time`이 같이 줄면 확정 |
| M-3 | 수신기 유무(2.5절) | 수신 클라이언트(VLC 등) 5스트림 연결 상태/미연결 상태 각각 `stat dumpave -num=120 -ms=0.1` |
| M-4 | 뷰당 Lumen 메모리 | `r.SceneCapture.DumpMemory`(WITH_EDITOR 전용, 비쿡 `-game`에서 동작) |
| M-5 | 자체방호 축 캡쳐 실제 크기·비용 | 2-PC 자체방호 클라이언트에서 `ProfileGPU` 1회(Battlefield/Gimbal이 도는 프레임이 걸리게 2번) |
| M-6 (선택) | 인코드 내부 분해 | `-trace=cpu,gpu,frame`으로 Unreal Insights 10초 — `RtspStreamEncodeFrame` 안 스트림별 분포 |

코드 수정이 필요한 실험(제안만): `nvEncLockBitstream`·`nvEncEncodePicture` 앞뒤 `TRACE_CPUPROFILER_EVENT_SCOPE`; CineCamera PP에 `bOverride_DynamicGlobalIlluminationMethod=None` 넣어 캡쳐 1개만 Lumen A/B; 캡쳐 ShowFlags `DynamicShadows=0` A/B.

---

## 7. 범위 밖 발견 (다른 세션)

- **S2**: ISKM 틱+플러시 바운드 이중 계산(1.5절). 숲 ISKM 16개의 GT 비용은 에디터 바이너리에서 직렬로 과대 측정됨(1.4절) — S2도 M-1 결과를 기준으로 삼을 것.
- **S3/S4**: 없음. Niagara 대기(1.1절 1)는 교전 중 QuadCam 플러시에 섞일 수 있음 — M2 시점 dumpframe으로 확인 가능.
- UGV 메인 뷰 `SetFixedViewportSize` 사용이 메모리 노트와 충돌해 보임(4절) — RTSP 담당 확인.
