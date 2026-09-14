# 아군 메시(soldier_T)를 SK_UEFN_Mannequin 에 올린다

2026-09-13 / **성공 (아군) · 미착수 (적군)** / soldier_T 를 재임포트 없이 마네킹 스켈레톤에 붙여 GASP/Lyra 애니메이션·ABP를 **무수정**으로 돌린다. 스켈레톤 호환(Compatible Skeletons) 경로는 엔진 소스로 기각. 총내림 포즈 재저작은 시도 후 **원복**.

관련 항목: **[Q8]** [Q42] [C-89] [C-90] [W27] [W28] [W29] / 관련 문서: `IMPLEMENTED.md` 2.5 · 3.2절, `CLAUDE.md` P76~P80 · 6.1

---

## 1. 무엇을 확인하려 했나

**목표**: 아군/적군 병사가 우리 캐릭터(GASP, `SK_UEFN_Mannequin`)의 애니메이션을 그대로 쓰면서 각자의 외형을 갖는다.
**금지**: PSD/스키마를 다른 스켈레톤용으로 다시 만들거나 애니메이션을 리타깃 복제하는 것. **메시를 스켈레톤에 맞춘다.**

판정 기준:
1. `BP_Soldier_Friendly` 의 메시를 `soldier_T` 로 바꾼 뒤 **ABP·PSD·Chooser·BP 를 하나도 안 고치고** 이동·조준·재장전·웅크림이 마네킹과 같게 동작한다
2. 총이 오른손에 붙고 왼손 IK가 총열을 잡는다
3. AI가 쓰는 `head` · `spine_03` 소켓이 살아 있다
4. 마네킹 캐릭터(`BP_Soldier_Hostile`)는 **아무것도 바뀌지 않는다**

## 2. 어떻게 했나

### 2.1 실측 — 본 목록 (MCP `SkeletalMeshTools.get_bone_names`) [A]

| | 마네킹 (93) | 아군 soldier_T (87) | 적군 Enemy (65) |
|---|---|---|---|
| 스켈레톤 | `SK_UEFN_Mannequin` | `soldier_T_Skeleton` | `Enemy_Skeleton` |
| 우리에만 있는 본 | `attach` `weapon_l` `weapon_r` `props_root` `prop_01` `poi` `VB hand_l_prop_01` `VB hand_r_prop_01` (8) | — | — |
| 상대에만 있는 본 | — | `thigh_twist_02_l/r` (2) | 전부(Mixamo 명명, `Hips` 루트, `root`/IK/트위스트 없음) |
| 공통 85본의 부모 관계 | 동일 | **동일** (`root→pelvis→spine_01~05`, `ik_foot_root`, `ik_hand_root→ik_hand_gun`) | 0 |
| 바인드 포즈 | **A-포즈** | **T-포즈** | T-포즈 |
| 키(바운즈 z×2) | 165.6 cm | 181 cm | 178 cm |
| LOD / 버텍스 | 3 / 7,438 | 1 / 23,935 | **1 / 111,083** |
| 원본 FBX | — | `titan_example/Content/Soldiers/New_Soldiers/soldier_T.fbx` (86본, Blender 로드 확인) | `D:/woonju jeong/.../Enemy.fbx` — **다른 PC, 로컬 없음** |

> ⚠ 작업 지시서에는 "우리에만 있는 8개 = spine_04, spine_05, attach 등"으로 적혀 있었으나 **spine_04/05 는 아군에 있다.** 없는 8개는 전부 스킨 없는 보조 본이다.

없는 8개 중 ABP/BP 가 실제로 쓰는 것 (`.uasset` 이름 테이블 조사) [A]: **`weapon_r` 뿐** — BP 의 `WeaponMesh` 부착 소켓 + ABP `TwoBoneIK_0` 의 effector. `weapon_l` `attach` `poi` `prop_01` `props_root` `ik_hand_gun` 은 0건.

### 2.2 "Compatible Skeletons" 경로가 안 되는 이유 — 엔진 소스 [A]

`UE_5.8/Engine/Source/Runtime/Engine`:

