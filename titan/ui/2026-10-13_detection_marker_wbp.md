# 탐지 박스 WBP 마커화 + 실루엣 마스크 강조 (UAV/RCWS)

2026-10-13 / 완료(C++ + 에디터 에셋 — UGV 테스트 화면 PIE 기본 동작 확인, Monitor1/2·RTSP·2-PC는 미검증, 디자이너 PNG 대기; 디자인 확인용 테스트 레벨 `L_TD_Test` 추가 — 10절) / 탐지 박스를 대상 종류×진영별 디자이너 WBP(`UDetectionMarkerWidget`)로 그림(글자 크기 고정, 거리 C++ 바인딩) + 탐지된 대상만 그리는 저해상도 "정답 마스크" 캡처(`UDetectionMaskCaptureComponent`)를 UMG 머티리얼로 반투명 스케치 강조 — LIG RTSP 영상에는 안 실림.

---

## 0. 현재 상태 + 남은 것 (2026-10-13 기준, 여기부터 읽을 것)

옛 인계 TODO(MF/머티리얼 그래프, 마커 WBP, 3개 화면 지정, 에디터 빌드)는 **전부 끝났다**. 에셋 경로·규칙은 3절, 머티리얼은 4.5절.

| 항목 | 상태 |
|---|---|
| C++ (1절 표) | 완료. 에디터 타깃 사용자 빌드 3회 성공(7절, 4차 = 테스트 레벨용) |
| `MF_DetectionMaskCoverage` / `M_UI_DetectionSketch` | 완료(MCP 제작, 사용자가 머티리얼 에디터 **Apply** 후 정상 — 9절 함정 2) |
| MI 6개(조합별 색) | 사용자 제작, `UseStateColor=0` + 각자 `OverrideColor` |
| 마커 WBP 14개(RCWS 7 + UAV 7) | 레이아웃 사용자 제작. `BoxImage`는 아직 빈 자리(알파 0) — **디자이너 PNG 대기** |
| 3개 화면 오버레이 설정 | 완료 — 기본 마커 + `MarkerKindRules` 6행(3.4절) |
| PIE | New_kadex_0811 UGV 테스트 화면에서 박스·거리·반투명 실루엣 정상("아주 잘돼") |
| 디자인 확인용 테스트 레벨 `L_TD_Test` | 완료 — 시나리오 없이 6종 대상을 한 화면에 놓고 자체방호/UGV 축으로 바로 PIE. `ADetectionTestDirector` + 1인 재스폰. 자체방호 축 PIE 검증(10절) |
| Tracking/Locked(RCWS 자동조준 연동) | **범위 밖으로 뺌**(사용자 결정) — 코드는 남겨 두고 미사용, 별도 작업 |
| P4 | 아무것도 제출 안 함(사용자가 "제출"이라고 할 때만). 신규 에셋 23개(+`L_TD_Test.umap`)는 **아직 `p4 add` 안 됨** — 8절 |

**남은 것**: 6절.

요청 원문: `memo.md` 829행 — "uav/rcws 타겟 디텍션 시 노란색/빨간색/파란색 사각형들 디자인 가능하게(WBP 기반, 글자/이미지), 박스가 작아져도 글자는 안 작아지게 / 반투명 오버레이 강조 가능한지 / selfdefense monitor1·2 및 ugv 테스트 WBP에서도".

---

## 1. 무엇이 바뀌었나

| 파일 | 내용 |
|---|---|
| `Source/titan_example/UI/DetectionMarkerWidget.h/.cpp` (신규) | `UDetectionMarkerWidget : UUserWidget`(Abstract, WBP 부모용), `EDetectionMarkerState`, `FDetectionMarkerInfo`(+ `Kind` FName·내부 `KindRuleIndex`). `BindWidgetOptional` **`DistanceText`** — C++가 `GetDistanceText()`("125 m", 정수 반올림, 모르면 빈 문자열)를 정수값이 바뀔 때만 씀. `MaskImage` MID 생성 실패 시 1회 Warning `[MaskDiag] ... MaskImage 브러시가 머티리얼이 아니라...` |
| `Source/titan_example/UI/DetectionOverlayWidget.h/.cpp` | 마커 풀/배치, 상태 판정, `SetDetectionsFrom`, 디자이너 미리보기. 신규 USTRUCT **`FDetectionMarkerKindRule`** + 프로퍼티 **`MarkerKindRules`**(3.4절), `FindKindRuleIndex`/`GetMarkerClass(const FDetectionMarkerInfo&)`/`HasAnyMarkerClass()`. 진단 cvar **`Detection.MaskDiag`**(`LogMaskDiag`, 3.5절). `SDetectionOverlay`는 색이 정해진 선 박스 목록(`FDetectionOverlayLineBox`)만 그림 |
| `UI/SelfDefenseMonitor1Widget.cpp` | `UAVViewDetectionOverlay->SetDetectionsFrom(드론 TargetDetection)` |
| `UI/SelfDefenseMonitor2Widget.cpp` / `UI/UGVTestDashboardWidget.cpp` | `SetDetectionsFrom(...)`. 한때 넣었던 `SetLockState(...)` 호출은 **제거**(Tracking/Locked 미사용) |
| `Source/titan_example/Detection/DetectionMaskCaptureComponent.h/.cpp` (신규) | 실루엣 마스크 캡처(4절), `EDetectionMaskUpdateMode`, 진단용 `GetShowOnlyCount()` |
| `Detection/TargetDetectionComponent.h/.cpp` | `GetCompletedScanCount()` — 마스크 "스캔 동기" 모드용. 탐지 동작 불변 |
| 에셋 `Content/widget/target_detection/**` (신규 22개) | MF 1, M 1, MI 6, WBP 14 — 3절·4.5절 |
| `Content/widget/WBP_SelfDefenseMonitor1/2`, `WBP_UGVTestDashboard` | 오버레이 기본 마커 클래스 + `MarkerKindRules` 6행 |
| `Source/titan_example/Detection/DetectionTestDirector.h/.cpp` (신규) | 테스트 레벨 감독 액터 `ADetectionTestDirector`(10절) — 축 선택, 트럭 RCWS 자동 감시, 드론 호버·전 진영 탐지, 죽은 병사 재스폰. 정적 `FindTestAxis(World, OutAxis)` |
| `titan_exampleGameMode.cpp` | `InitNewPlayer`: URL에 `?Axis=`가 없으면 레벨의 감독 액터 `TestAxis`를 씀(감독 없는 다른 레벨은 불변) |
| `UI/ScenarioRespawnSubsystem.h/.cpp` | 공개 `AActor* RespawnActor(AActor* OldActor)` 추가 — 1인 재스폰(10절) |
| 레벨 `Content/widget/target_detection/L_TD_Test.umap` (신규) | 디자인 확인용 테스트 레벨(10절) |

