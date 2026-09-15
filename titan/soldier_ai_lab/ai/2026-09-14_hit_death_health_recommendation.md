# 병사 피격 · 사망 · 체력/데미지 — 구조 조사 보고와 방식 추천

2026-09-14 / **완료(구현됨 — 2026-09-15, 구현 기록은 `2026-09-15_health_hit_death_implementation.md`)** / 새 병사에게는 데미지를 *받는* 코드가 0줄이고(투사체는 이미 표준 경로로 *주고* 있다), 반입된 HitReact 13 · Death 6 은 **우리 ABP 에 없는 슬롯**(`AdditiveHitReact` · `FullBody`)을 겨냥한다. 추천: **피격 = 클립 애디티브 몽타주, 사망 = Death 몽타주 → 블렌드아웃에서 GASP `Ragdoll_Start`**(래그돌 함수는 이미 있다). titan 스프링(B)은 서버 전용 틱·리그 종속 이식이라 **차선**으로 미룬다. 체력은 C++ `USoldierHealthComponent` 하나, 데이터 기반, 서버 권위.

관련 항목: [W18] [W17] [R7] [W23] [C-75] [C-78] [C-83] / 관련 문서: `IMPLEMENTED.md` 2.4 · 3.1 · 5절, `assets/2026-09-14_design_team_animation_handoff.md` 4절, `weapons/2026-09-12_projectile_port.md`, titan 루트 기준 `../ai_combat/enemy_hit_reaction_physics_system.md` · `../ai_combat/enemy_scenario_combat_expansion.md` Part A

> **읽기 전용 조사였다.** 코드·에셋을 고치지 않았고 MCP 는 읽기만 썼다. 한 가지 한계: 조사 도중(15:10) **에디터가 종료됐다**(`Saved/Logs/titan_example.log` 에 `LogExit: Preparing to exit` — 크래시가 아니라 정상 종료, 마지막 MCP 호출 30초 뒤). 그 뒤의 항목은 **디스크의 `.uasset` 바이트 스캔과 엔진 소스**로 확인했고, 에디터가 있어야 보이는 것은 [C] 로 남겼다(7절).

---

## 0. 한 장 요약

| 질문 | 답 | 신뢰도 |
|---|---|---|
| 새 병사가 데미지를 **받는** 경로가 있나 | **없다.** `BP_SoldierCharacter` 와 부모 `SandboxCharacter_CMC` 둘 다 `ReceiveAnyDamage` / `ReceivePointDamage` / `ReceiveRadialDamage` **미구현**, C++ 모듈에 `TakeDamage`/`OnTake*Damage` 바인딩 **0건** | [A] |
| 투사체는 무엇을 부르나 | `UGameplayStatics::ApplyPointDamage(34, ShotDirection, Hit, InstigatorController, this, UDamageType)` — **표준 엔진 경로**, 서버 사본만 | [A] |
| 제압 판정을 데미지와 공유할 수 있나 | 판정 **기하**(이번 틱 선분 ↔ 가슴 최근접)는 공유 가능하지만 **판정 자체는 다르다** — 제압은 *스친 것*(Tick), 명중은 *맞은 것*(OnHit). 둘은 이미 다른 함수에 있고 그대로 두는 게 맞다 | [A] |
| GASP 에 래그돌이 있나 | **함수 2개가 있다** — `SandboxCharacter_CMC::Ragdoll_Start / Ragdoll_End`(pelvis 이하 SimulatePhysics). **호출처는 우리 캐릭터 어디에도 없다.** ABP 에는 래그돌 관련 노드/변수 0 | [A] |
| HitReact 13 · Death 6 의 실체 | HitReact: **전부 로컬공간 애디티브**(`AAT_LocalSpaceBase`), 0.7~1.33 s, 슬롯 **`AdditiveHitReact`**. Death: **비애디티브 전신**, 0.93~1.13 s, 슬롯 **`FullBody`**, 세그먼트가 앞뒤로 잘려 있고 **blendOut 0**. **둘 다 우리 ABP 에 슬롯 노드가 없다**(있는 것: `DefaultSlot` · `FullBodyAdditivePreAim` · `UpperBody` · `UpperBodyAdditive`) | [A] |
| 슬롯 그룹(P24) | 스켈레톤의 슬롯 그룹은 **`DefaultGroup` 하나에 6개 슬롯 전부**(`DefaultSlot` · `FullBodyAdditivePreAim` · `UpperBodyAdditive` · `UpperBody` · `FullBody` · `AdditiveHitReact`), `Partials` 는 **빈 그룹**. 단일 슬롯 몽타주라 P24 무효화는 없다. **대신 같은 그룹이라 서로 끊는다** — 피격 몽타주가 재장전·사격 몽타주를 멈춘다(엔진 규칙) | [A] 디스크 |
| titan 스프링(B)은 이식 가능한가 | 스프링 수학 자체는 ~220 줄로 독립적이다. 결합은 세 곳 — ① Mixamo 본 이름 하드코딩 ② ABP 변수 6종을 리플렉션으로 밀어 넣음(우리 ABP 엔 없음 → 변수 6 + `ModifyBone` 6 노드 추가) ③ **`HasAuthority()` 뒤에서만 틱** → 클라이언트는 아무것도 못 본다. 부호값은 컴포넌트 공간 기준이라 리그 무관하게 넘어올 것으로 보이나 **재측정 필수** | [A] / 부호는 [B] |
| 권위 게이트 | AI 컴포넌트 9쌍 · 관전 폰 · Math 에 `HasAuthority` **0건**. 있는 곳은 투사체 `OnHit` 뿐. 데미지는 그 게이트 덕에 *결과적으로* 서버 전용이다 | [A] |

---

## 1. 구조 파악

### 1a. 데미지를 받는 경로 · 투사체 · 제압 [A]

**받는 쪽 — 없다.**

- `BP_SoldierCharacter`: `list_events` 결과 `ReceiveAnyDamage` · `ReceivePointDamage` · `ReceiveRadialDamage` 전부 `bIsImplemented: false`. 부모 `SandboxCharacter_CMC`(→ `/Script/Engine.Character`)도 동일.
- `Source/SoldierLab/` 전체에서 `TakeDamage` · `OnTakeAnyDamage` · `OnTakePointDamage` · `Health` 는 **투사체의 주석에만** 나온다(`Weapons/SoldierProjectile.h:207-214`, `.cpp:459-468`).
- 변수 목록에도 체력·사망 관련 이름이 없다(`BP_SoldierCharacter` 변수 36개 — `AimCorrection` … `AITargetBlindFireV`).

**주는 쪽 — 표준 경로로 이미 주고 있다.**

```
SoldierProjectile.cpp
  102-103   CollisionComponent->OnComponentHit.AddDynamic(this, &ASoldierProjectile::OnHit)
  424-428   if (!HasAuthority()) { Deactivate(); return; }      ← 명중 확정은 서버 사본만
  433       bHitEnemy = OtherActor->IsA<ACharacter>()            ← 이펙트 선택용 대역 [R7]
  457-468   if (OtherActor && OtherActor != this && DamagePerHit > 0.f)
              ApplyPointDamage(OtherActor, DamagePerHit, ImpactDirection, Hit,
                               InstigatorController, this, DamageTypeClass)
SoldierProjectile.h
  207-214   DamagePerHit = 34.f  ·  DamageTypeClass (기본 UDamageType, .cpp:71)
```

