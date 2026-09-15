# 머리 조준 추종 · 1인칭 눈–조준선 정렬 (Head Aim / Sight Alignment)

2026-09-14 (11차 2026-09-15) / **완료 — 10차 "성공. 이제 모든게 완벽해"(21:30) · 11차 몸 회전·관전 위임·neck_02 "잘됨"(09-15)** / 총을 눈으로 가져오는 게 아니라 **머리(눈)를 총의 조준선에** 놓는다 — `Pose/SoldierHeadAimComponent` 하나가 시선 추종과 눈 정렬을 닫힌 루프로 한다. 총·왼손 IK는 건드리지 않는다. **기본 OFF, H 키.**

관련: `ai/2026-09-14_cover_frame_fix_and_observer.md` 3c·3d절, `IMPLEMENTED.md` 2.5c(총구 되먹임) · 5.1절, `CLAUDE.md` P104~P113 · P119~P120.

> **읽는 법**: **0' 절이 최종 코드다.** 0~0.10 절은 같은 날 열 번 고친 시간순 기록이고, 0' 과 어긋나는 값은 0' 이 맞다. 열 번 중 절반은 **10차의 버그 하나**(몸 피치를 델타 재조립에서 빠뜨림)가 만든 스프링을 다른 장치로 가리려던 것이었다 — 그 장치들(가중치·래치·토르소 프레임)은 그 자체로 필요해서 남았지만, 다음에 비슷한 증상이 오면 **먼저 재조립 수식을 의심할 것**(P109).

> ⚠ 이 문서의 첫 판(같은 날 17:20)은 "총을 눈으로 IK" 였다. 사용자 정정: **머리가 총에 간다.** 총은 이미 마우스를 지연 추종하므로 머리가 총을 따르면 "급선회 시 잠깐 틀어졌다 정렬"이 저절로 나오고, 왼손 IK 도 그대로 꺼 둘 수 있다. 첫 판은 폐기.

---

## 0'. 최종 설계 (2026-09-14 21:30, 10차 = 사용자 확인 완료) — 0~0.10 절의 시간순 정정을 합친 것 [A]

`USoldierHeadAimComponent` (`Source/SoldierLab/Pose/`). 매 틱, 메시 틱보다 먼저(선행 조건 등록). 값은 전부 `EditAnywhere`(`SoldierLab|HeadAim|…`) — 배치된 병사 인스턴스가 오버라이드하면 인스턴스가 이긴다.

