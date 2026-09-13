# [C-34] 클립 종류 ↔ 커브 세트 매핑표 — **확정**

2026-09-04 / **성공 (996클립 전수 실측)** / 대량 반입 전에 "어떤 클립에 어떤 모디파이어를 거는가"를 굳혔다.
가장 중요한 결론은 표 자체가 아니라 **"Epic의 데이터에는 일관된 표가 없다"** 는 사실이다.

관련: [C-34] 해결 · [C-36] 해결 · [C-37] 확정(생성기 없음) · 신규 [C-45] [C-46] [C-47]
관련 문서: `2026-09-03_gasp_curve_manifest.md` 8절(**이 문서가 대체한다**) · `2026-09-04_p0-4_complete.md` 3절

---

## 1. 무엇을 확인하려 했나

`2026-09-03_gasp_curve_manifest.md` 8.1절의 표에 구멍이 두 개 있었다:

- 제자리회전의 `movedata_speed`가 "✅/일부없음"으로 뭉개져 있었다
- `steeringtargettime` / `enable_turninplacesteering`은 **생성기를 몰랐다**

**판정 기준**: 반입할 클립 종류마다 **모디파이어 스택을 표로 지정할 수 있는가.**

## 2. 어떻게 했나

`get_asset_tags`의 `CurveNameList` / `AnimSyncMarkerList`를 **996개 클립 전수**로 떴다
(`Walk` 452 · `Crouch` 431 · `Idle` 48 · `Sprint` 65). 추가로 GASP 모디파이어 18개의 CDO를 전부 읽었다.

> **MCP 요령 (신규)**: BP 모디파이어의 CDO는 `<경로>.Default__<이름>_C` 로 읽는다.
> `<경로>.<이름>_C`(클래스)로는 프로퍼티가 안 읽힌다. → `CLAUDE.md` 6.1절
>
> **함정**: `execute_tool_script` 안에서 `try/except`로 감싸도 **툴 호출이 한 번이라도 실패하면
> 배치 전체가 실패로 반환된다.** 긴 배치를 돌리는 동안 다른 MCP 호출을 섞지 말 것.

---

## 3. ★ 핵심 결론 — Epic의 데이터는 일관돼 있지 않다

같은 종류의 클립인데 커브 세트가 다르다. 실측 [A]:

| 클립 | 커브 |
|---|---|
| `M_Neutral_Walk_Turn_L_045_Lfoot` | `contact_l/r` `enable_warping` |
| `M_Neutral_Walk_Turn_L_090_Lfoot` | `contact_l/r` `enable_warping` **`phase`** |
| `M_Relaxed_Walk_Turn_090_L_Lfoot` | `contact_l/r` `enable_warping` `phase` **`movedata_speed` `steeringtargettime` `disable_additiveleans`** |
| `M_Relaxed_Stand_Turn_090_L` | `contact_l/r` `enable_turninplacesteering` `steeringtargettime` |
| `M_Neutral_Crouch_Idle_Turn_090_L` | 〃 + **`movedata_speed`** |

"걸으며 90° 회전"이라는 **같은 카테고리에서 커브가 3종/4종/6종**으로 갈린다.
Neutral/Relaxed 티어 차이도 아니고 각도 차이도 아니다 — **저작 시점의 편차**로 보인다.

### 3.1 그래서 방침을 바꾼다

> **Epic의 표를 복제하려 하지 말 것.** 그런 표는 없다.
> **커브를 소비하는 쪽(PSD 스키마·ABP)이 무엇을 읽는지로 우리 규칙을 정하고, 균일하게 적용한다.**

티어를 따라가는 것도 답이 아니다. Neutral 세트(우리가 쓰는 CMC/Dense 계열)가 오히려 더 들쭉날쭉하다 —
`M_Neutral_Walk_Loop_FL`에는 `phase`가 없고, `M_Neutral_Walk_Turn_L_045`에는 `movedata_speed`가 없다.

### 3.2 ★ 스키마가 실제로 읽는 커브는 `Phase` 하나뿐이다 [A]

PSS 25개 중 대표 6개의 채널 구성을 실측했다:

