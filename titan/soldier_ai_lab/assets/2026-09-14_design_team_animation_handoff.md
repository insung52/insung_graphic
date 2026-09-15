# 디자인팀 애니메이션 핸드오프 — 실사용 시퀀스 목록과 "시퀀스로 못 고치는 것"

2026-09-14 / **완료 (2026-09-15 갱신 — 디자이너 전달본은 `2026-09-14_designer_guide.html`)** / 새 병사 이동 시스템(GASP MM + Lyra 라이플)이 **지금 실제로 재생하는** 애님 시퀀스 전체 목록(**103개** + 몽타주 2 + 묶음 3), 수정 규칙, 그리고 시퀀스가 아니라 **본을 직접 돌리는 절차 층**의 목록(디자인팀이 시퀀스를 고쳐도 안 바뀌는 것). 왼손 IK 는 토글(`LeftHandIKEnabled`, 기본 OFF)로 전환 완료(5절).

관련 항목: [W7] [W8] [W18] [C-90] [W65] [W66] **[C-120]** / 관련 문서: `IMPLEMENTED.md` 2절(전체 배선), `animation/prototypes/2026-09-04_c34_clip_curve_mapping.md`(커브 규칙), `migration/2026-09-14_asset_cleanup.md`, **`2026-09-14_designer_guide.html`**(디자이너에게 그대로 주는 완성본 — 부록에 103개 전체 경로, 디스크 존재 검증 완료), `2026-09-14_designer_guide_draft.md`(그 초안), `animation/prototypes/2026-09-15_sharp_turn_pop_bf_head_scale_and_weapon_socket.md`(총내림 2차 시험 · 머리 스케일 · 급선회 결론)

> 실측 방법 [A]: `SoldierCharacter_ABP` · `BP_SoldierCharacter` · `CHT_Soldier_Databases` · PSD 17개 · AO 2개 · 몽타주 2개 · 애디티브 포즈의 `.uasset` 참조 테이블을 바이트 스캔(2026-09-14). P95(`_숫자` FName 분할)는 TurnInPlace 8개에서 실제로 걸렸고 디스크 대조로 복원했다. 엔진 레지스트리(`get_dependencies`) 재확인은 PIE 가 꺼진 뒤 → 6절.

---

## 1. 한 장 요약 — 디자인팀에게

```
고쳐도 되는 것 (시퀀스, 총 ~~100개~~ 103개)  2절 표. 이름·경로·길이(루프)·커브를 지키면 그대로 반영된다
                                        (로코 61 · AO 30 · 총내림 델타 2 · BF 3 · Actions 4 · GASP 뱅킹 포즈 3
                                         + 몽타주 2 + 묶음 3(AO_Rifle_ADS · AO_Rifle_Crouch · BS1D_Additive_Lean_Run) — 7절 정정)
고쳐도 안 바뀌는 것 (절차 층, 8가지)    2절 표. 본을 코드가 직접 움직인다 — 여기 문제는 우리에게 보고
반입만 되고 안 쓰는 것                  Actions/ 의 ~~HitReact 13 · Death 6~~(→ 2026-09-15 배선됨, 2.8절) · Dash · Equip · Melee · Grenade · Pistol/Shotgun, Rifle/_Extra 12, AO_Rifle_Aim, MM_Rifle_LowReady, *_OverridePose
```

> ★ **2026-09-15**: 피격 13 · 사망 6 몽타주가 배선됐다 — "고쳐도 되는 것"이 **100 → 119개**. 2.8절.

**왼손 IK 는 기본 OFF 로 전환한다** (토글 변수 `LeftHandIKEnabled`, 3.6절). 시퀀스에서 왼손이 총열에 정확히 붙도록 FK 로 고치는 것이 디자인팀의 몫이고, 우리 쪽에서 언제든 다시 켤 수 있다.

---

## 2. 실사용 애님 시퀀스 — 역할별 [A]

전부 `/Game/SoldierLab/Animations/` 아래. 스켈레톤은 전부 `SK_UEFN_Mannequin`.

### 2.1 로코모션 — Motion Matching 이 고르는 61개

MM 은 매 프레임 이 61개 중에서 **포즈 유사도 + 궤적**으로 프레임을 고른다. 클립 하나를 고치면 그 클립이 골라지는 순간의 모습만 바뀐다 — 단, **속도·루프 길이·루트모션을 바꾸면 선택 자체가 바뀐다**(3.2절).

