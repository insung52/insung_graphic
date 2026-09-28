# AI 포즈 층 3종 — 스캔 몸 회전 · 걸음 브리지 · 자세 축 스무더 (+ 관전 폰 비행 속도 · 롤 잔류)

2026-09-18 / 완료(사용자 PIE 확인) / AI 세션이 발행한 계약 4개(`GetAimPoint()`/`IsScanning()` · `GetDesiredGait()` · `GetPoseUrgency()`)를 포즈 층이 **컴포넌트 3개**로 소비한다 — 총 내린 idle 의 캡슐 회전(`SoldierScanTurnComponent`) · Walk→GASP `WantsToWalk`(`SoldierGaitBridgeComponent`) · 자세 축 4종의 사다리꼴 프로파일(`SoldierPoseSmootherComponent`). 스무더의 "급하면 1프레임 스냅"은 **`RampAxisTo` 가 rate ≤ 0 을 '램프 없음 = 목표 반환'으로 읽는 것**이 원인이었다(P167). 같은 라운드에 관전 폰 휠 비행 속도 · `slomo` 무관 카메라 · 롤 잔류 수정.

`ai/2026-09-18_patrol_scan_and_move_robustness.md`(AI 세션 — 계약을 **발행**한 쪽)의 짝. 이 문서는 **소비**하는 쪽이다. 머리 추종(H)·1인칭 쪽 최종 설계는 그대로 `animation/2026-09-14_sight_alignment_plan.md` 0' 절.

---

## 0. 한 장 요약 [A]

| 컴포넌트 | 파일 (`Source/SoldierLab/Pose/`) | BP 컴포넌트 | 읽는 계약 | 쓰는 곳 | 디버그 |
|---|---|---|---|---|---|
| **스캔 몸 회전** | `SoldierScanTurnComponent.{h,cpp}` | `AC_SoldierScanTurn` | `WantsToAim()` · `IsScanning()` · `HasContact()` · `GetAimPoint()` | 캡슐 yaw (`SetActorRotation`) | `SoldierLab.Debug.ScanTurn 1` |
| **걸음 브리지** | `SoldierGaitBridgeComponent.{h,cpp}` | `AC_SoldierGaitBridge` | `GetDesiredGait()` | BP `CharacterInputState.WantsToWalk` (리플렉션) | — |
| **자세 축 스무더** | `SoldierPoseSmootherComponent.{h,cpp}` | `AC_SoldierPoseSmoother` | `GetDesiredStance/Lean/BlindFireH/V()` · `GetPoseUrgency()` | BP `StanceAxis`+`AITargetStance` · `LeanCurrent`+`AITargetLean` · `BlindFireH`+`AITargetBlindFireH` · `BlindFireV`+`AITargetBlindFireV` | `SoldierLab.Debug.PoseSmooth 1` |

셋 다 **AI 전용** — `Pawn->IsPlayerControlled()` 면 틱에서 즉시 반환. 플레이어 병사의 축은 키가, 몸 회전은 마우스가, 걸음은 Ctrl 이 그대로 몬다. 셋 다 `USoldierEngagementComponent` 를 **틱 선행 조건**으로 등록(이번 프레임의 목표를 읽는다). 값은 전부 `EditAnywhere`(`SoldierLab|ScanTurn|…` · `SoldierLab|Gait|…` · `SoldierLab|PoseSmooth|…`).

**H(머리 추종)와의 관계 — 없다.** `USoldierHeadAimComponent::bEnabled` 는 기본 false 이고 **H 키로만** 켜진다(플레이어 1인칭, 또는 관전 중 그 병사에게). AI 병사에게 자동으로 켜는 로직은 **없다** — 09-17 에 한 번 넣었던 `bEnableForAI`/`bTurnBodyForAI` 는 사용자 결정("H 는 1인칭 플레이 부가 기능, 노출되면 안 됨")으로 **되돌렸고**, AI 의 몸 회전은 별도 컴포넌트(ScanTurn)로 갔다. 헤더 주석에도 그렇게 박혀 있다.

---

## 1. `USoldierScanTurnComponent` — 총 내린 idle 의 몸이 눈을 따라 돈다 [A]

### 1.1 왜

