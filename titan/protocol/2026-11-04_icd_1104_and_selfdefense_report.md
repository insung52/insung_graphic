# ICD 1104 반영(UGV) + 자기방호 드론 위치보고(K05.1) + 해발 고도 기준

2026-11-04 / 완료(코드, 빌드 전) / LIG 11/04 피드백 S1 담당(P1~P4, P7~P9) 구현. 빌드·실측은 사용자. 에디터 작업은 §6 목록으로 미룸.

관련: 작업 목록 `documents/feedback_1104/2026-11-04_feedback_1104_work_list.md`(§1 S1), 역질문 `…/2026-11-04_lig_questions_feedback_1104.md`,
ICD 정본 `protocol/lig_icd_ugv_rc_full.md`(1104 반영), `protocol/protocol_icd.md`. 같은 날 S2(드론 탐지, D2/D3)가 같은 파일
`Network/UGVRemoteControlSubsystem.cpp`를 먼저 수정했고, S1 변경은 그 위에 얹었다.

---

## 1. UGV ICD 1104 (`Network/UGVRemoteControlSubsystem.cpp/.h`)

| ID | 함수 | 변경 |
|---|---|---|
| P1 | `SendBasicinfo` | 키 `ConrtrolRight`→`ControlRight`, `Batterry`→`Battery` |
| P1 | `BuildDetectionObjectJson`(익명 네임스페이스) + `SendObjectDetectionResult` 호출 2곳 | `bUseLatterTypo` 인자 삭제, RCWS·ADU 모두 `Letter` |
| P1 | `Handle_RC_FireMode` | 원문 `BRUST`/`CONTINUS`에 더해 정상 철자 `BURST`/`CONTINUOUS`도 수용(LIG가 오타를 고쳐 보낼 경우 대비) |
| P2 | `SendRCWSStatus` | `Azimuth` = `CurrentData.AzimuthDegrees` 반올림 0~359(진북 0° 시계방향 — RCWS 화면 표시값과 동일), `Elevation` = `CurrentData.ElevationDegrees + 90` 0~180 clamp(90=수평), `TotalBullet` = `AmmoCurrent`. RCWS 없으면 `RCWSStatus=OFF` + 세 필드 0 |
| P3 | `Initialize` / `SendBasicInformation` | `RuntimeStartSeconds` 기록, `Runtime` = `"HH-MM-SS"`(하이픈, 시는 24 넘어도 계속 증가). 프로세스 기동 기준·재시작 초기화 없음 — Q3 답이 "초기화"면 재시작 훅에서 `RuntimeStartSeconds`만 다시 찍으면 됨(주석 표시) |
| P4 | `Handle_RC_RemoteDriving` | `TurnInput = sign(s)·|s/100|^RemoteSteeringExponent·RemoteSteeringScale`. 신규 Config `RemoteSteeringScale=0.6`, `RemoteSteeringExponent=1.5`(원격 경로 전용, 로컬 키보드 경로 불변). 입력 100 → 0.6, 50 → 0.21, 20 → 0.054 |
| P5 | — | `ObjectClass` 현행 유지(작업 없음). 분류는 S2가 `Detection/DetectionClassification`으로 공용화 |

판단 메모:
- **Elevation 기준** — `CurrentData.ElevationDegrees`는 월드 수평 기준이라 차체가 기울면 마운트 clamp(-20~60°, 차체 기준) 밖 값이 나올 수 있다(평지에선 70~150). 사용자 결정으로 ICD 문구 그대로 "고각+90"을 이 값으로 보낸다. 차체 기준이 필요하다는 LIG 답이 오면 `RCWSFireControlComponent.cpp:~1146`의 `Owner->GetActorQuat().Inverse() * SightWorldQuat` 식을 쓰면 된다.
- **TotalBullet vs 최대 탄수** — 현재 UGV(`BP_UGV_0901`) AmmoMax는 C++ 기본 600(BP 오버라이드 없음). ICD "MAX 200발"에 맞춘 최대 탄수 변경은 S4(W14/R2) 담당.
- 탐지 결과의 ADU 쪽 `Altitude` 키: ICD 1104 시트2 ADU Objects(E45~E53)에는 `Altitude`가 없는데 우리는 보냄(0 고정). 무해한 추가 필드라 유지.

## 2. 해발 고도 기준 (P7) — 프로젝트 최초

- 신규 `Settings/TitanGeoSettings.h/.cpp` — `UTitanGeoSettings`(UDeveloperSettings, `DefaultGame.ini`, Project Settings > Titan Geo): `SceneZeroMslMeters`, `MslReferencePoints[]`.
- `UI/GeoCoordinateUtils.h` — `GetAltitudeMslMeters(double SceneZ)` / `(const FVector&)` = `Z/100 × GetDistanceScaleFactor() + SceneZeroMslMeters`. K05.1·드론 JSON(S2 D4) 공용.
- 콘솔 `titan.Geo.CalibrateMsl` — 기준점마다 랜드스케이프 지면을 위→아래로 트레이스(나무/바위에 맞으면 무시 목록에 넣고 재시도, 클래스 이름에 `Landscape`), 점별 오프셋·평균·RMS/최대 잔차·참고 회귀(`해발 = a·씬m + b`)를 로그로.

