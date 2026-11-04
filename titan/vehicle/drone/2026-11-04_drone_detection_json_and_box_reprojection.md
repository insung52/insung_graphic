# 드론 BoundingBox JSON 생성 + UGV 객체인식 박스 재투영 (LIG 11/04 피드백 S2: D1~D5)

2026-11-04 / 구현 완료, 빌드·검증 대기(사용자 빌드) / 근거: `documents/feedback_1104/` (메일 2-2·2-3, ICD 1104 시트2 J14, `sample1_boxed_metadata.json`)

## 1. 바뀐 것 한눈에

| ID | 내용 | 파일 |
|---|---|---|
| D1 | 드론 RTSP 송출 해상도 640×360 → **1024×576** | `Config/DefaultGame.ini` `[/Script/titan_example.StreamResolutionSubsystem] SelfDefenseUavResolution` |
| D2 | UGV 객체인식 박스를 **송신 시점 카메라(+착탄 셰이크)로 재투영**, 화면 밖 대상 제외, 최대 10개(가까운 순), 송신 5→20Hz | `Network/UGVRemoteControlSubsystem.h/.cpp`, 신규 `Detection/DetectionFrameProjection.h/.cpp` |
| D3 | 객체 분류 공용화(enum + ICD 문자열/드론 classId·라벨 매핑) | 신규 `Detection/DetectionClassification.h/.cpp`, `UGVRemoteControlSubsystem.cpp`에서 기존 함수 제거 |
| D4 | 드론 BoundingBox JSON 생성(짐벌 캡처 1회 = 1프레임) | 신규 `Drone/DroneDetectionReporterComponent.h/.cpp`, `Drone/DronePawn.h/.cpp` |
| D5 | 전송 = S1 공용 통로 `USelfDefenseReportSubsystem::SendReportJson(DetectionReportPort, json, "DroneDetection")` | (D4 파일 안) |
| — | S1 K05.1용 `ADronePawn::GetFlightVelocityMS()` | `Drone/DronePawn.h/.cpp` |

## 2. UGV 박스 어긋남(D2) 원인과 수정

**원인**: `SendObjectDetectionResult`가 `FDetectedTarget::ScreenMin/MaxUV`를 그대로 보냈다. 이 값은 **마지막 탐지 스캔(`ScanIntervalSeconds` 0.1초) 시점의 카메라**로 계산된 것이고 송신이 5Hz라 박스 나이가 0~0.3초. 포탑 선회·줌·주행 중이면 그만큼 영상에서 밀린다(줌 FOV 5°, 20°/s 선회, 0.15초 = 화면 폭의 60%).
우리 화면에서 안 보였던 이유: `UDetectionOverlayWidget`은 매 프레임 칼만 추적/재투영으로 그린다. `rc_mock_client.py`는 박스 JSON을 로그로만 남기고 영상 위에 안 그린다 → **UDP 박스를 RTSP 영상에 겹쳐본 적이 없었다.**
부차 원인: 착탄 카메라 셰이크는 `PlayerCameraManager` 모디파이어라 메인뷰(=UGV축 RCWS RTSP)에만 실리고 탐지 카메라(`SightCamera`) 트랜스폼엔 없다.

**수정**: 대상 목록(시선·진영 필터 통과)은 스캔 결과를 그대로 쓰고, **박스 기하만** 송신 시점에 다시 투영한다.
- 카메라 = `SightCamera` 월드 트랜스폼 × `FSceneCaptureViewParity::GetLocalViewShakeOffset(UGV)` (`DetectionFrameProjection::GetRenderedViewTransform`). 메인뷰 카메라 `PrimaryViewCamera`는 `SightCamera`의 identity 자식이고 FOV가 매 틱 복사되므로 같은 시점.
- FOV = `SightCamera->FOVAngle`(수평), 화면비 = `Detection->GetProjectionAspectRatio()`(UGV축 고정 해상도 경로에선 1920×1080).
- 박스 = `UTargetDetectionComponent::ProjectActorVisualBoxUnclamped`(스캔·오버레이와 같은 8코너) → 화면과 안 겹치면 제외 → 0..1로 자름 → ×65535.
- ICD `TotalObject` 0~10 → 넘치면 카메라에서 가까운 순으로 10개.
- 비교 스위치 `bReprojectObjectDetectionBoxes`(기본 true, false = 예전 동작). 송신 주기 `ObjectDetectionSendHz` 20.
- RCWS(13)·ADU(12) 두 메시지 모두 같은 재투영 박스를 쓴다(ADU는 원래 RCWS 결과를 그대로 재사용하던 구조).

