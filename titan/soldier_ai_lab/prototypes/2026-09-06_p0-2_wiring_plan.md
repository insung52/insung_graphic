# P0-2 배선 계획 — 견착 DB를 실제로 재생시키는 최소 경로

2026-09-06 / 조사 완료·착수 전 / MM이 우리 견착 DB를 조회하게 만드는 데 무엇이 필요한지 확정했다.

관련: **[C-44]**(견착 DB에서 급선회) · **[C-24]**(점진적 폴백) · [C-48](회전 노티파이) / 관련 문서:
`../animation/2026-09-02_gasp_abp_analysis.md` 7·15.2·15.3절 · `2026-09-04_c34_clip_curve_mapping.md`

---

## 1. 현재 재료 — 클립 1개뿐이다 [A]

`get_dependencies`로 확인:

```
PSD_Soldier_Walk_Test  →  Walking_Anim  하나뿐
                          schema PSS_Default · normset PSN_Dense_All  ✅ (CMC 경로, [Q11] 확정과 일치)
```

| 클립 | 붙은 모디파이어 | 커브 | PSD |
|---|---|---|---|
| `Walking_Anim` | 6종 (완성) | 5종 + `L`/`R` 마커 | ✅ |
| `Turning_Right_90_Degrees_Anim` | `AM_EncodeRootBone` · `FootContactCurveModifier` | `contact_l/r` 만 | ❌ |
| `Rifle_Aiming_Idle_Anim` | **없음** | **없음** | ❌ |

(모디파이어 인스턴스는 `<경로>.<이름>:AnimationModifiersAssetUserData_0`의
`animationModifierInstances`로 원격 검수했다 — `../CLAUDE.md` 6.1b절)

## 2. ★ 챙터는 ABP 노드에 박혀 있다 [A]

```lisp
(fn Update_MotionMatching (Context Node)
  (bind _result (Animation|EvaluateChooser:CHT_PoseSearchDatabases self)
    (:execute
      (bind _output_get (Variables|MotionMatching|SetValidDatabases _result))
      (Animation|MotionMatching|SetDatabasesToSearch
        (Animation|MotionMatching|ConverttoMotionMatchingNode Node) _output_get
        (MotionMatching|Get_MMInterruptMode))
      (return))))
```

- 챙터 에셋이 **노드 타입에 붙어 있다**(`EvaluateChooser:CHT_PoseSearchDatabases`).
  `K2Node_EvaluateChooser`의 `Chooser` 프로퍼티라 **핀이 아니다** → `set_pin_value`로 못 바꾼다.
  `retarget_node_class`도 **클래스 참조 전용**이라 안 먹는다. → **Details 패널에서 수동 교체**
- 챙터를 담는 ABP 변수는 없다(변수 76개 전수 확인). **런타임 교체 불가**

→ **ABP를 복제해야 한다.** `../CLAUDE.md` **P1**이 지시하는 방법이기도 하다.

### 2.1 조준 축은 이미 ABP 변수로 있다 [A]

`SandboxCharacter_CMC_ABP`의 변수 76개 중:

```
RotationMode  RotationMode_LastFrame   ← E_RotationMode (OrientToMovement / Strafe / Aim)
Stance  Gait  MovementMode  MovementState  MMDatabaseLOD
ValidDatabases  CurrentSelectedDatabase  CurrentDatabaseTags
```

챙터는 `EvaluateChooser(..., self)` 로 **ABP 자신을 컨텍스트로 받으므로**,
`RotationMode`를 **컬럼으로 바로 쓸 수 있다.** 구조체를 새로 만들 필요가 없다.

---

## 3. 배선 방식 — GASP 3단 체인을 복제하지 않는다

`../animation/...` 15.3절이 "무기 자세 축은 2단에 컬럼 추가"라고 판정했지만, 그건 **최종 형태**다.
지금 필요한 것은 **판정용 최소 배선**이고, 챙터가 **복수를 반환한다**는 성질 덕에 훨씬 짧게 끝난다.

