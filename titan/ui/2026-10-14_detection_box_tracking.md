# 탐지 박스 추적(칼만) + 화면 가장자리 마커 누출 수정

2026-10-14 / 완료 / 탐지 박스 계단 이동의 진짜 원인(화면 위젯의 0.1 s 패널 스로틀)을 고치고, 오버레이에 대상별 등속 칼만 추적기를 넣어 박스가 매 프레임 매끄럽게 따라가게 함 + 0..1로 잘린 UV/1.2 여유 때문에 마커가 화면 가장자리에 붙거나 글자만 남던 문제 수정. 사용자 빌드 후 PIE "잘 작동함", P4 Change 45로 제출.

선행 문서: [`ui/2026-10-13_detection_marker_wbp.md`](2026-10-13_detection_marker_wbp.md) (마커 WBP·실루엣 마스크). 이 문서는 그 후속이다.

---

## 0. 요약

| 항목 | 내용 |
|---|---|
| 증상 1 | 탐지 박스가 대상/카메라를 매끄럽게 못 따라가고 0.1 s마다 계단식으로 이동 |
| 증상 2 | 대상이 화면 왼쪽 밖으로 나갈 때 라벨이 안쪽으로 밀려 들어오다 한 번에 사라짐 / 박스는 없는데 라벨만 남음 |
| 수정 | ① `SetDetectionsFrom` 호출을 매 프레임 경로로 이동(3개 화면) ② 마커 글자 값 별도 스로틀 ③ 오버레이 박스 추적기(CV 칼만) ④ 자르지 않은 UV(`ScreenMin/MaxUVUnclamped`) + 화면과 안 겹치면 숨김 + 오버레이 Clip to Bounds |
| 탐지 판정 | **불변** — 대상 목록·진영·Confidence는 그대로 스캔 기준. RCWS 조준/판정은 계속 잘린 `ScreenMin/MaxUV` 사용 |
| 빌드 | 헤더(UPROPERTY/USTRUCT 필드) 변경 → 사용자가 에디터 닫고 정식 빌드 |
| 검증 | 사용자 PIE "잘 작동함" |
| P4 | Change 45 제출 완료(7절) |

## 1. 계단 이동의 진짜 원인 — 화면 위젯 스로틀

- 처음엔 탐지 스캔 주기 탓으로 보고 `UTargetDetectionComponent::ScanIntervalSeconds`를 0으로 해 봤다. 스캔은 실제로 매 틱 돌았다
  (`TargetDetectionComponent.cpp` `TickComponent` — `TimeSinceLastScan < ScanIntervalSeconds`면 return). **그런데도 박스는 계단식이었다.**
- 원인: 세 화면 위젯이 `SetDetectionsFrom`을 `RefreshPanel()` 안에서 부르고 있었고, `RefreshPanel()`은 `RefreshIntervalSeconds` = **0.1 s**로
  스로틀된다. 스캔을 아무리 빨리 돌려도 오버레이는 0.1 s마다만 새 박스를 받았다.
- 수정: 호출을 매 프레임 도는 `RefreshSmoothValues()`로 옮김. 패널 텍스트는 그대로 0.1 s.

| 화면 | 오버레이 | 탐지기 |
|---|---|---|
| `USelfDefenseMonitor1Widget` (드론) | `UAVViewDetectionOverlay` | 드론 `TargetDetection` |
| `USelfDefenseMonitor2Widget` (트럭 RCWS) | `MainViewDetectionOverlay` | 트럭 `TargetDetection` |
| `UUGVTestDashboardWidget` | `MainViewDetectionOverlay` | `CachedTargetDetection` — **`if (!CachedRCWS) return;` 앞**에 둠(RCWS 캐시가 없어도 박스는 갱신) |

## 2. 마커 글자 스로틀

위치를 매 프레임 갱신하면 거리 숫자도 매 프레임 바뀌어 읽기 힘들고 텍스트 무효화가 잦다. 그래서 값만 따로 늦춘다.