```
몸 프레임     BodyQuat = TorsoBone(spine_03) 실제 회전 × TorsoRel⁻¹      ← 캡슐도 메시 컴포넌트도 아니다 (P111)
              Body = BodyQuat.Rotator()  (피치·롤 있음)
              누적 보정(Correction · NeckBend · NeckStretch)은 프레임 사이에 이 프레임에 저장,
              틱 시작에 M·q·M⁻¹ 로 월드 변환, 끝에 되돌림 (P110)
조준 Aim      = Controller->GetControlRotation()          ← GetBaseAimRotation 은 뷰 회전 (P104)
                AI 는 접촉 중이면 Engagement->GetAimPoint() 방향
차단          IsBlindFiring(): ABP BF_AlphaL/R/U 중 |값| > BlindFireThreshold 0.05 → 전체 off
조준선 학습   sight 소켓 정지(≤ 30°/s ∧ ≤ 15 cm/s 가 0.1 s) ∧ 조준경 +X 가 Aim 의 AimAlignmentDegrees 8 안
              → 스냅샷. 학습값 대비 TrackSmallMovesCm 3 · Deg 3 이내면 LineTrackRate 12/s 로 연속 추적,
                그 이상은 정지까지 홀드. 저장 = 토르소 프레임의 점+방향 + Aim 프레임의 방향 오프셋(SettledSightDirInAim)
weld 가중치   w_target = smoothstep(각도: AlignZeroDegrees 35 → 0, AlignFullDegrees 3 → 1)
              w 가 1 에 닿으면 래치 — 견착 유지 중 벗어남을 UnweldDelaySeconds 0.5 까지 무시
              허용 조건: bEnabled ∧ (플레이어 ∨ bApplyToAI) ∧ !블라인드파이어 ∧ AOActive ∧ 학습된 선 있음
              WeldBlend = w_target 으로 WeldBlendRate 12/s 램프
2단           둘러보기 : 켜짐이면 항상. 목표 = 몸 yaw → Aim 을 LookAroundStrength(1.0) 만큼
              정렬(weld) : WeldBlend > 0. 목표 방향 = Aim → Aim×오프셋 을 w 로 블렌드
알파          Strength(1) 로 램프(AlphaRampPerSecond 4). 꺼지면 보정을 풀어 둔다(안티 와인드업)
클램프        Delta = Aim − Body, yaw ±75 / pitch ±55
              AimQuat = FRotator(Body.Pitch + Delta.Pitch, Body.Yaw + Delta.Yaw, 0)   ← ★ 두 축 다 몸에 더한다 (P109)
방향 루프     지난 프레임 머리 실제 시선(head 소켓 × HeadRel⁻¹) → AimQuat 전방의 swing 오차
              (FindBetweenNormals) × CorrectionGain 8/s 를 Correction 에 누적,
              ToSwingTwist 로 시선축 twist 제거 (P106), 총량 상한(yaw+pitch)
눈 위치       목표 = 학습된 선 위, 현재 눈에서 최근접점 (EyeReliefMinCm 5 ~ MaxCm 20 뒤)
              실제 목표 = 자연 눈(지난 프레임 측정 − 현재 굽힘·스트레치) + w × (선 − 자연 눈)   ← 출력에 w 를 곱하면 감아올림
목 굽힘       오차의 레버(neck_01→눈) 직교 성분을 각도로(|Δ⊥|/|L| × Fraction, ≤ 180°/s) NeckBend 에 누적,
              레버축 twist 제거, MaxNeckBendDegrees 60 상한. neck_01 : neck_02 = NeckBendSplit 0.5
스트레치      목이 못 하는 성분(레버 방향 성분, 굽힘 상한이면 전체 잔여)을 머리 본 월드 이동으로,
              게인 절반, MaxNeckStretchCm 5 · 30 cm/s   ← 기하 한계: 방향을 유지하면 유효 레버 = 목 길이 (P112)
ABP 쓰기      HeadAimRotation = Correction
              NeckAimRotation = Slerp(I, NeckBend, 1−Split) × Slerp(I, Correction, NeckShare 0.35)
              Neck2AimRotation = Slerp(I, NeckBend, Split) · HeadAimLocation = NeckStretch · HeadAimAlpha
ABP 적용      TwoBoneIK_0 → ModifyBone_6(neck_01) → ModifyBone_9(neck_02) → ModifyBone_7(head, 회전+이동)
              → ModifyBone_8(head, 전부 Ignore — 사용자 잔여, 통과 · 삭제 권고 [W54]) → ComponentToLocalSpace_2
              회전·이동 모두 Add to Existing · World Space (노드 프로퍼티는 사용자 수동, P108)
              ⚠ neck_02(ModifyBone_9)는 2026-09-15 새벽까지 Bone None·Ignore 로 남아 있었다(0.11절) — 지금은 채워짐
몸 회전       (2026-09-15, 0.11절) H 켜짐 ∧ 비조준 ∧ 블라인드파이어 아님 ∧ 정지(지면·가속 0·속도 ≤ BodyTurnMaxSpeedCms 10)
              ∧ 플레이어 → 카메라 yaw 가 캡슐 yaw 에서 BodyTurnStartDegrees 60 넘으면 캡슐 yaw 만
              BodyTurnRateDegPerSec 360 으로 돌리고 BodyTurnStopDegrees 10 안이면 멈춤(히스테리시스, 0 = 스냅).
              메시 붙잡기·발 옮기기(turn-in-place)는 GASP OffsetRootBone + MM 이 조준 모드와 같은 경로로 (P119)
```

