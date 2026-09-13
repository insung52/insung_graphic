# Lyra 라이플 로코모션 이관 — 129클립

2026-09-09 / **진행 중** / Mixamo 대신 Epic 저작 Lyra 세트로 갈아탄다.
설계 `assets` 6절이 처음부터 지목했던 경로다.

관련: [C-46] 해결 · [C-51] 해결 예정 · [C-44] 측정 가능해짐
관련 문서: `2026-09-04_c34_clip_curve_mapping.md`(매핑표) · `2026-09-09_walk_quality_debugging.md`

---

## 1. 왜 갈아탔나

`2026-09-09_walk_quality_debugging.md`로 견착 전진 보행은 살렸지만, **클립이 1개**였다.
GASP가 227개로 내는 품질을 1개로 낼 방법은 없다. 사용자 목표는

> "현재 gasp 정도의 움직임 퀄리티 + 총을 견착한 상태, 조준한 상태까지 다 반영"

이고, 이는 **Start/Stop/Pivot/TurnInPlace가 방향별로 갖춰진 세트**를 요구한다.
Mixamo에는 그게 없다(피벗·관성 전이 클립이 아예 없다). Lyra에는 있다.

## 2. 반입 결과 [A]

`/Game/SoldierLab/Animations/Rifle` — **129개, 전부 AnimSequence**

```
Skeleton     SK_UEFN_Mannequin  129/129     리타깃 성공
루트모션      True 99 / False 30            보존됨
```

| 폴더 | 개수 | 내용 |
|---|---:|---|
| `Loops` | 12 | Walk/Jog/Crouch_Walk × Fwd·Bwd·Left·Right |
| `Starts` | 12 | 〃 |
| `Stops` | 12 | 〃 |
| `Pivots` | 12 | 〃 |
| `TurnInPlace` | 8 | Stand/Crouch × 90/180 × L/R |
| `Idles` | 5 | Idle_ADS · Idle_Hipfire · Crouch_Idle · IdleBreak ×2 |
| `Poses` | 17 | AimOffset 15 + OverridePose 2 |
| `_Extra` | 12 | Jump 6 · Lean 3 · Crouch Entry/Exit 2 · `Jog_Fwd_RAW` 1 |
| `_MF` | 39 | 여성 변형 — 나중에 |

`Jog_Fwd_RAW`는 미가공 원본으로 보여 `Loops`에서 뺐다 — DB에 중복으로 들어가면 안 된다 [B].

**Walk / Jog / Crouch 3스탠스가 4방향 × Start·Loop·Stop·Pivot으로 완비**되어 있다.
설계 5.5.4의 "3스탠스 60~90개" 요건을 처음으로 충족한다.

## 3. ★ Lyra는 루트가 이미 정확하다 — Mixamo와 결정적으로 다르다 [A]

`SoldierRootFacingModifier`를 `DiagnoseOnly`로 돌린 실측:

| 클립 | root yaw − travel | pelvis(Y) − travel |
|---|---|---|
| `MM_Rifle_Walk_Fwd` | **mean 0.0° / sd 0.0** | mean 4.9° / sd 2.7 |
| `MM_Rifle_Walk_Left` | **mean 90.0° / sd 0.0** | mean 160.7° / sd 3.1 |

**두 가지가 동시에 확인된다:**

1. **루트 facing 교정이 필요 없다.** 리타깃이 무손실이었다.
2. **우리 진단 공식이 옳다.** 옆걸음에서 정확히 90.0°가 나온 것이 그 증거다 —
   몸은 정면을 보고 왼쪽으로 이동하니 root−travel = 90°가 정답이다.
   (`2026-09-08_root_facing_56deg.md`의 90° 공식 오류가 재발하지 않았음을 교차 검증한다)

### 3.1 ⚠ `AM_EncodeRootBone`을 걸면 **안 된다**

매핑표 4절 1행은 전 클립 ✅이지만 **Lyra에는 예외다.**
`AM_EncodeRootBone`은 pelvis yaw를 루트 yaw로 덮어쓴다. Lyra 실측으로는

```
Walk_Fwd    루트 0.0°가 pelvis  4.9°로 덮여씀      → 5° 틀어짐
Walk_Left   루트 90.0°가 pelvis 160.7°로 덮여씀    → 71° 틀어짐 (파괴적)
```

**이미 맞는 루트를 골반 각도로 망가뜨리는 결과가 된다.**
매핑표 1행/1b행은 **"루트가 없거나 틀린 클립"(Mixamo) 전용**이었다.
→ 매핑표에 이 단서를 추가한다.

### 3.2 부수 관찰 — 정석적인 라이플 자세는 골반이 정면이다

