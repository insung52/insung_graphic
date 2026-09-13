# 연속 stance 축 — 네 번째(마지막) 연속 자세 축

2026-09-12 / **성공** / `StanceAxis` 0..1 하나가 **골반 높이 · 이동 속도 상한 · Gait 라벨 · MM 데이터베이스**를 함께 몬다. 골반 높이는 **매 프레임 역산**으로 잡았다. 사용자가 단계마다 확인했다.

관련 항목: **[C-2]** · **[C-3]** · **[C-4]** 갱신 · **[C-79]** · **[C-80]** 신설 · **[W9]~[W13]** 신설 · **[W6]** 확장 · **[Q41]** 결정 / 관련 문서: `IMPLEMENTED.md` 2.4·**2.5f 신설**·2.6·3·4·6절 · `CLAUDE.md` **P44~P46** · 6.1절 · `animation/prototypes/2026-09-12_blind_fire_axis.md` · `animation/2026-09-02_pose_pipeline_spec.md` 6.2절(정정)

> **이 문서의 값어치는 배선이 아니라 8절의 실패 연쇄에 있다.**
> 문턱에서 캐릭터가 **한 번 툭 떨어지는** 증상 하나를 두고 **네 번 다르게 진단**했고,
> 그중 셋은 **정착 상태 값을 읽고 과도구간을 추론한 것**이라 틀렸다.
> 네 번째(= 실제 원인)는 **과도구간 자체를 계측하자 한 번에** 나왔다 → **P44**.
> 그리고 그 사이에 어시스턴트가 **앞서 두 번 통한 패턴(램프)을 반사적으로 재적용**했다 → **P45**.

---

## 1. 무엇을 만들었나 [A]

총구 조준 정렬(2026-09-11) · 린 · 블라인드 파이어(2026-09-12)에 이은 **네 번째이자 마지막 연속 축**이다.

| 축 | 범위 | 뜻 |
|---|---|---|
| `StanceAxis` | **0(기립) ~ 1(웅크림)** | 실루엣 높이. 누르고 있으면 등속 램프, **떼면 그 값에서 멈춘다** |

**조작 규약은 린·블라인드 파이어와 같다** — `StepAxis`로 등속 램프, 유지.

```
V = 내리기(웅크림 쪽)      B = 올리기(기립 쪽, Negate)
```

> **왜 C가 아닌가**: `C`는 **기존 앉기 토글**(`IA_Crouch`)이 이미 쓰고 있었다. 사용자가 V/B를 골랐다.

**판정 기준**: ① 키를 누른 만큼 **높이가 연속으로** 변하고 ② 떼면 그 높이를 유지하며 ③ 문턱에서 **툭 떨어지거나 솟구치지 않고** ④ 웅크린 채로 **달려지지 않으며** ⑤ 그동안 **MM이 발작하지 않을 것**.

---

## 2. 설계 분석 — "stance"는 **네 개의 하위 시스템을 묶은 이름**이다 [A]

먼저 확인한 것은 "무엇을 연속으로 만들어야 하는가"였다. 하나가 아니었다.

```
① 캡슐 높이        86 ↔ 60 (half-height), 반지름 30
② 눈/카메라 높이    100 ↔ 32
③ MM 데이터베이스   기립 클립셋 ↔ 웅크림 클립셋
④ AO 자산          AO_Rifle_ADS ↔ AO_Rifle_Crouch
```

**넷은 성질이 다르고, 따라서 처방이 달라야 한다.** ①②는 스칼라라 연속화가 가능하고, ③④는 **자산 선택**이라 원리적으로 이산이다.

### 2.1 실측값 [A]

```
capsuleHalfHeight              86        crouchedHalfHeight          60
baseEyeHeight                 100        crouchedEyeHeight           32
메시 컴포넌트 relative Z      −88
bCrouchMaintainsBaseLocation   false     ← 원래 값. 8.3절에서 true 로 바꿨다
```

### 2.2 ★ 클립 재고가 설계를 결정했다 [A]

```
기립     Walk  +  Jog          ← 두 단계
웅크림   Walk  만              ← **웅크려 달리는 클립이 없다**

PSD 6개가 기립 세트를 그대로 미러링한다
    PSD_Rifle_Crouch_Idles · Crouch_TurnInPlace ·
    Crouch_Walk_{Loops, Starts, Stops, Pivots}

중간 높이 클립은 **없다**
```

두 가지 결론이 여기서 바로 나온다.

- **웅크리면 속도를 Walk 상한으로 묶어야 한다.** 클립이 없으니 선택이 아니라 제약이다 → 4절
- **중간 높이는 클립이 아니라 *변형*으로 만들 수밖에 없다** → 골반 오프셋 + LegIK(3절)

### 2.3 ★ 결합 하나가 이미 풀려 있었다 [A]

```
walkSpeeds   (291.31, …)
crouchSpeeds (291.31, …)     ← 같다
```

