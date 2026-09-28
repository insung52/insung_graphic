# 게임 스레드 묶음 — `stat dumpframe` 로 이름 붙인 카메라 2.0 ms · ABP cvar 폴링 · 숨은 총 메시 · 총구 Niagara 누수 · URO 크래시

2026-09-21 / 완료(카메라·ABP·총구 완료, URO 폐기·되돌림 완료, C++ 4건 빌드·실측 완료 — 후속 10~13절; **13절 순서 ①~⑤ 는 후편 `2026-09-21_game_thread_structural_pool_rays_bridge.md` 에서 실행됐다(14절)**) / 전편([W98]~[W103] 인계표)의 실행 기록. MCP 에 콘솔 툴이 없어 **`stat dumpframe -ms=0.05` 의 로그 블록을 파싱**해 "컴포넌트 틱" 안의 무명 ms 에 틱 함수별 이름을 붙였다 — 그러자 전편이 못 본 **AI 병사 40명 전원의 GASP 카메라 시스템 2.0 ms** 가 나왔고(관전 폰은 병사 카메라를 안 빌린다) BeginPlay 에서 틱만 끄자 World Tick 22.6 → 19.4/18.4. 같은 방법으로 전편의 오독 셋(ABP 2개 · 키네마틱 본 스킵 · WeaponMesh 가 숨는 쪽)을 바로잡았다. ABP 의 cvar 문자열 조회 7개를 1 Hz 로, 숨은 총 메시 `OnlyTickPoseWhenRendered`, 총구 화염은 상주 `MuzzleFlashFX`(발당 `SpawnSystemAttached` 가 `bAutoDestroy=false` 라 8정에 106개 누적되던 누수도 해결). **URO 는 켜자 2분 만에 포즈 NaN 크래시 → 폐기**, `a.URO.Enable=0` 안전장치. C++ 4건(Health/Comms `TickInterval` · HeadAim 조기 반환 · Squad/Zone 스코프 [W103] · `WindSource` 20 Hz)은 빌드 대기.

전편: **`ai/2026-09-21_perf_instrumentation_and_cover_cost.md`**(계측 · Cover 8.92 → 1.92 · 인계표 7절 — **이 문서가 그 표의 실행 기록이고, 그쪽 10절이 이 문서의 정정을 받는다**) · `ai/2026-09-14_cover_frame_fix_and_observer.md`(관전 폰 — 병사 카메라를 안 빌리는 근거) · `ai/2026-09-15_health_hit_death_implementation.md`(피격 부위 = 메시 바디 트레이스 — 키네마틱 본 스킵 불가의 근거).
원칙: **신설 P186**(`CLAUDE.md` 5절 — URO 는 GASP 병사에 켜지 말 것) · P182(재고 → 확인 → 이름 붙은 것만) · P107(GASP GameplayCamera 는 Deactivate 금지). 값 없음(튜닝값 신설 없음 — `CVarPollSeconds 1.0` · `TargetPushIntervalSeconds 0.05` 는 박자). 작업 **[W107]~[W109]**, **[W98] ①② · [W99] · [W100] · [W103]** 갱신, 측정 **[C-165]**.

> 신뢰도: **[A]** = 이 세션의 `stat dumpframe` 로그 파싱(`Saved/Logs/titan_example.log` 의 `LogStats:` 블록) + 엔진/프로젝트 소스 file:line + MCP 되읽기. 조건: `L_SoldierScenario` 적 15 / 아군 20 **PIE**(전편과 동일, 로깅 on) — 일부는 `New_kadex_0811` 40명 PIE(표기). 캡처 사이의 교전 시점이 같지 않아 **±0.3 ms 는 잡음**. URO 크래시 **원인은 [B]**(추정 — 재현 [C-165]).
>
> ⚠ ID 주의: 이 세션이 처음 잡았던 W104~W106 · C-163 · P184 는 **같은 날 분대 세션이 먼저 등록**했다(`OPEN_ITEMS.md` 상단 주석). 이 문서는 **W107~W109 · C-165 · P186** 을 쓴다.

---

## 0. 한 장 요약

