# 무기 · 투사체 이식 — `ARCWSProjectile` → `ASoldierProjectile`

2026-09-12 / **동작 확인** / `titan_example`의 탄도 투사체를 의존 4종을 끊어 이식하고,
라이플 액터(`BP_AR4Rifle`)와 캐릭터 배선까지 붙여 **실제로 총알이 나간다.**

관련: **[C-81]** · **[C-82]** · **[R7]** · **[W14]~[W17]** / 원칙: **P47~P52**
관련 문서: `IMPLEMENTED.md` 3.1절 · `CLAUDE.md` 5절

---

## 1. 무엇을 왜 가져왔나

`titan_example`의 `ARCWSProjectile`을 `Source/SoldierLab/Weapons/SoldierProjectile.h`(~560줄) ·
`.cpp`(~1050줄)로 이식했다.

**히트스캔이 아니라 진짜 투사체인 것이 요점이다.** 중력이 걸린 포물선 비행을
`UProjectileMovementComponent`가 몬다 — **리드 사격(lead)·탄착 낙차(drop)·비행 시간**이
있어야 교전이 읽히고, 그 셋은 AI 층이 나중에 쓸 물리적 재료이기도 하다.

총구 섬광·발사음·반동은 **여기에 없다.** 그것들은 총구에서 일어나므로 쏘는 쪽(= 무기 액터)의
소유다. 이식 시점에 그 경계를 그대로 유지했다.

---

## 2. 끊어낸 의존 4종 [A]

`titan_example`의 전투 스택(진영 컴포넌트 · 리플리케이션 라우팅 · 바람 액터)은 이 프로젝트에
아직 존재하지 않는다. 네 군데를 끊었고 **네 군데 전부 호출 지점에 "무엇이 있었고 무엇을 되돌리면
복구되는가"를 주석으로 남겼다.**

| # | 원본 | 대체 | 잃은 것 |
|---|---|---|---|
| ① | `UDetectableTargetComponent::Faction` 으로 "살점인가" 판정 | `bHitEnemy = OtherActor && OtherActor->IsA<ACharacter>()` | 진영 구분. **틀려도 대가는 스파크/혈흔 이펙트를 잘못 고르는 것뿐** — 데미지 판정에는 안 쓰인다 |
| ② | `ReportHitToInstigator` → 3분기 Multicast(`URCWSFireControl` / `UAllyFormation` / `UEnemyCombat`) | `PlayImpactEffect(InstigatorActor, ...)` **직접 호출** | 리플리케이션. 세 핸들러가 **전부** `PlayImpactEffect`로 되돌아왔으므로, 방송할 대상이 없는 지금은 그 종착점만 남긴 것과 같다 |
| ③ | `ReportRicochetToInstigator` → 같은 3분기 Multicast(`Multicast_LaunchRicochet`) | `World->SpawnActor<ASoldierProjectile>(...)` + `LaunchFrom(...)` **직접 호출** | 〃. 바운스 카운트와 `ProjectileGravityScale`을 물려주므로 **도탄 캡과 탄도 연속성은 유지**된다 |
| ④ | `AWindSource::GetWindVectorCmsForNiagara` | `SetVectorParameter(FName("WindVectorCms"), FVector::ZeroVector)` | 바람. 나이아가라 파라미터는 그대로 두고 **0을 먹인다** — 바람 소스가 생기면 그 한 줄만 바꾼다 |

> **왜 파라미터를 남겨 두고 0을 먹이는가**: 소비자(나이아가라 그래프)를 건드리지 않으면
> 복구가 한 줄이 된다. 소비자를 고쳐 버리면 복구가 에셋 작업이 된다.

**복구 지점 4개는 [W17]로 등록했다.** 진영/리플리케이션이 생기는 시점에 여기로 돌아온다.

---

## 3. 살아남은 기능 [A]

