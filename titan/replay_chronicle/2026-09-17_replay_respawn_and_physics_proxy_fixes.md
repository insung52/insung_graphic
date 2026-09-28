# Chronicle 2일차 — 체크포인트 재스폰이 CDO를 드러내는 문제, 물리 프록시 지터/정지 수정, UI 정리

2026-09-17 / 완료 / 리플레이에서만 재발하던 "사격→엄폐 미끄러짐"의 정체는 **체크포인트 재스폰이 레벨 인스턴스 오버라이드를 버리고 CDO(옛 `ABP_Enemy_kadex2`)로 돌아가는 것** → CDO AnimClass를 `_New`로. UGV/드론이 1초마다 앞뒤로 튀던 건 물리 복제 기본 모드의 하드 스냅 → `PredictiveInterpolation` 강제. 스크럽 뒤 UGV가 레벨 원위치에 박혀 있던 건 초기 `ReplicatedMovement` 1회 유실 → 1.5초 스냅 창. 카메라/일시정지 UI 동작 정리.

> 09-16 문서(`2026-09-16_chronicle_replay_plugin.md`)의 후속. 툴 구조·엔진 함정 12건은 그쪽,
> 여기는 **재생 결과가 실제 게임과 달라 보이던 원인 3건과 그 수정**, 그리고 UI 변경분만.
> 같은 조사에서 나온 **게임 코드 리플리케이션 결함 4건**(적군 애니/피격/사망, 시나리오 누수)은
> 리플레이 전용이 아니라 실기 클라이언트도 같으므로
> `replication/2026-09-17_enemy_anim_death_replication_gaps.md`로 분리했다.
> 사용자 가이드는 여전히 `Plugins/Chronicle/README.md` / `Chronicle_Guide.html`.

---

## 1. 체크포인트 재스폰은 CDO에서 만든다 — 인스턴스 오버라이드 소멸

### 1-1. 증상

`replication/…gaps.md` §1~§4를 다 고친 뒤에도 리플레이에서 적군이 **사격 직후 엄폐로 이동할 때
서서 미끄러졌다** — 09-11에 고친 바로 그 증상(`ai_combat/2026-09-11_enemy_slide_and_fire_gate.md`)이
리플레이에서만 되살아났다. 실기 PIE(리슨서버)에선 정상.

### 1-2. 진단 로그가 찍은 것

`Enemy.ClientAnimDiag 1`(`replication/…gaps.md` §5):

```
ABP(ABP_Enemy_kadex2_C) 상태=Knee Speed=600 … 슬롯 Fire=1.00
```

AnimClass가 **`ABP_Enemy_kadex2`** — `FireRecoil` 슬롯이 전신으로 물려 있던 09-11 이전 ABP다.
레벨 인스턴스 15명은 전부 `ABP_Enemy_kadex2_New`를 쓰지만, **`BP_Enemy_kadex` CDO의 메시
AnimClass는 여전히 `ABP_Enemy_kadex2`**였다(09-11 문서 함정 "CDO를 고쳐도 인스턴스는 안 바뀐다"의
정확히 반대 방향 — 인스턴스만 고치고 CDO는 안 고친 상태).

### 1-3. 왜 리플레이에서만

`DemoNetDriver.cpp:3368 RespawnNecessaryNetStartupActors` — 체크포인트 로드/스크럽 때 레벨 배치
("net startup") 액터를 **클래스에서 새로 스폰**한다. 복제되는 프로퍼티는 스트림으로 채워지지만
AnimClass처럼 복제되지 않는 프로퍼티의 인스턴스별 오버라이드는 `.umap`에만 있어서 재스폰된
액터엔 없다 → CDO 값. 실기 클라이언트는 레벨을 직접 로드하므로 인스턴스 값을 그대로 가진다.

**일반 교훈: 복제되지 않는 프로퍼티의 인스턴스 전용 오버라이드는 리플레이에서 전부 CDO 값으로
보인다.** 리플레이에서만 뭔가 다르면 먼저 CDO와 인스턴스 값을 비교할 것.

### 1-4. 수정

