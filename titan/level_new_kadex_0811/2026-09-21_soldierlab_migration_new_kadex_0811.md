# New_kadex_0811 → SoldierLab 병사 이관: 구 BP 병사 40명 교체 · `ASoldierZone` 8개 · DT `DT_ScenarioSteps_ThreeStage_SoldierLab` 26행 · 첫 완주

2026-09-21 / 진행중(레벨·DT 저작 완료 · 첫 PIE 전 체인 순서대로 완주("아주 잘됨") · 3분대 도주 문제의 코드 수정 뒤 2차 PIE 대기) / 본 레벨의 적 15·아군 25를 `BP_Enemy_kadex`/`BP_Ally_kadex`에서 `BP_Soldier_Hostile`/`BP_Soldier_Friendly`로 바꾸고, 마커 군집 위에 분대별 `ASoldierZone` 8개를 세워 `ScenarioConfig_1.SquadZones`로 묶었으며, 시험 레벨 DT 를 드론 행까지 얹은 26행으로 다시 저작했다. 첫 실행에서 3분대가 3차로 "도주"하면서 아군과 계속 교전하고 UGV 가 대타를 계속 쏘던 문제 → 새 동사 `BreakContact` + 제외 플래그의 두 소비자 연결(`../soldier_ai_lab/squad/2026-09-21_break_contact_and_targeting_exclusion.md`).

선행: `2026-09-18_soldierlab_three_stage_test_level.md`(시험 레벨 `L_SoldierScenario` — 이 문서는 그 7절 "New_kadex_0811 적용 시"의 실행 기록, [W70]) · `2026-09-17_soldierlab_squad_scenario_link.md`(시나리오 ↔ SoldierLab 연결) · `../soldier_ai_lab/squad/2026-09-18_squad_layer_fixes_quota_engage_range.md`(정원제·`EngageRangeCm`·트리거) · `scenario_authoring_guide.md`(DT 일반, 09-21 배너·2.6절) · `scenario_three_stage_combat.md`(구 병사 기준 구현 현황 — 09-21 배너).
구 시스템(이 레벨에서 이제 죽은 것): `../ai_combat/2026-08-31_enemy_squad_reorg.md` · `../ai_combat/2026-09-03_dynamic_squad_reassignment_and_casualty_log.md`.

> 신뢰도: **[A]** 코드·에셋·로그 실측(문서 세션이 `Source/`·`.umap`·`.uasset` 문자열·`Saved/Logs/titan_example-backup-2026.09.21-05.02.59.log` 를 다시 읽었다 — 줄 번호는 그 시점) · **[B]** 잠정 — 존 좌표·DT 행 값은 세션 기록(MCP 로 쓴 값)에서 옮겼고 에디터에서 다시 열어 확인하지 않았다 · **[C]** 미측정. ⚠ `New_kadex_0811.umap` 은 **`user2` 도 체크아웃 중**(바이너리 — 제출 순서 조율). `Source/` 트리는 누군가 넓게 `p4 edit` 을 쳐서 거의 전부 "opened" 라 opened 여부가 변경 신호가 아니다.

---

## 0. 한 장 요약