| 무엇 | 결과 | 상태 |
|---|---|---|
| **계측법** — `stat dumpframe -ms=0.05` → 로그 `LogStats:` 블록(≈ 3000줄, 스레드별 스탯 계층) 파싱 | "컴포넌트 틱" 안의 ms 에 **틱 함수별(클래스·컴포넌트별) 이름**이 붙는다 — Insights 없이 가장 싼 방법. 이후 전후 비교 전부 이 방법 | [A] · 1절 |
| **전편 오독 정정 셋** | (a) `BlueprintUpdateAnimation` 68 calls 는 ABP 2개가 아니라 **GT 34 + 워커 34**(같은 스탯 스코프) (b) `KinematicBonesUpdateType=SkipAllBones` 는 **채택 불가**(피격 부위 = 바디 트레이스) (c) BeginPlay `SetVisibility(false)` 는 **스폰된 총 액터의 메시**에 걸리고, 보이는 총은 **캐릭터 WeaponMesh** 다 | [A] · 2절 |
| **새로 이름 붙은 비용**(37명, New_kadex_0811) | **GameplayCamera 1.60 + SpringArm 0.38 = 2.0 ms**(AI 40명 전원이 GASP 스탠드얼론 카메라 시스템을 매 틱 평가) · 캐릭터 BP ReceiveTick 2.36 + AC_PreCMCTick 0.94 · CMC 1.86 · CharacterMesh0 1.61 · 숨은 총 메시 0.19+0.34 · `AWindSource` 0.95 · titan TargetDetection ×3 1.2 · PIE 전용 Landscape 5.7/Slate 5/뷰포트 3 | [A] · 3절 |
| **사격 스폰** | `BP_AR4Rifle.Shoot` **발당 ≈ 1.1 ms**(전편 0.65 는 평균) · `SpawnSystemAttached` 가 `bAutoDestroy=false` → **Niagara 컴포넌트 영구 누적**(8정에 106개) | [A] · 3.2절 |
| **수정 ①** 카메라 틱 off(비플레이어) | GameplayCamera/SpringArm 틱 0건 · World Tick **22.6 → 19.4 / 18.4** | ✅ PIE · 4.1절 |
| **수정 ②** ABP cvar 폴링 1 Hz | GT 이벤트 그래프 병사당 29 → 22 µs · CharacterMesh0 틱(아군 20) 0.834 → 0.616 | ✅ PIE · 4.2절 |
| **수정 ③** 숨은 총 메시 `OnlyTickPoseWhenRendered` + 캐릭터 WeaponMesh AnimClass None | 0.26 → 0.07 | ✅ PIE · 4.3절 |
| **수정 ④** 캐릭터 BP 틱 다이어트(`AnimBP` 캐시, Tick 의 캐스트 제거) | 측정 전 | 4.4절 |
| **수정 ⑤** 총구 화염 상주 `MuzzleFlashFX` + `Activate(bReset)` | 누수 해결 · 측정 전 | 4.5절 |
| **수정 ⑥ URO** — `bEnableUpdateRateOptimizations=true` | ⛔ **약 2분 만에 에디터 크래시**(`BonePose.h:645` NaN) → 폐기, `a.URO.Enable=0` 안전장치, 되돌리기 [W107] | ❌ · 5절 · **P186** |
| **수정 ⑦ C++ 4건**(빌드 대기) | Health/Comms `TickInterval 0.1` · HeadAim 조기 반환(0.18 ms/35명) · Squad/Zone 스코프([W103]) · `WindSource` 20 Hz(0.95 → ≈ 0.05) | 빌드 대기 · 6절 |
| **남은 ms**(35명, 카메라 뒤, World Tick ≈ 20) | 사격 스폰 2.2 · 캐릭터 BP 2.8 · SoldierLab 3.5 · 애니 GT 2.6 · CMC 1.8 · 물리 1.4 · 투사체 0.7 · 틱 오버헤드 0.95 … 현실적 바닥 ≈ **13~14 ms** | 7절 |
| **후속(16:55~17:05, 재기동 뒤)** — [W107] 완료 · C++ 4건 실측 · 레이트레이스 소유자별 · 투사체 **영구 누적** 발견 · 남은 후보 순서 ①~⑥ | 카메라 틱 0 · HeadAim/Health/Comms 목록 소멸 · `Shoot` 1.1 → 0.94 · 레이 1,030~1,119회/프레임 2.6~3.0 ms(Engagement 0.46 = 병사 35명 ignore 목록) · `ASoldierProjectile` 은 풀 설계인데 BP 가 발마다 `SpawnActor` → `Deactivate` 뒤 영구 누적 → [W109] 착수 | [A] · **10~13절** |

---

## 1. 계측법 신설 — `stat dumpframe` 로그 파싱 [A]

전편은 `stat game` / `stat anim` 스크린샷이 근거였다. 그 방법의 한계: "컴포넌트 틱" 덩어리 안에서 **어느 컴포넌트**가 얼마인지 안 나온다(전편 5.1절 "이동·트랜스폼 ≈ 4~7" 처럼 범위로만). MCP 에는 콘솔 명령 툴이 없어(메모리 — fps 도 로그로 셌다) Insights 도 못 띄운다.

| 단계 | 무엇 | 비고 |
|---|---|---|
| 1 | 사용자가 PIE 콘솔에 **`stat dumpframe -ms=0.05`** | 그 프레임의 스탯 계층 전체를 한 번 덤프. `-ms` 문턱 아래는 생략 |
| 2 | `Saved/Logs/titan_example.log` 에 **`LogStats:`** 블록 ≈ 3000줄 | 스레드별(GameThread / 워커 / 렌더) 계층 · 각 줄 = 스코프 이름 · incl/excl ms · calls |
| 3 | 파싱 — 스코프 이름에 **클래스·컴포넌트 이름이 들어 있다**(`TickComponent [SpringArmComponent]` · `BP_SoldierCharacter_C ReceiveTick` · `AWindSource Tick` …) | 이걸로 "병사당 틱 함수 25개"(전편 1절)가 **어느 25개**인지, 각각 몇 ms 인지 나온다 |

`stat SoldierLab`(전편) 은 **우리 코드**의 스코프고, 이 방법은 **엔진·BP 를 포함한 전부**다 — 둘이 짝이다. 이후 전후 비교(4절)는 전부 이 블록 두 장을 대조한 것. ⚠ 한 프레임 스냅샷이라 평균이 아니다 — 두 프레임 이상 뜨고, 교전 시점을 맞춘다(P182 규약 그대로).

---

## 2. 전편 오독 정정 셋 [A]

### 2.1 (a) "`BlueprintUpdateAnimation` 68 calls = 병사당 ABP 2개(메인 + PP)" 는 틀렸다

`STAT_BlueprintUpdateAnimation` 은 **두 스레드에서 같은 스코프**로 열린다:

| 어디 | 무엇 | 스레드 |
|---|---|---|
| `Engine/Private/Animation/AnimInstance.cpp:795` | `BlueprintUpdateAnimation`(이벤트 그래프) | **게임 스레드** |
| `Engine/Private/Animation/AnimInstanceProxy.cpp:1354` | `BlueprintThreadSafeUpdateAnimation` | **워커 스레드** |

→ 68 = **34 GT + 34 워커**, ms 는 두 스레드 **합산**. 실제 GT 이벤트 그래프는 **병사당 29~40 µs**(37명 ≈ 1.2 ms) 였다(1절 방법으로 분리). 전편의 "3.47 ms 를 스레드-세이프로 옮기면 거의 다 준다"([W98] ①)는 기대치 **≤ 1 ms** 로 내려간다.

PP ABP 는 없다 — `soldier_T` · `new_enemy_T` 스켈레탈 메시 둘 다 PostProcessAnimBlueprint **None**(MCP 되읽기). `obj list class=SoldierCharacter_ABP_C` = **82** 는 PIE 40 + **에디터 월드 40**(틱 안 함 — `AreActorsInitialized` false) + 프리뷰 2 이지 병사당 2개가 아니다.

### 2.2 (b) `KinematicBonesUpdateType = SkipAllBones` 는 채택 불가

전편 [W98] ② 의 근거 "살아 있는 병사의 피직스 바디는 QueryOnly 라 본을 따라갈 이유가 없다" 가 틀렸다 — **QueryOnly 라서 따라가야 한다**: 피격 부위 판정이 **메시 바디 트레이스**다(`Source/SoldierLab/AI/SoldierHealth.cpp:289-305 ResolveBoneByTrace` → `BodyPartScaleFor`, 09-15 체력 문서 3절). 바디가 포즈를 안 따라가면 머리 쏜 탄이 가슴 바디에 맞는다. `UpdateKinematicBonesToAnim` 0.97 ms 는 **정당한 비용** — 줄이려면 바디 수(피직스 에셋)를 줄이는 쪽이지 갱신을 끄는 쪽이 아니다. 폐기.

### 2.3 (c) "캐릭터 WeaponMesh 를 BeginPlay 에서 SetVisibility(false)" 는 반대다

`IMPLEMENTED.md` 3절 ② 가 "WeaponMesh 의 역할 ② BeginPlay 에서 SetVisibility(false)" 라 적었는데, 노드를 보면:

| 노드 | self | 대상 |
|---|---|---|
| `BP_SoldierCharacter` BeginPlay `K2Node_CallFunction_84` `SetVisibility(false)` | **← SpawnActor 결과**(`BP_AR4Rifle`) | **스폰된 총 액터의 `WeaponMesh`** |
| 캐릭터 컴포넌트 `WeaponMesh`(SK_AR4_X / 적군 SK_KA74U_X) | — | **보이는 총**(총구 보정 · 왼손 그립 소켓도 이쪽 — 3절 ③④ 는 맞다) |

즉 **병사 폰당 스켈레탈 메시 2 + 총 액터 1** 이고, 숨는 것은 총 액터 쪽이다. 캐릭터 WeaponMesh 의 `AnimClass = ABP_Weap_Rifle` 은 **죽은 참조**였다 — 그 ABP 의 스켈레톤과 SK_AR4_X 의 본 이름이 **0개 일치**해(`USkeleton::IsCompatibleMesh`, `Engine/Private/Animation/Skeleton.cpp:648`) 애님 인스턴스가 아예 안 생겼다. **None 으로 정리**(4.3절).

---

## 3. 새로 이름 붙은 비용 — 전 상태 [A]

병사 37명 프레임, `New_kadex_0811` PIE(40명 스폰, 3 사망). 게임 스레드, ms.

### 3.1 병사 몫

| 덩어리 | ms | 세부 | 어디 |
|---|---|---|---|
| **GASP 카메라** | **2.0** | `GameplayCamera` 1.60(= CameraDirector_SandboxCharacter BP 0.34 + Chooser 0.15 + 스윕 0.08 + 나머지 평가) + `SpringArm` 0.38(`bDoCollisionTest` 스윕) | AI 병사 40명 **전원**이 카메라 시스템을 매 틱 평가 — `bAutoActivate` + `bRunStandaloneCameraSystem` = true(`SandboxCharacter_CMC` CDO 상속) → `GameplayCameraComponentBase.cpp:552-568` BeginPlay 에서 활성, `:622-657` 틱 · `SpringArmComponent.cpp:197` 스윕. **보는 사람이 없는 카메라 40개** |
| 캐릭터 BP | 3.3 | `ReceiveTick` 2.36 + `AC_PreCMCTick` 0.94 | 4.4절 |
| SoldierLab | 4.0 | Cover 2.19 · Sight 0.86 · Engagement 0.42 · ScanTurn 0.35 · HeadAim 0.18 | Cover 는 전편 1.92 와 같은 자리(장면 차) |
| CMC | 1.86 | `UCharacterMovementComponent::TickComponent` | |
| 애니 GT | 1.61 + | `CharacterMesh0` 틱 | 4.2절 |
| **숨은 총 메시** | 0.19 + 0.34 | 총 액터 `WeaponMesh`(보이지 않는데 포즈 틱) | 4.3절 |

런타임 병사 컴포넌트 **38개** = CDO 29 + PIE 카메라 프록시/프러스텀 6 + OutputCamera + GameplayTasks + SoldierLabDetectable — 전편 [W99] "≈ 23" 은 Transform calls 역산이었고 실제 목록은 이것(카메라 계열이 8개).

