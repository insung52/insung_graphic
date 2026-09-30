# 구현 현황 — 실제로 만들어져 있는 것

> ★ **2026-09-14 — `titan_example` 편입 완료.** 아래 경로는 전부 `/Game/...` 그대로지만 **프로젝트가 바뀌었다**:
> `C:\working\kadex\titan_example`. C++ 는 `Source/SoldierLab/` · `Source/SoldierLabEditor/` 모듈 2개.
> 이관 중 생긴 변경(타입 개명 · cvar 개명 · `AC_VisualOverrideManager` 편집)은
> `migration/2026-09-14_titan_example_migration.md` 3.1 · 3.4 · 4.3 참고.

2026-09-30 / 유지보수 / **애니메이션 층(L4) 완료 · 무기/투사체 배선 완료 · AI 층 동작 확인 · 아군 메시(soldier_T) · ★ 적군 메시(new_enemy_T) 교체 완료 · ★ AI 전투 거동 2라운드(위험 지도 → 노출 회계) 완료 · ★ 체력·피격·사망 완료(병사가 죽는다, 아군은 무적) · ★★ 위험 지도 폐기 → 상황 필드(조명 모델) 3단계까지 PIE 확인 · ★ 09-18 필드 LOD(밉 + 퇴거) + 오버레이 v2 · ★ 09-18 오후 순찰(낡은 조망 비용) · 부채꼴 스캔 · 이동 강건성(solid/미지=열림/도달성/유예/Hold 기울기) PIE 확인, ~~저녁 얇은 엄폐 A/B/C 빌드 전~~ → ★ 09-18 밤 A/B/C PIE "잘됨" + 코너 멈춤 루프 수정 + **긴장도·걸음(`GetDesiredGait`) · 포즈 급박도(`GetPoseUrgency`)** 계약 → 포즈 층 3종이 소비, 전부 PIE 확인 · ★★ **09-21 분대 스코프 필드(PIE ✅) · 엣지 전진(코너 멈춤 대체, 빌드됨·PIE 대기) · AI 가 소유하는 사격 콘 + 버스트(PIE ✅, 무기 BP 미배선) · 섀도우 재캐스트 문턱 + riders(~~빌드 전~~ → 같은 날 빌드·PIE ✅) · ★ 09-21 늦게 **오버레이 노출 보정**(EV10 레벨에서 숯검정이던 오버레이 — `SoldierDebug::Bright` · `USoldierDebugMeshComponent` · cvar `Debug.ExposureScale`, PIE ✅; `Squad/`·`Pose/`·`Weapons/` 는 [W97]) · ★ **09-21 성능 계측 `stat SoldierLab` + off 스위치 7 → Cover Tick 8.92 → 1.92 ms(결정 보존 수정 넷), World Tick 28.5 → 22.6 — 남은 것은 다른 층([W98]~[W101])** · ★★ **09-21 분대 세션: New_kadex_0811 본 레벨이 SoldierLab 병사로 이관돼 첫 PIE 전 체인 완주("아주 잘됨") · 동사 `BreakContact` + 표적 제외 2소비자(2차 PIE 대기)** · ★ **09-22 titan 시나리오 재시작 리셋 계약 3종(`ResetForRestart` ×2 · `RecallAll`/`Park`) + `SoldierLab.ResetWorld`, L_SoldierScenario PIE ✅ (P187)** · ★ **09-23 `USoldierHealthComponent::EndPlay` 가 병사가 들고 있던 액터(소총)를 같이 파괴 — `bDestroyCarriedActorsOnDestroy`, 2-PC 실기 ✅ ([W116] 해결, P188)**.**
★★ **2026-09-30 — AI 조준 체인 재구성(떨림 해결, 사용자 판정) + 대각선 편향 임시 완화 + 맹목사격 임시 off** → `ai/2026-09-30_ai_real_pitch_and_aim_smoothing.md`(14절 요약) · `animation/2026-09-30_diagonal_aim_stop_selection.md`. **신규 C++ 3쌍**: `AI/SoldierAIController`(`ASoldierAIController::UpdateControlRotation` — AI 컨트롤 회전 = 교전 `GetAimRotation()` 피치 포함, cvar `SoldierLab.Aim.RealPitch`; **`AIC_Soldier` 부모 = 이것**, 옛 부모 `AIController`) · `Pose/SoldierAnimLibrary`(`USoldierAnimLibrary::UpdateStanceAimOffset` — 서기↔앉기 AO 관성 전환 0.25 s, ABP `OnUpdate_StanceAimOffset` = `BlendSpacePlayer_1` On Update, cvar `SoldierLab.Pose.AimOffsetInertialSwitch`, [W11] 해결) · `Debug/SoldierAimTrace`(`USoldierAimTraceSubsystem`, `SoldierLab.Debug.AimTrace`). **수정**: `SoldierEngagement` `SlewAim`(오차 ≤ 3° 40 · ≥ 30° 150°/s, `AimSlewDegreesPerSecond` 240 → 150, UPROPERTY `AimSlewNear*`/`AimSlewFarErrorDeg`) + `SoldierLab.Engagement.BlindFire`(기본 0 — `PlanAperture` Blind 후보 3종 제외) · `SoldierAIBridgeComponent` 조준 보정 재설계(2.5c 추기) + 웅크림 슈미트 트리거 + AI 조준 on/off 속도 밴드·최소 유지(헤더 `LastAimToggleSeconds`) + AI 몸통 회전 상한 · `SoldierPoseSmootherComponent::StepAxis` 착지 · `Build.cs` + `PoseSearch`/`BlendStack`/`AnimGraphRuntime`. **에셋**: 재장전 몽타주 2 BlendIn 0.25 · 피격 Med/Hvy 12 BlendIn 0.05 · 스켈레톤 슬롯 그룹 `HitReact`(`AdditiveHitReact` 분리) · `BP_AR4Rifle` 마지막 Delay 0.9(사격 금지 2.2 s = 재장전 몽타주) · 대각선 임시 편향 PSD 8(Stops `baseCostBias 0.2` · Loops `continuingPoseCostBias −0.05`). 원칙 P195~P200. ★ 같은 날 왼손 그립 소켓 회전 캡처 완료(2.5절 추기).

★ **2026-09-22 — 시나리오 재시작 리셋 계약(titan 세션이 이 모듈에 추가, L_SoldierScenario PIE ✅)** → `../level_new_kadex_0811/2026-09-22_scenario_restart_implementation.md`(구현) · `../level_new_kadex_0811/2026-09-10_scenario_auto_restart_design.md` 4a절(계약). titan 의 시나리오 재시작은 레벨 리로드가 아니라 **같은 월드의 인플레이스 리셋** — 병사(`USoldierHealthComponent` 보유 액터)는 titan `UScenarioRespawnSubsystem` 이 스냅샷(클래스·트랜스폼·인스턴스 저작 델타 `SquadId`/`bSquadLeader`/`bInvincible`)으로 Destroy → 재스폰하고, **살아남는 SoldierLab 월드 서브시스템은 이 모듈이 소유한 리셋 함수**로 되돌린다: `USoldierSquadSubsystem::ResetForRestart()` · `USoldierSituationFieldSubsystem::ResetForRestart()` · `USoldierProjectilePoolSubsystem::RecallAll()` + `ASoldierProjectile::Park()`. 콘솔 `SoldierLab.ResetWorld`(셋 한 번에, 시험 레벨 단독 검증). **원칙 P187** — 서브시스템에 새 상태를 두면 리셋에도 넣는다. 함정 둘(titan 쪽에서 해결): `BP_Soldier_*` 의 `AutoPossessAI=PlacedInWorld` 는 **스폰된** 폰에 컨트롤러를 안 붙인다(재스폰은 지연 스폰 중 `PlacedInWorldOrSpawned` 로 바꿈) · 병사만 Destroy 하면 병사가 스폰한 `BP_AR4Rifle` 액터가 남아 `OwningCharacter is not valid` 스팸 + 사이클마다 두 배(딸린 액터 동반 파괴). ~~평상시 사망 `DestroyAfterSeconds` 뒤 소총 잔존 여부는 **미확인 → [W116]**~~ → ↓

★ **2026-09-23 — 병사가 스폰해 들고 있는 것은 병사가 치운다: `USoldierHealthComponent::EndPlay` + `bDestroyCarriedActorsOnDestroy` ([W116] 해결, 원칙 P188)** → `../level_new_kadex_0811/2026-09-22_scenario_restart_implementation.md` §5. 소총 누수는 재시작만의 문제가 아니라 **평상시 사망 경로에도 있던 것**이었다 — 확정 근거 넷: ① `BP_SoldierCharacter` 가 BeginPlay 에서 소총을 `SpawnActor` 해 `Rifle` 에 들고 있는데 그 BP 에 `K2_DestroyActor` 노드도 `EndPlay` 핸들러도 **0개**(`BP_Soldier_Hostile/Friendly` 도 0) · ② `USoldierHealthComponent` 는 `DestroyAfterSeconds` 뒤 `Owner->Destroy()` 만 · ③ 엔진 `UWorld::DestroyActor` 는 **자기 Owner 만** 비우고(`Engine/Private/LevelActor.cpp`) 자기가 소유한 액터는 어태치만 끊고 살려 둠 · ④ `BP_AR4Rifle` 은 **`bReplicates=false`**(CDO 실측)라 프로세스마다 자기 것이 있어 서버가 치워도 클라 것은 남는다. **수정**: `EndPlay` 오버라이드가 `EEndPlayReason::Destroyed` 일 때만, 자기가 어태치(`GetAttachedActors` 재귀)하거나 소유(`Children`)한 액터를 같이 파괴(`bDestroyCarriedActorsOnDestroy`, EditAnywhere, 기본 true). **BP 수정 0건**으로 모든 파괴 경로·모든 프로세스를 덮는다. 예외 둘 — 풀링 투사체(`ASoldierProjectile`: 풀이 포인터를 쥐고 Owner 만 사수로 바꿔 쓴다) · 클라이언트의 **복제된** 액터(`ROLE_Authority` 인 것만). 순서상 안전: `DestroyActor` → `Destroyed()` → `RouteEndPlay` → 컴포넌트 `EndPlay` 가 어태치 해제·`SetOwner(NULL)` **이전**. titan 재시작 쪽 정리(`DestroySoldierAttachments`/`DestroyOrphanedChildActors`)는 **안전망**으로 남았다(낙하산 등 병사가 아닌 재스폰 대상용). New_kadex_0811 **2-PC 실기 검증 ✅**(사용자 "해결됨").

★ **2026-09-28 — 나뭇잎이 시야를 가린다: `AI/SoldierFoliageOcclusion`(신규) + `SoldierSight` 어른거림 누적 (titan 숲 세션이 이 모듈에 추가)** → `../nanite/2026-09-28_forest_nanite_foliage_migration.md` E절(titan 루트 기준). New_kadex_0811 숲이 Megaplants(Nanite Foliage)로 바뀌면서 titan 설계 문서 `../level_new_kadex_0811/2026-09-01_foliage_occlusion_ideas.md` 안 2 를 구현. **[A] 코드·빌드·등록**: `USoldierFoliageOcclusionSubsystem`(`UTickableWorldSubsystem`) — 수관 = 세워진 타원체(`FSoldierCanopy`), 평면 2D 격자(CSR, 셀 10 m), `ComputeTransmittance(From, To, Cutoff, OutFractionAtCutoff)` = DDA 로 셀을 걷고 `exp(−Σ 감쇠 × 길이)`, 광학 깊이 5 에서 중단, **트레이스 0 — 물리적으로 이미 트인 선분에 "얼마나 잘"만 답한다**. 이 모듈은 나무 출처를 모른다 — titan `UForestCanopyRegistrarSubsystem` 이 BeginPlay 에 `RegisterCanopies(SourceId, …)` 로 채운다(모듈 의존이 titan → SoldierLab 한 방향이라 계산은 여기). PIE 로그 `[Foliage] Registered 39482 forest crown(s)`. cvar `SoldierLab.Foliage.Enabled`(0 = 잎 투명 A/B) · `SoldierLab.Debug.Foliage`(60 m 안 수관) · `SoldierLab.Foliage.CellSizeCm`, `stat SoldierLab` 에 `Foliage Queries`/`Foliage: Queries`(`SoldierLabLog`). **`SoldierSight`**: `FoliageClearTransmittance 0.5`(이상이면 예전처럼 즉시 목격) · 아래면 **어른거림 누적** `Noticed += T × 경과(≤0.25 s) / FoliageNoticeSeconds 0.6` → 1 이면 목격, `FoliageMinTransmittance 0.05` 미만은 무시, `FoliageGlimpseMemorySeconds 1` 공백이면 초기화 · 필드 비우기(`MarkClearAlongRay` — 콘 스윕·표적 선·빈 땅)는 T 가 0.5 밑으로 떨어지는 지점에서 멈춤 · 시체 확인(LearnDeath)·`ReportClearView` 도 T ≥ 0.5 · **`SweepRays`(엄폐 층 엣지 모양)는 물리 그대로 — 잎은 은폐지 모서리가 아니다**. 같은 서브시스템을 titan 차량 탐지(`UTargetDetectionComponent`)도 쓴다(`../guide/detection_dev_guide.md` 3.4a). **[B] 한계**: 성목 수관 하단이 지면 5~6 m 라 선 병사 눈높이(1.6 m) 시선은 대부분 수관 밑 — 지상에선 묘목·줄기 프록시만 가리고 하층 고사리는 미등록. **[C] 값 튜닝 미실시**(`FoliageNoticeSeconds`·수종 감쇠) — 사용자 "잘 된다" 뿐, 측정 항목 = 숲 가장자리 적 발견 시간 전/후 비교. *OPEN_ITEMS ID 는 부여하지 않았다(다음 AI 세션이 필요 시 등록).* 새 원칙 번호 없음.
★★ **2026-09-21 — New_kadex_0811 이관 · `BreakContact` · 표적 제외 (분대 세션)** → `../level_new_kadex_0811/2026-09-21_soldierlab_migration_new_kadex_0811.md` · `squad/2026-09-21_break_contact_and_targeting_exclusion.md`. 구 `BP_Enemy_kadex_1~15`/`BP_Ally_kadex_1~25` 삭제 → `BP_Soldier_Hostile_1~15`(분대 1/2/3)/`BP_Soldier_Friendly_1~25`(1~5) 같은 트랜스폼, 마커 113개·경로 스플라인·드론 경로·낙하산·트럭·UGV 유지 · **`ASoldierZone` 8개** 분대별(적 3+2+1 — 분대별 `NavQueryFilter_EnemySquad1/2/3` 를 존이 나른다, 아군 북/남) · `ScenarioConfig_1.SquadZones` · DT **`DT_ScenarioSteps_ThreeStage_SoldierLab` 26행**(`ThreeStage` − Retarget 2·HoldFleeingFire·AllyAmbush + AllyDefend·AllyEngage·Squad3Run·Squad3Stand; 적/아군 행 `IssueSquadOrder`, 드론·UGV·트럭 행 그대로). 첫 PIE(04:38 UTC): EnemyApproach +1 → UAVSpotted +82 → UGVArriveZone1 +170 → EnemyEngage +180 → Flee2 +210(`reinforce quota=10 living=7 needed=3 moved=3`) → AllyEngage +260 → UGVMoveZone2 +274 → Flee3 +350 → CommandPostFire → ExcludeFleeingEnemies. **문제**: 3분대 3차 "도주"(`Withdraw ReturnFireOnly`)가 엄폐 홉 후퇴 + UGV 가 제외 뒤에도 대타를 쏨 → 원인 셋(RCWS 스위치 반쪽 누락 · SoldierLab 아군이 제외 플래그 안 읽음 · Withdraw 는 전투 이동) → **`ESoldierOrderVerb::BreakContact`**(`IsBreakingContact()` 이면 `ScorePosition` 의 Fighting/Route/Danger/Suppression 0 · dwell 0 · 스프린트 · 자세 0, 존 안은 평소 Hold — 도주는 ROE 가 아니라 땅의 가격 P184) · **`IsContactExcluded`**(후보 제외·잠금 해제, 인지 불변 P185) · `IssueSquadOrderSpec` `SetTargetable(false)` → UGV `bRespectEnemyTargetingExclusion=true`(트럭 제외) · DT `Squad3Run`(Flee3 +6 s BreakContact HoldFire Rush) · `Squad3Stand`(트럭 80 m 사격 → Occupy z2 Free). `GM_SoldierLab` 에서도 시나리오는 돈다(GameInstance 서브시스템, PC CDO 폴백 — RTSP/HUD/토스트만 잃음). 1v1 `L_SoldierTest` DT 5행 기록. 값 [C-163]~[C-164], [Q51], [W104]~[W106]([W106] = 문서 세션 발견: RCWS 스티키 표적이 제외를 안 봐 물고 있던 한 명은 시야를 1 s 잃어야 놓는다), **[W70] 해결**. 빌드 함정: `UFUNCTION` 없는 private 헬퍼를 다른 컴포넌트에서 → C2248.
★ **2026-09-21 — 성능 계측 · 엄폐 틱 비용** → `ai/2026-09-21_perf_instrumentation_and_cover_cost.md`. 35명(`L_SoldierScenario`)에서 World Tick 1.7 → 29 ms 인데 `stat game` 은 6 ms 만 이름을 댔다 → **`STATGROUP_SoldierLab`**(`AI/SoldierLabLog.h`, `stat SoldierLab` — 시스템 틱마다 사이클 카운터, Cover 하위 7 · Field 5, `Traces: Sight/Cover/Engagement/Field` · `Soldiers Ticked`) + **`SoldierLab.<Sight|Perception|Cover|Engagement|Suppression|Comms|Field>.Enabled`** off 스위치 + **`SoldierLab.Cover.Avoidance`**(RVO 런타임 A/B). 첫 계측 **Cover Tick 8.92 ms · 487 트레이스/프레임 = 프레임의 1/3**. 수정 넷(전부 결정 보존 — 같은 눈·후보·비용 함수): (a) **이동 중 스윕 정지 `bSweepSuspended`**(`FinishSweep` 의 `bAlreadyGoing` 과 같은 식 — 어차피 버리던 스윕, 정지하면 새 발·새 눈으로 새 스윕) + 볼 곳만 **`UpdateWatchPoint()`** 로 `WatchRefreshSeconds 0.25` 마다 (b) 발밑 HERE 재평가 매 틱(27 트레이스) → **`HereEvalIntervalSeconds 0.1`** ∨ 걸음 > `MicroStepCm` ∨ 새 스윕 (c) **경로 가지치기** — 경로 없이 best 에 지면 경로 트레이스 생략(경로 비용 ≥ 0 이라 정확) (d) 눈 0 스윕 **`CalmCandidatesPerTick 2`**(트레이스 0 이라 후보 전체가 매 틱 돌던 것). → **Cover 1.92 ms · 243 트레이스**(Candidate 1.29 / Score 0.51 / Begin 0.43 / Route 0.35 / Here 0.15), SoldierLab 합 ≈ 3.3, **World Tick 28.5 → 22.6**(프레임 31, GPU 6 — 게임 스레드 바운드). RVO A/B 0.3~0.5 ms → 켜 둠; "켬" 캡처의 **`DispatchBlockingHit` 58 ms 히치**는 별건(무기 `OnHit` 동기 로드 의심 [W101]). 남은 게임 스레드: 애니 ≈ 7.3(`BlueprintUpdateAnimation` 3.47 · 키네마틱 본 0.97 → [W98] 포즈) · 이동/트랜스폼 4~7(컴포넌트 ≈ 23/병사 · 오버랩 → [W99] 캐릭터 BP) · BP 틱 ≈ 2.5([W99]) · 투사체 스폰 0.65([W100]) · 이 층 다음 몫 ≈ 1 ms([W102], 다른 층 뒤) · 분대 스코프 [W103]. 워커 19.7 ms 는 병렬(대기 1.15 만). 원칙 **P182~P183**, 값 [C-162].
★★ **2026-09-21 — 분대 스코프 필드 · 엣지 전진 · 사격 콘 · 섀도우 감축** → `ai/2026-09-21_per_squad_field_edge_advance_fire_model.md`. ① 상황 필드가 **분대 하나당 하나**(`FScope` = 진영 × 분대 슬롯, `USoldierFieldSettings::MaxSquadsPerFaction 3`, 슬롯은 처음 말하는 순서·넘치면 슬롯 0 공유 + 분대당 1회 경고, 공개 API 전부 `const USoldierIdentityComponent* Who`, 호라이즌만 세계 공유, 무전 `ReceiveSharedRecord` → 받는 분대 필드에 **관측 시각**으로 `ReportSighting`, **`bTakesSquadOrders=false`(UGV 표적 Identity)는 스코프·앵커·관찰 대상 아님** — 오버레이가 UGV 를 중심으로 잡던 원인, cvar `Debug.Field.Squad`/`.Centre`, 헤더 `HOSTILE/1`) · `L_SoldierScenario` 아군 4 → **3분대(7/7/6)**. ② **엣지 전진** — 09-18 코너 멈춤(`UpdateCornerPause`·`CornerStopCm`·`CornerPauseSeconds`) **삭제**, `USoldierSightComponent::GetSweepRays()`(콘 스윕 광선 보관)에서 엣지(옆 광선은 멀리 가는데 ≤ 10 m 에 멈춘 광선)를 읽고 `USoldierSituationFieldSubsystem::GetWedgePresence`(쐐기 안 경계도 × m², 구운 호라이즌으로 가림, **트레이스 0**)로 한 걸음이 여는 양을 값 매겨 `StepPresenceBudget 6` 안에서 가장 멀리 가는 걸음(60 cm × 1..4, ±105° 부채꼴)을 딛고 연 쐐기의 글로우가 `AnalyzedPresence 1` 아래로(또는 `MaxLookSeconds 3`) 내려가면 다음 — 호도 타이머도 없다(P176·P177). 접촉이면 그 자리 정지 + 걸어온 자리를 10 s 엄폐 후보로. 교전: `GetAdvanceView`(쐐기 조준 + 벽 쪽 린 0.7) · `bWantsToAim = 접촉 ∥ 전진` · 전진 = Walk · `UrgencyLookPeek`. **빌드됨(CL 500), PIE [W95]**. ③ **사격 콘을 AI 가 소유**(P178) — `GetShotSpreadDegrees()` = `WeaponSpreadDegrees 0.8`(← 3) × `(1 + MovementSpreadScale 6 × v/600)`(← 2) × 자세(`Lean 1.5`/`Blind 15`, ← 1.4/4 — 가치 사거리 보존) × (1+반동) + **`AimSettleDeg`**(선회 `AimSettleInitialDeg 2.5` → `exp(−dt/0.4)`, 발마다 `+RecoilKickDeg 0.6`, 이동 바닥 `MoveWobbleDeg 1.5`) · 조준 게이트 = 기록 반경 ≤ `TargetRadiusCm 45 × AimedHitTolerance 2` 이면 지금 콘 → `Aimed` / 정착 콘 → **`Settling`** / 예비 → `Suppressive` · **버스트** `BurstRoundsMin 2..Max 5`(제압은 Max) · `BurstPauseSeconds 0.5 × (1 ± RhythmJitter 0.35)` · `FRandomStream` 이름 시드 → **`Pacing`** · `[Engage]` 꼬리 `cone wobble burst next` · `KnowledgeToSpreadRatio` 삭제. PIE ✅. ⚠ **무기 BP 가 아직 고정 콘, `BP_SoldierCharacter` 는 `HasContact()` → [W93]**. ④ **섀도우 감축 [B]** — `FLight::ShadowEye/ShadowCastTime`, 따라가는 라이트는 `ShadowRecastMoveCm 0`(= 한 셀) ∧ `ShadowRecastSeconds 0.5` 뒤에만 재캐스트, 얼면 즉시, 반 셀 안 다른 분대 라이트는 **riders** 로 한 벌의 트레이스에 동승, 헤더 3줄째 **비용 줄**(감축 전 실측 `shadows 0.03 ms · 0 waiting · 96 alive` = **`MaxLights 16` 상한** [W92]). 원칙 **P176~P180**, 값 [C-157]~[C-161], 작업 [W92]~[W96], **[W74]·[W85] 해결**. ★ **같은 날 늦게**: ④ 정식 빌드 · PIE ✅ "아주 잘됨"(**[W96] 해결**, 재측정 동일 0.03 ms · 0 waiting · 96 alive — 96 은 alive 지 waiting 이 아니다) · `L_SoldierScenario` 3분대 재편 MCP 저장 확인 · ⑤ **오버레이 노출 보정** → `ai/2026-09-21_debug_overlay_exposure.md`: 디버그 프리미티브는 톤매퍼 앞이라 EV10 고정 레벨에서 선형 1.0 = 숯검정, `DrawDebug*`·배처 `DrawMesh` 는 8-bit → 레벨 무변경, **뷰 노출의 역수만큼 밝게** — `SoldierDebug::GetExposureScale`(씬 뷰 익스텐션이 `GetLastEyeAdaptationExposure` 읽음) · `Bright(FColor) → FLinearColor` · 래퍼 `Line/Point/Sphere/Circle` · 신규 **`USoldierDebugMeshComponent`**(`AI/SoldierDebugMesh`, 필드 사각형·링) · cvar `SoldierLab.Debug.ExposureScale`(0 자동) · `Build.cs` + `RenderCore`/`RHI` · `AI/` 24곳 교체. 원칙 **P181**, 작업 **[W97]**(`Squad/` 9 · `Pose/` 7 · `Weapons/` 1 — `Arrow` 래퍼 필요). 엔진 내비메시 `P` 뷰는 비목표.
★ **2026-09-18 — 상황 필드 2일차**: 3단계(호라이즌/앰비언트) 빌드·PIE("딱 내가 원하는 그림이 이제 나옴") → **밉 + 다중 앵커 퇴거**(`LevelCount 3`, 거친 레벨은 레벨 0의 집계 `FCoarseCell`/`FCoarseHorizon`, 병사 아무나에게서 `DetailRadiusCm 8000 × 4^L` 밖 디테일은 부모에 **잔여물**로 접고 해제 — 메모리가 지나간 자리가 아니라 병사가 있는 자리에 묶임, `SoldierLab.Field.CellSizeCm` cvar) → **오버레이 v2**(자체 `ULineBatchComponent` 0.1 s flush+refill, 색별 메시, `Debug.Field.Level −1` 클립맵 링, 불투명도 = 신선도, 반경 12000, 대칭 캡 6000, 헤더 2줄, 라이트 색 = 출처, 볼 곳 화살표) → **라이트 부정 증거**(`ClearViewHalfLifeSeconds 4`). 섀도우 지면 아래 허용 −110 → `GroundSlackCm 40`. LOD 링·배처까지 PIE 확인, **대칭 캡·라이트 v2·부정 증거·헤더 2줄·파랑끼는 빌드 전 [B]**. → 시스템 문서 **16~18절**, 원칙 **P152~P157**, 값 [C-140]~[C-143], 작업 [W79]~[W83]([W83] = 들은 라이트 필터 주석·코드 불일치).
★ **2026-09-17~18 — AI 포즈 층 3종 (포즈 세션) + 관전 폰 추기** → **`animation/2026-09-18_ai_pose_layer_scanturn_gait_smoother.md`**. AI 세션이 발행한 계약의 **소비 측**이 들어갔다, 전부 `Source/SoldierLab/Pose/` · `BP_SoldierCharacter` 컴포넌트, **AI 전용**(`IsPlayerControlled()` 면 반환): **`SoldierScanTurnComponent`**(`AC_SoldierScanTurn` — 총 내림 ∧ 정지 ∧ `IsScanning()`/`HasContact()` 면 캡슐 yaw 를 `GetAimPoint()` 로 20°에서 시작 5°에서 멈춤 180°/s, 메시는 GASP OffsetRootBone+MM TIP) · **`SoldierGaitBridgeComponent`**(`AC_SoldierGaitBridge` — `GetDesiredGait()==Walk` → `CharacterInputState.WantsToWalk` 리플렉션 매 틱, GUID 접미사 P170, Sprint 는 BP 소유) · **`SoldierPoseSmootherComponent`**(`AC_SoldierPoseSmoother` — stance/lean/BF-H/BF-V 를 **사다리꼴 프로파일**(축별 `MaxSpeedUp/Down/Acceleration`, Stance 1.6/0.9/5 = 앉는 쪽이 빠름) × `GetPoseUrgency()` 로 `ScaleAtCalm 0.5`~`ScaleAtUrgent 1.6`, 목표 변경 시 속도 연속, BP 상태+AI 목표 변수 동시 쓰기 + 액터 틱 선행 P168, BP 램프 rate 는 **0.0001 로 얼림** — 0 이면 `RampAxisTo` 가 목표를 돌려줘 급할 때 1프레임 스냅 **P167**). cvar `SoldierLab.Debug.ScanTurn` · `.PoseSmooth`(`ext` 감사). **셋 다 사용자 PIE 확인**(ScanTurn 돎 · 걷기 나옴 · 스냅 해결 "해결완료"). **H 는 AI 에 자동으로 안 켜진다**(09-17 `bEnableForAI` 되돌림, P171). 관전 폰: 자유 비행 **휠 = 비행 속도**(`FlySpeedCms 1200` ×/÷1.25) · `bIgnoreTimeDilation`(slomo 무관) · **롤 잔류 수정**(컨트롤 회전은 yaw·pitch 만, P169). 값 [C-154], 작업 [W91], **[C-152] 포즈 측 확인**.
★ **2026-09-18 오후~저녁 — 순찰 · 부채꼴 스캔 · 이동 강건성 · 얇은 엄폐** → `ai/2026-09-18_patrol_scan_and_move_robustness.md`. 사용자 보고("적이 죽은 뒤 아군이 가만히, 화살표가 몸과 반대")의 원인 넷: **섹터는 부채꼴**(필드가 arc 안에서 가장 안 훑은 방위, `GetMostExposedDirection` 편향/arc 파라미터) · 콘 스윕 **띠 넓히기**(16 m 밖 3셀 폭) · **볼 곳은 항상 계산**(눈 유무 무관, 쓸지는 교전 층) · 교전 층이 **`GetAimPoint()`/`IsScanning()` 발행** → 포즈 세션 `Pose/SoldierScanTurnComponent`가 몸을 돌림. **순찰 = 낡은 조망 비용**(`GetStaleVantage`, 존/목표 `PatrolWeight 1.0`·`PatrolStaleSeconds 30`, 눈 0일 때만 — 경로 없음). 로그로 잡은 이동 결함 셋: 큐브 꼭대기 셀(**`FHorizon::bSolid`** + `MoveTo` 전 **`FindPathSync` 부분 경로 불허** + `RejectCandidate 30 s` + **`MoveGraceSeconds 0.75`**) · 벽 꼭대기(**`GetExposureByStance` → bool, 미지 = 열림**) · 밴드 밖 평평한 Hold(**기울기 계속**). CQB `WatchTravelBias 1`·`WatchApproachBias 0.5`·`ScanDwellSeconds 2`. **`MinStance`**(배정/명령, Rush 무시 — titan DT 미연결). [W83] 해결. **오후 묶음 빌드·PIE — "잘 되는 거 같음. 이제 정상적이다."** 저녁 **A/B/C(활동도 가중 은폐 `HiddenGazeFraction 0.5` · 미세 위치 `MicroStepCm 30`/`CoverAcceptanceRadiusCm 20` · 코너 멈춤 `CornerPauseSeconds 0.8` + 굽이 너머 미리 보기)는 ~~빌드 전 [B]~~ → 밤 빌드·PIE "잘됨"** — 진짜 파이 자르기(경로 모양)는 아님. ★ **밤 4차**(같은 문서 12~17절): 첫 빌드 로그의 **코너 멈춤 루프**(스폰 옆 굽이에서 0.82 s 마다 `MOVE` 60 s — 굽이 기억이 경로 인덱스 + 재개 뒤 유예 만료 → `LastPausedCornerLocation` 자리 기억 + 재개 시 `LastMoveIssuedSeconds = Now`, P173·P174) · 눈 나타나면 즉시 재개 · `RejectedCandidates` 만료 정리([W90]) · `IsScanning()` 섹터만 있어도 true([W89]) · `GetExposure` 미지 = `UnknownPresence × AmbientWeight`(0 아님) · **긴장도 `GetTension()`**(알람 = 접촉 ∥ 제압 ∥ 사선 거부 ∥ 1 s 안 총성, `TensionHalfLifeSeconds 20`) + **걸음 `GetDesiredGait()`** Walk/Jog/Sprint(Jog = 접촉 ∨ 긴장 ≥ `JogTension 0.3`, Cautious 무접촉 = Walk — "조용한 경비는 걷는다", P175) · **포즈 급박도 `GetPoseUrgency()`**(`Urgency*` 7값 max + 제압 — "AI 는 목표 + 숫자 하나, 움직임은 포즈 층", P172) · `[Engage]` 로그 끝 `tension gait urg`. 원칙 **P158~P166 · P172~P175**, 값 [C-148]~[C-153] · **[C-155]~[C-156]**, 작업 [W85]~[W88]. ~~ScanTurn 연동 미확인.~~ 포즈 세션 확인, 걸음·급박도 소비도 PIE "잘됨".
★ **2026-09-18 — 분대 명령 층이 빌드되어 시험 레벨 `L_SoldierScenario`(적 15·아군 20·UGV·트럭, 드론 없음)에서 처음 돌았다** → `squad/2026-09-18_squad_layer_fixes_quota_engage_range.md` · `../level_new_kadex_0811/2026-09-18_soldierlab_three_stage_test_level.md`. 첫 PIE 수정: 도착선 일원화(`ASoldierZone::ArrivalFraction` → 배정, `ArrivalInsetFraction` 삭제, Approach 비용 `1 + 거리/scale` 계단) · 존 **에디터 표시**(구 3개 + 섹터 화살표 + 스프라이트) · `bTakesSquadOrders`(차량 제외) · **정원제·대타** `ReinforceSquads` + DT `Quota` · **`EngageRangeCm`**(ROE 사거리 — 적군이 스폰 지점에서 92 m 밖 UGV 를 쏘던 것) · 트리거 `EnemyFireStarted`/`EnemyNearFriendlySoldiers` · RCWS 청각 ← SoldierLab 총성(`OnGunshot`) · `LogTick` 순환 경고(CMC `bTickBeforeOwner=false`, 2.5d-1 정정, [C-146]) · 총구 `DrawDebugCoordinateSystem` 해제. 완주(ScenarioComplete)는 아직 [C]. 1차 전투지 위치 [Q50].
★★ **2026-09-17 저녁 — 위험 지도(`SoldierDangerMap`)가 삭제되고 상황 필드 `USoldierSituationFieldSubsystem`(`AI/SoldierSituationField`)이 들어섰다** → **`ai/2026-09-17_situation_field_lighting_model.md`**. 위험도를 **저장하지 않는다** — 목격(라이트)이 그림자를 긋고 안 본 땅(사전값 0.5)이 앰비언트다. 값은 **Project Settings → Game → SoldierLab Situation Field**(`USoldierFieldSettings`, `DefaultGame.ini`). 오버레이 `SoldierLab.Debug.Field`(월드당 1회). `LogSoldierAI`는 `AI/SoldierLabLog`로 이사. 잠입(사전값·콘 스윕·스프린트 규칙·볼 곳) → `ai/2026-09-17_infiltration_and_unknown_ground.md`. ~~**1·2단계 + 오버레이는 PIE 확인, 3단계(앰비언트·필드 후보·필드 자세)는 빌드 전 [B].**~~ → 09-18 전부 PIE 확인. 같은 날 오전분(위협 보너스·사선 거부·히스테리시스·차량 높이)은 `ai/2026-09-17_threat_bonus_lane_denied_vehicle_heights.md`. 새 원칙 **P143~P151**.
★★ **2026-09-17 — 애니메이션이 진영별 2벌이 됐다. 이 문서의 클립·PSD·Chooser 경로가 전부 바뀌었다** → **`animation/prototypes/2026-09-17_ally_enemy_anim_set_split.md`**. `/Game/SoldierLab/Animations_Enemy/`(`Enemy_`) · `/Game/SoldierLab/Animations_Ally/`(`ALLY_`) 각 247개, 옛 `Animations/`·`PoseSearch/` 는 **소멸**. 스켈레톤·ABP 는 **1벌 그대로**이고 ABP 안에서 `UseAllyAnimSet` 이 **5축을 분기**한다(2.4b). 작업 기준 세트는 **적군**.
★★ **2026-09-17 — 피격·사망이 실제로 화면에 나오기 시작했다** → **`ai/2026-09-17_hit_death_three_causes.md`**. 09-15 에 배선은 끝났으나 **원인 셋**(C++ 생성자의 경로 하드코딩 · 배치 인스턴스의 빈 배열 · 사망 슬롯 `FullBody` 부재)으로 한 번도 재생된 적이 없었다. 몽타주는 이제 **BP 데이터**이고 사망은 **순수 래그돌**(몽타주 OFF), 피직스 에셋은 두 메시 다 `PA_UEFN_Mannequin`. 새 원칙 **P134~P141**.
★ **2026-09-14~15 애니메이션 정리 라운드**: 적군 메시 · 왼손 IK 토글(기본 OFF) + 그립 오프셋 런타임 산출 · 급선회 스냅 해결(`maxRotationError` −1 복귀, [C-80]) · BF 머리 부풀기 해결(`ModifyBone_8`) · 총 오프셋을 메시 소켓으로 · 총내림 클립 2차 시험 실패 → `animation/prototypes/2026-09-15_sharp_turn_pop_bf_head_scale_and_weapon_socket.md` · 디자이너 가이드 완성 `assets/2026-09-14_designer_guide.html`.
★ **2026-09-15 저녁 — 체력·피격·사망**: `AI/SoldierHealth` C++ 컴포넌트 하나 + ABP `AdditiveHitReact` 슬롯 경로 + BP 총구 보정 게이트 AND. 사용자 PIE "잘됨", 수치는 [C-110]~[C-118] → `ai/2026-09-15_health_hit_death_implementation.md`
AI가 GASP 몸을 실제로 운전한다 — 인지·시야·무전·제압·교전·엄폐·**목표** + **위험 지도 · 부채꼴 후보 · 표적 잠금 · 엄폐↔사격 사이클(노출 회계) · 실제 총구 소켓·포즈별 오프셋 학습** + ★ **2026-09-16~17 분대 항 넷(자리 주장·사선 비우기·표적 분담·엄호 이동, 등록부만) · 배운 죽음(시체를 보거나 무전으로 — 방송 아님) · 부정 증거 · 두 점 시야(가슴→머리)** → `ai/2026-09-16_squad_terms_and_learned_death.md`. **L0 명령(이동·경계·점령·방어) · L1 분대 객체는 여전히 없다**(→ [W55]) — 있는 것은 비용 항과 선호뿐.
★ **2026-09-14 교정 라운드**: 노출의 사다리(조리개·사격자세) · 반동 · 조준 선회 · `FightingCost` · **목표 액터의 루트 컴포넌트 버그** → `ai/2026-09-14_exposure_ladder_and_corrections.md`
★ **2026-09-14~15 거동 라운드**: `ai/2026-09-14_danger_map_and_position_commitment.md` → **`ai/2026-09-15_exposure_cycle_and_muzzle_learning.md`**(값이 바뀐 자리는 이쪽이 최신). ~~⚠ 수비수가 정착해 쓰지 못하는 문제는 여전히 미해결이다 → [C-95]~~ → **해결**(기준면 + 위 두 라운드). 사용자 평가 "지금까지는 가장 좋네"(2026-09-15).

