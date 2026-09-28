# 분대 명령 층 — `BreakContact` 동사 · 표적 제외 플래그의 두 소비자(RCWS 브리지 · 개인 교전)

2026-09-21 / 진행중(코드·빌드 완료 · New_kadex_0811 2차 PIE 판정 대기 [C-163]) / New_kadex_0811 첫 완주에서 3분대의 3차 "도주"가 엄폐 홉 후퇴였고(`Withdraw` + `ReturnFireOnly` 는 땅을 계속 싸울 값으로 매긴다) UGV 와 아군 보병이 제외된 분대를 계속 쐈다(RCWS 는 구 이펙트가 켜 주던 플래그가 안 켜졌고, SoldierLab 아군은 제외 플래그를 읽는 코드 자체가 없었다). 새 규칙 동사 `BreakContact`(땅의 가격에서 엄폐·위험·제압을 뺀다, 규칙이 아니라 가격)와 `IsContactExcluded`(시나리오 지시 — 인지 규칙 아님, P130 불변)로 고쳤다.

선행: `2026-09-17_command_layer_design.md`(동사·배정 계약) · `2026-09-18_squad_layer_fixes_quota_engage_range.md`(정원제·`EngageRangeCm`·사격 게이트 8절) · 레벨·DT·로그는 `../level_new_kadex_0811/2026-09-21_soldierlab_migration_new_kadex_0811.md`(이 문서의 "왜"는 그쪽 5절). 구 이펙트 `ExcludeFleeingEnemiesFromAllyTargeting`: `../ai_combat/2026-09-03_dynamic_squad_reassignment_and_casualty_log.md`(superseded on New_kadex_0811).

> 신뢰도: **[A]** 코드 실측(문서 세션이 2026-09-21 저녁에 `Source/` 를 다시 읽었다 — 줄 번호는 그 시점) · **[B]** 잠정 · **[C]** 미측정. ⚠ `SoldierEngagement`/`SoldierCover` 는 같은 날 다른 세션(성능·오버레이·사격 콘)이 편집한 판이라 이 문서에 없는 변경이 같은 파일에 있다 — 그쪽은 `ai/2026-09-21_*.md`.

---

## 0. 한 장 요약

| # | 무엇 | 어디 | 상태 |
|---|---|---|---|
| 1 | 새 동사 **`ESoldierOrderVerb::BreakContact`** + `FSoldierAssignment::bBreakContact` · `bTargetableByOwnSideWeapons` 주석을 실제 뜻으로 | `Squad/SoldierOrderTypes.h:44-49` · `:223-230` · `:232-241` | [A] |
| 2 | `BuildAssignment` case · 라벨 ` BREAK` / ` excl` | `Squad/SoldierSquadSubsystem.cpp:295-302` · `:36` · `:90-97` | [A] |
| 3 | 엄폐 — `IsBreakingContact()` · `ScorePosition` 이 Fighting/Route/Danger/Suppression 을 0 으로 · dwell 없음 | `AI/SoldierCover.h:148-154` · `.cpp:152-160` · `:1833-1845` · `:2048-2052` | [A] |
| 4 | 교전 — `IsContactExcluded(record)` 로 후보 제외·잠금 해제 · 이동 중 스프린트 · 자세 0 | `AI/SoldierEngagement.h:882-887` · `.cpp:411-417` · `:486-491` · `:681-686` · `:850-855` · `:1578-1582` | [A] |
| 5 | 시나리오 `IssueSquadOrderSpec`: `SetTargetable(false)` 가 UGV RCWS `bRespectEnemyTargetingExclusion=true` 도 켠다 | `titan_example/UI/ScenarioStateSubsystem.cpp:1914-1928` | [A] |
| 6 | `L_SoldierTest` 1v1 셋업(`Zone_AllyBase` · `ScenarioConfig_0` · `DT_ScenarioSteps_SoldierTest1v1` 5행) | `Content/SoldierLab/Levels/L_SoldierTest.umap` · `Content/Scenario/DT_ScenarioSteps_SoldierTest1v1.uasset` | [B] 6절 |
| 7 | 빌드 함정 — `UFUNCTION` 없는 private 헬퍼를 다른 컴포넌트에서 부르면 C2248 | 7절 | [A] |