| 무엇 | 값 |
|---|---|
| 레벨 | `Content/New_kadex_0811.umap` (2026-09-21 14:02 저장본, MCP 저작) [A] |
| 병사 | 적 `BP_Soldier_Hostile_1~15`(분대 `1`/`2`/`3` × 5, 각 첫 번째 `bSquadLeader`) · 아군 `BP_Soldier_Friendly_1~25`(분대 `1`~`5` × 5) — **구 `BP_Enemy_kadex_N`/`BP_Ally_kadex_N` 과 같은 트랜스폼** [A: `.umap` 에 `BP_Enemy_kadex`/`BP_Ally_kadex` 문자열 0개, `BP_Soldier_*` 40개] |
| 전투지 | `ASoldierZone` **8개**(1절) — 적 분대별 3+2+1, 아군 북/남 2 [A: `Z0_S1_Engage` … `ZF_South_Defend` 문자열] |
| `ScenarioConfig_1` | `SquadZones`(2절) · `ScenarioStepTable = DT_ScenarioSteps_ThreeStage_SoldierLab` [A] · 그 외 **그대로**: RunMode Demo · `bSoldierLabHostilesStartHidden=true`(기본값, 드론 `RevealEnemies` 가 푼다) · `UGVFormUpDestination=ugvpoint1` · `UGVZone2Destination=ugvpoint2` · `CommandPost=BP_TitanTruck4` · `bDemoForceCommandPostAutoFire=true`(원래 값) · Zone3 목적지 비어 있음 [B] |
| DT | `/Game/Scenario/DT_ScenarioSteps_ThreeStage_SoldierLab` **26행**(3절) — `DT_ScenarioSteps_ThreeStage`(26행) 에서 4행 빼고 4행 더함, 적/아군 행은 `IssueSquadOrder` [A: 행 이름 · B: 값] |
| 유지 | TargetPoint 113개 전부(`enemy_fire/cover_{zone}_{n}` · `ally_fire/cover_n` · `ugvpoint1/2` · `EnemyCube` 태그 `TargetPoint_1`) · 적 경로 스플라인 `RoadCenterline_Enemy1/2/3` · 드론 `uavpath`/`uavpath2` · 낙하산 · 트럭 · UGV · 드론 [A: 문자열] |
| 삭제 | 구 병사 40 액터(사용자 승인, 백업 = P4 리비전) |
| 아웃라이너 | `SoldierLab/Hostile` · `SoldierLab/Friendly` · `SoldierLab/Zones` |
| 첫 PIE | 2026-09-21 04:38 UTC(13:38 KST) — **전 체인이 순서대로 발동**(4절). 문제 1건: 3분대 3차 "도주"가 도주가 아니었다(5절) |

구 마커(`enemy_fire_1_1` 같은 TargetPoint)는 **지우지 않았다** — SoldierLab 병사는 안 읽지만 존 위치의 근거이고 구 시스템 레벨(`kadex_test`)과 같은 저작 규약이라 남겼다. 이 레벨에서 구 `UEnemyCombatComponent` 흐름(`CombatZones`/`LastStandZoneIndex`/`FleeQuota`/`BeginEnemyFleeZone2/3`/`ExcludeFleeingEnemiesFromAllyTargeting`/`HoldFleeingEnemyFire`/`RetargetEnemies*`)은 **대상 액터가 0명이라 죽은 코드**다(컴파일은 됨, 7절).

---

## 1. `ASoldierZone` 8개 — 마커 군집 위에 세운 위치 [B 좌표 / A 존재]

레벨 기하(사용자·세션이 마커 좌표로 잰 것, cm):

```
적 스폰      (-30500,  9500)                 서쪽 끝
1차 전투지   마커 군집 중심 (-2500, 10500, 3050)   스폰에서 ≈280 m 동쪽
ugvpoint1    (-3034, 14320)                  1차 존에서 ~40 m 북
2차 전투지   2분대 군집 (15900, 2200) · 3분대 군집 (18900, 3100)   1차에서 ≈200 m
ugvpoint2    (15565, 1110)                   2분대 2차 군집에서 15 m(⚠ 8절)
아군         x 23000~25600 · y −700~6800, 서쪽을 봄 — 2차 전투지에서 60~90 m 동쪽
3차 전투지   (53700, 12900, −3680)           2차에서 ≈360 m 동쪽, 트럭 (57330, 12280) · UGV 시작 (58790, 13500) · 드론 옆
```

