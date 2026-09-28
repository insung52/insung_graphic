# WORKLOG — 프로젝트 전체 작업 시간순 기록

2026-08-31 / 진행중 / titan 프로젝트 전체를 실제 작업 순서대로 서술한 타임라인.

`DOCS_INDEX.md`가 "문서가 어디 있는가"의 카탈로그라면, 이 문서는 **"무슨 일이 어떤 순서로
있었는가"**의 서사다. 겹치는 정보가 있는 게 정상 — 목적이 다름. 날짜 출처/불확실성 표기는
`DOCS_INDEX.md` 서두 참고(특히 `2026-07-24 09:40~41` 일괄 이관 건).

---

## 2026년 6월 — 사전 조사, 카메라 파이프라인 초기 구현

- **06-23** `tanksim_tank_analysis.md` — 참고 프로젝트 TankSim의 `ATankPawn` 분석(4분할
  카메라 기능 설계 전 사전조사).
- **06-24** `guide/quadcam_usage_guide.md` — QuadCamModule(4분할 카메라) 플러그인 최초 구현.
- **06-25** `titan_dev_status.md` — QuadCamModule 개발 현황(M키 토글, 플러그인화).

## 2026년 7월 상순 — 조작/탐지/UI 기반 다지기

- **07-06** `guide/pixelstreaming_setup_guide.md`, (구)`ugv_driving_dev_guide.md`(현재
  `_archive/path/`) — Pixel Streaming 테스트, UGV 수동/자동 주행 최초 구현(`kadex_demo` 레벨,
  `BP_UGV`).
- **07-08** `guide/joystick_camera_control_dev_guide.md` — 조이스틱(Logitech Extreme 3D Pro)
  기반 RCWS/UAV 카메라 조작.
- **07-09** `guide/detection_dev_guide.md` — 카메라 기반 객체 탐지(BBox) 모사 구현.
- **07-10** `guide/ui_dev_guide.md` — Monitor1/2 UI 연동(당시 최신, 지금은 레거시로 확인됨).
- **07-12~14** `guide/rcws_fire_control_dev_guide.md` — RCWS 자동/수동 조준·발사 메커니즘.

## 2026년 7월 하순 — 시나리오/아군 시스템 설계 착수

- **07-24 전후** (일괄 이관 타임스탬프) 다수 초기 dev guide/메모 작성 — `all.md`(Phase #4
  시나리오 공식 명세), `chaos.md`/`M1A2_UGV_Conversion.md`(UGV Chaos Vehicle 물리),
  `titan_quadcam_plan.md`(archived) 등.
- **07-28** (구)`ally_and_scenario_system_plan.md`(archived) — 아군 BP+시나리오 관리 설계 논의.
- **07-29** `content_asset_inventory.md` — 디자인팀 제공 Content 폴더 전수 조사(마켓플레이스
  애셋 정리).
- **07-30** (구)`scenario_datatable_system_plan.md`(archived) — 시나리오 DataTable화 설계안.

## 2026년 8월 상순 — LIG 협업 시작, 아키텍처 확정

- **08-04** (구)`scenario_implementation_status.md`, `ally_character_animation_design.md`
  (둘 다 archived) — 아군+UGV 동반이동 구현, 아군 애니메이션 개편 설계.
- **08-05** `infra_architecture/system_architecture_design_spec.md`/`idea.md`/`idea_review.md`,
  (구)`시나리오.md`(archived) — LIG 원본 PDF 분석 시작, 전체 아키텍처 설계 착수.
- **08-06** `_archive/lig_response_0806_review.md`/`nats_infra_setup.md`/`sessions_idea.md`,
  `documents/문의내용0806.md` — LIG 최초 Q&A, NATS 인프라 구축(이후 폐기), 세션 분할 구상.
- **08-07** `_archive/udp_test_findings.md`/`architecture_decisions.md`, `ally_animation_request.md`,
  `ally_move.md` — LIG 참조구현 검토, UDP 전면 통일 결정(NATS 폐기), 아군 애니메이션 요청서.
- **08-10** `ui/kadex_test_dashboard_wbp_spec.md`, `ai_combat/ally_ai_combat_system_status.md`/
  `enemy_ai_combat_system_status.md` — 아군/적군 전투 컴포넌트 최초 구현.
- **08-11** `rcws/rcws_preview_actor_asset_ref_todo.md`.
- **08-12** `genesis/2026-08-12_genesis_ugv_conversion.md`(별도 병행 트랙, 현재 일시중단),
  `sfx_vfx/hit_effects_idea.md`→`hit_effects_implementation.md` — 피격 이펙트 1차 구현.
- **08-13** `_archive/restructure_status_0813.md`, `sfx_vfx/hit_effects_update_2026-08-13.md`
  — 구조 개편 스냅샷, 피격 이펙트 2차(도탄/혈흔/화염).
- **08-14** `protocol/lig_icd_ugv_rc_full.md`, `replication_audit.md`(§8까지) — **LIG 정식
  ICD 확보**(가장 중요한 전환점 중 하나), 리플리케이션 감사+구현 대부분 완료.

## 2026년 8월 중순 — LIG 프로토콜 구현, RTSP 착수

- **08-16~17** `protocol/ugv_rc_feature_gap_analysis.md`/`selfdefense_rc_feature_gap_analysis.md`
  — UGV축/자체방호축 LIG 프로토콜 구현+cmd별 갭 분석.
- **08-17** `rtsp/rtsp_integration_complete_0817.md` — RTSP 실 카메라(UGV 5+자체방호 7)
  연결 완료, mount 확정.
- **08-18** `rtsp/rtsp_poc_findings.md`(최종), (구)`structure/rc_gui/`(2026-08-31 삭제됨,
  중복 코드베이스로 확인) — NVENC/GStreamer RTSP 파이프라인 PoC 마무리.
- **08-19** `rtsp/linux_wayland_x11_present_bottleneck.md`,
  `rtsp/rtsp_latency_investigation.md`(경과) — Linux 풀스크린 프레임폭락 해결, RTSP 지연 조사 시작.
- **08-20** `camera_pipeline/rtsp_postprocess_parity_0820.md`, `rtsp/rtsp_resolution_customization_0820.md`
  — RTSP 스트림 SSR/피격흔들림 누락 원인 규명, 해상도 커스터마이징+CCTV 잘림버그 해결.
- **08-21** `ui/graphics_settings_analysis.md`, `ui/ingame_settings_input_system.md` — 인게임
  Settings 위젯 Input 완료+Graphics 조사.

## 2026년 8월 하순 — 레벨 디자인/자율주행 집중 스프린트, LIG 1차 답변

- **08-22** 신규 레벨 NavMesh 최초 구축(당시 `newlevel/`, 08-27 갱신판만 남기고
  `vehicle/ugv/2026-08-27_new_kadex_0811_navmesh_autonomous_driving.md`로 2026-08-31 중복
  통합됨 — 상세는 그 문서 참고), `new_kadex_0811_forest_perf.md`,
  `level_new_kadex_0811/scenario.md` — 신규 레벨(New_kadex_0811) NavMesh 구축, 성능 폭락(2.3→31fps) 수정.
- **08-23** `level_new_kadex_0811/scenario_authoring_guide.md`/`scenario_three_stage_combat.md` — 3단계
  전투 시나리오 DataTable 구현 완료.
- **08-24~25** `ai_combat/enemy_locomotion_animation_pipeline.md`/`enemy_hit_reaction_physics_system.md`/
  `enemy_scenario_combat_expansion.md`, `rtsp/rtsp_client_reception_guide.md`
  (2026-08-24) — 적군 AI/애니메이션/전투 대규모 개편(Part A~G 완료), RTSP LIG 공유용 최종
  가이드 작성.
- **08-25** `vehicle/ugv/2026-08-22_ugv_corner_braking_dev_guide.md` — 커브 선행 감속(제동 곡선) 구현 완료.
- **08-26** `vehicle/ugv/2026-08-26_ugv_track_lock_implementation_plan.md`, `sfx_vfx/hit_effects_update_2026-08-26.md`
  — "공중에 뜬 바퀴" 오래된 버그 완전 해결, 피격 이펙트 최종(전체 재질 커버).
- **08-26** `level_new_kadex_0811/2026-08-26_level_rescale_to_real_world.md`(**09-22 사후 작성** — 당시 devlog 를 안 남겨
  다른 세션이 이 변화를 모르고 작업했음) — **New_kadex_0811 을 현실 1:1 로 재스케일.** 랜드스케이프 대각 코너 위경도 2점으로
  레벨이 26.2% 크다는 것을 확정(Vincenty 등 4방법 0.057% 안), 월드 원점 기준 k=0.7921933250 으로 랜드스케이프·액터 328·볼륨 6·
  도로 스플라인 4·PCG 숲 10 을 균일 축소(소품 스케일 불변), 큐브 독립 검증 0.001 m. PCG 점박이 마스크가 바뀐 원인(Spatial Noise
  월드 XY 고정) → `tree2`/`plant` 그래프 Transform.scale=1/k 로 정확 복원. `GeoCoordinateUtils.h` 보정점 5→2(랜드스케이프 코너,
  Scale≈1.0006·회전 -86.23°), **`GetDistanceScaleFactor` 1.2135→1.0006** 파급 15곳 감사(표시 -17.6%, 물리 스펙 경로 씬 속도 +21%).
  MCP 함정 6건(`set_actor_transform` 생략 필드 리셋, BP 컴포넌트 X 성분만 기록 등). 미해결: 랜드스케이프 동쪽 변이 미니맵 밖.
- **08-27** `vehicle/ugv/2026-08-26_ugv_obstacle_avoidance.md`(확정),
  `vehicle/ugv/2026-08-27_new_kadex_0811_navmesh_autonomous_driving.md`(갱신판),
  `vehicle/drone/drone_flight_dev_guide.md`, `titan/README.md` 갱신 —
  장애물 회피 4대 원인 해결(실측 34.6km/h), 드론 물리 전면 재구현, 최상위 README 갱신.
- **08-28** `documents/response_0828.md` — **LIG 1차 답변 도착**(요청/응답, 안전장치, ICD
  오기입 정정, RC_ActivateMovement 정체 등 대거 해소).
- **08-29** `sfx_vfx/wind_system.md` — 동적 바람 시스템.

## 2026년 8월 31일 — LIG 답변 반영, 문서 정리 착수

- LIG 답변을 `protocol/protocol_icd.md`/`ugv_rc_feature_gap_analysis.md`/
  `protocol/lig_questions_0816.md`에 전부 반영, 후속 질문 3건 정리.
- UGV축 세션이 `RC_ActivateMovement` 재매핑(차량 시동 → RCWS 조향 게이트) 실제 구현 완료.
- **문서 정리 이니셔티브 시작**: `CLAUDE.md`(문서 작성 규칙) 도입, `CURRENT_STATE.md`/
  `DOCS_INDEX.md` 신설, 중복/레거시 문서 정리(`structure/rc_gui/`·`README_save.md` 삭제,
  `genesis/` 재정리, 레거시 dev guide 16개 `_archive/`로 이동), `guide/` 폴더 신설(레퍼런스성
  dev guide 9개 이동, 최신화 필요 경고 배너 부착), 이 `WORKLOG.md` 작성.
- `rcws/2026-08-31_selfdefense_camera_shake_bugs.md` — 자체방호축 카메라 버그 2건
  (RCWS 조향 시 환경카메라/CCTV 떨림, UGV 발사 셰이크가 자체방호 3화면 오염) **원인 확정 +
  코드 수정 완료**. 둘 다 셰이크 경로 하나에서 갈라진 문제였고, 사용자 최초 가설(공유 트랜스폼
  리플리케이션 노이즈)은 반증됨. `SceneCaptureViewParity`(프로브 모디파이어 신설) +
  `RCWSProjectile`(명중 셰이크 발사자 게이팅) 수정. **2-PC 실환경 검증은 미실시**.

## 2026년 8월 28일 ~ 9월 1일 — 드론이 구 BP_UAV를 대체

08-27에 구동계만 완성돼 있던 새 물리 드론(`ADronePawn`)에 나머지 기능을 전부 얹어 구
`AUAVPawn`/`BP_UAV`를 대체한 작업. 상세는 `vehicle/drone/2026-09-01_drone_replaces_bp_uav.md`,
동작 레퍼런스는 같은 폴더 `drone_flight_dev_guide.md`.

- **08-28** 짐벌 이식(BP CineCamera + 런타임 SceneCapture, 구 UAV와 같은 패턴). 본 회전을
  컴포넌트 공간 절대값으로 넘겨야 한다는 걸 몰라서 2회 재작업 — "밖에선 팬이 도는데 짐벌
  시점에선 안 먹는" 증상.
- **08-28** 프로펠러 사운드(평균/편차/로터별 3모드 + 구 UAV식 우선순위 보호), 자율비행 1차 구현.
  설계 원칙은 "**위치를 직접 쓰지 않는다**" — 오토파일럿이 사람과 똑같은 4채널 스틱 입력만
  만들어서 물리를 그대로 살린다.
- **08-29** 다른 세션이 만든 바람 시스템(`sfx_vfx/wind_system.md`) 연동. 자율비행 버그 3건
  해결 — 45km/h 경로를 10km/h로 기어가던 Pure Pursuit 오배선, 커브 감속이 아예 안 걸리던
  `√(a/κ)` 공식, 바람을 미리 읽어 완벽 상쇄하던 비현실적 항력 피드포워드. 셋 다 UGV 자율주행
  로직을 **구조만 보고 옮긴** 것이 원인이었다.
- **08-29** 시나리오 재배선 — **드론의 낙하산 관측 성공이 UGV 출발 트리거**가 됐다(예전엔
  "UAV 적 감지"). 적 탐색 단계·아군 집결 대기는 제거. 트리거 `UAVParachuteObserved`와
  이펙트 2개 신설, `DT_ScenarioSteps_ThreeStage`에 드론 행 2개 추가.
- **08-30** 단계별 탐지(아군만 → +낙하산 → +적군), 클라이언트 권위 리플리케이션, 위젯 배선.
  드론 조종 주체가 서버(UGV축)가 아니라 클라이언트(자체방호축)라 방향을 뒤집었다. Chaos
  Resimulation은 RTSP 지연 +33ms와 UGV 거동 변화 때문에 기각
  (`replication/2026-09-01_drone_client_authoritative.md`).
- **08-31** 유니티 빌드가 익명 네임스페이스를 합치면서 난 상수 재정의 오류 정리.
- **09-01** 단일 프로세스 `New_kadex_0811`에서 시나리오 전체 흐름 검증 완료, 문서 최신화.
  2대 PC 실환경 검증은 이 시점엔 남아 있었음(→ 09-15 완료).
- **09-02** UGV 탐지 `ObjectClass`를 ICD 원문 `Human`/`Car` 2값에서 플랫 6값
  (Ally/Enemy/UGV/MobileCommandPost/Drone/Parachute)으로 확장. 낙하산이 `Car`로 나가던 원인은
  `BP_Parachute`가 `ACharacter`가 아닌 `AActor` 직속이라 2분류의 else 가지로 떨어진 것 —
  `Faction==EnemyEvidence`로 집는다. LIG가 재량으로 확정해준 항목이라 답변 대기 없이 구현,
  통보만 남음(`protocol/2026-09-02_object_class_expansion.md`).
- **09-02** **UGV를 디자인팀 신규 모델(궤도 없는 차륜 6×6, 단발 중기관총 포탑)로 교체** —
  블렌더 리깅(본 10개, 옛 이름 유지)부터 `SK_UGV_0901`/`BP_UGV_0901`/`ABP_UGV_0901`/머티리얼
  7종/주행 튜닝까지. 궤도 비주얼·총열 회전 제거, 스키드 스티어와 TrackLock C++은 그대로 재사용
  (좌우 분류가 휠 개수에 무관해서 6륜에도 그대로 동작). 주행·포탑·서스펜션·시나리오 자율주행·
  피격 재질 판정 전부 실동작 확인. 총열 회전 발사 게이트는 `bUseBarrelSpin` 플래그로 제거
  (`URCWSFireControlComponent`를 개틀링인 TitanTruck과 공유해서 통째로 못 걷어냄 — 기본값 true라
  트럭은 그대로). **작업 완료**, BP 고아 노드 정리만 나중 과제로 남음.
  휠 반지름 15→31.6cm·개수 16→6이 주행에 미친 영향과 스케일 규칙, 블렌더 FBX 루트 스케일 100
  함정, ABP Retarget이 EventGraph 캐스트를 안 바꾸는 함정 전부
  `vehicle/ugv/2026-09-02_ugv_0901_new_model_rig.md`에 정리.

- **09-03** `ui/graphics_settings_analysis.md` 팔로업 — 8/21 Graphics 탭 사전조사 문서를 그동안의
  변경분에 맞춰 재검증·갱신(`§0-2`). 정정 4건(가장 큰 것: `[ConsoleVariables]`의 cvar 우선순위가
  `SetByConsoleVariablesIni`가 아니라 `SetBySystemSettingsIni` — 결론은 동일), 계획 변동 2건
  (프레임 상한은 물리 결정성 대책이라 탭에서 60 위로 열면 안 됨 / 창 모드는 실제 운용이
  `-fullscreen` 런치 인자라 후순위). 카메라 인벤토리도 드론·UGV BP 교체분 반영.
  이어서 `DumpCVars` 전수 실측으로 **`UGameUserSettings` 품질 경로가 완전 no-op**임을 확정하고
  (`sg.*` 12개가 전부 `SystemSettingsIni` 고정), 채택 구조(`UTitanGraphicsSettings` 단일 소스)와
  최종 세팅 후보 목록을 문서 §9/§10에 정리. **구현은 미착수, 사용자 검토 대기.**

- **09-03** `BP_UGV_0901` 블루프린트 로직 C++ 이관 + 죽은 노드 전면 정리. 9/2 문서 §9에 남겨둔
  "고아 노드 ~250개" 과제였는데 실제로는 4개 그래프에 걸쳐 **427개 + 변수 39개**였다
  (`UpdateTurretVisuals` 안에만 168개 — 살아있는 체인의 중복 복제본이 통째로 남아 있었음).
  정리하는 김에 남은 BP 로직 4개(`UpdateTurretVisuals` / `SetManualControl` / `SetBraking` /
  Tick)를 새 부모 **`AUGV0901Pawn`**(`AUGVWheeledVehiclePawn` 서브클래스)으로 전부 내렸다.
  BP는 로직 0줄 · 변수 0개의 순수 데이터 에셋이 됐고 `.uasset`이 **1,150KB → 67KB**.
  구형 `BP_UGV_Vehicle_new`는 안 건드림(같은 부모를 쓰고 아직 자기 BP에서 같은 니아가라
  파라미터를 굴리므로, 부모에 넣었으면 이중 구동이 됐다 — 그래서 서브클래스).
  함정 2건: **죽은 노드를 "연결된 노드"로 판정하면 안 된다**(exec 선만 끊긴 체인이 Tick의
  `DeltaSeconds` 데이터 핀에 아직 물려 있어서 249개 중 230개가 살아있는 걸로 나옴 — exec
  도달성 + 데이터 생산자 역추적 2단계로 해야 4개가 나온다), **BP의 기본 float은 실제로
  double**(C++를 `float`로 잡았더니 ABP가 타입 불일치로 컴파일 실패 — BP가 핀으로 직접 읽는
  프로퍼티는 `double`이어야 함). 컴포넌트 22개·차량 튜닝 값 전부 보존 확인, ABP 캐스트가
  새 C++ 부모로 정상 해석됨, PIE에서 네이티브 Tick 포탑 추종 확인.
  과열 연기 실사격 확인과 실주행 확인은 남음. `vehicle/ugv/2026-09-03_ugv_0901_bp_to_cpp.md`.

## 2026년 9월 3일 ~ 5일 — 드론 교전 관측 이동

교전이 시작되면 드론이 **경로 스플라인 위에서 "전황을 가장 잘 보여주는 지점"으로 스스로
이동**하는 시스템. 짐벌은 각도만 바꿀 뿐 거리를 못 줄여서, 교전이 2·3차 전투지로 옮겨가면
"전원이 담기긴 하는데 아무것도 분간이 안 되는" 그림이 남던 문제. 상세:
`vehicle/drone/2026-09-05_drone_engagement_observation.md`, 동작 레퍼런스는 같은 폴더
`drone_flight_dev_guide.md` 16절.

- **09-03** 관측 지점 선정 구현. 처음엔 "내가 정한 이상적 거리·고도에 가까운 곳"이라는 대리
  지표로 짰다가 사용자 지적으로 **실제 프레이밍 결과**(필요 화각·조준 부감)로 갈아엎음. 이어서
  연속 최적화가 3분간 25m씩 21회 기어가는 래칫을 만들어, **제약 만족**(조건을 만족하는 동안은
  움직일 이유가 없다) 방식으로 재설계.
- **09-03** 프레이밍 대상을 `TargetDetection`에서 분리 — 탐지 결과를 입력으로 쓰면
  `줌아웃→탐지해제→프레이밍축소→줌인` 순환 의존이 생긴다. 레지스트리 직결로 전환.
- **09-04** 유도 루프 구조 결함 3연쇄 해결. 적분 와인드업 → 그 수정이 축별이 아니라 3차원
  전체를 얼리던 것 → **속도 피드포워드 부재로 인한 ζ=0.51 저감쇠 진동**(주기 6.8초). 경로
  추종이 원래 부드러웠던 건 거버너 속도를 쓰는 이미 피드포워드 구조였기 때문이고, 관측만
  순수 위치 피드백이었다.
- **09-04** 겉보기 크기 필터가 **화각에 반비례**해 교전 광각에서 탐지 사거리를 800m→45m로
  무너뜨리던 버그. 폰의 미러 프로퍼티가 BP 컴포넌트 설정을 매 플레이마다 덮어쓰고 있었다.
- **09-05** 카메라와 기체 방향 분리(짐벌 요는 원래 무제한이라 기수가 피사체를 향할 이유가
  없었다), 도주 중인 적 트래킹 제외, 3차 전환을 시나리오 스텝이 아니라 월드 상태로 게이트.
  **실동작 확인 완료.**

## 2026년 9월 10일 — UGV 서스펜션 승차감 튜닝

`New_kadex_0811`의 작은 바위 액터들에 콜리전을 켜서 주행 중 자연스러운 덜컹거림이 생기게 한
뒤, 충격이 너무 날카로워서 서스펜션을 다시 잡은 작업. 상세:
`vehicle/ugv/2026-09-10_ugv_0901_suspension_tuning.md`.

- 09-02 교체 때의 서스펜션 값은 **휠 개수 스케일 규칙(× 16/6)으로 기계 환산한 것**이라
  승차감을 본 적이 없었다. 승차감 기준으로 다시 잡아 `SpringRate 900 → 200`,
  `SuspensionMaxRaise 12 → 16`, `SuspensionSmoothing 0 → 5`.
- 구조 규명: **Chaos 5.8은 서스펜션을 힘이 아니라 PBD 컨스트레인트로 푼다.** 차체 운동
  (`FPBDSuspensionConstraints`)·그립(`FSimpleSuspensionSim`)·바퀴 비주얼(`GetSuspensionOffset`)이
  서로 다른 코드이고 같은 프로퍼티를 다르게 해석한다. 힘 기반 경로
  (`AddForceAtPosition`)는 `p.Vehicle.DisableConstraintSuspension` 전용이라 이걸 읽고 판단하면
  틀린다.
- **죽은 값 2개 발견.** `SpringPreload`는 솔버에서 주석 처리돼 있어 UE 5.8에서 어떤 경로로도
  동작하지 않는다(09-02에 180 → 450으로 환산한 건 효과 0). `RollbarScaling`은 축이 **휠 클래스
  기준**으로 묶이는 탓에 6륜이 한 축이 되고, 롤바 코드가 `축당 휠 2개`만 처리해서 스킵된다 —
  **이 차량은 안티롤바가 0**이다.
- `MaxRaise`와 `MaxDrop`이 대칭이 아니라는 것도 확인. 압축량이 `MaxDrop` 기준으로 계산되므로
  **MaxRaise는 차고를 안 바꾸고 "바닥 치기 전 여유"만 늘린다.** 기존 12cm는 12cm 넘는 돌에서
  트래블이 끝나 충격이 감쇠 없이 차체로 직행하던 상태였다.

## 2026년 9월 10일 — 드론 수동 조종 비행/짐벌 분리

수동 조종이 `UAV` 하나로 비행+짐벌을 한 덩어리로 넘겨받던 걸 **두 축으로 분리**해, "기체는
알아서 날고 사람은 카메라만 돌려본다"(`UAVGimbal`)를 가능하게 했다. 전시에서 가장 많이 쓰이는
조합. 상세: `vehicle/drone/2026-09-10_drone_manual_control_split.md`, 동작 레퍼런스는
`drone_flight_dev_guide.md` 17절.

- 폰의 수동 상태를 bool → 모드 enum(`None/GimbalOnly/Full`)으로. 효과를 거는 쪽에서는 **비행/짐벌
  두 질문으로 나눠서** 본다 — 뭉뚱그리면 "짐벌만 수동"이 비행까지 멈춘다.
- 함정 넷: 짐벌 자동 로직 안에 **비행 몫(관측 지점 선정)이 섞여 있던 것**, 정찰 단계 Idle 가드가
  교전 중 짐벌 입력을 통째로 버리던 것, **짐벌 버튼이 비행 IMC 안에 있어** "버튼 켰는데 무반응"
  (상태 기계 로그는 멀쩡해 보여 헤맴), 같은 물리 축을 비행·카메라 두 용도로 쓰며 생긴 극성 왕복.
- 곁가지로 **수동 해제 시 출발 위치로 576km/h 역주행**하던 버그를 잡았다. 자율비행 재진입을
  "지상에서 새로 시작"으로 오판해 이륙 단계로 들어가고, 이륙 속도 상한을 스플라인 첫 포인트까지의
  거리에서 역산한 결과. 교전 관측 중이었을 땐 **관측 상태까지 잃고 경로 끝으로 주행**하는 두 겹
  버그였다(사용자가 "교전 트래킹에도 같은 버그 있을 수 있다"고 짚어줘서 발견).
- 짐벌 기본 자세 복귀 추가. 사용자가 먼저 제안한 "스켈레탈 메시 본을 돌려 기본각을 바꾼다"는
  누적 오프셋 구조상 복귀 자체를 못 만들어서, 코드 파라미터로 넣었다.

## 2026-09-10 — Graphics 설정 탭 구현 (09-03 조사에 이어)

상세: `ui/2026-09-10_graphics_settings_implementation.md`(구현·WBP 계약·함정 8건),
설계 근거는 `ui/graphics_settings_analysis.md`.

- **런타임 품질 변경이 처음으로 가능해졌다.** 그 전까지 `sg.*` 12개가 `WindowsEngine.ini`의
  `[ConsoleVariables]`(`SetBySystemSettingsIni`)에 박혀 있어 `Scalability::SetQualityLevels`
  (`SetByScalability`)가 조용히 거부됐고, 품질 변경이 **완전 no-op**이었다. 신규
  `UTitanGraphicsSettings`(`UDeveloperSettings`, `Config/DefaultGame.ini`)로 이관 →
  `DumpCVars sg.` 실측에서 12개 전부 `Scalability`로 풀림, 거부 경고 12줄 → 2줄.
- **단일 소스 구조** — `defaultconfig`라 P4 공유 + 패키징 포함이고, 에디터·게임이 같은 파일을 읽어
  "에디터는 A인데 패키지는 B"가 구조적으로 불가능해진다(엔진 기본은 에디터/게임이 서로 다른 ini를
  읽어서 갈라진다 — `LaunchEngineLoop.cpp:2866`). 개별 Lumen/VSM 튜닝은
  `Config/DefaultScalability.ini`의 프리셋 재정의로 옮겼다(엔진 프리셋 위에 키 단위 병합이라
  덮을 키만 적으면 되고, 현재 쓰는 단계만 채우면 기본 화면이 동일하다).
- **플랫폼 불일치 2건 해소** — 반사 방식이 Windows=SSR(범위 밖 값 3) / Linux=Lumen으로 갈려 있던 것을
  SSR로 통일, Lumen 원거리 GI 튜닝이 Windows 전용 ini라 **Linux 납품 빌드에 아예 빠져 있던 것**을
  공통 ini로 올렸다. 둘 다 **Linux 룩이 바뀌므로 패키지 실측 필요.**
- **초목 품질 항목은 제거** — `sg.FoliageQuality`가 이 레벨에서 효과가 0이었다(나무가 PCG로 뿌린 뒤
  레벨에 구워진 ISM/HISM이라 `pcg.Quality`는 런타임 생성 PCG만 갱신). 대신 실제로 성능을 지배하는
  **WPO 거리·LOD 배율을 숫자 입력으로** 노출했다 — 실측 곡선이 있는 연속량이라 4단계로 쪼개면
  정보를 버린다는 사용자 지적을 받아 드롭다운에서 `USpinBox`로 바꿨다.
- 겪은 함정 8건은 문서 §7에. 특히 **액터 존재 여부로 UI를 비활성화하면 안 된다**(차량이 없는
  `kadex_lobby`가 정작 설정을 하라고 만든 화면이라, 거기서만 드론·전장 항목이 비활성이 됐다) /
  **Overlay에서 Fill은 겹침**(컨트롤이 라벨을 덮거나 쪼그라듦 — HorizontalBox + SizeBox로 재설계) /
  `UDeveloperSettings` 상속 시 `Build.cs`에 모듈 명시 필요(컴파일은 통과하고 링크에서만 터짐).
- **빌드·WBP까지 완료, 동작 확인됨** — `WBP_GraphicsRow`를 HorizontalBox + SizeBox로 재구성하고
  `WBP_GraphicsSectionHeader`를 신설한 뒤 `[GameSettingsWidget] Graphics 탭 23개 항목 생성`
  (헤더 5 + 항목 18) 로그 확인. 라벨/컨트롤 겹침·잘림 없음.
