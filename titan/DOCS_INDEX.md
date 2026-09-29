# 문서 전체 목록 (DOCS_INDEX)

2026-09-29 / 진행중 / titan 폴더 전체 .md 문서 카탈로그, 2026-08-31 시스템별 폴더 재편
반영판(이후 세션별로 항목 추가 중).

폴더는 "시스템 하나당 폴더 하나" 축으로 통일됨(`CLAUDE.md` 참고). `guide/`는 에버그린
레퍼런스, 나머지는 전부 devlog(시간순 기록, 안 고침). 날짜는 문서 내용 기준, 없으면
파일명/mtime 기준 — `2026-07-24 전후` 표기는 옛 `C:\private\titan`에서 일괄 이관된
타임스탬프라 실제 작성일이 아닐 수 있음.

이 문서는 목록만 다룬다 — 현재 상태 요약은 `CURRENT_STATE.md`, 전체 시간순 서사는
`WORKLOG.md`, 문서 작성 규칙은 `CLAUDE.md` 참고.

---

## 최상위 (`titan/`)

- `README.md` — 진입점.
- `memo.md` — 사용자 개인 스크래치 노트(수정 금지). **최신화하지 않는 문서라 현황·요구사항의
  근거로 인용하지 말 것** — 여기 적힌 항목은 이미 해결됐거나 폐기됐을 수 있다(실제로 2026-09-04에
  `CURRENT_STATE.md` §11이 이 파일에서 옮겨온 백로그를 들고 있다가, 이미 해결된 "Linux 조이스틱
  미작동"을 미해결로 잘못 전달한 사례가 있었음 — 그 줄은 제거됨).
- `all.md` — Phase #4 시나리오 공식 명세 원문, 지금도 근거 문서로 유효.
- `content_asset_inventory.md` (2026-07-29) — 디자인팀 제공 Content 폴더 전수 조사.
- `ally_animation_request.md` (2026-08-07) — 디자인팀 아군 애니메이션 요청서, 현재 최신본.
- `ally_move.md` (2026-08-07) — 아군 Posture×Alert×Movement FSM 설계, §10만 구버전.

## `guide/` — 에버그린 레퍼런스 (⚠️ 내용은 대부분 옛날 것, 상단 경고 배너 확인)

- `soldier_movement_speed_guide.html` (2026-09-28 신규, **2026-09-29 내용 갱신 중 — 메인 세션이
  이메일 형식 HTML로 다시 쓰고 있다**) — 병사 이동 속도를 상황별로 조정하는 방법.
  **애니메이션 시스템을 모르는 사람(기획·디자이너) 대상**이라 HTML로 썼고, 브라우저로 열어서
  보면 됨. 대상 표는 `Content/SoldierLab/Data/DT_SoldierMovement`(**행 16개 × 열 2개 =
  걸음걸이 상한 `MaxGait` + 속도 배율 `SpeedScale` 0.75~1.25**), 연결점은
  `BP_SoldierCharacter → AC_SoldierMovementProfile`.
  ⚠ **09-28 판(배율 3열 · 안전범위 0.45~1.2)은 폐기됐다** — 배율을 임의 범위로 잡아 발이
  미끄러진 1차 구현이고, 가속·회전 2열은 `AC_PreCMCTick`이 덮어써서 애초에 죽은 값이었다.
  실효 범위 **0.75~1.25**는 ABP `Get_DynamicPlayRate`가 커브 없는 클립에 쓰는 대체값이다.
  구현·설계 근거와 정정 기록은 `soldier_ai_lab/animation/2026-09-29_movement_policy_and_playrate_band.md`
  (원칙 P193).

- `ui_dev_guide.md` (2026-07-10, 09-22 구현 현황에 재시작 확인창 `WBP_RestartPrompt`/축 규칙 한 줄 추기, **09-23 상태 패널 누적값 재시작 리셋 한 줄 추기**) — Monitor1/2 UI. `Monitor1Widget` 자체가 레거시로 확인됨,
  현재 위젯 개발기록은 `ui/kadex_test_dashboard_wbp_spec.md`.
- `quadcam_usage_guide.md` (2026-06-24) — QuadCamModule 사용법.
- `titan_dev_status.md` (2026-06-25) — QuadCamModule 내부 아키텍처+VRAM 버그 수정.
  `quadcam_usage_guide.md`와 병합 검토 대상.
- `rcws_fire_control_dev_guide.md` (2026-07-12~14, 이후 절별 추기 — 09-21 **3.3절** 시나리오 표적 제외 `bRespectEnemyTargetingExclusion` 경로 신설 · 09-22 **8.10절** 시나리오 재시작 `ResetForScenarioRestart`(RCWS 두 컴포넌트가 되돌리는 것/남기는 것)) — RCWS 조준/발사 메커니즘. 최신은
  `protocol/ugv_rc_feature_gap_analysis.md`/`protocol/selfdefense_rc_feature_gap_analysis.md`.
- `pixelstreaming_setup_guide.md` (2026-07-06) — Pixel Streaming. RTSP로 방향 잡힌 뒤
  실사용 여부 불명.
- `real2world.md` (**2026-09-22 전면 재작성, 최신**) — 씬↔위경도↔미니맵 픽셀↔UTM 변환의 현재
  동작. 레벨은 1:1(08-26 재스케일), 보정은 랜드스케이프 코너 2점(Scale≈1.0006, 회전 -86.23°, 씬
  +X≈남), `GetDistanceScaleFactor` 용도별 분류, 미니맵 상수 8개, 동쪽 오버행 주의. 옛 스크래치
  메모는 하단에 접어서 보존.
- `joystick_camera_control_dev_guide.md` (2026-07-08) — 조이스틱 RCWS/UAV 조작. 최신
  버튼맵은 `protocol/selfdefense_rc_feature_gap_analysis.md` §3.
- `detection_dev_guide.md` (2026-07-09, §2 에 09-18 총성 버스 · 09-21 `bTargetableByFriendlyForces` 소비자 · 09-22 시나리오 재시작 시 등록/`ForceRescan` · **09-23 분대 맥락(`SquadId`/`bBreakingContact`) 복제 + always relevant 부수효과**, §3.6 에 드론 전용 `bIgnoreBreakingContactTargets` 추기) — 객체 탐지(BBox) 모사.
- `mcp/unreal-mcp-claude-code.md`, `mcp/additional-mcp-integrations.md` (원날짜 불명) —
  unreal-mcp/Claude Code 연동 설정.

## `vehicle/ugv/` — UGV 주행/자율주행/물리

> **현행 UGV는 `BP_UGV_0901`(6×6 차륜)이다.** 2026-09-02에 궤도 16륜
> `BP_UGV_Vehicle_new`를 대체했고, 부모 클래스는 2026-09-03부로 **`AUGV0901Pawn`**.
> 아래 08-26/08-22 문서들은 구형 궤도 UGV 시절 기록이지만 주행 로직(스키드 스티어,
> TrackLock, 커브 감속)은 그대로 재사용 중이라 여전히 유효하다.
>
> 📌 **2026-09-23 — 서스펜션 트레이스에서 `ECC_PhysicsBody`(래그돌 시체·떨군 총)를 제외한 한 줄은 이 폴더가 아니라
> `soldier_ai_lab/ai/2026-09-23_corpse_vehicle_interaction_and_weapon_drop.md` 에 있다**(바뀐 코드 대부분이 SoldierLab 사망 처리
> 쪽이고 차량은 생성자 한 줄이라 그쪽에 묶었다). 원칙 P192 — "시체·떨어진 물체는 차량 서스펜션에 지면으로 보이면 안 된다;
> 채널에서 빼되 물리 접촉은 남긴다". 서스펜션/차량 물리를 만질 때 같이 볼 것.

- `2026-09-10_ugv_0901_suspension_tuning.md` (2026-09-10) — 레벨의 작은 바위에 콜리전을 켠 뒤
  승차감 튜닝. **Chaos 5.8 서스펜션은 힘이 아니라 PBD 컨스트레인트로 풀린다**는 구조(차체 운동/
  그립/바퀴 비주얼이 서로 다른 코드), **`SpringPreload`·`RollbarScaling`이 엔진에서 죽은 값**이라는
  확인, `MaxRaise`와 `MaxDrop`이 대칭이 아닌 이유. 서스펜션 값을 만지기 전에 볼 것. **완료.**
- `2026-09-02_ugv_0901_new_model_rig.md` (2026-09-02) — 신규 6륜 모델 교체 전 과정. 블렌더
  리깅·FBX 파이프라인, 스켈레톤/좌표, 머티리얼 7슬롯, **휠 반지름·개수 변경이 주행에 미치는
  영향과 스케일 규칙**(제동토크는 반지름 × 휠 수 둘 다 곱해야 함), 총열회전 제거
  (`bUseBarrelSpin` 플래그). **완료.**
- `2026-09-03_ugv_0901_bp_to_cpp.md` (2026-09-03) — BP 로직을 `AUGV0901Pawn`으로 이관하고
  죽은 노드 427개·변수 39개 제거(`.uasset` 1,150KB → 67KB). **죽은 노드 판정을 "연결된
  노드"로 하면 안 되는 이유**(exec 도달성 + 데이터 생산자 역추적 2단계)와 **BP의 기본
  float은 실제로 double**이라는 함정. **완료** — 과열 연기 실사격 확인만 남음.
- `2026-08-26_ugv_obstacle_avoidance.md` (2026-08-27 확정) — 장애물 회피 4대 원인 규명·해결,
  실측 34.6km/h·조향포화 0회. **완료.**
- `2026-08-26_ugv_track_lock_implementation_plan.md` (2026-08-26) — "공중에 뜬 바퀴" 버그 근본원인 3단
  규명·해결. **완료, 실기 확인.**
- `2026-08-22_ugv_corner_braking_dev_guide.md` (2026-08-22/25) — 커브 선행 감속(제동 곡선). **완료.**
- `2026-08-27_new_kadex_0811_navmesh_autonomous_driving.md` (2026-08-27, 최신판) — 신규 레벨
  내비메시 3층 구조 구축(A* 노드 예산/부분경로 버그 절 포함). **2026-08-31 재확인**: 이전엔
  `level_new_kadex_0811/`에도 08-22 시점 구버전 사본이 남아있었는데(내용이 완전히 이 문서의
  부분집합이라 고유 정보 없음 확인됨), 중복 제거하고 이 파일 하나로 통합함 — 레벨
  스플라인/PCG 구성값도 여기 담겨 있으니 레벨 작업 중이어도 이 경로에서 찾을 것.

## `vehicle/drone/`

- `drone_flight_dev_guide.md` (최종 2026-09-15, 09-16 §10.2 정정 — 서브스테핑은 의도적으로 끈 게 아니라
  엔진 기본값) — **드론 시스템 에버그린 레퍼런스.** 로터별
  추력→강체운동 물리, 자율비행(Pure Pursuit+제동곡선), **교전 관측 이동(16절)**,
  **수동 조종·비행/짐벌 분리(17절)**, 짐벌(**2축 안정화 12.4절**),
  프로펠러 사운드, 바람, 단계별 탐지, 시나리오 연동, 리플리케이션(**모드별 주체 표 15절, 2 PC
  검증 완료** — 09-23 15.2절에 **권한 규약**(서버 = 복제되는 명령 / 시뮬 주체 = 물리·짐벌, 시나리오 재시작도 같은 선으로 나눈다,
  `bParachuteObserved` 는 복제 안 됨) 추가, **09-23 후속으로 그 규약의 ★ 예외 = 상태 패널(`StatusHUD`) 은 서버/시뮬 주체로 나누지 않고 모든 프로세스가 각자 되돌린다**(`CurrentData` 비복제 +
  틱 권한 게이트 없음, 빌드 대기))까지 전부.
  `guide/`에 드론 문서가 없어서 이 파일이 그 역할을 겸한다 — **드론 동작이 바뀌면 여기를 같이
  고칠 것.**
- `2026-09-23_drone_remote_rotor_and_squad_tracking.md` — **원격 로터 회전 복제 + 분대 트래킹
  판정 이관.** (1) 원격은 Flight 틱이 꺼져 `RotorThrustN`이 0이라 프로펠러 소리가 idle에 고정되고
  날개도 멈춰 있었다 → `RepRotorSpin01/Spread01`, 소리·날개가 한 소스를 보게. (2) 적군을
  SoldierLab 병사로 교체하며 "3분대 도주 제외"가 **조용히 죽어 있던** 것(판정 근거가 새 병사엔
  없는 `UEnemyCombatComponent`) → 복제되는 `SquadId`/`bBreakingContact` + `ResolveEnemyTrackingFacts`.
  ⚠ **함정 2건이 이 문서의 핵심**: `Withdraw`는 도주가 아니다(교전 직후 2·3분대에 내려가는
  평범한 전투지 이동 — 도주로 치면 시나리오가 통째로 망가진다), 분대 이름은 `"Squad3"`이 아니라
  `"3"`(정본은 `AScenarioConfig::SquadZones`). 둘 다 추측으로 틀려 되돌린 실사고.
- `2026-09-15_drone_gimbal_stabilization.md` — **짐벌 2축 안정화.** 짐벌 각도 기준을 기체 →
  수평 프레임으로 바꿔 가감속 기울기가 카메라에 안 실림(롤은 상쇄 안 함, 실제 2축과 동일). 자동
  추적 목표각을 같은 프레임으로 통일하는 게 핵심(안 맞추면 가짜 오차를 쫓아 더 떨림). CineCamera
  니어플레인이 씬캡쳐(위젯/RTSP)에 복사 안 되던 것 수정.
- `2026-09-10_drone_manual_control_split.md` — 수동 조종을 **비행/짐벌 두 축으로 분리**해
  "카메라만 수동" 모드 신설. 함정 4건(짐벌 자동 로직 안에 비행 로직이 섞여 있던 것, 정찰 단계
  Idle 가드가 교전 중 입력을 버리던 것, 짐벌 버튼이 비행 IMC 안에 있어 "버튼 켰는데 무반응",
  같은 축을 두 용도로 쓰며 생긴 극성 왕복)와 **수동 해제 시 576km/h 역주행 버그**(+ 교전 관측
  상태 유실). 스켈레탈 메시 본으로 기본 각도를 바꾸면 왜 안 되는지도 정리.
- `2026-09-05_drone_engagement_observation.md` — 교전 관측 이동 구현 경과. **함정 8건**(대리
  지표로 목표함수 오설정, 연속 최적화 래칫, 클램프된 화각으로 초과량 판정, 단일 문턱 진동,
  TargetDetection 순환 의존, 경로 끝까지 역주행, 시나리오 스텝 vs 월드 상태, UGV도 RCWS 보유)과
  **유도 루프 구조 결함 3연쇄**(적분 와인드업 → 축별 누락 → 속도 피드포워드 부재, ζ=0.51 진동).
- `2026-09-01_drone_replaces_bp_uav.md` — 구 `BP_UAV` 갭 분석과 대체 작업 경과, 겪은 함정 6건
  (짐벌 본 공간, 자율주행 10km/h 고정, 커브 감속 무효, 바람 과잉상쇄, 트리거 불일치, 유니티
  빌드 상수 재정의). 당시 남겨둔 "2프로세스 실환경 검증"은 2026-09-15에 완료
  (`replication/2026-09-15_drone_two_pc_validation.md`).

## `rcws/` — RCWS 전용 devlog

- `2026-09-17_rcws_gunfire_hearing.md` (2026-09-17, **진행중**) — RCWS 청각 보조. 시각 탐지도 응시할
  기억도 없을 때 총구 기준 50m(09-21 확정; SoldierLab 레벨에서 25m는 적 배치 밖이었음) 안 최근 적 총성 방향을 4초 조사하는
  `ERCWSAutoAimPhase::InvestigatingGunfire` 단계(우선순위 시각 → 응시 → 총성 → 스윕, LOS 검사 없음).
  총성은 `UDetectableTargetSubsystem::ReportGunfire` 이벤트 버스(적군 사격 멀티캐스트가 서버에서 보고,
  아군 총성은 보고 안 함). **코드 완료, 풀 빌드·PIE 검증 전.**
- `2026-09-16_rcws_target_memory_and_body_part_aim.md` (2026-09-16, **진행중**) — RCWS 성능
  업그레이드. 타겟 기억/경계도(`FRCWSTargetMemory`) + 놓친 타겟의 마지막 조준점 4초 응시
  (`ERCWSAutoAimPhase::WatchingLastKnown`) + 탐지 샘플을 뼈(머리/가슴/골반)로 바꿔 머리만 내놓은
  적도 획득(`AcquireRule=AnyVisibleSample`) + 보이는 부위 조준(`AimPartPriority`). **코드·빌드·
  UGV/트럭 BP+인스턴스 `AcquireRule` 설정 완료, PIE 1차 확인(응시 7→4초) — 머리만 나온 적 획득/
  재출현/전방우선 세부 검증·튜닝 대기.**
- `2026-09-15_search_sweep_hull_relative_elevation.md` (2026-09-15) — 자동정찰 탐색 스윕이
  **내리막에서 하늘을 보던** 문제. 고각 목표가 월드 수평(`CurrentData.ElevationDegrees`) 기준이라
  차체가 기울면 위로 들렸음 → 새 `SearchSweepElevationDegrees`(기본 -3°, 차체 기준)로 좌우와
  기준 통일(트럭 카메라 오프셋 때문에 기준을 마운트 → 조준선으로 정정). **완료 — 2026-09-16
  빌드·UGV/트럭 확인.** Live Coding으로 빌드하면 BP "missing property" 함정 있음(문서 참고).
- `2026-08-31_selfdefense_camera_shake_bugs.md` — 자체방호축 카메라 떨림 + UGV 발사 셰이크
  누출 버그 2건, 원인 확정+코드 수정 완료(`SceneCaptureViewParity`/`RCWSProjectile`).
  **2-PC 실환경 검증 대기.**
- `rcws_preview_actor_asset_ref_todo.md` (2026-08-11) — `RCWSPreviewActor` 하드코딩 에셋
  경로 정리 방법. **아직 미착수 TODO.**

## `camera_pipeline/` — SceneCapture/QuadCam 아키텍처

- `rtsp_postprocess_parity_0820.md` (2026-08-20) — RTSP 스트림에 SSR/피격흔들림 안 나오던
  원인 규명(RCWS 메인뷰 카메라와 RTSP용 SceneCapture가 실제로 다른 카메라, 엔진이
  SceneCapture에 `ReflectionMethod=None` 강제). `SceneCaptureViewParity` 모듈 도입 —
  `rcws/2026-08-31_selfdefense_camera_shake_bugs.md`의 선행 문서.
- `2026-09-29_battlefield_capture_tsr_jitter_blink.md` (2026-09-29, 완료) — 자체방호 환경카메라(`BattlefieldCapture`)에서
  원거리 트럭 환기구가 ~1초 주기로 번쩍이던 원인 = TSR 지터(11샘플 × 2프레임 1회 캡쳐). SceneViewExtension으로 **이 캡쳐
  뷰만 지터 끔**(TSR 누적 유지). 기각안(트럭 Nanite, 캡쳐 TAA 끄기, 디버그 cvar)과 품질 트레이드오프, 다른 캡쳐 확장 보류.

## `ai_combat/` — 적/아군 AI·애니메이션·전투

- `2026-09-11_enemy_slide_and_fire_gate.md` (2026-09-11) — ★★ 적군이 **사격→엄폐 이동 시
  애니메이션 없이 미끄러지던** 원인 규명. 범인은 **`FireRecoil` 슬롯이 AnimGraph 메인 사슬에
  전신으로 물려 있던 것**(같은 그래프의 `ReloadSlot`은 원래부터 상체 전용이었다) — 버스트 직후
  바로 엄폐로 전환하는데 1.17초짜리 반동 몽타주가 이동 포즈를 100% 덮었다. 결정적 증거는
  로그의 `슬롯 Fire=1.00`. 아군 ABP도 같은 결함이라 함께 수정. 추가로 **배속**(`IsSprinting`
  오판정 → `PoseMoveSpeed` 300→200)과 그 부작용인 **사격 정지**(`PoseArrivalToleranceCm`
  10→50, 마커 도착 판정이 안 떨어져 한 발도 못 쏨)까지. **폐기한 가설 7건을 표로 남겼으니
  비슷한 증상 재발 시 먼저 읽을 것.** 진단 로그 2종은 `#if 0`으로 보존. §9 추기(09-17): 리플레이
  에서 재발 — 원인은 `BP_Enemy_kadex` CDO AnimClass가 옛 ABP였던 것.
- `2026-09-11_ally_skeleton_migration_prep.md` (2026-09-11) — ★★ **아군 신규 스켈레톤
  (`soldier_T_Skeleton`) 이관 완료.** 중복 입고된 두 폴더 중 **재생 길이로 `KS_`를 최종본으로
  확정**, 블렌드스페이스 2종 신설(27/23샘플), `ABP_Ally_kadex_T` 배선 — **본 참조 16곳
  매핑표**(Mixamo→UE5 마네킹, 겹치는 본 0개) + 시퀀스 8곳, `Rifle_Socket` 신설,
  `BP_Ally_kadex`와 **레벨 인스턴스 25명** 전환. 적군도 `AS_Enemy/` 단일화(CL 459로 엎드리기
  2종 입고)해 아군 의존 제거. 함정 3건: **CDO를 고쳐도 레벨 인스턴스는 안 바뀐다**(이 레벨은
  월드파티션이 아니라 액터 개별 저장 불가), **`set_pin_value`가 dirty 플래그를 안 세워 저장이
  조용히 누락**, **발소리 노티파이가 재익스포트에서 유실**. PIE 육안 검증 4건 대기.
  요청 메일은 `2026-09-11_ally_pose_request_email.html`.
- `2026-09-10_ally_enemy_animation_split.md` (2026-09-10) — ★ 디자이너가 적군 스켈레톤 수정 +
  애니메이션을 `AS_Enemy/`로 분리(CL 452·454·455)하면서 **공유 블렌드스페이스 참조가 끊겨
  에디터 로드 에러 발생**. 원인은 이름 변경 후 남은 리다이렉터의 대상이 삭제된 것(루트에 껍데기
  22개). 적군용 블렌드스페이스 2종 신설·배선으로 **적군은 복구 완료**, 아군은 새 스켈레톤
  (`soldier_T_Skeleton`)용 시퀀스 **입고 대기**. 아군/적군은 이제 애니메이션을 공유하지 않음.
- `2026-09-03_enemy_combat_fixes.md` (2026-09-03) — 적군 전투 로직 수정 3건. **Firing 상태에
  시간 제한이 없어 일부 적군이 영구히 멈추던 버그**(원인 2경로 규명, `FiringStuckGraceSeconds`
  안전망 + 상시 진단 로그), 사격선 안전장치의 검사 시작점을 캡슐 중심→실제 총구로 교정,
  왼손 파지 IK 양쪽 ABP에서 비활성화. **버스트 중 사격선 재검사 없음은 미해결로 남김.**
- `2026-09-01_enemy_spin_on_hit_investigation.md` (2026-09-01) — 살아있는 적이 이동 중 피격되면
  몸이 회전하는 현상 조사. 후보 2개(총구 Yaw 되먹임 / 소총 Pitch 무클램프 적분기)를 세워 진단
  로그로 실측했으나 **두 차례 모두 재현 실패 → 보류.** 증상 형태(이동 중에만 발생, 정지하면
  자연 복구)는 총구 Yaw 되먹임 가설과 일치하나 미확증. 재발 시 로그 되살려 재개.
- `2026-09-01_animation_asset_inventory.md` (2026-09-01) — **디자인팀 공유용**. 아군/적군이 실제로
  재생 중인 애니메이션 시퀀스 전체 목록(블렌드스페이스 3종·슬롯별 단발 동작·적군 전용)과,
  애니메이션 없이 코드로 구현한 동작들(기울임/조준각/사주경계/피격 반응/사망 랙돌 등) 정리.
  미사용 추정 애셋 목록과 개발 확인 예정 항목 4건 포함.
- `2026-08-31_enemy_squad_reorg.md` (2026-08-31, **09-21 superseded 배너** — New_kadex_0811 은 SoldierLab) — 적 15명 3분대 재편, 분대별 도주 경로/NavMesh
  필터, 경로 가중치 근본원인 수정. 경로 스플라인·`NavQueryFilter_EnemySquad1/2/3` 는 살아남아 분대별 `ASoldierZone.NavFilterClass` 로 흐른다.
- `2026-09-03_dynamic_squad_reassignment_and_casualty_log.md` (2026-09-03, **09-21 superseded 배너**) — 구 정원제·`ExcludeFleeing…`·`HoldFleeingEnemyFire`;
  New_kadex_0811 에서는 `Quota`/`SetTargetable`/`BreakContact` 가 대체. `kadex_test` 에서만 유효.
- `enemy_locomotion_animation_pipeline.md`/`enemy_hit_reaction_physics_system.md`/
  `enemy_scenario_combat_expansion.md` (2026-08-24~25) — 3단계 전투 확장(Part A~G 완료),
  피격 리액션 감쇠조화진동자 전환, 로코모션 시간-2배-흐름 버그 수정. **전부 완료.**
- `ally_ai_combat_system_status.md`/`enemy_ai_combat_system_status.md` (2026-08-10) —
  전투 컴포넌트 최초 구현 기록, 위 문서들 이전 상태.

## `level_new_kadex_0811/` — 신규 레벨 디자인/시나리오

- `2026-09-22_scenario_restart_implementation.md` (2026-09-22, **09-23 2-PC 절 추가 / 완료 — L_SoldierScenario 단일 프로세스 ✅ · New_kadex_0811
  2-PC ✅**; 남은 것 = 드론 짐벌 배율 + **상태 패널 누적값** 리셋 **빌드 대기 2건** · 10사이클 추이 · 장시간 반복 · P4 제출 조율. 낙하산 `RespawnActors` 연결은 **불필요로 종결**(정적 액터)) — ★★ **시나리오 재시작(확인창 + 자동 재시작) 구현 기록.** 흐름(DT 행 `ScenarioRestartPrompt` → 확인창 →
  `RequestScenarioRestart` 유일 진입점 → 페이드/리셋/재스폰/재시작 시퀀스) · 변경 파일 표(신규 `UI/ScenarioResettable.h` ·
  `UI/ScenarioRespawnSubsystem` · `UI/RestartPromptWidget`, 수정 ~35 + DT 2) · `WBP_RestartPrompt` 규격(BindWidget 이름) · **확인창 축 규칙**(자체방호
  우선, GameState 멀티캐스트 + PC Server RPC) · 설계와 달라진 점(지오메트리 캐시도 비움 · `AutoPossessAI` 함정 · 딸린 소총 액터 동반 파괴) · 남은 것.
  **§5 2026-09-23 2-PC 절**: ★ 최종 원인 = **레벨 GameMode 오버라이드가 `GM_SoldierLab`**(titan GameState/PC 부재 → 멀티캐스트 서버-로컬 폴백 +
  드론 이중 시뮬 주체, **2-PC 에서만** 드러남) → `BP_KadexTestGameMode` 복구 · 레벨별 GameMode 표 · 멀티캐스트 3종(`Begin`/`Apply`/`End`) ·
  **드론 리셋 권한 분리**(서버 = 복제 명령 / 시뮬 주체 = 물리·짐벌·`bParachuteObserved`) · 소총 누수 근본 수정([W116], SoldierLab `EndPlay`) ·
  스냅샷의 에디터 임시 컴포넌트 제외 · PlayerController `public`(C2248).
  **09-23 후속**: 드론 **짐벌 배율(`ZoomLevel`) 리셋 추가**(`InitialZoomLevel` 스냅샷 → 시뮬 주체가 복원, **빌드 대기**) · 런타임 상태 전수 대조
  (일부러 안 되돌리는 것 = 진단 누적값 · 복제 미러 · BeginPlay 1회 세팅) · `ViewMode` 는 로컬 카메라 모드라 **리셋 안 함**(결정) · **낙하산 재스폰 폐기**(정적 액터).
  **09-23 후속 2(§5 끝)**: **상태 패널 누적값**(드론 배터리·비행시간 / UGV 배터리·주행거리) 리셋 신설 — ★ **호출 위치가 두 컴포넌트에서 정반대**(드론 `UStatusHUDComponent` =
  비복제 + 틱 권한 게이트 없음 → **모든 프로세스**, 시뮬 주체 판정보다 먼저 / UGV `UUGVStatusComponent` = 복제 + 서버 생성 → **서버만**) · 같은 계열 전수 검색 결과 · RCWS 탄약은
  이미 초기화됨(확인) · **빌드 대기**.
- `2026-09-10_scenario_auto_restart_design.md` (2026-09-10 작성, 09-22 전면 개정 v3, **구현 완료 — 상단 "구현에서 달라진 점" 배너 1~4(09-22) + 5~9·11(09-23: 각 프로세스 정리 · 드론 권한별 리셋 · 소총 누수 근본 수정은 SoldierLab · 짐벌 배율 · 낙하산 폐기 · **상태 패널 누적값**)**) — 재시작 설계:
  병사는 스냅샷 재스폰·차량/드론은 부활로 정한 근거, 실측 사실 표, 무한 재시작·오발동 분석(`ScenarioComplete` 의 Prereq 가 루프 브레이커),
  층별 리셋 목록(SoldierLab 리셋 계약 4a · titan 서브시스템 · UGV/트럭/드론), 스냅샷 메커니즘, 확인창/체크값(ini) 설계, 실행 시퀀스 12단, 검증 "PIE 와
  동일" 판정표. 본문은 설계 시점 그대로 — 최신 사실은 위 구현 문서.
- `2026-09-21_soldierlab_migration_new_kadex_0811.md` (2026-09-21, 진행중 — 첫 PIE 전 체인 완주, 코드 수정 뒤 2차 PIE 대기) — ★★ **본 레벨
  New_kadex_0811 을 SoldierLab 병사로 이관.** 구 `BP_Enemy_kadex`/`BP_Ally_kadex` 40 삭제 → `BP_Soldier_Hostile_1~15`/`Friendly_1~25` 같은
  트랜스폼 · 마커 113·경로 스플라인·드론 경로 유지 · **`ASoldierZone` 8개** 좌표/반경/필터 표(분대별 — 경로 필터를 존이 나른다) ·
  `SquadZones` · DT `DT_ScenarioSteps_ThreeStage_SoldierLab` **26행** diff(뺀 4·더한 4·`IssueSquadOrder` 로 바뀐 6) · 첫 PIE 시간 축(+1 → … → +354,
  `[Squad] reinforce` 로그) · **3분대 3차 "도주"가 엄폐 홉이었고 UGV 가 제외 뒤에도 쏘던 문제** 원인 셋 · `GM_SoldierLab` 에서도 도는 이유와 잃는 것 ·
  이 레벨에서 죽은 구 기제 · 2차 PIE 체크리스트 7단계 · [C-163][C-164][Q51][W104]~[W106], [W70] 해결.
- `2026-09-18_soldierlab_three_stage_test_level.md` (2026-09-18, 09-21 추기 — 본 레벨 이관 완료) — ★ SoldierLab 병사로 3단계
  시나리오를 다시 저작한 **시험 레벨 `L_SoldierScenario`**(적 15·아군 20·UGV·트럭, 드론 없음) + DT
  `DT_ScenarioSteps_SquadThreeStage` **13행**(`IssueSquadOrder` — `EngageRangeCm`·`Quota`·새 트리거 `EnemyFireStarted`/
  `EnemyNearFriendlySoldiers`; Retarget·HoldFleeingFire 행이 왜 필요 없나) · 예상 흐름 · ⚠ 1차 전투지가 아군에서 54 m 인 열린 문제
  [Q50] · 검증 체크리스트 8단계 · MCP 저작 함정 4건. 코드 수정은 `soldier_ai_lab/squad/2026-09-18_…`.
- `2026-09-17_soldierlab_squad_scenario_link.md` (2026-09-17, 09-18 정정 배너) — 시나리오 ↔ SoldierLab 분대 명령 연결: 새 이펙트
  `IssueSquadOrder`·행 필드 `SquadOrder`·트리거 `SquadOrderAchieved`·`ScenarioConfig.SquadZones`·브리지. 저작안 3절은 09-18 시험 DT 로 대체.
- `2026-09-15_demo_ugv_autofire_on_zone1_arrival.md` (2026-09-15) — **데모 모드 UGV RCWS
  자동사격(탐색 스윕) 시작 시점을 레벨 시작 → 1차 목적지 도착으로.** 새 이펙트
  `SetDemoUGVAutoFire`(데모 게이트 내장) + DT 행 `UGVArriveZone1`(`ActorStopped`), 레벨 시작
  강제는 지휘소만 남김. **완료 — 09-15 빌드 후 EffectType 설정·DT 저장, 09-16 PIE 확인.**
- `scenario_authoring_guide.md`/`scenario_three_stage_combat.md` (2026-08-23, 09-15 갱신, **09-21 배너**) — 3단계 전투
  시나리오 DataTable 구현. **완료.** ⚠ 09-21 부터 New_kadex_0811 의 적/아군은 SoldierLab + `IssueSquadOrder` — 본문의 구 병사 흐름은
  `kadex_test` 에만 해당. 저작 가이드 **2.6절**(09-21 신설)이 SoldierLab 시대의 `SquadOrder` 필드 · 트리거 3 · `ASoldierZone` 프로퍼티 · `SquadZones` 레퍼런스.
  **2.7절**(09-22 신설) = 시나리오 재시작 — 이펙트 `ShowRestartPrompt` · 행 `ScenarioRestartPrompt` 저작 예 · `Scenario|Restart` 6필드 · 확인창 축 규칙 ·
  체크값 ini/`-autorestart` · 콘솔 `titan.ScenarioRestart`/`titan.ScenarioAutoRestart`/`SoldierLab.ResetWorld`; 6절 트러블슈팅에 재시작 항목 5개.
  **09-23 추가**: 2.7절에 ⚠ **레벨별 GameMode 표**(New_kadex_0811·kadex_test = `BP_KadexTestGameMode` / kadex_lobby = `BP_TestGameMode` /
  `GM_SoldierLab` 은 단일 프로세스 관전 전용) + 트러블슈팅에 "2-PC 에서 클라만 페이드 없음·드론 미복귀" 항목.
- `2026-08-26_level_rescale_to_real_world.md` (2026-08-26 작업, **09-22 사후 문서화** — 당시 devlog 가 없어 다른 세션이 몰랐음) — ★★
  **레벨을 현실 1:1 로 재스케일.** 랜드스케이프 대각 코너 위경도 2점으로 레벨이 26.2% 크다는 것을 4방법(Vincenty 등 0.057% 안)으로
  확정, 월드 원점 기준 k=0.7921933250 3축 균일 축소(랜드스케이프 loc/scale · 액터 328 위치 · 볼륨 6 · 도로 스플라인 4 · PCG 숲 10),
  큐브 독립 검증 0.001 m. **PCG 숲 점박이 마스크가 바뀐 원인**(Spatial Noise 가 월드 XY 고정) → `tree2`/`plant` 그래프 Transform.scale=1/k 로
  정확 복원(에셋 이름 ≠ 실제 참조 그래프 함정). `GeoCoordinateUtils.h` 보정점 5→2, **`GetDistanceScaleFactor` 1.2135→1.0006 파급
  15곳**(표시 -17.6%, 물리 스펙 경로 씬 속도 +21% → 시나리오 타이밍 재점검 필요), 단위 변환 23곳 전수 감사. MCP 함정 6건. 미해결:
  랜드스케이프 동쪽 변이 미니맵 밖 205~340 m. **완료.**
- `scenario.md` (2026-08-22) — 위 시나리오 요구사항 정리판(구현 전).
- `new_kadex_0811_forest_perf.md` (2026-08-22) — PIE 2.3→31fps 성능 폭락 수정. **완료.**
- **[2026-08-31] 내비메시 구축 문서는 여기 없음** — `vehicle/ugv/
  2026-08-27_new_kadex_0811_navmesh_autonomous_driving.md` 참고(예전엔 이 폴더에도 08-22
  시점 구버전 사본이 있었는데, 완전히 그 문서의 부분집합이라 중복 제거함).

## `sfx_vfx/` — 사운드/파티클/환경 이펙트

- `2026-09-21_combat_audio_voice_budget_and_attenuation.md` (2026-09-21) — **최신.** 35명 교전
  총성 끊김 원인(발사량 ×4 × 보이스 상한 32), MaxChannels 64, 총성 크랙/꼬리 MetaSound 분리 +
  Concurrency 2겹, C++ 런타임 감쇠 NaturalSound+LPF 통일, SoldierLab `SurfaceImpactEffects` 복원,
  1인칭 사수 강조를 `SA_Weapon` 커스텀 곡선만으로. `SA_Weapon` 정본 = SoldierLab 쪽.
- `hit_effects_update_2026-08-26.md` (2026-08-26) — 지형/바위/나무/PCG 전체 재질별
  피격 이펙트, `PhysMaterialOverride` 필수 교훈, VFX 노출 수정. 09-21 후속 단락 있음.
- `hit_effects_update_2026-08-13.md` (2026-08-13) — 도탄/혈흔·화염 VFX/사운드.
- `hit_effects_implementation.md` (2026-08-12) — 1차 구현+트러블슈팅.
- `hit_effects_idea.md` (2026-08-12 이전) — 최초 기획 메모.
- `wind_system.md` (2026-08-29) — `AWindSource` 동적 바람, 식생+Niagara 8종+드론 물리 연동.

## `ui/` — WBP 위젯 개발

- `kadex_test_dashboard_wbp_spec.md` (2026-08, 최신) — **현재 실사용 위젯**
  (`UGVTestDashboardWidget`/`SelfDefenseDashboardWidget`/`AxisSelectionWidget`)의
  `BindWidgetOptional` 필드 스펙 + 버그 수정 이력. `guide/ui_dev_guide.md`(구 Monitor1Widget)
  를 대체하는 현재 UI 개발 기록.
- `2026-09-10_graphics_settings_implementation.md` (2026-09-10) — **Graphics 탭 구현.**
  `UTitanGraphicsSettings`(`UDeveloperSettings`) 단일 소스로 ini 하드코딩을 이관해 **런타임 품질
  변경을 처음으로 가능하게** 만든 작업(이전엔 cvar 우선순위 때문에 완전 no-op이었음). 반사 방식
  SSR 플랫폼 통일, Lumen 튜닝이 Windows 전용이라 **Linux 납품 빌드에 빠져 있던 것** 해소,
  나무 WPO/LOD를 숫자 입력으로 노출(실측 곡선 기반), 카메라 캡쳐 주기 3종. WBP 계약(레이아웃 포함)과
  **함정 8건**(UDeveloperSettings 링크 에러, `INDEX_NONE` 타입, Overlay에서 Fill=겹침, 액터 존재
  여부로 UI 비활성화 금지 등). **빌드·WBP·탭 23행·게임 레벨 대상 탐색까지 확인 완료
  (§8-1에 식생 대상 50개 전수 확인) — 패키지 실측(VSync·Linux 룩)만 남음.**
- `graphics_settings_analysis.md` (2026-08-21, **팔로업 2026-09-03**) — 위 작업의 **사전 조사·설계**
  문서. `DumpCVars` 전수 실측으로 "하드코딩된 cvar가 스케일러빌리티를 차단한다"는 핵심 제약을 확정
  (§0-2 F), 채택 구조 설계(§9), 확정 스코프(§10), 프리셋 설계(§12). 왜 그렇게 만들었는지를 찾을 때 볼 것.
- `ingame_settings_input_system.md` (2026-08-21) — Settings 위젯 Input 탭 구현. **완료.**
- `truck.png`/`ugv.png` — 듀얼 모니터 레이아웃 목업 이미지.

## `protocol/` — LIG 원격통제기 UDP/JSON 프로토콜

- `protocol_icd.md` (최종 2026-08-31, 09-15 §3.3/§4.1 RTSP 전송 "TCP만"→"TCP/UDP 둘 다, TCP 권장"
  정정) — **필드 단위 명세, 가장 자주 참조되는 핵심 문서.**
- `ugv_rc_feature_gap_analysis.md` (최종 2026-08-31) — UGV축 cmd별 구현 상태 대조표.
- `selfdefense_rc_feature_gap_analysis.md` (2026-08-17) — 자체방호축 동일 성격 대조표.
- `lig_questions_0816.md` (최종 2026-09-02) — **LIG 문의 통합본, 최신.** 1차 답변 반영 완료,
  후속 질문 3건 + 회신 2건 발송 대기(§5-2는 질문이 아니라 결정 통보).
- `2026-09-22_minimap_georeference_for_lig.md` (2026-09-22, 완료 — 수치 확정, **발송 대기**) — LIG 가 요청한 지도 이미지
  지리참조(픽셀 스케일 ScaleX/Y/Z + 타이포인트 I,J,K,X,Y,Z). 현재 미니맵 `m_map.png`(1024×1024, EPSG:4326 정렬, 행 47~977 콘텐츠)
  기준 확정값: ScaleX 0.000055534311 · ScaleY 0.000047961398 deg/px(≈4.88/5.32 m, 비정사각), 타이포인트 (0,0)/(0,47) 둘 다,
  네 꼭지점 WGS84+UTM 52S, 격자 수렴각 -0.50° 주의, 원본 PNG 위치. 열린 결정 = 동쪽 확장+UTM 정렬 재생성(그러면 코드 상수 8개
  재측정). 답장 초안 부록.
- `2026-09-02_object_class_expansion.md` (2026-09-02) — 탐지 `ObjectClass`를 `Human`/`Car`
  2값에서 플랫 6값으로 확장. 낙하산이 `Car`로 나가던 원인 조사 + 판정 순서.
- `lig_icd_ugv_rc_full.md` (2026-08-14) — LIG 정식 ICD 원문 전체 전사.
- `lig_questions_0807_draft.md`/`lig_questions_udp_reliability_0814.md` — **폐기, 기록용**
  (`lig_questions_0816.md`로 통합됨).

## `rtsp/` — RTSP 영상 송출

- `2026-09-16_rtsp_axis_gate_dangling_timer_fix.md` (2026-09-16, 완료) — `FRtspAxisGate::ResolveLocalAxis`의
  0.1초 폴링 타이머가 raw `this`를 캡처해 **오너가 5초 안에 파괴되면 크래시**(리플레이 재생 스크럽 루프로
  발견, 정상 플레이에도 잠재). `Owner` 인자 + `CreateWeakLambda`로 수정, 호출부 5곳(UGV 브릿지/드론/
  AmbientFX/트럭/구 UAV). 빌드는 사용자.
- `rtsp_poc_findings.md` (2026-08-18 최종, 09-15 추기) — NVENC/GStreamer PoC 전체 기록(크로스플랫폼
  포함). SDK 13.1.15 언급은 당시 기록이고 09-15에 13.0.37로 내림(추기 주석만 붙임).
- `2026-09-15_lig_rtsp_describe_timeout_analysis.md` (2026-09-15, 완료) — **LIG PC에서 RTSP DESCRIBE
  20초 타임아웃 원인 분석.** IP 오인 → 스크린샷 대조로 "인코딩 프레임 0장" 확정 → 원인은 드라이버
  595.84 vs NVENC SDK 13.1(610+ 필요). SDK 13.0.37로 내림 + 인코더 실패 시 마운트 미등록(즉시 404)
  코드 수정, 사내 리눅스 PC를 595.84로 내려 실증. LIG 재발송만 남음.
- `rtsp_integration_complete_0817.md` (2026-08-17, 09-16 §5 추기 1줄) — 실 카메라 연결 완료, mount 확정,
  §5 `FRtspAxisGate` 축 게이팅 도입 배경.
- `rtsp_integration_status_0817.md` (해소됨 — 위 문서로 대체).
- `linux_wayland_x11_present_bottleneck.md` (2026-08-19, 09-15 추기) — Linux 풀스크린 프레임폭락 해결.
  09-15 추기: X11 폴백 래퍼 수동 복사는 `Config/BootstrapPreamble.sh` 자동 삽입으로 대체됨.
- `rtsp_client_reception_guide.md` (2026-08-24, 09-15 §2.4에 서버 드라이버 요구 1줄 추가, §1.1/§1.2
  프로토콜 행을 "TCP/UDP 둘 다, TCP 권장"으로 정정) — **LIG 공유용 최종 수신 가이드.**
- `rtsp_latency_investigation.md` (2026-08-19 최종) — 지연 441ms→68ms 조사 전체.
- `RTSP_Perf_Investigation.md` (2026-08-19, 09-15 추기) — 위 조사 원본 진행 로그.
- `rtsp_resolution_customization_0820.md` (2026-08-20) — 해상도 커스터마이징+CCTV 잘림버그+
  RCWS 이중렌더링 해결. **완료.**

## `replication/`

- `2026-09-17_enemy_anim_death_replication_gaps.md` (2026-09-17, 완료·2-PC 실기 검증 대기) — ★★
  **Chronicle 리플레이로 드러난 클라이언트 결함 4건**(리플레이 = 클라이언트 하나 더이므로 자체방호
  클라이언트도 동일). (1) 적군 `GaitTopSpeed`/`IsSprinting` 미복제 → 서버 전용 틱만 쓰는 값이라
  클라 CDO 0 → ABP `Speed` 항상 0 → **idle 포즈로 미끄러짐**(08-25 gait 재설계 변수가 8월 복제
  목록에 빠져 있었음); (2) 피격 스프링 트리거·적분 서버 전용 → Multicast + 비권위 틱 적분; (3) 사망
  BP 체인(래그돌/총 분리) 서버 전용, 클라엔 `IsDead`만 → `FEnemyDeathReplicationInfo` 복제 프로퍼티
  + `OnRep`에서 C++로 같은 코스메틱(RPC 대신 프로퍼티인 이유: 리플레이 체크포인트/늦은 접속),
  `BP_Enemy_Base`에 `NotifyDeathForReplication` 노드 1개 삽입; (4) 리플레이 재생 월드에서 시나리오
  스텝이 새로 발동 — BeginPlay가 DemoNetDriver 부착보다 먼저라 `NM_Client` 가드가 샘 →
  `IsPlayingReplay()` 매 틱 게이트 + `BeginMove/Flee` HasAuthority(리플레이 전용, 실기 무관).
  진단 cvar `Enemy.ClientAnimDiag`. 서버 동작은 전부 불변.
- `replication_audit.md` (최종 2026-08-14, §8 최신; §0-1에 2026-09-17 추기, **§9 2026-09-23 신설**) — 리플리케이션
  감사+구현 로그, 거의 완료. ⚠️ §8의 "UAV(2026-08-13 구현 완료)" 항목은 구 `AUAVPawn` 기준이라
  옛날 얘기 — 새 드론은 아래 문서 참고. **§9 = 레벨 GameMode 오버라이드가 titan 계열이 아니면 2-PC 가 조용히 깨진다**
  (`GM_SoldierLab` 실사고: titan GameState/PC 부재 → 재시작 멀티캐스트 서버-로컬 폴백 + 드론 이중 시뮬 주체; 단일 프로세스에선 증상 없음) + 멀티캐스트 3종
  + 후속(짐벌 배율은 시뮬 주체가 복원) + **후속 2: "복제되는 상태는 서버가, 비복제 로컬 상태는 각 프로세스가"의 사례 2개** — 상태 패널 리셋 호출 위치가 드론(비복제 → 전 프로세스)과
  UGV(복제 → 서버만)에서 정반대.
- `2026-09-23_net_relevancy_battlefield.md` (2026-09-23, 완료) — ★★ **클라 드론 화면에 전장이
  통째로 없던 원인 = 거리 기반 네트워크 관련성.** 기준점은 씬캡쳐가 아니라 연결의 ViewTarget
  (=트럭)이고 전장은 923 m 밖인데 컷이 150 m(엔진 기본 `NetCullDistanceSquared`)라, 병사가
  얼어붙고 사격 멀티캐스트가 **송신 단계에서 폐기**되고(`NetDriver.cpp:8243`) `bIsRevealed`
  OnRep도 안 와 탐지에서 빠졌다. 드론만 멀쩡했던 건 `SetOwner` 덕, 낙하산은 `bReplicates=false`라
  로컬 사본이어서. **Solo/호스트 자기화면은 관련성 판정 자체가 없어 증상이 안 난다**(3형태 표).
  수정 = 탐지 대상 액터 always relevant + 성능 계산. 부록: 클라에만 치트 매니저가 없어
  `ToggleDebugCamera`가 죽던 건(`AuthGameMode` 부재).
- `2026-09-15_drone_two_pc_validation.md` (2026-09-15, 완료) — ★ **2대 PC 실환경 첫 검증 + 버그 3건.**
  (a) 데모 모드에서 서버·클라 둘 다 주체(판정 순서), (b) 풀 시스템에서 `Server_ReportState`가
  **로그 없이** 폐기 — 엔진 기본 `AutoPossessAI`로 AI 컨트롤러가 빙의해 `APawn::GetNetConnection`이
  null(`SetOwner`만으론 부족), (c) 서버가 주체일 때 Rep*를 아무도 안 채움. "왜 주체가 모드별로
  다른가" 표, 진단 로그 읽는 법, 검증 로그 근거.
- `2026-09-01_drone_client_authoritative.md` — 새 드론은 서버가 아니라 **자체방호축 클라이언트가
  시뮬레이션**한다(조종 주체가 그쪽이라). Chaos Resimulation 기각 근거(RTSP +33ms, UGV 거동
  변화), 배선, `SetOwner` 소유권 함정. 상단 09-15 배너로 판정 규칙 변경·AI 빙의 함정·서버 주체
  게시 경로 반영. 2대 PC 검증은 2026-09-15 완료.

## `replay_chronicle/` — Chronicle 리플레이 녹화/재생 에디터 툴 (2026-09-16 신설)

- `2026-09-16_chronicle_replay_plugin.md` (2026-09-16, 완료) — ★ **`Plugins/Chronicle`**(에디터 전용,
  게임 모듈 의존성 없음). 엔진 Replay System(DemoNetDriver)을 EUW 패널로 감싼 것: PIE 자동 녹화,
  목록/재생(PIE를 대신 띄움), 타임라인 스크럽, 배속 0.5~5x, 카메라 텔레포트/Follow/Free. **Rewind
  Debugger 대신 Replay System을 고른 이유**(Niagara/사운드/UI가 나옴), **엔진 함정 12건**(Standalone
  녹화 시 Multicast 전부 누락 → Listen Server 필수, `demo.RecordHz` 기본 8Hz, `bIsEnabledInPIE`,
  `USlider::SetValue`가 OnValueChanged를 쏴서 생기는 스크럽 무한루프, 첫 스크럽의 심리스 트래블로
  상태 유실 → 매 프레임 재강제, 일시정지 중 스펙테이터 입력 등), 낙하산 숨김 리플리케이션 사각지대
  후보. **사용자 가이드는 `Plugins/Chronicle/README.md` / `Chronicle_Guide.html`**(여기 중복 안 함).
  상단 09-17 추기로 후속 문서 연결.
- `2026-09-17_replay_respawn_and_physics_proxy_fixes.md` (2026-09-17, 완료) — ★ 2일차. **리플레이
  체크포인트는 레벨 배치 액터를 클래스에서 재스폰**(`DemoNetDriver.cpp:3368`)하므로 복제 안 되는
  프로퍼티의 인스턴스 오버라이드가 전부 CDO 값으로 보인다 — `BP_Enemy_kadex` CDO AnimClass가 옛
  `ABP_Enemy_kadex2`라 09-11 "사격→엄폐 미끄러짐"이 리플레이에서만 재발, CDO를 `_New`로. **UGV/드론
  1초 주기 앞뒤 지터**는 물리 복제 `Default` 모드의 4m 하드 스냅 → 재생 월드 프록시에
  `PredictiveInterpolation` 강제(1초 스캔). **스크럽 뒤 UGV가 레벨 원위치에 박힘**은 재스폰 직후
  초기 `ReplicatedMovement` 1회가 물리 바디 준비 전에 버려지고 정지 차량이라 다시 안 오는 것 →
  1.5초 스냅 창(0.25초마다 `GetReplicatedMovement()`로 텔레포트, 일시정지 중에도). UI: 드롭다운=
  텔레포트, Go/Follow, Free=현재 시점, 일시정지 시작, 끝에서 Play=처음부터.

## `rc_mockup_tools/` — RC 목업/테스트 클라이언트

- `udp_protocol_client/README.md` (2026-08-15), `rtsp_viewer_test/README.md` (2026-08-17) —
  UDP/RTSP 테스트 도구.
- `hq_stub/README.md` (2026-08-06) — NATS 기반이라 전송계층 UDP 통일 결정으로 보류 상태.

## `packaging/` — 패키징/배포 실행 절차

- `kadex_0915_패키징_실행가이드.md` (2026-09-15, 완료) — **★ 현재 배포용 실행 가이드(패키지와 함께
  받는 쪽에 넘기는 문서, Ubuntu 실행 절차만).** 파일명은 배포물 관례(`kadex_<빌드날짜>_…`)라 날짜 접두
  규칙의 예외. 필수 라이브러리·NVIDIA 드라이버 **≥570**·Vulkan ICD 두 경로, `./titan_example.sh` 하나로
  Wayland/X11 자동 판별, 축 선택 화면 입력값, 접속 정보(RTSP TCP/UDP 둘 다, 방화벽 시 TCP 권장),
  증상표, 09-02 대비 변경 이력. **패키징(우리 쪽) 절차는 여기 없고 아래 내부 가이드 §2가 유일.**
- `kadex_0902_패키징_실행가이드.md` (2026-09-02, 09-15 갱신, **폐기 — 위 0915 가이드로 대체**) — 09-02
  패키지를 받은 쪽과의 대조용으로만 보관. `run_titan_example.sh`/`titan_example_x11_fallback.sh` 안내가
  더 이상 맞지 않음(세션 판별이 `titan_example.sh`에 내장됨).
- `2026-09-15_linux_nvidia_driver_595_run_install.md` (2026-09-15, 완료) — **고객 환경 재현용**: 리눅스
  테스트 PC 드라이버를 특정 버전(595.84)으로 맞추는 NVIDIA 공식 `.run` 설치 절차, 검증 명령,
  커널 hold, Secure Boot 주의, 원복(`--uninstall` → `ubuntu-drivers install`). `.run` 설치본은 Vulkan
  ICD가 `/etc/vulkan/icd.d/`에 들어감.
- `ugv_controller_demo_실행가이드.md` — **통제기 목업 GUI(`ugv_controller_demo`, 구 `ugv_rc_gui`)
  실행 가이드(단독 배포용).** Windows/Linux 설치, 실행 인자, 조작 순서(연결 → 제어권+REMOTE →
  주행/조준), 조이스틱 매핑, 탐지 bbox 색.
- `2026-09-02_linux_package_ugv_host_rc_test_guide.md` — 위 문서들의 **내부용 상세판**. 패키징 절차,
  데모/풀 시스템 스위치 배경, 코드 근거, 로그 확인 포인트까지 포함. §2는 2026-09-15에 정정
  (MCP 서버 켜진 상태에서는 커스텀 빌드로 패키징; §2-4 래퍼 스크립트 복사 절차는
  `Config/BootstrapPreamble.sh` 자동 삽입으로 폐지; §2-5 성공 확인 체크리스트·§2-6 패키징 절차
  변경 이력 신설 — **패키징 절차의 유일한 문서**), §3-1/§7-1에 드라이버 버전(≥570)·`.run` ICD
  경로 추기, §3-2/§3-4/§7을 프리앰블 기준으로 수정, §1/§3-3/§5/§7의 RTSP "TCP만" 표기를 "TCP/UDP
  둘 다, TCP 권장"으로 정정.
- `2026-09-15_linux_cook_failed_mcp_port_clash.md` — 쿡이 `Done!`까지 돌고도 `Cook failed`로 끝나던
  원인 조사. 커맨드릿은 Error 로그 1줄이면 실패하는데, 쿠커가 에디터와 같은 127.0.0.1:8000에 MCP
  서버를 띄우려다 남긴 바인드 에러가 원인(CDO Constructor 에러는 오진, `AdditionalCookerOptions`
  ini 키는 존재하지 않음). `ProjectCustomBuilds`로 쿠커 포트만 8001로 비켜 해결, 향후 진단 절차 포함.

## `infra_architecture/`

- `2026-09-16_slomo_physics_dt_clamp_investigation.md` (2026-09-16, **보류**) — **`slomo N>1`이
  New_kadex_0811에서 안 먹는 이유.** `UWorld::Tick`이 TimeDilation을 곱한 dt를 `FChaosScene::SetUpForFrame`이
  `MaxPhysicsDeltaTime`(엔진 기본 1/30)으로 클램프 → 20~30fps 레벨은 배속 여유 0(`/Game/test` 58fps는 2배까지).
  프로젝트가 물리 스텝 설정을 한 번도 안 건드렸음을 P4 31리비전으로 확인, 드론 가이드 §10.2 오독 정정,
  해결 후보 3개 비교표(MaxPhysicsDeltaTime 상향 기각 / 서브스테핑 1/55 보류 / 세트포인트 배속 보류).
  실제 요구는 리플레이 툴(`replay_chronicle/`)로 해결돼 구현 안 함.
- `architecture_decisions.md` (2026-08-07) — Layer C/UGV축/Layer A 아키텍처 결정 기록.
- `system_architecture_design_spec.md` (2026-08-05) — 최초 PDF 분석 데이터화.

## `genesis/` — 별도 병행 연구 트랙(일시중단, 재개 가능성 있어 보관)

- `2026-08-12_genesis_ugv_conversion.md` — `titan_example_genesis`(별도 프로젝트 카피)에서
  UGV를 외부 Python 물리 서버(Genesis)로 구동. 2026-08-31 확인: 최근 작업 안 함, 폐기 아님.

## `soldier_ai_lab/` — 고사실감 병사 AI/애니메이션 R&D (자체 `CLAUDE.md` · `IMPLEMENTED.md` · `CURRENT_STATE.md` · `OPEN_ITEMS.md` 보유)

이 폴더는 자체 인덱스 체계를 갖는다 — 전체 목록은 `soldier_ai_lab/CLAUDE.md` 1절(읽기 순서)과 `soldier_ai_lab/IMPLEMENTED.md`.
여기에는 titan 본체 상태와 직접 맞물리는 최신 문서만 적는다.

- `animation/2026-09-29_movement_policy_and_playrate_band.md` (2026-09-28~29, 완료 — **PIE 실측 ✅**) — ★★ **상황별 이동 정책 + 재생배율 밴드**:
  "시나리오에서 병사가 너무 빠르다" 는 제보에서 시작해, **애니메이션을 모르는 사람도 표로 조절할 수 있게** 만든 층. ★ **이 문서의 핵심은 밴드다** —
  ABP `Get_DynamicPlayRate` 는 `Clamp(Speed2D / 클립의 MoveData_Speed, Min, Max)` 로만 적응하고, **우리 클립엔 `Min/MaxDynamicPlayRate` 커브가 없어
  대체값 0.75 / 1.25 가 쓰인다** → 실측 `ALLY_MM_Rifle_Walk_Fwd` **291.31** · `Jog_Fwd` **582.62**(= 캐릭터 `WalkSpeeds`/`RunSpeeds` 와 동일, 클립이
  리타이밍돼 있다) → **미끄러짐 없는 구간은 Walk 218~364 · Jog 437~728**(그 사이 364~437 은 두 세트가 다 MM DB 에 있어 매칭이 섞는다).
  구현 = 신규 `AI/SoldierMovementProfile.{h,cpp}` + `Content/SoldierLab/Data/DT_SoldierMovement`(**행 16개 × 열 2개** — `MaxGait` 걸음걸이 **상한**(클립이
  그 속도로 authored 되어 공짜) + `SpeedScale`(**0.75~1.25 클램프 3겹** = `UPROPERTY meta` · 코드 · 전역 cvar 합산 후)), 상한은
  `USoldierEngagementComponent` 의 gait 결정 **끝**에서 씌우고 배율은 **gait 속도 벡터**에 곱한다. 상황 선택 2단 = ① `BreakContact`→`Rush` 명령이
  이긴다(도주하며 재장전하는 병사가 걸어서 도망치지 않게) ② 아니면 **최종 속도(걸음 기준값 × 배율)가 가장 낮은 행**. ⚠ **`Aiming`(견착 —
  엄폐지 사이를 달릴 때도 켜지므로 Jog 0.90) ≠ `Firing`(멈춰서 쏨 — Walk 0.75)**. cvar `SoldierLab.Move.Enabled`/`.SpeedScale`/`SoldierLab.Debug.Move`.
  ★★ **2026-09-28 1차 구현은 폐기·정정됐다**(7절): CMC 의 `MaxWalkSpeed` 에 곱했더니 **`AC_PreCMCTick` 이 CMC 직전에 매 프레임 덮어써서 전혀 안
  먹었고**, 입력으로 옮긴 뒤엔 **배율 0.45~0.7 이 밴드 밖이라 발이 미끄러졌다**(안전범위 0.45~1.2 는 **측정 없는 임의값**이었다) · **가속·회전 2열은
  같은 이유로 죽은 값**이라 삭제("적용된다" 는 오보 정정). 검증 = 전 행을 `Walk/0.75`(218, 원래의 2.7배 느림)로 밀어 **느려지고 미끄러짐 없음** 확인
  → 계획값 원복 후에도 정상(둘 다 사용자 확인). 원칙 **P193**, 값 **[C-174]**(방향별·Crouch 클립 authored 속도 미측정 — 현재 미끄러짐은 관측 안 됨),
  작업 **[W120]**(218 보다 느리게 = 커브 베이크/클립 추가, **표로는 불가**) · **[W121]**(가속·회전은 틱 순서부터) · **[W122]**(`DT_SoldierMovement` P4 add).
  ⚠ 디자이너용 안내는 `guide/soldier_movement_speed_guide.html`(09-29 재작성 중 — 09-28 판은 3열·0.45~1.2 기준이라 틀렸다).
- `ai/2026-09-23_corpse_vehicle_interaction_and_weapon_drop.md` (2026-09-23, **코드 완료·검증 대기**) — ★ **차량이 밟는 시체 · 사망 시 무기 드롭**.
  UGV(`BP_UGV_0901`)가 쓰러진 적을 밟고 지나가는데 **밀려나는 그림은 좋지만** 시체가 땅에 박혀 떨고, 그 값이 서스펜션으로 흘러
  차량이 뒤집힐 수 있는 경로가 열려 있었다(**뒤집힘 목격 0회 — 예방 수정**). 원인: Chaos 서스펜션은 `ECC_WorldDynamic` 채널로
  트레이스하고 응답은 `WheelTraceCollisionResponses` 를 쓰는데 **엔진 기본값이 "차량만 Ignore, 나머지 전부 Block"**
  (`ChaosWheeledVehicleMovementComponent.cpp:1142-1143`)이고 UGV 가 이를 안 덮었다 — 래그돌 시체는 `ECC_PhysicsBody` + `QueryAndPhysics`
  (`SoldierHealth.cpp:652-653`)라 **바퀴가 시체를 지면으로 읽었다**(그 바퀴만 지면이 수십 cm 위로 뛰어 스프링 힘이 튀고 마찰까지
  시체의 물리재질에서 온다). 수정 ① **차량 한 줄** — `UUGVWheeledVehicleMovementComponent` 생성자에
  `WheelTraceCollisionResponses.SetResponse(ECC_PhysicsBody, ECR_Ignore)`; **물리 접촉은 그대로 둬 밀려나는 그림은 유지**한다.
  수정 ② **사망 시 무기 드롭**(`USoldierHealthComponent`: `bDropWeaponOnDeath`/`DroppedWeaponMassKg 3.5`/`WeaponMeshComponentName`) —
  래그돌이 시작되는 **바로 그 순간** 떨구고, 떨군 것은 시체와 **같은 `ECC_PhysicsBody`** 로 두어 ①의 한 줄이 둘을 동시에 덮으며,
  응답은 전부 Ignore 에서 시작해 **WorldStatic/WorldDynamic/PhysicsBody/Vehicle 만 Block**(⚠ 바닥의 소총이 시야벽·엄폐물로
  계산되면 안 되므로 **Sight/Cover 제외**), `FreezeCorpse` 8 s 에 재우고 `EndPlay` 정리 목록에 드롭분 추가.
  ⚠ **함정: 화면에 보이는 총은 스폰된 액터가 아니라 `BP_SoldierCharacter` 자기 컴포넌트 `WeaponMesh`(`SK_KA74U_X`)**,
  `BP_AR4Rifle`(`SK_AR4_X`)은 숨은 총구/FX 용 — 첫 빌드에서 총이 손에 그대로 붙어 있던 이유이고 드롭 본체는 **컴포넌트 detach** 다
  (안 보이는 부착 액터는 안 떨군다). 손대지 않은 것: 차체↔래그돌 **물리 접촉 자체**(끼임·떨림) · `FreezeCorpse` 가 애님만 멈추고
  래그돌 바디는 계속 시뮬. 원칙 **P192**, 값 [C-171]~[C-173], 작업 [W118]~[W119]. **차량 쪽 변경도 이 문서에 있다**(`vehicle/ugv/` 에 별도 문서 없음).
- `ai/2026-09-23_vehicle_target_engagement_fix.md` (2026-09-23, 완료 — `L_SoldierTest` + **New_kadex_0811 PIE ✅**) — ★★ **차량 표적 교전**:
  New_kadex_0811 3차 전투지에서 적 3분대가 이동형지휘소 트럭(`BP_TitanTruck`)을 거의 안 쏘던 원인 둘. 확정은 `SoldierLab.Debug.Engagement.Log 1`
  의 `[Engage] … tgt=BP_TitanTruck … believed 1 worth 1 **aperture 0**` — 표적·확신·가치는 통과, **사격 자세만 0**. ① **주원인: 레인 트레이스가
  표적 자신을 벽으로 읽는다** — 차량은 소켓이 없어 조준점이 바운즈 50%(차체 한가운데)인데 `IsShotBlockedByWorld` 는 충돌점이 조준점에서
  `LaneToleranceCm 200` 밖이면 막힘으로 보고, 차량은 Pawn 이 아니라 `BodiesAreNotWalls()` 가 안 무시한다 → 트럭 실측 측면 133 cm(통과) ·
  **정면/후면 310~340 cm(막힘)** = 측면 축 **±48° 밖이 사각**, `PlanAperture` 의 일곱 자세가 **같은 레인 테스트 하나**라 동시 탈락 → `Blocked` →
  `bLaneDenied` → 재배치 반복(= "서 있다 이동"). 수정 = `IsShotBlockedByWorld/PlanAperture/FindAperture` 에 `TargetActor` 추가, **표적(또는 그 부착
  액터)을 맞히면 도달**, 캐시 키에도 표적 — 200 cm 규칙은 남아 **대인 불변**. ② **엄폐 층 위협 눈높이** — 차량 기록은 차체 한가운데(≈2 m),
  실제 포탑은 지붕(≈3.3 m+) → 1 m 낮은 눈으로 엄폐를 계산(+ fight probe 가 차체 콜리전 **내부**에서 출발). 수정 = `HasSocket(TargetSocket)` 이
  false 일 때만 눈 단차를 **형상**(바운즈 80% − 50%)에서 — 형상은 보면 아는 것이라 P130 위반 아님. 곁가지 `HasSocket()` public(비-`UFUNCTION`
  → Live Coding). ⚠ `BP_TitanTruck` 은 `BodyMesh` yaw 270° 라 **장축이 액터 X축** · 시험 레벨 재현은 `ScenarioConfig_0.bDemoAutoStartScenario=false`
  (자동 시작이 `EnemyInfiltrate` 의 HoldFire 를 걸어 `roe=hold`; 명령 없으면 기본 배정이 **ROE Free**) · **진영 판정은 원래 정상**이었다.
  원칙 **P189**, 값 **[C-166]~[C-168]**(차량 크기 `TargetRadiusCm` · 조준점을 포탑으로? · 엄폐 품질 수치).
- `animation/2026-09-23_low_ready_upper_body_layer.md` (2026-09-23, 완료 — 빌드·PIE ✅) — ★★ **로우레디(총 내림) 상체 레이어**:
  정지·걷기 총 내림 / 조깅 올림, 왼손 그립 IK 미사용. `SoldierCharacter_ABP` 7단 그래프(소스 6 → 진영 선택 → **로우레디 = Idle_ADS + LowReady
  MS additive** → **걷기 흔들림 = `Make Dynamic Additive`(정지 Hipfire, `AimedPose` 캐시)** → 팔·목에서 흔들림 제거 → 합성(× 0.6) → 최종
  `Layered blend per bone` → **`Slot 'UpperBody'` 의 Source**) + C++ 포즈 전용 램프. ★ **터진 것 다섯의 원인**: 걷기 2배속 = 포즈 **fan-out** ·
  빙의 시 안 보임 = 위의 가중치 1 슬롯이 덮음 · **오른손 튐 = `Curve Blend Option` 기본 `Override` + 가중치 정확히 0 이면 노드 통째 스킵**
  (`AnimNode_LayeredBoneBlend.cpp:249`)의 합 → additive 로 딸려온 `Enable_Warping` 이 0.6→0 에서 1 로 튐 → **`UseBasePose`** · 멈추면 2프레임 만에
  뚝 = 애님그래프 `Speed2D` 직결(CMC 감속이 2프레임) → **C++ 램프**, 그것도 **캐릭터 `WeaponLowered` 와 분리된 별도 값**(캐릭터 값은 몸통
  요 회전속도·조준 보정 게인을 계속 쓴다) · 총을 앞으로 내밈 = 흔들림 additive 에 든 **정적 팔 오프셋**. 커스터마이즈 표(`WeaponRaiseRate`
  2→**2.5** / `WeaponLowerRate` 8→**3** · cvar `SoldierLab.Pose.*` 5개 · Ease Exponent 2 · 흔들림 0.6 · 구간 20/150 · LowReady Explicit Time 2.5) ·
  블렌드 마스크 `BM_LowReady_Layer`/`BM_LowReady_Sway`(Branch Filter 는 본별 가중치 불가). 원칙 **P190~P191**, 값 **[C-169]~[C-170]**, 작업 **[W117]**.
- `animation/prototypes/2026-09-23_ally_crouch_ik_bones_removed.md` (2026-09-23, 완료) — ★ **아군 앉기 무릎/발 IK**: 아군만 자세를 낮출 때 무릎이
  안 굽고 몸이 떠다니던 원인은 애님이 아니라 **에셋** — 디자이너 재임포트 메시(#499) **LOD0 의 `Bones to Remove` 에 `ik_*` 본**(계층엔 있지만
  `NonRequiredBone` 이라 런타임 포즈에 없음 → 다리 IK 가 조용히 죽는다). ⚠ **FBX/블렌더 대조는 "본 동일" 로 오진했고 정답은 스켈레톤 트리
  아이콘(속이 빈 동그라미)이었다.** 이어진 발 꼬임은 `ALLY_MM_Rifle_Crouch_Idle` 한 장만 `ik_foot_*` 가 원점 → **`AM_Copy_IKFootRoot`** 적용.
- `ai/2026-09-17_situation_field_lighting_model.md` (2026-09-17 → **09-18 갱신**, 진행중 — 3단계·LOD 링·오버레이 배처
  PIE 확인, 09-18 후반부 빌드 대기) — ★★ **상황 필드** `USoldierSituationFieldSubsystem`: 진영별 위험 지도
  (`SoldierDangerMap`) 폐기, 위험도를 **저장하지 않고** 목격(점광원)의 그림자 + 안 본 땅(사전값 0.5)의 앰비언트로
  **파생**. 라이트 생애(목격→추적→얼림→해제, 0.5 s 연속창), 섀도우 48방향 × 2줄, 호라이즌 맵, `SoldierLab.Debug.Field`
  오버레이, Project Settings `USoldierFieldSettings`(43개 값 전부 [C]), 폐기한 접근(누적 버퍼·가상 관찰자). 원칙
  P143~P151. **09-18 16~18절**: LOD = **밉(레벨 0 집계) + 다중 앵커 퇴거**(왜 클립맵이 아닌가 — 계산은 레벨 크기와
  무관, 메모리 ∝ 지나간 면적), 오버레이 v2(자체 배처 flush+refill, 클립맵 링, 불투명도 = 신선도, 대칭 캡, 헤더
  2줄), 라이트 부정 증거(`ClearViewHalfLifeSeconds 4`, 삭제 아님). 원칙 P152~P157. **09-18 오후 20~23절**: 순찰
  `GetStaleVantage` · 부채꼴 스캔/주시 편향 · solid 셀 · 미지 = 열림(`GetExposureByStance` → bool) · 콘 스윕 띠 넓히기 ·
  [W83] 해결. **09-18 밤 24절**: `GetExposure` 미지 = `UnknownPresence × AmbientWeight`(22.2 끝 정정). **09-21 25절 포인터**: 필드가 **진영별 → 분대별**(`FScope`) · `GetWedgePresence` · 섀도우 재캐스트 문턱 → `ai/2026-09-21_…` (이 문서의 "진영별" 은 그 뒤로 "분대별"). ⚠ 1~15절 `.cpp` 줄 번호는 09-17 판, 16~18절은 09-18 오전 판, 20~23절은 저녁 판, 24절은 밤 판(2501줄), 09-21 판은 3036줄.
- `ai/2026-09-17_infiltration_and_unknown_ground.md` (2026-09-17 → 09-18 갱신, 진행중) — ★ **잠입** "안 본 곳은 적이
  있다고 친다": 노출이 믿는 적에 대해서만 정의돼 눈 0 = 노출 0이던 진단, `UnknownPresence 0.5`, 시야 콘 스윕,
  스프린트 규칙(접촉∥제압∥사선 거부 · `Cautious`/`Rush`), 볼 곳 루프(스캔 패턴 없음), 골든앵글 후보 회전,
  필드 후보/자세(09-18 PIE 확인). 09-18 추가: 볼 곳 화살표(오버레이) · 훑은 선의 라이트 부정 증거. **09-18 오후 13절**:
  섹터 = 부채꼴 정정(6절) · CQB 볼 곳 편향·도착 머무름 · `MinStance`. 분대 층에 남은 것(경로·대형·DT의 Cautious/섹터 발행).
- `ai/2026-09-18_patrol_scan_and_move_robustness.md` (2026-09-18 오후~**밤**, 진행중 — 오후 묶음 PIE "이제 정상적이다",
  ~~저녁 A/B/C 빌드 전, ScanTurn 연동 미확인~~ → 밤 A/B/C PIE "잘됨", ScanTurn 포즈 세션 확인) — ★★★ **순찰 · 부채꼴 스캔 · 이동 강건성 · 얇은 엄폐 · (밤 12~17절) 코너 멈춤 루프 · 긴장도/걸음 · 포즈 급박도**: "적이 죽은 뒤 가만히
  서 있는 아군"의 원인 넷(존 섹터를 고정 방위로 읽음 → **부채꼴** · 콘 스윕 광선 사이 셀 → 띠 넓히기 · 볼 곳의 눈 0 게이트
  틈 → 항상 계산 · 몸이 안 돎 → `GetAimPoint()/IsScanning()` 계약 + 포즈 세션 `SoldierScanTurnComponent`) · **순찰 = 경로가
  아니라 낡은 조망 비용**(`GetStaleVantage`, 존/목표 `PatrolWeight 1.0`) · 로그로 잡은 이동 결함 셋(큐브 꼭대기 셀 = 굽기가
  기하 안에서 시작 → `bSolid` + `FindPathSync` 부분 경로 불허 + 거부 30 s + 유예 0.75 s / 벽 꼭대기 = 미지를 노출 0으로 →
  `GetExposureByStance` bool, 미지 = 열림 / 밴드 밖 평평한 Hold → 기울기 계속) · CQB "눈이 발을 이끈다"(`WatchTravelBias`,
  `ScanDwellSeconds`) · `MinStance`(titan DT 미연결) · [W83] 해결 · 저녁 **A** 활동도 가중 은폐(`HiddenGazeFraction 0.5` —
  나무는 한 방위만 가린다) **B** 미세 위치(30 cm 후보 + 수용 반경 20 cm, 병사별 복셀 기각) **C** 코너 멈춤 + 굽이 너머 미리
  보기(진짜 파이 자르기 아님). **밤 4차 12~17절**: 첫 빌드 로그의 **코너 멈춤 루프**(굽이 기억이 경로 인덱스 → **자리**로, 재개된
  이동 = 방금 발행한 이동, 눈 나타나면 즉시 재개) · [W89]/[W90] 코드로 해결 · `GetExposure` 미지 ≠ 0 · **긴장도 `GetTension()` +
  걸음 `GetDesiredGait()`**(조용한 경비는 걷는다) · **포즈 급박도 `GetPoseUrgency()`**(AI는 목표 + 숫자 하나, 움직임은 포즈 층) ·
  1프레임 점프는 AI 층 아님(포즈 세션 P167) · 잠입 접근 현황. 원칙 P158~P166 · **P172~P175**, 값 [C-148]~[C-153] · **[C-155]~[C-156]**,
  작업 [W85]~[W88]. 계약 쪽은 `squad/2026-09-17_command_layer_design.md` 10절, 필드 쪽은 시스템 문서 20~24절.
  ~~ScanTurn 연동 미확인~~ → 포즈 세션이 09-18 PIE 확인(아래). **09-21 18절 정정**: 저녁 C 코너 멈춤은 **삭제**, 엣지 전진으로 대체([W85] 해결) →
  `ai/2026-09-21_…`.
