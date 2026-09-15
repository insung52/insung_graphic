# 적군 메시(enemy_T → new_enemy_T)를 SK_UEFN_Mannequin 에 올린다

2026-09-14~15 / **완료 — `BP_Soldier_Hostile` 이 `new_enemy_T` 로 동작, 사용자 PIE "플레이 잘 됨, 총 붙음"** / 디자인팀이 [Q42] A안대로 UE5 표준 리그로 다시 납품한 적군 메시를 아군(`2026-09-13_ally_mesh_on_mannequin_skeleton.md`)과 **같은 절차**로 마네킹 스켈레톤에 붙였다. 리스킨·리타깃·애니메이션 변경 없음. 납품은 **세 번** 있었고 셋째(`NewFolder/new_enemy_T`)를 채택 — 첫째는 스켈레톤 문제, 둘째는 Auto-Rig Pro 명명이라 기각(2.1절). `StanceStandZ/CrouchZ` 실측만 남았다 [C].

관련 항목: ~~[Q42]~~(해결) [W27] [W29] [W37] **[C-119]** / 관련 문서: `animation/prototypes/2026-09-13_ally_mesh_on_mannequin_skeleton.md`(선례 · 엔진 근거), `IMPLEMENTED.md` 3.2절(체크리스트), `animation/prototypes/2026-09-15_sharp_turn_pop_bf_head_scale_and_weapon_socket.md`(같은 날 후속 — 소켓 오프셋 흡수 · 머리 스케일)

> ⚠ 2절 표와 3절 절차는 **첫 납품 `enemy_T`** 기준으로 쓴 것이다. 실제 채택된 것은 **`new_enemy_T`** 이고 그 경위와 실측은 **2.1절 · 3.1절**에 있다. 원문은 판단 이력으로 남긴다(CLAUDE.md 3.2).

---

## 1. 무엇을 확인하려 했나

아군과 동일한 판정 기준:

1. `BP_Soldier_Hostile` 의 메시를 `enemy_T` 로 바꾼 뒤 **ABP·PSD·Chooser·BP 그래프를 하나도 안 고치고** 이동·조준·재장전·웅크림이 마네킹/아군과 같게 동작한다
2. 총이 오른손에 붙고 왼손 IK 가 총열을 잡는다
3. AI 가 쓰는 `head` · `spine_03` 본이 살아 있다
4. 아군(`BP_Soldier_Friendly`)과 마네킹은 **아무것도 바뀌지 않는다**

## 2. 실측 — 새 적군은 아군과 같은 리그다 [A]

MCP `SkeletalMeshTools` 로 세 메시를 기계적으로 대조했다 (P17).

| | 마네킹 `SKM_UEFN_Mannequin` | 아군 `SoldierLab/Characters/Ally/soldier_T` | **적군 `Soldiers/New_enemy_soldiers/enemy_T`** |
|---|---|---|---|
| 스켈레톤 (작업 전) | `SK_UEFN_Mannequin` | `SK_UEFN_Mannequin` (Assign 완료) | **`enemy_T_Skeleton`** (임포트 시 자동 생성, 참조 = enemy_T 뿐) |
| 메시 본 수 | 87 + 보조 8 | 87 | **87** |
| 본 이름 집합 | — | UE5 표준 | **아군과 완전히 동일** (`enemy − ally = ∅`, `ally − enemy = ∅`) |
| 마네킹에만 있는 본 | `attach` `weapon_l` `weapon_r` `props_root` `prop_01` `poi` `VB hand_l/r_prop_01` | 없음 | 없음 (아군과 동일) |
| `thigh_twist_02_l/r` | 스켈레톤에 있음 (아군 Assign 때 추가됨) | 부모 `thigh_l/r` | **부모 `thigh_l/r`** — 스켈레톤과 일치 |
| 부모 체인 (공통 85본) | 기준 | 동일 | **동일** (불일치 0건) |
| 바인드 포즈 · 키 | A-포즈 · 165.6 cm | T-포즈 · 181 cm | T-포즈 · **178.7 cm** (bounds z 89.37 × 2) |
| LOD / 버텍스 | 3 / 7,438 | 1 / 23,935 | **1 / 111,083** ⚠ |
| 머티리얼 슬롯 | `lambert1` | `Ch15_body` `Ch_49_body` `Ch_49_eyelashes` | `Ch_49_body2` `Ch_49_eyelashes` `Ch_49_body1` (슬롯 0 = body2) |
| 소켓 (작업 전) | `weapon_r` 외 5 | `Rifle_Socket` `weapon_r` 외 5 | **없음** |
| 피직스 에셋 | `PA_UEFN_Mannequin` | `soldier_T_PhysicsAsset` | `enemy_T_PhysicsAsset` (임포트 생성) |
| 참조하는 곳 | — | `BP_Soldier_Friendly` | **0건** (피직스·스켈레톤 자기 것뿐) — 아직 아무도 안 쓴다 |

