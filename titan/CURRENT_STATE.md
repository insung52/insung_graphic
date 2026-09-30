# Titan (KADEX 전시회) — 현재 프로젝트 상태

2026-09-30 / 진행중 / 전체 시스템별 현재 상태 스냅샷 — 문서 정리 1차 작업의 산출물.
(최초 작성 2026-09-01, 이후 항목별로 날짜를 붙여 갱신 중.)

이 문서는 "지금 뭐가 어디까지 되어 있는가"만 다룬다. 문서 자체의 목록(날짜/위치)은
`DOCS_INDEX.md`, 신규 문서 작성 규칙은 `CLAUDE.md` 참고. 오래되면 이 문서도 다시 갱신 필요 —
아래 각 항목이 가리키는 원본 문서의 날짜를 보고 신뢰도를 판단할 것.

---

## 1. UGV↔원격통제기(LIG) 프로토콜 — 거의 완료

UDP+JSON, LIG 정식 ICD(`protocol/lig_icd_ugv_rc_full.md`) 기준 구현 완료
(`protocol/ugv_rc_feature_gap_analysis.md`). **2026-08-28 LIG 1차 답변**
(`documents/response_0828.md`)으로 대부분의 불확실했던 부분 해소, `RC_ActivateMovement`
오매핑(차량 시동으로 잘못 구현했던 것 → RCWS 조향 게이트로 정정) 2026-08-31 재작업 완료.
남은 것: 자체방호축 PC 고정 IP, LIG에 보낼 후속 질문 3건(§12). 상세: `protocol/protocol_icd.md`
§3, `protocol/lig_questions_0816.md`.

## 2. 자체방호(이동형지휘소)축 로컬 통합 — 거의 완료

조이스틱 입력 → RCWS/UAV 짐벌 배선 완료(EO/IR, 발사모드, 장전, 안전/암, 축전환, 조준 브레이크
전부 연결·실측). 상세: `protocol/selfdefense_rc_feature_gap_analysis.md`.

## 3. RTSP 영상 송출 — 완료

UGV 5스트림 + 자체방호 7스트림(부가 1개 포함) 전부 실 카메라 연결, mount 확정
(`protocol/protocol_icd.md` §3.3/§4.1). 종단 지연 441~484ms → **68ms**로 최적화(수신측
GStreamer+NVDEC). 전송은 TCP interleaved 권장(2026-09-15 정정: UDP도 됨 — 코드에 프로토콜 제한
호출 없음, gst-rtsp-server 기본값; TCP는 저지연 검증 기준이고 UDP는 RTP 포트가 동적 협상이라 방화벽
환경에선 TCP). Linux 패키지 빌드 풀스크린 프레임 폭락
(11fps) 원인 규명·해결(Wayland/Xwayland 이슈). RTSP 스트림에 SSR/피격흔들림 안 나오던 문제
(SceneCapture가 메인 뷰포트와 다른 카메라라 `ReflectionMethod=None` 강제되던 것) 원인 규명,
해상도 커스터마이징+CCTV 잘림버그+RCWS 이중렌더링도 해결됨(2026-08-20,
`camera_pipeline/rtsp_postprocess_parity_0820.md`/`rtsp_resolution_customization_0820.md`).
상세: `rtsp/`, `camera_pipeline/`. **순수 Xorg 세션 검증 완료(2026-09-04)** — Wayland/Xorg 양쪽에서
시뮬레이터 실행 + RTSP 수신 확인, 세션 자동 판별 런처(`run_titan_example.sh`) 신설
(`packaging/2026-09-02_linux_package_ugv_host_rc_test_guide.md` §7-1). **[2026-09-15] 그 자동 판별을
`Config/BootstrapPreamble.sh`로 옮겨 `titan_example.sh`에 패키징마다 자동 삽입**(UAT
`StageBootstrapExecutable` 훅) — 래퍼 스크립트 복사 절차 폐지, 받는 쪽은 `./titan_example.sh`만 실행.
배포용 실행 가이드는 `packaging/kadex_0915_패키징_실행가이드.md`(받는 쪽 절차만, 0902판 폐기), 패키징
절차는 내부 가이드 §2가 유일. 남은 것: 자체방호축 6스트림
정밀 지연 재측정. **[2026-09-15] NVENC SDK 13.1.15 → 13.0.37로 내림** — 09-02 LIG 전달 패키지가
LIG PC(드라이버 595.84, 업데이트 거부)에서 인코더 초기화 실패로 RTSP 불통이었음(13.1 = 드라이버
610+, 13.0 = 570+). 함께 `RtspStreamComponent`를 인코더 성공 후에만 마운트 등록하도록 바꿔
인코더 실패 시 20초 타임아웃 대신 즉시 404. 사내 리눅스 PC를 595.84로 내려 정상 동작 실증.
**LIG 재발송 대기.** (`rtsp/2026-09-15_lig_rtsp_describe_timeout_analysis.md`) **[2026-08-31 원인 확정+코드 수정 완료]** 자체방호축에서 RCWS
조준 이동 시 전장카메라/CCTV가 떨리는 버그, UGV 발사 반동이 자체방호축 카메라에도 리플리케이션
되는 버그(2-PC 환경) — `SceneCaptureViewParity`/`RCWSProjectile` 수정 완료
(`rcws/2026-08-31_selfdefense_camera_shake_bugs.md`), **2-PC 실환경 검증만 남음**.
**[2026-09-29 완료]** 환경카메라(`selfdefense/env_camera`)에서 원거리 트럭 환기구가 ~1초 주기로 번쩍이던 것 — TSR 지터가 원인,
`BattlefieldCapture` 뷰에서만 지터를 끄고 TSR 누적은 유지(PIE 검증 완료). CCTV/드론 짐벌/RCWS 조준경 확장은 보류
(`camera_pipeline/2026-09-29_battlefield_capture_tsr_jitter_blink.md`).

## 4. 멀티플레이(리슨서버) 리플리케이션 — 거의 완료

기반 플러밍, RCWS(투사체 판정 서버 권위화 포함), 아군/적군 전투, UGV 구동, UAV까지 리플리케이션
완료 + 실기 테스트 통과. 상세: `replication/replication_audit.md` §0-1/§8.

## 5. 인게임 Settings 위젯 — Input 완료, Graphics 동작 확인(게임 레벨 검증만 남음)

Input 탭 완료(`ui/ingame_settings_input_system.md`).

**Graphics 탭 — 2026-09-03~10 작업. 빌드·WBP까지 완료하고 실제 동작 확인됨**(`Graphics 탭 23개 항목
생성` 로그, 헤더 5 + 항목 18). 상세는
`ui/2026-09-10_graphics_settings_implementation.md`(구현·WBP 계약·검증 상태),
설계 근거는 `ui/graphics_settings_analysis.md`(조사).

핵심은 **런타임 품질 변경을 처음으로 가능하게 만든 것**이다. 그 전까지는 `sg.*` 12개가
`WindowsEngine.ini`의 `[ConsoleVariables]`(`SetBySystemSettingsIni`)에 박혀 있어서
`Scalability::SetQualityLevels`(`SetByScalability`)가 조용히 거부됐고, 품질 변경이 **완전 no-op**
이었다(`DumpCVars` 전수 실측으로 확정). 이관 후 `sg.*`가 전부 `Scalability`로 풀리고 거부 경고가
12줄 → 2줄로 줄었다(남은 2줄은 VRAM 안전장치라 의도적 고정).

- **구조**: 신규 `UTitanGraphicsSettings`(`UDeveloperSettings`, `Config/DefaultGame.ini`)가 단일
  소스. `defaultconfig`라 P4로 공유되고 패키징에 포함되며, 에디터·게임 양쪽이 같은 파일을 읽어
  **"에디터는 A인데 패키지는 B"가 구조적으로 불가능**해진다(엔진 기본은 서로 다른 ini를 읽어 갈라짐).
  개별 Lumen/VSM 튜닝은 `Config/DefaultScalability.ini`의 프리셋 재정의로 이관.
- **플랫폼 불일치 2건 해소** — 반사 방식이 Windows=SSR / Linux=Lumen으로 갈려 있던 것을 SSR로 통일,
  Lumen 원거리 GI 튜닝이 Windows 전용 ini라 **Linux 납품 빌드에 아예 빠져 있던 것**을 공통화.
  → **Linux 패키지 룩 실측 필요**(둘 다 그쪽이 바뀜).
- **항목**: 헤더 5 + 항목 18. 품질 11축 + 반사/AA 방식 + VSync + **나무 WPO 거리·LOD 배율**(실측
  곡선 기반 숫자 입력) + 카메라 캡쳐 주기 3종. `sg.FoliageQuality`는 이 레벨에서 효과가 0이라
  제거(나무가 이미 구워진 ISM이라 `pcg.Quality`가 손댈 대상이 아님).
- **제외**: 프레임 상한(물리 결정성 대책), 창 모드(운용이 `-fullscreen` 런치 인자), 레벨 PPV.
  **보류**: 렌더 스케일·캡쳐 해상도(축/RTSP와 얽힘 — 캡쳐 해상도는 축 선택 화면 값과 실제로 겹침).
