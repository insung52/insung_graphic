# SoldierLab 3단계 시나리오 시험 레벨 `L_SoldierScenario` + DT `DT_ScenarioSteps_SquadThreeStage`

2026-09-18 / 진행중(레벨·DT 저작 완료 · 첫 PIE 여러 회 · 흐름 완주 검증 대기) / 드론 없는 SoldierLab 병사 35명(적 15 · 아군 20) + UGV + 트럭으로 3단계 시나리오를 `IssueSquadOrder` 행 13개로 다시 저작한 시험 레벨. 첫 PIE에서 드러난 문제(스폰 지점에서 UGV 사격 · 도착 미완료 · 차량이 분대원으로 잡힘)의 코드 수정은 `../soldier_ai_lab/squad/2026-09-18_squad_layer_fixes_quota_engage_range.md`. 1차 전투지 위치가 아군과 너무 가까운 문제는 **열려 있음**(사용자 결정).

선행: `2026-09-17_soldierlab_squad_scenario_link.md`(시나리오 ↔ SoldierLab 연결 — 이 문서가 그 4·5절 "시험 레벨 계획"의 실행 기록) · `../soldier_ai_lab/squad/2026-09-17_command_layer_design.md`(명령 층 설계) · `scenario_authoring_guide.md`(DT 저작 일반) · `scenario_three_stage_combat.md`(구 BP 병사 기준 구현 현황) · `2026-09-15_demo_ugv_autofire_on_zone1_arrival.md`(`SetDemoUGVAutoFire`).
구 정원제/대타 기능(이번에 SoldierLab용으로 재구현): `../ai_combat/2026-09-03_dynamic_squad_reassignment_and_casualty_log.md`.

> 신뢰도: **[A]** 코드·에셋 실측 · **[B]** 잠정 · **[C]** 미측정. DT 행 값은 MCP `DataTableTools`로 쓴 것을 세션 기록에서 옮겼다 — 에디터에서 다시 열어 확인하지 않았다 [B].

> ★ **2026-09-21 추기** — 7절 "New_kadex_0811 적용 시"가 실행됐다 → **`2026-09-21_soldierlab_migration_new_kadex_0811.md`**(본 레벨 SoldierLab 이관 · 존 8개 · DT 26행 · 첫 완주 · 3분대 도주 문제). 시험 레벨 쪽 변경: 아군이 4분대 → **3분대(7/7/6)** 로 재편됐고(09-21 AI 세션, `MaxSquadsPerFaction 3`) 이 DT 의 아군 분대 '4' 참조는 [W94]. 본 레벨 DT 는 `EnemyEngage` 트리거를 `UGVFiredNearEnemy`(접근이 HoldFire 라 적이 먼저 쏠 일이 없음)로, 3차 도주를 `Withdraw` → `BreakContact` → `Occupy` 세 행으로 저작했다 — 3.1절의 "3차도 `ReturnFireOnly` 한 행" 판단은 **본 레벨에서 절반만 맞았다**(안 쏘는 건 되지만 도주가 안 됨).

---

## 0. 한 장 요약

| 무엇 | 값 |
|---|---|
| 레벨 | `Content/SoldierLab/Levels/L_SoldierScenario.umap` (2026-09-17 16:47 저장본) — `L_SoldierTest` 복제 기반, 평지 |
| 병사 | 적 `BP_Soldier_Hostile` 15 = 분대 `1`/`2`/`3` × 5 (남서 모서리) · 아군 `BP_Soldier_Friendly` 20 = 분대 `1`~`4` × 5 (중앙 바위 무더기) — 각 인스턴스 `AC_SoldierIdentity.SquadId` |
| 차량 | `BP_UGV_0901` · `BP_TitanTruck`(이동형지휘소) · `TargetPoint` `ugvpoint1`(1차 집결) / `ugvpoint2`(2차) |
| 전투지 | `ASoldierZone` 4개 — `Zone_Hostile_0_Engage`(태그 **`EnemyCube`** — 데모 자동 시작용) · `Zone_Hostile_1_Withdraw` · `Zone_Hostile_2_Escape` · `Zone_Friendly_0_Defend`(r = 20 m) |
| `ScenarioConfig_1` | RunMode **Demo** · 자동 시작 3 s(`DemoAutoStartDelaySeconds` 기본값) · `SquadZones`: Hostile 1/2/3 → [Z0, Z1, Z2] 공통, Friendly 1~4 → [Z_F0] · `UGVFormUpDestination = ugvpoint1` · `UGVZone2Destination = ugvpoint2` · `CommandPost = BP_TitanTruck` · **`bDemoForceCommandPostAutoFire = false`**(기본 true) · **`bSoldierLabHostilesStartHidden = false`**(기본 true) · `ScenarioStepTable = DT_ScenarioSteps_SquadThreeStage` |
| DT | `/Game/Scenario/DT_ScenarioSteps_SquadThreeStage` **13행** (2026-09-17 16:52 저장본) — `DT_ScenarioSteps_ThreeStage`(17행) 에서 드론 6행 · Retarget 2행 · HoldFleeingFire 1행을 뺀 뒤 적/아군 행을 `IssueSquadOrder`로 교체 |
| 드론 | 없음 — 드론 행·`RevealEnemies` 없이 적군이 처음부터 보인다(`bSoldierLabHostilesStartHidden=false`) |
| 상태 | 첫 PIE 수 회. 코드 수정 뒤 **흐름 완주(ScenarioComplete)는 아직 못 봤다** [C] |