**기존 WBP 호환**: 마커 클래스가 하나도 없으면 예전처럼 선으로 그린다. 레거시 `Monitor1Widget`/`MissionDashboardWidget`/`SelfDefenseDashboardWidget`은
`SetDetections`를 그대로 쓰므로 거리 -1.

### 노랑/빨강/파랑의 정체

C++ 색은 진영뿐이다 — `FriendlyColor` / `EnemyColor`(빨강) / `EvidenceColor`(노랑, 낙하산 `EnemyEvidence`).
3개 화면의 `FriendlyColor` 오버라이드 = **(0.012, 0.309, 0.708)** → "파랑 박스 = 아군"으로 **확인**. 이제 마커 WBP를 쓰는 대상은
색이 진영 색이 아니라 **조합별 MI의 `OverrideColor`**(실루엣)와 WBP 디자인에서 나온다.

## 2. 상태 (`EDetectionMarkerState`)

| 상태 | 조건 | 색 프로퍼티 | 사용 |
|---|---|---|---|
| `Friendly` | 진영 Friendly | `FriendlyColor` | 사용 |
| `Enemy` | 진영 Enemy | `EnemyColor` | 사용 |
| `Evidence` | 진영 EnemyEvidence(낙하산) | `EvidenceColor` | 사용 |
| `Tracking` | 대상 == RCWS `CurrentAutoAimTarget`, 게이지 충전 중 | `TrackingColor` | **미사용** |
| `Locked` | 위 + `bIsLockedOn` | `LockedColor` | **미사용** |

`SetLockState()`·`TrackingColor`/`LockedColor`·상태 판정 코드는 남아 있지만 아무도 `SetLockState`를 부르지 않아 Tracking/Locked가 안 나온다.
RCWS 자동조준 연동은 **별도 작업**(사용자 결정). 그때 `SelfDefenseMonitor2Widget`/`UGVTestDashboardWidget`에서 `SetLockState(RCWSFireControl...)`만
다시 넣으면 됨(`CurrentAutoAimTarget`/`bIsLockedOn`/`LockOnGaugeValue`는 이미 복제되는 값).

## 3. 마커 WBP

### 3.1 왜 글자가 안 작아지나

오버레이가 대상마다 마커 WBP 하나를 만들고, 내부 `SConstraintCanvas` 슬롯의 **앵커를 박스 UV 범위(Min~Max)로 늘려서** 배치한다 —
스케일이 아니라 **레이아웃 크기**가 박스에 맞춰진다. 그래서 WBP 안에서:

| 무엇 | 앵커 | 결과 |
|---|---|---|
| 박스를 따라야 하는 것(테두리, 실루엣) | **스트레치**(Min≠Max, 0..1), 오프셋 0 | 박스와 같이 늘고 줄음 |
| 고정 크기 글자/아이콘 | **점 앵커** + Size To Content | 박스가 작아져도 그대로 |

⚠ **Scale Box 쓰지 말 것** — 글자까지 같이 줄어든다.

### 3.2 폴더·이름 (`/Game/widget/target_detection/`)

| 폴더 | 내용 |
|---|---|
| `Core/` | `MF_DetectionMaskCoverage`(4.4) |
| `Materials/` | `M_UI_DetectionSketch`(4.5), `MI_DetSketch_{SoldierAlly,SoldierEnemy,Truck,UGV,Drone,Parachute}` |
| `Markers/RCWS/` | `WBP_DetMarker_RCWS`(공용 기본/폴백, MaskImage 없음) + `_SoldierAlly`·`_SoldierEnemy`·`_Truck`·`_UGV`·`_Drone`·`_Parachute` |
| `Markers/UAV/` | 같은 구성 `WBP_DetMarker_UAV*` 7개 |
| `Textures/` | 디자이너 프레임 PNG 자리(`T_DetBox_...`) |

- 이름 규칙: **`WBP_DetMarker_<보는쪽>_<조합>`**. 부모 클래스 전부 `DetectionMarkerWidget`.
- RCWS = Monitor2(트럭) + UGV 테스트 화면, UAV = Monitor1 드론 화면. 드론 화면이 훨씬 작아서 **폰트를 줄인 별도 세트**(C++ 콘텐츠 스케일 옵션 대신 사용자 선택).

### 3.3 WBP 레이아웃 (사용자 제작)

