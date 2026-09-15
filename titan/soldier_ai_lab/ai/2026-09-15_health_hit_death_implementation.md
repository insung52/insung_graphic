# 병사 체력 · 피격 반응 · 사망 — 구현 기록

2026-09-15 / **완료** / C++ `USoldierHealthComponent` 하나가 데미지 수신·체력·피격 몽타주·사망 몽타주→래그돌·AI 정지를 전부 맡는다. ABP 에 `AdditiveHitReact` 슬롯 경로 3노드, `BP_SoldierCharacter` 에 컴포넌트 + 총구 보정 게이트 AND. 아군은 `bInvincible` 체크박스로 불사. 사용자 PIE 확인 "잘됨" — **수치는 하나도 안 쟀다**(6절 [C]).

관련 항목: ~~[W18]~~ 해결 · 신규 [C-110]~[C-118] [W60]~[W63] [Q46] [Q47] [R8] / 관련 문서: **`2026-09-14_hit_death_health_recommendation.md`**(전날 조사·추천 — 구조 파악과 후보 비교는 그쪽이 원본이다), `IMPLEMENTED.md` 0 · 2.4 · 3 · 4 · 5.2절, `assets/2026-09-14_design_team_animation_handoff.md` 2.8절, `weapons/2026-09-12_projectile_port.md`, titan 루트 기준 `../ai_combat/enemy_hit_reaction_physics_system.md`(임펄스 한 틱 지연의 출처)

> 코드 인용은 `Source/SoldierLab/AI/SoldierHealth.h` / `.cpp` 의 줄 번호다(2026-09-15 저녁 판). 헤더는 `.h:NNN`, 본문은 `.cpp:NNN`.

---

## 0. 한 장 요약

| 질문 | 답 | 신뢰도 |
|---|---|---|
| 무엇이 생겼나 | `USoldierHealthComponent`(`AI/SoldierHealth.h/.cpp`, 약 400+780 줄) 한 클래스. 체력 · 표준 엔진 데미지 수신 · 부위 배율 · 무적 · 피격 애디티브 몽타주 · 사망 몽타주 → 래그돌 · 등록부/AI/틱 정지 · 리플리케이션 · 디버그 오버레이 | [A] 코드 |
| 투사체는 고쳤나 | **한 줄도 안 고쳤다.** `ApplyPointDamage(34, …)` 표준 경로를 컴포넌트가 `OnTakePointDamage` 로 받는다(`.cpp:163-167`) | [A] |
| 추천안(C-2) 그대로인가 | 골격은 그대로 — 피격 = HitReact 애디티브 몽타주, 사망 = Death 몽타주 → 끝나기 0.1 s 전 래그돌. **달라진 것 셋**: ① 연출을 BP 가 아니라 C++ 이 전부 한다 ② 스켈레톤 슬롯 그룹을 **안 갈랐다**(`bStopAllMontages=false` 로 대신) ③ `Ragdoll_Start` 를 부르지 않고 같은 일을 C++ 에서 직접 한다. 4절 | [A] |
| ABP 는 | `ApplyAdditive_1` 과 `Slot 'DefaultSlot'` 사이에 `AdditiveIdentityPose_7 → Slot 'AdditiveHitReact'(AnimGraphNode_Slot_4) → ApplyAdditive_0(Alpha 핀 1.0)` 3노드. 재장전 애디티브 경로와 같은 모양. 사망은 기존 `DefaultSlot` 이라 ABP 변경 없음 | [A] MCP 실측·컴파일·저장 |
| BP 는 | `BP_SoldierCharacter` 에 `AC_SoldierHealth`. Tick 의 총구 보정 게이트(2.5c) 가 `AND(기존 InRange 0..2, NOT IsHitReacting)` 로 바뀜 — 소비자 셋(누적 `SelectRotator_42` · `AimGate` HUD 행 · 보조) 전부 AND 출력을 읽는다 | [A] |
| 아군은 죽나 | **안 죽는다** — 사용자 결정. `BP_Soldier_Friendly` 의 상속 컴포넌트 `AC_SoldierHealth` 에 **`Invincible (무적)` 체크**(에디터 수동). 맞으면 움찔은 하고 체력만 안 준다 | [A] 사용자 |
| 검증은 | 사용자 PIE "잘됨". 적군이 맞고 움찔하고 죽어 쓰러지고 시체가 남는다. **숫자 0개** — 본 이름 None 빈도 · 루트모션 · 정착 프레임 · 임펄스 · 45구 비용 전부 [C] | [A] 거동 / [C] 수치 |
| 빌드 상태 | 마지막 빌드는 `HasAnyFlags(RF_ClassDefaultObject)` 가드가 **있던** 생성자다. 가드를 뺀 현재 소스는 **다음 빌드 대기** — 그때까지 몽타주 배열은 MCP 로 템플릿에 직접 써 둔 값으로 돈다(5.1절) | [A] |

---

## 1. 무엇이 생겼나 — `USoldierHealthComponent` API

