# 급선회 끊김 · 맹목사격 머리 부풀기 · 무기 소켓 오프셋 흡수 · 왼손 그립 런타임 산출 · 총내림 전용 클립 2차 시험

2026-09-15 / **완료 (4건 성공 · 1건 실패·원복)** / 비조준 급선회 때 메시가 뚝 끊기던 것은 `OffsetRootBone.maxRotationError = 90` 이 원인이라 −1(GASP 원본)로 복귀 · 맹목사격 때 아군/적군 머리가 커지던 것은 **BF 포즈 3장에 마네킹 PP ABP 의 head ×1.15 가 구워져 있던 것**이라 `ModifyBone_8(head, 스케일 Replace 1, ComponentSpace)` 로 상쇄 · 총 부착 오프셋을 컴포넌트에서 **메시 소켓 `weapon_r` 로 흡수**해 애님 에디터 프리뷰와 런타임을 일치 · 왼손 그립 오프셋을 BeginPlay 에서 **총 메시의 `LeftHandGrip` 소켓으로 산출** · 총내림 전용 클립(`MM_Rifle_LowReady`) 로컬 애디티브 2차 시험은 **실패·전부 원복**. 전부 사용자 PIE 확인 [A].

관련 항목: **[C-80]** [C-74] [C-90] [W54] [W65] [W66] [W67] / 관련 문서: `IMPLEMENTED.md` 2.4 · 2.5 · 2.5e · 3절, `animation/prototypes/2026-09-11_sharp_turn_while_aiming.md`(90 을 넣은 경위), `animation/prototypes/2026-09-12_blind_fire_axis.md`(BF 포즈 저작), `animation/prototypes/2026-09-13_ally_mesh_on_mannequin_skeleton.md`(총내림 1차 시험 · 소켓 규약), `animation/prototypes/2026-09-14_enemy_mesh_on_mannequin_skeleton.md`(같은 날 적군 메시 교체)

> 신뢰도: 별도 표기가 없으면 **[A]** — MCP 로 노드/프로퍼티를 읽고 쓴 뒤 되읽었고, 거동은 사용자가 PIE 로 확인했다. 추정은 (추정) 또는 [B]/[C] 로 표시한다.

---

## 1. 급선회(비조준) 끊김 — `maxRotationError` 90 → −1 복귀 ([C-80] 확정)

### 1.1 증상

조준하지 않은 상태(`OrientToMovement`)에서 **A↔D 반전 · WASD 난타**를 하면 메시가 한 프레임에 뚝 끊긴다(스냅). 조준 중(`Strafe`)에는 없다. 마네킹 · 아군 `soldier_T` · 적군 `new_enemy_T` 전부 동일 — 즉 메시가 아니라 그래프의 문제.

### 1.2 원인

`OffsetRootBone_0.maxRotationError = 90`. 2026-09-11 에 조준 중 급선회 포즈 뒤집힘 대책으로 GASP 원본 −1(상한 없음)에서 90 으로 바꾼 값이다(`IMPLEMENTED.md` 2.4 표 · `2026-09-11_sharp_turn_while_aiming.md`).

```
비조준  캡슐 회전 = bOrientRotationToMovement, RotationRate 즉시(720°/s 이상) → 반전 입력에 캡슐이 180° 를 거의 즉시 돈다
        메시는 OffsetRootBone 이 rotationHalfLife 로 뒤따르므로 메시–캡슐 각도차가 순간적으로 90 을 넘는다
        → maxRotationError 클램프가 그 순간 각도차를 90 으로 잘라 메시를 "끌어당긴다" = 스냅
조준    캡슐이 유한 각속도(2.5d, YawRate_Up 90°/s)로 돌기 때문에 각도차가 90 에 도달하지 못한다 → 증상 없음
```

