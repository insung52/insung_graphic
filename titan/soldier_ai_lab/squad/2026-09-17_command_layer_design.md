# L0/L1 분대 명령 층 — 설계·구현 (시나리오 ↔ SoldierLab 연결)

2026-09-17 / 진행중(코드 완료 · 빌드·PIE 검증 대기) / 시나리오가 SoldierLab 분대에 **제약**(구역·ROE·속도·공세성)만 주는 명령 층(`Squad/`)과, 두 병사 체계를 잇는 브리지(`titan_example/Soldiers/SoldierLabBridgeSubsystem`)를 구현. 적·아군 **전원 SoldierLab로 교체**하기로 결정.

관련: `drafts/PLAN.md`(초안 — 이 문서가 그 3.2절 "제약을 주지 명령을 주지 않는다"만 승계하고 토큰·사기·조·StateTree는 채택하지 않았다) · `drafts/OPEN_QUESTIONS.md`(Q24/Q25/Q28 답은 1절) · `ai/2026-09-16_squad_terms_and_learned_death.md`(S1~S4 선호 — 그대로 유효, 이 층은 그 위에 얹힌다) · 시나리오 쪽 기록 `../level_new_kadex_0811/2026-09-17_soldierlab_squad_scenario_link.md`.
원칙: P4(진영은 데이터) · P5(HasAuthority) · P6(튜닝은 데이터) · P89(규칙이 아니라 선호) · P130(세계가 대신 알려주지 않는다).

> 신뢰도: **[A]** 코드로 확인 · **[B]** 잠정 · **[C]** 미측정. **빌드는 아직 한 번도 안 했다** — 아래 [A]는 "코드가 그렇게 쓰여 있다"이지 "돌아간다"가 아니다.
>
> ★ **2026-09-18 정정** — 빌드·첫 PIE 뒤 바뀐 것(9절 참고, 전문 `2026-09-18_squad_layer_fixes_quota_engage_range.md`): 3절 `ArrivalInsetFraction` 은 **삭제**(도착선은 `ASoldierZone::ArrivalFraction`) · 3.1절 존은 **에디터에서 보인다** · 2.3/2.4절에 `EngageRangeCm` 추가 · 6절 DT 필드에 `Quota`, 트리거에 `EnemyFireStarted`/`EnemyNearFriendlySoldiers` 추가 · 5절 브리지가 차량 Identity 에 `bTakesSquadOrders=false` · RCWS 청각에 SoldierLab 총성 연결 · 6.1절 저작안은 시험 DT 13행으로 대체(`../level_new_kadex_0811/2026-09-18_soldierlab_three_stage_test_level.md` 3절).

---

## 0. 한 장 요약

| 무엇 | 어디 | 상태 |
|---|---|---|
| 명령 계약 `FSoldierAssignment` / `FSoldierSquadOrder` / `ESoldierROE` / `ESoldierMoveSpeed` / `ESoldierTaskMode` | `Source/SoldierLab/Squad/SoldierOrderTypes.h` | [A] 신규 |
| L1 `USoldierSquadSubsystem`(UTickableWorldSubsystem, 서버 전용) | `Squad/SoldierSquadSubsystem.{h,cpp}` | [A] 신규 |
| 저작 액터 `ASoldierZone` | `Squad/SoldierZone.{h,cpp}` | [A] 신규 |
| `USoldierIdentityComponent`에 `SquadId`·`bSquadLeader`·`Assignment`(Set/Get) · 레지스트리 `GetGunshotCount` | `AI/SoldierIdentity.{h,cpp}` | [A] |
| 개인 AI 훅 — Cover(과업 비용·Aggression·NavFilter) · Engagement(ROE·섹터·속도) · Perception(화망 구역) | `AI/SoldierCover` · `AI/SoldierEngagement` · `AI/SoldierPerception` | [A] |
| P5 게이트 — Cover/Engagement/Sight/Comms 틱에 `HasAuthority` | 위 4파일 | [A] · 2PC 미검증 [C-125] |
| 브리지 `USoldierLabBridgeSubsystem` | `Source/titan_example/Soldiers/SoldierLabBridgeSubsystem.{h,cpp}` | [A] 신규 |
| titan 변경 — 탐지 플래그 · RCWS 총성 · RCWS 탄 제압 | `Detection/DetectableTargetComponent.h` · `Vehicles/RCWSFireControlComponent.{h,cpp}` · `Vehicles/RCWSProjectile.{h,cpp}` | [A] |
| 시나리오 연결 — `IssueSquadOrder` 이펙트 · `FScenarioSquadOrderSpec` 행 필드 · `SquadOrderAchieved` 트리거 · `AScenarioConfig::SquadZones` · 콘솔 `titan.SquadOrder` | `UI/ScenarioStepTypes.h` · `UI/ScenarioStateSubsystem.{h,cpp}` · `UI/ScenarioConfig.{h,cpp}` | [A] |
| 모듈 의존 `titan_example → SoldierLab` (한 방향) | `titan_example.Build.cs` | [A] |
| 시험 레벨 · DT · 로그 판정 | — | **[C]** 전부 대기 |

Perforce: 위 파일 전부 `user4_DESKTOP-81S78B2_4340` 체크아웃(신규 7파일 `p4 add`), **미제출**. ⚠ titan 쪽 8파일은 `user2@user2_jiseong`도 열어 두고 있다 — 서브밋 시 머지.

---

## 1. 결정 사항 (사용자, 2026-09-17)