`UCLASS(ClassGroup=(SoldierLab), meta=(BlueprintSpawnableComponent))`, `SetIsReplicatedByDefault(true)`(`.cpp:75`). 카테고리는 전부 `SoldierLab|Health` 아래. **전부 `EditAnywhere`** 라 자식 BP · 배치 인스턴스에서 덮는다(P6).

### 1.1 프로퍼티와 기본값

| 프로퍼티 | 기본 | 왜 이 값인가 | 줄 |
|---|---|---|---|
| `MaxHealth` | 100 | 소총 34 × 3발. titan 도 34/100 | `.h:145` |
| **`bInvincible`** (`Invincible (무적)`) | false | 체력만 안 줄고 나머지(피격 반응·`LastHit`·오버레이 `[INV]`)는 그대로. **시나리오에서 아군은 죽으면 안 된다**는 요구를 코드 변경 없이 BP 체크박스로 | `.h:153` · `.cpp:212-217` |
| `BodyParts` | head 2.5 · neck_01 2.0 · clavicle_l/r 0.6 · thigh_l/r 0.75 · pelvis 1.0 | **첫 매치 우선** + `BoneIsChildOf` 로 하위 본까지(`.cpp:266-276`). head·neck 를 pelvis(전신) 앞에 둬야 이긴다. 값은 추천 문서 3절의 예시값을 그대로 — **감이다** | `.cpp:85-91` |
| `UnknownBoneScale` | 1 | 본 이름이 끝내 None 이면 몸통 취급 | `.h:165` |
| `FriendlyFireScale` | 1.0 | 같은 진영(`USoldierIdentityComponent::Faction` 비교, `.cpp:280-304`) 사격 배율. 0 이면 아군 사격 무효. [Q47] 결정 전 값 | `.h:169` |
| `bResolveBoneByTrace` | true | 본 None 이면 메시에 `LineTraceComponent` ±60 cm(`BoneTraceHalfLengthCm`, `.cpp:44`) — 추천 1a-(나) 안. 캡슐 콜리전은 안 건드린다 | `.cpp:306-326` |
| `LightDamageBelow` / `HeavyDamageFrom` | 25 / 60 | 배율 적용 **후** 데미지로 판정. 34 기준: 머리 85·목 68 → Heavy, 몸통 34·다리 25.5 → Medium, 팔 20.4 → Light | `.cpp:347-358` |
| `HitReactFront/Back/Left/Right` | Lyra 13개 (표 1.2) | `FSoldierHitReactMontages{Light,Medium,Heavy}` 배열 3개씩. 빈 칸은 한 단계 가벼운 쪽으로 폴백 | `.cpp:115-127` |
| `HitReactSlot` | `AdditiveHitReact` | `IsHitReacting()` 이 `AnimInstance->IsSlotActive(HitReactSlot)` 로 묻는다 | `.cpp:729-733` |
| `HitReactMinIntervalSeconds` | 0.2 | 연사 중 매 발 몽타주가 겹쳐 시작되는 것을 막는 최소 간격. 감 | `.cpp:501-504` |
| `DeathFront/Back/Left/Right` | Lyra 6개 (표 1.2) | 방향별 배열, 비면 Front 로 폴백 | `.cpp:129-134` · `:386-401` |
| `bPlayDeathMontage` | true | false 면 몽타주 없이 즉시 래그돌 = 추천 문서의 차선 (D) | `.cpp:556-567` |
| `RagdollLeadSeconds` | 0.1 | 몽타주 길이 − 0.1 s 에 래그돌. Death 몽타주가 `blendOut 0` + 세그먼트 조기 종료라 **끝까지 기다리면 MM 포즈 한 프레임**이 보인다(추천 1c). 0.1 은 감 → [C-117] | `.cpp:561-562` |
| `RagdollRootBone` | pelvis | GASP `Ragdoll_Start` 와 동일 | `.cpp:589` |
| `bInheritVelocity` | true | 죽는 순간 `GetVelocity()` 를 래그돌 전체 선속도로(`.cpp:536`, `:593`) — titan `LastVelocity` 와 같은 뜻 | |
| `DeathImpulseMagnitude` | 1500 | **titan 리그 값 그대로. 마네킹에서 미측정** → [C-118] | `.h:250` |
| `DeathSuppressionImpulse` / `RadiusCm` | 0.4 / 1500 | 사망자 반경 15 m 의 **같은 진영**에 `0.4 × (1 − d/r)` 제압 임펄스. `USoldierSuppressionComponent::ApplyImpulse` 의 "분대원 전사" 예약 용도 | `.cpp:418-445` |
| `CorpseFreezeAfterSeconds` | 8 | 래그돌 정착 뒤 `bPauseAnims` + 메시 틱 off. 바디는 sleep 상태로 콜리전만 남는다. 8 s 는 감 → [C-116] | `.cpp:626-635` |
| `DestroyAfterSeconds` | 0 | 0 = 시체를 남긴다([Q46] 결정 전 기본) | `.cpp:450-456` |

**표 1.2 — 기본 몽타주(생성자가 `ConstructorHelpers::FObjectFinder` 로 로드, `/Game/SoldierLab/Animations/Actions/`)** [A] `.cpp:100-135`