레벨 파일 안에서 실측 확인한 것 [A]: `SoldierZone` 3회 · 위 4개 존 이름 · `ugvpoint1/2` · `BP_TitanTruck` · `BP_UGV_0901` · `DT_ScenarioSteps_SquadThreeStage` · `EnemyCube` · `ScenarioConfig` 문자열이 `.umap`에 있다.

---

## 1. 왜 시험 레벨인가 (다시)

`2026-09-17_soldierlab_squad_scenario_link.md` 5절의 계획 그대로 — New_kadex_0811(숲·26% 스케일·드론·낙하산)에서 바로 하면 무엇이 분대 층 문제이고 무엇이 레벨 문제인지 갈라지지 않는다. 평지 + 바위 무더기 하나면 `[Squad]`/`[Engage]`/`[적 사상]` 로그가 그대로 판정 근거가 된다. 시나리오 시스템(`UScenarioStateSubsystem`)은 titan PlayerController 없이도 `RunMode=Demo` + `EnemyCube` 태그 액터만 있으면 자동 시작한다(같은 문서 4절 체크리스트 1·2번).

---

## 2. `ScenarioConfig_1` — 기본값과 다르게 둔 두 플래그

| 플래그 | 값 | 이유 |
|---|---|---|
| `bDemoForceCommandPostAutoFire` (`ScenarioConfig.h:108`, 기본 `true`) | **false** | 켜 두면 데모 시작 순간부터 트럭 RCWS가 자동사격 상태라, 평지에서 **보이는 적을 t=0부터 쏜다**(New_kadex_0811 은 숲과 낙하산 숨김이 가려 줬다). 3차 전투지 행 `CommandPostFire`(11행)에서 명시적으로 켠다 |
| `bSoldierLabHostilesStartHidden` (`:153`, 기본 `true`) | **false** | 드론이 없으니 `RevealEnemies`를 부를 행이 없다. 숨긴 채면 UGV RCWS 탐지 레지스트리에 안 잡혀 `UGVFiredNearEnemy` 계열이 영영 안 뜬다 |

나머지는 기본값. `SquadZones` 는 분대별 행 하나씩(Hostile 1 · 2 · 3 각각 `Zones=[Z0,Z1,Z2]`, Friendly 1~4 각각 `[Z_F0]`) — 세 적 분대가 같은 존 배열을 공유하는 것은 시험 레벨 단순화이고, New_kadex_0811 에서는 분대별로 다른 존을 준다([W70]).

---

## 3. DT `DT_ScenarioSteps_SquadThreeStage` — 13행 [B]

`ThreeStage`(`scenario_authoring_guide.md` 4절 17행) 대비: 드론 6행(UAVTakeoff/Patrol/…/RevealEnemies)·`RetargetToAllies`·`RetargetToCommandPost`·`HoldFleeingFire` 제거, 적/아군 행은 `IssueSquadOrder`. UGV·트럭·`ScenarioComplete` 행은 이펙트 그대로.