- `BP_RifleProjectile` · titan 의 `BP_RCWSProjectile` 둘 다 `DamagePerHit` 를 **오버라이드하지 않는다**(`.uasset` 이름 테이블에 `Damage*` 없음) → 소총도 RCWS 도 **34** 다. titan `RCWSProjectile.h:190` 도 34. 체력 100 이면 **3발**.
- `AActor::TakeDamage` 순서(엔진 `Actor.cpp:3417-3473`): **`ReceivePointDamage`/`OnTakePointDamage`(3431-3432) → `ReceiveAnyDamage`/`OnTakeAnyDamage`(3472-3473)**. 포인트 델리게이트가 `BoneName` · `ShotDirection` · `HitInfo` 를 그대로 준다. titan `BP_Enemy_Base` 가 PointDamage 에서 `LastHit*` 를 캐싱하고 AnyDamage 에서 쓰는 것이 이 순서에 기댄 것인데, **우리는 포인트 데미지 하나로 끝난다**(투사체가 전부 포인트 데미지).
- ⚠ **`HitInfo.BoneName` 이 `None` 일 가능성** [C-110]: 엔진 기본 `ACharacter` 는 캡슐 `Pawn`(QueryAndPhysics) · 메시 `CharacterMesh`(QueryOnly, WorldDynamic 기본 Block)(`Character.cpp:79, 130-131` · `BaseEngine.ini:3110, 3112`). 투사체는 오브젝트 타입 `WorldDynamic` 스윕이라 **둘 다 Block 이고 바깥에 있는 캡슐이 먼저 걸린다** → 본 이름 없음. titan 이 정확히 이 함정을 겪었다(`../ai_combat/enemy_hit_reaction_physics_system.md` 1절). `SandboxCharacter_CMC.uasset` 에 `CollisionProfileName` · `BodyInstance` 이름이 있어 **GASP 가 무언가 오버라이드했을 수 있다** — 어느 컴포넌트를 어떻게인지는 에디터에서 봐야 한다. 대안 둘: (가) titan 식 — 캡슐 `WorldDynamic=Ignore` + 메시 Block(`collisionProfileName="Custom"` 필수) (나) 콜리전은 그대로 두고 **`OnHit` 에서 메시에 대해 보조 라인트레이스**(재질 트레이스 `.cpp:504-520` 와 같은 축, ±50 cm)로 본 이름만 얻는다 — 캡슐 이동 충돌을 안 건드리므로 **(나)를 권한다**.

**제압 — 이미 "스친 것" 판정이 있고, 명중과는 함수가 다르다.**

```
SoldierProjectile.cpp
  322       ApplySuppressionAlongSegment(PreviousLocationForWhiz, CurrentLocationForWhiz)   ← Tick 매 틱
  330-395   등록부(USoldierRegistrySubsystem) 전원에 대해
              Body = Soldier->GetTargetLocation()                 (spine_03, :361)
              MissDistanceCm = |Body − ClosestPointOnSegment|      (:385-386)
              Suppression->ApplyNearMiss(MissDistanceCm)           (:393, 비행당 1회)
SoldierSuppression.h
  57  ApplyNearMiss(MissDistanceCm)   65 SuppressionRadiusCm 500   69 NearMissImpulse 0.35   79 RecoveryPerSecond 0.25
  61  ApplyImpulse(Amount)            ← "폭발, 분대원 전사" 용으로 이미 비워 둔 훅
```

- 제압은 **Tick 의 선분 기하**, 명중은 **`OnComponentHit`**. 같은 총알이 맞으면 그 틱에 둘 다 일어난다(맞은 탄은 최근접 거리 ≈ 0 이라 최대 임펄스도 받는다). **공유할 것은 없다** — 이미 자연스럽게 겹친다.
- ⚠ `ApplySuppressionAlongSegment` 는 **`HasAuthority` 게이트가 없다**. 투사체가 `bReplicates=false` 라 클라이언트도 자기 사본을 날리고 자기 쪽 병사 제압값을 올린다(`.cpp:415-423` 주석). 지금은 클라이언트에 AI 판단이 없어 무해하지만 1f 와 같은 계열.
- `ApplyImpulse` 가 있으므로 **"분대원이 쓰러졌다"를 제압 임펄스로** 주는 것은 새 훅 없이 한 줄이다(3절).

### 1b. GASP 캐릭터의 래그돌 · 피직스 에셋 · PhysicalAnimation

**래그돌 함수는 있고, 부르는 곳은 없다** [A]:

```
SandboxCharacter_CMC (read_graph_dsl — 손실 있음, 인자 일부 누락)
  Ragdoll_Start:  CharacterMovement.SetMovementMode(…)  →  IsRagdolling = true
                  → Capsule.SetCollisionEnabled(…)  →  Mesh.SetCollisionObjectType(PhysicsBody)
                  → Mesh.SetCollisionEnabled(QueryAndPhysics)
                  → Mesh.SetAllBodiesBelowSimulatePhysics("pelvis", true)
  Ragdoll_End:    IsRagdolling = false → SetMovementMode(Falling) → Capsule QueryAndPhysics
                  → Mesh ObjectType Pawn · QueryOnly → SetAllBodiesSimulatePhysics(false)
  Get_PropertiesForRagdoll: (IsRagdolling 을 돌려주는 것으로 보임 — DSL 이 `return 0` 으로 뭉갬)
```

- `SandboxCharacter_CMC:EventGraph` 에서 `find_nodes(title="Ragdoll")` **0건**. `BP_SoldierCharacter` 도 0건. 즉 GASP 샘플의 래그돌 키 바인딩은 이 캐릭터 계열엔 안 딸려 왔다(추정: 샘플의 PlayerController/입력 쪽).
- **`SoldierCharacter_ABP` 에 래그돌 관련 노드가 없다** — AnimGraph 71 노드 목록에 `RigidBody` · `PoseSnapshot` 없음, 변수 목록에 `IsRagdolling` 없음. GASP 래그돌은 **순수 물리**(ABP 미관여)다. 우리 ABP 에 그대로 성립한다.
- **PhysicalAnimationComponent 는 없다** — `BP_SoldierCharacter` · `SandboxCharacter_CMC` · 자식 2개의 이름 테이블 어디에도 `PhysicalAnimation` 없음. titan `BP_Enemy_Base` 에는 있지만 강도 전부 0 = 순수 래그돌(`../ai_combat/enemy_scenario_combat_expansion.md` Part A 4항) → **GASP `Ragdoll_Start` 와 결과가 같다. 들여올 필요 없다.**

**피직스 에셋** — MCP 로 바디 목록을 못 읽는다(`CLAUDE.md` 6.1). `.uasset` 이름 테이블로 *본 이름*만 세었다 [B]:

| 에셋 | 메시 | 본 이름(바디 후보) | `BoneName` 태그 수 | 컨스트레인트 태그 수 |
|---|---|---|---|---|
| `Characters/UEFN_Mannequin/Rigs/PA_UEFN_Mannequin` | `SKM_UEFN_Mannequin`(적군 마네킹) | pelvis · spine_01~05 · neck_01/02 · head · clavicle/upperarm/lowerarm/hand L/R · thigh/calf/foot L/R = **23** | 23 | 22 |
| `SoldierLab/Characters/Ally/soldier_T_PhysicsAsset` | `soldier_T`(아군) | 위에서 **spine_01 · neck_01/02 · head 제외 = 20** (head 는 이름은 있음) | 29 | 19 |
| `Soldiers/New_enemy_soldiers/enemy_T_PhysicsAsset` | `enemy_T`(titan 쪽, 마네킹 스켈레톤 아님) | 미확인 — [Q42] 결정 전엔 관계없음 | — | — |

이름 테이블은 바디의 정확한 개수·형상·프로파일을 말해 주지 않는다. **에디터에서 열어 볼 것 → [C-111]**(판정: 45명 래그돌에 쓸 만한 바디 수인가 · `soldier_T` 에 spine_01/목 바디가 실제로 없어 상체가 꺾이는가).

### 1c. HitReact 13 · Death 6 클립/몽타주 실체 [A] (MCP `get_properties` 실측, 에디터 종료 전)

⚠ 프롬프트의 "Front/Back/Left/Right × Lgt/Med/Hvy" 가 아니다 — **Front Lgt×4 · Front Med×2 · Front Hvy×1 · Back Lgt/Med · Left Lgt/Med · Right Lgt/Med = 13**. Hvy 는 정면뿐이다.

