# 위험 지도와 자리 지키기 — 전투 AI의 구조적 결핍 5건 보완

2026-09-14 / 완료(빌드·실측됨 — **값 다수가 후속에서 바뀜, 14절 정정 참고**) → 후속은 `ai/2026-09-15_exposure_cycle_and_muzzle_learning.md` / "위험"이 *지금 한 명의 눈에 보이느냐*로만 정의돼 있던 것을 **땅의 기억(위험 지도) · 모든 적의 눈 · 그림자로 미끄러지는 후보 · 머무름 · 표적 잠금**으로 넓혔다. 사용자 관측 ①공격수 저돌 ②수비수 안절부절 ③꼬리물기는 서로 다른 버그가 아니라 **한 가지 결핍**의 세 얼굴이었다.

`ai/2026-09-14_cover_frame_fix_and_observer.md`([C-95] 기준면 수정)의 후속. 그 문서가 "빌드됐으나 정착 여부 미확인"으로 남긴 것을 실측하려다, **실측 이전에 코드에서 읽히는 구조적 결핍**이 먼저 나와 이번 라운드가 됐다.
관련 원본: `ai/2026-09-13_engagement_and_cover.md`(방아쇠·엄폐 기하) · `ai/2026-09-13_objective_and_position_cost.md`(세 비용) · `ai/2026-09-14_exposure_ladder_and_corrections.md`(값 이력).
원칙: P10(계측 먼저) · P44(과도구간) · P6(튜닝값은 데이터) · P89(규칙이 아니라 선호) · P103(높이는 발에서).

> ⚠ **이 문서의 거동 서술은 전부 [B]다.** 코드는 읽고 썼지만(그 부분은 [A]), **한 번도 돌려 보지 않았다.** 11절의 판정 기준으로 실측한 뒤 정정 절을 붙인다.

---

## 0. 한 장 요약

| 무엇 | 어디 | 상태 |
|---|---|---|
| **위험 지도** — 진영별 땅의 기억 (보였던 곳 · 엄폐 없던 곳 · 총알 지나간 곳) | `AI/SoldierDangerMap.{h,cpp}` **신규** `USoldierDangerMapSubsystem` | [A] 코드 · [C] 거동 |
| **모든 눈** — 엄폐 판정을 primary 한 명이 아니라 기억 속 적 전원(가까운 3명)에게 | `SoldierCover::GatherThreatEyes / EvaluatePosition` | [A] · [C] |
| **그림자로 밀어 넣기** — 링 후보를 첫 트레이스가 맞힌 면 바로 뒤로 | `SoldierCover::SnapIntoShadow` | [A] · [C] |
| **경로 표본을 땅에 투영** (능선 문제) | `SoldierCover::EvaluateRoute` · `ProjectToGround` | [A] · [C] |
| **비용식 확장** — 위험(기억) 항 + 제압 항(HERE만), 항별 보존 | `FSoldierPositionCost` · `ScorePosition` | [A] · [C] |
| **머무름 · 결정 규칙** — 도착 후 3초 dwell, 이동 중 훑은 스윕은 폐기 | `SoldierCover::FinishSweep` | [A] · [C] |
| **표적 잠금** — 뚜렷이 나은 적이 나올 때만 바꾼다 | `SoldierEngagement::SelectTarget` | [A] · [C] |
| **가슴 높이 자동 측정** — 시야 판정과 같은 자 | `SoldierIdentity::ObserveStance` | [A] · [C] |
| **계측** — 오버레이 2줄(항별) · 교전 표적/오차/스위치 · 스윕 로그 · 위험 셀 그리기 | `Cover` / `Engagement` 오버레이 · `LogSoldierAI` | [A] |
| 대가(사망) · 협동 | — | **범위 밖** (사용자 결정) |

**빌드 0회. 실측 0회.** 사용자가 빌드한 뒤 11절로 본다.

---

## 1. 무엇이 빠져 있었나 — 코드와 레벨에서 읽은 것

### 1.1 사용자 관측 (2026-09-14) [A · 관측됨]

① 공격수가 엄폐 없이 밀고 들어온다 ② 수비수가 안절부절못하고 자리를 옮긴다 ③ 둘이 꼬리를 문다(공격수가 오면 수비수가 흔들리고, 수비수가 흔들리면 공격수가 더 온다).

### 1.2 코드에는 "단계"가 없다 [A]