판정 **[C-163]**(New_kadex_0811 2차 PIE). 원칙 **P184 · P185**.

---

## 1. 세 동사가 다른 것 — `Withdraw` · `HoldFire` · `BreakContact` [A]

첫 New_kadex 실행에서 "3분대 3차 도주" 를 `Withdraw z2 Rush ReturnFireOnly` 로 냈더니 3분대가 2차 존 주변 엄폐를 홉하며 아군과 사격을 주고받았다. 이름이 "Withdraw" 라 도주로 읽히지만 배정은 다음과 같다:

| 동사 | 배정 | 땅의 가격(엄폐 층) | 방아쇠 | 결과 |
|---|---|---|---|---|
| `MoveTo` / `Withdraw` | `Mode=Approach`, 존·속도·ROE 새로(`SoldierSquadSubsystem.cpp:253-278`) | 목표 거리 **+ 엄폐 + 경로 위험 + 제압 + 팀원** — 전투 이동 | ROE 대로 | 엄폐에서 엄폐로, 홉 사이 `MinDwellSeconds` 머무름, 쏘는 놈이 있으면 (`ReturnFireOnly`) 응사. **360 m 를 이렇게 가면 도주로 안 보인다** |
| `SetROE HoldFire` | ROE 만 바꿈(`:280-283`) | 위와 같음 | 안 당김 | "안 쏘는 전투 이동" — 여전히 엄폐를 골라 머문다 |
| **`BreakContact`** | 존 유지 · `bBreakContact=true` · ROE/사거리/속도 같이(`:295-302`) | **목표 거리 + 팀원 자리만**(`SoldierCover.cpp:1839-1845`) — 엄폐·위험·제압 항 0 | ROE 대로(보통 HoldFire 를 같이 준다) | 매 홉이 존에 가장 가까운 자리, 머무름 없음, 이동 중 스프린트, 서서 뛴다. 존 안에 들어오면 평소처럼 Hold |

**왜 ROE 가 아니라 가격인가(P184)** — "도주하면 쏘지 마라 + 뛰어라" 를 규칙으로 쓰면 `Rush` 가 이미 있고(`SoldierEngagement.cpp:1570-1574`, 이동 중만 스프린트) HoldFire 도 있는데 왜 안 됐는가를 설명 못 한다. 안 된 것은 **엄폐 층이 매 홉을 아직 싸울 자리로 값을 매긴 것**이다 — 쏘는 눈이 있으면 그 눈에서 숨는 자리가 싸고, 그 자리에 도착하면 dwell 이 붙고, 그 사이 `ReturnFireOnly` 가 열린다. 도주는 "어디로"가 아니라 "무엇을 셈에 넣지 않는가" 로 만들어야 한다. 그래서 동사 하나가 비용 항 넷을 0 으로 만들 뿐, 새 상태 기계는 없다(P89 — 선호지 규칙이 아니다).

`Withdraw` 는 그대로 둔다 — "싸우며 물러나는" 2차 도주(정원 10, ReturnFireOnly) 에는 맞는 동사다. 3차 도주가 `Withdraw`(정원 채우기 + 존 지정) → +6 s `BreakContact HoldFire Rush` → 트럭이 쏘면 `Occupy z2 Free` 세 행이 된 이유가 이것이다(레벨 문서 3절 `Squad3Run`/`Squad3Stand`).

---

## 2. 계약 — `SoldierOrderTypes.h` [A]

```cpp
// :44-49
/** Keep the zone and stop fighting for the ground on the way to it: run. Sets
 *  FSoldierAssignment::bBreakContact; carries the ROE and engage range like SetROE. The next
 *  zone verb clears it. */
BreakContact,

// :223-230  (주석 정정 — "own side" 는 이 병사의 편이 아니라 '시나리오가 보는 쪽')
/** Whether the scenario lets the OTHER side's weapons target this soldier at all - vehicles
 *  (through the detection bridge) and the enemy's own infantry (USoldierEngagementComponent
 *  skips such a contact as a target; it is still seen and still hidden from). A scenario
 *  directive, not knowledge: "the squad that is leaving is somebody else's fight now". */
bool bTargetableByOwnSideWeapons = true;

// :232-241
bool bBreakContact = false;
```

