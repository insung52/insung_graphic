# 교전과 엄폐 — 앎을 방아쇠와 발로 바꾸는 층

2026-09-13 / **구현 완료 · 수치 검증 완료** / 방아쇠는 "맞출 수 있나"가 아니라
**"내 앎이 내 무기보다 나쁜가"** 로 걸린다. 엄폐는 **시야 판정을 거꾸로 돌린 것**이고
볼륨도 마커도 저작 데이터도 없다.

관련: **[C-83]** · **[C-86]** · **[C-87]** · **[W18]~[W24]** / 원칙: **P66~P69**
관련 문서: `ai/2026-09-13_perception_stack.md` · `ai/2026-09-13_ai_bridge_and_scene.md` ·
**`ai/2026-09-13_objective_and_position_cost.md`**(★ 7절의 갱신분) ·
`IMPLEMENTED.md` 2.5f절(연속 stance 축) · `cover/drafts/` (미채택)

> ★★ **2026-09-14 — 이 문서에는 후속편이 있다**: `ai/2026-09-14_exposure_ladder_and_corrections.md`
> 그곳에서 바뀐 것 — **막힌 사선에 조리개를 찾고 사격 자세를 고른다**(3.1절의 확장) ·
> **반동 누적**([W26] 해결, 4절의 `EffectiveSpreadAtCm` 에 곱셈 항) ·
> **조준 선회 240 °/s** 와 새 의도 `Traversing`(1절의 열거형에 값이 하나 늘었다) ·
> **재장전 조건**(5절 — 오지 않는 소강상태를 기다리고 있었다) ·
> **7절 위치 선택의 첫 항이 노출에서 `FightingCost` 로** · 8절 튜닝값 다수.
> **여기 적힌 기구는 그대로 유효하고, 값과 사다리만 넓어졌다.**

> ★ **2026-09-13 갱신 — 7절(엄폐)의 *점수식*이 그 뒤에 넓어졌다.**
> 여기 적힌 **엄폐 기하**(시야를 거꾸로 돌린 판정 · 한 스윕이 주는 두 답)는 그대로 유효하다.
> 그러나 **"어느 자리를 고르는가"는 이제 노출 하나가 아니라 세 비용**(도착지 노출 · 경로
> 노출 · 땅의 값)이고, 8절 표의 `MaxTracesPerTick 4` · `MoveImprovementMargin 0.35` 는
> **옛 값**이다(→ **6** · **0.25**). 전문: **`ai/2026-09-13_objective_and_position_cost.md`**.
> 10절의 **[D10]**(목표 개념 자체가 없음)은 그 문서로 해소됐다 — 다만 **레벨에 목표 액터가
> 아직 한 개도 놓여 있지 않아** 셋째 항은 지금 항상 0이다 → **[W25]**.

> ⚠ **`cover/drafts/`의 EQS + Smart Object 초안은 여전히 미반영이다.** 여기 적힌 엄폐는
> **EQS도 Smart Object도 쓰지 않는다.** 설계 문서의 "Smart Object 예약 + EQS 스코어링"
> 결정([R2]·[Q34])은 **이 구현에 적용되지 않았다** — 예약이 필요해지는 분대 층에서 다시
> 만나게 될 문제다.

---

## 1. 교전 — `SoldierEngagement` [A]

```
ESoldierFireIntent   Hold / Suppressive / Aimed / Masked / Blocked
                     (→ 2026-09-14에 **Traversing** 추가 — 총이 아직 표적에 안 왔다)
```

컴포넌트는 **결정만 하고 행동하지 않는다.** 조준만 직접 건다(`AAIController::SetFocalPoint` —
조준은 컨트롤러의 일이다). 나머지는 전부 **게시**하고, 캐릭터 블루프린트가 적용한다.
그리고 블루프린트는 **자기만 볼 수 있는 두 가지**를 되돌려 준다 —
`SetActualStance(실제 도달한 자세)` · `SetWeaponState(탄/탄창/재장전중)`.
**각 사실이 그것을 진짜로 쥔 쪽에 남는다.**

---

## 2. ★ 방아쇠 — "맞출 수 있나"를 **묻지 않는다** (P66)

`WeaponSpreadDegrees = 3`. 그 무기로 "맞출 확률"을 물으면 **쓸모 있는 답이 없다.**

| 거리 | 탄착 반경 | 명중 확률 |
|---|---|---|
| 5m | 26cm | **100%** |
| 10m | 52cm | 33% |
| 20m | 105cm | 8% |
| 40m | 210cm | **2%** |
| 80m | 420cm | 0.5% |

