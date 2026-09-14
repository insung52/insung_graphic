# 구현 현황 — 실제로 만들어져 있는 것

> ★ **2026-09-14 — `titan_example` 편입 완료.** 아래 경로는 전부 `/Game/...` 그대로지만 **프로젝트가 바뀌었다**:
> `C:\working\kadex\titan_example`. C++ 는 `Source/SoldierLab/` · `Source/SoldierLabEditor/` 모듈 2개.
> 이관 중 생긴 변경(타입 개명 · cvar 개명 · `AC_VisualOverrideManager` 편집)은
> `migration/2026-09-14_titan_example_migration.md` 3.1 · 3.4 · 4.3 참고.

2026-09-14 / 유지보수 / **애니메이션 층(L4) 완료 · 무기/투사체 배선 완료 · AI 층 동작 확인 · 아군 메시(soldier_T) 교체 완료.**
AI가 GASP 몸을 실제로 운전한다 — 인지·시야·무전·제압·교전·엄폐·**목표**. **L0 명령 · L1 분대는 여전히 초안만.**
★ **2026-09-14 교정 라운드**: 노출의 사다리(조리개·사격자세) · 반동 · 조준 선회 · `FightingCost` · **목표 액터의 루트 컴포넌트 버그** → `ai/2026-09-14_exposure_ladder_and_corrections.md`
⚠ **수비수가 정착해 쓰지 못하는 문제는 여전히 미해결이다** → **[C-95]**.

---

> **이 문서의 역할**: `CURRENT_STATE.md`가 "지금 뭘 할 차례인가"라면, 이 문서는
> **"지금 무엇이 존재하는가"** 다. 새 세션이 코드/에셋을 건드리기 전에 읽는다.
>
> 신뢰도 표기는 `CLAUDE.md` 3.1절 규칙을 따른다 — **[A]확정 / [B]잠정 / [C]미측정**.
> 여기 적힌 [A] 항목은 2026-09-11에 **에디터 실물을 읽어서** 확인했다.

---

## 0. 한 장 요약

| 층 | 상태 | 실체 |
|---|---|---|
| **L4 모션** | ✅ **동작 확인** | GASP ABP 복제 + Lyra 라이플 세트 이식. 이동·조준·견착/총내림·사격·재장전·왼손 IK + **연속 자세 축 4종**(총구 정렬·린·블라인드 파이어·**stance**) |
| **무기 · 투사체** | ✅ **동작 확인** (2026-09-12) | `ASoldierProjectile` 이식(포물선 탄도·도탄·재질별 명중·총알 휘파람) + `BP_AR4Rifle` + 캐릭터 배선. 3.1절 |
| **AI 층 (인지~교전)** | ✅ **동작 확인** (2026-09-13) | C++ **9쌍** `Source/SoldierLab/AI/`. 진영·기록/감쇠/융합·시야·무전·제압·교전·엄폐·**목표** + 디버그 규약. 5절 ★ **09-14 교정 라운드**: 조리개/사격자세 · 반동 · 조준 선회 · `FightingCost`. `ai/2026-09-14_exposure_ladder_and_corrections.md` |
| L3 실행 | ⚠ **형태가 바뀜** | StateTree가 아니라 **컴포넌트 + Tick 접합**이 한다. `AIC_Soldier`의 `StartLogic`은 **삭제**됐다 → [C-43] |
| L2 판단 | ⚠ **얇은 판이 돈다** | 유틸리티 스코어러는 없다. 교전/엄폐가 각자 자기 질문을 푼다. `ai/drafts/`는 **여전히 미반영** |
| L1 분대 | ⬜ 초안만 | `squad/drafts/` |
| L0 명령 | ⬜ 초안만 | `squad/drafts/` 의 명령 스키마. **목표·임무 개념은 [D10]으로 생겼다** — 다만 `ASoldierObjective` 는 **레벨 마커 한 개**일 뿐 명령도 국면 전환도 아니다 → [W25] ★ **09-14에 목표가 실제로 돌기 시작했다** — 다만 여전히 **마커 하나 · 소유권 변경 없음** |
| 엄폐 | ✅ **동작 확인** (2026-09-13) | **시야 판정을 거꾸로 돌린 것.** 볼륨·마커·베이크 없음. `cover/drafts/`의 EQS/SmartObject 초안은 **미채택** |
| **목표(objective)** | ✅ **이제야 실제로 돌기 시작했다** (2026-09-14) | `ASoldierObjective` + 세 비용 스코어러. **레벨에 0개 배치**라 셋째 항이 항상 0 → **[W25]** · `ai/2026-09-13_objective_and_position_cost.md` ★ **09-14**: 레벨에 `Objective_AllyBase` **1개 배치**. 그런데 **놓기만 했으면 여전히 안 돌았다** — 루트 컴포넌트가 없어 `GetActorLocation` 이 **영원히 월드 원점**을 답하고 있었다(**P90**). 반경·밴드·환율도 전부 바뀜다 → **[C-94]** · `ai/2026-09-14_exposure_ladder_and_corrections.md` 6절 |

**플레이어가 WASD로 직접 조작해 검증 가능하고, `GM_SoldierObserver`로 관전하면 AI끼리 싸운다.**
**단 병사는 죽지 않는다** — 데미지·체력·사망이 없다 → [W18].

---

## 1. 대상 프로젝트 [A]

```
경로       C:\working\kadex\anim_test\SoldierLab
엔진       UE 5.8
베이스     GASP (Game Animation Sample) — 원본 캐릭터/ABP를 지우지 않고 병존 (P2)
클립 출처   Lyra Starter Game   C:\working\kadex\anim_test\LyraStarterGame
```

문서는 이 폴더(`titan/soldier_ai_lab/`)에, 코드·에셋은 언리얼 프로젝트에 있다.

---

## 2. 애니메이션 층 (L4)

### 2.1 클립 자산 [A]

```
/Game/SoldierLab/Animations/Rifle/           130개
    Loops/       Walk · Jog · Crouch_Walk × Fwd/Bwd/Left/Right      (12)
    Starts/ Stops/ Pivots/                                          (36)
    TurnInPlace/ Turn L/R × 90/180 × Stand/Crouch                   (8)
    Idles/       Idle_ADS · Idle_Hipfire · IdleBreak ×2 · Crouch_Idle  (5)
    Poses/       조준 오프셋 포즈 15장
    AO_Rifle_Aim  AimOffset — Yaw −180..180 × Pitch −90..90, 샘플 15
    _Extra/ _MF/  미사용 보관

/Game/SoldierLab/Animations/Actions/          95개
    HitReact 13 (Front/Back/Left/Right × Lgt/Med/Hvy) · Death 6 (4방향)
    Dash · Equip/Unequip · Melee · GrenadeToss 등
    → 배선된 것은 사격/재장전뿐이다. 나머지는 반입만 돼 있다
```

**리타깃**: `RTG_Lyra_to_UEFN` (`IK_LyraManny` → GASP의 `IK_UEFN_Mannequin`).
루트 facing 손실 없음이 실측으로 확인됐다 → `animation/prototypes/2026-09-09_lyra_rifle_migration.md` 3절.

**커브**: 폴더별 모디파이어 스택 A~D로 일괄 생성. 어떤 클립에 어떤 커브를 거는지는
`animation/prototypes/2026-09-04_c34_clip_curve_mapping.md` 4절이 **유일한 기준**이다 (P8).

### 2.2 Pose Search [A]

```
/Game/SoldierLab/PoseSearch/Rifle/
    PSD_Rifle_Stand_{Idles, Idles_LowReady, TurnInPlace}
    PSD_Rifle_Stand_Walk_{Loops, Starts, Stops, Pivots}
    PSD_Rifle_Stand_Jog_{Loops, Starts, Stops, Pivots}
    PSD_Rifle_Crouch_{Idles, TurnInPlace}
    PSD_Rifle_Crouch_Walk_{Loops, Starts, Stops, Pivots}      = 17개
    PSN_Rifle_All                                              정규화 세트 (멤버 17)
```

- 스키마는 GASP 원본 `PSS_Default` / `PSS_Idle`을 그대로 쓴다
- **정규화 세트는 우리 것을 따로 만들었다** — Epic 것을 가리키면 GASP 25개 DB의
  척도로 우리 클립을 재게 된다 (P19)
- 검색 모드 `PCAKDTree`

### 2.3 Chooser [A]

```
/Game/SoldierLab/PoseSearch/CHT_Soldier_Databases      4열 × 17행
    열: Stance · MovementState · Gait · RotationMode    출력: PoseSearchDatabase
    (_BAK 은 이전 MMDatabaseLOD 구성 백업)

    행 1    Stand·Idle·Any·RotationMode  =    Strafe   →  Stand_Idles (Idle_ADS 1개)
    행 17   Stand·Idle·Any·RotationMode  Not  Strafe   →  Stand_Idles_LowReady
    행 2    Stand·Idle·Any·RotationMode  Any           →  Stand_TurnInPlace
    나머지                               Any
```

중첩 표를 쓰지 않는다. **통과하는 행이 전부 기여하므로**(`Chooser.cpp:712-713`)
DB 하나당 행 하나를 두고 행끼리 상호배타가 되게 짰다. Walk / ≠Walk 분기는
`MatchNotEqual`로 처리해 Run·Sprint를 함께 받는다. → 같은 문서 8절, `CLAUDE.md` 6.1d

### 2.4 `SoldierCharacter_ABP` — 애님 그래프 [A]

`/Game/SoldierLab/Animation/SoldierCharacter_ABP` — GASP `SandboxCharacter_CMC_ABP` 복제본.

```
MotionMatching_0 / TwoWayBlend
 → BlendListByInt_0 → ApplyMeshSpaceAdditive_2   (Lean 뱅킹 ← BlendSpacePlayer_0
                                                   [BS1D_Additive_Lean_Run])
   |
   |  IdentityPose_2 → Slot 'FullBodyAdditivePreAim' → ApplyMeshSpaceAdditive_1   (사격 반동)
   |
   |  BlendListByBool_0 → DeadBlending_0 → ApplyMeshSpaceAdditive_0 . Additive   (조준)
   |      pose0 (true)  : BlendSpacePlayer_1 = AO_Rifle_ADS   견착  (Enable_AO)
   |      pose1 (false) : LayeredBoneBlend_0                  총내림(로우레디)
   v
[ BF_L ] → [ BF_R ] → [ BF_U ]          ★ 2026-09-12 추가 — 블라인드 파이어 3레이어 (2.5e)
   v
SaveCachedPose 'AimedPose'
   |-- UseCachedPose_0 → LayeredBoneBlend_1 . BasePose
   +-- UseCachedPose_1 → Slot 'UpperBody' → LayeredBoneBlend_1 . BlendPoses_0
                           blendMode = BranchFilter · branchFilters = [spine_01, depth 1]
                           curveBlendOption = UseMaxValue
LayeredBoneBlend_1 → ApplyAdditive_1 ← IdentityPose_3 → Slot 'UpperBodyAdditive'
   v
Slot 'DefaultSlot' → OffsetRootBone_0 → RemapCurves_0 → LocalToComponentSpace_0
   → ModifyBone spine_01..05        ← **린(lean) 축** ⚠ 이 문서에 아직 절이 없다 → [W7]
   → FootPlacement_0
   → ModifyBone pelvis (Z += PelvisDrop, BMM_Additive, BCS_ComponentSpace)
                                    ← ★ 2026-09-12 추가 — **stance 축** (2.5f)
   → LegIK_1 → TwoBoneIK_0 (hand_l) → ComponentToLocalSpace_2
   → PoseSearchHistoryCollector_0 → Root
```

> ★ **`FootPlacement_0`과 `LegIK_1` *사이*라는 위치가 stance 축의 전부다** [A].
> `FootPlacement_0`은 `pelvisBone = pelvis`로 **골반을 스프링으로 소유**하므로 그 **앞**에
> 골반 오프셋을 넣으면 되돌려진다. 반면 `LegIK_1`은 이미 심어진 `ik_foot_l/r`로 발을
> **되돌리므로**, 사이에서 골반만 내리면 **무릎이 굽는다** = 웅크림. → 2.5f-2

> ⚠ **정정 (2026-09-12 실측)**: 이 체인에 **`ModifyBone spine_01..05`(린 축)가 빠져 있었다.**
> 린 축은 구현돼 동작 중인데 이 문서에 절이 없다 → **[W7]**(소급 문서화).
>
> ⚠ **`BlendSpacePlayer_1`(AO 블렌드스페이스)은 독립된 전체 포즈가 아니라
> `ApplyMeshSpaceAdditive_0`의 *애디티브 입력*이다** — `BlendListByBool_0` → `DeadBlending_0`을
> 거쳐 들어간다 [A]. (이 문서와 `animation/2026-09-02_gasp_abp_analysis.md` 76행은 원래
> 맞게 적혀 있었다. 2026-09-12 세션 중간에 이를 전체 포즈로 읽은 판단이 있었고 그것이
> 틀린 것이다 → `animation/prototypes/2026-09-12_blind_fire_axis.md` 9.6절)
>
> **`TwoBoneIK_0`(hand_l)이 그래프의 마지막 스켈레탈 컨트롤 노드**라는 점이
> 블라인드 파이어가 왼손 IK를 다시 짤 필요가 없었던 이유다 (2.5e).

**분기 지점에 Cache Pose를 반드시 둔다.** 상태를 가진 상류 체인(MM·BlendStack)을
두 갈래로 그냥 뽑으면 **매 프레임 두 번 평가돼 이동이 2배속이 된다.** 2026-09-10에 실제로 겪었다.

`MotionMatching_0` 내부 블렌드스택 서브그래프에는 GASP 원본 그대로
`OrientationWarping` · `StrideWarping` · `Steering` ×2 · `ResetRoot`가 들어 있다.
**전 프로퍼티를 GASP과 diff해 동일함을 확인했다**(2026-09-11) — 이 노드들은 건드리지 않았다.

**GASP에서 바꾼 애님 노드는 두 개뿐이다** [A] (2026-09-11 MCP 실측):

| 노드 | 프로퍼티 | GASP | 우리 | 왜 |
|---|---|---|---|---|
| `OffsetRootBone_0` | **`maxRotationError`** | **−1**(상한 없음) | **90** | 상한이 없으면 급선회 시 메시가 캡슐에서 무제한으로 벌어지고 **그 벌어짐이 그대로 AimOffset 입력**이라 포즈가 뒤집힌다. `maxTranslationError`에는 30이 있는데 회전만 무제한이었다 |
| `BlendListByBool_0` | `BlendTime_0` / `BlendTime_1` **(핀)** | 0.1 / 0.1 | ~~0.75 / 1.50~~ → **0.375 / 0.25** | AO 문턱 근처의 채터링을 블렌드로 누른다 |

> ⚠ **정정 (2026-09-12 실측)**: `BlendTime`은 ~~0.75 / 1.50~~ 이 아니라 **0.375 / 0.25** 다.
> 그 사이에 값이 바뀐 것을 문서가 따라가지 못했다. 이 **0.375초**(AO를 켜는 쪽)는
> 조준 보정 게인의 회복 시간(0.5초)을 정하는 기준값이기도 하다 → 2.5d절 ·
> `animation/prototypes/2026-09-12_body_yaw_rate_and_aim_antiwindup.md` 11.2절.

