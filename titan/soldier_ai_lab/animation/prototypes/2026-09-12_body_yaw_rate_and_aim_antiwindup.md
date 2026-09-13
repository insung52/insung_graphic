# 유한 몸통 각속도 · 조준 보정 적분 와인드업 제거

2026-09-12 / **성공(2건)** / ① 캡슐이 카메라를 **즉시** 따라가던 것을 **무기 자세에 따라 변하는 유한 각속도**(90~720°/s)로 바꿨다. ② 총을 내린 동안 조준 보정 적분기가 **열린 루프로 포화**(0.42초에 ±25° 클램프 도달)하던 것을 **`Enable_AO` 게이트**로 막았다.

관련 항목: **[C-76]** · **[C-77]** · **[R6]** 신설 / 관련 문서: `animation/prototypes/2026-09-11_sharp_turn_while_aiming.md` · `animation/prototypes/2026-09-11_muzzle_aim_alignment.md` · `IMPLEMENTED.md` 2.4·2.5c·2.5d·3절 · `CLAUDE.md` **P37~P40**

> **이 문서의 값어치는 결과가 아니라 실패 경로에 있다.**
> ① 틱 순서 때문에 **`SetRotationRate` 쓰기가 여러 라운드 동안 조용히 무시됐고**, 그걸
> "문제없어보임"이라는 **판별력 없는 확인**으로 통과시켰다(5절).
> ② 적분기 게이트를 **세 번** 잘못 걸었고, 세 번 다 이유가 달랐다(10절).

---

## 1. 이 세션이 한 일 두 가지

| # | 기능 | 한 줄 |
|---|---|---|
| **A** | **유한 몸통 각속도** (`UpdateBodyYawRate`) | 접지 중 `RotationRate.Yaw`를 `−1`(즉시)에서 **무기 자세로 보간되는 유한값**으로 바꿨다 |
| **B** | **조준 보정 안티 와인드업** | `AimCorrection` 적분기를 **`Enable_AO`(포즈를 실제로 고르는 함수)** 로 게이트했다 |

둘은 **같은 증상의 양끝**이다 — A가 "총을 내리면 빨리 돈다"를 물리로 만들었고,
그 결과 총이 내려가 있는 시간이 길어지자 B의 와인드업이 드러났다.

---

# A. 유한 몸통 각속도 (무기 자세 연동)

## 2. 문제 — "총을 내려서 빨리 돈다"에 근거가 없었다 [A]

2026-09-11에 `Enable_AO` 문턱을 70으로 낮춰 **급선회 중에는 총을 내리게** 했다
(`animation/prototypes/2026-09-11_sharp_turn_while_aiming.md` 4.2절). 그런데 그 결정에는
**아무 이득이 없었다** — 캡슐이 어차피 카메라를 **한 프레임에** 따라잡기 때문이다.

```
현실                          이때까지의 구현
총을 들고 있으면 못 돈다       조준 중이든 아니든 캡슐 yaw = 컨트롤러 yaw  (같은 프레임)
총을 내려야 빨리 돈다          "총을 내림"이 회전 속도에 아무 영향을 주지 않는다
```

**판정 기준**: 조준 중(총을 든 상태)에 마우스를 빠르게 90° 돌리면 **몸이 즉시 따라오지 않고**
눈에 보이는 시간이 걸릴 것, 그리고 **총이 내려가 있는 동안에는 확연히 빨라질 것**.

## 3. 원인 — 부모가 **매 프레임** 회전 속도를 덮어쓴다 [A]

GASP 부모 `/Game/Blueprints/SandboxCharacter_CMC`의 `UpdateRotation_PreCMC`
(2026-09-11에 읽어 둔 것을 재확인):

```
접지 중  SetRotationRate (0, −1, 0)     ← −1 = 즉시 회전 (CharacterMovementComponent.cpp:6595-6599)
공중     SetRotationRate (0, 200, 0)
조준 분기 → bUseControllerDesiredRotation = true , bOrientRotationToMovement = false
```

**이 함수는 매 프레임 돈다.** 구동원은 컴포넌트 `AC_PreCMCTick`이다:

```
AC_PreCMCTick (컴포넌트)  ── Tick 델리게이트 브로드캐스트 ─→  부모의 PreCMCTick 커스텀 이벤트
                                                              ├ UpdateRotation_PreCMC
                                                              └ UpdateMovement_PreCMC
```

> 즉 **부모 BP를 고치지 않고 회전 속도를 바꾸려면, 부모가 쓴 뒤에 우리가 덮어써야 한다.**
> 이 한 문장이 5절 실패의 전부다.

