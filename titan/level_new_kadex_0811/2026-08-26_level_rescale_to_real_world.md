# New_kadex_0811 레벨 현실 1:1 재스케일 + real2world 재보정

2026-08-26 작업 (문서화 2026-09-22) / 완료 / 현실보다 26.2% 크게 지어진 레벨 전체를 월드 원점 기준 k=0.7921933250 으로 균일 축소해 진짜 1:1 로 만들고, `GeoCoordinateUtils.h` 보정점을 랜드스케이프 코너 2점으로 교체. 큐브 독립 검증 오차 0.001 m.

> 📌 왜 한 달 뒤에 문서화하는가 — 08-26 작업 당시 devlog 를 남기지 않아 다른 세션이 이 재스케일과 `GetDistanceScaleFactor()` 값 변화(1.2135 → 1.0006)를 모른 채 작업했다. 이 문서는 그 공백을 메우는 사후 기록이다. 수치·절차는 전부 당시 세션 로그 그대로.

관련: `guide/real2world.md`(현재 동작 레퍼런스, 이 작업 반영판) · `protocol/2026-09-22_minimap_georeference_for_lig.md`(같은 수치를 LIG 지도 공유용으로 정리) · 내비메시 `vehicle/ugv/2026-08-27_new_kadex_0811_navmesh_autonomous_driving.md`.

---

## 1. 배경 — 레벨이 현실보다 26.2% 컸다

레벨 제작 시 랜드스케이프의 두 대각 꼭지점에 대응하는 실좌표가 정해져 있었다(사용자 제공):

| 언리얼 꼭지점 | 위도 | 경도 |
|---|---|---|
| `-X +Y` | 37.9204090 N | 128.1948530 E |
| `+X -Y` | 37.9038185 N | 128.2188489 E |

랜드스케이프 실측(`Landscape_1`): location `(-124835.307, 124835.304, 0)`, yaw `-90°`, scale `62/62/30`, 바운즈 스팬 `249984.00 cm` → `249984 / 62 = 4032.000` quads(63 컴포넌트 × 64). 완전한 정사각형, 한 변 **2499.84 m**, 대각 **3535.31 m**.

같은 두 점의 현실 거리를 네 방법으로 계산 — 전부 0.057% 안:

| 방법 | 대각 거리 |
|---|---|
| 평면 ENU(도당 미터 급수) | 2800.65 m |
| **Vincenty 측지선(WGS84)** | **2800.65 m** |
| UTM zone 52 평면 | 2799.70 m |
| Haversine 구체 | 2799.05 m |

→ **k = 2800.65 / 3535.31 = 0.7921933250** (현실 m / 언리얼 m). 언리얼이 **26.2% 큼**. 1/k = 1.2623181343.

### 1.1 전제 검증 (결론을 뒤집을 수 있는 것만)

- **대각 vs 인접 꼭지점** — 인접이었다면 k = 2800.65/2499.84 = 1.120 으로 "언리얼이 10.7% 작다"가 되어 정반대. `-X+Y` / `+X-Y` 는 두 축 부호가 모두 뒤집히므로 대각이 확실.
- **어느 대각이든 k 동일** — 정사각형의 두 대각선 길이가 같으므로 코너를 반대로 짚었어도 회전만 바뀌고 스케일은 불변.
- **독립 정황 증거** — 도출된 회전(-86.11°)으로 랜드스케이프 **로컬 축**의 방위를 계산하면 로컬 +X → 86.11°(동), 로컬 +Y → 176.11°(남). 북쪽이 위인 래스터 하이트맵의 전형적 배치(X=열=동, Y=행=남)와 3.89° 차이로 일치. 랜드스케이프 액터의 yaw -90 은 이 계산에 안 들어간 정보라 코너 대응이 맞다는 독립 확인.
- **민감도** — 회전을 정확히 -90° 로 고정하고 스케일만 풀어도 k = 0.790369(26.52%). 자유 회전 대비 0.23% 차이, 결론 불변.

---

## 2. 방법 — 월드 원점 기준 3축 균일 상사변환