1인칭 카메라(`Camera/SoldierFirstPersonComponent`, `FollowSocket`)와의 관계: 카메라 = `eyes` 소켓 위치 + **머리의 실제 시선**(`GetHeadLook`, 진입 시 캘리브레이션). 정렬이 정착하면 카메라 위치는 `sight` +X 선 위, 방향은 조준선과 일치. 롤은 정렬 대상이 아니라 린·애니메이션 롤이 남는다. 머리 추종 알파만큼만 소켓을 따르고 나머지는 컨트롤 회전(`bFollowSocketOnlyWhileHeadAims`). 1인칭 전용 니어플레인 `NearClipPlaneCm 2`(`FMinimalViewInfo::PerspectiveNearClipPlane`) — 전역값(10)으로는 총이 잘린다.

디버그 `SoldierLab.Debug.HeadAim 1`: 초록 목표 방향 · 빨강 실제 머리 · 흰 조준 · 시안 목표 눈 · 마젠타 실제 눈 · 글자 `a / on / ai / onaim / weld=x.xx(L=래치) / sight(STILL|moving/held|none) / bend=x/max / stretch=x/max / eye-res(cm) / look-res(°)`. 정착 시 `eye-res` ≈ 0 · `look-res` ≈ 0 이면 정렬된 것.

**세션 첫 조준**만은 학습된 선이 없어 정착 후 weld 가 시작된다(그 뒤 재견착부터는 옛 선 기준으로 미리 블렌드) → [W48].

**관전 폰과의 관계**(2026-09-15 확인, [W50] 해결): 관전 폰(`Observer/SoldierObserverPawn`)이 1인칭(T)일 때 따라다니는 병사의 이 컴포넌트에 뷰를 **빌린다** — `BeginExternalView()`(소켓→시선 캘리브레이션, `bExternalView` 면 알파 무관 소켓 완전 추종) / `EndExternalView()`, 관전 폰 `CalcCamera` 오버라이드 → `ComputeView()`. 빙의 없음. 관전 **H** = 그 병사의 `ToggleHeadAim()`. HUD `headaim:ON/off · / soldier eyes`. Tab·3인칭 복귀·EndPlay 에서 해제. `bUseSoldierFirstPersonComponent`(기본 on) 를 끄면 옛 head 소켓 간이 뷰.

---

## 0. 무엇이 바뀌나 [A]

머리 추종의 **목표**가 둘로 늘었다:

| | 이전 (17:00) | Sight 모드 (`bAlignEyeToSight`, 기본 on) |
|---|---|---|
| 목표 방향 | 컨트롤 회전(마우스) | **총 `sight` 소켓의 X축** |
| 목표 위치 | 없음 | **소켓 − X × `EyeReliefCm` 10** (+ `SightEyeOffset` 트림) 에 **눈 소켓**이 오도록 |
| 적용 | Modify Bone(head) 회전 Additive | + Modify Bone(head) **이동 Additive(월드)** ← ABP 변수 `HeadAimLocation` |

둘 다 **같은 닫힌 루프**다 — 지난 프레임의 최종 눈 소켓 위치/시선과 목표의 오차를 `CorrectionGain`/s 만큼 좁힌다. AO·목·애니메이션이 뭘 하든 알아서 수렴하고, 빠른 흔들림은 통과한다. 이동은 `MaxEyeOffsetCm 12` 로 상한(뺨을 개머리판에 붙이는 양이지 목을 늘리는 양이 아니다) · `MaxEyeStepCmPerSecond 120`.

머리 IK 솔버는 안 쓴다. 몇 cm 는 머리 본 이동으로 충분하고, 목이 보이게 늘어나면 `MaxEyeOffsetCm` 을 줄이거나 조준경 소켓을 옮긴다.

### 0.1 ★ 2차 (18:00) — 총의 *순간* 자세가 아니라 *정착된* 자세를 따른다 [A]

실측: 우클릭 견착 시 총이 올라오며 오버슈트하는 것, 급선회 시 총이 지연되는 것이 **그대로 머리 목표**가 되어 3인칭에서 머리가 덜렁거렸다. 고침 셋:

| | 무엇 | 왜 |
|---|---|---|
| ① 방향 | 소켓 X축이 아니라 **컨트롤 회전**(`bFollowSightDirection=false` 기본) | 총구 되먹임이 총을 결국 거기 수렴시키므로 "조준 완료된 총의 방향" = 컨트롤 회전. 총 과도가 방향엔 전혀 안 들어감 |
| ② 위치 | 소켓이 **정지했을 때만**(각속도 ≤ 30°/s ∧ 선속도 ≤ 15 cm/s 가 0.1 s) 눈 목표를 학습, 그 값을 **메시 프레임**에 붙잡아 둠 | 몸이 돌면 목표도 돌고, 총이 흔들려도 목표는 안 움직임. 피치가 바뀌어 총이 옮겨가면 정착 뒤 새 위치로 |
| ③ 대기 | `AOActive` 후 `AimSettleDelaySeconds 0.35` 지나야 알파 램프 | 올라오는 오버슈트 구간(AO 블렌드 0.375 s + 되먹임 회복)을 아예 안 봄 |

디버그 글자에 `sight: STILL / moving/held / none` 와 `eye=이동량` 이 붙었다.

### 0.2 ★ 3차 (18:30) — 머리 이동 폐기, 목 굽힘으로 [A]

실측: 머리 본을 이동시키니 머리가 목에서 떨어져 "띠용띠용" 거렸고, 조준 중 달리기(총 내림 애니메이션)도 따라갔다.

| | 무엇 |
|---|---|
| **기본 OFF** | `bEnabled=false` (아군·적군 공통, H 로 켬) |
| **정렬 게이트** | `AimAlignmentDegrees 8` — 조준경 X축·컨트롤 회전 내적. 총 올리는 중 · 달리며 총 내림 · 블라인드파이어가 규칙 없이 한 번에 빠진다. `AimSettleDelaySeconds` 는 0 으로 |
| **눈 위치 = 목 굽힘** | `neck_01`(피벗)에서 눈까지의 레버를 돌려 눈을 조준선에 올린다 — 이동 없음. 오차의 레버 직교 성분만 각도로 환산(`|Δ⊥|/|L|`), 닫힌 루프 누적, 레버축 twist 제거, `MaxNeckBendDegrees 30` 상한. `neck_01`/`neck_02` 에 `NeckBendSplit 0.5` 로 나눠 얹는다(ABP 노드 `ModifyBone_9` = neck_02 · 변수 `Neck2AimRotation`, 배선 완료). 머리 방향 루프가 그 위에서 시선을 유지하므로 **고개를 숙이고 눈만 정렬**된다 |
| `HeadAimLocation` | 항상 0 을 쓴다(구 그래프 호환). head 노드 Translation Mode 는 Ignore 로 되돌려도 된다 |

### 0.3 ★ 4차 (19:00) — "완벽하지 않다"의 두 원인 [A]

3차에서 정렬이 미세하게 어긋났다. 카메라 소켓 문제가 아니라 3차 설계의 구조적 오차 둘:

| 오차 | 원인 | 고침 |
|---|---|---|
| 방향 | 머리가 *컨트롤 회전*을 보는데 총의 조준선은 그와 총구 되먹임 잔여(1~2°)만큼 다르다 | 정지·정렬 시 **조준선 방향을 컨트롤 회전 프레임에서 학습**(`SettledSightDirInAim`)해 항상 적용 → 정착 시 조준선과 정확히 일치, 과도 중엔 안 흔들림 |
| 위치 | 목 굽힘은 레버 직교 방향으로만 눈을 옮기는데 목표가 조준선 위 **한 점**(아이릴리프 고정)이라 레버 방향 잔여가 남았다 | 목표를 **선**으로: 조준선에서 현재 눈에 가장 가까운 점(`EyeReliefMinCm 5 ~ MaxCm 20` 범위). 선을 따라가는 성분은 비용이 없으니 목은 가로지르는 일만 한다 |