- 오버레이 신규 UPROPERTY **`MarkerTextRefreshIntervalSeconds`**(기본 0.1, 카테고리 `Detection Overlay|Marker`, 0이면 매 갱신).
- 마커 위치·크기·실루엣은 매 프레임, `DistanceText` **값**만 이 주기.
- 처음 배정된 마커는 바로 채움. 디자이너 미리보기는 항상 갱신.
- `UDetectionMarkerWidget::ApplyMarkerInfo(NewInfo, bFirstAssignment, bRefreshText = true)` — `bRefreshText=false`면 C++가 채우는 글자를 안 건드림.

## 3. 박스 추적 (오버레이 쪽 CV 칼만)

### 3.1 설계 결정 (사용자와 합의)

- **관찰자(카메라) 움직임은 정확히 따라가야 한다.** 실제 시스템은 거리계로 거리를 알므로 대상의 월드 위치를 안다 → 카메라가 돌거나 줌해도 박스가 오차 없이 붙는다.
- **대상 자신의 움직임은 스캔 결과로만 추정**한다(스캔 사이엔 예측, 스캔이 오면 보정) — "추적기" 느낌.

| 후보 | 판단 |
|---|---|
| 지연 보간(두 스캔 사이 보간) | 기각 — 지연이 한 스캔만큼 늘어남 |
| 매 프레임 실제 위치 재투영(A안) | 기각 — 정답을 그냥 그리는 것이라 추적기 같지 않음 |
| α-β 필터 | 검토 |
| **등속(CV) 칼만** | **채택** — SORT/ByteTrack 계열의 표준 |
| IMM | 과함(10 Hz 스캔엔 이득 없음) |

### 3.2 구현

**탐지기 쪽** (`TargetDetectionComponent.h/.cpp`):
- 스캔마다 `FScanView`(카메라 위치/회전/FOV/화면비/월드 시각, `bValid`)를 기록 → `GetLastScanView()`.
- 공개 `GetProjectionAspectRatio()`(= `ResolveAspectRatio()`) — 스캔 밖에서 같은 투영을 재현할 때.

**오버레이 쪽** (`DetectionOverlayWidget.h/.cpp`, `SetDetectionsFrom` 경로 전용):
1. 매 호출마다 **현재 카메라**(위치/회전/tan(FOV/2)/화면비/월드 시각)를 잡음. 탐지기가 바뀌면 추적 초기화.
2. `GetCompletedScanCount()`가 바뀌었을 때만 `UpdateBoxTracks` — 새 스캔 = 새 측정.
3. **측정**: 박스 중심 UV(자르지 않은 값)를 **스캔 시점 카메라**로, 대상 루트 바운드 중심의 view 깊이만큼 역투영 → 월드 위치.
   이 점을 스캔 카메라로 다시 투영하면 정확히 스캔 박스 중심이라 스캔 순간 박스가 안 튄다. 박스 크기도 스캔 카메라로 **cm 단위**로 저장.
4. **필터**: 축별 독립 CV 칼만(이산 백색 가속도 공정잡음 Q = `TrackManeuverAccelCmPerSec2`², 측정잡음 R = `TrackMeasurementNoiseCm`²).
   - 새 추적은 측정 위치·속도 0·속도 분산 큼(`TrackMaxSpeedCmPerSec`²) → 다음 측정 두어 번에 속도가 잡힘.
   - 속도는 `TrackMaxSpeedCmPerSec`로 클램프.
   - 보정 순간의 화면 점프는 **지수 감쇠 오프셋**(`TrackCorrectionBlendSeconds`)으로 흡수.
   - 크기는 `TrackSizeResponse`로 lerp.