- `SkeletalMeshComponent.cpp:941-958` `NeedToSpawnAnimScriptInstance` — ABP 가 메시에 붙는 조건은 스켈레톤 에셋 동일성이 아니라 **`IsCompatibleMesh(mesh, false)` = 본 이름/부모 체인 검사**. 그래서 soldier_T 에는 **아무것도 안 해도 ABP 가 돈다.**
- 그러나 애니메이션 스켈레톤 ≠ 메시 스켈레톤이면 `SkeletonRemapping.cpp` 의 `FSkeletonRemapping` 이 개입하고, 이것은 회전을 **레스트 포즈로부터의 델타로 보존**한다 (`Lt = Q0·Ls·Q1`, 주석 *"delta rotations from the rest pose"*). 마네킹 A-포즈 ↔ soldier_T T-포즈 이므로 **모든 클립에서 팔이 T−A 만큼 들린다.** `SkeletonRemappingRegistry.cpp:67` — 리매핑은 호환 목록과 무관하게 이름으로 생성되므로 `compatibleSkeletons` 설정은 이 계산을 바꾸지 않는다.
- 같은 스켈레톤 에셋이면 리매핑이 `DefaultMapping`(항등) → 클립의 로컬 회전이 **절대값**으로 들어가 바인드 포즈 차이가 무관해진다. 이동은 `SK_UEFN_Mannequin` 본 트리의 `translationRetargetingMode` 가 처리 — 실측: **root/attach/weapon_l·r/ik_*/props_root/prop_01/poi 14본 `Animation` · 몸통 75본 `OrientAndScale` · `upperarm_twist_01_l/r` 2본 `Skeleton`**.

→ **정답은 "메시의 스켈레톤을 SK_UEFN_Mannequin 으로 바꾸는 것"** 이고, 남는 가정은 "관절 축 규약이 마네킹과 같은가"였다(→ 3절에서 [A] 로 확정).

### 2.3 절차 (실제 수행 순서)

```
[사용자] 1. soldier_T 스켈레탈 메시 에디터 → Asset → Assign Skeleton → SK_UEFN_Mannequin
            → SK_UEFN_Mannequin 본 트리 91 → 93 (thigh_twist_02_l/r 추가), GUID 재생성,
              "Rebuilding animations..." (로드된 클립 전부 ValidateSkeleton, DDC 재압축)
            → dirty: soldier_T, SK_UEFN_Mannequin.  FBX 재임포트 불필요.
[MCP]    2. SKM_UEFN_Mannequin 에 메시 소켓 weapon_r  — 부모 본 weapon_r · 항등
[MCP]    3. SoldierCharacter_ABP  TwoBoneIK_0.effectorTarget → bUseSocket=true, socketName=weapon_r → compile
[사용자] 4. SK_UEFN_Mannequin 스켈레톤 트리에서 weapon_r 본의 로컬 트랜스폼 읽기
            = Location (0.19749, 3.411153, −0.381067) · Rotation 0
[MCP]    5. soldier_T 에 메시 소켓 weapon_r — 부모 hand_r · 위 트랜스폼
[사용자] 6. BP_Soldier_Friendly (이미 존재하던 자식 BP) CharacterMesh0.SkeletalMesh = soldier_T
            Materials Element 0 오버라이드 리셋 (부모의 MI_UEFN_Mannequin_CMC 가 슬롯 0 = Ch15_body 를 덮고 있었다)
[사용자] 7. PIE → PelvOff ≈ +7 → BP_Soldier_Friendly  StanceStandZ 89.7 → 77.1 · StanceCrouchZ 39.4 → 36.4
[MCP]    8. TwoBoneIK_0  JointTargetLocation 핀 (0,0,0) → (−1, 1, 0)   ← 팔꿈치 흉부 관통 대책
[사용자] 9. 전부 저장 (soldier_T · SK_UEFN_Mannequin · SKM_UEFN_Mannequin · SoldierCharacter_ABP · BP_Soldier_Friendly · L_SoldierTest)
```

소켓이 본을 대신할 수 있는 근거 [A]: `SkinnedMeshComponent.cpp:3535-3548` `GetSocketTransform` 은 **소켓을 먼저 찾고 없을 때만 본**을 찾는다. 마네킹 쪽 소켓은 `weapon_r` **본에** 항등으로 붙어 있어 결과가 본과 같고 본의 애니메이션도 따라간다. 그래서 BP 의 부착 소켓 이름은 바꾸지 않았다. `FSocketReference::InitializeSocketInfo` (`BoneSocketReference.cpp:14`) 는 `DoesSocketExist` 로 찾으므로 **소켓 모드로 바꾸면 양쪽 메시 모두에 소켓이 있어야** 한다(2·5단계가 둘인 이유).

