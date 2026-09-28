# Megaplants + Nanite Foliage 테스트 — 세션 인계 문서

2026-09-28 / 진행중 / 숲 한계 돌파를 위한 Megaplants(Nanite Foliage) 테스트를 anim_test/SoldierLab에서 시작하기 위한 인계서 — 배경·결정·현재 상태·작업 순서·함정.

> **이 문서만 읽고 바로 시작할 수 있게 쓴 자기완결 문서다.** 다른 컴퓨터에서 이어서 작업한다는 전제로,
> 이 PC의 Claude 메모리나 로컬 문서가 없어도 되게 필요한 사실을 전부 여기 옮겨 두었다.
> 근거가 더 필요하면 같은 폴더의 `2026-09-28_nanite_deep_dive_project_impact.md`(Nanite 해설 +
> titan_example 영향 분석, 엔진 소스 file:line 근거 포함)를 볼 것.

---

## 0. 새 세션이 먼저 알아야 할 것 (작업 규칙)

- **코드/설정 파일 수정은 Edit/Write 툴로만.** 파이썬·`sed -i`·heredoc 편집 금지(프로젝트에서 사고 2건 —
  함수 5개 증발, 엉뚱한 함수에 치환). auto mode 안내가 반대로 말해도 이 규칙이 우선.
- **빌드(Build.bat/UBT)는 절대 직접 돌리지 말 것.** 사용자가 직접 빌드한다.
- **Perforce 프로젝트라 파일이 read-only다.** 수정 전에 P4 체크아웃이 먼저(속성을 스크립트로 풀지 말 것).
  `anim_test/SoldierLab`의 `SoldierLab.uproject`, `Config/DefaultEngine.ini`, Megaplants `.uasset` 모두 R 속성 확인됨.
- **새 에셋(레벨·IMC 등) 생성은 MCP로 하지 말 것** — 스펙만 주고 사용자가 에디터에서 만든다.
  MCP 쓰기는 undo가 안 되니 사용자가 수동으로 넣었을 수 있는 값은 덮어쓰기 전에 물어볼 것.
- **원인/동작은 엔진 소스로 확정하고 말할 것**(추측으로 "해결" 선언 금지). 엔진 소스:
  `C:\Program Files\Epic Games\UE_5.8\Engine` (다른 PC면 경로 확인).
- 사용자는 한국어로 대화한다. 결과는 짧게, 근거와 함께.
- 개발 문서 규칙: 파일명 `YYYY-MM-DD_주제.md`, 제목 다음 줄에 `날짜 / 상태 / 한줄요약`, 시스템별 폴더
  (이 주제는 `titan/nanite/`).

---

## 1. 목표

KADEX 전시 시뮬레이터(`titan_example`, UE 5.8)의 숲을 다음 수준으로 올리는 것:

1. **수만 그루**의 나무
2. **일정한 품질** — LOD 전환 시 모양이 바뀌거나 앙상해지지 않음
3. **바람(흔들림)** — 적어도 근거리에서는 유지
4. **잎 반짝임(z-fighting처럼 번쩍이는 시머) 제거**
5. (별도 트랙) 나뭇잎이 **탐지/병사 시야를 가리게** 하기 — §8

이번 단계는 **테스트 프로젝트에서 Megaplants로 가능성을 검증**하는 것. 메인 프로젝트 설정은 아직 안 바꾼다.

---

## 2. 배경 — 지금 숲이 왜 한계인가

메인 프로젝트 `C:\working\works\kadex\titan_example`, 레벨 `New_kadex_0811`:

- PCG(`BP_SplineForest_*`, 그래프 `PCG_SplineForest_tree2` / `PCG_SplineForest_plant`)가 뿌린
  **ISM/HISM 약 50개 컴포넌트 / 약 95,000 인스턴스**(2026-09-10 기준, 숲을 더 뿌리면 바뀜).
