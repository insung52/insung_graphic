# 드론 — 원격 로터 회전 복제 + 분대 트래킹 판정 이관(SoldierLab)

2026-09-23 / 완료 / 원격 프로세스에서 프로펠러 소리가 idle에 고정되고 날개도 멈춰 있던 것(회전이 복제 대상에 없었음)과, 적군을 SoldierLab 병사로 교체한 뒤 "3분대 도주 제외" 판정이 통째로 죽어 있던 것을 수정. **`Withdraw`는 도주가 아니다** — 그렇게 봤다가 시나리오를 한 번 망가뜨린 이력 포함.

같은 날 리플리케이션 쪽 큰 건(네트워크 관련성)은
`replication/2026-09-23_net_relevancy_battlefield.md`. 현재 동작은
`drone_flight_dev_guide.md` 15·16절.

---

## 1. 프로펠러 RPM / 날개 회전이 원격에서 죽어 있던 것

### 1.1 증상

자체방호 클라이언트에서 드론 소리가 **RPM을 안 따라간다.** 가속/감속과 무관하게 idle 레이어만
계속 재생된다.

### 1.2 원인

시뮬 주체가 아닌 프로세스는 물리를 아예 안 돌린다(15절 규약):

- `ADronePawn::ApplySimulationAuthority`가 `Flight->SetComponentTickEnabled(false)`
- Tick도 주체 분기에서만 `Flight->TickFlight(DeltaTime)`를 부른다

그래서 원격에선 `UDroneFlightComponent::RotorThrustN`이 **영원히 0**이다. 그리고
`UDronePropAudioComponent::TickComponent`(`DronePropAudioComponent.cpp:142`)는 그 추력에서
역산한 회전비(`GetRotorAngularSpeedRadPerSec`)를 메타사운드 파라미터로 넘긴다 → 0 → idle 고정.

같은 소스를 쓰는 `ADronePawn::UpdateRotorVisuals`도 마찬가지라, **원격에선 날개도 안 돌고
있었다**(소리만 보고 있어서 눈치채기 어려웠던 부분).

복제 항목은 위치/회전/속도/짐벌각/줌뿐 — 추력·RPM은 아예 안 건너갔다.

### 1.3 수정

회전 상태를 복제 대상에 추가했다.

| | |
|---|---|
| 복제 프로퍼티 | `RepRotorSpin01`(평균 회전비 0~1), `RepRotorSpread01`(로터간 편차) |
| 만드는 곳 | `ADronePawn::ComputeRotorSpinStats()` — 주체는 Flight의 실제 로터 추력에서, 원격은 복제된 값을 그대로 |
| 게시 경로 | **양쪽 다** — 데모(서버 주체)는 Tick에서 Rep*에 직접, 풀(클라 주체)은 `Server_ReportState`에 인자 2개 추가 |
| 소비 | 소리 `UDronePropAudioComponent::SetReplicatedSpin()`, 날개 `ADronePawn::GetRotorSpin01ForVisuals()` |

**소리와 날개가 같은 값을 보게** 한 곳에서 만든다 — 원격에서 "소리는 도는데 날개는 멈춰
있는" 식의 어긋남을 구조적으로 막는다. 원격은 로터별 편차를 안 보내고 평균 하나로 전부 같이
돈다(로터별로 보내봐야 눈·귀로 구분이 안 된다).

`SetReplicatedSpin`에 음수를 넣으면 다시 로컬 Flight를 읽는다(= 주체 프로세스의 기본 동작).

비용: 30 Hz × float 2개. 무시할 수준.

---

## 2. "3분대 도주 시 트래킹 제외"가 죽어 있던 것

### 2.1 증상

적군을 SoldierLab 병사로 교체한 뒤, 3분대가 3차 전투지로 도주를 시작해도 **드론이 계속
3분대까지 포함해 전장을 트래킹**했다. 따라서 5번 단계(이동형지휘소 복귀)로도 넘어가지 못했다.

### 2.2 원인 — 판정 근거가 새 병사에는 없는 컴포넌트였다

드론의 세 판정이 전부 `UEnemyCombatComponent`만 보고 있었다:

| 판정 | 옛 근거 | SoldierLab 병사에서는 |
|---|---|---|
| 도주 중인가 | `EEnemyState::Flee` | 컴포넌트 자체가 없음 |
| 마지막(3)분대인가 | `LastStandZoneIndex == 2` | 〃 |

호출부가 전부 `if (Combat && …) continue;` 꼴이라, 컴포넌트가 없으면 **조용히 "전원 비-도주 /
전원 비-3분대"로 통과**했다. 컴파일도 로그도 아무 문제를 안 낸다.