### 3.2 사격 스폰 — 발당

`BP_AR4Rifle.Shoot` **발당 ≈ 1.1 ms**:

| 단계 | ms |
|---|---|
| 투사체 `BeginDeferredActorSpawnFromClass` | 0.45 |
| `FinishSpawningActor` | 0.12 |
| 총구 `SpawnSystemAttached` | 0.38 |
| `LaunchFrom` | 0.08 |

한 프레임 2발 = **2.25 ms**. 전편의 "0.65 ms" 는 프레임 평균(사격 밀도 ÷)이었다. ⚠ 기존 `SpawnSystemAttached` 가 **`bAutoDestroy=false`** — 발당 Niagara 컴포넌트가 총 액터 아래 **영구 누적**(8정에 106개 확인)되는 **누수**였다. 4.5절.

### 3.3 비병사 · PIE 전용

| 항목 | ms | 비고 |
|---|---|---|
| `AWindSource` Tick | 0.95 | MPC 11개를 **매 프레임** 푸시(`titan_example/Environment/WindSource.cpp`) → 6절 20 Hz |
| titan `TargetDetection` ×3 | 1.2 | UGV·트럭·자체방호 — titan 몫, 이 세션 손 안 댐 |
| Landscape 서브시스템 | 5.7 | **PIE 전용**(에디터 랜드스케이프 틱) — Standalone 엔 없다 |
| Slate | 5 | PIE 전용 |
| 뷰포트 | 3 | PIE 전용 |

→ PIE 절대값에서 ≈ 14 ms 는 에디터 몫. 전편 8절 "Standalone 에서 재라"의 크기가 이것.

---

## 4. 수정과 결과 [A]

### 4.1 카메라 — 비플레이어 병사의 카메라 틱 off ✅

`BP_SoldierCharacter` BeginPlay 끝에:

```
if (not IsPlayerControlled)
    → SetComponentTickEnabled(GameplayCamera, false)
    → SetComponentTickEnabled(SpringArm, false)
    → SetComponentTickEnabled(Camera(NotUsedByDefault), false)
```

- **Deactivate 가 아니라 틱만 끈다** — P107(GASP GameplayCamera 는 Deactivate 금지: 재활성화 시 카메라가 머리 위에 뜬다) 준수.
- 관전 폰의 3인칭/1인칭 follow 는 **관전 폰 자신을 옮기는 구조**(`Observer/SoldierObserverPawn.cpp:393-471`)라 병사의 GameplayCamera 를 빌리지 않는다 — 꺼도 관전에 영향 없음(PIE 확인). 1인칭 `AC_SoldierFirstPerson` 은 자기 카메라 컴포넌트를 쓴다.
- 플레이어가 빙의한 병사(GM_SoldierLab)는 `IsPlayerControlled` true 라 그대로.

| | 전 | 후 |
|---|---|---|
| GameplayCamera / SpringArm 틱 | 1.60 / 0.38 | **0건**(두 프레임) |
| World Tick(`L_SoldierScenario` 35명) | 22.6(전편) | **19.4 / 18.4** |

### 4.2 ABP — cvar 폴링 1 Hz ✅

`SoldierCharacter_ABP` 이벤트 그래프의 `Update` 가 매 틱 **콘솔변수 7개를 문자열로 조회 + `ComponentHasTag` 2회** 하고 있었다(GASP 원본 `SandboxCharacter_CMC_ABP` 의 `Update_CVarDrivenVariables` 상속 — 디버그 토글용).

| 바꾼 것 | 내용 |
|---|---|
| 새 함수 `Update_CVarDrivenVariables`(ABP 쪽 재선언) | `EventBlueprintInitializeAnimation` 에서 1회 |
| `Update` | 새 변수 **`CVarPollSeconds`**(DeltaTime 누적) ≥ **1.0** 일 때만 호출 → 1 Hz |
| `Update_PropertiesFromCharacter` · `Update_Logic` | 분기 **불변** |

| | 전 | 후 |
|---|---|---|
| GT 이벤트 그래프 / 병사 | 29 µs | **22 µs** |
| `CharacterMesh0` 틱(아군 20) | 0.834 | **0.616** |

⚠ 함정: MCP `create_node` 가 **같은 이름의 GASP 원본 클래스 함수**(`SandboxCharacter_CMC_ABP_C::Update_CVarDrivenVariables`)를 잡아 컴파일 에러 "self is not a SandboxCharacter_CMC_ABP_C" → `declaring_class` 를 `SoldierCharacter_ABP_C` 로 지정해서 해결.

### 4.3 숨은 총 메시 — `OnlyTickPoseWhenRendered` ✅

`BP_AR4Rifle.WeaponMesh`(스폰된 총, BeginPlay 에서 숨겨지는 쪽 — 2.3절) `VisibilityBasedAnimTickOption = OnlyTickPoseWhenRendered`: **0.26 → 0.07**(트랜스폼 전파만 남음). 캐릭터 `WeaponMesh` 의 AnimClass 는 죽은 참조라 **None**(2.3절 — 인스턴스가 원래 안 생겼으니 ms 변화 없음, 정리).

### 4.4 캐릭터 BP 틱 다이어트 (측정 전)

