# AI 병사가 걷지 않던 이유 — 원인 6개를 순서대로 벗겨낸 기록

2026-09-09 / **해결** / "다리가 질질 끌린다"의 원인은 하나가 아니라 **여섯 개가 겹쳐 있었다.** 전부 우리 쪽 에셋·설정 결함이고, **GASP 시스템은 한 줄도 바꾸지 않았다.**

관련: [C-31] 재번복 · [C-46] · [C-49] · 신규 [C-50] / 관련 문서: `2026-09-08_root_facing_56deg.md`(공식 오류 정정 포함) · `2026-09-06_p0-2_wiring_plan.md`

---

## 1. 증상

```
AI 병사가 걷기 애니메이션을 재생하지 않고 미끄러짐
"상체가 와이어에 매달려 수평이동하면서 다리도 바닥에서 미끄러진다"
정지 중 0.4~0.5초 주기로 버벅거림
```

## 2. 결정적이었던 방법 — 대조군 두 개

추측을 끊은 것은 **"정답을 아는 것과 통째로 비교"** 였다. 두 번 다 여기서 답이 나왔다.

| 대조군 | 무엇을 증명했나 |
|---|---|
| **런 A**: 우리 병사 + GASP 데이터 | GASP NPC와 동일하게 걸었다 → **궤적생성·MM·ABP·캐릭터 전부 결백** |
| **PSD 내용을 GASP 클립 하나로 교체** | 정상 → **우리 클립이 범인**임을 확정 |

그리고 구조 diff:

```
AnimGraph 노드 22개 설정      GASP와 차이 0
포즈 흐름 21엣지              GASP와 차이 0
캐릭터 CDO 134개 프로퍼티     walkSpeeds 하나만
애님시퀀스 프로퍼티 전수       6개만 차이 (그중 2개가 원인)
```

> **MCP로 AnimGraph를 읽을 수 있다는 것**이 이 전부를 가능하게 했다 —
> `find_nodes` + `get_node_infos` + `get_properties`. `../CLAUDE.md` 6.1절 정정 참고.

---

## 3. 원인 6개

### (1) `ik_foot_*`가 비어 Orientation Warping이 무효였다 — [C-46]

워핑 노드 실측: `iKFootRootBone = ik_foot_root`, `iKFootBones = [ik_foot_l, ik_foot_r]`.
**워핑은 IK 발 본을 회전시켜 동작한다.** 리타깃 클립에 그 가상본이 없으면 회전시킬 대상이 없다.
→ `AM_Copy_IKFootRoot`를 파이프라인에 추가.

### (2) Stride Warping 노드가 없었다

설계 5.5.3 [2]층이 요구하는데 **GASP에는 없다**(블렌드스택 노드 전수 조회로 확인).
GASP는 속도별 클립이 227개라 필요가 없었던 것이다. **우리는 클립이 적으므로 필수.**
→ `AnimationBlendStackGraph_0`의 `OrientationWarping_1` 다음에 삽입.

### (3) ★ `bForceRootLock = false` — 포즈에 루트 이동이 남아 있었다

가장 큰 원인. `AnimSequence.cpp:1862`:

```cpp
if ((ExtractionContext.bExtractRootMotion && RootMotionReset.bEnableRootMotion)
    || RootMotionReset.bForceRootLock)
{
    RootMotionReset.ResetRootBoneForRootMotion(OutPose[0], RequiredBones);
}
```

루트가 잠기는 조건이 **OR 두 개**다. **GASP는 루트모션으로 캐릭터를 움직이지 않으므로**
(CMC가 이동, `OffsetRootBone`이 메시 정합) `bExtractRootMotion = false`이고,
**첫 조건이 성립하지 않는다. GASP는 `bForceRootLock = true`로 잠근다.**

우리는 `false`였다 → 포즈 안에 루트 이동이 그대로 남아 → 메시가 포즈 안에서 전진하다가
**루프 지점에서 원점으로 순간이동.** 이것이 "와이어에 매달려 수평이동"의 정체다.

> 나는 이 항목을 diff에서 보고도 *"`bEnableRootMotion`이 true니 어차피 잠긴다"* 며
> **소스를 안 읽고 넘겼다.** 실제 조건은 OR였다.

### (4) 루트 facing 오차 — 34° (56°가 아니었다)