*(표적 반경 30cm 가정. 탄착 반경 = 거리 × tan 3°)*

**완벽하게 위치를 아는 40m 표적이 2%다.** 명중확률 게이트는 **영영 안 쏘거나, 아무 뜻도
없을 만큼 낮게 잡히거나** 둘 중 하나다.

### 2.1 대신 묻는 질문

```
RadiusCm  ≤  SpreadCm × KnowledgeToSpreadRatio        →  Aimed
```

**앎(해상도 반경)이 무기(탄착 반경)보다 좁은가.**

- 반경이 탄착보다 **넓으면** — 더 조준해 봐야 사는 것이 없다
- 반경이 탄착보다 **좁으면** — 이제 한계는 무기이고, 기다려도 사격이 나아지지 않는다

**양변이 전부 시스템이 이미 생산하는 물리량이다.** 튜닝된 문턱이 존재하지 않는다.
`KnowledgeToSpreadRatio = 1.0` 은 문턱이 아니라 **비교의 배율**이고, 기본값이 1이라는 것은
"있는 그대로 비교한다"는 뜻이다.

### 2.2 제압 사격은 그 아래 칸이다

```
RadiusCm ≤ SuppressiveRadiusCm 500  AND  탄창잔량 > SuppressiveReserveFraction 0.4
    →  Suppressive
```

**맞출 기대는 없지만 탄이 충분히 가깝게 지나간다**는 것이 제압의 정의이고, 그 거리는
제압 컴포넌트가 쓰는 것과 같은 500cm다(`..._perception_stack.md` 7절).

### 2.3 믿음 게이트

```
Certainty ≥ MinCertaintyToEngage 0.25
```

**센티미터까지 특정된 접촉이라도 믿지 않을 수 있다.** 기록은 해상도와 별개로 확신이
썩는다(P62) — 이 게이트가 없으면 **텅 빈 출입구에 탄창을 비우는** 병사가 나온다.

---

## 3. 세 개의 거절 — **따로 묻는다** [A]

거절이 **어느 쪽이 아니라고 했는지 말할 수 있어야** 하므로 합치지 않았다.

### 3.1 `Blocked` — ★ 사선은 **총구에서** 쏜다

```
GetMuzzleLocation() = 액터위치 + Z( Lerp(StandMuzzleHeightCm 140,
                                        CrouchMuzzleHeightCm 95,
                                        ActualStance) − CapsuleHalfHeightCm )
```

**높이가 ACTUAL stance로 보간된다. 이것이 이 절의 요점이다.**

**엄폐와 사격이 상태 기계 없이 서로를 배제한다.** 낮은 담 뒤에 웅크리면 **총열이 담 아래**다.
"엄폐 중에는 못 쏜다"고 쓴 사람이 없다 — **담이 총열 앞에 있을 뿐**이다. 쏘려면
**일어서서 시야에 들어오는 수밖에 없다.**

> 발에서부터 재는 것도 의도다(`− CapsuleHalfHeightCm`). 웅크릴 때 **캡슐이 줄어드는 것**과
> **총구가 내려가는 것**을 둘 다 세면 총열이 두 번 내려간다.

**`LaneToleranceCm = 200`**: 탄이 **적의 엄폐물 반대쪽 입술**에서 죽는 사격은 여전히
제압으로 값이 있다. 이 여유가 거절하는 것은 **자기 쪽 벽에 처박는 사격**뿐이다.

### 3.2 `Masked` — 아군이 사선 안에 있다

`FriendlyClearanceCm = 150` 안에 아군이 있으면 거절. **벽이 앞을 막은 것과 아군이 앞을 막은
것은 다른 사건**이므로 3.1과 따로 묻는다.

### 3.3 감당 가능성 — 탄약

```
Suppressive 는 탄창잔량 > SuppressiveReserveFraction 0.4 일 때만
Aimed 는 잔량 조건 없음
```

**조준 사격은 자기 탄값을 하고, 제압 사격은 하지 않는다.** 그래서 제압은 **남는 것으로만**
사는 사치이고, 바닥난 탄창에는 남는 것이 없다.

---

## 4. ★ 이동 — 비율 게이트에는 **절대 게이트**가 필요하다 (P67) [A]

