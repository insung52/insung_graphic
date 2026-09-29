# New_kadex_0811 숲 Nanite Foliage 이관 (Megaplants + 잎 가림 + DynamicWind)

2026-09-29 갱신(최초 2026-09-28) / 진행중 / 메인 프로젝트 Nanite ON, PCG 숲 38,972그루 + 직접 배치 216그루를 Megaplants 스켈레탈 나무로 교체(배치 위치 보존), 잎이 탐지를 가리는 투과율 시스템, DynamicWind 연동, 줄기 프록시 대응 완료. 09-29: 고사리 미끄러짐(H) 해결(Nanite OFF), 나무 "뚝뚝" 튐(I) 원인 확정·수정, 드론 낙하산 미탐지(J) 잎 밀도 0.1로 해결 — 남은 것은 리눅스 Nanite 폴백·HDRI 각도·RTSP 흔들림 확인·P4 머지 등 마무리 항목(맨 아래 "남은 작업").

선행 문서(같은 날, 같은 폴더):
- `2026-09-28_nanite_deep_dive_project_impact.md` — Nanite 원리 + 켰을 때 프로젝트 영향(VSM·리눅스 폴백 등)
- `2026-09-28_megaplants_nanite_foliage_test_handoff.md` — anim_test 테스트 인계서
- `2026-09-28_megaplants_test_results.md` — 10만 그루 성능, **22비트 스키닝 버퍼 천장(개별 액터 ~137그루 크래시)**

이 문서는 그 테스트 결과를 **메인 프로젝트 `titan_example` / 레벨 `New_kadex_0811`에 실제로 적용한 기록**이다.
아래 A~J는 모두 세션 중 에디터·로그·소스로 확인한 사실이다(확인 못 한 것은 해당 줄에 명시).
I·J절과 H절 결론은 2026-09-29 추가분이다.

---

## 0. 요약

| 항목 | 상태 |
|---|---|
| A. `r.Nanite.ProjectEnabled=True` + `r.Nanite.Foliage=True`, PVE/DynamicWind 플러그인 | ✅ 적용 (부수효과: VSM이 처음으로 실제 동작) |
| B. Megaplants 마이그레이션 사고(머티리얼 Parent=None 등) | ✅ 원본 덮어쓰기로 복구 |
| C. PCG 숲 교체 (디자이너 스플라인 배치 그대로) | ✅ 38,972그루 좌표 동일, LOD 문제 해결 |
| D. 직접 배치 나무 216그루 → 마커 BP + PCG | ✅ 마커 이동/복제로 편집 가능, 구 액터 삭제 |
| E. 잎이 탐지를 가림(투과율) — 병사 시야 + 차량 카메라 | ✅ 빌드·등록 확인(39,482 수관). 잎 밀도는 J절에서 0.1로 조정 |
| F. `TreeCollisionProxyBuilder` 스켈레탈 나무 지원 | ✅ 사용자 Rebuild·Build Paths 완료 |
| G. DynamicWind를 `AWindSource`에 연결 | ✅ 사용자 흔들림 확인 (RTSP 캡처 쪽은 미확인) |
| H. 작은 고사리가 통째로 미끄러짐 | ✅ **09-29 해결** — 고사리 2개 메시 Nanite OFF(사용자). 바위 2개는 선택 과제 |
| I. 나무가 한 그루씩 가끔 "뚝뚝" 튐 | ✅ **09-29 원인 확정(플러그인 셰이더 회전축 전환) + 우리 쪽 수정**(`bDynamicWindIgnoreWander`), 사용자 확인 |
| J. 드론이 나무 밑 낙하산을 못 잡음 | ✅ **09-29 해결** — `LeafExtinctionPerMeter` 8행 전부 0.1(사용자) |

---

## A. 전역 설정

### A.1 변경 내용

| 파일 | 변경 |
|---|---|
| `Config/DefaultEngine.ini` | `r.Nanite.ProjectEnabled=False` → **`True`**, **`r.Nanite.Foliage=True`** 추가(주석 포함). 원래 False는 CL 288(2026-07-22)에서 온 값 |
| `titan_example.uproject` | 플러그인 **`ProceduralVegetationEditor`**, **`DynamicWind`** 활성화(사용자가 에디터에서 켬) |
| `Source/titan_example/titan_example.Build.cs` | `"DynamicWind"` 모듈 의존 추가(G절) |

### A.2 부수효과 — VSM이 이제 진짜로 돈다

`r.Shadow.Virtual.Enable=1`은 원래 켜져 있었지만, VSM은 플랫폼 Nanite 지원을 요구한다
(`Engine/Source/Runtime/RenderCore/Private/RenderUtils.cpp:1498-1504`,
`DoesPlatformSupportVirtualShadowMaps` → `return bIsProjectEnable && DoesPlatformSupportNanite(Platform);`).
그래서 지금까지는 **실제로는 CSM**이었고, Nanite를 켠 순간부터 VSM이 처음 동작한다. 그림자 모양·비용이
바뀐 건 이 때문이다(딥다이브 문서 B-2 (5)).

### A.3 성능 기준선

| 조건 | PIE 기본 뷰 fps |
|---|---|
| Nanite OFF + 옛 나무(교체 전) | 약 32~38 (대부분 ~37) |
| Nanite ON + **옛 나무**(교체 전) | 약 15 + VRAM 빨간 경고 (사용자 측정) |
| Nanite ON + Megaplants(교체 후) | 사용자 "잘 돌아감" — **수치 미기록** |

→ 다음 측정 때 교체 후 fps를 같은 뷰에서 기록할 것.

### A.4 아직 안 바꾼 것 — 패키징 전 필수