- 나무: 소나무 `SM_Scots_Pine_Forest_02`, 자작나무 `SM_BHF_BirchTreeA` / `TinnyA`(`MWPaperBirchForest` 팩).
- **성능 이력**: PIE 2.3fps → WPO 거리 30m 제한으로 21fps → 자작나무 `InstanceLODDistanceScale=0.5`로 31fps.
  WPO를 거리 무제한으로 두면 2~3fps(전 인스턴스가 매 프레임 정점 연산). 효과의 무릎은 20~50m.
- **LOD 문제**: 소나무 벤더 LOD4는 십자 카드(`x-tree` 슬롯, `MI_Scots_Pine_Xtree_Sheet_02`)라 LOD3→4에서
  다른 나무가 된다. 언리얼 자동 LOD 재생성은 폴리곤 감산이라 가지·침엽이 "머리숱 빠진 것처럼" 앙상해짐 — 근본 한계.
  나무 개수 때문에 근거리에서도 LOD를 강제로 낮춰야 했음.
- **잎 머티리얼**: `BLEND_Masked` + `TwoSidedFoliage` + WPO 바람(`MPC_Wind`) — 알파 카드 잎. 반짝임의 주원인.
- **메인 프로젝트는 Nanite OFF** (`r.Nanite.ProjectEnabled=False`, Perforce CL 288, 2026-07-22, 다른 팀원이
  "나나이트, 루멘 off, scalability setting 최적화"로 끔. 당시 VRAM 오류/성능 하락 대응 중 여러 설정을 동시에 뒤집었고,
  Nanite 단독 측정 기록은 없음).

---

## 3. 결정된 방향

- **Nanite Foliage (UE 5.7 도입, 5.8 개선, 전부 Experimental)** 로 간다. 구성:
  - **Nanite Assemblies** — 가지/잎 파트를 에셋 안에서 인스턴싱(수백만 폴리 나무를 수십 MB로)
  - **Nanite Voxel** — 멀리서 잎 덩어리를 픽셀 크기 복셀로 그려 **밀도/부피 유지**(LOD 개념 없음)
  - **Nanite Skinning + DynamicWind** — WPO 대신 **본(스켈레톤)으로 바람**. 작은 화면 크기에선 자동 정지
- **에셋은 Quixel Megaplants(Fab, 무료)** 로 교체 검토. 지금 나무에 Nanite만 켜는 건 LOD 튐만 해결하고
  알파 카드 잎·WPO 바람 한계는 그대로라 목표에 못 미침(아래 표).

| | 기존 나무 + Nanite 체크 | Megaplants |
|---|---|---|
| LOD 튐 | 해결 | 해결 |
| 원거리 밀도 | 불확실(알파 카드가 단순화로 뭉개짐) | Voxel로 유지 |
| 잎 반짝임 | 남음(마스크) | 크게 줄 것으로 예상(지오메트리 잎) |
| 바람 거리 제한 | 계속 필요(WPO 정점 비용) | 본 기반이라 해결 |
| 작업량 | 적음 | 에셋 교체 + PCG 그래프 교체 |

- **테스트는 `anim_test/SoldierLab` 프로젝트에서** 한다(사용자가 Megaplants를 여기 추가함). 메인 프로젝트는 건드리지 않는다.

---

## 4. 테스트 프로젝트 현재 상태 (2026-09-28 확인)

경로: `C:\working\works\kadex\anim_test\SoldierLab\SoldierLab.uproject` (UE 5.8, Perforce)
MCP: `anim_test/.mcp.json` → `unreal-mcp` `http://127.0.0.1:8001/mcp`

> 참고: 이 프로젝트는 원래 병사 AI R&D용(SoldierLab)이었고 2026-09-14에 titan_example로 이관 완료됐다.
> 즉 병사 작업엔 더 안 쓰이는 원본이라 **렌더링 실험용으로 설정을 바꿔도 부담이 적다.**

