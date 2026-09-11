# 인게임 Graphics 설정 탭 — 구현

2026-09-10 / 진행중(코드·WBP·빌드·게임 레벨 대상 탐색까지 확인, 패키지 실측 대기) / 품질 프리셋·렌더링 방식·나무 성능·카메라 캡쳐 주기를 인게임에서 조절. ini 하드코딩을 단일 소스로 이관하고 플랫폼 불일치도 같이 해소.

사전 조사·설계 근거는 `ui/graphics_settings_analysis.md`(§9 채택 구조 / §10 확정 스코프 / §12 프리셋 설계).
이 문서는 **실제로 무엇을 만들었고 무엇이 남았는지**만 다룬다.

---

## 1. 왜 이 작업이 필요했나 — 한 줄

**하드코딩된 cvar가 런타임 품질 변경을 원천 차단하고 있었다.** `sg.*` 12개가
`Config/Windows/WindowsEngine.ini`의 `[ConsoleVariables]`(`SetBySystemSettingsIni`, 0x05)에 박혀 있어서,
`Scalability::SetQualityLevels`(`SetByScalability`, 0x02)가 조용히 거부됐다. 그룹 멤버는 `sg.*`의
`SetOnChangedCallback`이 떠야만 적용되므로 **품질 변경이 "일부만 먹는" 게 아니라 완전 no-op**이었다.
(2026-09-03 `DumpCVars` 전수 덤프로 확정 — 조사 문서 §0-2 F.)

덤으로 두 가지가 더 드러났다:
- 그 파일이 **Windows 전용**이라 Linux 빌드는 튜닝을 하나도 안 받고 있었다. **LIG 납품물이 Linux 패키지**다.
- 에디터와 게임이 **서로 다른 ini**를 읽어서(`LaunchEngineLoop.cpp:2866`), "에디터는 A인데 패키지는 B"가
  엔진 기본 동작이었다.

---

## 2. 구조

```
Config/DefaultGame.ini                    ① 프로젝트 기본값 (P4 공유, 패키징 포함)
  └─ UTitanGraphicsSettings (UDeveloperSettings, config=Game, defaultconfig)
        ├─ 스케일러빌리티 12축
        ├─ 캡쳐 주기 3종
        └─ 식생 2값 (WPO 거리 / LOD 배율)
                  │
                  │  UTitanGraphicsSubsystem (UEngineSubsystem)
                  ├── OnPostEngineInit           ──▶ Apply()            (cvar/스케일러빌리티)
                  └── OnPostWorldInitialization
                        └─ UWorld::OnWorldBeginPlay ──▶ ApplyCaptureRates() / ApplyFoliageSettings()
                                                          (컴포넌트 프로퍼티 — 액터가 있어야 함)

Saved/Config/<Platform>/GameUserSettings.ini   ② 최종 사용자 오버라이드
  ├─ [ScalabilityGroups]   (엔진 Scalability::SaveState가 관리)
  └─ [TitanGraphics]       (캡쳐 주기 / 식생 — 우리가 직접 관리)

Config/DefaultScalability.ini              프리셋 재정의 (엔진 BaseScalability.ini 위에 키 단위 병합)
  ├─ [ShadowQuality@2]              현재 쓰는 그림자 단계에만 우리 값
  └─ [GlobalIlluminationQuality@1]  현재 쓰는 GI 단계에만 우리 값
```

**설계의 요점 3가지**

1. **`UDeveloperSettings` + `defaultconfig`** — 값이 `Config/DefaultGame.ini`에 저장되니 P4로 공유되고
   패키징에 포함된다. 에디터/PIE/패키지가 **같은 파일 하나**를 읽으므로 "에디터는 A, 패키지는 B"가
   구조적으로 불가능해진다. Project Settings에 UI가 자동 생성되고 `PostEditChangeProperty`로 즉시 반영된다.
2. **적용 시점은 `OnPostEngineInit`** — 그 앞에 `Scalability::LoadState`와
   `UGameUserSettings::ApplyNonResolutionSettings`가 먼저 돌아 스케일러빌리티를 건드린다.