---

> **이 문서의 역할**: `CURRENT_STATE.md`가 "지금 뭘 할 차례인가"라면, 이 문서는
> **"지금 무엇이 존재하는가"** 다. 새 세션이 코드/에셋을 건드리기 전에 읽는다.
>
> 신뢰도 표기는 `CLAUDE.md` 3.1절 규칙을 따른다 — **[A]확정 / [B]잠정 / [C]미측정**.
> 여기 적힌 [A] 항목은 2026-09-11에 **에디터 실물을 읽어서** 확인했다.

---

## 0. 한 장 요약

| 층 | 상태 | 실체 |
|---|---|---|
| **L4 모션** | ✅ **동작 확인** | GASP ABP 복제 + Lyra 라이플 세트 이식. 이동·조준·견착/총내림·사격·재장전·왼손 IK + **연속 자세 축 4종**(총구 정렬·린·블라인드 파이어·**stance**) |
| **무기 · 투사체** | ✅ **동작 확인** (2026-09-12) | `ASoldierProjectile` 이식(포물선 탄도·도탄·재질별 명중·총알 휘파람) + `BP_AR4Rifle` + 캐릭터 배선. 3.1절 |
| **AI 층 (인지~교전)** | ✅ **동작 확인** (2026-09-13) | C++ **9쌍** `Source/SoldierLab/AI/`. 진영·기록/감쇠/융합·시야·무전·제압·교전·엄폐·**목표** + 디버그 규약. 5절 ★ **09-14 교정 라운드**: 조리개/사격자세 · 반동 · 조준 선회 · `FightingCost`. `ai/2026-09-14_exposure_ladder_and_corrections.md` |
| L3 실행 | ⚠ **형태가 바뀜** | StateTree가 아니라 **컴포넌트 + Tick 접합**이 한다. `AIC_Soldier`의 `StartLogic`은 **삭제**됐다 → [C-43] |
| L2 판단 | ⚠ **얇은 판이 돈다** | 유틸리티 스코어러는 없다. 교전/엄폐가 각자 자기 질문을 푼다. `ai/drafts/`는 **여전히 미반영** |
| L1 분대 | ✅ **빌드 · 시험 레벨 PIE · ★ 09-21 New_kadex_0811 본 레벨 첫 완주** [A 코드 / C 2차 PIE] | `Squad/SoldierSquadSubsystem` — 분대 객체 없이 등록부를 (Faction, `SquadId`)로 묶음. 명령→멤버별 `FSoldierAssignment`(제약만), 지연 출발, Approach→Hold(**도착선 = 존의 `ArrivalFraction`**, 09-18), achieved, **정원제 `ReinforceSquads`**(09-18, 본 레벨 로그 `quota=10 living=7 needed=3 moved=3`). `squad/2026-09-17_command_layer_design.md` → `squad/2026-09-18_squad_layer_fixes_quota_engage_range.md` → **`squad/2026-09-21_break_contact_and_targeting_exclusion.md`** |
| L0 명령 | ✅ **빌드 · 본 레벨 적용** (09-18 → ★ 09-21) | `Squad/SoldierOrderTypes.h`(`FSoldierSquadOrder`·ROE·Speed·**`EngageRangeCm`**·★ 09-21 **동사 `BreakContact`**(존 유지, 가는 길의 엄폐·경로·위험·제압 항 0 = 도주) · `bTargetableByOwnSideWeapons` = 시나리오 지시, 소비자 둘(브리지 → RCWS · `IsContactExcluded`)) · `Squad/SoldierZone` 저작 액터(**에디터에서 보임**, 09-18) · 시나리오 `IssueSquadOrder` 이펙트(+`Quota`, ★ 09-21 `SetTargetable(false)` 가 UGV RCWS `bRespectEnemyTargetingExclusion` 도 켬) + 트리거 `EnemyFireStarted`/`EnemyNearFriendlySoldiers` + 콘솔 `titan.SquadOrder`. **적·아군 전원 SoldierLab로 교체 — ★ 09-21 New_kadex_0811 이관 완료**(구 병사 40 삭제, `ASoldierZone` 8개, DT `DT_ScenarioSteps_ThreeStage_SoldierLab` 26행, 첫 PIE 전 체인 완주 "아주 잘됨"): `../level_new_kadex_0811/2026-09-21_soldierlab_migration_new_kadex_0811.md`. 시험 레벨 `L_SoldierScenario` + DT 13행: `../level_new_kadex_0811/2026-09-18_soldierlab_three_stage_test_level.md`. 옛 행 ↓ |
| ~~L0 명령 (옛 행)~~ | ⬜ 초안만 | `squad/drafts/` 의 명령 스키마. **목표·임무 개념은 [D10]으로 생겼다** — 다만 `ASoldierObjective` 는 **레벨 마커 한 개**일 뿐 명령도 국면 전환도 아니다 → [W25] ★ **09-14에 목표가 실제로 돌기 시작했다** — 다만 여전히 **마커 하나 · 소유권 변경 없음** |
| 엄폐 | ✅ **동작 확인** (2026-09-13) | **시야 판정을 거꾸로 돌린 것.** 볼륨·마커·베이크 없음. `cover/drafts/`의 EQS/SmartObject 초안은 **미채택** |
| **목표(objective)** | ✅ **이제야 실제로 돌기 시작했다** (2026-09-14) | `ASoldierObjective` + 세 비용 스코어러. **레벨에 0개 배치**라 셋째 항이 항상 0 → **[W25]** · `ai/2026-09-13_objective_and_position_cost.md` ★ **09-14**: 레벨에 `Objective_AllyBase` **1개 배치**. 그런데 **놓기만 했으면 여전히 안 돌았다** — 루트 컴포넌트가 없어 `GetActorLocation` 이 **영원히 월드 원점**을 답하고 있었다(**P90**). 반경·밴드·환율도 전부 바뀜다 → **[C-94]** · `ai/2026-09-14_exposure_ladder_and_corrections.md` 6절 |
| **관전 · 1인칭 · 머리 추종** | ✅ (2026-09-14 저녁 → 09-18 추기) | `Observer/SoldierObserverPawn` · `Camera/SoldierFirstPersonComponent` · `Pose/SoldierHeadAimComponent` — 5.1절 표. 머리 추종은 **기본 OFF · H 수동 토글뿐, AI 자동 활성화 없음**(P171). ~~마지막 2건(둘러보기 2단 · 선 기반 눈 목표+학습 방향 오프셋)은 빌드·확인 대기~~ → 09-14 21:30 확인. ~~[C-95] 정착 여부 미확인~~ → 09-15 해결. ★ **09-18**: 관전 휠 비행 속도 · slomo 무관 · 롤 잔류 수정 |
| **AI 포즈 층 (계약 소비)** | ✅ **동작 확인** (2026-09-17~18) | `Pose/SoldierScanTurnComponent`(총 내린 idle 의 캡슐 회전) · `Pose/SoldierGaitBridgeComponent`(Walk → GASP `WantsToWalk`) · `Pose/SoldierPoseSmootherComponent`(자세 축 4종 사다리꼴 × urgency) — AI 세션의 `GetAimPoint()/IsScanning()/GetDesiredGait()/GetPoseUrgency()` 를 읽는 쪽. 셋 다 **AI 전용**. 5.1절 표 · `animation/2026-09-18_ai_pose_layer_scanturn_gait_smoother.md` |

| **체력 · 피격 · 사망** | ✅ **동작 확인** (2026-09-15) | C++ `USoldierHealthComponent`(`AI/SoldierHealth`) 하나 — 표준 `OnTakePointDamage` 수신(투사체 무변경) · 부위 배율 · `bInvincible` · HitReact 애디티브 몽타주 13(`bStopAllMontages=false`, 재장전 안 끊김) · Death 몽타주 6 → 끝 0.1 s 전 래그돌(속도 관성 + 다음 틱 임펄스) · 등록부 Unregister + SoldierLab 컴포넌트/CMC/AIController/Tick 정지 · `Health/bDead/LastHit` 복제. **아군은 `BP_Soldier_Friendly` 의 `Invincible (무적)` 체크로 안 죽는다**(사용자 결정). 수치 전부 [C] → [C-110]~[C-118]. ★ **09-23**: `EndPlay`(`Destroyed` 일 때) 가 **자기가 어태치/소유한 액터(소총 등)를 같이 파괴** — `bDestroyCarriedActorsOnDestroy` 기본 켬, 풀링 투사체·클라 복제 액터 제외(P188, [W116]). `ai/2026-09-15_health_hit_death_implementation.md` |

**플레이어가 WASD로 직접 조작해 검증 가능하고(T 1인칭 · H 머리 추종), `GM_SoldierObserver`로 관전하면 AI끼리 싸운다(F 추적 · T 1/3인칭).**
~~**단 병사는 죽지 않는다** — 데미지·체력·사망이 없다 → [W18].~~ → **2026-09-15 해결** — 적군은 죽고 시체가 남는다. 아군은 무적(맞으면 움찔만). `SoldierLab.Debug.Health 1` 로 체력·마지막 피격이 보인다.

---

## 1. 대상 프로젝트 [A]

```
경로       C:\working\kadex\anim_test\SoldierLab
엔진       UE 5.8
베이스     GASP (Game Animation Sample) — 원본 캐릭터/ABP를 지우지 않고 병존 (P2)
클립 출처   Lyra Starter Game   C:\working\kadex\anim_test\LyraStarterGame
```

문서는 이 폴더(`titan/soldier_ai_lab/`)에, 코드·에셋은 언리얼 프로젝트에 있다.

---

## 2. 애니메이션 층 (L4)

### 2.1 클립 자산 [A]

> ★★ **2026-09-17 — 아군/적군 세트 분리.** 옛 `/Game/SoldierLab/Animations/` 와 `/Game/SoldierLab/PoseSearch/` 는 **소멸했다.**
> 전문: **`animation/prototypes/2026-09-17_ally_enemy_anim_set_split.md`**. 스켈레톤은 **1벌 그대로**(가르는 안은 기각 — ABP 1200노드·PSD 17·Chooser·AO·몽타주가 전부 두 벌이 되고 P76 의 이점이 사라진다). 갈린 것은 **클립·PSD·Chooser** 뿐이다.

```
/Game/SoldierLab/Animations_Enemy/     247개 · 전부 Enemy_ 접두사    ← 기준 세트
    Animations/Rifle/           130개
        Loops/       Walk · Jog · Crouch_Walk × Fwd/Bwd/Left/Right      (12)
        Starts/ Stops/ Pivots/                                          (36)
        TurnInPlace/ Turn L/R × 90/180 × Stand/Crouch                   (8)
        Idles/       Idle_ADS · Idle_Hipfire · IdleBreak ×2 · Crouch_Idle  (5)
        Poses/       조준 오프셋 포즈 15장
        AO_Rifle_Aim  AimOffset — Yaw −180..180 × Pitch −90..90, 샘플 15
        _Extra/       미사용 보관   (옛 _MF/ 39개는 사본에 안 따라왔다)
    Animations/Actions/          95개
        HitReact 13 (Front/Back/Left/Right × Lgt/Med/Hvy) · Death 6 (4방향)
        Dash · Equip/Unequip · Melee · GrenadeToss 등
    PoseSearch/                  2.2절

/Game/SoldierLab/Animations_Ally/      247개 · 전부 ALLY_ 접두사 · 같은 구조
/Game/SoldierLab/Animation/  (단수, 그대로)  SoldierCharacter_ABP · CHT_Soldier_CharacterAnimations
```

예: `Enemy_MM_Rifle_Idle_ADS` = `/Game/SoldierLab/Animations_Enemy/Animations/Rifle/Idles/Enemy_MM_Rifle_Idle_ADS`,
아군은 같은 자리에서 `Animations_Ally` + `ALLY_`. **아래 이 문서의 모든 클립 이름은 접두사를 뺀 공통 이름으로 적는다.**

**작업 기준은 적군 세트다** [A] — 적군 시나리오가 최우선이라 모든 애니메이션을 적군 기준으로 먼저 맞추고 아군은 나중에 재피팅한다(사용자 전략). 디자인팀 검수 대상도 적군 세트.

배선된 것은 사격/재장전 + **피격 13 · 사망 6**(2026-09-15)이고 나머지 Actions 는 반입만 돼 있다.
⚠ **사망 몽타주는 2026-09-17 부터 재생되지 않는다** — `bPlayDeathMontage = false`(순수 래그돌, 사용자 결정). 슬롯은 `FullBody` → **`DefaultSlot`** 으로 고쳐 둔 채다 → `ai/2026-09-17_hit_death_three_causes.md` 4·6절

**리타깃**: `RTG_Lyra_to_UEFN` (`IK_LyraManny` → GASP의 `IK_UEFN_Mannequin`).
루트 facing 손실 없음이 실측으로 확인됐다 → `animation/prototypes/2026-09-09_lyra_rifle_migration.md` 3절.

**커브**: 폴더별 모디파이어 스택 A~D로 일괄 생성. 어떤 클립에 어떤 커브를 거는지는
`animation/prototypes/2026-09-04_c34_clip_curve_mapping.md` 4절이 **유일한 기준**이다 (P8).

### 2.2 Pose Search [A]

```
/Game/SoldierLab/Animations_Enemy/PoseSearch/Rifle/     (접두사 Enemy_)
/Game/SoldierLab/Animations_Ally/PoseSearch/Rifle/      (접두사 ALLY_)   ★ 2026-09-17 두 벌
    PSD_Rifle_Stand_{Idles, Idles_LowReady, TurnInPlace}
    PSD_Rifle_Stand_Walk_{Loops, Starts, Stops, Pivots}
    PSD_Rifle_Stand_Jog_{Loops, Starts, Stops, Pivots}
    PSD_Rifle_Crouch_{Idles, TurnInPlace}
    PSD_Rifle_Crouch_Walk_{Loops, Starts, Stops, Pivots}      = 17개
    PSN_Rifle_All                                              정규화 세트 (멤버 17)
   (한 층 위) PSD_Soldier_Walk_Test                            P0-1 실험 잔재 (5.3절)
```

★ **PSD 의 클립 목록(`TArray<FInstancedStruct>`, MCP 로는 못 읽는다)이 Advanced Copy 에서 제대로 remap 됐다** [A] —
사본 PSD 가 사본 클립을 가리키고 원본 폴더 참조 0건(디스크 바이트 스캔). 정규화 세트도 사본끼리 묶였다.

- 스키마는 GASP 원본 `PSS_Default` / `PSS_Idle`을 그대로 쓴다
- **정규화 세트는 우리 것을 따로 만들었다** — Epic 것을 가리키면 GASP 25개 DB의
  척도로 우리 클립을 재게 된다 (P19)
- 검색 모드 `PCAKDTree`

### 2.3 Chooser [A]

```
/Game/SoldierLab/Animations_Enemy/PoseSearch/Enemy_CHT_Soldier_Databases   4열 × 17행
/Game/SoldierLab/Animations_Ally/PoseSearch/ALLY_CHT_Soldier_Databases     동   ★ 2026-09-17 두 벌
    열: Stance · MovementState · Gait · RotationMode    출력: PoseSearchDatabase
    (_BAK 은 이전 MMDatabaseLOD 구성 백업)

    행 1    Stand·Idle·Any·RotationMode  =    Strafe   →  Stand_Idles (Idle_ADS 1개)
    행 17   Stand·Idle·Any·RotationMode  Not  Strafe   →  Stand_Idles_LowReady
    행 2    Stand·Idle·Any·RotationMode  Any           →  Stand_TurnInPlace
    나머지                               Any
```

중첩 표를 쓰지 않는다. **통과하는 행이 전부 기여하므로**(`Chooser.cpp:712-713`)
DB 하나당 행 하나를 두고 행끼리 상호배타가 되게 짰다. Walk / ≠Walk 분기는
`MatchNotEqual`로 처리해 Run·Sprint를 함께 받는다. → 같은 문서 8절, `CLAUDE.md` 6.1d

★ **2026-09-17 — Chooser 가 두 벌이 된 것은 Advanced Copy 의 덤이다** [A]. `CHT_Soldier_Databases` 가 확인 대화상자에서
**체크된 채로** 복사돼 사본 PSD 를 가리키는 Chooser 가 자동으로 생겼다 — 덕분에 "열 하나 추가 + 17행 복제"(에디터 수작업)가
**불필요해졌다.** 진영 선택은 Chooser 안이 아니라 **ABP 의 Branch + 두 번째 EvaluateChooser** 가 한다 → 2.4절 끝

### 2.4 `SoldierCharacter_ABP` — 애님 그래프 [A]

`/Game/SoldierLab/Animation/SoldierCharacter_ABP` — GASP `SandboxCharacter_CMC_ABP` 복제본.

```
MotionMatching_0 / TwoWayBlend
 → BlendListByInt_0 → ApplyMeshSpaceAdditive_2   (Lean 뱅킹 ← BlendSpacePlayer_0
                                                   [BS1D_Additive_Lean_Run])
   |
   |  IdentityPose_2 → Slot 'FullBodyAdditivePreAim' → ApplyMeshSpaceAdditive_1   (사격 반동)
   |
   |  BlendListByBool_0 → DeadBlending_0 → ApplyMeshSpaceAdditive_0 . Additive   (조준)
   |      pose0 (true)  : BlendSpacePlayer_1 = AO_Rifle_ADS   견착  (Enable_AO)
   |      pose1 (false) : LayeredBoneBlend_0                  총내림(로우레디)
   v
[ BF_L ] → [ BF_R ] → [ BF_U ]          ★ 2026-09-12 추가 — 블라인드 파이어 3레이어 (2.5e)
   v
SaveCachedPose 'AimedPose'
   |-- UseCachedPose_0 → LayeredBoneBlend_1 . BasePose
   +-- UseCachedPose_1 → Slot 'UpperBody' → LayeredBoneBlend_1 . BlendPoses_0
                           blendMode = BranchFilter · branchFilters = [spine_01, depth 1]
                           curveBlendOption = UseMaxValue
LayeredBoneBlend_1 → ApplyAdditive_1 ← IdentityPose_3 → Slot 'UpperBodyAdditive'
   |                     ⚠ Alpha 핀 0.0 · node.alpha 0 — 재장전 애디티브는 사실상 OFF [B] → [R8]
   v
ApplyAdditive_0 ← AdditiveIdentityPose_7 → Slot 'AdditiveHitReact' (AnimGraphNode_Slot_4)
   |                                    ★ 2026-09-15 추가 — 피격 반응 (Alpha 핀 1.0 고정,
   |                                      HitReact 13 이 AAT_LocalSpaceBase 라 ApplyAdditive)
   v
Slot 'DefaultSlot' → OffsetRootBone_0 → RemapCurves_0 → LocalToComponentSpace_0
   ↑ 사망 몽타주(Death 6, 전신)가 여기서 재생된다 — ABP 변경 없이 기존 슬롯 사용 (2026-09-15)
   → ModifyBone spine_01..05        ← **린(lean) 축** ⚠ 이 문서에 아직 절이 없다 → [W7]
   → FootPlacement_0
   → ModifyBone pelvis (Z += PelvisDrop, BMM_Additive, BCS_ComponentSpace)
                                    ← ★ 2026-09-12 추가 — **stance 축** (2.5f)
   → LegIK_1 → TwoBoneIK_0 (hand_l)
   → ModifyBone_6 (neck_01) → ModifyBone_9 (neck_02) → ModifyBone_7 (head)
                                    ← ★ 2026-09-14 저녁 — **머리 조준 추종**(다른 세션, 5.2절 · `animation/2026-09-14_sight_alignment_plan.md`)
                                       회전 Additive · WorldSpace · Alpha ← HeadAimAlpha
   → ModifyBone_8 (head)  scale **Replace (1,1,1) · BCS_ComponentSpace** · 회전/이동 Ignore · alpha 1
                                    ← ★ 2026-09-15 — **BF 포즈에 구워진 마네킹 PP head ×1.15 상쇄** (2.5e-7)
   → ComponentToLocalSpace_2
   → PoseSearchHistoryCollector_0 → Root
```

> ⚠ **2026-09-15 정정**: 끝 체인 `TwoBoneIK_0 → ComponentToLocalSpace_2` 사이에 `ModifyBone` 4개가 들어왔다.
> `_6/_9/_7` 은 **머리 추종 세션의 것**이고 `_8` 은 이 세션의 head 스케일 상쇄다. `_8` 은 [W54] 가 "잔여물 · 삭제 권고"로
> 적어 둔 노드였는데 **이제 쓰인다 — 지우지 말 것**([W54] 철회). 같은 ABP 를 두 세션이 동시에 편집할 때는
> 남의 노드에 얹지 말고 자기 노드를 만든다(P125).

> ★ **`FootPlacement_0`과 `LegIK_1` *사이*라는 위치가 stance 축의 전부다** [A].
> `FootPlacement_0`은 `pelvisBone = pelvis`로 **골반을 스프링으로 소유**하므로 그 **앞**에
> 골반 오프셋을 넣으면 되돌려진다. 반면 `LegIK_1`은 이미 심어진 `ik_foot_l/r`로 발을
> **되돌리므로**, 사이에서 골반만 내리면 **무릎이 굽는다** = 웅크림. → 2.5f-2

> ⚠ **정정 (2026-09-12 실측)**: 이 체인에 **`ModifyBone spine_01..05`(린 축)가 빠져 있었다.**
> 린 축은 구현돼 동작 중인데 이 문서에 절이 없다 → **[W7]**(소급 문서화).
>
> ⚠ **`BlendSpacePlayer_1`(AO 블렌드스페이스)은 독립된 전체 포즈가 아니라
> `ApplyMeshSpaceAdditive_0`의 *애디티브 입력*이다** — `BlendListByBool_0` → `DeadBlending_0`을
> 거쳐 들어간다 [A]. (이 문서와 `animation/2026-09-02_gasp_abp_analysis.md` 76행은 원래
> 맞게 적혀 있었다. 2026-09-12 세션 중간에 이를 전체 포즈로 읽은 판단이 있었고 그것이
> 틀린 것이다 → `animation/prototypes/2026-09-12_blind_fire_axis.md` 9.6절)
>
> **`TwoBoneIK_0`(hand_l)이 그래프의 마지막 스켈레탈 컨트롤 노드**라는 점이
> 블라인드 파이어가 왼손 IK를 다시 짤 필요가 없었던 이유다 (2.5e).

**분기 지점에 Cache Pose를 반드시 둔다.** 상태를 가진 상류 체인(MM·BlendStack)을
두 갈래로 그냥 뽑으면 **매 프레임 두 번 평가돼 이동이 2배속이 된다.** 2026-09-10에 실제로 겪었다.

`MotionMatching_0` 내부 블렌드스택 서브그래프에는 GASP 원본 그대로
`OrientationWarping` · `StrideWarping` · `Steering` ×2 · `ResetRoot`가 들어 있다.
**전 프로퍼티를 GASP과 diff해 동일함을 확인했다**(2026-09-11) — 이 노드들은 건드리지 않았다.

**GASP에서 바꾼 애님 노드는 ~~두 개~~ → 한 개뿐이다** [A] (2026-09-11 MCP 실측 · 2026-09-15 정정):

| 노드 | 프로퍼티 | GASP | 우리 | 왜 |
|---|---|---|---|---|
| ~~`OffsetRootBone_0`~~ | ~~**`maxRotationError`**~~ | ~~**−1**(상한 없음)~~ | ~~**90**~~ → **−1 (GASP 원본으로 복귀, 2026-09-15)** | ~~상한이 없으면 급선회 시 메시가 캡슐에서 무제한으로 벌어지고 **그 벌어짐이 그대로 AimOffset 입력**이라 포즈가 뒤집힌다. `maxTranslationError`에는 30이 있는데 회전만 무제한이었다~~ → **정정**: 90 은 **비조준 급선회(A↔D 반전)에서 메시–캡슐 각도차가 90 을 넘는 순간 클램프가 메시를 끌어당겨 스냅**을 냈다. 90 을 넣은 이유(조준 중 뒤집힘)는 2026-09-12 의 유한 몸통 각속도(2.5d)가 이미 막고 있어 값이 불필요해져 있었다. −1 로 되돌리자 스냅 사라짐 + 뒤집힘 재발 없음(사용자 PIE) → [C-80] 확정 · `animation/prototypes/2026-09-15_sharp_turn_pop_bf_head_scale_and_weapon_socket.md` 1절 · P124 |
| `BlendListByBool_0` | `BlendTime_0` / `BlendTime_1` **(핀)** | 0.1 / 0.1 | ~~0.75 / 1.50~~ → **0.375 / 0.25** | AO 문턱 근처의 채터링을 블렌드로 누른다 |

> ⚠ **정정 (2026-09-12 실측)**: `BlendTime`은 ~~0.75 / 1.50~~ 이 아니라 **0.375 / 0.25** 다.
> 그 사이에 값이 바뀐 것을 문서가 따라가지 못했다. 이 **0.375초**(AO를 켜는 쪽)는
> 조준 보정 게인의 회복 시간(0.5초)을 정하는 기준값이기도 하다 → 2.5d절 ·
> `animation/prototypes/2026-09-12_body_yaw_rate_and_aim_antiwindup.md` 11.2절.

> ⚠⚠ **`BlendListByBool`은 불리언 방향이 뒤집혀 있다 — `true`가 0번 입력이다**
> (`AnimNode_BlendListByBool.cpp`: *"Intentionally flipped boolean sense"*).
> 따라서 **AO를 켤 때 0.375초 · 끌 때 0.25초**다.
> 그리고 이 값은 **노드 프로퍼티가 아니라 핀에 있다** — `node.blendTime`은 `[0.1, 0.1]`로
> GASP 원본 그대로다. 핀이 이긴다 (**P25**).
>
> ⚠ **`maxRotationError`와 `Enable_AO` 문턱은 묶여 있다** → 2.5b절 끝.
>
> ⚠ `BlendListByBool_0`의 나머지 설정은 GASP 원본 그대로다 —
> `transitionType = Inertialization` · `blendType = Custom` ·
> `customBlendCurve = /Game/Characters/UEFN_Mannequin/Animations/AimOffset/AO_Blend_Curve` ·
> `blendProfile = SK_UEFN_Mannequin:FastHead_Weight` [A].
> **`AO_Blend_Curve`의 키 값은 아직 아무도 안 봤다** — MCP로는 읽을 수 없다.
> 최댓값이 1.0을 넘으면 **독립적인 오버슈트 원인**이다 → **[R6]**.

> 상세 → `animation/prototypes/2026-09-11_sharp_turn_while_aiming.md`

> ⚠ MCP로 애님 노드를 고친 뒤에는 **반드시 `compile_blueprint`** 를 부른다. 저장은 컴파일이 아니다 (P23).
> ⚠ 핀으로 노출된 프로퍼티는 **핀 리터럴이 `node.<prop>`를 이긴다** (P25).

#### 2.4b ★ 진영 분기 — `UseAllyAnimSet` 하나로 5축 [A] (2026-09-17)

애니메이션 세트가 두 벌이 됐으나 **ABP 는 한 벌**이다(2.1절). 구동 변수는 하나:

```
ABP 변수      UseAllyAnimSet (bool, 기본 false = 적군)
캐릭터 변수    UseAllyAnimSet (bool, Instance Editable)
              → BeginPlay 의 Cast(SoldierCharacter_ABP) 뒤에 Set 노드 삽입
BP_Soldier_Friendly = true  /  BP_SoldierCharacter · BP_Soldier_Hostile = false
```

| 축 | 방법 | 노드 |
|---|---|---|
| 로코모션 61클립 | `Update_MotionMatching` 에 Branch + **두 번째 EvaluateChooser** | `K2Node_IfThenElse_0` · `K2Node_EvaluateChooser2_0`(ALLY_CHT) · `K2Node_VariableSet_1`(SetValidDatabases) · `K2Node_VariableGet_1`(GetValidDatabases → `SetDatabasesToSearch.Databases`) |
| 조준 AO 2 | Select 3단 (스탠스 Select ×2 → 진영 Select) → `BlendSpacePlayer_1.BlendSpace` | `K2Node_Select_1`(ally) · `K2Node_Select_2`(faction) · `K2Node_VariableGet_15` |
| 총내림 델타 1 | 노드 복제 + `BlendPosesByBool`(blend time 0) | `AnimGraphNode_SequencePlayer_1` · `AnimGraphNode_BlendListByBool_1` |
| 블라인드파이어 3 | 동 | `SequenceEvaluator_3/4/5` · `BlendListByBool_2/3/4` |
| 사격/재장전 몽타주 2 | 캐릭터 변수 `FireMontage` · `ReloadMontage`(Instance Editable) → `PlayAnimMontage` 핀 | `K2Node_VariableGet_35/36` |

전부 컴파일·저장 완료. 피격·사망 몽타주 19개는 ABP 가 아니라 **체력 컴포넌트 템플릿**이 진영별로 들고 있다(5절 · `ai/2026-09-17_hit_death_three_causes.md` 2절).

⚠ 함정 — `EvaluateChooser` 는 **Chooser 에셋이 노드 타입에 구워져 있다**(제네릭으로 만든 뒤 `chooser` set, 그리고 `mode`/`structOutputMode` 를 원본과 맞출 것) · 변수 setter 의 `type_id` 는 **카테고리 경로**를 탄다 · **BP 변수 추가 직후에는 CDO 에 프로퍼티가 없다**(컴파일 먼저) · `BlendListByBool` 은 **true 가 `BlendPose_0`** → `CLAUDE.md` 6.1 · P135·P138

### 2.5 왼손 IK [A]

> ★★ **2026-09-29 — 아래 구조는 대체됐다** → **`animation/2026-09-29_left_hand_grip_ik.md`**. 요약: IK **다시 켬**(`LeftHandIKEnabled` true) ·
> 목표 = 들고 있는 총 메시 `LeftHandGrip` 소켓의 **위치+회전**(`LegIK_1 → CopyBone_0(hand_r→ik_hand_gun) → ModifyBone_10(ik_hand_l ← LeftHandGripLocation/Rotation, 부모공간) → TwoBoneIK_0(effector 본 ik_hand_l, 회전 가져옴, 알파 보간)`) ·
> 알파 = `1 − Max(DisableLHandIK, LeftHandIKBlock)` · `LeftHandIKBlock` 은 C++ `USoldierAIBridgeComponent::UpdateLeftHandIK` 가 씀(사망 · 총 없음 · cvar `SoldierLab.Pose.LeftHandIK 0`, 기본값 1 = C++ 없으면 꺼짐) ·
> `LeftHandGripOffset` 은 더 이상 안 읽힌다. 아래는 09-13~15 기록으로 남긴다.
> ★ **2026-09-30 추기** — 소켓 회전 캡처 완료: `SK_AR4_X` `LeftHandGrip` 회전 P35.7/Y−161.7/R−153.3 · `SK_KA74U_X` P41.8/Y17.5/R27.0. PIE 확인 [C-175] 남음(그 문서 9절, 원칙 P194).

```
TwoBoneIK_0   iKBone         = hand_l
              effectorTarget = ~~본 weapon_r~~ → **소켓 `weapon_r`** (bUseSocket = true, BCS_BoneSpace)   ← 2026-09-13
              jointTarget    = lowerarm_l      (BCS_BoneSpace)
              EffectorLocation 핀     ← 변수 LeftHandGripOffset    ~~현재 (−30, 8, 4)~~ → **BeginPlay 에서 캐릭터가 산출해 써 넣는다** (2026-09-15, 아래)
              JointTargetLocation 핀  = **(−1, 1, 0)**   ← 2026-09-13. 0 이면 "FK 팔꿈치 자리를 폴로" — 아군에서 팔꿈치가 흉부를 뚫었다
              Alpha 핀         ← ~~Get Curve Value("DisableLHandIK")~~
                                 → **SelectFloat( A = GetCurveValue("DisableLHandIK"), B = 1.0, bPickA = LeftHandIKEnabled )**   ← 2026-09-14
              alphaInputType = Float · alphaScaleBiasClamp  scale −1 / bias +1  (반전)  ← 그대로. OFF 면 B=1.0 → 1−1 = 0
변수  LeftHandIKEnabled (Boolean)   기본 **false**    ← ★ 2026-09-14. 디자인팀이 왼손을 FK 로 맞추는 기간 동안 IK OFF
                                                     새 노드 K2Node_CallFunction_5(SelectFloat) · K2Node_VariableGet_7
                                                     캐릭터 BP 쪽 세터는 없다 — ABP 변수 기본값이 유일한 스위치
```

> ★ **2026-09-15 — `LeftHandGripOffset` 은 상수가 아니라 런타임 산출값이다** [A]. `BP_SoldierCharacter` BeginPlay 의 총 스폰
> 체인 끝(`SetActorEnableCollision(false)` 뒤 · `SetOwningCharacter` 앞)에서
> `ABP.LeftHandGripOffset = InverseTransformLocation( T = Mesh.GetSocketTransform("weapon_r", RTS_World),
> Location = WeaponMesh.GetSocketLocation("LeftHandGrip") )` — 즉 **총 메시의 `LeftHandGrip` 소켓을 `weapon_r` 소켓 기준
> 로컬로** 바꾼 값. 변수 기본값 `(−30, 8, 4)` 는 남아 있으나 시작 시 덮어써진다. 전제: `WeaponMesh` 의 메시 = 스폰되는 총 메시
> (3절) · 총 메시에 `LeftHandGrip` 소켓(`SK_AR4_X`: `Muzzle` · `LeftHandGrip` · `sight`). 노드 `K2Node_VariableGet_32/33` ·
> `K2Node_CallFunction_71~74` · `K2Node_DynamicCast_3` · `K2Node_VariableSet_16`. 사용자 PIE "잘됨".
> `animation/prototypes/2026-09-15_sharp_turn_pop_bf_head_scale_and_weapon_socket.md` 4절

> ★ **2026-09-13 — effector 가 본에서 소켓으로 바뀌었다** [A]. 아군 메시 `soldier_T` 에 `weapon_r` 본이 없어서다.
> 소켓 이름은 본과 같은 `weapon_r` 이고 **양쪽 메시에 각각 있다** — 마네킹 `SKM_UEFN_Mannequin` 은 `weapon_r` **본에**
> 항등으로(본과 결과 동일, 애니메이션 따라감), `soldier_T` 는 `hand_r` 에 마네킹 본의 레스트 오프셋
> `(0.19749, 3.411153, −0.381067)` 로. 소켓 모드는 소켓이 없는 메시에서 `LogAnimation: socket doesn't exist` 와 함께
> **조용히 꺼지므로**, 새 메시를 붙일 때마다 소켓 추가가 필수다 → 3.2절.

- 부착 대상은 **`weapon_r`** 다. 가상본 `hand_r_prop_01_Socket`은 안 된다 —
  `props_root` 밑이라 애니메이션이 안 들어가 원점에 붙어 있다
- 끄고 켜는 판정은 코드가 아니라 **클립의 `DisableLHandIK` 커브**가 한다 (P27)
- `AlphaInputType = Curve`는 이 프로젝트에서 **동작하지 않는다.**
  `Get Curve Value` 노드를 Alpha 핀에 직결해야 한다 (P28)

### 2.5b 조준 자세 — 견착/총내림을 **의도로** 가른다 [A]

AimOffset은 조준 *방향* 델타만 더하는 회전 애디티브다. **고개를 숙인 자세는 베이스 클립에 있다.**
그래서 베이스를 의도로 골라야 한다 — 안 그러면 MM이 포즈 유사도로 골라 절반은 틀린다.

```
AO_Rifle_ADS      15장 ← MM_Rifle_Idle_ADS_AO_*          베이스 MM_Rifle_Idle_ADS
AO_Rifle_Crouch   15장 ← MM_Rifle_Crouch_Idle_AO_*        베이스 MM_Rifle_Crouch_Idle   ⬜ 미배선
AO_Rifle_Aim      (구) Hipfire 포즈로 만들어져 있던 것. 보존만
```

- 조준 판정은 **`RotationMode == Strafe`** 다. `Enable_AO`가 테스트하는 값이고,
  `E_RotationMode` 바이트 순서는 `0=OrientToMovement · 1=Strafe · 2=Aim`
- 애디티브 갈래(`BlendListByBool_0`)는 **`Enable_AO`** 가 가른다 — ~~의도 **AND 각도 한계**(Idle 180 / 이동 115)~~ → **정정: 아래 2.5b-1절**
- 베이스 클립 갈래(Chooser)는 **의도만** 본다. `Enable_AO`를 쓰면 몸을 크게 틀 때
  베이스가 Hipfire로 튀어 고개가 벌떡 든다

> ⚠ **`Get_MMInterruptMode`에 `RotationMode != 직전` 항을 추가했다**(2026-09-11).
> 없으면 조준을 풀어도 MM이 재검색하지 않아 **ADS 클립이 몇 초씩 남는다.**
> `OR_1`의 네 번째 입력(A=MovementState, B=Gait·Moving, C=Stance, D=RotationMode).

#### 2.5b-1 `Enable_AO`의 실제 구조와 문턱 [A] (2026-09-11 정정)

`read_graph_dsl`이 **항을 하나 빠뜨렸다**(6.1f와 같은 종류). 배선을 직접 추적한 실제 구조는
**3항 AND**다:

```
Enable_AO =      | Get_AOValue.X |  ≤  문턱(MovementState)      ← X(yaw)만 본다. Y는 미연결
            AND  RotationMode == NewEnumerator1                (= Strafe / 견착 의도)
            AND  GetSlotLocalWeight("DefaultSlot") < 0.5       ← ★ DSL이 빠뜨린 항 (몽타주 중 AO 차단)
```

**문턱** — `MovementState`에 대한 `Select`:

| `MovementState` | 뜻 | GASP 원본 | **우리 값** |
|---|---|---|---|
| `NewEnumerator4` | **Idle** | 115 | **70** |
| `NewEnumerator0` | **Moving** | 180 | **70** |

> ⚠ **정정**: 위 2.5b절 본문에 **"Idle 180 / 이동 115"** 로 적혀 있었으나 **거꾸로다.**
> 열거자 대응은 `Update_States`가 정한다 — `IsMoving` **true → `NewEnumerator0`**(Moving),
> **false → `NewEnumerator4`**(Idle) [A]. GASP 원본은 **Idle 115 / Moving 180**이다.

**70으로 낮춘 이유**: 급선회로 조준각이 70°를 넘으면 AO를 꺼 **총을 내리고**, 몸이 따라잡아
각이 줄면 다시 올린다. 정지/이동을 가를 이유가 없어 같은 값으로 뒀다. PIE 확인 완료 [A].

#### 2.5b-2 ⚠⚠ `maxRotationError` ↔ `Enable_AO` 문턱은 **결합돼 있다** [A]

```
maxRotationError = 90     ← 천장.  AimOffset 입력이 여기까지만 커진다 (2.4절)
Enable_AO 문턱   = 70     ← 그 안에서 "총을 내릴 결정선"
```