`EyeReliefCm` 은 사라졌다. 1인칭 `LocationOffset` 은 0 이어야 카메라가 정확히 눈 소켓에 있다.

### 0.4 2단 구조 (19:15) [A]

비조준 1인칭에서 카메라만 돌아 두개골 안이 보였다. 추종을 두 단으로:

| 단 | 조건 | 하는 것 |
|---|---|---|
| **둘러보기** | 켜짐 | 시선 방향 루프만, 목표 = 컨트롤 회전을 몸 facing 에서 `LookAroundStrength`(기본 1) 만큼 간 곳. 눈 정렬·목 굽힘 없음, 학습 방향 오프셋도 안 씀 |
| **정렬(weld)** | + 견착 ∧ 조준경이 조준선 8° 안 | 학습된 조준선 방향 + 목 굽힘으로 눈을 선 위에 |

⚠ `LookAroundStrength < 1` 이면 머리를 타는 1인칭 카메라도 마우스 각도를 다 못 따라간다(카메라 = 머리). 3인칭용으로만 낮출 것.

### 0.5 ★ 5차 (19:30) — 회전만으로는 못 닿는다: 스트레치 [A · 실측]

디버그 실측(정착 상태): 눈 소켓을 눈에 두면 `bend=60/60 · eye-res=2.7cm · look-res=0.1°`, 소켓을 머리 밖으로 옮기면 `bend=40/60 · eye-res=4.8cm`.
**기하의 한계다**: 방향 루프가 머리를 조준선과 평행하게 잡아두므로 목 굽힘으로 눈이 갈 수 있는 곳은 **목 길이(~10 cm) 반지름의 구면**뿐(머리 회전분은 상쇄). 60° 를 다 써도 5 cm 내려간다. 40° 에서 멈춘 경우는 잔여가 레버 방향(방사)이라 회전 기울기가 0 — 그 피벗으로 갈 수 있는 최근접점. 스트레치 없는 2본 IK 의 도달 한계와 같다.
→ **스트레치 복귀, 작게**: 목이 못 하는 성분(레버 방향 성분, 또는 굽힘이 상한에 걸리면 전체 잔여)만 머리 본 **월드 이동 Additive** 로, 게인 절반 · `MaxNeckStretchCm 5` · 30 cm/s. 3차에서 뺐던 "띠용"은 목표가 총의 과도를 쫓아서였고, 지금은 정지 게이트·학습 목표라 목표가 조용하다. ABP `ModifyBone_7`(head) **Translation Mode = Add to Existing · World Space** 다시 필요. `MaxNeckBendDegrees` 30 → 60.
나머지 거리는 시스템 몫이 아니다 — **ADS 포즈의 뺨 높이**와 **조준경 높이(라이저)** 로 눈 가까이 가져오는 것이 정답이고, 시스템은 마지막 몇 cm 만 한다. 디버그에 `stretch=`, `eye-res=`, `look-res=` 추가.

### 0.6 6차 (19:45) — 작은 움직임은 추적, 큰 점프만 홀드 [A]

실측: 카메라를 살짝만 움직여도 0.3 s 뒤 스르륵 재정렬이 보였다. 2차의 "정지 스냅샷 → 움직이면 홀드" 가 원인 — 마우스 피치 몇 도에도 목표선이 옛 값에 묶였다가 정지 후 점프. → 매 틱 메시 프레임의 조준선을 학습값과 비교해 **3 cm·3° 이내면 `LineTrackRate 12`/s 로 연속 추종**, 그 이상(총 올리기·반동·급 피치)만 정지까지 홀드. 남는 지연은 루프 게인(`CorrectionGain 8`) 몫 — 빠르게 하려면 12~15.

### 0.7 7차 (20:00) — 스위치가 아니라 가중치 · 블라인드파이어 차단 [A]