| 스키마 | 채널 |
|---|---|
| **`PSS_Relaxed_Loops`** | Trajectory · **Curve `Phase` (t=−0.0333)** · **Curve `Phase` (t=0)** · Heading |
| `PSS_Default` ← **우리 것** | Trajectory · Group{Position, Velocity×2, Heading} |
| `PSS_Relaxed_Starts` / `_Pivots` / `_Stops` / `_StandTurn` / `PSS_Idle` | Trajectory · Group |

**`Phase`를 읽는 스키마는 `PSS_Relaxed_Loops` 단 하나다.** 그것도 **루프 전용**이다 —
두 시점(현재/한 프레임 전)을 같이 보는 건 **보행 주기의 연속성**을 지키려는 것이고,
start/stop/pivot/turn은 애초에 루프가 아니라 주기가 없다.

이것이 Epic의 편차를 설명한다: **`phase`는 있으나 마나 한 클립이 많아서 저작이 들쭉날쭉한 것이다.**

> **우리에게 주는 결론**: [Q11]에서 CMC를 확정하며 PSD를 `PSS_Default` + `PSN_Dense_All`로
> 바꿨으므로 **지금 우리 PSD는 `phase`를 아예 안 읽는다.**
> 그래도 **루프 클립에는 굽는다** — 생성 비용이 0에 가깝고, 나중에 견착 전용 루프 스키마를
> 만들 때 필요해지며, `contact_l/r`과 달리 넣어서 손해 보는 커브가 아니다. [B]
>
> ⚠ `2026-09-04_p0-4_complete.md` 4절의 `PSS_Relaxed_Loops` / `PSN_Relaxed_All` 표기는
> [Q11] 전환 **이전** 값이다. 현행은 `PSS_Default` / `PSN_Dense_All`.

### 3.3 안전 방향 — "빠뜨리는 것"이 "넣는 것"보다 안전하다

커브가 없으면 `Get Curve Value from Animation`이 **0**을 돌려준다(매니페스트 8.2절).
즉 **누락 = 그 기능 OFF**다. 반대로 잘못 넣으면 **기능이 켜진다.**

| 커브 | 잘못 **넣으면** | 잘못 **빠뜨리면** |
|---|---|---|
| `enable_warping` | 정지 중에 워핑이 켜져 **발이 미끄러진다** | 워핑 안 걸림(품질 저하) |
| `contact_l/r` | — | **발 IK가 조용히 오작동**(P8) |
| `phase` | 무해 — 우리 스키마는 안 읽는다(3.2절) | 루프 스키마를 쓰게 되면 채널이 0으로 고정 |
| `movedata_speed` | 거의 무해 | ABP 속도 소스 결손 |

→ **애매하면 빼고, `contact_l/r`만은 예외 없이 만든다.**

---

## 4. ★ 확정 매핑표 — 우리 반입 규칙

행은 **모디파이어 적용 순서**다(= 목록 순서 = `Apply All` 순서, `CLAUDE.md` P12).

| # | 모디파이어 | ①이동 루프 | ②출발 | ③정지 | ④피벗 | ⑤걸으며 회전 | ⑥제자리 회전 | ⑦Idle·정지포즈 | ⑧AimOffset·additive |
|---|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
| 1 | `AM_EncodeRootBone` | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ |
| 1b | **`SoldierRootFacingModifier`** ★ 주3 | ✅ | ✅ | ✅ | ✅ | ✅ | **❌** | ❌ | ❌ |
| 1c | **`AM_Copy_IKFootRoot`** ★ 주4 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ |
| 2 | `AM_FootSteps_*_Soldier` | ✅ **마커 ON** | ✅ 마커 OFF | ✅ OFF | ✅ OFF | ✅ OFF | ⚠ 주1 | 선택 | ❌ |
| 3 | `AM_BakePhaseCurveFromFootstepNotifies` | ✅ | ✅ | ✅ | ✅ | ✅ | **❌** | ❌ | ❌ |
| 4 | `FootContactCurveModifier` (자작) | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ |
| 5 | `AM_MoveData_Speed` | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ |
| 6 | `AM_WarpingAlpha_Soldier` | ✅ | ✅ | **❌** | ✅ | ✅ | **❌** | **❌** | ❌ |
| 7 | `bEnableRootMotion = true` | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | — |
| 7b | **`bLoop`** ★ 주2 | **true** | ❌ | ❌ | ❌ | ❌ | ❌ | 루프만 **true** | ❌ |
| 7c | **`bForceRootLock = true`** ★ 주5 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| 7d | **`curveCompressionSettings`** = `SandboxAnimCurveCompressionSettings` ★ 주6 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| 8 | **`UTurnInPlaceCurvesModifier`** (자작) — 스티어링 커브 2종 + PoseSearch 노티파이 2종 | | ✅ | ✅ | ✅ | ✅ | ✅ | | |

