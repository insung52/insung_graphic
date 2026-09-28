# 차량이 밟는 시체 · 사망 시 무기 드롭

2026-09-23 / 코드 완료·검증 대기 / 차량이 시체를 밟을 때 서스펜션이 시체를 지면으로 읽던 경로를 끊고, 사망 시 총을 손에서 떨어뜨리게 함.

대상: `Source/titan_example/Vehicles/UGVWheeledVehicleMovementComponent.cpp`(한 줄) · `Source/SoldierLab/AI/SoldierHealth.{h,cpp}`(드롭 일체).
관련: `../vehicle/ugv/2026-09-16_ugv_flip_spin_projectile_physics_body.md`(**같은 증상의 다른 원인** — 그때는 서스펜션이 아니라 자기 탄의 키네마틱 물리 바디가 `Hull1`을 쳤다. 이번 건은 **진짜로 서스펜션 경로**다) · `ai/2026-09-15_health_hit_death_implementation.md`(`USoldierHealthComponent` API) · `ai/2026-09-17_hit_death_three_causes.md`(래그돌·피직스 에셋 — **값이 바뀐 자리는 그쪽이 최신**) · `../level_new_kadex_0811/2026-09-22_scenario_restart_implementation.md` §5(`EndPlay` + `bDestroyCarriedActorsOnDestroy`, P188 — **이 문서의 5절이 그 정리 목록에 드롭분을 더한다**).
원칙: **P192**(시체·떨어진 물체는 차량 서스펜션에 지면으로 보이면 안 된다) · P188(자기가 든 것은 자기가 치운다) · P130(세계가 대신 알려 주지 않는다 — 4절의 "화면에 보이는 총이 무엇인가").
값: **[C-171]~[C-173]**. 작업: **[W118]~[W119]**.

> **왜 `ai/` 인가**: 바뀐 코드의 대부분(`USoldierHealthComponent` 의 드롭 일체)이 SoldierLab 모듈의 사망 처리이고, titan 쪽은 **생성자 한 줄**이다. 차량 물리 자체를 바꾼 것이 아니라 "차량이 시체를 무엇으로 보는가"를 바꿨다 → 사망/시체 축으로 읽는 편이 다음 사람에게 맞다. `vehicle/ugv/` 쪽에서 찾을 사람을 위해 `DOCS_INDEX.md` 의 UGV 절에도 같이 걸어 뒀다.

---

## 1. 문제 제기 [A · 사용자 관측]

- **UGV(`BP_UGV_0901`)가 쓰러진 적군을 가끔 밟고 지나간다.** 시체가 **밀려나는 그림 자체는 좋다** — 없애고 싶은 것이 아니다.
- 그런데 **시체가 땅에 박혀 부들부들 떨거나 이상하게 움직이는** 경우가 있다.
- 그것 때문에 **차량 서스펜션에 이상한 값이 들어가 차량이 뒤집히는 일은 절대 일어나면 안 된다.**

⚠ **실제 뒤집힘 목격은 아직 0회다.** 이번 수정은 **예방**이다 — "떨린다"는 관측은 실제이고, 그 떨림이 어디로 흘러가는지를 코드에서 따라가 보니 서스펜션으로 가는 길이 열려 있었다는 것이 1차 결론이다(2절). 재현된 뒤집힘을 고친 것이 아니므로 **판정도 정성적일 수밖에 없다**(6절).

---

## 2. 원인 분석 — 바퀴가 시체를 지면으로 읽는다 [A · 엔진 소스]

### 2.1 Chaos 서스펜션은 무엇을 지면으로 치는가

Chaos 바퀴는 매 물리 서브스텝마다 바퀴 위치에서 아래로 레이/스피어캐스트를 쏘고, **맞은 것을 그 바퀴의 지면으로 삼는다.** 채널과 응답은 컴포넌트의 프로퍼티가 그대로 쓰인다 [A · `ChaosWheeledVehicleMovementComponent.cpp`]:

| 곳 | 내용 |
|---|---|
| `:486` · `:497` | 서스펜션 트레이스 발사 — 트레이스 채널 `ECC_WorldDynamic`, **응답은 `WheelTraceCollisionResponses`** |
| `:1142-1143` | 그 기본값 — **`ECC_Vehicle` 만 `Ignore`, 나머지 전부 `Block`** |
| `:1776` | 매 프레임 물리 입력으로 전달(= 런타임에 바꿔도 다음 프레임부터 먹는다) |