| 바꾼 것 | 내용 |
|---|---|
| 새 변수 **`AnimBP`**(`SoldierCharacter_ABP_C` 참조) | BeginPlay 에서 `CastToSoldierCharacter_ABP` 결과 저장 |
| `SetUseAllyAnimSet`(ABP 쪽) | Tick → **BeginPlay 로 이동**(진영은 안 바뀐다) |
| Tick 의 `CastToSoldierCharacter_ABP` | **제거** — `SetAimCorrection` / `SetLeanTactical` / `GetAOActive` 가 `AnimBP` 사용 |
| 디버그 `SetAxis` 7회 게이트 | **생략** — 이득(µs) 대비 리스크(축 HUD 가 안 뜨는 회귀) |

`ReceiveTick` 2.36 + `AC_PreCMCTick` 0.94 의 남은 몫은 C++ 이관([W108]).

### 4.5 총구 화염 — 상주 `MuzzleFlashFX` (측정 전, 누수는 해결)

| 바꾼 것 | 내용 |
|---|---|
| `BP_AR4Rifle` 새 컴포넌트 **`MuzzleFlashFX`**(NiagaraComponent) | `MuzzlePoint` 자식 · `bAutoActivate=false` · Asset `NS_MuzzleFlash` |
| `Shoot` · `PlayShotCosmetics`(리플리케이션 세션이 만든 함수) | `SpawnSystemAttached` → **`Activate(bReset=true)`** |
| 자식 `BP_AK47Rifle` | 상속 확인 |

발당 0.38 ms(3.2절) 가 사라져야 하고, 컴포넌트 누적(8정에 106개)도 없어진다. 측정은 다음 PIE.

---

## 5. URO — 켜자 크래시, 폐기 ❌ [A 사실 · B 원인]

`bEnableUpdateRateOptimizations = true` 를 부모 CDO + 자식 2 CDO + `L_SoldierScenario` 인스턴스 35 에 썼다(MCP CDO 쓰기는 자식/인스턴스에 전파 안 됨 — 메모리 — 그래서 직접). 검증 PIE **약 2분 만에 에디터 크래시**:

```
Assertion failed: !ParentBone.ContainsNaN() [BonePose.h:645]  Pose[1] ... -inf
직전: [SoldierIdentity] BP_Soldier_Friendly_C_19 heights measured: chest stand 1172835309453312000.0 cm
      Chaos `Invalid world space inflated bounds` ensure 연발(16:16)
```

- **원인 추정 [B]**: GASP 모션매칭 / `OffsetRootBone` / `DeadBlending` 이 URO 가 건너뛴 프레임(포즈 미갱신 + 큰 DeltaTime)을 상정하지 않아 발산 → NaN. 전편 [W98] ③ 의 "⚠ MM 트래젝토리 · 발 IK 가 프레임 스킵에 민감" 이 경고가 아니라 **크래시**였다. 확정은 빈 레벨 병사 1명 재현 → **[C-165]**.
- 에디터가 죽어 MCP 로 되돌리지 못했다 → **`Config/DefaultEngine.ini [ConsoleVariables] a.URO.Enable=0`** 안전장치. `ShouldUseUpdateRateOptimizations` = 메시 플래그 **AND** cvar(`SkinnedMeshComponent.cpp:1810`) 이라 플래그가 켜져 있어도 안 돈다.
- **후속 [W107]**: 에디터 재기동 뒤 플래그 false 로 되돌리고(부모 CDO · 자식 2 · 인스턴스 35) cvar 줄 삭제. 자동저장 `Saved/Autosaves/Game/SoldierLab/Blueprints/BP_SoldierCharacter_Auto1.uasset`(16:04) 이 **URO 직전 상태**(4.4절 다이어트 포함).
- 원칙 **P186** — URO 는 GASP 병사에 켜지 말 것. 애니 GT 1.2 ms 의 방법은 재검토(7절).

---

## 6. C++ 4건 — 빌드 대기(사용자 빌드) [A 코드 · 실측 전]

| 파일 | 바꾼 것 | 기대 |
|---|---|---|
| `AI/SoldierHealth.cpp` | `PrimaryComponentTick.TickInterval 0.1` + `DrawDebugString` 수명 = 간격 | Ticks Queued −35(ms 아님) |
| `AI/SoldierComms.cpp` | `TickInterval 0.1` | 위와 같음 — [W102] ③ 의 절반(Suppression/Perception 은 **DeltaTime 의존**이라 그대로) |
| `Pose/SoldierHeadAimComponent.cpp` | H off ∧ AI ∧ 상태 정지(Alpha·WeldBlend ≈ 0 · CorrectionLocal·NeckBendLocal 항등 · NeckStretchLocal 0 · 디버그 off)면 **조기 반환**(언와인드 중엔 계속 돎) | HeadAim 0.18 ms / 35명 → ≈ 0 |
| `Squad/SoldierSquadSubsystem.cpp` · `Squad/SoldierZone.cpp` | `SCOPE_CYCLE_COUNTER(STAT_SoldierLab_Squad / _Zone)` | **[W103] 해결** — `stat SoldierLab` 에 분대 층이 보인다 |
| `titan_example/Environment/WindSource.h/.cpp` | 새 UPROPERTY **`TargetPushIntervalSeconds 0.05`** — MPC 11개 + Niagara 푸시를 20 Hz 로 | 0.95 → ≈ 0.05 ms(New_kadex_0811) |

⚠ `WindSource.h` UPROPERTY 추가 = 헤더 리플렉션 변경 → **Live Coding 아님, 정식 빌드**(메모리 — missing property).

---

