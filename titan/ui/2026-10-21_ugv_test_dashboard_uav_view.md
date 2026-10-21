# UGV 테스트 대시보드에 UAV(드론) 짐벌 영상 표시

2026-10-21 / 진행중(빌드·WBP 배치 대기) / UGV 테스트 대시보드의 QuadCam 4분할 자리에 드론 짐벌 영상(+탐지 오버레이)을 띄울 수 있게 `UAVViewImage`/`UAVViewDetectionOverlay` 슬롯과 드론 쪽 "로컬 화면 캡쳐 요청" API를 추가.

관련 문서:
- 같은 대시보드의 직전 작업(고정 해상도 스트레치): [`2026-10-15_ugv_test_dashboard_stretch_with_fixed_viewport.md`](2026-10-15_ugv_test_dashboard_stretch_with_fixed_viewport.md)
- 드론 짐벌 카메라 구조: [`../vehicle/drone/drone_flight_dev_guide.md`](../vehicle/drone/drone_flight_dev_guide.md) §12(짐벌 카메라), §15(리플리케이션 — 시뮬 주체, 원격 보간, RTSP)
- WBP 이름 표: [`kadex_test_dashboard_wbp_spec.md`](kadex_test_dashboard_wbp_spec.md) §2

---

## 1. 요구

사용자 의도: UGV 테스트 대시보드(WBP 부모 `UUGVTestDashboardWidget`)에서 **UGV QuadCam 4분할
자리에 UAV(드론 `ADronePawn`) 화면을 띄운다.** QuadCam은 화면에서 빠져도 된다.

문제: 드론 짐벌 캡쳐는 원래 **자체방호축 프로세스에서만** 돌았다 — 다른 축에서는 BeginPlay의 축 판정
콜백(`DronePawn.cpp` ~276행)이 `bDisableGimbalCapture = true`로 꺼 버린다. 그래서 UGV 호스트에서는
짐벌 렌더 타깃이 비어 있다.

## 2. 변경 (C++, 빌드 전, P4 opened·미제출)

### 2.1 `Source/titan_example/Drone/DronePawn.h/.cpp`

- 신규 public `void SetGimbalCaptureRequestedByLocalDisplay(bool bRequested)` + private
  `int32 LocalGimbalDisplayRequests`(요청 수 카운트 — 등록/해제 짝).
- `UpdateGimbalCapture()` 게이트:
  `if (bDisableGimbalCapture) return;` → `if (bDisableGimbalCapture && LocalGimbalDisplayRequests <= 0) return;`
  - 즉 **자체방호축이 아닌 프로세스(UGV 호스트)에서도 로컬 화면이 요청하면 짐벌 캡쳐가 돈다.**
  - `bDisableGimbalCapture` 자체는 그대로 두고 게이트에서 OR 조건으로만 보기 때문에, 축 판정 콜백과
    대시보드 등록의 **호출 순서와 무관**하다(어느 쪽이 먼저 와도 결과 동일).
- 로그: `[Drone] 로컬 화면 짐벌 영상 요청 등록/해제 — 요청 수 N, 축 판정 캡쳐 끔=0/1.`

### 2.2 `Source/titan_example/UI/UGVTestDashboardWidget.h/.cpp`

- 신규 `BindWidgetOptional`:
  - `UAVViewImage` (Image)
  - `UAVViewDetectionOverlay` (Detection Overlay, `UAVViewImage`와 같은 위치/크기)
- `NativeConstruct`: `GetActorOfClass(ADronePawn)` → `CachedDrone`.
  **`UAVViewImage`가 WBP에 있을 때만** `SetGimbalCaptureRequestedByLocalDisplay(true)` — 없으면 캡쳐 비용 0,
  기존 동작 그대로. 드론을 못 찾으면 Warning
  `[UGVTestDashboard] UAVViewImage가 있지만 레벨에서 ADronePawn을 못 찾음`.
- `NativeDestruct`: 짝 맞춰 `SetGimbalCaptureRequestedByLocalDisplay(false)`.
- `BindCameraImages`: 드론의 `CameraRenderTarget`을 `M_SceneCaptureDisplay` MID로 `UAVViewImage`에 바인딩.
- `RefreshSmoothValues`(매 프레임): `UAVViewDetectionOverlay->SetDetectionsFrom(Drone->TargetDetection)`
  — 다른 탐지 오버레이와 같은 매 프레임 경로(`guide/ui_dev_guide.md` 2026-10-14 노트 참고).
