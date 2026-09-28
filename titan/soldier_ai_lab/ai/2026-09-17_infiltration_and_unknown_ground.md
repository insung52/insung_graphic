# 잠입 — "안 본 곳은 적이 있다고 친다"

2026-09-17 → 09-18 갱신 / 진행중 (사전값·콘 스윕·스프린트 규칙·볼 곳·골든앵글은 빌드·PIE 확인 · ~~필드 후보·필드 자세·Rush 우회는 빌드 전 [B]~~ → **09-18 3단계 빌드·PIE 확인**(거동 판정 [C-133]·[C-135]는 열림) · ~~09-18 추가분(볼 곳 화살표·라이트 부정 증거)은 빌드 전 [B]~~ → 09-18 오후 빌드 · **09-18 오후 CQB 추가(볼 곳 편향 · 도착 머무름 · 섹터 = 부채꼴 · `MinStance`) → 13절**, 저녁 A/B/C(얇은 엄폐)는 빌드 전 [B] · 값 전부 [C]) / 지식 0으로 시작하는 적군이 첫 목표까지 **광장을 전력 질주**하던 원인 — "믿는 적이 없으면 노출이 0"이라는 정의 — 을 걷어내고, **안 본 땅에 0.5의 사전 확률**을 주고, **눈이 훑은 만큼만** 그 값을 물리고, **급할 때나 명령받았을 때만** 뛰게 했다. 머리는 **가장 모르는 방향**을 본다. 스캔 패턴은 코드에 없다 — 보는 것이 곧 지우는 것이라 루프가 스스로 돈다.

시스템 쪽(필드의 구조·라이트·섀도우·앰비언트·오버레이)은 `ai/2026-09-17_situation_field_lighting_model.md` — 이 문서는 **병사가 어떻게 움직이는가**만 다룬다. 같은 날 오전 선행 수정은 `ai/2026-09-17_threat_bonus_lane_denied_vehicle_heights.md`.
원칙: **P144·P149·P150**(신설) · P89(규칙이 아니라 선호) · P133(부정 증거) · P86(시야 콘은 조준을 따른다) · P10(계측 먼저).

---

## 0. 한 장 요약

| 무엇 | 어디 | 상태 |
|---|---|---|
| **사전값** — 안 본 셀 = 0.5, 낡음은 0이 아니라 0.5를 향해 | `USoldierFieldSettings::UnknownPresence` · `SoldierSituationField::AgedPresence` | [A] PIE |
| **콘 스윕** — 눈이 땅을 훑어 사전값을 물린다(유일) | `SoldierSight::SweepCone` (2줄/틱, 21줄, 120°, 40 m) | [A] PIE |
| **스프린트 규칙** — 접촉·제압·사선 거부 중 하나일 때만, 엄폐로 갈 때만 | `SoldierEngagement.cpp:1354-1363` `bUrgent` | [A] PIE |
| **명령 우선** — `Cautious`는 절대 안 뛰고 `Rush`는 늘 뛴다 · Rush는 필드 조심 전부 우회 | `SoldierEngagement.cpp:1373-1385` · `SoldierCover::IsRushing` | 앞 [A] · 뒤 **[B]** |
| **볼 곳(watch point)** — 접촉 없으면 가장 모르는 방향으로 조준 | `SoldierCover::BeginSweep` → `GetWatchPoint` → `SoldierEngagement.cpp:1270-1300` | **[B]** |
| **골든앵글 회전** — 링·부채꼴이 스윕마다 2.39996 rad 돈다 | `SoldierCover::BeginSweep` `CandidateRotationRad` | [A] PIE |
| **필드 후보** — 진영 필드의 가장 어두운 셀 6개를 후보에 | `SoldierCover::AddFieldCandidates` · `FieldCandidateCount 6` | **[B]** |
| **필드 자세** — 눈 0일 때 필드가 노출·필요 자세·은폐를 답한다 | `SoldierCover::EvaluatePosition` 눈 0 분기 · `HiddenThreshold 0.25` | **[B]** |
| ⛔ 가상 관찰자 `AddUnknownWatchers` | 같은 날 도입 → 철회 | 시스템 문서 12.2절 |