```
CHT_Soldier_Databases   (신규, 2행)
  행 1  (조건 없음)  →  PSD_Soldier_Rifle          ← 우리 견착 DB
  행 2  (조건 없음)  →  CHT_PoseSearchDatabases    ← GASP 3단 체인 전체 (중첩 챙터 결과)
```

- **실험 1 ([C-44])**: 행 2를 끄면 **견착 DB만** 후보 → "견착 데이터만으로 얼마나 버티는가"
- **실험 2 ([C-24])**: 행 2를 켜면 **견착 + GASP 총내림 합집합** → 점진적 폴백이 실제로 작동하는가

행 하나를 켜고 끄는 것이 곧 실험 전환이다. **GASP 에셋은 하나도 안 건드린다**(P2 유지).

> 나중에 데이터가 갖춰지면 15.3절 (가)안대로 2단에 `RotationMode` 컬럼을 넣어
> 정식 구조로 옮긴다. 지금 그걸 먼저 하면 **판정 전에 구조부터 짓는 셈**이 된다.

### 3.1 작업 목록 — 진행 현황 (2026-09-08)

| # | 작업 | 상태 |
|---|---|---|
| 1 | `Rifle_Aiming_Idle_Anim` ⑦Idle 세트 | ✅ EncodeRootBone → FootContact → MoveData_Speed |
| 2 | `Turning_Right_90_Degrees_Anim` ⑥제자리회전 세트 | ✅ + `UTurnInPlaceCurvesModifier` (FootSteps는 제거) |
| — | **`bLoop` 교정** (신규 발견, 3.2절) | ✅ MCP로 수정·저장 |
| 3 | 세 클립을 PSD에 편입 → Build Index | ⬜ **사용자** |
| 4 | `CHT_Soldier_Databases` | ✅ 복제 + 컨텍스트 타입 교체. **행 결과는 사용자** |
| 5 | `SoldierCharacter_ABP` | ✅ 복제 (102그래프·76변수·인터페이스 2종 승계) |
| 6 | 복제본 `Update_MotionMatching`의 EvaluateChooser → 우리 챙터 | ⬜ **사용자** (Details 패널) |
| 7 | `BP_SoldierCharacter` (자식 BP) + AnimClass 교체 | ✅ MCP로 생성·설정·컴파일·저장 |
| 8 | `NPCLevel`에 배치 + `AIC_Soldier` | ⬜ **사용자** |

만들어진 것:

```
/Game/SoldierLab/Animation/SoldierCharacter_ABP        SandboxCharacter_CMC_ABP 복제
/Game/SoldierLab/PoseSearch/CHT_Soldier_Databases      CHT_PoseSearchDatabases 복제
/Game/SoldierLab/Blueprints/BP_SoldierCharacter        SandboxCharacter_CMC 의 자식
```

> **캐릭터는 복제가 아니라 자식 BP로 만들었다.** 바꿀 것이 메시의 `AnimClass` 하나뿐이라
> 복제할 이유가 없고, 자식이면 GASP 캐릭터의 수정이 그대로 따라온다.
>
> **ABP는 자식이 될 수 없어서 복제했다** — `EvaluateChooser`가 가리키는 챙터가 노드에 박혀
> 있어서 자식에서 갈아끼울 방법이 없다(2절).
>
> 안전 확인: `SandboxCharacter_CMC`의 그래프 27개에 **ABP 클래스로의 하드 캐스트가 없다**
> (인터페이스 `BPI_SandboxCharacter_ABP`로만 대화한다). 그래서 형제 클래스인 우리 ABP를
> 물려도 깨지지 않는다. 복제본도 인터페이스 2종을 그대로 승계했다.

### 3.1b ★ 함정 — ABP가 쓰는 챙터는 **하나가 아니다** [A]

실험 1을 돌리자 PIE 로그에 매 프레임 에러가 났다:

```
LogChooser: Error: Chooser Table: CHT_CMCCharacterAnimations ContextData entry 0
  expects an object of type SandboxCharacter_CMC_ABP_C,
  but an object of type SoldierCharacter_ABP_C was passed in.
```

2절에서 `Update_MotionMatching`의 `CHT_PoseSearchDatabases`만 보고 "챙터를 갈아끼우면 된다"고
판단했는데, **`SetBlendStackAnimFromChooser`가 `CHT_CMCCharacterAnimations`를 따로 쓴다.**

