# 이관 전 에셋 정리 — 참조 전수 스캔 결과

2026-09-14 / **스캔 완료 · 정리 미착수** / `Content/` 전체 `.uasset`/`.umap` 을 바이트 단위로 훑어 패키지 참조를 추출하고, ① 지울 것 ② 이름이 애매한 것 ③ `SoldierLab/` 밖에 남은 우리 것 ④ 이동 완료 후 남은 빈 폴더를 확정했다. **`GM_SoldierLab` 의 참조 하나가 약 2.8 GB 를 끌고 온다.**

관련 항목: [W36] [W37] [W38] [W39] [C-99] / 관련 문서: `migration/2026-09-14_titan_example_migration.md` 4절 · 6.0절

---

## 0. 방법과 그 한계 — 먼저 읽을 것

`.uasset` 은 에디터 포맷이라 압축되지 않는다. 파일 전체에서 `/Game/...` 문자열을 ASCII·UTF-16LE 양쪽으로 뽑아 참조 그래프를 만들었다. 에디터를 열지 않고 전수 조사가 되는 유일한 방법이다.

**두 가지 오차가 있고, 둘 다 실제로 물렸다:**

**① `FName` 숫자 접미 — 거짓 고아를 만든다 [A]**
UE 의 `FName` 은 `Foo_90` 을 **베이스 `Foo` + 숫자 90** 으로 쪼개 저장한다. 그래서 이름이 `_숫자` 로 끝나는 에셋은 참조하는 쪽 파일에 **온전한 이름이 없다.**

```
PSD_Rifle_Stand_TurnInPlace 안의 문자열  →  ".../MM_Rifle_TurnLeft"   ← _90 이 없다
실제 참조 대상                            →  ".../MM_Rifle_TurnLeft_90"
```

보정 전 1차 스캔은 `MM_Rifle_TurnLeft_90` · `_180` · `Crouch_Turn*` **8개를 고아로 잘못 잡았다.** 전부 PSD 가 쓰는 살아 있는 클립이다. 후행 `_숫자` 를 떼고 다시 대조해서 바로잡았다.

> **P95** — 참조 스캔 결과에서 **이름이 `_숫자` 로 끝나는 항목은 따로 검증한다.** `FName` 이
> 숫자 접미를 분리 저장하므로 문자열 대조로는 보이지 않는다. 이 오차는 **"지워도 된다"**
> 쪽으로 틀리므로 조용히 자산을 잃는다.

**② 낡은 경로 문자열 — 거짓 의존을 만든다 [A]**
에셋을 옮겨도 **이전 경로가 name table 에 남는다.** 예를 들어 `Ally/Materials/Ch15_body` 안에는 아직 `/Game/Soldiers/New_Soldiers/Ch15_body` 가 들어 있는데, 그 폴더는 **지금 비어 있다.** 대상 파일의 존재 여부로 갈라야 살아 있는 의존과 구분된다.

**따라서 이 문서의 "참조 0건"은 [B]다.** 소프트 참조(문자열로 들고 런타임에 로드), C++ 에서의 하드코딩 경로, `Config/*.ini` 에서의 참조는 이 스캔에 안 잡힌다. **지우기 전에 에디터의 Reference Viewer 로 한 번 더 확인한다.**

---

## 1. 제일 큰 것 — `GM_SoldierLab` 이 2.8 GB 를 끌고 온다 [A]

```
/Game/SoldierLab/Blueprints/GM_SoldierLab
    -> /Game/Blueprints/RetargetedCharacters/BP_Echo
    -> /Game/Blueprints/RetargetedCharacters/BP_Manny
    -> /Game/Blueprints/RetargetedCharacters/BP_Quinn
    -> /Game/Blueprints/RetargetedCharacters/BP_Twinblast
    -> /Game/Blueprints/RetargetedCharacters/BP_UE4_Mannequin
-> /Game/MetaHumans/Kellan
```

GASP 샌드박스의 **"캐릭터 바꿔가며 보기"** 목록이다. `GM_Sandbox` 를 복제할 때 딸려 왔고, 우리는 병사만 쓴다.

이 배열 하나가 참조 폐포로 끌고 오는 것:

| 폴더 | 크기 |
|---|---|
| `Characters/Echo` | 1.1 GB |
| `Characters/Paragon` (Twinblast) | 751 MB |
| `Characters/UE5_Mannequins` (Manny/Quinn) | 350 MB |
| `MetaHumans/Kellan` | 301 MB |
| `Characters/UE4_Mannequin` | 17 MB |
| **합계** | **≈ 2.5 GB** |

**→ `GM_SoldierLab` 의 캐릭터 목록 배열을 비우는 것이 이번 정리에서 가장 큰 한 수다.** 관전/빙의는 `GM_SoldierObserver` 가 하고, 그쪽은 이 목록을 안 쓴다.

같은 성격으로 **`M_RifleTracer` → `/Game/EvolveStudio/MasterAssets`** 가 하나 걸려 있다. 트레이서 머티리얼이 남의 마스터 머티리얼을 상속한다 → 플랫하게 만들지 판단 ([W39]).

---

## 2. `SoldierLab/` 밖에 남은 우리 것 — 이동이 덜 끝났다 [A]

**살아 있는 참조**만 추린 것이다(낡은 문자열 제외).

| # | 남아 있는 것 | 참조원 | 조치 |
|---|---|---|---|
| **M1** | `/Game/Characters/Soldier/Mat/Mat_Soldier` · `Mat_soldier2` | **`SoldierLab/Characters/Ally/soldier_T`** 의 머티리얼 슬롯 | `Ally/Materials/` 로 이동 |
| **M2** | `/Game/Characters/Soldier/Mat/soldier_re_*` 텍스처 6개 | 위 머티리얼 | **`Ally/Materials/` 에 동명 중복본이 이미 있다.** 어느 쪽이 쓰이는지 확인 후 한쪽 제거 |
| **M3** | `/Game/Characters/Soldier/Rifle_Aiming_Idle` + `_Skeleton` + `_PhysicsAsset` | `Enemy_Skeleton`, `SK_AR4_X` | **옛 Mixamo 병사 메시.** 38 MB 폴더의 본체. 이관 대상 아님 — 참조를 끊고 두고 간다 |
| **M4** | `/Game/Vehicles/UGV_OLD/M_RCWSRound` | **`SoldierLab/Weapons/Blueprints/BP_RifleProjectile`** | titan RCWS 투사체 이식 잔재. `SoldierLab/Effects/Tracer/` 로 이동 |
| **M5** | `/Game/Gun_effect/Decal_Bullet/Demo/Materials/M_Decal_Bullet` | `BP_RifleProjectile` (탄흔 데칼) | `SoldierLab/Effects/Impact/` 로 이동 |
| **M6** | `/Game/Gun_effect/Audio/WavFiles/...` 4건 | `MS_hit_rifle_enemy` 등 | `SoldierLab/Effects/Audio/Soundwaves/` 로 이동 |
| **M7** | `/Game/FPS_Weapon_Bundle/Weapons/Materials/AR4/M_AR4` · `Ammunition/M_762x39_Empty` | `SK_AR4_X` · `SM_AR4_*` | `SoldierLab/Weapons/Meshes/Materials/` 로 이동 — **그 폴더가 지금 비어 있다**(4절) |
| **M8** | `/Game/EvolveStudio/MasterAssets/...` | `M_RifleTracer` | 1절 참고 |
| **M9** | `/Game/NewLevelSequence` | **`L_SoldierTest`** | 루트에 떠 있는 `NewLevelSequence`/`1`/`2` 중 하나를 레벨이 물고 있다. **쓰레기일 가능성이 높다** — 레벨에서 끊고 3개 다 삭제 |

### 2.1 폴더가 엇갈린 것 — 아군 메시가 적군 폴더를 쓴다

```
SoldierLab/Characters/Ally/soldier_T
    -> SoldierLab/Characters/Enemy/Materials/Eyelashes/Ch_49_eyelashes   ← 적군 폴더
```

아군 메시의 속눈썹 머티리얼이 **적군 폴더**에 있다. 적군을 새로 받아 `Characters/Enemy/` 를 통째로 갈아엎을 때 **아군이 같이 깨진다.** 공용이면 `Characters/Shared/` 로 올린다 → **[W37]**

---