| 위젯 | 배치 | 역할 |
|---|---|---|
| (루트) Canvas Panel | | |
| `BoxImage` | 스트레치 0..1, 오프셋 0 | **디자이너 프레임 PNG 자리**. 머티리얼 브러시 넣지 말 것. 지금은 브러시 비움·RoundedBox·ColorAndOpacity 알파 0(안 보이는 자리표시) |
| `InfoBox`(Horizontal Box) | 좌상 점 앵커, Alignment (0,1), Position Y −2, Size To Content | 박스 바로 위 글자 줄 |
| └ `LabelText` | Auto 슬롯 | WBP마다 고정 문구(조합별 디자인) |
| └ `DistanceText` | Auto 슬롯 | **C++가 채움**(`BindWidgetOptional`) — BP 그래프 불필요 |
| `MaskImage` | 스트레치 0..1, 오프셋 **정확히 0**, 브러시 = 그 조합 MI | 실루엣(조합 WBP에만, 기본 WBP엔 없음) |

- 프로젝트 관례: 값 바인딩은 C++에서(`BindWidgetOptional`), WBP 그래프 안 씀. `OnMarkerUpdated`/`OnMarkerStateChanged` BP 이벤트는 등장 애니메이션 등 필요할 때만.
- 글자가 카메라 프레임 밖으로 삐져나오는 게 싫으면 오버레이 Clipping = Clip to Bounds.

### 3.4 오버레이 설정 — 종류×진영 규칙 (`Detection Overlay | Marker`)

**마커 클래스 결정 순서**: 맞는 `MarkerKindRules` 행의 `MarkerWidgetClass` → `MarkerWidgetClassByState` → `MarkerWidgetClass` → 없으면 선 박스.

`FDetectionMarkerKindRule` = { `Kind`(FName, 마커 `Info.Kind`로 전달), `TargetClass`(`TSoftClassPtr<AActor>`, IsA 매칭, 강제 로드 안 함),
`bMatchFaction` + `Faction`, `RequiredTag`(선택), `MarkerWidgetClass` }. **위에서부터 첫 매칭 행이 이긴다.** 클래스도 태그도 없는 행은 무시.

| Kind | TargetClass | Faction |
|---|---|---|
| SoldierAlly | `/Script/Engine.Character` | Friendly |
| SoldierEnemy | `/Script/Engine.Character` | Enemy |
| Truck | `TitanTruck` | 무관(`bMatchFaction=false`) |
| UGV | `UGVWheeledVehiclePawn`(BP_UGV_0901·BP_UGV_Vehicle* 포함, UGV_OLD 무시) | 무관 |
| Drone | `DronePawn`(BP_Drone, 구 BP_UAV 제외) | 무관 |
| Parachute | `/Game/Parachute/BP_Parachute.BP_Parachute_C` | 무관 |

- 3개 화면 모두 같은 6행, WBP만 그 화면 폴더 것(Monitor2·UGV 대시보드 = `RCWS/`, Monitor1 = `UAV/`).
- 진영 사실(`guide/detection_dev_guide.md` §2): 진영이 갈리는 건 보병뿐, 트럭/UGV/드론은 전부 Friendly, 낙하산은 EnemyEvidence. New_kadex_0811 병사는 `BP_Soldier_Friendly`/`BP_Soldier_Hostile`(Character).

| 화면(WBP) | 오버레이 | 기본 `MarkerWidgetClass` |
|---|---|---|
| `WBP_SelfDefenseMonitor1` | `UAVViewDetectionOverlay` | `WBP_DetMarker_UAV` |
| `WBP_SelfDefenseMonitor2` | `MainViewDetectionOverlay` | `WBP_DetMarker_RCWS` |
| `WBP_UGVTestDashboard` | `MainViewDetectionOverlay` | `WBP_DetMarker_RCWS` |

`MarkerWidgetClassByState` 비움, `MinMarkerSize` 0(선택: 드론 32 / RCWS 64), `bEnableDetectionMask` true. `Preview Markers In Designer`는 가짜 박스 + **규칙마다 가짜 박스 1개씩**(아래 줄, 그 규칙의 진영).

### 3.5 진단 — `Detection.MaskDiag 1`

기본 0. 켜면 오버레이마다 1초에 1번:
`[MaskDiag] <overlay>: stage=... detector=... detected= markers= withMaskImage= capture= showOnly= texture=WxH`

| stage | 뜻 |
|---|---|
| `off(이유)` | 마스크 꺼짐/MaskImage 가진 마커 없음 등 |
| `FAIL(FindOrCreateFor)` | 캡처 컴포넌트 생성 실패 |
| `wait(overlay pixel size 0)` | 오버레이 첫 프레임, 크기 아직 0 |
| `requested` | 정상 요청 중 |

정상 예(UGV 테스트 PIE): `stage=requested`, detected 1–2, markers = withMaskImage = detected, `capture=DetectionMaskCapture_0`, showOnly 일치, `texture=960x540`.

### 3.6 구현 메모

- 마커는 대상(액터)별로 이어 쓴다 — 같은 대상·같은 클래스면 같은 인스턴스. 사라진 대상의 마커는 Collapsed로 풀에 두고 재사용.
- 마커는 `HitTestInvisible`. 디자이너 미리보기 인스턴스는 `RF_Transient`.
- `MarkerSize`/`Min Marker Size`는 오버레이의 **지난 프레임 지오메트리** 기준 — 최소 크기가 한 프레임 늦게 적용.
- 거리 = 탐지 카메라(`GetCamera()`) ↔ 대상 액터, cm/100(레벨 1:1).
- 박스 좌표·갱신 주기는 그대로(탐지 0.1 s 스캔, 위치 보간 없음).

## 4. 실루엣 강조 — 정답 마스크 캡처 + UMG 머티리얼

### 4.1 왜 이 방식인가 (요구: LIG 영상엔 절대 안 감, 가림 무관, 객체인식 "흉내")