---

## 1. 요구 — 사용자의 말

시나리오 시작 시 적군은 **지식 0**으로 첫 목표(첫 전투지)까지 **잠입**한다. 사용자가 그은 선:

- **"문을 열고 들어가는 것"이지 "박차고 들어가는 것"이 아니다.**
- **모든 병사**(정찰이든 돌격이든)가 **안 본 땅은 점유된 것으로** 취급한다.
- **뛰는 것은 급할 때나 명령받았을 때만.**

이건 새 기능 요청이 아니라 **기존 거동의 결함 보고**였다 — 09-17 오전 빌드 뒤 적군 분대가 접촉 없이 광장을 대각선으로 전력 질주해 첫 전투지에 들어갔다.

---

## 2. 진단 — 노출이 "믿는 적"에 대해서만 정의돼 있었다 [A · 코드]

세 겹이 겹쳐 있었다:

1. **엄폐 층의 조기 탈출.** `EvaluatePosition`은 `Eyes.Num()==0`이면 트레이스 없이 `CanHide=true, Exposure=0`을 반환했다(옛 코드). 기억 속 적이 없다 = **아무도 나를 못 본다** = 자세도 엄폐 개념도 없음. 링 후보 12개가 전부 노출 0이라 자리 선택은 목표 비용만 남고, 목표를 향해 **직선**.
2. **위험 지도의 침묵.** 09-14 지도는 셀에 "보였음/공터였음/총알" 시각을 찍는 구조라 **아무 일도 안 일어난 셀은 0**이었다. 안 본 곳과 확인한 곳이 같은 값.
3. **스프린트가 "재배치면 무조건".** `WantsToSprint`가 `IsMovingToCover() && !WantsToFire()`뿐이라 접촉 없는 이동도 전부 달렸다(`SoldierEngagement.cpp:1359-1361` 주석).

"안 본 곳은 적이 있다고 친다"는 셋 다에 답이 돼야 했다: **(a)** 안 본 땅에 값이 있어야 하고 **(b)** 눈 0일 때 그 값이 자세·자리에 들어가야 하고 **(c)** 뛰는 조건이 따로 있어야 한다.

---

## 3. 사전값 — `UnknownPresence 0.5` (P149) [A · PIE]

`USoldierFieldSettings::UnknownPresence = 0.5`(`SoldierFieldSettings.h:73-82`). 한 번도 관측 안 된 셀은 경계도 0.5를 읽고, **모든 경계도 값은 0이 아니라 0.5를 향해 낡는다**(`AgedPresence`, `SoldierSituationField.cpp:320-332`, 반감기 `PresenceHalfLifeSeconds 8`).

> 0은 "적이 없다"인데 빈 지도가 뜻하는 것은 그게 아니라 "아무도 확인 안 했다"다. 1분 전에 확인한 복도는 다시 모르는 복도가 된다 — 그래야 맞다. (`SoldierFieldSettings.h:76-79`)

이 값이 곧 **앰비언트의 방사 강도**다(시스템 문서 6절). 사전값을 낮추면 잠입이 대담해지고 0이면 09-17 오전으로 돌아간다.

---

## 4. 콘 스윕 — 보는 것만이 사전값을 물린다 [A · PIE]

`USoldierSightComponent::SweepCone`(`SoldierSight.cpp:142-198`), `TickComponent`가 **표적 트레이스보다 먼저, 무조건** 부른다(`:233-237` — "볼 사람이 없는 병사가 가장 봐야 하는 병사다").