| 존 | 위치 (cm) | `RadiusCm` | yaw(= 섹터) | `NavFilterClass` | 서 있는 마커 군집 | `SquadZones` 에서의 역할 |
|---|---|---|---|---|---|---|
| `Z0_S1_Engage` | (−4200, 10500, 3060) | 1000 | 90 | `NavQueryFilter_EnemySquad1` | `enemy_fire/cover_1_1~5` (1분대 1차) | Hostile 1 → z0 |
| `Z0_S2_Engage` | (−1800, 10000, 3040) | 1200 | 90 | `NavQueryFilter_EnemySquad2` | `enemy_fire/cover_1_6~10` | Hostile 2 → z0 |
| `Z0_S3_Engage` | (−1400, 11000, 3100) | 1000 | 90 | `NavQueryFilter_EnemySquad3` | `enemy_fire/cover_1_11~15` | Hostile 3 → z0 |
| `Z1_S2_Withdraw` | (15900, 2200, 600) | 1200 | 0 | Squad2 | `enemy_fire/cover_2_6~10` (2분대 2차) | Hostile 2 → z1 |
| `Z1_S3_Withdraw` | (18900, 3100, 0) | 1200 | 0 | Squad3 | `enemy_fire/cover_2_11~15` | Hostile 3 → z1 |
| `Z2_S3_Escape` | (53700, 12900, −3680) | 1500 | 0 | Squad3 | `enemy_fire/cover_3_11~15` (3분대 3차, 트럭 앞) | Hostile 3 → z2 |
| `ZF_North_Defend` | (24400, 4500, −900) | 2500 | 180 | — | `ally_fire/cover_1~15` | Friendly 1·2·3 → z0 |
| `ZF_South_Defend` | (24000, 0, −750) | 2000 | 180 | — | `ally_fire/cover_16~25` | Friendly 4·5 → z0 |

- **분대마다 존을 따로 둔 이유**: 존이 `NavFilterClass` 를 나르므로(`Squad/SoldierZone.h:102-104` → `ApplyToOrder` → `FSoldierSquadOrder::NavFilterClass` → 배정) 분대별 필터 = 08-31 에 저작한 **경로 스플라인 3개**(`RoadCenterline_Enemy1/2/3` = `NavArea` 가중, `../vehicle/ugv/2026-08-27_new_kadex_0811_navmesh_autonomous_driving.md` 3절)를 그대로 쓰려면 분대별로 존이 있어야 한다. 시험 레벨(존 공유)과 다른 점.
- `ArrivalFraction 0.8` · `BandCm 1200` · `ApproachScaleCm 3000` · `bUseSector true` · `SectorHalfWidthDeg 45` · `PatrolWeight 1.0` · `PatrolStaleSeconds 30` 은 **전부 기본값**(`SoldierZone.h:64-100`) [A 기본값 / B 인스턴스].
- yaw 는 섹터 = "아는 게 없을 때 보는 부채꼴 ±45°"(`SoldierOrderTypes.h:173-181`): 1차 존 **90**(+Y) = `ugvpoint1` 이 있는 북쪽(UGV 가 오는 방향), 2차 존 **0**(+X) = 아군이 있는 동쪽, 아군 존 **180**(−X) = 적이 오는 서쪽. 3차 존 0 은 트럭(+X 쪽)을 향한다.
- 위치는 **사용자가 미세 조정 예정**. 특히 `Z2_S3_Escape` 를 지휘소 정면(트럭에서 ≈37 m)에 둔 것은 "3차 = 트럭 RCWS 가 잡는 자리"라는 시나리오 의도에서 나온 세션 판단이지 요구사항이 아니다 → [Q51].

---

## 2. `ScenarioConfig_1.SquadZones` [B]

```
Hostile  "1" → [Z0_S1_Engage]
Hostile  "2" → [Z0_S2_Engage, Z1_S2_Withdraw]
Hostile  "3" → [Z0_S3_Engage, Z1_S3_Withdraw, Z2_S3_Escape]
Friendly "1","2","3" → [ZF_North_Defend]
Friendly "4","5"     → [ZF_South_Defend]
```

`ZoneIndex` 는 이 배열의 인덱스(0 = 1차, 1 = 2차, 2 = 3차). 1분대는 2차 존이 없으므로 `EnemyFleeToZone2` 의 대상이 `2,3` 이고 1분대원은 **정원제로 편입될 때만**(`SquadId` 가 2 나 3 으로 바뀌어) 2차로 간다 — 구 시스템의 `LastStandZoneIndex` 와 같은 결과를 다른 기제로.

`FindSquadZone(Faction, SquadId, ZoneIndex)` 가 없는 인덱스를 받으면 `IssueSquadOrder … 의 N번 전투지가 AScenarioConfig::SquadZones에 없음 — 이 분대는 건너뜀` 경고(`UI/ScenarioStateSubsystem.cpp:1964-1969`) — 첫 실행 로그에 이 경고 0건 [A].

