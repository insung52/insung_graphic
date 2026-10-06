# 숲 바람 재구성 — 머티리얼 Time 되감기(MPC_TitanTime) + 스태틱 나무 WPO 바람 시도

2026-10-01 / 진행중 / 풀·식생 머티리얼의 float Time 정밀도 떨림을 `UTitanMaterialTimeSubsystem`+`MPC_TitanTime`으로 전역 해결(사용자 검증 완료). 스태틱 나무에 WPO 바람(`MF_TitanTreeWind`)을 붙였으나 Nanite 복셀은 WPO를 안 해서 `WorldPositionOffsetDisableDistance` 2 m로 사실상 끔 — 나무 바람은 후속 과제(스켈레탈 복귀 + DynamicWind 사본 플러그인 추천).

선행 문서(같은 폴더): `2026-09-28_forest_nanite_foliage_migration.md` — 숲 이관 전체, **K·L절(09-30 스켈레탈→스태틱 전환, DynamicWind 장시간 떨림 원인)**이 이 문서의 바로 앞 단계다.
바람 시스템 전체: `sfx_vfx/wind_system.md`.

아래는 세션 중 엔진 소스·에디터·PIE로 확인한 사실이다. 사용자 확인은 그렇게 적었다.

---

## 0. 요약

| 항목 | 상태 |
|---|---|
| A. 머티리얼 Time float 정밀도 → `MPC_TitanTime.WrappedTime`으로 전역 교체(풀·양치류·소형 가문비) | ✅ 완료 — 사용자: "풀 튀는 거 전혀 없고 시간 오래 지나도 안 떨림" |
| B. 스태틱 Megaplants 나무에 WPO 바람(`MF_TitanTreeWind` + 마스터 복사본) | ⏸ 만들었지만 **복셀 나무는 텍스처만 미끄러짐** → `WorldPositionOffsetDisableDistance` 200 cm = **사실상 나무 바람 끔(사용자 결정, 임시)** |
| C. 나무 바람 후속 방향 | 설계단계 — (1) 스켈레탈 복귀 + DynamicWind 사본 플러그인(추천) / (2) Voxelize 끄고 스태틱+WPO |
| D. P4 | ⚠ 전부 **미제출**(default CL, 다른 세션과 같은 워크스페이스) — 6절 |

---

## 1. 배경 — 왜 머티리얼 시간을 직접 만들었나

09-30에 DynamicWind 나무의 장시간 떨림 원인이 **월드 시간 float 정밀도**로 확정됐다(숲 이관 문서 L절,
`DynamicWindProvider.cpp:525`). 같은 원리가 **머티리얼 Time 노드**에도 그대로 있다:

- Time 노드는 `View.GameTime`/`RealTime`을 읽는데 이건 **float이고 월드 시간을 되감지 않은 값**이다
  (`Engine/Source/Runtime/Engine/Private/SceneView.cpp:2891`, `Public/SceneView.h:974-975`).
  월드가 1일쯤 돌면 float 최소 단위가 ~0.008 s가 되어 프레임마다 시간 증가가 들쭉날쭉 → 바람 WPO가 미세하게 떨린다.
- Time 노드의 **Period 옵션은 해결이 안 된다** — 셰이더 안에서 하는 `fmod`라 이미 뭉개진 float을 되감을 뿐이다
  (`HLSLMaterialTranslator.cpp:5294`).
- **엔진 시간(`ViewFamily.Time`) 자체를 되감는 안은 기각** — 렌더러가 그 절대값을 저장된 타임스탬프와 비교한다:
  데칼 페이드 `DecalRenderingShared.cpp:312`, 오클루전 히스토리 `SceneOcclusion.cpp:1109`, 프리미티브 최근 렌더 시간
  `SceneViewState.h:540`, `SceneVisibility.cpp:5574`, CSM 캐시 만료 `ShadowSetup.cpp:6558`. 시간이 뒤로 가면 이것들이 깨진다.

→ 엔진 시간은 그대로 두고, **바람 머티리얼만 읽는 별도 시간**을 우리가 double로 계산해 MPC로 넣는다.

---

## 2. A — `UTitanMaterialTimeSubsystem` + `MPC_TitanTime` (✅ 완료)

### 2.1 코드

`Source/titan_example/Environment/MaterialTimeSubsystem.h/.cpp` (신규, `UTickableWorldSubsystem`)
- 지원 월드: Game / PIE / **Editor**(`DoesSupportWorldType`) + `IsTickableInEditor() = true` — 에디터 뷰포트에서도 풀이 흔들려야 하므로.
- 매 틱: `WrappedTime = World->GetTimeSeconds() - TimeBase`를 **double로 계산한 뒤 float로** MPC에 쓴다(`.cpp:112-135`).
  `WrappedTimePrev = WrappedTime - DeltaSeconds` — 머티리얼의 **Previous Frame Switch**(이전 프레임 WPO → 모션 벡터)용.
  기준점을 옮긴 프레임에도 현재값−델타라서 모션 벡터가 튀지 않는다(`.cpp:130-132`).
