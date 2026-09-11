# `UTurnInPlaceCurvesModifier` — 회전 클립이 필요로 하는 것 전부를 한 번에

2026-09-08 / **성공 (빌드·등록 확인)** / 손 저작으로 계획했던 스티어링 커브 2종 + PoseSearch 노티파이 2종을 모디파이어 하나로 묶었다. 만들면서 **"게이트 0.5초 고정"이 틀렸다는 것**이 드러났다.

관련: [C-45] 해결 · [C-48] 해결 · 신규 [C-45b] / 관련 문서: `2026-09-04_c34_clip_curve_mapping.md` 5절

---

## 1. 왜 모디파이어로 갔나

[C-45]는 "커브가 키 1~2개니 손으로 찍으면 된다"로 닫았었다. 실제로 해보니 두 가지가 걸렸다:

1. **에디터 조작이 자명하지 않다** — 커브 추가 / 키 보이기 / 커브 에디터 진입이 각각 다른 메뉴다
2. **게이트 시각이 클립마다 다르다**(5절) — 상수를 물려주면 출처가 바뀌는 순간 틀린다.
   **클립에서 재는 편이 옳고, 재려면 코드가 필요하다**

`Source/SoldierLabEditor/`는 이미 있으므로 추가 비용은 파일 두 개다.

## 2. 무엇을 하는가

`Source/SoldierLabEditor/AnimModifiers/TurnInPlaceCurvesModifier.{h,cpp}`

| 스위치 | 산출물 |
|---|---|
| `bGenerateSteeringTargetTime` | `steeringtargettime` **상수 1.0** |
| `bGenerateSteeringGate` | `enable_turninplacesteering` **1 → 0 계단** |
| `bGeneratePoseSearchNotifies` | `PoseSearch` 트랙에 노티파이 스테이트 2종 |

노티파이는 앞구간 `Override Continuing Pose Cost Bias`(기본 −1.0), 뒷구간 `Block Transition In`.
경계는 `NotifySplitTime`이 음수면 **게이트 시각을 따라간다**(기본).

**기본값 그대로 Add → Apply.** 설정할 것이 없다.

### 2.1 설계 판단 세 가지

**PoseSearch 플러그인에 링크하지 않는다.** 노티파이 클래스들이 `MinimalAPI`라 멤버 접근이
안 되고, 클래스 포인터 두 개 때문에 모듈 의존성을 늘릴 이유가 없다. `TSoftClassPtr`에
경로를 박아두고 `LoadSynchronous()`로 푼다:

```cpp
BlockTransitionNotifyClass = TSoftClassPtr<UAnimNotifyState>(FSoftObjectPath(
    TEXT("/Script/PoseSearch.AnimNotifyState_PoseSearchBlockTransition")));
```

`CostAddend`는 리플렉션으로 넣는다 — 역시 헤더가 필요 없다:

```cpp
if (FFloatProperty* Property = FindFProperty<FFloatProperty>(State->GetClass(), TEXT("CostAddend")))
    Property->SetPropertyValue_InContainer(State, ContinuingPoseCostBias);
```

**재적용이 노티파이를 쌓지 않게 한다.** 전용 트랙(`PoseSearch`)을 쓰고, 이미 있으면
`RemoveAnimationNotifyEventsByTrack`으로 비운 뒤 다시 찍는다. Revert는 **우리가 만든 트랙일 때만**
트랙째 지운다(`GeneratedNotifyTracks`에 기록).

**루트모션이 포즈에 들어가야 한다.** `bIncorporateRootMotionIntoPose = true`(= `bIgnoreRootLock`).
빠뜨리면 루트가 고정돼 **yaw 프로파일이 평평하게 나온다** — `FootContactCurveModifier`에서
겪은 것과 같은 함정이다(`../CLAUDE.md` P9 계열).

## 3. 검증

```
UnrealEditor-SoldierLabEditor.dll                       2026-09-08 15:07
/Script/SoldierLabEditor.TurnInPlaceCurvesModifier      등록됨
blockTransitionNotifyClass        → /Script/PoseSearch.AnimNotifyState_PoseSearchBlockTransition          실재
continuingPoseCostBiasNotifyClass → /Script/PoseSearch.AnimNotifyState_PoseSearchOverrideContinuingPoseCostBias  실재
```

소프트 경로 두 개가 실제 클래스로 해석되는 것까지 MCP `search_subclasses`로 확인했다.

> **UHT는 통과했는데 컴파일이 안 될 때**: `Unable to build while Live Coding is active`.
> **새 UCLASS는 Live Coding으로 안 된다**(새 리플렉션 타입 등록 불가) — 에디터를 닫고 빌드해야 한다.
> UHT가 먼저 돌므로 **에디터를 안 닫아도 리플렉션 문법 오류는 잡힌다**(`3 generated files written`).

## 4. ★ 계측이 뒤집은 것 — 게이트는 0.5초 고정이 아니다

`bLogYawProfile`(기본 켬)이 루트 yaw 프로파일을 찍는다. 우리 클립 실측 [A]:

```
[Turning_Right_90_Degrees_Anim] length 2.400s (146 samples)
root yaw net 88.8 deg, total travelled 94.6 deg
10%@0.417  25%@0.583  50%@0.900  75%@1.167  90%@1.367  95%@1.450  100%@2.400
→ enable_turninplacesteering closes at 1.367s (auto, from yaw profile)
```

**0.417초는 회전이 10% 진행된 시점이다.** 계획대로 0.5초에 게이트를 닫았으면
**돌기 시작하기도 전에 스티어링이 꺼졌을 것이다.**

GASP 두 클립(090·180)이 둘 다 0.5초 근처였던 건 **그 클립들이 회전을 앞부분에서 끝내기
때문**이었다. 표본이 같은 출처뿐이라 "절대 시간"과 "진행률"이 구분되지 않았다.

### 4.1 진짜 규칙 — 마지막 발이 착지할 때 [B]

| 클립 | 게이트 OFF | 마지막 발 착지 |
|---|---|---|
| GASP `M_Relaxed_Stand_Turn_090_L` (60f/2.0s) | 프레임 17 | 프레임 ~19 |
| 우리 `Turning_Right_90_Degrees_Anim` (72f/2.4s) | **프레임 41** | **프레임 41** |

우리 클립에서 **yaw 90% 지점과 발 착지 시점이 독립적으로 같은 답을 냈다.**
그래서 `bAutoSteeringOffTime`(기본 켬)이 `SteeringOffYawFraction`(기본 0.9)로 클립에서 직접 뽑는다.

→ **[C-45b]**: GASP 클립의 yaw 프로파일은 아직 안 떴다. 0.567초가 그 클립의 yaw 90% 지점이면
   규칙이 [A]가 된다. 클립을 우리 폴더로 복제해 같은 모디파이어를 걸면 1분이다(원본 무손상).

### 4.2 부수 확인 — 회전 클립의 루트모션이 맞다 [A]

`root yaw net 88.8°`. 리타깃 + `AM_EncodeRootBone`(pelvis 축 Y)이 **회전 클립에서도 90°를
제대로 살렸다.** [C-30]의 "회전 클립으로 A안 재검증은 아직"에 대한 첫 실측이다.

## 5. 곁가지 — `AM_FootSteps`를 이 클립에서 뺀 이유

적용해 보니 **양발 노티파이가 같은 프레임(~40)에 찍혔다.** 원인은 엔진 소스에 있다
(`FootstepAnimEventsModifier.cpp:311`, `:183`):

```cpp
case FootBoneSpeed:
  return (Prev < thr && Curr >= thr)                        // (A) 발이 떨어지는 순간
      || (bIsLast && Curr < thr && MinBelowThr != MAX_FLT);  // (B) 마지막 샘플
...
bSkipNextNotify = FootBoneSpeed < thr && bShouldSkipNotifyIfFootBoneSpeedStartsBelowThreshold;
```

- 이벤트는 **발이 떨어질 때** 발화하고, 위치는 직전 접지 구간의 **최저속 시각**에 찍힌다
- **시작 시 이미 접지 중인 발은 첫 이벤트를 버린다.** 제자리 회전은 **양발 다** 접지로 시작한다
- 그 뒤 이 클립엔 두 번째 이탈이 없다 → **양발 모두 (B) 마지막-샘플 분기**로 떨어져
  "마지막 접지 구간의 최저속 시각"에 찍히고, 회전이 끝난 뒤 둘 다 정지해 있으니 **한 점으로 수렴**한다

근본 원인은 클립이다 — `FootContactCurveModifier` 로그 실측:

```
ball_l  Z 3.49~6.40  speed max 216.7 cm/s   below-Z 100%
ball_r  Z 3.52~4.27  speed max 117.0 cm/s   below-Z 100%
```

**발이 아예 안 뜬다.** Mixamo의 이 클립은 발을 들지 않고 **비비면서 도는** 동작이다 → **[C-38]** 사례 추가.

**그래서 뺐다.** 노티파이의 용처는 두 개뿐인데 —
`AM_BakePhaseCurve`의 입력(제자리회전엔 `phase`를 안 만든다, 매핑표 ⑥)과 **발소리**다.
**모션 매칭에도 발 IK에도 영향이 0**이고, 회전 끝에 발소리가 겹쳐 나는 것뿐이다.

> 발소리가 필요해지면 `contact_l`/`contact_r`이 1로 복귀하는 프레임에 손으로 두 개 찍으면 된다.
> **커브는 이미 정답을 갖고 있다** — 오른발 21, 왼발 41에 순서대로 착지한다.

## 6. 매핑표에 미치는 영향

`2026-09-04_c34_clip_curve_mapping.md` 4절 표의 ⑥제자리 회전 행이 바뀐다:

| | 이전 | 지금 |
|---|---|---|
| `AM_FootSteps_*` | ✅ 마커 OFF | **발소리가 필요할 때만.** 발을 안 드는 클립에서는 오작동한다 |
| 8·9행(손 저작) | 손으로 | **`UTurnInPlaceCurvesModifier` 하나로 대체** |
