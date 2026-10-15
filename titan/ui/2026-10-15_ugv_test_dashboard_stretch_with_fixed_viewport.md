# UGV 테스트 대시보드 — 고정 해상도 뷰포트에서 3D와 같이 축별 스트레치

2026-10-15 / 완료(2026-10-15 사용자 빌드·실측 확인 — 스트레치 + §5.1 실루엣 마스크 화면비) / 고정 해상도 모드에서 UMG 대시보드가 창 리사이즈를 안 따라가고 잘리던 문제를, WBP 루트를 Slate 래퍼로 감싸 3D와 같은 축별 배율을 걸어 해결.

관련 문서:
- 렌더 방식 정본: [`../rtsp/rtsp_resolution_customization_0820.md`](../rtsp/rtsp_resolution_customization_0820.md) §5-1 "최종 형태"
- 같은 날 축 선택 화면 대시보드 체크박스: [`2026-10-15_axis_selection_ugv_dashboard_toggle_and_saved_choices.md`](2026-10-15_axis_selection_ugv_dashboard_toggle_and_saved_choices.md)
- 같은 파일을 먼저 연 작업(경로 미리보기 오버레이): [`../vehicle/ugv/2026-10-15_ugv_path_preview_overlay.md`](../vehicle/ugv/2026-10-15_ugv_path_preview_overlay.md)

## 1. 배경

UGV축은 원래 "UI 없이 RTSP 송출만" 전제였는데, 전시에서 UGV 모드로 직접 플레이될 가능성이 생겨
테스트 대시보드(WBP, 부모 `UUGVTestDashboardWidget`)를 다듬는 중이다. 같은 날 축 선택 화면에
"대시보드 표시" 체크박스도 추가됐다(위 관련 문서).

렌더 방식(2026-08-20 확정, rtsp 문서 §5-1):
- `Utitan_exampleGameEngine`이 게임 뷰포트를 `RenderDirectlyToWindow(false)`(전용 RT)로 만들고
  `FSceneViewport::SetFixedViewportSize`를 건다.
- 씬은 고정 해상도(`UStreamResolutionSubsystem`의 RCWS 해상도, 기본 1920x1080)로 렌더되고,
  Slate가 그 RT를 **창 크기로 축별로 늘여** 그린다. PIE는 원래 전용 RT 경로.

## 2. 증상

WBP가 완전히 고정 크기로만 나온다. 창을 줄이면 대시보드가 같이 줄지 않고 **잘린다**(3D는 창에
맞춰 늘고 줄어드는데 UI만 안 따라감).

## 3. 원인 (엔진 소스, UE 5.8)

- UMG의 DPI 스케일은 `SGameLayerManager::GetGameViewportDPIScale`이 정한다 — 기준이
  `Viewport->GetSize()`(`SGameLayerManager.cpp:477`, `489`).
- `SetFixedViewportSize`는 `bForceViewportSize=true`를 세운다(`SceneViewport.cpp:1879~1888`) →
  `GetSize()`가 창이 아니라 **고정값**을 돌려준다 → DPI 스케일이 창 크기와 무관하게 불변 →
  위젯 배율이 고정.
- 반면 위젯이 배치되는 영역은 **실제 창**이다. 그래서 "배율은 고정, 영역은 창" → 창이 작아지면 잘림.

## 4. 기각한 대안

| 대안 | 기각 이유 |
|---|---|
| `ScaleBox` | `Stretch=Fill`은 `FinalScale=1`로 레이아웃만 재배치한다(`SScaleBox.cpp:199~201`) — 배율이 안 붙음. 나머지 모드는 **균일 배율**뿐. |
| Slate 레이아웃 트랜스폼 | 역시 균일 배율만 지원. |
| 균일 배율(비율 유지) 자체 | 3D는 축별로 늘어나므로, 균일 배율이면 3D 위에 겹친 레티클/탐지 박스가 3D와 어긋난다. 비추천. |

## 5. 해결

`UUGVTestDashboardWidget::RebuildWidget` 오버라이드 — `Super::RebuildWidget()` 결과를 파일 로컬
Slate 위젯 **`SUGVFixedViewportStretch`**(`SCompoundWidget`)로 감싼다.

- `OnArrangeChildren`에서 고정 모드(`FSceneViewport::HasFixedSize()`)면:
  - `Scale = (위젯 영역의 절대 픽셀 크기 / 고정 해상도)` — **축별**(X, Y 따로).
  - 자식 로컬 크기 = 로컬 크기 / Scale.
  - `FSlateRenderTransform(FScale2f(Scale))`, 피벗 (0,0)으로 배치.
  - 즉 자식은 **"창 = 고정 해상도"였을 때의 레이아웃**으로 배치되고, 3D와 **동일한 변환**이 붙는다.