> **주1 — ⑥제자리 회전의 `AM_FootSteps`는 "발소리가 필요할 때만"이다.**
> 발을 들지 않고 비비면서 도는 클립에서는 **양발 노티파이가 한 프레임에 뭉친다**
> (엔진 판정이 "시작 시 접지 중인 발의 첫 이벤트를 버리고" → 두 번째 이탈이 없어 마지막-샘플
> 분기로 떨어진다). 용처가 발소리뿐이라 **MM·발 IK에는 영향이 0**이므로 그냥 빼면 된다.
> → `2026-09-08_turn_in_place_curves_modifier.md` 5절 · **[C-38]**
>
> **8행은 원래 "손 저작"이었으나 2026-09-08에 모디파이어로 대체됐다.** 기본값 그대로
> Add → Apply. 게이트 시각은 클립의 루트 yaw 프로파일에서 자동으로 잡는다(5.0d절).
>
> **주5 — `bForceRootLock = true`는 모든 반입 클립에 필수다.** GASP 클립은 전부 true다.
> 루트가 잠기는 조건은 `(bExtractRootMotion && bEnableRootMotion) || bForceRootLock`
> (`AnimSequence.cpp:1862`)인데, **GASP는 루트모션으로 캐릭터를 움직이지 않아 앞 조건이 false**다.
> false로 두면 **포즈에 루트 이동이 남아 루프 지점에서 원점으로 순간이동**한다.
> → `2026-09-09_walk_quality_debugging.md` 3절 (3)
>
> **주6 — 커브 압축을 `SandboxAnimCurveCompressionSettings`로 바꾼다.** 프로젝트 기본값은
> ACL이고 Epic은 GASP 클립에만 이 커스텀 설정을 지정해뒀다. `contact_l/r`은 전환점에
> 0.001초 간격 키 2개로 만든 사각파라 손실 압축에 취약하다.
> (2026-09-09 시점에 이것 자체가 증상의 원인은 아니었으나, GASP와 정합을 맞춘다)
>
> **주3 — `SoldierRootFacingModifier`(자작)는 `AM_EncodeRootBone` 바로 다음에 온다.**
> `AM_EncodeRootBone`은 pelvis의 yaw를 그대로 루트 yaw로 쓰므로 **견착 자세의 골반 블레이드가
> 루트에 구워진다.** 실측 **56°** — Orientation Warping이 그만큼 다리를 틀어버린다.
> **제자리 회전·Idle에는 걸지 말 것**(이동이 없어 진행 방향이 정의되지 않는다).
> → `2026-09-08_root_facing_56deg.md`
>
> **주4 — `AM_Copy_IKFootRoot`는 Orientation Warping의 전제 조건이다.**
> 워핑 노드가 `ik_foot_root` / `ik_foot_l` / `ik_foot_r`를 회전시켜 동작한다. 리타깃 클립에서
> 이 가상본이 비어 있으면 **워핑이 회전시킬 대상이 없다** → [C-46]
>
> **주2 — `bLoop`은 커브도 모디파이어도 아닌 시퀀스 자체 플래그다.** 2026-09-08에 뒤늦게
> 발견해 추가했다. **순환하는 클립만 true**(이동 루프 · Idle **루프**). Idle이라도 *Break*는 false.
> GASP 실측: `Walk_Loop_F`/`Walk_Loop_FL`/`Stand_Idle_Loop` = true,
> `Walk_Start`/`Stop`/`Pivot`/`Stand_Turn_090`/`Idle_Break_v02` = false.
>
> 틀리면 **조용히 망가진다** — `PoseSearchAssetSampler.cpp:377`이 `IsLoopable()`을 이 플래그로
> 판정하고, false면 ① 인덱싱이 클립 끝에서 wrap 대신 **clamp**(마지막 포즈가 얼어붙은 채 특징 생성),
> ② `LoopingCostBias`가 안 붙고, ③ BlendStack이 **끝에서 멈춘다**.
> `get_asset_tags`에는 안 나오고 `list_properties`로만 보인다.

