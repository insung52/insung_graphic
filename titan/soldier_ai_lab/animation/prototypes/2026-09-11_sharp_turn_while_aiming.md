# 조준 중 급선회 — 포즈 뒤집힘과 `Enable_AO` 재설계

2026-09-11 / **성공** / 원인은 캡슐 회전 속도가 아니라 `OffsetRootBone.maxRotationError = −1`(상한 없음)이었다. **90으로 제한**하니 뒤집힘이 사라졌고, 이어서 `Enable_AO` 문턱을 **70/70**으로 낮춰 급선회 중에는 총을 내리게 했다.

관련 항목: **[C-72] 해결** · **[C-74]** 신설 / 관련 문서: `animation/prototypes/2026-09-11_muzzle_aim_alignment.md` · `IMPLEMENTED.md` 2.4·2.5b절 · `CLAUDE.md` **P36**

> ⚠ **2026-09-12 정정 3건 → 9절.** 2절(캡슐 즉시 회전) · 4.2절(`BlendTime`) · 5절(결합 쌍)이
> 그 뒤에 바뀌었다. 후속 문서: `animation/prototypes/2026-09-12_body_yaw_rate_and_aim_antiwindup.md`

---

## 1. 증상

조준(견착) 상태에서 마우스를 빠르게 돌린다. 어느 속도를 넘어서면

- **몸이 뒤집힌다** — 상체가 반대쪽으로 확 꺾인다
- 애니메이션이 눈에 띄게 **깨진다**(제자리회전 클립이 제대로 안 나오고 포즈가 부러진다)

**판정 기준**: 마우스를 최대 속도로 좌우로 흔들어도 뒤집힘이 없고, 제자리회전 클립이 정상 재생되며,
상체가 카메라를 따라가면 성공.

---

## 2. 먼저 틀린 가설 — "다리가 느리게 도는 건 캡슐 회전 속도가 낮아서다" [A] ❌

> ⚠ **"접지 중 캡슐은 즉시 회전한다"는 2026-09-12부터 더 이상 현행이 아니다 → 9절 정정.**
> 아래 GASP **부모**의 배선 분석 자체는 지금도 맞다(부모는 여전히 `−1`을 쓴다).

`SandboxCharacter_CMC.UpdateRotation_PreCMC`를 배선 단위로 읽었다(MCP 실측):

```
Sequence
 ├ then_0 : Branch( WantsToStrafe OR WantsToAim )
 │            true  → bUseControllerDesiredRotation = true  , bOrientRotationToMovement = false
 │            false → bUseControllerDesiredRotation = false , bOrientRotationToMovement = true
 └ then_1 : Branch( CharacterMovement.IsFalling )
              true  → RotationRate = ( 0 , **200** , 0 )      ← 공중
              false → RotationRate = ( 0 , **−1**  , 0 )      ← 접지 ★
```

**`−1`은 "느리다"가 아니라 "즉시"다** [A]:

```cpp
// CharacterMovementComponent.cpp:6595-6599
float GetAxisDeltaRotation(float InAxisRotationRate, float DeltaTime)
{
    return (InAxisRotationRate >= 0.f) ? FMath::Min(InAxisRotationRate * DeltaTime, 360.f) : 360.f;
}
```

음수면 한 프레임에 **360°**가 허용되므로 `FMath::FixedTurn`이 목표각으로 **곧장 스냅**한다.
즉 **접지 상태에서 캡슐 yaw는 컨트롤러를 한 프레임 만에 따라잡는다.**

> **그러므로 눈에 보이는 회전 지연은 캡슐이 아니라 `OffsetRootBone`(메시 오프셋)이다.**
> 이 정정이 다음 절의 진짜 원인으로 곧장 이어졌다 — **회전이 느린 게 아니라 벌어짐에 상한이 없었다.**

---

## 3. ★ 진짜 원인 — `OffsetRootBone.maxRotationError = −1` [A]

`SoldierCharacter_ABP:AnimGraph.AnimGraphNode_OffsetRootBone_0` 실측:

| 프로퍼티 | GASP 원본 | 우리(수정 전) | 우리(수정 후) |
|---|---|---|---|
| `translationMode` | Interpolate | Interpolate | Interpolate |
| `rotationMode` | Accumulate | Accumulate | Accumulate |
| `translationHalflife` | 0.2 | 0.2 | 0.2 |
| `rotationHalfLife` | 0.1 | 0.1 | 0.1 |
| `maxTranslationError` | **30** | 30 | 30 |
| **`maxRotationError`** | **−1** | **−1** | **90** ★ |

