# 노출 회계와 총구 학습 — 위험 지도 이후의 실측 라운드 6건

2026-09-15 / 진행중 (코드·빌드 완료, 사용자 평가 "지금까지는 가장 좋네", 값은 전부 [C]) / `ai/2026-09-14_danger_map_and_position_commitment.md`의 첫 빌드 뒤 **로그를 읽고 고친 여섯 라운드**. 예산 게이트 잠김 → 높이 실측 → 가치 게이트 분리 → 경로 상한 제거 → 사격 활동도 → **노출 회계(엄폐↔사격 사이클)** 와 **총구 소켓 실측·포즈별 오프셋 학습**.

전편: `ai/2026-09-14_danger_map_and_position_commitment.md`(위험 지도·모든 눈·머무름·표적 잠금 — 그 문서의 값 중 이 문서가 바꾼 것은 그쪽 끝 정정 절에 있다).
원본: `ai/2026-09-13_engagement_and_cover.md` · `ai/2026-09-13_objective_and_position_cost.md` · `ai/2026-09-14_exposure_ladder_and_corrections.md`.
원칙: P10(계측 먼저) · P44(과도구간) · P84(비교용 비용에 천장 금지) · P89(규칙이 아니라 선호) · **신설 P114~P118**(`CLAUDE.md` 5절).

> **이 문서의 방식**: 각 라운드는 **로그로 확정한 사실 [A] → 원인 → 수정 → 값(← 옛값)** 순이다. 로그는 `SoldierLab.Debug.Cover.Log 1` · `SoldierLab.Debug.Engagement.Log 1`(카테고리 `LogSoldierAI`, MCP `LogsToolset.GetLogEntries`로 읽어 파이썬으로 세션별·병사별로 갈랐다). 거동의 최종 평가는 사용자 육안 한 줄뿐이고 **수치 판정은 12절에 남아 있다.**

---

## 0. 한 장 요약

| R | 증상 (사용자) | 로그가 말한 것 [A] | 고친 것 | 파일 |
|---|---|---|---|---|
| **R1** | 이동을 아예 못 함 | 세션 전체 `[Cover] sweep` **0줄**, 오버레이 `#0 … cand 0` | 예산 하한 = 후보 1개 | Cover |
| **R2** | 소극적 · 큰 건물 뒤에서 놂 | 가슴 소켓 **96~105 / 50~57 cm**(상수 135/80) · BLIND 0.5가 건너기보다 쌈 · 8 m 옆 담을 후보로 못 찾음 · 첫 4 s는 웅크림 80 | 부채꼴 후보 · fight probe = 눈/총구 · 웅크림 초기 추정 · `NoFiringPositionCost 0.9` · 머무름은 엄폐 있을 때만 · 눈 스무딩 | Cover · Identity |
| **R3** | 총을 거의 안 쏨 | 아군 `hold` 전이 **35/35 = worth 0** | 가치 게이트를 총 산포만으로 · 제압사격의 앎 한도 분리 · 관찰 | Engagement |
| **R4** | 한 명만 돌격, 나머지 정지 | 12 m 클램프로 **긴 도약이 미터당 쌈** · 적 `blocked` 30 s(눈은 넘고 총구는 못 넘는 담) | 경로 위험 길이 비례 · 부채꼴을 눈 전부에서 · fight probe = min(눈, 총구) | Cover · Identity |
| **R5** | 소강에도 정지 | 전진 후보의 "공터였던 기억" 0.55~0.8이 이득 0.43을 이김 · 건너는 위험이 "쏘고 있나"를 안 봄 | 기록에 마지막 총성 시각 · 눈 활동도 가중 · 자리 기억은 눈 0일 때만 | Perception · Cover · DangerMap |
| **R6** | "옛날 FPS처럼 몸이 왔다갔다하며 쏨" | 사이클이 아니라 **래치** + 담 꼭대기 **진동 버그** | **노출 회계**(ExposureAccount) · 계획된 조리개 · 실제 총구 소켓 · 포즈별 총구 오프셋 학습 · 코너 판정 | Engagement · Identity · Cover |

**남은 것(사용자 다음 요구)**: 겹치는 엄폐 자리(예약 없음, 특히 수비수) · 분대 통신·화망 · "노는 병사"의 정확한 원인 · 사망/대가 → 13절.

---

## 1. R1 — 예산 게이트가 잠겨 병사 전원이 부동 [A]

**사실**: 위험 지도 빌드 뒤 첫 세션의 `LogSoldierAI`에 `[SoldierIdentity] … measured` 7줄뿐, `[Cover] sweep` 은 **0줄**. 오버레이 둘째 줄 `#0 best … =-1.00 @0m cand 0 (fan 0) -> stay +m0.00 dwell -1.0s`. 공터에 선 병사가 제압 0.44를 받으며 그대로 서 있었다(스크린샷 05:54).