- **게임 레벨 대상 탐색 확인 완료** — `New_kadex_0811`에서 `QuadCam 2개 / Drone 1개 / Truck 1개`,
  `ISM/HISM 50개(인스턴스 94936)`. **50개의 소유 액터를 전수 확인해 전부 숲(`BP_SplineForest_*` 10개
  + `TreeCollisionProxyBuilder`)임을 검증**했다 — 옛 문서의 `17개/58,400`은 plant 액터가 2→8개로
  늘기 전 숫자였다(구현 문서 §8-1).
- **남은 것**: VSync는 패키지에서만 검증 가능, Linux 패키지 룩 확인, 나무 WPO/LOD 값 변경이
  실측 fps 곡선대로 움직이는지 확인.

## 6. 레벨 디자인 / UGV 자율주행 — 2026-08-21~27 집중 작업, 대부분 완료

신규 레벨 `New_kadex_0811`(PCG 숲) 내비메시 인프라 구축, 성능 폭락 수정(2.3→31fps), 3단계
전투 시나리오 구현. UGV 자율주행: 커브 선행 감속, 궤도 잠금(오래된 "공중에 뜬 바퀴" 버그
해결), 장애물 회피(4가지 원인 규명, 실측 34.6km/h·조향포화 0회) 전부 완료. 상세:
`level_new_kadex_0811/`, `vehicle/ugv/` 폴더(파일별 날짜는 `DOCS_INDEX.md` 참고).

**2026-08-26 — 레벨이 현실 1:1 이 됐다(09-22 사후 문서화 — 이 사실을 몰랐던 세션이 있었음).** `New_kadex_0811` 은
랜드스케이프 대각 코너 위경도 기준으로 현실보다 26.2% 크게 지어져 있었고, 월드 원점 기준 k=0.7921933250 으로 전부(랜드스케이프
loc/scale, 액터 328 위치, 볼륨, 도로 스플라인, PCG 숲 스플라인) 균일 축소했다 — 소품 자체 스케일은 불변. 큐브 독립 검증 0.001 m.
`GeoCoordinateUtils.h` 보정점은 옛 레벨 눈대중 5점 → **랜드스케이프 코너 2점(정확값)**, fit `Scale≈1.0006 · 회전 -86.23° · 씬 +X≈남`.
**`GetDistanceScaleFactor()` 가 1.2135 → 1.0006 이 되어 UAV/UGV 물리 스펙 변환 경로의 씬 속도가 21% 빨라지고 표시값은 17.6%
줄었다(둘 다 옳음)** — 시나리오 타이밍/커브 감속 튜닝 재점검은 미실시. PCG 숲 점박이 마스크는 Spatial Noise Transform.scale=1/k 로 복원.
**미해결: 랜드스케이프 동쪽 변이 미니맵 이미지 밖 205~340 m**(§11). 상세 `level_new_kadex_0811/2026-08-26_level_rescale_to_real_world.md`,
현재 변환 레퍼런스 `guide/real2world.md`.

**2026-09-02~03 — UGV 차량 자체가 교체됐다.** 궤도 16륜 `BP_UGV_Vehicle_new` → 6×6 차륜
`BP_UGV_0901`(신규 모델·리깅·머티리얼·주행 튜닝 전부 실동작 확인). 이어서 09-03에 그 BP의
로직을 전부 C++ `AUGV0901Pawn`으로 내리고 죽은 노드 427개·변수 39개를 제거해
`.uasset`을 1,150KB → 67KB로 줄였다 — BP는 이제 컴포넌트와 튜닝 값만 든 데이터 에셋이다.
**UGV를 코드에서 찾을 때는 `BP_TestPlayerController.UGVVehicleClass`가 유일한 진입점**이고
지금 `BP_UGV_0901_C`를 가리킨다. 구형 `BP_UGV_Vehicle_new`는 폴백으로 남아 있다.

**2026-09-10 — 서스펜션 승차감 재튜닝.** 레벨의 작은 바위에 콜리전을 켜서 덜컹거림이 생기게 한
뒤, 09-02의 값(휠 개수 스케일 규칙으로 기계 환산한 것 — 승차감을 본 적 없음)을 승차감 기준으로
다시 잡았다: `SpringRate 900 → 200`, `SuspensionMaxRaise 12 → 16`, `SuspensionSmoothing 0 → 5`.
같은 작업에서 **Chaos 5.8이 서스펜션을 힘이 아니라 PBD 컨스트레인트로 푼다**는 구조와,
**`SpringPreload`·`RollbarScaling`이 엔진에서 동작하지 않는 죽은 값**이라는 사실이 확인됐다.
서스펜션 값을 만지기 전에 `vehicle/ugv/2026-09-10_ugv_0901_suspension_tuning.md`를 볼 것.
후속 후보는 같은 문서 §6(댐핑비 인하, 휠 클래스 3분할).

**2026-09-23 — 서스펜션이 시체를 지면으로 읽지 않는다(UGV 생성자 한 줄).** `UUGVWheeledVehicleMovementComponent` 생성자에
`WheelTraceCollisionResponses.SetResponse(ECC_PhysicsBody, ECR_Ignore)`. Chaos 서스펜션 트레이스의 엔진 기본 응답이 **"차량만 Ignore,
나머지 전부 Block"** 이라 래그돌 시체(`ECC_PhysicsBody`+`QueryAndPhysics`)와 떨군 총이 바퀴에 땅으로 보였고, 바퀴 하나가 그걸 밟으면
스프링 힘과 접지 마찰이 그 바퀴만 튀어 **차체가 들리는 경로**가 열린다(뒤집힘 목격 0회 — 예방). **물리 접촉은 안 건드렸다** — 차가
시체를 밀어내는 그림은 그대로다. ⚠ 헤더 상단의 "프로퍼티도 동작도 추가하지 않는다" 주석이 이제 사실이 아니다([W119]). 상세와 병사 쪽
무기 드롭은 `soldier_ai_lab/ai/2026-09-23_corpse_vehicle_interaction_and_weapon_drop.md`(원칙 P192).

**2026-09-15~16 — 데모 모드 UGV RCWS 자동사격(탐색 스윕) 시작 시점**을 레벨 시작 → 1차 목적지
도착(`UGVArriveZone1` 행, `SetDemoUGVAutoFire` 이펙트)으로 옮김, 09-16 PIE 확인 완료. 같은 날
탐색 스윕 고각도 월드 수평 → 차체 기준 -3°(`SearchSweepElevationDegrees`)로 바꿔 내리막에서 하늘
보던 문제 해결, UGV/트럭 확인 완료(`level_new_kadex_0811/2026-09-15_demo_ugv_autofire_on_zone1_arrival.md`,
`rcws/2026-09-15_search_sweep_hull_relative_elevation.md`).

**2026-09-21 — New_kadex_0811 의 적군/아군이 SoldierLab 병사로 이관됐다.** 구 `BP_Enemy_kadex_1~15`/`BP_Ally_kadex_1~25` 삭제 →
`BP_Soldier_Hostile_1~15`(분대 1/2/3)/`BP_Soldier_Friendly_1~25`(1~5) 같은 트랜스폼, 마커 113·경로 스플라인·드론 경로 유지, 분대별
`ASoldierZone` 8개 + `ScenarioConfig_1.SquadZones`, DT **`DT_ScenarioSteps_ThreeStage_SoldierLab`** 26행(적/아군 행은 `IssueSquadOrder`,
드론·UGV·트럭 행 그대로). 첫 PIE 에서 3단계 체인이 순서대로 완주("아주 잘됨"); 3분대 3차 도주가 엄폐 홉이던 문제는 `BreakContact` 동사로
(§7). 구 `EnemyCombatComponent` 시나리오 기제는 이 레벨에서 죽은 코드(`kadex_test` 에만 유효). 상세:
`level_new_kadex_0811/2026-09-21_soldierlab_migration_new_kadex_0811.md` · 저작 레퍼런스 `scenario_authoring_guide.md` 2.6절.

**2026-09-22 — 시나리오 재시작(확인창 + 자동 재시작) 구현 완료, `L_SoldierScenario` PIE "완벽".** 적 전멸 5 s 뒤 확인창
(`WBP_RestartPrompt`, [예]/[아니요]/☐ 자동 재시작 — 체크 시 10 s 카운트다운, 체크값은 ini `[Scenario] bAutoRestart` + `-autorestart` 로 유지).
**레벨 리로드가 아니라 인플레이스 리셋**: 병사는 `UScenarioRespawnSubsystem` 이 월드 시작 스냅샷(클래스/트랜스폼/저작 델타)으로
Destroy → 재스폰(딸린 소총 액터 동반 파괴, `AutoPossessAI=PlacedInWorldOrSpawned` 로 AI 빙의), UGV/트럭/드론은 `IScenarioResettable::
ResetForScenarioRestart` 로 제자리 부활(RCWS 탄약/모드/표적 리셋, 드론 `Flight->ResetTo`), SoldierLab 서브시스템은 그 모듈의 리셋 계약
(`ResetForRestart` ×2 · `RecallAll`). 유일 진입점 `UScenarioStateSubsystem::RequestScenarioRestart()`, 콘솔 `titan.ScenarioRestart` /
`titan.ScenarioAutoRestart`. 확인창은 **자체방호 화면 우선**(UGV 호스트+자체방호 클라 → 자체방호에만, 단독이면 그 축), 클라 입력은 PC Server RPC.
DT 행 `ScenarioRestartPrompt`(이펙트 `ShowRestartPrompt`)는 `DT_ScenarioSteps_ThreeStage_SoldierLab`·`DT_ScenarioSteps_SquadThreeStage` 둘 다에.
**남은 것(2026-09-23 갱신)**: **빌드 대기 2건**(드론 짐벌 배율 + **상태 패널 누적값**) 이 들어간 **빌드로 2회차 확인 — 배터리 100 % · 비행시간 00:00 · 주행거리 0 · 짐벌 화각 1회차와 동일**(아직 빌드 전) ·
10사이클 메모리/fps · 장시간 무인 반복 · **P4 제출 순서 조율**(일부 파일은 다른 세션이 동시 편집 중).
낙하산 `RespawnActors` 연결은 **불필요로 종결**(2026-09-23 사용자 확정 — 정적 액터라 초기화할 것이 없다. 프로퍼티는 병사 아닌 재스폰 대상이 생길 때를 위해 남김). 상세:
`level_new_kadex_0811/2026-09-22_scenario_restart_implementation.md` · 저작 `scenario_authoring_guide.md` 2.7절.

