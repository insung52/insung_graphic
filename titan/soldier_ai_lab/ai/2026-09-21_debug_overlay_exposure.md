# 디버그 오버레이 노출 보정 — 톤매퍼 앞에서 그려지는 선을 EV10 레벨에서도 보이게

2026-09-21 / 완료 (빌드·PIE ✅, 사용자 "아주 잘됨") / 노출이 EV10 에 고정된 사실적 라이팅 레벨에서 **모든 AI 오버레이(와 엔진 내비메시 `P` 뷰)가 숯검정**으로 나왔다. 원인은 디버그 프리미티브가 **톤매퍼 앞**에서 그려져 씬과 똑같이 노출되는데, `DrawDebug*` 는 8-bit `FColor` 라 선형 1.0 위로 못 올라가는 것. 레벨 라이팅/PP 는 건드리지 않고, **뷰가 지난 프레임에 적용한 노출의 역수만큼 밝게** 그린다 — `SoldierDebug::GetExposureScale(World)`(씬 뷰 익스텐션이 `FSceneView::GetLastEyeAdaptationExposure()` 를 매 프레임 읽음) · `SoldierDebug::Bright(World, FColor) → FLinearColor` · 래퍼 `SoldierDebug::Line/Point/Sphere/Circle`(월드 라인 배처, 선형 색) · 새 `USoldierDebugMeshComponent`(`AI/SoldierDebugMesh`, 라인 배처의 메시 경로를 선형 색으로 재구현 — 필드 오버레이의 사각형·라이트 링) · cvar `SoldierLab.Debug.ExposureScale`(0 = 자동, 아니면 수동 배율). `AI/` 의 `DrawDebugLine/Point/Sphere/Circle` 호출 전부 래퍼로 교체(grep 24곳 · 7파일), `DrawDebugString` 은 그대로(화면 글자). **`Squad/` · `Pose/` · `Weapons/` 의 17곳은 다른 세션 몫** → [W97]. 엔진 내비메시 `P` 뷰는 엔진 쪽(`UNavArea::DrawColor` 가 `FColor`)이라 **손대지 않는다**(사용자 지시, 비목표). 원칙 **P181**.

전편: `ai/2026-09-21_per_squad_field_edge_advance_fire_model.md`(같은 날 같은 세션 — 분대 필드·엣지 전진·사격 콘·섀도우 감축; 그 문서 12절이 여기로 포인터) · `ai/2026-09-17_situation_field_lighting_model.md` 17절(오버레이 v2 — 자체 배처 flush+refill, P155; **메시 절반이 이 문서로 옮겨 갔다**) · `ai/2026-09-13_perception_stack.md`(오버레이 공통 규약 `SoldierDebugDraw`, P70).
원칙: **신설 P181**(`CLAUDE.md` 5절) · P7(디버그 1급 시민) · P155(배처 직접 소유) · P10(계측 먼저). 짝: [W97](나머지 폴더의 오버레이 이관).

> 신뢰도: **[A]** 코드로 확인(file:line, 2026-09-21 작업 트리 — `SoldierDebugDraw.h` 92줄 / `.cpp` 304줄 · `SoldierDebugMesh.h` 55줄 / `.cpp` 146줄 · `SoldierSituationField.cpp` 3081줄 / `.h` 616줄 · `SoldierLab.Build.cs` 47줄) · **[B]** 잠정 · **[C]** 미측정. 빌드·PIE 는 사용자 보고("아주 잘됨"). "EV10 에서 선형 1.0 ≈ 흰색의 0.1 %" 는 헤더 주석(`SoldierDebugDraw.h:62-69`)의 서술이지 이 세션이 잰 값이 아니다 [B].

---

## 0. 한 장 요약

