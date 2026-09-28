# real2world — 씬 좌표 ↔ 위경도 ↔ 미니맵 픽셀 변환 (현재 동작)

2026-09-22 갱신 / 에버그린 / `GeoCoordinateUtils.h` 가 지금 어떻게 씬 좌표를 실좌표로 바꾸는가. New_kadex_0811 은 **현실 1:1** 이고 보정은 랜드스케이프 코너 2점(정확값)이다.

> 📌 [2026-09-22] 전면 재작성. 이전 내용은 원날짜 불명의 스크래치 메모(옛 레벨 픽셀↔위경도↔월드 대응표)였고 맨 아래 "이력"에 보존. 2026-08-26 재스케일·재보정 경위는 `level_new_kadex_0811/2026-08-26_level_rescale_to_real_world.md`, LIG 지도 공유 수치는 `protocol/2026-09-22_minimap_georeference_for_lig.md`.

---

## 1. 한 줄 요약

- **레벨(`New_kadex_0811`)은 현실과 1:1** — 2026-08-26 에 26.2% 크던 것을 k=0.7921933250 으로 균일 축소. 큐브 독립 검증 오차 0.001 m / 2.8 km.
- **보정점은 랜드스케이프 대각 코너 2점**(설계값, 눈대중 아님). 코드가 계산하는 fit: `Scale ≈ 1.000554`, `Rotation = -86.229°`. 씬 **+X ≈ 남쪽**(방위각 176.2°), +Y ≈ 서쪽.
- 내부 canonical 은 lat/lon, LIG 와이어로 나갈 때만 UTM 52S(WGS84).
- 미니맵은 위경도 정렬 정사진 1024×1024, 픽셀↔위경도는 코너 상수로 **정확한 선형 보간**(fit 무관).

## 2. 파일과 진입점

전부 `Source/titan_example/UI/GeoCoordinateUtils.h`(헤더 온리, `namespace GeoCoordinateUtils`).

| 함수 | 용도 | 주 호출자 |
|---|---|---|
| `WorldLocationToLatLong(FVector)` | 씬 → 위경도 | UAV 위경도 표시, 미니맵 마커 |
| `LatLongToWorldLocation(lat, lon)` | 역변환 | 미니맵 클릭 → 월드 |
| `WorldLocationToUTM` / `UTMToWorldLocation` | LIG 와이어 좌표(`East`/`North`/`Zone`/`Letter`) | `UGVRemoteControlSubsystem`, `HQ_MissionMoveToEngage` |
| `GetDistanceScaleFactor()` | 씬 cm 당 실제 cm = fit 의 `Scale`(≈1.0006) | 아래 §4 |
| `SceneYawToBearingDegrees` / `SceneVectorToBearingDegrees` | 씬 yaw/방향 → 진방위(0=북, 시계방향) | RCWS 방위각, UAV 헤딩, 나침반 리본 |
| `SceneYawToMapScreenAngleDegrees` | 씬 yaw → 미니맵 아이콘 화면 회전각 | 미니맵 차량 아이콘 |
| `MapPixelToLatLong` / `LatLongToMapPixel` | 미니맵 픽셀 ↔ 위경도(정확, 회전 없음) | |
| `WorldLocationToMapPixelUV` / `MapPixelUVToWorldLocation` | 씬 ↔ 미니맵 UV(위 둘의 합성) | `AMinimapCaptureActor`, 대시보드 위젯 |
| `GetMapSceneWidthCm()` | 미니맵 전체 폭이 차지하는 씬 cm | FOV 콘, 100 m 축척바 |
| `FormatDMS` | `N 37° 55' 13.5"` 문자열 | UI |

## 3. 보정 — `GeoCalibrationPoints` 와 fit

```cpp
constexpr FGeoCalibrationPoint GeoCalibrationPoints[] = {
    { 37.9204090, 128.1948530, -98893.69699143617,  98893.69458325558 }, // Landscape_1 코너, 씬 -X/+Y
    { 37.9038185, 128.2188489,  99141.95916536597, -99141.96157354656 }, // Landscape_1 코너, 씬 +X/-Y
};
```