**HitReact 13 (`Animations/Actions/MM_HitReact_*` + `AM_MM_HitReact_*`)**

| 항목 | 값 |
|---|---|
| 애디티브 | **전부 `AAT_LocalSpaceBase`**(로컬공간). ⚠ 사격 `MM_Rifle_Fire` 는 `AAT_RotationOffsetMeshSpace`(메시공간) — **같은 `ApplyMeshSpaceAdditive` 노드에 못 꽂는다.** 로컬 애디티브 경로는 재장전 `MM_Rifle_Reload_Additive`(`AAT_LocalSpaceBase`) → `Slot 'UpperBodyAdditive'` → `ApplyAdditive_1` 이 이미 있다 |
| 기준 포즈 | 7개 `ABPT_AnimFrame`(자기 자신 f0; `Back_Lgt_01` 만 `Actions/MM_Rifle_Idle_ADS` f0), 6개 `ABPT_LocalAnimFrame`(정상, P21). P20 의 "끊긴 참조" 없음 |
| 루트 | `bEnableRootMotion false` · `bForceRootLock false`. 애디티브라 루트 이동은 델타로만 들어간다(정지 클립 기준이므로 실질 0 으로 추정 — 검증은 [C-112] 에 묶음) |
| 길이 | 0.70 ~ 1.33 s (Front_Lgt_01 0.70 · Front_Hvy 0.80 · Right_Lgt 1.33) |
| 몽타주 | 슬롯 **`AdditiveHitReact`** 1개 · blendIn 0 또는 0.06 · **blendOut 0.5**(Right_Med 0.6) · 세그먼트 = 클립 전체(단 `Left_Med_01` 은 1.0 s 중 0.8 까지) |

**Death 6 (`MM_Death_*` + `AM_MM_Death_*`)**

| 항목 | 값 |
|---|---|
| 애디티브 | **`AAT_None`** — 전신 포즈. 슬롯 **`FullBody`** 1개 |
| 루트 | `bEnableRootMotion false` · `bForceRootLock false`. ⚠ 몽타주 쪽 `bEnableRootMotionTranslation/Rotation` 은 MCP 로 못 읽었고(엔진 기본값 true) ABP `RootMotionMode` 는 오버라이드 없음(기본 `RootMotionFromMontagesOnly`). **클립 루트 트랙에 이동이 있는지가 관건** → [C-112] |
| 길이 / 세그먼트 | 0.93 ~ 1.13 s. **세그먼트가 앞뒤로 잘려 있다** — `Death_Front_01` 은 1.10 s 클립의 0.15 ~ 1.00, `Death_Right_01` 0.09 ~ 0.933 |
| 블렌드 | blendIn 0.06~0.10 · **blendOut 0** · `blendOutTriggerTime −1` |

**우리 ABP 의 슬롯 노드 4개** (`SoldierCharacter_ABP:AnimGraph.AnimGraphNode_Slot_0..3` 실측): `DefaultSlot` · `FullBodyAdditivePreAim` · `UpperBody` · `UpperBodyAdditive`. **`AdditiveHitReact` 도 `FullBody` 도 없다.** 지금 `PlayAnimMontage(AM_MM_HitReact_*)` 를 불러도 **아무것도 안 보인다** — 슬롯 이름은 스켈레톤에 등록돼 있어 `HasValidSlotSetup` 을 통과하고 재생은 되지만, 그 슬롯을 읽는 노드가 그래프에 없다(추정: `LogAnimMontage` 경고도 안 뜬다 — 슬롯 검증은 스켈레톤 기준이지 그래프 기준이 아니다).

**스켈레톤 슬롯 그룹 (`SK_UEFN_Mannequin.uasset` 바이트 스캔, 오프셋 105105~105374)** [A·디스크]:

```
DefaultGroup : DefaultSlot · FullBodyAdditivePreAim · UpperBodyAdditive · UpperBody · FullBody · AdditiveHitReact
Partials     : (비어 있음)
```

- P24(`AM_MM_Rifle_Reload` 가 두 그룹에 걸침) 는 디스크 상으로는 `UpperBody` 가 `DefaultGroup` 에 있어 해소된 상태다(어떻게 옮겼는지는 안 읽었다). 단일 슬롯 몽타주인 HitReact/Death 는 애초에 P24 대상이 아니다.
- ⚠ **대신 "한 그룹에 몽타주 하나" 규칙이 적용된다** — `UAnimInstance::Montage_PlayInternal` 이 같은 그룹의 몽타주를 전부 멈춘다(`AnimInstance.cpp:2762-2770`, *"Enforce 'a single montage at once per group' rule"*). 지금 구성에서 **피격 몽타주가 뜨는 순간 재장전 몽타주(UpperBody+UpperBodyAdditive)와 사격 몽타주가 끊긴다.** 재장전 완료는 `BP_AR4Rifle` 쪽 타이머가 정한다고 기록돼 있으므로(`IMPLEMENTED.md` 3.1) "팔은 내려갔는데 탄창은 찬다"가 된다(추정 — 타이머 배선은 직접 안 봤다). 해결은 스켈레톤 에디터에서 `AdditiveHitReact` 를 **별도 그룹**(예: `HitReact`)으로 옮기는 것 — 에셋 작업 한 번. Death 는 끊어도 된다(죽었다).

**MM 이 도는 채로 전신 몽타주가 재생되면** — 2.4절 체인에서 `DefaultSlot` 은 `ApplyAdditive_1` 뒤, `OffsetRootBone_0` 앞이다. 그 슬롯에 전신 몽타주가 가중치 1로 들어오면 **상류(MM · 린 뱅킹 · 사격 · AO · BF · UpperBody 재장전)가 전부 덮이고**, 하류는 그대로 돈다:

| 하류 노드 | 사망 포즈 위에서 하는 일 | 대응 |
|---|---|---|
| `OffsetRootBone_0` | 루트만 건드린다. 이동이 멈추면(MOVE_None) 정착 | 없음 |
| `ModifyBone spine_01..05`(린) | 린 값이 남아 있으면 시체 척추를 비튼다 | 사망 시 `LeanTactical`·`AITargetLean` → 0 |
| `FootPlacement_0` · `LegIK_1` | 누워 가는 몸에 발 접지·골반 스프링을 건다 — **결과 미측정** | [C-113]. 필요하면 사망 시 알파 0 |
| `ModifyBone pelvis (PelvisDrop)` | stance 축이 골반을 계속 내린다 | 사망 시 `PelvisDrop` → 0(`UpdateStance` 정지) |
| `TwoBoneIK_0`(왼손) | 기본 OFF 로 전환됨(핸드오프 5절) | 없음 |

그리고 **blendOut 0 + 세그먼트 조기 종료**라 몽타주가 끝나는 프레임에 **MM idle 로 순간 복귀**한다 — Death 몽타주만으로는 시체가 서 있는다. 그래서 (A) 단독은 성립하지 않고, 몽타주 끝에서 **래그돌로 넘기거나 마지막 프레임을 잡아야** 한다(2절).

### 1d. titan 스프링(B)의 결합도 [A]

`Source/titan_example/Soldiers/EnemyCombatComponent.h/.cpp` — 파일은 1116/2718 줄이지만 스프링은 독립된 덩어리다:

| 조각 | 위치 | 줄 수 | 결합 |
|---|---|---|---|
| 튜너블 25개 | `.h:729-827` | ~100 | 없음 (`UPROPERTY` 만) |
| 상태 구조체 + 선언 | `.h:843-880` | ~40 | 없음 |
| `TriggerHitReactionPhysics` | `.cpp:724-835` | 112 | **본 이름 하드코딩**: `LeftUpLeg` · `RightUpLeg` · `LeftArm` · `RightArm`(:754-757) · `Hips`(:761) — Mixamo. 마네킹은 `thigh_l/r` · `upperarm_l/r` · `pelvis` |
| `TickOneHitReactionSpring` | `.cpp:837-875` | 39 | **ABP 변수에 리플렉션 쓰기** `ECC_SetFloatPropertyByName` |
| `TickHipsHitReactionSpring` | `.cpp:877-910` | 34 | 동일 |
| `TickHitReactionSpring` | `.cpp:912-941` | 30 | ABP 변수 이름 14개(`HitReactionOffsetPitch/Yaw/Roll` · `…LeftLegOffsetPitch/Roll` · `…RightLeg…` · `…LeftArm…` ×3 · `…RightArm…` ×3 · `…HipsOffsetX/Y/Z`) |
| 호출 | `.cpp:416` (TickComponent 끝) · `BP_Enemy_Base` PointDamage → `TriggerHitReactionPhysics` | — | ⚠ **`.cpp:223` `if (!HasAuthority()) return;` 이 스프링 틱 앞에 있다** → **스프링은 서버에서만 돈다.** ABP 변수는 리플리케이트되지 않으므로 클라이언트 화면에는 피격 반응이 없다(titan 문서에 이 사실은 없다 — 여기서 처음 확인) |
| ABP 쪽 | `ABP_Enemy_kadex2` 의 `Transform(Modify) Bone` 6개(Spine · LeftUpLeg · RightUpLeg · **LeftHand**(IK 출력본) · RightArm · Hips 이동) | — | 우리 ABP 에는 없다 → 변수 14 + 노드 6 추가 |

**떼어낼 수 있는가** — 있다. 수학(감쇠조화진동자, `.cpp:844-861`)은 순수 함수고 튜너블은 전부 UPROPERTY 다. 이식 비용은 코드가 아니라 **ABP 배선**이다: 변수 14개 + `ModifyBone` 6개를 MCP 로 넣고 컴파일(P23)·핀(P25) 함정을 지나야 한다. 컴포넌트로 만들면 아군/적군 한 벌(P4)은 지켜진다.

**부호값이 마네킹에서도 성립하는가** — `TriggerHitReactionPhysics` 는 `Mesh->GetComponentTransform().InverseTransformVectorNoScale(HitDirection)` 로 **메시 컴포넌트 공간**의 방향을 만들고(:738-740), ABP 의 `ModifyBone` 도 `BCS_ComponentSpace` + `BMM_Additive` 로 **같은 공간의 축**에 회전을 얹는다(`AnimNode_ModifyBone.cpp:82` — 헤더 :757-763 주석). 즉 킥의 방향과 적용 축이 같은 공간이라 **본의 로컬 축(리그)과 무관**하다. 마네킹도 메시가 액터 기준 Yaw −90 이라 `Pitch=−1 / Yaw=0.3 / Roll=1.0` 은 그대로 넘어올 **것으로 보인다** [B]. 단 다리의 거울 부호(`LegPitchSign −1`)는 "spine 은 위로, 다리는 아래로 뻗는다"에서 온 것이라 마네킹도 같고, 팔 부호 3종은 titan 에서도 **미실측**(`enemy_hit_reaction_physics_system.md` 9절). P77 대로 **메시가 바뀌면 실측값은 전부 다시 잰다** — 스프링을 택하면 [C-114].

### 1e. 새 병사의 절차 층과의 충돌

2.4절 체인 기준으로 **피격 오프셋을 넣을 자리**는 둘이다:

| 자리 | 형태 | 장점 | 충돌 |
|---|---|---|---|
| **① `ApplyAdditive_1` 뒤 · `DefaultSlot` 앞** (로컬 공간) | `IdentityPose → Slot 'AdditiveHitReact' → ApplyAdditive` 3노드 — 재장전 애디티브(`IdentityPose_3 → Slot 'UpperBodyAdditive' → ApplyAdditive_1`)와 **같은 모양** | 모든 축(AO·BF·린 뱅킹·UpperBody)의 **위에** 얹힌다. AO 가 정면을 다시 잡아 주지 않으므로 움찔이 그대로 보인다 | 아래 총구 보정 |
| ② `LocalToComponentSpace_0` 뒤 (컴포넌트 공간) | `ModifyBone` 추가 — 린 축(spine_01..05)과 **같은 자리, 같은 노드 종류** | 스프링(B)이 들어갈 자리. 린과 같은 본에 두 개의 additive `ModifyBone` 이 쌓인다(합성 순서만 있고 충돌은 없다) | 동일 |

**총구 보정 적분기(2.5c)와의 관계 — 진짜 위험이다** [B]:

- 적분기는 `ERR = Delta(GetControlRotation, 총구 전방)` 를 게이트 `카메라 각속도 ≤ 2°/프레임` 로 걸고 **게인 0.05(≈0.33 s)** 로 누적한다. 피격은 카메라를 안 움직이므로 **게이트가 열린 채** 총구가 흔들린다 → 0.7~1.3 s 짜리 움찔은 시간상수 0.33 s 의 적분기에 **거의 전부 학습**되고, 끝나면 다시 풀린다. 결과: 움찔이 나오는 동안 AO 가 반대로 총구를 되돌려 **"맞았는데 총은 표적에 붙어 있는" 포즈**, 끝나면 반대 방향 잔류 → 두 번의 과도현상. [C-78](블라인드 파이어)과 같은 기구.
- 대응은 P35/P37 그대로다 — **게이트를 입력으로**: `IsSlotActive('AdditiveHitReact')`(또는 스프링 에너지 > ε)이면 누적을 **유지**(0 이 아니라 hold — 2.5c 의 "게이트 거짓 → 유지" 와 같은 분기). 이건 기존 `SelectRotator(bPickA = GATE)` 의 GATE 에 AND 하나 다는 일이다.
- 측정: `AimErr` · `AimCorr` HUD 행을 피격 순간에 `slomo 0.1` 로 본다(P44). 게이트 없이 `AimCorr` 가 움찔 방향으로 감기면 위 [B] 가 확정된다 → [C-115].

**린 · stance · 왼손 IK**: 린은 ①/② 어느 쪽이든 그냥 합성된다. stance(`PelvisDrop`, `FootPlacement_0` 과 `LegIK_1` 사이)는 피격 애디티브가 골반을 건드리지 않는 한 무관하다 — HitReact 클립이 pelvis 에 델타를 갖는지는 [C-113] 에 묶는다. 왼손 IK 는 기본 OFF 다.

**사망 시 멈춰야 하는 것** — 지금은 아무것도 "죽음"을 모른다. 한 곳에서 끊을 수 있는 스위치가 있다:

| 무엇 | 왜 멈춰야 하나 | 어떻게 |
|---|---|---|
| **등록부 `USoldierRegistrySubsystem`** | 소비자 8곳이 전부 이 배열을 돈다 — `Comms.cpp:128` · `Cover.cpp:96` · `Engagement.cpp:170` · `Perception.cpp:143, 309` · `Sight.cpp:145` · `Observer:149` · `Projectile:342`. 시체가 남아 있으면 **계속 보이고, 조준되고, 제압당하고, 무전으로 보고된다**. titan 도 같은 버그가 있었다(`DetectableTargetComponent.h:84-92` — "적 사살해도 계속 등록된 채로 남아 RCWS 가 시체를 쏘는 버그", 리플렉션 폴링으로 땜질) | `Identity->Unregister` 는 지금 `EndPlay` 에서만 부른다(`SoldierIdentity.cpp:95`). **사망 시 명시적으로 부르는 한 줄**이면 8곳이 동시에 정리된다 |
| AI 컴포넌트 7개의 Tick | 진입 가드가 전부 `Perception == nullptr` 류뿐(`Engagement.cpp:210` · `Cover.cpp:323` · `Sight.cpp:123` · `Perception.cpp:480` · `Comms.cpp:179` · `Suppression.cpp:55`). 죽어도 `SetFocalPoint`(`Engagement.cpp:296-298`) · `MoveToLocation`(`Cover.cpp:447-470`) 가 돈다 — titan 의 "시체가 빙글빙글" 이 정확히 이것(`enemy_ai_combat_system_status.md` 4절) | `SetComponentTickEnabled(false)` ×7 또는 각 Tick 첫 줄 `if (!Health->IsAlive()) return;` |
| `AIController` | 이동/포커스가 남는다 | `StopMovement()` · `ClearFocus()` |
| `BP_SoldierCharacter` Tick | `AimCorrection` · `UpdateBodyYawRate/BlindFire/Stance` · AI 분기의 `Shoot()` 이 계속 돈다 | `SetActorTickEnabled(false)`(가장 넓음) 또는 Tick 첫 Branch |
| GASP CMC | `Ragdoll_Start` 가 `SetMovementMode` 를 이미 부른다 | 재사용 |
| ABP(MM) | 시체 45구가 매 프레임 MM 검색을 한다 | `bPauseAnims`/`bNoSkeletonUpdate` — 래그돌 읽기와의 관계는 [C-116] |
| 소총 액터 | `SetActorEnableCollision(false)` 상태로 손에 붙어 있다 | detach + 물리 또는 그대로 두고 함께 소멸 |