| 무엇 | 어디 | 상태 |
|---|---|---|
| **원인** — 디버그 프리미티브는 톤매퍼 **앞**, `DrawDebug*` 와 `ULineBatchComponent::DrawMesh` 는 8-bit `FColor`(선형 1.0 상한), 배처의 선·점만 `FLinearColor` | `SoldierDebugDraw.h:60-69` · `SoldierDebugMesh.h:9-21` | [A] |
| **`SoldierDebug::GetExposureScale(World)`** — `FSoldierDebugExposureExtension : FSceneViewExtensionBase` 가 `SetupView` 에서 `bIsGameView` 뷰의 `GetLastEyeAdaptationExposure()` 를 `GLastGameViewExposure` 에 저장(게임 스레드, 렌더 없음); 첫 호출에 `FSceneViewExtensions::NewExtension` 으로 등록해 프로세스 수명 동안 유지; 반환 = `clamp(1/exposure, 1e-3, 1e6)`, 아직 프레임이 없으면 1 | `SoldierDebugDraw.cpp:26-54` · `:203-217` | [A] · PIE ✅ |
| **cvar `SoldierLab.Debug.ExposureScale 0`** — 0 = 자동(위), 그 외 = 그 값을 배율로 그대로 | `.cpp:18-24` · `:205-208` | [A] |
| **`SoldierDebug::Bright(World, FColor) → FLinearColor`** — `FLinearColor(FColor)`(sRGB → 선형) × 배율, **알파는 안 곱함**(알파는 덮임이지 빛이 아니다) | `.cpp:219-225` | [A] |
| **래퍼 `Line / Point / Sphere / Circle`** — 월드 라인 배처(`GetLineBatcher(World)`)에 한 프레임, `Line` 에 `LifeTime > 0` 이면 `WorldPersistent` 배처(`DrawDebugLine` 과 같은 규칙). `Sphere` 는 대원 세 개(`DrawDebugSphere` 방식), `Circle` 은 `YAxis/ZAxis` 평면(`DrawDebugCircle` 인자) | `.h:77-91` · `.cpp:227-303` | [A] · PIE ✅ |
| **`USoldierDebugMeshComponent`**(신규 `AI/SoldierDebugMesh.{h,cpp}`) — `UPrimitiveComponent` + 자체 `FPrimitiveSceneProxy`; `FMesh{Verts, Indices, FLinearColor Colour, DepthPriority}`; `GetDynamicMeshElements` 가 `FDynamicMeshBuilder` 로 메시를 만들고 **같은 `GEngine->DebugMeshMaterial`** 에 `FColoredMaterialRenderProxy(Parent, Mesh.Colour)`(한 프레임 프록시) — 배처의 메시 경로와 **색 타입만 다르다**; `DrawMesh`/`Flush`(`MarkRenderStateDirty` + `UpdateBounds`), 정점은 월드 공간(변환 identity), 충돌 없음·틱 없음·스트리밍 무시, 뷰 관련성 = 반투명 | `SoldierDebugMesh.h:22-55` · `.cpp:13-146` | [A] · PIE ✅ |
| **필드 오버레이 배선** — `DebugMeshes`(`GetDebugMeshes()`, `RF_Transient`, 서브시스템 `Deinitialize` 에서 해제)에 **셀 사각형**(색 그룹당 1 메시)과 **라이트 링**(annulus); 선·점·구·화살표는 그대로 `DebugBatcher`; 둘 다 같은 틱에 `Flush` + 채우기(P155 유지); 로컬 `HDR` 람다 = `ExposureScale` 곱(알파 제외) | `SoldierSituationField.h:611-615` · `.cpp:106-113` · `:140-159` · `:2630-2656` · `:2932` · `:2969-2970` · `:2995` · `:3000` · `:3033-3034` | [A] · PIE ✅ |
| **`AI/` 호출 전부 래퍼로** — `SoldierCover` 11 · `SoldierObjective` 3 · `SoldierSight` 3 · `SoldierEngagement` 2 · `SoldierPerception` 2 · `SoldierSuppression` 2 · `SoldierComms` 1(= 24, grep `SoldierDebug::(Line\|Point\|Sphere\|Circle)\(`); `AI/` 에 `DrawDebug(Line\|Point\|Sphere\|Circle\|…)` **0건** | 4절 표 | [A] |
| **`Build.cs`** `PrivateDependencyModuleNames` **+ `RenderCore` · `RHI`**(자체 씬 프록시) | `SoldierLab.Build.cs:34-37` | [A] |
| **안 한 것** — `Squad/` 9 · `Pose/` 7 · `Weapons/` 1 은 다른 세션 소유 → [W97] · 엔진 내비메시 `P` 뷰(비목표) · 레벨 라이팅/PP 무변경 | 4.2절 | — |
| **컴파일 수정** — 필드 오버레이의 지역 변수 `Exposure` 가 바깥 것을 가려 **C4456(에러 취급)** → `ExposureScale` 로 개명 | `SoldierSituationField.cpp:2652` | [A] |

