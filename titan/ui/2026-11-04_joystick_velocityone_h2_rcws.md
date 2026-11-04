# 조이스틱 — VelocityOne H2 미니스틱 → 트럭 RCWS, 메인 스틱 → 드론 카메라 (+ 수동 토글 재편)

2026-11-04 / 코드 완료(빌드·실측 대기), 에디터·실장비 작업 남음 / LIG 11/04 피드백 5번(work_list W12, J1~J3).

## 1. 요구와 결정

메일 원문: "[자기방호] 조이스틱의 H2 스틱값으로 RCWS 제어, 조이스틱의 메인 스틱으로 UGV 카메라 제어.
velocityone_tester_offline.tar.gz 내 joystick_reader.py 참고해서 입력 신호 값 매핑."

사용자 결정(2026-11-04, 같은 날 두 단계로 정했다):
1. "UGV 카메라"는 **자기방호 드론 카메라**로 해석한다(LIG에는 확인 통보만).
2. 이 요청은 **조이스틱 하나로 이동형지휘소 RCWS와 드론 카메라를 동시에** 조종하라는 뜻으로 본다. 그래서
   수동 조종 토글을 다시 짠다.
   - **이동형지휘소 RCWS 화면(Monitor2) 토글 = 모드 A**: RCWS 수동 + 드론 카메라 수동을 동시에 켠다.
   - **드론 화면(Monitor1) 토글 = 모드 B**: 드론 전체 수동(비행 + 카메라). 명세에 없던 기능이지만 남긴다.
   - 두 토글은 상호배타다(하나를 켜면 다른 하나는 꺼짐). 둘 다 끄면 전자동이다.
   - "드론 짐벌만 수동" 토글은 없애고 모드 A에 흡수했다.
   - 드론 줌은 조이스틱에 넣지 않는다. 기존 RCWS 버튼은 전부 트럭 RCWS에 연결한다.

## 2. 장비 분석 (velocityone_tester, 실행하지 않고 읽기만 함)

장비는 Turtle Beach VelocityOne(VID 0x10f5)이다. 툴은 Linux evdev를 직접 읽는다. 각 입력이 **진짜 아날로그 축인지,
디지털(버튼·hat)인지** 확인한 결과는 아래와 같다.

| 물리 입력 | evdev | 종류 (근거) | 툴의 값 범위 |
|---|---|---|---|
| 메인 스틱 | ABS_X / ABS_Y | **아날로그 축**(`normalize_axis` 양방향 분기, 시뮬레이션 값도 연속) | -1~1 |
| 비틀기 | ABS_Z | 아날로그 축 | -1~1 |
| **H2** | ABS_RX / ABS_RY | **아날로그 미니 썸스틱**(UI 표기 "Secondary Axis (H2 Mini Stick)"). hat 아님 | -1~1 |
| 레버1 / 레버2 / 트림휠 | ABS_RZ / ABS_THROTTLE / ABS_RUDDER | 아날로그, 단방향 | 0~1 (UE/SDL에서는 -1~1) |
| POV hat | ABS_HAT0X / ABS_HAT0Y | **디지털 8방향**(값 -1/0/+1로 클램프). SDL은 축이 아니라 hat으로 낸다 | -1/0/1 |
| 버튼 | BTN 0x120~0x12f, 0x2c0~0x2c7 | 디지털. 툴 표기 B1~B16, B17~B24 | 0/1 |
| 트랙패드 | EV_REL, BTN_LEFT(0x110) | 상대값/버튼 | — |

- 툴 버그: 0x110(트랙패드 클릭)과 0x2c3이 둘 다 "B20"으로 표시된다.
- 툴에는 데드존이 없다.
- UE(JoystickPlugin, SDL2 Linux) 키 예측: Axis_0=X, 1=Y, 2=Z, **3=RX, 4=RY**, 5=RZ, 6=THROTTLE, 7=RUDDER.
  hat은 `Hat_0_*`, 버튼은 `Button_N`(0부터). SDL은 장치가 보고하는 ABS 코드를 오름차순으로 번호 매기므로 **실측 전 값이다.**