---

## 3. DT `DT_ScenarioSteps_ThreeStage_SoldierLab` — 26행 [A 행 이름 / B 값]

원본 `DT_ScenarioSteps_ThreeStage`(26행, 2026-09-15 저장본)를 복제한 뒤:

| 뺀 행 (4) | 왜 |
|---|---|
| `RetargetToAllies` · `RetargetToCommandPost` | SoldierLab 에는 표적 선호가 없다 — 위협(나를 쏜 놈) 기반으로 알아서 넘어간다(시험 레벨 문서 3.1절) |
| `HoldFleeingFire` | `ReturnFireOnly` 가 "3 s 안에 나를 쏜 접촉에게만"이라 트럭 응사가 저절로 열린다 → 3차 도주도 `ReturnFireOnly`. **다만 5절의 문제로 이 판단은 절반만 맞았다** — "안 쏜다"는 됐지만 "도주한다"가 안 됐다 |
| `AllyAmbush` | `BroadcastAmbush` 는 구 `AllyFormationComponent` 전용. 대신 `AllyDefend`/`AllyEngage` |

| 더한 행 (4) | 언제 | 명령 |
|---|---|---|
| `AllyDefend` | TimerOnly +1 s | Friendly 전원 · **Occupy** z0 · ReturnFireOnly · agg 0.3 · `EngageRangeCm 6000`(시험 레벨은 5000 — 숲 시야에 맞춰 60 m) |
| `AllyEngage` | `EnemyNearFriendlySoldiers` 8000 | Friendly 전원 · Occupy z0 · **Free** · agg 0.5 |
| `Squad3Run` (첫 실행 뒤 추가) | Prereq `EnemyFleeToZone3` · TimerOnly +6 s | Hostile `3` · **BreakContact** · HoldFire · Rush |
| `Squad3Stand` (첫 실행 뒤 추가) | Prereq `EnemyFleeToZone3` · `CommandPostFiredNearEnemy` 8000 | Hostile `3` · Occupy z2 · Rush · Free · agg 0.8 |

`IssueSquadOrder` 로 바뀐 행:

| 행 | 구 (`ThreeStage`) | 신 (`…_SoldierLab`) |
|---|---|---|
| `EnemyApproach` | TimerOnly +1 s → `BeginEnemyEngagementApproach` | 같은 트리거 → Hostile 전원 · **MoveTo** z0 · Cautious · **HoldFire** · agg 0.3 (로그 라벨 "은밀 접근 … 사격 금지, 안전 우선 0.3") |
| `EnemyEngage` | `UGVFiredNearEnemy` 10000 → `BeginEnemyEngage` | **트리거 그대로**(시험 레벨의 `EnemyFireStarted` 아님 — 접근이 HoldFire 라 적이 먼저 쏠 일이 없다) → Hostile 전원 · **Occupy** z0 · Rush · Free · agg 0.8 |
| `EnemyFleeToZone2` | `EnemyCasualtyCountAtLeast` 3 → `BeginEnemyFleeZone2` | 같은 트리거 → Hostile `2,3` · **Withdraw** z1 · Rush · ReturnFireOnly · **`Quota 10`** · stagger 0.5~3 s |
| `EnemyFleeToZone3` | Prereq Flee2 · casualties ≥ 7 → `BeginEnemyFleeZone3` | 같은 → Hostile `3` · Withdraw z2 · Rush · ReturnFireOnly · **`Quota 5`** |
| `ExcludeFleeingEnemies` | Prereq Flee3 · +4 s → `ExcludeFleeingEnemiesFromAllyTargeting` | 같은 → Hostile `3` · **SetTargetable false** |
| `DroneFrameZone2` | Prereq `AllyAmbush` | Prereq → **`AllyEngage`** |