### 3.1 ⚠⚠ 오버라이드 경로는 막혀 있다 [A]

`UpdateRotation_PreCMC`를 직접 오버라이드해서 고치는 길을 먼저 시도했고 **버렸다**:

| 시도 | 결과 |
|---|---|
| `add_function_graph("UpdateRotation_PreCMC")` | ❌ 거부 — *"inherited event-shape function; it must be placed as an event node"* |
| `add_event("UpdateRotation_PreCMC")` | ⚠ **오버라이드가 만들어진다. 그런데 비어 있는 오버라이드는 부모 구현을 통째로 억제한다** — 캐릭터의 회전 설정이 아예 안 돌게 된다 |
| 그 안에 `Parent: UpdateRotation_PreCMC` 호출 노드 생성 | ❌ **이 API로는 만들 방법이 없다.** "Add call to parent function"은 에디터 수동 조작이다 |

**→ 부모 함수 오버라이드 경로는 수동 에디터 조작 없이는 쓸 수 없다.**
이 세션에서 만든 오버라이드는 **즉시 삭제했다.** (부모 BP는 손대지 않았다 — **P2** 유지)

## 4. 만든 것 — `BP_SoldierCharacter.UpdateBodyYawRate()` [A]

새 함수 그래프. **Event Tick 끝에서** 호출한다.

```
BodyErr   = | Delta(Rotator)( GetControlRotation , GetActorRotation ).Yaw |      캡슐 추종 오차

WpnTgt    = SelectFloat( 1.0 ,
                         MapRangeClamped( BodyErr ,
                                          WeaponLowerAngle , WeaponLowerAngleFull ,
                                          0.0 , 1.0 ) ,
                         bPickA = NOT AOActive )          ← AO가 꺼져 있으면 목표를 1.0으로 강제

WeaponLowered = Max( RampAxisTo( 현재 , WpnTgt , WeaponLowerRate , dt ) ,      ← 8.0  (0.125s)
                     RampAxisTo( 현재 , WpnTgt , WeaponRaiseRate , dt ) )      ← 2.0  (0.5s)

ω         = SelectFloat( 200 ,                                    ← 공중: 부모 값 유지
                         Lerp( YawRate_Up , YawRate_Down , WeaponLowered ) ,
                         bPickA = CharacterMovement.IsFalling )

CharacterMovement.SetRotationRate( MakeRotator( Roll=0 , Pitch=0 , Yaw=ω ) )
```

### 4.1 ★ `Max` 두 개로 **비대칭 램프**를 만든다 [A]

`RampAxisTo`는 등속 램프라 **한 방향 속도밖에 못 준다.** "내릴 때는 빠르게, 올릴 때는 느리게"를
비교 노드 없이 얻는 요령:

```
목표 > 현재 (내리는 중)   빠른 램프가 더 **큰** 값을 준다   →  Max 가 빠른 쪽을 고른다   0.125s
목표 < 현재 (올리는 중)   빠른 램프가 더 **작은** 값을 준다  →  Max 가 느린 쪽을 고른다   0.5s
```

**분기 없이 부호를 읽는다.** 블루프린트 팔레트에 산술 노드가 없다는 제약(**P33**) 아래에서
쓸 수 있는 몇 안 되는 수단이다.

### 4.2 재사용한 C++ [A]

`USoldierAxisLibrary::RampAxisTo(Current, Target, RatePerSecond, DeltaSeconds)` —
`Source/SoldierLab/Math/SoldierAxisLibrary.h`에 **이미 있던** 것이다.
`FInterpConstantTo`를 펼쳐 쓴 등속 램프이고 목표에서 정확히 멈춘다. **빌드가 필요 없었다.**

> `FInterpTo`/`Lerp`는 지수형이라 목표 근처에서 늘어지고, 입력이 멈추면 목표로 붕괴한다.
> 자세 축에는 **등속 램프**가 맞다(그 헤더의 주석이 이유를 적어 놨다).

## 5. ★★ 최대 실패 — 쓰기가 **조용히 무시됐다** [A]

**`SetRotationRate`를 Event Tick에서 써 봐야 아무 일도 일어나지 않았다.**
`AC_PreCMCTick`이 같은 프레임에 다시 `−1`로 덮어썼기 때문이다(3절).

이 상태가 **여러 라운드 동안 발각되지 않았다.** 이유는 기술이 아니라 검증 방법에 있다:

