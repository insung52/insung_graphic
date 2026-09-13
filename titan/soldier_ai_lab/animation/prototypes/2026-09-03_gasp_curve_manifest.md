# P0-4b · GASP 기준 커브 매니페스트 — 반입 클립이 도달해야 할 목표 상태

2026-09-03 / **성공 (에셋 실측)** / 커브는 고정 목록이 아니라 **클립 종류별로 다른 세트**였다. 그리고 `contact_l/r`은 우리가 만들 수단이 없다.

> ## ⚠ 2026-09-04 — **8절의 매핑표는 폐기됐다**
>
> **`2026-09-04_c34_clip_curve_mapping.md` 를 볼 것.** 996개 클립 전수 실측으로 대체됐다.
> 이 문서 8.1절 표는 표본이 Walk 폴더에 치우친 상태에서 만든 것이고, 아래 두 문장이 **틀렸다**:
>
> - ~~"유일하게 모든 클립에 있는 것은 `contact_l`/`contact_r` 뿐"~~ →
>   커브가 **0개인 클립이 179개**, `contact` 없이 다른 커브만 있는 클립이 18개 있다
> - ~~"루프는 `L`/`R` 싱크마커"~~ → 996개 중 마커를 가진 클립은 **4개뿐**이다
>   (`M_Relaxed_Walk_Loop_F` 계열 3 + `Pivot_F_B_Lfoot` 1). 8방향 루프에는 없다
>
> 9절([C-28] `contact_l/r`은 이진 커브)과 10절(파이프라인)은 **유효하다.**

> **2026-09-03 저녁 갱신**: 8~10절이 이 문서의 결론이다. 3~5절(초기 조사)보다 **8절 이후를 먼저 볼 것.**

관련 항목: **[C-26]/[C-27] 비교 기준 확보** · 신규 **[C-28]** / 관련 문서: `assets/2026-09-02_...md` 4.2절 ·
`animation/2026-09-02_gasp_abp_analysis.md` 3.3b절

---

## 1. 무엇을 확인하려 했나

[C-26]/[C-27]은 "우리 클립의 커브가 쓸 만한가"를 묻는데, **비교 기준이 없으면 판정할 수 없다.**
그래서 GASP 기존 클립이 실제로 어떤 커브를 갖고 있는지를 먼저 떴다.

**판정 기준**: 반입 클립이 도달해야 할 **목표 상태를 항목 단위로 적을 수 있는가**.

## 2. 어떻게 했나

MCP. `AssetTools.get_asset_tags`의 `CurveNameList` / `AnimSyncMarkerList` 필드가 커브·마커
이름을 그대로 준다(에셋을 열지 않아도 된다). `ProgrammaticToolset.execute_tool_script`로 일괄 조회.

```
editor_toolset.toolsets.asset.AssetTools.get_asset_tags        커브/마커 이름
editor_toolset.toolsets.object.ObjectTools.get_properties      모디파이어 CDO 실측값
editor_toolset.toolsets.blueprint.BlueprintTools.get_parent    모디파이어의 네이티브 부모
```

## 3. 결과

### 3.1 GASP walk 클립의 커브 구성 [A] — 실측

| 클립 | 커브 | 싱크마커 | RootMotion |
|---|---|---|---|
| `M_Relaxed_Walk_Loop_F` | `contact_l` `contact_r` `movedata_speed` **`enable_warping`** `phase` | **`L` `R`** | True |
| `M_Neutral_Walk_Loop_F` | 〃 (`Phase` 대문자) | 없음 | True |
| `M_Relaxed_Walk_Start_F_Lfoot` | 〃 | 없음 | True |
| **`M_Relaxed_Walk_Stop_F_Lfoot`** | `contact_l` `contact_r` `movedata_speed` `phase` **`enable_turninplacesteering`** **`steeringtargettime`** | 없음 | True |
| `M_Relaxed_Walk_Pivot_F_B_Lfoot` | 〃 + `enable_warping` | 없음 | True |
| `M_Relaxed_Walk_Turn_180_L_Lfoot` | 〃 + `enable_warping` + `steeringtargettime` | 없음 | True |
| `M_Neutral_Walk_Arc_F_Tight_L` | `contact_l/r` `movedata_speed` `enable_warping` (**phase 없음**) | 없음 | True |
| `M_Neutral_Walk_Circle_Strafe_L` | 〃 | 없음 | True |