**결과 커브 세트:**

| 종류 | 커브 | 싱크마커 |
|---|---|---|
| ① 이동 루프 | `contact_l/r` `movedata_speed` `enable_warping` `phase` | **`L` `R`** (정면 루프만) |
| ② 출발 | 〃 | 없음 |
| ③ 정지 | `contact_l/r` `movedata_speed` `phase` | 없음 |
| ④ 피벗 | `contact_l/r` `movedata_speed` `enable_warping` `phase` | 없음 |
| ⑤ 걸으며 회전 | 〃 (+ `steeringtargettime` ⚠불가) | 없음 |
| ⑥ 제자리 회전 | `contact_l/r` `movedata_speed` (+ 스티어링 2종 ⚠불가) | 없음 |
| ⑦ Idle·정지 포즈 | `contact_l/r` `movedata_speed` | 없음 |
| ⑧ AimOffset·additive | **없음** | 없음 |

### 4.1 근거 — 왜 이 배치인가

- **③정지에 `enable_warping` 없음** — Epic의 `Walk_Stop` / `Reface_Stop` / 제자리회전 **전부**에 없다.
  커브를 0으로 채우는 게 아니라 아예 안 만드는 것이 규약이다 [A]
- **⑥제자리회전에 `phase` 없음** — Epic의 `Stand_Turn_090/180` 8개 전부에 없다 [A].
  **단 발자국 노티파이는 있다** — `M_Relaxed_Stand_Turn_090_L`을 열어 보면 `Footstep Left`/`Right`
  트랙에 `FoleyEvent: Walk` 노티파이가 각각 하나씩 찍혀 있다(90° 회전에도 발은 한 번씩 딛는다).
  즉 Epic은 **`AM_FootSteps_*`는 걸고 `AM_BakePhaseCurve`는 일부러 안 걸었다.**
  루프가 아니라 주기가 없으니 `phase`가 무의미하기 때문이다(3.2절)
- **⑥에도 `FootContactCurveModifier`는 건다** — 우리 모디파이어는 **노티파이에 의존하지 않고**
  본 높이/속도를 직접 샘플링한다. `Turning_Right_90_Degrees_Anim`에 `contact_l/r`이
  실제로 생성됨을 확인했다 → **[C-36] 해결** [A]
- **⑧에 아무것도 안 검** — Epic의 포즈·리인 클립 **179개가 커브 0개**다 [A]

### 4.2 `AM_FootSteps_*` 변종 선택

`AM_FootSteps_Walk` / `_Run` / `_Crouch`의 CDO는 **발소리 노티파이 클래스만 다르다** [A]:

```
공통      sampleRate 60 · groundThreshold 4 · speedThreshold 0.1
          ball_l → "Footstep Left" 트랙,  ball_r → "Footstep Right" 트랙
          footstepNotifyDetectionTechnique = FootBoneSpeed
          bShouldGenerateSyncMarkers = false      ★ CDO 기본값은 마커 OFF
차이       Walk   → BP_AnimNotify_FoleyEvent_Walk_L / _R
          Run    → BP_AnimNotify_FoleyEvent_Run_L / _R
          Crouch → BP_AnimNotify_FoleyEvent_Crouch_L / _R
```

→ **보행 속도대(walk/run/crouch)에 맞는 변종을 고른다.** 발소리만 달라지고 커브 결과는 같다.

> CDO 기본이 `bShouldGenerateSyncMarkers = false`이므로 **①루프에서만 켜야 한다.**
> 켤 때 `syncMarkerDetectionTechnique = FootBoneSpeed`, 마커 이름 `L`/`R`.
> `CLAUDE.md` P11대로 **CDO를 고쳐도 이미 붙인 인스턴스엔 안 먹는다** — Remove → 재Add.

### 4.3 싱크마커는 사실상 안 쓴다 [A] — 문서 정정

996개 중 **싱크마커를 가진 클립은 4개뿐이다**:

```
M_Relaxed_Walk_Loop_F · M_Relaxed_Walk_Slope_down_Loop_F
M_Relaxed_Walk_Slope_up_Loop_F · M_Neutral_Walk_Pivot_F_B_Lfoot
```