```
탄도        중력 포물선 (UProjectileMovementComponent)
풀링        LaunchFrom() 로 활성화 / Deactivate() 로 숨기고 전 기능 정지
            ⚠ 풀 자체는 아직 없다 — SoldierLab에 무기 컴포넌트가 없어 발당 SpawnActor 한다.
              LaunchFrom은 어느 쪽이든 같으므로 **풀을 나중에 붙이는 것은 쏘는 쪽의 변경**이다
트레이서     tracerInterval 발마다 (BP_AR4Rifle 기본 3)
도탄        바운스 캡 3 · 재질별 흡수율/전용 사운드
재질별 명중   Wood / Hard / Dirt / Metal / Glass — 이펙트·사운드·데칼·도탄·발화확률이 행 단위
데칼        재질별 + 적 혈흔 + 지면 혈흔. **FIFO 상한** (MaxActiveImpactDecals 200)
지면 혈흔     명중 후 지연 스폰(타이머). 투사체 인스턴스에 의존하지 않아 풀 재사용에 안전
명중 화염     샘플 레이가 맞은 재질의 IgnitionChance로 발화 (맞은 재질이 아니라 **샘플 레이**가 기준)
카메라 셰이크  명중 지점 기준
총알 휘파람   WhizSound · WhizDetectionRadiusCm 200 · WhizBroadPhaseRadiusCm 1500
```

### 3.1 ★ 총알 휘파람이 곧 AI 제압 신호의 자리다 [A]

`Tick()`의 휘즈 블록은 **이번 프레임 이동 선분과 청취자 사이의 최근접 거리**를
`ClosestPointOnSegment`로 구한다. **이것이 제압(suppression) 신호가 필요로 하는 기하 그 자체다** —
"몇 cm 옆으로 지나갔는가".

**지금은 로컬 카메라 하나만 시험하고 소리만 낸다.** 주변 병사로 확장하는 것이 예정된 훅이고,
`WhizBroadPhaseRadiusCm 1500`(선분-점 거리 계산 자체를 생략하는 광역 컬링)이 이미 그 비용 구조를
갖고 있다. → **[W16]**

---

## 4. `Build.cs` — `Niagara`가 링크 타임에 필요하다 [A]

```csharp
PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Niagara" });
```

**에셋 참조 때문이 아니다.** 투사체가 스폰한 나이아가라 컴포넌트에 `SetVectorParameter` /
`SetFloatParameter`로 `User.` 파라미터를 **직접 써 넣기** 때문에 심볼이 필요하다.
참조만이었다면 모듈 의존 없이도 됐다.

---

## 5. `Config/DefaultEngine.ini` [A]

> ⚠ 이 파일은 Perforce 읽기전용 플래그가 걸려 있었다. **사용자의 1회 허가**를 받아 플래그를
> 지우고 편집했고 **`DefaultEngine.ini.bak` 백업이 남아 있다.**
> **P4V 체크아웃은 아직 안 했다** → **[W15]**

### 5.1 `PhysicalSurfaces` — 선언 **순서**가 의미를 갖는다

```ini
[/Script/Engine.PhysicsSettings]
+PhysicalSurfaces=(Type=SurfaceType1,Name="Wood")
+PhysicalSurfaces=(Type=SurfaceType2,Name="Hard")
+PhysicalSurfaces=(Type=SurfaceType3,Name="Dirt")
+PhysicalSurfaces=(Type=SurfaceType4,Name="Metal")
+PhysicalSurfaces=(Type=SurfaceType5,Name="Glass")
```

`titan_example`과 **1:1로 맞췄다.** 이유가 중요하다 —

- 우리 코드(`FindSurfaceEffectSet`)는 **이름**으로 찾는다
- 그런데 **마이그레이션해 온 피지컬 머티리얼 `.uasset`은 인덱스를 저장한다**

즉 **순서를 바꾸거나 중간에 끼워 넣으면 에셋이 조용히 다른 재질이 된다.** 에러도 경고도 없다.
ini에도 같은 주석을 박아 뒀다.

### 5.2 커스텀 트레이스 채널 2종 — `Cover` · `Sight`

```ini
+DefaultChannelResponses=(Channel=ECC_GameTraceChannel4,DefaultResponse=ECR_Block,bTraceType=True,...,Name="Cover")
+DefaultChannelResponses=(Channel=ECC_GameTraceChannel5,DefaultResponse=ECR_Block,bTraceType=True,...,Name="Sight")
```

**한 채널이 아니라 두 채널인 것이 설계다** — 철망·유리는 총알을 막지만 시야는 막지 못하고,
연막은 시야를 막지만 엄폐가 아니며, 수풀은 둘 다 아니다. 한 채널로는 이걸 표현할 수 없다.
([Q21] 결정과 같은 내용)

**지금 파는 이유**: 채널을 나중에 추가하면 **이미 배치된 에셋 전부를 훑어 응답을 지정**해야 한다.
지금은 공짜다.