```
EffectiveSpreadAtCm(d) = d × tan(3°) × ( 1 + MovementSpreadScale 2.0
                                             × min(1.5, 속도 / ReferenceSpeedCms 600) )
        → 2026-09-14 갱신: × 사격자세 배수(Lean 1.4 / Blind 4.0) × (1 + 반동)
TotalErrorCm = √( RadiusCm² + SpreadCm² )        ← 두 오차는 독립이다
절대 게이트    TotalErrorCm ≤ SuppressiveRadiusCm 500
```

**2절의 앎↔무기 비교는 비율이다.** 비율만 두면 **무기가 절망적인 병사도 자기 앎이 더
절망적인 한 계속 자격을 얻는다** — 그게 바로 **40m를 전력질주하며 탄창을 비우는** 병사를
만드는 논리다. 절대 게이트가 "그 탄이 아무 근처에도 안 떨어진다면 맞지도 겁주지도 못하므로
살 것이 없다"를 말한다.

### 4.1 그래서 떨어지는 교전 거리 [A]

| 이동 | 교전 상한 |
|---|---|
| 정지 | **95m** |
| 걷기 | **57m** |
| 조깅 | **32m** |
| 전력질주 | **24m** |

전력질주 값은 **속도 계수의 상한**(`1 + 2.0 × 1.5 = 4.0`)에 걸린 값이다 — 폭발에 날아간
병사에게 **어떤 달리기로도 도달 못 할 속도의 벌점**을 물리지 않기 위한 클램프다.

**근접전은 그대로 run-and-gun이다.** 5m에서는 전력질주 중의 콘도 폭이 1m 남짓이다.

> ⚠ **스태미나 시스템은 이 계산 어디에도 없다.** 달리면 못 쏘는 것이 아니라,
> **달리면서 쏜 탄이 아무 데도 안 떨어질 뿐**이다.
> 위 네 값은 **계산으로 얻은 것이고 플레이에서 확인하지 않았다** → **[C-86]**.

---

## 5. 재장전 — 규칙이 아니라 같은 회계 [A]

```
bEmpty  = Ammo <= 0                                → 무조건
그 외    MagFraction < ReloadBelowFraction 0.5
         AND  bLull  = !WantsToFire()          ← ⚠ 이 소강상태는 오지 않았다
         AND  bSafe  = Exposure ≤ MaxExposureToReload 0.5

→ 2026-09-14 갱신: bSafe 와 소강상태 둘 다에 **"숨을 곳이 있다"(CanHideHere)** 가 들어갔고,
   재장전 중에는 엄폐가 있으면 DesiredStance 를 1 로 강제한다(담 뒤에서 넣는다)
```

**재장전의 값은 그것이 막는 사격이다.** 그러니 그 값이 가장 쌀 때 산다 — 어차피 쏠 것이
없고, 탁 트인 곳에 서 있지도 않을 때.

**탁 트인 곳에 묶인 병사는 반 남은 탄창을 그대로 들고 있다.** "언제 재장전하라"는 규칙을
쓰지 않고 나온 거동이다.

---

## 6. ⚠ `WantsToFire()` 는 **긍정 열거로 쓴다** (P68) [A]

```
⛔  return FireIntent != ESoldierFireIntent::Hold;
✅  return FireIntent == ESoldierFireIntent::Suppressive
        || FireIntent == ESoldierFireIntent::Aimed;
```

부정형으로 써 두면 **열거형에 값이 하나 늘 때 조용히 뒤집힌다.** `Masked` 를 추가한 순간
**병사들이 자기 편을 관통해 사격했다** — 아무 줄도 고치지 않았는데.

`bLull` 이 `!WantsToFire()` 를 쓰므로 **같은 실수가 재장전 판단까지 오염시켰다.**

---

## 7. 엄폐 — `SoldierCover` [A]

```
SearchRadiusCm        1200
CandidateCount        12
HeightSamples         3
StandChestHeightCm    135
CrouchChestHeightCm   80
MaxTracesPerTick      4          → 갱신 6   (후보 한 개가 3발에서 6발로 올랐다)
MoveImprovementMargin 0.35       → 갱신 0.25 → **0.3** (09-14)
InnerRingFraction     —          → **0.45** (09-14, 두 겹 링)
CoverChannel          ECC_GameTraceChannel5 ("Sight")   ⚠ 8.1절
```

> ★ **이 절은 "이 자리가 나를 가려 주는가"까지가 원본이다.** 그 답들로 **어느 자리를
> 고르는가**는 뒤에 **세 비용 스코어러**로 넓어졌다 —
> `Cost = Exposure + RouteRiskWeight×RouteRisk + ObjectiveWeight×ObjectiveCost`.
> 전문: **`ai/2026-09-13_objective_and_position_cost.md`** 2절.

