# 분대 명령 층 — 첫 PIE 수정 여섯: 도착선 일원화 · 존 에디터 표시 · 차량 분대 제외 · 정원제(대타) · `EngageRangeCm` · 새 트리거 2 · RCWS 청각 브리지

2026-09-18 / 진행중(코드·빌드 완료 · PIE 일부 확인 · 완주 판정 대기) / 09-17 명령 층을 시험 레벨 `L_SoldierScenario`에서 처음 돌리자 나온 문제 — 병사가 존 경계 3~4 m 밖에 정착해 명령이 안 끝남, 차량이 아군 분대원으로 잡힘, 스폰 지점에서 92 m 밖 UGV에 사격해 접근 단계 소멸 — 를 고치고, 구 시나리오의 정원제·대타와 RCWS 청각 보조를 SoldierLab 병사에도 연결했다. 병사의 사격 결정 게이트 정리(5절)와 "접근 중 앉아 걷기" 제안(6절)은 미구현.

선행: `2026-09-17_command_layer_design.md`(설계·구현 — 이 문서가 그 3절 `ArrivalInsetFraction`·3.1절 존·6절 시나리오 연결을 **정정**한다) · 시나리오 쪽 `../level_new_kadex_0811/2026-09-18_soldierlab_three_stage_test_level.md`(레벨·DT·검증 순서) · 구 정원제 `../ai_combat/2026-09-03_dynamic_squad_reassignment_and_casualty_log.md` · RCWS 청각 `../rcws/2026-09-17_rcws_gunfire_hearing.md`.
틱 순환 경고·디버그 좌표축은 별도: `animation/prototypes/2026-09-18_tick_cycle_warning_charmovecomp.md`.

> 신뢰도: **[A]** 코드 실측(2026-09-18 문서 세션이 `Source/` 를 다시 읽었다 — 줄 번호는 그 시점) · **[B]** 잠정 · **[C]** 미측정. ⚠ `SoldierEngagement`/`SoldierCover`/`SoldierOrderTypes.h` 는 **같은 시각 다른 세션(상황 필드·순찰)이 편집 중**이라, 이 문서에 없는 필드(`PatrolWeight`/`PatrolStaleSeconds`, 섹터 = 호(arc), `CombatStanceFloor`)가 소스에 있다 — 그쪽 문서 몫.

> ★ **2026-09-21 정정/추가** → **`2026-09-21_break_contact_and_targeting_exclusion.md`** · `../../level_new_kadex_0811/2026-09-21_soldierlab_migration_new_kadex_0811.md`.
> - **8.1절 `MinStance` 제안은 09-18 오후 다른 세션이 SoldierLab 쪽을 구현했다**(`SoldierOrderTypes.h:136-147` · `SoldierSquadSubsystem.cpp:267` · `SoldierEngagement.cpp:857-862`, Rush 면 무시) — 남은 것은 titan `FScenarioSquadOrderSpec`/콘솔/존 연결([W84] 절반). 09-21 의 `BreakContact` 가 `DesiredStance=0` 을 그 바닥 **앞**에 두므로 연결 시 `bBreakContact` 면 바닥을 건너뛸 것.
> - **동사 하나 추가: `BreakContact`** — `Withdraw` 는 "싸우며 물러남"(엄폐·위험·제압을 계속 셈)이라 진짜 도주가 아니다. 3차 도주는 `Withdraw`(정원) → `BreakContact HoldFire Rush` → 트럭이 쏘면 `Occupy Free` 세 행.
> - **표적 제외의 두 소비자** — `SetTargetable(false)` 가 이제 UGV RCWS `bRespectEnemyTargetingExclusion` 도 켜고(구 이펙트가 하던 반쪽), SoldierLab 보병은 `IsContactExcluded` 로 직접 읽는다. 09-17 연결 문서 1절의 "브리지 → `bTargetableByFriendlyForces`" 만으로는 UGV 가 안 지켰다(스위치가 인스턴스 기본 false).
> - 8절 게이트 표에 **제외**(believed 앞, `PickCandidate` 단계)와 **BreakContact**(자세 0 · 스프린트) 가 추가됐다.
> - New_kadex_0811 본 레벨은 `EnemyEngage` 를 6절의 `EnemyFireStarted` 가 아니라 원래 `UGVFiredNearEnemy` 로 뒀다(접근 ROE 가 HoldFire) — [C-145] 는 시험 레벨 전용 질문으로 남는다. **[W70] 해결**(본 레벨 이관 완료).