**이동 오차에는 상한 30이 있는데 회전 오차에만 상한이 없었다.** GASP 출하 기본값이고
우리가 손댄 값이 아니다 [A] — `SandboxCharacter_CMC_ABP`도 `−1`이다(직접 비교).

### 3.1 왜 그게 포즈를 부러뜨리는가

```
캡슐 yaw   ── 즉시 컨트롤러를 따라간다 (2절)
메시 yaw   ── OffsetRootBone 이 halfLife 0.1 로 뒤따라간다
             ↓
        둘의 차이 = 루트와 조준 방향의 벌어짐
             ↓
   Get_AOValue = Delta( AimingRotation , RootRotation ) (+보정)
             ↓
        그 벌어짐이 **그대로 AimOffset 입력**이 된다
```

`maxRotationError = −1`이면 이 벌어짐에 **상한이 없다.** 빠르게 돌수록 커지고, 저작된
AimOffset 범위(`AO_Rifle_ADS` Yaw −180..180이지만 표본은 5장)를 넘어서면 **포즈가 뒤집힌다.**

> **구조적으로 말하면**: `maxRotationError`는 "메시가 캡슐에서 얼마나 벌어져도 되는가"를 정하는
> 값인 동시에 **AimOffset 입력의 상한**이다. 둘이 같은 값이다.

### 3.2 조치와 결과 [A]

`maxRotationError` **−1 → 90**. PIE 확인:

- **뒤집힘이 전혀 없다**
- 제자리회전(turn-in-place) 클립이 **제대로 재생된다**
- 상체가 빠른 조준을 **따라간다**

**[C-72] 해결.**

> ⚠ 이 프로퍼티는 **핀으로 노출돼 있지 않다** — `node.maxRotationError`가 진실이다.
> 반영에는 `compile_blueprint`가 필요하다 (**P23**).

---

## 4. `Enable_AO` — DSL이 또 항을 빠뜨렸다 [A]

`maxRotationError`로 뒤집힘은 없어졌지만, 급선회 중에 **총이 계속 들려 있어** 어색했다.
총을 내리려면 AimOffset을 끄는 판정(`Enable_AO`)을 손대야 한다. 그래서 이 함수를 다시 읽었다.

`read_graph_dsl`은 **항을 하나 빠뜨렸다** — `CLAUDE.md` **6.1f**와 같은 종류의 실패다.
배선을 직접 추적(`find_nodes` + `get_node_infos`)한 **실제 구조는 3항 AND**다:

```
Enable_AO =      | Get_AOValue.X |  ≤  문턱(MovementState)
            AND  RotationMode == NewEnumerator1        (= Strafe / 견착)
            AND  GetSlotLocalWeight("DefaultSlot") < 0.5
```

| 항 | 뜻 | 빠뜨리면 |
|---|---|---|
| 각도 한계 | 조준각이 저작 범위 안인가 | 급선회에서 포즈가 부러진다 |
| `RotationMode == Strafe` | **의도**가 조준인가 | 총내림 상태에도 AO가 걸린다 |
| `GetSlotLocalWeight("DefaultSlot") < 0.5` | **몽타주가 재생 중이 아닌가** | ← **DSL이 빠뜨린 항.** 전신 몽타주(트래버설 등) 중에 AO가 얹혀 포즈가 섞인다 |

비교 대상은 `Get_AOValue`의 **X 성분(yaw)** 뿐이다 — Y(pitch) 출력은 연결돼 있지 않다 [A].

### 4.1 ⚠ 정정 — Idle/Moving 문턱을 **거꾸로** 적었었다

세션 앞부분과 `IMPLEMENTED.md` 2.5b절에 **"정지 180 / 이동 115"** 로 적혀 있었다. **틀렸다.**

문턱은 `MovementState`에 대한 `Select` 노드가 고른다. GASP 원본 실측:

```
Select( Index = MovementState )
    NewEnumerator4  →  115
    NewEnumerator0  →  180
```

어느 열거자가 무엇인지는 `Update_States`가 정한다 [A]:

```
Branch( MovementAnalysis|IsMoving )
    true   →  MovementState = NewEnumerator0     ← Moving
    false  →  MovementState = NewEnumerator4     ← Idle
```

(`E_MovementState` 에셋의 표시 이름도 `Moving` / `Idle` 두 개뿐이다.)