> ⚠⚠ **`BlendListByBool`은 불리언 방향이 뒤집혀 있다 — `true`가 0번 입력이다**
> (`AnimNode_BlendListByBool.cpp`: *"Intentionally flipped boolean sense"*).
> 따라서 **AO를 켤 때 0.375초 · 끌 때 0.25초**다.
> 그리고 이 값은 **노드 프로퍼티가 아니라 핀에 있다** — `node.blendTime`은 `[0.1, 0.1]`로
> GASP 원본 그대로다. 핀이 이긴다 (**P25**).
>
> ⚠ **`maxRotationError`와 `Enable_AO` 문턱은 묶여 있다** → 2.5b절 끝.
>
> ⚠ `BlendListByBool_0`의 나머지 설정은 GASP 원본 그대로다 —
> `transitionType = Inertialization` · `blendType = Custom` ·
> `customBlendCurve = /Game/Characters/UEFN_Mannequin/Animations/AimOffset/AO_Blend_Curve` ·
> `blendProfile = SK_UEFN_Mannequin:FastHead_Weight` [A].
> **`AO_Blend_Curve`의 키 값은 아직 아무도 안 봤다** — MCP로는 읽을 수 없다.
> 최댓값이 1.0을 넘으면 **독립적인 오버슈트 원인**이다 → **[R6]**.

> 상세 → `animation/prototypes/2026-09-11_sharp_turn_while_aiming.md`

> ⚠ MCP로 애님 노드를 고친 뒤에는 **반드시 `compile_blueprint`** 를 부른다. 저장은 컴파일이 아니다 (P23).
> ⚠ 핀으로 노출된 프로퍼티는 **핀 리터럴이 `node.<prop>`를 이긴다** (P25).

### 2.5 왼손 IK [A]

```
TwoBoneIK_0   iKBone         = hand_l
              effectorTarget = ~~본 weapon_r~~ → **소켓 `weapon_r`** (bUseSocket = true, BCS_BoneSpace)   ← 2026-09-13
              jointTarget    = lowerarm_l      (BCS_BoneSpace)
              EffectorLocation 핀     ← 변수 LeftHandGripOffset    현재 (−30, 8, 4)
              JointTargetLocation 핀  = **(−1, 1, 0)**   ← 2026-09-13. 0 이면 "FK 팔꿈치 자리를 폴로" — 아군에서 팔꿈치가 흉부를 뚫었다
              Alpha 핀         ← Get Curve Value("DisableLHandIK")
              alphaInputType = Float · alphaScaleBiasClamp  scale −1 / bias +1  (반전)
```

> ★ **2026-09-13 — effector 가 본에서 소켓으로 바뀌었다** [A]. 아군 메시 `soldier_T` 에 `weapon_r` 본이 없어서다.
> 소켓 이름은 본과 같은 `weapon_r` 이고 **양쪽 메시에 각각 있다** — 마네킹 `SKM_UEFN_Mannequin` 은 `weapon_r` **본에**
> 항등으로(본과 결과 동일, 애니메이션 따라감), `soldier_T` 는 `hand_r` 에 마네킹 본의 레스트 오프셋
> `(0.19749, 3.411153, −0.381067)` 로. 소켓 모드는 소켓이 없는 메시에서 `LogAnimation: socket doesn't exist` 와 함께
> **조용히 꺼지므로**, 새 메시를 붙일 때마다 소켓 추가가 필수다 → 3.2절.

- 부착 대상은 **`weapon_r`** 다. 가상본 `hand_r_prop_01_Socket`은 안 된다 —
  `props_root` 밑이라 애니메이션이 안 들어가 원점에 붙어 있다
- 끄고 켜는 판정은 코드가 아니라 **클립의 `DisableLHandIK` 커브**가 한다 (P27)
- `AlphaInputType = Curve`는 이 프로젝트에서 **동작하지 않는다.**
  `Get Curve Value` 노드를 Alpha 핀에 직결해야 한다 (P28)

### 2.5b 조준 자세 — 견착/총내림을 **의도로** 가른다 [A]

AimOffset은 조준 *방향* 델타만 더하는 회전 애디티브다. **고개를 숙인 자세는 베이스 클립에 있다.**
그래서 베이스를 의도로 골라야 한다 — 안 그러면 MM이 포즈 유사도로 골라 절반은 틀린다.

```
AO_Rifle_ADS      15장 ← MM_Rifle_Idle_ADS_AO_*          베이스 MM_Rifle_Idle_ADS
AO_Rifle_Crouch   15장 ← MM_Rifle_Crouch_Idle_AO_*        베이스 MM_Rifle_Crouch_Idle   ⬜ 미배선
AO_Rifle_Aim      (구) Hipfire 포즈로 만들어져 있던 것. 보존만
```

- 조준 판정은 **`RotationMode == Strafe`** 다. `Enable_AO`가 테스트하는 값이고,
  `E_RotationMode` 바이트 순서는 `0=OrientToMovement · 1=Strafe · 2=Aim`
- 애디티브 갈래(`BlendListByBool_0`)는 **`Enable_AO`** 가 가른다 — ~~의도 **AND 각도 한계**(Idle 180 / 이동 115)~~ → **정정: 아래 2.5b-1절**
- 베이스 클립 갈래(Chooser)는 **의도만** 본다. `Enable_AO`를 쓰면 몸을 크게 틀 때
  베이스가 Hipfire로 튀어 고개가 벌떡 든다

> ⚠ **`Get_MMInterruptMode`에 `RotationMode != 직전` 항을 추가했다**(2026-09-11).
> 없으면 조준을 풀어도 MM이 재검색하지 않아 **ADS 클립이 몇 초씩 남는다.**
> `OR_1`의 네 번째 입력(A=MovementState, B=Gait·Moving, C=Stance, D=RotationMode).

#### 2.5b-1 `Enable_AO`의 실제 구조와 문턱 [A] (2026-09-11 정정)

`read_graph_dsl`이 **항을 하나 빠뜨렸다**(6.1f와 같은 종류). 배선을 직접 추적한 실제 구조는
**3항 AND**다:

```
Enable_AO =      | Get_AOValue.X |  ≤  문턱(MovementState)      ← X(yaw)만 본다. Y는 미연결
            AND  RotationMode == NewEnumerator1                (= Strafe / 견착 의도)
            AND  GetSlotLocalWeight("DefaultSlot") < 0.5       ← ★ DSL이 빠뜨린 항 (몽타주 중 AO 차단)
```

**문턱** — `MovementState`에 대한 `Select`:

| `MovementState` | 뜻 | GASP 원본 | **우리 값** |
|---|---|---|---|
| `NewEnumerator4` | **Idle** | 115 | **70** |
| `NewEnumerator0` | **Moving** | 180 | **70** |

> ⚠ **정정**: 위 2.5b절 본문에 **"Idle 180 / 이동 115"** 로 적혀 있었으나 **거꾸로다.**
> 열거자 대응은 `Update_States`가 정한다 — `IsMoving` **true → `NewEnumerator0`**(Moving),
> **false → `NewEnumerator4`**(Idle) [A]. GASP 원본은 **Idle 115 / Moving 180**이다.

**70으로 낮춘 이유**: 급선회로 조준각이 70°를 넘으면 AO를 꺼 **총을 내리고**, 몸이 따라잡아
각이 줄면 다시 올린다. 정지/이동을 가를 이유가 없어 같은 값으로 뒀다. PIE 확인 완료 [A].

#### 2.5b-2 ⚠⚠ `maxRotationError` ↔ `Enable_AO` 문턱은 **결합돼 있다** [A]

```
maxRotationError = 90     ← 천장.  AimOffset 입력이 여기까지만 커진다 (2.4절)
Enable_AO 문턱   = 70     ← 그 안에서 "총을 내릴 결정선"
```

- **문턱이 천장 이상이면 영원히 발동하지 않는다.** `maxRotationError = 90`이 `(Aim − Root)`를
  90°로 막고, 여기에 총구 보정(±25°)만 더해지므로 `|Get_AOValue.X|`의 실질 상한은 **약 115°** [B]
- 즉 **GASP 원본의 115 / 180은 `maxRotationError = 90` 아래에서는 사실상 "AO를 끄지 않는다"** [B]
- **한쪽만 바꾸면 다른 쪽의 의미가 조용히 달라진다. 항상 쌍으로 적고 쌍으로 바꾼다**
- 두 값 모두 **눈으로 정한 값이지 계측한 값이 아니다** → **[C-74]**

### 2.5c 총구 정렬 되먹임 보정 [A] (2026-09-11)

AimOffset은 **근사**다. `AO_Rifle_ADS`는 5(Yaw) × 3(Pitch) = 15장이고 **pitch 표본이 3장뿐**이라
총구가 조준 방향에서 벗어나며, 벗어나는 양이 **조준 각도에 따라 변한다**(정면 정지 실측
pitch −11.4° · yaw +10.1°). 그래서 상수 오프셋이 아니라 **닫힌 되먹임 루프**로 잡는다.

```
BP_SoldierCharacter (매 Tick, AI 병사 포함)
    ERR   = Delta(Rotator)( GetControlRotation , MakeRotFromX(총구 +X 전방벡터) )
    GATE  = max( |Delta(GetControlRotation, PrevAimRot).Pitch| ,
                 |Delta(GetControlRotation, PrevAimRot).Yaw|   )  ≤  2.0    ← 카메라 각속도
    GAIN  = SelectFloat( 0.0 , Lerp(0.05, 0.0, WeaponLowered) , bPickA = NOT AOActive )
                                                               ↑ ★ 2026-09-12 추가: 안티 와인드업
    누적  = SelectRotator( A = CombineRotators(이전 누적, Lerp(Rotator)((0,0,0), ERR, GAIN)) ,
                           B = 이전 누적 ,                    ← 게이트 거짓이면 **그대로 유지**
                           bPickA = GATE )
           → NormalizeAxis → Clamp(±25°) → MakeRotator(Roll = 0)
           → AimCorrection (캐릭터)  →  Cast to SoldierCharacter_ABP  →  AimCorrection (ABP)
    PrevAimRot ← GetControlRotation                              (틱 끝)

SoldierCharacter_ABP · Get_AOValue
    Delta( AimingRotation , Delta( RootRotation , AimCorrection ) )  ==  (Aim − Root) + Corr
```

> ⚠⚠ **정정 (2026-09-12) — 위의 `GetControlRotation`은 이제 `GetController()`를 거치지 않는다** [A].
> 원래 배선은 **`GetControlRotation( GetController() )`** 였고, `GetController()`가 null이면
> 로그에 `Accessed None trying to read CallFunc_GetController_ReturnValue`가 찍히며
> **조준 방향이 `(0,0,0)`으로 읽혔다.** **AI에게 치명적이다** — 총구 정렬(2.5c)과 무기 내리기(2.5d)는
> 애초에 **45명의 AI 병사를 위한 기능**인데 입력이 쓰레기값이면 `BodyErr`가 통째로 무의미해진다.
> **두 호출부(`UpdateBodyYawRate` · Tick의 조준 루프)를 GASP 자신의 패턴으로 교체했다**:
> ```
> SelectRotator( A = GetControlRotation[Pawn] , B = GetBaseAimRotation ,
>                bPickA = IsLocallyControlled )
> ```
> 죽은 노드 3개를 같이 제거했다.
> ⚠ **함정**: `GetControlRotation`은 **`APawn`과 `AController` 양쪽에 존재**한다. DSL 라이터가
> Controller 쪽에 붙여 *"This blueprint (self) is not a Controller…"* 로 블루프린트를 깨뜨렸다 —
> **`create_node` + `declaring_class = /Script/Engine.Pawn`** 으로 만들어야 한다
> (`CLAUDE.md` 6.1f-3). 상세 → `animation/prototypes/2026-09-12_continuous_stance_axis.md` 10절.
> ⚠ 이것은 **[C-75](AI의 `PrevAimRot` 동결)와 별개 문제**다. [C-75]는 그대로 열려 있다.

| 값 | 왜 |
|---|---|
| **게이트 = 카메라 각속도 ≤ 2.0°/프레임** | **입력**으로 건다. 오차(=자기 출력)로 걸면 **자기 잠금**이 생긴다 (**P35**) |
| **게이트 거짓 → 유지**(감쇠 아님) | 0으로 감쇠시키면 카메라를 움직이는 동안 보정이 사라져 총구가 다시 벌어진다 |
| **게인 0.05** (이력 0.5 → 0.25 → 0.05) | **게인이 곧 학습 시간상수.** 0.05 ≈ 20프레임 ≈ 0.33초로, `OffsetRootBone`이 정착하는 시간에 맞췄다. 0.25는 몸의 추적 지연까지 학습해 **오버슛**했다 (**P35**) |
| **게인 게이트 = `AOActive`** (2026-09-12) | **AO가 꺼져 있으면 보정은 포즈에 닿지 않는데 적분기는 계속 돈다** = 열린 루프. ERR 20°·게인 0.05면 **0.42초에 ±25 클램프에 도달**한다. 액추에이터(`Enable_AO`)가 결합돼 있을 때만 적분한다 (**P37**) |
| **게인 회복 0.5초 > 포즈 블렌드 0.375초** | 다시 들 때 **포즈가 먼저 서고 게인이 나중에 붙는다.** 반대면 블렌드 구간에서 다시 포화한다 (**P38**) |
| **Clamp ±25°** | 안티 와인드업. ⚠ 오차가 25°를 넘으면 **클램프에 두 번째 안정 평형점**이 생긴다 → **[C-76]** (**P40**) |

- ~~**게인 0.25** (0.5는 링잉)~~ → 위 표 참고 · **Clamp ±25°** 는 안티 와인드업
- `Delta(A, B)`는 `NormalizedDeltaRotator` = **성분별 뺄셈**이므로 위 중첩이 곧 성분별 덧셈이 된다.
  `CombineRotators`(합성)로 같은 것을 하려다 **월드 −X에서 부호가 뒤집혀 발산**했다 → **P32**
- `AimingRotation`은 ABP 전체(노드 1,200개)에서 **`Get_AOValue` 한 곳에서만 읽힌다**(2026-09-11 전수 검색).
  그래서 편향을 넣어도 몸통 facing·궤적이 흔들리지 않는다
- 결과: **월드 −X를 포함한 전 방향에서 오차 ≈ 0**

> 상세 · 실패한 접근 4종 + **실패한 게이트 설계 3종**은
> **`animation/prototypes/2026-09-11_muzzle_aim_alignment.md`**(9절 + **13절 정정**).
> 안티 와인드업 게이트와 그 **또 다른 실패 3종**은
> **`animation/prototypes/2026-09-12_body_yaw_rate_and_aim_antiwindup.md`** 10·11절.
> ⚠ 이 루프와 함께 들어간 **계측 장치가 아직 Tick에 물려 있다** → 6절.