교전 층이 눈(컨트롤 회전)을 돌린다 — 접촉이면 표적, 없으면 안 훑은 방위(`IsScanning()`). 그런데 그 상태에서 **몸**을 돌리는 것이 아무것도 없었다: GASP 의 orient-to-movement 는 움직일 때만, aim 모드는 견착일 때만 캡슐을 돌린다. 총 내리고 서 있는 수비수는 눈이 옆구리를 봐도 몸은 마지막 전투 쪽 — "화살표가 몸과 반대"(AI 세션 09-18 사용자 보고).

### 1.2 게이트 (전부 AND, `SoldierScanTurnComponent.cpp`)

```
bEnabled ∧ AI ∧ Engagement 있음
∧ !Engagement->WantsToAim()                          ← 견착이면 GASP aim 모드가 몸을 돌린다. 여기선 손 뗌
∧ (!bOnlyWhileScanningOrInContact ∨ IsScanning() ∨ HasContact())
∧ 정지: 지면 ∧ 가속 0 ∧ Velocity2D ≤ MaxSpeedCms 10  ← 움직이면 CMC OrientToMovement 몫
∧ GetAimPoint() 유효
→ Δyaw = 조준 방위 − 캡슐 yaw
   |Δ| > StartDegrees 20 이면 회전 시작, |Δ| ≤ StopDegrees 5 면 정지 (히스테리시스)
   회전 중: 캡슐 yaw 만 TurnRateDegPerSec 180 으로 (0 = 스냅)
```

메시는 건드리지 않는다 — GASP `OffsetRootBone` 이 메시를 붙잡고 MM 이 turn-in-place 클립을 고른다. 조준 모드에서 마우스를 홱 돌릴 때와 **같은 경로**(P119 와 동일 원리, 플레이어 몸 회전 `bTurnBodyAfterLook` 의 AI 판). 180°/s 는 교전 층의 조준 선회 240°/s 보다 낮게 — 몸이 눈을 **따라가지** 앞지르지 않게.

### 1.3 디버그

`SoldierLab.Debug.ScanTurn 1` — 시안 화살표 = 조준 방위, 흰 = 캡슐 전방, 라벨 `scanturn SCAN|contact d=<Δ°> [TURN]`. `SCAN` 은 `IsScanning()`, `contact` 는 접촉. 관전 시 `SoldierLab.Debug.AI.Self 0` 필요.

### 1.4 알려진 틈

- [W89] `IsScanning()` 은 `bHaveWatch` 라서 **섹터만 있고 굽기 전**엔 false → 기본값(`bOnlyWhileScanningOrInContact true`)에서 그 순간은 안 돈다. 눈에 띄면 옵션 false 또는 교전 층에서 `bScanning = bHaveWatch ∥ bHasSector`. AI 세션과 조율.
- 총 내리고 **걷는** 중엔 이 컴포넌트가 손을 떼고 CMC OrientToMovement 가 진행 방향으로 돌린다 — 이동 중 볼 곳은 AI 세션의 `WatchTravelBias` 가 진행 방향으로 몰아 두므로 충돌 없음.

---

## 2. `USoldierGaitBridgeComponent` — Walk 가 GASP 에 닿게 [A]

### 2.1 왜

GASP 는 입력 상태 구조체에서 gait 를 고른다: `WantsToSprint`(∧ 가능) → Sprint, 아니면 `WantsToWalk` → Walk, 아니면 Run. BP 브리지가 `WantsToAim` 과 `WantsToSprint` 는 매 틱 밀어 넣고 있었지만 **Walk 는 길이 없어서** AI 세션이 "안정 상태면 편안히 걷는다"(`GetDesiredGait()==Walk`)를 만들어도 병사는 조깅했다.

### 2.2 무엇을

매 틱 `CharacterInputState.WantsToWalk = (Engagement->GetDesiredGait() == ESoldierGait::Walk)` 를 **리플렉션으로** 쓴다. `bDriveSprint` 기본 **false** — Sprint 필드는 BP 브리지가 계속 소유(P89 스프린트 규칙은 이미 BP 경로로 들어간다). 매 틱 쓰는 이유: BP 가 자기 Break/Make 로 구조체를 통째로 다시 쓰더라도 순서 무관하게 값이 남게.

### 2.3 함정 — BP 구조체 필드 이름엔 GUID 가 붙는다 (P170)

