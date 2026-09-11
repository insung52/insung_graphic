# UGV_0901 서스펜션 승차감 튜닝

2026-09-10 / 완료 / 레벨의 작은 돌에 콜리전을 켠 뒤 덜컹거림을 부드럽게. Chaos 5.8 서스펜션이
힘이 아니라 **PBD 컨스트레인트**로 풀린다는 구조를 규명하고, `SpringPreload`·`RollbarScaling`이
**엔진에서 죽은 값**임을 확인.

---

## 1. 배경

`New_kadex_0811`의 작은 바위 액터들에 콜리전을 활성화해서 UGV 주행에 자연스러운 덜컹거림이
생기게 했다. 다만 충격이 너무 날카로워서 서스펜션을 무르게 잡는 작업.

기존 값은 2026-09-02 궤도→차륜 교체 때 **휠 개수 스케일 규칙**(`× 16/6`)으로 기계적으로
환산한 것이었다(`2026-09-02_ugv_0901_new_model_rig.md` §6). 그 규칙은 "구형과 물리적으로
등가"를 맞추기 위한 것이지 승차감을 본 게 아니다.

---

## 2. Chaos 5.8 서스펜션은 경로가 셋으로 갈린다

가장 중요한 구조. **차체가 튀는 것과 그립과 바퀴 비주얼이 서로 다른 코드로 계산된다.**

| 경로 | 결정하는 것 | 읽는 값 |
|---|---|---|
| `FPBDSuspensionConstraints::ApplySingle` | **차체가 실제로 튀는 정도** | `SpringRate`, `SuspensionDampingRatio`, `SuspensionMaxRaise/Drop` |
| `FSimpleSuspensionSim::Simulate` | 휠 하중 → **그립**(차체를 밀지는 않음) | `SpringRate`, `SuspensionDampingRatio`, `WheelLoadRatio` |
| `UChaosWheeledVehicleMovementComponent::GetSuspensionOffset` | **바퀴 본의 시각적 위치만** | `SuspensionSmoothing` |

### 2-1. 차체를 미는 것은 컨스트레인트다

`ChaosWheeledVehicleMovementComponent.cpp:1266-1272`가 차량 생성 시점에 휠 값을 컨스트레인트로
옮긴다:

```cpp
Constraint->SetHardstopStiffness(1.0f);
Constraint->SetSpringStiffness(Chaos::MToCm(Wheel->SpringRate) * 0.25f);
Constraint->SetSpringPreload(Chaos::MToCm(Wheel->SpringPreload));
Constraint->SetSpringDamping(Wheel->SuspensionDampingRatio * 5.0f);
Constraint->SetMinLength(-Wheel->SuspensionMaxRaise);
Constraint->SetMaxLength(Wheel->SuspensionMaxDrop);
Constraint->SetAxis(-Wheel->SuspensionAxis);
```

`ApplySuspensionForces`의 `AddForceAtPosition`은 **`p.Vehicle.DisableConstraintSuspension`이
켜졌을 때만** 쓰인다(같은 파일 `:658`). 기본 경로가 아니다.

솔버 본체(`PBDSuspensionConstraints.cpp:570-585`):

```cpp
FReal SpringCompression = AxisDotNormal * (Setting.MaxLength - Distance) /*+ Setting.SpringPreload*/;
FReal SpringVelocity    = FVec3::DotProduct(ArmVelocity, SurfaceNormal);
const FReal S = SpringMassScale * Setting.SpringStiffness * Dt * Dt;
const FReal D = SpringMassScale * Setting.SpringDamping   * Dt;
DLambda = (S * SpringCompression - D * SpringVelocity);
if (DLambda < 0) { DLambda = 0; }   // 서스펜션은 차체를 끌어내릴 수 없다
```

여기서 세 가지가 나온다:

1. **`SpringPreload`가 주석 처리돼 있다** → §3.
2. **강성이 `Dt²`에 비례한다** → §7의 프레임률 주의.
3. 압축량은 `MaxLength - Distance`이고 `Distance`는 `[MinLength, MaxLength]`로 잘리므로
   **압축량 상한 = MaxRaise + MaxDrop**. 그 이상은 아무리 세게 밟혀도 힘이 안 늘고, 남은
   충격은 그대로 차체 운동량이 된다.

### 2-2. 그립은 별도 계산

`ApplySuspensionForces`의 `PSuspension.Simulate()` 결과는 차체를 밀지 않고
`PWheel.SetWheelLoadForce()`로만 들어간다(`:665`):