## 7. 남은 ms 와 현실적 바닥

`L_SoldierScenario` 35명, 카메라 수정 뒤, 교전 중 2프레임 평균, World Tick ≈ 20(PIE, 로깅 on).

| 덩어리 | ms | 다음 |
|---|---|---|
| 사격 스폰(프레임당 2발) | 2.2 | 투사체 풀링 **[W109]** −2 · 총구 상주(4.5, 측정 전) |
| 캐릭터 BP 틱 | 2.8(ReceiveTick 2.0 + PreCMC 0.75) | C++ 이관 **[W108]** −1.8 |
| SoldierLab | 3.5(Cover 1.7 · Sight 0.85 · Engagement 0.6 · Pose 0.3) | [W102] −0.9 |
| 애니 GT | 2.6(메시 틱 1.3 + PostTick 1.3) | −1.2 — **URO 폐기로 방법 재검토**(후보: 먼 병사 `bNoSkeletonUpdate` 거리 티어(MM 안 건드림) · 피직스 에셋 바디 수) |
| CMC | 1.8 | — |
| 물리 틱 함수 | 1.4 | — |
| 투사체 비행 | 0.7 | 풀링과 같이 |
| 틱 오버헤드(틱 함수 ≈ 850) | 0.95 | 솎기 −0.3(6절 TickInterval) |
| Niagara 매니저 | 0.5 | — |
| 비틱 · 기타 | 0.7 + 0.7 | — |

**현실적 바닥 ≈ 13~14 ms**(−2 −1.8 −0.9 −1.2 −0.3). PIE 전용 ≈ 14 ms(3.3절)는 Standalone 에서 저절로 빠진다.

---

## 8. 다른 세션과의 조율

리플리케이션 세션이 **같은 시간에** `SoldierEngagement`(복제 미러 + Multicast) · `ScanTurn` · `SoldierProjectile` · `DetectableTargetComponent` · `BP_SoldierCharacter` **Tick**(HasAuthority 분기 · 바운드 이벤트 2) · `BP_AR4Rifle`(`ReportShotFired` · `PlayShotCosmetics`) 을 편집했다. 파일 겹침 없이 순서 조율 — **BeginPlay 는 이 세션, Tick 은 저쪽**; `PlayShotCosmetics` 의 `SpawnSystemAttached` → `Activate` 치환(4.5절)은 저쪽 함수를 이쪽이 손댄 유일한 자리(합의). 투사체 풀링([W109])은 저쪽 `SoldierProjectile` 편집이 안정된 뒤.

---

## 9. 미해결 · 정정 대상

| ID | 무엇 |
|---|---|
| ~~**[W107]**~~ | ~~URO 되돌리기 — 플래그 false(부모 CDO · 자식 2 · 인스턴스 35) + `a.URO.Enable=0` 삭제(5절)~~ → ✅ **완료(10절)** |
| **[W108]** | 캐릭터 BP 틱 C++ 이관 — ReceiveTick 2.0 + PreCMC 0.75(4.4절 잔여) → 순서 ④(13절) |
| **[W109]** | 투사체 풀링 — `ASoldierProjectile`, 리플리케이션 세션 편집 안정 뒤(3.2절 발당 0.57) → 리플리케이션 세션 완료, **착수**(12·13절 A1~A3) |
| **[C-165]** | URO 크래시 원인 확정 — 빈 레벨 병사 1명 + URO 로 재현되는가, 어느 노드(MM / OffsetRootBone / DeadBlending)에서 NaN 이 처음 나오나 |
| [W98] | ① 기대치 ≤ 1 ms 로 하향 ② 폐기 ③ URO 폐기 — 새 방향 필요(7절) |
| [W99] | 카메라 · Camera(NotUsed) · AnimBP 캐시 완료 / 컴포넌트 38개 목록 확인 / C++ 이관은 [W108] |
| [W100] | 발당 1.1 ms 실측 · 총구 Niagara 완료(누수 해결) · 풀링은 [W109] |
| [W103] | 완료(빌드 대기) → 빌드됨(10절) |
| 전편 문서 | 10절 정정(2.1~2.3 + 7절 표) · `IMPLEMENTED.md` 3절 ② 정정 |

---

## 10. 후속 — 재기동 뒤 URO 되돌림 [W107] 완료 · C++ 4건 빌드 [A] (16:55~17:05)

사용자가 정식 빌드(새 DLL 16:55) + 에디터 재기동. 그 뒤 MCP 로:

| 무엇 | 결과 |
|---|---|
| `bEnableUpdateRateOptimizations=false` | `BP_SoldierCharacter` · `BP_Soldier_Friendly` · `BP_Soldier_Hostile` **CDO 3** + `New_kadex_0811` 인스턴스 **40**(로드 시 전부 true 였다 → 전부 false). BP 3개 저장. 레벨은 **저장 안 함** — 인스턴스 값이 CDO 와 같아져 델타가 없다 |
| `Config/DefaultEngine.ini` `[ConsoleVariables] a.URO.Enable=0` | 줄 **제거**. 파일이 원본과 동일해져 P4 체크아웃 **revert** |
| 6절 C++ 4건 | 빌드됨 — 실측은 11절 |