`FSoldierPlayerInputState` 류 BP 정의 구조체의 실제 `FProperty` 이름은 `WantsToWalk_2_ABCD1234…` 다. `FindPropertyByName(TEXT("WantsToWalk"))` 는 **null**. `TFieldIterator<FBoolProperty>` 로 돌며 `GetAuthoredName() == 원함 ∨ 이름 == 원함 ∨ 이름.StartsWith(원함 + "_")` 로 잡는다(`FindStateBool`). 못 찾으면 `LogTemp` 경고 1줄 후 무력.

---

## 3. `USoldierPoseSmootherComponent` — 서보가 아니라 몸 [A]

### 3.1 왜

교전 층은 축의 **목표**만 낸다(stance · lean · BF-H · BF-V). BP 는 각 축을 **플레이어 키 스텝 rate 로 등속** 램프했다 → AI 의 앉기가 딱 시작하고 딱 멈추며, 목표가 도중에 바뀌면 그 자리에서 다시 등속. 사용자 요구(AI 세션 브리핑): 사다리꼴 속도 · 목표 변경 시 현재 속도에서 이어감 · 앉기는 내려갈 때 더 빠르게 · `GetPoseUrgency()`(0~1) 로 스케일 · 축별 속도는 Details 노출.

### 3.2 프로파일 (`StepAxis`, 축당 매 틱)

```
d       = clamp(Target) − Value
MaxSpd  = (d ≥ 0 ? MaxSpeedUp : MaxSpeedDown) × Scale
Accel   = Acceleration × Scale
Wanted  = sign(d) · min(MaxSpd, sqrt(2·Accel·|d|))     ← 사다리꼴: 순항 또는 "지금 감속하면 딱 목표에 서는" 속도
Vel     = FInterpConstantTo(Vel, Wanted, dt, Accel)    ← 속도가 Accel 로 Wanted 를 쫓는다 → 목표가 바뀌어도 속도 연속
Value  += Vel · dt
|d| ≤ 0.002 ∧ |Vel| ≤ 0.02 → Value = Target, Vel = 0   (헌팅 대신 착지)
Value 가 [Min,Max] 벽에 닿으면 Vel = 0                  (벽은 벽)
Scale   = max(0.05, lerp(ScaleAtCalm 0.5, ScaleAtUrgent 1.6, GetPoseUrgency()))
```

축별 기본값(`FSoldierPoseAxisMotion{MaxSpeedUp, MaxSpeedDown, Acceleration}`, 단위 = 축 단위/s · /s²):

| 축 | Up | Down | Accel | 비고 |
|---|---|---|---|---|
| Stance (0 서다 → 1 앉다) | **1.6** | **0.9** | 5 | "Up" = 1 쪽 = **앉는 방향**이 빠르다 |
| Lean (−1..1) | 1.2 | 1.2 | 4 | |
| BlindFireH (−1..1) | 1.2 | 1.2 | 4 | |
| BlindFireV (0..1) | 1.2 | 1.2 | 4 | |

전부 [C] — 사용자 "평상시는 괜찮아 보임" 이후 숫자 조정은 없었다.

### 3.3 BP 와의 접합 — 상태와 목표를 **같이** 쓴다

BP 그래프는 매 틱 `AITarget*` ← 교전 목표 를 다시 읽고 `RampAxisTo(상태, AITarget, Rate)` 로 상태를 민다. 그래프를 안 고치고 그 램프를 통과시키는 법: 스무더가 **상태 변수와 AI 목표 변수에 같은 값**을 쓴다 → 램프가 덮을 거리가 0 → 값이 그대로 통과. 여기에 두 가지가 더 필요했다.

1. **틱 순서** — 이 컴포넌트는 Engagement 뒤 · **액터 틱 앞**(`Owner->AddTickPrerequisiteComponent(this)`). 그래야 BP 가 이번 프레임의 부드러운 값으로 crouch/골반/속도 상한을 파생한다.
2. **BP 램프 rate 를 얼려야 한다** — BP 가 `AITarget*` 를 매 틱 다시 덮으므로 목표만 써서는 부족. `RateVariablesToFreeze = {StanceRate, BlindFireRate}` 를 첫 틱에 인스턴스 변수로 덮는다(플레이어 병사는 안 건드림 → 자기 rate 유지). 린 램프의 rate 는 그래프 **리터럴 핀**이라(`K2Node_CallFunction_92.RatePerSecond`) BP 에서 직접 바꿨다.