- `ai/2026-09-21_per_squad_field_edge_advance_fire_model.md` (2026-09-18 밤 ~ 09-21, 진행중 — ①③ PIE ✅ · ② 빌드됨/PIE 대기 · ④ 빌드 전) —
  ★★★ **분대 스코프 상황 필드 · 엣지 전진 · AI 가 소유하는 사격 콘 · 섀도우 수요 감축**: ① 필드 = 분대의 지식(`FScope` = 진영 × 분대,
  `MaxSquadsPerFaction 3`, 공개 API 전부 `Who`, 무전 → 받는 분대 필드에 관측 시각으로, `bTakesSquadOrders=false` 는 병사 아님 — UGV 조종 중
  오버레이가 UGV 를 따라가던 원인, cvar `Debug.Field.Squad`/`.Centre`, `L_SoldierScenario` 아군 4 → 3분대) ② 09-18 코너 멈춤 **삭제** → 콘 스윕
  광선의 엣지(`GetSweepRays`) + 필드 쐐기 적분(`GetWedgePresence`, 트레이스 0)으로 걸음이 여는 경계도 × m² 를 예산(`StepPresenceBudget 6`)과
  비교해 걸음을 고르고 연 조각이 눈에 익으면 다음 — 호·타이머 없음, **설계 논의 + 기각 둘**(손 그린 호+머무름 · 후보별 광선 부채꼴) ③ 콘
  `GetShotSpreadDegrees()` = 0.8° × 이동 × 자세 × 반동 + 흔들림(선회 2.5°→0.4 s, 발마다 +0.6°, 이동 바닥 1.5°), 새 의도 `Settling`/`Pacing`,
  버스트 2~5 · 0.5 s × 지터, `[Engage]` 꼬리 `cone wobble burst next` — **무기 BP·`WantsToAim` 배선 대기 [W93]** ④ 섀도우 재캐스트 한 셀 ∧ 0.5 s +
  riders + 비용 줄(실측 96 alive = `MaxLights` 상한 [W92]). 원칙 **P176~P180**, 값 [C-157]~[C-161], 작업 [W92]~[W96], **[W74]·[W85] 해결**.
  9절 = 브리핑과 코드가 어긋난 자리. 시스템 문서 25절·09-18 거동 문서 18절이 여기로 포인터. **같은 날 늦게 10~12절**: ④ 정식 빌드·PIE ✅
  (**[W96] 해결**, 재측정 0.03 ms · 0 waiting · 96 alive 동일 — 96 은 alive) · 3분대 재편 저장 확인 · 오버레이 노출 보정 포인터(아래).