---

## 0. 한 장 요약

| # | 무엇 | 어디 | 상태 |
|---|---|---|---|
| 1 | **도착선 일원화** — `ArrivalFraction` 이 존→명령→배정으로 흐르고 분대 층·엄폐 층이 같은 선을 읽는다. `USoldierSquadSubsystem::ArrivalInsetFraction` 삭제 | `Squad/SoldierOrderTypes.h:116-124, 247-249` · `Squad/SoldierZone.h:68-74` · `Squad/SoldierSquadSubsystem.cpp:261, 425-438` · `AI/SoldierCover.cpp:165-179` | [A] 코드 · 판정 [C-144] |
| 2 | **`ASoldierZone` 에디터 표시** — 반경(노랑)·도착선(초록)·밴드(흐림) 구 + 섹터 화살표 + `S_TargetPoint` 스프라이트, 전부 에디터 전용 | `Squad/SoldierZone.{h,cpp}` | [A] |
| 3 | **`USoldierIdentityComponent::bTakesSquadOrders`** — 브리지가 차량에 붙이는 Identity 는 false. `GetMembers` 가 거른다 | `AI/SoldierIdentity.h:70` · `Squad/SoldierSquadSubsystem.cpp:119-125` · `titan_example/Soldiers/SoldierLabBridgeSubsystem.cpp:176-178` | [A] |
| 4 | **정원제·대타** `ReinforceSquads` + DT 필드 `Quota` | `Squad/SoldierSquadSubsystem.{h:69-79, cpp:141-220}` · `titan_example/UI/ScenarioStepTypes.h:188-195` · `ScenarioStateSubsystem.cpp:1923-1940` | [A] 코드 · 판정 5절 체크리스트 |
| 5 | **`EngageRangeCm`** — ROE 의 사거리. 배정·명령·DT 세 곳, 방아쇠 게이트에서 ROE 옆 | `SoldierOrderTypes.h:132-141, 257-259` · `SoldierEngagement.cpp:959-964` · `ScenarioStepTypes.h:162-166` | [A] 코드 · 판정 [C-145] |
| 6 | **트리거 2** `EnemyFireStarted` · `EnemyNearFriendlySoldiers` | `ScenarioStepTypes.h:121-129` · `ScenarioStateSubsystem.cpp:288-319, 1209-1214, 1342-1349` · `.h:435-436` | [A] |
| 7 | **RCWS 청각 브리지** — 레지스트리 `OnGunshot` 델리게이트 → 브리지 → `UDetectableTargetSubsystem::ReportGunfire(Enemy)` | `AI/SoldierIdentity.{h:397-399, cpp:530-552}` · `SoldierLabBridgeSubsystem.cpp:46-77` | [A] 코드 · 판정 [C-147] |
| 8 | 유니티 빌드 충돌 — `SoldierSquadSubsystem.cpp` 익명 네임스페이스 헬퍼에 `Squad` 접두(`SquadVerbLabel`/`SquadROELabel`/`SquadSpeedLabel`/`SquadModeLabel`/`SquadFactionLabel`) | `SoldierSquadSubsystem.cpp:25-60` | [A] (09-17 문서엔 없음 — 이번에 기록) |

Perforce: `ScenarioStepTypes.h` · `ScenarioStateSubsystem.{h,cpp}` 는 **`user2` 도 체크아웃 중** — 제출 시 머지.

---

## 1. 도착 불일치 — 분대는 0.8×r 에서 "도착", 엄폐는 r 에서 "당김 끝" [A]

