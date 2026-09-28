# Megaplants + Nanite Foliage 대량 배치 성능 테스트 — 결과

2026-09-28 / 1차 완료 / RTX 4060 Laptop에서 Nanite Foliage 자작나무 **10만 그루가 GPU 7.77ms, Nanite 패스는 그중 1.22ms** — 나무 개수는 더 이상 병목이 아님. 단 **개별 액터 배치는 약 137그루에서 엔진 어서트로 크래시**하므로 인스턴스드 경로(ISKMC/PCG)가 선택이 아닌 필수.

> 인계서: `2026-09-28_megaplants_nanite_foliage_test_handoff.md` (배경·결정·작업 순서)
> Nanite 해설/영향 분석: `2026-09-28_nanite_deep_dive_project_impact.md`

---

## 0. 한 줄 결론

**"1만~4만 그루에서 성능이 버티는가"라는 이번 단계의 질문은 통과. 그것도 큰 마진으로.**
titan_example 숲(약 40,000그루)의 **2.5배인 10만 그루**를 깔았는데 GPU가 프레임 예산의 **47%만** 쓰고 나머지는 놀고 있었다. 화질 쪽 목표(LOD 튐 없음, 잎 반짝임 없음)도 육안으로 달성.

다만 **프로젝트 목표 전체가 끝난 것은 아니다.** 바람·씬캡쳐·리눅스·소나무가 미검증이다. §6 참조.

---

## 1. 테스트 환경

| | |
|---|---|
| 프로젝트 | `C:\working\kadex\anim_test\SoldierLab` (UE **5.8.2**) |
| 레벨 | `/Game/SoldierLab/Treetest` — 5km×5km 평면(`/Engine/BasicShapes/Plane` 스케일 5000) + DirectionalLight + SkyAtmosphere + SkyLight. 그 외 없음 |
| GPU | **NVIDIA GeForce RTX 4060 Laptop GPU**, 드라이버 610.88 (`Chosen D3D12 Adapter Id = 0`) |
| CPU / RAM | AMD Ryzen 7 8845HS (8C/16T) / 32GB |
| OS | Windows 11 25H2 |
| 에셋 | `Content/Megaplant_Library/Tree_Silver_Birch/Tree_Silver_Birch_01/` — `_A` ~ `_D` 4종 스켈레탈 메시. **원본 무수정** |
| 렌더 설정 | `r.Nanite.ProjectEnabled=True`, `r.Nanite.Foliage=True`, VSM on, Lumen GI+반사, TSR |

### 활성화한 플러그인 (이 세션에서 추가)

`SoldierLab.uproject`에 두 개 추가 — 둘 다 엔진 기본값이 `"EnabledByDefault": false`다.

```json
{ "Name": "DynamicWind", "Enabled": true },
{ "Name": "ProceduralVegetationEditor", "Enabled": true }
```

- 엔진에 DLL이 이미 빌드돼 있어 **리빌드 불필요, 에디터 재시작만** 하면 됐다.
- PVE가 의존성으로 `Dataflow`, `GeometryScripting`, `PCG`, `DynamicWind`를 끌고 온다.
- 이번 성능 테스트 자체에는 두 플러그인 모두 **불필요**했다(바람을 안 썼고 재익스포트도 안 했으므로). PVE 에셋을 열거나 바람을 쓸 때만 필요.

### cvar 확인 — 추가 설정 불필요

`r.Nanite.AllowVoxels`와 `r.Nanite.AllowAssemblies`가 **0**이었지만 문제 없었다. 엔진이 OR 게이트다:

```cpp
// Source/Runtime/RenderCore/Private/RenderUtils.cpp:1369-1387
bool NaniteAssembliesSupported() { return bAllowNaniteFoliage || bAllowAssemblies; }
bool NaniteVoxelsSupported()     { return bAllowNaniteFoliage || bAllowVoxels; }
```

`r.Nanite.Foliage=1` 하나로 Voxel·Assembly 둘 다 열린다. `r.Nanite.AllowSkinnedMeshes=1`(기본값)도 필요하며 이미 켜져 있었다.

### 에셋 실측값