Mixamo 클립의 골반 블레이드가 **34°**였던 것과 대비된다. Lyra Fwd는 **4.9°**다.
즉 Lyra는 **골반은 진행 방향, 상체만 무기 쪽**으로 저작돼 있다.
Orientation Warping이 다리를 틀 여지가 애초에 거의 없다 — 이것이 견착 이동이
자연스러운 이유이고, Mixamo에서 우리가 싸웠던 문제의 근원이 저작 품질이었음을 보여준다.

## 4. 일괄 설정 — MCP로 완료 [A]

```
루프 지정   25개   Walk·Jog·Crouch_Walk 4방향 + Idle_ADS/Hipfire/Crouch_Idle
1회성       89개   Start·Stop·Pivot·Turn·Jump·IdleBreak
AO 포즈     15개   애디티브 설정 보존 (건드리지 않음)
커브 압축   129개  → SandboxAnimCurveCompressionSettings
bForceRootLock     루트모션 보유 클립에 true
```

`bLoop` 판정식: `^(MM|MF)_Rifle_(Crouch_)?(Walk|Jog)_(Fwd|Bwd|Left|Right)$` + Idle 루프 3종.
**`IdleBreak`은 false**다(매핑표 주2).

## 5. ★ 모디파이어 스택 — Lyra판

매핑표 4절에서 1행·1b행을 뺀 것이다(3.1절). 폴더 하나 = Ctrl+A 한 번 = 스택 하나.

| 스택 | 폴더 | 모디파이어 (이 순서로 Add) |
|---|---|---|
| **A** | `Loops` `Starts` `Pivots` | `AM_Copy_IKFootRoot` → `AM_FootSteps_Walk` → `AM_BakePhaseCurveFromFootstepNotifies` → `FootContactCurveModifier` → `AM_MoveData_Speed` → `AM_WarpingAlpha` |
| **B** | `Stops` | 스택 A에서 **`AM_WarpingAlpha` 제외** |
| **C** | `TurnInPlace` | `AM_Copy_IKFootRoot` → `FootContactCurveModifier` → `AM_MoveData_Speed` → `TurnInPlaceCurvesModifier` |
| **D** | `Idles` | `AM_Copy_IKFootRoot` → `FootContactCurveModifier` → `AM_MoveData_Speed` |
| — | `Poses` `_Extra` `_MF` | 걸지 않는다 (매핑표 ⑧) |

### 5.1 GASP 원본을 그대로 쓴다 — `_Soldier` 변종 불필요 [A]

CDO 전수 비교 결과:

| 자산 | GASP 원본과의 차이 |
|---|---|
| `AM_WarpingAlpha_Soldier` | **차이 없음** (전 프로퍼티 동일) |
| `AM_FootSteps_Walk_Soldier` | 싱크마커만 ON (`L`/`R`, FootBoneSpeed) |

매핑표 4.3절대로 **싱크마커는 모션 매칭이 안 쓴다**(996개 중 4개만 보유, 블렌드스페이스용).
따라서 마커를 켤 이유가 없고, GASP 원본을 쓰면 `_Walk`/`_Run`/`_Crouch` 3속도대가
**설정 없이 바로** 확보된다.

### 5.2 `AM_FootSteps` 변종은 1차 반입에서 통일한다 [B]

원칙대로면 Walk→`_Walk`, Jog→`_Run`, Crouch→`_Crouch`이지만, 변종 간 차이는
**발소리 노티파이 클래스뿐이다**(매핑표 4.2절). 커브 결과·MM·발 IK에 영향이 **0**이므로
1차에서는 `AM_FootSteps_Walk`로 통일하고, 발소리는 나중에 해당 부분집합만 재적용한다.
선택 작업이 12회 → 4회로 준다.

### 5.3 `TurnInPlaceCurvesModifier`는 `TurnInPlace`에만 건다 [B]

매핑표 8행은 ②출발~⑥제자리회전에 ✅이지만, 그건 GASP의 **Reface** 계열
(`Walk_Reface_Start/Stop`)을 반영한 것이다. Lyra 세트에는 Reface 클립이 없고
Start/Stop은 직진, Pivot은 방향 전환이다.

매핑표 3.3절 **"애매하면 빼고"** — 커브를 잘못 넣으면 기능이 **켜지고**,
빠뜨리면 꺼질 뿐이다. 1차는 `TurnInPlace` 8개로 한정한다.
Pivot에 스티어링이 필요한지는 PIE에서 보고 판정한다.

## 6. 모디파이어 적용 결과 — 61클립 전부 정상 [A]

`CurveNameList`는 **세미콜론 구분**이다(쉼표로 파싱하면 전부 "누락"으로 오판한다).
폴더별로 관측된 **서로 다른 커브 조합이 1종**이므로 클립 간 편차가 없다:

| 폴더 | 커브 |
|---|---|
| `Loops` | `phase` `contact_l` `contact_r` `movedata_speed` `enable_warping` |
| `Starts` `Pivots` | 〃 + `distance` |
| `Stops` | `distance` `phase` `contact_l/r` `movedata_speed` — **워핑 없음** (의도대로) |
| `TurnInPlace` | `contact_l/r` `movedata_speed` `steeringtargettime` `enable_turninplacesteering` + `remainingturnyaw` `turnyawweight` |
| `Idles` | `contact_l/r` `movedata_speed` |