**따라서 GASP 원본은 `Idle = 115` / `Moving = 180`이다.** 이전 서술은 정반대였다
→ `IMPLEMENTED.md` 2.5b절에 정정을 달았다 (`CLAUDE.md` 3.3절).

### 4.2 조치 — 두 문턱을 **70 / 70** 으로 [A]

```
Select( Index = MovementState )
    NewEnumerator4 (Idle)    :  115  →  **70**
    NewEnumerator0 (Moving)  :  180  →  **70**
```

의도: **급선회로 조준각이 70°를 넘으면 AO를 끈다 = 총을 내린다.** 몸이 따라잡아 각이 줄면
다시 올린다. 정지/이동을 가를 이유가 없다고 보아 같은 값으로 뒀다.

**문턱이 떨릴 걱정은 따로 처리하지 않았다** — `BlendListByBool_0`이 이미 느리게 블렌드한다:

```
AnimGraphNode_BlendListByBool_0   (핀 값이 진실 — P25)    ★ 값은 그 뒤 바뀌었다 → 9.2절
    BlendPose_0  ← BlendSpacePlayer_1 (AO)            BlendTime_0 = 0.75 s
    BlendPose_1  ← LayeredBoneBlend_0 (총내림)        BlendTime_1 = 1.50 s
    transitionType = Inertialization
    customBlendCurve = AO_Blend_Curve · blendProfile = FastHead_Weight
```

> ⚠⚠ **`BlendListByBool`은 불리언 방향이 뒤집혀 있다** — `true`가 **0번** 입력이다 [A]:
> ```cpp
> // AnimNode_BlendListByBool.cpp
> // Note: Intentionally flipped boolean sense (the true input is #0, and the false input is #1)
> return GetActiveValue() ? 0 : 1;
> ```
> 그러므로 **AO를 켤 때 0.75초 · 끌 때 1.50초**다. 끄는 쪽이 두 배 느려 **문턱 근처의 채터링이
> 저절로 눌린다.** 추가 히스테리시스 로직이 필요 없었다.
>
> ⚠ 그리고 이 값은 **노드 프로퍼티가 아니라 핀에 있다.** `node.blendTime`은 `[0.1, 0.1]`로
> GASP 원본 그대로이고, 실제로 쓰이는 건 핀의 `0.75 / 1.5`다 — **P25**의 교과서적 사례다.
> 노드 프로퍼티만 읽고 "안 고쳐졌다"고 판단할 뻔했다.

**결과 [A]**: PIE에서 **어느 회전 속도를 넘으면 총이 내려가고, 몸이 따라잡으면 깔끔하게 되올라온다.**

---

## 5. ★★ 두 값은 **묶여 있다** — 같이 튜닝해야 한다 [A]

```
maxRotationError = 90        ← 천장. AimOffset 입력이 여기까지만 커진다
Enable_AO 문턱   = 70        ← 그 안에서 "총을 내릴 선"
```

**`Enable_AO` 문턱이 천장 이상이면 영원히 발동하지 않는다.**

- `maxRotationError = 90`은 `(AimingRotation − RootRotation)` 항을 90°로 막는다
- 여기에 총구 보정(`AimCorrection`)이 ±25°까지 더해지므로 `|Get_AOValue.X|`의 실질 상한은
  **약 115°** 다 [B] — 즉 **문턱 115 이상은 절대 발동하지 않고, 90~115 구간은 보정값이
  밀어 줄 때만 발동한다**
- 그래서 **GASP 원본의 Idle 115 / Moving 180은 `maxRotationError = 90` 아래에서는 사실상
  "AO를 끄지 않는다"와 같다** [B]

> **규칙: `maxRotationError`는 천장, `Enable_AO` 문턱은 그 안의 결정선이다.
> 한쪽만 바꾸면 다른 쪽의 의미가 조용히 달라진다. 항상 쌍으로 적고 쌍으로 바꾼다.**

두 값(90 / 70) 모두 **PIE에서 눈으로 정한 값이지 계측한 값이 아니다** → **[C-74]**.

---

## 6. 판정

**성공.** 1절의 세 기준을 전부 통과했다 [A].

| 기준 | 결과 |
|---|---|
| 최대 속도 회전에서 뒤집힘 없음 | ✅ |
| 제자리회전 클립 정상 재생 | ✅ |
| 상체가 빠른 조준을 추종 | ✅ |
| (추가) 급선회 시 총을 내렸다가 되올림 | ✅ |

---

## 7. 이것이 바꾸는 것

