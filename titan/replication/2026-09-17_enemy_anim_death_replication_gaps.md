# 적군 이동 애니메이션·피격·사망 리플리케이션 사각지대 4건 + 리플레이 월드의 시나리오 누수

2026-09-17 / 완료(2-PC 실기 검증 대기) / Chronicle 리플레이로 드러난 클라이언트 쪽 결함 — 적군 `GaitTopSpeed`/`IsSprinting` 미복제(→ 서서 미끄러짐), 피격 스프링 서버 전용(→ 흔들림 없음), 사망 래그돌/총 낙하 서버 전용(→ 서서 죽고 총이 떠 있음), 리플레이 월드에서 시나리오 스텝이 도는 넷모드 판정 구멍. 전부 자체방호축 클라이언트에도 그대로 해당. 코드+BP 수정 완료, 리플레이에서 검증됨.

> 8월 리플리케이션 작업의 기록은 `replication_audit.md` §0-1/§8. 이 문서는 그 위에 얹는 추가분이고,
> 발견 경로(리플레이 툴)는 `replay_chronicle/2026-09-16_chronicle_replay_plugin.md`, 리플레이 전용
> 수정은 `replay_chronicle/2026-09-17_replay_respawn_and_physics_proxy_fixes.md`.
>
> **리플레이 재생 = 클라이언트 하나 더 붙인 것.** DemoNetDriver는 녹화된 리플리케이션 스트림을
> 클라이언트 입장으로 재생하므로, 리플레이에서 안 보이는 건 2대 PC 풀 시스템의 SelfDefense
> 클라이언트에서도 안 보인다(2026-09-16 문서 §4-2에서 세운 가설이 이번에 4건 전부에서 사실로
> 확인됨). 반대로 §4(시나리오 누수)만은 리플레이 전용이고 실기 클라이언트엔 없다.

---

## 0. 한눈에 보기

| # | 증상(클라/리플레이) | 원인 | 수정 | 서버 동작 |
|---|---|---|---|---|
| 1 | 적군이 idle 포즈로 미끄러짐, 아군은 걷지만 뛰기 앵커 불일치 | `GaitTopSpeed`/`IsSprinting` BP 변수 미복제 | 두 BP에서 `Replicated`로 전환 | 불변 |
| 2 | 피격 흔들림 없음 | `TriggerHitReactionPhysics`가 서버 체인에서만 불리고, 스프링 적분도 권위 게이트 뒤 | Multicast + 비권위 틱에서 스프링 적분 | 불변 |
| 3 | 서서 죽고 총이 손 위치에 떠 있다가 본체 소멸 | 사망 코스메틱 체인 전체가 서버 전용 BP, 클라엔 `IsDead`만 감 | 사망 정보 구조체 복제 + `OnRep`에서 C++로 동일 코스메틱 | 불변 |
| 4 | (리플레이만) 시나리오 스텝/드론 자율비행/적군 이동이 재생 월드에서 새로 발동 | `GetNetMode()==NM_Client` 가드가 재생 월드 BeginPlay 시점엔 안 걸림 | `IsPlayingReplay()` 게이트 + `BeginMove/Flee` HasAuthority 방어 | 불변 |
| 5 | (진단) | — | `Enemy.ClientAnimDiag` cvar 신설 | — |

**왜 8월 작업이 못 잡았나**: `replication_audit.md` §8의 "Blueprint 자세/애니메이션 변수 복제"
목록(`IsProne`/`IsKneeling`/`IsHoldingWeapon?`/`HasTarget`/`AimPitch`/`LeanAlpha`/
`BurstShotsRemaining`/`IsReloading?`/`IsDead`/`TargetLocation`)은 2026-08-11 시점의 ABP 입력을
기준으로 만든 것이다. **`GaitTopSpeed`/`IsSprinting`은 2026-08-25 gait 재설계**
(`ai_combat/enemy_locomotion_animation_pipeline.md`)에서 ABP `Speed` 공식의 입력으로 새로 들어온
변수라 그 목록에 없었고, 이후 아무도 복제 목록에 추가하지 않았다. 사망 코스메틱(래그돌/총 분리)은
8월 작업의 스코프에 애초에 없었다(`IsDead` 플래그만 복제, ABP엔 죽음 상태도 없음). 당시 실기
검증은 "위치/자세가 맞는가"까지였고 사망 장면을 클라이언트 화면에서 본 기록이 없다.

