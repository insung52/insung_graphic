# UGV 자율주행 예상 경로 오버레이 (대시보드 전용, RTSP 미송출)

2026-10-15 / 완료(1차 구현 + 경계 AA 시간 누적, 사용자 확인) / 찢어지던 Niagara 리본 대신, RTSP 메인뷰 복사 **뒤**에 붙는 렌더러 확장(컴퓨트 2패스 + 마스크 시간 누적)이 깊이 기반으로 지형 위에 예상 주행 곡선을 그린다 — 경로는 이동 명령당 1회 예측해 고정, 모양은 디자이너용 `UUGVPathStyleAsset`.

> **읽는 순서**: §1 요구 → §2 왜 이 구조인가 → §3 지금 어떻게 동작하나 → §4 디자이너 가이드 → §5 cvar →
> §6 트러블슈팅/교훈(개정 이력 요약 포함) → §7 변경 파일 → §8 남은 한계.
> 같은 날 1~6차 개정을 거쳤다. 최종 단계(마스크 시간 누적)에 대한 사용자 평가: "완벽하지는 않은데 어느정도 해결, 수렴되고 있음".

---

## 1. 요구사항 (사용자, 2026-10-15)

기존 경로 미리보기 = `BP_UGV` 계열의 Niagara 리본(`PathPreviewFX`). **두 번째 자율 이동 명령부터 찢어져서** 꺼 둔 상태였다.

| # | 요구 | 구현 |
|---|---|---|
| 1 | **UGV 테스트 대시보드에서만** 보일 것 | 대시보드가 실제로 생성됐을 때만(§3-1). 대시보드 on/off 체크박스는 다른 세션 작업 — `ui/2026-10-15_axis_selection_ugv_dashboard_toggle_and_saved_choices.md` |
| 2 | RTSP·씬 캡처 어디에도 안 나갈 것 | RTSP 메인뷰 복사 **이후**에 그림(§2) |
| 3 | 트럭 RCWS 메인 뷰에는 안 보일 것 | 로컬 PC 뷰 타깃이 UGV일 때만 |
| 4 | 지형 위에, 깊이감 있게(가릴 건 가리게) | 픽셀마다 씬 깊이 → 월드 좌표 복원, 지면 픽셀만 칠함 |
| 5 | 두 번째 명령에서도 찢어지지 않을 것 | 렌더러에 상태 없음 — `SetPath`마다 버퍼 통째 교체 |
| 6 | 실제 동적 주행처럼 부드러운 곡선 | 내비 폴리라인이 아니라 순수추종 전방 시뮬 결과(§3-2) |

## 2. 왜 SceneViewExtension인가 (메시/데칼/PP 머티리얼이 안 되는 이유)

UGV RCWS RTSP는 씬 캡처가 아니라 **메인 뷰포트 렌더 결과의 복사본**이다(`bStreamRcwsFromMainView=true`,
`RtspEncoder` `FMainViewFrameSource`). 복사 지점은 `EPostProcessingPass::VisualizeDepthOfField` after-pass —
2026-08-21 EO/IR(After Tonemapping PP 머티리얼)이 스트림에서 빠지던 버그 때문에 Tonemap에서 옮긴 지점이고
(`Plugins/RtspEncoder/.../MainViewFrameSource.cpp:52-75` 주석), **After-Tonemapping PP 체인 뒤**다
(UE 5.8 `PostProcessing.cpp`: `:1637` Tonemap after-pass → `:1676` `BL_SceneColorAfterTonemapping` → `:1707` VisualizeDepthOfField after-pass).

따라서 화면에 그려지는 것은 사실상 전부 RTSP에도 실린다:

| 방식 | 결과 |
|---|---|
| Niagara 리본 / 스플라인 메시 / 데칼 / 반투명 메시 | 메인 뷰에 그려짐 → 복사본에 포함 |
| `bHiddenInSceneCapture` | 씬 캡처 전용 플래그 — 메인뷰 복사엔 무의미 |
| `OnlyOwnerSee` | 같은 PC의 메인 뷰 = 소유자 뷰 → 복사됨 |
| PP 머티리얼(After Tonemapping 포함) | 복사 지점보다 앞 → 복사됨 |