`AM_EncodeRootBone`이 pelvis yaw를 루트 yaw로 복사하므로 **견착 블레이드가 루트에 구워진다.**
→ `USoldierRootFacingModifier` 신설로 교정. 상세는 `2026-09-08_root_facing_56deg.md`.

### (5) ★ 진단 공식이 90° 틀려 있었다

(4)를 재면서 **−56°**가 나왔고 그걸 블레이드로 단정했는데, **GASP 클립으로 대조하니 −90.0° / sd 0.0** 이 나왔다.
`sd 0.0`은 데이터가 아니라 **공식 오류**의 서명이다.

```cpp
YawFromHeading(v)   = Atan2(-v.X, v.Y)   ← 엔진 convention
YawFromDirection(d) = Atan2( d.Y, d.X)   ← 내가 진행방향에 쓴 것.  정확히 90° 차이
```

보정하면 GASP **0°**, 우리 **+34°**. **실제 오차는 34°였고, 내 교정은 그것을 90°로 키웠다**
(그래서 캐릭터가 옆으로 걸었다). 공식을 `YawFromHeading`으로 통일해 해결.

> **정답이 0이어야 할 대조군이 없었으면 영영 못 찾았다.** 사용자가
> *"얘는 root·pelvis·진행방향이 다 앞인데 의미가 있나"* 라고 물었는데, **바로 그래서 의미가 있었다.**

### (6) ★★ 레벨 인스턴스의 프로퍼티 오버라이드가 CDO 변경을 가렸다 — 신규 [C-50]

`WalkSpeeds`를 200 → 100 → 90 → 80으로 바꿔가며 시험했는데 **전부 반영되지 않았다.**
레벨에 배치된 `BP_SoldierCharacter` 인스턴스가 **자기 값 200을 갖고 있었기 때문**이다.

이것이 그동안의 관측을 오염시켰다:

```
실제 캐릭터 속도 200,  클립 속도 90  →  StrideScale = 2.2  →  클램프 1.8
→ 관측된 "보폭이 프리뷰보다 2배"가 정확히 이것
```

> **MCP로 CDO를 바꿔도 배치된 액터가 그 프로퍼티를 오버라이드하고 있으면 안 먹는다.**
> 그리고 오버라이드 여부는 **레벨을 열어야만** 보인다 — 에셋만 봐서는 알 수 없다.
> → `../CLAUDE.md` 6.1절에 추가.

---

## 4. 최종 설정

```
클립 (Walking_Anim)
  bForceRootLock            true        ★ GASP와 동일
  bLoop                     true
  curveCompressionSettings  SandboxAnimCurveCompressionSettings   (GASP와 동일)
  루트 facing               진행 방향 정렬 (SoldierRootFacingModifier)

ABP (SoldierCharacter_ABP)
  StrideWarping 노드         OrientationWarping_1 다음
    Mode=Graph · Pelvis=pelvis · IKFootRoot=ik_foot_root
    FootDefinitions = [ik_foot_l/foot_l/thigh_l, ik_foot_r/foot_r/thigh_r]
    StrideScaleModifier = clamp [0.8, 1.25] + Interp     ← 잔차 보정용으로만
    LocomotionSpeed ← Speed2D

캐릭터 (레벨 인스턴스!)
  WalkSpeeds  (80, 72, 60)   ★ 클래스 기본값이 아니라 인스턴스에 넣어야 반영된다
```

### 4.1 속도 정합의 원칙

```
주 정합 : 캐릭터 속도 = 클립 속도
보조    : Stride Warping ±25% + MM playRate ±15%  →  합계 대략 ±30~40% 흡수
```

**Stride Warping 클램프를 좁게(±25%) 잡은 것이 안전장치다.** 워핑이 크게 개입해야 하는 상황이
생기면 그것은 **데이터가 부족하다는 신호**로 드러나야 하고, 조용히 늘려서 가리면 안 된다.

> 클립이 늘어나면 방향이 반대가 된다 — 캐릭터 속도는 게임 디자인이 정하고,
> 그 속도대를 커버하는 클립을 갖춘다. 지금 캐릭터를 클립에 맞추는 것은 **클립이 1개라 생긴 임시방편**이다.

### 4.2 리타깃이 속도를 바꾼다 [A]

```
원본 FBX     136.6cm / 1.367s = 99.9 cm/s
리타깃 후    122.7cm / 1.367s = 89.8 cm/s     ← 10% 감소
```

Mixamo ↔ UEFN 마네킹 체격 차이 때문이다. **속도 정합은 반드시 리타깃 후 값으로 해야 한다.**

