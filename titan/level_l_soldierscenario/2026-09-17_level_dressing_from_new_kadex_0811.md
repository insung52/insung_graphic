# L_SoldierScenario 레벨 드레싱 (New_kadex_0811 환경 복제)

2026-09-17 / 완료 / 200m×200m 평면 전투 시뮬 레벨에 New_kadex_0811 과 동일한 라이팅·PPV·안개·HDRI 를 이식하고 나무·돌을 MCP 로 배치. PCG/Landscape 미사용.

레벨: `/Game/SoldierLab/Levels/L_SoldierScenario` (titan_example). 작업은 전부 unreal-mcp 로 했고
**레벨과 새 MI 는 저장하지 않은 상태로 넘김**(사용자 확인 후 Save All).

---

## 1. 지면

- `StaticMeshActor_0` → 라벨 `Ground_Plane`. `/Engine/BasicShapes/Plane` 스케일 200 = ±100m, z=0.
- 머티리얼: 신규 `/Game/SoldierLab/Materials/MI_Ground_MossyGrass`
  - 부모 `MI_ugsnfawlw`(Forest_Path, 마스터 `M_MS_Srf`) — 텍스처만 Mossy_Grass(`T_vd3mebls_2K_B/N/ORM`)로 교체, `Tiling`=60(≈3.3m 타일), `PhysMaterial`=`PM_Ground`.
  - New_kadex_0811 랜드스케이프(`MI_GlacierValley`)가 쓰는 Megascans 표면 4종(Mossy_Grass / Forest_Path / Stony_Soil / Rock_Cliff) 중 Mossy_Grass 를 골랐다.

### ⚠ 함정: `MI_vd3mebls`(Mossy_Grass 원본 MI)는 이 프로젝트에서 **렌더가 안 된다**
부모가 `M_MS_Srf_Trm`(transmission 마스터)인데, 붙이면 메시가 투명해진다(테스트 큐브에서도 동일, 셰이더 컴파일 완료 후에도 동일). 그래서 정상 동작하는 `M_MS_Srf` 계열 MI 를 부모로 잡고 텍스처만 갈아끼웠다. `_Trm` 마스터가 왜 깨지는지는 미조사.

## 2. 라이팅/환경 — New_kadex_0811 값 그대로

New_kadex_0811 은 열지 않고 `get_properties` 에 `/Game/New_kadex_0811.New_kadex_0811:PersistentLevel.<Actor>` refPath 를 직접 넣어 읽었다(패키지 로드만 되고 에디터 레벨 전환은 안 일어남 — 다른 레벨 값 참조할 때 유용).

| 액터 | 원본(New_kadex_0811) | 이 레벨 | 값 |
|---|---|---|---|
| DirectionalLight | `DirectionalLight_1` | `Sun_DirectionalLight`(기존 `DirectionalLight_0` 재사용) | Movable, 2500 lux, 5280K(bUseTemperature), 회전 P-58.10/Y20.64/R78.37, SourceAngle 0.20884, CSM 거리 19520/4단, DF 그림자 51200 + RayTracedDFShadows, AtmosphereSunLight, CastVolumetricShadow |
| HDRI 백드롭 | `HDRIBackdrop_C_0` | `HDRIBackdrop_C_0` | `/Game/hdri/kadex_hdr`, Intensity 100, Size 5000m, LightingDistanceFactor 0.5, ProjectionCenter (0,0,170), yaw -99.98 |
| ↳ 내장 Skylight | | | SpecifiedCubemap=`kadex_hdr`, Intensity **1500**, LightColor (0.867, 0.914, 1.0), Res 128 |
| ExponentialHeightFog | `ExponentialHeightFog_0` | `ExponentialHeightFog_0` | Density 0.02, HeightFalloff 0.18, VolumetricFog ON, ScatteringDistribution 0.8123, VolumetricFogDistance 6000, Inscattering 검정 |
| PostProcessVolume | `PostProcessVolume_0` | `PostProcessVolume_0` | Unbound. 오버라이드만 나열: ColorGammaMidtones (0.9783, 0.9901, 1), ColorGainMidtones (0.9824, 0.9951, 1), ColorGammaHighlights (1,1,1), SceneFringe 0.1, ChromaticAberrationStartOffset 0, AutoExposure Histogram, Min/MaxBrightness **10/10**(고정 노출), ExposureBias 1, LocalExposure ShadowContrast 0.9 / DetailStrength 1.3 / MiddleGreyBias 0, Vignette 0.4 |