### 2-1. 화면비 일회성 전달 보강 (W5 F1, 2026-11-04 재분석 후 사용자 결정으로 이것만 진행)

계통 오차 가정으로 송신 박스 경로 전체를 다시 추적한 결과, 정상 고정 해상도 경로에서는 카메라·투영 수식·RTSP
복사 영역이 일치했다(박스 카메라 = 화면 카메라, 엔진 MaintainXFOV 투영과 같은 수식). 다만 같은 증상(세로 0.743배
압축, 화면 위·아래 끝에서 최대 약 138px)을 지금 코드에서도 재현할 수 있는 길이 하나 있었다.
`URCWSComponent::SetRenderedAspectRatio`는 `Detection->GetCamera() == SightCamera`일 때만 탐지 컴포넌트에 전달하는데,
UGV축에선 `UVehicleRtspBridgeComponent`가 이걸 **한 번만** 부른다. 그때 탐지 컴포넌트가 아직 카메라를 못 찾았으면
화면비가 영구히 미설정이 되고, 투영이 SightCamera 렌더타깃 화면비(BP 1226×928)로 폴백한다.
수정: `UTargetDetectionComponent::SyncRenderedAspectFromOwnerRCWS()`가 스캔마다 소유 RCWS(같은 SightCamera를 쓸 때만)의
`GetRenderedAspectRatio()`를 끌어와 다르면 맞춘다(정상 경로에선 같은 값이라 no-op). 보정이 일어나면 경고 1회
`화면비 X → RCWS 렌더 화면비 Y로 보정`. RCWS도 화면비를 모르는 채로 메인뷰 스트림(`bDisableSightCapture`)이면 약 3초 뒤
경고 1회 `메인뷰 스트림인데 렌더 화면비가 미설정`. `RCWSComponent.*`는 다른 세션이 편집 중이라 손대지 않고
탐지 컴포넌트 쪽에서만 구현했다.
남은 후보(LIG 표시측 매핑 — 레터박스 미보정 등, 엎드림 자세의 캡슐 바운드)는 이번에 손대지 않았다(사용자 결정).

## 3. 분류 공용화(D3)

`EDetectedObjectClass { Ally, Enemy, UGV, MobileCommandPost, Drone, Parachute }`.
판정 순서(EnemyEvidence → Character → TitanTruck → Drone/UAV → WheeledVehicle → 진영 폴백+경고)와 폴백은 이동 전과 동일하고, null 액터 가드만 추가했다.
- `ToIcdObjectClass` = 기존 UGV ICD 문자열(바이트 단위 동일).
- `ToDroneClassId` = enum + 1 (**enum 순서를 바꾸면 드론 classId가 바뀐다** — 새 값은 맨 뒤에).
- `ToDroneLabel` = ICD 문자열 그대로("기존 UGV 객체 탐지 분류를 그대로" 결정). 샘플은 한글 표시명("T-72A 전차")이었으므로, 표시명 요청이 오면 이 함수만 고치면 된다.

## 4. 드론 BoundingBox JSON(D4·D5)

**호출 시점**: `ADronePawn` 짐벌 캡처 경로에서 `GimbalCapture->CaptureScene()` **직전**(트랜스폼·렌즈를 이번 캡처 값으로 맞춘 뒤). 캡처 1회 = 프레임 1개라 박스가 그 프레임 영상과 같은 카메라로 계산된다. 짐벌 캡처는 라운드로빈 1/2라 60틱 기준 약 30프레임/초.

**생성 위치**: 자체방호축 프로세스에서만 — RTSP 스트림을 만드는 축 게이트 콜백 안에서 `DroneDetectionReporter`를 런타임 생성. 다른 축에선 null이라 비용 없음.

**필드**(샘플 키·철자·자릿수 그대로, 메시지 1개 = `{"frames":[{...}]}`):