- **예산 분리**: `ConeSweepTracesPerTick 2`(`SoldierSight.h:70-82`) — 표적 트레이스 `MaxTracesPerTick 5`와 **서로 굶기지 않게** 따로. 0이면 잠입 OFF.
- `ConeSweepRays 21`줄을 120° 콘(`SightHalfAngleDeg 60`)에 등간격, 틱마다 2줄씩 **라운드로빈**(`SweepCursor`) — 고개를 돌린 병사의 땅 지식이 **본 순서대로** 따라온다. 21인 이유: 앰비언트가 경계도를 표본하는 거리(`HorizonRangeCm 4000`)까지 이웃 광선이 이웃 셀에 떨어져야 광선 사이에 안 지운 틈이 안 남는다(`:96-106`).
- `ConeSweepRangeCm 4000`(시야 120 m보다 짧다 — "들어가는 땅"에 대한 것이라).
- **막힌 곳까지만** `MarkClearAlongRay` — 벽 뒤를 지우는 것은 이 층 전체가 피하려는 전지성이다(`:180-184`).
- `SoldierLab.Debug.Sight 1`이면 스윕 선이 어두운 회색/청록으로 보인다.

목격 시에도 사선을 **먼저** 비우고 라이트를 켠다(`:288-301`) — 순서를 바꾸면 적이 선 셀이 지워진다(P151).

**09-18 추가 — 훑은 선은 라이트에도 부정 증거다**: `MarkClearAlongRay`가 땅을 비우는 것 외에 그 선이 지나간 **얼린 라이트**에 "빈 채로 보고 있는 시간"을 적립한다(`ContradictLightsAlong`). 라이트 밝기는 `LightHalfLifeSeconds 20` 위에 `ClearViewHalfLifeSeconds 4`를 한 번 더 곱해 — 사용자 질문 "직접 가서 봤는데 없으면 사라지나?"에 대한 답이 "아니오"였던 것을 "더 빨리 잊는다"로. **삭제가 아니다**(웅크리고 있거나 2 m 옆에 있을 수 있다, P156). 시스템 문서 18절 · [C-142] · 들은 라이트 처리 불일치 [W83]. **빌드 전 [B]**.

---

## 5. 스프린트 규칙 — 속도는 노출로 산다 (P150) [A · PIE]

`SoldierEngagement.cpp:1354-1385`:

```
bUrgent       = bHasContact || SuppressionNow > 0 || bLaneDenied
bWantsToSprint = Cover->IsMovingToCover() && !WantsToFire() && bUrgent
switch (Task.Speed):
  Cautious → false            (절대 안 뛴다)
  Rush     → IsMovingToCover() (무조건 — 조심 없이)
```

> 달리기는 상황이 **이미 결정됐을 때** 하는 것이다: 누가 알려져 있거나, 탄이 지나가거나, 이 자리가 더 안 통하거나. 아무도 안 본 땅을 건너는 것은 반대 경우다 — 달리기는 되돌릴 수 없고 걷기는 뭔가 나타나는 순간 멈출 수 있다. 그것이 조심스럽게 움직인다는 것의 전부다. (`:1354-1360`)

`bLaneDenied`가 포함된 이유: 사선이 거부된 자리를 떠나는 이동은 **이미 위험한** 자리를 떠나는 것이라 급하다(오전 수정, 선행 문서 2절).

⚠ `Cautious`는 여전히 **조깅**이다(걷기 gait 없음, [W69]). 명령 없이 접촉 없는 이동도 조깅.

---

## 6. 머리는 가장 모르는 방향으로 — 볼 곳 루프 [B]

`SoldierCover::BeginSweep`(`SoldierCover.cpp:1163-1175`): 눈 0 ∧ 필드 있음 ∧ Rush 아님이면 `Field->GetMostExposedDirection(발)` → `WatchPoint`. 필드는 앰비언트 8방향 중 **(트임 × 확산 경계도)가 최대인 방향의 트인 끝점**을 준다(`SoldierSituationField.cpp:1127-1158` — "첫 1 m가 아니라 보이는 끝을 본다").