## 2.5 외부 의존 전수 감사 — 이펙트·무기 (2026-09-14, 엔진 레지스트리 [A])

`AssetTools.get_dependencies` 를 **재귀 폐포**로 돌려 `SoldierLab/Effects` · `SoldierLab/Weapons` 가 밖으로 닿는 것을 전부 뽑고, 각 대상의 `get_referencers` 로 **우리 전용인지 공유인지** 갈랐다. 0절의 문자열 스캔이 아니라 **엔진 레지스트리**라 이쪽이 권위 있다.

**외부 대상 63개**: NiagaraExamples 49 · Gun_effect 10 · FPS_Weapon_Bundle 2 · EvolveStudio 1 · Vehicles 1.

### 2.5a 먼저 알아야 할 것 — 시스템을 복사해도 의존은 안 끊긴다 [A]

`SoldierLab/Effects/Muzzle/NS_MuzzleFlash` 와 `NiagaraExamples/.../NS_MuzzleFlash` 의 의존성 목록이 **완전히 동일하다**(양쪽 110개, 그중 NiagaraExamples 13개).

**나이아가라 시스템을 복제해도 그 안의 이미터·머티리얼·메시는 원본 자산을 그대로 가리킨다.** 이미터는 상속(inheritance)이고 머티리얼/메시는 참조다. 따라서 "복사본으로 바꾸면 의존이 끊긴다"는 **틀렸다.** 끊으려면 이미터를 **Remove Parent Emitter** 로 지역화하고, 머티리얼·메시·텍스처를 따로 가져와야 한다.

> **P97** — **에셋을 복제하는 것과 의존을 끊는 것은 다른 일이다.** 컨테이너(나이아가라 시스템 ·
> 머티리얼 인스턴스 · 블루프린트)를 복사하면 **껍데기만 우리 것이 되고 내용물은 남의 것을
> 계속 가리킨다.** 폐포를 재귀로 확인하기 전에는 "우리 폴더로 가져왔다"고 말할 수 없다.

### 2.5b 우리 전용 — **그냥 옮기면 된다** (37개)

밖에서 참조하는 것이 **하나도 없다.** 이동해도 아무것도 안 깨진다.

| 묶음 | 개수 | 내용 |
|---|---|---|
| 충격 이미터 | 4 | `NE_Impact_LightDecal` · `_MeshAndSprite` · `_SecondarySprite` · `_Sprite` |
| 충격 다이내믹 인풋 | 2 | `BurstVector` · `RelativeOffsetFloat` (NiagaraScript) |
| 충격 머티리얼 인스턴스 | 6 | `MI_BulletHole` · `MI_BulletHole_Glass` · `MI_ImpactFlash` · `MI_SimpleDebris` · `MI_SimpleDebris_Translucent` · `MI_SmokePuffLight_8x8` |
| 파편 메시 | 11 | `SM_GlassShard_01~03` · `S_Concrete_Rubble_*` 3 · `S_Pine_Bark_Piece_*` 2 · `S_Rock_shopk` · `S_Tree_Debris_*` · `S_Wood_Debris_*` |
| 충격 텍스처 | 3 | `T_Smoke_Wispy` · `T_Smoke_Wispy_Normals` · `CutoutMask` |
| `Gun_effect` 사운드 | 9 | `WAV_Shotgun_ImpactBody01~04` · `WAV_hit_damage_1~5` |
| 기타 | 3 | `Gun_effect/.../M_Decal_Bullet` · `EvolveStudio/.../T_BulletTrace` · `Vehicles/UGV_OLD/M_RCWSRound` |

**충격(impact) 계열은 통째로 우리 것으로 가져올 수 있다** — 샘플 팩의 다른 어떤 것도 이것들을 쓰지 않는다. 이것이 이 감사의 가장 쓸모 있는 결과다.

### 2.5c 공유 — **복제해야 한다** (그리고 그만한 값어치인지 판단)