**2026-09-23 — New_kadex_0811 2-PC 검증 완료("이제 잘됨"). 최종 원인은 레벨 GameMode 오버라이드였다.** 클라이언트만 페이드가 안 되고 재시작 뒤
드론이 원위치로 안 돌아온 이유는 **World Settings GameMode 오버라이드가 `GM_SoldierLab` 로 바뀌어 있던 것**(09-21 SoldierLab 이관 때 들어옴).
그 GM 은 순정 `AGameModeBase` 파생이라 `Atitan_exampleGameState`(→ 재시작 멀티캐스트가 전부 **서버-로컬 폴백**)도, `Atitan_examplePlayerController`
(→ 축/데모 플래그 미결정 → 드론이 **서버·클라 둘 다 시뮬 주체**)도 없다. **단일 프로세스에서는 폴백이 정답이라 증상이 안 난다 — 2-PC 에서만 깨진다.**
해결 = 오버라이드를 **`BP_KadexTestGameMode`**(부모 `Atitan_exampleGameMode`)로 복구. **운용 규칙: 2-PC·전시 구성은 항상 titan 계열 GM,
`GM_SoldierLab` 은 병사 거동 자유 관전용 단일 프로세스 전용**(`kadex_test` = `BP_KadexTestGameMode`, `kadex_lobby` = `BP_TestGameMode`).
같이 들어간 것: GameState **멀티캐스트 3종**(`Multicast_ScenarioRestartBegin/Apply/End` → 각 프로세스가 `RunLocalRestart*` — 페이드·로컬 정리·자기가
시뮬하는 `IScenarioResettable` 리셋) · **드론 리셋 권한 분리**(서버 = 복제되는 명령, 시뮬 주체 = 물리·짐벌·`bParachuteObserved`; 차량은 `HasAuthority`
게이트) · **소총 액터 누수 근본 수정**(SoldierLab `USoldierHealthComponent::EndPlay` + `bDestroyCarriedActorsOnDestroy` — 평상시 사망 경로에도 있던
누수, [W116] 해결) · 재스폰 스냅샷의 에디터 임시 컴포넌트 제외. 재시작 시 GameState 가 titan 것이 아니면 경고 로그를 남긴다.
**같은 날 후속**: 드론 **짐벌 배율(`ZoomLevel`)** 도 리셋 대상으로 추가(저작값이지만 자동 정찰/교전 프레이밍이 런타임에 바꾼다 → `InitialZoomLevel` 스냅샷 →
시뮬 주체가 `SetZoomLevel` 로 복원). ⚠ **아직 빌드 전 — 다음 빌드 반영 예정이므로 "검증 완료"가 아니다.** 드론·UGV 컨트롤러 런타임 상태 전수 대조 결과 그 외 누락은 없고,
안 되돌리는 것들(진단 누적값 · 복제 미러 `Rep*` · BeginPlay 1회 세팅 · `SnapStart*`)은 의도적이다. 드론 `ViewMode` 는 **로컬 카메라 모드라 리셋하지 않는다**(결정).
낙하산 재스폰은 **폐기**(정적 액터). 전체 동작은 사용자 판정 "일단 지금 다 잘 작동하는 상태임".
**같은 날 후속 2 — 상태 패널 누적값도 리셋 대상**(사용자 지적: 재시작해도 드론 배터리·비행시간이 안 줄어든 채 이어짐, UGV 는 배터리 + 누적 주행거리). 두 컴포넌트에
`ResetForScenarioRestart()` 신설(`UStatusHUDComponent` = 드론·트럭 / `UUGVStatusComponent` = UGV) + `AUGVAIController::TankTotalDistanceTraveledCm=0`. **★ 호출 위치가 정반대다** —
드론 패널은 `CurrentData` 가 **비복제 + 틱 권한 게이트 없음**이라 **모든 프로세스**가 시뮬 주체 판정보다 **먼저** 되돌리고, UGV 패널은 `CurrentData` 가 **복제 + 서버에서만 생성**이라
**서버에서만**(`HasAuthority` 게이트 안) 되돌린다. 트럭은 드론과 같은 컴포넌트지만 서버가 시뮬하므로 지금은 게이트 안(클라 트럭 패널을 쓰게 되면 밖으로). RCWS 탄약은 이미 되돌려지고
있었다(확인 완료). 같은 계열 전수 검색에서 추가 누락 없음. ⚠ **이것도 빌드 전.**
상세: 같은 문서 §5(끝) · `replication/replication_audit.md` §9 · 드론 규약 `vehicle/drone/drone_flight_dev_guide.md` 15.2절 · `guide/ui_dev_guide.md`.

## 7. 적군/아군 AI·애니메이션·전투 — 2026-08-24~25 대규모 개편 완료, 분대 재편 검증 대기

로코모션 버그(시간 2배 흐름) 해결, 피격 리액션을 감쇠조화진동자 물리로 전면 교체, 3단계
전투 확장(Part A~G) 전부 완료(액티브 랙돌 사망, 전투지 3세트, 도주+타겟전환 캐스케이드).
**2026-08-31**: 적 15명을 5명씩 3분대로 재편(분대별 도주 경로/NavMesh 필터) — **PIE
재검증 대기**(`ai_combat/2026-08-31_enemy_squad_reorg.md`). 애니메이션 애셋 전수 목록(디자인팀
공유용)도 정리됨(`ai_combat/2026-09-01_animation_asset_inventory.md`). 상세: `ai_combat/` 폴더.

**2026-09-17 — SoldierLab(고사실감 병사 AI, `Source/SoldierLab/`, 문서는 `soldier_ai_lab/`)**: 적·아군 전원을
SoldierLab 병사로 교체하기로 결정(09-17 오후, L0/L1 분대 명령 층 코드 완료·빌드 대기). 같은 날 저녁 **위험
지도를 폐기하고 상황 필드(조명 모델)**로 교체 — 위험도를 저장하지 않고 목격(점광원)+그림자+안 본 땅(앰비언트)
으로 파생, 잠입("안 본 곳은 적이 있다고 친다") 거동 추가. ~~1·2단계 PIE 확인, **3단계(앰비언트·필드 후보) 빌드
대기**.~~ 값은 Project Settings → Game → SoldierLab Situation Field, 전부 미측정. 상세: `soldier_ai_lab/CURRENT_STATE.md`
· `soldier_ai_lab/ai/2026-09-17_situation_field_lighting_model.md`.

**2026-09-18 — 상황 필드 2일차**: 3단계(앰비언트) PIE 확인("딱 내가 원하는 그림"). 큰 맵 후퇴전에서 필드 메모리가
지나간 면적만큼 쌓이던 것을 **밉(거친 레벨 = 레벨 0 집계) + 다중 앵커 퇴거**(병사 아무나에게서 80 m × 4^L 밖
디테일을 부모에 잔여물로 접고 해제)로 상한 — 계산은 레벨 크기와 무관하고 메모리만 문제였다는 분석에서.
오버레이 v2(자체 배처로 깜빡임/fps 해결, 클립맵 링, 불투명도 = 신선도) · 라이트 부정 증거(빈 채로 보고 있으면
4 s 반감기 추가, 삭제 아님). LOD 링·배처까지 PIE 확인, ~~**대칭 캡·라이트 시각 v2·부정 증거·헤더 2줄은 빌드
대기**~~ → 오후 빌드에 포함. 같은 시스템 문서 16~18절 · 원칙 P152~P157.