## 1. 적군 이동 애니메이션 — `GaitTopSpeed`/`IsSprinting` 미복제

### 1-1. 메커니즘

`ABP_Enemy_kadex2(_New)`/`ABP_Ally_kadex*`의 이동 축 입력(`ai_combat/enemy_locomotion_animation_pipeline.md` §1):

```
Speed = Clamp(VelocityXY / GaitTopSpeed, 0..1) * (IsSprinting ? 600 : 300)
```

- 적군 `GaitTopSpeed`는 **`UEnemyCombatComponent::TickComponent` 안에서만** 쓰인다 —
  `ApplyGaitForDesiredSpeed`(`EnemyCombatComponent.cpp:2588-2590`)와 `MoveToward`(`:2505`). 이
  틱은 2026-08 작업으로 통째로 `HasAuthority` 게이트 뒤에 있다(`:231`). 클라이언트에선 한 번도
  안 써지므로 CDO 기본값 0 → `VelocityXY / 0` → Clamp → `Speed`가 항상 0 → 위치는 서버 값대로
  움직이는데 포즈는 idle. **"서서 미끄러지는 적군"**.
- 아군은 `BP_ThirdPersonCharacter` EventTick이 모든 머신에서 로컬로 `GaitTopSpeed`를 계산해 넣으므로
  대체로 걸었고, `IsSprinting`만 안 맞아 뛰기 앵커(600)를 못 타는 정도였다.

### 1-2. 수정

`BP_Enemy_Base`와 `BP_ThirdPersonCharacter`의 `GaitTopSpeed`/`IsSprinting` 변수를 `Replicated`로
(MCP `set_variable_replication`, 컴파일·저장). RepNotify 불필요 — ABP가 매 프레임 직접 읽는 값이라
8월의 `AimPitch`/`TargetLocation`과 동일 패턴.

델타 리플리케이션이라 서버 값이 바뀔 때만 오는데, 두 값 모두 상태 전환마다 서버가 다시 쓰므로
문제없다. 단 **클라이언트가 이 값을 로컬로 덮어쓰면 서버 값이 다시 안 바뀌는 한 영영 틀린 채로
남는다** — §4의 방어 가드가 필요한 이유.

## 2. 피격 리액션 — 서버 전용 트리거 + 서버 전용 적분

### 2-1. 원인

- `TriggerHitReactionPhysics`는 평범한 `BlueprintCallable`이었고, 부르는 곳이 `BP_Enemy_Base`
  `EventAnyDamage` 체인 = 서버 전용(데미지는 서버만 받음). 클라이언트에선 킥 자체가 안 들어옴.
- 설령 들어와도 스프링 적분 `TickHitReactionSpring`이 권위 게이트 안쪽(`:432`)에 있어서 오프셋이
  갱신되지 않았다.

### 2-2. 수정 (`Soldiers/EnemyCombatComponent.h/.cpp`)

```
TriggerHitReactionPhysics(Dir, Loc, Bone)                      // :960 — BP 호출 지점 시그니처 불변
  ├─ HasAuthority && NetMode != NM_Standalone
  │     → Multicast_TriggerHitReactionPhysics(...)             // :974, NetMulticast Unreliable
  │           └─ _Implementation → TriggerHitReactionPhysicsLocal
  └─ TriggerHitReactionPhysicsLocal(...)                       // :979 — 원래 본문 그대로 이름만 변경
```

- 리슨서버는 Multicast 루프백으로 자기도 `Local`을 한 번 더 받지만, 서버 쪽은 어차피 직접 호출
  경로가 이미 있으므로 `Multicast` 뒤에 무조건 `Local`을 부르는 게 아니라 **권위+네트워크면
  Multicast만, 그 외(Standalone)면 Local만** 돈다 — 서버에서 두 번 킥 들어가지 않는다.
