# SoldierLab 을 titan_example 로 옮긴다 — 충돌 실사와 절차

2026-09-14 / **✅ 이관 완료** / 디자인팀이 GASP/Lyra 채택을 결정해 `SoldierLab` 을 `titan_example` 본체에 편입한다. 두 프로젝트의 `.uproject` · `Config` · `Content` 를 실측해 충돌 지점을 확정했다. **막을 것으로 예상했던 3건은 충돌이 없다. 폐포 실측 결과 이관 총량은 920 MB 다(4절) — 정리 전 추정 3 GB 는 폴더 크기로 짐작한 것이라 틀렸다.**

관련 항목: [W33] [W34] [W35] [C-98] [Q43] / 관련 문서: `IMPLEMENTED.md` 6절, `weapons/2026-09-12_projectile_port.md` (역방향 이관의 선례)

> 이 문서는 **"무엇이 조용히 깨지는가"** 의 목록이다. 이관 자체는 UE `Migrate` 도구가 하지만,
> 그 도구가 절대 알려주지 않는 것이 `Config` 의 채널 번호와 플러그인이다. 그 둘이 안 맞으면
> **에러 없이** 엄폐 판정이 엉뚱한 것에 맞고 ABP 가 통째로 안 뜬다.

---

## 1. 두 프로젝트 실측 대조 [A]

측정 대상: `C:/working/kadex/anim_test/SoldierLab` · `C:/working/kadex/titan_example`

| | SoldierLab | titan_example | 판정 |
|---|---|---|---|
| 엔진 | 5.8 | **5.8** | ✅ 같다 |
| C++ 모듈 | `SoldierLab`(Runtime) + `SoldierLabEditor`(Editor) | `titan_example`(Runtime) 하나 | ✅ 이름 충돌 없음 |
| 활성 플러그인 | 29종 | 20종 | ⚠ **교집합이 3종뿐** (2절) |
| 커스텀 트레이스 채널 | `GameTraceChannel1~5` (Traversable/Mouse/Obstacle/**Cover**/**Sight**) | **`[/Script/Engine.CollisionProfile]` 절 자체가 없다 — 커스텀 채널 0개** | ✅ **번호 충돌 없음** |
| `PhysicalSurfaces` | SurfaceType1~5 = Wood/Hard/Dirt/Metal/Glass | **SurfaceType1~5 = Wood/Hard/Dirt/Metal/Glass** | ✅ **완전히 같다** |
| `SK_UEFN_Mannequin` | 있음 (아군 작업으로 **2본 추가·GUID 재생성**됨) | **없음** (`Content/Characters` = `Mannequins`, `Soldier` 뿐) | ✅ **덮어쓸 것이 없다** |
| GASP 기반 | 전체 보유 | **PSD/CHT/PSN 0건 — GASP 흔적 없음** | ⚠ 전부 새로 들어간다 (4절) |
| `soldier_T` | `/Game/SoldierLab/Characters/Ally/soldier_T` (스켈레톤 = `SK_UEFN_Mannequin`) | `/Game/Soldiers/New_Soldiers/soldier_T` (스켈레톤 = `soldier_T_Skeleton`) | ⚠ **경로가 달라 공존한다** (5절) |

### 1.1 걱정했는데 아니었던 것 3건 — 왜 아니었나

**① 트레이스 채널 번호 충돌.** 가장 위험하다고 본 항목이었다. 우리가 `GameTraceChannel4`=Cover, `5`=Sight 를 쓰는데 titan 이 같은 번호를 다른 이름으로 쓰고 있으면, **채널은 번호로 저장되고 이름은 표시용일 뿐이라** 병사의 엄폐 판정이 통째로 엉뚱한 물체에 맞는다. 에러는 나지 않는다.
실측: `titan_example/Config` 전체에 `GameTraceChannel` 문자열이 **0건**, `[/Script/Engine.CollisionProfile]` 절 자체가 없다. 커스텀 채널을 한 번도 안 만든 프로젝트다. 1~5번이 전부 비어 있다.

**② `PhysicalSurfaces` 번호 충돌.** 투사체의 재질별 명중 효과(`PM_Wood` 등 5종)가 여기 걸려 있다. 실측 결과 **다섯 줄이 글자까지 동일**하다. 당연한데, `weapons/2026-09-12_projectile_port.md` 의 투사체가 원래 titan 에서 온 것이라 그 설정을 같이 가져왔기 때문이다. **역방향 이관이 남긴 이득이다.**

**③ `SK_UEFN_Mannequin` GUID 충돌.** 아군 작업에서 이 스켈레톤에 `thigh_twist_02_l/r` 2본이 추가되고 GUID 가 재생성됐다. titan 에 같은 이름 에셋이 있으면 덮어쓰기가 되고, 그쪽 클립이 전부 DDC 재압축 대상이 된다. 실측: **titan 에 `UEFN` 문자열을 가진 에셋이 0건.** 덮어쓸 것이 없다.

---

## 2. 플러그인 — 16종을 켜야 한다 [A]

titan 에 없고 SoldierLab 이 쓰는 것:

```
PoseSearch                  ← Motion Matching. 없으면 PSD/PSN 이 로드 불가
MotionTrajectory            ← MM 의 궤적 입력
AnimationWarping            ← Orientation/Stride 워핑
AnimationLocomotionLibrary
AnimationLayering
Chooser                     ← CHT_Soldier_* 데이터베이스 선택
Locomotor
CurveExpression
MotionWarping
SmartObjects                ← ST_Soldier_SmartObject
GameplayBehaviorSmartObjects
GameplayInteractions
DrawDebugLibrary
AnimationBudgetAllocator    ← 45명 성능 목표에서 쓸 것
Mover / ChaosMover / NetworkPrediction / MoverExamples / MovieSceneAnimMixer
RigLogic / HairStrands / LiveLink / LiveLinkControlRig
```

두 프로젝트에 **이미 공통인 것은 `ModelContextProtocol` · `ModelingToolsEditorMode` · `EditorToolset` 3종뿐**이다.

- `Mover` 계열은 GASP 샘플이 켜 둔 것이고 **우리 병사는 `SandboxCharacter_CMC`(CharacterMovementComponent) 계열**이라 실제로 필요한지 확인 후 정한다 → **[W34]**. 다만 GASP 콘텐츠 안에 `SandboxCharacter_Mover` 가 참조로 얽혀 있으면 켜야 로드된다.
- `RigLogic`/`HairStrands`/`LiveLink` 는 GASP 의 MetaHuman·Echo 샘플용이다. 4절에서 그 콘텐츠를 빼면 **불필요하다.**