- **리눅스 `bGenerateNaniteFallbackMeshes=False`** (`DefaultEngine.ini:179`,
  `[/Script/LinuxTargetPlatform.LinuxTargetSettings]`) — **그대로다.** Nanite가 켜진 지금은 실효 설정이라,
  SM5 기동/아토믹 미지원/반투명 슬롯 등에서 메시가 증발할 수 있다 → 패키징 전 기본값(True)으로 되돌릴 것
  (딥다이브 문서 B-2 (1), 인계서 §8).
- 엔진 버전: PVE 메시가 든 패키지는 UE-386321 때문에 **5.8.2 이상 필수**. 이 PC 엔진은
  `Engine/Build/Build.version` 기준 **5.8.2(CL 56702186)** 라 충족 — **빌드/납품 PC도 5.8.2인지 따로 확인할 것.**

### A.5 Perforce 주의

- 다음 파일은 **user2@user2_jiseong 도 열어 둔 상태** → 서브밋 때 머지 주의:
  `DefaultEngine.ini`, `titan_example.uproject`, `WindSource.h/.cpp`, `titan_example.Build.cs`,
  `TargetDetectionComponent.h/.cpp`, `DetectionTypes.h`, `TreeCollisionProxyBuilder.h/.cpp`.
- 새 C++ 파일은 add로 열려 있음.
- **(2026-09-29) 사용자가 CL 519로 서브밋**(이전 바람 변경분 포함). 그 뒤 I절 수정 때문에
  `WindSource.h/.cpp`를 **다시 열었다** — 이 두 파일은 여전히 user2@user2_jiseong도 열어 둔 상태라 머지 주의.
  (CL 519에 위 목록 중 정확히 어떤 파일·에셋이 들어갔는지는 이 문서에서 확인하지 않았다.)
- **`Content/Megaplant_Library/`(301파일)와 `Content/SplineForest/`의 새 에셋은 아직 P4에 없다** → add 필요.

---

## B. 함정 — Megaplants 마이그레이션이 조용히 망가졌다

**증상**: 나무가 전부 기본 회색 머티리얼로 렌더됨.

**원인**: Megaplants(Baltic Pine, Silver Birch, English Oak, Hornbeam — 각 A~D 스켈레탈 메시)를
`anim_test/SoldierLab`에서 에디터 **Migrate**로 가져올 때, titan_example에 **PVE/DynamicWind 플러그인이
꺼져 있었다.** 그 결과:
- 머티리얼 인스턴스 **15개 전부 Parent=None**으로 저장됨
- 메시의 `DynamicWindSkeletalData` AssetUserData가 **떨어져 나감**
- PVE 프리셋/바람 설정 에셋 **14개는 아예 안 옴**(287 vs 301 파일)
- **에러 로그 0줄**

**복구**: 에디터 종료 → titan 쪽 복사본을 스크래치패드에 백업 → `Content/Megaplant_Library`를
anim_test 원본(301파일)으로 덮어씀.

**교훈**: **플러그인에 의존하는 콘텐츠는, 대상 프로젝트에서 그 플러그인을 먼저 켠 뒤에 Migrate한다.**
안 켜고 옮기면 클래스를 못 찾은 참조가 조용히 None으로 저장된다.

---

## C. PCG 숲 교체 — 디자이너가 깐 스플라인 배치 그대로

### C.1 원본 그래프 (건드리지 않음)

`PCG_SplineForest_tree2`:
```
GetSpline → SplineSampler(interior, 간격 450) → Projection(+GetLandscape) → SpatialNoise
  → DensityFilter(0.38) → AttributeMaths(OneMinus) → ExecuteBlueprint(ScaleByDensity)
  → TransformPoints(스케일 0.6~1.0 균일, yaw 0~360, seed 408230234)
  → StaticMeshSpawner(가중 1:1:1 — 소나무 / 자작 A / 자작 Tinny)
```

### C.2 새 그래프 `/Game/SplineForest/PCG_SplineForest_tree2_Nanite` (복사본)

앞단은 원본과 동일하고 **스포너만 교체**:
```
… TransformPoints
  → LoadDataTable(DT_ForestTrees, Output Type = Attribute Set)
  → MatchAndSetAttributes(Use Match Weight = Weight)
  → AttributeMathsOp Multiply($Scale × ScaleMultiplier → $Scale)   ← InA·InB 둘 다 MatchAndSet에서
  → (자동 삽입된 Filter Data - Point)
  → Instanced Skinned Mesh Spawner(Mesh Attribute = Mesh)
```

- **왜 DataTable + MatchAndSet인가**: 스킨드 스포너에는 가중치 셀렉터가 없다 — 속성 기반 셀렉터뿐
  (`Engine/Plugins/PCG/Source/PCG/Public/MeshSelectors/PCGSkinnedMeshSelector.h`). 그래서 가중치 선택을
  MatchAndSet으로 앞단에서 하고, 스포너는 `Mesh` 속성만 읽는다.
- ⚠ 스킨드 스포너의 Mesh Attribute 기본값은 `@Last` — **틀린 값이다. `Mesh`로 바꿔야 한다.**

### C.3 수종 표 — `FForestTreeRow` + `DT_ForestTrees`

C++ 구조체 `Source/titan_example/Environment/ForestTreeRow.h`(`FForestTreeRow`):
`Mesh`(TSoftObjectPtr<USkinnedAsset>) · `Weight` · `ScaleMultiplier` · `Species` ·
`CanopyBottomCm` · `CanopyTopCm` · `CanopyRadiusCm` · `LeafExtinctionPerMeter`.
(수관 필드는 E절 잎 가림용 — PCG와 탐지가 **같은 표 하나**를 본다.)

`/Game/SplineForest/DT_ForestTrees` 8행:

| 행 | 수종 | Weight | ScaleMultiplier | 수관 하단 | 잎 감쇠(/m) | 비고 |
|---|---|---|---|---|---|---|
| Pine_A~D | Baltic Pine | 각 1 | 0.7 | ≈0.45h | ~~0.25~~ → **0.1** | 옛 스코틀랜드 소나무 자리 |
| Birch_A~C | Silver Birch | 각 1.333 | 1.33 | ≈0.35h | ~~0.35~~ → **0.1** | 옛 자작 A 자리 |
| Birch_D | Silver Birch | 4 | 0.45 | | **0.1** | **묘목 역할**(옛 `BirchTreeTinnyA` 대체) |