### 7.1 ★ 엄폐는 **시야 판정을 거꾸로 돌린 것**이다

가상의 위치에 몸이 서 있다고 치고, **위협의 눈에서** 그 몸으로 트레이스한다.

**볼륨도, 마커도, 저작 데이터도 없다.** 그래서 **벽을 옮긴 디자이너는 엄폐를 옮긴 것**이다.
엄폐 데이터를 다시 굽는 단계가 존재하지 않는다.

### 7.2 ★ 한 번의 스윕이 **두 가지 답**을 준다

선 채로의 가슴 높이(135)에서 웅크린 가슴 높이(80)까지 **몸을 따라 내려가며** 트레이스한다.
**처음으로 막히는 높이**가 이 위치가 지켜 주는 높이다.

```
Exposure      = FirstHidden / (Samples − 1)      ← 여기 서면 얼마나 드러나는가
RequiredStance = Exposure                        ← 일단 가면 얼마나 낮춰야 하는가

→ 2026-09-14에 **세 번째 답**이 같은 스윙에서 나왔다 (새 트레이스 0발):
bCanHide  = FirstHidden < Samples    어느 높이에선가 막힌다
bCanFight = FirstHidden > 0          맨 위 표본이 아직 보인다 = 쏴 수 있다
```

- **전부 뚫린다** → `Exposure = 1`, `RequiredStance = 0`
  (드러난 채이고, 더 낮춰도 달라지지 않으므로 자세를 요구하지 않는다)
- **선 채로 이미 막힌다** → `Exposure = 0`, 자세도 0
- 그 사이 → 딱 그만큼 웅크린다

**낮은 담 뒤의 병사는 정확히 필요한 만큼만 웅크린다.** 그 값을 정한 것은 **담의 높이**이지
문턱이 아니다. (`HeightSamples = 3` 이므로 지금 나오는 값은 0 / 0.5 / 1 셋이다 — 해상도를
올리는 것은 표본 수를 올리는 것뿐이다.)

### 7.3 후보 — 네브메시 위에 투영된 링

`SearchRadiusCm` 반경의 링에 `CandidateCount` 개를 놓고 **네브메시에 투영**한다.
**투영이 바로 "숨을 자리"와 "바위 속"을 가르는 것**이다.

시야와 같은 라운드로빈 예산(`MaxTracesPerTick 4` → **갱신 6**, 후보당 `HeightSamples 3` → **갱신 6**(경로 표본 3발이 붙었다) 소모).
이유도 같다 — **어디로 갈지에 대해 몇 프레임 낡은 답은 멈칫하는 것보다 훨씬 싸다**
(`..._perception_stack.md` 5.2절).

### 7.4 이동 결정 — 건너는 값을 결정에 물린다

`MoveImprovementMargin = 0.35`(→ **갱신 0.25**). 지금 자리보다 **이만큼 이상 나아야** 움직인다.
**탁 트인 땅을 건너는 값**을 결정 쪽에 물리는 장치다.

**이미 진행 중인 이동은 건드리지 않는다.** 매 바퀴 다시 고르게 뒀더니 **지난 목적지에
닿기 전에 새 목적지를 골랐고**, 그것은 **허둥대는 것**으로 읽힌다 —
그리고 하필 **쏠 수도 없고 무엇의 뒤에 있지도 않은 유일한 상태**다.

### 7.5 ⚠ 출하 전에 잡은 버그 — 예산만으로는 루프가 안 멈춘다 (P69) [A]

후보 루프가 **영영 돌 수 있었다.** 네브메시 투영에 실패한 후보는 **트레이스를 한 발도 쓰지
않으므로 예산이 줄지 않는다** — 링 전체가 오프메시인 자리에 선 병사는 그 자리에서 돈다.

```
✅  while (Budget >= SamplesPerCandidate && Attempts-- > 0)
```

**반복 횟수로도 막았다.** **0 비용 반복이 가능한 루프는 예산만으로 끝나지 않는다.**

---

## 8. 튜닝값 [A]

