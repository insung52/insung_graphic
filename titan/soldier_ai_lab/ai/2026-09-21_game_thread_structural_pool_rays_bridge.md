# 게임 스레드 구조 묶음 — 투사체 풀 · 레이트레이스 채널 응답+캐시 · CMC/메시 데이터 · 박자 고정 · 캐릭터 BP 틱 → C++ (World Tick 18.3/20.3 → 11.3/14.0)

2026-09-21 / 완료(사용자 PIE 확인 "잘된다, 교전 중 50fps 후반") / 전편(`2026-09-21_game_thread_batch_cameras_abp_muzzle.md`) 13절의 **후보 순서 ①~⑥ 중 ①~⑤ 를 이 세션이 실행**한 기록. ① 투사체 풀 `USoldierProjectilePoolSubsystem`(발당 스폰 0.83 → 0, 누수 종결) · ② Engagement 레이 — 병사 무시를 `AddIgnoredActor` 35명 루프 대신 **Pawn 채널 응답 Ignore**(`SoldierQuery::BodiesAreNotWalls()`) + `IsShotBlockedByWorld` 결과 캐시(씬 쿼리 1,030~1,120 → 357/576회) · ③ CMC `bAlwaysCheckFloor=false`·`bEnablePhysicsInteraction=false` + **병사 메시 콜리전 QueryAndPhysics → QueryOnly**(문서가 QueryOnly 라 적었던 것이 실제론 아니었다 — 정정) · ⑤ Cover 후보 걸음·Sight 스캔을 **시간(0.033 s)에 고정** · ④ **캐릭터 BP EventTick 본문을 C++ `USoldierAIBridgeComponent::TickBridge` 로 노드 순서 그대로 이식**(아군 20 ReceiveTick 1.2 → 0.22). 오늘 누적 World Tick **22.6(원 29) → 11~14 ms**. ⑥([W112]·[W113])은 남음. 별건 발견: `AN_Reload` 노티파이의 `LogAbilitySystem: Error` 41회/PIE → [W114], `AC_PreCMCTick` 0.8 ms C++ 이관 후보 → [W115].

전편: **`ai/2026-09-21_game_thread_batch_cameras_abp_muzzle.md`**(계측법 · 카메라 · 11.2절 레이 소유자별 · 12절 투사체 누적 발견 · **13절 순서 ①~⑥ = 이 문서의 목차**) · `ai/2026-09-21_perf_instrumentation_and_cover_cost.md`(계측 · P182 규약). 원칙: P182(계측 → 이름 붙은 것만) · P183(결정 보존) · P186(URO 금지 — 건드리지 않음). 새 튜닝값 6(`LaneCacheMoveCm` · `LaneCacheSeconds` · `CandidateIntervalSeconds` · `ScanIntervalSeconds` · `MaxFlightDistanceCm` · cvar `SoldierLab.Projectile.PoolMax`). 작업 **[W102] · [W108] · [W109] · [W110] · [W111] 해결**, **[W114] · [W115]** 신설, [W112] · [W113] 남음.

> 신뢰도: **[A]** = 이 세션의 `stat dumpframe -ms=0.05` 로그 파싱(전편 1절 방법) + 프로젝트 소스 file:line + MCP 되읽기 + 사용자 육안(명중 · 거동 · fps). 조건: `L_SoldierScenario` 적 15 / 아군 20 **PIE**(로깅 on), 2026-09-21 17:10~18:10, 단계마다 교전 중 2프레임. 캡처 사이 교전 시점이 같지 않아 **±0.3 ms 는 잡음**(전편 규약 그대로). 메시 콜리전 QueryOnly 의 `FEndPhysics` 효과만 **[B]**(잡음에 묻혀 판정 보류).
>
> ⚠ ID: 이 문서는 **W114 · W115** 를 쓴다(W113 까지 전편·분대 세션이 썼다). 새 C · P 없음.

---

## 0. 한 장 요약