결론: **enemy_T 는 소켓만 빼면 아군 soldier_T 와 같은 규격이다.** 옛 `SoldierLab/Characters/Enemy/Enemy`(Mixamo 65본, `Hips` 루트)와는 다른 물건이고, [Q42] 에서 "soldier_T 와 같은 규격"으로 요청한 것이 그대로 왔다.
Assign Skeleton 시 **새 본이 추가되지 않는다** — `SK_UEFN_Mannequin` 은 아군 때 이미 `thigh_twist_02_l/r` 를 받았고, 적군의 본은 전부 그 안에 있으며 부모도 같다. 즉 아군 때 치른 대가(GUID 재생성 · 전 클립 DDC 재압축)는 **이번엔 없다** [B — Assign 후 스켈레톤 dirty 여부로 확정].

⚠ 버텍스 111k · LOD 1개는 옛 `Enemy` 와 같은 수치다 — 디자인팀이 같은 소스 메시를 리그만 바꿔 다시 낸 것으로 보인다 (추정). 45명 배치 성능([C-6]/[W6] 계열)에는 이쪽이 더 무겁다. LOD 는 별건 → 6절.

### 2.1 세 번의 납품 — 첫째 스켈레톤 문제, 둘째 ARP 리그 기각, 셋째 채택 [A] (2026-09-14~15)

| 납품 | 경로 | 본 | 판정 |
|---|---|---|---|
| ① | `/Game/Soldiers/New_enemy_soldiers/enemy_T` | 87본, 아군 `soldier_T` 와 본 집합·부모 체인 완전 동일(위 표) | 스켈레톤에 문제가 있어 디자인팀이 다시 냈다(어떤 문제였는지는 이 세션 기록에 없다 [C]). 이 세션에서 MCP 로 `weapon_r` 소켓만 추가된 채 남아 있다(미저장) |
| ② | `/Game/Soldiers/New_enemy_soldiers/NewFolder/enemy_T` | **80본, Blender Auto-Rig Pro 명명** — `root_x` · `spine_01_x` · `arm_stretch_l` · `forearm_stretch_l` · `thigh_stretch_l` · `leg_stretch_l` · `c_subneck_1_x` · `toes_01_l` … | **기각.** 마네킹과 이름이 겹치는 본이 `root` · `hand_l/r` · `foot_l/r` **5개뿐**이고 부모도 다르다. `IsCompatibleMesh` 를 통과할 수 없고 Assign 하면 75본이 스켈레톤에 추가된다 — P76 의 "본 이름/부모 체인" 조건을 원천적으로 못 맞춘다 |
| ③ | **`/Game/Soldiers/New_enemy_soldiers/NewFolder/new_enemy_T`** | UE5 표준 명명 | **채택** → 3.1절 |

**디자인팀 안내용 본 호환 규칙**(이번 기각으로 정리한 것):

1. **이름·부모가 정확히 맞아야 하는 것** = `root` · `pelvis` · `spine_01~05` · `neck_01/02` · `head` · `clavicle` · `upperarm` · `lowerarm` · `hand` · `thigh` · `calf` · `foot` · `ball`(좌우) + 손가락 20개
2. **`ik_foot_root` · `ik_foot_l/r` 필요** — `LegIK`/`FootPlacement` 의 타깃. 없으면 발 IK 가 **조용히 꺼진다.** `ik_hand_*` 는 현재 사용 0건이나 넣을 것(마네킹 규격)
3. **twist 본은 애니메이션 호환에 불필요** — 단 **다른 이름의 twist 는 최악**이다(스켈레톤에 추가되고 아무 클립도 안 움직여 굳는다). 마네킹 이름 그대로 넣거나 아예 빼거나
4. 바인드 포즈(T/A) · 비율 · 메시 전용 추가 본은 **무관**(P76 — 같은 스켈레톤 에셋이면 리매핑이 항등)