3. **cvar가 아닌 값은 "밀어넣기"** — 캡쳐 주기·식생은 컴포넌트 프로퍼티라 자동 반영 경로가 없고,
   `UQuadCamComponent`는 별도 플러그인이라 게임 모듈 설정 클래스를 **볼 수 없다**. 그래서 월드 BeginPlay
   훅에서 액터를 순회해 대입한다(`UStreamResolutionSubsystem` → `UVehicleRtspBridgeComponent`와 같은 제약).

---

## 3. 파일 변경

### 신규 (전부 `p4 add` 필요)

| 파일 | 역할 |
|---|---|
| `Source/titan_example/Settings/TitanGraphicsSettings.h/.cpp` | 설정 데이터 + 적용 로직 + 사용자 오버라이드 저장/삭제 |
| `Source/titan_example/Settings/TitanGraphicsSubsystem.h/.cpp` | `OnPostEngineInit` / 월드 BeginPlay 훅 |
| `Source/titan_example/UI/GraphicsSettingRowWidget.h/.cpp` | 설정 행 하나(드롭다운 또는 숫자 입력) |
| `Config/DefaultScalability.ini` | 프리셋 재정의 2섹션 |

### 수정

| 파일 | 내용 |
|---|---|
| `Source/titan_example/UI/GameSettingsWidget.h/.cpp` | Graphics 탭 — 항목 정의(`RefreshGraphicsList()`), 행 동적 생성, 기본값 복원 |
| `Source/titan_example/titan_example.Build.cs` | `"DeveloperSettings"` 추가 |
| `Plugins/QuadCamModule/.../QuadCamComponent.h/.cpp` | `CaptureEveryNTicks` 신설 + 캡쳐 게이트 |
| `Config/DefaultEngine.ini` | 반사 SSR 통일 / Lumen 튜닝 6줄 플랫폼 공통화 / Lumen 예산 3줄 제거 |
| `Config/Windows/WindowsEngine.ini` | `sg.*` 12줄 + Lumen·VSM 7줄 + 반사 1줄 제거 → `[ConsoleVariables]`가 빈 섹션 |

---

## 4. ini 이관 결과

| 대상 | 이전 위치 | 이후 위치 | 이유 |
|---|---|---|---|
| `sg.*` 12개 | `WindowsEngine.ini [ConsoleVariables]` | `UTitanGraphicsSettings` (`DefaultGame.ini`) | 우선순위가 런타임 변경을 막았음 |
| Lumen/VSM 그룹 멤버 10개 | 〃 + `DefaultEngine.ini` RendererSettings | `DefaultScalability.ini` | 〃. 값은 그대로라 기본 단계 화면은 동일 |
| Lumen 비그룹 값 6개 | `WindowsEngine.ini` | `DefaultEngine.ini [ConsoleVariables]` | **Windows 전용이라 Linux가 못 받고 있었음** |
| `r.ReflectionMethod` | Default=1(Lumen) / Windows=3(범위 밖) | `DefaultEngine.ini`에 **2**(SSR) 하나로 | 플랫폼 불일치 해소. Windows 무변화, Linux가 바뀜 |

**실측 확인** — 이관 후 `DumpCVars sg.` 결과 12개 전부 `LastSetBy: Scalability`로 바뀌고 값은 유지됐다.
그리고 PIE 로그의 `SetByScalability ... was ignored` 경고가 **12줄 → 2줄**로 줄었다. 남은 2줄은
`r.Streaming.PoolSize` / `LimitPoolSizeToVRAM`으로, VRAM 안전장치라 **의도적으로 고정**한 것이다.

> **왜 프리셋 재정의가 2섹션뿐인가** — `Scalability::SetGroupQualityLevel`은
> `ApplyCVarSettingsFromIni(<섹션>, GScalabilityIni, SetByScalability)`로 **섹션에 적힌 키를 전부**
> 적용하고(`Scalability.cpp:446`), config 계층은 섹션을 **키 단위로 병합**한다. 그래서 엔진 프리셋을
> 베낄 필요 없이 덮을 키만 적으면 되고, 현재 쓰는 단계(Shadow=2 / GI=1)만 채우면 "기본 상태의 화면이
> 지금과 동일"이 보장된다. 다른 단계는 엔진 표준값으로 움직인다 — 의도한 동작이다.
>
> `IrradianceFieldGather.*` 등은 **엔진 그룹 멤버가 아니라서** 어느 단계에서도 덮이지 않는다.
> `FinalGatherMethod=0`인 단계에서만 효력이 있고 나머지에선 조용히 무시될 뿐 해가 없어서 전역으로 남겼다.