피하려면 RTSP를 메인뷰 복사 대신 별도 씬 캡처로 돌려야 하는데, 그건 UGV 시점을 **두 번 렌더**하는 것이라 기각.
**TSR도 같은 이유로 오버레이에 못 쓴다** — TSR은 톤매핑 이전이라 거기서 그리면 RTSP에 샌다. 그래서 TSR의 원리(지터
프레임 누적)만 마스크에 직접 구현했다(§3-3).

**해법 — 같은 after-pass에 더 낮은 우선순위로 붙기.**

- `SceneViewExtension.cpp:116` — 뷰 익스텐션은 `GetPriority()` **내림차순** 정렬. `PostProcessing.cpp` `AddAfterPass`가 그 순서로
  콜백을 체인하고 **마지막 콜백이 `OverrideOutput`**(최종 타깃)을 받는다.
- 오버레이 **-100**, `FMainViewFrameSource` **0** → RTSP 복사 **뒤**·Slate UI **앞**(`UGVPathOverlay.cpp:145`, 구독 `:151-161`).
- 씬 캡처 뷰패밀리는 `Context.Viewport == nullptr`이라 `IsActiveThisFrame_Internal`에서 자동 제외(`:214-219`, 대상 뷰포트와
  같을 때만). 렌더 스레드에서도 `bIsSceneCapture`/`bIsReflectionCapture`/`bIsPlanarReflection`이면 통과(`:300-312`).
- 그릴 게 없으면 `OverrideOutput`에 복사만 하고 넘긴다(`OpenColorIODisplayExtension.cpp:82`, `TitanFlickerViewExtension`과 같은 규칙).

## 3. 현재 동작 (파이프라인)

```
UUGVTestDashboardWidget  NativeConstruct/Destruct → Register/UnregisterDashboard
UUGVPathPreviewSubsystem (UTickableWorldSubsystem, Game/PIE 월드)
   ├─ 표시 조건 판정 (§3-1)
   ├─ 이동 명령(PathRevision)당 1회 AUGVAIController::BuildPredictedDrivePath → SampleGround → 고정 (§3-2)
   ├─ 매 프레임 차량을 고정 경로에 투영 → SetVehicleArcLength, 필요 시 rebase
   ├─ PathPreviewStyle(UUGVPathStyleAsset) → SetStyle, 패턴 머티리얼 → 렌더 타깃 → SetPatternTexture (§4)
   └─ IUGVPathOverlay (SetTargetViewport / SetPath / SetVehicleArcLength / SetStyle / SetPatternTexture / SetVisible)
Plugins/UGVPathOverlay (FSceneViewExtensionBase, priority -100)
   └─ VisualizeDepthOfField after-pass:
        FieldCS(깊이 해상도) → CompositeCS(출력 해상도, 지터 보정 + 시간 누적) → 원래 포맷 타깃에 복사 (§3-3)
```

### 3-1. 게임 쪽 `UUGVPathPreviewSubsystem` (`Source/titan_example/UI/UGVPathPreviewSubsystem.cpp`)

**표시 조건(전부 만족)** — `Tick` `:96-165`

| # | 조건 | 비고 |
|---|---|---|
| 1 | 이 프로세스에 `UUGVTestDashboardWidget`이 떠 있음 | `bCreateUGVTestDashboard`는 직접 안 봄 — 축 선택 체크박스가 실행마다 덮어쓰므로 "실제로 떴는가"가 유일하게 믿을 신호 |
| 2 | `AUGVAIController::IsMoving()` | 아니면 숨기고 경로 캐시 리셋(도착/실패/시나리오 리셋 즉시 사라짐) |
| 3 | 대시보드 소유 PC의 뷰 타깃 == UGV | 트럭 RCWS 화면이면 숨기되 경로 캐시는 유지(돌아오면 이어서) |
| 4 | `titan.UGVPath.Enable != 0` | |

클라이언트(자체방호축)에는 대시보드도 AI 컨트롤러도 없음 → 리플리케이션 불필요.

**경로 고정 + 차량 투영**

- `PathController`/`PathRevision`이 컨트롤러의 `GetPathRevision()`과 다르면 = 새 이동 명령 → `RebuildPath(…, 0)`.
  첫 점 s = 0(출발점에 고정). `PathRevision`은 `MoveToDestinationInternal`이 경로를 세울 때마다 ++(`UGVAIController.cpp:621`).