**원인**: 후보 하나의 트레이스 = `(1 fight + 3 heights + 8 routes) × 눈 3 + 스냅 1 = 37` > `MaxTracesPerTick 36`. `while (Budget >= SamplesPerCandidate …)` 가 **한 번도 참이 되지 않아** 후보 0개 평가 → 스윕이 영영 안 끝남 → `MoveToLocation` 0회. 적이 2명일 땐 돌고 3명이면 멈추는 구조였다.

**수정**: `Budget = max(MaxTracesPerTick, SamplesPerCandidate)` — **틱당 최소 후보 1개**. 예산은 속도(rate)이지 허가(permission)가 아니다 → **P114**.

---

## 2. R2 — 몸의 자를 상수로 두고 있었다 [A]

### 2.1 실측

| 병사 | 가슴(spine_03) 기립 / 웅크림 | 눈(head) | 상수였던 값 |
|---|---|---|---|
| Friendly ×4 (soldier_T) | **96.1~99.3 / 49.6~55.4 cm** | 140~150 | 135 / 80 / 160 |
| Hostile ×3 (마네킹) | **103.9~105.5 / 54.4~57.1 cm** | 140~151 | 〃 |

(`SoldierIdentity::ObserveStance` 로그. 눈은 R4 이후 min(눈, 총구 140)이라 140으로 찍힌다.)

**귀결**: 높이 표본 135/107/80이 실제 몸보다 **40 cm 높았다**. 1 m 안팎의 담(Low_Plaza 100 · Crate 110 · Med_Def 135)이 전부 "숨지만 못 쏨"(BLIND)으로 읽혔고, 74 cm 담(Low_Def_A/B)은 엄폐로 안 잡혔다. 그리고 **웅크림 높이는 병사가 처음 웅크릴 때까지 80으로 남아** 첫 결정(세션 시작 ~4 s)이 틀린 자로 내려졌다.

### 2.2 수정

- `bMeasureChestHeights`: 기립을 재는 순간 웅크림을 **기립 × (초기 80/135) 로 추정**해 채우고, 실제로 웅크리면 실측으로 교체. 눈 소켓도 같이 잰다(`MeasuredEyeHeightCm`).
- **"쏠 수 있나"(bCanFight) 를 가슴이 아니라 눈 높이에서** 묻는다(`GetFightProbeHeightCm`). 적이 시험하는 건 내 가슴(숨었나), 내가 시험하는 건 내 눈·총구(쏠 수 있나) — 두 질문의 자가 다르다 → **P115**. R4에서 min(눈, 총구)로 좁혀짐.
- 로그 `heights measured: chest stand X / crouch Y, eye Z (minimum wall that is cover: Y)` — 디자이너용 최소 담 높이.

### 2.3 같은 라운드의 나머지 (로그 근거)

| 사실 [A] | 수정 | 값 |
|---|---|---|
| Ally_A/B: 30 s 중 157/171회 `HERE f0.50`(BLIND), `best 0.50 → stay` 수십 초 — **BLIND(0.5) < 건너기(≤0.6)+여유(0.3)** | `NoFiringPositionCost` | **0.9** (← 0.5) |
| Ally_A 첫 결정: 8 m 앞 Low_Def_A 대신 12 m 뒤 건물로. Ally_B3: 7 m 옆 상자 못 찾고 **11 s 공터** — 링 12점이 그 자리에 안 떨어지고 그림자 스냅은 "눈과 링점 *사이*"만 | **부채꼴 후보**: 가까운 눈에서 병사 주변 원판(`FanRadiusCm`)을 향해 `FanRays`줄 × 2높이(기립·웅크림 가슴)로 쏴서 맞은 곳 60 cm 뒤마다 후보(`ShadowBehind`: 도달 반경 안·네브메시 투영·투영점이 여전히 충돌점보다 먼 쪽). 링 12점은 예비(눈 0일 때 목표 기울기용). 1.5 m 내 후보 병합 | `bFanCandidates true` · `FanRadiusCm 2400` · `FanRays 32→24`(R4) · `CandidateMergeCm 150` |
| Ally_A t=1.6~3.8: 정지 상태에서 `HERE` 0/0.5/1.0 깜빡임 — 총성 기록 지터(±0.25×거리)로 눈이 수 m씩 이동 | 눈 위치 스무딩(`FSmoothedEye`, 액터 또는 근접으로 매칭, 2 s 미갱신 시 폐기) | `ThreatEyeSmoothingSeconds 1.0` |
| Ally_B3 t=2.3~13: `HERE 1.83 vs best 0.92 → stay(dwell)` — **공터에 선 병사를 머무름 여유 1.0이 붙잡음** | 머무름은 `bCanHide`일 때만 | — |
| 20 m 도약을 표본 3점으로 판정 | 경로 표본 간격 기반(`RouteSampleSpacingCm 400`, `MaxRouteSamples 8`), 땅 투영 | (← `RouteSamples 3`) |
| 예산 | `MaxTracesPerTick` | **36** (← 19 ← 6) |