> ⚠ 플러그인이 빠지면 에셋이 "로드 실패"가 아니라 **조용히 껍데기로** 열린다.
> 선례: `weapons/2026-09-12_projectile_port.md` — 반대 방향으로 가져온 `BP_RifleProjectile`
> 이 부모 클래스(`/Script/titan_example.RCWSProjectile`)가 없어 **12.8 KB 껍데기**로 왔다.
> 같은 일이 ABP·PSD 에서 일어나면 발견이 훨씬 늦다.

---

## 3. C++ 모듈 — **완료 (2026-09-14)** · 합쳐질 때만 드러난 충돌 3건 [A]

```
SoldierLab/Source/SoldierLab/        →  titan_example/Source/SoldierLab/        (29 파일)
SoldierLab/Source/SoldierLabEditor/  →  titan_example/Source/SoldierLabEditor/  ( 9 파일)
```

`Target.cs` 는 **옮기지 않았다** — titan 의 기존 두 타깃에 모듈 이름만 더했다.

| 파일 | 변경 |
|---|---|
| `titan_example.uproject` | `Modules` 에 `SoldierLab`(Runtime) · `SoldierLabEditor`(Editor) 추가 |
| `titan_example.Target.cs` | `ExtraModuleNames += "SoldierLab"` |
| `titan_exampleEditor.Target.cs` | `ExtraModuleNames += "SoldierLab", "SoldierLabEditor"` |
| `Source/SoldierLab/SoldierLab.cpp` | **`IMPLEMENT_PRIMARY_GAME_MODULE` → `IMPLEMENT_MODULE`** |
| `Source/SoldierLab/Weapons/SoldierProjectile.h/.cpp` | 타입 2개 · 콘솔 변수 1개 개명 |
| `Config/DefaultEngine.ini` | `[CoreRedirects]` 2줄 추가 |

- `titan_example` 모듈의 `Build.cs` 는 **건드리지 않았다.** `Niagara` · `NavigationSystem` 은 이미 그 목록에 있었고, 어차피 우리 모듈의 `SoldierLab.Build.cs` 가 자기 것을 따로 선언한다.
- **모듈 이름 `SoldierLab` 은 고정이다.** 블루프린트가 부모 클래스를 `/Script/SoldierLab.SoldierProjectile` 처럼 **모듈 이름으로** 참조하므로, 바꾸면 `BP_RifleProjectile` 등이 부모를 잃고 껍데기가 된다(`weapons/2026-09-12_projectile_port.md` 의 12.8 KB 껍데기와 같은 사고).

### 3.1 충돌 3건 — **각각으로는 멀쩡했고 합쳐야 드러난다** [A]

이관 전에 두 모듈의 심볼을 대조해서 찾았다. 셋 다 **빌드하기 전에** 잡혔다.

| # | 충돌 | 왜 생겼나 | 조치 |
|---|---|---|---|
| **X1** | `IMPLEMENT_PRIMARY_GAME_MODULE` 이 둘 | `SoldierLab` 은 **자기 프로젝트의 주 게임 모듈**이었다. 한 타깃에 primary 는 하나뿐 | `IMPLEMENT_MODULE` 로 내림 |
| **X2** | `UENUM EImpactSurfaceCategory` · `USTRUCT FImpactSurfaceEffectSet` 이 둘 | 우리 투사체가 titan 의 `ARCWSProjectile` 에서 **이식된 것**이라 타입 이름까지 따라왔다. UHT 의 스크립트 타입 이름은 **모듈이 달라도 전역에서 유일**해야 한다 | `ESoldierImpactSurface` · `FSoldierImpactEffectSet` 로 개명 + **`[CoreRedirects]`** |
| **X3** | 콘솔 변수 `p.RCWS.ImpactDebug` 이 둘 | 동. 같은 이름을 두 번 등록하면 **모듈 로드 시점에** 터진다 — 즉 7절 검증 **1번**에서 바로 걸린다 | `SoldierLab.Debug.Impact` 로 개명 (나머지 cvar 와 접두사 통일) |

**X2 는 개명만 하면 안 된다.** 이미 저장된 블루프린트의 프로퍼티(`BP_RifleProjectile` 의 `SurfaceImpactEffects` 등)가 타입을 잃으므로 `[CoreRedirects]` 로 이어줘야 한다. `OldName` 에 **패키지 접두(`/Script/SoldierLab.`)를 붙여야** titan 쪽 동명 타입과 섞이지 않는다.

> **P99** — **두 프로젝트를 합칠 때는 빌드하기 전에 심볼을 대조한다.** 이식으로 생긴 코드는
> **원본과 이름을 공유한 채로** 돌아오고, 그 이름들은 각자의 프로젝트 안에서는 완벽히 정상이다.
> 대조 대상은 셋 — ① `IMPLEMENT_PRIMARY_GAME_MODULE` ② `UCLASS`/`USTRUCT`/`UENUM` 이름
> ③ 콘솔 변수 이름. 이번에는 셋 다 실제로 걸렸고, 빌드를 돌렸다면 ①②는 컴파일 에러로,
> ③은 **에디터가 뜨지 않는 것**으로 나타났을 것이다.

### 3.2 확인해 둔 것 (충돌 없음)

- 우리 두 모듈의 `#include` 는 **전부 엔진 헤더 아니면 자기 모듈 것** — titan 헤더 의존 0건
- `UObject` 타입 이름 대조: titan 211개 vs SoldierLab 29개 → X2 해소 후 **충돌 0**
- 콘솔 변수 대조 → X3 해소 후 **충돌 0**. 우리 cvar 12개는 전부 `SoldierLab.Debug.*`
- 클래스 이름: titan 의 `ACoverPoint` 와 우리 `USoldierCoverComponent` 는 별개

### 3.3 남은 일 (사용자)

```
1. .uproject 우클릭 → Generate Visual Studio project files
2. 빌드
3. 에디터 기동 → 7절 검증 1~3번
```