| DB (`PoseSearch/Rifle/`) | 클립 (`Animations/Rifle/`) | 수 |
|---|---|---|
| `PSD_Rifle_Stand_Idles` | `Idles/MM_Rifle_Idle_ADS` | 1 |
| `PSD_Rifle_Stand_Idles_LowReady` | `Idles/MM_Rifle_Idle_Hipfire` · `MM_Rifle_IdleBreak_Fidget` · `MM_Rifle_IdleBreak_Scan` | 3 |
| `PSD_Rifle_Stand_TurnInPlace` | `TurnInPlace/MM_Rifle_TurnLeft_90` · `_180` · `TurnRight_90` · `_180` | 4 |
| `PSD_Rifle_Stand_Walk_Loops` | `Loops/MM_Rifle_Walk_{Fwd,Bwd,Left,Right}` | 4 |
| `PSD_Rifle_Stand_Walk_Starts` | `Starts/MM_Rifle_Walk_{Fwd,Bwd,Left,Right}_Start` | 4 |
| `PSD_Rifle_Stand_Walk_Stops` | `Stops/MM_Rifle_Walk_{…}_Stop` | 4 |
| `PSD_Rifle_Stand_Walk_Pivots` | `Pivots/MM_Rifle_Walk_{…}_Pivot` | 4 |
| `PSD_Rifle_Stand_Jog_Loops` | `Loops/MM_Rifle_Jog_{…}` | 4 |
| `PSD_Rifle_Stand_Jog_Starts` | `Starts/MM_Rifle_Jog_{…}_Start` | 4 |
| `PSD_Rifle_Stand_Jog_Stops` | `Stops/MM_Rifle_Jog_{…}_Stop` | 4 |
| `PSD_Rifle_Stand_Jog_Pivots` | `Pivots/MM_Rifle_Jog_{…}_Pivot` | 4 |
| `PSD_Rifle_Crouch_Idles` | `Idles/MM_Rifle_Crouch_Idle` | 1 |
| `PSD_Rifle_Crouch_TurnInPlace` | `TurnInPlace/MM_Rifle_Crouch_Turn{Left,Right}_{90,180}` | 4 |
| `PSD_Rifle_Crouch_Walk_Loops` | `Loops/MM_Rifle_Crouch_Walk_{…}` | 4 |
| `PSD_Rifle_Crouch_Walk_Starts` | `Starts/MM_Rifle_Crouch_Walk_{…}_Start` | 4 |
| `PSD_Rifle_Crouch_Walk_Stops` | `Stops/MM_Rifle_Crouch_Walk_{…}_Stop` | 4 |
| `PSD_Rifle_Crouch_Walk_Pivots` | `Pivots/MM_Rifle_Crouch_Walk_{…}_Pivot` | 4 |

DB 선택은 `PoseSearch/CHT_Soldier_Databases`(Stance × MovementState × Gait × RotationMode → DB). 웅크려 달리는 클립은 없다 — 웅크림은 Walk 뿐.

### 2.2 조준 오프셋 — 포즈 30장 + 기준 2장

`AimOffset` 블렌드스페이스가 **한 프레임짜리 애디티브 포즈**를 Yaw × Pitch 로 섞는다. 각 포즈는 `(포즈 − 기준 클립 프레임 0)` 이므로 **기준 클립의 프레임 0 을 고치면 15장이 전부 같이 움직인다.**

| 에셋 | 포즈 (`Rifle/Poses/`) | 기준 (`ABPT_AnimFrame`, frame 0) |
|---|---|---|
| `Rifle/AO_Rifle_ADS` (기립·견착) | `MM_Rifle_Idle_ADS_AO_{CC,CU,CD,LC,LU,LD,RC,RU,RD,LBC,LBU,LBD,RBC,RBU,RBD}` 15 | `Idles/MM_Rifle_Idle_ADS` |
| `Rifle/AO_Rifle_Crouch` (웅크림·견착) | `MM_Rifle_Crouch_Idle_AO_{…}` 15 | `Idles/MM_Rifle_Crouch_Idle` |

포즈 이름: C/L/R/LB/RB = Yaw 0/−90/+90/−180/+180 · C/U/D = Pitch 0/+/−. ⚠ pitch 표본이 3장뿐이라 총구가 조준선에서 벗어나고 그것을 **총구 보정 루프(2절 ④)** 가 메운다 — 포즈를 정확히 고치면 보정량이 자동으로 줄어든다.

### 2.3 총내림(로우레디) — 전용 클립이 **없다** ★

총내림은 별도 포즈 클립이 아니다. 아래 둘의 합성이다 (`IMPLEMENTED.md` 2.4 · `animation/prototypes/2026-09-13_ally_mesh_on_mannequin_skeleton.md` 2.4절):