| 필드 | 값 |
|---|---|
| `frameIndex` | 캡처마다 +1, 시나리오 재시작 시 0 |
| `scenarioTime` | `HH:MM:SS.mmm`, 리포터 시작/재시작 기준 월드 경과 시간 |
| `sourceUUID` | 드론 액터 경로 기반 결정적 GUID(소문자 하이픈) |
| `location` | 드론 위경도(소수 7) + 해발 고도(소수 2, `GeoCoordinateUtils::GetAltitudeMslMeters`) |
| `sourceOrientation` | **카메라** 월드 방향: yaw = 진북 기준 시계방향 0~360, pitch = 위+, roll = 카메라 롤 |
| `camera` | azimuthAngle/elevationAngle = sourceOrientation yaw/pitch, `FOV` = `GimbalCapture->FOVAngle`(수평, 줌 반영) |
| `targets[]` | 탐지 스캔 목록 중 이 프레임 화면과 겹치는 대상만(자기 자신 제외) |
| `targetUUID` | 대상 액터 경로 기반 결정적 GUID |
| `targetLocation` / `targetOrientation` | 액터 위치(해발) / 액터 회전(yaw는 bearing 변환) |
| `targetBoundingBox` | 0..1 UV × `OutputResolution`(= `SelfDefenseUavResolution`, 1024×576) 정수 픽셀, 좌상단 원점. 1px 미만이면 제외 |
| `targetBoundingBoxLable.lable` / `targetClassId` | D3 분류 |

직렬화는 `FJsonSerializer` 대신 직접 문자열로 만든다. UE Json 라이터는 double을 `%.17g`로 써서 `37.915203099999998` 같은 값이 나오고 키 순서도 샘플과 같다는 보장이 없기 때문이다.

**전송**: 자체 소켓 없이 S1의 `USelfDefenseReportSubsystem::SendReportJson`. 파일(`Saved/SelfDefenseReports/DroneDetection_*.jsonl`, `bWriteReportFile`)·UDP(`bEnableUdpSend`, 기본 꺼짐)·로비 IP/포트(`DetectionReportPort` 기본 9101)를 K05.1과 공유한다. 리포터 쪽 스위치는 `bEmitFrames`(내보내기 on/off)와 `LogEveryNFrames`(디버그 로그)뿐.

## 5. D1 확인 결과

- 드론 RTSP는 이미 송출 중: `rtsp://<자체방호PC>:8554/selfdefense/uav_gimbal`, H.264 NVENC, 4000kbps, 30fps.
- 구 `AUAVPawn`도 같은 mount를 만든다(`UAVPawn.cpp:188`). 레벨 바이너리를 검색한 결과 `BP_UAV`는 `kadex_demo.umap`·`kadex_demo_0716.umap`에만 있고, 메인 레벨 `New_kadex_0811.umap`(월드 파티션 아님)에는 없다 → 메인 레벨에선 충돌 없음.
- 로비 툴팁 문구 "기본 640x360"(`UI/AxisSelectionWidget.cpp:898-899`)은 이번에 안 고쳤다(내 담당 파일 아님).

## 6. 빌드 후 확인할 것

1. UGV축: RTSP RCWS 영상 위에 UDP 박스를 겹쳐 봤을 때 선회·줌 중에도 박스가 따라가는지. `bReprojectObjectDetectionBoxes`를 false로 바꿔 예전 동작과 비교.
2. UGV축: 사격 중(착탄 셰이크) 박스가 영상과 같이 흔들리는지.
3. 탐지 대상 11개 이상일 때 `TotalObject` ≤ 10인지.
4. 자체방호축: `Saved/SelfDefenseReports/DroneDetection_*.jsonl` 한 줄을 샘플과 비교(키 구조·타입·자릿수). `LogEveryNFrames=30`으로 로그 확인도 가능.
5. 드론 RTSP가 1024×576으로 나오는지(ffprobe/rtsp_viewer_test). 박스 픽셀을 영상 캡처 위에 그려 위치가 맞는지.
6. 시나리오 재시작 후 frameIndex·scenarioTime이 0부터 다시 시작하는지.
7. UGV ObjectClass 문자열이 이전과 같은지(D3 회귀).

## 7. 남은 것 / 주의

- 새 파일 6개 `p4 add` 필요, Generate Project Files 필요(새 .cpp).
- 프레임 파일 출력은 약 30줄/초라 장시간 실행 시 파일이 커진다(대상 5개 기준 줄당 약 2KB → 약 200MB/시간). 장시간 테스트 땐 `bWriteReportFile` 또는 리포터 `bEmitFrames`를 끌 것.
- `targetUUID`는 액터 경로 기반이라, 재시작 스냅샷으로 재스폰된 대상은 새 ID가 된다.
- 드론 UI의 빨간 박스(오버레이)는 칼만 추적 기본값이라 JSON 박스와 몇 px 다를 수 있다. `bSnapBoxesToTrueTarget`을 켜면 같은 계산이 된다.