---

## 1. 증상과 원인 [A]

**증상**(사용자, `L_SoldierScenario` 류 사실적 라이팅 레벨 — 노출 EV10 고정): `SoldierLab.Debug.*` 오버레이 전부가 **숯검정**. 필드의 셀 사각형도, 엄폐 후보 점도, 교전 사선도. 엔진의 내비메시 `P` 뷰도 마찬가지로 검게 나왔다 — 즉 우리 코드의 색 선택 문제가 아니다.

**원인**(헤더 주석 `SoldierDebugDraw.h:60-69` · `SoldierDebugMesh.h:9-21`):

1. 디버그 프리미티브(라인 배처의 선·점·메시)는 **씬과 함께 톤매퍼 앞에서** 그려진다. 씬의 모든 것과 똑같이 노출된다.
2. EV10 으로 고정된 뷰에서 선형 `(1, 0, 0)` 은 화면 흰색의 **수천 분의 일**이다(주석 서술 [B]). 지면이 200 nit 급으로 잡힌 레벨에서 1.0 은 "어두운 방 안의 촛불"이 아니라 그냥 검정.
3. `DrawDebugLine/Point/Sphere/Circle` 은 **`FColor`(8-bit)** 를 받아 배처에 넘긴다. 1.0 위로 갈 수 없다.
4. `ULineBatchComponent` 는 **선·점은 `FLinearColor`** 로 받는데(`DrawLine`/`DrawPoint`) **메시는 `FColor`** 로 저장한다(`DrawMesh`). 그래서 선은 밝게 넣을 길이 있지만 필드의 사각형(09-18 v2 부터 `DrawMesh`)은 길이 없다.

같은 원인이 titan 본체에 있는 nit 기준 VFX 문제(메모리 `project_vfx_nit_scale` — 이 레벨은 노출 배율 1e-4, 지면 200 nit, VFX 는 nit 단위로 작성)와 한 뿌리다: **이 레벨에서는 "1.0 = 밝다" 가 성립하지 않는다.**

---

## 2. 고려한 것과 기각한 것

| 안 | 무엇 | 왜 기각 |
|---|---|---|
| **(a) 레벨 라이팅/PP 를 낮춘다** | 디버그 볼 때 노출을 풀거나 PPV 를 끈다 | 진단 도구 때문에 **관찰 대상(씬)을 바꾸는 것** — 오버레이를 켠 순간 병사가 보는 세계(빛, 그림자)가 달라진다. 그리고 다른 세션들이 라이팅을 잡아 둔 값(`project_vfx_nit_scale`)을 건드리지 말라는 메모리. **P181** |
| **(b) 수동 배율 상수** | 색에 ×N 을 박는다 | N 은 레벨·시간대마다 다르다(낮 EV10 / 밤). 자동이 기본이어야 하고 수동은 **미세 조정용**으로만 — 그래서 cvar 로 남겼다(`ExposureScale` 0 = 자동, 아니면 그 값) |
| **(c) `DrawDebug*` 그대로 두고 색만 키운다** | — | 8-bit 라 불가능(1절 3) |
| **(d) 엔진 내비메시 `P` 뷰도 고친다** | `UNavArea::DrawColor` 등 | 엔진 쪽(`FColor`)이고 사용자가 **명시적으로 손대지 말라** 함 — **비목표**. 내비메시는 필요하면 노출 낮은 레벨(`L_SoldierTest`)에서 본다 |
| **(e) 채택 — 뷰가 적용한 노출의 역수만큼 밝게** | 지난 프레임 눈 순응(eye adaptation) 결과를 읽어 `1/exposure` 를 색에 곱한다 | 씬 무변경 · 레벨/시간대 무관(자동) · 렌더 코드 0(읽기만) · 선·점은 배처가 이미 선형이라 값만 키우면 되고, 메시는 색 타입만 바꾼 컴포넌트 하나면 된다 |

---

## 3. 기계