- MPC를 `Initialize`에서 **`Super::Initialize`보다 먼저** 로드한다 — 부모가 끝에서 `GetTickableTickType()`으로 틱 여부를
  다시 정하는데 그때 MPC가 비어 있으면 영영 틱하지 않는다(`.cpp:56-58`, 엔진 `WorldSubsystem.cpp:101`). MPC가 없으면 로그 한 줄만 남기고 틱 안 함.

**기준점(TimeBase) 리셋 — 되감는 순간 이 값을 쓰는 애니메이션이 한 번 튀므로 화면이 가려질 때 한다**:
1. **시나리오 재시작 암전** — `Atitan_exampleGameState::Multicast_ScenarioRestartApply_Implementation`
   (`titan_exampleGameState.cpp:117-124`)에서 `ResetTimeBase()`. 멀티캐스트라 **서버·클라 각자** 리셋한다.
   무인 운영 한 사이클(~8.5분)마다 0부터 다시 시작하므로 정밀도도 최상.
2. **안전장치 주기** — 재시작이 없을 때(수동 운용·에디터 방치) `Titan.MaterialTime.WrapSeconds`(기본 **14400 = 4시간**)마다
   기준점을 주기만큼 옮긴다(`.cpp:116-126`). 헤더 주석 기준 4시간이면 float 최소 단위 ~0.001 s(30 fps에서 프레임 간 속도 불규칙 ~3%).
   월드 시간이 뒤로 간 경우(`Titan.Debug.AdvanceWorldTime` 음수 등)도 여기서 기준점을 다시 잡는다.
3. **수동** — 콘솔 `Titan.MaterialTime.Reset`.

**풍향/풍속 복사(10-01 추가, B절 함정 1 대응)**: `MPC_Wind`(`/Game/RealBiomes/Materials/MPC/MPC_Wind`)의
`Wind Direction`/`Wind Speed`를 매 프레임 `MPC_TitanTime`의 `WindDirection`/`WindSpeed`로 복사한다(`.cpp:137-143`).
양쪽 MPC에 해당 파라미터가 다 있을 때만(`.cpp:64-75`). 이유: 머티리얼 하나는 MPC를 **2개까지만** 참조할 수 있다.

### 2.2 에셋

**`/Game/Materials/MPC_TitanTime`** (신규): Scalar `WrappedTime`, `WrappedTimePrev`, `WindSpeed` · Vector `WindDirection`.

**Time 노드 교체**(→ MPC `WrappedTime` + Previous Frame Switch(`WrappedTimePrev`)):

| 에셋 | 교체한 Time 노드 수 |
|---|---|
| RealBiomes `MF_Wind_Sway` | 1 |
| RealBiomes `MF_Wind_Gusts` | 2 |
| RealBiomes `MF_Wind_Bending` | 3 |
| PN_interactiveSpruceForest `PN_WindAnimation` / `PN_MultiBend` / `PN_Bending` | 각 1 |
| `/Game/Materials/Masters/M_FoliageCustomWind` | 1 (Panner의 Time 입력) |

**엔진 함수 복사본** `/Game/Materials/Functions/TitanTime/`: `SimpleGrassWind`, `Motion_4WayChaos` — 엔진 원본은 수정할 수 없으므로
복사해서 Time을 교체하고, `MF_Wind_Bending`·`M_FoliageCustomWind`가 이 복사본을 호출하도록 바꿨다.

**대상 범위** = New_kadex_0811에서 **실제로 흔들리는 식생 머티리얼 계열 3개**:
- `M_VegetationShader`(RealBiomes) — 풀·양치류
- `M_FoliageCustomWind` — Megascans 양치류·지피
- PN `MA_Summer` — 소형 가문비

제외: MW 자작 머티리얼(레벨에 그걸로 흔들리는 메시가 없음). 프로젝트 전체에서 Time 노드를 쓰는 에셋은 **40개**(대부분 VFX) — **미처리**.

### 2.3 검증

사용자: **"풀 튀는 거 전혀 없고 시간 오래 지나도 안 떨림."** (`Titan.Debug.AdvanceWorldTime`으로 월드 시간을 민 상태 포함)

---

## 3. B — 스태틱 나무 WPO 바람 시도 (⏸ 현재 2 m = 사실상 끔)

09-30에 숲 나무를 스태틱 Nanite 메시로 바꾸면서 나무는 **바람 없음** 상태가 됐다. 10-01에 WPO로 바람을 다시 붙여 봤다.

### 3.1 만든 것