```
HitReactFront   Light  AM_MM_HitReact_Front_Lgt_01 · _02 · _03 · _04
                Medium AM_MM_HitReact_Front_Med_01 · _02
                Heavy  AM_MM_HitReact_Front_Hvy_01
HitReactBack    Light  Back_Lgt_01     Medium Back_Med_01      (Heavy 없음 → Medium 폴백)
HitReactLeft    Light  Left_Lgt_01     Medium Left_Med_01
HitReactRight   Light  Right_Lgt_01    Medium Right_Med_01
DeathFront      AM_MM_Death_Front_01 · _02 · _03
DeathBack/Left/Right   AM_MM_Death_Back_01 / Left_01 / Right_01
```

### 1.2 함수 · 델리게이트 · 콘솔

| 종류 | 이름 | 무엇 | 줄 |
|---|---|---|---|
| BlueprintPure | `GetHealth` · `GetHealthFraction` · `IsAlive` · `IsDead` · `GetLastHit` | 그대로 | `.h:273-290` |
| BlueprintPure | **`IsHitReacting()`** | 피격 몽타주가 재생 중인가 = 총구 보정 적분기가 **멈춰야 하는** 순간(P35·P37 — 액추에이터 결합 여부로 건다) | `.cpp:729-733` |
| BlueprintCallable | `Kill(Instigator)` | 무적 무시 즉사. 스크립트/테스트용. 서버 전용 | `.cpp:238-256` |
| BlueprintCallable | `SetInvincible(bool)` | | `.h:299` |
| BlueprintAssignable | `OnDamaged(Damage, Direction, BoneName, Instigator)` | 비치명 피격마다, 모든 프로세스에서(서버는 자기 경로, 클라이언트는 OnRep). ⚠ 5.5절 — **치명타에는 안 온다** | `.cpp:492` |
| BlueprintAssignable | `OnDeath(Direction, Instigator)` | 사망 연출 시작 시 한 번, 모든 프로세스 | `.cpp:548` |
| 콘솔 | `SoldierLab.Debug.Health 1` | 머리 위 235 cm 에 `HP 66/100 [INV]` + 둘째 줄 `Front 34 spine_02`(마지막 피격 방향·데미지·본). 죽으면 회색 `DEAD`. `SoldierDebug::ShouldDrawFor` 규약 | `.cpp:753-782` |
| 콘솔 | `SoldierLab.Invincible 1` | 전원 무적(체력만) | `.cpp:34-38` |

**리플리케이트**: `Health` · `bDead`(`OnRep_Dead`) · `LastHit`(`OnRep_LastHit`) — `FSoldierHitInfo{Direction, Location, BoneName, Damage, bLethal, Serial}`(`.h:40-65`). `Serial` 이 매 피격 증가해 같은 값의 연속 피격도 OnRep 이 뜬다. `Instigator` 는 복제하지 않는다(클라이언트에선 null, `.h:387-388`).

---

## 2. 배선

### 2.1 ABP `SoldierCharacter_ABP` — 슬롯 경로 3노드 [A]

```
… LayeredBoneBlend_1 → ApplyAdditive_1 ← IdentityPose_3 → Slot 'UpperBodyAdditive'     (재장전 애디티브, 기존)
                            ↓
   ★ 신규         ApplyAdditive_0 ← AdditiveIdentityPose_7 → Slot 'AdditiveHitReact' (AnimGraphNode_Slot_4)
                       (Alpha 핀 1.0)
                            ↓
                   Slot 'DefaultSlot' (AnimGraphNode_Slot_0)  → OffsetRootBone_0 → …    (기존 — 사망 몽타주가 여기서 재생)
```

- HitReact 13 이 전부 **`AAT_LocalSpaceBase`** 라 `ApplyAdditive` 다. 사격(`MM_Rifle_Fire`, `AAT_RotationOffsetMeshSpace`)이 쓰는 `ApplyMeshSpaceAdditive` 에는 못 꽂는다(추천 1c).
- Alpha 는 **핀** 1.0 고정 — `IsSlotActive` 가 게이트이므로 알파로 켜고 끌 이유가 없다. 핀이 `node.alpha` 를 이긴다(P25).
- 사망 몽타주는 기존 `DefaultSlot` 이라 ABP 변경 없음. `DefaultSlot` 은 `ApplyAdditive` 뒤 · `OffsetRootBone_0` 앞 — 전신 몽타주가 가중치 1 로 들어오면 상류(MM·린 뱅킹·AO·BF·상체 재장전)를 전부 덮고 하류(`OffsetRootBone` · 린 `ModifyBone` · `FootPlacement` · `PelvisDrop` · `LegIK` · 머리 추종)는 그대로 돈다 → 그래서 `ZeroPoseAxes`(3.3절)가 필요했고, `FootPlacement`/`LegIK` 와의 충돌은 [C-113].
- 컴파일 + 저장 완료. 스켈레톤 슬롯 그룹은 **안 건드렸다**(6개 슬롯 전부 `DefaultGroup`, `Partials` 비어 있음 — 4.2절).