모든 액터 location 을 `p → k·p`(원점 기준), 랜드스케이프는 location 과 scale 을 둘 다 ×k, **Z 까지 같이**. 이유:
- 원점 기준이면 순수 스케일이 정확한 역변환을 가지고(§4 PCG 보정에서 이게 결정적), 랜드스케이프 코너 = `k × 옛 코너` 가 정확히 성립.
- Z 도 줄여야 지형 경사가 안 변하고, 지면에 놓인 액터가 그대로 붙어 있음(옛 Z/XY 비 30/62 보존).
- **소품(나무·바위·차량)의 자체 스케일은 건드리지 않음**(사용자 요구) — 위치만 이동.

```
Landscape  loc (-124835.3071, 124835.3040, 0) → (-98893.6970, 98893.6946, 0)
           scale 62/62/30 → 49.1159861/49.1159861/23.7657997
           yaw -90 유지
           한 변 4032 × 49.1159861 = 198035.656 cm = 1980.36 m
자체 검증  loc×k + 4032×(62×k) = 99141.9592 == corner B×k   ✔
```

레벨 구성 실측(338 액터): 모놀리식 190 MB umap(World Partition 아님, 전 액터 로드), **InstancedFoliageActor 없음**, StaticMeshActor 295(나무/바위/마커), 스플라인 액터 14, PCG 숲 10(`BP_SplineForest_tree` ×2 · `_plant` ×8), 볼륨 5, 차량 3, 시스템 10. PCG 인스턴스는 숲 액터 하나에만 39,025개 — 직접 손대지 않고 스플라인만 줄이고 Regenerate.

### 2.1 검증된 절차 (이 순서 그대로)

| 단계 | 대상 | 방법 | 검증 |
|---|---|---|---|
| 1 | `Landscape_1` | `set_actor_transform` loc×k + scale×k + **yaw -90 명시** | 바운즈 스팬 1980.3565615680213 m vs 예측 1980.356561568 — 10자리 일치, 4032 quads 유지 |
| 2 | 나머지 328 액터 | location ×k, **rotation·scale 을 읽어서 그대로 다시 써넣음** | 시스템 액터 10 스킵, 어태치 0 |
| 2b | 볼륨 6 (`BlockingVolume`×3 · `NavMeshBoundsVolume` · `Brush_0` · `PostProcessVolume`) | scale 도 ×k | 브러시는 형상이 로컬 폴리곤이라 위치만 옮기면 영역이 안 줄어듦 — 소품이 아니라 영역이니 예외 |
| 3 | 도로 스플라인 4 (`Actor_7/8/3/4` = `RoadCenterline_*`) | Spline 컴포넌트 `RelativeLocation ×k` + `RelativeScale3D = k` | 첫 점 월드좌표 `k × 옛 월드좌표` 와 오차 0.000000 cm |
| 4 | `BP_SplineForest_*` 10 | **사용자가 Details 패널에서 직접** 액터 scale 1 유지 + Spline 컴포넌트 scale 0.792 | 아래 함정 참고 |

단계 1 을 **단독으로 먼저** 한 덕에 `set_actor_transform` 이 생략한 rotation 을 0 으로 밀어버리는 것(§6)을 랜드스케이프 하나에서 잡았다 — 일괄 스크립트였으면 338개 회전이 날아갈 뻔했음.

**4단계 함정**: MCP `set_properties` 가 **BP 인스턴스 컴포넌트에서는 벡터의 X 성분만 쓴다**(`(0.5,0.5,0.5)` → `(0.5,1,1)`, 8회 반복해도 수렴 안 함). `SplineCurves` 배열 쓰기는 `true` 를 돌려주며 **조용히 무시**. 액터 scale 에 k 를 주면 스플라인은 줄지만 PCG 가 뿌린 ISM 인스턴스까지 0.79배로 그려짐 → 사용자가 손으로 처리.

### 2.2 검증

