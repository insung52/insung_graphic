# RCWS 청각 보조 — 근처 적 총성 방향 자동 조사

2026-09-17 / 진행중(코드 완료, SoldierLab 적군 연동 확인 — 반경 50m로 재조정 후 PIE 재검증 대기) / 시각 탐지도 응시할 기억도 없을 때 50m(100m→25m→50m) 안 적 총성 방향을 4초 조사하는 `InvestigatingGunfire` 단계 추가. 총성은 `UDetectableTargetSubsystem` 이벤트 버스로 적군 사격 트리거가 보고.

## 요청 / 확정 파라미터

사용자(2026-09-17): 시각 `TargetDetection`만으로는 한계 — "아무 타겟이 없을 때(락온이나 경계도)
근처에서 총을 발사하는 소리가 들리면 그 방향을 우선 자동 정찰". 질의로 확정한 것:

- **반경은 근처 100m로 짧게**(탐지 사거리 400m보다 훨씬 좁게). → 같은 날 **25m로 재조정** → **2026-09-21
  50m로 확정**(기본값 `GunfireHearingRangeCm`=5000, 사용자: "50m, 너무 길면 안 됨"). 배경은 아래
  "SoldierLab 적군 미반응 조사(09-21)" 절.
- **우선순위는 시각 발견이 항상 우선**, 아무것도 안 보일 때만 청각 보조.
- **아군 총성은 경계 대상 아님.**
- **엄폐물 뒤에서 난 총성이라도 그 방향을 봐야 함**(시선이 막혀 있어도 — 적이 나오는 순간 탐지가 잡게).

## 조사 결과

- 프로젝트에 청각 인프라가 **전혀 없다** — `AIPerception`/`ReportNoiseEvent`/`PawnNoiseEmitter` 사용처 0건.
- 적군 사격은 서버(`TickCombat` 등)가 `NetMulticast` RPC 두 개를 부르고, 그 `_Implementation`이
  BP 이벤트(`FireSingleShotAtNearestAlly`/`FireAtAlly` — 투사체 스폰/머즐 화염/사운드)를 호출하는
  구조. 호출부는 4곳(매복 단발/이동 중 단발/자세 없는 버스트/엄폐 버스트, `EnemyCombatComponent.cpp`
  `:1425/:1545/:1564/:1709`). 따라서 **C++ 멀티캐스트 구현부가 곧 "쐈다"는 순간** — 여기 한 줄이면
  호출부 4곳과 BP를 건드리지 않고 전부 잡힌다.
- 옵션 A(자체 이벤트 버스, 채택) vs 옵션 B(UE `AIPerception` Hearing, 기각): B는 컨트롤러/센스 설정이
  따로 필요하고 덜 결정적이며 얻는 게 없다 — RCWS 자동조준은 컨트롤러 없이 컴포넌트 틱에서 돈다.

## 설계

### 총성 이벤트 버스 (`UDetectableTargetSubsystem`)

- `FGunfireEvent{Location, Instigator(TWeakObjectPtr), Faction, Time}`.
- `ReportGunfire(Location, Instigator, Faction)` → `RecentGunfire` 배열에 추가. 보고 시점에
  10초(`GunfireRetentionSeconds`) 지난 항목을 앞에서 잘라내고, 128개(`MaxGunfireEvents`)를 넘으면
  가장 오래된 것 제거. `GetRecentGunfire()`로 읽기(오래된 순).
- **서버 전용, 리플리케이트 안 함** — 듣는 쪽(RCWS 자동조준)이 서버 권위라서.
- **결정적 데이터** — 실제 오디오 감쇠/누가 듣고 있는지와 무관. 탐지의 픽셀 게이트 기준 해상도를
  고정한 것과 같은 원칙(AI 거동이 관전 상태에 따라 달라지면 안 됨).

### 보고 (`UEnemyCombatComponent::ReportGunfireToSubsystem`)

- `Multicast_TriggerFireSingleShotAtNearestAlly_Implementation` / `Multicast_TriggerFireAtAlly_Implementation`
  맨 앞에서 호출. 멀티캐스트는 서버에서도 로컬 실행되므로 `HasAuthority` 게이트만 걸면 **서버에서
  딱 한 번**.
- 위치 = `GetFireLaneOrigin()` = 소총 `MuzzlePoint`(없으면 액터 위치 폴백, 사격선 검사와 동일).
- `Faction = Enemy` 고정. 아군/RCWS 사격은 보고하지 않는다(사용자 확정). 나중에 넓히려면 Faction만
  실어 보고하고 듣는 쪽이 거르면 됨.