`M_Neutral_Walk_Loop_F`에도, 8방향 `Walk_Loop_FL/BL/...` 어디에도 없다.
매니페스트 8.1절이 "루프는 `L`/`R` 싱크마커"라고 적었는데 **정면 Relaxed 루프에 한정된 얘기**였다.

→ 모션 매칭은 싱크마커를 안 쓴다(블렌드스페이스 동기화용이다). **품질에 영향 없다.**
우리 `Walking_Anim`에 `L`/`R`이 붙어 있지만 무해하므로 그대로 둔다.

---

## 5. ★ 만들 수 없는 커브 — [C-37] 확정

`/Game/Blueprints/AnimModifiers/`의 **18개를 전수 확인**했다 [A]:

```
AM_BakePhaseCurveFromFootstepNotifies   AM_Copy_IKFootRoot   AM_DistanceFromLedge
AM_FootSpeed_L / _R                     AM_FootSteps_Walk / _Run / _Crouch / _Modulation
AM_MoveData_Speed                       AM_OrientationWarpingAlpha   AM_RateWarpingAlpha
AM_RemoveCurves  AM_RenameCurve  AM_ReorderCurves  AM_Reset_Attach
AM_TriggerWeightThreshold               AM_WarpingAlpha
```

**`steeringtargettime` / `enable_turninplacesteering`을 만드는 것은 없다.**
`contact_l/r`과 같은 상황이다 — Epic의 원본 저작물이거나 사내 툴 산출물이다.

이 두 커브는 회전 계열 클립 **전부**에 붙어 있다(Relaxed 기준):

| 클립군 | `enable_turninplacesteering` | `steeringtargettime` |
|---|:-:|:-:|
| `Stand_Turn_090/180` | ✅ | ✅ |
| `Crouch_Idle_Turn_090/180` | ✅ | ✅ |
| `Walk_Reface_Stop_*` | ✅ | ✅ |
| `Walk_Reface_Start_*` | ❌ | ✅ |
| `Walk_Turn_*` / `Walk_Spin_F_360_*` | ❌ | ✅ |

### 5.0 ★ 커브 모양 실측 — **손 저작으로 간다** [A]

`M_Relaxed_Stand_Turn_090_L`(60프레임)을 열어 확인:

```
enable_turninplacesteering   ▔▔▔▔▔▔▔┐              1 → 0 계단
                                    └────────
steeringtargettime           ────────────────      전 구간 상수 = 1.0
```

**실측값** (`M_Relaxed_Stand_Turn_090_L` = 60프레임 / 2.000s / 30fps):

| 커브 | 값 |
|---|---|
| `enable_turninplacesteering` | **1.0** (프레임 0~17 = 0.000~0.567s) → **0.0** (프레임 18~ = 0.600s~) |
| `steeringtargettime` | **1.0** 전 구간 상수 |

게이트가 꺼지는 지점은 **클립의 28%** 지점이고, 같은 클립의 `contact_r`이 다시 1로
돌아오는 프레임(~19)보다 **2프레임 정도 앞선다.** 즉 "마지막 발이 착지하는 순간"과
거의 같지만 정확히 일치하지는 않는다 [B].

**표본 2개로 갈렸다** — `M_Relaxed_Stand_Turn_180_L`(65프레임 / 2.167s)도 `steeringtargettime`이
**1.0 그대로**였다. → **고정 상수 1.0 확정** (길이 비례 아님) [A]

| 클립 | 길이 | `steeringtargettime` | 게이트 OFF |
|---|---|---|---|
| `M_Relaxed_Stand_Turn_090_L` | 60f / 2.000s | **1.0** | 프레임 **17** = 0.567s (28%) |
| `M_Relaxed_Stand_Turn_180_L` | 65f / 2.167s | **1.0** | 프레임 **15** = 0.500s (23%) |

**★ 회전각이 두 배인데 게이트는 오히려 빨리 꺼진다.** 즉 게이트 길이는
클립 길이·회전량과 무관한 **절대 시간 ≈0.5초**다 [B]. "발이 착지할 때까지"가 아니다
(180° 클립에서는 착지가 훨씬 뒤일 것이다).