- **상사변환 정확성** — 액터 쌍 사이 거리비(`CineCamera↔PostProcessVolume` 등 3쌍)가 k 와 **1e-16** 안에서 일치. 상대 기하가 비트 단위로 보존됐으므로 지면에 붙어 있던 것은 여전히 붙어 있음.
- 338 액터 전수 확인, 새 랜드스케이프 범위 밖 0개. 랜드스케이프 잔차 스케일 1.000001.
- **큐브 독립 검증**(사용자 배치) — `Cube`(`-X-Y` 코너 안쪽 53.5 cm) / `Cube2`(`+X+Y` 안쪽 39.4 cm), 즉 **보정에 쓰지 않은 반대쪽 대각선**. 씬 실측 2799.719 m ↔ 두 큐브를 새 보정으로 실좌표 변환 후 Vincenty 2799.719 m — **차이 0.001 m**. 정확한 코너 간 2800.647 m 에서 두 inset 을 뺀 예측 2799.7186 m 과 실측 2799.7186 m, 잔차 0.0 mm.

작업은 원본 `New_kadex_0811` 에 직접 적용(첫 시도는 복제본에서 하려고 저장 없이 리로드로 폐기 — 디스크 umap 무손상 확인 후). 복제본 `New_kadex_0811_dup.umap` 은 작업 전 상태의 백업으로 남김(190 MB 에디터 복제가 커밋 64/66 GB 메모리에서 수 분 걸렸음 — 다음엔 파일 복사로).

---

## 3. GeoCoordinateUtils.h 재보정

`GeoCalibrationPoints` 를 5점(옛 `kadex_demo_0716` 의 눈대중 랜드마크, RMS 53 m) → **랜드스케이프 대각 코너 2점**으로 교체. 설계값이라 평균 낼 눈대중 오차가 없고, 이 경우 `ComputeSceneToRealFit()` 의 2점 경로(정확한 복소 나눗셈)가 올바른 추정량이다.

```cpp
{ 37.9204090, 128.1948530, -98893.69699143617,  98893.69458325558 }, // -X / +Y
{ 37.9038185, 128.2188489,  99141.95916536597, -99141.96157354656 }, // +X / -Y
```

코드가 계산하는 값: **Scale 1.000553788 · RotationRadians -1.504984324 (-86.229250°)** · Anchor 37.9121138 N / 128.2068509 E · scene (124.13, -124.13) · 씬 +X = 방위각 176.23°(거의 남쪽).

**Scale 이 1.0 이 아니라 1.00055 인 이유**: `MetersPerLatitudeDegree = 111320` 상수가 이 위도 실제값 110994.8 보다 커서 N-S 를 0.293% 과대, cos 유도 경도 계수는 E-W 를 0.126% 과소평가 → 균일 스케일 하나가 그 사이에 놓인다. **상수를 고치지 말 것** — 양방향에서 상쇄돼 lat/long 정확도에 영향 없고, `GetMapSceneWidthCm()`·미니맵 축척바까지 파급된다. 헤더 주석에 명시함.

옛 레벨 대비: scale 1.2135(씬이 현실보다 *작았음*) · 회전 -1.23° · +X = 동쪽(91°) → 새 레벨 scale 1.0006 · 회전 -86.23° · +X = 남쪽. 스케일 방향이 뒤집히고 85° 돌아갔으므로 옛 5점은 이 레벨에서 처음부터 무효였다.

---

## 4. PCG 숲 마스크가 바뀐 문제와 해법

재생성 후 `BP_SplineForest_tree` 의 점박이(채움/비움) 분포가 달라졌다. 그래프 구조(`PCG_SplineForest_tree2`/`_plant` 동일):

```
GetSplineData → SplineSampler(OnInterior) → Projection(landscape)
  → Spatial Noise(Perlin2D) → AttributeMathsOp(OneMinus) → DensityFilter
  → ExecuteBlueprint(ScaleByDensity) → StaticMeshSpawner
```

마스크의 정체 = **`Spatial Noise` 를 생 월드 XY 좌표로 평가**(Transform 단위행렬, Iterations 4, Seed -1826055198) + `DensityFilter`(tree: LowerBound 0.38 컬링 / plant: 0~1 무컬링, 대신 ScaleByDensity 크기 변주). 노이즈 필드가 월드 원점 고정이라 좌표를 k배 줄이자 숲 입장에서 무늬가 1/k = 1.26배 확대 + 다른 영역을 읽게 됨.