`SoldierEngagement.cpp:1270-1300`: 접촉 없을 때 조준은 **섹터 명령 > 볼 곳 > 없음**. 조준은 표적 때와 같은 `AimSlewDegreesPerSecond 240`으로 선회하고 시야 콘이 조준을 따르므로(P86) 콘 스윕이 그 방향을 훑는다 → 경계도가 떨어진다 → 그 방향이 더는 최악이 아니다 → 다음 스윕이 다른 곳을 가리킨다.

> 스캔 패턴도 타이머도 없다 — **스캐닝은 피드백 루프다.** (`SoldierCover.h:146-154`)

~~`Task.bHasSector`(분대 명령의 경계 방향)가 있으면 그것이 이긴다 — 명령이 "어디를 보라"고 했으면 그게 답이다.~~ → **09-18 오후 정정(13.1절)**: 섹터는 고정 방위가 아니라 **부채꼴(arc)** 이고, 그 안에서 필드가 가장 안 훑은 방위를 고른다. 고정 방위로 읽자 존(기본 `bUseSector true`)으로 명령받은 병사는 이 루프가 **한 번도 안 돌았다.**

**09-18 추가 — 볼 곳이 이제 보인다**: 필드 오버레이(`SoldierLab.Debug.Field 1`)가 관찰 중인 병사의 발에서 `GetMostExposedDirection` 방위로 **보라색 2.5 m 화살표**를 그린다(`SoldierSituationField.cpp:2269-2288` — 방위만, 끝점은 40 m까지 가서 경로로 읽히므로 짧게). 이 루프가 도는지는 화살표가 훑은 뒤 **다른 방향으로 넘어가는가**로 판정한다(10절 "볼 곳 루프" 행). 3단계는 09-18 빌드·PIE로 확인됐고(시스템 문서 14절), 화살표 자체는 **빌드 전 [B]**.

---

## 7. 골든앵글 회전 [A · PIE]

`BeginSweep`(`SoldierCover.cpp:1155-1158`): `CandidateRotationRad += 2.39996323 rad`(mod 2π). 링 12칸의 각도(`:545`)와 부채꼴 광선의 지터(`:489-491`, 한 광선 간격 안에서)가 이 값을 쓴다.

**왜**: 안 움직인 병사의 후보는 스윕마다 **똑같은 20여 점**이었다 — 월드 각도 고정 링 12칸 + 콘 고정 오프셋 부채꼴. 12 m 링에서 이웃 칸 사이 6 m 땅은 경기 내내 한 번도 표본 안 됐고 거기 아무리 좋은 엄폐가 있어도 못 찾았다(`SoldierCover.h:669-682`). 골든앵글은 **이전 스윕 각도를 절대 안 반복**하고 연속 스윕을 가장 고르게 퍼뜨리는 유일한 회전 — 4스윕이면 링을 4배 촘촘히, 스윕당 비용 증가 0.

---

## 8. 필드 후보와 필드 자세 [B, 빌드 전]

### 8.1 후보 — `AddFieldCandidates` (`SoldierCover.cpp:343-374`)

`BuildCandidates`(`:567-594`) 순서: **부채꼴 그림자**(기억 속 눈마다) → **필드의 어두운 셀**(`FindDarkestCells`, `SearchRadiusCm 1200` 안, `FieldCandidateCount 6`, Rush 아닐 때) → **링**. 필드 셀은 내비메시에 투영돼야 후보가 된다.

> 아무도 기억 안 날 때 이것이 **엄폐가 어디 있는가**를 말하는 유일한 후보다 — 링은 땅이 어느 쪽으로 기우는가만 말한다. 적이 안 보이는 병사가 길 한복판 대신 **문간에서 문간으로** 가게 하는 것이 이것이다. (`SoldierCover.h:281-293`)

### 8.2 자세 — `EvaluatePosition` 눈 0 분기 (`SoldierCover.cpp:616-652`)