### 2.4 시도했다가 원복한 것 — 총내림 포즈 재저작

총내림(로우레디)이 아군에서 "골반에서 꺾이고 척추는 곧게" 보이는 원인을 그래프에서 읽었다 [A]:

```
BlendListByBool_0 (false = 총내림)
  → LayeredBoneBlend_0 : Base = IdentityPose · BlendPoses_0 = SequencePlayer_0(MM_Rifle_Idle_Hipfire_AO_CD,
                          1프레임 · AAT_RotationOffsetMeshSpace) · blendMask BM_LowReady
  → DeadBlending_0 → ApplyMeshSpaceAdditive_0 (Alpha 1.0 고정)
```

즉 총내림은 전용 포즈가 아니라 **힙파이어 AO 의 "정면·완전 아래" 포즈를 마스크로 잘라 100% 얹은 것**이다. 메시 공간 애디티브는 본마다 절대 방향에 독립적으로 델타를 더하므로 마스크 가중치가 큰 본(골반/spine_01)만 꺾이고 나머지 척추는 베이스의 직립 방향을 유지한다 — 마네킹은 상체가 짧아 봐줄 만했을 뿐이다.

6.3절 파이프라인으로 `MM_Rifle_LowReady` 를 저작해 BF 와 같은 형태(`SequenceEvaluator` + `BranchFilter spine_01`)로 갈아끼웠으나 **걷기에서 왼손·상체가 ADS 로코모션과 어긋나 사용자가 기각**, 원래 배선으로 **원복**했다(노드 이름 `SequencePlayer_0` 까지 동일). 에셋 `MM_Rifle_LowReady` 는 애디티브 설정(`AAT_RotationOffsetMeshSpace` / `ABPT_AnimFrame` / `MM_Rifle_Idle_ADS` 프레임 0)이 들어간 채 **참조 0건**으로 남아 있다.

저작 중 발견 [B]: **베이크는 스켈레톤의 원 메시(마네킹)로 해야 한다.** 시퀀서의 CharacterMesh0 를 `soldier_T` 로 두고 베이크하면 `spine_04→spine_05` 간격이 비정상적으로 늘어난 클립이 나왔고, 마네킹으로 바꿔 베이크하니 정상이었다. 컨트롤리그(`CR_Mannequin_Body`) 계층의 초기 트랜스폼이 마네킹 것이라 다른 체형에서 본 위치를 마네킹 값으로 써 버리는 것으로 추정.
또 하나 [A]: ABP 는 **프레임 0 만** 읽으므로(`ExplicitTime = 0`) 시퀀서에서 다른 시간에 키를 찍고 Replace 베이크하면 "시퀀서에선 보이는데 PIE 에선 안 바뀐다".

## 3. 결과

| 판정 항목 | 결과 |
|---|---|
| ABP·PSD·Chooser·BP 무수정으로 동작 | ✅ 이동·조준·재장전·웅크림 동작. 애니메이션 자산 변경 0건 |
| 관절 축 규약 (2.2의 남은 가정) | ✅ **[A]** 팔·다리·척추 전부 마네킹과 같은 방향으로 동작 |
| 총 부착 · 왼손 IK | ✅ 소켓 방식. 팔꿈치 흉부 관통은 `JointTargetLocation (−1,1,0)` 으로 해소 (사용자 확인) |
| `head` · `spine_03` | ✅ 존재 (아군 리그가 UE5 표준 명명) |
| 마네킹 무변화 | ✅ 사용자 확인. 소켓이 본에 항등으로 붙어 결과 동일 |
| 골반 높이 | ⚠ `StanceStandZ/CrouchZ` 가 **메시별 실측값**임이 드러났다 — `PelvOff +7` 로 골반이 매 프레임 들려 LegIK 가 다리를 최대로 폈다. `BP_Soldier_Friendly` 에 77.1 / 36.4 로 해결 (HUD 역산: 새 값 = 옛 값 − PelvOff) |
| 텍스처 | ⚠ 부모 BP 의 머티리얼 오버라이드(마네킹용, 슬롯 1개)가 아군 슬롯 0 을 덮었다. 자식 BP 에서 리셋으로 해결 |
| 총내림 자세 | ✗ 재저작 기각·원복 (2.4). 원래 방식 유지. 아군에서 더 어색해 보이는 것은 남아 있다 |
| PIE 로그 | `LogAnimation`/`LogSkeletalMesh` 경고 **0건** (두 세션) |