**안 바뀐 행 (16)**: `UAVMission` · `UAVSpotted`(→ `MoveUGVToZone1Destination`) · `RevealEnemies` · `UAVDetectionOff`(`bEnabled=false`) · `UGVSurveillance`(false) · `UGVAutoFire`(false) · `UGVArriveZone1`(`SetDemoUGVAutoFire`) · `DroneSeeEnemies` · `DroneWideView` · `UGVMoveZone2` · `UGVSpeedLimitOn` · `DroneChasePath` · `UGVMoveZone3`(false) · `DroneFrameZone3` · `CommandPostFire` · `ScenarioComplete`. `bEnabled=false` 4행은 원본과 같다.

DT 에셋 안에 실제로 든 enum 문자열 [A]: 동사 `MoveTo`/`Occupy`/`Withdraw`/`SetTargetable`/`BreakContact` · ROE `Free`/`ReturnFireOnly`/`HoldFire` · 속도 `Cautious`/`Normal`/`Rush` · 트리거 `EnemyNearFriendlySoldiers`/`UGVFiredNearEnemy`/`CommandPostFiredNearEnemy`/`EnemyCasualtyCountAtLeast` — `EnemyFireStarted`·`AllyFireStarted`·`SquadOrderAchieved` 는 **이 DT 에 없다**.

---

## 4. 첫 PIE — 시간 축 (2026-09-21 04:38 UTC, `titan_example-backup-2026.09.21-05.02.59.log`) [A]

`Squad3Run`/`Squad3Stand` 가 **아직 없던** DT 로 돌린 실행이다(두 행은 이 실행의 문제를 보고 추가, DT 저장 14:06 KST > 실행 13:38). 데모 자동 시작 `[2026.09.21-04.38.30:004]` 기준:

| +s | 스텝 | 로그 |
|---|---|---|
| +1 | `EnemyApproach` · `AllyDefend` | `시나리오 스텝 발동: EnemyApproach (시작+1s -> 적군 전 분대 1차 전투지로 은밀 접근(Cautious, 사격 금지, 안전 우선 0.3))` · `AllyDefend (… 응사만, 소극적 0.3, 사거리 60m)` |
| +3 | `UAVMission` | |
| +82 | `UAVSpotted` → `RevealEnemies` | `UAV가 낙하산(적) 발견 -> UGV 자율주행 시작` |
| +170 | `UGVArriveZone1` | `(데모) RCWS ARM+자동사격, 탐색 스윕(자동정찰) 시작` |
| +180 | `EnemyEngage` → `DroneSeeEnemies` · +2 `DroneWideView` | `UGV RCWS가 100m 이내에서 최초 사격 -> 적군 전 분대 1차 전투지 교전 돌입(Rush, Free, 공세 0.8)` |
| +210 | `EnemyFleeToZone2` → `DroneChasePath` | `[Squad] reinforce Hostile quota=10 living=7 needed=3 moved=3 : BP_Soldier_Hostile_1(1->2), BP_Soldier_Hostile_5(1->2), BP_Soldier_Hostile_4(1->3)` · `정원 10명 채우기 — ESoldierFaction::Hostile 분대 [2,3]에 3명 편입` |
| +260 | `AllyEngage` → `DroneFrameZone2` | `적 보병이 아군 보병 80m 이내 -> 아군 교전 개시(거점 유지, Free, 공세 0.5)` |
| +274 | `UGVMoveZone2` → `UGVSpeedLimitOn` | `적과 55m 이상 멀어짐 -> UGV도 [UGV 2차 목적지]로 이동` |
| +350 | `EnemyFleeToZone3` → `DroneFrameZone3` | `[Squad] reinforce Hostile quota=5 living=4 needed=1 moved=1 : BP_Soldier_Hostile_6(2->3)` |
| +353 | `CommandPostFire` | |
| +354 | `ExcludeFleeingEnemies` | `분대 명령 ESoldierOrderVerb::SetTargetable zone=0 → ESoldierFaction::Hostile 1개 분대, 5명` |

전 체인이 요구 순서대로 발동했고 사용자 판정 "아주 잘됨". 시험 레벨과 달리 접근 중 `EnemyFireStarted` 가 당겨질 위험이 없었던 것은 접근 ROE 가 HoldFire 라서다([C-145] 의 New_kadex 판은 성립하지 않는다 — 다른 질문이 됐다).