### 1.1 증상
`[Squad] order … MoveTo r=1500` 뒤 병사들이 존 링 **안쪽 12~15 m 띠**, 일부는 **링 밖 ~4.5 m** 에 정착했고 `[Squad] reached` 가 안 찍혀 `achieved` 가 영영 안 났다(→ `SquadOrderAchieved` 도 안 뜸).

### 1.2 원인 — 같은 "도착"을 두 층이 다른 숫자로 쟀다
- 분대 층(09-17): 발이 `Anchor` 에서 `RadiusCm × ArrivalInsetFraction(0.8)` 안 → reached(`2026-09-17_command_layer_design.md` 3절).
- 엄폐 층 `TaskCost` Approach(09-17): 구역 **가장자리(r)** 까지 거리 / `ApproachScaleCm` — 링에 닿으면 비용 0. 즉 r 과 0.8r 사이 12~15 m 띠는 엄폐 층에겐 "다 왔다"인데 분대 층에겐 "아직".
- 링 **밖** 정착까지 나온 이유: Approach 비용의 기울기가 `ObjectiveWeight 2 / ApproachScaleCm 3000` = **0.00067/cm** 이라, 4.5 m 더 가 봐야 0.3 이득 — `MoveImprovementMargin 0.3`(`SoldierCover.h:432`)을 못 넘어 엄폐가 조금이라도 좋은 자리에 멈춘다.

### 1.3 수정 — 숫자 하나, 계단 하나
- **`FSoldierAssignment::ArrivalFraction`**(기본 0.8, clamp 0.1~1) 신설. 출처는 **`ASoldierZone::ArrivalFraction`**(존 액터 프로퍼티, 반경 옆에서 저작) → `ApplyToOrder` 가 `FSoldierSquadOrder::ArrivalFraction` 으로 → `BuildAssignment` 가 배정으로(`SoldierSquadSubsystem.cpp:261`). `IsInsideZone` 은 **배정의** fraction 을 읽는다(`:433-437`). 서브시스템 프로퍼티 `ArrivalInsetFraction` 은 **삭제**(`.h:86-88` 주석만 남음).
- **`TaskCost` Approach** (`SoldierCover.cpp:165-179`):
  ```
  ArrivalCm = RadiusCm × ArrivalFraction
  ToLine    = dist − ArrivalCm
  ToLine ≤ 0 → 0
  else       → 1 + ToLine / ApproachScaleCm
  ```
  선 안은 0, 선 밖은 **1 + 기울기** — 계단 1 이 "선 밖 어느 자리도 선 안 어느 자리보다 최소 홀드 밴드 하나만큼 비싸다"를 만든다. 기울기만으로는 margin 0.3 에 걸려 선 앞 몇 m 에서 멈추던 것이 이 계단으로 사라진다. Hold 전환(선 안 0)과는 이음새 없음.
- 존 표시(2절)의 **초록 안쪽 원 = 이 선**. 분대 전원이 초록 안에 있으면 achieved.

판정 **[C-144]**: `[Squad] reached` 시각의 발 위치가 초록 원 안, `[Cover] task=approach…` 마지막 스윕이 `stay` 로 끝나는 위치가 같은 원 안. 링 밖 정착 0건.

---

## 2. `ASoldierZone` 이 에디터에서 보인다 [A]

09-17 은 `SoldierLab.Debug.Zone 1` 로 **PIE 에서만** 그렸다 — 레벨 저작 중엔 보이지 않는 `TargetPoint` 와 같아서 반경이 엄폐를 품는지 알 길이 없었다(P85). `SoldierZone.cpp:31-80`:

| 컴포넌트 | 무엇 | 색/크기 |
|---|---|---|
| `RadiusShape` `USphereComponent` | `RadiusCm` | (230,200,60) 굵기 3 |
| `ArrivalShape` | `RadiusCm × ArrivalFraction` — **도착선** | (120,220,120) 굵기 1.5, fraction 1 이면 숨김 |
| `BandShape` | `RadiusCm + BandCm` | 같은 노랑 α 90, 굵기 0(가는 선), band 0 이면 숨김 |
| `SectorArrow` `UArrowComponent` | 길이 = `RadiusCm`(최소 50), **액터 회전 = 섹터 방향** | `bUseSector` 로 표시 |
| `Sprite` `UBillboardComponent` | `/Engine/EditorResources/S_TargetPoint`, z +100, scale 2 | |