실측: 총내림→조준(또는 급선회 후 재견착)에서 총이 8° 안에 들어오는 **한 프레임에 정렬이 켜져** "턱" 하고 보였다. 이진 게이트의 본질.
→ **weld 가중치** = 조준경–조준선 각도의 smoothstep(`AlignZeroDegrees 15` → 0, `AlignFullDegrees 4` → 1), `WeldBlendRate 12`/s 로 변화율 제한. 총이 올라오며 조준선에 다가올수록 눈이 미리 내려가 만나는 순간이 없다. 위치 루프의 목표는 `자연 눈 위치 + w × (선 − 자연 눈 위치)` — 자연 눈 위치는 지난 프레임 측정값에서 현재 굽힘·스트레치를 되돌려 추정. 출력에 w 를 곱하면 루프가 감아올리므로 목표 쪽에 건다. 방향의 학습 오프셋도 w 로 블렌드. `AimAlignmentDegrees 8` 은 이제 **학습 게이트**로만.
**블라인드파이어**: ABP `BF_AlphaL/R/U` 중 하나라도 `BlindFireThreshold 0.05` 를 넘으면 머리 추종 **전체**(둘러보기 포함) off — 저작된 머리 포즈와 다투지 않는다.

### 0.8 8차 (20:30) — 보정은 몸 프레임에 산다 [A]

실측: 시야를 확 돌리면 머리가 이상한 축으로 꺾였다(조준 여부 무관, H 끄면 정상). 원인 둘 — ① 누적 보정(머리 회전·목 굽힘·스트레치)을 **월드 공간**에 들고 있어서, "턱 아래로 15°" 가 몸이 90° 돌면 몸 기준 **롤**이 됐다. → 프레임 사이엔 **메시 프레임**에 저장하고 틱 시작에 `M·q·M⁻¹` 로 월드 변환, 끝에 되돌림. ② 클램프 기준이 캡슐 회전이었는데 GASP 급선회 시 메시가 캡슐에서 최대 90° 벗어나(OffsetRootBone) 메시 기준 160° 를 요구했다. → 기준을 **메시 facing**(`MeshQuat × FacingLocal`)으로. 1인칭 니어플레인 `NearClipPlaneCm 2` 도 같은 시각 추가.

### 0.9 9차 (21:00) — weld 래치 · 토르소 프레임 [A]

실측: 8차로도 조준 중 시야 급회전 시 "용수철". 원인 = 7차 가중치 자체 — 급선회 때 GASP 가 총을 잠깐 내리면(2.5d) 각도가 벌어져 w 가 0.3~0.5 로 떨어지고 눈이 풀렸다가 다시 붙는 왕복. 가중치는 **처음 올라올 때만** 필요하다.
→ **래치**: w 가 1 에 닿으면 잠금. 견착 유지 중 잠깐 벗어나는 건 `UnweldDelaySeconds 0.5` 까지 무시(눈은 몸 프레임의 선에 그대로, 총만 시야에서 나갔다 돌아옴). 그 이상 벗어나거나 견착 해제 시 정상 해제.
→ 8차의 "메시 컴포넌트 프레임"은 틀렸다 — OffsetRootBone 은 컴포넌트가 아니라 **루트 본**을 돌리므로 컴포넌트 회전 = 캡슐 회전. 몸 프레임은 **`TorsoBone`(spine_03) 의 실제 회전 × 레퍼런스 상대회전**으로. 클램프·보정 저장·학습된 조준선 세 가지 모두 이 프레임. 디버그 `weld=x.xxL`(L = 래치).

### 0.10 10차 (21:20) — 스프링의 진짜 원인: 피치 재조립 버그 [A]

실측: 급선회로 총이 내려갈 때 고개가 **위로** 솟았다. 9차에서 몸 기준을 캡슐(피치 0) → 척추 본(피치 있음)으로 바꾸면서, 클램프 뒤 목표 재조립이 `FRotator(Delta.Pitch, Body.Yaw + Delta.Yaw, 0)` — 야우는 몸에 더했는데 **피치는 델타 그대로**. 상체가 앞으로 20° 숙이면 목표 피치가 조준보다 20° 위. 상체가 돌아오면 내려오니 스프링으로 보였다. → `Body.Pitch + Delta.Pitch`. 7~9차의 가중치·래치는 이 버그를 가리고 있던 셈이지만 그 자체로도 필요한 장치라 유지.