⚠ titan 의 두 타깃은 `IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8` 을 쓰는데 **SoldierLab 의 옛 타깃은 이 줄이 없었다.** 암묵 include 가 줄어드는 설정이라, 우리 코드에서 "전에는 되던" 헤더 누락이 컴파일 에러로 나올 수 있다. 나오면 해당 `#include` 를 명시적으로 추가하면 된다 — 설계 문제가 아니다.

⚠ **백업**: `titan_example.uproject.bak_20260914` · `Source/*.Target.cs.bak_20260914` · `Config/DefaultEngine.ini.bak_20260914`. titan 은 Perforce 라 원본이 읽기 전용이었고 **속성을 해제하고 편집했다** — 정식 체크아웃은 따로 해야 변경이 유실되지 않는다.

---

---

## 3.4 `/Script/LyraGame` 의존 — **들여오지 않는다** (결정) [A]

SoldierLab PIE 가 늘 뱉던 `AN_PlayWeaponMontage` 컴파일 에러(P79 가 "늘 뜨는 것"으로 기록)의 정체는 **Lyra 의 C++ 게임 모듈 의존**이다. 정식 프로젝트에 들어가기 전에 정리한다.

### 무엇인가

`LyraGame` 은 Epic 의 Lyra Starter Game **C++ 게임 모듈**이다. 애니메이션 라이브러리가 아니라 GAS(Gameplay Ability System) 위에 선 **멀티플레이 슈터 프레임워크 한 벌** — 어빌리티·인벤토리·장비·팀·GameFeature 구조 전체다.

2026-09-09 Lyra 라이플 세트를 이식할 때(`animation/prototypes/2026-09-09_lyra_rifle_migration.md`) 우리가 가져온 것은 **애니메이션 에셋뿐**이고 프레임워크는 가져오지 않았다. 그런데 에셋 몇 개가 그 모듈의 클래스를 참조한 채로 따라왔다.

### 실측 [A]

| | 수 |
|---|---|
| `/Script/LyraGame` 을 참조하는 에셋 | **88** |
| 그 중 **이관 폐포에 들어가는 것** | **1** — `Characters/Heroes/.../AN_PlayWeaponMontage` |

나머지 87 은 `Characters/Heroes/` 안의 Lyra 원본(`ABP_Mannequin_Base` · `ABP_ItemAnimLayersBase` · 원본 로코모션 84 · `PhysMat_Player*`)이고 **우리가 아무것도 참조하지 않아 따라오지 않는다.**

쓰이는 Lyra 심볼 전체:

| 심볼 | 쓰는 곳 | 우리 대응물 |
|---|---|---|
| `LyraEquipmentManagerComponent` · `LyraEquipmentInstance` · `LyraWeaponInstance` | `AN_PlayWeaponMontage` | `BP_SoldierCharacter` 가 `BP_AR4Rifle` 을 **직접** 들고 있다 |
| `LyraAnimInstance` | `ABP_Mannequin_Base` · `ABP_ItemAnimLayersBase` | `SoldierCharacter_ABP` (GASP 복제). 미사용 |
| `LyraContextEffects*` | 원본 클립의 발소리 노티파이 | GASP `AC_FoleyEvents`. 미사용 |

### 왜 포함하지 않나

1. **이미 푼 문제다.** Lyra 의 장비 시스템은 "인벤토리에서 현재 장비를 찾아 스폰된 액터를 얻는" 추상화다. 우리 무기는 캐릭터가 직접 들고 있고 `SetWeaponState` 로 탄약·재장전을 이미 보고한다. 남의 인벤토리 추상화를 들여와 **우리 총을 찾을** 이유가 없다.
2. **모듈 하나가 아니다.** `LyraGame` 은 `GameplayAbilities` · `GameplayTags` · `ModularGameplay` · `GameFeatures` · `CommonUI` · `CommonGame` 위에 선다. 넣는다는 것은 **납품 프로젝트의 게임플레이 아키텍처를 GAS 기반으로 바꾸는 결정**이지 컴파일 에러 하나를 고치는 일이 아니다.
3. **넣어도 동작하지 않는다.** 그 노티파이가 하려는 일은 **무기 메시에 몽타주 재생**인데 `SK_AR4_X` 에는 ABP 가 없다. 같은 이유로 `BP_SoldierCharacter` 의 무기 몽타주 노드를 이미 지웠다(Accessed None). Lyra 를 다 들여와도 결과는 "아무 일도 안 일어남"이다.

> **P100** — **없는 모듈을 참조하는 에셋은, 그 모듈을 들여와서 고치는 것이 아니라 참조를
> 끊어서 고친다.** 이식으로 딸려온 참조는 *그 기능이 필요해서* 생긴 것이 아니라 *에셋이
> 그 자리에 있었기 때문에* 생긴 것이다. "에러가 나니 의존을 채워 넣자"는 방향은 프레임워크
> 하나를 통째로 들여오게 만든다 — 되묻어야 할 질문은 **"그 기능을 우리가 쓰고 있었나"** 다.
> 여기서는 쓰고 있지 않았고, 쓸 수도 없었다.

### 조치

`SoldierLab/Animations/Actions/` 의 **`AM_MM_Rifle_Fire` · `AM_MM_Rifle_Reload` · `AM_MM_Rifle_Reload_Emote_MW`** 세 몽타주의 노티파이 트랙에서 `Play Weapon Montage` 를 제거한다. 그러면 연쇄로 빠지는 것:

- `Characters/Heroes/.../AN_PlayWeaponMontage` (98 KB, 유일한 LyraGame 의존)
- `/Game/Weapons/Rifle/Animations/AM_Weap_Rifle_Fire` · `AM_Weap_Rifle_Reload` (노티파이 인자였던 무기 몽타주)

→ **titan 에 `/Script/LyraGame` 을 참조하는 에셋이 0 개**가 된다.

`Weapons/Rifle/` 과 `Characters/Heroes/` 의 원본은 건드리지 않는다 — 참조가 끊기면 폐포에서 알아서 빠진다.

### ✅ 완료·검증 (2026-09-14) [A]

사용자가 세 몽타주에서 노티파이를 제거했다. 엔진 레지스트리로 폐포를 다시 계산해 대조한 결과:

| 항목 | 전 | 후 |
|---|---|---|
| 폐포 안에서 `/Script/LyraGame` 을 참조하는 에셋 | 1 (`AN_PlayWeaponMontage`) | **0** |
| `/Game/Weapons/` 폐포 진입분 | 4 (`ABP_Weap_Rifle` · `AM_Weap_Rifle_Fire` · `_Reload` · `SK_Rifle`) | **1** (`ABP_Weap_Rifle` 만) |
| 폐포 전체 | 940개 / 922 MB | **934개 / 920 MB** |