> ⚠⚠ **`SetPrevAimRot`이 `IsLocallyControlled` Branch 안에 있다** [A]. `AimCorrection` 자체는
> 분기 **앞**에서 갱신되므로 AI도 보정을 받지만, **`PrevAimRot`은 AI에게 영원히 초기값**이라
> 속도 게이트가 계속 닫혀 **AI의 누적 보정이 얼어붙을 수 있다** [B] → **[C-75]**.

### 2.5d 유한 몸통 각속도 — 무기 자세가 회전 속도를 정한다 [A] (2026-09-12)

GASP 부모는 **접지 중 `RotationRate = (0, −1, 0)`** 을 매 프레임 쓴다. **음수는 "즉시"** 라
캡슐이 컨트롤러를 한 프레임에 따라잡았고, 그래서 **"총을 내리면 빨리 돈다"는 결정에
아무 물리적 근거가 없었다.** 그것을 유한 각속도로 바꾼 것이 이 절이다.

```
BP_SoldierCharacter . UpdateBodyYawRate()        ← 새 함수 그래프. Event Tick **끝**에서 호출

  BodyErr       = | Delta(Rotator)( GetControlRotation , GetActorRotation ).Yaw |
  WpnTgt        = SelectFloat( 1.0 ,
                               MapRangeClamped( BodyErr ,
                                                WeaponLowerAngle , WeaponLowerAngleFull , 0 , 1 ) ,
                               bPickA = NOT AOActive )
  WeaponLowered = Max( RampAxisTo(현재, WpnTgt, WeaponLowerRate, dt) ,    ← 내릴 때 0.125s
                       RampAxisTo(현재, WpnTgt, WeaponRaiseRate, dt) )    ← 올릴 때 0.5s
  ω             = SelectFloat( 200 , Lerp(YawRate_Up, YawRate_Down, WeaponLowered) ,
                               bPickA = CharacterMovement.IsFalling )
  CharacterMovement.SetRotationRate( MakeRotator( Roll=0 , Pitch=0 , Yaw=ω ) )
```

- ⚠ **`GetControlRotation`은 2026-09-12부터 `SelectRotator(GetControlRotation[Pawn],
  GetBaseAimRotation, IsLocallyControlled)`** 다 — `GetController()` 경유를 걷어냈다. 2.5c의 정정 참고
- `Max(빠른 램프, 느린 램프)`는 **비대칭 속도를 비교 노드 없이** 얻는 요령이다 —
  내릴 때는 빠른 쪽이 더 크고, 올릴 때는 빠른 쪽이 더 작다
- `RampAxisTo`는 **기존 C++** `USoldierAxisLibrary`(4절). 빌드 불필요

#### 2.5d-1 ⚠⚠ `BeginPlay`의 틱 선행 조건이 **이 기능의 전제**다 [A]

`AC_PreCMCTick` 컴포넌트가 부모의 `UpdateRotation_PreCMC`를 매 프레임 구동해 `−1`을 다시 쓴다.
**순서를 고정하지 않으면 우리 쓰기는 조용히 무시된다.** `Parent: BeginPlay` 직후에:

```
self.AddTickPrerequisiteComponent( GetComponentByClass(AC_PreCMCTick) )
CharacterMovement.AddTickPrerequisiteActor( self )
                      →  AC_PreCMCTick  →  우리 Tick  →  CMC
```

> **`BodyErr == 0.000`(정확히 0)은 `RotationRate.Yaw = −1`이 살아 있다는 서명이다.**
> 0이 아닌 값이 나와야 유한 각속도가 먹고 있는 것이다. **육안으로는 판별되지 않는다** —
> 애니메이션은 자기 속도로 계속 돌기 때문이다 (**P39**).

#### 2.5d-2 튜닝값 [A]

| 변수 | 값 | 왜 |
|---|---|---|
| `YawRate_Up` | **90** °/s | 총을 든 상태 — 90°에 1초. "들고는 빨리 못 돈다"가 체감된다 |
| `YawRate_Down` | **720** °/s | 총을 내린 상태 — 90°에 0.125초. 사실상 제약 없음 |
| `WeaponLowerAngle` | **30** ° | `BodyErr`가 넘으면 내리기 시작 |
| `WeaponLowerAngleFull` | **65** ° | 완전히 내려감. **`Enable_AO` 문턱 70보다 약간 아래** — 포즈가 바뀌기 직전에 각속도가 먼저 풀린다 |
| `WeaponLowerRate` | **8.0** (0.125 s) | 내리는 쪽은 빠르게 |
| `WeaponRaiseRate` | **2.0** (0.5 s) | 올리는 쪽은 느리게. **AO 블렌드 0.375초보다 느려야** 게인이 포즈보다 늦게 회복한다 (2.5c) |

> ⚠⚠ **`WeaponLowerAngleFull 65` 가 [C-74]의 결합 쌍에 세 번째로 합류했다** —
> `maxRotationError 90`(천장) / `Enable_AO 70`(결정선) / `WeaponLowerAngleFull 65`(각속도 해제선).
> **셋을 함께 본다.**
>
> ⚠ `WeaponLowered`(캡슐 기준 연속값)와 `Enable_AO`(메시 기준 불리언)는 **다른 신호다.**
> 조준 보정 게이트에 `WeaponLowered`를 썼다가 실패했다 →
> `animation/prototypes/2026-09-12_body_yaw_rate_and_aim_antiwindup.md` 10.2절.

> 상세 → `animation/prototypes/2026-09-12_body_yaw_rate_and_aim_antiwindup.md`

### 2.5e 블라인드 파이어 축 — 저작 포즈를 연속 애디티브로 얹는다 [A] (2026-09-12)

총구 조준 정렬(2.5c) · 린에 이은 **세 번째 연속 자세 축**이다. 사용자 확인 완료 [A].

```
BlindFireH   −1(좌) ~ +1(우)          BlindFireV   0 ~ +1(상)
키  1 = 좌   2 = 위   3 = 우   4 = 리셋
누르고 있으면 등속 램프 · **떼면 그 값에서 멈춘다**(린 축과 같은 규약)
```

#### 2.5e-1 ★ 왜 IK가 아니라 저작 포즈인가 [A]

처음 권고안은 **순수 IK**(무기 트랜스폼을 오프셋하고 팔 IK가 풀게 하는 것)였다. **기각됐다.**

| | |
|---|---|
| 사용자가 **특정 자세(타르코프류)** 를 이미 그리고 있었다 | "팔이 닿는다"가 아니라 **정해진 실루엣**이 필요했다 |
| 그 자세는 **몸통/척추가 돌아간다** | **팔 IK로는 만들 수 없다** |

**팔 IK 체인이 척추로 전파되지 않는 것은 결함이 아니라 설계다** [A] — 표준 바이페드 리그에서
예외 없이 `upperarm → lowerarm → hand` 세 마디다. → **저작 포즈 + 연속 애디티브**를 채택하고
**IK는 왼손을 그립에 붙잡아 두는 역할로 축소**했다.

#### 2.5e-2 에셋 [A]

세 장 전부 `SK_UEFN_Mannequin`. 베이크 결과는 **151키 / 150프레임 / 5.0초**이고
**쓰는 것은 프레임 0 하나뿐**이다.

```
/Game/SoldierLab/Animations/Rifle/Poses/MM_Rifle_BlindFire_L
/Game/SoldierLab/Animations/Rifle/Poses/MM_Rifle_BlindFire_R
/Game/SoldierLab/Animations/Rifle/Poses/MM_Rifle_BlindFire_U

애디티브 변환 (3장 동일)
    additiveAnimType = AAT_RotationOffsetMeshSpace
    refPoseType      = ABPT_AnimFrame
    refPoseSeq       = /Game/SoldierLab/Animations/Rifle/Idles/MM_Rifle_Idle_ADS
    refFrameIndex    = 0
```

- **척추/몸통과 머리까지** 저작돼 있다(2.5e-1의 이유가 그대로 반영된 것)
- **왼손이 총에 붙어 있는 상태**로 저작됐다
- ⚠ `ABPT_AnimFrame`은 **다른 에셋을 참조하는** 기준 포즈다 — 리타깃하면 끊기고 **소스까지 바뀐다** (**P20**)

> **저작 시작 포즈는 정확성에 영향이 없다** [A]. 메시 스페이스 애디티브는 `(저작 − 기준)`이고
> 기준 위에 얹으면 저작 포즈가 정확히 복원된다. A-포즈에서 저작해도 결과는 같고,
> 조준 아이들에서 출발하는 이득은 **작업량뿐**이다. 이 확인이 필요했던 이유는
> 작동하는 저작 경로(`CR_Mannequin_Body`, 비레이어드)가 **`Bake To Control Rig` 없이는
> 조준 포즈를 물려받을 수 없기** 때문이다 → `CLAUDE.md` 6.3절.

#### 2.5e-3 애님 그래프 — 마스크 애디티브 3레이어 [A]

`ApplyMeshSpaceAdditive_0`(조준 적용)와 `SaveCachedPose_0 'AimedPose'` **사이**에 직렬로 끼운다.

```
ApplyMeshSpaceAdditive_0 → [BF_L] → [BF_R] → [BF_U] → SaveCachedPose 'AimedPose' → (하류 그대로)

레이어 한 벌 (3벌 동일)
    AdditiveIdentityPose ───────────────────────→ LayeredBoneBlend . BasePose
    SequenceEvaluator(MM_Rifle_BlindFire_X,
                      ExplicitTime = 0)  ───────→ LayeredBoneBlend . BlendPoses_0
                                                   BlendWeights_0 = 1.0
    LayeredBoneBlend ───────────────────────────→ ApplyMeshSpaceAdditive . Additive
    ApplyMeshSpaceAdditive . Alpha  ←  ABP 변수  BF_AlphaL / BF_AlphaR / BF_AlphaU

마스크
    blendMode  = BranchFilter
    layerSetup = [ { branchFilters: [ { boneName: "spine_01", blendDepth: 1 } ] } ]
```

- 베이스가 **애디티브 항등 포즈**이므로 마스크 밖의 본은 **델타 0** — `LayeredBoneBlend`는
  순전히 "애디티브를 상체로 마스킹"하는 용도다
- `spine_01` 브랜치는 **척추·쇄골·양팔·목·머리**를 덮고 **골반과 다리를 제외**한다 →
  **로코모션이 전혀 건드려지지 않는다**
- 정지 포즈이므로 **`SequenceEvaluator`(시간 지정) + `ExplicitTime = 0`** 을 쓴다. `Player`가 아니다

> ★ **왼손 IK 재작업이 필요 없었다** [A]. `TwoBoneIK_0`(`hand_l`)이 **그래프의 마지막
> 스켈레탈 컨트롤 노드**(`FootPlacement_0 → LegIK_1` 뒤)라, 새 레이어가 몸통을 아무리 틀어도
> **마지막에 왼손을 그립에 다시 앉힌다.**

> ⚠⚠ **마스크 규약이 이제 두 벌이다** [A]. 기존 `LayeredBoneBlend_0`(로우레디)은
> **`blendMode = BlendMask`** + 블렌드 마스크 에셋 **`SK_UEFN_Mannequin:BM_LowReady`** 를 쓰고,
> 새 세 개는 **`BranchFilter`** 를 쓴다. 둘 다 동작하지만 다음 사람이 혼동한다 → **[W8]**.

**새 노드 ID**: `AnimGraphNode_IdentityPose_4/5/6` · `AnimGraphNode_SequenceEvaluator_0/1/2` ·
`AnimGraphNode_LayeredBoneBlend_2/3/4` · `AnimGraphNode_ApplyMeshSpaceAdditive_3/4/5` ·
게터 `K2Node_VariableGet_3/4/5`.

#### 2.5e-4 캐릭터 — `UpdateBlindFire()` [A]

새 함수 그래프. **Event Tick 끝**에서 `UpdateBodyYawRate` **다음**에 부른다.

```
reset      = InRange( BlindFireInputReset , 0.5 , 2.0 )

BlindFireH = SelectFloat( RampAxisTo( BlindFireH , 0 , BlindFireResetRate , dt ) ,
                          StepAxis ( BlindFireH , BlindFireInputH , BlindFireRate , dt , −1 , 1 ) ,
                          reset )
BlindFireV = SelectFloat( RampAxisTo( BlindFireV , 0 , BlindFireResetRate , dt ) ,
                          StepAxis ( BlindFireV , BlindFireInputV , BlindFireRate , dt ,  0 , 1 ) ,
                          reset )

ABP.BF_AlphaL = MapRangeClamped( BlindFireH , 0 , −1 , 0 , 1 )
ABP.BF_AlphaR = MapRangeClamped( BlindFireH , 0 , +1 , 0 , 1 )
ABP.BF_AlphaU = MapRangeClamped( BlindFireV , 0 , +1 , 0 , 1 )

HUD 행  "BF_H"  "BF_V"
```

- ★ **`MapRangeClamped`를 반파 정류기로 쓴다** — 입력 범위의 **방향만 뒤집으면** 음의 반파가
  클램프에 잘린다. 부호 있는 축 하나를 좌/우 두 알파로 가르는 데 **비교·분기 노드가 필요 없다.**
  블루프린트 팔레트에 산술 노드가 없기 때문에 쓰는 요령이다 (**P33**, 다른 곳의 `Lerp` = 곱셈과 같은 계열)
- `StepAxis` / `RampAxisTo`는 **기존 C++** `USoldierAxisLibrary`(4절). **빌드 불필요.**
  등속 램프라 **입력이 멈춰도 값이 붕괴하지 않는다** — "떼면 유지" 규약이 여기서 나온다

#### 2.5e-5 변수와 튜닝값 [A]

| 변수 | 위치 | 값 / 비고 |
|---|---|---|
| `BlindFireH` · `BlindFireV` | 캐릭터 | 축 상태 |
| `BlindFireInputH` · `BlindFireInputV` · `BlindFireInputReset` | 캐릭터 | 입력 액션이 써 넣는 값 |
| **`BlindFireRate`** | 캐릭터 · **Instance Editable** | **1.0** — 0→1 에 1초. 자세를 잡는 것은 **의도적**이어야 한다 |
| **`BlindFireResetRate`** | 캐릭터 · **Instance Editable** | **3.0** — 0까지 약 **0.33초**. 푸는 것은 **즉각적**이어야 한다 |
| `BF_AlphaL` · `BF_AlphaR` · `BF_AlphaU` | **ABP** | 각 레이어의 `ApplyMeshSpaceAdditive.Alpha` |

> 잡기 1.0 / 풀기 3.0 의 **비대칭**은 2.5d의 `WeaponLowerRate 8.0` / `WeaponRaiseRate 2.0`과
> 같은 사고방식이다.

#### 2.5e-6 입력 [A]

```
IA_BlindFireH · IA_BlindFireV · IA_BlindFireReset      셋 다 Axis1D
```

**셋 다 `IA_Lean`을 복제해서 만들었다.** ① `AssetTools`에 **에셋 생성 함수가 없어** 복제가
유일한 수단이고, ② `IA_Lean`은 **트리거가 이미 비워져 있어** 앞서 기록한
**`InputTriggerPressed` 1프레임 버그**를 자동으로 피한다.