| | 본 | 버텍스(LOD0) | Nanite |
|---|---:|---:|---|
| `Tree_Silver_Birch_01_A` | 2,940 | 7,284 | `bEnabled=true`, `ShapePreservation=Voxelize`, Assembly 7파트 |
| `_B` | **8,678** | 6,042 | 동일 |
| `_C` | 4,136 | 5,730 | 동일 |
| `_D` | 1,762 | 4,662 | 동일 |

PVE 익스포트 시점에 이미 Nanite Foliage 설정이 다 되어 있었다. **손댈 것이 없었다.**
`NaniteAssemblyData.parts`는 `SKM_Branch_Silver_Birch_05/06/07` + `_Up_01~04` 7종 파트로 구성.

메시에 `UDynamicWindSkeletalData` AssetUserData도 이미 붙어 있고 `bIsEnabled=true`, 시뮬레이션 그룹 3개(트렁크 2 + 잎 1). 이번 테스트에선 사용하지 않았다.

---

## 2. 측정 결과 — 10만 그루

배치: ISKMC 40개 × 2,500 인스턴스 = **100,000그루**, 3,010m × 3,010m 범위, 랜덤 yaw, 스케일 0.8~1.3.

### stat unit

| | ms |
|---|---:|
| Frame | 16.67 |
| Game | 5.88 |
| Draw | 6.22 |
| RHIT | 2.82 |
| **GPU** | **7.77** |

Draws **171**, Prims **10.2K**, Mem 3.42GB, VRam **3.29GB / 7.02GB**, RenderRes 75.5% (1449×740).

**Frame 16.67ms는 정확히 60fps — vsync/패널 고정이지 성능 한계가 아니다.** 근거는 아래 Queue Total의 Idle이다.

### stat gpu — Graphics Queue 0 (Busy Avg, ms)

```
Queue Total                    7.27   (Max 7.66 / Min 7.12,  Wait 0.44,  Idle 7.40)
  Postprocessing               2.79
  TemporalSuperResolution      1.76
  RenderDeferredLighting       1.09
  Shadow Depths                0.94
  Editor Primitives            0.81   ← 에디터 전용, 게임엔 없음
  Lights                       0.70
  Nanite VisBuffer             0.62
  Shadow Projection            0.62
  Nanite BasePass              0.44
  Basepass                     0.30
  Slate UI                     0.25   ← 에디터 전용
  Translucent Lighting         0.10
  LumenReflections             0.09
  Nanite Streaming             0.08
  Nanite Readback              0.08
  LumenSceneUpdate             0.07
  SkyAtmosphereLUTs            0.05
  VirtualTextureUpdate         0.05
  HZB / Prepass                0.03 / 0.03
  LumenIrradianceFieldGather   0.03
  Translucency / LightGrid     0.02 / 0.02
  SkyAtmosphere                0.01
```

### stat gpu — Compute Queue 0 (Busy Avg, ms)

```
Queue Total                    1.47   (Wait 2.24,  Idle 11.87)
  LumenIrradianceFieldGather   0.58
  TemporalSuperResolution      0.46
  Postprocessing               0.45
  LumenSceneLighting           0.23
  RenderDeferredLighting       0.10
```

### 분해

| 묶음 | ms | 비고 |
|---|---:|---|
| **Nanite 전체** | **1.22** | VisBuffer 0.62 + BasePass 0.44 + Streaming 0.08 + Readback 0.08 |
| 그림자(VSM) | 1.56 | Shadow Depths 0.94 + Shadow Projection 0.62 |
| 해상도 종속 | 4.55 | Postprocessing 2.79 + TSR 1.76 — **나무 수와 무관** |
| 에디터 전용 | 1.06 | Editor Primitives 0.81 + Slate UI 0.25 — 게임 빌드엔 없음 |
| Graphics Queue **Idle** | **7.40** | 프레임의 44%가 노는 중 |

**핵심: 나무 10만 그루의 Nanite 비용이 1.22ms.** 가장 비싼 항목은 Postprocessing + TSR(4.55ms)인데 이건 화면 해상도에 비례하지 나무 개수와 무관하다. 즉 **나무를 더 심어도 이 4.55ms는 안 움직인다.**

Draws 171개도 주목할 만하다. 인스턴싱이 제대로 먹고 있다는 뜻.