전부 `bIsEditorOnly` · `SetHiddenInGame(true)` · `NoCollision` · 내비 영향 없음 · 틱 없음 · `bDrawOnlyIfSelected=false`(선택 안 해도 그림). `OnConstruction` → `RefreshShapes()` 라 Details 에서 값을 바꾸면 즉시 다시 그린다. PIE 오버레이(`SoldierLab.Debug.Zone`)는 그대로 있고 도착선 원이 추가됐다(`:149-150`).

컴파일 함정 [A]: `ConstructorHelpers::FObjectFinderOptional` 은 `Get()`/`Succeeded()` 이지 `Succeeds()` 가 아니다 · `UShapeComponent::LineThickness` 는 protected → `SetLineThickness()` · 이 프로젝트는 **C4458(지역 변수가 멤버를 가림)을 에러**로 다룬다 — 람다 인자 이름 주의.

---

## 3. 차량이 아군 분대원으로 잡히던 것 — `bTakesSquadOrders` [A]

브리지(09-17)가 UGV·트럭에 `USoldierIdentityComponent(Friendly)` 를 붙이자(적군이 차량을 보게 하려고) 분대 층의 `GetMembers(Friendly, NAME_None)` 이 그 차량을 **"분대 미지정 아군"** 으로 셌다. 증상 둘:
- `IssueSquadOrder` 가 진영 전체를 대상으로 하면 `Targets` 에 `NAME_None` 이 들어가 `[ScenarioStateSubsystem] IssueSquadOrder Occupy: Friendly/(none) 의 0번 전투지가 AScenarioConfig::SquadZones에 없음 — 이 분대는 건너뜀` 경고(`ScenarioStateSubsystem.cpp:1951-1954`).
- `SquadOrderAchieved(Friendly)` 가 **영영 참이 안 됨** — 차량은 존에 안 가니까.

수정: `USoldierIdentityComponent::bTakesSquadOrders`(`SoldierIdentity.h:70`, 기본 **true**, EditAnywhere). 브리지 `BridgeVehicles` 가 false 로 만들고 등록(`SoldierLabBridgeSubsystem.cpp:176-178`), `GetMembers` 가 `!bTakesSquadOrders` 를 건너뛴다(`SoldierSquadSubsystem.cpp:121-122`). 레지스트리(시야·표적)에는 그대로 있다 — 적군이 차량을 보는 것은 안 바뀐다.

---

## 4. 정원제·대타 — `ReinforceSquads` + `Quota` [A]

### 4.1 요구
구 시나리오(`../ai_combat/2026-09-03_dynamic_squad_reassignment_and_casualty_log.md`)의 "2차 전투지엔 무조건 10명, 죽은 2·3분대 몫은 1분대가 대타". 그 구현은 `BeginEnemyFleeToZone` 의 명단 스냅샷 + `UEnemyCombatComponent` 전용이라 SoldierLab 병사에는 안 먹는다(설계 결정 1 — 구 이펙트는 구 BP 병사용으로만 남김).