| 순서 | 무엇 | 전 → 후 (ms, 35명 교전 프레임) | 상태 |
|---|---|---|---|
| **①** | **투사체 풀** — `USoldierProjectilePoolSubsystem::Acquire` · BP `Shoot`/`PlayShotCosmetics` · 도탄 `Multicast_LaunchRicochet` 전부 풀 · `CollisionComponent` QueryOnly · `BP_RifleProjectile` 빈 EventTick 삭제 · `MaxFlightDistanceCm 60000` | `Shoot` 안 스폰 0.83/발 → **0** · `FEndPhysics` 1.03 → 0.62/0.46 · 투사체 액터 47 에서 **정지**(비행 15) · World Tick 18.3/20.3 → 17.1/14.3 | ✅ [W109] · A절 |
| **②** | **레이트레이스** — 병사 무시를 채널 응답으로(`SoldierQuery::BodiesAreNotWalls()`, Engagement 1 · Cover 5 · Field 2 사이트) + `IsShotBlockedByWorld` 8슬롯 캐시 | 씬 쿼리 1,030~1,120회 2.6~3.0 → **357/576회 0.96/1.42** · Engagement 레인 182~188회 0.46 → **23/35회 0.07/0.10** · Engagement 틱 0.67~0.82 → 0.22/0.17 | ✅ [W110] · B절 |
| **③** | **데이터** — CMC `bAlwaysCheckFloor=false` · `bEnablePhysicsInteraction=false` · **병사 메시 QueryAndPhysics → QueryOnly**(정정) | `FindFloor` 99~105회 → **44/27회** · 메시 콜리전 효과는 [B] | ✅ [W111] · C절 |
| **⑤** | **박자** — Cover 눈 있는 스윕 후보 걸음 `CandidateIntervalSeconds 0.033` · Sight `ScanIntervalSeconds 0.033` | 30 fps 거동·비용 동일, 60 fps 에서 트레이스 절반 | ✅ [W102] · D절 |
| **④** | **캐릭터 BP 틱 → C++** — `USoldierAIBridgeComponent::TickBridge` 가 `BP_SoldierCharacter` EventTick 본문을 그대로, BP 는 `Parent:Tick → TickBridge → Branch(ShouldRunBlueprintCopy)` | 아군 20 `ReceiveTick` 1.16~1.21 → **0.22** · World Tick 15.1/15.1 → **11.3/14.0** · 바인딩 경고 0 | ✅ [W108] · E절 |
| — | **누적** — 문서 기준선 22.6(원 29) → **11~14** | 카메라 −2.0 · 총구 −0.4/발+누수 · 투사체 풀 −0.8/발+누수 · 레이 −1.5~2 · 캐릭터 BP 틱 −1.6 · 기타 −0.5 | F절 |
| — | **남은 것** | Cover 1.2~2.6(점수) · CMC 1.3 · 메시 틱 1.1 · `AC_PreCMCTick` 0.8([W115]) · 틱 오버헤드 1.2(≈ 600 틱 함수, [W112]) · Sight 0.9 · 물리 0.6 · [W113] | F절 |
| — | **별건** — `AN_Reload` 노티파이가 어빌리티 컴포넌트 없는 병사에 `GameplayEvent.ReloadDone` → `LogAbilitySystem: Error` 41회/PIE | 비용은 로그뿐(기존) | [W114] · F절 |

---

## A. ① 투사체 풀 — [W109] 해결 [A]

전편 12절의 발견("풀 설계인데 BP 가 발마다 `SpawnActor` → 주차된 투사체 영구 누적")의 수정. 설계 A1~A3 그대로 + 거리 상한 하나.

### A.1 바꾼 것

