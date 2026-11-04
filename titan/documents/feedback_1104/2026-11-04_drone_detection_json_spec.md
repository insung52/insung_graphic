# 드론 객체탐지 BoundingBox JSON 형식

2026-11-04 / 설계단계 / `sample1_boxed_metadata.json`을 분석해 정리한 형식 정의 + 우리 쪽 구현 매핑 초안. 실시간 전송 여부·경로는 LIG 답변 대기.

> 성격: **드론 카메라 영상에서 탐지·추적한 객체 데이터**(드론 상태 + 객체별 박스·위치). 위치보고(K05.1)와는 별개.
> 아래 표의 범위·값은 모두 **샘플 1개에서 관측한 값**이며 규격이 아니다.

---

## 1. 샘플 파일 관측 결과

| 항목 | 값 |
|---|---|
| 파일 크기 | 약 860KB |
| 최상위 | `{ "frames": [ ... ] }` (객체 1개, 키 `frames`뿐) |
| 프레임 수 | 240개 (`frameIndex` 0~239) |
| 시간 | `scenarioTime` `00:00:00.000` ~ `00:00:09.958` (약 10초) |
| 프레임 간격 | 0.041~0.042초 → **약 24fps** |
| 프레임당 타겟 수 | 항상 5개 (전 프레임 동일) |
| `sourceUUID` | 1종 (드론 1대) |
| `targetUUID` | 5종 (프레임 사이에 유지 → **추적 결과**) |
| 라벨 / classId | `"T-72A 전차"` / `1` 하나뿐 |

→ 실시간 스트림을 약 10초 기록한 파일로 보인다. 한 프레임이 메시지 하나일 가능성이 높지만 **LIG 확인 전엔 추정**.

## 2. 구조

```json
{
  "frames": [
    {
      "frameIndex": 0,
      "scenarioTime": "00:00:00.000",
      "sourceUUID": "00949795-2db9-56cf-915d-9edce1dfc5f5",
      "location":          { "latitude": 37.8870258, "longitude": 128.2434063, "altitude": 680.68 },
      "sourceOrientation": { "yaw": 101.217, "pitch": -56.818, "roll": 0.0 },
      "camera":            { "azimuthAngle": 101.217, "elevationAngle": -56.818, "FOV": 48.455 },
      "targets": [
        {
          "targetUUID": "e34f5a46-76d0-5b2d-960b-19a7da28afdb",
          "targetLocation":    { "latitude": 37.8865226, "longitude": 128.2450497, "altitude": 481.37 },
          "targetOrientation": { "yaw": 180.0, "pitch": -0.217, "roll": 7.996 },
          "targetBoundingBox": { "x": 618, "y": 192, "width": 43, "height": 20 },
          "targetBoundingBoxLable": { "lable": "T-72A 전차" },
          "targetClassId": 1
        }
      ]
    }
  ]
}
```

- 키 철자는 샘플 그대로: `targetBoundingBoxLable`, `lable` (오타처럼 보이지만 **샘플을 따르는 것이 안전**. 수정 요청은 LIG에 확인).
- `FOV` 키만 대문자.

## 3. 필드 정의

### 3-1. 프레임 레벨

| 필드 | 타입 | 관측 | 의미(추정) |
|---|---|---|---|
| `frameIndex` | int | 0부터 1씩 증가 | 프레임 번호 |
| `scenarioTime` | string | `HH:MM:SS.mmm` | 시나리오 경과 시간 |
| `sourceUUID` | string(UUID) | 고정 | 드론(촬영 주체) ID |
| `location.latitude` | number | 소수점 7자리, 37.8864~37.8871 | 드론 위도 |
| `location.longitude` | number | 소수점 7자리, 128.2434~128.2442 | 드론 경도 |
| `location.altitude` | number | 소수점 2자리, 642~681 | 드론 고도(m, 해발 추정) |
| `sourceOrientation.yaw/pitch/roll` | number | 소수점 3자리. yaw 96~114, pitch −64~−57, roll 0.0 | 촬영 주체 자세(deg) |
| `camera.azimuthAngle` | number | **`sourceOrientation.yaw`와 모든 프레임에서 동일** | 카메라 방위각 |
| `camera.elevationAngle` | number | **`sourceOrientation.pitch`와 모든 프레임에서 동일** | 카메라 고각(아래로 음수) |
| `camera.FOV` | number | 39.6~48.5 (프레임마다 변함 = 줌) | 카메라 화각(deg) |

→ **`sourceOrientation`이 사실상 카메라 시선 방향**이다(샘플 기준). 우리 드론은 기체 yaw와 짐벌 방향이 다르므로(짐벌 수평 프레임 안정화) 어느 쪽을 `sourceOrientation`에 넣을지 LIG 확인 필요.

### 3-2. 타겟 레벨 (`targets[]`)