**`/Game/Megaplant_Library/Materials/Functions/MF_TitanTreeWind`** — Custom 노드 하나:
- 흔들림 = **`PreSkinnedPosition`(나무 로컬 위치)의 높이²** 비례 수평 흔들림 + `PerInstanceRandom` 위상 + 잎 flutter.
  Nanite 어셈블리 파트도 `PreSkinnedPosition`이 **나무 로컬 공간으로 변환돼** 들어온다(`NaniteVertexFetch.ush:34`,
  `NaniteVertexFactory.ush:244`) — 그래서 파트 단위가 아니라 나무 전체 기준 높이로 휜다.
- 시간·풍향·풍속은 **`MPC_TitanTime` 하나만** 읽는다(함정 1).
- 파라미터(그룹 `Tree Wind`): `TreeWind_SwayAmpCm` 5 · `TreeWind_SwayFreq` 0.25 · `TreeWind_FlutterCm` 1.5 · `TreeWind_SpeedRef` 1.

**마스터 복사본** `/Game/Megaplant_Library/Materials/Masters/`: `MA_Foliage_Trees`, `MA_Foliage_Trees_TwoBarkBlend`
(원본은 PVE 플러그인 콘텐츠라 복사). Set Material Attributes로 WPO를 끼워 넣고, 잎 마스크는 기존 **`DefaultLit Trunk` 스위치를 재사용**.

**나무 MI 7개 부모 교체**: `MI_Baltic_Pine_01_Bark`, `_Bark_Nubs`, `_Foliage`, `MI_Baltic_Pine_Sapling_01_Bark`,
`MI_Silver_Birch_01_Bark_Blend`, `_Bark`, `_Foliage`.

### 3.2 함정

1. **머티리얼당 MPC 최대 2개.** 잎 머티리얼은 PVE 잎 함수가 이미 MPC 1개를 쓴다 → 나무 함수가 `MPC_Wind` + `MPC_TitanTime`을
   더 쓰면 3개 → **잎 컴파일 실패, 기본 머티리얼로 대체되고 WPO 없음**("Material references too many MaterialParameterCollections").
   → 풍향/풍속을 `MPC_TitanTime`으로 복사(2.1)해서 **나무 함수는 MPC 하나만** 읽게 해 해결.
2. **Nanite 복셀 래스터는 WPO를 하지 않는다.** 래스터 경로는 스키닝만 처리한다(`NaniteRasterizer.usf` 1487~1600,
   스키닝은 1566 `SampleVoxelPerBrickSkinningTransform`). 반면 **셰이딩은 WPO를 적용**한다(`NaniteVertexFactory.ush` 1126~1148).
   → 복셀로 그려지는 나무는 **모양은 그대로인데 텍스처만 바람만큼 미끄러진다.**
   복셀화는 거리 고정이 아니라 **화면 오차 기준**이라, 실측상 소나무는 ~5 m에서도 복셀이었다.
   확인 도구: Nanite Visualization 모드 **`Voxels`**, **`EvaluateWPO`**.
3. **잎 flutter가 멀리서 "흐르는 텍스처"로 보임.** flutter(노멀 방향 ±~3 cm, 2.3 Hz, `dot(위치)` 줄무늬 파동)가 그림자 밝음/어둠을
   반복시켜 멀리서 TSR을 거치면 흐르는 무늬처럼 보인다. 소나무 잎 `TreeWind_FlutterCm` 0으로 가까이선 해결.
4. **MCP로 `SetMaterialAttributes`의 `AttributeSetTypes`를 쓰면 에디터 크래시**(UECC …3454487A…, Array index out of bounds).
   핀 추가는 사용자가 에디터에서 직접 했다.

### 3.3 현재 값 (임시)

| 대상 | 값 |
|---|---|
| 소나무 MI 3개 | `TreeWind_SwayAmpCm` **0.5** |
| 소나무 잎 MI | `TreeWind_FlutterCm` **0** |
| 자작 MI | 함수 기본값(Flutter 1.5 그대로) |
| `WorldPositionOffsetDisableDistance` | 200 cm (남아 있지만 아래 플래그로 무의미) |
| **`bEvaluateWorldPositionOffset`** | **false** — 레벨 숲 컴포넌트 24개 + 두 스폰너 템플릿(`PCG_SplineForest_tree2_Nanite`, `PCG_PlacedTrees`), 2026-10-06 |

**사용자 결정**: 나무 바람은 **끈 상태(임시)**. 나중에 플러그인을 뜯어고치든 해서 **복셀까지 흔들리게** 할 예정.

