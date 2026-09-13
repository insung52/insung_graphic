# 블라인드 파이어 축 — 세 번째 연속 자세 축

2026-09-12 / **성공** / 저작 포즈 3장을 마스크 애디티브로 얹어 `BlindFireH`(−1~+1) · `BlindFireV`(0~+1) 두 연속 축을 만들었다. 사용자 확인 — **"완전잘돼. 구현 완료."**

관련 항목: **[C-8]** 해결(형태가 바뀜) · **[C-78]** 신설 · **[W7]** · **[W8]** 신설 / 관련 문서: `IMPLEMENTED.md` 2.4·**2.5e 신설**·2.6·3절 · `CLAUDE.md` **P41~P43** · **6.3절 신설** · `animation/prototypes/2026-09-12_body_yaw_rate_and_aim_antiwindup.md`

> **이 문서의 값어치는 애님그래프가 아니라 포즈 저작 경로에 있다.**
> 배선 자체는 반나절이었다. 시간을 먹은 것은 **"포즈를 무엇으로 만드는가"** 였고
> 거기서 **막다른 길 넷**을 지났다. 마지막 하나(5절)는 **웹 검색으로 찾을 수 없었고**,
> 에셋을 블루프린트로 열어 **변수 목록을 읽어서** 나왔다.

---

## 1. 무엇을 만들었나 [A]

총구 조준 정렬(2026-09-11) · 린(lean)에 이은 **세 번째 연속 축**이다.

| 축 | 범위 | 뜻 |
|---|---|---|
| `BlindFireH` | **−1(좌) ~ +1(우)** | 엄폐물 좌/우로 몸을 내밀어 보지 않고 쏘는 자세 |
| `BlindFireV` | **0 ~ +1(상)** | 엄폐물 위로 팔을 들어 쏘는 자세 |

**조작 규약은 린 축과 같다** — 키를 누르고 있으면 **등속으로 램프**되고, **떼면 그 값에서 멈춘다**(0으로 돌아가지 않는다). 리셋은 별도 키다.

```
1 = 좌      2 = 위      3 = 우      4 = 리셋
```

**판정 기준**: 세 방향 모두 ① 키를 누른 만큼 연속적으로 자세가 변하고 ② 떼면 그 자세를 유지하며 ③ 그동안 **하체 로코모션이 흐트러지지 않고** ④ **왼손이 총에서 떨어지지 않을 것**.

---

## 2. 설계 결정 — 순수 IK를 기각했다 [A]

어시스턴트가 **먼저 권한 것은 순수 IK 안**이었다. 무기 트랜스폼에 오프셋을 주고 팔 IK가 풀게 하면 **저작 클립이 한 장도 필요 없다**는 것이 근거였다.

**사용자가 기각했다.** 이유가 결정적이다:

| | |
|---|---|
| 사용자는 **특정 자세(타르코프류)** 를 이미 그리고 있었다 | "팔이 어딘가에 닿는다"가 아니라 **정해진 실루엣**이 필요했다 |
| 그 자세는 **몸통/척추가 돌아간다** | **팔 IK로는 만들 수 없다** |

**팔 IK가 척추를 돌리지 못하는 것은 결함이 아니라 설계다** [A]. 팔 IK 체인은 표준 바이페드 리그에서 예외 없이 `upperarm → lowerarm → hand` 세 마디이고, **척추로 전파되지 않는다.** 전파시키려면 FullBody IK가 필요한데 그건 45명 규모에서 비용이 다르다([C-11]).

**→ 채택안: 저작 포즈 + 연속 애디티브 블렌딩.** IK는 **왼손을 총 그립에 붙잡아 두는 역할로 축소**한다.

> 이 기각은 **P41의 사례이기도 하다** — 어시스턴트가 권한 IK 안은 "되는가"를 확인하지 않고
> "되어야 할 것 같다"로 권한 것이었다. 4.1절에서 같은 실수를 한 번 더 한다.

---

## 3. 포즈 저작 — **통하는 경로** [A]

막다른 길을 넷 지난 뒤 확정된 순서다. **다음 세션은 이것만 따라가면 된다.**

