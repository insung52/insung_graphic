# 시나리오 저작 가이드 — DataTable 구조 · 레벨 세팅 (2026-08-23)

> ⚠️ **[2026-09-21] New_kadex_0811 의 적군/아군은 SoldierLab 병사 + `IssueSquadOrder` 행이다.** 아래 3.2절(`BP_Enemy_kadex`
> 마커)·3.3절(`BP_Ally_kadex`)·2.4절의 적/아군 이펙트(`BeginEnemy*`/`RetargetEnemies*`/`Broadcast*`/`ExcludeFleeing…`)와
> 4절 17행 표는 **구 BP 병사를 쓰는 레벨(`kadex_test`)에만 해당**한다. SoldierLab 시대의 저작(존 액터 · `SquadZones` ·
> `SquadOrder` 필드 · 새 트리거)은 **2.6절**과 `2026-09-21_soldierlab_migration_new_kadex_0811.md`(New_kadex_0811 현재 DT 26행) ·
> `2026-09-18_soldierlab_three_stage_test_level.md`(시험 레벨 13행) 참고.

새 레벨에서 3단계 전투 시나리오(또는 그 변형)를 **코드 수정 없이** 굴리기 위해
"무엇을 어디에 채워야 하는가"만 모은 실무용 문서.

- 요구사항 원본: `C:\working\insung_grapic\titan\newlevel\scenario.md`
- 구현 현황/설계 배경: `scenario_three_stage_combat.md` (이 문서와 짝)
- 관련 코드: `Source/titan_example/UI/ScenarioStepTypes.h`, `UI/ScenarioStateSubsystem.*`,
  `UI/ScenarioConfig.h`, `Soldiers/EnemyCombatComponent.*`, `Soldiers/AllyFormationComponent.*`

---

## 1. 전체 구조 — 값이 사는 세 곳

| 축 | 담당 | 어디서 편집 |
|---|---|---|
| **언제 무엇이 발동하는가** | 스텝 DataTable (한 행 = 스텝) | 콘텐츠 브라우저의 DataTable 에셋 |
| **어디로 / 무엇을** (레벨 액터 참조) | `ScenarioConfig` 액터 + 각 개체의 마커 | 레벨 뷰포트 + 디테일 패널 |
| **어떻게 보이는가** (속도·간격·자세) | 컴포넌트 EditAnywhere 프로퍼티 | 개체 디테일 패널 (또는 BP 기본값) |