- 실제 시스템: 세그멘테이션 모델이 대상 픽셀 마스크를 추론 → 운용 UI가 그 위에 윤곽/채움을 그린다. 우리는 모델 대신 엔진이
  **정답 마스크**를 그린다(CARLA/AirSim이 학습 데이터 라벨을 만드는 방식).
- **포스트프로세스/스텐실은 불가**: RTSP 메인뷰 송출(`Plugins/RtspEncoder/.../MainViewFrameSource.cpp:46-63`)은 After-Tonemapping PP
  머티리얼까지 다 적용된 뒤(VisualizeDepthOfField 지점)를 복사하고, 드론 짐벌은 Monitor1과 RTSP `uav_gimbal`이 같은 렌더 타깃 → 무조건 LIG로 간다.
  UMG는 그 뒤에 Slate가 따로 합성하므로 **구조적으로 송출에 안 섞인다**. `OverlayMaterial`은 모든 뷰에 보여서 탈락.

### 4.2 `UDetectionMaskCaptureComponent`

| 항목 | 내용 |
|---|---|
| 붙는 곳 | 탐지기 카메라(`UTargetDetectionComponent::GetCamera()` — 드론 `GimbalCapture`, 트럭/UGV `SightCamera`)의 **자식**으로 런타임 생성(`FindOrCreateFor`, `RF_Transient`). 위치는 부모를 따라가고 FOV는 매 틱 복사 |
| 언제 존재/렌더 | 보이는 마커 중 `MaskImage`를 가진 게 있을 때만 오버레이가 `RequestMask` → 요청이 0.5 s(`StaleRequestSeconds`) 끊기면 캡처 정지. UI 없는 프로세스·마스크 안 쓰는 WBP는 비용 0 |
| 무엇을 그리나 | `PRM_UseShowOnlyList` — **이번 `DetectedTargets` 액터 + 부착 액터**(손의 총 등)만. 지형·나무·하늘 없음 → 가림과 무관한 전체 실루엣. 대상이 0이면 캡처 생략 |
| 출력 | `SCS_SceneDepth` → `R32F` 렌더 타깃(R = view Z cm, 빈 곳은 아주 큰 값), **필터 Nearest**(경계 처리 때문 — 4.4) |
| 해상도/화면비 | 오버레이 영역 실제 픽셀 × `MaskResolutionScale`(기본 0.5), 한 변 16~2048. 화면비가 표시 영역과 같아 탐지 투영(가로 FOV = `FOVAngle`)과 일치 |
| 꺼 둔 것 | 쇼 플래그 PostProcessing/AA/TemporalAA(지터 → 윤곽 떨림 방지)/MotionBlur/Bloom/EyeAdaptation/Lighting/그림자/GI·Lumen/AO/반사/하늘·대기·안개/반투명/파티클/데칼. NaniteMeshes는 유지. `bExcludeFromSceneTextureExtents` |
| 틱 | `TG_PostUpdateWork`(RCWS 슬루/짐벌이 이번 프레임 자세를 다 잡은 뒤) |

**타이밍 — "누가"와 "얼마나 자주"를 분리** (오버레이 `Detection Overlay | Mask`):

| 프로퍼티 | 뜻 |
|---|---|
| 대상 목록 | 항상 탐지 스캔을 따른다(= 모델이 발견/상실한 것). 설정 없음 |
| `Mask Update Mode` | `EveryFrame`(기본 — 애니메이션을 매끄럽게 따라감) / `FixedRate`(`Mask Update Rate Hz`, "AI 추론 영상" 느낌) / `SyncWithDetectionScan`(박스와 같은 0.1 s 박자) |
| `FixedRate` 보정 | 박자 사이에 대상이 새로 잡히거나 사라진 스캔이 오면 **목록이 실제로 바뀐 경우에만** 즉시 1회 캡처 — 박스는 떴는데 윤곽이 없는 순간 방지 |
| `Mask Resolution Scale` | 0.05~1.0 |
| `Mask Depth Tolerance Scale/Bias` | 대상 분리 깊이 범위 = 루트 바운드 반경 × 1.2 + 50 cm(기본) |
| `Enable Detection Mask` | 끄면 마스크 전체 OFF |

**겹친 대상 분리**: 마스크는 한 장(모든 대상)이고, 각 마커가 `TargetDepthCm ± DepthToleranceCm` 깊이만 자기 것으로 친다. 대상 깊이는
**매 갱신 현재 위치**(루트 바운드 중심의 view Z)라, 윤곽이 박스(0.1 s 탐지 결과)보다 자주 움직여도 맞는다. 같은 깊이에서 화면상 겹친 두 대상은
서로의 윤곽이 상대 마커 사각형 안에 일부 보일 수 있다(마커 사각형으로만 잘림) — 한계로 둠.

### 4.3 마커 WBP 연결 — `MaskImage`

마커 WBP에 이름이 정확히 **`MaskImage`**인 Image(스트레치 0..1·오프셋 0)를 두고 브러시에 MI를 넣으면, C++가 그 브러시 머티리얼로 MID를 만들어
매 갱신 아래 파라미터를 넣는다(`DetectionMarkerWidget.h`의 `DetectionMaskParams`). 마스크가 없으면 `MaskImage`는 Collapsed.
브러시가 머티리얼이 아니면 MID를 못 만들어 1회 Warning(`[MaskDiag] ... 머티리얼이 아니라...`). 머티리얼에 없는 이름은 무시된다.