`animation/prototypes/2026-09-09_lyra_rifle_migration.md` 9절에서 **클립 저작 속도를 실측해 맞춘 결과**(P30) 둘이 같은 값이 됐다. 즉 **속도는 이미 자세와 분리돼 있었다** — 공짜로 얻은 결합 해소 하나다. 남은 것은 "Jog를 못 쓰게 막는 것"뿐이다.

---

## 3. 1단계 — 축 · 입력 · HUD [A]

```
StanceAxis = StepAxis( StanceAxis , StanceInput , StanceRate , dt , 0 , 1 )
```

- `StepAxis`는 **기존 C++** `USoldierAxisLibrary`(`Source/SoldierLab/Math/`). 빌드 불필요
- 새 함수 **`UpdateStance()`** 를 **Event Tick 끝**, `UpdateBlindFire` **다음**에 부른다
- `IA_Stance`(Axis1D)는 **`IA_Lean` 복제**다 — 에셋 생성 함수가 없고(`CLAUDE.md` 6.1), `IA_Lean`은 트리거가 비어 있어 `InputTriggerPressed` 1프레임 버그를 자동으로 피한다
- `IMC_Sandbox` 매핑: **V** → 내리기 · **B** → 올리기(**Negate**)

**HUD 행** (`USoldierDebugAxes`): `Stance` 하나로 시작해, 8절의 진단 과정에서 **여섯 행이 더 붙었다** — `PelvWZ` · `PelvTgt` · `PelvOff` · `Crouched` · `SpdCap` · `GaitClamp`. **그 여섯 행이 이 문서의 결론을 만들었다**(P7·P44).

---

## 4. 2단계 — 골반 오프셋 + LegIK: **연속성의 다리 쪽 절반** [A]

### 4.1 ★★ 구조적 발견 — `FootPlacement`가 **골반을 소유한다**

```
FootPlacement_0
    pelvisBone       = pelvis
    pelvisSettings   : maxOffset 250 , linearStiffness 100 , linearDamping 1 ,
                       pelvisHeightMode = AllLegs
    interpolation    : on
```

즉 **골반을 스프링으로 붙잡고 있는 노드가 이미 있다.** 그 **앞**에 골반 오프셋을 넣으면 `FootPlacement`가 그것을 "보정해야 할 편차"로 보고 **되돌리려 든다**.

**그런데 `LegIK_1`이 `FootPlacement_0` *뒤*에 있다.** 그리고 `LegIK_1`은 `foot_l/r`을 **`ik_foot_l/r`에 맞추는** 노드이며, `ik_foot_*`는 `FootPlacement`가 **이미 땅에 심어 놓은** 것이다.

```
FootPlacement_0            발을 심는다 (ik_foot_* 확정)
      ↓
★ ModifyBone(pelvis)      골반만 내린다 — 발은 이미 확정돼 있어 따라 내려가지 않는다
      ↓
LegIK_1                    foot_l/r 을 ik_foot_* 로 되돌린다 → **무릎이 굽는다**
```

**골반을 내리면 LegIK가 무릎을 굽힌다. 그것이 곧 웅크림이다.**

### 4.2 배선 [A]

```
ModifyBone                bone            = pelvis
                          Translation ←  MakeVector( 0 , 0 , PelvisDrop )   ← ABP float
                          translationMode = BMM_Additive
                          translationSpace = BCS_ComponentSpace
```

사용자 확인: 기립 포즈에서 **−50이 사용 한계**다("스쿼트하는 정도"). 그 이상은 다리가 파탄난다.

> ⚠ **`animation/2026-09-02_gasp_abp_analysis.md` 10절과 `animation/2026-09-02_pose_pipeline_spec.md` 6.2절이
> "자세 높이 축은 `FootPlacement` **앞**"이라고 적고 있었다. 틀렸다** — 앞에 넣으면 스프링과 싸운다.
> **정답은 `FootPlacement`와 `LegIK` *사이*** 이고, 수단도 Control Rig이 아니라 **`ModifyBone` 한 개**였다.
> 두 문서에 정정 주석을 달았다.

---

## 5. 3단계 — 속도 상한, 그리고 **속도만으로는 부족했다** [A]

### 5.1 상한 (4a)

```
cap = Lerp( runSpeeds.x , walkSpeeds.x ,
            MapRangeClamped( StanceAxis , 0 , StanceThreshold , 0 , 1 ) )

CharacterMovement.MaxWalkSpeed         = Min( 부모가 쓴 값 , cap )
CharacterMovement.MaxWalkSpeedCrouched = Min( 부모가 쓴 값 , cap )
```

- **`Min`이라 우리 쓰기는 *낮추기만* 한다.** 부모가 무엇을 넣든 상한 역할만 한다
- 부모(`UpdateMovement_PreCMC`)는 **매 PreCMC 틱에 둘 다 다시 쓴다.** 우리 Tick이 그 뒤에 도는 것은 **앞 세션에서 넣은 틱 선행 조건** 덕이다(`IMPLEMENTED.md` 2.5d-1). 순서를 안 잡으면 **조용히 무시된다**(P39)
- **둘 다** 써야 한다 — 문턱을 넘으면 CMC가 `MaxWalkSpeedCrouched`를 보기 때문이다