- `ai/2026-09-21_debug_overlay_exposure.md` (2026-09-21 늦게, 완료 — 빌드·PIE ✅) — ★★ **디버그 오버레이 노출 보정**: 노출 EV10 고정 레벨에서
  모든 AI 오버레이(와 엔진 내비메시 `P` 뷰)가 숯검정 — 디버그 프리미티브는 톤매퍼 **앞**, `DrawDebug*`·배처 `DrawMesh` 는 8-bit `FColor`(선형 1.0
  상한). 기각 넷(레벨 라이팅/PP · 상수 배율 · 색만 키우기 · 엔진 `P` 뷰 수정 — 비목표) → 채택: **뷰가 지난 프레임에 적용한 노출의 역수만큼 밝게**.
  `SoldierDebug::GetExposureScale`(`FSceneViewExtensionBase` + `GetLastEyeAdaptationExposure`) · `Bright(FColor) → FLinearColor`(알파 제외) · 래퍼
  `Line/Point/Sphere/Circle` · 신규 `USoldierDebugMeshComponent`(`AI/SoldierDebugMesh` — 배처 메시 경로를 선형 색으로, 필드 사각형·링) · cvar
  `SoldierLab.Debug.ExposureScale`(0 자동) · `Build.cs` + `RenderCore`/`RHI`. `AI/` 24곳 교체 표 · 안 바꾼 `Squad/` 9 · `Pose/` 7 · `Weapons/` 1
  → **[W97]**(`Arrow` 래퍼 없음). 함정: 블룸 · 한 프레임 지연 · C4456 · 익스텐션 수명. 원칙 **P181**. 새 [C] 없음.