```
이벤트 배선 (IA_Lean 과 동일)
    Triggered  →  Set <해당 Input 변수> = ActionValue
    Completed  →  Set <해당 Input 변수> = 0

IMC_Sandbox 매핑
    키 1 → IA_BlindFireH  (**Negate**)      키 3 → IA_BlindFireH
    키 2 → IA_BlindFireV                    키 4 → IA_BlindFireReset
```

> ⚠ **키 매핑은 에디터 수작업이다.** `ObjectTools.get_properties`가 `IMC_Sandbox`의
> `mappings`를 **매핑이 있는데도 `[]`로 반환**한다 — 이 배열이 API로 직렬화되지 않는다.

> ⚠ **미측정 [B]**: BF 레이어는 조준 애디티브 **위**에 얹히므로 총구가 조준선에서 크게 벗어난다.
> `AimCorrection` 적분기는 **총구 전방벡터**로 오차를 재므로 **BF 자세를 "고쳐야 할 오차"로
> 학습**할 수 있다(→ **P40**의 두 번째 평형점 · [C-76]과 같은 기구) → **[C-78]**.

> 상세 · **포즈 저작의 막다른 길 4종** · **리그 IK/FK 모드 변수 발견** →
> `animation/prototypes/2026-09-12_blind_fire_axis.md` (저작 경로 자체는 `CLAUDE.md` 6.3절)

### 2.5f 연속 stance 축 — 네 번째(마지막) 연속 축 [A] (2026-09-12)

총구 정렬(2.5c) · 린 · 블라인드 파이어(2.5e)에 이은 **네 번째 연속 자세 축**이다.
사용자가 **단계마다** 확인했다 [A].

```
StanceAxis  0(기립) ~ 1(웅크림)      V = 내리기 · B = 올리기(Negate)
누르고 있으면 등속 램프 · **떼면 그 값에서 멈춘다**(린·블라인드 파이어와 같은 규약)
```

> **키가 C가 아닌 이유**: `C`는 기존 앉기 토글(`IA_Crouch`)이 이미 쓰고 있었다. 사용자가 V/B를 골랐다.

#### 2.5f-1 ★ "stance"는 **네 하위 시스템의 묶음 이름**이다 [A]

무엇을 연속화해야 하는지부터 갈랐다. **성질이 달라 처방도 다르다.**

```
① 캡슐 높이        86 ↔ 60 (half-height, 반지름 30)    ⬜ **아직 이진** → [W9]
② 눈/카메라 높이    100 ↔ 32                            ⬜ 미착수    → [W10]
③ MM 데이터베이스   기립 클립셋 ↔ 웅크림 클립셋          ✅ 이산(설계) — 문턱에서 전환
④ AO 자산          AO_Rifle_ADS ↔ AO_Rifle_Crouch      ✅ 이진 Select — 연속 블렌드는 → [W11]
```

**실측** [A]: `capsuleHalfHeight 86` / `crouchedHalfHeight 60` / `baseEyeHeight 100` /
`crouchedEyeHeight 32` / 메시 컴포넌트 relative Z **−88** / `bCrouchMaintainsBaseLocation` **원래 false**.

**클립 재고가 설계를 정했다** [A]:

```
기립     Walk + Jog          웅크림   Walk **만** (웅크려 달리는 클립이 없다)
웅크림 PSD 6개가 기립 세트를 미러링   중간 높이 클립은 **없다**
```

- 웅크려 달릴 **클립이 없으므로** 속도 상한은 선택이 아니라 제약이다 → 2.5f-3
- 중간 높이는 **클립이 아니라 변형**으로만 만들 수 있다 → 2.5f-2
- ★ **`walkSpeeds`와 `crouchSpeeds`가 둘 다 291.31**이라 **속도는 이미 자세와 분리돼 있었다**
  (P30의 실측 교정 결과) — 결합 하나가 공짜로 풀려 있었다

#### 2.5f-2 골반 오프셋 + LegIK — 높이의 연속성 [A]

```
FootPlacement_0  →  ModifyBone(pelvis)  →  LegIK_1
                        bone            = pelvis
                        Translation    ← MakeVector(0, 0, PelvisDrop)   ← ABP float
                        translationMode  = BMM_Additive
                        translationSpace = BCS_ComponentSpace
```

**`FootPlacement_0`이 골반을 소유한다**(`pelvisBone = pelvis`, `maxOffset 250`,
`linearStiffness 100`, `pelvisHeightMode AllLegs`, interpolation on). 그 **앞**에 오프셋을 넣으면
스프링이 되돌린다. **뒤의 `LegIK_1`은 `foot_l/r`을 이미 심어진 `ik_foot_l/r`로 되돌리므로**,
**사이**에서 골반만 내리면 **무릎이 굽는다** — 그것이 곧 웅크림이다.

- 사용자 확인: 기립 포즈 기준 **−50이 사용 한계**("스쿼트하는 정도")
- ⚠ 기존 설계 문서 두 곳이 이 축을 **"`FootPlacement` 앞 / Control Rig"** 로 적고 있었다.
  **둘 다 틀렸다** — 정정 주석을 달았다(`animation/2026-09-02_pose_pipeline_spec.md` 6.2절 ·
  `animation/2026-09-02_gasp_abp_analysis.md` 10절)

#### 2.5f-3 속도 상한 **과 Gait 클램프** — 둘 다 필요하다 [A]

```
cap = Lerp( runSpeeds.x , walkSpeeds.x ,
            MapRangeClamped( StanceAxis , 0 , StanceThreshold , 0 , 1 ) )
CharacterMovement.MaxWalkSpeed         = Min( 부모가 쓴 값 , cap )
CharacterMovement.MaxWalkSpeedCrouched = Min( 부모가 쓴 값 , cap )

Stance ≥ StanceThreshold  →  SetGait( NewEnumerator0 = Walk )     ← 부모의 SetGait 를 덮어쓴다
```

- **`Min`이라 우리 쓰기는 낮추기만 한다.** 부모(`UpdateMovement_PreCMC`)는 매 PreCMC 틱에
  **둘 다** 다시 쓰고, 우리 Tick이 그 뒤에 도는 것은 **2.5d-1의 틱 선행 조건** 덕이다
- ★★ **속도만 묶으면 안 된다** [A]. `GetDesiredGait`는 **입력 크기와 `CanSprint`로** Gait를 정하고
  **실제 속도를 보지 않는다.** 그래서 `Gait`가 Run에 남고 → 예측 궤적이 **582 기준**으로 만들어지고
  → 맞는 Jog Loop가 없어 → **`Jog_Stop`이 매 걸음 재선택**되며 떨었다.
  **뒷대각선 속도 불일치(P30·P31)와 서명이 정확히 같다.** → **P46**

#### 2.5f-4 DB 전환 [A]

```
StanceAxis ≥ StanceThreshold  →  Crouch()   (매 Tick)
StanceAxis <  StanceThreshold  →  UnCrouch() (매 Tick)
      → IsCrouching() → ABP Stance 열거형 → Chooser → Crouch 계열 PSD
```

**GASP의 DB 구동 선택 경로를 그대로 유지한다** — 사용자가 명시적으로 요구했다
(*"끄면 GASP의 기능을 거의 안 쓰는 것"*).

> ⚠ **결과: 기존 `IA_Crouch` 토글이 무력해졌다** [A]. stance 축이 `bIsCrouched`를 **매 프레임
> 소유**하므로 토글의 결과가 다음 프레임에 덮어써진다 → **[W13]**

#### 2.5f-5 ★★ 골반 높이 **역산** — 문턱 팝을 없앤 것 [A]

오프셋을 직접 정하지 않는다. **원하는 월드 높이를 정하고 매 프레임 역산한다.**

```
PelvTgt = StanceTargetHeight  ( ActorZ , MeshRelZ , StanceStandZ , StanceCrouchZ , StanceAxis )
PelvOff = SolveBoneHeightOffset( PelvTgt , PelvWZ(실측) , PelvOff(직전 프레임) )   → ABP PelvisDrop
```

두 함수는 **새로 추가한 C++**(4절):

```cpp
SolveBoneHeightOffset(Target, Measured, Applied)  =  Target - (Measured - Applied)
StanceTargetHeight(ActorZ, MeshRelZ, StandZ, CrouchZ, Stance)
                                                  =  (ActorZ + MeshRelZ)
                                                     + Lerp(StandZ, CrouchZ, Stance)
```

- **`− Applied` 한 항이 핵심이다.** 실측값에는 **직전 프레임에 우리가 넣은 오프셋이 들어 있어서**,
  그걸 빼야 **애니메이션 자신이 만든 높이**가 복원된다
- ★ **되먹임 루프가 아니다** [A]. 전개하면 `offset_n = Target_n − clipHeight_{n−1}` 로
  이전 오프셋이 **소거된다** — **1프레임 지연의 피드포워드 해**이지 진동할 극점이 없다.
  **그래서 블렌드 길이도 블렌드 커브도 알 필요가 없다**(그것이 실패의 원인이었다 → 아래)
- ⚠ **왜 C++인가**: **두 runtime float의 뺄셈**이 필요한데 블루프린트 팔레트에 산술 노드가 없고
  (**P33**), 지금까지 쓴 우회(`Lerp`=곱셈, `MapRangeClamped`=반파 정류기, `Delta(Rotator)`=회전 전용)로는
  표현되지 않는다. `USoldierAxisLibrary`는 **이미 존재하는 클래스**라 P13(에디터 닫고 빌드)에 안 걸린다
- `StanceDropMax` · `StanceRaiseMax` · `StanceBlendRate`는 **실패한 접근의 잔해로 이제 안 쓰인다** → **[W6]**

> ★★ **이 자리에 도달하기까지 같은 증상을 네 번 잘못 진단했다** — ① 전환 클립 재생
> (`MM_Rifle_Crouch_Entry/Exit`는 **참조 0건**이라 재생 경로가 없다) ② 메시 26유닛 점프
> (`ActorZ + MeshZ`가 기립·웅크림 **양쪽 다 0.284**, 세 값이 같은 프레임에 바뀐다 — 엔진이 이미 상쇄)
> ③ `bCrouchMaintainsBaseLocation = false` (**`true`로 바꿔도 아무것도 안 변했다**)
> ④ **실제 원인** — 우리 오프셋은 **즉시**, 클립 교체는 **DeadBlending으로 블렌드**. 계단 대 곡선.
> **틀린 셋이 전부 "정착값을 읽고 과도구간을 추론"한 것**이었다 → **P44**.
> 그리고 그 사이에 **앞서 두 번 통한 램프 패턴을 반사적으로 재적용**했다 → **P45**.
> 전문: `animation/prototypes/2026-09-12_continuous_stance_axis.md` 8절

#### 2.5f-6 `bCrouchMaintainsBaseLocation = true` [A]

`BeginPlay`에서 **`Class|CharacterMovementComponent|SetCrouchMaintainsBaseLocation`** 노드로 켠다.

- ⚠ **`ObjectTools.set_properties`로는 못 쓴다** — **비트필드(`uint8:1`)** 라서다(`CLAUDE.md` 6.1).
  세터 노드를 쓰면 **변경이 그래프에 보인다**는 이점도 있다
- ⚠⚠ **정직하게 기록한다: 이 설정은 그것을 위해 넣은 증상(문턱 팝)을 고치지 못했다** [A].
  HUD의 `CrouchMB` 행으로 런타임 값이 `true`인 것까지 확인했는데 증상이 그대로였다.
  **그 자체로는 옳은 설정이라 남겨 두었을 뿐이다** — 이것을 해결책으로 읽지 말 것

#### 2.5f-7 변수 · 튜닝값 · 계측 [A]

| 변수 | 위치 | 값 / 비고 |
|---|---|---|
| `StanceAxis` | 캐릭터 | 축 상태 0..1 |
| `StanceInput` | 캐릭터 | `IA_Stance`가 매 프레임 써 넣는다 |
| **`StanceRate`** | 캐릭터 · **Instance Editable** | **1.0** — 0→1 에 1초 |
| **`StanceThreshold`** | 캐릭터 · **Instance Editable** | **0.5** — **DB 전환점이자 속도 상한의 무릎.** 0.35에서 올렸다(2.5f-8) |
| **`StanceStandZ`** | 캐릭터 · **Instance Editable** | **89.7** — 기립 클립의 **메시 루트 위 골반 높이** |
| **`StanceCrouchZ`** | 캐릭터 · **Instance Editable** | **39.4** — 웅크림 클립의 같은 값 |
| `PelvisDrop` | **ABP** | `ModifyBone(pelvis)`의 Z 오프셋. `UpdateStance()`가 매 Tick 써 넣는다 |
| ~~`StanceDropMax` · `StanceRaiseMax` · `StanceBlendRate`~~ | 캐릭터 | **죽은 변수** → [W6] |

**함수**: `UpdateStance()` — **Event Tick 끝**, `UpdateBlindFire` **다음**에 호출.
**입력**: `IA_Stance`(Axis1D, **`IA_Lean` 복제**) — `IMC_Sandbox`에 **V**(내리기) · **B**(올리기, **Negate**).
**HUD 행 7개 신설**: `Stance` · `PelvWZ` · `PelvTgt` · `PelvOff` · `Crouched` · `SpdCap` · `GaitClamp`.

#### 2.5f-8 문턱 0.35 → 0.5 로 옮겨 **움찔을 없앴다** [B]

```
조건   제자리 · **조준 안 함** · 천천히 웅크리는 중 · stance **0.35~0.5**(= 당시 문턱 직후)
증상   약 1초에 한 번 움찔하고 **몸이 실제로 돈다**. 조준/기립/이동하면 사라진다
오버레이 판정 (`a.AnimNode.MotionMatching.DebugDrawInfoVerbose 1`)
       검색 대상 DB = `PSD_Rifle_Crouch_Idles` + `PSD_Rifle_Crouch_TurnInPlace` **둘뿐**
       (즉 "LowReady idle DB가 안 걸러진다"는 가설은 **틀렸다**)
       Blend Stack 에 `MM_Rifle_Crouch_TurnLeft_90`이 **두 벌**, time 0.48 / 1.57
       → 제자리회전 클립이 **약 1.1초마다 자기를 재트리거**하고 있었다
```

**해결은 사용자의 제안이었다 — `StanceThreshold` 0.35 → 0.5.** 확인: *"움찔구간 완벽히 사라짐"*.

> **기구는 증명되지 않았다** [B]. 그럴듯한 설명은 "0.35에서 전환하면 웅크림 클립보다 골반을
> **약 33유닛** 들어올려야 하고 0.5에서는 **약 25유닛**이라 다리·발 심기에 무리가 덜 간다"인데,
> **그것이 재트리거를 멈춘 경로는 추적하지 않았다.** 재발하면 볼 레버 둘 → **[C-79]** ·
> **[C-80]**(`OffsetRootBone.maxRotationError = 90`이 90° 제자리회전의 해소를 막는 **자초한 회귀** 후보).
> ⚠ 앞선 교정은 그대로 살아 있고 원인이 **아니다** — `Crouch_TurnInPlace.baseCostBias 0` ·
> `Crouch_Idles.loopingCostBias −0.10`.

