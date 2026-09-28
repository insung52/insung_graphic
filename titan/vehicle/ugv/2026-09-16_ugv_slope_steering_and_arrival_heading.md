# UGV 자율주행 — 횡경사 조향 보정 + 도착 방향 정렬

2026-09-16 / 진행중 (3차 실주행까지 반영, 4차 검증 대기) / 횡경사에서 옆으로 밀리는 걸 슬립각·롤로 되먹이는 조향 보정(A+B)과, 목적지 TargetPoint와 **같은 위치·방향에 매번 동일하게 주차**시키는 도착 정렬(내비메시 경로로 T까지 → 제동 곡선 → 1m 크리핑 → 10cm 정지 → 제자리선회 → 잔차 30cm/3° 이내면 스냅 확정)을 `AUGVAIController`에 추가. 실주행 3회에서 드러난 함정(접근점 P가 숲으로 떨어짐 → **폐기**, 정지 판정 오판, Chaos 브레이크 ON/OFF 문제 → 별도 문서)을 §7에 기록.

선행 문서: `2026-08-22_ugv_corner_braking_dev_guide.md`(커브 감속), `2026-08-26_ugv_obstacle_avoidance.md`(요레이트 조향),
`2026-08-31_ugv-speed-pi-controller.md`(속도 PI — "앞뒤 기울기 스로틀 보정"의 실체), `2026-09-03_ugv_0901_bp_to_cpp.md`(현재 UGV 차량)
같은 날 파생 문서: `2026-09-16_ugv_chaos_brake_proportional_and_brake_steer.md`(브레이크가 ON/OFF였던 문제 — 실주행 2·3차의 진짜 원인)

---

## 0. 시작 시점의 코드 상태

- 차량 `BP_UGV_0901`(6×6 차륜, 로직 `AUGV0901Pawn`). **여전히 스킨스티어** — `SetManualControl`이 `SetSteeringInput`을
  부르고 `FUGVWheeledVehicleSimulation`이 좌우 휠 토크 차이로 돌린다(`bAffectedBySteering` 전 휠 false).
- 조향: 요레이트 PI + 저속 방향오차 P 블렌드. **롤·횡속도·크로스트랙 항은 전혀 없었다.**
- 속도: PI 절대출력(08-31). 경사 보상은 경사를 재는 게 아니라 속도 오차 적분항이 흡수하는 구조.
- 도착: 마지막 경로점 반경 `AcceptanceRadius`(**10m**) 안 → `HandleMoveCompleted` → `Idle` → 핸드브레이크.
  방향은 안 봤고, 시나리오는 `ATargetPoint`를 들고 있으면서 `GetActorLocation()`만 넘겼다(회전 버림).

---

## 1. 문제 1 — 횡경사에서 조향이 매끄럽지 않음

### 1.1 왜 기존 제어기로는 안 되는가

횡경사에서 두 가지가 겹친다:

1. **중력 횡성분으로 차체가 내리막 쪽으로 평행이동**(미끄러짐). 이건 요 오차가 아니라 **위치 오차**라
   요레이트 PI가 아예 못 본다. lookahead 지점이 옆으로 벗어나면서 뒤늦게 방향오차로 바뀌어야 조향이 들어간다
   → "밀린 뒤 고치는" 지연.
2. **좌우 접지하중 비대칭** — 내리막쪽 바퀴에 하중이 실려 견인력이 달라지고, 스킨스티어라 그게 곧 요 외란.
   요레이트 적분항(`YawRateKi=0.02`)이 잡긴 하지만 느리다.

### 1.2 구현 — 두 겹

**A. 사이드슬립(횡속도) 보상 — 주 제어**
강체 속도를 차체 축으로 분해해 슬립각 `β = atan2(v_lat, |v_fwd|)`를 구하고, **목표 방향에서 빼준다**:
`AngleError += clamp(−β·SideslipGain, ±SideslipMaxCorrectionDeg)`.

