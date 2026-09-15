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
  **2대 PC 실환경 검증만 남음.**
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
- **미완**: 새 enum이 에디터에 아직 없어 그 행의 `EffectType`이 `None`이다. **빌드 → DT에서
  `SetDemoUGVAutoFire`로 설정 → 저장 → PIE 로그 순서 확인**이 남았다. 재시작 설계(09-10)에는
  "UGV 리셋 시 RCWS 모드를 `Remote`로 되돌릴 것" 메모를 추가했다(안 하면 2회차부터 출발 전 스윕).

### 같은 날 — RCWS 탐색 스윕 고각을 차체 기준으로(내리막에서 하늘 보던 문제)

현장 피드백 "자동정찰 중 내리막길에서 포탑이 너무 위를 본다". 상세:
`rcws/2026-09-15_search_sweep_hull_relative_elevation.md`.

- 원인은 `UpdateSearchSweep`이 고각 목표를 `CurrentData.ElevationDegrees == 0`으로 잡은 것 —
  이 값은 2026-07-20부터 `RefreshAzimuthElevation`이 **월드 수평 기준**으로 계산하므로, 내리막에서
  차체가 기울면 마운트를 차체 대비 위로 들어 수평을 유지 → 하늘. 좌우 스윕은 원래 차체 기준.
- 새 프로퍼티 `SearchSweepElevationDegrees`(기본 **-3°**, 차체 기준, 음수 = 아래). 오차는
  `SearchSweepElevationDegrees - 마운트 relative pitch`(마운트 없으면 SightCamera relative pitch,
  `AddPanTiltInput`과 같은 분기). AutoSurveillance·AutoAim/AutoFire 무표적 양쪽, UGV·지휘소 공통.
- 안정화(기본 OFF)를 켜면 이 목표와 싸우지만 좌우 스윕도 원래 그랬던 기존 구조라 안 건드림.
  같은 파일이 `user2_jiseong`에게도 체크아웃돼 있어 서브밋 시 머지 가능성.
- **남은 것**: 빌드 후 실차 경사로 확인. `guide/rcws_fire_control_dev_guide.md` §8.4 갱신함.

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

---

## 다음에 예정된 것 (이 시점 기준)

- 드론 **2프로세스(2대 PC) 실환경 검증** → 통과하면 구 `AUAVPawn`/`BP_UAV`와 시나리오 폴백
  분기 제거. 2·3차 전투지 추격 스플라인 추가(코드 준비됨, 레벨 작업만).
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
- UGV 서스펜션 후속(우선순위 순): `SuspensionDampingRatio 0.7 → 0.45~0.5`, 휠 클래스 3분할
  (앞/중/뒤 — `RollbarScaling`을 살리고 축별 스프링 분리를 가능하게 함), `WheelLoadRatio 0.5 →
  0.3`. `vehicle/ugv/2026-09-10_ugv_0901_suspension_tuning.md` §6.
- **UGV 자율주행 튜닝 오버레이**(`UUGVDriveTuningWidget` + `FUGVPursuitTelemetry`) — 코드는
  작성됐으나 **빌드/실동작 미확인**. 콘솔 `UGV.Tuning 1`로 띄우는 구조이고 WBP 없이도
  동작한다. "부딪히지 않는 선에서 최대 속도"를 잡기 위한 것(거버너 개입 시점, 전방 여유거리,
  브레이크 횟수 표시).