- `ai/2026-09-21_perf_instrumentation_and_cover_cost.md` (2026-09-21, 완료 — 빌드·PIE 실측, 이 층 몫은 끝, 나머지는 인계) — ★★ **성능 계측 + 엄폐 틱 비용**:
  35명에서 World Tick 1.7 → 29 ms 인데 `stat game` 은 6 ms 만 이름을 댐 → **`stat SoldierLab`**(`STATGROUP_SoldierLab`, `AI/SoldierLabLog.h` — 시스템 틱 9 + Cover 하위 7 + Field 5
  사이클 카운터, `Traces: Sight/Cover/Engagement/Field` · `Soldiers Ticked`) + **off 스위치 `SoldierLab.<System>.Enabled` 7** + `SoldierLab.Cover.Avoidance`(RVO 런타임 A/B). 첫 계측
  **Cover Tick 8.92 ms · 487 트레이스/프레임 = 프레임의 1/3** → 결정 보존 수정 넷(이동 중 스윕 정지 `bSweepSuspended` + `UpdateWatchPoint()` 0.25 s · 발밑 `HereEvalIntervalSeconds 0.1` ·
  경로 가지치기(정확) · 눈 0 `CalmCandidatesPerTick 2`) → **1.92 ms · 243**, World Tick **28.5 → 22.6**(GPU 6 — 게임 스레드 바운드). 전/후 표 · RVO A/B(0.3~0.5 ms, 켜 둠)와 A/B 를 오염시킨
  **`DispatchBlockingHit` 58 ms 히치**(무기 `OnHit` 동기 로드 의심) · **소유자별 인계 표**(포즈 ABP 스레드-세이프 3.5/키네마틱 본 1.0/URO · 캐릭터 BP 컴포넌트 23/병사·오버랩·틱 2.5 · 투사체 풀링
  0.65 · 이 층 다음 몫 ≈ 1 · 분대 스코프 두 줄 — [W98]~[W103]) · **측정 규약**(로깅 off · Standalone · 세 갈래 확인 · 평균+max · 워커 ≠ 게임 스레드). 원칙 **P182~P183**, 값 [C-162].
  ⚠ 숫자는 PIE + 로깅 on — 절대값보다 차이를 믿을 것. 별도 성능 폴더는 없음(소유자가 전부 SoldierLab 안). **⚠ 5.1·7절의 읽기 셋은 10절 정정**(ABP 2개 아님 · 키네마틱 본 스킵 불가 · 발당 1.1 ms).