**시나리오 시작**: PIE에서 `` ` `` → `BeginScenarioEnemyContact`
→ `EnemyCube` 태그 액터 위치를 "수신한 적 예상 좌표"로 저장 + **스텝 평가 시작**.
(이 명령을 치기 전에는 테이블의 어떤 행도 평가되지 않는다.)

평가 주기는 `UScenarioStateSubsystem::ScenarioStepTickInterval`(기본 0.2초).
한 번 발동한 스텝은 다시 발동하지 않는다(레벨 재시작 또는 2.7절의 시나리오 재시작 전까지 — 재시작이 발동 이력을 비운다).

---

## 2. DataTable 완전 설명

### 2.1 에셋 만들기와 연결

1. 콘텐츠 브라우저 → 우클릭 → **Miscellaneous ▸ Data Table**
2. Row Structure에 **`ScenarioStepRow`** 선택
3. 만든 에셋을 레벨의 **`ScenarioConfig` 액터 ▸ `Scenario|Steps` ▸ `ScenarioStepTable`** 에 연결
   - 비워두면 서브시스템 생성자 기본값 `/Game/Scenario/DT_ScenarioSteps`가 쓰인다.
   - 레벨마다 다른 테이블을 쓰려면 반드시 이 필드로 지정할 것(서브시스템은 GameInstance 단위라
     에디터에서 레벨별로 직접 지정할 방법이 이것뿐).
4. 행 추가: 테이블 에디터 상단 **Add** → 행 이름(RowName)이 곧 스텝 ID. 다른 행이
   `PrerequisiteStepId`로 이 이름을 참조한다. **이름을 바꾸면 참조도 같이 고칠 것.**

현재 3단계 시나리오용 테이블: `/Game/Scenario/DT_ScenarioSteps_ThreeStage` (17행, 4절 참고)

### 2.2 행 필드 레퍼런스

| 필드 | 타입 | 설명 |
|---|---|---|
| **RowName** | (행 이름) | 스텝 ID. `PrerequisiteStepId`가 가리키는 대상 |
| `DebugLabel` | Text | 사람이 읽는 설명. 로그에 같이 찍힘 — "무슨 스텝인지" 적어두면 디버깅이 쉬움 |
| `PrerequisiteStepId` | Name | **이 스텝이 발동된 뒤부터** 평가 시작. 비우면(None) 시나리오 시작 즉시부터 평가. 타이머의 기준 시각도 이 스텝의 발동 시각 |
| `TriggerType` | Enum | 발동 조건 종류 (2.3 참고) |
| `TriggerDelaySeconds` | float | `TimerOnly` 전용 — 기준 시각으로부터 경과 시간(초) |
| `TriggerDistanceThreshold` | float | 거리 조건(cm). 트리거마다 "이하/이상" 의미가 다름 (2.3) |
| `TriggerCountThreshold` | int32 | `EnemyCasualtyCountAtLeast` 전용 — **몇 명 사망 시**. 0이면 "0명 이상"이라 즉시 발동하니 반드시 1 이상 |
| `EffectType` | Enum | 발동 시 실행할 동작 (2.4 참고) |
| `NotificationKind` | Name | `ShowUIMessage` 전용 — `DT_NotificationWidgets`의 RowName |
| `TimeoutSeconds` | float | (2026-09-22, fail-safe) 기준 시각(Prereq 발동/시나리오 시작)으로부터 이 시간이 지나면 **트리거 조건과 무관하게 발동** = "원래 조건 OR 시간". 0 = 없음, `Manual` 제외. 정상 분포 밖에서만 걸리도록 흐름 로그 실측 최대치 ×1.4~2 로. 발동 시 엔진 로그 Warning + `events.log` `FAIL` + `cycles.csv` `timeouts` 열(2.8절) |
| `SkipIfStepFired` | Name | (2026-09-22, fail-safe) 이 스텝이 이미 발동했으면 이 행은 평가하지 않음(이력에도 안 남음). TimerOnly 후속 조치 행(가지치기 등)이 정상 사이클에서 헛발질하지 않게 하는 게이트 |
| `bFailSafeStep` | bool | (2026-09-23) "이 행은 fail-safe 사다리다" 표시 — 흐름 로그가 `assists` 로 세고 그 사이클을 `CompleteAssisted` 로 적는다. **명령이 실제로 대상을 가졌을 때만 센다**(`Squad1LateWithdraw` 는 매 사이클 타이머로 발동하지만 잔존이 없으면 대상 0명 — 그걸 세면 전 사이클이 `CompleteAssisted` 가 돼 신호가 죽는다). `IssueSquadOrder` 로 된 사다리는 이 플래그가 없으면 평범한 스텝과 구분이 안 된다(12시간 실행이 "85사이클 전부 Complete" 로 보였다 — 실제로는 38사이클이 사다리를 탔다) |
| `bEnabled` | bool | 스텝 on/off. **끄면 이 스텝은 영원히 안 켜지고, 이 스텝을 Prereq로 삼은 뒤 스텝들도 전부 대기 상태로 멈춘다**(의도된 동작 — 흐름을 여기서 끊고 싶을 때 사용) |

### 2.3 트리거 레퍼런스

| TriggerType | 언제 참이 되나 | 읽는 필드 | 레벨에 필요한 것 |
|---|---|---|---|
| `Manual` | 자동으로는 절대 안 켜짐. `UScenarioStateSubsystem::FireScenarioStep(StepId)` 호출로만 (BlueprintCallable — 콘솔 직접 호출은 불가, BP에서 호출) | — | — |
| `TimerOnly` | 기준 시각 + `TriggerDelaySeconds` 경과 | Delay | — |
| `DistanceThreshold` | UGV ↔ **현재 목적지** 거리가 임계값 **이하** | Distance | UGV, 목적지가 설정돼 있어야 함 |
| `LeaderDistanceFromEnemyAtLeast` | UGV ↔ **가장 가까운 살아있는 적** 거리가 임계값 **이상** (적이 0명이면 발동 안 함) | Distance | UGV, 적군 |
| `ActorStopped` | UGV의 `AUGVAIController::IsMoving()`이 false | — | UGV + AI 컨트롤러 |
| `UAVArrived` | UAV `MissionState == Arrived` | — | 레벨에 `AUAVPawn` 1대 |
| `UAVEnemyDetected` | UAV 짐벌이 Faction==Enemy를 감지 | — | UAV + 적군에 `DetectableTargetComponent(Faction=Enemy)` |
| `EnemyDetected` | **UGV RCWS**가 적을 감지(모드 무관) | — | UGV에 `RCWSFireControlComponent` |
| `UGVFiredNearEnemy` | UGV RCWS가 **최근접 적과 `TriggerDistanceThreshold` 이내인 상태에서 발사** (자동/수동 구분 없음) | Distance | UGV RCWS, 적군 |
| `CommandPostFiredNearEnemy` | 위와 동일하되 대상이 **이동형지휘소** RCWS | Distance | `ScenarioConfig.CommandPost` 지정 필수 |
| `AllyFireStarted` | 등록된 아군 중 **누구라도 사격 버스트를 시작** | — | 아군(`AllyFormationComponent`)이 교전 상태여야 함 |
| `EnemyCasualtyCountAtLeast` | **누적 사망 적 수 ≥ `TriggerCountThreshold`** | Count | 적군에 `DetectableTargetComponent` (사망 시 등록 해제되는 걸로 카운트) |
| `AllEnemiesEliminated` | Enemy 진영 등록 타겟이 0 | — | ⚠️ 적 스폰 **전에도** 참이 되므로 반드시 적 등장 이후 스텝을 Prereq로 걸 것 |

### 2.4 이펙트 레퍼런스

| EffectType | 동작 | 레벨에 필요한 것 |
|---|---|---|
| `None` | 아무것도 안 함(순수 타이밍 마커용) | — |
| `BeginUAVMission` | UAV를 **`EnemyCube` 태그 액터 좌표**로 발진 | UAV, EnemyCube 태그 액터 |
| `RevealEnemies` | Enemy 전원 `SetRevealed(true)` (그 전까진 화면에 안 보임) | — |
| `DisableUAVTargetDetection` | UAV 감지 컴포넌트 틱 정지 | UAV |
| `MoveUGVToZone1Destination` | UGV를 **1차 목적지**로 자율주행 (+ 이후 거리/감지 트리거의 기준 액터로 UGV 등록) | `ScenarioConfig.UGVFormUpDestination` |
| `MoveUGVToZone2Destination` | UGV를 2차 목적지로 | `UGVZone2Destination` |
| `MoveUGVToZone3Destination` | UGV를 3차 목적지로 | `UGVZone3Destination` |
| `SetUGVAutoSurveillance` | UGV RCWS 자동 경계(탐색 스윕) | UGV RCWS |
| `SetUGVAutoFire` | UGV RCWS 자동 조준+발사 | UGV RCWS |
| `SetCommandPostAutoFire` | 지휘소 RCWS 자동 조준+발사 | `ScenarioConfig.CommandPost` |
| `SetDemoUGVAutoFire` | (2026-09-15) **데모 전용** — `IsDemoMode() && bDemoForceUGVAutoFire`일 때만 UGV RCWS를 ARM + AutoFire + `DemoFireMode`로. 아니면 로그만 남기고 아무것도 안 함 → FullSystem과 DT를 공유해도 행을 켜둔 채 둘 수 있다(`SetUGVAutoFire`는 모드를 안 봐서 통제기와 충돌) | UGV RCWS, `RunMode=Demo` |
| `BeginEnemyEngagementApproach` | 적 전원 **1차 전투지로 경계 이동 시작**(총 내림/저속/숙임/둘러보기, **사격 안 함**) | 적 `CombatZones[0]` 마커 |
| `BeginEnemyEngage` | 적 전원 **교전 돌입**(총 들고 뛰어서 엄폐 + 사격 시작) | 적 `CombatZones[0]` |
| `BeginEnemyFleeZone2` / `BeginEnemyFleeZone3` | 적 전원 2차/3차 전투지로 **단계적 도주**(개체별 랜덤 지연 후 순차 이탈) | 적 `CombatZones[1]` / `[2]` 마커. 비어 있는 개체는 그 자리 유지 |
| `RetargetEnemiesToAllies` / `RetargetEnemiesToCommandPost` | 적 전원 타겟 초기화 → 다음 틱에 **범위 내 최근접 유효 타겟** 재획득. **두 값의 동작은 동일**(이름은 의도 표시용) | — |
| `BroadcastApproach` / `BroadcastAmbush` | 아군 전원 접근/매복(엄폐·사격) 전환. 분대장이 있으면 수신호 재생 후 | 아군 `AmbushMarker`, 자세 마커 |
| `BeginAllyFormUpAndAdvance` | (구 시나리오) 아군 집결 + UGV 출발 | `UGVFormUpDestination` |
| `RaiseSquadSignal` / `LowerSquadSignal` | 분대장 정지 수신호 올림/내림 | `bIsSquadLeader` 아군 |
| `UAVEngagementZoomOut` | UAV 짐벌이 아군+적 전체를 프레이밍 | UAV |
| `ShowUIMessage` | 알림 위젯 표시 | `DT_NotificationWidgets`에 해당 RowName |
| `ShowRestartPrompt` | (2026-09-22) "재시작 하시겠습니까?" 확인창을 띄움([예]/[아니요]/[☐ 자동 재시작]). 어느 화면에 뜨는지·재시작이 무엇을 하는지는 **2.7절** | `ScenarioConfig.bRestartPromptEnabled`(기본 true), `WBP_RestartPrompt` |
| `KillSquads` | (2026-09-22, fail-safe 가지치기) `SquadOrder.Faction/SquadIds`(비우면 진영 전원)의 **살아있는** SoldierLab 병사를 `Kill(가해자=UGV)` 로 제거 — 피격 사망과 같은 경로라 래그돌·등록부 해제·사망 수·전멸 트리거가 그대로 따라온다. 반드시 `SkipIfStepFired` 게이트와 함께(2.8절 사다리) | `SquadOrder.Faction`, `SquadIds` |

### 2.5 흐름 설계 규칙

- **순서를 강제하는 건 오직 `PrerequisiteStepId` 체인.** 여러 행이 같은 Prereq를 가리키면 병렬 진행.
- 트리거는 Prereq가 충족된 **뒤부터만** 평가된다 — 조건이 그 전에 이미 참이었어도 무시.
- 한 스텝은 한 번만 발동. 반복이 필요하면 별도 행으로 만들 것.
- 흐름을 중간에 끊고 싶으면 그 지점 스텝의 `bEnabled`를 끄면 된다(뒤 스텝 전부 대기).
- 타이밍만 벌리고 싶으면 `EffectType=None` + `TimerOnly` 행을 중간에 끼워 넣는 게 가장 안전.
- ⚠️ **적군 행동 스텝은 UAV/UGV 진행 스텝에 묶지 말 것** (2026-08-23 실사용에서 걸린 함정).
  적군의 교전 돌입/도주는 "UGV가 근거리에서 쐈다", "N명 죽었다" 같은 **조건만 맞으면** 발동해야
  하는데, 이 행들의 `PrerequisiteStepId`를 UAV/UGV 체인(`UAVSpotted` → `UGVSurveillance` →
  `UGVAutoFire`)에 걸어두면 **그 체인 중 하나만 `bEnabled`를 꺼도 적군이 영영 반응하지 않는다**
  (UGV가 바로 옆에서 쏴서 3명을 죽여도 아무 일도 안 일어남). 적군 행동 행은 Prereq를 비우거나
  (조건만으로 판정) **다른 적군 행동 행**에만 걸어서 순서를 잡을 것.
- 반대로 "여러 번 발동하면 안 되는 순서"(2차 도주 → 3차 도주)는 적군 행동끼리 Prereq로 묶어서
  보장한다. `EnemyCasualtyCountAtLeast`는 **누적** 카운트라, 임계값이 작은 행을 Prereq 없이 두면
  큰 행보다 먼저/동시에 켜질 수 있다.

### 2.6 SoldierLab 시대의 행 — `IssueSquadOrder` · `SquadOrder` 필드 · 존 액터 (2026-09-17 신설, 09-21 갱신)

SoldierLab 병사(`BP_Soldier_Hostile`/`BP_Soldier_Friendly`, `Source/SoldierLab/`)에게는 2.4절의 적/아군 이펙트가
**아무 일도 안 한다**. 적/아군 행동은 이펙트 하나 `IssueSquadOrder` 로만 내리고, "언제"는 행의 트리거가, "어디로"는
레벨의 `ASoldierZone` + `ScenarioConfig.SquadZones` 가, "어떻게"는 행의 `SquadOrder` 구조체가 정한다
(`UI/ScenarioStepTypes.h:132-196` `FScenarioSquadOrderSpec`). 전부 **제약**이고 개인의 판단(어디 서고 언제 쏘나)은 안 건드린다.

**`SquadOrder` 필드** (`FScenarioSquadOrderSpec`):

| 필드 | 뜻 | 비고 |
|---|---|---|
| `Faction` | `Hostile` / `Friendly` | |
| `SquadIds` | 대상 분대 이름 배열(각 병사 인스턴스의 `AC_SoldierIdentity.SquadId`). **비우면 그 진영 전원**(분대 미지정 병사 포함) | 구역 동사에서 비우면 각 분대가 **자기** `ZoneIndex` 존으로 |
| `Verb` | `MoveTo`(가서 잡는다) · `Occupy`(처음부터 잡는다) · `Withdraw`(MoveTo 와 같되 "싸우며 물러남" — 순차 이탈 의도) · `SetROE` · `SuppressArea` · `SetTargetable` · `Clear` · **`BreakContact`**(09-21: 존은 두고 가는 길의 엄폐·위험·제압을 셈에서 뺀다 = 진짜 도주. ROE/사거리/속도를 같이 나름, 다음 존 동사가 지움) | 구역 동사 = MoveTo/Occupy/Withdraw/SuppressArea |
| `ZoneIndex` | `SquadZones[분대].Zones[ZoneIndex]` — 0/1/2 = 1차/2차/3차 | 구역 동사 전용 |
| `Speed` | `Cautious`(절대 안 뜀, 무접촉이면 걷기) · `Normal` · `Rush`(이동 중 항상 뜀, 다른 조심도 다 푼다) | |
| `ROE` | `Free` · `ReturnFireOnly`(3 s 안에 나를 쏜 접촉에게만) · `HoldFire`(조준·관찰·엄폐는 함) | |
| `EngageRangeCm` | ROE 의 사거리 — 이보다 먼 접촉엔 안 쏨. 0 = 무제한(≈95 m, 총의 가치 판단) | 접근 단계 보존용(09-18) |
| `Aggression` | 0.5 = 개인 가중치 그대로. 높으면 땅의 가치 ↑·사격 없는 자리 비용 ↑ | 적 교전 0.8 · 아군 방어 0.3 |
| `StaggerMin/MaxSeconds` | 개체별 출발 지연 범위 | Withdraw 의 "한두 명씩" |
| `bTargetable` | `SetTargetable` 전용. **false** = UGV RCWS 와 SoldierLab 아군 보병이 이 분대를 안 쏜다(보고 숨는 건 그대로), 이동형지휘소는 계속 쏜다. 09-21 부터 이 행이 UGV RCWS `bRespectEnemyTargetingExclusion` 도 켠다 | |
| `SuppressAreaRadiusCm` | `SuppressArea` 전용, 0 = 해제, 중심 = ZoneIndex 존 | |
| `Quota` | 정원제 — 구역 동사 직전에 `SquadIds` 분대들의 생존자 합이 이 수가 되도록 같은 진영 다른 분대에서 목적지에 가까운 순으로 **`SquadId` 를 바꿔** 편입(영구). 0 = 끔. `SquadIds` 비우면 무시 | 2차 10 · 3차 5 |

⚠ `MinStance`(접근 중 앉아 걷기)는 SoldierLab 배정에는 있지만 **이 구조체엔 없다**(`soldier_ai_lab/OPEN_ITEMS.md` [W84]).

**SoldierLab 시대에 추가된 트리거**:

| TriggerType | 언제 참 | 읽는 필드 |
|---|---|---|
| `SquadOrderAchieved` | 행의 `SquadOrder.Faction/SquadIds` 분대(비우면 진영 전 분대)가 마지막 구역 명령을 **생존 전원 도착선 안**으로 달성한 순간. 전멸 분대는 발동 안 함 | `SquadOrder` |
| `EnemyFireStarted` | SoldierLab **적군** 누군가가 스텝 시작 이후 한 발이라도 쏘면(`AllyFireStarted` 의 적군판) | — |
| `EnemyNearFriendlySoldiers` | 살아있는 SoldierLab 적 보병 ↔ 아군 보병 최단 XY 거리 ≤ `TriggerDistanceThreshold`(cm). 차량은 안 셈 | Distance |

`AllyFireStarted` 는 SoldierLab 아군 총성도 센다(09-17). `EnemyCasualtyCountAtLeast`/`AllEnemiesEliminated`/`UGVFiredNearEnemy`/
`CommandPostFiredNearEnemy`/`RevealEnemies` 는 브리지(`USoldierLabBridgeSubsystem`) 덕에 SoldierLab 병사에도 그대로 동작한다.

**존 액터 `ASoldierZone`** (`Source/SoldierLab/Squad/SoldierZone.h`) — TargetPoint 대신 배치하고 `ScenarioConfig ▸ SquadZones`
(진영 + 분대 이름 + `Zones[]` 배열)에 등록. 에디터에서 반경(노랑)·도착선(초록)·밴드(흐림) 구와 섹터 화살표가 보인다.

| 프로퍼티 | 뜻 | 기본 |
|---|---|---|
| `RadiusCm` | 이 안이 "잡은" 땅. **싸울 만한 엄폐를 품을 만큼** 넓게(안 그러면 땅과 엄폐 사이에서 진동) | 1500 |
| `ArrivalFraction` | 반경 × 이 비율 안에 들어와야 "도착"(초록 원). 분대 층 achieved 판정과 엄폐 층의 당김 끝이 **같은 선** | 0.8 |
| `BandCm` | 반경 밖에서 홀드 비용이 1 까지 오르는 거리(그 밖은 계속 오름) | 1200 |
| `ApproachScaleCm` | 접근 환율 — 도착선 밖 비용 `1 + 거리/이 값`. 낮추면 홉이 길어짐(페이싱 손잡이) | 3000 |
| **액터 회전** + `bUseSector` / `SectorHalfWidthDeg` | 아는 게 없을 때 보는 **부채꼴**(액터 forward ± 반각). 병사는 그 안에서 가장 안 훑은 방위를 본다 | true / 45 |
| `PatrolWeight` / `PatrolStaleSeconds` | 홀드 중 "안 본 지 N s 넘은 땅"을 볼 수 있는 자리가 싸짐 → 경로 없는 순찰. 0 = 제자리 | 1.0 / 30 |
| `NavFilterClass` | 이 존으로 가는 모든 이동의 내비 필터(분대별 경로 스플라인 가중 = 분대별 존이 필요한 이유) | 없음 |
| `ZoneLabel` | 에디터 라벨 | |

`ScenarioConfig` 추가 필드: `SquadZones`(위) · `bSoldierLabHostilesStartHidden`(기본 true — 드론 `RevealEnemies` 가 풀기 전까지
UGV 탐지 레지스트리에서 숨김; 드론 없는 레벨은 false).

콘솔에서 DT 없이 명령: `titan.SquadOrder <Hostile|Friendly> <SquadId|*> <Verb> [ZoneIndex] [ROE] [Speed] [Aggression]`
(서버 월드에서). 로그 판정: `SoldierLab.Debug.Squad 1` · `.Squad.Log 1` → `[Squad] order/reached/achieved/reinforce`.

### 2.7 시나리오 재시작 — 확인창 · 자동 재시작 · `Scenario|Restart` 필드 (2026-09-22 신설)

시나리오 완료 뒤 "재시작 하시겠습니까?" 확인창을 띄우고, [예] 또는 자동 재시작 체크(10초 카운트다운)로 시나리오를
처음부터 다시 돌린다. 설계 근거는 `2026-09-10_scenario_auto_restart_design.md`, 구현/파일 목록은
`2026-09-22_scenario_restart_implementation.md`.

**재시작은 레벨 리로드가 아니라 인플레이스 리셋이다.** 같은 월드 안에서 병사(SoldierLab `BP_Soldier_*`, `USoldierHealthComponent`
보유 액터)와 `RespawnActors` 목록의 액터는 **스냅샷 기반 재스폰**(월드 시작 때 찍어 둔 클래스/트랜스폼/인스턴스 저작값으로
새로 스폰 — BeginPlay 가 전부 다시 돈다), UGV·이동형지휘소·드론은 **제자리 부활**(`IScenarioResettable::ResetForScenarioRestart` —
스폰 위치 텔레포트·탄약 리필·RCWS 모드/표적 초기화·드론 비행 리셋; RTSP 스트림은 안 끊긴다). 서브시스템(스텝 발동 이력·사망
카운트·분대 명령·상황 필드·투사체 풀)도 함께 초기화되며, 결과는 PIE 새로 시작과 같아야 한다. 리셋 뒤에는 레벨 시작과 같은 경로로
데모 자동 시작(`DemoAutoStartDelaySeconds` 뒤 `BeginEnemyContactScenario`)이 다시 돈다 — 풀 시스템이면 레벨 시작처럼 콘솔/통제기가 시작한다.

**DT 행 저작 예** (`DT_ScenarioSteps_ThreeStage_SoldierLab` · `DT_ScenarioSteps_SquadThreeStage` 둘 다 들어 있음):

| RowName | Prereq | Trigger | 값 | Effect | 비고 |
|---|---|---|---|---|---|
| `ScenarioRestartPrompt` | `ScenarioComplete` | TimerOnly | 5s | `ShowRestartPrompt` | 완료 토스트 5초 뒤 확인창. Prereq 를 `ScenarioComplete`(→ `EnemyEngage`)에 거는 게 **무한 재시작 방지 장치** — 새 사이클에서 UGV/적이 다시 쏘기 전엔 완료도 확인창도 못 뜬다. 끄려면 `bEnabled=false` 또는 `ScenarioConfig.bRestartPromptEnabled=false` |

**`ScenarioConfig ▸ Scenario|Restart` 필드**:

| 필드 | 기본 | 의미 |
|---|---|---|
| `bRestartPromptEnabled` | true | false 면 확인창이 안 뜸(풀 시스템에서 끄고 싶을 때). 콘솔 `titan.ScenarioRestart` / PC Exec `RequestScenarioRestart` 는 이 값과 무관하게 항상 동작한다(게이트는 확인창 표시에만 걸림) |
| `AutoRestartCountdownSeconds` | 10 | 자동 재시작 체크 시 확인창이 뜬 뒤 카운트다운 길이("예 (10)" → 0 에 재시작) |
| `RestartFadeOutSeconds` | 0.3 | 리셋 전 페이드 아웃(텔레포트·스폰 팝 가림) |
| `RestartFadeInSeconds` | 0.5 | 재스폰 완료 후 페이드 인 |
| `RestartRespawnPerFrame` | 8 | 한 프레임에 재스폰할 액터 수(GASP 캐릭터 40 동시 스폰 스파이크 회피) |
| `RespawnActors` | [] | 병사 외에 재스폰할 레벨 액터 목록. **New_kadex_0811 은 비워 둔다**(2026-09-23 사용자 확정) — 한때 낙하산 `BP_Parachute_C_3` 을 넣을 예정이었으나 **낙하산은 정적 액터라 재시작 때 초기화할 것이 없다**. 프로퍼티는 남겨 둔다: 병사가 아닌 재스폰 대상(런타임에 상태가 바뀌고 되돌려야 하는 레벨 액터)이 생기면 여기에 넣으면 된다 |

**확인창이 뜨는 화면**: UGV 호스트 + 자체방호 클라이언트 → **자체방호(클라이언트)에만** / 자체방호 단독 → 자체방호 / UGV 단독 → UGV /
데모·풀 시스템 무관 / 축이 없는 PIE·시험 레벨 → 무조건. 클라이언트에서 누른 [예]·체크값은 Server RPC 로 서버에 올라가고, 재시작이
시작되면 모든 화면의 확인창이 닫힌다. 위젯은 `/Game/widget/notify/WBP_RestartPrompt`(부모 C++ `URestartPromptWidget`, 위젯 이름
`YesButton`/`NoButton`/`AutoRestartCheckBox` 필수 — 규격은 구현 문서 §4).

**⚠ 레벨 GameMode 가 titan 계열이어야 한다 (2026-09-23 실사고)** — 재시작은 서버가 `Atitan_exampleGameState` 의 멀티캐스트 3종
(`Multicast_ScenarioRestartBegin` / `…Apply` / `…End`)으로 **각 프로세스에게 "자기 것을 치우고 자기가 시뮬하는 것을 리셋하라"**고 시킨다
(페이드는 로컬 `PlayerCameraManager`, 소총 `BP_AR4Rifle` 은 `bReplicates=false`, 드론은 시뮬 주체가 클라). 레벨 World Settings 의 GameMode
오버라이드가 titan 계열이 아니면 그 GameState 가 아예 없어서 **전부 서버-로컬 폴백**으로 빠진다 — 단일 프로세스에서는 그 폴백이 곧 정답이라
멀쩡하고, **2-PC 에서만** 클라가 페이드·정리·리셋을 하나도 못 받는다.

| 레벨 | GameMode 오버라이드 | 비고 |
|---|---|---|
| `New_kadex_0811` | **`BP_KadexTestGameMode`**(부모 `Atitan_exampleGameMode`) | 전시·2-PC 구성은 반드시 이것 |
| `kadex_test` | `BP_KadexTestGameMode` | 같음 |
| `kadex_lobby` | `BP_TestGameMode`(부모 `AGameModeBase`) | 대기실이라 시나리오와 무관 |
| (자유 관전) | `GM_SoldierLab` | 병사 거동을 관전 폰으로 볼 때만, **단일 프로세스**로. 이걸 켠 채 2-PC 로 돌리지 말 것 |

**자동 재시작 체크값**: 재시작·레벨 트래블·앱 재실행을 넘어 유지된다 — 정본은 서버의 `GameUserSettings.ini` `[Scenario] bAutoRestart`
(GameInstance 서브시스템 `UScenarioStateSubsystem` 이 시작 시 읽음). 커맨드라인 `-autorestart` 로도 켤 수 있다(전시장 무인 반복용).

**콘솔**:

| 명령 | 동작 |
|---|---|
| `titan.ScenarioRestart` | 확인창 없이 즉시 재시작(서버 월드) |
| `titan.ScenarioAutoRestart <0\|1>` | 자동 재시작 체크값 설정(ini 에 저장) |
| `SoldierLab.ResetWorld` | SoldierLab 쪽 리셋 계약만(분대·상황 필드·투사체 풀) — 시험 레벨 `L_SoldierScenario` 단독 검증용, 병사 재스폰·차량 부활은 안 함 |

### 2.8 시나리오 흐름 로그 · hard fail-safe (2026-09-22 신설)

무인 반복 실행(자동 재시작 ON)을 밤새 돌린 뒤 "어느 사이클이 어디서 멈췄나"를 찾기 위한 장치. 코드는
`UI/ScenarioMonitorSubsystem.h/.cpp`(서버 전용 월드 서브시스템), 배경은 `2026-09-22_scenario_flow_log_and_failsafe.md`.

**로그 파일** — `Saved/ScenarioLogs/<yyyy.mm.dd-hh.mm.ss>_<레벨>/` (AScenarioConfig 가 있는 레벨에서만, 클라이언트 프로세스는 안 만듦):

| 파일 | 내용 |
|---|---|
| `cycles.csv` | 사이클 1행 — `cycle, start_wall, end_wall, duration_s, result(Complete/CompleteAssisted/FailSafe/Restart/SessionEnd), last_step, last_step_offset_s, hostile_deaths, friendly_deaths, assists, timeouts, pruned, avg_fps, actors, rcws_rounds, soldier_rounds, restart_before_ms` + **DT 행마다 한 열**(그 사이클에서 발동한 +s, 미발동은 빈칸). 스텝별 소요 시간 통계는 이 파일로. `avg_fps`/`actors`/`projectiles` 는 장시간 실행의 성능 추이용 — 12시간 실행에서 fps 26→22 로 내려가며 물리 클램프 탓에 같은 주행이 87→97 s 로 늘었다(devlog §7.1) |
| `events.log` | `벽시계 \| C<사이클> +<사이클 내 초> \| 태그 \| 내용` 한 줄씩. 태그: `SCN`(사이클/스텝/분대 명령/재시작) · `UGV`/`CP`(자율주행 시작·종료, 드라이브 모드, RCWS 모드·ARM·표적 획득/상실/전환·**표적별 첫 사격**·탄 소진) · `UAV`(오토파일럿 상태, 낙하산 관측, 탐지 단계, 프레이밍, 짐벌 정찰, 수동 조종) · `SQD`(분대 명령 달성) · `SLD`(분대 편입, 작업 전이, **명령 뒤 첫 사격**, 사망 ← 누가) · `FAIL`(fail-safe) · `SNAP`(전 액터 상태 스냅샷) · `MARK`(사람 메모). 발사/피격은 개별 기록하지 않는다 — 위치·속도는 Chronicle 리플레이 몫 |

**클라이언트 모드 (2026-09-23)** — 클라이언트 프로세스(자체방호 PC)도 `…_client` 폴더에 따로 기록한다. 시나리오 진행(스텝·분대·사망 판정)은 전부 서버 몫이라 클라가 적는 것은 **"이 프로세스가 무엇을 받았나"** 뿐이고, 목적은 서버 로그와 **벽시계로 대조**해 *"서버는 쐈는데 클라는 못 봤다"*(릴리번시 컷·패킷 유실)를 증명하는 것이다.

| 클라가 적는 것 | 출처 |
|---|---|
| `SLD … 사격 수신(이 사이클 첫 발) — 시점까지 Nm` | `USoldierEngagementComponent::OnRemoteShotFired`(서버 `Multicast_ShotFired`, **Unreliable**) 도착 |
| `NET … 복제 정지 Ns — 시점까지 Nm (릴리번시 컷/패킷 유실 의심)` / `복제 재개` | 병사 위치가 15 s 이상 안 바뀌고 **시점에서 200 m 이상** 떨어져 있을 때(시체 제외). 거리 문턱은 2026-09-28 에 붙였다 — 그 전엔 사이클당 ~100건이 찍혔는데 엄폐 사격 중 제자리인 병사(정상)가 대부분이었다. 복제가 끊겨서 언 것이면 반드시 멀리 있다 |
| `SLD … 사망 수신 ← ?` | 사망 코스메틱(모든 프로세스에서 발동). 가해자는 복제 안 되므로 항상 `?` |
| `UGV`/`CP` RCWS 모드·표적·`사격 수신` | 전부 복제되는 값. 클라는 `ShotsFiredCount` 를 못 보므로 **탄약 감소**로 발사를 판정 |
| `UAV` 드론 상태 | 자체방호 클라가 **시뮬 주체**라 로컬 값이 정본 |
| 사이클 경계 | 재시작 멀티캐스트(`RunLocalRestartBegin/End`). 클라의 `cycles.csv` 열: `cycle, start_wall, end_wall, duration_s, result, shots_seen, shooters_seen, deaths_seen, frozen_soldiers, avg_fps, actors, rcws_rounds, soldier_rounds` |

클라에는 UGV 컨트롤러(`AUGVAIController`)가 없으므로 자율주행 시작/종료 줄은 서버 로그에만 있다.

**첫 실전 결론(2026-09-23)**: 클라는 40명 중 39명의 사격을 받고 있었고(1사이클 3,925발, 첫 수신 거리 중앙 355 m) — 즉 **릴리번시 컷은 "적이 안 쏜다"의 원인이 아니었다.** 진짜 원인은 병사가 차량(UGV/트럭)을 표적으로 제대로 못 보던 버그였다(CL 510, devlog §8.2). 이 로그의 값어치는 바로 이것 — *"서버는 쐈는데 클라가 못 받았다"와 "서버에서도 안 쐈다"를 갈라 주는 것*이다.

**3분대가 3차 구간에서 조용한 것은 설계다**(devlog §8.3): `Squad3Run` 의 `BreakContact` 가 ROE 를 HoldFire 로 덮고, `ExcludeFleeingEnemies` 의 `SetTargetable(false)` 때문에 아군·UGV 가 안 쏘니 `ReturnFireOnly` 도 안 열린다. `Squad3Stand`(트럭이 80 m 안에서 사격, 또는 Flee3 +240 s 타임아웃)가 걸려야 Free 가 된다 — 실측 침묵 구간 **130~180 s**. 구역 동사(`Occupy`)는 배정을 통째로 초기화하므로 그때 `bBreakContact`·`SetTargetable(false)` 가 함께 풀린다.

> **드론 트래킹도 같은 신호를 읽는다 (2026-09-23)** — 드론은 `bBreakContact` 가 켜진 동안 그 분대를 전장 트래킹과 타겟 디텍션에서 빼고, `Squad3Stand` 의 `Occupy` 로 풀리는 순간 다시 넣는다(이동형지휘소 RCWS 래치와 거의 동시). 그래서 **`Squad3Run` 을 옮기거나 `BreakContact` 를 `Withdraw` 로 바꾸면 드론 연출이 같이 바뀐다** — `Withdraw` 는 도주가 아니라 전투 이동이라 드론이 계속 따라간다. 드론 쪽 판정은 `vehicle/drone/drone_flight_dev_guide.md` 16.2-1절, 실사고는 `vehicle/drone/2026-09-23_drone_remote_rotor_and_squad_tracking.md` 3절.

사이클 번호는 첫 실행이 1, 재시작 #N 뒤가 N+1(엔진 로그의 `시나리오 재시작 #N` 과 맞춤). 사이클은 `BeginScenarioSteps`(스텝 평가 시작)에
서 시작하고 `RequestScenarioRestart` 가 수락되는 순간 닫힌다. "완료"는 `AllEnemiesEliminated` 트리거 행(`ScenarioComplete`)의 발동.