즉 **엔진 기본값은 "차량끼리만 안 밟고 나머지는 전부 땅"** 이다. 그리고 **UGV 쪽에는 이를 덮어쓰는 코드가 없었다** — `UUGVWheeledVehicleMovementComponent` 는 물리 스레드 시뮬 객체만 갈아 끼우는 얇은 래퍼였고 프로퍼티는 손대지 않았다.

### 2.2 시체는 그 트레이스에 정확히 걸린다

병사는 사망하면 `USoldierHealthComponent::StartRagdoll` 에서 메시가 이렇게 바뀐다 [A · `SoldierHealth.cpp:648-654`]:

```cpp
Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);   // :650  캡슐은 빠지고
Mesh->SetCollisionObjectType(ECC_PhysicsBody);                  // :652  메시가 물리 바디가 되고
Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);  // :653  쿼리에도 잡히고
Mesh->SetAllBodiesBelowSimulatePhysics(RagdollRootBone, true, true); // :654  pelvis 아래가 시뮬
```

`ECC_PhysicsBody` 는 2.1 표의 "나머지 전부 Block" 에 들어간다. **바퀴의 서스펜션 레이가 시체를 지면으로 읽는다.**

> ★ `QueryAndPhysics` 가 핵심이다. 09-21 에 **살아 있는** 병사 메시는 `QueryOnly` 로 내렸지만(`ai/2026-09-21_game_thread_structural_pool_rays_bridge.md` ③), 래그돌은 물리로 굴러야 하므로 여기서 다시 올라간다 — 즉 **시체만 이 조건을 만족한다.**

### 2.3 그러면 무슨 일이 일어나는가

바퀴 여섯 중 **하나만** 시체 위를 지난다. 그 바퀴에게 지면은 **갑자기 수십 cm 위**로 뛴다 →

1. 서스펜션이 그 높이차를 **압축량**으로 읽는다 → 스프링 힘이 그 한쪽에서만 튄다.
2. 그 힘이 **차체 한쪽 모서리를 들어올린다**(PBD 컨스트레인트로 차체에 직결 — `../vehicle/ugv/2026-09-10_ugv_0901_suspension_tuning.md`).
3. 덤으로 **마찰 계수까지 시체의 물리재질에서 가져온다** — 접지 마찰이 한 바퀴만 딴판이 되면 스키드스티어 차량은 방향이 튄다.
4. 시체는 시체대로 "바퀴가 나를 땅으로 밟는" 동안 물리 접촉으로도 밀려서 **밑에 깔린 채 떨린다** — 관측된 "부들부들"이 여기서도 나온다.

**뒤집힘으로 가는 길이 이것이다.** 목격이 0회인 이유는 여섯 바퀴 × 시체 한 구가 만드는 순간적인 단차라서 대개는 한두 프레임에 지나가기 때문이고, 조건(속도·각도·시체 자세)이 맞으면 언제든 난다.

### 2.4 두 번째 경로 — 이번에 손대지 않음

차체/바퀴 콜리전과 **20개 바디 래그돌의 물리 접촉 자체**는 별개의 이야기다. 시체가 차체와 지면 사이에 끼면 **매 서브스텝 depenetration 이 서로 싸우며** 떨림이 생기고, 그 반력이 차체로 들어간다. 이건 "지면으로 읽는" 문제가 아니라 "두 강체가 낀" 문제라서 **다른 수단**(시체 시뮬 해제·차량 채널에서 시체를 물리적으로도 빼기·미는 힘 상한)이 필요하다 → **[W118]**.

**이번 수정은 1번 경로만 끊는다.** 2번을 안 건드린 것은 의도적이다 — 그게 "밀려나는 그림"을 만드는 바로 그 접촉이기 때문이다(3.1절).

---

## 3. 적용한 수정

### 3.1 차량 쪽 — 한 줄 [A · 코드]

