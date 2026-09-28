# 분대 항 넷과 "배운" 죽음 — 등록부만으로 서로를 아는 병사들

2026-09-16 / 진행중 (코드·빌드·PIE 완료, 사용자 평가 "잘 됨", 값은 전부 [C]) / `ai/2026-09-15_exposure_cycle_and_muzzle_learning.md`의 후속. **분대 항 넷**(자리 주장 · 사선 비우기 · 표적 분담 · 엄호 이동)을 분대 객체 없이 **등록부 하나로** 비용식에 얹었고, 사망 시스템이 붙자 드러난 **유령 표적**을 "세계가 알려 주는" 방식(사용자 거부)이 아니라 **눈으로 보고 입으로 전하는** 방식으로 풀었다. 그 뒤 **블라인드 펄스 · 부정 증거 · 시야 거리 · 두 점 시야**까지 네 건.

전편: `ai/2026-09-15_exposure_cycle_and_muzzle_learning.md`(노출 회계 · 총구 학습 — 그 문서 끝 정정 절에 이 문서가 바꾼 것이 있다).
사망 시스템: `ai/2026-09-15_health_hit_death_implementation.md`(다른 세션 — 이 문서는 그 위에 얹힌 **인지 쪽 경계**만 다룬다).
원칙: P89(규칙이 아니라 선호) · P63(감쇠는 질의 시) · P86(시야 콘은 조준을 따른다) · **신설 P130~P133**(`CLAUDE.md` 5절).

> **이 문서의 방식**은 전편과 같다 — **로그로 확정한 사실 [A] → 원인 → 구현 → 값(← 옛값)**. 로그는 `SoldierLab.Debug.Cover.Log 1` · `SoldierLab.Debug.Engagement.Log 1`(`LogSoldierAI`). 사용자 육안 평가는 "잘 됨" 한 줄이고 **수치 판정은 8절에 남아 있다.**

---

## 0. 한 장 요약

| 무엇 | 어디 | 상태 |
|---|---|---|
| **S1 자리 주장** — 같은 그림자를 둘이 고르지 않게 | `SoldierIdentity::SetClaim/GetClaim` · `SoldierCover::SquadCost` | [A] 구현 · 겹침 실측 미완 [C] |
| **S2 사선 비우기** — 아군 총구→표적 선분 안에 서지 않게 | `SoldierCover::SquadCost` | [A] 구현 · `MASKED` 실측 미완 [C] |
| **S3 표적 분담** — 아군이 이미 잡은 적은 감점 | `SoldierEngagement::PickCandidate` | [A] 구현 |
| **S4 엄호 이동** — 아군이 쏘는 적의 눈은 값이 반 | `SoldierEngagement::IsCoveringEnemy` · `SoldierCover::IsEnemyCoveredByTeammate` | [A] 구현 · 교착 방지 설계 [B] |
| 비용에 `q` 항 | `FSoldierPositionCost::Squad` · 로그/오버레이 `q` | [A] |
| **유령 표적** — 죽은 적을 계속 조준 | 1차(전지적 방송) **철회** → 눈(`SoldierSight` 시체 확인) + 입(`SoldierComms` 사망 보고) | [A] 구현 |
| **블라인드 펄스** — 총을 올렸다 내렸다 | `SoldierEngagement` 종료 조건을 시작 이유별로 | [A] 구현 |
| **부정 증거** — 보고 있는데 없다 | `SoldierSight` → `SoldierPerception::ReportClearView` | [A] 구현 |
| 시야 거리 60 → **120 m** | `SoldierSight::SightRangeCm` | [A] 값 변경 · 귀결 [C] |
| **두 점 시야** — 가슴 → 머리 | `SoldierSight::HasLineOfSight` · `SoldierCover::EvaluatePosition` · `SoldierIdentity::GetHeadHeightCm` | [A] 구현 · **미제출** |