| # | 결정 | 근거·귀결 |
|---|---|---|
| 1 | **옛 titan 병사(`UEnemyCombatComponent` / `UAllyFormationComponent`)는 안 쓴다. 적·아군 전원 SoldierLab로 교체** | 개인 전투 AI가 이미 SoldierLab에 있으니 옛 소스와 엮지 않는다. 구 이펙트(`BeginEnemyEngage`, `BeginEnemyFleeZoneN`, `BroadcastAmbush` …)는 구 BP 병사 전용으로 남고 SoldierLab 병사에는 아무 일도 안 한다 |
| 2 | 아군 = **거점 방어**: 사주경계, 적이 와도 적극 살상 안 함, 안전하게 엄폐하다 가끔 사격 | `Occupy` + 낮은 `Aggression` + (권고) `ReturnFireOnly` → [Q49] |
| 3 | **표적 선호 없음** — 지금처럼 가장 위협적인(확신 최대 − 분담 감점) 적 | `RetargetEnemiesToAllies/CommandPost` 대응물을 만들지 않았다. UGV→아군→트럭 전환은 총성 기록의 확신이 만든다 (추정) → [C-122] |
| 4 | **3분대 3차 도주는 은밀히** — 사격 없이 뛴다 | `Withdraw` + `HoldFire` + `Rush`. `ReturnFireOnly`는 2차 도주(1차→2차)에만 |
| 5 | 의존 방향 `titan_example → SoldierLab` 한 쪽 | SoldierLab은 titan을 모른다(`SoldierLab.Build.cs`에 titan을 넣지 말 것). 브리지·시나리오 연결은 전부 titan 쪽 |
| 6 | 전투지 저작 단위 = **분대당 `ASoldierZone` N개**. 기존 15×6 마커는 무시(백업) | `AScenarioConfig::SquadZones[{Faction, SquadId, Zones[]}]` |
| 7 | 분대 편성 = **배치 인스턴스의 `USoldierIdentityComponent::SquadId`**(Q28 답: 스포너 아님, DataTable+태그 아님) | 등록부를 (Faction, SquadId)로 묶는다 — 분대 액터가 없다 |
| 8 | 적 3×5 = 분대 1/2/3 (Q24/Q25 답), 아군 25(현 레벨 실측)도 분대 저작 가능 | 규모에 딸린 산술(토큰·조)이 없으니 몇 명이든 무방 |
| 9 | titan 파일 수정 자유(기존 로직 안 해치면), 리플리케이션·RTSP는 신중 | RTSP는 안 건드렸다. 리플리케이션은 P5 게이트만 추가, 실기 검증 대기 |
| 10 | **새 테스트 레벨**에서 먼저, 그 뒤 New_kadex_0811 | 사용자가 레벨 생성(MCP로 레벨 안 만든다) |
| 11 | 리스크 1(적군이 숲에 숨어 시나리오가 안 진행) **경계** | `Aggression` 다이얼이 그 자리. 판정은 [C-123] |

**Q: 기존 UGV_0901을 적군이 위협으로 인지하나?** — 된다 [A·코드]. 브리지가 `DetectableTargetComponent(Friendly)` 차량에 `USoldierIdentityComponent(Friendly)`를 런타임 부착하고, RCWS `Fire()`가 `BroadcastGunshot`을 부른다. 적군 시야 트레이스는 **대상 액터를 무시**(`SoldierSight::HasLineOfSight`, `Params.AddIgnoredActor(TargetActor)`)하므로 소켓 없는 차량의 폴백 표적점이 메시 안이어도 판정이 선다. 값(눈 +160/표적 +110, 액터 원점 기준)은 [C-126].

---

## 2. 명령 계약 — `Squad/SoldierOrderTypes.h` [A]

### 2.1 `ESoldierROE`

| 값 | 뜻 | 정의 근거 |
|---|---|---|
| `Free` | 믿을 만하고 값어치 있는 접촉이면 쏜다 | 기존 방아쇠 그대로 |
| `ReturnFireOnly` | **최근 `ReturnFireWindowSeconds`(3 s, `USoldierEngagementComponent` 프로퍼티) 안에 사격한 접촉에게만** Aimed/Suppressive. 나머지는 조준·관찰만 | 설계 문서(`design/2026-09-01_architecture.md` 11.2)와 초안 `SoldierOrder.h`의 `EEngagementRule::ReturnFireOnly`는 **이름뿐 정의가 없었다** — 이 세션이 정의. 기록의 `LastGunshotTimeSeconds`를 읽으므로 새 판단 재료 없음 |
| `HoldFire` | 절대 안 쏜다. 조준·관찰 피크·엄폐는 그대로 | 방아쇠만 막는다 |

초안의 `PositiveIDRequired`는 채택하지 않았다.

### 2.2 `ESoldierMoveSpeed`

| 값 | 스프린트 | 총 |
|---|---|---|
| `Cautious` | 절대 안 함 | 접촉 없으면 내림(기존 BP 배선 `WantsToAim ← HasContact` 그대로) |
| `Normal` | 기존 선호(이동 중 ∧ 안 쏨) | 〃 |
| `Rush` | 이동 중이면 항상 | 〃 |

⚠ `Cautious`의 "걷기(150 cm/s)"는 **미구현** — 지금은 스프린트 금지뿐이라 조깅으로 간다 → [W69].

### 2.3 `FSoldierAssignment` — 분대 → 개인 (전부 제약, PLAN.md 3.2절 규칙)