#### 2.5f-9 ★ 설계 결정 — **자세는 AI가 소유하고 속도는 결과다** [A]

"웅크린 채 달리려 하면 **(A) 속도를 묶는가, (B) 웅크림을 푸는가**" — **(A) 채택**.

```
자세 = 생존 결정(노출 실루엣)   ← 협상 불가, AI가 소유하는 **입력**
속도 = 그 결정의 결과           ← 협상 가능
```

**(B)는 이동 요청이 실루엣을 조용히 높인다.** AI가 "일어서라"고 명령한 적이 없는데 일어서 있고,
그 결정이 **어디에도 기록되지 않아 추적 불가능**해진다. 엄폐 판단이 자세 높이를 읽는 이상
이것은 **원인을 모르는 피격**이 된다. → **P37 · P38**(소유자는 하나, 부수 효과로 덮어쓰지 않는다)

**슈터 관례의 손맛은 비용 없이 살릴 수 있다**: **스프린트 *입력*이 입력 계층에서 stance를 0으로
명령**하게 하면 된다. AI는 그 명령을 내리지 않으면 그만이다 — 규칙이 아니라 **입력의 의미**로 표현된다.
→ **[Q41]**

> **기각한 대안**: 두 MM 출력(기립/웅크림 로코모션)을 블렌드하는 것 [A].
> **위상 동기 마커가 필요**하고(재료는 있다 — ABP가 `Phase_History` · `Contact_L_History` ·
> `Contact_R_History`를 든다) **캐릭터당 MM 검색이 두 번**이 된다 — 45명 @60fps에서 불가능하고,
> 위상이 어긋나면 **발이 미끄러진다**. **채택: 로코모션만 한 문턱에서 이산, 나머지는 전부 연속.**

> 상세 · **네 번의 오진 연쇄** · 툴링 →
> `animation/prototypes/2026-09-12_continuous_stance_axis.md`

### 2.5g AI 다리 — 연속 축 넷에 붙은 같은 모양 [A] (2026-09-13)

축을 만든 쪽(2.5c~2.5f)과 **AI가 그것을 모는 법**은 다른 문제다. 후자는 `BP_SoldierCharacter`에
변수 5개와 축마다 `SelectFloat` 하나로 끝났다 — **애님 그래프는 한 줄도 바뀌지 않았다**(P3).

```
SetX( SelectFloat( A = RampAxisTo(X, clamp(AITarget), Rate, dt),
                   B = <기존 플레이어 식 그대로>,
                   bPickA = AIPoseDriven ) )
```

**`Rate` 가 플레이어의 키 스텝이 쓰는 바로 그 변수**라는 것이 요점이다 (P71) — AI는
사람보다 축을 빠르게 움직일 수 없다. 그리고 **`RampAxisTo` 에는 클램프가 없어서**
목표를 `MapRangeClamped` 로 싸야 한다(플레이어 경로는 키 스텝이 클램프를 품고 있었다).

지금 AI가 실제로 미는 것은 **`AITargetStance` 하나**다. 린·블라인드 파이어는 다리만
놓여 있다 → **[W19]**. 상세: 5.2절 · `ai/2026-09-13_ai_bridge_and_scene.md` 1절.

---

### 2.6 ★ 확정된 튜닝값

여기 있는 숫자는 전부 **실측 또는 실패를 통해 정해진 값**이다. 근거 없이 바꾸지 말 것.

**이동 속도** [A] — 클립의 `MoveData_Speed` 실측값과 일치시킨 것

```
walkSpeeds    (291.31, 291.31, 291.31)
runSpeeds     (582.62, 582.62, 582.62)
crouchSpeeds  (291.31, 291.31, 291.31)
sprintSpeeds  (700, 700, 700)             ← [C-60] 아직 클램프 밖 (1.20×)
```

**Lyra 라이플 세트는 전 방향 단일 속도로 저작돼 있다**(방향 간 오차 0.001).
GASP의 방향별 비율(1:0.9:0.75)을 옮겨오면 옆 −30%·뒤 −40%가 어긋나
**Loop가 영영 선택되지 않고 Start/Stop이 매 걸음 재선택된다.**
→ P30 · P31 · `animation/prototypes/2026-09-09_lyra_rifle_migration.md` 9절

**비용 편향** [A] — Epic 값을 그대로 쓰지 않고 우리 클립 밀도로 다시 잡은 것 (P18)

| DB | 우리 값 | Epic Dense | 왜 |
|---|---|---|---|
| `Stand_Idles.baseCostBias` | **0** | +0.10 | 분리 후 클립이 **1개**(Idle_ADS)라 패널티를 받으면 못 이긴다 |
| `Stand_TurnInPlace.baseCostBias` | **0** | −0.20 | 원래 −0.05였다. 2026-09-11에 0으로. **급선회 반응이 부족한지 재확인 필요** |
| `Crouch_TurnInPlace.baseCostBias` | **0** | 0 | −0.05를 넣었다가 웅크리기 발작을 일으켰다 |
| `Crouch_Idles.loopingCostBias` | **−0.10** | −0.005 | **의도적 이탈** — GASP은 웅크리기 idle이 4개, 우리는 1개다 |

나머지 DB는 GASP Dense와 동일하다.

**조준 · 급선회** [A] — 2026-09-11. **아래 두 값은 결합돼 있다. 반드시 같이 본다** (2.5b-2절)

```
OffsetRootBone_0.maxRotationError      90      (GASP −1)   ← 천장: AimOffset 입력의 상한
Enable_AO 문턱  Idle / Moving          70 / 70 (GASP 115/180) ← 그 안의 "총을 내릴 결정선"
BlendListByBool_0  BlendTime_0 / _1    0.375 / 0.25 s (핀, GASP 0.1/0.1)
                                                ← true=0번이므로 AO 켤 때 0.375 · 끌 때 0.25

AimCorrection 게인 (Lerp Alpha)        0.05    ← 학습 시간상수 ≈ 0.33s (2.5c)
AimCorrection 게이트 (카메라 각속도)    ≤ 2.0°/frame
AimCorrection 게인 게이트              AOActive (Enable_AO)   ← 안티 와인드업 (2.5c, 2026-09-12)
AimCorrection Clamp                    ±25°

몸통 각속도  YawRate_Up / _Down        90 / 720 °/s            ← 2.5d (2026-09-12)
             WeaponLowerAngle / Full   30 / 65 °
             WeaponLowerRate / Raise   8.0 (0.125s) / 2.0 (0.5s)

블라인드 파이어  BlindFireRate         1.0   (0→1 에 1초)       ← 2.5e (2026-09-12)
                 BlindFireResetRate    3.0   (0 까지 약 0.33초)
                 BF 마스크             BranchFilter · spine_01 · blendDepth 1

stance 축        StanceRate            1.0   (0→1 에 1초)       ← 2.5f (2026-09-12)
                 StanceThreshold       0.5   DB 전환점 **이자** 속도 상한의 무릎
                                             ← 0.35였다. 0.5로 올려 웅크림 제자리회전
                                               재트리거(움찔)를 없앴다 [B] (2.5f-8)
                 StanceStandZ          89.7  기립 클립의 메시 루트 위 골반 높이
                 StanceCrouchZ         39.4  웅크림 클립의 같은 값
                 PelvisDrop 사용 한계  −50   기립 포즈 기준("스쿼트하는 정도") [A]
                 캡슐                  86 ↔ 60 **이진** (연속화 미착수 → [W9])
```

> **`StanceStandZ` / `StanceCrouchZ`는 "튜닝값"이 아니라 실측값이다** — 두 클립의 골반 높이다.
> 클립을 바꾸면 다시 재야 한다. `StanceThreshold`만이 취향의 값이고, **0.35 → 0.5 변경의
> 기구는 증명되지 않았다** [B] → [C-79].

**문턱 ≥ 천장이면 영원히 발동하지 않는다.** 90/70 둘 다 **감으로 정한 값**이다 → **[C-74]**.
**2026-09-12에 `WeaponLowerAngleFull 65`가 같은 쌍에 합류했다** — 이제 셋을 함께 본다(2.5d-2).
`YawRate_Up 90` · `YawRate_Down 720`도 계측값이 아니라 PIE에서 눈으로 정한 값이다 [B].

---

## 3. 캐릭터 · 무기 · 입력 [A]

```
/Game/SoldierLab/Blueprints/BP_SoldierCharacter    GASP SandboxCharacter_CMC 복제
    WeaponMesh = SK_Rifle @ 소켓 weapon_r
                 relLoc (−10, 0, 0) · relRot yaw 90 · animClass ABP_Weap_Rifle
    gait       = Run   ← 기본값. walkSpeeds는 걷기 입력이 있어야 쓰인다 ([C-52])
    AimCorrection  (Rotator)  총구 정렬 누적 보정값. 매 Tick 갱신 → ABP로 전달 (2.5c)
    PrevAimRot     (Rotator)  직전 프레임의 GetControlRotation. **속도 게이트의 상태**다 (2.5c)
                              ⚠ 대입이 IsLocallyControlled 분기 안에 있다 → [C-75]
    AimLoopActive  (Boolean)  ⚠ **조준 보정에는 쓰이지 않는다** — 죽은 오차 크기 게이트
                              (`SelectFloat 25/20` → `InRange`)만 구동하고, 그 결과가
                              **화면의 `GATE` 표시**로 나간다. 즉 화면의 GATE는 실제 게이트가
                              아니다. 정리 대상 → **[W6]** (프로토타입 문서 11절)

    ── 2026-09-12 추가 (2.5d) ──────────────────────────────────────────────
    AOActive       (Boolean)  ABP 의 AOActive 를 **매 Tick 캐시**한 것. 1프레임 지연.
                              ⚠ 인라인으로 읽지 않고 캐시하는 이유: `CastToSoldierCharacter_ABP`
                              노드가 Tick 안에서 `SetAimCorrection` **뒤에** 실행되므로
                              순수 체인으로 끌어오면 그 시점엔 **null**이다
    WeaponLowered  (Float)    0=견착 1=총내림. 각속도 보간 알파이자 조준 게인 감쇠 알파
    YawRate_Up / YawRate_Down / WeaponLowerAngle / WeaponLowerAngleFull /
    WeaponLowerRate / WeaponRaiseRate                 ← 전부 **Instance Editable**. 값은 2.5d-2
    함수 UpdateBodyYawRate()   Event Tick **끝**에서 호출
    BeginPlay                  `Parent: BeginPlay` 직후에 틱 선행 조건 2줄 (2.5d-1) ★ 전제

    ── 2026-09-12 추가 (2.5e · 블라인드 파이어) ──────────────────────────
    BlindFireH / BlindFireV                (Float)  축 상태. −1..+1 / 0..+1
    BlindFireInputH / BlindFireInputV /
    BlindFireInputReset                    (Float)  입력 액션이 매 프레임 써 넣는다
    BlindFireRate 1.0 / BlindFireResetRate 3.0       ← **Instance Editable**. 값은 2.5e-5
    함수 UpdateBlindFire()     Event Tick **끝** — `UpdateBodyYawRate` **다음**에 호출

    ── 2026-09-12 추가 (2.5f · 연속 stance 축) ──────────────────────────
    StanceAxis                             (Float)  축 상태 0(기립)..1(웅크림)
    StanceInput                            (Float)  IA_Stance 가 매 프레임 써 넣는다
    StanceRate 1.0 / StanceThreshold 0.5 /
    StanceStandZ 89.7 / StanceCrouchZ 39.4           ← **Instance Editable**. 값은 2.5f-7
    ~~StanceDropMax · StanceRaiseMax · StanceBlendRate~~  **죽은 변수** → [W6]
    함수 UpdateStance()        Event Tick **끝** — `UpdateBlindFire` **다음**에 호출
                               ① 축 램프 ② 속도 상한(Min) + **Gait 클램프** ③ Crouch()/UnCrouch()
                               ④ 골반 높이 역산 → ABP.PelvisDrop
    BeginPlay                  `SetCrouchMaintainsBaseLocation(true)` 추가 (2.5f-6)
                               ⚠ 비트필드라 `set_properties`로는 못 쓴다
    ⚠ **`IA_Crouch` 토글은 이제 무력하다** — stance 축이 `bIsCrouched`를 매 프레임 소유한다 → [W13]

/Game/SoldierLab/Animation/SoldierCharacter_ABP
    AimCorrection  (Rotator)  캐릭터가 매 Tick 써 넣는다. Get_AOValue 에서만 쓰인다 (2.5c)
    AOActive       (Boolean)  **`Update_Logic` 에서 `Update_States` 직후**에 `Enable_AO()` 결과를
                              대입한다 [A] (2026-09-12). 그 자리인 이유는 신선한
                              `RootTransform`/`RotationMode`를 보게 하려는 것.
                              캐릭터가 이 값을 읽어 조준 보정 게인을 끊는다 (2.5c)
    BF_AlphaL / BF_AlphaR / BF_AlphaU   (Float)   블라인드 파이어 3레이어의 애디티브 알파.
                              캐릭터의 `UpdateBlindFire()`가 매 Tick 써 넣는다 (2.5e)
    PelvisDrop     (Float)    `FootPlacement_0` 과 `LegIK_1` **사이**의 `ModifyBone(pelvis)` Z 오프셋.
                              캐릭터의 `UpdateStance()`가 **역산해서** 매 Tick 써 넣는다 (2.5f)

/Game/SoldierLab/Blueprints/GM_SoldierLab          defaultPawn = BP_SoldierCharacter
/Game/SoldierLab/Input/IA_Fire · IA_Reload         IMC_Sandbox에 매핑
/Game/SoldierLab/Input/IA_Lean                     린 축 ⚠ 이 문서에 절이 없다 → [W7]
/Game/SoldierLab/Input/IA_BlindFireH · IA_BlindFireV · IA_BlindFireReset
                                                   Axis1D. **`IA_Lean` 복제**로 생성 (2.5e-6)
                                                   키 1(Negate)·3 → H · 2 → V · 4 → Reset
/Game/SoldierLab/Input/IA_Stance                   Axis1D. **`IA_Lean` 복제** (2.5f-7)
                                                   키 **V** → 내리기 · **B**(Negate) → 올리기
                                                   ⚠ C를 못 쓴 이유: 기존 `IA_Crouch` 토글과 충돌
/Game/Weapons/Rifle/Mesh/SK_Rifle                  + LeftHandGrip 소켓
```

**EventGraph 배선**