- 사용자는 모드 A를 "드론 카메라 메인 스틱, RCWS hat"으로 불렀다. 장비상 **H2는 hat이 아니라 아날로그 미니스틱**이다.
  POV hat은 따로 있고, 모드 B에서 드론 카메라 4방향을 맡는다.

## 3. 모드별 입력 배정 (최종)

| | 메인 스틱(아날로그) | 비틀기 | H2(아날로그) | POV hat(디지털) | RCWS 버튼 |
|---|---|---|---|---|---|
| **A: RCWS + 드론 카메라** (Monitor2) | 드론 짐벌 pan/tilt (`DroneGimbalLookAction`) | — | 트럭 RCWS pan/tilt (`CameraLookAction`) | — | 트럭 RCWS |
| **B: 드론 전체 수동** (Monitor1) | 드론 비행 사이클릭 (`DroneCyclicAction`) | 드론 요 | — (입력 버림) | 드론 짐벌 4방향 (`DroneGimbalUp/Down/Left/Right`) | — (대상 없음) |
| 자동 | 입력 버림 | — | 입력 버림 | — | — |

### 모드끼리 입력이 겹치지 않게 한 방법

9/01과 9/10에 겪은 충돌을 다시 정리하면 이렇다. 같은 물리 축이 두 IMC에 동시에 매핑되면, 우선순위가 높은 쪽이 축을
소비해 버린다. 그래서 낮은 쪽 액션은 아예 발화하지 않는다("스틱이 통째로 무반응"). 이번 구조의 규칙은 다음과 같다.

- `IMC_MouseLook`(상시, 우선순위 0): `CameraLookAction`(H2), **신규 `DroneGimbalLookAction`(메인 스틱)**, RCWS 버튼.
- `IMC_DroneManual`(우선순위 10): **모드 B에서만 올린다**(예전에는 "짐벌만 수동"일 때도 올렸다). 사이클릭(메인 스틱), 요, 스로틀, hat 4방향.
  - 모드 A: `IMC_DroneManual`이 내려가 있다. 그래서 메인 스틱은 `DroneGimbalLookAction`이 그대로 받는다.
  - 모드 B: `IMC_DroneManual`의 사이클릭이 메인 스틱을 소비한다. 이건 의도한 동작이다. 그래서 `DroneGimbalLookAction`은 발화하지 않는다.
- 코드에도 이중 게이트를 걸었다. 바인딩이 어떻게 되어 있어도 맞게 동작한다.
  - `DoDroneGimbalLook`: 짐벌 수동 모드(A, 콘솔 UAVGimbal)가 아니면 버린다.
  - `DoCameraLook`: 자기방호축에서 대상이 UAVGimbal이면 버린다(모드 B에서 H2가 짐벌로 새는 것 차단).
  - `SendDroneManualFlightInput`: 모드 B가 아니면 버린다.
- 예전에 "짐벌만 수동"이 쓰던 우회는 삭제했다. 비행 사이클릭을 짐벌로 돌려 쓰던 `SendDroneManualFlightInput` 분기와,
  그 때문에 필요했던 `bInvertDroneGimbalOnlyTilt` 노브다. 이제 짐벌 스틱은 사이클릭과 극성을 공유하지 않는다.

### Input 설정 탭 대응

설정 탭 백엔드(`UTitanInputBindingSubsystem`)는 행마다 `TargetContext`(어느 IMC에 심을지)와 바인딩별 Negate를 지원한다.
그래서 **드론 카메라가 모드에 따라 다른 입력에 걸리는 문제**는 행 두 종류로 풀린다.
- "드론 카메라 (메인 스틱)" 행 → `IA_DroneGimbalLook`, TargetContext = `IMC_MouseLook`. 모드 A용이다.
- 기존 "드론 짐벌 상/하/좌/우" 행 → TargetContext = `IMC_DroneManual`. 모드 B용(hat)이다.
- 메인 스틱 하나를 "드론 카메라 (메인 스틱)"과 "드론 사이클릭" **두 행에 같이 배정해도 된다.** 두 행이 서로 다른 IMC에
  있고, 그 IMC들이 동시에 올라가지 않기 때문이다.