### 이미 되어 있는 것 (`Config/DefaultEngine.ini`)
```
r.Nanite.ProjectEnabled=True
r.Nanite.Foliage=True          ← Nanite Foliage (Experimental) 이미 켜짐
r.Shadow.Virtual.Enable=1      ← Nanite가 켜져 있으니 VSM 실제로 동작
r.DynamicGlobalIlluminationMethod=1  (Lumen GI)
r.ReflectionMethod=1           (Lumen 반사 — 메인 프로젝트는 2=SSR, 비교 시 주의)
r.Lumen.TraceMeshSDFs=1        (메인 프로젝트는 0)
r.AntiAliasingMethod=4         (TSR — Voxel 노이즈 억제에 필요)
DefaultGraphicsRHI_DX12, D3D12 SM5 + SM6
```

### 들어와 있는 Megaplants
`Content/Megaplant_Library/` (817 MB) — **Silver Birch(자작나무) 1종만**. Baltic Pine(소나무)은 아직 없음.
```
Tree_Silver_Birch/
  Tree_Silver_Birch_01/
    PVE_Preset_Silver_Birch_01      ← PVE 프리셋
    PVE_Silver_Birch_01 / _Data     ← PVE 그래프/데이터
    Birch_WindSettings_01           ← DynamicWind 바람 설정
    Tree_Silver_Birch_01_A~D        ← 이미 익스포트된 나무 4종(스켈레탈 메시) + 각 _Skeleton
  Instances/   (87개: Branch_*/Twig_* 파트, SKM_* 스켈레탈 파트 — Assembly 구성요소)
  Materials/   (MI_Silver_Birch_01_Bark / _Bark_Blend / _Bark_Blend_High / _Bark_Young / _Foliage)
  Textures/
```

### ⚠ 첫 확인 사항 — 플러그인
`SoldierLab.uproject`의 Plugins 목록에 **`ProceduralVegetationEditor`와 `DynamicWind`가 없다.**
둘 다 엔진 기본값이 `"EnabledByDefault": false`다. uproject가 read-only라 에디터에서 켰어도 저장이 안 됐을 수 있다.
→ 에디터를 열어 Edit > Plugins에서 두 플러그인 상태 확인. 꺼져 있으면 PVE 프리셋과 WindSettings 에셋이
로드되지 않거나(Unknown class) 바람이 안 돈다. **uproject를 P4 체크아웃한 뒤** 켜고 재시작할 것.
(PVE 플러그인은 의존성으로 DynamicWind·PCG·Dataflow·GeometryScripting을 끌고 온다.)

---

## 5. 엔진 소스로 확인한 사실 (UE 5.8)

| 사실 | 근거 |
|---|---|
| PVE Export 기본값 = **SkeletalMesh**(바람용 본 포함). StaticMesh로도 뽑을 수 있지만 "no wind animation support" | `Plugins/Experimental/ProceduralVegetationEditor/Source/ProceduralVegetation/Public/Params/PVExportParams.h:57-58` |
| Export 기본 `Create Nanite Foliage = true`, `Nanite Shape Preservation = Voxelize` | 같은 파일 `:60-64` |
| 스켈레탈 Export 콜리전 옵션: None / TrunkOnly / AllGenerations(physics asset) | 같은 파일 `:69` |
| PCG에 **"Instanced Skinned Mesh Spawner"** 노드가 있어 스켈레탈 나무를 인스턴스로 대량 배치 가능(`UInstancedSkinnedMeshComponent`) | `Plugins/PCG/Source/PCG/Private/Elements/PCGSkinnedMeshSpawner.cpp:75-77` |
| `r.Nanite.Foliage`는 기본 0, **ECVF_ReadOnly**(시작 시에만 읽힘 → 에디터 재시작 필요) | `Source/Runtime/Engine/Private/Rendering/NaniteResources.cpp:118-122` |
| Voxel/Assembly 지원은 `r.Nanite.Foliage` 또는 `r.Nanite.AllowVoxels`/`AllowAssemblies`로 게이트 | `Source/Runtime/RenderCore/Private/RenderUtils.cpp:1370-1386` |
| **VSM은 Nanite가 켜진 플랫폼에서만 동작** → 메인 프로젝트는 `r.Shadow.Virtual.Enable=1`이어도 실제론 CSM | `RenderUtils.cpp:1498-1505` |
| 리눅스 `bGenerateNaniteFallbackMeshes=False`는 대상 플랫폼이 Nanite를 지원할 때만 폴백을 뺌 → 메인에서 Nanite를 켜는 순간 실효 | `Source/Runtime/Engine/Private/StaticMesh.cpp:209-217` |
| 반투명 슬롯이 하나라도 있는 Nanite 메시는 통째로 폴백으로 그려짐(폴백이 없으면 안 그려짐) | `NaniteResources.cpp:112-116` 외 (deep dive 문서 A-4) |