- `TickComponent` 비권위 분기(`:231-239`)에서 `return` 전에 `TickHitReactionSpring(DeltaTime)`을
  돌린다. 순수 코스메틱(ABP 오프셋 변수만 씀, `ai_combat/enemy_hit_reaction_physics_system.md`)이라
  서버 상태와 다툴 것이 없다. 서버 경로(`:432`)는 그대로.
- Unreliable로 둔 이유: 발사 이펙트 Multicast와 같은 논리 — 한 번 빠져도 다음 피격에 정상.

## 3. 사망 — 서버 전용 BP 체인 → 사망 정보 복제 + 클라이언트 C++ 코스메틱

### 3-1. 원인

`BP_Enemy_Base::EventAnyDamage` 사망 체인(서버 전용) 전체:

```
SetIsDead(true) → StopMovement/DisableMovement → Capsule NoCollision → Mesh QueryAndPhysics
→ PhysicalAnimation.ApplyPhysicalAnimationSettingsBelow("Hips", 강도 전부 0, MaxLinear/AngularForce 1e6, includeSelf)
→ Mesh.SetAllBodiesSimulatePhysics → SetAllPhysicsLinearVelocity(LastHitVelocity)
→ DelayUntilNextTick → AddImpulseAtLocation(LastHitDirection*DeathImpulseMagnitude, LastHitLocation, LastHitBoneName)
→ Gun 컴포넌트 DetachFromComponent(KeepWorld)+SetSimulatePhysics
→ CurrentRifle 액터 DetachFromActor(KeepWorld) + WeaponMesh QueryAndPhysics + SetSimulatePhysics
→ Delay 5 → DestroyActor(rifle) → DestroyActor(self)
```

클라이언트에 도달하는 건 `IsDead`(plain Replicated, RepNotify 없음)뿐이고 ABP에 죽음 상태가 없으니
**서서 죽는다.** `CurrentRifle`은 복제 안 되고 라이플 액터의 분리도 서버 로컬이라 총은 손 소켓
위치에 그대로 떠 있다가, 5초 뒤 서버의 `DestroyActor`가 복제돼 라이플→본체 순으로 사라진다.

### 3-2. 왜 Multicast RPC가 아니라 복제 프로퍼티인가

리플레이는 체크포인트에서 **복제된 상태**를 복원한다. 사망 시점을 지나 스크럽하면 RPC는 이미
지나간 것이라 없고, 프로퍼티는 체크포인트에 남아 `OnRep`이 다시 온다. 2대 PC에서도 늦게 접속한
클라이언트/관련성 재진입 시 같은 장점. 그래서 값은 프로퍼티로 보내고 코스메틱은 `OnRep`에서
한 번만 실행한다(`Serial`로 중복 방지).

### 3-3. 수정 (`Soldiers/EnemyCombatComponent.h/.cpp`)

- `USTRUCT FEnemyDeathReplicationInfo`(`.h:105`): `Serial`(uint8), `HitDirection`
  (`FVector_NetQuantizeNormal`), `HitLocation`(`FVector_NetQuantize`), `HitBoneName`, `HitVelocity`
  (`FVector_NetQuantize`), `ImpulseMagnitude`.
- `UPROPERTY(Transient, ReplicatedUsing=OnRep_DeathInfo) DeathInfo`(`.h:892`) +
  `GetLifetimeReplicatedProps`(`.cpp:549`, 컴포넌트에 처음 추가됨 — 기존 Multicast들은 프로퍼티
  복제가 없었음).
- `NotifyDeathForReplication(HitDirection, HitLocation, HitBoneName, HitVelocity, ImpulseMagnitude)`
  (`.cpp:555`, BlueprintCallable): **서버는 값만 채운다.** 서버 자신의 래그돌/총 분리는 이 노드
  뒤에 이어지는 기존 BP 체인이 그대로 수행 — 서버 동작 불변.
