# 시나리오 ↔ SoldierLab 분대 명령 연결 (시나리오 쪽에서 본 기록)

2026-09-17 / 진행중(코드 완료 · 빌드·시험 레벨 검증 대기) / 스텝 테이블이 새 이펙트 `IssueSquadOrder` 한 종류로 SoldierLab 분대에 구역·ROE·속도·공세성을 주고, 브리지가 SoldierLab 병사를 RCWS/사망 카운트에 연결. **적·아군 전원 SoldierLab로 교체 결정** — 구 BP 병사용 이펙트는 그대로 남되 새 병사에는 무효.

설계·개인 AI 쪽 전문: `C:\working\insung_grapic\titan\soldier_ai_lab\squad\2026-09-17_command_layer_design.md`.
관련: `scenario_authoring_guide.md`(DT 저작 실무 — 이 문서가 그 확장) · `scenario_three_stage_combat.md`(구현 현황) · `2026-09-01_scenario_run_modes_demo_fullsystem.md`(데모 모드).

> ★ **2026-09-18 정정** — 시험 레벨 `L_SoldierScenario` 와 DT `DT_ScenarioSteps_SquadThreeStage`(13행)가 만들어져 첫 PIE 를 돌았다 → **`2026-09-18_soldierlab_three_stage_test_level.md`**(레벨·DT·검증 순서). 이 문서에서 바뀐 곳: 2절 행 필드에 **`EngageRangeCm`**(ROE 사거리) · **`Quota`**(정원제·대타) 추가, 트리거에 **`EnemyFireStarted`** · **`EnemyNearFriendlySoldiers`** 추가 · 3절 저작안은 시험 DT 로 대체(S1 트리거 = `EnemyFireStarted`, 3차 도주도 `ReturnFireOnly`, `RetargetToCommandPost` 행 불필요) · 4절 체크리스트의 존은 이제 **에디터에서 반경·도착선·섹터가 보인다**(회전 = 섹터) · 5절 1~2번 완료, 3번 진행중. 코드 쪽 상세는 `soldier_ai_lab/squad/2026-09-18_squad_layer_fixes_quota_engage_range.md`.

> ★ **2026-09-21** — New_kadex_0811 본 레벨이 SoldierLab 병사로 이관됐다(5절 계획의 완료) → `2026-09-21_soldierlab_migration_new_kadex_0811.md`. 동사에 **`BreakContact`** 추가, `SetTargetable(false)` 가 UGV RCWS `bRespectEnemyTargetingExclusion` 도 켬, SoldierLab 아군 보병이 제외 플래그를 직접 읽음 → `soldier_ai_lab/squad/2026-09-21_break_contact_and_targeting_exclusion.md`. 1절 표의 "안 바뀐 것" 중 `ExcludeFleeingEnemiesFromAllyTargeting` 의 RCWS 반쪽은 이제 `IssueSquadOrderSpec` 에도 있다.

---

## 1. 무엇이 바뀌었나 (코드, `Source/titan_example`)

| 파일 | 내용 |
|---|---|
| `titan_example.Build.cs` | `SoldierLab` 모듈 의존 추가 (**한 방향** — SoldierLab은 titan을 모른다) |
| `UI/ScenarioStepTypes.h` | 이펙트 `IssueSquadOrder` · 트리거 `SquadOrderAchieved` · 행 필드 `SquadOrder`(`FScenarioSquadOrderSpec`) |
| `UI/ScenarioStateSubsystem.{h,cpp}` | `ExecuteSquadOrder`/`IssueSquadOrderSpec` · `AllyFireStarted`가 SoldierLab 아군 총성(`USoldierRegistrySubsystem::GetGunshotCount(Friendly)`)도 셈 · 드론 Zone2 프레이밍에 SoldierLab 아군 포함 · 콘솔 `titan.SquadOrder` |
| `UI/ScenarioConfig.{h,cpp}` | `SquadZones`(분대별 `ASoldierZone` 배열) · `bSoldierLabHostilesStartHidden` · `FindSquadZone` |
| `Soldiers/SoldierLabBridgeSubsystem.{h,cpp}` (신규) | SoldierLab 병사에 런타임 `UDetectableTargetComponent` 부착(적=Enemy, 시작 시 숨김), 사망 폴링 → `SetIncapacitated(true)`, 명령의 표적 제외 → `bTargetableByFriendlyForces`, UGV/트럭에 `USoldierIdentityComponent(Friendly)` 부착(적군이 차량을 표적으로 봄) |
| `Detection/DetectableTargetComponent.h` | `bTargetableByFriendlyForces` — 구 `UEnemyCombatComponent::bTargetableByAlliesAndUGV`의 탐지 층 판. UGV RCWS(`bRespectEnemyTargetingExclusion`)가 둘 다 읽음 |
| `Vehicles/RCWSFireControlComponent.{h,cpp}` | `Fire()` → `USoldierPerceptionLibrary::BroadcastGunshot`(`SoldierLabGunshotAudibleRangeCm 20000`, 0이면 끔) |
| `Vehicles/RCWSProjectile.{h,cpp}` | 서버 사본 틱마다 SoldierLab 병사 제압(`USoldierSuppressionComponent::ApplyAlongSegment`) |

