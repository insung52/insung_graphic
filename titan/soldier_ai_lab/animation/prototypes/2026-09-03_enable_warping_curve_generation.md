# P0-4a · `Enable_Warping` 커브 생성 방법

2026-09-03 / **성공 (문서 검증 단계)** / GASP가 이미 생성 모디파이어를 갖고 있다. 자체 제작 불필요.

관련 항목: **[C-25]** / 관련 문서: `animation/2026-09-02_pose_pipeline_spec.md` 11절 ·
`assets/2026-09-02_asset_supply_and_collaboration.md` 4b · `animation/2026-09-02_gasp_abp_analysis.md` 16절

> **범위 주의**: 이 기록은 **"생성 수단이 존재하는가"** 까지다(에셋 실측·[A]).
> **"우리가 반입한 Mixamo 클립에서 쓸 만한 커브가 나오는가"** 는 아직 안 봤다 → 6절 [C-26].

---

## 1. 무엇을 확인하려 했나

`Enable_Warping` 커브가 없으면 Orientation Warping의 `Alpha`가 영원히 0이라 방향 커버가 0이 된다
(`animation/2026-09-02_gasp_abp_analysis.md` 16절). 설계에서는 **"루트 본 `RotationSpeed`를 `UMotionExtractorModifier`로
뽑아 임계값 이하를 1로 만드는 방식이 유력"** 이라고만 적어 두고 미검증으로 남겼다.

**판정 기준**: 반입 클립에 `Enable_Warping` 커브를 **자동으로** 붙일 수단이 있는가.
(수작업 키 편집밖에 없으면 실패 — 클립 수만큼 비용이 곱해진다)

## 2. 어떻게 했나

MCP(`editor_toolset`)로 프로젝트 콘텐츠를 조회. 에디터를 띄운 채 원격 질의만 했고 에셋은 건드리지 않았다.

```
1) 파일시스템에서 모디파이어 후보 탐색
   Content/Blueprints/AnimModifiers/  →  AM_WarpingAlpha / AM_OrientationWarpingAlpha
                                          / AM_RateWarpingAlpha 발견

2) BlueprintTools.get_parent
   → /Script/AnimationModifiers.AnimationModifier   (진짜 애님 모디파이어가 맞다)

3) BlueprintTools.list_variables / list_events
4) ObjectTools.get_properties  (CDO: Default__AM_WarpingAlpha_C 등)
```

## 3. 결과

### 3.1 GASP는 커브 생성 모디파이어를 **이미 갖고 있다** [A]

세 개가 **같은 로직에 이름·블렌드 시간만 다른** 형제다.

| 모디파이어 | 생성 커브 | BlendIn/Out |
|---|---|---|
| **`AM_WarpingAlpha`** | **`Enable_Warping`** ← **우리가 필요한 것** | 0.25 / 0.25 |
| `AM_OrientationWarpingAlpha` | `Enable_OrientationWarping` | 0.25 / 0.25 |
| `AM_RateWarpingAlpha` | `Enable_PlayRateWarping` | 0.40 / 0.40 |

### 3.2 CDO 기본값 [A] — 실측

```
CurveName                 = Enable_Warping        (AM_WarpingAlpha)
SamplesPerSecond          = 30
BlendInTime               = 0.25
BlendOutTime              = 0.25
RotationAngleThreshold    = 5      (도)
TranslationAngleThreshold = 5      (도)
bInvert                   = false
bDeleteCurveOnRevert      = true
```

### 3.3 판정 축이 설계 추정과 다르다 [A]

변수 목록이 알고리즘을 드러낸다:

```
bGoingStraight / bLastGoingStraight
RootMotionTranslationAngle / LastRootMotionTranslationAngle
RotationError / TranslationError
RotationAngleThreshold / TranslationAngleThreshold
BlendInTime / BlendOutTime
```

즉 30Hz로 샘플링하며 프레임 간 **루트모션의 회전 오차**와 **이동 방향 각도 오차**를 **각각** 보고,
**둘 다 5° 미만일 때만** "직선(`bGoingStraight`)"으로 판정한다. 그리고 켜짐/꺼짐 경계를
0.25초 블렌드로 이어 **연속 알파**를 만든다.