- **문턱이 천장 이상이면 영원히 발동하지 않는다.** `maxRotationError = 90`이 `(Aim − Root)`를
  90°로 막고, 여기에 총구 보정(±25°)만 더해지므로 `|Get_AOValue.X|`의 실질 상한은 **약 115°** [B]
- 즉 **GASP 원본의 115 / 180은 `maxRotationError = 90` 아래에서는 사실상 "AO를 끄지 않는다"** [B]
- **한쪽만 바꾸면 다른 쪽의 의미가 조용히 달라진다. 항상 쌍으로 적고 쌍으로 바꾼다**
- 두 값 모두 **눈으로 정한 값이지 계측한 값이 아니다** → **[C-74]**

> ⚠ **정정 (2026-09-15)**: `maxRotationError` 는 **−1(상한 없음)로 되돌렸다**(2.4절 표 · [C-80]). 위 절의 "천장"은
> 이제 **없다** — `Enable_AO 70` 은 천장 안의 결정선이 아니라 그냥 결정선이고, `|Get_AOValue.X|` 의 상한은
> 클램프가 아니라 캡슐 각속도(2.5d)와 메시 추적 지연이 정한다. **[C-74] 결합 쌍에서 90 항이 빠진다** →
> 남는 쌍은 `Enable_AO 70` / `WeaponLowerAngleFull 65`. 이 절의 본문은 90 시절의 판단 이력으로 남긴다.

### 2.5c 총구 정렬 되먹임 보정 [A] (2026-09-11)

AimOffset은 **근사**다. `AO_Rifle_ADS`는 5(Yaw) × 3(Pitch) = 15장이고 **pitch 표본이 3장뿐**이라
총구가 조준 방향에서 벗어나며, 벗어나는 양이 **조준 각도에 따라 변한다**(정면 정지 실측
pitch −11.4° · yaw +10.1°). 그래서 상수 오프셋이 아니라 **닫힌 되먹임 루프**로 잡는다.

```
BP_SoldierCharacter (매 Tick, AI 병사 포함)
    ERR   = Delta(Rotator)( GetControlRotation , MakeRotFromX(총구 +X 전방벡터) )
    GATE  = max( |Delta(GetControlRotation, PrevAimRot).Pitch| ,
                 |Delta(GetControlRotation, PrevAimRot).Yaw|   )  ≤  2.0    ← 카메라 각속도
    GAIN  = SelectFloat( 0.0 , Lerp(0.05, 0.0, WeaponLowered) , bPickA = NOT AOActive )
                                                               ↑ ★ 2026-09-12 추가: 안티 와인드업
    누적  = SelectRotator( A = CombineRotators(이전 누적, Lerp(Rotator)((0,0,0), ERR, GAIN)) ,
                           B = 이전 누적 ,                    ← 게이트 거짓이면 **그대로 유지**
                           bPickA = GATE )
           → NormalizeAxis → Clamp(±25°) → MakeRotator(Roll = 0)
           → AimCorrection (캐릭터)  →  Cast to SoldierCharacter_ABP  →  AimCorrection (ABP)
    PrevAimRot ← GetControlRotation                              (틱 끝)

SoldierCharacter_ABP · Get_AOValue
    Delta( AimingRotation , Delta( RootRotation , AimCorrection ) )  ==  (Aim − Root) + Corr
```

> ⚠⚠ **정정 (2026-09-12) — 위의 `GetControlRotation`은 이제 `GetController()`를 거치지 않는다** [A].
> 원래 배선은 **`GetControlRotation( GetController() )`** 였고, `GetController()`가 null이면
> 로그에 `Accessed None trying to read CallFunc_GetController_ReturnValue`가 찍히며
> **조준 방향이 `(0,0,0)`으로 읽혔다.** **AI에게 치명적이다** — 총구 정렬(2.5c)과 무기 내리기(2.5d)는
> 애초에 **45명의 AI 병사를 위한 기능**인데 입력이 쓰레기값이면 `BodyErr`가 통째로 무의미해진다.
> **두 호출부(`UpdateBodyYawRate` · Tick의 조준 루프)를 GASP 자신의 패턴으로 교체했다**:
> ```
> SelectRotator( A = GetControlRotation[Pawn] , B = GetBaseAimRotation ,
>                bPickA = IsLocallyControlled )
> ```
> 죽은 노드 3개를 같이 제거했다.
> ⚠ **함정**: `GetControlRotation`은 **`APawn`과 `AController` 양쪽에 존재**한다. DSL 라이터가
> Controller 쪽에 붙여 *"This blueprint (self) is not a Controller…"* 로 블루프린트를 깨뜨렸다 —
> **`create_node` + `declaring_class = /Script/Engine.Pawn`** 으로 만들어야 한다
> (`CLAUDE.md` 6.1f-3). 상세 → `animation/prototypes/2026-09-12_continuous_stance_axis.md` 10절.
> ⚠ 이것은 **[C-75](AI의 `PrevAimRot` 동결)와 별개 문제**다. [C-75]는 그대로 열려 있다.

| 값 | 왜 |
|---|---|
| **게이트 = 카메라 각속도 ≤ 2.0°/프레임** | **입력**으로 건다. 오차(=자기 출력)로 걸면 **자기 잠금**이 생긴다 (**P35**) |
| **게이트 거짓 → 유지**(감쇠 아님) | 0으로 감쇠시키면 카메라를 움직이는 동안 보정이 사라져 총구가 다시 벌어진다 |
| **게인 0.05** (이력 0.5 → 0.25 → 0.05) | **게인이 곧 학습 시간상수.** 0.05 ≈ 20프레임 ≈ 0.33초로, `OffsetRootBone`이 정착하는 시간에 맞췄다. 0.25는 몸의 추적 지연까지 학습해 **오버슛**했다 (**P35**) |
| **게인 게이트 = `AOActive`** (2026-09-12) | **AO가 꺼져 있으면 보정은 포즈에 닿지 않는데 적분기는 계속 돈다** = 열린 루프. ERR 20°·게인 0.05면 **0.42초에 ±25 클램프에 도달**한다. 액추에이터(`Enable_AO`)가 결합돼 있을 때만 적분한다 (**P37**) |
| **게인 회복 0.5초 > 포즈 블렌드 0.375초** | 다시 들 때 **포즈가 먼저 서고 게인이 나중에 붙는다.** 반대면 블렌드 구간에서 다시 포화한다 (**P38**) |
| **Clamp ±25°** | 안티 와인드업. ⚠ 오차가 25°를 넘으면 **클램프에 두 번째 안정 평형점**이 생긴다 → **[C-76]** (**P40**) |

- ~~**게인 0.25** (0.5는 링잉)~~ → 위 표 참고 · **Clamp ±25°** 는 안티 와인드업
- `Delta(A, B)`는 `NormalizedDeltaRotator` = **성분별 뺄셈**이므로 위 중첩이 곧 성분별 덧셈이 된다.
  `CombineRotators`(합성)로 같은 것을 하려다 **월드 −X에서 부호가 뒤집혀 발산**했다 → **P32**
- `AimingRotation`은 ABP 전체(노드 1,200개)에서 **`Get_AOValue` 한 곳에서만 읽힌다**(2026-09-11 전수 검색).
  그래서 편향을 넣어도 몸통 facing·궤적이 흔들리지 않는다
- 결과: **월드 −X를 포함한 전 방향에서 오차 ≈ 0**

> 상세 · 실패한 접근 4종 + **실패한 게이트 설계 3종**은
> **`animation/prototypes/2026-09-11_muzzle_aim_alignment.md`**(9절 + **13절 정정**).
> 안티 와인드업 게이트와 그 **또 다른 실패 3종**은
> **`animation/prototypes/2026-09-12_body_yaw_rate_and_aim_antiwindup.md`** 10·11절.
> ⚠ 이 루프와 함께 들어간 **계측 장치가 아직 Tick에 물려 있다** → 6절.

> ⚠⚠ **`SetPrevAimRot`이 `IsLocallyControlled` Branch 안에 있다** [A]. `AimCorrection` 자체는
> 분기 **앞**에서 갱신되므로 AI도 보정을 받지만, **`PrevAimRot`은 AI에게 영원히 초기값**이라
> 속도 게이트가 계속 닫혀 **AI의 누적 보정이 얼어붙을 수 있다** [B] → **[C-75]**.

> ★★ **2026-09-30 — 위 루프는 재설계됐다** [A] (`Pose/SoldierAIBridgeComponent.cpp:408-460`, 09-21 부터 BP 대신 C++ `TickBridge` 가 돈다) → `ai/2026-09-30_ai_real_pitch_and_aim_smoothing.md` 6.2 · 7 · 10절, P196.
> ```
> 적분 조건  bWeaponOnAim = AOActive ∧ !IsAnyMontagePlaying ∧ !IsHitReacting ∧ WeaponLowered < 0.05
> 게인      1 − exp(−SoldierLab.Aim.CorrectionRate 10 · dt)       ← 틱당 0.05(≈ 3/s) 대체, 시간 기반
> 데드밴드   오차 중 SoldierLab.Aim.CorrectionDeadbandDeg 0.3° 안쪽은 건드리지 않음
> 게이트    조준 각속도 ≤ SoldierLab.Aim.CorrectionGateDegPerSec 120°/s 면 적분, 넘으면 유지   ← 프레임당 2° 대체
> 밖에서    bWeaponOnAim 거짓이면 SoldierLab.Aim.CorrectionLeakRate 6/s 로 0 을 향해 누설   ← "유지" 대체 (재장전 중 ±25° 포화 방지)
> 상한      변화량 축당 SoldierLab.Aim.CorrectionMaxDegPerSec 60°/s, 그 뒤 ±25° 클램프
> 조준      IsLocallyControlled ? GetControlRotation : GetBaseAimRotation        PrevAimRot 은 분기 없이 매 틱 기록(:479)
> ```
> AI 의 컨트롤 회전이 이제 **피치를 가진다**(`ASoldierAIController`, P198) — 전에는 AI 총의 위아래 전부를 이 보정기가 맞췄다. 위 표의 "게이트 거짓 → 유지" · "게인 0.05" 는 BP 옛 본문(`SoldierLab.AIBridge.Native 0`) 기준이다.

### 2.5d 유한 몸통 각속도 — 무기 자세가 회전 속도를 정한다 [A] (2026-09-12)

GASP 부모는 **접지 중 `RotationRate = (0, −1, 0)`** 을 매 프레임 쓴다. **음수는 "즉시"** 라
캡슐이 컨트롤러를 한 프레임에 따라잡았고, 그래서 **"총을 내리면 빨리 돈다"는 결정에
아무 물리적 근거가 없었다.** 그것을 유한 각속도로 바꾼 것이 이 절이다.

```
BP_SoldierCharacter . UpdateBodyYawRate()        ← 새 함수 그래프. Event Tick **끝**에서 호출

  BodyErr       = | Delta(Rotator)( GetControlRotation , GetActorRotation ).Yaw |
  WpnTgt        = SelectFloat( 1.0 ,
                               MapRangeClamped( BodyErr ,
                                                WeaponLowerAngle , WeaponLowerAngleFull , 0 , 1 ) ,
                               bPickA = NOT AOActive )
  WeaponLowered = Max( RampAxisTo(현재, WpnTgt, WeaponLowerRate, dt) ,    ← 내릴 때 0.125s
                       RampAxisTo(현재, WpnTgt, WeaponRaiseRate, dt) )    ← 올릴 때 0.5s
  ω             = SelectFloat( 200 , Lerp(YawRate_Up, YawRate_Down, WeaponLowered) ,
                               bPickA = CharacterMovement.IsFalling )
  CharacterMovement.SetRotationRate( MakeRotator( Roll=0 , Pitch=0 , Yaw=ω ) )
```

- ⚠ **`GetControlRotation`은 2026-09-12부터 `SelectRotator(GetControlRotation[Pawn],
  GetBaseAimRotation, IsLocallyControlled)`** 다 — `GetController()` 경유를 걷어냈다. 2.5c의 정정 참고
- `Max(빠른 램프, 느린 램프)`는 **비대칭 속도를 비교 노드 없이** 얻는 요령이다 —
  내릴 때는 빠른 쪽이 더 크고, 올릴 때는 빠른 쪽이 더 작다
- `RampAxisTo`는 **기존 C++** `USoldierAxisLibrary`(4절). 빌드 불필요

#### 2.5d-1 ⚠⚠ `BeginPlay`의 틱 선행 조건이 **이 기능의 전제**다 [A]

`AC_PreCMCTick` 컴포넌트가 부모의 `UpdateRotation_PreCMC`를 매 프레임 구동해 `−1`을 다시 쓴다.
**순서를 고정하지 않으면 우리 쓰기는 조용히 무시된다.** `Parent: BeginPlay` 직후에:

```
self.AddTickPrerequisiteComponent( GetComponentByClass(AC_PreCMCTick) )
CharacterMovement.AddTickPrerequisiteActor( self )
                      →  AC_PreCMCTick  →  우리 Tick  →  CMC
```

> ⚠⚠ **2026-09-18 정정 — 위 두 줄만으로는 순서가 서지 않았다.** 엔진이 CMC 에 기본으로 거는 반대 엣지
> (`UMovementComponent::bTickBeforeOwner = true` → 액터가 CMC **뒤**, `MovementComponent.cpp:186-188`)와 **순환**이
> 되어 `FTickTaskManager` 가 우리 엣지를 매 프레임 버리고 `LogTick: … CharMoveComp … would form a cycle` 을 찍고
> 있었다(시험 레벨 35명 = 초당 600줄). 수정: `BP_SoldierCharacter` `CharMoveComp` **`Tick Before Owner = false`**
> (+ 자식 BP·레벨 인스턴스). 경고는 사라졌고 **실제 순서는 아래 서명 시험으로 재확인 대기 [C-146]** —
> `animation/prototypes/2026-09-18_tick_cycle_warning_charmovecomp.md`. 같은 세션이 Event Tick 의 총구
> `DrawDebugCoordinateSystem`(모든 소총의 xyz 축) exec 을 끊었다(노드 잔존).

> **`BodyErr == 0.000`(정확히 0)은 `RotationRate.Yaw = −1`이 살아 있다는 서명이다.**
> 0이 아닌 값이 나와야 유한 각속도가 먹고 있는 것이다. **육안으로는 판별되지 않는다** —
> 애니메이션은 자기 속도로 계속 돌기 때문이다 (**P39**).

#### 2.5d-2 튜닝값 [A]

| 변수 | 값 | 왜 |
|---|---|---|
| `YawRate_Up` | **90** °/s | 총을 든 상태 — 90°에 1초. "들고는 빨리 못 돈다"가 체감된다 |
| `YawRate_Down` | **720** °/s | 총을 내린 상태 — 90°에 0.125초. 사실상 제약 없음 |
| `WeaponLowerAngle` | **30** ° | `BodyErr`가 넘으면 내리기 시작 |
| `WeaponLowerAngleFull` | **65** ° | 완전히 내려감. **`Enable_AO` 문턱 70보다 약간 아래** — 포즈가 바뀌기 직전에 각속도가 먼저 풀린다 |
| `WeaponLowerRate` | **8.0** (0.125 s) | 내리는 쪽은 빠르게 |
| `WeaponRaiseRate` | **2.0** (0.5 s) | 올리는 쪽은 느리게. **AO 블렌드 0.375초보다 느려야** 게인이 포즈보다 늦게 회복한다 (2.5c) |

> ⚠⚠ **`WeaponLowerAngleFull 65` 가 [C-74]의 결합 쌍에 세 번째로 합류했다** —
> `maxRotationError 90`(천장) / `Enable_AO 70`(결정선) / `WeaponLowerAngleFull 65`(각속도 해제선).
> **셋을 함께 본다.**
>
> ⚠ `WeaponLowered`(캡슐 기준 연속값)와 `Enable_AO`(메시 기준 불리언)는 **다른 신호다.**
> 조준 보정 게이트에 `WeaponLowered`를 썼다가 실패했다 →
> `animation/prototypes/2026-09-12_body_yaw_rate_and_aim_antiwindup.md` 10.2절.

> 상세 → `animation/prototypes/2026-09-12_body_yaw_rate_and_aim_antiwindup.md`

### 2.5e 블라인드 파이어 축 — 저작 포즈를 연속 애디티브로 얹는다 [A] (2026-09-12)

총구 조준 정렬(2.5c) · 린에 이은 **세 번째 연속 자세 축**이다. 사용자 확인 완료 [A].

```
BlindFireH   −1(좌) ~ +1(우)          BlindFireV   0 ~ +1(상)
키  1 = 좌   2 = 위   3 = 우   4 = 리셋
누르고 있으면 등속 램프 · **떼면 그 값에서 멈춘다**(린 축과 같은 규약)
```

#### 2.5e-1 ★ 왜 IK가 아니라 저작 포즈인가 [A]

처음 권고안은 **순수 IK**(무기 트랜스폼을 오프셋하고 팔 IK가 풀게 하는 것)였다. **기각됐다.**

| | |
|---|---|
| 사용자가 **특정 자세(타르코프류)** 를 이미 그리고 있었다 | "팔이 닿는다"가 아니라 **정해진 실루엣**이 필요했다 |
| 그 자세는 **몸통/척추가 돌아간다** | **팔 IK로는 만들 수 없다** |

**팔 IK 체인이 척추로 전파되지 않는 것은 결함이 아니라 설계다** [A] — 표준 바이페드 리그에서
예외 없이 `upperarm → lowerarm → hand` 세 마디다. → **저작 포즈 + 연속 애디티브**를 채택하고
**IK는 왼손을 그립에 붙잡아 두는 역할로 축소**했다.

#### 2.5e-2 에셋 [A]

세 장 전부 `SK_UEFN_Mannequin`. 베이크 결과는 **151키 / 150프레임 / 5.0초**이고
**쓰는 것은 프레임 0 하나뿐**이다.

```
<세트>/Animations/Rifle/Poses/<접두>MM_Rifle_BlindFire_L        ★ 2026-09-17 부터 진영별 2벌
<세트>/Animations/Rifle/Poses/<접두>MM_Rifle_BlindFire_R           <세트> = Animations_Enemy | Animations_Ally
<세트>/Animations/Rifle/Poses/<접두>MM_Rifle_BlindFire_U           <접두> = Enemy_ | ALLY_

애디티브 변환 (3장 동일)
    additiveAnimType = AAT_RotationOffsetMeshSpace
    refPoseType      = ABPT_AnimFrame
    refPoseSeq       = <세트>/Animations/Rifle/Idles/<접두>MM_Rifle_Idle_ADS   ← 사본끼리 remap 됨 [A]
    refFrameIndex    = 0
```

★ **그래프의 BF 3레이어도 진영 분기가 붙었다** (2026-09-17) — 레이어마다 `SequenceEvaluator` 를 복제하고
`BlendListByBool`(blend time 0)로 갈랐다: `SequenceEvaluator_3/4/5` · `BlendListByBool_2/3/4`,
조건은 `UseAllyAnimSet` → 2.4b절.

- **척추/몸통과 머리까지** 저작돼 있다(2.5e-1의 이유가 그대로 반영된 것)
- **왼손이 총에 붙어 있는 상태**로 저작됐다
- ⚠ `ABPT_AnimFrame`은 **다른 에셋을 참조하는** 기준 포즈다 — 리타깃하면 끊기고 **소스까지 바뀐다** (**P20**)

> **저작 시작 포즈는 정확성에 영향이 없다** [A]. 메시 스페이스 애디티브는 `(저작 − 기준)`이고
> 기준 위에 얹으면 저작 포즈가 정확히 복원된다. A-포즈에서 저작해도 결과는 같고,
> 조준 아이들에서 출발하는 이득은 **작업량뿐**이다. 이 확인이 필요했던 이유는
> 작동하는 저작 경로(`CR_Mannequin_Body`, 비레이어드)가 **`Bake To Control Rig` 없이는
> 조준 포즈를 물려받을 수 없기** 때문이다 → `CLAUDE.md` 6.3절.

#### 2.5e-3 애님 그래프 — 마스크 애디티브 3레이어 [A]

`ApplyMeshSpaceAdditive_0`(조준 적용)와 `SaveCachedPose_0 'AimedPose'` **사이**에 직렬로 끼운다.

```
ApplyMeshSpaceAdditive_0 → [BF_L] → [BF_R] → [BF_U] → SaveCachedPose 'AimedPose' → (하류 그대로)

레이어 한 벌 (3벌 동일)
    AdditiveIdentityPose ───────────────────────→ LayeredBoneBlend . BasePose
    SequenceEvaluator(MM_Rifle_BlindFire_X,
                      ExplicitTime = 0)  ───────→ LayeredBoneBlend . BlendPoses_0
                                                   BlendWeights_0 = 1.0
    LayeredBoneBlend ───────────────────────────→ ApplyMeshSpaceAdditive . Additive
    ApplyMeshSpaceAdditive . Alpha  ←  ABP 변수  BF_AlphaL / BF_AlphaR / BF_AlphaU

마스크
    blendMode  = BranchFilter
    layerSetup = [ { branchFilters: [ { boneName: "spine_01", blendDepth: 1 } ] } ]
```

- 베이스가 **애디티브 항등 포즈**이므로 마스크 밖의 본은 **델타 0** — `LayeredBoneBlend`는
  순전히 "애디티브를 상체로 마스킹"하는 용도다
- `spine_01` 브랜치는 **척추·쇄골·양팔·목·머리**를 덮고 **골반과 다리를 제외**한다 →
  **로코모션이 전혀 건드려지지 않는다**
- 정지 포즈이므로 **`SequenceEvaluator`(시간 지정) + `ExplicitTime = 0`** 을 쓴다. `Player`가 아니다

> ★ **왼손 IK 재작업이 필요 없었다** [A]. `TwoBoneIK_0`(`hand_l`)이 **그래프의 마지막
> 스켈레탈 컨트롤 노드**(`FootPlacement_0 → LegIK_1` 뒤)라, 새 레이어가 몸통을 아무리 틀어도
> **마지막에 왼손을 그립에 다시 앉힌다.**

> ⚠⚠ **마스크 규약이 이제 두 벌이다** [A]. 기존 `LayeredBoneBlend_0`(로우레디)은
> **`blendMode = BlendMask`** + 블렌드 마스크 에셋 **`SK_UEFN_Mannequin:BM_LowReady`** 를 쓰고,
> 새 세 개는 **`BranchFilter`** 를 쓴다. 둘 다 동작하지만 다음 사람이 혼동한다 → **[W8]**.

**새 노드 ID**: `AnimGraphNode_IdentityPose_4/5/6` · `AnimGraphNode_SequenceEvaluator_0/1/2` ·
`AnimGraphNode_LayeredBoneBlend_2/3/4` · `AnimGraphNode_ApplyMeshSpaceAdditive_3/4/5` ·
게터 `K2Node_VariableGet_3/4/5`.

#### 2.5e-4 캐릭터 — `UpdateBlindFire()` [A]

새 함수 그래프. **Event Tick 끝**에서 `UpdateBodyYawRate` **다음**에 부른다.

```
reset      = InRange( BlindFireInputReset , 0.5 , 2.0 )

BlindFireH = SelectFloat( RampAxisTo( BlindFireH , 0 , BlindFireResetRate , dt ) ,
                          StepAxis ( BlindFireH , BlindFireInputH , BlindFireRate , dt , −1 , 1 ) ,
                          reset )
BlindFireV = SelectFloat( RampAxisTo( BlindFireV , 0 , BlindFireResetRate , dt ) ,
                          StepAxis ( BlindFireV , BlindFireInputV , BlindFireRate , dt ,  0 , 1 ) ,
                          reset )

ABP.BF_AlphaL = MapRangeClamped( BlindFireH , 0 , −1 , 0 , 1 )
ABP.BF_AlphaR = MapRangeClamped( BlindFireH , 0 , +1 , 0 , 1 )
ABP.BF_AlphaU = MapRangeClamped( BlindFireV , 0 , +1 , 0 , 1 )

HUD 행  "BF_H"  "BF_V"
```

- ★ **`MapRangeClamped`를 반파 정류기로 쓴다** — 입력 범위의 **방향만 뒤집으면** 음의 반파가
  클램프에 잘린다. 부호 있는 축 하나를 좌/우 두 알파로 가르는 데 **비교·분기 노드가 필요 없다.**
  블루프린트 팔레트에 산술 노드가 없기 때문에 쓰는 요령이다 (**P33**, 다른 곳의 `Lerp` = 곱셈과 같은 계열)
- `StepAxis` / `RampAxisTo`는 **기존 C++** `USoldierAxisLibrary`(4절). **빌드 불필요.**
  등속 램프라 **입력이 멈춰도 값이 붕괴하지 않는다** — "떼면 유지" 규약이 여기서 나온다

#### 2.5e-5 변수와 튜닝값 [A]

| 변수 | 위치 | 값 / 비고 |
|---|---|---|
| `BlindFireH` · `BlindFireV` | 캐릭터 | 축 상태 |
| `BlindFireInputH` · `BlindFireInputV` · `BlindFireInputReset` | 캐릭터 | 입력 액션이 써 넣는 값 |
| **`BlindFireRate`** | 캐릭터 · **Instance Editable** | **1.0** — 0→1 에 1초. 자세를 잡는 것은 **의도적**이어야 한다 |
| **`BlindFireResetRate`** | 캐릭터 · **Instance Editable** | **3.0** — 0까지 약 **0.33초**. 푸는 것은 **즉각적**이어야 한다 |
| `BF_AlphaL` · `BF_AlphaR` · `BF_AlphaU` | **ABP** | 각 레이어의 `ApplyMeshSpaceAdditive.Alpha` |

> 잡기 1.0 / 풀기 3.0 의 **비대칭**은 2.5d의 `WeaponLowerRate 8.0` / `WeaponRaiseRate 2.0`과
> 같은 사고방식이다.

#### 2.5e-6 입력 [A]

```
IA_BlindFireH · IA_BlindFireV · IA_BlindFireReset      셋 다 Axis1D
```

**셋 다 `IA_Lean`을 복제해서 만들었다.** ① `AssetTools`에 **에셋 생성 함수가 없어** 복제가
유일한 수단이고, ② `IA_Lean`은 **트리거가 이미 비워져 있어** 앞서 기록한
**`InputTriggerPressed` 1프레임 버그**를 자동으로 피한다.

```
이벤트 배선 (IA_Lean 과 동일)
    Triggered  →  Set <해당 Input 변수> = ActionValue
    Completed  →  Set <해당 Input 변수> = 0

IMC_Sandbox 매핑
    키 1 → IA_BlindFireH  (**Negate**)      키 3 → IA_BlindFireH
    키 2 → IA_BlindFireV                    키 4 → IA_BlindFireReset
```

> ⚠ **키 매핑은 에디터 수작업이다.** `ObjectTools.get_properties`가 `IMC_Sandbox`의
> `mappings`를 **매핑이 있는데도 `[]`로 반환**한다 — 이 배열이 API로 직렬화되지 않는다.

> ⚠ **미측정 [B]**: BF 레이어는 조준 애디티브 **위**에 얹히므로 총구가 조준선에서 크게 벗어난다.
> `AimCorrection` 적분기는 **총구 전방벡터**로 오차를 재므로 **BF 자세를 "고쳐야 할 오차"로
> 학습**할 수 있다(→ **P40**의 두 번째 평형점 · [C-76]과 같은 기구) → **[C-78]**.

> 상세 · **포즈 저작의 막다른 길 4종** · **리그 IK/FK 모드 변수 발견** →
> `animation/prototypes/2026-09-12_blind_fire_axis.md` (저작 경로 자체는 `CLAUDE.md` 6.3절)

#### 2.5e-7 ⚠⚠ BF 포즈 3장에는 마네킹 PP ABP 의 스케일이 구워져 있다 — `ModifyBone_8` 로 상쇄 [A] (2026-09-15)

증상: BF 를 켜면 **`soldier_T` · `new_enemy_T` 의 머리가 커진다.** 마네킹에선 안 보인다.

원인 [A, 사용자 확인]: 2.5e-2 의 세 클립은 시퀀서에서 **마네킹 메시로 베이크**했는데, 그때 마네킹의 포스트프로세스 ABP
`ABP_UEFN_Mannequin_PostProcess`(head Replace **1.15** · thigh_l/r **(1, 1.12, 1.12)** · foot 1, 전부 `BCS_ComponentSpace`)
결과까지 클립에 들어갔다. 기준 `MM_Rifle_Idle_ADS` f0 의 head 스케일은 1.0 이라 **메시 공간 애디티브 델타에 head ×1.15 가 남는다.**
마네킹은 PP ABP 가 그래프 뒤에서 스케일을 **Replace** 하므로 델타가 가려졌고, PP ABP 가 없는 두 메시(3.2절)에서 드러났다.
`MM_Rifle_LowReady` 도 같은 방식 베이크라 같은 문제가 있을 것 [B].

조치: 재베이크는 **불가**(포즈 저작 시퀀서 `/Game/NewLevelSequence` 가 깨진 `CR_Mannequin_Body` 에 의존 → [Q48]). 대신 그래프 끝 체인:

```
ModifyBone_8 (head)   scaleMode BMM_Replace (1,1,1) · scaleSpace **BCS_ComponentSpace** · rotation/translation Ignore · alpha 1
                      위치: ModifyBone_7 뒤 · ComponentToLocalSpace_2 앞 (2.4절 체인)
```

마네킹은 PP 가 뒤에서 1.15 를 다시 걸므로 무변화. 사용자 PIE "잘됨". thigh 1.12 는 **상쇄하지 않았다** [C].

> ⚠ **함정 둘 (P121 · P122)**: ① 스케일 Replace 를 `BCS_BoneSpace` 로 걸면 **no-op** 다(본 자기 기준 = 항등). ComponentSpace 로.
> ② 시퀀서 Bake Animation Sequence 는 PP ABP 결과까지 굽는다 — 베이크 전 메시 컴포넌트 `Disable Post Process Blueprint` 를 켤 것.
> 재베이크는 **[W65]**. `animation/prototypes/2026-09-15_sharp_turn_pop_bf_head_scale_and_weapon_socket.md` 2절

### 2.5f 연속 stance 축 — 네 번째(마지막) 연속 축 [A] (2026-09-12)

총구 정렬(2.5c) · 린 · 블라인드 파이어(2.5e)에 이은 **네 번째 연속 자세 축**이다.
사용자가 **단계마다** 확인했다 [A].

```
StanceAxis  0(기립) ~ 1(웅크림)      V = 내리기 · B = 올리기(Negate)
누르고 있으면 등속 램프 · **떼면 그 값에서 멈춘다**(린·블라인드 파이어와 같은 규약)
```

> **키가 C가 아닌 이유**: `C`는 기존 앉기 토글(`IA_Crouch`)이 이미 쓰고 있었다. 사용자가 V/B를 골랐다.

#### 2.5f-1 ★ "stance"는 **네 하위 시스템의 묶음 이름**이다 [A]

무엇을 연속화해야 하는지부터 갈랐다. **성질이 달라 처방도 다르다.**

```
① 캡슐 높이        86 ↔ 60 (half-height, 반지름 30)    ⬜ **아직 이진** → [W9]
② 눈/카메라 높이    100 ↔ 32                            ⬜ 미착수    → [W10]
③ MM 데이터베이스   기립 클립셋 ↔ 웅크림 클립셋          ✅ 이산(설계) — 문턱에서 전환
④ AO 자산          AO_Rifle_ADS ↔ AO_Rifle_Crouch      ~~✅ 이진 Select — 연속 블렌드는 → [W11]~~
                                                       → ✅ 2026-09-30 관성 전환 0.25 s(`USoldierAnimLibrary::UpdateStanceAimOffset`, [W11] 해결)
   ★ 2026-09-30: 웅크림 판정(`Crouch()/UnCrouch()`)은 이제 슈미트 트리거 — 임계값 ± `SoldierLab.Pose.CrouchHysteresis 0.05` (P195)
```

**실측** [A]: `capsuleHalfHeight 86` / `crouchedHalfHeight 60` / `baseEyeHeight 100` /
`crouchedEyeHeight 32` / 메시 컴포넌트 relative Z **−88** / `bCrouchMaintainsBaseLocation` **원래 false**.

**클립 재고가 설계를 정했다** [A]:

```
기립     Walk + Jog          웅크림   Walk **만** (웅크려 달리는 클립이 없다)
웅크림 PSD 6개가 기립 세트를 미러링   중간 높이 클립은 **없다**
```

- 웅크려 달릴 **클립이 없으므로** 속도 상한은 선택이 아니라 제약이다 → 2.5f-3
- 중간 높이는 **클립이 아니라 변형**으로만 만들 수 있다 → 2.5f-2
- ★ **`walkSpeeds`와 `crouchSpeeds`가 둘 다 291.31**이라 **속도는 이미 자세와 분리돼 있었다**
  (P30의 실측 교정 결과) — 결합 하나가 공짜로 풀려 있었다

#### 2.5f-2 골반 오프셋 + LegIK — 높이의 연속성 [A]

```
FootPlacement_0  →  ModifyBone(pelvis)  →  LegIK_1
                        bone            = pelvis
                        Translation    ← MakeVector(0, 0, PelvisDrop)   ← ABP float
                        translationMode  = BMM_Additive
                        translationSpace = BCS_ComponentSpace
```

**`FootPlacement_0`이 골반을 소유한다**(`pelvisBone = pelvis`, `maxOffset 250`,
`linearStiffness 100`, `pelvisHeightMode AllLegs`, interpolation on). 그 **앞**에 오프셋을 넣으면
스프링이 되돌린다. **뒤의 `LegIK_1`은 `foot_l/r`을 이미 심어진 `ik_foot_l/r`로 되돌리므로**,
**사이**에서 골반만 내리면 **무릎이 굽는다** — 그것이 곧 웅크림이다.

- 사용자 확인: 기립 포즈 기준 **−50이 사용 한계**("스쿼트하는 정도")
- ⚠ 기존 설계 문서 두 곳이 이 축을 **"`FootPlacement` 앞 / Control Rig"** 로 적고 있었다.
  **둘 다 틀렸다** — 정정 주석을 달았다(`animation/2026-09-02_pose_pipeline_spec.md` 6.2절 ·
  `animation/2026-09-02_gasp_abp_analysis.md` 10절)

#### 2.5f-3 속도 상한 **과 Gait 클램프** — 둘 다 필요하다 [A]

```
cap = Lerp( runSpeeds.x , walkSpeeds.x ,
            MapRangeClamped( StanceAxis , 0 , StanceThreshold , 0 , 1 ) )
CharacterMovement.MaxWalkSpeed         = Min( 부모가 쓴 값 , cap )
CharacterMovement.MaxWalkSpeedCrouched = Min( 부모가 쓴 값 , cap )

Stance ≥ StanceThreshold  →  SetGait( NewEnumerator0 = Walk )     ← 부모의 SetGait 를 덮어쓴다
```

- **`Min`이라 우리 쓰기는 낮추기만 한다.** 부모(`UpdateMovement_PreCMC`)는 매 PreCMC 틱에
  **둘 다** 다시 쓰고, 우리 Tick이 그 뒤에 도는 것은 **2.5d-1의 틱 선행 조건** 덕이다
- ★★ **속도만 묶으면 안 된다** [A]. `GetDesiredGait`는 **입력 크기와 `CanSprint`로** Gait를 정하고
  **실제 속도를 보지 않는다.** 그래서 `Gait`가 Run에 남고 → 예측 궤적이 **582 기준**으로 만들어지고
  → 맞는 Jog Loop가 없어 → **`Jog_Stop`이 매 걸음 재선택**되며 떨었다.
  **뒷대각선 속도 불일치(P30·P31)와 서명이 정확히 같다.** → **P46**

#### 2.5f-4 DB 전환 [A]

```
StanceAxis ≥ StanceThreshold  →  Crouch()   (매 Tick)
StanceAxis <  StanceThreshold  →  UnCrouch() (매 Tick)
      → IsCrouching() → ABP Stance 열거형 → Chooser → Crouch 계열 PSD
```

**GASP의 DB 구동 선택 경로를 그대로 유지한다** — 사용자가 명시적으로 요구했다
(*"끄면 GASP의 기능을 거의 안 쓰는 것"*).

> ⚠ **결과: 기존 `IA_Crouch` 토글이 무력해졌다** [A]. stance 축이 `bIsCrouched`를 **매 프레임
> 소유**하므로 토글의 결과가 다음 프레임에 덮어써진다 → **[W13]**

#### 2.5f-5 ★★ 골반 높이 **역산** — 문턱 팝을 없앤 것 [A]

오프셋을 직접 정하지 않는다. **원하는 월드 높이를 정하고 매 프레임 역산한다.**

```
PelvTgt = StanceTargetHeight  ( ActorZ , MeshRelZ , StanceStandZ , StanceCrouchZ , StanceAxis )
PelvOff = SolveBoneHeightOffset( PelvTgt , PelvWZ(실측) , PelvOff(직전 프레임) )   → ABP PelvisDrop
```

두 함수는 **새로 추가한 C++**(4절):

```cpp
SolveBoneHeightOffset(Target, Measured, Applied)  =  Target - (Measured - Applied)
StanceTargetHeight(ActorZ, MeshRelZ, StandZ, CrouchZ, Stance)
                                                  =  (ActorZ + MeshRelZ)
                                                     + Lerp(StandZ, CrouchZ, Stance)
```

- **`− Applied` 한 항이 핵심이다.** 실측값에는 **직전 프레임에 우리가 넣은 오프셋이 들어 있어서**,
  그걸 빼야 **애니메이션 자신이 만든 높이**가 복원된다
- ★ **되먹임 루프가 아니다** [A]. 전개하면 `offset_n = Target_n − clipHeight_{n−1}` 로
  이전 오프셋이 **소거된다** — **1프레임 지연의 피드포워드 해**이지 진동할 극점이 없다.
  **그래서 블렌드 길이도 블렌드 커브도 알 필요가 없다**(그것이 실패의 원인이었다 → 아래)
- ⚠ **왜 C++인가**: **두 runtime float의 뺄셈**이 필요한데 블루프린트 팔레트에 산술 노드가 없고
  (**P33**), 지금까지 쓴 우회(`Lerp`=곱셈, `MapRangeClamped`=반파 정류기, `Delta(Rotator)`=회전 전용)로는
  표현되지 않는다. `USoldierAxisLibrary`는 **이미 존재하는 클래스**라 P13(에디터 닫고 빌드)에 안 걸린다
- `StanceDropMax` · `StanceRaiseMax` · `StanceBlendRate`는 **실패한 접근의 잔해로 이제 안 쓰인다** → **[W6]**

> ★★ **이 자리에 도달하기까지 같은 증상을 네 번 잘못 진단했다** — ① 전환 클립 재생
> (`MM_Rifle_Crouch_Entry/Exit`는 **참조 0건**이라 재생 경로가 없다) ② 메시 26유닛 점프
> (`ActorZ + MeshZ`가 기립·웅크림 **양쪽 다 0.284**, 세 값이 같은 프레임에 바뀐다 — 엔진이 이미 상쇄)
> ③ `bCrouchMaintainsBaseLocation = false` (**`true`로 바꿔도 아무것도 안 변했다**)
> ④ **실제 원인** — 우리 오프셋은 **즉시**, 클립 교체는 **DeadBlending으로 블렌드**. 계단 대 곡선.
> **틀린 셋이 전부 "정착값을 읽고 과도구간을 추론"한 것**이었다 → **P44**.
> 그리고 그 사이에 **앞서 두 번 통한 램프 패턴을 반사적으로 재적용**했다 → **P45**.
> 전문: `animation/prototypes/2026-09-12_continuous_stance_axis.md` 8절