**90 을 넣은 이유(조준 중 뒤집힘)는 2026-09-12 의 유한 몸통 각속도(2.5d)가 이미 막고 있었다.** 즉 값 하나가 다른 대책으로 불필요해졌는데 그대로 남아 비조준 경로에서 부작용만 냈다. [C-80] 이 "자초한 회귀 후보"로 등록해 둔 바로 그 값이다 — 다만 [C-80] 이 예상한 증상(90° 제자리회전 미해소·재트리거)이 아니라 **반전 스냅**으로 나타났다.

### 1.3 조치

```
SoldierCharacter_ABP  AnimGraph  OffsetRootBone_0.maxRotationError   90 → −1   (GASP 원본 복귀)
compile_blueprint (P23)
```

### 1.4 검증

사용자 PIE: 비조준 반전 끊김 **사라짐** + 조준 중 급선회 뒤집힘 **재발 없음** → "성공". 마네킹/아군/적군 동일.

### 1.5 이것이 바꾸는 것

- [C-74] 결합 3개조 `maxRotationError 90 / Enable_AO 70 / WeaponLowerAngleFull 65` 에서 **90 항이 빠진다** — 천장이 없어졌으므로 `Enable_AO 70` 은 이제 "천장 안의 결정선"이 아니라 그냥 결정선이다. 남는 쌍은 `70 / 65`.
- `IMPLEMENTED.md` 2.4 의 "GASP 에서 바꾼 애님 노드 두 개" 표에서 `OffsetRootBone_0` 행이 원복된다 → 바꾼 노드는 `BlendListByBool_0` **하나**.
- 원칙 → `CLAUDE.md` **P124**(값 하나를 넣은 이유가 다른 대책으로 사라졌는지 확인).

---

## 2. 맹목사격 시 머리 부풀기 — BF 포즈에 구워진 PP 스케일을 `ModifyBone_8` 로 상쇄

### 2.1 증상

키 1/2/3 으로 블라인드 파이어를 켜면 **아군 `soldier_T` · 적군 `new_enemy_T` 의 머리가 커진다.** 마네킹(`BP_SoldierCharacter`)에서는 안 보인다.

### 2.2 원인 [A, 사용자 확인]

BF 포즈 3장(`Rifle/Poses/MM_Rifle_BlindFire_L/R/U`)은 시퀀서에서 **마네킹 메시로 베이크**했다(`CLAUDE.md` 6.3 · P80). 그때 마네킹 메시에 붙어 있는 포스트프로세스 ABP `ABP_UEFN_Mannequin_PostProcess` 의 결과까지 클립에 구워졌다:

```
ABP_UEFN_Mannequin_PostProcess (전부 BCS_ComponentSpace · Replace)
    head        scale 1.15
    thigh_l/r   scale (1, 1.12, 1.12)
    foot_l/r    scale 1
```

기준 클립 `MM_Rifle_Idle_ADS` 프레임 0 의 head 스케일은 1.0 이므로, 메시 공간 애디티브 델타 `(BF 포즈 − 기준)` 에 **head ×1.15 가 남는다.** 마네킹에서는 PP ABP 가 그래프 뒤에서 스케일을 **Replace** 하므로 델타가 가려졌고, PP ABP 가 없는 `soldier_T` / `new_enemy_T` 에서 그대로 드러났다.

같은 방식으로 베이크한 `MM_Rifle_LowReady` 에도 같은 것이 들어 있을 것이다 [B — 참조 0건이라 지금은 무해].

### 2.3 조치

재베이크는 **불가** — 포즈를 저작한 `/Game/NewLevelSequence` 가 이전 프로젝트/깨진 리그(`CR_Mannequin_Body`, 7절)에 의존한다. 대신 그래프 끝 체인에서 head 스케일을 1 로 되돌린다:

```
SoldierCharacter_ABP  AnimGraph  끝 체인
    … → TwoBoneIK_0 → ModifyBone_6(neck_01) → ModifyBone_9(neck_02) → ModifyBone_7(head)
      → ★ ModifyBone_8(head)          ← 이 세션
            scaleMode   = BMM_Replace · (1, 1, 1)
            scaleSpace  = BCS_ComponentSpace          ← ⚠ BoneSpace 면 no-op (2.5절)
            rotation / translation = BMM_Ignore
            alpha       = 1 고정
      → ComponentToLocalSpace_2
```

`ModifyBone_8` 은 다른 세션이 [W54] 에서 "전부 Ignore 인 잔여 노드, 삭제 권고"로 적어 둔 그 노드다 — 이 세션이 그 자리를 head 스케일 상쇄용으로 채웠다(추정: 같은 이름·같은 본·같은 위치). **[W54] 의 삭제 권고는 철회된다.**

마네킹은 PP ABP 가 뒤에서 1.15 를 다시 걸므로 **무변화**. `ModifyBone_6/7/9` 는 **다른 세션의 1인칭 머리 조준 추종**(변수 `HeadAimAlpha` · `NeckAimRotation` · `HeadAimRotation`, 회전 Additive WorldSpace — `IMPLEMENTED.md` 5.2 · `animation/2026-09-14_sight_alignment_plan.md`)이고 같은 ABP 를 **동시에** 편집 중이었다. 처음에 `ModifyBone_7` 에 스케일을 얹었다가 남의 노드라 **원복**하고 `_8` 을 썼다.

### 2.4 검증

사용자 PIE "잘됨" — 아군/적군 BF 시 머리 크기 정상, 마네킹 무변화.

### 2.5 함정 2건 (→ `CLAUDE.md` P121 · P122)

1. **`ModifyBone` 의 스케일 Replace 를 `BCS_BoneSpace` 로 걸면 no-op 다.** 본 자기 기준 공간에서 스케일 1 = 항등이라, 변환해 들어갔다가 그대로 곱해 나온다. 처음 BoneSpace 로 넣고 "안 바뀐다"로 한 번 헛돌았다. **ComponentSpace 로 걸 것.**
2. **시퀀서 Bake Animation Sequence 는 PP ABP 결과까지 굽는다.** 마네킹으로 베이크한 포즈에는 head 1.15 · thigh 1.12 가 들어간다. 베이크 전 시퀀서의 메시 컴포넌트에서 **`Disable Post Process Blueprint`** 를 켤 것. 이미 구운 클립은 [W65].

---

## 3. 무기 부착 오프셋을 메시 소켓에 흡수 — `WeaponMesh` 상대 트랜스폼 0

### 3.1 증상

애님 에디터에서 `weapon_r` 소켓에 Add Preview Asset 으로 총을 붙이면 **런타임 총 위치와 어긋난다.** 디자인팀이 왼손을 FK 로 총열에 맞추려면 프리뷰가 런타임과 같아야 한다.

### 3.2 원인

부착 체인이 두 단계였다:

```
(전)  hand_r → 메시 소켓 weapon_r (캐릭터 메시별 · 아군 (0.19749, 3.411153, −0.381067) rot 0)
            → WeaponMesh 컴포넌트   relLoc (−10, 0, 0) · relRot yaw 180        ← 프리뷰에는 없는 단계
            → BP_AR4Rifle (Snap, 0) → weaponMesh (0)
```

프리뷰는 소켓에 **직접** 붙으므로 `WeaponMesh` 컴포넌트의 오프셋만큼 어긋난다.

### 3.3 조치 (사용자)

세 메시의 `weapon_r` **메시 소켓**에 컴포넌트 오프셋을 흡수하고 컴포넌트는 0 으로:

| 메시 | `weapon_r` 소켓 (후) | 비고 |
|---|---|---|
| `SoldierLab/Characters/Ally/soldier_T` | 부모 `hand_r` · loc **(−9.80, 3.41, −0.38)** · yaw **180** | 계산값 = 옛 소켓 + 컴포넌트 오프셋 |
| `Soldiers/New_enemy_soldiers/NewFolder/new_enemy_T` | 부모 `hand_r` · 같은 값 | |
| `Characters/UEFN_Mannequin/Meshes/SKM_UEFN_Mannequin` | 부모 본 `weapon_r` · loc **(−10, 0, 0)** · yaw **180** | 본 기준이라 옛 항등 + 오프셋 |