```cpp
// Source/titan_example/Vehicles/UGVWheeledVehicleMovementComponent.cpp:6-22
UUGVWheeledVehicleMovementComponent::UUGVWheeledVehicleMovementComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    WheelTraceCollisionResponses.SetResponse(ECC_PhysicsBody, ECR_Ignore);   // :21
}
```

**서스펜션 트레이스에서만** `ECC_PhysicsBody` 를 뺀다.

- ✅ 끊는 것: **"서스펜션이 시체(와 떨군 총)를 지면으로 계산하는"** 경로 하나.
- ✅ **그대로 두는 것: 물리 접촉.** 차체·바퀴 콜리전은 아무것도 안 바뀌었다 — **시체가 밀려나는 그림은 유지된다.** 사용자가 좋다고 한 부분을 건드리지 않는 것이 이 수정의 범위를 한 줄로 묶은 이유다.

**적용 범위**: `SetDefaultSubobjectClass` 로 이 컴포넌트를 박는 것은 `AUGVWheeledVehiclePawn`(`UGVWheeledVehiclePawn.cpp:10`)뿐이고, `AUGV0901Pawn` 이 그 자식이다 → **UGV 계열만 해당.** 트럭·드론 등 다른 차량은 엔진 기본값 그대로다.

> ⚠ **헤더 주석이 한 발 늦었다**: `UGVWheeledVehicleMovementComponent.h` 상단에 "프로퍼티도 동작도 추가하지 않으므로 `p.UGV.TrackLock.Enabled 0` 이면 엔진 원본과 완전히 동일" 이라고 적혀 있는데, **이제는 생성자가 트레이스 응답을 하나 바꾼다.** 이 한 줄은 cvar 로 끄는 것이 아니다 → **[W119]**(주석 정정, 코드 수정 아님).

### 3.2 병사 쪽 — 사망 시 무기 드롭 [A · 코드]

**왜 같이 하는가**: 손에 용접된 총을 든 채 래그돌이 쓰러지면 "총 맞은 사람"이 아니라 "마네킹이 넘어지는" 그림이 된다(옛 적군 BP 는 떨궜다). 그리고 **떨군 총도 차량 바퀴 밑에 들어간다** — 3.1절의 제외 한 줄이 **시체와 총을 동시에 커버하도록** 오브젝트 타입을 맞춘 것이 이 두 축을 한 문서에 묶는 이유다.

**신설 `UPROPERTY` 3개** [A · `SoldierHealth.h`]:

| 프로퍼티 | 기본값 | 뜻 |
|---|---|---|
| `bDropWeaponOnDeath` | `true` (`:283`) | 사망 시 떨굴 것인가 |
| `DroppedWeaponMassKg` | **3.5** (`:291`) | 떨군 것의 질량 [kg] |
| `WeaponMeshComponentName` | `"WeaponMesh"` (`:302`) | **컴포넌트** 이름(4절 — 이게 본체다) |

**① 떨구는 시점 — 래그돌이 시작되는 바로 그 순간** [A · `SoldierHealth.cpp:661-666`]:

```cpp
// The hand lets go at the same moment it stops being animated, not before
if (bDropWeaponOnDeath) { DropCarriedWeapons(); }
```

`StartRagdoll()` 안, 메시가 시뮬로 넘어간 직후다. **더 일찍(사망 몽타주 중에) 떨구면 화면상 아직 총을 쥐고 있는 손에서 총이 빠진다.** 손이 애니메이션을 놓는 순간과 총을 놓는 순간이 같아야 한다.

**② 공통 처리 `ReleaseAsDebris(UPrimitiveComponent*)`** [A · `SoldierHealth.cpp:756-785`] — 액터로 떨구든 컴포넌트로 떨구든 여기를 지난다:

```cpp
Body->SetCollisionObjectType(ECC_PhysicsBody);                  // :767
Body->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);  // :768

Body->SetCollisionResponseToAllChannels(ECR_Ignore);            // :775  ★ 전부 Ignore 에서 시작
Body->SetCollisionResponseToChannel(ECC_WorldStatic,  ECR_Block); // :776
Body->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block); // :777
Body->SetCollisionResponseToChannel(ECC_PhysicsBody,  ECR_Block); // :778
Body->SetCollisionResponseToChannel(ECC_Vehicle,      ECR_Block); // :779
Body->SetGenerateOverlapEvents(false);                            // :780

Body->SetMassOverrideInKg(NAME_None, FMath::Max(0.1f, DroppedWeaponMassKg), true); // :782
Body->SetSimulatePhysics(true);                                   // :783
Body->SetPhysicsLinearVelocity(DeathVelocity);                    // :784
```