| 필드 | 타입 | 관측 | 의미(추정) |
|---|---|---|---|
| `targetUUID` | string(UUID) | 5종, 프레임 간 유지 | 추적 ID |
| `targetLocation.latitude/longitude/altitude` | number | 고도 478~487 | 타겟 위경도·고도 |
| `targetOrientation.yaw/pitch/roll` | number | yaw 41~180, pitch −7~12, roll 1.7~10 | 타겟 자세 |
| `targetBoundingBox.x/y/width/height` | **int(픽셀)** | x 158~835, y 87~457, w 17~76, h 19~68 | 영상 좌상단 기준 박스 |
| `targetBoundingBoxLable.lable` | string | `"T-72A 전차"` | 클래스 라벨 |
| `targetClassId` | int | 1 | 클래스 번호 |

- 박스 좌표는 정수 픽셀이지만 **기준 영상 해상도는 샘플에 없다**. x 최대 835이므로 최소 가로 ~840px 이상.
- 한 프레임에 박스가 화면 밖/가려짐인 타겟을 포함하는지는 알 수 없다(샘플은 항상 5개 전부 포함).

## 4. 우리 구현 매핑 (초안)

| JSON 필드 | 우리 쪽 데이터 | 비고 |
|---|---|---|
| `frameIndex` | 송신 카운터 | 시나리오 재시작 시 0으로? (확인) |
| `scenarioTime` | 시나리오 경과 시간 | 재시작 시 리셋 |
| `sourceUUID` | 드론 고정 UUID(설정값) | |
| `location` | 드론 월드 위치 → `GeoCoordinateUtils` | K05.1과 동일 소스 |
| `sourceOrientation` | 드론 기체 또는 짐벌 카메라 방향 | **어느 쪽인지 확인** |
| `camera.azimuthAngle/elevationAngle` | 짐벌 pan/tilt (월드 기준) | 짐벌 가이드 12.4절 |
| `camera.FOV` | 짐벌 카메라 FOV (`CameraFOV / ZoomLevel`) | 줌 배율 반영 |
| `targets[].targetUUID` | 탐지 대상 액터별 고정 UUID | 추적 ID 유지 |
| `targets[].targetLocation` | 대상 월드 위치 → 위경도 | |
| `targets[].targetOrientation` | 대상 액터 회전 | |
| `targets[].targetBoundingBox` | 대상 → 드론 카메라 화면 투영 | **신규 구현** (아래) |
| `targets[].targetBoundingBoxLable/ClassId` | 객체 분류 매핑 | **classId 표 필요** |

### 신규 구현이 필요한 부분 (규모 대)

1. **박스 계산**: 대상 바운드를 드론 짐벌 카메라의 뷰/투영으로 화면 좌표에 투영, 영상 해상도 기준 정수 픽셀 사각형으로 변환. 줌/FOV 변화 반영.
2. **가시성**: 화면 밖·가려짐 판정(트레이스) 후 포함 여부 결정. 탐지 가능 거리 제한 여부.
3. **추적 ID**: 대상 액터별 UUID 고정, 사망·재스폰 시 ID 정책(재시작 스냅샷 재스폰 → 새 UUID?).
4. **영상과의 시간 정렬**: RTSP 인코드 지연 때문에 영상은 박스보다 늦다 → UGV 쪽 "박스 어긋남" 피드백(ICD 문서 2-4)과 같은 원인일 수 있음. `scenarioTime`/`frameIndex`와 영상 프레임 대응 방법 검토.
5. **탐지 대상 범위**: 적/아군/차량 중 무엇을 내보낼지(현재 탐지 모델: 보병만 아군/적군, 차량·UGV·드론은 전부 Friendly — 탐지 가이드 `guide/detection_dev_guide.md`).
6. **직렬화·전송**: 프레임 단위 JSON 생성 + 설정 가능한 전송부(경로 미정).

기존 UGV 객체인식(`UGV_Period_ObjectDetectionResult`)은 박스를 65535 스케일 정수로 보내는 별도 포맷 — 박스 투영 로직은 참고·재사용 가능하나 **JSON 스키마와 좌표 단위(픽셀)는 다르다**.

## 5. LIG 확인 필요 (요약)

1. 실시간 프레임별 전송인가, 영상 1개당 파일 생성인가 → 실시간이면 경로(IP/포트/프로토콜)와 프레임당 메시지 형태
2. 전송 주기(영상 fps와 동일 24fps? 우리 RTSP는 현재 30fps 선언)
3. `targetClassId` / 라벨 표 (우리 객체 분류 매핑)
4. 박스 기준 영상 해상도, 영상 밖/가려진 타겟 포함 여부
5. `sourceOrientation`이 기체 자세인지 카메라 시선인지
6. `altitude` 기준(해발/지면), `scenarioTime` 기준과 재시작 시 동작
7. 키 철자(`Lable`) 그대로 유지할지

## 6. 구현 방침 (제안)

- 프레임 1개를 `{ "frames": [ {...} ] }` 중 요소 하나로 보내는 형태를 기본안으로 하고, 직렬화와 전송을 분리.
- 샘플 JSON을 **회귀 비교용 정본**으로 쓰고, 생성 결과의 키 구조·타입이 샘플과 같은지 스키마 수준으로 비교.
- 박스 정확도 검증은 영상 캡처 위에 박스를 그려 눈으로 확인(UGV 어긋남 조사와 같은 방법).