### 1f. 리플리케이션 [A]

- `Source/SoldierLab/` 에서 `HasAuthority` · `GetLocalRole` · `NetMode` · RPC 가 있는 파일은 **`SoldierProjectile.cpp` 하나**(`:424` OnHit, 도탄 RNG `:560`). AI 9쌍 · Observer · Math 에 **0건**. P5(모든 L0~L3 Tick 에 `HasAuthority()` 게이트)는 **아직 지켜지지 않았다** — 리슨서버에서 클라이언트의 시뮬레이티드 프록시 위에서도 7개 컴포넌트가 돌고, BP Tick 의 `SetAIPoseDriven(NOT IsPlayerControlled)` 는 AI 폰의 컨트롤러가 클라이언트에 복제되지 않으므로 클라이언트에서 true 가 되어 **클라이언트도 `Shoot()` 을 부를 것이다** [B](투사체는 로컬 사본이라 `OnHit` 에서 버려진다 — 멀티 검증은 한 번도 안 했다, `IMPLEMENTED.md` 6절).
- 데미지는 **결과적으로** 서버 권위다 — `ApplyPointDamage` 가 서버 사본에서만 불린다. 그러나 체력 컴포넌트는 그 우연에 기대지 말고 자기 핸들러에 `HasAuthority()` 를 둔다(3절).
- 몽타주는 서버에서 `PlayAnimMontage` 해도 시뮬레이티드 프록시에 **자동 재생되지 않는다**(`ACharacter` 는 루트모션 몽타주만 `RepRootMotion` 으로 복제). 래그돌은 각 프로세스가 **각자 시뮬레이션**한다(동기화 없음, 코스메틱). 즉 (A)(B)(C)(D) **어느 쪽이든 "맞았다/죽었다" 사실을 복제하고 연출은 각자 로컬로 돌린다**. 이 축에서 스프링이 몽타주보다 불리한 점은 없고, titan 스프링의 현 구현이 서버 전용 틱이라는 점만 다르다(1d).

---

## 2. 방식 비교

후보 정의 (프롬프트 기준, (C) 는 두 방향을 분리했다):

- **(A)** 클립 몽타주만 — HitReact 애디티브 + Death 전신. ⚠ Death 몽타주 끝에서 MM 으로 복귀하므로 **단독으론 성립하지 않는다**(마지막 프레임 유지 또는 래그돌 필요)
- **(B)** titan 스프링 피격 + 순수 래그돌 사망(속도 관성 + 임펄스) 이식
- **(C-1)** 피격 스프링 + 사망 Death 클립 → 끝에서 래그돌
- **(C-2)** ★ 피격 HitReact 애디티브 몽타주 + 사망 Death 클립 → 블렌드아웃에서 GASP `Ragdoll_Start`
- **(D)** 피격 HitReact 애디티브 몽타주 + 순수 래그돌 사망(속도 관성 + 임펄스, titan Part A)

| 축 | (A) 클립만 | (B) 스프링+래그돌 | (C-1) 스프링+Death→래그돌 | **(C-2) 몽타주+Death→래그돌** | (D) 몽타주+순수 래그돌 |
|---|---|---|---|---|---|
| ① 이동 중 피격 자연스러움 | 로컬 애디티브라 다리는 MM 을 따라간다. 클립에 다리/골반 델타가 있으면 발이 미끄러진다 [C-113] | 척추·팔·골반만 흔든다. 다리 스프링은 접지와 싸운다(titan 도 Hips 는 이동만 줬다 — 6절) [B] | (B)와 같음 | (A)와 같음 | (A)와 같음 |
| ② 45명 성능 | 몽타주 = 재생 중에만 비용. 사망 후 포즈 유지면 물리 0 [A] | 스프링 산술은 무시 가능(6×3 축). `ModifyBone` 6 개는 알파 0 이어도 매 프레임 평가. 래그돌 23 바디 × 시체 수 → 정착 후 처리 필요 [C-116] | (B)와 같음 | 몽타주 + 래그돌(정착 후 처리 [C-116]) | (D)=(C-2) 에서 Death 1 s 만 뺀 것 |
| ③ 디자인팀이 시퀀스로 고칠 수 있는 범위 | **피격·사망 전부 시퀀스** — 핸드오프 4절의 "못 고치는 층"이 **안 는다** [A] | 피격이 코드 층(⑨번째 절차 층)으로 **는다**. 사망은 물리 — 시퀀스 없음 | 피격은 코드 층, 사망은 시퀀스 | **피격·사망 시퀀스 + 정착만 물리** | 피격만 시퀀스 |
| ④ 방향·부위·세기 활용 | 방향 4 × 세기 2(+정면 Hvy) **이산 13**. 부위 없음. `BoneName` 은 안 쓴다 | **연속** — 방향 벡터 그대로, 부위 5(척추/다리 2/팔 2/골반) + 지렛대 + 채찍, 세기 연속. ⚠ 부위엔 `BoneName` 이 필요하고 지금은 `None` 일 가능성 [C-110] | (B) | (A) — 부위는 **데이터 배율(체력)에만** 쓴다 | (A) |
| ⑤ 구현 비용 / 되돌리기 | ABP 노드 3 + 스켈레톤 그룹 1 + BP 몇 노드. 몽타주 안 부르면 그대로 원복 [A] | C++ ~250 줄 이식 + ABP 변수 14 + `ModifyBone` 6(MCP AnimGraph 편집 최대 규모, P23/P25) + 부호 재측정 [C-114] | (B) + Death 배선 | **(A) + `Ragdoll_Start` 호출 1줄.** 가장 싸다 | (A)의 피격 절반 + 래그돌 호출 |
| ⑥ 멀티플레이 | 사실 복제 + 로컬 재생 — 몽타주 트리거를 OnRep/멀티캐스트로 | 같은 형태(킥 파라미터를 복제). ⚠ titan 현 구현은 서버 전용 틱 — 그대로 옮기면 **클라이언트에 안 보인다** [A] | (B) | (A) | (A) |
| ⑦ 아군/적군 코드 한 벌 | 컴포넌트 하나 + 데이터 테이블 [A] | 컴포넌트 하나. 튜너블 25 는 메시별로 갈릴 수 있다(P77) | 동일 | 동일 | 동일 |
| 치명 결함 | **Death 끝에서 MM 복귀** — 단독 불가 [A] | 서버 전용 · 리그 종속 · 클라이언트 미표시 [A] | (B)의 피격 결함 그대로 | 특별한 것 없음. Death 루트모션 [C-112] · 정착 프레임 스냅 [C-117] 두 측정이 남는다 | 사망 연출이 "풀썩" — 시퀀스로 못 고친다 |

---

## 3. 체력/데미지 시스템 제안 — 최소 사양

GAS 는 쓰지 않는다(`migration/2026-09-14_titan_example_migration.md` 3.4 — `/Script/LyraGame` 을 들이지 않기로 했다). 컴포넌트 하나:

```
Source/SoldierLab/AI/SoldierHealth.h / .cpp        (AI 폴더 — 등록부·진영과 같은 곳)
USoldierHealthComponent : UActorComponent

  데이터 (전부 UPROPERTY, 자식 BP·DataAsset 에서 덮는다 — P6)
    MaxHealth            100
    Health               Replicated(OnRep_Health)   ← 서버가 쓴다
    bDead                Replicated(OnRep_Dead)
    LastHit              Replicated(OnRep_LastHit)  { Direction, BoneName, Damage, Serial }   ← 연출 트리거. Serial 이 바뀔 때만 재생
    BodyPartMultipliers  UDataAsset/DataTable  { BoneGroup → Scale }   예) head/neck 3.0 · spine 1.0 · 팔 0.6 · 다리 0.7
                         본→그룹은 마네킹 이름으로 `BoneIsChildOf` (head·neck_01/02 / spine_01..05 / clavicle→hand / thigh→foot)
    FriendlyFireScale    1.0   (0 = 아군 사격 무효. 진영은 USoldierIdentityComponent 로 읽는다 — 새 소스 없음)

  입력
    BeginPlay(HasAuthority 만): Owner->OnTakePointDamage.AddDynamic(HandlePointDamage)
                                 Owner->OnTakeAnyDamage.AddDynamic(HandleAnyDamage)  ← 포인트가 아닌 것(폭발)만 처리, 포인트는 중복 방지
    HandlePointDamage: HasAuthority 가드 → 진영 비교(InstigatorController->GetPawn 의 Identity) → 부위 배율 → Health -= → LastHit 갱신
                       → OnDamaged.Broadcast(Dir, Bone, Damage, Instigator) → Health ≤ 0 → Die()

  출력 (델리게이트, BlueprintAssignable)
    OnDamaged(Direction, BoneName, Damage, Instigator)        ← 피격 연출 · 인지("맞았다"는 강한 관측, 나중에)
    OnDeath(Direction, BoneName, Instigator)                   ← 아래 한 곳에서 전부 끊는다

  Die() (서버)
    bDead = true → Identity->Unregister() (등록부 = 8 소비자 동시 정리)
    → AI 컴포넌트 7 Tick off → AIController StopMovement/ClearFocus
    → 근처 아군 Suppression->ApplyImpulse(…) ("분대원 전사", SoldierSuppression.h:61 의 예약된 용도)
    → OnDeath 방송 → (연출은 OnRep_Dead 가 서버·클라이언트에서 각자) 
```

**`BP_Enemy_Base` 와 대조 — 무엇을 가져오고 무엇을 안 가져오나** (`.uasset` 이름 테이블 + titan 문서):

| titan `BP_Enemy_Base` | 가져오나 | 이유 |
|---|---|---|
| `Health`(float, BP 변수) · `MaxHealth` 없음 · 부위 배율 없음 | ✗ | C++ 컴포넌트로. titan 은 `Health/IsDead` 가 BP 전용이라 C++ 쪽이 **리플렉션 폴링**(`DetectableTargetComponent.h:84-92`)으로 죽음을 알아냈다 — 그 우회를 만들 이유가 없다 |
| `IsDead` → `EnemyCombatComponent::TickComponent` 첫 줄 리턴(`.cpp:231-234`) | ✓ 형태만 | 우리는 `OnDeath` 델리게이트로 끊는다 |
| PointDamage 에서 `LastHitLocation/Direction/BoneName/Velocity` 캐시 → AnyDamage 에서 사용 | ✓ 구조 | 우리는 포인트 핸들러 하나에서 끝난다. `LastHitVelocity`(죽기 직전 속도)는 래그돌 관성 주입용 — 그대로 유용 |
| 사망: `StopMovementImmediately` · `DisableMovement` · 캡슐 콜리전 off · 메시 QueryAndPhysics · `SetAllBodiesSimulatePhysics` | ✓ = `Ragdoll_Start` | GASP 함수가 같은 일을 한다. 새로 쓰지 않는다 |
| `PhysicalAnimationComponent` + `ApplyPhysicalAnimationSettingsBelow("Hips", 강도 0)` | ✗ | 강도 0 = 순수 래그돌. 컴포넌트 불필요 |
| `SetAllPhysicsLinearVelocity(LastVelocity)` → **`DelayUntilNextTick`** → `AddImpulseAtLocation(Dir × DeathImpulseMagnitude 1500, HitLocation, Bone)` | ✓ | Chaos 가 킨네마틱→시뮬 전환을 처리하기 전 임펄스는 조용히 씹힌다(`enemy_hit_reaction_physics_system.md` 7절). **한 틱 지연은 반드시 옮긴다.** 1500 은 titan 리그 값 — 재측정 [C-118] |
| `DeathIndex` 랜덤 | ✓ 변형 | 우리는 방향(Front/Back/Left/Right) → 후보 몽타주 중 랜덤 |
| 5초 후 `DestroyActor` | △ | 시체를 남길지 [Q46] |
| `TriggerHitReactionPhysics` 호출 | ✗ (차선) | 2절 |

주의 두 가지:
- 새 병사에는 `UDetectableTargetComponent` 가 없다 — titan 의 **RCWS 자동조준 등록부**에 안 잡힌다. 맞으면 데미지는 들어오지만 RCWS 가 *겨냥*하진 않는다. 이 문서 범위 밖 → [W60].
- `bHitEnemy = IsA<ACharacter>()`([R7]) 는 이펙트 선택이고 데미지와 무관하다. 체력 컴포넌트가 진영을 알게 되면 [W23] 의 대체 소스가 하나 더 생긴다(투사체가 `USoldierIdentityComponent` 를 직접 읽어도 된다 — 어느 쪽이든 한 줄).

---

## 4. 최종 추천

### 추천: **(C-2) 피격 = HitReact 애디티브 몽타주 · 사망 = Death 몽타주 → 블렌드아웃에서 GASP `Ragdoll_Start`**

1. **가장 싸고 가장 되돌리기 쉽다** — ABP 노드 3개(재장전 애디티브와 같은 모양) + 스켈레톤 슬롯 그룹 1회 + `Ragdoll_Start` 호출. 새 절차 층이 하나도 안 생긴다(핸드오프 4절의 "시퀀스로 못 고치는 층"이 그대로 8개).
2. **디자인팀의 손이 닿는다** — 피격·사망이 전부 시퀀스라 검수·수정 루프가 지금 세팅 그대로 돈다. 스프링을 넣으면 그들이 고칠 수 없는 층이 하나 늘고, 그 층은 총구 보정 적분기와도 싸운다(1e).
3. **titan 스프링은 그대로는 못 쓴다** — 서버 전용 틱(클라이언트 미표시) · Mixamo 본 이름 · ABP 변수 14 + 노드 6. "완성된 것을 옮긴다"가 아니라 "다시 만든다"에 가깝고, 그 값어치(연속 방향·부위)는 **13장으로 부족하다고 측정된 뒤에** 사야 한다(P10).

### 차선: **(D) 피격 몽타주 + 순수 래그돌 사망(속도 관성 + 한 틱 지연 임펄스)**

Death 클립이 [C-112](루트 이동) 나 [C-113](하류 노드와의 충돌) 에서 문제가 되면 사망만 (D) 로 내린다 — (C-2) 에서 `PlayAnimMontage(Death)` 한 줄을 빼고 즉시 `Ragdoll_Start` 를 부르면 된다. titan 이 실제로 쓰고 사용자가 확인한 형태다.

**(B)/(C-1) 스프링은 보류** — 13장이 이산적이라 거슬린다는 것이 **관측**되면(예: 다리 피격이 상체 움찔로만 나온다) 그때 컴포넌트로 이식한다. 그 경우도 1d 의 세 결합을 풀고 [C-114] 부호 재측정을 거친다.

### 구현 순서 (각 단계에 검증 — P7 "표시 먼저" · P10 "계측부터")