**Perforce**: 사용자 CL **471**(S1~S4 + 사망 학습), CL **472**(블라인드 수정 · 부정 증거 · 시야 12 m) 제출. **두 점 시야는 미제출**(체크아웃 상태, 2026-09-17 기준).

---

## 1. 분대 항 넷 — 등록부만으로 [A · 코드]

**전제**: 분대 객체·리더·명령(L0)은 아직 없다. 넷 다 `USoldierRegistrySubsystem`(전 병사의 평평한 배열)을 읽어 **기존 비용식에 항을 더하거나 기존 선택에 감점을 넣는** 형태다. 규칙("한 명만 가라")이 아니라 선호(P89)라, 겹쳐야 할 이유가 더 크면 겹친다. 무전(`SoldierComms`) 확장은 없다 — 같은 편이 서로 어디 있는지는 무전 없이도 아는 것으로 본다.

### 1.1 S1 자리 주장 (claim)

- 스윕 끝마다 병사가 자기 Identity에 **"가는 자리"**(MOVE 결정 순간의 목적지) 또는 **"선 자리"**를 공표한다(`SetClaim`). 주장의 **나이는 땅이 바뀐 시각**부터 센다 — 같은 자리를 다시 말해도 나이가 안 돌아간다.
- 후보 점수: 아군 주장 `ClaimRadiusCm 250` 안이면 `+ClaimedCost 1.0`.
- **현재 자리**는 **상대가 먼저 주장했을 때만** 비용 → 겹친 둘 중 **정확히 한 명**(나중 온 쪽)이 비킨다. 둘 다 비키거나 둘 다 버티는 대칭이 없다.
- 목적지를 **고르는 순간** 주장하므로, 0.3 s 뒤 스윕하는 동료는 아무도 도착하기 전에 그 자리가 잡힌 것을 본다.

### 1.2 S2 사선 비우기

- 아군 중 접촉이 있는 병사마다 **실제 총구(→ 없으면 눈)에서 `AimPoint`까지의 선분**을 그리고, 후보의 가슴점이 그 선분에서 `LaneClearanceCm 150` 안이면서 **양 끝 사이**에 있으면 `+LaneCost 0.6`.
- 현재 자리에도 같은 비용 → 아군 사선에 서 있는 병사가 비킨다. `MASKED`(아군이 사선에 있어 사격 거절)를 **위치 층에서** 줄이는 것이 목적이다.

### 1.3 S3 표적 분담

- 표적 후보 선택이 `GetPrimaryContact`(확신 최대)에서 **`PickCandidate`**로: 점수 = 확신 − `TargetCrowdingPenalty 0.15` × (그 적을 이미 잠근 아군 수, `CountTeammatesOn`). 동률은 반경 작은 쪽.
- 표적 **교체 문턱**(`TargetSwitchCertaintyMargin 0.3`)도 **같은 점수**로 비교한다 — 아군 셋이 붙은 적은 그 이유만큼만 떠날 수 있다.
- 확신이 이긴다: 겨우 믿는 접촉을 위해 확실한 접촉을 버리지 않는다.

### 1.4 S4 엄호 이동 (bounding overwatch)

**핵심**: 엄호는 **의도가 아니라 사실**로만 인정한다.

```
IsCoveringEnemy(E) = 정지(속도 ≤ CoveringStillSpeedCms 20)
                   ∧ WantsToFire()
                   ∧ GetLockedEnemy() == E
```

이 사실이 성립하는 아군이 있으면 그 적의 **눈 활동도 × `CoveredEyeFactor 0.5`** → 그 눈 아래를 건너는 위험이 반값. 쏘는 사람은 남고, 안 쏘는 사람은 갈 수 있고, 도착해 쏘기 시작하면 역할이 바뀐다 — 교대 이동이 **명령 없이** 나온다.

**대칭 교착 방지** (둘이 서로의 적을 쏘는 중 → 둘 다 상대를 엄호자로 보고 둘 다 뛰는 경우):