| 문서 | 절 | 어떻게 |
|---|---|---|
| `IMPLEMENTED.md` | 2.4 · 2.5b · 2.6 | `maxRotationError 90` · `Enable_AO` 3항 구조와 70/70 · 결합 경고 · Idle/Moving 정정 |
| `OPEN_ITEMS.md` | C절 | **C-72 해결** / **C-74** 신설 |
| `CLAUDE.md` | 5절 | **P36**(증상 패치 대신 근본 원인) |
| 형제 문서 | — | `animation/prototypes/2026-09-11_muzzle_aim_alignment.md` 9~12절 |

---

## 8. 남은 것

| ID | 항목 |
|---|---|
| **C-74** | `maxRotationError 90` ↔ `Enable_AO 70`이 **감으로 잡힌 쌍**이다. "저작된 AO 범위의 실제 파탄 각도"를 재서 천장을 정하고, 문턱은 그 안에서 정해야 한다 |
| [C-73] | `AO_Rifle_ADS`의 pitch 표본 3장 — 문턱을 낮춘 지금은 덜 드러나지만 남아 있다 |
| — | `AO_Rifle_Crouch`는 여전히 미배선이다. 웅크린 채 급선회하면 서기용 AO가 걸린다(`IMPLEMENTED.md` 6절) |

---

## 9. 정정 (2026-09-12)

`CLAUDE.md` 3.3절에 따라 본문은 그대로 두고 여기에 정정을 붙인다.

### 9.1 ⚠⚠ 2절 — 접지 중 캡슐이 "즉시" 회전한다는 것은 **더 이상 현행이 아니다** [A]

부모(`SandboxCharacter_CMC`)가 접지 중 `RotationRate = (0, −1, 0)`을 쓰는 것은 지금도 맞다.
그러나 **2026-09-12부터 우리 캐릭터가 그 값을 매 틱 덮어쓴다**:

```
BP_SoldierCharacter . UpdateBodyYawRate()      Event Tick 끝
    ω = Lerp( YawRate_Up 90 , YawRate_Down 720 , WeaponLowered )      °/s
    CharacterMovement.SetRotationRate( MakeRotator(0, 0, ω) )

BeginPlay (Parent: BeginPlay 직후)   ← 이게 없으면 위 쓰기가 조용히 무시된다
    self.AddTickPrerequisiteComponent( GetComponentByClass(AC_PreCMCTick) )
    CharacterMovement.AddTickPrerequisiteActor( self )
```

**그래서 4.2절의 "급선회에는 총을 내린다"에 비로소 이득이 생겼다** — 총을 내린 동안에만
몸이 빨리 돈다(90 → 720 °/s). 이 문서를 쓸 당시에는 **총을 내려도 회전 속도가 같았고**,
그 점을 이 문서는 알아차리지 못했다.

> ⚠ 그리고 **`BodyErr == 0.000`(정확히 0)이 `RotationRate.Yaw = −1`의 서명**이다 —
> 2절의 관찰을 숫자로 재확인하는 방법이기도 하다 (`CLAUDE.md` **P39**).

### 9.2 ⚠ 4.2절 — `BlendListByBool_0`의 `BlendTime`이 다르다 [A]

```
이 문서 작성 시점   BlendTime_0 = 0.75   BlendTime_1 = 1.50
2026-09-12 실측     BlendTime_0 = 0.375  BlendTime_1 = 0.25
```

그 사이에 바뀐 값이고 문서가 따라가지 못했다. **AO를 켜는 쪽이 0.375초**라는 사실이
조준 보정 게인의 회복 시간(0.5초)을 정하는 기준이 됐다 →
`animation/prototypes/2026-09-12_body_yaw_rate_and_aim_antiwindup.md` 11.2절.

노드의 나머지 설정(`transitionType = Inertialization` · `customBlendCurve = AO_Blend_Curve` ·
`blendProfile = FastHead_Weight`)은 그대로다. ⚠ **`AO_Blend_Curve`는 아직 아무도 안 봤다** → **[R6]**.

### 9.3 5절의 결합 쌍이 **3개조가 됐다** [A]

```
maxRotationError      90      천장       AimOffset 입력의 상한
Enable_AO 문턱        70      결정선     총을 내릴 각도
WeaponLowerAngleFull  65      해제선     몸통 각속도가 완전히 풀리는 각도   ← 2026-09-12 추가
```

**65 < 70 < 90 이라는 순서에 의미가 있다** — 각속도가 먼저 풀리고, 그 다음 포즈가 바뀌고,
천장은 그 바깥에 있다. → **[C-74]**