**해석**: 스티어링은 **회전 초반 0.5초 동안만 절차적으로 방향을 틀 수 있고**, 그 뒤로는
애니메이션 자신의 루트모션에 맡긴다. `steeringtargettime = 1.0`은 그 0.5초 창 안에서
**1초 앞의 루트모션까지 내다보고** 얼마나 돌 예정인지 계산한다는 뜻이다.

**둘 다 자작 모디파이어가 필요 없다.** `enable_turninplacesteering`은 키 2개(1 → 0),
`steeringtargettime`은 키 1개다. → **(b) 손 저작 채택. [C-45]의 "어떻게"는 닫혔다.**

**의미** — 엔진 소스 `FAnimNode_Steering`(`AnimationWarping/.../AnimNode_Steering.h`)에서 확인:

| 노드 프로퍼티 | 뜻 | 기본값 |
|---|---|---|
| `AnimatedTargetTime` | **몇 초 앞의 루트모션을 샘플링해 "이 애니가 얼마나 돌 예정인가"를 알아낼지** | 2.0 |
| `ProceduralTargetTime` | 애디티브 보정으로 목표 방향에 도달할 목표 시간 | 0.2 |
| `RootMotionThreshold` | 이 각도 미만이면 루트모션 스케일링을 끔 | 1.0 |
| `DisableSteeringBelowSpeed` | 이 속도 미만이면 스티어링 전체 비활성 | 1.0 |

`steeringtargettime`은 `AnimatedTargetTime`에 물리는 값으로 보인다 [B] — 상수인 것과,
"미래 몇 초를 볼지"라는 의미가 상수 저작과 맞아떨어진다.
`enable_turninplacesteering`은 스티어링 게이트다: **회전 스텝 구간에만 1**, 정착 후 0.

> **남은 미지수는 상수의 숫자뿐이다** → 아래 5.0b

### 5.0b 상수 값 읽는 법 (엔진 소스 확인 [A])

MCP로는 커브 **값**을 못 읽는다(`CLAUDE.md` 6.1). 에디터에서 두 방법이 있다:

| 방법 | 근거 |
|---|---|
| **A. 커브 트랙 위에 마우스를 올린다** → 곡선 옆에 **Y축 경계값** 라벨이 뜬다. 상수 커브면 그게 답 | `AnimTimelineTrack_Curve.cpp:269-275` — `SCurveBoundsOverlay`, `BoundsLabelFormat "{1}"`(Y축만), **hover 중에만 보임** |
| **B. 커브 이름 행을 더블클릭** → 커브 에디터가 열려 키 값이 숫자로 보인다. 행에 마우스를 올리면 나타나는 **`Curve ▼` → `Edit Curve`** 도 같은 동작 | 같은 파일 288행(`SetOnMouseDoubleClick`) · 303행(`MakeTrackButton`) · 319-323행 |

### 5.0c ★ 손 저작이 아니라 **모디파이어**로 간다 — `UTurnInPlaceCurvesModifier`

~~손으로 찍는다~~ → **자작 모디파이어 하나가 세 가지를 다 한다** (2026-09-08 신설,
`Source/SoldierLabEditor/AnimModifiers/TurnInPlaceCurvesModifier.*`):

```
steeringtargettime            상수 1.0
enable_turninplacesteering    1 → 0 계단
PoseSearch 노티파이 2종        Continuing Pose Cost Bias(앞) + Block Transition In(뒤)  ← [C-48]
```

기본값 그대로 Add → Apply 하면 끝이다. 설정할 것이 없다.
자세한 내용은 **`2026-09-08_turn_in_place_curves_modifier.md`**.

### 5.0d ★ 정정 — 게이트 시각은 **고정 0.5초가 아니다**

5.0b에서 "클립 길이·회전각과 무관한 절대 시간 ≈0.5초"라고 적었는데, 그건 **GASP 두 클립이
둘 다 회전을 앞부분에서 끝내기 때문**이었다. 표본이 같은 출처뿐이라 구분이 안 됐다.

우리 Mixamo 클립을 실측하니 완전히 다르다 [A]:

```
Turning_Right_90_Degrees_Anim   2.400s   root yaw net 88.8° / travelled 94.6°
10%@0.417  25%@0.583  50%@0.900  75%@1.167  90%@1.367  95%@1.450  100%@2.400
```

**0.417초는 회전이 겨우 10% 진행된 시점이다.** 0.5초에 게이트를 닫았으면 정작 돌기 시작하기도
전에 스티어링을 꺼버리는 셈이었다.