### 3.2 ★ `Stop` 클립에는 `enable_warping` 이 아예 없다 [A]

이게 이번 조회에서 가장 값진 관측이다. 16절의 "워핑은 직선 구간만 커버한다"가
**커브를 넣고 0으로 깎는 방식이 아니라, 커브 자체를 안 만드는 방식**으로 구현돼 있다.

- 커브가 없으면 `Get Curve Value from Animation`이 0을 돌려준다 → 워핑 Alpha = 0
- 즉 **정지 동작 중에는 워핑이 구조적으로 못 켜진다**
- 대신 `enable_turninplacesteering` / `steeringtargettime` 이라는 **다른 축**이 들어간다

**우리 반입 클립에 이 규칙을 그대로 적용해야 한다**: 루프에는 `Enable_Warping`을 만들고,
정지/전환 클립에는 **만들지 않는 것이 정답**이다. 전 구간 1인 커브를 넣는 것보다 낫다.

### 3.3 커브는 3종이 아니라 5종이었다 [A]

`CLAUDE.md` P8과 `assets/...` 4.2절이 **`contact_l/r` + `Enable_Warping` 3종**만 필수로
적었는데, 실제 GASP 클립은 **모든 클립이 예외 없이** 아래를 갖고 있다:

```
contact_l / contact_r     발 심기          (문서에 있음)
enable_warping            워핑 게이트       (문서에 있음, Stop 계열은 제외)
movedata_speed            루트 이동 속력    ← 문서에 없었음
phase                     보행 위상        ← 문서에 없었음 (Arc/Strafe 계열은 제외)
```

`phase`는 이미 **[C-4]**(Stand↔Crouch DB 경계 위상 정합)에서 이름이 나왔던 그 커브다.
즉 **[C-4]를 풀려면 반입 클립에도 `phase`가 있어야 한다.**

### 3.4 모디파이어 체인 — GASP에 전부 있다 [A] · CDO 실측

| 모디파이어 | 네이티브 부모 | 생성물 | 실측 설정 |
|---|---|---|---|
| `AM_FootSpeed_L` | `MotionExtractorModifier` | 커브 **`FootSpeed_L`** | Bone **`ball_l`**, `TranslationSpeed`, `XYZ`, `ComponentSpace`, 30Hz, **`bNormalize=false`** |
| `AM_FootSpeed_R` | 〃 | `FootSpeed_R` | Bone `ball_r`, 나머지 동일 |
| `AM_MoveData_Speed` | 〃 | `MoveData_Speed` | Bone **`root`**, `TranslationSpeed`, **`XY`**, `ComponentSpace`, 30Hz |
| `AM_WarpingAlpha` | (BP) `AnimationModifier` | `Enable_Warping` | 30Hz, 임계 5°/5°, 블렌드 0.25s — P0-4a 문서 참고 |
| `AM_BakePhaseCurveFromFootstepNotifies` | (BP) `AnimationModifier` | 커브 **`phase`** | `Oscillate=true`, 왼발 **+1** / 오른발 **−1**, 노티파이 트랙 `Footstep Left` / `Footstep Right` 를 읽는다 |
| `AM_FootSteps_Walk` | `FootstepAnimEventsModifier` | 발자국 **노티파이 + 싱크마커** | `phase`의 **선행 조건** |
| `AM_Copy_IKFootRoot` | `CopyBonesModifier` | `ik_foot_*` 배치 | 본 쌍 복사 |

**의존 순서가 드러난다:**

```
AM_FootSteps_Walk  →  (Footstep Left/Right 노티파이 생성)  →  AM_BakePhaseCurveFromFootstepNotifies  →  phase
```

`phase`는 노티파이에서 파생되므로 **발자국 노티파이를 먼저 굽지 않으면 만들 수 없다.**

### 3.5 문서의 `contact_l/r` 생성 계획과 GASP 실물이 다르다 [A]