설계 의도 네 가지:

- **오브젝트 타입 `ECC_PhysicsBody` 는 시체와 같은 타입을 일부러 골랐다.** 3.1절의 제외 한 줄이 **시체와 떨군 총을 동시에** 덮게 하려는 것이다. 바퀴 밑의 소총도 지면이면 안 된다.
- **"전부 Ignore 에서 시작해 필요한 넷만 Block"**: 땅(`WorldStatic`)·지형지물(`WorldDynamic`)·다른 시체/총(`PhysicsBody`)·차량(`Vehicle`)에만 부딪친다. **`Sight`/`Cover` 채널을 일부러 뺐다** — 이 AI 는 그 두 채널로 "무엇이 보이는가 / 무엇 뒤에 숨는가"를 정한다. **바닥에 굴러다니는 소총이 시야를 막거나 엄폐물로 계산되면 안 된다.** (기본값을 두고 빼는 대신 0에서 더하는 방향으로 쓴 이유: 채널이 늘어날 때 조용히 새로 Block 되지 않게.)
- **질량 3.5 kg 오버라이드**: 물리에셋의 기본 질량은 소총 크기 hull × 밀도라 실제 소총과 자릿수가 다르다. 안 고치면 **모루처럼 떨어진다.** 3.5 는 실총 무게에서 고른 값이고 **미측정**이다 → [C-171].
- **사망 순간 속도 상속**(`DeathVelocity`): 달리다 죽으면 총도 같이 날아간다.

**③ 8초 뒤 재우기** [A · `SoldierHealth.cpp:814-845`]: `FreezeCorpse`(`CorpseFreezeAfterSeconds` 기본 **8 s**, `SoldierHealth.h:265`)가 이제 떨군 것들도 `PutRigidBodyToSleep()` 한다(`:835` 액터 쪽 · `:842` 컴포넌트 쪽). 시체당 강체 하나가 솔버에 영구히 남는 것을 막는다.

**④ `EndPlay` 정리 목록에 드롭분 추가** [A · `SoldierHealth.cpp:182-190`]: 기존 `bDestroyCarriedActorsOnDestroy` 경로(09-22 재시작 세션이 넣은 P188)는 `GetAttachedActors` + `Owner->Children` 로 대상을 모은다. **떼어내면 `GetAttachedActors` 에 안 잡히므로**, `DroppedWeapons` 목록을 그 집합에 따로 더한다:

```cpp
// And what was let go of on death: dropping detaches it, so neither list above has it any more
for (const TWeakObjectPtr<AActor>& Weak : DroppedWeapons)
    if (AActor* Weapon = Weak.Get(); IsValid(Weapon)) Carried.Add(Weapon);
```

**이게 없으면 떨군 총이 월드에 영구히 남는다** — P188 로 막은 누수가 드롭 기능 때문에 다시 열릴 뻔한 자리다.

---

## 4. ⚠ 함정 — **손에 들린 총은 스폰된 액터가 아니다**

**첫 구현은 "부착 액터를 떼어 떨구기" 하나뿐이었고, 빌드해 보니 총이 오른손에 그대로 붙어 있었다.** 로그는 정상이었다(떨굴 액터를 찾아 떨궜다고 나왔다). 화면만 안 바뀌었다.

확인해 보니 **화면에 보이는 총과 코드가 떨군 총이 서로 다른 물건**이었다 [A]:

| | 화면에 보이는 총 | 코드가 떨구던 것 |
|---|---|---|
| 무엇 | `BP_SoldierCharacter` **자기 컴포넌트** `WeaponMesh` | BeginPlay 에서 스폰해 붙이는 **액터** `BP_AR4Rifle` |
| 메시 | `SK_KA74U_X` | `SK_AR4_X` |
| 쓰임 | **보이는 총** | 총구 소켓 · FX 앵커 |
| 물리에셋 | `SK_KA74U_X_Physics` | `SK_AR4_X_Physics` |