```
베이스   Chooser 가 RotationMode ≠ Strafe 일 때 PSD_Rifle_Stand_Idles_LowReady 를 고른다
         → MM_Rifle_Idle_Hipfire (+ IdleBreak 2) 가 재생된다             ← 시퀀스 ✅ 고칠 수 있다
애디티브  LayeredBoneBlend_0 (blendMode BlendMask · SK_UEFN_Mannequin:BM_LowReady)
         ← SequencePlayer_0 = Rifle/Poses/MM_Rifle_Idle_Hipfire_AO_CD   (1프레임, AAT_RotationOffsetMeshSpace)
            기준 = Rifle/Poses/MM_Rifle_Idle_Hipfire_AO_CC frame 0      ← 시퀀스 ✅ 고칠 수 있다
         → DeadBlending_0 → ApplyMeshSpaceAdditive_0 (Alpha 1.0 고정)
갈래     BlendListByBool_0 ← Enable_AO (false = 총내림). 켤 때 0.375 s · 끌 때 0.25 s
```

즉 "힙파이어 idle 의 **정면·완전 아래** 조준 포즈"를 **마스크(`BM_LowReady`, 골반/spine_01 가중치 큼)로 잘라 100% 얹은 것**이다. 디자인팀이 만질 수 있는 자리는 ① `MM_Rifle_Idle_Hipfire`(베이스) ② `MM_Rifle_Idle_Hipfire_AO_CD` / `_CC`(델타의 두 끝) ③ `BM_LowReady` 블렌드 마스크(스켈레톤 에셋 안, 본별 가중치). 이동 중 총내림은 **베이스가 ADS 로코모션 클립**이라 같은 애디티브가 그 위에 얹힌다 — 그래서 idle 과 이동을 동시에 만족하는 전용 포즈 한 장(`MM_Rifle_LowReady`, 참조 0건)이 기각됐다 → [C-90].

> ⚠ 시퀀스 말고 **캐릭터 쪽 값**이 둘 더 걸려 있다 — `Enable_AO` 문턱 70° (이 각을 넘으면 총을 내린다) · `WeaponLowered` 램프(내리기 0.125 s / 올리기 0.5 s, 몸통 각속도 90↔720 °/s 를 정한다). 이건 2절 ⑤.

### 2.4 블라인드 파이어 — 베이크한 포즈 3장

| 포즈 (`Rifle/Poses/`) | 기준 | 마스크 | 구동 |
|---|---|---|---|
| `MM_Rifle_BlindFire_L` · `_R` · `_U` | `Idles/MM_Rifle_Idle_ADS` frame 0 (`ABPT_AnimFrame`) | `BranchFilter spine_01` (척추·팔·목·머리, 다리 제외) | `BF_AlphaL/R/U` 0..1 (키 1/2/3, 리셋 4) |

**시퀀스 ✅ 고칠 수 있다** — 150프레임짜리지만 **프레임 0 만 읽는다**(`SequenceEvaluator · ExplicitTime 0`). 프레임 0 에만 키를 찍을 것. 기준이 `MM_Rifle_Idle_ADS` 프레임 0 이므로 **ADS idle 을 고치면 BF 3장이 같이 틀어진다.**

### 2.5 사격 · 재장전 — 몽타주 2개

| 몽타주 (`Actions/`) | 세그먼트 | 슬롯 | 호출 |
|---|---|---|---|
| `AM_MM_Rifle_Fire` | `Actions/MM_Rifle_Fire` (애디티브, 기준 `Actions/MM_Rifle_Idle_Hipfire`) | `FullBodyAdditivePreAim` | 탄이 실제로 나간 뒤 `OnWeaponFired` (P49) |
| `AM_MM_Rifle_Reload` | `Actions/MM_Rifle_Reload` + `MM_Rifle_Reload_Additive` | `UpperBody` + `UpperBodyAdditive` ⚠ **같은 슬롯 그룹이어야 한다**(P24) | `IA_Reload` / AI 재장전 |

⚠ `Actions/MM_Rifle_Idle_ADS` · `Actions/MM_Rifle_Idle_Hipfire` 는 `Rifle/Idles/` 의 것과 **다른 에셋**(Lyra 원본 애디티브 기준용 사본)이다. 재장전 중 왼손 IK 를 끄는 판정은 `MM_Rifle_Reload` 의 **`DisableLHandIK` 커브**가 한다(3.6절).

### 2.6 GASP 원본에서 그대로 쓰는 것 (건드리지 않는다)