- 매 프레임 `ProjectVehicle`(`:339-381`): 차량 XY를 고정 경로에 투영해 차량 s를 구함. 직전 구간 근처 창(−4..+64)부터 찾고
  (U턴 오접착 방지), `RebaseDistanceCm`보다 멀면 전체 검색 폴백. 렌더러는 **차량 s + `StartOffsetCm` 이전**을 숨긴다.
- **재예측(rebase)** `:150-157` — ① 선에서 `RebaseDistanceCm`(기본 75) 넘게 벗어났거나 ② `MaxLength`로 잘린 예측의 끝이
  `MaxLength`의 절반 이내로 다가왔을 때만, **최소 0.5 s 간격**(`MinRebuildIntervalSeconds`). 새 경로의 첫 s = 차량의 현재 s →
  패턴이 땅 위 제자리에 이어진다.
- ⚠️ 누적 거리 s는 서브시스템(`RebuildPath` `:214-226`)과 렌더러(`UGVPathOverlay.cpp` `PushToRenderThread` `:240-249`)가
  **같은 식(3D 거리)**으로 계산한다. 한쪽만 바꾸면 "지나온 구간 숨김" 경계가 차량과 어긋난다.

**지면 샘플링(콜리전 높이)** — `SampleGround` `:235-337`. 경로를 만들 때만(명령당 1회 + rebase).

- 목적: 콜리전 없는 풀/고사리를 띠에서 빼기(사용자 아이디어 — 바퀴가 무시하고 지나가는 것 = 콜리전 없는 것).
  픽셀엔 콜리전 정보가 없으니 **"바퀴가 밟는 면의 높이"를 바퀴와 같은 조건의 트레이스로 재고**, 그보다 솟은 픽셀을 뺀다.
- 트레이스 조건 = `ChaosWheeledVehicleMovementComponent.cpp:2902`와 동일: `ECC_WorldDynamic` + `WheelTraceCollisionResponses`
  (UGV0901은 Pawn/PhysicsBody 무시, 엔진 기본 Vehicle 무시) + 복합 트레이스, UGV 자신 제외. 시작 = 예측 Z +200 cm, 끝 −400 cm
  (위에서 길게 쏘면 나무 수관에 걸림).
- 점마다 띠 가로 5곳(−1, −0.5, 0, +0.5, +1 × 반폭) → 중앙 높이로 점 Z 교체 + 가로 오프셋 4개(`FUGVPathGroundSample.LateralOffsetsCm`).
- 지형은 울퉁불퉁한 곡면(사용자 지적) → 이웃 점 사이 중간점 3곳(좌·중·우)도 실측, 선형 보간과의 차이를 구간 오차로,
  점별 `ExtraToleranceCm` = 양옆 구간 오차 최대. 평지는 기본 허용치로 빡빡하게, 굴곡 구간만 여유.
- 비용 로그: `[UGVPathPreview] ground sampled: N points, M traces, X ms`(점당 약 8 트레이스).

### 3-2. 컨트롤러 예측 `AUGVAIController::BuildPredictedDrivePath` (`Vehicles/UGVAIController.cpp:918-1033`, 선언 `.h:303`, `GetPathRevision()` `.h:307`)

내비 경로(`CurrentPath`) 자체가 아니라 **차량이 실제로 그릴 곡선**을 예측한다.

- 시작 = 현재 차량 위치/Yaw. 실제 주행과 **같은 `FindChaosLookaheadTarget`**으로 lookahead 점을 잡고, 순수추종 곡률
  **κ = 2·sin α / L_d**(`UpdateChaosPursuit`의 요레이트 제어와 같은 식)를 25 cm 스텝으로 적분 → 코너를 실제처럼 안쪽으로 자름.
- |α| ≥ `ThrottleCutoffAngleDeg`이면 실제로 스로틀 0 = 제자리선회 → 전진 없이 헤딩만 컷오프 직전까지 돌림(반복 상한 +64).
- 도착 반경 안에서 멈추면 선을 목적지까지 닫음. 출력 간격 = `titan.UGVPath.OutputSpacingCm`.
- **Z = 지면 프로파일 + 출발 시 지면 위 높이**(`:961-1004`). 지면 프로파일(`RebuildGroundProfileIfNeeded` `:870-916`)은 경로당 1회,
  200 cm(`GroundProfileSpacingCm`) 간격으로 `ProjectPointToNavigation`(extent 50×50×1000) — 레캐스트 점은 코너에만 있어 선형보간 Z는
  언덕에서 수 m씩 어긋나기 때문. 매 스텝 Z를 지면에 붙이는 건 오르막 예측 끊김 수정(§6).