```
BP_SoldierCharacter.WeaponMesh   relLoc (0,0,0) · relRot (0,0,0)     ← MCP 되읽기로 0 확인
```

메시 소켓은 스켈레탈 메시별이므로 **아군/적군이 다른 총·다른 소켓값을 가질 수 있다** — 갈라질 때의 규약은 [W67].

### 3.4 검증

사용자 PIE 확인. 프리뷰와 런타임 총 위치 일치(사용자 육안).

### 3.5 덤 — `soldier_T` 의 `Rifle_Socket`

titan 구 시스템(`BP_Ally_kadex`) 잔재다. SoldierLab 의 BP/ABP 는 `weapon_r` 만 참조한다(디스크 스캔). 무시하거나 지워도 된다. ⚠ titan 쪽 `/Game/Soldiers/New_Soldiers/soldier_T`(동명 다른 에셋, [W33])의 것은 **건드리지 말 것**.

### 3.6 `WeaponMesh` 컴포넌트의 실제 역할 (실측)

① `BP_AR4Rifle` 의 부착 앵커 ② BeginPlay 에서 `SetVisibility(false)` (구 GASP 무기 메시 숨김, P48) ③ **총구 보정(2.5c)이 `WeaponMesh.GetSocketTransform("Muzzle", World)` 를 읽는다.** 사망 시 드롭 용도가 아니다. 즉 `WeaponMesh` 의 스켈레탈 메시는 **스폰되는 총과 같은 메시**여야 하고(지금 `SK_AR4_X`), 그 메시에 `Muzzle` 소켓이 있어야 한다.

---

## 4. 왼손 IK 토글 + 그립 오프셋 런타임 산출

### 4.1 토글 `LeftHandIKEnabled` (2026-09-14, 핸드오프 5절 사양의 실행)

```
SoldierCharacter_ABP
    변수  LeftHandIKEnabled (Boolean)  기본 false        ← 디자인팀이 FK 로 왼손을 맞추는 기간 OFF
    TwoBoneIK_0.Alpha ← SelectFloat( A = GetCurveValue("DisableLHandIK")   (K2Node_CallFunction_1, 기존)
                                     B = 1.0
                                     bPickA = LeftHandIKEnabled )            (K2Node_CallFunction_5 · K2Node_VariableGet_7, 신규)
    alphaScaleBiasClamp scale −1 / bias +1 은 그대로 → OFF 면 1 − 1 = 0
```

캐릭터 BP 쪽 세터(핸드오프 5절의 `LeftHandIK` 변수)는 **만들지 않았다** — 당시 "다른 BP 의 변수 세터는 `create_node` 로 못 만든다"(P55)로 봤기 때문. 그런데 4.2 에서 `Class|SoldierCharacterABP|SetLeftHandGripOffset` 가 **`create_node` 로 만들어졌다** → P55 의 그 부분은 틀렸다(`CLAUDE.md` 6.1 정정). 세터가 필요해지면 같은 방법으로 만들면 된다.

### 4.2 그립 오프셋을 총 메시 소켓에서 산출 (2026-09-15)

`LeftHandGripOffset` 상수 `(−30, 8, 4)` 는 마네킹+`SK_Rifle` 시절 눈으로 맞춘 값이라 메시/총이 바뀌면 틀린다(P77). 총 메시의 `LeftHandGrip` 소켓 위치를 `weapon_r` 소켓 기준 로컬로 바꿔 시작 시 써 넣는다:

```
BP_SoldierCharacter  EventGraph  BeginPlay  총 스폰 체인
    SpawnActor BP_AR4Rifle → AttachActorToComponent(WeaponMesh, Snap)
    → SetVisibility(WeaponMesh, false) → SetActorEnableCollision(Rifle, false)
    → ★ Cast( Mesh.GetAnimInstance → SoldierCharacter_ABP )                     (K2Node_DynamicCast_3)
    → ★ ABP.LeftHandGripOffset = InverseTransformLocation(                      (K2Node_VariableSet_16)
              T        = Mesh.GetSocketTransform("weapon_r", RTS_World),
              Location = WeaponMesh.GetSocketLocation("LeftHandGrip") )
    → (기존) Rifle.SetOwningCharacter → …
    노드: K2Node_VariableGet_32(Mesh) · _33(WeaponMesh) · K2Node_CallFunction_71/72/73/74 · K2Node_DynamicCast_3 · K2Node_VariableSet_16
```

- 기존 상수 `(−30, 8, 4)` 는 **시작 시 덮어써진다**(변수 기본값은 남아 있으나 의미 없음).
- 조건: `WeaponMesh` 의 메시 = 스폰되는 총 메시(3.6 과 같은 규약) · 총 메시에 **`LeftHandGrip` 소켓** 필요. `SK_AR4_X` 소켓: `Muzzle` · `LeftHandGrip` · `sight`.
- `Variables|Character|GetMesh` 같은 **네이티브 프로퍼티 게터도 `find_node_types` 에 나온다** → `CLAUDE.md` 6.1.

### 4.3 검증

사용자 PIE "잘됨" — `LeftHandIKEnabled = true` 로 켜고 왼손이 `LeftHandGrip` 소켓에 앉는 것 확인. 기본은 여전히 false.

---

## 5. 총내림 전용 클립 2차 시험 — 실패 · 전부 원복 ([C-90] 갱신)

### 5.1 무엇을 확인하려 했나

1차(2026-09-13, `2026-09-13_ally_mesh_on_mannequin_skeleton.md`)는 `MM_Rifle_LowReady` 를 메시 공간 애디티브로 얹어 걷기에서 어긋나 기각했다. 이번엔 **로컬 공간 애디티브**로 바꾸면 로코모션 위에 얹혀도 자연스러운지 본다. 판정: 정지·걷기·조깅에서 총내림 자세가 1차보다 낫고 왼손/팔꿈치가 깨지지 않으면 성공.

### 5.2 어떻게 했나

```
MM_Rifle_LowReady.additiveAnimType   메시 공간(1차 시험 값) → AAT_LocalSpaceBase
AnimGraph  ApplyMeshSpaceAdditive_0 와 [BF_L] 사이에 삽입
    SequenceEvaluator(MM_Rifle_LowReady, t=0) → LayeredBoneBlend(BlendMask BM_LowReady) → ApplyAdditive(로컬)
    ApplyAdditive.Alpha = AOActive ? 0 : 1
    기존 LayeredBoneBlend_0.BlendWeights_0 = 0   (기존 힙파이어 델타 끔)
```

### 5.3 결과

나쁨. 정지에서도 자세가 어색하고 걷기에서 팔이 깨짐. **왼손 IK 도 꺼져 있어(4.1, 기본 OFF) 왼손이 비정상** — IK 가 켜져 있던 1차보다 더 나쁘게 보였을 수 있다 [B]. 전부 원복: 새 노드 6개 삭제 · `LayeredBoneBlend_0.BlendWeights_0 = 1` · `additiveAnimType` 메시 공간으로. `MM_Rifle_LowReady` 는 값은 원복됐고 dirty 만 남아 있다.

### 5.4 판정 · 결론 (사용자 합의)

**실패.** 이동 중 총내림은 **총 내린 로코모션 클립이 없는 한 델타일 수밖에 없고, 델타는 작을 때만 자연스럽다.** 현행 힙파이어 델타 `AO_CD − AO_CC`(어깨 피치뿐 · 팔꿈치/손 ≈ 항등)가 되는 이유가 그것이다. 저작 포즈(팔꿈치·손까지 바뀜)는 메시/로컬 어느 공간이든 걷기 베이스 위에서 깨진다 → `CLAUDE.md` **P123**.