**Lyra가 자체 커브를 갖고 왔다** — `distance`(디스턴스 매칭) · `remainingturnyaw` ·
`turnyawweight` · `disablelhandik`(1클립). 우리 `PSS_Default`가 안 읽으므로 무해하다(매핑표 3.3절).
나중에 디스턴스 매칭을 쓸 여지가 생긴 셈이다.

## 7. ★ PSD 구조 — GASP `Dense` 세트를 그대로 따른다

우리 CMC 경로가 쓰는 것은 `Dense` 세트다. Lyra의 재고가 여기에 **정확히 맞아떨어진다**
(Lyra `Jog` = GASP `Run` 속도대).

### 7.1 Epic의 Dense 설정 실측 [A]

DB 종류마다 스키마·비용 편향이 다르다. **복제하되 고치지 않는다** — 3절과 같은 이유로
Epic 데이터에는 편차가 있지만(예: `Crouch_Walk_Stops`만 `PSS_Stop`이 아니라 `PSS_Default`),
동작하는 설정을 베끼는 편이 "고치는" 것보다 안전하다.

| DB | 스키마 | contPose | base | loop | tags |
|---|---|---:|---:|---:|---|
| `Stand_Idles` | **`PSS_Idle`** | −0.01 | **+0.10** | **−0.10** | — |
| `Stand_Walk_Loops/Starts/Pivots` | `PSS_Default` | −0.01 | 0 | −0.005 | — |
| `Stand_Walk_Stops` | **`PSS_Stop`** | **−0.30** | 0 | −0.005 | `Stops` |
| `Stand_Jog_Loops` | `PSS_Default` | −0.01 | 0 | −0.005 | — |
| `Stand_Jog_Starts` | `PSS_Default` | **−0.05** | 0 | −0.005 | — |
| `Stand_Jog_Stops` | **`PSS_Stop`** | **−0.30** | 0 | −0.005 | `Stops` |
| `Stand_Jog_Pivots` | `PSS_Default` | **−0.05** | 0 | −0.005 | `Pivots` |
| `Stand_TurnInPlace` | `PSS_Default` | **−0.05** | **−0.20** | −0.005 | `TurnInPlace` |
| `Crouch_Idles` | **`PSS_Idle`** | −0.01 | 0 | −0.005 | — |
| `Crouch_Walk_*` | `PSS_Default` | −0.01 | 0 | −0.005 | Stops만 `Stops` |
| `Crouch_TurnInPlace` | `PSS_Default` | −0.01 | 0 | −0.005 | `TurnInPlace` |

정규화 세트는 전부 `PSN_Dense_All`, 검색 모드 `PCAKDTree`(주성분 4).

**읽는 방식**: `continuingPoseCostBias`가 음수일수록 "지금 재생 중인 클립에 계속 머무른다".
Stops의 −0.30은 **정지 동작을 중간에 끊지 않겠다**는 뜻이고, TurnInPlace의 `base` −0.20은
**회전 DB 자체를 우선 선택**하게 만든다.

### 7.2 생성한 16개 [A]

`/Game/SoldierLab/PoseSearch/Rifle/` — `PSD_Soldier_Walk_Test`를 복제해 MCP로 설정 주입.
(원본이 이미 `PSS_Default` + `PSN_Dense_All` + Epic 기본 편향이라 복제원으로 정확했다)

| DB | 채울 클립 | 개수 |
|---|---|---:|
| `PSD_Rifle_Stand_Idles` | `Idles/` 중 `Idle_ADS` `Idle_Hipfire` `IdleBreak_Fidget` `IdleBreak_Scan` | 4 |
| `PSD_Rifle_Stand_Walk_Loops` | `Loops/MM_Rifle_Walk_*` | 4 |
| `PSD_Rifle_Stand_Walk_Starts` | `Starts/MM_Rifle_Walk_*` | 4 |
| `PSD_Rifle_Stand_Walk_Stops` | `Stops/MM_Rifle_Walk_*` | 4 |
| `PSD_Rifle_Stand_Walk_Pivots` | `Pivots/MM_Rifle_Walk_*` | 4 |
| `PSD_Rifle_Stand_Jog_Loops` | `Loops/MM_Rifle_Jog_*` | 4 |
| `PSD_Rifle_Stand_Jog_Starts` | `Starts/MM_Rifle_Jog_*` | 4 |
| `PSD_Rifle_Stand_Jog_Stops` | `Stops/MM_Rifle_Jog_*` | 4 |
| `PSD_Rifle_Stand_Jog_Pivots` | `Pivots/MM_Rifle_Jog_*` | 4 |
| `PSD_Rifle_Stand_TurnInPlace` | `TurnInPlace/MM_Rifle_Turn*` | 4 |
| `PSD_Rifle_Crouch_Idles` | `Idles/MM_Rifle_Crouch_Idle` | 1 |
| `PSD_Rifle_Crouch_Walk_Loops` | `Loops/MM_Rifle_Crouch_*` | 4 |
| `PSD_Rifle_Crouch_Walk_Starts` | `Starts/MM_Rifle_Crouch_*` | 4 |
| `PSD_Rifle_Crouch_Walk_Stops` | `Stops/MM_Rifle_Crouch_*` | 4 |
| `PSD_Rifle_Crouch_Walk_Pivots` | `Pivots/MM_Rifle_Crouch_*` | 4 |
| `PSD_Rifle_Crouch_TurnInPlace` | `TurnInPlace/MM_Rifle_Crouch_Turn*` | 4 |

