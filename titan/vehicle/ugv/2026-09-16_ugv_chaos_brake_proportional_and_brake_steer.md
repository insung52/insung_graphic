# UGV Chaos 브레이크 — 자율주행 브레이크가 ON/OFF였던 문제와 제동 중 조향 상실

2026-09-16 / 진행중 (구현 완료, 4차 실주행 검증 대기) / Chaos 차량의 `bReverseAsBrake`가 자율주행 컨트롤러의 비례 브레이크(-0.1~-0.6)를 전부 **풀브레이크**로, 정지 직전엔 **후진 스로틀**로 바꾸고 있었고, 브레이크 토크가 스키드스티어 차동 구동토크를 통째로 폐기해 **제동 중 조향이 0**이 되는 엔진 구조를 확인. 자율주행 전용 비례 브레이크 모드(`UUGVWheeledVehicleMovementComponent`)와 브레이크 조향(`UUGVWheeledVehicleSimulation`)으로 해결.

관련 문서: `2026-09-16_ugv_slope_steering_and_arrival_heading.md`(이 문제가 드러난 실주행 이력 §7),
`2026-08-22_ugv_corner_braking_dev_guide.md`(커브 감속 — 이 문서의 수정 전엔 그 튜닝값들이 실제 의미가 없었다),
`2026-08-26_ugv_track_lock_implementation_plan.md`(`UUGVWheeledVehicleSimulation` 구조)

---

## 0. 증상 (실주행 로그, `bLogPursuitDiagnostics`)

| 로그 | 관찰 |
|---|---|
| 10:41:16~19 (커브 앞) | `th=-0.23`(브레이크 23% 요청)인데 v 30→26→20→15→12→10→8→6km/h — 급제동 |
| 10:41:31~33 (90° 코너 진입) | `st=1.00, th=-0.27~-0.58`인데 **`yM=-0.5, 0.2, 0.1`** 1.5초 → 각도오차 12→37° → 조향컷으로 목표 13km/h → 코너 한복판 2차 급감속. 브레이크 풀리자(`th=+0.3`) 그제서야 `yM=37` |
| 08:20:48 (시케인, 충돌) | `th=-0.21` 동안 `yM≈0` 3샘플 → 풀조향에도 안 돌아 나무 충돌 |
| 10:41:37~47 (도킹 정지 후) | `v=2.9km/h th=-0.2`가 8초 유지 = **후진 크리핑**. 정지 판정 11cm였는데 최종 잔차 59cm |

사용자 표현: "90도 코너 앞에서 엄청 크게 감속됐다가 코너 도는 중간에도 엄청 크게 감속됨. 타이밍이 안 맞음."
→ 타이밍 문제가 아니라 **브레이크와 조향이 물리적으로 양립 불가**였던 것.

---

## 1. 엔진 소스로 확정한 원인 (UE 5.8)

### ① 브레이크 입력이 크기와 무관하게 1.0 — `ChaosVehicleMovementComponent.cpp:1146` `CalcThrottleBrakeInput`

```cpp
if (bReverseAsBrake)                       // 기본 true
    ...
    else if (RawBrakeInput > 0.f)
        if (VehicleState.ForwardSpeed > WrongDirectionThreshold)   // 100cm/s = 3.6km/h
        {  BrakeOut = 1.0f;  ThrottleOut = 0.0f; }                 // ← 크기 무시
```

자율주행 컨트롤러는 음수 ForwardInput을 `SetBrakeInput(|x|)`로 보낸다(`DispatchSetManualControl`). 3.6km/h 이상에선
그 값이 얼마든 **100% 브레이크**(`MaxBrakeTorque`×1.0, 휠 기본 1500Nm). 커브 감속 램프(`-0.06~-0.58`)가 전부 슬램 브레이크.

### ② 제동 중 스키드스티어 소멸 — `WheelSystem.cpp:92` `FSimpleWheelSim::Simulate`

```cpp
bool Braking = BrakeTorque > FMath::Abs(DriveTorque);   // 브레이크 > 구동이면 구동토크 통째로 폐기
```

우리 스키드스티어(`UUGVWheeledVehicleSimulation`)는 좌우 궤도의 **구동토크 차이**(±`SkidSteerTorqueNm` 900 × 속도감쇠,
바퀴당 100Nm 안팎). 브레이크 토크(바퀴당 1500Nm)가 이걸 덮으면 요 모멘트가 0. 즉 **브레이크를 밟는 동안은 조향 입력이
아무 일도 안 한다.** 코너 진입 = 감속 중이라 정확히 그 순간 못 돈다.

### ③ 정지 직전 브레이크 = 후진 — 같은 함수 + `UpdateState`

```cpp
// ProcessInputs_Internal (UpdateState)
else if (RawBrakeInput > 0 && RawThrottleInput <= 0 && GetTargetGear() >= 0)
    if (|ForwardSpeed| < WrongDirectionThreshold) SetTargetGear(-1, true);   // 3.6km/h 미만이면 후진 기어
// CalcThrottleBrakeInput
else if (GetTargetGear() < 0) { ThrottleOut = RawBrakeInput; BrakeOut = 0; }  // 브레이크 크기가 후진 스로틀
```

도킹 정지 직후 정렬 단계가 브레이크를 밟으면(28~100cm/s 구간) 후진 기어로 바뀌고 그 크기가 후진 스로틀이 된다.

### 참고 — 설계 의도
①③은 사람이 S키 하나로 "달리는 중엔 브레이크, 서면 후진"을 쓰게 하는 UX 편의. 수동 조작엔 맞다. 컨트롤러가 비례
브레이크를 낸다는 전제가 없을 뿐.

---

## 2. 해결

### 2.1 비례 브레이크 모드 — `UUGVWheeledVehicleMovementComponent::UpdateState` 오버라이드