```
1. BP_SoldierCharacter 를 레벨에 배치한다
   ★ 이유: 무기가 에디터에서 렌더링되므로 포즈를 잡는 동안 총구 방향이 보인다

2. Level Sequence → [+ Add] 로 그 액터를 추가
   → 액터 바인딩의 [+] → COMPONENTS → **CharacterMesh0**        ← 이것이 스켈레탈 메시 트랙

3. CharacterMesh0 트랙의 [+] → Animation → MM_Rifle_Idle_ADS     ← 베이스 포즈

4. [+] → Animation → Control Rig
   → ★ **Filter Asset By Skeleton 체크를 끈다** (4.4절)
   → CR_Mannequin_Body

5. ★ 리그의 **R Arm IK Mode** / **L Arm IK Mode** 를 켠다 (5절)

6. 포즈 작업 순서 — **몸통부터**
      body_ctrl  +  spine_01~05_ctrl      몸통 회전 (이 자세의 본체)
      hand_r_ik_ctrl                      총구 방향
      arm_r_pv_ik_ctrl                    팔꿈치
      hand_l_ik_ctrl                      왼손

7. 스켈레탈 메시 바인딩 우클릭 → **Bake Animation Sequence**
   → /Game/SoldierLab/Animations/Rifle/Poses/
```

---

## 4. ★★ 막다른 길 4종 — 각각 **왜** 막혔나

### 4.1 `CR_UEFN_Mannequin_FullBodyIK`는 **포즈용 리그가 아니다** [A]

```
/Game/Characters/UEFN_Mannequin/Rigs/CR_UEFN_Mannequin_FullBodyIK
    컨트롤: hand_l_target , hand_r_target       ← 정확히 **2개**
```

이름에 `FullBodyIK`가 들어 있고 `Characters/UEFN_Mannequin/Rigs/` **아래 유일한 리그**라 당연히 포즈용이라고 봤다. **아니었다** — 런타임 손 IK 이펙터 리그다.

두 타깃이 원점에 놓여 있는 것도 오해를 키웠다. **죽은 소켓이 아니라, 에디터에서 그것을 구동하는 것이 아무것도 없어서** 원점에 있는 것이다.

> **이것을 권한 것은 어시스턴트의 오류다.** `list_variables` / 컨트롤 목록 확인 **한 번**이면
> 막지 못할 이유가 없었다. → **P41**

### 4.2 `Bake To Control Rig`가 **이 빌드에 없다** [A]

기존 애니메이션의 포즈를 컨트롤 리그의 컨트롤로 옮기는 그 메뉴를 **찾을 수 없었다.**

```
애니메이션 **섹션** 우클릭  →  Properties / Edit / Order / Active / Locked / Group /
                              Delete / Key This Section / Motion Blending Options /
                              Show Skeleton                       ← Bake To Control Rig 없음
애니메이션 **트랙** 우클릭  →  없음
```

**결과**: 풀 컨트롤 리그는 **자기 기본 포즈에서 출발할 수밖에 없다.** 조준 아이들 자세를 물려받을 방법이 없다.

> 이것이 8절(저작 시작 포즈는 정확성에 영향이 없다)을 확인해야 했던 이유다.
> **없는 기능을 우회하기 전에 "우회해도 결과가 같은가"를 먼저 증명했다.**

### 4.3 내장 **FK Control Rig은 에셋이 아니다 — 목록에 영원히 안 나온다** [A]

```
Control Rig 선택 팝업
┌──────────────────────────────┐
│  FK Control Rig       ← ★ **헤더처럼 보이지만 클릭 가능한 항목이다**
│  [ 검색창           ]         이 글자를 클릭하면 본별 FK 리그가 자동 생성된다
│  ─────────────────────        │
│  CR_... (에셋 목록)           │
│  [ ] Layered                  ← 켜면 **레이어링 지원 리그만** 남는다
└──────────────────────────────┘                (CR_UEFN_Mannequin_FullBodyIK 가 빠진다)
```

검색창 **아래**의 에셋 목록만 훑으면 영원히 못 찾는다. FK 리그는 **포즈 저작에는 쓸 수 있지만 IK가 없다** — 손 위치를 직접 잡을 수 없어 총을 든 자세에는 불리하다.