### 3.1 노출 읽기 — `GetExposureScale` (`SoldierDebugDraw.cpp:26-54` · `:203-217`)

- `FSoldierDebugExposureExtension : FSceneViewExtensionBase`. `SetupView(ViewFamily, View)` 에서 `View.bIsGameView` 이면 `View.GetLastEyeAdaptationExposure()` 를 읽어 유한·양수일 때만 `GLastGameViewExposure` 에 저장. **게임 스레드**, 아무것도 그리지 않는다 — `FSceneView` 에 뷰 상태가 붙은 채로 볼 수 있는 유일한 훅이라서 이 자리다(주석 `:29-33`).
- 등록은 **첫 `GetExposureScale` 호출**에서 `FSceneViewExtensions::NewExtension<>()`, `TSharedPtr` 정적으로 잡아 **프로세스 수명 동안 유지**(해제 없음 — 몇 바이트짜리고 모든 뷰 패밀리를 본다).
- 반환: cvar 가 0 보다 크면 그 값. 아니면 `GLastGameViewExposure > 0 ? clamp(1/exposure, 1e-3, 1e6) : 1`. **첫 프레임은 1**(아직 읽은 노출이 없다) — 한 프레임 뒤부터 맞는다.
- 값의 뜻: 씬의 색 C 는 화면에 `C × Exposure` 로 나오므로 `C / Exposure` 를 넣으면 C 로 나온다(주석 `:215`).

### 3.2 `Bright` (`:219-225`)

`FLinearColor Linear(Colour)`(**sRGB → 선형** 변환하는 생성자) 의 RGB 에 배율을 곱한다. **알파는 그대로** — 알파는 덮임(coverage)이지 빛이 아니다. 기존 오버레이 코드가 `FColor` 로 색을 정해 둔 것(P70 — 색 = 출처, 굵기 = 신선도)은 한 글자도 안 바뀐다.

### 3.3 래퍼 (`:227-303`)

| 래퍼 | 인자 | 어디에 |
|---|---|---|
| `Line(World, Start, End, FColor, Thickness=0, DepthPriority=0, LifeTime=0)` | `DrawDebugLine` 과 같되 `bPersistent`/`LifeTime` 대신 `LifeTime` 하나 | `LifeTime > 0` → `WorldPersistent` 배처(`DrawDebugLine` 규칙과 동일 — `SoldierComms` 무전선 `CommsDebugDrawSeconds`) · 아니면 `World` 배처 한 프레임 |
| `Point(World, Position, Size, FColor, DepthPriority=0)` | | `World` 배처 `DrawPoint` |
| `Sphere(World, Centre, Radius, Segments, FColor, Thickness=0, DepthPriority=0)` | 대원 세 개(`DrawDebugSphere` 와 같은 모양), `Segments ≥ 4` | 선으로 |
| `Circle(World, Centre, Radius, Segments, FColor, YAxis, ZAxis, Thickness=0, DepthPriority=0)` | `DrawDebugCircle` 의 `YAxis/ZAxis` 평면 | 선으로 |

**없는 것**: `Arrow`(`DrawDebugDirectionalArrow`) · `Box` · `Capsule` · `Cone` — `AI/` 에는 없어서 안 만들었다. `Pose/` 가 화살표를 5곳 쓴다 → [W97] 에서 래퍼를 하나 더 만들거나 `Line` 둘로.

### 3.4 `USoldierDebugMeshComponent` (`AI/SoldierDebugMesh.{h,cpp}`)

라인 배처의 메시 절반을 **색만 선형으로** 재구현한 것.