| 파라미터(이름 그대로) | 타입 | 값 |
|---|---|---|
| `DetectionMask` | **Texture Object Parameter** | 마스크 렌더 타깃 |
| `MaskUVRect` | Vector | 마커 사각형의 오버레이 UV (minU, minV, maxU, maxV) |
| `MaskTexelSize` | Vector | (1/W, 1/H, W, H) |
| `TargetDepthCm` / `DepthToleranceCm` | Scalar | 대상 깊이 / 허용범위 |
| `MaskValid` | Scalar | 1 |
| `StateColor` | Vector | 상태 색(오버레이 진영 색) — 지금 MI는 `UseStateColor=0`이라 안 씀 |
| `Confidence` / `VisibleFraction` / `LockProgress` / `IsLocked` | Scalar | 탐지/락온 값(머티리얼엔 `VisibleFraction` 없음, `LockProgress`/`Confidence`는 미연결) |

### 4.4 경계 처리 (저해상도 마스크) — 코어 머티리얼 함수 `MF_DetectionMaskCoverage`

깊이를 하드웨어 bilinear로 키우면 경계 텍셀에서 대상 깊이와 배경(무한대)이 섞인 **엉뚱한 깊이**가 나와 테두리에 띠가 생긴다. 그래서
**먼저 판정, 나중에 보간**: 렌더 타깃은 Nearest로 두고, 4텍셀을 각각 "이 대상인가(0/1)"로 판정한 뒤 그 0/1을 bilinear로 섞어 부드러운
커버리지를 만든다. 외곽선은 마스크 텍셀이 아니라 **화면 px 단위**(`ddx/ddy`로 화면 1 px의 마스크 UV 크기를 구해 8방향 샘플) — 마스크 해상도와
무관하게 두께 고정. 디자이너는 이 함수를 열 필요 없다.

실제 에셋 `/Game/widget/target_detection/Core/MF_DetectionMaskCoverage`:
- Function Input 8개: `MaskTex`(Texture2D, 미리보기 = 엔진 DefaultTexture), `LocalUV`(Vector2 — 기본 TexCoord[0]을 Preview 핀 + `bUsePreviewValueAsDefault`로),
  `MaskUVRect`(Vector4), `MaskTexelSize`(Vector4), `TargetDepthCm`, `DepthToleranceCm`, `OutlineWidthPx`(기본 2), `DilateTexels`(기본 0)
- Custom 노드 1개(Output Type **CMOT Float 4**, 입력 `Tex/LocalUV/UVRect/TexelSize/TargetDepth/Tolerance/OutlineWidthPx/DilateTexels`, 아래 HLSL)
- ComponentMask R/G/B → Function Output `Coverage`/`Interior`/`Outline`

```hlsl
// Inputs: Tex(Texture Object), LocalUV(float2), UVRect(float4), TexelSize(float4),
//         TargetDepth, Tolerance, OutlineWidthPx, DilateTexels
// Return float4(Coverage, Interior, Outline, 0)
float2 maskUV = lerp(UVRect.xy, UVRect.zw, LocalUV);
float2 pxX = ddx(maskUV);
float2 pxY = ddy(maskUV);
int r = (int)clamp(DilateTexels, 0.0, 1.0);
float2 dirs[9] = { float2(0,0), float2(1,0), float2(-1,0), float2(0,1), float2(0,-1),
                   float2(0.7071,0.7071), float2(-0.7071,0.7071), float2(0.7071,-0.7071), float2(-0.7071,-0.7071) };
float cov[9];
for (int i = 0; i < 9; i++)
{
    float2 uv = maskUV + (pxX * dirs[i].x + pxY * dirs[i].y) * OutlineWidthPx;
    float2 st = uv * TexelSize.zw - 0.5;
    float2 b = floor(st);
    float2 f = st - b;
    float m00 = 0, m10 = 0, m01 = 0, m11 = 0;
    for (int y = -r; y <= r; y++)
    {
        for (int x = -r; x <= r; x++)
        {
            float2 o = b + float2(x, y);
            float d00 = Texture2DSampleLevel(Tex, TexSampler, (o + float2(0.5, 0.5)) * TexelSize.xy, 0).r;
            float d10 = Texture2DSampleLevel(Tex, TexSampler, (o + float2(1.5, 0.5)) * TexelSize.xy, 0).r;
            float d01 = Texture2DSampleLevel(Tex, TexSampler, (o + float2(0.5, 1.5)) * TexelSize.xy, 0).r;
            float d11 = Texture2DSampleLevel(Tex, TexSampler, (o + float2(1.5, 1.5)) * TexelSize.xy, 0).r;
            m00 = max(m00, abs(d00 - TargetDepth) <= Tolerance ? 1.0 : 0.0);
            m10 = max(m10, abs(d10 - TargetDepth) <= Tolerance ? 1.0 : 0.0);
            m01 = max(m01, abs(d01 - TargetDepth) <= Tolerance ? 1.0 : 0.0);
            m11 = max(m11, abs(d11 - TargetDepth) <= Tolerance ? 1.0 : 0.0);
        }
    }
    cov[i] = lerp(lerp(m00, m10, f.x), lerp(m01, m11, f.x), f.y);
}
float coverage = cov[0];
float interior = smoothstep(0.35, 0.65, coverage);
float minN = 1.0;
for (int j = 1; j < 9; j++) { minN = min(minN, cov[j]); }
// 안쪽 외곽선: 나는 안인데 OutlineWidthPx 떨어진 이웃 중 바깥이 있음
float outline = (OutlineWidthPx > 0.0) ? interior * (1.0 - smoothstep(0.35, 0.65, minN)) : 0.0;
return float4(coverage, interior, outline, 0);
```

비용: 픽셀당 텍스처 읽기 9×4 = 36회(`DilateTexels`=1이면 324회 — 얇은 총열·손가락이 끊길 때만 켤 것). 마커 사각형 안에서만 돈다.

