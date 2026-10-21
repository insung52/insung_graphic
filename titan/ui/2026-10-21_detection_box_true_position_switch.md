# 탐지 박스 "실제 위치 스냅" BP 스위치 (`bSnapBoxesToTrueTarget`)

2026-10-21 / 진행중(빌드·검증 대기) / 오버레이 박스를 칼만 추적기 대신 매 프레임 대상 실제 바운드 투영으로 그리는 스위치 추가(기본 false = 기존 동작).

## 1. 배경

`ui/2026-10-14_detection_box_tracking.md` §3.1에서 "A안: 매 프레임 실제 위치 재투영"을 "정답을 그냥 그리는 것이라
추적기 같지 않다"는 이유로 기각하고 등속 칼만 추적기를 채택했다. 사용자가 두 방식을 고를 수 있게 해 달라고 요청해서
A안을 **스위치로** 추가했다. 기본값 false라 기존 동작은 바뀌지 않는다.

## 2. 동작

`UDetectionOverlayWidget::bSnapBoxesToTrueTarget` (`EditAnywhere, BlueprintReadWrite`, Category `Detection Overlay|Tracking`, 기본 false)

| 값 | 박스 위치 |
|---|---|
| false | 기존 칼만 추적기(`bTrackBoxes`를 따름) |
| true | 매 프레임 대상의 실제 바운드를 **현재 카메라**로 투영. 추적기·예측 없음, `bTrackBoxes`보다 우선 |

- `SetDetectionsFrom`: 스위치가 켜지면 `BuildCameraViewProjection`으로 현재 카메라 행렬만 잡고(`bHasTrueView`/`TrueViewProjection`/
  `TrueViewLocation`) 추적기는 `Reset`. 다시 추적 모드로 바꾸면 다음 스캔부터 새로 잡는다.
- `SetDetections`(레거시)·탐지기 null 경로는 `bHasTrueView=false` → 기존대로 스캔 박스.
- `BuildInfos`: `bHasTrueView`면 `UTargetDetectionComponent::ProjectActorVisualBoxUnclamped`로 박스 + 거리(바운드 중심↔카메라).
  대상이 파괴됐으면 스캔 박스, 카메라 뒤이거나 바운드가 없으면 숨김. 그 뒤 화면 겹침 판정·Clip to Bounds는 기존 그대로.
- 신규 `public static UTargetDetectionComponent::ProjectActorVisualBoxUnclamped(Actor, ViewProjection, OutMinUV, OutMaxUV, OutWorldCenter)` —
  `EvaluateTarget`과 같은 `CalculateVisualBoundsLocalSpace` 8코너 + `ProjectWithViewProjectionUnclamped`. 같은 카메라면 스캔의
  `ScreenMin/MaxUVUnclamped`와 같은 값.

**불변**: 탐지 판정(박스 출현·소멸, 페이드아웃)·Confidence·RCWS 조준(잘린 `ScreenMinUV`)·UDP 보고는 0.1 s 스캔 그대로.
**비용**: 대상당 프레임마다 컴포넌트 바운드 합산 + 8코너 투영.
**효과**: 10-14 문서 §3.4의 실루엣↔박스 어긋남(급정지 때)은 true 모드에선 없다.

## 3. 바뀐 파일 (`Source/titan_example/`)

- `UI/DetectionOverlayWidget.h/.cpp`
- `Detection/TargetDetectionComponent.h/.cpp`

## 4. 빌드 / P4 / 남은 것

- 헤더 UPROPERTY 추가 → **Live Coding 불가, 에디터 닫고 정식 빌드**(사용자).
- P4: 4파일 `p4 edit` 상태, 미제출.
- 검증 대기: false에서 기존 추적 동작 불변, true에서 박스가 실루엣에 붙는지, 토글 왕복 시 추적기가 다음 스캔부터 정상 재획득하는지.
