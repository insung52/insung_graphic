# 나나이트(Nanite) 해설 + titan_example 프로젝트 영향 분석

2026-09-28 / 완료(조사 전용, 프로젝트 변경 없음) / 나나이트 원리·Nanite Foliage 현황 설명과, 켰을 때 이 프로젝트의 시스템별 영향(최대 발견: 지금 VSM이 실제로는 꺼져 있음)과 단계별 검증 계획.

> 조사 범위: 엔진 소스 `C:\Program Files\Epic Games\UE_5.8\Engine`(이하 `Engine/`), 프로젝트 코드/Config, P4 이력,
> 개발 문서, 실행 중인 에디터(unreal-mcp **읽기 전용** 조회만), 웹 문서.
> 표기: **[확인: 근거]** = 소스·파일·로그·공식 문서로 확인. **[추정]** = 추론, 실측 필요.
> 소스와 웹 글이 어긋나면 소스를 따랐다.
> **프로젝트 소스·Config·에셋은 하나도 바꾸지 않았다. 빌드도 돌리지 않았다.**

---

## 0. 3줄 요약 + 핵심 발견

**나나이트를 3줄로 설명하면:**
1. 메시를 128삼각형짜리 "클러스터" 조각으로 잘게 나누고, 조각들을 여러 단계의 단순화 트리(DAG)로 미리 구워 둔다.
2. 매 프레임 GPU가 화면 픽셀 크기에 맞는 조각만 골라 그린다. 그래서 LOD를 손으로 만들 필요가 없고, LOD 전환 팝핑도 없다.
3. 대가도 있다. 켜는 순간 고정비(스트리밍 풀 512MB와 뷰당 컬링/래스터 패스)가 붙고, 알파컷(Masked)이나 WPO(바람)처럼 "프로그래머블"한 머티리얼에서는 이득이 크게 줄어든다.

**이번 조사의 핵심 발견 (중요도순):**

| # | 발견 | 근거 |
|---|---|---|
| **F1** | **지금 이 프로젝트는 VSM(Virtual Shadow Maps)을 실제로 안 쓰고 있다.** `r.Shadow.Virtual.Enable=1`이 설정돼 있어도 엔진은 VSM에 "플랫폼 나나이트 지원"을 요구하는데, `r.Nanite.ProjectEnabled=False`라서 이 조건에서 탈락한다. 그래서 전부 기존 방식 그림자맵(CSM)으로 돈다. `DefaultScalability.ini [ShadowQuality@2]`의 VSM 튜닝 3줄도 현재는 효과가 없다. **거꾸로 말하면, 나나이트를 켜면 게임 전체의 그림자 방식이 CSM에서 VSM으로 바뀐다.** 숲만의 변화가 아니다. | [확인: `Engine/Source/Runtime/RenderCore/Private/RenderUtils.cpp:1498-1505` `DoesPlatformSupportVirtualShadowMaps = ProjectEnabled && DoesPlatformSupportNanite(Platform)`, `:1520-1527` `UseVirtualShadowMaps = r.Shadow.Virtual.Enable && DoesPlatformSupportVirtualShadowMaps && DoesRuntimeSupportNanite(...)`, `:1273-1281` ProjectEnabled=0이면 즉시 false]. 런타임 실측은 아직 없다(§C 0단계). |
| **F2** | **리눅스 패키지 "나나이트 메시 233종 지오메트리 증발" 우려는 현재 설정에서는 성립하지 않는다.** 폴백 제거(strip)는 대상 플랫폼이 나나이트를 지원할 때만 일어나는데, ProjectEnabled=0이면 "미지원"으로 판정되기 때문이다. **대신 나나이트를 켜는 순간 이 위험이 실제가 된다.** 리눅스는 폴백이 쿡에서 빠지고, 런타임에 나나이트가 못 돌면(SM5로 뜸, 64비트 아토믹 미지원, 반투명 슬롯 등) 해당 메시가 **아예 안 그려진다.** | [확인: `Engine/Source/Runtime/Engine/Private/StaticMesh.cpp:209-217` strip 조건, `RenderUtils.cpp:1436-1453` `DoesTargetPlatformSupportNanite`, `Engine/Public/StaticMeshComponentHelper.h:528-588` 폴백이 없으면 프록시를 만들지 않고 경고만 남김] |
| **F3** | **지금 "나나이트 플래그 on" 메시들은 벤더 원본이 아니라 자동 단순화된 폴백으로 렌더되고 있다.** 소나무는 53,511 tris 원본이 LOD0에서 7,069로 그려지고, 건물 `b_01`은 7,130 tris가 **872**로 그려진다. "LOD0부터 품질이 낮다"는 기존 관찰의 원인이 이것이다. | [확인: MCP `get_asset_tags` — `NaniteTriangles` vs `Triangles`, `Engine/Source/Developer/MeshBuilder/Private/StaticMeshBuilder.cpp:173-205`] |
| **F4** | **나나이트를 켜도 지금 가진 나무 에셋으로는 "균일한 고품질 숲"이 되지 않는다.** 소나무는 원본 자체가 알파카드 기반 53k tris라 나나이트는 이걸 쪼개서 단순화할 뿐이고, 먼 거리에서는 카드가 뭉개져 숲이 듬성해질 위험이 있다(이 문제를 푸는 게 5.7+의 Voxel). 가장 무거운 자작나무(`MWPaperBirchForest`)는 **나나이트 off**다. 진짜 목표(수만 그루 균일 품질)에는 **Nanite Foliage 전용 에셋**(Megaplants/PVE: 지오메트리 잎 + Assembly + Voxel + Skinning 바람)이 필요한데, 이 경로는 **5.8에서도 전부 Experimental**이고 **5.8/5.8.1 패키지 빌드 크래시 버그(UE-386321, 5.8.2 수정 예정)**가 있다. | [확인: MCP 태그, Epic Nanite Foliage 문서, 포럼 트래커 — §A-7, §B-11] |
| **F5** | 씬캡쳐는 캡쳐 하나마다 **나나이트 컬링과 래스터 전체를 따로** 돈다. 나나이트는 해상도가 작아도 줄지 않는 고정비가 있어서, 320x180 CCTV 4방 같은 작은 캡쳐가 상대적으로 비싸진다. 다행히 이 프로젝트 캡쳐는 전부 `bAlwaysPersistRenderingState=true`라 2-pass 오클루전(이전 프레임 HZB)과 VSM 캐시가 캡쳐별로 유지될 조건은 갖췄다. | [확인: `Engine/Source/Runtime/Engine/Private/Components/SceneCaptureComponent.cpp:407-432`, `Engine/Source/Runtime/Renderer/Private/Nanite/NaniteCullRaster.cpp:4089-4091`, 프로젝트 `RCWSComponent.cpp:141` 등] |

---

# Part A — 나나이트란 무엇인가 (입문자용, 그러나 정확하게)

## A-1. 기존 방식의 문제

기존 렌더링은 "메시 하나 = 드로우콜 하나 = 정점 셰이더가 모든 정점을 처리"하는 구조다. 멀리 있는 물체를 싸게 그리려면
사람이 **LOD0~LOD4를 미리 만들어 두고** 화면 크기 임계값(screen size)으로 통째로 갈아끼운다.
이 프로젝트가 겪은 문제가 전부 여기서 나왔다.
- 벤더 LOD3에서 LOD4로 넘어갈 때 **다른 나무(x-tree 카드)**로 바뀐다.
- 자동 감산 LOD는 "머리숱 빠진" 모양이 된다.
- 성능 때문에 가까이서도 LOD를 강제로 낮춘다(`InstanceLODDistanceScale=0.5`).

## A-2. 나나이트의 핵심 아이디어 — "메시를 지도처럼 타일링"

비유하자면 구글 지도다. 지구 전체 위성사진을 한 장으로 받지 않는다. 줌 레벨별 타일을 미리 구워 두고, 지금 화면에 필요한
해상도의 타일만 받아서 붙인다. 나나이트가 메시에 하는 일이 이것이다.