- 규칙 동사(`SetROE`/`SuppressArea`/`SetTargetable`/`BreakContact`)는 `Current` 를 복사해 한 가지만 바꾸고, 존 동사(`MoveTo`/`Occupy`/`Withdraw`)는 `FSoldierAssignment()` 에서 새로 시작하되 **`bTargetableByOwnSideWeapons` 만 이어받는다**(`SoldierSquadSubsystem.cpp:277`) — 제외는 존이 바뀌어도 남고, `bBreakContact` 는 존 동사가 지운다(기본값 false 로 초기화). "다음 존 동사가 지운다" 가 이 한 줄이다.
- `BreakContact` 는 시나리오 쪽에서 **구역 동사가 아니다**(`ScenarioStateSubsystem.cpp:1887-1890` `bZoneVerb` 에 없음) — `SquadIds` 비우면 진영 전체 한 번에(`:1931-1937`), `Quota` 는 무시. `ZoneIndex` 는 안 읽는다.
- 라벨(`SoldierSquadAssignmentLabel`, `:90-97`): `approach@z3 Withdraw r1500 roe=hold spd=rush agg=0.50 BREAK excl rev7` — ` BREAK` 는 `bBreakContact`, ` excl` 은 `!bTargetableByOwnSideWeapons`. `SoldierLab.Debug.Squad` 오버레이와 `[Squad]`/`[Cover]`/`[Engage]` 로그가 같은 문자열을 쓴다.
- 콘솔 `titan.SquadOrder` 는 `StaticEnum<ESoldierOrderVerb>()->GetValueByNameString` 으로 파싱하므로(`ScenarioStateSubsystem.cpp:77-93`) **`titan.SquadOrder Hostile 3 BreakContact 0 HoldFire Rush` 가 그대로 먹는다** — 주석·경고문(`:53` · `:93`)의 동사 목록에만 `BreakContact` 가 빠져 있다(문자열뿐, 동작 무관).

---

## 3. 엄폐 층 — `IsBreakingContact()` [A]

```cpp
// SoldierCover.cpp:152-160
bool USoldierCoverComponent::IsBreakingContact() const
{
    if (Identity == nullptr || !Identity->GetAssignment().IsActive()) return false;
    const FSoldierAssignment& Task = Identity->GetAssignment();
    return Task.bBreakContact && Task.Mode == ESoldierTaskMode::Approach;
}
```

- **`Mode == Approach` 조건** — 분대 층이 도착선 안에 들어온 순간 `Mode` 를 `Hold` 로 바꾼다(`SoldierSquadSubsystem.cpp:485-493`, `bBreakContact` 는 그대로 남음). 그 뒤 `IsBreakingContact()` 는 false → 존 안에서는 평소 Hold 가격(엄폐·순찰)으로 돌아간다. "도착하면 자리를 잡는다" 가 따로 코드 없이 나온다.
- **`ScorePosition`** (`:1833-1845`): `Cost.Fighting = Route = Danger = Suppression = 0`. 남는 것은 `Objective`(Approach 계단 `1 + 거리/ApproachScaleCm`, 09-18)와 `Squad`(자리 주장 `ClaimedCost` — 대열이 한 점에 겹치지 않게). 주석이 원인을 남긴다: *"(2026-09-21: the third squad, told to withdraw under ReturnFireOnly, hopped from cover to cover trading fire with the allies and never left the second zone.)"*
- **dwell 없음** (`:2048-2052`): `bDwelling = !IsBreakingContact() && …` — `MinDwellSeconds`(엄폐 있을 때)도 `ScanDwellSeconds`(눈 0 도착 머무름, 09-18)도 안 붙는다. 도착 즉시 다음 스윕이 다음 홉을 정한다.
- 후보 생성·스윕·`RejectCandidate`·엣지 전진(09-21 AI 세션)은 안 건드렸다 — 엣지 전진은 `bAdvancingNow && !bHasContact` 일 때 걷는데(`SoldierEngagement.cpp:1588-1594`), 도주 중 접촉이 있으면 안 걸리고 접촉이 없는 구간에서는 걸릴 수 있다 [C-163 에서 볼 것 — 도주 중 `ADVANCE begins` 가 찍히면 `IsBreakingContact()` 를 그 게이트에도 넣는다].