합 **61클립**.

### 7.3 ⚠ `animationAssets`는 MCP로 못 건드린다 [A]

`TArray<FInstancedStruct>`라 **읽기·쓰기 둘 다 실패**한다:

```
GetObjectProperties ... the following properties could not be read: animationAssets
SetObjectProperties ... the following properties could not be set: animationAssets
```

→ **클립 목록만은 에디터에서 손으로 넣어야 한다.** 나머지(자산 생성·스키마·비용 편향·태그)는
전부 MCP로 끝난다. → `CLAUDE.md` 6.1절에 추가

복제원의 `Walking_Anim` · `Rifle_Aiming_Idle_Anim` 2개가 16개 전부에 딸려 들어가 있으므로
채우기 전에 지운다.

### 7.4 채우기 검증 — `get_dependencies`로 우회한다 [A]

`animationAssets`를 못 읽어도 **참조는 읽힌다.** 16개 DB 전부 의도한 클립만 들어갔음을 확인했다:

```
클립 61 + (스키마·정규화 2 × 16 DB) = 93개 참조,  missing 0
```

→ 못 읽는 프로퍼티라도 **참조 그래프로 검수할 수 있다.**

## 8. ★ 챙터 — `CHT_Soldier_Databases` [C-51]

### 8.1 ★ 챙터가 DB를 여러 개 내는 원리 — 실측 [A]

> ⚠ 초안에 **"행 하나가 PSD 목록을 반환한다"**고 적었는데 **틀렸다.** 확인 없이 단정했다.

**(1) 행 결과 타입은 세 가지뿐이고 목록 타입은 없다** (`ObjectChooser_Asset.h` · `Chooser.h:220,240`):

```
Asset             자산 1개 (하드 참조)
Nested Chooser    같은 에셋 안에 박힌 하위 표
Evaluate Chooser  다른 챙터 에셋
```

**(2) 챙터는 첫 매치에서 멈추지 않는다** (`Chooser.cpp:712-713`):

```cpp
// of the rows that passed all column filters, iterate through them calling the callback until it returns Stop
for (FChooserIndexArray::FIndexData& SelectedIndexData : *IndicesOut)
```

→ **모든 열 필터를 통과한 모든 행**의 결과가 수집돼 `SetDatabasesToSearch`로 넘어간다
(`AnimNode_MotionMatching.h:175` `TArray<...> DatabasesToSearch`, `:109` 주석).

**(3) 그래서 GASP는 `Nested Chooser`를 쓴다.** 행 결과를 하위 표로 두고 그 안에 **필터 열 없이**
DB를 행마다 하나씩 넣으면 안쪽 행이 전부 통과해 여러 DB가 나온다.
근거: `CHT_PoseSearchDatabases_Dense`의 참조가 **PSD 35개 · 외부 챙터 0개** — 7행짜리 표가
35개를 참조하는데 외부 챙터를 안 쓰므로 결과가 에셋 내부에 박혀 있다는 뜻이다 [A].

(최상위 `CHT_PoseSearchDatabases`는 반대로 PSD 0개 · 하위 챙터 3개 = LOD 분기용 `Evaluate Chooser`)

### 8.2 ★ 우리 구조 — 중첩 없이 **16행**

(2)를 그대로 쓰면 **Nested Chooser가 필요 없다.** DB 하나당 행 하나를 두고
같은 조건의 행들이 함께 통과하게 하면 된다. 사용자가 드롭다운으로 PSD만 고르면 되므로
중첩 표를 만드는 것보다 훨씬 단순하다.

| 행 | Stance | MovementState | Gait | Result (자산 1개) |
|---|---|---|---|---|
| 1–2 | Stand | Idle | ANY | `Stand_Idles` / `Stand_TurnInPlace` |
| 3–6 | Stand | Moving | **= Walk** | `Stand_Walk_` Loops / Starts / Stops / Pivots |
| 7–10 | Stand | Moving | **≠ Walk** | `Stand_Jog_` Loops / Starts / Stops / Pivots |
| 11–12 | Crouch | Idle | ANY | `Crouch_Idles` / `Crouch_TurnInPlace` |
| 13–16 | Crouch | Moving | ANY | `Crouch_Walk_` Loops / Starts / Stops / Pivots |