1. **클러스터**: 임포트할 때 메시를 약 128삼각형짜리 조각(클러스터)으로 자른다.
2. **클러스터 DAG**: 이웃 클러스터를 묶어 절반으로 단순화하고 다시 쪼개는 과정을 반복해서, 여러 단계의 단순화 계층을 만든다.
   기존 LOD와 다른 점은 **"메시 전체"가 아니라 "조각별로" 단계를 고른다는 것**이다. 같은 나무라도 카메라 쪽 가지는 고해상도,
   반대쪽은 저해상도로 그릴 수 있고, 경계가 이어지도록 설계돼 있어 **팝핑이 없다.**
3. **LOD 선택 기준은 "픽셀당 오차"**: 조각의 단순화 오차가 화면에서 약 1픽셀(`r.Nanite.MaxPixelsPerEdge=1.0`) 이하가 되는
   가장 거친 단계를 고른다. [확인: `Engine/Source/Runtime/Renderer/Private/Nanite/NaniteShared.cpp:195-203` —
   `LODScale = ViewToPixels / MaxPixelsPerEdge`, `ViewToPixels`는 투영행렬(FOV)과 뷰 높이(픽셀)에서 나온다]
   → **줌(FOV 축소)하거나 해상도가 높으면 자동으로 더 촘촘한 조각을 쓴다.** RCWS 16배 줌에 유리한 성질이다.
4. **GPU 주도 컬링**: CPU가 드로우콜을 만드는 게 아니다. GPU가 인스턴스 → 클러스터 순으로 절두체 컬링과
   오클루전 컬링(HZB, 이전 프레임 깊이 피라미드)을 직접 한다. 두 번째 패스에서는 "이전 프레임에 가려졌던 것"을 다시 검사한다(2-pass occlusion).
5. **SW/HW 래스터**: 픽셀보다 작은 삼각형은 하드웨어 래스터라이저가 비효율적이다. 그래서 작은 삼각형은 컴퓨트 셰이더
   소프트웨어 래스터로, 큰 삼각형은 HW 래스터로 보낸다(`r.Nanite.MinPixelsPerEdgeHW=32`). [확인: 엔진 에이전트 조사,
   `Engine/Source/Runtime/Renderer/Private/Nanite/NaniteCullRaster.cpp:133-137`]
6. **비지빌리티 버퍼**: 래스터 단계에서는 머티리얼을 계산하지 않는다. 픽셀마다 "어느 인스턴스의 어느 삼각형인가"만
   64비트로 기록한다(그래서 **64비트 이미지 아토믹**이 필수다). 셰이딩은 나중에 화면 공간에서 머티리얼별로 한 번에 한다.
   → 오버드로(겹쳐 그리기) 비용이 셰이딩에서 사라진다.
7. **스트리밍**: 모든 클러스터를 VRAM에 올리지 않는다. 필요한 페이지만 디스크에서 스트리밍 풀
   (`r.Nanite.Streaming.StreamingPoolSize=512MB`, 고정 예약)로 올린다. [확인: `Engine/Source/Runtime/Engine/Private/Rendering/NaniteStreamingManager.cpp:58-65`]

## A-3. "전통적 LOD"를 무엇이 대체하나

| 기존 | 나나이트 |
|---|---|
| LOD0~N 메시, screen size 임계값 | 클러스터 DAG, 픽셀 오차 기준 자동 선택 |
| `InstanceLODDistanceScale`, `ForcedLodModel`, `MinLOD`, `r.StaticMeshLODDistanceScale` | **나나이트 렌더에는 영향 없음.** `ForcedLodModel`/`MinLOD`는 폴백/레이트레이싱 메시에만 쓰이고, `InstanceLODDistanceScale`은 NaniteResources.cpp에서 참조조차 안 된다 [확인: 엔진 에이전트, `NaniteResources.cpp:891,1047-1049,1817-1834`] |
| 해당 없음 | `r.Nanite.MaxPixelsPerEdge`(전역 디테일), `r.Nanite.ViewMeshLODBias.*`(TSR 등 업스케일 시 보정, `DeferredShadingRenderer.cpp:381-393,1637-1644`) |
| HISM 클러스터 트리 | 쓰지 않음. GPU Scene 인스턴스 컬링 [추정: NaniteResources.cpp에 참조 없음] |
| 디더 LOD 전환(`DitheredLODTransition`) | 의미 없음(전환 자체가 없음) |
| `InstanceEndCullDistance` | **유지된다** [확인: `NaniteResources.cpp:1846-1858`] |

`SceneCaptureComponent::LODDistanceFactor`도 나나이트에서는 **클러스터 LOD에는 안 쓰이고 컬링 판정(최소 반경/거리 컬링)에만**
쓰인다 [확인: `NaniteShared.cpp:212-213,249,269`].

## A-4. 나나이트와 다른 기능의 상호작용