### 5.3 ⚠ 정정 — "채널 인덱스가 충돌할 것"이라는 예고는 틀렸다

이식 전에 **"두 프로젝트의 콜리전 채널 인덱스가 부딪칠 것"이라고 경고했다.**
실제로 열어 보니 **`titan_example`은 커스텀 채널을 하나도 쓰지 않는다.** 충돌은 없었다.

기존 채널 1~3(`Traversable` / `Mouse` / `Obstacle`)은 전부 **GASP가 원래 갖고 있던 것**이고,
우리는 그 뒤(4·5)에 이어 붙였을 뿐이다.

**교훈**: 확인하지 않은 가정에서 나온 경고는 작업을 왜곡한다 — 받는 쪽은 그것을 근거로
순서를 바꾸거나 우회로를 만든다. **파일을 한 번 여는 것이 경고 한 줄보다 싸다** → **P52**.
(`CLAUDE.md` 3.2절 "추정과 사실을 섞지 않는다"의 실패 사례이기도 하다)

---

## 6. 블루프린트 [A]

### 6.1 `BP_RifleProjectile` — **다시 만들었다**

```
/Game/SoldierLab/Weapons/Blueprints/BP_RifleProjectile     부모 /Script/SoldierLab.SoldierProjectile
```

마이그레이션으로 넘어온 것은 **12.8 KB짜리 망가진 껍데기**였다. 부모가
`/Script/titan_example.RCWSProjectile` — **그 프로젝트에만 있는 C++ 클래스**라서 컴포넌트도
그래프도 전부 소실된 상태였다. **C++ 이식을 끝낸 뒤 0부터 다시 만들었다.**

> **C++ 부모를 가진 BP는 마이그레이션으로 건너오지 않는다.** 에셋은 복사되지만 부모가 없으면
> 내용이 남지 않는다. 순서는 **C++ 먼저, BP 나중**이다.

CDO 배선:

| 슬롯 | 에셋 |
|---|---|
| 탄자 머티리얼 | `M_RCWSRound` |
| 트레이서 | `NS_Rifle_Tracer` |
| 기본 명중 | `NS_Rifle_Dirt` + `MS_hit_rifle_dirt` |
| 적 명중 | `NS_Blood` + `MS_hit_rifle_enemy` |
| 혈흔 데칼 | `MI_Blood` (적 + 지면 혈흔 양쪽) |
| 휘파람 | `MS_bullet_whizz` |
| `surfaceImpactEffects` | **5행** — Wood/Hard/Dirt/Metal/Glass. 행마다 이펙트 + 사운드 + `M_Decal_Bullet` + `MS_Ricochet` |

### 6.2 `BP_AR4Rifle` — 순수 블루프린트

```
/Game/SoldierLab/Weapons/Blueprints/BP_AR4Rifle            부모 /Script/Engine.Actor
```

`Shoot` 이벤트의 실행 순서 —

```
Branch(CanShoot?)  →  Branch(탄약 있음?)
                      → 총구 사운드
                      → 총구 섬광 나이아가라
                      → CanShoot? = false
                      → shotsFiredCount++
                      → SpawnActor BP_RifleProjectile
                      → SoldierProjectile::LaunchFrom
                      → ammoInMag--
                      → CallOnWeaponFired          ← 디스패처. 여기까지 와야 "쐈다"
```

- 탄창이 비면 **자동 `StartReload`**
- `SetTimerByEvent(fireRate)`가 `CanShoot?`를 다시 켠다

기본값:

| 변수 | 값 |
|---|---|
| `fireRate` | 0.12 |
| `muzzleVelocity` | 80000 |
| `tracerInterval` | 3 |
| `magSize` | 30 |
| `bulletSpreadDegrees` | 3 |

> **[C-82]**: 위 값들과 `SoldierProjectile`의 이식된 튜닝값(도탄 캡 3 · 데칼 상한 200 ·
> 휘즈 반경 200/1500)은 **전부 `titan_example`에서 온 숫자**다. 이 프로젝트에서 재보지 않았다.
> P18·P30과 같은 계열의 위험 — **남의 데이터에 대해 옳은 숫자**일 수 있다.

### 6.3 `BP_SoldierCharacter` — `BeginPlay` 추가분