| 대상 | 누구와 공유 |
|---|---|
| `NE_MuzzleFlash_Base` · `_Smoke` · `_Sparks_Base` · `NE_BulletShells` | `NiagaraExamples/.../NS_MuzzleFlash` |
| `MI_Flipbook_Pyro_Muzzle` · `MI_Flipbook_Smoke_Muzzle` · `MI_MuzzleFlash_Sphere` · `MI_Stylized_Sparkes` · `MI_BulletShell_FX` · `SM_BulletShell` · `T_ThinSmoke_FX` · `MI_Sparks` | 동 |
| `NET_Gameplay_Burst` (NiagaraEffectType) | 동 |
| `M_BrightCore` · `M_SmokeAndFire_Sprites` (마스터 머티리얼) | NiagaraExamples 내부 다른 MI들 |
| `T_SmokePuffLight_EOO_Loop` · `_Normals_Loop` | 동 |
| **총소리·재장전 4개** (`assault_rifle_gunshot_01` · `01~03_assault_rifle_reload_*`) | ★ **`/Game/Soldiers/Weapons/BP_EnemyRifle`** — titan 기존 적군 소총 |
| `M_AR4` · `M_762x39_Empty` | `FPS_Weapon_Bundle` 자신의 `SK_AR4`·`SM_AR4` |

> ★ **`BP_EnemyRifle` 은 titan 의 기존 자산인데 우리 `SK_AR4_X` 도 참조하고 있다.**
> 즉 **titan 과 SoldierLab 이 이미 서로를 물고 있다.** 이관 후 어느 쪽을 정리하든 상대가 깨진다
> — 이관 전에 이 얽힘을 끊을지 말지 정해야 한다 → **[W43]**

### 2.5d 판단 — 할 값어치가 있는가

`NiagaraExamples` 전체가 **62 MB** 다. [W36]의 2.5 GB 에 비하면 작다. **크기 때문이라면 할 일이 아니다.**

할 이유가 있다면 그것은 **소유권**이다 — 납품물이 엔진 샘플 팩 에셋을 참조하면 유지보수와 라이선스 정리가 남의 폴더에 묶인다. 그 판단이면:

1. **2.5b 37개를 옮긴다** — 위험 0, 이것만으로 의존이 절반 이하로 준다
2. 머즐 계열은 **이미터 4개를 Remove Parent Emitter 로 지역화** 후 머티리얼 7개 복제·재지정
3. 사운드 4개는 복제 (BP_EnemyRifle 때문에 이동 불가)

⚠ **이동은 리다이렉터를 남긴다.** 옮긴 뒤 반드시 `Fix Up Redirectors` — 안 하면 리다이렉터가 이관 폐포에 따라간다(P96).

### 2.5e 이미 한 것

- `BP_AR4Rifle` 의 `Niagara|SpawnSystemAttached` → `SystemTemplate` 핀을
  `/Game/NiagaraExamples/.../NS_MuzzleFlash` → **`/Game/SoldierLab/Effects/Muzzle/NS_MuzzleFlash`** 로 교체, 컴파일·저장 완료.
  ⚠ **이것만으로는 의존이 안 줄었다**(2.5a). 우리가 소유한 시스템을 쓰게 된 것이고, 2.5c 의 지역화가 그 위에 얹혀야 실제로 끊긴다.
  부수 효과로 `SoldierLab/Effects/Muzzle/NS_MuzzleFlash` 가 **참조 0건에서 벗어나** 3.5절 삭제 대상에서 빠졌다.


---

## 3. 지울 것 [B] — 참조 0건, 합계 약 44 MB

### 3.1 백업본 3개 (4.5 MB) — 이관 전에 반드시

```
SoldierLab/Animation/SoldierCharacter_ABP_BAK_0911        4172 K
SoldierLab/Blueprints/BP_SoldierCharacter_BAK_0911b        289 K
SoldierLab/PoseSearch/CHT_Soldier_Databases_BAK              4 K
```

옮기면 **titan 에 영구히 산다.** 남의 프로젝트에 남기는 백업은 백업이 아니라 쓰레기다.

### 3.2 라이플이 아닌 무기 · 죽음 · 피격 (약 12 MB) — **Heroes 641 MB 를 끊는다**

참조 0건이면서 **`/Game/Characters/Heroes/Mannequin/Animations/Actions/` 를 물고 있는** 것들이다. 지우면 Heroes 의 대형 애니메이션 의존이 같이 끊긴다.