---

## 3. R3 — 안 쏘는 이유는 게이트 하나였다 [A]

**사실**: 아군 4명의 `[Engage]` 전이 중 `→ hold` **35회 전부 `worth 0`**(그중 2회는 `+offtarget`). 적군도 `notworth` 43회. 아군은 `hold` 42~48 s / `blocked` 10~20 s / `MASKED` 7~11 s, `AIMED` 0.2~2.5 s. 적군은 `AIMED` 10~12 s.

**원인**: `bWorthTheRound = √(know² + spread²) ≤ SuppressiveRadiusCm 500`. 50~60 m에서 총 산포만 262~313 cm이고, 적이 엄폐 뒤라 **거의 안 보이니** 위치 기억 반경이 4~9 m(사격 40 cm + 200 cm/s 성장)로 자라 합이 5 m를 넘는다 → 거절. 이 게이트(P67)는 "뛰면서 40 m 사격"·"담 위로 총만 흔들기" 같은 **총 쪽 오차**를 막으려던 것인데 **앎 쪽 오차**까지 묶어, "저 상자 뒤 어딘가"라는 앎으로는 제압사격조차 못 하게 돼 있었다. 적은 아군이 공터에 서 있어 계속 보이니(`know 40~70`) 쐈다.

**수정**:
```
bWorthTheRound = SpreadCm ≤ SuppressiveRadiusCm 500          ← 총 오차만 (뜻이 좁아짐)
Aimed         : RadiusCm ≤ SpreadCm × KnowledgeToSpreadRatio  (그대로)
Suppressive   : RadiusCm ≤ SuppressiveKnowledgeRadiusCm 1000 (신설 — "어느 엄폐물 뒤인지는 안다")
                ∧ MagFraction > SuppressiveReserveFraction 0.4
```
그리고 **관찰**: 접촉 기억이 `ObserveAfterSeconds 1.5`보다 낡고 제압 < `ObserveSuppressionThreshold 0.3`이고 쏠 게 없고 지금 자리에 엄폐가 있으면 엄폐의 "숙여라"를 무시하고 일어서서 본다(자기 잠금 — 숙임→못 봄→기억 낡음→못 쏨→일어날 이유 없음 — 을 끊는다). → R6에서 노출 회계의 **관찰 회차**로 흡수됐다.

**덤(기록만)**: `MASKED` 8~11 s — 아군 둘이 같은 자리로 몰려 사선에 서로 들어감. 자리 예약 없음 → **[W51]**.

---

## 4. R4 — 한 명만 돌격한 이유는 경로 비용의 천장 [A]

**사실**(벽을 높인 레벨, 48 s 세션): Enemy_A3 `MOVE` 8회, 19~22 m 도약 3연속으로 t=47 s에 Ally_B3 1 m 앞까지; Enemy_A2 `stay` 275/314회 — 11 m 앞 엄폐 앞에서 20 s 넘게 정지.

**원인**: 경로 위험 `Span = clamp(L / SearchRadiusCm 1200, 0, 1)` — **12 m 넘는 도약은 전부 0.6**. A3의 22 m 도약: 비용 0.6 vs 목표 이득 1.47 → 크게 남음. A2의 11 m: 0.55 vs 0.74 → 0.19 남는데 여유 0.3 미달. **짧은 도약이 긴 도약보다 손해**라는 역전 — P84("비교용 비용에 천장 금지")를 위험 지도 라운드에서 스스로 어긴 자리.

**수정**: `Span = L / SearchRadiusCm` **클램프 제거**(단위는 12 m).

**같은 세션의 다른 사실**: 적군 `blocked`(믿지만 총구 못 내밈) **29~33 s**. 사용자가 벽을 높여 **눈(150)은 넘는데 총구(140)는 못 넘는 담**이 생겼고, fight probe가 눈 높이라 그 자리를 사격 위치로 골라 놓고 정작 총은 막힘. → `GetFightProbeHeightCm = min(눈, StandMuzzleHeightCm)`(Engagement가 BeginPlay에 총구 높이를 Identity에 넣어 준다). 그리고 부채꼴을 **가까운 눈 하나가 아니라 기억 속 눈 전부(≤3)** 에서 — 한 눈의 그림자가 다음 눈에겐 공터라 `HERE`가 스윕마다 OPEN으로 뒤집히던 것. `FanRays 32 → 24`(스윕당 트레이스 = 24 × 2 × 눈).

---

## 5. R5 — 소강 상태에서도 안 움직이는 구조적 이유 [A]