#### 2.5f-6 `bCrouchMaintainsBaseLocation = true` [A]

`BeginPlay`에서 **`Class|CharacterMovementComponent|SetCrouchMaintainsBaseLocation`** 노드로 켠다.

- ⚠ **`ObjectTools.set_properties`로는 못 쓴다** — **비트필드(`uint8:1`)** 라서다(`CLAUDE.md` 6.1).
  세터 노드를 쓰면 **변경이 그래프에 보인다**는 이점도 있다
- ⚠⚠ **정직하게 기록한다: 이 설정은 그것을 위해 넣은 증상(문턱 팝)을 고치지 못했다** [A].
  HUD의 `CrouchMB` 행으로 런타임 값이 `true`인 것까지 확인했는데 증상이 그대로였다.
  **그 자체로는 옳은 설정이라 남겨 두었을 뿐이다** — 이것을 해결책으로 읽지 말 것

#### 2.5f-7 변수 · 튜닝값 · 계측 [A]

| 변수 | 위치 | 값 / 비고 |
|---|---|---|
| `StanceAxis` | 캐릭터 | 축 상태 0..1 |
| `StanceInput` | 캐릭터 | `IA_Stance`가 매 프레임 써 넣는다 |
| **`StanceRate`** | 캐릭터 · **Instance Editable** | **1.0** — 0→1 에 1초 |
| **`StanceThreshold`** | 캐릭터 · **Instance Editable** | **0.5** — **DB 전환점이자 속도 상한의 무릎.** 0.35에서 올렸다(2.5f-8) |
| **`StanceStandZ`** | 캐릭터 · **Instance Editable** | **89.7** — 기립 클립의 **메시 루트 위 골반 높이** |
| **`StanceCrouchZ`** | 캐릭터 · **Instance Editable** | **39.4** — 웅크림 클립의 같은 값 |
| `PelvisDrop` | **ABP** | `ModifyBone(pelvis)`의 Z 오프셋. `UpdateStance()`가 매 Tick 써 넣는다 |
| ~~`StanceDropMax` · `StanceRaiseMax` · `StanceBlendRate`~~ | 캐릭터 | **죽은 변수** → [W6] |

**함수**: `UpdateStance()` — **Event Tick 끝**, `UpdateBlindFire` **다음**에 호출.
**입력**: `IA_Stance`(Axis1D, **`IA_Lean` 복제**) — `IMC_Sandbox`에 **V**(내리기) · **B**(올리기, **Negate**).
**HUD 행 7개 신설**: `Stance` · `PelvWZ` · `PelvTgt` · `PelvOff` · `Crouched` · `SpdCap` · `GaitClamp`.

#### 2.5f-8 문턱 0.35 → 0.5 로 옮겨 **움찔을 없앴다** [B]

```
조건   제자리 · **조준 안 함** · 천천히 웅크리는 중 · stance **0.35~0.5**(= 당시 문턱 직후)
증상   약 1초에 한 번 움찔하고 **몸이 실제로 돈다**. 조준/기립/이동하면 사라진다
오버레이 판정 (`a.AnimNode.MotionMatching.DebugDrawInfoVerbose 1`)
       검색 대상 DB = `PSD_Rifle_Crouch_Idles` + `PSD_Rifle_Crouch_TurnInPlace` **둘뿐**
       (즉 "LowReady idle DB가 안 걸러진다"는 가설은 **틀렸다**)
       Blend Stack 에 `MM_Rifle_Crouch_TurnLeft_90`이 **두 벌**, time 0.48 / 1.57
       → 제자리회전 클립이 **약 1.1초마다 자기를 재트리거**하고 있었다
```

**해결은 사용자의 제안이었다 — `StanceThreshold` 0.35 → 0.5.** 확인: *"움찔구간 완벽히 사라짐"*.

> **기구는 증명되지 않았다** [B]. 그럴듯한 설명은 "0.35에서 전환하면 웅크림 클립보다 골반을
> **약 33유닛** 들어올려야 하고 0.5에서는 **약 25유닛**이라 다리·발 심기에 무리가 덜 간다"인데,
> **그것이 재트리거를 멈춘 경로는 추적하지 않았다.** 재발하면 볼 레버 둘 → **[C-79]** ·
> **[C-80]**(`OffsetRootBone.maxRotationError = 90`이 90° 제자리회전의 해소를 막는 **자초한 회귀** 후보).
> ⚠ 앞선 교정은 그대로 살아 있고 원인이 **아니다** — `Crouch_TurnInPlace.baseCostBias 0` ·
> `Crouch_Idles.loopingCostBias −0.10`.

#### 2.5f-9 ★ 설계 결정 — **자세는 AI가 소유하고 속도는 결과다** [A]

"웅크린 채 달리려 하면 **(A) 속도를 묶는가, (B) 웅크림을 푸는가**" — **(A) 채택**.

```
자세 = 생존 결정(노출 실루엣)   ← 협상 불가, AI가 소유하는 **입력**
속도 = 그 결정의 결과           ← 협상 가능
```

**(B)는 이동 요청이 실루엣을 조용히 높인다.** AI가 "일어서라"고 명령한 적이 없는데 일어서 있고,
그 결정이 **어디에도 기록되지 않아 추적 불가능**해진다. 엄폐 판단이 자세 높이를 읽는 이상
이것은 **원인을 모르는 피격**이 된다. → **P37 · P38**(소유자는 하나, 부수 효과로 덮어쓰지 않는다)

**슈터 관례의 손맛은 비용 없이 살릴 수 있다**: **스프린트 *입력*이 입력 계층에서 stance를 0으로
명령**하게 하면 된다. AI는 그 명령을 내리지 않으면 그만이다 — 규칙이 아니라 **입력의 의미**로 표현된다.
→ **[Q41]**

> **기각한 대안**: 두 MM 출력(기립/웅크림 로코모션)을 블렌드하는 것 [A].
> **위상 동기 마커가 필요**하고(재료는 있다 — ABP가 `Phase_History` · `Contact_L_History` ·
> `Contact_R_History`를 든다) **캐릭터당 MM 검색이 두 번**이 된다 — 45명 @60fps에서 불가능하고,
> 위상이 어긋나면 **발이 미끄러진다**. **채택: 로코모션만 한 문턱에서 이산, 나머지는 전부 연속.**

> 상세 · **네 번의 오진 연쇄** · 툴링 →
> `animation/prototypes/2026-09-12_continuous_stance_axis.md`

### 2.5g AI 다리 — 연속 축 넷에 붙은 같은 모양 [A] (2026-09-13)

축을 만든 쪽(2.5c~2.5f)과 **AI가 그것을 모는 법**은 다른 문제다. 후자는 `BP_SoldierCharacter`에
변수 5개와 축마다 `SelectFloat` 하나로 끝났다 — **애님 그래프는 한 줄도 바뀌지 않았다**(P3).

```
SetX( SelectFloat( A = RampAxisTo(X, clamp(AITarget), Rate, dt),
                   B = <기존 플레이어 식 그대로>,
                   bPickA = AIPoseDriven ) )
```

**`Rate` 가 플레이어의 키 스텝이 쓰는 바로 그 변수**라는 것이 요점이다 (P71) — AI는
사람보다 축을 빠르게 움직일 수 없다. 그리고 **`RampAxisTo` 에는 클램프가 없어서**
목표를 `MapRangeClamped` 로 싸야 한다(플레이어 경로는 키 스텝이 클램프를 품고 있었다).

지금 AI가 실제로 미는 것은 **`AITargetStance` 하나**다. 린·블라인드 파이어는 다리만
놓여 있다 → **[W19]**. 상세: 5.2절 · `ai/2026-09-13_ai_bridge_and_scene.md` 1절.

---

### 2.6 ★ 확정된 튜닝값

여기 있는 숫자는 전부 **실측 또는 실패를 통해 정해진 값**이다. 근거 없이 바꾸지 말 것.

**이동 속도** [A] — 클립의 `MoveData_Speed` 실측값과 일치시킨 것

```
walkSpeeds    (291.31, 291.31, 291.31)
runSpeeds     (582.62, 582.62, 582.62)
crouchSpeeds  (291.31, 291.31, 291.31)
sprintSpeeds  (700, 700, 700)             ← [C-60] 아직 클램프 밖 (1.20×)
```

**Lyra 라이플 세트는 전 방향 단일 속도로 저작돼 있다**(방향 간 오차 0.001).
GASP의 방향별 비율(1:0.9:0.75)을 옮겨오면 옆 −30%·뒤 −40%가 어긋나
**Loop가 영영 선택되지 않고 Start/Stop이 매 걸음 재선택된다.**
→ P30 · P31 · `animation/prototypes/2026-09-09_lyra_rifle_migration.md` 9절

**비용 편향** [A] — Epic 값을 그대로 쓰지 않고 우리 클립 밀도로 다시 잡은 것 (P18)

| DB | 우리 값 | Epic Dense | 왜 |
|---|---|---|---|
| `Stand_Idles.baseCostBias` | **0** | +0.10 | 분리 후 클립이 **1개**(Idle_ADS)라 패널티를 받으면 못 이긴다 |
| `Stand_TurnInPlace.baseCostBias` | **0** | −0.20 | 원래 −0.05였다. 2026-09-11에 0으로. **급선회 반응이 부족한지 재확인 필요** |
| `Crouch_TurnInPlace.baseCostBias` | **0** | 0 | −0.05를 넣었다가 웅크리기 발작을 일으켰다 |
| `Crouch_Idles.loopingCostBias` | **−0.10** | −0.005 | **의도적 이탈** — GASP은 웅크리기 idle이 4개, 우리는 1개다 |

나머지 DB는 GASP Dense와 동일하다.

**조준 · 급선회** [A] — 2026-09-11. **아래 두 값은 결합돼 있다. 반드시 같이 본다** (2.5b-2절)

```
OffsetRootBone_0.maxRotationError      ~~90~~ → −1 (= GASP 원본, 2026-09-15 복귀)
                                                ← 90 은 비조준 급선회 스냅의 원인이었다. 천장 없음 (2.4절 표 · [C-80])
Enable_AO 문턱  Idle / Moving          70 / 70 (GASP 115/180) ← 그 안의 "총을 내릴 결정선"
BlendListByBool_0  BlendTime_0 / _1    0.375 / 0.25 s (핀, GASP 0.1/0.1)
                                                ← true=0번이므로 AO 켤 때 0.375 · 끌 때 0.25

AimCorrection 게인 (Lerp Alpha)        0.05    ← 학습 시간상수 ≈ 0.33s (2.5c)
AimCorrection 게이트 (카메라 각속도)    ≤ 2.0°/frame
AimCorrection 게인 게이트              AOActive (Enable_AO)   ← 안티 와인드업 (2.5c, 2026-09-12)
AimCorrection Clamp                    ±25°

몸통 각속도  YawRate_Up / _Down        90 / 720 °/s            ← 2.5d (2026-09-12)
             WeaponLowerAngle / Full   30 / 65 °
             WeaponLowerRate / Raise   8.0 (0.125s) / 2.0 (0.5s)

블라인드 파이어  BlindFireRate         1.0   (0→1 에 1초)       ← 2.5e (2026-09-12)
                 BlindFireResetRate    3.0   (0 까지 약 0.33초)
                 BF 마스크             BranchFilter · spine_01 · blendDepth 1

stance 축        StanceRate            1.0   (0→1 에 1초)       ← 2.5f (2026-09-12)
                 StanceThreshold       0.5   DB 전환점 **이자** 속도 상한의 무릎
                                             ← 0.35였다. 0.5로 올려 웅크림 제자리회전
                                               재트리거(움찔)를 없앴다 [B] (2.5f-8)
                 StanceStandZ          89.7  기립 클립의 메시 루트 위 골반 높이
                 StanceCrouchZ         39.4  웅크림 클립의 같은 값
                 PelvisDrop 사용 한계  −50   기립 포즈 기준("스쿼트하는 정도") [A]
                 캡슐                  86 ↔ 60 **이진** (연속화 미착수 → [W9])
```

> **`StanceStandZ` / `StanceCrouchZ`는 "튜닝값"이 아니라 실측값이다** — 두 클립의 골반 높이다.
> 클립을 바꾸면 다시 재야 한다. `StanceThreshold`만이 취향의 값이고, **0.35 → 0.5 변경의
> 기구는 증명되지 않았다** [B] → [C-79].

**문턱 ≥ 천장이면 영원히 발동하지 않는다.** 90/70 둘 다 **감으로 정한 값**이다 → **[C-74]**.
**2026-09-12에 `WeaponLowerAngleFull 65`가 같은 쌍에 합류했다** — 이제 셋을 함께 본다(2.5d-2).
`YawRate_Up 90` · `YawRate_Down 720`도 계측값이 아니라 PIE에서 눈으로 정한 값이다 [B].

---

## 3. 캐릭터 · 무기 · 입력 [A]

```
/Game/SoldierLab/Blueprints/BP_SoldierCharacter    GASP SandboxCharacter_CMC 복제
    WeaponMesh = ~~SK_Rifle~~ SK_AR4_X (3.1절 교체) @ 소켓 weapon_r
                 ~~relLoc (−10, 0, 0) · relRot yaw 90 · animClass ABP_Weap_Rifle~~
                 → ★ 2026-09-15: relLoc **(0,0,0)** · relRot **(0,0,0)** (MCP 되읽기 확인). 옛 오프셋 (−10,0,0)·yaw 180(위 줄엔 90 으로 적혀 있었으나 2026-09-15 실측은 180)은
                   세 메시의 `weapon_r` **메시 소켓**에 흡수됐다(3.2절 소켓 표). 애님 에디터 프리뷰(소켓 직결)와 런타임이 같아진다
                 ★ WeaponMesh 의 실제 역할 3가지 [A]: ① BP_AR4Rifle 부착 앵커 ~~② BeginPlay 에서 SetVisibility(false) (P48)~~
                   ③ **총구 보정(2.5c)이 `WeaponMesh.GetSocketTransform("Muzzle", World)` 를 읽는다** · 그리고 2026-09-15 부터
                   ④ 왼손 그립 오프셋 산출이 `WeaponMesh.GetSocketLocation("LeftHandGrip")` 을 읽는다(2.5절).
                   사망 드롭 용도 아님. → 이 컴포넌트의 메시는 **스폰되는 총과 같은 메시**여야 하고 `Muzzle` · `LeftHandGrip` 소켓이 필요하다.
                   > ★ **정정 2026-09-21 (게임 스레드 묶음 세션, `ai/2026-09-21_game_thread_batch_cameras_abp_muzzle.md` 2.3절) [A]** — ② 는 반대다.
                   > BeginPlay 의 `SetVisibility(false)`(노드 `K2Node_CallFunction_84`) 는 self ← **SpawnActor 결과**, 즉 **스폰된 총 액터 `BP_AR4Rifle` 의 WeaponMesh** 를 숨긴다.
                   > **캐릭터의 WeaponMesh(SK_AR4_X / 적군 SK_KA74U_X)가 화면에 보이는 총**이다 → 폰당 스켈레탈 메시 2(캐릭터 + 캐릭터 WeaponMesh) + 총 액터 1.
                   > 캐릭터 WeaponMesh 의 `AnimClass = ABP_Weap_Rifle` 은 본 이름 0개 일치(`USkeleton::IsCompatibleMesh`, `Skeleton.cpp:648`)라 인스턴스가 안 생기던 **죽은 참조** → **None** 으로 정리.
                   > 숨은 총 액터 메시는 `VisibilityBasedAnimTickOption = OnlyTickPoseWhenRendered`(0.26 → 0.07 ms). ③④ 는 그대로 맞다.
                   아군/적군 총이 갈라질 때의 규약 → [W67]
    gait       = Run   ← 기본값. walkSpeeds는 걷기 입력이 있어야 쓰인다 ([C-52])
    AimCorrection  (Rotator)  총구 정렬 누적 보정값. 매 Tick 갱신 → ABP로 전달 (2.5c)
    PrevAimRot     (Rotator)  직전 프레임의 GetControlRotation. **속도 게이트의 상태**다 (2.5c)
                              ⚠ 대입이 IsLocallyControlled 분기 안에 있다 → [C-75]
    AimLoopActive  (Boolean)  ⚠ **조준 보정에는 쓰이지 않는다** — 죽은 오차 크기 게이트
                              (`SelectFloat 25/20` → `InRange`)만 구동하고, 그 결과가
                              **화면의 `GATE` 표시**로 나간다. 즉 화면의 GATE는 실제 게이트가
                              아니다. 정리 대상 → **[W6]** (프로토타입 문서 11절)

    ── 2026-09-12 추가 (2.5d) ──────────────────────────────────────────────
    AOActive       (Boolean)  ABP 의 AOActive 를 **매 Tick 캐시**한 것. 1프레임 지연.
                              ⚠ 인라인으로 읽지 않고 캐시하는 이유: `CastToSoldierCharacter_ABP`
                              노드가 Tick 안에서 `SetAimCorrection` **뒤에** 실행되므로
                              순수 체인으로 끌어오면 그 시점엔 **null**이다
    WeaponLowered  (Float)    0=견착 1=총내림. 각속도 보간 알파이자 조준 게인 감쇠 알파
    YawRate_Up / YawRate_Down / WeaponLowerAngle / WeaponLowerAngleFull /
    WeaponLowerRate / WeaponRaiseRate                 ← 전부 **Instance Editable**. 값은 2.5d-2
    함수 UpdateBodyYawRate()   Event Tick **끝**에서 호출
    BeginPlay                  `Parent: BeginPlay` 직후에 틱 선행 조건 2줄 (2.5d-1) ★ 전제

    ── 2026-09-12 추가 (2.5e · 블라인드 파이어) ──────────────────────────
    BlindFireH / BlindFireV                (Float)  축 상태. −1..+1 / 0..+1
    BlindFireInputH / BlindFireInputV /
    BlindFireInputReset                    (Float)  입력 액션이 매 프레임 써 넣는다
    BlindFireRate 1.0 / BlindFireResetRate 3.0       ← **Instance Editable**. 값은 2.5e-5
    함수 UpdateBlindFire()     Event Tick **끝** — `UpdateBodyYawRate` **다음**에 호출

    ── 2026-09-12 추가 (2.5f · 연속 stance 축) ──────────────────────────
    StanceAxis                             (Float)  축 상태 0(기립)..1(웅크림)
    StanceInput                            (Float)  IA_Stance 가 매 프레임 써 넣는다
    StanceRate 1.0 / StanceThreshold 0.5 /
    StanceStandZ 89.7 / StanceCrouchZ 39.4           ← **Instance Editable**. 값은 2.5f-7
    ~~StanceDropMax · StanceRaiseMax · StanceBlendRate~~  **죽은 변수** → [W6]
    함수 UpdateStance()        Event Tick **끝** — `UpdateBlindFire` **다음**에 호출
                               ① 축 램프 ② 속도 상한(Min) + **Gait 클램프** ③ Crouch()/UnCrouch()
                               ④ 골반 높이 역산 → ABP.PelvisDrop
    BeginPlay                  `SetCrouchMaintainsBaseLocation(true)` 추가 (2.5f-6)
                               ⚠ 비트필드라 `set_properties`로는 못 쓴다
    ⚠ **`IA_Crouch` 토글은 이제 무력하다** — stance 축이 `bIsCrouched`를 매 프레임 소유한다 → [W13]

    ── 2026-09-15 추가 (체력 · 피격 · 사망) ──────────────────────────────
    AC_SoldierHealth   (USoldierHealthComponent, SCS 컴포넌트 — MCP ActorTools.add_component)
                               템플릿 BP_SoldierCharacter_C:AC_SoldierHealth_GEN_VARIABLE 에
                               몽타주 배열 13+6 을 set_properties 로 직접 기입(C++ CDO 배열이
                               템플릿에 상속되지 않았다 — P127. 생성자 수정분은 다음 빌드 대기)
    Tick 2.5c GATE     K2Node_CallFunction_56 (InRange 0..2) 출력이 곧장 소비자로 가던 것을
                               ANDBoolean( InRange_56 , NOTBoolean( AC_SoldierHealth.IsHitReacting ) )
                               로 바꿈. AND 출력 → SelectRotator_42.bPickA (누적/유지) ·
                               CallFunction_49 (AimGate HUD 행) · CallFunction_4 (보조) 셋 전부.
                               피격 몽타주 재생 중엔 총구 보정 적분기가 **유지**(P35·P37)
    BP_Soldier_Friendly        상속 AC_SoldierHealth 의 `Invincible (무적)` = true (사용자가 에디터에서.
                               도구로 자식 BP 오버라이드는 못 쓴다 — P128)

    ── 2026-09-17 추가 (진영별 애님 세트 · 2.4b) ─────────────────────────
    UseAllyAnimSet     (Boolean, Instance Editable)  기본 false = 적군 세트
                               BeginPlay 의 Cast(SoldierCharacter_ABP) 뒤에서 ABP 의 동명 변수로 전달
                               BP_Soldier_Friendly = true / BP_Soldier_Hostile = false
    FireMontage /
    ReloadMontage      (AnimMontage, Instance Editable)  PlayAnimMontage 의 핀으로 들어간다
                               (옛 배선은 AM_MM_Rifle_Fire / _Reload 를 리터럴로 물고 있었다)
    AC_SoldierHealth 템플릿    몽타주 19개를 **여기서** 들고 있다 — C++ 생성자의 경로 하드코딩을 걷어냈다(P134)
                               BP_SoldierCharacter = Enemy_ 19 / BP_Soldier_Friendly = ALLY_ 19 오버라이드
                               BP_Soldier_Hostile 은 자체 템플릿이 없어 부모(Enemy) 상속
    AC_SoldierHealth.bPlayDeathMontage = **false**  (BP_SoldierCharacter · BP_Soldier_Friendly)
                               순수 래그돌. 사용자 결정 → `ai/2026-09-17_hit_death_three_causes.md` 6절
    ⚠ 자식 BP 의 CDO 는 부모 기본값을 **안 물려받는다** (P137) · 배치 인스턴스는 템플릿 갱신을
      **안 따라온다** (P139) · 읽기 전용 에셋의 쓰기는 성공하고 **저장만 실패**한다 (P136)

/Game/SoldierLab/Animation/SoldierCharacter_ABP
    UseAllyAnimSet (Boolean)  ★ 2026-09-17. 진영 분기 5축의 구동 변수 (2.4b)
    AimCorrection  (Rotator)  캐릭터가 매 Tick 써 넣는다. Get_AOValue 에서만 쓰인다 (2.5c)
    AOActive       (Boolean)  **`Update_Logic` 에서 `Update_States` 직후**에 `Enable_AO()` 결과를
                              대입한다 [A] (2026-09-12). 그 자리인 이유는 신선한
                              `RootTransform`/`RotationMode`를 보게 하려는 것.
                              캐릭터가 이 값을 읽어 조준 보정 게인을 끊는다 (2.5c)
    BF_AlphaL / BF_AlphaR / BF_AlphaU   (Float)   블라인드 파이어 3레이어의 애디티브 알파.
                              캐릭터의 `UpdateBlindFire()`가 매 Tick 써 넣는다 (2.5e)
    PelvisDrop     (Float)    `FootPlacement_0` 과 `LegIK_1` **사이**의 `ModifyBone(pelvis)` Z 오프셋.
                              캐릭터의 `UpdateStance()`가 **역산해서** 매 Tick 써 넣는다 (2.5f)

/Game/SoldierLab/Blueprints/GM_SoldierLab          defaultPawn = BP_SoldierCharacter
/Game/SoldierLab/Input/IA_Fire · IA_Reload         IMC_Sandbox에 매핑
/Game/SoldierLab/Input/IA_Lean                     린 축 ⚠ 이 문서에 절이 없다 → [W7]
/Game/SoldierLab/Input/IA_BlindFireH · IA_BlindFireV · IA_BlindFireReset
                                                   Axis1D. **`IA_Lean` 복제**로 생성 (2.5e-6)
                                                   키 1(Negate)·3 → H · 2 → V · 4 → Reset
/Game/SoldierLab/Input/IA_Stance                   Axis1D. **`IA_Lean` 복제** (2.5f-7)
                                                   키 **V** → 내리기 · **B**(Negate) → 올리기
                                                   ⚠ C를 못 쓴 이유: 기존 `IA_Crouch` 토글과 충돌
/Game/Weapons/Rifle/Mesh/SK_Rifle                  + LeftHandGrip 소켓
```

**EventGraph 배선**

```
── 2026-09-12 갱신 (3.1절 · 투사체 이식) ────────────────────────────────
IA_Fire   (Triggered) → Rifle.Shoot()              ← **몽타주를 직접 재생하지 않는다**
Rifle.OnWeaponFired (디스패처)
                      → PlayAnimMontage( ~~AM_MM_Rifle_Fire~~ → 변수 **FireMontage** )   ★ 2026-09-17 (2.4b)
                        ↑ 탄이 실제로 스폰·발사된 뒤에만 방송된다 (P49)
IA_Reload (Triggered) → PlayAnimMontage( ~~AM_MM_Rifle_Reload~~ → 변수 **ReloadMontage** )

~~WeaponMesh.GetAnimInstance.Montage_Play(AM_Weap_Rifle_Fire / AM_Weap_Rifle_Reload)~~
    **삭제됨** — 무기 메시를 `SK_AR4_X`로 갈아끼웠고 그쪽엔 애님 블루프린트가 없어
    `GetAnimInstance`가 항상 None이었다 (`Accessed None ... Node: Montage_Play`). P50

IA_Lean / IA_BlindFireH / IA_BlindFireV / IA_BlindFireReset / IA_Stance
                                                                ← 연속 축 입력 (공통 형태)
          (Triggered) → Set <Input 변수> = ActionValue
          (Completed) → Set <Input 변수> = 0
          ⚠ 트리거 목록은 **비워 둔다** — `InputTriggerPressed`를 걸면 1프레임만 들어온다
```

> ⚠ 몽타주의 슬롯이 서로 다른 **슬롯 그룹**에 걸쳐 있으면 몽타주가 통째로 무효가 된다.
> 재장전이 "총만 움직이고 캐릭터는 가만히" 있으면 이것이다. `LogAnimMontage`에 원인이 찍힌다 (P24).

---

### 3.1 무기 액터 · 투사체 [A] (2026-09-12)

**전문: `weapons/2026-09-12_projectile_port.md`** — 끊어낸 의존 4종, 버그 6건, 남은 것.

```
Source/SoldierLab/Weapons/SoldierProjectile.h (~560줄) / .cpp (~1050줄)
    titan_example 의 ARCWSProjectile 이식. **히트스캔이 아니라 중력 포물선 투사체**
    (리드 사격·탄착 낙차·비행 시간이 AI 층이 쓸 물리적 재료다)
    풀링 설계: LaunchFrom() / Deactivate()
    ~~⚠ **풀 자체는 없다** — 무기 컴포넌트가 없어 발당 SpawnActor 한다.~~
      LaunchFrom 은 어느 쪽이든 같으므로 풀 추가는 **쏘는 쪽의 변경**이다
    ★ 09-21 [A]: 없는 것이 비용이 아니라 **누수**다 — Deactivate()(.cpp:1191)는 숨김+충돌·틱 off 만이고
      Destroy 가 없는데 BP_AR4Rifle.Shoot/PlayShotCosmetics · 도탄 Multicast_LaunchRicochet
      (AI/SoldierEngagement.cpp:248-268)이 발마다 SpawnActor → 주차된 투사체가 **영구 누적**
      (비행 21발 프레임에 TracerTrailComponent 80개). 발당 스폰 0.94 ms. 풀 + 빈 EventTick 삭제 +
      QueryOnly → ~~**[W109] 착수**~~ ★ **09-21 후편 [W109] 해결** (`ai/2026-09-21_game_thread_structural_pool_rays_bridge.md` A절):
    ★ **풀 = `Weapons/SoldierProjectilePool.h/.cpp` `USoldierProjectilePoolSubsystem`**(월드 서브시스템, 2026-09-21)
      `Acquire(TSubclassOf<ASoldierProjectile>, AActor* Owner)` — 클래스별 풀, 주차된(틱 꺼진) 것을 커서부터 재사용
      → 없으면 상한까지 스폰 → 상한이면 RCWS 풀처럼 라운드로빈(클래스당 1회 경고). 상한 cvar **`SoldierLab.Projectile.PoolMax` 96**
      쏘는 쪽 전부 `Acquire → LaunchFrom`: BP_AR4Rifle.Shoot · PlayShotCosmetics · 도탄 Multicast_LaunchRicochet(.cpp:263)
      CollisionComponent **QueryOnly**(.cpp:124, ← QueryAndPhysics — 명중은 ProjectileMovement 이동 스윕의 블로킹 히트, OnComponentHit 그대로)
      **`MaxFlightDistanceCm 60000`**(.h:206, 600 m) — 시간 상한 `MaxFlightTimeSeconds 5`(.h:199)와 같은 자리(Tick .cpp:264-267)에서 주차, 도탄은 튕긴 지점부터
      결과: Shoot 안 스폰 0.83 ms/발 → 0 · 투사체 액터 47 에서 정지(비행 15). 3.1 의 "발당 SpawnActor" 서술은 09-21 전 이야기다
    총구 섬광·발사음·반동은 **여기 없다** — 총구에서 일어나므로 무기 액터 소유

/Game/SoldierLab/Weapons/Blueprints/BP_RifleProjectile    부모 /Script/SoldierLab.SoldierProjectile
    ★ 09-21 후편: EventGraph 의 빈 EventTick · BeginPlay · Overlap 노드 삭제(발당 BP ReceiveTick 디스패치 제거) — 그래프는 비어 있는 것이 맞다
    M_RCWSRound · NS_Rifle_Tracer · NS_Rifle_Dirt + MS_hit_rifle_dirt
    NS_Blood + MS_hit_rifle_enemy · MI_Blood(적 + 지면 혈흔)  · MS_bullet_whizz
    ~~surfaceImpactEffects **5행** — 행마다 이펙트 + 사운드 + M_Decal_Bullet + MS_Ricochet~~
    ⚠ **09-21 정정**: 디스크 CDO 확인 시 5행이 전부 Dirt/None 빈 껍데기였다(왜 비어 있었는지는 미확인).
      titan 구 BP 값으로 다시 채움 — `sfx_vfx/2026-09-21_combat_audio_voice_budget_and_attenuation.md` §6.
      피격음 감쇠 1000/150000 → 200/2500 도 같은 날
    ⚠ 마이그레이션본은 **부모가 /Script/titan_example.RCWSProjectile 이라 12.8KB 껍데기**였다
      (컴포넌트·그래프 전부 소실). **C++ 이식 후 0부터 다시 만들었다** — 순서는 C++ 먼저다

/Game/SoldierLab/Weapons/Blueprints/BP_AR4Rifle           부모 /Script/Engine.Actor (순수 BP)
    Shoot: Branch(CanShoot?) → Branch(탄약) → 총구 사운드 → 총구 섬광 NS(09-21: 상주 MuzzleFlashFX Activate)
           → CanShoot?=false → shotsFiredCount++ → ~~SpawnActor BP_RifleProjectile~~
             ★ 09-21 후편: GetSoldierProjectilePoolSubsystem → Acquire(BP_RifleProjectile_C, OwningCharacter)
           → SoldierProjectile::LaunchFrom → ammoInMag-- → **CallOnWeaponFired**
    탄창이 비면 자동 StartReload · SetTimerByEvent(fireRate) 가 CanShoot? 를 재장전
    기본값  fireRate 0.12 · muzzleVelocity 80000 · tracerInterval 3
            magSize 30 · bulletSpreadDegrees 3          → 전부 이식값. **[C-82]**

BP_SoldierCharacter · BeginPlay 추가분
    라이플 스폰 → WeaponMesh 에 부착(SnapToTarget)
    → 구 GASP 무기 메시 **컴포넌트 단위** SetVisibility(false, propagate=false)   ← P48
    → SetActorEnableCollision(Rifle, false)                                        ← P48
    → ★ 2026-09-15: Cast(Mesh.GetAnimInstance → SoldierCharacter_ABP)
      → ABP.LeftHandGripOffset = InverseTransformLocation( Mesh.GetSocketTransform("weapon_r", World),
                                                           WeaponMesh.GetSocketLocation("LeftHandGrip") )   ← 2.5절
    → Rifle->OwningCharacter · Rifle 변수 보관
    → OnReloadStarted / OnWeaponFired 바인드
```

**이식하며 끊어낸 의존 4종** — 넷 다 호출 지점에 복구 방법이 주석으로 남아 있다 → **[W17]**

| 원본 | 대체 |
|---|---|
| `UDetectableTargetComponent::Faction` | `bHitEnemy = OtherActor && OtherActor->IsA<ACharacter>()`. 틀려도 **이펙트를 잘못 고르는 것뿐** → **[R7]** |
| `ReportHitToInstigator` (3분기 Multicast) | `PlayImpactEffect(InstigatorActor, ...)` 직접 호출 — 세 핸들러가 전부 여기로 되돌아왔다 |
| `ReportRicochetToInstigator` (3분기 Multicast) | `World->SpawnActor<ASoldierProjectile>(...)` + `LaunchFrom(...)` |
| `AWindSource::GetWindVectorCmsForNiagara` | `SetVectorParameter(FName("WindVectorCms"), FVector::ZeroVector)` |

**살아남은 기능**: 포물선 탄도 · 풀링 배관 · 트레이서 · 도탄(바운스 캡 3) ·
재질별 명중 세트(Wood/Hard/Dirt/Metal/Glass) · 데칼 FIFO 상한(200) · 지면 혈흔 ·
명중 화염 · 카메라 셰이크 · **총알 휘파람**.

> ★ **총알 휘파람이 AI 제압 신호의 자리다.** `Tick()`의 휘즈 블록이
> `ClosestPointOnSegment`로 **이번 프레임 이동 선분과 청취자의 최근접 거리**를 구한다 —
> 제압 신호가 필요로 하는 기하 그 자체다. **지금은 로컬 카메라 하나만 보고 소리만 낸다.**
> 주변 병사로 확장하는 것이 예정된 훅 → **[W16]**.
> 값: `WhizDetectionRadiusCm 200` · `WhizBroadPhaseRadiusCm 1500`(광역 컬링) [A]

**`SoldierLab.Build.cs`** — `PublicDependencyModuleNames` 에 **`Niagara`** 추가.
에셋 참조 때문이 아니라 **투사체가 나이아가라 `User.` 파라미터를 직접 써서** 링크 타임에 필요하다.

**`Config/DefaultEngine.ini`** [A]

```ini
[/Script/Engine.PhysicsSettings]
+PhysicalSurfaces=(Type=SurfaceType1,Name="Wood")    ; ⚠ **선언 순서가 의미를 갖는다**
+PhysicalSurfaces=(Type=SurfaceType2,Name="Hard")    ;   코드는 이름으로 찾지만
+PhysicalSurfaces=(Type=SurfaceType3,Name="Dirt")    ;   마이그레이션한 피지컬 머티리얼
+PhysicalSurfaces=(Type=SurfaceType4,Name="Metal")   ;   .uasset 은 **인덱스**를 저장한다.
+PhysicalSurfaces=(Type=SurfaceType5,Name="Glass")   ;   재정렬·삽입 = 조용한 오작동

[/Script/Engine.CollisionProfile]
+DefaultChannelResponses=(Channel=ECC_GameTraceChannel4,DefaultResponse=ECR_Block,bTraceType=True,...,Name="Cover")
+DefaultChannelResponses=(Channel=ECC_GameTraceChannel5,DefaultResponse=ECR_Block,bTraceType=True,...,Name="Sight")
;   한 채널이 아니라 두 채널인 것이 설계다 — 철망·유리는 총알을 막고 시야는 통과시킨다([Q21]).
;   **지금 파는 이유**: 나중에 추가하면 이미 배치된 에셋 전부의 응답을 손봐야 한다
```

> ⚠ 이 파일은 Perforce 읽기전용 플래그를 **사용자 1회 허가로** 지우고 편집했다.
> **`DefaultEngine.ini.bak` 백업 존재 · P4V 체크아웃 미처리** → **[W15]**

> ⚠ **이식 전 예고("두 프로젝트의 콜리전 채널 인덱스가 충돌한다")는 틀렸다.**
> `titan_example`은 커스텀 채널을 하나도 쓰지 않는다. 기존 채널 1~3
> (`Traversable`/`Mouse`/`Obstacle`)은 전부 GASP 것이고 우리는 그 뒤에 이어 붙였다 → **P52**

---

### 3.2 캐릭터 메시 — 아군 `soldier_T` 를 마네킹 스켈레톤에 [A] (2026-09-13)

**전문: `animation/prototypes/2026-09-13_ally_mesh_on_mannequin_skeleton.md`** — 실측·엔진 근거·기각한 경로·원복한 시도.

원칙: **메시를 `SK_UEFN_Mannequin` 에 맞춘다.** PSD/스키마/애니메이션은 한 벌이고 어느 것도 바뀌지 않았다.