`AIPoseDriven`(BP bool) 이 false 면 전부 건너뛴다 — BP 와 같은 스위치를 본다.

### 3.4 ★ 함정 — "급하면 1프레임에 스냅" 의 원인 (P167)

첫 판은 얼린 rate 를 **0** 으로 쓰고 린 핀도 **0.0** 으로 놨다. 평상시엔 멀쩡했는데 급한 상황(urgency 높음)에서 stance 가 한 프레임에 목표로 뛰었다. `ext` 감사(아래)는 0 — 다른 손이 쓴 게 아니다.

원인은 `USoldierAxisLibrary::RampAxisTo` (`Math/SoldierAxisLibrary.cpp:19-22`):

```cpp
if (DeltaSeconds <= 0.f || RatePerSecond <= 0.f)
{
    return Target;      // ← "램프 없음" = 목표를 그대로
}
```

rate 0 은 "정지"가 아니라 **"즉시"** 다. 스무더가 상태·목표를 같은 값으로 써도, 그 다음 BP 가 `AITarget*` 를 **raw 교전 목표**로 다시 덮고 램프를 부르면 rate 0 이 raw 목표를 돌려준다 → 매 틱 스냅. 평상시에 안 보였던 건 raw 목표와 부드러운 값의 거리가 작아서였다(`Scale 0.5` 로 램프가 느리니 급할 때만 벌어진다).

고침: `FrozenRate 0.0001`(양수 극소값, `FMath::Max(KINDA_SMALL_NUMBER, FrozenRate)`) — 프레임당 0.0000017 이동 = 사실상 정지, 스냅 분기는 안 탄다. 린 핀도 `0.0001` 로 다시 저장. **사용자 확인 "해결완료"(09-18).**

### 3.5 디버그 — `SoldierLab.Debug.PoseSmooth 1`

머리 위 한 줄: `pose u=<urgency> x<Scale>  st <값>(<속도>)><목표> ext<±>  lean … bfH … bfV …`.

- `값(속도)>목표` — 속도가 0 → ±순항 → 0 으로 사다리꼴을 그리면 정상. 급할 때 `(±2.x)` 근처까지 올랐다 내려온다.
- `ext` = 지난 틱에 우리가 쓴 값 − 이번 틱에 읽은 값. **0 이 아니면 다른 손이 축을 쓰고 있다**(BP 램프가 안 얼었거나 옛 브리지) — 그 값은 진실로 받아들여 이어간다(되돌려 스냅하지 않음). 3.4 의 스냅에서 ext 가 0 이었던 것이 "우리 뒤에 누가 쓴 게 아니라 **램프 자체가 목표를 돌려준 것**" 의 단서였다.

### 3.6 남은 후보 — 이건 스냅이 아니다

축이 부드러워도 **GASP 크라우치 DB 전환**은 여전히 이산이다: `StanceThreshold 0.5` 문턱을 넘는 순간 `Crouch()`/`UnCrouch()` 로 MM 데이터베이스가 바뀐다(IMPLEMENTED 2.5f-4). 골반 높이 역산(2.5f-5)으로 **높이** 팝은 없지만 클립 선택이 바뀌는 "뚝" 은 GASP 블렌드(`BlendListByBool` 시간 · 관성화) 몫. 지금은 눈에 안 띈다 → [W91].

---

## 4. 관전 폰 — 휠 비행 속도 · `slomo` 무관 · 롤 잔류 [A]

`Observer/SoldierObserverPawn.{h,cpp}`. 09-14 문서(`ai/2026-09-14_cover_frame_fix_and_observer.md`)의 후속.