**사실**(66 s 세션): Enemy_A3 `stay` 194/194. `HERE f0.00 o4.10 =4.10`(좋은 엄폐) vs 7 m 앞 엄폐 후보 `f0.00 r0.35 o3.67 d0.55~0.74 =4.6`. Enemy_A2: 주변이 전부 BLIND(0.9)라 기울기 0. 총성이 멎은 구간에서도 값이 그대로.

**원인 둘**:
1. **자리의 "공터였던 기억"이 엄폐를 벌한다** — 위험 지도가 2 m 격자라 "상자 뒤"와 "상자 옆"이 같은 칸. 다른 각도에서 한 번 공터로 찍히면 30 s간 그 칸의 엄폐까지 0.55~0.8. 눈이 살아 있을 땐 실시간 트레이스가 정확한데 기억이 그걸 덮었다 → **P116**.
2. **건너는 위험이 "적 눈에 보이나"만 보고 "적이 지금 쏘고 있나"를 안 본다** — 기억 속 적은 20 s 넘게 남으니 총성이 멎어도 비용이 그대로. 사람은 "사격 멎었다, 지금 뛰어"를 한다 → 노출의 값은 *보는 눈*이 아니라 *쏘는 총*에 비례해야 한다 → **P117**.

**수정**:
- `FSoldierEnemyRecord::LastGunshotTimeSeconds` (신설, `ReportGunshot`에서 찍고 `FuseRecords`는 max). 총성은 위치 게이트(양 반경 합)로 시야 기록에 융합되므로 같은 적의 기록에 붙는다.
- 눈마다 **활동도**: 최근 `FiringRecentSeconds 3` 안에 쐈으면 1.0, 그 뒤 `FiringFadeSeconds 5`에 걸쳐 `QuietEyeWeight 0.3`으로, 한 번도 안 쏜 적은 0.3. **건너는 위험(실시간 = 그 점을 보는 눈 중 최대 활동도)과 "보였던 기억"(× 스윕 최대 활동도)에 곱한다.** "총알이 지나간 기억"과 "공터에 서 있음"(NoCoverCost)은 그대로(`GetRouteDangerSplit`).
- 자리의 공터 기억(`GetPositionDanger`)은 **눈이 0일 때만** 적용.
- 로그·오버레이에 `act=`(스윕 최대 활동도).

**기대 산수**: 총성 3 s 멎으면(`act 0.3`) 7 m 전진의 건너는 값 0.35 → 0.1, 목표 이득 0.43이 여유 0.3을 넘음 → 소강 때 전진. 교전 중(`act 1.0`)은 이전과 같음. **실측 미완** → [C-104].

---

## 6. R6 — 엄폐↔사격 사이클은 없었다: 노출 회계 [A · 코드]

### 6.1 무엇이 있었고 무엇이 없었나

| 부품 | 있었나 |
|---|---|
| 이 자리에서 얼마나 숙여야 숨나 (`RequiredStance`) | ✅ |
| 막힌 총구의 우회 (Direct→Over→Right→Left) | ✅ |
| 우회에 몸을 얼마나 내보내나 (Open/Lean/Blind, 제압 ≥ 0.45 또는 4 m 안이면 Blind) | ✅ |
| Q/E 린 · 블라인드파이어 포즈 축 | ✅ |
| **노출 ↔ 엄폐를 오가는 리듬** | ❌ |

사격 결정이 **매 틱 독립**이라 "쏠 가치 있는 표적이 보이는 한 계속 노출"이었다. 일어서게 하는 건 `Over` 한 줄(`DesiredStance = 0`), 다시 숙이게 하는 건 제압 0.45 또는 표적 소실뿐 — 사이클이 아니라 **래치**.

**"몸이 왔다갔다"는 리듬이 아니라 버그였다**: 숙인 채(총구 95 < 담 100) → 막힘 → `Over` → 일어서라(0) → 일어서면 `Direct`가 뚫림 → `Direct` 분기는 자세를 안 건드려 엄폐의 "숙여라(1.0)"가 이김 → 내려감 → 막힘 → `Over` → … **담 꼭대기가 진동의 축**. 매 틱 사선을 다시 물어 자세를 정한 결과 → **P118**.

### 6.2 설계 — 노출은 회계다

규칙("3 s 쏘고 2 s 숨어라")이 아니라 **노출이 값을 갖고 쌓이고 빠지는 양 하나**(제압도가 "맞고 있는 느낌"을 모델링한 것과 같은 꼴).