| 필드 | 뜻 | 제약인가 명령인가 | 읽는 곳 |
|---|---|---|---|
| `Mode` None/Approach/Hold | 구역 비용의 모양 | **제약** — 땅의 값 | `SoldierCover::TaskCost` |
| `Anchor`/`RadiusCm`/`BandCm`/`ApproachScaleCm` | 구역 | **제약** — "이 좌표로"가 아니라 "이 구역이 값이 있다" | 〃 |
| `Speed` | 2.2절 | **제약** | `SoldierEngagement` 스프린트 |
| `ROE` | 2.1절 | **제약** — 방아쇠 허가 | `SoldierEngagement` 방아쇠 |
| `Aggression` 0..1 | 0.5 = 개인 가중치 그대로. `ObjectiveWeight`와 `NoFiringPositionCost`에 `2×Aggression` 배율 | **제약** — 선호의 세기 | `SoldierCover::AggressionScale` |
| `bHasSector`/`SectorYawDeg`/`SectorHalfWidthDeg` | 접촉 없을 때 보는 방향 | **제약** — 눈이 갈 곳 | `SoldierEngagement` 섹터 조준 |
| `bHasSuppressArea`/`SuppressAreaCenter`/`SuppressAreaRadiusCm` | 화망 구역 | **제약** — 기록 하나 주입, 쏠지는 기존 게이트 | `SoldierPerception::ReportOrderedArea` |
| `NavFilterClass` | 이 배정 동안의 경로 필터 | **제약** — 회랑 대신 가중 경로 | `SoldierCover::FinishSweep` → `MoveToLocation` |
| `bTargetableByOwnSideWeapons` | 자기편 차량·타 분대가 이 병사를 쏠 수 있나 | 연출 플래그 | 브리지 → `UDetectableTargetComponent::bTargetableByFriendlyForces` |
| `Revision` / `SquadId` / `Verb` | 변경 감지·로그 | — | 로그 |

**넣지 않은 것**: 초안의 `Corridor(FBox)`(이 레벨은 스플라인 필터가 그 역할), `FocusTarget`/표적 선호(결정 3), 토큰·조·사기.

### 2.4 `FSoldierSquadOrder` — L0 → 분대

`Verb`(`MoveTo`/`Occupy`/`Withdraw`/`SetROE`/`SuppressArea`/`SetTargetable`/`Clear`) + 구역(Anchor/Radius/Band/ApproachScale) + Speed/ROE/Aggression + 섹터 + 화망 + `NavFilterClass` + `bTargetable` + `StaggerMin/MaxSeconds`(개체별 출발 지연) + `ZoneTag`(발행자가 붙이는 정수 — 시나리오의 0/1/2, 분대 층은 해석 안 함) + `Source`.

| Verb | 만드는 Assignment |
|---|---|
| `MoveTo` | Mode **Approach** → 도착 시 Hold. 전부 새로(옛 ROE 안 샘) |
| `Occupy` | Mode **Hold** 처음부터 |
| `Withdraw` | `MoveTo`와 같되 의미상 이탈 — 지연 출발·ROE는 행이 정한다(코드상 MoveTo와 동일 분기) |
| `SetROE` / `SuppressArea` / `SetTargetable` | 서 있는 배정의 그 필드만 바꾼다. **지연 출발 대기 중인 배정에도 같이** 적용(HoldFire를 도주 중에 말했는데 출발 순간 풀리는 일 방지) |
| `Clear` | 배정 삭제 → 레벨 `ASoldierObjective`로 폴백 |

---

## 3. L1 — `USoldierSquadSubsystem` [A]

- **분대 객체 없음.** `USoldierRegistrySubsystem`을 (Faction, `SquadId`)로 묶어 매번 읽는다. 스폰·사망·분대 변경에 따로 할 일이 없다. `SquadId` None = "분대 미지정 병사들"이라는 한 분대; `IssueOrder(..., bAllOfFaction=true)`가 진영 전체.
- `IssueOrder(Faction, SquadId, Order, bAllOfFaction)` → 살아있는 멤버마다 `BuildAssignment(현재, Order)`. 구역 동사는 분대 기록(`FSquadState`: Order·Serial·IssuedTime·StartedWith·bAchieved·ZoneTag)을 새로 쓰고, 규칙 동사는 진척을 안 건드린다.
- **지연 출발**: 멤버마다 `FRandRange(StaggerMin, StaggerMax)` 뒤 `ReleaseMember` → `Identity->SetAssignment`. 0이면 즉시.
- **Approach → Hold 자동 전환**: 발이 `Anchor`에서 ~~`RadiusCm × ArrivalInsetFraction(0.8)`~~ **`RadiusCm × Assignment.ArrivalFraction`**(→ 정정 9절, 2026-09-18: 서브시스템 상수가 아니라 존 액터의 값이 배정까지 흘러오고, 엄폐 층 `TaskCost` 도 같은 선을 읽는다) 안에 들어오면 같은 배정의 Mode만 Hold로(ROE·속도 유지 — 집결점에 닿았다고 사격이 열리지 않는다).
- **achieved**: 살아있는 전원이 현 명령 Serial로 도달했을 때. 전멸 분대는 achieved가 되지 않는다.
- `GetSquadStatus`(Alive/StartedWith/InZone/Pending/bAchieved/Centroid/ZoneTag) · `IsOrderAchieved` · `GetSquadZoneTag` · `GetSquadIds` · `HasSquadlessMembers`.
- 서버 전용(`GetNetMode() != NM_Client`). 복제 없음 — 병사가 움직이는 결과만 복제된다.
- 로그 `LogSoldierAI` `[Squad] order #N … / release … / reached … / achieved …`(`SoldierLab.Debug.Squad.Log 1`), 오버레이 `SoldierLab.Debug.Squad 1`(구역 링·밴드·병사 라벨 `approach@z3 Withdraw r1500 roe=hold spd=rush agg=0.50 rev7`).
- 라벨 헬퍼 `SoldierSquadAssignmentLabel()`(`SoldierOrderTypes.h` 선언)을 Cover/Engagement 로그가 공유.