**hard fail-safe** — 사이클이 `ScenarioConfig.FailSafeMaxCycleSeconds`(기본 **1200** = 20분, 0 = 끔)를 넘도록 완료가 안 되면 `SNAP` 스냅샷을
남기고 `RequestScenarioRestart()` 를 부른다(적을 죽여 완료로 밀지 않는다 — `ScenarioComplete` 의 Prereq 가 `EnemyEngage` 라 그 전에
멈추면 전멸시켜도 완료가 안 뜨고, 데이터도 오염된다). 완료 뒤 `titan.ScenarioFailSafe.MaxWaitAfterCompleteSeconds`(기본 120) 안에 재시작이 안
와도 같은 조치. 스텝별 시간 예산은 아직 없다 — `cycles.csv` 로 분포를 본 뒤 넣는다.

| 콘솔 | 기본 | 동작 |
|---|---|---|
| `titan.ScenarioFailSafe.Mode <0\|1\|2>` | 1 | 0 끔 / 1 **자동 재시작 체크 ON 일 때만** / 2 항상. 사람이 관전·디버그 중엔 1 이면 안 끼어든다 |
| `titan.ScenarioFailSafe.MaxCycleSeconds <s>` | 0 | 0 보다 크면 `ScenarioConfig` 값 대신 이걸 씀 |
| `titan.ScenarioFailSafe.MaxWaitAfterCompleteSeconds <s>` | 120 | 완료 뒤 재시작 대기 상한(자동 재시작 ON 일 때만). 0 = 끔 |
| `titan.ScenarioLog.Enabled <0\|1>` | 1 | 0 이면 파일을 안 만든다(월드 시작 시점에 읽음). fail-safe 도 같이 꺼진다 |
| `titan.ScenarioLog.Mark <메모>` | | `events.log` 에 `MARK` 한 줄 — 관전하다 "여기 이상함" 을 박아 두는 용도 |
| `titan.ScenarioLog.Snapshot` | | `SNAP` 스냅샷 강제(차량·드론·병사 전원의 위치/상태/체력/배정) |