`/Game/Characters/UEFN_Mannequin/Animations/Poses/BS1D_Additive_Lean_Run`(이동 뱅킹 애디티브) · `AimOffset/AO_Blend_Curve` · `ExperimentalStateMachineData/StrafeOffsetCurveContainer` · `MotionMatchingData/Schemas/PSS_Default·PSS_Idle·PSS_Stop`. 그리고 `Animation/CHT_Soldier_CharacterAnimations`(GASP 비무장 chooser 복제본, UEFN 클립 387개 참조) — **우리 병사는 라이플 세트만 쓰므로 실사용 없음 [B]**(ABP 가 참조만 유지).

### 2.7 반입만 되고 배선 없는 것 (고쳐도 아무 데도 안 나온다)

`Actions/`: ~~`HitReact` 13 · `Death` 6~~(→ **2026-09-15 배선됨, 2.8절**) · `Dash` 5 · `Rifle_Equip/DryFire/Melee/GrenadeToss/Spawn*` · `Pistol_*` · `Shotgun_*` 및 그 몽타주 · `Reload_Emote_MW`. `Rifle/_Extra/` 12(Crouch_Entry/Exit · Jump · Lean 클립). `Rifle/AO_Rifle_Aim`(구 힙파이어 AO). `Rifle/Poses/MM_Rifle_LowReady` · `MM_Rifle_Crouch_OverridePose` · `MM_Rifle_Hipfire_OverridePose` · `MM_Rifle_Idle_Hipfire_AO_*` 나머지 13장. ~~피격/사망은 **[W18]** 에서 배선 예정 — 그때 목록이 는다.~~ → 늘었다(2.8절).

### 2.8 피격 · 사망 — 몽타주 19개 (2026-09-15 배선) [A]

`Actions/AM_MM_HitReact_*` 13 · `Actions/AM_MM_Death_*` 6. 재생은 C++ `USoldierHealthComponent`(`ai/2026-09-15_health_hit_death_implementation.md`)가 하고, **클립·몽타주는 시퀀스 ✅ 고칠 수 있다** — 3.x 규칙(이름·경로 유지 · 리임포트 · 커브 유지)만 지키면 그대로 반영된다. 어느 클립을 쓸지는 컴포넌트 프로퍼티(BP 에서 교체 가능)이므로 **클립을 추가·교체하려면 우리에게 말할 것**(3.8 과 같은 이유).

**어느 클립이 언제 나오나** — 방향 × 세기. 방향은 탄이 *날아온* 쪽(병사 기준 앞/뒤/좌/우), 세기는 부위 배율을 곱한 데미지(소총 34 기준: 머리 85 · 목 68 → Heavy / 몸통 34 · 다리 25.5 → Medium / 팔 20.4 → Light). 같은 칸에 여러 개면 랜덤, 빈 칸은 한 단계 가벼운 쪽.

| | Light (< 25) | Medium (25~60) | Heavy (≥ 60) | 길이 |
|---|---|---|---|---|
| **Front** | `HitReact_Front_Lgt_01·02·03·04` | `Front_Med_01·02` | `Front_Hvy_01` | 0.70~0.80 s |
| **Back** | `Back_Lgt_01` | `Back_Med_01` | (없음 → Medium) | |
| **Left** | `Left_Lgt_01` | `Left_Med_01` (세그먼트 0~0.8 s) | (없음 → Medium) | 최장 1.33 s (`Right_Lgt`) |
| **Right** | `Right_Lgt_01` | `Right_Med_01` | (없음 → Medium) | |
| **사망** | `Death_Front_01·02·03` / `Death_Back_01` / `Death_Left_01` / `Death_Right_01` — 방향만, 세기 무관 | | | 0.93~1.13 s |

```
피격   HitReact 13 = 전부 로컬공간 애디티브(AAT_LocalSpaceBase, 기준 = 자기 f0 · Back_Lgt_01 만 Actions/MM_Rifle_Idle_ADS f0)
       슬롯 'AdditiveHitReact' → ABP 의 ApplyAdditive_0 (재장전 애디티브 ApplyAdditive_1 바로 뒤, DefaultSlot 앞)
       → 이동·조준·린·블라인드파이어 위에 *얹힌다*. 걷는 중 맞으면 다리는 계속 걷는다
       몽타주 blendIn 0/0.06 · blendOut 0.5 (Right_Med 0.6). 재장전·사격 몽타주를 끊지 않는다
사망   Death 6 = 전신 포즈(비애디티브). 슬롯 'DefaultSlot' → 상류(MM·AO·BF·재장전) 전부 덮는다
       몽타주 blendIn 0.06~0.10 · blendOut 0 · 세그먼트가 앞뒤로 잘려 있다(Front_01: 1.10 s 클립의 0.15~1.00)
       끝나기 0.1 s 전에 래그돌(물리)로 넘어간다 — 클립의 마지막 프레임이 곧 물리의 첫 자세
```

