# 상황별 이동 정책과 재생배율 밴드 — 걸음걸이 상한 + 속도 배율 0.75~1.25

2026-09-29 / 완료·실측 검증 / 상황별 이동 정책(걸음걸이 상한 + 속도 배율 0.75~1.25)을 GASP 재생배율 밴드에 맞춰 구현

같은 축 전편: `animation/2026-09-18_ai_pose_layer_scanturn_gait_smoother.md`(`USoldierGaitBridgeComponent` — **이 문서가 그 브리지의 상류를 고친다**) · `ai/2026-09-18_patrol_scan_and_move_robustness.md` 12~17절(걸음 `GetDesiredGait()` 의 **발행 측**) · `animation/2026-09-02_gasp_abp_analysis.md`(ABP 구조 — 15·16절 워핑).
원칙: **P193**(속도의 기준은 클립에 구워진 authored 속도다) · P10(계측 먼저) · P159(값을 임의로 정하지 않는다).
값: **[C-174]**. 작업: **[W120]~[W122]**.

> **한 줄 요약** — 병사 속도는 이제 표 한 장(`DT_SoldierMovement`, 행 16개 × 열 2개)으로 조절한다. 열은 **걸음걸이 상한**과 **속도 배율**뿐이고, 배율은 **0.75~1.25 로 강제 클램프**된다. 그 밴드는 임의값이 아니라 **ABP `Get_DynamicPlayRate` 가 커브 없는 클립에 쓰는 대체값**이고, 벗어나면 발이 미끄러진다.

---

## 0. 요구와 첫 실패 (2026-09-28)

**요구 [A · 사용자]**: "시나리오에서 병사가 너무 빠르다." 상황별로 속도를 조절하되, **애니메이션 시스템을 모르는 사람(기획·디자이너)도 표로 커스텀 가능하게.**

**1차 구현(2026-09-28, 폐기됨)**: 상황 16개 × **(속도 · 가속 · 회전) 배율 3열** DataTable + 컴포넌트. 두 단계로 실패했다.

### ⓐ 전혀 안 먹었다 — `AC_PreCMCTick` 이 매 프레임 다시 쓴다 [A · 실측]

CMC 의 `MaxWalkSpeed` 에 배율을 곱했다. 그런데 GASP 는 **`AC_PreCMCTick` 컴포넌트**가 **CMC 직전에** 캐릭터의 `UpdateMovement_PreCMC` 를 돌려 `MaxWalkSpeed` · `MaxAcceleration` · `RotationRate` 를 **매 프레임 다시 쓴다.** 우리 컴포넌트 틱은 액터 틱 뒤였지만 그보다 **앞**이라, 쓴 값이 그대로 지워졌다.

> 이 층이 CMC 프로퍼티를 목표로 삼을 수 없다는 것이 여기서 확정됐다. 살아남는 길은 **CMC 가 그 값을 계산할 때 읽는 입력**(gait 속도 벡터)을 바꾸는 것뿐이다.

### ⓑ 입력에 곱하니 속도는 변했지만 발이 미끄러졌다 [A · 관측]

gait 속도 벡터에 곱하도록 바꾸자 속도는 정확히 변했다. 대신 **발이 미끄러졌다.** 당시 배율은 **0.45~0.7** 이었고, 안전범위를 **0.45~1.2** 로 적어 뒀다 — **측정 없이 임의로 정한 값이었고, 이게 근본 실수다.** 실제 밴드는 0.75~1.25(2.3절)이므로 그 배율은 전부 밴드 밖이었다.

**사용자 지시로 1차 구현은 폐기했다**(컴포넌트 제거 · 원복) — 값을 고치는 대신 **구조를 먼저 읽기로** 했다. 아래 2절이 그 결과다.

---

## 1. 실제 체인 — AI 에서 애니메이션까지 [A · 코드 확인]

### 1.1 AI → 이동

```
USoldierEngagementComponent::GetDesiredGait()          ← AI 가 원하는 걸음걸이 (SoldierEngagement.h:281)
  → USoldierGaitBridgeComponent                        ← CharacterInputState.WantsToWalk / WantsToSprint 에 기록
  → SandboxCharacter_CMC::GetDesiredGait  →  Gait
  → AC_PreCMCTick  →  UpdateMovement_PreCMC
  → CalculateMaxSpeed( Gait + WalkSpeeds/RunSpeeds/SprintSpeeds
                       + Curve_StrafeSpeedMap + 진행 방향 )
  → MaxWalkSpeed
```