- `ai/2026-09-21_game_thread_structural_pool_rays_bridge.md` (2026-09-21 17:10~18:10, 완료 — 사용자 PIE 확인) — ★★ **게임 스레드 구조 묶음(후편) = 아래 문서 13절 순서 ①~⑤ 의 실행 기록**, World Tick
  **18.3/20.3 → 11.3/14.0**(오늘 누적 22.6 → 11~14, "교전 중 50fps 후반"): ① **투사체 풀** 신규 `Weapons/SoldierProjectilePool` `USoldierProjectilePoolSubsystem::Acquire`(클래스별 풀 · 주차 재사용 · 상한 cvar
  `SoldierLab.Projectile.PoolMax 96` · 넘으면 RCWS 식 라운드로빈) — BP `Shoot`/`PlayShotCosmetics`·도탄 전부 풀, `CollisionComponent` QueryOnly, 빈 EventTick 삭제, `MaxFlightDistanceCm 60000` → 스폰 0.83/발 → 0,
  투사체 47 에서 정지 ② **레이** 신규 `AI/SoldierQuery.h` `BodiesAreNotWalls()`(Pawn 채널 응답 Ignore — 35명 `AddIgnoredActor` 루프를 Engagement·Cover·Field 8 사이트에서 제거) + `IsShotBlockedByWorld` 8슬롯 캐시
  (`LaneCacheMoveCm 15`/`LaneCacheSeconds 0.15`) → 씬 쿼리 1,030~1,120 → 357/576회 ③ CMC `bAlwaysCheckFloor`/`bEnablePhysicsInteraction` false + ★ **병사 메시 QueryAndPhysics → QueryOnly(정정 — 문서가 QueryOnly 라 적었으나 아니었다)**
  ⑤ Cover `CandidateIntervalSeconds 0.033` · Sight `ScanIntervalSeconds 0.033`(프레임 → 시간) ④ **캐릭터 BP EventTick 본문 → C++ `Pose/SoldierAIBridgeComponent::TickBridge`**(노드 순서·Kismet 산술 그대로, 변수 47개는
  BP 소유, cvar `SoldierLab.AIBridge.Native` A/B) → 아군 20 ReceiveTick 1.2 → 0.22. 남은 것 F절(Cover 점수 · CMC · 메시 틱 · `AC_PreCMCTick` 0.8 → **[W115]** · 틱 오버헤드 → [W112] · [W113]) · 별건 `AN_Reload` 노티파이 에러 로그 → **[W114]**.
  작업 **[W102]·[W108]~[W111] 해결**, 새 C·P 없음.