- SceneX/Y 는 `Landscape_1` 트랜스폼(location `(-98893.697, 98893.695, 0)`, yaw -90, XY scale 49.11598615, 4032×4032 quads)에서 유도한 **정확한 코너**. 한 변 198035.656 cm = 1980.36 m = 코너 위경도가 실제로 벌어진 거리.
- `ComputeSceneToRealFit()` 은 최소제곱 상사 fit(균일 스케일 + 회전, 반사 없음)인데, 2점이면 정확한 복소 나눗셈으로 환원 → 두 코너를 잔차 0 으로 통과.
- **왼손 좌표계 보정**: `SceneDY` 를 부호 반전해서 복소평면에 올린다(2026-07-21 픽스, 지금도 유효). 이걸 빼면 고정 방위는 맞아도 **회전 방향이 거울상**이 된다.
- 결과: `Scale 1.000553788`, `RotationRadians -1.504984324`(-86.229250°), Anchor 37.9121138 N / 128.2068509 E.

**Scale 이 정확히 1.0 이 아닌 이유**: `MetersPerLatitudeDegree = 111320` 이 평면 상수라 이 위도(실제 110994.8 m/°)에서 N-S 를 0.293% 과대, cos 유도 경도 계수는 E-W 를 0.126% 과소 → 균일 스케일이 그 사이(+0.055%)에 놓인다. 양방향에서 같은 상수를 쓰므로 위경도 판독엔 상쇄되고 보정점은 정확히 통과한다. **이 상수를 "고치지" 말 것** — `GetMapSceneWidthCm()`/축척바까지 같이 움직인다.

**랜드스케이프를 옮기거나 리사이즈하면 두 SceneX/Y 를 반드시 새 트랜스폼에서 재계산**할 것. 그대로 두면 모든 위경도·속도·사거리 판독이 조용히 틀어진다.

## 4. `GetDistanceScaleFactor()` — 지금은 거의 항등

씬 cm → 실제 cm 배율. 08-26 이전(옛 `kadex_demo_0716` 5점 보정)엔 **1.2135**(씬이 현실보다 작았음)였고 지금은 **≈1.0006**. 호출부 15곳은 그대로 두는 게 맞다(레벨 스케일이 또 바뀌면 자동으로 따라감):

- **표시**(곱함): UAV 고도/속도(`UAVPawn::UpdateStatusHUDFlightData`), UGV 속도/주행거리(`UGVPawn::UpdateUGVStatusData`, `UGVAIController`), LIG 속도 보고(`UGVRemoteControlSubsystem`), 미니맵 100 m 축척바(`Monitor1Widget`/`SelfDefenseMonitor1Widget::GetScaleBar100mPixelWidth` — 이쪽은 **나눔**, 실제 100 m 를 씬 cm 로).
- **실스펙 → 씬 물리**(나눔): UAV 상승 높이·가속(`AscendHeightMeters`, `VerticalAccelMPerSecSq`), UGV 에스코트 속도·제동 곡선, RCWS 포구속도 + 투사체 중력(`SceneMuzzleVelocityCmS`/`SceneProjectileGravityScale`, 둘 다 1/Scale 이라 비행시간 불변).
- **의도적으로 안 쓰는 곳**: `RCWSComponent` 거리계 `RangeMeters` — 트레이스가 실제로 닿는 거리와 표시가 일치해야 하므로 plain cm→m(2026-07-24 픽스).

위젯(`Monitor1/2`, `SelfDefenseMonitor1/2`, `SelfDefenseDashboard`, `StatusHUDWidget`)은 `Data.SpeedKmh` 등을 그냥 출력만 하고 계산이 없다 — 보정은 전부 상류 생산자에서.

## 5. 미니맵 픽셀 ↔ 위경도

이미지 `/Game/widget/m_map`(원본 `m_map.png`, 1024×1024, 2026-07-30 생성, `AMinimapCaptureActor::MapTexture`).

```cpp
MapImageWidth/Height        = 1024
MapContentPixelMinX/MaxX    = 0 / 1023      // 좌우 여백 없음
MapContentPixelMinY/MaxY    = 47 / 977      // 행 0~46, 978~1023 은 검은 레터박스
MapTopRightLatitude/Longitude   = 37.9374196 / 128.2149834
MapBottomLeftLatitude/Longitude = 37.8928155 / 128.1581718
```