| 단계 | 할 일 | 검증 (판정 기준) |
|---|---|---|
| **0. 계측** | 체력 HUD 행(`SoldierDebugAxes` 에 `HP` · `LastHitBone` · `LastHitDir` · `Dead`) + 피격 시 `UE_LOG`(공격자 · 본 · 방향 · 배율 · 남은 HP). `OnTakePointDamage` 를 BP 로 임시 바인드해 **`BoneName` 이 `None` 인지부터** 찍는다 | PIE 1회: 병사에게 맞을 때마다 로그 1줄. `BoneName` ≠ None 이면 [C-110] 닫힘. None 이면 1a 의 (나) 보조 트레이스 |
| **1. 체력 컴포넌트** | `USoldierHealthComponent`(3절) — 새 `UCLASS` 라 **에디터 닫고 빌드**(P13). `BP_SoldierCharacter` 에 컴포넌트 추가. 연출 없이 **숫자만** | HUD 의 `HP` 가 34 씩 준다. 0 에서 `Dead` 가 켜지고 **병사가 등록부에서 빠져 아무도 그를 안 쏜다**(`SoldierLab.Debug.Perception 1` 오버레이에서 기록이 늙어 사라진다). 죽은 병사가 **회전·이동·발사를 멈춘다** |
| **2. 사망 연출** | `OnRep_Dead`/서버 공통 경로에서 `PlayAnimMontage(방향별 Death)` → `OnMontageBlendingOut` 에서 `Ragdoll_Start` → `SetAllPhysicsLinearVelocity(LastVelocity)` → 다음 틱 `AddImpulseAtLocation`. 사망 시 `LeanTactical`·`PelvisDrop`·BF 알파 0 | `slomo 0.1`(P44)로 몽타주 끝 프레임을 본다 — **MM 포즈로 튀는 프레임이 없어야** 한다 [C-117]. 메시가 캡슐에서 미끄러지면 [C-112]. 누워 가는 동안 발/골반이 튀면 [C-113] → `FootPlacement` 알파 0 |
| **3. 피격 연출** | 스켈레톤: `AdditiveHitReact` 를 **새 그룹**으로. ABP: `IdentityPose → Slot 'AdditiveHitReact' → ApplyAdditive` 를 `ApplyAdditive_1` 과 `DefaultSlot` 사이에(재장전 애디티브 자리 복제). BP: `OnDamaged` → 방향(ShotDirection vs 액터 전방 → F/B/L/R) × 세기(Damage/DamagePerHit → Lgt/Med/Hvy, 데이터) → 몽타주 랜덤 | ① 재장전 중 맞아도 **재장전 몽타주가 살아 있다**(`LogAnimMontage` 에 stop 로그 없음) ② 걷는 중 맞아도 발이 안 미끄러진다 [C-113] ③ 웅크린 채 맞아도 일어서지 않는다(애디티브 확인) |
| **4. 총구 보정 게이트** | 2.5c 의 GATE 에 `NOT IsSlotActive('AdditiveHitReact')` AND. 누적은 유지(hold) | 피격 순간 `AimCorr` 가 **움직이지 않는다**(P39 — 숫자 서명). 게이트 전에 한 번 재서 [C-115] 의 [B] 를 확정하고 넣는다 |
| **5. 45명 정착 처리** | 시체 N 초 후: 바디 sleep 확인 → 메시 Tick off(또는 `bPauseAnims`) → 시체 유지/삭제([Q46]) | 45명 동시 사망 PIE 에서 fps 곡선(`feedback_unreal_fps_measure_via_log` 방식) [C-116] |
| **6. 리플리케이션** | `Health`·`bDead`·`LastHit` OnRep 로 클라이언트 연출. 리슨서버 2프로세스 | 클라이언트 화면에서 피격/사망이 서버와 같은 순간에 보인다. 몽타주 방향이 같다 |

---

## 5. 새 미해결 항목 (OPEN_ITEMS 형식 — ~~**등록은 하지 않았다**~~ → **2026-09-15 등록됨, ID 는 재번호 — 8절 표 참고**)

| ID | 항목 | 내용 · 판정 기준 | 상태 |
|---|---|---|---|
| **C-110** | 투사체가 새 병사에 맞을 때 `HitInfo.BoneName` 이 `None` 인가 | 엔진 기본 콜리전(캡슐 Pawn / 메시 CharacterMesh)이면 캡슐이 먼저 걸린다(`Character.cpp:79,130` · `BaseEngine.ini:3110,3112`). `SandboxCharacter_CMC` 가 오버라이드했는지는 에디터에서. **판정**: `OnTakePointDamage` 로그의 `BoneName`. None 이면 `OnHit` 보조 트레이스(1a-나) | 측정 대기 |
| **C-111** | `PA_UEFN_Mannequin` · `soldier_T_PhysicsAsset` 바디 실체 | 이름 테이블은 23/20 본. 에디터에서 바디 수·형상·컨스트레인트·`soldier_T` 의 spine_01/목 바디 부재 확인. **판정**: 래그돌이 목·허리에서 부러지지 않는가 | 에디터 |
| **C-112** | Death 6 클립의 루트 트랙에 이동이 있는가 | `bEnableRootMotion false` · `bForceRootLock false` · 몽타주 루트모션 플래그 미확인. **판정**: 에디터 프리뷰에서 루트 본 이동 / PIE 에서 사망 후 메시가 캡슐에서 미끄러지는가. 있으면 `bForceRootLock true` 또는 몽타주 루트모션 유지 | 에디터 |
| **C-113** | HitReact/Death 가 하류 절차 노드(`FootPlacement_0` · `LegIK_1` · `ModifyBone pelvis`)와 싸우는가 · HitReact 에 다리/골반 델타가 있는가 | **판정**: 걷는 중 피격 시 발 미끄러짐 · 누워 가는 동안 발/골반 튐. 있으면 사망 시 알파 0 | PIE |
| **C-114** | (스프링 채택 시) 부호값 6종의 마네킹 재측정 | 컴포넌트 공간이라 이전될 것으로 보이나 P77. titan 팔 부호 3종은 원래도 미실측 | 보류 |
| **C-115** | 총구 보정 적분기가 피격 움찔을 학습하는가 | **판정**: 피격 순간 `AimCorr` 가 움찔 방향으로 감기면 [B] 확정 → 4단계 게이트 | PIE |
| **C-116** | 45구 래그돌의 비용과 정착 후 처리 | 바디 sleep 여부 · 메시 Tick off 가 마지막 포즈를 유지하는가 · fps 곡선 | PIE |
| **C-117** | Death 몽타주 → 래그돌 전환 프레임에 MM 포즈로 튀는 프레임이 있는가 | blendOut 0 · 세그먼트 조기 종료라 발생 가능. **판정**: `slomo 0.1`. 있으면 `OnMontageBlendingOut` 대신 `blendOutTriggerTime` 앞당김 | PIE |
| **C-118** | `DeathImpulseMagnitude 1500` · `LastVelocity` 관성이 마네킹 바디 질량에서 맞는가 | titan 값. 재측정 | PIE |
| **W60** | 새 병사에 `UDetectableTargetComponent` 부재 — titan RCWS/UAV 자동조준 등록부에 안 잡힌다 | 데미지는 들어오나 RCWS 가 겨냥하지 않는다. 시나리오 합류 시점에 결정 | 열림 |
| **W61** | P5 위반 — AI 컴포넌트 7개 · BP Tick 의 AI 분기에 `HasAuthority` 게이트 없음 | 클라이언트도 판단·발사(로컬 사본)한다. 체력 작업과 별개지만 같은 리슨서버 검증에서 드러난다 | 열림 |
| **W62** | `ApplySuppressionAlongSegment` 클라이언트 실행 | 서버 전용으로 가둘지, 아니면 제압을 로컬 코스메틱으로 볼지(현재 AI 판단이 서버에만 있다면 무해) | 열림 |
| **W63** | 피격을 인지 층에 되먹임 — "맞았다"는 방향만 있는 강한 관측(총성 기록과 같은 형태) | `OnDamaged(Direction)` → `SoldierPerception` 기록. P62(신선도 高 · 해상도 低) | 열림 |
| **Q46** | 시체를 남기나 지우나, 남기면 시야/엄폐 트레이스를 막게 두나 | `Ragdoll_Start` 가 메시를 `PhysicsBody` 로 바꿔 총알·시야(`Sight` 채널 기본 Block)를 막는다. 등록부에서 빠지면 엄폐 트레이스의 "등록부 전원 무시"에서도 빠져 **시체가 엄폐가 된다** | 사용자 |
| **Q47** | 아군 사격(FriendlyFire) 허용 여부와 배율 | 3절 `FriendlyFireScale`. `Masked`(P68) 가 사선을 막지만 산포는 막지 못한다 | 사용자 |