> 설계 추정(`RotationSpeed` 단일 축 임계값)은 **절반만 맞았다.** 회전만 보면
> "제자리에서 옆으로 미끄러지는" 구간이 직선으로 오판된다. 이동 방향 축이 같이 필요하다.

### 3.4 ★ 파이프라인 순서가 강제된다 [A]

판정이 **루트모션**을 읽으므로, `AM_WarpingAlpha`는 **루트모션이 이미 있는 클립**에만 쓸 수 있다.

```
IK Retarget → UEncodeRootBoneModifier (루트모션 합성) → AM_WarpingAlpha
                                       ↑ 이게 먼저다. 순서를 바꾸면 커브가 전부 0 또는 전부 1
```

인플레이스 Mixamo 클립에 `AM_WarpingAlpha`를 먼저 걸면 **루트가 안 움직이므로 오차가 0 → 전 구간 1**이
나온다. **조용히 잘못된 커브가 생긴다** — 에러가 안 나므로 특히 위험하다.

## 4. 판정

**성공.** 자동 생성 수단이 존재하며, 자체 제작(C++ 모디파이어 신규 작성)은 **불필요**하다.
`RotationSpeed` 기반 자작 계획은 폐기한다.

부수적으로 `AnimationModifierLibrary` 플러그인이 **엔진 기본 활성(`EnabledByDefault=true`)** 임을
확인했다 — `UEncodeRootBoneModifier` / `UMotionExtractorModifier`도 지금 바로 쓸 수 있다.

## 5. 이것이 바꾸는 것

| 문서 | 절 | 어떻게 바뀌나 |
|---|---|---|
| `assets/2026-09-02_asset_supply_and_collaboration.md` | 4b | "루트 `RotationSpeed` 임계값(유력)" → **`AM_WarpingAlpha` 적용**. 단계 순서에 "EncodeRootBone 먼저" 명시 |
| `animation/2026-09-02_pose_pipeline_spec.md` | 11절 표 | `Enable_Warping` 생성 열 → `AM_WarpingAlpha` |
| `OPEN_ITEMS.md` | C-25 | 해결 |
| `CURRENT_STATE.md` | 4절 ① | 체크리스트 항목 교체 |
| `CLAUDE.md` | 5절 P8 | 커브 3종 규약에 **적용 순서**를 덧붙일 것 |

- [x] `OPEN_ITEMS.md`에서 C-25 해결 표시
- [x] `CURRENT_STATE.md` 갱신
- [ ] `assets/` · `animation/` 원 문서에 정정 절 추가 (다음 세션)

## 6. 막힌 것 / 다음에 확인할 것

| ID | 항목 | 왜 |
|---|---|---|
| **C-26** | Mixamo 리타깃 클립에서 임계값 **5°가 적절한가** | GASP 클립은 Epic이 정성껏 만든 것이다. 리타깃 클립은 노이즈가 더 크다 → 직선 구간에서도 5°를 넘어 커브가 깜빡일 수 있다. 넘치면 임계값을 올리거나 `SamplesPerSecond`를 낮춰 평활할 것 |
| **C-27** | `AM_WarpingAlpha`를 우리 클립에 걸었을 때 **커브가 전 구간 1이 아닌가** | 3.4절의 함정. **커브 그래프를 눈으로 확인하는 것이 P0-4의 필수 검수 항목** |

### 다음 세션이 바로 할 수 있는 것

MCP로 GASP 기존 클립의 `Enable_Warping` 커브 값을 떠서 **정답 모양을 먼저 본다.**
(예: 직선 walk 루프는 전 구간 1에 가깝고, pivot/turn 클립은 중앙이 0으로 파여 있어야 한다)
그 모양을 기준으로 우리 클립의 결과를 비교하면 C-26/C-27을 한 번에 판정할 수 있다.

### MCP 한계 (한 건 추가)

`BlueprintTools.read_graph_dsl`로 `AM_WarpingAlpha`의 `EventGraph`를 읽으면 **빈 문자열**이 온다.
`list_events`는 `OnApply` 구현됨으로 보고하는데도 그렇다. **애님 모디파이어 그래프 본문은 MCP로
못 읽는다** — 변수 목록과 CDO 값으로 역추론했다. 그래프 자체는 에디터에서 열어볼 것.
→ `CLAUDE.md` 6.1절에 추가함.