> **세는 방법**: 그래프 102개를 DSL로 훑는 건 실패한다(합성 노드·순환 그래프에서 툴이 죽는다).
> **`get_dependencies`로 ABP의 참조 목록을 뜨는 것이 확실하다** — 챙터가 몇 개든 한 번에 나온다.
> 결과: **정확히 2개**. `CHT_CMCCharacterAnimations`, `CHT_Soldier_Databases`.

→ `CHT_Soldier_CharacterAnimations`로 복제하고 컨텍스트 타입을 교체했다.
   컨텍스트가 2개(ABP 클래스 + `S_ChooserOutputs` 구조체)라 **둘 다 유지한 채 첫 번째만** 바꿔야 한다.

#### 이것이 형제 복제 방식의 구조적 비용이다

우리 ABP는 GASP ABP의 **형제**(둘 다 `AnimInstance` 직속)라, GASP 에셋이 타입 검사를 하는
지점마다 걸린다. 자식이었다면 전부 통과했겠지만, `EvaluateChooser`가 노드에 박혀 있어
자식으로는 챙터를 갈아끼울 수 없다(2절).

> **남은 위험**: 의존성 목록의 `S_ChooserOutputs` · `StrafeOffsetCurveContainer` 등에도
> 같은 검사가 있을 수 있다. **로그를 계속 볼 것.**
> 또 하나 — 복제본의 의존성에 **`/Game/Blueprints/SandboxCharacter_CMC_ABP`가 남아 있다.**
> 어디서 원본을 참조하는지 미확인. 에러가 나면 여기부터 본다.

### 3.2 ★ 신규 발견 — `bLoop`이 매핑표에서 빠져 있었다 [A]

우리 세 클립이 **전부 `bLoop = false`** 였다. GASP 실측과 대조하면 틀렸다:

| GASP 클립 | `bLoop` |
|---|---|
| `M_Relaxed_Walk_Loop_F` · `M_Neutral_Walk_Loop_FL` · `M_Relaxed_Stand_Idle_Loop` | **true** |
| `M_Relaxed_Walk_Start_F` · `_Stop_B` · `_Pivot_F_B` · `M_Relaxed_Stand_Turn_090_L` · `M_Relaxed_Stand_Idle_Break_v02` | false |

**규칙: 순환하는 클립만 true.** Idle이라도 *Break*(1회성)는 false다.

왜 중요한가 — `PoseSearchAssetSampler.cpp:377`이 `IsLoopable()`을 **`SequenceBase->bLoop` 그대로**
반환하고, 그 값이 세 군데를 가른다:

| 위치 | `bLoop=false`일 때 |
|---|---|
| `PoseSearchAssetSampler.cpp:322` `WrapOrClampTime` | 클립 끝에서 **wrap 대신 clamp** → 마지막 포즈가 얼어붙은 채로 특징이 만들어진다 |
| `PoseSearchAssetIndexer.cpp:219` | `LoopingCostBias`가 안 붙는다 (루프를 선호하게 만드는 편향) |
| `AnimNode_MotionMatching.cpp:278` | `SearchResult.bLoop`이 BlendStack에 그대로 전달 → **클립이 끝나고 멈춘다** |

→ `Walking_Anim`(보행 순환)과 `Rifle_Aiming_Idle_Anim`(조준 대기 순환)을 **true로 고쳤다**.
`Turning_...`은 false 유지. **매핑표 4절에 행을 추가해야 한다.**

> 커브도 모디파이어도 아닌 **시퀀스 자체 플래그**라서 [C-34] 조사에서 통째로 빠졌다.
> `get_asset_tags`에도 안 나온다 — `list_properties`로만 보인다.

---

## 4. 클립 마무리 명세 (4절 매핑표 적용)

### 4.1 `Rifle_Aiming_Idle_Anim` — ⑦Idle·정지 포즈