## 3. 절차 (아군 2.3절과 동일 · 실제 수행 순서)

```
[MCP]    1. enemy_T 에 메시 소켓 weapon_r — 부모 hand_r · (0.19749, 3.411153, −0.381067) · rot 0   ✅ 완료 (2026-09-14, 미저장)
            ← 마네킹 weapon_r 본의 레스트 오프셋. 아군과 같은 값. SKM_UEFN_Mannequin 쪽 소켓과
              ABP TwoBoneIK_0 의 소켓 모드는 아군 때 이미 됐으므로 이번엔 메시 한쪽만 한다
[사용자] 2. enemy_T 스켈레탈 메시 에디터 → Asset → Assign Skeleton → SK_UEFN_Mannequin
            → 기대: 본 추가 0 · 스켈레톤은 dirty 되지 않음(2절). "Rebuilding animations" 가 돌면 그건 추가가 있었다는 뜻
[사용자] 3. BP_Soldier_Hostile
            CharacterMesh0.SkeletalMesh = enemy_T
            CharacterMesh0.Materials[0] 오버라이드 리셋   ← 지금 MI_UEFN_Mannequin_CMC 가 슬롯 0(Ch_49_body2)을 덮는다 (실측)
            StanceStandZ / StanceCrouchZ  ← 지금 89.7 / 39.4 (마네킹값). 시작값은 아군 77.1 / 36.4 로 두고 4에서 실측
            (P53: 컴포넌트/CDO 는 set_properties 가 못 쓴다 — 에디터에서)
[사용자] 4. PIE → HUD PelvOff 읽기 → 새 값 = 옛 값 − PelvOff (기립·웅크림 각각) → 3에 반영
            같이 볼 것: 팔 높이(축 규약) · 총 위치 · 왼손 IK · 팔꿈치 흉부 관통 · 아군/마네킹 무변화 · LogAnimation/LogSkeletalMesh 경고 0건
[사용자] 5. 저장 — enemy_T · BP_Soldier_Hostile (+ SK_UEFN_Mannequin 이 dirty 면 그것도). Perforce 체크아웃 먼저
```

### 3.1 실제 수행 — `new_enemy_T` 기준 [A] (2026-09-14~15)

위 절차의 대상만 셋째 납품으로 바뀌었고 순서는 같다. 1번(소켓)은 첫 납품 `enemy_T` 에 먼저 넣었다가 `new_enemy_T` 에 다시 넣었다.

```
[사용자] Assign Skeleton  new_enemy_T → SK_UEFN_Mannequin        ✅ 본 추가 0 (2절 예측대로 — 스켈레톤 무변경)
[실측]   new_enemy_T      111,083 버텍스 · LOD 1 · 키 178.7 cm
                          머티리얼 슬롯  Ch_49_body2 / Ch_49_eyelashes / Ch_49_body1
                          ⚠ 임포트 직후 세 슬롯 전부 WorldGridMaterial 이었다
[MCP]    SkeletalMeshTools.set_material  슬롯 → 상위 폴더 New_enemy_soldiers/ 의 기존 MI
                          Ch_49_body1 / Ch_49_body2 / Ch_49_eyelashes                     ✅
[MCP]    메시 소켓 weapon_r  부모 hand_r                                                    ✅
                          → 2026-09-15 에 컴포넌트 오프셋을 흡수해 (−9.80, 3.41, −0.38) · yaw 180 으로 바뀜
                            (후속 문서 3절 — 아군·마네킹도 같이)
[사용자] BP_Soldier_Hostile
           CharacterMesh0.SkeletalMesh = new_enemy_T                                        ✅
           머티리얼 오버라이드 3슬롯 None                                                    ✅ (MI_UEFN_Mannequin_CMC 가 슬롯 0 을 덮던 것 해제)
           StanceStandZ / StanceCrouchZ                                                    ⬜ 미실측 → [C-119] (아군 77.1 / 36.4 시작값 권고)
[사용자] PIE                                                                                ✅ "플레이 잘 됨, 총 붙음"
```