```
어시스턴트: (틱에서 SetRotationRate 하는 배선을 넣고) 확인해 주세요
사용자    : "문제없어보임"
어시스턴트: ✅ 로 접수하고 다음으로 넘어감          ← ★ 여기가 사고 지점
```

**"문제없어 보임"은 판별력이 없는 확인이다.** 회전 속도를 바꿔도 **애니메이션은 자기 속도로
계속 돌기 때문에** 육안으로는 빠른지 느린지 구별되지 않는다. 실제로 사용자는 중간에
**"몸이 느리게 도는 것 같기도"** 라고 보고했는데, 이 문장은 참·거짓 어느 쪽으로도 쓸 수 없다.

### 5.1 ★ 판별 신호 — `BodyErr == 0.000` 은 `RotationRate.Yaw = −1`의 **서명**이다 [A]

조준 중에는 `bUseControllerDesiredRotation = true`라 CMC가 캡슐을 컨트롤러 쪽으로 돌린다.
회전 속도가 **즉시**면 한 프레임에 도달하므로 **ActorRotation ≡ ControlRotation**이고,

```
BodyErr = |Delta(GetControlRotation, GetActorRotation).Yaw|  ==  0.000   (정확히)
```

**0이 아닌 `BodyErr`가 곧 "유한 각속도가 실제로 먹고 있다"는 증거다.** 반대로 0.000이 계속
찍히면 우리 쓰기는 무시되고 있는 것이다. **이 값을 화면에 올리자 한 프레임 만에 판정됐다.**

> **관찰이 불확실하면 관찰을 개선하는 것이 먼저다** — P10 · P16 · P29 · P34와 같은 계열.
> 이번에 새로 배운 것은 **"값에 서명이 있으면 감상 대신 그 값을 본다"** 이다 → **P39**.

### 5.2 확정타 — 극단값 시험 (P29) [A]

`YawRate_Up = 20`. 그러면 90° 회전에 **4.5초**가 걸려야 한다. 육안으로 절대 못 놓친다.
이 시험을 통과하기 전까지는 기능이 있다고 인정하지 않았다.

### 5.3 ★ 고친 방법 — 틱 선행 조건 [A]

`BeginPlay`의 **`Parent: BeginPlay` 직후**에 두 줄을 넣었다. **부모 BP는 손대지 않았다(P2).**

```
self.AddTickPrerequisiteComponent( GetComponentByClass( AC_PreCMCTick ) )
CharacterMovement.AddTickPrerequisiteActor( self )
```

보장되는 순서:

```
AC_PreCMCTick  (부모가 −1 을 쓴다)
      ↓
우리 Tick      (UpdateBodyYawRate 가 ω 를 덮어쓴다)
      ↓
CMC            (그 ω 로 캡슐을 돌린다)
```

**결과 [A]**: `YawRate_Up = 20`에서 `BodyErr`가 **최대 75**까지 올라갔다. 캡슐이 카메라보다
75° 뒤처졌다는 뜻 — 유한 각속도가 확실히 작동한다.

### 5.4 ⚠ DSL/API 사실 — `AddTickPrerequisite*`의 **타깃이 어느 인자인가** [A]

부모 BP의 DSL에는 이런 줄이 있다:

```lisp
(Components|Tick|AddTickPrerequisiteComponent (GetCharacterMovement) (GetAC_PreCMCTick))
```

**어느 쪽이 타깃(self)인지 DSL만 봐서는 알 수 없다.** `get_node_infos`로 핀을 직접 읽어 확정했다:

| 노드 계열 | 타깃(`self`) 위치 |
|---|---|
| `Components\|...` · `Actor\|...` · `SoldierLab\|...` | **첫 번째** 오브젝트 핀 |
| `Class\|...` (예: `Class\|...\|GetComponentByClass`) | **마지막** 오브젝트 핀 |

**따라서 위 줄은 "CMC가 `AC_PreCMCTick` 뒤에 틱한다"는 뜻이다** — 부모는 이미 같은 기법을
쓰고 있었다. 우리는 그 사슬의 **중간에** 우리 액터를 끼운 것이다.

## 6. 튜닝값 (인스턴스 편집 가능 변수) [A]