- 상하 극성은 각 행의 Negate로 맞춘다(코드 노브 없음).

## 4. 코드 변경

`titan_examplePlayerController.h/.cpp`
- `ESelfDefenseManualTarget::TruckRCWSAndUAVGimbal` 추가. 기존 `TruckRCWS`와 `UAVGimbal`은 콘솔용으로 남겼다
  (`SetSelfDefenseManual Truck|Gimbal|Combined|UAV|None`).
- 헬퍼 `IsTruckRCWSManual` / `IsDroneGimbalOnlyManual`을 추가했다. 단일 `==` 비교 대신 이걸로 판정한다.
- `SetSelfDefenseManualTarget`: 모드 A에서 CameraControlTarget = TruckRCWS로 둔다(H2와 RCWS 버튼이 트럭으로 감).
  `IMC_DroneManual`은 모드 B에서만 올린다.
- `ApplySelfDefenseManualTarget`: 트럭과 드론을 **각각** 판정하고, 수동 여부가 실제로 바뀔 때만 건드린다.
  예전처럼 "이전 대상 복귀 → 새 대상 진입"으로 하면 TruckRCWS → Combined 전환에서 `TruckModeBeforeManual`이 Remote로
  덮어써진다(10-12 EndPlay 버그와 같은 형태). 드론은 Full↔GimbalOnly 전환 때 기존처럼 None을 거친다.
- `DroneGimbalLookAction` UPROPERTY와 `DoDroneGimbalLook` 핸들러를 추가했다.
  - 기본 에셋 경로 `/Game/Input/Actions/IA_DroneGimbalLook`은 ConstructorHelpers로 물어 둔다. 에셋이 없으면 경고만 남긴다.
  - 짐벌 환산과 권한 분기는 예전 GimbalOnly 경로와 같다(2-PC 검증된 경로).
- `ApplyLookDeadZone`: 원형 데드존 `CameraLookDeadZone`(기본 0.1). H2와 메인 스틱 짐벌이 같이 쓴다.
- ~~`DoRCWSUAVAxisToggle`(Button_8): 자기방호축에서는 무시한다.~~ **2026-11-05 제거**(액션 프로퍼티·바인딩·핸들러·Input 탭 행).
  이 세션이 Input 탭에 "UGV축 전용"으로 남겨 뒀지만 UGV축은 드론을 다루지 않아 쓸 곳이 없었다(사용자 지적).

`UI/SelfDefenseMonitor2Widget`: `TruckManualControlCheckBox`를 모드 A로 연결했다(체크 표시는 TruckRCWS 단독일 때도 켜짐).
`UI/SelfDefenseMonitor1Widget`: `UAVGimbalManualControlCheckBox`를 코드에서 숨기고 바인딩하지 않는다. 드론 전체 수동 토글은 그대로다.

## 5. 남은 작업 (에디터·실장비) — 축 번호는 모두 예측값

- [ ] **IA_DroneGimbalLook 생성**: `/Game/Input/Actions/IA_DroneGimbalLook`, Value Type = Axis2D(Vector2D).
      경로가 다르면 BP_TestPlayerController의 `Input|Drone > DroneGimbalLookAction`에 지정한다.
- [ ] **DA_TitanInputSchema**
  - 새 행 "드론 카메라 (메인 스틱)": Action=IA_DroneGimbalLook, Roles 2개(X / Y + `bApplySwizzleYXZ`), `bAllowAxisKeys`=true,
    TargetContext=`IMC_MouseLook`.
  - CameraLook 행 표시명을 "이동형지휘소 RCWS 조준 (H2 미니스틱)"으로 바꾼다. TargetContext가 `IMC_MouseLook`인지 확인한다.
  - 드론 사이클릭/요/스로틀/짐벌 4방향 행의 TargetContext가 `IMC_DroneManual`인지 확인한다.