**해법**: Spatial Noise `Transform.scale = 1/k = 1.2623181342761252`. 샘플좌표 = 1.2623 × (k·P) = P 로 예전과 **완전히 동일한 노이즈 값**을 읽는다. 원점 기준 축소였기 때문에 순수 스케일만으로 정확한 역변환(다른 피벗이었으면 이동값도 필요). 설정 객체는 노드의 `SettingsInterface` 프로퍼티로(`Settings` 아님). 사용자 확인 "완벽함".

**함정 — 그래프 에셋 이름 ≠ 실제 참조 그래프**: `SplineForest/` 에 `PCG_SplineForest_tree`/`tree1`/`tree2`/`plant`/`BP_plant`/`grass` 가 다 있는데 레벨이 실제로 쓰는 건 **`tree2`**(tree ×2)와 **`plant`**(×8) 뿐. 이름만 보고 `tree` 를 먼저 고쳤다가 아무 변화가 없어 헛돌았고(원복함), `<숲 액터>.PCG → GraphInstance → Graph` 를 읽어서야 확인. tree 와 tree2 는 설정이 완전히 같아 값만 봐서는 구분 불가.

복원 안 되는 것: `SamplerParams.bSeedFromLocalPosition=false` 라 점별 시드를 월드 좌표에서 뽑음 → 마스크는 정확히 복원되지만 **개별 나무의 메시 선택·회전은 섞임**(좌표계가 바뀐 이상 원리적 한계).

밀도 정정: 처음엔 "숲이 1.59배 조밀해진다"고 봤으나 **PCG 숲은 해당 없음** — `interiorSampleSpacing` 이 절대값(tree 450 cm, plant 193.46 cm)이라 면적이 줄면 개수만 줄고 m²당 밀도는 그대로. 1.59배(=1/k²)는 **수동 배치 281개 액터** 쪽만 해당(개수 고정, 면적 0.628배).

---

## 5. `GetDistanceScaleFactor()` 1.2135 → 1.0006 파급

15곳에서 사용. 두 갈래 모두 결과적으로 옳지만 **런타임 거동이 실제로 바뀐다**:

- **표시용** (`Monitor1Widget`/`SelfDefenseMonitor1Widget` 축척바 · `UGVRemoteControlSubsystem` 속도 보고 · `UAVPawn` 고도/속도 · `UGVPawn`/`UGVAIController` 속도·주행거리): 표시값 17.6% 감소 — 이제 씬이 1:1 이라 맞는 값.
- **실스펙 → 씬 물리** (`UAVPawn` 상승/순항/수직 가속, `UGVAIController` 제동거리·에스코트 속도, `RCWSFireControlComponent` 포구속도+투사체 중력): **씬 속도가 21% 빨라짐** → UAV 비행시간·UGV 주행시간·코너 감속 거동이 바뀜. **`BeginScenarioEnemyContact` 전체 타이밍 재점검 필요**, `vehicle/ugv/2026-08-22_ugv_corner_braking_dev_guide.md` 의 실측 튜닝값 재조정 가능성. RCWS 는 포구속도·중력 둘 다 1/Scale 이라 비행시간 T=R/v 불변, 이제 진짜 실세계 탄도(850 m/s 급)가 그대로 적용됨.

**전수 감사**: 단위 변환 상수가 있는 23곳 전부 — 9곳 정상 보정, 14곳 미보정은 전부 정당(퍼센트→비율 오탐 10 · `RCWSComponent` `RangeMeters` 의도적 미보정 2(거리계 표시가 트레이스 도달거리와 일치해야 함, 07-24 픽스) · 씬 단위끼리 비교 2(UAV 기체 시각 기울기, 지형 회피 여유거리)). **누락 없음.** 위젯(Monitor1/2, SelfDefenseMonitor1/2, SelfDefenseDashboard, StatusHUDWidget)은 전부 순수 표시라 손댈 곳 없음 — 보정은 상류 생산자(`UAVPawn::UpdateStatusHUDFlightData`, `UGVPawn::UpdateUGVStatusData`, `UGVAIController`, `UGVRemoteControlSubsystem` UTM/속도)에서. `SelfDefenseMonitor1Widget` 은 `Monitor1Widget` 의 충실한 복제본(축척바 공식 동일).

