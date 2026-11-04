# LIG 피드백(11/04) 작업 목록

2026-11-04 / 구현 완료(P4 change 527, 2026-11-05), 일부 검증·LIG 답 대기 / LIG 11/04 수정요청을 세션별 작업으로 정리 — 4개 세션 코드 분석 결과와 사용자 결정 반영.

> **2026-11-05 상태**: 아래 "바로" 항목과 J1·J4, ICD 정합성 B5·B4·B1, W5 F1, P6(IR)·J2 에디터 작업까지 전부 구현·제출됨(change 527).
> 남은 검증·대기 항목과 다른 PC 이어받기 절차는 **`2026-11-05_handoff.md`** 가 정본.

원본: `email.md`, ICD 엑셀 0806/1104·`기동플랫폼_위치보고전문형식.xlsx`(비번 titan), `sample1_boxed_metadata.json`, `velocityone_tester_offline.tar.gz`.
LIG 질문: `2026-11-04_lig_questions_feedback_1104.md` (질문 Q1~Q5, 이해 확인 B-1·B-2, 송부용 예시 §F).

> 메일 머리말: UI 수정·카메라 시점 변경은 후순위(디자이너 협의 후). 구현 항목 위주로 진행.
> 규모는 빌드·검증 포함 추정. 빌드는 사용자가 한다.

---

## 0. 한눈에 보기

| 세션 | 담당 | 규모 | 바로 시작 | LIG 답 대기 |
|---|---|---|---|---|
| **S1** 프로토콜 | UGV ICD 1104 반영, 드론 위치보고(K05.1), 해발 고도 | 약 1.5일 | P1~P7, P9 | P8 전송부(Q1·Q2) |
| **S2** 드론 탐지 | 드론 BoundingBox JSON, RCWS 박스 어긋남, 드론 RTSP | 약 2~2.5일 | D1~D4 | D5 전송부(Q1·Q2) |
| **S3** 조이스틱 | 수동 토글 2개 재편(모드 A: H2=트럭 RCWS + 메인 스틱=드론 카메라 / 모드 B: 드론 전체 수동) | 약 1일 | J1 코드 완료 | J2 에디터, J3 실장비 |
| **S4** RCWS | 자동 재장전, 최대 탄수, 고각 제한, 초기화 시간 회신 | 반나절~1일 | R1~R4 | R3 범위(디자인팀 질문) |
| S5 UI·카메라 (보류) | 패널 색·로고·드론 패널 이동·명칭·카메라 위치 | 약 1일 | — | 디자이너 협의 |

### 추가로 진행된 작업 (11/04 중 결정)

| 항목 | 결과 |
|---|---|
| 팀장님 `ICD_정합성_수정사항.md` 대조 | B5 `RC_Movement` 속도 지령(60°/s), B4 `RC_RemoteDriving` 0.5s 타임아웃 정지, B1 주기 송신 실시간 dt — 구현(S1). 나머지(A6/A7 ACK, B3, A8, C3~C10, D1, D3)는 기존 LIG 답변·방침으로 보류 |
| W5 박스 어긋남 재분석 | 우리 송신 계산 경로에 계통 오차 없음(ugv_controller_demo 겹쳐보기로 정상 확인). F1(탐지 화면비 매 스캔 동기화)만 반영, F2/F3·디버그 도구는 안 함 |
| P7 해발 | `SceneZeroMslMeters=794` 단일 오프셋으로 확정(지형을 많이 손봐서 DEM 비율 보정 무의미) |
| P6 IR / J2 에디터 | MCP로 처리: `M_PP_RCWS_IR` 틴트 회색화, `IA_DroneGimbalLook` 생성, `DA_TitanInputSchema` DroneGimbalLook 행 추가. WBP는 사용자 |
| 8000 포트 수신 지연 의심 | 구버전 rc_mock_client 테스트 결과였고 데모로는 정상 → 문제 없음으로 종결 |

---

## 1. S1 프로토콜 (`Network/UGVRemoteControlSubsystem`, 신규 자기방호 송신부)