`Weapons/Rifle/Animations/` 의 GASP 원본 3개는 여전히 `AN_PlayWeaponMontage` 를 물고 있으나 **우리가 참조하지 않으므로 따라오지 않는다** — 의도한 대로다.

> ⚠ `Characters/Heroes/Mannequin/Animations/AnimModifiers/FootstepEffectTagModifier` 는 **폐포에 있다.**
> 다만 이것은 **`/Script/LyraGame` 을 참조하지 않는다** — P79 가 `AN_PlayWeaponMontage` 와 함께
> 묶어 적었지만 원인이 다르다. PIE 로그에 이쪽 에러가 계속 뜨면 따로 봐야 한다 → **[C-100]**

## 4. 폐포 실측 — **922 MB** ([Q43] 해결 · [C-98] 해결)

> **2026-09-14 갱신.** 아래의 "2.7 GB 를 통째로 넣을 것인가"는 **잘못된 걱정이었다.**
> 에셋 정리([W36] 캐릭터 목록 비우기 + 미사용 삭제) 후 엔진 레지스트리로 **재귀 폐포를
> 실측**한 결과, 실제로 따라오는 것은 **922 MB** 다. `Migrate` 는 폴더가 아니라 **참조된 것만**
> 가져간다 — `UEFN_Mannequin/Animations` 2.7 GB 중 걸리는 것은 **524 MB** 뿐이다.

| 묶음 | 크기 | 에셋 수 |
|---|---|---|
| `Content/SoldierLab/` | **318 MB** | 377 |
| `Characters/UEFN_Mannequin/` | **524 MB** | 400 (전체 2.7 GB 중) |
| `Characters/Heroes/` | 39.9 MB | 43 |
| `NiagaraExamples/` | ~29 MB | 48 |
| GASP `Blueprints/` | ~8 MB | 37 |
| `Audio` · `Levels/LevelPrototyping` · `Gun_effect` · 기타 | ~3 MB | 28 |
| **합계** | **920 MB** | **934** |

- **Echo · Paragon · MetaHumans · UE4_Mannequin · IsolatedExamples 는 폐포에서 완전히 빠졌다** — [W36]의 `GM_SoldierLab` 캐릭터 목록을 비운 결과다. 정리 전이었다면 여기에 약 2.5 GB 가 더 붙었다.
- `Heroes` 39.9 MB 는 **전부** 남겨 둔 `AM_MM_Dash_*` · `Death_*` · `HitReact_*` 몽타주에서 온다. 지우면 그만큼 빠지지만 **922 MB 에서 882 MB 가 되는 차이**라 급하지 않다.
- `UEFN_Mannequin` 524 MB 는 비무장 로코모션 중 `CHT_Soldier_Databases` 가 실제로 고르는 것들이다. **이것을 더 줄이려면 ABP 에서 비무장 분기를 끊어야 하고, 그 순간 GASP 원본과 갈라진다.** 922 MB 는 납품 프로젝트가 감당할 수 있는 크기이므로 **자르지 않는다.**

### 4.1 옛 기술 (2026-09-14 오전, 정리 전 추정)

~~titan 에 GASP 가 전혀 없으므로 기반 전체가 새로 들어간다. `Content/SoldierLab/` 347 MB 에
`Characters/UEFN_Mannequin/Animations/` 2.7 GB 가 붙을 수 있다. **[Q43] 3 GB 를 그냥 얹을
것인가.**~~ → **폐포를 재지 않고 폴더 크기로 추정한 것이 틀렸다.** 폴더 크기는 상한이지
비용이 아니다. **P98**

---

## 4.2 실제 이관 결과 — **예측이 틀렸다** (2026-09-14) [A]

| | 4절 예측 | **실제** |
|---|---|---|
| 이관 시도 | 934개 / 920 MB | **3622개 / 약 5.5 GB** |
| Echo · Paragon · MetaHumans · UE5_Mannequins | "폐포에 없다" | **전부 들어왔다** |

### 4.2a 왜 틀렸나 [A]

4절의 폐포는 `AssetTools.get_dependencies` 를 재귀로 돌려 계산했다. 그 결과가 **Migrate 가 실제로 모으는 집합보다 작다** — 소프트 참조·클래스 참조를 포함하지 않는다.

같은 시점에 바이트 스캔(`/Game/...` 문자열 추출)으로 잰 값은 **3613개 / 5532 MB** 로 실제(3622개)와 거의 일치했는데, **낡은 경로 문자열과 `FName` 베이스 충돌 때문에 과다 보고한다고 판단해 버렸다.** 과다 보고하는 것은 맞지만 그 오차보다 **`get_dependencies` 의 누락이 훨씬 컸다.**

> **P101** — **이관 규모는 `get_dependencies` 재귀로 재면 안 된다.** 그것은 하드 참조만 보고
> **소프트/클래스 참조를 빠뜨린다** — 2026-09-14 에 934개로 예측한 이관이 실제로는 3622개였다.
> 바이트 스캔은 과다 보고하지만 **누락은 하지 않는다.** 두 방법이 4배 어긋나면 **큰 쪽을
> 믿고**, 확정은 **Migrate 확인 대화상자의 파일 목록**으로 한다(그것이 실제로 복사될 집합이다).

### 4.2b 무엇이 끌고 왔나 [A]

```
L_SoldierTest · BP_Soldier_Friendly  →  AC_VisualOverrideManager  ─┐
GM_SoldierLab                        →  PC_Sandbox               ─┴→  GM_Sandbox
                                                                        ├→ BP_Kellan      (MetaHumans 365)
                                                                        ├→ BP_Echo        (Echo 134)
                                                                        └→ ABP_GenericRetarget
                                                                              ├→ Paragon/TwinBlast 198
                                                                              └→ UE4_Mannequin 20
SoldierLab/Rigs/RTG_Lyra_to_UEFN     →  IK_UE5_Mannequin_Retarget  →  UE5_Mannequins 105
SoldierLab/Animations/Actions/*      →  Characters/Heroes 140
```