### 화질 (육안)

- **LOD 전환 흔적 없음** — 기존 소나무 LOD3→4에서 십자 카드로 바뀌던 문제가 사라짐 (목표 1 달성)
- **잎 반짝임/시머 없음** (목표 4 달성)
- 원거리에서 앙상해지는 느낌 없음 (목표 2 달성으로 보임, 단 §6의 16배 줌 검증은 미실시)

### 스케일링 곡선

| 그루 | 배치 방식 | 범위 | 결과 |
|---:|---|---|---|
| 137 | 개별 SkeletalMeshActor | 300m | 💥 **엔진 어서트 크래시** (§3) |
| 1,000 | ISKMC ×4 | 300m | 60fps 캡 |
| 10,000 | ISKMC ×4 | 950m | 60fps 캡, 화질 이상 없음 |
| **100,000** | **ISKMC ×40** | **3,010m** | **60fps 캡, GPU 7.77ms (Idle 7.40ms)** |

---

## 3. 핵심 발견 1 — 22비트 스키닝 버퍼 천장 (⚠ 인계서에 없던 항목)

### 증상

나무를 **개별 `SkeletalMeshActor`로** 배치하다 약 137그루에서 크래시:

```
Assertion failed: ObjectSpaceBufferOffset == INDEX_NONE
                  || ObjectSpaceBufferOffset <= (1 << 22) - 1
  [File: Engine/Source/Runtime/Renderer/Private/Skinning/SkinningSceneExtension.h] [Line: 173]
```

### 원인

Nanite 스킨드 메시는 CPU에서 미리 스키닝하지 않고 GPU가 래스터 중에 스키닝한다. 그래서 씬의 **모든** 스킨드 메시 본 데이터가 공유 GPU 버퍼에 올라간다 (`SkinningSceneExtension.h:180-190`의 `FBuffers`).

프록시가 등록될 때마다 전역 스팬 할당자에서 구역을 받는다:

```cpp
// SkinningSceneExtension.cpp:1260-1268
const uint32 ObjectSpaceNeededSize = Data.MaxObjectSpaceCount * Data.Proxy->GetObjectSpaceFloatCount();
Data.ObjectSpaceBufferOffset = SceneData->ObjectSpaceAllocator.Allocate(ObjectSpaceNeededSize);
```

- `GetMaxBoneObjectSpaceCount() = BoneHierarchy.Num()` — **본 개수** (`SkinningSceneExtensionProxy.h:70-73`)
- `GetObjectSpaceFloatCount() = 4(quat) + 3(translation) + (HasScale ? 3 : 0)` = **7** (`:121-125`)

그 구역의 **시작 오프셋**이 GPU 셰이더와 C++이 공유하는 구조체의 22비트 필드에 들어간다:

```c
// Engine/Shaders/Shared/SkinningDefinitions.h:158-182
#define SKINNING_BUFFER_HIERARCHY_OFFSET_BITS    22
#define SKINNING_BUFFER_TRANSFORM_OFFSET_BITS    22
#define SKINNING_BUFFER_OBJECT_SPACE_OFFSET_BITS 22

struct FSkinningHeader
{
    UINT_TYPE HierarchyBufferOffset   : 22;
    UINT_TYPE TransformBufferOffset   : 22;
    UINT_TYPE ObjectSpaceBufferOffset : 22;   // ← 터진 것
    UINT_TYPE MaxTransformCount       : 16;
    UINT_TYPE CurrentTransformSlot    : 1;
    UINT_TYPE MaxInfluenceCount       : 5;
    UINT_TYPE bHasScale               : 1;
    UINT_TYPE bIsRefPose              : 1;
    UINT_TYPE Unused                  : 6;
};   // 합 96비트 = uint32 3개
```

**메모리 부족이 아니라 주소를 가리킬 비트가 없는 것이다.** GPU 구조체 레이아웃이라 cvar/ini로 못 늘린다. 천장은 2²²−1 = **4,194,303 floats**.

### 계산 — 크래시 지점과 일치

A→B→C→D 순환 배치였으므로 4그루 1세트 = (2,940 + 8,678 + 4,136 + 1,762) × 7 = **122,612 floats**.