```
ExposureAccount (0..1)
  오름: 몸이 보이는 동안 — ActualStance < CoverStance − 0.1  또는  |ActualLean| > 0.5
        속도 = 표적 활동도 × (1 + Suppression) / ExposureRiseSeconds 1.5
        (활동도: 표적 기록의 LastGunshot 기준, PeekFiringRecentSeconds 3 / PeekFiringFadeSeconds 5 / PeekQuietActivity 0.3)
  내림: 숨어 있는 동안 — 1 / ExposureFallSeconds 2.0
  블라인드(총만 내밈)는 오르지 않는다
결정 (히스테리시스)
  숨음 → 내밈:  ExposureAccount < PeekStartBelow 0.2  ∧  (쏠 가치 ∨ 관찰 필요)
  내밈 → 숨음:  ExposureAccount > PeekStopAbove 0.8  ∨ 표적 소실 ∨ 너무 위험해짐(bTooDear) ∨ 블라인드 불필요 ∨ 포즈 끝났는데 막힘
```

물리적 근거: 상승 시간상수 ≈ 적이 나타난 몸에 총을 돌려 정착시키는 시간(240 °/s 선회 + 시야 갱신 + 콘 정착 ≈ 1~2 s), 하강 ≈ 내가 숨은 뒤 적의 내 위치 기억이 낡는 시간(반경 200 cm/s 성장 vs 그 거리의 산포). 둘 다 우리 상수의 귀결이고 AI는 코드 한 벌(P4)이라 내 상수로 적을 예측한다. **값 자체는 [C]** → [C-105].

### 6.3 계획된 조리개 — `PlanAperture`

숨은 채로 "어느 우회가 뚫릴까"를 **예측 총구 위치**로 미리 정한다. 후보 순서(비용 낮은 순):

```
Direct/Open   실제 총구(소켓)에서              ← 아무 재주도 안 씀
Over/Open     PredictMuzzle(stance 0)            ← 낮은 담 (bTooDear면 제공 안 함)
Right/Lean    PredictMuzzleLean(stance, +1)      ← 코너 (〃)
Left/Lean     PredictMuzzleLean(stance, −1)      (〃)
Over/Blind    PredictMuzzleBlindUp(stance)       ← 항상 제공, bTooDear면 이것부터
Right/Blind   PredictMuzzleBlindSide(stance, +1)
Left/Blind    PredictMuzzleBlindSide(stance, −1)
```
첫 번째로 `IsShotBlockedByWorld`(LaneTolerance 200, 부착 액터 무시)가 뚫리는 것을 채택. **내미는 동안은 자세를 다시 묻지 않는다** — 계획된 자세를 유지(Over/Open → stance 0, Lean → `DesiredLean ±1`, Blind → `DesiredBlindFireV/H`). Direct/Open은 자세를 안 건드린다(숙인 채 쏘는 상자 뒤 케이스).

- 엄폐가 없는 자리(`!bCanHide`)는 사이클 없이 계획대로 즉시 적용(몸이 어차피 보인다).
- 숨은 채 실제 사선이 뚫리면(`bHiddenAsAsked ∧ bActualLaneClear`) 노출 없이 그대로 쏜다.
- **관찰 회차**: `bWorthShot`이 아니고 `bStale(age > 1.5 s) ∧ 제압 < 0.3`이면 쏘지 않고 보기만(`WantsToObserve`) — 보는 중에 게이트가 열리면 쏜다.
- 재장전은 사이클을 끊고 숙인다.
- 종료 조건에 **"포즈에 도달했는데도 막힘"**(`bPlanFailed`)이 있어 예측이 틀리면 다시 계획한다.

### 6.4 총구 — 실제 소켓을 읽고, 포즈별 오프셋을 배운다

**① 실제 총구(사격용)**: `USoldierIdentityComponent::GetMuzzleLocation` — `MuzzleSocket "Muzzle"`(SK_AR4_X)을 캐릭터 자신과 **부착 액터**(`GetAttachedActors`)의 SceneComponent에서 찾아 캐시(1 s 간격 재탐색). **방아쇠 판정(`bActualLaneClear`)은 이 위치에서 표적까지 선이 뚫릴 때만** — 린/블라인드 포즈가 총을 어디 뒀든 벽에 안 쏜다. "얼마나 총이 이동하는지"를 아무도 알려 줄 필요가 없다.

**② 예측 총구(계획용)**: `ObservePose(Stance, Lean, BlindFireH, BlindFireV)` — 축이 끝에 닿았을 때 실제 총구의 **몸 기준 오프셋**(발 원점 + 액터 yaw)을 기록. 기립/웅크림은 절대값, 린·블라인드 3종은 그 스탠스 총구에서의 **델타**. 첫 표본은 대체, 이후 0.05 스무딩. 실제 축은 캐릭터 BP 변수 `StanceAxis`(`SetActualStance`로 들어옴) · `LeanCurrent` · `BlindFireH` · `BlindFireV`를 **리플렉션**으로 읽는다(`ReadActualPoseAxes`, HeadAim 컴포넌트와 같은 패턴 — BP 수정 없음).