- `UPrimitiveComponent` 자식. 생성자: `bAutoActivate` · 틱 없음 · `NoCollision` 프로파일 · `bUseEditorCompositing` · 오버랩 없음 · `SetIgnoreStreamingManagerUpdate(true)`(보고할 머티리얼 없음)(`.cpp:94-103`).
- `DrawMesh(Verts, Indices(int32), FLinearColor, DepthPriority=SDPG_World)` — `FMesh` 추가, 인덱스 `uint32` 로, 바운즈 누적, `MarkRenderStateDirty` + `UpdateBounds`. `Flush()` — 비어 있으면 아무것도 안 함(`.cpp:105-135`).
- `CreateSceneProxy()` — 메시가 있을 때만 `FSoldierDebugMeshSceneProxy`(메시 배열 **복사** — 렌더 스레드 사본, `bWillEverBeLit=false`). `GetDynamicMeshElements`: 뷰마다, 메시마다 `FDynamicMeshBuilder` 에 정점(`FColor::White` — 정점 색은 안 쓴다)·삼각형을 넣고 **`GEngine->DebugMeshMaterial` 의 렌더 프록시 위에 `FColoredMaterialRenderProxy(Parent, Mesh.Colour)`** 를 만들어 `Collector.RegisterOneFrameMaterialProxy` — 이 한 줄이 배처와의 유일한 차이(주석 `:65-66`). `GetMesh(FMatrix::Identity, …)` — 정점은 월드 공간(`.cpp:32-72`).
- `GetViewRelevance`: `bDrawRelevance = IsShown(View)` · 동적 · **`bSeparateTranslucency` + `bNormalTranslucency`**(`.cpp:74-82`). `CalcBounds` = 누적 `FBox`(`.cpp:142-146`).

### 3.5 필드 오버레이 배선 (`SoldierSituationField.cpp`)

- 멤버 `DebugMeshes`(`UPROPERTY(Transient)`, `.h:611-613`) · `GetDebugMeshes()`(`.cpp:140-159` — `DebugBatcher` 와 같은 패턴: `NewObject(this, RF_Transient)` + `RegisterComponentWithWorld`) · `Deinitialize` 에서 `UnregisterComponent`(`.cpp:106-113`).
- `DrawDebug`(`.cpp:2627-`): 배처와 메시 둘 다 없으면 반환, 스코프 무효면 둘 다 `Flush`, 아니면 **같은 틱에 둘 다 `Flush` + 채우기**(09-18 P155 그대로 — 빈 프레임 없음). `const float ExposureScale = SoldierDebug::GetExposureScale(World)` + `HDR` 람다(RGB × 배율, 알파 그대로)(`:2650-2656`).
- **메시로 가는 것**: 셀 사각형(색 그룹당 1 메시, 8단계 양자화 유지 — `Meshes->DrawMesh(Verts, Indices, HDR(FLinearColor(Colour)))`, `:2932`) · 라이트 링 annulus(`:2995`). **배처에 남는 것**: 라이트 눈 구(`DrawSphere`, `:2969`) · 지면 핀(`DrawLine`, `:2970`) · 그림자 미완 흰 점(`DrawPoint`, `:3000`) · 볼 곳 보라 화살표(`DrawDirectionalArrow`, `:3033`) — 전부 `HDR(…)` 를 거친다.
- 그래서 `SoldierSituationField.cpp` 의 `#include "DrawDebugHelpers.h"`(`:9`) 는 이제 `DrawDebugString`(헤더 글자) 용이다.

### 3.6 cvar `SoldierLab.Debug.ExposureScale` (`SoldierDebugDraw.cpp:18-24`)

`0`(기본) = 자동 — 매 프레임 뷰의 노출을 읽어 정확히 보정. 그 외 = **그 값을 배율로 그대로**(자동 끔). 용도는 5절의 블룸.

---

## 4. 적용 범위

### 4.1 바꾼 것 — `AI/` 전부 (grep, 24곳 · 7파일)

| 파일 | 래퍼 호출 | 무엇 |
|---|---|---|
| `SoldierCover.cpp` | 11 (`:623` `:1381` `:1454` `:1582` `:2304` `:2309` `:2315` `:2324` `:2331-2332` `:2338`) | 후보 점 · 몸 밴드/경로 광선 · 위험 표본 · best 구 둘 · 눈 점 · 엣지 핀 · 쐐기 두 줄 · 후보 점 |
| `SoldierObjective.cpp` | 3 (`:101-104`) | 반경·밴드 원 + 기둥 |
| `SoldierSight.cpp` | 3 (`:122` `:127` `:199`) | 표적 광선 · 막힌 점 · 콘 스윕 광선 |
| `SoldierEngagement.cpp` | 2 (`:1687-1688`) | 총구 사선 + 총구 점 |
| `SoldierPerception.cpp` | 2 (`:742` `:745`) | 기록 원 + 기둥 |
| `SoldierSuppression.cpp` | 2 (`:188-189`) | 제압 막대 |
| `SoldierComms.cpp` | 1 (`:204`) | 무전선(`LifeTime` = `CommsDebugDrawSeconds` → 영속 배처) |
| `SoldierSituationField.cpp` | (래퍼 아님) | 자체 배처 `HDR(…)` + `DebugMeshes` — 3.5절 |