### 2.2 `BP_SoldierCharacter` — 컴포넌트 + 총구 보정 게이트 [A]

- **`AC_SoldierHealth`** 추가 — MCP `ActorTools.add_component`(owner = 블루프린트 에셋, SCS 컴포넌트로 들어감). 템플릿 이름 `BP_SoldierCharacter_C:AC_SoldierHealth_GEN_VARIABLE`.
- 템플릿의 몽타주 배열 13+6 은 `ObjectTools.set_properties` 로 **직접 써 넣었다**(5.1절의 이유). `BodyParts` 는 C++ CDO 값이 그대로 왔다.
- **Tick 의 2.5c 게이트**: 기존 `K2Node_CallFunction_56`(= `InRange(카메라 각속도, 0..2)`)의 출력이 세 곳으로 가던 것을,

```
GetACSoldierHealth (Variables|Default|GetACSoldierHealth)
   → IsHitReacting (SoldierLab|Health|IsHitReacting, declaring_class /Script/SoldierLab.SoldierHealthComponent)
   → NOTBoolean (Math|Boolean|NOTBoolean)
InRange_56 ─┐
            ├→ ANDBoolean (Math|Boolean|ANDBoolean) ─→ SelectRotator_42.bPickA     (누적 갱신 / 유지)
NOT ────────┘                                       ├→ CallFunction_49          (AimGate HUD 행)
                                                    └→ CallFunction_4           (보조 소비자)
```

  세 소비자가 **전부** AND 출력을 읽으므로 화면의 `AimGate` 가 실제 게이트와 같다([W6] 의 "화면 GATE 가 죽은 게이트를 찍는다" 함정을 여기서는 안 만들었다). 게이트 거짓 = 누적값 **유지**(0 이 아니라 hold) — 2.5c 의 기존 분기 그대로.
- 컴파일 에러 0. 사용자가 P4V 체크아웃 후 저장(10:24). `BP_Soldier_Friendly` / `_Hostile` 재컴파일.
- **`BP_Soldier_Friendly`**: 상속된 `AC_SoldierHealth` 의 `Invincible (무적)` 을 사용자가 에디터에서 체크. 도구로는 못 쓴다(5.2절).

### 2.3 투사체 · 등록부 · 제압 — 변경 없음 [A]

`ASoldierProjectile::OnHit` 의 `ApplyPointDamage(DamagePerHit 34, ImpactDirection, Hit, InstigatorController, this, DamageTypeClass)` 그대로. 등록부 `USoldierRegistrySubsystem::Unregister` 와 `USoldierSuppressionComponent::ApplyImpulse` 는 **이미 있던 함수**를 부른다.

---

## 3. 데이터 흐름

### 3.1 피격 (비치명) [A]

```
서버   투사체 OnHit → ApplyPointDamage → Owner.OnTakePointDamage
       → HandlePointDamage(.cpp:172)        ShotFromDirection = 탄의 *진행* 방향 (투사체가 그렇게 넘긴다)
       → ApplyHit(.cpp:191)                 HasAuthority · !bDead · RawDamage > 0
            본 None ∧ bResolveBoneByTrace → ResolveBoneByTrace (메시 LineTraceComponent ±60 cm)
            Damage = Raw × BodyPartScaleFor(본) × FactionScaleFor(사수)
            !무적 → Health −= Damage
            LastHit = {Direction, Location, Bone, Damage, bLethal, Serial++}
            → PlayHitReaction(LastHit)      서버는 OnRep 이 안 돌므로 직접
클라   LastHit 복제 → OnRep_LastHit(.cpp:461)  Serial ≠ PlayedSerial → PlayHitReaction
공통   PlayHitReaction(.cpp:487)
            OnDamaged.Broadcast
            !bDead · 최소 간격 0.2 s
            몽타주 = PickHitReact(DirectionFor(Direction), StrengthFor(Damage))
            Montage_Play(…, bStopAllMontages = false)         ← 재장전·사격 몽타주 안 끊김
```

**방향**(`.cpp:330-345`): `FromDir = ActorTransform⁻¹(−TravelDirection)`. `|x| ≥ |y|` 면 x 부호로 Front/Back, 아니면 y 부호로 Right/Left. 정면에서 날아온 탄(진행 −X) → Front.

### 3.2 사망 [A]