```cpp
ForceMagnitude = WheelLoadRatio * ForceMagnitude + (1 - WheelLoadRatio) * RestingForce;
```

`RestingForce`는 `SpringPreload`가 아니라 **`스프렁매스 × g`를 엔진이 직접 계산**한
값이다(`:1667`). 이쪽 `DampingRatio`는 `ComputeDamping()`에서 **1.0으로 클램프**되지만
(`SuspensionUtility.h:55`), 컨스트레인트 쪽은 클램프가 없다 — 같은 프로퍼티인데 두 경로의
유효 범위가 다르다.

### 2-3. 비주얼은 또 다른 경로

`FSimpleSuspensionSim::GetSpringLength()`(이동평균)를 호출하는 곳은 엔진 전체에서
`GetSuspensionOffset()` 하나뿐이고(`SuspensionSystem.h:196`), 그건 애니메이션(`WheelController`
노드)·디버그 드로우·스냅샷만 쓴다. **`SuspensionSmoothing`은 물리에 전혀 닿지 않는다.**

게임 스레드 쪽에도 **두 번째 스무딩**이 따로 있다(`ChaosWheeledVehicleMovementComponent.cpp:2875`):

```cpp
float InterpolationMultiplier = 1.0f - (Wheel->SuspensionSmoothing / 11.0f);
Offset += (NewOffset - CachedState[WheelIndex].WheelOffset) * InterpolationMultiplier;
Offset = FMath::Clamp(Offset, -Wheel->SuspensionMaxDrop, Wheel->SuspensionMaxRaise);
```

즉 값 하나로 **피직스 스레드 이동평균 + 게임 스레드 지수보간**이 겹쳐 걸린다. 0이면 배율이
1.0이라 스무딩이 아예 없다. 마지막 줄 때문에 **바퀴 본의 시각적 가동 범위는 정확히
`[-MaxDrop, +MaxRaise]`** — `MaxRaise`를 올리면 바퀴가 그만큼 차체 안으로 더 들어간다.

---

## 3. ★ 죽어 있는 값 2개

### `SpringPreload` — 엔진에서 주석 처리됨

§2-1의 `/*+ Setting.SpringPreload*/`. `SetSpringPreload()`로 컨스트레인트에 값이 들어가긴
하는데 솔버가 안 읽는다. `FSimpleSuspensionSim::Simulate()`도 `SpringRate`와 댐핑만 쓴다.
**UE 5.8에서 이 프로퍼티는 어떤 경로로도 동작하지 않는다.**

> 09-02에 `180 → 450 (× 16/6)`으로 올린 건 무해하지만 효과가 0이었다. 지금도 450으로 두었다 —
> 엔진이 나중에 주석을 풀 가능성이 있어 값 자체는 스케일이 맞는 상태로 남겨두는 편이 낫다.

### `RollbarScaling` — 축 그룹이 안 만들어짐

`RecalculateAxles()`가 축을 **휠 클래스 포인터 기준**으로 묶는다
(`ChaosWheeledVehicleMovementComponent.cpp`, `AxleToWheelMap.Add(Wheel, ...)`). UGV_0901은 6륜이
전부 `BP_UGV_Wheel_0901` 하나를 쓰므로 **6륜짜리 축이 딱 1개** 만들어진다. 그런데 롤바 코드는

```cpp
//#todo: only works with 2 wheels on an axle at present
if (Axle.Setup.WheelIndex.Num() == 2)
```

가드에 막혀 통째로 스킵된다. **이 차량은 안티롤바가 0이다** — 한쪽 바퀴만 돌을 타면 차체가
복원력 없이 그대로 기운다.

살리려면 휠 클래스를 앞/중/뒤 3개로 복제해서 `WheelSetups`에 축별로 배정해야 한다. 이번엔
안 했다(§6).

---

## 4. 최종 값

`BP_UGV_Wheel_0901` — 6륜 공용.

| 프로퍼티 | 09-02 | **09-10** | 의도 |
|---|---|---|---|
| `SpringRate` | 900 | **200** | cm당 반발력을 낮춰 충격 완화. 사용자가 직접 잡은 값(제안값 700보다 훨씬 무름) |
| `SuspensionMaxRaise` | 12 | **16** | 압축 여유 확보. 차고는 안 변함(§5) |
| `SuspensionSmoothing` | 0 | **5** | 바퀴 본 떨림 억제(비주얼 전용) |
| `SuspensionMaxDrop` | 12 | 12 | 유지 |
| `SuspensionDampingRatio` | 0.7 | 0.7 | 유지 |
| `WheelLoadRatio` | 0.5 | 0.5 | 유지 |
| `SpringPreload` | 450 | 450 | **죽은 값**(§3) |
| `RollbarScaling` | 0.15 | 0.15 | **죽은 값**(§3) |
| `SweepShape` / `SweepType` | Spherecast / SimpleSweep | 유지 | 이미 최선 |