**안 바뀐 것**: 구 이펙트(`BeginEnemyEngagementApproach`, `BeginEnemyEngage`, `BeginEnemyFleeZone2/3`, `RetargetEnemies*`, `HoldFleeingEnemyFire`, `ExcludeFleeingEnemiesFromAllyTargeting`, `BroadcastAmbush/Approach`)와 그 트리거는 그대로 — **구 BP 병사(`BP_Enemy_kadex`/`BP_Ally_kadex`) 전용**이고 SoldierLab 병사에는 아무 일도 안 한다. 사망 카운트 트리거(`EnemyCasualtyCountAtLeast`/`AllEnemiesEliminated`)·`RevealEnemies`·RCWS 트리거는 브리지 덕에 SoldierLab 병사에도 그대로 동작한다.

---

## 2. DT 행 필드 — `SquadOrder` (`FScenarioSquadOrderSpec`)

| 필드 | 뜻 |
|---|---|
| `Faction` | Hostile / Friendly |
| `SquadIds` | 대상 분대(`USoldierIdentityComponent::SquadId`). **비우면 그 진영의 모든 분대 + 분대 미지정 병사**(각자 자기 ZoneIndex 전투지로) |
| `Verb` | `MoveTo`(가서 지킴) · `Occupy`(처음부터 지킴) · `Withdraw`(이탈, 지연 출발과 짝) · `SetROE` · `SuppressArea` · `SetTargetable` · `Clear` |
| `ZoneIndex` | `AScenarioConfig::SquadZones[분대].Zones[ZoneIndex]` — 0=1차, 1=2차, 2=3차 |
| `Speed` | `Cautious`(스프린트 금지) · `Normal` · `Rush`(이동 중 항상 스프린트) |
| `ROE` | `Free` · `ReturnFireOnly`(최근 3 s 안에 쏜 적에게만) · `HoldFire` |
| `Aggression` | 0.5 = 개인 가중치 그대로. 적군 1차 교전 0.7~0.8, 아군 방어 0.3 |
| `StaggerMinSeconds`/`StaggerMaxSeconds` | 개체별 출발 지연(구 `MinFleeCommitDelaySeconds`~`Max` 자리) |
| `bTargetable` | `SetTargetable` 전용 — false면 UGV RCWS·아군이 이 분대를 안 쏨(이동형지휘소는 무시) |
| `SuppressAreaRadiusCm` | `SuppressArea` 전용 — 중심은 ZoneIndex 전투지. 0 = 해제. ≤ 1000이어야 제압사격 게이트를 통과 |
| `EngageRangeCm` (09-18 추가) | ROE 의 사거리(cm). 이보다 먼 접촉엔 ROE 가 뭐든 안 쏨(조준·엄폐는 함). 0 = 제한 없음(총 자체 가치 ≈ 95 m). 구역 동사·`SetROE` 둘 다 나름 |
| `Quota` (09-18 추가) | 정원제. 구역 동사에서 `SquadIds` 의 생존자 합이 이 수가 되도록 같은 진영 다른 분대에서 목적지(첫 분대의 ZoneIndex 전투지)에 가장 가까운 병사를 **`SquadId` 를 바꿔** 편입(영구). 0 = 끔. `SquadIds` 빈 행에선 무시. 로그 `[Squad] reinforce …` |

트리거 `SquadOrderAchieved`는 `SquadOrder.Faction/SquadIds`만 읽는다(살아있는 전원이 마지막 구역 명령의 **도착선**(`ASoldierZone::ArrivalFraction × RadiusCm`, 09-18) 안에 도달). 09-18 추가 트리거: `EnemyFireStarted`(SoldierLab 적군 총성 수 > 스텝 시작 기준선) · `EnemyNearFriendlySoldiers`(살아있는 적/아군 보병 최단 XY 거리 ≤ `TriggerDistanceThreshold`, 차량 제외).

## 3. 3단계 시나리오 매핑 (저작안, 미검증)