**스텝 fail-safe 사다리 (2026-09-22 설계, DT 부품 = `TimeoutSeconds` · `SkipIfStepFired` · `KillSquads`)** — 원칙: **정상 분포 안에서는 개입하지 않고(값 = 실측 최대 ×1.4~2), 분포 밖에서 티 안 나는 수단부터.** 1시간 실측(7사이클)에서 유일한 병목은 1분대 잔존(2차 존이 없어 zone 0 에 남는 정원 편입 탈락자)이 숨어 `UGVMoveZone2`(적 전원 55 m 밖)를 최대 176 s 막은 것. `New_kadex_0811` DT 에 넣는 행:

| 행 | 사다리 | Prereq | 트리거 | Timeout / Skip | 이펙트 | 근거(실측) |
|---|---|---|---|---|---|---|
| `EnemyFleeToZone2` 기존 | 1차 교전 상한 | **`EnemyEngage`로 변경**(원래 None — 타임아웃 기준을 "교전 시작"으로) | 사망 ≥3 | Timeout **120** | 기존 | Engage→Flee2 16~49 s |
| `Squad1LateWithdraw` 신규 | 1분대 1차 | `EnemyFleeToZone2` | TimerOnly 90 | — | 적군 [1] `Withdraw` zone 1 Rush ReturnFireOnly | Flee2→UGVMoveZone2 23~176 s. 게이트 불필요(정상 사이클에 그 시점 생존 1분대원은 같은 문제의 씨앗) |
| `Squad1LateRun` 신규 | 1분대 1차 | `Squad1LateWithdraw` | TimerOnly 2 | — | 적군 [1] `BreakContact` zone 1 Rush ReturnFireOnly | **`BreakContact` 는 존을 바꾸지 않는다**("the zone stays" — `SoldierSquadSubsystem.cpp` BreakContact 케이스). 강제 검증에서 BreakContact 만 내리니 1분대원이 zone 0 에 그대로 서 있었다 → `Withdraw` 로 존을 먼저 옮기고 +2 s 뒤 BreakContact(`EnemyFleeToZone3`→`Squad3Run` 과 같은 2행 패턴). 뛰면 UGV 시야에 들어와 죽거나 55 m 밖으로 빠지거나 — 둘 다 자연 진행 |
| `UGVMoveZone2` 기존 | 1분대 2차 | 기존 | 55 m | Timeout **240** | 기존 | "찾다가 포기하고 다음 목표로" |
| `EnemyFleeToZone3` 기존 | 2차 교전 상한 | 기존 Flee2 | 사망 ≥7 | Timeout **240** | 기존 | Flee2→Flee3 23~128 s |
| `Squad3Stand` 기존 | 3차 교전 상한 | 기존 Flee3 | 트럭 80 m 사격 | Timeout **240** | 기존 Occupy z2 | Flee3→Stand 126~151 s **로 일정** — 240 미만이면 정상 사이클에 걸린다 |
| `AllyClearZone2` 신규 | 2분대 1차 | `Squad3Stand` | TimerOnly **150**(첫 값 60 은 12시간 실행에서 38/85 발동 — 그중 33회가 "그냥 3차 교전이 길었을 뿐"이라 올림) | Skip if `ScenarioComplete` | 아군 [*] `Occupy` **zone 1**(`ZF_Zone2_Clear`, 2분대 2차 군집, yaw 180, r 1500) Rush Free 0.8 | 2차 실측(17:17 런 C8): 2분대 잔존 3명이 2차 존 엄폐에서 **340 s** 동안 UGV(ugvpoint2, 못 봄)·아군(70~95 m, 80 m 밖)·ROE(ReturnFireOnly) 셋 다에 안 걸려 교착 → 무적 아군 25명이 서쪽으로 밀고 들어가 소탕. 처음 "선택"으로 내렸던 행을 데이터가 되살렸다 |
| `Squad3Expose` 신규 | 3분대 1차 | `Squad3Stand` | TimerOnly 120 | Skip if `ScenarioComplete` | 적군 [3] `Occupy` **zone 3**(`Z3_S3_Kill`, 트럭 정면 개활지) Rush Free 0.8 | Stand→완료 44~83 s(2차 런 44~169) |
| `PruneAll` 신규 | 최종 | `Squad3Expose` | TimerOnly 90 | Skip if `ScenarioComplete` | `KillSquads` 적군 전원 | 어느 단계 잔존이든 여기서 정리 → 전멸 → 완료 |