| | `assets/...` 4.2절 [4] 계획 | GASP `AM_FootSpeed_L` 실물 |
|---|---|---|
| 본 | `foot_l` / `foot_r` | **`ball_l` / `ball_r`** |
| 정규화 | `bNormalize=true` | **`false`** |
| 커브 이름 | `contact_l` / `contact_r` | **`FootSpeed_L` / `FootSpeed_R`** |

이름이 다르다 → **`FootSpeed_L`에서 `contact_l`로 가는 단계가 따로 있다**(폴더에 `AM_RenameCurve`,
`AM_RemoveCurves`, `AM_ReorderCurves`가 같이 있는 이유로 보인다). **(추정)**
또는 `contact_l/r`이 Epic의 원본 FBX에 이미 저작돼 있고 `FootSpeed_*`는 별도 용도일 수도 있다.
→ **[C-28]**

의미상으로도 반대다: "FootSpeed"는 **움직일 때 큼**, "contact"는 **심겼을 때 큼**이어야 한다.
ABP의 `RemapCurves`(3.3b절 / [U7])가 이 변환을 맡을 가능성이 있다. **(추정)**

## 4. 판정

**성공.** 반입 클립의 목표 상태를 항목 단위로 확정했다. [C-26]/[C-27]은 이제
"느낌"이 아니라 **이 표와 대조**해서 판정한다.

## 5. 이것이 바꾸는 것

| 문서 | 어떻게 |
|---|---|
| `CLAUDE.md` P8 | **커브 3종 → 5종**(+싱크마커), 그리고 **Stop 계열은 `Enable_Warping`을 만들지 않는다** |
| `assets/...` 4.2절 | [4]단계 파라미터 수정(ball vs foot, normalize), `phase`·`movedata_speed` 단계 추가, 의존 순서 명시 |
| `CURRENT_STATE.md` P0-4 | 체크리스트에 `movedata_speed`·`phase` 추가 |
| `OPEN_ITEMS.md` | **[C-28]** 신규 |

## 6. 막힌 것 / 다음에 확인할 것

| ID | 항목 | 왜 |
|---|---|---|
| **C-28** | `contact_l/r`의 **실제 생성 경로** | 7절에서 좁혀졌으나 미결 |

---

## 7. [C-28] 추적 (2026-09-03 추가) — 좁혀졌지만 아직 안 닫힘

### 7.1 Epic의 **정규 커브 순서**를 찾았다 [A]

`AM_ReorderCurves`의 CDO에 목록이 박혀 있다. 이것이 Epic이 정한 **표준 커브 세트**다:

```
CurvesToOrder = [ contact_l, contact_r, movedata_speed, enable_warping, Phase, steeringtargettime ]
SetColors     = true   (각 커브에 고정 색 지정 — 커브 에디터에서 눈으로 구분하려고)
```

3.3절에서 클립별로 관측한 것과 정확히 일치한다. **반입 클립도 이 순서·이 이름을 따르면 된다.**

### 7.2 `FootSpeed_*` 는 출하 클립에 **하나도 없다** [A]

Walk 클립 40개의 `CurveNameList`를 집계했다:

```
contact_l  28    contact_r  28    movedata_speed  28
enable_warping  28    phase  23    Phase  1    enable_strafewarping  4
FootSpeed_L / FootSpeed_R  →  0개
```

즉 `AM_FootSpeed_L/R`의 산출물은 **최종 에셋에 남지 않는다.** 중간 산출물이거나 다른 용도다.

> 부수 발견: **`enable_strafewarping`** 이라는 커브가 4개 클립에 더 있다(3.3절 표에 없던 것).
> 스트레이프 전용 워핑 게이트로 보인다. **(추정)**

### 7.3 남은 두 갈래

| 가설 | 근거 | 반증 |
|---|---|---|
| **(가)** `AM_FootSpeed_*` → `AM_RenameCurve`로 `contact_*`로 개명 | 폴더에 `AM_RenameCurve`가 같이 있고, `FootSpeed_*`가 출하물에 없다 | `MotionExtractorModifier`는 `CustomCurveName`을 직접 지정할 수 있다. `contact_l`을 원했으면 **처음부터 그렇게 적었을 것** |
| **(나)** `contact_l/r`은 Epic의 원본 FBX에 **저작**돼 있다 | (가)의 반증이 그대로 근거가 된다 | — |