문서는 인지 → 교전 → 엄폐 → 목표 층으로 정리돼 있지만 **코드에는 '엄폐 중 / 사격 중 / 전진 / 후퇴' 같은 상태가 없다.** 병사가 매 틱 하는 일은 둘뿐이다 — (가) 기억 속 적 한 명을 골라 조준하고 조건이 맞으면 쏜다, (나) 주변 12 m / 5.4 m 원 위의 점 12개를 프레임당 하나씩(12틱 ≈ 0.2 s에 한 바퀴 = **스윕**) 점수 매겨, 지금 자리보다 0.3점 이상 좋으면 걸어간다. "돌진"이라는 판단은 어디에도 없고, 덧셈 결과가 그렇게 보일 뿐이다. → **튜닝할 자리도 화면에서 읽을 자리도 없었다.** 이번 라운드도 상태를 만들지 않았다(P89) — 대신 **빠진 항을 채웠다**(P72와 같은 방향).

### 1.3 구조적 결핍 목록

번호는 사용자에게 보고한 목록 그대로다. 4·9는 사용자 결정으로 이번 범위 밖.

| # | 빠진 것 | 코드/레벨 근거 [A] | 귀결 [B] |
|---|---|---|---|
| **1** | **위험 구역 개념** | `SoldierCover.cpp` `EvaluatePosition`: 위협 기록이 없으면 `bCanHide=true`, 경로 위험 0. 기록은 시야 20 s · 총성 8 s 반감기로 사라지고, 사라지는 순간 맵 전체가 안전. "어디서 보였다 / 총알이 지나갔다"를 **땅에** 기억하는 자리가 없다 | 시야가 끊기면 방금 총 맞던 공터로 걸어 나감 |
| **2** | **뚫린 공간 돌파가 싸다** | 경로 위험 = `0.6 × 보인 비율 × (거리/12 m)` → **최대 0.6**. 공격수의 목표 항은 `2.0 × d/3000` → 12 m 전진 = **−0.8**. 즉 완전히 보이는 공터 12 m를 뛰는 것이 산술상 이득(`0.6 − 0.8 + 0.3 = 0.1`, 경로 표본 3개 중 하나만 가려져도 음수). 경로는 직선 3점, 보는 적의 수·거리·사격 여부 무관 | 광장을 그대로 가로지름 |
| **3** | **엄폐물을 찾는 눈** | 후보 = 고정 반지름 12점, 간격 바깥 링 **6.3 m** / 안쪽 **2.8 m**. 레벨 엄폐물 폭 2~4 m | 대부분의 벽·상자는 한 번도 후보가 된 적이 없음 → 후보가 전부 공터 → 목표에 가까운 공터가 이김 |
| **5** | **후퇴** | 공격 목표 항은 앞으로만 당김. 수비는 반경 안 전부 0이라 "물러날 선"이 없음 | 밀리면 흔들리기만 함 |
| **6** | **위협이 "가장 최근에 본 한 명"으로 뭉개짐** | `GetPrimaryContact`는 certainty 최대. `FuseRecords`가 융합 시 `ObservedTimeSeconds = Now`, 시야 certainty = 1.0으로 **재도장**하므로 certainty 순서 = 최근 트레이스 순서. Sight는 틱당 3발 라운드로빈 → 적 3명이 보이면 **primary가 틱마다 바뀔 수 있다.** Engagement의 `AimPoint`와 Cover의 `ThreatEye`가 둘 다 이것을 읽었다 | 총이 240 °/s로 돌기만 하고(`traversing`) 안 쏨 · 스윕마다 위협 지점이 다른 적 |
| **7** | **머무름(commitment)** | 도착 즉시 재비교. 스윕 도중 `Here`와 링 중심이 매 틱 갱신되므로 **걸어가며 훑은 12점은 서로 다른 위치에서 찍힌 것** → 도착 직후 비교가 어긋남 | 수비수가 도착하자마자 또 출발 |
| **8** | **판정 높이가 상수** | 몸 표본 발 위 135 / 107.5 / 80 cm(`StandChestHeightCm`/`CrouchChestHeightCm` 상수). 레벨 실측(MCP, 2026-09-14): `Low_Def_A/B` 꼭대기 **74 cm**(최저 표본 80 아래 → 엄폐 아님), `Med_Def` **135**(최상 표본과 같은 높이 → `bCanFight`가 mm로 뒤집힘), `Crate_Plaza` 90, `Crate_Plaza2/3` 110, `Low_Plaza_A/B` 100, `Low_Atk` 100, `Low_Atk2` 60, `Med_Atk` 170, `Cube` 84 | 수비 구역의 "낮은 담" 둘이 실제론 공터 취급 |

레벨 배치 실측 [A]: 아군 `Ally_A/B` (−2300, ±500) 목표 옆, `Ally_B2` (−537, 218) · `Ally_B3` (−486, −801) 전방(목표 밖 → obj 0.67로 뒤로 당김), 적 `Enemy_A/A2/A3` x = 4000, 목표 `Objective_AllyBase` (−2422, 0). 시야 60 m라 **A/B ↔ 적은 서로 안 보이고 B2/B3만 교전**한다 — ③의 꼬리물기는 B2/B3 쌍이 만들 가능성이 크다 [B].