### 5.2 ★★ 속도를 묶어도 **Gait 라벨이 Run에 남아 있었다**

속도만 291로 묶자 **매 걸음 캐릭터가 떨었다.** 원인:

```
GetDesiredGait 는 **입력 크기와 CanSprint** 로 Gait를 정한다 — **실제 속도를 보지 않는다**

→ Gait = Run 유지
→ 예측 궤적이 **582** 기준으로 만들어진다
→ 실제 속도는 291 → 어떤 Jog Loop도 맞지 않는다
→ MM이 **`Jog_Stop`을 반복 선택**한다  → 매 걸음 떨림
```

**이것은 뒷대각선 버그(P30/P31)와 서명이 정확히 같다** — 속도와 라벨이 어긋나면 **Loop가 아니라 Stop/Start/Pivot이 반복 선택**되고, 증상은 미끄러짐이 아니라 **떨림**으로 나온다.

**해결**:

```
Stance ≥ StanceThreshold  →  SetGait( NewEnumerator0 = Walk )     ← 부모의 SetGait 를 덮어쓴다
```

> ★ 이것은 **새 규칙이 아니라 일관성**이다. `Gait`는 **연속량(속도)에 붙은 이산 라벨**이다.
> 연속량을 바꿨으면 **라벨도 같이 바꿔야** 한다. 안 바꾸면 MM은 라벨을 믿고 궤적을 만들고,
> 그 궤적에 맞는 클립이 데이터셋에 없다 → **P46**.

---

## 6. 4단계 — DB 전환 (4b) [A]

```
StanceAxis ≥ StanceThreshold   →  Crouch()        매 Tick 호출
StanceAxis <  StanceThreshold   →  UnCrouch()      매 Tick 호출
        → IsCrouching()  →  ABP 의 Stance 열거형  →  Chooser  →  Crouch 계열 PSD 선택
```

**GASP의 DB 구동 선택 경로를 그대로 유지한다.** 사용자가 명시적으로 요구한 조건이다 — 이걸 끄면 *"GASP의 기능을 거의 안 쓰는 것"* 이 된다.

> ⚠ **결과: 기존 `IA_Crouch` 토글이 무력해졌다** [A]. stance 축이 `bIsCrouched`를 **매 프레임 소유**하므로 토글이 무엇을 하든 다음 프레임에 덮어써진다. 제거하거나 "stance를 0/1로 명령하는 입력"으로 재정의할 것 → **[W13]**

---

## 7. 5단계 — ★ 골반 높이 **역산** (최종형) [A]

문턱에서 골반 오프셋이 **50유닛을 즉시 뛰는** 것이 8절의 증상이었다. 최종 해법은 **오프셋을 직접 정하지 않고, 원하는 월드 높이에서 매 프레임 역산하는 것**이다.

### 7.1 새 C++ 두 개 — `USoldierAxisLibrary` [A]

```cpp
// Source/SoldierLab/Math/SoldierAxisLibrary.h
static float SolveBoneHeightOffset(float TargetWorldZ, float MeasuredWorldZ, float AppliedOffset);
    // = TargetWorldZ - (MeasuredWorldZ - AppliedOffset)

static float StanceTargetHeight(float ActorWorldZ, float MeshRelativeZ,
                                float StandLocalZ, float CrouchLocalZ, float Stance);
    // = (ActorWorldZ + MeshRelativeZ) + Lerp(StandLocalZ, CrouchLocalZ, Stance)
```

```
PelvTgt = StanceTargetHeight( ActorZ , MeshRelZ , StanceStandZ , StanceCrouchZ , StanceAxis )
PelvOff = SolveBoneHeightOffset( PelvTgt , PelvWZ(실측) , PelvOff(직전 프레임) )
        → ABP  PelvisDrop
```

**핵심은 `− AppliedOffset` 한 항이다.** `MeasuredWorldZ`에는 **직전 프레임에 우리가 넣은 오프셋이 이미 들어 있다.** 그걸 빼야 **애니메이션 자신이 만든 높이**가 복원된다.

### 7.2 ★ 이것은 되먹임 루프가 아니다 — 발산하지 않는다 [A]

전개해 보면 이전 오프셋이 소거된다:

```
Measured_n   =  clipHeight_{n−1}  +  offset_{n−1}
offset_n     =  Target_n  −  ( Measured_n  −  offset_{n−1} )
             =  Target_n  −  clipHeight_{n−1}
```

**남는 것은 "목표 − 직전 프레임의 클립 높이"뿐이다.** 즉 **1프레임 지연을 갖는 피드포워드 해**이지, 게인이나 극점을 갖는 폐루프가 아니다. 진동할 구조 자체가 없다.

