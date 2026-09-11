# P0-4d · `contact_l/r` 생성 — 자작 모디파이어

2026-09-04 / **성공** / [C-33] 해결. 엔진 기본 모디파이어로는 불가능해서 C++ 에디터 모듈을 신설하고 직접 만들었다.

관련 항목: **[C-33] 해결** / 관련 문서: `2026-09-03_gasp_curve_manifest.md` 9절

---

## 1. 왜 자작해야 했나

`contact_l/r`은 **모든 GASP 클립에 예외 없이 있는 유일한 커브**이고, `FootPlacement` 노드가
`RemapCurves`를 통해 발 심기 타이밍을 여기서 얻는다. 없으면 발 IK가 조용히 오작동한다.

그런데 만들 수단이 없었다:

| 후보 | 왜 안 되나 |
|---|---|
| `UMotionExtractorModifier` | 연속값만 낸다. **사각파가 안 나오고**, 비교 연산자도 없다(None/Add/Sub/Div/Mul) |
| `UFootstepAnimEventsModifier` | 접지를 판정하긴 하나 **걸음당 노티파이 1개**만 낸다 → on/off 구간이 없다 |
| `UCurveFromSyncMarkersModifier` | 싱크마커 시점에 키를 찍는다. 그런데 `AM_FootSteps_Walk`은 `bShouldGenerateSyncMarkers=false` |
| BP로 작성 | `read_graph_dsl` 출력에 손실이 있어(구조체 분해가 `ToolMenus\|Get`으로 오역) DSL 왕복이 위험 |

→ **C++ 에디터 모듈 신설.**

## 2. 만든 것

```
Source/SoldierLabEditor/                          ← 신설 (Type: Editor)
├── SoldierLabEditor.Build.cs                     AnimationModifiers, AnimationBlueprintLibrary,
├── SoldierLabEditor.h / .cpp                       AnimationCore, UnrealEd
└── AnimModifiers/FootContactCurveModifier.h/.cpp

SoldierLab.uproject         Modules += { SoldierLabEditor, Editor }
SoldierLabEditor.Target.cs  ExtraModuleNames += "SoldierLabEditor"
```

> 엔진의 `FPoseExtractorUtilityLibrary`(`EncodeRootBoneModifier`가 쓰는 것)는 플러그인 `Private/`에
> 있어 외부에서 못 쓴다. 공개 API인 **`UAnimPoseExtensions::GetAnimPoseAtTime` +
> `GetBonePose(..., EAnimPoseSpaces::World)`** 를 썼다.

### 판정 로직

```
planted(i) = (Z(i) <= EffectiveGroundThreshold) AND (speed(i) <= SpeedThreshold)
```

- 속도는 **중앙차분**(앞뒤 샘플) — 노이즈 한 프레임이 상태를 못 뒤집는다
- 키는 **상태가 바뀔 때만**, 전환 직전에 이전 값을 한 번 더 찍어(`TransitionEpsilon` 0.001s)
  보간이 모서리를 깎지 못하게 한다. `AM_WarpingAlpha`와 같은 방식
- 짧은 구간 필터를 **양쪽 상태에 대칭으로** 건다(3.3절)

## 3. 세 번 틀렸고, 그때마다 계측으로 잡았다

### 3.1 루트모션을 빼면 안 된다 [A]

처음에 `bIncorporateRootMotionIntoPose = false`로 뒀다. **정확히 거꾸로였다.**

이 플래그는 내부적으로 `FAnimExtractContext::bIgnoreRootLock`에 매핑된다(`AnimPose.cpp:669`).
`false`면 루트가 잠겨 **캐릭터가 제자리걸음**이 되고, 그러면 디딘 발이 몸 아래에서
**보행 속도(~100cm/s)로 뒤로 미끄러진다** → 접지가 하나도 안 잡힌다.

루트모션이 **포함**돼야 디딘 발이 월드에서 진짜 정지한다.

**검증 지표**: 로그의 `track net travel`. 122.7cm가 나오면 포함된 것, 0에 가까우면 잠긴 것.

### 3.2 ★ 고정 높이 임계값은 못 쓴다 — 자동 보정 [A]

`GroundThreshold = 4.0`(GASP `AM_FootSteps_Walk`과 같은 값)으로 시작했더니:

```
ball_l  Z min/avg/max = 2.79 / 5.28 / 11.48   below-Z 26%   BOTH 11%
ball_r  Z min/avg/max = 2.42 / 6.13 / 15.97   below-Z 54%   BOTH 45%
```

**리타깃이 왼발을 오른발보다 띄워놨다.** 고정 임계 하나로는 한 발은 통과하고 한 발은 탈락한다.
GASP 클립은 저작 데이터라 발이 정확히 Z=0에 닿지만, **리타깃 클립은 발마다 자기 바닥 높이가 다르다.**

→ `bAutoCalibrateGroundHeight`(기본 켜짐): **발마다 자기 최저 Z + `GroundHeightMargin`(3.0cm)** 를
실효 임계로 쓴다.

```
ball_l  thr 4.00 → 5.79   below-Z 26% → 68%   BOTH 11% → 38%
ball_r  thr 4.00 → 5.42   below-Z 54% → 61%   BOTH 45% → 45%
```

**수백 개 클립을 돌릴 파이프라인에서는 이게 필수다.** 클립마다 손으로 임계를 맞출 수 없다.

### 3.3 짧은 구간 필터는 양쪽에 걸어야 한다 [A]

`MinContactDuration`만 있어서 짧은 *접지*는 지웠지만 짧은 *공중* 구간은 안 메웠다.
접지 도중 속도가 순간 튀면 **하나의 접지가 둘로 갈라져** 헛디딤처럼 보인다.

→ `MinReleaseDuration`(0.08s) 추가. **틈을 먼저 메우고, 그 다음 짧은 접지를 지운다.**
순서를 바꾸면 갈라진 조각이 각각 지워진다.

**루프 이음매**: 클립 양 끝에 걸친 구간은 실제 접지가 클립 밖으로 이어지는 것이라 길이로
판단하면 안 된다. 양 끝 구간은 필터에서 제외했다.

## 4. 최종 설정값

| 프로퍼티 | 기본값 | 근거 |
|---|---|---|
| `Feet` | `ball_l`→`contact_l`, `ball_r`→`contact_r` | GASP `AM_FootSteps_Walk`이 ankle이 아니라 **ball**을 쓴다 |
| `bAutoCalibrateGroundHeight` | `true` | 3.2절 |
| `GroundHeightMargin` | 3.0 cm | 실측(최저 2.4~2.8, 평균 5.3~6.1) |
| `SpeedThreshold` | 10.0 cm/s | 스윙 최고 369cm/s, 스탠스 0.0 — 분리가 뚜렷해 민감하지 않다 |
| `SampleRate` | 60 | GASP 동일 |
| `MinContactDuration` | 0.05 s | |
| `MinReleaseDuration` | 0.08 s | 3.3절 |
| `TransitionEpsilon` | 0.001 s | |

## 5. 결과

```
Walking_Anim          : enable_warping ; contact_l ; contact_r
M_Relaxed_Walk_Loop_F : contact_l ; contact_r ; movedata_speed ; enable_warping ; phase  (+ L/R 마커)
```

커브 모양 육안 확인 완료 — 좌우 교대 사각파, 갈라짐 없음.

## 6. 남은 것

| 항목 | 수단 |
|---|---|
| `MoveData_Speed` | `AM_MoveData_Speed` (GASP 것 그대로) |
| `phase` | `AM_FootSteps_Walk` → `AM_BakePhaseCurveFromFootstepNotifies` |
| `L`/`R` 싱크마커 | `AM_FootSteps_Walk`의 `bShouldGenerateSyncMarkers`를 켜야 한다 (GASP 기본은 꺼짐) |

## 7. 배운 것 — 방법론

**임계값을 추측으로 세 번 조정하다가, 계측을 넣고 한 번에 잡았다.**
`UE_LOG`로 Z·속도의 min/avg/max와 각 조건 통과 비율을 찍자 원인(높이 비대칭)이 즉시 드러났다.
로그는 MCP `EditorToolset.LogsToolset.GetLogEntries`로 읽을 수 있어 왕복이 빠르다.

**같은 종류의 문제(임계값·판정 기준)를 만나면 조정 전에 계측부터 넣을 것.**