### 3.1 `ASoldierZone` — 저작 액터

루트 `Anchor` + `RadiusCm 1500` · `BandCm 1200` · `ApproachScaleCm 3000` · `bUseSector`(액터 회전의 yaw가 섹터 중심) · `SectorHalfWidthDeg 45` · `NavFilterClass` · `ZoneLabel`. `ApplyToOrder(FSoldierSquadOrder&)`가 구역 필드를 복사. `SoldierLab.Debug.Zone 1`로 전부 그린다. ⚠ 반경은 **싸울 만한 엄폐를 품어야** 한다(P85) — 안 그러면 땅과 엄폐가 서로 당겨 진동.

---

## 4. 개인 AI 훅 — 판단식은 그대로, 읽는 값만 늘었다 [A]

### 4.1 `SoldierCover`
- `TaskCost(Foot)`: 배정이 활성이면 **`ASoldierObjective`를 대체**(둘을 더하면 두 땅이 동시에 당긴다 — P85). Hold = 반경 안 0·밴드 상승(수비식), Approach = ~~구역 **가장자리**까지 거리/`ApproachScaleCm`~~ **도착선(`RadiusCm × ArrivalFraction`) 안 0, 밖 `1 + 거리/ApproachScaleCm`**(→ 정정 9절: 가장자리 기준이라 병사가 링 안쪽 12~15 m 띠·링 밖 4.5 m 에 정착해 명령이 안 끝났다), 천장 없음(P84).
- `AggressionScale()` = `2 × Aggression`(배정 없으면 1). `ScorePosition`의 Objective 항과 `FightingCost`의 `NoFiringPositionCost` 절반에 곱한다. 대담한 분대는 못 쏘는 자리를 더 비싸게 산다.
- `ResolveObjective()`를 **매 스윕**(`BeginSweep`)에 — BeginPlay 고정([W25])이 풀렸다. 명령이 지워지면 지금 서 있는 곳의 목표로 돌아간다.
- `FinishSweep`의 `MoveToLocation`에 `Assignment.NavFilterClass` 전달.
- `[Cover]` 로그에 `task=<라벨>` 추가.

### 4.2 `SoldierEngagement`
- `bROEAllows` — 접촉에 대해 묻는다(2.1절). `bWorthShot`(내밀 이유)과 방아쇠 둘 다 게이트. 막히면 새 의도 **`Restrained`**(쏠 만하고 조준도 됐는데 규칙이 막음 — `hold`와 구분해 로그에 남긴다). `WantsToFire()`는 긍정 열거라(P68) 값 추가가 안전했다.
- **섹터 조준**: 접촉 없고 `bHasSector`면 `AimRotation`을 섹터 yaw로 같은 등속 램프(240°/s)로 돌리고 `SetFocalPoint` → 시야 콘이 따라간다(P86). ⚠ 비조준 idle에서 캡슐은 안 돈다(P119) — 눈은 가고 몸은 안 도는 상태 (추정) → [W69].
- 속도: `Cautious` → `bWantsToSprint=false`, `Rush` → 이동 중이면 항상.
- `WantsToAim()` 접근자 신설(= `HasContact`, BP는 여전히 `HasContact` 직결) → [W68].
- `[Engage]` 로그에 `roe=` 추가.

### 4.3 `SoldierPerception`
- `ReportOrderedArea(Center, RadiusCm)`: `Shared` 기록(확신 `OrderedAreaCertainty 0.6`, 반감기 3×refresh, 총성 시각 −1, **`bOrderedArea=true`**)을 **융합하지 않고 교체**한다. `FindMatchingRecord`가 ordered 기록은 흡수도 흡수당하지도 않게 — 반복 재도장으로 반경이 30 cm까지 줄어 "구역 중심 정조준"이 되는 것을 막는다. 구역 안에서 난 총성은 자기 기록을 따로 만든다.
- 틱에서 배정의 화망 구역을 `OrderedAreaRefreshSeconds 1`마다 재도장, 배정이 빠지면 삭제. 쏠지는 기존 제압 게이트(앎 ≤ `SuppressiveKnowledgeRadiusCm 1000`)가 정한다 — 반경 1000 넘는 구역은 안 쏜다.
- `ReturnFireOnly`에서는 화망 구역을 안 쏜다(총성 시각 −1).

### 4.4 P5 게이트
`SoldierCover`/`SoldierEngagement`/`SoldierSight`/`SoldierComms` 틱 진입에 `Owner->HasAuthority()`. `SoldierPerception`은 화망 재도장 블록만. `SoldierSuppression` 회복은 클라에서도 돈다(무해, [W62] 그대로).

---

## 5. 브리지 — `USoldierLabBridgeSubsystem` (titan_example) [A]

두 체계는 서로를 모른다: titan(RCWS 탐지·자동사격·드론·사망 카운트·RevealEnemies)은 `UDetectableTargetComponent` 레지스트리만, SoldierLab(시야·청각·표적)은 `USoldierIdentityComponent` 레지스트리만 본다. 브리지가 `ScanIntervalSeconds 0.25`마다(서버):