**그래서 블렌드 길이도 블렌드 커브도 알 필요가 없다.** DeadBlending이 클립을 어떻게 섞든, 섞인 결과의 골반 높이를 실측해서 그 차이를 채우면 된다. 8.4절의 "블렌드를 램프로 상쇄한다"는 접근이 왜 원리적으로 불가능했는지가 여기서 같이 설명된다.

### 7.3 왜 블루프린트가 아니라 C++인가 [A]

**두 runtime float의 뺄셈**이 필요하다. 블루프린트 팔레트에 산술 노드가 없고(**P33**), 지금까지 써 온 우회 수단으로는 표현이 안 된다:

| 우회 수단 | 쓴 곳 | 왜 여기선 안 되나 |
|---|---|---|
| `Lerp`를 곱셈 대용으로 | 각속도 보간(2.5d) | 뺄셈이 아니다 |
| `MapRangeClamped`를 반파 정류기로 | 블라인드 파이어(2.5e) | 상수 구간 매핑이지 두 변수의 차가 아니다 |
| `Delta(Rotator)` | 총구 보정(2.5c) | **회전 전용**이다 |

→ **C++ 두 함수를 추가하는 것이 정직한 해법**이었다. `USoldierAxisLibrary`는 이미 존재하므로 새 `UCLASS`가 아니고, 따라서 **P13(에디터 닫고 빌드)에 걸리지 않는다.**

### 7.4 부산물 — 죽은 변수 3개 [A]

`StanceDropMax` · `StanceRaiseMax` · `StanceBlendRate`는 8.4절의 실패한 접근들이 남긴 것이고 **이제 아무도 읽지 않는다.** 정리 대상 → **[W6]**.

---

## 8. ★★★ 실패 연쇄 — 같은 증상에 네 개의 틀린 진단

증상은 하나였다. **문턱을 지나는 순간 캐릭터가 툭 주저앉았다가 자세를 잡는다.**

### 8.1 ❌ 가설 1 — "전환 클립이 재생된다" (사용자 제기) [A]

**합리적인 가설이었다.** 실제로 클립이 **존재하기 때문**이다:

```
/Game/SoldierLab/Animations/Rifle/_Extra/MM_Rifle_Crouch_Entry
/Game/SoldierLab/Animations/Rifle/_Extra/MM_Rifle_Crouch_Exit
```

**근거로 기각했다**:

```
get_referencers(MM_Rifle_Crouch_Entry)   →  **0건**
get_referencers(MM_Rifle_Crouch_Exit)    →  **0건**

get_dependencies(PSD_Rifle_Crouch_Idles) →  MM_Rifle_Crouch_Idle     만
get_dependencies(PSD_Rifle_Stand_Idles)  →  MM_Rifle_Idle_ADS        만
```

**MM에는 이 클립들을 재생할 경로가 없다.** 앉는 것처럼 보인 것은 **인어셜라이즈 블렌드 + 오프셋 점프**였다.

> `_Extra/`에 있고 참조가 0이면 **보관물**이다. 이 판정은 `get_referencers` 한 번이면 끝난다.

### 8.2 ❌ 가설 2 — "`OnStartCrouch`에서 메시가 26유닛 즉시 점프한다" (어시스턴트 제기) [A]

`crouchedHalfHeight 60` vs `capsuleHalfHeight 86` → 차 **26**. 캡슐이 줄면 메시가 그만큼 떠 보일 것이라는 추론이었다.

**계측으로 기각했다**:

```
기립     ActorZ 88.284   CapHH 86   MeshZ −88      →  ActorZ + MeshZ =  0.284
웅크림   ActorZ 62.284   CapHH 60   MeshZ −62      →  ActorZ + MeshZ =  0.284
```

**셋이 같은 프레임에 함께 바뀌고, 합은 변하지 않는다. 엔진이 이미 상쇄하고 있다.**

### 8.3 ❌ 가설 3 — "`bCrouchMaintainsBaseLocation = false`가 캡슐 바닥을 띄운다" [A]

추론은 **증상과 정확히 일치했다** — `false`면 앉을 때 캡슐 **바닥**이 올라가고, CMC가 그것을 다시 바닥으로 스냅하니 한 번 툭 떨어져 보인다.

**그래서 `true`로 바꿨다. 아무것도 안 변했다.** HUD에 `CrouchMB` 행을 띄워 런타임 값이 실제로 `true`인 것까지 확인했다. **기각.**

> **설정 자체는 그 자체로 옳으므로 되돌리지 않고 남겼다** [A]. 다만
> **"고치려던 증상은 못 고쳤다"를 정직하게 적는다** — 안 적으면 다음 사람이
> 이 값이 그 증상의 해결책이라고 읽는다.
>
> ⚠ **쓰기 수단**: `ObjectTools.set_properties`로는 **비트필드(`uint8:1`)를 못 쓴다.**
> `BeginPlay`에서 **`Class|CharacterMovementComponent|SetCrouchMaintainsBaseLocation`** 노드로 넣었다.
> 덤으로 **그래프에 변경이 보인다**는 이점이 있다 → `CLAUDE.md` 6.1