★ **[W36] 에서 `GM_SoldierLab` 의 캐릭터 목록을 비운 것은 맞았지만 소용이 없었다** — 실제 경로는 우리 게임모드가 아니라 **GASP 원본 `GM_Sandbox`** 였고, 거기로 가는 길이 `AC_VisualOverrideManager` 와 `PC_Sandbox` **둘**이다. 하나만 끊으면 다른 쪽으로 그대로 들어온다(시뮬레이션 확인: VOM 만 끊으면 3613 → 3612).

### 4.2c 그래도 이관 자체는 성공했다 [A]

| | 수 |
|---|---|
| 성공 | **3465** |
| 실패 | 157 |
| **그 중 `/Game/SoldierLab/`** | **377 성공 / 0 실패** |

실패 157건은 **둘 다 무해하다**:

- **143 — "destination file is read only"**: titan 이 **이미 갖고 있는** 에셋이고 Perforce 읽기 전용이라 덮어쓰기를 거부한 것이다(NiagaraExamples · FPS_Weapon_Bundle · Gun_effect · Characters/Soldier · EvolveStudio). 143개 전부 **양쪽 파일 크기가 같다** — MD5 만 다른 것은 패키지 헤더의 저장 메타데이터 차이다. **누락 0건.**
- **14 — "the package didn't contain an asset"**: `/Game/Audio/Modulation/*` 12개(오디오 모듈레이션 버스)와 `Characters/Heroes/PhysMat_Player*` 2개. 후자는 **Lyra 물리 머티리얼 클래스**라 원본 프로젝트에서도 로드가 안 되던 것이다(3.4절과 같은 뿌리).

### 4.2d 정리하면 얼마나 줄어드나 (시뮬레이션) [B]

| 조치 | 폐포 |
|---|---|
| 현재 | 3613개 / **5532 MB** |
| `GM_Sandbox` 로 가는 길 끊기 | 2887개 / **3331 MB** (−2.2 GB) |
| + `RTG_*` 2개 제거 | −331 MB |
| + Heroes 몽타주(Dash/Death/HitReact 등) 삭제 | −537 MB |
| **남는 것** | 약 **2.5 GB** — 대부분 `UEFN_Mannequin` 1874 MB(GASP 로코모션, 실제로 필요) |

**끊는 순서가 중요하다**: 참조를 먼저 제거하고 그 다음에 폴더를 지운다. 반대로 하면 `AC_VisualOverrideManager` · `PC_Sandbox` 에 끊어진 참조가 남는다.

⚠ `PC_Sandbox` 는 GASP 플레이어 컨트롤러로 **카메라·입력이 걸려 있을 수 있다.** 떼기 전에 우리가 실제로 무엇을 쓰는지 확인할 것 → **[C-101]**

---

## 4.3 이관 후 정리 — **2.0 GB 제거 완료** (2026-09-14) [A]

4.2d 의 시뮬레이션대로 **편집 2건**으로 `GM_Sandbox` 를 폐포에서 끊었다. `Content` 폴더를 지운 것이 아니라 **참조를 끊은 것**이다(P100).

### 무엇을 했나

| # | 대상 | 변경 | 근거 |
|---|---|---|---|
| ① | `GM_SoldierLab` | `PlayerControllerClass` : `PC_Sandbox` → `PlayerController` | `PC_Sandbox` 의 부모는 **순수 `PlayerController`** 이고 내용은 `NextPawn`(캐릭터 순환) · `NextVisualOverride`(외형 순환) · 모바일 가상 조이스틱 숨김뿐. 나머지 입력 이벤트는 **전부 빈 이벤트**. ★ **`IMC_Sandbox` 는 `SandboxCharacter_CMC` 와 `BP_ObserverPawn` 이 붙인다 — PC 가 아니다.** 그래서 입력이 죽지 않는다 |
| ② | `AC_VisualOverrideManager` `FindAndApplyVisualOverride` | `CastToGM_Sandbox` · `GetVisualOverrides_Soft` · `SetVisualOverridesList` · `GetGameMode` 4노드 삭제 후 진입 exec 재연결 | **이미 죽은 코드였다** — `GM_SoldierLab` 의 부모가 `GameModeBase` 라 `GM_Sandbox` 로의 캐스트가 **런타임에 항상 실패**한다. 하는 일 없이 `GM_Sandbox` 를 하드 참조로 붙들어 2 GB 를 끌고 왔을 뿐 |

② 후 남은 함수 본문:
```
(fn FindAndApplyVisualOverride ()
  (bind _returnvalue (|GetVisualOverrideWithCVAR_Soft (Variables|Default|GetVisualOverridesList)))
  (CallFunction|LoadVisualOverride _returnvalue))
```
→ `AC_VisualOverrideManager` 의 의존은 `BFL_HelpfulFunctions` 하나만 남았다.

> ⚠ `AC_VisualOverrideManager` **컴포넌트 자체는 뗄 수 없다.** `BP_SoldierCharacter` 의 부모가
> `SandboxCharacter_CMC` 이고(**복제본이 아니라 자식 클래스다** — 이전 문서의 "복제" 표기는
> 부정확했다) 그 부모가 가진 컴포넌트이기 때문이다. 그래서 컴포넌트는 두고 **안쪽 캐스트만**
> 끊었다.

### 결과 [A]

| | 이관 직후 | ①② 후 |
|---|---|---|
| 폐포 | 3613개 / **5532 MB** | 2865개 / **3511 MB** |

폐포에서 빠진 것: `GM_Sandbox` · `PC_Sandbox` · `BP_Echo` · `BP_Kellan` · Echo · Paragon · MetaHumans · UE4_Mannequin · RetargetedCharacters.

### 남은 것과 그 이유

| 폴더 | 크기 | 왜 남나 |
|---|---|---|
| `Characters/UEFN_Mannequin` | 1874 MB | **GASP 로코모션. 실제로 쓴다** |
| `Characters/Heroes` | 526 MB | 사용자가 남기기로 한 Dash/Death/HitReact 몽타주 |
| `Characters/UE5_Mannequins` | 331 MB | `RTG_Lyra_to_UEFN`(리타깃 도구)가 물고 있다. 사용자가 남기기로 함 |
| 기타 | ~780 MB | Tutorial/Blueprints · Soldiers/New_Soldiers · Weapons/Rifle · LevelPrototyping 등 |

### titan 에서 지워도 되는 것 [A]

titan `Content` 13,063개를 전수 스캔해 **서로 말고는 참조하는 것이 없음**을 확인했다:

```
Characters/Echo · Characters/Paragon · MetaHumans
Characters/UE4_Mannequin · Blueprints/RetargetedCharacters
Blueprints/GM_Sandbox · Blueprints/PC_Sandbox
```
지운 뒤 **Fix Up Redirectors**(P96).

> ⚠ **`SandboxCharacter_Mover` 는 지울 수 없다** — GASP 초이서 두 개
> (`CHT_MoverCharacterAnimations_PoseMatch` · `CHT_PoseSearchDatabases_Relaxed`, 둘 다
> `UEFN_Mannequin` 안)가 참조한다. 따라서 **`Mover`/`ChaosMover`/`NetworkPrediction`/
> `MoverExamples`/`MovieSceneAnimMixer` 플러그인 5개는 켜 둬야 한다.** ([W34] 정정)

---

## 4.4 최종 상태 — **이관 완료** (2026-09-14) [A]

`titan_example` 에서 에디터 기동 · PIE · `L_SoldierTest` 실행 **모두 확인**. 이 PC 의 `SoldierLab` 프로젝트에서의 작업은 여기서 끝난다.

### 폐포 추이

| 단계 | 에셋 | 용량 |
|---|---|---|
| Migrate 직후 | 3613 | 5532 MB |
| 참조 정리 후(4.3 ①②) | 2865 | 3511 MB |
| **폴더 삭제 후 (최종)** | **2865** | **3510 MB** |

| 구성 | 에셋 | 용량 | 비고 |
|---|---|---|---|
| `Content/SoldierLab/` | 377 | 320 MB | 우리가 만든 것 전부 |
| `Characters/UEFN_Mannequin` | 1583 | 1874 MB | GASP 로코모션 — **실제로 쓴다** |
| `Characters/Heroes` | 103 | 526 MB | 남겨 둔 Dash/Death/HitReact 몽타주 |
| `Characters/UE5_Mannequins` | 105 | 331 MB | `RTG_Lyra_to_UEFN`(리타깃 도구)가 물고 있다 |
| 기타 | 697 | ~459 MB | Tutorial · Soldiers/New_Soldiers · Weapons/Rifle · LevelPrototyping · NiagaraExamples 등 |

### 플러그인 대조 [A]

`titan_example` **42개 활성** / `SoldierLab` 29개. **SoldierLab 이 쓰는 것은 4개만 빼고 전부 켜져 있고, 그 4개는 전부 불필요하다.**

| titan 에 없는 것 | 판정 |
|---|---|
| `RigLogic` | MetaHuman 얼굴용 — **MetaHumans 를 지웠으므로 불필요** |
| `HairStrands` | Echo 머리카락용 — **Echo 를 지웠으므로 불필요** |
| `LiveLink` · `LiveLinkControlRig` | 모션캡처 입력. 미사용 |

★ **`Mover` 계열 5개(`Mover`/`ChaosMover`/`NetworkPrediction`/`MoverExamples`/`MovieSceneAnimMixer`)는 켜 둬야 한다** — GASP 초이서 두 개(`CHT_MoverCharacterAnimations_PoseMatch` · `CHT_PoseSearchDatabases_Relaxed`, 둘 다 `UEFN_Mannequin` 안)가 `SandboxCharacter_Mover` 를 참조한다.

**이름이 비슷해서 잘못 켰던 것 1건** — 21개를 수작업으로 켜는 과정에서 나왔다:

| 잘못 켠 것 | 켜려던 것 |
|---|---|
| `MovieScenePoseSearchTracks` (Sequencer 의 PoseSearch 트랙 저작용, 미사용) | `MovieSceneAnimMixer` |

→ 제거하고 `MovieSceneAnimMixer` · `GameplayInsights` 를 추가했다(2026-09-14, 에디터 종료 상태에서 `.uproject` 직접 편집). 나머지 20개는 정확했다.

> **P102** — **플러그인을 이름으로 수작업 검색해 켤 때는 켠 목록을 원본과 대조한다.**
> UE 에는 `MovieSceneAnimMixer` / `MovieScenePoseSearchTracks`, `StateTree` /
> `GameplayStateTree` / `StateTreeToolset` 처럼 **접두가 겹치는 형제 플러그인**이 많다.
> 잘못 켜도 **에러가 나지 않는다** — 그냥 안 쓰는 모듈이 하나 더 로드될 뿐이라 발견되지 않는다.
> 대조는 `.uproject` 두 개의 `Plugins` 집합 차집합으로 끝난다.

### 남은 자잘한 것

- `Characters/Echo/Rigs/CR_Echo_Helpers` · `CR_Echo_Twist` — **참조 0건**. 지우면 800 KB 회수
- 리다이렉터 4개 중 2개(`Vehicles/UGV/*`)는 titan 원래 것. `Fix Up Redirectors` 로 정리

### 이관하지 *않은* 것

`SoldierLab` 프로젝트 쪽에만 남는 것 — 필요하면 개별로 다시 가져온다.

- `SoldierLab/Animations/Actions/` 의 미사용 몽타주 일부, `Rifle/_MF/` · `_Extra/`(사용자가 남긴 것은 따라갔다)
- `Saved/closure_*.txt`(이 문서의 측정 산출물)
- `Config/DefaultInput.ini`(Perforce 읽기 전용, 한 번도 수정 안 함 — 매핑은 `IMC_Sandbox` 안에 있다)

---

## 5. `soldier_T` 가 둘이 된다 [A]

| | 경로 | 스켈레톤 | 쓰는 곳 |
|---|---|---|---|
| titan 기존 | `/Game/Soldiers/New_Soldiers/soldier_T` | `soldier_T_Skeleton` (87본, T-포즈) | `ABP_Ally_kadex_T` · `BP_Ally_kadex` |
| 이관본 | `/Game/SoldierLab/Characters/Ally/soldier_T` | **`SK_UEFN_Mannequin`** | `BP_Soldier_Friendly` |

**경로가 달라 공존하고 서로 간섭하지 않는다.** titan 의 기존 병사 시스템은 그대로 돈다.

이것은 의도한 결과다 — **1차 이관에서 titan 의 기존 병사 자산을 하나도 건드리지 않는다.** 나란히 놓고 비교할 수 있어야 교체 판단이 가능하고, 되돌릴 수도 있어야 한다.