레벨 쪽(2026-09-22 반영 완료): `SquadZones` 적군 "1" → `[Z0_S1_Engage, Z1_S2_Withdraw]`, 적군 "3" → `[…, Z2_S3_Escape, Z3_S3_Kill]`(`SoldierZone_8`, (55000,13200,−3742) 트럭 정면 22 m 평지, 반경 500), 아군 "1"~"5" → `[ZF_North/South, ZF_Zone2_Clear]`(`SoldierZone_9`, (15900,2200,600) = `Z1_S2_Withdraw` 와 같은 자리, yaw 180, 반경 1500, 필터 없음). 아군 무적은 `BP_Soldier_Friendly ▸ AC_SoldierHealth ▸ Invincible`(클래스 기본값 — 이 레벨엔 인스턴스 체크가 없어서 실제로 죽는다, 1시간 실측 아군 오사 2건).

`cycles.csv` 의 `result` 가 `CompleteAssisted` 면 그 사이클은 타임아웃/가지치기가 끼어든 것(`timeouts`/`pruned` 열) — 자연 완주(`Complete`)와 구분해서 볼 것. 사다리가 자주 걸리면 그건 fail-safe 가 아니라 근본 원인(페이싱·존 배치·정원 값)을 볼 신호다.

---

## 3. 레벨 셋업 체크리스트 (새 레벨에서 처음부터)

### 3.1 필수 액터

| # | 액터 | 필수 설정 | 없으면 |
|---|---|---|---|
| 1 | 아무 액터(보통 TargetPoint) | **Tag에 `EnemyCube`** | `BeginScenarioEnemyContact`가 실패하고 시나리오가 아예 시작 안 됨 (`no actor tagged 'EnemyCube'` 경고) |
| 2 | `ScenarioConfig` (Place Actors에서 검색) | 레벨에 **정확히 1개**. 필드는 3.4 | UGV 이동/지휘소 관련 이펙트가 전부 경고만 남기고 스킵 |
| 3 | UAV (`BP_UAV`) | 1대 | `BeginUAVMission` 경고 (`레벨에서 AUAVPawn을 못 찾음`) |
| 4 | UGV (`BP_UGV_Vehicle`) | RCWS + AI 컨트롤러 | UGV 이동/사격 스텝 전부 스킵 |
| 5 | 이동형지휘소 | RCWS를 가진 액터 | 3차 전투지 스텝만 안 돌고 나머지는 정상 |
| 6 | NavMeshBoundsVolume | 모든 이동 경로/마커를 덮을 것 | 적/아군이 목적지로 못 감 |