### 8.4 ✅ 실제 원인 — **즉시 전환 vs 블렌드된 전환** [A]

```
우리 오프셋   문턱에서 **즉시** 50유닛 점프        (설계상 그렇게 만들었다)
클립 교체     DeadBlending 이 **블렌드**한다       (인어셜라이즈)
```

**한쪽은 계단, 한쪽은 곡선이다. 어긋나는 구간이 곧 팝이다.**

직접 계측했다 — 전환 중 **`PelvWZ`가 ±50 스파이크**를 그리고, 전환 전후에는 양쪽 다 정확히 맞았다.

**두 방향의 시도가 모두 실패했다**:

| 시도 | 왜 실패했나 |
|---|---|
| 오프셋을 **램프**로 (`StanceBlendRate`) | 직선 램프는 **인어셜라이즈 곡선을 상쇄하지 못한다.** 두 곡선의 모양이 다르므로 잔차가 남는다 |
| 둘 다 **즉시**로 | 클립 교체 쪽을 즉시로 만들 수단이 없다 — 블렌드는 DeadBlending이 한다 |

**→ 7절의 역산.** 상쇄를 시도하지 않고, **매 프레임 실측해서 남은 차이를 채운다.** 블렌드의 모양을 알 필요가 없어진다.

### 8.5 ⚠ 어시스턴트의 자기 번복을 기록한다 → **P45** [A]

```
1차   "즉시 전환 + 지연되는 액추에이터 → 램프로 맞춘다"
      근거로 **총 내리기**(WeaponLowerRate)와 **조준 게인 회복**(P38) 두 사례를 들었다
2차   "아니다. 둘 다 즉시여야 상쇄된다"  → 램프 제거
결과  **둘 다 틀렸다.** 정답은 셋째 범주(역산)였다
```

**앞서 두 번 통한 패턴을, 겉만 닮은 문제에 반사적으로 적용했다.** 두 선례에서 액추에이터는 **단조로운 1차 지연**이었고 램프로 맞출 수 있었다. 여기서는 액추에이터가 **모양을 알 수 없는 인어셜라이즈 곡선**이라 **어떤 직선으로도 상쇄되지 않는다** — 애초에 다른 문제였다.

> **판정법: "이 패턴이 저기서 통한 *이유*가 여기서도 성립하는가"를 한 문장으로 말해 본다.**
> 못 하면 그건 유추가 아니라 습관이다. (P36의 "패치 두 개째가 신호다"와 같은 계열)

### 8.6 ★ 계측 규율 — 셋 중 셋이 같은 오류였다 → **P44** [A]

```
가설 1   참조 0건 — **정착 상태의 에셋 관계**를 보고 전환 중 거동을 추론했다
가설 2   기립/웅크림 **정착값** 두 벌을 비교했다 — 그 사이 프레임은 안 봤다
가설 3   프로퍼티 **설정값**을 보고 전환 중 캡슐 거동을 추론했다
```

**셋 다 "정착 상태를 읽고 과도구간을 추론"했다. 증상은 과도구간에만 존재한다.**

해결은 **과도구간 자체를 계측**하고 나서 나왔다:

```
HUD 행 추가   PelvWZ · PelvTgt · PelvOff · ActorZ · CapHH · MeshZ · CrouchMB
읽는 법       콘솔 `slomo 0.1`    ← 1프레임 과도현상을 눈으로 읽을 수 있게 늘린다
```

> **`slomo 0.1`은 이 프로젝트의 기본 도구로 승격할 만하다.** 계측 행이 있어도
> 1프레임짜리 스파이크는 화면에서 읽히지 않는다. 10배로 늘리면 읽힌다.

---

## 9. 움찔 버그 — 문턱을 0.35 → 0.5로 옮겨서 해결 [B]

높이 문제가 끝난 뒤에도 **별개의 증상**이 남아 있었다.

```
조건   제자리 · **조준 안 함** · 천천히 웅크리는 중 · stance **0.35~0.5** 구간
       (= 당시 StanceThreshold 0.35 를 막 넘긴 직후)
증상   약 1초에 한 번 움찔거리고 **실제로 몸이 돈다**
해소   조준하거나 · 일어서거나 · 움직이면 사라진다
```

### 9.1 기각한 가설 둘 — MM 디버그 오버레이가 판정했다 [A]

```
a.AnimNode.MotionMatching.DebugDrawInfo 1
a.AnimNode.MotionMatching.DebugDrawInfoVerbose 1
```

| 가설 | 판정 |
|---|---|
| "Chooser의 **비조준 행이 Stance로 게이트되지 않아** `PSD_Rifle_Stand_Idles_LowReady`(IdleBreak 포함)가 검색 대상에 남는다" | ❌ **틀렸다.** 오버레이의 `Databases to search:`에는 **`PSD_Rifle_Crouch_Idles`와 `PSD_Rifle_Crouch_TurnInPlace` 둘뿐**이었다 |
| 비용 편향이 또 문제다 | ❌ **아니다.** 앞선 교정이 그대로 살아 있다 — `Crouch_TurnInPlace.baseCostBias 0` · `Crouch_Idles.loopingCostBias −0.10`. 그래서 새 원인을 찾아야 했다 |