```
서버   ApplyHit … Health ≤ 0 ∧ !무적 → LastHit.bLethal = true → Die(.cpp:405)
            bDead = true
            반경 1500 cm 같은 진영 → Suppression.ApplyImpulse(0.4 × (1 − d/r))
            → PlayDeath(LastHit)            서버도 뷰어다
            DestroyAfterSeconds > 0 이면 타이머
클라   OnRep_LastHit(bLethal) → PlayDeath   /   OnRep_Dead → PlayDeath   (늦게 합류한 클라이언트도 시체를 본다; 멱등)
공통   PlayDeath(.cpp:519)   bDeathPlayed 래치
            DeathVelocity = GetVelocity()                       ← 이동을 끄기 *전에*
            Registry.Unregister(Identity)                      ← 소비자 8곳(Comms·Cover·Engagement·Perception×2·Sight·Observer·Projectile) 동시 정리
            StopDriving()  (3.3)
            OnDeath.Broadcast
            ZeroPoseAxes(ABP)  (3.3)
            bPlayDeathMontage → Montage_Play(PickDeath(방향), bStopAllMontages = true)   ← 재장전은 시체 위에서 끝낼 이유가 없다
                                 타이머 max(길이 − RagdollLeadSeconds, 0.01) → StartRagdoll
            아니면 즉시 StartRagdoll
       StartRagdoll(.cpp:570)                                   = GASP Ragdoll_Start 와 같은 일
            캡슐 NoCollision · 메시 ObjectType PhysicsBody · QueryAndPhysics · SetAllBodiesBelowSimulatePhysics(pelvis)
            bInheritVelocity → SetAllPhysicsLinearVelocity(DeathVelocity)
            SetTimerForNextTick → ApplyDeathImpulse            ← Chaos 는 같은 프레임 임펄스를 버린다(titan 실측)
            CorpseFreezeAfterSeconds → FreezeCorpse
       ApplyDeathImpulse(.cpp:609)   AddImpulseAtLocation(Direction × 1500, LastHit.Location, 본 또는 FindClosestBone)
       FreezeCorpse(.cpp:626)        bPauseAnims = true · 메시 SetComponentTickEnabled(false)
```

### 3.3 죽은 병사가 멈추는 것 — `StopDriving` · `ZeroPoseAxes` [A]

| 무엇 | 어떻게 | 줄 |
|---|---|---|
| 캐릭터 Tick(`AimCorrection` · stance · BF · yaw rate · AI 분기 `Shoot`) | `SetActorTickEnabled(false)` | `.cpp:653` |
| SoldierLab 컴포넌트 전부(인지·시야·무전·제압·교전·엄폐·머리 추종·1인칭 · **앞으로 추가될 것도**) | 이름 열거 대신 **클래스의 패키지가 `/Script/SoldierLab` 인가**로 판정 → `SetComponentTickEnabled(false)`. Health 자신은 제외(오버레이용) | `.cpp:659-671` |
| CMC | `StopMovementImmediately` · `DisableMovement` · 틱 off | `.cpp:677-679` |
| AIController | `StopMovement` · `ClearFocus(Gameplay)` | `.cpp:687-688` |
| ABP 축 변수 | 리플렉션으로 0: `LeanTactical` · `PelvisDrop` · `BF_AlphaL/R/U` · `HeadAimAlpha`(float/double 둘 다 처리) · `AimCorrection`(FRotator). 없는 이름은 건너뜀 | `.cpp:693-725` |

시체가 등록부에서 빠지므로 **아무도 시체를 보지·조준하지·무전하지·제압당하지 않는다** — titan 의 "RCWS 가 시체를 쏜다" 버그(`DetectableTargetComponent.h:84-92`)를 구조로 막았다. 그리고 등록부에서 빠지면 엄폐 트레이스의 "등록부 전원 무시"에서도 빠져 **시체가 엄폐물이 된다** → [Q46].

### 3.4 리플리케이션 [A] 코드 / [C] 미검증

- 서버만 바인드·판정(`.cpp:163-167`, `:195`). 복제되는 사실은 `Health` · `bDead` · `LastHit` 셋.
- 연출은 **서버도 클라이언트도 같은 함수**(`PlayHitReaction` / `PlayDeath`)를 돈다 — 서버는 직접, 클라이언트는 OnRep. 코드 경로가 하나라 "서버에선 나오는데 클라에선 안 나온다"가 원리적으로 없다.
- 래그돌은 각 프로세스가 각자 시뮬레이션(동기화 없음, 코스메틱). `DeathVelocity` 는 클라이언트에선 복제된 속도.
- **2프로세스 검증은 안 했다** — 이 프로젝트의 모든 것과 같다(`IMPLEMENTED.md` 6절 "멀티플레이 검증"). 이 컴포넌트에 `HasAuthority` 가드가 있는 것과 별개로 AI 컴포넌트 쪽은 P5 위반이 그대로다 → [W61].

---

## 4. 결정 사항과 이유

### 4.1 몽타주 + 래그돌, titan 스프링(B) 아님 [A]

전날 추천(`2026-09-14_hit_death_health_recommendation.md` 2·4절)대로. 요점만: ① ABP 3노드 + 컴포넌트로 끝나고 되돌리기 쉽다 ② 피격·사망이 **전부 시퀀스**라 디자인팀이 고칠 수 있다(핸드오프 4절의 "못 고치는 층" 8개가 안 는다) ③ titan 스프링은 서버 전용 틱(클라이언트 미표시) · Mixamo 본 이름 · ABP 변수 14 + `ModifyBone` 6 이라 "옮기는 것"이 아니라 "다시 만드는 것"이고, 그 값어치(연속 방향·부위)는 **13장이 부족하다고 관측된 뒤에** 산다(P10). 스프링은 [C-114] 로 보류.