- 항공기 크랩각 보상과 같은 원리 — 옆으로 밀리는 만큼 기수를 반대로 틀어 **속도 벡터**가 lookahead를 향하게.
- 미끄러지는 *원인*(중력/하중)과 무관하게 *결과*(횡속도)를 재므로 지형 센싱 불필요.
- `AngleError`를 바꾸는 방식이라 요레이트 루프와 저속 P 분기 양쪽에 자동 반영.
- `|v_fwd|`를 쓰는 건 뒤로 굴러갈 때 부호가 뒤집혀 반대로 보정하는 걸 막기 위해.
- `SideslipMinSpeedKmh` 이하에서는 끔 — 정지 노이즈, 그리고 제자리선회 중 횡속도는 회전에서 나오는 것.

**B. 롤 피드포워드 — 보조**
`RightDown = −RightVector.Z`(오른쪽이 낮을수록 +) × `RollSteerGain` → 조향 바이어스. A가 "밀린 뒤 반응"이면
B는 "밀리기 전에 선행". 최종 조향에 더하되 **스킨스티어 감쇠 보상 나눗셈 전**에 더한다 — 고속에서 조향 권위가
깎이는 만큼 바이어스도 같이 보상받아야 한다.

**부호 규약**: 롤은 회전자 `Roll`이 아니라 `RightVector.Z`로 판정한다. UE 회전자 Roll 부호를 잘못 짚으면 반대로
조향하는 사고가 나는데, 벡터로 재면 그 여지가 없다(값은 `sin(roll)`과 같음).

**"조향에 따라 미래 기울기가 정해진다"는 되먹임 우려**: B는 현재 롤에만 반응하고 게인이 작아 발산할 만큼 세지
않다. 크라운을 넘는 순간 부호가 바뀌는 건 오히려 정확한 동작. 주 제어는 A.

### 1.3 파라미터 (`BP_UGVAIController_new` → `UGV AI|Slope Steering`)

| 프로퍼티 | 기본값 | 의미 |
|---|---|---|
| `bEnableSideslipCompensation` | true | A on/off |
| `SideslipGain` | 1.0 | β 1°당 보정 각도. 1.0 = 밀리는 각도만큼 정확히 되돌림 |
| `SideslipMaxCorrectionDeg` | 15° | 과보정(오르막 파고들기) 상한 |
| `SideslipMinSpeedKmh` | 3 | 이하에선 끔 |
| `bEnableRollFeedforward` | true | B on/off |
| `RollSteerGain` | 0.5 | sin(roll) 1.0당 조향. 10° 경사(sin≈0.17)에서 ≈0.09 선행 |
| `RollFeedforwardMaxSteer` | 0.3 | 바이어스 상한 |

진단 로그(`bLogPursuitDiagnostics`)에 `slip=%.1fdeg rollBias=%.2f` 추가.

---

## 2. 문제 2 — 도착 시 차체 방향을 TargetPoint에 맞추기

### 2.1 설계 (최종) — 내비메시 경로 그대로 T까지 → 정밀 정지 → 제자리선회 → 정착·스냅

목표(사용자 확정): **매 주행 종료 시 TargetPoint와 동일한 위치·방향에 주차, 변수 없이.** 단 10cm/1°를 쫓느라
앞뒤로 꿈틀거리거나 옆으로 미끄러지는 비정상 움직임은 금지. 그래서 "위치는 주행으로, 방향은 제자리선회로
먼저 맞추고, 마지막 눈에 안 띄는 잔차만 스냅으로 확정"하는 구조다. 물리 루프만으로는 잔차가 항상 남으므로 결정성은
스냅이 준다.

새 API `AUGVAIController::MoveToDestinationFacing(FVector Destination, float FinalYawDeg)`. 기존 `MoveToDestination`은
그대로(둘 다 `MoveToDestinationInternal`로 합침).