> ★ 공통 기구: **위협이 한 점이고, 그 점이 스윕(0.2 s)보다 빨리 움직인다.** ①은 그 점이 지금 안 보이면 공터가 싸지는 것, ②·⑥·⑦은 그 점이 튀는 것, ③은 둘의 결합이다.

---

## 2. 위험 지도 — `USoldierDangerMapSubsystem` [A · 코드]

`AI/SoldierDangerMap.{h,cpp}`, `UWorldSubsystem`. **땅의 기억**이다. 지금까지 기억은 적에 대해서만 있었다.

```
진영별  TMap<FIntPoint, FCell>      Cells[3]   (Friendly / Hostile / Neutral — Neutral은 아무도 안 씀)
CellSizeCm 200                       (상수 — 해상도이지 튜닝값이 아니다)
FCell   SeenStandingTime             선 몸이 여기서 보였다      ← EvaluateRoute, 표본이 보이면
        OpenGroundTime               여기엔 숨을 것이 없었다     ← EvaluatePosition, !bCanHide
        FiredUponTime                여기서 총알이 스쳐 갔다     ← USoldierSuppressionComponent::ApplyNearMiss
```

읽기(반감기는 **읽는 쪽**이 넘긴다 — `SoldierCover.DangerHalfLifeSeconds 30`):

```
GetPositionDanger  = decay(OpenGroundTime)                                  자리에 쓴다
GetRouteDanger     = decay(max(SeenStanding, OpenGround, FiredUpon))        길에 쓴다
decay(t)           = 0.5 ^ ((Now − t) / HalfLife)
```

**자리와 길이 다른 스탬프를 읽는 이유**: 낮은 담 위로 선 몸이 보인 것, 그 담에 총알이 박힌 것은 **담이 하는 일**이다. 그것을 자리 비용에 물리면 병사는 좋은 담을 쓴 순간 그 담을 떠난다. 자리에 물리는 것은 "여기엔 어느 높이에서도 가릴 것이 없었다"뿐이다. 반면 **건너감은 서서 하는 것**이므로 길에는 셋 다 물린다.

**새 트레이스는 0발이다.** 엄폐 판정이 이미 매 틱 수십 개의 "이 점은 적 눈에서 보인다/안 보인다"를 만들고 버리고 있었다. 그것을 적을 뿐이다. 근접탄은 제압 컴포넌트가 이미 받던 이벤트다.

무전 공유는 하지 않았다(13절 후보).

---

## 3. 모든 눈 — 한 명이 아니라 전원 [A · 코드]

`SoldierCover::GatherThreatEyes`:

```
Perception 기록 전부  →  certainty ≥ MinThreatCertaintyForCover (0.05)  →  가까운 순  →  MaxThreatEyes (3)
눈 위치 = 기록 예측위치 + ThreatEyeAboveContactCm (20)
```

`EvaluatePosition(Foot, Eyes[])`: 높이 표본 i는 **전원에게 막혀야** hidden. 하나라도 보면 그 높이는 seen이고 남은 눈은 트레이스하지 않는다(조기 종료). `FirstHidden` = 처음으로 전원에게 막힌 높이.

```
bCanHide  = FirstHidden < Samples      어느 높이에선가 전원에게서 숨는다
bCanFight = FirstHidden > 0            최상 표본을 하나라도 본다 = 그 적을 쏠 수 있다
```

눈 목록은 **한 스윕 동안 동결**(`SweepEyes`)한다 — 전 라운드의 "한 바퀴 스냅샷"과 같은 이유. 이제 Cover에는 "primary 한 명 고르기" 자체가 없다.

`MinThreatCertaintyForCover 0.05`가 사격 문턱 `MinCertaintyToEngage 0.25`보다 훨씬 낮은 것은 의도다 — **쏠 만큼 확신은 없어도 그 앞에 서 있고 싶지는 않은** 적이 있다.

---

## 4. 그림자로 밀어 넣기 — `SnapIntoShadow` [A · 코드]