옛 조기 탈출 자리에 필드 읽기가 들어갔다. `GetExposureByStance` → `HiddenThreshold 0.25`:

| 읽기 | 뜻 | Exposure | RequiredStance | CanHide |
|---|---|---|---|---|
| Standing < 0.25 | 서도 안 보인다 | 0 | 0 | true |
| Crouched < 0.25 | 서면 보이고 웅크리면 안 보인다 — **낮은 벽** | 1 | 1 | true |
| 그 외 | 아무리 낮춰도 보인다 — 공터 | 1 | 0 | false |

트레이스가 주는 것과 **같은 세 사실**이라 하류(자세·`FightingCost`·이동 결정)가 그대로 돈다. `CanFight`는 기본 true 유지. **Rush면 옛 거동**(`CanHide=true`, 어디든 똑같이 안전).

### 8.3 Rush 우회 — `IsRushing()` (`:332-341`)

배정 `Speed == Rush`면 필드 후보·필드 자세·볼 곳 **셋 다 건너뛴다.** "명령은 **있을지도 모르는 것**에 대한 조심을 끄는 것 — 진짜 적은 사실이라 여전히 숨는다. 이것과 교전 층의 스프린트 규칙은 같은 문장이다: 명령받았거나, 이미 조심하기엔 늦었을 때 뛴다."

---

## 9. 튜닝값 — 이번에 신설·변경 (전부 [C])

### 9.1 `USoldierSightComponent` → [C-134]

```
ConeSweepTracesPerTick  2      (신설 — 표적 트레이스와 별도 예산, 0이면 잠입 OFF)
ConeSweepRays           21     (신설)
ConeSweepRangeCm        4000   (신설)
SightingRadiusCm        100    (신설 — 목격 라이트의 반경)
```

### 9.2 `USoldierPerceptionComponent` → [C-134]

```
HeardPresence           0.75   (신설 — 총성 라이트 밝기, 사전값 0.5보다 커야 한다)
```

### 9.3 `USoldierCoverComponent` → [C-135]

```
FieldCandidateCount     6      (신설)
CandidateRotationRad    골든앵글 2.39996 rad/스윕 (상수)
DangerWeight            0.8    (값 그대로, 뜻이 바뀜 — 위험 지도 → 필드 GetExposure, 눈 0일 때만)
DangerHalfLifeSeconds   삭제   (위험 지도와 함께)
LaneDeniedCost          1.0    (오전분, 선행 문서)
```

### 9.4 `USoldierEngagementComponent`

스프린트 규칙에 새 값은 없다(`bUrgent`는 기존 상태의 조합). 오전분(`ThreatenedBonus` 등)은 선행 문서 → [C-136].

필드 자체의 값(`UnknownPresence` · `HiddenThreshold` 등)은 시스템 문서 10절 → [C-130]~[C-133].

---

## 10. 판정 기준 — 다음 실측 [C]

| 항목 | 방법 | 기준 |
|---|---|---|
| 접촉 없는 이동 속도 | `[Engage]` 로그 `sprint` 전이 · 육안 | 첫 접촉 전 스프린트 0회(Rush 명령 제외) |
| 문간→문간 | `SoldierLab.Debug.Cover.Log 1`, `eyes=0` 스윕의 `tried` / `MOVE @` 목적지 | 목적지가 필드 후보(어두운 셀)에 떨어지는 비율 — 링/직선이 아니라 |
| 사전값이 물러나는가 | `SoldierLab.Debug.Field 1` 채널 1 | 콘 방향 초록이 검게 → 8 s 뒤 중간 초록 복귀 |
| 볼 곳 루프 | 채널 4(앰비언트) + 조준 방향 | 접촉 없는 병사의 조준이 보라가 진한 방향으로 돌고, 훑은 뒤 다른 방향으로 넘어감. 한 방향 고정이면 실패 |
| 골든앵글 | `SoldierLab.Debug.Cover 1` 후보 점 | 정지 병사의 후보 점이 스윕마다 다른 각도에 찍힘 |
| 필드 자세 | 눈 0 병사의 `st`(오버레이) | 문간에서 0/낮은 벽에서 1/광장에서 0+Exposure 1 |