**2026-09-18 오후~저녁 — 순찰 · 부채꼴 스캔 · 이동 강건성 · 얇은 엄폐**: "적이 죽은 뒤 아군이 가만히 서 있고 볼 곳
화살표가 몸과 반대"의 원인 넷을 고쳤다 — 명령의 섹터를 고정 방위가 아니라 **부채꼴**로(필드가 그 안에서 가장 안 훑은
방위), 콘 스윕 띠 넓히기, 볼 곳은 항상 계산(쓸지는 교전 층), 교전 층이 `GetAimPoint()/IsScanning()`을 발행해 포즈 세션의
`SoldierScanTurnComponent`가 총 내린 idle의 몸을 돌린다. **순찰**은 경로가 아니라 **낡은 조망에 대한 비용**(존·목표
`PatrolWeight 1.0`, 눈 0일 때만). 로그로 잡은 이동 결함 셋(큐브 꼭대기 셀 → solid 셀 + `MoveTo` 전 경로 검사 + 30 s 거부 +
0.75 s 유예 / 벽 꼭대기 → 미지 = 열림 / 밴드 밖 평평한 Hold → 기울기 계속). CQB "눈이 발을 이끈다". `MinStance`(titan DT
미연결). **오후 묶음 빌드·PIE "이제 정상적이다"**; 저녁 얇은 엄폐 A/B/C(활동도 가중 은폐 · 미세 위치 · 코너 멈춤)는
~~**빌드 전**~~ → 밤 빌드·PIE "잘됨". 실제 레벨의 나무가 Sight 채널을 막는지 확인 필요. 상세: `soldier_ai_lab/ai/2026-09-18_patrol_scan_and_move_robustness.md`
· 원칙 P158~P166.

**2026-09-18 밤 — 4차: 코너 멈춤 루프 · 긴장도/걸음 · 포즈 급박도**: A/B/C 첫 빌드 로그가 스폰 옆 굽이에서 0.82 s마다
`MOVE`를 재발행하는 루프를 드러냄(굽이 기억이 경로 인덱스 + 재개 뒤 유예 만료) → 굽이는 **자리**로 기억, 재개된 이동은 방금
발행한 이동, 눈이 나타나면 즉시 재개. 조용한 병사가 어디를 가든 조깅하던 것 → **긴장도**(알람 뒤 반감 20 s)와 **걸음**
`GetDesiredGait()` Walk/Jog/Sprint(조용한 경비는 걷는다, Cautious 무접촉은 Walk) → 포즈 세션 GaitBridge가 GASP `WantsToWalk`로.
자세 전환이 기계적 → **AI는 목표 + 급박도 숫자 하나(`GetPoseUrgency()`, `Urgency*` 7값)만 내고 움직임은 포즈 층**(결정은 이산,
움직임은 연속). 전부 PIE "잘됨". 잠입 접근의 개인 층은 완성 — 남은 것은 titan 쪽(`MinStance` DT · 나무 Sight 콜리전 · 내비
가중 경로). 상세: 같은 문서 **12~17절** · 원칙 P172~P175.

**2026-09-18 밤 ~ 09-21 — 분대 스코프 필드 · 엣지 전진 · 사격 콘 · 섀도우 감축**: 상황 필드가 진영 하나가 아니라 **분대 하나당
하나**(`MaxSquadsPerFaction 3`; 남의 분대 목격은 무전이 닿았을 때 관측 시각으로만; UGV 의 표적용 Identity 는 병사가 아니라 스코프·앵커·
관찰 대상에서 제외 — 오버레이가 UGV 를 따라가던 원인; cvar `Debug.Field.Squad`/`.Centre`; `L_SoldierScenario` 아군 4 → 3분대). 09-18 코너
멈춤을 **엣지 전진**으로 대체 — 콘 스윕 광선의 엣지 + 필드 쐐기 적분(트레이스 0)으로 한 걸음이 여는 경계도 × m² 를 예산과 비교해 걸음을
고르고 연 조각이 눈에 익으면 다음: 호도 타이머도 없이 파이 자르기가 **창발**한다(빌드됨, PIE 판정 대기). 사격 콘을 무기 고정 3° 에서 **AI 가
소유하는 콘**(정착 0.8° × 이동 × 자세 × 반동 + 선회/반동/이동 흔들림)으로 — 새 의도 `Settling`(정착하면 맞는다, 기다림)·`Pacing`(버스트
2~5발 사이), PIE ✅; **무기 BP 가 아직 이 콘을 안 읽고 캐릭터 BP 가 `HasContact()` 를 aim 모드에 먹인다**(BP 배선 대기). 섀도우 재캐스트를
한 셀·0.5 s 문턱 + 같은 자리 라이트 동승으로 줄이는 코드는 ~~빌드 전~~ **같은 날 빌드·PIE ✅**(재측정 0.03 ms · 0 waiting · 96 alive — 동일);
비용 줄 실측에서 라이트가 `MaxLights 16` 상한에 붙어 있음이 드러남(96 은 alive 지 waiting 이 아니다).
상세: `soldier_ai_lab/ai/2026-09-21_per_squad_field_edge_advance_fire_model.md` · 원칙 P176~P180.

**2026-09-21 늦게 — 디버그 오버레이 노출 보정**: 노출이 EV10 에 고정된 사실적 라이팅 레벨에서 SoldierLab 의 모든 AI 오버레이(와 엔진
내비메시 `P` 뷰)가 **숯검정**으로 나왔다 — 디버그 프리미티브는 톤매퍼 **앞**에서 씬과 같이 노출되는데 `DrawDebug*` 는 8-bit `FColor` 라
선형 1.0 위로 못 올린다(titan 본체의 nit 기준 VFX 문제와 같은 뿌리). 레벨 라이팅/PP 는 건드리지 않고 **뷰가 지난 프레임에 적용한 노출의
역수만큼 밝게** 그린다: `SoldierDebug::GetExposureScale`(씬 뷰 익스텐션이 `GetLastEyeAdaptationExposure()` 를 읽음) · `Bright(FColor) → FLinearColor` ·
래퍼 `SoldierDebug::Line/Point/Sphere/Circle` · 신규 `USoldierDebugMeshComponent`(라인 배처의 메시 경로를 선형 색으로 — 필드 사각형·라이트 링) ·
cvar `SoldierLab.Debug.ExposureScale`(0 자동). `AI/` 24곳 교체, PIE ✅. `Squad/`·`Pose/`·`Weapons/` 17곳은 소유 세션 몫, 엔진 `P` 뷰는 비목표.
상세: `soldier_ai_lab/ai/2026-09-21_debug_overlay_exposure.md` · 원칙 P181.

**2026-09-21 — 성능 계측 + 엄폐 틱 비용**: 빈 레벨 World Tick 1.7 ms 가 병사 35명(`L_SoldierScenario`)에서 **29 ms**(fps 20~30)인데 `stat game` 은 6 ms 만 이름을
댔다 → **`stat SoldierLab`**(`STATGROUP_SoldierLab` — 시스템 틱마다 사이클 카운터 + `Traces:` 카운터)과 **`SoldierLab.<System>.Enabled` off 스위치 7** · `SoldierLab.Cover.Avoidance`
(RVO A/B)를 먼저 심고, 계측이 이름을 댄 **엄폐 틱 8.92 ms(프레임의 1/3, 487 트레이스/프레임)**만 고쳤다 — 전부 결정 보존(이동 중 스윕 정지 · 발밑 0.1 s 박자 · 경로 가지치기 ·
눈 0 후보 2/틱) → **1.92 ms · 243 트레이스, World Tick 28.5 → 22.6 ms**(프레임 31, GPU 6 — 게임 스레드 바운드). RVO 는 0.3~0.5 ms 라 켜 둠; A/B 중 잡힌 **`DispatchBlockingHit` 58 ms 히치**는
무기 `OnHit` 의 동기 에셋 로드 의심(별건). **남은 게임 스레드는 SoldierLab 이 아니다** — 애니메이션 게임 스레드 ≈ 7.3(ABP 이벤트 그래프 3.5 · 키네마틱 본 1.0 → 포즈) · 이동/트랜스폼 4~7
(병사당 부착 컴포넌트 ≈ 23 · 오버랩 → 캐릭터 BP) · 캐릭터 BP 틱 2.5 · 투사체 스폰 0.65(→ 무기 풀링); SoldierLab 합은 3.3 ms 로 순서가 뒤다. 소유자별 인계 [W98]~[W103], 측정 규약(로깅 off ·
Standalone · 세 갈래 확인) 원칙 P182. 상세: `soldier_ai_lab/ai/2026-09-21_perf_instrumentation_and_cover_cost.md`.

**2026-09-21 — 분대 명령 층 본 레벨 적용 + `BreakContact` + 표적 제외**: New_kadex_0811 이관(§6) 첫 PIE 에서 3분대 3차 "도주"
(`Withdraw ReturnFireOnly`)가 2차 존 엄폐를 홉하며 아군과 교전했고, UGV 는 `ExcludeFleeingEnemies` 뒤에도 대타를 계속 쐈다. 원인 셋 —
구 이펙트가 켜 주던 UGV RCWS `bRespectEnemyTargetingExclusion=true` 를 `SetTargetable` 경로가 안 켬 · SoldierLab 아군이 제외 플래그를
아예 안 읽음 · `Withdraw` 는 전투 이동(엄폐 층이 매 홉을 싸울 자리로 값 매김). 수정: 새 동사 **`BreakContact`**(존 유지, 엄폐·경로·위험·
제압 항 0 · dwell 0 · 스프린트 · 자세 0 — "도주는 ROE 가 아니라 땅의 가격", P184) · `IsContactExcluded`(후보 제외·잠금 해제, 인지 불변 P185) ·
`IssueSquadOrderSpec` 이 UGV 스위치를 켬(트럭 제외) · DT `Squad3Run`/`Squad3Stand`. 2차 PIE 판정 [C-163]. 문서 세션 발견: RCWS
스티키 표적이 제외를 안 봐 물고 있던 한 명은 시야 1 s 잃어야 놓음([W106]). `GM_SoldierLab` 에서도 시나리오는 돈다(RTSP/HUD/토스트만 잃음) — ⚠ **단, 단일 프로세스 한정**: titan GameState/PC 가 없어 2-PC 에서는 재시작
멀티캐스트가 클라에 안 가고 드론 시뮬 주체가 둘이 된다(09-23, §6 끝).
상세: `soldier_ai_lab/squad/2026-09-21_break_contact_and_targeting_exclusion.md` · `guide/rcws_fire_control_dev_guide.md` 3.3절.