### 4.2 슬롯 그룹을 안 가르고 `bStopAllMontages = false` [A]

추천 3단계는 "`AdditiveHitReact` 를 별도 슬롯 그룹으로"(스켈레톤 에셋 편집)였다. 이유는 엔진의 **한 그룹에 몽타주 하나** 규칙(`UAnimInstance::Montage_PlayInternal`, `AnimInstance.cpp:2762-2770`) — 같은 그룹이면 피격 몽타주가 재장전·사격 몽타주를 끊는다.

그런데 그 규칙은 **`bStopAllMontages` 인자가 true 일 때만** 적용된다. `Montage_Play(…, bStopAllMontages = false)`(`.cpp:515`)로 부르면 같은 그룹의 다른 몽타주를 건드리지 않는다. 그래서 **에셋을 안 고쳤다** — 스켈레톤은 여전히 `DefaultGroup` 하나에 6개 슬롯, `Partials` 빈 그룹. 사망 몽타주는 반대로 **true** 로 불러 재장전을 끊는다(`.cpp:560`).

⚠ 부작용 가능성 [B]: 슬롯 그룹이 하나라 **피격 중에 재장전이 시작되면** 재장전(`bStopAllMontages` 기본 true 인 BP `PlayAnimMontage`)이 피격 몽타주를 끊는다. 반대 방향은 막았고 이 방향은 안 막았다 — 눈에 띄면 그때 그룹을 가른다. 지금은 "재장전이 피격을 이긴다"가 오히려 자연스럽다고 본다.

### 4.3 연출을 BP 그래프가 아니라 C++ 이 전부 [A]

추천 3단계는 "BP: `OnDamaged` → 방향 × 세기 → 몽타주 랜덤"이었다. C++ 로 옮긴 이유:
- **서버·클라이언트 한 경로**(3.4). BP 로 하면 OnRep 배선을 BP 에서 또 해야 하고, 이 프로젝트는 BP 그래프를 MCP 로 고치는 비용이 크다(P33·P55·P92 — 산술 노드·벡터 노드가 없고, 노드가 발견돼도 안 만들어진다).
- 사망 시 멈춰야 하는 것(3.3)이 **컴포넌트 8종 + CMC + 컨트롤러 + ABP 변수 7종**인데, 이걸 BP 에서 하면 노드 수십 개다. C++ 의 `GetOutermost() == /Script/SoldierLab` 판정은 컴포넌트가 늘어도 안 고친다.
- 델리게이트(`OnDamaged` · `OnDeath`)는 남겨 뒀다 — BP 가 연출을 **더할** 자리(효과음·데칼·인지 되먹임 [W63]).

### 4.4 몽타주 경로를 생성자에 하드코딩 [A]

`ConstructorHelpers::FObjectFinder` 로 `/Game/SoldierLab/Animations/Actions/AM_MM_*` 19개를 기본값으로(`.cpp:100-135`). P6("튜닝은 데이터")과 어긋나 보이지만:
- 컴포넌트를 **추가하는 순간 동작**해야 했다 — BP 에서 19개를 손으로 넣는 것은 P53 시절엔 불가능했고 지금도 비싸다.
- **BP 에서 덮을 수 있다**(`EditAnywhere`). 클립을 바꾸면 BP 에서 바꾸면 된다.
- 첫 로드 이후 `FObjectFinder` 는 캐시 히트라 비용은 룩업 하나.
- ⚠ 대가는 5.1절의 함정 — 이 배열이 BP 컴포넌트 템플릿에 **상속되지 않았다.**

### 4.5 `Ragdoll_Start` 를 부르지 않고 같은 일을 직접 [A]

GASP `SandboxCharacter_CMC::Ragdoll_Start` 는 BP 함수라 C++ 에서 부르려면 리플렉션 호출이다. 하는 일이 5줄(`.cpp:583-589`)이라 그대로 옮겼고, `IsRagdolling` 변수는 안 세운다(아무도 안 읽는다 — 추천 1b: ABP 에 래그돌 노드 0).

### 4.6 아군 불사 = BP 체크박스 [A]

"아군은 시나리오상 죽을 필요가 없다"(사용자). 코드에 진영 분기를 두지 않고(P4 — 아군/적군 코드 한 벌) `bInvincible` 을 `BP_Soldier_Friendly` 의 상속 컴포넌트에서 체크. 맞으면 움찔은 하고 체력만 안 준다 — "맞았다"는 정보는 살아 있어야 [W63](인지 되먹임)이 성립한다.

---

## 5. 함정 · 배운 것

### 5.1 ★ BP 컴포넌트 템플릿이 C++ CDO 의 배열 기본값을 안 물려받았다 [A]