**의미도 반대다**: `FootSpeed`는 *움직일 때* 크고, `contact`는 *심겼을 때* 커야 한다.
단순 개명이면 의미가 뒤집힌다. ABP의 `RemapCurves`([U7], 3.3b절)가 이 반전을 맡고 있어서
이름이 느슨한 것일 가능성도 있다. **(추정)**

### 7.4 어떻게 닫는가 — 에디터에서 1분

**`M_Relaxed_Walk_Loop_F`를 열어 `contact_l` 커브 모양을 본다.**

| 관측 | 결론 |
|---|---|
| 발이 **떠 있을 때(스윙)** 값이 큼 | 이건 speed다 → (가). 우리도 `AM_FootSpeed_*` + 개명으로 간다 |
| 발이 **땅에 있을 때(스탠스)** 값이 큼 | 진짜 contact다 → (나). **우리는 이 커브를 만들 수단이 아직 없다** |

**(나)로 판명되면 P0-4의 난이도가 올라간다** — 발 접지를 직접 판정하는 모디파이어가 따로 필요해진다.
그때는 `AM_FootSteps_Walk`(`FootstepAnimEventsModifier`)의 `groundThreshold`/`speedThreshold`
기반 접지 판정을 커브로 바꾸는 쪽을 먼저 본다.

### 아직 못 한 것 — 커브 **값**

이번 조회는 커브의 **이름**까지다. 실제 **값(모양)** 은 아직 못 떴다 —
`get_asset_tags`는 이름만 주고, 커브 값을 주는 MCP 툴을 아직 찾지 못했다.
"직선 루프는 1, pivot은 중앙이 0으로 파임"을 **수치로** 확인하려면 에디터에서 커브 트랙을
직접 보거나 다른 경로가 필요하다. → 다만 3.2절(Stop에 커브 자체가 없음) 덕분에
**구조적 확인은 이미 됐다.**

---

## 8. ★ 정정 — 커브는 "필수 N종"이 아니라 **클립 종류별 세트**다

3.3절에서 "모든 클립이 예외 없이 5종을 갖는다"고 썼는데 **틀렸다.** 표본이 Walk 폴더에
치우쳐 있었다. 제자리 회전 18개를 추가로 뜨자 그림이 바뀌었다.

### 8.1 실측 — 클립 종류 ↔ 커브 세트

| 클립 종류 | `contact_l/r` | `movedata_speed` | `enable_warping` | `phase` | 그 외 |
|---|---|---|---|---|---|
| 직선 루프 (`Walk_Loop_F`) | ✅ | ✅ | ✅ | ✅ | 루프는 `L`/`R` 싱크마커 |
| 출발 (`Walk_Start_F`) | ✅ | ✅ | ✅ | ✅ | |
| 피벗 (`Walk_Pivot_F_B`) | ✅ | ✅ | ✅ | ✅ | |
| 걸으며 회전 (`Walk_Turn_180_L`) | ✅ | ✅ | ✅ | ✅ | `steeringtargettime` |
| **정지 (`Walk_Stop_F`)** | ✅ | ✅ | **❌ 없음** | ✅ | `enable_turninplacesteering`, `steeringtargettime` |
| **제자리회전 90°/180°** | ✅ | ✅/일부없음 | **❌ 없음** | ❌ | `enable_turninplacesteering`, `steeringtargettime` |
| **제자리회전 45°/135°** | ✅ | ✅/일부없음 | **❌ 없음** | ❌ | 없음 |
| 호·스트레이프 (`Arc`, `Circle_Strafe`) | ✅ | ✅ | ✅ | ❌ | 일부 `enable_strafewarping` |

**유일하게 모든 클립에 있는 것은 `contact_l` / `contact_r` 뿐이다.**

### 8.2 규약 — "못 쓰는 축은 커브를 만들지 않는다"

`Walk_Stop`과 제자리회전 18개 **전부**에 `enable_warping`이 없다. 커브가 없으면
`Get Curve Value from Animation`이 0을 돌려주고 워핑 Alpha가 0이 된다(16절).

즉 Epic은 **워핑을 쓰면 안 되는 동작에 커브를 0으로 채워 넣는 게 아니라, 커브 자체를
안 만든다.** 이건 데이터 규약이지 우연이 아니다.