**2026-09-23 — 병사가 차량(트럭)을 상대로 교전한다.** New_kadex_0811 3차 전투지에서 적 3분대가 이동형지휘소 트럭(`BP_TitanTruck`)을
거의 안 쏘고 자리만 옮기던 것(대인은 정상). `SoldierLab.Debug.Engagement.Log 1` 의 `[Engage] … tgt=BP_TitanTruck … believed 1 worth 1
**aperture 0**` — 표적·확신·가치는 전부 통과하고 **사격 자세만 0**. 원인 둘: ① **사격 레인 트레이스가 표적 자신을 벽으로 읽었다** —
차량은 소켓이 없어 조준점이 바운드박스 50%(차체 한가운데)인데, `IsShotBlockedByWorld` 는 충돌점이 조준점에서 `LaneToleranceCm 200`(사람
크기 기준) 밖이면 막힘으로 보고 차량은 Pawn 이 아니라 트레이스가 무시하지도 않는다 → 트럭 실측으로 **측면은 133 cm(통과), 정면·후면은
310~340 cm(막힘)** = 측면 축 ±48° 밖이 사각이고, `PlanAperture` 의 일곱 자세가 전부 같은 레인 테스트 하나를 써서 동시에 탈락 → `Blocked` →
사선 거부 래치 → 재배치 반복(= 관측된 "서 있다 이동"). ② **엄폐 층이 차량 위협의 눈을 차체 한가운데(≈2 m)에 뒀다** — 실제 포탑은
지붕(≈3.3 m+)이라 1 m 이상 낮은 눈으로 엄폐를 계산했고, 그 눈이 차량 콜리전 **내부**라 "여기서 반격 가능한가" 트레이스가 설계 밖
케이스였다. 수정: ① 표적 액터를 레인 판정까지 넘겨 **표적(또는 그 부착 액터)을 맞히면 "도달"**(200 cm 규칙은 남아 **대인 동작 불변** —
병사는 Pawn 이라 애초에 무시된다), 레인 캐시 키에도 표적 추가 ② 표적 소켓이 **없을 때만**(= 차량) 눈높이 단차를 그 대상의 **형상**
(바운즈 80% − 50%)에서 계산 — 형상은 보면 아는 것이라 지각 규칙 위반이 아니다. `L_SoldierTest`(트럭 정면 = 최악 조건) 재현 후 수정 확인 →
**New_kadex_0811 본 레벨에서도 사격 정상(사용자 확인)**. 남은 [C-166]~[C-168]: `TargetRadiusCm 45` 가 사람 가슴 기준이라 트럭도 45 cm 표적처럼
조준 게이트를 재는 것(원거리 Aimed → Suppressive) · 차량 조준점이 포탑이 아니라 차체 중앙 · 차량 위협 엄폐 품질 수치 미측정.
상세: `soldier_ai_lab/ai/2026-09-23_vehicle_target_engagement_fix.md`(원칙 P189).

**2026-09-23 — 로우레디(총 내림) 상체 레이어 + 아군 앉기 IK.** 병사가 정지·걷기에서 총을 내리고 조깅에서 올린다(`SoldierCharacter_ABP`
레이어 + `USoldierAIBridgeComponent` 의 **포즈 전용 램프** — 캐릭터의 `WeaponLowered` 와 **분리된 별도 값**이다. 캐릭터 값은 몸통 요 회전속도와
조준 보정 게인을 계속 쓴다). 값은 `BP_SoldierCharacter` 의 `WeaponRaiseRate 2.5`/`WeaponLowerRate 3` 과 cvar `SoldierLab.Pose.*` 5개(다음 정식 빌드에
`UPROPERTY` 승격 [W117]). ⚠ 이 프로젝트에서 재발하기 쉬운 **엔진 함정 둘**이 여기서 확정됐다: **`Layered blend per bone` 은 가중치가 정확히
0 이면 노드가 통째로 스킵되고**(`AnimNode_LayeredBoneBlend.cpp:249`), **`Curve Blend Option` 기본값 `Override` 는 레이어 커브가 베이스 커브를 덮는다**
— 둘이 합쳐져 additive 로 딸려온 로코모션 커브(`Enable_Warping` 등)가 0 에 닿는 마지막 프레임에 튀고, 증상은 "총이 다 올라올 때 오른손이
턱 하고 튐" 으로 나타났다(해결 = `UseBasePose`). 같은 날 **아군만 앉을 때 무릎이 안 굽던 것**도 해결 — 원인은 애님이 아니라 **디자이너가
재임포트한 메시 LOD0 의 "Bones to Remove" 에 `ik_*` 본이 들어가 있던 것**이고, **FBX/블렌더 대조로는 "본 동일" 로 오진했다(정답은 스켈레톤
트리의 속이 빈 동그라미 아이콘 = NonRequiredBone)**. 상세: `soldier_ai_lab/animation/2026-09-23_low_ready_upper_body_layer.md`(P190~P191) ·
`soldier_ai_lab/animation/prototypes/2026-09-23_ally_crouch_ik_bones_removed.md`.

**2026-09-23 — 차량이 밟는 시체 · 사망 시 무기 드롭(코드 완료·재빌드 실측 대기).** UGV(`BP_UGV_0901`)가 쓰러진 적을 가끔 밟고 지나가는데,
**밀려나는 그림은 좋지만** 시체가 땅에 박혀 부들부들 떨고 그 값이 서스펜션으로 흘러갈 수 있었다(**뒤집힘 목격은 0회 — 예방**). 원인:
Chaos 서스펜션은 `ECC_WorldDynamic` 채널로 트레이스하고 응답은 `WheelTraceCollisionResponses` 를 쓰는데 **엔진 기본값이 "차량만 Ignore, 나머지
전부 Block"**(`ChaosWheeledVehicleMovementComponent.cpp:1142-1143`)이고 UGV 에 이를 덮는 코드가 없었다 — 사망 시 래그돌 메시는
`ECC_PhysicsBody` + `QueryAndPhysics` 라 **바퀴가 시체를 지면으로 읽었다**(그 바퀴만 지면이 수십 cm 위로 뛰어 스프링 힘이 튀고 마찰까지 시체의
물리재질에서 온다). 수정 ①: `UUGVWheeledVehicleMovementComponent` 생성자에 **한 줄** `WheelTraceCollisionResponses.SetResponse(ECC_PhysicsBody,
ECR_Ignore)` — **물리 접촉은 그대로 둬 밀려나는 그림은 유지**한다. 적용 범위는 `AUGVWheeledVehiclePawn` 자손(= UGV 계열)뿐. 수정 ②: 사망 시
무기 드롭(`USoldierHealthComponent`) — 래그돌이 시작되는 **바로 그 순간** 떨구고(더 일찍이면 쥔 손에서 총이 빠진다), 떨군 것은 시체와 **같은
`ECC_PhysicsBody`** 로 두어 ①의 한 줄이 둘을 동시에 덮게 하고, 응답은 전부 Ignore 에서 시작해 **WorldStatic/WorldDynamic/PhysicsBody/Vehicle 만
Block**(바닥의 소총이 시야벽·엄폐물로 계산되면 안 되므로 **Sight/Cover 제외**), 질량 3.5 kg 오버라이드. ⚠ **함정: 화면에 보이는 총은 스폰된
액터가 아니라 `BP_SoldierCharacter` 자기 컴포넌트 `WeaponMesh`(`SK_KA74U_X`)** 이고 `BP_AR4Rifle`(`SK_AR4_X`)은 숨은 총구/FX 용이다 — 첫 빌드에서
총이 손에 그대로 붙어 있던 이유가 이것이고, 드롭 본체는 **컴포넌트 detach** 경로다(안 보이는 부착 액터는 안 떨군다). 남은 것: 차체↔래그돌
**물리 접촉 자체**(끼임·떨림)와 `FreezeCorpse` 가 애님만 멈추고 래그돌 바디는 계속 시뮬한다는 점은 **손대지 않았다**([W118]).
상세: `soldier_ai_lab/ai/2026-09-23_corpse_vehicle_interaction_and_weapon_drop.md`(원칙 P192, 값 [C-171]~[C-173], 작업 [W118]~[W119]).
⚠ **UGV 쪽에서 찾는 경우**: 차량 파일은 `Source/titan_example/Vehicles/UGVWheeledVehicleMovementComponent.cpp` 한 줄이고, 그 근거와 남은 항목은
전부 위 SoldierLab 문서에 있다(`vehicle/ugv/` 에 별도 문서 없음).