---

## 5. 탭 항목 — 헤더 5 + 항목 18

| 섹션 | 항목 | 컨트롤 | 실제로 건드리는 것 |
|---|---|---|---|
| **기본** | 전체 품질 | 드롭다운 4 | `SetFromSingleQualityLevel` (12축 일괄) |
| **화질** | 그림자 | 드롭다운 4 | `sg.ShadowQuality` — **실측 2→0에서 50→57fps** |
| | 전역 조명(GI) | 드롭다운 4 | `sg.GlobalIlluminationQuality` |
| | 반사 | 드롭다운 4 | `sg.ReflectionQuality` |
| | 텍스처 | 드롭다운 4 | `sg.TextureQuality` — `MipBias`/`MaxAnisotropy` 등. 풀 크기는 고정 |
| | 시야 거리 / 이펙트 / 셰이딩 / 포스트프로세스 | 드롭다운 4 ×4 | 각 `sg.*` |
| **렌더링 방식** | 안티에일리어싱 품질 | 드롭다운 4 | `sg.AntiAliasingQuality` |
| | **반사 방식** | 드롭다운 3 | `r.ReflectionMethod` (SSR/Lumen/끔), `SetByConsole` |
| | 안티에일리어싱 방식 | 드롭다운 4 | `r.AntiAliasingMethod` (TSR/TAA/FXAA/없음), `SetByConsole` |
| | 수직 동기화 | 드롭다운 2 | `UGameUserSettings::SetVSyncEnabled` |
| **나무·초목** | **바람 흔들림 거리** | **숫자 0~200m** | `SetWorldPositionOffsetDisableDistance` (전 ISM/HISM) |
| | **나무 LOD 전환 배율** | **숫자 0.1~2.0** | `SetLODDistanceScale` (전 ISM/HISM) |
| **카메라 피드** | CCTV / 드론 짐벌 / 전장 카메라 갱신 주기 | 드롭다운 4 ×3 | `CaptureEveryNTicks` / `*RoundRobinCount` |

### 왜 나무 2항목만 숫자 입력인가

둘 다 **연속량이고 실측 성능 곡선이 있는** 값이라 4단계로 쪼개면 정보를 버린다(사용자 지적).
드래그로 훑으면서 fps를 보고 맞추는 게 자연스러운 사용법이다. `USpinBox`는 드래그와 직접 타이핑을
한 위젯에서 다 준다.

| | 실측 (`level_new_kadex_0811/new_kadex_0811_forest_perf.md`) |
|---|---|
| WPO 거리 | `0(무제한)=2.3fps / 100m=7 / 50m=16 / **30m=21** / 20m=21~24 / off=22~24` — knee 20~50m |
| LOD 배율 | `1.0=21.8fps / **0.5=32** / 0.35=36` |

기본값(30m / 0.5)이 현재 레벨 저작값과 같다. `OnValueChanged`가 아니라 **`OnValueCommitted`** 를 쓴다 —
적용이 월드의 ISM/HISM 50개(94,936 인스턴스, §8-1 실측) 순회라 드래그 중 매 프레임 돌릴 수 없다.

### 제거한 항목 — "초목 품질"(`sg.FoliageQuality`)

**이 레벨에서 효과가 0**이라서다. 그 그룹은 `foliage.DensityScale`/`grass.DensityScale`/`pcg.Quality`를
세팅하는데,
- 앞의 둘은 Foliage 시스템·랜드스케이프 그래스용이고 우리 나무는 **PCG로 뿌린 뒤 레벨에 구워진
  ISM/HISM**이다(`.umap` 185MB, PCG 액터 5개가 `bGenerated=true`라 실행 시 재생성 안 함).
- `pcg.Quality`는 엔진 설명 그대로 "**Runtime** Quality Branch/Select 노드"에만 영향이고, 값이 바뀌면
  `RefreshAllRuntimeGenExecutionSources()`만 부른다 — 런타임 생성 PCG만 갱신한다.