1. 엄호자로 인정하는 아군은 **자리 비용(`GetPositionCost`)이 내 것보다 나쁘지 않은** 쪽만 — 나쁜 자리에 있는 쪽이 먼저 간다.
2. 동률이면 **등록부 앞 번호**가 엄호자(결정적 타이브레이크).
3. 내가 출발하는 순간 나는 정지가 아니므로 **누구의 엄호자도 아니다** → 상대는 할인 없음 → 대기.
4. 둘 다 숨은 구간(내밀기 사이)이면 아무도 엄호자가 아님 → 이전 거동 그대로.

남는 위험 [B]: "아군이 쏘는 적"과 "나를 보는 적"이 **다른 적**이면 할인은 **눈별**로 걸리므로 다른 적의 눈은 값 그대로다. 4개 시나리오 중 동시 출발이 남는 건 **두 스윕 끝이 0.2 s 안에 겹치는** 경우뿐이고, 그때도 둘 다 좋은 후보로 가므로 결과는 "엄호 없는 이동" 한 번이다.

### 1.5 비용에 `q` 항

`FSoldierPositionCost`에 `Squad` 항(S1+S2)이 추가되어 `Total()`에 들어간다. 로그 `[Cover] … HERE f o d s q =tot | best f r o d q =tot`, 오버레이 둘째 줄에도 `q`.

---

## 2. 유령 표적 — 죽은 적을 계속 조준한다 [A · 로그]

**증상**(사용자): 아군·적군 한 명씩 남은 상황에서 **몇 초 동안 서로 멀뚱거리다** 시간이 지나야 쏜다. 사이에 엄폐물도 없었다.

**로그** (사망 시스템 첫 세션):

- `Ally_A` t=28~47.5: `tgt Enemy_A`(**이미 사망**) — 그런데 `age 0.0`으로 기록이 **계속 갱신**된다. 사망 시 등록부에서 빠져 시야는 안 잡히지만, **살아 있는 Enemy_A2의 익명 총성**이 위치 게이팅으로 죽은 Enemy_A의 기록에 **융합**되고 있었다(유령 기록의 반경이 12 m까지 자라 있어 근처 총성이 전부 빨려 들어감) → 확신 1.0 유지 → 표적 교체 문턱(+0.3)을 못 넘음.
- `Enemy_A2`도 같은 방식으로 `tgt Ally_B`(사망, t=34.7)를 t=47.6까지 잠금.
- **시야 콘이 조준 방향을 따르므로**(P86) 둘 다 유령을 보느라 **서로를 못 봤다.** 47.5 s에 우연히 콘에 들어오자 즉시 AIMED.

**원인**: 사망 시스템은 등록부 해제까지 했는데(`USoldierHealthComponent::Die` → `Unregister`), **인지 기록 쪽에 "이 적은 죽었다"가 전달되는 경로가 없었다.**

### 2.1 ⛔ 1차 시도 — 철회

등록부 `Unregister`에 방송(`OnUnregistered`)을 달아 **모든** 인지 컴포넌트가 그 적의 기록을 즉시 삭제하고, 등록되지 않은 적을 가리키는 기록은 애초에 받지 않게 했다. **사용자가 거부**: "누가 죽는 걸 방송한다고? 정식 팀원 소통 루트를 타고 방송되는 거 맞아?" — 맞는 지적이다. 이것은 **양쪽 전원이 즉시 아는 전지적 채널**이고, 이 프로젝트의 인지 원칙(관측한 것만 안다, P63 계열)에 어긋난다. 전부 되돌렸다(`grep`으로 잔재 0건 확인).

### 2.2 ✅ 최종 — 죽음도 관측·전달로만 (P130)