### 3.2 적군 개체마다 (`BP_Enemy_kadex` — `EnemyCombatComponent`)

**개체당 마커 6개**가 필요하다. `CombatZones` 배열을 3개로 만들고 각 원소에:

| 배열 | 의미 | FiringPose.Marker | CoverPose.Marker |
|---|---|---|---|
| `CombatZones[0]` | 1차 전투지 | 노출해서 사격할 위치 | 엄폐 위치 **= 접근 이동의 목적지** |
| `CombatZones[1]` | 2차 전투지 | 〃 | 〃 **= `BeginEnemyFleeZone2`의 도주 목적지** |
| `CombatZones[2]` | 3차 전투지 | 〃 | 〃 **= `BeginEnemyFleeZone3`의 도주 목적지** |

- 마커는 아무 액터나 가능하지만 **TargetPoint 권장**.
- **마커의 회전(Yaw)도 의미가 있다** — 엄폐(Cover) 자세일 때 그 방향을 바라본다.
  사격(Firing) 자세일 때는 타겟을 향하므로 회전 무시.
- 포즈마다 `BodyPose`(Standing / Crouched / Prone), `Lean`(None / Left / Right) 지정.
- 엄폐↔사격은 **두 마커 사이를 실제로 걸어서** 오간다. 두 지점이 너무 멀면 사이클이 느려지니
  1~3m 정도 권장(현재 레벨은 약 2m).
- `AllyDetectRange` 스피어의 `SphereRadius` = 적이 아군/차량을 인지하는 거리(현재 20000 = 200m).
- `bAutoBeginMoveForTesting`은 **꺼둘 것**(켜면 시나리오와 무관하게 BeginPlay에서 출발).

> 팁: 개체별로 zone을 다 채우기 번거로우면, 한 개체를 완성한 뒤 복제하고 마커만 교체하는 게 빠르다.
> zone 마커가 비어 있는 개체는 해당 도주 명령을 **조용히 무시**하고 그 자리에 남는다.

### 3.3 아군 개체마다 (`BP_Ally_kadex` — `AllyFormationComponent`)

| 값 | 의미 |
|---|---|
| `AmbushMarker` | 매복 지점(이 위치로 이동해서 교전) |
| `FiringPose` / `CoverPose` / `NoTargetPose` | 각 Marker+BodyPose+Lean. 적군과 달리 **zone 개념 없이 1세트** |
| `bIsSquadLeader` | 분대장 1명. 수신호 몽타주(`DeploySignalMontage`, `TakeCoverSignalMontage`) 지정 |
| `SquadId`, `DefaultWatchYawDeg` | 분대 구분 / 대기 시 바라보는 방향 |
| `EnemyDetectRange` 반경 | 아군이 적을 인지하는 거리(현재 20000) |
| `bAutoBeginAmbushForTesting` | **꺼둘 것** (켜면 시작하자마자 교전) |

> 2차 전투지는 "아군 시야에 들어오는 지역"이어야 하므로, **아군 매복 지점과 적군
> `CombatZones[1]`의 위치 관계**가 이 단계 연출의 핵심이다.

### 3.4 `ScenarioConfig` 필드

| 필드 | 넣는 것 | 쓰는 이펙트/트리거 |
|---|---|---|
| `UGVFormUpDestination` | UGV **1차 목적지** TargetPoint | `MoveUGVToZone1Destination` |
| `UGVZone2Destination` | UGV 2차 목적지 | `MoveUGVToZone2Destination` |
| `UGVZone3Destination` | UGV 3차 목적지 | `MoveUGVToZone3Destination` |
| `UGVStandbyDestination` | (구 흐름 전용, 비워도 됨) | `BroadcastApproach` |
| `CommandPost` | 이동형지휘소 액터 | `SetCommandPostAutoFire`, `CommandPostFiredNearEnemy` |
| `ScenarioStepTable` | 이 레벨의 스텝 DataTable | 전체 |
| `Scenario|Restart` 6종(`bRestartPromptEnabled` · `AutoRestartCountdownSeconds` · `RestartFadeOutSeconds` · `RestartFadeInSeconds` · `RestartRespawnPerFrame` · `RespawnActors`) | 재시작 확인창/재스폰 설정 — 2.7절 | `ShowRestartPrompt` |
| `FailSafeMaxCycleSeconds` | (2026-09-22) 사이클 최대 시간(기본 1200s) — 넘으면 강제 재시작. 2.8절 | (fail-safe) |

**UGV 1차 목적지 잡는 법**: scenario.md 요구사항이 "적군이 1차 전투지에 도달하기보다 **UGV가
1차 목적지에 먼저 도착**"이므로, UGV 주행 거리/속도와 적군 `PatrolMoveSpeed`(150)를 비교해
UGV가 먼저 도착하도록 잡는다. 또 `UGVFiredNearEnemy`의 거리 임계값 안에 들어와야 1차 교전이
시작되므로, **목적지 ↔ 적군 1차 전투지 거리 < 그 임계값**이 되어야 한다.

### 3.5 실행 모드 — 데모 / 풀 시스템 (2026-09-01 신설)

같은 `ScenarioConfig` 액터의 `Scenario|Run Mode` 카테고리. 상세 배경은
`2026-09-01_scenario_run_modes_demo_fullsystem.md`.

| 필드 | 기본 | 의미 |
|---|---|---|
| `RunMode` | `FullSystem` | `Demo`면 아래 3가지가 한꺼번에 켜짐 |
| `bDemoForceUGVAutoFire` | true | UGV RCWS를 ARM+AutoFire로 강제 — 시점은 DT 행 `UGVArriveZone1`(1차 목적지 도착 시, 2026-09-15). 이 플래그는 on/off만 |
| `bDemoForceCommandPostAutoFire` | true | 이동형지휘소 RCWS도 동일 |
| `bDemoAutoStartScenario` | true | 콘솔 `BeginScenarioEnemyContact` 없이 자동 시작 |
| `DemoAutoStartDelaySeconds` | 3.0 | 자동 시작까지 대기 |

- **FullSystem**: 실제 납품 구성(PC 2대 + 통제기/상위체계). RCWS 조준·사격은 통제기 SW가 쥐고,
  시나리오 시스템은 적군 행동만 담당. 그래서 `UGVSurveillance`/`UGVAutoFire` 행은 꺼둔다.
- **Demo**: 전시/데모용. 통제기 연동(`UUGVRemoteControlSubsystem`)을 **소켓째로 끄고**, RCWS를
  자동사격으로 켜두고, 시나리오를 자동 시작한다 — 언리얼 프로세스 하나만 켜도(PIE 포함)
  전체 흐름이 끝까지 돈다.
  - ⚠️ (2026-09-15) RCWS 자동사격 시점이 둘로 갈린다: **이동형지휘소는 레벨 시작 시**(코드
    `ApplyDemoRCWSAutoFire`), **UGV는 1차 목적지 도착 시** — DT 행 `UGVArriveZone1`
    (Prereq `UAVSpotted`, `ActorStopped`, 이펙트 `SetDemoUGVAutoFire`). 도착 전 UGV 포탑은
    `Remote` 모드라 스윕도 사격도 안 한다. 예전처럼 시작 즉시 켜려면 그 행을 Prereq 없음 +
    `TimerOnly` 0초로. 상세 `2026-09-15_demo_ugv_autofire_on_zone1_arrival.md`.
**우선순위(서버 기준)**: 커맨드라인 `-demo`/`-fullsystem` > 접속 URL `?Demo=` > 액터 `RunMode`.
**클라이언트는 무조건 서버 값을 따른다**(`GameState::bDemoRunMode`가 리플리케이트) — 2 PC에서
한쪽만 데모로 켜지는 사고가 안 난다.

**시작 방법 3가지**(대기실 `kadex_lobby`의 축 선택 화면 = `WBP_AxisSelection2`):

| 버튼 | 결과 | 데모 체크박스 |
|---|---|---|
| `HostButton` | `open <Map>?Listen?Axis=UGV?Demo=<0\|1>` — UGV축 리슨서버 | 적용됨 |
| `ClientButton` + IP | `open <IP>?Axis=SelfDefense` — 서버가 떠 있어야 함 | **무시**(서버가 정함) |
| `SoloButton` | `open <Map>?Axis=SelfDefense?Demo=<0\|1>` — **호스트 없이 단독**(1 PC 전시용) | 적용됨(기본 체크) |

콘솔로도 동일: `HostListenServer UGV 1` / `StartSoloAxis SelfDefense 1` / `ConnectToHost <IP> SelfDefense`.

- Solo가 **자체방호축**인 이유: 알림 토스트·미니맵이 붙는 `SelfDefenseMonitor1`이 이 축에서만
  생성된다. UGV축 단독으로 띄우면 시나리오는 다 돌지만 알림이 아무데도 안 뜬다.
- ⚠️ Solo에 `?Listen`을 붙이면 안 된다 — 자체방호축이 리슨서버가 되면 "UGV PC = 호스트" 전제가
  깨지고 `UUGVRemoteControlSubsystem`(UGV축+서버에서만 활성)이 영영 안 붙는다.
- 데모에서 RCWS를 미리 자동사격으로 켜둬도 **순서는 앞당겨지지 않는다** — 적군은 `RevealEnemies`
  전까지 `bIsRevealed=false`이고 `TargetDetectionComponent`가 그런 대상은 스캔에서 통째로 제외한다.
- ⚠️ 데모 모드에서도 **적군 행동은 여전히 스텝 테이블이 쥔다** — 이 스위치는 "누가 RCWS를
  조작하는가"만 바꾼다.

### 3.5 최종 점검