- 목적지·길이 상한 어느 쪽에도 못 닿고 끝나면 `[UGVAIController] BuildPredictedDrivePath ended early …` Warning(`:1016-1020`).
- 출력 점의 Z는 내비 지면 Z — 서브시스템의 `SampleGround`가 콜리전 높이로 다시 덮어쓴다.

### 3-3. 렌더러 (`Plugins/UGVPathOverlay`)

- `LoadingPhase: PostConfigInit` — 셰이더 디렉터리 매핑(`/Plugin/UGVPathOverlay`)을 셰이더 컴파일 초기화 전에(TitanFlickerMeter와 같은 이유).
- 공개 인터페이스 `IUGVPathOverlay`(`Public/UGVPathOverlay.h`) — `Create()`가 뷰 익스텐션을 만들어 등록. 게임 모듈은 렌더러 내부 헤더를 몰라도 됨.
- 게임 → 렌더 스레드: `ENQUEUE_RENDER_COMMAND`로 페이로드(점, 누적 s, 청크 AABB, 지면 샘플, 스타일)를 **통째로 교체**(`PushToRenderThread` `:233-282`).
  점 상한 1024(넘치면 차량 가까운 앞쪽만). 청크 = 16구간 단위 AABB(반폭/Z허용치/가로 지면 높낮이만큼 확장).
- 대규모 월드 좌표: C++에서 `PreViewTranslation`을 더해 Translated World(float)로 구조화 버퍼 업로드.
- 색은 **톤매핑 이후 디스플레이 값** — 노출/EV 무관(디버그 드로우 같은 노출 역보정 불필요). 앞을 가리는 나무/병사/차량은
  깊이가 그 물체라 자동으로 가려진다. GPU 스탯 이름 `UGVPathOverlay`.

**① FieldCS — 깊이 해상도**(`UGVPathOverlay.usf:126-248`, 디스패치 = 깊이 텍스처 크기, 뷰 사각형 밖 0. 렌더 해상도
`FViewInfo::ViewRect`는 렌더러 private라 셰이더가 `View.ViewRectMinAndSize`로 거른다)

1. 씬 깊이 → `ScreenToTranslatedWorld`로 P 복원. reversed-Z 0 = 하늘 통과. 뷰 사각형은 half 정밀도 `ViewRectMin` 대신 정수 `ViewRectMinAndSize`.
2. 청크 AABB 컬링 → 통과한 구간 중 **XY 거리 최소** + |P.z − 지면Z| ≤ `ZToleranceCm` + ExtraTol(거친 1차 필터: 수관·차체 윗면 제외).
   지면Z = 경로 중앙 Z + 가로 5샘플 선형 보간 오프셋.
3. 마스크 곱: 띠 몸통(반폭, 깊이 픽셀 월드 크기로 AA) × 길이 방향(차량 s + StartOffset 이전 숨김 → FadeIn, 끝 FadeOut) ×
   차량 앞 거리별 불투명도 곡선(64샘플 표) × `Opacity` × **위 향한 면**(이웃 깊이 픽셀로 면 노멀, |N.z| ≥ `MinUpNormalZ`) ×
   **높이 마스크**(`Tol = HeightToleranceCm + 카메라거리 × HeightToleranceGrowthCmPerM + ExtraTol`, 위로 Tol·아래로 2·Tol 넘으면 0).
4. 출력 `PF_A32B32G32R32F`: (가중치, s, 가로 위치, 깊이 픽셀 월드 크기). 32F인 이유: s가 수만 cm.

**② CompositeCS — 출력 해상도**(`.usf:254-416`)

1. **TSR 지터 보정**: 필드는 투영이 서브픽셀 지터된 프레임의 깊이, 화면 색은 지터 없는 TSR 출력 → 필드를 읽는 위치에
   `View.TemporalAAJitter`(NDC)를 픽셀로 환산해 더함: x + J.x·W/2, y − J.y·H/2(`:265-271`).