| 경로 | 구현 |
|---|---|
| **눈** | `SoldierSight`가 살아 있는 표적 스캔을 마친 뒤 **남은 트레이스 예산으로**, 기억 속 적 중 **등록부에서 빠진 몸**을 같은 거리·같은 시야각·같은 트레이스로 확인. 몸에 선이 닿으면 그때 `USoldierPerceptionComponent::LearnDeath` — 기록 삭제 + `KnownDead` 집합 등록. 못 보면 계속 살아 있다고 믿고, 근처 총성이 그 기록에 융합되는 것도 그대로(**정직한 오판** — 실제 병사도 속는다) |
| **입** | `SoldierComms::BeginCasualtyTransmission` — **같은 입, 같은 `VoiceRangeCm 2500`, 같은 `MessageDurationSeconds 1.2`**로 "X 쓰러짐"을 전달. 새로 배운 사망(`TakeDeathToReport`)이 있으면 접촉 보고보다 **먼저**. 받은 쪽도 `LearnDeath`. 사망만 예외적으로 **릴레이**(낡지 않는 사실이고 각자 한 번만 말하니 홉 단위로 퍼지다 멈춤 — `bRelayHearsay`와 무관) |
| 방어 | 자기가 죽은 줄 **아는** 적에 대한 늦은 무전은 무시(`IntegrateRecord`, 자기 지식 기준) |

`USoldierRegistrySubsystem::IsRegisteredActor`는 남겼다 — **"어느 기억이 시체인지 골라 쳐다볼 대상을 정하는" 용도**이고, 지식은 트레이스가 성공할 때만 흘러간다. 주석에 그렇게 못 박았다.

**기대 거동**: 죽인 쪽은 조준하던 곳에 시체가 있으니 다음 틱에 배움 → 표적 해제 → 남은 적 탐색. 못 본 쪽(반대편)은 무전 25 m 안이면 1.2 s 뒤, 멀면 기억이 낡을 때까지(또는 시체를 볼 때까지) 유령을 믿는다.

---

## 3. 블라인드 펄스 — 총을 올렸다 내렸다 [A · 로그]

**증상**(사용자): 적이 다 죽은 뒤 아군 한 명이 **블라인드 파이어로 총을 위로 올렸다 내렸다** 반복.

**로그**: `Ally_A` t=61~74 **전이 693회**, 매 틱 `blocked→hold`(`direct/open`, 사선 뚫림) ↔ `hold→blocked`(`over/blind`, 막힘) 반복. 표적은 t≈52에 들은 **익명 총성 기록**(9.8 m, 확신 0.55→0.25로 15 s에 걸쳐 소멸).

**원인** (두 겹):

1. 예상점이 **벽 뒤 2 m 근처**라 총구의 미세한 움직임에 `LaneToleranceCm 200` 경계가 걸려 **사선이 매 틱 뚫림/막힘으로 반전**.
2. 막힌 틱마다 `PlanAperture`가 "총 올려 블라인드"(`Over/Blind`)를 고르고 → **같은 틱에** 종료 조건 `bBlindNoLongerNeeded`("제압도가 낮으니 블라인드는 불필요")가 **즉시 취소** → `DesiredBlindFireV`에 1이 한 틱씩 찍혀 BP 축이 오르내림. 설계 실수: 블라인드는 **"너무 위험해서"**뿐 아니라 **"다른 자세로는 안 뚫려서"**(기하) 선택될 수 있는데, 종료 조건이 전자만 가정했다.

**수정** (`SoldierEngagement`):

- `bBlindForCost` 기록 — 블라인드를 **위험 때문에** 골랐는지, **기하 때문에** 골랐는지. `bBlindNoLongerNeeded`는 위험 쪽에만(제압도가 떨어지면 종료). 기하 쪽은 표적 소실·포즈 도달 후에도 막힘(`bPlanFailed`)·`bBlindNoShot`으로만 종료.
- `bWorthShot`에 **앎 ≤ `SuppressiveKnowledgeRadiusCm 1000`** 조건 추가 — 제압사격조차 못 할 만큼 낡은 표적에 총만 들고 서 있을 이유가 없다. 같은 이유로 내민 뒤 앎이 나빠지면 `bBlindNoShot`으로 종료.
- **관찰 회차는 블라인드 자세로는 하지 않는다**(`bCanLookThisWay = bExposes`) — 총으로는 못 본다.