빌드 후 `AC_SoldierHealth` 를 추가하니 템플릿(`BP_SoldierCharacter_C:AC_SoldierHealth_GEN_VARIABLE`)의 **`BodyParts` 는 채워져 있고 몽타주 배열 19개는 전부 비어** 있었다. 차이는 생성자에서 `BodyParts` 는 무조건 채웠고, 몽타주 로드는 `HasAnyFlags(RF_ClassDefaultObject)` 가드 안에 있었다는 것뿐이다. **컴포넌트 템플릿은 CDO 가 아니라 자기 생성자로 새로 만들어지는 객체**라 가드 안의 코드가 안 돌고, CDO 값을 복사해 오지도 않는다(관측 — 엔진 소스로 확인하진 않았다).

- 소스는 가드를 **뺐다**(`.cpp:96-99` 주석) → **다음 빌드부터** 템플릿에도 들어간다.
- 그때까지: `ObjectTools.set_properties` 로 템플릿에 19개를 직접 썼다 → 5.2절.
- 새 원칙 **P127**(`CLAUDE.md` 5절). 같은 클래스의 다른 컴포넌트에 `RF_ClassDefaultObject` 가드가 있는지는 안 훑었다 [C].

### 5.2 P53 정정 #2 — `set_properties` 가 컴포넌트 템플릿에 **썼다** [A]

P53(2026-09-13)은 "컴포넌트 템플릿 · 배치 인스턴스의 컴포넌트 · CDO 에는 못 쓴다"였다. 오늘 `BP_SoldierCharacter_C:AC_SoldierHealth_GEN_VARIABLE` 에 **오브젝트 참조 배열 19개(`TArray<TObjectPtr<UAnimMontage>>`, 중첩 구조체 `HitReactFront.Light` 포함)와 bool** 이 `true` 로 들어갔고, 되읽기·PIE 로 확인됐다. 즉 "컴포넌트 템플릿에 못 쓴다"는 **일반 명제가 아니다** — 09-13 에 실패한 것은 float·FName·enum 이었으니 **타입이나 대상 컴포넌트에 따라 갈리는 것**으로 보인다 [B].

여전히 **못 하는 것**: **자식 BP 의 상속 컴포넌트 오버라이드**. `BP_Soldier_Friendly` 의 `AC_SoldierHealth.bInvincible` 을 쓰려 하자 **부모 템플릿에 들어가 전원이 무적**이 됐고 되돌렸다. 자식 BP 의 오버라이드는 사용자가 에디터에서 체크. 새 원칙 **P128**.

### 5.3 P13 재확인 — 에디터 켜 둔 채 빌드하면 DLL 이 안 바뀐다 [A]

사용자의 첫 빌드가 "성공"했는데 `search_subclasses(ActorComponent, "SoldierHealth")` 가 `[]`(같은 호출로 `SoldierSuppression` 은 나옴). DLL 타임스탬프 07:42(빌드 전), UBT `Log.txt` 에 `SoldierHealth.cpp.obj` 없음, DLL 문자열에 심볼 없음. 에디터 닫고 다시 빌드하니 08:49 DLL 에 들어왔다. **"빌드 성공" 메시지는 새 UCLASS 가 들어갔다는 뜻이 아니다** — 판정은 `search_subclasses` / DLL 심볼 / obj 파일 존재 셋 중 하나로.

### 5.4 `Math|Boolean|ANDBoolean` 은 만들어진다 [A]

P33·P91 은 **산술**(+ − × ÷, 벡터) 노드가 `create_node` 로 안 만들어진다는 것. 불리언 `ANDBoolean` · `NOTBoolean` 은 된다(오늘 둘 다 생성·연결·컴파일). 컴포넌트 함수 노드는 `declaring_class = /Script/SoldierLab.SoldierHealthComponent` 를 명시(P58). 새 원칙 **P129**.

### 5.5 헤더 주석과 코드가 다른 곳 하나 [A]

`.h:303` 은 `OnDamaged` 를 "치명타 포함 모든 피격"이라 적었지만, 코드는 치명타 경로(`ApplyHit → Die → PlayDeath`)에서 `OnDamaged` 를 **방송하지 않는다**(`PlayHitReaction` 안에만 있다, `.cpp:492`). 치명타는 `OnDeath` 만 온다. BP 에서 "맞았다" 를 셀 때 마지막 한 발이 빠진다 — 의도인지는 [C], 어느 쪽이든 주석이나 코드 한쪽을 맞출 것 → [W63] 착수 시 같이.

### 5.6 `ApplyAdditive_1`(재장전 애디티브) 알파가 0 이다 [B]

09-14 조사 중 관측: `ApplyAdditive_1` 의 Alpha **핀 0.0** · `node.alpha 0`. 즉 `Slot 'UpperBodyAdditive'` 경로(`MM_Rifle_Reload_Additive`)는 **사실상 꺼져 있다.** 2026-09-10 에 "재장전 애디티브를 끄려다 핀을 안 봐서 못 껐다"(P25)는 기록과 반대 상황 — 그 뒤 누군가 핀까지 0 으로 놓은 것으로 보인다. 오늘 **안 건드렸다**(이 세션 범위 밖). 재장전이 상체 `UpperBody` 슬롯만으로 충분한지, 아니면 되살릴지 → **[R8]**.

---