5절의 "인스턴스 35(`L_SoldierScenario`)"는 크래시 전 메모리에만 쓴 값이고 그 레벨은 저장되지 않았다(CDO 와 같아 델타도 없음). 재기동 뒤 되돌린 것은 `New_kadex_0811` 인스턴스 40(로드 시 true → false). `L_SoldierScenario` 는 CDO 를 false 로 되돌린 뒤에 열렸으므로 인스턴스가 CDO 를 따라 false — 17:10 MCP 되읽기로 확인: 병사 35 중 `bEnableUpdateRateOptimizations=true` **0** [A]. **[W107] 해결**, P186 그대로.

---

## 11. 후속 — 배치 실측(재기동 후) · 레이트레이스 소유자별 [A]

조건: `L_SoldierScenario` 35명 PIE, 교전 중 **2프레임**, `stat dumpframe -ms=0.05`. World Tick **18.3 / 20.3 ms** — 두 번째는 Cover 2.7 + 사격 스폰 1.1 이 겹친 프레임(잡음 ±0.3 규약 그대로).

### 11.1 이 배치의 전후

| 항목 | 전(3·7절) | 후 | 비고 |
|---|---|---|---|
| GameplayCamera / SpringArm 틱 | 1.60 / 0.38 | **0**(트랜스폼 전파 0.06 만) | 4.1절 확정 |
| `SoldierHeadAimComponent` 틱 | 0.18 | **목록에서 소멸** | 6절 조기 반환 |
| `SoldierHealth` / `Comms` 틱 | (Ticks Queued 35 ×2) | **목록에서 소멸**(10 Hz) | 6절 `TickInterval 0.1` |
| `BP_AR4Rifle.Shoot` 발당 | 1.1 | **0.94** — `SpawnSystemAttached` **0건**, 남은 것 = 투사체 `BeginDeferredActorSpawnFromClass` 0.65 + `FinishSpawningActor` 0.18 + `LaunchFrom` 0.11 | 4.5절 총구 상주 확정. 남은 0.94 는 전부 **투사체 스폰** → 12절 |
| 캐릭터 BP `ReceiveTick`(아군 20) | 1.21 | 1.21(**변화 없음**) | 4.4절 `AnimBP` 캐시 이득은 µs — C++ 이관 [W108] 이 남은 길 |
| `AWindSource` | 0.95 | **미측정** — `L_SoldierScenario` 에 WindSource 가 없다 | `New_kadex_0811` 에서 |
| 런타임 에러 · 크래시 | — | **없음** | |

결론: 이 배치에서 실제 ms 를 낸 것은 **카메라 2.0 + HeadAim 0.2 + 총구 0.4/발**. Health/Comms 는 틱 수만 줄었고(ms 아님), BP 다이어트는 측정 안에 안 들어온다.

### 11.2 레이트레이스 소유자별 — `SceneQueryTotal` 을 부모 스코프에 귀속

같은 두 프레임에서 `dumpframe` 의 `SceneQueryTotal` 줄을 부모 스코프(누가 불렀나)로 귀속했다. 총 **1,030~1,119회/프레임, 2.6~3.0 ms**:

| 소유자 | 회/프레임 | ms | 어디 · 왜 |
|---|---|---|---|
| Cover: Candidate | 367~459 | 0.8~1.1 | 전편 (c)(d) 뒤에도 가장 큰 덩어리 — [W102] ② 후보 2틱 |
| Cover: Begin Sweep | 144~240 | 0.36~0.55 | 눈 있는 스윕 |
| **Engagement** | **182~188** | **0.46** | `IsShotBlockedByWorld`(`AI/SoldierEngagement.cpp:371-417`)가 **매 틱** 실제 총구(`:1167`) + 앉은 총구(`:1247`) + `PlanAperture` 옵션마다(`:506`) ≈ **5.5회/병사/프레임** — 호출마다 레지스트리 병사 **35명을 `AddIgnoredActor` 로 목록 구성**(`:387-404`). 트레이스 자체보다 **ignore 목록 35개 × 190회** 가 비용 → **[W110]** |
| Sight | 102~105 | 0.35~0.38 | [W102] ① 격틱 |
| CMC `Char FindFloor` | 99~105 | 0.30 | `bAlwaysCheckFloor=true` 라 **정지 병사도 매 프레임** 바닥 스윕 → **[W111]** |
| Cover: Here / Route | 70~120 | 0.2~0.4 | 전편 (b)(c) 뒤 잔여 |

---

## 12. 후속 — 투사체 발견: 풀링 설계인데 BP 가 스폰해서 **영구 누적** [A]

| 사실 | 근거 |
|---|---|
| `ASoldierProjectile` 은 **풀링용 설계** | `Weapons/SoldierProjectile.h:7` 헤더 주석("Designed to be pooled") · `LaunchFrom` 이 전체 리셋(`.cpp:198-252`) · `Deactivate()`(`.cpp:1183`)는 **숨김 + 충돌·틱 off 만, `Destroy` 없음** |
| 쏘는 쪽은 발마다 **`SpawnActor`** | `BP_AR4Rifle.Shoot` · `PlayShotCosmetics`(리플리케이션 세션 함수) · 도탄 `USoldierEngagementComponent::Multicast_LaunchRicochet_Implementation`(`AI/SoldierEngagement.cpp:246-268`) |
| → 주차된 투사체가 **영구 누적** | 실측: 한 프레임에 비행 중 **21발**인데 `TracerTrailComponent` **80개**. 총구 Niagara 누수(3.2절)와 같은 모양의 누수가 하나 더 있었다 |
| `BP_RifleProjectile` EventGraph 에 **빈 `EventTick` 노드** | 발당 BP `ReceiveTick` 디스패치 — 30발에 **0.25 ms** 가 빈 이벤트에 든다 |
| `CollisionComponent` 가 `QueryAndPhysics` | `.cpp:118-119`. 명중은 **이동 스윕**이라 `QueryOnly` 가능 — titan 투사체 선례 있음(메모리: 키네마틱 물리바디가 PT 에서 UGV Hull 과 충돌하던 버그를 QueryOnly 로 고침) |
| RCWS 는 이미 풀 | `RCWSFireControlComponent`(`Vehicles/RCWSFireControlComponent.cpp:163-171` 64개 사전 스폰 · `:1569-1584` 라운드로빈) — 모양 참고 |