**2026-09-29 — 병사 이동 속도가 표 한 장으로 조절된다(걸음걸이 상한 + 배율 0.75~1.25).** "시나리오에서 병사가 너무 빠르다" 는 제보에서 나온 작업.
조절점은 `Content/SoldierLab/Data/DT_SoldierMovement` — **행 16개(상황) × 열 2개**: `MaxGait`(그 상황에서 허용하는 가장 빠른 걸음걸이 = **상한**,
AI 판단을 덮지 않는다) + `SpeedScale`(그 걸음 속도에 곱하는 배율). 연결점은 `BP_SoldierCharacter → AC_SoldierMovementProfile`.
★ **배율 범위 0.75~1.25 는 임의값이 아니다** — GASP ABP 의 `Get_DynamicPlayRate` 가 `Clamp(Speed2D / 클립의 MoveData_Speed, Min, Max)` 로만
애니메이션을 적응시키고, **`Min/MaxDynamicPlayRate` 커브가 없는 클립에는 대체값 0.75 / 1.25 를 쓴다**(우리 클립 전부 해당). 클립 실측
`ALLY_MM_Rifle_Walk_Fwd` **291.31** · `Jog_Fwd` **582.62** 가 캐릭터 `WalkSpeeds`/`RunSpeeds` 와 **같으므로**(클립이 그 속도로 리타이밍돼 있다)
**발이 미끄러지지 않는 구간은 Walk 218~364 · Jog 437~728** 이고, 표는 그 안에만 머문다(세 겹 클램프 — 에디터 입력 제한 + 코드 + 전역 cvar 합산 후).
현재 값은 Jog 1.00 = 583(기본·돌입·도주·엄폐 이동)부터 Walk 0.75 = 218(사격·모서리 전진·엿보기·피격)까지. 콘솔 `SoldierLab.Move.Enabled`(A/B) ·
`SoldierLab.Move.SpeedScale`(전역 배율) · `SoldierLab.Debug.Move 1`(상황 전환 로그). **PIE 실측 완료** — 전 행을 218 로 밀어 "확실히 느려지고
미끄러짐 없음" 확인 후 계획값 원복(둘 다 사용자 확인). ⚠ **09-28 의 1차 구현(배율 3열 · 안전범위 0.45~1.2)은 폐기됐다**: CMC 의 `MaxWalkSpeed` 에
곱했더니 **GASP `AC_PreCMCTick` 이 CMC 직전에 매 프레임 덮어써서 전혀 안 먹었고**, 입력으로 옮긴 뒤엔 배율이 위 밴드 밖이라 발이 미끄러졌다
(그 안전범위는 **측정 없이 정한 임의값**이었다). 가속·회전 배율 2열도 같은 이유로 **죽은 값**이라 삭제했다 — 상황별 가속·회전이 필요해지면
틱 순서를 먼저 해결해야 한다. 218 보다 더 느리게 하려면 **표로는 불가능**하고 클립에 재생배율 커브를 굽거나 느린 걷기 클립을 추가해야 한다.
상세: `soldier_ai_lab/animation/2026-09-29_movement_policy_and_playrate_band.md`(원칙 P193, 값 [C-174], 작업 [W120]~[W122]).
디자이너용 안내는 `guide/soldier_movement_speed_guide.html`(**09-29 재작성 중** — 09-28 판은 3열·0.45~1.2 기준이라 현재와 다르다).

**2026-09-30 — AI 병사 조준(총구) 떨림 해결(사용자 판정) + 조준 대각선 이동 발 끌림 원인 확정.**
조준 떨림은 신규 프레임 단위 조준 녹화기(`SoldierLab.Debug.AimTrace 1` → `Saved/AimTrace/<시각>/<병사>.csv`)로 8회 녹화하며 원인을 하나씩 지웠다 —
엔진 `AAIController` 가 AI 컨트롤 피치를 0 으로 두던 것(신규 `ASoldierAIController` 가 교전 조준 회전을 그대로 씀, `AIC_Soldier` 부모 변경) · 선회 오차 비례 40~150°/s ·
자세 축 리밋 사이클로 웅크림이 매 프레임 토글되던 것(착지 + 슈미트 트리거) · 조준 보정 와인드업(총이 조준에 있을 때만 빠르게 적분, 밖에선 누설) ·
서기↔앉기 AO 즉시 교체(관성 0.25 s) · 뛰는 중 조준 해제 · 몽타주 슬롯 그룹 분리와 사격 금지 = 재장전 몽타주 길이(`BP_AR4Rifle` Delay). 정지 총구 오차 중앙 5~6° → 1.4~1.7°.
남은 것: 앉아 걷기 조준 오차 · 아군 재장전 1.6 s 손목(디자이너 애셋). 상세 `soldier_ai_lab/ai/2026-09-30_ai_real_pitch_and_aim_smoothing.md`(원칙 P195~P199).
대각선 발 끌림은 우리 걷기/조깅 Loops 포즈 DB 가 **4방향뿐**(GASP 18방향)이라 45° 에서 모션매칭이 Stop 을 고르는 것 — 편향 임시 완화만 적용했고(8 PSD, P4 미제출)
**근본 수정(대각선 클립 추가)은 대기**. 곁가지: 이동 정책(`SoldierMovementProfile`)이 **플레이어 조작 캐릭터에도 적용**되고 있음(미수정) · 적군 일부 클립 발 접지 커브 누락.
**맹목사격은 임시로 꺼져 있다**(cvar `SoldierLab.Engagement.BlindFire`, 기본 0 — 다시 켤지는 결정 대기). 상세 `soldier_ai_lab/animation/2026-09-30_diagonal_aim_stop_selection.md`(P200).

## 8. 피격 이펙트 · 교전 오디오 — 완료

지형/바위/나무/PCG 전체 재질별 피격 이펙트(파티클/사운드/데칼) 배선 완료. 상세:
`sfx_vfx/hit_effects_update_2026-08-26.md`.

**2026-09-21 교전 오디오**: SoldierLab 35명 교전에서 총성이 끊기던 원인(발사량 ×4 × 보이스 상한 32)
해결 — `DefaultEngine.ini` MaxChannels 64, 총성을 `MSS_RifleCrack`/`_Enemy`/`MSS_RifleTail` MetaSound로
분리(꼬리는 Owner 2 + 전역 8 Concurrency), C++ 런타임 감쇠(RCWS 발사음·피격·휘즈·SoldierLab 투사체)를
NaturalSound+LPF 헬퍼로 통일, RCWS 발사음 Concurrency 8, 1인칭 사수 강조는 SoldierLab `SA_Weapon`(정본)
커스텀 곡선만으로. SoldierLab `BP_RifleProjectile.SurfaceImpactEffects`가 비어 있던 것도 복원. 오디오
차폐는 개활지라 보류. **음속 지연** `USoldierAudioLibrary::SpawnSoundAtLocationSpeedOfSound`(RCWS 발사음·피격음,
소총 피격음 C++ + `BP_AR4Rifle.PlayShotSounds` 크랙/꼬리 BP)도 풀 빌드·배선·PIE 청취 검증 완료(PIE 종료 크래시 1건 —
람다의 `TStrongObjectPtr` — 값 복사로 수정됨).
상세 `sfx_vfx/2026-09-21_combat_audio_voice_budget_and_attenuation.md`.

## 9. 드론(UAV) 물리 재구현 + 교전 관측 이동 — 완료, 2대 PC 실환경 검증 완료(2026-09-15·23)

기존 운동학 근사 비행을 로터별 추력→토크→강체 운동 정통 모델로 전면 재구현
(`ADronePawn`). **2026-09-01 기준 구 `BP_UAV` 대체 작업까지 전부 완료** — 구동계·수동 조종
(실기 검증), 자율비행(스플라인 경로 추종, Pure Pursuit+제동곡선), 짐벌 카메라+자동 정찰,
프로펠러 사운드, 바람 반응, 단계별 탐지(아군 → +낙하산 → +적군), 시나리오 연동, RTSP
(`selfdefense/uav_gimbal`), 리플리케이션.

시나리오가 바뀌었다: **드론의 낙하산 관측 성공이 UGV 출발 트리거**가 됐다(예전엔 "UAV 적 감지").
적 탐색 단계와 아군 집결 대기는 제거됨. `DT_ScenarioSteps_ThreeStage`에 `DroneSeeEnemies`/
`DroneWideView` 행 신설.

리플리케이션은 **풀 시스템에선 클라이언트(자체방호축) 권위, 데모 모드에선 리슨서버 권위** —
풀은 조종 주체가 클라라 지연 때문이고, 데모는 자체방호 클라가 없는 1 PC 구성이 정상이라서.
주체는 접속 여부가 아니라 모드로 판정한다(핸드오버 회피). Chaos Resimulation은 RTSP 지연 +33ms와
UGV 거동 변화 때문에 기각(`replication/2026-09-01_drone_client_authoritative.md`).