- **버스트는 첫 발 1회만 보고** — BP 이벤트 안의 개별 탄까지 잡으려면 BP를 손대야 해서 안 함.
  "총성이 났다"는 신호로는 충분.

### 소비 (`URCWSFireControlComponent`)

`UpdateAutoAim`의 `!Target` 분기 — 응시(`WatchingLastKnown`) `return` **뒤**에 위치. 즉 우선순위:

```
시각 Tracking ──▶ 기억 응시 WatchingLastKnown ──▶ 총성 조사 InvestigatingGunfire ──▶ 탐색 스윕 Searching
   ▲                                                    │ (bInvestigateGunfire && GunfireInvestigateSeconds>0)
   │  누구든 DetectedTargets에 보이면 어느 단계에서든 즉시  │ PickGunfireToInvestigate(Now):
   └────────────────────────────────────────────────────┤   최근 GunfireMemorySeconds(3s) 안 + 반경
                                                        │   GunfireHearingRangeCm(50m) 안 + Faction==Enemy
                                                        │   + Time > InvestigatedGunfireTime(이미 조사한 것 제외)
                                                        │   + bRespectEnemyTargetingExclusion(도주 분대 제외)
                                                        │   → 가장 최근 것. 사격자 경계도 ≥ GunfireAlertnessFloor
                                                        │ 새 총성 → GunfirePoint/타이머 갱신(조사 연장)
                                                        │ SlewSightTowardWorldPoint(GunfirePoint) — LOS 검사 없음
                                                        ▼ GunfireInvestigateSeconds(4s) 만료
                                             ResumeSearchSweepFromCurrentAim() → Searching (블렌드 아웃)
```

- 슬루는 추적/응시와 같은 `SlewSightTowardWorldPoint`(`MaxAutoAimSlewRateDegPerSec`).
- LOS 검사 없음 — 엄폐물 뒤 총성도 그 방향을 본다(사용자 확정).
- 조사 중에도 매 틱 `SelectNearestEnemyTarget`이 먼저 돌므로 누구든 보이면 즉시 Tracking.
- `CurrentAutoAimTarget=null`이라 락온 게이지 0, 발사 없음(응시와 동일).
- **경계도 연동** — 사격자의 `TargetMemories` 항목 `Alertness`를 최소 `GunfireAlertnessFloor`(0.5)까지
  올린다("들었으니 경계"). 그 적이 시야에 들어오면 거리 가중/락온 단축이 바로 적용.
  `LastAimLocation`은 **안 넣는다**(본 적이 없으니) — 이를 위해 `FRCWSTargetMemory::bHasLastAimLocation`
  신설. 응시 진입 조건이 이 플래그를 보므로 총성으로만 만들어진 기억이 응시 지점(원점)으로
  쓰이는 일이 없다.
- `WatchRemainingSeconds`를 조사 남은 시간으로 재사용, `AutoAimPhase` 값 3 = `InvestigatingGunfire`.
- 로그: `근처 적 총성(NNm) → 총성 방향 4.0초 조사`(조사 진입 시 1회), `총성 조사 종료 → 탐색 스윕 복귀`.

## 변경 파일

| 파일 | 내용 |
|---|---|
| `Detection/DetectableTargetSubsystem.h/.cpp` | `FGunfireEvent` USTRUCT, `ReportGunfire()`, `GetRecentGunfire()`, `RecentGunfire` 배열(보존 10s / 최대 128) |
| `Soldiers/EnemyCombatComponent.h/.cpp` | `ReportGunfireToSubsystem()` 신규 — 두 `Multicast_TriggerFire*_Implementation`에서 호출(HasAuthority 게이트, 위치 `GetFireLaneOrigin()`) |
| `Vehicles/RCWSFireControlComponent.h/.cpp` | `ERCWSAutoAimPhase::InvestigatingGunfire`, `FRCWSTargetMemory::bHasLastAimLocation`, 프로퍼티 5개(아래 표), `PickGunfireToInvestigate()`, 상태 `GunfirePoint`/`InvestigatedGunfireTime`/`GunfireInvestigateStartTime`, `UpdateAutoAim` `!Target` 분기에 조사 단계 삽입, 스윕 복귀 로그에 `총성 조사 종료` 분기 |

BP·레벨 인스턴스는 안 건드렸다(기본값 그대로 켜짐).

## 튜닝값 (`Fire Control|Auto-Aim|Hearing`)