(잎 감쇠는 2026-09-29 사용자가 **8행 전부 0.1**로 내렸다 — J절. 수관 하단/상단/반경은 그대로.)

가중 합이 소나무 4 : 자작 A~C 4 : 묘목 4 = 원본의 1:1:1과 같다.

수관 값은 메시 바운드에서 뽑았다(높이 cm / 수관 반경 cm, 반경 = 바운드 반폭 × 0.8):

| | A | B | C | D |
|---|---|---|---|---|
| Pine | 2725 / 529 | 2995 / 467 | 1460 / 272 | 1200 / 229 |
| Birch | 1560 / 258 | 1339 / 347 | 1181 / 276 | 789 / 144 |

`ScaleMultiplier`는 **옛 나무와 같은 실제 크기**가 나오게 잡았다. 옛 메시 높이(스케일 1):
`SM_Scots_Pine_Forest_02` 21.1 m, `SM_BHF_BirchTreeA` 18.1 m, `SM_BHF_BirchTreeTinnyA` 3.5 m,
배치 스케일 중앙값 0.70.

### C.4 적용과 검증

`BP_SplineForest_tree_C_1`(38,972그루)·`_C_2`(294그루)의 PCG 컴포넌트 그래프를 새 그래프로 바꾸고 Generate.

- 개수 **38,972 동일**
- 샘플 박스 x[-32000,-27000] y[-41000,-36000] 182그루 — **좌표가 cm 단위까지 동일**
- 수종은 재추첨됨: 소나무 12,959 / 자작 26,013 (1:2 비율 동일)
- **LOD 문제 완전 해결**(사용자). 잎 반짝임은 약간 남았지만 옛 나무보다 훨씬 나음.

### C.5 남은 잎 반짝임 — 조사한 것 (허용 판정)

| 시도 | 결과 |
|---|---|
| VSM 페이지 풀 | 정상(1024 페이지 중 138 요청) |
| `r.Shadow.Virtual.SMRT.RayCountDirectional 0` | 변화 없음 |
| `ShowFlag.Specular 0` | 변화 없음 |
| Lumen 스크린 트레이스 끔 / GI 끔 / TSR 설정 변경 | 개선 없음 |
| TSR 얇은 지오메트리 감지 | `r.Nanite.Foliage`가 켜지면 자동 ON(`Renderer/Private/PostProcess/TemporalSuperResolution.cpp:489-501` `ShouldEnableThinGeometryDetection`) — 이미 켜져 있음 |
| Megaplants 머티리얼 | 이미 `bVoxelNDF=true`. **`bVoxelOpacity=false`는 미시험** 옵션 |

→ 수용 가능한 수준으로 판정, 여기서 멈춤.

### C.6 라이팅 발견 — HDRI 스카이라이트가 100° 돌아가 있다 (권고, 조치 여부 미확인)

`HDRIBackdrop_C_0` 액터는 yaw −100°인데 그 Skylight의 `SourceCubemapAngle`은 **0**이다.
`SLS_SpecifiedCubemap`은 액터 회전을 무시하므로 **하늘 조명이 태양과 약 100° 어긋나 있다.**
권고: `SourceCubemapAngle` ≈ −100(**부호는 눈으로 확인**) + Recapture. **사용자가 적용했는지는 미확인.**

참고: 자작나무 잔가지가 어두운 건 정상이다(실제 은자작 잔가지는 어둡고, 가지에
`MI_Silver_Birch_01_Bark_Young`이 쓰임).

---

## D. 직접 배치 나무 216그루 → 마커 + PCG (편집 가능, 바람 가능)

### D.1 왜 그냥 교체가 안 되나

- 레벨에 손으로 놓은 `StaticMeshActor` 나무 216그루(자작 A/B/C/D/Medium/Small 150,
  `Scots_Pine_Border_01/02` + `Forest_02` 66, BlockAll 콜리전).
- **스켈레탈 나무를 개별 액터로 놓을 수 없다** — 22비트 스키닝 버퍼 천장, 약 137그루에서 엔진 어서트
  (`2026-09-28_megaplants_test_results.md` §3).
- **Foliage 모드도 안 된다** — 엔진 폴리지 타입은 InstancedStaticMesh와 Actor 두 종류뿐, 스킨드 폴리지
  타입이 없다.

### D.2 해결 — 마커 BP를 놓으면 PCG가 그 자리에 나무를 뿌린다

**마커 BP** (`/Game/SplineForest/`):
- `BP_ForestTreeMarker` (Actor) — DefaultSceneRoot · 캡슐 `TrunkCollision`(반경 25, 반높이 300, z 300,
  **BlockAll**, HiddenInGame) · Billboard · 태그 `TreeMarker`
- 자식 `BP_ForestTreeMarker_Birch`(태그 `TreeMarker`,`Birch`) / `BP_ForestTreeMarker_Pine`(`TreeMarker`,`Pine`)

**그래프** `/Game/SplineForest/PCG_PlacedTrees`:
```
Get Actor Data(AllWorldActors, ByTag Birch / Pine, GetSinglePoint, Merge single point data,
               bTrackActorsOnlyWithinBounds = false)
  → Filter Attribute Elements(DT 행: Species == SilverBirch / BalticPine)
  → Match And Set Attributes(균일)
  → Instanced Skinned Mesh Spawner(Mesh 속성)
```
- `ScaleMultiplier`는 **안 쓴다 — 마커 스케일 = 최종 나무 스케일.**
- PCG 볼륨 `PCG_PlacedTrees_Volume_*` (폴더 `Forest`, 태그 **`NoTreeProxy`** — F절)