#### 진짜 규칙 — "마지막 발이 착지할 때 닫힌다" [B]

| 클립 | 게이트 OFF | 마지막 발 착지(`contact` 복귀) |
|---|---|---|
| GASP `M_Relaxed_Stand_Turn_090_L` (60f) | 프레임 17 | 프레임 ~19 |
| 우리 `Turning_Right_90_Degrees_Anim` (72f) | **프레임 41** (yaw 90% 지점) | **프레임 41** |

**yaw 진행률 90%와 발 착지 시점이 독립적으로 같은 답을 냈다.** 그래서 모디파이어는
`bAutoSteeringOffTime`으로 **클립의 루트 yaw 프로파일에서 직접 뽑는다** — 클립마다 회전이
차지하는 구간이 다르므로 상수를 물려주면 출처가 바뀌는 순간 틀린다.

> 아직 안 닫힌 것: **GASP 클립의 yaw 프로파일을 안 떴다.** GASP의 0.567초가 그 클립의
> yaw 90% 지점인지 확인하면 규칙이 [A]가 된다. 클립을 우리 폴더로 복제해 같은 모디파이어를
> 걸면 1분이다(원본은 안 건드린다). → **[C-45b]**

#### 부수 확인 — 회전 클립의 루트모션이 맞다 [A]

`root yaw net 88.8°` — 리타깃 + `AM_EncodeRootBone`(pelvis 축 Y)이 **회전 클립에서도 90°를
제대로 살렸다.** [C-30]에 "회전 클립으로 A안 재검증은 아직"으로 남아 있던 항목의 첫 실측이다.
`travelled 94.6°`와의 차이 5.8°는 오버슈트 후 정착이다.

### 5.3 ★ 회전 클립의 진짜 장치는 커브가 아니라 **PoseSearch 노티파이**였다 [A]

`M_Relaxed_Stand_Turn_090_L`에는 노티파이 트랙 2개가 더 있다:

| 노티파이 | 엔진 정의 | 이 클립에서의 구간 |
|---|---|---|
| **`Pose Search: Block Transition In`** | "이 노티파이와 겹치는 결과는 **검색이 반환하지 않는다.** 단 앞선 검색 결과가 진행해 들어오는 것은 허용" | 프레임 ~13 → 끝 |
| **`Pose Search: Override Continuing Pose Cost Bias`** | 그 구간의 **유지 비용 편향**을 덮어쓴다. 음수 = 계속 머무를 확률↑ | 0 → ~13 |

출처: `PoseSearch/Source/Runtime/Public/PoseSearch/PoseSearchAnimNotifies.h:38-44, 62-65` ·
`PoseSearchDatabase.h:511-516` (`ContinuingPoseCostBias` 기본 −0.01).

**해석**: 회전 클립은 **앞부분으로만 진입할 수 있고**(뒷부분은 Block Transition),
일단 들어가면 **비용 편향으로 붙잡아 둔다.** 그래서 90° 회전이 중간에 끊기지 않는다.
**이건 커브가 아니라 노티파이라서 [C-34] 조사에서 놓칠 뻔했다** — `AnimNotifyList` 태그가
BP·노티파이스테이트를 안 잡기 때문이다(`CLAUDE.md` 6.1).

→ **[C-48] 신규**: 우리 `Turning_Right_90_Degrees_Anim`에는 이 두 노티파이가 **없다.**
   견착 회전을 DB에 넣으면 MM이 회전 중간으로 튀어 들어가거나 중간에 빠져나갈 수 있다.
   **P0-2에서 [C-44](급선회)를 판정할 때 이 노티파이 유무가 교란 변수가 된다.**

### 5.1 부수 발견 — 문서에 없던 워핑 커브 2종 [A]

| 모디파이어 | 커브 | 블렌드 |
|---|---|---|
| `AM_WarpingAlpha` | `Enable_Warping` | 0.25s |
| **`AM_OrientationWarpingAlpha`** | **`Enable_OrientationWarping`** | 0.25s |
| **`AM_RateWarpingAlpha`** | **`Enable_PlayRateWarping`** | **0.40s** |