### 4.2 구현 — `USoldierSquadSubsystem::ReinforceSquads(Faction, SquadIds, Quota, Toward)` (`SoldierSquadSubsystem.cpp:141-220`)
1. 나열된 분대들의 **생존자 합** `Living`(`GetMembers`, 차량 제외) → `Needed = Quota − Living`, 0 이하면 아무것도 안 함.
2. 풀 = 같은 진영에서 나열된 분대 **밖**의 전원, `Toward` 까지 XY 거리 오름차순(`:169-179`) — "이미 앞서 있는 놈이 전투를 되돌아 지나지 않아도 되는 놈"(구 시스템과 같은 우선순위).
3. `Needed` 명을 차례로 **가장 얇은 분대**(동률이면 먼저 나열된 쪽)에 편입: `Soldier->SquadId = Target; bSquadLeader = false`(`:189-203`). **영구** — 다음 명령("3분대만 3차로")도 편입된 인원을 그 분대원으로 본다.
4. 로그(항상): `[Squad] reinforce Hostile quota=10 living=4 needed=6 moved=6 : BP_Soldier_Hostile_3(1->2), …`. 모자라면 같은 줄이 **Warning** + `(faction exhausted)`.
5. 명령은 여기서 안 내린다 — 호출자가 바로 뒤에 내린다.

### 4.3 시나리오 연결
`FScenarioSquadOrderSpec::Quota`(`ScenarioStepTypes.h:188-195`, 기본 0 = 끔). `UScenarioStateSubsystem::IssueSquadOrderSpec` 이 **구역 동사(MoveTo/Occupy/Withdraw/SuppressArea) + Quota>0 + SquadIds 비어 있지 않음** 일 때 명령 발행 **직전**에 부른다(`ScenarioStateSubsystem.cpp:1923-1940`). `Toward` = **첫 분대**의 `ZoneIndex` 전투지(`FindSquadZone(Faction, SquadIds[0], ZoneIndex)`) — 없으면 `정원 N명: 첫 분대 X의 Z번 전투지가 없어 편입 생략` 경고 후 명령만. 진영 전체(`SquadIds` 빈) 행에서는 무시된다.

구 시스템과 다른 점: 스냅샷·`LastStandZoneIndex`·역할 카운트가 없다. "정원"은 명령 순간 살아 있는 사람 수 하나로 정의되고, 편입 = `SquadId` 변경이라 `SoldierLab.Debug.Squad` 라벨·`[Squad] achieved …/2` 집계에 바로 반영된다.

---

## 5. `EngageRangeCm` — ROE 의 사거리 [A]

### 5.1 증상 (첫 PIE)
`EnemyApproach`(MoveTo z0 Cautious Free) 발행 직후 **적 5/6/7/10 번이 스폰 지점에서 92 m 밖 UGV 에 단발**을 시작 — 반동으로 산포가 커지면 `worth` 게이트가 닫히고, 회복되면 다시 한 발, 의 진동. 그 총성이 t≈7 s 에 `EnemyFireStarted` 를 당겨 **접근 단계가 통째로 사라졌다.**

왜 92 m 가 쏠 만한 거리였나: 시야 120 m 안(`SightRangeCm 12000`) + 소총 산포 `WeaponSpreadDegrees 3°`(`SoldierEngagement.h:302`)의 제압 가치 한계 = `dist × tan 3° ≤ SuppressiveRadiusCm 500`(`:315`) → **≈ 95 m**. 92 m 는 그 안쪽 3 m 다. 판단은 맞다 — "소총으로 90 m 밖 차량에 열지 마라"는 개인이 아니라 **분대의 말**이어야 한다.

### 5.2 구현
- `FSoldierAssignment::EngageRangeCm`(`SoldierOrderTypes.h:132-141`, 0 = 제한 없음) · `FSoldierSquadOrder::EngageRangeCm`(`:257-259`) · `FScenarioSquadOrderSpec::EngageRangeCm`(`ScenarioStepTypes.h:162-166`).
- 흐름: 구역 동사와 **`SetROE` 둘 다** 나른다(`BuildAssignment` `:264, :277`) — ROE 와 한 몸.
- 게이트(`SoldierEngagement.cpp:959-964`): ROE 판정(`bROEAllows`) **바로 뒤**에 `Task.EngageRangeCm > 0 && DistanceCm > EngageRangeCm → bROEAllows = false`. 같은 자리라 로그의 `Restrained` 가 한 뜻("분대가 안 된다고 했다")을 유지한다. 조준·관찰·엄폐는 그대로.
- `[Engage]` 로그 `roe=free/4000`(`:1420-1425`) — 슬래시 뒤가 사거리(0 = 무제한).