| 모디파이어 | |
|---|---|
| `AM_EncodeRootBone` | ✅ (pelvis 1.0 / orientation pelvis 축 Y) |
| `FootContactCurveModifier` | ✅ |
| `AM_MoveData_Speed` | ✅ |
| `AM_FootSteps_*` / `AM_BakePhaseCurve` / `AM_WarpingAlpha` | ❌ |
| `bEnableRootMotion` | ✅ true |

⚠ **[C-38] 주의**: 이 클립은 이동이 0이라 발이 전혀 안 뜬다. 접지 판정이 **속도 단일 조건**으로
퇴화해 `contact_l/r`이 전 구간 1이 나올 것이다 — **그게 정답이다**(정지 포즈니까).
로그로 `below-Z` / `below-speed` 비율을 확인해 두면 [C-38]의 표본이 하나 더 생긴다.

### 4.2 `Turning_Right_90_Degrees_Anim` — ⑥제자리 회전

이미 `AM_EncodeRootBone` + `FootContactCurveModifier`가 붙어 있다. 추가로:

| | |
|---|---|
| `AM_FootSteps_*_Soldier` | ✅ **싱크마커 OFF** (노티파이만 — 회전 중에도 발은 딛는다) |
| `AM_MoveData_Speed` | ✅ |
| `AM_BakePhaseCurve` / `AM_WarpingAlpha` | ❌ (루프가 아니라 `phase` 무의미 / 정지계라 워핑 금지) |
| 손 저작 커브 | `steeringtargettime` = **1.0 상수** · `enable_turninplacesteering` = **1.0 → 0.0 @ 0.5s** |
| 손 저작 노티파이 | `Pose Search: Block Transition In` · `Override Continuing Pose Cost Bias` → **[C-48]** |

클립이 72프레임 / 2.400s / 30fps이므로 **0.5s = 프레임 15**.

⚠ **모디파이어 목록 순서**(P12): `EncodeRootBone` → `FootSteps` → `FootContact` → `MoveData_Speed`.
기존에 붙은 `FootContactCurveModifier` **앞에** `FootSteps`를 끼워 넣어야 한다.

---

## 4b. ★ 실험 결과 (2026-09-08)

### 실험 1 — 견착 DB만 (클립 3개)

| 관측 | |
|---|---|
| 상체 | ✅ 조준 유지 — 베이스 클립이 `Rifle_Aiming_Idle_Anim`이라 견착으로 보인다 |
| 하체 | ❌ **질질 끌린다.** 발이 정지한 채 캡슐만 이동 |
| 벤치 SmartObject | 정상 (몽타주 슬롯이라 MM DB와 무관) |

전방 walk 1 + 조준 대기 1 + 우회전 1로는 옆·뒤·출발·정지에 맞는 궤적이 없다.
**[C-44] 1차 답: 전혀 못 버틴다.**

### 실험 2 — 견착 + GASP Dense 합집합

Rewind Debugger `Blend Weights`가 결정적이었다 — **재생된 클립이 전부 `M_Neutral_*`(GASP)이고
우리 클립은 하나도 없다.** `Pose Search` 트랙에는 두 DB 간 전이 비용이 나란히 찍히므로
**배선은 정상이고 비용 경쟁에서 100% 진 것**이다.

부수 관측: `BS_Neutral_AO_Stand`(조준 오프셋)은 가중치가 걸려 있는데도 견착으로 안 보인다.
**조준 오프셋은 애디티브라 기존 포즈를 기울일 뿐, 견착 자세를 만들지 못한다.**
→ **견착 자세는 베이스 클립에서 온다.**

### ★ 왜 졌나 — `PSS_Default`는 상체를 아예 안 본다 [A]

채널 실측:

```
Trajectory                              weight 0.873   ← 매칭의 주축
Position_0   foot_l  (기준 foot_r)       weight 1.0
Velocity_1   foot_l                      weight 0.3
Velocity_2   foot_r                      weight 0.3
Heading_0    pelvis, 축 Y                weight 0.1
```

**발과 골반뿐이다. 팔·무기 자세는 비용에 들어가지 않는다.**

이것이 두 가지를 뜻한다:

1. **MM은 견착과 총내림을 구분하지 못한다.** 스스로 견착을 고를 이유가 없다.
   GASP가 이긴 것은 순전히 **발·궤적 커버리지**(3개 대 수백 개) 때문이다