**2026-10-06 — 2 m 거리 제한으로는 안 꺼졌던 이유**: WPO 끄는 거리는 나무 밑동이 아니라 **인스턴스 바운드 중심(수관 높이)**에서
**각 뷰의 카메라 위치**까지 잰다(`NaniteCullingCommon.ush:473-475` `CenterTranslatedWorld - NaniteView.CullingViewOriginTranslatedWorld`,
501-503에서 비교). 땅 위 메인 카메라는 수관 중심 2 m 안에 못 들어가 한 번도 안 켜졌지만, 높이 달린 **환경맵 카메라(씬 캡쳐, 뷰마다 따로 계산)**는
수관 중심 2 m 안의 나무에서 WPO가 켜져 자작 잎 flutter(1.5)와 복셀 텍스처 미끄러짐이 캡쳐 화면에만 보였다. → 거리 제한 대신
**`bEvaluateWorldPositionOffset=false`**로 완전히 끔(모든 뷰, WPO 비용도 0). 바람을 다시 넣을 때 true로 되돌릴 것.

---

## 4. C — 나무 바람 후속 선택지 (설계단계)

| 안 | 내용 | 장점 | 위험/비용 |
|---|---|---|---|
| **(1) 추천** | 스켈레탈로 복귀 + DynamicWind를 **다른 이름(예: `TitanWind`)의 프로젝트 플러그인 / 트랜스폼 프로바이더**로 복사 | 이름이 달라 엔진 PVE와 충돌 없음(L.3의 포크 실패 회피). 시간 되감기(`MPC_TitanTime`과 같은 방식)·풍향 축 뒤집힘(`DynamicWindEval.usf:168-170`)을 직접 수정 가능. **복셀도 브릭 스키닝으로 흔들린다** | 예상 1~2일. Renderer private 헤더를 써야 할 수 있음 |
| (2) | Voxelize 끄고 스태틱 + WPO(지금 만든 B절 그대로) | 작업물 재사용 | 복셀 LOD의 이점(LOD 튐 없음·잎 반짝임 감소·성능) 상실 위험 — **실측 필요** |

(1)로 가면 B절 작업물(`MF_TitanTreeWind`, 마스터 복사본 2개, MI 7개 부모 교체)은 **미사용**이 된다(되돌릴지 여부는 그때 결정).
A절(`MPC_TitanTime`)은 어느 쪽이든 그대로 쓴다.

---

## 5. 콘솔 / 디버그

| 명령 | 뜻 |
|---|---|
| `Titan.MaterialTime.WrapSeconds <초>` | 안전장치 되감기 주기(기본 14400) |
| `Titan.MaterialTime.Reset` | `WrappedTime`을 지금부터 0으로(풀·나무 바람이 한 번 튐) |
| `Titan.Debug.AdvanceWorldTime [초=43200]` | 월드 `TimeSeconds`를 앞으로 민다 — 장시간 떨림 재현용, PIE 확인용으로만(`WindSource.cpp:15-32`) |
| Nanite Visualization `Voxels` / `EvaluateWPO` | 복셀로 그려지는 범위 / WPO 평가 범위 확인 |

---

## 6. P4 — 미제출 상태 (insung_desktop1, default CL)

⚠ **같은 워크스페이스를 다른 세션도 쓰고 있다.** 09-30 변경분은 공유 default CL 때문에 다른 세션의 change 16('총구 떨림 문제 수정…')에
같이 제출된 전례가 있다(숲 이관 문서 K.6). 제출할 때 파일 목록을 직접 골라 담을 것.

- **코드**: `MaterialTimeSubsystem.h/.cpp`(add), `titan_exampleGameState.cpp`, `WindSource.h/.cpp`, `ForestTreeRow.h`
- **에셋 편집**: RealBiomes `MF_Wind_Sway`/`MF_Wind_Gusts`/`MF_Wind_Bending`, `PN_WindAnimation`/`PN_MultiBend`/`PN_Bending`,
  `M_FoliageCustomWind`, 나무 MI 7개, PCG 그래프 2개
- **에셋 추가**: `MPC_TitanTime`, `TitanTime/SimpleGrassWind`·`Motion_4WayChaos`, `MF_TitanTreeWind`, 마스터 복사본 2개
- **`L_Basic.umap` 체크아웃** — 테스트 볼륨 `TEST_DynamicWindTimeTest`가 들어 있음. **제출하면 안 됨**(revert 대상).

---

## 7. 남은 작업

1. **나무 바람** — 4절 (1)/(2) 결정 후 진행. (1)이면 B절 작업물 정리.
2. **P4 제출** — 6절 목록, 다른 세션 파일과 분리. `L_Basic.umap` revert.
3. (선택) 나머지 Time 노드 에셋 40개(대부분 VFX) — 장시간 운용에서 떨림·끊김이 보이는 것만 `MPC_TitanTime`으로.
4. 리셋 순간의 튐이 암전 밖(4시간 주기 되감기)에서 눈에 띄는지 — 전시 장시간 운용에서 관찰.