**오버레이가 보여준 실제 상태**:

```
선택된 클립   MM_Rifle_Crouch_TurnLeft_90
Blend Stack   같은 클립이 **두 벌**, time 0.48 / 1.57
              → 제자리회전 클립이 약 **1.1초마다 자기 자신을 재트리거**하고 있다
```

`ShouldTurnInPlace`는 **`|Delta(조준 방향, 몸 방향)| ≥ 50°`** 에서 발동한다.

### 9.2 해결 — 사용자의 제안 [B]

**`StanceThreshold` 0.35 → 0.5.** 확인: **"움찔구간 완벽히 사라짐".**

**기구는 증명되지 않았다** [B]. 그럴듯한 설명은 이것이다:

```
문턱 0.35 에서 전환 → 웅크림 클립보다 골반을 약 **33유닛 들어올려야** 한다
문턱 0.5  에서 전환 → 약 **25유닛**
```

다리와 발 심기에 가해지는 무리가 작아진다는 것인데, **그것이 제자리회전 재트리거를 멈춘 경로는 추적하지 않았다.**

### 9.3 재발하면 볼 레버 둘 — 미시험 [B] → **[C-79]**

| 레버 | 상태 |
|---|---|
| **`Crouch_TurnInPlace.continuingPoseCostBias = −0.01`** 인데 **`Stand_TurnInPlace`는 −0.05** | 남아 있는 **비대칭**이다. 연속 포즈 할인이 작을수록 **재선택이 쉬워진다** — 증상의 방향과 일치한다 |
| **`OffsetRootBone.maxRotationError`를 −1 → 90 으로 바꾼 것**(2026-09-11, 급선회 뒤집힘 대책) | **90° 클램프가 90° 제자리회전이 각을 해소하는 것을 막을 수 있다.** 그러면 회전이 영영 끝나지 않고 재트리거된다 → **자초한 회귀 후보** → **[C-80]** |

> ⚠ **[C-80]은 [C-74]의 결합 3개조**(`maxRotationError 90` / `Enable_AO 70` / `WeaponLowerAngleFull 65`)**를 건드린다.** 한쪽만 바꾸지 말 것.

---

## 10. 함께 고친 별개 버그 — `GetControlRotation(GetController())` [A]

로그에 이것이 찍히고 있었다:

```
Accessed None trying to read CallFunc_GetController_ReturnValue
```

**두 곳**이 같은 형태였다 — `UpdateBodyYawRate`와 Tick의 조준 루프가 둘 다 `GetControlRotation(GetController())`를 부른다. **`GetController()`가 null이면 조준 방향이 `(0,0,0)`으로 읽힌다.**

> ★ **이것은 AI에게 결정적이다.** 총구 정렬 보정(2.5c)과 무기 내리기(2.5d)는 **45명의 AI 병사를 위해 만든 기능**인데, 그 입력이 쓰레기값이면 `BodyErr`가 통째로 무의미해진다.

**GASP 자신의 패턴으로 교체했다** (두 곳 모두):

```
SelectRotator( A = GetControlRotation[Pawn] ,
               B = GetBaseAimRotation ,
               bPickA = IsLocallyControlled )
```

죽은 노드 **3개**를 같이 제거했다.

### 10.1 ⚠ 함정 — `GetControlRotation`은 **두 클래스에 있다** [A]

```
APawn::GetControlRotation          ← 우리가 원하는 것
AController::GetControlRotation
```

DSL 라이터가 **Controller 쪽에 붙였고**, 컴파일 에러가 났다:

```
This blueprint (self) is not a Controller, therefore ' Target ' must have a connection.
```

**DSL로는 `declaring_class`를 넘길 수 없다.** `create_node` + **`declaring_class = /Script/Engine.Pawn`** 으로 만들어야 한다. `CLAUDE.md` 6.1f-1·6.1f-2의 같은 함정이 **세 번째**로 나왔다.

---

## 11. 설계 결정 — **자세는 AI가 소유하고, 속도는 결과다** [A] → **[Q41]**

사용자 질문: 웅크린 채 달리려 하면 **(A) 속도를 묶을 것인가, (B) 웅크림을 풀 것인가.**

**(A)를 채택했다.** 이유가 이 프로젝트 전체에 적용되므로 여기 적는다.

```
자세  =  생존 결정 (노출 실루엣)      ← 협상 불가
속도  =  그 결정의 결과              ← 협상 가능
```

**(B)는 이동 요청이 실루엣을 조용히 높인다.** AI는 "일어서라"고 **명령한 적이 없는데** 일어서 있고, 그 결정은 **어디에도 기록되지 않아 추적이 불가능**하다. 엄폐 판단이 캡슐/자세 높이를 읽는 이상 이것은 곧 **원인을 모르는 피격**이 된다.