- **게임 레벨 대상 탐색까지 확인** — `New_kadex_0811`에서 `QuadCam 2개 / Drone 1개 / Truck 1개`,
  `ISM/HISM 50개(인스턴스 94936)`. 기대값으로 적어둔 `17개/58,400`과 안 맞아서 **에디터에서 소유
  액터를 전수 확인**했더니 50개 전부 숲(`BP_SplineForest_*` 10개 + `TreeCollisionProxyBuilder` 1개)
  소유였다 — 옛 인벤토리가 plant 액터 2개 시절 숫자였고 그 뒤 8개로 늘어난 것. 무관한 인스턴스
  메시가 섞인 건 없다(프록시 4개는 `bHiddenInGame`이라 렌더 자체를 안 하고, 같이 잡히는 작은 바위
  2종은 plant PCG가 고사리와 함께 뿌리는 숲 스캐터). 상세: 구현 문서 §8-1.
- **남은 것**: VSync는 패키지에서만 검증 가능, Linux 패키지 룩 확인, 나무 WPO/LOD 값 변경이
  실측 fps 곡선대로 움직이는지 확인.

## 2026-09-15 — 데모 모드 UGV RCWS 자동정찰 시작 시점을 1차 목적지 도착으로

데모 모드에서 UGV 포탑이 출발도 하기 전(레벨 시작 1초 뒤)부터 탐색 스윕을 돌던 것을, UGV가
1차 목적지에 도착한 뒤에 켜지도록 옮긴 작업. 상세:
`level_new_kadex_0811/2026-09-15_demo_ugv_autofire_on_zone1_arrival.md`.

- 원인은 `ApplyDemoRCWSAutoFire()`(레벨 시작 +1초)가 UGV·지휘소 둘 다 ARM+AutoFire로 강제한 것.
  AutoFire는 타겟이 없으면 `UpdateSearchSweep`을 돌리므로 정지해 있는 UGV가 정찰하는 그림이 됐다.
  DT의 `UGVSurveillance`/`UGVAutoFire` 행은 꺼져 있어 무관.
- **새 이펙트 `SetDemoUGVAutoFire`** — 데모 게이트(`IsDemoMode() && bDemoForceUGVAutoFire`)가
  이펙트 안에 있어 FullSystem과 같은 DT를 써도 행을 켜둘 수 있다(`SetUGVAutoFire`는 모드를 안 봐서
  통제기와 충돌). 레벨 시작 강제는 이제 **이동형지휘소만**, UGV 분기는 삭제.
  람다를 `ApplyDemoRCWSAutoFireTo(Owner, Label)` 멤버로 추출해 두 경로가 같은 코드를 쓴다.
- **새 DT 행 `UGVArriveZone1`** — Prereq `UAVSpotted`, `ActorStopped`. `MoveToDestination`이
  호출 안에서 동기적으로 `bIsMoving=true`를 세우므로, `UAVSpotted` 이후 첫 `!IsMoving()` 틱이
  곧 도착이다. 경로 실패 시엔 즉시 발동(RCWS는 어쨌든 ARM되므로 허용).
- 동작 변화: 1차 목적지로 주행 중엔 UGV가 쏘지 않고, `EnemyEngage`·`DroneSeeEnemies` 체인이
  도착 이후로 밀린다. 되돌리려면 그 행을 Prereq 없음 + `TimerOnly` 0초로(코드 수정 불필요).
- 작업 당일엔 새 enum이 에디터에 아직 없어 그 행의 `EffectType`이 `None`이었다 — 같은 날 빌드 후
  MCP로 `SetDemoUGVAutoFire`로 설정·DT 저장. 재시작 설계(09-10)에는
  "UGV 리셋 시 RCWS 모드를 `Remote`로 되돌릴 것" 메모를 추가했다(안 하면 2회차부터 출발 전 스윕).
- **09-16 확인 완료** — PIE에서 도착 전 정지, 도착 후 스윕 시작 "잘됨". 문서 상태 완료로.

### 같은 날 — RCWS 탐색 스윕 고각을 차체 기준으로(내리막에서 하늘 보던 문제)

현장 피드백 "자동정찰 중 내리막길에서 포탑이 너무 위를 본다". 상세:
`rcws/2026-09-15_search_sweep_hull_relative_elevation.md`.

- 원인은 `UpdateSearchSweep`이 고각 목표를 `CurrentData.ElevationDegrees == 0`으로 잡은 것 —
  이 값은 2026-07-20부터 `RefreshAzimuthElevation`이 **월드 수평 기준**으로 계산하므로, 내리막에서
  차체가 기울면 마운트를 차체 대비 위로 들어 수평을 유지 → 하늘. 좌우 스윕은 원래 차체 기준.
- 새 프로퍼티 `SearchSweepElevationDegrees`(기본 **-3°**, 차체 기준, 음수 = 아래). 1차 구현(CL 476)은
  오차를 `SearchSweepElevationDegrees - 마운트 relative pitch`로 잡았는데, 트럭 BP의 조준 카메라가
  마운트 아래 자체 pitch -6.1°를 갖고 있어 트럭이 -9.1°를 보는 문제가 나와 같은 날 기준을
  **조준 카메라의 차체 상대 pitch**로 정정. AutoSurveillance·AutoAim/AutoFire 무표적 양쪽, UGV·지휘소 공통.
- 안정화(기본 OFF)를 켜면 이 목표와 싸우지만 좌우 스윕도 원래 그랬던 기존 구조라 안 건드림.
  같은 파일이 `user2_jiseong`에게도 체크아웃돼 있어 서브밋 시 머지 가능성.
- **09-16 확인 완료** — 풀 빌드 후 UGV·TitanTruck 모두 정상. **함정**: 처음 Live Coding으로 빌드했을
  때 `BP_TitanTruck::UpdateTurretVisuals`에서 `missing property 'BarrelSpinGaugeValue'`가 뜨며 총열이
  안 돌았다 — 코드 버그 아님, 새 UPROPERTY가 든 클래스를 Live Coding이 BP에 반영 못 한 것. 에디터
  닫고 풀 빌드 → 재시작 → BP 컴파일·저장으로 해결. `guide/rcws_fire_control_dev_guide.md` §8.4 갱신함.

---

## 2026-09-15 — 리눅스 패키징 `Cook failed` 원인 규명(MCP 8000 포트 충돌)

에디터에서 Linux Package Project가 3회 연속 `Cook failed`(ExitCode=25)로 끝났는데, 쿡은
2055/2055 `Done!`까지 돌았고 에셋 에러는 0건이었다. 상세:
`packaging/2026-09-15_linux_cook_failed_mcp_port_clash.md`.

- 커맨드릿은 `GWarn->GetNumErrors()>0`이면 종료 코드 1(`LaunchEngineLoop.cpp`) — Error 로그
  **1줄**이면 쿡 실패. 그 한 줄은 `LogHttpListener: Error: HttpListener unable to bind to
  127.0.0.1:8000`이었다. 에디터 환경설정 Auto Start Server(unreal-mcp)가 켜져 있어 쿠커 프로세스도
  같은 포트에 MCP 서버를 띄우려 한 것. 09-02 이후 켜진 사용자 설정이라 전에는 안 났다.
- 오진 2건: 매번 찍히는 `CDO Constructor (UGVChaosPawn)` Error는 별도 피드백 컨텍스트라 카운트
  안 됨(원인 아님, `UGVChaosPawn.cpp`는 `FObjectFinderOptional`로 정리만). `DefaultGame.ini`에
  넣어본 `AdditionalCookerOptions=` 키는 존재하지 않아 패키징 1회 낭비.
- 해결: `DefaultGame.ini`에 `ProjectCustomBuilds` "Package Linux (MCP 8000 회피)" 추가 — 표준
  Package Project 커맨드 + `-additionalcookeroptions=-ModelContextProtocolPort=8001`. UAT
  `-PrintOnly` 드라이런으로 커맨드라인 동일성 확인. 에디터 재시작 후 Platforms ▸ Project Custom
  Builds에서 실행. 기본 Package Project는 MCP 서버가 떠 있는 한 여전히 실패.
- 문서: 패키징 가이드 §2 정정, `guide/mcp/unreal-mcp-claude-code.md` 주의사항 추가.

## 2026-09-15 — LIG RTSP 접속 실패 원인 확정(드라이버 595 vs NVENC SDK 13.1) → SDK 13.0.37로 내림

09-02 전달 리눅스 패키지가 LIG PC에서 RTSP DESCRIBE 후 20초 타임아웃. 한동안 IP 문제로 오인했으나
스크린샷 시간순 대조로 "DESCRIBE는 도달, 인코딩 프레임 0장"을 확정. 상세:
`rtsp/2026-09-15_lig_rtsp_describe_timeout_analysis.md`.

- 원인: RtspEncoder가 vendoring한 NVIDIA Video Codec SDK 13.1.15는 드라이버 **610+** 필수
  (`NvEncoder::LoadNvEncApi`가 드라이버 최대 API 버전과 비교해 throw). LIG PC는 595.84이고
  방산 안정성 사유로 업데이트 거부. 개발 PC(610.88)에선 재현 불가. 전달 가이드에 드라이버
  최소 버전이 없었고 예시가 `nvidia-driver-550`이었음.
- 진단을 방해한 코드 결함: `URtspStreamComponent::SetupEncoderAndStream`이 마운트 등록 → 인코더
  생성 순서라 인코더 실패 시 마운트가 남아 클라이언트가 404 대신 20초 타임아웃(gst-rtsp-server는
  라이브 appsrc 첫 버퍼가 와야 SDP 완성 — `rtsp_poc_findings.md` §1.14와 같은 메커니즘).
- 수정: (1) SDK **13.1.15 → 13.0.37**(드라이버 570+), `ThirdParty/NvCodec/` 9개 파일 교체 후 UE
  통합 패치 재적용, 호출 API 시그니처 동일해서 인코더 코드 무수정. (2) 인코더 먼저 → 성공 시에만
  `RegisterStream`, 실패 시 즉시 404 + 원인 Error 로그. (3) 실행 가이드에 드라이버 ≥570
  요구·확인·증상표 추가, Vulkan ICD 확인을 `/usr/share`·`/etc` 두 경로로.
- 검증: 사내 리눅스 PC(Ubuntu 22.04, RTX 4070 SUPER)를 NVIDIA 공식 `.run`으로 LIG와 동일한
  **595.84로 내려서** 새 패키지 실행 → RTSP 5스트림 정상. `.run` 설치본은 ICD를
  `/etc/vulkan/icd.d/`에 넣는다는 점 확인. 절차·원복은
  `packaging/2026-09-15_linux_nvidia_driver_595_run_install.md`.