향후 두 갈래(둘 다 미실행):
1. **정지 총내림**은 `MM_Rifle_Idle_Hipfire`(베이스) 직접 편집으로 커버 — 델타를 `Moving` 에만 게이트하는 안.
2. **디자인팀에 총내림 로코모션 클립 요청** — 최소 Walk/Jog 루프 8장, 제대로 하려면 Start/Stop/Pivot 까지 32장 → [W66]. 시점 추후.

현행(힙파이어 델타 + `BM_LowReady`) 유지.

---

## 6. 이것이 바꾸는 것

| 문서 | 절 | 어떻게 |
|---|---|---|
| `IMPLEMENTED.md` | 2.4 | `maxRotationError` 표 정정(−1 복귀) · 끝 체인에 ModifyBone_6/7/9(타 세션)·_8 추가 |
| `IMPLEMENTED.md` | 2.5 · 2.5e | `LeftHandIKEnabled` · 그립 오프셋 런타임 산출 · BF 포즈 스케일 함정 + `ModifyBone_8` |
| `IMPLEMENTED.md` | 3 · 3.2 · 6 | `WeaponMesh` 상대 0 · 소켓 흡수 · 역할 3가지 · 적군 완료 · 새 미해결 |
| `OPEN_ITEMS.md` | [C-80] 해결 · [C-74] · [C-90] · [W54] 갱신 · [W65] [W66] [W67] 신규 | |
| `CLAUDE.md` | 5절 P121~P126 · 6.1 P55 정정 | |
| `assets/2026-09-14_design_team_animation_handoff.md` | 2.3 · 5 · 6 | 총내림 결론 · IK 토글 완료 |

- [x] 원 문서에 결과 반영
- [x] `OPEN_ITEMS.md` 등록/해결 표시
- [x] `CURRENT_STATE.md` 갱신

## 7. 막힌 것 / 다음에 확인할 것

- **[W65]** BF 3장 + `MM_Rifle_LowReady` 의 head 1.15 · thigh 1.12 재베이크 — 시퀀서 `NewLevelSequence` 가 깨진 리그에 의존해 지금은 불가. 재저작 경로가 생기면 `Disable Post Process Blueprint` 켜고 다시 굽고 `ModifyBone_8` 을 뗀다. thigh 1.12 는 지금 **상쇄하지 않았다** — 허벅지가 두꺼워 보이는지 아무도 안 봤다 [C]
- **[W66]** 총내림 로코모션 클립 요청(8 or 32장)
- **[W67]** 아군/적군 총기가 갈라질 때 `WeaponMesh` 메시 = 스폰 총 메시 · `Muzzle`/`LeftHandGrip` 소켓 규약
- **[Q48]** `/MoverExamples/.../CR_Mannequin_Body` 컴파일 에러 + `/Game/NewLevelSequence`(포즈 저작 스크래치, `L_SoldierTest` 가 참조) 처분 — 시퀀스+레벨 액터 삭제 후 [W34] 의 Mover 계열 플러그인 끄기 제안, 미결정
- **[W64]** PIE 로그 `LogAbilitySystem: Error: SendGameplayEventToActor … GameplayEvent.ReloadDone` 병사마다 반복 — 재장전 몽타주의 GASP 노티파이가 GAS 이벤트를 보내는데 우리 캐릭터에 ASC 가 없다. 무해. 노티파이를 떼면 조용해진다
- 미저장 자산(사용자 저장/체크아웃): `SoldierCharacter_ABP` · `BP_SoldierCharacter` · `new_enemy_T` · `enemy_T`(옛것, 소켓만 추가됨) · `MM_Rifle_LowReady`(값 원복, dirty 만)