2. 그러므로 **비용 편향이 필수**다. 구조 문제가 아니라 튜닝 문제이고,
   오히려 폴백 구상([C-24])에는 유리하다 — 상체가 비용에 없으니 총내림 하체를 써도 벌점이 없다

> 처음에 "상체 포즈 비용 때문에 견착 클립이 밀린다"고 추정했으나 **틀렸다.** 스키마 실측으로 뒤집혔다.

### 다음 손잡이 — `BaseCostBias`

`PoseSearchDatabase.h:518` — "Negative values make it more likely to be picked".
Epic이 `PSD_Dense_*` 35개에서 실제로 쓰는 값:

| 값 | 쓰는 곳 |
|---|---|
| **−0.5** | `Stand_Run_SpinTransition` (최강) |
| −0.3 | `Jumps_Far` |
| −0.2 | `Stand_TurnInPlace` |
| −0.1 | `*_FromTraversal` |
| 0.0 | 대부분 |
| **+0.1** | `Stand_Idles` (덜 뽑히게) |

→ 우리 PSD를 **−0.5**(Epic 최댓값)로 설정했다. 이걸로도 안 뒤집히면 격차가 Epic의 설계 범위를
   넘는다는 뜻이므로, 그 자체가 유의미한 관측이다.

## 4c. ★★ 조준 오프셋으로는 견착 자세를 만들 수 없다 [A]

`baseCostBias` 실험 결과부터:

| 값 | 결과 |
|---|---|
| 0.0 | GASP 100% (견착 0회) |
| **−0.5** | **우리 100%** (GASP 0회) |

격차가 0.5보다 작아 **한 번에 넘어간다. 이건 스위치지 믹서가 아니다.**
그리고 중간값을 찾아봐야 소용없다 — **베이스 클립이 견착 여부를 결정**하므로 섞이면 총이 오르내린다.

### 왜 AO가 견착을 못 만드나 — 저작 방식 실측

```
additiveAnimType = AAT_RotationOffsetMeshSpace     ← 회전 델타만
refPoseType      = ABPT_AnimFrame
refPoseSeq       = M_Neutral_AO_Stand_X0_Y0        ← 42개 전부 X0_Y0 기준
```

**`X0_Y0`는 자기 자신이 기준이라 델타가 0이다.** 정면 조준일 때 AO는 아무 일도 안 한다.
`CURRENT_STATE` 3.4절이 조달 요청을 "견착 정지 포즈 42개"로 바꿨는데,
**그 42개는 조준 *방향* 델타이지 견착 *자세*가 아니다.** 조달 스펙의 전제가 틀렸다.

> 수량도 정정: `BS_Neutral_AO_Stand`의 샘플은 **15개**다(X ±90/±45/0 × Y ±90/0).
> `X±135` 포즈는 에셋으로만 있고 Stand 블렌드스페이스에는 안 들어간다.

### 애디티브 우회 시도와 그 한계 [A]

기준 포즈를 총내림 중립으로 바꾼 애디티브(`AO_Rifle_Stand_Additive`)를 만들어
`BS_Rifle_AO_Stand_Test`(15샘플 전부 동일)로 넣어 봤다. **견착은 나왔지만 두 가지가 깨졌다:**

| 증상 | 원인 |
|---|---|
| 걸을 때 **견착한 팔이 걸음에 맞춰 스윙** | 애디티브는 베이스에 *더한다.* GASP walk의 팔 스윙이 남는다 |
| 정지 시 **팔이 뒤틀려 머리 쪽으로** | 델타 = (견착 − 기준). 베이스가 기준 포즈일 때만 정확하고, 다른 idle이면 어긋난다 |

**애디티브로는 원리상 정확해질 수 없다.** 상체를 *더하는* 게 아니라 *교체*해야 한다.

### ❌ 내가 제안한 `Layered blend per bone`은 **설계 위반이다** (사용자가 지적, 2026-09-08)

~~런타임 `Layered blend per bone`으로 상체를 교체한다~~ → **폐기.**

`../design/2026-09-01_architecture.md` **5.5.1절**이 명시적으로 금지한 방식이다:

> **안 되는 것 — 로코모션 중에 상반신만 다른 파지자세로 레이어/블렌드 하는 것.**
> 걷기/뛰기는 발맞춤에 동기화된 전신 움직임이라, 상반신과 하반신을 따로 블렌드하면
> **상체가 허공에 붕 뜬 채 고정되고 다리만 뛰는 이음새**가 눈에 띈다.

같은 문서 1553행: *"이 프로젝트가 이미 **실패로 확인한** 방식. 채택하지 않음."*

**내가 관측한 증상이 바로 그 실패다** — "견착한 팔이 걸음 속도에 맞게 스윙한다"는 것이
상체와 하체가 서로를 모른다는 신호인데, 그것을 애디티브의 결함으로만 읽고
**금지된 방식으로 가자고 했다.** 애디티브 실험 자체도 같은 함정의 다른 형태였다.

> `../CLAUDE.md` 3.1절의 실패 패턴이 또 나왔다 — **관련 설계 문서를 읽지 않고 제안했다.**
> 이번 것은 [B]를 [A]로 읽은 게 아니라 **아예 확인하지 않은 것**이라 더 나쁘다.

### 설계가 허용하는 경로 — A안 / B안

`../assets/2026-09-02_asset_supply_and_collaboration.md` 5절:

| 안 | 내용 | 상태 |
|---|---|---|
| **A (권장)** | 견착 DB는 축소하고 **Orientation Warping**으로 메운다. 급선회 순간엔 총내림으로 바꾸는 것이 오히려 자연스럽다(연출로 흡수) | **워핑이 실제로 켜지는지 미확인** ← 먼저 할 것 |
| **B** | GASP의 start/stop/pivot을 리타깃한 뒤 **상체만 견착으로 교체해 새 풀바디 클립으로 굽는다.** 런타임 블렌드가 아니라 **오프라인 클립 생성**이라 5.5.1절에 걸리지 않는다(문서 202-205행 명시) | 도구 제작 필요 |

### ★ 오늘 실측이 B안을 강하게 뒷받침한다

`PSS_Default`의 포즈 채널이 **전부 하체**(foot_l/foot_r/pelvis)라는 실측(4b절)은
**상체를 바꿔 구운 클립의 매칭 비용이 원본과 동일하다**는 뜻이다.
즉 GASP의 전환 데이터를 견착으로 변환해도 **MM 품질이 그대로 유지된다.**

(본문 406-409행이 09-02에 이미 같은 결론을 적어놨고, 오늘 독립적으로 재확인됐다.)

> **B안의 도구**: 엔진 `UCopyBonesModifier`는 **같은 클립 안의 본 쌍 복사**만 한다.
> **다른 클립의 상체 포즈를 가져오려면 자작 모디파이어가 필요하다** —
> `UAnimPoseExtensions::GetAnimPoseAtTime`으로 견착 포즈를 읽어 대상 클립의 상체 본 트랙을
> 덮어쓰는 방식. `Source/SoldierLabEditor/`가 이미 있으므로 파일 두 개다.

### 이것이 조달에 미치는 영향

성공하면 **하체는 GASP 총내림 그대로 쓰고(0원) 상체 포즈만 사면 된다.**
`CURRENT_STATE` 3.4절의 "42 정지 포즈"는 이렇게 갈라진다:

| 항목 | 수량 | 성격 |
|---|---|---|
| 견착 상체 포즈 | Stand/Crouch 각 1 | `Layered blend per bone`의 BlendPose. **이게 견착 자세를 만든다** |
| 견착 기준 조준 AO | 15 × 2 자세 | `AAT_RotationOffsetMeshSpace`, 기준 = 위의 견착 포즈 |
| 견착 로코모션 | **0?** | 하체를 GASP로 쓸 수 있으면 안 사도 된다 ← **이번 테스트가 결정한다** |

## 4d. ★★ A안이 막혔던 두 가지 — 둘 다 데이터가 아니라 배선이었다 (2026-09-08)

MCP로 AnimGraph를 읽을 수 있다는 걸 알게 되면서(`../CLAUDE.md` 6.1절 정정) 실측이 가능해졌다.