- [ ] **입력 탭에서 실장비로 배정**(패키지에서 각 PC마다 저장)
  - RCWS 조준 = Axis_3/4
  - 드론 카메라 = Axis_0/1
  - 드론 사이클릭 = Axis_0/1, 요 = Axis_2
  - 스로틀 = **Axis_3 금지**(H2-X). 레버 Axis_5 또는 버튼으로.
  - 짐벌 4방향 = Hat_0 상/하/좌/우
  - RCWS 버튼 = Button_N
- [ ] **WBP**
  - Monitor2 토글 라벨을 "RCWS + 드론 카메라 수동"으로 바꾼다.
  - Monitor1에서 "짐벌만 수동" 체크박스와 라벨을 삭제한다(코드에서는 이미 숨김. 라벨만 남아 보일 수 있다).
- [ ] **실측(J3)**: 인게임 Joystick Viewer로 SDL 장치명, 축/버튼 번호, H2가 정말 연속값인지, 극성, 드리프트 크기, B20을 확인한다.

## 5-1. Input 탭 개편 — 조작부 중심 (2026-11-04 2차)

사용자 지적: Input 탭 라벨이 행 이름(CameraLook, DroneCyclic …)이라 지금 조작 체계(H2 / 메인 스틱 / 햇 / 모드 A·B)와
맞지 않는다. 결정한 내용은 다음과 같다.
- **조작부 중심**으로 바꾼다. 축과 햇은 물리 조작부 단위로 **한 번만** 배정한다. 동작 연결은 코드 표에 고정한다.
  RCWS 기능 버튼은 기능별로 배정한다.
- **저장 초기화, 빈 배정으로 시작한다**(키보드 기본값도 없음).
- **모드 안내표**(읽기 전용)를 탭 위쪽에 둔다.
- 동작 연결의 기본값은 velocityone_tester를 따른다(H2 = RCWS 조준, 메인 스틱 = 모드 A 드론 카메라 / 모드 B 비행, 햇 = 모드 B 드론 카메라).

구현:
- 신규 파일 `Input/TitanInputPresentation.h`(코드 표): 행마다 표시명, 섹션, 순서, **KeySourceRow**, 모드 A/B 안내 문구를 둔다.
  - `DroneCyclic`의 KeySourceRow는 `DroneGimbalLook`이다. 그래서 Input 탭에 "메인 스틱 (X / Y)" 한 줄만 나오고,
    그 배정이 모드 A 카메라(IMC_MouseLook)와 모드 B 비행(IMC_DroneManual)에 같이 들어간다. 두 IMC는 동시에 올라가지 않는다.
  - 따라가는 행은 원본 행의 Negate도 같이 쓴다. 그래서 Negate의 의미는 "장비 극성 맞추기"(앞으로 밀면 -1)이고,
    기능별 극성은 코드가 맡는다(비행 = `bInvertDroneManualCyclicPitch`, 카메라 = raw 그대로).
  - 표를 DA가 아니라 코드에 둔 이유: 모드별 의미가 PlayerController 모드 로직과 한 몸이고, 바이너리 DA의 구조체 배열을
    MCP로 고치는 건 위험하다. DA는 행/액션/IMC/Role 정의만 맡는다. 표에 없는 행은 "기타" 섹션에 행 이름으로 나온다.
- `UTitanInputBindingSubsystem`
  - 저장 슬롯을 `TitanInputBindings_v2`로 바꿨다(옛 슬롯은 무시).
  - SeedDefaults는 DefaultGroups를 시드하지 않고, 보이는 행마다 빈 그룹을 하나씩 만든다.
  - ApplyToMappingContexts는 따라가는 행에 원본 행의 키를 심는다.
- `GameSettingsWidget::RefreshKeybindList`: 맨 위에 모드 안내표, 그 아래 섹션 제목 + 순서대로 행. 따라가는 행은 숨긴다.
  TextBlock은 코드에서 만들므로 WBP 수정은 필요 없다.