- `OnRep_DeathInfo`(`.cpp:571`): 비권위 + `Serial != 0` + 미적용 Serial일 때만
  `ApplyDeathCosmeticsOnClient()`.
- `ApplyDeathCosmeticsOnClient`(`.cpp:583`): 위 BP 체인의 코스메틱 부분을 같은 순서·같은 핀 값
  (NoCollision / QueryAndPhysics / `"Hips"` / 강도 0·MaxForce 1e6)으로 재생. 임펄스는
  `SetTimerForNextTick → ApplyDeathImpulseOnClient`(`.cpp:669`) — BP의 `DelayUntilNextTick`과
  같은 이유(키네마틱→시뮬 전환 전에 넣은 임펄스는 Chaos가 버림). `Gun` 컴포넌트는 이름으로,
  라이플 액터는 **`GetAttachedActors()`**로 찾는다(`CurrentRifle`이 복제되지 않으므로; 적군에
  부착되는 액터는 라이플뿐). `WeaponMesh` 없으면 루트 프리미티브로 폴백. Destroy는 서버 것이
  복제돼 오므로 안 함.
- **값을 바꾸려면 BP 노드와 C++ 양쪽을 같이 바꿔야 한다** — `.cpp:594` 주석에 명시.

### 3-4. BP 수정 1곳 — `BP_Enemy_Base` EventGraph

`EventAnyDamage` 사망 분기, **`SetIsDead(true)`와 `SetAimPitch` 사이**에 노드 1개 삽입:

```
NotifyDeathForReplication(
    Target        = EnemyCombatComponent,
    HitDirection  = LastHitDirection,
    HitLocation   = LastHitLocation,
    HitBoneName   = LastHitBoneName,
    HitVelocity   = LastHitVelocity,
    ImpulseMagnitude = DeathImpulseMagnitude)
```

MCP `create_node`/`connect_pins`로 넣고 그래프를 다시 읽어 핀 연결 확인, 컴파일·저장. 기존 체인
노드는 하나도 안 건드림.

## 4. 리플레이 재생 월드에서 시나리오가 도는 문제 (리플레이 전용, 실기 클라이언트엔 없음)

### 4-1. 증상

리플레이 재생 로그에 `시나리오 스텝 발동: EnemyFleeToZone2/3, DroneChasePath`, `[Drone] 자율비행
시작`, 적군 `[EnemyPath] BeginNavPath`가 찍혔다 — **녹화된 스트림을 재생하는 월드에서 시나리오가
새로 시작**된 것.

### 4-2. 원인 — 엔진 로드 순서