`DrawDebugString`(`SoldierCover` · `SoldierEngagement` · `SoldierPerception` · `SoldierHealth` 등)은 **그대로** — 캔버스 글자라 톤매퍼 뒤다.

⚠ 브리핑은 "22곳" 이라 했는데 grep 은 **24곳**이다(위 표) — 두 줄에 걸친 호출을 어떻게 셌는가의 차이로 보인다 [B].

### 4.2 안 바꾼 것 — 다른 세션 소유 → [W97]

| 폴더 | 파일 | 호출 | 비고 |
|---|---|---|---|
| `Squad/` | `SoldierZone.cpp` `:145-158` | `DrawDebugCircle` 3 · `DrawDebugLine` 3 | 래퍼 있음(`Circle`/`Line`). 단 `Colour.WithAlpha(80)` 같은 알파는 `Bright` 가 안 건드리므로 그대로 |
| `Squad/` | `SoldierSquadSubsystem.cpp` `:617-646` | `DrawDebugCircle` 2 · `DrawDebugLine` 1 | 래퍼 있음 |
| `Pose/` | `SoldierScanTurnComponent.cpp` `:117-119` | `DrawDebugDirectionalArrow` 2 | **`Arrow` 래퍼 없음** — 만들거나 `Line` 둘 |
| `Pose/` | `SoldierHeadAimComponent.cpp` `:909-927` | `DrawDebugDirectionalArrow` 3 · `DrawDebugSphere` 2 | 위와 같음 |
| `Weapons/` | `SoldierProjectile.cpp` `:251` | `DrawDebugSphere` 1 | 브리핑에 없던 곳 — 투사체 궤적 점(`Impact` 오버레이) |

### 4.3 비목표

- **엔진 내비메시 `P` 뷰** — `UNavArea::DrawColor` 가 `FColor`, 엔진 렌더 코드. 사용자 지시로 손대지 않는다. 필요하면 노출 낮은 레벨에서 본다.
- **레벨 라이팅 · PPV · 노출** — 무변경(2절 (a)).

---

## 5. 주의 · 함정

| 무엇 | 설명 |
|---|---|
| **블룸** | 배율이 곱해진 선은 HDR 값이라 **블룸이 켜진 레벨에서 번질 수 있다**. `SoldierLab.Debug.ExposureScale` 을 자동값보다 **조금 낮게 수동으로** 놓으면 된다(자동값은 헤더에 안 찍힌다 — 필요하면 `[Field]` 헤더에 한 항 추가하는 것이 자연스럽다 [B]). |
| **한 프레임 지연** | `GetLastEyeAdaptationExposure` 는 **지난 프레임** 값. 노출이 급변하는 순간(레벨 로드 첫 프레임, 밤낮 전환)에 한 프레임 어긋난다. 첫 프레임은 배율 1. |
| **영속 배처의 선** | `Line(…, LifeTime > 0)` 은 발행 시점의 배율로 굳는다 — 수명 동안 노출이 바뀌면 안 따라간다(무전선 1.2 s 정도라 무해). |
| **C4456** | 필드 오버레이의 지역 `Exposure` 가 같은 이름의 바깥 변수를 가려 **경고 = 에러**로 빌드가 섰다 → `ExposureScale` 로 개명(`SoldierSituationField.cpp:2652`). 이 프로젝트는 C4458/C4456 을 에러로 다룬다(09-18 `C4458 = 에러` 와 같은 자리). |
| **익스텐션 수명** | 프로세스 종료까지 등록된 채 — 에디터에서 PIE 를 여러 번 돌려도 하나. 게임 뷰만 읽으므로 에디터 뷰포트의 노출은 안 섞인다. |
| **알파** | `Bright` 는 알파를 안 곱한다. 배율이 큰 레벨에서 반투명 사각형이 "더 진해 보이면" 알파가 아니라 RGB 가 화면 상한에 몰린 것이다(블룸과 같은 처방 — 배율을 조금 낮춘다) [B]. |
| **다른 폴더의 옛 호출** | [W97] 전까지 `Squad/`·`Pose/`·`Weapons/` 오버레이는 EV10 레벨에서 **계속 검다**. "존 링이 안 보인다"는 이 원인이지 존이 없는 것이 아니다. |