```
BeginPlay
  → 라이플 스폰
  → WeaponMesh 에 부착 (SnapToTarget)
  → 구 GASP 무기 메시를 **컴포넌트 단위로** SetVisibility(false, propagate = false)   ← 6.4 ②
  → SetActorEnableCollision(Rifle, false)                                              ← 6.4 ③
  → Rifle->OwningCharacter = self
  → Rifle 변수에 보관
  → OnReloadStarted  바인드
  → OnWeaponFired    바인드                                                            ← 6.4 ④
```

---

### 6.4 ★ 배선하며 만난 버그 6건 — 이 절이 이 문서의 본체다

#### ① `BP_AR4Rifle`이 컴파일되지 않았다 — **원인이 둘이었다**

- **증상**: 컴파일 실패. 보고된 원인은 `SpawnActor` 노드의 클래스 핀이 `None`
- **실제**: 그것도 맞았지만, **별개로** `LaunchFrom` 노드가 **스테일 상태로 `BP_AR4Rifle`에
  바인드**돼 있었다. 마이그레이션 과정에서 함수의 선언 클래스가 엉뚱한 곳을 가리키게 된 것
- **고친 것**: 노드를 **지우고 다시 만들었다** — `declaring_class = /Script/SoldierLab.SoldierProjectile`
  로 생성하고 **핀 8개를 다시 연결**했다. 핀만 고쳐서는 낫지 않는다
- **교훈** → **P47**. **보고된 원인 하나를 고쳤다고 해서 원인이 하나였던 것은 아니다.**
  컴파일러는 첫 번째 것만 말해 주기도 한다

#### ② 총구 섬광이 안 보였다 — 액터 단위로 숨겼다

- **증상**: 새 라이플의 총구 섬광이 아예 안 나온다
- **원인**: 구 GASP 무기를 `SetActorHiddenInGame(true)`로 숨겼는데, **새 라이플이 쓰는
  `MuzzlePoint`가 그 액터에 달려 있었다.** 액터를 숨기면 부착물까지 전부 숨는다
- **고친 것**: **컴포넌트 단위** `SetVisibility(false, propagate = false)`
- **교훈** → **P48**. **"치운다"는 조작은 가장 좁은 범위로 한다.** 넓게 숨기면 아직 쓰는 것까지
  조용히 딸려 간다

#### ③ 투사체가 안 보였다 — **자기 총열을 맞고 있었다**

- **증상**: 총알이 아예 날아가지 않는다(스폰 직후 사라짐)
- **원인**: 라이플이 **별도 액터**인데 투사체의 무시 목록에 없었다. 발사하자마자 **자기 총열과
  충돌**해 즉시 명중 처리된 것
- **고친 것**: `SetActorEnableCollision(Rifle, false)`
- **교훈** → **P48**(같은 뿌리). **무기를 별도 액터로 분리하는 순간 "자기 자신"의 경계가
  바뀐다** — 가시성도 충돌 무시도 액터 경계에서 끊긴다. 메시 컴포넌트였다면 둘 다 공짜였다

#### ④ 재장전 몽타주가 사격에 잘려 나갔다 — **조건 앞에서 부작용을 냈다**

- **증상**: 재장전 중에 클릭하면 재장전 모션이 끊긴다. 그런데 총알은 안 나간다
- **원인**: `IA_Fire`가 `Shoot()`를 **부르기 전에** `PlayAnimMontage(AM_MM_Rifle_Fire)`를
  **무조건** 재생했다. 재장전 중에는 `Shoot()`이 `CanShoot? = false`로 거절하므로 **총알은
  안 나가는데 몽타주만 재생**되고, 같은 슬롯의 재장전 몽타주를 밀어냈다
- **고친 것**:
  - `IA_Fire.Triggered` → **`Shoot()` 직접 호출**(몽타주 없음)
  - 사격 몽타주를 **라이플의 `OnWeaponFired` 디스패처**로 옮겼다 —
    **탄이 실제로 스폰되고 발사된 뒤에만** 방송되는 지점
- **교훈** → **P49**. 이것이 이 프로젝트의 상시 규칙이 적용된 자리다 —
  **해법은 타이머나 래치가 아니라 인과에서 나와야 한다.** "쏘는 모션"은 "쐈다"의 결과지
  "쏘려 했다"의 결과가 아니다. 그 지점에 옮겨 놓으면 **막을 조건이 필요 없어진다**

#### ⑤ `Accessed None trying to read (real) property CallFunc_GetAnimInstance_ReturnValue_1`, Node: `Montage_Play`