둘 다 물리에셋이 있으므로 **둘 다 떨굴 수는 있다.** 문제는 **어느 쪽이 본체인가**였다.

**해결 — 컴포넌트 경로가 본체, 액터 경로는 보조** [A · `SoldierHealth.cpp:741-750`]:

```cpp
if (UPrimitiveComponent* Held = FindHeldWeaponMesh())      // :745
{
    Held->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);  // :747
    ReleaseAsDebris(Held);                                                     // :748
    DroppedMeshes.Add(Held);                                                   // :749
}
```

`FindHeldWeaponMesh()`(`:787-812`)는 소유자의 스켈레탈 메시 중 **캐릭터 본체가 아니고** `WeaponMeshComponentName`(기본 `"WeaponMesh"`)과 이름이 같은 것을 고른다. 이름이 비어 있으면 "본체가 아닌 아무 스켈레탈 메시".

### 4.1 추가 규칙 — **보이지 않는 부착 액터는 떨구지 않는다** [A · `SoldierHealth.cpp:725-732`]

```cpp
if (!Body->IsVisible() || Body->bHiddenInGame) { continue; }
```

병사는 **눈에 안 보이는 보조 액터**를 달고 다닌다(위 표의 `BP_AR4Rifle` 이 바로 그것 — 총구 소켓/FX 용으로 숨겨져 있다). 그것까지 떨구면 **아무도 볼 수 없는 물리 파편이 바닥에 쌓인다.** 안 보이는 것은 시체에 붙은 채로 남아 시체와 함께 파괴된다(3.2절 ④).

> 풀링 투사체도 같은 루프에서 제외된다(`:714` `Carried->IsA<ASoldierProjectile>()`) — 나는 탄은 풀 소유라 떨구는 대상이 아니다.

### 4.2 일반화

**"BP 가 손에 무엇을 들고 있는가"는 코드에서 한 가지 모양이 아니다.** 컴포넌트일 수도, 스폰된 액터일 수도, 둘 다일 수도 있다. `GetAttachedActors` 만 보고 "총을 찾았다"고 하면 **찾긴 찾는데 화면의 총이 아니다.** 판정은 **눈에 보이는가**(`IsVisible`)로 해야 한다 — P130 의 같은 결이다: 코드 구조가 화면을 대신 알려 주지 않는다.

---

## 5. 재시작 시 정리 — 떨군 것은 어디로 가는가 [A · 코드]

사용자 질문("재시작하면 저것들 안 남나?")에 답하기 위해 확인한 것:

**① 재시작은 시체를 포함해 전부 지운다** [A · `ScenarioRespawnSubsystem.cpp:251-265`]:

```cpp
// 1) 지금 살아있는 대표 액터(원본/직전 재스폰본, 시체 포함) 전부 파괴.
for (FActorSnapshot& Snapshot : Snapshots)
    if (AActor* Old = Snapshot.Live.Get())
    {
        DestroyedChildren += DestroyChildActors(*Old);   // :260
        Old->Destroy();                                  // :261
    }
```

**② 떨군 것이 컴포넌트면** — 시체 액터 소유이므로 `Old->Destroy()` 와 **함께 소멸한다.** 떼어낸 것은 부착뿐이고 소유(`Outer`)는 그대로다.

**③ 떨군 것이 액터면** — `USoldierHealthComponent::EndPlay` 의 정리 목록이 잡는다(3.2절 ④). `Destroy()` → `RouteEndPlay` → 컴포넌트 `EndPlay` 순서라 **시체가 사라지는 그 경로에서 같이 처리된다.**

**④ 안전망** — 재시작 쪽 `DestroyChildActors`(`:334-386`)는 **부착(재귀) + Owner 기준**으로 한 번 더 훑는다. 이제는 대개 0을 돌려주지만([`:342-346`] 주석이 그 사실을 적고 있다) 남겨 둔다: 낙하산처럼 병사가 아닌 재스폰 대상, 그리고 `USoldierHealthComponent` 가 없거나 꺼진 경우를 위해서다.