### DEM 조회 (2026-11-04, OpenTopoData `srtm30m`)

랜드스케이프 대각 코너 두 점(37.9204090/128.1948530, 37.9038185/128.2188489) 사이 1/6·1/2·5/6 격자:

| 위도 \ 경도 | 128.1988523 | 128.2068509 | 128.2148496 |
|---|---|---|---|
| 37.9176439 | 863 | 814 | 800 |
| 37.9121138 | 905 | 802 | 707 |
| 37.9065836 | 842 | 650 | 595 |

평균 **775.3 m** → 처음 임시값 `SceneZeroMslMeters = 775`(아래 실측으로 794로 대체됨. 가정: 랜드스케이프 중간 높이 ≈ 씬 Z 0. 씬 지면 Z를 재지 못해서). 참고: 미니맵 bbox 10×10 격자는 553~969 m(평균 774.8).

### ⚠ 단일 오프셋의 한계

랜드스케이프 Z 스케일 23.7658 → 하이트맵이 표현할 수 있는 높이는 ±256×23.77 cm = **±60.8 m(최대 121.7 m)**. 그런데 같은 영역 실제 지형 차는 **310 m 이상**(595~905). 즉 높낮이가 현실보다 최소 2.5배 압축돼 있어서, 오프셋 하나로는 지점에 따라 **±100~150 m** 어긋날 수 있다.
분석 단계에서 쓴 "450 m" 추정은 틀렸다(LIG 샘플 좌표가 맵 밖 남동쪽 계곡).

### 실측 결과와 결정 (2026-11-04, 사용자 PIE)

`titan.Geo.CalibrateMsl` 9점: **평균 오프셋 794.05 m**(임시값 775 대비 +19), RMS 잔차 85.81 m, 최대 잔차 142.89 m, 참고 회귀 a = 1.7687.
회귀로도 잔차가 거의 안 줄어든다 — 높이 비율 문제가 아니라 랜드스케이프를 프로젝트에 맞게 깎고 올린 부분이 많아 실제 지형과 지점별 대응 자체가 없다는 뜻.
**결정(사용자): 단일 오프셋 유지, `SceneZeroMslMeters = 794`.** 비율 보정·DEM 테이블 방식은 안 함 — 씬 안에서 수평 비행하면 보고 고도도 일정해야 하므로.
결과적으로 보고 해발은 "씬 고도 + 794 m"이고, 지면 해발은 실제 DEM과 지점별로 최대 ~143 m 차이 날 수 있다(LIG에 설명 필요 시 이 수치 사용).
값은 `Config/DefaultGame.ini` `[/Script/titan_example.TitanGeoSettings]`(정본)와 C++ 기본값 둘 다 794.

## 3. 드론 위치보고 K05.1 (P8)

신규 `Network/SelfDefenseReportSubsystem.h/.cpp` — `USelfDefenseReportSubsystem`(GameInstance 서브시스템, `Config=Game`).

- 활성 조건: 로컬 PC의 `PlayerAxis == SelfDefense`(호스트/클라 무관). 드론 = 레벨의 첫 `ADronePawn`.
- 주기 `PositionReportHz = 1`, **실시간 슬롯 고정**(`FPlatformTime::Seconds`로 다음 송신 시각 관리, 나머지 보존, 히치로 밀리면 1회만 보내고 재정렬).
  처음엔 FTSTicker DeltaTime을 누적했는데 게임 dt 클램프 때문에 저fps(17fps PIE)에서 1.5~2.8초 간격으로 나가서 고침(사용자 PIE 로그).
  드론 객체탐지 JSON은 짐벌 캡처마다(`UDroneDetectionReporterComponent::CaptureFrame→EmitFrame`) 보내는 구조라 이 문제와 무관. 메시지 `{"UnitPositionList":[{…19필드…}]}`, 봉투 없음(수신 사양 미정).
- 필드 규칙(사용자 결정):

| 필드 | 값 |
|---|---|
| `unitReferenceNumber` | `KIB01-HHC01-00000-00000-FPV01`(엑셀 F2, Config) |
| `latitude`/`longitude` | 정수 문자열. 기본 Magnitude `round(도/90×16777215)` / `round(도/180×33554431)`, Config로 Offset(`(도+90)/180`, `(도+180)/360`) 전환. 1~최댓값 clamp |
| `locationSource`/`locationQuality` | `"미식별"` |
| `course` | 8방위 `북/동북/동/동남/남/서남/서/서북`. 수평속도 ≥ `CourseHoldSpeedKph`(2) 이면 이동방향, 아니면 기수방향 |
| `speedKph` | 수평 대지속도 × `GetDistanceScaleFactor` × 3.6, `"%.2f"` |
| `altitudeFeet` | `GetAltitudeMslMeters × 3.28084`, `"%.2f"`(해발) |
| `year`~`second` | UTC + `UtcOffsetHours`(9, KST), 연 4자리·나머지 2자리 0채움 |
| `aerialType`/`maritimeType`/`subsurfaceType`/`groundType` | 공란 |