---

## 4. 교전 층 — `IsContactExcluded` · 스프린트 · 자세 [A]

```cpp
// SoldierEngagement.cpp:411-417
bool USoldierEngagementComponent::IsContactExcluded(const FSoldierEnemyRecord& Record)
{
    const AActor* Enemy = Record.Enemy.Get();
    const USoldierIdentityComponent* EnemyIdentity =
        Enemy != nullptr ? Enemy->FindComponentByClass<USoldierIdentityComponent>() : nullptr;
    return EnemyIdentity != nullptr && !EnemyIdentity->GetAssignment().bTargetableByOwnSideWeapons;
}
```

| 자리 | 코드 | 뜻 |
|---|---|---|
| 후보 선정 | `PickCandidate` 루프 첫 줄 `if (IsContactExcluded(Record)) continue;` (`:486-491`) | 제외된 적은 점수도 안 매긴다 |
| 잠금 | `if (LockedCertainty < MinCertaintyToEngage \|\| IsContactExcluded(Locked)) bLockedStillHeld = false;` (`:681-686`) | 제외 명령 순간 이미 물고 있던 표적을 놓는다 — 09-14 표적 잠금(`ai/2026-09-14_danger_map_and_position_commitment.md`)의 예외 하나 |
| 스프린트 | `if (Task.bBreakContact && Cover && Cover->IsMovingToCover()) bWantsToSprint = true;` (`:1578-1582`) | Speed 가 뭐든 이동 중엔 뛴다(`Cautious` 도 덮음) |
| 자세 | `if (Task.bBreakContact && Cover && Cover->IsBreakingContact()) DesiredStance = 0.f;` (`:850-855`) | 서서 뛴다. 재장전(`bWantsToReload`)의 웅크림은 그 아래에서 여전히 강제 |

- **인지는 그대로**(P185) — `IsContactExcluded` 는 `SoldierPerception` 기록에 손대지 않는다. 제외된 적도 보이고, 그 눈에서 숨고(`SweepEyes` 에 남음), 위협 보너스(`ReportThreatenedBy`)도 쌓인다. 시야 콘이 유령을 향하던 P130 의 문제는 이 경로로 재현되지 않는다 — **표적 목록에서만 빠진다.** "제외 = 시나리오 지시" 이지 "죽었다/무해하다" 가 아니다.
- `static` 인 이유: 자기 상태를 안 읽고 상대 액터만 본다 — 다른 컴포넌트(엄폐)가 같은 판정을 쓰게 될 때를 위해 public static.
- 표적 액터가 `USoldierIdentityComponent` 를 갖지 않으면(차량, 구 BP 병사) 항상 false — 차량은 브리지가 Identity 를 다는데 그 배정은 기본값(`bTargetableByOwnSideWeapons=true`)이라 무해.
- ⚠ **`MinStance` 와의 순서** — `DesiredStance=0`(`:852-855`) 바로 뒤에 09-18 의 `MinStance` 바닥(`:857-862`, `Speed != Rush` 일 때)이 온다. `BreakContact` 를 `Cautious` + `MinStance>0` 인 배정 위에 얹으면 바닥이 자세를 도로 올린다. DT 는 `Rush` 를 주고 titan DT 엔 `MinStance` 필드 자체가 없어([W84]) 지금은 안 겹치지만, `MinStance` 를 DT 에 연결할 때 `bBreakContact` 면 바닥을 건너뛰게 할 것 [B].

---

## 5. 제외 플래그의 두 소비자 — 왜 둘인가 [A]