| 묶음 | 개수 | 내용 |
|---|---|---|
| `AM_MM_Death_*` | 6 | 사망 몽타주 |
| `AM_MM_HitReact_*` | 13 | 피격 리액션 몽타주 |
| `AM_MM_Dash_*` | 5 | 대시 |
| `MM_Pistol_*` | 8 | 권총 시퀀스 |
| `MM_Shotgun_*` | 4 | 산탄총 시퀀스 |
| `AM_MM_Pistol_Melee` · `Pistol_Spawn` · `Shotgun_Melee` | 3 | |
| `MM_Rifle_Spawn` · `_Fast` · `_Turn180` | 3 | 스폰 연출 |
| `AM_MM_Rifle_Equip` · `Melee` · `GrenadeToss` · `DryFire` · `Reload_Emote_MW` · `Generic_Unequip` | 6 | 쓰지 않는 라이플 동작 |

> ⚠ **`AM_MM_Rifle_Fire` 와 `AM_MM_Rifle_Reload` 는 쓰인다. 지우지 말 것.**
> 다만 이 둘도 `Heroes/Abilities/AN_Melee` · `AN_Reload` (애님노티파이 BP) 를 물고 있어서
> **Heroes 폴더 자체는 폐포에서 완전히는 안 빠진다.** 빠지는 것은 대형 애니메이션 쪽이다.

### 3.3 `_MF/` 39개 (약 21 MB) — Lyra 여성 마네킹 변종

`MF_Rifle_*` 전부 참조 0건. Lyra 가 남녀 두 벌(`MM_` = Manny Male, `MF_` = Manny Female)을 배포하는데, 우리는 `MM_` 만 쓴다. **[C-98] 이 여기서 해결된다 — 미사용이 맞다.**

`Mf_Rifle_IdleBreak_Fidget` 하나만 **대소문자가 틀려 있다**(`Mf_` ↔ `MF_`). 어차피 지울 것이지만 기록해 둔다.

### 3.4 `_Extra/` 12개 (약 3.5 MB) — 점프·크라우치 전환

`MM_Rifle_Jump_*` 6, `MM_Rifle_Jog_Lean_*` 3, `MM_Rifle_Crouch_Entry`/`Exit`, `MM_Rifle_Jog_Fwd_RAW`. 전부 참조 0건. **병사는 점프하지 않는다.**

### 3.5 시험·기각·원본 잔재

```
SoldierLab/Animations/BS_Rifle_AO_Stand_Test          29 K   블렌드스페이스 시험용
SoldierLab/Animations/Rifle/AO_Rifle_Aim              13 K   쓰이지 않는 AO
SoldierLab/Animations/Rifle/Poses/MM_Rifle_LowReady  909 K   저작했다 기각·원복 ([C-90])
SoldierLab/Animations/Rifle/Poses/MM_Rifle_*_OverridePose  234 K (2개)
SoldierLab/PoseSearch/PSD_Soldier_Walk_Test                 초기 MM 시험 DB
SoldierLab/Animations/Rifle_Aiming_Idle_Anim                Mixamo 잔재 (루트에 떠 있음)
SoldierLab/Animations/Turning_Right_90_Degrees_Anim   862 K  동
SoldierLab/Animations/Walking_Anim                          동
SoldierLab/Source/Mixamo/*                            ~1.1 MB  임포트 원본 8개
SoldierLab/Source/CONTROL_GASP_Walk_Loop_F                  컨트롤리그 시험
SoldierLab/Characters/Ally/soldier_T_Skeleton          26 K   고아 ([W29])
SoldierLab/Characters/Ally/Materials/Ch_49_body        70 K   참조 0 — 중복본
SoldierLab/Weapons/Meshes/SM_AR4_X · SM_AR4_Mag · SM_AR4_Mag_Empty  831 K
        스태틱 메시판. 우리는 SK_AR4_X 를 쓴다 (참조 8건)
```

> `MM_Rifle_LowReady` 는 **지우기 전에 [C-90] 을 먼저 판단한다.** 총내림 자세를 다시 손볼
> 생각이 있으면 남긴다. 기각 사유가 "걷기에서 왼손·상체 어긋남"이었고 그 원인이 아직
> 살아 있으므로, 지우면 그 시도의 흔적이 사라진다.

### 3.6 지우면 안 되는 "참조 0건" — 오탐