`UScenarioStateSubsystem`은 `GetNetMode() == NM_Client`로만 자기를 막았다. 리플레이 재생에서는
엔진 `UEngine::LoadMap`이 `World->BeginPlay()`를 먼저 돌리고 **그 뒤** `PostLoadMapWithWorld`에서
DemoNetDriver를 붙인다. `UWorld::InternalGetNetMode()`(`World.cpp:9607`)는 데모 드라이버가 있어야
그 넷모드를 돌려주므로, BeginPlay 시점의 시나리오 타이머/스텝 시작 코드는 자기를 Standalone으로
보고 통과한다. 이후 스텝이 클라이언트 복제본의 `BeginMove/BeginFlee`를 불러
`ApplyGaitForDesiredSpeed`가 `GaitTopSpeed`/`IsSprinting`/`IsKneeling`을 로컬로 덮어썼고, §1-2의
델타 복제 특성상 서버 값이 다시 안 바뀌면 그대로 굳었다(리플레이에서만 남던 "사격→엄폐
미끄러짐"의 마지막 한 조각 — `replay_chronicle/2026-09-17_…` §1도 같이 볼 것).

실기 2-PC 클라이언트는 접속부터 NetDriver가 있으니 이 구멍이 없다.

### 4-3. 수정

- `UScenarioStateSubsystem::IsReplayPlayback()`(`UI/ScenarioStateSubsystem.cpp:1021`,
  `World->IsPlayingReplay()`) 신설. 매 틱 `TickScenarioSteps`(`:1033`), `DemoAutoStartScenario`
  (`:515`), `ApplyDemoRCWSAutoFire`(`:456`)에서 게이트. BeginPlay 시점의 한 번짜리 가드는 못 믿으니
  **매 틱** 막는다.
- 방어 겹: `UEnemyCombatComponent::BeginMove`(`:687`)/`BeginEngageAtCurrentZone`(`:729`)/
  `BeginFlee`(`:785`) 진입부에 `HasAuthority` 가드. 누가 부르든 비권위 복제본은 상태 전환·gait
  덮어쓰기를 하지 않는다.

## 5. 진단 도구 — `Enemy.ClientAnimDiag`

`EnemyCombatComponent.cpp:478-539`. cvar `Enemy.ClientAnimDiag 1`(기본 0)이면 **비권위 프로세스**
(클라이언트/리플레이)에서 0.1초마다:

```
[EnemyCombat][클라진단] <actor> role=N: 실제속도 GaitTop Sprint Kneel HasTarget Dead
  || ABP(<AnimClass>) 상태 Speed Dir Kneel || 슬롯 Fire Reload Default 몽타주 || MaxWalk Crouched MoveMode
```

`ai_combat/2026-09-11_enemy_slide_and_fire_gate.md` §7의 `[속도진단]`(서버 전용 틱 안, `#if 0`)의
클라이언트판. 이게 리플레이 잔여 미끄러짐의 범인(CDO AnimClass 불일치, `ABP(ABP_Enemy_kadex2_C)
… 슬롯 Fire=1.00`)을 찍어냈다. C++ 입력값·ABP 실제값·슬롯 가중치 3층을 한 줄에 놓는 형식은 09-11
교훈 그대로.

## 6. 상태

- 코드/BP 수정 완료, 사용자 빌드, **Chronicle 리플레이에서 검증됨("잘됨")**: 적군 걷기/뛰기,
  피격 흔들림, 래그돌+총 낙하, 재생 월드에서 시나리오 스텝 0건.
- **2-PC 실기(UGV PC 호스트 + SelfDefense 클라이언트) 재검증 대기** — §1~§3. 리플레이와 실기는
  같은 스트림이라 결과가 다를 이유는 없지만, 8월 작업 때도 "리플리케이션 이론상 맞음 → 실기에서
  BP 사각지대 발견"이 반복됐으니(`replication_audit.md` §8 2026-08-11 항목) 확인은 할 것.
- 낙하산(`SM_Parachute`) 숨김 복제 확인(`replay_chronicle/2026-09-16_…` §4-2)은 이번에 안 봄 —
  여전히 열려 있음.
- P4: `Soldiers/EnemyCombatComponent.h/.cpp`, `UI/ScenarioStateSubsystem.h/.cpp`,
  `BP_Enemy_Base.uasset`, `BP_ThirdPersonCharacter.uasset` 체크아웃/수정 상태(미제출).
  Chronicle 플러그인 09-16분과 `RtspAxisGate` 수정은 CL 486으로 제출됨.

## 관련 문서

- `replication_audit.md` §0-1/§8 — 8월 리플리케이션 작업(이 문서가 보완하는 대상).
- `replay_chronicle/2026-09-16_chronicle_replay_plugin.md` §4-2 — "리플레이 = 클라이언트 하나 더"
  가설의 출발점. `replay_chronicle/2026-09-17_replay_respawn_and_physics_proxy_fixes.md` — 같은
  조사에서 나온 리플레이 전용 수정(CDO AnimClass, 물리 프록시).
- `ai_combat/enemy_locomotion_animation_pipeline.md` — `GaitTopSpeed`/`IsSprinting`이 생긴 08-25 재설계.
- `ai_combat/enemy_hit_reaction_physics_system.md` — 피격 스프링(코스메틱 전용이라 비권위 적분이 안전한 근거).
- `ai_combat/2026-09-11_enemy_slide_and_fire_gate.md` — 진단 로그 형식의 원형, 리플레이 재발 추기.