```
── 2026-09-12 갱신 (3.1절 · 투사체 이식) ────────────────────────────────
IA_Fire   (Triggered) → Rifle.Shoot()              ← **몽타주를 직접 재생하지 않는다**
Rifle.OnWeaponFired (디스패처)
                      → PlayAnimMontage(AM_MM_Rifle_Fire)
                        ↑ 탄이 실제로 스폰·발사된 뒤에만 방송된다 (P49)
IA_Reload (Triggered) → PlayAnimMontage(AM_MM_Rifle_Reload)

~~WeaponMesh.GetAnimInstance.Montage_Play(AM_Weap_Rifle_Fire / AM_Weap_Rifle_Reload)~~
    **삭제됨** — 무기 메시를 `SK_AR4_X`로 갈아끼웠고 그쪽엔 애님 블루프린트가 없어
    `GetAnimInstance`가 항상 None이었다 (`Accessed None ... Node: Montage_Play`). P50

IA_Lean / IA_BlindFireH / IA_BlindFireV / IA_BlindFireReset / IA_Stance
                                                                ← 연속 축 입력 (공통 형태)
          (Triggered) → Set <Input 변수> = ActionValue
          (Completed) → Set <Input 변수> = 0
          ⚠ 트리거 목록은 **비워 둔다** — `InputTriggerPressed`를 걸면 1프레임만 들어온다
```

> ⚠ 몽타주의 슬롯이 서로 다른 **슬롯 그룹**에 걸쳐 있으면 몽타주가 통째로 무효가 된다.
> 재장전이 "총만 움직이고 캐릭터는 가만히" 있으면 이것이다. `LogAnimMontage`에 원인이 찍힌다 (P24).

---

### 3.1 무기 액터 · 투사체 [A] (2026-09-12)

**전문: `weapons/2026-09-12_projectile_port.md`** — 끊어낸 의존 4종, 버그 6건, 남은 것.

```
Source/SoldierLab/Weapons/SoldierProjectile.h (~560줄) / .cpp (~1050줄)
    titan_example 의 ARCWSProjectile 이식. **히트스캔이 아니라 중력 포물선 투사체**
    (리드 사격·탄착 낙차·비행 시간이 AI 층이 쓸 물리적 재료다)
    풀링 설계: LaunchFrom() / Deactivate()
    ⚠ **풀 자체는 없다** — 무기 컴포넌트가 없어 발당 SpawnActor 한다.
      LaunchFrom 은 어느 쪽이든 같으므로 풀 추가는 **쏘는 쪽의 변경**이다
    총구 섬광·발사음·반동은 **여기 없다** — 총구에서 일어나므로 무기 액터 소유

/Game/SoldierLab/Weapons/Blueprints/BP_RifleProjectile    부모 /Script/SoldierLab.SoldierProjectile
    M_RCWSRound · NS_Rifle_Tracer · NS_Rifle_Dirt + MS_hit_rifle_dirt
    NS_Blood + MS_hit_rifle_enemy · MI_Blood(적 + 지면 혈흔)  · MS_bullet_whizz
    surfaceImpactEffects **5행** — 행마다 이펙트 + 사운드 + M_Decal_Bullet + MS_Ricochet
    ⚠ 마이그레이션본은 **부모가 /Script/titan_example.RCWSProjectile 이라 12.8KB 껍데기**였다
      (컴포넌트·그래프 전부 소실). **C++ 이식 후 0부터 다시 만들었다** — 순서는 C++ 먼저다

/Game/SoldierLab/Weapons/Blueprints/BP_AR4Rifle           부모 /Script/Engine.Actor (순수 BP)
    Shoot: Branch(CanShoot?) → Branch(탄약) → 총구 사운드 → 총구 섬광 NS
           → CanShoot?=false → shotsFiredCount++ → SpawnActor BP_RifleProjectile
           → SoldierProjectile::LaunchFrom → ammoInMag-- → **CallOnWeaponFired**
    탄창이 비면 자동 StartReload · SetTimerByEvent(fireRate) 가 CanShoot? 를 재장전
    기본값  fireRate 0.12 · muzzleVelocity 80000 · tracerInterval 3
            magSize 30 · bulletSpreadDegrees 3          → 전부 이식값. **[C-82]**

BP_SoldierCharacter · BeginPlay 추가분
    라이플 스폰 → WeaponMesh 에 부착(SnapToTarget)
    → 구 GASP 무기 메시 **컴포넌트 단위** SetVisibility(false, propagate=false)   ← P48
    → SetActorEnableCollision(Rifle, false)                                        ← P48
    → Rifle->OwningCharacter · Rifle 변수 보관
    → OnReloadStarted / OnWeaponFired 바인드
```

**이식하며 끊어낸 의존 4종** — 넷 다 호출 지점에 복구 방법이 주석으로 남아 있다 → **[W17]**

| 원본 | 대체 |
|---|---|
| `UDetectableTargetComponent::Faction` | `bHitEnemy = OtherActor && OtherActor->IsA<ACharacter>()`. 틀려도 **이펙트를 잘못 고르는 것뿐** → **[R7]** |
| `ReportHitToInstigator` (3분기 Multicast) | `PlayImpactEffect(InstigatorActor, ...)` 직접 호출 — 세 핸들러가 전부 여기로 되돌아왔다 |
| `ReportRicochetToInstigator` (3분기 Multicast) | `World->SpawnActor<ASoldierProjectile>(...)` + `LaunchFrom(...)` |
| `AWindSource::GetWindVectorCmsForNiagara` | `SetVectorParameter(FName("WindVectorCms"), FVector::ZeroVector)` |

**살아남은 기능**: 포물선 탄도 · 풀링 배관 · 트레이서 · 도탄(바운스 캡 3) ·
재질별 명중 세트(Wood/Hard/Dirt/Metal/Glass) · 데칼 FIFO 상한(200) · 지면 혈흔 ·
명중 화염 · 카메라 셰이크 · **총알 휘파람**.

> ★ **총알 휘파람이 AI 제압 신호의 자리다.** `Tick()`의 휘즈 블록이
> `ClosestPointOnSegment`로 **이번 프레임 이동 선분과 청취자의 최근접 거리**를 구한다 —
> 제압 신호가 필요로 하는 기하 그 자체다. **지금은 로컬 카메라 하나만 보고 소리만 낸다.**
> 주변 병사로 확장하는 것이 예정된 훅 → **[W16]**.
> 값: `WhizDetectionRadiusCm 200` · `WhizBroadPhaseRadiusCm 1500`(광역 컬링) [A]

**`SoldierLab.Build.cs`** — `PublicDependencyModuleNames` 에 **`Niagara`** 추가.
에셋 참조 때문이 아니라 **투사체가 나이아가라 `User.` 파라미터를 직접 써서** 링크 타임에 필요하다.

**`Config/DefaultEngine.ini`** [A]

```ini
[/Script/Engine.PhysicsSettings]
+PhysicalSurfaces=(Type=SurfaceType1,Name="Wood")    ; ⚠ **선언 순서가 의미를 갖는다**
+PhysicalSurfaces=(Type=SurfaceType2,Name="Hard")    ;   코드는 이름으로 찾지만
+PhysicalSurfaces=(Type=SurfaceType3,Name="Dirt")    ;   마이그레이션한 피지컬 머티리얼
+PhysicalSurfaces=(Type=SurfaceType4,Name="Metal")   ;   .uasset 은 **인덱스**를 저장한다.
+PhysicalSurfaces=(Type=SurfaceType5,Name="Glass")   ;   재정렬·삽입 = 조용한 오작동

[/Script/Engine.CollisionProfile]
+DefaultChannelResponses=(Channel=ECC_GameTraceChannel4,DefaultResponse=ECR_Block,bTraceType=True,...,Name="Cover")
+DefaultChannelResponses=(Channel=ECC_GameTraceChannel5,DefaultResponse=ECR_Block,bTraceType=True,...,Name="Sight")
;   한 채널이 아니라 두 채널인 것이 설계다 — 철망·유리는 총알을 막고 시야는 통과시킨다([Q21]).
;   **지금 파는 이유**: 나중에 추가하면 이미 배치된 에셋 전부의 응답을 손봐야 한다
```

> ⚠ 이 파일은 Perforce 읽기전용 플래그를 **사용자 1회 허가로** 지우고 편집했다.
> **`DefaultEngine.ini.bak` 백업 존재 · P4V 체크아웃 미처리** → **[W15]**

> ⚠ **이식 전 예고("두 프로젝트의 콜리전 채널 인덱스가 충돌한다")는 틀렸다.**
> `titan_example`은 커스텀 채널을 하나도 쓰지 않는다. 기존 채널 1~3
> (`Traversable`/`Mouse`/`Obstacle`)은 전부 GASP 것이고 우리는 그 뒤에 이어 붙였다 → **P52**

---

### 3.2 캐릭터 메시 — 아군 `soldier_T` 를 마네킹 스켈레톤에 [A] (2026-09-13)

**전문: `animation/prototypes/2026-09-13_ally_mesh_on_mannequin_skeleton.md`** — 실측·엔진 근거·기각한 경로·원복한 시도.

원칙: **메시를 `SK_UEFN_Mannequin` 에 맞춘다.** PSD/스키마/애니메이션은 한 벌이고 어느 것도 바뀌지 않았다.

```
/Game/SoldierLab/Characters/Ally/soldier_T          스켈레톤 = SK_UEFN_Mannequin   ← Assign Skeleton (FBX 재임포트 아님)
    87본 (마네킹 91본 중 attach · weapon_l/r · props_root · prop_01 · poi 6본이 없고, thigh_twist_02_l/r 2본이 더 있다)
    바인드 T-포즈 · 키 181cm (마네킹 A-포즈 · 165.6cm) — 같은 스켈레톤 에셋이면 바인드 포즈 차이는 무관하다
    메시 소켓 weapon_r        부모 hand_r · (0.19749, 3.411153, −0.381067) · rot 0    ← 총 부착 + 왼손 IK effector
    머티리얼 슬롯 Ch15_body / Ch_49_body / Ch_49_eyelashes  ← Mat_Soldier / Mat_soldier2 / Ch_49_eyelashes
    피직스 에셋 soldier_T_PhysicsAsset (원래 것 그대로. 바디 확인은 에디터에서 → [W29])
    포스트프로세스 ABP 없음  ← 마네킹의 ABP_UEFN_Mannequin_PostProcess 는 thigh 1.12 · head 1.15 스케일 보정이라 붙이지 않는다
/Game/SoldierLab/Characters/Ally/soldier_T_Skeleton  고아 (참조 0건) → 삭제 예정 [W29]

/Game/Characters/UEFN_Mannequin/Meshes/SKM_UEFN_Mannequin
    메시 소켓 weapon_r        부모 본 weapon_r · 항등             ← 2.5절의 소켓 모드 때문에 필요
/Game/Characters/UEFN_Mannequin/Meshes/SK_UEFN_Mannequin
    본 트리 91 → 93  (+ thigh_twist_02_l / thigh_twist_02_r, Assign Skeleton 이 병합)  GUID 재생성 · 전 클립 DDC 재압축 1회

/Game/SoldierLab/Blueprints/BP_Soldier_Friendly    부모 BP_SoldierCharacter · Faction Friendly   (2026-09-13 AI 세션에서 생긴 자식 BP)
    CharacterMesh0.SkeletalMesh = soldier_T
    CharacterMesh0.Materials[0] 오버라이드 리셋      ← 부모의 MI_UEFN_Mannequin_CMC 가 슬롯 0(Ch15_body)을 덮었다
    StanceStandZ 77.1 · StanceCrouchZ 36.4           ← 마네킹 89.7 / 39.4. **메시별 실측값** (아래)
/Game/SoldierLab/Blueprints/BP_Soldier_Hostile     마네킹 그대로. 적군 메시는 미착수 → [Q42]

L_SoldierTest   Ally_A · Ally_B · Ally_B2 · Ally_B3 = BP_Soldier_Friendly / Enemy_A · B · C = BP_Soldier_Hostile
```

**새 메시 하나를 붙일 때 필요한 것 (체크리스트)** — 애니메이션 쪽은 손대지 않는다:

1. 스켈레탈 메시 에디터 → Asset → **Assign Skeleton** → `SK_UEFN_Mannequin`. 본 이름이 UE5 표준이고 부모 체인이 같아야 한다(`IsCompatibleMesh`). 메시에만 있는 본은 스켈레톤에 **추가**된다(GUID 재생성 → 클립 재압축) — 그 대가를 받아들일지 먼저 정한다
2. 메시 소켓 `weapon_r` — `hand_r` 밑, 오프셋 `(0.19749, 3.411153, −0.381067)`. MCP `SkeletalMeshTools.add_socket` + `set_socket_transform`
3. 자식 BP: 메시 지정 · 머티리얼 오버라이드 리셋 · `StanceStandZ/CrouchZ` 실측
4. PIE 로 팔 높이(축 규약) · 총 위치 · 왼손 · 웅크림 확인. **마네킹이 안 바뀌었는지**도 같이

**`StanceStandZ / StanceCrouchZ` 는 클립뿐 아니라 메시에도 종속이다** [A]. 2.5f 의 골반 역산이 `Target = base + StanceStandZ` 를 쓰는데
이 값은 마네킹 골반 높이라, 다른 체형에서는 `PelvOff` 가 0 이 아닌 값에 정착해 매 프레임 골반을 밀어 올리거나 내린다
(아군: +7 → LegIK 가 다리를 최대로 펴고 상체가 숙는 "어정쩡한" 자세). 재는 법은 HUD 역산 — 기립·웅크림 각각에서
**새 값 = 옛 값 − PelvOff**. 자동 산출은 [W27].

**기각한 경로** [A]: `soldier_T_Skeleton` 을 그대로 두고 Compatible Skeletons 로 애니메이션을 공유하는 것.
엔진의 스켈레톤 간 리매핑(`SkeletonRemapping.cpp`)이 회전을 **레스트 포즈 델타**로 보존하므로 A-포즈↔T-포즈 차이만큼
팔이 들린다 → **P76**.

**시도 후 원복** [A]: 총내림 포즈 재저작(`MM_Rifle_LowReady`). 걷기에서 ADS 로코모션과 어긋나 기각. 총내림은 원래대로
`MM_Rifle_Idle_Hipfire_AO_CD` + `BM_LowReady` 마스크다(2.4절). 에셋은 참조 0건으로 보존 → [C-90].

**적군 (`Enemy`)** [A]: Mixamo 65본(`Hips` 루트, `root`/IK/트위스트 없음, 겹치는 본 0), 111,083 버텍스 · LOD 1개, 원본 FBX 는 다른 PC.
리스킨 없이는 이 경로로 못 올린다. 경로 결정 → **[Q42]** (권장: 디자인팀에 "UEFN 마네킹 리그 · 마네킹 비율 피팅 · LOD" 스펙으로 요청).

---

## 4. 자작 C++ [A]