**행은 상호배타여야 한다.** 초안의 5행 설계는 2행(`Walk`)과 3행(`ANY`)이 걷는 중에 **동시에**
통과해 Walk·Jog DB를 같이 검색하는 오류가 있었다. `MatchNotEqual`(`EnumColumn.h:20`)로
`≠ Walk`를 쓰면 Run·Sprint를 함께 받아 배타성과 Sprint 커버를 동시에 얻는다.

### 8.3 ⚠ 열 값은 행 개수가 정해진 **뒤에** 넣어야 한다

`columnsStructs`는 런타임 프로퍼티라 MCP로 쓸 수 있지만, 행 개수는 에디터 전용
`ResultsStructs`가 정한다. 그리고 `SetNumRows`는 **초과분을 잘라낸다**:

```cpp
// IChooserColumn.h:138-141
virtual void SetNumRows(int32 NumRows) override
{ if (NumRows <= RowValuesProperty.Num()) { ... 잘라냄 ... } }
```

3행짜리 표에 5행분 열 값을 미리 넣어봐야 3개로 잘린다. → **순서: 사용자가 행 16개 + 결과 지정
→ 그 다음 MCP로 필터 셀 48개 주입.**

컨텍스트는 `SoldierCharacter_ABP_C`, 출력 타입은 `PoseSearchDatabase`로 이미 맞다.
직전 상태(`MMDatabaseLOD` 1열 3행)는 `CHT_Soldier_Databases_BAK`에 백업해 뒀다.

### 8.3 아직 안 한 것 — 견착/총내림 분기

Lyra는 `Idle_ADS`(조준)와 `Idle_Hipfire`(총내림에 가까움)를 **둘 다** 준다.
지금은 둘 다 `Stand_Idles`에 들어가 있어 **MM이 포즈 유사도로 고른다** — 의도로 고르지 않는다.
의도로 몰려면 DB를 나누고 `RotationMode` 열을 추가해야 한다. → **[C-55]**

로코모션이 도는 것을 먼저 확인한 뒤에 판단한다. 지금 나누면 교란 변수가 하나 더 생긴다.

## 8.4 ⚠ 리타깃은 **Lyra 모디파이어까지 들고 온다** [A]

리타깃 클립에 Lyra의 모디파이어가 **우리 것보다 앞에** 붙어 온다:

```
DistanceCurveModifier   FootFXAnimModifier   AnimModTest
TurnYawAnimModifier     SyncMarkerAnimModifier
```

이것들이 `distance` · `remainingturnyaw` · `turnyawweight` 커브의 출처였다(6절).
런타임엔 무해하지만 **`Apply All`을 누르면 우리 것보다 먼저 돌아** 결과를 망칠 수 있다.

발단은 `FootstepEffectTagModifier`의 컴파일 실패였다 — `/Script/LyraGame`(Lyra C++ 게임 모듈)의
`LyraContextEffectAnimNotify`·`FLyraContextEffectAnimNotify*Settings`·`SetParameters`를 참조하는데
우리 프로젝트엔 그 모듈이 없다. **고칠 수 없고 삭제가 답이다.** `FootFXAnimModifier`가 그 자식이라
우리 클립까지 참조 사슬에 걸렸다.

### 제거 방법 두 가지

| 대상 | 방법 |
|---|---|
| 로드되는 4개 (`DistanceCurve`·`FootFX`·`TurnYaw`·`SyncMarker`) | 콘텐츠 브라우저 다중 선택 → **Remove Modifiers** |
| **`AnimModTest`** — 클래스 에셋이 프로젝트에 **없다**(`find_assets` 0건) | 목록에 뜨지 않는다. **MCP로 배열 조작** |

> ★ **고아 모디파이어 인스턴스**: 클래스 에셋 없이 인스턴스만 남으면 Remove 창이 나열조차 못 한다.
> `animationModifierInstances`를 `[]`로 비운 뒤 **남길 것들의 기존 refPath를 그대로 다시 쓰면** 된다
> (클래스 경로로는 인스턴스를 만들 수 없지만, **이미 존재하는 서브오브젝트 참조는 쓸 수 있다**).
> 129개 중 86개를 이 방법으로 정리했다. → `CLAUDE.md` 6.1c
>
> ⚠ **하지만 배열에서 빼도 인스턴스 서브오브젝트는 패키지에 고아로 남아 클래스 참조를 붙든다** [A].
> 에디터 경로는 `RemoveAnimationModifierInstance()`로 파괴하지만 MCP는 배열만 건드린다:
>
> ```
> MM_Rifle_TurnLeft_90  (에디터 Remove)  →  get_dependencies에 Lyra 참조 없음  ✅
> MM_Rifle_Walk_Fwd     (MCP 배열 조작)  →  FootstepEffectTagModifier 참조 잔존 ⚠
> ```
>
> **스택 자체는 정상**이라 Apply 동작에는 영향이 없다. 남는 것은 참조 껍데기뿐이고,
> 에디터 재시작 후 재저장하거나 [C-56]에서 Lyra 폴더를 지우면 함께 정리된다.
> → **가능하면 에디터 경로를 쓰고, MCP는 창에 안 뜨는 고아 인스턴스 전용으로 쓸 것.**