| 변수 | 값 | 왜 |
|---|---|---|
| `YawRate_Up` | **90** °/s | 총을 든 상태. 90° 돌리는 데 1초 — "총을 들고는 빨리 못 돈다"가 체감된다 |
| `YawRate_Down` | **720** °/s | 총을 내린 상태. 90°를 0.125초 — 사실상 제약이 없다 |
| `WeaponLowerAngle` | **30** ° | `BodyErr`가 이 값을 넘으면 총을 내리기 시작한다 |
| `WeaponLowerAngleFull` | **65** ° | 여기서 완전히 내려간다. `Enable_AO` 문턱 **70**보다 **약간 아래** — 포즈가 바뀌기 직전에 각속도가 먼저 풀려 있게 |
| `WeaponLowerRate` | **8.0** (= 0.125 s) | 내리는 쪽은 빠르게. 돌려야 할 때 즉시 권한을 준다 |
| `WeaponRaiseRate` | **2.0** (= 0.5 s) | 올리는 쪽은 느리게. **AO 블렌드(0.375 s)보다 느려야 한다** → 11.2절 |

> ⚠ `WeaponLowered`는 **캡슐 기준의 연속값**이고 `Enable_AO`는 **메시 기준의 불리언**이다.
> **둘은 같은 것이 아니다.** 이 차이가 B파트 두 번째 실패의 원인이다(10.2절).

---

# B. 조준 보정 적분 와인드업 제거

## 7. 증상 [A]

총을 내렸다가(우클릭을 떼거나, 급선회로 자동으로 내려가거나) **다시 들면 총구가 한참 위로
솟구쳤다가 1초쯤에 걸쳐 제자리로 내려온다.**

## 8. 기구 — **열린 루프 적분기** [A]

총구 정렬 보정(`animation/prototypes/2026-09-11_muzzle_aim_alignment.md`)은 순수 적분기다:

```
acc  +=  0.05 × ERR          (게인 0.05 = Lerp(Rotator) 의 Alpha)
acc   =  Clamp(±25°)
게이트:  max(|Δ카메라.Pitch|, |Δ카메라.Yaw|) ≤ 2.0°/프레임      ← 카메라 각속도 **뿐**
```

**총이 내려가 있으면 `Enable_AO`가 false가 되어 AO 갈래가 블렌드 아웃된다.
그동안 `AimCorrection`은 포즈에 아무 영향도 주지 않는다.** 그런데 적분기는 계속 돈다.

```
닫힌 루프:  보정 → 총구가 움직임 → ERR 감소 → 수렴
열린 루프:  보정 → (아무 일도 안 일어남) → ERR 그대로 → acc 가 **선형으로** 증가
```

**수렴하지 않는다. 포화한다.**

### 8.1 산수 [A]

```
ERR ≈ 20° ,  60 fps
0.05 × 20° = 1° / 프레임
±25° 클램프까지  25 프레임  =  0.42 초
```

**총을 0.5초만 내려도 보정은 이미 최대치다.** 사용자 관측과 일치한다 —
총이 내려가 있을 때 화면의 `AimCorr`는 **언제나 25.000**이었다.

## 9. 주입 경로 전수 확인 — 소비자는 하나뿐이다 [A]

고치기 전에 **`AimCorrection`이 포즈에 닿는 경로가 정말 하나인지** 확인했다.
`SoldierCharacter_ABP`의 **최상위 그래프 69개를 전수 조사**한 결과:

```
AimCorrection 을 읽는 곳   =  Get_AOValue  단 한 곳
Disable_AO 커브를 읽는 곳  =  Get_AOValue  단 한 곳
```

⚠ 두 그래프(`Update_TargetRotation` · `OnUpdate_TransitionToLocomotion`)는 **DSL로 읽히지 않았다** —
`RInterpTo`가 만드는 데이터 흐름 순환에서 리더가 죽는다. 이 둘은 `find_nodes` + `get_node_infos`로
**노드 타입을 직접 확인**해 `AimCorrection` 사용이 없음을 확정했다. 둘 다 깨끗하다.

> **소비 지점이 하나면 게이트도 한 곳이면 된다.** 이 확인이 11절의 설계를 가능하게 했다.

## 10. ★★ 실패한 게이트 3종 — 이 문서에서 가장 값진 부분 [A]

### 10.1 `bOrientRotationToMovement` (= 우클릭 여부) — **의도에 걸었다** ❌

우클릭을 떼면 곧바로 게인을 0으로. **우클릭 케이스는 정확히 고쳐졌다.**

**그런데 총을 다시 들 때 깨진다.** 게이트는 **명령**에 열리는데 무기 포즈는
`BlendListByBool_0`의 **0.375초** 블렌드를 거쳐야 올라온다. 그 창 동안 **게인은 이미 최대**다:

```
0.375 s × 60 fps ≈ 22 프레임
0.05 × 20° × 22 프레임  ≈  22°     ← ±25 클램프 코앞. 사실상 포화 그대로다
```