**2026-09-03~05 — 교전 관측 이동 추가, 실동작 확인 완료.** 교전이 시작되면 드론이 **활성 경로
스플라인 위에서 "전황을 가장 잘 보여주는 지점"으로 스스로 이동**한다(짐벌은 각도만 바꿀 뿐
거리를 못 줄이므로). 자유비행이 아니라 그려진 선 위에서만 고르는 구조라 지형 회피가 필요 없다.
프레이밍 대상은 시나리오 단계별로 정해지고 **레지스트리에서 직접** 읽는다 — 탐지 결과를 쓰면
`줌아웃→탐지해제→프레이밍축소→줌인` 순환 의존이 생긴다. 도주 중인 적은 대상에서 뺀다.

이 과정에서 유도 루프의 구조 결함 3연쇄를 해결했다(적분 와인드업 → 축별 안티와인드업 누락 →
**속도 피드포워드 부재로 인한 ζ=0.51 저감쇠 진동**). 경로 추종이 원래 부드러웠던 건 거버너
속도를 쓰는 이미 피드포워드 구조였기 때문이고, 관측만 순수 위치 피드백이라 6.8초 주기로
출렁였다. 겉보기 크기 필터(`MinScreenSizeFraction`)가 화각에 반비례해 교전 광각에서 탐지
사거리를 800m→45m로 무너뜨리던 버그도 같이 잡았다.

**2026-09-10 — 수동 조종을 비행/짐벌 두 축으로 분리, 실동작 확인 완료.** "드론 짐벌 카메라만
수동" 모드(`UAVGimbal`)를 추가해, 기체는 자율비행/교전 관측을 계속하면서 사람이 카메라만 돌려볼
수 있다(전시에서 가장 많이 쓰이는 조합). 기존 토글들과 상호배타. 수동을 놓으면 짐벌이 기본
자세로 천천히 복귀한다.