셋 다 임계 5°/5°, 30Hz로 동일하고 **커브 이름만 다르다.**
그런데 **996개 클립 어디에도 `Enable_OrientationWarping` / `Enable_PlayRateWarping`이 없다.**
→ 출하물에 안 쓰이는 실험용으로 보인다. **우리도 쓰지 않는다.** [B]

### 5.2 부수 발견 — 우리 파이프라인에 없는 단계 `AM_Copy_IKFootRoot`

```
bonePairs = [ attach → ik_foot_root,  foot_r → ik_foot_r,  foot_l → ik_foot_l ]
bonePoseSpace = World
```

리타깃 클립의 `ik_foot_*` 가상본이 채워져 있지 않으면 발 IK가 어긋난다.
우리 11단계에는 이 단계가 없다. → **[C-46] 신규**

---

## 6. 관측된 커브 전량 (996클립) [A]

| 커브 | 클립 수 | 생성기 |
|---|---:|---|
| `contact_l` / `contact_r` | 383 | **자작** `FootContactCurveModifier` |
| `movedata_speed` | 362 | `AM_MoveData_Speed` |
| `enable_warping` | 319 | `AM_WarpingAlpha` |
| `phase` | 220 | `AM_BakePhaseCurveFromFootstepNotifies` |
| `steeringtargettime` | 62 | **없음** ⚠ |
| `enable_turninplacesteering` | 18 | **없음** ⚠ |
| `disable_ao` | 8 | 없음 (Idle Break 전용) |
| `disable_additiveleans` | 8 | 없음 (`Walk_Turn`/`Spin` 전용) |
| `sprintweight` | 1 | 없음 (`Transition_Walk_to_Sprint` 1개) |

### 6.1 "전 클립 공통은 `contact_l/r`" 은 틀렸다 — P8 정정

매니페스트 8.1절의 문장이다. 실측하면:

- **커브가 하나도 없는 클립 179개** (포즈·리인·`Relaxed_Crouch_Diamond/Hourglass` 84개)
- **`contact_l/r`이 없는데 다른 커브는 있는 클립 18개** — `M_Relaxed_Walk_Spin_LL_B` 등
  (`enable_warping` `movedata_speed` `phase`만 있음)

Spin 18개는 규약이 아니라 **누락으로 보인다** [B] — 같은 폴더의 `Walk_Spin_F_360_*` 8개는
`contact_l/r`을 갖고 있다. **우리는 이 편차를 따라하지 않는다**(3.2절: 빠뜨리면 발 IK가 망가진다).

---

## 7. 판정

**성공.** 4절 표로 클립 종류마다 모디파이어 스택이 지정된다. 대량 반입 진행 가능.

다만 **회전 계열은 스티어링 커브 2종이 비어 있는 채로 들어간다.** [C-45]에서 판정한다.

## 8. 이것이 바꾸는 것

| 문서 | 어떻게 |
|---|---|
| `CLAUDE.md` P8 | "전 클립 공통 `contact_l/r`" 삭제 → **4절 표 참조**로 교체. 안전 방향(3.2절) 추가 |
| `CLAUDE.md` 6.1절 | CDO 읽기 경로 `Default__*_C`, 배치 오염 함정 |
| `2026-09-03_gasp_curve_manifest.md` 8절 | **이 문서로 대체됨** 표시 |
| `2026-09-04_p0-4_complete.md` 3절 | 11단계는 ①루프 기준이었음을 명시 → 4절 표가 일반형 |
| `OPEN_ITEMS.md` | C-34/C-36 해결 · C-37 확정 · **C-45 C-46 C-47** 신규 |

## 9. 막힌 것 / 다음에 확인할 것

| ID | 항목 | 왜 |
|---|---|---|
| **C-48** | 회전 클립의 **PoseSearch 노티파이 2종** 누락 | 5.3절. P0-2 [C-44] 판정의 교란 변수 |

> **[C-45]는 닫혔다** — 5.0c의 레시피가 답이다. 남은 건 실제로 찍고 PIE에서 보는 것뿐이고,
> 그건 P0-2의 일이다.
| **C-46** | `AM_Copy_IKFootRoot` 단계가 우리에게도 필요한가 | 리타깃 클립의 `ik_foot_*` 상태 미확인 |
| **C-47** | `disable_ao` / `disable_additiveleans` 생성기 없음 | 소수 클립 전용. 손 저작으로 충분할 듯 [B] |