- [ ] `EnemyCube` 태그 액터 1개
- [ ] `ScenarioConfig` 1개 + 필드 6종
- [ ] UAV / UGV / (지휘소) 배치
- [ ] 적 개체마다 zone 3개 × 마커 2개
- [ ] 아군 개체마다 매복 마커 + 자세 3종, 분대장 1명
- [ ] 테스트 자동시작 플래그 2종 모두 OFF
- [ ] NavMesh가 모든 마커/경로를 덮는지 (`P` 키로 확인)
- [ ] 스텝 테이블이 `ScenarioConfig`에 연결됐는지

---

## 4. 현재 3단계 시나리오 테이블 (`DT_ScenarioSteps_ThreeStage`, 17행)

| RowName | Prereq | Trigger | 값 | Effect | 이 행이 요구하는 레벨 값 |
|---|---|---|---|---|---|
| `UAVMission` | — | TimerOnly | 3s | BeginUAVMission | EnemyCube 태그, UAV |
| `EnemyApproach` | — | TimerOnly | 1s | BeginEnemyEngagementApproach | 적 `CombatZones[0]` |
| `UAVSpotted` | UAVMission | UAVEnemyDetected | — | MoveUGVToZone1Destination | `UGVFormUpDestination` |
| `RevealEnemies` | UAVSpotted | TimerOnly | 0s | RevealEnemies | — |
| `UAVDetectionOff` | UAVSpotted | TimerOnly | 2s | DisableUAVTargetDetection | UAV |
| `UGVSurveillance` | UAVSpotted | TimerOnly | 0s | SetUGVAutoSurveillance | UGV RCWS |
| `UGVAutoFire` | UGVSurveillance | EnemyDetected | — | SetUGVAutoFire | UGV RCWS |
| `UGVArriveZone1` (2026-09-15) | UAVSpotted | **ActorStopped** | — | SetDemoUGVAutoFire (⚠️ 빌드 전엔 `None` — 빌드 후 설정 필요) | UGV + AI 컨트롤러, `RunMode=Demo`(아니면 no-op) |
| `EnemyEngage` | **—** | **UGVFiredNearEnemy** | 6000 | BeginEnemyEngage | UGV가 적 60m 이내에서 사격 |
| `EnemyFleeToZone2` | **—** | **EnemyCasualtyCountAtLeast** | **3** | BeginEnemyFleeZone2 | 적 `CombatZones[1]` |
| `UGVMoveZone2` | EnemyFleeToZone2 | LeaderDistanceFromEnemyAtLeast | 5500 | MoveUGVToZone2Destination | `UGVZone2Destination` |
| `AllyAmbush` | EnemyFleeToZone2 | TimerOnly | 5s | BroadcastAmbush | 아군 매복 마커 |
| `RetargetToAllies` | EnemyFleeToZone2 | **AllyFireStarted** | — | RetargetEnemiesToAllies | 아군이 적을 보고 쏠 수 있는 배치 |
| `EnemyFleeToZone3` | EnemyFleeToZone2 | **EnemyCasualtyCountAtLeast** | **7** | BeginEnemyFleeZone3 | 적 `CombatZones[2]`. ⚠️ 누적 카운트라 2차 도주 임계값보다 **커야** 함 |
| `UGVMoveZone3` | EnemyFleeToZone3 | LeaderDistanceFromEnemyAtLeast | 7200 | MoveUGVToZone3Destination | `UGVZone3Destination` |
| `CommandPostFire` | EnemyFleeToZone3 | TimerOnly | 3s | SetCommandPostAutoFire | `CommandPost` |
| `RetargetToCommandPost` | EnemyFleeToZone3 | **CommandPostFiredNearEnemy** | 8000 | RetargetEnemiesToCommandPost | 지휘소가 적 80m 이내에서 사격 |
| `ScenarioComplete` | EnemyEngage | AllEnemiesEliminated | — | ShowUIMessage(`ScenarioComplete`) | `DT_NotificationWidgets` |

**체인 구조(2026-08-23 재배선)**: 적군 행동 3행(`EnemyEngage` → `EnemyFleeToZone2` →
`EnemyFleeToZone3`)만 서로 묶여 있고 UAV/UGV/아군 스텝에는 의존하지 않는다. UAV·UGV 행을
테스트로 꺼도 적군은 조건만 맞으면 정상 반응한다. 반대로 UGV 후속 이동/아군 매복/지휘소
사격 행은 적군 행동 행을 Prereq로 삼는다(적이 도주해야 의미가 있으므로).

거리 값은 kadex_test 지오메트리 기준 — **새 레벨에서는 반드시 재계산**할 것
(요령: 각 단계에서 "UGV ↔ 최근접 적" 실제 거리를 재고, 단계 사이 값의 중간쯤을 임계값으로).

---

## 5. 자주 하는 커스텀

**① 다음 전투지로 도망가는 사망자 수 바꾸기**
`EnemyFleeToZone2` / `EnemyFleeToZone3` 행의 `TriggerCountThreshold` (현재 3 / 7, **누적** 기준).

**② UGV 자동사격 전환을 끄고 수동 사격으로 진행**
`UGVSurveillance` / `UGVAutoFire` 행의 `bEnabled` 해제 → `EnemyEngage`의 `PrerequisiteStepId`를
`UAVSpotted`로 변경(안 그러면 뒤가 전부 멈춤). 발사 카운터는 자동/수동을 구분하지 않으므로
**조작자가 직접 쏴도 `UGVFiredNearEnemy`는 정상 발동**한다.

**③ 1차 교전 트리거를 "거리 무관 최초 사격"으로**
`EnemyEngage`의 `TriggerDistanceThreshold`를 아주 크게(예: 100000).

**④ 특정 단계에서 흐름 멈추고 관찰**
멈추고 싶은 지점 스텝의 `bEnabled` 해제 → 그 뒤 스텝은 전부 대기.

**⑤ 전투지를 2단계로 줄이기**
`EnemyFleeToZone3` 이후 행들의 `bEnabled` 해제, `ScenarioComplete`의 Prereq를
`RetargetToAllies`로 변경.

**⑥ 적 인원 늘리기**
적 개체를 복제하고 **zone 3개 × 마커 2개**를 새로 배치·연결. 사망자 수 임계값도 인원에 맞춰 조정.

**⑦ 단계 사이 여유 시간 주기**
`EffectType=None` + `TimerOnly` 행을 끼우고, 뒤 스텝의 Prereq를 그 행으로 변경.

**⑧ 도주가 너무 우르르 몰릴 때**
적 `Min/MaxFleeCommitDelaySeconds`(기본 0.5~3) 범위를 넓히면 더 흩어져서 빠진다.

---

## 6. 트러블슈팅

로그 카테고리 `Logtitan_example`, 전부 `[ScenarioStateSubsystem]` 접두어.