- 남은 것: LIG에 새 패키지 재발송(메일 초안은 "드라이버 업데이트 요청" 문구를 빼고 "595.84 대응
  빌드 재전달"로 수정 필요).

## 2026-09-15 — 리눅스 실행 스크립트에 세션 자동 판별 내장(`Config/BootstrapPreamble.sh`) + 실행 가이드 0915판 + RTSP 전송 방식 정정

09-04에 만든 `run_titan_example.sh`(Wayland/X11 자동 판별 래퍼)는 UE 패키징이 자동으로 넣어주지
않아 매번 결과물 폴더에 손으로 복사해야 했다. UAT의 `LinuxPlatform.Automation.cs`
`StageBootstrapExecutable`가 `<Project>/Config/BootstrapPreamble.sh`가 있으면 그 내용을 생성
`titan_example.sh` 맨 앞에 끼워 넣는다는 것을 확인하고, 같은 로직을 그 파일로 옮겼다.

- 동작: 사용자가 `-sdlvideodriver=`를 안 줬고 `WAYLAND_DISPLAY`가 비어 있으면
  `set -- -sdlvideodriver=x11 "$@"`; `WAYLAND_DISPLAY`가 있으면 네이티브 Wayland; 명시 인자는 항상
  존중. 생성 스크립트가 `#!/bin/sh`(dash)라 POSIX sh만 씀.
- 검증: stage-only UAT(`BuildCookRun -skipbuild -skipcook -stage`)로
  `Saved/StagedBuilds/Linux/titan_example.sh`에 `### Added from project Config BootstrapPreamble.sh`
  마커 블록 삽입, LF 줄바꿈, `sh -n` OK, `sh`로 실행 시 판별 메시지 출력 후 리눅스 바이너리 도달.
  같은 날 "Package Linux (MCP 8000 회피)" 커스텀 빌드로 전체 패키징 성공.
- 결과: 받는 쪽은 `./titan_example.sh`(+ `-fullscreen` 등)만 실행하면 된다. 프로젝트 루트의
  `run_titan_example.sh` / `titan_example_x11_fallback.sh`는 레거시로 남김(같이 있어도 무해).
  주의: `BootstrapPreamble.sh`가 체크아웃에 없으면 패키징이 조용히 블록 없는 스크립트를 만든다 —
  결과물의 `titan_example.sh`를 열어 확인하는 항목을 가이드에 넣었다.
- 문서: **`packaging/kadex_0915_패키징_실행가이드.md` 신규** — 받는 쪽(Ubuntu) 실행 절차만 담은
  배포용 문서, 09-02 가이드를 대체·폐기 표시(프리앰블 기준 실행, 드라이버 ≥570/595.84 실증, `.run`
  주의, RTSP TCP/UDP 정정). 패키징(우리 쪽) 절차는 배포물에 넣지 않고 내부 가이드
  `2026-09-02_linux_package_ugv_host_rc_test_guide.md` §2에 모음 — §2-4 프리앰블, NVENC SDK 13.0.37
  경로 사전 확인, **§2-5 성공 확인 체크리스트**(쿡 로그 8001 / `[RtspEncoder]` 경고 없음 / 출력 폴더
  구성 / 스크립트 마커 블록 / 동봉물), **§2-6 패키징 절차 변경 이력** 신설. §3-2/§3-4/§7 정정,
  `rtsp/linux_wayland_x11_present_bottleneck.md`·`rtsp/RTSP_Perf_Investigation.md`에 09-15 추기.
- **RTSP 전송 방식 정정**: 사용자 확인으로 "TCP interleaved / UDP 둘 다 됨"이 맞음(서버 코드에
  `set_protocols` 류 제한 호출 없음 → gst-rtsp-server 기본값). 내부 가이드 §1/§3-3/§5/§7,
  `rtsp/rtsp_client_reception_guide.md` §1.1/§1.2, `protocol/protocol_icd.md` §3.3/§4.1,
  `CURRENT_STATE.md` §3의 "TCP만/UDP 미지원" 문구에 정정 주석. TCP 권장 근거(저지연 검증 기준; UDP는 RTP/RTCP 포트가 접속 시 동적 협상이라 방화벽
  환경에선 8554만 열면 되는 TCP)를 방화벽 절마다 명시.

## 2026-09-15 — SoldierLab 병사 체력 · 피격 반응 · 사망 구현 (병사가 죽는다)

`soldier_ai_lab` 의 새 병사에게 데미지를 *받는* 코드가 0줄이던 것을 C++ 컴포넌트 하나로 채운
작업. 전날(09-14) 조사·추천 문서의 (C-2) 안 그대로. 상세:
`soldier_ai_lab/ai/2026-09-15_health_hit_death_implementation.md`(조사·추천은 같은 폴더의
`2026-09-14_hit_death_health_recommendation.md`, `drafts/` 에서 올라옴).

- **`USoldierHealthComponent`**(`Source/SoldierLab/AI/SoldierHealth.h/.cpp`, 신규 UCLASS) — 표준
  `OnTakePointDamage`/`OnTakeRadialDamage` 를 서버에서만 바인드(투사체의 `ApplyPointDamage(34)` 는
  무변경) · 부위 배율(head 2.5 · neck 2.0 · 팔 0.6 · 다리 0.75) · 캡슐 피격이면 메시 트레이스로
  본 해석 · `bInvincible`(체력만 안 줄고 움찔은 함) · HitReact 애디티브 몽타주 13 을 방향×세기로
  (`bStopAllMontages=false` 라 재장전을 안 끊는다 — 슬롯 그룹은 안 갈랐다) · Death 몽타주 6 →
  끝 0.1 s 전 래그돌(pelvis 이하, 속도 관성 + **다음 틱** 임펄스 1500 — titan 의 Chaos 실측 그대로)
  · 사망 시 등록부 Unregister(8 소비자 정리) + `/Script/SoldierLab` 컴포넌트 전부 틱 off + CMC/
  AIController 정지 + ABP 축 변수 0 · 8 s 후 시체 틱 정지 · `Health/bDead/LastHit` 복제, 연출은
  서버·클라 같은 코드 경로. 콘솔 `SoldierLab.Debug.Health 1` / `SoldierLab.Invincible 1`.
- ABP `SoldierCharacter_ABP` 에 `IdentityPose → Slot 'AdditiveHitReact' → ApplyAdditive` 3노드(재장전
  애디티브와 같은 모양). `BP_SoldierCharacter` 에 `AC_SoldierHealth` + Tick 의 총구 보정 게이트를
  `AND(기존, NOT IsHitReacting)` 로. **아군은 `BP_Soldier_Friendly` 의 `Invincible (무적)` 체크**(사용자
  결정 — 시나리오상 아군은 죽을 필요 없음).
- 사용자 PIE 확인 "잘됨". 수치는 하나도 안 쟀다 → [C-110]~[C-118](본 None 빈도 · 루트모션 ·
  정착 프레임 · 임펄스 · 45구 비용 · 게이트 실효).
- 함정 3건 → `soldier_ai_lab/CLAUDE.md` P127~P129: ① BP 컴포넌트 템플릿이 C++ CDO 의 배열 기본값을
  안 물려받는다(`RF_ClassDefaultObject` 가드 안이면 템플릿에 없음 — 가드 제거, **다음 빌드 대기**)
  ② `set_properties` 가 컴포넌트 템플릿에 **썼다**(P53 정정 #2; 자식 BP 오버라이드는 여전히 불가)
  ③ `Math|Boolean|ANDBoolean`/`NOTBoolean` 은 만들어진다(P33 은 산술 한정). 덤: 에디터 켠 채 빌드하면
  "성공"인데 DLL 이 안 바뀐다(P13 재확인 — `search_subclasses` 로 판정).
- 문서: `IMPLEMENTED.md` 0/2.4/3/4/5.2/6절 · `OPEN_ITEMS.md`(~~[W18]~~, +C-110~118 · W60~63 · Q46~47 · R8)
  · `CURRENT_STATE.md` · 핸드오프 2.8절(피격·사망 몽타주 19개 표). `guide/` 에는 병사 문서가 없어 무변경.
  ⚠ 같은 시각 다른 세션(급선회/총내림)과 ID 를 동시에 잡아 빈 번호가 생겼다(C-108/109 · W55~59 ·
  Q44/45) — `OPEN_ITEMS.md` 머리말.

## 2026-09-15 — 드론 짐벌 2축 안정화 + 2대 PC 실환경 첫 검증(버그 3건)

상세: `vehicle/drone/2026-09-15_drone_gimbal_stabilization.md`,
`replication/2026-09-15_drone_two_pc_validation.md`. 동작 레퍼런스는 `drone_flight_dev_guide.md`
12.4절·15절.

- **짐벌 안정화.** 낙하산 순항 때 가감속 기울기가 카메라에 그대로 실려 보였다. 리그가 `CamYaw`/
  `CamPitch` 본뿐이라 롤은 상쇄 안 하고(사용자 확인) 요·피치만 — 짐벌 각도의 **기준 프레임을
  기체 → 수평 프레임**으로 바꾸고 본 각도는 매 틱 역변환으로 새로 뽑는다. 구현 전에 분석해보니
  각도를 만드는 곳은 자동 추적/스윕의 `GetActorQuat()` 변환 2곳뿐이라, 거기를 같은 프레임으로
  통일하면 추적 로직은 한 줄도 안 바뀐다(안 맞추면 추적기가 가짜 오차를 쫓아 더 떨린다). 곁가지로
  CineCamera의 니어플레인이 씬캡쳐(위젯/RTSP)엔 복사 안 되던 것 수정.
- **2대 PC 실환경 검증 — "전혀 리플리케이션 안 됨".** 09-01 이후 처음 돌린 진짜 2 PC. 양쪽 로그를
  받아 원인 셋을 갈랐다:
  - 데모 모드: 판정 순서가 "자체방호면 true"가 "데모면 서버"보다 먼저라 **서버·클라 둘 다 주체**.
    09-01 문서에 "판정이 안 갈린다"고 써둔 게 코드와 달랐다. 데모 검사를 선행시키고 클라는
    GameState 도착을 잠깐 기다린다.
  - 풀 시스템: 서버 로그도 클라 로그도 전부 정상인데 `Server_ReportState`만 서버에서 안 돈다.
    엔진 소스를 따라가 보니 **레벨 배치 폰의 엔진 기본 `AutoPossessAI=PlacedInWorld`**로 AI
    컨트롤러가 빙의해 있었고, `APawn::GetNetConnection()`은 Owner보다 Controller를 먼저 봐서 null →
    서버가 `bNetOwner` 불일치로 RPC를 **Verbose 로그로만** 폐기. `SetOwner`는 멀쩡히 됐는데도.
    `AutoPossessAI=Disabled` + `GetNetConnection()` 오버라이드(Owner 사슬만).
  - 데모 + 클라: 위 둘을 고치니 클라가 정상적으로 원격이 됐는데 드론이 출발점에 굳음. 서버가
    주체일 때 Rep*를 채우는 곳이 **RPC 구현 하나뿐**이라 아무도 안 쓰고 있었다 — 서버 Tick에서 직접
    게시. 이 조합은 원래 코드에서 한 번도 안 만들어진 경로.
  - 진단 로그 2종(클라 첫 송신 조건 / 서버 수신 횟수)을 남겨 다음엔 로그만으로 갈리게 했다.
    최종 로그: 서버 `수신 191회`, 클라 `Owner=PC, 액터채널=있음`. 풀/데모 양쪽 사용자 확인.
- 사용자 질문 "왜 주체가 모드별로 다른가"에 답한 내용을 문서에 정리 — 풀은 조종 지연(클라
  권위), 데모는 클라 없는 1 PC 구성(서버 권위), 접속 여부가 아니라 모드로 판정하는 건 핸드오버를
  피하기 위해.

## 2026-09-16 — RCWS 성능 업그레이드: 타겟 기억(경계도)+마지막 위치 응시+부위 기반 탐지/조준

사용자 요청 "UGV/TitanTruck RCWS 성능 업그레이드". 상세:
`rcws/2026-09-16_rcws_target_memory_and_body_part_aim.md`. 동작 레퍼런스는
`guide/rcws_fire_control_dev_guide.md` §3/§3.2/§5, `guide/detection_dev_guide.md` §3.4 갱신함.
**코드·풀 빌드·인스턴스 설정 완료, PIE 1차 확인 — 세부 검증·튜닝 남음.**

- **현재 상태 분석** — 문제 둘. (1) 기억이 없다: 유예 1초 뒤 곧장 스윕 복귀라 적이 2~3초만
  엄폐해도 카메라가 딴 데 가 있음. (2) 탐지가 박스 중심 수직선 3점 중 2점(0.6/0.5) 규칙이라
  머리만 내놓은 적(1/3)은 영영 미획득, 조준점도 바운드 중심이라 엄폐물에 박힘. Lean/엎드림은
  캡슐이 안 따라가서 샘플이 엄폐물/허공.
- **탐지(`UTargetDetectionComponent`)** — 샘플을 수직선에서 **뼈**(`BodyParts` 기본
  Head/Chest/Pelvis, Mixamo/UE5 마네킹 이름 후보 둘 다)로. `AcquireRule=AnyVisibleSample`이면 1점만
  보여도 획득(임계값은 시간 게이트로 남음). `ReacquireGraceSeconds`(10s) 안에 잡혔던 적은 즉시
  재획득. `FDetectedTarget`에 `VisibleFraction`/`Parts`(부위별 가시성) 노출.
- **조준(`URCWSFireControlComponent`)** — `AimPartPriority`[Chest, Pelvis, Head]로 **보이는 부위**를
  조준(현재 뼈 위치 재계산). 타겟별 `FRCWSTargetMemory`(경계도 0.5s 상승/15s 감쇠) → 표적 선정
  유효거리 가중(0.5, 전방 45° 우선은 상위 규칙 유지)·락온 충전 단축(×0.5). 유예 후 놓치면 대체
  타겟 없을 때 마지막 조준점을 **4초 응시**(`ERCWSAutoAimPhase::WatchingLastKnown`, 초기 7초 → PIE
  1차 확인 후 4초로 조정), 누구든 보이면
  즉시 추적, 만료 시 스윕 오프셋을 현재 방향에서 이어받아 블렌드 아웃. 사망 타겟은 응시 없이 즉시
  놓음(죽은 적 1초 물던 기존 동작도 제거). `AutoAimPhase` 리플리케이트.
- **빌드·PIE 1차 확인** — 에디터 닫고 풀 빌드(새 UPROPERTY 다수, Live Coding 금지 — 09-15
  `BarrelSpinGaugeValue` missing property 사고). PIE 1차 "잘 되는 것 같다" → 응시 시간
  `WatchLastKnownSeconds` 기본값 **7 → 4초**로 조정. 뼈 기반 `BodyParts` 기본값은 인스턴스에 생성자
  값이 그대로 전파된 것 확인.
- **`AcquireRule=AnyVisibleSample` 설정 완료(MCP)** — UGV(`BP_UGV_0901` 기본값 + 레벨 인스턴스
  `BP_UGV_0901_C_1`)·트럭(`BP_TitanTruck` 기본값 + `BP_TitanTruck_C_4`) 네 곳 전부, `BP_UGV_0901`·
  `BP_TitanTruck`·`New_kadex_0811` 저장. 부수 발견 둘: (1) BP에서 붙인 컴포넌트의 기본값 경로는
  `<BP>_C:TargetDetection_GEN_VARIABLE`, C++ `CreateDefaultSubobject` 컴포넌트는
  `Default__<BP>_C:TargetDetection` — 아키타입 경로 모양이 다르다. (2) BP 기본값을 맞추면 인스턴스 값이
  아키타입과 같아져 `.umap`에 델타 직렬화로 남지 않는다(grep 0건이 정상, BP `.uasset`엔 있음).
- **남은 것** — PIE 세부 검증(머리만 나온 적 획득·머리 조준, 재출현 즉시 재획득+락온 단축, 전방 45°
  우선 유지, 트럭에서도 동일), 튜닝(응시 4초/경계도 감쇠 15초/거리 가중 0.5).

## 2026-09-16 — UGV 자율주행: 횡경사 조향 보정 + 도착 방향 정렬 + Chaos 브레이크 ON/OFF 문제 (실주행 3회, 4차 검증 대기)

`vehicle/ugv/2026-09-16_ugv_slope_steering_and_arrival_heading.md`,
`vehicle/ugv/2026-09-16_ugv_chaos_brake_proportional_and_brake_steer.md`.

- **횡경사에서 옆으로 밀린 뒤에야 고치던 문제** — 요레이트 PI는 위치 오차(미끄러짐)를 못 본다. 강체 횡속도로
  슬립각 β를 재서 목표 방향에서 빼는 **사이드슬립 보상(A)** + `RightVector.Z` 기반 **롤 피드포워드(B)** 추가
  (`UGV AI|Slope Steering`). 실주행 로그상 B를 요레이트 적분이 반대 부호로 상쇄하는 흔적 → 코너링 느낌 달라졌으면
  B부터 꺼서 A/B.
- **도착 시 차체 방향·위치** — `MoveToDestinationFacing(Location, Yaw)` 신설. 최종 설계: 내비메시 경로 그대로 T까지 →
  경로 잔여 길이 기준 제동 곡선 → 마지막 1m 3km/h 크리핑(**절대 서지 않음** — 음수 적분 폐기·조향컷 해제·최소 스로틀) →
  10cm+정지거리 앞 정지 → 스로틀 0 제자리선회 2.5° → 0.3초 정착 후 잔차 ≤30cm/3°면 **XY·Yaw 스냅 확정**.
  시나리오 Config TargetPoint 4곳 회전 연동, 콘솔 `MoveUGVToScenarioPoint FormUp|Zone2|...` 추가.
- **폐기한 설계**: 목적지 뒤 12m 접근점 P — 2차 실주행에서 P가 도로 옆 숲 속 고립 내비메시 조각에 떨어져 partial 경로
  → 나무 사이 직진 → 충돌. "내비메시 밖 점은 장애물 유무를 알 수 없다". 방향은 전부 선회가 맡는다.
- **Chaos 브레이크 3중 함정(엔진 소스 확정)**: `bReverseAsBrake`가 ① 3.6km/h 이상에서 브레이크 입력을 크기 무관
  **1.0**으로, ③ 그 미만에선 **후진 기어+후진 스로틀**로 바꿈; ② `FSimpleWheelSim`은 브레이크>구동이면 구동토크를 통째로
  버려 **제동 중 스키드스티어 요가 0**. 커브 앞 급제동·코너 중간 2차 급감속·시케인 충돌 2회·도킹 후 후진 크리핑이 전부
  이것. 해결: `UUGVWheeledVehicleMovementComponent::UpdateState` 오버라이드로 자율주행 중 **비례 브레이크 모드**(Auto 진입
  시 on), `UUGVWheeledVehicleSimulation`에 **브레이크 조향**(안쪽 궤도 브레이크 몰아주기, cvar `p.UGV.SkidSteer.BrakeSteerBlend`).
  3차 실주행에서 비례 브레이크는 확인, 브레이크 조향은 게이트가 네트워크 예측 전용 멤버(`VehicleInputs`)라 안 걸렸음 → 수정.
- **커브 튜닝값(`CornerDecelMetersPerSecSq` 등)은 이 수정 전엔 실제 감속도와 무관했다** — 튜닝은 4차 이후에.
- 같은 날 CL485(user3)로 `New_kadex_0811.umap` UGV 경로·spruce_small 메시 변경됨 — 새 나무가 프록시 Species에 없으면
  내비메시 미반영 콜리전. 시케인 충돌 자리와의 관계 미확인.

## 2026-09-16 — `slomo` 배속 안 먹는 원인(물리 dt 클램프) → Chronicle 리플레이 플러그인 신설 + RtspAxisGate 크래시 수정

`infra_architecture/2026-09-16_slomo_physics_dt_clamp_investigation.md`,
`replay_chronicle/2026-09-16_chronicle_replay_plugin.md`, `rtsp/2026-09-16_rtsp_axis_gate_dangling_timer_fix.md`.

- **왜 `slomo 5`가 New_kadex_0811에서 무효인가** — `UWorld::Tick`(`LevelTick.cpp:1596`)이 TimeDilation을
  곱한 dt를 `FChaosScene::SetUpForFrame`(`ChaosScene.cpp:344`)이 `MaxPhysicsDeltaTime`(엔진 기본 1/30)으로
  잘라서. 실측 `/Game/test` ≈58fps(여유 2배, `slomo 2`까지만), `New_kadex_0811` ≈20~30fps(여유 0, 배속
  전무, `slomo<1`만 됨). MCP로 `Default__PhysicsSettings` 라이브 확인 + `DefaultEngine.ini` P4 31리비전 전부
  `bSubstepping`/`MaxPhysicsDeltaTime` 키 없음 → 물리 스텝 설정은 한 번도 손댄 적 없음. **드론 가이드 §10.2의
  "UGV 때문에 서브스테핑을 의도적으로 껐다"는 오독이라 정정**(ini 주석은 `t.MaxFPS=60` 근거). 후보 3개 비교
  (MaxPhysicsDeltaTime 상향은 서스펜션 25배 강성·제어 6Hz로 기각 / 서브스테핑 `MaxSubstepDeltaTime=1/55`는
  게임스레드 컨트롤러 6Hz 위험으로 보류 / 세트포인트 배속은 스코프 과다로 보류). **구현 안 함 — 진짜 요구는
  "6분 시나리오 반복 확인"이라 아래 리플레이 툴로 해결.**
- **`Plugins/Chronicle` 신설**(insung52, 에디터 전용, 게임 모듈 의존성 없음) — 엔진 Replay System(DemoNetDriver)을
  EUW 패널(`EUW_ChroniclePanel`, 로직은 전부 C++ BindWidget)로 감쌈: PIE 자동 녹화(`Auto`), 녹화/정지,
  `Saved/Demos` 목록·삭제, 재생(PIE를 대신 띄움), 타임라인 스크럽, 배속 0.5/1/2/5, 일시정지 시작, 카메라
  (드롭다운 텔레포트/Follow/Free), Tools ▸ Chronicle 메뉴. Rewind Debugger는 트랜스폼/포즈만 재생해 Niagara/
  사운드/UI가 안 나와 기각. **엔진 함정 12건**을 실측으로 잡음 — 핵심 4개: (1) Standalone PIE 녹화는 Multicast
  RPC 전부 누락(`World.cpp:9607` 넷모드가 데모 틱 밖에서 Standalone) → **Play As Listen Server 필수**;
  (2) `USlider::SetValue`가 `OnValueChanged`를 쏴서 스크럽 무한루프 → `OnMouseCaptureEnd`에만; (3) 첫 스크럽이
  심리스 트래블(`DemoNetDriver.cpp:2584`)로 일시정지/스펙테이터/뷰타겟 전부 리셋 → 매 프레임 재강제;
  (4) 일시정지 중 스펙테이터 이동 불가 → `bShouldPerformFullTickWhenPaused`(리플렉션)+`bExecuteWhenPaused`.
  사용자 가이드는 `Plugins/Chronicle/README.md`/`Chronicle_Guide.html`. **리플리케이션 사각지대 후보 발견**:
  적 낙하산(`SM_Parachute`) 숨김이 로컬이라 착지 후 녹화 시작 시 리플레이에 남음 — 2 PC 늦은 접속에서도
  같을 가능성, 미수정.
- **`FRtspAxisGate::ResolveLocalAxis` 크래시 수정**(게임 코드, P4 체크아웃, 빌드는 사용자) — 0.1초 폴링
  타이머가 raw `this` 캡처, 리플레이 스크럽 루프가 UGV를 재스폰하는 동안 발화해 `VehicleRtspBridgeComponent.cpp:29`
  ACCESS_VIOLATION. 정상 플레이에서도 BeginPlay 후 5초 내 오너 파괴 시 잠재. `Owner` 인자 +
  `CreateWeakLambda`, 호출부 5곳(UGV 브릿지/드론/AmbientFX/트럭/구 UAV).
- 문서: `replay_chronicle/` 폴더 신설(`CLAUDE.md` 폴더표 추가), 드론 가이드 §10.2 정정, RTSP 0817 문서 §5 추기.

## 2026-09-17 — Chronicle 리플레이로 잡은 리플리케이션 사각지대 4건(적군 애니/피격/사망/시나리오 누수) + 리플레이 전용 수정 3건

`replication/2026-09-17_enemy_anim_death_replication_gaps.md`,
`replay_chronicle/2026-09-17_replay_respawn_and_physics_proxy_fixes.md`. 09-16 저녁~17.
**전부 구현·빌드·리플레이 검증("잘됨"), 2-PC 실기 재검증만 남음.**

- **전제** — 리플레이 재생은 클라이언트 하나가 더 붙은 것과 같다(09-16 §4-2 가설). 그래서 리플레이에서 안 보이는
  건 자체방호 클라이언트에서도 안 보인다 — 이번에 4건에서 확인됨. 8월 감사(`replication_audit.md` §8)가 못 잡은
  이유: **`GaitTopSpeed`/`IsSprinting`은 08-25 gait 재설계에서 생긴 변수**라 08-11 복제 목록에 없었고, 사망
  코스메틱은 스코프 밖이었음.
- **(1) 적군이 클라에서 idle 포즈로 미끄러짐** — ABP `Speed = Clamp(V/GaitTopSpeed)*(IsSprinting?600:300)`인데
  `GaitTopSpeed`는 서버 전용 `UEnemyCombatComponent::TickComponent`(`:231` 게이트) 안에서만 써져 클라 CDO 0 →
  Speed 0. 아군은 BP EventTick이 로컬 계산이라 걷긴 했고 뛰기 앵커만 불일치. → `BP_Enemy_Base`/
  `BP_ThirdPersonCharacter` 두 변수 `Replicated`(MCP).
- **(2) 피격 흔들림 없음** — `TriggerHitReactionPhysics`가 서버 BP AnyDamage 체인에서만 불리고 스프링 적분도 권위
  게이트 뒤. → 권위+네트워크면 `Multicast_TriggerHitReactionPhysics`(Unreliable) → `…Local`(원 본문), 비권위
  틱에서 `TickHitReactionSpring` 실행(코스메틱이라 안전).
- **(3) 서서 죽고 총이 손 위치에 떠 있음** — 사망 BP 체인(래그돌/PhysicalAnimation/총 분리/Destroy) 전체가 서버
  전용, 클라엔 `IsDead`만. → `FEnemyDeathReplicationInfo{Serial, HitDir(NetQuantizeNormal), HitLoc, Bone,
  HitVel, ImpulseMag}`를 `ReplicatedUsing=OnRep_DeathInfo`로(컴포넌트 첫 `GetLifetimeReplicatedProps`);
  BP `SetIsDead`→`SetAimPitch` 사이에 `NotifyDeathForReplication` 노드 1개(서버는 값만 저장, 기존 체인 그대로);
  `OnRep`(비권위, Serial 1회)이 `ApplyDeathCosmeticsOnClient`로 같은 핀 값으로 재생, 라이플은 `CurrentRifle`이
  복제 안 돼 `GetAttachedActors()`로, 임펄스는 다음 틱. **RPC 대신 프로퍼티인 이유: 리플레이 체크포인트/늦은
  접속에서 상태가 복원됨.** 값 바꾸면 BP·C++ 양쪽.
- **(4) 리플레이 재생 월드에서 시나리오가 새로 발동**(로그 `시나리오 스텝 발동: EnemyFleeToZone2/3`, `[Drone]
  자율비행 시작`) — `UEngine::LoadMap`이 `BeginPlay` 뒤에 DemoNetDriver를 붙여 `GetNetMode()==NM_Client` 가드가
  BeginPlay 시점에 샘(`World.cpp:9607`). 스텝이 클라 복제본 `BeginMove/Flee`를 불러 gait 변수를 로컬로 덮음 →
  델타 복제라 안 돌아옴. → `IsReplayPlayback()`(`IsPlayingReplay`)을 `TickScenarioSteps`(매 틱)/
  `DemoAutoStartScenario`/`ApplyDemoRCWSAutoFire`에, `BeginMove/BeginEngageAtCurrentZone/BeginFlee`에
  HasAuthority 방어. 실기 2-PC엔 없는 구멍.
- **진단 cvar `Enemy.ClientAnimDiag 1`** — 비권위 프로세스에서 0.1초마다 C++ 입력/ABP 실제값/슬롯 가중치 한 줄.
- **리플레이 전용 (a) CDO 노출** — 위를 다 고쳐도 리플레이에서만 09-11 "사격→엄폐 미끄러짐" 재발. 진단 로그
  `ABP(ABP_Enemy_kadex2_C) … 슬롯 Fire=1.00` → `BP_Enemy_kadex` **CDO** AnimClass가 옛 ABP(인스턴스 15명만
  `_New`). 체크포인트가 레벨 배치 액터를 클래스에서 재스폰(`DemoNetDriver.cpp:3368`)해 인스턴스 오버라이드 소멸.
  CDO를 `_New`로. **교훈: 복제 안 되는 프로퍼티의 인스턴스 오버라이드는 리플레이에서 전부 CDO로 보인다.**
- **(b) UGV/드론 1초 주기 앞뒤 지터** — 물리 복제 `Default` 모드가 성긴 타겟 사이를 외삽하다 4m 하드 스냅.
  Chronicle `bPredictiveInterpolationInReplay`(기본 on): 재생 월드 프록시에 1초 스캔으로
  `SetPhysicsReplicationMode(PredictiveInterpolation)`. 실기 UGV 모드는 안 건드림(8월 미해결 항목 그대로).
- **(c) 스크럽 뒤 UGV가 레벨 원위치에 박혀 포탑만 돎** — 재스폰 직후 초기 `ReplicatedMovement` 1회가 물리 바디
  준비 전에 버려지고(`ActorReplication.cpp:304`), 주차 중이라 다시 안 옴. `BeginSnapWindow(1.5s)`(스크럽 완료/
  재생 시작)에서 0.25초마다 `GetReplicatedMovement()` 위치로 50cm 초과 시 텔레포트, 일시정지 중에도.
- **UI** — 드롭다운=액터 뒤 텔레포트, `Go/Follow`=SetViewTarget, `Free`=현재 시점 분리, 일시정지 시작, 끝에서
  Play=처음부터. 가이드 2파일 반영.
- P4: `EnemyCombatComponent.h/.cpp`, `ScenarioStateSubsystem.h/.cpp`, Chronicle 소스, `BP_Enemy_Base`/
  `BP_ThirdPersonCharacter`/`BP_Enemy_kadex` 체크아웃(미제출). 09-16분(Chronicle+RtspAxisGate)은 CL 486 제출됨.
- 문서: `replication_audit.md` §0-1 추기, 09-16 Chronicle 문서 상단 추기, 09-11 미끄러짐 문서 §9 추기.
  `guide/`엔 리플리케이션/적군 AI 에버그린 문서가 없어 갱신 대상 없음.

## 2026-09-17 — RCWS 청각 보조: 근처 적 총성 방향 조사

`rcws/2026-09-17_rcws_gunfire_hearing.md`. 동작 레퍼런스는 `guide/rcws_fire_control_dev_guide.md` §3.2,
`guide/detection_dev_guide.md` §2 갱신함. **코드 완료, 빌드·PIE 검증 전.**

- **요청** — 시각 `TargetDetection`만으로는 한계, "아무 타겟이 없을 때 근처에서 총소리가 들리면 그 방향을
  우선 자동 정찰". 확정: 반경 100m로 짧게, 시각 발견이 항상 우선(없을 때만 청각 보조), 아군 총성은 경계
  대상 아님, 엄폐물 뒤 총성도 그 방향을 봄.
- **조사** — 프로젝트에 `AIPerception`/`ReportNoiseEvent`/`PawnNoiseEmitter` 0건. 적군 사격은 서버가 부르는
  `NetMulticast` RPC 2개(호출부 4곳)의 `_Implementation`이 BP 이벤트(투사체/머즐/사운드)를 여는 구조라
  그 구현부가 곧 "쐈다"는 순간. UE AIPerception Hearing(옵션 B)은 컨트롤러/센스 설정만 늘고 덜 결정적이라
  기각, 자체 이벤트 버스(옵션 A) 채택.
- **버스** — `UDetectableTargetSubsystem::ReportGunfire(Location, Instigator, Faction)` →
  `RecentGunfire`(`FGunfireEvent`, 보존 10s/최대 128). 서버 전용·비복제·실제 오디오 감쇠와 무관한 결정적
  데이터. 보고는 `UEnemyCombatComponent::ReportGunfireToSubsystem`(두 멀티캐스트 구현부, `HasAuthority`
  게이트로 서버 1회, 위치 `GetFireLaneOrigin()`=소총 MuzzlePoint, Faction=Enemy). 버스트는 첫 발 1회만.
- **RCWS** — `ERCWSAutoAimPhase::InvestigatingGunfire` 추가, 우선순위 **시각 → 기억 응시 → 총성 조사 →
  스윕**(`UpdateAutoAim` `!Target` 분기, 응시 `return` 뒤). `PickGunfireToInvestigate`가 50m
  (`GunfireHearingRangeCm`, 100m → 같은 날 25m → 09-21 50m 확정)/3s(`GunfireMemorySeconds`) 안 가장 최근 적 총성(이미 조사한 것 제외, 도주 분대
  제외 플래그 준수)을 골라 `GunfireInvestigateSeconds`(4s) 동안 `SlewSightTowardWorldPoint` — LOS 검사 없음.
  새 총성이면 지점/타이머 갱신, 만료 시 `ResumeSearchSweepFromCurrentAim()` 블렌드 아웃. 사격자 경계도
  ≥`GunfireAlertnessFloor`(0.5), 단 `LastAimLocation`은 안 넣음 — `FRCWSTargetMemory::bHasLastAimLocation`
  신설로 총성만 아는 적이 응시 지점(원점)이 되는 일 차단. 로그 `근처 적 총성(NNm) → 총성 방향 4.0초 조사` /
  `총성 조사 종료 → 탐색 스윕 복귀`.
- **시나리오 영향** — 적은 `BeginEngageAtCurrentZone` 전엔 안 쏘므로 1차 교전 시작은 안 앞당겨짐. 실효는
  2·3차 이동/도주 분대 견제 사격, 엄폐 뒤 사격. `EnemyDetected` 트리거는 탐지 기반이라 무관.
- **남은 것** — 에디터 닫고 풀 빌드(새 UPROPERTY 5 + UENUM 값 + USTRUCT, Live Coding 금지), PIE 검증(총성 후
  스윕이 그 방향으로, 시각 우선, 아군 사격/50m 밖 무반응, 만료 후 블렌드 아웃), 튜닝(50m/3s/4s/0.5).
- **09-21 후속** — `L_SoldierScenario`(SoldierLab 적군)에서 "총성 조사가 안 된다" 리포트. 경로는 정상
  (`USoldierLabBridgeSubsystem`이 `OnGunshot`→`ReportGunfire`로 옮기고 있었음, 매 발 보고), 원인은 적 배치가
  UGV 1차 목적지에서 70~76m라 **25m 반경 밖**. 기본값 50m로 재조정(헤더 → 재빌드 필요). 확인 순서
  (로그 → 모드 → 거리 → 경로)를 devlog에 기록.

---

## 2026-09-17 — 게임플레이 태그 ini 누락: `BP_Soldier_Friendly` 배치 시 ensure

`soldier_ai_lab/migration/2026-09-14_titan_example_migration.md` §11(전문) · `soldier_ai_lab/CLAUDE.md` **P142**.
**해결·사용자 확인 완료.**

- **증상** — `BP_Soldier_Friendly`를 레벨에 처음 배치하면 `Ensure condition failed: !IsValid()`
  (`GameplayTagContainer.cpp:1250`), `MatchesTag called on an invalid gameplay tag
  SmartObject.ObjectType.Player`. 스택은 `UnrealEditor-StateTreeEditorModule.dll`.
- **원인** — `titan_example/Config/`에 **`DefaultGameplayTags.ini`가 아예 없었다**(등록 태그 0개).
  원본 `anim_test/SoldierLab/Config/DefaultGameplayTags.ini`에 태그 39 + 리다이렉트 4가 있고 그중
  `SmartObject.ObjectType.*` · `StateTree.SmartObject.*` · `Foley.*`가 전부 미등록 상태였다.
  09-14 이관이 Content/Source/플러그인은 옮겼으나 이 config는 안 따라왔다(Migrate는 Content만 옮긴다).
- **사슬** — 배치 → `BP_Soldier_Friendly`→`AC_SmartObjectAnimation`, `BP_SoldierCharacter`→`AIC_Soldier`
  →`ST_Soldier_SmartObject`(GASP 복제본)가 로드 → StateTree 에디터가 컴파일·검증하며 태그 쿼리 → ensure.
  크래시 아님, 진행은 됨. 그 조건은 영원히 false이고 `StartLogic`도 지워져 있어 **전투 AI 영향 없음**.
- **조치** — 원본 ini를 `titan_example/Config/DefaultGameplayTags.ini`로 복사(p4 add, 바이트 동일).
  게임플레이 태그는 기동 시 등록되므로 **에디터 재시작 필요** → 재시작 후 해결 확인.
- **교훈(P142)** — Migrate가 안 옮기는 Config가 지금까지 셋: 콜리전 채널 · DDCvar · 게임플레이 태그.
  셋 다 에러 없이 조용히 틀리거나 엉뚱한 모듈의 ensure로만 드러난다. 다음 합병 때는 `Config/*.ini`를
  **파일 단위로 diff**해 없는 파일부터 셀 것.
- **미적용(정리 세션 몫)** — 죽은 참조 제거(`AIC_Soldier`의 `ST_Soldier_SmartObject`,
  `BP_Soldier_Friendly`의 `AC_SmartObjectAnimation`). `Foley.*`는 ini에 남아야 한다.

---

## 2026-09-17 — SoldierLab 위험 지도 폐기 → 상황 필드(조명 모델) + 잠입 거동 + 오전 교전 수정 넷

`soldier_ai_lab` 의 진영별 위험 지도(`SoldierDangerMap`, 09-14)를 `p4 delete` 하고
`USoldierSituationFieldSubsystem`(`Source/SoldierLab/AI/SoldierSituationField`)으로 교체. 위험도를
**저장하지 않고 파생**한다 — 목격이 점광원, 엄폐가 그림자, 안 본 땅(사전값 0.5)이 앰비언트. 상세:
`soldier_ai_lab/ai/2026-09-17_situation_field_lighting_model.md`(시스템) ·
`soldier_ai_lab/ai/2026-09-17_infiltration_and_unknown_ground.md`(거동) ·
`soldier_ai_lab/ai/2026-09-17_threat_bonus_lane_denied_vehicle_heights.md`(오전 선행분).

- **오전 넷**(빌드·PIE, 사용자 "이제 잘 작동하는거 같아. 이상하게 멈춰있는 애들은 없긴했음") —
  ① 맞으면서 엉뚱한 곳을 보던 것: 표적 점수에 `ThreatenedBonus 0.6 × 나를 쏘는 놈`(`ReportThreatenedBy`,
  근접탄·피격이 호출) ② 어떤 총구 자세로도 안 뚫리는 자리에 15 s 앉아 있던 것("15 s 전이 0" 실측):
  `IsLaneDenied` 2 s 래치 → 엄폐 HERE 비용 +1.0 ③ 30 Hz 방아쇠 떨림: 가치 게이트 히스테리시스 1.2
  ④ 병사가 UGV 바퀴를 겨누던 것: 비-캐릭터는 바운즈 Z 비율(눈 0.8/표적 0.5).
- **잠입 요구**("문을 열고 들어가는 것이지 박차고 들어가는 것이 아니다") — 진단: 노출이 *믿는 적*에
  대해서만 정의돼 눈 0 = 노출 0 → 광장 질주. 답: `UnknownPresence 0.5`(관측은 0이 아니라 0.5를 향해
  낡는다) + 시야 콘 스윕(2 트레이스/틱, 사전값을 물리는 유일한 것) + 스프린트 규칙(접촉∥제압∥사선 거부
  일 때만, `Cautious` 절대/`Rush` 항상).
- **"붉은 얼룩" 진단** — 사용자: 전부 붉고 엄폐/공터 구별 없고 안 내려온다, "GI 같다 — 점광원+그림자여야".
  구조적 원인: 위험도가 45명 × 눈 3 × 광선 48의 **출처 없는 누적 버퍼**(30 s 타이머). 결론 = 렌더링
  방정식(위험도 = ∫ 경계도 × 가시성): **저장 1**(경계도) + **정적 1**(호라이즌 맵, 셀당 8방향 × 2높이
  지연 굽기) + **파생 1**(위험도 = 라이트 그림자 + 앰비언트), 캐시 유효성은 타이머가 아니라 라이트
  **세대**. 라이트의 주인은 적 액터도 "사람에 대한 믿음"도 아닌 **목격**(사용자 지적 2회) — 0.5 s
  연속창 넘기면 그 자리에 얼고 다음은 새 라이트(왼쪽 벽/오른쪽 벽 둘 다 켜진 채). 섀도우 48방향 × 2줄,
  셀별로 광선 높이가 자세(웅크려도/서면/아님)를 정한다. 쓰는 쪽은 눈·귀만 — 엄폐 부채꼴·제압은 **안 쓴다**.
  같은 날 도입한 "가상 관찰자"(안 본 땅에 상상의 눈 2개)는 가설을 사실로 써서 **철회**.
- **오버레이** `SoldierLab.Debug.Field 1` — 월드당 1회(진영 그림, `GetObservedSoldier` 신규), 채널 3 = R 위험도/
  G 경계도(노랑 살상 지대·검정 훑음), 4 = 앰비언트, 크기 = 신선도, 라이트 핀/링. 값 37개는 Project Settings →
  Game → **SoldierLab Situation Field**(`USoldierFieldSettings : UDeveloperSettings`, `Build.cs` `DeveloperSettings`
  추가) — 전부 [C], 사용자가 오버레이 보며 잡을 예정.
- **검증**: 1·2단계 + 오버레이 빌드·PIE(사용자 "잘되는거같아"). **3단계(호라이즌/앰비언트·필드 후보 6·눈 0
  필드 자세·볼 곳 루프·골든앵글 회전·Rush 우회)는 코드만, 빌드 대기 [B].**
- 문서: `soldier_ai_lab/CLAUDE.md`(읽기 순서 · **P143~P151** · 6.2c cvar) · `IMPLEMENTED.md` 0/5.1절 ·
  `OPEN_ITEMS.md`(+C-130~C-139 · W74~W78, C-102 superseded, C-126 갱신) · `CURRENT_STATE.md` 밤 블록 ·
  `ai/2026-09-14_danger_map_and_position_commitment.md` 상단 superseded 배너. `guide/` 에 병사 문서 없어 무변경.

---

## 2026-09-18 — SoldierLab 상황 필드 2일차: 3단계 PIE 확인 → LOD(밉 + 다중 앵커 퇴거) · 오버레이 v2 · 라이트 부정 증거

`Source/SoldierLab/AI/SoldierSituationField.h/.cpp` **재작성**(1488 → 2332줄), `SoldierFieldSettings.h` 갱신. 상세:
`soldier_ai_lab/ai/2026-09-17_situation_field_lighting_model.md` **16~18절**(같은 문서를 확장, 1~15절의 `.cpp` 줄
번호는 09-17 판이라 어긋남을 상단에 명시).

- **3단계 검증** — 사용자 빌드·PIE: 호라이즌/앰비언트 + 오버레이가 "딱 내가 원하는 그림이 이제 나옴". 문서 세션
  지적으로 섀도우의 "셀 지면 110 cm 아래까지 웅크려도 보임"을 `GroundSlackCm 40`으로 정정(그 아래는 캐시
  오차). 위험 지도 잔존 주석 [W78] 정리.
- **LOD 결정** — 사용자 "복셀 GI처럼 클립맵 레벨을?" → 분석: 필드는 요구 주도·희소라 맵 전체를 도는 틱 루프가
  없고 **계산은 레벨 크기와 무관**; 커지는 것은 **메모리 ∝ 지나간 면적**(09-17까지 셀 해제 코드 없음)과 굽기 ∝
  경로 길이. 시나리오(적군이 큰 맵을 후퇴하며 싸움)에서 진짜 문제는 **꼬리**. 진영에는 병사 15명이라 클립맵
  중심이 없음 → **(A) 밉**: 레벨 1+는 레벨 0의 집계(경계도 max · lit 평균 · 트임 평균, 레벨 0 쓰기가 `bDirty`만
  세우고 다음 읽기가 재집계, TTL 1 s, 아무도 직접 안 씀) **+ (B) 다중 앵커 퇴거**: 진영 병사 아무나에게서
  `DetailRadiusCm 8000 × LevelScale^L` 밖 레벨 L 디테일을 부모에 **잔여물**로 접고 해제(2 s 키 스냅샷, 1024/틱,
  호라이즌은 양 진영 앵커 기준, 굽기 큐에 있으면 안 접음). 오버레이만 진짜 클립맵. `LevelCount 1 → 3`(200/800/3200),
  읽기는 `SampleFinest`(레벨 0 → 1 → 2), 섀도우는 앵커 80 m 안 라이트만, `FirstLevelFor`/`DetailRangeCm` 삭제,
  `SoldierLab.Field.CellSizeCm` cvar. 메모리 ≈ 병사당 5천 셀 × 80 B 상한. 빌드·PIE — 링이 거리에 따라 거칠어짐,
  "기능적으로는 아주 잘 작동".
- **오버레이 v2** — 사용자 보고 "4000 cm에서 깜빡임 + fps 저하". 원인: 수명 있는 `DrawDebugMesh`는 월드
  `PersistentLineBatcher`로 가서 **매 프레임 모든 요소를 늙히고 렌더 상태를 재생성**, 그리고 30 fps에서 재그리기
  0.133 s vs 수명 0.12 s = 빈 프레임. 답: 서브시스템이 `ULineBatchComponent`를 소유(`DefaultLifeTime 0`),
  0.1 s마다 **한 틱에 flush + refill**, 색 8단계 양자화로 **색별 메시 1개**, `Debug.Field.Level −1` 클립맵 링
  (레벨 0 25 m / ×4 / 반경 12000), **불투명도 = 신선도**(사용자: 링에서는 크기가 레벨이니 크기는 0.7 고정),
  거친 셀은 평평한 사각형. 빌드·PIE 확인. 이어 사용자가 **50 cm 셀 시험**에서 오버레이가 **−X 반쪽만** 그려지는
  것을 발견 → 원인 X 순 걷기 + 캡 → 링 면적을 예산과 비교해 **대칭 축소** + 헤더 CAPPED, `MaxDebugCells 6000`.
  같은 회차: 미굽기 셀 **파랑끼**(모름 ≠ 안전) · 헤더 2줄(관찰 병사 발밑 셀의 direct/ambient/자세/presence/
  open/seen) · 볼 곳 보라 화살표 2.5 m · 라이트 불투명도 = 밝기, 초록 봄/호박 들음(인지 오버레이 색), 흰 점 =
  그림자 미완 — **이 회차부터 빌드 전 [B]**.
- **부정 증거** — "직접 가서 봤는데 없으면 사라지나?" → 아니오였다(20 s 반감뿐). `MarkClearAlongRay`가 지나간
  **얼린** 라이트에 빈 채로 본 **시간**을 적립(`ClearViewSeconds`, 간격 < 1 s면 간격, 아니면 0.1 s), 밝기에
  `0.5^(t/ClearViewHalfLifeSeconds 4)` 추가. **삭제 아님**(웅크렸거나 2 m 옆일 수 있다). 다시 보이면 0. [B].
  ⚠ 문서 세션 발견: 주석은 들은 라이트를 제외한다지만 코드는 안 거르고 반경이 커서 **더 잘 걸린다** → [W83].
- 문서: 시스템 문서 0·3·5·6·7·8·9·10·11·13·14절 갱신 + **16~19절 신설** · 잠입 문서 4·6절 추가 ·
  `soldier_ai_lab/CLAUDE.md`(**P152~P157** · 6.2c cvar · 읽기 순서) · `IMPLEMENTED.md` 머리글/5.1/현행값 ·
  `OPEN_ITEMS.md`(+C-140~C-143 · +W79~W83 · **W78 해결** · C-139 superseded) · `CURRENT_STATE.md` 09-18 블록.
  `guide/`에 병사 문서 없어 무변경.

## 2026-09-18 오후~저녁 — SoldierLab 순찰 · 부채꼴 스캔 · 이동 강건성 · 얇은 엄폐 ("적이 죽은 뒤 가만히 서 있는 아군")

상세: `soldier_ai_lab/ai/2026-09-18_patrol_scan_and_move_robustness.md`(거동) · 시스템 문서 `ai/2026-09-17_situation_field_lighting_model.md`
**20~23절**(필드 몫) · `ai/2026-09-17_infiltration_and_unknown_ground.md` 13절 · `squad/2026-09-17_command_layer_design.md` 10절(계약).
`Source/SoldierLab/AI/SoldierSituationField`(2332 → 2491줄) · `SoldierCover`(→ 1909줄) · `SoldierObjective` · `SoldierEngagement` · `Squad/*`.
포즈 세션이 같은 날 `Pose/SoldierScanTurnComponent`를 썼다(여기선 계약만 기록).

- **사용자 보고** — 시험 레벨에서 적이 전멸한 뒤 아군이 가만히 서 있고, 보라 화살표(볼 곳)가 몸과 반대를 가리키며 5분 뒤
  뒤집힌다. 원인 넷: (a) 존이 기본으로 주는 섹터(`bUseSector true`)를 교전 층이 **고정 방위**로 읽어 명령받은 병사의 스캔
  루프가 영영 안 돌았다 → **섹터 = 부채꼴** — 필드 `GetMostExposedDirection`에 편향·arc 파라미터(선택만 제한, 적분은
  그대로), 교전은 볼 곳 > 섹터 중심 (b) 콘 스윕 광선 6° 간격이 20 m 밖에서 셀(2 m)보다 넓어 사이 셀이 영영 안 비워짐 →
  `MarkClearAlongRay`가 16 m 밖에서 **3셀 폭 띠** (c) 볼 곳이 엄폐 층 "눈 0"에 게이트돼 죽은 적 기록이 ≈ 90 s 눈으로
  남는 동안 교전 층(다른 문턱)과의 틈에서 **아무도 조준을 안 몰았다** → 볼 곳은 항상 계산, 쓸지는 교전 층 (d) 총 내린
  idle에서 몸이 안 돎 → 교전 층이 무접촉 분기에서도 `GetAimPoint()`/`IsScanning()` 발행 → 포즈 세션의
  `SoldierScanTurnComponent`(정지 ∧ 총 내림 ∧ 스캔/접촉이면 캡슐을 180°/s로). `[Cover]` 로그 `watch=` 추가.
- **순찰** — "적 정보 없는 존 방어는 계속 순찰해야 한다." 경로가 아니라 **비용**: `GetStaleVantage(Foot)`(앰비언트 적분의
  방사체를 "안 본 지 얼마나"로) → Hold/수비 반경 안 `+ PatrolWeight × (1 − vantage)`, 눈 0일 때만. 낡은 조망이 싸다 → 서서
  본다 → 콘이 비운다 → 비싸진다 → 다음 자리, 경로 없음. 존·`ASoldierObjective` `PatrolWeight 1.0 / PatrolStaleSeconds 30`
  (`L_SoldierTest`엔 명령이 없어 목표에도 — 첫 시험 로그 `[Squad]` 0줄).
- **로그로 잡은 이동 결함 셋**(`titan_example.log` 08:52·09:51) — ① Friendly가 230 s 동안 60/s `MOVE @(+4,−492)` = 큐브
  꼭대기 셀: 호라이즌 굽기가 큐브 안에서 시작(전부 `bStartPenetrating`) → 가장 어두운 셀 → 내비 투영이 큐브 위 → 부분
  경로로 벽까지 → 정지로 읽혀 매 틱 재발행. 답: `FHorizon::bSolid`(전방향 255 + 후보 제외) · `MoveTo` 전 `FindPathSync`
  부분 경로 불허 + `RejectCandidate` 30 s + 무진전 거부 · `MoveGraceSeconds 0.75`. ② `x=−1824` 벽 꼭대기 REJECT 35건 —
  안 구운 셀을 노출 0 = 완벽 은폐로 읽음 → `GetExposureByStance` → bool, **미지 = 열림**. ③ Enemy_A `hold@z1` 후보 전부
  `o3.20` 200 s — Hold 비용이 밴드 끝 1.0에서 평평 → 밴드 밖에서도 계속 오름(TaskCost·Objective 수비).
- **CQB** — 이동 중 볼 곳을 진행 방향으로(`WatchTravelBias 1`), 정지 Approach는 앵커(`WatchApproachBias 0.5`), 눈 0 도착
  머무름 `ScanDwellSeconds 2`. 분대 세션 목록 답: "숙여서 침투"는 필드 자세로 이미 있고 "어디서나 더 낮게"는
  `FSoldierAssignment::MinStance`(Rush 무시, **titan DT/콘솔 미연결**) · "은폐 경로"는 필드 후보·경로 앰비언트로 이미 있음.
  [W83](들은 라이트 부정 증거) 코드로 해결.
- **빌드·PIE** — 사용자 "잘 되는 거 같음. 이제 정상적이다": 적군이 존으로 돌아오고, REJECT 루프 없고, 순찰이 움직인다.
  ~~ScanTurn이 실제로 도는지는 미확인.~~ → 포즈 세션이 PIE 확인(아래 "AI 포즈 층 3종" 항목).
- **저녁 A/B/C — 코드만, 빌드 전**: A 은폐 판정 = 자세마다 모든 눈, 보는 눈 활동도 비율 ≤ `HiddenGazeFraction 0.5`(나무는
  한 방위만 가려 눈 둘이면 어떤 나무도 엄폐가 아니었다) · B 미세 위치 = 정지 ∧ 눈 있음일 때 발 주변 8 × 30 cm 후보 +
  수용 반경 20 cm(병사별 고해상도 복셀은 기각 — 눈이 움직이면 낡는다) · C 코너 멈춤 = 도달성 경로의 굽이 1.5 m 앞에서
  0.8 s `PauseMove` + 볼 곳을 굽이 너머로 — **진짜 파이 자르기(경로 모양)는 아님**. 레벨: 시험 레벨에 얇은 원기둥 필요,
  실제 레벨 나무의 Sight 채널 Block 확인 필요.
- 문서: 새 1건 + 시스템 문서 0·7·10·13·14·18절 + **20~23절** · 잠입 문서 13절 · 분대 설계 10절 · `soldier_ai_lab/CLAUDE.md`
  (읽기 순서 · **P158~P166** · 6.2c cvar `watch=`/`ScanTurn`) · `IMPLEMENTED.md` · `OPEN_ITEMS.md`(+C-148~C-153 · +W85~W90 ·
  **W83 해결 · W84 절반**) · `CURRENT_STATE.md`. ⚠ 브리핑의 "마지막 C-143/W83"은 병행 세션이 이미 C-147/W84까지 써서
  C-148/W85부터. `guide/`에 병사 문서 없어 무변경.

## 2026-09-17~18 — SoldierLab AI 포즈 층 3종(ScanTurn · GaitBridge · PoseSmoother) + 관전 폰 추기 (병행 포즈 세션)

상세: `soldier_ai_lab/animation/2026-09-18_ai_pose_layer_scanturn_gait_smoother.md` · `ai/2026-09-14_cover_frame_fix_and_observer.md` 5절.
`Source/SoldierLab/Pose/SoldierScanTurnComponent` · `SoldierGaitBridgeComponent` · `SoldierPoseSmootherComponent`(신규 3쌍) ·
`Observer/SoldierObserverPawn` · `Content/SoldierLab/Blueprints/BP_SoldierCharacter`. AI 세션(위 항목)이 발행한 계약을 **소비**하는 쪽.

- **계약 소비 3종, 전부 AI 전용**(`IsPlayerControlled()` → return): 총 내리고 서 있는 AI 의 **몸이 볼 곳으로 돈다**(캡슐 yaw 를
  `GetAimPoint()` 로, 20°에서 시작 5°에서 정지, 180°/s — 메시는 GASP OffsetRootBone + MM turn-in-place) · `GetDesiredGait()==Walk` 를
  GASP `CharacterInputState.WantsToWalk` 에 리플렉션으로(BP 구조체 필드는 GUID 접미사 → authored name 매칭) · 자세 축 4종(stance ·
  lean · BF-H/V)을 **사다리꼴 프로파일**(축별 MaxSpeedUp/Down/Accel, 앉는 쪽이 빠름 1.6/0.9) × `GetPoseUrgency()` 로 0.5~1.6 배,
  목표가 바뀌어도 속도 연속. BP 그래프는 안 고치고 상태+AI 목표 변수를 같은 값으로 써서 BP 램프를 통과.
- ★ **"급하면 1프레임 스냅" 원인** — BP 램프 rate(`StanceRate`/`BlindFireRate` + 린 램프 리터럴 핀)를 **0** 으로 얼렸는데
  `USoldierAxisLibrary::RampAxisTo` 는 rate ≤ 0 이면 **Target 을 그대로 반환**("램프 없음") → BP 가 raw 목표를 다시 덮은 뒤 램프가
  raw 를 돌려줌. `ext` 감사값 0 이 단서(제3의 손이 아니라 램프 자체). `FrozenRate 0.0001` + 핀 0.0001 로 해결, 사용자 "해결완료".
  남는 이산 전환은 GASP 크라우치 DB(문턱 0.5) — 눈에 띄면 GASP 블렌드 쪽 [W91].
- **H(머리 추종)는 AI 에 자동으로 켜지 않는다** — 09-17 에 `bEnableForAI` 로 AI 에 H 를 켜 몸을 돌리게 했다가 사용자 되돌림("H 는
  1인칭 부가 기능, 노출되면 안 됨"). AI 몸 회전은 위 ScanTurn 으로 분리.
- **관전 폰**: 자유 비행 시 **휠 = 비행 속도**(1200 cm/s ×/÷1.25, 추적 중엔 3인칭 거리 그대로) · `bIgnoreTimeDilation`
  (`CustomTimeDilation = 1/배속` — "slomo 로 낮추면 카메라까지 느려져 불편") · **롤 잔류 수정**(F → T → 3인칭/해제 뒤 카메라가
  기울어진 채 — 컨트롤 회전 동기화는 yaw·pitch 만, 전환 시 롤 0). 사용자 "잘됐음".
- 검증: ScanTurn 돎 · 걷기 나옴 · 스무더 평상시 사다리꼴 + 급할 때 스냅 없음 · 휠/slomo/롤 — 전부 사용자 PIE. [C-152] 포즈 측 확인.
- 문서: 새 1건 + `soldier_ai_lab/CLAUDE.md`(읽기 순서 · **P167~P171** · 6.2c cvar `PoseSmooth`/조작키 휠) · `IMPLEMENTED.md`(머리글/0/4/5.1/
  현행값/5.2) · `OPEN_ITEMS.md`(+C-154 · +W91 · C-152 포즈 측 확인) · `CURRENT_STATE.md` · `README.md` · 관전 문서 5절 ·
  `animation/2026-09-14_sight_alignment_plan.md` 0'(AI 와의 관계) · 디자이너 가이드 4.3/4.4. Perforce: Pose/ 8 · Observer/ 2 ·
  Camera/ 2 · BP_SoldierCharacter.uasset **미제출**(CL 469 이후). `guide/`에 병사 문서 없어 무변경.

## 2026-09-18 밤 — SoldierLab 순찰·스캔·이동 강건성 4차: 코너 멈춤 루프 · 긴장도/걸음 · 포즈 급박도 (AI 세션, 위 포즈 세션이 소비하는 계약의 발행 측)

상세: `soldier_ai_lab/ai/2026-09-18_patrol_scan_and_move_robustness.md` **12~17절**(같은 날 문서에 이어 붙임, 새 문서 없음) · 시스템 문서
`ai/2026-09-17_situation_field_lighting_model.md` **24절** · `squad/2026-09-17_command_layer_design.md` 10절(계약 3개 추가).
`Source/SoldierLab/AI/SoldierCover`(1909 → 1931줄) · `SoldierEngagement`(1502 → 1586줄) · `SoldierSituationField`(2491 → 2501줄).
포즈 세션의 `Pose/SoldierGaitBridgeComponent` · `SoldierPoseSmootherComponent`는 읽기만(소비 측 계약 기록).

- **저녁 A/B/C 첫 빌드 → 로그**(13:15, `L_SoldierTest` 명령 있음) — Enemy_A `HERE o4.21`이 60 s 동일, `MOVE`가 **0.82 s마다**.
  루프: 스폰 옆 굽이 → `PauseMove` 0.8 s → 재개 순간 속도 0 ∧ 유예 0.75 s 이미 만료 → "정지" → 재결정 → 새 경로가 같은
  굽이에 **새 인덱스** → 또 멈춤. 수정: 굽이 기억을 인덱스에서 **자리**로(`LastPausedCornerLocation`, `CornerStopCm` 반경 — 굽이는
  자리다, 경로 인덱스에 건 기억은 재탐색에서 죽는다) + 재개 시 `LastMoveIssuedSeconds = Now`(재개된 이동 = 방금 발행한
  이동) + 눈이 나타나면 즉시 재개. 문서 세션 지적 셋도 코드로: `RejectedCandidates` 만료 정리 · `IsScanning()` 섹터만 있어도
  true · `GetExposure` 미지 = `UnknownPresence × AmbientWeight`(0 아님). **A/B/C + 이것까지 빌드·PIE, 사용자 "잘됨".**
- **조용한 병사가 어디를 가든 조깅** — 스프린트 규칙 아래가 GASP 기본 Run이라서. **긴장도** `GetTension()` = 알람(접촉 ∥
  제압 ∥ 사선 거부 ∥ 적 기록 총성 1 s 안) 뒤 `0.5^(age/20 s)`, **걸음** `GetDesiredGait()` = Sprint / Jog(접촉 ∨ 긴장 ≥ 0.3,
  단 Cautious ∧ 무접촉 = Walk) / Walk. 포즈 세션 `GaitBridge`가 Walk → GASP `WantsToWalk`(Sprint는 옛 BP 브리지, Jog = 기본
  Run, 긴장도 자체는 GASP gait가 이산이라 안 섞음). 사용자 "잘됨" — 조용한 경비는 걷는다.
- **자세 전환(Q/E 린 · V/B 앉기 · 맹목사격)이 기계적** → 논의 결론: **AI는 목표 4개 + 급박도 숫자 하나만 내고 움직임(가속·
  순항·정착, 축별, 앉기는 내려갈 때 빠르게, 목표 변경 시 속도 연속)은 포즈 층이 소유** — 교전은 이미 실제 자세를 되읽어
  지연이 설계상 허용됨 · 축별 물리는 몸의 것 · 목표/급박도는 바뀔 때만 복제 · 결정은 이산, 움직임은 연속. `GetPoseUrgency()`
  = `Urgency*` 7값(Idle 0.15 · ContactIdle 0.35 · LookPeek 0.3 · ShootPeek 0.6 · Retreat 0.7/0.6 s · Reload 1.0) max + 제압.
  에디터 분할: 상황별 급박도는 교전 컴포넌트에, 축별 속도는 스무더에. `[Engage]` 로그 끝 `tension gait urg`.
- **스무더 뒤 "급할 때만 1프레임 점프"** — 이쪽 확인: AI 층은 어떤 축도 쓰지 않음(리플렉션 읽기뿐), 목표가 한 틱에 0→1로
  뛰는 건 설계(제압·재장전·조리개). 후보 셋(BP 램프 rate 0 = 즉시 / 옛 브리지 직접 쓰기 / GASP `Crouch()` 문턱)을 냈고 **어느
  것인지 이 세션은 확정 못 함**; 사용자 "구현 완료" → 포즈 세션이 첫 번째로 확정·기록(P167). 새 W 없음.
- **잠입 접근 현황 답** — 개인 층 완성(사전값 · 필드 후보 · 기하 자세 + `MinStance` · 조용하면 걷기 · 눈이 발을 이끈다 · 코너
  멈춤 · 도착 머무름 · 콘 스윕 · Rush가 전부 품). 시나리오는 이미 `MoveTo r1200 roe=hold spd=cautious agg=0.30`. 우리 몫 아님:
  내비 가중 경로 · `MinStance` titan DT · 나무 Sight 콜리전.
- 문서: 새 문서 없음 — 09-18 거동 문서 12~17절 + 0/8/9/10/11절 갱신 · 시스템 문서 22.2 정정 + 24절 · 분대 설계 10절 ·
  `soldier_ai_lab/CLAUDE.md`(읽기 순서 · **P172~P175** · P166 [B] 해제 · 6.2c `[Engage]` 꼬리) · `IMPLEMENTED.md` · `OPEN_ITEMS.md`
  (+C-155~C-156 · **W89 · W90 해결** · C-152/C-153 갱신) · `CURRENT_STATE.md`. 번호는 병행 포즈 세션이 P171/C-154/W91까지 써서
  P172/C-155부터. `guide/`에 병사 문서 없어 무변경.

## 2026-09-18 밤 ~ 09-21 — SoldierLab 분대 스코프 상황 필드 · 엣지 전진(파이 자르기의 창발) · AI 가 소유하는 사격 콘 · 섀도우 수요 감축

상세: `soldier_ai_lab/ai/2026-09-21_per_squad_field_edge_advance_fire_model.md`(새 문서, 절 넷). `Source/SoldierLab/AI/SoldierSituationField`(2501 → 3036줄) ·
`SoldierCover`(1931 → 2374줄) · `SoldierEngagement`(1586 → 1712줄) · `SoldierSight` · `SoldierFieldSettings.h` · `SoldierPerception.cpp` · `SoldierDebugDraw.cpp`.
Perforce CL 498 · 500 제출(사용자) + 섀도우 감축분은 작업 트리(빌드 전).

- **상황 필드가 분대 하나당 하나** — 진영 하나에 필드 하나는 전지(한 명이 본 것을 전원이 즉시 앎)였다. `FScope` = 진영 × 분대 슬롯
  (`USoldierFieldSettings::MaxSquadsPerFaction 3`, 슬롯은 처음 말하는 순서, 넘치면 슬롯 0 공유 + 분대당 1회 경고), 공개 API 전부
  `const USoldierIdentityComponent* Who`, 호라이즌(기하)만 세계 공유. 남의 분대 목격은 **무전이 닿았을 때 관측 시각으로**(`ReceiveSharedRecord` →
  `ReportSighting`). **`bTakesSquadOrders=false`(titan 브리지가 UGV 에 단 표적용 Identity)는 스코프·앵커·관찰 대상 아님** — 그것이 첫 Friendly
  "분대"로 슬롯을 먹고 UGV 조종 중 오버레이가 UGV 를 따라가던 원인. cvar `SoldierLab.Debug.Field.Squad`(슬롯 고정) · `.Centre 1`(그린 분대의
  병사 전원 주위 링 — UGV 에서 분대 보기), 헤더 `[Field] HOSTILE/1`. `L_SoldierScenario` 아군 4분대(5/5/5/5) → **3분대(7/7/6)**. PIE ✅.
- **엣지 전진** — 09-18 저녁 코너 멈춤(+ 잠깐 있었던 손으로 그린 파이 호)을 **삭제**. 사용자: 파이 자르기는 **정보 굶주림 대 위험에서 창발**해야
  한다 — 팀원이 치운 땅은 그냥 지나가고, 방은 짧게 복도는 길게, 호의 반경은 예산에서, 새 트레이스 없이, 타이머 없이. 기각: 손 그린 호 + 머무름
  (물리적 뜻 없는 상수, 치운 복도에서도 의식) · 후보마다 광선 부채꼴(걸음 하나에 수백 트레이스, 이미 아는 것을 다시 잼). 채택: 콘 스윕 광선을
  슬롯별로 보관(`USoldierSightComponent::GetSweepRays`) → 옆은 멀리 가는데 짧게 멈춘 광선 = **엣지** → 걸음이 눈-엣지 선을 돌리며 여는 쐐기를
  **`GetWedgePresence`**(경계도 × m², 표본 셀의 구운 호라이즌으로 가림, 트레이스 0)로 값 매김 → 예산 `StepPresenceBudget 6` 안에서 가장 멀리 가는
  걸음(60 cm × 1..4, ±105° 부채꼴) → 연 조각의 글로우가 `AnalyzedPresence 1` 아래로(≤ 3 s) 내려가면 다음. 엣지에 가까울수록 같은 걸음이 많이 열어
  발이 밀려난다 = 호는 예산이 만든다. 접촉 → 그 자리 정지, 걸어온 자리는 10 s 엄폐 후보. 교전: 쐐기 조준 + 벽 쪽 린, `WantsToAim` 진전 중
  true, Walk. **CL 500 에 포함돼 빌드됐으나 PIE 판정 미보고** [W95].
- **사격 콘을 AI 가 소유** — 무기 고정 3° 로는 40 m 완벽 위치에 2% 명중이라 조준 게이트 무의미, 선회 직후·버스트 중·달리는 중이 같은 콘.
  `GetShotSpreadDegrees()` = `WeaponSpreadDegrees 0.8`(← 3) × `(1 + MovementSpreadScale 6 × v/600)`(← 2) × 자세(`Lean 1.5`/`Blind 15`, ← 1.4/4 —
  가치 사거리 보존) × (1+반동) + **흔들림**(선회 `AimSettleInitialDeg 2.5` → `exp(−dt/0.4)`, 발마다 `RecoilKickDeg 0.6`, 이동 바닥 `MoveWobbleDeg 1.5`).
  조준 게이트: 기록 반경 ≤ `TargetRadiusCm 45 × AimedHitTolerance 2` 이면 지금 콘 → `Aimed` / 정착하면 → **`Settling`**(새 의도 — 40 m ≈ 0.65 s ·
  60 m ≈ 1.5 s · 10 m 0) / 예비 → `Suppressive`. **버스트** `BurstRoundsMin 2..Max 5`(제압 5) · `BurstPauseSeconds 0.5 × (1 ± RhythmJitter 0.35)` ·
  `FRandomStream` 병사 이름 시드 → **`Pacing`**. `[Engage]` 꼬리 `cone wobble burst next`, `KnowledgeToSpreadRatio` 삭제. PIE ✅ 거동.
  ⚠ **무기 BP 가 아직 고정 콘으로 쏘고 `BP_SoldierCharacter` 가 `HasContact()` 를 aim 모드에 먹인다** → [W93](그 전엔 탄이 따르지 않는 콘에 대한
  결정, 명중률·린 판정 불가).
- **섀도우 수요 감축 + 비용 줄** — 헤더 3줄째 `[Field] cost/tick: shadows 0.03 ms (96 rays/tick, 0 waiting of 96 alive)` 실측: ms 는 문제 아니고
  **96 = 6 스코프 × `MaxLights 16` 상한**이 문제 [W92]. 코드(빌드 전): 따라가는 라이트는 **한 셀 ∧ `ShadowRecastSeconds 0.5`** 뒤에만 재캐스트
  (`FLight::ShadowEye/ShadowCastTime`, 보이는 적은 엄폐 스윕의 실제 눈이 잰다), 얼면 즉시, 반 셀 안 다른 분대 라이트는 **riders** 로 한 벌의
  트레이스에 동승해 각자의 필드에 씀. 넘침 경고 분대당 1회. 헤더 UPROPERTY 추가라 정식 빌드 [W96].
- 문서: 새 1건 + `soldier_ai_lab/CLAUDE.md`(읽기 순서 · **P176~P180** · 6.2c cvar/로그 꼬리) · `IMPLEMENTED.md`(머리글 · 5.1 SituationField/
  FieldSettings/Sight/Perception/Engagement/Cover/DebugDraw · 값 블록) · `OPEN_ITEMS.md`(+C-157~C-161 · +W92~W96 · **W74·W85 해결**) ·
  `CURRENT_STATE.md` · 시스템 문서 25절 포인터 · 09-18 거동 문서 18절 정정. `guide/` 에 병사 문서 없어 무변경.

## 2026-09-21 늦게 — SoldierLab 섀도우 감축 빌드·PIE 확인 · 디버그 오버레이 노출 보정 (EV10 레벨에서 숯검정이던 오버레이)

상세: `soldier_ai_lab/ai/2026-09-21_per_squad_field_edge_advance_fire_model.md` **10~12절**(섀도우 감축 검증 · 레벨 재편 확인 · 포인터) ·
**`soldier_ai_lab/ai/2026-09-21_debug_overlay_exposure.md`**(새 문서). `Source/SoldierLab/AI/SoldierDebugDraw`(92/304줄) · **`AI/SoldierDebugMesh`(신규, 55/146줄)** ·
`SoldierSituationField`(3036 → 3081줄) · `AI/` 7파일의 오버레이 호출 · `SoldierLab.Build.cs`. 작업 트리(제출은 사용자). 전부 빌드·PIE "아주 잘됨"(사용자).

- **섀도우 감축(④) 정식 빌드 → PIE ✅, [W96] 해결** — `FLight::ShadowEye/ShadowCastTime` · 한 셀 ∧ 0.5 s 재캐스트 문턱 · 얼림 즉시 · riders 그대로.
  비용 줄 재측정 `shadows 0.03 ms · 0 waiting · 96 alive` = **감축 전과 동일**(이 장면은 원래 예산이 남아 ms 로는 차이가 안 보인다 — 효과는 예산이
  모자라는 장면에서 waiting 이 안 쌓이는 것, [C-161] 그대로). ⚠ 사용자가 "96" 을 waiting 으로 읽었는데 **alive**(6 스코프 × `MaxLights 16` 상한,
  [W92] 그대로)다. `L_SoldierScenario` 아군 3분대(7/7/6) 재편은 MCP 로 저장됨 확인 — DT 분대 '4' 는 [W94] 그대로.
- **오버레이 노출 보정** — 노출 EV10 고정 레벨에서 `SoldierLab.Debug.*` 오버레이와 엔진 내비메시 `P` 뷰가 **전부 숯검정**. 원인: 디버그
  프리미티브는 **톤매퍼 앞**에서 씬과 같이 노출되고, `DrawDebug*` 와 `ULineBatchComponent::DrawMesh` 는 8-bit `FColor` 라 선형 1.0 위로 못
  올린다(배처의 선·점만 `FLinearColor`). 기각: 레벨 라이팅/PP 낮추기(진단 도구 때문에 관찰 대상을 바꾸는 것 — P181) · 상수 배율(레벨·시간대
  마다 다름) · 엔진 `P` 뷰 수정(`UNavArea::DrawColor` 가 `FColor`, 사용자 "손대지 말 것" — 비목표). 채택: **뷰가 지난 프레임에 적용한 노출의 역수만큼
  밝게** — `SoldierDebug::GetExposureScale(World)`(`FSceneViewExtensionBase` 가 `SetupView` 에서 게임 뷰의 `GetLastEyeAdaptationExposure()` 를
  저장, 게임 스레드·렌더 없음, 첫 호출에 등록) · `Bright(World, FColor) → FLinearColor`(sRGB → 선형 × 배율, 알파 제외) · 래퍼
  `SoldierDebug::Line/Point/Sphere/Circle`(월드 라인 배처 한 프레임, `Line` 의 `LifeTime` > 0 은 영속 배처) · 신규 **`USoldierDebugMeshComponent`**
  (`UPrimitiveComponent` + 자체 씬 프록시, 같은 `GEngine->DebugMeshMaterial` 에 `FColoredMaterialRenderProxy(선형 색)` — 필드 오버레이의 셀
  사각형·라이트 링이 이걸 쓰고 선·점·구·화살표는 배처에 `HDR(…)`) · cvar **`SoldierLab.Debug.ExposureScale`**(0 자동 · 블룸 레벨은 조금 낮게 수동) ·
  `Build.cs` `PrivateDependencyModuleNames` + `RenderCore`/`RHI`. `AI/` 의 `DrawDebugLine/Point/Sphere/Circle` **24곳 전부** 래퍼로(Cover 11 ·
  Objective 3 · Sight 3 · Engagement 2 · Perception 2 · Suppression 2 · Comms 1), `DrawDebugString` 은 그대로(화면 글자). 컴파일 함정: 필드
  오버레이의 지역 `Exposure` 가 바깥을 가려 C4456(에러) → `ExposureScale`. **안 한 것**: `Squad/` 9 · `Pose/` 7(화살표 5 — `Arrow` 래퍼 없음) ·
  `Weapons/` 1 은 소유 세션 몫 → **[W97]**. 새 튜닝값 없음(cvar 는 [C] 아님).
- 문서: 새 1건 + 09-21 필드 문서 10~12절 · `soldier_ai_lab/CLAUDE.md`(읽기 순서 · **P181** · 6.2c `Debug.ExposureScale`·`.Field` 추기) ·
  `IMPLEMENTED.md`(머리글 · 5.1 SituationField/FieldSettings/DebugDraw/**DebugMesh 신규 행**/Build.cs · 값 블록) · `OPEN_ITEMS.md`(**W96 해결** ·
  +W97 · C-161 갱신) · `CURRENT_STATE.md`(soldier_ai_lab · titan 루트) · `DOCS_INDEX.md`. `guide/` 에 병사 AI 문서 없어 무변경.

## 2026-09-21 — SoldierLab 성능 계측(`stat SoldierLab`) + 엄폐 틱 비용: 35명에서 프레임의 1/3 이던 Cover 8.9 → 1.9 ms, World Tick 28.5 → 22.6

상세: **`soldier_ai_lab/ai/2026-09-21_perf_instrumentation_and_cover_cost.md`**(새 문서). `Source/SoldierLab/AI/SoldierLabLog.{h,cpp}`(60/34줄 — `STATGROUP_SoldierLab`) ·
`AI/SoldierCover.{h,cpp}`(999/2491줄 — `TickComponent` 재구성) · `SoldierSight/Perception/Engagement/Suppression/Comms/SituationField.cpp`(`.Enabled` cvar + 스코프 + 트레이스 카운터).
전부 사용자 빌드 · PIE 실측(`stat game` / `stat SoldierLab` / `stat anim` / `stat PoseSearch` 캡처, InclusiveAvg, 로깅 on).

- **기준선** — 빈 `L_SoldierScenario` World Tick 1.71 ms(60 fps) → 병사 35(적 15 / 아군 20, UGV·트럭·UAV 있음, PCG 없음) **29.2 ms**(max 35.6), fps 20~30.
  `stat game` 이 이름을 댄 것은 ≈ 6 ms(Blueprint 6.7/386 calls · CharMovement 1.4 · Transform 0.82/752 calls · Spawn 0.7), Ticks Queued 968 vs 82(병사당 ≈ 25 틱 함수).
  사용자 "무식하게 바로 하지 말고" → **계측 → A/B 확인 → 계측이 ms 로 이름 댄 것만 수정**(원칙 P182).
- **Phase 0 계측** — `STATGROUP_SoldierLab`(`stat SoldierLab`): 시스템 틱 9(Sight · Perception · Cover · Engagement · Suppression · Comms · Health · Objective · Field) + Cover 하위 7(Here /
  Begin Sweep / Candidate / Route / Score / Finish (path) / Edge Advance) + Field 5(Shadows / Bake / Evict / Overlay / Wedge Reads) + Squad/Zone 선언만 · DWORD `Traces: Sight/Cover/Engagement/Field` ·
  `Soldiers Ticked`. off 스위치 **`SoldierLab.<System>.Enabled`** 7 + **`SoldierLab.Cover.Avoidance`**(RVO 런타임). 첫 계측(교전 중): **Cover Tick 8.92 ms(excl 6.86, 34 calls) · Traces: Cover 487/프레임(max 624)** ·
  Sight 0.96(108 트레이스) · Engagement 0.35(57) · Field 0.08 · 나머지 ≈ 0 — **Cover 혼자 World Tick 의 31 %**. `stat anim` 워커 19.7 ms 는 병렬(게임 스레드엔 대기 1.15 만).
- **Cover 수정 넷, 전부 결정 보존**(같은 눈·후보·비용 함수 → 같은 자리, 원칙 P183): (a) **이동 중 스윕 정지** `bSweepSuspended`(`Moving ∧ (유예 0.75 s 안 ∨ 속도 > 20)` ∨ 엣지 전진 관찰 — `FinishSweep` 이
  `bAlreadyGoing`/`bSweepWalkedWhileMoving` 으로 어차피 버리던 스윕을 **안 걷는다**, 정지하면 지금 발·지금 눈으로 새 스윕) + 이동 중 필요한 볼 곳만 **`UpdateWatchPoint()`** 로 떼어 `WatchRefreshSeconds 0.25`
  마다(이동 중 스캔 거동 불변) (b) 발밑 HERE 를 매 틱(눈 셋이면 27 트레이스/틱/병사)에서 **`HereEvalIntervalSeconds 0.1` ∨ 걸음 > `MicroStepCm 30` ∨ 새 스윕** 으로(스윕 끝에 한 번 비교되는 값, 눈은
  얼려 있다) (c) **경로 가지치기** — 경로 비용 ≥ 0 이므로 경로 없이 best 에 지는 후보는 경로 트레이스 생략(**정확** — best 동일) (d) 눈 0 스윕은 **`CalmCandidatesPerTick 2`**(트레이스 0 이라 후보
  30~40개 전체가 매 틱 통째로 — 필드 읽기 + `GetStaleVantage` 적분). 하위 스코프 7 도 같이.
- **결과** — Cover **1.92 ms**(Candidate 1.29 / Score 0.51 / Begin Sweep 0.43 / Route 0.35 / Here 0.15 / Finish 0.03 / Edge Advance 0.02, 중첩) · Traces: Cover **243** · SoldierLab 합 ≈ **3.3**(Cover 1.9 ·
  Sight 0.9 · Engagement 0.3 · Field 0.1) · **World Tick 28.5 → 22.6 ms**, 프레임 31 ms, GPU 6 ms → **게임 스레드 바운드**. 남은 게임 스레드(실측): **애니 ≈ 7.3**(`BlueprintUpdateAnimation` 3.47/68 calls =
  병사당 ABP 2 · AnimGameThreadTime 3.82 · SkinnedMeshComp Tick 1.85 · FinalizeAnimationUpdate 1.22 · UpdateKinematicBonesToAnim 0.97) · **이동/트랜스폼 ≈ 4~7**(Char Movement 1.3~2.3 · EndScopedMovementUpdate
  0.7~1.5 · PostTick 1.15 · MoveComponent 0.85~1.0 · Transform 0.75~0.83/≈ 800 calls = 병사당 부착 컴포넌트 ≈ 23 · UpdateOverlaps 0.17/50~65 calls) · **캐릭터/무기 BP 틱 ≈ 2.5**(Blueprint 5.7~7.1 − ABP) ·
  **투사체 스폰 0.65** · Ticks Queued ~960~1000.
- **RVO A/B** `SoldierLab.Cover.Avoidance 0`: Char Movement 1.58→1.32 · MoveComponent 2.20→1.01 · EndScoped 0.93→0.73 — 단 "켬" 캡처에 **히치**(World Tick max 84 · MoveComponent max 59.9 ·
  `PrimComp DispatchBlockingHit` max 58.8)가 섞여 평균을 끌어올렸다 → 정상 상태 RVO **0.3~0.5 ms, 켜 둔다**. 히치는 별건: 한 프레임의 `OnHit` 디스패치 58 ms = 투사체 명중 BP 이벤트의
  **동기 에셋 로드** 전형(추정) → 무기/BP 소유자 [W101](preload / async).
- **소유자별 인계**(ms 가 이유, `OPEN_ITEMS.md`): **포즈** [W98] ABP 이벤트 그래프 → 스레드-세이프(3.5) · `KinematicBonesUpdateType = SkipAllBones` 사망 전까지(1.0) · URO/거리별 애님 레이트 /
  **캐릭터 BP** [W99] 부착 컴포넌트 ≈ 23/병사 · 오버랩 컴포넌트 · 매 프레임 틱 로직(2.5) / **무기** [W100] 투사체 풀링(0.65) · [W101] 히치 / **SoldierLab 자체**(다른 층 뒤) [W102] Sight 표적
  트레이스 간격(0.4) · Cover 후보 박자(0.5) · Health/Suppression/Comms/Perception `TickInterval` / **분대** [W103] `SCOPE_CYCLE_COUNTER(STAT_SoldierLab_Squad/Zone)` 두 줄.
- **측정 규약**(문서 8절 · P182): AI 로깅 off · 오버레이 off · 같은 카메라 · 같은 교전 시점 · **Standalone**(이번은 PIE + 로깅 on — 차이만 믿을 것) · 의심 대상은 **세 갈래**(자기 스코프 · `.Enabled 0`
  의 World Tick 차 · 병사 수 스케일링)로 확인 · 평균과 max 같이 · 워커/GPU ms ≠ 게임 스레드 ms · 전후 stat 줄을 문서에.
- 문서: 새 1건 · `soldier_ai_lab/CLAUDE.md`(읽기 순서 · **P182~P183** · 6.2c 성능 계측 블록 — `stat SoldierLab` · `.Enabled` 7 · `.Avoidance` · 규약) · `IMPLEMENTED.md`(머리글 · 5.1 `SoldierLabLog`/`SoldierCover`
  행 · 값 블록) · `OPEN_ITEMS.md`(**+C-162 · +W98~W103**) · `CURRENT_STATE.md`(soldier_ai_lab · titan 루트) · `DOCS_INDEX.md`. `guide/` 에 병사 AI·성능 문서 없어 무변경, 별도 성능 폴더 없음(소유자가 전부 SoldierLab 안이라 `soldier_ai_lab/ai/` 에 둠).

## 2026-09-21 — 교전 오디오: 총성 끊김(보이스 32) → MaxChannels 64 + 크랙/꼬리 MetaSound + 런타임 감쇠 NaturalSound/LPF 통일 + 1인칭 사수 강조

상세: `sfx_vfx/2026-09-21_combat_audio_voice_budget_and_attenuation.md`. PIE 실청취 검증 완료(사용자 "아주 잘됨"), P4 제출 안 함.

- **증상/원인** — `L_SoldierScenario`(SoldierLab 적 15/아군 20 + UGV + 트럭) 교전 시 총성이 뚝뚝 끊김. 사용자 가설 "사운드 관리
  시스템에서 새 병사가 빠졌나"는 아님 — 총성엔 관리 장치가 원래 없었고(발소리 Concurrency·엔진 루프 Priority 90뿐), 구/신 라이플의
  `Shoot` 배선은 동일. 진짜 원인은 **발사량 ×4**(구 ≈1발/s/명 → SoldierLab ≈3.8발/s/명, 35명이면 100발/s+) × **보이스 상한 32**
  (`AudioMaxChannels=0` → 엔진 기본, 로그 `MaxSources: 32`). 총성이 전부 Priority 1·풀볼륨 동률이라 `GetSortedActiveWaveInstances`가
  새 총성마다 기존 총성을 랜덤하게 정지시키고, 원샷은 가상화 대상이 아니라 그냥 끊김.
- **수정** — ① `DefaultEngine.ini` `+QualityLevels=(…MaxChannels=64)`(재시작 후 `MaxSources: 64`). ② 총성을 MetaSound로 분리(사용자
  제작): `MSS_RifleCrack`(3샘플 랜덤·피치 ±1, Concurrency Max 16) / `MSS_RifleCrack_Enemy`(피치 -2.5~-0.5, AK 샘플 전 임시) /
  `MSS_RifleTail`(4.75s 여운, `UE.Attenuation` Distance로 Gain, `SC_RifleTail_Owner` Max 2 + `SC_RifleTail_Global` Max 8). `BP_AR4Rifle`에
  `PlayShotSounds`(Faction Switch) 함수, `Shoot`의 구 `SpawnSoundAttached` 대체. ③ 피격음·휘즈·RCWS 발사음이 원거리에서 총성보다
  또렷하던 역전 — C++ 런타임 `USoundAttenuation`이 두 값만 넣어 Linear·LPF 없음이 기본이었음 → 파일별 `*_ConfigureRuntimeAttenuation`
  헬퍼로 NaturalSound(-60dB)+LPF 2m→50m 통일(`RCWSFireControlComponent.cpp`·`RCWSProjectile.cpp`·`SoldierProjectile.cpp`, Live Coding
  검증). SoldierLab `BP_RifleProjectile` 피격음 1000/150000 → 200/2500, `BP_RCWSProjectile` 1500/25000 → 1000/15000, 레벨 UGV·트럭
  `FireSoundFalloffDistanceCm` 250000 → 150000. RCWS 발사음 공유 Concurrency Max 8 신설(`UGV_Gunshot` 2.07s×1200rpm = 41보이스).
  ④ SoldierLab `BP_RifleProjectile.SurfaceImpactEffects` 5행이 문서와 달리 `Dirt/None` 빈 껍데기 → titan 구 BP 값으로 채움(왜 비었는지
  미확인). ⑤ 1인칭: 자기 탄 휘즈는 총구↔카메라 ≤3m면 스킵(거리 기준 — 관전 폰은 Instigator로 못 잡음), 옆 아군 총성이 더 크던 것은
  새 2D 사운드 없이 SoldierLab `SA_Weapon`(정본 채택) 풀볼륨 30m → 1m + Custom 곡선 + `NonSpatializedRadius` 1~2.5m +
  `PriorityAttenuation`으로 → 내 총 : 옆 아군 ≈ 2:1. ⑥ **음속 지연**(완료 — 풀 빌드·BP 배선·PIE 검증 "성공"): 신규 `Source/SoldierLab/Weapons/
  SoldierAudioLibrary` — `SpawnSoundAtLocationSpeedOfSound`(가장 가까운 로컬 카메라 거리 ÷ 음속만큼 늦게, ≤7m는 즉시, cvar
  `SoldierLab.Audio.SpeedOfSoundCms` 34300/0=끔). 발마다 독립 람다 타이머(BP Delay·SetTimerByEvent는 버스트에서 발 빠짐, MetaSound
  Trigger Delay는 지연 중 보이스 점유), WorldContext로 오너 유지, 위치는 쏜 순간 고정. RCWS 발사음·RCWS 피격음·소총 피격음(C++) +
  `BP_AR4Rifle.PlayShotSounds` 크랙/꼬리(BP, 삭제 후 재생성 — 같은 이름 재추가 시 `_0` 남는 함정)에 적용, 휘즈 제외. **PIE 종료 크래시
  1건 수정**: 람다의 `TStrongObjectPtr<USoundAttenuation>`이 Outer 사슬로 PIE 월드를 붙잡아 `PlayLevel.cpp:553` 어설션 → 감쇠는
  `FSoundAttenuationSettings` 값 복사 + 재생 시 `NewObject(GetTransientPackage())` 재생성, 사운드/Concurrency는 `TWeakObjectPtr`.
- **보류(사용자 결정)** — 오디오 차폐(개활지), 밀도 베드, 1P 저역 레이어, 덕킹 SoundMix.
- 문서: 새 1건 + `guide/rcws_fire_control_dev_guide.md` §12.1 추기 · `sfx_vfx/hit_effects_update_2026-08-26.md` 후속 단락 ·
  `soldier_ai_lab/IMPLEMENTED.md`·`weapons/2026-09-12_projectile_port.md` 5행 서술 정정 · `OPEN_ITEMS.md` W22 · `CURRENT_STATE.md` §8 ·
  `DOCS_INDEX.md`.

## 2026-09-17 밤~18 — SoldierLab 분대 명령 층 2일차: 빌드 → 시험 레벨 `L_SoldierScenario` 첫 PIE → 수정 6건 + 틱 순환 경고 (병행 세션)

09-17 오후의 L0/L1 명령 층을 처음 빌드해 시험 레벨에서 돌렸다. 상세: `level_new_kadex_0811/2026-09-18_soldierlab_three_stage_test_level.md`(레벨·DT·검증 순서) ·
`soldier_ai_lab/squad/2026-09-18_squad_layer_fixes_quota_engage_range.md`(코드 수정·사격 게이트) ·
`soldier_ai_lab/animation/prototypes/2026-09-18_tick_cycle_warning_charmovecomp.md`(틱 순환).

- **시험 레벨** — `Content/SoldierLab/Levels/L_SoldierScenario`: 적 15(분대 1/2/3, 남서)·아군 20(분대 1~4, 중앙 바위)·
  `BP_UGV_0901`·`BP_TitanTruck`·`ugvpoint1/2`·`ASoldierZone` 4개(Z0 에 `EnemyCube` 태그). `ScenarioConfig` Demo 자동 시작,
  `bDemoForceCommandPostAutoFire=false`(안 그러면 평지에서 트럭이 t=0 부터 쏨) · `bSoldierLabHostilesStartHidden=false`(드론 없음).
  DT `DT_ScenarioSteps_SquadThreeStage` **13행** = `ThreeStage` 17행 − 드론 6 − Retarget 2 − HoldFleeingFire(표적 선호 없음 /
  `ReturnFireOnly` 가 트럭 사격에 저절로 응사) + 적/아군 행은 `IssueSquadOrder`. ⚠ Z0 가 아군에서 54 m 라 1차에 아군이 낀다 — 사용자 결정 [Q50].
- **첫 PIE 수정** — (1) 도착 불일치: 분대는 0.8r 에서 "도착", 엄폐는 r 에서 "당김 끝"이라 병사가 그 띠(+링 밖 4.5 m)에 정착해 명령이
  안 끝남 → `ASoldierZone::ArrivalFraction` 한 숫자가 명령→배정으로 흐르고 Approach 비용은 선 밖 `1 + 거리/scale`(계단이
  `MoveImprovementMargin 0.3` 을 이김), `ArrivalInsetFraction` 삭제. (2) 존 **에디터 표시**(반경·도착선·밴드 구 + 섹터 화살표 +
  스프라이트, 에디터 전용). (3) 브리지가 차량에 붙인 Identity 가 `Friendly/(none)` 분대원으로 잡혀 `SquadOrderAchieved` 가 영영 안 남 →
  `bTakesSquadOrders=false`. (4) 적 4명이 스폰에서 92 m 밖 UGV 를 쏴(시야 120 m + 산포 3° 가치 한계 ≈95 m) t≈7 s 에 교전 개시 →
  **`EngageRangeCm`**(ROE 사거리, 배정·명령·DT, `[Engage] roe=free/4000`). (5) 구 정원제·대타(09-03)를 SoldierLab 용으로 —
  `USoldierSquadSubsystem::ReinforceSquads` + DT `Quota`(`SquadId` 영구 편입, `[Squad] reinforce …`). (6) 트리거 `EnemyFireStarted` ·
  `EnemyNearFriendlySoldiers`. (7) RCWS 청각(09-17)이 SoldierLab 총성을 못 듣던 것 → 레지스트리 `OnGunshot` → 브리지 →
  `ReportGunfire(Enemy)`, 발마다(`guide/detection_dev_guide.md` §2 · `guide/rcws_fire_control_dev_guide.md` 추기).
- **`LogTick … CharMoveComp … would form a cycle` 초당 600줄** — 09-12 의 `CharacterMovement.AddTickPrerequisiteActor(self)` 가 엔진
  `UMovementComponent::bTickBeforeOwner=true`(`MovementComponent.cpp:186-188`)의 반대 엣지와 순환 → 엔진이 우리 엣지를 매 프레임
  버림(`TickTaskManager.cpp:2658`). 즉 09-12 가 "보장"이라 적은 순서는 실제로는 반대였을 가능성. CMC `Tick Before Owner=false`,
  서명 시험(`YawRate_Up=20 → BodyErr≠0`) 대기 [C-146]. 덤: 모든 소총 총구의 xyz 디버그 축(`DrawDebugCoordinateSystem`) exec 끊음.
- 미구현 제안: `FSoldierAssignment::MinStance`(접근 중 앉아 걷기 — 지금은 아는 위협이 없으면 서서 간다) [W84].
- MCP 함정: `DataTableTools.set_rows` 부분 구조체 = 나머지 리셋 · 구조체 배열은 `[]` 후 통째 · PIE 중 `add_to_scene` 거부 ·
  `save_assets` 가 읽기 전용에 false 를 조용히 반환. C++: `FObjectFinderOptional::Get()`, `SetLineThickness()`, C4458 = 에러.
- Perforce: `ScenarioStepTypes.h`·`ScenarioStateSubsystem.{h,cpp}` 는 `user2` 도 체크아웃 — 제출 시 머지.
- 문서: 새 3건 + `soldier_ai_lab/IMPLEMENTED.md`(머리글/0/5.1/2.5d-1/6절) · `OPEN_ITEMS.md`(+C-144~C-147 · +Q50 · +W84) ·
  `CURRENT_STATE.md` 09-18 병행 블록 · `CLAUDE.md`(읽기 순서·6.1 MCP 함정·6.2c cvar) · 09-17 두 문서와 09-12 틱 문서에 정정 절 ·
  `guide/` 2건 추기.

## 2026-09-21 — New_kadex_0811 본 레벨을 SoldierLab 병사로 이관: 첫 PIE 전 체인 완주 → 3분대 도주 문제 → `BreakContact` 동사 + 표적 제외 두 소비자 (분대 세션)

시험 레벨(09-18)에서 검증한 분대 명령 층을 본 레벨에 적용했다. 상세: `level_new_kadex_0811/2026-09-21_soldierlab_migration_new_kadex_0811.md`(레벨·존·DT·로그·게임 모드) ·
`soldier_ai_lab/squad/2026-09-21_break_contact_and_targeting_exclusion.md`(코드·동사 의미·제외 경로).

- **레벨(MCP, 사용자 승인)** — 구 `BP_Enemy_kadex_1~15`/`BP_Ally_kadex_1~25` 삭제(P4 백업), `BP_Soldier_Hostile_1~15`(분대 1/2/3 × 5, 첫째 리더) ·
  `BP_Soldier_Friendly_1~25`(1~5 × 5)를 **같은 트랜스폼**에. 마커 113 · 경로 스플라인 `RoadCenterline_Enemy1/2/3` · `uavpath/2` · 낙하산 · 트럭 · UGV · 드론 유지.
  **`ASoldierZone` 8개**를 마커 군집 위에(분대별 — 존이 `NavFilterClass` 를 나르므로 08-31 의 분대별 경로 필터를 살리려면 분대별 존): `Z0_S1/S2/S3_Engage`(1차, 스폰에서 280 m) ·
  `Z1_S2/S3_Withdraw`(2차, +200 m) · `Z2_S3_Escape`(3차, +360 m, 트럭 앞 37 m) · `ZF_North/South_Defend`(아군, 2차에서 60~90 m). `ScenarioConfig_1.SquadZones` Hostile 1→[Z0] · 2→[Z0,Z1] · 3→[Z0,Z1,Z2] ·
  Friendly 1~3→[ZF_N] · 4~5→[ZF_S]. 나머지 Config 그대로(Demo · `bSoldierLabHostilesStartHidden=true` · `ugvpoint1/2` · `BP_TitanTruck4`).
- **DT `DT_ScenarioSteps_ThreeStage_SoldierLab` 26행** = `ThreeStage` 26행 − `RetargetToAllies`·`RetargetToCommandPost`·`HoldFleeingFire`·`AllyAmbush` + `AllyDefend`·`AllyEngage`·`Squad3Run`·`Squad3Stand`.
  적 행 `IssueSquadOrder`: 접근 MoveTo Cautious **HoldFire** 0.3 → `EnemyEngage`(`UGVFiredNearEnemy` 10000 그대로) Occupy Rush Free 0.8 → Flee2(사망 ≥3) 2·3분대 Withdraw z1 `Quota 10` → Flee3(≥7) 3분대 z2 `Quota 5` →
  `ExcludeFleeingEnemies` SetTargetable false. 아군: `AllyDefend`(Occupy ReturnFireOnly 0.3 사거리 60 m) → `AllyEngage`(`EnemyNearFriendlySoldiers` 8000 → Free). 드론·UGV·트럭 행 그대로.
- **첫 PIE(04:38 UTC)** — `EnemyApproach` +1 → `UAVSpotted` +82 → `UGVArriveZone1` +170 → `EnemyEngage` +180 → `EnemyFleeToZone2` +210(`[Squad] reinforce Hostile quota=10 living=7 needed=3 moved=3`) →
  `AllyEngage` +260 → `UGVMoveZone2` +274 → `EnemyFleeToZone3` +350(`quota=5 … moved=1`) → `CommandPostFire` → `ExcludeFleeingEnemies`. 전부 순서대로, 사용자 "아주 잘됨".
- **문제** — `ExcludeFleeingEnemies` 뒤에도 `[RCWSFireControl] BP_UGV_0901_C_1: 타겟 BP_Soldier_Hostile_C_4/6/3 …`, 3분대는 2차 존 엄폐를 홉하며 아군과 교전. 원인 셋: (1) 구 이펙트가 켜 주던 **UGV RCWS
  `bRespectEnemyTargetingExclusion=true`**(인스턴스 기본 false, 트럭은 설계상 false)를 `SetTargetable` 경로가 안 켬 (2) SoldierLab 아군 보병이 제외 플래그를 **아예 안 읽음** (3) `Withdraw`+`ReturnFireOnly` 는
  전투 이동 — 엄폐 층이 매 홉을 싸울 자리로 값 매김 + dwell + 응사.
- **코드** — `Squad/SoldierOrderTypes.h` **동사 `BreakContact`**(존 유지, ROE/사거리/속도 동반, 다음 존 동사가 지움) + `bBreakContact` · `SoldierSquadSubsystem.cpp` `BuildAssignment`/라벨 ` BREAK`/` excl` ·
  `AI/SoldierCover` `IsBreakingContact()`(`bBreakContact && Mode==Approach`) → `ScorePosition` 의 Fighting/Route/Danger/Suppression **0**, dwell 없음 · `AI/SoldierEngagement` `IsContactExcluded`(상대 배정
  `!bTargetableByOwnSideWeapons` → 후보 제외·잠금 해제, **인지는 그대로**) · 이동 중 스프린트 · 자세 0 · `UI/ScenarioStateSubsystem.cpp` `IssueSquadOrderSpec`: `SetTargetable(false)` → UGV RCWS 스위치 ON.
  DT `Squad3Run`(Flee3 +6 s, BreakContact HoldFire Rush) · `Squad3Stand`(트럭 80 m 사격 → Occupy z2 Free). 원칙 P184(도주는 ROE 가 아니라 땅의 가격) · P185(제외는 인지가 아니라 표적 목록).
  빌드 함정: `UFUNCTION` 없는 private 헬퍼를 다른 컴포넌트에서 → C2248(public 으로).
- **게임 모드** — `GM_SoldierLab` 에서도 시나리오는 돈다(GameInstance 서브시스템 · UGV 는 titan PC CDO 폴백 · GameState null 가드). 잃는 것: RTSP · HUD/미니맵 · 토스트 · 데모 플래그 복제.
- 문서 세션 발견: **RCWS 스티키 표적이 제외를 안 본다**(`RCWSFireControlComponent.cpp:584-613` — 제외 순간 물고 있던 한 명은 시야 1 s 잃어야 놓음) → [W106]. 드론 프레이밍은 아직 구 `LastStandZoneIndex` 를 읽어
  SoldierLab 적군엔 필터 통과(코스메틱) → [W104]. 구 `EnemyCombatComponent` 시나리오 기제는 이 레벨에서 죽은 코드 → [W105](`kadex_test` 결정 뒤). `MinStance` 는 SoldierLab 쪽은 09-18 에 구현됐고 titan DT 연결만 남음([W84]).
- Perforce: `New_kadex_0811.umap` 은 `user2` 도 체크아웃(바이너리 — 순서 조율). `Source/` 트리는 넓은 `p4 edit` 로 거의 전부 opened — opened 가 변경 신호가 아니다.
- 문서: 새 2건 + `scenario_three_stage_combat.md`/`scenario_authoring_guide.md`(09-21 배너 + **2.6절** SoldierLab 시대 행·필드·존·트리거) · `ai_combat/` 08-31·09-03 두 문서 superseded 배너 ·
  09-17 연결·09-18 시험 레벨·09-18 분대 수정 문서에 추기 · `soldier_ai_lab/IMPLEMENTED.md`(머리글·0·5.1·값) · `OPEN_ITEMS.md`(+C-163~C-164 · +Q51 · +W104~W106 · −W70) · `CURRENT_STATE.md` ·
  `CLAUDE.md`(읽기 순서 · P184~P185) · `guide/rcws_fire_control_dev_guide.md` **3.3절** 신설 · `guide/detection_dev_guide.md` §2 추기 · 루트 `CURRENT_STATE.md` §6·§7.

## 2026-09-21 — SoldierLab 게임 스레드 묶음: `stat dumpframe` 로 이름 붙인 AI 카메라 2.0 ms 제거(World Tick 22.6 → 19.4/18.4) · ABP cvar 폴링 · 총구 Niagara 누수 · URO 크래시로 폐기 → 되돌림 · 재기동 후 실측 18.3/20.3 · 투사체 영구 누적 발견 (성능 후속 세션)

성능 계측 세션의 인계표 [W98]~[W103] 실행. 상세: **`soldier_ai_lab/ai/2026-09-21_game_thread_batch_cameras_abp_muzzle.md`**(새 문서). 조건: `L_SoldierScenario` 적 15/아군 20 PIE(일부 New_kadex_0811 40명).
⚠ ID: 처음 잡은 W104~W106·C-163·P184 는 분대 세션이 선점 → **W107~W109 · C-165 · P186**.

- **계측법 신설** — MCP 엔 콘솔 툴이 없어 사용자가 PIE 콘솔에 **`stat dumpframe -ms=0.05`** → `Saved/Logs/titan_example.log` 의 `LogStats:` 블록(≈ 3000줄, 스레드별 스탯 계층)을 파싱하면
  **틱 함수별(클래스·컴포넌트별) ms** 가 전부 나온다 — Insights 없이 "컴포넌트 틱" 안의 무명 ms 에 이름을 붙이는 가장 싼 방법(`stat SoldierLab` 은 우리 코드만). 이후 전후 비교 전부 이 방법.
- **전편 오독 정정 셋 [A]** — (a) `BlueprintUpdateAnimation` 68 calls 는 병사당 ABP 2개가 아니라 **GT 34 + 워커 34**(`AnimInstance.cpp:795` 와 `AnimInstanceProxy.cpp:1354` 가 같은 스탯 스코프),
  실제 GT 이벤트 그래프 병사당 29~40 µs; PP ABP 는 None, `obj list` 82 = PIE 40 + 에디터 월드 40 + 프리뷰 2 (b) `KinematicBonesUpdateType=SkipAllBones` **채택 불가** — 피격 부위가 메시 바디 트레이스
  (`SoldierHealth.cpp:289-305`) (c) BeginPlay `SetVisibility(false)` 는 **스폰된 총 액터의 WeaponMesh**(self ← SpawnActor)에 걸리고 **캐릭터 WeaponMesh 가 보이는 총**; 그 AnimClass `ABP_Weap_Rifle` 은 본 0개
  일치(`Skeleton.cpp:648`)라 죽은 참조 → None.
- **새로 이름 붙은 비용**(37명) — **GASP GameplayCamera 1.60 + SpringArm 0.38 = 2.0 ms**: AI 병사 40명 전원이 스탠드얼론 카메라 시스템을 매 틱 평가(`bAutoActivate`+`bRunStandaloneCameraSystem` CDO 상속 →
  `GameplayCameraComponentBase.cpp:552-568` · `:622-657`, `SpringArmComponent.cpp:197` 스윕) · 캐릭터 BP ReceiveTick 2.36 + PreCMC 0.94 · CMC 1.86 · CharacterMesh0 1.61 · 숨은 총 메시 0.19+0.34 ·
  `AWindSource` 0.95(MPC 11개 매 프레임) · titan TargetDetection ×3 1.2 · **PIE 전용** Landscape 5.7/Slate 5/뷰포트 3. 병사 컴포넌트 38개(CDO 29 + PIE 카메라 프록시/프러스텀 6 + 3).
  **사격 발당 ≈ 1.1 ms**(투사체 스폰 0.57 + 총구 `SpawnSystemAttached` 0.38 + Launch 0.08 — 전편 0.65 는 평균) · `SpawnSystemAttached` `bAutoDestroy=false` 로 **Niagara 컴포넌트 영구 누적 누수**(8정에 106개).
- **수정 ✅ PIE** — `BP_SoldierCharacter` BeginPlay 끝 `not IsPlayerControlled` → GameplayCamera · SpringArm · Camera(NotUsedByDefault) **틱 off**(Deactivate 아님 P107; 관전 폰은 자기 폰을 옮겨 병사 카메라 안 빌림)
  → 카메라 틱 0건, **World Tick 22.6 → 19.4/18.4** · ABP `Update_CVarDrivenVariables`(cvar 7 문자열 조회 + `ComponentHasTag` 2 매 틱) → Initialize 1회 + `CVarPollSeconds` ≥ 1.0(1 Hz): 29 → 22 µs,
  CharacterMesh0(아군 20) 0.834 → 0.616(함정: MCP `create_node` 가 GASP 원본 클래스 동명 함수를 잡음 → `declaring_class`) · `BP_AR4Rifle.WeaponMesh` `OnlyTickPoseWhenRendered` 0.26 → 0.07.
- **수정(측정 전)** — 캐릭터 BP 변수 `AnimBP` 캐시 + `SetUseAllyAnimSet` BeginPlay 이동 + Tick 캐스트 제거 · `BP_AR4Rifle` 상주 **`MuzzleFlashFX`** NiagaraComponent + `Shoot`/`PlayShotCosmetics` 의
  `SpawnSystemAttached` → `Activate(bReset)`(누수 해결, `BP_AK47Rifle` 상속 확인).
- ⛔ **URO 폐기** — `bEnableUpdateRateOptimizations=true`(부모 CDO + 자식 2 + 인스턴스 35) → 검증 PIE **약 2분 만에 에디터 크래시** `!ParentBone.ContainsNaN() [BonePose.h:645] Pose[1] … -inf`
  (직전 `[SoldierIdentity] … chest stand 1.17e18 cm` + Chaos bounds ensure). 추정: GASP MM/`OffsetRootBone`/`DeadBlending` 이 건너뛴 프레임을 상정 안 함 [C-165]. 에디터가 죽어 못 되돌림 →
  `DefaultEngine.ini [ConsoleVariables] a.URO.Enable=0` 안전장치(`SkinnedMeshComponent.cpp:1810` 플래그 AND cvar) → **재기동 뒤 플래그 false + cvar 삭제 [W107] 먼저**. 원칙 **P186**.
- **C++ 4건 빌드 대기(사용자 빌드)** — `AI/SoldierHealth.cpp`·`AI/SoldierComms.cpp` `TickInterval 0.1` · `Pose/SoldierHeadAimComponent.cpp` H off ∧ AI ∧ 정지면 조기 반환(0.18 ms/35명) ·
  `Squad/SoldierSquadSubsystem.cpp`·`Squad/SoldierZone.cpp` `SCOPE_CYCLE_COUNTER` **[W103] 해결** · `titan_example/Environment/WindSource.h/.cpp` **`TargetPushIntervalSeconds 0.05`**(20 Hz, 0.95 → ≈ 0.05; 헤더 = 정식 빌드).
- **남은 ms**(35명, World Tick ≈ 20) — 사격 스폰 2.2 · 캐릭터 BP 2.8 · SoldierLab 3.5 · 애니 GT 2.6 · CMC 1.8 · 물리 1.4 · 투사체 0.7 · 틱 오버헤드 0.95 … **현실적 바닥 ≈ 13~14**
  (풀링 [W109] −2 · BP 틱 C++ [W108] −1.8 · [W102] −0.9 · 애니 −1.2 방법 재검토 · 솎기 −0.3). 조율: 리플리케이션 세션이 같은 시간에 Engagement·ScanTurn·Projectile·Detectable·캐릭터 Tick·`BP_AR4Rifle` 편집 — BeginPlay 이쪽 · Tick 저쪽.
- 문서: 새 1건 · 전편 `perf_instrumentation_and_cover_cost.md` **10절 정정** · `soldier_ai_lab/CLAUDE.md`(읽기 순서 · **P186** · 6.2c `stat dumpframe` · `a.URO.Enable`) · `IMPLEMENTED.md`(3절 WeaponMesh 정정 · 5.2 `BP_SoldierCharacter`/`BP_AR4Rifle`/ABP) ·
  `OPEN_ITEMS.md`(**+C-165 · +W107~W109 −W103**, W98/W99/W100 갱신) · `CURRENT_STATE.md`. `guide/` 에 병사 AI·성능 문서 없어 무변경.
- **후속(16:55~17:05, 같은 문서 10~13절)** — ✅ **[W107] 해결**: 사용자 정식 빌드(새 DLL 16:55) + 재기동 뒤 MCP 로 `bEnableUpdateRateOptimizations=false` 를 CDO 3 + `New_kadex_0811` 인스턴스 40(로드 시 true) 에, BP 3 저장,
  `a.URO.Enable=0` 줄 제거(ini 원본과 동일 → P4 revert). **실측**(`L_SoldierScenario` 35명 교전 2프레임, World Tick **18.3 / 20.3**): 카메라 틱 0 · HeadAim/Health/Comms 틱 목록 소멸 · `Shoot` 발당 **1.1 → 0.94**
  (`SpawnSystemAttached` 0건, 남은 0.94 = 전부 투사체 스폰) · 캐릭터 BP ReceiveTick 1.21 변화 없음 · `WindSource` 는 이 레벨에 없어 미측정 → 이 배치의 실제 ms 는 **카메라 2.0 + HeadAim 0.2 + 총구 0.4/발**.
  **레이트레이스 소유자별**(1,030~1,119회/프레임 2.6~3.0 ms): Cover Candidate 0.8~1.1 · Begin Sweep 0.36~0.55 · **Engagement 0.46**(`IsShotBlockedByWorld` ≈ 5.5회/병사/프레임, 호출마다 병사 35명 ignore 목록 — `SoldierEngagement.cpp:371-417`) ·
  Sight 0.35 · CMC `FindFloor` 0.30(`bAlwaysCheckFloor=true`). ★ **투사체 발견 [A]** — `ASoldierProjectile` 은 풀 설계(`SoldierProjectile.h:7`, `Deactivate` 는 숨김만)인데 `BP_AR4Rifle.Shoot`/`PlayShotCosmetics`·도탄 `Multicast_LaunchRicochet` 이
  발마다 `SpawnActor` → 비행 21발에 `TracerTrailComponent` **80개 영구 누적**; `BP_RifleProjectile` 빈 `EventTick`(30발 0.25 ms); 콜리전 `QueryAndPhysics`(QueryOnly 가능). 문서: `OPEN_ITEMS.md`(**−W107 · +W110~W113**, W109 착수·보강, W108 갱신) ·
  `CURRENT_STATE.md` · `IMPLEMENTED.md` 3.1절 한 줄. 리플리케이션 세션 완료 — 2PC 검증은 내일(사용자).

## 2026-09-21 17:10~18:10 — SoldierLab 게임 스레드 구조 묶음(후편): 투사체 풀 · 레이 채널 응답+캐시 · CMC/메시 데이터 · 박자 시간 고정 · 캐릭터 BP 틱 → C++ — World Tick 18.3/20.3 → 11.3/14.0 (오늘 누적 22.6 → 11~14, 사용자 "교전 중 50fps 후반")

위 세션 13절의 후보 순서 ①~⑤ 실행. 상세: **`soldier_ai_lab/ai/2026-09-21_game_thread_structural_pool_rays_bridge.md`**(새 문서). 조건: `L_SoldierScenario` 적 15/아군 20 PIE, `stat dumpframe` 파싱, 전부 [A] + 사용자 PIE 확인. ID **W114~W115**, 새 C·P 없음.

- **① 투사체 풀 [W109] ✅** — 신규 `Source/SoldierLab/Weapons/SoldierProjectilePool.h/.cpp` **`USoldierProjectilePoolSubsystem::Acquire(Class, Owner)`**: 클래스별 풀, 주차된(틱 꺼진) 투사체를 커서부터 재사용 → 없으면 상한(cvar
  `SoldierLab.Projectile.PoolMax` 96)까지 스폰 → 상한이면 RCWS 처럼 라운드로빈(클래스당 1회 경고). `BP_AR4Rifle.Shoot`·`PlayShotCosmetics`: `SpawnActor+MakeTransform(+Cast)` → `GetSoldierProjectilePoolSubsystem → Acquire → LaunchFrom`,
  도탄 `Multicast_LaunchRicochet_Implementation` 도 `Acquire`(튕길 때마다 1개씩 새던 것 해결). `CollisionComponent` **QueryAndPhysics → QueryOnly**(`SoldierProjectile.cpp:124`, RCWSProjectile 선례 — 명중은 이동 스윕 블로킹 히트라 `OnComponentHit` 그대로,
  "명중 잘됨") · `BP_RifleProjectile` 빈 `EventTick`·BeginPlay·Overlap 삭제 · 신규 **`MaxFlightDistanceCm 60000`**(사용자 제안 — 800 m/s 탄이 시간 상한 5 s 만으로는 빗맞힐 때 ≈ 4 km, 교전 정지 95 m · 레벨 ≤ 500 m; 시간 상한과 같은 자리에서 주차, 도탄은 튕긴 지점부터).
  결과: `Shoot` 안 스폰 **0.83 ms/발 → 0**, `FEndPhysics` 1.03 → 0.62/0.46, 투사체 액터 **47 에서 정지**(비행 15), 경고 0, World Tick 18.3/20.3 → **17.1/14.3**.
- **② 레이트레이스 [W110] ✅** — 신규 `AI/SoldierQuery.h` **`SoldierQuery::BodiesAreNotWalls()`**(`FCollisionResponseParams` 의 **ECC_Pawn → Ignore**): 병사는 캡슐·메시 모두 ECC_Pawn 이고 이 레벨들의 Pawn 오브젝트 타입은 병사뿐(MCP 확인 — 트럭 `BodyMesh`
  ECC_Vehicle·`WindowMesh` WorldDynamic, UGV `VehicleMesh` ECC_Vehicle, 드론 `CollisionBox` ECC_PhysicsBody, 총 메시 2종 NoCollision). 레지스트리 35명 `AddIgnoredActor` 루프를 **Engagement 1 · Cover 5(`IgnoreBodies` 는 주석 남기고 빈 함수) · Field 2** 사이트에서 제거.
  `IsShotBlockedByWorld` 결과 캐시: 새 튜닝 **`LaneCacheMoveCm 15` · `LaneCacheSeconds 0.15`**, 8슬롯 링 `FLaneAnswer`(mutable) — `PlanAperture` 옵션 트레이스도 자동으로 캐시. 결과: 씬 쿼리 **1,030~1,120회 2.6~3.0 → 357/576회 0.96/1.42 ms**, Engagement 레인 **182~188회 0.46 → 23/35회 0.07/0.10**,
  Engagement 틱 0.67~0.82 → 0.22/0.17, Cover 후보 트레이스 단가 2.4 → 1.9 µs. `[Engage]` 판정 불변.
- **③ 데이터 [W111] ✅ + 정정** — CMC `bAlwaysCheckFloor=false`(정지 병사 바닥 스윕 생략: `FindFloor` **99~105 → 44/27회**) · `bEnablePhysicsInteraction=false`. ★ **병사 메시 콜리전 `QueryAndPhysics → QueryOnly`** — 문서(전편 [W98] ② · `IMPLEMENTED.md`)는 QueryOnly 라
  적었으나 **실제 CDO 는 GASP 커스텀 프로파일 QueryAndPhysics 였다**(정정). 사망 래그돌은 `SoldierHealth.cpp:572 StartRagdoll` 이 QueryAndPhysics 로 되돌리므로 안전, 피격 부위 트레이스는 Query. `FEndPhysics` 효과는 잡음에 묻혀 [B]. 부모 CDO + 자식 2 CDO + `L_SoldierScenario` 인스턴스 35 적용·저장.
- **⑤ 박자 [W102] ✅** — `SoldierCover` **`CandidateIntervalSeconds 0.033`** + `LastCandidateStepSeconds`(눈 있는 스윕의 후보 걸음) · `SoldierSight` **`ScanIntervalSeconds 0.033`** + `LastScanSeconds` — **프레임이 아니라 시간에 고정**(30 fps 거동·비용 동일, 60 fps 에서 트레이스 절반,
  값을 올리면 결정 지연 ↔ 비용). Score 캐시는 눈 세대마다 바뀌어 생략.
- **④ 캐릭터 BP 틱 → C++ [W108] ✅** — 신규 `Source/SoldierLab/Pose/SoldierAIBridgeComponent.h/.cpp` **`TickBridge(DeltaSeconds)`**: `BP_SoldierCharacter` EventTick 본문(조준 보정 회전 수학 → 린 램프 → 입력 상태 구조체 Sprint/Aim 직접 쓰기(`UpdateInputState_Server` 본문 =
  `SetCharacterInputState` 라 권한에선 동치) → AOActive/AIPoseDriven/AI 다리 5개 → `UpdateBodyYawRate`/`UpdateBlindFire`/`UpdateStance` → WantsToFire/Reload → Rifle `Shoot`/`StartReload` 이벤트)을 **노드 순서·Kismet 산술(RLerp/ComposeRotators/NormalizedDeltaRotator/MapRangeClamped/InRange) 그대로** 이식.
  변수 47개(캐릭터 34·부모 4·ABP 7)는 **BP 소유 그대로 FProperty 로 읽고 씀**(포즈 스무더·게이트브리지·리플리케이션 분기가 같은 변수를 쓰므로 소유권 불변). BeginPlay 바인딩 실패 시 경고 + BP 원본 경로, cvar `SoldierLab.AIBridge.Native 0` 으로 A/B.
  BP: 컴포넌트 `AC_SoldierAIBridge` 추가, EventTick = `Parent:Tick → TickBridge → Branch(ShouldRunBlueprintCopy) → (true) 옛 본문`(리플리케이션 세션의 `HasAuthority` 분기·바운드 이벤트 2 는 옛 본문 안에 그대로).
  결과: 아군 20명 ReceiveTick **1.16~1.21 → 0.22 ms**(적은 목록 밖), World Tick 15.1/15.1 → **11.3/14.0**, 바인딩 경고 0, 사용자 "잘된다, 교전 중 50fps 후반".
- **누적·남은 것** — 오늘 22.6(원 29) → **11~14 ms**: 카메라 −2.0 · 총구 −0.4/발+누수 · 풀 −0.8/발+누수 · 레이 −1.5~2 · BP 틱 −1.6 · 기타 −0.5. 남은 상위(교전 프레임): Cover 1.2~2.6(점수) · CMC 1.3 · 메시 틱 1.1 · `AC_PreCMCTick` 0.8(GASP, C++ 이관 후보 **[W115]**) ·
  틱 오버헤드 1.2(틱 함수 ≈ 600 → [W112]) · Sight 0.9 · 물리 0.6 · [W113]. 별건 발견: 재장전 몽타주의 GASP 노티파이 `AN_Reload`(`Content/Characters/Heroes/Abilities/AN_Reload`)가 어빌리티 컴포넌트 없는 병사에 `GameplayEvent.ReloadDone` → `LogAbilitySystem: Error` 41회/PIE(기존, 비용은 로그뿐) → **[W114]**.
- 문서: 새 1건 · 전편 `game_thread_batch_cameras_abp_muzzle.md` **14절 후편 참조**(헤더 완료) · `OPEN_ITEMS.md`(**−W100 −W102 −W108~W111 · +W114~W115**, W112·W113 갱신) · `CURRENT_STATE.md` · `IMPLEMENTED.md`(3.1 풀·QueryOnly·거리 상한 · 3.2 메시 콜리전 정정 인용 · 5.1 C++ +3 + 튜닝값 · 5.2 `BP_SoldierCharacter` 컴포넌트 +1·EventTick 구조) ·
  `DOCS_INDEX.md` · `soldier_ai_lab/CLAUDE.md` 읽기 순서. `guide/` 에 병사 AI·성능 문서 없어 무변경. 리플리케이션 세션 완료 — 2PC 검증은 내일(사용자).

## 2026-09-22 — 시나리오 재시작(확인창 + 자동 재시작) 구현: 병사 재스폰 + 차량·드론 제자리 부활 + SoldierLab 리셋 계약, L_SoldierScenario PIE "완벽"

09-10 설계(v3, 09-22 전면 개정 — `level_new_kadex_0811/2026-09-10_scenario_auto_restart_design.md`)를 그대로 구현.
상세: **`level_new_kadex_0811/2026-09-22_scenario_restart_implementation.md`**. 요구: 적 전멸 5초 뒤 "재시작 하시겠습니까?"
확인창(예/아니요/☐ 자동 재시작), 체크 시 10초 카운트다운, 체크값 유지, 결과는 PIE 새로 시작과 동일. **레벨 리로드 없음**(사용자 명시).

- **흐름** — DT 행 `ScenarioRestartPrompt`(Prereq `ScenarioComplete`, TimerOnly 5 s, 신설 이펙트 `EScenarioEffectType::ShowRestartPrompt`)
  → GameState `Multicast_ShowScenarioRestartPrompt` → 자기 축이면 `UNotificationSubsystem::ShowRestartPrompt` → `WBP_RestartPrompt`(사용자 제작,
  부모 C++ `URestartPromptWidget`, BindWidget `YesButton`/`NoButton`/`AutoRestartCheckBox` + 선택 `YesLabel`("예 (7)")/`MessageText`) →
  **`UScenarioStateSubsystem::RequestScenarioRestart()` 유일 진입점**(확인창·카운트다운·콘솔·향후 HUD 버튼) → 확인창 닫기(멀티캐스트) + 스텝 루프
  ClearTimer → 페이드 아웃 0.3 s → `ExecuteScenarioReset`(탄 주차, 토스트 정리, 재스폰 시작, SoldierLab/titan 서브시스템 리셋, GameState 적 예상
  위치/페이즈 초기화, `IScenarioResettable` 전부 호출) → 8구/프레임 재스폰 완료 → 브리지 `ForceRescan`(적 hidden) + 검증(등록부/탐지 레지스트리 수,
  실패 시 에러 + 확인창 재표시) → `ApplyDemoRunModeSetup()` + 3 s 뒤 `BeginEnemyContactScenario`(레벨 시작과 같은 경로) → 페이드 인 0.5 s →
  사이클 카운터/소요 ms 로그.
- **병사·낙하산은 재스폰** — 신규 `UI/ScenarioRespawnSubsystem`(월드 서브시스템): `OnWorldBeginPlay`(UE5.8 은 액터 BeginPlay **이전**)에
  `USoldierHealthComponent` 보유 액터 + `AScenarioConfig::RespawnActors` 의 클래스/트랜스폼/**저작 델타**(EditAnywhere 중 아키타입과 다른 값
  ExportText — 실측 `AC_SoldierIdentity.SquadId/bSquadLeader`, 아군 `AC_SoldierHealth.bInvincible`) 스냅샷 → 재시작 시 병사 + **딸린 액터**(소총
  `BP_AR4Rifle` 등, `GetAttachedActors` 재귀 + Owner, 풀링 투사체 제외) Destroy → `bDeferConstruction` 스폰 + **`AutoPossessAI=PlacedInWorldOrSpawned`**
  로 바꿔 `FinishSpawning`(BeginPlay 전 AI 빙의) → ImportText 델타 복원.
- **UGV/트럭/드론은 제자리 부활** — 신규 인터페이스 `IScenarioResettable`(`UI/ScenarioResettable.h`, `ResetForScenarioRestart`). UGV: `AUGVAIController`
  리셋(경로/도착 정렬/적분기/속도 제한/드라이브 모드·시동) → 스폰 트랜스폼 텔레포트 + Chaos `ResetVehicleState` → `URCWSComponent`(탄약 AmmoMax
  리필·마운트 원위치·줌/카메라·발사 모드·장전) → `URCWSFireControlComponent`(모드 Remote·표적/락온/기억/총성조사/배럴스핀/발사사이클·
  `bRespectEnemyTargetingExclusion`·ARM 초기값, `ShotsFiredCount` 유지). 트럭: RCWS 둘. 드론: 수동 해제 → Autopilot Disengage → `Flight->ResetTo(스폰)`
  → 짐벌 원위치 → 정찰 단계/`bParachuteObserved`/프레이밍/탐지 단계 초기값.
- **SoldierLab 리셋 계약(그 모듈 소유)** — `USoldierSquadSubsystem::ResetForRestart` · `USoldierSituationFieldSubsystem::ResetForRestart`(Built* 무효화 →
  `EnsureLevels` 첫 실행 경로, **지오메트리 캐시까지** — 설계의 "유지"는 폐기) · `USoldierProjectilePoolSubsystem::RecallAll` + `ASoldierProjectile::Park` /
  `ARCWSProjectile::Park`. 콘솔 `SoldierLab.ResetWorld`. 원칙 P187(서브시스템 새 상태는 리셋에도) · [W116](`soldier_ai_lab/OPEN_ITEMS.md`).
- **확인창이 뜨는 화면**(사용자 확정) — UGV 호스트 + 자체방호 클라 → **자체방호에만** / 자체방호 단독 → 자체방호 / UGV 단독 → UGV / 데모·풀
  무관. 서버가 자체방호 접속 여부를 판정해 `Multicast_ShowScenarioRestartPrompt(Countdown, bShowOnUGVAxis)` 로 뿌리고, 클라의 [예]/카운트다운/체크값은
  `Atitan_examplePlayerController::RequestScenarioRestart / SetScenarioAutoRestart`(Exec → `Server_*` RPC)로 서버에. 체크값 정본 = 서버 GameInstance
  서브시스템 + `GameUserSettings.ini [Scenario] bAutoRestart` + 커맨드라인 `-autorestart`, 클라는 GameState `bScenarioAutoRestart` 리플리케이트.
  콘솔 `titan.ScenarioRestart`(확인창 없이) / `titan.ScenarioAutoRestart <0|1>`. `AScenarioConfig` `Scenario|Restart` 6필드
  (`bRestartPromptEnabled` · `AutoRestartCountdownSeconds 10` · `RestartFadeOutSeconds 0.3` · `RestartFadeInSeconds 0.5` · `RestartRespawnPerFrame 8` · `RespawnActors`).
- **무한 재시작 방지** — `ScenarioComplete` 가 Prereq `EnemyEngage` 를 물어 새 사이클에서 UGV/적이 다시 쏘기 전엔 완료 불가(구조적 루프 브레이커).
  `EnemyFleeToZone2`(사망 ≥ 3) 오발동은 "루프 먼저 끄고 재스폰 + 3 s 뒤 시작" 순서로, `AllyEngage`(80 m) 오판정은 시체를 재스폰 전 Destroy 해서 차단.
- **1차 테스트 결함 2건 → 수정** — (1) 재스폰 병사가 안 움직임 = `BP_Soldier_*` `AutoPossessAI=PlacedInWorld` 라 스폰된 폰에 컨트롤러 미부착 → 위
  지연 스폰 + `PlacedInWorldOrSpawned`. (2) GT 지연이 사이클마다 두 배 + `BP_AR4Rifle … OwningCharacter is not valid` 스팸 = 병사만 Destroy 하니
  소총 액터가 남아 죽은 주인 참조 → 딸린 액터 동반 파괴. 수정 후 **L_SoldierScenario PIE 사용자 판정 "완벽하다"**.
- **남은 것** — New_kadex_0811 `ScenarioConfig_1.RespawnActors=[BP_Parachute_C_3]` 연결(맵은 user2 도 체크아웃 중) + 그 레벨 PIE 검증 · 10사이클
  메모리/fps 추이 · 2-PC 검증(확인창 축·RPC) · 평상시 사망 `DestroyAfterSeconds` 뒤 소총 잔존 여부(SoldierLab BP, [W116]).
- Perforce: 워크스페이스 `user4_DESKTOP-81S78B2_4340`, 소스 ~35파일 edit + 5파일 add(`UI/ScenarioResettable.h`, `UI/ScenarioRespawnSubsystem.h/.cpp`,
  `UI/RestartPromptWidget.h/.cpp`) + DT 2개(`DT_ScenarioSteps_ThreeStage_SoldierLab`, `DT_ScenarioSteps_SquadThreeStage`) edit, 미제출. 일부는 user2 동시 체크아웃.
- 문서: 구현 문서 갱신(완료 상태) · 설계 v3 헤더 + "구현에서 달라진 점" 배너 · `scenario_authoring_guide.md` 2.7절 신설(이펙트/필드/콘솔/트러블슈팅) ·
  `guide/rcws_fire_control_dev_guide.md` 8.10절 · `guide/detection_dev_guide.md` §2 · `guide/ui_dev_guide.md` · `soldier_ai_lab/`(CURRENT_STATE · IMPLEMENTED ·
  OPEN_ITEMS W116 · CLAUDE.md P187 + `SoldierLab.ResetWorld`).

## 2026-09-22 — LIG 지도 지리참조 요청 대응 + 08-26 재스케일 사후 문서화

LIG 가 시뮬레이션 지도 이미지의 **픽셀 스케일(ScaleX/Y/Z) + 타이포인트(I,J,K,X,Y,Z)** 를 요청(시뮬레이션개발팀 경유). 상세
**`protocol/2026-09-22_minimap_georeference_for_lig.md`**.

- 현재 미니맵 `m_map.png`(1024×1024, `/Game/widget/m_map`, 2026-07-30 임포트) 재확인 — WGS84 위경도 정렬(EPSG:4326), 콘텐츠
  행 47~977(레터박스), `GeoCoordinateUtils.h` 코너 상수와 픽셀 스캔 일치. 원본 PNG 는 `Downloads\m_map.png`(== `Documents\SIMPLEXI
  받은파일\m_map.png`), `m_map_0730.png` 은 별도 익스포트지만 지리참조 동일.
- 확정값: ScaleX 0.000055534311 · ScaleY 0.000047961398 deg/px(≈4.88/5.32 m, 비정사각), 타이포인트 (0,0)→(128.1581718, 37.9396738) /
  콘텐츠 (0,47)→(128.1581718, 37.9374196), 네 꼭지점 WGS84+UTM 52S, 격자 수렴각 -0.50°(UTM 무회전 아핀이면 모서리 30~40 m 오차)
  주의. 교차 확인에서 랜드스케이프 `+X-Y` 코너가 픽셀 열 1092.6 — 08-26 의 동쪽 오버행이 그대로임을 재확인.
- 답장 초안 작성(부록), 발송 대기. 지도 재생성(동쪽 확장·UTM 정렬·정사각 픽셀·레터박스 제거)은 별도 제안 — 하면 코드 상수 8개 재측정.
- **문서 공백 메움**: 08-26 재스케일이 devlog 없이 지나가 다른 세션이 몰랐던 문제 → `level_new_kadex_0811/2026-08-26_level_rescale_to_real_world.md`
  사후 작성, `guide/real2world.md` 전면 재작성(옛 스크래치 메모 → 현재 동작 레퍼런스), `CURRENT_STATE.md` §6/§11/§12 · `DOCS_INDEX.md` 반영.

## 2026-09-23 — 시나리오 재시작 2-PC 검증: 최종 원인은 레벨 GameMode 오버라이드(`GM_SoldierLab`) + 멀티캐스트 3종 · 드론 권한 분리 · 소총 누수 근본 수정 → "이제 잘됨"

09-22 구현분을 **New_kadex_0811 2-PC(UGV축 호스트 + 자체방호축 클라이언트)** 에서 검증. 상세는 같은 문서
**`level_new_kadex_0811/2026-09-22_scenario_restart_implementation.md` §5**(2026-09-23 절).

- **증상** — 호스트는 정상인데 **클라이언트만** ① 카메라 페이드가 전혀 안 되고 ② 재시작 후 드론이 원위치로 안 돌아왔다(+ 1차 테스트와 같은 소총 에러 스팸).
- **★ 최종 원인: 레벨 World Settings 의 GameMode 오버라이드가 `GM_SoldierLab` 로 바뀌어 있었다**(2026-09-21 SoldierLab 이관 때 들어옴).
  `GM_SoldierLab` 의 부모는 순정 `AGameModeBase` 고 `GameStateClass=AGameStateBase` · `PlayerControllerClass=APlayerController`(실측). 그래서
  (a) `Atitan_exampleGameState` 가 없어 재시작의 **모든 멀티캐스트가 서버-로컬 폴백**으로 빠져 클라가 페이드·정리·리셋을 하나도 못 받고
  (b) `Atitan_examplePlayerController` 가 없어 `PlayerAxis`/데모 플래그가 안 정해져 `ADronePawn::ResolveShouldSimulateDrone` 에서 서버와 클라가
  **둘 다 시뮬 주체**가 되어 각자 물리를 돌렸다(애초에 동기화가 아니었음). **단일 프로세스에서는 폴백이 곧 정답이라 L_SoldierScenario 는 멀쩡했다 — 2-PC 에서만 깨진다.**
  해결: 오버라이드를 원래대로 **`BP_KadexTestGameMode`**(부모 `Atitan_exampleGameMode`)로 되돌림 → 사용자 **"이제 잘됨"**. 근거 = P4 `Content/New_kadex_0811.umap#43`(09-18, 이관 직전)엔
  `BP_KadexTestGameMode` 하나뿐, `GM_SoldierLab` 은 #44(09-21) 이후. 레벨별: `New_kadex_0811`·`kadex_test` = `BP_KadexTestGameMode`, `kadex_lobby` = `BP_TestGameMode`(대기실, 무관).
  **운용 규칙**: `GM_SoldierLab` 은 병사 거동 자유 관전용 — **단일 프로세스에서만**. 2-PC·전시 구성은 항상 titan 계열 GM.
  진단 경고 추가: 재시작 시 GameState 가 `Atitan_exampleGameState` 가 아니면 `[ScenarioStateSubsystem] 재시작: 이 월드의 GameState 가 … 아님(게임 모드=…)` (Standalone 제외).
- **멀티캐스트 3종 — "각 프로세스가 자기 것을 치운다"** — 카메라 페이드(로컬 `PlayerCameraManager`) · 소총 `BP_AR4Rifle`(`bReplicates=false`) · 코스메틱 투사체 풀은 프로세스마다 따로다.
  `Atitan_exampleGameState::Multicast_ScenarioRestartBegin(FadeOut)` / `…Apply()` / `…End(FadeIn)` → 각 프로세스가 `UScenarioStateSubsystem::RunLocalRestartBegin/Apply/End`.
  Begin 은 **병사 파괴보다 먼저**(클라가 자기 병사 살아있는 동안 소총을 치우도록), Apply 는 화면이 검은 순간에 **자기가 시뮬하는** `IScenarioResettable` 만, End 는 고아 청소 + 페이드 인.
  각 프로세스는 **자기가 스폰한 것만**(`ROLE_Authority`) 파괴/주차하고 풀링 투사체는 항상 제외. titan GameState 가 없는 월드에선 로컬 폴백.
- **드론 리셋을 권한별로 분리** — 드론은 이 프로젝트에서 **유일하게 서버가 아닌 쪽이 물리를 돌리는 액터**(풀 시스템에서 자체방호 클라가 시뮬 주체, 서버는 `Server_ReportState` 로 받은 위치를
  복제본에 적용만). 서버에서만 `Flight->ResetTo` 를 불러 봤자 다음 보고에 덮인다. `ResetForScenarioRestart_Implementation` 을 `DroneFollowPath` 와 같은 규약으로 나눔 —
  서버 = 복제되는 것(`FramingSet` · `SetDetectionPhase(InitialDetectionPhase)` · `CommandedPathId`+카운터 · `RepEngagementFocusLocations`+카운터), 시뮬 주체 = 물리·입력·짐벌·정찰
  (`Flight->ResetTo(SpawnLocation, SpawnYawDegrees)` · Autopilot `Disengage` · `bParachuteObserved`). **`bParachuteObserved` 는 복제되지 않고 시뮬 주체에서만 갱신된다.**
  차량(`AUGV0901Pawn`/`ATitanTruck`)은 반대로 맨 앞에서 `if (!HasAuthority()) return;`.
- **소총 액터 누수 근본 수정([W116] 해결, SoldierLab 쪽)** — 1차 테스트의 `BP_AR4Rifle … OwningCharacter is not valid` 스팸 + GT 지연은 **재시작만의 문제가 아니라 평상시 사망 경로의 누수**였다.
  확정: `BP_SoldierCharacter` 에 `K2_DestroyActor`/`EndPlay` **0개** · `USoldierHealthComponent` 는 `Owner->Destroy()` 만 · 엔진은 **자기 Owner 만** 비우고 소유 액터는 어태치만 끊음
  (`Engine/Private/LevelActor.cpp`) · `BP_AR4Rifle` 은 `bReplicates=false` 라 프로세스마다 자기 것. 수정 = `USoldierHealthComponent::EndPlay`(`Destroyed` 일 때만) 가 자기가 어태치/소유한 액터를
  같이 파괴, 스위치 `bDestroyCarriedActorsOnDestroy`(기본 켬), 예외는 풀링 투사체·클라의 복제 액터. **BP 수정 0건.** 엔진 호출 순서상 안전(`DestroyActor` → `Destroyed()` → `RouteEndPlay` → 컴포넌트 `EndPlay`).
  titan 쪽 `DestroySoldierAttachments`/`DestroyOrphanedChildActors` 는 **안전망**으로 남김(낙하산처럼 병사가 아닌 재스폰 대상용). 원칙 **P188**.
- **그 외** — 재스폰 스냅샷에서 에디터 시각화용 임시 컴포넌트 제외(`CameraProxyMeshComponent_N`/`DrawFrustumComponent_N`/`OutputCameraComponent` — PIE 에서 스스로 생기고 번호가 매번 달라
  "재스폰본에 없음" 경고 + `RF_TextExportTransient` 라 ExportText 가 빈 문자열) · `titan_examplePlayerController::RequestScenarioRestart`/`SetScenarioAutoRestart` 를 `public` 으로(서브시스템이 부름, C2248 빌드 에러).
- **현재 상태** — L_SoldierScenario 단일 프로세스 ✅ / New_kadex_0811 2-PC ✅.
- 문서: 구현 문서 §1·§2·§3·§5 · 설계 v3 배너 5~7 · `scenario_authoring_guide.md` 2.7절(GameMode 표)·트러블슈팅 · `vehicle/drone/drone_flight_dev_guide.md` 15.2절(권한 규약) ·
  `replication/replication_audit.md` §9 신설 · `soldier_ai_lab/`(OPEN_ITEMS [W116] ✅ · IMPLEMENTED · CURRENT_STATE · CLAUDE.md **P188** + 콘솔 표) · `CURRENT_STATE.md` · `DOCS_INDEX.md`.

**같은 날 후속 — 드론 짐벌 배율 리셋 누락 + 리셋 대상 전수 대조 + 낙하산 재스폰 폐기**(사용자: "일단 지금 다 잘 작동하는 상태임")

- **드론 짐벌 배율(`ZoomLevel`)이 재시작에서 안 되돌려졌다 [사용자 지적]** — `ADronePawn::ZoomLevel` 은 `EditAnywhere` 저작값(기본 1.0)인데 자동 정찰/교전 프레이밍이 런타임에 계속 바꾼다
  (`GimbalSearchZoomLevel` ↔ `GimbalZoomInLevel` 램프 · 광각 `RequiredZoom` · 수동 전환 `BeginManualZoomTransition`). 리셋이 **전환 플래그(`bManualZoomTransitionActive`)만 끄고 값은 안 되돌려서**
  2회차가 지난 사이클이 끝난 배율에서 시작했다(정찰 램프가 천천히 수렴하므로 첫 몇 초만 1회차와 다름). 수정 = `InitialZoomLevel` 을 BeginPlay 에 스냅샷(`InitialDetectionPhase` 와 같은 방식) →
  리셋의 **시뮬 주체 구간**에서 `SetZoomLevel(InitialZoomLevel)`(그 함수가 `SyncGimbalLensFromCineCamera` 까지 해서 FOV 즉시 반영), `ManualZoomStartLevel`/`ManualZoomTargetLevel` 도 초기 배율로.
  서버의 `RepZoomLevel` 은 시뮬 주체의 다음 `Server_ReportState` 로 따라온다. ⚠ **아직 빌드 전 — 다음 빌드에 들어간다. "검증 완료"가 아니다.**
- **런타임 상태 전수 대조 — 그 외 누락 없음** — 드론(`ADronePawn`)과 UGV 컨트롤러(`AUGVAIController`)의 런타임 멤버를 리셋 함수와 하나씩 대조. 줌 외에 빠진 것은 없었고, **안 되돌리는 것들은 의도적**이다:
  ① 진단 누적값(`Diag*`/`*LogAccum`/`StateReports*`/`LastStateReportLogTime`) — 무해 ② 복제 미러(`Rep*`) — 시뮬 주체 값이 오면 덮임 ③ BeginPlay 1회 세팅(`bAxisResolved`/`bMappingContextApplied`/
  `bGimbal*BoneValid`/`bDisableGimbalCapture`/도로 세그먼트 캐시) — **되돌리면 오히려 깨진다**(`bAxisResolved` 를 내리면 드론 Tick 이 통째로 조기 리턴) ④ `bSnapInProgress=false` 면 무의미한 값(`SnapStart*`).
- **드론 `ViewMode` 는 리셋하지 않는다(결정)** — `EDroneViewMode`(Chase/Onboard/Gimbal)는 사람이 드론을 직접 조종할 때 `OnCameraTogglePressed` 로 바꾸는 **로컬 카메라 모드**(`L_DroneTest` 류 워크플로).
  전시·2-PC 구성에선 아무도 드론을 빙의하지 않아 값이 안 바뀐다 → 리셋 불필요(사용자 확인 완료).
- **낙하산은 재스폰 대상이 아니다 [사용자 확정]** — `ScenarioConfig.RespawnActors` 를 **비워 둔다**("낙하산은 정적이라서 초기화할 필요 없음"). 그동안 여러 문서에 "남은 것: 낙하산 `RespawnActors` 연결"로
  적혀 있던 항목을 전부 **불필요(정적 액터)** 로 정정. `RespawnActors` 프로퍼티 자체는 남긴다(병사가 아닌 재스폰 대상이 생기면 쓰는 자리).
- **[W116] 교차 세션 정리** — 같은 누수를 SoldierLab GT 부하 세션에도 보고했었으나 사용자가 "그 세션에 시킬 일이 아니다, 네가 고쳐라"고 해서 **이 세션이 직접 수정**했다. 그쪽 세션은 `SoldierHealth.h/.cpp` 를
  전혀 건드리지 않았고(제안은 승인 대기였다가 이쪽 수정 확인 후 철회) — **충돌 없음, 수정 주체는 한 세션뿐**. `obj list class=BP_AR4Rifle_C` 실측 카운트는 아직 없다(2-PC 실기 증상 소멸만 확인).
- **남은 것** — ① 드론 줌 수정이 들어간 빌드로 2회차 짐벌 그림 확인 ② 10사이클 자동 재시작 뒤 `stat memory`/fps 추이 ③ 장시간 무인 반복 ④ **P4 제출 순서 조율** — 이 세션 워크스페이스
  (`user4_DESKTOP-81S78B2_4340`)에 소스 다수 + DT 2개가 열려 있고 `Soldiers/SoldierLabBridgeSubsystem.cpp` 처럼 **다른 세션이 동시에 편집 중인 파일**이 있다(그쪽 변경 내용은 해당 세션 문서 몫).
- 문서: 구현 문서 §3·§5(헤더 포함) · 설계 v3 배너 **8~9 추가** · `scenario_authoring_guide.md` 2.7절 `RespawnActors` 행·트러블슈팅 1행 추가 · `vehicle/drone/drone_flight_dev_guide.md` 15.2절
  (짐벌 배율 · `ViewMode` · "일부러 안 되돌리는 것") · `replication/replication_audit.md` §9 후속 문단 · `soldier_ai_lab/OPEN_ITEMS.md` [W116] 교차 세션 줄 · `CURRENT_STATE.md` · `DOCS_INDEX.md`.

**같은 날 후속 2 — 상태 패널 누적값(배터리·비행시간·주행거리) 미초기화** [사용자 지적]

- **발견** — 재시작해도 **드론 배터리와 비행시간이 초기화되지 않는다**. 확인해 보니 **UGV 에도 같은 문제**가 있었다(배터리 + 누적 주행거리). 배터리가 0 이 돼도 비행이 멈추는 로직은 없어 기능 장애는 아니지만
  **"재시작 = 레벨 시작과 같은 상태"** 기준에 어긋난다.
- **원인** — 두 상태 컴포넌트가 리셋 계약에 빠져 있었다. 드론·트럭 `UStatusHUDComponent`: `ElapsedTime` 이 영원히 누적 → `BatteryPercent = 100 − ElapsedTime × 0.05`, `CurrentData.FlightTimeSeconds += DeltaTime`,
  고도/속도 그래프 히스토리·평활 필터·샘플 타이머도 이어짐. UGV `UUGVStatusComponent`: `ElapsedTime` → `BatteryPercent = 100 − ElapsedTime × 0.03`(온도도 여기서 파생). 누적 주행거리의 원본은
  `AUGVAIController::TankTotalDistanceTraveledCm` 이고 `UUGVStatusComponent::CurrentData.DistanceTraveledKm` 이 매 틱 거기서 파생된다.
- **수정** — `UStatusHUDComponent::ResetForScenarioRestart()` 신설(`ElapsedTime`/`TimeSinceLastSample`/`TimeSinceLastUIUpdate` 0, `CurrentData = FUAVStatusData()`(배터리 100 · 비행시간 0 · 그래프 히스토리 비움 —
  GPS/링크/임무 등은 다음 틱 `GenerateDummyData` 가 다시 채움), 그래프 필터 새 구조체로, `bHasRealFlightData`/`bHasRealWaypoint` 내림) · `UUGVStatusComponent::ResetForScenarioRestart()` 신설(`ElapsedTime`·
  `RealDistanceTraveledKm` 0, `BatteryPercent=100`, `DistanceTraveledKm=0` — **구조체를 통째로 밀지 않는다**: 기어 라벨(`SetGearData`)처럼 매 틱 다시 안 채워지는 필드가 있어서. 온도·속도·경사는 매 틱 파생이라 무관) ·
  `AUGVAIController::ResetForScenarioRestart` 에 `TankTotalDistanceTraveledCm=0.f` · 호출부 셋(`ADronePawn` · `AUGV0901Pawn` · `ATitanTruck`).
- **★ 호출 위치가 두 컴포넌트에서 정반대다** — **드론 패널은 모든 프로세스에서** 되돌린다(`ADronePawn::ResetForScenarioRestart_Implementation` 의 **시뮬 주체 판정보다 먼저**, `HasAuthority` 분기 밖): `UStatusHUDComponent::CurrentData`
  는 **리플리케이트되지 않고** 틱에 권한 게이트도 없어 프로세스마다 자기 값을 따로 누적한다. **UGV 패널은 서버에서만** 되돌린다(`AUGV0901Pawn` 의 `HasAuthority` 게이트 **안**): `UUGVStatusComponent::TickComponent` 가
  `bUseDummyData && GetOwner()->HasAuthority()` 일 때만 값을 만들고 `CurrentData` 가 `ReplicatedUsing = OnRep_CurrentData` 로 복제된다. 트럭은 드론과 **같은 컴포넌트**지만 트럭 자체를 서버가 시뮬하므로 지금은 게이트 안 —
  클라 화면의 트럭 패널을 쓰게 되면 밖으로 옮겨야 한다(코드 주석에 명시). "복제되는 상태는 서버가, 비복제 로컬 상태는 각 프로세스가"의 구체 사례 2개.
- **같은 계열 전수 검색 — 추가 누락 없음** — `UDroneAutopilotComponent::SpoolElapsedSeconds`(다음 `BeginPathFollowing` 이 0 으로, 자가 복구) · `AUGVPawn::TotalDistanceTraveledCm`/`AUAVPawn::MissionElapsedSeconds`
  (이 레벨에서 안 쓰는 **구 변형** — 현재는 `BP_UGV_0901`/`ADronePawn`) · `AUGVAIController` 의 `AlignElapsedSeconds`/`SnapElapsedSeconds`, 드론 `ObservationHoldSeconds`/`ManualZoomElapsedSeconds`(이미 각자 리셋에 있음).
- **RCWS 탄약은 이미 초기화되고 있다**(사용자 질문 확인) — `URCWSComponent::ResetForScenarioRestart()` 첫 두 줄이 `CurrentData.AmmoMax = AmmoMax; CurrentData.AmmoCurrent = AmmoMax;` 이고 호출부가
  `Vehicles/UGV0901Pawn.cpp`·`Vehicles/TitanTruck.cpp` 양쪽에 있다. `CurrentData` 가 복제되므로 클라 대시보드 숫자도 따라온다.
- ⚠ **빌드 전** — 이 수정과 직전의 드론 짐벌 배율 수정은 **둘 다 다음 빌드 반영 예정**이다. 2회차에서 **배터리 100 % · 비행시간 00:00 · 주행거리 0** 으로 시작하는지 확인할 것(드론 패널은 **호스트·클라 양쪽 화면**에서).
- 문서: 구현 문서 헤더·§2 표(신규 2행 + 호출부 4행)·§3-1·**§5 끝 신설 문단** · 설계 v3 배너 **11 추가** · `scenario_authoring_guide.md` 트러블슈팅 1행 · `vehicle/drone/drone_flight_dev_guide.md` 15.2절(★ 예외 블록) ·
  `replication/replication_audit.md` §9 후속 · `guide/ui_dev_guide.md` 갱신 1건 · `CURRENT_STATE.md` · `DOCS_INDEX.md`.

## 2026-09-23 — 드론 2-PC 잔여 4건: 클라 화면에 전장이 통째로 없던 원인(네트워크 관련성 150 m) · 원격 로터 회전 복제 · 분대 트래킹 이관 · 클라 치트 매니저 (드론 세션)

시작은 "클라에서 드론 소리가 RPM을 안 따라간다 + 드론 탐지가 낙하산·트럭 2개뿐 + 교전 이펙트가
안 보인다(*혼자 다른 씬에 있는 느낌*)"는 사용자 리포트. 넷이 전부 **자체방호 클라이언트에서만**
나는 증상이었다.

- **[제일 큰 건] 거리 기반 네트워크 관련성** — 관련성 기준점은 짐벌 씬캡쳐가 아니라 연결의
  `ViewTarget`(=트럭). 실측 트럭 (57330,12280,-3920) ↔ 전장 (-34870,13550,1030) = **923 m**인데
  `NetCullDistanceSquared`는 엔진 기본 225,000,000(**150 m**), `bAlwaysRelevant` 전부 false,
  `bUseDistanceBasedRelevancy` 기본 true(`GameNetworkManager.cpp:54`). `IsNetRelevantFor`
  (`ActorReplication.cpp:388`)에서 살아남은 건 드론(`PostLogin`의 `SetOwner` → `IsOwnedBy(RealViewer)`)과
  트럭(자기가 ViewTarget)뿐. **사격/피격 멀티캐스트도 같은 검사로 송신 단계에서 폐기**
  (`NetDriver.cpp:8243`)됐고, `bIsRevealed` OnRep이 안 와 적군은 숨은 채 탐지에서도 빠졌다.
  낙하산만 멀쩡했던 건 `bReplicates=false`라 프로세스마다 로컬 사본이어서.
  **Solo(`NM_Standalone`)·호스트 자기화면은 관련성 판정 자체가 없어서**(원격 연결 전용) 증상이
  안 났고, 그래서 오래 "PIE 특유의 문제"로 오해돼 왔다. 수정 =
  `UDetectableTargetComponent::BeginPlay`에서 복제되는 소유 액터를 `bAlwaysRelevant`로
  (`bForceAlwaysNetRelevant`) — 이 컴포넌트를 단 액터 집합이 곧 "드론/RCWS가 봐야 하는 대상"이라
  범위가 정확히 맞고, 병사 BP는 `ACharacter` 직속이라 C++ 생성자 자리가 없으며 SoldierLab 병사는
  런타임 부착이라 자동으로 따라간다. 비용은 30 Hz 캡(`NetServerMaxTickRate`) 기준 ~30 KB/s.
- **원격 로터 회전** — 원격은 Flight 틱이 꺼져 `RotorThrustN`이 0 → 프로펠러 소리 idle 고정,
  **날개도 정지**(소리만 보고 있어 놓치던 부분). `RepRotorSpin01/Spread01` 추가하고 소리·날개가
  `ComputeRotorSpinStats()` 한 소스를 보게 — 데모·풀 양쪽 게시 경로에 다 배선.
- **분대 트래킹 판정 이관** — 적군을 SoldierLab 병사로 교체한 뒤 "3분대 도주 제외"가 **조용히
  죽어 있었다**. 판정 근거가 새 병사엔 없는 `UEnemyCombatComponent`였고 호출부가
  `if (Combat && …) continue;` 꼴이라 전원 통과. 복제되는 `SquadId`/`bBreakingContact`를 신설해
  브리지(`SyncSoldiers`)가 채우고, `ResolveEnemyTrackingFacts()`가 구/신을 흡수. 탐지 박스에서도
  빼는 드론 전용 스위치(`bIgnoreBreakingContactTargets`) 추가 — **지휘소 RCWS엔 켜면 안 된다**
  (그쪽 발견이 6번 국면 트리거).
- **⚠ 추측으로 두 번 되돌림** — ① `Withdraw`를 도주로 같이 봤다. 실제 표에서 `EnemyFleeToZone2`
  (적 3명 사망 = 교전 직후)가 2·3분대에 `Withdraw`를 내리므로, 교전하자마자 2·3분대가
  탐지·트래킹에서 사라지고 1분대 전멸 시 드론이 지휘소로 날아갔다(사용자: *"완전히 잘못됐어"*).
  진짜 도주는 `Squad3Run`의 `BreakContact`. ② 분대 이름을 `"Squad3"`로 적었는데 실제는 `"3"`
  (정본 `AScenarioConfig::SquadZones`; 레벨의 `Squad3Path`는 경로 액터 라벨). 둘 다 **DT를 직접
  읽었으면 1분이면 확인됐을 것**이라, 불일치 시 경고 로그 1회를 넣어 폴백에 묻히지 않게 했다.
- **덤: 클라에서만 `ToggleDebugCamera`가 "Command not recognized"** — 클라엔 `AuthGameMode`가
  없어 `AddCheats`의 조건이 false → 치트 매니저 자체가 없었다(`GameModeBase.cpp:1413`).
  `BeginPlay`에서 로컬 컨트롤러면 `EnableCheats()`(비-쉬핑). 패키징 호스트도 같이 풀린다.
- 실환경 재검증에서 사용자 확인 — 관련성/소리/치트 "해결. 아주 잘됨", 분대 트래킹 "잘돼".
- 미해결로 남긴 것: **PIE 로비 경유 시 드론 화면의 나무 라이팅·팝핑**(패키징에선 안 남).
  식생은 복제 대상이 아니라 관련성과 무관 — 성질만 같다("씬캡쳐는 엔진의 뷰 의존 시스템이 아는
  시점이 아니다"). 계측 전이라 원인 단정하지 않음.
- 문서: `replication/2026-09-23_net_relevancy_battlefield.md`(신규) ·
  `vehicle/drone/2026-09-23_drone_remote_rotor_and_squad_tracking.md`(신규) ·
  `vehicle/drone/drone_flight_dev_guide.md` 15.2·**15.4 신설**·16.2·**16.2-1 신설**·16.6·16.7·18절 ·
  `guide/detection_dev_guide.md` §2·§3.6 · `level_new_kadex_0811/scenario_authoring_guide.md`
  (드론이 `bBreakContact`를 읽는다는 주의) · `CURRENT_STATE.md` §9 · `DOCS_INDEX.md`.

## 2026-09-23 — SoldierLab 차량 표적 교전 복구(`aperture 0` = 표적 자신이 벽) + 차량 위협 눈높이 (AI 세션)

New_kadex_0811 3차 전투지에서 적 3분대가 이동형지휘소 트럭(`BP_TitanTruck`)을 거의 안 쏘고 멀뚱히 서 있다 자리만 옮기던 문제.
대인 전투는 정상이었다. 상세: **`soldier_ai_lab/ai/2026-09-23_vehicle_target_engagement_fix.md`**(새 문서). ID **C-166~C-168 · P189**, 새 W 없음.

- **확정** — 콘솔 `SoldierLab.Debug.Engagement.Log 1` → `LogSoldierAI` 의 `[Engage]` 전이 줄.
  실측: `[Engage] Enemy_A2 t=11.71 hold -> blocked roe=hold/0 tgt=BP_TitanTruck dist=8942 … | believed 1 worth 1 **aperture 0(none/open)**`.
  사격 게이트를 앞에서부터 짚으면 표적(`tgt=`)·확신(`believed 1`)·가치(`worth 1`)는 전부 통과하고 **사격 자세만 0** — 질문이 `PlanAperture` 하나로 좁혀졌다.
- **★ 원인 1(주원인): 사격 레인 트레이스가 표적 자신을 벽으로 읽는다 [A]** — 세 조각의 합.
  (a) 차량은 `head`/`spine_03` 소켓이 없어 `GetSocketOrFallback` 폴백 = **바운드박스 50%(차체 한가운데)** 가 조준점, 80% 가 눈(09-17 오전에 "UGV 바퀴 겨눔" 을 고치며 넣은 값).
  (b) `IsShotBlockedByWorld` 는 충돌점이 조준점에서 `LaneToleranceCm`(**200 cm**, 사람 반지름 ≈ 45 기준의 여유값)보다 멀면 막힘으로 본다.
  (c) 그 트레이스는 자기 부착물과 **Pawn 타입만** 무시하는데(`SoldierQuery::BodiesAreNotWalls()`, 09-21) 차량은 Vehicle/WorldDynamic 이고 Sight 채널을 Block 한다.
  → 트럭 본체 메시 `Titan_Truck` 실측(로컬 바운드 X ±133 · Y −340…+310(전장 6.5 m) · Z 0…328)으로 **측면 ≈133 cm(통과) / 정면·후면 310~340 cm(막힘)** =
  트럭 **측면 축 ±48° 안에서만 사격 허용, 정면·후면 각 ±42° 부채꼴이 사각**. `PlanAperture` 는 Direct·좌우 린·기립·블라인드 3종까지 **전부 같은 레인 테스트 하나**로
  검사하므로 일곱이 동시에 탈락 → `aperture 0` → `Blocked` → (2 s) `bLaneDenied` → 엄폐 층이 HERE 에 비용 → **자리를 옮긴다 → 새 자리도 같은 사각 → 반복**(= 관측된 "서 있다 이동").
  ⚠ `BP_TitanTruck` 은 `BodyMesh` RelativeRotation yaw 270° 라 **장축이 액터 X축** — 레벨에서 사각 방향을 읽을 때 주의.
- **수정 1** — `IsShotBlockedByWorld(Muzzle, Target, const AActor* TargetActor = nullptr)` · `PlanAperture(…, TargetActor)` · `FindAperture(…, TargetActor)`.
  `HitActor == TargetActor || HitActor->GetAttachParentActor() == TargetActor` 면 **"도달"**. 호출부는 교전 틱에서 `Contact.Enemy.Get()` 을 `LaneTarget` 으로 뽑아 전달
  (청각 등 액터 없는 기록은 `nullptr` → 기존 "추정점 근처에 떨어졌나" 규칙 그대로 — 소리로만 아는 적에게 액터를 발명하지 않는다, P130).
  레인 캐시 `FLaneAnswer` 에 `TargetActor` 추가 + **캐시 키로도 비교**(같은 좌표에 다른 표적이 오는 경우). **200 cm 규칙은 그대로 남아 대인 동작 불변**(병사는 Pawn 이라 애초에 무시).
- **원인 2 / 수정 2: 엄폐·사격가능 판정의 위협 눈높이** — `USoldierCoverComponent::GatherThreatEyes` 는 위협 눈 = 기록 위치 + `ThreatEyeAboveContactCm 20`.
  차량 기록은 차체 한가운데(≈2 m)에 있고 실제 RCWS 포탑은 지붕(≈3.3 m+) → **1 m 이상 낮은 눈**으로 엄폐·그림자를 계산해 "숨었다고 판단한 자리" 가 실제로는 뚫렸고,
  게다가 그 눈이 **차체 콜리전 내부**라 거기서 출발하는 fight probe 가 설계에 없던 케이스였다. 수정 = 위협 Identity 가 **표적 소켓을 갖고 있지 않을 때만**
  (= 바운드박스 폴백 = 차량) 눈 단차를 그 대상의 **형상**(`GetEyeLocation().Z − GetTargetLocation().Z` = 80% − 50%)에서 계산. **형상은 보면 아는 것이지 위치가 아니라
  지각 규칙(P130) 위반이 아니다.** 병사는 +20 cm 유지. 곁가지: `USoldierIdentityComponent::HasSocket()` public(**비-`UFUNCTION` = 순수 C++ 이라 Live Coding 으로 빌드됨**).
- **검증** — ① 전용 시험 레벨 `L_SoldierTest` 에 `BP_TitanTruck` 배치(적 3명 96 m, **트럭 정면 = 최악 조건**)로 `aperture 0` 재현 → ② 수정 후 사격 정상 →
  ③ **메인 레벨 New_kadex_0811 에서도 트럭 상대 사격 정상 확인(사용자, 2026-09-23)**. ②만으로 안 닫은 이유: 평지 한 대짜리 시험 레벨은 원인 2가 거의 안 드러난다.
  ⚠ 시험 레벨 재현 세팅: `ScenarioConfig_0.bDemoAutoStartScenario = false`(자동 시작이 `DT_ScenarioSteps_SoldierTest1v1` 의 `EnemyInfiltrate` = ROE **HoldFire** 를 걸어
  로그가 `roe=hold` 였다; 분대 명령이 없으면 기본 배정이 **ROE Free** = "보이면 쏨") · 트럭은 `RCWSFireControl.CurrentMode=Remote` + `bDemoForceCommandPostAutoFire=false` 라 반격 안 함.
  **팀 판정은 원래부터 정상**이었다(트럭 `DetectableTarget` Friendly → `USoldierLabBridgeSubsystem` 이 `SoldierIdentity(Friendly)` 부착 → 적 보병이 표적으로 잡음).
- **남은 것 [C]** — [C-166] `TargetRadiusCm 45` 가 사람 가슴 기준이라 2.7 × 6.5 m 트럭도 45 cm 표적처럼 조준 게이트를 잼(원거리 `Aimed` → `Suppressive`; 반영하면 버스트/정착 박자가 같이 바뀐다) ·
  [C-167] 차량 조준점이 포탑이 아니라 차체 중앙("RCWS 를 노린다" 는 표현이 아직 없다) · [C-168] 차량 위협에 대한 엄폐 품질은 메인 레벨 **정성 확인만**, 수치 미측정.
- 문서: 새 1건 · `soldier_ai_lab/`(OPEN_ITEMS **C-166~C-168** · CURRENT_STATE · CLAUDE.md **P189**) · `CURRENT_STATE.md` §7 · `DOCS_INDEX.md`.

## 2026-09-23 — SoldierLab 로우레디(총 내림) 상체 레이어 + 아군 앉기 무릎/발 IK (포즈/애니 세션)

상체만 총을 내리는 레이어를 `SoldierCharacter_ABP` 에 신설하고(정지·걷기 내림 / 조깅 올림, 왼손 그립 IK 미사용), 같은 날 아군 앉기 IK 문제 둘을 해결했다.
상세: **`soldier_ai_lab/animation/2026-09-23_low_ready_upper_body_layer.md`** · **`soldier_ai_lab/animation/prototypes/2026-09-23_ally_crouch_ik_bones_removed.md`**(둘 다 새 문서).
ID **C-169~C-170 · W117 · P190~P191**.

- **그래프(데이터 흐름 순)** — ① 소스 포즈 6개 `Sequence Evaluator`(ALLY/Enemy × `*_MM_Rifle_LowReady` / `*_MM_Rifle_Idle_ADS` / `*_Idle_Hipfire`) → 진영 `Blend Poses by bool`(blend 0) ×3
  ② **로우레디 = `Apply Mesh Space Additive`(Base Idle_ADS + Additive LowReady, Alpha 1)** — LowReady 애님이 ADS 기준 MS additive 라 ADS 위에 얹어야 포즈가 된다
  ③ **걷기 흔들림 = `Make Dynamic Additive`(Base 정지 Hipfire, Additive `Use cached pose 'AimedPose'`, MS)** = "지금 로코모션 포즈 − 정지 힙파이어"
  ④ 그 additive 를 `Layered blend per bone` 에 통과시켜 **clavicle_l/r · neck_01 이하에서 흔들림 제거**(반대 입력 = `Additive Identity Pose`)
  ⑤ 로우레디 + 흔들림(Alpha = `Map Range Clamped(Speed2D, 20→150)` × 0.6) ⑥ 최종 `Layered blend per bone`(Base = `AimedPose` 캐시, MS Rotation Blend,
  가중치 = `FInterp Ease in Out(WeaponLowered, Exp 2)`) → ⑦ **`Slot 'UpperBody'` 의 Source**. ABP 변수 `WeaponLowered` 는 C++ 브리지가 매 틱 쓴다.
- **터졌던 문제 다섯과 원인** —
  ① *걷기 발이 2배속* = 로코모션 출력의 **포즈 fan-out**(서브그래프가 프레임당 두 번 평가). 해결 = 기존 `AimedPose` 캐시를 두 번 읽기(새 `SaveCachedPose` 는 MCP 로 못 만듦).
  ② *플레이어 빙의 시 로우레디가 전혀 안 보임* = 레이어를 `LayeredBoneBlend_1.Base` 에 먹였는데 **그 위 가중치 1짜리 상체 슬롯이 캐시로 덮어씀**. 해결 = 레이어를 슬롯 **Source** 로.
  ③ ★ *총이 거의 다 올라왔을 때 오른손이 턱 하고 튐* = `Layered blend per bone` 의 **`Curve Blend Option` 기본값 `Override`**. 엔진 구현상 레이어 가중치가 **0 보다 크기만 하면**
  레이어 커브가 베이스 커브를 통째로 덮고(`AnimationRuntime.cpp` `BlendCurves` 기본 분기), 가중치가 **정확히 0 이 되면 노드 자체가 스킵되어**(`AnimNode_LayeredBoneBlend.cpp:249`)
  베이스 커브가 그대로 나온다 → 흔들림 additive 에 섞여 들어온 로코모션 커브(`Enable_Warping` · `MoveData_Speed` · `contact_l/r` · `Disable_AO`)가 0.6→0 으로 줄다
  **마지막에 1 로 튀면서 오리엔테이션 워핑이 한 번에 돌아갔다**. 해결 = **`Curve Blend Option = UseBasePose`**.
  ④ *멈추면 1~2프레임 만에 총이 역순으로 팍 내려감* = 조깅 올리기를 애님그래프에서 `Speed2D` 에 직결(250/400)했는데 CMC 감속이 **2프레임**에 끝남. 해결 = **속도 기반 올리기를 C++ 램프로**
  (`USoldierAIBridgeComponent::UpdateBodyYawRate` 말미) — ★ ABP 의 `WeaponLowered` 를 **캐릭터의 `WeaponLowered` 와 분리된 별도 값**으로 램프(같은 목표·같은 레이트, 이전 값은 ABP 에서 읽음).
  캐릭터 값은 몸통 요 회전속도(`YawRate_Up/Down`)와 조준 보정 게인을 계속 쓰므로 안 건드린다 — **이 분리가 핵심**. 애님그래프의 250/400 노드는 연결을 끊어 무효화.
  ⑤ *총을 앞으로 내미는 것처럼 보임* = 흔들림 additive 에는 움직임뿐 아니라 **로코모션 애님과 힙파이어 정지 자세 사이의 정적 팔 오프셋**도 들어 있어 × 0.6 이 총을 앞으로 민다. 해결 = 위 ④번 노드.
- **커스터마이즈 지점** — 내림/올림 시간 = `BP_SoldierCharacter` 클래스 디폴트 **`WeaponRaiseRate` 2→2.5 · `WeaponLowerRate` 8→3**(예전엔 레이어가 spine_05 부터만 덮어 이 값이 체감 안 됐다) ·
  속도별 내림 정도 = cvar `SoldierLab.Pose.WalkLowered 1.0` / `.JogLowered 0.0` / `.IdleSpeed 20` / `.RaiseSpeedStart 250` / `.RaiseSpeedFull 400`(0 = 사격자세, 1 = 완전 로우레디, 정지는 무조건 1) ·
  전환 곡선 = `FInterp Ease in Out` Exponent 2 · 흔들림 양 = `Multiply` B 0.6, 구간 20/150 · 로우레디 자세 = LowReady `Sequence Evaluator` Explicit Time 2.5.
- **블렌드 마스크** — 스켈레톤 `SK_UEFN_Mannequin` 에 `BM_LowReady_Layer`(레이어가 어느 본을 얼마나 덮는지) · `BM_LowReady_Sway`(흔들림을 어느 본에서 지우는지) 신설,
  두 `Layered blend per bone` 을 **Blend Mode = Blend Mask** 로 전환. Branch Filter 는 "이 본부터 자식 전부 가중치 1" 이라 본별 가중치를 못 줘서 마스크로 갔다. 마스크 값은 사용자 조정 [C-169].
- **★ 재발 주의(굵게)** — **가중치가 정확히 0 이면 `Layered blend per bone` 노드가 통째로 스킵된다**(뼈는 같아 안 보이고 **커브만 튄다**) · **`Layered blend per bone` 을 쓸 때 additive 경로에
  로코모션 커브가 따라 들어온다**(기본 `Override` 라 조용히 베이스를 덮는다). 이 둘은 이 프로젝트에서 재발하기 쉽다.
- **곁가지 — 아군 앉기 무릎/발 IK 2건(같은 날 해결)** — ① 아군만 V/B 로 자세를 낮출 때 **무릎이 안 굽고 몸이 위아래로 떠다니던** 원인은 **디자이너가 재임포트한 메시(#499) LOD0 의
  "Bones to Remove" 에 `ik_*` 본이 들어가 있던 것**(계층엔 있지만 `NonRequiredBone` 이라 런타임 포즈에 없음 → 다리 IK 가 조용히 무효). ⚠ **FBX/블렌더 비교로는 "본 동일" 로 오진했고,
  정답은 스켈레톤 트리 아이콘(속이 빈 동그라미 = NonRequiredBone)이었다** — 함정으로 기록. ② 이어서 앉은 자세에서 발이 꼬이던 것은 **`ALLY_MM_Rifle_Crouch_Idle` 한 장만**
  `ik_foot_*` 가 원점에 박혀 있던 것 → **Animation Data Modifier `AM_Copy_IKFootRoot`**(attach→ik_foot_root, foot_l/r→ik_foot_l/r) 적용으로 해결.
- **남은 것** — [W117] cvar 5개 `UPROPERTY` 승격(헤더 리플렉션이라 Live Coding 불가 — **다음에 에디터를 닫는 정식 빌드 때**) · [C-169] 블렌드 마스크 가중치 미확정 · [C-170] 조깅 raise 구간(250/400) 미측정.
- 문서: 새 2건 · `soldier_ai_lab/`(OPEN_ITEMS **C-169~C-170 · W117** · CURRENT_STATE · CLAUDE.md **P190~P191**) · `CURRENT_STATE.md` §7 · `DOCS_INDEX.md`.

## 2026-09-23 — 차량이 밟는 시체(서스펜션이 시체를 지면으로 읽던 경로 차단) + 사망 시 무기 드롭 (시체·차량 세션)

UGV(`BP_UGV_0901`)가 쓰러진 적군을 가끔 밟고 지나가는데, **밀려나는 그림 자체는 좋지만** 시체가 땅에 박혀 부들부들 떨었고,
그 값이 서스펜션으로 흘러 **차량이 뒤집히는 일은 절대 안 된다**는 요구가 출발점이다(⚠ **실제 뒤집힘 목격은 0회 — 예방 수정**).
같은 자리에서 사망 시 무기를 손에서 떨구는 것도 함께 넣었다. 상세: **`soldier_ai_lab/ai/2026-09-23_corpse_vehicle_interaction_and_weapon_drop.md`**(새 문서).
ID **C-171~C-173 · W118~W119 · P192**. **코드 완료 · 재빌드 실측 대기.**

- **원인(엔진 소스로 확정)** — Chaos 서스펜션은 `ECC_WorldDynamic` 채널로 레이/스피어캐스트를 쏘고 응답은 컴포넌트의
  `WheelTraceCollisionResponses` 를 그대로 쓴다(`ChaosWheeledVehicleMovementComponent.cpp:486`·`:497` 트레이스, `:1776` 물리 입력 전달).
  그 **기본값이 "`ECC_Vehicle` 만 Ignore, 나머지 전부 Block"**(`:1142-1143`)이고 **UGV 쪽엔 이를 덮어쓰는 코드가 없었다.**
  병사는 사망 시 `USoldierHealthComponent::StartRagdoll` 에서 메시가 **`ECC_PhysicsBody` + `QueryAndPhysics`** 로 바뀐다
  (`SoldierHealth.cpp:652-653`) → **바퀴가 시체를 지면으로 읽는다**: 바퀴 하나만 지면이 갑자기 수십 cm 위에서 잡혀 스프링 힘이
  튀고, **마찰 계수까지 시체의 물리재질에서** 가져온다(스키드스티어라 방향도 튄다). 이것이 뒤집힘으로 가는 경로다.
  ⚠ **두 번째 경로는 이번에 손대지 않았다** — 차체와 20개 바디 래그돌의 **물리 접촉 자체**(사이에 끼면 매 서브스텝 depenetration 이
  싸우며 떨림이 생기고 반력이 차체로 들어감) → [W118].
- **수정 ① 차량(한 줄)** — `UUGVWheeledVehicleMovementComponent` 생성자에 `WheelTraceCollisionResponses.SetResponse(ECC_PhysicsBody, ECR_Ignore)`.
  끊는 것은 "**서스펜션이 시체를 지면으로 계산하는**" 경로 하나뿐이고, **물리 접촉은 그대로 둬 밀려나는 그림은 유지**한다(사용자가
  좋다고 한 부분). 적용 범위 = `SetDefaultSubobjectClass` 로 이 컴포넌트를 박는 `AUGVWheeledVehiclePawn` 자손(= UGV 계열)뿐.
- **수정 ② 사망 시 무기 드롭(`USoldierHealthComponent`)** — 신설 `UPROPERTY` 3개: `bDropWeaponOnDeath`(true) · `DroppedWeaponMassKg`(3.5) ·
  `WeaponMeshComponentName`("WeaponMesh"). **떨구는 순간은 `StartRagdoll()` 안** — 손이 애니메이션을 놓는 바로 그 순간이다
  (더 일찍 떨구면 화면상 쥐고 있는 손에서 총이 빠진다). 공통 처리 `ReleaseAsDebris(UPrimitiveComponent*)`: 오브젝트 타입
  **`ECC_PhysicsBody`**(← ①의 제외 한 줄이 **시체와 총을 동시에** 덮게 하려고 일부러 같은 타입) + `QueryAndPhysics`, 응답은
  **전부 Ignore 에서 시작해 WorldStatic/WorldDynamic/PhysicsBody/Vehicle 만 Block** — ⚠ **Sight/Cover 채널을 뺀 이유는 "바닥에
  굴러다니는 소총이 시야를 막거나 엄폐물로 계산되면 안 되기 때문"**, 질량 3.5 kg 오버라이드(물리에셋 기본 질량은 소총 크기 hull
  밀도라 **모루처럼** 떨어진다), 시뮬 on, 사망 순간 속도 상속. `FreezeCorpse`(기본 8 s)에 떨군 것들도 `PutRigidBodyToSleep()`.
  `EndPlay` 의 기존 `bDestroyCarriedActorsOnDestroy` 정리 대상에 **떨군 목록(`DroppedWeapons`/`DroppedMeshes`) 추가** — 떼어내면
  `GetAttachedActors` 에 안 잡혀 월드에 남는다(09-22 P188 로 막은 누수가 다시 열릴 뻔한 자리).
- **★ 함정 — 손에 들린 총은 스폰된 액터가 아니다** — 첫 구현이 "부착 액터를 떼어 떨구기"였는데 **총이 오른손에 그대로 붙어
  있었다.** 화면에 보이는 총은 `BP_SoldierCharacter` 가 **자기 컴포넌트로 들고 있는 `WeaponMesh`(`SK_KA74U_X`)** 이고,
  BeginPlay 에서 스폰해 붙이는 `BP_AR4Rifle`(`SK_AR4_X`)은 **총구 소켓/FX 용 별개 액터**(숨김)였다. 그래서 드롭은
  **컴포넌트를 `DetachFromComponent(KeepWorldTransform)` 로 떼는 경로가 본체**이고 액터 경로는 보조다(두 메시 다 물리에셋 보유 —
  `SK_KA74U_X_Physics` / `SK_AR4_X_Physics`). 추가 규칙: **보이지 않는 부착 액터는 떨구지 않는다**(`IsVisible()`/`bHiddenInGame`) —
  안 보이는 보조 액터가 물리 파편으로 바닥에 남는 것을 막는다.
- **재시작 시 정리(질문받아 확인)** — `UScenarioRespawnSubsystem` 이 재시작 때 대표 액터를 **시체 포함 전부 `Destroy()`**
  (`ScenarioRespawnSubsystem.cpp:251-265`). 떨군 것이 **컴포넌트**면 시체 액터 소유라 함께 소멸하고, **액터**면
  `USoldierHealthComponent::EndPlay` 의 정리 목록이 잡는다. 재시작 쪽 `DestroyChildActors`(부착 재귀 + Owner 기준)는 안전망으로 남는다.
- **검증 상태** — 서스펜션 제외는 **적용·빌드됨**(뒤집힘 사례가 원래 드물어 **정성 확인만 가능**, 미측정 [C-172]).
  무기 드롭은 **첫 빌드에서 안 떨어짐** → 위 함정 파악 후 컴포넌트 경로 추가 → **재빌드·실측 대기**. 확인 로그:
  `LogSoldierAI: [Death] <이름> dropped N carried actor(s) and M held mesh(es).` — **`M`=1 이어야 화면의 총이 떨어진 것**(`N`=0 이 정상).
- **손대지 않은 것** — 차체↔래그돌 **물리 접촉 자체**(끼임·떨림), 그리고 **`FreezeCorpse` 는 애님만 멈추고 래그돌 바디는 계속
  시뮬레이션된다**(진짜 sleep/시뮬 해제 미적용) → 둘 다 [W118]. UGV 컴포넌트 헤더의 "프로퍼티도 동작도 추가하지 않는다" 주석이
  이제 사실이 아님 → [W119].
- **남은 것** — [C-171] 떨군 총 질량 3.5 kg·마찰 미측정 · [C-172] 서스펜션 제외 후 실제 거동 · [C-173] `ECC_PhysicsBody` 제외가
  **다른 물리 오브젝트**(파편·소품·추락 드론)에 주는 영향 미검토(제외는 타입 단위다).
- 문서: 새 1건 · `soldier_ai_lab/`(OPEN_ITEMS **C-171~C-173 · W118~W119** · CURRENT_STATE · CLAUDE.md **P192** + 체력·사망 읽기 순서) ·
  `CURRENT_STATE.md` §6(UGV)·§7 · `DOCS_INDEX.md`(soldier_ai_lab + `vehicle/ugv/` 포인터). `guide/` 는 **미변경**(차량 물리·병사 사망을
  다루는 에버그린 문서가 없다 — 있는 둘은 탐지/RCWS 이고 이 수정과 접점이 없다).

## 다음에 예정된 것 (이 시점 기준)

- **SoldierLab 차량 교전 다듬기(09-23)** — `TargetRadiusCm 45` 가 사람 가슴 기준이라 트럭도 45 cm 표적처럼 조준 게이트를 재는 것
  [C-166](반영하면 원거리 트럭 사격이 전부 `Aimed` 로 올라가 버스트·정착 박자가 같이 바뀌니 부작용부터 볼 것) · 차량 조준점을
  포탑으로 옮길 것인가 [C-167] · 차량 위협에 대한 엄폐 품질 수치 [C-168](본 레벨 정성 확인만 함).
  `soldier_ai_lab/ai/2026-09-23_vehicle_target_engagement_fix.md` 5절.
- **로우레디 상체 레이어 후속(09-23)** — **[W117] 포즈 cvar 5개(`SoldierLab.Pose.*`) `UPROPERTY` 승격은 헤더 리플렉션이라
  다음에 에디터를 닫는 정식 빌드 때** — [C-170](조깅 raise 구간 250/400 값 잡기)이 여기 묶여 있다. 블렌드 마스크
  `BM_LowReady_Layer`/`BM_LowReady_Sway` 가중치는 사용자가 스켈레톤 에디터에서 조정 중 [C-169].
- **시체·차량 후속(09-23)** — **무기 드롭 재빌드 후 실측**(로그의 `M`=1 확인)이 먼저다. 그 다음 [W118] 차체↔래그돌 물리 접촉
  (끼임·떨림) + `FreezeCorpse` 가 시체 바디를 안 재우는 것 — 후보 (a) 시체 바디도 sleep/시뮬 해제(⚠ **밀려나는 그림이 죽고,
  8초 전에 밟히면 그대로**) (b) 차량 콜리전에서 시체 제외(⛔ 원하는 그림과 반대) (c) 바디 수·미는 힘 상한. 그리고 [C-173]
  `ECC_PhysicsBody` 를 쓰는 액터 목록을 뽑아 **바퀴가 올라타야 정상인 것**이 그 안에 없는지 확인.
  `soldier_ai_lab/ai/2026-09-23_corpse_vehicle_interaction_and_weapon_drop.md` 7절.
  `soldier_ai_lab/animation/2026-09-23_low_ready_upper_body_layer.md` 8절.
- **LIG 지도 지리참조 답장 발송(09-22)** — `protocol/2026-09-22_minimap_georeference_for_lig.md` 부록 초안 + `m_map.png` 첨부. 지도 재생성
  여부(동쪽 오버행 해소 겸)는 사용자 결정 → 하면 `GeoCoordinateUtils.h` 상수 8개 재측정 + LIG 재공유.
- **08-26 스케일 변경 뒤 시나리오 타이밍 재점검** — `GetDistanceScaleFactor` 1.2135→1.0006 으로 UAV/UGV/RCWS 물리 스펙 경로가 씬 단위로
  21% 빨라짐. `BeginScenarioEnemyContact` 전체 흐름·커브 감속 튜닝값 실측 확인 안 됨. `RCWSFireControlComponent.h` 낡은 헤더 주석 정리.
- **시나리오 재시작 후속(09-23 갱신)** — ~~본 레벨 PIE · 2-PC~~ **완료(09-23, "이제 잘됨")**, 이후 전체 정상 동작("일단 지금 다 잘 작동하는 상태임").
  남은 것: **드론 짐벌 배율 리셋 수정이 들어간 빌드로 2회차 확인(아직 빌드 전)** → 10사이클 자동 재시작 뒤
  `stat memory`/fps → 장시간 무인 반복(`cycles.csv`) → **P4 제출 순서 조율**(`Soldiers/SoldierLabBridgeSubsystem.cpp` 등은 다른 세션이 동시 편집 중).
  ~~`RespawnActors=[BP_Parachute_C_3]` 연결~~ **불필요로 종결(09-23, 낙하산은 정적 액터)**. ~~사망 시체 소총 잔존 [W116]~~ 해결(P188, `obj list` 실측 카운트만 미측정).
  ⚠ **운용**: 2-PC·전시 구성에서 레벨 GameMode 오버라이드는 항상 `BP_KadexTestGameMode` — `GM_SoldierLab` 은 단일 프로세스 관전 전용(09-23 실사고).
- **SoldierLab 성능 후속** — ~~① 투사체 풀 [W109] → ② Engagement 레이 [W110] → ③ CMC 데이터 [W111] → ④ 캐릭터 BP 틱 → C++ [W108] → ⑤ Cover 후보·Sight 박자 [W102]~~ **전부 완료(같은 날 후편, World Tick 11~14 — 바닥 13~14 도달)**. 남은 것:
  ⑥ 컴포넌트 틱 통합 [W112](틱 함수 ≈ 600, 오버헤드 1.2) · **`AC_PreCMCTick` 0.8 ms C++ 이관 [W115]**([W108] 과 같은 방법, CMC 앞 틱 순서 보존) · 총 액터 메시 제거 [W113](≈ 0.25, 순위 낮음) · Cover 점수 계산 1.2~2.6(후보 수·`FanRays`) ·
  애니 GT 1.1 은 URO 불가라 `Update_PropertiesFromCharacter` 프로퍼티 액세스화가 남은 길([W98] ①) · URO 크래시 원인 [C-165](빈 레벨 1명 재현) · titan `TargetDetection` ×3 1.2 ms 미조사 · `WindSource` 20 Hz 는 New_kadex_0811 에서 실측.
  별건: `AN_Reload` 노티파이의 `LogAbilitySystem: Error` 41회/PIE [W114](로그뿐). **2PC 리플리케이션 검증(내일, 사용자)** — 이 세션이 `BP_SoldierCharacter` Tick(옛 본문 보존) · `BP_AR4Rifle` · `SoldierEngagement`/`SoldierProjectile` 위에 얹은 변경이 같이 검증된다. 메시 QueryOnly 의 `FEndPhysics` 효과 [B] 는 조용한 프레임에서 재측정.

- **SoldierLab 분대 명령 층 — New_kadex_0811 2차 PIE [C-163]** — `Squad3Run` 뒤 3분대가 진짜 도주하는가(`roe=hold` + 스프린트 · 엄폐 홉 0) ·
  UGV `타겟 BP_Soldier_Hostile_*` 0줄([W106] 틈만큼 늦을 수 있음) · 아군 `tgt=` 에 3분대 없음 · 트럭 사격 → `Squad3Stand` → 응사 · `ScenarioComplete`.
  페이싱·문턱 [C-164] · 존 위치 [Q51](사용자). 그 뒤 `MinStance` titan DT [W84] · 드론 프레이밍 [W104] · 2PC [C-125]. 시험 레벨 쪽 [Q50]·[C-144]~[C-147]·[C-146] 은 그대로. ~~New_kadex_0811 재저작 [W70]~~ 완료.

- **SoldierLab 09-21 묶음 후속** — ① BP 배선 둘 [W93](무기 산포 = `GetShotSpreadDegrees()` · `BP_SoldierCharacter` aim 모드 =
  `WantsToAim()`) → ② 엣지 전진 PIE 판정 [W95][C-158](`ADVANCE begins/step/ends` 로그 + 시안 핀/쐐기) → ③ ~~섀도우 감축 정식 빌드 →
  비용 줄 재측정 [W96]~~ 같은 날 완료(동일) → `MaxLights 16` 상한 [W92] · [C-161] 눈에 띄면 → ④ 분대 필드 거동 [C-157] · DT 분대 '4' [W94] → 콘·정착 [C-159] · 버스트 [C-160].
- **SoldierLab 오버레이 이관 [W97]** — `Squad/`(존 링·분대 링) · `Pose/`(ScanTurn·HeadAim 화살표·구) · `Weapons/`(투사체 점)의 `DrawDebug*` 를
  `SoldierDebug::*` 로(각 폴더 세션, `Arrow` 래퍼 하나 추가). 그 전까지 EV10 레벨에서 그 오버레이들은 검다.
- **SoldierLab 얇은 엄폐 A/B 숫자 판정** — 활동도 가중 은폐 · 미세 위치 [C-153](C 코너 멈춤은 09-21 에 삭제); 그 전에
  `L_SoldierTest`에 얇은 원기둥 배치 + 실제 레벨 나무의 Sight 채널 확인 [W88]. ~~ScanTurn 연동 [C-152]~~ 포즈 세션 확인 —
  남은 것은 스무더 축 속도 [C-154]·크라우치 DB "뚝" [W91] (눈에 띄면). 순찰 주기 [C-148] · CQB [C-149] · REJECT 반복 없음 [C-150]. `MinStance` titan 연결 [W84].
- **SoldierLab 상황 필드 09-18 오전분 판정**(오후 빌드에 포함됨) — 대칭 캡(`SoldierLab.Field.CellSizeCm 50`에서 링이 양쪽
  대칭 + CAPPED) · 미굽기 파랑끼 · 헤더 2줄 · 화살표 · 라이트 v2 · 부정 증거(얼린 핀이 정면 응시에 더 빨리
  흐려지나) — [C-141][C-142]. ~~[W83](들은 라이트 필터) 사용자 결정~~ 오후 해결. 오버레이 끈 채 셀 수가 후퇴전에서 병사 수 ×
  ≈5천 안에 머무는가 [C-138][C-140][C-143]. `USoldierFieldSettings` 값 잡기(43개, 헤더 2줄 보며). 거동 판정
  [C-133][C-135](문간→문간)는 여전히 열림 (`soldier_ai_lab/CURRENT_STATE.md` 09-18 블록).
- **2-PC 실기 재검증(UGV PC 호스트 + SelfDefense 클라이언트)** — 09-17 리플리케이션 수정 3건: 적군 걷기/뛰기
  애니메이션, 피격 흔들림, 사망 래그돌+총 낙하(`replication/2026-09-17_enemy_anim_death_replication_gaps.md`
  §6). 리플레이에선 검증됐고 이론상 동일하지만 8월처럼 BP 사각지대가 또 있을 수 있음. 09-17 수정분
  P4 제출도 아직.
- **RCWS 청각 보조(09-17) 반경 50m 재빌드 + PIE 검증** — 총성 후 스윕이 그 방향으로 도는지, 시각 우선, 아군
  사격/50m 밖 무반응, 4초 만료 후 블렌드 아웃(`rcws/2026-09-17_rcws_gunfire_hearing.md` 체크리스트).
  09-16 RCWS 성능 업그레이드의 세부 검증(머리만 나온 적/재출현/전방 우선)도 같은 PIE에서.
- 드론: 2대 PC 검증이 끝났으니 구 `AUAVPawn`/`BP_UAV`와 시나리오 폴백 분기 제거 착수 가능.
  2·3차 전투지 추격 스플라인 추가(코드 준비됨, 레벨 작업만). 원격 보간 품질(`RemoteInterpSpeed`)
  튜닝은 문제 보고 있을 때.
- 자체방호축 카메라 버그 2건 **2-PC 실환경 검증**(코드 수정은 완료, 빌드/실측만 남음 —
  `rcws/2026-08-31_selfdefense_camera_shake_bugs.md` §3의 절차).
- Graphics 탭 **패키지 실측** — VSync는 에디터에서 구조적으로 안 먹으니 패키지에서만 확인 가능,
  반사 SSR 통일 + Lumen 튜닝 공통화로 바뀐 **Linux 룩 확인**.
- `guide/` 문서 내용 실제 최신화(2단계, 시스템별로 별도 세션 — 아직 착수 전).
- 언리얼 에셋/코드 정리(레거시 BP, 폴더 구조) — 별도 세션 착수 예정, 아직 시작 전.
- LIG 후속 질문 3건 + 회신 2건 발송 대기(`protocol/lig_questions_0816.md`).
- **LIG에 595.84 대응 리눅스 패키지 재발송**(SDK 13.0.37 빌드 + `packaging/kadex_0915_패키징_실행가이드.md`,
  `rtsp/2026-09-15_lig_rtsp_describe_timeout_analysis.md` "남은 일").
- 레벨 디자인/UGV 자율주행 지속.
- **적 낙하산 숨김 리플리케이션 확인(여전히 열림, 09-17에도 안 봄)** — Chronicle 리플레이에서 안 사라지는
  것이 2 PC 클라이언트에서도 같은지(`replay_chronicle/2026-09-16_chronicle_replay_plugin.md` §4-2). 09-17에
  같은 가설로 찾은 4건이 전부 실제 결함이었으니 이것도 그럴 가능성이 높음. `RtspAxisGate` 수정분은 09-16 CL 486으로
  제출됨.
- UGV 서스펜션 후속(우선순위 순): `SuspensionDampingRatio 0.7 → 0.45~0.5`, 휠 클래스 3분할
  (앞/중/뒤 — `RollbarScaling`을 살리고 축별 스프링 분리를 가능하게 함), `WheelLoadRatio 0.5 →
  0.3`. `vehicle/ugv/2026-09-10_ugv_0901_suspension_tuning.md` §6.
- **UGV 자율주행 튜닝 오버레이**(`UUGVDriveTuningWidget` + `FUGVPursuitTelemetry`) — 코드는
  작성됐으나 **빌드/실동작 미확인**. 콘솔 `UGV.Tuning 1`로 띄우는 구조이고 WBP 없이도
  동작한다. "부딪히지 않는 선에서 최대 속도"를 잡기 위한 것(거버너 개입 시점, 전방 여유거리,
  브레이크 횟수 표시).