> ⚠ **떨군 액터는 부착이 풀린 상태**라 ④의 `GetAttachedActors` 로는 안 잡힌다. Owner 가 살아 있으면 Owner 루프가 잡지만, **BP 스폰이 Owner 를 안 세웠다면 아무도 못 잡는다** — 그래서 ③(`DroppedWeapons` 명시 목록)이 있어야 한다. 이 둘은 중복이 아니라 서로 다른 구멍을 막는다.

---

## 6. 검증 상태

| 항목 | 상태 |
|---|---|
| **서스펜션 `PhysicsBody` 제외** | **적용·빌드됨.** 뒤집힘 사례가 원래 드물어(목격 0회) **정성 확인만 가능** — 미측정 [C-172] |
| **무기 드롭** | **첫 빌드에서 안 떨어짐** → 원인(4절: 컴포넌트 vs 액터) 파악 후 컴포넌트 경로 추가 → **재빌드·실측 대기 중** |

**확인 로그** — 드롭이 실제로 무엇을 잡았는지는 이 한 줄로 본다 [A · `SoldierHealth.cpp:752-753`]:

```
LogSoldierAI: [Death] <이름> dropped N carried actor(s) and M held mesh(es).
```

읽는 법: **`M` 이 1 이어야 화면의 총이 떨어진 것**이다(4절의 `WeaponMesh`). `N` 은 보이는 부착 액터 수이고, 지금 배선에서는 **0 이 정상**이다(보조 소총은 숨겨져 있어 4.1절에서 걸러진다). `N≥1 M=0` 이면 4절의 그 함정을 다시 밟은 것이다.

### 6.1 손대지 않은 것

- **차체 ↔ 래그돌의 물리 접촉 자체**(끼임·떨림) — 2.4절. 이번 수정과 무관하게 남아 있다 → **[W118]**.
- **`FreezeCorpse` 는 애님만 멈춘다** — `bPauseAnims = true` + `SetComponentTickEnabled(false)`(`:820-821`). **래그돌 바디는 계속 시뮬레이션된다**(주석도 "The bodies keep sleeping in the scene as collision" 이라고 적지만, 실제로 `PutRigidBodyToSleep` 을 부르는 것은 **떨군 것들뿐**이다 — 시체 본체는 아니다). 진짜 sleep/시뮬 해제는 미적용 → **[W118]** 에 같이 묶는다.

---

## 7. 남은 것

### [C] — 측정해야 아는 것

| # | 항목 | 판정 기준 |
|---|---|---|
| **[C-171]** | **떨군 총의 질량 3.5 kg · 마찰 미측정** | `DroppedWeaponMassKg 3.5`(`SoldierHealth.h:291`)는 실총 무게에서 고른 값이고 **떨어지는 모습으로 확인한 적이 없다.** 마찰/반발은 아예 안 건드렸다(물리에셋의 물리재질 그대로). 판정 = 총이 **한두 번 튀고 눕는가**(모루처럼 박히거나 비닐처럼 미끄러지지 않는가), 경사면에서 계속 굴러 내려가지 않는가 |
| **[C-172]** | **서스펜션 제외 이후 실제 거동(뒤집힘 예방 효과)** | 원래 목격 0회라 **"안 난다"를 증명할 수 없다.** 차선책 = 시체 위를 **일부러** 지나게 해서 (a) 차체 롤/피치 스파이크, (b) 한쪽 바퀴 접지 마찰 튐이 사라졌는지 본다. 판정 = 시체를 밟는 프레임에 차체 자세가 **평지 주행과 같은 범위**인가 |
| **[C-173]** | **`ECC_PhysicsBody` 제외가 다른 물리 오브젝트에 주는 영향 미검토** | 제외는 **타입 단위**라 시체·떨군 총 말고도 그 타입인 모든 것이 서스펜션에 안 보인다 — 파편·굴러다니는 소품·추락한 드론 등. **"차가 그 위로 그냥 지나가 버리는" 그림**이 어디서 이상해지는지 아직 안 봤다. 판정 = `ECC_PhysicsBody` 를 쓰는 액터 목록을 뽑아, 바퀴가 **올라타야 정상인** 것이 그 안에 있는지 |

### [W] — 파생 작업

