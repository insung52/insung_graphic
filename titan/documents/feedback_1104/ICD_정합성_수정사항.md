# UGV_RC_ICD 정합성 점검 — 수정 필요 사항

- 점검일: 2026-11-04
- 기준 문서: `C:\Work\kadex\UGV_RC_ICD.xlsx` (시트 1 통신프로토콜 / 2 슈어→LIG / 3 LIG→슈어 / SQ_Diagram)
- 대상 코드: `Source/titan_example/Network/UGVRemoteControlSubsystem.{h,cpp}`, `UGVRemoteControlTypes.{h,cpp}`
- 점검 방법: ICD와 코드를 정적으로 대조 (빌드·실기 통신 테스트 미수행)

> **배경**: 현재 코드는 예전 ICD(`lig_icd_ugv_rc_full.md`) 기준으로 작성됐다. 최신 xlsx에서 오타 정정, 필드 추가, SQ 다이어그램이 반영됐지만 코드에는 아직 들어가지 않았다.

---

## 요약

| 우선순위 | ID | 내용 | 난이도 |
|---|---|---|---|
| 🔴 P0 | A1~A3 | JSON 키 오타 3건 (ICD에서 정정됨) | 매우 쉬움 |
| 🔴 P0 | B5 | `RC_Movement` 1건당 1회만 적용 → RCWS 조향 사실상 불가 | 중간 |
| 🔴 P0 | B4 | RC 통신 두절 감지 없음 → UGV가 마지막 명령으로 계속 주행 | 중간 |
| 🟠 P1 | A4 | `UGV_RCWS_Status`에 Azimuth / Elevation / TotalBullet 누락 | 쉬움 |
| 🟠 P1 | A5 | `UGV_Period_BasicInformation`에 Runtime 누락 | 쉬움 |
| 🟠 P1 | B1 | 실제 송신 주기가 ICD보다 낮음 (20Hz → 약 15Hz) | 쉬움 |
| 🟠 P1 | B2 | ObjectDetectionResult 5Hz (SQ 기준 10Hz) | 매우 쉬움 |
| 🟠 P1 | C2 | TotalObject 상한(10) 미적용 | 쉬움 |
| 🟡 P2 | A6, A7 | 이벤트 메시지 ACK와 재전송 미구현 (ACK_Movement 포함) | 큼 (LIG 협의 필요) |
| 🟡 P2 | B3 | 연결 상태 머신 없음 (핸드셰이크 전에 주기 송신) | 중간 |
| 🟡 P2 | A8 | `RC_MotionMode` 미구현 (no-op) | 의미 확인 필요 |
| 🟡 P2 | C3, C5, C6 | Brake 비례값 무시 / Gear 보고 불일치 / EMERGENCY 해제 경로 | 쉬움~중간 |
| ⚪ 확인 | C1, C4, C7~C10, D | LIG에 확인해야 하는 항목 | — |

---

## 🔴 P0 — 즉시 수정

### A1~A3. JSON 키 오타 (ICD에서 정정됨)

ICD xlsx에 `기존 -> 수정` 형태로 정정이 명시되어 있다. 코드에는 "LIG 파서가 오타를 기대한다"는 주석과 함께 옛 철자가 남아 있다. 이대로면 **RC가 이 값들을 읽지 못한다.**

| ID | 메시지 | ICD (최신) | 현재 코드 | 위치 |
|---|---|---|---|---|
| A1 | `UGV_Period_Basicinfo` | `ControlRight` | `ConrtrolRight` | `UGVRemoteControlSubsystem.cpp:833` |
| A2 | `UGV_Period_Basicinfo` | `Battery` | `Batterry` | `UGVRemoteControlSubsystem.cpp:836` |
| A3 | `UGV_Period_ObjectDetectionResult` (RCWS, src=13) | `Letter` | `Latter` | `UGVRemoteControlSubsystem.cpp:128`, `:876` |

**수정 방법**
- 키 문자열을 ICD 철자로 바꾼다.
- A3은 `BuildDetectionObjectJson`의 `bUseLatterTypo` 분기를 없애고 항상 `Letter`를 쓴다.
- `UGVRemoteControlTypes.h:5-7`의 "오타 그대로 유지" 주석을 갱신한다. `BRUST`/`CONTINUS`는 ICD에 아직 그 철자로 남아 있으므로 유지한다.

### B5. `RC_Movement` — 조이스틱을 계속 밀어도 RCWS가 3°만 돌고 멈춤