무브먼트 컴포넌트(`Mass 1500`, `CenterOfMassOverride (0,0,10)`, `ChassisWidth/Height 175/146`)는
안 건드렸다. 나머지 휠 값(`WheelRadius 31.6`, `MaxBrakeTorque 400`, `CorneringStiffness 2670`
등)도 09-02 그대로.

컨스트레인트에 실제로 들어가는 값으로 환산하면:

| | 09-02 | 09-10 |
|---|---|---|
| `SpringStiffness` = `SpringRate × 100 × 0.25` | 22500 | **5000** |
| `SpringDamping` = `DampingRatio × 5` | 3.5 | 3.5 |
| `MinLength` / `MaxLength` | −12 / 12 | **−16** / 12 |
| 압축량 상한 (`MaxRaise+MaxDrop`) | 24 cm | **28 cm** |

---

## 5. 각 값이 실제로 하는 일

**`SuspensionMaxRaise`** — 컨스트레인트에서 `MinLength = -MaxRaise`로만 쓰인다. 압축량 계산
자체는 `MaxLength - Distance`, 즉 `MaxDrop` 기준이므로 **MaxRaise를 올려도 어느 위치에서든
스프링 힘이 바뀌지 않는다 = 차고가 안 변한다.** 순수하게 "바닥 치기 전까지의 여유"만 늘어난다.
바퀴 반지름 31.6cm에 압축 트래블이 12cm였다는 건 **12cm 넘는 돌을 밟는 순간 트래블이 끝나고
충격이 감쇠 없이 차체로 직행**했다는 뜻이라, 이번 변경의 핵심이다.

부수 효과 2가지:
- 비주얼 클램프가 `+16`으로 늘어 바퀴가 차체 안으로 4cm 더 들어간다 → 휠 메시가 헐/펜더를
  뚫지 않는지는 눈으로 봐야 한다.
- `FSimpleSuspensionSim`의 `MaxLength = MaxRaise + MaxDrop`도 24 → 28로 커져서 하중 추정이
  올라간다. `WheelLoadRatio 0.5`라 절반만 반영되지만, 실주행으로 확정한 제동 성능
  (`MaxBrakeTorque 400` → 5.06 m/s²)이 미묘하게 달라질 수 있다.

**`SuspensionMaxDrop`** — 이쪽은 `MaxLength`라서 올리면 모든 위치에서 압축량이 같이 커진다 =
**차고가 올라가고 스프링 힘이 세진다.** MaxRaise와 성격이 완전히 다르다. 그래서 안 건드렸다.

**`SpringRate`** — 컨스트레인트 강성이자 그립 계산의 스프링 상수. 900 → 200은 4.5배 무른
것이라 정지 시 압축량도 그만큼 깊어진다. MaxRaise를 12에서 16으로 먼저 늘려둔 게 이 값을
쓸 수 있게 해준 전제다.

**`SuspensionDampingRatio`** — 컨스트레인트는 **압축/신장 댐핑을 하나의 값으로 공유**한다
(`FSimpleSuspensionSim` 쪽만 `CompressionDamping`/`ReboundDamping`이 갈리는데 그건 차체를
안 민다). 댐퍼 힘은 속도 비례라 작은 돌처럼 짧고 빠른 입력에서 제일 세게 나오므로, 아직
날카롭다고 느껴지면 **여기를 0.45~0.5로 내리는 게 다음 후보**다. 대가는 차체 출렁임 증가
(RCWS 조준 안정성과 트레이드오프).

**`WheelLoadRatio`** — 승차감이 아니라 "돌 밟을 때 순간적으로 접지가 빠져 울컥하는" 쪽.
0에 가까울수록 휠 하중을 균등하게 가정해 그립이 일정해진다.

---

## 6. 안 한 것 / 후속 후보

우선순위 순.

1. **`SuspensionDampingRatio` 0.7 → 0.45~0.5** — 아직 날카로우면 여기부터.
2. **휠 클래스 3분할(앞/중/뒤)** — `RollbarScaling`을 처음으로 살리고, 동시에 **축별 스프링
   분리**(앞은 무르게 / 뒤는 단단하게)가 가능해진다. 요철에서 피칭 잡는 정석. 에셋 3개 증가.
   휠 **개수**는 그대로이므로 09-02의 스케일 규칙(`CorneringStiffness` 등)은 재계산 불필요.