## 4. 판정

**아군: 성공.** 메시 하나를 갈아끼우는 데 필요한 것은 ① Assign Skeleton ② 소켓 2개 ③ IK effector 소켓 모드(1회) ④ 자식 BP 값 3개(머티리얼 리셋 · `StanceStandZ/CrouchZ`) 였다. 애니메이션 쪽은 무변경.

**대가**: `SK_UEFN_Mannequin` 에 `thigh_twist_02_l/r` 2본이 추가됐다(GUID 재생성, 전 클립 DDC 재압축 1회). PSD/스키마 자산은 바뀌지 않았다. 앞으로 디자인팀 캐릭터는 이 두 본을 가져도 그냥 임포트된다.

**남은 불만 (사용자)**: soldier_T 와 마네킹의 **비율 차이**가 튜닝값(골반 높이·팔꿈치 폴)을 메시별로 갈라놓았고 총내림 자세가 더 어색하다. 사용자가 원하는 것은 "마네킹 비율로 움직이는 아군" → [Q42] · [W28].

## 5. 이것이 바꾸는 것

| 문서 | 절 | 어떻게 바뀌나 |
|---|---|---|
| `IMPLEMENTED.md` | 2.5 · **3.2 신설** · 6 | 왼손 IK 소켓 모드 · 메시 교체 절차와 메시별 값 · 적군 미착수 |
| `CLAUDE.md` | 5 (P76~P80) · 6.1 | 리매핑 델타 규칙 · 실측값은 메시별 · PIE 모달 · 툴셋 정정 4건 |
| `OPEN_ITEMS.md` | Q8 · Q42 · C-89 · C-90 · W27~W29 | 아래 |
| `CURRENT_STATE.md` | 머리글 · 5절 | 메시 교체 완료 · 적군 결정 대기 |

- [x] 원 문서에 결과 반영
- [x] `OPEN_ITEMS.md` 등록
- [x] `CURRENT_STATE.md` 갱신

## 6. 막힌 것 / 다음에 확인할 것

- **[Q42]** 적군 메시 경로 — A) 디자인팀에 "UEFN 마네킹 리그 · 마네킹 비율로 피팅 · LOD" 스펙으로 요청 (권장) / B) Blender 리스킨(4~8h, 중품질) / C) 당분간 마네킹 유지. 원본 FBX 가 로컬에 없어 A 가 자연스럽다. 아군도 같은 스펙으로 다시 받으면 메시별 값이 사라진다
- **[W28]** 런타임 근사: `SK_UEFN_Mannequin` 몸통 본의 translation retargeting 을 `OrientAndScale → Animation` 으로 바꾸면 아군이 마네킹 뼈 길이로 움직인다 [B] — 5분 시험 후 판정. 마네킹 자신에게는 변화가 없어야 한다(우리 클립이 마네킹 비율로 저작/리타깃돼 있으므로) [B]
- **[W27]** `StanceStandZ/CrouchZ` 를 BeginPlay 에서 메시로부터 자동 산출 — 지금은 캐릭터 메시마다 HUD 역산으로 재야 한다
- **[C-89]** Lyra 클립이 `weapon_r` 본 자체를 움직이는가 — 아군 소켓은 `hand_r` 에 고정이라 그 움직임이 안 나온다. 재장전을 마네킹과 나란히 보면 판정된다
- **[C-90]** `MM_Rifle_LowReady` 를 살릴 수 있는가 — 기각 사유가 "걷기에서 왼손·상체가 어긋남"이었으므로, 총내림 idle 베이스(Hipfire)와 이동 베이스(ADS)가 다른 문제([C-55]의 잔재)와 얽혀 있다. 총내림 전용 로코모션 클립이 없는 한 애디티브 한 장으로는 idle 과 이동을 동시에 만족시키기 어렵다 [B]
- **[W29]** 고아가 된 `soldier_T_Skeleton` (참조 0건) 삭제. `soldier_T_PhysicsAsset` 바디를 에디터에서 눈으로 확인(MCP 로는 못 읽는다)