디자인팀이 만질 때 알아 둘 것: ① 피격 클립에 **다리/골반 델타**를 넣으면 걷는 중 발이 미끄러진다([C-113] 미측정) ② 사망 클립의 **마지막 프레임**이 래그돌 시작 자세라 거기서 바닥에 닿아 있어야 자연스럽다 ③ 사망 클립의 루트 이동은 안 쓰이는 것으로 보이나 미측정([C-112]) ④ 사망 클립 끝의 0.1 s 는 보이지 않는다(래그돌이 먼저). ⑤ 4절의 절차 층 ①②④⑥ 은 **사망 시 0 으로 리셋**되고 ⑧(발 접지·IK)은 사망 포즈 위에서도 그대로 돈다.

---

## 3. 수정 규칙 — 이걸 어기면 조용히 망가진다

| # | 규칙 | 왜 |
|---|---|---|
| 3.1 | **이름·경로·에셋을 바꾸지 않는다.** 리임포트로 덮어쓴다 | PSD·AO·몽타주·ABP 가 전부 에셋 참조. 새 에셋을 만들면 어디에도 안 걸린다 |
| 3.2 | **로코모션 61개의 이동 속도는 방향 간 동일**을 유지(Walk 291.31 · Jog 582.62 · Crouch 291.31 cm/s) | MM 은 궤적으로 고른다. 속도가 어긋나면 Loop 대신 Start/Stop 이 매 걸음 재선택돼 **떨린다**(P30·P31). 속도를 바꾸려면 캐릭터의 `walkSpeeds/runSpeeds/crouchSpeeds` 도 같이 |
| 3.3 | **루프는 루프**(첫·끝 프레임 연속), **`bForceRootLock = true`** 유지 | 풀리면 루프 시 원점으로 순간이동 (P15) |
| 3.4 | **커브를 잃지 않는다.** 리임포트 후 각 클립의 Animation Modifiers 를 **목록 순서대로 Apply All** | `contact_l/r`(발 IK) · `enable_warping` · `Phase` · `DisableLHandIK` 등이 클립 안에 산다. 없으면 발 IK 가 조용히 오작동하고 워핑이 꺼진다. 순서는 P12. 어떤 클립에 어떤 커브인지는 `animation/prototypes/2026-09-04_c34_clip_curve_mapping.md` 4절이 유일한 기준 |
| 3.5 | **애디티브 기준 프레임 0 의 종속**을 안다 | `MM_Rifle_Idle_ADS` f0 → AO 15장 + BF 3장. `MM_Rifle_Crouch_Idle` f0 → Crouch AO 15장. `MM_Rifle_Idle_Hipfire_AO_CC` f0 → 총내림 델타. 기준을 고치면 딸린 것을 같이 확인 |
| 3.6 | **1프레임 포즈는 프레임 0 에만 키를 찍는다** | ABP 가 `ExplicitTime 0` 만 읽는다. 다른 프레임은 시퀀서에서만 보인다 (P80) |
| 3.7 | 포즈 저작·베이크는 **마네킹 메시**로 | soldier_T/enemy_T 로 베이크하면 spine 간격이 틀어진 클립이 나온다 (P80) |
| 3.8 | PSD 에 클립을 **넣고 빼는 것은 우리에게** 말한다 | DB 의 비용 편향·정규화가 클립 밀도에 맞춰져 있다 (P18·P19) |
| 3.9 | 스켈레톤(`SK_UEFN_Mannequin`)에 **본을 추가하지 않는다** | 전 클립 DDC 재압축 + 메시 세 개 영향 |

---

## 4. ★ 시퀀스가 아니라 본을 직접 움직이는 층 — 디자인팀이 시퀀스로 못 고치는 것

이것들은 **애님 그래프/캐릭터 코드가 매 프레임 본을 회전·이동**시킨다. 시퀀스를 아무리 고쳐도 이 층의 결과는 안 바뀌고, 반대로 이 층이 시퀀스 위에 **항상 얹힌다**. 문제가 이 층에서 나면 우리에게 온다.