- 브리지는 `bDriveSprint = false` 다(`Pose/SoldierGaitBridgeComponent.h:39`) → **AI 는 Sprint 를 요청하지 않는다.** 이 사실이 뒤에서 두 번 쓰인다(MM DB 에 Sprint 세트가 없어도 문제가 없는 이유 · 상한이 Sprint 일 때 `bWantsToSprint` 를 끄는 이유).
- 속도는 **걸음걸이(이산) × 속도 벡터(캐릭터 변수) × 방향 커브**로 나온다. 즉 **속도 벡터가 이 체인의 유일한 연속 손잡이**다.

### 1.2 이동 → 애니메이션

`Get_PropertiesForAnimation` 이 ABP 로 넘기는 것은 **Gait · Velocity · CurrentAcceleration · MaxAcceleration · BrakingDeceleration · 바닥 · 회전**이다. **속도 *설정값*(WalkSpeeds 등)은 넘기지 않는다** [A].

> 그래서 **ABP 는 실제 속도만 본다.** 우리가 설정값을 바꿔도 ABP 는 "왜" 를 모르고, 바뀐 `Speed2D` 만 본다 — 아래 재생배율 식의 분자가 그것이다.

---

## 2. 재생배율 알고리즘 — 이 문서의 핵심 [A · ABP 그래프 전수 확인]

`Get_DynamicPlayRate` 를 그래프에서 처음부터 끝까지 읽은 결과:

```
AnimSequence = 현재 BlendStack 클립 ,  AnimTime = 그 시간

MinDynamicPlayRate 커브 없으면  →  0.75
MaxDynamicPlayRate 커브 없으면  →  1.25
MoveData_Speed     커브 없으면  →  재생배율 1.0 즉시 반환 (적응 없음)
Enable_Warping     커브 없으면  →  1.0 즉시 반환

배율 = Clamp( Speed2D / MoveData_Speed , Min , Max )
최종 = Lerp( 1.0 , 배율 , Enable_Warping )
```

### 2.1 우리 클립에는 Min/Max 커브가 없다 → 실효 밴드 ±25%

`MinDynamicPlayRate`/`MaxDynamicPlayRate` 커브가 구워져 있지 않으므로 **대체값 0.75 / 1.25 가 그대로 쓰인다** [A]. 즉 **실제 속도가 클립이 만들어진 속도의 ±25% 를 벗어나면 애니메이션이 더는 따라오지 못하고, 그 차이가 그대로 발 미끄러짐이 된다.**

### 2.2 클립 실측 — authored 속도는 캐릭터 설정값과 같다 [A · 에디터 Curves 패널, 사용자 판독]

| 클립 | `MoveData_Speed` | 캐릭터 설정값 |
|---|---|---|
| `ALLY_MM_Rifle_Walk_Fwd` | **291.31** | `WalkSpeeds` = 291.31 |
| `ALLY_MM_Rifle_Jog_Fwd` | **582.62** | `RunSpeeds` = 582.62 |

**정확히 같다** — 클립이 이 속도에 맞춰 리타이밍돼 있다. 즉 **기본 상태(배율 1.0)에서는 재생배율이 정확히 1.0** 이고, 우리가 배율을 곱하는 만큼이 그대로 재생배율의 이탈량이 된다.

### 2.3 따라서 미끄러짐 없는 구간

| 걸음 | authored | 하한 ×0.75 | 상한 ×1.25 |
|---|---|---|---|
| Walk | 291.31 | **218** | **364** |
| Jog | 582.62 | **437** | **728** |

그 사이 구간(**364 ~ 437**)은 Walk·Jog **두 세트가 모두 MM DB 에 있어** 모션매칭이 섞어 메운다 — 구멍이 아니다.

### 2.4 MM DB 구성 [A · 에셋 확인]

Rifle 기준:

| 묶음 | 내용 |
|---|---|
| Stand Walk | Loops · Starts · Stops · Pivots + TurnInPlace |
| Stand Jog | Loops · Starts · Stops · Pivots + TurnInPlace |
| Crouch Walk | 위와 같은 구성 |
| Idles | Idles · Idles_LowReady |