- 고정 모드가 아니면 기존처럼 꽉 채움(래퍼가 아무것도 안 함).
- 디자이너(`IsDesignTime()`)에서는 감싸지 않음.
- 렌더 트랜스폼은 Slate 히트테스트가 반영하므로 버튼 클릭 위치도 같이 맞는다(이론상. 2026-10-15
  사용자 실측은 스트레치 "성공"까지만 — 클릭 위치를 따로 언급하진 않음).

스위치: `UPROPERTY bStretchWithFixedViewport`(기본 `true`, Category `UGV Dashboard|Layout`) — 끄면 기존 동작.

확인 로그(배율이 1% 이상 바뀔 때만 출력):

```
[UGVTestDashboard] 고정 해상도 %dx%d → 창 %.0fx%.0f, 배율 X=%.3f Y=%.3f
```

### 5.1 부수 효과

- ~~마스크 크기가 렌더 해상도 기준 픽셀이 된다~~ — **틀린 분석이었다(같은 날 실측으로 정정).**
  `FGeometry::GetAbsoluteSize()`는 `AccumulatedRenderTransform`을 쓴다(`Geometry.h:548~550`) — 즉 렌더
  트랜스폼을 **포함**한다. 그래서 대시보드 안에서도 마스크 크기는 **창 픽셀 비율**로 요청됐다.
- **실측 증상(사용자, 2026-10-15)**: 대시보드 WBP는 잘 리사이즈되는데 TargetDetection 오버레이(실루엣
  마커)는 **창 비율이 렌더 해상도 비율과 딱 맞을 때만** 제대로 입혀짐.
- **원인**: 마스크 캡처(`UDetectionMaskCaptureComponent`)는 렌더 타깃 크기로 투영 화면비를 정한다
  (`TickComponent` 주석 "화면비 = 렌더 타깃 크기"). 탐지 투영은 렌더 해상도(16:9,
  `VehicleRtspBridgeComponent`의 `SetRenderedViewSize(Resolution)`)인데 마스크 RT는 창 비율 → 실루엣 어긋남.
  예전(표시 영역 비율 = 투영 비율)엔 같은 값이라 안 드러났다.
- **수정**: `UDetectionOverlayWidget::UpdateMaskRequest` — 해상도는 표시 픽셀 가로를 그대로 쓰되,
  세로를 `가로 / Detector->GetProjectionAspectRatio()`로 맞춤. 고정 모드가 아닌 축(자체방호 등)은
  두 비율이 원래 같아서 동작 불변. **2026-10-15 사용자 실측 확인**("잘돼").
- 오버레이 박스/조준점은 UV × 자기 로컬 크기로 그리므로 영향 없음.

### 5.2 의도된 트레이드오프

창 비율이 렌더 비율(16:9)과 다르면 글자/패널도 3D처럼 납작하거나 길쭉해진다. 3D 위 레티클·탐지 박스
정렬을 지키기 위한 선택.

### 5.3 미적용 범위

대시보드 밖에서 `AddToViewport`되는 다른 UGV축 화면 위젯은 같은 문제(고정 배율 + 잘림)를 가질 수
있다. 이번엔 대시보드만 처리.

## 6. 변경 파일 / P4

- `Source/titan_example/UI/UGVTestDashboardWidget.h` / `.cpp` — 이미 같은 날 경로 미리보기 오버레이
  작업으로 opened 상태였고, 그 위에 최소 Edit로 추가. **미제출.**

## 7. 남은 작업 (사용자)

> **2026-10-15 사용자 실측 확인** — 빌드 후 대시보드 스트레치 "성공", 실루엣 마스크(§5.1) "잘돼".
> 아래 2의 세부 항목(버튼 클릭 위치, 로그 배율 값, standalone/패키지별 확인)은 개별 언급이 없어 따로
> 확인됐다고 적지 않음.

1. 새 UPROPERTY라 **에디터 닫고 정식 빌드**(Live Coding 불가).
2. PIE와 standalone/패키지에서 창을 가로·세로로 리사이즈하며 확인:
   - 대시보드가 3D와 같이 늘고 줄며 잘리지 않는지
   - 버튼 클릭 위치가 맞는지
   - 레티클/탐지 박스가 3D와 정렬되는지
   - 위 확인 로그의 배율 값이 창/고정 비율과 맞는지