### D.3 이관

- 옛 위치에 마커 216개 스폰(폴더 `Forest/TreeMarkers`).
  스케일 = 옛 스케일 × 옛 메시 높이 ÷ 수종 평균 새 높이(자작 12.17 m, 소나무 20.95 m).
- 마커를 옮기거나 복제하면 나무가 다시 생성됨(**사용자 확인**). 단 변형(A~D)은 위치 시드라
  **마커를 움직이면 A~D가 바뀔 수 있다.**
- 옛 216그루는 삭제. ⚠ 삭제 전 함정: 숨긴 옛 액터가 콜리전을 계속 가졌다 — MCP로는 프로파일을
  NoCollision으로만 바꿀 수 있고 `collisionEnabled`는 리로드 전까지 그대로 남았다. 그래서 숨기기가 아니라 삭제.
- 레벨 스캔: 옛 나무 메시 0개. **남긴 것**: InstancedFoliageActor 가문비 묘목(spruce_small 23),
  그루터기 81, 쓰러진 소나무 71.

---

## E. 잎이 탐지를 가린다 — 구현·빌드·등록 확인

`level_new_kadex_0811/2026-09-01_foliage_occlusion_ideas.md`의 **안 2(해석적 캐노피 감쇠)**를 구현했다.
잎에 콜리전을 주지 않고, 수관을 타원체로 등록해 시선이 지나는 길이만큼 Beer-Lambert로 감쇠시킨다.

### E.1 계산 — `USoldierFoliageOcclusionSubsystem` (SoldierLab 모듈)

`Source/SoldierLab/AI/SoldierFoliageOcclusion.h/.cpp` (`UTickableWorldSubsystem`)
- 수관 = 세워진 타원체(`FSoldierCanopy`: 중심, 수평 반경, 반높이, cm당 감쇠)
- 평면 2D 격자(CSR, 셀 10 m, cvar `SoldierLab.Foliage.CellSizeCm`)
- `ComputeTransmittance(From, To, Cutoff, OutFractionAtCutoff)` — 선분의 XY 그림자가 지나는 셀을
  Amanatides-Woo DDA로 걷고, `exp(-Σ 감쇠 × 통과 길이)`. 누적 광학 깊이 5(투과율 ≈ 0.7%)에서 적분 중단.
  **트레이스 0개** — 순수 산술. **물리적으로 이미 트인 선분에 대해 "얼마나 잘 보이나"만 답한다.**
- `SourceId`별 등록(`RegisterCanopies`/`UnregisterSource`), BP용 `Get Foliage Transmittance`
- cvar: **`SoldierLab.Foliage.Enabled`**(0이면 항상 1 = 잎 투명, A/B용) ·
  **`SoldierLab.Debug.Foliage`**(시점 60 m 안 수관 그리기)
- 스탯(`stat SoldierLab`): `Foliage Queries`(사이클) · `Foliage: Queries`(카운터) — `AI/SoldierLabLog.h/.cpp`

참고 감(계산 예시, 실측 아님): 소나무 감쇠 0.25/m 수관을 8 m 가로지르면 exp(−2) ≈ 0.14.
(09-29 현재 값 0.1/m이면 같은 8 m가 exp(−0.8) ≈ 0.45 — J절.)

### E.2 등록 — `UForestCanopyRegistrarSubsystem` (titan_example 모듈)

`Source/titan_example/Environment/ForestCanopyRegistrar.h/.cpp`
- `UForestCanopySettings` — **Project Settings > Game > Forest Canopy**: `TreeTable`(기본
  `/Game/SplineForest/DT_ForestTrees`), `bRegisterCanopies`(끄면 수관 0 = A/B용)
- `OnWorldBeginPlay`: 레벨의 모든 `UInstancedSkinnedMeshComponent` → 메시로 표 행 찾기 → 인스턴스마다 수관 하나
  (Bottom/Top/Radius × 인스턴스 스케일). 표에 없는 메시·감쇠 0인 행은 건너뜀.
- 콘솔 **`Titan.Forest.RefreshCanopies`** — 다시 훑기.
- **왜 두 모듈로 나눴나**: 모듈 의존이 titan_example → SoldierLab 한 방향뿐. 계산은 병사 시야(SoldierLab)와
  차량 탐지(titan) 둘 다 쓰니 아래층에, 수종 표(`FForestTreeRow`)와 PCG 숲을 아는 등록은 위층에.

**PIE 로그 확인**:
`[Foliage] Registered 39482 forest crown(s) from DT_ForestTrees (0 skinned instance(s) not in the table)`,
격자 132×73 @ 1000 cm. (39,482 = 38,972 + 294 + 216)

### E.3 병사 시야 — `USoldierSightComponent`

`Source/SoldierLab/AI/SoldierSight.h/.cpp`, 새 프로퍼티(카테고리 `SoldierLab|Sight|Foliage`):

| 프로퍼티 | 기본 | 뜻 |
|---|---|---|
| `FoliageClearTransmittance` | 0.5 | 이 이상이면 예전처럼 즉시 목격 |
| `FoliageNoticeSeconds` | 0.6 | 투과율 1로 계속 봤을 때 알아채는 시간 |
| `FoliageMinTransmittance` | 0.05 | 이 미만은 아예 안 보임 |
| `FoliageGlimpseMemorySeconds` | 1.0 | 이만큼 못 보면 누적 초기화 |

- T ≥ 0.5 → 목격(기존과 동일). 0.05 ≤ T < 0.5 → **어른거림 누적**
  `Noticed += T × 경과 / FoliageNoticeSeconds`(경과는 0.25 s로 상한) → 1이 되면 목격.