- `KeybindRowWidget`: 라벨에 표시명을 쓴다.
- rowName(저장 키)은 그대로 둔다.

**메인 스틱 하나뿐인 장비로 테스트하기(Extreme 3D Pro: X/Y/비틀기/노브 + 햇)**: "H2 미니스틱" 행에 `Hat_0_X`/`Hat_0_Y`
(Axis1D로 등록되어 축 행에서 고를 수 있음)나 노브 축을 배정한다. 아니면 모드를 바꿀 때마다 메인 스틱을 다시 배정한다.
햇을 H2에 쓰면 디지털이라 RCWS가 최대 속도로만 돈다.

## 6. 빌드 후 테스트 포인트

1. 모드 A
   - H2 → 트럭 RCWS pan/tilt, 메인 스틱 → 드론 짐벌. **둘을 동시에 움직였을 때 각각 반응하는지.**
   - RCWS 버튼(발사/EO·IR/줌/모드/안전/장전/브레이크)이 트럭으로 가는지.
   - 드론은 자율비행/교전 관측을 계속하는지.
2. 모드 B: 메인 스틱 = 비행, 비틀기 = 요, hat = 짐벌, H2는 무반응.
3. A ↔ B 전환: 상호배타인지(한쪽 체크가 꺼짐), 전환 직후 스틱이 죽지 않는지, B를 해제할 때 576km/h 역주행이 재발하지 않는지.
4. 둘 다 해제(전자동): 트럭 RCWS가 수동 진입 전 모드로 복귀하는지, 드론 짐벌이 기본 자세로 돌아가는지.
5. ~~Button_8은 자기방호축에서 무반응이어야 한다.~~ 축전환 버튼 자체가 제거됨(2026-11-05). Input 탭에 "RCWSUAVAxisToggle" 행이 안 보여야 한다.
6. UGV축 RCWS 조준 감도(데드존 0.1)가 괜찮은지.

### 2026-11-05 후속 수정 — 모드 A에서 짐벌이 기본 자세로 끌려감
- 증상(사용자): 모드 A에서 메인 스틱으로 드론 카메라를 돌려도 특정 방향으로 천천히 되돌아감.
- 원인: `ADronePawn::UpdateGimbalRecon`의 Idle 단계(자율비행 중, 정찰 시작 전)가 `TickGimbalHomeReturn`
  (기본 자세 요 0 / 피치 −15°, 20°/s)을 **수동 판정 없이** 불렀다. 모드 A는 비행이 자동이라 이 함수에 계속 들어온다.
  다른 단계(낙하산 관찰/교전 프레이밍)는 이미 `IsGimbalManuallyControlled()`로 막혀 있었다.
- 수정: 복귀를 `!IsGimbalManuallyControlled()`일 때만. 수동을 끄면 다음 틱부터 복귀·트래킹이 다시 동작한다.
- 확인 포인트: 모드 A에서 짐벌을 돌려 놓고 손 떼면 그 자리에 머무는지(단계 무관) → 모드 A 해제 시 기본 자세/교전 프레이밍으로 돌아가는지.
  참고로 짐벌 요는 기체 기준이라, 자동 비행 중 기체가 선회하면 화면 방향도 같이 돈다. 이건 복귀와 별개다.
7. 2-PC(자체방호 = 클라이언트)에서 모드 A의 짐벌과 RCWS가 서버 쪽 화면/RTSP에 반영되는지.
8. Input 탭
   - 처음 열면 모든 칸이 비어 있는지(새 슬롯). 맨 위 안내표와 섹션 제목이 보이는지. "메인 스틱 (비행)" 행은 없는지.
   - 메인 스틱을 한 번 배정한 뒤 모드 A(카메라)와 모드 B(비행)에서 둘 다 동작하는지.
   - Negate를 켜면 두 모드에서 같이 뒤집히는지.
   - Apply 후 재시작했을 때 배정이 유지되는지.