- `HasLivingNonLastStandEnemies()` → 3분대를 못 걸러 영원히 true → 5번 단계 진입 불가
- `CollectKnownEnemyLocations()` → 도주 중인 적을 계속 프레이밍
- `CollectEnemyLocationsForSet()` Zone3 분기 → 마지막 분대를 못 고름

### 2.3 설계 — 서버 전용 지식을 복제 층으로 옮김

SoldierLab의 원본은 `USoldierIdentityComponent::SquadId`와
`FSoldierAssignment::bBreakContact`인데, **둘 다 복제되지 않는 서버 전용 지식**이다. 그런데
드론 교전 프레이밍은 풀 시스템에서 **자체방호 클라이언트**가 돌린다(시뮬 주체).

그래서 이미 복제되는 `UDetectableTargetComponent`에 두 필드를 신설하고, 브리지가 채운다:

```
USoldierIdentityComponent::SquadId          ─┐
FSoldierAssignment::bBreakContact           ─┘ USoldierLabBridgeSubsystem::SyncSoldiers
                                                 ↓ SetSquadContext()
UDetectableTargetComponent::SquadId  /  ::bBreakingContact   (둘 다 Replicated)
                                                 ↓
ADronePawn::ResolveEnemyTrackingFacts()   ← 세 판정이 전부 이걸 통과
```

`ResolveEnemyTrackingFacts()`는 **SoldierLab 우선, 구 `BP_Enemy_kadex` 폴백**이라 구 레벨
(`kadex_test`)도 그대로 돈다.

`Quota` 편입으로 `SquadId`가 바뀌면(`ReinforceSquads`는 편입을 **영구**로 한다) 그것도 그대로
따라간다 — 옛 `LastStandZoneIndex`(저작 시 고정된 필드)보다 오히려 정확하다.

### 2.4 탐지에서도 제외 — 드론만

사용자 요구가 "타겟 디텍션 **및** 트래킹에서 제외"라, 스캔 단계에도 스위치를 뒀다.

`UTargetDetectionComponent::bIgnoreBreakingContactTargets` — **기본 꺼짐**,
`ADronePawn` 생성자에서만 켠다.

> ⚠ **이동형지휘소 RCWS는 절대 켜면 안 된다.** 그쪽이 도주해 온 3분대를 발견하는 것이
> 시나리오 6번 국면의 트리거(`bTruckEngagementLatched`)다. 거기서 걸러버리면 래치가 영영
> 안 걸려 시나리오가 멈춘다.

---

## 3. ⚠ 함정 2건 — 이 문서의 핵심

작업 중 **추측으로 두 번 틀렸다.** 둘 다 "컴파일도 되고 로그도 안 나는데 시나리오만 조용히
망가지는" 종류라, 다음에 같은 자리를 건드릴 사람을 위해 남긴다.

### 3.1 `Withdraw`는 도주가 아니다 (시나리오를 통째로 망가뜨림)

"철수 중"을 `bBreakContact || Verb == Withdraw`로 넓게 잡았다. 근거 없이 "Withdraw =
물러남 = 도주"라고 가정한 것.

**레벨이 실제로 쓰는 표**(`ScenarioConfig_1` → `DT_ScenarioSteps_ThreeStage_SoldierLab`)에서
`Withdraw`는 **평범한 전투지 이동**이다:

| 행 | 트리거 | 대상 | 동사 |
|---|---|---|---|
| `EnemyFleeToZone2` | 적 3명 사망 (= **교전 시작 직후**) | **2·3분대** | `Withdraw` → zone 1 |
| `EnemyFleeToZone3` | 적 7명 사망 | 3분대 | `Withdraw` → zone 2 |

결과(사용자 리포트와 정확히 일치):

- 교전이 시작하자마자 2·3분대가 탐지·트래킹에서 사라져 **1분대 5명만 잡힘**
- 1분대가 전멸하는 순간 "마지막 분대 아닌 생존 적 0" 판정 → 드론이 2분대를 두고
  **이동형지휘소로 날아가버림**
- 그러다 아군이 사격하면 그제서야 아군·UGV를 다시 트래킹

**진짜 도주 신호는 그 다음 행이다.** 표를 읽고 나서야 보였다:

| 행 | 트리거 | 동사 | 의미 |
|---|---|---|---|
| `Squad3Run` | `EnemyFleeToZone3` **+6초** | `BreakContact` + `HoldFire` | 3분대 접촉 단절 — 엄폐/응사 안 하고 경로 따라 달림. **여기서 제외 시작** |
| `Squad3Stand` | `CommandPostFiredNearEnemy`(80 m) | `Occupy` | 구역 동사라 `bBreakContact`가 **자동으로 풀린다** = 재포함 시점까지 일치 |