---

## 6. 작업 순서 (체크리스트)

### 1단계 — 에셋이 제대로 도는지
- [ ] §4의 플러그인 확인/활성화(uproject 체크아웃 → 재시작)
- [ ] `Tree_Silver_Birch_01_A~D`를 레벨에 몇 개 배치 → 메시 에디터에서 Nanite on, Shape Preservation = Voxelize,
      Nanite Assembly 여부 확인
- [ ] 바람이 흔들리는지(DynamicWind + `Birch_WindSettings_01`), 풍향/세기 조절이 어디서 되는지 확인
- [ ] `PVE_Preset_Silver_Birch_01`을 PVE 에디터로 열어 변형 생성/Export가 되는지(5.8 호환 확인 —
      **5.7용 PVE 에셋은 5.8에서 안 열림**)
- [ ] 뷰모드 `Nanite Visualization`(Clusters/Overdraw/Voxels 등)으로 원거리에서 Voxel로 바뀌는지 확인

### 2단계 — 대량 배치 성능
- [ ] 테스트 레벨(사용자가 에디터에서 생성 — MCP로 레벨 만들지 말 것)에 PCG 그래프로
      **Instanced Skinned Mesh Spawner**를 써서 수천 → 1만 → 3만 → 5만+ 그루 단계별 배치
      (비교용으로 메인 프로젝트 숲 규모 ≈ 95k 인스턴스, 이 중 나무 ≈ 4만~6만)
- [ ] 단계별로 측정: fps, `stat gpu`(BasePass / Nanite / ShadowDepths·VSM / Lumen), `stat Nanite`,
      VRAM(`stat RHI` 또는 작업관리자), 바람 on/off 차이
- [ ] 화질 확인: 근거리(잎 반짝임), 중거리(전환 흔적), 원거리 1~2 km(밀도·Voxel 노이즈). 전시 시뮬레이터는
      RCWS 카메라가 **16배 줌**으로 원거리를 보므로 좁은 FOV 원경도 볼 것
- [ ] (가능하면) 같은 레벨에 기존 자작나무(`SM_BHF_BirchTreeA`)에 Nanite만 켠 버전을 나란히 놓고 A/B

### 3단계 — 메인 프로젝트 특유 조건 흉내
- [ ] **SceneCaptureComponent2D에서 바람이 움직이는지** — 메인 프로젝트는 RTSP로 씬캡쳐를 최대 12스트림
      송출한다(1920x1080 1개, 1280x720, 640x360, 320x180 x4 등, 수동 `CaptureScene()`,
      `bAlwaysPersistRenderingState=true`). 5.6에서 "씬캡쳐가 있으면 Nanite 스킨 메시가 애니메이션 안 됨"
      커뮤니티 보고가 있어 **필수 검증**. 캡쳐 추가당 GPU ms도 잴 것(Nanite는 뷰마다 컬링/래스터를 새로 돈다)