| 컴포넌트 | 값 | 기본 |
|---|---|---|
| Engagement | `WeaponSpreadDegrees` | 3 |
| | `SuppressiveRadiusCm` | 500 |
| | `MovementSpreadScale` / `ReferenceSpeedCms` | 2.0 / 600 (계수 상한 4.0) |
| | `KnowledgeToSpreadRatio` | 1.0 |
| | `MinCertaintyToEngage` | 0.25 |
| | `FriendlyClearanceCm` | 150 |
| | `StandMuzzleHeightCm` / `CrouchMuzzleHeightCm` | 140 / 95 |
| | `LaneToleranceCm` | 200 |
| | `SuppressiveReserveFraction` | 0.4 |
| | `ReloadBelowFraction` / `MaxExposureToReload` | 0.5 / 0.5 |
| | `StanceUnderFullSuppression` | 1.0 |
| Cover | `SearchRadiusCm` / `CandidateCount` | 1200 / 12 |
| | `HeightSamples` | 3 |
| | `StandChestHeightCm` / `CrouchChestHeightCm` | 135 / 80 |
| | `MaxTracesPerTick` | 4 |
| | `MoveImprovementMargin` | 0.35 |

**이 값들은 전부 [C-87]** — 계산상의 귀결은 4.1·7.2절에 있으나 **플레이에서 재본 적이 없다.**

### 8.1 ⚠ `Cover` 채널(`GameTraceChannel4`)을 **아무도 안 쓴다** [A]

시야·엄폐·사선 **셋 다 `GameTraceChannel5`("Sight")** 를 쓴다. 엄폐가 시야 판정을
거꾸로 돌린 것이므로 **같은 채널이어야 맞다**(다른 채널이면 "보이는데 엄폐가 된다"가
생긴다). 다만 **[Q21]이 채널을 둘 판 이유** — "엄폐물은 시야를 막지만 유리·철망은 엄폐가
되면서 시야는 통과한다" — 는 **아직 실현되지 않았다.** → **[W24]**

---

## 9. 디버그 [A]

```
SoldierLab.Debug.Engagement   총구→조준점 선 + 텍스트
                              "AIMED / suppress / MASKED / blocked / hold
                               know <반경> vs weapon <탄착>  c <확신>  ammo n/m"
SoldierLab.Debug.Cover        위협의 눈 → 후보 몸 높이들로 향하는 트레이스
                              막힘 = 짙은 회색 / 안 막힘 = 옅은 회색
                              (+ 갱신분: 경로 표본 점 · 후보 구체 · "exp/obj/cost/stance")
SoldierLab.Debug.Objective    목표 원과 밴드 — `..._objective_and_position_cost.md` 8절
```

**교전 선을 가슴이 아니라 총구에서 그린다.** 가슴에서 그리면 **이 오버레이가 존재하는
단 하나의 이유** — 총열이 담 아래 있다는 것 — 이 안 보인다.

사격 의도의 색은 **출처 색(초록/호박/청록)을 빌리지 않는다.** 그것은 앎이 아니라
**행동**이므로 자기 램프를 갖는다(`..._perception_stack.md` 8절, P70).

---

## 10. 이 층이 **하지 않는** 것 [A]

| 없는 것 | 결과 | ID |
|---|---|---|
| ~~**목표·임무 개념 자체**~~ | ✅ **해소 (2026-09-13)** — `ASoldierObjective` + 세 비용 스코어러. ⚠ 단 **레벨에 한 개도 놓여 있지 않아** 셋째 항은 지금 항상 0이다 → **[W25]** · `ai/2026-09-13_objective_and_position_cost.md` | ~~[D10]~~ |
| 분대 조율·명령 | 각자 논다 | `squad/drafts/` |
| ~~경로의 노출 — 어디로 **건너가는지**는 안 본다~~ | ✅ **들어왔다 (2026-09-13)** — 다만 **직선 표본이지 네브메시 경로가 아니다.** 건물을 돌아가는 경로는 여전히 틀리게 읽는다 | **[W20]**(범위 축소) |
| 엄폐 후보가 **한 반경의 링 하나** | **2m 앞의 완벽한 자리는 후보가 된 적이 없다** | **[W21]** |
| ~~**반동 누적** — 지속 사격도 맹목사격도 산포를 넓히지 않는다~~ ✅ **들어왔다 (2026-09-14)** | **30번째 탄이 첫 탄과 같다.** 제압 사격이 공짜로 정확하고, 맹목사격의 "근거리 전용"이 자동으로 성립하지 않는다. 붙일 자리는 `EffectiveSpreadAtCm` 한 함수다 | **[W26]** |
| 데미지·체력·사망 | 병사는 죽지 않는다 — 교전이 **끝나지 않는다.** ⚠ **의도된 미구현** — 위험 회피는 노출 점수만으로 성립하고, 피격 반응은 **합류 시점에 `titan_example` 에서** 가져올 계획이다 [B] | **[W18]** |
| 45명 규모 성능 | | **[C-83]** |