2. 필드 bilinear 4탭 — 가중치는 그대로 보간, s/가로 위치/픽셀 크기는 가중치 가중 평균(띠 밖 탭이 안 섞이게).
3. **마스크 시간 누적**(`:294-342`): 재투영은 엔진 방식 그대로(`DBufferNormalReprojection.ush`) — 속도 버퍼 x>0(움직이는 물체)이면
   디코드한 속도, 아니면 깊이 + `View.ClipToPrevClip`(지터 없는 행렬). 히스토리를 **현재 필드 3×3 min/max로 클램프** 후
   `lerp(History, Current, TemporalAlpha)`. 이번 프레임 판정이 0인데 누적값이 남은 픽셀은 3×3 필드에서 s/가로 위치를 가져와 패턴 좌표로 씀.
   히스토리 = 출력 해상도 `PF_R16F` 풀링 RT 1장(대상 뷰포트가 하나라 뷰별 상태 불필요).
4. **히스토리 리셋**(`UGVPathOverlay.cpp:425-430`): `titan.UGVPath.Temporal` 0, 해상도 변경, `View.bCameraCut`, 프레임 번호 끊김(오버레이를
   안 그린 프레임이 있었음), 새 이동 명령(첫 점 s = 0인 새 경로 세대).
5. 합성: 패턴 텍스처가 있으면 `Scene·(1 − op·w) + rgb·w`(밉은 출력 픽셀당 텍셀 수로 선택, 멀리서 지글거림 방지),
   없으면 내장 패턴(채움 + 테두리 + 흐르는 셰브론, 띠 안쪽 62%).

**③ 복사** — 합성은 뷰 사각형 크기 `PF_FloatRGBA` 임시 텍스처(SceneColor에 UAV 플래그가 없을 수 있고, A2B10G10R10은 특히 Vulkan에서
스토리지 이미지 지원 미보장) → `AddDrawTexturePass`로 원래 포맷 타깃(`OverrideOutput` 또는 새 타깃)에 복사(`:484-492`).

**패턴 머티리얼 경로**(서브시스템 `UpdatePattern` `:383-421`): 머티리얼을 매 프레임 transient `UTextureRenderTarget2D`(RGBA16f,
2의 거듭제곱 크기, 자동 밉, (0,0,0,1) 클리어)에 `DrawMaterialToRenderTarget`. Substrate 반투명 블렌드(색 One/InvSrcAlpha,
알파 Zero/InvSrcAlpha — `TranslucentRendering.cpp` `GetTranslucentBlendState`) 때문에 결과는 **RGB = 프리멀티플라이드 색,
A = 투과율(1 − 불투명도)** — 셰이더가 이 형식 그대로 합성한다. 같은 오버레이 패스라 RTSP/씬 캡처엔 여전히 안 나간다.

## 4. 디자이너 가이드 — `UUGVPathStyleAsset` (`Source/titan_example/UI/UGVPathStyleAsset.h`)

- 만들기: 콘텐츠 브라우저 우클릭 > Miscellaneous > Data Asset > `UGVPathStyleAsset`.
- 지정: `WBP_UGVTestDashboard` 기본값의 **"Path Preview Style"**(`UUGVTestDashboardWidget::PathPreviewStyle`). **비우면 내장 패턴.**
- PIE 중 값 수정 → 즉시 반영(`PostEditChangeProperty` → `EditRevision`). 곡선 에셋 자체 수정은 에디터 빌드에서 1초 주기 재읽기로 반영.
- **실제 세팅된 에셋(2026-10-15, 사용자 생성 + MCP 세팅)** — `/Game/Vehicles/UGV/Effects/PathPreview/`:
  `WBP_UGVTestDashboard`(PathPreviewStyle) → `DA_UGVPathStyle`(PatternMaterial, OpacityByDistanceAhead) → `MI_UGVPathPattern`
  → `M_UGVPathPattern` + 마스크 텍스처 `T_UGVPathArrow_Mask`(512×128, sRGB 끔, TC_Masks, X Wrap; R = 바탕, G = 화살표, B = 가장자리 선).
  머티리얼: 흐르는 UV(U − Time×`FlowSpeed`, V) → `MaskTexture`(SamplerType Masks, Wrap) → 색 = 바탕→(B)가장자리→(G)화살표 순 Lerp,
  각 `색×밝기`; 투명도 = `BandOpacity×R` →(B) `EdgeOpacity` →(G) `ArrowOpacity`. 파라미터 그룹 Band/Edge/Arrow/Animation/Texture.
  색 값은 화면값(sRGB 수치/255). 밝기 > 1은 흰색에서 잘릴 뿐 번지지 않음(오버레이가 Bloom 이후라서 — 글로우는 미구현 후속 후보).
  `C_UGVPathOpacityAhead` 키는 MCP로 못 써서 비어 있음(비면 무시). 마스크 PNG 생성 스크립트는 세션 스크래치에만 있음(원본 보관 위치 미정).