DT: `EnemyApproach` 행 4000, `AllyDefend` 행 5000(`../level_new_kadex_0811/2026-09-18_…test_level.md` 3절). 판정 **[C-145]**: 접근 중 `[Engage] … roe=free/4000 … -> Restrained` 만 찍히고 `EnemyFireStarted` 가 UGV 의 `SetDemoUGVAutoFire` 이후에 발동.

---

## 6. 새 트리거 2 [A]

| 트리거 | 정의 | 코드 |
|---|---|---|
| `EnemyFireStarted` | SoldierLab **Hostile** 총성 수(`USoldierRegistrySubsystem::GetGunshotCount(Hostile)`)가 스텝 시작 시점 기준선 `ScenarioSoldierLabHostileShotBaseline`(`ScenarioStateSubsystem.h:435-436`)보다 커지면. `AllyFireStarted` 의 적군판 | 기준선 `:973-977`(시작 시) · `:1121`(리셋) · 평가 `:1209-1211` · `:1342-1344` |
| `EnemyNearFriendlySoldiers` | 살아있는 Hostile 보병 ↔ Friendly 보병 최단 **XY** 거리 ≤ `TriggerDistanceThreshold`. 보병 = `USoldierHealthComponent` 보유(브리지가 차량에 붙인 Identity 제외). 어느 쪽이 0명이면 −1(불발) | 헬퍼 `ComputeNearestHostileToFriendlySoldierDistance`(`:288-319`, 행마다 재계산 `:1212-1214`) · `:1346-1349` |

구 `UGVFiredNearEnemy` 와 `EnemyFireStarted` 는 같은 이펙트를 가진 짝 행으로 둘 수 있다("먼저 쏘는 쪽이 교전 개시") — 시험 DT 는 `EnemyFireStarted` 만 쓴다.

---

## 7. RCWS 청각 보조 ← SoldierLab 적군 총성 [A]

RCWS 의 총성 방향 조사(`RCWSFireControlComponent bInvestigateGunfire`, `../rcws/2026-09-17_rcws_gunfire_hearing.md`)는 `UDetectableTargetSubsystem::ReportGunfire` 로만 먹고, 그 호출자는 **구 `UEnemyCombatComponent::ReportGunfireToSubsystem`**(버스트당 1회) 뿐이었다 — SoldierLab 적군은 아무리 쏴도 RCWS 가 못 들었다.

- `USoldierRegistrySubsystem::OnGunshot`(`DECLARE_MULTICAST_DELEGATE_ThreeParams(ShotLocation, Shooter, Faction)`, `SoldierIdentity.h:397-399`). `CountGunshot(Shooter, ShotLocation)` 이 카운트 뒤 **서버 + `USoldierHealthComponent` 보유 액터**만 브로드캐스트(`.cpp:545-551`) — 차량이 나르는 Identity 의 총성(RCWS 자신)은 안 돈다.
- 브리지 `BindGunshots`(틱마다 시도, 핸들 잡히면 1회, `SoldierLabBridgeSubsystem.cpp:46-59`) → `OnSoldierGunshot`: **Hostile 만** `Detection->ReportGunfire(ShotLocation, Shooter, EMilitaryFaction::Enemy)`(`:61-77`). 아군 총성을 보고하면 포탑이 자기 편 쪽으로 돈다(구 규칙과 동일).
- **발마다** 들어온다(구 경로는 버스트당). `ReportGunfire` 가 보존 10 s / 최대 128 개로 알아서 자른다(`DetectableTargetSubsystem.cpp:15-40`) — 버퍼가 SoldierLab 총성으로 채워져 구 BP 적군 총성이 밀릴 수 있으나 두 병사 체계를 섞어 쓰지 않기로 했으니(결정 1) 무시.

판정 **[C-147]**: 시험 레벨에서 UGV 시야 밖(엄폐 뒤) SoldierLab 적군 사격 → RCWS 로그 `근처 적 총성(NNm) → 총성 방향 4.0초 조사`.