| ID | 작업 | 원본 근거 | 진행 | 내용 |
|---|---|---|---|---|
| P1 | 키 오타 3건 | ICD 시트2 D4·D7·E24 | 바로 | `ConrtrolRight`→`ControlRight`(:833), `Batterry`→`Battery`(:836), `Latter`→`Letter`(`BuildDetectionObjectJson`의 `bUseLatterTypo` 제거). FireMode는 `BRUST`/`CONTINUS`와 정상 철자 둘 다 받게 |
| P2 | `UGV_RCWS_Status`에 3필드 | ICD 시트2 D11~D13 | 바로 | `Azimuth` = 현재 RCWS 방위값 그대로(`CurrentData.AzimuthDegrees`, 진북 0°), `Elevation` = ICD 문구대로 고각+90(0~180), `TotalBullet` = 현재 잔탄 |
| P3 | `Runtime` 추가 | ICD 시트2 D34 | 바로(기본안) | `UGV_Period_BasicInformation`에 `HH-MM-SS`, 프로세스 기동 기준 누적. Q3 답이 "초기화"면 재시작 훅에서 리셋 |
| P4 | 조향 감도 하향 | ICD 시트3 I7 | 바로 | 원격 경로만 `sign·|x/100|^1.5 × 0.6`, 지수·배율은 설정값 |
| P5 | ObjectClass 메모 | ICD 시트2 H44 메모 | 확정(작업 없음) | 현행 분류값 유지 |
| P6 | IR 흑백 | ICD 시트3 I12 메모 | 바로(기본안) | `M_PP_RCWS_IR` 녹색 틴트 제거(에디터 작업). Q4 답이 "열상 강조"면 별도 작업으로 재산정 |
| P7 | 해발 고도 헬퍼 | (K05.1·JSON 공용) | 바로 | 실제 지형 고도 자료(구글어스 등)로 기준점 몇 곳의 지표 해발을 구해 씬 Z→해발 오프셋 산출. `GeoCoordinateUtils`에 `GetAltitudeMslMeters()` = (씬Z/100)×`GetDistanceScaleFactor()`+오프셋. S2도 사용 |
| P8 | 드론 위치보고 K05.1 | 위치보고전문형식.xlsx, 메일 2-1 | 생성부 바로 / 전송부 Q1·Q2 대기 | 신규 `Network/SelfDefenseReportSubsystem`(자기방호축 프로세스에서만). 엑셀 정의대로 직렬화(위경도 정수 인코딩, course 8방위, 고도 ft·속도 km/h 소수 둘째 자리, 실시간 KST). 수신 IP/포트는 **로비 화면 입력**(기존 RTSP/UDP 설정과 같은 방식). 답 전엔 로그·파일 출력 + 로컬 수신 스텁 검증 |
| P9 | 로비 화면 송신 설정 | 사용자 결정 | 바로 | 축 선택 화면에 드론 보고 수신 IP/포트 입력란(K05.1·JSON 공용) |

수정 파일: `Network/UGVRemoteControlSubsystem.h/.cpp`, `Vehicles/RCWSComponent.h/.cpp`, `UI/GeoCoordinateUtils.h`, `Drone/DronePawn.h/.cpp`(`GetFlightVelocityMS()` 추가), 신규 `Network/SelfDefenseReportSubsystem.h/.cpp`, 로비(축 선택) 위젯, `M_PP_RCWS_IR`.

## 2. S2 드론 탐지 (`Drone/`, `Detection/`)