```
Source/SoldierLabEditor/AnimModifiers/
    FootContactCurveModifier      contact_l / contact_r 이진 사각파 생성 (P8b)
    TurnInPlaceCurvesModifier     제자리회전 커브
    SoldierRootFacingModifier     루트 facing 계측용

Source/SoldierLab/                런타임 모듈 — ~~아직 비어 있다~~ **더 이상 비어 있지 않다** [A]
    Math/SoldierAxisLibrary       StepAxis · RampAxisTo — 자세 축용 **등속** 램프
                                  (BlueprintPure. `FInterpTo`/`Lerp`는 지수형이라 목표 근처에서
                                   늘어지고 입력이 멈추면 목표로 붕괴한다 — 자세 축에는 안 맞는다)
                                  → `UpdateBodyYawRate` 가 `RampAxisTo` 를 쓴다 (2.5d)

                                  ★ 2026-09-12 추가 — 골반 높이 역산 2종 (2.5f-5)
                                  SolveBoneHeightOffset(TargetWorldZ, MeasuredWorldZ, AppliedOffset)
                                      = TargetWorldZ - (MeasuredWorldZ - AppliedOffset)
                                  StanceTargetHeight(ActorWorldZ, MeshRelativeZ,
                                                     StandLocalZ, CrouchLocalZ, Stance)
                                      = (ActorWorldZ + MeshRelativeZ)
                                        + Lerp(StandLocalZ, CrouchLocalZ, Stance)
                                  ⚠ **존재 이유**: 블루프린트 팔레트에 산술 노드가 없고(P33),
                                  기존 우회(`Lerp`=곱셈 · `MapRangeClamped`=반파 정류기 ·
                                  `Delta(Rotator)`=회전 전용)로는 **두 runtime float의 뺄셈**을
                                  표현할 수 없다. 기존 클래스에 함수만 추가한 것이라 **P13에 안 걸린다**
    Weapons/SoldierProjectile     ★ 2026-09-12 추가 — 탄도 투사체 (3.1절)
                                  ~560줄(h) / ~1050줄(cpp). titan_example ARCWSProjectile 이식.
                                  Build.cs 에 **Niagara** 추가가 전제다(User. 파라미터 직접 쓰기)

    AI/                           ★ 2026-09-13 추가 — AI 층 9쌍, ~3,670줄 (5절)
        SoldierIdentity           ESoldierFaction · 눈/표적 소켓 · USoldierRegistrySubsystem
        SoldierPerception         ★ FSoldierEnemyRecord · 감쇠 · 역분산 가중 융합 · BroadcastGunshot
        SoldierSight              시야 생산자 (싸구려 기각 + 트레이스 예산)
        SoldierComms              전달 생산자 (말하는 데 걸리는 시간 = 지연 + 속도제한)
        SoldierSuppression        0..1 제압 스칼라. SoldierProjectile이 몬다
        SoldierEngagement         ★ 사격 의도 5종 · 총구 높이 · 재장전 회계
        SoldierCover              시야 판정을 거꾸로 — 노출 + 필요 자세를 한 스윕에서
                                  위치 선택은 세 비용(도착지 노출 + 경로 노출 + 땅의 값)
        SoldierObjective          ★ 땅의 값. 로직 없는 레벨 마커 액터 — 한 개가 양쪽을 섬긴다
                                  ⚠ 레벨 배치 0개 [W25]
        SoldierDebugDraw          네 오버레이 공통 규약 (색=출처 / 크기=해상도 / 굵기=신선도)
                                  Build.cs 에 **AIModule**(SetFocalPoint) ·
                                  **NavigationSystem**(엄폐 후보 네브메시 투영) 추가가 전제다

    Debug/SoldierDebugAxes        라벨 붙은 축 계측 HUD 컴포넌트.
                                  콘솔 `SoldierLab.Debug.Axes 1` / `...Axes.All 1`
                                  기여자는 `SetAxis`만 부른다. 표시가 꺼져 있으면 수집 자체를 건너뛴다
```

> ⚠ 위 두 파일은 **2026-09-11 심야에 추가됐는데 문서에 반영되지 않고 있었다**(2026-09-12 소스 실측).
> `SoldierDebugAxes`가 있으므로 6절·[W6]의 "PrintString 3줄" 기술은 **낡았을 수 있다** [B] —
> 계측을 정리할 때 BP 배선을 실물로 다시 볼 것.

> ⚠ 새 `UCLASS`는 Live Coding으로 안 들어간다. 에디터를 닫고 빌드해야 한다 (P13).
> ⚠ 모디파이어에서 루트모션을 읽으려면 `bIncorporateRootMotionIntoPose = true` (P14).

---

## 5. AI 층 [A] (2026-09-13)

> 상세 4부작:
> **`ai/2026-09-13_perception_stack.md`**(인지·시야·무전·제압·디버그 규약) ·
> **`ai/2026-09-13_engagement_and_cover.md`**(교전·엄폐 기하) ·
> **`ai/2026-09-13_objective_and_position_cost.md`**(★ 목표 · 세 비용 위치 선택) ·
> **`ai/2026-09-13_ai_bridge_and_scene.md`**(블루프린트 배선·레벨·도구 함정 9건) ·
> ★ **`ai/2026-09-14_exposure_ladder_and_corrections.md`**(**교정 라운드 — 값이 바뀜 자리는 이쪽이 최신이다**).
> 이 절은 **무엇이 어디 있는가**만 적는다.

### 5.1 C++ — `Source/SoldierLab/AI/` 9쌍 [A]

| 파일 | 무엇 | 한 줄 |
|---|---|---|
| `SoldierIdentity` | 진영 · 소켓 · 등록부 | `ESoldierFaction{Friendly,Hostile,Neutral}` · 눈 `head` / 표적 **`spine_03`**(정수리만 넘어온 병사는 보이는 게 아니다) · `USoldierRegistrySubsystem`은 **평평한 배열 하나** |
| `SoldierPerception` | ★ 이 층의 본체 | `FSoldierEnemyRecord`는 **관측된 그대로 + 절대 시각**만 담고, **감쇠는 전부 질의 함수**에 있다(P63). **신선도와 해상도를 합치지 않는다**(P62). 융합은 **역분산 가중**(P65) ★ **09-14**: **추측항법이 만료된다** — `VelocityTrustSeconds 1.5`(총성 기록 0). 속도에 대한 믿음은 위치에 대한 믿음보다 먼저 죽는다 (P88) |
| `SoldierSight` | 시야 생산자 | 싸구려 기각(거리²·콘 dot)은 전부, **트레이스만 예산**(라운드로빈 3/틱). **"시야 상실"을 보고하지 않는다** |
| `SoldierComms` | 전달 생산자 | 지연을 지연으로 모델링하지 않았다 — **말하는 데 걸리는 시간**이 낡음과 속도제한을 동시에 만든다. 방송 판정은 **정보량**으로 |
| `SoldierSuppression` | 0..1 스칼라 | 인과적·연속적이라는 것만 주장한다. 회복은 **무조건** 돈다 — 사격이 앞지를 뿐 |
| `SoldierEngagement` | ★ 결정 | 방아쇠는 **"앎이 무기보다 나쁜가"**(P66). 거절 3종은 따로 — `Blocked`(총구에서 트레이스) / `Masked` / 탄약 ★ **09-14**: 막히면 거절하는 대신 **조리개**(Direct/Over/Right/Left)를 찾고 **사격 자세**(Open/Lean/Blind)를 고른다(P81). 사다리를 고르는 것은 **제압도**다. **반동**은 탄창이 줄어드는 것을 보고 센다([W26]). **조준은 240°/s 로 선회**하고 시야 콘도 그 회전을 읽는다(P86) → 새 의도 `Traversing`. `GetDesiredLean` / `GetDesiredBlindFireH/V` 발행([W19] 해결) · `WantsToSprint`(P89) |
| `SoldierCover` | 엄폐 + 위치 선택 | **시야 판정을 거꾸로 돌린 것.** 한 스윕이 **노출 + 필요 자세** 두 답을 준다. 자리는 **세 비용 한 통화**로 고른다 — `Exposure + RouteRiskWeight×RouteRisk + ObjectiveWeight×ObjectiveCost` ★ **09-14**: 첫 항이 노출에서 **`FightingCost`** 로 바뀌었다(P82) — 한 스윙이 이제 **세 답**을 준다(노출 · 필요 자세 · **싸울 수 있는가**). 경로 위험은 **거리로 스케일**(P83) · 후보는 **두 겹 링** · 위협 추정은 **한 바퀴 동안 얼린다** · **정지 감지**(P87)와 **RVO 회피**도 여기 |
| `SoldierObjective` | ★ 땅의 값 (2026-09-13 추가) | **로직 없는 레벨 마커 액터.** 한 개가 **쥔 쪽에겐 수비 · 나머지 전부에겐 공격**이 된다. 수비는 **반경 안 평평한 0**(거리로 매기면 수비대가 중심점으로 붕괴한다 — P73), 공격은 **평지 없는 비례**. ⚠ **레벨에 아직 0개** → **[W25]** ⚠ **09-14 정정**: 루트 컴포넌트가 없어 **여태까지 모든 거리를 (0,0,0) 에서 재고 있었다**(P90). 공격 비용의 **clamp 제거**(P84) · 반경 900→1500 · 밴드 900→1200 · 환율 6000→3000(P85) |
| `SoldierDebugDraw` | 오버레이 공통 규약 | 색=출처 · 회색=센서 활동 · 크기=해상도 · 굵기=신선도 (P70) |

`Build.cs`: **`AIModule`**(`SetFocalPoint`) · **`NavigationSystem`**(엄폐 후보 투영) 추가.

**엄폐/위치 튜닝값 갱신 (2026-09-13 후반)**: `RouteSamples 3` · `RouteRiskWeight 1.0` ·
`ObjectiveWeight 1.2` 신설, `MaxTracesPerTick 4→6`(후보 한 개가 3발→6발) ·
`MoveImprovementMargin 0.35→0.25`. `ASoldierObjective`: `RadiusCm 900` ·
`DefendBandCm 900` · `ApproachScaleCm 6000`. **전부 미측정** → **[C-88]**.
⚠ **이 단락의 값 대부분은 2026-09-14에 바뀌었다** — 아래가 현행값이다.

★ **2026-09-14 교정분** → **[C-91]~[C-94]** · **[C-96]**:

```
SoldierCover     NoCoverCost 1.0 / NoFiringPositionCost 0.5   (신설 — FightingCost)
                 RouteRiskWeight       0.6   (← 1.0)  거리로도 스케일한다 (P83)
                 ObjectiveWeight       2.0   (← 1.2)
                 MoveImprovementMargin 0.3   (← 0.25 ← 0.35)
                 InnerRingFraction     0.45  (신설 — 두 겹 링)
                 StallSpeedCms         20    (신설 — 정지 감지, P87)
                 bUseAvoidance / AvoidanceRadiusCm   true / 300   (RVO)
SoldierObjective RadiusCm 1500 (← 900) · DefendBandCm 1200 (← 900)
                 ApproachScaleCm 3000 (← 6000) · 공격 비용의 clamp **제거**
SoldierEngagement  조리개/자세  LateralReachCm 70 · LeanSpreadScale 1.4 ·
                                BlindSpreadScale 4.0 · SuppressionToGoBlind 0.45 ·
                                BlindFireCloseRangeCm 400
                   조준 선회    AimSlewDegreesPerSecond 240 · OnTargetConeRatio 1.0
                   반동        RecoilPerShot 0.18 · RecoilRecoveryPerSecond 1.2 ·
                                MaxRecoilSpreadScale 2.5 · BlindRecoilScale 2.0
SoldierPerception  VelocityTrustSeconds 1.5   (총성 기록은 0)
```

**하나도 재지 않았다.** 그리고 ★ **수비수가 정착하지 못하는 문제는 여전히 미해결** → **[C-95]**.

### 5.2 블루프린트 [A]

```
BP_SoldierCharacter    컴포넌트 7개 — AC_SoldierIdentity / Perception / Sight / Comms /
                                      Suppression / Engagement / Cover
                       AI 다리 변수 5개 (카테고리 SoldierLab|AI Bridge, 인스턴스 편집 가능)
                           AIPoseDriven · AITargetLean · AITargetStance
                           AITargetBlindFireH · AITargetBlindFireV
BP_Soldier_Friendly    ─┬ BP_SoldierCharacter 의 자식. Faction 기본값 하나만 다르다 (P4)
BP_Soldier_Hostile     ─┘
BP_AR4Rifle            Tick 에서 SetWeaponState(탄/탄창/재장전중) 를 캐릭터 교전 컴포넌트로
AIC_Soldier            ⚠ StartLogic 노드 삭제 — 상속된 ST_Soldier_SmartObject 를 멈춘다
BP_ObserverPawn        부모 DefaultPawn. 이동 방향은 GetControlRotation 에서 온다
                       ★ 빙의 — Tick 에서 조준선 아래의 병사를 HoverSoldier 에 저장해
                          "[F] 빙의" 를 그리고 F 가 **그 저장된 값**을 쓴다 (P94)
                          나올 때도 같은 키 + **SpawnDefaultController()** — 안 하면 그냥 서 있다
                       ⚠ 입력은 InputAction 이 아니라 직접 키 이벤트다 → [W31]
GM_SoldierObserver     GetDefaultPawnClassForController 오버라이드 (P53 때문에 CDO를 못 쓴다)
/Game/SoldierLab/Levels/L_SoldierTest      ★ 2026-09-14 재건
                       네비 영역  x −4000..7000 · y −4500..4500   = 110m × 90m
                       이전 19블록 시가지(액터 22개) 제거 → SoldierLab_Urban/ 5개 폴더
                         Screens   Screen_N · Screen_S            30m 차폐벽 2장
                         Defence   Bld_Def_N/S · Low_Def_A/B · Med_Def
                         Plaza     Crate_Plaza ×3 · Low_Plaza_A/B   중앙 약 25m 공지
                         Flanks    Bld_N1/N2 · Bld_S1/S2 · Low_N · Low_S
                         Attack    Bld_Atk_N/S · Med_Atk · Low_Atk/Atk2
                       SoldierLab_Test   적대 · 아군 · PlayerStart 전부 재배치
                       ✅ Objective_AllyBase (−2500, 0) Friendly — 목표 **1개** 배치됨
                          배치된 수비 엄폐가 전부 목표 비용 0.00~0.05 임을 확인 [A]
                       ⚠ 규모를 키운 이유: 교전 상한이 **정지 95m** 라
                          옛 레벨에서는 어디서 쓰든 항상 사거리 안이었다 → [C-97]
```

**AI→몸 다리의 모양 — 연속 축 전부 동일** (P71):

```
SetX( SelectFloat( A = RampAxisTo(X, clamp(AITarget), Rate, dt),
                   B = <기존 플레이어 식>,
                   bPickA = AIPoseDriven ) )
```

- **`Rate` 는 플레이어의 키 스텝이 쓰는 바로 그 변수**다 — AI가 사람보다 축을 빠르게
  움직일 수 없다
- 목표는 **`MapRangeClamped` 로 클램프**한다 — `RampAxisTo` 에 클램프가 없다

**Tick 접합 순서**:

```
SetAIPoseDriven(NOT IsPlayerControlled) → SetAITargetStance(Engagement) →
★ SetAITargetLean / SetAITargetBlindFireH / SetAITargetBlindFireV (Engagement) →   ← 09-14, [W19]
SetActualStance(StanceAxis) → <기존 갱신 체인> →
Branch(AIPoseDriven) → Branch(WantsToFire) → Shoot / else Branch(WantsToReload) → StartReload
```