### 4.4 ⚠⚠ `Filter Asset By Skeleton`은 **본 호환성이 아니라 스켈레톤 에셋 동일성**을 본다 [A]

이것이 가장 오래 속인 함정이다.

```
우리 메시의 스켈레톤        SK_UEFN_Mannequin
CR_Mannequin_Body 의 것     UE5 마네킹 스켈레톤        ← **다른 에셋**
본 계층                     동일
```

필터가 **에셋 동일성**으로 거르므로 `CR_Mannequin_Body`가 목록에서 **사라진다**. 그런데 이 리그는 우리 메시를 **완벽하게 구동한다.**

**본 계층이 같다는 증거** — Anim Outliner에 이것들이 그대로 있다:

```
calf_twist_01_r    thigh_twist_01_r
ik_foot_root       ik_foot_l / ik_foot_r
ik_hand_root       ★ ik_hand_gun ★     ik_hand_l / ik_hand_r
```

**`ik_hand_gun`은 Epic 마네킹 특유의 본**이다. 이것이 있다는 것은 계층이 Epic 표준 그대로라는 뜻이다.

> **→ 필터를 끈다.** 그리고 일반화: **UI 필터가 기대한 것을 숨기면, "없다"고 판단하기 전에
> 그 필터가 실제로 무엇을 비교하는지 묻는다.** → **P42**

---

## 5. ★★★ 핵심 발견 — 리그가 **팔다리별로 IK/FK를 변수로** 고른다 [A]

`CR_Mannequin_Body`를 붙이고 나서도 **증상이 반으로 갈렸다**:

```
다리 · 발 IK 컨트롤     ✅ 반응한다
팔 · 손 · 머리 컨트롤   ❌ 아무 일도 일어나지 않는다
```

원인은 리그가 들고 있는 **불리언 리그 변수**였다:

```
L Arm IK Mode        R Arm IK Mode
L Leg IK Mode        R Leg IK Mode
Spine IK Mode        Neck IK Mode
```

**다리는 기본값이 IK, 팔·척추·목은 기본값이 FK다.** 비대칭의 전부가 이 한 줄로 설명된다.

### 5.1 왜 Anim Outliner에서 안 보이는가 [A]

**이것들은 컨트롤이 아니라 리그 *변수*다.** Anim Outliner는 컨트롤만 나열하므로 여기엔 절대 안 나온다.

```
보이는 곳 ①   시퀀서에서 Control Rig 트랙을 펼치면 **채널**로 나온다
보이는 곳 ②   Control Rig 트랙을 선택했을 때 Details 패널
```

### 5.2 ★ 어떻게 찾았나 — **웹 검색으로는 못 찾았다** [A]

여러 번의 표적 검색이 **아무것도 주지 않았다.** 답을 준 것은 한 줄이다:

```python
BlueprintTools.list_variables("/Game/Characters/UE5_Mannequins/Rigs/CR_Mannequin_Body")
```

**컨트롤 리그 에셋은 블루프린트로 열린다** — `list_variables` · `list_graphs`가 그대로 동작한다.

> **프로젝트 고유 설정은 웹보다 에셋이 빠르다.** 웹에는 "이 리그의 변수 이름"이 있을 이유가
> 없다. → **P43** (`CLAUDE.md` 6.3절에도 수단으로 기록)

---

## 6. 기각한 가설 — 시퀀서 키잉 [A]