대신 위 나무 2항목이 그 자리를 대체한다. (`sg.FoliageQuality`는 "전체 품질"에는 여전히 포함된다.)

### 적용·저장 정책

**즉시 적용 + 즉시 저장.** 품질은 효과를 바로 봐야 하고, 로비에서 맞추고 레벨로 들어가는 운용이라
"Apply 누르는 걸 깜빡"하는 경우를 없애는 편이 낫다(Input 탭이 `ApplySaveButton`을 쓰는 건 키 리매핑의
중간 상태가 위험해서다 — 성격이 다르다).

되돌리기는 **`GraphicsResetButton`** 이 담당한다: `GameUserSettings.ini`의 `[ScalabilityGroups]` /
`[TitanGraphics]` 두 섹션을 지우고 프로젝트 기본값을 재적용한다.

> ★ **이 버튼이 없으면 안 되는 이유** — 품질을 한 번이라도 바꾸면 그 PC의 `GameUserSettings.ini`에
> `[ScalabilityGroups]`가 생기고, 그때부터 `Config/DefaultGame.ini`를 고쳐도 **그 PC에는 반영되지 않는다.**
> 프레임 상한(2026-08-25) 때 겪은 것과 같은 함정이다.

---

## 6. WBP 계약 (에디터 작업 — 사용자 몫)

### 6-1. `tab_graphics`를 **CanvasPanel → VerticalBox로 교체** ✅ 완료

08-21 "틀만" 시절 CanvasPanel이 남아 있었다. CanvasPanel에 ScrollBox를 넣으면 앵커/오프셋을 손으로
잡아야 하고 창 크기를 안 따라간다. `tab_input`과 동일한 구조로 맞춘 것.

```
tab_graphics (VerticalBox)
├─ [0] GraphicsListContainer   ScrollBox   Fill(1.0), padding 0, Fill/Fill
└─ [1] HorizontalBox                       Auto, HAlign_Right, padding Top/Bottom 10
        └─ GraphicsResetButton             "기본값으로 되돌리기"
```

### 6-2. `WBP_GraphicsRow` (부모 = `GraphicsSettingRowWidget`) ✅ 완료

```
Border                              루트. Padding 10 / RoundedBox / CornerRadii 5
└─ HorizontalBox
    ├─ [0] VerticalBox              Size = Fill(1.0),  VAlign = Center
    │       ├─ LabelText            Auto,  AutoWrapText = true
    │       └─ DescriptionText      Auto,  AutoWrapText = true,  Padding Top = 4
    └─ [1] SizeBox                  Size = Auto,  VAlign = Center,  Padding Left = 12
            · WidthOverride = 220
            └─ VerticalBox
                ├─ OptionComboBox   Auto,  HAlign = Fill
                └─ ValueSpinBox     Auto,  HAlign = Fill
```

> ★ **Overlay를 쓰면 안 된다** — Overlay는 자식을 **겹쳐** 놓는 패널이라, 컨트롤을 `Fill`로 두면
> 라벨 위를 덮고 `Right`로 두면 콘텐츠 크기까지 쪼그라든다(둘 다 실제로 겪음, 2026-09-10).
> `WBP_KeybindRow`가 Overlay로 되는 건 우측이 작은 버튼 하나뿐이라서다. 여기선 컨트롤이 폭을
> 차지해야 하므로 흐름 배치(HorizontalBox)가 맞다.
>
> ★ **라벨 열은 반드시 `Fill`** — `Auto`로 두면 텍스트가 무한 폭을 원해서 컨트롤을 화면 밖으로
> 밀어내고 잘린다. `Fill`이라서 폭이 정해지고, 그래야 `AutoWrapText`가 동작한다.
>
> SizeBox 안에 VerticalBox를 한 겹 두는 이유: SizeBox는 자식이 하나뿐인데 컨트롤이 둘이다.
> 둘 중 하나는 항상 `Collapsed`라 세로로 쌓아도 보이는 건 하나고 공간도 하나만 차지한다.

**반응형 동작**: 창 폭이 바뀌면 라벨 열만 신축(컨트롤 220px 고정) / 긴 설명은 2~3줄로 wrap되며 카드
높이가 자동 증가 / 양 열 `VAlign=Center`라 설명이 여러 줄이어도 컨트롤이 수직 중앙.