| 오프셋 (몸 기준 X앞/Y오른쪽/Z위, cm) | 기본값 (측정 전) [C] |
|---|---|
| `StandMuzzleOffsetCm` | (40, 20, 140) |
| `CrouchMuzzleOffsetCm` | (40, 20, 95) |
| `LeanRightMuzzleDeltaCm` / `LeanLeft…` | (0, +70, −10) / (0, −70, −10) |
| `BlindUpMuzzleDeltaCm` | (0, 0, +55) |
| `BlindRightMuzzleDeltaCm` / `BlindLeft…` | (−10, +80, 0) / (−10, −80, 0) |

`MuzzleAtStance`도 이제 `PredictMuzzle`을 쓴다(Identity 없을 때만 옛 140/95). 블라인드의 정확도는 이미 `BlindSpreadScale 4.0`이 처리한다 — 판정에 필요한 건 "총이 벽을 넘었나"뿐이고 그건 ①이 답한다.

### 6.5 코너를 사격 위치로 — Cover의 fight probe

지금까지 "쏠 수 있나"는 눈/총구 높이 한 점이라 **코너는 늘 BLIND(0.9)** 였다(벽면 뒤라 정면이 안 보이니). 판정에 **린 좌·우 두 점**(`GetLeanReachCm` = 학습된 린 델타의 |Y|, 기본 70; 옆 방향은 그 눈을 향해 섰을 때의 오른쪽 = `Up × Forward`)을 더해 "옆으로 내밀면 보인다"도 사격 위치로 친다. 높은 벽의 코너가 낮은 담과 같은 0점을 받고 거기서 린 사이클이 돈다. 후보당 눈마다 최대 +2 트레이스, 예산 산수 `(3 + heights + routes) × eyes`.

### 6.6 나올 그림 [B]

낮은 담 → 숙임 → (계정 낮음) 일어서 사격 ~1.5 s → 숨음 ~2 s → 반복. 총성이 몰리면(제압 0.3) 내밈이 짧아지고 숨음이 길어진다. 0.45 넘으면 총만 올려 블라인드. 코너 → 같은 리듬을 Q/E 린으로. 조용하면(활동도 0.3) 길게 내밀고 짧게 숨는다 — 아무도 안 쏘는데 계속 숙였다 일어서는 모양은 안 나온다. **사용자 육안 평가: "지금까지는 가장 좋네"(2026-09-15 새벽). 수치 판정은 12절.**

---

## 7. 그 밖에 확인된 것

- **표적 잠금**(전편 7절)은 동작 확인: 오버레이 `switches 0`, `aimErr 0.0` [A].
- 레벨: 사용자가 R4 전에 **벽 높이를 올렸다**(어느 액터를 얼마나인지 미실측) → [C-106]. 전편 1.3의 높이 표는 그 이전 값.
- 적군의 `hold` 29.6 s(Enemy_A3, 아군 1 m 앞) — `BlindFireCloseRangeCm 400` 안이라 Blind → 산포 ×4 → worth 0. 사망이 없어 얼굴을 맞대는 상황 자체가 W18의 귀결.

---

## 8. 컴파일·규칙 사고 2건

| 사고 | 내용 | 교훈 |
|---|---|---|
| **C4458 ×2** | `FindAperture`의 지역변수 `Aperture`/`Posture`가 클래스 멤버를 가림. 첫 수정은 엉뚱한 곳(`PlanAperture`의 지역 구조체 필드)을 고쳐 재발 → 컴파일러가 가리킨 **줄 번호를 먼저 볼 것**. P75(`AActor::Role`)와 같은 유형 | 도메인에서 자연스러운 이름일수록 이미 쓰이고 있다 |
| **`sed -i` 편집 1건** | 예산 산수 `+1 → +3` 두 줄을 `sed`로 치환(결과는 grep으로 검증됨). 프로젝트 규칙(소스는 Edit/Write만) 위반 | 규칙은 규칙 — 이후는 전부 Edit |

---

## 9. 계측 — 최종 형식

### 9.1 엄폐 오버레이 2줄 (`SoldierLab.Debug.Cover 1`)
```
exp 0.50 st 0.50 hide+fight | HERE f0.00 o0.00 d0.00 s0.12 =0.12 | eyes 3 act 1.00 (Enemy_A2 45m)
#127 best f0.00 r0.27 o0.00 d0.00 =0.27 @5m cand 19 (fan 9) -> stay +m0.30 dwell 4.1s  (moving)
```
1줄 = 지금 자리(항별 f 싸움 / o 목표 / d 기억 / s 제압), 눈 수·활동도·가장 가까운 위협. 2줄 = 마지막 완료 스윕(래치): best 항별, 거리, 후보 수(부채꼴 수), 결정 `stay|MOVE|MOVING|discard`, 여유, 머무름 경과. 부채꼴 후보는 주황 점, 링은 회색 점, 눈은 빨간 점, 진행 중 best 파란 구, 마지막 best 노란(이동)/회색(정지) 구.