5. 이번 스캔에 없는 대상의 추적은 즉시 삭제 → **박스가 뜨고 사라지는 건 지금처럼 스캔(탐지 판정) 그대로.**
6. 매 프레임 `ProjectTrackBox`: 표시 위치 = 추정 위치 + 속도 × min(경과, `TrackMaxExtrapolationSeconds`) + 감쇠 오프셋 → **현재 카메라**로 투영(0..1로 자르지 않음).
7. 표시 거리 = 추적 위치까지 거리(추적기가 없으면 예전처럼 카메라↔액터).
8. `SetDetections`(레거시, 카메라 모름) 경로는 추적 없이 스캔 박스 그대로.

### 3.3 튜닝 프로퍼티 (`Detection Overlay|Tracking`)

| 프로퍼티 | 기본값 | 뜻 |
|---|---|---|
| `bTrackBoxes` | true | 끄면 예전처럼 스캔 박스를 그대로 그림 |
| `TrackManeuverAccelCmPerSec2` | 400 | 반응성(클수록 측정을 빨리 따라감, 덜 매끄러움) |
| `TrackMeasurementNoiseCm` | 15 | 측정을 얼마나 믿나(클수록 부드럽고 늦음) |
| `TrackMeasurementJitterCm` | 0 | 측정에 실제 잡음을 섞는 옵션("진짜 모델" 흉내) |
| `TrackMaxExtrapolationSeconds` | 0.2 | 스캔 사이 예측 상한 |
| `TrackCorrectionBlendSeconds` | 0.06 | 보정 점프 흡수 시간상수(0이면 즉시) |
| `TrackMaxSpeedCmPerSec` | 3000 | 추정 속도 상한 |
| `TrackSizeResponse` | 0.5 | 스캔마다 크기 lerp 비율 |

권장 `ScanIntervalSeconds`: **0.1 그대로**(추적기가 그 사이를 메운다).

### 3.4 한계

- 실루엣(정답 마스크)은 매 프레임 실제 위치라 마커 사각형 안에서 보인다. 대상이 **급정지**하면 박스가 잠깐 앞서 나가/뒤처져 실루엣 가장자리가 잘릴 수 있다 — `MarkerPadding`으로 완화.

## 4. 마커가 화면 가장자리에 붙거나 글자만 남던 문제

증상: 대상이 왼쪽으로 화면을 벗어날 때 라벨이 안쪽으로 밀려 들어오다 한 번에 사라짐. 박스는 없는데 라벨이 남음.

원인 두 겹:
- **(a)** `FDetectedTarget::ScreenMin/MaxUV`는 `ProjectWithViewProjection`이 0..1로 잘라서 넣는다 → 마커 사각형이 화면 가장자리에 붙어 줄어들고, Clip to Bounds가 잘라 낼 게 없었다.
- **(b)** 탐지기는 코너 NDC ±1.2까지 "화면 안"으로 친다(조준·판정용 20% 여유). 박스 전체가 그 띠 안에 있으면 잘린 박스는 폭 0이 되는데 대상은 계속 탐지됨 → 고정 크기 라벨만 보였다.

수정:
- 신규 `FDetectedTarget::ScreenMinUVUnclamped/ScreenMaxUVUnclamped` — 카메라 앞에 있는 코너 전부를 신규 static `ProjectWithViewProjectionUnclamped`로 투영해 구한 자르지 않은 박스.
- 기존 잘린 필드는 **그대로** → RCWS 조준/판정 영향 없음.
- 오버레이는 마커·선 박스 배치와 추적기 측정을 자르지 않은 값으로 한다. 값이 비어 있으면(Min == Max, BP가 직접 만든 항목 등) 잘린 값으로 폴백.
- 오버레이는 박스가 [0,1]과 **전혀 안 겹치면 마커를 숨긴다** — 탐지기의 1.2 여유와 별개인 엄격한 표시 규칙.
- 사용자가 화면 WBP의 오버레이에 **Clipping = Clip to Bounds** 설정.
- 메모: 박스 위에 붙은 라벨은 화면 위쪽 가장자리에서 먼저 잘린다 — 라벨 레이아웃은 디자인과 함께 결정 예정.