`Default__BP_Enemy_kadex_C:CharacterMesh0`의 `AnimClass`를 `ABP_Enemy_kadex2_New`로(MCP
`set_properties`, 컴파일·저장). 아군은 CDO/인스턴스 모두 `ABP_Ally_kadex_T`로 이미 일치했다.

## 2. 물리 프록시 지터 — 기본 물리 복제 모드의 하드 스냅

### 2-1. 증상

1배속 재생에서 UGV/드론이 **약 1초마다 뒤로 팍, 앞서갔다 다시 뒤로**. 녹화 당시 New_kadex_0811이
20~30fps라 서버 프레임당 하나인 물리 타겟이 성겼다.

### 2-2. 원인

`EPhysicsReplicationMode::Default`는 서버 위치를 "목표"로만 받고 그 사이를 로컬 물리로 채우다가
오차가 `p.MaxLinearHardSnapDistance`(`FRigidBodyErrorCorrection` 기본 400cm)를 넘으면 하드 스냅.
샘플이 성기면 로컬 외삽이 앞서가다 스냅으로 되돌아오는 게 반복된다. 8월 감사에서
`physicsReplicationMode`를 MCP로 못 찾아 `Default`로 방치했던 그 항목(`replication_audit.md` §8
UGV 섹션 미해결 체크박스)이 리플레이에서 눈에 띈 것.

### 2-3. 수정 (`ChronicleSubsystem`)

설정 `bPredictiveInterpolationInReplay`(기본 on, `ChronicleSubsystem.h:68`). 재생 월드를 1초마다
훑어 **시뮬레이티드 프록시 + 이동 복제 + 물리 시뮬 루트**인 액터에
`AActor::SetPhysicsReplicationMode(PredictiveInterpolation)`(`ChronicleSubsystem.cpp:235-267`).
엔진 `ActorReplication.cpp:1181`이 모드 전환 시 현재 타겟을 비우므로 재생 중 전환해도 안전.
체크포인트 재스폰으로 액터가 새로 생기니 한 번이 아니라 주기 스캔(적용 집합은
`PredictiveInterpolationApplied`로 중복 방지). 로그 `리플레이 물리 프록시 N개에
PredictiveInterpolation 적용`. 캐릭터(캡슐, 물리 아님)는 대상이 아니다.

리플레이 월드에만 건다 — 실기 클라이언트의 UGV 모드는 이 툴이 안 바꾼다. 실기에서도 끊기면
`replication_audit.md` §8 그 항목대로 `VehicleMesh`에서 직접 바꿀 것.

## 3. 스크럽 뒤 UGV가 레벨 원위치에 서 있는 문제 — 초기 `ReplicatedMovement` 1회 유실

### 3-1. 증상

스크럽하면 UGV가 **레벨 시작 위치**에 나타나 포탑만 돌아간다. 3초짜리
`np2.PredictiveInterpolation.ErrorAccumulationSeconds` 하드 스냅 대기가 아니다 — 타임라인을 다시
움직일 때까지 **계속** 거기 있다.

### 3-2. 메커니즘

1. §1처럼 재스폰된 UGV는 `ReplicatedMovement`를 초기 리플리케이션으로 **한 번** 받는다
   (`OnRep_ReplicatedMovement → PostNetReceivePhysicState → SetRigidBodyReplicatedTarget`,
   `ActorReplication.cpp:304`).
2. 그 순간 물리 바디가 아직 준비 전이면 타겟이 버려진다.
3. 그 장면에서 UGV가 정지해 있으면(전투지에 주차) 서버 쪽 `ReplicatedMovement`가 다시 안 바뀌고
   → 델타 복제라 OnRep이 다시는 안 온다. 포탑 yaw 같은 일반 복제값만 갱신되니 "제자리에서 포탑만
   도는" 그림.

### 3-3. 수정 (`ChronicleSubsystem`)