> ⚠ **`Remove Modifiers`는 `RevertFromAnimationSequence`를 먼저 부른다**
> (`SAnimationModifierContentBrowserWindow.cpp:637`). 즉 **커브가 지워진다.**
> Lyra 커브 3종은 우리가 안 읽으므로 손실이 없지만, **제거 후에는 우리 스택을 재적용해야 한다.**
> 반면 MCP 배열 조작은 Revert를 부르지 않아 커브가 보존된다.

### 정리 후 최종 상태 [A]

```
외래 모디파이어 0개 · 폴더마다 스택 1종 · 커브 누락 0
Loops/Starts/Pivots  Copy_IKFootRoot > FootSteps_Walk > BakePhase > FootContact > MoveData > WarpingAlpha
Stops                〃 (WarpingAlpha 없음)
TurnInPlace          Copy_IKFootRoot > FootContact > MoveData > TurnInPlaceCurves
Idles                Copy_IKFootRoot > FootContact > MoveData
```

## 8.5 PIE 1차 — 두 가지 증상과 원인 [A]

### 증상 A: 이동 중 발이 끌린다 → **속도 설정 불일치.** [C-52] 동시 해결

클립의 `movedata_speed` 커브를 에디터에서 호버해 실측(매핑표 5.0b의 방법):

```
MM_Rifle_Walk_Fwd   291.31 cm/s
MM_Rifle_Jog_Fwd    582.62 cm/s
```

우리 설정은 `walkSpeeds=(80,72,60)` · `runSpeeds=(500,350,300)`이었다.

> ★ **[C-52]의 정체**: "`walkSpeeds`를 낮춰도 이동속도가 안 변한다"의 답은
> **캐릭터의 기본 `gait`가 `Run`**이라는 것이다. `walkSpeeds`는 **한 번도 쓰인 적이 없었다.**
> 실제로 쓰이던 건 `runSpeeds`(500)이고, Jog 클립은 583로 저작돼 있었다 →
> 발이 땅보다 14% 느려서 끌렸다. **→ [C-52] 해결**

측정값으로 교체(Y·Z는 GASP 비율 유지):

```
walkSpeeds  (80,72,60)    →  (291.31, 262.18, 218.48)
runSpeeds   (500,350,300) →  (582.62, 407.83, 349.57)
```

결과: **관성·감속까지 포함해 견착 이동이 자연스러워졌고 발 미끄러짐이 사라졌다.**

> 남은 것: 옆·뒤·앉기 속도는 GASP 비율로 추정한 값이다.
> `Walk_Left`/`Walk_Bwd`/`Crouch_Walk_Fwd`의 커브를 재면 정확해진다.

### 증상 B: Idle에서 0.5~1초마다 움찔 + 왼쪽으로 조금씩 회전

**ABP 프리뷰에서도 났다** — AI도 StateTree도 없는 환경이므로 원인은 데이터/ABP 안이다.

분리 실험이 결정적이었다: Chooser에서 **Result가 `PSD_Rifle_Stand_TurnInPlace`인 행만 비활성화**하니
증상이 사라졌다. Idle DB를 1개로 줄이는 것으로는 안 사라졌다 → **선택 후보 수가 아니라 회전 DB 진입**.

조사한 것과 결과:

| 후보 | 판정 |
|---|---|
| Idle DB에 클립 4개(ADS·Hipfire·IdleBreak 2) | ❌ 1개로 줄여도 증상 유지 |
| Idle 클립의 `bForceRootLock = false` | ⚠ **실제 버그였고 고쳤다**(13개) — 루트 회전 누적. 하지만 움찔거림의 원인은 아님 |
| 정규화 세트가 GASP 것 | ⚠ **실제 버그였고 고쳤다**(P19) — 하지만 증상은 유지 |
| 진입 차단 노티파이 구간이 넓다 | ❌ 로그 실측 결과 **우리 30% vs GASP 22%** — 문제 아님 |
| **비용 편향** | ✅ **원인** |

#### ★ 원인 — Epic의 편향은 "클립이 많다"를 전제한다

```
PSD_Rifle_Stand_Idles        baseCostBias = +0.10   (패널티)
PSD_Rifle_Stand_TurnInPlace  baseCostBias = −0.20   (할인)
                             → 회전 DB가 0.30 유리
```