> **폐기된 1차 설계 — 접근점 P.** 처음엔 T에서 FinalYaw 반대로 12m 물러난 접근점 P를 만들어 `[현재→P→T]`로
> 짜서 마지막 직선에서 방향이 저절로 맞게 하려 했다. 2차 실주행에서 **P가 도로 옆 숲 속 고립 내비메시 조각에
> 떨어져**(타깃포인트 yaw의 반대 = 도로 진입 방향이 아니라 숲) 경로가 partial이 났고, "partial이면 마지막 점을 P로
> 강제" 로직이 나무 사이로 직진시켜 충돌. **내비메시 밖/경로 밖에 찍은 점은 장애물 유무를 알 수 없다**는 게 교훈이라
> P 개념 자체를 제거했다(사용자 지적: "12m 뒤에 장애물이 없는지 어떻게 판단하고?"). 방향은 전부 3단계 선회가 맡는다.

1. **경로** — `MoveToDestination`과 완전히 같은 T까지의 내비메시 경로. 마지막 점은 정확히 T(도로 중앙 보정이 끌었거나
   경로가 T 조금 앞에서 끝났으면 되돌림). 단 경로탐색을 안 거친 직선은 **3m까지만** 허용 — 그보다 멀리 끊긴 partial
   경로면 억지로 직진시키지 않고 경로 끝을 도킹 기준점으로 삼아 거기서 방향만 맞춘다(경고 로그, 위치는 그만큼 어긋남).
2. **정밀 정지** — 거리는 **경로 잔여 길이**(`ComputeRemainingPathDistance`)로 잰다(반경으로 재면 굽어 들어오는
   목적지 앞에서 너무 일찍 감속).
   - 속도 거버너 후보 `√(v_creep² + 2·a·d)` — `DockCreepDistanceCm`(1m) 바깥은 제동 곡선, 안쪽은
     `DockCreepSpeedKmh`(3km/h) 크리핑. 크리핑 구간의 추종 목표는 T 자체(`FindChaosLookaheadTarget`)라 횡오차는
     자연히 수렴한다.
   - **크리핑 구간은 절대 서지 않는다** — 음수 속도적분(제동 잔류) 폐기, 조향컷 해제, 속도 < 크리핑이면
     `DockCreepMinThrottle`(0.15) 보장. 3차 실주행에서 T 1.5m 앞 2초 완전정지("도착했는데 왜 가만히 있지?")의 답.
   - 정지 판정(둘 다 만족): 속도 ≤ 크리핑×`DockStopSpeedFactor`(1.5) **and** `T까지 거리 ≤ DockStopToleranceCm(10) +
     v²/2·DockHandbrakeDecelMps2`. 감속도는 실측 **2m/s²**(1차 설계의 5는 오판 → 잔차 31cm). 크리핑 구간에서 T가 이미
     차체 뒤로 넘어갔으면(전방축 투영 음수) 속도 무관 즉시 정지.
   - `FindChaosLookaheadTarget`의 "도착" 반경(기본 150cm)을 도킹 중엔 `DockStopToleranceCm`로 줄인다 — 안 그러면
     1.5m 앞에서 스로틀이 끊겨 짧게 선다.
   - 지나쳐도 **후진으로 되돌리지 않는다** — 잔차는 4단계에서 스냅 한계 안이면 확정, 밖이면 그대로 둔다.
3. **Aligning** — 잔여 속도 28cm/s 이상이면 브레이크(비례 브레이크 모드라 후진으로 튀지 않음 — 파생 문서 참고). 서면
   **스로틀 0 + 방향오차 P 조향**으로 제자리선회(스킨스티어는 정지 상태에서도 돈다 — `SkidSteerTorque`가 스로틀과 독립).
   경로 방향 그대로 도착하므로 선회량은 최대 180°. 오버슈트 방지 2중: 오차 `AlignFineAngleDeg`(6°) 안쪽은 최소 조향을
   0.5→0.35로 낮추고, 현재 각속도로 0.3초 안에 목표를 지나칠 상황이면 조향을 놓아 관성으로 들어간다.
   `|오차| ≤ FinalHeadingToleranceDeg`(2.5°) 또는 `AlignTimeoutSeconds`(15초) → Settling.