3. **`WheelLoadRatio` 0.5 → 0.3**.
4. **바위 콜리전 품질** — `SM_BHF_Rock*`는 자동 분해된 **컨벡스 헐 4개(각 16버텍스)**라 면이
   거칠다(`CollisionTraceFlag = CTF_UseDefault`, `SweepType`이 `SimpleSweep`이므로 이 헐을
   탄다). 특정 바위만 "턱"처럼 느껴지면 그 메시의 헐 수를 늘리는 게 서스펜션보다 직접적이다.

---

## 7. 측정 / 반복 방법

**PIE 중 실시간 변경** — `UChaosWheeledVehicleMovementComponent::SetSuspensionParams(Rate,
Damping, Preload, MaxRaise, MaxDrop, WheelIndex)`가 BlueprintCallable이다. 휠 인덱스별로 호출해야
하지만 에디터 재시작 없이 값을 바꿔가며 감을 잡을 수 있다. 내부에서 `FSimpleSuspensionSim`과
컨스트레인트 양쪽을 다시 세팅한다.

**디버그 드로우**

```
p.Vehicle.ShowSuspensionLimits 1     // 트래블 한계 — 바닥 치는지 바로 보임
p.Vehicle.ShowSuspensionForces 1     // 힘 벡터
p.Vehicle.ShowSuspensionRaycasts 1   // 스피어캐스트가 돌을 어떻게 타는지
p.Vehicle.DisableConstraintSuspension 1  // 힘 기반 서스펜션으로 전환(비교용, 하드스톱 없음)
```

**⚠ 프레임률 의존성** — `bTickPhysicsAsync=false`라 물리 dt = 프레임 dt이고, §2-1에서 보듯
강성이 `Dt²`에 비례한다. 60fps에서 잡은 느낌이 30fps에서는 **강성 4배**가 된다
(`MaxPhysicsDeltaTime` 1/30 클램프까지). `Config/DefaultGameUserSettings.ini`의
`FrameRateLimit=60`이 실제로 걸린 상태에서 튜닝할 것. 전시 PC 프레임이 흔들리면 승차감도
같이 흔들린다.

---

## 8. 함정 요약

- 서스펜션은 **힘이 아니라 PBD 컨스트레인트**로 풀린다. 힘 기반 코드(`AddForceAtPosition`)를
  읽고 튜닝 판단을 하면 틀린다 — 그 경로는 `p.Vehicle.DisableConstraintSuspension` 전용이다.
- **`SpringPreload`는 UE 5.8에서 완전히 죽은 값**이다(솔버에서 주석 처리).
- **`RollbarScaling`도 죽어 있다** — 축이 휠 클래스 기준으로 묶여 6륜이 한 축이 되고,
  롤바 코드는 `축당 휠 2개`만 처리한다.
- **`MaxRaise`와 `MaxDrop`은 대칭이 아니다.** MaxRaise는 여유만 늘리고(차고 불변),
  MaxDrop은 차고와 스프링 힘을 같이 올린다.
- **`SuspensionSmoothing`은 물리에 안 닿는다.** 대신 값 하나로 스무딩이 두 겹 걸리고
  (PT 이동평균 + GT 지수보간), 바퀴 본의 시각적 가동 범위를 `[-MaxDrop, +MaxRaise]`로 자른다.
- `SuspensionDampingRatio`는 **경로마다 유효 범위가 다르다** — 그립 계산은 1.0에서 클램프,
  컨스트레인트는 클램프 없음.
- 휠 6개가 **같은 휠 클래스 하나**를 공유하므로 축별/위치별 튜닝이 원천적으로 불가능하다.
  나누려면 클래스를 복제해야 한다.

---

## 관련 문서

- `2026-09-02_ugv_0901_new_model_rig.md` §6 — 휠 반지름·개수 변경 시의 스케일 규칙, 09-02
  시점 전체 튜닝 값.
- `2026-09-03_ugv_0901_bp_to_cpp.md` — BP 로직의 C++ 이관. 이제 BP는 컴포넌트와 **튜닝 값만**
  든 데이터 에셋이라, 이 문서의 값들이 곧 BP의 전부다.
- `2026-08-26_ugv_track_lock_implementation_plan.md` — 스키드 스티어/TrackLock C++
  (`UGVWheeledVehicleSimulation`). 서스펜션에는 손대지 않는다(`ApplySuspensionForces`
  오버라이드 없음) — 확인함.