- 이미지가 **위경도 정렬(북쪽 위, 회전 없음)** 이라 `MapPixelToLatLong` 은 콘텐츠 영역 안에서 축 정렬 선형 보간 — fit 과 완전히 독립, 오차는 픽셀 고르는 정밀도뿐.
- 씬 → 미니맵은 `WorldLocationToLatLong`(fit) → `LatLongToMapPixel`(정확) 합성. 아이콘 회전은 `SceneYawToMapScreenAngleDegrees = SceneYaw + 90° − RotationDegrees`.
- 픽셀은 정사각이 아님(E-W 4.88 m, N-S 5.32 m — 지리 박스 ≈4996×4951 m 를 1024×931 에 넣어서). 코드는 축별 보간이라 문제없음.
- **랜드스케이프 동쪽 오버행 (알고 있는 사실, 무시하기로 결정 2026-09-22)**: 레벨의 `-Y` 변이 미니맵 동쪽 끝(128.2149834°E)보다 205~340 m 밖에 있다 → 그 띠에서는 `LatLongToMapPixel` 이 X > 1023 을 돌려주고 마커가 이미지 밖으로 나간다(`+X-Y` 코너 = 픽셀 (1092.6, 747.6)). 시나리오 액터가 전부 랜드스케이프 중심부에서만 움직이므로 실질 영향 없음 — 재생성·클램프 모두 안 함. 나중에 그 띠를 쓰게 되어 **이미지를 다시 뽑으면 위 상수 8개를 전부 재측정할 것**(코너 4 + 레터박스 4). UTM 정렬 이미지로 바꾸면 `MapPixelToLatLong` 구조 자체(위경도 박스 전제)를 손봐야 한다.

## 6. UTM (LIG 와이어)

`LatLongToUTM`/`UTMToLatLong` — WGS84 타원체, 표준 Redfearn 정/역식, Zone 은 경도에서 계산(이 지역 **52**), Letter 는 MGRS 밴드(**S**). ICD 에 Datum 명시가 없어 WGS84 로 가정(`protocol/protocol_icd.md` §6). 지도 네 꼭지점의 UTM 값은 `protocol/2026-09-22_minimap_georeference_for_lig.md` §3.1.

## 7. 재보정이 필요해지는 경우

| 상황 | 할 일 |
|---|---|
| 랜드스케이프 이동/리사이즈 | `GeoCalibrationPoints` SceneX/Y 를 새 코너에서 재계산(§3) |
| 새 레벨 | 그 레벨 랜드스케이프 코너의 위경도(설계값)로 2점 교체. 회전·스케일 방향이 완전히 다를 수 있음(옛 레벨은 +X=동, 지금은 +X=남) |
| 미니맵 이미지 교체 | §5 상수 8개 재측정. 위경도 정렬이 아니면 함수 구조 변경 |
| 레벨 스케일 변경 | `GetDistanceScaleFactor` 가 자동 추종하지만 **물리 스펙 변환 경로(§4)의 씬 속도가 바뀌므로 시나리오 타이밍 재점검** |

---

## 이력

- **2026-07-21** 왼손 좌표계 회전 방향 거울상 버그 픽스(SceneDY 반전).
- **2026-07-30** `kadex_demo_0716` 레벨, 눈대중 랜드마크 5점 최소제곱 fit(RMS 53 m/5 km), 미니맵 `m_map.png` 코너 정확 생성.
- **2026-08-14/16** UTM 정/역변환 추가(LIG ICD 확정).
- **2026-08-26** `New_kadex_0811` 을 1:1 로 재스케일(k=0.7921933250), 보정점을 랜드스케이프 코너 2점으로 교체, `GetDistanceScaleFactor` 1.2135 → 1.0006. 상세 `level_new_kadex_0811/2026-08-26_level_rescale_to_real_world.md`.
- **2026-09-22** 이 가이드 재작성, LIG 지도 지리참조 수치 확정.

<details>
<summary>옛 스크래치 메모 원문 (원날짜 불명, 옛 레벨 `kadex_demo_0716` 기준 — 지금은 무효)</summary>

```
이미지 픽셀 위치 / 위도 경도 / 언리얼 에디터 월드 위치

1. 502, 26        37°55'36.09"N 128°10'53.53"E    -2990.0, -110670.0
2. 527, 632       37°54'44.30"N 128°10'59.46"E     8210.0,  20210.0
```
</details>