### 4.5 디자이너용 머티리얼 `M_UI_DetectionSketch` + 조합별 MI

`/Game/widget/target_detection/Materials/M_UI_DetectionSketch` — Domain **User Interface**, Blend **Translucent**
(Substrate 프로젝트라 에디터에 "Translucent Grey Transmittance"로 표시 — 정상).

| 그룹 | 파라미터(기본값) |
|---|---|
| Detection (C++ sets) | `DetectionMask`(TextureObjectParameter), `MaskUVRect`, `MaskTexelSize`, `TargetDepthCm`, `DepthToleranceCm`, `MaskValid`(0), `StateColor`, `IsLocked`, `LockProgress`·`Confidence`(존재만, 미연결) |
| Style | `FillOpacity` 0.25, `OutlineOpacity` 0.9, `OutlineWidthPx` 2, `DilateTexels` 0, `UseStateColor` **0**, `OverrideColor`, `HatchSpacingPx` 6, `HatchStrength` 0.4, `LockedPulseSpeed` 6, `LockedPulseAmount` 0.3 |
| Preview | `PreviewShape`(1) |

그래프:
- **Final Color** = `Lerp(OverrideColor, StateColor, UseStateColor)` — `OverrideColor`는 **RGB만** 쓰고 알파 무시. 투명도는 `FillOpacity`/`OutlineOpacity`/`HatchStrength`로.
- **Hatch** = Custom `return Parameters.SvPosition.xy;` → `(x+y)/HatchSpacingPx` → Frac → Step(0.5) → `Lerp(1, step, HatchStrength)`
- **Opacity** = `(Interior×(1−Outline)×FillOpacity×Hatch + Outline×OutlineOpacity) × max(MaskValid, PreviewShape) × Pulse`
  (`Pulse = 1 − IsLocked × LockedPulseAmount × (0.5+0.5·sin(Time×LockedPulseSpeed))` — 지금은 IsLocked가 안 와서 항상 1)
- **PreviewShape(미리보기 전용 가짜 실루엣)**: Custom 노드 "PreviewSilhouette"(머리+몸통 SDF, 외곽선은 화면 px).
  `Interior/Outline = Lerp(가짜×PreviewShape, 실제 MF 출력, MaskValid)`. 런타임엔 C++가 `MaskValid=1`을 넣어 **실제 실루엣이 항상 이기고**,
  마스크가 없으면 C++가 MaskImage를 Collapsed → **PreviewShape=1 그대로 둬도 안전**. 덕분에 MI 미리보기·마커 WBP 디자이너에서 사람 모양이 보인다.

**MI 6개**(`Materials/MI_DetSketch_{SoldierAlly,SoldierEnemy,Truck,UGV,Drone,Parachute}`, 사용자 제작): `UseStateColor=0` + 각자 `OverrideColor`.
조합 WBP의 `MaskImage` 브러시가 이 MI. 색·채움·빗금 튜닝은 MI에서.

### 4.6 한계 / 메모
- 오버레이의 디자이너 미리보기(가짜 박스)엔 마스크가 없어 `MaskImage`가 숨겨진다 — 실루엣은 PIE에서. 마커 WBP 자체를 열면 `PreviewShape`의 가짜 사람 모양이 보인다.
- 마스크는 탐지 카메라 기준 투영이다. RCWS 메인뷰는 실제 게임 뷰포트(`PrimaryViewCamera`, 같은 가로 FOV)라 TSR 지터만큼(서브픽셀~1 px) 어긋날 수 있다 — 실측 필요.
- 대상의 숨김 메시·병사의 숨은 `BP_AR4Rifle` 등 `bHiddenInGame`은 원래 안 그려진다.
- 처음 요청 1~2프레임은 마스크가 없다(마커 생성 → 다음 갱신에 요청 → 그다음 캡처).

## 5. 검증 상태

- [x] 게임 타깃 컴파일 — 마커(1차) + 마스크(2차), 7절
- [x] 에디터 타깃 빌드(사용자, 2회 — 3차 변경 포함)
- [x] `MF_DetectionMaskCoverage` + `M_UI_DetectionSketch` 제작, Custom HLSL 컴파일 OK
- [x] 마커 WBP 14개 + MI 6개 + 3개 화면 오버레이 지정(기본 클래스 + 규칙 6행)
- [x] PIE(New_kadex_0811, **UGV 테스트 화면**): `Detection.MaskDiag 1`로 파이프라인 정상(3.5절), 박스·거리·반투명 실루엣 정상 표시
- [ ] PIE: Monitor1(드론)·Monitor2(트럭) 화면, **실루엣 정렬(RCWS 메인뷰)·겹친 대상 분리**, 갱신 모드 3종, 대상 사망·재시작 시 정리
- [ ] **LIG 영상 확인**: RTSP 수신 화면(`uav_gimbal`, RCWS 스트림)에 박스·실루엣이 **없는지**
- [ ] 비용: `stat gpu`/`ProfileGPU`로 마스크 캡처 ms(드론 1 + RCWS 1~2)
- [ ] 2-PC: 클라 프로세스에서만 마스크가 도는지

## 6. 남은 것 / 후속 후보