| 어디 | 무엇 | 비고 |
|---|---|---|
| **신규 `Source/SoldierLab/Weapons/SoldierProjectilePool.h/.cpp`** | `USoldierProjectilePoolSubsystem`(월드 서브시스템) — **`Acquire(TSubclassOf<ASoldierProjectile>, AActor* Owner)`**(`.h:51` · `.cpp:25`): 클래스별 풀, **주차된(틱 꺼진) 투사체를 커서부터 재사용** → 없으면 상한까지 스폰해 풀에 넣음 → 상한이면 **RCWS 풀처럼 라운드로빈**으로 아직 나는 것을 재사용(클래스당 1회 경고 `.cpp:77`) | 상한 cvar **`SoldierLab.Projectile.PoolMax` 96**(`.cpp:11-14`). 96 = 소총 40정 버스트에서 동시 비행 수(≈ 발사율 × 비행시간)보다 넉넉(헤더 주석 `.h:1-12`) |
| `BP_AR4Rifle.Shoot` · `PlayShotCosmetics` | `SpawnActor + MakeTransform(+Cast)` → **`GetSoldierProjectilePoolSubsystem → Acquire(BP_RifleProjectile_C, OwningCharacter) → LaunchFrom`** | `LaunchFrom` 은 어느 쪽이든 같다(IMPLEMENTED 3.1 "풀 추가는 쏘는 쪽의 변경" 그대로) |
| `AI/SoldierEngagement.cpp:248-268 Multicast_LaunchRicochet_Implementation` | `SpawnActor` → **`Pool->Acquire`**(`.cpp:263`) | 튕길 때마다 1개씩 새던 것 해결 |
| `Weapons/SoldierProjectile.cpp:119-124` | `CollisionComponent` **`QueryAndPhysics → QueryOnly`** | `RCWSProjectile` 선례(메모리 — 키네마틱 물리바디가 PT 에서 UGV Hull 과 충돌하던 버그). 명중은 `ProjectileMovement` **이동 스윕의 블로킹 히트**라 `OnComponentHit` 그대로 — 사용자 육안 "명중 잘됨" |
| `BP_RifleProjectile` EventGraph | 빈 **`EventTick` · `BeginPlay` · Overlap 노드 삭제** | 발당 BP `ReceiveTick` 디스패치 제거(전편 12절: 30발 0.25 ms) |
| `Weapons/SoldierProjectile.h:206` | 신규 **`MaxFlightDistanceCm = 60000`**(600 m) | 사용자 제안 — 800 m/s 탄이 시간 상한 `MaxFlightTimeSeconds 5`(`.h:199`)만으로는 위로 빗맞힐 때 ≈ 4 km 비행. 교전 정지 95 m · 레벨 교전 ≤ 500 m. `Tick`(`.cpp:264-267`)에서 **시간 상한과 같은 자리에서 같은 방식으로 주차**, 도탄은 튕긴 지점(`SpawnLocation` 재설정)부터 |
| `Weapons/SoldierProjectile.h:1-12` | 머리 주석 갱신 — "풀은 `USoldierProjectilePoolSubsystem`, 쏘는 쪽이 `Acquire()` → `LaunchFrom()`" | |

### A.2 전후

| 항목 | 전(전편 11·12절) | 후 |
|---|---|---|
| `BP_AR4Rifle.Shoot` 안 투사체 스폰(`BeginDeferredActorSpawnFromClass` + `FinishSpawningActor`) | **0.83 ms/발**(0.65 + 0.18) | **0**(재사용 — 스폰 줄 소멸) |
| `FEndPhysics` | 1.03 | **0.62 / 0.46** |
| 투사체 액터 수 | 비행 21발에 `TracerTrailComponent` 80개, 계속 증가 | **47 에서 정지**(비행 15) |
| `[ProjectilePool]` 상한 경고 | — | **0** |
| World Tick | 18.3 / 20.3 | **17.1 / 14.3** |

`bReplicates=false`([W62])라 복제 함정 없음 — 리플리케이션 세션의 `PlayShotCosmetics` 도 클라에서 같은 풀을 쓴다(클라 월드의 서브시스템).

---

## B. ② 레이트레이스 — [W110] 해결 [A]

전편 11.2절: `SceneQueryTotal` 1,030~1,119회/프레임 2.6~3.0 ms 중 Engagement 182~188회 0.46 — 트레이스 자체보다 **호출마다 레지스트리 병사 35명을 `AddIgnoredActor` 로 목록 구성**하는 것이 비용이었다. Cover · Field 도 같은 루프를 갖고 있었다.

### B.1 병사 무시를 채널 응답으로

| 어디 | 무엇 |
|---|---|
| **신규 `Source/SoldierLab/AI/SoldierQuery.h`**(25줄) | **`SoldierQuery::BodiesAreNotWalls()`**(`:19-22`) — `FCollisionResponseParams` 에 **`ECC_Pawn → ECR_Ignore`**. 쿼리의 오브젝트 타입 응답이라 ignore 목록이 필요 없다 |
| 전제 [A] | 병사는 **캡슐·메시 모두 ECC_Pawn** 이고, 이 레벨들에서 Pawn 오브젝트 타입은 병사뿐 — MCP 확인: 트럭 `BodyMesh` ECC_Vehicle · `WindowMesh` WorldDynamic, UGV `VehicleMesh` ECC_Vehicle, 드론 `CollisionBox` ECC_PhysicsBody, 총 메시 2종 NoCollision. **Pawn 타입을 쓰는 새 액터가 들어오면 이 전제가 깨진다**(헤더 주석 `:6`) |
| 제거한 `AddIgnoredActor` 35명 루프 | **Engagement 1**(`IsShotBlockedByWorld` `SoldierEngagement.cpp:416`) · **Cover 5**(`SoldierCover.cpp:616` · `:1391` · `:1465` · `:1566` · `:1859` — `IgnoreBodies`(`:249-255`)는 **주석 남기고 빈 함수**, 호출 자리 `:591`·`:1357`·`:1535`·`:1854` 그대로) · **Field 2**(`SoldierSituationField.cpp:1896` · `:2072`) |