### 0.11 11차 (2026-09-15 새벽) — 몸이 머리를 따라 돈다 · Strafe 우회 폐기 · neck_02 수동 완료 [A]

**증상**: H 켜고 비조준으로 카메라를 돌리면 머리만 돌고(클램프 ±75°) 몸은 안 돈다. GASP 는 조준/스트레이프에서만 turn-in-place 를 낸다.

**시도 1 — GASP Strafe 모드 (폐기)**: `CharacterInputState.WantsToStrafe` 를 리플렉션으로 켜서 GASP 가 캡슐을 돌리게 했다. 동작은 했지만 **Strafe/Aim 은 회전 모드이자 곧 견착 상태**라 우클릭 없이 항상 조준 자세가 됐다 → 사용자 취소, 코드 제거 (P120).

**시도 2 — 캡슐만 C++ 로 (채택, 사용자 확인 "잘됨")**: 근거는 GASP `UpdateRotation_PreCMC` BP 실측 — GASP 는 액터 회전을 직접 쓰지 않고 **CMC 플래그만** 고른다: 조준/스트레이프 = `bUseControllerDesiredRotation`(RotationRate −1, 즉시), 그 외 = `bOrientRotationToMovement`(**가속이 있을 때만** 회전, idle 은 손대지 않음). 그러니 idle 비조준에서 우리가 `SetActorRotation` 으로 캡슐 yaw 를 돌려도 CMC 가 되돌리지 않고, 메시는 OffsetRootBone 이 붙잡고 MM 이 TIP 클립을 고른다 — **조준 모드에서 마우스를 홱 돌릴 때와 같은 경로**. WASD 가 들어오면 정지 조건이 깨져 우리가 물러나고 CMC(OrientToMovement)가 캡슐을 가져간다 → 달리는 동안은 지금처럼 머리만 카메라를 본다. 값은 0' 절 "몸 회전" 단락. AI 는 컨트롤러가 돌리므로 플레이어 전용.

**ABP 경고 2건**(컴파일러: "You must pick a bone" · "No components to modify"): 실측 `ModifyBone_9`(neck_02 자리)가 **Bone None · Rotation Ignore** 로 남아 있었다 — 9차에 배선만 하고 P108(노드 프로퍼티 수동)이 누락. 그때까지 목 굽힘은 `NeckBendSplit 0.5` 중 neck_01 몫만 실제로 들어갔고 닫힌 루프가 그만큼 더 굽혀 메꿨다. 사용자가 neck_02 · Add to Existing · World Space 로 채움 → 두 마디로 나뉜다. 잔여 `ModifyBone_8`(head, 전부 Ignore, 핀 없음)은 통과 노드 — 삭제 권고 → [W54].

## 1. 게이트 [A]