`BeginSnapWindow(1.5초)`(`ChronicleSubsystem.cpp:180`)를 **스크럽 완료 시**(`:622`)와 **데모 재생
시작 시**(`:297`, 트래블 뒤 재개 포함)에 연다. 창이 열려 있는 동안 0.25초마다
`SnapPhysicsProxiesToReplicatedMovement()`(`:187-231`):

- 대상은 §2와 같은 조건 + `GetReplicatedMovement().bRepPhysics`(일반 이동 복제는 엔진이
  `PostNetReceiveLocationAndRotation`으로 직접 적용하므로 제외).
- 목표 = `FRepMovement::RebaseOntoLocalOrigin(Rep.Location, Actor)`. 0에 가까우면 초기 리플리케이션
  전(기본값)이라 건너뜀 — **원점으로 보내면 안 된다**.
- 현재 위치와 50cm 넘게 다를 때만 `SetWorldLocationAndRotation(…, TeleportPhysics)` + 선속/각속도.
- **일시정지 중에도 돈다**(스크럽 직후 정지 상태에서 바로 제자리에 보이게). 정상 재생 중엔 오차가
  작아 no-op. 로그 `스크럽 직후 물리 프록시 N개를 복제 위치로 스냅`.

물리 리플리케이션 경로를 거치지 않고 복제값을 직접 꽂는 것이라 §2의 모드와 무관하게 동작한다.

## 4. UI/동작 변경 (09-16 문서 이후)

| 항목 | 09-16 | 09-17 |
|---|---|---|
| 드롭다운에서 액터 선택 | Follow | **자유 카메라를 액터 뒤로 텔레포트**(`GoToActorByName`) |
| `Go/Follow` 버튼 | — | `SetViewTarget` 팔로우 |
| `Free` 버튼 | 옛 스펙테이터 위치로 복귀 | **현재 시점**(`GetPlayerViewPoint`)에서 분리 |
| 재생 끝에서 Play | 정지 | `Scrub(0)` 후 재생(처음부터) |
| 시작 상태 | 재생 | **일시정지 시작**(`bStartPaused`, 기본 on) |

아래는 09-16 §3 함정 목록에 이미 있는 것들의 구현 확정 — 상태 유지 루프 `TickPlaybackState`
(`bDesiredPaused` 의도값, 따라갈 액터 이름, 마지막 스펙테이터 트랜스폼을 매 프레임 재강제; 첫
스크럽이 심리스 트래블 `DemoNetDriver.cpp:2584`), 일시정지 중 스펙테이터 이동
(`bShouldPerformFullTickWhenPaused`를 리플렉션으로, 축 바인딩 `bExecuteWhenPaused`;
`PlayerController.cpp` TickActor 일시정지 경로가 RotationInput을 0으로 밀고 UpdateRotation 전에
리턴), EUW `bIsEnabledInPIE` 리플렉션 강제, ASCII 버튼 라벨(Roboto에 ▶/⏸ 없음), 팝업 Border는
루트 Overlay 직계 자식이어야 ScrollBox가 스크롤됨. 가이드 두 파일에 반영돼 있음.

## 5. 상태

- 전부 구현·빌드·**사용자 리플레이 검증 완료("잘됨")** — UGV 지터 없음, 스크럽 뒤 제자리 스냅,
  적군 정상 이동(§1 + `replication/…gaps.md`).
- Chronicle 소스는 CL 486(09-16) 이후 수정분 미제출(체크아웃 상태). `BP_Enemy_kadex.uasset`도
  체크아웃/수정.
- 낙하산 숨김 복제(09-16 §4-2)는 여전히 미확인.

## 관련 문서

- `2026-09-16_chronicle_replay_plugin.md` — 툴 구조, 엔진 함정 12건, 낙하산 사각지대 후보.
- `replication/2026-09-17_enemy_anim_death_replication_gaps.md` — 같은 날 조사에서 나온 게임 코드
  리플리케이션 결함 4건(실기 클라이언트 공통).
- `ai_combat/2026-09-11_enemy_slide_and_fire_gate.md` — §1이 되살린 원래 증상과 ABP 수정.
- `replication_audit.md` §8 UGV 섹션 — `physicsReplicationMode` 미해결 항목(§2의 배경).