```
FSoldierAssignment::bTargetableByOwnSideWeapons (SetTargetable 동사가 씀)
   ├─ ① titan 브리지 SyncSoldiers (SoldierLabBridgeSubsystem.cpp:200-201, 매 틱)
   │      → UDetectableTargetComponent::SetTargetableByFriendlyForces(값)
   │      → URCWSFireControlComponent::SelectNearestEnemyTarget (RCWSFireControlComponent.cpp:449-460)
   │           if (bRespectEnemyTargetingExclusion) { 구 EnemyCombat 플래그 || Detectable 플래그 → continue }
   │      ⚠ bRespectEnemyTargetingExclusion 은 인스턴스 기본 false (.h:685-690) —
   │        UGV 만 켜고 트럭은 안 켠다("도망쳐 오는 분대와 싸우는 게 그쪽 역할")
   │        → 켜는 곳: 구 이펙트 ExcludeFleeingEnemiesFromAllyTargeting (ScenarioStateSubsystem.cpp:1502-1509)
   │                   신 IssueSquadOrderSpec SetTargetable(false)      (:1914-1928)   ← 이번에 추가
   └─ ② USoldierEngagementComponent::IsContactExcluded (4절)   ← 이번에 추가
          SoldierLab 보병(아군)이 직접 읽음 — 브리지·탐지 컴포넌트 안 거침
```

- ①은 09-17 에 이미 있었다(`../level_new_kadex_0811/2026-09-17_soldierlab_squad_scenario_link.md` 1절 표). 빠진 것은 **RCWS 가 그 플래그를 보게 하는 스위치** — 구 이펙트가 두 가지를 한 몸으로 했는데 `SetTargetable` 동사로 옮기면서 반쪽만 옮겼다. 첫 New_kadex 실행에서 `ExcludeFleeingEnemies` 발동 뒤에도 `[RCWSFireControl] BP_UGV_0901_C_1: 타겟 BP_Soldier_Hostile_C_4/6/3 …` 이 이어진 것이 증거(레벨 문서 5절).
- ②는 없었다 — 시험 레벨에는 이 단계에 아군이 안 끼어서 드러나지 않았고, New_kadex 에서는 3분대 3차 경로가 아군 거점 옆을 지나 아군이 계속 쐈다.
- 트럭은 여전히 **둘 다 안 지킨다** — ①은 플래그가 false, ②는 트럭에 `USoldierEngagementComponent` 가 없다. 3차 = 트럭의 교전이라는 시나리오 의도 그대로.
- 리플리케이션: `bRespectEnemyTargetingExclusion` 은 EditAnywhere 프로퍼티(복제 지정 없음) — 서버에서 `IssueSquadOrderSpec` 이 켜면 서버 RCWS 가 지킨다. 표적 선정은 서버 전용이라 충분 [B]. 2-PC 는 [C-125].
- ⚠ **문서 세션 발견 — RCWS 의 스티키 표적은 제외를 안 본다** [A]: `UpdateAutoAim` 의 유지 조건(`RCWSFireControlComponent.cpp:584-613`)은 "`DetectedTargets` 에 아직 있는가"(액터·진영만, `:586-590`)뿐이라 **제외 순간 이미 물고 있던 적은 시야를 `TargetRetentionGraceSeconds 1` 넘게 잃어야 놓고**, 그 뒤 `SelectNearestEnemyTarget` 에서야 제외가 먹는다. ①의 스위치를 켜도 그 한 명은 사선이 끊길 때까지 맞는다. 구 이펙트 시절부터 같은 틈이고, 도주 병사가 스프린트로 곧 사선을 끊어 실무에선 짧을 것 [B]. 보병 쪽(②)은 잠금 해제(`:683`)로 이 틈이 없다. 막으려면 `:613` 의 조건에 제외 검사 한 줄 → **[W106]**(RCWS 몫). [C-163] 판정 ③ "UGV 타겟 줄 0" 은 이 틈만큼 늦게 0 이 될 수 있다.

---

## 6. `L_SoldierTest` 1v1 셋업 [B]