- 필드 비우기(`MarkClearAlongRay` — 콘 스윕, 표적 선, 빈 땅 확인)는 **T가 0.5 아래로 떨어지는 지점에서 멈춘다.**
- 시체 확인(LearnDeath)·`ReportClearView`도 T ≥ 0.5 필요.
- **`SweepRays`(엄폐 층의 엣지 모양)는 물리 그대로** — 잎은 은폐지 모서리가 아니다.

### E.4 차량 카메라 탐지 — `UTargetDetectionComponent`

`Source/titan_example/Detection/TargetDetectionComponent.cpp:438-483`, `DetectionTypes.h:85-93`
- `FDetectedTargetPart.Transmittance` 추가. **`bVisible`은 물리 그대로**(조준용 — 총알은 잎을 뚫는다).
- `VisibleFraction` = 샘플별 투과율의 **평균**(잎 없으면 예전 "안 가려진 수 / 전체"와 같다).
- `FScanResult.MaxTransmittance` 추가. **`AnyVisibleSample` 규칙(RCWS)은 0/1 대신 `MaxTransmittance`를 쫓는다**
  (`:188-194`) — 안 그러면 RCWS만 숲을 꿰뚫어 봤다.
- 재획득 기억(`ReacquireGraceSeconds`)도 `MaxTransmittance ≥ LoseConfidenceThreshold`일 때만(`:206-207`).

### E.5 한계

- 성목 수관은 지면 5~6 m 위에서 시작한다(자작 하단 35% h, 소나무 45% h). **지상 병사 눈높이(1.6 m) 시선은
  대부분 수관 밑으로 지나간다.** 지면 높이에서 가리는 건 묘목(Birch_D)과 줄기 프록시뿐.
- 하층 식생(고사리 등)은 **등록 안 됨.**
- 효과가 가장 큰 건 **높은 카메라** — RCWS, 드론, CCTV.
- 사용자 "잘 된다". 감쇠 값은 09-29에 0.1로 조정(J절). **병사 쪽 튜닝(`FoliageNoticeSeconds` 등)은 아직.**

---

## F. `ATreeCollisionProxyBuilder` — 스켈레탈 나무 지원

`Source/titan_example/Tools/TreeCollisionProxyBuilder.h/.cpp` (툴 원본 설명은
`vehicle/ugv/2026-08-26_ugv_obstacle_avoidance.md` 1절)

추가한 것:
- `FTreeProxySpecies.TreeSkinnedMeshes`(스킨드 에셋 목록) — **TrunkCylinder 전용**, 반경/높이는 메시 로컬 값
- 액터 태그 **`SkipSourceActorTag` = `"NoTreeProxy"`** — 이 태그가 붙은 액터의 나무는 소스에서 제외.
  직접 배치 볼륨(`PCG_PlacedTrees_Volume_*`)에 붙어 있다 — 마커가 이미 자기 캡슐을 갖고 있으니까.
- 소스 수집에 `UInstancedSkinnedMeshComponent` 인스턴스 추가(`GetInstanceData` + 컴포넌트 트랜스폼)
- **두 번째 안전장치**: TrunkCylinder 종이 있는데 소스 나무가 **0그루 매칭**이면 Rebuild **중단**(기존 프록시 유지).
  이게 없었으면 스켈레탈 숲에서 Rebuild 한 번에 **줄기 콜리전 전부가 지워졌다**(옛 코드는 `TreeMesh`만 봤다).
- 로그용 `GetSpeciesName`

레벨 인스턴스 Species **6개**:

| 항목 | 반경 / 높이(로컬) | 비고 |
|---|---|---|
| 옛 나무 2종(스태틱) | — | 이제 0그루 매칭(남겨 둠) |
| 바위 `CopySourceMesh` 2종 | — | 그대로 |
| **Baltic Pine A~D** | 22.9 / 1000 | 옛 16 / 700 ÷ 0.7 |
| **Silver Birch A~C** | 16.5 / 647 | 옛 22 / 860 ÷ 1.33 |

Birch_D(묘목)는 프록시 없음(옛 Tinny와 같다).

수종이 재추첨돼 옛 프록시가 줄기 굵기·위치와 안 맞게 됐으므로 **사용자가 Preview → Rebuild Proxies →
Build Paths** 실행 — "잘됨".

---

## G. 바람 — DynamicWind를 `AWindSource`의 5번째 소비처로

Megaplants는 WPO가 아니라 **본 스키닝**으로 흔들린다 — 엔진 `DynamicWind` 플러그인이 GPU에서 본을 돌리고,
입력은 전역 풍향 하나 + 풍속 하나뿐(`UDynamicWindSubsystem`).

- `AWindSource::PushToDynamicWind()` — 기존 푸시 주기(`TargetPushIntervalSeconds` 0.05 s)에 MPC·Niagara와 같이 호출
- 방향: 액터 전방(= 현재 풍향)의 **수평 성분** — **09-29부터 기본은 배회를 뺀 기준 풍향**
  (`bDynamicWindIgnoreWander`, I절)
- `WindSpeed = clamp(|풍속 m/s| × DynamicWindSpeedPerMS(3), 0, 100)` — DynamicWind의 WindSpeed는 m/s가 아니라
  **0~100 척도**이고 셰이더가 /100으로 정규화한다(`Engine/Plugins/Experimental/DynamicWind/Shaders/DynamicWindEval.usf:81-82`, 플러그인 기본 15)
- `WindAmplitude = DynamicWindAmplitude`(1)
- 토글 `bDriveDynamicWind`

**함정 — 플러그인 헤더를 못 쓴다** (첫 빌드 실패):
`Public/DynamicWindSubsystem.h`가 `DynamicWindLog.h`를 include하는데 그 파일은 플러그인
**`Source/DynamicWind/Internal/`**에 있어서 플러그인 밖 모듈에선 C1083. 게다가 `UDynamicWindSubsystem` UCLASS에
`DYNAMICWIND_API`가 없어 `StaticClass()`도 링크 안 됨.
(처음엔 소스 주석에 "Private 폴더"라고 잘못 적었다 — 2026-09-29 `WindSource.cpp` 주석을 `Internal` 폴더로 정정함.)
→ 해결: 클래스를 이름(`/Script/DynamicWind.DynamicWindSubsystem`)으로 찾고 `GetSubsystemBase`,
UFUNCTION `UpdateWindParameters`를 `ProcessEvent`로 호출(파라미터 구조체 헤더 `DynamicWindParameters.h`는 공개).
안전장치: `ParmsSize != sizeof(인자 구조체)`면 경고 후 이 연동을 끈다.