엄폐 판정("적 눈에서 이 몸으로 선을 긋고 막히나")은 **모양을 전혀 안 본다.** 큐브·바위·원기둥·능선 어느 것에도 같다. 깨지는 것은 **후보를 어디서 뽑느냐**뿐이었다(1.3 #3).

```
가장 가까운 눈  →  후보(기립 높이)  트레이스 1발
  막히지 않음                          → 그대로 (그림자가 없다)
  충돌점이 후보에서 ShadowStepCm 안     → 그대로 (이미 바로 뒤다)
  아니면  Behind = 충돌점 + 진행방향(수평) × ShadowStepCm (60)
          |Behind − 발| > SearchRadiusCm × 1.1   → 기각 (적 옆 건물 벽은 내 후보가 아니다)
          네브메시 투영 실패                      → 기각
          투영점이 충돌점보다 눈에 가까움          → 기각 (두꺼운 건물의 **적 쪽** 면으로 투영된 것)
          채택
```

12방향 × 2링이므로 **내 주변 12 m 안의 모든 장애물 그림자가 30° 간격으로 훑어진다.** 링 위의 점이 상자를 못 맞혀도 **적과 그 점 사이에** 상자가 있기만 하면 후보가 상자 뒤로 미끄러진다. 충돌면이 땅(능선)이면 능선 뒤가 후보가 된다.

**경사지 보정** — `EvaluateRoute`의 표본을 `ProjectToGround`(네브메시 투영, 반경 300)한 뒤 기립 높이를 더한다. 두 발 위치를 직선으로 이으면 능선을 넘는 길의 중간 점이 **땅속**에 박혀 어디서도 "막힘"으로 읽혔다 — 가장 노출된 길이 가장 안전하게 매겨지는 오류.

한계 [B]: 후보 링 자체는 그대로다("2 m 앞의 완벽한 자리"는 눈과 그 점 사이에 있을 때만 잡힌다). [W21] **절반 해결**로 본다. 바위처럼 위가 울퉁불퉁하면 `HeightSamples 3`이 거칠 수 있다 — 5로 올리면 되고 구조는 같다.

---

## 5. 비용식 [A · 코드]

```
Cost(자리) = Fighting                      !hide 1.0  +  !fight 0.5
           + RouteRiskWeight  0.6 × Route   표본별 max(live 보임, 기억된 위험) 의 평균 × 거리 스케일
           + ObjectiveWeight  2.0 × Obj
           + DangerWeight     0.8 × Danger  GetPositionDanger (OpenGround 기억)
           + SuppressionWeight 0.5 × Suppression   ← HERE 에만 (후보는 총 맞고 있지 않다)
```

`FSoldierPositionCost`가 **항별로 보존**된다 — 오버레이와 로그가 "어느 항이 이겼나"를 말할 수 있어야 하기 때문이다(P7·P10). 옛 코드는 총합 하나만 남겼다.

주의할 귀결 [B]: 라이브로 평가된 공터 후보는 `EvaluatePosition`이 그 자리에서 `MarkOpenGround`를 찍으므로 **Fighting 1.0 + Danger 0.8 = 1.8**로 매겨진다. HERE도 매 틱 찍히므로 같은 1.8. 즉 공터끼리는 공정하고, 공터 → 기억이 없는 공터 이동은 이득이 없다.

---

## 6. 머무름과 결정 — `FinishSweep` [A · 코드]

```
매 틱      경로추종 Moving → 아님 으로 바뀐 순간 ArrivedTimeSeconds = Now
           속도 > StallSpeedCms (20) 이면 bSweepWalkedWhileMoving = true

스윕 끝    bAlreadyGoing (Moving ∧ 진척)            → 유지, MOVING
           bSweepWalkedWhileMoving                   → DISCARD (이동 중 훑은 링은 비교가 아니다)
           margin = dwell 중 ? DwellBreakMargin 1.0 : MoveImprovementMargin 0.3
                    dwell 중 = Now − Arrived < MinDwellSeconds 3
           best + margin < HERE                      → MoveToLocation, MOVE
           아니면                                    → stay
```

dwell 동안 결정을 **막지 않고 문턱을 올린다** — 도착해 보니 공터(NoCover 1.0 + Danger 0.8)면 여전히 떠날 수 있어야 한다.

**후퇴는 별도 로직이 없다** [B]: 현 자리가 위험 지도에서 뜨겁고 총을 맞고 있으면 HERE가 비싸지고, 후보는 360°에 있으니 뒤쪽 엄폐가 이긴다. 안 나오면 그때 항을 추가한다.

---

## 7. 표적 잠금 — `SoldierEngagement::SelectTarget` [A · 코드]

```
잠긴 표적 재발견   LockedEnemy(약참조) 있으면 GetRecordFor
                   없으면(총성 기록) LockedLocation 으로 FindRecordNear  ← Perception 신규 헬퍼
해제               certainty < MinCertaintyToEngage (0.25)
교체               후보 certainty ≥ 잠긴 것 + TargetSwitchCertaintyMargin (0.3)
                   또는 후보 거리 ≤ 잠긴 것 거리 × TargetSwitchDistanceRatio (0.6)
계측               TargetSwitches, LastTargetSwitchTime, DebugAimErrorDeg, DebugTargetName
```

1.3 #6의 "틱마다 바뀌는 primary"를 **재도장 한 프레임으로는 넘을 수 없는 여유**로 막는다. 이것은 규칙이 아니라 히스테리시스다 — 뚜렷이 나은 적은 여전히 이긴다.

---

## 8. 가슴 높이 자동 측정 — `SoldierIdentity::ObserveStance` [A · 코드]

적의 시야 판정은 `spine_03` **한 점**을 본다. 그러니 엄폐 판정도 "서 있을 때 spine_03 높이"와 "웅크렸을 때 spine_03 높이"를 써야 두 판정이 같은 자를 쓴다. 상수 135/80은 메시가 바뀌면 틀린다(P77).

```
USoldierIdentityComponent
  StandChestHeightCm 135 · CrouchChestHeightCm 80     초기값
  bMeasureChestHeights true
  ObserveStance(ActualStance)    ← Engagement::SetActualStance 가 매 틱 호출 (BP 접합이 이미 그 값을 넘긴다)
     소켓이 실재할 때만.  Stance ≤ 0.05 → Stand,  ≥ 0.95 → Crouch 에 표본
     표본 = spine_03.Z − 발.Z.   첫 표본은 대체, 이후 0.05 스무딩
     둘 다 재면 LogSoldierAI 에 1줄:  "… stand X cm, crouch Y cm (minimum wall that is cover: Y cm)"
  GetChestHeightCm(Stance)
```

`SoldierCover`의 `StandChestHeightCm` / `CrouchChestHeightCm` UPROPERTY는 **삭제**됐다(두 소스 금지). Cover와 `SnapIntoShadow`는 Identity의 값을 읽는다. Identity가 없으면 135/80 폴백.

**설계 함의**: 웅크린 spine_03 높이(재면 ~80 cm 근처로 예상 [C])가 **이 애니메이션 세트에서 엄폐가 되는 최소 담 높이**다. 엎드리기가 없으니 `Low_Def_A/B`(74 cm)가 엄폐가 아닌 것은 **맞는 판정**이고, 고칠 것은 판정이 아니라 레벨(또는 엎드리기)이다. 이 숫자를 `assets/2026-09-14_designer_guide_draft.md`에 넣을 것(실측 후).

---

## 9. 계측 [A · 코드]

### 9.1 엄폐 오버레이 — 2줄 (`SoldierLab.Debug.Cover 1`)

```
exp 0.50 st 0.50 hide+fight | HERE f0.00 o0.00 d0.00 s0.00 =0.00 | eyes 2 (Enemy_A 31m)
#37 best f0.00 r0.18 o0.00 d0.00 =0.18 @7m snap 4/12 -> stay +m0.30 dwell 1.2s  (moving)
```

- 1줄 = **지금 이 자리**, 항별(f Fighting · o Objective · d Danger · s Suppression) + 동결된 눈의 수와 가장 가까운 것.
- 2줄 = **마지막으로 끝난 스윕의 결정**(`FSoldierSweepReport` 래치): best의 항별(f r o d) · 거리 · 그림자 스냅 수/후보 수 · 결정(`stay` / `MOVE` / `MOVING` = 이동 유지 / `discard` = 이동 중 훑음) · 적용된 margin · 도착 후 경과. 옛 오버레이의 `best`는 진행 중 스윕 값이라 결정 직후 `-1.00`으로 리셋돼 정속에선 읽을 수 없었다.
- 그림: 진행 중 best(파랑 구) · 마지막 스윕 best(노랑 = MOVE 발행 / 회색 = 아님) · 눈(빨강 점) · 몸 표본 선(막힘 짙은 회색 / 안 막힘 옅은 회색) · 경로 표본 점(위험이 높을수록 붉게).

### 9.2 교전 오버레이 — 2줄째 추가 (`SoldierLab.Debug.Engagement 1`)

```
AIMED  over/open  know 120 vs weapon 340  recoil 0.54  ammo 17/30
tgt Enemy_A2  aimErr 1.3  c 0.98  switches 3 (12.4s ago)
```

### 9.3 스윕 로그 (`SoldierLab.Debug.Cover.Log 1`, 카테고리 `LogSoldierAI`, MCP `LogsToolset`으로 읽힌다)

```
[Cover] Ally_B2 sweep 37 t=12.34 eyes=2 nearest=Enemy_A@3120 | HERE f1.00 o0.00 d0.80 s0.12 =1.92 | best f0.00 r0.18 o0.00 d0.00 =0.18 @(+640,-310) 712cm tried 12 snapped 4 | MOVE margin 0.30
```

한 병사 쌍의 **시간순 표**(위협 → 비용 → 이동 발행)는 이 줄로 만든다 — ③의 되먹임 고리를 확정하는 재료.

### 9.4 위험 지도 (`SoldierLab.Debug.Danger 1`)

그려지는 병사(`AI.Filter` / `AI.Self` 규약 그대로) 주변 `SearchRadiusCm × 1.5` 안의 셀. 상자 = 길 위험(굵기·투명도), 가운데 점 = 자리 위험. 색은 주황 — 앎이므로 채도가 있지만 적의 세 출처 색은 아니다(P70).

### 9.5 갈래 읽는 법 (exposure_ladder 12절의 ⓐ/ⓑ/ⓒ를 이 줄로)

| 화면 | 갈래 | 볼 곳 |
|---|---|---|
| 1줄 `HERE =0.00 hide+fight`인데 2줄이 `MOVE` | ⓐ 이동 발행 | 2줄의 margin · dwell · `discard`가 연속되는가(StallSpeed 오판) |
| 1줄 `hide`↔`OPEN` 또는 `+fight`↔`+BLIND`가 스윕마다 뒤집힘 | ⓑ 위협 추정 | `eyes N (이름)`이 바뀌는가(→ 이제는 전원이라 드물어야 함) · 교전 `switches` · `Med_Def` 같은 경계 높이 |
| 2줄 best 총합이 정말 HERE − margin 아래 | ⓒ 가중치 | best의 f/r/o/d 중 **어느 항이 HERE보다 작은가** — 한 줄로 증명된다 |
| 공격수 2줄 best가 `f1.00 … o<HERE.o>` 로 이김 | 공터→공터 전진 | `d`가 붙어 있는가(기억) · `snap`이 0/12인가(후보가 엄폐를 못 찾음 → 3번) |

---

## 10. 튜닝값 — 전부 [C]

**하나도 재지 않았다.** 옛값은 `(← 옛값)`.

### 10.1 `USoldierCoverComponent`

| 값 | 기본 | 뜻 |
|---|---|---|
| `MaxTracesPerTick` | **19** (← 6) | 눈 3 × (높이 3 + 경로 3) + 스냅 1 = 후보 1개/틱. 옛 6은 눈 1개 기준의 같은 뜻 |
| `MaxThreatEyes` | **3** (신설) | 가까운 순 |
| `MinThreatCertaintyForCover` | **0.05** (신설) | 사격 문턱 0.25보다 낮게 |
| `bSnapCandidatesIntoShadow` | **true** (신설) | |
| `ShadowStepCm` | **60** (신설) | 충돌면 뒤로 |
| `DangerWeight` | **0.8** (신설) | |
| `DangerHalfLifeSeconds` | **30** (신설) | |
| `SuppressionWeight` | **0.5** (신설) | HERE만 |
| `MinDwellSeconds` | **3** (신설) | |
| `DwellBreakMargin` | **1.0** (신설) | |
| `RouteRiskWeight` / `ObjectiveWeight` / `MoveImprovementMargin` | 0.6 / 2.0 / 0.3 (변경 없음) | ⚠ 1.3 #2의 `0.6 < 0.8` 저울은 **아직 안 맞췄다** — 항별 계측을 보고 정한다 |
| `NoCoverCost` / `NoFiringPositionCost` | 1.0 / 0.5 (변경 없음) | |
| `StallSpeedCms` / `bUseAvoidance` / `AvoidanceRadiusCm` | 20 / true / 300 (변경 없음) | |
| `SearchRadiusCm` / `CandidateCount` / `InnerRingFraction` / `HeightSamples` / `RouteSamples` / `ThreatEyeAboveContactCm` | 1200 / 12 / 0.45 / 3 / 3 / 20 (변경 없음) | |
| ~~`StandChestHeightCm` / `CrouchChestHeightCm`~~ | **삭제** → Identity | |

### 10.2 `USoldierEngagementComponent`

| 값 | 기본 |
|---|---|
| `TargetSwitchCertaintyMargin` | **0.3** (신설) |
| `TargetSwitchDistanceRatio` | **0.6** (신설) |

### 10.3 `USoldierIdentityComponent`

| 값 | 기본 |
|---|---|
| `StandChestHeightCm` / `CrouchChestHeightCm` | **135 / 80** (Cover에서 이관, 측정 전 초기값) |
| `bMeasureChestHeights` | **true** |

### 10.4 `USoldierDangerMapSubsystem`

`CellSizeCm 200` — **상수**(해상도). 반감기·가중치는 읽는 쪽(Cover)의 데이터다.

---

## 11. 판정 기준 (빌드 후) [C]

`SoldierLab.Debug.Cover 1 · Engagement 1 · Perception 2 · Danger 1 · AI.Filter Friendly` (공격수는 `Hostile`), `slomo 0.2`, 필요하면 `Cover.Log 1`로 시간순 표.

| 대상 | 통과 | 실패 시 볼 곳 |
|---|---|---|
| 수비수 (Ally_A/B, Med_Def·Bld_Def 근처) | 1줄 `HERE =0.00 hide+fight` 유지, 2줄 `stay` 연속, 30 s 동안 MOVE 발행 ≤ 1~2회 | 9.5 표 |
| 수비수 사격 | 교전 `switches` 증가율이 초당 0에 가깝고 `aimErr`가 3° 아래로 정착, `AIMED/suppress`가 나옴 | `switches`가 계속 오르면 7절 여유값 · `tgt`가 총성 기록이면 FindRecordNear 반경 |
| 공격수 (Enemy_*) | 2줄 best가 `f0.00`(엄폐)인 후보로만 MOVE, `snap`이 0/12가 아님, OPEN→OPEN 전진(best `f1.00`으로 이김)이 사라짐 | 남아 있으면 항별 값으로 저울(RouteRiskWeight·ApproachScale) 조정 — 이번엔 숫자를 보고 |
| 위험 지도 | 광장(`Crate_Plaza` 부근, x ≈ 0~2200)을 건넌 뒤 `Danger 1`에 셀이 남고 30 s에 걸쳐 옅어짐 | 안 남으면 `MarkSeenStanding`이 안 찍히는 것 — eyes 0인지 |
| 꼬리물기 ③ | 로그로 B2/B3 ↔ Enemy 쌍의 스윕 결정을 시간순으로 놓았을 때, 한쪽 MOVE가 상대 HERE의 `hide/fight` 뒤집힘으로 이어지는 사슬이 **끊어졌는가** | 안 끊기면 어느 항이 뒤집히는지(f? d?)로 갈래 |
| 가슴 높이 | LogSoldierAI에 측정 1줄, 값이 135/80 근처인가 | 크게 다르면 소켓·stance 축 전달 확인 |

**알려진 위험** [B]:
- `StallSpeedCms 20`이 RVO 회피의 미세 이동을 "이동"으로 읽으면 스윕이 **계속 `discard`** 되어 결정이 안 나온다. 2줄에 `discard`가 연속되면 임계를 올린다(값은 데이터).
- 트레이스가 병사당 최대 **약 3배**(눈 3개) 늘었다. 45명 규모는 어차피 미측정 → [C-83]에 얹힌다.
- `MinThreatCertaintyForCover 0.05`면 거의 잊은 적도 눈으로 센다 — 눈이 3개로 잘리므로 가까운 셋만. 먼 옛 기록이 가까운 셋을 밀어내는 일은 거리 정렬로 막힌다.
- 공터 후보에 Danger 0.8이 즉시 찍혀 1.8이 되는 것(5절)은 의도이나, `NoCoverCost`와 사실상 합쳐진 값이라 두 다이얼이 겹친다. 실측 후 한쪽으로 정리할 수 있다.

---

## 12. 변경 파일 (Perforce `user4_DESKTOP-81S78B2_4340` 체크아웃, **미제출**)

```
Source/SoldierLab/AI/SoldierDangerMap.{h,cpp}      신규 (p4 add)   USoldierDangerMapSubsystem · LogSoldierAI 정의
Source/SoldierLab/AI/SoldierCover.{h,cpp}          재작성          모든 눈 · 그림자 스냅 · 경로 투영 · 항별 비용 · dwell/discard · 오버레이 2줄 · 로그 · Danger 그리기
Source/SoldierLab/AI/SoldierEngagement.{h,cpp}     SelectTarget · ObserveStance 호출 · 오버레이 2줄째
Source/SoldierLab/AI/SoldierIdentity.{h,cpp}       가슴 높이 2값 + ObserveStance + GetChestHeightCm
Source/SoldierLab/AI/SoldierPerception.{h,cpp}     FindRecordNear
Source/SoldierLab/AI/SoldierSuppression.{h,cpp}    BeginPlay(Identity) · ApplyNearMiss → MarkFiredUpon
```

새 `UCLASS`(서브시스템)가 있으므로 **에디터를 닫고 빌드**해야 한다(P13). BP 쪽 변경 없음 — `BP_SoldierCharacter` Tick 접합은 그대로 `SetActualStance`를 넘기고 있고 그것이 측정의 입력이다. Cover 컴포넌트 템플릿에 옛 `StandChestHeightCm` 오버라이드가 저장돼 있었다면 조용히 버려진다(기본값이었으므로 문제 없음 [B]).

---

## 13. 이 라운드가 하지 않은 것 — 미해결 후보 (ID는 실측 후 `OPEN_ITEMS.md`에 부여)

| 후보 | 왜 지금 안 했나 |
|---|---|
| **위험 지도의 무전 공유** — 지금은 각 병사가 자기 진영 지도에 쓰고 진영 전체가 읽는다(이미 공유된 셈). "한 명이 본 것을 못 본 동료가 안다"는 것이 맞는지, 아니면 병사별로 갈라야 하는지 | 분대 층이 없어 판단 보류. 진영 공유가 지금 규모(4:3)에선 오히려 자연스럽다 [B] |
| **후보 링 자체의 대체** — 그림자 스냅은 "눈과 링 점 사이의 장애물"만 잡는다 | [W21] 절반 해결. 실측에서 `snap 0/12`가 반복되면 다음 |
| **대가(체력·사망)** | 사용자 결정으로 범위 밖. [W18] 그대로 |
| **협동(엄호·이동 분담·자리 중복 회피)** | 사용자 결정으로 범위 밖. `squad/drafts/` 그대로 |
| **후퇴 항** | 6절 — 위험/제압 항의 귀결로 나오는지 먼저 본다 |
| **저울 `0.6 < 0.8`** | 1.3 #2. 항별 계측을 보고 정한다 — 추측으로 바꾸지 않는다(P10) |

---

## 14. 정정 (2026-09-15) — 빌드·실측 뒤 후속 라운드가 이 문서에서 바꾼 것

빌드 후 로그를 읽고 여섯 라운드가 이어졌다. 전문은 **`ai/2026-09-15_exposure_cycle_and_muzzle_learning.md`**. 이 문서의 본문은 그대로 두고, 그 뒤에 달라진 자리만 적는다(옛값은 취소선).

| 절 | 이 문서 | 지금 (후속 R#) |
|---|---|---|
| 10.1 `MaxTracesPerTick` | ~~19~~ | **36** + **틱당 최소 후보 1개 하한** — 눈 3명이면 후보당 37발 > 36이라 **후보 0개·스윕 0회·전원 부동**이었다 (R1) |
| 10.1 `NoFiringPositionCost` | ~~0.5~~ | **0.9** — BLIND가 건너기(≤0.6)+여유(0.3)보다 싸서 건물 뒤에서 영영 놀았다 (R2) |
| 4절 그림자 스냅 | ~~후보 생성의 본체~~ | **부채꼴 후보**(`bFanCandidates`, `FanRadiusCm 2400`, `FanRays 24`, 눈 전부에서)가 본체, 링 12점은 눈 0일 때의 예비, 스냅은 링 점에만 보조 (R2·R4) |
| 5.2 경로 위험 span | ~~`clamp(L/1200, 0, 1)`~~ | **`L/1200` 클램프 제거** — 긴 도약이 미터당 싸지는 역전이 "한 명만 돌격"의 원인 (R4, P84) |
| 5절 자리의 `Danger` 항 | ~~항상~~ | **눈이 0일 때만** — 2 m 격자가 상자 뒤/옆을 같은 칸으로 묶어 엄폐를 벌했다 (R5) |
| 5절 경로 위험·"보였던 기억" | ~~보이면 1~~ | × **눈 활동도**(최근 3 s 총성 1.0 → 5 s에 걸쳐 0.3) — 소강 이용 (R5) |
| 8절 가슴 높이 | ~~135/80 근처일 것~~ | 실측 **기립 96~105 / 웅크림 50~57 cm**, 눈 140~150. 웅크림은 기립×(80/135)로 선추정 (R2) |
| 3절 `bCanFight` | ~~최상 가슴 표본이 보임~~ | **min(눈, 총구 140) 높이 + 린 좌·우 2점** — 가슴 기준으론 1 m 담이 전부 BLIND였고, 눈 기준으론 총구가 못 넘는 담을 골랐다 (R2·R4·R6) |
| 6절 머무름 | ~~도착 후 3 s 무조건~~ | **`bCanHide`일 때만** — 공터에 선 병사를 붙잡았다 (R2) |
| 5.2 경로 표본 | ~~`RouteSamples 3`~~ | 간격 400 cm, 최대 8 (R2) |
| 3절 눈 위치 | ~~기록 그대로~~ | 스무딩 1.0 s — 총성 지터로 HERE가 깜빡였다 (R2) |
| 7절 표적 잠금 | [B] | **[A] 동작 확인** — `switches 0`, `aimErr 0.0` |
| 11절 "알려진 위험" 첫째(StallSpeed discard 연속) | [B] | 관측되지 않았다 — `DISCARD`는 이동 중 정지 시에만 몇 회 (R2 로그) |
| 13절 후보 링 대체 | 미해결 | 부채꼴로 **해결** ([W21] 닫힘 판정은 실측 후) |
| 13절 저울 `0.6 < 0.8` | 미해결 | R4·R5로 형태가 바뀜(길이 비례·활동도). 값 자체는 여전히 [C] → [C-104] |

새 층(교전 쪽): 가치 게이트 분리(R3) · **노출 회계 사이클**(R6) · 실제 총구 소켓 · 포즈별 총구 오프셋 학습(R6) — 이 문서 범위 밖이므로 후속 문서 3·6절.