| 항목 | 내용 |
|---|---|
| **휠 = 비행 속도** (추적 안 할 때) | `FlySpeedCms 1200`, 노치마다 × / ÷ `FlySpeedWheelFactor 1.25`, `MinFlySpeedCms 100` ~ `MaxFlySpeedCms 20000`. `ApplyFlySpeed()` 가 `UFloatingPawnMovement` 의 MaxSpeed 와 Accel/Decel 을 **같은 비율로** 스케일. 추적 중엔 휠 = 3인칭 거리(기존, `WheelStepCm 50`) |
| **`bIgnoreTimeDilation` true** | `CustomTimeDilation = 1 / 월드 유효 배속` → `slomo 0.1` 로 전투를 늦춰도 카메라는 실시간 속도. 사용자 요구("slomo 로 낮추면 카메라까지 느려져 불편") |
| **롤 잔류 수정** | 증상: 빙의(F) → 1인칭(T) → 3인칭 복귀 또는 F 해제 뒤 자유 카메라가 **기울어진 채**. 원인: 1인칭 뷰가 머리의 실제 시선(린·애니메이션 롤 포함)을 컨트롤 회전에 동기화했고, 아무것도 롤을 빼 주지 않았다. 고침 두 곳 — ① 외부 뷰 동기화(`UpdateExternalView` 틱)에서 `SetControlRotation(FRotator(Pitch, Yaw, 0))` **yaw·pitch 만** ② 뷰가 병사 눈을 안 거치는 상태로 바뀔 때(추적/해제/T 토글) 컨트롤 회전의 롤을 0 으로 (`SoldierObserverPawn.cpp:109-119`, 틱마다가 아니라 전환 시). **사용자 확인 "잘됐음"** (P169) |

H 키는 그대로 "따라다니는 병사의 `ToggleHeadAim()`"(AI 병사엔 `bApplyToAI` 필요). 관전이 AI 병사의 H 를 **자동으로** 켜는 로직은 없다(위 0절).

---

## 5. 변경 파일 (CL 469 이후, 미제출) [A]

```
Source/SoldierLab/Pose/SoldierScanTurnComponent.{h,cpp}      add
Source/SoldierLab/Pose/SoldierGaitBridgeComponent.{h,cpp}    add
Source/SoldierLab/Pose/SoldierPoseSmootherComponent.{h,cpp}  add
Source/SoldierLab/Pose/SoldierHeadAimComponent.{h,cpp}       edit (bEnableForAI/bTurnBodyForAI 넣었다 되돌림 — 순변경은 주석뿐)
Source/SoldierLab/Observer/SoldierObserverPawn.{h,cpp}       edit (휠 비행 속도 · CustomTimeDilation · 롤)
Source/SoldierLab/Camera/SoldierFirstPersonComponent.{h,cpp} edit (09-15 분)
Content/SoldierLab/Blueprints/BP_SoldierCharacter.uasset     edit — AC_SoldierScanTurn · AC_SoldierGaitBridge · AC_SoldierPoseSmoother
                                                                    (MCP ActorTools.add_component) + 린 램프 핀 1.0 → 0.0001
```

---

## 6. 판정 기준 → 결과 [A]

| 무엇 | 어떻게 | 결과 |
|---|---|---|
| ScanTurn: 총 내린 idle 이 볼 곳으로 도는가 | `SoldierLab.Debug.ScanTurn 1` 라벨 `TURN`, 흰 화살표가 시안을 180°/s 로 따라감 | ✅ 사용자 PIE |
| GaitBridge: 안정 상태에서 걷는가 | 접촉 없음 · 급하지 않음 → 조깅이 아니라 걷기 클립 | ✅ 사용자 PIE |
| PoseSmoother: 평상시 사다리꼴 | `PoseSmooth 1` 속도가 0 → ± → 0 | ✅ "평상시는 괜찮아 보임" |
| PoseSmoother: 급할 때 스냅 없음 | urgency 높을 때 `st` 가 한 프레임에 안 뜀 | ✅ 09-18 "해결완료" (3.4 수정 후) |
| 관전 휠 비행 속도 · slomo 무관 | `slomo 0.1` 에서 WASD 속도 그대로 | ✅ |
| 관전 롤 잔류 | F → T → T / F 해제 후 수평 | ✅ "잘됐음" |

---

## 7. 이 라운드가 하지 않은 것

- 축 속도 숫자 튜닝(3.2 표 전부 [C-154]).
- GASP 크라우치 DB 전환의 "뚝" — 눈에 띌 때 [W91].
- `IsScanning()` 틈 [W89] — AI 세션과 조율 대기.
- Perforce 제출(5절).