| 분류 | 필드 | 기본값 | 의미 |
|---|---|---|---|
| Pattern | `PatternMaterial` | 없음 | 비우면 내장 패턴 |
| | `PatternResolution` | 512×128 | 패턴 RT 크기(X=진행, Y=좌우), 2의 거듭제곱으로 올림 |
| | `TileLengthCm` | 450 | 패턴 U 0..1 한 칸의 경로 길이(내장 패턴은 화살표 간격) |
| Band | `HalfWidthCm` | 90 | 띠 반폭 |
| | `Opacity` | 1 | 전체 불투명도 배율 |
| | `OpacityByDistanceAhead` | 없음 | 곡선 X = 차량 앞 m, Y = 배율(0..1). 비우면 1 |
| | `StartOffsetCm` / `FadeInCm` / `FadeOutCm` | 400 / 300 / 150 | 차체 위 방지 시작 거리, 페이드인, 목적지 앞 페이드아웃 |
| Ground | `ZToleranceCm` | 120 | 경로 높이와 픽셀 높이의 거친 허용 차(1차 필터) |
| | `MinUpNormalZ` | 0.55 | 이보다 위를 향한 면에만(나무 줄기·차체 옆면 제외) |
| | `HeightToleranceCm` | 15 | 콜리전 지면보다 이만큼 넘게 솟은 픽셀은 제외(풀 거르기). 작으면 지면 띠에 구멍 |
| | `HeightToleranceGrowthCmPerM` | 0.2 | 카메라에서 1 m 멀어질 때마다 허용치 +cm(먼 지형 LOD 오차) |
| Prediction | `MaxLengthM` | 300 | 차량 앞 몇 m까지 |
| | `RebaseDistanceCm` | 75 | 차량이 선에서 이만큼 벗어나면 재예측 |
| Built-in Pattern | `FillColor` / `EdgeColor` / `ArrowColor` | (0.10,0.85,1.00,0.28) / (0.35,0.95,1.00,0.90) / (0.85,1.00,1.00,0.70) | 화면 표시값, A = 불투명도 |
| | `EdgeWidthCm` / `ArrowLengthCm` / `ArrowSpeedCmPerSec` | 12 / 110 / 350 | 테두리 두께, 셰브론 길이, 흐르는 속도 |

**패턴 머티리얼 규칙**(헤더 주석과 동일)

- Material Domain = Surface, Blend Mode = Translucent, Shading Model = Unlit, Apply Fogging 끔.
- Emissive Color = 화면 표시 색 그대로(0..1, 노출과 무관), Opacity = 불투명도(0..1).
- TexCoord[0]: U = 경로 진행 방향(0..1이 `TileLengthCm` 한 칸, 반복), V = 띠 왼쪽 0 .. 오른쪽 1.
- 애니메이션은 Time/Panner — U에서 시간을 빼면 차량 진행 방향으로 흐른다.

## 5. 콘솔 변수

| cvar | 기본값 | 위치 | 의미 |
|---|---|---|---|
| `titan.UGVPath.Enable` | 1 | `UGVPathPreviewSubsystem.cpp:21` | 표시 켬/끔 |
| `titan.UGVPath.OutputSpacingCm` | 100 | `UGVPathPreviewSubsystem.cpp:23` | GPU로 보내는 점 간격(작을수록 정밀) |
| `titan.UGVPath.Temporal` | 1 | `UGVPathOverlay.cpp:35` | 마스크 시간 누적(0 = 끔 → 경계 계단 A/B 비교) |
| `titan.UGVPath.TemporalAlpha` | 0.1 | `UGVPathOverlay.cpp:38` | 누적에서 이번 프레임 비중(0.02~1로 클램프). 작을수록 부드럽고 반응 느림 |