## 4. 결과 [A]

- `BP_Soldier_Hostile` 이 `new_enemy_T` 외형으로 **ABP·PSD·Chooser·BP 그래프 무수정**으로 이동·조준·재장전·웅크림 한다. 총이 `weapon_r` 소켓에 붙는다. 아군·마네킹 무변화(사용자 육안).
- `SK_UEFN_Mannequin` 에 본 추가 0 — 아군 때 치른 GUID 재생성/DDC 재압축이 **이번엔 없었다**(2절의 [B] → [A]).
- 임포트 머티리얼이 `WorldGridMaterial` 로 오는 것은 이 납품의 특성이다 — 다음 납품에서도 슬롯 확인이 먼저다.
- `StanceStandZ/CrouchZ` 는 **재지 않았다.** HUD `PelvOff` 가 0 에 정착하는지 아무도 안 봤다 → **[C-119]**. 키 178.7 cm(아군 181) 라 아군 값 77.1/36.4 근처일 것 [B].
- 같은 날 후속(2026-09-15): BF 시 머리 부풀기(BF 포즈에 구워진 마네킹 PP 스케일 — PP ABP 가 없는 이 메시에서 드러남)와 총 소켓 오프셋 흡수는 **후속 문서**에 있다. 적군 고유 문제가 아니라 아군과 공통이었다.

## 5. 판정

**성공** — 1절 기준 ①②④ 충족(사용자 PIE). ③(`head` · `spine_03` 이 AI 에 살아 있는가)은 본 집합이 아군과 동일하므로 성립한다고 보나 AI 오버레이로 따로 확인하진 않았다 [B]. `StanceZ` 실측은 [C-119].

## 6. 막힌 것 / 다음에 확인할 것

- **[C-119]** `BP_Soldier_Hostile.StanceStandZ/CrouchZ` 실측 — 아군과 같은 HUD 역산(`새 값 = 옛 값 − PelvOff`, 기립·웅크림 각각). 지금 값은 마네킹 89.7/39.4 인지 아군 시작값 77.1/36.4 인지도 미확인
- **경로** — 아군은 `SoldierLab/Characters/Ally/` 인데 적군은 titan 폴더 `Soldiers/New_enemy_soldiers/NewFolder/` 에 있다. [W37] 의 "SoldierLab 밖에 남은 우리 것" 항목이 하나 는다. 옮기면 리다이렉터 + P96 순서 문제라 **지금은 그대로 두고** 정리 세션에서 한 번에
- **삭제 후보 (전부 미삭제 · 사용자 확인 전) → [W29]**
  - 첫 납품 `Soldiers/New_enemy_soldiers/enemy_T` + `enemy_T_Skeleton` + `enemy_T_PhysicsAsset` — 참조가 자기 스켈레톤/피직스뿐. 이 세션이 넣은 소켓만 dirty 로 남아 있다(저장할 가치 없음)
  - 둘째 납품 `NewFolder/enemy_T`(ARP 리그) — 기각분
  - `NewFolder/new_enemy_T_Skeleton` — Assign 뒤 고아
  - 옛 Mixamo `SoldierLab/Characters/Enemy/Enemy` 메시·스켈레톤·피직스 3개(30 MB). ⚠ **`Characters/Enemy/Materials/` 는 지우면 안 된다** — 아군 `soldier_T` 가 `Materials/Eyelashes/Ch_49_eyelashes` 를 참조한다 ([W37])
- **`new_enemy_T` 의 피직스 에셋** — 바디 목록은 MCP 로 못 읽는다. 에디터에서 눈으로 ([W29] ②와 동일). 어느 피직스 에셋이 붙어 있는지도 미확인 [C]
- **LOD** — 111k 버텍스 · LOD 1개. 아군(24k)의 4.6배. 디자인팀에 LOD 3단 요청([Q42] 스펙에 이미 있음) 또는 에디터 자동 LOD
- **[W27]** — 메시가 하나 더 늘었으니 `StanceStandZ/CrouchZ` 자동 산출의 값어치가 올라간다
- **미저장** — `new_enemy_T` · `BP_Soldier_Hostile` · (`enemy_T` 옛것은 저장 불필요). Perforce 체크아웃은 사용자 몫