### B.2 `IsShotBlockedByWorld` 결과 캐시

| 어디 | 무엇 |
|---|---|
| `AI/SoldierEngagement.h:887-897` | 새 튜닝 **`LaneCacheMoveCm 15`** · **`LaneCacheSeconds 0.15`** — 총구·표적이 둘 다 15 cm 안이고 0.15 s 안이면 지난 답을 돌려준다(P180: 트레이스는 정보를 새로 만들 때만) |
| `AI/SoldierEngagement.h:980-989` | 8슬롯 링 **`FLaneAnswer`**(`mutable` — `IsShotBlockedByWorld` 가 const), 오래된 것부터 교체 |
| `AI/SoldierEngagement.cpp:370-424` | 최신 슬롯부터 탐색(`:380-387` — 틱이 방금 물은 것을 다시 묻는 경향) → 미스면 트레이스(`:416`) → 슬롯 기록(`:424`) |
| 호출자 | 실제 총구(`:1177`) · 앉은 총구(`:1257`) · **`PlanAperture` 옵션마다(`:516`)** — 셋 다 같은 함수라 **PlanAperture 옵션 트레이스도 자동으로 캐시를 탄다**(전편의 "PlanAperture 저빈도" 는 별도 박자 없이 이걸로 흡수) |

### B.3 전후 (같은 레벨, 교전 2프레임)

| 항목 | 전 | 후 |
|---|---|---|
| 씬 쿼리 총(`SceneQueryTotal`) | 1,030~1,120회 · 2.6~3.0 ms | **357 / 576회 · 0.96 / 1.42 ms** |
| Engagement 레인 트레이스 | 182~188회 · 0.46 | **23 / 35회 · 0.07 / 0.10** |
| Engagement 틱 | 0.67~0.82 | **0.22 / 0.17** |
| Cover 후보 트레이스 단가 | 2.4 µs | **1.9 µs**(ignore 목록 35개가 빠진 몫) |

`[Engage]` 판정은 불변(캐시 창 0.15 s 는 조준 정착 0.4 s · 버스트 0.5 s 보다 짧다). 거동 차이 보고 없음.

---

## C. ③ 데이터 — [W111] 해결 + 메시 콜리전 정정 [A]

코드 아님 — 부모 CDO `BP_SoldierCharacter` + 자식 2 CDO(`BP_Soldier_Friendly` · `_Hostile`) + `L_SoldierScenario` 인스턴스 35 에 MCP 로 적용·저장(CDO 쓰기는 자식/인스턴스에 전파 안 됨 — 메모리).

| 무엇 | 값 | 이유 · 결과 |
|---|---|---|
| CMC `bAlwaysCheckFloor` | true → **false** | 정지 병사의 바닥 스윕 생략 — `Char FindFloor` **99~105회 → 44/27회**/프레임. 경사·계단·웅크림 거동 이상 보고 없음 |
| CMC `bEnablePhysicsInteraction` | true → **false** | 병사가 물리 오브젝트를 밀 일이 없다(사용자 승인 후보) |
| **병사 메시(`CharacterMesh0`) 콜리전** | **`QueryAndPhysics` → `QueryOnly`** | ★ **정정** — 전편 문서(`perf_instrumentation_and_cover_cost.md` 7절 [W98] ② · `IMPLEMENTED.md`)는 "살아 있는 병사의 피직스 바디는 QueryOnly" 라 적었으나 **실제 CDO 는 GASP 커스텀 프로파일 `QueryAndPhysics` 였다**. 사망 래그돌은 `AI/SoldierHealth.cpp:554-572 StartRagdoll` 이 **`QueryAndPhysics` 로 되돌리므로**(`:572`) 안전, 피격 부위 트레이스(`ResolveBoneByTrace`)는 Query. **`FEndPhysics` 효과는 잡음에 묻혀 판정 보류 [B]**(A절 투사체 QueryOnly 와 같은 프레임에 들어가 분리 불가) |