> **교훈: 의도에 건 게이트는 액추에이터의 수송 지연만큼 늦춰야 한다.** → **P38**

### 10.2 `WeaponLowered` / `BodyErr` (= 캡슐 기준) — **다른 신호에 걸었다** ❌

A파트에서 만든 `WeaponLowered`를 그대로 쓰면 될 것 같았다. **아니었다.**

```
포즈 게이트 :  Enable_AO   ←  Get_AOValue  ←  RootTransform  ←  OffsetRootBone 의 오프셋 루트 트랜스폼
                                                                (= 지연된 메시 회전, + yaw 90° 규약)
게인 게이트 :  BodyErr     ←  GetActorRotation                  (= 캡슐, 지연 없음)
```

**두 신호는 시간상수가 다르다.** `Update_EssentialValues`가 `RootTransform`을 **OffsetRootBone의
오프셋 루트 트랜스폼**에서 가져오기 때문에, 포즈 쪽은 메시 지연을 먹고 캡슐 쪽은 안 먹는다.

**결정적 증거**(스크린샷 계측):

```
BodyErr  0.000     WpnLow 0.000     AimGain 0.050        ← 게이트: "전부 정상"
AimErr   P 73      Y 99                                   ← 실제: 총이 등 뒤에 있다
```

**게이트가 "이상 없음"이라고 말하는 동안 무기는 눈에 보이게 등 뒤에 있었다.**
두 신호가 어긋나는 창이 곧 와인드업이 일어나는 창이다.

### 10.3 ✅ `Enable_AO` 자체 — **포즈를 실제로 고르는 함수에 걸었다**

```
ABP                 새 bool 변수 AOActive
                    Update_Logic 에서 Update_States **직후** 에  AOActive = Enable_AO()
                       ↑ 신선한 RootTransform / RotationMode 를 보게 하려고 이 자리다

캐릭터 Tick         ABP 에서 AOActive 를 **캐시**한다 (1프레임 지연)
AimGain             SelectFloat( 0.0 ,
                                 Lerp( 0.05 , 0.0 , WeaponLowered ) ,
                                 bPickA = NOT AOActive )
WpnTgt              AOActive 가 false 이면 목표를 **1.0 으로 강제** (4절)
```

**결과 [A]**: 사용자 확인 — **"완벽히 해결됬어"**, **"AOActive 는 총 내려가면 false 로 잘 나옴"**.

> ⚠ **왜 "캐시"인가**: `CastToSoldierCharacter_ABP` 노드는 Tick 안에서 `SetAimCorrection`
> **뒤에** 실행된다. 순수 체인으로 캐스트 출력을 끌어오면 그 시점엔 아직 **null**이라 값이
> 읽히지 않는다. 그래서 **캐릭터 변수에 받아 두고 다음 프레임에 쓴다.** 1프레임 지연은
> 0.375초 블렌드에 비하면 무시할 수 있다.

## 11. 최종형과 그 이유 [A]

### 11.1 왜 `Enable_AO`인가

**포즈를 고르는 함수가 곧 액추에이터의 결합 스위치다.** `BlendListByBool_0`은 `Enable_AO`의
바인딩으로만 구동되므로, `Enable_AO`가 false인 동안 `AimCorrection`은 **정의상** 포즈에
닿지 않는다. 다른 어떤 신호(의도 · 캡슐 오차 · 메시 지연)를 골라도 **어딘가에서 반드시 어긋나고,
어긋나는 창이 정확히 와인드업이 생기는 창이다.** → **P37**

### 11.2 ★ 회복은 **포즈보다 느려야 한다**

```
포즈가 올라오는 시간     BlendTime_0 = 0.375 s      (AO 켜는 쪽)
게인이 돌아오는 시간     WeaponRaiseRate 2.0 = 0.5 s
```

`AOActive == false`인 동안 `WpnTgt`를 1.0으로 강제해 두었으므로, 다시 들 때 게인은
**0.5초에 걸쳐** 돌아온다. **0.5 > 0.375** 이므로 포즈가 먼저 서고 게인이 나중에 붙는다 —
10.1절이 깨진 이유를 구조적으로 막았다. → **P38**

### 11.3 ★ "자기 참조라 안 된다"는 처음 판단은 **틀렸다** [A]

처음에 `Enable_AO`로 게이트하는 안을 **기각**했다. 이유는:

```
Get_AOValue = Delta( AimingRotation , Delta( RootRotation , AimCorrection ) )
                                                            ↑ 보정이 덧셈으로 들어 있다
Enable_AO   = | Get_AOValue.X | ≤ 70  AND  ...
→ "보정이 게이트를 움직이고 게이트가 보정을 움직인다. 자기 참조다."
```