> **P37 · P38과 같은 계열이다** — 소유자가 하나여야 하고, 부수 효과로 남의 상태를 덮어쓰면 안 된다.

**슈터 관례의 손맛은 비용 없이 살릴 수 있다**: **스프린트 *입력*이 입력 계층에서 stance를 0으로 명령**하게 하면 된다. 그러면 "달리면 일어선다"는 체감은 그대로이고, **AI는 그 명령을 내리지 않으면 그만**이다. 규칙이 아니라 **입력의 의미**로 표현된다.

### 11.1 기각한 대안 — 두 MM 출력을 블렌드하기 [A]

"기립 로코모션과 웅크림 로코모션을 연속으로 섞으면 되지 않나"는 자연스러운 발상이지만 **두 가지 이유로 기각했다.**

| 이유 | 내용 |
|---|---|
| **위상 동기가 필요하다** | 두 걸음 사이클을 섞으려면 **동기 마커로 위상을 맞춰야** 한다. 재료 자체는 있다 — ABP가 `Phase_History` · `Contact_L_History` · `Contact_R_History`를 들고 있다. **없는 것은 재료가 아니라 예산이다** |
| **캐릭터당 MM 검색이 두 번** | 45명 × 60fps에서 감당할 수 없다 ([C-11]과 같은 종류의 비용) |
| 위상이 어긋나면 | **발이 미끄러진다.** 상관없는 두 사이클의 평균은 어느 쪽도 아니다 |

**채택한 설계: 로코모션만 한 문턱에서 이산, 나머지는 전부 연속.**

---

## 12. 확정값 · 변수 · 계측 [A]

**튜닝값** — 전부 `BP_SoldierCharacter`에 **Instance Editable**

| 변수 | 값 | 왜 |
|---|---|---|
| `StanceRate` | **1.0** | 0→1 에 1초. 린·블라인드 파이어와 같은 감각 |
| `StanceThreshold` | **0.5** | **DB 전환점이자 속도 상한의 무릎**. 0.35에서 올렸다 — 9절 |
| `StanceStandZ` | **89.7** | 기립 클립에서 **메시 루트 위 골반 높이** |
| `StanceCrouchZ` | **39.4** | 웅크림 클립에서 같은 값 |

**HUD 행 (신규 7)**: `Stance` · `PelvWZ` · `PelvTgt` · `PelvOff` · `Crouched` · `SpdCap` · `GaitClamp`
**죽은 변수 (정리 대상)**: `StanceDropMax` · `StanceRaiseMax` · `StanceBlendRate` → **[W6]**

---

## 13. 판정

| 기준 | 결과 |
|---|---|
| ① 높이가 연속으로 변한다 | ✅ 골반 역산 + LegIK |
| ② 떼면 유지 | ✅ `StepAxis` |
| ③ 문턱에서 안 튄다 | ✅ 7절의 역산으로 해결 (그 전까지 4번 틀림) |
| ④ 웅크린 채 못 달린다 | ✅ 속도 상한 **+ Gait 클램프**(둘 다 필요) |
| ⑤ MM 발작 없음 | ✅ 문턱 0.5 이후 — 단 **기구 미증명** [B] → [C-79] |

**사용자가 단계마다 확인했다** → **성공.**

---

## 14. 남은 것

| ID | 항목 |
|---|---|
| **W9** | ★ **캡슐이 여전히 이진이다** — `Crouch()`가 문턱에서 86↔60을 한 번에 바꾼다. **보이는 높이만 연속이고 충돌·엄폐 높이는 이진**이다. **엄폐 판단이 읽는 것이 바로 그 높이**이므로 AI 관점에서 가장 값어치 있는 남은 조각이다. 동시에 **위험도 가장 높다** — 관통, 계단 오르기, 그리고 **일어설 때의 천장 스윕**을 CMC는 **이진 경우에 대해서만** 구현해 두었다. 안정성 판정은 **[C-3]** |
| **W10** | **카메라** — `CameraRig_CrouchOffset`은 Camera Pose 공간의 고정 `TranslationOffset (40, 0, −30)`이고 **블렌더블/데이터 파라미터가 하나도 없다.** 즉 **블렌드되는 이진**이지 float를 못 받는다. **결정: 리그를 고치지 않는다. 비활성화하고 SpringArm Z를 stance 축으로 직접 몬다**(캐릭터가 이미 SpringArm을 갖고 있다). 미착수 |
| **W11** | **AO 연속 블렌드** — 지금은 `Select(Stance == Crouch ? AO_Rifle_Crouch : AO_Rifle_ADS)` **이진**이다. DB 전환과 **같은 순간**에 갈리므로 눈에 거슬리지 않아 뒤로 미뤘다 |
| **C-79** | **9절 움찔의 기구 미증명** [B] + 레버 둘(`continuingPoseCostBias` 비대칭) |
| **C-80** | **`OffsetRootBone.maxRotationError = 90`이 90° 제자리회전을 막는가** — 자초한 회귀 후보 |
| **W13** | **`IA_Crouch` 토글이 무력해졌다** — stance 축이 `bIsCrouched`를 매 프레임 소유한다 |
| **W6** | 계측/잔해 정리에 **7행 + 죽은 변수 3개**가 추가됐다 |
| — | **웅크려 걷기 다리 사이클**: DB 전환 후에는 웅크림 Walk 클립을 쓰므로 정상이다. 다만 **0 ~ 문턱 구간에서는 기립 사이클을 골반만 내린 채** 걷는다 [A]. 수용 가능하다고 판단했다 → [C-2] |
| **W12** | **중간 자세 3점 블렌드** [B] — 중간 웅크림 포즈를 **한 장 저작**해 0 / 0.5 / 1 로 블렌드하면, 두 끝점 사이 직선을 받아들이는 대신 **가운데를 저작자가 통제**할 수 있다. 저작 경로는 이미 있다(`CLAUDE.md` 6.3절). **[C-2]의 품질 개선 수단이기도 하다** |