## 5. 함정

1. **UI 스로틀이 스캔 주기 변경을 가렸다.** `ScanIntervalSeconds=0`으로 스캔은 매 틱 돌았지만, 화면 위젯이 `RefreshPanel()`(0.1 s)에서만 오버레이에 넘겨 겉보기엔 변화가 없었다.
   "주기를 바꿨는데 그대로"면 데이터 생산자만 보지 말고 **소비자의 갱신 경로/스로틀**까지 따라갈 것.
2. **잘린 UV + 1.2 여유 = 화면 밖 대상의 유령 마커.** 판정용 0..1 클램프 박스로 표시를 배치하면 마커가 가장자리에 붙고, 판정 여유(±1.2) 안에서는
   폭 0 박스 + 라벨만 남는다. 표시는 자르지 않은 값 + 별도의 엄격한 화면 겹침 판정 + Clip to Bounds로.

## 6. 바뀐 파일

| 파일 | 내용 |
|---|---|
| `Detection/DetectionTypes.h` | `FDetectedTarget::ScreenMinUVUnclamped/ScreenMaxUVUnclamped` |
| `Detection/TargetDetectionComponent.h/.cpp` | `ProjectWithViewProjectionUnclamped`, 스캔 결과에 unclamped 박스, `FScanView`/`GetLastScanView()`, `GetProjectionAspectRatio()` |
| `UI/DetectionOverlayWidget.h/.cpp` | 박스 추적기(`FBoxTrack`, `UpdateBoxTracks`/`ProjectTrackBox`/`GetTrackDisplayPosition`/`ResetBoxTracks`), Tracking 프로퍼티 8개, `MarkerTextRefreshIntervalSeconds`, unclamped 배치 + 화면 밖 숨김 |
| `UI/DetectionMarkerWidget.h/.cpp` | `ApplyMarkerInfo(..., bRefreshText)` |
| `UI/SelfDefenseMonitor1Widget.cpp`, `SelfDefenseMonitor2Widget.cpp`, `UGVTestDashboardWidget.cpp` | `SetDetectionsFrom`을 `RefreshSmoothValues()`로 이동 |
| `Content/widget/WBP_SelfDefenseMonitor1/2.uasset` | 오버레이 Clip to Bounds |
| `Content/Vehicles/Titan/BP_TitanTruck.uasset` | Change 45에 포함(이 세션에서 바뀐 값은 확인 안 함) |

## 7. 빌드 / P4

- 빌드: 헤더 리플렉션 변경이라 Live Coding 불가 → 사용자가 에디터 닫고 정식 빌드, PIE "잘 작동함".
- P4: **Change 45**(2026-10-14 00:34, "객체인식 보간 적용, 오버레이 밖으로 튀어나오는 문제 해결") 제출 확인(`p4 describe -s 45`) — 위 6절의 소스 13개 + `BP_TitanTruck.uasset`, `WBP_SelfDefenseMonitor1/2.uasset`.
  `WBP_UGVTestDashboard.uasset`은 Change 45에 **없다** — UGV 테스트 화면 오버레이의 Clip to Bounds 설정 여부는 확인 안 함.
- 선행 마커 작업은 Change 43·44(2026-10-13).

## 8. 남은 것

- 라벨 레이아웃(위쪽 가장자리에서 먼저 잘림) — 디자인과 함께.
- 선행 문서 5·6절의 미검증 항목(RTSP 누출 없음, `stat gpu`, 2-PC 등)은 그대로.
- 디자이너 마커 디자인 12종 요청 + MI 튜닝 가이드: [`ui/2026-10-14_detection_marker_mi_designer_guide.html`](2026-10-14_detection_marker_mi_designer_guide.html)(디자이너 메일 발송용).