1. SoldierLab 병사(Identity + `USoldierHealthComponent`)에 `UDetectableTargetComponent` 부착 — Hostile→`Enemy`, Friendly→`Friendly`. 적군은 `AScenarioConfig::bSoldierLabHostilesStartHidden`(기본 true)이면 `bIsRevealed=false`로 시작(UAV 발견 전 안 보임 — 구 BP 저작과 동일). 설정 액터 없는 레벨에선 보인다.
2. **사망 폴링** `Health->IsDead()` → `SetIncapacitated(true)` 한 번 — 탐지 레지스트리에서 빠져 RCWS가 시체를 안 쏘고 `EnemyCasualtyCountAtLeast`/`AllEnemiesEliminated`가 센다. 이건 titan 장부에 사실을 옮기는 것이지 병사의 지식이 아니다(P130 무관).
3. 배정의 `bTargetableByOwnSideWeapons` → `Detectable->SetTargetableByFriendlyForces`.
4. `Friendly` 탐지 대상인데 Identity 없는 액터(UGV·이동형지휘소)에 `USoldierIdentityComponent(Friendly, bMeasureChestHeights=false, bLearnMuzzleOffsets=false)` 부착. 드론(`bLowPriorityForEnemyTargeting`)은 제외 — 선호 없는 SoldierLab이 하늘을 쏘게 된다.

BP·레벨 저작 변경 0. 로그 `[SoldierLabBridge] … 부착.`

### 5.1 titan 쪽 변경
| 파일 | 내용 |
|---|---|
| `Detection/DetectableTargetComponent.h` | `bTargetableByFriendlyForces` + Set/Is — 구 `UEnemyCombatComponent::bTargetableByAlliesAndUGV`를 탐지 층으로 올린 것. RCWS(`bRespectEnemyTargetingExclusion`)가 둘 다 읽는다(구 레벨 동작 불변) |
| `Vehicles/RCWSFireControlComponent.{h,cpp}` | `Fire()`에 `USoldierPerceptionLibrary::BroadcastGunshot(this, Muzzle, SoldierLabGunshotAudibleRangeCm 20000, Owner)`. 아군 보병은 `IsProbablyFriendlyFire`로 자기편 차량 총성을 기록 안 만든다 |
| `Vehicles/RCWSProjectile.{h,cpp}` | 서버 사본 틱마다 `USoldierSuppressionComponent::ApplyAlongSegment`(신설 static, `SoldierProjectile`의 계산을 옮긴 것) — 휘바람 선분과 **별도 추적**(그쪽은 휘바람 후 갱신이 멈춘다). `SuppressedThisFlight`로 한 발 한 번 |
| `AI/SoldierIdentity` 레지스트리 | `CountGunshot`/`GetGunshotCount(Faction)` — `BroadcastGunshot`가 센다. 시나리오 `AllyFireStarted`의 재료 |

---

## 6. 시나리오 연결 [A]

| 무엇 | 어디 |
|---|---|
| 이펙트 `EScenarioEffectType::IssueSquadOrder` | 행의 `SquadOrder`(`FScenarioSquadOrderSpec`: Faction · SquadIds(비면 진영 전체 = 모든 분대 + 미지정) · Verb · ZoneIndex · Speed · ROE · Aggression · StaggerMin/Max · bTargetable · SuppressAreaRadiusCm) |
| 트리거 `EScenarioTriggerType::SquadOrderAchieved` | `SquadOrder.Faction/SquadIds`의 마지막 구역 명령을 전원 달성 |
| `AllyFireStarted` | 기존 `AllyFormationComponent` 합 **+ SoldierLab Friendly 총성 수**(기준선 `ScenarioSoldierLabFriendlyShotBaseline`) |
| `AScenarioConfig::SquadZones[{Faction, SquadId, Zones[]}]` + `FindSquadZone` · `bSoldierLabHostilesStartHidden` | 레벨 액터 참조는 ScenarioConfig에 두는 기존 원칙 |
| `UScenarioStateSubsystem::IssueSquadOrderSpec(Spec, Label)` | 이펙트·콘솔 공용 |
| 콘솔 `titan.SquadOrder <Hostile\|Friendly> <SquadId\|*> <Verb> [ZoneIndex] [ROE] [Speed] [Aggression]` | `FAutoConsoleCommandWithWorldAndArgs` — PlayerController Exec이 아닌 이유: 시험 레벨은 GASP 관전 PC를 쓴다. 서버 월드에서만 |
| 드론 Zone2 프레이밍 | SoldierLab 아군(생존)도 포함 |

`ExcludeFleeingEnemiesFromAllyTargeting`/`HoldFleeingEnemyFire`/`RetargetEnemies*`는 구 BP 전용으로 그대로.

### 6.1 3단계 시나리오 → 명령 매핑 (DT 저작안, [B])