```
4,194,303 ÷ 122,612 = 34.2 세트 = 136.8 그루
```

137번째에서 넘는다. 로그상 `SkeletalMeshActor_140`까지 스폰 후 터졌다 — 어서트는 렌더 스레드가 헤더를 빌드할 때 발생하므로 스폰보다 몇 프레임 뒤처진다. 계산과 맞는다.

### 왜 이렇게 빨리 터지나

**버텍스 5천 개짜리 메시에 본이 1,762~8,678개.** PVE가 바람을 위해 잔가지마다 본을 심어서 생긴, 버텍스보다 본이 많은 기형적 메시다. 이 버퍼 예산은 일반 캐릭터(본 100~200개) 기준이라 나무 한 그루가 그 **30~80배**를 먹는다.

### 해결 — 인스턴스드 경로

`GetMaxBoneObjectSpaceCount()`는 **인스턴스 수를 곱하지 않는다.** 이 버퍼에 들어가는 건 "레퍼런스 포즈의 본 위치"라 에셋당 하나면 되기 때문이다.

| 배치 방식 | 프록시 | object-space 할당 |
|---|---:|---|
| SkeletalMeshActor × 100,000 | 100,000 | 💥 137에서 크래시 |
| **ISKMC × 40 (인스턴스 100,000)** | **40** | 1,226,120 floats = **예산의 29.2%** |

10만 인스턴스가 이 숫자에 **전혀** 기여하지 않는다. 오직 프록시(컴포넌트) 수에만 비례한다.

### 남은 22비트 두 개 — 바람을 붙일 때 재계산 필요

- `TransformBufferOffset` (22비트): `TransformBufferCount = UniqueAnimationCount × MaxTransformCount × 2` (`SkinningSceneExtension.cpp:1247`). ref pose(이번 테스트)에선 작다. **DynamicWind를 붙이면 `UniqueAnimationCount = DYNAMIC_WIND_DIRECTIONALITY_SLICES`가 곱해지므로** (`DynamicWindData.cpp:36`) 본 8,678개짜리 B 나무에서 다시 계산해야 한다.
- `MaxTransformCount`는 16비트(65,535) → B의 8,678본은 안전.
- `HierarchyBufferOffset` (22비트): 미검증.

---

## 4. 핵심 발견 2 — 바람은 ISKMC 전용이다

이번 테스트에선 바람을 쓰지 않았지만, 조사 과정에서 확정된 사실을 남긴다.

`UDynamicWindData::CreateRenderProxy()`가 받는 인자가 `FInstancedSkinningSceneExtensionProxy*`다 (`DynamicWindData.cpp:64`). 즉 **일반 `USkeletalMeshComponent`/`SkeletalMeshActor`로는 DynamicWind가 안 돈다.** 반드시 `UInstancedSkinnedMeshComponent`여야 한다.

바람을 걸려면:
1. `UDynamicWindData` 에셋 생성 — 전용 팩토리는 `ShouldShowInNewMenu()=false`라 메뉴에 안 뜬다. 부모 팩토리를 통해야 한다: 콘텐츠 브라우저 → **Add → Miscellaneous → Transform Provider Data** → 클래스 피커에서 `DynamicWindData` 선택 (`TransformProviderFactory.cpp:90-125`)
2. ISKMC의 `TransformProvider`에 그 에셋 지정
3. 메시에 `UDynamicWindSkeletalData` AssetUserData 필요 — Megaplants는 이미 있음

PCG 경로도 열려 있다: `FSoftSkinnedMeshComponentDescriptor`에 `SkinnedAsset`과 `TransformProvider`가 둘 다 `EditAnywhere`로 있다 (`AnimBank.h:572-578`). 즉 PCG Instanced Skinned Mesh Spawner 노드 하나에서 나무와 바람이 동시에 걸린다.

바람은 GPU 시뮬이 매 프레임 자동으로 돈다 — `ProvideTransforms()`가 렌더 스레드에서 호출되며(`DynamicWindProvider.cpp:330-354`), 기본 파라미터(WindSpeed=15, Amplitude=1, Direction=+X)로 이미 분다. `UpdateWindParameters()`는 값을 바꿀 때만 필요하다.

---

## 5. 재현 방법