---

## 6. 이 세션에서 확인하지 못한 것 (에디터 종료로 [C] 로 남긴 것)

- `BP_SoldierCharacter` / `SandboxCharacter_CMC` 의 캡슐·메시 **콜리전 프리셋** → [C-110]
- 피직스 에셋 **바디 목록** → [C-111] (MCP 로 원래 못 읽는다)
- Death 클립 **루트 트랙** · 몽타주 루트모션 플래그 → [C-112]
- `AO_Blend_Curve` ([R6]) — 무관하나 같은 종류의 미확인
- 스켈레톤 슬롯 그룹은 **디스크 저장본** 기준이다. 메모리에서 바뀌었는데 저장 안 된 상태라면 다르다(`CLAUDE.md` 6.1 "레지스트리는 디스크 기준")

**확인한 것의 출처 정리**: MCP 실측(에디터 종료 전) — 클립/몽타주 프로퍼티 19쌍 · ABP 슬롯 4 · AnimGraph 노드 목록 · ABP 변수 · BP 이벤트/함수/변수 목록 · `Ragdoll_Start/End` DSL · 피직스 에셋 참조. 바이트 스캔 — 스켈레톤 슬롯 그룹 · PA 본 이름 · BP 이름 테이블(PhysicalAnimation/Damage 부재, `DamagePerHit` 미오버라이드) · ABP `RootMotionMode` 미오버라이드. 엔진 소스(5.8) — `AnimInstance.cpp:2762-2770` · `Actor.cpp:3417-3473` · `Character.cpp:79,130-131` · `BaseEngine.ini:3110-3112`. 프로젝트 소스 — 1절의 파일:줄.

---

## 8. 정정 · 후일담 (2026-09-15 — 구현 뒤)

> 3.3 규칙대로 본문은 안 고쳤다. 추천 (C-2) 는 **그대로 채택·구현됐고** 사용자가 PIE 로 확인했다("잘됨"). 구현 기록은 **`2026-09-15_health_hit_death_implementation.md`**. 이 문서는 `ai/drafts/` 에서 `ai/` 로 올라왔다(초안 폴더 규칙).

**추천에서 달라진 것**

| 추천 (4절 구현 순서) | 실제 | 이유 |
|---|---|---|
| 3단계 "BP: `OnDamaged` → 방향 × 세기 → 몽타주 랜덤" | **C++ 이 전부 한다** — `USoldierHealthComponent::PlayHitReaction / PlayDeath`. BP 는 델리게이트만 받는다 | 서버·클라이언트 한 경로, BP 그래프 편집 비용(P33·P55·P92). 구현 기록 4.3절 |
| 3단계 "스켈레톤: `AdditiveHitReact` 를 새 그룹으로" | **안 갈랐다.** `Montage_Play(…, bStopAllMontages = false)` 로 같은 그룹의 재장전·사격을 안 끊는다. 슬롯 그룹은 `DefaultGroup` 하나 그대로 | 1c 의 "한 그룹에 몽타주 하나" 규칙은 `bStopAllMontages = true` 일 때만 적용된다. 에셋 변경 0. 구현 기록 4.2절 |
| 2단계 "`OnMontageBlendingOut` 에서 `Ragdoll_Start`" | 타이머 **길이 − `RagdollLeadSeconds` 0.1 s** 에서 C++ 이 같은 일을 직접(`StartRagdoll`). GASP BP 함수는 안 부른다 | blendOut 0 이라 BlendingOut 콜백은 사실상 끝 프레임이다. 구현 기록 4.5절 |
| 4단계 총구 보정 게이트 — "게이트 전에 한 번 재서 [C-115] 을 확정하고 넣는다" | **재지 않고 넣었다.** `AND(InRange, NOT IsHitReacting)` | 사용자 우선순위. [C-115] 은 "게이트가 막나"로 질문이 바뀌어 남는다 |
| 1a-(나) 보조 트레이스 | 그대로 — `bResolveBoneByTrace`, 메시 `LineTraceComponent` ±60 cm. **None 빈도는 안 쟀다** → [C-110] | |
| 0단계 계측 | `SoldierDebugAxes` 행이 아니라 별도 오버레이 `SoldierLab.Debug.Health 1`(머리 위 `HP x/y` + 마지막 피격) | 병사 45명 전원을 한눈에 보려면 액터 위 표시가 맞다 |
| 3절 `OnTakeAnyDamage` 바인드(폭발용) | `OnTakeRadialDamage` 바인드 | 포인트가 아닌 것 = 래디얼뿐. AnyDamage 는 포인트와 중복이라 안 걸었다 |
| 3절 `BodyPartMultipliers` DataAsset | 컴포넌트 `TArray<FSoldierBodyPartScale>` 프로퍼티(EditAnywhere) | DataAsset 을 만드는 비용 대비 이득 없음. 자식 BP 에서 덮는다 |
| 아군 사격·시체 처리 [Q46][Q47] | **아군은 안 죽는다**(사용자 결정) — `bInvincible` 을 `BP_Soldier_Friendly` 에서 체크. 시체는 남긴다(`DestroyAfterSeconds 0`), `FriendlyFireScale 1.0` | Q 둘은 여전히 열림 |

**ID 재번호** — 5절의 ID 는 등록 없이 매긴 것이라, 같은 날 다른 세션 둘과 겹쳤다: 노출 회계 라운드가 **C-102~C-107** 을 먼저 등록했고, 급선회/총내림 세션(`animation/prototypes/2026-09-15_sharp_turn_pop_bf_head_scale_and_weapon_socket.md`)과는 **W55~W58 · Q44 · C-108/109** 를 동시에 잡았다가 서로 비켜 갔다(그쪽은 최종 C-119~C-120 · W64~W67 · Q48 · P121~P126, 빈 번호는 `OPEN_ITEMS.md` 머리말). 본문의 ID 참조를 아래대로 **기계 치환**했다(본문 서술은 그대로):

```
초안 C-102 → C-110   C-103 → C-111   C-104 → C-112   C-105 → C-113   C-106 → C-114
초안 C-107 → C-115   C-108 → C-116   C-109 → C-117   C-110 → C-118
초안 W44 → W60       W45 → W61       W46 → W62       W47 → W63
초안 Q44 → Q46       Q45 → Q47
```

**본문 서술 중 이제 낡은 것**(취소선은 안 그었다 — 조사 시점의 사실이다): 0절 "받는 경로가 없다" · 1c "우리 ABP 에 슬롯 노드가 없다" · 1e "아무것도 죽음을 모른다" — 전부 09-15 에 해소. 1c 의 "피격 몽타주가 재장전을 끊는다"는 **`bStopAllMontages = false` 로 회피**됐으므로 스켈레톤 작업이 필요 없다. 1e 의 "총구 보정 적분기와의 관계 [B]" 는 게이트로 막았고 측정은 [C-115].

**추가 관측 (09-14 조사 중, 이 문서에 안 적었던 것)**: `ApplyAdditive_1`(재장전 애디티브) 의 Alpha **핀이 0.0** · `node.alpha 0` — 재장전 애디티브 경로가 사실상 꺼져 있다 [B]. 안 건드렸다 → **[R8]**.