**"컨트롤 리그 값은 키를 찍지 않으면 0으로 되돌아간다"** 는 실제로 존재하는 동작이고 공식 문서에도 있다(`S` 키 또는 Auto Key —
https://dev.epicgames.com/documentation/unreal-engine/animating-with-control-rig-in-unreal-engine).

**그런데 이번 증상의 원인은 아니었다.**

```
판별자:  키잉이 원인이라면 **다리도 같이** 실패했어야 한다.
         다리는 됐다.  →  키잉은 원인이 아니다.
```

> **비대칭은 그 자체로 가설 판별기다.** "전부 안 된다"가 아니라 "일부만 안 된다"이면,
> 전역적인 원인(키잉·저장·컴파일)은 **그 한 줄로 기각된다.** (P16·P39와 같은 계열)

---

## 7. ★ 저작 시작 포즈는 **정확성에 영향이 없다** [A]

4.2절(`Bake To Control Rig` 부재) 때문에 조준 아이들 포즈를 리그에 물려받을 수 없었다. 그래서 **"A-포즈에서 저작해도 되는가"** 를 먼저 따졌다.

**된다.** 메시 스페이스 애디티브는 정의상

```
애디티브  =  (저작 포즈  −  기준 포즈)

기준 포즈 위에 다시 얹으면        →  저작 포즈가 **정확히** 복원된다
로코모션 포즈 위에 얹으면          →  **같은 델타**가 더해진다
```

즉 **어디서 출발해 저작했는지는 결과에 들어가지 않는다.** 기준 포즈(`refPoseSeq` / `refFrameIndex`)만 맞으면 된다.

**조준 아이들에서 출발하는 것의 이득은 오직 작업량**이다 — 이미 총을 든 자세이므로 손댈 곳이 적다. 정확성 문제가 아니다.

> 이 확인이 없었으면 `Bake To Control Rig`를 찾느라 더 오래 헤맸을 것이다.
> **없는 기능을 우회하기 전에, 우회해도 결과가 같은지부터 증명한다.**

---

## 8. 에셋 [A]

세 장 전부 `SK_UEFN_Mannequin` 스켈레톤. 베이크 결과는 **151키 / 150프레임 / 5.0초**이고 **쓰이는 것은 프레임 0 하나뿐**이다(9절의 `SequenceEvaluator`가 `ExplicitTime = 0`을 쓴다).

```
/Game/SoldierLab/Animations/Rifle/Poses/MM_Rifle_BlindFire_L
/Game/SoldierLab/Animations/Rifle/Poses/MM_Rifle_BlindFire_R
/Game/SoldierLab/Animations/Rifle/Poses/MM_Rifle_BlindFire_U
```

**애디티브 변환** (세 장 동일):

```
additiveAnimType = AAT_RotationOffsetMeshSpace
refPoseType      = ABPT_AnimFrame
refPoseSeq       = /Game/SoldierLab/Animations/Rifle/Idles/MM_Rifle_Idle_ADS
refFrameIndex    = 0
```

- 사용자가 **팔뿐 아니라 척추/몸통과 머리까지** 저작했다 — 2절의 기각 이유가 그대로 반영된 것이다
- **왼손은 총에 붙어 있는 상태로 저작**돼 있다

> ⚠ `ABPT_AnimFrame`은 **다른 에셋을 참조하는** 기준 포즈다. 리타깃하면 이 참조가 끊기고
> **소스까지 바뀐다** (**P20**). 이 세 장을 리타깃할 일이 생기면 참조를 다시 확인할 것.

---

## 9. 애님그래프 — `SoldierCharacter_ABP` [A]

### 9.1 끼운 위치

조준 애디티브를 적용한 **직후**, 캐시 포즈를 저장하기 **직전**이다.

```
ApplyMeshSpaceAdditive_0        (조준 오프셋 적용)
      →  [BF_L]  →  [BF_R]  →  [BF_U]  →  SaveCachedPose_0 'AimedPose'  →  (하류 그대로)
```

**왜 여기인가**: `AimedPose` 캐시 **위쪽**에 넣어야 상체 슬롯(`UpperBody`)과 몽타주가 이 자세 위에서 돌고, 하류의 IK가 마지막에 왼손을 다시 잡아준다(9.4절).

### 9.2 레이어 한 벌의 구조 (3벌 동일)

```
AdditiveIdentityPose ────────────────────────────→ LayeredBoneBlend . BasePose
SequenceEvaluator(MM_Rifle_BlindFire_X, ExplicitTime 0)
                     ───────────────────────────→ LayeredBoneBlend . BlendPoses_0
                                                   BlendWeights_0 = 1.0
LayeredBoneBlend     ───────────────────────────→ ApplyMeshSpaceAdditive . Additive
ApplyMeshSpaceAdditive . Alpha  ←  ABP 변수  BF_AlphaL / BF_AlphaR / BF_AlphaU
```

**`LayeredBoneBlend`는 "애디티브를 상체로 마스킹"하는 데만 쓴다.** 베이스가 애디티브 항등 포즈이므로, 마스크 밖의 본은 **델타 0**이 되어 아무 영향을 받지 않는다.

### 9.3 마스크 [A]

```
blendMode   = BranchFilter
layerSetup  = [ { branchFilters: [ { boneName: "spine_01", blendDepth: 1 } ] } ]
```

`spine_01`부터 아래로 **척추 · 쇄골 · 양팔 · 목 · 머리**를 포함하고, **골반과 다리를 제외**한다. 그래서 **로코모션이 전혀 건드려지지 않는다** — 판정 기준 ③ 충족.

> ⚠⚠ **이 그래프에는 이제 마스크 규약이 두 벌 존재한다** [A].
> 기존 `LayeredBoneBlend_0`(로우레디)는 **`blendMode = BlendMask`** 에 블렌드 마스크 에셋
> **`SK_UEFN_Mannequin:BM_LowReady`** 를 쓰고, 새 세 개는 **`BranchFilter`** 를 쓴다.
> 기능적으로는 둘 다 되지만 **다음 사람이 "왜 두 가지인가"를 묻게 된다** → **[W8]**.

### 9.4 ★ 왼손 IK를 다시 짤 필요가 없었던 이유 [A]

```
TwoBoneIK_0 (hand_l) 은 그래프의 **마지막 스켈레탈 컨트롤 노드**다
        ... → FootPlacement_0 → LegIK_1 → **TwoBoneIK_0** → ComponentToLocalSpace_2 → ...
```

새 레이어는 그보다 **훨씬 위**(컴포넌트 스페이스 진입 전)에 들어가므로, 몸통을 아무리 틀어도 **`TwoBoneIK_0`가 마지막에 왼손을 그립에 다시 앉힌다.** 판정 기준 ④가 **배선 없이** 충족됐다.

> 이것은 운이 아니라 **순서의 결과**다. "IK는 항상 마지막"이 규칙으로서는 틀렸지만
> (`animation/2026-09-02_pose_pipeline_spec.md`), **이 그래프에서는 실제로 마지막이었다.**

### 9.5 ★ 전체 평가 체인 (정정 포함) [A]

`IMPLEMENTED.md` 2.4절의 체인이 **린 축의 `ModifyBone`을 빠뜨리고 있었다.** 실측한 전체는 이것이다:

```
MotionMatching / TwoWayBlend
  → BlendListByInt_0
  → ApplyMeshSpaceAdditive_2   (+ BS1D_Additive_Lean_Run)
  → ApplyMeshSpaceAdditive_1   (+ Slot 'FullBodyAdditivePreAim')
  → ApplyMeshSpaceAdditive_0   (+ DeadBlending ← BlendListByBool_0)
                                    pose0 = AO 블렌드스페이스      (견착)
                                    pose1 = LayeredBoneBlend_0     (총내림/로우레디 애디티브)
  → [ BF_L → BF_R → BF_U ]                                   ← ★ 이번에 추가
  → SaveCachedPose 'AimedPose'
  → LayeredBoneBlend_1 (+ Slot 'UpperBody')
  → ApplyAdditive_1    (+ Slot 'UpperBodyAdditive')
  → Slot 'DefaultSlot'
  → OffsetRootBone_0 → RemapCurves_0 → LocalToComponentSpace_0
  → ModifyBone spine_01..05          ← **린(lean) 축**
  → FootPlacement_0 → LegIK_1 → TwoBoneIK_0 (hand_l)
  → ComponentToLocalSpace_2 → PoseHistory → Root
```

### 9.6 ⚠ 세션 중의 오독 정정 — `BlendSpacePlayer_1` [A]

이 세션 중간에 **`BlendSpacePlayer_1`(AO 블렌드스페이스)을 "독립된 전체 포즈"로 읽은 판단**이 있었다. **틀렸다.**

```
실제:  BlendSpacePlayer_1  →  BlendListByBool_0 (pose0)  →  DeadBlending_0
                          →  ApplyMeshSpaceAdditive_0 의 **Additive 입력**
```

즉 **애디티브 항**이지 베이스 포즈가 아니다.

> 다행히 **기존 문서는 원래 맞게 적혀 있었다** — `animation/2026-09-02_gasp_abp_analysis.md`
> 76행 · `IMPLEMENTED.md` 2.4절 둘 다 애디티브로 그려 두었다. **틀린 것은 문서가 아니라
> 이 세션의 중간 판단이었다.** 다른 문서를 고칠 필요는 없었고, 기록을 위해 여기 남긴다.

### 9.7 새로 생긴 노드 ID [A]

```
AnimGraphNode_IdentityPose_4 / _5 / _6
AnimGraphNode_SequenceEvaluator_0 / _1 / _2
AnimGraphNode_LayeredBoneBlend_2 / _3 / _4
AnimGraphNode_ApplyMeshSpaceAdditive_3 / _4 / _5
K2Node_VariableGet_3 / _4 / _5              (BF_AlphaL / R / U 게터)
```

---

## 10. 캐릭터 — `BP_SoldierCharacter.UpdateBlindFire()` [A]

새 함수 그래프. **Event Tick 끝**에서 `UpdateBodyYawRate` **다음**에 부른다.

```
reset       = InRange( BlindFireInputReset , 0.5 , 2.0 )

BlindFireH  = SelectFloat( RampAxisTo( BlindFireH , 0 , BlindFireResetRate , dt ) ,
                           StepAxis ( BlindFireH , BlindFireInputH , BlindFireRate , dt , −1 , 1 ) ,
                           reset )
BlindFireV  = SelectFloat( RampAxisTo( BlindFireV , 0 , BlindFireResetRate , dt ) ,
                           StepAxis ( BlindFireV , BlindFireInputV , BlindFireRate , dt ,  0 , 1 ) ,
                           reset )

ABP.BF_AlphaL = MapRangeClamped( BlindFireH , 0 , −1 , 0 , 1 )
ABP.BF_AlphaR = MapRangeClamped( BlindFireH , 0 , +1 , 0 , 1 )
ABP.BF_AlphaU = MapRangeClamped( BlindFireV , 0 , +1 , 0 , 1 )

HUD 행  "BF_H"  "BF_V"
```

### 10.1 ★ `MapRangeClamped`를 **반파 정류기로** 쓴다 [A]

부호 있는 축 하나(`−1..+1`)를 **좌/우 두 알파**로 갈라야 한다. 블루프린트 팔레트에 산술 노드가 없으므로(**P33**) 비교·분기 없이 이렇게 한다:

```
BF_AlphaL = MapRangeClamped( H , 0 , −1 , 0 , 1 )     H = −0.6  →  0.6      H = +0.6  →  0(클램프)
BF_AlphaR = MapRangeClamped( H , 0 , +1 , 0 , 1 )     H = +0.6  →  0.6      H = −0.6  →  0(클램프)
```

**입력 범위의 방향을 뒤집는 것만으로 음의 반파가 잘린다.** `Lerp`를 곱셈 대용으로 쓰는 것과 같은 계열의 요령이다.

### 10.2 왜 `StepAxis`/`RampAxisTo`인가 [A]

둘 다 **기존 C++** `USoldierAxisLibrary`(`Source/SoldierLab/Math/`)다. **빌드가 필요 없었다.**

- **등속 램프**라 목표에서 정확히 멈춘다. `FInterpTo`/`Lerp`는 지수형이라 **입력이 멈추면 목표로 붕괴**한다 — "떼면 그 값을 유지"라는 규약과 정면으로 어긋난다
- `StepAxis`는 입력 부호대로 상·하한 안에서 램프한다 → 축의 **유지** 의미가 여기서 나온다

### 10.3 새 변수 [A]

| 변수 | 종류 | 비고 |
|---|---|---|
| `BlindFireH` / `BlindFireV` | Float | 축 상태 |
| `BlindFireInputH` / `BlindFireInputV` / `BlindFireInputReset` | Float | 입력 액션이 매 프레임 써 넣는 값 |
| **`BlindFireRate`** | Float · **Instance Editable** | **1.0** — 0→1 에 1초 |
| **`BlindFireResetRate`** | Float · **Instance Editable** | **3.0** — 0으로 돌아가는 데 약 0.33초 |

ABP 쪽 새 변수: **`BF_AlphaL` · `BF_AlphaR` · `BF_AlphaU`** (Float).

> 리셋이 **3배 빠른** 이유: 자세를 잡는 것은 의도적이어야 하고, **푸는 것은 즉각적이어야** 한다.
> 2.5d절의 `WeaponLowerRate 8.0` / `WeaponRaiseRate 2.0` 비대칭과 같은 사고방식이다.

---

## 11. 입력 [A]

```
IA_BlindFireH        Axis1D
IA_BlindFireV        Axis1D
IA_BlindFireReset    Axis1D
```

**셋 다 `IA_Lean`을 복제해서 만들었다.** 이유가 두 가지다:

1. `AssetTools`에 **에셋 생성 함수가 없다**(13절) — 복제가 유일한 수단이다
2. `IA_Lean`은 **이미 트리거가 비워져 있다.** 앞서 기록한 **`InputTriggerPressed` 1프레임 버그**를 피하려고 정리해 둔 상태라, 복제하면 그 정리가 따라온다

**이벤트 배선은 `IA_Lean`과 동일**하다:

```
Triggered  →  Set <해당 Input 변수> = ActionValue
Completed  →  Set <해당 Input 변수> = 0
```

**`IMC_Sandbox` 매핑**:

| 키 | 액션 | 수정자 |
|---|---|---|
| **1** | `IA_BlindFireH` | **Negate** |
| **3** | `IA_BlindFireH` | — |
| **2** | `IA_BlindFireV` | — |
| **4** | `IA_BlindFireReset` | — |

> ⚠ **매핑은 손으로 넣어야 한다.** `ObjectTools.get_properties`가 `IMC_Sandbox`의
> `mappings`를 **`[]`로 반환**한다 — 실제로는 매핑이 들어 있는데도 그렇다. 배열이 이 API로
> 직렬화되지 않는다 (13절).

---

## 12. 판정

| 기준 | 결과 |
|---|---|
| ① 키를 누른 만큼 연속적으로 변한다 | ✅ |
| ② 떼면 그 자세를 유지한다 | ✅ (린 축과 같은 규약) |
| ③ 하체 로코모션이 흐트러지지 않는다 | ✅ `spine_01` 브랜치 필터가 골반·다리를 제외 |
| ④ 왼손이 총에서 떨어지지 않는다 | ✅ `TwoBoneIK_0`가 그래프 마지막이라 자동 |

**사용자 확인: "완전잘돼. 구현 완료."** → **성공.**

---

## 13. 툴링 — 이번에 확정된 것

`CLAUDE.md` 6.1절·6.3절에 반영했다. 요지만:

| 사실 | 영향 |
|---|---|
| **`AssetTools`에 에셋 *생성* 함수가 없다** — duplicate / move / delete / create_folder / save / find 뿐 | 새 에셋이 필요하면 **같은 클래스의 기존 에셋을 복제하고 프로퍼티를 고친다.** 입력 액션 3개를 이렇게 만들었다 |
| **`create_node`는 함수 이름이 처음 매치되는 클래스에 붙는다** | 같은 이름이 다른 클래스에도 있으면 *"This blueprint (self) is not a `<OtherClass>`_C, therefore 'Target' must have a connection."* → **`declaring_class`를 넘겨서** 지정한다 (ABP 자신의 `_C`) |
| **`find_node_types`는 멤버 이름에서 밑줄을 지운다** — `BF_AlphaL` → `Variables\|Default\|GetBFAlphaL` / `Class\|SoldierCharacterABP\|SetBFAlphaL` | 리더는 밑줄형(`\|SetBF_AlphaL`)을 찍는데 **그 이름으로는 생성이 안 된다.** ⚠ **파생 함정**: DSL 되읽기를 **문자열 카운트**로 검증하면 거짓 음성이 난다 → **`get_node_infos`로 노드 타입을 세라** |
| 애님 노드 타입 ID에 **에셋이 박힌다** — `Animation\|Sequences\|Evaluate'MM_Rifle_BlindFire_L'` (by-time) · `Animation\|Sequences\|Play'...'` | 정지 포즈는 **Evaluate + `ExplicitTime = 0`** |
| **`editor_toolset.toolsets.skeletalmesh.SkeletalMeshTools`가 이 빌드에 등록돼 있지 않다** | 본 목록을 이 경로로 못 얻는다. **Anim Outliner 스크린샷**이나 **컨트롤 리그의 계층**에서 읽을 것 |
| ★ **컨트롤 리그 에셋은 블루프린트로 열린다** — `BlueprintTools.list_variables` / `list_graphs` 동작 | **5절의 IK/FK 모드 변수를 이걸로 찾았다.** 웹 검색은 실패했다 → **P43** |
| `execute_tool_script` 안에서 **`unreal` 파이썬 모듈이 차단**돼 있다 | 허용 import: `re time math datetime copy json` (**`collections` 불가**) — 6.1b의 재확인 |
| **`ObjectTools.get_properties`가 Input Mapping Context의 `mappings`를 `[]`로 준다** | 매핑이 실제로 있어도 그렇다. **키 매핑은 에디터 수작업** |

---

## 14. 이것이 바꾸는 것

| 문서 | 절 | 어떻게 |
|---|---|---|
| `IMPLEMENTED.md` | 2.4 · **2.5e 신설** · 2.6 · 3 · 6 | 전체 평가 체인 정정(린 `ModifyBone` 누락) · 블라인드 파이어 축 전체 · 튜닝값 2개 · 변수/입력/함수 |
| `CLAUDE.md` | 5절 · 6.1 · **6.3절 신설** | **P41~P43** · 툴링 사실 8건 · **포즈 저작 경로와 막다른 길 4종** |
| `OPEN_ITEMS.md` | C · W | **[C-8] 해결**(형태가 바뀜) · **[C-78]** · **[W7]** · **[W8]** 신설 · [W6] 계측 축 추가 |
| `CURRENT_STATE.md` | 머리말 | 2026-09-12 추가분에 블라인드 파이어 한 줄 |

---

## 15. 남은 것

| ID | 항목 |
|---|---|
| **C-78** | **블라인드 파이어 × 총구 정렬 보정의 상호작용 미측정** [B] — BF 레이어는 `Get_AOValue`가 만든 조준 애디티브 **위**에 얹히므로 총구가 조준선에서 크게 벗어난다. 그런데 `AimCorrection` 적분기는 **총구 전방벡터**로 오차를 재므로, BF 자세를 "고쳐야 할 오차"로 학습할 수 있다. 그러면 **BF를 켤 때마다 보정이 클램프로 밀린다**(P40의 두 번째 평형점 · [C-76]과 같은 기구). **판정**: BF를 켠 채 `AimErr` / `AimCorr` 표시를 본다. 필요하면 `AimGain` 게이트에 **BF 알파 조건**을 추가한다 |
| **W7** | **린(lean) 축이 문서에 없다** — 구현돼 동작 중인데(`ModifyBone spine_01..05` · `IA_Lean` · Q/E) `IMPLEMENTED.md`에 절이 없다. 이 문서 9.5절의 체인이 그 존재를 처음 기록한 것이다. **소급 문서화 필요** |
| **W8** | **마스크 규약이 두 벌이다** — `LayeredBoneBlend_0`은 `BlendMask`(+`BM_LowReady` 에셋), 새 세 개는 `BranchFilter`. 하나로 통일하거나 **왜 다른지를 문서에 적을 것** |
| **W6** | 계측 정리에 **`BF_H` · `BF_V` 두 행이 추가**됐다 |
| — | **애디티브 3장은 각각 "끝점" 하나씩**이다. 중간 각도는 선형 보간이라 **좌↔우를 동시에 밀면**(현 배선상 불가능하지만) 겹칠 수 있다. 세 레이어가 직렬이므로 **순서 의존성**이 있다는 것만 알아 둘 것 [B] |
| — | 베이크가 **151키/5초**인데 프레임 0만 쓴다. 용량 문제는 아니지만 **1프레임으로 줄이면 의도가 분명해진다** |