→ **우리도 정지·제자리회전 클립에는 `AM_WarpingAlpha`를 걸면 안 된다.**
(2026-09-03 `Turning_Right_90_Degrees_Anim`에 실수로 걸었다가 확인)

---

## 9. ★★ [C-28] 해결 — `contact_l/r`은 **이진 커브**다. 우리에겐 생성 수단이 없다

### 9.1 근거 — 커브 모양 실물 [A]

`M_Relaxed_Walk_Turn_180_L_Lfoot`의 커브 트랙을 눈으로 확인:

```
contact_l   ┌─┐ ┌─┐ ┌─┐    깔끔한 사각파 (0 ↔ 1)
contact_r  ─┘ └─┘ └─┘ └    좌우 교대, 위상 반대
```

**`UMotionExtractorModifier`의 `TranslationSpeed`는 연속값이라 사각파가 나올 수 없다.**
게다가 그 모디파이어에는 임계값 기능이 없다(`EMotionExtractor_MathOperation` =
None/Add/Sub/Div/Mul 뿐 — 비교 연산이 없다).

### 9.2 결론

| 가설 | 판정 |
|---|---|
| (가) `AM_FootSpeed_*` → `AM_RenameCurve`로 `contact_*` 개명 | **❌ 기각.** 연속값이 사각파가 될 수 없다 |
| (나) 원본 FBX에 저작됐거나 별도 수단으로 생성 | **✅ 채택** |

**`AM_FootSpeed_L/R`이 만드는 `FootSpeed_L/R`은 `contact_l/r`과 다른 것이다.**
(그래서 출하 클립 40개에 `FootSpeed_*`가 0개인 것도 설명된다 — 다른 용도의 중간 산출물)

### 9.3 이것이 새 블로커다 **[C-33]**

`contact_l/r`은 **모든 클립에 예외 없이 있는 유일한 커브**이고, GASP의 발 배치
(`FootPlacement` 노드)가 `RemapCurves`를 통해 이걸로 발 심기 타이밍을 결정한다(3.3b절).
**없으면 발 IK가 조용히 오작동한다**(`CLAUDE.md` P8).

그런데 **우리에겐 만들 방법이 없다.**

> **리스크 장부가 뒤집혔다.** 최대 리스크로 잡았던 `Enable_Warping`은 GASP에 생성기가
> 있어서 **공짜로 풀렸다**([C-25]). 반대로 쉽게 볼 수 있었던 `contact_l/r`이
> **진짜 블로커**가 됐다.

### 9.4 유력한 경로 (미검증)

`AM_FootSteps_Walk`(`FootstepAnimEventsModifier`)는 `groundThreshold` / `speedThreshold`로
**접지를 실제로 판정**하고 노티파이·싱크마커를 생성한다. 그리고
`AM_BakePhaseCurveFromFootstepNotifies`는 **노티파이를 읽어 커브(`phase`)를 굽는다.**

→ **같은 구조로 "노티파이 → `contact_l/r` 커브"를 만드는 모디파이어를 우리가 작성**하는 것이
가장 유력하다. `AM_BakePhaseCurveFromFootstepNotifies`를 복제해서 개조하는 것이 출발점.

---

## 10. 그래서 반입 파이프라인은 이렇게 바뀐다

```
[1] 비인플레이스 클립 확보                                    (14절)
[2] IK Retarget → SK_UEFN_Mannequin                         ✅ 검증됨
[3] AM_EncodeRootBone   (pelvis 1.0 / orientation pelvis-Y)  ✅ 검증됨
[4] AM_FootSteps_Walk   → 발자국 노티파이 + 싱크마커
[5] ??? → contact_l / contact_r                             ❌ 수단 없음 [C-33]
[6] AM_MoveData_Speed   → MoveData_Speed
[7] AM_BakePhaseCurve   → phase          (루프/전환 클립만)
[8] AM_WarpingAlpha     → Enable_Warping  ★ 직선계 클립만. 정지·제자리회전 제외 (8.2절)
[9] bEnableRootMotion = true → PSD 편입
```

**[5]가 뚫려야 P0-4가 끝난다.**