### (1) `ik_foot_*`가 비어 있으면 Orientation Warping이 무력화된다 — [C-46]

`AnimGraphNode_MotionMatching_0.AnimationBlendStackGraph_0` 안의 워핑 노드 실측:

```
OrientationWarping_1
  iKFootRootBone = ik_foot_root
  iKFootBones    = ik_foot_l, ik_foot_r      ★ 워핑은 이 본들을 회전시킨다
  spineBones     = spine_01 ~ spine_05       ★ 그 회전을 여기로 테이퍼 분배
  rotationAxis   = Z,  maxCorrectionDegrees = 180
  minRootMotionSpeedThreshold = 10
  warpingSpace   = RootBoneTransform
  alphaCurveName = None,  alpha = 1
```

설계 5.5.2절이 인용한 동작 원리 그대로다. **우리 파이프라인엔 `AM_Copy_IKFootRoot`가 없었으므로
워핑이 회전시킬 대상이 없었다.** 실험 1의 "다리 질질"을 데이터 부족으로 읽었던 것이 오독이다.
→ 매핑표에 단계 추가 필요.

> 같은 그래프에 **Stride Warping은 없다** ([C-22] 답). GASP는 속도별 클립이 많아 필요가 없다.

### (2) 클립 속도와 AI 이동 속도가 2배 어긋나 있었다

```
SandboxCharacter_CMC.WalkSpeeds = (200, 180, 150)   전방 200 cm/s
Walking_Anim                    = 99.9 cm/s          ← 정확히 절반
MotionMatching 노드 playRate    = { 0.85, 1.15 }     ← ±15%만 허용
```

**2배가 필요한데 1.15배로 잘린다** → 발이 이동을 못 따라가 종종거린다("갓 걸음마").
GASP는 속도별 클립이 많아 이 문제가 안 생기고, 그래서 Stride Warping도 안 쓴다.

→ **자식 BP(`BP_SoldierCharacter`)의 `WalkSpeeds`를 (100, 90, 75)로** 낮췄다.
   GASP의 방향별 비율(1 / 0.9 / 0.75)은 유지. **GASP 원본은 안 건드린다.**
   설계 A안이 *"조준 이동은 실제로 **느리고** 전환이 적다"* 를 전제하므로 방향도 맞다.

### 남은 손잡이 (아직 안 건드림)

| 항목 | 현재값 | 의미 |
|---|---|---|
| `playRate` | {0.85, 1.15} | 넓히면 속도차를 재생속도로 흡수. 너무 넓으면 부자연스럽다 |
| `blendTime` | 0.5 | 클립 전환 블렌드. 클립이 적으면 길어서 뭉개질 수 있다 |
| `poseReselectHistory` | 0.3 | 재선택 억제 시간 |
| `maxActiveBlends` | 4 | 동시에 섞이는 클립 수. 3클립뿐인데 4면 항상 전부 섞인다 |

## 5. 판정 기준 (P0-2)

| 실험 | 무엇을 보나 | 항목 |
|---|---|---|
| 1 | 견착 DB만으로 옆·뒤·급선회에서 **얼마나 무너지는가** | **[C-44]** |
| 2 | GASP 총내림을 합쳤을 때 **자연히 메워지는가**, 상체는 조준 오프셋이 덮는가 | **[C-24]** |

**이 둘의 차이가 곧 조달 규모다** — 폴백으로 메워지는 만큼은 안 사도 된다.

## 6. 미리 잡아둘 교란 변수

| | |
|---|---|
| **[C-48]** | 회전 클립에 PoseSearch 노티파이가 없으면 MM이 회전 중간으로 튀어 들어간다. **없는 채로 측정하면 "데이터 부족" 으로 오독한다** → 4.2절에서 미리 넣는다 |
| **[C-44] 전제** | [C-1]은 8방향이 완비된 GASP 비무장 DB로 통과했다. 실험 1은 정반대 조건이다 |
| 회전 클립 출처 | `Turning_Right_90_Degrees_Anim`이 **견착인지 미확인**. 비견착이어도 하체는 쓸 수 있지만 기록해 둘 것 |