> 전편 2.2절의 "QueryOnly **라서** 본을 따라가야 한다"(키네마틱 본 스킵 불가)는 결론은 그대로다 — 바디가 Query 를 받는 한 포즈를 따라가야 한다. 이 정정으로 바뀌는 것은 "원래 QueryOnly 였다" 는 전제뿐이다.

---

## D. ⑤ 박자 — [W102] 해결 [A]

전편 [W102] ②("눈 있는 스윕도 후보 하나를 2틱에") 와 ①("보이는 표적 격틱")의 실행. 프레임이 아니라 **시간**에 고정했다 — 30 fps 에서 지금과 같은 거동·비용, 60 fps 에서 트레이스 절반, 값을 올리면 결정 지연 ↔ 비용 트레이드(값 하나로 조절).

| 어디 | 무엇 |
|---|---|
| `AI/SoldierCover.h:582` · `:1015-1016` · `.cpp:2334-2341` | **`CandidateIntervalSeconds 0.033`** + `LastCandidateStepSeconds` — 눈 있는 스윕의 후보 걸음을 시계로. 눈 0 스윕은 `CalmCandidatesPerTick 2`(전편) 그대로 |
| `AI/SoldierSight.h:87` · `:174-175` · `.cpp:233-240` | **`ScanIntervalSeconds 0.033`** + `LastScanSeconds` — 같은 원리 |
| 생략 | Score 캐시 — 눈 세대마다 바뀌어 캐시할 것이 없다. [W102] ③(Health/Comms `TickInterval`)은 전편 6절에서 이미 완료 |

30 fps PIE 에서는 전후 ms 가 같다(설계상). 거동 판정 [C-162](박자 값의 결정 지연 — 눈에 띄면) 그대로.

---

## E. ④ 캐릭터 BP 틱 → C++ — [W108] 해결 [A]

전편 11.1절: `AnimBP` 캐시 뒤에도 아군 20명 `ReceiveTick` 1.21 변화 없음 → 본문 자체를 옮겨야 했다. 리플리케이션 세션 작업 완료로 조율 조건이 풀렸다(2PC 검증은 내일 사용자).

### E.1 `USoldierAIBridgeComponent`

| 어디 | 무엇 |
|---|---|
| **신규 `Source/SoldierLab/Pose/SoldierAIBridgeComponent.h/.cpp`**(112 + 506줄) | **`TickBridge(DeltaSeconds)`**(`.h:43` · `.cpp:251`) = `BP_SoldierCharacter` EventTick 본문을 **노드 순서 · Kismet 산술 그대로**(`RLerp` · `ComposeRotators` · `NormalizedDeltaRotator` · `MapRangeClamped` · `InRange` 그대로 호출) 이식: 조준 보정 회전 수학(2.5c) → 린 램프 → 입력 상태 구조체 Sprint/Aim **직접 쓰기**(`UpdateInputState_Server` 본문 = `SetCharacterInputState` 라 권한에선 동치) → `AOActive`/`AIPoseDriven`/AI 다리 5개 → `UpdateBodyYawRate`/`UpdateBlindFire`/`UpdateStance` → `WantsToFire`/`Reload` → Rifle `Shoot`/`StartReload` 이벤트 |
| 변수 소유권 | **변수 47개(캐릭터 34 · 부모 4 · ABP 7)는 BP 소유 그대로**, `FProperty` 로 읽고 쓴다 — 포즈 스무더 · 게이트브리지 · 리플리케이션 분기가 같은 변수를 쓰므로 소유권 불변(P170 과 같은 리플렉션 경로) |
| 바인딩 | BeginPlay 에서 이름으로 바인딩, 실패 시 경고 + BP 원본 경로로(`.cpp:109` · `:170` · `:195` · `:264`) — **`ShouldRunBlueprintCopy()`**(`.h:55` · `.cpp:232`) |
| A/B | cvar **`SoldierLab.AIBridge.Native`**(`.cpp:91-93`) 0 이면 BP 원본 본문이 돈다(P182 — 전후 비교용) |

### E.2 BP 쪽

`BP_SoldierCharacter`: 컴포넌트 **`AC_SoldierAIBridge`** 추가. EventTick = **`Parent:Tick → TickBridge(DeltaSeconds) → Branch(ShouldRunBlueprintCopy) → (true) 옛 본문`**. 리플리케이션 세션의 `HasAuthority` 분기 · 바운드 이벤트 2개는 **옛 본문 안에 그대로**(A/B 시 함께 돈다). 자식 `BP_Soldier_Friendly`/`_Hostile` 은 상속.