### 9.2 교전 오버레이 2줄 (`SoldierLab.Debug.Engagement 1`)
```
AIMED  over/open  know 120 vs weapon 340  recoil 0.54  ammo 17/30
tgt Enemy_A2  aimErr 0.0  c 0.99  switches 0 (-1.0s ago)  acct 0.35 PEEK
```
`acct` = 노출 계정, `PEEK` / `PEEK(look)`(관찰 회차) / `hidden`. 사선은 **실제 총구 소켓**에서 그린다.

### 9.3 로그 (카테고리 `LogSoldierAI`)
```
SoldierLab.Debug.Cover.Log 1
[Cover] <name> sweep N t= eyes= act= nearest=<name>@cm | HERE f o d s =tot | best f r o d =tot @(dx,dy) cm tried N fan M snapped K | stay|MOVE|MOVING|DISCARD margin X (dwell)

SoldierLab.Debug.Engagement.Log 1          ← 의도가 바뀔 때만 1줄
[Engage] <name> t= <from> -> <to> tgt= dist= know= spread= err= c= age= | believed worth aperture(ap/posture) onTarget(aimErr) reloading ammo supp stance lean peek obs acct act switches

SoldierLab.Debug.Danger 1                  ← 진영 위험 지도 셀 (테두리 = 길 위험, 점 = 자리 위험)
[SoldierIdentity] <name> heights measured: chest stand / crouch, eye (minimum wall that is cover)
```

### 9.4 읽는 법
- 이동을 안 한다 → `cand 0`이면 R1류(예산), `best`가 HERE보다 여유 안에 있으면 저울, `discard` 연속이면 `StallSpeedCms`.
- 안 쏜다 → `[Engage]`의 `-> hold` 줄에서 `believed/worth/aperture/onTarget/reloading/ammo` 중 0인 것. `blocked`가 길면 그 자리는 총구가 못 넘는 담(fight probe와 어긋남) 또는 `hidden` 사이 대기(정상 — 사이클).
- 리듬 → `acct`가 0.2↔0.8 사이를 오가고 `PEEK`/`hidden`이 교대하는가. 제압 오르면 `PEEK` 구간이 짧아지는가.

---

## 10. 튜닝값 — 이번 라운드 신설·변경 (전부 [C])

### 10.1 `USoldierCoverComponent`

| 값 | 기본 | 라운드 |
|---|---|---|
| `MaxTracesPerTick` | **36** (← 19 ← 6) + **하한 = 후보 1개** | R1·R2 |
| `bFanCandidates` / `FanRadiusCm` / `FanRays` / `CandidateMergeCm` | **true / 2400 / 24 (← 32) / 150** (신설) | R2·R4 |
| `RingCandidateCount` (← `CandidateCount`) | 12 | R2 |
| `RouteSampleSpacingCm` / `MaxRouteSamples` (← `RouteSamples 3`) | **400 / 8** | R2 |
| `ThreatEyeSmoothingSeconds` | **1.0** (신설) | R2 |
| `NoFiringPositionCost` | **0.9** (← 0.5) | R2 |
| 경로 위험 span | **클램프 제거**(단위 `SearchRadiusCm 1200`) | R4 |
| `FiringRecentSeconds` / `FiringFadeSeconds` / `QuietEyeWeight` | **3 / 5 / 0.3** (신설) | R5 |
| 자리의 공터 기억(`DangerWeight` 항) | **눈 0일 때만** | R5 |
| 머무름(`MinDwellSeconds 3` / `DwellBreakMargin 1.0`) | `bCanHide`일 때만 적용 | R2 |
| fight probe | min(눈, 총구) + 린 좌·우 2점 | R2·R4·R6 |
| `RouteRiskWeight` / `ObjectiveWeight` / `MoveImprovementMargin` / `DangerWeight` / `DangerHalfLifeSeconds` / `SuppressionWeight` / `NoCoverCost` | 0.6 / 2.0 / 0.3 / 0.8 / 30 / 0.5 / 1.0 (변경 없음) | |

### 10.2 `USoldierEngagementComponent`

| 값 | 기본 | 라운드 |
|---|---|---|
| `SuppressiveRadiusCm` | 500 — **뜻이 좁아짐**(총 산포만) | R3 |
| `SuppressiveKnowledgeRadiusCm` | **1000** (신설) | R3 |
| `ObserveAfterSeconds` / `ObserveSuppressionThreshold` | **1.5 / 0.3** (신설) | R3 |
| `ExposureRiseSeconds` / `ExposureFallSeconds` | **1.5 / 2.0** (신설) | R6 |
| `PeekStartBelow` / `PeekStopAbove` | **0.2 / 0.8** (신설) | R6 |
| `PeekQuietActivity` / `PeekFiringRecentSeconds` / `PeekFiringFadeSeconds` | **0.3 / 3 / 5** (신설) | R6 |
| `LeanAxisVariable` / `BlindFireHVariable` / `BlindFireVVariable` | `LeanCurrent` / `BlindFireH` / `BlindFireV` (BP 변수명) | R6 |
| `TargetSwitchCertaintyMargin` / `TargetSwitchDistanceRatio` | 0.3 / 0.6 (전편) | |