PCG 없이, **새 에셋 0개, Megaplants 원본 무수정**으로 재현 가능하다. MCP 툴 기준:

1. 레벨에 빈 `Actor` 하나 생성 (`SceneTools.add_to_scene_from_class`, `/Script/Engine.Actor`)
2. 거기에 `/Script/Engine.InstancedSkinnedMeshComponent`를 붙인다 (`ActorTools.add_component`)
3. `SkinnedAsset` 지정 (`ObjectTools.set_properties`)
4. **별도 호출로** `InstanceData` 주입

### 함정 2개

- **`SkinnedAsset`과 `InstanceData`를 같은 `set_properties` 호출에 넣으면 안 된다.** 메시 설정이 인스턴스 배열을 리빌드해서 트랜스폼이 전부 (0,0,0)으로 날아간다. 반드시 나눠서, 메시를 먼저.
- **배열 세터가 diff 방식이라 "크기 변경 + 원소 변경"을 동시에 거부한다** (`ArrayAdd: elements changed alongside the size change; insertion points are ambiguous`). 기존 인스턴스가 있으면 `InstanceData: []`로 **비운 뒤** 채워야 한다.

### 구조체

```cpp
// InstancedSkinnedMeshComponent.h:27-55
struct FSkinnedMeshInstanceData
{
    FTransform3f Transform;   // JSON: {"rotation":{x,y,z,w}, "translation":{x,y,z}, "scale3D":{x,y,z}}
    uint32 AnimationIndex;    // JSON: "animationIndex"
};
```

### 처리량

컴포넌트 하나에 2,500 인스턴스 주입이 **약 20~30초**. 컴포넌트가 늘수록 씬 갱신 비용이 붙어 느려진다. MCP 호출 타임아웃이 120초라 **한 호출에 3~4개 컴포넌트**가 적당하다. 그 이상이면 백그라운드로 넘어가며 그동안 에디터가 멈춘다.

40개 컴포넌트를 11회로 나눠 채웠다.

---

## 6. ⚠ 아직 검증 안 된 것

**"성능 통과"는 이번 단계의 좁은 질문에 대한 답이다.** 아래는 미검증이며, 메인 이관 전에 하나씩 확인해야 한다.

| 항목 | 상태 | 왜 중요한가 |
|---|---|---|
| **씬캡쳐(RTSP) 부하** | ❌ 미검증 | titan_example은 최대 12스트림 송출. **Nanite는 뷰마다 컬링/래스터를 새로 돈다** — 이번 1.22ms가 캡쳐 수만큼 곱해질 수 있다. 5.6에서 "씬캡쳐가 있으면 Nanite 스킨 메시가 애니메이션 안 됨" 보고도 있음 |
| **VSM 페이지 풀** | ❌ 미검증 | 이번엔 그림자 1.56ms였지만 뷰가 13개(메인+캡쳐12)면 페이지 풀을 공유하며 압박 |
| **바람** | ❌ 미검증 (의도적) | titan_example은 연기/낙엽 파티클, 드론 주행, WPO 나뭇잎이 바람 시스템에 물려 있음. DynamicWind로 갈지 기존 WPO를 유지할지 미결정. §3의 `TransformBufferOffset` 재계산 필요 |
| **WPO 바람 비용** | ❌ 미검증 | 기존 WPO 바람을 Nanite 메시에 붙이면 programmable raster가 되어 fixed-function보다 비싸다. 공식 문서가 Nanite Skinning을 미는 핵심 논거가 이것 |
| **리눅스 SM6 / Vulkan** | ❌ 미검증 | 납품은 리눅스 패키지(LIG PC, NVIDIA 595.84). Nanite Foliage 리눅스 사례 보고 0건 |
| **패키징** | ❌ 미검증 | UE-386321(5.8·5.8.1 패키지 시작 시 크래시)은 5.8.2에서 수정 예정이었고 **이 PC는 이미 5.8.2**라 버전 요건은 충족. 실제 패키징 테스트는 안 함 |
| **16배 줌 원경** | ❌ 미검증 | RCWS 카메라 조건. 좁은 FOV에서 Voxel 노이즈/밀도 확인 필요 |
| **소나무(Baltic Pine)** | ❌ 없음 | 현재 Silver Birch 1종뿐. Fab에서 5.8 지원 버전 확인 후 추가 필요 |
| **실제 씬 합산** | ❌ 미검증 | 이번은 평면 + 라이트 3개뿐인 빈 레벨. 병사/차량/Niagara/UI가 올라간 상태의 여유는 별개 |
| **해상도** | ⚠ 주의 | RenderRes 1449×740(75.5%). 1920×1080 네이티브나 그 이상에선 해상도 종속 비용(4.55ms)이 커진다 |
| **60fps 상한의 정체** | ⚠ 미해결 | `t.MaxFPS`는 이미 0. vsync 또는 패널 60Hz 추정. GPU Idle 7.40ms가 여유의 증거지만, 언캡 측정을 하면 더 정확한 마진을 얻을 수 있다 (`r.VSync 0` 시도 권장) |