```
/Game/SoldierLab/Characters/Ally/soldier_T          스켈레톤 = SK_UEFN_Mannequin   ← Assign Skeleton (FBX 재임포트 아님)
    87본 (마네킹 91본 중 attach · weapon_l/r · props_root · prop_01 · poi 6본이 없고, thigh_twist_02_l/r 2본이 더 있다)
    바인드 T-포즈 · 키 181cm (마네킹 A-포즈 · 165.6cm) — 같은 스켈레톤 에셋이면 바인드 포즈 차이는 무관하다
    메시 소켓 weapon_r        부모 hand_r · ~~(0.19749, 3.411153, −0.381067) · rot 0~~ → **(−9.80, 3.41, −0.38) · yaw 180** (2026-09-15, 아래 소켓 표)    ← 총 부착 + 왼손 IK effector
    메시 소켓 Rifle_Socket    titan 구 시스템(BP_Ally_kadex) 잔재 — SoldierLab BP/ABP 참조 0건. 무시/삭제 가능 (titan 쪽 동명 메시의 것은 건드리지 말 것, [W33])
                              ★ 2026-09-17 디스크 스캔 재확인: **새 시스템에 필요한 캐릭터 메시 소켓은 `weapon_r` 하나뿐이다** [A]
    머티리얼 슬롯 Ch15_body / Ch_49_body / Ch_49_eyelashes  ← Mat_Soldier / Mat_soldier2 / Ch_49_eyelashes
    피직스 에셋 ~~soldier_T_PhysicsAsset~~ → ★ **PA_UEFN_Mannequin** (2026-09-17 교체, MCP assign_physics_asset)
                  자동 생성본은 관절 제한이 기본값이라 래그돌에서 **무릎·팔꿈치가 반대로 꺾였다.**
                  스켈레톤이 같아 본 이름으로 매칭된다. 대가: 바디가 마네킹 체형 기준이라 캡슐이 메시와 약간 어긋난다
                  → 진영별 복제 + 바디 피팅은 **[W72]**(에디터 수작업) · 정량 판정 **[C-127]**
    포스트프로세스 ABP 없음  ← 마네킹의 ABP_UEFN_Mannequin_PostProcess 는 thigh 1.12 · head 1.15 스케일 보정이라 붙이지 않는다
/Game/SoldierLab/Characters/Ally/soldier_T_Skeleton  고아 (참조 0건) → 삭제 예정 [W29]

/Game/Characters/UEFN_Mannequin/Meshes/SKM_UEFN_Mannequin
    메시 소켓 weapon_r        부모 본 weapon_r · ~~항등~~ → **(−10, 0, 0) · yaw 180** (2026-09-15)    ← 2.5절의 소켓 모드 때문에 필요
/Game/Characters/UEFN_Mannequin/Meshes/SK_UEFN_Mannequin
    본 트리 91 → 93  (+ thigh_twist_02_l / thigh_twist_02_r, Assign Skeleton 이 병합)  GUID 재생성 · 전 클립 DDC 재압축 1회
                     ★ 적군 new_enemy_T Assign 때는 본 추가 0 (2026-09-14) — 스켈레톤 무변경

★ 2026-09-14~15 — 적군 (전문: `animation/prototypes/2026-09-14_enemy_mesh_on_mannequin_skeleton.md`)
/Game/Soldiers/New_enemy_soldiers/NewFolder/new_enemy_T    스켈레톤 = SK_UEFN_Mannequin   ← Assign Skeleton (본 추가 0)
    UE5 표준 명명 · 111,083 버텍스 · LOD 1 · 키 178.7 cm
    메시 소켓 weapon_r        부모 hand_r · (−9.80, 3.41, −0.38) · yaw 180        ← 아군과 같은 값
    피직스 에셋 ~~new_enemy_T_PhysicsAsset~~ → ★ **PA_UEFN_Mannequin** (2026-09-17, 아군과 같은 이유·같은 대가)
    머티리얼 슬롯 Ch_49_body2 / Ch_49_eyelashes / Ch_49_body1                     ← 임포트 시 전부 WorldGridMaterial 이던 것을
                                                                                    MCP set_material 로 상위 폴더 MI Ch_49_body1/body2/eyelashes 에 할당
    포스트프로세스 ABP 없음   ← 아군과 같다. 그래서 BF 포즈의 구워진 head 1.15 가 이 메시에서도 드러났다 (2.5e-7)
    ⚠ titan 폴더(`Soldiers/`)에 있다 — SoldierLab 밖의 우리 것. 이동은 정리 세션에서 (P96)
    납품 이력: ① `New_enemy_soldiers/enemy_T`(87본, 스켈레톤 문제로 재납품) ② `NewFolder/enemy_T`(80본 **Auto-Rig Pro 명명**, 겹치는 본 5개 → 기각) ③ 이것
/Game/Soldiers/New_enemy_soldiers/NewFolder/new_enemy_T_Skeleton   고아 → 삭제 후보 [W29]
/Game/Soldiers/New_enemy_soldiers/enemy_T (+_Skeleton · _PhysicsAsset)   첫 납품. 참조 = 자기 것뿐 → 삭제 후보 [W29] (소켓만 dirty)
/Game/Soldiers/New_enemy_soldiers/NewFolder/enemy_T                        둘째 납품(ARP) → 삭제 후보 [W29]
/Game/SoldierLab/Characters/Enemy/Enemy (+_Skeleton · _PhysicsAsset)       옛 Mixamo → 삭제 후보 [W29]. ⚠ `Characters/Enemy/Materials/` 는 아군이 참조 — 삭제 금지 [W37]

/Game/SoldierLab/Blueprints/BP_Soldier_Friendly    부모 BP_SoldierCharacter · Faction Friendly   (2026-09-13 AI 세션에서 생긴 자식 BP)
    CharacterMesh0.SkeletalMesh = soldier_T
    CharacterMesh0.Materials[0] 오버라이드 리셋      ← 부모의 MI_UEFN_Mannequin_CMC 가 슬롯 0(Ch15_body)을 덮었다
    StanceStandZ 77.1 · StanceCrouchZ 36.4           ← 마네킹 89.7 / 39.4. **메시별 실측값** (아래)
/Game/SoldierLab/Blueprints/BP_Soldier_Hostile     ~~마네킹 그대로. 적군 메시는 미착수 → [Q42]~~
    → ★ 2026-09-14~15: CharacterMesh0.SkeletalMesh = **new_enemy_T** · 머티리얼 오버라이드 3슬롯 None · 사용자 PIE "플레이 잘 됨, 총 붙음"
      StanceStandZ / StanceCrouchZ  **미실측** → [C-119] (아군 77.1 / 36.4 시작값 권고. 현재 값이 무엇인지도 미확인)

L_SoldierTest   Ally_A · Ally_B · Ally_B2 · Ally_B3 = BP_Soldier_Friendly / Enemy_A · B · C = BP_Soldier_Hostile
```

> ★ **정정 2026-09-21 — 병사 메시(`CharacterMesh0`) 콜리전은 QueryOnly 가 아니었다** [A] (`ai/2026-09-21_game_thread_structural_pool_rays_bridge.md` C절)
> `ai/2026-09-21_perf_instrumentation_and_cover_cost.md` 7절 [W98] ② 와 이 문서가 "살아 있는 병사의 피직스 바디는 QueryOnly" 라고 전제했지만, **실제 CDO 는 GASP 커스텀 프로파일 `QueryAndPhysics`** 였다(MCP 되읽기).
> 09-21 후편에서 **`QueryOnly` 로 바꿨다** — 부모 `BP_SoldierCharacter` CDO + `BP_Soldier_Friendly`/`_Hostile` CDO + `L_SoldierScenario` 인스턴스 35(CDO 쓰기는 자식/인스턴스에 전파 안 됨). 같은 자리에서 CMC `bAlwaysCheckFloor=false` · `bEnablePhysicsInteraction=false`([W111]).
> 안전한 이유: 사망 래그돌은 `AI/SoldierHealth.cpp:554-572 StartRagdoll` 이 **`QueryAndPhysics` 로 되돌리고**(`:572`) 피격 부위 판정 `ResolveBoneByTrace` 는 Query 다. 키네마틱 본 스킵 불가(전편 2.2절)는 그대로 — Query 를 받는 바디는 포즈를 따라가야 한다.
> `FEndPhysics` 에 대한 효과는 투사체 QueryOnly 와 같은 프레임에 들어가 **분리 판정 보류 [B]**. **오브젝트 타입은 ECC_Pawn 그대로** — 09-21 부터 AI 트레이스가 병사를 `AddIgnoredActor` 대신 **Pawn 채널 응답 Ignore**(`AI/SoldierQuery.h BodiesAreNotWalls()`)로 거르므로, 병사 메시/캡슐의 오브젝트 타입을 바꾸면 트레이스가 병사에 막힌다.

**`weapon_r` 메시 소켓 규약 (2026-09-15 확정)** [A] — 총 부착 오프셋은 컴포넌트가 아니라 **메시 소켓**이 갖는다(3절 `WeaponMesh` 상대 0):

| 메시 | `weapon_r` 소켓 | 비고 |
|---|---|---|
| `SKM_UEFN_Mannequin` | 부모 본 `weapon_r` · **(−10, 0, 0) · yaw 180** | 본 기준. 애니메이션 따라감 |
| `soldier_T` (SoldierLab) | 부모 `hand_r` · **(−9.80, 3.41, −0.38) · yaw 180** | = 마네킹 본 레스트 오프셋 + 컴포넌트 오프셋 |
| `new_enemy_T` | 부모 `hand_r` · 같은 값 | |

메시 소켓은 스켈레탈 메시별이라 **아군/적군이 다른 총·다른 소켓값을 가질 수 있다.** 이득: 애님 에디터 Skeleton Tree → `weapon_r` → Add Preview Asset 으로 `SK_AR4_X` 를 붙이면 **런타임과 같은 자리**에 온다(디자인팀 왼손 FK 작업의 전제).

**새 메시 하나를 붙일 때 필요한 것 (체크리스트)** — 애니메이션 쪽은 손대지 않는다:

1. 스켈레탈 메시 에디터 → Asset → **Assign Skeleton** → `SK_UEFN_Mannequin`. 본 이름이 UE5 표준이고 부모 체인이 같아야 한다(`IsCompatibleMesh`). 메시에만 있는 본은 스켈레톤에 **추가**된다(GUID 재생성 → 클립 재압축) — 그 대가를 받아들일지 먼저 정한다
2. 메시 소켓 `weapon_r` — `hand_r` 밑, 오프셋 ~~`(0.19749, 3.411153, −0.381067)`~~ → **`(−9.80, 3.41, −0.38) · yaw 180`**(2026-09-15 규약, 위 표). MCP `SkeletalMeshTools.add_socket` + `set_socket_transform`
   ★ 본 호환 조건(디자인팀 안내용, 2026-09-15): 이름·부모가 정확히 맞아야 하는 본 = root · pelvis · spine_01~05 · neck_01/02 · head · clavicle · upperarm · lowerarm · hand · thigh · calf · foot · ball + 손가락 20개 / `ik_foot_root` · `ik_foot_l/r` 필수(없으면 발 IK 조용히 OFF) · `ik_hand_*` 도 넣을 것 / twist 본은 불필요하되 **다른 이름의 twist 는 최악**(스켈레톤에 추가되고 굳음) / 바인드 포즈·비율은 무관(P76). Blender Auto-Rig Pro 명명(`spine_01_x` · `arm_stretch_l` …)은 통과 못 한다
   ★ 임포트 머티리얼이 전부 `WorldGridMaterial` 로 올 수 있다 — 슬롯부터 확인 (`SkeletalMeshTools.set_material`)
3. 자식 BP: 메시 지정 · 머티리얼 오버라이드 리셋 · `StanceStandZ/CrouchZ` 실측
4. PIE 로 팔 높이(축 규약) · 총 위치 · 왼손 · 웅크림 · **BF 켰을 때 머리 크기**(2.5e-7 — PP ABP 없는 메시에서 드러난다) 확인. **마네킹이 안 바뀌었는지**도 같이

**`StanceStandZ / StanceCrouchZ` 는 클립뿐 아니라 메시에도 종속이다** [A]. 2.5f 의 골반 역산이 `Target = base + StanceStandZ` 를 쓰는데
이 값은 마네킹 골반 높이라, 다른 체형에서는 `PelvOff` 가 0 이 아닌 값에 정착해 매 프레임 골반을 밀어 올리거나 내린다
(아군: +7 → LegIK 가 다리를 최대로 펴고 상체가 숙는 "어정쩡한" 자세). 재는 법은 HUD 역산 — 기립·웅크림 각각에서
**새 값 = 옛 값 − PelvOff**. 자동 산출은 [W27].

**기각한 경로** [A]: `soldier_T_Skeleton` 을 그대로 두고 Compatible Skeletons 로 애니메이션을 공유하는 것.
엔진의 스켈레톤 간 리매핑(`SkeletonRemapping.cpp`)이 회전을 **레스트 포즈 델타**로 보존하므로 A-포즈↔T-포즈 차이만큼
팔이 들린다 → **P76**.

**시도 후 원복** [A]: 총내림 포즈 재저작(`MM_Rifle_LowReady`). 걷기에서 ADS 로코모션과 어긋나 기각. 총내림은 원래대로
`MM_Rifle_Idle_Hipfire_AO_CD` + `BM_LowReady` 마스크다(2.4절). 에셋은 참조 0건으로 보존 → [C-90].
★ **2026-09-15 2차 시험(로컬 공간 애디티브 + `BM_LowReady` + 알파 `AOActive ? 0 : 1`)도 실패·전부 원복.** 결론(사용자 합의):
**이동 중 총내림은 총 내린 로코모션 클립이 없는 한 델타일 수밖에 없고, 델타는 작을 때만 자연스럽다**(P123). 힙파이어 델타(어깨 피치뿐)가
되는 이유. 저작 포즈(팔꿈치·손까지)는 어느 공간이든 걷기 위에서 깨진다. 향후 ① 정지 총내림은 `MM_Rifle_Idle_Hipfire` 직접 편집 + 델타를
Moving 에만 게이트(미실행) ② 디자인팀에 총내림 로코모션 클립 요청(Walk/Jog 루프 8장, 제대로는 32장) → **[W66]**.
`animation/prototypes/2026-09-15_sharp_turn_pop_bf_head_scale_and_weapon_socket.md` 5절

~~**적군 (`Enemy`)** [A]: Mixamo 65본(`Hips` 루트, `root`/IK/트위스트 없음, 겹치는 본 0), 111,083 버텍스 · LOD 1개, 원본 FBX 는 다른 PC.
리스킨 없이는 이 경로로 못 올린다. 경로 결정 → **[Q42]** (권장: 디자인팀에 "UEFN 마네킹 리그 · 마네킹 비율 피팅 · LOD" 스펙으로 요청).~~
→ ✅ **[Q42] A안으로 해결 (2026-09-14~15)** — 디자인팀이 UE5 표준 리그로 재납품(세 번째 `new_enemy_T` 채택, 위 블록). 옛 `Enemy` 는 삭제 후보 [W29].

---

## 4. 자작 C++ [A]

```
Source/SoldierLabEditor/AnimModifiers/
    FootContactCurveModifier      contact_l / contact_r 이진 사각파 생성 (P8b)
    TurnInPlaceCurvesModifier     제자리회전 커브
    SoldierRootFacingModifier     루트 facing 계측용

Source/SoldierLab/                런타임 모듈 — ~~아직 비어 있다~~ **더 이상 비어 있지 않다** [A]
    Math/SoldierAxisLibrary       StepAxis · RampAxisTo — 자세 축용 **등속** 램프
                                  (BlueprintPure. `FInterpTo`/`Lerp`는 지수형이라 목표 근처에서
                                   늘어지고 입력이 멈추면 목표로 붕괴한다 — 자세 축에는 안 맞는다)
                                  → `UpdateBodyYawRate` 가 `RampAxisTo` 를 쓴다 (2.5d)
                                  ⚠ `RampAxisTo` 는 **rate ≤ 0 이면 Target 을 그대로 반환**(램프 없음 = 즉시).
                                  "정지"로 쓰려면 0.0001 같은 극소 양수 (P167, 09-18 스냅 사고)

                                  ★ 2026-09-12 추가 — 골반 높이 역산 2종 (2.5f-5)
                                  SolveBoneHeightOffset(TargetWorldZ, MeasuredWorldZ, AppliedOffset)
                                      = TargetWorldZ - (MeasuredWorldZ - AppliedOffset)
                                  StanceTargetHeight(ActorWorldZ, MeshRelativeZ,
                                                     StandLocalZ, CrouchLocalZ, Stance)
                                      = (ActorWorldZ + MeshRelativeZ)
                                        + Lerp(StandLocalZ, CrouchLocalZ, Stance)
                                  ⚠ **존재 이유**: 블루프린트 팔레트에 산술 노드가 없고(P33),
                                  기존 우회(`Lerp`=곱셈 · `MapRangeClamped`=반파 정류기 ·
                                  `Delta(Rotator)`=회전 전용)로는 **두 runtime float의 뺄셈**을
                                  표현할 수 없다. 기존 클래스에 함수만 추가한 것이라 **P13에 안 걸린다**
    Weapons/SoldierProjectile     ★ 2026-09-12 추가 — 탄도 투사체 (3.1절)
                                  ~560줄(h) / ~1050줄(cpp). titan_example ARCWSProjectile 이식.
                                  Build.cs 에 **Niagara** 추가가 전제다(User. 파라미터 직접 쓰기)

    AI/                           ★ 2026-09-13 추가 — AI 층 9쌍, ~3,670줄 (5절)
        SoldierIdentity           ESoldierFaction · 눈/표적 소켓 · USoldierRegistrySubsystem
        SoldierPerception         ★ FSoldierEnemyRecord · 감쇠 · 역분산 가중 융합 · BroadcastGunshot
        SoldierSight              시야 생산자 (싸구려 기각 + 트레이스 예산)
        SoldierComms              전달 생산자 (말하는 데 걸리는 시간 = 지연 + 속도제한)
        SoldierSuppression        0..1 제압 스칼라. SoldierProjectile이 몬다
        SoldierEngagement         ★ 사격 의도 5종 · 총구 높이 · 재장전 회계
        SoldierCover              시야 판정을 거꾸로 — 노출 + 필요 자세를 한 스윕에서
                                  위치 선택은 세 비용(도착지 노출 + 경로 노출 + 땅의 값)
        SoldierObjective          ★ 땅의 값. 로직 없는 레벨 마커 액터 — 한 개가 양쪽을 섬긴다
                                  ⚠ 레벨 배치 0개 [W25]
        SoldierDebugDraw          네 오버레이 공통 규약 (색=출처 / 크기=해상도 / 굵기=신선도)
                                  Build.cs 에 **AIModule**(SetFocalPoint) ·
                                  **NavigationSystem**(엄폐 후보 네브메시 투영) 추가가 전제다
        SoldierHealth             ★ 2026-09-15 추가 — 체력 · 데미지 수신 · 피격 반응 · 사망 (~400/~780줄)
                                  표준 OnTakePointDamage/OnTakeRadialDamage 바인드(서버만) · 부위 배율
                                  (BoneIsChildOf, 첫 매치) · 본 None 이면 메시 LineTraceComponent ±60 cm ·
                                  bInvincible · HitReact 몽타주(방향 4 × 세기 3, bStopAllMontages=false) ·
                                  Death 몽타주 → 길이−0.1 s 에 래그돌(pelvis 이하, 속도 관성, 다음 틱 임펄스
                                  1500) · 등록부 Unregister · /Script/SoldierLab 컴포넌트 전부 틱 off ·
                                  ABP 축 변수 7종 0 · 8 s 후 시체 틱 정지 · Health/bDead/LastHit 복제.
                                  콘솔 SoldierLab.Debug.Health 1 · SoldierLab.Invincible 1.
                                  ★ **2026-09-17 — 몽타주는 이제 C++ 이 아니라 BP 데이터다** [A].
                                  생성자의 `ConstructorHelpers::FObjectFinder` 19개 + include 를 **삭제**했다
                                  (`/Game/SoldierLab/Animations/Actions/` 하드코딩이 세트 분리로 죽어 배열이 전부
                                  빈 채였고, 같은 로드가 옛 몽타주를 **루트셋에 박아** 폴더 삭제까지 막았다 — P134).
                                  기본값은 `BP_SoldierCharacter`(Enemy_ 19) / `BP_Soldier_Friendly`(ALLY_ 19) 템플릿.
                                  **`bPlayDeathMontage = false`** — 사망은 순수 래그돌(사용자 결정, [C-128]).
                                  사망 몽타주 12장의 슬롯은 `FullBody` → **`DefaultSlot`** 으로 고쳐 유지(P140).
                                  → `ai/2026-09-17_hit_death_three_causes.md`
                                  몽타주 19개는 생성자 FObjectFinder 기본값(BP 에서 덮는다)
                                  → `ai/2026-09-15_health_hit_death_implementation.md`

    Debug/SoldierDebugAxes        라벨 붙은 축 계측 HUD 컴포넌트.
                                  콘솔 `SoldierLab.Debug.Axes 1` / `...Axes.All 1`
                                  기여자는 `SetAxis`만 부른다. 표시가 꺼져 있으면 수집 자체를 건너뛴다

    ★ 2026-09-14 오후~저녁 추가 (titan_example 편입 후) — 5.1절 표 참고
    Observer/SoldierObserverPawn  관전 폰 (ADefaultPawn 자식). 등록부+시야각 픽, F 추적(AI 그대로),
                                  T 1/3인칭(병사 1인칭 컴포넌트에 뷰 위임, CalcCamera 오버라이드), H 그 병사
                                  머리 추종 토글, Tab 다음, 휠 거리. 충돌 없음. 키는 FKey 프로퍼티(BindKey)
    Camera/SoldierFirstPersonComponent
                                  1인칭. 별도 뷰타겟 액터(ASoldierFirstPersonViewTarget, CalcCamera
                                  오버라이드)로 SetViewTarget 교환 — GASP GameplayCamera 는 건드리지 않음.
                                  런타임 IA/IMC 로 T 키. Build.cs 에 **EnhancedInput** 추가가 전제
    Pose/SoldierHeadAimComponent  머리·목 조준 추종 + 눈–조준선 정렬 + 몸 회전(캡슐 yaw, 정지·비조준 시).
                                  닫힌 루프·월드 Additive, ABP 변수 5개에 리플렉션 쓰기. 기본 OFF, H 키.
                                  콘솔 `SoldierLab.Debug.HeadAim 1`
                                  ★ 09-18: H 는 **수동 토글뿐** — AI 자동 활성화(bEnableForAI) 넣었다 되돌림 (P171)

    ★ 2026-09-17~18 추가 (포즈 세션) — AI 세션 계약의 소비 측, 셋 다 AI 전용 (IsPlayerControlled → return)
    Pose/SoldierScanTurnComponent 총 내림 ∧ 정지 ∧ (IsScanning ∨ HasContact) 면 캡슐 yaw 를 GetAimPoint() 로.
                                  Start 20° / Stop 5° 히스테리시스, 180°/s, 메시는 GASP OffsetRootBone + MM TIP.
                                  Engagement 틱 선행. 콘솔 `SoldierLab.Debug.ScanTurn 1`
    Pose/SoldierGaitBridgeComponent
                                  GetDesiredGait()==Walk → CharacterInputState.WantsToWalk (리플렉션, 매 틱).
                                  BP 구조체 필드는 GUID 접미사라 GetAuthoredName/prefix 매칭 (P170). bDriveSprint 기본 off
    Pose/SoldierPoseSmootherComponent
                                  stance/lean/BF-H/BF-V 목표를 사다리꼴 프로파일로(축별 MaxSpeedUp/Down/Accel ×
                                  urgency 스케일 0.5~1.6, 목표 변경 시 속도 연속). BP 상태+AI 목표 변수 동시 쓰기,
                                  액터 틱 선행 (P168). BP 램프 rate(StanceRate/BlindFireRate)를 0.0001 로 얼림 —
                                  0 이면 RampAxisTo 가 목표를 반환해 스냅 (P167). `ext` 감사.
                                  콘솔 `SoldierLab.Debug.PoseSmooth 1`
                                  → animation/2026-09-18_ai_pose_layer_scanturn_gait_smoother.md
```

> ⚠ 위 두 파일은 **2026-09-11 심야에 추가됐는데 문서에 반영되지 않고 있었다**(2026-09-12 소스 실측).
> `SoldierDebugAxes`가 있으므로 6절·[W6]의 "PrintString 3줄" 기술은 **낡았을 수 있다** [B] —
> 계측을 정리할 때 BP 배선을 실물로 다시 볼 것.

> ⚠ 새 `UCLASS`는 Live Coding으로 안 들어간다. 에디터를 닫고 빌드해야 한다 (P13).
> ⚠ 모디파이어에서 루트모션을 읽으려면 `bIncorporateRootMotionIntoPose = true` (P14).

---

## 5. AI 층 [A] (2026-09-13)

> 상세 4부작:
> **`ai/2026-09-13_perception_stack.md`**(인지·시야·무전·제압·디버그 규약) ·
> **`ai/2026-09-13_engagement_and_cover.md`**(교전·엄폐 기하) ·
> **`ai/2026-09-13_objective_and_position_cost.md`**(★ 목표 · 세 비용 위치 선택) ·
> **`ai/2026-09-13_ai_bridge_and_scene.md`**(블루프린트 배선·레벨·도구 함정 9건) ·
> ★ **`ai/2026-09-14_exposure_ladder_and_corrections.md`**(**교정 라운드**) ·
> ★★ **`ai/2026-09-17_situation_field_lighting_model.md`**(**상황 필드 — 땅의 기억은 이쪽이 최신, 09-14 위험 지도는 폐기**) ·
> **`ai/2026-09-17_infiltration_and_unknown_ground.md`**(잠입 거동).
> 이 절은 **무엇이 어디 있는가**만 적는다.

### 5.1 C++ — `Source/SoldierLab/AI/` ~~9쌍~~ **11쌍 + 설정 헤더 1 + ★ 09-21 `SoldierQuery.h` 1 + ★ 09-28 `SoldierFoliageOcclusion` 1쌍** [A] (2026-09-17: `SoldierDangerMap` −1 · `SoldierSituationField` `SoldierLabLog` +2 · `SoldierFieldSettings.h` +1) + `Pose/` ~~4쌍~~ **5쌍**(HeadAim · ★ 09-18 ScanTurn · GaitBridge · PoseSmoother · ★ 09-21 **AIBridge**) + `Observer/` · `Camera/` 1쌍씩 + ★ 09-21 `Weapons/SoldierProjectilePool` 1쌍(투사체 본체는 3.1절) + ★ **09-30 `AI/SoldierAIController` · `Pose/SoldierAnimLibrary` · `Debug/SoldierAimTrace` 1쌍씩**(`Debug/` 에는 기존 `SoldierDebugAxes` 도 있다)