**이 기각은 틀렸다.** 구분해야 할 것은 이것이다:

| | |
|---|---|
| ❌ **제어기 자신의 오차**로 게이트 | 자기 잠금. 틀어질수록 고칠 수 없게 된다 → **P35**, `animation/prototypes/2026-09-11_muzzle_aim_alignment.md` 10.2절 |
| ✅ **액추에이터가 결합돼 있는가**로 게이트 | 포화 인지형 안티 와인드업의 **교과서 형태**. 액추에이터의 결합 여부가 제어 출력에 부분적으로 의존해도 마찬가지다 |

적분을 멈추는 조건이 "오차가 크다"가 아니라 **"내 출력이 어디에도 가지 않는다"** 이므로,
게이트가 닫힌 상태가 오차를 키우는 되먹임이 없다. **잠기지 않는다.**

## 12. 확인한 부수 사실 [A]

| 사실 | 왜 중요한가 |
|---|---|
| **`Enable_AO`의 블루프린트 호출자는 0개다.** 유일한 소비자는 AnimGraph `BlendListByBool_0`의 바인딩 | 여기에 손대도 파급 범위가 사실상 없다. `AOActive` 변수를 따로 만든 근거이기도 하다 |
| `BlendListByBool_0`: `BlendPose_0`(true) = `AnimGraphNode_BlendSpacePlayer_1`(AO 블렌드스페이스) · `BlendPose_1`(false) = `AnimGraphNode_LayeredBoneBlend_0` | true가 0번(반전된 불리언 감각) — 09-11 문서 4.2절의 경고 그대로 |
| **핀 값 `BlendTime_0 = 0.375` / `BlendTime_1 = 0.25`** (노드의 `blendTime` 배열은 여전히 `[0.1, 0.1]`) | ⚠ **`IMPLEMENTED.md`와 09-11 문서에 적힌 0.75 / 1.50과 다르다.** 그 사이에 바뀐 값이다 → 17절에서 정정. 핀이 이긴다(**P25**) |
| `transitionType = Inertialization` · `blendType = Custom` · `customBlendCurve = /Game/Characters/UEFN_Mannequin/Animations/AimOffset/AO_Blend_Curve` · `blendProfile = SK_UEFN_Mannequin:FastHead_Weight` | 블렌드 시간이 곡선과 본 프로파일로 한 번 더 변형된다 → 커브 미확인 **[R6]** |
| **`SoldierCharacter_ABP`의 부모는 `/Script/Engine.AnimInstance`** 다 (GASP ABP가 아니다) | 자기 함수를 부르는 노드를 만들 때 `create_node`가 **같은 이름의 다른 클래스 함수**에 붙어 *"This blueprint (self) is not a SandboxCharacter_CMC_ABP_C, therefore 'Target' must have a connection."* 컴파일 에러가 난다. **`declaring_class`를 넘겨서** 해결 |
| `RotationMode`의 출처: `Get_PropertiesForAnimation`이 `select(bOrientRotationToMovement, NewEnumerator0, NewEnumerator1)`로 만든다 | 즉 **Strafe(`NewEnumerator1`) ⟺ `bOrientRotationToMovement == false`**. 10.1절의 우클릭 게이트가 `Enable_AO`와 **한 항만 공유**한다는 증거다 |

---

## 13. 기록만 하고 **고치지 않은 것** 2건

사용자 지시로 **기록만** 한다.

### 13.1 [C-76] 간헐적 무기 잠금 — 원인 미규명, 열어 둔다

**아주 가끔**, 급선회 뒤에 총이 내려간 채 **영영 올라오지 않는다.**

관측된 상태(스크린샷 `C:\Users\insung52\Pictures\Screenshots\스크린샷 2026-09-12 100104.png`):

```
BodyErr   0.000        WpnLow  0.000        WpnTgt  0.000
AimGain   0.050        AimGate true
AimCorr   P 25.000  Y 25.000          ← 양축 모두 클램프에 붙어 있다
AimErr    P 73.265  Y 99.063          ← 보정 권한(±25)의 4배
```

시각적으로는 **소총이 캐릭터 등 뒤에 있고 양팔이 들려 있다.** 시간이 지나도 풀리지 않고
**마우스를 움직여야만** 빠져나온다.

**분석 [B]**: 8절의 포화 적분기에는 **클램프에 두 번째 안정 평형점**이 있다.