`IMPLEMENTED.md` 3.1절 "풀 자체는 없다 — 쏘는 쪽의 변경" 이 그대로 맞다. 다만 **없는 것이 비용이 아니라 누수**라는 것이 새 사실이다.

---

## 13. 후속 — 남은 최적화 후보와 순서(사용자 승인) [A 실측 · B 예상치]

| 순서 | 무엇 | 예상 | ID |
|---|---|---|---|
| **①** | **투사체** — A1 풀(월드 서브시스템 `USoldierProjectilePoolSubsystem`, 클래스별 풀, 주차된 것 재사용 · 없으면 스폰 · 상한) + A2 `BP_RifleProjectile` 빈 `EventTick` 삭제 + A3 `CollisionComponent` `QueryOnly` | −0.8 ms/발(교전 프레임 −1.5~3) + 누수 제거 | **[W109] 지금 착수** |
| ② | Engagement 레이 — 병사 무시를 **채널 응답**으로(ignore 목록 35 × 190 회 제거) + 표적·총구 불변 시 **캐시** + `PlanAperture` 저빈도 | −0.3~0.4 | **[W110]** |
| ③ | CMC 데이터 2개 — `bAlwaysCheckFloor=false` · `bEnablePhysicsInteraction=false` | −0.2 | **[W111]** |
| ④ | 캐릭터 BP 틱 → C++ `USoldierAIBridgeComponent` | −1.5~2 | [W108], 2PC 리플리케이션 검증(내일) 뒤 |
| ⑤ | Cover 후보 2틱 · Sight 격틱 | −0.5~0.8(거동 판정 동반) | [W102] |
| ⑥ | 컴포넌트 틱 통합(`SoldierBrain` 하나로 −0.4) · 총 액터 메시 제거 | 구조가 큼 | **[W112]** · **[W113]** |

- 애니 GT(2.6)는 URO 불가(P186)라 남은 길은 `Update_PropertiesFromCharacter` 의 **프로퍼티 액세스화**(이벤트 그래프 → 워커) — [W98] ① 잔여, 이득 ≤ 1.
- titan `TargetDetection` ×3 1.2 ms(New_kadex_0811)는 **미조사**(titan 몫).
- 조율: 리플리케이션 세션 작업은 **완료**, 2PC 검증은 내일(사용자) — 8절의 "그쪽 안정 뒤" 조건이 풀렸다. `SoldierProjectile` · `BP_AR4Rifle` 편집 겹침 없음.

---

## 14. 후편 참조 — 13절 순서 ①~⑤ 실행 (17:10~18:10) [A]

13절 표의 **①~⑤ 가 같은 날 후편 `ai/2026-09-21_game_thread_structural_pool_rays_bridge.md` 에서 실행됐다**(사용자 PIE 확인). 결과만 한 줄씩 — 상세·file:line 은 그쪽 A~E절.

| 순서 | 결과 | 후편 절 |
|---|---|---|
| ① 투사체 풀 [W109] | `USoldierProjectilePoolSubsystem::Acquire` · BP/도탄 전부 풀 · QueryOnly · 빈 EventTick 삭제 · `MaxFlightDistanceCm 60000` → `Shoot` 안 스폰 0.83/발 → 0, 투사체 47 에서 정지, World Tick 18.3/20.3 → 17.1/14.3 | A |
| ② Engagement 레이 [W110] | 병사 무시를 **Pawn 채널 응답 Ignore**(`SoldierQuery::BodiesAreNotWalls()`, Engagement·Cover·Field 8 사이트) + `IsShotBlockedByWorld` 8슬롯 캐시 → 씬 쿼리 1,030~1,120 → 357/576회, Engagement 레인 182~188 → 23/35회 | B |
| ③ CMC 데이터 [W111] | `bAlwaysCheckFloor=false` · `bEnablePhysicsInteraction=false` → `FindFloor` 99~105 → 44/27회. ★ **정정**: 병사 메시 콜리전은 **QueryAndPhysics 였다**(2.2절·전편 [W98] ② 의 "QueryOnly" 전제가 틀림) → QueryOnly 로 | C |
| ⑤ 박자 [W102] | `CandidateIntervalSeconds 0.033` · `ScanIntervalSeconds 0.033` — 프레임이 아니라 시간에 고정 | D |
| ④ 캐릭터 BP 틱 → C++ [W108] | `USoldierAIBridgeComponent::TickBridge`(EventTick 본문 그대로, 변수는 BP 소유) → 아군 20 ReceiveTick 1.2 → 0.22, World Tick 15.1 → **11.3/14.0** | E |
| ⑥ [W112] · [W113] | 남음. 새 [W114](`AN_Reload` 노티파이 에러 로그) · [W115](`AC_PreCMCTick` 0.8 ms) | F |

7절의 "현실적 바닥 ≈ 13~14" 는 **11~14 로 도달**했다(PIE, 로깅 on).