- **ICD / SQ_Diagram**: `RC_Movement`는 **비주기**이고 `[Joystick event occur]` 때만 송신된다. 즉 스틱 값이 바뀔 때만 메시지가 온다.
- **현재 코드** (`UGVRemoteControlSubsystem.cpp:703-705`): 메시지 1건마다 `AddPanTiltInput`을 최대 3° 한 번만 호출한다. 스틱을 한 방향으로 계속 밀고 있으면 새 메시지가 오지 않으므로 처음 3°만 움직이고 멈춘다.
- 헤더 주석(`UGVRemoteControlSubsystem.h:188-190`, "폴링 주기마다 적용")도 실제 동작과 다르다.

**수정 방법**
- `Handle_RC_Movement`에서는 마지막 XAxis / YAxis / BrakeButton 값만 저장한다.
- `TickPoll`에서 매 틱 `값/100 × 각속도(deg/s) × DeltaTime`을 적용한다(속도 지령 방식).
- XAxis=0, YAxis=0이 들어오거나 게이트(`bRCWSSteeringActivated`, BrakeButton)가 비활성이 되면 정지한다.

### B4. RC 통신 두절 감지 없음 (안전)

- **ICD 시트1**: 주기성 메시지는 "( )회까지 미응답"이면 비정상 절차. **SQ**: 주기 루프는 `until connection false`.
- **현재 코드**: `RC_Request_BIT`(1Hz)나 `RC_RemoteDriving`(20Hz)이 끊겨도 타임아웃이 없다. 마지막 `SetThrottleInput`/`SetSteeringInput` 값이 차량 이동 컴포넌트에 그대로 남는다.
- **결과**: RC가 꺼지거나 네트워크가 끊기면 **UGV가 마지막 주행 명령대로 계속 달린다.**

**수정 방법**
- 마지막 `RC_RemoteDriving` 수신 시각을 기록한다.
- 일정 시간(예: 20Hz × 5회 = 250ms, 값은 LIG와 협의)이 넘으면 `DispatchSetManualControl(0,0)`과 `DispatchSetBraking(true)`를 호출한다.
- `RC_Request_BIT`이 N회(예: 3초) 들어오지 않으면 연결 끊김으로 판정한다(B3과 연계).

---

## 🟠 P1 — ICD 필드와 주기 맞추기

### A4. `UGV_RCWS_Status` 필드 누락

ICD에는 다음 필드가 추가되어 있다. 시트2에 "데이터 필드 추가요청"이라는 피드백도 달려 있다.

| 키 | 타입 | 값 | 코드 소스 후보 |
|---|---|---|---|
| `RCWSStatus` | String | `ON` / `OFF` | ✅ 구현됨 |
| `Azimuth` | Int | 0~360 | `FRCWSData::AzimuthDegrees` |
| `Elevation` | Int | 0~180 (수신 측이 -90 보정, 즉 수평=90) | `FRCWSData::ElevationDegrees + 90` |
| `TotalBullet` | Int | 최대 200발 | `FRCWSData::AmmoCurrent` (⚠️ `AmmoMax=600`, C9 참고) |

위치: `UGVRemoteControlSubsystem.cpp:840-846`

### A5. `UGV_Period_BasicInformation` — `Runtime` 누락

- ICD: `Runtime` String `"HH-MM-SS"` (총 실행시간 = UGV 에뮬레이터 구동시간)
- 수정 방법: 서브시스템 `Initialize` 시각 또는 `FPlatformTime::Seconds()`를 기준으로 경과 시간을 포맷한다. **구분자는 `:`가 아니라 `-`이다.**
- 위치: `UGVRemoteControlSubsystem.cpp:917-929`

### B1. 실제 송신 주기가 ICD보다 낮음

- 위치: `UGVRemoteControlSubsystem.cpp:218-260`
- 원인
  - 누적 타이머를 주기만큼 빼지 않고 `Accum = 0.f`로 리셋한다.
  - `FTSTicker`의 0.02s 간격이 프레임 단위로 정렬되면서 실제 간격이 길어진다.
- 60fps 기준으로 티커가 약 33ms마다 돌 때:
  - `UGV_RCWS_Status` (ICD 20Hz) → 약 15Hz
  - `UGV_Period_Basicinfo` (ICD 10Hz) → 7.5~10Hz