**페이싱** — 스폰 → 1차 존 280 m 를 Cautious 홉으로 가는 데 UGV 도착(+170)과 거의 같이 끝났다(EnemyEngage +180). 시험 레벨 문서 4.2절의 "~2 분" 우려가 그대로이지만 드론 정찰(+82)·UGV 주행(+170)이 그 시간을 채워 데모로는 어색하지 않았다 [C-164]. 필요하면 `Z0_*` 의 `ApproachScaleCm` · 스폰 위치로 조정.

---

## 5. 문제 — 3분대의 3차 "도주"가 도주가 아니었다 [A]

`ExcludeFleeingEnemies` 뒤 로그:

```
[RCWSFireControl] BP_UGV_0901_C_1: 타겟 BP_Soldier_Hostile_C_4 시야 상실(경계도 0.93) → 마지막 조준점 4.0초 응시   (+374)
[RCWSFireControl] BP_UGV_0901_C_1: 타겟 BP_Soldier_Hostile_C_6 시야 상실(경계도 0.40) …                              (+380)
[RCWSFireControl] BP_UGV_0901_C_1: 타겟 BP_Soldier_Hostile_C_3 …  (이후 90 s 동안 12회)
```

`Hostile_4`·`Hostile_6` 은 대타로 3분대에 편입된 병사(4절 reinforce 줄) — 제외 명령 5명 안에 들었는데 UGV 가 계속 물고 있었다. 그리고 3분대는 2차 존 근처 엄폐를 홉하며 아군과 사격을 주고받았고 3차 존까지 안 갔다(관전).

원인 셋(코드 확인 [A]):

1. **RCWS 반쪽이 빠졌다** — 구 `ExcludeFleeingEnemiesFromAllyTargeting` 이펙트는 적에게 표시를 다는 것과 **UGV RCWS `bRespectEnemyTargetingExclusion=true` 를 켜는 것** 두 가지를 했다(`UI/ScenarioStateSubsystem.cpp:1502-1509`). 새 `IssueSquadOrder SetTargetable` 경로는 앞의 것만 했다. 이 플래그의 레벨 인스턴스 기본값은 **false**(`Vehicles/RCWSFireControlComponent.h:685-690`, 트럭은 설계상 false 유지 — "도망쳐 오는 분대와 싸우는 게 그쪽 역할"). 그래서 브리지가 `bTargetableByFriendlyForces=false` 를 잘 써 줘도(`Soldiers/SoldierLabBridgeSubsystem.cpp:201`) RCWS 가 `:453` 의 `if (bRespectEnemyTargetingExclusion)` 을 안 들어갔다.
2. **SoldierLab 아군 보병이 제외 플래그를 아예 안 읽었다** — `USoldierEngagementComponent` 후보 선정에 그런 검사가 없었다. 아군이 3분대를 계속 쏘니 3분대의 `ReturnFireOnly` 가 아군에게 계속 열렸다.
3. **`Withdraw` + `ReturnFireOnly` + 엄폐 가격 = 엄폐 홉 후퇴** — Withdraw 의 배정은 MoveTo 와 같은 Approach 모드(`Squad/SoldierSquadSubsystem.cpp:253-260`)라 엄폐 층이 매 홉을 엄폐·경로 위험·제압으로 값을 매기고, 쏘는 아군이 있으면 응사하며 홉 사이에 머문다(`MinDwellSeconds`). 360 m 를 이렇게 가면 도주로 안 보인다.

수정(전부 `../soldier_ai_lab/squad/2026-09-21_break_contact_and_targeting_exclusion.md`):