## 6. 검증 상태

### 6.1 확인된 것 [A] — 사용자 PIE, 2026-09-15

- 적군이 맞으면 방향에 맞는 움찔이 나오고, 재장전 중 맞아도 재장전이 이어진다.
- 체력 0 에서 쓰러져 래그돌로 정착하고 시체가 남는다. 죽은 병사는 안 돌고 안 쏜다.
- 아군은 맞아도 움찔만 하고 안 죽는다(`Invincible` 체크).
- `SoldierLab.Debug.Health 1` 오버레이가 `HP x/y` 와 마지막 피격을 찍는다.
- 총구 보정 게이트 AND 가 컴파일·저장됐고 `AimGate` 행이 AND 출력을 찍는다.

이상은 전부 **"잘됨"** 한마디다. 숫자는 없다.

### 6.2 안 잰 것 [C] — 측정 방법과 판정 기준

| ID | 무엇 | 어떻게 재나 | 판정 |
|---|---|---|---|
| **[C-110]** | 캡슐 피격에서 `BoneName` 이 None 인 빈도 · 보조 트레이스가 본을 찾는 비율 | `SoldierLab.Debug.Health 1` 둘째 줄의 본 이름을 연사 20발에서 센다(또는 `ApplyHit` 에 `UE_LOG` 한 줄) | None 이 남으면 캡슐 콜리전 오버라이드(titan 식) 검토 |
| **[C-111]** | `PA_UEFN_Mannequin` · `soldier_T_PhysicsAsset` 바디 실체 | 에디터에서 연다(MCP 로 못 읽는다) | 목·허리에서 안 꺾이나. `soldier_T` 에 spine_01/목 바디가 정말 없나 |
| **[C-112]** | Death 클립 루트 이동 | 사망 후 `slomo 0.1` 로 메시가 캡슐에서 미끄러지나 | 미끄러지면 `bForceRootLock` |
| **[C-113]** | HitReact/Death 가 `FootPlacement_0` · `LegIK_1` · `PelvisDrop` 과 싸우나 | 걷는 중 피격 발 미끄러짐 · 누워 가는 동안 발/골반 튐 | 있으면 사망 시 알파 0 |
| **[C-114]** | (스프링 채택 시) 부호값 마네킹 재측정 | 보류 | — |
| **[C-115]** | 적분기가 피격 움찔을 학습하나 — **게이트를 넣었으니 이제 "게이트가 막나"** | 피격 순간 `AimCorr` 행이 안 움직이는가(P39 수치 서명) | 움직이면 `IsSlotActive` 타이밍 문제 |
| **[C-116]** | 45구 래그돌 비용 · 8 s 후 정지의 실효 | 45명 동시 사망 fps 곡선(`feedback_unreal_fps_measure_via_log`) · sleep 여부 | 예산 안인가 |
| **[C-117]** | 몽타주 → 래그돌 전환 프레임의 스냅 | `slomo 0.1` 로 끝 0.1 s | MM 포즈로 튀는 프레임 0 |
| **[C-118]** | 임펄스 1500 · 속도 관성이 마네킹 바디 질량에 맞나 | 정면 사살 시 쓰러지는 방향·거리 | titan 값 그대로라 재측정 |

---

## 7. 남은 일

| ID | 무엇 | 시점 |
|---|---|---|
| — | **다음 빌드** — 생성자 가드 제거분(5.1). 빌드 후 템플릿 값이 C++ 기본과 같은지 되읽기 | 다음 빌드 |
| [C-110]~[C-118] | 6.2절 | AI 관찰 |
| **[W60]** | 새 병사에 `UDetectableTargetComponent` 부재 — titan RCWS/UAV 자동조준 등록부에 안 잡힌다 | 시나리오 합류 |
| **[W61]** | P5 위반 — AI 컴포넌트 · BP Tick AI 분기에 `HasAuthority` 없음 | 리슨서버 검증 |
| **[W62]** | `ApplySuppressionAlongSegment` 가 클라이언트에서도 돈다 | 같이 |
| **[W63]** | 피격을 인지에 되먹임 — `OnDamaged(Direction)` → `SoldierPerception` 기록(P62: 신선도 高·해상도 低). 5.5 의 치명타 `OnDamaged` 도 그때 | 다음 AI 작업 |
| **[Q46]** | 시체를 남기나(`DestroyAfterSeconds 0`) · 남기면 시야/엄폐를 막게 두나 | 사용자 |
| **[Q47]** | 아군 사격 허용 여부·배율(`FriendlyFireScale 1.0`) | 사용자 |
| **[R8]** | `ApplyAdditive_1` 알파 0 — 재장전 애디티브가 꺼져 있는 것이 의도인가 | 애니메이션 정리 시 |
| [W18] | ✅ 해결 | — |

Perforce: `Source/SoldierLab/AI/SoldierHealth.{h,cpp}` add · `BP_SoldierCharacter` · `SoldierCharacter_ABP` · `BP_Soldier_Friendly` · `BP_Soldier_Hostile` 편집 — 제출 여부는 사용자 [C].
