# 현재 상태 — soldier_ai_lab

2026-09-30 / **★★ 09-30 — AI 조준(총구) 떨림 해결(녹화 8회 후 사용자 판정): 엔진이 AI 컨트롤 피치를 0 으로 두던 것 → `ASoldierAIController` · 선회 오차 비례 40~150°/s · 조준 보정 재설계(총이 조준에 있을 때만 빠르게, 밖에선 누설) · 자세 축 리밋 사이클 + 웅크림 슈미트 트리거 · 서기↔앉기 AO 관성 전환([W11] 해결) · 뛰면 조준 해제 · 몽타주 슬롯 그룹 분리 + 사격 금지 = 재장전 몽타주 길이. 도구 = 프레임 단위 조준 녹화기 `SoldierLab.Debug.AimTrace`. 원칙 P195~P199, 값 [C-176] · [C-178], 작업 [W127]~[W129]** · **★ 09-30 조준 대각선 이동 발 끌림 — Loops DB 가 4방향뿐이라 45° 에서 Stop 이 이긴다(원인 확정), 편향 임시 완화만 적용, 근본 수정(대각선 클립) 대기 [W124] · 맹목사격 임시 off(`SoldierLab.Engagement.BlindFire 0`, [Q52]) · P200** · **★ 09-29 왼손 그립 IK 재가동(총 소켓 위치+회전) — 09-30 소켓 회전 캡처 완료, PIE [C-175], P194** · **★★ 09-29 — 병사 이동 속도가 표 한 장(`DT_SoldierMovement`, 행 16 × 열 2)으로 조절된다: 걸음걸이 **상한** + 속도 배율 **0.75~1.25**. 그 밴드는 임의값이 아니라 ABP `Get_DynamicPlayRate` 가 커브 없는 클립에 쓰는 대체값이고, 클립 authored 속도 실측(Walk 291.31 · Jog 582.62 = 캐릭터 설정값과 동일)에서 안전 구간 Walk 218~364 · Jog 437~728 이 나온다. PIE 실측 ✅ — 원칙 P193, 값 [C-174], 작업 [W120]~[W122]. ⚠ 09-28 1차 구현(배율 3열 · 안전범위 0.45~1.2 · CMC `MaxWalkSpeed` 목표)은 **폐기·정정**: `AC_PreCMCTick` 이 CMC 직전에 매 프레임 덮어써서 전혀 안 먹었고, 입력으로 옮긴 뒤엔 배율이 밴드 밖이라 발이 미끄러졌다. 가속·회전 열은 같은 이유로 죽은 값이라 삭제** · **★ 09-23 저녁 — 차량이 시체를 밟을 때 서스펜션이 시체를 지면으로 읽던 경로를 끊고(UGV 컴포넌트 한 줄, 물리 접촉은 유지해 밀려나는 그림은 남김), 사망 시 총을 손에서 떨구게 함(컴포넌트 `WeaponMesh` 가 본체 — 액터가 아니다). 코드 완료·재빌드 실측 대기, 원칙 P192, 값 [C-171]~[C-173], 작업 [W118]~[W119]** · **★★ 09-23 차량(트럭) 상대 교전이 뚫렸다 — `aperture 0` 의 원인은 ① 사격 레인 트레이스가 표적 자신(차량)을 벽으로 읽던 것(표적 액터를 레인 판정까지 넘겨 "표적을 맞혔으면 도달", 대인 불변) ② 엄폐 층이 차량 위협의 눈을 차체 한가운데에 두던 것(소켓 없는 표적만 형상에서 눈높이 단차) — `L_SoldierTest` + New_kadex_0811 PIE ✅, 원칙 P189, 값 [C-166]~[C-168]** · **★★ 09-23 로우레디(총 내림) 상체 레이어 — 정지·걷기 내림 / 조깅 올림, `SoldierCharacter_ABP` 7단 그래프 + 포즈 전용 C++ 램프(캐릭터 `WeaponLowered` 와 분리) + 블렌드 마스크 2개, PIE ✅; 엔진 함정 둘 확정(가중치 0 = `Layered blend per bone` 노드 스킵 · `Curve Blend Option` 기본 Override 가 로코모션 커브를 덮어 워핑이 튐) — 원칙 P190~P191, 값 [C-169]~[C-170], 작업 [W117]** · **★ 09-23 아군 앉기 무릎/발 IK — 원인은 애님이 아니라 에셋(재임포트 메시 #499 LOD0 `Bones to Remove` 에 `ik_*` · `ALLY_MM_Rifle_Crouch_Idle` 의 `ik_foot_*` 원점), 해결 완료** · **★ 09-23 `USoldierHealthComponent::EndPlay` 가 병사가 스폰해 들고 있던 액터(소총 `BP_AR4Rifle`)를 같이 파괴한다 — `bDestroyCarriedActorsOnDestroy`(기본 켬), New_kadex_0811 2-PC 실기 ✅, [W116] 해결 · 원칙 P188(스폰한 쪽이 치운다 — 엔진은 소유 액터를 안 따라 지움)** · **★ 09-22 titan 시나리오 재시작이 이 모듈의 리셋 계약(`ResetForRestart` ×2 · `RecallAll`/`Park`, 콘솔 `SoldierLab.ResetWorld`)을 호출한다 — L_SoldierScenario PIE ✅, 원칙 P187(서브시스템 새 상태는 리셋에도)** · (이하 09-21) **★★ 상황 필드가 분대 하나당 하나가 됐다(PIE ✅) · 09-18 코너 멈춤을 엣지 전진(파이 자르기의 창발 — 호·타이머 없음, 트레이스 0)으로 대체(빌드됨 CL 500, PIE 대기) · 사격 콘을 AI 가 소유(정착 0.8° + 흔들림, `Settling`/`Pacing`, 버스트 2~5, PIE ✅ — 무기 BP·`WantsToAim` 배선 대기) · 섀도우 재캐스트 문턱 + riders + 비용 줄(~~빌드 전~~ → **같은 날 빌드·PIE ✅ "아주 잘됨"**, 실측 96 alive = `MaxLights` 상한) · ★ **오버레이 노출 보정** — EV10 레벨에서 숯검정이던 모든 오버레이를 레벨 무변경으로 **뷰 노출의 역수만큼 밝게**(`SoldierDebug::Bright` · 래퍼 `SoldierDebug::*` · 신규 `USoldierDebugMeshComponent` · cvar `SoldierLab.Debug.ExposureScale`, PIE ✅) · ★ **성능 계측** — 35명에서 World Tick 29 ms 의 원인을 `stat SoldierLab` + off 스위치로 이름 붙이고(**Cover 8.92 ms = 프레임의 1/3**) 결정 보존 수정 넷으로 **1.92 ms**, World Tick **28.5 → 22.6**; 남은 것은 애니 게임 스레드 7.3 · 이동/트랜스폼 4~7 · 캐릭터 BP 2.5 · 투사체 0.65 → **다른 층으로 인계 [W98]~[W101]** · ★ **게임 스레드 묶음(후속)** — `stat dumpframe` 로그 파싱으로 틱 함수별 이름을 붙이자 **AI 병사 전원의 GASP 카메라 2.0 ms** 가 나와 BeginPlay 틱 off → World Tick **22.6 → 19.4/18.4**; ABP cvar 폴링 1 Hz · 숨은 총 메시 `OnlyTickPoseWhenRendered` · 총구 상주 Niagara(누수 해결) 완료, 전편 오독 셋 정정(ABP 2개 아님 · 키네마틱 본 스킵 불가 · 숨는 건 총 액터 메시), ⛔ **URO 는 2분 만에 포즈 NaN 크래시 → 폐기(P186, ~~되돌리기 [W107]~~ 재기동 뒤 되돌림 완료)**, ~~C++ 4건 빌드 대기~~ → **빌드·실측 완료(World Tick 18.3/20.3)**, 재기동 후 실측에서 **투사체가 풀 설계인데 발마다 스폰돼 영구 누적** 발견 → 풀 [W109] 착수, 남은 후보 순서 ①~⑥([W110]~[W113] 신설) · ★★ **게임 스레드 구조 묶음(후편, 17:10~18:10) — 순서 ①~⑤ 전부 실행·PIE ✅**: ① 투사체 풀 `USoldierProjectilePoolSubsystem`(스폰 0.83/발 → 0, 투사체 47 에서 정지, QueryOnly, `MaxFlightDistanceCm 600 m`) · ② 레이 — 병사 무시를 **Pawn 채널 응답**(`SoldierQuery::BodiesAreNotWalls()`) + `IsShotBlockedByWorld` 캐시(씬 쿼리 1,030~1,120 → 357/576회) · ③ CMC `bAlwaysCheckFloor`/`bEnablePhysicsInteraction` false + **병사 메시 QueryAndPhysics → QueryOnly(정정 — 문서가 QueryOnly 라 적었지만 아니었다)** · ⑤ Cover 후보·Sight 스캔 박자 0.033 s 시간 고정 · ④ **캐릭터 BP EventTick 본문 → C++ `USoldierAIBridgeComponent::TickBridge`**(아군 20 ReceiveTick 1.2 → 0.22) → World Tick **18.3/20.3 → 11.3/14.0**(오늘 누적 22.6 → 11~14, 사용자 "교전 중 50fps 후반"); **[W102]·[W108]~[W111] 해결**, [W112]·[W113] 남음, 새 [W114](`AN_Reload` 노티파이 에러 로그) · [W115](`AC_PreCMCTick` 0.8) · ★★ **분대 세션(병행): New_kadex_0811 본 레벨이 SoldierLab 병사로 이관돼 3단계 시나리오 전 체인이 첫 PIE 에서 순서대로 완주("아주 잘됨") — 3분대 3차 "도주"가 엄폐 홉 후퇴였고 UGV 가 제외된 분대를 계속 쏘던 문제 → 동사 `BreakContact` + 표적 제외 2소비자(RCWS 스위치 · `IsContactExcluded`), 2차 PIE 대기 [C-163]**** / 09-18 밤 이후 AI 세션 묶음 넷 + 같은 날 늦게 둘 + 성능 → `ai/2026-09-21_per_squad_field_edge_advance_fire_model.md`(10~12절) · **`ai/2026-09-21_debug_overlay_exposure.md`** · **`ai/2026-09-21_perf_instrumentation_and_cover_cost.md`** · **`ai/2026-09-21_game_thread_batch_cameras_abp_muzzle.md`** · **`ai/2026-09-21_game_thread_structural_pool_rays_bridge.md`**(후편) · 분대 → **`../level_new_kadex_0811/2026-09-21_soldierlab_migration_new_kadex_0811.md`** · **`squad/2026-09-21_break_contact_and_targeting_exclusion.md`**. 원칙 **P176~P186**, 값 [C-157]~[C-165], 작업 [W92]~[W115], **[W70]·[W74]·[W85]·[W96]·[W100]·[W102]·[W103]·[W107]·[W108]·[W109]·[W110]·[W111] 해결**.

★★ **2026-09-30 — AI 조준 떨림 해결 (원칙 P195~P199, 값 [C-176] · [C-178], 작업 [W127]~[W129], [W11] · [C-93] 해결) — 녹화 8회 후 사용자 "해결" 판정** → `ai/2026-09-30_ai_real_pitch_and_aim_smoothing.md`(14절 = 최종 요약).
① **증상 [A · 사용자]** — 표적 전환 시 조준이 너무 빨리 돎 · 조준선 색 바뀔 때 틱틱 · **AI 가 앉기/서기 임계값을 지날 때 총구가 1프레임에 튐** · 엄폐 뒤에서 자세를 낮출 때 프레임 단위 도리도리.
② **측정 도구** — `Debug/SoldierAimTrace` `USoldierAimTraceSubsystem`(`SoldierLab.Debug.AimTrace 1`/`.Filter`) → `Saved/AimTrace/<시각>/<병사>.csv`, 교전·컨트롤러·몸·MM 블렌드 스택·본·총구·ABP/BP 변수 전부. 첫 녹화에서 컨트롤 회전은 매끄럽고 총열만 ±10°/프레임 → 애니메이션 쪽 발원(P199). `Build.cs` + `PoseSearch` · `BlendStack` · `AnimGraphRuntime`.
③ **원인 → 수정** — (a) `AAIController` 는 초점이 폰이 아니면 **컨트롤 피치 0** → 총 위아래를 보정기가 떠맡음 → 신규 `AI/SoldierAIController`(컨트롤 회전 = 교전 `GetAimRotation()`), `AIC_Soldier` 부모 변경(옛 부모 `AIController`), `SoldierLab.Aim.RealPitch`(P198) (b) 선회 240°/s 등속 → `SlewAim` 3° 이하 40 · 30° 이상 150°/s([C-93] 해결) (c) 목표 자세 0.5 = 임계값 0.5 에서 사다리꼴이 리밋 사이클 → 웅크림 매 프레임 토글(49~121회/분) → `StepAxis` 착지 + `UpdateStance` 슈미트 트리거 `SoldierLab.Pose.CrouchHysteresis 0.05`(P195) (d) 조준 보정이 재장전 중에도 적분해 ±25° 에 들러붙고 추종이 느림 → 총이 조준에 있을 때만 10/s 적분 · 밖에선 6/s 누설 · 60°/s 상한(P196) (e) 서기↔앉기 AO 가 `Select` 로 즉시 교체 → `USoldierAnimLibrary::UpdateStanceAimOffset` 관성 0.25 s, ABP `OnUpdate_StanceAimOffset`([W11] 해결) (f) 뛰기 클립 위 AO 가 총을 70~80° 비켜 듦 → AI 는 400 cm/s 위에서 조준 해제(340 재개, 최소 유지 0.8 s, AI 몸통 회전 ≤ 360°/s) (g) 몽타주 — 재장전 BlendIn 0.1 → 0.25 · 피격 Med/Hvy 0 → 0.05 · 스켈레톤 슬롯 그룹 `HitReact` 분리(사용자) · `BP_AR4Rifle` 마지막 Delay 0.7 → 0.9 로 사격 금지 2.2 s = 재장전 몽타주 2.2 s(P197).
④ **결과** — 웅크림 토글 튐 75 → 2 · 일어나기 max 13.3 → 3.9° · 정지 총구 오차 중앙 5~6 → 1.4~1.7° · 재장전 직후 사격 척추 56.9/74.7 → 0.5/2.5°/프레임 · 피격→사격 전환은 피격 동작 자체 수준. ⚠ 측정 정정: 피치 75~80° 근처 본 회전은 **쿼터니언 각도 차**로(짐벌락).
⑤ **남은 것** — [C-176] 앉아 걷기 조준 오차 p90 ~24° · [W127] 아군 재장전 1.6 s 손목(`ALLY_MM_Rifle_Reload_Additive1`, 사용자 처리 중) · [W128] ABP 옛 Select 잔해 · [C-178] 값 넓은 장면 검증 + 2-PC 원격 피치 · [W129] P4 제출. MCP 한계 둘 기록(BP 함수 by-ref/Thread Safe 플래그 · 스켈레톤 슬롯 그룹 — `CLAUDE.md` 6.1).

★ **2026-09-30 — 조준 대각선 이동 발 끌림 (원칙 P200, [C-177] · [Q52] · [W124]~[W126]) — 원인 확정 · 임시 완화 · 근본 수정 대기** → `animation/2026-09-30_diagonal_aim_stop_selection.md`.
① **증상** — 조준 + 걷기 + 대각선에서 발이 끌림(아군·적군), 달리기·비조준도 45° 부근 통과 시 ~0.2 s. GASP 원본은 정상.
② **원인 [A]** — 우리 `*_PSD_Rifle_Stand_Walk/Jog_Loops` 는 **4방향**, GASP 는 FL/FR/BL/BR 포함 18 → 45° 쿼리에서 Stop(0.533)이 Loop(0.702)를 이기고 `continuingPoseCostBias −0.3` 으로 유지. 차단 A/B(워핑 · 발 배치 · 이동 정책 · AIBridge · 왼손 IK 등) 전부 무변화. "왜 지금" 은 미확정 [C-177](플레이어도 이동 정책으로 ×0.85 — `SoldierMovementProfile` 에 플레이어 게이트 없음 [W125]).
③ **임시 완화(저장, P4 체크아웃·미제출)** — Stand Walk/Jog Stops `baseCostBias` 0 → 0.2 · Loops `continuingPoseCostBias` −0.01 → −0.05(아군·적군 8 PSD). 사용자 평가: 안정 구간 개선, 전환 버벅임 남음 → **편향 튜닝 중단**.
④ **남은 것** — [W124] 대각선 클립 추가(Lyra `MF_Rifle_*_{Fwd,Bwd}_{Left,Right}` 8개 리타깃, 아군·적군) 후 편향 원복 검토 · [W125] 플레이어 제외 · [W126] 적군 `contact_l/r` 커브 누락 · Crouch Walk 는 편향 미적용.
⑤ **맹목사격 임시 비활성** — 신규 cvar `SoldierLab.Engagement.BlindFire`(기본 0, `SoldierEngagement.cpp` `PlanAperture`), 코드 경로는 유지. 다시 켤지 [Q52].

★ **2026-09-29 → 09-30 — 왼손 그립 IK (원칙 P194, [C-175] · [W123])** → `animation/2026-09-29_left_hand_grip_ik.md`. 꺼 두었던 왼손 IK 를 다시 켰다 — 목표 = 들고 있는 총 메시 `LeftHandGrip` 소켓의 위치+회전(`CopyBone hand_r→ik_hand_gun` → `ModifyBone ik_hand_l` → `TwoBoneIK`, 손목 회전 포함), 해제 = `DisableLHandIK` 커브 · 사망 · 총 없음 · cvar. **09-30 소켓 회전 캡처 완료**(`SK_AR4_X` P35.7/Y−161.7/R−153.3 · `SK_KA74U_X` P41.8/Y17.5/R27.0). 남은 것 = PIE 확인 [C-175]. *(09-29 에 이 요약이 빠져 있어 09-30 문서 정리 때 추가.)*

★★ **2026-09-29 — 상황별 이동 정책 + 재생배율 밴드 (원칙 P193, 값 [C-174], 작업 [W120]~[W122]) — PIE 실측 ✅** → `animation/2026-09-29_movement_policy_and_playrate_band.md`.
① **요구** — "시나리오에서 병사가 너무 빠르다"(사용자). 상황별로 속도를 조절하되 **애니메이션을 모르는 사람도 표로 커스텀 가능하게.**
② **★ 이 작업의 핵심 사실 [A]** — ABP `Get_DynamicPlayRate` 는 `배율 = Clamp(Speed2D / 클립의 MoveData_Speed, Min, Max)` 로만 적응하고, **`Min/MaxDynamicPlayRate` 커브가 없으면 대체값 0.75 / 1.25** 가 쓰인다(우리 클립 전부 해당). 클립 실측 `ALLY_MM_Rifle_Walk_Fwd` **291.31** · `Jog_Fwd` **582.62** = 캐릭터 `WalkSpeeds`/`RunSpeeds` 와 **동일**(클립이 그 속도로 리타이밍돼 있다) → **미끄러짐 없는 구간 = Walk 218~364 · Jog 437~728**. 사이 구간(364~437)은 Walk·Jog 두 세트가 모두 MM DB 에 있어 매칭이 섞는다. MM DB = Rifle Stand Walk/Jog(Loops·Starts·Stops·Pivots + TurnInPlace) + Crouch Walk + Idles/Idles_LowReady, 4방향, **146 클립 전부 루트모션 ON**, **Sprint 세트 없음**(브리지 `bDriveSprint=false` 라 AI 는 Sprint 요청 안 함).
③ **구현** — 신규 `AI/SoldierMovementProfile.{h,cpp}`(`USoldierMovementProfileComponent` + `FSoldierMovementProfileRow`) + `Content/SoldierLab/Data/DT_SoldierMovement`. 열은 **2개**: `MaxGait`(걸음걸이 **상한** — 클립이 그 속도로 authored 되어 있어 원리적으로 미끄러짐 0) · `SpeedScale`(**0.75~1.25 클램프 3겹** = `UPROPERTY meta` + 코드 + 전역 cvar 합산 후 재클램프). 상한은 `USoldierEngagementComponent` 의 gait 결정 **끝**에서 씌우고(덮어쓰지 않고 상한만 — 걸으라는 지시가 Jog 로 올라가지 않는다), 배율은 **gait 속도 벡터**(`WalkSpeeds`/`RunSpeeds`/`SprintSpeeds`/`CrouchSpeeds`, BeginPlay 에 원본 기억 → 매 틱 `원본 × 배율`)에 곱한다. 컴포넌트가 없으면 무제한 = 예전 동작.
④ **상황 선택 2단** — ① `BreakContact` → `Rush` 명령이 걸려 있으면 **그 행이 이긴다**(안 그러면 도주하며 재장전하는 병사가 걸어서 도망친다) ② 아니면 **최종 속도(`걸음 기준값 × 배율`)가 가장 낮은 행**, 동점은 열거자 순서. ⚠ **`Aiming`(=`WantsToAim()`, 엄폐지 사이를 달릴 때도 켜지므로 Jog 0.90) ≠ `Firing`(=`WantsToFire()`, 멈춰서 쏘는 구간 Walk 0.75)** — 사용자 지적으로 분리했다.
⑤ **현재 값** — Default·Rush·BreakContact·MovingToCover Jog 1.00 = **583** / Aiming Jog 0.90 = **524** / Normal·Suppressed Jog 0.85 = **495** / Wounded Jog 0.80 = **466** / Reloading·Scanning Walk 0.85 = **248** / Cautious·Crouched Walk 0.80 = **233** / EdgeAdvance·Firing·Peeking·HitReacting Walk 0.75 = **218**. 같은 값이 C++ `GetDefaultProfile` 에도 있어 표가 없어도 같게 돈다. cvar `SoldierLab.Move.Enabled`(A/B, 원본 속도 복구) · `SoldierLab.Move.SpeedScale`(전역, 밴드로 잘림) · `SoldierLab.Debug.Move 1`(`[Move] 이름 A -> B (max Walk, scale 0.75) walk 218 run 437`).
⑥ ⛔ **09-28 1차 구현은 폐기·정정** — ⓐ CMC 의 `MaxWalkSpeed` 에 곱했더니 **전혀 안 먹었다**: GASP `AC_PreCMCTick` 이 **CMC 직전에** `UpdateMovement_PreCMC` 로 `MaxWalkSpeed`·`MaxAcceleration`·`RotationRate` 를 매 프레임 다시 쓴다(우리 틱이 그보다 앞) ⓑ 입력으로 옮기니 속도는 변했지만 **발이 미끄러졌다** — 배율 0.45~0.7 이 ②의 밴드 밖이었고, **안전범위 0.45~1.2 는 측정 없이 정한 임의값이었다(근본 실수)** ⓒ **가속·회전 배율 2열은 `AC_PreCMCTick` 이 덮어써서 죽은 값**이었다("적용된다" 고 잘못 보고한 것을 정정, 열 삭제) → 되살리려면 틱 순서부터 [W121].
⑦ **검증** — 전 행을 `Walk / 0.75`(= 218, 원래 Jog 583 대비 **2.7배 느림**)로 밀어 PIE → **확실히 느려지고 발 미끄러짐 없음**(사용자 확인). 계획값 원복 후에도 정상(사용자 확인). 밴드 하단 끝을 먼저 밟은 이유 = 거기서 안 미끄러지면 그 위 전부 안전하다.
⑧ **남은 것** — [W120] **218 보다 느리게는 표로 불가능**(클립에 `Min/MaxDynamicPlayRate` 커브를 굽거나 느린 걷기 클립을 MM DB 에 추가) · [W121] 가속·회전은 틱 순서 해결이 먼저 · [W122] `DT_SoldierMovement` **P4 add 미완** · [C-174] 방향별(Left/Bwd)·Crouch 클립의 authored 속도 미측정(현재 버전에서 **미끄러짐은 관측되지 않았다**).

★ **2026-09-23 저녁 — 차량이 밟는 시체 · 사망 시 무기 드롭 (원칙 P192, 값 [C-171]~[C-173], 작업 [W118]~[W119]) — 코드 완료·재빌드 실측 대기** → `ai/2026-09-23_corpse_vehicle_interaction_and_weapon_drop.md`.
① **문제 제기 [A · 사용자 관측]** — UGV(`BP_UGV_0901`)가 쓰러진 적을 가끔 밟고 지나간다. **밀려나는 그림은 좋다**(유지 대상). 그런데 시체가 땅에 박혀 **부들부들 떨고**, 그 때문에 서스펜션에 이상한 값이 들어가 **차량이 뒤집히는 일은 절대 안 된다**. ⚠ 실제 뒤집힘 목격은 **0회 — 예방 수정**이다.
② **원인 [A · 엔진 소스]** — Chaos 서스펜션은 `ECC_WorldDynamic` 채널로 트레이스하고 응답은 `WheelTraceCollisionResponses` 를 쓰는데(`ChaosWheeledVehicleMovementComponent.cpp:486`·`:497`·`:1776`) **기본값이 "`ECC_Vehicle` 만 Ignore, 나머지 전부 Block"**(`:1142-1143`)이고 UGV 에 이를 덮는 코드가 없었다. 시체는 `StartRagdoll` 에서 `ECC_PhysicsBody` + `QueryAndPhysics` 로 바뀐다(`SoldierHealth.cpp:652-653`) → **바퀴가 시체를 지면으로 읽는다**: 그 바퀴만 지면이 수십 cm 위로 뛰어 스프링 힘이 튀고 **마찰까지 시체의 물리재질에서** 온다. 두 번째 경로(차체↔래그돌 **물리 접촉** 자체 = 끼임·떨림)는 **이번에 손대지 않음** → [W118].
③ **수정 1(차량, 한 줄)** — `UUGVWheeledVehicleMovementComponent` 생성자에 `WheelTraceCollisionResponses.SetResponse(ECC_PhysicsBody, ECR_Ignore)`(`:21`). **물리 접촉은 그대로 둬 밀려나는 그림을 유지**한다. 적용 범위는 `AUGVWheeledVehiclePawn` 자손(= UGV 계열)뿐.
④ **수정 2(사망 시 무기 드롭)** — `USoldierHealthComponent` 에 `bDropWeaponOnDeath`(true) · `DroppedWeaponMassKg`(3.5) · `WeaponMeshComponentName`("WeaponMesh"). 떨구는 순간은 **`StartRagdoll()` 안**(손이 애니메이션을 놓는 그 순간 — 더 일찍 떨구면 쥔 손에서 총이 빠진다). 공통 처리 `ReleaseAsDebris` = 오브젝트 타입 **`ECC_PhysicsBody`**(③의 한 줄이 시체와 총을 **동시에** 덮게 하려고 일부러 같은 타입) + `QueryAndPhysics` + **전부 Ignore 에서 시작해 WorldStatic/WorldDynamic/PhysicsBody/Vehicle 만 Block**(⚠ **Sight/Cover 를 뺀 이유 = 바닥에 굴러다니는 소총이 시야를 막거나 엄폐물로 계산되면 안 되기 때문**) + 질량 3.5 kg 오버라이드(물리에셋 기본은 소총 크기 hull 밀도라 **모루처럼** 떨어진다) + 시뮬 on + 사망 순간 속도 상속. `FreezeCorpse`(8 s)에 떨군 것들도 `PutRigidBodyToSleep`, `EndPlay` 의 `bDestroyCarriedActorsOnDestroy` 정리 목록에 **떨군 목록 추가**(떼어내면 `GetAttachedActors` 에 안 잡힌다 — P188 로 막은 누수가 다시 열릴 뻔한 자리).
⑤ ⚠ **함정 — 손에 들린 총은 스폰된 액터가 아니다.** 첫 구현("부착 액터를 떼어 떨구기")에서 **총이 오른손에 그대로 붙어 있었다.** 화면의 총은 `BP_SoldierCharacter` **자기 컴포넌트 `WeaponMesh`(`SK_KA74U_X`)** 이고, BeginPlay 에서 스폰해 붙이는 `BP_AR4Rifle`(`SK_AR4_X`)은 **숨은** 총구 소켓/FX 용 별개 액터였다(둘 다 물리에셋 보유). → 드롭 본체는 **컴포넌트 `DetachFromComponent(KeepWorldTransform)`** 경로, 액터 경로는 보조. 추가 규칙: **안 보이는 부착 액터는 안 떨군다**(`IsVisible()`/`bHiddenInGame`) — 안 보이는 물리 파편이 바닥에 남는 것을 막는다.
⑥ **재시작 정리** — `UScenarioRespawnSubsystem` 이 대표 액터를 **시체 포함 전부 `Destroy()`**(`ScenarioRespawnSubsystem.cpp:251-265`). 떨군 것이 **컴포넌트**면 시체 액터 소유라 함께 소멸, **액터**면 `USoldierHealthComponent::EndPlay` 의 정리 목록이 잡는다. 재시작 쪽 `DestroyChildActors`(부착 재귀 + Owner)는 **안전망**으로 남는다.
⑦ **검증** — 서스펜션 제외는 **적용·빌드됨**(정성 확인만 가능, [C-172]). 무기 드롭은 **재빌드·실측 대기**. 확인 로그 `LogSoldierAI: [Death] <이름> dropped N carried actor(s) and M held mesh(es).` — **`M`=1 이어야 화면의 총이 떨어진 것**이고 `N`=0 이 정상이다.
⑧ **남은 것** — [W118] 차체↔래그돌 물리 접촉(끼임·떨림) + **`FreezeCorpse` 는 애님만 멈추고 래그돌 바디는 계속 시뮬**(진짜 sleep 미적용) · [W119] UGV 컴포넌트 헤더 주석 정정 · [C-171] 질량/마찰 미측정 · [C-172] 제외 후 거동 · [C-173] `PhysicsBody` 제외가 다른 물리 오브젝트(파편·드론)에 주는 영향.

★★ **2026-09-23 — 차량(트럭) 상대 교전이 안 되던 원인 둘과 수정 둘 (원칙 P189, 값 [C-166]~[C-168])** → `ai/2026-09-23_vehicle_target_engagement_fix.md`.
① **증상·확정** — New_kadex_0811 3차 전투지에서 적 3분대가 `BP_TitanTruck` 을 거의 안 쏘고 자리만 옮겼다(대인은 정상). `SoldierLab.Debug.Engagement.Log 1` 의 `[Engage] … tgt=BP_TitanTruck … believed 1 worth 1 aperture 0(none/open)` → **표적·확신·가치는 전부 통과, 사격 자세만 0**.
② **원인 1(주원인) [A]** — 차량은 소켓이 없어 조준점 = 바운즈 **50%**(차체 한가운데). `IsShotBlockedByWorld` 는 충돌점이 조준점에서 `LaneToleranceCm 200` 보다 멀면 막힘으로 보는데, 차량은 Pawn 이 아니라 `BodiesAreNotWalls()` 가 안 무시하고 Sight 채널을 Block 한다 → **표적 자신이 벽**. 트럭 메시 실측 측면 133 cm(통과) / 정면·후면 310~340 cm(막힘) = **측면 축 ±48° 밖은 사각**. `PlanAperture` 는 일곱 자세를 **같은 레인 테스트 하나**로 검사해 전부 동시 탈락 → `Blocked` → `bLaneDenied` → 재배치 반복(= 관측된 "서 있다 이동"). ⚠ `BP_TitanTruck` 은 `BodyMesh` yaw 270° 라 **장축이 액터 X축**.
③ **수정 1** — `IsShotBlockedByWorld`/`PlanAperture`/`FindAperture` 에 `const AActor* TargetActor = nullptr` 추가, `HitActor == TargetActor \|\| 부착부모 == TargetActor` 면 **도달**. 호출부는 `Contact.Enemy.Get()` 을 `LaneTarget` 으로(청각 등 액터 없는 기록은 `nullptr` → 기존 규칙 그대로, P130). 레인 캐시 `FLaneAnswer` 에도 `TargetActor` 를 넣고 **캐시 키로 비교**. 200 cm 규칙은 남아 **대인 동작 불변**(병사는 Pawn 이라 애초에 무시).
④ **원인 2 / 수정 2** — `GatherThreatEyes` 의 위협 눈 = 기록 + `ThreatEyeAboveContactCm 20`. 차량 기록은 차체 한가운데(≈2 m), 실제 포탑은 지붕(≈3.3 m+) → **1 m 이상 낮은 눈**으로 엄폐를 계산해 "숨었다고 판단한 자리"가 뚫렸고, 그 눈이 **차체 콜리전 내부**라 fight probe 가 설계 밖 케이스였다. 수정: **`HasSocket(TargetSocket)` 이 false 일 때만**(= 바운드박스 폴백 = 차량) 눈 단차를 그 대상의 형상(`GetEyeLocation().Z − GetTargetLocation().Z` = 80% − 50%)에서 계산. **형상은 보면 아는 것이라 위치 정보가 아니다** → P130 위반 아님. 병사는 +20 cm 유지. 곁가지: `USoldierIdentityComponent::HasSocket()` public(비-`UFUNCTION` → Live Coding 으로 빌드됨).
⑤ **검증** — `L_SoldierTest` 에 트럭 배치(적 3명 96 m, **정면 = 최악 조건**)로 재현 → 수정 후 사격 정상 → **New_kadex_0811 본 레벨에서도 정상(사용자 확인)**. ⚠ 시험 레벨 재현 세팅: `ScenarioConfig_0.bDemoAutoStartScenario=false`(자동 시작이 `EnemyInfiltrate` 의 **HoldFire** 를 걸어 `roe=hold` 가 된다; 분대 명령이 없으면 기본 배정이 **ROE Free**) · 트럭은 `RCWSFireControl.CurrentMode=Remote` + `bDemoForceCommandPostAutoFire=false` 라 반격 안 함. **진영 판정은 원래 정상**이었다(트럭 `DetectableTarget` Friendly → 브리지가 `SoldierIdentity(Friendly)` 부착).
⑥ **남은 것** — [C-166] `TargetRadiusCm 45` 가 사람 가슴 기준이라 트럭도 45 cm 표적처럼 조준 게이트를 잼(원거리 `Aimed` → `Suppressive`) · [C-167] 차량 조준점이 포탑이 아니라 차체 중앙 · [C-168] 차량 위협 엄폐 품질 수치 미측정(정성만).

★★ **2026-09-23 — 로우레디(총 내림) 상체 레이어 (원칙 P190~P191, 값 [C-169]~[C-170], 작업 [W117])** → `animation/2026-09-23_low_ready_upper_body_layer.md`.
① **결과** — 정지·걷기는 총 내림(LowReady), 조깅은 올림, 전환 부드러움, **왼손 그립 IK 미사용**. PIE 확인 ✅.
② **그래프** — 소스 6개(ALLY/Enemy × LowReady/Idle_ADS/Idle_Hipfire) → 진영 `Blend Poses by bool` → **로우레디 = `Apply Mesh Space Additive`(Base Idle_ADS + LowReady)**(LowReady 애님이 ADS 기준 MS additive다) → **걷기 흔들림 = `Make Dynamic Additive`(Base 정지 Hipfire, Additive `AimedPose` 캐시)** → 그 흔들림을 `Layered blend per bone` 으로 **clavicle_l/r·neck_01 이하에서 0** → 로우레디 + 흔들림(Alpha = `Map Range Clamped(Speed2D, 20→150)` × 0.6) → 최종 `Layered blend per bone`(Base = `AimedPose` 캐시, MS Rotation Blend, 가중치 = `FInterp Ease in Out(WeaponLowered, Exp 2)`) → **`Slot 'UpperBody'` 의 Source**.
③ **터진 것 다섯** — (a) 걷기 2배속 = 로코모션 **포즈 fan-out** → `AimedPose` 캐시를 두 번 읽기(새 `SaveCachedPose` 는 MCP 로 못 만듦) (b) 플레이어 빙의 시 안 보임 = 위의 가중치 1 상체 슬롯이 덮음 → 레이어를 **슬롯 Source** 로 (c) **다 올라올 때 오른손이 튐** = `Curve Blend Option` 기본 **Override** + **가중치 0 이면 노드 통째 스킵**(`AnimNode_LayeredBoneBlend.cpp:249`)의 합 → additive 로 딸려온 `Enable_Warping` 등이 0.6→0 에서 1 로 튐 → **`UseBasePose`** (d) 멈추면 2프레임 만에 뚝 = 애님그래프 `Speed2D` 직결 → **C++ 램프로 이동**(캐릭터 `WeaponLowered` 와 **분리된 별도 값**, 그래프 250/400 노드는 연결 해제) (e) 총을 앞으로 내밈 = 흔들림 additive 에 든 **정적 팔 오프셋** → 위 ②의 팔·목 제거 노드.
④ **커스터마이즈** — `BP_SoldierCharacter` `WeaponRaiseRate` 2→**2.5** / `WeaponLowerRate` 8→**3**(예전엔 레이어가 spine_05 부터라 체감이 없었다) · cvar `SoldierLab.Pose.WalkLowered 1.0`/`.JogLowered 0.0`/`.IdleSpeed 20`/`.RaiseSpeedStart 250`/`.RaiseSpeedFull 400` · `FInterp Ease in Out` Exponent 2 · 흔들림 `Multiply` B 0.6, 구간 20/150 · LowReady `Sequence Evaluator` Explicit Time 2.5.
⑤ **블렌드 마스크** — `SK_UEFN_Mannequin` 에 `BM_LowReady_Layer` · `BM_LowReady_Sway` 신설, 두 `Layered blend per bone` 을 **Blend Mask 모드**로(Branch Filter 는 본별 가중치를 못 줌). 마스크 값은 사용자 조정 중 [C-169].
⑥ **남은 것** — [W117] cvar 5개 `UPROPERTY` 승격(헤더 리플렉션이라 **에디터 닫는 정식 빌드** 때) · [C-169] 마스크 가중치 · [C-170] 조깅 raise 구간.

★ **2026-09-23 — 아군 앉기 무릎/발 IK (해결, 새 항목 없음)** → `animation/prototypes/2026-09-23_ally_crouch_ik_bones_removed.md`.
아군만 V/B 로 낮출 때 **무릎이 안 굽고 몸이 떠다니던** 원인은 디자이너 재임포트 메시(**#499**) **LOD0 의 `Bones to Remove` 에 `ik_*` 본이 들어가 있던 것** — 계층에는 있지만 LOD0 에서 `NonRequiredBone` 이라 런타임 포즈에 없고, 다리 IK 가 조용히 죽었다. ⚠ **FBX/블렌더 대조는 "본 동일" 로 오진했다 — 정답은 스켈레톤 트리 아이콘(속이 빈 동그라미 = NonRequiredBone)**. 이어서 앉은 자세에서 발이 꼬이던 것은 **`ALLY_MM_Rifle_Crouch_Idle` 한 장만** `ik_foot_*` 가 원점에 박혀 있던 것 → **`AM_Copy_IKFootRoot`**(attach→ik_foot_root, foot_l/r→ik_foot_l/r) 적용으로 해결.

★ **2026-09-23 — 소총 액터 누수의 근본 수정: `USoldierHealthComponent::EndPlay` + `bDestroyCarriedActorsOnDestroy` ([W116] 해결, 원칙 P188)** → `../level_new_kadex_0811/2026-09-22_scenario_restart_implementation.md` §5(2-PC 절). titan 재시작 세션이 2-PC 테스트 중 확정해 **이 모듈의 C++ 를 직접 수정**했다(BP 수정 0건).
① **원인 [A]** — `BP_SoldierCharacter` 가 BeginPlay 에서 소총을 `SpawnActor` 해 손에 붙이는데 그 BP 에 `K2_DestroyActor` 도 `EndPlay` 도 **0개**, `USoldierHealthComponent` 는 `DestroyAfterSeconds` 뒤 `Owner->Destroy()` 만, 엔진 `UWorld::DestroyActor` 는 **자기 Owner 만** 비우고 자기가 소유한 액터는 어태치만 끊고 살려 둔다(`Engine/Private/LevelActor.cpp`). 게다가 `BP_AR4Rifle` 은 **`bReplicates=false`** 라 프로세스마다 자기 것이 있어 서버가 치워도 클라 것은 남는다 → 사망마다 고아 소총 1개 + `OwningCharacter is not valid` 스팸 + GT 지연 누적. **재시작만의 문제가 아니라 평상시 사망 경로의 누수였다.**
② **수정** — `EndPlay` 오버라이드가 `EEndPlayReason::Destroyed` 일 때만, 자기가 어태치(`GetAttachedActors` 재귀)하거나 소유(`Children`)한 액터를 같이 파괴. 스위치 `bDestroyCarriedActorsOnDestroy`(EditAnywhere, 기본 true). 예외 둘 — 풀링 투사체(`ASoldierProjectile`: 풀이 포인터를 쥔다) · 클라이언트의 **복제된** 액터(`ROLE_Authority` 인 것만). 순서상 안전: `DestroyActor` → `Destroyed()` → `RouteEndPlay` → 컴포넌트 `EndPlay` 가 어태치 해제·`SetOwner(NULL)` **이전**.
③ **검증** — New_kadex_0811 2-PC 실기, 사용자 판정 "해결됨"(`obj list class=BP_AR4Rifle_C` 카운트 실측은 안 함). titan 재시작 쪽 정리(`DestroySoldierAttachments`/`DestroyOrphanedChildActors`)는 **안전망**으로 남았다(낙하산 등 병사가 아닌 재스폰 대상용).
④ **원칙 P188** — 액터가 런타임에 스폰해서 들고 있는 것은 자기가 치운다. 이 컴포넌트를 안 단 액터가 뭔가를 들고 있으면 여전히 그 액터의 몫이다.

★ **2026-09-22 — titan 시나리오 재시작(확인창 + 자동 재시작)이 SoldierLab 리셋 계약을 호출한다 (titan 세션이 이 모듈에 추가)** → `../level_new_kadex_0811/2026-09-22_scenario_restart_implementation.md`(구현) · `../level_new_kadex_0811/2026-09-10_scenario_auto_restart_design.md` 4a절(계약 정의). ID **W116 · P187**, 새 C 없음.
① **계약(이 모듈 소유, titan 이 호출만)** — `USoldierSquadSubsystem::ResetForRestart()`(`Squads`/`Members` 맵 · `NextOrderSerial` 초기화) · `USoldierSituationFieldSubsystem::ResetForRestart()`(Built* 무효화 → `EnsureLevels` 가 첫 실행 경로로 재구축, **지오메트리 캐시 `Horizons` 까지 비움** — 설계의 "유지"는 폐기, "PIE 와 동일" 기준) · `USoldierProjectilePoolSubsystem::RecallAll()`(나는 탄 전부 `ASoldierProjectile::Park()`). 콘솔 **`SoldierLab.ResetWorld`** = 셋 한 번에(시험 레벨 단독 검증용 — 병사 재스폰·차량 부활은 안 함, 그건 titan `titan.ScenarioRestart`).
② **병사는 재스폰이지 부활이 아니다** — titan `UScenarioRespawnSubsystem` 이 `OnWorldBeginPlay` 에 `USoldierHealthComponent` 보유 액터의 클래스/트랜스폼/인스턴스 저작 델타(실측 `AC_SoldierIdentity.SquadId/bSquadLeader`, 아군 `AC_SoldierHealth.bInvincible`)를 스냅샷 → 재시작 시 Destroy → 8구/프레임 재스폰 → 델타 복원. 컴포넌트 13개의 BeginPlay 가 전부 다시 돌므로 병사 쪽은 리셋 코드가 없다. 등록부·`DetectableTarget` 은 EndPlay 자동 + 브리지 `ForceRescan`. `ReinforceSquads` 가 바꾼 `SquadId` 도 저작값으로 돌아온다.
③ **함정 둘(titan 쪽에서 해결, 이 모듈 BP 를 건드리진 않음)** — (a) `BP_Soldier_*` 의 **`AutoPossessAI=PlacedInWorld`** 는 스폰된 폰에 컨트롤러를 안 붙인다(`APawn::PostInitializeComponents` 는 월드 시작 중에만 빙의) → 재스폰본이 안 움직였다. 재스폰이 지연 스폰 중 `PlacedInWorldOrSpawned` 로 바꿔 `FinishSpawning`(BeginPlay 전 빙의). SoldierLab 쪽에서 병사를 런타임 스폰할 일이 생기면 같은 함정. (b) 병사만 Destroy 하면 병사가 스폰해 손에 붙인 **`BP_AR4Rifle` 액터가 남아** 매 틱 `OwningCharacter is not valid` 스팸 + 사이클마다 두 배 → 딸린 액터(`GetAttachedActors` 재귀 + Owner) 동반 파괴. ~~**평상시 사망 `DestroyAfterSeconds` 뒤에도 소총이 남는지는 미확인 → [W116]**~~ → **09-23 남는 것으로 확정 + `USoldierHealthComponent::EndPlay` 로 근본 수정(위 09-23 블록, P188)**.
④ **원칙 P187** — 월드 서브시스템에 새 런타임 상태(맵·캐시·카운터)를 두면 그 서브시스템의 `ResetForRestart()` 에도 넣는다. 서브시스템은 재시작을 살아남으므로 빠뜨리면 **1회차는 멀쩡, 2회차부터 어긋난다**. 검증 결과: L_SoldierScenario 에서 재시작 뒤 2회차 사용자 판정 "완벽하다". **09-23 New_kadex_0811 2-PC 도 검증 완료**("이제 잘됨" — 최종 원인은 레벨 GameMode 오버라이드가 `GM_SoldierLab` 로 바뀌어 있던 것. `GM_SoldierLab` 은 titan GameState/PC 가 없어 재시작 멀티캐스트가 전부 서버-로컬 폴백이 된다 → **2-PC·전시 구성에서는 `BP_KadexTestGameMode`, `GM_SoldierLab` 은 단일 프로세스 관전 전용**). titan 쪽 남은 것: 드론 짐벌 배율 리셋 **빌드 대기** · 10사이클 메모리/fps · 장시간 무인 반복 · P4 제출 순서 조율(낙하산 `RespawnActors` 연결은 09-23 **불필요로 종결** — 정적 액터).

★★ **2026-09-21 17:10~18:10 — 게임 스레드 구조 묶음: 투사체 풀 · 레이 채널 응답+캐시 · CMC/메시 데이터 · 박자 · 캐릭터 BP 틱 → C++ (성능 후속 세션 후편, 전편 13절 순서 ①~⑤ 실행)** → `ai/2026-09-21_game_thread_structural_pool_rays_bridge.md`. 조건 `L_SoldierScenario` 35명 PIE, `stat dumpframe` 파싱, 전부 [A]·사용자 PIE 확인. ID **W114~W115**, 새 C·P 없음.
① **투사체 풀 [W109] ✅** — 신규 `Weapons/SoldierProjectilePool.h/.cpp` `USoldierProjectilePoolSubsystem::Acquire(Class, Owner)`: 클래스별 풀, 주차된 것 커서부터 재사용 → 상한(cvar `SoldierLab.Projectile.PoolMax` 96)까지 스폰 → 상한이면 RCWS 식 라운드로빈(1회 경고). `BP_AR4Rifle.Shoot`·`PlayShotCosmetics`·도탄 `Multicast_LaunchRicochet` 전부 `Acquire → LaunchFrom`. `CollisionComponent` **QueryOnly**(`SoldierProjectile.cpp:124`, 명중은 이동 스윕 — "명중 잘됨") · `BP_RifleProjectile` 빈 EventTick/BeginPlay/Overlap 삭제 · 신규 **`MaxFlightDistanceCm 60000`**(사용자 제안 — 5 s 시간 상한만으론 빗맞힌 탄이 4 km). 결과: `Shoot` 안 스폰 **0.83/발 → 0**, `FEndPhysics` 1.03 → 0.62/0.46, 투사체 **47 에서 정지**(비행 15), World Tick 18.3/20.3 → 17.1/14.3.
② **레이 [W110] ✅** — 신규 `AI/SoldierQuery.h` **`SoldierQuery::BodiesAreNotWalls()`**(`FCollisionResponseParams` ECC_Pawn → Ignore; 병사는 캡슐·메시 모두 Pawn, 이 레벨들의 Pawn 타입은 병사뿐 — 트럭/UGV Vehicle · 드론 PhysicsBody · 총 NoCollision MCP 확인). 35명 `AddIgnoredActor` 루프를 Engagement 1 · Cover 5(`IgnoreBodies` 빈 함수) · Field 2 사이트에서 제거. `IsShotBlockedByWorld` 캐시 **`LaneCacheMoveCm 15` · `LaneCacheSeconds 0.15`** 8슬롯 `FLaneAnswer`(PlanAperture 옵션도 자동). 결과: 씬 쿼리 **1,030~1,120회 2.6~3.0 → 357/576회 0.96/1.42 ms**, Engagement 레인 182~188 → 23/35회, Engagement 틱 0.67~0.82 → 0.22/0.17.
③ **데이터 [W111] ✅ + 정정** — CMC `bAlwaysCheckFloor=false`(`FindFloor` 99~105 → 44/27회) · `bEnablePhysicsInteraction=false` · ★ **병사 메시 콜리전 `QueryAndPhysics → QueryOnly`** — 문서(전편 [W98] ② · IMPLEMENTED)는 QueryOnly 라 적었으나 **실제 CDO 는 GASP 커스텀 프로파일 QueryAndPhysics 였다**. 사망은 `SoldierHealth.cpp:572 StartRagdoll` 이 QueryAndPhysics 로 되돌려 안전, 피격 부위 트레이스는 Query. `FEndPhysics` 효과는 [B]. 부모 CDO + 자식 2 + 인스턴스 35 적용·저장.
⑤ **박자 [W102] ✅** — `SoldierCover` **`CandidateIntervalSeconds 0.033`** + `LastCandidateStepSeconds`(눈 있는 스윕 후보 걸음) · `SoldierSight` **`ScanIntervalSeconds 0.033`** + `LastScanSeconds` — 프레임이 아니라 **시간**에 고정(30 fps 거동·비용 동일, 60 fps 트레이스 절반, 값 ↑ = 결정 지연 ↔ 비용). Score 캐시는 눈 세대마다 바뀌어 생략.
④ **캐릭터 BP 틱 → C++ [W108] ✅** — 신규 `Pose/SoldierAIBridgeComponent.h/.cpp` **`TickBridge(DeltaSeconds)`** = `BP_SoldierCharacter` EventTick 본문(조준 보정 회전 수학 → 린 램프 → 입력 상태 Sprint/Aim 직접 쓰기 → AOActive/AIPoseDriven/AI 다리 5 → `UpdateBodyYawRate`/`UpdateBlindFire`/`UpdateStance` → WantsToFire/Reload → Rifle `Shoot`/`StartReload`)을 **노드 순서·Kismet 산술 그대로**. 변수 47개(캐릭터 34·부모 4·ABP 7)는 **BP 소유 그대로 FProperty 로**(스무더·게이트브리지·리플리케이션 분기가 같은 변수). BP: 컴포넌트 `AC_SoldierAIBridge`, EventTick = `Parent:Tick → TickBridge → Branch(ShouldRunBlueprintCopy) → (true) 옛 본문`(리플리케이션 세션의 `HasAuthority` 분기·바운드 이벤트 2 는 옛 본문 안). 바인딩 실패 시 경고 + BP 경로, cvar `SoldierLab.AIBridge.Native 0` A/B. 결과: 아군 20 ReceiveTick **1.16~1.21 → 0.22**, World Tick 15.1/15.1 → **11.3/14.0**, 경고 0, 사용자 "잘된다, 교전 중 50fps 후반".
⑥ **누적·남은 것** — 오늘 22.6(원 29) → **11~14**(카메라 −2.0 · 총구 −0.4/발 · 풀 −0.8/발 · 레이 −1.5~2 · BP 틱 −1.6 · 기타 −0.5). 남은 상위: Cover 1.2~2.6(점수) · CMC 1.3 · 메시 틱 1.1 · **`AC_PreCMCTick` 0.8 → [W115]** · 틱 오버헤드 1.2(틱 함수 ≈ 600 → [W112]) · Sight 0.9 · 물리 0.6 · [W113]. 별건: `AN_Reload` 노티파이(`Content/Characters/Heroes/Abilities/AN_Reload`)가 어빌리티 컴포넌트 없는 병사에 `GameplayEvent.ReloadDone` → `LogAbilitySystem: Error` 41회/PIE(기존, 비용은 로그뿐) → **[W114]**. 리플리케이션 세션 완료, 2PC 검증 내일(사용자).

★★ **2026-09-21 — New_kadex_0811 SoldierLab 이관 · `BreakContact` · 표적 제외 (분대 세션)** → `../level_new_kadex_0811/2026-09-21_soldierlab_migration_new_kadex_0811.md` · `squad/2026-09-21_break_contact_and_targeting_exclusion.md`.
① **레벨** — 구 병사 40 삭제(P4 백업), `BP_Soldier_Hostile_1~15`(분대 1/2/3 × 5, 첫째 리더) · `BP_Soldier_Friendly_1~25`(1~5 × 5) 를 옛 트랜스폼에. 마커 113·경로 스플라인 3·드론 경로·낙하산·트럭·UGV 유지. **`ASoldierZone` 8개** = `Z0_S1/S2/S3_Engage`(1차, 분대별 — 존이 `NavFilterClass` 를 나르므로 분대별 경로 스플라인 필터를 살리려면 분대별 존) · `Z1_S2/S3_Withdraw` · `Z2_S3_Escape`(트럭 앞 37 m) · `ZF_North/South_Defend`. `SquadZones`: Hostile 1→[Z0] · 2→[Z0,Z1] · 3→[Z0,Z1,Z2] · Friendly 1~3→[ZF_N] · 4~5→[ZF_S]. 기하: 스폰 → 1차 280 m → 2차 200 m → 아군 60~90 m → 3차 360 m.
② **DT `DT_ScenarioSteps_ThreeStage_SoldierLab` 26행** — `ThreeStage` 26행 − `RetargetToAllies`·`RetargetToCommandPost`·`HoldFleeingFire`·`AllyAmbush` + `AllyDefend`(Occupy ReturnFireOnly 0.3 사거리 60 m) · `AllyEngage`(`EnemyNearFriendlySoldiers` 8000 → Free) · `Squad3Run` · `Squad3Stand`. 적 행: `EnemyApproach` MoveTo Cautious **HoldFire** 0.3 · `EnemyEngage`(트리거 `UGVFiredNearEnemy` 그대로) Occupy Rush Free 0.8 · Flee2 Withdraw Quota 10 · Flee3 Withdraw Quota 5 · `ExcludeFleeingEnemies` SetTargetable false. 드론·UGV·트럭 행 그대로(`bEnabled=false` 4행 포함).
③ **첫 PIE**(04:38 UTC) — `EnemyApproach` +1 → `UAVSpotted` +82 → `UGVArriveZone1` +170 → `EnemyEngage` +180 → `EnemyFleeToZone2` +210(`[Squad] reinforce Hostile quota=10 living=7 needed=3 moved=3 : Hostile_1(1->2), Hostile_5(1->2), Hostile_4(1->3)`) → `AllyEngage` +260 → `UGVMoveZone2` +274 → `EnemyFleeToZone3` +350(`quota=5 living=4 needed=1 moved=1 : Hostile_6(2->3)`) → `CommandPostFire` → `ExcludeFleeingEnemies`. 전부 순서대로, 사용자 "아주 잘됨". 페이싱: 접근 280 m ≈ 180 s 가 드론·UGV 와 맞물려 허용 [C-164].
④ **문제** — `ExcludeFleeingEnemies` 뒤에도 `[RCWSFireControl] BP_UGV_0901_C_1: 타겟 BP_Soldier_Hostile_C_4/6/3 …`(대타 포함) 이어짐, 3분대는 2차 존 엄폐를 홉하며 아군과 교전, 3차 존까지 안 감. 원인 셋: (a) 구 `ExcludeFleeingEnemiesFromAllyTargeting` 이 켜 주던 **UGV RCWS `bRespectEnemyTargetingExclusion=true`**(인스턴스 기본 false, 트럭은 설계상 false)를 `SetTargetable` 경로가 안 켬 (b) SoldierLab 아군 보병이 제외 플래그를 **아예 안 읽음** (c) `Withdraw` + `ReturnFireOnly` = 엄폐 층이 매 홉을 싸울 자리로 값 매김 + dwell + 응사.
⑤ **수정** — **`ESoldierOrderVerb::BreakContact`**(규칙 동사, ROE/사거리/속도 동반, 다음 존 동사가 지움) → `USoldierCoverComponent::IsBreakingContact()`(`bBreakContact && Mode==Approach` — 존 안은 Hold 로 평소 가격) 이면 `ScorePosition` 의 Fighting/Route/Danger/Suppression **0**(Objective+Squad 만), `MinDwell`/`ScanDwell` 없음 · 교전: `bWantsToSprint=true`(이동 중) · `DesiredStance=0`(재장전 제외) · **`IsContactExcluded(record)`**(상대 배정 `!bTargetableByOwnSideWeapons` → `PickCandidate` 제외 + 잠금 해제, **인지·은폐·위협 보너스는 그대로** P185) · `IssueSquadOrderSpec`: `SetTargetable(false)` → UGV RCWS 스위치 ON(트럭 제외) · DT `Squad3Run`(Flee3 +6 s, BreakContact HoldFire Rush) · `Squad3Stand`(`CommandPostFiredNearEnemy` 8000 → Occupy z2 Rush Free 0.8). 라벨 ` BREAK`/` excl`. 도주 = ROE 가 아니라 **땅의 가격**(P184). ⚠ `MinStance` 바닥이 `DesiredStance=0` 뒤에 와서 Rush 아니면 도로 올린다 — titan DT 연결([W84]) 때 `bBreakContact` 면 건너뛸 것.
⑥ **게임 모드** — `GM_SoldierLab` 에서도 시나리오는 돈다(`UScenarioStateSubsystem` = GameInstance 서브시스템, `ResolveUGVPawn` 은 titan PC CDO 의 `UGVVehicleClass` 폴백, GameState 사용은 전부 null 가드). 잃는 것: RTSP · HUD/미니맵 · `ShowUIMessage` 토스트 · 데모 플래그 복제.
⑦ 죽은 것(컴파일은 됨): 구 `EnemyCombatComponent` 시나리오 기제 전부([W105], `kadex_test` 결정 뒤) · 드론 프레이밍의 분대 판정이 `LastStandZoneIndex` 를 읽어 SoldierLab 적군엔 필터 통과([W104], 코스메틱) · ⚠ 문서 세션 발견: **RCWS 스티키 표적은 제외를 안 본다**(`RCWSFireControlComponent.cpp:584-613` — 제외 순간 물고 있던 한 명은 시야를 1 s 잃어야 놓음, [W106] RCWS 몫). 1v1 `L_SoldierTest` DT(`AllyGuard`/`EnemyInfiltrate`/`EnemyAssault`/`EnemyArrived`/`ScenarioComplete`, 에셋 mtime 09-18) 기록. Perforce: `New_kadex_0811.umap` 은 `user2` 도 체크아웃(바이너리 — 순서 조율), `Source/` 는 넓은 `p4 edit` 로 opened 가 신호 아님.

★ **2026-09-21 — 성능 계측 · 엄폐 틱 비용 (AI 세션)** → `ai/2026-09-21_perf_instrumentation_and_cover_cost.md`.
① **기준선** — 빈 레벨 World Tick 1.71 ms → 35명(`L_SoldierScenario`, 적 15 / 아군 20, UGV·트럭·UAV, PIE, 로깅 on) **29.2 ms**, fps 20~30. `stat game` 이 이름을 댄 것은 ≈ 6 ms(Blueprint 6.7 · CharMovement 1.4 · Transform 0.82/752 calls · Spawn 0.7), Ticks Queued 968(병사당 ≈ 25). 사용자 "무식하게 바로 하지 말고" → 계측 → A/B → 이름 붙은 것만(P182).
② **Phase 0 계측** — `AI/SoldierLabLog.h` **`STATGROUP_SoldierLab`**(`stat SoldierLab`): 시스템 틱 9 + Cover 하위 7(Here / Begin Sweep / Candidate / Route / Score / Finish / Edge Advance) + Field 5(Shadows / Bake / Evict / Overlay / Wedge Reads) + Squad/Zone 선언([W103]) · `Traces: Sight/Cover/Engagement/Field` · `Soldiers Ticked`. off 스위치 **`SoldierLab.<Sight|Perception|Cover|Engagement|Suppression|Comms|Field>.Enabled`** + **`SoldierLab.Cover.Avoidance`**(RVO 런타임). 첫 계측: **Cover Tick 8.92 ms(34 calls) · Traces: Cover 487/프레임(max 624)** · Sight 0.96(108) · Engagement 0.35(57) · Field 0.08 — Cover 혼자 프레임의 31 %.
③ **Cover 수정 넷, 전부 결정 보존**(P183 — 같은 눈·후보·비용 함수 → 같은 자리): (a) **이동 중 스윕 정지** `bSweepSuspended`(`FinishSweep` 의 `bAlreadyGoing` 과 같은 식 — 어차피 버리던 스윕을 안 걷는다, 정지하면 지금 발·지금 눈으로 새 스윕) + 볼 곳만 **`UpdateWatchPoint()`** 로 `WatchRefreshSeconds 0.25` 마다(이동 중 스캔 거동 불변) (b) 발밑 HERE 매 틱 27 트레이스 → **`HereEvalIntervalSeconds 0.1`** ∨ 걸음 > `MicroStepCm 30` ∨ 새 스윕(스윕 끝에 한 번 읽히는 값이고 눈은 얼려 있다) (c) **경로 가지치기** — 경로 비용 ≥ 0 이라 경로 없이 best 에 지는 후보는 경로 트레이스 생략(정확, best 동일) (d) 눈 0 스윕 **`CalmCandidatesPerTick 2`**(트레이스 0 이라 후보 30~40개가 매 틱 통째로 — 필드 읽기 + `GetStaleVantage` 적분). `[Cover|Cost]` 카테고리.
④ **결과** — Cover **1.92 ms**(Candidate 1.29 / Score 0.51 / Begin Sweep 0.43 / Route 0.35 / Here 0.15 / Finish 0.03 / Edge Advance 0.02 — 하위는 중첩) · Traces: Cover **243** · SoldierLab 합 ≈ **3.3**(Cover 1.9 · Sight 0.9 · Engagement 0.3 · Field 0.1) · **World Tick 28.5 → 22.6 ms**, 프레임 31, GPU 6 → 게임 스레드 바운드. **워커 애니 19.7 ms 는 병렬**(대기 `Post Tick Component Update` 1.15 뿐) — 게임 스레드의 애니 ≈ 7.3 이 문제.
⑤ **RVO A/B** `Cover.Avoidance 0`: Char Movement 1.58→1.32 · MoveComponent 2.20→1.01 · EndScoped 0.93→0.73 — 단 "켬" 캡처에 **히치**(World Tick max 84 · `DispatchBlockingHit` max 58.8) → 정상 상태 RVO 0.3~0.5 ms, **켜 둔다**. 히치는 별건 — 무기 `OnHit` 의 동기 에셋 로드 의심(추정) → **[W101]**.
⑥ **인계**(7절 표, ms 가 이유) — 포즈: ABP 이벤트 그래프 → 스레드-세이프(`BlueprintUpdateAnimation` 3.47, 병사당 ABP 2) · `KinematicBonesUpdateType = SkipAllBones` 사망 전까지(0.97) · URO → **[W98]** / 캐릭터 BP: 부착 컴포넌트 ≈ 23/병사(Transform 800 calls) · 오버랩 컴포넌트 · 틱 로직 ≈ 2.5 → **[W99]** / 무기: 투사체 풀링 0.65 → **[W100]** · 히치 **[W101]** / 이 층 다음 몫(Sight 표적 트레이스 간격 0.4 · Cover 후보 박자 0.5 · 틱 함수 솎기) → **[W102]**, 다른 층 뒤 / 분대: `SCOPE_CYCLE_COUNTER(STAT_SoldierLab_Squad/Zone)` → **[W103]**.
⑦ **측정 규약**(8절, P182) — 로깅 off · 오버레이 off · 같은 카메라/교전 시점 · Standalone · 세 갈래 확인(자기 스코프 / `.Enabled 0` 차 / 병사 수 스케일링) · 평균과 max 같이 · 워커/GPU ms ≠ 게임 스레드 ms · 전후 stat 줄 필수. 이번 숫자는 PIE + 로깅 on 이라 **차이만 믿을 것**. 값 [C-162](박자 값의 거동 지연 — 눈에 띄면).

★ **2026-09-21 — 게임 스레드 묶음: 카메라 · ABP · 숨은 총 메시 · 총구 Niagara · URO 크래시 (성능 후속 세션, [W98]~[W103] 실행)** → `ai/2026-09-21_game_thread_batch_cameras_abp_muzzle.md`. ⚠ ID 는 **W107~W109 · C-165 · P186**(처음 잡은 W104~W106·C-163·P184 는 분대 세션이 선점).
① **계측법** — MCP 엔 콘솔 툴이 없어 사용자가 PIE 콘솔에 **`stat dumpframe -ms=0.05`** → `Saved/Logs/titan_example.log` 의 `LogStats:` 블록(≈ 3000줄, 스레드별 스탯 계층)을 파싱하면 **틱 함수별(클래스·컴포넌트별) ms** 가 전부 나온다 — Insights 없이 "컴포넌트 틱" 안의 무명 ms 에 이름을 붙이는 가장 싼 방법. `stat SoldierLab`(우리 코드)의 짝. 이후 전후 비교 전부 이 방법.
② **전편 오독 정정 셋 [A]** — (a) `BlueprintUpdateAnimation` 68 calls 는 병사당 ABP 2개가 아니라 **GT 34 + 워커 34**(같은 스탯 스코프: `AnimInstance.cpp:795` · `AnimInstanceProxy.cpp:1354`), 실제 GT 이벤트 그래프 병사당 29~40 µs, PP ABP 는 None, `obj list` 82 = PIE 40 + 에디터 월드 40 + 프리뷰 2 (b) `KinematicBonesUpdateType=SkipAllBones` **채택 불가** — 피격 부위가 메시 바디 트레이스(`SoldierHealth.cpp:289-305`) (c) BeginPlay `SetVisibility(false)` 는 **스폰된 총 액터의 WeaponMesh**(노드 `K2Node_CallFunction_84`, self ← SpawnActor)에 걸리고 **캐릭터 WeaponMesh 가 보이는 총**; 그 AnimClass `ABP_Weap_Rifle` 은 본 0개 일치(`Skeleton.cpp:648`)라 죽은 참조 → None.
③ **새로 이름 붙은 비용**(37명 New_kadex_0811) — **GameplayCamera 1.60 + SpringArm 0.38 = 2.0 ms**(AI 40명 전원이 GASP 스탠드얼론 카메라를 매 틱 평가: `bAutoActivate`+`bRunStandaloneCameraSystem` CDO 상속 → `GameplayCameraComponentBase.cpp:552-568` · `:622-657`, `SpringArmComponent.cpp:197` 스윕) · 캐릭터 BP ReceiveTick 2.36 + PreCMC 0.94 · Cover 2.19 · Sight 0.86 · Engagement 0.42 · ScanTurn 0.35 · HeadAim 0.18 · CMC 1.86 · CharacterMesh0 1.61 · 숨은 총 메시 0.19+0.34 · `AWindSource` 0.95(MPC 11개 매 프레임) · titan TargetDetection ×3 1.2 · **PIE 전용** Landscape 5.7 / Slate 5 / 뷰포트 3. 병사 컴포넌트 38개(CDO 29 + PIE 카메라 프록시/프러스텀 6 + 3). **사격 발당 ≈ 1.1 ms**(투사체 스폰 0.57 + 총구 `SpawnSystemAttached` 0.38 + Launch 0.08 — 0.65 는 평균) · `SpawnSystemAttached` `bAutoDestroy=false` 로 **Niagara 컴포넌트 영구 누적 누수**(8정에 106개).
④ **수정 ✅ PIE** — 카메라: `BP_SoldierCharacter` BeginPlay 끝 `not IsPlayerControlled` → GameplayCamera · SpringArm · Camera(NotUsedByDefault) **`SetComponentTickEnabled(false)`**(Deactivate 아님 — P107; 관전 폰 follow 는 `SoldierObserverPawn.cpp:393-471` 이 자기 폰을 옮겨 병사 카메라 안 빌림) → 카메라 틱 0건, **World Tick 22.6 → 19.4/18.4** · ABP `SoldierCharacter_ABP`: `Update_CVarDrivenVariables`(cvar 7 문자열 조회 + `ComponentHasTag` 2)를 Initialize 1회 + `CVarPollSeconds` ≥ 1.0 마다(1 Hz) → GT 이벤트 그래프 29 → 22 µs, CharacterMesh0(아군 20) 0.834 → 0.616(함정: MCP `create_node` 가 GASP 원본 클래스의 동명 함수를 잡음 → `declaring_class` 지정) · `BP_AR4Rifle.WeaponMesh`(숨은 쪽) `OnlyTickPoseWhenRendered` 0.26 → 0.07.
⑤ **수정(측정 전)** — 캐릭터 BP: 변수 **`AnimBP`** 에 BeginPlay 캐스트 저장, `SetUseAllyAnimSet` BeginPlay 이동, Tick 캐스트 제거(디버그 `SetAxis` 7회 게이트는 리스크로 생략) · 총구: `BP_AR4Rifle` 상주 **`MuzzleFlashFX`**(MuzzlePoint 자식 · `bAutoActivate=false` · `NS_MuzzleFlash`), `Shoot` · `PlayShotCosmetics`(리플리케이션 세션 함수) 의 `SpawnSystemAttached` → `Activate(bReset)`, `BP_AK47Rifle` 상속 확인 — 누수 해결.
⑥ ⛔ **URO 폐기** — `bEnableUpdateRateOptimizations=true`(부모 CDO + 자식 2 CDO + 인스턴스 35 직접) → 검증 PIE **약 2분 만에 에디터 크래시** `!ParentBone.ContainsNaN() [BonePose.h:645] Pose[1] … -inf`, 직전 `[SoldierIdentity] … chest stand 1.17e18 cm` + Chaos bounds ensure 연발. 원인 추정 [B]: GASP MM / `OffsetRootBone` / `DeadBlending` 이 건너뛴 프레임을 상정 안 함 → **[C-165]**. 에디터가 죽어 못 되돌림 → `DefaultEngine.ini [ConsoleVariables] a.URO.Enable=0`(플래그 AND cvar, `SkinnedMeshComponent.cpp:1810`) → **재기동 뒤 플래그 false + cvar 삭제 [W107] 먼저**. 자동저장 `BP_SoldierCharacter_Auto1.uasset`(16:04) = URO 직전. 원칙 **P186**.
⑦ **C++ 4건 빌드 대기** — `AI/SoldierHealth.cpp` `TickInterval 0.1` + DrawDebugString 수명 = 간격 · `AI/SoldierComms.cpp` `TickInterval 0.1`(Suppression/Perception 은 DeltaTime 의존이라 그대로) · `Pose/SoldierHeadAimComponent.cpp` H off ∧ AI ∧ 상태 정지면 조기 반환(언와인드 중 계속, 0.18 ms/35명) · `Squad/SoldierSquadSubsystem.cpp` · `Squad/SoldierZone.cpp` `SCOPE_CYCLE_COUNTER` **[W103] 해결** · `titan_example/Environment/WindSource.h/.cpp` **`TargetPushIntervalSeconds 0.05`**(20 Hz, 0.95 → ≈ 0.05; 헤더 UPROPERTY = 정식 빌드).
⑧ **남은 ms**(35명, 카메라 뒤, World Tick ≈ 20) — 사격 스폰 2.2 · 캐릭터 BP 2.8 · SoldierLab 3.5 · 애니 GT 2.6 · CMC 1.8 · 물리 1.4 · 투사체 비행 0.7 · 틱 오버헤드 0.95(틱 함수 ≈ 850) · Niagara 0.5 · 기타 1.4. **현실적 바닥 ≈ 13~14**(풀링 [W109] −2 · BP 틱 C++ [W108] −1.8 · [W102] −0.9 · 애니 −1.2(URO 폐기로 방법 재검토) · 솎기 −0.3). 조율: 리플리케이션 세션이 같은 시간에 Engagement·ScanTurn·Projectile·Detectable·`BP_SoldierCharacter` Tick·`BP_AR4Rifle` 편집 — 파일 겹침 없이(BeginPlay 이쪽 · Tick 저쪽).
**후속(16:55~17:05, 같은 문서 10~13절)** — 후속 ① ✅ **[W107] 해결** — 사용자 정식 빌드(새 DLL 16:55) + 재기동 뒤 MCP 로 `bEnableUpdateRateOptimizations=false` 를 CDO 3(`BP_SoldierCharacter`·`_Friendly`·`_Hostile`) + `New_kadex_0811` 인스턴스 40(로드 시 true) 에, BP 3 저장(레벨은 델타 없어 저장 안 함), `a.URO.Enable=0` 줄 제거(ini 가 원본과 같아져 P4 revert). C++ 4건 빌드됨. 후속 ② **실측**(`L_SoldierScenario` 35명, 교전 2프레임, World Tick **18.3 / 20.3** — 두 번째는 Cover 2.7 + 사격 스폰 1.1 겹침): 카메라 틱 **0** · `SoldierHeadAim` · `SoldierHealth`/`Comms` 틱 목록에서 **소멸** · `Shoot` 발당 **1.1 → 0.94**(`SpawnSystemAttached` 0건; 남은 0.94 = 투사체 `BeginDeferredActorSpawnFromClass` 0.65 + `FinishSpawningActor` 0.18 + `LaunchFrom` 0.11) · 캐릭터 BP ReceiveTick(아군 20) 1.21 **변화 없음** · `WindSource` 는 이 레벨에 없어 미측정 · 에러/크래시 없음 → 이 배치의 실제 ms 는 **카메라 2.0 + HeadAim 0.2 + 총구 0.4/발**. **레이트레이스 소유자별**(`SceneQueryTotal` 부모 귀속, 1,030~1,119회/프레임 2.6~3.0 ms): Cover Candidate 367~459 · Begin Sweep 144~240 · **Engagement 182~188회 0.46**(`IsShotBlockedByWorld` 매 틱 ≈ 5.5회/병사, 호출마다 병사 35명 `AddIgnoredActor` — `SoldierEngagement.cpp:371-417`) · Sight 102~105 · CMC `FindFloor` 99~105 0.30(`bAlwaysCheckFloor=true` — 정지 병사도) · Cover Here/Route 70~120. 후속 ③ ★ **투사체 발견 [A]** — `ASoldierProjectile` 은 풀 설계(`SoldierProjectile.h:7` · `Deactivate` 는 숨김만, Destroy 없음)인데 `BP_AR4Rifle.Shoot`/`PlayShotCosmetics`·도탄 `Multicast_LaunchRicochet`(`SoldierEngagement.cpp:246-268`)이 발마다 `SpawnActor` → 비행 21발에 `TracerTrailComponent` **80개**(영구 누적, 총구 Niagara 누수와 같은 모양); `BP_RifleProjectile` 빈 `EventTick`(30발 0.25 ms); `CollisionComponent` `QueryAndPhysics`(QueryOnly 가능). **남은 후보 순서(사용자 승인)**: ① 투사체 풀 `USoldierProjectilePoolSubsystem` + 빈 EventTick 삭제 + QueryOnly(−0.8/발) **[W109] 지금 착수** → ② Engagement 레이 채널응답·캐시·PlanAperture 저빈도(−0.3~0.4) **[W110]** → ③ CMC `bAlwaysCheckFloor`·`bEnablePhysicsInteraction` false(−0.2) **[W111]** → ④ 캐릭터 BP 틱 → C++ `USoldierAIBridgeComponent`(−1.5~2) [W108], 2PC 검증(내일) 뒤 → ⑤ Cover 후보 2틱·Sight 격틱(−0.5~0.8) [W102] → ⑥ 컴포넌트 틱 통합 **[W112]** · 총 액터 메시 제거 **[W113]**. 애니 GT 는 `Update_PropertiesFromCharacter` 프로퍼티 액세스화가 남은 길, titan `TargetDetection` ×3 1.2 미조사. 리플리케이션 세션 완료 — 겹침 조율 끝.

★★ **2026-09-18 밤 이후 ~ 09-21 — 분대 스코프 필드 · 엣지 전진 · 사격 콘 · 섀도우 감축 (AI 세션)** → `ai/2026-09-21_per_squad_field_edge_advance_fire_model.md`.
① **필드 = 분대의 지식**(P179) — `FScope` = 진영 × 분대 슬롯(`MaxSquadsPerFaction 3`, 처음 말하는 순서, 넘치면 슬롯 0 공유 + 분대당 1회 경고), 공개 API 전부 `Who`, 호라이즌만 세계 공유, 무전 `ReceiveSharedRecord` → 받는 분대 필드에 **관측 시각**으로 `ReportSighting`(남의 분대 목격은 무전이 닿았을 때만, 흐리게, 늦게). **`bTakesSquadOrders=false`(UGV 표적 Identity)는 스코프·앵커·관찰 대상 아님** — 첫 Friendly "분대"로 슬롯을 먹고 UGV 조종 중 오버레이가 UGV 를 따라가던 원인. cvar `Debug.Field.Squad`(슬롯 고정)·`.Centre 1`(분대원 전원 주위 링 — UGV 에서 분대 보기), 헤더 `HOSTILE/1`. `L_SoldierScenario` 아군 4 → **3분대(7/7/6)**(`_16`·`_17` → 1, `_18`·`_19` → 2, `_20` → 3) — DT 의 분대 '4' 참조는 [W94]. ✅ PIE.
② **엣지 전진**(P176·P177) — 사용자: "파이 자르기는 정보 굶주림 대 위험에서 **창발**해야 한다, 호도 타이머도 새 트레이스도 안 된다". 기각 둘: 손으로 그린 호 + 머무름(잠깐 있었음 — 상수뿐, 치운 복도에서도 의식, 방과 복도가 같은 호) · 후보마다 광선 부채꼴(걸음 하나에 수백 트레이스, 이미 있는 정보를 다시 잼). 채택: 콘 스윕 광선을 슬롯별로 보관(`GetSweepRays`) → 옆은 멀리 가는데 짧게 멈춘 광선 = **엣지** → 걸음이 눈-엣지 선을 돌리며 여는 쐐기를 **`GetWedgePresence`**(경계도 × m², 구운 호라이즌으로 가림, 트레이스 0)로 값 매김 → 예산 `StepPresenceBudget 6` 안에서 가장 멀리 가는 걸음(60 cm × 1..4, ±105°) → 연 조각의 글로우가 `AnalyzedPresence 1` 아래로(≤ 3 s) 내려가면 다음. 엣지 가까우면 같은 걸음이 많이 열어 발이 밀려난다 = **호는 예산이 만든다**; 팀원이 치운 땅은 0 을 열어 곧장 간다; 방은 짧고 복도는 길다. 접촉 → 그 자리 정지 + 걸어온 자리 10 s 엄폐 후보. 교전: 쐐기 조준 + 벽 쪽 린 0.7, `bWantsToAim = 접촉 ∥ 전진`, Walk, `UrgencyLookPeek`. 코너 멈춤(`UpdateCornerPause`·`CornerStopCm`·`CornerPauseSeconds`) 삭제. **빌드됨(CL 500) · PIE 미보고 → [W95]**.
③ **AI 가 소유하는 사격 콘**(P178) — 무기 고정 3° 로는 40 m 완벽 위치에 2% 명중이라 조준 게이트 무의미, 선회 직후·버스트 중·달리는 중이 같은 콘. `GetShotSpreadDegrees()` = `0.8°`(← 3) × `(1 + 6 × v/600)`(← 2) × 자세(`Lean 1.5`/`Blind 15`) × (1+반동) + **흔들림**(선회 2.5° → 0.4 s 지수 정착, 발마다 +0.6°, 이동 바닥 1.5°). 조준 게이트: 기록 반경 ≤ 90 cm 이면 지금 콘 → `Aimed` / 정착하면 → **`Settling`**(40 m ≈ 0.65 s · 60 m ≈ 1.5 s · 10 m 0 [B]) / 예비 → `Suppressive`. **버스트** 2~5발(제압 5) · 0.5 s × (1 ± 0.35) · 이름 시드 → **`Pacing`**. `[Engage]` 꼬리 `cone wobble burst next`. ✅ PIE 거동. ⚠ **무기 BP 가 아직 고정 콘으로 쏘고 `BP_SoldierCharacter` 가 `HasContact()` 를 aim 모드에 먹인다 → [W93]** — 그 전에는 명중률·린 판정 불가.
④ **섀도우 수요 감축** ~~[B]~~ → **✅ 빌드·PIE(같은 날 늦게, [W96] 해결)**(P180) — 비용 줄 실측(감축 전): `shadows 0.03 ms · 0 waiting · 96 alive` → ms 는 문제 아니고 **96 = 6 스코프 × `MaxLights 16` 상한**이 문제([W92]). 코드: `FLight::ShadowEye/ShadowCastTime`, 따라가는 라이트는 **한 셀 ∧ 0.5 s** 뒤에만 재캐스트(보이는 적은 엄폐 스윕의 실제 눈이 잰다), 얼면 즉시, 반 셀 안 다른 분대 라이트는 **riders** 로 한 벌의 트레이스에 동승. 감축 뒤 재측정 **동일**(0.03 ms · 0 waiting · 96 alive — 이 장면은 원래 예산이 남는다; ⚠ 96 은 alive 지 waiting 이 아니다). 개별 판정은 [C-161] "눈에 띄면". 문서 10절.
⑤ **오버레이 노출 보정 ✅ PIE**(P181) → **`ai/2026-09-21_debug_overlay_exposure.md`** — EV10 고정 노출 레벨에서 `SoldierLab.Debug.*` 오버레이(와 엔진 내비메시 `P` 뷰)가 **전부 숯검정**: 디버그 프리미티브는 톤매퍼 **앞**이고 `DrawDebug*`·배처 `DrawMesh` 는 8-bit `FColor` 라 선형 1.0 위로 못 올린다. 기각: 레벨 라이팅/PP 건드리기(진단 도구 때문에 관찰 대상을 바꾸는 것) · 상수 배율(레벨·시간대마다 다름) · 엔진 `P` 뷰 수정(엔진 쪽, 사용자 "손대지 말 것" — 비목표). 채택: **뷰가 지난 프레임에 적용한 노출의 역수만큼 밝게** — `SoldierDebug::GetExposureScale`(`FSceneViewExtensionBase` 가 게임 뷰의 `GetLastEyeAdaptationExposure()` 를 매 프레임 읽음, 렌더 없음) · `Bright(FColor) → FLinearColor`(알파 제외) · 래퍼 `SoldierDebug::Line/Point/Sphere/Circle`(월드 라인 배처) · 신규 **`USoldierDebugMeshComponent`**(`AI/SoldierDebugMesh` — 배처 메시 경로를 선형 색으로, 같은 `DebugMeshMaterial`; 필드 사각형·라이트 링) · cvar **`SoldierLab.Debug.ExposureScale`**(0 자동 · 블룸 레벨은 조금 낮게 수동) · `Build.cs` + `RenderCore`/`RHI`. `AI/` 24곳 교체, `DrawDebugString` 그대로. **`Squad/` 9 · `Pose/` 7 · `Weapons/` 1 은 소유 세션 몫 → [W97]**(`Arrow` 래퍼 없음). 새 튜닝값 없음.

<details>
<summary>2026-09-18 밤 시점 머리글 (접힘)</summary>

2026-09-18 (밤) / **★ 오후: "적이 죽은 뒤 가만히 서 있는 아군" → 섹터 = 부채꼴 · 볼 곳 항상 · `GetAimPoint()/IsScanning()` 계약(포즈 세션 ScanTurn) · 순찰(낡은 조망 비용) · 로그로 잡은 이동 결함 셋(solid 셀 · 미지 = 열림 · 도달성/유예 · Hold 기울기) · CQB — 빌드·PIE "이제 정상적이다". 저녁: 얇은 엄폐 A/B/C ~~코드만, 빌드 전~~ → 밤 빌드·PIE "잘됨". ★ 밤 4차: 첫 빌드 로그의 코너 멈춤 루프 수정(굽이는 자리 · 재개 = 재발행) · 긴장도 + 걸음 `GetDesiredGait()`(조용한 경비는 걷는다) · 포즈 급박도 `GetPoseUrgency()`(AI 는 목표 + 숫자 하나) → 포즈 층 3종이 소비, 전부 PIE "잘됨"** / 오전 블록(LOD·오버레이 v2·부정 증거)의 미빌드분도 오후 빌드에 들어갔다. → `ai/2026-09-18_patrol_scan_and_move_robustness.md`(**12~17절** = 밤) · 시스템 문서 **20~24절** · 잠입 문서 **13절** · 분대 설계 문서 **10절**. ~~ScanTurn 연동은 미확인([C-152])~~ → **포즈 세션 PIE 확인**(아래 블록).

</details>

**병행 포즈 세션(09-17~18): AI 포즈 층 3종(ScanTurn · GaitBridge · PoseSmoother) + 관전 폰 휠 비행 속도·slomo 무관·롤 잔류 수정 — 전부 사용자 PIE 확인. 스무더의 "급하면 1프레임 스냅"은 `RampAxisTo` rate 0 = 즉시(P167)가 원인, 0.0001 로 해결("해결완료").** → `animation/2026-09-18_ai_pose_layer_scanturn_gait_smoother.md`.

★ **2026-09-18 밤 — 순찰·스캔·이동 강건성 4차: 코너 멈춤 루프 · 긴장도/걸음 · 포즈 급박도 (AI 세션, 계약 발행 측)** (원칙 **P172~P175**, 값 [C-155]~[C-156], **[W89] · [W90] 해결**, [C-153] 거동 확인) → `ai/2026-09-18_patrol_scan_and_move_robustness.md` **12~17절** · 시스템 문서 **24절** · 분대 설계 문서 **10절**(계약 3개 추가).
① **A/B/C 첫 빌드 로그(13:15, `L_SoldierTest` 명령 있음)** — Enemy_A `HERE o4.21` 60 s 동일, `MOVE` **0.82 s 마다**: 스폰 옆 굽이 → `PauseMove` 0.8 s → 재개 순간 속도 0 ∧ 유예 0.75 s 이미 만료 → "정지" → 재결정 → 새 경로 = 같은 굽이가 **새 인덱스** → 다시 멈춤. 수정: 굽이 기억을 **자리**로(`LastPausedCornerLocation`, `CornerStopCm` 반경, P173) + 재개 시 **`LastMoveIssuedSeconds = Now`**(P174) + **눈이 나타나면 즉시 재개**. 문서 세션 지적 셋도 코드로: `RejectedCandidates` 만료 정리([W90]) · `IsScanning()` 섹터만 있어도 true([W89]) · `GetExposure` 미지 = `UnknownPresence × AmbientWeight`(0 아님). ✅ A/B/C + 이것까지 PIE "잘됨".
② **조용한 병사가 어디를 가든 조깅** → **긴장도** `GetTension()` = 알람(접촉 ∥ 제압 ∥ 사선 거부 ∥ 적 기록 총성 1 s 안) 뒤 `0.5^(age/TensionHalfLifeSeconds 20)` · **걸음** `GetDesiredGait()` = Sprint(`WantsToSprint`) / Jog(접촉 ∨ 긴장 ≥ `JogTension 0.3`, **Cautious ∧ 무접촉 = Walk**) / Walk. 소비 = 포즈 세션 `GaitBridge`(Walk → GASP `WantsToWalk`, Sprint 는 옛 BP 브리지, Jog = 기본 Run). 긴장도 자체는 안 섞는다(GASP gait 이산 · AI 입력 크기 1). ✅ "잘됨"(P175).
③ **자세 전환이 기계적** → 논의 결론 **"AI 는 목표 4개 + 급박도 숫자 하나, 움직임(가속·순항·정착·비대칭·목표 변경 시 속도 연속)은 포즈 층"**(P172 — 교전은 이미 실제 자세를 되읽어 지연이 설계상 허용 · 축별 물리는 몸의 것 · 목표는 바뀔 때만 복제 · 결정 이산/움직임 연속). `GetPoseUrgency()` = `Urgency*` 7값(Idle 0.15 · ContactIdle 0.35 · LookPeek 0.3 · ShootPeek 0.6 · Retreat 0.7/0.6 s · Reload 1.0) max + 제압. 에디터 분할: 상황별 급박도는 교전 컴포넌트, 축별 속도는 스무더. `[Engage]` 로그 끝 `tension gait urg`.
④ **스무더 뒤 "급할 때만 1프레임 점프"** — 이쪽 확인: AI 층은 **어떤 축도 쓰지 않는다**(리플렉션 읽기뿐), 목표가 한 틱에 0→1 로 뛰는 것은 설계(제압 스파이크·재장전 덕·조리개 전환). 후보 셋(BP 램프 rate 0 = 즉시 / 옛 브리지 직접 쓰기 / GASP `Crouch()` 문턱) 중 **어느 것인지 이 세션은 확정 못 함**; 사용자 "구현 완료" → 포즈 세션이 첫 번째로 확정(P167, `RampAxisTo` rate ≤ 0 = Target 반환). 새 W 없음.
⑤ **잠입 접근 현황 답** — 개인 층 완성(사전값·앰비언트·필드 후보 · 기하 자세 + `MinStance` · 조용하면 걷기 · 눈이 발을 이끈다 · 코너 멈춤 · 도착 머무름 · 콘 스윕 · Rush 가 전부 품). 시나리오는 이미 `MoveTo r1200 roe=hold spd=cautious agg=0.30`. **우리 몫 아님**: 내비 가중 경로 · 걸음 → GASP(됐음) · `MinStance` titan DT([W84]) · 나무 Sight 콜리전([W88]).

★ **2026-09-17~18 — AI 포즈 층 3종 · 관전 폰 추기 (병행 포즈 세션)** (원칙 **P167~P171**, 값 [C-154], 작업 [W91], **[C-152] 포즈 측 확인**) → `animation/2026-09-18_ai_pose_layer_scanturn_gait_smoother.md` · `ai/2026-09-14_cover_frame_fix_and_observer.md` **5절**.
① **계약 소비** — AI 세션이 발행한 `GetAimPoint()/IsScanning()` · `GetDesiredGait()` · `GetPoseUrgency()` 를 `Source/SoldierLab/Pose/` 컴포넌트 셋이 읽는다, **전부 AI 전용**(`IsPlayerControlled()` → return): **`SoldierScanTurnComponent`**(총 내림 ∧ 정지 ∧ 스캔/접촉 → 캡슐 yaw 를 조준 방위로, 20°/5° 히스테리시스, 180°/s, 메시는 GASP TIP) · **`SoldierGaitBridgeComponent`**(`Walk` → `CharacterInputState.WantsToWalk` 리플렉션 매 틱 — BP 구조체 필드 GUID 접미사 P170, Sprint 는 BP 소유) · **`SoldierPoseSmootherComponent`**(stance/lean/BF-H/BF-V 사다리꼴, Stance 1.6/0.9/5 앉는 쪽 빠름, urgency ×0.5~1.6, 목표 변경 시 속도 연속, BP 상태+목표 동시 쓰기 + 액터 틱 선행 P168). `BP_SoldierCharacter` 에 `AC_SoldierScanTurn`/`AC_SoldierGaitBridge`/`AC_SoldierPoseSmoother`.
② **"급하면 1프레임 스냅"** — `ext` 0(제3의 손 없음)인데 급할 때 stance 가 한 프레임에 뜀. 원인 = BP 램프 rate 를 **0** 으로 얼린 것: `RampAxisTo` 는 rate ≤ 0 이면 **Target 을 그대로 반환**(`SoldierAxisLibrary.cpp:19-22`), BP 가 `AITarget*` 를 raw 로 다시 덮은 뒤 램프가 raw 를 돌려줌. `FrozenRate 0.0001` + 린 램프 리터럴 핀 0.0001. ✅ "해결완료". 남는 이산 전환은 GASP 크라우치 DB(문턱 0.5) [W91] — 지금은 안 띔.
③ **H 는 AI 에 자동으로 안 켜진다** — 09-17 "AI 도 볼 곳으로 몸을" 을 `bEnableForAI` 로 했다가 사용자 되돌림("H 는 1인칭 부가 기능, 노출 금지"). 답이 ①의 ScanTurn(P171). `HeadAim` 순변경은 주석뿐.
④ **관전 폰** — 자유 비행 **휠 = 비행 속도**(`FlySpeedCms 1200` ×/÷1.25, 추적 중엔 거리 그대로) · `bIgnoreTimeDilation`(`CustomTimeDilation = 1/배속`, "slomo 로 낮추면 카메라까지 느려져 불편") · **롤 잔류**(F → T → T/해제 뒤 카메라 기울어짐 → 컨트롤 회전 동기화는 yaw·pitch 만 + 전환 시 롤 0, P169). ✅ "잘됐음".
⑤ Perforce: `Pose/` 8파일(HeadAim 2 + 신규 6) · `Observer/` 2 · `Camera/` 2 · `BP_SoldierCharacter.uasset` — CL 469 이후 **미제출**.

★ **2026-09-18 오후~저녁 — 순찰 · 부채꼴 스캔 · 이동 강건성 · 얇은 엄폐** (원칙 **P158~P166**, 값 [C-148]~[C-153], 작업 [W85]~[W90], **[W83] 해결 · [W84] 절반**).
① **사용자 보고** "적이 다 죽은 뒤 아군이 가만히, 보라 화살표가 몸과 반대, 5분 뒤 화살표 뒤집힘" → 원인 넷: (a) 존이 기본으로 주는 섹터를 교전 층이 **고정 방위**로 읽어 명령받은 병사의 스캔이 영영 안 돎 → **섹터 = 부채꼴**(필드 `GetMostExposedDirection`에 arc/편향 파라미터, 선택만 제한·적분은 그대로; 볼 곳 > 섹터 중심, P163) (b) 콘 스윕 광선 6° 간격이 20 m 밖에서 셀보다 넓어 사이 셀이 영영 안 비워짐 → `MarkClearAlongRay` **16 m 밖 3셀 폭 띠** (c) 볼 곳이 엄폐 층 "눈 0"에 게이트돼 죽은 적 기록이 ≈ 90 s 눈으로 남는 동안 교전 층(다른 문턱)과의 **틈** → 볼 곳은 **항상 계산**, 쓸지는 교전 층(P165) (d) 총 내린 idle에서 **몸이 안 돎** → 교전 층이 무접촉 분기에서도 `AimPoint` + `IsScanning()` 발행 → **포즈 세션이 `Pose/SoldierScanTurnComponent`**(정지 ∧ 총 내림 ∧ 스캔/접촉이면 캡슐을 180°/s로) 작성. `[Cover] watch=` 로그 추가.
② **순찰** — "적 정보 없는 존 방어는 계속 순찰해야 한다". 경로가 아니라 **비용**(P164): `GetStaleVantage(Foot)` = 앰비언트 적분의 방사체를 "안 본 지 얼마나"로, Hold/수비 반경 안 `+ PatrolWeight × (1 − vantage)`, 눈 0일 때만. 낡은 조망이 싸다 → 서서 본다 → 콘이 비운다 → 비싸진다 → 다음. 존·`ASoldierObjective` `PatrolWeight 1.0 / PatrolStaleSeconds 30`(`L_SoldierTest`엔 명령이 없어 목표에도 — 첫 시험 로그 `[Squad]` 0줄). ✅ PIE "순찰이 움직인다".
③ **로그 3건**(`titan_example.log` 08:52·09:51) — Friendly가 230 s 동안 60/s `MOVE @(+4,−492)` = **큐브 꼭대기 셀**: 굽기가 큐브 안에서 시작(전부 `bStartPenetrating`) → 가장 어두운 셀 → 내비 투영이 큐브 위 → 부분 경로로 벽까지 → 정지로 읽혀 매 틱 재발행. 답: **`FHorizon::bSolid`**(전방향 255 + 후보 제외, P160) · `MoveTo` 전 **`FindPathSync` 부분 경로 불허** + **`RejectCandidate` 30 s** + 무진전 거부(P161) · **`MoveGraceSeconds 0.75`**(P162). 2차: `x=−1824` **벽 꼭대기** REJECT 35건 — 안 구운 셀을 노출 0 = 완벽 은폐로 읽음 → **`GetExposureByStance` → bool, 미지 = 노출 1·은폐 불가**(P159). 3차: Enemy_A `hold@z1` 후보 전부 `o3.20` 200 s — Hold 비용이 밴드 끝 1.0에서 평평 → **밴드 밖에서도 계속 오름**(TaskCost·Objective 수비, P158). ✅ PIE — 적군 존 복귀, REJECT 루프 없음.
④ **CQB "눈이 발을 이끈다"** — 이동 중 볼 곳을 진행 방향으로(`WatchTravelBias 1`: 정후방 0·정면 ×2, 이동 중엔 섹터 arc 미적용) · 정지 Approach는 앵커 방향(`WatchApproachBias 0.5`) · 눈 0 도착 머무름 `ScanDwellSeconds 2`(엄폐 무관). ⚠ 오버레이 화살표는 arc만 반영.
⑤ 분대 세션 목록 답 — "숙여서 침투": 필드 자세가 이미 숨는 곳에서만 웅크림, "어디서나 더 낮게"는 **`MinStance`**(배정/명령, Rush 무시 — **titan DT/콘솔/존 미연결** → [W84] 절반) · "은폐 경로": 필드 후보·경로 앰비언트 가격으로 이미 있음 · 스프린트/Rush 불변. **[W83]** 들은 라이트를 부정 증거에서 제외 — 해결.
⑥ **저녁 A/B/C — 코드만, 빌드 전 [B]**: **A** 은폐 판정 = 자세마다 모든 눈, 보는 눈 활동도 비율 ≤ `HiddenGazeFraction 0.5`(나무는 한 방위만 가린다 — 눈 둘이면 어떤 나무도 엄폐가 아니었다, P166) · **B** 미세 위치 = 정지 ∧ 눈 있음일 때 발 주변 8 × `MicroStepCm 30` 후보(병합 우회) + `CoverAcceptanceRadiusCm 20`·`bStopOnOverlap=false`(반 몸 앞에서 멈추면 나무 **옆**) — 병사별 고해상도 복셀은 기각(눈이 움직이면 낡는다) · **C** 코너 멈춤 + 미리 보기 = 도달성 경로 보관 → 굽이(`CornerAngleDeg 35`, 5 m 안) 1.5 m 앞에서 `PauseMove` 0.8 s(눈 0 ∧ Rush 아님, 굽이당 1회), 볼 곳 편향은 굽이 너머 — **진짜 파이 자르기(경로 모양)는 아님** [W85]. 레벨: `L_SoldierTest`에 얇은 원기둥 필요, 실제 레벨 나무의 Sight 채널 확인 [W88].

<details>
<summary>2026-09-18 오전 시점 머리글 (접힘)</summary>

2026-09-18 / **★ 상황 필드 2일차 — 3단계 PIE 확인("딱 내가 원하는 그림") → LOD(밉 + 다중 앵커 퇴거) · 오버레이 v2 · 라이트 부정 증거. LOD 링·배처까지 PIE 확인, 후반부 5건 빌드 대기** / 09-17 밤의 상황 필드 위에 **메모리 상한**(병사에게서 먼 디테일을 부모에 잔여물로 접고 해제 — 후퇴전의 꼬리가 안 쌓임)과 **볼 만한 오버레이**(자체 배처, 클립맵 링, 불투명도 = 신선도)를 얹었고, "직접 가서 봤는데 없으면?"에 대한 답(더 빨리 잊되 지우지 않음)을 넣었다. → `ai/2026-09-17_situation_field_lighting_model.md` **16~18절** · `ai/2026-09-17_infiltration_and_unknown_ground.md`(4·6절 추가). **병행 세션: 분대 명령 층이 빌드되어 시험 레벨 `L_SoldierScenario` 첫 PIE — 수정 6건, 완주 대기** → 아래 두 번째 ★ 블록.

</details>

★ **2026-09-18 오전 — 상황 필드 LOD · 오버레이 v2 · 부정 증거** (원칙 **P152~P157**, 값 [C-140]~[C-143], 작업 [W79]~[W83], [W78] 해결). **오후 추기**: 아래 "빌드 전 [B]" 표시분(대칭 캡·라이트 v2·헤더 2줄·파랑끼·부정 증거)은 **오후 빌드에 포함됐다**(항목별 확인은 안 함) · [W83]은 오후에 코드로 해결.
① **3단계 검증** ✅ 빌드·PIE — 호라이즌/앰비언트·오버레이가 "딱 내가 원하는 그림". 문서 세션 지적으로 섀도우의 "셀 지면 110 cm 아래까지 웅크려도 보임" 판정을 **`GroundSlackCm 40`**으로 고침(그 아래는 캐시 오차지 사선이 아니다). 잔존 주석 [W78] 정리.
② **LOD 결정** — 사용자 "복셀 GI처럼 클립맵을?" → 분석: 필드는 요구 주도·희소라 **계산은 레벨 크기와 무관**, 커지는 것은 **메모리 ∝ 지나간 면적**(해제 코드가 없었다)과 굽기 ∝ 경로. 시나리오(적군이 큰 맵을 후퇴하며 싸움)에서 진짜 문제는 **꼬리**(P152). 진영엔 관찰자가 15명이라 클립맵 중심이 없다 → **밉**(레벨 1+는 레벨 0의 집계: 경계도 max · lit 평균 · 트임 평균, `bDirty`로만 갱신, 아무도 직접 안 씀) **+ 다중 앵커 퇴거**(병사 아무나에게서 `DetailRadiusCm 8000 × 4^L` 밖 레벨 L을 부모에 **잔여물**로 접고 해제, 2 s 스냅샷 1024/틱, 호라이즌은 양 진영 앵커 기준) — 오버레이만 진짜 클립맵(P153). `LevelCount 1 → 3`, `SampleFinest` 읽기, 섀도우는 앵커 80 m 안 라이트만, `SoldierLab.Field.CellSizeCm` cvar. ✅ 빌드·PIE — 링이 거리에 따라 거칠어짐, "기능적으로는 아주 잘 작동".
③ **오버레이 v2** — 사용자 보고 "4000 cm에서 깜빡이고 fps 떨어짐". 원인: 수명 있는 `DrawDebugMesh`는 월드 persistent batcher로 가서 **매 프레임 늙히고 렌더 상태 재생성** + 30 fps에서 0.133 s 간격 vs 0.12 s 수명 = 빈 프레임(P155). 답: 서브시스템이 `ULineBatchComponent`를 소유, 0.1 s마다 **한 틱에 flush + refill**, 색 8단계 양자화로 **색별 메시 1개**, `Level −1` 클립맵 링(25 m/100 m/12000), **불투명도 = 신선도**(사용자: 링이 있으니 크기는 레벨), 거친 셀은 평평한 사각형. ✅ 빌드·PIE. 이어서 **50 cm 셀 시험이 캡 버그를 드러냄**(X 순 걷기라 −X 반쪽만 — 사용자 발견) → 링 면적을 예산과 비교해 **대칭 축소** + CAPPED(P154), `MaxDebugCells 6000` · 미굽기 셀 **파랑끼**(모름 ≠ 안전) · 헤더 2줄(`DescribeGround` — 관찰 병사 발밑 셀의 direct/ambient/자세/presence/open/seen) · 볼 곳 **보라 화살표 2.5 m** · 라이트 불투명도 = 밝기, **초록 봄/호박 들음**, 흰 점 = 그림자 미완 — **여기부터 빌드 전 [B]**.
④ **부정 증거** — "직접 가서 봤는데 없으면 사라지나?" → 아니오였다. 이제 `MarkClearAlongRay`가 지나간 **얼린** 라이트에 빈 채로 본 **시간**을 적립(`ClearViewSeconds`), 밝기에 `0.5^(t/ClearViewHalfLifeSeconds 4)` 추가. **삭제 아님**(웅크렸거나 2 m 옆일 수 있다, P156). 다시 보이면 0. **[B]**. ⚠ 문서 세션 발견: 주석은 들은 라이트를 제외한다지만 코드는 안 거르고 넓은 반경 때문에 **더 잘 걸린다** → [W83] 사용자 결정.
⑤ 채널 4 질문 — 앰비언트는 채널 3 빨강에 **포함**돼 있고 4는 그 절반만 따로 보는 디버그 뷰.

★ **2026-09-17 밤~18 — 분대 명령 층 2일차 (병행 세션): 빌드 → 시험 레벨 `L_SoldierScenario` 첫 PIE → 수정 6건** (판정 [C-144]~[C-147], 결정 [Q50], 작업 [W84]) → `squad/2026-09-18_squad_layer_fixes_quota_engage_range.md` · `../level_new_kadex_0811/2026-09-18_soldierlab_three_stage_test_level.md` · `animation/prototypes/2026-09-18_tick_cycle_warning_charmovecomp.md`.
① **시험 레벨** — 적 15(분대 1/2/3)·아군 20(1~4)·UGV_0901·트럭, `ASoldierZone` 4개, `ScenarioConfig` Demo 자동 시작(`bDemoForceCommandPostAutoFire=false` · `bSoldierLabHostilesStartHidden=false`), DT `DT_ScenarioSteps_SquadThreeStage` **13행**(드론·Retarget·HoldFleeingFire 제거, 적/아군은 `IssueSquadOrder`). ⚠ Z0 가 아군에서 54 m 라 1차에 아군이 낀다 → [Q50].
② **첫 PIE 가 드러낸 것과 답** — (a) 병사가 링 안쪽 띠·링 밖 4.5 m 에 정착, 명령 미완료 → **도착선 한 숫자**(`ASoldierZone::ArrivalFraction` → 배정, `ArrivalInsetFraction` 삭제) + Approach 비용 계단 `1 + 거리/scale`. (b) 존이 안 보여 반경을 못 잡음 → **에디터 전용 구 3개 + 섹터 화살표 + 스프라이트**. (c) UGV/트럭이 `Friendly/(none)` 분대원으로 → `bTakesSquadOrders=false`. (d) 적 4명이 스폰 지점에서 92 m 밖 UGV 를 쏴 t≈7 s 에 교전 개시 → **`EngageRangeCm`**(ROE 사거리, DT 4000). (e) 구 정원제·대타 → **`ReinforceSquads` + DT `Quota`**(`SquadId` 영구 편입). (f) 트리거 `EnemyFireStarted` · `EnemyNearFriendlySoldiers`. (g) RCWS 청각이 SoldierLab 총성을 못 듣던 것 → `OnGunshot` → 브리지 → `ReportGunfire`.
③ **`LogTick … would form a cycle` 초당 600줄** — 09-12 의 `AddTickPrerequisiteActor(self)` 가 엔진 `bTickBeforeOwner` 엣지와 순환, 우리 엣지가 매 프레임 버려지고 있었다(= 09-12 5.3절의 "보장"은 보장이 아니었다). CMC `Tick Before Owner=false`. **서명 시험 대기 [C-146]**. 총구 xyz 디버그 축(`DrawDebugCoordinateSystem`) exec 끊음.
④ 사격 게이트 정리(believed → ROE → EngageRange → worth ≈95 m → 회계 → aperture → onTarget)와 "접근 중 서서 간다" 진단 → [W84] `MinStance` 제안(미구현, 다른 세션이 Engagement 편집 중).
⚠ Perforce: `ScenarioStepTypes.h` · `ScenarioStateSubsystem.{h,cpp}` 는 `user2` 도 체크아웃 — 제출 시 머지. MCP 함정 4건(부분 구조체 쓰기 리셋 · 구조체 배열 `[]` 후 통째 · PIE 중 배치 거부 · 읽기 전용 save false)은 시험 레벨 문서 6절.

### 남은 일 (2026-09-21 기준)

```
0. ★ BP 배선 둘 [W93] — 무기 산포 = GetShotSpreadDegrees() · BP_SoldierCharacter aim 모드 = WantsToAim()(HasContact 대신) — 사용자 BP 작업, [C-158][C-159] 판정의 전제
1. ★ 엣지 전진 PIE 판정 [W95] — `SoldierLab.Debug.Cover.Log 1` 의 `ADVANCE begins/step/ends` + 시안 핀/쐐기 (재료 [C-158]) — 걸음 논리는 [W93] 전에도 판정 가능, 린은 뒤
2. ~~★ 섀도우 감축 정식 빌드 → 비용 줄 재측정 [W96]~~ ✅ 같은 날(재측정 동일 0.03 ms · 0 waiting · 96 alive) → 남은 것: MaxLights 16 상한 [W92] (헤더 1줄 followed+frozen 으로 누가 차지했나 먼저) · [C-161] 개별 판정은 눈에 띄면
2b. 오버레이 이관 [W97] — Squad/ · Pose/ · Weapons/ 의 DrawDebug* → SoldierDebug::* (각 폴더 세션, Arrow 래퍼 하나 추가) — 그 전까지 EV10 레벨에서 존 링·ScanTurn·HeadAim 화살표는 검다
2c. ★ 성능 인계 (World Tick 22.6 ms, 게임 스레드 바운드 — ms 큰 순): [W98] 포즈(ABP 스레드-세이프 3.47 · 키네마틱 본 0.97 · URO) → [W99] 캐릭터 BP(컴포넌트 23/병사 · 오버랩 · 틱 2.5) → [W100] 투사체 풀링 0.65 · [W101] OnHit 58 ms 히치 → 그 뒤 [W102] 이 층 ≈ 1 ms · [W103] 분대 스코프 두 줄. 재측정은 Standalone · 로깅 off 로(P182). 박자 값 [C-162] 는 눈에 띄면
3. 분대 필드 거동 [C-157] — Debug.Field.Squad 로 두 분대 번갈아: 남의 목격은 무전 뒤에만·흐리게 · DT 분대 '4' 참조 [W94](분대 세션)
4. 콘·정착 숫자 [C-159] · 버스트 박자 [C-160] — [W93] 뒤
5. 09-18 잔여: [C-153] 숫자(그 전 [W88] 원기둥) · [C-148][C-149][C-150] · [C-154][C-155][C-156] 눈에 띄면 · [W86][W87]
6. ★ 분대: **New_kadex_0811 2차 PIE [C-163]**(`Squad3Run` 뒤 3분대 `roe=hold`+스프린트·엄폐 홉 0 · UGV `타겟 BP_Soldier_Hostile_*` 0줄 · 아군 `tgt=` 에 3분대 없음 · 트럭 사격 → `Squad3Stand` → 트럭 응사 · `ScenarioComplete`; 도주 중 `ADVANCE begins` 찍히면 엣지 전진 게이트에 `IsBreakingContact()`) → 페이싱·문턱 [C-164] · 존 위치 [Q51](사용자) → ~~[W70]~~ ✅ → [W84] `MinStance` titan DT(연결 시 `bBreakContact` 면 바닥 건너뜀) · [W103] 분대 스코프 두 줄 · [W104] 드론 프레이밍 분대 판정 · [C-125] 2PC · 시험 레벨 쪽 [Q50] · [C-144][C-145][C-147] · [C-146] 은 그대로
7. 필드 09-18 잔여: [C-141][C-142] · [C-138][C-140][C-143](오버레이 끈 채) · USoldierFieldSettings 값(이제 46개) · [C-133][C-135] · [W75]
8. Perforce — CL 498·500 제출됨(사용자), ④ 작업 트리 미제출 · 포즈 세션분(CL 469 이후) 그대로
```

<details>
<summary>남은 일 (2026-09-18 밤 기준, 접힘)</summary>

```
0. ~~★ 빌드 → PIE: 저녁 A/B/C~~ ✅ 밤 "잘됨" — 남은 것은 숫자 판정 [C-153](미세 이동 반복 · HiddenGazeFraction 0.5 후함), 그 전에 [W88] L_SoldierTest 에 얇은 원기둥(사용자 배치)
   · ~~ScanTurn 연동 [C-152]~~ 포즈 세션 확인 ✅ · ~~[W89]~~ 코드로 닫음 · 스무더 축 속도값 [C-154] · 크라우치 DB "뚝" [W91] (눈에 띄면)
   · 밤 신설 값 — 긴장도·걸음 [C-155](반감 20 s · JogTension 0.3 — 알람 뒤 ≈ 35 s 에 걷기로 돌아오는가) · 급박도 [C-156](Urgency* 7, Retreat 0.6 s 가 내려오는 시간보다 짧지 않은가) — 눈에 띄면
   · 순찰 주기·범위 [C-148] · CQB 볼 곳 [C-149] · REJECT 반복 없음 [C-150]
1. ★ 분대: [Q50] Z0 위치 결정 → 시험 레벨 완주(체크리스트 8단계, 시험 레벨 문서 5절) — [C-144][C-145][C-147] + [C-122]~[C-124]
   → [C-146] 틱 순서 서명 시험(YawRate_Up=20 → BodyErr≠0) → [W84] MinStance titan 연결(DT/콘솔/존) · [W70] New_kadex_0811 재저작(나무 Sight 채널 [W88] 확인 먼저) · [C-125] 2PC
2. 오전 미빌드분 판정(오후 빌드에 포함됨): 대칭 캡(CellSizeCm 50 으로 링이 양쪽 대칭 + CAPPED) · 파랑끼 · 헤더 2줄 · 라이트 v2 · 부정 증거(얼린 핀이 정면 응시에 더 빨리 흐려지나) — [C-141][C-142]
3. 오버레이 끈 채 헤더 cells L0/L1/L2 가 후퇴전에서 병사 수 × ≈5천 안에 머무는가 — [C-138][C-140][C-143]; 링 바깥 깜빡임(쐈다 접힘)이 보이면 DetailRadiusCm ↑ [W80]
4. USoldierFieldSettings 값 잡기(43개, 헤더 2줄 보며) — UnknownPresence · HiddenThreshold · AmbientWeight · ContinuityWindowSeconds · ClearViewHalfLifeSeconds
5. 거동 판정 [C-133][C-135](eyes=0 문간→문간, [Cover] 로그) · [W75] 내리막(채널 0) · 시나리오 DT Cautious/섹터 발행 [W70]
6. 진짜 파이 자르기 [W85] · 얇은 엄폐 후속 [W86][W87] · ~~RejectedCandidates 정리 [W90]~~(밤 해결) — 낮음
7. 아래 09-17 블록의 미저장분·[W71]~[W73] 그대로 유효
8. Perforce 제출 — 포즈 세션분(Pose/ 8 · Observer/ 2 · Camera/ 2 · BP_SoldierCharacter.uasset, CL 469 이후)
```

</details>

<details>
<summary>2026-09-17 밤 시점 머리글 — 위험 지도 폐기 → 상황 필드 (접힘)</summary>

2026-09-17 (밤) / **★★ 위험 지도 폐기 → 상황 필드(조명 모델) · 잠입 거동 — 1·2단계 PIE 확인, 3단계 빌드 대기** / 진영별 위험 지도(`SoldierDangerMap`, 09-14)를 `p4 delete`하고 `USoldierSituationFieldSubsystem`으로 바꿨다. **위험도를 저장하지 않는다** — 목격이 점광원, 엄폐가 그림자, 안 본 땅(사전값 0.5)이 앰비언트. 적군이 지식 0으로 광장을 질주하던 것이 "안 본 곳은 적이 있다고 친다"로 바뀌었다. → `ai/2026-09-17_situation_field_lighting_model.md` · `ai/2026-09-17_infiltration_and_unknown_ground.md` · (오전) `ai/2026-09-17_threat_bonus_lane_denied_vehicle_heights.md`

★★ **2026-09-17 저녁~밤 — 상황 필드 · 잠입** (원칙 **P143~P151**, 값 [C-130]~[C-139], 작업 [W74]~[W78]).
① **오전 넷** ✅ 빌드·PIE("이제 잘 작동하는거 같아. 이상하게 멈춰있는 애들은 없긴했음"): 위협 보너스(나를 쏘는 놈 +0.6, `ReportThreatenedBy`) · 사선 거부 래치(`IsLaneDenied` 2 s → 엄폐 HERE +1.0, "15 s 전이 0" 해결) · 가치 히스테리시스 1.2(30 Hz 방아쇠 떨림) · 차량 바운즈 높이(눈 0.8/표적 0.5).
② **잠입 요구** — "문을 열고 들어가는 것이지 박차고 들어가는 것이 아니다." 진단: 노출이 **믿는 적**에 대해서만 정의돼 눈 0 = 노출 0. 답: `UnknownPresence 0.5`(**0이 아니라 0.5를 향해 낡음**, P149) + 콘 스윕(`SweepCone` 2줄/틱 — 사전값을 물리는 유일한 것) + 스프린트 규칙(`bUrgent` = 접촉∥제압∥사선 거부, `Cautious` 절대/`Rush` 항상, P150). ✅ PIE.
③ **붉은 얼룩 진단** — 사용자: "GI처럼 보인다, 점광원+그림자여야 한다." 구조적 원인 = 위험도가 **출처 없는 누적 버퍼**(P147). 결론: 위험도 = ∫ 경계도 × 가시성(렌더링 방정식) → **저장 1(경계도) + 정적 1(호라이즌) + 파생 1(위험도)**, 파생의 유효성은 타이머가 아니라 라이트 **세대**(P143). 라이트의 주인은 액터도 사람도 아닌 **목격**(P145) — 0.5 s 연속창 넘기면 얼고, 다음은 새 라이트, 왼쪽 벽은 켜진 채. ✅ 빌드·PIE("잘되는거같아").
④ **쓰기 정리** — 눈(목격·콘·빈 땅)과 귀(총성 0.75)만 쓴다. 엄폐 부채꼴·몸 밴드·경로·제압 근접탄은 **안 쓴다**(P144). 가상 관찰자(`AddUnknownWatchers`, 같은 날 도입)는 가설을 사실로 써서 **철회**.
⑤ **3단계 [B, 빌드 0회]** — 호라이즌 맵(8방향 × 2높이, 지연 굽기 8셀/틱, 진영 공유) + 앰비언트(호라이즌 × 확산 경계도) · `FindDarkestCells` → 엄폐 후보 6 · 눈 0 `EvaluatePosition`이 필드로 은폐/낮은 벽/공터 판정(`HiddenThreshold 0.25`) · 볼 곳 루프(가장 모르는 방향 조준 → 훑음 → 다음 방향, 스캔 패턴 없음) · 골든앵글 후보 회전 · `IsRushing()` 우회.
⑥ **오버레이** ✅ — `SoldierLab.Debug.Field 1`, **월드당 1회**(P146, `GetObservedSoldier`), 채널 3 = R 위험도/G 경계도(노랑 = 살상 지대, 검정 = 훑음), 4 = 앰비언트(보라), 크기 = 신선도, 라이트 핀(굵음 추적/얇음+링 얼림). 값은 **Project Settings → Game → SoldierLab Situation Field**(`USoldierFieldSettings`, 37개 전부 [C] — 사용자가 오버레이 보며 잡을 예정).
⑦ 부수: `LogSoldierAI` → `AI/SoldierLabLog` · `Build.cs` `DeveloperSettings` · `DangerHalfLifeSeconds` 삭제 · 잔존 주석 [W78].

### 남은 일 (2026-09-17 밤 기준)

```
1. ★ 3단계 빌드(새 UCLASS 없음 — Live Coding 가능성 있으나 헤더 UPROPERTY 추가라 정식 빌드 권장, P13/메모리) → PIE
   판정: 채널 4 보라가 광장/골목을 가르는가 · eyes=0 병사가 문간→문간으로 가는가([C-133][C-135]) · 왼쪽 벽 핀이 남는가([C-131])
2. 오버레이 보며 USoldierFieldSettings 값 잡기 — 특히 UnknownPresence · HiddenThreshold · AmbientWeight · ContinuityWindowSeconds
3. [W75] 내리막 과소 보고 확인(채널 0) · [C-138] fps
4. 시나리오 DT가 Cautious/섹터를 실제로 발행([W70]) — 잠입은 지금 "명령 없을 때의 기본값"으로만 나온다
5. 아래 저녁 블록(애님 세트·피격/사망 미저장분)은 그대로 유효
```

(1번은 09-18에 완료 — 위 머리글.)

</details>

<details>
<summary>2026-09-17 저녁 시점 머리글 — 애님 세트 분리 · 피격/사망 (접힘)</summary>

2026-09-17 (저녁) / **★ 애니메이션이 진영별 2벌로 갈렸다 · ★ 피격/사망이 실제로 화면에 나오기 시작했다** / 클립 247개를 통째로 복제해 `Animations_Enemy/`(`Enemy_`) · `Animations_Ally/`(`ALLY_`) 로 갈랐고(**스켈레톤·ABP 는 1벌 그대로**, `UseAllyAnimSet` 이 5축을 분기), 09-15 에 배선만 돼 있던 피격·사망이 **원인 셋**을 걷어내고 동작하기 시작했다. → `animation/prototypes/2026-09-17_ally_enemy_anim_set_split.md` · `ai/2026-09-17_hit_death_three_causes.md`

★ **2026-09-16~17 저녁 — 애님 세트 분리 · 피격/사망 3중 원인** (원칙 **P134~P141**).
① **세트 분리**: 스켈레톤을 가르는 안은 **기각**(ABP 1200노드·PSD 17·Chooser·AO·몽타주가 전부 두 벌이 되고 P76 의 이점이 사라진다) → **스켈레톤 1벌 + 애니메이션 2벌.** Advanced Copy(UE 5.8 에는 **메뉴가 없다**, 파이썬만 — P135)로 `Animations` + `PoseSearch` 를 **한 번에** 넘겨 PSD↔PSN↔클립·Chooser 까지 사본끼리 remap. 이름 변경은 MCP `AssetTools.move` 494회 실패 0 · 리다이렉터 잔존 0. 옛 `Animations/`·`PoseSearch/` 소멸.
② **런타임 분기 5축**: `UseAllyAnimSet`(ABP + 캐릭터, Instance Editable) 하나로 로코모션 61 · AO 2 · 총내림 델타 1 · BF 3 · 몽타주 2. 전부 컴파일·저장 완료.
③ **프리뷰 메시**: `PreviewSkeletalMesh` 는 MCP 로 못 읽지만 `SetPreviewSkeletalMesh` 가 BlueprintCallable — 적군 227개 지정 완료(아군은 사용자, **[C-129]**). ⚠ Preview Scene Settings 로 메시만 바꾸면 **총이 안 보인다**(에셋의 Preview Mesh 기준으로 한 번만 읽힘).
④ **피격/사망 — 원인 셋**: C++ 생성자의 콘텐츠 경로 하드코딩(배열이 전부 빔 + 옛 몽타주를 **루트셋에 박아** 폴더 삭제까지 막았다, **P134**) → 걷어내고 **BP 데이터**로 / 레벨 배치 인스턴스가 **빈 배열을 직렬화**(**P139**, 플레이어만 되고 AI 만 안 되던 이유) → 재배치 / Death 슬롯 **`FullBody` 가 ABP 에 없었다**(**P140**) → 몽타주 12장을 `DefaultSlot` 으로.
⑤ **사망은 순수 래그돌** — 몽타주를 켜면 **이중 낙하**(애니메이션이 눕히고 물리가 다시 눕힌다)라 `bPlayDeathMontage = false`(사용자 선택). 되살릴 때의 튜닝 **[C-128]**.
⑥ **피직스 에셋** — 자동 생성본이 무릎·팔꿈치를 반대로 꺾어 두 메시 다 `PA_UEFN_Mannequin` 으로 교체. 진영별 피팅 **[W72]** · 정량 판정 **[C-127]**.
⑦ 정정: `a.AnimNode.MotionMatching.**DebugDrawInfoVerbose**` 는 **기본값이 이미 true** 이고 실제 스위치는 **`DebugDrawInfo`** 다(P141) — 디자이너 가이드 5절 포함 여러 문서가 틀리게 안내하고 있었다. 그리고 `soldier_T` 의 `Rifle_Socket` 은 titan 잔재이고 **새 시스템에 필요한 캐릭터 메시 소켓은 `weapon_r` 하나뿐**이다.
⑧ 디자이너 배포본 `assets/2026-09-14_designer_guide.html` **전면 갱신**(경로 103개 + 0·2·4.1·5절 + 2.9절 신설) — **검수 기준은 적군 세트**임을 명시.

### ⚠ 미저장 / 체크아웃 대기 (사용자 몫)

세션 종료 시점에 **MCP 로 값은 들어갔으나 P4 읽기 전용이라 저장 실패**한 것들. **이 상태로 에디터를 재시작하면 값이 유실된다**(P136 — 실제로 한 번 겪었다):

```
Content/SoldierLab/Blueprints/BP_SoldierCharacter.uasset            (bPlayDeathMontage=false)
Content/SoldierLab/Blueprints/BP_Soldier_Friendly.uasset            (동)
Content/SoldierLab/Animations_Enemy/.../Enemy_AM_MM_Death_*.uasset   6개 (슬롯 DefaultSlot)
Content/SoldierLab/Animations_Ally/.../ALLY_AM_MM_Death_*.uasset     6개 (동)
Content/SoldierLab/Characters/Ally/soldier_T.uasset                 (PA_UEFN_Mannequin 할당)
Content/Soldiers/New_enemy_soldiers/NewFolder/new_enemy_T.uasset    (동)
Source/SoldierLab/AI/SoldierHealth.cpp   체크아웃됨 · 편집 완료 · 이미 빌드했고 그 후 추가 편집 없음
```

그리고 프리뷰 메시 227개의 저장분이 **Uncontrolled Changelist** 에 있다 → **[W73]**.

### 남은 일 (2026-09-17 저녁 기준)

```
1. 위 미저장분 저장 + 체크아웃 · [C-129] 아군 세트 프리뷰 메시
2. [W71]  AdditiveHitReact 슬롯 그룹 분리 — 사격이 피격 몽타주를 끊는 구조적 위험 (에디터 수작업)
3. [W72]  피직스 에셋 진영별 복제·바디 피팅 · [C-127] 래그돌 품질 정량 판정
4. 디자인팀이 적군 세트 수정 착수 — 그때까지 두 세트는 같은 내용의 복제본이다
5. (분대 명령 층은 그대로 대기) ↓ 아래 오후 블록
```

</details>

<details>
<summary>2026-09-17 오후 시점 머리글 (접힘)</summary>

2026-09-17 (오후) / **★ L0/L1 분대 명령 층 코드 완료 — 빌드·시험 레벨·PIE 검증 대기** / 시나리오가 SoldierLab 분대에 **제약**(구역·ROE·속도·공세성)만 주는 `Squad/` 3파일 + 개인 AI 훅 3곳 + titan 브리지 + `IssueSquadOrder` 이펙트. **적·아군 전원 SoldierLab로 교체 결정**(옛 titan 병사 폐기). → `squad/2026-09-17_command_layer_design.md`

★ **2026-09-17 오후 — 분대 명령 층 (L0/L1)** (`squad/2026-09-17_command_layer_design.md`, 시나리오 쪽 `../level_new_kadex_0811/2026-09-17_soldierlab_squad_scenario_link.md`).
사용자 결정: 옛 `EnemyCombatComponent`/`AllyFormationComponent` 안 씀 · 아군은 거점 방어(낮은 공세성, 가끔 사격) · 표적 선호 없음 · 3분대 3차 도주는 `HoldFire`로 은밀히 · 의존 `titan_example → SoldierLab` 한 방향 · 전투지는 분대당 `ASoldierZone`(15×6 마커 무시) · 새 시험 레벨 먼저.
구현 [B, 빌드 전]: `FSoldierAssignment`/`FSoldierSquadOrder`(`ESoldierROE` — `ReturnFireOnly` = 최근 3 s 안에 쏜 적에게만) · `USoldierSquadSubsystem`(분대 객체 없음, 등록부 (Faction, `SquadId`), 지연 출발, Approach→Hold, achieved) · Cover `TaskCost`가 Objective 대체 + `Aggression` 배율 + NavFilter · Engagement ROE 게이트(`Restrained` 의도)·섹터 조준·Speed↔스프린트 · Perception 화망 구역(`bOrderedArea`, 융합 안 함) · **P5 `HasAuthority` 게이트**(Cover/Engagement/Sight/Comms) · 브리지 `USoldierLabBridgeSubsystem`(런타임 `DetectableTargetComponent` 부착·사망 폴링→`SetIncapacitated`·차량에 `Identity(Friendly)`) · RCWS 발사→`BroadcastGunshot`, RCWS 탄→`ApplyAlongSegment` 제압 · 시나리오 `IssueSquadOrder`/`SquadOrderAchieved`/`AScenarioConfig::SquadZones`/콘솔 `titan.SquadOrder`.
⚠ **한 번도 안 돌렸다.** 새 UCLASS/UENUM 다수 — 에디터 닫고 정식 빌드(P13). Perforce 체크아웃 31파일(신규 7) 미제출, titan 8파일은 `user2`도 열어 둠.

### 남은 일 (2026-09-17 오후 기준, 우선순위)

```
1. [W55]  빌드 → 시험 레벨(사용자 생성: ScenarioConfig RunMode=Demo · EnemyCube 태그 · ASoldierZone · SquadId · SquadZones)
          → DT 저작(6.1절 매핑) → 로그 판정 [C-122]~[C-126]
2. [C-123] ★ 적군이 숲 엄폐에 숨어 시나리오가 안 진행되는가 — Aggression 다이얼
3. [C-125] 2PC 리플리케이션(P5 게이트 후 첫 검증)
4. [W70]  New_kadex_0811 재저작(적 15·아군 25 교체, 존, DT)  [W68][W69] 잔가지  [Q49] 아군 ROE/공세성
5. (이전) [C-108][C-109][C-121] · [C-102]~[C-107] · [C-83] · [W56] · [W53]
```

</details>

<details>
<summary>2026-09-17 오전 시점 머리글 (접힘)</summary>

2026-09-17 / **★ 분대 항 넷(자리 주장·사선 비우기·표적 분담·엄호 이동, 등록부만) · ★ 배운 죽음(시체를 보거나 무전으로 — 방송은 사용자 거부로 철회) · 블라인드 펄스 수정 · 부정 증거 · 시야 120 m · 두 점 시야(가슴→머리)** / 사용자 평가 "잘 됨". 병사가 죽고, 죽은 것을 **본 사람만** 알고 25 m 안에 전하며, 같은 그림자를 둘이 고르지 않고, 쏘는 동료가 있으면 그 적의 눈 아래를 건넌다. → `ai/2026-09-16_squad_terms_and_learned_death.md`

★ **2026-09-16~17 — 분대 항 · 배운 죽음 · 시야** (`ai/2026-09-16_squad_terms_and_learned_death.md`, 원칙 **P130~P133**).
S1~S4는 분대 객체·명령 없이 `USoldierRegistrySubsystem`만 읽어 비용식에 `q` 항을 얹은 것(값 [C-108]). **유령 표적**(사망 뒤 15~20 s 죽은 적 조준, 시야 콘이 유령을 향해 살아 있는 적을 못 봄)은 1차 "등록부 방송"이 **전지적이라 사용자 거부 → 철회**, 최종은 **눈(시체 확인 트레이스) + 입(`SoldierComms` 사망 보고, 같은 입·25 m·1.2 s, 사망만 릴레이)**. 블라인드 펄스(전이 693회)는 종료 조건을 시작 이유별로(P132). 부정 증거(보고 있는데 없음 → 확신 1.5 s 반감, [C-109]). 시야 60 → **120 m**([C-121] — 첫 9 s 무반응의 원인). **두 점 시야**: 가슴만 보고 가슴만 숨기던 대칭 맹점(P131) → 시야는 가슴→머리, 엄폐는 자세마다 둘 다.
Perforce: **CL 471 · 472 제출**(사용자). **두 점 시야 변경분(Identity/Sight/Cover/Perception/Engagement)은 미제출.**

### 남은 일 (2026-09-17 기준, 우선순위)

```
1. [W55]  ★ L0 분대 명령 층 — "특정 위치로 경계하며 이동 · 특정 위치 점령 후 방어" 같은 명령. 실제 월드 투입의 전제.
          지금은 목표 마커 하나 + 선호뿐. squad/drafts/ 명령 스키마와 맞춰 설계부터
2. [C-108] [C-109] [C-121] 값 실측 — 분대 항(겹침·MASKED·엄호 이동 동시 출발) · 부정 증거 반감기 · 시야 120 m 귀결
   [C-102]~[C-107] 이전 라운드 값도 여전히 미실측
3. Perforce 제출 — 두 점 시야 변경분(AI/ 5쌍 중 일부)
4. [C-83]  45명 성능 — 트레이스가 또 늘었다(시체·빈 땅·2점·부채꼴 눈 전부)
5. [W56]  거리 의존 발견 시간 — 120 m에서 즉시 보는 건 틀렸다; "알아채는 데 몇 초"가 거리에 따라
6. [W53]  "노는 병사" — 유령 표적·시야 거리·두 점 시야로 상당 부분 설명됨 [B]; 남는 것은 로그 집계로
```

</details>

<details>
<summary>2026-09-15 저녁 시점 머리글 (접힘)</summary>

2026-09-15 저녁 / **★ 체력·피격·사망 완료 · ★ 적군 메시(`new_enemy_T`) 교체 완료 · 왼손 IK 토글(기본 OFF) + 그립 런타임 산출 · 급선회 스냅/BF 머리 부풀기 해결 · 디자이너 가이드 완성** / 병사가 맞으면 움찔하고 죽으면 쓰러지며(아군 무적), 아군·적군 외형이 전부 교체됐고 애니메이션 층은 디자인팀 FK 검수 대기 상태. AI 는 2라운드 거동(위험 지도 · 노출 회계), 사용자 평가 "지금까지는 가장 좋네". 값은 전부 [C].

★ **2026-09-14~15 — 애니메이션 정리 라운드** (`animation/prototypes/2026-09-15_sharp_turn_pop_bf_head_scale_and_weapon_socket.md` · `animation/prototypes/2026-09-14_enemy_mesh_on_mannequin_skeleton.md`, 같은 날 체력 세션과 **같은 ABP/BP 를 동시에 편집**했다 — P125).
① **적군 메시** — 디자인팀 재납품 세 번(첫째 스켈레톤 문제 · 둘째 Auto-Rig Pro 명명 기각 · 셋째 `new_enemy_T` 채택). Assign Skeleton 본 추가 0 · 머티리얼 WorldGridMaterial → MI 할당 · `BP_Soldier_Hostile` 교체, 사용자 PIE "플레이 잘 됨, 총 붙음". ~~[Q42]~~ 해결. `StanceStandZ/CrouchZ` 는 **미실측** → [C-119].
② **왼손 IK 토글** `LeftHandIKEnabled`(ABP, 기본 **false** — 디자인팀이 왼손을 FK 로 맞추는 동안) + **그립 오프셋을 BeginPlay 에서 총 메시 `LeftHandGrip` 소켓으로 산출**(상수 (−30,8,4) 폐기). 총 부착 오프셋은 컴포넌트가 아니라 **세 메시의 `weapon_r` 메시 소켓**으로 흡수(`WeaponMesh` 상대 0) — 애님 에디터 프리뷰 = 런타임.
③ **급선회 스냅(비조준 A↔D 반전)** — `OffsetRootBone.maxRotationError 90` 이 원인, **−1(GASP 원본) 복귀**. 조준 중 뒤집힘은 09-12 유한 각속도가 이미 막고 있었다. ~~[C-80]~~ 확정 · [C-74] 는 70/65 쌍만 남음.
④ **BF 시 아군/적군 머리 부풀기** — BF 포즈 3장에 마네킹 PP ABP 의 head ×1.15 가 **구워져 있었다**(P122). `ModifyBone_8(head, 스케일 Replace 1, ComponentSpace)` 로 상쇄(~~[W54]~~ "삭제 권고" 철회). 재베이크는 [W65].
⑤ **총내림 전용 클립 2차 시험(로컬 애디티브) 실패·원복** — 결론: 총 내린 로코모션 클립 없이는 델타뿐, 델타는 작을 때만 자연스럽다(P123). [C-90] → 클립 요청 **[W66]**.
⑥ **디자이너 가이드 완성** `assets/2026-09-14_designer_guide.html`(실사용 시퀀스 **103개** 전체 경로 부록, 디스크 검증) · 핸드오프 md 갱신. 조작 확정: Ctrl 걷기/조깅 · 우클릭 조준 · 좌클릭 사격 · R 재장전 · V/B 앉기 · Q/E 린 · 1/3/2/4 BF · T 1·3인칭 · 휠 거리 · 관전 F/Tab. C 앉기 토글은 무력.
⑦ 발견: `GM_SoldierLab.PawnClasses_Soft` 가 실제로 안 비워져 있었고 GASP GM 은 그 0번을 스폰한다(P126, [W36] 정정, 사용자가 고침) · `/MoverExamples/.../CR_Mannequin_Body` 컴파일 에러 + `/Game/NewLevelSequence` 처분 **[Q48]** · `LogAbilitySystem ReloadDone` 로그 노이즈 **[W64]** · 총기 분기 시 `WeaponMesh`/소켓 규약 **[W67]** · `CHT_Soldier_CharacterAnimations` 실사용 확인 **[C-120]**. 새 원칙 **P121~P126**.
⚠ **미저장 자산(사용자 저장/체크아웃)**: `SoldierCharacter_ABP` · `BP_SoldierCharacter` · `new_enemy_T` · `enemy_T`(옛것 — 저장 불필요) · `MM_Rifle_LowReady`(값 원복, dirty 만). 옛 적군 에셋 삭제는 [W29] ④~⑨.
⚠ 사용자 작업 원칙(2026-09-15): **세션 중 긴 문서 작성으로 소통을 끊지 말고, 문서 정리는 작업 후 서브에이전트에 위임한다.**

★ **2026-09-15 저녁 — 체력 · 피격 · 사망** (`ai/2026-09-15_health_hit_death_implementation.md`, 조사·추천은 `ai/2026-09-14_hit_death_health_recommendation.md` — `drafts/` 에서 올라옴).
C++ `USoldierHealthComponent`(`AI/SoldierHealth`) 하나가 표준 `OnTakePointDamage` 로 데미지를 받고(투사체 무변경) · 부위 배율 · HitReact 애디티브 몽타주 13(`bStopAllMontages=false` 라 재장전 안 끊김, 슬롯 그룹 안 가름) · Death 몽타주 6 → 끝 0.1 s 전 래그돌 · 등록부/AI 컴포넌트/CMC/컨트롤러/Tick 을 한 곳에서 정지 · `Health/bDead/LastHit` 복제. ABP 에 `AdditiveHitReact` 슬롯 경로 3노드, BP Tick 총구 보정 게이트에 `NOT IsHitReacting` AND. **아군은 `BP_Soldier_Friendly` 의 `Invincible (무적)` 체크로 안 죽는다**(사용자 결정, 맞으면 움찔만). 사용자 PIE **"잘됨"**. ~~[W18]~~ 해결. 새 원칙 **P127~P129**(템플릿이 CDO 배열을 안 물려받음 · P53 정정 #2 · ANDBoolean 생성 가능).
⚠ **수치는 0개** — 본 None 빈도 · Death 루트모션 · 정착 프레임 스냅 · 임펄스 1500 · 45구 비용 · 피격 게이트 실효 → **[C-110]~[C-118]**. 사용자 결정 대기 **[Q46]**(시체 유지·엄폐화) · **[Q47]**(아군 사격). 관측 **[R8]** `ApplyAdditive_1`(재장전 애디티브) 알파 0.
⚠ **다음 빌드 필요**: 생성자의 `RF_ClassDefaultObject` 가드를 뺀 소스는 **미빌드** — 그때까지 BP 템플릿의 몽타주 배열은 MCP 로 직접 써 둔 값. Perforce: `AI/SoldierHealth.{h,cpp}` add + `BP_SoldierCharacter`/`SoldierCharacter_ABP`/`BP_Soldier_Friendly`/`_Hostile` 편집, 제출 여부 [C].

★ **2026-09-14 밤 ~ 09-15 새벽 — AI 거동 라운드** (`ai/2026-09-14_danger_map_and_position_commitment.md` → **`ai/2026-09-15_exposure_cycle_and_muzzle_learning.md`**).
사용자 관측 ①공격수 저돌 ②수비수 안절부절 ③꼬리물기 → 코드에서 읽은 **구조적 결핍**(위험 구역 개념 없음 · 뚫린 공간이 쌈 · 엄폐물 못 찾음 · 후퇴 없음 · primary contact 틱마다 교체 · 머무름 없음 · 판정 높이 상수) → 위험 지도(`USoldierDangerMapSubsystem`) · 모든 눈 · 머무름 · 표적 잠금 빌드 → **로그 실측 6라운드**(R1 예산 게이트 잠김 → R2 높이 실측·부채꼴 후보 → R3 가치 게이트 분리 → R4 경로 상한 제거 → R5 사격 활동도 → R6 **노출 회계 사이클 + 실제 총구 + 포즈별 총구 학습 + 코너 판정**). 새 원칙 **P114~P118**.
⚠ 수치 판정(리듬·코너 린·소강 전진·총구 학습)은 미완 → 후속 문서 12절. Perforce: `Source/SoldierLab/AI/` 12파일 체크아웃(신규 `SoldierDangerMap.{h,cpp}` add), **미제출**.

### 남은 일 (2026-09-15 기준, 우선순위)

```
1. [W53]  "노는 병사" 원인 확정 — [Engage] 전이 로그의 게이트 0 항목(believed/worth/aperture/onTarget/reloading/ammo)으로. 추측 금지(P10)
2. [W51]  엄폐 자리 예약 — 같은 후보를 둘 이상이 고름(수비수), MASKED 8~11 s의 원인
3. [W52]  분대 통신·화망 — 사각 없는 위치에서의 제압(적 예상 위치 사격), 엄호/이동 분담, 위험 지도 공유 여부
4. ~~[W18]  사망/대가 — 없으니 공격수가 수비수 코앞까지 걸어옴~~ → ✅ 09-15 저녁 해결. 남은 것: 다음 빌드(생성자 가드 제거분) · [Q46][Q47] 결정 · [C-110]~[C-118] 실측
5. [C-102]~[C-107] 값 실측 — 위험지도·머무름·활동도·노출 회계·총구 오프셋·벽 높이
6. Perforce 제출 — AI/ 12파일 + Camera/·Pose/·Observer/ 6파일(1인칭·머리 추종 세션분, CL 469 이후) + AI/SoldierHealth.{h,cpp}(add) + BP/ABP 4개
   + 애니메이션 정리분: SoldierCharacter_ABP · BP_SoldierCharacter · new_enemy_T (+ 소켓 바뀐 soldier_T · SKM_UEFN_Mannequin)
7. 애니메이션 쪽 — [C-119] 적군 StanceZ 실측(PIE HUD PelvOff 한 번) · [W29] 옛 적군 에셋 4벌 삭제 · [Q48] NewLevelSequence/CR 처분 결정
   · [W66] 총내림 로코 클립 요청 시점 · 디자인팀 왼손 FK 검수 결과 오면 LeftHandIKEnabled 재검토
```

> **2026-09-15 새벽, 1인칭·머리 추종 세션 마무리(다른 세션)**: [W50] 관전 폰 1인칭 위임 + 관전 H 확인 ✅ · **몸 회전** 추가(H 켜짐·비조준·정지 시 카메라 60° 밖이면 캡슐 yaw 만 돌려 GASP TIP, P119 · Strafe 우회는 항상 견착이라 폐기 P120) · ABP `ModifyBone_9`(neck_02) 수동 완료(그전엔 Bone None) · ~~잔여 `ModifyBone_8` 삭제 권고 [W54]~~(→ 같은 날 오후 BF 머리 스케일 상쇄용으로 **사용 중**, 철회). 사용자 "잘됨". 상세 `animation/2026-09-14_sight_alignment_plan.md` 0'·0.11절.

</details>

<details>
<summary>2026-09-14 밤 시점의 머리글 (접힘)</summary>

2026-09-14 밤 / **★ `titan_example` 편입 완료 · [C-95] 기준면 수정 빌드됨(정착 미확인) · 관전 폰 C++ · 1인칭(T) · 머리 조준 추종 + 눈–조준선 정렬(H, 기본 OFF) 사용자 확인 "완벽" · AI 층 동작 확인 · 아군 메시 교체 완료** / 병사가 스스로 보고·듣고·전달받고·제압당하고·쏘고·엄폐한다. 플레이어는 1인칭으로 들어가 조준경 뒤에서 본다 — 눈이 조준선 위에 온다.

★ **2026-09-14 밤 — 관전 · 1인칭 · 머리 추종 3종 완성, 사용자 최종 확인 "성공. 이제 모든게 완벽해" (21:30)** (`ai/2026-09-14_cover_frame_fix_and_observer.md` 0' 절 · **`animation/2026-09-14_sight_alignment_plan.md` 0' 절**).
`Observer/SoldierObserverPawn`(F 추적 · T 1/3인칭 · Tab · 휠, 픽 15°) · `Camera/SoldierFirstPersonComponent`(T, 뷰타겟 교환, `eyes` 소켓, 니어플레인 2 cm) ·
`Pose/SoldierHeadAimComponent`(H, **기본 OFF**, 닫힌 루프 Additive, 몸 프레임 = spine_03, 2단 둘러보기/weld 가중치+래치, 목 굽힘 60° + 스트레치 5 cm 로 눈을 조준선 위에, 맹목사격 시 off).
머리 추종은 같은 날 **열 번** 고쳤다 — 그 절반은 **몸 피치를 델타 재조립에서 빠뜨린 한 줄**(P109)이 만든 "용수철"을 다른 장치로 가리던 것. 원칙 P103~P113(`CLAUDE.md` 5절).
⚠ **[C-95] 는 빌드만 됐다** — 수비수가 정착하는지 아무도 안 봤다. 1.6절 기준으로 확인이 다음 세션 첫 일.
Perforce: 사용자 CL 469 제출, 그 뒤 변경분(`Camera/` 2 · `Pose/` 2) 미제출.
**아군은 이제 `soldier_T` 외형이다** — `Assign Skeleton` 으로 `SK_UEFN_Mannequin` 에 올려 애니메이션·PSD·ABP 를
**하나도 안 고치고** 돈다 (`IMPLEMENTED.md` 3.2절). 적군 외형은 **결정 대기 [Q42]** — 지금은 마네킹 그대로이고 AI 작업을 막지 않는다.
⚠ 메시를 바꾸면 `StanceStandZ/CrouchZ` 같은 **실측값을 메시별로 다시 재야 한다**(P77). 사용자가 원하는 다음 시험은
"아군을 마네킹 비율로"(translation retargeting `Animation`, 5분) → **[W28]**.
★ **2026-09-14 교정 라운드가 들어갔다** — 노출의 사다리(조리개·사격자세) · 반동 · 조준 선회 ·
`FightingCost` · 재장전 조건 · 추측항법 만료 · 정지 감지/RVO · **110m × 90m 레벨 재건** · 빙의.
**목표 층은 이번 라운드에야 처음 실제로 돌았다** — 레벨에 1개 놓았고, 더 큰 것은 `ASoldierObjective` 가
**루트 컴포넌트가 없어 영원히 월드 원점에 서 있었다**는 것이다(**P90**).
⚠⚠ **그러나 수비수가 정착해 쓰지 못하는 문제는 두 번의 시도로도 해결되지 않았다.**
**다음 세션의 첫 일은 세 번째 추측이 아니라 진단이다** → **[C-95]** · `ai/2026-09-14_exposure_ladder_and_corrections.md` 12절.

★ **2026-09-14 오후 — [C-95] 진단 완료, 수정 빌드 대기** (`ai/2026-09-14_cover_frame_fix_and_observer.md`).
사용자 관측 `HERE ≈ 1.0` 으로 갈라졌다: 12절의 세 갈래가 아니라 **후보(발)와 HERE(캡슐 중심, +90 cm)를 다른
높이에서 재고 있었다.** 같은 버그가 `RequiredStance` 를 0 으로 눌러 **제압 없이는 웅크리지 않던** 원인이기도 하다.
발 기준 통일(`GetFeetLocation`, **P100**) · 위협 눈 2.6 m → 기록+20 · 아군 몸 무시 · 총구 발 기준.
**관전 폰도 C++ `ASoldierObserverPawn` 으로 재작성** — 옛 BP 는 Visibility 트레이스라 병사를 못 맞혔고 F 가 `Possess` 라
AI 를 멈췄다. 이제 F 추적(AI 계속) · T 1/3인칭(병사 조작과 같은 키) · Tab 다음 · 휠 거리. **빌드 후 `BP_ObserverPawn` 부모를
`SoldierObserverPawn` 으로 바꿔야 한다**(그래프는 비워 둠). 직접 조작 모드 1인칭은 여전히 없다([W30]).

</details>

---

## ★ 2026-09-14 — `titan_example` 편입 **완료**

디자인팀이 GASP/Lyra 채택을 결정해 `SoldierLab` 을 본체에 편입했다. **에디터 기동 · PIE · `L_SoldierTest` 실행 전부 확인.**
**이 PC 의 `SoldierLab` 프로젝트에서의 작업은 여기서 끝난다** — 이후 작업은 `C:\working\kadex	itan_example` 에서 한다.

전문: **`migration/2026-09-14_titan_example_migration.md`** (충돌 실사 · 절차 · 결과) ·
**`migration/2026-09-14_asset_cleanup.md`** (참조 전수 스캔 · 외부 의존 감사)

### 무엇이 넘어갔나

| 구성 | 에셋 | 용량 |
|---|---|---|
| `Content/SoldierLab/` | 377 | 320 MB |
| `Characters/UEFN_Mannequin` (GASP 로코모션) | 1583 | 1874 MB |
| `Characters/Heroes` · `UE5_Mannequins` · 기타 | 905 | 1316 MB |
| **합계** | **2865** | **3510 MB** |

C++ 는 `Source/SoldierLab/`(Runtime) + `Source/SoldierLabEditor/`(Editor) 두 모듈. 플러그인 **42종 활성**
(SoldierLab 이 쓰던 것 중 `RigLogic`·`HairStrands`·`LiveLink`·`LiveLinkControlRig` 4개만 빠졌고, 넷 다 삭제한 샘플 콘텐츠용이라 불필요).

### 이관에서 배운 것 (P95~P101)

- **P98** 폴더 크기는 상한이지 이관 비용이 아니다 — 2.7 GB 폴더에서 실제로 따라온 건 1874 MB
- **P99** 두 프로젝트를 합칠 때는 **빌드 전에 심볼을 대조**한다 — `IMPLEMENT_PRIMARY_GAME_MODULE` · `UCLASS`/`USTRUCT`/`UENUM` 이름 · 콘솔 변수. 이번에 **셋 다** 걸렸다
- **P100** 없는 모듈을 참조하는 에셋은 모듈을 들여와 고치지 않고 **참조를 끊어** 고친다 (`/Script/LyraGame` → 0건)
- **P101** ★ **이관 규모를 `get_dependencies` 재귀로 재지 말 것** — 소프트/클래스 참조를 빠뜨린다. 934개로 예측한 것이 실제 3622개였다

### 남은 일 (우선순위)

```
(2026-09-14 밤 갱신)
1. [C-95]  ★ 수비수 정착 — 원인은 잡았고(기준면, P103) 빌드도 됐다. **정착하는지 보는 것**이 남았다.
           SoldierLab.Debug.Cover 1 + AI.Filter Friendly 로 `HERE 0.00 hide+fight → stay` 유지되는가 (1.6절)
   ~~[W50]  관전 폰 1인칭 → 병사 1인칭 컴포넌트 위임 + 관전 H 키~~ ✅ 2026-09-15 확인
2. 디자인팀 핸드오프 — `assets/2026-09-14_designer_guide_draft.md` 조작표는 채워졌다(조준/사격/재장전 키만 확인 필요)
     · 적군 스켈레톤 규격서 — "soldier_T 와 같은 규격" 한 줄이면 된다 [Q42]
     · [W45] weapon_r 소켓 없는 병사 3명
     · [W49] 견착 포즈 뺨 높이·조준경 높이 — 머리 추종이 닫을 수 있는 거리(굽힘 60° + 스트레치 5 cm)의 나머지는 포즈·소켓 몫
3. [W35]  계측 잔해 게이트 — 45명이 전부 DrawDebugCoordinateSystem 을 그린다
4. [C-99] 재질별 명중이 실제로 갈리는가 · [C-100] FootstepEffectTagModifier
5. Perforce 제출 — CL 469 이후 변경분(`Camera/SoldierFirstPersonComponent.{h,cpp}` · `Pose/SoldierHeadAimComponent.{h,cpp}`)
~~6. [W30]  1인칭 카메라~~ ✅ 구현됨   ~~[W46] 맹목사격 게이트~~ ✅ BF_Alpha* 로 전체 off   ~~[W47] 머리 추종 검증~~ ✅ 10차 확인
   (작은 것) [W48] 세션 첫 조준은 정착 후 weld 시작 — 눈에 띄면
```

<details>
<summary>2026-09-11 시점의 한 줄 요약 (접힘)</summary>

2026-09-11 / **★ 애니메이션 층(L4) 마감** / 이동·조준·견착/총내림·사격·재장전·왼손 IK가
전부 동작하고, 전 방향 로코모션의 속도 불일치([C-58])와 웅크리기 회전 발작(P18 재발)까지 해결했다.
플레이어가 WASD로 직접 검증 가능하다. **다음은 AI 상위 계층이다.**

</details>

> **무엇이 존재하는지는 `IMPLEMENTED.md`** — 에셋 목록, 애님 그래프 구조, 확정된 튜닝값과 그 근거.
> 이 문서는 "다음에 뭘 할 것인가"만 다룬다.

> **2026-09-12 추가분**: **유한 몸통 각속도**(총을 내리면 빨리 돈다 — 90~720 °/s)와
> **조준 보정 안티 와인드업**(`Enable_AO` 게이트)이 들어갔다. 새 미해결 3건 —
> **[C-76]**(간헐적 무기 잠금, 방아쇠 미규명) · **[C-77]**(극단 상방 조준, **의도적 미해결**) ·
> **[R6]**(`AO_Blend_Curve` 미확인).
> 전문: `animation/prototypes/2026-09-12_body_yaw_rate_and_aim_antiwindup.md`

> **2026-09-12 추가분 (2)**: **블라인드 파이어 축**이 들어갔다 — `BlindFireH`(−1~+1) ·
> `BlindFireV`(0~+1) 두 연속 축이 **저작 포즈 3장을 마스크 애디티브로** 구동한다(키 1·2·3·4).
> 총구 정렬 · 린에 이은 **세 번째 연속 자세 축**이고 사용자 확인 완료.
> 새 미해결 3건 — **[C-78]**(BF와 총구 보정 적분기의 상호작용 미측정) ·
> **[W7]**(린 축이 문서에 없다) · **[W8]**(마스크 규약 두 벌). **[C-8] 해결**(형태가 바뀜).
> 전문: `animation/prototypes/2026-09-12_blind_fire_axis.md`
> **포즈 저작 경로 자체는 `CLAUDE.md` 6.3절에 규약으로 적었다** — 막다른 길 4종 포함.

> **2026-09-12 추가분 (3)**: **연속 stance 축**이 들어갔다 — `StanceAxis`(0 기립 ~ 1 웅크림, 키 **V/B**)
> 하나가 **골반 높이 · 이동 속도 상한 · `Gait` 라벨 · MM 데이터베이스**를 함께 몬다.
> **네 번째이자 마지막 연속 자세 축**이고 사용자가 단계마다 확인했다.
> 골반 높이는 `FootPlacement`와 `LegIK` **사이**의 `ModifyBone(pelvis)`를 **매 프레임 역산**해서 잡는다
> (새 C++ 2함수). **같은 증상을 네 번 오진한 기록이 이 문서의 본체다** → **P44**(과도구간을 계측하라) ·
> **P45**(통한 패턴을 반사적으로 옮기지 말라) · **P46**(이산 라벨은 연속량과 함께 바꾼다).
> 새 미해결 — **[C-79]**(움찔 해결의 기구 미증명) · **[C-80]**(`maxRotationError 90` 회귀 후보) ·
> **[W9]**(★ 캡슐이 아직 이진 — **엄폐 판단이 읽는 높이**) · **[W10]**(카메라) · **[W11]**(AO 연속) ·
> **[W12]**(중간 포즈 3점 블렌드) · **[W13]**(`IA_Crouch` 무력화). **[Q41] 결정**: 웅크린 채 달리면
> **속도를 묶는다** — 자세는 AI가 소유하는 결정이고 속도가 결과다.
> 덤으로 **AI에게 치명적이던 `GetControlRotation(GetController())` null 버그**를 두 호출부에서 고쳤다.
> 전문: `animation/prototypes/2026-09-12_continuous_stance_axis.md`

> **2026-09-12 추가분 (4)**: **무기·투사체가 붙었다** — `titan_example`의 `ARCWSProjectile`을
> **의존 4종을 끊어** `Source/SoldierLab/Weapons/SoldierProjectile`로 이식하고,
> `BP_RifleProjectile`(마이그레이션본이 껍데기여서 **0부터 재작성**) · `BP_AR4Rifle`(순수 BP) ·
> `BP_SoldierCharacter` 배선까지 마쳤다. **실제로 총알이 날아가고 재질별로 터진다.**
> 포물선 탄도 · 도탄 · 재질별 명중 세트 · 데칼 · 지면 혈흔 · **총알 휘파람**이 살아 있다.
> ★ **휘즈의 최근접 판정이 AI 제압 신호의 자리다** — 기하는 이미 있고 지금은 카메라만 본다 → **[W16]**.
> `Config/DefaultEngine.ini`에 `PhysicalSurfaces` 5종(**선언 순서가 의미를 갖는다**)과
> 트레이스 채널 `Cover`/`Sight`를 팠다. **이식 전 "채널 인덱스가 충돌한다"는 예고는 틀렸다** —
> `titan_example`은 커스텀 채널을 쓰지 않는다 → **P52**.
> 배선 중 버그 6건에서 **P47~P51**이 나왔다(원인이 둘인 컴파일 오류 · 액터 단위 숨김 ·
> 자기 총열 명중 · **조건 앞에서 낸 부작용이 재장전 몽타주를 잘라먹은 것** ·
> 메시 교체 후의 `GetAnimInstance` None · 노드 이동 후 남은 exec 링크의 무한 재귀).
> 새 미해결 — **[C-81]**(디버그 궤적 라인만 안 보인다) · **[C-82]**(이식 튜닝값 미검증) ·
> **[R7]**(진영 판정의 정식 소스) · **[W14]**(에셋 경로) · **[W15]**(**P4V 체크아웃 미처리**) ·
> **[W16]** · **[W17]**(끊어낸 의존의 복원 지점).
> 전문: `weapons/2026-09-12_projectile_port.md`

> **2026-09-13 — AI 층이 들어갔다 (이 문서의 현재 머리글)**: `Source/SoldierLab/AI/` **C++ 8쌍**과
> `BP_SoldierCharacter` 컴포넌트 7개, `BP_Soldier_Friendly`/`_Hostile`, 관전 폰, 시가지 시험 레벨.
> **AI가 GASP 몸을 실제로 운전한다.**
>
> 구조적으로 새로 정한 것 — **신선도와 해상도를 합치지 않는다**(P62) · **감쇠는 질의 함수에**(P63) ·
> **불확실은 라벨이 아니라 지터**(P64) · **융합은 역분산 가중이라 우선순위 규칙이 없다**(P65) ·
> **게이트는 이미 있는 두 양의 비교**(P66, "앎이 무기보다 나쁜가") · **비율 게이트에는 절대 게이트를**(P67) ·
> **술어는 긍정 열거로**(P68) · **0 비용 반복이 가능한 루프는 횟수로도 막는다**(P69) ·
> **오버레이의 한 채널은 한 뜻**(P70) · **AI는 플레이어와 같은 레이트 상수를 쓴다**(P71).
> 도구 함정 9건이 **P53~P61**로 올라갔다 — 그중 **[P53] `set_properties`가 이 환경에서 쓰기를 못 한다**가
> 가장 아프다(모든 기본값을 사용자가 직접 넣어야 한다).
>
> ★ **"AI가 총을 안 들고 몸도 안 돈다"의 근본 원인**은 `S_PlayerInputState.WantsToAim` 이었다.
> 파생 플래그 `AOActive`를 만지던 이전 패치는 증상 처치였으므로 **되돌렸다** → **P36**.
>
> 해결 — **[W3]**(컨트롤러 조준 구동) · **[W16]**(제압 신호) · **[C-43]**(형태가 바뀜: StateTree 아님) ·
> **[R7]**(정식 진영 소스 생김, 투사체만 잔여).
> 새 미해결 — **[C-83]**(45명 성능) · **[C-84]**(제압 상수) · **[C-85]**(인지 상수) ·
> **[C-86]**(교전 거리) · **[C-87]**(교전/엄폐 튜닝값) · **[D10]**(★ 목표·임무 개념) ·
> **[W18]**(사망 없음) · **[W19]**(린/BF를 AI가 안 몬다) · **[W20]**(경로 노출) ·
> **[W21]**(엄폐 후보가 링 하나) · **[W22]**(소리 차폐) · **[W23]**(투사체 진영) · **[W24]**(`Cover` 채널 미사용).
> 전문 3부작: **`ai/2026-09-13_perception_stack.md`** · **`ai/2026-09-13_engagement_and_cover.md`** ·
> **`ai/2026-09-13_ai_bridge_and_scene.md`**

> **2026-09-13 추가분 (2) — 목표(objective)와 위치 비용**: `ASoldierObjective`(9번째 파일 쌍)와
> **세 비용 위치 스코어러**가 들어갔다 —
> `Cost = Exposure + RouteRiskWeight×RouteRisk + ObjectiveWeight×ObjectiveCost`, **전부 0..1 한 통화**.
> 목표 액터는 **로직이 없는 레벨 마커**이고 **한 개가 쥔 쪽에겐 수비·나머지 전부에겐 공격**이 된다.
> 수비 비용은 반경 안이 **평평한 0**(거리로 매기면 수비대가 중심점으로 붕괴한다), 공격은 **평지 없는 비례**.
> 경로 노출 표본이 함께 들어와 **[W20]이 절반 해결**됐다(직선 표본 — 네브메시 경로가 아니다).
> 새 원칙 — **무모함은 값이 안 매겨진 항이다**(P72) · **지킬 것이 구역이면 비용도 구역**(P73) ·
> **비용은 실제로 수행되는 방식으로 매긴다**(P74, 경로는 *서 있는 높이*로) ·
> **엔진 베이스가 쓰는 이름을 지역변수로 쓰지 말 것**(P75, `Role` → C4458).
> 해결 — **[D10]**. 새 미해결 — **[C-88]**(목표 층 값과 거동) · **[W25]**(★ **레벨 배치 0개 · 목표가
> `BeginPlay` 에 하나로 고정**) · **[W26]**(반동 누적이 없다 — 30번째 탄이 첫 탄과 같다).
> ⚠ **이 조각은 아직 한 번도 돌아 본 적이 없다** — 레벨에 목표가 없으면 셋째 항은 0이다.
> 전문: **`ai/2026-09-13_objective_and_position_cost.md`**

> ★ **2026-09-14 — 교정 라운드 (이 문서의 현재 머리글)**: 목표 층이 들어온 뒤의 모든 것.
>
> **교전** — 총구가 막히면 거절하는 대신 **조리개**(Direct/Over/Right/Left)를 찾고, 그 다음
> **몸을 얼마나 같이 내보낼지**를 고른다(Open / Lean / Blind — 칸을 고르는 것은 **제압도**).
> 조리개의 답이 **위 아니면 옆**이고 그것이 포즈 파이프라인이 이미 갖고 있던 두 축이었다 → **[W19] 해결**.
> **반동**은 탄창이 줄어드는 것을 보고 센다(새 훅 없음) → **[W26] 해결**.
> **조준이 240 °/s 로 선회**하고 시야 콘도 그 회전을 읽는다(머리가 안 돌아간 표적은 보지 못한다) → 새 의도 `Traversing`.
> 재장전은 오지 않는 소강상태를 기다리는 대신 **숨을 곳이 있는 순간**을 산다.
>
> **위치 비용** — 첫 항이 노출에서 **`FightingCost`** 로 바뀌었다: 노출만 최소화하면
> **깊은 구석이 만점**이고 거기 있는 병사는 **맞을 수도 쏸 수도 없다** — 실제로 그러고 있었다.
> 경로 위험은 **건너는 거리로 스케일**하고 가중치를 1.0→0.6 으로 내렸다
> (같게 두었더니 **아무도 스폰에서 안 움직였다**). 후보는 **두 겹 링**, 위협 추정은 **한 바퀴 동안 얼린다**,
> 위협이 없으면 `bCanHide` 는 **true** 다(아무도 안 쏭면 엄폐가 없는 자리란 없다).
>
> **목표** — ★ `ASoldierObjective` 가 **맨 `AActor`** 라 루트 컴포넌트가 없었고, 그래서
> **모든 목표 거리가 (0,0,0) 에서 재어지고 있었다**(P90). 공격 비용의 **clamp 제거**(평평한 비용에는 기울기가 없다),
> 환율 6000→3000, 반경 900→1500 · 밴드 900→1200(**수비 사격 위치가 1077cm 로 옛 반경 바로 밖**이었다).
> 레벨에 `Objective_AllyBase` **1개 배치** → **[W25] 절반 해결**.
>
> **그 밖** — 추측항법 만료(`VelocityTrustSeconds 1.5` — 병사들이 **환영을 향해** 자리를 옮기고 있었다) ·
> **정지 감지**(보고된 프리즈의 정체 — 마주 보고 걸어간 둘이 경로추종기를 영원히 `Moving` 으로 두었다) ·
> **RVO 회피** · **스프린트**(규칙이 아니라 **느릴 이유의 부재**) · **레벨 재건 110m × 90m** · 관전 **빙의**.
>
> 새 원칙 **P81~P94** — 막힘의 답은 다른 총구 위치(P81) · **완벽한 엄폐와 사격 위치는 반대말**(P82) ·
> 시간 규모가 다른 비용을 같은 가중치로 더하지 말 것(P83) · 비교에만 쓰는 비용에 천장 금지(P84) ·
> 구역 반경은 싸울 엄폐를 품어야 한다(P85) · 레이트 제한은 **양을 만드는 자리에서 한 번**(P86) ·
> 진행 중은 상태가 아니라 **진척**으로 묻는다(P87) · 믿음의 반감기는 양마다 다르다(P88) ·
> ★ **거동은 규칙이 아니라 선호로 쓴다**(P89, **사용자 명시 요구**) · 루트 없는 `AActor` 는 위치를 못 갖는다(P90) ·
> 도구 함정 3건(P91~P93) · 화면의 예고와 키의 실행은 같은 계산에서 나와야 한다(P94).
>
> 해결 — **[W19]** · **[W26]**. 절반 — **[W20]** · **[W21]** · **[W25]**.
> 새 미해결 — **[C-91]**(사다리 상수) · **[C-92]**(반동) · **[C-93]**(조준 선회) · **[C-94]**(위치 비용 재교정값) ·
> **★★ [C-95]**(수비수가 정착하지 못한다 — **세 갈래 진단**) · **[C-96]**(속도 신뢰) ·
> **[C-97]**(레벨 규모·정지 판정·회피) · **[W30]**(★ **1인칭 카메라가 없다 — 요청됐고 안 만들었다**) ·
> **[W31]**(관전 입력이 InputAction 이 아니다) · **[W32]**(조리개 트레이스가 예산 밖).
> 전문: **`ai/2026-09-14_exposure_ladder_and_corrections.md`**

> **오늘 한 일 전문: `animation/prototypes/2026-09-09_lyra_rifle_migration.md`** — 반입·PSD·Chooser·
> 디버깅 전 과정. **다음 세션은 이 문서만 보면 로코모션 상태를 전부 안다.**

> **전문: `animation/prototypes/2026-09-04_p0-4_complete.md`** — 확정된 11단계 파이프라인, 모디파이어
> 순서 규약, PSD 설정값이 여기 있다. **반입 작업을 하려면 이 문서만 보면 된다.**
>
> **★ 계획 대비 무엇이 달라졌는지: `design/2026-09-04_p0_checkpoint.md`** — 리스크 장부가 뒤집힌 것,
> 인플레이스 전제 오류, CMC/Mover 분기, 반복된 실패 패턴, 남은 계획 조정.
> **새 세션은 `CLAUDE.md` → `CURRENT_STATE.md` → 이 점검 문서 순으로 볼 것.**

> **환경 (2026-09-03 갱신)**: 프로젝트는 `C:\working\kadex\anim_test\SoldierLab`(`works\` 없음),
> 문서는 `C:\mine\insung_grapic\titan\soldier_ai_lab`, **MCP 포트 8000**. → `CLAUDE.md` 6·7절

---

## 1. 한 줄 요약

**"GASP 정도의 움직임 + 총을 든 상태"가 처음으로 성립했다. 이제 전투 동작(사격·재장전·피격)과
로우레디를 얹을 차례다.**

### 1.1 현재 보유 재고 (2026-09-09)

| 구분 | 내용 | 상태 |
|---|---|---|
| **로코모션** | Lyra 라이플 61클립 — Walk·Jog·Crouch × 4방향 × Loop/Start/Stop/Pivot + TurnInPlace + Idle | ✅ PIE 동작 |
| **PSD / Chooser** | PSD 16개(Epic Dense 설정 복제) · `PSN_Rifle_All` · Chooser 3열 16행 | ✅ |
| **조준 오프셋** | `AO_Rifle_Aim` (Yaw 5 × Pitch 3) — GASP AO 배관 재사용 | ✅ 동작 |
| **액션** | 95개 — 사격·재장전·피격 13·사망 6·근접·수류탄·대시 | ✅ 사격·재장전 배선 완료 |
| **무기** | `SK_Rifle` → `weapon_r` 부착 + `ABP_Weap_Rifle`(볼트·탄창) | ✅ |
| **로우레디** | `AO_CD` + 뼈 마스크 | ✅ (완벽하진 않음) |
| **조준 포즈** | AO 포즈 15 + 오버라이드 2 | ✅ 애디티브 복구됨 |
| **플레이어 조작** | `GM_SoldierLab` — WASD·조준·앉기·달리기 | ✅ |


### 1.2 오늘 규명한 원인 세 가지

전부 "우리 설정이 Epic 전제와 안 맞았던 것"이지 GASP/Lyra 결함이 아니었다.

| 증상 | 원인 |
|---|---|
| 이동 중 발 끌림 | 기본 `gait`가 `Run`이라 `walkSpeeds`가 **한 번도 안 쓰였다.** 583cm/s 클립을 500으로 재생 → **[C-52] 해결** |
| Idle 0.5~1초 주기 움찔 + 좌회전 | **Epic의 비용 편향은 클립 227개를 전제**한다. idle 1개인 우리 세트에선 반대로 작동 → `../CLAUDE.md` P18 |
| 조준 상하좌우 무반응 | 데이터·배선 모두 정상이었고 **컴파일/저장 반영 문제**였다 |

---

## 2. 프로젝트 현황

| 항목 | 상태 |
|---|---|
| UE 프로젝트 | ✅ 생성 완료 (UE5.8, GASP 기반, C++ 전환됨) |
| **소스컨트롤** | ✅ **Perforce(P4V)로 확정.** `.p4ignore` 작성 완료 (프로젝트 루트) |
| 플러그인 | ⚠️ GASP 기본 + StateTree 툴셋. **AI 계열(SmartObjects·NavCorridor·FullBodyIK 등) 미확인** → `CLAUDE.md` 7.3절 |
| GASP 동작 | ✅ PIE 확인 — 관성 이동·정지·제자리회전·벤치 상호작용 전부 자연스러움 |
| MCP | 포트 8001. **PC를 옮기면 Auto Start Server를 다시 켜야 함** → `CLAUDE.md` 7.4절 |

> **다른 PC에서 처음 시작한다면 `CLAUDE.md` 7절을 먼저 볼 것.**
> 특히 `Binaries/`가 P4에서 제외돼 있어 **C++ 빌드를 먼저 해야 에디터가 열린다.**

---

## 3. 이번 세션(2026-09-01~03)에 확정된 것

### 3.1 P0-1은 사실상 통과했다

**궤적이 플레이어 입력이 아니라 캐릭터 이동 상태에서 생성된다.**
`PoseSearchGenerateTrajectory(forCharacter)`가 캐릭터를 통째로 받는 엔진 함수라, AI가 CMC를
구동하면 동일 경로로 궤적이 나온다. 설계에서 "최대 리스크"로 잡았던 `USoldierTrajectorySource`
자체 구현이 **불필요**하다.

AI가 손댈 것은 정확히 2개: `PreviousDesiredControllerYaw`(위협 방향) + `TrajectoryGenerationData`
재튜닝.

### 3.2 워핑이 무엇을 커버하는지 확정 — 조달 계획이 정밀해졌다

Epic이 GASP 그래프에 남긴 주석으로 확정:

| 카테고리 | Orientation Warping | 결론 |
|---|---|---|
| **정상상태 루프**(직선 이동) | ✅ 커버 — "전진 보행 하나로 여러 각도 스트레이프" | 견착 루프는 **전방 위주 소수 클립으로 충분** |
| **start / stop / pivot / turn** | ❌ 구조적으로 못 함 (직선 구간에서만 동작) | **데이터 필수 — 여기가 진짜 비용** |

### 3.3 반입 클립 필수 커브 3종

```
contact_l / contact_r   발 심기 타이밍     없으면 발 IK 오작동
Enable_Warping          워핑 허용 구간     없으면 방향 커버 0
Disable_AO (선택)       조준 억제 구간
```

**이게 디자인팀 요청 스펙의 핵심 항목이 된다.**

### 3.4 조달 요청의 성격이 바뀌었다

> ~~"라이플 견착 로코모션 클립 수백 개"~~
> → **"라이플 견착 정지 포즈 42개 + 무기 자세 앵커 3개 + 전환 클립 소수"**

조준 공간 전체가 **7 yaw × 3 pitch × 2 자세 = 42 정지 포즈**로 정의된다(로코모션 1,450개의 3%).
정지 포즈는 루트모션도 접지 커브도 필요 없어 **제작 난이도가 완전히 다르다.**

### 3.5 GASP는 AI 샘플이기도 하다

`STT_FindSmartObject → STT_ClaimSlot → STT_UseSmartObject` 흐름이 **중첩 상태**로 구현돼 있다.
부모 상태가 자원을 보유하므로 **상태를 벗어나면 예약이 자동 해제**된다 — 설계에서 "사망 시
가장 위험한 항목"으로 표시했던 SmartObject 예약 누수가 **구조로 해결**된다.

---

## 4. 다음 작업 — 우선순위 순

### ✅ P0-4 · 반입 파이프라인 검증 — **완료 (2026-09-04)**

> **결과**: `Walking_Anim`이 `enable_warping`·`contact_l`·`contact_r`·`MoveData_Speed`·`Phase`
> + `L`/`R` 싱크마커를 갖고 `SK_UEFN_Mannequin` 위에서 루트모션으로 재생된다.
> `PSD_Soldier_Walk_Test` 인덱싱 성공(경고 0). **GASP `M_Relaxed_Walk_Loop_F`와 구성이 동일하다.**
>
> 절차·설정값은 **`animation/prototypes/2026-09-04_p0-4_complete.md`**. 아래는 진행 이력이다.

<details>
<summary>진행 이력 (접힘)</summary>

### ① P0-4 · 반입 파이프라인 검증 (1일) ★ 최우선

**이제 모든 것의 전제가 됐다.** 커브 3종을 만들 수 있는지가 조달 계획 전체를 좌우한다.

> **2026-09-03 진전**: 최대 리스크였던 `Enable_Warping` **생성 수단 문제는 끝났다.**
> GASP에 `AM_WarpingAlpha` 모디파이어가 이미 있다 → **[C-25] 해결**.
> `animation/prototypes/2026-09-03_enable_warping_curve_generation.md`
> 남은 것은 "수단이 있는가"가 아니라 **"우리 클립에서 쓸 만한 값이 나오는가"**([C-26]/[C-27]).

**순서를 지킬 것 — 바꾸면 커브가 조용히 망가진다:**

- [x] **클립 확보·반입 완료 (2026-09-03)** — `Walking.fbx` (비인플레이스 확인: 골반 순이동
      136.6cm / 1.367s / 99.9cm·s⁻¹ / 완전 순환 루프). ★ **"In Place" 체크 해제**가 필수
      · ~~인플레이스~~ 는 틀린 지시였다 → `assets/2026-09-02_...md` 14절
      · `Rifle Aiming Idle.fbx`는 순이동 0 → **커브 검증엔 못 쓴다**(견착 앵커용)
      · 반입 위치 `/Game/SoldierLab/Source/Mixamo/` → `Walking_Anim` / `Walking_Skeleton` / `Walking`
        (42키 / 1.366667s — 원본과 일치, 리샘플 없음)
- [x] `Walking_Anim` 전진 확인 (임포트에서 Translation 살아있음)
- [x] **IK Rig `IK_Mixamo`** — auto generate. 체인·손가락 정상, **수정 불필요**
      · IK Goal이 일부 `None`이어도 무관 — **소스 리그에서는 안 읽힌다**(전수 확인)
      · `Root Motion: Hips`도 그대로 둘 것 — 비우면 op이 초기화에 실패한다
- [x] **IK Retargeter `RTG_Mixamo_to_UEFN`** — 타깃은 기존 `IK_UEFN_Mannequin` 재사용
      · ⚠⚠ 새 리타기터는 **op 스택이 비어 있다.** `Op Stack` 탭 → **`Add Default Ops`** 를
        먼저 눌러야 한다. **안 누르면 Auto Align이 아무것도 안 한다**(체인 매핑이 없어서)
      · 리타깃 포즈: `Source` 선택 → `Auto Align ▼ → Align All Bones`(방식 **`Default`**) → T→A 정렬
      · ⚠ **Root Motion op은 꺼둠** — 켜면 골반이 바닥에 붙는다 **[C-32] 원인 미규명**
- [x] **`AM_EncodeRootBone` → 루트모션 합성** ← ★ 워핑 커브보다 먼저
      · `/Game/SoldierLab/AnimModifiers/AM_EncodeRootBone` (pelvis 1.0 / **회전 비움 [C-31]**)
      · **루트 전진 확인, 발 미끄러짐 없음**
- [x] `bEnableRootMotion = true`
- [x] 산출물 `/Game/SoldierLab/Animations/Walking_Anim` — **`SK_UEFN_Mannequin`**, 42키, 1.366667s
- [x] `AM_WarpingAlpha_Soldier` 적용 → `Walking_Anim`은 **전 구간 1** ✅ 규약대로
      · GASP 원본을 복제해 우리 폴더에 둠(임계값 튜닝 대비)
- [x] **커브 세트가 클립 종류별로 다르다는 것 확인** → **[C-34]** 매핑표 필요
      · 정지·제자리회전 클립에는 `Enable_Warping`을 **만들지 않는 것이 정답**
      · `Turning_Right_90_Degrees_Anim`에 실수로 걸었음 → **Revert 필요**
- [x] **[C-28] 해결** — `contact_l/r`은 **이진 사각파**. `MotionExtractor`로는 못 만든다

- [x] **[C-33] 해결 — `contact_l`/`contact_r` 생성** (2026-09-04)
      · **C++ 에디터 모듈 `SoldierLabEditor` 신설** + `UFootContactCurveModifier` 자작
      · 엔진 기본 모디파이어로는 불가능했다(연속값 / 걸음당 노티파이 1개)
      · ★ 핵심은 **발별 자동 높이 보정** — 리타깃이 좌우 발 높이를 다르게 만든다 **[C-35]**
      · `animation/prototypes/2026-09-04_foot_contact_curve_modifier.md`

**현재 `Walking_Anim`: `enable_warping` · `contact_l` · `contact_r` (5종 중 3종)**

- [ ] `AM_FootSteps_Walk` → 발자국 노티파이 (`phase`의 선행 조건)
      · ⚠ 싱크마커도 필요하면 `bShouldGenerateSyncMarkers`를 켜야 한다 (GASP 기본은 꺼짐)
- [ ] `AM_MoveData_Speed` → `MoveData_Speed`
- [ ] `AM_BakePhaseCurveFromFootstepNotifies` → `phase` (루프·전환 클립만)
- [ ] PSD 편입 → MM으로 재생

**판정**: 발 미끄러짐 없이 재생되고, 워핑이 실제로 켜지는가
→ **전반부(반입·리타깃·루트모션) 통과. 후반부(커브 5종)가 남았다.**

#### 확보한 클립 (2026-09-03)

| 파일 | 내용 | 쓸 곳 |
|---|---|---|
| `~/Downloads/Rifle Aiming Idle.fbx` | Mixamo 표준 리그(65본, 루트=`mixamorig:Hips`, Root 없음), 스킨 포함, FBX 7700 | **리타깃 경로 검증 전용** + 3.4절 **42개 정지 포즈의 첫 앵커**(Stand·정면·0°) |
| (미확보) 라이플 견착 walk, **비인플레이스** | | **커브 검증([C-26]/[C-27])에 필수** |

> **왜 idle로는 커브 검증이 안 되나**: 이동이 없으면 ① `EncodeRootBone`이 옮길 이동이 없고
> ② 발 접지 구간이 없어 `contact_l/r`이 평평하며 ③ 회전·이동 오차가 0이라
> `AM_WarpingAlpha`가 **전 구간 1**을 뱉는다. 이게 바로 [C-27]의 고장 신호와 **같은 값**이라
> 정상인지 고장인지 구분할 수 없다. **판정력이 0이다.**

</details>

### ② P0-0 잔여 셋업 (반나절)

- [x] **플러그인 감사 완료 (2026-09-03, 이관 PC)** — `.uproject` + 엔진 `.uplugin` 대조

  | 상태 | 플러그인 |
  |---|---|
  | ✅ `.uproject`에 명시 | SmartObjects, GameplayBehaviorSmartObjects, AnimationBudgetAllocator, AnimationWarping, PoseSearch, Chooser, MotionTrajectory, GameplayInteractions |
  | ✅ 엔진 기본 활성 | **AnimationModifierLibrary**, AnimationSharing |
  | ✅ 의존성으로 따라옴 | StateTree, GameplayStateTree, **NavCorridor**, ContextualAnimation ← `GameplayInteractions`가 끌어온다 |
  | ❌ **꺼져 있음 — 켤 것** | **FullBodyIK**, **SignificanceManager**, **GameplayInsights**(Rewind Debugger) |

- [x] **플러그인 항목 완료 (2026-09-04)**
      · `FullBodyIK` — `IKRig`/`ControlRigModules` **의존성으로 이미 활성**
      · `SignificanceManager` — `AnimationSharing`(엔진 기본 활성) 의존성으로 이미 활성
      · `GameplayInsights` — `.uproject`에 추가. **Rewind Debugger 동작 확인됨**
      · ⚠ 브라우저 검색어는 `GameplayInsights`가 아니라 **"Animation Insights"**(FriendlyName)
- [ ] 프로젝트 설정 — NavMesh Dynamic, EQS 프레임 예산 3ms, Cover 트레이스 채널
- [ ] P4 워크스페이스 등록 + `p4 set P4IGNORE=.p4ignore` 후 초기 제출
- [ ] Rewind Debugger / Pose Search Debugger 켜서 동작 확인

### ✅ P0-1 · AI가 MM 구동 — **완료 (2026-09-04)**

> **판정**: `SandboxCharacter_CMC_C_1` + `AIC_Soldier` + `ST_Soldier_SmartObject`로
> **AI가 조준 방향을 유지한 채 이동**하고, 그 과정의 옆걸음·뒷걸음·급선회에서 **자세가 무너지지 않는다.**
> → **[C-1] 잠정 통과, [W4](TrajectoryGenerationData 재튜닝) 불필요.**
>
> **우리가 만든 것**(`/Game/SoldierLab/AI/`):
> `STT_SetSoldierInputState`(5개 축 노출) · `STT_FocusToPlayer` ·
> `ST_Soldier_Patrol_Subtree` · `ST_Soldier_SmartObject` · `AIC_Soldier`
>
> ⚠ **과대해석 금지**: GASP 비무장 DB는 8방향이 완비돼 있다. **데이터가 충분한 조건에서의 결과**이며,
> 전방 위주 소수 클립뿐인 견착 DB에서는 다시 봐야 한다 → **[C-44]**, P0-2의 본 판정.
>
> 전문: `ai/prototypes/2026-09-04_p0-1_ai_drives_mm.md`

<details>
<summary>진행 이력 (접힘)</summary>

### ③ P0-1 · AI가 MM 구동 (2~3일)

> **2026-09-04 조사: GASP에 이미 구현돼 있다.** 배선조차 대부분 불필요하다.
>
> `/Game/Blueprints/AI/` 에 `AIC_NPC_SmartObject` + StateTree 3종 + 태스크 6종이 있고,
> 그중 **`STT_SetCharacterInputState`** 가 핵심이다:
>
> ```lisp
> parent : StateTreeTaskBlueprintBase        vars : Character, WantsToWalk
> (event EventEnterState
>   (bind _pawn (CastToBPI_SandboxCharacter_Pawn (GetCharacter)))
>   (Setters|Set_CharacterInputState _pawn false (GetWantstoWalk) false false false))
> ```
>
> **AI가 플레이어와 똑같은 인터페이스(`BPI_SandboxCharacter_Pawn::Set_CharacterInputState`)로
> input state를 채운다.** 즉 AI → inputState → CMC → 궤적 → MM 경로가 **이미 하나로 통일돼 있다.**
> 3.1절의 "`USoldierTrajectorySource` 자체 구현 불필요" 결론이 에셋 수준에서도 확인됐다.
>
> **`BPI_SandboxCharacter_Pawn`의 다른 함수들도 우리 설계와 맞물린다:**
> `AddTarget` / `RemoveTarget`(표적 지정) · `Get_MMIResult` · `Get_PropertiesForAnimation`(ABP가 읽는 창구)
>
> → **P0-1은 "만드는 일"이 아니라 "확인하고 한계를 재는 일"이다.**

궤적 문제가 해결됐으므로 **배선 작업**이다.
- [ ] GASP 캐릭터에 AIController 부착, `inputState`를 AI가 채우도록 교체
- [ ] `SetFocus`/컨트롤 로테이션으로 조준 구동
- [ ] `PreviousDesiredControllerYaw` 공급
- [ ] 급선회 품질 확인 → `TrajectoryGenerationData` 재튜닝 **[C-1]**
  - ⚠️ Epic이 Steering 노드를 "작업 중, 원치 않는 거동 있음"으로 표기 — 첫 번째 의심 대상

</details>

### ✅ [C-34] 반입 매핑표 — **완료 (2026-09-04)**

> **`animation/prototypes/2026-09-04_c34_clip_curve_mapping.md` 4절이 확정표다.** 996클립 전수 실측.
> 클립 8종류 × 모디파이어 7단계. **대량 반입 전에 굳혀야 했던 항목이 닫혔다.**
>
> 핵심은 표가 아니라 **"Epic의 데이터에는 일관된 표가 없다"** 는 사실이다 — 같은 "걸으며 90° 회전"이
> 커브 3종/4종/6종으로 갈린다. 복제하지 말고 소비하는 쪽 기준으로 균일 적용한다.
>
> 부수 확정: **[C-36] 해결**(회전 클립에 `contact_l/r` 생성됨) · **[C-37] 확정**(스티어링 커브
> 생성기가 모디파이어 18개 어디에도 없음) · **[C-45] 해결**(→ 아래) ·
> 신규 **[C-46]**(`AM_Copy_IKFootRoot` 누락) · **[C-47]** · **[C-48]**
>
> **[C-45] 스티어링 커브 = 손 저작.** 클립 길이·회전각과 무관하게 항상 같다 —
> `steeringtargettime` **1.0 상수**, `enable_turninplacesteering` **1.0 → 0.0 @ 0.5s 계단**.
> 자작 모디파이어가 필요 없다. 레시피는 위 문서 5.0c절.
>
> **[C-48] ★ 회전이 안 끊기는 장치는 커브가 아니라 노티파이였다.** GASP 회전 클립에는
> `Pose Search: Block Transition In`(뒷구간 — MM이 **새로 진입 못 함**)과
> `Override Continuing Pose Cost Bias`(앞구간 — 유지 편향)가 붙어 있다. 우리 클립엔 없다.
> **P0-2에서 [C-44](급선회 품질)를 판정할 때 이게 교란 변수다** — 노티파이 없이 나온 결과를
> "견착 데이터 부족 탓"으로 오독할 수 있다.
>
> ⚠ 문서 두 곳을 정정했다: 매니페스트 8절의 "전 클립 공통 `contact_l/r`"과 "루프는 L/R 마커"가
> 둘 다 틀렸고(커브 0개 클립 179개 / 마커 보유 클립 4개), P0-4 문서의 PSD 설정은 [Q11] 전환 전 값이었다.

### ✅ 견착 전진 보행 — **정상 작동 (2026-09-09)**

> AI 병사가 견착 클립으로 **발 미끄러짐 없이 걷는다.** 원인이 하나가 아니라 **여섯 개가 겹쳐**
> 있었고, 전부 우리 쪽 에셋·설정 결함이었다. **GASP 시스템은 한 줄도 안 바꿨다.**
> 전체 기록: **`animation/prototypes/2026-09-09_walk_quality_debugging.md`**
>
> | # | 원인 |
> |---|---|
> | 1 | `ik_foot_*` 미채움 → Orientation Warping 무효 **[C-46]** |
> | 2 | Stride Warping 노드 부재 (설계 [2]층 요구사항, GASP엔 없음) |
> | 3 | **`bForceRootLock = false`** → 포즈에 루트 이동이 남아 루프 시 순간이동 ★최대 |
> | 4 | 루트 facing 34° 오차 (pelvis 블레이드가 루트에 구워짐) |
> | 5 | 진단 공식 90° 오류 — GASP 대조군이 `sd 0.0`으로 뱉어 발각 |
> | 6 | **레벨 인스턴스의 `WalkSpeeds` 오버라이드**가 CDO 변경 4회를 전부 가림 **[C-50]** ★ |
>
> **반입 파이프라인에 3단계가 추가됐다** — `SoldierRootFacingModifier`, `AM_Copy_IKFootRoot`,
> 그리고 클립 설정 2종(`bForceRootLock`, 커브 압축). 매핑표 4절에 반영.
>
> ⚠ **아직 우회책이 남아 있다** — MM 파라미터 3개와 PSD `Disable Reselection`을
> GASP 기본값으로 되돌릴 수 있는지 확인해야 한다 → **[C-49]**

### ★ 다음 작업 (2026-09-09 갱신)

| 순 | 항목 | 사용자 손이 필요한 부분 |
|---|---|---|
| 1 | **로우레디** — `AO_CD` 델타를 뼈 마스크로 상체에만 적용해 `BlendListByBool_0`의 비조준 분기에 꽂는다 | `BM_LowReady` 블렌드 마스크 생성 + 노드 3개 배치 (설정은 MCP) |
| 2 | **사격·재장전 배선** — 몽타주 95개가 이미 들어와 있다 | **소총 메시를 `ik_hand_gun`에 부착**하는 것이 선행 조건 |
| 3 | **[C-44] 급선회 판정** | ~~`rotationRate.yaw = 360` 상수가 GASP 기본값임을 확인했다~~ → **정정**: GASP 부모는 접지 시 **`(0, −1, 0)` = 즉시 회전**, 공중 200이다. 그리고 **2026-09-12부터 우리가 이 값을 매 틱 덮어쓴다**(무기 자세에 따라 90~720 °/s) → `IMPLEMENTED.md` 2.5d |
| 4 | **[C-58]** 옆·뒤·앉기 속도 실측 | 커브 호버 5회 |
| ~~5~~ | ~~**AI 상위 계층** — 인지·위협평가·엄폐·분대~~ | **✅ 절반 완료 (2026-09-13)** — 인지·무전·제압·교전·엄폐가 **초안이 아닌 다른 형태로** 들어갔다. **분대·명령은 그대로 남았다** |
| ~~6~~ | ~~★★ **목표(objective) 층**~~ | **✅ 코드 완료 (2026-09-13)** — `ASoldierObjective` + 세 비용 스코어러 → [D10] |
| **6b** | ★★ **목표 액터를 `L_SoldierTest` 에 놓는다** — 아군 소유 1개면 공격·수비가 동시에 생긴다. 그 전까지 6번은 **코드로만 존재한다** | **에디터 배치 1회** (⚠ [P53] — 기본값은 도구로 못 넣는다) → **[W25]**, 그 뒤 **[C-88]** |
| 7 | **데미지·체력·사망** — 병사가 죽지 않아 교전이 끝나지 않는다 | 클립 19개는 이미 반입돼 있다 → **[W18]** |
| 8 | **45명 성능 측정** | 시험 레벨은 있다(`L_SoldierTest`). 측정 절차가 없다 → **[C-83]** · [D3] |

> **[C-56] Lyra 원본 삭제는 취소됐다.** `Actions`에 사격·재장전·피격·사망이 들어 있었고,
> 앞으로도 조달처로 남는다. 컴파일 에러(`FootstepEffectTagModifier`)는 쿠킹 전에만 정리하면 된다.

<details>
<summary>이전 P0-2 정의 (접힘)</summary>

### ④ P0-2 재정의 · 견착 이동 품질 (2일)

> **선행 조건이 하나 붙었다 (2026-09-04)**: 견착 클립을 재생시키려면 **ABP/챙터 배선**이 필요하다.
> `rotationMode == Aim`은 상태값일 뿐이고, 어떤 DB를 조회할지는 `CHT_PoseSearchDatabases`와
> ABP가 정한다. **클립을 DB에 넣어도 챙터가 안 가리키면 안 뽑힌다.** 계획에 없던 작업량이다.
>
> **재료는 준비됨**: `Walking_Anim`(견착 walk, 커브 5종) · `Rifle_Aiming_Idle_Anim`(견착 정지,
> 리타깃 완료) · `Turning_Right_90_Degrees_Anim`
>
> **본 판정은 [C-44] + [C-24]** — 방향 커버리지가 부족한 견착 DB에서 급선회가 버티는가.

"워핑 각도 한계"가 아니라 **"전환 데이터 없이 견착 이동이 얼마나 버티는가"** 로 바뀌었다.
- [ ] 소총 메시를 `ik_hand_gun`에 부착, 임시 견착 포즈
- [x] 루프는 워핑으로 커버되는지 확인 — **Lyra 4방향 루프 확보로 워핑 의존이 줄었다**
- [x] **전환(출발/정지/급선회)에서 얼마나 무너지는지 측정** — Lyra가 Start/Stop/Pivot을 방향별로
      제공해 **조달 규모 문제 자체가 해소**됐다. 남은 것은 급선회(Spin) 클립뿐이다

</details>

### ⑤ P0-3 · 엄폐 슬롯 루프 (1~2일)

GASP의 `FindSmartObject → ClaimSlot → UseSmartObject` 중첩 패턴을 그대로 쓰고 **탐색만 EQS로 교체**.

---

## 5. 막혀 있는 것 / 결정 대기

| # | 항목 | 상태 |
|---|---|---|
| ~~Q4~~ | ~~소스컨트롤~~ | ✅ **Perforce(P4V) 확정** (2026-09-03) |
| ~~Q8~~ | ~~캐릭터 메시 (기존 리스킨 / 신규 / MetaHuman)~~ | ✅ **아군 확정 (2026-09-13)** — 기존 리스킨 `soldier_T` 를 `SK_UEFN_Mannequin` 에 Assign Skeleton. 적군은 **[Q42]** |
| **Q42** | **적군 메시 경로** — A) 디자인팀에 "UEFN 마네킹 리그 · 마네킹 비율 피팅 · LOD" 스펙 (권장, 아군도 같은 스펙으로 재납품) / B) Blender 리스킨 / C) 당분간 마네킹 | **결정 대기.** 참조 리그는 `SKM_UEFN_Mannequin` FBX export |
| Q10 | 견착 전환 동작 조달 방안 (A/B/C안) | P0-2 결과 후 |
| Q7 | 디자인팀 공지 시점 | P0 완료 후 비교 영상과 함께 |

---

## 6. 협업 상태

- 디자인팀은 현재 **기존 22종 시퀀스 정리 + 기존 스켈레톤 스킨 웨이팅 수정** 중
- 그 작업은 **낭비가 아니다** — `titan_example` 현재 빌드의 품질을 올리는 작업
- 리그 교체 제안은 **한 번 보류된 상태**. P0 검증 + 비교 영상을 만든 뒤 재제안
- 타임박스: **1주**. 안 되면 접고 현행 유지 (`assets/...` 11절)

---

## 7. 리스크

| 리스크 | 완화 |
|---|---|
| ~~`Enable_Warping` 커브를 자동 생성 못 하면 방향 커버가 0~~ | ✅ **해소** — `AM_WarpingAlpha` 확보 |
| ~~`contact_l/r`을 만들 수단이 없다~~ | ✅ **해소** — `UFootContactCurveModifier` 자작 **[C-33]** |
| 리타깃이 **좌우 발 높이를 다르게** 만든다 | 커브는 자동 보정으로 우회했으나 **발 IK 품질에는 남아 있다** **[C-35]** |
| 클립 종류별 커브 세트를 잘못 적용하면 조용히 틀린다 | **[C-34]** 매핑표를 대량 반입 **전에** 확정 |
| 견착 전환 데이터를 못 구하면 이동 품질이 무너짐 | 복수 DB 반환을 이용한 **점진적 폴백**(총내림 클립 혼합) — `animation/..._gasp_abp_analysis.md` 15.3절 |
| 리그 교체가 무산될 수 있음 | **설계 7~12절(AI 시스템)은 스켈레톤과 무관** — 전체의 75%가 생존 |
| Steering 노드가 실험적 | P0-1에서 급선회 품질 관찰 시 첫 의심 대상 |