- `ESoldierOrderVerb::BreakContact` — 존은 그대로, **엄폐 층이 Fighting/Route/Danger/Suppression 을 0 으로**(땅 = 목표 거리 + 팀원 자리만), dwell 없음, 이동 중 항상 스프린트, 서서 뛴다. ROE/사거리/속도를 같이 나른다. 다음 존 동사가 지운다.
- `USoldierEngagementComponent::IsContactExcluded` — 표적 액터의 배정이 `!bTargetableByOwnSideWeapons` 면 후보에서 빼고 잡고 있던 잠금도 놓는다(보는 것·숨는 것은 그대로).
- `IssueSquadOrderSpec`: `SetTargetable(false)` 이면 UGV RCWS `bRespectEnemyTargetingExclusion=true` 도 켠다(`UI/ScenarioStateSubsystem.cpp:1914-1928`, 트럭은 안 건드림).
- DT: `Squad3Run`(Flee3 +6 s, BreakContact HoldFire Rush) · `Squad3Stand`(트럭이 80 m 안에서 쏘면 Occupy z2 Free 0.8) 추가(3절).

2차 PIE 판정 → **[C-163]**.

---

## 6. 게임 모드 — `GM_SoldierLab` 에서도 시나리오는 돈다 [A]

사용자 질문 "GM 을 SoldierLab 것으로 바꿔도 되나": 된다, 단 잃는 것이 있다.

| 되는 이유 | 근거 |
|---|---|
| `UScenarioStateSubsystem` 은 **GameInstance 서브시스템** — 게임 모드·PC 무관 | `UI/ScenarioStateSubsystem.cpp:42-49` 주석 |
| `ResolveUGVPawn` 은 titan PC 가 없으면 **`Atitan_examplePlayerController` CDO 의 `UGVVehicleClass`** 로 폴백 | `titan_examplePlayerController.cpp:697-705` |
| `Atitan_exampleGameState` 를 쓰는 곳은 전부 null 가드(`GetScenarioGameState` 가 nullptr → 조용히 스킵) | `:44-48` |
| 데모 자동 시작은 `RunMode=Demo` + `EnemyCube` 태그만 있으면 됨 | 시험 레벨이 이미 이 조건으로 돈다 |

| 잃는 것 | 이유 |
|---|---|
| RTSP · HUD/미니맵 · 탐지 오버레이 | titan PC/HUD 소유 |
| `ShowUIMessage` 토스트(`ScenarioComplete`) | 폴백이 titan PC + SelfDefense 축을 요구 |
| 데모 플래그 리플리케이션 | 단일 PC 에서만 의미 |

첫 실행은 원래 GM(titan)으로 돌렸다. `GM_SoldierLab`(관전 폰)으로 돌리는 것은 병사 거동을 볼 때의 선택지.

---

## 7. 이 레벨에서 죽은 것 (컴파일은 되는 코드) [A]

| 구 기제 | 어디 | 상태 |
|---|---|---|
| `UEnemyCombatComponent` 의 `CombatZones`/`LastStandZoneIndex`/`FleeQuota`/`SquadId` | `Soldiers/EnemyCombatComponent.*` | 대상 액터 0 — `kadex_test` 레벨에만 해당 |
| 이펙트 `BeginEnemyEngagementApproach`/`BeginEnemyEngage`/`BeginEnemyFleeZone2/3`/`RetargetEnemies*`/`HoldFleeingEnemyFire`/`ExcludeFleeingEnemiesFromAllyTargeting`/`BroadcastAmbush/Approach` | `UI/ScenarioStateSubsystem.cpp` | 이 DT 에 없음. `ExcludeFleeing…` 의 RCWS 반쪽은 `IssueSquadOrderSpec` 으로 옮겨졌다 |
| `ScenarioZoneRoles` · 구 정원제(09-03) | 〃 | `ReinforceSquads`/`Quota` 가 대체 |
| **드론 프레이밍/관측의 분대 판정** — `UEnemyCombatComponent::LastStandZoneIndex` 를 읽음 | `Drone/DronePawn.cpp:1177-1178` · `:1324-1325` · `:1369` | SoldierLab 적군엔 컴포넌트가 없어 필터가 **통과** → 살아있고 드러난 적 전원을 프레이밍. 코스메틱. 브리지로 배정(`SquadId`/`ZoneTag`)을 읽게 바꿀 수 있다 → [W104] |

정리(삭제)는 별도 세션 몫 → [W105]. `kadex_test` 가 아직 구 병사를 쓰므로 지금은 지우지 않는다.

---

## 8. 열린 것 · 다음 PIE 체크리스트