→ P132("종료 조건은 시작 이유별로").

---

## 4. 부정 증거 — 보고 있는데 아무도 없다 [A · 코드]

3절의 유령(익명 총성, 15 s)이 드러낸 결핍: 기록의 예상점을 **훤히 보고 있는데도** 확신이 소리의 반감기(8 s)대로만 낡는다.

**구현**: `SoldierSight`가 살아 있는 표적·시체 확인 뒤 **남은 예산으로**, 이번 틱에 갱신되지 않은 기록의 예상점(`PredictLocation`)이 콘·거리 안이면 눈→예상점 트레이스. **뚫리면** `USoldierPerceptionComponent::ReportClearView(index, dt)` → `ObservedCertainty × 0.5^(dt / ClearViewHalfLifeSeconds 1.5)`.

- 예상점에 **살아 있는 적**이 서 있으면 그 몸이 선을 막는다(Sight 채널이 폰을 막음) → 기록 그대로.
- 예상점이 **상자 뒤**면 상자가 막는다 → 그대로(거기 있을 수 있다).
- **빈 공터**만 깎인다. 관측을 관측대로 쓰는 것(P63)이다 — "봤는데 없더라"도 관측이다.

→ P133("부정 증거").

---

## 5. 시야 거리 — 처음 9 s의 무반응 [A · 로그]

**증상**(사용자): 시작 후 몇 초 동안 서로 반응이 없다.

**로그**: 첫 `[Engage]` 전이가 t=9.0, 첫 눈이 붙는 순간의 거리 **5973~5978 cm** — 정확히 `SightRangeCm 6000` 경계. 스폰 거리(~63 m)가 시야 밖이고 공격수가 목표로 전진해 60 m 안에 들어와야 첫 목격. 청각(100 m)만 그 전에 닿았다.

**판단**: 교전 상한이 **정지 95 m**(`ai/2026-09-13_engagement_and_cover.md` 4.1절)인데 시야가 60 m면 앞뒤가 안 맞는다. 공터의 사람은 그보다 훨씬 멀리서 보인다 — 거리에 따라 커져야 할 것은 **알아채는 시간**이지 "보이는가"가 아니고, 그건 아직 없다(→ [W56]).

**값**: `SightRangeCm 12000 (← 6000)` · `MaxTracesPerTick 5 (← 3)`(시체 확인·빈 땅 확인이 같은 예산을 쓴다) → [C-121].

---

## 6. 두 점 시야 — 가슴이 보여야 보인다는 맹점 [A · 코드]

**증상**(사용자, 빙의 관전): 담이 1 m 안팎인 이 맵에서 **머리·어깨만 담 위로 나온** 적이 내 눈에는 보이는데 AI는 안 쏘고 멀뚱거린다.

**코드 실측 — 트레이스의 양 끝**:

| 트레이스 | 시작점 | 대상점 |
|---|---|---|
| 시야 | 내 `head` 소켓(실측 ~140 cm) | 상대 **`spine_03` 한 점**(가슴, 97~105 cm) — 머리는 안 봄 |
| 엄폐 | 적 눈(기록 + 20) | 내 몸 가슴 높이 표본 97 → 55(3단) — **머리 높이 없음** |
| 사선 | 내 실제 총구 | 기록 위치(= 상대 가슴점) |

처음 설계는 "정수리만 넘어온 병사는 의미 있게 보이는 게 아니다"(`ai/2026-09-13_perception_stack.md` 1절)였다. 이 맵에서는 정확히 그 반대가 흔하다. 그리고 AI 자신의 엄폐도 **가슴만 숨기면 "숨었다"**라 머리를 내놓고 안심한다 — **보는 점과 숨기는 점이 같은 집합이라 서로 짝이 맞는 맹점**이었고, 사망이 들어오니 드러났다(→ P131).