다만 **이름이 같은 에셋이 둘이라 혼동의 소지**가 있다. 디자인팀에 넘길 때 반드시 경로로 구분해서 말할 것 → **[W33]**.

고아 에셋 `soldier_T_Skeleton`(이관본 쪽, 참조 0건)은 **이관 전에** 지운다 — 옮기고 나면 titan 의 동명 에셋과 헷갈린다. ([W29] 와 같은 건)

---

## 6. 절차

### 6.0 이관 전 (SoldierLab 쪽에서)

```
1. 고아 에셋 정리 — soldier_T_Skeleton(참조 0건, [W29]) 삭제
2. 백업 에셋 정리 판단 — SoldierCharacter_ABP_BAK_0911 · BP_SoldierCharacter_BAK_0911b
                        · CHT_Soldier_Databases_BAK · PSD_Soldier_Walk_Test
                        (남길지 정한다. 남기면 titan 에 백업본이 영구히 산다)
3. 미사용 애니메이션 확인 — Rifle/_MF/ 39개가 Lyra 여성 마네킹 변종이면 참조 0건일 것 → [C-98]
                            Actions/MM_* 중 Pistol/Shotgun/Death/HitReact 계열
4. 계측 잔해 게이트 — BP_SoldierCharacter Tick 의 DrawDebugCoordinateSystem(분기 밖!)
                     + PrintString 다수 [W6]. 45명이 전부 그린다
5. 레벨 저장 · 전체 저장
```

### 6.1 Config (도구가 절대 안 해주는 부분 — **먼저** 한다)

`titan_example/Config/DefaultEngine.ini` 에 추가:

```ini
[/Script/Engine.CollisionProfile]
+DefaultChannelResponses=(Channel=ECC_GameTraceChannel1,DefaultResponse=ECR_Ignore,bTraceType=True,bStaticObject=False,Name="Traversable")
+DefaultChannelResponses=(Channel=ECC_GameTraceChannel2,DefaultResponse=ECR_Ignore,bTraceType=True,bStaticObject=False,Name="Mouse")
+DefaultChannelResponses=(Channel=ECC_GameTraceChannel3,DefaultResponse=ECR_Ignore,bTraceType=False,bStaticObject=False,Name="Obstacle")
+DefaultChannelResponses=(Channel=ECC_GameTraceChannel4,DefaultResponse=ECR_Block,bTraceType=True,bStaticObject=False,Name="Cover")
+DefaultChannelResponses=(Channel=ECC_GameTraceChannel5,DefaultResponse=ECR_Block,bTraceType=True,bStaticObject=False,Name="Sight")
+Profiles=(Name="TraversalObjectPreset",...)   ; SoldierLab 쪽 원문 그대로
+Profiles=(Name="CharacterCapsule",...)
+Profiles=(Name="ObstaclePreset",...)
```

- **번호를 그대로 가져가야 한다.** 채널은 번호로 저장되고 이름은 표시용이다. 4·5 를 다른 번호로 옮기면 이미 배치된 모든 엄폐물의 응답 설정을 다시 해야 한다.
- `PhysicalSurfaces` 는 **이미 같으므로 건드리지 않는다.**
- `Traversable`(1번)은 GASP 트래버설용이다. 4절에서 트래버설을 안 쓰기로 하면 불필요하지만, 번호를 비워 두는 편이 안전하다.

### 6.2 플러그인

`titan_example.uproject` 의 `Plugins` 에 2절 목록 추가 → **에디터 재시작** → 켜졌는지 확인.

### 6.3 C++ 모듈

```
Source/SoldierLab/       →  titan_example/Source/SoldierLab/
Source/SoldierLabEditor/ →  titan_example/Source/SoldierLabEditor/
```

`titan_example.uproject` 의 `Modules` 에 두 항목 추가 후 **프로젝트 파일 재생성 → 빌드**.
(빌드는 사용자가 한다.)

### 6.4 콘텐츠

에디터에서 `Content/SoldierLab` 폴더 우클릭 → **Migrate** → 대상 `titan_example/Content`.

> **확인 대화상자의 파일 목록을 먼저 읽을 것.** 여기에 참조 폐포가 전부 나온다 —
> Echo/Paragon/MetaHumans 가 목록에 뜨면 무언가가 그것을 참조하고 있다는 뜻이고,
> 그 참조를 먼저 끊어야 한다. **목록을 안 읽고 OK 를 누르면 6 GB 가 들어갈 수 있다.**

`Content/Input/IMC_Sandbox` 와 `Content/Blueprints/` 의 GASP 기반은 폐포에 자동으로 포함된다.

### 6.5 입력 매핑 (수작업)

`IMC_Sandbox` 의 키 매핑은 **`ObjectTools.get_properties` 로 읽히지 않아** 에디터에서 눈으로 확인해야 한다 (`IMPLEMENTED.md` 2.5절). 우리가 추가한 매핑:

| Input Action | 키 |
|---|---|
| `IA_Fire` · `IA_Reload` | (IMC_Sandbox 실물 확인 필요) |
| `IA_Stance` (Axis1D) | **V** 내리기 · **B** 올리기(Negate) |
| `IA_Lean` · `IA_BlindFireH` · `IA_BlindFireV` · `IA_BlindFireReset` | (동) |

> `SoldierLab/Config/DefaultInput.ini` 는 Perforce 읽기전용이라 **한 번도 수정하지 않았다.**
> 매핑은 전부 `IMC_Sandbox` 에셋 안에 있다. 즉 `.ini` 는 옮길 것이 없다.
> 관전 빙의는 이 때문에 Input Action 이 아니라 **키 이벤트 F** 로 직접 물려 있다.

---

## 7. 이관 후 검증 — 순서대로, 앞이 실패하면 뒤는 의미 없다

| # | 확인 | 실패 시 의미 |
|---|---|---|
| 1 | 에디터가 경고 없이 뜬다 | 플러그인 누락 (2절) |
| 2 | `BP_SoldierCharacter` 가 껍데기가 아니다 (컴포넌트 7개 보임) | C++ 모듈 미빌드 (3절) |
| 3 | `SoldierCharacter_ABP` 컴파일 성공 | Chooser/PoseSearch 누락 |
| 4 | `L_SoldierTest` PIE 에서 병사가 **이동**한다 | PSD/PSN 미로드 = MM 실패 |
| 5 | 병사가 **엄폐물 뒤로 간다** | **트레이스 채널 번호** (6.1절) |
| 6 | 사격 시 재질별 명중 이펙트가 맞다 | `PhysicalSurfaces` |
| 7 | `LogAnimation`/`LogSkeletalMesh` 경고 0건 | 스켈레톤/커브 |
| 8 | 45명 배치 시 프레임 | `AnimationBudgetAllocator` · [W6] 계측 잔해 |