| # | 행 | Prereq | Trigger | Effect / `SquadOrder` |
|---|---|---|---|---|
| 1 | `EnemyApproach` | — | TimerOnly +1 s | Hostile · `*` · **MoveTo** z0 · **Cautious** · **Free** · **`EngageRangeCm 4000`** · stagger 0~2 s |
| 2 | `UGVMoveZone1` | — | TimerOnly +1 s | (구 행 그대로) UGV → `UGVFormUpDestination` |
| 3 | `UGVArriveZone1` | — | `ActorStopped` | `SetDemoUGVAutoFire` (`2026-09-15_demo_ugv_autofire_on_zone1_arrival.md`) |
| 4 | `EnemyEngage` | — | **`EnemyFireStarted`** (신규) | Hostile · `*` · **Occupy** z0 · Rush · Free · agg **0.8** |
| 5 | `EnemyFleeToZone2` | — | `EnemyCasualtyCountAtLeast` **3** | Hostile · `2,3` · **Withdraw** z1 · Rush · **ReturnFireOnly** · **`Quota 10`** · stagger 0.5~3 s |
| 6 | `UGVMoveZone2` | `EnemyFleeToZone2` | `LeaderDistanceFromEnemyAtLeast` 5500 | UGV → `UGVZone2Destination` |
| 7 | `UGVSpeedLimitOn` | (구 행 그대로) | | |
| 8 | `AllyDefend` | — | TimerOnly +1 s | Friendly · `*` · **Occupy** z0(Z_F0) · **ReturnFireOnly** · agg **0.3** · **`EngageRangeCm 5000`** |
| 9 | `AllyEngage` | — | **`EnemyNearFriendlySoldiers`** 8000 (신규) | Friendly · `*` · Occupy z0 · **Free** · agg 0.5 |
| 10 | `EnemyFleeToZone3` | `EnemyFleeToZone2` | `EnemyCasualtyCountAtLeast` **7** | Hostile · `3` · Withdraw z2 · Rush · ReturnFireOnly · **`Quota 5`** |
| 11 | `CommandPostFire` | (`EnemyFleeToZone3` 추정 — 세션 기록에 Prereq 미기재, DT 에서 확인) | TimerOnly +3 s | (구 행 — 트럭 RCWS 자동사격 ON) |
| 12 | `ExcludeSquad3` | (〃) | TimerOnly +4 s | Hostile · `3` · **SetTargetable false** |
| 13 | `ScenarioComplete` | `EnemyEngage` | `AllEnemiesEliminated` | (구 행) |

### 3.1 17행 → 13행에서 빠진 것과 그 이유

| 구 행 | 왜 필요 없나 |
|---|---|
| `RetargetToAllies` / `RetargetToCommandPost` | SoldierLab 에는 표적 선호가 없다(설계 결정 3). 표적은 **위협 기반** — 확신 최대 + 나를 쏜 놈 보너스(`ReportThreatenedBy`, `ai/2026-09-17_threat_bonus_lane_denied_vehicle_heights.md`). 아군이 쏘기 시작하면 아군이, 트럭이 쏘면 트럭이 위협이 되어 알아서 넘어간다 [B] → [C-122] |
| `HoldFleeingFire` | `ReturnFireOnly` 의 정의가 "**최근 3 s 안에 나를 향해 쏜 접촉에게만**"(`SoldierEngagement.cpp:952-955`, `ReturnFireWindowSeconds 3`)이라 도주 중 조용히 뛰다가 트럭이 쏘면 그 트럭에게 응사가 **저절로** 열린다. 3차 도주를 `HoldFire`로 두면(09-17 저작안) 트럭 사격에 응사가 안 열리므로 **이번 DT는 3차도 `ReturnFireOnly`** — 09-17 안의 "3차 = HoldFire + 트럭 사격 시 SetROE Free" 2행이 1행으로 줄었다 |
| 드론 6행 + `RevealEnemies` | 시험 레벨에 드론 없음. 적군은 처음부터 보임 |

### 3.2 새 필드·트리거가 이 DT에서 하는 일