- `ai/2026-09-21_game_thread_batch_cameras_abp_muzzle.md` (2026-09-21, 완료 — 카메라·ABP·총구 완료, URO 폐기·되돌림, C++ 4건 빌드·실측, **13절 순서 ①~⑤ 는 위 후편에서 실행(14절)**) — ★ **게임 스레드 묶음 = 위 인계표 [W98]~[W103] 의 실행 기록**:
  계측법 **`stat dumpframe -ms=0.05` → 로그 `LogStats:` 블록 파싱**(틱 함수별 ms, MCP 콘솔 툴 없음 — `stat SoldierLab` 의 짝) · 전편 오독 셋 정정 · **AI 병사 전원의 GASP GameplayCamera+SpringArm 2.0 ms** →
  BeginPlay `not IsPlayerControlled` 면 틱 off(Deactivate 아님 P107), World Tick **22.6 → 19.4/18.4** · ABP cvar 폴링 1 Hz(`CVarPollSeconds`) · 숨은 총 메시 `OnlyTickPoseWhenRendered` · `AnimBP` 캐시 ·
  총구 상주 `MuzzleFlashFX`(`SpawnSystemAttached` 누수 8정 106개 해결) · ⛔ **URO 는 2분 만에 포즈 NaN 크래시 → 폐기, `a.URO.Enable=0` 안전장치(P186, 되돌리기 [W107], 원인 [C-165])** · C++ 4건(Health/Comms
  `TickInterval` · HeadAim 조기 반환 · Squad/Zone 스코프 [W103] ✅ · `WindSource` 20 Hz) 빌드 대기 · 남은 ms 표와 현실적 바닥 ≈ 13~14. 작업 [W107]~[W109]. ⚠ ID 는 분대 세션이 W104~W106·C-163·P184 를 선점해 비킨 것.