방향은 **Fwd / Bwd / Left / Right**. 클립 **146개, 전부 루트모션 ON**. ⚠ **Sprint 세트는 없다** — 1.1 의 `bDriveSprint = false` 때문에 AI 경로에서는 문제되지 않는다(플레이어가 Shift 로 달릴 때는 Jog 클립이 배속으로 늘어나는 기존 동작 그대로).

---

## 3. 최종 구현

### 3.1 열은 2개다

신규 `USoldierMovementProfileComponent` + 행 구조체 `FSoldierMovementProfileRow`
(`Source/SoldierLab/AI/SoldierMovementProfile.{h,cpp}`).

| 열 | 뜻 | 왜 안전한가 |
|---|---|---|
| **`MaxGait`** (Walk/Jog/Sprint) | AI 의 `GetDesiredGait()` 결과에 씌우는 **상한** | **클립이 그 걸음 속도로 authored 되어 있다** — 걸음을 한 칸 내리는 것은 원리적으로 미끄러짐이 없다(공짜) |
| **`SpeedScale`** | gait 속도 벡터에 곱하는 배율 | **0.75~1.25 로 클램프**. 근거는 2.1~2.3 |

배율은 세 겹으로 잘린다 — **① `UPROPERTY meta = (ClampMin="0.75", ClampMax="1.25")`** 로 에디터 입력 자체를 제한, **② `ResolveProfile` 에서 재클램프**(`SoldierMovementProfile.cpp:329-333`), **③ 전역 cvar 를 곱한 뒤 또 재클램프**(`:354-357`). **표를 만지는 사람이 밴드를 몰라도 되게** 하는 것이 목적이다.

### 3.2 ⛔ 가속·회전 열은 뺐다 (1차 구현의 오보 정정)

1차 구현에는 가속·회전 배율 열이 있었고 **"적용된다"고 보고했는데, 사실이 아니었다** — 0절 ⓐ 의 `AC_PreCMCTick` 이 `MaxAcceleration`/`RotationRate` 를 매 프레임 다시 쓰므로 **죽은 값**이었다. 헷갈릴 여지를 남기지 않기 위해 열을 **삭제**했다. 상황별 가속·회전이 필요해지면 **틱 순서를 먼저 해결해야 한다** → [W121].

### 3.3 상한을 적용하는 위치

`USoldierEngagementComponent` 의 gait 결정 **끝**(`AI/SoldierEngagement.cpp:1802-1812`):

```cpp
if (MovementProfile != nullptr)
{
    const ESoldierGait Ceiling = MovementProfile->GetMaxGait();
    if (DesiredGait > Ceiling) { DesiredGait = Ceiling; }
    if (Ceiling < ESoldierGait::Sprint) { bWantsToSprint = false; }
}
```

- **어떤 걸음을 원하는가는 교전 컴포넌트의 판단, 상황이 누구에게든 허용하는 최대는 정책 층의 판단**이다. 그래서 **덮어쓰지 않고 상한으로만** 작동한다 — 걸으라고 지시받은 병사가 상한 Jog 때문에 뛰게 되지는 않는다.
- **프로파일 컴포넌트가 없으면 무제한** = 예전 동작. 표도 컴포넌트도 없어도 시스템은 돈다.

### 3.4 상황 선택 — 2단 규칙

`ChooseSituation()`(`SoldierMovementProfile.cpp:196-305`):

**① 명령이 우선** — `BreakContact` → `Rush` 가 걸려 있으면 **그 행이 이긴다.** 이유: "가장 느린 행" 규칙만 두면 **도주하며 재장전하는 병사가 걸어서 도망친다.** 이탈이 돌입보다 강하다(둘이 동시에 걸리는 경우는 없지만 순서를 정해 뒀다).

**② 아니면 최종 속도가 가장 낮은 행** — 비교 단위는 **`걸음 기준값 × 배율`(cm/s)** 이다(`EffectiveSpeed`, `:186-194`). 배율만 비교하면 걸음이 다른 행끼리 비교가 안 된다(Walk 1.0 = 291 이 Jog 0.85 = 495 보다 느리다). 동점이면 **열거자 순서**가 앞선 쪽 — 같은 조합에서 항상 같은 답이 나와야 표를 신뢰할 수 있다.