```
오차 99°  >  보정 권한 25°
   → 보정을 최대로 밀어도 오차가 줄지 않는다
      → acc 는 클램프에 붙은 채 움직이지 않는다
         → 상태가 그대로 유지된다                 ← 외부 입력이 흔들 때까지
```

**총구를 처음 99° 틀어 놓는 방아쇠는 아직 모른다.** 안티 와인드업 적용 후에는 재현되지
않았지만 **근본 원인이 증명되지 않았으므로 닫지 않는다.** → **P40**

### 13.2 [C-77] 극단적 상방 조준 시 무기 공중제비 — **의도적 미해결(won't fix)**

거의 수직으로 위를 보면 무기가 뒤집히며 돈다. 짐벌 극점 부근에서 `AO_Rifle_ADS`의
pitch 표본 3장(−90 / 0 / +90)이 보간을 감당하지 못하는 것으로 보인다 **(추정)**.

**사용자 결정: 프로젝트 범위 밖. 고치지 않는다.** 전술 슈터에서 수직 상방 조준은
실제로 나오지 않는 자세이고, 고치려면 AO 표본 저작([C-73])이 필요하다.

---

## 14. 미확인 항목 — `AO_Blend_Curve` [R6] [C]

```
/Game/Characters/UEFN_Mannequin/Animations/AimOffset/AO_Blend_Curve
```

`BlendListByBool_0`의 `customBlendCurve`다. **이 세션에서 한 번도 열어보지 못했다** —
`FRichCurve`가 `ObjectTools`에 리플렉션되지 않고 `unreal` 파이썬 모듈은 막혀 있어
**어떤 MCP API로도 키 값을 읽을 수 없다.**

**왜 중요한가**: 커브의 최댓값이 **1.0을 넘으면** 블렌드가 오버슈트한다 — 즉 이 커브 자체가
**7절 증상의 독립적인 원인일 수 있다.** 안티 와인드업이 증상을 덮었더라도 남아 있을 수 있다.

**판정 기준**: 에디터에서 열어 최댓값이 ≤ 1.0인지 본다. **수동 확인 필요.**

---

## 15. 판정

| 기능 | 기준 | 결과 |
|---|---|---|
| A · 유한 몸통 각속도 | 조준 중 캡슐이 즉시 따라오지 않는다 | ✅ `BodyErr` 최대 75 (`YawRate_Up = 20` 극단값 시험) |
| A · 무기 자세 연동 | 총을 내리면 확연히 빨라진다 | ✅ 90 → 720 °/s |
| B · 와인드업 | 총을 다시 들 때 솟구침이 없다 | ✅ 사용자 확인 |
| B · 게이트 | 총을 내리면 `AOActive`가 false | ✅ 사용자 확인 |

**성공(2건).**

---

## 16. 툴링 — 이번에 새로 확정된 것

`CLAUDE.md` 6.1f절·P33에 반영했다. 요지만:

| 사실 | 영향 |
|---|---|
| `write_graph_dsl`의 파라미터 이름은 **`code`** (`script` 아님) | |
| **`Parent:` 호출 노드가 있는 그래프는 DSL 왕복이 불가능하다** — 리더는 `\|Parent:BeginPlay`를 뱉고 라이터는 *"does not exist"* 로 거부한다. 쓰기는 **어서션으로 중단**되며 그래프는 바뀌지 않는다 | `BeginPlay`/`Tick`이 있는 EventGraph는 **절대 DSL로 다시 쓰지 않는다.** `create_node`/`connect_pins`/`break_pins`로 고친다. 자기 완결형 함수 그래프는 왕복이 된다 |
| **DSL 라이터는 `bind`를 공유하지 않고 사용처마다 인라인한다** | 바인딩한 식이 **exec 노드**(예: 변수 Set)면 **두 개가 생기고 매 프레임 두 번 실행된다.** exec 문은 정확히 한 번만 등장하게 쓰고, 재사용은 **순수 게터만** |
| **DSL 리더는 분기 구조를 왜곡한다** | `Update_Logic`의 실제 구조는 `Branch.then → Update_MovementDirection → Update_TargetRotation` / `else` 비어 있음인데, DSL은 **양 갈래에 하나씩** 있는 if/else로 그렸다. **분기 위상은 `get_node_infos`로 확인한다** |
| `get_node_type_pins`는 **실제로 프로브 노드를 만들어** 동작한다 | 생성 불가 타입에서는 에러가 난다. 프로브는 남지 않는다 |
| **`find_node_types`는 멤버 이름에서 밑줄을 지운다** | 변수 `YawRate_Up` → `Variables\|Default\|GetYawRateUp`, ABP 함수 `Enable_AO` → `AimOffset\|EnableAO`. **DSL 리더는 여전히 밑줄형(`\|GetYawRate_Down`)을 찍는데 그 이름으로는 생성이 안 된다** (6.1f의 연장) |
| `remove_variable`의 파라미터는 **`name`**, `set_variable_instance_editable`은 **`variable_name`** | 헷갈리면 조용히 실패한다 |
| **한 `execute_tool_script`에서 그래프를 많이 훑으면 깨진다** | `K2Node_Composite`(컴포지트 서브그래프)가 **잡을 수 없는 `TypeError`** 로 배치 전체를 죽이고, `RInterpTo`의 데이터 흐름 순환은 `try/except`를 빠져나가는 에러를 낸다. **최상위 그래프만 거르고**(`:` 뒤 이름에 `.`이 없는 것) **묶음으로 잘라서** 돌린다 |
| **P33 정밀화** — 산술 연산자 노드(`Math\|Float\|float+float` 등)는 **그래프 안에 존재하고 읽히지만 `create_node`는 거부한다** | 쓸 수 있는 대체물: `Lerp(A,B,alpha)`(곱셈 대용) · `Math\|Float\|Max(Float)` · `Math\|Float\|SelectFloat` · `Math\|Float\|MapRangeClamped` · `Math\|Boolean\|NOTBoolean` |
| **6.1e 재확인** — PIE 중에도 `write_graph_dsl` · `create_node` · `compile_blueprint` · `ObjectTools.set_properties`는 **성공한다.** 실패하는 것은 `AssetTools.exists`(false)와 `save_assets`("Asset does not exist") | **수정은 들어가지만 저장은 PIE를 끈 뒤에** 해야 한다 |
| ⚠⚠ **`add_function_graph`는 이벤트 형태의 상속 함수를 거부하고, `add_event`로 만든 빈 오버라이드는 부모 구현을 통째로 억제한다.** `Parent: <함수>` 호출 노드를 만들 API가 없다 | **부모 함수 오버라이드 경로는 수동 에디터 조작 없이는 쓸 수 없다** (3.1절) |

---

## 17. 이것이 바꾸는 것

| 문서 | 절 | 어떻게 |
|---|---|---|
| `IMPLEMENTED.md` | 2.4 · **2.5d 신설** · 2.5c · 2.6 · 3 · 4 | `UpdateBodyYawRate` · 틱 선행 조건 · 새 변수 6개 + `AOActive` 2개 · 게인 체인 · `BlendTime` 정정 · 런타임 C++ 모듈 |
| `CLAUDE.md` | 5절 · 6.1f | **P37~P40** 추가 · DSL/API 사실 · **P33 정밀화** |
| `OPEN_ITEMS.md` | C · R절 | **C-76**(무기 잠금) · **C-77**(공중제비, 의도적 미해결) · **R6**(`AO_Blend_Curve`) 신설 |
| `animation/prototypes/2026-09-11_sharp_turn_while_aiming.md` | 2 · 4.2 → **9절 정정** | 접지 `RotationRate = −1`이 **더 이상 현행이 아니다** · `BlendTime` 정정 |
| `animation/prototypes/2026-09-11_muzzle_aim_alignment.md` | 9.2 → **13절 정정** | 게이트가 **카메라 각속도 단독이 아니다** — `AOActive` 게인 게이트가 붙었다 |

---

## 18. 남은 것

| ID | 항목 |
|---|---|
| **C-76** | 간헐적 무기 잠금 — 총구를 99° 틀어 놓는 **방아쇠**가 미규명. 포화 평형점 자체는 설명됐다(13.1절) |
| **C-77** | 극단 상방 조준 공중제비 — **의도적 미해결** |
| **R6** | `AO_Blend_Curve`의 키 값 (최댓값 > 1.0이면 독립적인 오버슈트 원인) — **에디터 수동 확인** |
| [C-74] | `maxRotationError 90` ↔ `Enable_AO 70`에 이제 **`WeaponLowerAngleFull 65`가 추가로 묶였다.** 셋을 함께 재야 한다 |
| [W6] | 계측·잔해 정리는 여전히 미완. 이번에 계측이 더 늘었다(`BodyErr` / `WpnLow` / `WpnTgt` / `AimGain`) |
| — | `YawRate_Up 90` · `YawRate_Down 720`은 **PIE에서 눈으로 정한 값**이다. 실제 병사가 조준 자세로 낼 수 있는 각속도를 재서 정한 값이 아니다 |