| 단계 · 트리거 | 행 (IssueSquadOrder) |
|---|---|
| S0 시작+1s | Hostile · `*` · **MoveTo** z0 · Cautious · **HoldFire** · agg 0.5 |
| S1 `UGVFiredNearEnemy` | Hostile · `*` · **Occupy** z0 · Rush · **Free** · **agg 0.7~0.8**(사격 위치로 나가 쓰러져야 진행) |
| 2차 도주 `EnemyCasualtyCountAtLeast 3` | Hostile · `2,3` · **Withdraw** z1 · Rush · **ReturnFireOnly** · stagger 0.5~3 s. 1분대는 명령 없음 → z0 Hold 유지 |
| 아군 매복 `+5s` | Friendly · `*` · **Occupy** z0(매복 구역) · Normal · **ReturnFireOnly**(권고, [Q49]) · **agg 0.3** |
| 3차 도주 `EnemyCasualtyCountAtLeast 7` | Hostile · `3` · **Withdraw** z2 · Rush · **HoldFire** · stagger. `+4s` `SetTargetable false`(UGV·아군 제외, 트럭은 무시) |
| 트럭 사격 `CommandPostFiredNearEnemy` | Hostile · `3` · **SetROE Free** |
| 드론 프레이밍 조건 `CurrentZoneIndex>=2` | → `USoldierSquadSubsystem::GetSquadZoneTag` 로 대체할 것(미착수) |

---

## 7. 판정 기준 — 다음 실측 [C]

| ID | 대상 | 통과 | 실패 시 |
|---|---|---|---|
| **[C-122]** | 적군이 **명령받은 zone에서** 쓰러지는가 — `[적 사상]`의 `위치` vs `[Squad] reached`/zone 링 | 1차 사망 3명이 z0 반경 안, 3차 사망이 z2 반경 안 | `Aggression`·`RadiusCm`·stagger |
| **[C-123]** | 숲 엄폐에 숨어 시나리오가 멈추는가 — `[Cover]`의 `hide+fight` 비율, `[Engage] hold`/`blocked` 체류 | S1 진입 후 60 s 안에 사망 3명 | `Aggression`↑, `NoFiringPositionCost`, `PeekStartBelow/StopAbove` |
| **[C-124]** | 킬 페이싱 — RCWS 34×3 vs `SoldierHealth` 체력 + 피크 1.5 s 창 | 구 실측(교전 15 s 만에 3명)보다 느리되 2차 도주 55~60 s 안에 7명 안 죽음 | `AutoFireCycleIntervalSeconds`·체력(사용자 몫) |
| **[C-125]** | 2PC 리플리케이션 — P5 게이트 후 클라에서 병사 이동·사망·사격 이펙트가 보이는가 | 리슨서버 2PC 완주 | `USoldierHealthComponent` 복제, GASP 이동 복제 |
| **[C-126]** | 차량 Identity 폴백 높이(눈 +160/표적 +110, 액터 원점) — 적군이 UGV/트럭을 실제로 보는가, 총성 기록이 목격으로 융합되는가 | `[Engage] tgt=BP_UGV_0901…` 잠금 | `EyeHeightCm`/`TargetHeightCm` 인스턴스 값 |
| [C-83] | 45명 성능(트레이스 증가분 없음, 브리지 스캔 0.25 s) | — | — |
| [C-108] | 분대 항 S1~S4가 명령 층 위에서도 유효한가 | — | — |

로그 3벌: `SoldierLab.Debug.Squad.Log 1` · `SoldierLab.Debug.Cover.Log 1`(`task=`) · `SoldierLab.Debug.Engagement.Log 1`(`roe=`) + titan `[적 사상]`·`시나리오 스텝 발동:`.

---

## 8. 미해결 → `OPEN_ITEMS.md`

| ID | 항목 |
|---|---|
| [C-122]~[C-126] | 7절 |
| **[W68]** | BP `WantsToAim` 배선을 `Engagement.WantsToAim()`으로(지금 `HasContact` 직결, 동작 동일) |
| **[W69]** | `Cautious`의 걷기 gait(지금 스프린트 금지뿐) · 섹터 조준 시 몸(캡슐) 회전(P119 경로 재사용) |
| **[W70]** | New_kadex_0811 재저작 — 적 15·아군 25 SoldierLab 교체, `ASoldierZone` 배치, `SquadZones`, DT 재작성, 드론 프레이밍의 `CurrentZoneIndex>=2` 판정을 `GetSquadZoneTag`로 |
| **[Q49]** | 아군 방어 ROE(`ReturnFireOnly` vs `Free`)·Aggression 값 · 숨겨진(`bIsRevealed=false`) 적을 SoldierLab 아군 시야가 트레이스로는 보는 문제(콜리전 무관 — 지식만 생기고 ROE가 사격을 막는다 (추정)) 처리 방향 |
| [W55] | 진행중 — 이 문서 |
| [W25] | Objective 매 스윕 재해석으로 절반 더 해결(소유권 변경은 여전히 없음) |

---

## 9. 정정 (2026-09-18) — 빌드 · 시험 레벨 첫 PIE 뒤

전문은 `2026-09-18_squad_layer_fixes_quota_engage_range.md`(분대·AI 쪽) · `../level_new_kadex_0811/2026-09-18_soldierlab_three_stage_test_level.md`(레벨 `L_SoldierScenario` · DT 13행 · 검증 순서). 이 문서에서 **더 이상 맞지 않는 곳**:

| 절 | 옛 서술 | 지금 |
|---|---|---|
| 0 · 3 | `ArrivalInsetFraction(0.8)` 이 서브시스템 프로퍼티 | **삭제.** `ASoldierZone::ArrivalFraction`(0.8) → `FSoldierSquadOrder` → `FSoldierAssignment::ArrivalFraction`. 분대 층 `IsInsideZone` 과 엄폐 층 `TaskCost` 가 **같은 선**을 읽는다 |
| 4.1 | Approach 비용 = 가장자리까지 거리/`ApproachScaleCm` | 도착선 안 0, 밖 **`1 + 거리/ApproachScaleCm`**(계단이 `MoveImprovementMargin 0.3` 을 이긴다) |
| 3.1 | 존은 `SoldierLab.Debug.Zone 1` 로 PIE 에서만 보임 | **에디터 전용 구 3개(반경·도착선·밴드) + 섹터 화살표 + 스프라이트**, `OnConstruction` 갱신 |
| 2.3 · 2.4 · 6 | — | **`EngageRangeCm`**(배정·명령·DT, ROE 와 함께 나름, 방아쇠 게이트에서 ROE 옆). 첫 PIE 에서 적군이 스폰 지점에서 92 m 밖 UGV 를 쏴 접근 단계가 사라진 것의 답 |
| 5 | 브리지가 차량에 `Identity(Friendly)` 부착 | + **`bTakesSquadOrders=false`** — 안 그러면 `Friendly/(none)` 분대원으로 잡혀 `SquadOrderAchieved(Friendly)` 가 영영 안 남 |
| 5.1 | RCWS 청각은 구 `EnemyCombatComponent` 총성만 | 레지스트리 `OnGunshot` → 브리지 → `ReportGunfire(Enemy)`. SoldierLab 적군 총성도 RCWS 가 듣는다 |
| 6 | DT 필드 목록 | + **`Quota`**(정원제·대타, `USoldierSquadSubsystem::ReinforceSquads`) · 트리거 + **`EnemyFireStarted`** · **`EnemyNearFriendlySoldiers`** |
| 6.1 | 저작안(S1 `UGVFiredNearEnemy`, 3차 `HoldFire` + 트럭 사격 시 `SetROE Free`) | 시험 DT: S1 = `EnemyFireStarted`, 3차도 **`ReturnFireOnly`**(트럭이 쏘면 응사가 저절로 열리므로 `SetROE` 행 불필요), `Quota 10/5`, 아군 `EnemyNearFriendlySoldiers 8000` 으로 Free 전환 |
| 1 결정 4 | 3분대 3차 도주는 `HoldFire` | 시험 DT 는 `ReturnFireOnly` — 사용자가 은밀 도주를 원하면 `HoldFire` 로 되돌리고 `SetROE Free` 행을 다시 넣는다 |

새 판정: [C-144] 도착선 · [C-145] EngageRange 가 접근 단계 보존 · [C-147] RCWS 청각. 새 결정: [Q50] 1차 전투지 위치(아군에서 54 m → ≥130 m). 새 작업: [W84] `MinStance`(접근 중 앉아 걷기) → **09-18 오후 절반 구현(10절)**.

---

## 10. 계약 추가 (2026-09-18 오후) — 순찰 · 섹터 = 부채꼴 · `MinStance` · 조준점 계약

전문(왜·로그·거동)은 `ai/2026-09-18_patrol_scan_and_move_robustness.md`. 여기는 **이 문서의 2·3절 계약이 어떻게 늘었는가**만 [A · 코드, 오후 빌드·PIE].

| 구조체 / 액터 | 추가 필드 | 기본 | 뜻 | 어디 |
|---|---|---|---|---|
| `FSoldierAssignment` | **`PatrolWeight`** | **0** | Hold 반경 안에서 `+ PatrolWeight × (1 − GetStaleVantage)` — 낡은 땅을 덜 내려다보는 자리가 비싸다. **눈 0일 때만**. 0 = 선 자리를 지킴 | `SoldierOrderTypes.h:184-196` · `SoldierCover::PatrolCost` `.cpp:203-220` |
| | **`PatrolStaleSeconds`** | 30 | 이 나이에 완전히 낡음 | `.h:198-200` |
| | **`MinStance`** | **0** | 자세 바닥값 — `DesiredStance = max(…, MinStance)`, **Rush면 안 적용**. "숨는 곳에서만 웅크림"(필드 자세) 위에 "어디서나 이만큼은"을 얹는 제약 | `.h:129-140` · `SoldierEngagement.cpp:795-800` |
| | `bHasSector` 등 (기존) | | **뜻이 바뀜 — 부채꼴(arc)** 이지 고정 방위가 아니다. 필드가 `SectorYawDeg ± SectorHalfWidthDeg` 안에서 가장 안 훑은 방위를 고르고, 중심은 필드가 여기 대해 할 말이 없을 때만. 고정 방위로 읽자 존(기본 `bUseSector true`)으로 명령받은 병사가 경기 내내 한 줄만 응시했다 | `.h:166-182` · `SoldierCover.cpp:1489-1495` · `SoldierEngagement.cpp:1289-1303` |
| `FSoldierSquadOrder` | `PatrolWeight` **0** · `PatrolStaleSeconds` 30 · `MinStance` **0** | | 존 동사(MoveTo/Occupy/Withdraw)와 함께 배정으로 복사 | `.h:267-269` · `:290-295` · `SoldierSquadSubsystem.cpp:263` · `:270-271` |
| `ASoldierZone` | **`PatrolWeight` 1.0** · **`PatrolStaleSeconds` 30** | | `ApplyToOrder`가 복사 — **존을 거친 명령만 순찰이 켜진다**(배정 기본 0). `MinStance`는 존에 **없다** | `SoldierZone.h:91-100` · `.cpp:123-124` |
| `ASoldierObjective` | **`PatrolWeight` 1.0** · **`PatrolStaleSeconds` 30** | | 명령 없는 레벨(`L_SoldierTest`)의 수비대도 순찰 — 첫 순찰 시험이 거기서 돌았고 `[Squad]` 로그 0줄이었다 | `SoldierObjective.h:98-108` · `SoldierCover.cpp:193-200` |
| `USoldierEngagementComponent` | **`GetAimPoint()`** · **`IsScanning()`** | | 접촉이 있으면 예측 위치, 없으면 스캔 중인 점(항상 AI 초점과 같은 점) / 접촉 없이 눈이 갈 곳이 있는 중(~~볼 곳이 있을 때만 true~~ → **09-18 밤: 필드의 볼 곳이든 명령의 부채꼴 중심이든 무접촉 조준점이 있으면 true**, [W89] 해결) | `SoldierEngagement.h:236-254`(밤 판 `:274-292`) · `.cpp:1315-1322`(밤 판 `:1298-1325`) |
| `USoldierEngagementComponent` (09-18 밤) | **`GetDesiredGait()`** → `ESoldierGait {Walk, Jog, Sprint}` · **`GetTension()`** 0..1 | Walk | 걸음: Sprint = `WantsToSprint`(명령의 Cautious/Rush 가 손댄 뒤) · Jog = 접촉 ∨ 긴장 ≥ `JogTension 0.3` — 단 **`Task.Speed == Cautious ∧ 무접촉 → Walk`** · 그 밖 Walk. 긴장 = 알람(접촉 ∥ 제압 ∥ 사선 거부 ∥ 1 s 안 총성) 뒤 `0.5^(age / TensionHalfLifeSeconds 20)`. 명령이 걸음에 닿는 유일한 자리가 Cautious 의 Walk 캡이다 | `.h:65-71` · `:190-203` · `:455-465` · `.cpp:1391-1416` · `:1441-1454` |
| `USoldierEngagementComponent` (09-18 밤) | **`GetPoseUrgency()`** 0..1 | | `GetDesiredStance/Lean/BlindFireH/V()` 는 **목표**이고 이것이 "얼마나 급히"의 단 하나 숫자 — `Urgency*` 7값의 max + 제압. 명령은 아직 손대지 않는다(Rush 가 급박도를 올리는 자리가 생길 수 있으나 없음) | `.h:205-214` · `:467-497` · `.cpp:1467-1502` |