| # | 층 | 무엇을 하나 | 어디서 | 시퀀스와의 관계 | 끄는 법 |
|---|---|---|---|---|---|
| ① | **린(좌우 기울이기)** Q/E | `spine_01..05` 를 컴포넌트 공간에서 직접 회전 (`ModifyBone` ×5, `LocalToComponentSpace_0` 뒤) | ABP · 캐릭터의 린 축 변수(이름은 [W7] 소급 문서화 전 — 누르면 램프, 떼면 유지) | 모든 시퀀스 위에 얹힌다. `BS1D_Additive_Lean_Run`(이동 뱅킹, GASP)과는 **별개**의 두 번째 기울기 → [C-9] | 축 값 0 (AI 는 `GetDesiredLean`) |
| ② | **stance 축** V/B | 골반(`pelvis`)을 `FootPlacement_0` 과 `LegIK_1` **사이**에서 Z 로 내린다 → LegIK 가 무릎을 굽힌다. 매 프레임 역산(`SolveBoneHeightOffset`) | ABP `PelvisDrop` · 캐릭터 `UpdateStance()` | 0 과 1 에서는 순수 클립(기립/웅크림 DB). **중간 높이는 클립이 아니라 변형**. 문턱 0.5 에서 DB·캡슐·Gait 가 이산 전환 | `StanceStandZ/CrouchZ` 를 클립 골반 높이와 맞추면 극단에서 오프셋 0 (메시별 실측, P77) |
| ③ | **조준 오프셋 적용** | AO 포즈(2.2)를 메시 공간 애디티브로 얹는다 | `ApplyMeshSpaceAdditive_0` · `Enable_AO` | 포즈 자체는 시퀀스(✅) — **얹는 각도**는 코드(`Get_AOValue`) | `Enable_AO` false = 총내림 |
| ④ | **총구 정렬 되먹임 보정** | 총구 전방벡터와 조준선의 오차를 적분해 AO 입력에 **±25°** 편향을 더한다 | 캐릭터 Tick → `AimCorrection` → ABP `Get_AOValue` | AO 포즈가 정확할수록 보정량이 0 에 가까워진다. **포즈를 FK 로 고치면 이 층은 자동으로 조용해진다** | 게인 0.05 → 0 (`AOActive` 게이트가 이미 총내림 중엔 끈다) |
| ⑤ | **총내림 합성** | 2.3절 — 시퀀스 두 장의 **델타를 마스크로 얹는 배선** + 캐릭터의 `WeaponLowered` 램프가 몸통 각속도(90↔720 °/s)와 보정 게인을 같이 몬다 | `LayeredBoneBlend_0` · `BM_LowReady` · `UpdateBodyYawRate()` | 델타의 양 끝(`_AO_CD`/`_CC`)과 베이스(`Idle_Hipfire`)는 시퀀스 ✅. **마스크 가중치와 문턱 70°/65°/90°(결합 쌍, [C-74])는 코드** | — (설계상 상시) |
| ⑥ | **블라인드 파이어 적용** | BF 포즈 3장(2.4)을 `spine_01` 브랜치 마스크로 얹는다 | `[BF_L]→[BF_R]→[BF_U]` | 포즈는 시퀀스 ✅, 마스크·알파는 코드 | 축 0 |
| ⑦ | **왼손 IK** | `hand_l` 을 소켓 `weapon_r` + `LeftHandGripOffset` (~~상수 (−30, 8, 4)~~ → 2026-09-15 부터 총 메시 `LeftHandGrip` 소켓에서 시작 시 산출, 5절) 로 끌어당긴다. 그래프의 **마지막** 스켈레탈 컨트롤 — 위의 전부 뒤에 손을 다시 앉힌다 | `TwoBoneIK_0` · 알파 = `1 − DisableLHandIK` 커브 | 왼손이 시퀀스에서 어디 있든 **IK 가 덮어쓴다** → 디자인팀이 FK 로 고친 왼손이 **안 보인다**. 그래서 **기본 OFF 로 전환** | **`LeftHandIKEnabled`** (아래 5절) |
| ⑧ | **GASP 기본 절차 층** | `OffsetRootBone`(메시가 캡슐을 지연 추적, ~~회전 상한 90°~~ → 상한 없음(−1), 7절 ⑤) · `OrientationWarping`/`StrideWarping`/`Steering`(클립 방향·보폭을 실제 속도에 맞게 다리 변형) · `FootPlacement`+`LegIK`(발 접지) · `RemapCurves` | MM 블렌드스택 안 · 후처리 체인 | 클립의 저작 속도와 실제 속도의 차이를 **다리 변형으로 흡수**한다. 차이가 크면 발이 미끄러지거나 꼬인다(대각선은 클립이 없어 워핑이 만든다) | `enable_warping` 커브 0 인 클립에서는 워핑 OFF (P8d) |