GASP는 클립이 227개라 "idle보다 잘 맞는 게 늘 있다"는 뜻이고 정상 동작한다.
**우리는 idle이 1개다.** 패널티를 주면 이길 수가 없어 MM이 계속 회전 클립으로 튀었고,
회전 클립 진입 창이 0~0.5초라 **움찔 주기(0.5~1초)와 정확히 일치**했다.

둘 다 **0.0**으로 놓자 해결. → **`CLAUDE.md` P18**

> **교훈**: 스키마·정규화 구조·검색 모드는 Epic 것을 복제해도 되지만,
> **비용 편향은 우리 데이터 밀도로 다시 잡아야 한다.** 같은 숫자가 밀도에 따라 반대로 작동한다.

#### 관찰 — 회전 클립의 64%는 정지 구간

```
MM_Rifle_TurnLeft_90   길이 1.667s · 회전 완료 0.600s · 진입차단 시작 0.500s
```

회전이 끝난 뒤 1초 넘게 서 있는 구간이 DB에 들어 있다. 진입은 차단되므로 지금은 문제가 없지만,
다시 불안정해지면 PSD의 **클립별 샘플링 구간**을 `[0, 0.7s]`로 잘라 후보에서 빼면 된다. → **[C-57]**

## 9. 다음

| 순서 | 항목 |
|---|---|
| 1 | ~~스택 A~D 적용~~ **완료** |
| 2 | ~~PSD 16개 생성·설정·채우기~~ **완료** (61클립 검증됨) |
| 3 | 챙터 16행 — 사용자가 행·결과 지정 → 내가 필터 셀 48개 주입 (8.2·8.3절) |
| 4 | PIE 확인 — 견착 보행·출발·정지·피벗·제자리회전 |
| 5 | **[C-55]** 견착/총내림을 의도로 분기 |
| 6 | **[C-44]** 급선회 판정 (이제 Pivot 클립이 있으므로 측정 가능) |

## 7. 열린 것

| ID | 항목 |
|---|---|
| **C-53** | `_Extra`의 Crouch Entry/Exit·Jump·Lean을 쓸 것인가 — 설계에 앉기/점프가 아직 없다 |
| **C-54** | `Poses`의 AimOffset 15장으로 AO를 만들 것인가, GASP AO를 재사용할 것인가 |
| **C-56** | **`/Game/Characters/Heroes` 의 Lyra 원본 266개를 어떻게 할 것인가.** `FootstepEffectTagModifier` 등이 `/Script/LyraGame` 참조로 **영구 컴파일 실패** 상태다. `ABP_RifleAnimLayers` 같은 Lyra ABP들도 같은 이유로 깨져 있을 것이다. 리타깃은 끝났으므로 원본은 불필요하지만 `RTG_Lyra_to_UEFN`이 참조 중이라 재리타깃 가능성과 저울질이 필요하다. **쿠킹·패키징 시 문제가 된다** |
| [C-50] | 레벨 인스턴스 오버라이드 전수 점검 (미결) |
| [C-52] | `WalkSpeeds`가 구동값이 아니라 상한처럼 동작하는 이유 (미결) |

---

## 9. 이동 속도 불일치 — [C-58] 해결 (2026-09-11) [A]

### 9.1 증상

조준 상태로 **옆·뒤·뒤대각선** 이동 시 발이 떨리며 끌렸다. 앞 방향만 멀쩡했다.
`a.AnimNode.MotionMatching.DebugDrawInfoVerbose 1`로 보니 원인이 즉시 드러났다 —
**옆·뒤 방향에서는 Loop가 아예 선택되지 않고** Start/Stop/Pivot이 한 걸음마다 재선택되고 있었다.

```
오른쪽 달리기   Left_Pivot ↔ Right_Start 교대
왼쪽  달리기   Left_Start 만 반복
뒤로  달리기   Bwd_Start + Fwd_Pivot 이 거의 한 발걸음마다 교대
A+S / S+D     Bwd_Jog_Stop 이 한 걸음마다 (가끔 Left/Right_Stop 섞임)
```

### 9.2 원인 — 캡슐 속도 vs 클립 저작 속도

`MoveData_Speed` 커브 실측 결과 **Lyra 라이플 세트는 전 방향 단일 속도**였다
(방향 간 오차 0.001):

| | 클립 저작 | 당시 캡슐 | 어긋남 |
|---|---|---|---|
| Jog 앞 | 582.62 | 582.62 | **0%** ← 유일하게 정상이던 방향 |
| Jog 옆 | 582.62 | 407.83 | −30% |
| Jog 뒤 | 582.62 | 349.57 | −40% |
| Walk 옆 | 291.31 | 262.18 | −10% |
| Walk 뒤 | 291.31 | 218.48 | −25% |
| Crouch 전방향 | 291.31 | 225/200/180 | −23~38% |