- **증상**: 매 사격·매 재장전마다 런타임 에러
- **원인**: `Montage_Play` 노드 **2개**가 **무기 메시의 `AnimInstance`**를 겨냥하고 있었다
  (`AM_Weap_Rifle_Fire` / `AM_Weap_Rifle_Reload`). GASP의 `SK_Rifle`은 `ABP_Weap_Rifle`을
  달고 있었지만 **메시를 `SK_AR4_X`로 갈아끼웠고 그쪽엔 애님 블루프린트가 없다.**
  `GetAnimInstance`가 **항상 None**을 반환했다
- **고친 것**: `Montage_Play` 2개와 그것을 먹이던 `GetAnimInstance` 2개를 **삭제**했다.
  저 몽타주들은 **이제 없는 스켈레톤을 겨냥한다**
- **교훈** → **P50**. **메시를 갈아끼우면 그 메시의 `AnimInstance`를 겨냥한 호출이 전부 None이
  된다.** `GetAnimInstance`의 None 에러가 나오면 **"그 사이 메시를 바꿨는가"가 원인 목록의
  첫 줄**이다. (볼트·탄창 애니메이션을 되살리려면 `SK_AR4_X`용 무기 ABP를 새로 만들어야 한다)

#### ⑥ 사격 몽타주를 옮기다 **무한 재귀를 만들 뻔했다**

- **증상**: (발현 전에 잡음)
- **원인**: ④에서 몽타주 노드를 `OnWeaponFired` 쪽으로 옮겼는데, **옛 exec 체인의
  `then → Shoot()` 링크가 그대로 살아 있었다.** 그대로 뒀다면
  `OnWeaponFired → 몽타주 → Shoot() → 발사 → OnWeaponFired → ...` **무한 재귀**
- **잡은 방법**: 편집 후 **핀 연결을 되읽어 확인**했다
- **교훈** → **P51**. **노드를 exec 체인 사이에서 옮겼으면 입력 링크와 출력 링크를 *둘 다*
  확인한다.** 새로 이은 쪽만 보면, 옮기기 전 체인의 잔재가 남아 **고리**가 된다

---

## 7. 남은 것

| # | 항목 |
|---|---|
| **[W14]** | `M_Decal_Bullet`(`/Game/Gun_effect/...`) · `M_RCWSRound`(`/Game/Vehicles/UGV_OLD/`)가 **마이그레이션된 경로 그대로** 있다. `SoldierLab/` 아래로 옮길 것 |
| **[W15]** | **P4V 체크아웃 미처리** — `Config/DefaultEngine.ini` · `Source/SoldierLab/SoldierLab.Build.cs` |
| **[W16]** | 휘즈 최근접 판정을 주변 병사로 확장 → **AI 제압 신호 생산자** (3.1절) |
| **[W17]** | 끊어낸 의존 4종의 복원 지점 (2절) |
| **[C-81]** | **디버그 궤적 라인이 보이지 않는다.** 그 외에는 전부 동작한다 |
| **[C-82]** | 이식한 튜닝값이 이 프로젝트에서도 맞는가 (6.2절) |
| **[R7]** | 진영(Faction) 판정의 정식 소스를 무엇으로 둘 것인가 — `bHitEnemy`의 본래 입력 |

---

## 8. 이것이 바꾸는 것

| 문서 | 절 | 어떻게 바뀌나 |
|---|---|---|
| `IMPLEMENTED.md` | 3.1(신설) | 무기 액터 · 투사체 · 명중 반응의 실체 |
| `IMPLEMENTED.md` | 3 | `IA_Fire` 배선이 바뀌었다 — 몽타주가 `OnWeaponFired`로 이동, 무기 메시 `Montage_Play` 2개 삭제 |
| `IMPLEMENTED.md` | 4 | 런타임 모듈에 `Weapons/SoldierProjectile` 추가 |
| `IMPLEMENTED.md` | 6 | "없는 것"에서 사격 반응이 빠지고, 리플리케이션/진영이 들어온다 |
| `CLAUDE.md` | 2 | `weapons/` 폴더 신설 |
| `CLAUDE.md` | 5 | **P47~P52** |
| `OPEN_ITEMS.md` | C·R·W | [C-81] · [C-82] · [R7] · [W14]~[W17] |
| `CURRENT_STATE.md` | 머리말 | 2026-09-12 추가분 (4) |

- [x] 원 문서에 결과 반영
- [x] `OPEN_ITEMS.md` 등록
- [x] `CURRENT_STATE.md` 갱신