- `animation/2026-09-18_ai_pose_layer_scanturn_gait_smoother.md` (2026-09-17~18, 완료 — 사용자 PIE 확인, 병행 포즈 세션) —
  ★★ **AI 포즈 층 3종 = AI 세션 계약의 소비 측** (`Source/SoldierLab/Pose/`, 전부 AI 전용): `SoldierScanTurnComponent`(총 내림 ∧
  정지 ∧ 스캔/접촉이면 캡슐 yaw 를 `GetAimPoint()` 로, 20°/5° 히스테리시스, 180°/s, 메시는 GASP TIP) · `SoldierGaitBridgeComponent`
  (`GetDesiredGait()==Walk` → GASP `CharacterInputState.WantsToWalk` 리플렉션, BP 구조체 필드 GUID 접미사) ·
  `SoldierPoseSmootherComponent`(stance/lean/BF 축 사다리꼴 프로파일 × `GetPoseUrgency()`, 앉는 쪽이 빠름, BP 상태+목표 동시
  쓰기). ★ 함정: **`RampAxisTo` 는 rate ≤ 0 이면 Target 반환** — BP 램프를 0 으로 얼리자 급할 때 1프레임 스냅, 0.0001 로 해결.
  관전 폰 추기(휠 = 비행 속도 · slomo 무관 · 롤 잔류 수정)와 **H 는 AI 에 자동 활성화 없음** 확정. 원칙 P167~P171, 값 [C-154],
  작업 [W91]. cvar `SoldierLab.Debug.ScanTurn` · `.PoseSmooth`.