| ID | 작업 | 원본 근거 | 진행 | 내용 |
|---|---|---|---|---|
| D1 | 드론 RTSP 확인 | 메일 2-2 | 바로 | 이미 송출 중(`rtsp://<자기방호PC>:8554/selfdefense/uav_gimbal`). 실수신 확인 + 해상도 640×360 → **1024×576**(설정값). 레벨에 구 `BP_UAV`가 남아 마운트가 겹치는지 확인 |
| D2 | RCWS 박스 어긋남 수정 | ICD 시트2 J14 | 바로 | 송신 시점에 현재 카메라(+사격 흔들림)로 박스 재투영, 송신 주기 5Hz → 10~30Hz. UDP 박스를 RTSP 영상에 겹쳐 그리는 검증 도구로 확인 |
| D3 | 객체 분류 공용화 | (D5·P5 공용) | 바로 | `ClassifyDetectedObject`를 `Detection/DetectionClassification`으로 이동, enum 반환 + `ToIcdObjectClass()` / `ToDroneClassId()` 매핑 |
| D4 | 드론 JSON 생성부 | sample1_boxed_metadata.json, 메일 2-3 | 바로 | 신규 `Drone/DroneDetectionReporterComponent`. 짐벌 캡처 직전 같은 카메라로 박스 투영(1024×576 픽셀). **그 프레임에 보이는 대상만**. 고도는 P7 해발 헬퍼, 키 철자는 샘플 그대로. 샘플과 키 구조·타입 비교 검증 |
| D5 | 드론 JSON 전송부 | | Q1·Q2 대기 | 기본안 UDP·프레임당 1메시지. classId·라벨은 기존 UGV 객체 탐지 분류(D3)를 그대로 따름. 수신 주소는 P9 로비 설정 사용 |

수정 파일: `Drone/DronePawn.h/.cpp`, 신규 `Drone/DroneDetectionReporterComponent.h/.cpp`, 신규 `Detection/DetectionClassification.h/.cpp`, `Network/UGVRemoteControlSubsystem.cpp`(D2·D3), 해상도 설정(ini).

## 3. S3 조이스틱 (`titan_examplePlayerController`, 입력 에셋)

| ID | 작업 | 원본 근거 | 진행 | 내용 |
|---|---|---|---|---|
| J1 | 수동 조종 토글 재편 (사용자 결정) | 메일 5 | **코드 완료** | 토글 2개, 서로 상호배타, 둘 다 끄면 전자동. **모드 A**(Monitor2 토글): 트럭 RCWS + 드론 카메라 동시 수동 — H2 미니스틱(ABS_RX/RY) = RCWS, 메인 스틱 = 드론 짐벌, RCWS 버튼은 전부 트럭. **모드 B**(Monitor1 토글): 드론 전체 수동 — 메인 스틱 = 비행, hat = 짐벌(명세 외 기능이나 유지). "짐벌만 수동" 토글 폐지(A에 흡수), 드론 줌은 조이스틱 미배정, 자기방호축에서 축전환(Button_8) 무시. 데드존 0.1. 상세 `ui/2026-11-04_joystick_velocityone_h2_rcws.md` |
| J2 | 에디터 작업 | | 후순위 | `IA_DroneGimbalLook`(Axis2D) 생성(없으면 모드 A 짐벌 미동작), `DA_TitanInputSchema` "드론 카메라(메인 스틱)" 행 추가·CameraLook 표시명 변경, Monitor2 토글 라벨 변경·Monitor1 "짐벌만 수동" 체크박스 삭제 |
| J4 | Input 탭 조작부 중심 개편 (사용자 결정) | | **코드 완료** | 축·햇은 물리 조작부(메인 스틱 X/Y, H2 X/Y, 비틀기, 스로틀 레버, 햇 4방향) 단위로 한 번만 배정, 조작부↔동작 연결은 코드 고정(`Input/TitanInputPresentation.h`). 메인 스틱 한 줄이 모드 A 드론 카메라·모드 B 비행 IMC 양쪽에 같은 키·Negate로 들어감. RCWS 버튼은 기능별 배정 유지. 탭 상단에 모드 A/B 안내표. 저장 슬롯 `TitanInputBindings_v2`로 바꿔 기존 배정 초기화(키보드 기본값 없음). rowName·DA 에셋 불변 |
| J3 | 실장비 매핑 | velocityone_tester | 장비 필요 | 키 배정은 패키지 인게임 Input 탭에서 PC별로. Joystick Viewer로 축·버튼 번호(예측: RCWS Axis_3/4, 드론 카메라·사이클릭 Axis_0/1, 요 Axis_2, 스로틀은 Axis_3 금지→레버 Axis_5, 짐벌 Hat_0), 극성, 드리프트, B20 확인 후 버튼표 LIG 회신 |