---

## 15. 툴링 — 이번에 확정된 것

`CLAUDE.md` 6.1절에 반영했다.

| 사실 | 영향 |
|---|---|
| **`ObjectTools.set_properties`는 비트필드(`uint8:1`)를 못 쓴다** — `bCrouchMaintainsBaseLocation` 실패 | **블루프린트 세터 노드**를 쓴다(`Class\|CharacterMovementComponent\|SetCrouchMaintainsBaseLocation`). 덤으로 변경이 그래프에 보인다 |
| **`PoseSearchDatabase`의 멤버는 `get_properties`로 안 읽힌다** (`animationAssets` · `notifyRecencyTimeOut` 둘 다 실패) | **`AssetTools.get_dependencies`를 PSD에 걸면** 참조 애니메이션이 나온다. 반대로 **`get_referencers`를 클립에 걸면** 어느 DB가 쓰는지 나온다 — `MM_Rifle_Crouch_Entry`가 미사용임을 이걸로 증명했다(8.1절) |
| **`CameraRigAsset`의 내부는 안 읽힌다** (`rootNode` 실패) | 카메라 리그는 **에디터에서 손으로** 본다 |
| **DSL의 `(if cond A B (else C D))`는 왕복된다** | `Utilities\|FlowControl\|Branch`로 기록되고 **한 갈래에 여러 문장**이 들어간다. `"NewEnumerator0"` 같은 **열거형 리터럴도 값으로 받는다** |
| **`Components\|SkeletalMesh\|GetSocketLocation`은 DSL에서 타깃 바인딩에 실패한다** (*"Could not connect pin Mesh to self"*) | **`Transformation\|GetSocketTransform <컴포넌트> "<소켓>"`** 을 쓰고 `(.z (.location ...))`로 높이를 뽑는다 |
| **명시적 `self` 인자를 받는 노드와 거부하는 노드가 있다** | `Transformation\|GetActorRotation self` ✅ / `Pawn\|IsLocallyControlled self` ❌ (*"Could not connect pin self to self"*). **Pawn 계열이 거부하면 생략한다** |
| ⚠⚠ **PIE 중에는 런타임 로그 스팸(`Accessed None`)이 툴 *에러*로 올라와 `execute_tool_script`를 통째로 중단시킨다** | 읽기 전용 그래프 조사조차 막힌다. **그래프 작업 전에 PIE를 끈다** (6.1e의 연장) |
| **`slomo 0.1`** | 1프레임 과도현상을 디버그 HUD에서 **읽을 수 있게** 만드는 실용적 수단 (8.6절 · **P44**) |

---

## 16. 이것이 바꾸는 것

| 문서 | 절 | 어떻게 |
|---|---|---|
| `IMPLEMENTED.md` | 0 · 2.4 · **2.5f 신설** · 2.6 · 3 · 4 · 6 | 연속 축 **4종**으로 · 체인에 `ModifyBone(pelvis)` 추가 · stance 축 전체 · 튜닝값 4개 · C++ 2함수 · `SelectRotator` 정정 |
| `CLAUDE.md` | 5절 · 6.1 | **P44 ~ P46** · 툴링 사실 8건 |
| `OPEN_ITEMS.md` | C · Q · W | **[C-79] · [C-80] · [W9]~[W13] · [Q41]** 신설 · [C-2]·[C-3]·[C-4]·[C-75]·[W6] 갱신 |
| `animation/2026-09-02_pose_pipeline_spec.md` | 6.2 | **"FootPlacement 앞"은 틀렸다** — 사이다. 수단도 Control Rig이 아니라 `ModifyBone` |
| `animation/2026-09-02_gasp_abp_analysis.md` | 10 | 같은 정정 |
| `CURRENT_STATE.md` | 머리말 | 2026-09-12 추가분 (3) |