| 에셋 | 왜 남기나 |
|---|---|
| `Levels/L_SoldierTest` | 레벨 자체. 루트다 |
| `PhysicsMaterials/PM_*` 5개 | **Config / C++ 에서 SurfaceType 로 참조**된다. 스캔에 안 잡힌다 |
| `Rigs/RTG_Lyra_to_UEFN` · `RTG_Mixamo_to_UEFN` · `IK_LyraManny` · `IK_Mixamo` | 리타깃 에디터 도구. 런타임 참조가 없는 게 정상이고, **지우면 Lyra 세트를 다시 리타깃할 수단을 잃는다** |
| `Blueprints/GM_SoldierLab` · `GM_SoldierObserver` | 게임모드. `DefaultEngine.ini` 에서 지정된다 |
| `AI/AIC_Soldier` · `ST_*` · `STT_*` | BP/레벨에서 클래스 참조 |

### 3.7 확인이 필요한 것 — 물리 머티리얼이 레벨에 안 걸려 있다 [B]

`PhysicsMaterials/example_mat/M_dirt` · `M_glass` · `M_hard` · `M_metal` · `M_wood` 5개가 **참조 0건**이고, `L_SoldierTest` 도 `PM_*` 을 하나도 참조하지 않는다.

`BP_RifleProjectile` 은 `MS_hit_rifle_dirt` / `_glass` / `_hard` / `_metal` / `_wood` 와 `NS_Rifle_*` 를 재질별로 들고 있는데, **레벨의 어떤 표면에도 물리 머티리얼이 안 붙어 있으면 전부 기본값으로 떨어진다.** 즉 재질별 명중 효과가 지금 실제로 동작하는지 불확실하다.

**→ [C-99] PIE 에서 나무/금속/유리에 쏴서 소리와 파티클이 갈리는지 확인.** 안 갈리면 `example_mat` 의 5개 머티리얼을 레벨 블록에 입히거나, 엄폐물 머티리얼에 `PM_*` 을 지정해야 한다. `migration` 검증 6번 항목이 이것이다.

---

## 4. 이동 완료 후 남은 빈 폴더 [A]

```
Content/SFX                              ← SoldierLab/Effects/Audio 로 이동 완료
Content/VFX/Rifle                        ← SoldierLab/Effects 로 이동 완료
Content/Soldiers/Effects                 ← 이동 완료
Content/Soldiers/New_Soldiers            ← SoldierLab/Characters/Ally 로 이동 완료
Content/SoldierLab/Weapons/Meshes/Materials   ← 비어 있다. M7 이 여기 들어와야 한다
Content/Tutorial/Blueprints/Enemy/Tex_Mat     ← 적군 머티리얼 이동 완료 (GASP 튜토리얼 폴더)
Content/Movies/LookAtPOI                 ← uasset 없음
Content/Collections                      ← 엔진 관리. 두면 된다
Content/Developers/insung52/Collections  ← 동
```

앞의 6개는 지운다. 빈 폴더는 UE 에디터에서 자동으로 안 사라지고, **옮기고 나면 titan 쪽에 이유 없는 빈 폴더로 남는다.**

---

## 5. 이름이 애매한 것 [B]

### 5.1 반드시 바꿔야 하는 것

| 지금 | 문제 | 제안 |
|---|---|---|
| `SoldierLab/Characters/Ally/soldier_T` | **titan 에 `/Game/Soldiers/New_Soldiers/soldier_T` 가 이미 있다.** 스켈레톤이 서로 다른 완전히 별개 에셋인데 이름이 같다 | `SKM_Ally_Soldier` |
| `SoldierLab/Animation/` ↔ `SoldierLab/Animations/` | **한 글자 차이의 형제 폴더.** 앞은 ABP·Chooser, 뒤는 시퀀스 | `Animation/` → `AnimBP/` |
| `SoldierLab/Source/` | UE 에서 `Source` 는 **C++ 소스 디렉터리**를 뜻한다. 실제 내용은 임포트 원본 | `_Import/` (3.5절대로 지우면 사라진다) |
| `SoldierLab/Characters/Enemy/Enemy` · `Enemy_Skeleton` · `Enemy_PhysicsAsset` | titan 에 `BP_Enemy_Base` · `ABP_Enemy_*` 가 있어 검색이 뒤섞인다 | `SKM_Hostile_Soldier` 등 |