| 프로퍼티 | 기본값 | 의미 |
|---|---|---|
| `bInvestigateGunfire` | true | 끄면 총성 무시(예전 동작) |
| `GunfireHearingRangeCm` | 5000 (50m; 100→25→50) | 총구 기준 이 거리 안의 적 총성만. 결정적 반경 |
| `GunfireMemorySeconds` | 3 | 이 시간 안에 난 총성만 새 조사 대상. 길면 이미 자리를 뜬 적의 옛 총성을 쫓음 |
| `GunfireInvestigateSeconds` | 4 | 총성 지점 응시 시간. 0이면 끔. 새 총성이 오면 타이머 갱신 |
| `GunfireAlertnessFloor` | 0.5 | 사격자 경계도 최소값. 0이면 끔 |

버스 쪽 상수(코드 고정): 보존 10초, 최대 128개 — 적 15명이 초당 몇 발 쏴도 수십 개 수준.

## 우선순위 / 기존 시스템 연동

- **응시(09-16)와의 관계** — 응시가 총성보다 우선. 응시 중 총성이 나도 무시되고, 응시 만료 후
  `GunfireMemorySeconds`(3s) 안이면 그때 조사로 이어진다. 조사 만료는 응시 만료와 같은
  `ResumeSearchSweepFromCurrentAim()` 블렌드 아웃.
- **경계도** — 총성만으로 0.5까지. 이후 보이면 `AlertnessRiseSeconds`로 1까지, 안 보이면
  `AlertnessDecaySeconds`(15s)로 감쇠(기존 규칙 그대로).
- **도주 분대 제외**(`bRespectEnemyTargetingExclusion` → `IsTargetableByAlliesAndUGV`) —
  `PickGunfireToInvestigate`에서도 같은 기준. 3차 도주 분대의 견제 사격을 UGV가 쫓아 보면 연출이
  깨진다. 트럭처럼 플래그가 꺼진 쪽은 도주 분대 총성도 본다.
- **시나리오** — 적은 `BeginEngageAtCurrentZone` 전엔 안 쏘므로 1차 교전 **시작 시점은 안
  앞당겨진다**. 실효는 2·3차 이동/도주 분대의 견제 사격, 엄폐 뒤 사격 때 "안 보이는데 어디서
  쏘는지"를 포탑이 먼저 향하는 것. `EnemyDetected` 트리거(`HasDetectedEnemyTarget`)는 탐지 기반이라
  조사 단계와 무관.
- **Remote/AutoSurveillance** — 자동조준 모드가 아니면 총성을 안 듣는다(분기에서 `Searching` 리셋).

## 남은 작업 / 검증 체크리스트

1. [x] **빌드 — 에디터 닫고 풀 빌드.** 새 UPROPERTY 5개 + UENUM 값 + USTRUCT(`FGunfireEvent`) →
   Live Coding 금지(09-15 `BarrelSpinGaugeValue` missing property 사고와 같은 종류). (09-21 이전 빌드 확인 —
   `L_SoldierScenario` 인스턴스에 `bInvestigateGunfire=true`, `GunfireHearingRangeCm=2500` 들어가 있음.)
   → **반경 50m 변경으로 헤더가 다시 바뀌었으니 재빌드 필요.**
2. PIE 검증(데모 모드 UGV AutoAim/AutoFire — 이제 `L_SoldierScenario`가 주 테스트 레벨)
   - [ ] 시야 밖 적이 쏘면 `근처 적 총성(NNm) → 총성 방향 4.0초 조사` 로그 + 스윕이 그 방향으로 돎
   - [ ] 조사 중 다른 적이 보이면 즉시 Tracking(시각 우선)
   - [ ] 아군 사격에 무반응, RCWS 자체 사격에 무반응
   - [ ] 50m 밖 총성 무반응(로그 없음)
   - [ ] 4초 뒤 `총성 조사 종료 → 탐색 스윕 복귀`, 스윕이 현재 방향에서 이어짐(되돌림 없음)
   - [ ] 응시 중 총성 → 무시되다가 응시 만료 후(3s 안이면) 조사로 이어지는지
   - [ ] 조사로 경계도 0.5가 된 적이 나타났을 때 락온 충전이 짧아지는지(`bLogAutoFireReadiness` `Alert=`)
   - [ ] 트럭(`BP_TitanTruck_C_4`)에서도 동일
3. 튜닝 — 50m/3s/4s/0.5가 실제 교전에서 적절한지. 버스트 중 계속 나는 총성이 조사를 무한 연장하지
   않는지(버스트 1회 보고라 그럴 일은 없어야 함 — 단 SoldierLab 경로는 **매 발** 보고, 아래 절 참고).
4. [x] `guide/rcws_fire_control_dev_guide.md` §3.2, `guide/detection_dev_guide.md` §2에 09-17 단락 추가.