| 행 | Prereq | Trigger | SquadOrder |
|---|---|---|---|
| `EnemyApproach` | — | TimerOnly 1s | Hostile · `*` · MoveTo z0 · Cautious · HoldFire |
| `EnemyEngage` | — | `UGVFiredNearEnemy` 10000 | Hostile · `*` · Occupy z0 · Rush · Free · agg 0.75 |
| `EnemyFleeToZone2` | — | `EnemyCasualtyCountAtLeast` 3 | Hostile · `2,3` · Withdraw z1 · Rush · ReturnFireOnly · stagger 0.5~3 |
| `AllyAmbush` | `EnemyFleeToZone2` | TimerOnly 5s | Friendly · `*` · Occupy z0 · Normal · ReturnFireOnly(결정 대기 [Q49]) · agg 0.3 |
| `EnemyFleeToZone3` | `EnemyFleeToZone2` | `EnemyCasualtyCountAtLeast` 7 | Hostile · `3` · Withdraw z2 · Rush · HoldFire · stagger |
| `ExcludeFleeingEnemies` | `EnemyFleeToZone3` | TimerOnly 4s | Hostile · `3` · SetTargetable false |
| `RetargetToCommandPost` | `EnemyFleeToZone3` | `CommandPostFiredNearEnemy` 8000 | Hostile · `3` · SetROE Free |

`RetargetToAllies`·`HoldFleeingFire` 행은 SoldierLab에선 불필요(표적 선호 없음 / HoldFire를 도주 행에 실음). UGV·드론·트럭·`ScenarioComplete` 행은 그대로.

## 4. 레벨 저작 체크리스트 (시험 레벨 → New_kadex_0811 공통)

- [ ] `ScenarioConfig` 1개: `RunMode=Demo`(시험 레벨은 titan PlayerController가 없어 콘솔 `BeginScenarioEnemyContact`를 못 친다 — 자동 시작이 유일한 진입), `ScenarioStepTable`, `bSoldierLabHostilesStartHidden`(시험 레벨은 false 권장)
- [ ] **`EnemyCube` 태그 액터 1개** — 없으면 `BeginEnemyContactScenario`가 스텝 평가를 시작하지 않는다
- [ ] 병사: `BP_Soldier_Hostile` / `BP_Soldier_Friendly` 배치, 각 인스턴스 `AC_SoldierIdentity.SquadId`(적 `1`/`2`/`3`, 아군 `A`… 임의 이름), `AutoPossessAI`
- [ ] 전투지: 분대당 `ASoldierZone` N개(1차/2차/3차) — 반경이 **싸울 엄폐를 품게**, 회전 = 경계 방향, 필요 시 `NavFilterClass=NavQueryFilter_EnemySquadN`
- [ ] `ScenarioConfig.SquadZones`에 (Faction, SquadId, Zones[0..2]) 연결. 아군 분대는 Zones[0] 하나(매복 구역)
- [ ] UGV(`BP_UGV_0901`)·트럭: `DetectableTargetComponent(Friendly)`가 있으면 브리지가 알아서 `Identity`를 붙인다
- [ ] NavMesh가 모든 구역을 덮는지
- [ ] 콘솔: `SoldierLab.Debug.Squad 1` · `SoldierLab.Debug.Zone 1` · `SoldierLab.Debug.Squad.Log 1` · `SoldierLab.Debug.Cover.Log 1` · `SoldierLab.Debug.Engagement.Log 1`; 손으로 명령: `titan.SquadOrder Hostile * MoveTo 0 HoldFire Cautious 0.7`

## 5. 시험 레벨 계획 → New_kadex_0811

1. 사용자가 새 레벨 생성(`L_SoldierTest` 복제 권장 — 내비·엄폐·병사가 이미 있다) + 위 체크리스트.
2. 빌드 후 DT 신규(`DT_ScenarioSteps_SquadTest`, 3절 행 + UGV 없이도 도는 TimerOnly 대체 트리거) — DT는 MCP로 저작 가능.
3. 로그 판정 [C-122]~[C-126](`soldier_ai_lab/OPEN_ITEMS.md`).
4. 통과 후 New_kadex_0811: 적 15·아군 25 교체, 존 배치, `DT_ScenarioSteps_ThreeStage` 행 교체 → [W70].

⚠ 리스크: 적군이 숲 엄폐에 숨어 시나리오가 안 진행될 수 있다([C-123]) — `Aggression`이 다이얼. 2PC 리플리케이션은 이번에 P5 게이트만 넣었고 실기 미검증([C-125]).