| 증상 | 확인 | 원인/해결 |
|---|---|---|
| 아무 스텝도 안 켜짐 | `시나리오 스텝 발동:` 로그가 하나도 없음 | `BeginScenarioEnemyContact`를 안 쳤거나, `no actor tagged 'EnemyCube'` 경고 → 태그 액터 배치 |
| 〃 | `ScenarioStepTable이 없어서...` 경고 | `ScenarioConfig.ScenarioStepTable` 연결 |
| 특정 스텝부터 안 넘어감 | 마지막으로 찍힌 발동 로그 확인 | 그 다음 행의 Prereq/트리거 조건 점검. `bEnabled` 꺼져 있는지도 확인 |
| **테스트로 몇 행을 껐더니 적군이 아무 반응 없음** | 껐던 행이 적군 행동 행의 Prereq 체인에 있는지 | 꺼진 스텝은 발동하지 않으므로 **그걸 Prereq로 삼은 뒤 스텝은 전부 영구 대기**. 적군 행동 행의 Prereq를 비우거나 다른 적군 행동 행으로 옮길 것(2.5 규칙) |
| 적군이 화면에 안 보임 | `RevealEnemies` 행의 Prereq(`UAVSpotted`)가 꺼져 있는지 | UAV 단계를 꺼둔 상태면 reveal이 안 됨. 콘솔 `titan.DebugRevealAllEnemies 1`로 강제로 보이게 할 수 있음 |
| PIE 켜자마자 교전 | — | 적 `bAutoBeginMoveForTesting` / 아군 `bAutoBeginAmbushForTesting` 확인 |
| UGV가 안 움직임 | `MoveUGVToZoneN...: AScenarioConfig(또는 그 목적지 필드)가 비어있음` | 목적지 필드 연결 |
| 〃 | `UGV 또는 AUGVAIController를 못 찾음` | UGV 배치/컨트롤러 확인 |
| 적이 도주 안 함 | 로그엔 스텝이 찍힘 | 해당 `CombatZones[N]` 마커가 비었음(비면 조용히 무시됨) |
| **적이 3차 전투지로 도주 안 함 (자체방호 축에서만)** | `EnemyApproach`/`UAVMission` 발동 로그가 **두 번씩** 찍혔는지 | 레벨 트래블(`open ...?Axis=SelfDefense`) 직후 스텝 평가 루프가 두 벌로 돌면서 `BeginScenarioSteps`가 재실행돼 `ScenarioEnemyCountBaseline`이 "지금 살아있는 적 수"로 다시 잡힌다 → 누적 사망자가 임계값에 영영 못 닿음. 2026-09-02 수정(`ResetForNewWorldIfNeeded`가 옛 월드 타이머를 `ClearTimer`, 재진입 가드에 `IsTimerActive` 추가). 그 이전 빌드면 UGV 축에선 멀쩡한데 자체방호 축에서만 재현됨 |
| 지휘소 단계가 안 돎 | `SetCommandPostAutoFire: AScenarioConfig::CommandPost가 비어있음` | `CommandPost` 지정 |
| 적이 안 쏨 | — | 정상일 수 있음 — `BeginEnemyEngage` 전에는 **의도적으로** 감지/사격이 꺼져 있다 |
| **PIE로 켰는데 1차 교전에서 멈춤** | `UGVSurveillance`/`UGVAutoFire` 행의 `bEnabled` | 풀 시스템 구성에선 통제기 SW가 사격 주체라 두 행이 꺼져 있다 → 아무도 안 쏘니 `UGVFiredNearEnemy`가 안 켜짐. 혼자 돌리려면 `RunMode=Demo`(§3.5) |
| 클라이언트(자체방호 PC)에서 시나리오를 시작했는데 서버 쪽이 안 움직임 | — | 2026-09-01 이전 빌드의 문제. 지금은 `BeginScenarioEnemyContact`가 Server RPC로 넘어간다. 스텝 평가 자체는 서버 전용이라 **클라 화면엔 알림 위젯이 안 뜬다**(리플리케이트되는 건 실제 움직임뿐) |
| 적이 목적지로 못 감 | 적 컴포넌트 경고 `이동 중 N초간 …밖에 이동 못함(정체 의심)` | NavMesh 미커버/막힘 |
| **재시작 뒤 병사가 안 움직임** (2026-09-22) | 재스폰된 `BP_Soldier_*` 에 AI 컨트롤러가 없음 | 병사 BP 가 `AutoPossessAI=PlacedInWorld` 라 **스폰된** 폰엔 컨트롤러가 안 붙는다. `UScenarioRespawnSubsystem` 이 지연 스폰 중 `PlacedInWorldOrSpawned` 로 바꿔 `FinishSpawning` 하므로 현재 빌드에선 해결 — 다시 보이면 재스폰 서브시스템의 그 처리(안전망 `SpawnDefaultController`)를 확인 |
| **재시작 뒤 `BP_AR4Rifle … OwningCharacter is not valid` 스팸 + 사이클마다 GT 지연 증가** (2026-09-22) | 재시작 전 병사가 스폰한 소총 액터가 남아 있음 | 병사만 Destroy 하면 손에 붙인 소총이 죽은 주인을 매 틱 참조하고 사이클마다 두 배로 는다. 재스폰 서브시스템이 딸린 액터(`GetAttachedActors` 재귀 + Owner 기준)를 같이 파괴하므로 현재 빌드에선 해결. ※ **2026-09-23: 이건 재시작만의 문제가 아니라 평상시 사망 경로에도 있던 누수였다** — `USoldierHealthComponent::EndPlay` + `bDestroyCarriedActorsOnDestroy`(기본 켬)로 SoldierLab 쪽에서 근본 수정([W116]). titan 재시작 쪽 정리는 이제 안전망 |
| 재시작 뒤 `UAVSpotted` 가 즉시 뜸 / `UGVArriveZone1` 이 안 뜸 / UGV 가 출발 전부터 스윕 | 2회차 스텝 로그를 1회차와 대조 | 각각 드론 `bParachuteObserved` 리셋 · UGV 컨트롤러 `IsMoving()` 리셋 · RCWS 모드(Remote) 리셋 누락 의심 — `IScenarioResettable::ResetForScenarioRestart` 구현부(구현 문서 §2) 확인. 2-PC 면 **드론 쪽은 시뮬 주체(자체방호 클라)가 리셋한다** — `bParachuteObserved` 는 복제되지 않고 시뮬 주체에서만 갱신되므로, 서버만 리셋하면 안 된다 |
| **2-PC 에서 클라이언트만 페이드가 안 되고 재시작 뒤 드론이 원위치로 안 돌아옴** (2026-09-23) | 로그에 `[ScenarioStateSubsystem] 재시작: 이 월드의 GameState 가 Atitan_exampleGameState 가 아님(게임 모드=…)` 경고 | **레벨 GameMode 오버라이드가 titan 계열이 아니다**(실사고: 09-21 SoldierLab 이관 때 `GM_SoldierLab` 로 바뀌어 있었음). 그러면 재시작의 멀티캐스트 3종이 전부 서버-로컬 폴백이 되어 클라가 페이드·정리·리셋을 못 받고, `Atitan_examplePlayerController` 도 없어 축/데모 플래그가 안 정해져 드론이 **양쪽 다 시뮬 주체**가 된다. 해결: World Settings ▸ GameMode Override 를 **`BP_KadexTestGameMode`** 로(2.7절 표). 단일 프로세스에서는 증상이 안 나타난다 |
| **2회차 짐벌 배율(화각)이 1회차와 다름 — 재시작 직후 드론 화면이 확대/축소된 상태로 시작** (2026-09-23) | 1회차 시작 직후와 2회차 시작 직후의 드론 영상 화각 대조 | 드론 `ADronePawn::ZoomLevel` 이 리셋에서 안 되돌려졌다. `ZoomLevel` 은 저작값(기본 1.0)이지만 자동 정찰/교전 프레이밍이 런타임에 계속 바꾸는데, 리셋이 전환 플래그(`bManualZoomTransitionActive`)만 끄고 값은 그대로 둬서 지난 사이클이 끝난 배율로 시작했다(정찰 램프가 천천히 수렴하므로 **첫 몇 초만** 다르다). 해결: `InitialZoomLevel` 스냅샷 → 시뮬 주체 구간에서 `SetZoomLevel(InitialZoomLevel)` — **수정이 반영된 빌드가 필요하다**(2026-09-23 시점 아직 빌드 전). 2-PC 면 짐벌은 **시뮬 주체(자체방호 클라)** 가 되돌린다 |
| **2회차 배터리/비행시간/주행거리가 1회차에 이어서 표시됨** (2026-09-23) | 2회차 시작 직후 드론 상태 패널의 배터리·비행시간, UGV 대시보드의 배터리·누적 주행거리 | 상태 패널 컴포넌트가 재시작 리셋 계약에 빠져 있었다 — `UStatusHUDComponent`(드론·트럭)/`UUGVStatusComponent`(UGV) 의 `ElapsedTime` 이 영원히 누적되고(배터리는 거기서 파생) 비행시간·주행거리도 계속 쌓인다. 배터리 0 이어도 비행이 멈추지는 않아 기능 장애는 아니지만 "재시작 = 레벨 시작과 같은 상태"에 어긋남. 해결: 두 컴포넌트에 `ResetForScenarioRestart()` 신설 + `AUGVAIController::TankTotalDistanceTraveledCm=0` — **수정이 반영된 빌드가 필요하다**(2026-09-23 시점 아직 빌드 전). 확인법: 2회차 시작 시 **배터리 100 % · 비행시간 00:00 · 주행거리 0**. 2-PC 면 드론 패널은 **호스트·클라 양쪽 화면 모두** 확인할 것(드론 패널 값은 복제되지 않고 프로세스마다 따로 누적된다 — 그래서 드론만 모든 프로세스가 각자 리셋한다. UGV 패널은 복제되므로 서버만 리셋) |
| 재시작 뒤 `EnemyFleeToZone2` 가 곧바로 발동 | 재스폰 전에 스텝 평가가 돌았는지 | 재시작은 "평가 루프 ClearTimer → 재스폰 완료 → 3초 뒤 시작" 순서여야 한다(사망 = Baseline − Alive 라 재스폰 전 평가는 즉시 참). `RequestScenarioRestart` 이외의 경로로 리셋하면 이 순서가 깨진다 |
| 재시작이 실패했다는 에러 + 확인창 재표시 | `[ScenarioStateSubsystem]` 검증 로그(등록부 적/아군 수, 탐지 레지스트리 적 수) | 재스폰 수가 스냅샷과 어긋남. `[ScenarioRespawn]` 스냅샷/델타 로그와 대조, [예]로 재시도 가능 |

---

## 7. 부록 — 콘솔 명령 / BP 호출

**콘솔(Exec)로 바로 되는 것**
- `BeginScenarioEnemyContact` — 시나리오 시작(스텝 평가 개시)
- `BeginEnemyFlee` — 적 전원 zone 1(2차 전투지)로 도주
- `BeginAllyApproach` / `BeginAllyAmbush` / `BeginAllyFollowing`
- `BeginAllyFormUpAndAdvance (X=..,Y=..,Z=..)` / `SetUGVStandbyDestination (X=..,Y=..,Z=..)`
- (2026-09-22) `titan.ScenarioRestart` — 확인창 없이 즉시 재시작 / `titan.ScenarioAutoRestart <0|1>` — 자동 재시작 체크값 /
  `SoldierLab.ResetWorld` — SoldierLab 리셋 계약만(2.7절). PlayerController Exec `RequestScenarioRestart` / `SetScenarioAutoRestart <0|1>` 은
  클라이언트에서도 되며 서버로 RPC 된다. 커맨드라인 `-autorestart`.

**BlueprintCallable만 가능(콘솔 불가)** — 필요하면 레벨 BP/디버그 위젯에서 호출
- `UScenarioStateSubsystem::FireScenarioStep(StepId)` — `Manual` 트리거 스텝 강제 발동
- `UScenarioStateSubsystem::RequestScenarioRestart()` — 재시작 유일 진입점(서버). 향후 HUD 버튼도 이걸 부를 것
- `BeginEnemyEngage()` / `BeginEnemyFleeToZone(int32)` / `RetargetAllEnemies()`
- `UEnemyCombatComponent::BeginMove(zone)` / `BeginEngageAtCurrentZone()` / `BeginFlee(zone)` / `ForceRetarget()`