수정 파일: `titan_examplePlayerController.cpp/.h`, `IMC_MouseLook`, `IMC_DroneManual`, `DA_TitanInputSchema`, `Config/DefaultInput.ini`, 조이스틱 가이드 문서.

## 4. S4 RCWS / 초기화

| ID | 작업 | 원본 근거 | 진행 | 내용 |
|---|---|---|---|---|
| R1 | 자동 재장전 [공통] | 메일 6-2 | 바로 | 잔탄 0 → 1.5초 뒤 최대 탄수로 보충(서버, 리플리케이션 자동 반영). 장전 상태 불변, 재시작 시 타이머 해제 |
| R2 | 최대 탄수 200 | ICD 시트2 D13 | 바로(B-1 이해 기준) | UGV·트럭 600 → 200. 600 기준으로 튜닝된 시나리오 의존 확인 |
| R3 | [UGV] 고각 제한 | 메일 6-1 | 바로(범위는 디자인팀 질문에서 확인) | 사격 중 총구 탄도 회전에도 차체 기준 제한 적용. 범위는 답 오면 설정값만 변경 |
| R4 | 초기화 시간 회신 | 메일 3 | 바로 | 코드 수정 없음. 구간별 실측표(질문 문서 B절) 회신. 필요하면 구간 로그 3줄 추가 |

수정 파일: `Vehicles/RCWSComponent.h/.cpp`, `Vehicles/RCWSFireControlComponent.cpp`, `UGVPawn.cpp`/`TitanTruck.cpp`(AmmoMax).

## 5. S5 UI·카메라 (보류 — 디자이너 협의 후)

| ID | 작업 | 원본 근거 |
|---|---|---|
| U1 | 패널 테두리 `#363636` [공통] | 메일 1 |
| U2 | 로고 LIG D&A → LIG [공통] | 메일 1 |
| U3 | [자기방호] 쿼터뷰 자리에 드론 영상 패널, 기존 드론 패널 삭제 후 상태정보 패널 확장 (패널은 16:9 유지 — RTSP 영상 비율) | 메일 1 |
| U4 | [자기방호] "UAV" → "자기방호 드론" (표시 문자열만) | 메일 1 |
| U5 | [공통] RCWS 카메라 위치 앞으로 | 메일 4 |
| U6 | [UGV] 주행카메라(전/후/좌/우) 높이 올리기 | 메일 4 |

---

## 6. 세션 간 순서·접점

1. **`UGVRemoteControlSubsystem.cpp`**: S2(D2 박스 재투영·D3 분류 이동) 먼저 → S1(P1~P4) 그 위에. S1 변경은 함수 몇 줄 단위라 충돌 작음.
2. **해발 헬퍼(P7)**: S1이 만들고 S2(D4)가 사용. S2는 그 전까지 임시 오프셋으로 진행.
3. **분류 공용화(D3)**: S2가 만들고 S1(P5)이 사용.
4. **최대 탄수(R2) ↔ TotalBullet(P2)**: S4가 값 변경, S1은 현재 잔탄만 읽으므로 순서 무관.
5. **로비 송신 설정(P9)**: S1이 만들고 P8·D5가 공용 사용.

## 7. 공통 규칙

- 소스 수정은 Edit/Write 툴로만, Perforce 체크아웃 후 편집(`-c insung_laptop1 -C utf8`). 읽기 전용 파일이면 사용자에게 체크아웃 요청.
- 빌드는 사용자가 한다(Live Coding 세션과 충돌).
- 완료 후 해당 시스템 폴더에 `YYYY-MM-DD_주제.md` devlog, 동작이 바뀌면 `guide/` 갱신. ICD 정본 `protocol/lig_icd_ugv_rc_full.md`·`protocol_icd.md`도 갱신.