**소비자 — `Pose/SoldierScanTurnComponent`** (포즈 세션 작성, 여기선 계약만): 총 내리고(`!WantsToAim()`) 발 멈춘(가속 0 ∧ 속도 ≤ 10) AI 병사가 `IsScanning() || HasContact()`이면 `GetAimPoint()` 방위로 캡슐을 돌린다(시작 20° · 끝 5° · 180°/s) — 게이트는 `SoldierScanTurnComponent.cpp:50-80`. 이것이 [W69]의 "섹터 조준 시 몸 회전" 절반에 대한 답이다(~~걷기 gait는 그대로 열림~~ → 밤에 `GetDesiredGait()` 로 닫힘, 아래). ~~동작 확인 [C-152]~~ → 포즈 세션 PIE 확인.

**소비자 — `Pose/SoldierGaitBridgeComponent`** (09-18 밤, 포즈 세션): 매 틱 `CharacterInputState.WantsToWalk = (GetDesiredGait() == Walk)` 리플렉션(`.cpp:92-93`), Sprint 는 `bDriveSprint false`(`.h:37-39`) 로 옛 BP 브리지 소유, Jog 는 GASP 기본 Run 이라 쓰기 없음. `GetTension()` 은 **소비 안 함** — GASP gait 가 이산이고 AI 입력 크기가 항상 1 이라 섞을 자리가 없다. 사용자 PIE "잘됨". **명령이 걸음에 닿는 경로**: `Task.Speed` → (Cautious: `bWantsToSprint=false` + 무접촉 Walk 캡 / Rush: 엄폐 이동이면 Sprint) → `DesiredGait` → `WantsToWalk`/`WantsToSprint`.

**소비자 — `Pose/SoldierPoseSmootherComponent`** (09-18 밤, 포즈 세션): `Scale = lerp(ScaleAtCalm 0.5, ScaleAtUrgent 1.6, GetPoseUrgency())` 로 축 4개를 사다리꼴 프로파일(`.cpp:209-215`). 계약의 뜻: **AI 는 목표 + 급박도만, 움직임은 몸의 것**(P172) — 명령 층이 자세를 만지려면 `MinStance`(목표 바닥)처럼 **목표**를 만지지 속도를 만지지 않는다. `ai/2026-09-18_patrol_scan_and_move_robustness.md` 13~14절 · `animation/2026-09-18_ai_pose_layer_scanturn_gait_smoother.md` 2~3절.

**Hold 비용의 모양도 바뀌었다** (4.1절 보강): 밴드 밖에서 1.0에 멈추던 것이 `1 + (Beyond − Band)/ApproachScaleCm`로 **계속 오른다**(`SoldierCover.cpp:146-169`) — 엄폐가 밴드 밖으로 끌어낸 홀더가 200 s 동안 모든 후보 `o3.20`으로 집에 못 가던 로그(P158). Approach와 같은 P84.

**titan 쪽 미연결**: `FScenarioSquadOrderSpec`·`titan.SquadOrder` 콘솔에 `MinStance`·`PatrolWeight`가 **없다**(`Source/titan_example` grep 0건) — DT는 순찰을 존 기본값으로만, `MinStance`는 아예 못 준다 → [W84] 절반.

이 세션의 판정·값: [C-148](순찰 값) · [C-151](`MinStance`) · ~~[C-152](ScanTurn)~~(포즈 측 확인) · 밤 [C-155](긴장도·걸음) · [C-156](급박도).