- 수정 방법
  - `Accum -= Period` 방식으로 바꾼다(밀린 횟수만큼 따라잡되, 최대 1회로 제한해도 됨).
  - 필요하면 송신을 별도 스레드나 고정 주기 타이머로 분리한다.

### B2. ObjectDetectionResult 주기

- SQ_Diagram의 "주기메세지 SQ"는 `[10HZ until connection false]` 루프 안에서 RCWS와 ADU의 `ObjectDetectionResult`, `NavigationInformation`을 송신한다.
- 현재 코드는 `ObjectDetectionSendHz = 5.f` (`UGVRemoteControlSubsystem.h:114`) → **10으로 변경**한다.
- `BasicInformation`(현재 1Hz)은 SQ에 없으므로 주기를 LIG에 확인한다.

### C2. `TotalObject` 상한 미적용

- ICD 범위는 0~10인데 현재는 탐지된 대상을 모두 보낸다(`UGVRemoteControlSubsystem.cpp:869-889`).
- 수정 방법: 거리순 등 우선순위로 정렬해 최대 10개까지만 보낸다.

---

## 🟡 P2 — 프로토콜 절차와 의미

### A6, A7. 이벤트 메시지 ACK와 재전송

- **ICD 시트1 "이벤트 메시지"**
  - 정상: Message → ACK
  - 비정상: `( )msec 내 ACK 응답이 없는 경우 3회까지 재시도`
- **SQ_Diagram**: `RC_Movement` → `ACK_Movement / RCWS` 응답이 명시되어 있다.
- **현재 코드**: RC 이벤트 명령에 ACK를 보내지 않고, 우리가 보내는 이벤트에도 재전송이 없다. 헤더 주석(`UGVRemoteControlTypes.h:9-18`)에 "ACK cmd가 정의돼 있지 않아 생략"이라고 되어 있다.
- **할 일**
  1. `ACK_Movement`부터 구현한다. src=13, recv=20이고, data 내용은 LIG에 확인한다.
  2. 나머지 RC_* 이벤트의 ACK cmd 이름, data 형식, 타임아웃(msec)을 LIG에 질의한다.

### B3. 연결 상태 머신 없음

- **SQ "초기접속"**: `[1HZ until reboot]`, `alt [connect==false]` 구간에서 `RC_Connection`/`RC_Connection_ADU`를 받고 `UGV_Response_Connection`으로 응답해 **Connection Done**이 된다. 그 다음에 주기 메시지를 시작한다.
- **현재 코드**: 소켓이 열리면 핸드셰이크 없이 주기 메시지를 즉시 송신한다. `RC_Connection.RequestDevice=0`(None)이 와도 연결 응답을 보낸다(`UGVRemoteControlSubsystem.cpp:441-447`).
- **수정 방법**
  - `bConnected` / `bConnectedADU` 상태를 둔다.
  - 연결이 된 뒤에만 해당 그룹(UGV·RCWS / ADU)의 주기 메시지를 송신한다.
  - B4의 타임아웃이 걸리면 연결 해제 상태로 되돌린다.
  - `RequestDevice=0`일 때의 의미(연결 해제인지)를 LIG에 확인한다.

### A8. `RC_MotionMode` 미구현

- ICD: `MotionMode` String, `RELEASE`=활성 / `PRESSED`=비활성 ("기동 모션 모드 설정")
- 현재 코드: 로그만 남기고 아무 동작도 하지 않는다(`UGVRemoteControlSubsystem.cpp:637-644`).
- 할 일: 이 명령이 실제로 무엇을 켜고 끄는지 LIG에 확인한 뒤 구현한다.

### C3. `RC_RemoteDriving.Brake` 비례값 무시

- ICD: Int `-0 ~ -100` (비례값). 상태천이도에서는 "Brake : 감속"
- 현재 코드: `Brake != 0`이면 브레이크 on/off로만 처리한다(`UGVRemoteControlSubsystem.cpp:492`).
- 수정 방법: `|Brake| / 100`을 브레이크 입력 크기로 반영한다.

### C5. Basicinfo `Gear` 보고값이 RC 지시와 어긋날 수 있음

- 현재는 차량 변속기 라벨 기준으로 `R`일 때만 `Back`, 나머지는 모두 `Front`로 보낸다(`UGVRemoteControlSubsystem.cpp:36-43`).
- RC가 `Gear=BACK`을 보내도 정지 중(N/P)이면 `Front`로 보고된다.
- 수정 방법: 마지막으로 수신한 RC 지시 기어를 보고값으로 쓰는 방안을 검토한다. `Turn`(제자리 선회)은 미지원으로 둔다.