**에셋 배선**: `/Game/SplineForest/DA_ForestDynamicWind`(클래스 `DynamicWindData`, Add → Miscellaneous →
Transform Provider Data로 생성) → 두 그래프의 스킨드 스포너 `TemplateDescriptor.transformProvider`에 지정 →
PCG 액터 3개 모두 재생성. **이 에셋이 없으면 본은 기본 포즈 그대로다.**

**22비트 `TransformBufferOffset` 재계산**: 바람을 붙이면 `UniqueAnimationCount = DYNAMIC_WIND_DIRECTIONALITY_SLICES = 8`
→ 본당 변환 ×16. 최악 변형 자작 B 8,678본. 추정 총합은 4,194,303 한참 아래. `UsesSkeletonBatching() = true`.

사용자: 나무 흔들림 확인. **RTSP 씬 캡처에서도 흔들리는지는 아직 명시적으로 확인 안 함.**

---

## H. ✅ 해결(09-29) — 작은 고사리가 뿌리부터 휘지 않고 통째로 미끄러진다

**증상**: Nanite ON 이후 `SM_Pine_Fern_Broad_Group_01/_02`(와 `SM_Pine_Rock_Small_01/_03`의 풀 부분)가
통째로 옆으로 미끄러진다. 나무 얘기가 아니다.

**단서**: 이 메시들이 `M_VegetationShader` WPO를 쓰면서 **Nanite가 켜진 유일한 메시**다.
같은 머티리얼인 `SM_Pine_Fern_Broad_01`은 Nanite OFF이고 정상.

**가설 (엔진 소스 기반, 인게임 미검증)**:
머티리얼이 흔들림을 `ObjectPositionWS` 기준 높이로 고정한다(`MF_Height_Gradient`, `MF_Vegetation_WPO`,
`MF_Wind_Bending`). `ObjectPositionWS`가 **인스턴스 위치**를 주는 건 `USE_INSTANCING || USE_INSTANCE_CULLING`으로
컴파일될 때뿐이다(`Engine/Shaders/Private/MaterialTemplate.ush:1878-1896`). Nanite 셰이딩은 둘 다 정의하지 않고
(`Engine/Source/Runtime/Engine/Private/Rendering/NaniteResources.cpp:3684-3721`의 define 목록,
`MaterialTemplate.ush:141-143`에서 `USE_INSTANCE_CULLING` 기본 0) → 프리미티브/컴포넌트 원점으로 떨어짐 →
인스턴스 기준 높이가 틀려져 식물 전체가 최대 진폭으로 흔들린다.

**검증 방법**: 콘솔 `r.Nanite 0` → 미끄러짐이 멈추면 가설 확정.

**수정안**:
1. **(권장) 이 4개 메시만 Nanite 끄기** — masked + WPO는 어차피 Nanite 최악 조합
2. 마스터 머티리얼의 기준점(앵커) 계산을 바꾸기

**결과 (2026-09-29)**: 사용자가 수정안 1을 적용 — **`SM_Pine_Fern_Broad_Group_01`·`_Group_02`의 Nanite를 끔**
(에디터에서 Nanite Settings `bEnabled=false` 확인). 사용자 확인: **미끄러짐이 멈췄다.**
- 원인 설명은 위 가설(Nanite 셰이딩에선 `ObjectPositionWS` = 컴포넌트 원점, `MaterialTemplate.ush:1878-1896`)을
  **작업 설명으로 유지**한다. `r.Nanite 0` A/B를 따로 했다는 기록은 없고, "해당 메시 Nanite OFF → 증상 소멸"이 근거다.
- **`SM_Pine_Rock_Small_01`/`_03`은 아직 Nanite ON**이다. 바위 위 작은 풀 슬롯이 같은 `M_VegetationShader`라
  **여전히 미끄러질 수 있다** — 사용자는 그대로 둠. 눈에 띄면 같은 방법(메시 Nanite OFF)으로 처리하면 된다(선택).
- 새 식생 메시를 들여올 때 규칙: **WPO 흔들림 머티리얼(특히 `ObjectPositionWS` 기준 앵커) + 인스턴스 배치 → Nanite 끄기.**

---

## I. ✅ 해결(09-29) — 나무가 한 그루씩 가끔 "뚝뚝" 튄다 (DynamicWind 회전축 전환)

### I.1 증상

Megaplants 나무가 **한 그루씩, 서로 다른 때에**(수 초~수십 초 간격) 가지가 뚝 튀었다.
`WindSource`의 `BaseSpeedMS 0.1`, `GustAmplitudeMS 0`처럼 거의 무풍으로 해도 계속 났다.

### I.2 배제한 것 (근거)

| 후보 | 배제 근거 |
|---|---|
| Nanite 스트리밍 | `stat NaniteStreaming` — 풀 15%만 사용 |
| 스키닝 버퍼 조각 모음(defrag) | 조각 모음은 **모든 헤더를 한꺼번에** 재할당한다(`Engine/Source/Runtime/Renderer/Private/Skinning/SkinningSceneExtension.cpp:764-790`) — 한 그루씩 튀는 모양이 아니다 |
| 나무별 방향성 슬라이스(8개) | 슬라이스 번호는 **인스턴스 yaw로 고정**된다(`Engine/Plugins/Experimental/DynamicWind/Source/DynamicWind/Private/DynamicWindData.cpp:50-61`, 45° 반올림) — 실행 중에 안 바뀜 |
| 다른 곳에서 바람 값을 덮어씀 | 레벨의 바람 액터는 `WindSource_1` 하나. PVE의 `BP_GlobalFoliageActor_UE5`는 레벨에 없음 |
| 시간 값 튐 | 셰이더 `Time` = 월드 `GetTimeSeconds()`(`.../DynamicWind/Private/DynamicWindProvider.cpp:525`) — 연속값 |