- `ai/2026-09-17_threat_bonus_lane_denied_vehicle_heights.md` (2026-09-17, 완료) — 오전 교전 수정 넷: 나를 쏘는
  놈 위협 보너스 · 사선 거부 → 재배치("15 s 전이 0" 해결) · 가치 게이트 히스테리시스(30 Hz 방아쇠 떨림) ·
  차량 표적 높이 = 바운즈 비율(UGV 바퀴 겨눔 해결).
- `ai/2026-09-14_danger_map_and_position_commitment.md` (2026-09-14, **위험 지도 부분 superseded 2026-09-17**) —
  나머지(모든 눈·그림자 스냅·머무름·표적 잠금)는 유효.
- `squad/2026-09-17_command_layer_design.md` (2026-09-17, **09-18 9절 정정** — 빌드·첫 PIE 뒤, **09-18 오후 10절 계약 추가**:
  `PatrolWeight`/`PatrolStaleSeconds`/`MinStance` · 섹터 = 부채꼴 · `GetAimPoint()/IsScanning()` 계약과 `SoldierScanTurnComponent`) —
  L0/L1 분대 명령 층. titan 시나리오 연결은 `level_new_kadex_0811/2026-09-17_soldierlab_squad_scenario_link.md`.
- `squad/2026-09-18_squad_layer_fixes_quota_engage_range.md` (2026-09-18, 진행중 — 코드·빌드 완료, 완주 판정 대기) — ★ 첫 PIE
  수정 6건: **도착선 일원화**(`ASoldierZone::ArrivalFraction` → 배정, Approach 비용 계단 `1 + 거리/scale`, `ArrivalInsetFraction`
  삭제) · 존 **에디터 표시**(구 3개 + 섹터 화살표 + 스프라이트) · `bTakesSquadOrders`(차량이 `(none)` 분대원으로 잡히던 것) ·
  **정원제·대타 `ReinforceSquads` + DT `Quota`**(09-03 구 기능의 SoldierLab 판) · **`EngageRangeCm`**(적군이 스폰에서 92 m 밖
  UGV 를 쏴 접근 단계가 사라진 것) · 트리거 2 · **RCWS 청각 ← SoldierLab 총성**(`OnGunshot` → 브리지 → `ReportGunfire`). 8절 =
  병사 사격 결정 게이트 목록(believed → ROE → EngageRange → worth ≈95 m → 회계 → aperture → onTarget) · `MinStance` 제안 [W84]. **09-21 정정 배너**(MinStance 는 SoldierLab 쪽 구현됨 · `BreakContact` · 제외 두 소비자 · [W70] 해결).
- `squad/2026-09-21_break_contact_and_targeting_exclusion.md` (2026-09-21, 진행중 — 코드·빌드 완료, New_kadex 2차 PIE 판정 [C-163] 대기) — ★★ 새 규칙 동사
  **`BreakContact`**(존 유지 · `IsBreakingContact()` 이면 엄폐 층이 Fighting/Route/Danger/Suppression 을 0 으로 · dwell 없음 · 스프린트 · 자세 0 — 도주는
  ROE 가 아니라 **땅의 가격**, P184) 와 `Withdraw`/`HoldFire` 의 차이 표 · **표적 제외 플래그의 두 소비자**(브리지 → 탐지 컴포넌트 → RCWS, 단
  `bRespectEnemyTargetingExclusion` 스위치를 이제 `SetTargetable(false)` 가 켬 / SoldierLab 보병 `IsContactExcluded` — 후보 제외·잠금 해제, **인지 불변** P185) ·
  1v1 `L_SoldierTest` DT 5행 기록 · C2248 빌드 함정 · ⚠ 문서 세션 발견 **RCWS 스티키 표적은 제외를 안 본다**([W106]). file:line 전부.
- `animation/prototypes/2026-09-18_tick_cycle_warning_charmovecomp.md` (2026-09-18, 부분 — 서명 시험 대기 [C-146]) — `LogTick
  … CharMoveComp … would form a cycle` 초당 600줄의 원인: 09-12 의 `AddTickPrerequisiteActor(self)` 가 엔진 `bTickBeforeOwner`
  엣지(`MovementComponent.cpp:186-188`)와 순환해 매 프레임 버려짐(`TickTaskManager.cpp:2658`) → 09-12 5.3절의 "보장 순서"는 보장이
  아니었다. 수정 CMC `Tick Before Owner=false`. 덤: 총구 `DrawDebugCoordinateSystem` exec 해제.
- `migration/2026-09-14_titan_example_migration.md` (2026-09-14~17) — SoldierLab → `titan_example` 편입 기록, §11
  게임플레이 태그 ini 누락(2026-09-17).

## `documents/` (.md만)

- `response_0828.md` (2026-08-28) — LIG 1차 답변 원문. `protocol/lig_questions_0816.md`에
  반영 완료.
- `문의내용0806.md` (2026-08-06) — 최초 LIG 문의 원문.
- (비-md 원본: PDF/PPTX/xlsx — 인덱스 대상 아님.)

## `_archive/` — 보관된 문서

각 파일 상단에 `[보관됨]` 노트로 사유/최신 문서 포인터 있음. 목록만:
`M1A2_UGV_Conversion.md`(BP_UGVFromTank는 채택 안 된 병행 시도, 엔지니어링 지식만 보관),
`tanksim_tank_analysis.md`, `aim.md`, `ally_and_scenario_system_plan.md`,
`ally_character_animation_design.md`, `chaos.md`, `charts.md`, `minimap.md`,
`mission_dashboard_widget_guide.md`, `scenario_datatable_system_plan.md`,
`scenario_implementation_status.md`, `status_hud_dev_guide.md`, `titan_quadcam_plan.md`,
`ui.md`, `시나리오.md`, `path/path.md`, `path/ugv_driving_dev_guide.md`,
`path/ugv_navmesh_autonomous_driving_dev_guide.md`(이 3개는 `_archive/path/` 하위) — 이상은
2026-08-31 이전 1차 정리분.

**2026-08-31 2차 정리(구조 재편)로 추가 보관**: `structure_README_track_index.md`(옛
`structure/README.md` 트랙 인덱스, `CURRENT_STATE.md`/`DOCS_INDEX.md`로 대체됨),
`sessions_idea.md`/`restructure_status_0813.md`/`idea.md`/`idea_review.md`(세션 분할·구조
개편 과정 기록, `WORKLOG.md`에 이미 흡수됨), `udp_test_findings.md`(LIG 참조구현 udp_test
분석, 정식 ICD로 대부분 대체됨), `lig_response_0806_review.md`(초기 LIG 답변 분석,
`protocol/`로 대체됨), `nats_infra_setup.md`(NATS 인프라, 폐기 결정됨).

---

## 파일명에 날짜 없음 (리네임 후보, 실행 안 함)

`guide/`, 대부분의 root 파일, `protocol/`·`rtsp/`·`replication/`·`infra_architecture/`의
핵심 문서 다수가 아직 `YYYY-MM-DD_` 접두 규칙 이전에 만들어짐 — 상호 참조 깨짐 위험 때문에
일괄 리네임은 안 함. 새로 만드는 문서부터 규칙 적용(`CLAUDE.md`).