### 3.5 ★ `Aiming` 과 `Firing` 은 다른 상황이다 (사용자 지적으로 수정)

첫 표는 `WantsToAim()` 하나로 "교전 중" 을 잡고 Walk 상한을 줬다. **틀렸다** — `WantsToAim()` 은 **엄폐지 사이를 달릴 때도 켜진다.** 그러면 "교전이 시작되면 아무도 뛰지 못하는" 병사가 된다.

| 상황 | 판정 | 상한·배율 | 뜻 |
|---|---|---|---|
| `Aiming` | `WantsToAim()` | **Jog 0.90** | 총을 견착한 채 이동 — **뛴다** |
| `Firing` | `WantsToFire()` | **Walk 0.75** | 실제로 쏘는 구간 — 멈춰서 쏜다 |

### 3.6 상황 16개와 판정 근거

| 상황 | 판정 근거 |
|---|---|
| `Default` | 아무 것에도 해당 없음(분대 명령 없는 병사 포함) |
| `Cautious` / `Normal` / `Rush` | 분대 명령 `FSoldierAssignment::Speed` |
| `BreakContact` | `FSoldierAssignment::bBreakContact` |
| `MovingToCover` / `EdgeAdvance` | `USoldierCoverComponent::IsMovingToCover()` / `IsAdvancing()` |
| `Aiming` / `Firing` / `Reloading` / `Peeking` / `Scanning` / `Crouched` | `USoldierEngagementComponent::WantsToAim()` / `WantsToFire()` / `IsReloading() \|\| WantsToReload()` / `IsPeeking()` / `IsScanning()` / `GetDesiredStance() ≥ CrouchStanceThreshold` |
| `Suppressed` | `USoldierSuppressionComponent::GetSuppression() ≥ SuppressedThreshold` |
| `HitReacting` / `Wounded` | `USoldierHealthComponent::IsHitReacting()` / `GetHealthFraction() ≤ WoundedHealthFraction` |

문턱 셋은 컴포넌트 `UPROPERTY` 다 — `WoundedHealthFraction 0.4` · `SuppressedThreshold 0.3` · `CrouchStanceThreshold 0.5`.

### 3.7 현재 DT 값과 결과 속도

`Content/SoldierLab/Data/DT_SoldierMovement`. 같은 값이 **C++ 기본값**(`GetDefaultProfile`, `:76-103`)에도 들어 있어 **표가 없어도 같게 동작한다**(행이 없으면 한 번 경고하고 기본값).

| 상황 | 상한 | 배율 | 결과 속도 |
|---|---|---|---|
| `Default` · `Rush` · `BreakContact` · `MovingToCover` | Jog | 1.00 | **583** |
| `Aiming` | Jog | 0.90 | **524** |
| `Normal` · `Suppressed` | Jog | 0.85 | **495** |
| `Wounded` | Jog | 0.80 | **466** |
| `Reloading` · `Scanning` | Walk | 0.85 | **248** |
| `Cautious` · `Crouched` | Walk | 0.80 | **233** |
| `EdgeAdvance` · `Firing` · `Peeking` · `HitReacting` | Walk | 0.75 | **218** |

전부 2.3절 밴드 안이다(Walk 218~364 · Jog 437~728).

### 3.8 콘솔

| cvar | 뜻 |
|---|---|
| `SoldierLab.Move.Enabled` | `0` = 정책 층 끄기(A/B). **원본 속도로 되돌리고 걸음 상한도 푼다** |
| `SoldierLab.Move.SpeedScale` | 표와 별개의 전역 배율. 표 배율과 합친 뒤 **밴드로 다시 잘린다** — 콘솔로도 밴드를 못 넘는다 |
| `SoldierLab.Debug.Move 1` | 상황이 바뀔 때마다 로그 |

```
[Move] Enemy_A2 Default -> Firing (max Walk, scale 0.75) walk 218 run 437
```