- 출력: `Saved/SelfDefenseReports/K051_<시작시각>.jsonl`(한 줄 1메시지, `bWriteReportFile`) + 로그 `[SelfDefenseReport] K05.1 …`(`bLogEachPositionReport`). UDP는 `bEnableUdpSend=false` 기본.
- 공용 송신 `SendReportJson(Port, Json, FileTag)` — S2 D5(드론 객체탐지 JSON)가 `DetectionReportPort` + `"DroneDetection"`으로 같은 경로 사용(짐벌 캡처마다 ~30Hz).
  객체탐지 파일 기록은 별도 스위치 `bWriteDetectionReportFile`(기본 **꺼짐** — 켜면 `DroneDetection_*.jsonl`이 시간당 ~200MB). K05.1 파일은 `bWriteReportFile`(기본 켜짐).
- 콘솔 `titan.SD.DumpK051` — 현재 상태로 1건 생성해 로그(송신 안 함).
- 속도는 `ADronePawn::GetFlightVelocityMS()`(S2 추가: 시뮬 주체면 강체 속도, 원격이면 `RepVelocity`) — 그래서 클라이언트(자체방호 PC가 클라)에서도 값이 나온다. 위치/자세는 원격 프로세스에서도 `Rep*`(30Hz)로 갱신되는 액터 트랜스폼.

예시(맵 중심 37.912114N 128.206851E, 씬 Z = 12000 cm → 해발 794 + 120.07 = 914.07 m, 북동 15 m/s — 손계산, 형식 확인용):
`{"UnitPositionList":[{"unitReferenceNumber":"KIB01-HHC01-00000-00000-FPV01","latitude":"7067330","longitude":"23899489","locationSource":"미식별","locationQuality":"미식별","course":"동북","speedKph":"54.00","altitudeFeet":"2998.92",…,"year":"2026","month":"11","day":"04","hour":"14","minute":"23","second":"05","aerialType":"",…}]}`

## 4. 로비 송신 설정 (P9)

`UI/AxisSelectionWidget.h/.cpp` — BindWidgetOptional `EditableText` 3개 `ReportIPText` / `PositionReportPortText` / `DetectionReportPortText`
(기본 `127.0.0.1` / `9100` / `9101`, 우리 임의값). NativeConstruct 프리필(FromInt), 기존 `HandleNetworkFieldCommitted` → `ApplyNetworkFieldsToSubsystem`에서
`SetReportNetworkConfig` + `SaveConfig()`(Game.ini `[/Script/titan_example.SelfDefenseReportSubsystem]`), 툴팁. RCIP 등 기존 5필드와 같은 경로.

## 5. 빌드 후 확인 포인트

- **Generate project files 필요**(신규 파일 4개). Build.cs 변경 없음.
- UGV축 + `rc_mock_client`(또는 Wireshark 8010):
  - `UGV_Period_Basicinfo`: `ControlRight`, `Battery` 키(옛 키 사라짐)
  - `UGV_RCWS_Status`(20Hz): `Azimuth`(RCWS 화면 방위와 일치), `Elevation`(수평 조준 시 90, 위로 올리면 증가), `TotalBullet`(사격 시 감소)
  - `UGV_Period_ObjectDetectionResult` src 13: `Letter` 키(옛 `Latter` 사라짐)
  - `UGV_Period_BasicInformation`(1Hz): `Runtime` `"00-01-23"` 형식, 시나리오 재시작해도 계속 증가
  - `RC_RemoteDriving` Steering 100 → 이전보다 확연히 덜 꺾임, `RC_FireMode` `BURST` 수용
- 자체방호축: `Saved/SelfDefenseReports/K051_*.jsonl` 1초 1줄, 콘솔 `titan.SD.DumpK051`, 드론 이동 방향과 `course` 일치, 자체방호 PC가 클라이언트여도 `speedKph`가 0이 아님.
- 게임 레벨에서 `titan.Geo.CalibrateMsl` → 평균 오프셋/잔차 기록, `SceneZeroMslMeters` 교체 여부 결정.
- 로비: 3필드 입력 → 재실행 시 값 유지.

## 6. 미룬 에디터 작업

1. **P6 IR 흑백** — `Content/Vehicles/UGV/Materials/M_PP_RCWS_IR`의 녹색 틴트 제거(흑백 white-hot). 트럭 RCWS도 같은 머티리얼(양 축 공통 변경).
2. **P9 WBP** — 로비 WBP에 `EditableText`(Editable Text Box 아님) 3개 `ReportIPText`/`PositionReportPortText`/`DetectionReportPortText` 배치.
3. ~~P7 실측~~ — **완료(2026-11-04)**: 실측 794.05 → `DefaultGame.ini` `SceneZeroMslMeters=794` 반영(§2 실측 결과와 결정).