### E.3 전후

| 항목 | 전 | 후 |
|---|---|---|
| 아군 20명 `BP_SoldierCharacter_C ReceiveTick` | 1.16~1.21 | **0.22**(적 15 는 `-ms=0.05` 목록 밖으로) |
| World Tick(직전 단계 뒤) | 15.1 / 15.1 | **11.3 / 14.0** |
| `SoldierAIBridge:` 바인딩 경고 | — | **0** |
| 사용자 | — | "잘된다, 교전 중 50fps 후반" |

`AC_PreCMCTick`(GASP 원본, 0.8 ms)은 이 이관에 안 들어갔다 → [W115].

---

## F. 누적과 남은 것

### F.1 오늘 누적 (문서 기준선 22.6 ms — 원 29)

| 단계 | 문서 | −ms |
|---|---|---|
| 카메라 틱 off | 전편 4.1 | −2.0 |
| 총구 Niagara 상주 | 전편 4.5 · 11.1 | −0.4/발 + 누수 |
| 투사체 풀 | A절 | −0.8/발 + 누수 |
| 레이 채널 응답 + 캐시 | B절 | −1.5~2 |
| 캐릭터 BP 틱 C++ | E절 | −1.6 |
| 기타(HeadAim · CMC 데이터 · 박자 …) | 전편 6 · C · D절 | −0.5 |
| **World Tick** | | **22.6 → 11~14**(PIE, 로깅 on — Standalone 은 PIE 전용 ≈ 14 가 더 빠진다, 전편 3.3절) |

### F.2 남은 상위 (교전 프레임, 35명)

| 덩어리 | ms | 다음 |
|---|---|---|
| Cover | 1.2~2.6 | 점수 계산 몫 — 캐시 불가(D절), 후보 수·`FanRays` 쪽 |
| CMC | 1.3 | — |
| 메시 틱(애니 GT) | 1.1 | `Update_PropertiesFromCharacter` 프로퍼티 액세스화([W98] ① 잔여, ≤ 1) |
| **`AC_PreCMCTick`**(GASP) | 0.8 | **C++ 이관 후보 → [W115]** |
| 틱 오버헤드(틱 함수 ≈ 600, ← 850) | 1.2 | **[W112]** 컴포넌트 틱 통합 |
| Sight | 0.9 | — |
| 물리 | 0.6 | — |
| 총 액터 메시 | 0.07 × 35 | [W113] |

### F.3 별건 발견 → [W114]

재장전 몽타주의 GASP 노티파이 **`AN_Reload`**(`Content/Characters/Heroes/Abilities/AN_Reload`)가 **어빌리티 컴포넌트 없는 병사**에 `GameplayEvent.ReloadDone` 을 보내 `LogAbilitySystem: Error` **41회/PIE**. 기존 문제고 비용은 로그뿐 — 노티파이 제거(SoldierLab 몽타주 복제본) 또는 병사에 무해한 수신자. 새 [W114].

### F.4 조율

리플리케이션 세션 작업 완료 — 2PC 검증은 내일(사용자). 이 세션이 만진 `SoldierEngagement.cpp`(도탄 · 레인 캐시) · `SoldierProjectile.*` · `BP_AR4Rifle` · `BP_SoldierCharacter` Tick 은 저쪽 편집 위에 얹었다(옛 본문 보존 — E.2절).

---

## 9. 미해결 · 정정 대상

| ID | 무엇 |
|---|---|
| ~~[W102]~~ | ✅ D절 |
| ~~[W108]~~ | ✅ E절 |
| ~~[W109]~~ | ✅ A절 |
| ~~[W110]~~ | ✅ B절 |
| ~~[W111]~~ | ✅ C절 |
| [W112] | 컴포넌트 틱 통합 — 틱 함수 ≈ 600, 오버헤드 1.2(F.2) |
| [W113] | 총 액터 메시 제거 — 남음 |
| **[W114]** | `AN_Reload` 노티파이 → `LogAbilitySystem: Error` 41회/PIE(F.3) |
| **[W115]** | `AC_PreCMCTick` 0.8 ms C++ 이관(E.3 · F.2) |
| [C-165] | URO 크래시 원인 — 그대로(이 세션 무관) |
| 정정 대상 | `IMPLEMENTED.md` 3.2절(메시 콜리전 QueryOnly 는 09-21 부터 사실 — 그 전엔 QueryAndPhysics) · 5.2 [W107] 잔여 문구 |