`L_SoldierTest`(`Content/SoldierLab/Levels/L_SoldierTest.umap`, 2026-09-18 18:04 저장본)에 시나리오를 얹어 "적 1명이 아군 거점으로 잠입" 을 명령으로 돌리는 셋업. DT `DT_ScenarioSteps_SoldierTest1v1.uasset`(2026-09-18 09:35) — 2026-09-21 00:59 UTC 로그에 발동 기록이 있다(`AllyGuard` · `EnemyInfiltrate` · `ScenarioComplete`). ⚠ 세션 기록은 "오늘(09-21) 만든 것" 으로 적었으나 **에셋 mtime 은 09-18** — 09-18 세션이 만들고 09-21 에 돌린 것으로 본다. 이 문서 외에 기록이 없어 여기 남긴다.

| 무엇 | 값 |
|---|---|
| 존 | `Zone_AllyBase`(`ASoldierZone`) — 아군 거점. Friendly → z0 |
| `ScenarioConfig_0` | Demo 자동 시작, `SquadZones` Friendly/Hostile 각 `[Zone_AllyBase]` (추정 — Hostile 의 목적지가 같은 존) |
| DT 5행 | `AllyGuard`(+1 s, Friendly Occupy z0 Free agg 0.4 — 로그 라벨 "아군 거점(Zone_AllyBase) 점유·주변 경계") · `EnemyInfiltrate`(+1 s, Hostile MoveTo z0 Cautious HoldFire agg 0.3 — "적군 은밀 침투") · `EnemyAssault`(`AllyFireStarted` → Hostile Free/Rush) · `EnemyArrived`(`SquadOrderAchieved`) · `ScenarioComplete`(`AllEnemiesEliminated`) |
| DT 에 든 문자열 [A] | `MoveTo`/`Occupy` · `Free`/`HoldFire` · `Cautious`/`Normal`/`Rush` · `AllyFireStarted`/`SquadOrderAchieved`/`AllEnemiesEliminated`/`TimerOnly` · `EngageRangeCm`/`Quota`/`bTargetable` |

용도: 잠입 거동(`ai/2026-09-17_infiltration_and_unknown_ground.md`)을 명령으로 재현 — "시나리오는 이미 `MoveTo r1200 roe=hold spd=cautious agg=0.30`" 이라고 09-18 밤 문서가 적은 그 명령이 이 DT 다.

---

## 7. 빌드 함정 [A]

`USoldierCoverComponent::IsBreakingContact()` 를 처음 private 에 두고 `SoldierEngagement.cpp` 에서 부르자 **C2248**(private 멤버 접근). `UFUNCTION` 이 없는 헬퍼는 리플렉션이 접근 지정자를 안 풀어 준다 — 다른 컴포넌트가 읽을 접근자는 public 블록에(`SoldierCover.h:153-154`, `BlueprintPure` 로 노출). 09-18 의 C4458/`SetLineThickness()` 와 같은 줄에 적어 둔다.

---

## 8. 미해결 → `OPEN_ITEMS.md`

| ID | 항목 |
|---|---|
| **[C-163]** | New_kadex_0811 2차 PIE — `Squad3Run` 뒤 3분대 `[Engage] roe=hold` + 스프린트, `[Cover]` 홉이 존 방향만, UGV `[RCWSFireControl] … 타겟 BP_Soldier_Hostile_*` 0줄, 아군 `[Engage] tgt=` 에 3분대 없음, 트럭 사격 → `Squad3Stand` → 3분대가 트럭에 응사. 도주 중 `ADVANCE begins` 가 찍히면 엣지 전진 게이트에 `IsBreakingContact()` 추가(3절) |
| **[W106]** | RCWS 스티키 표적이 제외를 안 본다(5절) — `UpdateAutoAim` 놓아주는 조건에 `IsTargetableByFriendlyForces`/`IsTargetableByAlliesAndUGV` 검사 한 줄(`bRespectEnemyTargetingExclusion` 일 때만) |
| [W84] | `MinStance` titan DT 연결 — 연결할 때 4절의 순서 주의(`bBreakContact` 면 바닥 건너뜀) |
| [W104] · [W105] · [Q51] · [C-164] | 레벨 문서 8.2절 |