### 5.2 관례에 안 맞는 것 (판단)

| 지금 | 비고 |
|---|---|
| `SoldierCharacter_ABP` | UE 관례는 접두 `ABP_`. GASP 원본(`SandboxCharacter_CMC_ABP`)을 따른 것이라 **일관성은 있다.** 바꾸면 문서 전체의 표기와 어긋난다 → **바꾸지 않기를 권함** |
| `Mf_Rifle_IdleBreak_Fidget` | 대소문자 불일치. 3.3 에서 삭제 대상 |
| `BP_AR4Rifle` | `BP_Rifle_AR4` 가 정렬에 유리하나 실익 적음 |
| `Source/SoldierLab/cppdummy.cpp/h` | 모듈 생성 시의 더미. 내용 확인 후 삭제 |

> **이름 변경은 삭제보다 위험하다.** UE 에서 Rename 하면 리다이렉터가 남고, 그 상태로
> Migrate 하면 리다이렉터까지 따라간다. **이름을 바꿀 거면 바꾼 뒤 `Fix Up Redirectors`
> 를 돌리고, 그 다음에 Migrate 한다.** 순서가 뒤집히면 titan 에 리다이렉터가 쌓인다 → **P96**

---

## 6. 권장 순서

```
1. GM_SoldierLab 의 캐릭터 목록 배열 비우기        ← 2.5 GB. 가장 큰 한 수
2. 3.1 백업본 3개 삭제
3. 3.2~3.5 삭제  (Reference Viewer 로 한 번 더 확인하면서)
4. 2절 M1·M4~M7 을 SoldierLab/ 안으로 이동
5. 2.1 아군/적군 공용 머티리얼 정리
6. Fix Up Redirectors  (전체 Content 대상)
7. 5.1 이름 변경 → 다시 Fix Up Redirectors
8. 4절 빈 폴더 삭제
9. [W6] 계측 잔해 게이트
10. 전체 저장 → Migrate
```

**1·2·3 만 해도 이관 크기가 크게 줄고 위험이 거의 없다.** 4 이후는 손이 가고 되돌리기가 번거로우니 시간이 없으면 1~3 + 8 만 한다.

---

## 7. 이것이 바꾸는 것

| 문서 | 절 | 어떻게 |
|---|---|---|
| `migration/2026-09-14_titan_example_migration.md` | 4절 · 6.0절 | 폐포 크기 추정 갱신, [C-98] 해결 |
| `CLAUDE.md` | 5 (P95 · P96) | `FName` 숫자 접미 / 리다이렉터 순서 |
| `OPEN_ITEMS.md` | [W36]~[W39] · [C-99] | 아래 |
| `IMPLEMENTED.md` | 2.5 · 4절 | 삭제·이동 반영 (정리 후) |

- [ ] 원 문서에 결과 반영 (정리 후)
- [ ] `OPEN_ITEMS.md` 등록
- [ ] `CURRENT_STATE.md` 갱신

---

## 8. 막힌 것 / 다음에 확인할 것

- **[C-99]** 재질별 명중 효과가 실제로 동작하는가 — `PM_*` 이 레벨의 어떤 표면에도 안 붙어 있다 (3.7)
- **[W36]** `GM_SoldierLab` 캐릭터 목록 비우기 — 2.5 GB
- **[W37]** 아군 메시가 `Characters/Enemy/` 의 속눈썹 머티리얼을 참조 — 적군 교체 시 아군이 깨진다 (2.1)
- **[W38]** `L_SoldierTest` → `/Game/NewLevelSequence` 참조 — 의도한 것인지 확인 후 끊기
- **[W39]** `M_RifleTracer` → `EvolveStudio` 마스터 머티리얼 상속 — 플랫하게 만들지 판단
- **[C-98] 해결** — `_MF/` 39개는 Lyra 여성 마네킹 변종, 참조 0건이 맞다 (3.3)
- **미확인**: 소프트 참조·C++ 하드코딩 경로는 이 스캔에 안 잡힌다. 삭제 전 Reference Viewer 확인 (0절)