### I.3 A/B 테스트

| 조작 | 결과 |
|---|---|
| `r.Skinning.DefaultAnimationMinScreenSize 1` (거의 모든 거리에서 애니 끔) | 사라짐 |
| `r.Skinning.DefaultAnimationMinScreenSize 0` (전 거리 애니) | **모든 거리에서** 튐 |
| `DynamicWind.OverrideSpeed 0` | 사라짐 |
| `DynamicWind.UseSine 1` (플러그인 디버그용 사인 경로) | 사라짐 — 진짜 해결책은 아님 |
| `DynamicWind.Enable` | `ECVF_ReadOnly`(`.../DynamicWind/Private/DynamicWindSubsystem.cpp:24-29`) — 기동 시에만 적용, 런타임 토글 불가(P4 문제 아님) |
| **`AWindSource.DirectionWanderDegrees = 0`** | **사라짐** ← 결정적 단서 |

→ 풍속이 거의 0이어도 **풍향이 돌기만 하면** 난다.

### I.4 원인 — 플러그인 셰이더의 회전축 전환

`Engine/Plugins/Experimental/DynamicWind/Shaders/DynamicWindEval.usf:168-170`:
```hlsl
const float3 WindHorRotVector = abs(dot(AdjustedBoneForward, SectionWindDirection)) > 0.999f ?
    normalize(cross(AdjustedBoneForward, float3(0.0f, 0.0f, 1.0f))) :
    normalize(cross(AdjustedBoneForward, SectionWindDirection));
```
가지마다 휘는 축을 `normalize(cross(가지 방향, 풍향))`으로 구한다. 풍향이 돌다가 **어떤 가지와 거의 평행**해지는
순간(① 외적 크기가 0 근처라 normalize 방향이 급변하고 ② |dot| > 0.999에서 전혀 다른 축 `cross(가지, Up)`으로
갈아탐) 그 가지가 반대쪽으로 뚝 휜다. 나무마다 yaw가 달라 **평행이 되는 시점이 나무마다 다르므로** "한 그루씩
다른 때에" 보인다. 풍향이 고정이면 평행을 "지나가는" 일이 없어 안 튄다.

### I.5 수정 — 우리 쪽에서 나무 풍향만 고정 (플러그인 무수정)

`AWindSource`에 **`bDynamicWindIgnoreWander`**(기본 **true**, 카테고리 `Wind|DynamicWind`) 추가
(`Source/titan_example/Environment/WindSource.h:154-165`, `WindSource.cpp:132-141`):
- 켜면 DynamicWind에는 **`BaseWindDirectionDegrees`만**(배회 없음) 넘긴다.
- MPC(풀)·Niagara·드론은 계속 **배회하는 풍향**을 받는다.
- 풍속·돌풍은 그대로 나무에 넘어가므로 나무는 계속 세기 변화에 맞춰 흔들린다.
- ⚠ **실행 중에 `BaseWindDirectionDegrees`를 바꾸면** 그 순간 일부 가지가 튄다 — 레벨별로 정해 두는 값.

**레벨 `WindSource_1` 현재 값**: `DirectionWanderDegrees` 50 · `bDynamicWindIgnoreWander` true ·
`BaseWindDirectionDegrees` 0 · `BaseSpeedMS` 1 · `GustAmplitudeMS` 0 · `DynamicWindSpeedPerMS` 3.

사용자 확인: **나무가 더 이상 튀지 않는다.** (빌드는 사용자가 함.)

### I.6 근본 수정이 필요해지면

DynamicWind 플러그인을 프로젝트 `Plugins/`로 포크해서 위 줄을 **정규화하지 않은 외적**(평행 근처에서 크기가
0으로 부드럽게 줄어듦)으로 바꾸면 풍향을 돌려도 된다. **하지 않았다** — 플러그인이 Experimental v0.1
(`DynamicWind.uplugin` `VersionName "0.1"`, `IsExperimentalVersion true`)이라 엔진 업데이트마다 포크 유지 비용이 든다.

---

## J. ✅ 해결(09-29) — 드론이 나무 밑 낙하산을 못 잡는다 (잎 밀도 과다)

**증상**: 드론 카메라가 나무 아래로 내려온 낙하산을 탐지하지 못함.

**원인**: 잎 감쇠가 너무 컸다. 드론은 `VisibleFraction` 규칙이라 신뢰도가 샘플 투과율 평균을 쫓고,
획득 문턱 `AcquireConfidenceThreshold` = 0.6, 유지 `LoseConfidenceThreshold` = 0.5
(`Source/titan_example/Detection/TargetDetectionComponent.h:89,96`). T = exp(−감쇠 × 통과 길이):

| 수관 한 개를 중심으로 관통 | 예전 감쇠 | 예전 T | 0.1/m일 때 T |
|---|---|---|---|
| 자작 A(직경 ≈3.8 m) | 0.35 | ≈0.26 | **≈0.68** |
| 소나무 A | 0.25 | ≈0.36 | **≈0.66** |

→ 예전 값에선 **수관 하나가 탐지를 완전히 막았다**(0.6에 영영 못 닿음). 0.1이면 수관 한 겹 너머는 잡히고,
두 겹이면(≈0.68² ≈ 0.46) 못 잡는다.

**조치(사용자)**: `DT_ForestTrees`의 `LeafExtinctionPerMeter`를 **8행 전부 0.1**. 수관 반경/하단/상단은 그대로.