---

## 7. 메인 이관 판단에 미치는 영향 (인계서 §8 갱신)

| 항목 | 인계서 평가 | 이번 결과 반영 |
|---|---|---|
| 성능 (나무 개수) | 미지수 | 🟢 **해결.** 10만 그루 GPU 1.22ms. 4만 그루는 여유 |
| LOD 튐 | 🔴 목표 | 🟢 **해결 확인** |
| 잎 반짝임 | 🔴 목표 | 🟢 **해결 확인** |
| 원거리 밀도 | 🔴 목표 | 🟡 육안 양호, 16배 줌 미검증 |
| PCG 그래프 교체 | 🟠 | 🟠 유지. 단 **ISKMC는 선택이 아니라 필수**임이 확정됨(§3) — StaticMeshSpawner로는 스켈레탈 나무를 못 뿌린다 |
| 엔진 버전 5.8.2+ | 🔴 | 🟢 이 PC 이미 5.8.2 |
| **(신규) 22비트 스키닝 천장** | — | 🟠 **신규 항목.** 컴포넌트(프록시) 수 × 본 수 × 7 ≤ 4,194,303. 현 구성 29.2%. 바람 추가 시 `TransformBufferOffset` 재계산 필요 |
| 씬캡쳐 12스트림 | 🟠 | 🔴 **위험도 상향.** Nanite가 뷰마다 컬링/래스터를 반복하므로 이번 결과가 그대로 적용되지 않음. **다음 최우선 검증 대상** |
| 리눅스 | 🔴 | 🔴 유지 |
| 바람 시스템 | 🟠 | 🟠 유지. §4 참조 |

---

## 8. 다음 단계 (우선순위)

1. **씬캡쳐 부하 측정** — 가장 큰 미지수. `SceneCaptureComponent2D`를 1 → 4 → 12개로 늘려가며 GPU ms. titan_example 조건(1920×1080 ×1, 1280×720, 640×360, 320×180 ×4, 수동 `CaptureScene()`, `bAlwaysPersistRenderingState=true`) 재현
2. **`r.VSync 0`으로 언캡 측정** — 10만 그루의 실제 마진 확정
3. **16배 줌 원경 화질** — 좁은 FOV, 3km 지점 Voxel 노이즈
4. **A/B 비교** — 기존 `SM_BHF_BirchTreeA`에 Nanite만 켠 버전을 같은 레벨에 나란히
5. **바람 방향 결정** — DynamicWind vs 기존 WPO. 결정 후 §3의 `TransformBufferOffset` 재계산
6. **소나무 확보** — Megaplants Baltic Pine (Fab, 5.8 지원 확인)
7. 리눅스 SM6 패키징

---

## 부록 — 현재 테스트 레벨 상태

`/Game/SoldierLab/Treetest`:

- `Actor_0` — ISKMC 40개 (`ISKM_BirchA`~`D`, `ISKM_Birch{A~D}_1`~`_9`), 각 2,500 인스턴스
- `StaticMeshActor_0` — 5km×5km 평면
- `DirectionalLight_0`, `SkyAtmosphere_0`, `SkyLight_0`

`TransformProvider`는 전부 `None`(ref pose, 바람 없음). 인스턴스 배열 10만 개가 레벨에 직렬화되면 파일이 커지므로, 파라미터를 바꿔 다시 뿌릴 거면 저장하지 않는 편이 낫다.