### 10.3 `USoldierIdentityComponent`

| 값 | 기본 | 라운드 |
|---|---|---|
| `StandChestHeightCm` / `CrouchChestHeightCm` / `bMeasureChestHeights` | 135 / 80 / true — 측정 전 초기값, 웅크림은 기립×(80/135)로 선추정 | R2 |
| `EyeHeightCm` | 160 — 측정 전 초기값 | R2 |
| `MuzzleSocket` | **"Muzzle"** (신설) | R6 |
| 총구 오프셋 7종 / `bLearnMuzzleOffsets` | 6.4 표 / true (신설) | R6 |

### 10.4 `FSoldierEnemyRecord`

`LastGunshotTimeSeconds` (신설, 기본 −1 = 없음). R5.

---

## 11. 변경 파일 (Perforce `user4_DESKTOP-81S78B2_4340` 체크아웃, **미제출**)

```
Source/SoldierLab/AI/SoldierDangerMap.{h,cpp}     (전편 신규) + GetRouteDangerSplit
Source/SoldierLab/AI/SoldierCover.{h,cpp}         재작성 2회 — 부채꼴 후보 · 활동도 · 예산 하한 · 린 probe
Source/SoldierLab/AI/SoldierEngagement.{h,cpp}    재작성 — 가치 게이트 분리 · 노출 회계 · PlanAperture · 실제 총구 · 리플렉션 축 · 전이 로그
Source/SoldierLab/AI/SoldierIdentity.{h,cpp}      높이·눈 측정 · 총구 소켓 · 포즈별 오프셋 학습 · 예측 총구
Source/SoldierLab/AI/SoldierPerception.{h,cpp}    LastGunshotTimeSeconds
Source/SoldierLab/AI/SoldierSuppression.{h,cpp}   (전편)
```
Camera/·Observer/·Pose/ 는 이번 범위 밖(건드리지 않음). BP 변경 없음.

---

## 12. 판정 기준 — 다음 실측 [C]

| 대상 | 통과 | 실패 시 |
|---|---|---|
| 낮은 담 뒤 병사 | 교전 2줄 `hidden → PEEK → hidden`이 반복, `acct`가 0.2↔0.8 | 안 나오면 `[Engage]` 전이 로그의 `peek/acct/act` |
| 제압 아래서 | `PEEK` 구간이 짧아지고 `hidden`이 길어짐; 0.45 넘으면 `over/blind` 또는 `right/left/blind` | 안 짧아지면 상승 속도(`ExposureRiseSeconds`)·활동도 |
| 코너 | Cover에서 코너 후보가 `f0.00`, 교전에서 `right/lean`·`left/lean` + Q/E 린 포즈 | BLIND로 남으면 fight probe 린 오프셋(`GetLeanReachCm`)·학습 여부 |
| 총구 학습 | 린/블라인드 포즈 뒤 예측 총구가 실제와 일치(포즈 도달 후 `bPlanFailed`가 드물다) | 자주 실패하면 축 변수명·`ObservePose` 조건 |
| 소강 | 총성 3 s 이상 멎은 구간(`act 0.3`)에 `MOVE`가 나오는가 — 로그에서 `act`와 `MOVE` 상관 | 안 나오면 저울(`QuietEyeWeight`·여유) |
| 겹치는 자리 | 같은 후보로 두 명 이상 MOVE하는 빈도 — 예약이 없으므로 **생길 것** → [W51] | |
| 노는 병사 | `[Engage]`에서 `hold`가 길게 유지되는 병사의 게이트 0 항목 → 원인 확정(사용자 요구) | |

---

## 13. 이 라운드가 하지 않은 것 → `OPEN_ITEMS.md`

| 항목 | ID |
|---|---|
| 위험 지도·머무름·활동도·노출 회계·총구 오프셋 기본값 **전부 실측 필요** | [C-102] [C-103] [C-104] [C-105] [C-107] |
| 레벨 벽 높이 갱신 실측 | [C-106] |
| **엄폐 자리 예약**(겹침, 특히 수비수 — `MASKED` 8~11 s의 원인) | [W51] |
| **분대 통신·화망**(사각 없는 위치로 제압, 엄호/이동 분담) | [W52] |
| **사망/대가** — 없으니 얼굴을 맞댄다 | [W18] 재강조 |
| "노는 병사" 원인 확정 | [W53] |