| 기능 | 나나이트에서 | 근거 |
|---|---|---|
| **Opaque** | 가장 빠른 경로("고정 기능" 래스터 빈) | [확인: `Engine/Source/Runtime/Engine/Public/NaniteSceneProxy.h:266-278`] |
| **Masked(알파컷)** | 지원. 단 **픽셀 프로그래머블 래스터**라서 래스터 단계에서 머티리얼 오파시티를 평가해야 한다. Epic: *"Masked materials are fairly expensive compared to Opaque ones"*. 잘려 나간 픽셀도 거의 같은 비용이 든다 | [확인: 같은 곳 + https://dev.epicgames.com/documentation/unreal-engine/working-with-naniteenabled-content] |
| **WPO(바람)** | 지원. 단 **정점 프로그래머블 래스터**라서 정점마다 WPO를 계산한다. 비용은 정점 수에 비례한다. 컴포넌트의 `WorldPositionOffsetDisableDistance`, `bEvaluateWorldPositionOffset`는 **나나이트도 그대로 존중**한다. 머티리얼의 `MaxWorldPositionOffsetDisplacement=0`이면 컬링 바운드가 WPO만큼 늘어나지 않아서 흔들린 가지가 잘리거나 그림자가 튈 수 있다 | [확인: `NaniteResources.cpp:923,939,759-768`, `Engine/Source/Runtime/Engine/Public/Materials/Material.h:1214-1220`, `NaniteCullRaster.cpp:293-297` `r.Nanite.Culling.WPODisableDistance`] |
| **PDO(Pixel Depth Offset)** | 지원(픽셀 프로그래머블). 테셀레이션이 켜지면 무시된다 | [확인: `NaniteSceneProxy.h:266-278`, Epic 문서] |
| **Two-sided** | 래스터 빈을 늘리지 않는다. 컬링 모드만 바뀐다 | [확인: `NaniteSceneProxy.h:275-276` 주석] |
| **Translucent** | **미지원**(`r.Nanite.AllowTranslucency=0`, "Heavy WIP"). 반투명 슬롯이 **하나라도** 있으면 머티리얼 감사(audit)가 실패해서 **컴포넌트 전체가 폴백 메시로** 그려진다. 폴백이 쿡에서 빠졌으면 **아무것도 안 그려진다** | [확인: `NaniteResources.cpp:112-116`, `Engine/Source/Runtime/Engine/Public/Rendering/NaniteResourcesHelper.h:269,309-312`] |
| 데칼 | 일반 디퍼드 데칼은 나나이트 표면에도 투영된다. **메시 데칼(mesh decal)**은 미지원 | [확인: Epic Nanite 문서] |
| **CustomDepth / Stencil** | 지원(`r.Nanite.CustomDepth=1`). 별도 컬/래스터 패스로 그리고 스텐실 쓰기 퍼뮤테이션도 있다 | [확인: `NaniteResources.cpp:206-212`, `Engine/Source/Runtime/Renderer/Private/CustomDepthRendering.cpp:350-425`, `Nanite/NaniteComposition.cpp:147-186`] |
| **VSM** | **VSM은 나나이트가 켜진 플랫폼에서만 동작한다(F1).** 나나이트 지오메트리는 VSM 페이지에 효율적으로 래스터되고, **정적인 페이지는 캐시돼서 다음 프레임에 재사용**된다. WPO/PDO/스켈레탈 변형은 해당 페이지를 **매 프레임 무효화**한다. 비나나이트 지오메트리는 VSM에서 *"much more expensive"* | [확인: `RenderUtils.cpp:1498-1527`, https://dev.epicgames.com/documentation/en-us/unreal-engine/virtual-shadow-maps-in-unreal-engine] |
| **Lumen** | Lumen 서피스 캐시(카드) 캡쳐가 나나이트 멀티뷰 래스터를 쓴다(카드 갱신이 싸진다). SW 트레이싱은 메시 SDF/글로벌 SDF를 쓰고(나나이트와 무관), HW RT는 **폴백 메시**를 쓴다 | [확인: https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-technical-details-in-unreal-engine] |
| **TSR** | 나나이트 자체는 TSR을 요구하지 않는다. 다만 얇은 잎과 **Voxel**의 확률적 노멀은 템포럴 누적 없이는 노이즈가 심하다. `r.Nanite.Foliage`가 켜지면 TSR 얇은 지오메트리 감지가 추가된다 | [확인: `Engine/Source/Runtime/Renderer/Private/PostProcess/TemporalSuperResolution.cpp:488-501`; 노이즈는 커뮤니티 https://forums.unrealengine.com/t/nanite-voxelization-is-noisy/2702720] |
| **ISM/HISM** | 같은 메시를 몇만 개 뿌려도 나나이트 데이터는 **메시 1개분**이다. HISM은 `bCreateNanite`일 때 `Nanite::FSceneProxy`를 만든다 | [확인: `Engine/Source/Runtime/Engine/Private/HierarchicalInstancedStaticMesh.cpp:2993-3004`] |
| **스켈레탈 메시** | 5.5+ 나나이트 스키닝 지원(`r.Nanite.AllowSkinnedMeshes=1`, `r.SkinnedMesh.RenderNanite=1`). 메시별로 `NaniteSettings.bEnabled`를 켜야 적용된다 | [확인: `NaniteResources.cpp:106-110`, `Engine/Source/Runtime/Engine/Private/Components/SkinnedMeshComponent.cpp:128-151`] |
| **랜드스케이프 나나이트** | `ALandscapeProxy::bEnableNanite`(생성된 static mesh로 렌더), `landscape.RenderNanite=1` | [확인: `Engine/Source/Runtime/Landscape/Classes/LandscapeProxy.h:489-491`] |
| **테셀레이션/디스플레이스먼트** | `r.Nanite.Tessellation=1`(Scalability), 머티리얼 `bEnableTessellation`. NaniteDisplacedMesh 플러그인은 Beta | [확인: `NaniteCullRaster.cpp:93-102`, uplugin] |
| 미지원 | 포워드 셰이딩, MSAA, VR 스테레오, 분할 화면, SingleLayerWater, 비-Surface 도메인 | [확인: `RenderUtils.cpp:1425-1434`, `NaniteResources.cpp:3402-3410`, Epic 문서] |

## A-5. 폴백 메시(Fallback mesh)란

나나이트 메시를 빌드하면 **"나나이트가 못 도는 상황에서 대신 쓸 일반 메시"**를 LOD0 자리에 같이 만든다. 이게 폴백이다
[확인: `StaticMeshBuilder.cpp:201-203` "the fractional Nanite cut"]. 기본 설정은 `FallbackTarget=Auto` →
`r.Nanite.Builder.FallbackTargetAutoRelativeError=1.0`이라, **원본보다 상당히 단순화된 메시**가 된다
(소나무 53,511→7,069, 건물 b_01 7,130→872. `NaniteFallbackPercent=100` 표시와 상관없이 상대오차 기준이 적용된다) [확인: MCP 태그, `NaniteBuilder.cpp:36-42`].

폴백이 쓰이는 곳:
- 프로젝트 나나이트 off(**지금 이 프로젝트**) — 모든 나나이트 메시가 폴백 + 원래 LOD1~N 체인으로 렌더된다.
- 하드웨어/드라이버가 64비트 아토믹을 지원하지 않을 때, SM5로 떴을 때.
- 반투명 등 머티리얼 감사 실패.
- HW 레이트레이싱, 물리 복합 콜리전(LOD0 렌더 데이터 기반, `StaticMesh.cpp:9262-9300`).

### "프로젝트 나나이트 off" vs "메시만 나나이트 on"의 실제 동작 [확인: 엔진 에이전트 조사 + 위 소스]
1. `r.Nanite.ProjectEnabled=0`이면 `DoesPlatformSupportNanite()`가 false다(`RenderUtils.cpp:1273-1281`).
   - 스트리머에 등록하지 않는다(`NaniteResources.cpp:336-339`).
   - 쿡할 때 나나이트 벌크 데이터를 **빼고**(`NaniteResources.cpp:393-398`), 폴백 LOD는 **항상** 넣는다.
   - **VSM도 꺼진다(F1).**
2. 메시에 나나이트 데이터가 있어도 컴포넌트는 `FStaticMeshSceneProxy`(ISM/HISM 프록시)로 폴백 LOD를 그린다.
3. 폴백마저 없으면 경고 "Unable to create a proxy ... doesn't have a fallback mesh"를 남기고 **렌더 안 함**(`StaticMeshComponentHelper.h:528-588`).
   에디터는 항상 폴백을 에뮬레이션해서 보여 주므로(`StaticMesh.cpp:3040-3056`) **쿡된 빌드에서만** 드러난다.

## A-6. `r.Nanite.ProjectEnabled` 자체

- `ECVF_ReadOnly`이고 시작할 때만 읽힌다. 에디터 재시작이 필요하다(프로세스 재시작). [확인: `RenderUtils.cpp:31-37`]
- *"cannot be used to force Nanite on on unsupported platforms"* — 지원 플랫폼에서 끄는 스위치일 뿐이다.
- 셰이더 퍼뮤테이션에 영향을 주므로, 켜면 **셰이더 재컴파일과 전체 재쿡**이 필요하다.

## A-7. Nanite Foliage (5.7에서 도입, 5.8 현재 Experimental)

5.7에서 "나나이트로 진짜 숲을 그리자"를 목표로 들어온 기능 묶음이다. **전부 Experimental** — Epic 문서: *"use caution when shipping with it"*
[확인: https://dev.epicgames.com/documentation/en-us/unreal-engine/nanite-foliage, 로드맵 카드 "Nanite Foliage & Skinning (Experimental)"].

| 구성요소 | 무엇 | 게이트/상태 |
|---|---|---|
| **Nanite Assemblies** | 한 나무를 "가지/잎 파트 인스턴스들의 조립체"로 저장한다(메시 안의 마이크로 인스턴싱, 최대 65k 파트). 수백만 삼각형 나무를 디스크/메모리 몇십 MB로 줄인다(Epic 예: 3.5GB → 29MB) | `r.Nanite.Foliage` 또는 `r.Nanite.AllowAssemblies`(기본 0) [확인: `RenderUtils.cpp:1370-1377`, `NaniteResources.cpp:124`]. 지원이 꺼져 있으면 빌드 실패(`StaticMeshBuilder.cpp:405-409`) |
| **Nanite Voxels** | 멀리서 잎 클러스터가 픽셀 크기가 되면, 삼각형 단순화(=듬성해짐) 대신 **복셀 표현**으로 바꿔 부피감과 밀도를 유지한다. LOD 메시가 필요 없다 | `r.Nanite.Foliage` 또는 `r.Nanite.AllowVoxels`(기본 0) [확인: `RenderUtils.cpp:1379-1386`]. 메시 `NaniteSettings.shapePreservation`(Voxelize) — 소나무는 현재 `None` [확인: MCP] |
| **Nanite Skinning + DynamicWind** | WPO 대신 **본(bone) 스키닝**으로 나무를 흔든다. 바운드가 정확하고 비용이 정점 수와 무관하다(Epic: 10만 본에 GPU 약 0.1ms). DynamicWind 플러그인이 전역 바람으로 본을 구동한다(`FSkinningTransformProvider`) | `Engine/Plugins/Experimental/DynamicWind/DynamicWind.uplugin`: `"VersionName": "0.1"`, *"Extremely experimental dynamic wind support for Nanite foliage."*, `"IsExperimentalVersion": true` [확인]. 제약(Epic): 전역 풍향만 지원, 충돌/근접 물리 반응 없음 |
| **PVE (Procedural Vegetation Editor)** | 나무를 절차적으로 생성·편집(성장, 시드, 접목)해서 위 포맷으로 출력한다 | `ProceduralVegetationEditor.uplugin`: `"IsExperimentalVersion": true`, Win64/Linux/Mac, DynamicWind·PCG·Dataflow에 의존 [확인]. **5.7 PVE 에셋은 5.8에서 안 열린다**(5.8 릴리스노트) |
| **Megaplants** | Quixel/Fab 제공 에셋. Baltic Pine(=Scots pine, *Pinus sylvestris*), Silver Birch 등. PVE 편집, Nanite Foliage, DynamicWind 호환 | https://www.fab.com/listings/a2b04e81-5075-479f-a9d2-4940022f330a 등 |
| `r.Nanite.Foliage` | 위 전부를 켜는 프로젝트 스위치(Project Settings "Nanite Foliage (Experimental)"). 기본 0, **ReadOnly**. 셰이더 키에 들어가므로 켜면 글로벌 셰이더 재컴파일 | [확인: `NaniteResources.cpp:118-122`, `Engine/Source/Runtime/RenderCore/Private/Shader.cpp:2966-2968`] |

중요한 설계 전제(Epic 문서): **Nanite Foliage는 알파 마스킹과 WPO 머티리얼을 전제로 하지 않는다.**
잎을 실제 지오메트리로 만들고 바람은 스키닝으로 처리하는 모델이다. 즉 **현재의 알파카드 + WPO(MPC_Wind) 나무를
그대로 두고 스위치만 켜서 얻는 기능이 아니다.**

---

# Part B — titan_example 프로젝트 영향 분석

## B-0. 현재 설정 스냅샷 [확인: `Config/DefaultEngine.ini`]

| 키 | 값 | 줄 |
|---|---|---|
| `r.Nanite.ProjectEnabled` | False | :117 |
| `[LinuxTargetSettings] +TargetedRHIs` | SF_VULKAN_SM6 (엔진 기본 SM5도 유지 → **SM5+SM6 둘 다 타깃**) | :172 (+`Engine/Config/BaseEngine.ini:3428-3429`) |
| `bGenerateNaniteFallbackMeshes` | False (Linux) | :173 |
| Windows | DX12, `+D3D12TargetedShaderFormats=PCD3D_SM6` | :135-140 |
| `r.Shadow.Virtual.Enable` | 1 (**그러나 F1에 의해 실효 없음**) | :100 |
| Lumen GI / SSR / `r.Lumen.TraceMeshSDFs=0` / HWRT off | | :80-110 |
| `r.Substrate` | True | :110 |
| `r.AntiAliasingMethod` | 4 (TSR) | :46 |
| `r.Streaming.PoolSize` | 4096 | :78 |
| 에디터 로그 | `Set CVar [[r.Nanite.ProjectEnabled:0]]`, `r.Nanite.Streaming.ReservedResources:1`(엔진 `BaseWindowsEngine.ini` 기본값이지 프로젝트 설정이 아님 — 기존 문서의 "DefaultEngine.ini에서 켜둠"은 정정) | `Saved/Logs/titan_example.log` |

## B-1. 나나이트를 왜 껐나 — 기록된 사실 vs 추측

**P4 이력(2026-07-22, 전부 user5@DESKTOP-JUNYOUNG)** [확인: `p4 describe` / `p4 diff2`, 설명은 CP949 디코딩]

| CL | 설명(원문) | 실제 변경 |
|---|---|---|
| 287 (09:29) | "dx12,sm6 -> dx11,sm5 로 변경 (**고급기능이 더 추가되서 vram 부족및 퍼포먼스 하락이 커지는 문제로 인해**)" | `r.RayTracing=True→False`, `RayTracingProxies.ProjectEnabled=True→False`, D3D12 SM6 주석 처리 + D3D11 SM5 |
| **288** (10:04) | "나나이트, 루멘 off, scalability setting 최적화" | `r.Nanite.ProjectEnabled=False` **추가**, `r.Shadow.Virtual.Enable=1→0`, Lumen GI/반사 → 0, **D3D12 SM6 다시 활성화**, Linux `bGenerateNaniteFallbackMeshes=False` **추가**, WindowsEngine.ini에 sg.* + VSM 예산(MaxPhysicalPages=512) |
| 289 (10:19) | "lumen on" | (Lumen 복구) |
| 292 (15:46) | "lumen 최적화, **vram오류 덜나도록**" | `r.Streaming.PoolSize=4096` + LimitPoolSizeToVRAM, Lumen 업데이트 예산. 영문 주석: *"RealBiomes ... the pool was **almost certainly** the real source of the VRAM pressure — Nanite just tipped it over the edge"*, *"See conversation for defaults"* |
| 303 (07-24, user4) | "씬캡쳐 vs 실제 렌더링 색감/음영 불일치 문제 해결" | `r.Shadow.Virtual.Enable=0→1` 복구 (**F1 때문에 실제로는 VSM이 안 켜졌을 것** [추정: 당시 런타임 확인 기록 없음]) |
| 343 (08-03, user4) | "hardware raytracing 옵션 해제(blas 메모리 잡아먹는 문제)" | 이후 HWRT off 유지 |

**해석**
- [확인] 당시 문제는 **VRAM 오류와 성능 하락**이다. 같은 날 HWRT(+RT 프록시), 나나이트, 루멘, VSM, SM6/SM5를 **동시에** 여러 번 뒤집었다.
- [확인] CL 292의 "나나이트는 마지막 한 방울"은 **나나이트를 이미 끈 뒤에** 쓴 주석이다. "almost certainly" / "See conversation"이라는 표현을 보면 AI 대화에서 나온 **가설**이다. **나나이트의 VRAM 기여를 분리 측정한 기록은 어디에도 없다.**
- [추정] 7/22 당시 VRAM 압박의 주범 후보는 ① 텍스처 스트리밍 풀 무제한(CL 292에서 캡을 걸어 해소) ② HWRT BLAS(CL 343에서 원인으로 확인) ③ Lumen이다. 나나이트 고유 비용은 스트리밍 풀 512MB에 버퍼 수십 MB 정도로 추정된다(§B-10).
- [확인] 2026-08-22 숲 성능 조사(`level_new_kadex_0811/new_kadex_0811_forest_perf.md`)의 "나나이트는 답이 아니다(2~3fps)"는 **나나이트를 켠 측정이 아니다.** 나나이트 on 메시(소나무)를 **폴백 상태에서 숨겨 본** 측정이라, 나나이트를 켰을 때의 성능을 말해 주지 않는다. 그 문서의 "Movable이라 VSM 캐시 불가"도 **VSM 자체가 꺼져 있다**는 F1로 전제가 바뀐다.
- **결정 권한**: CL 288 작성자(user5 / DESKTOP-JUNYOUNG)에게 당시 증상(어느 PC, VRAM 몇 GB, 어떤 경고)을 확인해야 한다.

## B-2. 시스템별 영향표

위험도: 🔴 높음(깨지거나 납품 영향) / 🟠 중간(실측 필요, 성능·화질 변동) / 🟢 낮음·무관

### (1) 리눅스 패키징 / Vulkan — 🔴

| 항목 | 내용 |
|---|---|
| 현재 상태 | Linux 타깃 SM5+SM6, 나나이트 off → 폴백은 **strip되지 않는다**(F2). LIG PC는 NVIDIA 595.84 고정. 사내 검증기는 RTX 4070 SUPER + 595.84 `.run` (`packaging/2026-09-15_linux_nvidia_driver_595_run_install.md`) |
| 나나이트 지원 조건 | VULKAN_SM6만 `bSupportsNanite=true`, VULKAN_SM5는 false [확인: `Engine/Config/VulkanPC/DataDrivenPlatformInfo.ini:65,190`]. 런타임에는 `VK_KHR_shader_atomic_int64` + `VK_EXT_shader_image_atomic_int64`, 그리고 R64_UINT UAV 지원이 필요하다(없으면 `GRHISupportsAtomicUInt64=false`) [확인: `Engine/Source/Runtime/VulkanRHI/Private/VulkanExtensions.cpp:510-536`, `VulkanDevice.cpp:825-828`]. RHI는 타깃 목록에서 가장 높은 레벨부터 Vulkan 프로파일(`VP_UE_Vulkan_SM6`)을 검사해서 통과하는 첫 레벨로 뜬다. `-sm5`로 강제할 수 있다 [확인: `Engine/Source/Runtime/RHI/Private/Linux/LinuxDynamicRHI.cpp:23-31`] |
| 나나이트 ON 시 | `bGenerateNaniteFallbackMeshes=False`가 **처음으로 실효를 갖는다.** 나나이트 메시 약 252종(§B-9)의 폴백이 쿡에서 빠진다. 이후 ① SM5로 뜨거나 ② 아토믹 미지원 ③ 반투명 슬롯 ④ `r.Nanite 0` 콘솔 등으로 나나이트가 안 돌면 **해당 메시가 증발한다.** 화면 경고 "fallback meshes were stripped during cooking…"이 뜬다 [확인: `Engine/Source/Runtime/Renderer/Private/SceneRendering.cpp:4547,4796-4799`]. 추가로 VSM이 켜지고 셰이더/쿡 크기가 바뀐다 |
| 위험 | 🔴 — 폴백 strip은 되돌리려면 재쿡이 필요하다. **나나이트를 켤 거면 `bGenerateNaniteFallbackMeshes`를 기본값(True)으로 되돌리는 것이 안전하다**(폴백 용량만 조금 늘어남). 595.84 + Vulkan 나나이트 안정성은 커뮤니티 보고가 없다. 같은 595 계열 + Blackwell(RTX 50) 환경에서 UE 5.7 Vulkan Xid 31 크래시 보고가 있다(나나이트 무관, https://forums.developer.nvidia.com/t/xid-31-vulkan-crash-on-5070ti-nvidia-open-unreal-engine-5-7/370489). **LIG PC의 GPU 모델은 문서에서 확인하지 못했다** |

### (2) RTSP / SceneCapture — 🟠(성능) ~ 🔴(자체방호 7스트림 동시일 때)

| 항목 | 내용 |
|---|---|
| 현재 상태 | 캡쳐는 전부 수동 `CaptureScene()`, `bCaptureEveryFrame=false`, **`bAlwaysPersistRenderingState=true`**, `ShowFlags.TemporalAA=true` [확인: `Source/titan_example/Vehicles/RCWSComponent.cpp:132,141,329`, `Vehicles/TitanTruck.cpp:141-144`, `Drone/DronePawn.cpp:350-356`, `Plugins/QuadCamModule/.../QuadCamComponent.cpp:156`]. 해상도: RCWS 1920x1080(메인 뷰포트 경로), CCTV 320x180 x4, 자체방호 RCWS 1280x720 / 환경 640x360 / UAV 640x360 [확인: `Vehicles/StreamResolutionSubsystem.h`]. 캡쳐 GI/반사는 엔진이 None으로 리셋하고 `SceneCaptureViewParity`가 PPV 오버라이드로 복구한다 |
| 나나이트 ON 시 | ① **캡쳐 1회 = 나나이트 인스턴스 컬링(숲 약 95k 인스턴스) + 클러스터 컬링 + 래스터 + 머티리얼 빈별 셰이딩을 통째로 1회 더** 돈다. 캡쳐 전용 경량 모드나 나나이트 전용 캡쳐 cvar는 없다 [확인: `Engine/Shaders/Private/Nanite/NaniteInstanceCulling.usf:44,91-93` — 캡쳐 플래그는 `bHiddenInSceneCapture` 필터뿐]. 이 고정비는 **해상도에 비례해 줄지 않는다** → 320x180 CCTV의 상대 비용이 오른다 [추정]. ② ViewState가 있어서 이전 프레임 HZB가 남아 **2-pass 오클루전이 동작한다**(ViewState가 없으면 오클루전 없이 1패스) [확인: `SceneCaptureComponent.cpp:407-432`, `Engine/Source/Runtime/Renderer/Private/DeferredShadingRenderer.cpp:603-612,1655-1657`, `NaniteCullRaster.cpp:4089-4091`]. 단, **라운드로빈으로 N틱마다 캡쳐하므로 "이전 HZB"가 N프레임 전 것**이라 카메라가 빨리 움직이면 오클루전 효율이 떨어진다 [추정]. ③ **VSM이 캡쳐 뷰마다 켜진다.** 클립맵 캐시는 ViewState 키로 분리된다 [확인: `Engine/Source/Runtime/Renderer/Private/VirtualShadowMaps/VirtualShadowMapClipmap.cpp:329-333`]. 물리 페이지 풀(`[ShadowQuality@2] r.Shadow.Virtual.MaxPhysicalPages=1024`, 엔진 기본 4096)은 씬 전체가 공유하는 캐시 매니저 소속이라 **메인 뷰 + 캡쳐 N개가 1024페이지를 나눠 쓴다** → 페이지 부족 시 그림자 해상도 저하 경고/깜빡임 위험 [추정: 풀 공유 구조는 `RendererScene.cpp:7300-7303` 확인, 부족 여부는 실측 필요]. ④ 나나이트 LOD가 캡쳐의 FOV와 해상도에 자동으로 맞춰진다 — RCWS 줌에 유리하고, CCTV는 자동으로 거칠어진다(좋은 쪽) [확인: A-2 3번] |
| 위험 | 🟠 — 커뮤니티에 "UE5 씬캡쳐가 UE4 대비 3배 비쌈", "캡쳐 3개로 80→20fps" 보고가 있다(나나이트 특정 아님). 5.6에서 "SceneCapture2D가 있으면 나나이트 스켈레탈이 애니메이션 안 됨" 보고가 있다(https://forums.unrealengine.com/t/eu5-6-bug-scene-capture-2d-and-skeletal-mesh-with-nanite-not-get-animate/2666843) → Nanite Foliage 스키닝(바람)이 캡쳐 영상에서 멈출 가능성이 있어 **필수 검증 항목**이다. 캡쳐가 켜진 상태로 **stat gpu 비교**가 판단의 핵심이다 |

### (3) 탐지 / CustomDepth / 픽셀 리드백 — 🟢

| 항목 | 내용 |
|---|---|
| 현재 상태 | `UTargetDetectionComponent`는 **CPU 라인트레이스**(`LineTraceSingleByChannel(ECC_Visibility)`)와 투영 수학만 쓴다. CustomDepth/Stencil/ReadPixels 사용은 **소스 전체에 0건**이다 [확인: `Source/titan_example/Detection/TargetDetectionComponent.cpp:438,523-550`, grep 결과] |
| 나나이트 ON 시 | 트레이스는 물리 콜리전 기준이라 렌더 방식과 무관하다. 나무 콜리전은 `TreeCollisionProxyBuilder`의 원기둥/복사 메시 프록시다. 복합 콜리전은 에디터 렌더 데이터 LOD(`LODForCollision`)에서 굽는다 [확인: `StaticMesh.cpp:9262-9300`]. 나중에 CustomDepth 하이라이트를 쓰게 돼도 나나이트가 지원한다 |
| 위험 | 🟢 |

### (4) 머티리얼 — 🟠

| 항목 | 내용 |
|---|---|
| 현재 상태 | 소나무 잎 `MI_Scots_Pine_Sheet_01`(부모 `M_VegetationShader`)과 자작 잎 `MTL_BHF_BirchLeaves`: **BLEND_Masked, MSM_TwoSidedFoliage, TwoSided, DitheredLODTransition=true, MaxWorldPositionOffsetDisplacement=0** [확인: MCP get_properties]. 바람은 머티리얼 WPO + `MPC_Wind`(`sfx_vfx/wind_system.md`). Substrate on |
| 나나이트 ON 시 | ① 잎이 **Masked + WPO라 래스터가 픽셀·정점 양쪽 모두 프로그래머블**이다. 나나이트 경로 중 가장 비싼 조합이다. ② `MaxWorldPositionOffsetDisplacement=0` → WPO로 흔들린 부분이 컬링 바운드 밖으로 나가 잘림이나 그림자 튐이 생길 수 있다. **나나이트로 갈 경우 값 설정이 필요하다** [확인: `Material.h:1214-1220`]. ③ Dithered LOD 전환은 무의미해진다. ④ Substrate: 나나이트 셰이딩이 Substrate GBuffer를 직접 쓰고, 나나이트 고유의 Substrate 제약은 소스에서 찾지 못했다 [확인: `Nanite/NaniteShading.cpp:1747-1759`; "제약 없음"은 추정]. ⑤ **반투명 슬롯이 있는 나나이트 메시는 폴백으로 떨어지고, 리눅스에서 폴백이 strip되면 증발한다** — 252종 전수 점검이 필요하다(건물 `b_01`은 Opaque로 확인). ⑥ Nanite Foliage 에셋은 알파/WPO가 아니라 지오메트리 잎 + 스키닝 바람이라, **현 `AWindSource`/`MPC_Wind` 연동을 DynamicWind(전역 풍향만)로 옮기거나 병행 설계**해야 한다 |
| 위험 | 🟠 |

### (5) 그림자 / VSM / Lumen — 🔴(F1 때문에 변화 폭이 크다)

| 항목 | 내용 |
|---|---|
| 현재 상태 | **실효 그림자 = CSM**(F1). `[ShadowQuality@2]`의 VSM 3줄(MaxPhysicalPages=1024, ResolutionLodBias*)은 현재 **무효** [확인: `Config/DefaultScalability.ini:52-55` + `RenderUtils.cpp:1520-1527`]. 참고 실측 "sg.ShadowQuality 2→0 에서 50→57fps"(DefaultScalability.ini:49 주석)는 **CSM 기준 수치**다. Lumen GI on(SW, 메시 SDF 트레이스 off), 반사 SSR |
| 나나이트 ON 시 | ① **모든 레벨, 모든 뷰(메인 + 캡쳐)의 그림자가 VSM으로 바뀐다.** 그림자 룩(선명도, 컨택트)과 비용 구조가 통째로 바뀐다. 차량/병사(비나나이트 스켈레탈)는 VSM에서 비나나이트 경로라 비싸다(Epic: "much more expensive"). ② 숲: WPO 나무는 WPO 거리(30m) 안쪽 페이지를 매 프레임 무효화하고, 바깥은 WPO를 안 하므로 **캐시된다**. 지금 Movable이라도 VSM 캐시는 "변형 여부" 기준으로 판단한다(Shadow Cache Invalidation Behavior) [확인: Epic VSM 문서]. 5.7+ `UseReceiverMaskDirectional`이 캐시 오버라이드를 깨는 보고가 있다 [커뮤니티]. ③ Lumen: 카드 캡쳐가 나나이트 래스터로 싸진다. CL 292 주석의 "LumenSceneUpdate가 프레임의 약 34%"였던 부분이 개선될 여지가 있다 [추정]. SW 트레이싱은 SDF 기반이라 무관하다 |
| 위험 | 🔴 — 나나이트 평가는 사실상 **"나나이트 + VSM 전환" 평가**다. 그림자 룩 변화에 대해 디자인팀 승인이 필요하다. VSM만 끄고 나나이트만 켜는 조합(`r.Shadow.Virtual.Enable=0`)도 가능하니 **A/B 매트릭스**로 봐야 한다(§C) |

### (6) 그래픽 설정 UI (`UTitanGraphicsSettings`) — 🟠

| 항목 | 내용 |
|---|---|
| 현재 상태 | `FoliageWpoDisableDistance`(기본 3000)와 `FoliageLodDistanceScale`(기본 0.5)을 월드의 전 ISM/HISM에 `SetWorldPositionOffsetDisableDistance`/`SetLODDistanceScale`로 밀어 넣는다 [확인: `Source/titan_example/Settings/TitanGraphicsSettings.cpp:253-293`, `.h:167-189`] |
| 나나이트 ON 시 | **WPO 거리 → 계속 유효**(나나이트도 컴포넌트 WPO disable distance를 존중, `NaniteResources.cpp:1860-1863`). **LOD 배율 → 나나이트 메시에는 무의미**(`InstanceLODDistanceScale` 미참조). 비나나이트로 남는 메시(자작나무 등)에는 계속 유효하다. 대체 노브 후보: `r.Nanite.MaxPixelsPerEdge`(1→2면 삼각형 약 1/4) [추정: 비율은 경험칙], `r.Nanite.ViewMeshLODBias.Offset`, Nanite Foliage라면 5.8의 "Nanite Pixel Programmable Distance"(5.8 릴리스노트). `ShadowQuality`는 이제 VSM 3줄이 **처음으로 먹게** 된다 |
| 위험 | 🟠 — 주석과 실측 곡선(WPO/LOD fps표)이 전부 폴백·CSM 기준이라 재측정이 필요하다 |

### (7) 차량 / 스켈레탈 / 캐릭터 — 🟢~🟠

| 항목 | 내용 |
|---|---|
| 현재 상태 | `BP_UGV_0901`, `Titan_Truck`(983,577 tris, LOD 1개, 나나이트 off), SoldierLab 병사(GASP 스켈레탈), `DronePawn`. 나나이트 on인 `/Game/Vehicles/UGV_OLD/TankX`는 미사용, `Data_Smith/Titan/titantruck/.../Turret_height`는 on [확인: MCP find_assets] |
| 나나이트 ON 시 | 스켈레탈은 메시별 플래그를 켜지 않는 한 기존 경로 그대로다. `Titan_Truck`는 나나이트를 켜면 백만 tris 문제를 LOD 없이 해결할 수 있는 **대표적 수혜 후보**다(단 에셋 변경은 별도 결정). `Turret_height`가 트럭 포탑이라면 나나이트/폴백 품질 차이가 가까이서 보일 수 있다 [추정]. 차량 그림자는 VSM 비나나이트 경로로 바뀐다(5번 항목) |
| 위험 | 🟢(기능) / 🟠(그림자 비용) |

### (8) 콜리전 / 내비메시 / PCG / TreeCollisionProxyBuilder / Chronicle / QuadCam / 기타 — 🟢

| 시스템 | 판단 | 근거 |
|---|---|---|
| 물리 콜리전 | 무관 — 단순 콜리전 또는 에디터 LOD 기반 복합 콜리전 | [확인: `StaticMesh.cpp:9262-9300`] |
| 내비메시 | 무관 — 콜리전 기반, 3층 구조(`project_new_kadex_navmesh`) | [추정: 내비 생성은 렌더 데이터를 쓰지 않음] |
| PCG | 무관 — 스포너는 컴포넌트를 만들 뿐이고 나나이트 여부는 메시 속성. 단 PCG 디스크립터의 `InstanceLODDistanceScale` 기본값은 나나이트 메시에 무의미 | 위 §A-3 |
| `TreeCollisionProxyBuilder` | 무관 — 프록시 ISM은 `/Engine/BasicShapes/Cylinder` 또는 원본 메시 복사, 렌더 숨김 | [확인: `Source/titan_example/Tools/TreeCollisionProxyBuilder.cpp:18,368-392`] |
| Chronicle 리플레이 / QuadCam 플러그인 | LOD/렌더 데이터 의존 코드 없음(grep 0건). QuadCam은 (2)의 씬캡쳐 영향만 받음 | [확인: grep `ForcedLod|MinLOD|LODDistanceScale|Nanite`] |
| Pixel Streaming | 프로젝트에 플러그인 없음 | [확인: `titan_example.uproject`] |
| `forcedLodModel` 사용 | 코드에 없음. 레벨에서 ISM에 쓰면 0.2fps 폭락 이력(폴백 경로). 나나이트 프록시에서는 무시됨 | [확인: forest_perf.md, `NaniteResources.cpp:891`] |
| RtspEncoder / NVENC | 나나이트와 무관. 단 GPU 시간이 늘면 인코드 제출 타이밍에 간접 영향 | [추정] |

### (9) 에셋 인벤토리 — 🟠 [확인: MCP `find_assets(/Game/, StaticMesh, NaniteEnabled=True)` 2026-09-28]

- **나나이트 on 스태틱 메시 약 252개**(2026-08 조사의 233개에서 증가).
  - RealBiomes 98, NiagaraExamples 42, Data_Smith 28(건물 b_*, 도로 d_road_02/03, 문, 텐트, 병사 소품, 트럭 포탑), `/Game/path/...` 30, NGrassPack 22, LevelPrototyping 11, MWPaperBirchForest **바위만** 8, Megascans 6, Fab 6, TankX 1.
- **숲의 주력 나무**
  - `SM_Scots_Pine_Forest_02`: 나나이트 on. 나나이트 53,511 tris / 폴백 7,069. `GenerateFallback=PlatformDefault`(→ 리눅스 strip 대상), `shapePreservation=None`(복셀화 안 함). LOD threshold는 사용자 수동값 `[1, 0.99, 0.7, 0.4, 0.2]`.
  - `SM_BHF_BirchTreeA`: **나나이트 off**, LOD0 105,913 tris.
  - `SM_BHF_BirchTreeTinnyA`: off.
  - 자작나무 팩의 나무 메시는 전부 off.
- 메모리 노트 `project_pine_lod_impostor_todo.md`의 "Nanite를 끄고"는 **메시 플래그 기준으로는 사실이 아니다**(지금도 on). 프로젝트가 off라서 폴백으로 도는 상태를 가리킨 것으로 보인다.
- 스켈레탈 메시의 나나이트 on 여부는 조회하지 않았다(미확인).

### (10) 성능 기대치 — 🟠 (핵심은 "측정 전에는 모른다"이고, 방향성은 아래와 같다)

**현재 기준선** [확인: forest_perf.md, 메모리 노트]
- 숲 PIE 약 31fps(WPO 30m + 자작나무 LOD 0.5). 숲을 통째로 숨기면 48~58fps.
- 비용은 대부분 자작나무 105k tris LOD 체인 + WPO 정점 연산 + 알파컷 오버드로.
- `t.MaxFPS=60`. 실제 레벨 목표는 30fps대.

**나나이트가 이기는 부분** [추정, 원리 기반]
- 고폴리 opaque 메시(트럭 98만 tris, 건물, 바위): LOD 없이 픽셀 비례 비용.
- 수만 인스턴스의 CPU 드로우콜/HISM 트리 비용 → GPU 컬링.
- VSM 캐시가 정적 나나이트 지오메트리의 그림자를 거의 공짜로 만든다.
- LOD 팝핑과 "다른 나무로 바뀜" 문제는 **구조적으로 해소**된다.

**나나이트가 지는·비기는 부분** [확인: Epic 문서 / 추정]
- 알파카드 잎: 마스크드 프로그래머블 래스터라 이득이 작다.
- 30m 안의 WPO: 정점 수 비례 비용이 그대로이고, 나나이트는 가까이서 고해상도 클러스터를 쓰므로 **오히려 WPO 정점 수가 늘 수 있다.**
- VSM 전환 비용.
- 씬캡쳐마다 붙는 고정비.
- 스트리밍 풀 512MB(8GB GPU 7,895MB 중).

**언제 켤 가치가 있나**
- (a) 숲을 Nanite Foliage 에셋(지오메트리 잎 + 스키닝)으로 교체할 계획이 있거나,
- (b) VSM으로 그림자 품질을 올리고 싶거나,
- (c) 고폴리 opaque 에셋이 늘어날 때.

지금 에셋 그대로 스위치만 켜는 것은 **"그림자 방식 전환 + 소나무/고사리/바위만 나나이트, 자작나무는 비나나이트 VSM"**이라는 가장 애매한 조합이다.

**VRAM** [확인·추정 혼합]

증가분 추정 합계: **약 600~700MB**(기존 문서 550~600MB에 VSM 풀 포함) [추정]

| 항목 | 크기 | 근거 |
|---|---|---|
| 스트리밍 풀 | 512MB 고정 | [확인: `NaniteStreamingManager.cpp:58-65`] |
| 지오메트리 | 이 레벨 약 1.5MB, 전체 252종 약 150MB 디스크 | [확인: forest_perf.md 태그 합] |
| 나나이트 고정 버퍼 | MaxCandidateClusters 16M / VisibleClusters 4M / Nodes 2M 등, 수십 MB급 | [확인: `NaniteShared.cpp:38-69`, 크기는 추정] |
| **VSM 물리 페이지 풀** | 1024페이지 × 128² × 4B ≈ 64MB + HZB/메타 | [추정] |

RTSP 인코더 버퍼와 합산한 여유는 **실측으로만** 판정할 수 있다.

### (11) Experimental 위험 (전시 납품 빌드 기준) — 🔴

| 항목 | 상태 | 근거 |
|---|---|---|
| Nanite Foliage / Assemblies / Voxels | 5.7, 5.8 모두 Experimental. Epic: "use caution when shipping" | Epic 문서 |
| DynamicWind | v0.1, "Extremely experimental" | [확인: uplugin] |
| PVE | Experimental. **5.7 에셋은 5.8에서 안 열림**(엔진 버전 올릴 때마다 호환이 깨질 수 있음) | [확인: uplugin, 5.8 릴리스노트] |
| **패키지 크래시** | PVE/Megaplants 나나이트 폴리지 메시를 쓰면 **5.8·5.8.1 패키지가 시작 시 크래시**(`FPVLightDetection::BuildLeafMeshGeometry` → `RawIndexBuffer.h:198`), UE-386321, 5.8.2 수정 예정 | https://forums.unrealengine.com/t/procedural-vegetation-nanite-foliage-meshes-crash-packaged-project/2730886 |
| Voxel 품질 | 트렁크 구멍(5.7.4), 노이즈(TSR 필수) | 커뮤니티 |
| Assembly 배치 GPU 크래시 | RTX 5070 Ti, 5.7.4 | 커뮤니티, Epic 응답 없음 |
| 리눅스 | Nanite Foliage 리눅스 사례 보고 0건(성공·실패 모두) | 조사 결과 |
| 나나이트 코어(Foliage 제외) | 5.0부터 프로덕션. 리눅스 Vulkan SM6 공식 지원 | Epic HW 요구사항 문서 |

**판단**
- **나나이트 코어**는 납품 빌드에 쓸 수 있는 성숙도다. 남은 조건은 리눅스 SM6 실측과 폴백 정책.
- **Nanite Foliage 스택**은 현 엔진 버전(5.8.x)에서 전시 납품용으로 쓰기에 위험하다. 최소한 5.8.2 적용과 리눅스 패키지 장시간(12시간급, `project_scenario_flow_log_failsafe` 수준) 무인 실행 검증이 선행돼야 한다.

---

# Part C — 권고와 검증 계획

## C-1. 권고 (요약)

1. **메인 프로젝트 설정은 지금 건드리지 않는다.** 이 문서는 조사 결과일 뿐 변경 권고가 아니다.
2. 나나이트 도입 여부를 **두 개의 독립된 결정**으로 나눈다.
   - **결정 ①: 나나이트 코어 + VSM 전환**(에셋 교체 없이). 그림자 룩과 성능, 캡쳐 비용, 리눅스 안정성으로 판단한다.
   - **결정 ②: 숲 에셋을 Nanite Foliage(Megaplants Baltic Pine / Silver Birch + DynamicWind)로 교체.** 품질 목표에 대한 진짜 답이지만 Experimental이고, 5.8.2 이상, 바람 시스템 재설계, 캡쳐 스키닝 검증이 선결 조건이다.
3. 나나이트를 켜는 결정을 하면 **같은 변경에서 `bGenerateNaniteFallbackMeshes=False`를 제거**(기본 True)하는 것을 강하게 권한다(F2).
4. 기존 문서 정정 대상(정리 세션용 메모)
   - `new_kadex_0811_forest_perf.md`의 "리눅스 233종 지오메트리 없음 가능성" → 현재 설정에서는 발생하지 않음(F2).
   - "Movable이라 VSM 캐시 불가" → VSM 자체가 꺼져 있음(F1).
   - "ReservedResources는 DefaultEngine.ini에서 켜둠" → 엔진 기본값임.
   - `graphics_settings_analysis.md` / `DefaultScalability.ini` 주석의 VSM 튜닝 → 현재 무효(F1).
   - **guide/ 갱신**: 그래픽 설정 레퍼런스가 guide/에 생기면 F1을 반영할 것. 이번 작업은 동작을 바꾸지 않았으므로 guide 수정은 하지 않았다.

## C-2. 단계별 검증 계획

모든 단계는 **메인 프로젝트가 아니라 복사본**(`titan_example_nanite_test` 같은 별도 워크스페이스 또는 P4 shelve)에서 한다.
롤백 = 복사본 폐기. `r.Nanite.ProjectEnabled`와 `r.Nanite.Foliage`는 ReadOnly라 **ini 수정 + 에디터 재시작**이 필요하다.

### 0단계 — 현재 상태 확인 (메인 프로젝트, 읽기만, 10분)
- PIE에서 `r.Shadow.Virtual.Visualize 1` / `stat ShadowRendering` / `stat gpu`로 **VSM이 실제로 도는지** 확인한다(F1 런타임 검증). `ShadowDepths`(CSM)가 보이고 VSM 패스가 없으면 F1 확정.
- 리눅스 패키지 로그에서 RHI 피처 레벨(`Vulkan SM6` vs `SM5`)과 `64bit atomics` 관련 줄을 확인한다(현재 어느 레벨로 뜨는지).
- **판정**: 결과를 이 문서에 추가 기록한다.

### 1단계 — 윈도우 복사본, 나나이트 코어만 (에셋 무변경)
- **설정**: `r.Nanite.ProjectEnabled=True`, 폴백 기본값 유지.
- **측정 매트릭스**(New_kadex_0811, 동일 RCWS 시야, RTSP 5스트림 켠 상태, forest_perf.md의 로그 프레임 카운터 방식 + `stat gpu` + `stat Nanite`):

| 케이스 | 나나이트 | VSM |
|---|---|---|
| A (기준선) | off | (실효 off) |
| B | on | `r.Shadow.Virtual.Enable=0` (CSM 유지) |
| C | on | on |

- **측정 항목**: fps, GPU ms(BasePass/ShadowDepths/VSM/Nanite/Lumen), 캡쳐 1회당 GPU ms(캡쳐 on/off 차이), VRAM(`stat RHI`, `r.Nanite.Streaming.*` 통계), VSM 페이지 부족 경고 여부, 화질(소나무/건물 근접, RCWS 16배 줌, CCTV).
- **통과 기준(안)**: B나 C가 A 대비 fps -10% 이내 **또는** 화질 이득이 명확할 것. VRAM 여유 1GB 이상. 캡쳐 GPU 합계가 30fps 예산(33ms) 안.

### 2단계 — 자작나무 1종 나나이트 변환 A/B (복사본)
- **설정**: `SM_BHF_BirchTreeA` 복제본에 나나이트를 켜서 ISM 1개에만 교체한다. `MaxWorldPositionOffsetDisplacement`를 설정한다.
- **측정**: 자작나무가 병목이던 곳에서 비나나이트(LOD 0.5) 대 나나이트를 비교한다. 먼 거리 잎이 듬성해지는지(알파카드 단순화)를 육안 비교한다.

### 3단계 — 리눅스 패키지 (복사본, SM6)
- **설정**: 1단계의 최선 조합으로 쿡한다. **폴백은 유지**(True).
- **대상 머신**: 사내 RTX 4070 SUPER + 595.84 `.run`.
- **측정**
  - 기동 로그: SM6, 나나이트 활성, 경고 없음.
  - 나나이트 메시가 전부 보이는지 확인한다. `-sm5` 강제 실행 시 **폴백으로 보이는지**(폴백 유지 확인).
  - RTSP 12스트림 fps와 인코더 지연.
  - VRAM(`nvidia-smi`).
  - **12시간 무인 시나리오 루프**에서 크래시/Xid 0건.
- **통과 기준**: 크래시 0, 메시 증발 0, RTSP 목표 fps 유지.

### 4단계 — Nanite Foliage 시험 (별도 테스트 프로젝트 권장)
- **전제**: 엔진 5.8.2+(UE-386321 수정본).
- **설정**: Megaplants Baltic Pine / Silver Birch, `r.Nanite.Foliage=1`, DynamicWind.
- **측정**
  - 수천에서 수만 그루 배치 시 fps와 VRAM.
  - **씬캡쳐에서 바람(스키닝)이 움직이는지.**
  - 캡쳐 영상의 Voxel 노이즈(TSR 히스토리 유무).
  - 리눅스 패키지 기동 + 12시간 루프.
  - 기존 `AWindSource`와의 풍향 연동 가능성.
- **판정**: 전시 일정 대비 리스크 수용 여부를 팀이 결정한다.

**롤백 수단**: 모든 단계가 복사본이다. 메인 적용 시에도 ini 2~3줄 되돌리기 + 재쿡으로 복귀할 수 있다(에셋 플래그 변경은 P4 revert).

## C-3. 사용자·팀이 내려야 할 결정 목록

1. **CL 288 작성자(user5 / DESKTOP-JUNYOUNG) 확인**: 7/22 당시 VRAM 오류가 어느 PC(GPU/VRAM)에서 어떤 증상이었는지, 나나이트 단독 원인이었는지.
2. **그림자 방식**: 나나이트를 켜면 VSM이 따라 켜진다. VSM 룩을 원하는가, CSM을 유지할 것인가(`r.Shadow.Virtual.Enable=0` 병행 가능).
3. **범위**: 결정 ①(코어)만 할지, 결정 ②(Nanite Foliage 에셋 교체)까지 할지. ②라면 엔진 5.8.2 업그레이드 일정.
4. **리눅스 폴백 정책**: `bGenerateNaniteFallbackMeshes`를 기본값으로 되돌릴지(권장).
5. **바람 시스템**: Nanite Foliage 도입 시 `AWindSource`/`MPC_Wind`와 DynamicWind(전역 풍향만)의 관계.
6. **LIG 납품 PC 사양 확인**: GPU 모델(현재 문서에는 드라이버 595.84만 확인됨), VRAM.
7. **디자인팀 승인**: 그림자 룩 변화와 숲 외형 변화.

## C-4. 확인하지 못한 것 (Unknowns)

- 런타임에서 VSM이 정말 꺼져 있는지(F1은 소스로 확정했지만 실측은 없다) — 0단계.
- 현재 리눅스 패키지가 SM6/SM5 중 어느 레벨로 뜨는지.
- LIG PC GPU 모델과 VRAM.
- 나나이트 on 252종 중 반투명 슬롯을 가진 메시가 있는지(전수 조사하지 않음. `b_01`만 Opaque 확인).
- 스켈레탈 메시 나나이트 플래그 현황.
- 씬캡쳐 N개와 VSM 물리 페이지 풀(1024) 경합의 실제 영향.
- 씬캡쳐에서 TSR 히스토리가 유지되는지(`ShowFlags.TemporalAA=true` + persistent state → [추정] 유지).
- 모든 성능 수치: 이 문서의 성능 서술은 원리 기반이고, **나나이트를 켠 실측은 프로젝트 역사상 한 번도 없다.**

## 참고

- 엔진 소스: 본문 인라인 인용(`Engine/...:줄`).
- 프로젝트: `Config/DefaultEngine.ini`, `Config/DefaultScalability.ini`, `Source/titan_example/Settings/TitanGraphicsSettings.*`, `Vehicles/RCWSComponent.cpp`, `Vehicles/TitanTruck.cpp`, `Drone/DronePawn.cpp`, `Plugins/QuadCamModule`, `Detection/TargetDetectionComponent.cpp`, `Tools/TreeCollisionProxyBuilder.cpp`.
- 문서: `level_new_kadex_0811/new_kadex_0811_forest_perf.md`, `ui/graphics_settings_analysis.md`, `ui/2026-09-10_graphics_settings_implementation.md`, `sfx_vfx/wind_system.md`, `packaging/2026-09-15_linux_nvidia_driver_595_run_install.md`.
- 웹(공식)
  - https://dev.epicgames.com/documentation/en-us/unreal-engine/nanite-virtualized-geometry-in-unreal-engine
  - https://dev.epicgames.com/documentation/unreal-engine/working-with-naniteenabled-content
  - https://dev.epicgames.com/documentation/en-us/unreal-engine/nanite-foliage
  - https://dev.epicgames.com/documentation/en-us/unreal-engine/virtual-shadow-maps-in-unreal-engine
  - https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-5-8-release-notes
  - https://dev.epicgames.com/documentation/unreal-engine/procedural-vegetation-editor-in-unreal-engine
- 웹(커뮤니티): 본문 각 항목의 링크.