✅ **2026-09-14 — 1~4 통과, 에디터·PIE·`L_SoldierTest` 정상.** 5~8 은 실사용에서 계속 볼 것.

**5번이 이 문서의 존재 이유다.** 1~4 가 다 통과해도 5 는 따로 깨질 수 있고, **깨져도 에러가 없다.** 병사가 엄폐물을 무시하고 벌판에 서 있으면 AI 버그로 보이지 AI 가 잘못된 채널을 보고 있다고는 안 보인다.

---

## 8. 이것이 바꾸는 것

| 문서 | 절 | 어떻게 |
|---|---|---|
| `README.md` | 문서 지도 | `migration/` 추가 |
| `CURRENT_STATE.md` | 머리글 · 다음 할 일 | titan_example 편입 결정 · ~~이관 대기~~ → 이관 완료(4.2절 · 10절 정정) |
| `OPEN_ITEMS.md` | [W33] [W34] [W35] [C-98] [Q43] | 아래 |
| `IMPLEMENTED.md` | 6절 | 이관 후 경로가 `/Game/SoldierLab/...` 그대로인지 기록 |

- [ ] 원 문서에 결과 반영 (이관 후)
- [ ] `OPEN_ITEMS.md` 등록
- [ ] `CURRENT_STATE.md` 갱신

---

## 9. 막힌 것 / 다음에 확인할 것

- **[Q43]** GASP 비무장 애니메이션 2.7 GB 를 통째로 넣을 것인가 — 납품 프로젝트의 용량 대 "자르면 GASP 업데이트를 못 받는다" 의 교환. **권장: 1차는 통째로, 절단은 별건**
- **[C-98]** 참조 폐포의 실제 크기 — Migrate 대화상자의 파일 목록으로만 알 수 있다. 비무장 로코모션 DB 가 실제로 참조되는지, `Rifle/_MF/` 39개가 참조 0건인지 여기서 같이 판정된다
- **[W33]** `soldier_T` 동명 에셋 2개 — 디자인팀 안내 시 반드시 경로로 구분
- **[W34]** `Mover`/`ChaosMover`/`NetworkPrediction`/`RigLogic`/`HairStrands`/`LiveLink` 가 실제로 필요한지 — 4절 절단 여부에 따라 달라진다
- **[W35]** [W6] 계측 잔해 게이트 — 45명 전원이 `DrawDebugCoordinateSystem` 을 그린다. 이관 전에 처리하는 편이 낫다 (옮기고 나면 titan 쪽 성능 문제로 보인다)
- **미확인**: `IMC_Sandbox` 의 `IA_Fire`/`IA_Reload`/`IA_Lean`/`IA_BlindFire*` 실제 키 — 에디터에서 눈으로 읽어 6.5절 표를 채울 것
- ⚠ **6.1절이 놓친 Config 2건 (2026-09-14 15:00 발견·수정)**: `[/Script/Engine.DataDrivenConsoleVariableSettings]` 의
  GASP `DDCvar.*` 27줄과 `[/Script/PoseSearch.PoseSearchSettings] AvailabilitiesBufferSize=230`. 빠지면 에러 없이
  **모든 DDCvar 가 0 으로 읽히고** 로그가 매 틱 경고로 도배된다. titan `DefaultEngine.ini` 에 추가함 → [W40].
  또 SoldierLab 원본 프로젝트는 이 PC 의 `C:\working\works\kadex\anim_test\SoldierLab` 에 **아직 있다**(6절 서두의 "`works\` 가 빠졌다"는 이 PC 기준으론 틀림)

---

## 10. 정정 (2026-09-15, 문서 정리 세션)

- **상태**: 헤더는 이미 "✅ 이관 완료" 다(4.2 · 4.4절). 다만 **8절의 표(`CURRENT_STATE.md` 행 "이관 대기")와 체크박스 셋은 이관 *전*에 쓴 채 남아 있었다** — 셋 다 2026-09-14 에 실제로 반영됐다(README 문서 지도 · `CURRENT_STATE.md` "2026-09-14 편입 완료" 블록 · `OPEN_ITEMS.md` [W33][W34][C-98][Q43] 해결 표시 · `IMPLEMENTED.md` 머리글). 8절은 판단 이력으로 두고 여기서 완료 처리한다.
  - [x] 원 문서에 결과 반영 (이관 후) → 4.2 · 4.4절
  - [x] `OPEN_ITEMS.md` 등록 → [W33] [W34] [C-98] [Q43] 해결, [W35] [W38] [W39] [W43] [W44] [W45] 는 열림
  - [x] `CURRENT_STATE.md` 갱신 → "★ 2026-09-14 — `titan_example` 편입 완료" 절
- **[W36] 관련 후속**: 4.3절이 "`GM_SoldierLab` 의 캐릭터 목록을 비웠다"고 적은 것은 **실제로 안 비워져 있었다** — `PawnClasses_Soft` 에 `[BP_SoldierCharacter, SandboxCharacter_CMC, SandboxCharacter_Mover]` 가 2026-09-15 까지 남아 있었고, GASP GM 은 `GetDefaultPawnClassForController` 오버라이드로 그 0번을 스폰한다(`CLAUDE.md` P126). 이관 폐포 절단 자체는 `AC_VisualOverrideManager` · `PC_Sandbox` 절단으로 이미 성립했으므로 용량 결과(3511 MB)는 그대로다. 사용자가 09-15 에 고침(방식 미확인 [C]). `OPEN_ITEMS.md` [W36] 정정 참고.
- **[W34] 관련 후속**: "Mover 계열 5개는 끌 수 없다(GASP 초이서 2개가 `SandboxCharacter_Mover` 참조)" 에 더해, `/MoverExamples/Characters/Mannequins/Rigs/CR_Mannequin_Body` 가 에디터 기동마다 컴파일 에러를 낸다. `/Game` 쪽 참조는 `/Game/NewLevelSequence`([W38]) 하나. 처분은 **[Q48]** (미결정).