낡은 수치 주석 갱신(전부 `[2026-08-26]` 날짜 주석 병기, **로직 미변경**): `GeoCoordinateUtils.h:188` · `UGVAIController.cpp:1121, 1492` · `UAVPawn.cpp:249, 562, 848` · `UGVPawn.cpp:351` · `RCWSComponent.cpp:346`.

**남긴 것 2건**:
- `RCWSFireControlComponent.h:41~51` 헤더 주석이 아직 "scene is built ~18.5% smaller" — 당시 read-only 라 못 고침. 로직은 정상.
- `RCWSFireControlComponent.h:361` `MaxEffectiveRangeMeters = 2000.f` 는 **선언과 주석에만 있고 어떤 .cpp 도 참조하지 않는 죽은 프로퍼티**(기존 상태, 이번 작업과 무관, 손대지 않음).

---

## 6. 이번에 발견한 MCP 함정 (메모리에도 기록됨)

| 함정 | 증상 | 대처 |
|---|---|---|
| `set_actor_transform` 생략 필드 리셋 | 스키마는 "unset = don't change" 라 하지만 rotation 을 생략하면 **0 으로 밀어버림**(랜드스케이프 yaw -90 → 0) | location/rotation/scale 3개 항상 전부 명시. 회전 걸린 액터 하나로 먼저 단독 시험 |
| `set_properties` BP 인스턴스 컴포넌트 | 벡터의 **X 성분만** 기록, 반복해도 안 됨. 일반 액터 컴포넌트는 정상 | BP 인스턴스는 `set_actor_transform`(액터 레벨) 또는 에디터 손 편집 |
| `SplineCurves` 쓰기 | `true` 반환, 값 불변 | 스플라인 포인트 재작성 불가 — 컴포넌트 scale 로 우회 |
| `get_properties` 없는 프로퍼티 | 프로그래매틱 스크립트가 끝까지 돌고도 **전체 결과 실패**(내부 try/except 로 못 막음) | 존재하는 프로퍼티만 묻기, `get_components` 에 `component_type=SceneComponent` 필터 |
| 그래프 에셋 이름 ≠ 참조 | §4 | `GraphInstance → Graph` 읽기 |
| 다른 MCP 세션 동시 접속 | 로그에 내가 안 부른 Niagara/Material 툴 호출이 찍힘(3.5코어 점유) | 별개 세션이 같은 에디터에 붙어 있을 수 있음을 인지 |

---

## 7. 남은 것 / 미해결

- 랜드스케이프 `-Y` 변이 미니맵 이미지 동쪽 끝 밖으로 나감 — `+X-Y` 코너 340 m, `-X-Y` 코너 205 m 초과 → `LatLongToMapPixel` 이 X > 1023 반환(1092.6 / 1065.1). **2026-09-22 종결: 무시.** 시나리오 액터는 랜드스케이프 중심부에서만 움직이므로 그 띠에 갈 일이 없음(사용자 확인). 재생성·클램프 안 함. 이건 재스케일이 만든 문제가 아니라 랜드스케이프 코너 좌표 자체가 미니맵 박스보다 동쪽이라 처음부터 있던 것 — 재스케일은 코드가 그걸 정확히 계산하게 했을 뿐.
- `TreeCollisionProxy`(ISM 18개 / 약 1,500 인스턴스) — 액터는 ×k 됐지만 ISM 인스턴스 배열은 그대로라 어긋남. **사용자가 재작업**(이후 `TreeCollisionProxyBuilder` 로 재생성됨).
- 네비메시 리빌드. 시나리오 타이밍 재점검(§5).
- 미니맵 코너 상수는 그대로 — 사용자가 준 DMS 와 기존 상수 차이 0.2 m 라 같은 `m_map.png` 로 판단. 이미지를 새로 뽑으면 레터박스 4상수(47/977 등)도 재측정.