4. **Settling → 스냅** — `SettleSeconds`(0.3초) 동안 속도<5cm/s·각속도<2°/s 유지를 확인 → 잔차 측정.
   `≤ SnapMaxPositionCm`(30) **and** `≤ SnapMaxYawDeg`(3)이면 `SnapBlendSeconds`(0.3초) ease-in-out 보간으로
   T의 **XY·Yaw로 확정(Z는 현재 지면 유지)**, 매 틱 속도 0으로 → `HandleMoveCompleted` → 핸드브레이크.
   **한계 밖이면 스냅하지 않고** 그 자리에서 완료 + 경고 로그 — 큰 거리를 순간이동하면 "옆으로 미끄러지는" 그림이 되기 때문.
5. **안전망** — 아직 T에 못 닿았는데 `AlignStartRadius`(2.5m) 안에서 `StalledNearDestinationSeconds`(1초)
   정지하면(장애물/부분경로) 그 자리에서 Aligning. Settling도 `AlignTimeoutSeconds`를 공유 상한으로 써서
   경사에서 미세하게 계속 미끄러져도 영원히 기다리지 않는다.

### 2.2 파라미터 (`UGV AI|Arrival`, 2026-09-16 3차 실주행 반영값)

| 프로퍼티 | 기본값 | 의미 |
|---|---|---|
| `bAlignFinalHeading` | true | 전역 스위치. 끄면 Facing이 MoveToDestination처럼 동작 |
| **`DockCreepDistanceCm`** | 100cm | 이 안쪽은 크리핑, 바깥은 제동 곡선 (300→100: 크리핑만 5초라 "왜 안 멈추지"로 보였음) |
| **`DockCreepSpeedKmh`** | 3 | 마지막 크리핑 속도 (2→3, 같은 이유) |
| `DockCreepMinThrottle` | 0.15 | 크리핑 구간 최소 스로틀(무정지 보장) |
| **`DockStopToleranceCm`** | 10 | 정지 목표. 위치 정확도의 핵심 노브 |
| `DockHandbrakeDecelMps2` | 2 | 정지거리 계산용 감속도(실측). 커브 제동 곡선의 `CornerDecelMetersPerSecSq`와 별개 |
| `DockStopSpeedFactor` | 1.5 | 속도가 크리핑×이 값 이하일 때만 정지 판정(제동 곡선 중 오판 방지) |
| `AlignStartRadius` | 250cm | 정체 안전망 반경(정상 도착 판정엔 안 쓰임) |
| `FinalHeadingToleranceDeg` | 2.5° | 선회 완료 허용각(1.5→2.5, 마지막 1°에 2.5초 걸림; 잔차는 스냅이 확정) |
| `AlignMinSteer` / `AlignFineMinSteer` | 0.5 / 0.35 | 제자리선회 최소 조향(거친/미세). 실측 0.35≈7°/s, 0.25≈3.5°/s |
| `AlignFineAngleDeg` | 6° | 미세 구간 진입각 |
| `AlignTimeoutSeconds` | 15 | 선회+정착 공유 타임아웃(최대 180° 선회 대비) |
| `StalledNearDestinationSeconds` | 1 | 정체 → 정렬 진입 대기 |
| **`SettleSeconds`** | 0.3 | 완전 정지 확인 시간 |
| **`bSnapToExactPose`** | true | 최종 스냅 on/off |
| **`SnapMaxPositionCm`** / **`SnapMaxYawDeg`** | 30 / 3 | 이 잔차 안에서만 스냅(15→30: 실측 잔차 29~59cm). 밖이면 스냅 없이 종료 |
| **`SnapBlendSeconds`** | 0.3 | 스냅 보간 시간. 0이면 즉시 |

삭제된 프로퍼티: `ApproachDistance`, `DockLateralGateCm`(접근점 설계와 함께).

**기대 정확도**: 도킹만으로 ±10~20cm·방향 ±2.5° 안팎 → 스냅 한계 안이면 **비트 단위 동일 자세**.
스냅 한계를 넘는 경우(경사에서 미끄러짐, 장애물로 정체)는 로그에 잔차가 찍히고 그 자세로 끝난다.