**남은 한계**: 탐지기별·대상별 잎 배율이 **없다** — 감쇠는 표 하나라 병사 시야·RCWS·드론이 같은 값을 본다.
필요하면 C++ 선택 과제: `UTargetDetectionComponent`에 `FoliageOcclusionScale`(탐지기별 배율),
`UDetectableTargetComponent`에 `bIgnoreFoliage`(대상별 무시). **미구현.**
조절 항목 전체 설명은 `guide/detection_dev_guide.md` 3.4a.

---

## 디자이너용 — 이제 숲을 어떻게 편집하나

| 하고 싶은 것 | 방법 |
|---|---|
| 숲 경계/범위 바꾸기 | **예전과 같다** — `BP_SplineForest_tree_C_*` 스플라인 점 이동 → PCG 컴포넌트 Generate |
| 나무 한 그루 추가 | 콘텐츠 브라우저에서 `BP_ForestTreeMarker_Birch` 또는 `_Pine`을 레벨에 드래그 |
| 한 그루 옮기기/복제/삭제 | 마커를 옮기기 / Alt 드래그 복제 / 삭제 → 나무가 따라 재생성 |
| 크기 | **마커 스케일 = 나무 스케일** (ScaleMultiplier 안 곱함) |
| 반영이 안 될 때 | `PCG_PlacedTrees_Volume_*`의 PCG 컴포넌트 Generate |
| 수종 비율/크기 | `DT_ForestTrees`의 Weight / ScaleMultiplier → 숲 PCG Generate |

⚠ 마커를 옮기면 그 자리 시드로 A~D 변형이 바뀔 수 있다.

**나무를 크게 바꾼 뒤에는 줄기 프록시를 다시 깐다**: `TreeCollisionProxyBuilder` 액터 선택 →
**Preview → Rebuild Proxies → Build Paths**. (직접 배치 마커는 자체 캡슐이 있어 프록시 대상이 아니다.)
잎 가림은 PIE 시작 때 자동 재등록(PIE 중엔 `Titan.Forest.RefreshCanopies`).

---

## 파일 / 에셋 목록

**C++ (신규)**
- `Source/titan_example/Environment/ForestTreeRow.h`
- `Source/titan_example/Environment/ForestCanopyRegistrar.h/.cpp`
- `Source/SoldierLab/AI/SoldierFoliageOcclusion.h/.cpp`

**C++ (수정)**
- `Source/SoldierLab/AI/SoldierSight.h/.cpp`, `Source/SoldierLab/AI/SoldierLabLog.h/.cpp`
- `Source/titan_example/Detection/TargetDetectionComponent.h/.cpp`, `DetectionTypes.h`
- `Source/titan_example/Environment/WindSource.h/.cpp`
- `Source/titan_example/Tools/TreeCollisionProxyBuilder.h/.cpp`
- `Source/titan_example/titan_example.Build.cs`

**설정**: `Config/DefaultEngine.ini`, `titan_example.uproject`

**에셋 (`/Game/SplineForest/`)**: `PCG_SplineForest_tree2_Nanite`, `PCG_PlacedTrees`, `DT_ForestTrees`,
`DA_ForestDynamicWind`, `BP_ForestTreeMarker`, `BP_ForestTreeMarker_Birch`, `BP_ForestTreeMarker_Pine`
**에셋 (`/Game/Megaplant_Library/`)**: Baltic Pine / Silver Birch / English Oak / Hornbeam (301파일)
**레벨 `New_kadex_0811`**: 두 숲 BP의 그래프 교체, 마커 216 + `PCG_PlacedTrees_Volume_*`, 옛 나무 216 삭제,
`TreeCollisionProxyBuilder` Species 6개 + 프록시 재생성, (09-29) `WindSource_1` 값(I.5)
**(09-29) 에셋 수정**: `SM_Pine_Fern_Broad_Group_01/_02` Nanite OFF, `DT_ForestTrees` 잎 감쇠 0.1

---

## 남은 작업

(2026-09-29 갱신 — 완료: ~~H 고사리 미끄러짐~~(고사리 2개 Nanite OFF), I 나무 튐 수정, J 드론 낙하산 탐지)

1. **리눅스 패키징** — `bGenerateNaniteFallbackMeshes`를 기본값(True)으로 되돌린 뒤 패키징 검증 (A.4) ← **패키징 전 필수**
2. **HDRI 스카이라이트 각도** — `SourceCubemapAngle` ≈ −100(부호 확인) + Recapture (C.6, 적용 여부 미확인)
3. **RTSP 씬 캡처에서 나무 흔들림 확인** (G)
4. **P4** — `WindSource.h/.cpp` 재오픈분(I절) 서브밋 시 user2와 머지. CL 519 이후 남은 파일·새 에셋(`Megaplant_Library`·`SplineForest`) add 여부 확인 (A.5)
5. **엔진 5.8.2+** — 이 PC는 5.8.2 확인. 빌드/납품 PC 확인 (UE-386321)
6. (선택) **바위 `SM_Pine_Rock_Small_01/_03` Nanite OFF** — 위 풀이 미끄러지면 (H)
7. (선택) **탐지기별/대상별 잎 배율** — `FoliageOcclusionScale`/`bIgnoreFoliage` C++ (J)
8. **잎 가림에 하층 식생(고사리) 등록** — 지상 병사 눈높이 은폐는 지금 묘목·줄기뿐 (E.5)
9. **병사 쪽 잎 튜닝** — `FoliageNoticeSeconds`·`FoliageClearTransmittance` 인게임 조정 (감쇠 값은 J절에서 0.1로 정함)
10. 교체 후 fps 수치 기록 (A.3)
11. (선택) 잎 반짝임 — `bVoxelOpacity` 시험 (C.5)
12. (선택) 풍향 배회를 나무에도 주고 싶으면 DynamicWind 플러그인 포크 (I.6)