- **디자인(사용자)**: 디자이너 PNG를 `Textures/`에 넣고 각 WBP `BoxImage` 브러시로 지정 + **ColorAndOpacity 알파 올리기**, 조합별 `LabelText` 문구, MI 색·채움 튜닝.
- **검증**: 5절 미체크 항목(Monitor1/2 PIE, RTSP 누출 없음, `stat gpu`, 2-PC).
- 선택: `MinMarkerSize`(드론 32 / RCWS 64).
- **Tracking/Locked ↔ RCWS 자동조준 연동**: 별도 작업(코드 남아 있음, 2절).
- `Detection.MaskDiag`는 디버깅 끝나면 0(기본값이 0이라 콘솔로 켰을 때만 해당).
- P4: 신규 에셋 23개(`L_TD_Test.umap` 포함) `p4 add` 후 사용자 지시 시 제출(8절).
- 후보: 박스 위치 보간(0.1 s 계단), 같은 깊이 겹친 대상의 완전 분리(대상별 ID 인코딩), 레거시 위젯 `SetDetectionsFrom` 적용 여부.

## 7. 빌드 기록

- 2026-10-13(1차, 마커): 에디터(Live Coding)가 켜져 있어 **게임 타깃** `Build.bat titan_example Win64 Development`로 검증 —
  `Result: Succeeded`, 13 액션 186 s. 경고 6개는 전부 기존 `Variant_Combat/AI/CombatStateTreeUtility.h` C4996(무관), 변경 파일 경고 0.
- 2026-10-13(2차, 마스크): 같은 방식 `Result: Succeeded`. 새 경고 `DronePawn.cpp:1582`/`UAVPawn.cpp:1004` C4701(`ReferenceRot`) — 기존 코드, 무관.
- 2026-10-13(3차): 사용자가 **에디터를 닫고 에디터 타깃** 빌드 — `MarkerKindRules` USTRUCT, `FDetectionMarkerInfo.Kind`, `DistanceText` BindWidgetOptional,
  진영 필터, `SetLockState` 호출 제거, `MaskDiag`. 빌드 에러 보고 없음. (새 UCLASS/UPROPERTY라 Live Coding으로는 안 들어감)
- 2026-10-13(4차, 테스트 레벨): 사용자가 에디터 타깃 빌드 — `ADetectionTestDirector`(신규 UCLASS), GameMode `InitNewPlayer` 축 폴백,
  `UScenarioRespawnSubsystem::RespawnActor`. 에디터에 클래스 확인, 빌드 에러 보고 없음.

## 8. P4 상태 (미제출 — 사용자가 "제출"이라고 할 때만)

| 구분 | 파일 |
|---|---|
| edit | `Content/widget/WBP_SelfDefenseMonitor1.uasset`, `WBP_SelfDefenseMonitor2.uasset`, `WBP_UGVTestDashboard.uasset`; `TargetDetectionComponent.h/.cpp`, `DetectionOverlayWidget.h/.cpp`, `SelfDefenseMonitor1Widget.cpp`, `SelfDefenseMonitor2Widget.cpp`, `UGVTestDashboardWidget.cpp`; `titan_exampleGameMode.cpp`, `UI/ScenarioRespawnSubsystem.h/.cpp` |
| add | `DetectionMaskCaptureComponent.h/.cpp`, `DetectionMarkerWidget.h/.cpp`, `DetectionTestDirector.h/.cpp` |
| **아직 add 안 됨** | `Content/widget/target_detection/` 신규 에셋 **23개**(MF 1, M 1, MI 6, WBP 14, 레벨 `L_TD_Test.umap` 1) — 제출 전 `p4 add` 필요 |

## 9. 함정 (이번 작업에서 발견)

1. **MCP로 만든 ComponentMask는 기본 R=G=true** — 4채널을 전부 명시해야 한다. `M_UI_DetectionSketch`의 마스크 4개가 RG로 생성돼 빗금/미리보기가 틀렸음(수정함).
2. **MCP `MaterialTools.recompile` + 저장으로는 런타임/미리보기 셰이더가 갱신되지 않았다** — 상수 빨강을 출력해도 MaskImage가 불투명 검은 사각형으로 나옴.
   사용자가 머티리얼 에디터에서 **Apply**를 누른 뒤에야 정상. 절차: MCP로 머티리얼을 고친 뒤 머티리얼 에디터를 열고 Apply
   (MCP 편집 전부터 열려 있었다면 **닫았다 다시 열고** Apply — 아니면 옛 사본으로 덮어씀).
3. `BoxImage`에 처음엔 `M_UI_DetectionSketch` 브러시(RoundedBox)가 들어가 있었음 — 틀림. `BoxImage`는 프레임 PNG 자리이고 실루엣은 `MaskImage`. 14개 WBP 모두 비움.
4. **MCP 구조체 배열 쓰기**(Custom 노드 입력, `MarkerKindRules`)는 "elements changed alongside the size change"로 실패한다 — 읽어 온 기존 원소를
   **그대로** 넘기고 새 원소는 **한 번에 하나씩** 추가할 것.
5. **MCP로 액터 프로퍼티를 다른 맵에 통째로 복사하면 저장이 실패한다**(10절) — 원본 맵 컴포넌트를 가리키는 포인터 프로퍼티까지 복사돼
   `Illegal reference to private object ... (private object belongs to an external map)`. 원본 맵을 참조하는 오브젝트 참조 프로퍼티는 빼거나 리셋할 것.
6. **MCP `add_to_scene`의 `snap_to_ground`는 NavMeshBoundsVolume 윗면(z≈10 m)에 붙는다** — 테스트 레벨에선 Z를 직접 지정.

## 10. 디자인 확인용 테스트 레벨 `L_TD_Test`

목적: 디자이너가 전체 시나리오를 기다리지 않고 마커 WBP·실루엣이 대상 종류별로 어떻게 보이는지 바로 확인.

### 10.1 쓰는 법

1. `/Game/widget/target_detection/L_TD_Test` 열기
2. 아웃라이너에서 `DetectionTestDirector_0` 선택 → Details **Detection Test** → `Test Axis`(SelfDefense / UGV)
3. PIE. 실루엣 파이프라인이 의심되면 콘솔 `Detection.MaskDiag 1`(3.5절)