- [ ] 캡쳐 영상에서 Voxel 노이즈(캡쳐는 TSR 히스토리 조건이 메인 뷰와 다를 수 있음)
- [ ] 소나무가 필요하면 Megaplants **Baltic Pine**(= Scots pine) 추가(Fab에서 5.8 지원 버전인지 확인)

### 4단계 — 결과 정리
- [ ] 측정 결과를 `titan/nanite/YYYY-MM-DD_megaplants_test_results.md`로 남기기
- [ ] 메인 이관 여부 판단 자료: 성능 곡선, 화질 스크린샷, 캡쳐 동작, 남은 리스크

---

## 7. 알려진 함정 / 리스크

- **패키징 크래시**: PVE/Megaplants Nanite Foliage 메시가 들어간 **패키지가 5.8·5.8.1에서 시작 시 크래시**
  (`FPVLightDetection::BuildLeafMeshGeometry` → `RawIndexBuffer.h:198`, UE-386321, **5.8.2 수정 예정**).
  에디터/PIE 테스트는 가능하지만 **납품 빌드는 5.8.2 이상 필수.**
  (https://forums.unrealengine.com/t/procedural-vegetation-nanite-foliage-meshes-crash-packaged-project/2730886)
- **전부 Experimental**: Nanite Foliage/Assemblies/Voxels, PVE, DynamicWind(v0.1 "Extremely experimental").
  DynamicWind는 **전역 풍향만**, 충돌/근접 물리 반응 없음. Epic: "use caution when shipping".
- **Voxel 노이즈**: TSR 필수. 커뮤니티에 트렁크 구멍(5.7.4), 노이즈 보고.
- **리눅스**: Nanite Foliage 리눅스 사례 보고 0건. 메인 프로젝트 납품은 **리눅스 패키지**(LIG PC, NVIDIA 드라이버
  595.84 고정, Vulkan)라 최종적으로 리눅스 SM6 실측이 필요. Vulkan Nanite는 SM6에서만(`VulkanPC/DataDrivenPlatformInfo.ini`).
- **WPO 머티리얼을 Nanite에 쓸 경우**: `MaxWorldPositionOffsetDisplacement`가 0이면 흔들린 부분이 컬링 바운드를
  벗어나 잘림/그림자 튐. (Megaplants는 본 바람이라 해당 없을 가능성이 큼 — 확인만)
- 이 PC에서 테스트 레벨 fps를 MCP로 잴 땐 콘솔 명령 툴이 없어서 **로그 프레임 카운터로 계산**했었다.

---

## 8. 메인 프로젝트(titan_example)로 옮길 때 영향 요약

상세는 `2026-09-28_nanite_deep_dive_project_impact.md` §B. 핵심만:

| 항목 | 영향 | 위험 |
|---|---|---|
| 전역 설정 | `r.Nanite.ProjectEnabled=True`, `r.Nanite.Foliage=1`, 플러그인 PVE/DynamicWind — **팀 전원과 납품 빌드가 바뀜**, CL 288 작성자와 합의 필요 | 🔴 |
| 그림자 | Nanite를 켜면 **게임 전체 그림자가 CSM → VSM**(메인 뷰 + 캡쳐 전부). `r.Shadow.Virtual.Enable=0`으로 CSM 유지도 가능. `[ShadowQuality@2]`의 VSM 튜닝(MaxPhysicalPages=1024)이 처음으로 유효해짐 | 🔴 |
| 리눅스 패키지 | `bGenerateNaniteFallbackMeshes=False`를 **기본값(True)으로 되돌릴 것** — 안 그러면 SM5 기동/아토믹 미지원/반투명 슬롯 시 메시 증발 | 🔴 |
| 엔진 버전 | 5.8.2+ (UE-386321) | 🔴 |
| PCG 숲 그래프 | `StaticMeshSpawner` → **Instanced Skinned Mesh Spawner**로 교체 | 🟠 |
| `TreeCollisionProxyBuilder` (`Source/titan_example/Tools/`) | PCG 나무 ISM에서 위치를 읽어 줄기 콜리전 원기둥을 까는 에디터 툴 — 스켈레탈 인스턴스(ISKMC)로 바뀌면 **수정 필요 가능성 큼** | 🟠 |
| 그래픽 설정 UI (`UTitanGraphicsSettings`) | `FoliageWpoDisableDistance`는 유효, `FoliageLodDistanceScale`은 Nanite 메시엔 무의미 | 🟠 |
| RTSP 씬캡쳐 | 캡쳐마다 Nanite 컬링/래스터 추가, VSM 페이지 풀 공유, 캡쳐에서 스킨 바람 멈춤 의심 | 🟠 |
| 바람 시스템 | 기존 `AWindSource` / `MPC_Wind`(머티리얼 WPO) → DynamicWind(전역 풍향)와 연동/대체 설계 필요 | 🟠 |
| 탐지·콜리전·내비메시·리플레이 | CPU 라인트레이스/물리 기반이라 무관 | 🟢 |

---

## 9. 별도 트랙 — 나뭇잎이 시야를 가리게 하기 (렌더링과 독립)

렌더 메시(Nanite든 아니든)로는 해결 안 되고 **AI용 데이터를 따로** 둬야 한다.

- 현재: `UTargetDetectionComponent`(차량 탐지)는 `ECC_Visibility` 라인트레이스, SoldierLab 병사 시야
  (`USoldierSightComponent`)는 전용 채널 `Sight`(`ECC_GameTraceChannel5`). 둘 다 **줄기 프록시만 막히고 잎은 통과**.
- 기존 설계안: `titan/level_new_kadex_0811/2026-09-01_foliage_occlusion_ideas.md` — **안 2(해석적 캐노피 감쇠,
  콜리전 없이 수관을 구로 근사해 VisibleFraction을 깎음) 추천**, 안 1(탐지 전용 채널 캐노피 프록시), 안 3(밀도맵), 안 4(스텐실 픽셀 카운트).
- 업계 선례: Arma 3 — 렌더와 별개인 **View Geometry LOD** + 식생 `viewDensity`(부분 투과, 예: 0.2).
  (https://community.bistudio.com/wiki/Arma_3_Vegetation_P3D)
- GPU PCG로 바꾸면 인스턴스가 CPU 쪽에 없을 수 있어 **나무 위치 데이터를 따로 굽는 방식**이 더 필요해질 수 있음(추정).

---

## 10. 참고

- Nanite Foliage 공식: https://dev.epicgames.com/documentation/en-us/unreal-engine/nanite-foliage
- PVE 공식(5.8): https://dev.epicgames.com/documentation/unreal-engine/procedural-vegetation-editor-pve-in-unreal-engine
- Megaplants 소개(80.lv): https://80.lv/articles/create-lush-nanite-foliage-ready-forests-with-this-free-quixel-asset-pack
- Megaplants Baltic Pine(Fab): https://www.fab.com/listings/a2b04e81-5075-479f-a9d2-4940022f330a
- Megaplants Silver Birch(Fab): https://www.fab.com/listings/94262633-61ed-4f4b-993e-dbc24af3e6ba
- Nanite Foliage 실측 사례(나무 77,376그루 62→119fps, UE5-Main): https://80.lv/articles/get-a-glimpse-of-nanite-foliage-with-voxel-representation-in-ue5-7
- 같은 폴더: `2026-09-28_nanite_deep_dive_project_impact.md` (Nanite 입문 해설 + 메인 프로젝트 시스템별 영향 + 단계별 검증 계획)
- 메인 프로젝트 숲 성능 기록: `titan/level_new_kadex_0811/new_kadex_0811_forest_perf.md`