### 6-3. `WBP_GraphicsSectionHeader` (부모 = `GraphicsSettingRowWidget`) — **필수** ✅ 완료

```
VerticalBox                    루트 (Border 쓰지 말 것 — 카드처럼 보이면 구분이 안 된다)
├─ [0] Spacer                  Auto,  Height 14                              위 카드와 간격
├─ [1] LabelText               Auto,  HAlign Left                            글꼴 크게/굵게 + 강조색
└─ [2] Border 또는 Image       Auto,  HAlign Fill, Height 1~2, 반투명 흰색    구분선
```

`LabelText` 하나만 바인딩되면 된다. **비워두면 헤더 행을 건너뛰고 경고를 남긴다** — 예전엔
`GraphicsRowWidgetClass`로 폴백했는데 디자인이 일반 행과 똑같아서 구분이 전혀 안 됐다.

### 6-4. 클래스 디폴트

```
GraphicsRowWidgetClass           = WBP_GraphicsRow
GraphicsSectionHeaderWidgetClass = WBP_GraphicsSectionHeader
```

---

## 7. 겪은 함정 (재발 방지)

| # | 함정 | 교훈 |
|---|---|---|
| 1 | **`UDeveloperSettings` 상속 시 링크 에러 무더기** — `DeveloperSettings`가 `Engine.Build.cs`의 `PublicDependencyModuleNames`에 있는데도 링크가 안 됐다. 헤더는 `PublicIncludePathModuleNames`에도 있어 **컴파일은 통과하고 링크에서만** 터진다 | 에러 심볼에 `__declspec(dllimport)`가 **빠져** 있으면 API 매크로가 안 깔린 것 → 우리 `Build.cs`에 모듈을 명시할 것 |
| 2 | **`INDEX_NONE`은 int32가 아니다**(이름 없는 enum) | 반환형 추론에 맡기는 람다에서 `INDEX_NONE`과 `int32`를 섞으면 C3487/C2440. `-> int32` 명시 |
| 3 | **`UEngine::SetMaxFPS`는 cvar의 기존 SetBy 이유를 승계한다** | "GameUserSettings는 전부 `SetByScalability`라 못 이긴다"는 일반화가 틀렸다. 항목마다 경로/우선순위가 다르다 |
| 4 | **`UGameUserSettings::ScalabilityQuality` 동기화를 빼먹으면 되돌아간다** | 창 모드/해상도가 바뀌는 순간 `ApplySettings(false)`가 LoadSettings 시점 스냅샷으로 재적용한다(`GameViewportClient.cpp:4129/4212`) |
| 5 | **액터 존재 여부로 UI를 비활성화하면 안 된다** — 드론/전장 갱신 주기를 월드에 액터가 있을 때만 활성화했더니, 차량이 없는 `kadex_lobby`(= 설정을 하라고 만든 화면)에서 둘 다 비활성이 됐다 | 저장되는 값은 **설정 시점에 대상이 존재할 필요가 없다.** 축 정보는 런타임 탐색이 아니라 고정된 사실이므로 설명 문구로 알릴 것 |
| 6 | **Overlay에서 Fill = 겹침** | §6-2 참고. 컨트롤이 폭을 차지하는 행은 HorizontalBox + SizeBox |
| 7 | **빌드 안 된 상태를 로그로 착각** — 캡쳐 주기 로그가 안 떠서 훅 문제를 의심했는데 DLL이 소스보다 오래된 것이었다. `[TitanGraphics] 적용` 줄은 설정 **필드값**을 찍을 뿐이라 구 빌드에서도 똑같이 나온다 | 이런 확인은 `Binaries/**/*.dll` mtime과 `Log file open, …` 시각부터 대조할 것 |
| 8 | **`memo.md`를 요구사항 근거로 인용** — `r.ReflectionMethod=3`의 의도를 찾다가 스크래치 메모를 요구사항으로 오독했다 | `memo.md`는 최신화 없는 개인 메모다. 의도를 모르면 추측하지 말고 선택지를 제시할 것 |

---

## 8. 검증 상태

### 완료