`guide/detection_dev_guide.md` §2 · `guide/rcws_fire_control_dev_guide.md` 청각 절에 09-18 추기.

---

## 8. 참고 — 병사의 사격 결정은 어디서 막히나 (`SoldierEngagement.cpp`, 09-18 판) [A]

"왜 안 쏘나 / 왜 쏘나" 를 로그 한 줄로 읽기 위한 게이트 목록. `[Engage]` 줄의 `believed worth aperture onTarget reloading ammo` 가 순서대로 이것이다.

| 게이트 | 조건 | 값 |
|---|---|---|
| believed | 기록 확신 ≥ `MinCertaintyToEngage` | 0.25 (`.h:482`). 시야는 120 m 두 점(가슴→머리) |
| ROE | `Free` / `ReturnFireOnly`(접촉이 `ReturnFireWindowSeconds` 안에 쐈나, `.cpp:952-955`) / `HoldFire` | 3 s (`.h:574`) |
| **EngageRangeCm** | 접촉 거리 ≤ 배정 사거리(0 = 무시) | 5절 |
| worth | `dist × tan(WeaponSpreadDegrees)` ≤ `SuppressiveRadiusCm`(사격 중엔 × `WorthHysteresis`) | 3° · 500 cm → ≈ 95 m (`.cpp:1195-1204`) |
| 노출 회계 | `ExposureAccount` vs `PeekValue` — 엄폐 뒤면 나올 차례인가 | `ai/2026-09-15_exposure_cycle_and_muzzle_learning.md` |
| aperture | 실제 총구에서 사선이 트인 출구(Over/Left/Right, Open/Lean/Blind) | |
| onTarget | 조준 오차 ≤ `WeaponSpreadDegrees × OnTargetConeRatio` | `.cpp:1210-1214` |
| 아군 사선 · 탄약/재장전 | | |

**접근 중 자세** — 앉아서 가지 **않는다.** `DesiredStance = max(SuppressionNow × StanceUnderFullSuppression, CoverStance)`(`.cpp:792-793`), `CoverStance` 는 아는 위협이 없으면 0, `Cautious` 는 스프린트만 막는다(`.cpp:1389-1393`). ★ 09-18 다른 세션이 넣은 `CombatStanceFloor 0.5`(`.h:393`, `.cpp:1013-1021`)는 **엄폐 없는 자리에서 · 믿는 접촉이 있고 · 정지 중일 때**만 바닥값을 올린다 — 접촉 없는 접근 이동에는 여전히 서서 간다.

### 8.1 제안(미구현) — `FSoldierAssignment::MinStance`
`DesiredStance` 계산 직후 `DesiredStance = max(DesiredStance, Task.MinStance)` 한 줄 + `FSoldierSquadOrder`/`FScenarioSquadOrderSpec` 필드 + DT `EnemyApproach` 행 0.6~1.0. "경계하며 접근"의 그림(웅크려 걷기)이 명령으로 나온다. Engagement/Cover 를 다른 세션이 편집 중이라 이번엔 안 넣었다 → **[W84]**. 걷기 gait([W69])와 짝.

---

## 9. 미해결 → `OPEN_ITEMS.md`

| ID | 항목 |
|---|---|
| **[C-144]** | 도착선 — `[Squad] reached` / 마지막 `[Cover] stay` 위치가 초록 원 안, 링 밖 정착 0 |
| **[C-145]** | `EngageRangeCm 4000` 이 접근 단계를 보존하는가 — `EnemyFireStarted` 가 UGV 자동사격 이후 |
| **[C-147]** | RCWS 청각이 SoldierLab 적군 총성에 반응하는가 |
| **[Q50]** | `Zone_Hostile_0_Engage` 위치(아군에서 54 m → ≥130 m) — 시험 레벨 문서 4.1절 |
| **[W84]** | `MinStance`(8.1절) |
| [C-122]~[C-126] · [W70] · [Q49] | 그대로. [C-125] 2PC 는 여전히 미검증 |