**조준 상태**: AI는 `WantsToAim` 을 **GASP 입력 상태 구조체**에 Break/Make 로 실어
`UpdateInputStateServer` 로 보낸다(다른 필드는 전부 보존). ★ **2026-09-14에 `WantsToSprint` 가 같은 경로로 붙었다** — 규칙이 아니라 **느릴 이유의 부재**다(P89). 프로젝트 자신의 기존 AI 경로
`STT_SetSoldierInputState` 를 읽어서 찾았다. ⚠ **"AI가 총을 안 들고 몸도 안 돈다"의 근본
원인**이 이것이었다 — `Enable_AO()` 가 `RotationMode == aim` 을 요구하고, 그것은
`S_PlayerInputState.WantsToAim` 에서 파생된다. 파생 플래그 `AOActive` 를 직접 만졌던
이전 패치는 **증상 처치**였으므로 되돌렸다 (P36).

### 5.3 남아 있는 P0-1 실험 자산 [B]

```
/Game/SoldierLab/AI/   ST_Soldier_Patrol_Subtree · ST_Soldier_SmartObject
                       STT_FocusToPlayer · STT_SetSoldierInputState
```

`ST_Soldier_SmartObject` 는 `ST_NPC_SandboxCharacter_SmartObject` 의 복제본이고
`Root/SmartObject/Patrol` 상태를 갖는다 — **병사들을 벤치로 걸어가게 하던 것**이다.
지금은 `StartLogic` 삭제로 **돌지 않는다.** 지우지는 않았다.
→ `ai/prototypes/2026-09-04_p0-1_ai_drives_mm.md`

### 5.4 ⬜ 여전히 초안만 [A]

`squad/drafts/`(L0 명령 · L1 분대) · `ai/drafts/`(인지·위협평가) ·
`cover/drafts/`(EQS/SmartObject) 의 C++ 초안은 **한 줄도 프로젝트에 들어가 있지 않다.**
컴파일된 적도 없다.

⚠ **인지와 엄폐는 초안과 *다른 물건*으로 구현됐다.** 이름이 겹치는 자리가 있으나
같은 타입이 아니다 — 초안을 읽고 코드를 예상하지 말 것.

---

## 6. 아직 없는 것 (착각 방지)

| 없는 것 | 비고 |
|---|---|
| **피격 반응 · 사망 · 데미지 · 체력** | 클립 19개는 반입돼 있으나 **배선 없음.** 래그돌은 GASP에 이미 있다. ⚠ **AI 층이 생긴 지금 이것이 더 아프다 — 병사가 죽지 않으니 교전이 끝나지 않는다** → **[W18]** |
| 이동 중 급선회(spin) | Lyra에 클립이 없다. 회전 클립 + 비용편향으로 근사 중 |
| 대각선 이동 전용 클립 | Lyra 세트는 4방향뿐. 워핑으로 메운다 — 가끔 발이 꼬이는 원인 |
| ~~견착/총내림 의도 분기~~ | **✅ 해결 (2026-09-11)** — Chooser `RotationMode` 열 + Idles DB 분리. [C-55] |
| ~~웅크리기 전용 AO 배선~~ | **✅ 배선됨 — 단 이진이다 (2026-09-12)**. `Select(Stance == Crouch ? AO_Rifle_Crouch : AO_Rifle_ADS)`. DB 전환과 **같은 순간**에 갈려서 눈에 거슬리지 않는다. **연속 블렌드는 미구현** → **[W11]** |
| **캡슐/엄폐 높이가 여전히 이진** | stance 축이 **보이는 높이만** 연속으로 만든다. `Crouch()`가 문턱에서 86↔60을 한 번에 바꾸므로 **충돌과 엄폐 높이는 이진**이고, **AI의 엄폐 판단이 읽는 것이 바로 그 높이**다. 남은 조각 중 **AI 관점에서 가장 값어치 있고 위험도 가장 높다**(관통·계단 오르기·**일어설 때의 천장 스윕**은 CMC가 이진 경우에 대해서만 구현해 두었다) → **[W9]** · 안정성 판정은 **[C-3]** |
| **웅크림 카메라** | `CameraRig_CrouchOffset`은 Camera Pose 공간의 고정 `TranslationOffset (40, 0, −30)`이고 **블렌더블/데이터 파라미터가 없다** — 즉 **블렌드되는 이진**이라 float를 못 받는다. **결정: 리그를 고치지 않고 비활성화한 뒤 SpringArm Z를 stance 축으로 직접 몬다.** 미착수 → **[W10]** |
| **`IA_Crouch` 토글이 무력** | stance 축이 `bIsCrouched`를 매 프레임 소유한다. 제거하거나 "stance를 0/1로 명령하는 입력"으로 재정의할 것 → **[W13]** |
| **적군 메시** | `BP_Soldier_Hostile` 은 **마네킹 그대로**다. `Enemy` 에셋은 Mixamo 리그라 리스킨 전에는 못 올린다. 경로 결정 대기 → **[Q42]**, 3.2절 |
| **`StanceStandZ/CrouchZ` 자동 산출** | 메시별 실측값이라 캐릭터 메시가 올 때마다 HUD 역산으로 재야 한다 → **[W27]** |
| **총내림 자세의 재저작** | 2026-09-13에 저작·배선했다가 걷기에서 어긋나 **원복**. `MM_Rifle_LowReady` 참조 0건으로 보존 → **[C-90]** |
| **아군의 "마네킹 비율" 근사** | 사용자 요구: soldier_T 가 마네킹 뼈 길이로 움직이게. translation retargeting `Animation` 5분 시험 미실시 → **[W28]** |
| **투사체의 리플리케이션** | 이식하며 **3분기 Multicast 라우팅을 걷어냈다** — 세 핸들러가 전부 `PlayImpactEffect`로 되돌아왔으므로 방송할 대상이 없는 지금은 직접 호출과 같다. 멀티가 생기면 여기로 돌아온다 → **[W17]** |
| **진영(Faction) 판정 — 투사체만** | **정식 소스는 2026-09-13에 생겼다**(`USoldierIdentityComponent::Faction`, 5.1절). AI 층은 전부 그것을 읽는데 **투사체만 아직 `bHitEnemy = IsA<ACharacter>()` 대역**이다 → **[R7]** · 갈아끼우기 **[W23]** |
| **바람** | 나이아가라 `WindVectorCms` 파라미터에 **0을 먹인다.** 바람 소스가 생기면 한 줄 |
| **투사체 풀** | `LaunchFrom` / `Deactivate` 는 있는데 **풀이 없다** — 무기 컴포넌트가 없어 발당 `SpawnActor` 한다. 붙이는 것은 **쏘는 쪽의 변경** |
| **무기 메시의 애니메이션(볼트·탄창)** | `SK_AR4_X` 에 애님 블루프린트가 없다. GASP `SK_Rifle`의 `ABP_Weap_Rifle`을 겨냥하던 `Montage_Play` 2개는 **삭제**했다(항상 None) → **P50**. 되살리려면 새 메시용 무기 ABP를 만들어야 한다 |
| **디버그 궤적 라인** | 안 보인다. 그 외에는 전부 동작한다 → **[C-81]** |
| **무기/투사체 튜닝값의 검증** | 전부 `titan_example` 에서 온 숫자다(도탄 3 · 데칼 200 · 휘즈 200/1500 · fireRate 0.12 …). 이 프로젝트에서 재본 적 없다 — P18·P30과 같은 계열의 위험 → **[C-82]** |
| ~~제압(suppression) 신호~~ | **✅ 해결 (2026-09-13)** — `ApplySuppressionAlongSegment()`가 휘즈와 같은 최근접 판정을 **병사 등록부**에 대해 한 번 더 한다. 그 과정에서 `PreviousLocationForWhiz` 가 휘즈 블록 **안**에서 갱신되던 버그를 고쳤다 → **[W16]** |
| **에셋 경로 정리** | `M_Decal_Bullet` · `M_RCWSRound` 가 **마이그레이션된 경로 그대로** 있다 → **[W14]** |
| ~~엄폐 · 인지~~ | **✅ 구현됨 (2026-09-13)** — 단 **초안과 다른 물건**이다. 5절 |
| **분대 · 명령** | 여전히 초안 단계. ★ **목표는 2026-09-14에 실제로 돌기 시작했다** — 레벨에 1개 배치 + 루트 컴포넌트 버그 수정(P90). 다만 여전히 **마커 하나 · 소유권 변경 없음 · 국면 전환 없음** → **[W25]** |
| ~~린 · 블라인드 파이어를 AI가 몰지 않음~~ | ✅ **해결 (2026-09-14)** — 교전 컴포넌트가 `GetDesiredLean` / `GetDesiredBlindFireH/V` 를 발행한다. ★ **새 판단 재료를 하나도 안 만들었다** — 조리개의 답이 **위 아니면 옆**이고 그것이 이미 있던 두 축이었다(P81). 칸을 고르는 것은 제압도 → ~~[W19]~~ · `ai/2026-09-14_exposure_ladder_and_corrections.md` 1절 |
| **경로의 노출(직선뿐) · 엄폐 후보의 다양성 · 소리 차폐** | 경로 항은 들어왔으나 **네브메시 경로가 아니라 직선 표본**이다(09-14에 **거리 스케일**이 붙었다). 후보는 09-14에 **두 겹 링**이 됐으나 여전히 **고정 반경 둘**이다. 각각 **[W20]** · **[W21]** · **[W22]** |
| ~~반동 누적이 없다~~ | ✅ **해결 (2026-09-14)** — `RecoilSpread` 가 `EffectiveSpreadAtCm` 의 곱셈 항으로 들어갔고, 발사는 **탄창이 줄는 것**으로 센다(새 훅 없음). 맹목사격은 상승률 2배 → ~~[W26]~~ · 값은 **[C-92]** |
| **1인칭 카메라 · 플레이어가 조종하는 병사의 1인칭 모드** | **요청됐고 만들지 않았다.** 빙의(P94)는 있으나 카메라는 3인칭이다. ⚠ 총구 정렬·조준 오프셋·블라인드 파이어의 **판정 기준이 1인칭에서 바뀐다** → **[W30]** |
| **조리개 트레이스가 예산 밖** | `FindAperture` 가 틱당 최대 4발을 쓰는데 엄폐의 라운드로빈 예산에 없다. 디버그를 켜면 **한 번 더** 돌아 8발이 된다 → **[W32]** · [C-83] |
| ★★ **수비수가 정착해 쓰지 못한다** | 두 번 시도했고 둘 다 실패했다. 다음은 추측이 아니라 **진단**이다 — 엄폐 오버레이가 **HERE · best · 여유**를 전부 찍는다 → **[C-95]** · `ai/2026-09-14_exposure_ladder_and_corrections.md` 12절 |
| **`Cover` 채널(`GameTraceChannel4`)이 미사용** | 시야·엄폐·사선 셋 다 `GameTraceChannel5`("Sight")를 쓴다. [Q21]이 채널을 둘 판 이유가 아직 실현되지 않았다 → **[W24]** |
| **45명 규모에서의 AI 비용** | 인지·시야·엄폐가 전부 매 틱 트레이스를 쓴다(예산은 있다). 한 번도 안 재봤다 → **[C-83]** |
| 멀티플레이 검증 | 한 번도 안 했다 |
| ~~블라인드 파이어(맹목사격)~~ | **✅ 해결 (2026-09-12)** — 단 **IK가 아니라 저작 포즈 + 연속 애디티브**로 성립했다. 2.5e절. [C-8]의 "IK 품질 하한"이라는 질문 자체가 형태를 바꿨다 |
| **린(lean) 축의 문서** | 축 자체는 **구현돼 동작 중**인데(`ModifyBone spine_01..05` · `IA_Lean` · Q/E) 이 문서에 절이 없다. 2.4절 체인이 그 존재를 처음 기록했다 → **[W7]** |
| **블라인드 파이어 × 총구 보정의 상호작용** | BF 자세를 `AimCorrection` 적분기가 **"고쳐야 할 오차"로 학습**할 수 있다 [B]. BF를 켠 채 `AimErr`/`AimCorr`를 봐야 한다 → **[C-78]** |
| **마스크 규약 통일** | `BlendMask`(+`BM_LowReady`)와 `BranchFilter` **두 벌이 그래프에 공존**한다 → **[W8]** |
| **간헐적 무기 잠금의 원인** | 급선회 뒤 아주 가끔 총이 내려간 채 안 올라온다(`AimErr Y 99°` · `AimCorr 25` 포화). **포화 적분기의 두 번째 평형점**으로 설명되지만 **99° 를 만든 방아쇠는 미규명.** 안티 와인드업 후 재현 안 됨 → **[C-76]** |
| **극단 상방 조준 시 무기 공중제비** | **의도적 미해결(won't fix).** 프로젝트 범위 밖 — 수직 상방 조준은 전술 슈터에서 나오지 않는 자세다 → **[C-77]** |
| `AO_Blend_Curve` 내용 확인 | `BlendListByBool_0`의 `customBlendCurve`. **MCP로 키 값을 읽을 수 없다**(`FRichCurve` 미노출 · `unreal` 파이썬 모듈 차단). 최댓값 > 1.0이면 독립적인 오버슈트 원인 → **[R6]**, 에디터 수동 확인 |
| 총구 정렬 **계측 장치의 제거/게이트** | `BP_SoldierCharacter` Tick에 `DrawDebugCoordinateSystem`(분기 **밖** — 모든 병사에게 그려진다) + `PrintString` 3줄(`ERR`/`ACC`/`GATE`)이 **아직 물려 있다.** 출하 전에 **제거하거나 디버그 플래그 뒤로** 보낼 것. ⚠ **`GATE` 표시는 죽은 오차 크기 게이트를 찍고 있다** — 실제 게이트(`InRange(0..2.0)`)가 아니다. 완전히 죽은 잔해는 `Delta(control, actorRotation)` · `Lerp(Rotator) Alpha 0.15` · `ToString(Rotator)` ×2 · `GetControlRotation` ×1 → **[W6]**, `animation/prototypes/2026-09-11_muzzle_aim_alignment.md` 11절. **2026-09-12에 계측 축이 더 늘었다** — `BodyErr` · `WpnLow` · `WpnTgt` · `AimGain` · `AimGate` · `AimCorr` · `AimErr` · **`BF_H` · `BF_V`** · **stance 축의 7행(`Stance` · `PelvWZ` · `PelvTgt` · `PelvOff` · `Crouched` · `SpdCap` · `GaitClamp`)**. 그리고 **죽은 변수 3개**(`StanceDropMax` · `StanceRaiseMax` · `StanceBlendRate`)가 생겼다. ⚠ **단 stance 7행은 함부로 떼지 말 것** — 이 축의 문제는 **과도구간에만 존재**해서 저 행들이 없으면 진단이 불가능하다(**P44**) |

---

## 7. 이 문서를 고칠 때

- **에셋을 추가/변경하면 여기부터 고친다.** 실물과 어긋나면 이 문서는 해로워진다
- 숫자를 바꿀 때는 **왜 그 값인지**를 같이 적는다. 2.6절이 그 원칙으로 쓰였다
- 새로 알게 된 함정은 이 문서가 아니라 `CLAUDE.md` 5절(P번호)에 넣고 여기서는 참조만 한다