- **`EngageRangeCm 4000`(1행)** — 첫 PIE 에서 적 5/6/7/10 번이 **스폰 지점에서 92 m 밖 UGV에 단발 사격**을 시작했고(시야 120 m 안 + 소총 산포 3°의 제압 가치 한계 ≈ 95 m 안쪽), 그 총성이 t≈7 s 에 4행 `EnemyFireStarted`를 당겨 **접근 단계가 통째로 사라졌다.** 40 m 밖에는 방아쇠를 막는다(조준·엄폐는 그대로). 상세 `../soldier_ai_lab/squad/2026-09-18_squad_layer_fixes_quota_engage_range.md` 4절.
- **`EnemyFireStarted`(4행)** — 09-17 안의 `UGVFiredNearEnemy` 를 대신한다. "UGV 든 적군이든 먼저 쏘는 쪽이 교전 개시". 지금 DT 에는 `UGVFiredNearEnemy` 짝 행이 없다 — UGV 가 먼저 쏘면 적군은 피격/근접탄으로 `ReportThreatenedBy` 가 서고 `Free` ROE 라 응사하며, 그 응사가 이 트리거를 당긴다 [B].
- **`Quota 10`(5행) / `Quota 5`(10행)** — 구 `BeginEnemyFleeToZone` 의 정원제·대타(`ai_combat/2026-09-03_…`)를 SoldierLab 용으로 재구현한 것. 2·3분대 생존자 합이 10 미만이면 1분대에서 z1 에 가장 가까운 병사를 `SquadId` 를 바꿔 편입한다. 로그 `[Squad] reinforce Hostile quota=10 living=N needed=M moved=K : Name(1->2), …`.
- **`EnemyNearFriendlySoldiers 8000`(9행)** — "아군은 경계만 하다가 적이 80 m 안에 오면 교전". 살아있는 적/아군 **보병**(`USoldierHealthComponent` 보유) 사이 최단 XY 거리. 차량은 안 센다.

---

## 4. 예상 흐름과 시간 축 [B]

```
t=3 s   데모 자동 시작 → 1·2·8행 (+1 s)
        적 15명 z0 로 Cautious 접근(stagger 0~2 s) · UGV 출발 · 아군 Z_F0 점령(ReturnFireOnly, 50 m 안만)
~2 min  적군 접근 — 스윕 반경 12 m + 정착 3 s(`MinDwellSeconds`) 홉이라 15명이 z0 에 다 들어오는 데 약 2분 [C]
        UGV 가 ugvpoint1 도착 → 3행 SetDemoUGVAutoFire
첫 총성 4행 EnemyEngage — 전원 Occupy z0 Rush Free agg 0.8
사망 3  5행 — 2·3분대(+1분대 대타로 10명) z1 로 Withdraw, ReturnFireOnly
        6행 UGV 2차 이동(적과 55 m 이상 벌어지면) · 7행 속도제한
적↔아군 80 m 9행 AllyEngage — 아군 Free
사망 7  10행 — 3분대(+대타 5명) z2 로
+3 s    11행 트럭 RCWS ON → 트럭이 쏘면 3분대 ReturnFireOnly 가 트럭에 응사
+4 s    12행 3분대 SetTargetable false (UGV·아군 표적 제외, 트럭은 무시)
전멸    13행 ScenarioComplete
```

### 4.1 ⚠ 열린 기하 문제 — Z0 가 아군과 54 m

`Zone_Hostile_0_Engage`(r = 15 m) 중심이 아군 배치점에서 **약 54 m**, 아군은 Z_F0 r = 20 m 에 흩어지므로 가장 가까운 적↔아군이 **~20 m** 까지 붙는다. 시야 120 m(`SoldierSight.h:50` `SightRangeCm 12000`)라 **1차 교전에 아군이 즉시 끼어든다** — "1차 = UGV 단독"이 안 나온다. `EnemyNearFriendlySoldiers 8000`(9행)도 접근 단계에서 이미 참이다. **Z0 를 아군에서 ≥130 m 로 옮겨야** 한다(아군 시야 밖 + 9행 문턱 밖). 아직 안 옮겼다 — 사용자 결정 대기. 옮길 때 `EngageRangeCm 4000`(1행)은 그대로 둬도 된다(접근 중 40 m 안 접촉만 쏨).

### 4.2 페이싱

15명이 z0 까지 홉으로 가는 데 ~2분은 데모로는 길다. 손잡이: `ASoldierZone::ApproachScaleCm`(3000 → 낮추면 기울기가 가팔라 한 홉이 길어짐) · 스윕 반경(`SoldierCover` `SearchRadiusCm 1200`) · `MinDwellSeconds 3` · 시작 위치를 z0 에 가깝게. `Rush` 는 접촉 없이는 안 뛰므로(09-17 잠입 규칙, `SoldierEngagement.cpp:1377-1379`) 속도로는 못 줄인다 — 실측 후 결정 [C].