### C6. `OperationMode=EMERGENCY` 해제 경로

- `RC_OperationMode=EMERGENCY`로 비상 래치가 걸리면 이후 `STAY`/`REMOTE`가 와도 풀리지 않는다. `RC_EmergencyStopRelease`로만 해제된다(`UGVRemoteControlSubsystem.cpp:525-527`).
- 그동안 Basicinfo는 계속 `Emergency`로 보고된다.
- 이 동작이 의도인지 LIG에 확인한다. 의도가 아니라면 `OperationMode` 경로의 비상과 `EmergencyStop` 경로의 비상을 별도 플래그로 나눈다.

---

## ⚪ LIG 확인 필요 (질의 리스트)

| ID | 항목 | 내용 |
|---|---|---|
| C1 | `ObjectClass` 값 | ICD는 `Human`/`Car`만 정의. 코드는 `Ally`/`Enemy`/`UGV`/`MobileCommandPost`/`Drone`/`Parachute` 6종을 송신한다(LIG 답변 response_0828 Q10 근거). **ICD 개정 요청** 또는 공식 근거 확보 필요 |
| C4 | `RC_RemoteDriving.Gear` 타입 | ICD Value Type은 **Int**인데 값은 `FRONT`/`BACK` 문자열로 적혀 있다. 코드는 문자열로만 처리하므로 정수가 오면 항상 전진한다 |
| C7 | `Altitude` | 모든 메시지에서 0 고정이다. ADU `Objects`에는 ICD상 `Altitude` 필드가 없는데 코드는 보내고 있다(제거 여부 확인) |
| C8 | UTM 자릿수 | ICD는 East 8자리, North 10자리. 코드는 미터 정수라 East 6자리, North 7자리가 나간다. 단위(cm/mm)를 확인해야 한다 |
| C9 | `TotalBullet` 상한 | ICD 최대 200발, RCWS `AmmoMax`=600 (`Vehicles/RCWSTypes.h:79`) |
| C10 | 속도 단위 | `Speed`(현재 km/h), `Velocity`(현재 m/s) — ICD에 단위 미기재 |
| D1 | 자체 확장 cmd | `HQ_MissionMoveToEngage`(수신)와 `RPT_ObjectiveReached`(RC 8011로 송신)는 ICD에 없다. RC 쪽에서 무시되는지, 오류가 나는지 확인 |
| D2 | 탐지 박스 위치 오차 | 시트2 피드백: "영상의 보이는 타겟의 위치와 box의 위치 차이가 심함". 스캔 시점과 송신 사이의 지연, 송신 주기(B2) 때문인지 실측 필요 |
| D3 | src/recv 검증 | 수신한 `src`/`recv`를 검증하지 않음. 필요 여부 확인 |
| — | ICD 내부 표기 불일치 | SQ 다이어그램의 `Period_RCWS_Status`는 시트2의 `UGV_RCWS_Status`와 이름이 다르다. 코드는 시트2 이름을 사용 중 |

---

## 참고 — ICD와 일치하는 항목 (수정 불필요)

- 전송 규약: UDP + JSON 봉투 `{cmd, src, recv, data}`, device 코드 Int(12/13/14/20)
- 포트: 수신 8000(주기)/8001(비주기), 송신 → 192.168.10.20:8010/8011
- 연결/BIT 응답: `UGV_Response_Connection` (차량 `"1"` / ADU `"Remote"`), `UGV_Response_BIT` (`StatusVMU`, `CountValue` echo), `UGV_Response_BIT_ADU` (On/Off 6항목)
- RCWS 명령: `RC_SelectCamera` (RELEASE=EO / PRESSED=IR, IR 흑백 후처리), `RC_FireMode` (SINGLE/BRUST/CONTINUS), `RC_ChargeWeapon`, `RC_FireWeapon`, `RC_ActivateFire`, `RC_ActivateMovement`
- 주행 관련 명령: `RC_Control_Right`, `RC_EmergencyStop` / `RC_EmergencyStopRelease`
- 주행명령 상태천이: 제어권 → 주행모드(REMOTE) → 기어 순서로 판정
- 송신 메시지: `UGV_Period_NavigationInformation` 필드 구성과 10Hz 주기, ObjectID 1~65535, 박스 좌표 65535 스케일링