---

## 11. 분대 층의 몫 — 이 문서가 하지 않은 것

잠입은 개인 거동만으로 완성되지 않는다. 남은 것은 **L0/L1**(`squad/2026-09-17_command_layer_design.md`, [W55]):

- **경로·대형·교대 엄호(bounding overwatch)** — 지금은 각자 필드를 읽고 각자 간다. S4(엄호 이동, 09-16)가 사실로서의 엄호는 주지만 "너 먼저, 나 다음"은 없다.
- **시나리오 DT가 실제로 `Cautious`와 섹터를 발행해야** 한다 — 지금 잠입 거동은 명령이 없을 때의 **기본값**으로만 나온다. `IssueSquadOrder`가 `Speed=Cautious, Sector=…`를 주면 스프린트가 봉쇄되고 조준이 섹터를 따른다. `Cautious` 걷기 gait는 [W69].
- **분대 스코프 필드** — 진영 공유는 근사([W74]).
- **거리 의존 발견 시간**([W56]) — 120 m 안이면 즉시 본다. 잠입 중 "알아채는 데 걸리는 시간"이 없다.

---

## 12. 변경 파일

시스템 문서 15절과 동일(같은 세션). 이 문서 몫: `SoldierSight.h/.cpp`(콘 스윕) · `SoldierEngagement.cpp:1354-1385`(스프린트) · `:1270-1300`(볼 곳) · `SoldierCover.h/.cpp`(`IsRushing` · `AddFieldCandidates` · 눈 0 분기 · 골든앵글 · `WatchPoint`) · `SoldierPerception.h`(`HeardPresence`).

---

## 13. 09-18 오후 추가 — CQB "눈이 발을 이끈다" · 섹터 = 부채꼴 · `MinStance` [A · PIE ✅, 저녁분 B]

전문은 `ai/2026-09-18_patrol_scan_and_move_robustness.md`(1·4·5절). 여기는 이 문서의 거동 서술이 **어떻게 바뀌었는가**만. 줄 번호는 09-18 저녁 판(`SoldierCover.cpp` 1909줄 · `SoldierEngagement.cpp` 1502줄).

### 13.1 6절 정정 — 섹터는 부채꼴, 볼 곳은 항상 계산

- **섹터 > 볼 곳**이 아니라 **볼 곳(부채꼴 안에서) > 섹터 중심**이다. `BeginSweep`이 정지 상태에서 배정의 `bHasSector`면 `SectorYawDeg ± SectorHalfWidthDeg`를 `GetMostExposedDirection`의 arc로 넘기고(`SoldierCover.cpp:1489-1495`), 교전 층은 볼 곳이 있으면 그 방위, 없을 때만(미굽기) 섹터 중심(`SoldierEngagement.cpp:1289-1303`). **이동 중엔 arc를 안 넘긴다** — 가는 곳을 본다.
- 볼 곳은 이제 **눈이 있어도 계산**한다(`:1423-1433`, Rush만 예외). 09-17의 "눈 0 ∧ …" 게이트는 죽은 적 기록이 ≈ 90 s 눈으로 남는 동안 교전 층(다른 문턱)과의 틈을 만들어 **아무도 조준을 안 몰았다** — 사용자가 본 "가만히 서서 화살표 반대를 보는 아군".
- 교전 층이 무접촉 분기에서도 **`GetAimPoint()`**(`SoldierEngagement.cpp:1321` — 항상 AI 컨트롤러 초점과 같은 점)와 **`IsScanning()`**(`:1322`, 볼 곳이 있을 때만 true)을 발행한다. 시야 콘은 원래 초점을 따라 돌았지만(P86) 총 내린 idle에서 **몸**은 안 돌았다([W69]의 "(추정)"이 사실로) — 포즈 세션의 `Pose/SoldierScanTurnComponent`가 이 둘을 읽어 캡슐을 돌린다(전문 1.5절, 동작 확인 [C-152]).