## SoldierLab 적군 미반응 조사 (2026-09-21)

**증상**: `L_SoldierScenario`(적·아군이 SoldierLab `BP_Soldier_Hostile/Friendly`로 교체된 테스트 레벨)에서
"타겟 없을 때 총성 방향 조사"가 한 번도 안 일어남 — `근처 적 총성` 로그 0건.

**경로 점검 — 전부 정상이었다.** 구 `BP_Enemy_kadex`의 `EnemyCombatComponent::ReportGunfireToSubsystem`
경로 대신, 다른 세션이 09-17에 놓아둔 **`USoldierLabBridgeSubsystem`**(`Soldiers/SoldierLabBridgeSubsystem.cpp`)이
SoldierLab 쪽 총성을 옮긴다:

```
BP_AR4Rifle.Shoot → CallOnWeaponFired
 → BP_SoldierCharacter.OnWeaponFired_Event → USoldierPerceptionLibrary::BroadcastGunshot(GetActorLocation, 100m, self)
 → USoldierRegistrySubsystem::CountGunshot → OnGunshot.Broadcast   (Hostile && USoldierHealthComponent 보유 && HasAuthority)
 → USoldierLabBridgeSubsystem::OnSoldierGunshot                     (Hostile만 통과)
 → UDetectableTargetSubsystem::ReportGunfire(Enemy)
 → URCWSFireControlComponent::PickGunfireToInvestigate
```

- 로그: 적 15명 `[SoldierLabBridge] DetectableTarget(Enemy) 부착` ✓, `UGVArriveZone1 → RCWS ARM+AutoFire` ✓
  (Remote가 아니라 자동조준 모드에 있었음)
- 인스턴스: `bInvestigateGunfire=true`, `GunfireHearingRangeCm=2500`, `bRespectEnemyTargetingExclusion=false`
- 구 경로와 다른 점 하나: 구 `EnemyCombatComponent`는 버스트당 1회 보고였는데 SoldierLab 경로는
  `Shoot` 이벤트마다(= **매 발**) 보고한다. `ReportGunfire`가 10s/128개로 자르니 문제는 없고, 오히려
  조사 연장이 더 자연스럽다.

**진짜 원인 — 거리.** UGV 1차 목적지 `TargetPoint_1`(-4554, -2206)에서 적 배치까지 `Hostile_C_0`(-9300, -7500)
**71m**, `Hostile_C_14`(-7800, -9100) **76m**. 반경이 09-17에 100m → 25m로 줄어든 상태라 총성이 한 번도
반경 안에 들어오지 않았다(경로가 끊긴 게 아니라 필터에서 전부 걸러진 것).

**조치**: 기본값 **50m**(`GunfireHearingRangeCm=5000`)로 재조정(사용자: "너무 길면 안 됨"). 참고로 SoldierLab
병사끼리의 청각(`BroadcastGunshot` 인자)은 100m라 RCWS가 여전히 더 짧다. 헤더 변경이라 재빌드 필요.

**교훈**: 이 기능이 "안 된다"고 보일 때 확인 순서 — (1) 로그에 `근처 적 총성`이 있는가, (2) RCWS가 AutoAim/
AutoFire인가(`데모 자동사격: ... AutoFire` 로그), (3) **적 총성 위치와 UGV 총구 거리가 반경 안인가**(레벨
배치를 MCP로 재어 볼 것), (4) 그 다음에야 보고 경로(`[SoldierLabBridge]` 부착 로그 → `OnGunshot` 게이트).

## 알려진 한계

- **버스트 1회 보고** — 버스트 안의 개별 탄은 보고되지 않는다(BP 이벤트 안에서 쏨). 조사 연장은
  다음 버스트/단발에서만.
- **서버 전용** — `RecentGunfire`/`GunfirePoint`는 리플리케이트하지 않는다(`AutoAimPhase`만).
  클라 UI가 조사 지점을 그리려면 별도 노출 필요(응시 `WatchPoint`와 같은 상황).
- **결정적 반경** — 실제 사운드 감쇠/차폐와 무관. 소음기, 지형 차폐 등은 반영 안 됨(의도).
- **총성 지점 = 총구** — 쏜 순간의 위치라 이동하며 쏘는 적은 4초 조사 동안 그 자리에 없을 수 있다.
  기억 응시가 마지막 **보이던** 지점을 보는 것과 같은 성격.
- 아군/RCWS 총성은 보고 자체가 안 되므로, 나중에 "아군 교전 방향 지원" 같은 기능을 붙이려면
  보고 범위와 `PickGunfireToInvestigate`의 Faction 필터를 함께 넓혀야 한다.