배율은 매 틱 **BeginPlay 에 기억한 원본 × 배율**로 다시 계산한다(`WriteGaitSpeeds`, `:167-184`) — 프레임마다 누적되지 않는다. 속도 벡터는 캐릭터 Blueprint 변수라 **리플렉션으로 이름(`WalkSpeeds`/`RunSpeeds`/`SprintSpeeds`/`CrouchSpeeds`)으로 찾는다** — 이 컴포넌트가 GASP 캐릭터 클래스를 직접 알지 않아도 되게. 못 찾으면 경고 한 줄 남기고 **걸음걸이 상한만** 동작한다.

---

## 4. 실측 검증 [A · PIE, 사용자 확인]

| 단계 | 내용 | 결과 |
|---|---|---|
| ① 극단값 | **전 행을 `Walk / 0.75`(= 218, 원래 Jog 583 대비 2.7배 느림)** 로 바꿔 PIE | **확실히 느려지고 발 미끄러짐 없음** ✅ |
| ② 계획값 | 3.7절 값으로 원복 | **정상** ✅ |

①을 먼저 한 이유: 밴드의 **하단 끝**을 밟아 보는 것이 "느려지는가" 와 "미끄러지지 않는가" 를 한 번에 확인하는 가장 강한 조건이다. 여기서 안 미끄러지면 그 위의 모든 값도 안 미끄러진다.

---

## 5. 남은 것 · 주의

### 5.1 218 보다 더 느리게 필요하면 — **표만으로는 불가능하다** → [W120]

218 cm/s 는 `Walk × 0.75` = **밴드의 하단 끝**이다. 더 느리게 하려면 둘 중 하나다:

1. 클립에 **`MinDynamicPlayRate` / `MaxDynamicPlayRate` 커브를 구워** 밴드를 넓힌다(2절의 대체값 대신 그 커브가 쓰인다).
2. **느린 걷기 클립을 MM DB 에 추가**한다(authored 속도가 낮은 세트를 늘리는 쪽).

⚠ **표의 배율을 0.75 아래로 적는 것은 답이 아니다** — 세 겹 클램프가 잘라내고, 클램프를 풀면 0-ⓑ 의 미끄러짐으로 돌아간다.

### 5.2 가속·회전을 상황별로 쓰려면 틱 순서부터 → [W121]

3.2절. `AC_PreCMCTick` 뒤에 도는 자리를 만들어야 한다.

### 5.3 방향별·Crouch 클립의 authored 속도 미측정 → [C-174]

`MoveData_Speed` 를 실측한 것은 **`Walk_Fwd` / `Jog_Fwd` 둘뿐**이다. 현재 속도 벡터는 3축이 동일(291.31 / 582.62)이므로, **방향별 클립의 authored 속도가 다르면 옆걸음·후진에서 밴드를 벗어날 수 있다.** Crouch 세트도 같다.

**단, 현재 버전에서 미끄러짐은 관측되지 않았다** [A] — 즉 지금 값으로는 문제가 없고, 이것은 **밴드를 더 밀 때(5.1) 먼저 확인해야 하는 항목**이다.

### 5.4 `DT_SoldierMovement` 는 **P4 add 미완** → [W122]

신규 애셋이라 체크인되지 않았다. 다른 PC / 패키지에서는 표가 없어 **C++ 기본값으로 동작한다**(같은 값이라 거동은 같지만, 표를 고쳐도 반영되지 않는다).

---

## 6. 원칙

**P193 — 속도를 바꿀 때 기준은 클립에 구워진 authored 속도이고, 허용 범위는 재생배율 클램프가 정한다. 임의의 안전범위를 정하지 말 것.**

모션매칭 로코모션에서 "속도를 N% 줄인다" 는 자유롭게 정할 수 있는 값이 아니다. 각 클립에는 **그 클립이 만들어진 속도**(`MoveData_Speed` 커브)가 들어 있고, ABP 는 `Clamp(Speed2D / MoveData_Speed, Min, Max)` 로만 적응한다. **Min/Max 커브가 없으면 대체값 0.75 / 1.25 다** — 그 밖은 애니메이션이 따라오지 못하고 곧 발 미끄러짐이다. 그러므로:

- **먼저 클립의 authored 속도를 읽는다.** 캐릭터 설정값과 같을 것이라고 가정하지 말고 커브를 본다(우리 경우는 같았지만, 그게 확인의 이유다).
- **범위는 밴드에서 나온다.** "0.45~1.2 면 괜찮겠지" 같은 숫자를 적지 말 것 — 1차 구현이 정확히 이것으로 실패했다.
- **더 큰 변화가 필요하면 값이 아니라 데이터를 바꾼다** — 커브를 굽거나 클립을 늘린다.
- **이산 축(걸음걸이)이 있으면 그것을 먼저 쓴다.** 클립이 그 속도로 authored 되어 있으므로 걸음을 내리는 것은 공짜고, 배율은 그 안의 미세 조정일 뿐이다.

곁가지 원칙 하나 — **엔진/템플릿이 매 프레임 다시 쓰는 프로퍼티를 목표로 삼지 말 것.** GASP 에서 `MaxWalkSpeed`·`MaxAcceleration`·`RotationRate` 는 `AC_PreCMCTick` 의 소유다. 그 값을 바꾸려면 **그것이 읽는 입력**을 바꾸거나 **틱 순서를 바꿔야** 하고, 둘 다 안 하고 쓰면 조용히 지워진다(가속·회전 열이 "적용된다"고 잘못 보고된 이유).

---

## 7. 정정 기록 — 1차 조사에서 내가 단정했던 것 중 틀린 것

CLAUDE.md 3.2·3.3절대로, 원래 서술을 지우지 않고 남긴다.

| # | 1차 조사의 서술 | 판정 |
|---|---|---|
| ① | ~~"3축(전/횡/후) 동일 속도 벡터가 발 미끄러짐의 원인이다"~~ | **철회 [A]** — 클립 authored 속도가 **291.31 단일값**이므로 3축 동일 벡터는 오히려 **정합적**이다. 미끄러짐의 원인은 벡터의 형태가 아니라 **배율이 밴드 밖이었던 것** |
| ② | ~~"돌입(`Rush`)·도주(`BreakContact`)에서 미끄러진다"~~ | **철회 [A]** — **관측되지 않은 추측**이었다. 두 행은 배율 1.00 이라 재생배율이 정확히 1.0 이고, 원리적으로 미끄러질 자리가 아니다 |
| ③ | ~~"안전범위는 0.45~1.2 다"~~ | **교체 [A]** — **근거 없는 임의값**이었다. 실측·엔진 확인 결과 **0.75~1.25**(2절). 이 한 줄이 0-ⓑ 실패의 직접 원인이다 |
| ④ | ~~"가속·회전 배율도 적용된다"~~ | **철회 [A]** — `AC_PreCMCTick` 이 덮어써서 **죽은 값**이었다(3.2절). 열 자체를 삭제 |

> 넷 다 **확인하지 않은 것을 세밀하게 쓴** 경우다(CLAUDE.md 3.1절이 경고하는 바로 그 실패 형태 — 이 프로젝트에서 다섯 번째다). ③이 특히 비싸다: 숫자를 적어 두면 다음 사람이 그것을 근거로 쓴다.

---

## 8. 바뀐 파일

| 파일 | 내용 |
|---|---|
| `Source/SoldierLab/AI/SoldierMovementProfile.h` / `.cpp` | **신규** — `ESoldierMovementSituation`(16 + `Count`) · `FSoldierMovementProfileRow`(`MaxGait` · `SpeedScale`) · `USoldierMovementProfileComponent`(상황 판정 2단 · 밴드 클램프 3겹 · 리플렉션으로 gait 속도 벡터 · cvar 3개). **설계 근거가 헤더 상단 주석에 있다** |
| `Source/SoldierLab/AI/SoldierEngagement.h` / `.cpp` | gait 결정 끝에 상한(`.cpp:1802-1812`) · `MovementProfile` 캐시(`:281`, `.h:1103`) · **`IsReloading()` 게터 추가**(`.h:307`) |
| `Content/SoldierLab/Data/DT_SoldierMovement` | **신규 애셋** — 행 16개. ⚠ **P4 add 미완** [W122] |
| `Content/SoldierLab/Blueprints/BP_SoldierCharacter` | 컴포넌트 `AC_SoldierMovementProfile` 부착 + `ProfileTable` 에 위 표 연결 |

**손대지 않은 것**: ABP · MM DB · 클립 · 캐릭터의 속도 벡터 기본값(런타임에만 곱한다) · `USoldierGaitBridgeComponent`(상한은 그 상류에서 걸린다).