**정리**: 디자인팀이 "시퀀스를 하나하나 FK 로 완벽히" 고칠 때 **결과가 그대로 보이는 것은 ①②③④⑥⑧ 이 0 인 상태**(정지 · 정면 조준 · 축 0 · 왼손 IK OFF)뿐이다. 검수는 그 상태에서 하고, 축을 하나씩 올리며 이 층의 합성을 따로 본다.

---

## 5. 왼손 IK 토글 — 사양 (2026-09-14) → ✅ ABP 쪽 완료 (2026-09-14 저녁, PIE 확인)

```
SoldierCharacter_ABP                                                                  ✅ 완료
    변수  LeftHandIKEnabled (Boolean)   기본값 false        ← 디자인팀 검수 기간 OFF
    TwoBoneIK_0.Alpha  ←  SelectFloat( A = GetCurveValue("DisableLHandIK") , B = 1.0 , bPickA = LeftHandIKEnabled )
                          (alphaScaleBiasClamp scale −1 / bias +1 은 그대로 → 꺼지면 1 − 1 = 0)
                          새 노드 K2Node_CallFunction_5(SelectFloat) · K2Node_VariableGet_7
~~BP_SoldierCharacter~~                                                               ⬜ 만들지 않음 (7절 정정)
~~    변수  LeftHandIK (Boolean, Instance Editable)  기본값 false~~
~~    BeginPlay: Cast SoldierCharacter_ABP → Set LeftHandIKEnabled = LeftHandIK      ← 액터/자식 BP 에서 켤 수 있다~~
```

- 커브 경로를 그대로 두는 이유: 켰을 때 재장전 등의 `DisableLHandIK` 판정이 그대로 살아야 한다 (P27)
- ★ **2026-09-15 추가** — 켰을 때의 그립 위치 `LeftHandGripOffset` 은 이제 상수 `(−30, 8, 4)` 가 아니라 **BeginPlay 에서 총 메시의 `LeftHandGrip` 소켓 위치를 `weapon_r` 소켓 기준으로 역변환해 써 넣는다.** 총 메시(`SK_AR4_X`)의 `LeftHandGrip` 소켓을 옮기면 IK 왼손이 따라간다 — 디자인팀이 만질 수 있는 자리가 하나 더 생긴 셈. `IMPLEMENTED.md` 2.5절

---

## 6. 막힌 것 / 다음에 확인할 것

- [x] 5절 배선 실행 + `compile_blueprint` (P23) — 2026-09-14 저녁 완료. 캐릭터 BP 쪽 세터는 안 만들었다(7절)
- [ ] 엔진 레지스트리로 2절 목록 재확인 (`get_dependencies`, PIE 꺼진 뒤) — 바이트 스캔은 과다 보고할 수 있다. ★ 대신 **디스크 존재 검증**은 `2026-09-14_designer_guide.html` 부록의 103개 경로에 대해 완료했다(레지스트리 참조 방향 확인은 아직)
- [ ] `CHT_Soldier_CharacterAnimations` 가 정말 미사용인지 [B] → **[C-120]** — PIE `a.AnimNode.MotionMatching.DebugDrawInfoVerbose 1` 로 검색 대상 DB 에 GASP 비무장 DB 가 안 나오면 확정. 확정되면 2.6절 갱신
- [x] 피격/사망/체력 배선 ~~[W18]~~ — **2026-09-15 완료**(`ai/2026-09-15_health_hit_death_implementation.md`). 2.7절의 HitReact 13 / Death 6 이 **2.8절**로 올라왔다. 남은 측정은 [C-110]~[C-118]
- [x] 디자인팀 전달용 문서 — `2026-09-14_designer_guide.html` 로 완성(2026-09-14 저녁). 이메일 형식 재사용은 안 했다
- [ ] **[W66]** 총내림 로코모션 클립 요청(7절 ③) — 시점 추후
- [ ] **[W65]** BF 3장의 head 스케일 재베이크(7절 ④) — 지금은 ABP 에서 상쇄 중

---

## 7. 정정 · 후속 (2026-09-15)

① **"총 100개" → 103개.** 실사용 애님 시퀀스 전수를 다시 세니 로코 61 · AO 30 · 총내림 델타 2(`MM_Rifle_Idle_Hipfire_AO_CD/_CC`) · BF 3 · Actions 4(`MM_Rifle_Fire` · `Actions/MM_Rifle_Idle_Hipfire` · `MM_Rifle_Reload` · `MM_Rifle_Reload_Additive`) · GASP 뱅킹 포즈 3(`M_Neutral_Run_Lean_Pose_Base/Left/Right`, 2.6절의 `BS1D_Additive_Lean_Run` 이 참조) = **103**. 여기에 몽타주 2 + 묶음 3(`AO_Rifle_ADS` · `AO_Rifle_Crouch` · `BS1D_Additive_Lean_Run`). 전체 경로는 `2026-09-14_designer_guide.html` 부록 [A].
   ⚠ 1절의 "★ 2026-09-15: 피격 13 · 사망 6 배선 → 100 → 119" 는 **다른 세션(피격/사망)이 100 기준으로 더한 것**이다. 103 기준이면 **122** [B — 그 세션의 2.8절이 확정되면 다시 센다]. 두 숫자가 어긋나는 것은 세는 기준(100 vs 103)의 차이이지 클립이 사라진 것이 아니다.