높이 결정:
- 원본은 지면(아군 발 높이 ≈ z -970) 기준으로 안개 액터가 26m 아래, HDRI 백드롭이 117m 아래에 있었다.
- 이 레벨은 지면 z=0 이므로 안개는 z=-2600(같은 26m 오프셋 → 지면 안개 밀도 동일), HDRI 백드롭은 **z=-20** 으로 뒀다. 원본처럼 117m 아래에 두면 평면 가장자리 밖으로 돔 바닥이 절벽처럼 보인다. -20 이면 HDRI 바닥 투영이 평면 바깥으로 이어져 지평선이 자연스럽다.
- HDRIBackdrop 은 BP 컨스트럭션이 MCP 스폰 시점엔 안 돌아서 Skylight 가 기본값(SLS_CapturedScene, 1.0)으로 남는다 → `HDRIBackdrop_C_0.Skylight` 에 직접 썼다.

## 3. 배치 (아웃라이너 폴더 `Environment/*`, `Lighting`)

시드 고정 LCG(`20260917`, `777`)로 배치했으므로 스크립트를 다시 돌리면 같은 결과.

| 폴더 | 수 | 내용 |
|---|---|---|
| `Environment/Trees` | 159 | 외곽 벨트 120그루(|x| 또는 |y| ∈ [70m, 97m], 최소간격 4.8m) + 내부 군집 8곳 39그루. 에셋: `SM_Scots_Pine_Forest_02`/`Border_01`/`Border_02`, `spruce_full_01`, `SM_BHF_BirchTreeA`/`MediumA`. 스케일 0.85~1.2, 야 랜덤 |
| `Environment/Rocks` | 83 | 중앙 개활지 엄폐용 바위 군집 9곳(`SM_BHF_RockA1~D2`, `SM_IcelandicMossyRock`, 스케일 1.6~2.6) + 큰 바위 14 + 작은 돌 40(`SM_Pine_Rock_Small_01/03`, `SM_MossyRocksA/B`) |
| `Environment/Debris` | 12 | 벨트 안쪽 가장자리에 `SM_FallenPineTree`, `SM_BrokenTreeStump`, `SM_ForestRootsA` |

중앙 ±55m 는 나무 없이 바위 엄폐물만 있는 개활지. 나무·돌 전부 z=0 에 놓았고(메시 피벗이 밑동/반쯤 묻힘) 스냅은 안 썼다.

## 4. 내비메시 (아웃라이너 폴더 `Navigation`)

- `NavMeshBoundsVolume_0`: 큐브 브러시 스케일 (100, 100, 17.5) @ z=1250 → 월드 AABB **±10000 × ±10000 × z[-500, 3000]** (평면 전체 + 바위 높이).
- `DefaultEngine.ini` 의 `bAutoCreateNavigationData=True` + `SupportedAgents` 2종 덕에 볼륨을 놓자마자
  `RecastNavMesh-Tank`(반경 300) / `RecastNavMesh-Default`(반경 35) 가 자동 생성됨. 두 액터 설정은 New_kadex_0811 의
  것과 동일(TileSizeUU 1000, cell 38/19/19, Static 생성) — 어차피 전부 config 에서 오는 값이라 레벨에서 손댈 것 없음.
- ⚠ MCP 로는 "구워졌는지" 확인 수단이 없다(액터 bounds 는 항상 0). 뷰포트에서 **P** 키로 초록 내비메시 확인,
  안 보이면 Build ▸ Build Paths.

## 5. 남은 것 / 주의

- **저장 안 함**. Save All 시 `L_SoldierScenario` + `MI_Ground_MossyGrass` 두 개가 저장 대상(신규 MI 는 P4 mark for add 필요).
- 나무는 New_kadex_0811 PCG 와 같은 에셋이라 WPO(바람) 머티리얼 그대로 — 159그루면 문제 없지만 더 늘릴 땐 [[project_new_kadex_forest_perf]] 참고.
- 지면 타일 반복이 눈에 띄면 `MI_Ground_MossyGrass.Tiling` 을 40~60 사이에서 조정.