> ⚠ `BP_UGVAIController_new`에서 이 값들을 직접 바꿔둔 적이 있으면 헤더 기본값이 안 먹는다(BP 오버라이드 우선).
> Details의 Arrival 카테고리에 노란 되돌리기 화살표가 있으면 그 값은 BP 것.

### 2.3 시나리오 연동 (`UScenarioStateSubsystem`)

`MoveToDestination` 호출부 3곳 전부 TargetPoint 회전을 전달하도록 수정:

| 호출부 | 방식 |
|---|---|
| Zone 이동(`MoveUGVToZone1/2/3Destination` 효과) | `Destination`이 `ATargetPoint*`라 `GetActorRotation().Yaw` 직접 |
| 대기 위치(`BeginAllyApproach`) | `SetUGVStandbyDestinationFromTargetPoint(const ATargetPoint*)` 신설, Config 경로에서 사용. 좌표만 받는 기존 `SetUGVStandbyDestination(FVector)`은 방향 없이 유지(호출 시 이전 방향 지움) |
| 집결(`BeginAllyFormUp`) | 시그니처 유지(콘솔/BP 호출부 존재). `SetUGVFormUpFinalHeading(float)` 신설, Config 경로(`BeginAllyFormUpAndAdvance`)에서 `BeginAllyFormUp` 직전에 세팅 |

시나리오 리셋 시 방향 플래그도 함께 초기화.

### 2.4 테스트 콘솔 명령

| 명령 | 동작 |
|---|---|
| `MoveUGVFromTankToFacing (X=,Y=,Z=) <Yaw>` | 좌표 + 도착 요 직접 지정 |
| `MoveUGVToScenarioPoint <FormUp\|Zone1\|1 / Zone2\|2 / Zone3\|3 / Standby>` | 레벨 `AScenarioConfig`의 UGV TargetPoint로 **방향 정렬 포함** 단독 출발. 시나리오 상태는 건드리지 않음 |

후자는 `UScenarioStateSubsystem::MoveUGVToScenarioDestination(FString)`을 Server RPC로 감싼 것. 이전에는 Config
TargetPoint로 가는 경로가 스텝 테이블 효과 아니면 아군 집결까지 통째로 도는 `BeginAllyFormUpAndAdvance`뿐이었다.

### ⚠ 2.5 레벨에서 확인할 것

**`AScenarioConfig`의 UGV TargetPoint 4개(`UGVFormUpDestination` / `UGVStandbyDestination` / `UGVZone2Destination` /
`UGVZone3Destination`) 회전값이 실제 원하는 도착 방향이어야 한다.** 지금까지 회전이 무시됐으니 기본값 0°로 방치돼
있을 가능성이 크고, 그러면 UGV가 전부 월드 +X를 보고 선다.

---

## 3. 후진 조향 반전 — 코드로 확인한 답

시뮬레이션의 뒤집기 조건은 입력 부호가 아니라 **실제 이동 방향**이다:

```cpp
const bool bFlip = FlipSteerInReverse != 0 && ForwardSpeedCmS < -ReverseFlipThresholdCmS;   // -50cm/s
```

- 감속 중(앞으로 가며 브레이크): 속도 양수 → **안 뒤집힘.**
- 제자리선회(속도≈0): 임계 미달 → 안 뒤집힘. 정렬 단계에 안전.
- 실제로 뒤로 굴러갈 때만 뒤집힘(경사에서 멈췄다 밀리는 경우). 요레이트 PI는 실측을 되먹이므로 자체 적응하지만
  저속 P 분기는 반대로 조향할 수 있음 — 현재 후진 기동이 없어 미처리. 필요해지면 P 분기에 `sign(v_fwd)`.

정렬에 후진이 필요 없어 이 문제를 아예 피해간다. (단 **자율주행 중 브레이크 입력이 3.6km/h 미만에서 후진 기어로
바뀌는** 엔진 동작은 별개 문제였다 — 파생 문서 §1-③.)

---

## 4. 변경 파일 (전부 P4 체크아웃 상태에서 편집, 미제출)