~~견착(`AOActive`) ∧ 켜짐(H) — 총 내림·달리기에선 회전·이동 모두 0 으로 풀린다.~~
~~정렬만 `AOActive` ∧ 조준경 +X 가 컨트롤 회전 8° 안일 때 (이진).~~
⚠ **최종(0' 절)**: 켜짐(H)이면 시선 추종(둘러보기)은 **항상** 돈다 — 안 그러면 비조준 1인칭에서 두개골 안이 보인다. **정렬(눈 위치·학습 방향)** 은 스위치가 아니라 **가중치**: 견착(`AOActive`) 중 조준경이 조준선에 다가오는 각도(35°→3°)로 0→1, 붙으면 래치(0.5 s 벗어남 무시). 총 올리는 중엔 총과 같은 리듬으로 붙고, 달리며 총 내림·급선회 dip 은 래치가 흡수하거나(짧으면) 풀린다(길면). **맹목사격은 ABP `BF_Alpha*` 를 읽어 머리 추종 전체를 끈다**(저작된 머리 포즈와 다투지 않음) → [W46] 해결. 린은 총이 조준선에 남으므로 안 빠지고 그게 의도(롤은 정렬 대상 아님).

## 2. 1인칭 카메라와의 접합 [A]

- 1인칭 `FollowSocket` 진입 시 정렬 기준을 **마우스가 아니라 머리의 실제 시선**(`GetHeadLook`)으로 잡는다. 진입 순간 머리가 총을 아직 못 따라잡았어도 오프셋이 남지 않는다.
- 눈 소켓은 **한 곳에서 정한다**: `EyeSocket` 이 None 이면 1인칭 컴포넌트의 `CameraSocket` 을 쓴다. 카메라와 정렬 루프가 같은 점을 "눈"으로 본다.
- 조준경을 정확히 들여다보려면 1인칭 `LocationOffset` 은 0 (또는 X 만 몇 cm).

## 3. 준비물 (사용자) [A]

1. **총 메시(`WeaponMesh`)에 `sight` 소켓** — 조준경 접안(또는 가늠쇠 뒤) 위치, **X 축이 총열 방향, Z 위**. 소켓이 맞는지는 1인칭 컴포넌트로 바로 확인 가능: `Anchor=WeaponMesh · CameraSocket=sight · FollowSocket · bAlignToAimOnEnter=false` → 카메라가 조준경을 정확히 들여다보면 OK. 확인 뒤 `Anchor=BodyMesh` 로 되돌린다.
2. ~~ABP `ModifyBone_7`(head): Translation Mode = Add to Existing~~ ~~→ 머리 이동은 폐기(0.2절)~~ → **최종(0.5절, 스트레치 복귀)**: `ModifyBone_6`(neck_01)·`ModifyBone_9`(neck_02)·`ModifyBone_7`(head) 세 노드 **Rotation Mode = Add to Existing · World Space**, 그리고 **`ModifyBone_7`(head) 의 Translation Mode 도 Add to Existing · World Space**(`HeadAimLocation` = 스트레치). P108 — MCP 가 못 쓰므로 사용자 수동. ⚠ `ModifyBone_9`(neck_02)는 2026-09-15 새벽에야 채워졌다(0.11절) — 컴파일러 경고가 그 서명이었다. 이제 셋 다 완료.
3. 빌드 → 사용자 확인: 3차 "잘됨", **10차 "성공. 이제 모든게 완벽해"** — [W47] 해결.

## 4. 판정 기준 → 결과 [A]

`SoldierLab.Debug.HeadAim 1`: **시안 구 = 목표 눈, 마젠타 구 = 실제 눈 소켓** — 견착 정지 시 겹쳐야 한다. 초록/빨강(방향)도 겹침. 글자의 `eye-res` · `look-res` 가 그 숫자다.

| 기준 | 결과 (2026-09-14 21:30 사용자 확인) |
|---|---|
| 1인칭 견착 정지: 크로스헤어 = 조준경 중심 | ✅ 5차 실측 `look-res 0.1°`, 스트레치 후 `eye-res` ≈ 0 |
| 마우스 급선회: 총이 먼저 돌고 조준경이 시야를 가로질렀다 재정렬 | ✅ 10차 이후. 그 전의 "용수철"은 피치 재조립 버그(0.10절) |
| 총내림/달리기 전환: 머리가 부드럽게 복귀(팝 없음) | ✅ 가중치+래치(0.7·0.9절) |
| 총 올라올 때 정렬이 켜지는 순간이 안 보임 | ✅ 창을 35°부터 열어서(0.7절 정정) |
| 3인칭: 뺨이 개머리판에 붙고 목이 늘어 보이지 않음 | ✅ 굽힘 60° + 스트레치 5 cm 안에서. 나머지 거리는 ADS 포즈·조준경 높이 몫 → [W49] |
| 비조준 1인칭에서 두개골 내부가 안 보임 | ✅ 둘러보기 단(0.4절) |
| 맹목사격 시 저작된 머리 포즈가 그대로 | ✅ 전체 off(0.7절) |
| 사격 반동: 조준경이 튀고 시야가 같이 튄 뒤 복귀 | [C] 별도로 안 봤다. 래치 0.5 s 안이면 붙어 있을 것 |