| # | 항목 |
|---|---|
| **[W118]** | **차체 ↔ 래그돌 물리 접촉(끼임·떨림) + 시체 시뮬 정지.** 2.4절 · 6.1절. 서스펜션 경로는 끊었지만 **두 강체가 끼는 문제는 그대로**다. 후보: (a) `FreezeCorpse` 가 시체 바디도 `PutRigidBodyToSleep`/`SetSimulatePhysics(false)` — 단 **밀려나는 그림이 죽는다**(8초 뒤라 대개 무방하나 확인 필요), (b) 차량 콜리전에서 시체를 물리적으로도 빼기 — **밀려나는 그림이 완전히 사라짐**(사용자가 원하는 그림과 반대), (c) 시체 바디 수 감축/미는 힘 상한. **(a)가 유력하나 8초 전에 밟히면 그대로다** — 값과 함께 정해야 한다 |
| **[W119]** | **`UGVWheeledVehicleMovementComponent.h` 상단 주석 정정.** "프로퍼티도 동작도 추가하지 않으므로 `p.UGV.TrackLock.Enabled 0` 이면 엔진 원본과 동일" 이 이제 사실이 아니다(3.1절) — 생성자의 트레이스 응답 한 줄은 cvar 와 무관하게 항상 먹는다. 다음에 그 파일을 열 때 같이 |

---

## 8. 원칙

**P192 — 시체·떨어진 물체는 차량 서스펜션에 지면으로 보이면 안 된다.**

Chaos 서스펜션의 기본 트레이스 응답은 **"차량만 빼고 전부 땅"**(`ChaosWheeledVehicleMovementComponent.cpp:1142-1143`)이다. 게임이 진행되면서 **바닥에 새로 생기는 물리 물체**(래그돌 시체, 떨군 무기, 파편)는 전부 이 기본값에 걸리고, 바퀴 하나가 그것을 지면으로 읽는 순간 **스프링 힘과 접지 마찰이 그 바퀴만 딴판**이 되어 차체로 올라온다 — 뒤집힘·방향 튐의 경로다.

원칙은 **채널을 지면 후보에서 빼되, 물리 접촉은 남기는 것**이다. 이 둘은 다른 축이고 섞으면 안 된다:

- **서스펜션 트레이스에서 뺀다** → "차가 시체를 타고 오르지 않는다".
- **콜리전은 그대로 둔다** → "차가 시체를 밀어낸다"(원하는 그림).

그러려면 **새로 바닥에 떨어지는 것을 같은 오브젝트 타입으로 통일해 두는 것**이 값싸다 — 그래야 차량 쪽 제외가 **한 줄로** 전부를 덮는다(3.2절이 떨군 총을 `ECC_PhysicsBody` 로 떨구는 이유). 새 "바닥에 남는 물체"를 추가할 때(배낭·헬멧·탄피·격추된 드론) **오브젝트 타입부터 정하고**, 차량 밑에 들어갈 수 있으면 이 타입을 쓴다.

---

## 9. 바뀐 파일

| 파일 | 내용 |
|---|---|
| `Source/titan_example/Vehicles/UGVWheeledVehicleMovementComponent.cpp` | 생성자에 `WheelTraceCollisionResponses.SetResponse(ECC_PhysicsBody, ECR_Ignore)`(`:6-22`) — **한 줄 + 근거 주석** |
| `Source/SoldierLab/AI/SoldierHealth.h` | `bDropWeaponOnDeath`(`:283`) · `DroppedWeaponMassKg`(`:291`) · `WeaponMeshComponentName`(`:302`) · `DropCarriedWeapons`(`:417`) · `ReleaseAsDebris`(`:420`) · `FindHeldWeaponMesh`(`:423`) · `DroppedWeapons`(`:472`) / `DroppedMeshes`(`:475`) |
| `Source/SoldierLab/AI/SoldierHealth.cpp` | `StartRagdoll` 에서 드롭 호출(`:661-666`) · `DropCarriedWeapons`(`:698-754`, 보이는 것만 + 컴포넌트 경로) · `ReleaseAsDebris`(`:756-785`) · `FindHeldWeaponMesh`(`:787-812`) · `FreezeCorpse` 재우기(`:824-844`) · `EndPlay` 정리 목록에 드롭분(`:182-190`) |

에셋/레벨 변경 없음. `ScenarioRespawnSubsystem.cpp` 는 **읽기만 했다**(5절 — 기존 동작으로 충분함을 확인).