**수정** — 눈과 가슴 **두 점**을 대칭으로:

1. **시야** `HasLineOfSight`: 상대 **가슴 → 막히면 머리**(`GetEyeLocation`) 순 2점. **보인 점이 기록 위치**가 되므로 머리만 보이면 **머리를 조준·사격**한다. 트레이스는 필요할 때만 2발(`OutTracesUsed`).
2. **엄폐** `EvaluatePosition`: 자세 3단(기립/중간/웅크림)마다 **가슴과 머리 두 점이 모두 모든 눈에서 가려져야 "숨음"**. 머리 높이는 `USoldierIdentityComponent::GetHeadHeightCm(stance)` — 기립 `head` 소켓 실측, **웅크린 `head` 소켓도 실측**(`MeasuredCrouchEyeHeightCm`; 측정 전엔 기립 머리-가슴 간격을 유지해 추정). 후보당 예산 `(3 + 2×heights + routes) × eyes`.
3. **시체 확인**도 같은 두 점 규칙.

**예상 귀결** [B]: (1) 서로를 훨씬 자주 보고 사격이 는다 (2) 병사들이 **더 깊이 숙인다**(`st` 0.5 → 1.0인 자리가 많아짐) (3) 1 m 담 뒤에서 웅크려도 머리(웅크림 눈 ~95 cm)가 담 위면 `OPEN`으로 읽혀 더 높은 담을 찾는다 — 사용자가 벽을 올려 둔 것이 여기에 유리하다.

---

## 7. 튜닝값 — 이번 문서 신설·변경 (전부 [C])

### 7.1 `USoldierCoverComponent` (Category `SoldierLab|Cover|Squad`)

| 값 | 기본 | ID |
|---|---|---|
| `ClaimRadiusCm` | **250** (신설) | [C-108] |
| `ClaimedCost` | **1.0** (신설) | [C-108] |
| `LaneClearanceCm` | **150** (신설) | [C-108] |
| `LaneCost` | **0.6** (신설) | [C-108] |
| `CoveredEyeFactor` | **0.5** (신설) | [C-108] |

### 7.2 `USoldierEngagementComponent`

| 값 | 기본 | ID |
|---|---|---|
| `TargetCrowdingPenalty` | **0.15** (신설) | [C-108] |
| `CoveringStillSpeedCms` | **20** (신설) | [C-108] |
| `bWorthShot` 조건 | 앎 ≤ `SuppressiveKnowledgeRadiusCm 1000` 추가 (값 그대로, 조건 변경) | — |

### 7.3 `USoldierPerceptionComponent`

| 값 | 기본 | ID |
|---|---|---|
| `ClearViewHalfLifeSeconds` | **1.5** (신설) | [C-109] |

### 7.4 `USoldierSightComponent`

| 값 | 기본 | ID |
|---|---|---|
| `SightRangeCm` | **12000** (← 6000) | [C-121] |
| `MaxTracesPerTick` | **5** (← 3) | [C-121] |

### 7.5 `USoldierIdentityComponent`

| 값 | 기본 | 비고 |
|---|---|---|
| 웅크린 머리 높이 | 실측(`MeasuredCrouchEyeHeightCm`), 측정 전 = 웅크림 가슴 + (기립 머리 − 기립 가슴) | 상수 없음 |

---

## 8. 판정 기준 — 다음 실측 [C]