---

## 5. 검증 체크리스트 (다음 PIE)

콘솔(PIE 시작 후): `SoldierLab.Debug.Squad 1` · `SoldierLab.Debug.Squad.Log 1` · `SoldierLab.Debug.Engagement.Log 1` · `SoldierLab.Debug.Zone 1` · `SoldierLab.Debug.AI.Self 0`.

| 순서 | 로그로 볼 것 | 통과 |
|---|---|---|
| 1 | `[ScenarioStateSubsystem] 시나리오 스텝 발동: EnemyApproach` 뒤 `[Squad] order #1 … MoveTo … r=1500` | 15명 대상 (차량이 `Friendly/(none)` 로 끼면 실패 — `bTakesSquadOrders` 수정 확인) |
| 2 | 접근 중 `[Engage] … roe=free/4000 … -> Restrained` | 40 m 밖 UGV 에 Restrained 만 찍히고 사격 없음. `EnemyFireStarted` 가 UGV 도착 전에 뜨면 실패 |
| 3 | `[Squad] reached` 15줄 → `[Squad] achieved Hostile/1 …` ×3 | 존 안(녹색 도착선 안)에서 멈춤. 도착선 밖에서 서 있으면 도착 불일치 재발 |
| 4 | `EnemyEngage` 발동 시각 vs UGV `SetDemoUGVAutoFire` 시각 | UGV 도착 이후 |
| 5 | `[적 사상] +Xs \| 누적 3명` → `[Squad] reinforce Hostile quota=10 living=… moved=…` → `[Squad] order … Withdraw` | moved = 10 − (2·3분대 생존) |
| 6 | `[Squad] reinforce … quota=5` (10행) · `Warning … (faction exhausted)` 여부 | 3분대 5명 확보 |
| 7 | `[Engage] tgt=BP_TitanTruck…` (3분대, 트럭 사격 후) | ReturnFireOnly 가 트럭에 열림 — [C-122] |
| 8 | `ScenarioComplete` 발동 | 완주 |

판정 ID: [C-122]~[C-124](`../soldier_ai_lab/OPEN_ITEMS.md`) + 이번 신설 [C-144] 도착선 · [C-145] EngageRange 접근 단계 보존.

---

## 6. 저작 함정 (MCP, 이 세션에서 밟은 것)

| 함정 | 증상 | 대응 |
|---|---|---|
| `DataTableTools.set_rows` 에 **중첩 구조체를 부분만** 쓰면(`squadOrder: {EngageRangeCm: 4000}`) | 그 구조체의 **나머지 필드가 기본값으로 리셋**된다(Verb None, ROE Free, Aggression 0.5 …) | `SquadOrder` 는 항상 **통째로** 쓴다 |
| `ObjectTools.set_properties` 로 원소가 이미 있는 구조체 배열(`SquadZones`) 갱신 | `ArrayAdd: elements changed alongside the size change` | `[]` 로 비운 뒤 전체 배열을 한 번에(`soldier_ai_lab/CLAUDE.md` 6.1c 와 같은 규칙) |
| `SceneTools.add_to_scene_from_*` | **PIE 실행 중이면 거부** | PIE 끄고 배치 |
| `AssetTools.save_assets` | `.uasset` 이 읽기 전용(Perforce)이면 **false 를 조용히** 돌려준다 | `p4 edit` 먼저, 디스크 mtime 으로 확인. `DT_ScenarioSteps_SquadThreeStage.uasset` 16:52 · `L_SoldierScenario.umap` 16:47 이 마지막 저장 |

---

## 7. New_kadex_0811 적용 시 (→ [W70])

시험 레벨 완주 후: 적 15·아군 25 를 SoldierLab 로 교체, 분대별 `ASoldierZone`(이제 에디터에서 반경·도착선·섹터가 보인다), `SquadZones`, DT 는 이 13행에 드론 6행 + `RevealEnemies` 를 다시 얹고 `bSoldierLabHostilesStartHidden=true`. `EnemyNearFriendlySoldiers` 문턱은 숲 시야에 맞춰 재측정.