---

## 6. 검증 · 값

- 빌드 ✅ · PIE ✅(사용자 "아주 잘됨" — EV10 레벨에서 필드 사각형·라이트 링·엄폐/교전/시야 오버레이가 의도한 색으로 읽힘).
- **새 튜닝값 없음** — `ExposureScale` 은 디버그 cvar 지 [C] 항목이 아니다.
- 미측정: 자동 배율의 실제 크기(EV10 에서 몇 배인가) · 블룸 번짐이 거슬리는 배율 문턱 — 눈에 띄면 5절.

---

## 7. 변경 파일

```
Source/SoldierLab/AI/SoldierDebugDraw.h/.cpp        GetExposureScale · Bright · Line/Point/Sphere/Circle · FSoldierDebugExposureExtension · cvar SoldierLab.Debug.ExposureScale   (92 / 304줄)
Source/SoldierLab/AI/SoldierDebugMesh.h/.cpp        신규 — USoldierDebugMeshComponent + FSoldierDebugMeshSceneProxy                                                            (55 / 146줄)
Source/SoldierLab/AI/SoldierSituationField.h/.cpp   DebugMeshes · GetDebugMeshes · DrawDebug 의 HDR 람다(ExposureScale, C4456) · 사각형/링 → 메시, 선/점/구/화살표 → 배처 HDR   (3036 → 3081줄)
Source/SoldierLab/AI/SoldierCover.cpp · SoldierObjective.cpp · SoldierSight.cpp · SoldierEngagement.cpp · SoldierPerception.cpp · SoldierSuppression.cpp · SoldierComms.cpp
                                                     DrawDebugLine/Point/Sphere/Circle → SoldierDebug::Line/Point/Sphere/Circle (24곳)
Source/SoldierLab/SoldierLab.Build.cs                PrivateDependencyModuleNames + RenderCore · RHI
```

Perforce: 작업 트리(제출은 사용자). 빌드는 사용자가(메모리 — Build.bat/UBT 안 돌린다).

---

## 8. 원칙

**P181** — **오버레이는 노출을 보정해 그린다. 안 보인다고 레벨 라이팅을 건드리지 않는다.** 디버그 프리미티브는 톤매퍼 앞에서 씬과 같이 노출되므로 EV10 레벨에서 선형 1.0 은 검정이다. 답은 씬을 바꾸는 것이 아니라 뷰가 적용한 노출의 역수만큼 밝게 그리는 것(`SoldierDebug::Bright` · `USoldierDebugMeshComponent`). **SoldierLab 오버레이는 `DrawDebug*` 대신 `SoldierDebug::*` 를 쓴다** — `DrawDebug*` 는 8-bit 라 보정이 불가능하다. `CLAUDE.md` 5절.

---

## 9. 브리핑과 코드가 어긋난 자리 (문서 세션 확인)

- 래퍼 호출 수: 브리핑 "22곳" vs grep **24곳**(4.1절 표).
- 브리핑은 `Squad/` 9 · `Pose/` 7 만 남았다고 했는데 **`Weapons/SoldierProjectile.cpp:251` 에 `DrawDebugSphere` 1곳**이 더 있다 — [W97] 에 포함.
- `Pose/` 의 7곳 중 5곳은 `DrawDebugDirectionalArrow` 인데 **`Arrow` 래퍼가 없다** — [W97] 가 하나 더 만들어야 한다.
- `USoldierDebugMeshComponent::DrawMesh` 는 인덱스를 `TArray<int32>` 로 받아 `uint32` 로 옮긴다(배처 `DrawMesh` 시그니처와 맞추려고) — 브리핑에 없던 세부.
- 필드 오버레이의 라이트 **눈 구·핀·흰 점·화살표는 메시가 아니라 배처**에 남았다(HDR 만 적용) — 브리핑의 "quads and light rings" 와 일치, 그 밖은 배처.