② **5절의 `BP_SoldierCharacter.LeftHandIK` 세터는 만들지 않았다.** 당시 "다른 BP 의 변수 세터는 MCP 가 못 만든다"(P55)로 판단했기 때문인데, 2026-09-15 에 `Class|SoldierCharacterABP|SetLeftHandGripOffset` 가 `create_node` 로 만들어졌으므로 그 판단은 틀렸다(`CLAUDE.md` 6.1 정정). 필요하면 같은 방법으로 만든다. 지금은 ABP 변수 기본값(false)만이 스위치다.

③ **총내림(2.3절) — 전용 클립 2차 시험 실패, 결론 확정.** `MM_Rifle_LowReady` 를 로컬 애디티브로 바꿔 얹어 봤으나 정지·걷기 모두 나빠 원복했다. 결론(사용자 합의): **이동 중 총내림은 총 내린 로코모션 클립이 없는 한 델타일 수밖에 없고, 델타는 작을 때만 자연스럽다** — 현행 힙파이어 델타(어깨 피치뿐)가 되는 이유. 그래서 2.3절의 "디자인팀이 만질 수 있는 자리" ①②③은 그대로이고, **제대로 하려면 총내림 로코모션 클립이 필요하다**: 최소 Walk/Jog 루프 8장(4방향 × 2), 제대로는 Start/Stop/Pivot 까지 32장 → **[W66]**. 상세 `animation/prototypes/2026-09-15_sharp_turn_pop_bf_head_scale_and_weapon_socket.md` 5절.

④ **BF 포즈 3장(2.4절)에 마네킹 PP ABP 의 head ×1.15 · thigh ×1.12 가 구워져 있다** [A]. 마네킹 메시로 베이크할 때 `ABP_UEFN_Mannequin_PostProcess` 결과까지 들어간 것. 마네킹에선 PP 가 다시 Replace 해서 안 보이고 `soldier_T`/`new_enemy_T` 에선 BF 켤 때 머리가 커졌다. 지금은 ABP 끝에서 head 스케일을 1 로 되돌려 가리고 있다(`ModifyBone_8`). **디자인팀이 BF 포즈를 다시 저작/베이크할 때는 시퀀서 메시 컴포넌트의 `Disable Post Process Blueprint` 를 켤 것** — 3절 수정 규칙에 3.10 으로 추가한다 → **[W65]**. `MM_Rifle_LowReady` 도 같은 방식 베이크라 같은 문제가 있을 것 [B].

⑤ **급선회 끊김(비조준 A↔D 반전 시 메시 스냅)은 시퀀스 문제가 아니었다.** `OffsetRootBone.maxRotationError = 90` 을 GASP 원본 −1 로 되돌려 해결(4절 ⑧ 층의 "회전 상한 90°" 는 이제 **없음**). 디자이너가 같은 증상을 보고하면 클립이 아니라 우리 쪽이다. 같은 문서 1절.

⑥ **총 부착 오프셋이 메시 소켓으로 옮겨졌다.** `WeaponMesh` 컴포넌트의 (−10,0,0)/yaw 180 을 세 메시의 `weapon_r` 메시 소켓에 흡수하고 컴포넌트는 0. 이제 **애님 에디터에서 `weapon_r` 소켓에 Add Preview Asset 으로 `SK_AR4_X` 를 붙이면 런타임과 같은 위치**에 온다 — 왼손 FK 작업의 전제. 같은 문서 3절.

### 3.10 (추가 규칙) 포즈를 마네킹으로 베이크할 때 PP ABP 를 끈다

| # | 규칙 | 왜 |
|---|---|---|
| 3.10 | 시퀀서 Bake 전 메시 컴포넌트 **`Disable Post Process Blueprint`** 켜기 | 마네킹 `ABP_UEFN_Mannequin_PostProcess` 의 head 1.15 · thigh 1.12 스케일이 클립에 구워진다. PP 가 없는 메시에서 델타로 드러난다(7절 ④) |