모양·길이는 전부 스타일 에셋(§4). 2차 개정에서 `RefreshSeconds`/`MaxLengthCm`/`HalfWidthCm`/`StartOffsetCm`/`ZToleranceCm` cvar는 제거됨.

## 6. 트러블슈팅 / 교훈 (개정 이력 요약)

| 차수 | 증상 | 원인 → 조치 |
|---|---|---|
| 1차 | 오버레이가 처음 그려지는 순간 에디터 크래시 | 픽셀 셰이더(`AddDrawScreenPass`)의 D3D12 그래픽스 PSO 생성 E_INVALIDARG(`LogD3D12RHI … 80070057` → `PipelineStateCache.cpp:712 "Shader compilation failures are Fatal"`, 브레드크럼 `UGVPathOverlay`). PSO 디스크립터는 정상이라 정확한 사유는 `-d3ddebug` 없이 미확정 → **컴퓨트로 교체**해 그래픽스 PSO 자체를 없앰(같은 지점 TitanFlicker 계측기와 같은 길). 면 노멀은 `ddx/ddy` 대신 이웃 깊이 픽셀 |
| 2차 | 화살표가 "약간 끊겨서 움직이는 느낌" | 0.1 s마다 재예측하며 s를 **매번 차량에서 0으로** 재시작 → 패턴이 달린 거리(10 km/h에서 ~28 cm)만큼 초당 10번 뒤로 튐 → 이동 명령당 1회 예측·고정 + 차량 투영 + rebase 시 s 연속(§3-1) |
| 2차 후 | 숲길 오르막에서 띠가 잘렸다가 다가가면 다시 보임 | `FindChaosLookaheadTarget`은 **3D 거리**로 최근접 구간을 고르는데 예측은 Z를 출발 높이에 고정 → 가상 차량이 땅속 높이에서 지난 구간에 붙음 → 목표점이 뒤로 → 제자리선회 반복 → 반복 상한에서 종료 → Z를 지면 프로파일 + 지면 위 높이로(§3-2), `ended early` 로그 추가 |
| 3차 | 작은 풀/고사리 위에 찢어진 듯한 색 덩어리 | Z 허용치 120 cm(내비메시 Z가 부정확해서 크게 잡음)라 고사리 잎 높이까지 통과 → **콜리전 지면 높이**(바퀴와 같은 트레이스)로 높이 마스크(§3-1). 사용자 확인 "잘 된다" |
| 4차 | 풀이 띠를 가리는 경계가 계단 | 판정 깊이는 TSR 이전 렌더 해상도, 색은 TSR 이후 출력 해상도 → 2패스(FieldCS 렌더 해상도 + CompositeCS bilinear). 체감 차이는 작았음. `r.AntiAliasingMethod 0`은 레벨 전체에 계단이 생겨 비교 불가 |
| 5차 | slomo 0.1에서 띠 전체가 미세하게 떨림 | TSR 지터 미보정 — 필드는 지터된 깊이, 합성은 지터 없는 위치 → ±0.5 렌더 픽셀 떨림. `TemporalAAJitter` 보정으로 해결(사용자 확인) |
| 6차 | 띠와 맞닿은 물체(풀, UGV 차체 모서리) 경계 계단 | 오버레이가 TSR 이후라 TSR의 서브픽셀 경계 혼합을 못 받음(구조적, §2) → 마스크 시간 누적(§3-3 ②-3). 사용자 "어느정도 해결, 수렴되고 있음" |
| 6차 중 | RDG 크래시 "has not been produced by any pass"(`RenderGraphValidation.cpp:429`) | `QueueTextureExtraction(HistoryOut)`을 그 텍스처를 쓰는 패스 추가 **전**에 호출 → 패스 추가 **뒤**로 옮김(`UGVPathOverlay.cpp:469-476`) |

**예전 Niagara 리본 찢어짐 가설(미검증)** — (a) 리본 파티클이 재생성(두 번째 명령) 사이에 남아 이전 경로와 섞임,
(b) `GetPathPointsSnappedToGround`가 각 점에서 +50 m 위에서 `ECC_WorldStatic` 트레이스 → 숲에서 나무 수관에 맞아 점이 공중으로 튐.
새 구조는 상태가 없고(버퍼 통째 교체) 지면 트레이스는 예측 Z +200 cm에서 시작하므로 둘 다 구조적으로 해당 없음. 리본을 되살릴 일이 없으면 확인 불필요.