같이 잡은 버그 둘: 수동 해제 시 **출발 위치로 576km/h 역주행**(자율비행 재진입을 "지상에서 새로
시작"으로 오판해 이륙 단계로 들어가던 것)과, 교전 관측 중 수동을 껐다 켜면 **관측 상태를 잃고
경로 끝까지 주행**하던 것.

**2026-09-15 — 짐벌 2축 안정화 + 2대 PC 실환경 검증 완료.** 짐벌 각도의 기준을 기체 → 수평
프레임으로 바꿔 낙하산 순항 가감속 기울기가 카메라에 안 실린다(요·피치만, 롤은 리그에 본이
없어 상쇄 안 함). 자동 추적 목표각을 같은 프레임으로 통일한 게 핵심. CineCamera 니어플레인이
씬캡쳐(위젯/RTSP)에 복사 안 되던 것도 수정.

2대 PC(서버=UGV축, 클라=자체방호축) 첫 실환경 검증에서 "전혀 리플리케이션 안 됨" → 버그 3건
수정 후 풀/데모 양쪽 정상: (a) 데모 모드에서 서버·클라 **둘 다** 주체(판정 순서), (b) 풀
시스템에서 `Server_ReportState`가 **로그 없이** 폐기 — 엔진 기본 `AutoPossessAI`로 AI 컨트롤러가
빙의해 `APawn::GetNetConnection`이 null, `SetOwner`만으론 부족했음(`AutoPossessAI=Disabled` +
`GetNetConnection` 오버라이드), (c) 서버가 주체일 때 Rep*를 아무도 안 채워 데모 클라 드론이
출발점에 굳음(서버 Tick에서 직접 게시). 상세 `replication/2026-09-15_drone_two_pc_validation.md`.

**2026-09-23 — 클라 드론 화면에 전장이 통째로 없던 것 해결(제일 큰 건).** 자체방호 클라에서만
탐지 결과가 낙하산·트럭 2개뿐, 총구화염·사격음 없음, 병사 정지 — *"혼자 다른 씬에 있는 느낌"*.
원인은 **거리 기반 네트워크 관련성**: 기준점은 짐벌 씬캡쳐가 아니라 연결의 ViewTarget(=트럭,
57330/12280)이고 전장은 923 m 밖인데 컷이 150 m(엔진 기본 `NetCullDistanceSquared`). 드론만
멀쩡했던 건 `PostLogin`의 `SetOwner` 덕, 낙하산은 `bReplicates=false`라 로컬 사본이어서.
사격·피격 **멀티캐스트도 관련성 검사로 송신 단계에서 폐기**됐다. **Solo/호스트 자기화면은
관련성 판정 자체가 없어(원격 연결 전용) 증상이 안 나서 오래 PIE 문제로 오해했다.** 수정 =
`UDetectableTargetComponent::BeginPlay`에서 탐지 대상 액터를 always relevant로(범위가 곧 "드론/
RCWS가 봐야 하는 대상"이라 훅이 맞음). 비용은 LAN에서 ~30 KB/s로 무의미. 같이: 클라에
`AuthGameMode`가 없어 치트 매니저가 안 생겨 죽어 있던 `ToggleDebugCamera`(패키징 호스트도 동일)
→ `EnableCheats()`. 상세 `replication/2026-09-23_net_relevancy_battlefield.md`.

**2026-09-23 — 원격 로터 회전 복제 + 분대 트래킹 판정 이관.** (1) 원격은 Flight 틱이 꺼져
`RotorThrustN`이 0이라 프로펠러 소리가 idle에 고정되고 날개도 멈춰 있었다 →
`RepRotorSpin01/Spread01` 추가, 소리·날개가 `ComputeRotorSpinStats()` 한 소스를 본다. (2) 적군을
SoldierLab 병사로 교체하며 "3분대 도주 제외"가 **조용히 죽어 있던** 것(판정 근거 `UEnemyCombatComponent`가
새 병사엔 없어 `if (Combat && …)`가 전원 통과) → 복제되는 `SquadId`/`bBreakingContact`를 브리지가
채우고 `ResolveEnemyTrackingFacts()`가 구/신을 흡수. ⚠ 작업 중 **`Withdraw`를 도주로 본 오판**
(실제론 교전 직후 2·3분대에 내려가는 평범한 전투지 이동 — 시나리오가 통째로 망가졌다)과
**분대 이름 `"Squad3"` 오기**(정본은 `AScenarioConfig::SquadZones` = `"3"`)로 두 번 되돌렸다.
상세 `vehicle/drone/2026-09-23_drone_remote_rotor_and_squad_tracking.md`.

남은 것: 구 `AUAVPawn`/`BP_UAV`와 폴백 분기 제거, 경로 고도 상향(부감이 선호 범위 -45~-60°를
못 채움 — 교전 반경 R 대비 R×1.0~1.25 위가 기준), 도주 시작 장면 프레이밍 개선(4단계 대상이
33개라 도주하는 적이 점으로 보임), 원격 보간(`RemoteInterpSpeed`) 품질 튜닝(문제 보고 없음),
**PIE 로비 경유 시 드론 화면의 나무 라이팅·팝핑**(패키징에선 안 남, 관련성과 무관 — 원인 미확정).
상세: `vehicle/drone/drone_flight_dev_guide.md`(레퍼런스),
`vehicle/drone/2026-09-01_drone_replaces_bp_uav.md`·`2026-09-05_drone_engagement_observation.md`·
`2026-09-15_drone_gimbal_stabilization.md`(작업 경과).

## 10. 문서 관리 — 2026-08-31, 1단계 완료

여러 세션이 독립적으로 문서를 만들면서 폴더가 뒤섞인 문제를 해결 중. **1단계(구조 재편) 완료**:
`CLAUDE.md`(문서 작성 규칙: 파일명 날짜 접두, 1줄 헤더), `DOCS_INDEX.md`(전체 카탈로그),
`WORKLOG.md`(전체 작업 시간순 서사, 신규), `guide/` 폴더 신설(레퍼런스성 dev guide 9개를 이동,
전부 "최신화 필요" 경고 배너 부착 — **내용 자체는 아직 옛날 것**), 레거시/중복 문서 16개
`_archive/`로 이동, `structure/rc_gui/`(중복 코드)·`README_save.md` 삭제, `genesis/` 재정리.
기존 문서 대량 리네임(날짜 접두 소급 적용)은 상호 참조 깨짐 위험 때문에 안 함.

**2단계(아직 착수 전)**: `guide/`의 각 문서 내용을 실제 최신 코드/동작에 맞게 다시 쓰는 작업 —
시스템별(RCWS, UGV 주행, UI, 카메라 파이프라인 등)로 쪼개서 별도 세션 필요, 한 번에 다 하긴 큼.

## 11. 알려진 미해결 이슈 (요약, 상세는 각 원본 문서)

- 자체방호축 카메라 떨림/발사반동 누출 버그 — 코드 수정 완료, 2-PC 실환경 검증만 남음(§3).
- **[2026-09-16] RCWS 성능 업그레이드(타겟 기억/경계도 + 마지막 위치 4초 응시 + 뼈 기반
  탐지/부위 조준) — 코드·풀 빌드·인스턴스 설정 완료, PIE 1차 확인(응시 7→4초 조정).** UGV/트럭
  BP 기본값 **과 레벨 인스턴스 네 곳 모두** `TargetDetection.AcquireRule=AnyVisibleSample` MCP로
  설정·저장(`.umap`엔 델타 직렬화로 안 남음 — 정상). 남은 것: PIE 세부 검증(머리만 나온 적
  획득·머리 조준, 재출현 즉시 재획득+락온 단축, 전방 45° 우선 유지, 트럭 동일) + 튜닝. 상세
  `rcws/2026-09-16_rcws_target_memory_and_body_part_aim.md`.
  - **[2026-09-17] 청각 보조 추가 — 빌드 완료, 반경 50m 재조정(09-21) 후 재빌드·PIE 검증 대기.**
    시각 탐지도 응시할 기억도 없을 때 50m(100→25→50m) 안 최근 적 총성 방향을 4초 조사
    (`InvestigatingGunfire`, 우선순위 시각 → 응시 → 총성 → 스윕). 총성은
    `UDetectableTargetSubsystem::ReportGunfire` 버스 — 구 적군은 `EnemyCombatComponent`, **SoldierLab
    적군은 `USoldierLabBridgeSubsystem`**이 보고(아군 총성 제외). 09-21 `L_SoldierScenario`에서 "안 된다"
    조사 결과 경로는 정상이고 적 배치가 UGV 목적지에서 70~76m라 25m 반경 밖이었음 → 50m로.
    검증 항목: 총성 후 스윕이 그 방향으로, 시각 우선, 아군 사격/50m 밖 무반응, 만료 후 블렌드
    아웃. 상세 `rcws/2026-09-17_rcws_gunfire_hearing.md`.
- 적 3분대 재편 — 코드 구현 완료, PIE 재검증 대기(§7).
- 자체방호축 PC 고정 IP 미확정(LIG 확인 필요).
- ~~미니맵 이미지가 랜드스케이프 동쪽을 못 덮는다~~ — **2026-09-22 종결(무시하기로 결정)**. 레벨 `-Y` 변이 `m_map.png`
  동쪽 끝 밖 205~340 m 인 건 사실이지만, 시나리오 액터는 전부 랜드스케이프 중심부에서만 움직이므로 그 띠에 갈 일이 없음
  (사용자 확인). 미니맵 재생성 안 함, `GeoCoordinateUtils.h` 상수도 그대로. 나중에 동쪽 띠를 쓰게 되면 그때 재생성
  (`protocol/2026-09-22_minimap_georeference_for_lig.md` §4, `guide/real2world.md` §5 에 재측정 절차 있음).
- **08-26 스케일 변경 이후 시나리오 타이밍 재점검 미실시** — `GetDistanceScaleFactor` 1.2135→1.0006 으로 UAV 상승/순항 가속·UGV
  제동거리·에스코트 속도·RCWS 탄도가 씬 단위로 21% 빨라졌다(옳은 값). `BeginScenarioEnemyContact` 흐름과
  `vehicle/ugv/2026-08-22_ugv_corner_braking_dev_guide.md` 튜닝값이 그 뒤 실측으로 다시 맞춰졌는지 확인 안 됨.
- `RCWSFireControlComponent.h:41~51` 헤더 주석이 아직 "scene ~18.5% smaller"(낡음, 로직은 정상) · 같은 파일 `MaxEffectiveRangeMeters=2000`
  은 어떤 .cpp 도 참조 안 하는 죽은 프로퍼티(기존 상태).
- `RC_MotionMode` 용도 불명(LIG도 "차후 논의", 급하지 않음).
- 자율주행 목적지 명령(`HQ_MissionMoveToEngage`)이 LIG 정식 스펙 아님, 확정 대기 중.
- 구 `AUAVPawn`/`BP_UAV` 및 시나리오 폴백 분기 제거 — 2대 PC 검증(2026-09-15 완료)이 끝났으니
  착수 가능(§9).
- 살아있는 적이 이동 중 피격되면 몸이 회전하는 현상 — 재현 실패로 보류
  (`ai_combat/2026-09-01_enemy_spin_on_hit_investigation.md`).
- 버스트 사격 중 사격선(아군 관통) 재검사 없음 — 사격 시작 시점에만 검사
  (`ai_combat/2026-09-03_enemy_combat_fixes.md`).
- **리눅스 패키지가 GStreamer/NVIDIA 라이브러리를 `DT_NEEDED`로 직접 링크** — 타겟 머신에
  `libgstreamer-1.0.so.0`/`libgstapp-1.0.so.0`/`libgstrtspserver-1.0.so.0`/`libnvidia-encode.so.1`/
  `libcuda.so.1`이 없으면 **RTSP만 꺼지는 게 아니라 프로세스가 로더 단계에서 즉시 죽는다**(2026-09-03
  외부 테스터 실행 실패로 확인). nouveau/Mesa 머신은 실행 불가. 런타임 soft-fail(dlopen 전환)
  미착수 — `packaging/2026-09-02_linux_package_ugv_host_rc_test_guide.md` §3-1.
- **Vulkan ICD 미설치 시 `Failed to load Vulkan Driver`로 실행 불가** — `nvidia-smi`가 되고
  `libvulkan1`이 있어도 발생한다(로더 ≠ 드라이버). 검증은 `vulkaninfo --summary`로 해야 함.
  같은 문서 §7-1. ICD 위치는 apt 설치본 `/usr/share/vulkan/icd.d/`, NVIDIA `.run` 설치본
  `/etc/vulkan/icd.d/`(2026-09-15 확인).
- **NVIDIA 드라이버 570 미만이면 프로세스는 뜨지만 RTSP만 안 된다**(2026-09-15 확정) — NVENC SDK
  13.0.37 기준. 실행 가이드 §0에 명시, 미달 시 클라이언트는 즉시 404(09-15 빌드부터).
  고객 드라이버 버전 재현 절차는 `packaging/2026-09-15_linux_nvidia_driver_595_run_install.md`.
- **에디터 MCP 서버(Auto Start Server, 8000)가 떠 있으면 기본 Package Project는 `Cook failed`로
  끝난다**(2026-09-15 원인 규명·우회 완료) — 쿠커가 같은 포트에 MCP 서버를 띄우려다 남긴 Error 1줄
  때문. 패키징은 Platforms ▸ Project Custom Builds ▸ "Package Linux (MCP 8000 회피)"로 할 것
  (`packaging/2026-09-15_linux_cook_failed_mcp_port_clash.md`). 이 경로로 09-15 전체 패키징 성공 확인.
- **리눅스 패키지 실행 스크립트의 Wayland/X11 자동 판별은 `Config/BootstrapPreamble.sh`에 의존**
  (2026-09-15) — 이 파일이 체크아웃에 없으면 패키징은 조용히 성공하지만 `titan_example.sh`에 판별
  블록이 빠져 순수 X11 머신에서 `wayland not available`로 죽는다. 결과물의 스크립트에 `### Added from
  project Config BootstrapPreamble.sh` 마커가 있는지 확인할 것
  (`packaging/2026-09-02_linux_package_ugv_host_rc_test_guide.md` §2-5 체크리스트).
- **UGV에 안티롤바가 없다** — `RollbarScaling=0.15`가 설정돼 있지만 엔진이 축을 **휠 클래스
  기준**으로 묶는 탓에 6륜이 한 축이 되고, 롤바 코드가 `축당 휠 2개`만 처리해서 스킵된다.
  한쪽 바퀴만 장애물을 타면 차체가 복원력 없이 기운다. 살리려면 휠 클래스를 앞/중/뒤 3개로
  복제해야 함(`vehicle/ugv/2026-09-10_ugv_0901_suspension_tuning.md` §3/§6).
- UGV 자율주행 튜닝 오버레이(`UUGVDriveTuningWidget`) — 코드 작성됨, **빌드/실동작 미확인**.
- Graphics Settings 위젯 실제 구현 미착수(§5).
- `guide/` 문서 내용 최신화 — 위 §10 2단계, 아직 시작 전.
- Unreal 에셋/코드 정리(레거시 BP, 폴더 구조) — 별도 세션 착수 예정, 아직 시작 전.

## 12. LIG에 발송 대기 중인 질문

`protocol/lig_questions_0816.md` §1 맨 위(1-신규-1~3) + §5(회신할 내용 2건, 아직 미발송) —
RC_OperationMode=EMERGENCY 레거시 가능성, 차량 시동 관련 커맨드 존재 여부, "LLM" 발언 확인,
NavMesh 기능 회신, ObjectClass 회신.

- **[2026-09-22 신규] 지도 이미지 지리참조 답장 대기** — LIG 가 픽셀 스케일(ScaleX/Y/Z)+타이포인트(I,J,K,X,Y,Z)를 요청
  (시뮬레이션개발팀 경유). 현재 `m_map.png` 기준 수치·답장 초안 완료(`protocol/2026-09-22_minimap_georeference_for_lig.md` 부록),
  발송만 남음. 지도 재생성(동쪽 확장·UTM 정렬)은 별도 제안.
- **ObjectClass(§5-2)는 이제 질문이 아니라 "결정 통보"** — LIG가 Q10에서 우리 재량으로
  확정해준 항목이라 답변을 기다리지 않고 2026-09-02에 플랫 6값(`Ally`/`Enemy`/`UGV`/
  `MobileCommandPost`/`Drone`/`Parachute`)으로 확장 구현 완료. 남은 건 발송뿐, 답변 대기
  아님. 상세는 `protocol/2026-09-02_object_class_expansion.md`.