`UpdateState(float)`(virtual, protected)는 엔진이 Raw 입력을 `ThrottleInput/BrakeInput`으로 바꾸는 지점. `Super` 호출 뒤
`bProportionalBrakeInput`이면:

```
ThrottleInput = InterpInputValue(dt, prev, clamp(RawThrottleInput, 0, 1))
BrakeInput    = InterpInputValue(dt, prev, |RawBrakeInput|)       // 원본과 같은 InputRate 램프
if (GetTargetGear() < 0) SetTargetGear(1, true)                    // 후진 기어 자동 전환 되돌림
```

- 게이트는 원본과 동일(`bRequiresControllerForInputs ? IsLocalController : true`, `PVehicleOutput` 존재).
- `AUGVAIController::SetDriveMode`가 **Auto 진입 시 on, Manual/Idle 시 off** — 수동 S키 후진은 그대로.
- `bReverseAsBrake` 자체를 끄지 않는 이유: 끄면 `else` 분기에서 "입력 없음 + 정지"일 때 자동 브레이크 1.0이 걸려
  **제자리선회(스로틀 0 + 조향)가 불가능**해진다. 켠 채로 결과값만 되돌리는 게 안전.
- 부작용: `ServerUpdateState` RPC는 부모가 보정 전 값으로 이미 보냄. 리모트 클라이언트의 로컬 시뮬 입력이 다를 수 있으나
  트랜스폼은 서버 물리 리플리케이션이 덮는다.

### 2.2 브레이크 조향 — `UUGVWheeledVehicleSimulation::ApplyWheelFrictionForces` (1b)

실제 궤도차량이 제동 중 도는 방식(한쪽 궤도 브레이크를 더 세게). `ApplyInput`이 바퀴마다 넣어둔 브레이크 토크를
조향 입력에 따라 좌우 궤도에 비대칭으로 다시 쓴다:

```
안쪽 궤도 × (1 + Blend·|s|),  바깥쪽 궤도 × (1 − Blend·|s|)    (합 보존)
우회전(+s): 좌측이 바깥(위 (1)에서 +SteerTorque를 받는 쪽) → 안쪽 = 우(Side 1)
```

- `Blend`=1, 풀조향이면 바깥 궤도 브레이크 0 → 그쪽 차동 구동토크(=조향 토크)가 살아나고, 안쪽은 잠겨 항력 → 요.
- 부분 조향(|s|=0.5)이면 바깥 0.5B < 그립이라 좌우 제동력 차이로 요. 단 풀브레이크(1.0)에선 양쪽 다 그립 포화라
  |s|가 커야 효과 — 2.1의 비례 브레이크가 전제.
- **실제 브레이크 입력(>0.01)이 있을 때만** — 스로틀 0의 엔진브레이크만 걸린 상태는 토크가 작아 차동이 살아 있고,
  `SetBrakeTorque`가 `bEngineBraking` 플래그를 지우는 부작용도 피한다.
- 수동 주행에도 적용된다(궤도차 거동). 손맛이 이상하면 cvar `p.UGV.SkidSteer.BrakeSteerBlend` 0.5 또는 0.

**함정(3차 실주행에서 밟음)**: 브레이크 입력 게이트를 부모의 `VehicleInputs.BrakeInput`으로 썼는데 이 멤버는
**네트워크 물리 예측이 켜진 경우에만 채워진다**(`ChaosVehicleManagerAsyncCallback.cpp:41-61`, `bUsingNetworkPhysicsPrediction`
아니면 `continue`). 항상 0 → 블록이 한 번도 안 돌았다. `ApplyInput(const FControlInputs&)` 오버라이드에서
`SteeringInput`과 같은 방식으로 `BrakeInput`을 직접 캐시하도록 수정.

### 2.3 커브 튜닝값에 미치는 영향

수정 전엔 `CornerDecelMetersPerSecSq`(제동 곡선 감속도) 등이 **실제 감속도와 무관**했다 — 목표 속도 아래로만 가면
풀브레이크. 이제 브레이크 크기가 비례로 들어가므로 커브 튜닝은 이 수정 뒤에 다시 하는 게 맞다. 튜닝값 목록은
`2026-08-22_ugv_corner_braking_dev_guide.md` + `UGVAIController.h` `UGV AI|Chaos Pursuit` 주석.

---

## 3. 검증 (3차 실주행, 2.2 게이트 수정 전)

- ✅ 비례 브레이크: `th=-0.21`에서 25.9→17.9km/h(≈2.4m/s²), `th=-1.0`일 때만 급제동. 정지 후 후진 크리핑 소멸.
- ❌ 브레이크 조향: 90° 코너 `th=-0.77~-1.0, st=1.00`에 `yM=-0.3, -0.0` 0.85초 — 게이트 버그(위 함정). 4차에서 재확인.
- 확인 방법: `th<0`인 샘플에서 `yM`(실측 요레이트)이 `yD`(목표)를 따라가는지. 0에 머물면 `p.UGV.SkidSteer.BrakeSteerBlend`
  값과 `ApplyInput` 캐시부터 확인.

## 4. 변경 파일 (P4 체크아웃, 미제출)

- `Source/titan_example/Vehicles/UGVWheeledVehicleMovementComponent.h/.cpp` — `SetProportionalBrakeInput`, `UpdateState` 오버라이드
- `Source/titan_example/Vehicles/UGVWheeledVehicleSimulation.h/.cpp` — `FUGVTrackLockParams::BrakeSteerBlend` + cvar,
  `BrakeInput` 캐시, `ApplyWheelFrictionForces` (1b)
- `Source/titan_example/Vehicles/UGVAIController.cpp` — `SetDriveMode`에서 모드 토글, include