### 10.2 레벨 구성 (MCP로 구성, 빈 레벨은 사용자 생성)

| 항목 | 내용 |
|---|---|
| GameMode | WorldSettings = `BP_KadexTestGameMode`. `AScenarioConfig` **없음** → 시나리오 안 돎, 적군이 처음부터 보임(브리지는 ScenarioConfig가 시킬 때만 적군을 숨김) |
| 라이팅 | `/Game/New_kadex_0811`에서 복사, 값 동일(diff 없음 확인): DirectionalLight, HDRIBackdrop(+Skylight), ExponentialHeightFog, PostProcessVolume(Unbound, EV10 노출), WindSource |
| 지면 | 엔진 Cube 스케일 (1000,1000,1), 윗면 z=0, 머티리얼 `/Game/SoldierLab/Materials/MI_Ground_MossyGrass` |
| 내비 | NavMeshBoundsVolume 중심 (35 m, 0), 스케일 (60,40,10) |

배치(트럭 = 원점, +X를 바라봄, 단위 m). 병사 Z = 캡슐 반높이 86 + 2 cm:

| 대상 | 클래스 | 위치 |
|---|---|---|
| 트럭 | `BP_TitanTruck` | (0, 0) |
| UGV | `BP_UGV_0901` | (15, 25) |
| 드론 | `BP_Drone` | (−15, −15) 지상 출발 |
| 아군 ×6 | `BP_Soldier_Friendly` | x=30, y=−15..15 (6 m 간격) |
| 적군 ×6 | `BP_Soldier_Hostile` | x=70, 트럭 쪽을 바라봄 |
| 낙하산 ×2 | `BP_Parachute` | (50, −30), (42, 32) |
| 드론 경로 | DroneFlightPath `TD_Hover` | (49, 0, 60) — 기본 1 m 스플라인, 끝점 = 호버 지점 |

드론 인스턴스: `MinGimbalPitchDegrees` −90, `GimbalHomePitchDeg` −90. `ParachuteActor`가 비어 있어 정찰이 Idle에 머물고 짐벌이 −90°를 유지 —
"ParachuteActor가 비어 있어…" 경고 로그는 **정상**.

### 10.3 `ADetectionTestDirector` (`Detection/DetectionTestDirector.h/.cpp`, 서버 전용)

| 프로퍼티 | 기본값 | 동작 |
|---|---|---|
| `TestAxis` | SelfDefense | PIE 전에 고름. GameMode `InitNewPlayer`가 URL `?Axis=`가 없을 때 `FindTestAxis`로 이 값을 씀 |
| `bTruckAutoSurveillance` | true | 트럭 RCWS → `ERCWSControlMode::AutoSurveillance`(스윕만, 사격 없음) |
| `DroneFlightPathId` | `TD_Hover` | `DroneFollowPath` → 끝점에서 호버 |
| `bDroneDetectAllFactions` | true | `SetDetectionPhase(All)` — 시나리오 없을 때 드론 기본 단계는 FriendlyOnly |
| `SetupDelaySeconds` | 1.0 | 위 설정을 거는 지연 |
| `bRespawnDeadSoldiers` / `RespawnDelaySeconds` | true / 3.0 | 0.5 s마다 병사(`USoldierHealthComponent`) 폴링, 죽은 지 지연 이상이면 `RespawnActor` |

`UScenarioRespawnSubsystem::RespawnActor(OldActor)`: 스냅샷 중 `Live == OldActor`인 항목을 찾아 그 액터 + 부착/소유 액터를 파괴하고 `RespawnOne`
(AutoPossessAI 보정, authored 델타 복원). 새 액터 반환 — 스냅샷에 없거나 일괄 재스폰 중이면 nullptr.

### 10.4 PIE 검증 (2026-10-13)

| 항목 | 결과 |
|---|---|
| 자체방호 축 | `InitNewPlayer` PlayerAxis=2, 드론 Owner = PC |
| 트럭 RCWS | `[DetectionTest] BP_TitanTruck_C_0 RCWS → 자동 감시`, CurrentMode=AutoSurveillance. 스윕 중 Hostile/Friendly/Parachute 번갈아 탐지 |
| 드론 | `탐지 단계 → 전부`, `TD_Hover` 도착, 위치 ≈(50, 0, 60 m). 12 s 동안 SoldierFriendly·SoldierHostile·TitanTruck·UGV_0901·Parachute 탐지(Drone 외 전부) |
| 재스폰 | `1인 재스폰 TD_Hostile_N → ...` 로그가 몇 초마다 반복 |
| UGV 축 | PlayerAxis=1은 되지만 대시보드가 안 뜸 — `BP_TestPlayerController.bCreateUGVTestDashboard`=false(UGV 모드 UI는 설계상 꺼 둠). 그래서 기본값이 SelfDefense |

### 10.5 알려진 점

- **아군은 무적**(`BP_Soldier_Friendly`에 `bInvincible` authored) → 적군만 죽고 재스폰. 양쪽 다 죽게 하려면 레벨의 아군 인스턴스 `AC_SoldierHealth.bInvincible` 끄기.
- **`Drone` 종류 마커는 이 레벨에서 안 나온다** — 드론이 하나뿐이고 RCWS는 수평으로 스윕.
- 병사는 기본 ROE대로 이동·엄폐한다(정지 마네킹 아님). 적군이 차량/드론을 쏠 수 있음(짧은 테스트에선 문제 없었음).
- 병사에서 `LogAbilitySystem: Error: SendGameplayEventToActor ... GameplayEvent.ReloadDone` 스팸 관찰 — 원인 미조사.