| 항목 | 근거 |
|---|---|
| 이관 후 `sg.*` 12개가 `Scalability`로 풀림 | `DumpCVars sg.` 실측 |
| 거부 경고 12줄 → 2줄 | PIE 로그 |
| 그림자 품질 실제 적용 | 사용자 실측 50→57fps |
| GI 품질 실제 적용 | 사용자 실측(전환 히칭이 적용된 증거) |
| 엔진 시작 훅 / 월드 BeginPlay 훅 | `[TitanGraphics] 적용` / `캡쳐 주기 적용(New_kadex_0811) — QuadCam 2개, Drone 1개, Truck 1개` |
| `tab_graphics` VerticalBox 교체 | 사용자 완료 |

### 완료 (2026-09-10 빌드·WBP 이후 추가)

| 항목 | 근거 |
|---|---|
| 빌드 통과 | `UnrealEditor-titan_example.dll` 09-10 16:56 / `UnrealEditor-QuadCamModule.dll` 09-05 11:08, 로그 오픈 16:57 |
| **WBP 2개 정상 동작** | `[GameSettingsWidget] Graphics 탭 23개 항목 생성` — 헤더 5 + 항목 18이 전부 생성됨. `GraphicsSectionHeaderWidgetClass` 미지정 경고도 안 뜸(= 지정됨) |
| 행 위젯 레이아웃(§6-2) | 사용자 확인 — 라벨/컨트롤이 겹치거나 잘리지 않음 |
| 식생 훅 호출 | `[TitanGraphics] 식생 적용(kadex_lobby) — ISM/HISM 0개 …` — 훅 자체는 정상. **로비에 인스턴스 메시가 없어서 0개가 맞다** |
| ✅ **게임 레벨 대상 탐색** | `New_kadex_0811` PIE(09-10 08:24) 로그:<br>`캡쳐 주기 적용(New_kadex_0811) — QuadCam 2개=x1, Drone 1개=2틱, Truck 1개=2틱`<br>`식생 적용(New_kadex_0811) — ISM/HISM 50개(인스턴스 94936)`<br>**소유 액터를 전수 확인해 50개 전부 숲 소유임을 검증했다** — 아래 §8-1 참고 |

### 8-1. 식생 대상 50개의 정체 (2026-09-10 에디터 전수 확인)

이 문서가 원래 기대값으로 적어둔 `17개/58,400`은 **낡은 숫자**였다. 근거였던
`level_new_kadex_0811/new_kadex_0811_forest_perf.md`(2026-08)는 **PCG 액터 5개** 기준이었는데,
그 뒤 레벨에서 plant 액터가 **2개 → 8개**로 늘었고 그 문서에 없던 `TreeCollisionProxyBuilder`의
프록시 ISM도 있다. 실제 소유자는 액터 11개 / 컴포넌트 50개다(레벨 액터 총 517개 중):

| 소유 액터 | 컴포넌트 | 내용 |
|---|---|---|
| `BP_SplineForest_tree_C_1`, `_C_2` | 3 + 3 | `HISM_SM_BHF_BirchTreeTinnyA` / `ISM_SM_BHF_BirchTreeA` / `ISM_SM_Scots_Pine_Forest_02` |
| `BP_SplineForest_plant_C_{1,2,3,4,5,7,9,11}` (8개) | 각 5 = 40 | `Fern_Broad_01` / `Fern_Broad_Group_01` / `_Group_02` / **`Pine_Rock_Small_01`** / **`Pine_Rock_Small_03`** |
| `TreeCollisionProxyBuilder_1` | 4 | 나무 콜리전 프록시 |

**무관한 인스턴스 메시가 섞인 건 없다** — 나무가 아닌 게 두 종류 잡히지만 둘 다 문제가 안 된다:

- **콜리전 프록시 4개**는 `SetVisibility(false)` + `bHiddenInGame = true`
  (`Tools/TreeCollisionProxyBuilder.cpp:391-392`)로 **렌더 자체를 안 한다.** WPO 거리·LOD 배율을
  걸어도 완전한 no-op이다.