- `Source/titan_example/Vehicles/UGVAIController.h/.cpp` — Slope Steering·Arrival 프로퍼티, `MoveToDestinationFacing`,
  `MoveToDestinationInternal`, `TickAlignment`, Tick 도착 판정 분기, 거버너 정지 곡선 + 크리핑 무정지 보정,
  `SpeedIntegralUnwindBoost`, `SetDriveMode`에서 비례 브레이크 모드 토글, 진단 로그
- `Source/titan_example/Vehicles/UGVWheeledVehicleMovementComponent.h/.cpp`, `UGVWheeledVehicleSimulation.h/.cpp` —
  파생 문서 참고(비례 브레이크 모드, 브레이크 조향)
- `Source/titan_example/UI/ScenarioStateSubsystem.h/.cpp` — `SetUGVStandbyDestinationFromTargetPoint`,
  `SetUGVFormUpFinalHeading`, `MoveUGVToScenarioDestination`, 호출부 3곳, 리셋 (제출됨)
- `Source/titan_example/titan_examplePlayerController.h/.cpp` — `MoveUGVFromTankToFacing`, `MoveUGVToScenarioPoint`(+Server RPC) (제출됨)

---

## 5. 검증 절차 (빌드 후)

1. `bLogPursuitDiagnostics` 켜고 `MoveUGVToScenarioPoint FormUp` — 로그 순서:
   `Facing: navmesh path to ... (complete), final yaw N — align in place on arrival` → (`cornerTarget`이 제동 곡선 →
   3km/h로 연속 하강, **`v=0.0`이 정지 판정 전에 나오면 안 됨**) → `Facing: dock stop (axis Ncm, lateral Ncm, radial Ncm, v=…)`
   (radial 20cm대, v 80cm/s대가 정상) → `[UGVPursuit] ALIGNING yawErr=…` → `Facing: aligned (x deg off) in Ns — settling`
   → `Facing: settled (pos Ncm, yaw N deg off) — snapping` → `Facing: snapped to exact pose … — done`.
   `completing WITHOUT snap` 경고가 나오면 잔차가 한계(30cm/3°)를 넘은 것 — 어느 축이 넘었는지 로그 값으로 확인.
   `PARTIAL PATH` 경고가 있으면 T 근처 내비메시 연결 문제(경로 끝이 3m 이상 짧으면 위치가 그만큼 어긋난 채 끝남).
2. 횡경사 구간 주행 — `slip` 부호/크기와 `rollBias` 확인. **오르막 쪽으로 파고들면** `SideslipGain` 0.7, `RollSteerGain` 0.3.
   반대로 여전히 밀리면 `SideslipGain` 1.3~1.5. **실주행 로그상 롤 피드포워드(`rollBias` -0.08~-0.15)를 요레이트 적분
   (`I` +0.05~0.07)이 반대 부호로 상쇄하는 흔적**이 있다 = A와 B가 같은 외란을 이중 보정. 코너링 느낌이 달라졌다면
   먼저 `bEnableRollFeedforward=false`로 A/B, 그 다음 `RollSteerGain` 0.2.
3. **위치 잔차가 크면**: 정지가 늦음 → `DockHandbrakeDecelMps2` ↓(1.5) 또는 `DockStopToleranceCm` ↑. 정지가 이름 →
   `DockHandbrakeDecelMps2` ↑.
4. **방향 잔차가 크면**: 선회가 오버슈트 → `AlignFineMinSteer` ↓ / `AlignFineAngleDeg` ↑. 선회가 안 끝나고 타임아웃 →
   `AlignMinSteer` ↑(정지마찰).
5. 스냅이 눈에 띄면 `SnapBlendSeconds` ↑(0.5), 반대로 잔차가 자주 한계를 살짝 넘어 스냅이 안 걸리면
   `SnapMaxPositionCm` ↑ — 단 그만큼 순간이동 거리도 늘어난다.

## 6. 미검증 / 남은 것