| 파일 | 무엇 | 한 줄 |
|---|---|---|
| `SoldierIdentity` | 진영 · 소켓 · 등록부 · ★ **몸의 자** | `ESoldierFaction{Friendly,Hostile,Neutral}` · 눈 `head` / 표적 **`spine_03`**(정수리만 넘어온 병사는 보이는 게 아니다) · `USoldierRegistrySubsystem`은 **평평한 배열 하나** · `GetFeetLocation()`(P103) ★ **09-15**: **높이를 상수가 아니라 몸에서 잰다**(P115) — `ObserveStance`가 축 양 끝에서 spine_03·head 높이를 실측(가슴 기립 **96~105** / 웅크림 **50~57** / 눈 140~150; 웅크림은 기립×(80/135) 선추정), `GetFightProbeHeightCm = min(눈, 총구 140)`. **총구**: `MuzzleSocket "Muzzle"`(SK_AR4_X, 부착 액터에서 탐색)을 `GetMuzzleLocation`으로 매 틱, `ObservePose`가 포즈 끝(기립/웅크림/린 ±1/블라인드 위·좌·우)에서 몸 기준 오프셋 학습 → `PredictMuzzle*` ★ **09-16~17**: **자리 주장**(`SetClaim/GetClaim` — 가는/선 자리, 나이는 땅이 바뀐 시각부터, S1의 재료) · `GetRegistryIndex`(엄호 타이브레이크) · `GetHeadHeightCm(stance)`(웅크린 `head` 소켓도 실측 — 두 점 엄폐의 재료) · 등록부 `IsRegisteredActor`(**쳐다볼 시체 고르기용**, 지식은 트레이스로만) ★ **09-17 오전**: `GetSocketOrFallback` — 소켓 없는 **비-`ACharacter`**(브리지가 등록한 UGV/트럭)는 `GetActorBounds` Z 비율로 **눈 0.8 · 표적 0.5**(원점+상수는 차체 안/땅 밑에 떨어져 바퀴를 겨눴다) → [C-126] 갱신 |
| `SoldierPerception` | ★ 이 층의 본체 | `FSoldierEnemyRecord`는 **관측된 그대로 + 절대 시각**만 담고, **감쇠는 전부 질의 함수**에 있다(P63). **신선도와 해상도를 합치지 않는다**(P62). 융합은 **역분산 가중**(P65) ★ **09-14**: **추측항법이 만료된다** — `VelocityTrustSeconds 1.5`(총성 기록 0). 속도에 대한 믿음은 위치에 대한 믿음보다 먼저 죽는다 (P88) ★ **09-15**: `LastGunshotTimeSeconds`(총성 시각, 융합은 max — 눈 활동도의 재료) · `FindRecordNear`(익명 기록을 위치로 재발견 — 표적 잠금용) ★ **09-16**: **배운 죽음** — `LearnDeath`(기록 삭제 + `KnownDead`; **눈이 시체를 보거나 무전으로 들었을 때만**, 세계가 대신 부르지 않는다 P130) · `IsKnownDead`(죽은 줄 아는 적의 늦은 무전 무시) · `TakeDeathToReport`(Comms용) · **`ReportClearView`**(부정 증거 — 보고 있는데 없으면 `ObservedCertainty × 0.5^(dt/ClearViewHalfLifeSeconds 1.5)`, P133) ★ **09-17**: `ReportThreatenedBy(Shooter)`(근접탄·피격이 호출 — 기록에 `LastThreatenedTimeSeconds`, **기록이 없으면 안 만든다**) · `GetSecondsSinceThreatened` · `ReportGunshot` → 필드 `ReportSighting(추정점, HeardPresence 0.75, 반경)` — 흐린 라이트(그림자 없음, 글로우만), **0.75는 사전값 0.5보다 커야 한다** ★ **09-21**: **`ReceiveSharedRecord`** 가 `IntegrateRecord` 뒤 **받는 병사의 분대 필드**에 `Field->ReportSighting(OwnIdentity, 위치, 부풀린 반경, 깎인 확신, ObservedTimeSeconds)` — 관측 시각 그대로라 태어나며 낡아 있고, 같은 분대 라이트면 `LastSeenTime = max` 로 자기 눈의 시계를 안 되돌린다. **남의 분대 목격이 우리 필드에 들어오는 유일한 길**(P179) |
| ~~**`SoldierDangerMap`**~~ | ⛔ **삭제 (2026-09-17 `p4 delete`)** | ~~진영별 2 m 격자, 셀당 보였음/공터였음/총알 지나감 세 시각, 30 s 반감, max-only~~ → 출처 없는 누적 버퍼라 "GI처럼 붉은 얼룩"(P147). `SoldierLab.Debug.Danger` cvar · `SoldierCover::DangerHalfLifeSeconds` 함께 소멸. 대체 ↓ |
| **`SoldierSituationField`** | ★★ **상황 필드** (2026-09-17 신규, `UTickableWorldSubsystem`; **09-18 재작성** 2332줄) | 진영별 XY 해시 격자(2.5D, 셀당 지면 Z 캐시 — 내비메시 → 하향 트레이스 → 점 Z), ~~희소 레벨 피라미드(`LevelCount`/`LevelScale`/`DetailRangeCm`, 기본 단일 격자)~~ → **09-18 밉 + 다중 앵커 퇴거**: 레벨 0 `FCell`만 쓰고, 레벨 1+ `FCoarseCell`(경계도 max · lit 평균 · 잔여물)/`FCoarseHorizon`(트임 평균)은 `MarkDirtyUp`으로 더럽혀져 읽을 때 재집계(`RefreshCoarse`, TTL `CoarseRefreshSeconds 1`); 진영 병사 **아무나**에게서 `DetailRadiusCm 8000 × LevelScale^L` 밖 레벨 L은 `Evict`/`EvictHorizons`(스냅샷 2 s, 1024/틱)가 부모에 `FoldInto` 후 해제; 읽기는 `SampleFinest`(레벨 0 → 1 → 2, `FSample` 한 모양); 섀도우는 앵커 80 m 안 라이트만; `SoldierLab.Field.CellSizeCm` cvar. **저장 1**: 경계도 `FCell.Presence`(**0이 아니라 `UnknownPresence 0.5`를 향해** 8 s 반감, 눈만 낮춤). **정적 1**: 호라이즌 `FHorizon`(8방향 × 2높이 uint8 셀 거리, 지연 굽기 8셀/틱 × 16, 진영 공유, 안 다시 굽되 **먼 것은 퇴거**). **파생 1**: 위험도 = **직접**(라이트 `FLight` — 목격, 사람 아님; 0.5 s 연속창 넘기면 **얼고** 다음은 새 라이트, `MaxLights 16`, 흐린 것(`SharpRadiusCm 400` 초과)은 글로우만; 섀도우 48방향 × 2줄, 셀별 광선 높이로 자세 2/1/0(지면 아래 허용 **`GroundSlackCm 40`**, ← −110), 셀당 `FLit{Light,Generation,Stance}` 슬롯 3, **타이머 없음 — 세대로 무효화**, 거리 감쇠 30→90 m; **09-18 부정 증거** `ClearViewSeconds` — 얼린 라이트를 뚫린 선이 지나면 `ContradictLightsAlong`이 시간을 적립, 밝기에 `0.5^(t/ClearViewHalfLifeSeconds 4)` 추가, 다시 보이면 0 [B]) **+ 앰비언트**(호라이즌 × 확산 경계도 8방향 평균, 라이트 제외). API: `ReportSighting` · `MarkClear(AlongRay)` · `GetExposure(ByStance)` · `GetMostExposedDirection` · `FindDarkestCells` · `GetPresence` · `GetCellCount(F, Level)`. 오버레이 `SoldierLab.Debug.Field(.Channel/.Faction/.Level/.RadiusCm)` **월드당 1회** — **09-18 v2**: 자체 `ULineBatchComponent`(`DebugBatcher`) 0.1 s flush+refill · 색 8단계 양자화 색별 `DrawMesh` · `Level −1` 클립맵 링(25 m/100 m/전체 12000) · **불투명도 = 신선도**(크기 0.7 고정) · 링 넘치면 **대칭 축소** + CAPPED(6000, [B]) · 미굽기 파랑끼 [B] · 라이트 불투명도 = 밝기, 초록 봄/호박 들음, 흰 점 = 그림자 미완 [B] · 헤더 2줄 `DescribeGround` [B] · 볼 곳 보라 화살표 2.5 m [B]. `ai/2026-09-17_situation_field_lighting_model.md` **16~18절** ★ **09-18 오후(2491줄, PIE)**: **`GetStaleVantage(Foot, StaleSeconds)`**(앰비언트 적분의 방사체를 "안 본 지 얼마나"로 — 순찰의 재료) · `GetMostExposedDirection`에 **`PreferDir/PreferWeight`(편향)·`ArcYawDeg/ArcHalfWidthDeg`(부채꼴 — 선택만 제한, 적분은 그대로)** · **`FHorizon::bSolid`**(굽기가 기하 안에서 시작 = 전방향 255 + `FindDarkestCells` 제외) · **`GetExposureByStance` → bool**(미지 = false) · `MarkClearAlongRay` **16 m 밖 3셀 폭 띠** · `ContradictLightsAlong` 들은 라이트 제외([W83]) · 라이트 링 annulus 메시 · 화살표에 배정 arc — **20~23절** ★★ **09-21(3036줄)**: **분대 스코프** — `FScope{Faction, SquadId, Levels, Lights, Anchors, EvictionQueue}` × `3 × MaxSquadsPerFaction`, `ScopeFor(Faction, SquadId)`(처음 말하는 순서로 슬롯, 넘치면 슬롯 0 + `WarnedOverflowSquads` 분대당 1회 경고) · `ScopeFor(Who)`(`bTakesSquadOrders=false` → −1) · **공개 API 전부 `Who`**(−1 = 쓰기 무시 / 0 / false / Max) · 호라이즌·`AllAnchors` 는 세계 공유 · 섀도우 예산은 세계 하나(가장 밝은 것, 따라가는 것 우선, 스코프 앵커 80 m 안만) · **`GetWedgePresence(Who, Apex, DirA, DirB, Range, Now)`**(쐐기 안 경계도 × m², 4°/갈래 ≤ 12 · 셀 표본 ≤ 48, 표본 셀의 자기 호라이즌(apex 쪽 bin)·solid 로 가림, 안 구운 셀은 사전값 + 큐, 트레이스 0 — 엣지 전진의 재료) · `GetDebugScope`(관찰 병사의 진영·슬롯 따라감, `Debug.Field.Faction`/`.Squad` 로 반쪽씩 고정) · `Debug.Field.Centre 1`(분대원 전원 주위 링) · `DescribeScope` `HOSTILE/1` · 헤더 3줄째 **비용 줄** `[Field] cost/tick: shadows … (rays/tick, waiting of alive) bake evict | overlay` · ~~**[B] 빌드 전**~~ → **같은 날 늦게 빌드·PIE ✅([W96] 해결, 재측정 0.03 ms · 0 waiting · 96 alive = 기준선 동일)**: `FLight::ShadowEye/ShadowCastTime` · `Refresh` 재캐스트 = 한 셀 ∧ `ShadowRecastSeconds 0.5` · `FreezeLight` 낡은 그림자 즉시 재캐스트 · `CastShadow` 패스 시작점 고정 + **riders**(다른 스코프 반 셀 안 라이트 동승, 셀마다 rider 별 `TouchCell`/`LightCell`) — `ai/2026-09-21_per_squad_field_edge_advance_fire_model.md` 1·2.4·4·10절 ★ **09-21 늦게(3081줄) — 오버레이 노출 보정(P181, PIE ✅)**: `DebugMeshes`(`USoldierDebugMeshComponent`, `GetDebugMeshes()`, `Deinitialize` 해제) 에 **셀 사각형·라이트 링**, 배처에는 선·점·구·화살표만, 둘 다 같은 틱 `Flush` + 채우기; `DrawDebug` 의 `ExposureScale = SoldierDebug::GetExposureScale(World)` + `HDR` 람다(RGB × 배율, 알파 그대로) — 지역 변수 `Exposure` 가 바깥을 가려 C4456(에러) → `ExposureScale` 로 개명. `ai/2026-09-21_debug_overlay_exposure.md` 3.5절 ★ **09-22**: **`ResetForRestart()`** — `Scopes`(셀·라이트·앵커·퇴거큐)뿐 아니라 **지오메트리 캐시(`Horizons`/`CoarseHorizons`)까지** Built* 무효화 → `EnsureLevels` 가 첫 실행과 같은 경로로 재구축(설계 4a 의 "지오메트리 유지"는 폐기, "PIE 와 동일" 기준). 시나리오 재시작 리셋 계약(P187), `SoldierLab.ResetWorld` 에 포함 |
| **`SoldierFieldSettings`** | Project Settings (2026-09-17 신규, `UDeveloperSettings`, `config=Game defaultconfig`; **09-18 갱신**) | **Game → SoldierLab Situation Field.** Resolution **7**(`CellSizeCm` `LevelCount` `LevelScale` **`DetailRadiusCm` `EvictionCellsPerTick` `EvictionIntervalSeconds` `CoarseRefreshSeconds`**, ~~`DetailRangeCm`~~) · Presence 3 · Lights **10**(+`ClearViewHalfLifeSeconds`) · Shadow 10 · Ambient 6 · Debug **7**(`MaxDebugCells` `DebugRefreshSeconds` `DebugDetailRadiusCm` `DebugFill` `DebugAlphaMin` `DebugAlphaMax` `DebugStaleSeconds`, ~~`DebugAlpha` `DebugFillMin/Max`~~) = **43개 값, 전부 미측정** → [C-130]~[C-133] [C-140]~[C-142]. 해상도 바꾸면 필드 비움(`[Field] resolution changed`) ★ **09-21**: Resolution **+`MaxSquadsPerFaction 3`**(1~4, 바꾸면 필드 비움) · Shadow **+`ShadowRecastMoveCm 0`(= 한 셀) · `ShadowRecastSeconds 0.5`**(~~빌드 전 [B]~~ → 같은 날 빌드·PIE ✅) = **46개** → [C-157] [C-161] |
| **`SoldierLabLog`** | `LogSoldierAI` 정의처 (2026-09-17 신규) + ★ **`STATGROUP_SoldierLab`**(09-21) | 위험 지도 헤더에 있던 것을 독립 — 로그만 쓰려고 서브시스템을 include 하던 결합 제거 ★ **09-21**: **`stat SoldierLab`** — `DECLARE_STATS_GROUP(STATGROUP_SoldierLab, STATCAT_Advanced)` + 사이클 스탯 23(시스템 틱 9 · Cover 하위 7 · Field 5 · Squad/Zone 선언만 [W103]) + DWORD 카운터 5(`Traces: Sight/Cover/Engagement/Field` · `Soldiers Ticked`), 전부 `SOLDIERLAB_API`. 각 시스템 `.cpp` 의 `TickComponent` 첫 줄 `SCOPE_CYCLE_COUNTER`, 동기 트레이스마다 `INC_DWORD_STAT`. 짝 = off 스위치 `SoldierLab.<System>.Enabled`(각 `.cpp` 의 `GSoldier*Enabled`). `.h:17-59` · `.cpp:7-34`. P182 |
| **`SoldierFoliageOcclusion`** | ★ **잎 투과율** (2026-09-28 신규, `UTickableWorldSubsystem`, 빌드·등록 ✅) | 수관 타원체(`FSoldierCanopy`) × 2D 격자(CSR, `SoldierLab.Foliage.CellSizeCm` 1000) · `ComputeTransmittance(From, To, Cutoff, OutFractionAtCutoff)` Beer-Lambert, 트레이스 0, 광학 깊이 5 에서 중단 · `RegisterCanopies(SourceId)`/`UnregisterSource` — **채우는 쪽은 titan `UForestCanopyRegistrarSubsystem`**(이 모듈은 나무 출처를 모름) · BP `Get Foliage Transmittance` · cvar `SoldierLab.Foliage.Enabled` · `SoldierLab.Debug.Foliage`. 소비자 = `SoldierSight`(↓) + titan `UTargetDetectionComponent`. 게임 스레드 전용. 위 상단 09-28 노트 |
| `SoldierSight` ★ **09-28 잎** | (아래 행에 더해) | **잎 투과율 T** — T ≥ `FoliageClearTransmittance 0.5` 면 기존 목격, 아래면 어른거림 누적(`FoliageNoticeSeconds 0.6` · `FoliageMinTransmittance 0.05` · `FoliageGlimpseMemorySeconds 1`, 경과 상한 0.25 s) · `MarkClearAlongRay` 는 T < 0.5 지점에서 멈춤 · LearnDeath/`ReportClearView` 는 T ≥ 0.5 · `SweepRays` 는 물리 그대로. 카테고리 `SoldierLab|Sight|Foliage` |
| `SoldierSight` | 시야 생산자 | 싸구려 기각(거리²·콘 dot)은 전부, **트레이스만 예산**(라운드로빈 **5**/틱, ← 3). **"시야 상실"을 보고하지 않는다** ★ **09-16~17**: **두 점** — 상대 **가슴(`spine_03`) → 막히면 머리(`head`)**, 보인 점이 기록 위치(머리만 보이면 머리 조준)(P131) · 남은 예산으로 **시체 확인**(등록부에서 빠진 기억 속 적의 몸을 같은 거리·콘·트레이스로 → `LearnDeath`, P130) · **빈 땅 확인**(기록 예상점이 콘 안·뚫리면 `ReportClearView`, P133) · `SightRangeCm` **12000** (← 6000 — 스폰 63 m·교전 95 m와 불일치로 첫 9 s 무반응) ★ **09-17 저녁**: **콘 스윕** `SweepCone`(표적 트레이스보다 먼저·무조건, `ConeSweepTracesPerTick 2` 별도 예산, `ConeSweepRays 21` 라운드로빈, `ConeSweepRangeCm 4000`) → 필드 `MarkClearAlongRay` — **사전값 0.5를 물리는 유일한 것**(0이면 잠입 OFF) · 목격 시 **비우기 먼저 → `ReportSighting(발, 1.0, SightingRadiusCm 100)`**(P151) · 빈 땅 확인도 필드에 비움 ★ **09-21**: **`FSoldierSweepRay{Eye, Dir, Stop, bBlocked, TimeSeconds}`** 를 슬롯별로 보관(`SweepRays[Slot]`, 각도순 — 이웃은 다른 틱), **`GetSweepRays()`** — 엄폐 층이 **엣지**(옆 광선은 ≥ 2×·+3 m 가는데 ≤ 10 m 에 멈춘 광선)를 읽는 자리, 트레이스 추가 없음(P177). 필드 호출은 전부 `OwnIdentity` 로(분대 스코프) |
| `SoldierComms` | 전달 생산자 | 지연을 지연으로 모델링하지 않았다 — **말하는 데 걸리는 시간**이 낡음과 속도제한을 동시에 만든다. 방송 판정은 **정보량**으로 ★ **09-16**: **사망 보고**(`BeginCasualtyTransmission` — 같은 입·25 m·1.2 s, 새로 배운 사망이 접촉 보고보다 먼저, 사망만 릴레이). 이것이 분대 통신의 전부다 — 자리·의도·명령은 없다 |
| `SoldierSuppression` | 0..1 스칼라 | 인과적·연속적이라는 것만 주장한다. 회복은 **무조건** 돈다 — 사격이 앞지를 뿐 ★ **09-17**: `ApplyNearMiss(거리, Shooter)` → `Perception->ReportThreatenedBy(Shooter)` · **필드에는 안 쓴다**(옛 `MarkFiredUpon` 폐기 — 지나간 탄은 사수 위치의 증거지 이 땅의 증거가 아니다, P144) |
| `SoldierEngagement` | ★ 결정 | 방아쇠는 **"앎이 무기보다 나쁜가"**(P66). 거절 3종은 따로 — `Blocked`(총구에서 트레이스) / `Masked` / 탄약 ★ **09-14**: 막히면 거절하는 대신 **조리개**(Direct/Over/Right/Left)를 찾고 **사격 자세**(Open/Lean/Blind)를 고른다(P81). 사다리를 고르는 것은 **제압도**다. **반동**은 탄창이 줄어드는 것을 보고 센다([W26]). **조준은 240°/s 로 선회**하고 시야 콘도 그 회전을 읽는다(P86) → 새 의도 `Traversing`. `GetDesiredLean` / `GetDesiredBlindFireH/V` 발행([W19] 해결) · `WantsToSprint`(P89) ★ **09-15**: **표적 잠금**(`SelectTarget` — 확신 +0.3 또는 거리 0.6배 이내일 때만 교체; 확인됨 `switches 0`) · **가치 게이트 분리**(`bWorthTheRound` = 총 산포 ≤ 500만, 제압사격은 앎 ≤ `SuppressiveKnowledgeRadiusCm 1000`) · ★ **노출 회계**(`ExposureAccount` 0..1 — 몸이 보이면 활동도×(1+제압)/1.5 s로 오르고 숨으면 2 s로 내림, **< 0.2 내밈 / > 0.8 숨김**) · `PlanAperture`(실제 총구 Direct → Over/Open → Lean R/L → Blind Up/R/L, 너무 위험하면 Open/Lean 미제공; 내미는 동안 자세 재질의 안 함) · **관찰 회차**(낡은 표적 + 제압 < 0.3이면 쏘지 않고 봄) · **방아쇠는 실제 총구 소켓의 사선으로** · `ReadActualPoseAxes`(BP `LeanCurrent`/`BlindFireH`/`BlindFireV` 리플렉션) · 전이 로그 `SoldierLab.Debug.Engagement.Log` (P118) ★ **09-16**: **표적 분담**(`PickCandidate` = 확신 − `TargetCrowdingPenalty 0.15`×아군 수, 교체 문턱도 같은 점수, S3) · **엄호의 사실** `IsCoveringEnemy`(정지 ∧ 사격 의도 ∧ 그 적 잠금 — 의도가 아니라 사실, S4의 재료) · **블라인드 펄스 수정**(`bBlindForCost` — 위험/기하 이유별 종료, 블라인드로는 관찰 안 함, `bWorthShot`에 앎 ≤ 1000, `bBlindNoShot`; P132) ★ **09-17 오전**: **위협 보너스** — 점수 `Certainty − Crowding×Crowd + ThreatenedBonus 0.6 × Threatened`(`Perception->GetSecondsSinceThreatened`, 5 s 기억, 나를 쏘는 놈은 분담 감점 면제) · **사선 거부** `IsLaneDenied()`(접촉 ∧ 믿음 ∧ (Blocked ∨ Aperture None)이 `LaneDeniedSeconds 2` 지속 → 래치, 엄폐가 HERE에 `LaneDeniedCost` 부과 — "15 s 전이 0" 해결) · **가치 히스테리시스** `bWorthTheRound = 산포 ≤ 500 × (bWasFiring ? WorthHysteresis 1.2 : 1)`(30 Hz 방아쇠 떨림 해결) ★ **09-17 저녁**: **스프린트 규칙** `bUrgent = 접촉 ∥ 제압 ∥ 사선 거부`, 엄폐로 갈 때 ∧ 급할 때만; `Cautious` 절대 안 뛰고 `Rush` 늘 뜀(P150) · ~~**무접촉 조준 = 섹터 > `Cover->GetWatchPoint()`(가장 모르는 방향) > 없음**~~ → ★ **09-18 오후 정정: 볼 곳(부채꼴 안에서) > 섹터 중심**(섹터를 고정 방위로 읽자 명령받은 병사의 스캔이 영영 안 돌았다, P163) · 무접촉 분기에서도 **`AimPoint`** 채움 + **`IsScanning()`**(~~볼 곳 있을 때 true~~ → **밤: 섹터 중심이든 볼 곳이든 무접촉 조준점이 있으면 true**, [W89]) 발행 — 포즈 세션 `Pose/SoldierScanTurnComponent`가 읽어 총 내린 idle 의 캡슐을 돌린다(~~연동 [C-152]~~ 포즈 측 PIE ✅) · **`MinStance`** 바닥(`DesiredStance = max(…, Task.MinStance)`, Rush 무시) — `ai/2026-09-18_patrol_scan_and_move_robustness.md` 1·5절 ★ **09-18 밤**: **긴장도 `GetTension()`**(`LastAlarmSeconds` — 알람 = `bUrgent` ∥ 적 기록의 총성 1 s 안, `0.5^(age/TensionHalfLifeSeconds 20)`) · **걸음 `GetDesiredGait()`** `ESoldierGait {Walk, Jog, Sprint}`(Sprint = `bWantsToSprint` 명령 반영 뒤 · Jog = 접촉 ∨ 긴장 ≥ `JogTension 0.3` · **Cautious ∧ 무접촉 = Walk**, P175) — 포즈 세션 `GaitBridge`가 Walk → GASP `WantsToWalk` · **포즈 급박도 `GetPoseUrgency()`** 0..1(`UrgencyIdle 0.15` / `ContactIdle 0.35` / `LookPeek 0.3` / `ShootPeek 0.6` / `Retreat 0.7`(내밈 끝 뒤 `RetreatUrgencySeconds 0.6`) / `Reload 1.0` 의 max + 제압 — "AI 는 목표 + 숫자 하나, 움직임은 포즈 층", P172) — 포즈 세션 `PoseSmoother`가 축 속도 ×0.5~1.6 · `[Engage]` 로그 끝 `tension %.2f gait %d urg %.2f` · **AI 층은 어떤 BP 축도 쓰지 않는다**(`ReadActualPoseAxes`는 읽기뿐) — 같은 문서 13~15절 ★★ **09-21(1712줄) — AI 가 콘을 소유한다(P178)**: **`GetShotSpreadDegrees()`** = `WeaponSpreadDegrees 0.8`(← 3) × `(1 + MovementSpreadScale 6 × min(1.5, v/ReferenceSpeedCms 600))`(← 2) × 자세(`LeanSpreadScale 1.5` ← 1.4 / `BlindSpreadScale 15` ← 4 — 가치 게이트 사거리 조깅 ≈ 40 m · 블라인드 ≈ 24 m 보존) × (1 + `RecoilSpread`) + **`AimSettleDeg`**(`[Accuracy]` — 선회 중 `max(·, AimSettleInitialDeg 2.5)`, 매 틱 `×exp(−dt/AimSettleSeconds 0.4)`, 발마다 `+RecoilKickDeg 0.6`, 이동 바닥 `MoveWobbleDeg 1.5 × v/600`, 상한 30) · **조준 게이트**: 기록 반경 ≤ `TargetRadiusCm 45 × AimedHitTolerance 2`(사격 중 ×1.2) 이면 지금 콘이 들어감 → `Aimed` / 정착 콘(이동 흔들림 포함) 들어감 → **`Settling`**(새 의도, 기다림 — 60 m ≈ 1.5 s · 10 m 0) / 예비 → `Suppressive` · **버스트**(`[Rhythm]`): `WantsToFire` 뒤 `Now < NextBurstSeconds` → **`Pacing`**, 남은 발 0 → `BurstRoundsMin 2..Max 5` 딜(제압 = Max), 발마다 감산, 0 → `NextBurst = Now + BurstPauseSeconds 0.5 × (1 ± RhythmJitter 0.35)`, `FRandomStream Rhythm` = `GetTypeHash(이름)` 시드 · `[Engage]` 꼬리 `cone %.2f wobble %.2f burst %d next %+.2f` · `KnowledgeToSpreadRatio` **삭제** · **엣지 전진 연동**: 무접촉 조준 = `Cover->GetAdvanceView`(쐐기 이등분 8 m + 벽 쪽 `DesiredLean`) > 볼 곳 > 섹터 · **`bWantsToAim = 접촉 ∥ IsAdvancing()`**(BP 는 아직 `HasContact()` [W93]) · 전진 ∧ 무접촉 = Walk(스프린트 해제) · 급박도 `UrgencyLookPeek`. ⚠ **무기 BP 가 `GetShotSpreadDegrees()` 를 읽기 전에는 탄이 옛 고정 콘** [W93] — `ai/2026-09-21_per_squad_field_edge_advance_fire_model.md` 3절 ★★ **09-30**: 선회 등속 240°/s → **`SlewAim()` 오차 비례**(오차 ≤ `AimSlewNearErrorDeg 3` 이면 `AimSlewNearDegreesPerSecond 40`, ≥ `AimSlewFarErrorDeg 30` 이면 `AimSlewDegreesPerSecond` **150**, 사이 선형 — 교전 · 수색 방향 두 호출부, `.cpp:469-479` · `:1189` · `:1661`, [C-93] 해결) · `GetAimRotation()` 게터(→ `ASoldierAIController` 가 컨트롤 회전으로 씀, P198) · 녹화기 `friend` · **`SoldierLab.Engagement.BlindFire`**(기본 **0**, `.cpp:33`) — `PlanAperture` 의 Blind 후보 3종(위·좌·우)을 1 일 때만 제시, 0 이면 Lean·일어서 쏘기만(막히면 엄폐 뒤에 머묾), 코드 경로 유지 — 맹목사격 **임시 비활성** [Q52]. `ai/2026-09-30_ai_real_pitch_and_aim_smoothing.md` 3절 · `animation/2026-09-30_diagonal_aim_stop_selection.md` 6절 |
| `SoldierCover` | 엄폐 + 위치 선택 | **시야 판정을 거꾸로 돌린 것.** 한 스윕이 **노출 + 필요 자세** 두 답을 준다. 자리는 **세 비용 한 통화**로 고른다 — `Exposure + RouteRiskWeight×RouteRisk + ObjectiveWeight×ObjectiveCost` ★ **09-14**: 첫 항이 노출에서 **`FightingCost`** 로 바뀌었다(P82) — 한 스윙이 이제 **세 답**을 준다(노출 · 필요 자세 · **싸울 수 있는가**). 경로 위험은 **거리로 스케일**(P83) · 후보는 **두 겹 링** · 위협 추정은 **한 바퀴 동안 얼린다** · **정지 감지**(P87)와 **RVO 회피**도 여기 ★ **09-14~15**: 위협 = **기억 속 적 전원**(가까운 순 ≤3, 스무딩 1 s, **활동도** 가중 — 최근 3 s 총성 1.0 → 0.3, P117) · 후보 = **부채꼴 그림자**(눈마다 24줄×2높이, 반경 24 m; 링은 예비) · 비용 항별 `FSoldierPositionCost`(Fighting/Route/Objective/Danger/Suppression) · 경로 위험 **길이 비례, 상한 없음**(P84) · 위험 지도 항(자리는 눈 0일 때만, P116) · **머무름**(엄폐 있을 때 3 s, 여유 1.0)·이동 중 스윕 폐기 · "쏠 수 있나" = min(눈, 총구) + **린 좌·우 2점**(코너 = 사격 위치) · 예산 하한 후보 1개(P114) · 오버레이 2줄 + `SoldierLab.Debug.Cover.Log` ★ **09-16~17**: 비용에 **`q`(Squad) 항** = `SquadCost` — **S1 자리 주장**(아군 주장 2.5 m 안 후보 +1.0, 현재 자리는 상대가 먼저일 때만 → 겹친 둘 중 한 명만 비킴) + **S2 사선 비우기**(아군 총구→표적 선분 1.5 m 안 +0.6) · **S4 엄호 이동** `IsEnemyCoveredByTeammate`(아군이 사실로 엄호하는 적의 눈 활동도 ×0.5; 엄호자는 내 자리보다 나쁘지 않은 쪽만, 동률은 등록부 앞) · **두 점 엄폐** — 자세 3단마다 **가슴+머리 둘 다** 모든 눈에서 가려져야 숨음(`GetHeadHeightCm`, 예산 `(3+2×heights+routes)×eyes`, P131) ★ **09-17**: `Field` 포인터(상황 필드) — Danger 항 = `DangerWeight 0.8 × Field->GetExposure`(**눈 0일 때만**) · 경로 표본 = `max(live, Field->GetExposure × 활동도)` · **부채꼴·몸 밴드·경로 광선은 필드에 안 쓴다**(P144) · HERE에 `LaneDeniedCost 1.0`(오전) · **골든앵글** `CandidateRotationRad += 2.39996 rad`/스윕(링 각도 + 부채꼴 지터 — 정지 병사가 같은 20점만 긋던 것) · **[B] 빌드 전**: `AddFieldCandidates`(`FindDarkestCells` → `FieldCandidateCount 6`, 부채꼴 다음·링 앞) · `EvaluatePosition` **눈 0 분기가 필드를 읽어** `HiddenThreshold 0.25`로 은폐/낮은 벽/공터 세 사실 산출(옛 `CanHide=true` 조기 탈출 제거) · `BeginSweep` → `WatchPoint`(`GetMostExposedDirection`) · `IsRushing()`(배정 Rush)이면 셋 다 우회 · `DangerHalfLifeSeconds` **삭제** ★ **09-18 오후(PIE ✅)**: **`PatrolCost`**(`GetStaleVantage` — Hold/수비 반경 안 `+ PatrolWeight × (1 − vantage)`, 눈 0일 때만) · `TaskCost` Hold **밴드 밖 기울기 계속**(P158) · `BeginSweep` 볼 곳을 **눈 유무 무관 항상**(P165) + 편향(이동 `WatchTravelBias 1` / 정지 Approach `WatchApproachBias 0.5`) + 정지 시 섹터 **arc** · `EvaluatePosition` 눈 0 분기 **미지 = 노출 1·은폐 불가**(`GetExposureByStance` false, P159) · `FinishSweep` **`MoveGraceSeconds 0.75`** 유예(P162) + `MoveTo` 전 **`FindPathSync` 부분 경로 불허**(P161) + **`RejectCandidate`/`IsRejected` 30 s**(`REJECT` 로그) + 무진전 거부 + **`ScanDwellSeconds 2`**(눈 0 도착 머무름) · `[Cover] watch=` ★ **09-18 저녁 ~~[B] 빌드 전~~ → 밤 PIE ✅ "잘됨"**: **A** 은폐 판정 = 자세마다 모든 눈, 보는 눈 활동도 비율 ≤ `HiddenGazeFraction 0.5`(P166) · **B** `AddMicroCandidates`(정지 ∧ 눈 있음, 8 × `MicroStepCm 30`, 병합 우회) + `CoverAcceptanceRadiusCm 20`·`bStopOnOverlap=false` + 무진전 "같은 자리" 15 cm · **C** `CurrentPath` 보관 → `FindNextCorner`(`CornerAngleDeg 35`, `CornerLookAheadCm 500`) → `UpdateCornerPause`(`CornerStopCm 150` 앞 `PauseMove` `CornerPauseSeconds 0.8`, 눈 0 ∧ Rush 아님, 굽이당 1회) + 볼 곳 편향 = 굽이 너머 — **진짜 파이 자르기 아님** ★ **09-18 밤**: 첫 빌드 로그의 **코너 멈춤 루프**(0.82 s 마다 `MOVE`) → 굽이 기억을 인덱스에서 **자리**(`LastPausedCornerLocation`, `CornerStopCm` 반경, P173)로 · 재개 시 **`LastMoveIssuedSeconds = Now`**(재개된 이동 = 방금 발행한 이동, P174) · **눈이 나타나면 즉시 재개**(`SweepEyes.Num() > 0`) · `RejectCandidate` 만료 `RemoveAll`([W90]). `ai/2026-09-18_patrol_scan_and_move_robustness.md` 7 · 12절 ★★ **09-21(2374줄) — 엣지 전진이 코너 멈춤을 대체**(P176·P177, 빌드됨 CL 500 · **PIE [W95]**): ~~`UpdateCornerPause`·`IsPausedAtCorner`·`CornerStopCm`·`CornerPauseSeconds`~~ **삭제**, `FindNextCorner`·`CornerAngleDeg 35`·`CornerLookAheadCm 500` 은 **볼 곳 미리 보기용으로만**. `[Advance]`: `FindViewEdges`(스윕 광선 이웃 쌍(0.75 s 안)에서 막힌 ≤ `EdgeRangeCm 1000` 광선 옆이 ≥ max(2×, +3 m) 가면 엣지, 진행 쪽만, 숨은 쪽 10° 탐침 쐐기가 `EdgeIgnorePresence 0.5` 미만이면 그냥 벽) → `UpdateEdgeAdvance`(매 `AdvanceCheckSeconds 0.1`; 시작 조건 눈 0 ∧ Rush 아님 ∧ 엄폐 이동 중 ∧ `Moving` → `StopMovement` + `bAdvancing`; 걸음 뒤 발이 멈추면 `AdvanceRestSeconds` 찍고 **`GlowInWedges` ≤ `AnalyzedPresence 1` ∥ ≥ `MaxLookSeconds 3`** 이면 다음; 엣지 없거나 걸음 못 내면 `EndAdvance` = 원 목적지 재발행) → `IssueAdvanceStep`(목표 방향 `{0, ±35, ±70, ±105}°` × `AdvanceStepCm 60` × 1..`AdvanceStepCount 4`, 내비 투영, 뒤로 제외, `OpenedByStep` = 엣지마다 눈-apex 선의 선회가 숨은 쪽이면 `GetWedgePresence(apex, Before, After, WedgeRangeCm 1500)`; 예산 `StepPresenceBudget 6`(Cautious ×0.5) 안 `진전 − 0.5 × 걸음 × 열림/예산` 최대, 없으면 1걸음 중 최소 열림) · 접촉 → `BreakAdvanceForContact`(그 자리 정지, 경로 리셋, 걸어온 자리(≤ 8)를 `AdvanceRetreatSeconds 10` 동안 후보에) · `IsAdvanceLooking()` 은 `bAlreadyGoing`(재결정 안 함), 새 엄폐 이동은 전진 취소 · `GetAdvanceView`/`IsAdvancing` (교전이 소비) · 로그 `[Cover] … ADVANCE begins/step/ends` · 오버레이 시안 핀(엣지) + 시안 두 줄(마지막 쐐기, 밝음 = 글로우 남음) — `ai/2026-09-21_per_squad_field_edge_advance_fire_model.md` 2절 · 값 [C-158] ★ **09-21 성능(2491줄, PIE ✅ 실측)** — `TickComponent` 재구성(P183): `SoldierLab.Cover.Enabled`/`.Avoidance` cvar(`:27-38`, 회피는 `bAvoidanceApplied` 래치로 다음 틱 적용 `:2181-2189`) · **`bSweepSuspended`**(`:2229-2244` — `Moving ∧ (유예 안 ∨ 속도 > StallSpeedCms)` ∨ `IsAdvanceLooking()` 이면 후보 루프 통째 건너뜀, 풀리면 `bSweepActive=false` → 새 스윕) · **`UpdateWatchPoint()`**(`:1887-1975`, `BeginSweep` 에서 떼어냄 — 정지 중 `WatchRefreshSeconds 0.25` 마다 `:2287-2294`) · HERE 는 **`HereEvalIntervalSeconds 0.1` ∨ 걸음 > `MicroStepCm` ∨ 새 스윕**(`LastHereEvalSeconds/Foot`, `:2257-2285` — 정지 여부 무관) · 후보 루프 **`CalmCandidatesPerTick 2`**(눈 0, `:2311-2316`) · **경로 가지치기**(`ScorePosition(…, 0)` 이 best 에 지면 `EvaluateRoute` 생략 `:2345-2381`) · 하위 스코프 7. `Category "SoldierLab|Cover|Cost"` `.h:548-563`. 8.92 → **1.92 ms**, 487 → **243** 트레이스. 값 [C-162] · `ai/2026-09-21_perf_instrumentation_and_cover_cost.md` 4절 |
| `SoldierObjective` | ★ 땅의 값 (2026-09-13 추가) | **로직 없는 레벨 마커 액터.** 한 개가 **쥔 쪽에겐 수비 · 나머지 전부에겐 공격**이 된다. 수비는 **반경 안 평평한 0**(거리로 매기면 수비대가 중심점으로 붕괴한다 — P73), 공격은 **평지 없는 비례**. ⚠ **레벨에 아직 0개** → **[W25]** ⚠ **09-14 정정**: 루트 컴포넌트가 없어 **여태까지 모든 거리를 (0,0,0) 에서 재고 있었다**(P90). 공격 비용의 **clamp 제거**(P84) · 반경 900→1500 · 밴드 900→1200 · 환율 6000→3000(P85) ★ **09-18 오후**: 수비 비용이 **밴드 밖에서도 `ApproachScaleCm` 비율로 계속 오름**(P158 — 1.0에서 멈춰 집으로 갈 기울기가 없었다) · **`PatrolWeight 1.0` · `PatrolStaleSeconds 30`** — 명령 없는 레벨(`L_SoldierTest`)의 수비대도 순찰(`SoldierCover::TaskCost`가 수비 반경 안에서 `PatrolCost` 가산) |
| `SoldierDebugDraw` | 오버레이 공통 규약 + ★ **노출 보정**(09-21) | 색=출처 · 회색=센서 활동 · 크기=해상도 · 굵기=신선도 (P70) ★ **09-17**: `GetObservedSoldier(World)` — 진영 단위 오버레이(필드)가 "누구 편"을 답하는 한 곳: 필터 이름 → 관전 폰 `GetFollowedSoldier()` → 조종/뷰타겟 병사(P146) ★ **09-21**: 조종 폰/뷰타겟은 **`bTakesSquadOrders` 인 Identity 만** 병사로(UGV 의 표적 Identity 제외 — UGV 조종 중 오버레이가 UGV 를 따라가던 원인, P179) ★★ **09-21 늦게 — 노출 보정(P181, PIE ✅)**: EV10 고정 레벨에서 오버레이 전부가 숯검정(디버그 프리미티브는 톤매퍼 **앞**, `DrawDebug*` 는 8-bit `FColor`). **`GetExposureScale(World)`** — `FSoldierDebugExposureExtension : FSceneViewExtensionBase` 가 `SetupView` 에서 게임 뷰의 `GetLastEyeAdaptationExposure()` 를 저장(게임 스레드, 렌더 없음, 첫 호출에 등록·프로세스 수명), 반환 `clamp(1/exposure, 1e-3, 1e6)`(첫 프레임 1) · **`Bright(World, FColor) → FLinearColor`**(sRGB → 선형 × 배율, **알파 제외**) · 래퍼 **`Line(…, LifeTime)`**(> 0 이면 영속 배처) **`Point` `Sphere`**(대원 3) **`Circle`**(`YAxis/ZAxis`) — 월드 라인 배처, 한 프레임 · cvar **`SoldierLab.Debug.ExposureScale 0`**(0 자동 · 아니면 수동 배율, 블룸 레벨은 조금 낮게). `AI/` 의 `DrawDebugLine/Point/Sphere/Circle` **24곳 전부** 래퍼로(Cover 11 · Objective 3 · Sight 3 · Engagement 2 · Perception 2 · Suppression 2 · Comms 1), `DrawDebugString` 은 그대로. **`Arrow` 래퍼 없음**. `Squad/`·`Pose/`·`Weapons/` 17곳은 [W97]. `ai/2026-09-21_debug_overlay_exposure.md` |
| **`SoldierDebugMesh`** | ★ **선형 색 디버그 메시** (2026-09-21 신규, PIE ✅) | `USoldierDebugMeshComponent : UPrimitiveComponent` — 라인 배처의 메시 경로(`DrawMesh`, `FColor` 저장)를 **`FLinearColor` 로** 재구현: `FMesh{Verts(월드), Indices, Colour, DepthPriority}` · `DrawMesh(Verts, int32 Indices, FLinearColor, DPG)` · `Flush()` · 자체 `FSoldierDebugMeshSceneProxy`(메시 배열 복사, `FDynamicMeshBuilder`, **같은 `GEngine->DebugMeshMaterial`** 에 `FColoredMaterialRenderProxy(Parent, Mesh.Colour)` 한 프레임 프록시 — 배처와 색 타입만 다름, 반투명 관련성, `bWillEverBeLit=false`) · 충돌·틱·스트리밍 없음. 소비자 = 필드 오버레이의 **셀 사각형(색 그룹당 1)·라이트 링**(`USoldierSituationFieldSubsystem::DebugMeshes/GetDebugMeshes`, 배처와 같은 틱 `Flush` + 채우기). `Build.cs` **`RenderCore` · `RHI`** 가 전제. `ai/2026-09-21_debug_overlay_exposure.md` 3.4절 |
| **`Observer/SoldierObserverPawn`** | ★ 관전 카메라 (2026-09-14 오후 신규) | `ADefaultPawn` 자식. **F** 조준선 아래 병사 추적/해제(AI 는 그대로 — `Possess` 아님) · **T** 1/3인칭(~~V~~ → 병사 조작과 통일) · **Tab** 다음 병사 · **휠** 거리. 픽은 트레이스가 아니라 **등록부+시야각**(`PickConeDegrees` **15**, 병사가 Visibility 를 무시하므로). 키는 `FKey` UPROPERTY + `BindKey`, IA/ini 불필요. 충돌 없음. ✅ **2026-09-15 확인**: 1인칭일 때 따라다니는 병사의 `USoldierFirstPersonComponent` 에 뷰를 **위임** — `BeginExternalView()/EndExternalView()`(소켓→시선 캘리브레이션, `bExternalView` 면 알파 무관 소켓 완전 추종), 관전 폰 `CalcCamera` 오버라이드가 `ComputeView()` 호출(빙의 없음). 옵션 `bUseSoldierFirstPersonComponent`(기본 true; 없으면 옛 head 소켓 간이 뷰). **H**(`HeadAimToggleKey`) = 그 병사의 `USoldierHeadAimComponent::ToggleHeadAim()`(AI 는 `bApplyToAI` 필요). HUD 상태줄 `headaim:ON/off` · `/ soldier eyes`. Tab·3인칭·EndPlay 에서 외부 뷰 해제. ★ **09-18**: 자유 비행 시 **휠 = 비행 속도**(`FlySpeedCms 1200`, ×/÷`FlySpeedWheelFactor 1.25`, 100~20000, `ApplyFlySpeed()` 가 FloatingPawnMovement MaxSpeed·Accel·Decel 비례) · **`bIgnoreTimeDilation`**(`CustomTimeDilation = 1/배속`, slomo 무관) · **롤 잔류 수정**(외부 뷰 동기화는 yaw·pitch 만, 전환 시 롤 0 — P169). `ai/2026-09-14_cover_frame_fix_and_observer.md` 2절·**5절** · `animation/2026-09-14_sight_alignment_plan.md` 0' 절 |
| **`Camera/SoldierFirstPersonComponent`** | ★ 1인칭 (2026-09-14 오후 신규) | 뷰타겟 교환: `ASoldierFirstPersonViewTarget`(`CalcCamera` 오버라이드)로 `SetViewTarget`, 돌아올 땐 캐릭터. **GASP `GameplayCamera` 는 건드리지 않는다**(Deactivate/재활성화는 망가짐 — P107). `Anchor` Body/Weapon · `CameraSocket`(사용자 `eyes` 소켓) · `LocationOffset`(폰/소켓 공간) · `RotationOffset` · `RotationMode` ControlRotation/**FollowSocket** · `bAlignToAimOnEnter`(머리 실제 시선 기준) · `bFollowSocketOnlyWhileHeadAims` · `bHideBodyFromOwner` = `PC->HiddenPrimitiveComponents` · **`NearClipPlaneCm 2`**(1인칭 뷰에만 `FMinimalViewInfo::PerspectiveNearClipPlane` — 전역 10 cm 로는 총이 잘린다). 런타임 IA/IMC, 키 **T**. 관전 폰이 빌려 쓰는 `BeginExternalView/EndExternalView`(2026-09-15 확인). `ai/2026-09-14_cover_frame_fix_and_observer.md` 3c절 |
| **`Pose/SoldierHeadAimComponent`** | ★ 머리·목 조준 추종 + 눈–조준선 정렬 (2026-09-14, 10차까지 사용자 확인 "완벽") | **닫힌 루프·월드 Additive**(총구 되먹임 2.5c 와 같은 꼴). **몸 프레임 = `TorsoBone`(spine_03) 실제 회전 × 레퍼런스 상대회전**(P111) — 보정·굽힘·스트레치·학습된 조준선 전부 이 프레임에 저장했다가 매 틱 월드 변환(P110). 방향: 지난 프레임 머리 실제 시선 vs 목표의 **swing** 오차를 `CorrectionGain 8`/s 누적, `ToSwingTwist` 로 시선축 twist 제거(P106), 몸 기준 yaw±75/pitch±55 클램프 후 **`Body.Pitch+Δ, Body.Yaw+Δ` 로 재조립**(P109), 안티와인드업. **2단**: 둘러보기(켜지면 항상, `LookAroundStrength 1`) / 정렬 = **weld 가중치**(조준경 +X ↔ 컨트롤 회전 각도 smoothstep `AlignZeroDegrees 35`→0 · `AlignFullDegrees 3`→1, `WeldBlendRate 12`/s, 1 에 닿으면 **래치** `UnweldDelaySeconds 0.5`, P113) — `AOActive` ∧ 학습된 선 필요. 조준선 학습: 정지 게이트(≤30°/s ∧ ≤15cm/s, 0.1s) ∧ `AimAlignmentDegrees 8` 안에서 스냅샷, 학습값 대비 `TrackSmallMovesCm/Deg 3` 이내면 `LineTrackRate 12`/s 연속 추적, 큰 점프는 홀드. 눈 목표 = 선 위 최근접점(`EyeReliefMinCm 5~MaxCm 20`)을 `자연 눈 + w×(선−자연 눈)` 로 블렌드. **목 굽힘**(neck_01/neck_02 `NeckBendSplit 0.5`, `MaxNeckBendDegrees 60`) + **스트레치**(목이 못 하는 성분만 머리 본 이동, `MaxNeckStretchCm 5`) — 유효 레버가 목 길이뿐인 기하 한계(P112). **맹목사격**(`BF_AlphaL/R/U` > 0.05) 이면 전체 off. 조준은 `Controller->GetControlRotation()`(P104), AI 는 접촉 시 `Engagement->GetAimPoint()`. **몸 회전**(2026-09-15, `SoldierLab\|HeadAim\|BodyTurn`): 켜짐 ∧ 비조준 ∧ 정지 ∧ 플레이어일 때 카메라 yaw 가 캡슐에서 `BodyTurnStartDegrees 60` 밖이면 **캡슐 yaw 만** `BodyTurnRateDegPerSec 360` 으로 돌리고 `BodyTurnStopDegrees 10` 안에서 멈춤 — 메시·turn-in-place 는 GASP OffsetRootBone + MM 이 조준 모드와 같은 경로로(P119; Strafe 우회는 항상 견착이라 폐기 P120). **기본 OFF**, 키 **H**, `bApplyToAI`. 전부 `EditAnywhere`(`SoldierLab\|HeadAim\|…`). 콘솔 `SoldierLab.Debug.HeadAim 1`. 최종 설계 **`animation/2026-09-14_sight_alignment_plan.md` 0' 절**. ★ **09-18 확정: H 는 수동 토글뿐, AI 에 자동으로 켜는 로직 없음**(09-17 `bEnableForAI`/`bTurnBodyForAI` 넣었다 되돌림, P171 — AI 몸 회전은 아래 ScanTurn) |
| **`Pose/SoldierScanTurnComponent`** | ★ AI 몸 회전 (2026-09-17~18, ✅ PIE) | **AI 전용**. `!WantsToAim()` ∧ (`IsScanning()` ∨ `HasContact()`, `bOnlyWhileScanningOrInContact`) ∧ 정지(지면·가속 0·`MaxSpeedCms 10`) 일 때 캡슐 yaw 를 `GetAimPoint()` 방위로 — `StartDegrees 20` 넘으면 시작, `StopDegrees 5` 안이면 정지, `TurnRateDegPerSec 180`(교전 조준 선회 240 보다 낮게, 몸이 눈을 따라감). 메시는 GASP OffsetRootBone + MM TIP(P119 의 AI 판). 견착이면 GASP aim 모드 몫, 이동 중이면 CMC OrientToMovement 몫. Engagement 틱 선행. 콘솔 `SoldierLab.Debug.ScanTurn 1`. 틈: 섹터만 있고 굽기 전 `IsScanning()==false` [W89]. `animation/2026-09-18_ai_pose_layer_scanturn_gait_smoother.md` 1절 |
| **`Pose/SoldierGaitBridgeComponent`** | ★ Walk → GASP (2026-09-17~18, ✅ PIE) | **AI 전용**. 매 틱 `CharacterInputState.WantsToWalk = (GetDesiredGait()==Walk)` 를 리플렉션으로(BP 구조체 필드 GUID 접미사 → `GetAuthoredName()`/prefix, P170). GASP gait = Sprint > **Walk** > Run 인데 Walk 만 길이 없었다. `bDriveSprint` 기본 **false**(Sprint 는 BP 브리지 소유). 같은 문서 2절 |
| **`Pose/SoldierPoseSmootherComponent`** | ★ 자세 축 사다리꼴 (2026-09-17~18, ✅ PIE "해결완료") | **AI 전용**, `AIPoseDriven` 게이트. stance/lean/BF-H/BF-V 각각 `FSoldierPoseAxisMotion{MaxSpeedUp, MaxSpeedDown, Acceleration}` — **Stance 1.6/0.9/5**(앉는 쪽 = Up 이 빠름) · 나머지 1.2/1.2/4 — × `lerp(ScaleAtCalm 0.5, ScaleAtUrgent 1.6, GetPoseUrgency())`. `Wanted = sign(d)·min(MaxSpd, √(2·a·|d|))`, `FInterpConstantTo(Vel, Wanted, a)` → 목표가 바뀌어도 속도 연속, 스냅 eps 0.002/0.02, 범위 벽. **BP 상태+AI 목표 변수를 같은 값으로 씀**(`StanceAxis`+`AITargetStance` …) + Engagement 뒤·**액터 틱 앞** 선행(P168). BP 램프 rate `RateVariablesToFreeze={StanceRate, BlindFireRate}` 를 첫 틱에 **`FrozenRate 0.0001`** 로(린 램프는 그래프 리터럴 핀 → BP 에서 0.0001). ⚠ 처음 0 으로 얼렸다가 **급할 때 1프레임 스냅** — `RampAxisTo` 가 rate ≤ 0 이면 Target 반환(P167). `ext`(지난 쓰기 − 이번 읽기) 감사. 콘솔 `SoldierLab.Debug.PoseSmooth 1`. 값 전부 [C-154]. 남은 이산 전환 = GASP 크라우치 DB(`StanceThreshold 0.5`) [W91]. 같은 문서 3절 ★ **09-30**: `StepAxis` 에 **"이번 프레임에 목표에 닿는 속도"(`|d|/dt`) 상한 + 한 스텝에 넘으면 목표에 착지·정지** — 목표 0.5 = 임계값 0.5 에서 `√(2ad)` 제동이 이산 프레임에서 목표를 넘나드는 **리밋 사이클**(0.4995 ↔ 0.5006)로 웅크림이 매 프레임 토글되던 것(P195). 자세·린·BF 축 전부. `ai/2026-09-30_ai_real_pitch_and_aim_smoothing.md` 6.1 · 7절 |
| **`Pose/SoldierAIBridgeComponent`** | ★ **캐릭터 BP EventTick 본문의 C++ 판** (2026-09-21 후편, ✅ PIE — [W108]) | **`TickBridge(DeltaSeconds)`** = `BP_SoldierCharacter` EventTick 본문(조준 보정 회전 수학(2.5c) → 린 램프 → 입력 상태 구조체 Sprint/Aim 직접 쓰기(`UpdateInputState_Server` 본문 = `SetCharacterInputState` 라 권한에선 동치) → `AOActive`/`AIPoseDriven`/AI 다리 5개 → `UpdateBodyYawRate`/`UpdateBlindFire`/`UpdateStance` → `WantsToFire`/`Reload` → Rifle `Shoot`/`StartReload` 이벤트)을 **노드 순서 · Kismet 산술(RLerp/ComposeRotators/NormalizedDeltaRotator/MapRangeClamped/InRange) 그대로** 이식. **변수 47개(캐릭터 34 · 부모 4 · ABP 7)는 BP 소유 그대로 `FProperty` 로 읽고 씀** — 스무더·게이트브리지·리플리케이션 분기가 같은 변수를 쓰므로 소유권 불변(P170 과 같은 리플렉션 경로). BeginPlay 바인딩 실패 시 경고 + BP 원본 경로(`ShouldRunBlueprintCopy()`), cvar **`SoldierLab.AIBridge.Native`**(0 = BP 옛 본문, A/B). AI·플레이어 공통(본문이 그랬다). 아군 20 ReceiveTick 1.16~1.21 → **0.22 ms**. `AC_PreCMCTick`(GASP)은 미이관 → [W115]. `ai/2026-09-21_game_thread_structural_pool_rays_bridge.md` E절 ★★ **09-30 (조준 떨림)**: 조준 보정 **재설계**(2.5c 추기 — 총이 조준에 있을 때만 `1−exp(−10·dt)` 적분 · 밖에선 6/s 누설 · 데드밴드 0.3° · 게이트 120°/s · 축당 60°/s 상한, cvar `SoldierLab.Aim.Correction*`, P196) · `UpdateStance` **슈미트 트리거**(`SoldierLab.Pose.CrouchHysteresis 0.05`, 플레이어 V/B 포함, P195) · AI `WantsToAim` = 접촉 ∧ 지면 속도 밴드(400 초과 해제 / 340 미만 재개, `SoldierLab.Aim.JogAimOffSpeed`/`OnSpeed`) + 전환 최소 유지 `JogAimMinHoldSeconds 0.8`(헤더 멤버 `LastAimToggleSeconds` — 에디터 재빌드) · AI 몸통 회전 상한 `SoldierLab.Aim.AIMaxBodyYawRate 360`(플레이어 무관). BP 옛 본문 경로(`AIBridge.Native 0`)의 옛 판정은 그대로. `ai/2026-09-30_ai_real_pitch_and_aim_smoothing.md` 7 · 10 · 11절 |
| **`AI/SoldierAIController`** | ★ **AI 컨트롤러 — 진짜 피치** (2026-09-30 신규, ✅ PIE) | `ASoldierAIController : AAIController` — `UpdateControlRotation` 오버라이드: 초점이 유효하고 교전이 조준한 적이 있으면 **컨트롤 회전 = `USoldierEngagementComponent::GetAimRotation()`**(선회 후, 피치 포함, roll 0), 아니면 엔진 원래 동작. 엔진은 초점이 폰이 아니면(`SetFocalPoint`) **피치를 0 으로** 둔다(P198). cvar `SoldierLab.Aim.RealPitch`(1, 0 = 엔진 규칙). **`AIC_Soldier` BP 부모 = 이 클래스**(옛 부모 `AIController` C++ — GASP `AIC_NPC_SmartObject` 복제본이라 잃은 것 없음, `bStartLogicAutomatically = false` 유지, `BP_Soldier_Friendly/Hostile` 둘 다 `AIC_Soldier_C`). 2-PC 원격 피치(`RemoteViewPitch`) 미확인 [C-178]. `ai/2026-09-30_ai_real_pitch_and_aim_smoothing.md` 2 · 3 · 4절 |
| **`Pose/SoldierAnimLibrary`** | ★ **ABP 용 BlueprintThreadSafe 함수** (2026-09-30 신규, ✅ PIE) | `USoldierAnimLibrary::UpdateStanceAimOffset(UpdateContext, Node, AllyStand, AllyCrouch, EnemyStand, EnemyCrouch, BlendTime=0.25)` — 애님 인스턴스의 `Stance`(1 = 웅크림)·`UseAllyAnimSet` 으로 AO 4개 중 하나를 고르고, 현재와 다르면 `UBlendSpacePlayerLibrary::SetBlendSpaceWithInertialBlending`(아래쪽 `DeadBlending_0` 이 흘려보냄), 첫 업데이트는 즉시. ABP 함수 **`OnUpdate_StanceAimOffset`**(`Context`·`Node` by-ref, Thread Safe — MCP 로 못 켜 사용자가 체크) = `BlendSpacePlayer_1` 의 On Update, `Select_2 → BlendSpace` 연결 해제(옛 Select 체인 잔해 [W128]). cvar `SoldierLab.Pose.AimOffsetInertialSwitch`(0 = 옛 즉시 교체). `Build.cs` `AnimGraphRuntime`. [W11] 해결. 같은 문서 9절 |
| **`Debug/SoldierAimTrace`** | ★ **프레임 단위 조준 녹화기** (2026-09-30 신규) | `USoldierAimTraceSubsystem`(월드 서브시스템, BP 변경 없음, 꺼져 있으면 비용 0). cvar `SoldierLab.Debug.AimTrace 1/0` · `.Filter`. `Saved/AimTrace/<시각>/<병사>.csv` — 교전 컴포넌트를 가진 캐릭터마다, 프레임당 1줄(교전 판단 → 컨트롤러 → 몸 → MM 선택 + 블렌드 스택 5칸 · 몽타주 → 본 월드 변환 → 총구 → ABP/캐릭터 BP 변수 자동 덤프). `USoldierEngagementComponent` 가 `friend` 로 선언. `Build.cs` `PoseSearch` · `BlendStack`. 사용법·판정 규칙은 `CLAUDE.md` 6.2c · P199. 같은 문서 5절 |
| **`AI/SoldierQuery.h`** | ★ **트레이스 공통 응답** (2026-09-21 후편, 헤더만 — [W110]) | **`SoldierQuery::BodiesAreNotWalls()`** → `FCollisionResponseParams` 에 **ECC_Pawn → Ignore**. 병사는 캡슐·메시 모두 Pawn 이고 이 레벨들의 Pawn 오브젝트 타입은 병사뿐(트럭/UGV ECC_Vehicle · 드론 PhysicsBody · 총 NoCollision)이라, 레지스트리 35명 `AddIgnoredActor` 루프(호출당 35 × 190회/프레임)를 **Engagement 1 · Cover 5(`IgnoreBodies` 는 빈 함수로 남김) · Field 2** 트레이스 사이트에서 제거. ⚠ **Pawn 타입 액터가 새로 들어오면 전제가 깨진다**. 같은 세션 Engagement `IsShotBlockedByWorld` 8슬롯 캐시(`FLaneAnswer`, `LaneCacheMoveCm 15`/`LaneCacheSeconds 0.15`)와 함께 씬 쿼리 1,030~1,120 → 357/576회. 같은 문서 B절 |
| **`Weapons/SoldierProjectilePool`** | ★ **투사체 풀** (2026-09-21 후편, ✅ PIE — [W109]) | `USoldierProjectilePoolSubsystem`(월드 서브시스템) `Acquire(Class, Owner)` — 3.1절. cvar `SoldierLab.Projectile.PoolMax 96` ★ **09-22**: **`RecallAll()`** — 나는 탄 전부 `ASoldierProjectile::Park()`(풀로 주차, 비활성) — 시나리오 재시작 리셋 계약(P187) |
| **`Squad/SoldierOrderTypes`** | ★ L0/L1 명령 계약 (2026-09-17, **빌드 전 [B]**) | `FSoldierAssignment`(Mode Approach/Hold · Anchor/Radius/Band/ApproachScale · Speed Cautious/Normal/Rush · ROE Free/ReturnFireOnly/HoldFire · Aggression · Sector · SuppressArea · NavFilterClass · bTargetableByOwnSideWeapons · Revision) — **전부 제약**. `FSoldierSquadOrder`(Verb MoveTo/Occupy/Withdraw/SetROE/SuppressArea/SetTargetable/Clear + Stagger + ZoneTag). `SoldierSquadAssignmentLabel()` ★ **09-18 오후**: 배정·명령에 **`PatrolWeight`(기본 0) · `PatrolStaleSeconds 30` · `MinStance`(기본 0)** 추가 — 존 동사와 함께 `BuildAssignment`가 복사 · **`bHasSector`의 뜻이 부채꼴(arc)로**(주석 갱신). titan `FScenarioSquadOrderSpec`엔 셋 다 **없음** ★ **09-21**: 동사 **`BreakContact`**(`.h:44-49`, 규칙 동사 — `bBreakContact=true` + ROE/`EngageRangeCm`/Speed 를 SetROE 처럼 나름, 다음 존 동사가 `FSoldierAssignment()` 리셋으로 지움) · `FSoldierAssignment::bBreakContact`(`:232-241`) · **`bTargetableByOwnSideWeapons` 주석 정정**(`:223-230` — "상대편 무기(차량은 브리지, 보병은 Engagement)가 이 병사를 표적으로 안 삼는다"는 시나리오 지시, 존 동사에도 이어짐 `SoldierSquadSubsystem.cpp:277`) · 라벨 ` BREAK`/` excl`(`:95-96`) [A] |
| **`Squad/SoldierSquadSubsystem`** | ★ L1 분대 (2026-09-17 → 09-18 빌드·PIE [A]) | 분대 객체 없음 — 등록부 (Faction, `SquadId`), **`bTakesSquadOrders=false`(차량) 제외**. `IssueOrder`(지연 출발, 규칙 동사는 대기 중 배정에도 적용) · Approach→Hold 자동(~~반경×`ArrivalInsetFraction 0.8`~~ → **반경 × 배정의 `ArrivalFraction`**, 존에서 옴, 09-18) · achieved(생존 전원 도달) · **`ReinforceSquads(Faction, SquadIds, Quota, Toward)`**(정원제 — 다른 분대에서 목적지에 가까운 순으로 `SquadId` 영구 변경, `[Squad] reinforce …`) · `GetSquadStatus/IsOrderAchieved/GetSquadZoneTag`. 서버 전용. `SoldierLab.Debug.Squad` / `.Squad.Log`. 익명 네임스페이스 헬퍼는 `Squad` 접두(유니티 빌드) ★ **09-22**: **`ResetForRestart()`**(명령/멤버 맵 · 시리얼 초기화 — `ReinforceSquads` 가 바꾼 `SquadId` 는 병사 재스폰이 저작값으로 되돌림) + 콘솔 **`SoldierLab.ResetWorld`**(분대 · 상황 필드 · 투사체 풀 셋 한 번에) — 시나리오 재시작 리셋 계약(P187) |
| **`Squad/SoldierZone`** | 저작 액터 (2026-09-17 → 09-18 [A]) | RadiusCm/**ArrivalFraction 0.8**/BandCm/ApproachScaleCm/섹터(액터 yaw, **09-18 오후부터 부채꼴** — `bUseSector true`·`SectorHalfWidthDeg 45`)/**`PatrolWeight 1.0`·`PatrolStaleSeconds 30`**(09-18 오후 — 존을 거친 명령만 순찰이 켜짐)/NavFilterClass. `ApplyToOrder`. **에디터 전용 표시**: 반경(노랑)·도착선(초록)·밴드(흐림) `USphereComponent` + 섹터 `UArrowComponent`(길이 = 반경) + `S_TargetPoint` 스프라이트, `OnConstruction` 갱신, 쿡 제외. PIE `SoldierLab.Debug.Zone` |
| 훅 (2026-09-17 → 09-18 → ★ 09-21 [A]) | Identity `SquadId`·`bSquadLeader`·`Assignment`·**`bTakesSquadOrders`**(09-18) · 레지스트리 `GetGunshotCount` · **`OnGunshot` 멀티캐스트**(09-18, 서버·병사만 — titan 브리지가 RCWS 청각 `ReportGunfire`로 넘김) | Cover: `TaskCost`(배정이 Objective **대체**, Approach = **도착선 안 0 / 밖 `1 + 거리/ApproachScaleCm`**, 09-18) · `AggressionScale`(2×agg) · NavFilter · Objective 매 스윕 재해석 · `[Cover] task=` · ★ 09-21 **`IsBreakingContact()`**(`bBreakContact && Mode==Approach`, `SoldierCover.cpp:152-160`) → `ScorePosition` 이 Fighting/Route/Danger/Suppression 을 0 으로(`:1839-1845`, Objective·Squad 만 남음) · `MinDwell`/`ScanDwell` 없음(`:2050`) / Engagement: ROE 게이트 + **`EngageRangeCm`**(09-18, ROE 옆 — 넘으면 `Restrained`) + 의도 `Restrained` · 섹터 조준 · Speed↔스프린트 · `ReturnFireWindowSeconds 3` · `WantsToAim()` · `[Engage] roe=free/4000` · ★ 09-21 **`IsContactExcluded(record)`**(static, 상대 배정 `!bTargetableByOwnSideWeapons` — `PickCandidate` 제외 `:488` · 잠금 해제 `:683`, 인지 불변) · `bBreakContact` → 이동 중 `bWantsToSprint=true`(`:1579-1582`) · `DesiredStance=0`(`:852-855`, 재장전 웅크림은 그대로 · ⚠ `MinStance` 바닥이 그 뒤 `:859-862` 라 Rush 아니면 도로 올린다) / Perception: `ReportOrderedArea`(`bOrderedArea` 기록은 융합 안 함, `OrderedAreaRefreshSeconds 1`·`OrderedAreaCertainty 0.6`) / **P5 게이트** Cover·Engagement·Sight·Comms / titan: `IssueSquadOrderSpec` `SetTargetable(false)` → UGV RCWS `bRespectEnemyTargetingExclusion=true`(`ScenarioStateSubsystem.cpp:1919-1928`, 트럭 제외) |

★ **2026-09-14 오후 — 높이 기준면 통일 (P103)**: `USoldierIdentityComponent::GetFeetLocation()`(원점 − 현재 캡슐 반높이)이
AI 층의 유일한 높이 기준이다. 엄폐 HERE · 경로 시작 · 총구(`MuzzleAtStance`) · 관전 폴백이 전부 이 위에 더한다.
`SoldierCover` 에 **`ThreatEyeAboveContactCm 20`** 신설(기록 위치는 이미 가슴이다), 엄폐 트레이스는 **등록부 전원 무시**,
오버레이에 **`st`**(RequiredStance) 추가. `SoldierEngagement` 의 상수 `CapsuleHalfHeightCm 90` 삭제. [C-95] 원인.

`Build.cs`: **`AIModule`**(`SetFocalPoint`) · **`NavigationSystem`**(엄폐 후보 투영) 추가. ★ 09-14 저녁: **`EnhancedInput`**(런타임 IA/IMC 토글) 추가. ★ 09-17 저녁: **`DeveloperSettings`**(`USoldierFieldSettings` — 필드 해상도는 액터가 아니라 프로젝트의 속성) 추가. ★ 09-21: `PrivateDependencyModuleNames` 에 **`RenderCore` · `RHI`**(`USoldierDebugMeshComponent` 의 자체 씬 프록시 — 오버레이 사각형을 선형 색으로, P181) 추가.

⚠ **[C-95] 상태 (2026-09-14 저녁)**: 기준면 수정은 **빌드됐으나 수비수 정착 여부는 사용자가 아직 보고하지 않았다** — 판정 기준은 `ai/2026-09-14_cover_frame_fix_and_observer.md` 1.6절.

**엄폐/위치 튜닝값 갱신 (2026-09-13 후반)**: `RouteSamples 3` · `RouteRiskWeight 1.0` ·
`ObjectiveWeight 1.2` 신설, `MaxTracesPerTick 4→6`(후보 한 개가 3발→6발) ·
`MoveImprovementMargin 0.35→0.25`. `ASoldierObjective`: `RadiusCm 900` ·
`DefendBandCm 900` · `ApproachScaleCm 6000`. **전부 미측정** → **[C-88]**.
⚠ **이 단락의 값 대부분은 2026-09-14에 바뀌었다** — 아래가 현행값이다.

★ **2026-09-14 교정분** → **[C-91]~[C-94]** · **[C-96]**:

```
SoldierCover     NoCoverCost 1.0 / NoFiringPositionCost 0.5   (신설 — FightingCost)
                 RouteRiskWeight       0.6   (← 1.0)  거리로도 스케일한다 (P83)
                 ObjectiveWeight       2.0   (← 1.2)
                 MoveImprovementMargin 0.3   (← 0.25 ← 0.35)
                 InnerRingFraction     0.45  (신설 — 두 겹 링)
                 StallSpeedCms         20    (신설 — 정지 감지, P87)
                 bUseAvoidance / AvoidanceRadiusCm   true / 300   (RVO)
SoldierObjective RadiusCm 1500 (← 900) · DefendBandCm 1200 (← 900)
                 ApproachScaleCm 3000 (← 6000) · 공격 비용의 clamp **제거**
SoldierEngagement  조리개/자세  LateralReachCm 70 · LeanSpreadScale 1.4 ·
                                BlindSpreadScale 4.0 · SuppressionToGoBlind 0.45 ·
                                BlindFireCloseRangeCm 400
                   조준 선회    ~~AimSlewDegreesPerSecond 240~~ · OnTargetConeRatio 1.0
                                ★ 09-30: AimSlewDegreesPerSecond 150 · AimSlewNearDegreesPerSecond 40 ·
                                AimSlewNearErrorDeg 3 · AimSlewFarErrorDeg 30 (오차 비례, [C-93] 해결)
                   반동        RecoilPerShot 0.18 · RecoilRecoveryPerSecond 1.2 ·
                                MaxRecoilSpreadScale 2.5 · BlindRecoilScale 2.0
SoldierPerception  VelocityTrustSeconds 1.5   (총성 기록은 0)
```

**하나도 재지 않았다.** ~~그리고 ★ 수비수가 정착하지 못하는 문제는 여전히 미해결 → [C-95]~~ → 해결(기준면 P103 + 아래 라운드).

★ **2026-09-14~15 거동 라운드 현행값** (`ai/2026-09-15_exposure_cycle_and_muzzle_learning.md` 10절이 원본, 전부 [C] → [C-102]~[C-105] [C-107]):

```
SoldierCover     MaxTracesPerTick 36 (← 19 ← 6) + 틱당 최소 후보 1개 하한
                 MaxThreatEyes 3 · MinThreatCertaintyForCover 0.05 · ThreatEyeSmoothingSeconds 1.0
                 bFanCandidates true · FanRadiusCm 2400 · FanRays 24 (← 32) · CandidateMergeCm 150
                 RingCandidateCount 12 (← CandidateCount) · InnerRingFraction 0.45 · bSnapCandidatesIntoShadow true · ShadowStepCm 60
                 RouteSampleSpacingCm 400 · MaxRouteSamples 8 (← RouteSamples 3) · 경로 span 클램프 제거
                 NoCoverCost 1.0 · NoFiringPositionCost 0.9 (← 0.5)
                 RouteRiskWeight 0.6 · ObjectiveWeight 2.0 · MoveImprovementMargin 0.3
                 DangerWeight 0.8 · DangerHalfLifeSeconds 30 · SuppressionWeight 0.5   (자리 Danger는 눈 0일 때만)
                 FiringRecentSeconds 3 · FiringFadeSeconds 5 · QuietEyeWeight 0.3
                 MinDwellSeconds 3 · DwellBreakMargin 1.0 (bCanHide일 때만) · StallSpeedCms 20 · bUseAvoidance true / 300
                 ~~StandChestHeightCm / CrouchChestHeightCm~~ → Identity
SoldierEngagement  TargetSwitchCertaintyMargin 0.3 · TargetSwitchDistanceRatio 0.6
                   SuppressiveRadiusCm 500 (총 산포만) · SuppressiveKnowledgeRadiusCm 1000 (신설)
                   ObserveAfterSeconds 1.5 · ObserveSuppressionThreshold 0.3
                   ExposureRiseSeconds 1.5 · ExposureFallSeconds 2.0 · PeekStartBelow 0.2 · PeekStopAbove 0.8
                   PeekQuietActivity 0.3 · PeekFiringRecentSeconds 3 · PeekFiringFadeSeconds 5
                   LeanAxisVariable LeanCurrent · BlindFireHVariable BlindFireH · BlindFireVVariable BlindFireV
SoldierIdentity    StandChestHeightCm 135 / CrouchChestHeightCm 80 / EyeHeightCm 160 (측정 전 초기값, bMeasureChestHeights true)
                   MuzzleSocket "Muzzle" · bLearnMuzzleOffsets true
                   StandMuzzleOffsetCm (40,20,140) · CrouchMuzzleOffsetCm (40,20,95)
                   LeanRight/LeftMuzzleDeltaCm (0,±70,−10) · BlindUpMuzzleDeltaCm (0,0,+55) · BlindRight/LeftMuzzleDeltaCm (−10,±80,0)
SoldierPerception  FSoldierEnemyRecord.LastGunshotTimeSeconds (−1 = 없음)
~~SoldierDangerMap   CellSizeCm 200 (상수)~~   → 2026-09-17 폐기, 아래 09-17 블록
```
⚠ 위 블록의 `DangerHalfLifeSeconds 30`은 **2026-09-17에 삭제**됐고 `DangerWeight 0.8`은 상황 필드 `GetExposure`를 읽는다.

★ **2026-09-16~17 분대 항·죽음·시야 현행값** (`ai/2026-09-16_squad_terms_and_learned_death.md` 7절이 원본, 전부 [C] → [C-108] [C-109] [C-121]):

```
SoldierCover|Squad   ClaimRadiusCm 250 · ClaimedCost 1.0 · LaneClearanceCm 150 · LaneCost 0.6 · CoveredEyeFactor 0.5
SoldierEngagement    TargetCrowdingPenalty 0.15 · CoveringStillSpeedCms 20
                     bWorthShot 에 앎 ≤ SuppressiveKnowledgeRadiusCm 1000 조건 추가 (값 그대로)
SoldierPerception    ClearViewHalfLifeSeconds 1.5 (신설)
SoldierSight         SightRangeCm 12000 (← 6000) · MaxTracesPerTick 5 (← 3)
SoldierIdentity      웅크린 머리 높이 실측(MeasuredCrouchEyeHeightCm), 측정 전 = 웅크림 가슴 + (기립 머리 − 기립 가슴)
SoldierComms         사망 보고는 기존 VoiceRangeCm 2500 · MessageDurationSeconds 1.2 공용
```
⚠ **두 점 시야(Sight/Cover/Identity 일부)는 2026-09-17 기준 Perforce 미제출** — CL 471(분대 항·죽음) · 472(블라인드·부정 증거·시야 12 m)까지가 제출분.

★ **2026-09-17 오전 — 교전 층 수정 넷 현행값** (`ai/2026-09-17_threat_bonus_lane_denied_vehicle_heights.md` 5절, 전부 [C] → [C-136] [C-126]):

```
SoldierEngagement    ThreatenedBonus 0.6 · ThreatenedMemorySeconds 5 · LaneDeniedSeconds 2 · WorthHysteresis 1.2
SoldierCover         LaneDeniedCost 1.0 (HERE만)
SoldierIdentity      비-캐릭터 폴백 = 바운즈 Z 비율 눈 0.8 / 표적 0.5
```

★★ **2026-09-17 저녁 — 상황 필드 · 잠입 현행값** (`ai/2026-09-17_situation_field_lighting_model.md` 10절 · `ai/2026-09-17_infiltration_and_unknown_ground.md` 9절, 전부 [C] → [C-130]~[C-135] [C-138] [C-139]):

```
USoldierFieldSettings (Project Settings → Game → SoldierLab Situation Field, DefaultGame.ini)   ★ 09-18 갱신
  Resolution  CellSizeCm 200 · LevelCount 3 (← 1) · LevelScale 4 · DetailRadiusCm 8000 (신설, DetailRangeCm 3000 삭제) ·
              EvictionCellsPerTick 1024 · EvictionIntervalSeconds 2 · CoarseRefreshSeconds 1   (신설)
  Presence    PresenceHalfLifeSeconds 8 · UnknownPresence 0.5 · BodyBandCm 200
  Lights      LightHalfLifeSeconds 20 · LightDeathThreshold 0.05 · ClearViewHalfLifeSeconds 4 (신설, [B]) · ContinuityWindowSeconds 0.5 ·
              MaxTrackedSpeedCms 600 · TrackToleranceCm 150 · MergeRadiusCm 300 · MaxLights 16 · SharpRadiusCm 400 · FrozenPresence 0.6
  Shadow      LightEyeHeightCm 160 · CrouchTopCm 110 · StandTopCm 175 · ShadowRays 48 · ShadowRaysPerTick 96 · ShadowRangeCm 6000 ·
              LightFullRangeCm 3000 · LightMaxRangeCm 9000 · ShadowChannel GameTraceChannel5 · MaxMarchCellsPerRay 32
              (상수 GroundSlackCm 40 — 셀 지면 아래 허용, ← −CrouchTop 110)
  Ambient     HorizonRangeCm 4000 · HorizonStandHeightCm 135 · HorizonCrouchHeightCm 70 · HorizonBakesPerTick 8 · AmbientWeight 1 · HiddenThreshold 0.25
  Debug       MaxDebugCells 6000 (← 4000) · DebugRefreshSeconds 0.1 · DebugDetailRadiusCm 2500 · DebugFill 0.7 · DebugAlphaMin 0.1 · DebugAlphaMax 0.8 ·
              DebugStaleSeconds 30   (DebugAlpha 0.35 · DebugFillMin/Max 삭제; 상수 DebugLiftCm 12)
  cvar        SoldierLab.Field.CellSizeCm 0 (신설) · SoldierLab.Debug.Field.Level -1 (← 0) · SoldierLab.Debug.Field.RadiusCm 12000 (← 4000)
SoldierSight         ConeSweepTracesPerTick 2 · ConeSweepRays 21 · ConeSweepRangeCm 4000 · SightingRadiusCm 100   (신설)
SoldierPerception    HeardPresence 0.75   (신설 — 사전값 0.5보다 커야 한다)
SoldierCover         FieldCandidateCount 6 (신설) · 골든앵글 2.39996 rad/스윕 (상수) · DangerWeight 0.8 (뜻 변경) · DangerHalfLifeSeconds 삭제
SoldierEngagement    스프린트 규칙 bUrgent (새 값 없음)

★ 09-18 오후~저녁 (ai/2026-09-18_patrol_scan_and_move_robustness.md 10절 → [C-148]~[C-153])
SoldierCover         WatchApproachBias 0.5 · WatchTravelBias 1.0 · ScanDwellSeconds 2 · MoveGraceSeconds 0.75 · CandidateRejectSeconds 30   (신설, PIE)
                     HiddenGazeFraction 0.5 · CoverAcceptanceRadiusCm 20 · MicroStepCm 30 · CornerAngleDeg 35 · CornerLookAheadCm 500 · CornerStopCm 150 · CornerPauseSeconds 0.8   (신설, ~~빌드 전~~ 밤 PIE ✅)
★ 09-18 밤 4차 (같은 문서 12~14절 → [C-155]~[C-156])
SoldierEngagement    [Gait] TensionHalfLifeSeconds 20 · JogTension 0.3 (+ 알람 창 총성 1 s 상수)                                              (신설, PIE)
                     [Pose] UrgencyIdle 0.15 · UrgencyContactIdle 0.35 · UrgencyLookPeek 0.3 · UrgencyShootPeek 0.6 · UrgencyRetreat 0.7 ·
                            RetreatUrgencySeconds 0.6 · UrgencyReload 1.0 (제압은 자기 값 max)                                             (신설, PIE)
SoldierCover         코너 "같은 굽이" 반경 = CornerStopCm 재사용 (새 값 없음)     SoldierSituationField   GetExposure 미지 = UnknownPresence × AmbientWeight (새 값 없음)
ASoldierZone         PatrolWeight 1.0 · PatrolStaleSeconds 30   (신설)     ASoldierObjective   PatrolWeight 1.0 · PatrolStaleSeconds 30   (신설)
FSoldierAssignment / FSoldierSquadOrder   PatrolWeight 0 · PatrolStaleSeconds 30 · MinStance 0   (신설 — titan DT 미연결)
SoldierSituationField   WidenFromCm = 셀 × 8 (상수) · USoldierFieldSettings 변경 없음
Pose/SoldierScanTurnComponent (포즈 세션)   StartDegrees 20 · StopDegrees 5 · TurnRateDegPerSec 180 · MaxSpeedCms 10 · bOnlyWhileScanningOrInContact true   (✅ PIE)

★ 09-17~18 포즈 세션 (animation/2026-09-18_ai_pose_layer_scanturn_gait_smoother.md → [C-154])
Pose/SoldierGaitBridgeComponent    bDriveSprint false · InputStateVariable CharacterInputState · WalkField WantsToWalk
Pose/SoldierPoseSmootherComponent  Stance {Up 1.6 · Down 0.9 · Accel 5} · Lean/BlindFireH/BlindFireV {1.2 · 1.2 · 4} · ScaleAtCalm 0.5 · ScaleAtUrgent 1.6
                                   RateVariablesToFreeze {StanceRate, BlindFireRate} · FrozenRate 0.0001 (0 금지, P167) · 스냅 eps 0.002/0.02 (상수)
BP_SoldierCharacter                린 AI 램프 핀 K2Node_CallFunction_92.RatePerSecond 1.0 → 0.0001 (스무더가 못 얼리는 리터럴)
Observer/SoldierObserverPawn       FlySpeedCms 1200 · FlySpeedWheelFactor 1.25 · Min 100 · Max 20000 · bIgnoreTimeDilation true · WheelStepCm 50 (3인칭 거리)

★★ 09-21 분대 필드 · 엣지 전진 · 사격 콘 · 섀도우 감축 (ai/2026-09-21_per_squad_field_edge_advance_fire_model.md 6절 → [C-157]~[C-161])
USoldierFieldSettings   Resolution MaxSquadsPerFaction 3 (신설, PIE) · Shadow ShadowRecastMoveCm 0 (= 한 셀) · ShadowRecastSeconds 0.5 (신설, 같은 날 빌드·PIE ✅ — 재측정 0.03 ms · 0 waiting · 96 alive)
cvar                    SoldierLab.Debug.ExposureScale 0 (신설 09-21 늦게 — 0 자동 = 뷰 노출의 역수 · 그 외 수동 배율; 디버그 cvar, [C] 아님, P181)
                        (상수) riders 동승 반경 반 셀 · 얼림 시 재캐스트 문턱 반 셀 · 비용 평활 Lerp 0.1 · GetWedgePresence 4°/갈래 ≤ 12 · 표본 ≤ 48
cvar                    SoldierLab.Debug.Field.Squad -1 · SoldierLab.Debug.Field.Centre 0   (신설)
SoldierCover [Advance]  bEdgeAdvance true · StepPresenceBudget 6 (Cautious ×0.5) · AnalyzedPresence 1 · MaxLookSeconds 3 · EdgeRangeCm 1000 · WedgeRangeCm 1500 ·
                        EdgeIgnorePresence 0.5 · AdvanceStepCm 60 · AdvanceStepCount 4 · AdvanceLean 0.7 · AdvanceCheckSeconds 0.1 · AdvanceRetreatSeconds 10   (신설, 빌드됨·PIE 대기)
                        (상수) 이웃 광선 신선도 0.75 s · 엣지 far ≥ max(2×near, near+300) · 탐침 10° · 부채꼴 {0,±35,±70,±105}° · 조준점 apex+8 m · 후퇴 자리 8
                        ~~CornerStopCm 150 · CornerPauseSeconds 0.8~~ 삭제 · CornerAngleDeg 35 · CornerLookAheadCm 500 유지(볼 곳 미리 보기만)
SoldierSight            (기존) ConeSweepRays 21 · SightHalfAngleDeg 60 → 엣지 해상도 6° · ConeSweepTracesPerTick 2 → 한 바퀴 ≈ 0.35 s
SoldierEngagement       WeaponSpreadDegrees 0.8 (← 3) · MovementSpreadScale 6 (← 2) · LeanSpreadScale 1.5 (← 1.4) · BlindSpreadScale 15 (← 4)   (PIE — 가치 사거리 보존)
             [Accuracy] AimSettleInitialDeg 2.5 · AimSettleSeconds 0.4 · RecoilKickDeg 0.6 · MoveWobbleDeg 1.5 · TargetRadiusCm 45 · AimedHitTolerance 2   (신설, PIE)
             [Rhythm]   BurstRoundsMin 2 · BurstRoundsMax 5 · BurstPauseSeconds 0.5 · RhythmJitter 0.35   (신설, PIE)  (상수) 흔들림 상한 30° · 시드 GetTypeHash(이름)
                        ~~KnowledgeToSpreadRatio~~ 삭제
⚠ 위 SoldierEngagement 09-14 교정분의 LeanSpreadScale 1.4 · BlindSpreadScale 4.0 은 09-21 에 1.5 · 15 로 바뀌었다(기준 3° → 0.8° 재스케일).
⚠ 무기 BP 는 아직 GetShotSpreadDegrees() 를 읽지 않는다 — 탄착은 옛 고정 콘 [W93].

★ 09-21 성능 계측 · 엄폐 비용 (ai/2026-09-21_perf_instrumentation_and_cover_cost.md 4절 → [C-162])
SoldierCover [Cover|Cost]  HereEvalIntervalSeconds 0.1 · CalmCandidatesPerTick 2 (ClampMin 1) · WatchRefreshSeconds 0.25 (바닥 0.05)   (신설, PIE 실측 — ms 는 [A], 거동 지연은 [C-162])
                        (재사용) 이동 중 판정 = MoveGraceSeconds 0.75 · StallSpeedCms 20 (FinishSweep 의 bAlreadyGoing 과 같은 식) · HERE 걸음 문턱 = MicroStepCm 30
cvar                    SoldierLab.Sight/Perception/Cover/Engagement/Suppression/Comms/Field.Enabled 1 · SoldierLab.Cover.Avoidance 1   (신설 — 계측용, [C] 아님)
실측 (35명 · PIE · 로깅 on)  Cover Tick 8.92 → 1.92 ms · Traces: Cover 487 → 243 · SoldierLab 합 ≈ 3.3 · World Tick 28.5 → 22.6 · RVO 0.3~0.5 ms (켜 둠)

★ 09-21 분대 — New_kadex_0811 이관 · BreakContact · 표적 제외 (squad/2026-09-21_break_contact_and_targeting_exclusion.md → [C-163]~[C-164])
FSoldierAssignment / FSoldierSquadOrder   bBreakContact false (신설, 새 튜닝값 없음 — 비용 항 0 은 상수) · 동사 BreakContact
New_kadex_0811 ASoldierZone ×8            Z0_S1/S2/S3_Engage r1000/1200/1000 yaw 90 · Z1_S2/S3_Withdraw r1200 yaw 0 · Z2_S3_Escape r1500 · ZF_North r2500 / ZF_South r2000 yaw 180 ·
                                          NavFilterClass = NavQueryFilter_EnemySquad1/2/3 · 나머지 기본값(ArrivalFraction 0.8 · Band 1200 · ApproachScale 3000 · Patrol 1.0/30)   [B 인스턴스]
DT_ScenarioSteps_ThreeStage_SoldierLab    EnemyApproach MoveTo z0 Cautious HoldFire agg 0.3 · EnemyEngage(UGVFiredNearEnemy 10000) Occupy z0 Rush Free 0.8 ·
                                          EnemyFleeToZone2(사망 ≥3) 2,3 Withdraw z1 Rush ReturnFireOnly Quota 10 stagger 0.5~3 · EnemyFleeToZone3(≥7) 3 Withdraw z2 Quota 5 ·
                                          ExcludeFleeingEnemies(+4 s) 3 SetTargetable false · Squad3Run(+6 s) 3 BreakContact HoldFire Rush · Squad3Stand(CommandPostFiredNearEnemy 8000) 3 Occupy z2 Rush Free 0.8 ·
                                          AllyDefend(+1 s) Occupy z0 ReturnFireOnly 0.3 EngageRangeCm 6000 · AllyEngage(EnemyNearFriendlySoldiers 8000) Occupy z0 Free 0.5   [B 값 / A 행 이름]
titan RCWS                                UGV bRespectEnemyTargetingExclusion: 인스턴스 false → SetTargetable(false) 행이 런타임에 true (트럭은 false 유지)

★ 09-21 후편 — 게임 스레드 구조 묶음 (ai/2026-09-21_game_thread_structural_pool_rays_bridge.md A~E절, 전부 [A] 실측 · 거동 지연은 [C-162])
SoldierEngagement [Lane]   LaneCacheMoveCm 15 · LaneCacheSeconds 0.15   (신설 — IsShotBlockedByWorld 8슬롯 캐시, PlanAperture 옵션 포함)
SoldierCover               CandidateIntervalSeconds 0.033   (신설 — 눈 있는 스윕의 후보 걸음을 시간에 고정; 눈 0 은 CalmCandidatesPerTick 2 그대로)
SoldierSight               ScanIntervalSeconds 0.033        (신설 — 스캔을 시간에 고정)
SoldierProjectile          MaxFlightDistanceCm 60000        (신설 — 600 m, 시간 상한 MaxFlightTimeSeconds 5 와 같은 자리 주차)
cvar                       SoldierLab.Projectile.PoolMax 96 (신설 — 클래스별 풀 상한, 넘으면 라운드로빈) · SoldierLab.AIBridge.Native 1 (A/B, [C] 아님)
CMC (BP 데이터, CDO 3 + 인스턴스 35)   bAlwaysCheckFloor false (← true) · bEnablePhysicsInteraction false (← true)
CharacterMesh0 (BP 데이터, 같은 범위)   CollisionEnabled QueryOnly (← QueryAndPhysics — 3.2절 정정 인용)
실측 (35명 · PIE · 로깅 on)  World Tick 18.3/20.3 → 11.3/14.0 · 씬 쿼리 1,030~1,120 → 357/576회 · Shoot 안 스폰 0.83/발 → 0 · 아군 20 ReceiveTick 1.2 → 0.22 · FindFloor 99~105 → 44/27회
```
~~⚠ **3단계(호라이즌/앰비언트 · `FindDarkestCells` · 눈 0 필드 자세 · 볼 곳 · 가상 관찰자 철회)는 코드만 있고 빌드 0회 [B].** 1·2단계 + 오버레이는 PIE에서 사용자 확인.~~ → **09-18: 3단계 + LOD 링 + 오버레이 v2 배처까지 PIE 확인.** 지금 [B]인 것: 대칭 캡 · `MaxDebugCells 6000` · 라이트 시각 v2 · `ClearViewHalfLifeSeconds` · 미굽기 파랑끼 · 헤더 2줄 · 화살표 2.5 m(시스템 문서 14절).

### 5.2 블루프린트 [A]

```
BP_SoldierCharacter    컴포넌트 ~~7개~~ ~~10개~~ ~~13개~~ **14개** — AC_SoldierIdentity / Perception / Sight / Comms /
                                      Suppression / Engagement / Cover
                       ★ 09-14 저녁 +2 — AC_SoldierFirstPerson (T · 1/3인칭) ·
                                        AC_SoldierHeadAim (H · 머리 추종, 기본 OFF, AI 자동 활성화 없음 P171)
                       ★ 09-15 저녁 +1 — AC_SoldierHealth (체력·피격·사망, 3절 참고.
                                        BP_Soldier_Friendly 에서 Invincible 체크)
                       ★ 09-17~18 +3 — AC_SoldierScanTurn (AI 몸 회전) · AC_SoldierGaitBridge (Walk→WantsToWalk) ·
                                        AC_SoldierPoseSmoother (자세 축 사다리꼴) — 셋 다 AI 전용, MCP add_component
                       ★ 09-18 EventGraph — 린 AI 램프 RampAxisTo 의 RatePerSecond 리터럴 1.0 → **0.0001**
                                        (0 이면 목표 반환 = 스냅, P167. 스무더가 얼리는 StanceRate/BlindFireRate 와 달리 변수가 아님)
                       AI 다리 변수 5개 (카테고리 SoldierLab|AI Bridge, 인스턴스 편집 가능)
                           AIPoseDriven · AITargetLean · AITargetStance
                           AITargetBlindFireH · AITargetBlindFireV
                       ★ 09-21 게임 스레드 묶음 (ai/2026-09-21_game_thread_batch_cameras_abp_muzzle.md 4.1 · 4.4절)
                           변수 **AnimBP** (SoldierCharacter_ABP_C 참조) — BeginPlay 의 CastToSoldierCharacter_ABP 결과 저장,
                             Tick 의 캐스트 제거(SetAimCorrection / SetLeanTactical / GetAOActive 가 이걸 씀) · SetUseAllyAnimSet 은 BeginPlay 로
                           BeginPlay 끝: `not IsPlayerControlled` → SetComponentTickEnabled(GameplayCamera, false) → (SpringArm, false)
                             → (Camera(NotUsedByDefault), false) — **틱만 off, Deactivate 아님**(P107). AI 40명의 GASP 카메라 2.0 ms 제거
                           ⚠ bEnableUpdateRateOptimizations 는 **URO 크래시로 false 여야 한다**(P186) — ~~지금 CDO·자식·인스턴스에 true 가 남아
                             `a.URO.Enable=0` 으로 막는 중 → [W107] 되돌리기~~ → 재기동 뒤 CDO 3 + 인스턴스 false 로 되돌림, cvar 줄 삭제 ([W107] 해결, 같은 문서 10절)
                           런타임 컴포넌트 38개 = CDO 29 + PIE 카메라 프록시/프러스텀 6 + OutputCamera + GameplayTasks + SoldierLabDetectable
                       ★ 09-21 후편 +1 — **AC_SoldierAIBridge** (USoldierAIBridgeComponent, 5.1절 — EventTick 본문의 C++ 판, [W108])
                           EventTick 구조: **Parent:Tick → TickBridge(DeltaSeconds) → Branch(ShouldRunBlueprintCopy) → (true) 옛 본문**
                             옛 본문(조준 보정 → 린 램프 → 입력 상태 → AI 다리 → Update* → Shoot/Reload + 리플리케이션 세션의 HasAuthority 분기 · 바운드 이벤트 2)은
                             그대로 남아 cvar SoldierLab.AIBridge.Native 0 이거나 바인딩 실패 때만 돈다. 변수 47개는 여전히 BP 소유(아래 "AI 다리" · 3절 변수 목록 불변)
                           CMC bAlwaysCheckFloor false · bEnablePhysicsInteraction false · CharacterMesh0 QueryOnly — CDO 3 + L_SoldierScenario 인스턴스 35 (3.2절 정정 인용, [W111])
                           아군 20 ReceiveTick 1.16~1.21 → 0.22 ms (ai/2026-09-21_game_thread_structural_pool_rays_bridge.md C · E절)
BP_Soldier_Friendly    ─┬ BP_SoldierCharacter 의 자식. Faction 기본값 하나만 다르다 (P4)
BP_Soldier_Hostile     ─┘
BP_AR4Rifle            Tick 에서 SetWeaponState(탄/탄창/재장전중) 를 캐릭터 교전 컴포넌트로
                       ★ 09-21 게임 스레드 묶음 — 컴포넌트 **MuzzleFlashFX**(NiagaraComponent, MuzzlePoint 자식, bAutoActivate=false, Asset NS_MuzzleFlash)
                         `Shoot` · `PlayShotCosmetics`(리플리케이션 세션 함수) 의 ~~SpawnSystemAttached~~ → **Activate(bReset=true)**
                         (SpawnSystemAttached 는 bAutoDestroy=false 라 발당 컴포넌트가 영구 누적 — 8정에 106개 — 누수였다). 자식 BP_AK47Rifle 상속 확인
                       WeaponMesh(스폰된 총 — BeginPlay 에서 숨겨지는 쪽, 3절 정정) VisibilityBasedAnimTickOption = **OnlyTickPoseWhenRendered**
                       발당 비용 실측 ≈ 1.1 ms(투사체 스폰 0.57 + 총구 0.38 + Launch 0.08) — ~~풀링 [W109]~~ ★ 09-21 후편: Shoot · PlayShotCosmetics 의 SpawnActor →
                         **GetSoldierProjectilePoolSubsystem → Acquire(BP_RifleProjectile_C, OwningCharacter) → LaunchFrom**([W109] 해결, 3.1절) — 발당 스폰 0.83 → 0
SoldierCharacter_ABP   ★ 09-21 게임 스레드 묶음 — 변수 **CVarPollSeconds**(float, DeltaTime 누적) · 함수 `Update_CVarDrivenVariables`(ABP 쪽 재선언 —
                         MCP create_node 가 GASP 원본 SandboxCharacter_CMC_ABP_C 의 동명 함수를 잡으므로 declaring_class 지정)
                         EventBlueprintInitializeAnimation 에서 1회 + Update 에서 CVarPollSeconds ≥ 1.0 마다(1 Hz). cvar 7 문자열 조회 + ComponentHasTag 2 가
                         매 틱이었다. Update_PropertiesFromCharacter · Update_Logic 불변. GT 이벤트 그래프 병사당 29 → 22 µs
AIC_Soldier            ⚠ StartLogic 노드 삭제 — 상속된 ST_Soldier_SmartObject 를 멈춘다
                         ★ 2026-09-30: 부모 AIController → **ASoldierAIController**(C++, 5.1절 — 컨트롤 회전에 피치, P198). MCP set_parent · 저장
BP_AR4Rifle            ★ 2026-09-30: StartReload 마지막 Delay 0.7 → 0.9 — 사격 금지 2.2 s = 재장전 몽타주 길이(P197). BP_AK47Rifle 은 상속.
                         ⚠ 재장전 몽타주 길이를 바꾸면 이 Delay 합도 같이
SK_UEFN_Mannequin      ★ 2026-09-30: 슬롯 그룹 — AdditiveHitReact 를 새 그룹 HitReact 로(사용자, Anim Slot Manager). 나머지 슬롯은 DefaultGroup (P197)
BP_ObserverPawn        ★ 2026-09-14 오후: 그래프 전부 삭제(38 노드). 옛 빙의는 ① Visibility 트레이스라
                          병사를 못 맞혔고 ② Possess 라 AI 를 멈췄다. 로직은 C++ ASoldierObserverPawn 으로.
                       ✅ 부모 = SoldierObserverPawn (빌드 후 set_parent, 디스크 확인 14:58)
                          (GM_SoldierObserver 의 GetDefaultPawnClassForController 가 이 BP 를 돌려주므로 BP 를 바꿨다)
                       플라이캠 이동은 DefaultPawn 엔진 정의 바인딩(WASD/QE/마우스) — FlyCam_* 는 titan 에 매핑이 없어 원래 죽어 있었다
GM_SoldierObserver     GetDefaultPawnClassForController 오버라이드 (P53 때문에 CDO를 못 쓴다)
SoldierCharacter_ABP   ★ 09-14 저녁 — 머리 추종 변수 5개 + Modify Bone 3개 (P53 이라 노드 프로퍼티는 사용자 수동)
                       변수  HeadAimRotation · NeckAimRotation · Neck2AimRotation (Rotator)
                             HeadAimAlpha (Float) · HeadAimLocation (Vector = 목 스트레치, 월드 이동)
                       체인  TwoBoneIK_0 → ModifyBone_6(neck_01) → ModifyBone_9(neck_02) → ModifyBone_7(head)
                             → ModifyBone_8(head, ~~전부 Ignore — 사용자 잔여, 통과 · 삭제 권고 [W54]~~ → ★ 09-15 **BF 포즈 head 1.15 상쇄용으로 사용 중**(scale Replace 1 · ComponentSpace, 2.5e-7) — 지우지 말 것, [W54] 철회) → ComponentToLocalSpace_2
                             세 노드 모두 Rotation = Add to Existing · World Space, Alpha ← HeadAimAlpha,
                             head 는 Translation 도 Add to Existing · World Space ← HeadAimLocation
                       ⚠ ModifyBone_9(neck_02) 는 2026-09-15 새벽까지 Bone None·Ignore 였다(P108 수동 누락,
                         컴파일러 경고 "You must pick a bone" 이 서명) — 사용자가 채움. 그전엔 굽힘이 neck_01 몫만
                       ⚠ Replace 로 두면 린 롤·블라인드파이어·걸음 흔들림이 전부 지워진다 (P105)
/Game/SoldierLab/Levels/L_SoldierTest      ★ 2026-09-14 재건
                       네비 영역  x −4000..7000 · y −4500..4500   = 110m × 90m
                       이전 19블록 시가지(액터 22개) 제거 → SoldierLab_Urban/ 5개 폴더
                         Screens   Screen_N · Screen_S            30m 차폐벽 2장
                         Defence   Bld_Def_N/S · Low_Def_A/B · Med_Def
                         Plaza     Crate_Plaza ×3 · Low_Plaza_A/B   중앙 약 25m 공지
                         Flanks    Bld_N1/N2 · Bld_S1/S2 · Low_N · Low_S
                         Attack    Bld_Atk_N/S · Med_Atk · Low_Atk/Atk2
                       SoldierLab_Test   적대 · 아군 · PlayerStart 전부 재배치
                       ✅ Objective_AllyBase (−2500, 0) Friendly — 목표 **1개** 배치됨
                          배치된 수비 엄폐가 전부 목표 비용 0.00~0.05 임을 확인 [A]
                       ⚠ 규모를 키운 이유: 교전 상한이 **정지 95m** 라
                          옛 레벨에서는 어디서 쓰든 항상 사거리 안이었다 → [C-97]
```

**AI→몸 다리의 모양 — 연속 축 전부 동일** (P71):

```
SetX( SelectFloat( A = RampAxisTo(X, clamp(AITarget), Rate, dt),
                   B = <기존 플레이어 식>,
                   bPickA = AIPoseDriven ) )
```

- **`Rate` 는 플레이어의 키 스텝이 쓰는 바로 그 변수**다 — AI가 사람보다 축을 빠르게
  움직일 수 없다
- 목표는 **`MapRangeClamped` 로 클램프**한다 — `RampAxisTo` 에 클램프가 없다

**Tick 접합 순서**:

```
SetAIPoseDriven(NOT IsPlayerControlled) → SetAITargetStance(Engagement) →
★ SetAITargetLean / SetAITargetBlindFireH / SetAITargetBlindFireV (Engagement) →   ← 09-14, [W19]
SetActualStance(StanceAxis) → <기존 갱신 체인> →
Branch(AIPoseDriven) → Branch(WantsToFire) → Shoot / else Branch(WantsToReload) → StartReload
```

> ★ **2026-09-21 후편** — 위 접합 순서는 그대로지만 **실행 주체가 C++ `USoldierAIBridgeComponent::TickBridge` 로 옮겨졌다**([W108], 5.1절). BP 그래프의 옛 본문은 `SoldierLab.AIBridge.Native 0` 일 때만 돈다. 순서·산술·변수 이름이 같으므로 이 절의 설명은 두 판 모두에 맞는다.

**조준 상태**: AI는 `WantsToAim` 을 **GASP 입력 상태 구조체**에 Break/Make 로 실어
`UpdateInputStateServer` 로 보낸다(다른 필드는 전부 보존). ★ **2026-09-14에 `WantsToSprint` 가 같은 경로로 붙었다** — 규칙이 아니라 **느릴 이유의 부재**다(P89). 프로젝트 자신의 기존 AI 경로
`STT_SetSoldierInputState` 를 읽어서 찾았다. ⚠ **"AI가 총을 안 들고 몸도 안 돈다"의 근본
원인**이 이것이었다 — `Enable_AO()` 가 `RotationMode == aim` 을 요구하고, 그것은
`S_PlayerInputState.WantsToAim` 에서 파생된다. 파생 플래그 `AOActive` 를 직접 만졌던
이전 패치는 **증상 처치**였으므로 되돌렸다 (P36).

### 5.3 남아 있는 P0-1 실험 자산 [B]

```
/Game/SoldierLab/AI/   ST_Soldier_Patrol_Subtree · ST_Soldier_SmartObject
                       STT_FocusToPlayer · STT_SetSoldierInputState
```

`ST_Soldier_SmartObject` 는 `ST_NPC_SandboxCharacter_SmartObject` 의 복제본이고
`Root/SmartObject/Patrol` 상태를 갖는다 — **병사들을 벤치로 걸어가게 하던 것**이다.
지금은 `StartLogic` 삭제로 **돌지 않는다.** 지우지는 않았다.
→ `ai/prototypes/2026-09-04_p0-1_ai_drives_mm.md`

### 5.4 ⬜ 여전히 초안만 [A]

`ai/drafts/`(인지·위협평가) · `cover/drafts/`(EQS/SmartObject) 의 C++ 초안은 **한 줄도 프로젝트에
들어가 있지 않다.** 컴파일된 적도 없다.

★ **`squad/drafts/`(L0 명령 · L1 분대)는 2026-09-17에 *일부* 구현됐다 — 단 초안과 다른 물건이다.**
승계한 것은 3.2절의 "제약을 주지 명령을 주지 않는다" 하나. 토큰·사기·조(Element)·`ASquadCoordinator`·
StateTree 노드·`FSoldierOrder`/`FOrderStatusReport` 스키마는 **채택하지 않았다**. 실물은
`Source/SoldierLab/Squad/` 3파일 + 5.1절 훅 → `squad/2026-09-17_command_layer_design.md`.

⚠ **인지와 엄폐는 초안과 *다른 물건*으로 구현됐다.** 이름이 겹치는 자리가 있으나
같은 타입이 아니다 — 초안을 읽고 코드를 예상하지 말 것.

---

## 6. 아직 없는 것 (착각 방지)

| 없는 것 | 비고 |
|---|---|
| ~~**피격 반응 · 사망 · 데미지 · 체력**~~ | ✅ **해결 (2026-09-15) · 실제로 화면에 나오기 시작한 것은 2026-09-17** — 배선은 09-15 에 끝났으나 **원인 3중**(C++ 경로 하드코딩 · 배치 인스턴스의 빈 배열 · 사망 슬롯 `FullBody` 부재)으로 한 번도 재생된 적이 없었다 → `ai/2026-09-17_hit_death_three_causes.md`. 지금은 피격 O · 사망은 **순수 래그돌**(몽타주 OFF). `USoldierHealthComponent` 하나(4절 · 0절 표). 클립 19개 전부 배선됨(HitReact 13 → `AdditiveHitReact` 슬롯, Death 6 → `DefaultSlot`). 래그돌은 GASP 함수를 부르지 않고 같은 일을 C++ 에서. 아군은 무적. 수치는 [C-110]~[C-118]. **옛 기술**: 클립 19개는 반입돼 있으나 **배선 없음.** 래그돌은 GASP에 이미 있다. ⚠ **AI 층이 생긴 지금 이것이 더 아프다 — 병사가 죽지 않으니 교전이 끝나지 않는다** → ~~[W18]~~ |
| 이동 중 급선회(spin) | Lyra에 클립이 없다. 회전 클립 + 비용편향으로 근사 중 |
| 대각선 이동 전용 클립 | Lyra 세트는 4방향뿐. 워핑으로 메운다 — 가끔 발이 꼬이는 원인 |
| ~~견착/총내림 의도 분기~~ | **✅ 해결 (2026-09-11)** — Chooser `RotationMode` 열 + Idles DB 분리. [C-55] |
| ~~웅크리기 전용 AO 배선~~ | **✅ 배선됨 — 단 이진이다 (2026-09-12)**. `Select(Stance == Crouch ? AO_Rifle_Crouch : AO_Rifle_ADS)`. DB 전환과 **같은 순간**에 갈려서 눈에 거슬리지 않는다. **연속 블렌드는 미구현** → **[W11]** |
| **캡슐/엄폐 높이가 여전히 이진** | stance 축이 **보이는 높이만** 연속으로 만든다. `Crouch()`가 문턱에서 86↔60을 한 번에 바꾸므로 **충돌과 엄폐 높이는 이진**이고, **AI의 엄폐 판단이 읽는 것이 바로 그 높이**다. 남은 조각 중 **AI 관점에서 가장 값어치 있고 위험도 가장 높다**(관통·계단 오르기·**일어설 때의 천장 스윕**은 CMC가 이진 경우에 대해서만 구현해 두었다) → **[W9]** · 안정성 판정은 **[C-3]** |
| **웅크림 카메라** | `CameraRig_CrouchOffset`은 Camera Pose 공간의 고정 `TranslationOffset (40, 0, −30)`이고 **블렌더블/데이터 파라미터가 없다** — 즉 **블렌드되는 이진**이라 float를 못 받는다. **결정: 리그를 고치지 않고 비활성화한 뒤 SpringArm Z를 stance 축으로 직접 몬다.** 미착수 → **[W10]** |
| **`IA_Crouch` 토글이 무력** | stance 축이 `bIsCrouched`를 매 프레임 소유한다. 제거하거나 "stance를 0/1로 명령하는 입력"으로 재정의할 것 → **[W13]** |
| ~~**적군 메시**~~ | ✅ **해결 (2026-09-14~15)** — `BP_Soldier_Hostile` = `new_enemy_T`(디자인팀 재납품 세 번째, UE5 표준 리그, Assign Skeleton 본 추가 0). 사용자 PIE "플레이 잘 됨, 총 붙음". 남은 것: `StanceStandZ/CrouchZ` 실측 **[C-119]** · 옛 에셋 삭제 [W29]. ~~`BP_Soldier_Hostile` 은 **마네킹 그대로**다. `Enemy` 에셋은 Mixamo 리그라 리스킨 전에는 못 올린다. 경로 결정 대기 → **[Q42]**~~, 3.2절 |
| **`StanceStandZ/CrouchZ` 자동 산출** | 메시별 실측값이라 캐릭터 메시가 올 때마다 HUD 역산으로 재야 한다 → **[W27]**. ★ 09-15: 적군 `new_enemy_T` 가 **아직 안 재어진 채** 돌고 있다 → **[C-119]** |
| **총내림 자세의 재저작** | 2026-09-13에 저작·배선했다가 걷기에서 어긋나 **원복**. `MM_Rifle_LowReady` 참조 0건으로 보존 → **[C-90]**. ★ **09-15 2차(로컬 애디티브)도 실패·원복.** 결론: 이동 중 총내림은 **총 내린 로코모션 클립 없이는 델타뿐이고 델타는 작을 때만 자연스럽다**(P123). 해법은 재저작이 아니라 **클립 요청**(Walk/Jog 루프 8장, 제대로는 32장) → **[W66]** · 3.2절 |
| **총내림 로코모션 클립** | 없다. 이동 중 총내림은 ADS 로코모션 + 힙파이어 델타(어깨 피치뿐)로 근사 중. 디자인팀 요청 시점 미정 → **[W66]** |
| **BF 포즈 3장 · `MM_Rifle_LowReady` 의 구워진 PP 스케일** | 마네킹 베이크 때 `ABP_UEFN_Mannequin_PostProcess` 의 head 1.15 · thigh 1.12 가 들어갔다(2.5e-7). head 는 `ModifyBone_8` 로 상쇄 중, **thigh 는 상쇄 안 함** [C]. 재베이크는 저작 시퀀서가 깨져 지금 불가 → **[W65]** · [Q48] |
| **`/MoverExamples/.../CR_Mannequin_Body` 컴파일 에러 · `/Game/NewLevelSequence`** | 엔진 플러그인 콘텐츠 리그가 "Setup Fingers uses [Bones] pins that no longer exist" · "Forward Spine has unmapped variables" 로 컴파일 실패. `/Game` 에서 이걸 참조하는 건 `/Game/NewLevelSequence`(2026-09-12 포즈 저작 스크래치, `L_SoldierTest` 가 참조 — [W38]) 하나. 제안: 시퀀스 + 레벨 액터 삭제 후 [W34] 의 Mover 계열 플러그인 끄기. **미결정** → **[Q48]** |
| **PIE 로그 `LogAbilitySystem: Error: SendGameplayEventToActor … GameplayEvent.ReloadDone`** | 병사마다 재장전마다 반복. 재장전 몽타주의 GASP 노티파이가 GAS 이벤트를 보내는데 우리 캐릭터에 ASC 가 없다. **무해.** 노티파이를 떼면 조용해진다 → **[W64]** |
| **`CHT_Soldier_CharacterAnimations` 실사용 여부** | GASP 비무장 chooser 복제본(UEFN 클립 387개 참조). `Animation/`(단수)에 그대로 있고 **진영별 `Enemy_CHT`/`ALLY_CHT` 와는 별개**다. ABP 가 참조만 하고 실사용 없음 [B] — 확정은 PIE **`a.AnimNode.MotionMatching.DebugDrawInfo 1`** 로 → **[C-120]** ⚠ 옛 문서들이 안내한 `DebugDrawInfoVerbose 1` 은 **단독으로는 아무것도 안 나온다**(P141) |
| **`AdditiveHitReact` 가 사격/재장전과 같은 슬롯 그룹** | `SK_UEFN_Mannequin` 의 슬롯 그룹은 `DefaultGroup` 하나뿐이고 슬롯 6개가 전부 거기 속한다. 피격은 `bStopAllMontages=false` 라 남을 안 끊지만 **`PlayAnimMontage` 는 기본값이 true** 라 사격·재장전이 **같은 그룹의 피격 몽타주를 정지시킨다**(`AnimInstance.h:626`). 2026-09-17 증상의 원인은 아니었으나 **구조적 위험은 그대로** — 정석은 별도 슬롯 그룹(Anim Slot Manager, **MCP 불가**) → **[W71]** |
| **피직스 에셋이 두 진영 다 마네킹 것** | 자동 생성본의 관절 제한이 기본값이라 무릎·팔꿈치가 반대로 꺾여 `PA_UEFN_Mannequin` 으로 교체했다(3.2절). 바디가 마네킹 체형이라 아군 181 cm · 적군 178.7 cm 에서 **캡슐이 메시와 약간 어긋난다.** 제한값 유지 + 바디만 피팅한 진영별 복제본이 장기안 → **[W72]** · 품질 정량 판정 **[C-127]** |
| **아군 세트의 프리뷰 메시** | 적군 세트 227개는 `new_enemy_T` 로 지정 완료. **아군 세트는 사용자가 스크립트 실행 예정** → **[C-129]**. `PreviewSkeletalMesh` 는 `EditAnywhere` 가 아니라 MCP 로 못 읽는다(`CLAUDE.md` 6.1) |
| **아군/적군 총기 분기 시 `WeaponMesh` · 소켓 규약** | `WeaponMesh` 의 메시 = 스폰 총 메시여야 하고(총구 보정 `Muzzle` · 그립 `LeftHandGrip` 소켓을 그 컴포넌트에서 읽는다) `weapon_r` 메시 소켓이 메시별이라 총이 갈라지면 소켓값도 갈라진다. 지금은 한 벌(`SK_AR4_X`)이라 문제 없음 → **[W67]** |
| **아군의 "마네킹 비율" 근사** | 사용자 요구: soldier_T 가 마네킹 뼈 길이로 움직이게. translation retargeting `Animation` 5분 시험 미실시 → **[W28]** |
| **투사체의 리플리케이션** | 이식하며 **3분기 Multicast 라우팅을 걷어냈다** — 세 핸들러가 전부 `PlayImpactEffect`로 되돌아왔으므로 방송할 대상이 없는 지금은 직접 호출과 같다. 멀티가 생기면 여기로 돌아온다 → **[W17]** |
| **진영(Faction) 판정 — 투사체만** | **정식 소스는 2026-09-13에 생겼다**(`USoldierIdentityComponent::Faction`, 5.1절). AI 층은 전부 그것을 읽는데 **투사체만 아직 `bHitEnemy = IsA<ACharacter>()` 대역**이다 → **[R7]** · 갈아끼우기 **[W23]** |
| **바람** | 나이아가라 `WindVectorCms` 파라미터에 **0을 먹인다.** 바람 소스가 생기면 한 줄 |
| **투사체 풀** | `LaunchFrom` / `Deactivate` 는 있는데 **풀이 없다** — 무기 컴포넌트가 없어 발당 `SpawnActor` 한다. 붙이는 것은 **쏘는 쪽의 변경**. ★ 09-21: `Deactivate` 가 `Destroy` 를 안 하므로 발마다 스폰된 투사체가 **영구 누적**(누수) — **[W109] 착수**(3.1절) |
| **무기 메시의 애니메이션(볼트·탄창)** | `SK_AR4_X` 에 애님 블루프린트가 없다. GASP `SK_Rifle`의 `ABP_Weap_Rifle`을 겨냥하던 `Montage_Play` 2개는 **삭제**했다(항상 None) → **P50**. 되살리려면 새 메시용 무기 ABP를 만들어야 한다 |
| **디버그 궤적 라인** | 안 보인다. 그 외에는 전부 동작한다 → **[C-81]** |
| **무기/투사체 튜닝값의 검증** | 전부 `titan_example` 에서 온 숫자다(도탄 3 · 데칼 200 · 휘즈 200/1500 · fireRate 0.12 …). 이 프로젝트에서 재본 적 없다 — P18·P30과 같은 계열의 위험 → **[C-82]** |
| ~~제압(suppression) 신호~~ | **✅ 해결 (2026-09-13)** — `ApplySuppressionAlongSegment()`가 휘즈와 같은 최근접 판정을 **병사 등록부**에 대해 한 번 더 한다. 그 과정에서 `PreviousLocationForWhiz` 가 휘즈 블록 **안**에서 갱신되던 버그를 고쳤다 → **[W16]** |
| **에셋 경로 정리** | `M_Decal_Bullet` · `M_RCWSRound` 가 **마이그레이션된 경로 그대로** 있다 → **[W14]** |
| ~~엄폐 · 인지~~ | **✅ 구현됨 (2026-09-13)** — 단 **초안과 다른 물건**이다. 5절 |
| **분대 · 명령** | 여전히 초안 단계. ★ **목표는 2026-09-14에 실제로 돌기 시작했다** — 레벨에 1개 배치 + 루트 컴포넌트 버그 수정(P90). 다만 여전히 **마커 하나 · 소유권 변경 없음 · 국면 전환 없음** → **[W25]** |
| ~~린 · 블라인드 파이어를 AI가 몰지 않음~~ | ✅ **해결 (2026-09-14)** — 교전 컴포넌트가 `GetDesiredLean` / `GetDesiredBlindFireH/V` 를 발행한다. ★ **새 판단 재료를 하나도 안 만들었다** — 조리개의 답이 **위 아니면 옆**이고 그것이 이미 있던 두 축이었다(P81). 칸을 고르는 것은 제압도 → ~~[W19]~~ · `ai/2026-09-14_exposure_ladder_and_corrections.md` 1절 |
| **경로의 노출(직선뿐) · 엄폐 후보의 다양성 · 소리 차폐** | 경로 항은 들어왔으나 **네브메시 경로가 아니라 직선 표본**이다(09-14에 **거리 스케일**이 붙었다). 후보는 09-14에 **두 겹 링**이 됐으나 여전히 **고정 반경 둘**이다. 각각 **[W20]** · **[W21]** · **[W22]** |
| ~~반동 누적이 없다~~ | ✅ **해결 (2026-09-14)** — `RecoilSpread` 가 `EffectiveSpreadAtCm` 의 곱셈 항으로 들어갔고, 발사는 **탄창이 줄는 것**으로 센다(새 훅 없음). 맹목사격은 상승률 2배 → ~~[W26]~~ · 값은 **[C-92]** |
| ~~**1인칭 카메라 · 플레이어가 조종하는 병사의 1인칭 모드**~~ | ✅ **2026-09-14 저녁 구현됨** — `Camera/SoldierFirstPersonComponent`(T) · 관전 폰도 T. 빙의(P94)는 폐기(Possess 가 AI 를 멈춤). 남은 것: [C-73]·[C-76]·[C-78] 을 1인칭에서 다시 판정 → [W30] 해결 표시 |
| **조리개 트레이스가 예산 밖** | `FindAperture` 가 틱당 최대 4발을 쓰는데 엄폐의 라운드로빈 예산에 없다. 디버그를 켜면 **한 번 더** 돌아 8발이 된다 → **[W32]** · [C-83] |
| ~~★★ **수비수가 정착해 쓰지 못한다**~~ | ✅ **해결 (2026-09-14~15)** — 넷째 원인은 **기준면**(P103), 그 뒤 로그 실측으로 예산 게이트·높이 상수·가치 게이트·경로 상한·활동도·노출 회계를 차례로 고쳤다. 사용자 평가 "지금까지는 가장 좋네". 수치 판정은 미완 → ~~[C-95]~~ · `ai/2026-09-15_exposure_cycle_and_muzzle_learning.md` 12절 |
| ~~★ **엄폐 자리 예약이 없다**~~ → **절반 (2026-09-16)** | 예약이 아니라 **주장 비용**(S1 `ClaimedCost`)과 **사선 비용**(S2 `LaneCost`)으로 갈랐다 — 겹칠 이유가 더 크면 겹친다(P89). 진짜 예약(점유 목록·양보 규칙)은 없고 겹침·`MASKED` 감소는 **미실측** → **[W51]** 절반 · [C-108] |
| ~~★ **분대 통신·화망이 없다**~~ → **절반 (2026-09-16)** | 통신은 **사망 보고**가 추가됐을 뿐(적 기록 공유는 전부터). 엄호/이동 분담은 S4(엄호받는 눈 ×0.5)로, 표적 분산은 S3로 — 전부 등록부 읽기이지 무전이 아니다. "사각 없는 위치로 옮겨 제압"·화망 구역 지정은 없음 → **[W52]** 절반 |
| ~~★ **L0 분대 명령 층이 없다**~~ → ~~코드 완료 · 빌드·PIE 미검증 (2026-09-17)~~ → **빌드 · 시험 레벨 첫 PIE (2026-09-18) — 완주 미확인 [C]** | `Squad/`(계약·분대 서브시스템·구역 액터) + 개인 AI 훅 3곳 + titan 브리지(`USoldierLabBridgeSubsystem`) + 시나리오 `IssueSquadOrder`. **적·아군 전원 SoldierLab 교체 결정.** 09-18: 시험 레벨 `L_SoldierScenario` + DT 13행 저작, 첫 PIE 수정 6건(도착선·존 표시·차량 제외·정원제·`EngageRangeCm`·RCWS 청각). 남은 것: 완주 · [C-122]~[C-126] · [C-144][C-145][C-147] · [Q50] 1차 전투지 위치 · [W84] `MinStance` · [W68]~[W70] · [Q49] → **[W55] 진행중**, `squad/2026-09-18_squad_layer_fixes_quota_engage_range.md` · `../level_new_kadex_0811/2026-09-18_soldierlab_three_stage_test_level.md`. **옛 기술**: "특정 위치로 경계하며 이동 · 점령 후 방어" 같은 **명령**이 없다. 있는 것은 목표 마커 하나(`ASoldierObjective`)와 위 선호들뿐 |
| **거리 의존 발견 시간이 없다** | 시야 120 m 안이면 즉시 본다. 멀수록 "알아채는 데" 오래 걸려야 → **[W56]** |
| ~~★ **사망·대가가 없다 (재강조)**~~ | ✅ **해결 (2026-09-15)** — 적군은 34 × 3발에 죽는다(머리 2발 · 팔 5발). 분대원 전사 시 반경 15 m 아군에 제압 임펄스 0.4×(1−d/r). ⚠ **아군은 무적이라 대가가 비대칭**이다 — 공격수(적군)만 줄어든다. **옛 기술**: 공터에서 30 s 맞아도 대가는 제압도뿐이라 공격수가 결국 수비수 코앞까지 걸어와 얼굴을 맞댄다 → ~~[W18]~~ |
| "노는 병사"의 원인 미확정 | 가끔 총을 안 쏘고 서 있는 병사. `[Engage]` 전이 로그의 게이트 0 항목으로 확정할 것 → **[W53]** |
| **`Cover` 채널(`GameTraceChannel4`)이 미사용** | 시야·엄폐·사선 셋 다 `GameTraceChannel5`("Sight")를 쓴다. [Q21]이 채널을 둘 판 이유가 아직 실현되지 않았다 → **[W24]** |
| **45명 규모에서의 AI 비용** | 인지·시야·엄폐가 전부 매 틱 트레이스를 쓴다(예산은 있다). 한 번도 안 재봤다 → **[C-83]** |
| 멀티플레이 검증 | 한 번도 안 했다. ★ 09-15: 체력 컴포넌트는 `Health/bDead/LastHit` 복제 + OnRep 연출로 **설계는** 돼 있으나 2프로세스 미검증. AI 컴포넌트의 P5 위반은 그대로 → **[W61]** · **[W62]** |
| ~~블라인드 파이어(맹목사격)~~ | **✅ 해결 (2026-09-12)** — 단 **IK가 아니라 저작 포즈 + 연속 애디티브**로 성립했다. 2.5e절. [C-8]의 "IK 품질 하한"이라는 질문 자체가 형태를 바꿨다 |
| **린(lean) 축의 문서** | 축 자체는 **구현돼 동작 중**인데(`ModifyBone spine_01..05` · `IA_Lean` · Q/E) 이 문서에 절이 없다. 2.4절 체인이 그 존재를 처음 기록했다 → **[W7]** |
| **블라인드 파이어 × 총구 보정의 상호작용** | BF 자세를 `AimCorrection` 적분기가 **"고쳐야 할 오차"로 학습**할 수 있다 [B]. BF를 켠 채 `AimErr`/`AimCorr`를 봐야 한다 → **[C-78]** |
| **마스크 규약 통일** | `BlendMask`(+`BM_LowReady`)와 `BranchFilter` **두 벌이 그래프에 공존**한다 → **[W8]** |
| **간헐적 무기 잠금의 원인** | 급선회 뒤 아주 가끔 총이 내려간 채 안 올라온다(`AimErr Y 99°` · `AimCorr 25` 포화). **포화 적분기의 두 번째 평형점**으로 설명되지만 **99° 를 만든 방아쇠는 미규명.** 안티 와인드업 후 재현 안 됨 → **[C-76]** |
| **극단 상방 조준 시 무기 공중제비** | **의도적 미해결(won't fix).** 프로젝트 범위 밖 — 수직 상방 조준은 전술 슈터에서 나오지 않는 자세다 → **[C-77]** |
| `AO_Blend_Curve` 내용 확인 | `BlendListByBool_0`의 `customBlendCurve`. **MCP로 키 값을 읽을 수 없다**(`FRichCurve` 미노출 · `unreal` 파이썬 모듈 차단). 최댓값 > 1.0이면 독립적인 오버슈트 원인 → **[R6]**, 에디터 수동 확인 |
| 총구 정렬 **계측 장치의 제거/게이트** | `BP_SoldierCharacter` Tick에 `DrawDebugCoordinateSystem`(분기 **밖** — 모든 병사에게 그려진다) + `PrintString` 3줄(`ERR`/`ACC`/`GATE`)이 **아직 물려 있다.** 출하 전에 **제거하거나 디버그 플래그 뒤로** 보낼 것. ⚠ **`GATE` 표시는 죽은 오차 크기 게이트를 찍고 있다** — 실제 게이트(`InRange(0..2.0)`)가 아니다. 완전히 죽은 잔해는 `Delta(control, actorRotation)` · `Lerp(Rotator) Alpha 0.15` · `ToString(Rotator)` ×2 · `GetControlRotation` ×1 → **[W6]**, `animation/prototypes/2026-09-11_muzzle_aim_alignment.md` 11절. **2026-09-12에 계측 축이 더 늘었다** — `BodyErr` · `WpnLow` · `WpnTgt` · `AimGain` · `AimGate` · `AimCorr` · `AimErr` · **`BF_H` · `BF_V`** · **stance 축의 7행(`Stance` · `PelvWZ` · `PelvTgt` · `PelvOff` · `Crouched` · `SpdCap` · `GaitClamp`)**. 그리고 **죽은 변수 3개**(`StanceDropMax` · `StanceRaiseMax` · `StanceBlendRate`)가 생겼다. ⚠ **단 stance 7행은 함부로 떼지 말 것** — 이 축의 문제는 **과도구간에만 존재**해서 저 행들이 없으면 진단이 불가능하다(**P44**) |

---

## 7. 이 문서를 고칠 때

- **에셋을 추가/변경하면 여기부터 고친다.** 실물과 어긋나면 이 문서는 해로워진다
- 숫자를 바꿀 때는 **왜 그 값인지**를 같이 적는다. 2.6절이 그 원칙으로 쓰였다
- 새로 알게 된 함정은 이 문서가 아니라 `CLAUDE.md` 5절(P번호)에 넣고 여기서는 참조만 한다