- 신규 `RefreshUAVViewResolution()`(`RefreshInterval`마다): 이미지 픽셀 크기로
  `SetCameraRenderTargetPixelSize` 후 재바인딩.
  - **고정 해상도 모드에서도 리사이즈한다** — UGV축에선 짐벌 RT가 RTSP 소스가 아니기 때문
    (드론 RTSP `selfdefense/uav_gimbal`은 자체방호축에만 생성됨, `drone_flight_dev_guide.md` §15.5).

## 3. 검토 결론

### 3.1 시뮬 주체·복제 — 호스트 영상이 맞게 나오는가

- 시뮬 주체는 **세션 시작 시 데모/풀 모드로 고정**된다(`ResolveShouldSimulateDrone`, 클라 접속으로
  넘어가는 핸드오버 없음). 데모 = 호스트, 풀 = 자체방호 클라.
- 어느 쪽이든 **호스트의 드론 복제본은 `TickRemoteInterpolation`이 위치·`RepGimbalYaw/Pitch`·`RepZoomLevel`을
  반영하고 짐벌 본까지 갱신**하므로, 호스트에서 캡쳐한 영상의 시점·줌이 맞다.
- 대시보드는 **표시 전용**이라 조종권과 충돌 없음.
- 한계:
  - 풀 모드에선 30Hz 보간 기반이라 클라 원본 영상보다 약간 덜 부드러울 수 있다.
  - 풀 모드에서 **자체방호 클라 접속 전에는 드론이 출발 위치에 떠 있는 화면**이 나온다(시뮬 주체가 클라라서).

### 3.2 성능

- **QuadCam을 화면에서 빼도 GPU 이득은 거의 없다.** QuadCam 캡쳐는 위젯 표시와 무관하게
  `bAlwaysVisible || bVisible`이면 매 틱 4대 중 1대씩 돈다 — RTSP CCTV 4스트림이 그 캡쳐를 쓰고,
  RTSP 쪽엔 접속자 없으면 끄는 처리가 없다.
- 짐벌 캡쳐는 `GimbalRoundRobinCount=2` / `Slot=1`이라 **2프레임에 1회 풀 씬 렌더가 추가**된다
  (S1 기준 캡쳐 고정비 ~2.4 ms — `infra_architecture/` S1 캡쳐/RTSP 비용 분석). 홀수 프레임만 무거워지는
  **불균일 프레임 타임**이 생길 수 있다.
- 부담되면: 그래픽 설정의 QuadCam `CaptureEveryNTicks` 상향(사용자 계획) 또는 드론 `GimbalRoundRobinCount` 상향.
- **UGV 호스트 GPU 여유는 미계측** — 빌드 후 A/B 필요(§4).

### 3.3 미확인

- `BP_UGV_0901`의 QuadCam `bAlwaysVisible` BP 값은 MCP 연결 실패로 확인 못 함.

## 4. 사용자 남은 작업

1. **에디터 닫고 정식 빌드** — 새 `UPROPERTY`(BindWidgetOptional 2개)·멤버 추가라 Live Coding 불가
   (Live Coding이면 BP에서 missing property가 남).
2. **WBP 배치**(UGV 테스트 대시보드 WBP, 부모 `UUGVTestDashboardWidget`): QuadCam 이미지 자리에
   - Image **`UAVViewImage`**
   - Detection Overlay **`UAVViewDetectionOverlay`** — `UAVViewImage`와 같은 위치/크기로 겹쳐서
   - 이름은 대소문자까지 정확히. QuadCam 4개 Image(`QuadCam*Image`)는 빼도 된다(Optional).
3. **확인 로그**(UGV 호스트):
   - `[Drone] 로컬 화면 짐벌 영상 요청 등록 — 요청 수 1, 축 판정 캡쳐 끔=1.` 이 나오면 정상(UGV축에선 축 판정이 끄고, 요청으로 다시 켜진 상태).
   - `[UGVTestDashboard] UAVViewImage가 있지만 레벨에서 ADronePawn을 못 찾음` 이 나오면 레벨에 드론 없음.
   - 대시보드 닫힐 때 `해제 — 요청 수 0`.
4. **화면 확인**: 데모 모드(호스트 시뮬)와 풀 모드(자체방호 클라 접속 전/후) 각각에서 짐벌 영상·줌·탐지 박스.
5. **성능 A/B**: UGV 호스트에서 `stat gpu` / `stat unit`, `UAVViewImage` 있는 WBP vs 없는 WBP.
   특히 짐벌 라운드로빈으로 인한 홀짝 프레임 불균일 여부.