→ **`bBreakContact` 하나만 본다.** 해제 시점까지 시나리오가 알아서 맞춰준다.

> `BreakContact`는 존을 유지하고 "땅의 가격"만 바꾸는 동사다(`SoldierSquadSubsystem`의 verb
> 처리 — `Withdraw`/`MoveTo`/`Occupy`는 `FSoldierAssignment`를 통째로 리셋하므로 오히려
> `bBreakContact`를 **지운다**). 배경: `soldier_ai_lab/squad/2026-09-21_break_contact_and_targeting_exclusion.md`.

### 3.2 분대 이름은 `"Squad3"`이 아니라 `"3"`

레벨 umap에 `Squad1`/`Squad2`/`Squad3` 문자열이 보이길래 그게 분대 이름인 줄 알고
`ObservationLastStandSquadId = "Squad3"`으로 뒀다. 그건 **경로 액터 라벨**(`Squad3Path`)이었다.

정본은 **`AScenarioConfig::SquadZones`** — New_kadex_0811의 Hostile 분대 이름은
`1` / `2` / `3`이다(`USoldierIdentityComponent::SquadId`와 글자 그대로 같아야 한다).

이름이 안 맞으면 3분대 판정이 **항상 false**가 되는데, 기존 폴백("해당 분대가 하나도 안
남았으면 살아있는 적 전원으로")이 화면을 채워줘서 **아무 티도 안 났다.**

그래서 불일치를 소리 나게 만들었다 — 적은 살아있는데 마지막 분대가 0명이면 1회 경고:

```
[Drone] 마지막 분대 '3'에 해당하는 생존 적이 하나도 없습니다(생존 적 N명, 실제 분대 이름: 1, 2)
        — 살아있는 적 전원으로 폴백합니다. … Observation Last Stand Squad Id가
        AScenarioConfig의 SquadZones 이름과 같은지 확인하세요.
```

> **확인**: 빌드 후 `BP_Drone`/레벨 인스턴스의 `Gimbal|Observation ▸ Observation Last Stand
> Squad Id`가 `3`으로 보이는지. 값을 손으로 건드린 적이 없으면 새 C++ 기본값을 따라가지만,
> override로 박혀 있으면 그대로 남는다(그 경우 위 경고가 뜬다).

---

## 4. 고친 뒤의 시간표

| 시점 | 3분대 상태 | 드론 |
|---|---|---|
| `EnemyFleeToZone2` (적 3명) | 2·3분대 zone1로 `Withdraw` | **계속 트래킹** (도주 아님) |
| `EnemyFleeToZone3` (적 7명) | 3분대 zone2로 `Withdraw`. `DroneFrameZone3`가 세트 3 지시 | 1·2분대 생존자가 남아 있어 **3차 전환 보류**(`ResolveEffectiveFramingSet` + `HasLivingNonLastStandEnemies`, 16.2절 기존 게이트) |
| `Squad3Run` (+6초) | **`BreakContact`** | 3분대만 탐지·트래킹에서 제외 |
| 1·2분대 전멸 | — | 세트 3 발효 → 적군 없음 → **이동형지휘소로 복귀** |
| 지휘소 RCWS 포착 | `Squad3Stand`가 `bBreakContact` 해제 | `bTruckEngagementLatched` 래치 → **3분대 + 지휘소** |

마지막 두 줄은 서로 독립된 두 경로(래치 / 동사 해제)가 거의 같은 순간에 맞아떨어진다. 래치가
먼저 걸려도 Zone3 분기는 `bBreakingContact`를 안 보고 **마지막 분대면 무조건 넣으므로** 래치가
이긴다(의도대로). 탐지 박스만 `Squad3Stand`(80 m)까지 잠깐 늦을 수 있는데 자기치유된다.

---

## 5. 검증

2026-09-23, 2대 PC 실환경. 3분대 도주 시점에 탐지 박스와 트래킹이 같이 빠지고, 1·2분대 전멸
후 이동형지휘소로 복귀했다가, 지휘소 RCWS 교전 개시 뒤 3분대+지휘소를 잡는 것까지 사용자
확인("잘돼").

---

## 6. 교훈

시나리오 판정의 근거를 **추측으로 정하지 말 것.** 동사 이름(`Withdraw`)과 레벨의 문자열
(`Squad3Path`)이 둘 다 그럴듯했지만 둘 다 틀렸다. 정본은 두 개뿐이다:

- **동사·트리거**: 레벨 `AScenarioConfig::ScenarioStepTable`이 가리키는 DT의 실제 행
- **분대 이름**: 같은 `AScenarioConfig::SquadZones`

DT는 `DataTableTools.get_rows`로 바로 읽힌다. 읽는 데 1분이면 되는 걸 두 번 건너뛰어서 두 번
되돌렸다.