- **작은 바위 2종**(`Pine_Rock_Small_01/03`)은 plant PCG 그래프가 고사리와 같이 뿌리는 숲 스캐터라
  같은 액터에 들어 있다. WPO 거리는 바위 머티리얼에 WPO가 없으니 무의미하고, LOD 배율 0.5는
  바위도 절반 거리에서 LOD를 넘긴다 — 숲 성능 목적과 방향이 같고 크기도 작아 그대로 뒀다.
  **분리하고 싶으면 메시 이름 필터가 아니라 컴포넌트 태그를 붙이는 쪽이 안전하다**(PCG
  재생성 때 이름은 바뀔 수 있다).

> 기대값을 다시 쓸 일이 있으면 **문서의 옛 인벤토리가 아니라 이 실측(50개/94,936)을 기준으로 할 것.**
> 숲을 더 뿌리면 또 늘어난다 — 개수 자체보다 **소유 액터가 전부 `BP_SplineForest_*`/프록시인지**가
> 판정 기준이다.

### 대기

| 항목 | 내용 |
|---|---|
| 값 변경의 실제 효과 | 나무 WPO/LOD를 로비에서 바꾼 뒤 레벨에 들어가 fps가 실측 곡선대로 움직이는지(§5) |
| **VSync** | 에디터에선 구조적으로 안 먹는다(엔진 `BaseEngine.ini [SystemSettingsEditor] r.VSync=0`). **패키지에서만** 확인 가능 |
| **Linux 패키지 룩** | 반사 Lumen→SSR 전환 + Lumen 튜닝 플랫폼 통일로 **원경 GI/반사가 바뀐다.** 개선 방향으로 예상하나 실측 필요 |
| 나머지 품질 축 | 시야거리·이펙트·셰이딩·포스트프로세스·AA품질 — 막힌 멤버 없음(감사 확인). 시각 확인은 안 함 |

> **참고 — 탭이 열릴 때 목록이 두 번 만들어진다.** `RefreshGraphicsList()`가 `NativeConstruct`와
> `HandleGraphicsTabClicked()` 양쪽에서 불려서, 위젯을 열고 Graphics 탭을 누르면 23행이 두 번
> 생성된다(로그에도 두 줄씩 찍힌다). 기능 문제는 없고 비용도 무시할 수준이라 그대로 뒀다 —
> 신경 쓰이면 `NativeConstruct` 쪽을 "현재 활성 탭이 Graphics일 때만"으로 좁히면 된다.

---

## 9. 남은 것 / 열린 항목

- **보류(축·RTSP와 얽힘)**: 렌더 스케일, 캡쳐 해상도. 캡쳐 해상도는 축 선택 화면 값과 실제로 겹치고,
  RTSP는 `BeginPlay`에 SDP가 확정돼 런타임 변경이 구조적으로 불가하다 — 조사 문서 §10-4.
- **제외(사용자 확정)**: 프레임레이트 상한(물리 결정성 대책), 창 모드(운용이 `-fullscreen` 런치 인자),
  레벨 PostProcessVolume(노출 기준이 VFX nit 트랙과 얽힘).
- **텍스처 풀 크기**: `r.Streaming.PoolSize=4096` / `LimitPoolSizeToVRAM=1`을 RendererSettings에 유지.
  이관하지 않기로 결정했다 — 막힌 2개는 나머지 7개와 기능이 겹치고(`MipBias`가 같은 목적을 더 직접
  수행), 실제 성능 레버인 `MaxAnisotropy`는 애초에 막혀 있지 않다. 풀 크기는 fps가 아니라 VRAM↔화질
  트레이드라 전시 PC 사양이 고정이면 내릴 이유도 없다.
- **나무 LOD 배율의 종별 구분 상실**: 현재 레벨 저작값은 자작나무 2종에만 0.5이고 소나무 등은 1.0인데,
  이 설정은 모든 인스턴스 메시에 같은 배율을 적용한다. 종별로 다르게 하려면 메시 이름 필터가 필요하고
  그건 레벨/PCG 저작 쪽 관심사다.
- **캡쳐 주기 "매 틱" 선택 시 충돌**: 드론(Slot=1)과 전장(Slot=0)은 Count가 모두 2 이상이면 패리티가
  갈려 절대 같은 프레임에 안 겹친다(선택지가 전부 2의 거듭제곱이라 성립). 다만 한쪽을 "매 틱"(=1)로
  두면 매 프레임 찍으니 반드시 겹친다 — UI 문구 보완 여부는 미정.