### 8.1 체크리스트 (2차 PIE, `Squad3Run`/`Squad3Stand` 포함 DT)

콘솔: `SoldierLab.Debug.Squad 1` · `SoldierLab.Debug.Squad.Log 1` · `SoldierLab.Debug.Engagement.Log 1`.

| 순서 | 볼 것 | 통과 |
|---|---|---|
| 1 | 4절 체인이 같은 순서로 | `EnemyApproach` → `UAVSpotted` → `UGVArriveZone1` → `EnemyEngage` → `EnemyFleeToZone2` → `AllyEngage` → `UGVMoveZone2` → `EnemyFleeToZone3` → `CommandPostFire` → `ExcludeFleeingEnemies` → **`Squad3Run`(+6 s)** |
| 2 | `[Squad] order … BreakContact` 5명 · 라벨에 ` BREAK` | 3분대 전원 |
| 3 | `[Engage]` 3분대 줄에서 `roe=hold` + 스프린트, `[Cover] task=approach` 의 홉이 존 방향으로만 | 엄폐 홉·응사 없음, 서서 뜀 |
| 4 | `ExcludeFleeingEnemies` 뒤 `[RCWSFireControl] BP_UGV_0901_C_1: 타겟 BP_Soldier_Hostile_*` **0줄** | UGV 가 3분대(대타 포함)를 안 잡음 |
| 5 | 아군 `[Engage] tgt=` 에 3분대원 이름 없음 | `IsContactExcluded` |
| 6 | 트럭이 쏘면(`CommandPostFiredNearEnemy` 8000) `Squad3Stand` → 3분대 `Occupy z2 Free` → `[Engage] tgt=BP_TitanTruck4…` | 3차 교전 성립 — [C-122] 의 New_kadex 판 |
| 7 | `ScenarioComplete` | 완주 |

### 8.2 열린 항목 → `../soldier_ai_lab/OPEN_ITEMS.md`

| ID | 항목 |
|---|---|
| **[C-163]** | BreakContact + 제외 2소비자가 2차 PIE 에서 5절 증상을 없애는가(8.1 의 2~5) |
| **[C-164]** | New_kadex_0811 페이싱 — 접근 280 m ≈ 180 s(드론·UGV 와 맞물려 지금은 허용). `EnemyNearFriendlySoldiers 8000` · `AllyDefend EngageRangeCm 6000` 이 숲 시야에서 맞는 문턱인가 |
| **[Q51]** | 존 위치 미세 조정 — 특히 `Z2_S3_Escape`(지휘소 정면 37 m) · `Z0_*` yaw · `ugvpoint2` 가 2분대 2차 군집에서 **15 m**(UGV 가 도주 분대 한가운데로 간다 — 옮기든 존을 옮기든) |
| **[W104]** | 드론 프레이밍의 분대 판정을 구 컴포넌트 대신 브리지/배정으로(7절) |
| **[W105]** | 구 `EnemyCombatComponent` 시나리오 기제 정리(7절) — `kadex_test` 처리 결정 뒤 |
| **[W106]** | (문서 세션 발견) UGV RCWS 의 **스티키 표적은 제외를 안 본다** — `ExcludeFleeingEnemies` 순간 물고 있던 한 명은 시야를 1 s 넘게 잃어야 놓는다(`RCWSFireControlComponent.cpp:584-613`). 8.1 의 4번이 그만큼 늦게 0 이 될 수 있다 |
| [C-125] | 2-PC FullSystem 에서 분대 층 리플리케이션 — 여전히 미검증 |
| — | 3분대 3차 경로가 아군 사선을 길게 가로지르면 블로킹 볼륨 스크린(`guide/detection_dev_guide.md` §8) — 2차 PIE 관전 뒤 |
| ✅ [W70] | New_kadex_0811 재저작 — 이 문서로 해결 |

Perforce: `New_kadex_0811.umap`(user2 동시 체크아웃) · `DT_ScenarioSteps_ThreeStage_SoldierLab.uasset`(신규) · `Source/` 5파일(코드 문서 참고). 제출 전 user2 와 `.umap` 순서 조율.