## 7. 변경 파일 (P4 기본 체인지리스트, 미제출)

**신규(add)**

- `Plugins/UGVPathOverlay/UGVPathOverlay.uplugin`
- `Plugins/UGVPathOverlay/Source/UGVPathOverlay/UGVPathOverlay.Build.cs`
- `Plugins/UGVPathOverlay/Source/UGVPathOverlay/Public/UGVPathOverlay.h`
- `Plugins/UGVPathOverlay/Source/UGVPathOverlay/Private/UGVPathOverlay.cpp`
- `Plugins/UGVPathOverlay/Source/UGVPathOverlay/Private/UGVPathOverlayModule.cpp`
- `Plugins/UGVPathOverlay/Shaders/Private/UGVPathOverlay.usf`
- `Source/titan_example/UI/UGVPathPreviewSubsystem.h/.cpp`
- `Source/titan_example/UI/UGVPathStyleAsset.h/.cpp`

**수정(edit)**

- `Source/titan_example/Vehicles/UGVAIController.h/.cpp` — `BuildPredictedDrivePath`, `RebuildGroundProfileIfNeeded`, `PathRevision`/`GroundProfile*`, `GetPathRevision()`
- `Source/titan_example/UI/UGVTestDashboardWidget.h/.cpp` — NativeConstruct `RegisterDashboard`, NativeDestruct(신규 오버라이드) `UnregisterDashboard`, `PathPreviewStyle` UPROPERTY
- `Source/titan_example/titan_example.Build.cs` — `PrivateDependencyModuleNames.Add("UGVPathOverlay")`

> ⚠️ 같은 기본 체인지리스트에 다른 세션 작업(TitanFlickerMeter, AxisLaunchOptions, PlayerController 등)이 섞여 있다 — 제출 시 분리할 것.

## 8. 남은 한계 / 후속

- **시간 누적 AA는 완벽하지 않음** — 가려졌다 드러난 픽셀(disocclusion)은 몇 프레임 계단, 카메라 급회전·줌 직후 짧은 계단/약한 잔상.
  `titan.UGVPath.TemporalAlpha`로 조절 가능. 풀/나무는 스태틱(WPO 없음)이라 잔상 위험은 낮음.
- **지면 샘플링 게임 스레드 히치** — 경로 (재)생성마다 실측 2.0~4.5 ms(301점 / 2405 트레이스). 필요하면 비동기 트레이스나 프레임 분할.
- **rebase가 5~15초마다 걸리는 것이 관찰됨**(`ground sampled` 로그) — 예측이 실제 주행과 조금씩 어긋난다는 뜻. 재예측 순간 차량 앞 모양이
  바뀌는 게 거슬리면 `RebaseDistanceCm` 상향 또는 이음매 블렌드 검토.
- **디자이너용 예시 패턴 머티리얼 + 스타일 에셋 미작성** — 규칙은 §4. 에셋 생성은 사용자/디자이너가 에디터에서.
- **리눅스/Vulkan 패키지 미검증** — SM5 컴퓨트 셰이더 컴파일·동작 확인 필요.
- **`stat GPU` `UGVPathOverlay` 비용 미보고** — 목표 < 0.3 ms(패턴 머티리얼 RT 그리기 포함).
- **랜드스케이프 전용 커스텀 뎁스 필터 보류(사용자 결정)** — Substrate(`r.Substrate=True`)라 셰이딩 모델 ID로 못 거름. 랜드스케이프 Render
  CustomDepth + 씬 깊이 비교 안은 메인뷰 추정 0.2~0.5 ms + 씬 캡처마다 같은 비용 가능성이라 보류. 콜리전 높이 판정으로 대부분 해결됨.
- 정리 미착수: 예전 `PathPreviewFX`(BP Niagara 컴포넌트)·BP `OnPathUpdated` 배선(비활성 상태로 둠), `GetPathPointsSnappedToGround`의
  TEMP DIAGNOSTIC 로그(2026-08-04) — 리본 정리 때 같이.
- 명시적 확인 기록이 없는 항목: RTSP(VLC) UGV RCWS 스트림에 선 없음, 대시보드 OFF·트럭 뷰 전환 시 선 없음, 시나리오 재시작 시 찢어짐 없음.