- [ ] **4차 실주행** — 크리핑 무정지·짧아진 크리핑·선회 단축·브레이크 조향 게이트 수정분 전부 미검증
- [ ] Config TargetPoint 4개 회전값 세팅(§2.5) — FormUp은 -161°로 확인됨, 나머지 3개 미확인
- [ ] 시케인 구간(08:20:47 / 11:32:23 두 번 충돌한 자리) — 브레이크 조향이 실제로 걸린 뒤에도 박히면 커브 값 튜닝
      (`CornerWindowDistance` 1500, `CornerMaxLateralAccelMps2` 1.8) 또는 그 자리 나무 프록시 반경
- [ ] 제자리선회의 회전 중심이 액터 원점과 다르면(스킨스티어 회전 중심 ≈ 좌우 휠 사이 중점) 선회 후 위치가 수 cm~수십 cm
      이동한다. 스냅 한계(30cm) 안이면 흡수되지만, 넘으면 "선회 전에 그 오프셋만큼 미리 어긋나게 세우기" 보정을 검토
- [ ] 후진 시 저속 P 분기 부호(§3) — 후진 기동이 생기면 그때
- [ ] 정지 상태에서 스로틀 1.0으로 나무를 계속 미는 "끼임" 상황에 대한 탈출 로직(후진 등)은 없음 — 1차 실주행에서 22초
      갇힘. 지금까지는 스스로 빠져나왔지만 시나리오 진행을 막을 수 있는 리스크

---

## 7. 실주행 이력 (2026-09-16, `MoveUGVToScenarioPoint FormUp`, 로그 `Saved/Logs/titan_example.log`)

| 회차 | 결과 | 원인 (로그 근거) | 조치 |
|---|---|---|---|
| 1차 | T 19.6m 앞에서 정지, 정렬 타임아웃, 스냅 없이 종료 | `dock stop (axis 1351cm, v=734cm/s)` — 정지거리 `v²/2a`의 `a`를 제동 곡선과 같은 2m/s²로 써서 곡선상에선 항상 `v²/2a≈d` → **감속 시작점에서 즉시 도착 판정**. 옆에서 P로 접근 중인데 축 투영 거리를 씀. 미세 선회 0.15 너무 약함 | 감속도 분리(5) + 속도 게이트 + 횡이탈 게이트, 미세 조향 0.25 |
| 2차 | 도착지 근방에서 반대로 꺾어 **나무 충돌** | `PARTIAL PATH: ends 592cm short` — 접근점 P(T 동쪽 12m)가 숲 속 고립 내비메시 조각. "partial이면 마지막 점을 P로 교체" 로직이 우로 62°→좌로 60° 지그재그로 나무 사이 직진 | **접근점 설계 폐기**(§2.1). 이 회차 첫 충돌(08:20:47 시케인)은 3.6km/h 이상 브레이크=풀브레이크 + 제동 중 요레이트 0 — 파생 문서 |
| 3차 | 시케인에서 1회 충돌(1.5초 후 탈출), 도착은 성공, T 1.5m 앞 2초 정지 후 재출발, 잔차 31cm 스냅 없음 | 비례 브레이크는 작동(`th=-0.21`에 2.4m/s²). 브레이크 조향은 게이트(`VehicleInputs.BrakeInput`)가 네트워크 예측 전용 멤버라 **한 번도 안 걸림**. 크리핑 구간 진입 시 속도 적분 -0.35 잔류 → 브레이크 → v=0. 정지거리 실측 1.8m/s² | 브레이크 게이트 수정, 크리핑 무정지 보정, `SpeedIntegralUnwindBoost`, 감속도 2, 크리핑 1m/3km/h, 선회 조향 0.5/0.35, 스냅 30cm |

레벨 쪽 변수: 같은 날 CL485(user3, "UGV 경로 및 에셋 변경")로 `New_kadex_0811.umap`과 `PN_interactiveSpruceForest`
spruce_small 메시가 바뀌었다. 새 spruce_small이 나무 프록시 빌더 Species에 없으면 콜리전은 있고 내비메시엔 없는
나무가 된다 — 시케인 충돌 자리와 관련 있는지 미확인.