### 13.2 이동 중 볼 곳 — CQB (`WatchTravelBias 1` · `WatchApproachBias 0.5` · `ScanDwellSeconds 2`)

"모퉁이를 돌면서 측면을 보는 병사는 중요한 것을 하나도 안 훑은 것이다." `BeginSweep`(`:1447-1495`):

| 상태 | 편향 | 값 |
|---|---|---|
| 이동 중(속도 > `StallSpeedCms 20`) | 진행 방향 — `Score ×= 1 + 1.0·dot` → 정후방 0 · 정면 ×2 | `WatchTravelBias 1.0` |
| 이동 중 ∧ 경로 굽이가 `CornerLookAheadCm 500` 안 (저녁 C, **[B]**) | **굽이 너머** 방향(속도 대신) | 〃 |
| 정지 ∧ Approach 배정 | 앵커 방향 ×1.5 | `WatchApproachBias 0.5` |
| 정지 ∧ 편향 없음 ∧ 섹터 | arc 제한 | — |

도착하면(`ArrivedTimeSeconds`) 눈 0일 때 **엄폐 유무와 무관하게** `ScanDwellSeconds 2`(`.h:370-377` · `FinishSweep` `:1557-1563`)는 `DwellBreakMargin 1.0`으로 잠긴다 — 다음 홉은 콘 스윕이 새 땅을 훑은 뒤. 4절의 콘 스윕은 그대로고, `MarkClearAlongRay`가 16 m 밖에서 **3셀 폭 띠**를 비우게 된 것(시스템 문서 23절)이 이 루프가 한 방위에 안 걸리게 한다.

### 13.3 자세 — `MinStance` (분대 세션의 "숙여서 침투 없음"에 대한 답)

8.2절의 필드 자세는 이미 **숨는 곳에서만** 웅크린다(낮은 벽 1, 공터 0 — 공터에서 웅크림은 속도만 잃는다). "**어디서나** 더 낮게"는 다른 요구라 명령의 제약으로: `FSoldierAssignment::MinStance 0..1`(`SoldierOrderTypes.h:129-140`) → `DesiredStance = max(DesiredStance, MinStance)`(`SoldierEngagement.cpp:795-800`), **Rush면 안 적용**(다른 조심과 함께 풀린다). ⚠ titan 쪽 DT(`FScenarioSquadOrderSpec`)·`titan.SquadOrder` 콘솔·`ASoldierZone`엔 아직 없다 → [W84] 절반. 값 [C-151].

### 13.4 5절 스프린트 규칙 · 8.3절 Rush — 불변

`bUrgent = 접촉 ∥ 제압 ∥ 사선 거부`(`SoldierEngagement.cpp:1384-1386`), `Cautious` 절대/`Rush` 항상(`:1398-1401`) 그대로. `IsRushing()`이 끄는 목록만 늘었다: 필드 후보 · 필드 자세 · 볼 곳 · **`MinStance`** · **코너 멈춤**(저녁 C, `SoldierCover.cpp:768`).

### 13.5 9절 값 추가 → [C-149] [C-151]

```
USoldierCoverComponent   WatchApproachBias 0.5 · WatchTravelBias 1.0 · ScanDwellSeconds 2   (신설)
FSoldierAssignment       MinStance 0   (신설 — 존 동사와 함께 흐름, titan DT 미연결)
```

10절 판정표의 "볼 곳 루프" 행은 이제 **명령받은 병사에게도** 적용된다(부채꼴 안에서 화살표가 도는가) · "접촉 없는 이동 속도" 행에 "이동 중 조준이 진행 방향 ±로 기우는가"(`SoldierLab.Debug.Engagement 1` 사선 + `[Cover] watch=1`)를 더한다.