`playRate` 클램프가 0.85–1.15라 −30%/−40%는 **흡수가 구조적으로 불가능**하다.
그래서 Loop는 영영 지고, 가감속 구간이라 0~최고속의 모든 속도 프레임을 가진
Start/Stop/Pivot이 매번 이겼다. → **P31**

**틀린 값의 출처는 우리다.** GASP 원본 `walkSpeeds (200,180,150)` ·
`runSpeeds (500,350,300)`의 **비율 1:0.9:0.75 / 1:0.7:0.6만 옮겨왔는데**,
그 비율은 GASP 클립이 방향별로 다른 속도로 저작돼 있기 때문에 성립하는 값이었다. → **P30**

### 9.3 수정

```
walkSpeeds    (291.31, 262.18, 218.48)  →  (291.31, 291.31, 291.31)
runSpeeds     (582.62, 407.83, 349.57)  →  (582.62, 582.62, 582.62)
crouchSpeeds  (225,    200,    180)     →  (291.31, 291.31, 291.31)
```

재인덱싱·컴파일 불필요. **해결 확인됨** — 전 방향 Loop 정상 선택, A+S 떨림 소멸.

### 9.4 빗나간 가설 3개 — 기록

| 가설 | 검증 | 결과 |
|---|---|---|
| MM이 Bwd↔Left를 교대 선택 | `continuingPoseCostBias` −0.01 → **−0.5** | ❌ 무변화. 머물 Loop 자체가 안 뽑히니 당연했다 |
| OrientationWarping의 135° 반전 | `a.AnimNode.OrientationWarping.Enable 0` | ❌ 무변화 |
| FootPlacement / 발 고정 | `a.AnimNode.FootPlacement.Enable(.Lock) 0` | ❌ 무변화 |

ABP도 무죄였다 — `OrientationWarping`·`MotionMatching` 노드를 GASP와 전 프로퍼티 diff한 결과
`groupRole`(무의미, `method=DoNotSync`) 외 **완전 동일**. → P17이 또 통했다.

**판정 기준 자체가 틀렸던 것이 핵심이다.** "발이 안 미끄러지면 속도는 맞다"고 보고
[C-58]을 건너뛰자고 안내했으나, MM이 Start/Stop으로 갈아끼워 미끄러짐을 감추고
떨림으로 내보내기 때문에 **미끄러짐 부재는 증거가 못 된다.** → P31

### 9.5 남은 것

- `sprintSpeeds` 700 ÷ 582.62 = **1.20×** 로 아직 클램프(1.15) 밖이다.
  Lyra에 스프린트 클립이 없어 Sprint도 Jog DB를 쓴다. **670**이면 범위 안. 게임플레이 판단 대기
- 뒷걸음질이 전진과 같은 속도가 됐다. 방향별 속도차를 되살리려면
  ① `playRate` 하한 확대(0.6배속은 슬로모션처럼 보임) ② 느린 Loop 리타임 추가 ③ 유지

## 10. 웅크리기 제자리회전 발작 — P18 재발 (2026-09-11) [A]

미조준 상태에서 마우스를 돌리면 웅크린 캐릭터가 1초마다 회전 클립으로 튀며 안절부절못했다.
2026-09-09에 **Stand에서 고쳤던 바로 그 증상**인데 Crouch는 테스트를 안 해 남아 있었다.

```
                idle 클립   회전 클립   회전 baseCostBias
우리  Stand        4개         4개        −0.05
우리  Crouch       1개 ★       4개        −0.05 ★   ← idle이 이길 방법이 없다
GASP  Crouch       4개        10개          0   ★
```

`PSD_Rifle_Crouch_Idles`는 `MM_Rifle_Crouch_Idle` **단 1개**다.
Lyra에 웅크리기 idle 루프가 그것뿐이라 클립 추가는 불가능하다
(`MM_Rifle_Crouch_Idle_AO_*` 15개는 조준 오프셋 포즈이지 idle 루프가 아니다).
게다가 **`Crouch_TurnInPlace`의 −0.05는 우리가 넣은 값이고 GASP은 0을 준다.**

### 수정 — P18대로 편향을 우리 밀도에 맞춤

```
PSD_Rifle_Crouch_TurnInPlace  baseCostBias     −0.05  →  0       (GASP과 일치시킴)
PSD_Rifle_Crouch_Idles        loopingCostBias  −0.005 →  −0.10   (Stand_Idles와 동일)
```

두 번째는 GASP(−0.005)와 다르다. **의도적이다** — GASP은 웅크리기 idle이 4개고 우리는 1개라,
같은 값으로는 같은 효과가 안 난다. P18의 "편향은 우리 밀도로 다시 잡을 것"을 그대로 적용한 것.

여전히 튀면 다음 단계는 `Crouch_TurnInPlace.baseCostBias`에 **양수 패널티**(+0.05~+0.10)다.