---

## 5. 우회책이었던 것 — 되돌릴 대상

원인 (3)을 모르고 증상을 가리려 넣은 것들이다. 근본 원인이 해결됐으니 **GASP 기본값으로 되돌려야 한다.**

| 항목 | 우회 중 값 | 원복 결과 |
|---|---|---|
| MM `blendTime` | 0.15 | ✅ **0.5** |
| MM `poseReselectHistory` | 0.1 | ✅ **0.3** |
| MM `maxActiveBlends` | 2 | ✅ **4** |
| PSD 항목 `Disable Reselection` | 체크 | ✅ **해제** |

**전부 원복했고 정상 동작한다. 우회책은 하나도 필요 없었다** → [C-49] 해결.

> ★ **내가 세웠던 "클립이 1.367초라 Epic의 4초 전제 상수와 안 맞는다"는 가설은 틀렸다.**
> 그럴듯한 계산(0.3s가 클립의 22%, 0.5s가 37%)까지 붙였지만, 진짜 원인은 `bForceRootLock`이었고
> 그것을 고치자 GASP 기본값 그대로 잘 돈다.
> **증상에 맞는 그럴듯한 메커니즘을 찾았다고 원인인 것은 아니다** — 우회책이 증상을 줄이면
> 가설이 맞은 것처럼 보이지만, 실제로는 원인을 가린 것일 수 있다.

### 5.1 남아 있는 구조적 단순화 — [C-51]

`Disable Reselection`을 끄고 idle 클립을 다시 넣으면 **멈췄다 다시 걸을 때 idle에 눌러앉아
다리가 끌린다.** 이것은 데이터 부족이 아니라 **우리가 GASP의 3단 챙터를 1단으로 접은 결과**다.

```
GASP:  중첩 챙터가 Speed 2D 로 DB 를 가른다
       (0, 20)  → PSD_Dense_Stand_Idles
       (20, ∞)  → PSD_Dense_Stand_Walk_Stops   ...
우리:  idle + walk 를 한 DB 에 넣고 선택을 비용 경쟁에 맡김
       → continuingPoseCostBias(-0.01)가 "머무르기"를 밀어 idle 에 고정
```

**클립이 들어오는 시점에 GASP 구조를 복제해야 한다**(설계 15.3절 (가)안). 지금 지으면 다시 짓게 된다.

---

## 6. 방법론 — 이번에 값이 컸던 것과 비쌌던 것

### 효과가 컸던 것

1. **정답을 아는 대조군을 세운다.** 런 A와 GASP 클립 교체가 매번 범위를 절반으로 잘랐다.
2. **통째로 diff한다.** 항목 몇 개를 눈으로 대조하지 말고 전 프로퍼티를 기계적으로 비교한다.
   `bForceRootLock`도 `curveCompressionSettings`도 diff가 뱉어준 것이다.
3. **`sd = 0`은 데이터가 아니라 공식을 의심하라는 신호다.**

### 비쌌던 것 — 반복된 실패 패턴

**소스를 읽지 않고 "어차피 그럴 것"으로 넘긴 것이 세 번 틀렸다.**

| # | 넘긴 판단 | 실제 |
|---|---|---|
| 1 | "`bEnableRootMotion`이 true니 `bForceRootLock`은 무의미" | OR 조건이라 **결정적이었다** |
| 2 | "`linkId = -1`이니 연결 안 된 노드" | 에디터 노드의 LinkID는 컴파일 산출물이라 **항상 −1** |
| 3 | "커브 이름이 같으니 클립은 동일" | 압축 설정이 달랐다 (태그에 안 나옴) |

그리고 **한 번에 두 개씩 바꿔서** 결과를 해석 못 한 구간이 있었다
(`WalkSpeeds` + `BlendSpacePlayer_1` 동시 변경).

`../CLAUDE.md` 3.1절의 패턴이 형태를 바꿔 계속 나온다 — **확인하지 않은 것을 확정처럼 다루는 것.**

---

## 7. 남은 것

| ID | 항목 |
|---|---|
| **C-49** | 우회 파라미터 4개를 GASP 기본값으로 되돌릴 수 있는가 (5절) |
| **C-50** | 레벨 인스턴스 오버라이드 점검 — 다른 프로퍼티에도 같은 함정이 있는가 |
| C-44 | 방향 클립 확보 후 급선회 판정 (P0-2 본 측정) |