| 대상 | 통과 | 실패 시 |
|---|---|---|
| 겹치는 자리 (S1) | `[Cover]` 로그에서 같은 2.5 m 안에 두 명의 `MOVE`/`stay`가 겹치는 시간이 이전 세션보다 줄었는가; `best … q1.00`이 실제로 다른 후보로 보내는가 | 안 줄면 `ClaimRadiusCm`·`ClaimedCost` |
| `MASKED` (S2) | `[Engage]`의 `MASKED` 체류 시간(이전 8~11 s) 감소 | `LaneCost`·`LaneClearanceCm` |
| 표적 분포 (S3) | 아군 셋의 `tgt`가 한 적에 몰리는 비율 감소 | `TargetCrowdingPenalty` |
| 엄호 이동 (S4) | `MOVE` 시각에 다른 아군의 `WantsToFire`가 켜져 있는 비율; 동시 출발(둘 다 `MOVING`, 아무도 사격) 빈도 | 동시 출발이 잦으면 타이브레이크(자리 비용·등록부 순) 재검토 |
| 유령 표적 | 사망 뒤 `tgt`에 죽은 이름이 남는 시간 — 죽인 쪽 < 1 s, 25 m 안 아군 ≤ 1.2 s + 입 대기 | 길면 시체 확인 예산·콘 |
| 블라인드 펄스 | `[Engage]` 전이 수가 병사당 수십 회 이하(693회 → ) | 다시 오르내리면 `bPlanFailed`/`LaneTolerance` 경계 |
| 부정 증거 | 익명 총성 유령이 공터에서 3~4 s 안에 `bel0` | 안 깎이면 콘·예산 |
| 시작 반응 | 첫 `[Engage]` 전이가 t≈9 s → 훨씬 이른 시점 | |
| 두 점 시야 | 머리만 보이는 적에 대한 `know 30`(머리점 목격) 발생, `st`가 1 m 담 뒤에서 1.0 | 없으면 `GetEyeLocation` 소켓·2점 순서 |
| 노는 병사 [W53] | 위 넷(유령·시야 거리·두 점·블라인드)으로 설명되는 비율 — [B] 상당 부분 | 남으면 `[Engage]` hold 게이트 집계 |

---

## 9. 변경 파일

```
AI/SoldierIdentity.{h,cpp}     claim · IsRegisteredActor · GetHeadHeightCm · 웅크린 눈 실측
AI/SoldierPerception.{h,cpp}   LearnDeath/IsKnownDead/TakeDeathToReport · ReportClearView · KnownDead
AI/SoldierSight.{h,cpp}        2점 HasLineOfSight · 시체 확인 · 빈 땅 확인 · 12000/5
AI/SoldierComms.{h,cpp}        BeginCasualtyTransmission · 사망 보고 우선·릴레이
AI/SoldierEngagement.{h,cpp}   PickCandidate/CountTeammatesOn/IsCoveringEnemy · bBlindForCost · bBlindNoShot
AI/SoldierCover.{h,cpp}        SquadCost · IsEnemyCoveredByTeammate · q 항 · 2점 엄폐 · 예산 산수
```

CL 471 · 472 제출(사용자). **두 점 시야(Identity/Sight/Cover/Perception/Engagement 일부)는 미제출.** BP 변경 없음. Camera/·Observer/·Pose/ 범위 밖.

---

## 10. 이 문서가 하지 않은 것 → `OPEN_ITEMS.md`

| 항목 | ID |
|---|---|
| **L0 분대 명령 층** — "특정 위치로 경계하며 이동 · 점령 후 방어" 같은 명령이 없다. 지금 있는 건 목표 마커 하나와 선호뿐 | **[W55]** |
| **거리 의존 발견 시간** — 120 m에서 즉시 보이는 것은 틀렸다; "보이나"가 아니라 "알아채는 데 몇 초"가 거리에 따라 커져야 | **[W56]** |
| 분대 항 값 · 부정 증거 반감기 · 시야 거리 귀결 실측 | [C-108] [C-109] [C-121] |
| 엄폐 자리 **예약**(진짜 예약이 아니라 주장 비용 — 절반) | [W51] 절반 |
| 분대 통신 — 사망 보고만(적 기록 공유는 전부터) | [W52] 절반 |
| 45명 성능 — 트레이스가 또 늘었다(시체·빈 땅·2점) | [C-83] |
