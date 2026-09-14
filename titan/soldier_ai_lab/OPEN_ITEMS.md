# 미해결 항목 추적 — soldier_ai_lab

2026-09-03 / 진행중 / 모든 문서의 미해결 항목을 ID로 통합 추적. 새 항목을 만들면 여기 등록하고, 해결되면 원 문서에 결과를 쓴 뒤 여기에 해결 표시.

> **ID는 전역으로 유일하다.** 새 항목은 각 접두의 최대 번호 +1을 쓴다.
> 접두 의미는 `CLAUDE.md` 4절 참고.

---

## 요약

| 접두 | 뜻 | 열림 | 해결 |
|---|---|---|---|
| **C** | 측정해야 아는 것 (실험 필요) | 58 (중 1건은 **의도적 미해결**) | 21 |
| **D** | 설계 백로그 (해당 단계에서 채움) | 9 | 1 |
| **Q** | 사용자 결정 필요 | 4 | 10 |
| **R** | 추가 조사 | 5 | 2 |
| **U** | GASP 미확인 | 2 | 5 |
| **W** | 분석에서 파생된 작업 | 36 | 5 (+ **[W20]** · **[W21]** · **[W25]** 절반) |

---

## C — 측정해야 아는 것

문서: `animation/2026-09-02_pose_pipeline_spec.md` 10절 · `ai/2026-09-02_upper_layer_plan.md` 12절 ·
`animation/2026-09-02_gasp_abp_analysis.md` · **`ai/2026-09-13_perception_stack.md`** ·
**`ai/2026-09-13_engagement_and_cover.md`** · **`ai/2026-09-13_objective_and_position_cost.md`** ·
**`ai/2026-09-14_exposure_ladder_and_corrections.md`**

| # | 항목 | 판정 기준 | 시점 |
|---|---|---|---|
| ~~C-1~~ | ~~AI 구동 시 `TrajectoryGenerationData` 값~~ | **✅ 잠정 통과 [B] (2026-09-04)** — `STT_FocusToPlayer`로 플레이어를 조준시킨 채 순찰시키자 옆걸음·뒷걸음·급선회가 강제 발생했는데 **자세가 무너지지 않았다.** `TrajectoryGenerationData` 재튜닝 불필요 → **[W4] 해소**. ⚠ 단 GASP 비무장 DB는 8방향이 완비돼 있어 **데이터가 충분한 조건에서의 결과**다. 견착 DB로는 다시 봐야 한다 → [C-44] | — |
| **C-44** | **견착 DB(방향 커버리지 부족)에서도 급선회가 버티는가** | [C-1]은 8방향이 완비된 GASP 비무장 DB로 통과했다. 견착은 전방 위주 소수 클립뿐이라 **같은 조건이 아니다.** 이것이 [C-24](점진적 폴백)와 함께 P0-2의 본 판정 | **P0-2 ★** |
| **C-2** | 자세 높이 블렌드 구간(0.4~0.7) 품질 | 높이 스윕하며 걸을 때 발 미끄러짐/다리 늘어남. ★ **2026-09-12 갱신 — 이제 스윕할 축이 생겼다**(`StanceAxis`, `IMPLEMENTED.md` 2.5f). 파탄은 관측되지 않았고 `PelvisDrop` **−50**이 기립 포즈 기준 사용 한계였다("스쿼트하는 정도") [A]. 다만 **발 미끄러짐을 수치로 재지는 않았다** [C]. 알려진 성질: **0 ~ 문턱 구간에서는 기립 걸음 사이클을 골반만 내린 채** 걷는다(문턱 이후는 웅크림 Walk 클립). **중간 높이 클립이 없으므로** 설계상의 정상 동작이다. 개선안 → **중간 자세 3점 블렌드**(포즈 한 장 저작 · `CLAUDE.md` 6.3절 경로) | P1 |
| **C-3** | 캡슐 연속 구동 안정성 | 바닥 스냅/충돌 해소가 흔들리지 않는가. ★ **2026-09-12 갱신 — 아직 판정 불가.** 캡슐은 여전히 **이진**(문턱에서 86↔60 한 번에)이라 연속 구동을 한 적이 없다. **착수 항목은 [W9]이고 이 항목이 그 판정 기준**이다. ⚠ 미리 알아 둘 위험 셋: **관통** · **계단 오르기(step-up)** · **일어설 때의 천장 스윕** — 마지막 것은 CMC가 **이진 경우에 대해서만** 구현해 두었다 | P1 |
| **C-4** | `Phase` 커브 기반 DB 경계 위상 정합 | Stand↔Crouch DB 전환이 매끄러운가. ★ **2026-09-12 갱신 — 위상 정합 없이도 수용 가능했다** [B]. 문턱 0.5에서 `Crouch()`/`UnCrouch()`로 DB를 갈고 **인어셜라이즈에 맡겼는데 정지 상태에서는 문제가 없었다** — 문턱의 팝은 DB가 아니라 **골반 오프셋의 계단**이 원인이었다(`animation/prototypes/2026-09-12_continuous_stance_axis.md` 8.4절). **남은 확인: 이동 중 전환.** 재료는 있다 — ABP가 `Phase_History` · `Contact_L/R_History`를 든다 | P1 |
| ~~C-5~~ | ~~견착 워핑 한계각~~ | **✅ 해결(형태가 바뀜)** — 워핑은 **직선 루프만** 커버, 전환은 못 함. `animation/..._gasp_abp_analysis.md` 16.2절 | — |
| C-6 | `AimDeadzone` / `AimTwistMax` 실제 값 | 재정렬이 과하지도 부족하지도 않은가. **GASP 기준값 60° 확인됨**(14.5절) | P1 |
| C-7 | `LowReady` 전용 조준 오프셋 필요 여부 | ADS 세트 축소 적용으로 충분한가 | P1 |
| ~~C-8~~ | ~~맹목사격 IK 품질 하한~~ | **✅ 해결 (2026-09-12) — 질문의 형태가 바뀌었다.** 블라인드 파이어는 **IK가 아니라 저작 포즈 3장 + 연속 마스크 애디티브**로 성립했다. 순수 IK 안은 사용자가 기각했다 — **몸통/척추가 돌아가는 자세**가 필요한데 팔 IK 체인은 `upperarm→lowerarm→hand` 세 마디라 척추로 전파되지 않는다(설계이지 결함이 아니다). IK는 **왼손을 그립에 붙잡는 역할**로 축소됐고, `TwoBoneIK_0`이 그래프의 마지막 스켈레탈 컨트롤 노드라 **재작업 없이** 그 역할을 한다. "어느 각도부터 고무처럼 보이는가"는 저작 포즈에는 해당하지 않는 질문이다 → `animation/prototypes/2026-09-12_blind_fire_axis.md` · `IMPLEMENTED.md` 2.5e절 | — |
| C-9 | 뱅킹 lean + 전술 lean 합성 방식 | 동시 적용 시 과회전하지 않는가. ★ **2026-09-12 갱신: 이제 둘 다 존재한다** — 뱅킹은 `ApplyMeshSpaceAdditive_2`(`BS1D_Additive_Lean_Run`), 전술 린은 `LocalToComponentSpace_0` 뒤의 `ModifyBone spine_01..05`(Q/E 연속 축). **두 지점이 체인에서 멀리 떨어져 있어** 합성 결과를 실제로 봐야 한다 (`IMPLEMENTED.md` 2.4절) | P1 |
| C-10 | 교란 진폭 ↔ 탄착 오차 | 쓸 만한 크기의 오차가 나오는가 | P1 |
| C-11 | 45명 동시 Control Rig 비용 | 애님 예산 2.5ms 내인가 | P3 |
| C-12 | Intent 전환 권한 규칙 (L2/L3 경계) | 실행 중 표적 변경은 누구 권한인가 | A3 |
| C-13 | StateTree 인스턴스 비용 × 45명 | | A6 |
| C-14 | 유틸리티 커브 초기값 | 글래스박스 UI 선행 필요 | A3 |
| C-15 | 4~10Hz 판단 주기의 체감 반응성 | 굼떠 보이지 않는가 | A3 |
| C-16 | 분대 동적 생성 vs 사전 배정 | | A4 |
| C-17 | 토큰 수 ↔ 분대 규모 | | A4 |
| C-18 | 아군/적군 단일 코드 원칙 유지 여부 | | A3 |
| C-19 | 서버 권위 게이트 전 진입점 적용 | | A5 |
| ~~C-20~~ | ~~Orientation Warping이 MM 경로에 있는가~~ | **✅ 해결 — 있다.** 15.1절 | — |
| C-21 | LOD 티어 차이가 **전환 상황**에서 드러나는가 | 급선회·정지에서 Dense↔Sparse 차이 | P3 |
| ~~C-22~~ | ~~`AnimationBlendStackGraph` 내부 구성~~ | **✅ 해결 — 16절.** 워핑+Steering 2개 | — |
| C-23 | 나머지 중첩 Chooser 6개 구성 | Stand Idles 패턴이 동일한가 | 견착 DB 작성 시 |
| C-24 | **점진적 폴백 품질** — 견착 DB에 총내림 클립을 섞었을 때 | 어색하지 않은가. 15.3절 (나) | P0-2 |
| ~~C-25~~ | ~~`Enable_Warping` 커브 자동 생성 가능 여부~~ | **✅ 해결(2026-09-03) — GASP에 `AM_WarpingAlpha` 모디파이어가 이미 있다.** 자체 제작 불필요. `animation/prototypes/2026-09-03_enable_warping_curve_generation.md` | — |
| **C-26** | 리타깃 클립에서 `AM_WarpingAlpha` 임계값 **5°가 적절한가** | 직선 구간에서 커브가 깜빡이지 않는가 | **P0-4 ★** |
| **C-27** | 우리 클립에 걸었을 때 커브가 **전 구간 1이 아닌가** | 인플레이스 클립에 잘못 걸면 조용히 전부 1이 된다 | **P0-4 ★** |
| ~~C-28~~ | ~~`contact_l/r`의 실제 생성 경로~~ | **✅ 해결(2026-09-03) — 이진 사각파다.** 연속값인 `MotionExtractor`로는 만들 수 없다 → `AM_FootSpeed_*` 개명설 기각. `animation/prototypes/2026-09-03_gasp_curve_manifest.md` 9절 | — |
| ~~C-33~~ | ~~`contact_l/r` 생성 수단이 없다~~ | **✅ 해결(2026-09-04)** — C++ 에디터 모듈 `SoldierLabEditor` 신설, `UFootContactCurveModifier` 자작. 높이+속도 판정, **발별 자동 높이 보정**이 핵심. `animation/prototypes/2026-09-04_foot_contact_curve_modifier.md` | — |
| **C-35** | 리타깃이 **발 높이를 좌우 비대칭으로** 만든다 | 실측: `ball_l` 최저 2.79cm / `ball_r` 2.42cm, 평균 5.28 vs 6.13. 커브는 자동 보정으로 우회했지만 **발 IK와 접지 품질에는 여전히 영향**. 리타깃 포즈나 `FloorConstraintOp`로 잡을 수 있는지 | P1 |
| ~~C-36~~ | ~~`Turning_..._Anim`에 `contact_l/r`이 없다~~ | **✅ 해결(2026-09-04)** — 적용 완료. 우회전이라 오른발 축(87% 접지, 최고 4.27cm) / 왼발 스텝(73%, 6.40cm, 이동 2배)으로 물리적으로 정합 | — |
| ~~C-37~~ | ~~`enable_turninplacesteering` / `steeringtargettime` **생성 수단 없음**~~ | **✅ 확정 (2026-09-04)** — `/Game/Blueprints/AnimModifiers/`의 **18개를 전수 확인**했고 이 둘을 만드는 것은 **없다.** `contact_l/r`과 같은 상황(원본 저작물 또는 사내 툴). 분포: `steeringtargettime` 62클립 / `enable_turninplacesteering` 18클립. **어떻게 할지는 → [C-45]** | — |
| **C-38** | 발이 지면 근처에만 머무는 클립에서 **접지 판정이 속도 단일 조건**이 된다 | `Turning_...`에서 `below-Z`가 양발 100% — 자동 보정 임계가 전 구간을 통과시켜 높이 판정이 무력해졌다. 이번엔 결과가 맞았지만 애매한 클립에서 의심할 지점 | P1 |
| ~~C-41~~ | ~~CMC 캐릭터로 AI 스택을 옮겼을 때 도는가~~ | **✅ 해결 — 옮길 것이 없다.** `NPCLevel`에 `SandboxCharacter_CMC` 3기와 `SandboxCharacter_Mover` 2기가 **똑같은 `AIC_NPC_SmartObject_C` / `AutoPossessAI=PlacedInWorld`로 이미 나란히 배치**돼 있다. 남은 것은 Rewind Debugger에서 CMC 액터를 골라 `Pose Search` 트랙을 눈으로 확인하는 것뿐 | — |
| ~~C-39~~ | ~~AI가 채워야 할 축이 무엇인가~~ | **✅ 해결 — `S_PlayerInputState` 5개 불리언.** `wantsToWalk`(GASP가 쓰는 유일한 것) / `wantsToSprint` / `wantsToStrafe` / **`wantsToAim`** / **`wantsToCrouch`**. ABP는 이를 `rotationMode`(OrientToMovement/Strafe/**Aim**) · `stance`(Stand/**Crouch**) · `gait` · `movementDirection`(F/B/LL/LR/RL/RR)로 변환한다. **견착 모드가 이미 데이터 모델에 있다** → `ai/prototypes/2026-09-04_p0-1_ai_drives_mm.md` 4절 | — |
| ~~C-42~~ | ~~`STT_SetCharacterInputState`를 5개 축 전부 노출하도록 확장~~ | **✅ 완료(2026-09-04)** — `/Game/SoldierLab/AI/STT_SetSoldierInputState`. MCP `write_graph_dsl`로 작성, 인터페이스 디스패치 유지. `ai/prototypes/2026-09-04_p0-1_ai_drives_mm.md` 4.3절 | — |
| ~~C-43~~ | ~~새 태스크를 StateTree에 물려 **AI가 실제로 조준·앉기를 구동하는지**~~ | **✅ 해결(질문의 형태가 바뀜) (2026-09-13)** — 판단은 **StateTree가 아니라 컴포넌트 + Tick 접합**이 한다. `AIC_Soldier`의 `StartLogic` 노드를 **지워** 상속된 `ST_Soldier_SmartObject`(벤치로 걸어가 앉고 순찰하던 것)를 멈췄다. 설계의 "유틸리티(판단) + StateTree(실행) 2계층"은 **아직 그 형태로 만들어지지 않았다** — 지금 있는 것은 그보다 얇다. `ai/2026-09-13_ai_bridge_and_scene.md` 6절 | — |
| ~~C-40~~ | ~~`PSS_Relaxed_Loops`가 CMC인가 Mover인가~~ | **✅ 해결 — Mover 쪽이다.** CMC는 `CHT_PoseSearchDatabases`→Dense/Sparse/ExtremeSparse(`PSS_Default`, `M_Neutral_*`), Mover는 `CHT_..._Relaxed`(`PSS_Relaxed_Loops`, `M_Relaxed_*`). CMC로 가면 우리 PSD의 스키마·정규화세트 **두 줄만** 바꾸면 된다 | — |
| ~~C-26/C-27~~ | ~~`Enable_Warping` 값 품질 / 전 구간 1 여부~~ | **✅ 판정 완료** — 직선 클립 전 구간 1로 규약에 맞음. 회전 클립은 애초에 커브를 만들지 않는 것이 정답이었다(8.2절). `animation/prototypes/2026-09-04_p0-4_complete.md` | — |
| ~~C-34~~ | ~~클립 종류 ↔ 커브 세트 **매핑표 확정**~~ | **✅ 해결 (2026-09-04)** — 996클립 전수 실측. `animation/prototypes/2026-09-04_c34_clip_curve_mapping.md` **4절이 확정표**(8종류 × 모디파이어 7단계). 핵심은 표가 아니라 **"Epic의 데이터에는 일관된 표가 없다"** 는 사실 — 같은 "걸으며 90° 회전"이 커브 3/4/6종으로 갈린다. 복제하지 말고 **소비하는 쪽 기준으로 균일 적용**한다 | — |
| ~~C-45~~ | ~~스티어링 커브를 어떻게 채우는가~~ | **✅ 해결 (2026-09-08)** — ~~손 저작~~ → **`UTurnInPlaceCurvesModifier` 자작**(빌드·등록 확인). 스티어링 커브 2종 + PoseSearch 노티파이 2종을 기본값 Add→Apply로 끝낸다. ⚠ **09-05에 적었던 "게이트 0.5초 고정"은 틀렸다** — GASP 두 클립이 회전을 앞부분에서 끝내서 그렇게 보였을 뿐이다. 우리 클립은 0.417s가 회전 10% 지점이라 0.5s면 **돌기도 전에 꺼진다.** 모디파이어가 루트 yaw 프로파일에서 자동으로 잡는다 → `animation/prototypes/2026-09-08_turn_in_place_curves_modifier.md` | — |
| **C-45b** | GASP 회전 클립의 **yaw 프로파일** | 게이트 규칙이 "yaw 진행률 90%"인지 확정하려면 GASP `M_Relaxed_Stand_Turn_090_L`의 0.567s가 그 클립의 90% 지점인지 봐야 한다. 우리 클립에서는 **yaw 90%와 마지막 발 착지가 같은 프레임(41)** 으로 일치했다 [B]. 클립을 우리 폴더로 복제해 같은 모디파이어를 걸면 1분(원본 무손상) | P0-2 중 |
| ~~C-48~~ | ~~회전 클립의 PoseSearch 노티파이 2종 누락~~ | **✅ 해결 (2026-09-08)** — `UTurnInPlaceCurvesModifier`가 `Continuing Pose Cost Bias`(앞) + `Block Transition In`(뒤)을 자동 배치한다. 경계는 스티어링 게이트 시각을 따라간다. PoseSearch 플러그인에 링크하지 않고 `TSoftClassPtr` 경로 + 리플렉션으로 처리 | — |
| ~~C-31~~ | ~~루트 회전 축 = pelvis Y~~ | **❌ 판정 번복 (2026-09-08)** — 09-03의 근거 두 개가 **facing을 검증하지 않았다**("루트 전진"=위치만, "net yaw 88.8°"=회전량만). 실측하니 **루트가 진행 방향보다 56° 틀어져 있었고**(sd 8.3), 견착 골반 블레이드가 루트에 구워진 것이다. Orientation Warping이 그만큼 다리를 틀고 있었다. → **`SoldierRootFacingModifier` 신설**로 교정, 재실측 `mean 0.0 / sd 0.0`. `animation/prototypes/2026-09-08_root_facing_56deg.md` | — |
| **C-59** | **Epic의 비용 편향을 그대로 복제하면 희소한 세트에서 반대로 작동한다** | `Stand_Idles` **+0.10 패널티** / `Stand_TurnInPlace` **−0.20 할인**은 클립 227개를 전제한 값이다. idle이 1개인 우리 세트에서는 idle이 이길 수 없어 **0.5~1초마다 회전 클립으로 튀었다**(움찔 + 왼쪽 회전). 둘 다 0으로 놓아 해결. **나머지 DB들의 편향도 같은 눈으로 재검토할 것** → `../CLAUDE.md` P18 | P0-2 |
| ~~C-49~~ | ~~소규모 DB에서 MM 파라미터 재튜닝이 필요한가~~ | **✅ 불필요 (2026-09-09)** — `blendTime 0.5` / `poseReselectHistory 0.3` / `maxActiveBlends 4` **GASP 기본값으로 전부 원복했고 정상 동작한다.** 우회가 필요했던 진짜 원인은 `bForceRootLock`이었다. **"클립이 짧아서 Epic 상수가 안 맞는다"는 추정은 틀렸다** | — |
| ~~C-52~~ | ~~`WalkSpeeds`가 속도를 정하지 않고 상한으로만 작동한다~~ | **✅ 해결 (2026-09-09)** — 상한이 아니었다. **캐릭터의 기본 `gait`가 `Run`**이라 `walkSpeeds`는 **한 번도 쓰인 적이 없었다.** 실제로 쓰이던 건 `runSpeeds`(500)이고 Lyra Jog 클립은 **582.62 cm/s**로 저작돼 있어 발이 14% 느렸다. 클립의 `movedata_speed`를 실측(매핑표 5.0b의 호버법)해 `walkSpeeds=(291.31,262.18,218.48)` · `runSpeeds=(582.62,407.83,349.57)`로 교체하자 **미끄러짐이 사라졌다** → `animation/prototypes/2026-09-09_lyra_rifle_migration.md` 8.5절 | — |
| ~~C-51~~ | ~~idle ↔ walk 선택을 속도로 가르는 구조가 없다~~ | **✅ 해결 (2026-09-09)** — Lyra 61클립을 GASP `Dense` 구조대로 **PSD 16개**로 나누고 Chooser를 **3열(Stance·MovementState·Gait) × 16행**으로 지었다. 속도가 아니라 **`Gait` 열거형**으로 가르는 것이 GASP의 실제 방식이었다(`= Walk` / `≠ Walk`). 8절 | — |
| ~~C-58~~ | ~~옆·뒤·앜기 속도는 GASP 비율로 추정한 값~~ | **✅ 해결 (2026-09-11)** — 실측 결과 **Lyra 라이플 세트는 전 방향 단일 속도**였다(Walk 291.31 / Jog 582.62 / Crouch 291.31, 방향 간 오차 0.001). GASP 비율(1:0.9:0.75 / 1:0.7:0.6)을 옮겨온 탓에 옆 −30% · 뒤 −40% 어긋나 **Loop가 영영 선택되지 않고 Start/Stop이 매 걸음 재선택됐다.** 세 백터를 모두 균일하게 고쳐 해결 → `animation/prototypes/2026-09-09_lyra_rifle_migration.md` 9절 · `../CLAUDE.md` P30·P31 | — |
| **C-60** | **`sprintSpeeds` 700이 재생속도 클램프 밖이다** | 700 ÷ Jog 클립 582.62 = **1.20×** 인데 MM 노드의 `playRate` 상한은 1.15다. Lyra에 스프린트 클립이 없어 Sprint도 Jog DB를 쓴다. [C-58]과 **같은 종류의 어긋남이 남아 있다.** **670**(=1.15×)이면 범위 안. 게임플레이 값이라 사용자 판단 대기 | P0-2 |
| **C-61** | **방향별 이동 속도차를 되살릴 것인가** | [C-58] 수정으로 뒷걸음질이 전진과 같은 속도가 됐다. 애니메이션에는 이게 정답이지만 전술 슈터로서는 어색할 수 있다. ① `playRate` 하한 확대(0.6배속은 슬로모션처럼 보임) ② 느린 Loop 리타임 추가(품질 최상, 작업 필요) ③ 유지. **AI 붙이기 전까지는 ③ 권장** | 나중 |
| **C-55** | **견착(`Idle_ADS`)과 총내림(`Idle_Hipfire`)을 의도로 분기** | 지금은 둘 다 `Stand_Idles`에 넣으면 MM이 **포즈 유사도로** 고른다(그래서 임시로 ADS만 남겼다). DB를 나누고 Chooser에 `RotationMode` 열을 추가해야 한다. 사용자 목표("총 내림/견착/조준이 다 반영")의 핵심 | **P0-2 ★** |
| **C-57** | 회전 클립의 **64%가 회전 후 정지 구간** | `TurnLeft_90`은 1.667s인데 회전이 0.600s에 끝난다. 진입은 차단돼 있어 지금은 무해하나, 재발하면 PSD의 **클립별 샘플링 구간**을 `[0, 0.7s]`로 잘라 후보에서 뺀다 | 필요 시 |
| **C-50** | **레벨 인스턴스 프로퍼티 오버라이드 점검** | 배치된 `BP_SoldierCharacter`가 `WalkSpeeds = 200`을 자체 보유해 **CDO 변경 4회가 전부 무시**됐고 그 사이 측정이 오염됐다(2026-09-09). 다른 프로퍼티에도 같은 오버라이드가 있는지 레벨에서 확인해야 한다. **에셋만 봐서는 안 보인다** | **P0-2 ★** |
| ~~C-31~~(2) | ~~루트 facing 교정값 56°~~ | **정정 (2026-09-09)** — 진단 공식이 90° 틀려 있었다(`Atan2(Y,X)` vs 엔진의 `Atan2(-X,Y)`). GASP 대조군이 **−90.0° / sd 0.0**을 뱉어 발각됐다. **실제 오차는 34°**이고, 56° 기준의 교정은 오히려 90°로 키워 캐릭터가 옆으로 걸었다. 공식 통일로 해결 | — |
| **C-49** | 소규모 DB에서 **MM 파라미터 재튜닝이 필요한가** | `poseReselectHistory 0.3` · `blendTime 0.5` · `maxActiveBlends 4` 는 Epic이 **227클립 DB를 전제로** 튜닝한 값이다. 클립이 1~3개면 정지 중 **0.4~0.5초 주기 버벅거림**(재선택 금지 0.3s + 블렌드 0.5s)과 스텝 꼬임(위상 다른 4클립 평균)을 만든다. **파라미터를 고칠 문제인가, 데이터를 채울 문제인가** | P0-2 |
| **C-46** | ★★ `AM_Copy_IKFootRoot` — **Orientation Warping의 전제 조건이었다** | 09-04엔 "발 IK가 어긋날 수 있다" 정도로 적었으나, 09-08에 워핑 노드를 실측하니 **`iKFootRootBone = ik_foot_root`, `iKFootBones = [ik_foot_l, ik_foot_r]`** 이다. 워핑은 **IK 발 본을 회전시켜** 작동한다(설계 5.5.2절). 우리 클립에 `ik_foot_*`가 안 채워져 있으면 **워핑이 회전시킬 대상이 없다** → **실험 1의 "다리 질질"의 유력한 원인.** 세 클립에 적용해 재시험할 것 | **P0-2 ★ 최우선** |
| **C-47** | `disable_ao` / `disable_additiveleans` 생성기 없음 | 각각 8개 클립 전용(Idle Break / `Walk_Turn`·`Spin`). 조준 오프셋·애디티브 리인을 구간별로 끄는 게이트. 소수라 손 저작으로 충분할 것 [B] | P1 |
| **C-29** | `enable_strafewarping` 커브의 역할 | Walk 클립 4개에만 있다. 스트레이프 전용 워핑 게이트로 보임 **(추정)** — 견착 스트레이프 DB에 필요할 수 있다 | P0-2 |
| **C-30** | 루트모션 생성 경로 — **A) 리타깃 Root Motion op** vs **B) `EncodeRootBoneModifier`** | **보류.** A의 `bRotateWithPelvis=true`로 회전 우려가 해소될 수 있어 A가 탈락한 것은 아니다. 현재 B로 진행(A에 [C-32] 버그가 있어서). 판정은 turn/pivot 클립 확보 후. `animation/prototypes/2026-09-03_mixamo_retarget_setup.md` 3절 | **P0-4** |
| **C-31** | `EncodeRootBone`의 **회전 설정** — 어느 본의 어느 축을 써야 루트 헤딩이 맞는가 | 현재 비워둠(회전 없음). `Yaw=Atan2(-Heading.X, Heading.Y)`라 축을 틀리면 옆을 보고 걷는다. **직선 클립으로는 검증 불가** | **P0-4** |
| **C-32** | ★ 리타깃 **Root Motion op을 켜면 타깃 골반이 바닥에 붙는다** — 원인 미규명 | `bPropagateToNonRetargetedChildren`을 꺼도 재현(가설 기각). 현재 op을 끄는 것으로 우회 중. **A안 채택의 전제** | **P0-4** |
| ~~C-72~~ | ~~`OffsetRootBone` 노드의 `maxRotationError` — 회전 오프셋 상한~~ | **✅ 해결 (2026-09-11)** — **`−1`(상한 없음) → `90`**. PIE 확인: 급선회에서 **뒤집힘이 전혀 없고**, 제자리회전 클립이 정상 재생되며, 상체가 빠른 조준을 따라간다 [A]. ⚠ 부수 정정: "다리가 느린 건 몸 회전 속도가 낮아서"는 **틀렸다** — `SandboxCharacter_CMC.UpdateRotation_PreCMC`가 접지 시 `RotationRate = (0, −1, 0)`을 넣고 **음수는 "즉시"**를 뜻한다(`CharacterMovementComponent.cpp:6595-6599`). 캡슐은 한 프레임에 따라잡고, 보이는 지연은 `OffsetRootBone`이다. ⚠⚠ **이 서술은 2026-09-12부터 현행이 아니다** — `UpdateBodyYawRate`가 부모의 `−1`을 매 틱 덮어써 **유한 각속도(90~720°/s)** 를 쓴다(`IMPLEMENTED.md` 2.5d). ⚠ 이 값은 **`Enable_AO` 문턱과 결합**돼 있다 → **[C-74]**. → `animation/prototypes/2026-09-11_sharp_turn_while_aiming.md` · `animation/prototypes/2026-09-12_body_yaw_rate_and_aim_antiwindup.md` | — |
| **C-73** | **`AO_Rifle_ADS`의 pitch 표본이 3장뿐**(−90 / 0 / +90) | 사이가 선형 보간이라 **중간 각도에서 총구 방향 오차**가 생긴다(정면 정지 실측 pitch −11.4° · yaw +10.1°). 지금은 되먹임 보정 루프가 흡수하고 있으나 **미세 진동이 남으면 표본을 늘려야 한다.** **Lyra 원본도 3장**이므로 추가는 저작 작업이다. [B] → 같은 문서 1절 | 필요 시 |
| **C-74** | **`maxRotationError 90` ↔ `Enable_AO 문턱 70`이 결합된 쌍인데 둘 다 감으로 잡혔다** | `maxRotationError`는 AimOffset 입력의 **천장**이고 `Enable_AO` 문턱은 그 안의 **결정선**이다. 문턱이 천장 이상이면 **영원히 발동하지 않는다**(보정 ±25°를 감안한 실질 상한 약 115° [B]) — GASP 원본의 115/180이 그래서 사실상 무력하다. 지금 90/70은 PIE에서 눈으로 정한 값이다. **측정할 것**: `AO_Rifle_ADS`가 실제로 파탄나기 시작하는 각도(→ 천장), 그 안에서 총을 내리는 게 자연스러운 각도(→ 문턱). **한쪽만 바꾸지 말 것.** ★ **2026-09-12 확장**: 결합 쌍이 **3개조가 됐다** — `maxRotationError 90`(천장) / `Enable_AO 70`(포즈 결정선) / **`WeaponLowerAngleFull 65`**(각속도 해제선). 셋 다 감으로 잡힌 값이고 순서(65 < 70 < 90)에 의미가 있다 → `animation/prototypes/2026-09-11_sharp_turn_while_aiming.md` 5절 · `animation/prototypes/2026-09-12_body_yaw_rate_and_aim_antiwindup.md` 6절 | P0-2 |
| **C-75** | **AI 병사에서 총구 보정 속도 게이트가 실제로 도는가** | `BP_SoldierCharacter` Tick의 실행 순서상 `SetAimCorrection`은 `IsLocallyControlled` Branch **앞**이지만 **`SetPrevAimRot`은 Branch 안**이다 [A]. 그러면 AI는 `PrevAimRot`이 초기값에 고정돼 `Delta(ControlRotation, PrevAimRot)`이 늘 크고, **게이트가 계속 닫혀 누적 보정이 얼어붙는다** [B] — PIE 미확인. **판정**: AI 병사를 조준시키며 총구 오차를 찍어 본다. 고치는 법은 `SetPrevAimRot`을 Branch 앞으로 옮기는 것. ★ **2026-09-12 갱신 — 인접한 다른 AI 버그 하나는 고쳤다(이 항목은 아니다).** 두 호출부(`UpdateBodyYawRate` · Tick의 조준 루프)가 **`GetControlRotation(GetController())`** 를 쓰고 있어 `GetController()`가 null이면 조준 방향이 **`(0,0,0)`으로 읽혔다**(로그의 `Accessed None trying to read CallFunc_GetController_ReturnValue`). **AI 45명에게 치명적**이라 GASP 패턴 **`SelectRotator(GetControlRotation[Pawn], GetBaseAimRotation, IsLocallyControlled)`** 로 교체했다. **`PrevAimRot`의 분기 위치 문제는 그대로 남아 있다** → `animation/prototypes/2026-09-12_continuous_stance_axis.md` 10절 | P0-1 |
| **C-76** | **간헐적 무기 잠금 — 총이 내려간 채 영영 안 올라온다** | 급선회 뒤 **아주 가끔** 발생. 관측 상태: `BodyErr 0.000` · `WpnLow 0.000` · `WpnTgt 0.000` · `AimGain 0.050` · `AimGate true` · **`AimCorr P 25.000 Y 25.000`**(클램프 포화) · **`AimErr P 73.265 Y 99.063`**. 시각적으로 **소총이 등 뒤에 있고 양팔이 들려 있다.** 시간이 지나도 안 풀리고 **마우스를 움직여야만** 빠져나온다. **분석 [B]**: 포화 적분기의 **두 번째 안정 평형점** — 99° 오차를 ±25° 권한으로는 줄일 수 없어 적분값이 클램프에 붙은 채 정지한다(→ **P40**). **미규명인 것은 "총구를 처음 99° 틀어 놓는 방아쇠"** 다. 2026-09-12 안티 와인드업 이후 재현되지 않았으나 **근본 원인이 증명되지 않았으므로 닫지 않는다.** **판정**: 재현 절차를 찾고, 못 찾으면 `AimErr > 30°`가 N프레임 지속될 때 `AimCorrection`을 0으로 리셋하는 탈출 경로를 넣고 재관찰. 스크린샷 `C:\Users\insung52\Pictures\Screenshots\스크린샷 2026-09-12 100104.png` → `animation/prototypes/2026-09-12_body_yaw_rate_and_aim_antiwindup.md` 13.1절 | P0-2 |
| **C-77** | **극단적 상방 조준 시 무기 공중제비** — ❌ **의도적 미해결(won't fix)** | 거의 수직으로 위를 보면 무기가 뒤집히며 돈다. 짐벌 극점 부근에서 `AO_Rifle_ADS`의 pitch 표본 3장(−90/0/+90)이 보간을 감당하지 못하는 것으로 보임 **(추정)**. **사용자 결정: 프로젝트 범위 밖, 고치지 않는다** — 전술 슈터에서 수직 상방 조준은 실제로 나오지 않는 자세이고, 고치려면 AO 표본 저작([C-73])이 필요하다. **재검토 조건**: 상방 교전(건물 위/헬기)이 요구사항에 들어오면 다시 연다 → 같은 문서 13.2절 | — (won't fix) |
| **C-78** | **블라인드 파이어와 총구 정렬 보정이 서로 싸우는가** | BF 레이어는 `ApplyMeshSpaceAdditive_0`(조준 애디티브) **위**에 얹히므로 총구가 조준선에서 크게 벗어난다. 그런데 `AimCorrection` 적분기는 **총구 전방벡터**로 오차를 재므로 **BF 자세를 "고쳐야 할 오차"로 학습**할 수 있고, 그러면 BF를 켤 때마다 보정이 **±25° 클램프로 밀린다** — **P40**의 두 번째 안정 평형점, [C-76]과 같은 기구다 [B]. **판정 기준**: BF를 켠 채 화면의 `AimErr` · `AimCorr`를 본다. `AimCorr`가 양축 25.000에 붙으면 확정. **대응안**: `AimGain` 게이트(현재 `AOActive`)에 **BF 알파 조건**을 AND로 추가한다 — 액추에이터 결합 여부로 거는 형태라 **P37**과 정합한다 → `animation/prototypes/2026-09-12_blind_fire_axis.md` 15절 · `IMPLEMENTED.md` 2.5e-6 | P0-2 |
| **C-79** | **웅크림 문턱 직후의 "움찔"이 왜 멈췄는지 증명되지 않았다** | 증상: 제자리 · **조준 안 함** · 천천히 웅크리는 중 · stance **0.35~0.5** 구간에서 약 1초에 한 번 움찔하고 **몸이 실제로 돈다**. 조준/기립/이동하면 사라진다. MM 오버레이 판정 — 검색 대상 DB는 `PSD_Rifle_Crouch_Idles` + `PSD_Rifle_Crouch_TurnInPlace` **둘뿐**(LowReady idle DB 혼입 가설은 **기각**), Blend Stack에 **`MM_Rifle_Crouch_TurnLeft_90`이 두 벌**(time 0.48 / 1.57) = **약 1.1초마다 자기 재트리거**. `ShouldTurnInPlace`는 `\|Delta(조준방향, 몸방향)\| ≥ 50°`에서 발동한다. **해결: `StanceThreshold` 0.35 → 0.5** (사용자 제안, *"움찔구간 완벽히 사라짐"*). **기구는 [B]** — "0.35면 웅크림 클립보다 골반을 약 33유닛, 0.5면 약 25유닛 들어올려야 하므로 다리·발 심기에 무리가 덜 간다"가 그럴듯하지만 **재트리거를 멈춘 경로는 추적하지 않았다.** ⚠ 앞선 교정은 살아 있고 원인이 **아니다**(`Crouch_TurnInPlace.baseCostBias 0` · `Crouch_Idles.loopingCostBias −0.10`). **재발 시 볼 레버 ①**: **`Crouch_TurnInPlace.continuingPoseCostBias = −0.01` 인데 `Stand_TurnInPlace`는 −0.05** — 연속 포즈 할인이 작을수록 재선택이 쉬워지므로 증상 방향과 일치하는 **남은 비대칭**이다. **레버 ②** → **[C-80]** → `animation/prototypes/2026-09-12_continuous_stance_axis.md` 9절 | P0-2 |
| **C-80** | **`OffsetRootBone.maxRotationError = 90`이 90° 제자리회전의 각 해소를 막는가 — 자초한 회귀 후보** | 2026-09-11에 급선회 포즈 뒤집힘 대책으로 **−1(상한 없음) → 90**으로 바꿨다([C-72]). 그런데 **90° 클램프가 걸려 있으면 90° 제자리회전 클립이 자기 각을 끝까지 해소하지 못할 수 있고**, 그러면 회전이 끝나지 않아 **계속 재트리거**된다 — [C-79]의 증상과 정확히 맞물린다. **판정**: [C-79]가 재발하면 `maxRotationError`를 키우거나(120~180) 잠시 −1로 되돌려 증상이 사라지는지 본다. ⚠⚠ **이 값은 [C-74]의 결합 3개조**(`maxRotationError 90` 천장 / `Enable_AO 70` 결정선 / `WeaponLowerAngleFull 65` 각속도 해제선)**의 천장이다. 한쪽만 바꾸지 말 것** → `animation/prototypes/2026-09-12_continuous_stance_axis.md` 9.3절 | P0-2 |
| **C-81** | **디버그 궤적 라인이 보이지 않는다** | 투사체 이식분 중 **이것만 안 나온다** — 탄도·명중·도탄·데칼·휘즈는 전부 동작 확인됐다. **판정 기준**: 발사 시 궤적 선이 그려지는가. 볼 곳: 그리기 자체가 안 도는가(플래그/분기) vs 그려지는데 안 보이는가(수명 0·두께·뎁스). ⚠ 이 프로젝트가 여러 번 당한 "조용한 0" 유형일 수 있다 → `weapons/2026-09-12_projectile_port.md` 7절 | P0-2 |
| **C-82** | **이식한 무기/투사체 튜닝값이 이 프로젝트에서도 맞는가** | 도탄 캡 3 · `MaxActiveImpactDecals` 200 · `WhizDetectionRadiusCm` 200 / `WhizBroadPhaseRadiusCm` 1500 · `fireRate` 0.12 · `muzzleVelocity` 80000 · `tracerInterval` 3 · `magSize` 30 · `bulletSpreadDegrees` 3 — **전부 `titan_example` 에서 온 숫자**이고 여기서 재본 적이 없다. **P18·P30과 같은 계열의 위험** — 남의 데이터에 대해서만 옳은 값일 수 있다. **판정 기준**: 45명 규모에서 데칼 상한·휘즈 광역 컬링의 비용, 그리고 `bulletSpreadDegrees 3`이 우리 교전거리에서 만드는 탄착군 → `weapons/2026-09-12_projectile_port.md` 6.2절 | P0-2 |
| **C-83** | **45명 규모에서 AI 스택의 비용** | 인지·시야·엄폐 셋이 **전부 매 틱 트레이스를 쓴다**(예산은 있다: 시야 3 / 엄폐 4). 45명 × 3진영 순회 × 라운드로빈이 애님 예산과 어떻게 겹치는지 **한 번도 안 재봤다.** 판정: 설계 14절의 60fps 기준 | **성능 단계 (P3)** |
| **C-84** | **제압 상수가 플레이에서 맞는 느낌인가** | 상수의 *귀결*은 계산으로 확인했다 — 1m 한 발 **+0.22 / 0.9초 해소**, 8.3발/s 지속사격 **0.49초 포화**, 4m에서는 **누적 안 됨**, 손익분기 **354cm**. 맞는 것은 산수이고, **그 숫자가 옳은 숫자인지는 미측정.** `ai/2026-09-13_perception_stack.md` 7.3절 | AI 튜닝 |
| **C-85** | **인지 상수는 측정이 아니라 튜닝값이다** | `UncertaintyGrowthCms 200`(이탈률) · 반감기 시야 20 / 소리 8 · `SoundRadiusPerDistance 0.25`. 이 중 **거리↔반경만 기하에서 나오고 나머지는 감**이다. 반감기는 "무언가 바뀌었는데 못 봤다"의 위험률이라 **원리상 실측 대상이 아니다** — 대신 *귀결*(몇 초 뒤 잊는가)로 판정할 것 | AI 튜닝 |
| **C-86** | **교전 거리 4종이 플레이에서 그대로 나오는가** | 계산값 **정지 95m / 걷기 57m / 조깅 32m / 질주 24m**(질주는 속도계수 상한 4.0에 걸린 값). 스태미나 시스템은 관여하지 않는다. **PIE에서 재본 적 없다.** `ai/2026-09-13_engagement_and_cover.md` 4.1절 | AI 튜닝 |
| **C-87** | **교전·엄폐 튜닝값 전체** | `SuppressiveRadiusCm 500` · `LaneToleranceCm 200` · `FriendlyClearanceCm 150` · `MinCertaintyToEngage 0.25` · `SuppressiveReserveFraction 0.4` · ~~`MoveImprovementMargin 0.35`~~(→ 0.25, **[C-88]**) · 엄폐 링 `1200cm × 12` · `HeightSamples 3`. **C-82와 같은 계열의 위험**(P18·P30) | AI 튜닝 |
| **C-88** | **목표·위치 비용 층의 값과 그 거동** ⚠ **아래 값들은 2026-09-14에 대부분 바뀌었다 → [C-94]** | `ObjectiveWeight 1.2` · `RouteRiskWeight 1.0` · `RouteSamples 3` · `MaxTracesPerTick 6` · `MoveImprovementMargin 0.25` · `RadiusCm 900` · `DefendBandCm 900` · `ApproachScaleCm 6000`. **하나도 재지 않았다.** 판정 기준은 값이 아니라 **귀결** — ① 수비수가 반경 안에서 *흩어져* 자리를 잡는가(중심점으로 붕괴하지 않는가) ② 공격수의 정면 경로가 비싸질 때 **측면이 남는가**(`..._objective_and_position_cost.md` 4절, 지금 **[B]**) ③ `ObjectiveWeight` 를 올렸을 때 "용감"이 "자살"로 넘어가는 지점. ⚠ **[W25]가 먼저다** — 레벨에 목표가 0개라 지금은 관찰 자체가 불가능하다 | AI 튜닝 |
| **C-89** | **Lyra 클립이 `weapon_r` 본 자체를 움직이는가** | 아군 `soldier_T` 의 `weapon_r` 은 본이 아니라 `hand_r` 에 고정된 **소켓**이라(`IMPLEMENTED.md` 2.5 · 3.2), 클립이 `weapon_r` 을 움직인다면(재장전 때 총이 손 안에서 기울기 등) 아군에서는 그 움직임이 안 나온다. 마네킹 쪽 소켓은 본에 붙어 있어 그대로다. **판정**: 재장전을 마네킹과 아군 나란히 PIE. 눈에 띄면 Blender 로 아군 메시에 본을 넣는 것을 검토(`animation/prototypes/2026-09-13_ally_mesh_on_mannequin_skeleton.md` 6절) | P0-2 |
| **C-90** | **저작한 총내림 포즈 `MM_Rifle_LowReady` 를 살릴 수 있는가** | 2026-09-13 저작·배선 후 **걷기에서 왼손·상체가 ADS 로코모션과 어긋나 기각, 원복**. 총내림 idle 베이스(`Idle_Hipfire`)와 이동 베이스(ADS 루프)가 달라서 애디티브 한 장으로는 둘을 동시에 만족시키기 어렵다 [B]. 에셋은 애디티브 설정이 들어간 채 참조 0건으로 보존. **판정 기준**: 총내림 전용 로코모션 클립 확보 여부, 또는 idle 베이스를 ADS 로 통일(Chooser 17행)했을 때의 걷기 품질 | P1 |
| **C-91** | **노출 사다리(조리개·자세) 상수가 플레이에서 맞는가** | `LateralReachCm 70` · `SuppressionToGoBlind 0.45` · `BlindFireCloseRangeCm 400` · `LeanSpreadScale 1.4` · `BlindSpreadScale 4.0`. **판정 기준**: ① 낮은 담 뒤의 병사가 *일어서서 쏘는 것* 과 *총만 올리는 것* 사이를 **제압도에 따라** 오가는가 ② 모서리에서 린이 실제로 나오는가 ③ 4m 안의 적에게 총만 내미는 것이 우스워 보이지 않는가. ⚠ 좌우 조리개는 **액터 right vector** 로 옆을 정한다 — **병사가 위협을 향하고 있다는 전제**이고, 조준 선회 중에는 성립하지 않을 수 있다(지금은 `Traversing` 이 그 구간의 사격을 막아 증상이 안 나온다 **(추정)**). `ai/2026-09-14_exposure_ladder_and_corrections.md` 1절 | AI 관찰 |
| **C-92** | **반동 상수 — 계산상의 귀결조차 아직 안 냈다** | `RecoilPerShot 0.18` · `RecoilRecoveryPerSecond 1.2` · `MaxRecoilSpreadScale 2.5` · `BlindRecoilScale 2.0`. **판정 기준**: 지속 사격 몇 발에서 상한에 닿는가, 그때 교전 상한(2.2절의 95/57/32/24m)이 얼마로 줄어드는가, 그리고 **손 떼고 몇 초면 회복되는가**. 이 셋은 **책상에서 계산 가능하다** — [C-86]처럼 계산부터 해 두고 플레이로 넘길 것. [W26]의 해결분 | AI 관찰 |
| **C-93** | **조준 선회 상수 — 240 °/s 가 굼뜬가 민첩한가** | `AimSlewDegreesPerSecond 240`(180° 선회에 0.75초) · `OnTargetConeRatio 1.0`. **판정 기준**: 측면에서 나타난 적에게 반응하는 동안 **`Traversing` 에 머무는 시간이 견딜 만한가.** ⚠ 이 값은 **시야 콘도 함께 제한한다**(P86) — 올리면 조준만 빨라지는 것이 아니라 **눈도 빨라진다.** 짝: [C-75](총구 보정 속도 게이트) | AI 관찰 |
| **C-94** | **위치 비용 재교정값 전체** | `NoCoverCost 1.0` · `NoFiringPositionCost 0.5` · `RouteRiskWeight 0.6`(← 1.0) · `ObjectiveWeight 2.0`(← 1.2) · `MoveImprovementMargin 0.3`(← 0.25) · `InnerRingFraction 0.45` · `ASoldierObjective` `RadiusCm 1500`(← 900) / `DefendBandCm 1200`(← 900) / `ApproachScaleCm 3000`(← 6000). **[C-88]의 옛 값 표는 이것으로 대체된다**(원 표는 지우지 않았다). **판정 기준**: 낮은 담이 실제로 최저 비용이 되는가 · 공격수가 전진하는가 · 수비수가 반경 안에서 *멈추는가*(→ [C-95]) | AI 관찰 |
| **C-95** | ★★ **수비수가 자리를 잡고 쏘지 못한다 — 두 번의 시도가 실패했다** | **증상**: 재배치가 너무 잦아 정착해 사격하지 못한다 [A · 관측됨]. 실패한 시도 ① 한 바퀴 위협 스냅샷 + 위협 없을 때 `bCanHide=true` ② 속도 신뢰 만료 + 목표 반경 확대. **다음은 또 한 번의 추측이 아니라 진단이다** — 엄폐 오버레이가 **HERE 비용 · 최선 후보 비용 · 여유**를 전부 찍도록 확장해 두었다. **세 갈래**: ⓐ `HERE` 가 0인데도 움직인다 → **이동 발행이 문제**(한 바퀴 종료 블록 · `bAlreadyGoing` · 정지 판정) ⓑ `HERE` 가 진동한다(hide/fight 가 뒤집힌다) → **위협 추정이 여전히 불안정**(`GetPrimaryContact` 대상 교체 · 반경 성장 · `HeightSamples 3` 에서 표본 하나로 뒤집히는 `bCanFight`) ⓒ `best` 가 정말로 더 낮다 → **가중치 문제**([C-94]). ⚠ **P10 · P44를 지킬 것 — 이번 두 실패가 정확히 그 둘을 어긴 결과다.** `ai/2026-09-14_exposure_ladder_and_corrections.md` 12절 | **★ 최우선** |
| **C-96** | **`VelocityTrustSeconds 1.5` 가 맞는가** | 추측항법이 속도를 들고 가는 최대 시간. 총성 기록은 **0**(소리는 진행 방향을 말하지 않는다). 고치기 전: 20초 된 기록이 **120m 앞**을 가리켰다 / 후: **9m** 에서 멈추고 반경만 열린다. **판정 기준**: 뛰어 지나가는 적에 대한 **리드 사격이 살아 있으면서** 모퉁이로 사라진 적을 향해 **엉뚱한 곳으로 이동하지 않는** 구간. **1.5초는 감이다** — [C-85]와 같은 계열 | AI 관찰 |
| **C-97** | **새 레벨 규모 · 정지 판정 · RVO 회피의 실효** | 네비 영역 **110m × 90m**(x −4000..7000 · y −4500..4500) · `StallSpeedCms 20` · `AvoidanceRadiusCm 300`. **판정 기준**: ① **정지 95m** 교전 상한이 이 규모에서 실제로 *제약*이 되는가(옛 레벨에서는 어디서 쏘든 사거리 안이었다) ② 30m 차폐벽 2장이 측면 회랑을 실제로 가려 **돌아가는 것이 대안이 되는가** ③ 아군끼리 겹치거나 마주 보고 멈추는 일이 사라졌는가([W29]가 아니라 **8.1·8.2절**). ⚠ `StallSpeedCms` 가 **정상 감속/회전까지 stall 로 읽으면** [C-95]ⓐ 가 된다 | AI 관찰 |
| **C-98** | **`Content/SoldierLab/` 의 참조 폐포가 실제로 얼마나 큰가** | titan 에 GASP 가 **한 조각도 없어** 기반 전체가 새로 들어간다. `Content/SoldierLab/` 은 **347 MB** 인데 `Characters/UEFN_Mannequin/Animations/` 만 **2.7 GB** 다(Crouch 722M · Walk 718M · Run 536M · Jump 188M …). 병사는 **라이플 자세로만** 싸우는데 비무장 로코모션이 얼마나 참조되는지는 **Migrate 확인 대화상자의 파일 목록으로만** 알 수 있다. 같은 자리에서 `Rifle/_MF/` 39개(Lyra 여성 마네킹 변종)가 참조 0건인지도 판정된다 → **정리 문서 3.3절에서 이미 참조 0 확인**. `migration/2026-09-14_titan_example_migration.md` 4절 | 이관 직전 |
| **C-99** | **재질별 명중 효과가 실제로 동작하는가 — 물리 머티리얼이 레벨에 안 걸려 있다** | `PhysicsMaterials/PM_*` 5개와 `example_mat/M_*` 5개가 **참조 0건**이고 `L_SoldierTest` 도 `PM_*` 를 하나도 참조하지 않는다. `BP_RifleProjectile` 은 `MS_hit_rifle_{dirt,glass,hard,metal,wood}` 와 `NS_Rifle_*` 를 재질별로 들고 있는데, **레벨의 어떤 표면에도 물리 머티리얼이 안 붙어 있으면 전부 기본값으로 떨어진다.** 즉 `weapons/2026-09-12_projectile_port.md` 가 만든 재질 분기가 지금 시험 레벨에서 **한 번도 갈린 적이 없을 수 있다.** **판정**: PIE 에서 나무/금속/유리에 쏴 소리와 파티클이 갈리는지 본다 | 이관 검증 6번 |
| **C-100** | **`FootstepEffectTagModifier` 가 여전히 컴파일 에러를 내는가** | P79 가 `AN_PlayWeaponMontage` 와 묶어 "늘 뜨는 것" 으로 적었는데 **원인이 다르다** — 이쪽은 `/Script/LyraGame` 을 참조하지 않는다(2026-09-14 전수 확인). 그리고 **이관 폐포 안에 있다**(`Characters/Heroes/Mannequin/Animations/AnimModifiers/`). 즉 titan 으로 따라간다. **판정**: 노티파이 제거 후 PIE 로그에 이쪽 에러가 남아 있는지 본다. 남으면 원인을 따로 잡아야 하고, 안 남으면 애초에 `AN_PlayWeaponMontage` 하나였던 것이다 | 이관 전 PIE |
| **C-101** | **`PC_Sandbox` 를 우리가 실제로 쓰고 있는가** | `GM_SoldierLab` 의 PlayerControllerClass 가 GASP `PC_Sandbox` 이고, 이것이 `AC_VisualOverrideManager` 와 함께 **`GM_Sandbox` 로 가는 두 경로 중 하나**다. 둘 다 끊어야 Echo/Paragon/MetaHumans 2.2 GB 가 빠진다(4.2d). 그런데 `PC_Sandbox` 에는 카메라·입력이 걸려 있을 수 있어 **떼면 조작이 죽을 수 있다.** **판정**: 무엇을 상속·오버라이드해서 쓰는지 그래프를 읽고, 필요한 것만 최소 PC 로 옮길 수 있는지 본다 | 이관 정리 시 |

---

## D — 설계 백로그

문서: `design/2026-09-01_architecture.md` 15.3절

| # | 항목 | 시점 |
|---|---|---|
| D1 | 애니메이션 **메모리** 예산 (MM DB 크기 기준선) | P1 |
| D2 | 엄폐 슬롯 베이크 산출물의 저장·버전관리 | P0-3 |
| D3 | 테스트 레벨 사양 + 성능 측정 방법 | P0(짐) / P3(성능) |
| D4 | 애님 노티파이 규약 | P1 |
| D5 | LOD 티어 전환 팝핑 대책 | P3 |
| D6 | Intent 전환 권한 규칙 (= C-12) | P1 |
| D7 | `EOrderRecipientType::Individual`의 의미 | P3 |
| D8 | UGV/차량 상호작용 | P3~P4 |
| D9 | P1 검수 범위 경계 | P1 착수 |
| ~~D10~~ | ~~★★ **목표·임무 개념 자체가 없다**~~ | **✅ 해결 (2026-09-13)** — `ASoldierObjective`(로직 없는 레벨 마커, **한 개가 쥔 쪽에겐 수비·나머지에겐 공격**)와 **세 비용 위치 스코어러**(도착지 노출 + 경로 노출 + 땅의 값, 전부 0..1 한 통화). ⚠ **레벨에 아직 0개 배치**라 셋째 항은 지금 항상 0이다 → **[W25]** · 값 검증 **[C-88]**. `ai/2026-09-13_objective_and_position_cost.md` |

---

## Q — 사용자 결정

| # | 항목 | 상태 |
|---|---|---|
| ~~Q1~~ | 프로젝트 이름/위치 | ✅ `anim_test/SoldierLab` |
| ~~Q2~~ | GASP 베이스 | ✅ 채택 |
| ~~Q3~~ | 스켈레톤 규격 | ✅ UE5 Mannequin 전면 교체 |
| ~~Q4~~ | ~~소스컨트롤~~ | ✅ **Perforce(P4V) 확정** (2026-09-03). `.p4ignore` 작성 완료 |
| Q5 | P1 순서 (로코모션 vs AI 판단) | 로코모션 먼저 (기본안) |
| ~~Q6~~ | 규모 목표 | ✅ 45명 |
| Q7 | 디자인팀 공지 시점 | P0 완료 후 비교 영상과 함께 |
| ~~Q8~~ | ~~캐릭터 메시 (리스킨/신규/MetaHuman)~~ | ✅ **아군 확정 (2026-09-13)** — 기존 리스킨 `soldier_T` 를 **Assign Skeleton** 으로 `SK_UEFN_Mannequin` 에 올렸다(재임포트·리타깃 없음). 적군은 **[Q42]** |
| **Q42** | ★ **적군 메시 경로** — `Enemy` 는 Mixamo 65본(겹치는 본 0, `root`/IK 없음, 111k 버텍스 · LOD 1개, 원본 FBX 는 다른 PC) | **A) 디자인팀 요청 (권장)**: "UEFN 마네킹 리그(`SK_UEFN_Mannequin` 계층 그대로) · 마네킹 A-포즈/비율로 피팅 · 새 본 추가 금지 · LOD0 3만 이하 + LOD 3단 · 슬롯 이름 유지 · FBX 납품". 참조 리그는 `SKM_UEFN_Mannequin` 을 Asset Actions → Export 로 뽑아 전달. **아군도 같은 스펙으로 다시 받으면** 메시별 값(골반 높이 · 팔꿈치 폴)이 사라진다 / B) Blender 리스킨 4~8h, 중품질, A 가 오면 버려진다 / C) 당분간 `BP_Soldier_Hostile` 은 마네킹 유지(AI 작업 영향 없음). `animation/prototypes/2026-09-13_ally_mesh_on_mannequin_skeleton.md` 6절 | **결정 대기** |
| **Q43** | ★ **GASP 비무장 애니메이션 2.7 GB 를 titan_example 에 통째로 넣을 것인가** | titan_example 은 **납품이 걸린 프로젝트**다. 자르려면 ABP 에서 비무장 분기를 끊어야 하고, 그 순간 **GASP 원본과 차이가 생겨 이후 업데이트를 못 받는다.** **권장: 1차 이관은 자르지 않는다** — 안 되면 이관 문제인지 절단 문제인지 구분이 안 된다(P26). 절단은 동작 확인 후 별건. `migration/2026-09-14_titan_example_migration.md` 4절 | **결정 대기** |
| ~~Q9~~ | 유료 팩 구매 | ✅ 배제 |
| Q10 | 견착 전환 동작 조달 (A/B/C안) | P0-2 결과 후 |
| ~~Q11~~ | ~~이동 시스템 — CMC vs Mover~~ | ✅ **CMC 확정 (2026-09-04)**. `PSD_Soldier_Walk_Test`를 `PSS_Default` + `PSN_Dense_All`로 전환 완료. 남은 일: `NPCLevel`의 배치 캐릭터를 CMC로 교체·재확인 → **[C-41]**. 아래는 판단 근거 |
| | (Q11 근거) | **권고: CMC.** 설계 3.1절 전제·`titan_example` 정합·Mover의 `NetworkPrediction`/`ChaosMover` 의존(P5와 얽힘) 때문. **이관 비용은 거의 0** — AI 스택이 `BPI_SandboxCharacter_Pawn` 인터페이스만 쓰고 두 캐릭터가 모두 구현한다. 우리 PSD는 스키마→`PSS_Default`, 정규화세트→`PSN_Dense_All` 두 줄만 변경. 남는 일은 `NPCLevel`의 배치 캐릭터 교체·재확인 → `ai/prototypes/2026-09-04_p0-1_ai_drives_mm.md` 3절 |

---

### 2026-09-11 — AI 초안 3건의 결정 사항

> 초안 3건이 각자 `Q12`부터 번호를 매겨 **3중 충돌**했다. 2026-09-11에 재번호했다 —
> 인지 `Q12~Q22` · 분대 `Q23~Q33` · 엄폐 `Q34~Q40`.
> 엄폐 초안의 `C-60~C-69`도 오늘 만든 [C-60]·[C-61]과 겹쳐 **`C-62~C-71`로 밀었다.**

| # | 결정 | 근거 |
|---|---|---|
| ~~Q36~~ | **Prone을 범위에서 무름** | 클립 0개(~15개 필요) · 캡슐 40cm 대응 · 슬롯 베이크 1.5배. `ESoldierCoverHeight`에 자리만 남긴다. **인지 초안의 `ExposureProne`은 제거한다** |
| ~~Q24~~ | **분대 표준 5명** (조 3+2) | 아군 6 + 적 3 = **9분대**. 설계 14절 P2 검수 기준이 5 vs 5 |
| ~~Q12~~ | **주표적 = 혼합**, `ThreatVsValueBias` 기본 0.5 + 프로파일별 분기 | 신병은 위협 쪽 · 정예는 교전가치 쪽. 설계 5.3 "개체차는 데이터다". **튜닝은 몸(1~2번 축)이 생긴 뒤** |
| ~~Q35~~ | **`Exposure`는 곱셈 인자 하나로 나눈다** — `PerceivedExposure = 인지Exposure × (1 − CurrentCoverQuality)` | 두 초안이 같은 이름의 다른 숫자를 쓰고 있었다. 방치하면 **엄폐가 두 번 곱해져 조용히 틀린다.** 공식은 `USoldierCoverSubsystem` 한 곳에만 |
| ~~Q22~~ | **위협 기억은 개인이 갖는다** + 분대 질의용 별도 컨텍스트 | 설계 7.4·9.3이 개인 소유 전제 — `bSharedBySquad` · 신뢰도 0.6 상한 · "직접 확인하러 간다"가 전부 거기서 나온다 |
| ~~Q23~~ | **`FThreatMemory`에 `LocationSigmaCm` 추가** | 위 결정의 전제. σ 없으면 "무전으로 들은 적을 정조준하지 않는다"를 구현할 자리가 없다 |
| ~~Q20~~ | **`LineOfFireClear` 기본값 1.0** + 디버그 HUD에 "미구현" 명시 | 곱셈 축이라 0으로 두면 **영영 안 쏘다.** 아군을 쏘더라도 **원인이 보이는** 쪽이 낚b다. 조용한 0은 이 프로젝트가 가장 여러 번 당한 유형 |
| ~~Q28~~ | **편제는 스포너 액터가 저작** | 45명 성능 레벨에서 "9분대 45명"을 한 번에 만들 방법이 필요. DataTable+태그는 레벨마다 태그를 심어야 해 실수가 진다 |
| ~~Q34~~ | **GameplayInteractions 사용** | 이미 SoldierLab에 켜져 있고 GASP 승계 방침과 일치. 설계 13절의 "안 쓴다"를 정정한다 |
| ~~Q21~~ | **`Sight` 트레이스 채널 신설** — Cover 채널과 **한 번에** 판다 | 엄폐물은 시야를 막지만 유리·철망은 엄폐가 되면서 시야는 통과한다. `ECC_Visibility` 재사용은 렌더링 설정에 인지가 끌려다니게 된다 |
| ~~Q17~~ | **타입 이름에 `Soldier` 접두 통일** | 지금은 공짜, 나중은 문서/코드 불일치 |
| **Q13** | “적이 나를 조준 중” 판정 방식 | **보류.** 현실적인 반응(최소 노출 사격 · 맹목사격)이 전부 **몸의 축이 생겨야 표현된다.** 판정 결과로 뭔 할지 모르는 채로 인터페이스를 굳히지 않는다 |
| ~~Q41~~ | **웅크린 채 달리려 하면 어떻게 되는가** — (A) 속도를 묶는다 / (B) 웅크림을 푼다 | ✅ **(A) 속도 상한 채택 (2026-09-12).** 이유가 일반화되므로 적어 둔다: **자세는 생존 결정(노출 실루엣)이고 속도는 그 결과다.** (B)는 **이동 요청이 실루엣을 조용히 높인다** — AI는 "일어서라"고 명령한 적이 없는데 일어서 있고, 그 결정이 어디에도 기록되지 않아 **추적 불가능**해진다. 엄폐 판단이 자세 높이를 읽는 이상 이것은 **원인을 모르는 피격**이 된다(**P37**·**P38** — 소유자는 하나, 부수 효과로 덮어쓰지 않는다). **슈터 관례의 손맛은 비용 없이 살릴 수 있다** — **스프린트 *입력*이 입력 계층에서 stance를 0으로 명령**하게 하면 되고, **AI는 그 명령을 내리지 않으면 그만**이다. 구현상 부수 조건: 속도만 묶으면 안 되고 **`Gait` 라벨도 Walk로 클램프**해야 한다(**P46**) → `IMPLEMENTED.md` 2.5f-3·2.5f-9 · `animation/prototypes/2026-09-12_continuous_stance_axis.md` 11절 |

---

## R — 추가 조사

문서: `ai/2026-09-02_upper_layer_plan.md` 13절

| # | 항목 | 상태 |
|---|---|---|
| ~~R1~~ | StateTree 서브트리 구성 가능 여부 | ✅ 가능 (`LinkedAsset`, 5.4+) |
| ~~R2~~ | EQS로 SmartObject 슬롯 뽑기 | ✅ 마찰은 질의가 아니라 예약. GASP에 답 있음 |
| R3 | `STT_PlayAnimFromBestCost` 동작 | 엄폐 진입 동작 선택에 쓸 수 있는지 |
| R4 | `STE_GetAIData` Evaluator 패턴 | 분대 blackboard 접근 원형 |
| ~~R5~~ | GASP StateTree 실제 구조 | ✅ 해결 — `ai/..._upper_layer_plan.md` 14절 |
| **R6** | **`AO_Blend_Curve`의 키 값** (`/Game/Characters/UEFN_Mannequin/Animations/AimOffset/AO_Blend_Curve`) | `BlendListByBool_0`의 `customBlendCurve`다. **MCP로는 읽을 수 없다** — `FRichCurve`가 `ObjectTools`에 리플렉션되지 않고 `unreal` 파이썬 모듈은 차단돼 있다. **최댓값이 1.0을 넘으면 블렌드가 오버슈트하므로 "총을 다시 들 때 솟구침"의 독립적인 원인일 수 있다** — 2026-09-12 안티 와인드업이 증상을 덮었더라도 남아 있을 수 있다. **판정 기준**: 에디터에서 열어 최댓값 ≤ 1.0인지 확인. **수동 확인 필요** → `animation/prototypes/2026-09-12_body_yaw_rate_and_aim_antiwindup.md` 14절 |
| **R7** | **진영(Faction) 판정의 정식 소스를 무엇으로 둘 것인가** | 투사체 이식에서 `UDetectableTargetComponent::Faction`을 끊고 **`bHitEnemy = OtherActor && OtherActor->IsA<ACharacter>()`** 로 대체했다. 지금은 **이펙트 선택(스파크 vs 혈흔)에만** 쓰이므로 틀려도 대가가 작지만, **인지·위협평가·사격 판정이 들어오면 같은 질문이 훨씬 비싸게 돌아온다.** 설계 원칙 **P4**("아군/적군은 코드 한 벌, 진영은 데이터")가 답의 형태를 이미 정해 두고 있다 — 조사할 것은 **어디에 그 데이터를 두는가**(캐릭터 프로퍼티 / 컴포넌트 / 팀 인터페이스 `IGenericTeamAgentInterface`)와 `titan_example`의 `UDetectableTargetComponent`를 그대로 가져올 가치가 있는가 → `weapons/2026-09-12_projectile_port.md` 2절  ★ **2026-09-13 — 정식 소스가 생겼다**: `USoldierIdentityComponent::Faction`(`ESoldierFaction` Friendly/Hostile/Neutral) + `USoldierRegistrySubsystem`. AI 층은 전부 이것을 읽는다. **투사체만 아직 `IsA<ACharacter>()` 대역을 쓴다** → 갈아끼우는 일은 **[W23]**. `ai/2026-09-13_perception_stack.md` 1절 |

---

## U — GASP 미확인

문서: `animation/2026-09-02_gasp_abp_analysis.md` 13절

| # | 항목 | 상태 |
|---|---|---|
| U1 | 스테이트머신 내부 상태/전환 | ⚠️ 부분 — 워핑 위치는 확인(16절). 상태 구성은 미확인. **실험 경로라 우선순위 낮음** |
| ~~U2~~ | 프로퍼티 바인딩 대응 | ✅ 해결 — 14.1절 |
| U3 | Trajectory 채널 `flags` 비트 의미 | 엔진 소스 확인 필요 |
| ~~U4~~ | Chooser 행별 결과 | ✅ 해결 — 15.2·15.3절 |
| U5 | Dense/Sparse/Extreme_Sparse 차이의 성격 | 클립 수인가 샘플링인가 |
| ~~U6~~ | ~~`Relaxed` 티어의 성격 — LOD인가 스타일인가~~ | **✅ 해결(2026-09-04) — 둘 다 아니다. `Mover` 경로의 DB 세트다.** LOD 티어는 Dense/Sparse/ExtremeSparse 셋(`PSS_Default` + `M_Neutral_*` 클립)이고, `Relaxed`는 Mover ABP 전용(`PSS_Relaxed_Loops` + `M_Relaxed_*` 클립). `ai/prototypes/2026-09-04_p0-1_ai_drives_mm.md` 3.2절 |
| ~~U7~~ | `RemapCurves` 역할 | ✅ 해결 — 3.3b절. 발 접지 구동원 |

---

## W — 분석에서 파생된 작업

문서: `animation/2026-09-02_gasp_abp_analysis.md` 13.1절

| # | 항목 | 시점 |
|---|---|---|
| W1 | 자산 반입에 **접지 커브 생성 단계** 추가 | ✅ 문서 반영 완료 / 구현은 P0-4 |
| W2 | 교란을 **Control Rig 연산**으로 구현 | P1 |
| ~~W3~~ | ~~`AIController`가 컨트롤 로테이션으로 조준 구동~~ | **✅ 해결 (2026-09-13)** — `USoldierEngagementComponent`가 `AAIController::SetFocalPoint`로 조준한다(조준은 컨트롤러의 일이다). `Build.cs`에 **`AIModule`** 추가 |
| ~~W4~~ | ~~`TrajectoryGenerationData` AI용 재튜닝~~ | **✅ 불필요 (2026-09-04)** — [C-1] 잠정 통과. 기본값으로 급선회가 버틴다. Epic이 Steering 노드를 "작업 중"으로 표기해 첫 의심 대상이었으나 문제가 나타나지 않았다 |
| W5 | `Disable_AO` 커브 패턴을 우리 무기 시스템에 채택 | P1 |
| **W6** | **총구 보정 루프의 계측 장치·잔해 노드 정리** — ⚠ 화면의 `GATE`는 **죽은 오차 크기 게이트**를 찍고 있어 다음 사람을 속인다. 실제 게이트(`InRange(0..2.0)`)에 다시 물리거나 뗄 것. 완전히 죽은 노드: `Delta(control, GetActorRotation)` · `Lerp(Rotator) Alpha 0.15` · `ToString(Rotator)` ×2 · `GetControlRotation` ×1. `AimLoopActive` 변수와 `SelectFloat(25/20)`도 함께. `DrawDebugCoordinateSystem`은 분기 밖이라 **모든 병사에게 그려진다**. → `animation/prototypes/2026-09-11_muzzle_aim_alignment.md` 11절. ★ **2026-09-12 갱신**: 계측 축이 더 늘었다(`BodyErr`·`WpnLow`·`WpnTgt`·`AimGain`, 그리고 블라인드 파이어의 **`BF_H`·`BF_V`**). 그리고 런타임 모듈에 **`USoldierDebugAxes`(라벨 행 HUD, `SoldierLab.Debug.Axes` cvar)** 가 생겨 있으므로 **"PrintString 3줄"이라는 기술 자체가 낡았을 수 있다** [B] — 정리 착수 전에 BP 배선을 실물로 다시 볼 것 (`IMPLEMENTED.md` 4절). ★★ **2026-09-12 갱신 (2)**: stance 축이 **계측 7행**(`Stance` · `PelvWZ` · `PelvTgt` · `PelvOff` · `Crouched` · `SpdCap` · `GaitClamp`)과 **죽은 변수 3개**(`StanceDropMax` · `StanceRaiseMax` · `StanceBlendRate` — 실패한 램프 접근의 잔해)를 더했다. ⚠ **단 stance 7행은 함부로 떼지 말 것** — 이 축의 문제는 **과도구간에만 존재**해서 저 행들이 없으면 진단 자체가 불가능하다(**P44**). **죽은 변수 3개는 걷어낸다**(P36: 덧댄 것은 원인을 찾은 뒤 반드시 걷어낸다) | P0-2 |
| **W7** | **린(lean) 축이 문서화돼 있지 않다 — 소급 문서화** | 구현돼 동작 중이다(`LocalToComponentSpace_0` 뒤의 `ModifyBone spine_01..05` · 입력 `IA_Lean` · Q/E, 누르면 등속 램프 / 떼면 유지). 그런데 `IMPLEMENTED.md`에 절이 없어 **2026-09-12에 전체 평가 체인을 정정하면서 그 존재가 처음 기록됐다**(2.4절). 블라인드 파이어 축(2.5e)이 **린과 같은 규약·같은 입력 형태**를 쓰므로 나란히 적으면 둘 다 짧아진다. **범위**: 축 변수·램프 속도·`ModifyBone` 노드 설정(축·공간·알파 출처)·입력 매핑 | P0-2 |
| **W8** | **마스크 규약이 두 벌 공존한다 — 통일하거나 이유를 적을 것** | `LayeredBoneBlend_0`(로우레디)은 `blendMode = BlendMask` + 블렌드 마스크 에셋 `SK_UEFN_Mannequin:BM_LowReady`, 2026-09-12에 추가된 `LayeredBoneBlend_2/3/4`(블라인드 파이어)는 `blendMode = BranchFilter` + `spine_01 / blendDepth 1`이다. **둘 다 동작하지만** 다음 사람이 "왜 두 가지인가"에서 멈춘다. 하나로 모으거나 **어느 경우에 어느 쪽을 쓰는지**를 `IMPLEMENTED.md` 2.5e-3에 규칙으로 적을 것 | P0-2 |
| **W9** | ★★ **캡슐 높이를 연속으로 구동한다 — 남은 조각 중 AI 관점에서 가장 값어치 있다** | stance 축은 **보이는 높이만** 연속으로 만들었다. `Crouch()`가 문턱에서 `capsuleHalfHeight` **86↔60을 한 번에** 바꾸므로 **충돌과 엄폐 높이는 여전히 이진**이고, **AI의 엄폐 판단이 읽는 것이 바로 그 높이**다. 즉 지금은 "반쯤 웅크린 병사"가 AI에게는 **서 있거나 앉아 있는 둘 중 하나**로만 보인다. ⚠ **동시에 위험도 가장 높다** — 관통 · 계단 오르기(step-up) · **일어설 때의 천장 스윕**(CMC가 **이진 경우에 대해서만** 구현해 두었다). 안정성 판정 기준은 **[C-3]** → `IMPLEMENTED.md` 2.5f-1 | P0-2 |
| **W10** | **웅크림 카메라를 stance 축으로 구동** | `CameraRig_CrouchOffset`은 Camera Pose 공간의 **고정 `TranslationOffset (40, 0, −30)`** 이고 **블렌더블/데이터 파라미터가 하나도 없다** — 즉 **블렌드되는 이진**이라 float를 받을 자리가 없다(⚠ `CameraRigAsset` 내부는 MCP로 안 읽혀 **에디터에서 손으로 확인**했다). **결정: 리그를 고치지 않는다. 비활성화하고 SpringArm Z를 stance 축으로 직접 몬다** — 캐릭터가 이미 SpringArm을 갖고 있다. 눈 높이 실측은 `baseEyeHeight 100` ↔ `crouchedEyeHeight 32`. 미착수 | P0-2 |
| **W11** | **AO를 연속 블렌드로 바꾼다** | 지금은 `Select(Stance == Crouch ? AO_Rifle_Crouch : AO_Rifle_ADS)` **이진**이다. **DB 전환과 같은 순간**에 갈리므로 지금까지 눈에 거슬리지 않았고 그래서 미뤘다. 연속화하면 `IMPLEMENTED.md` 2.5f-1의 ④가 닫힌다. ⚠ `animation/2026-09-02_pose_pipeline_spec.md` 6.2절의 예고("앞으로 추가하는 **모든** 애디티브는 앵커마다 버전이 필요하다")가 여기서 현실이 된다 | P1 |
| **W12** | **중간 자세 포즈 한 장을 저작해 3점 블렌드로 만든다** [B] | 지금 0↔1 사이는 **직선 보간**이다(중간 높이 클립이 없으므로). 중간 웅크림 포즈를 **한 장** 저작해 0 / 0.5 / 1 로 블렌드하면 **가운데를 저작자가 통제**할 수 있다. 저작 경로는 이미 확립돼 있다 → `CLAUDE.md` 6.3절. **[C-2]의 품질 개선 수단이기도 하다** | P1 |
| **W13** | **`IA_Crouch` 토글이 무력해졌다 — 제거하거나 재정의할 것** | stance 축이 `bIsCrouched`를 **매 Tick 소유**(`Crouch()`/`UnCrouch()`)하므로 토글이 무엇을 하든 다음 프레임에 덮어써진다. **살아 있는 입력처럼 보이는데 아무 효과가 없는 상태**라 다음 사람을 속인다. 선택지: ① 매핑 제거 ② **"stance를 0 또는 1로 명령하는 입력"으로 재정의**(2.5f-9의 스프린트 입력과 같은 형태 — 축의 소유권을 깨지 않고 값을 명령한다) | P0-2 |
| **W14** | **마이그레이션된 경로에 남은 에셋 2개를 `SoldierLab/` 아래로 옮긴다** | `M_Decal_Bullet`(`/Game/Gun_effect/Decal_Bullet/Demo/Materials/`) · `M_RCWSRound`(`/Game/Vehicles/UGV_OLD/`). 둘 다 `BP_RifleProjectile` CDO가 참조 중이라 **옮기면 리디렉터가 생긴다** — 옮긴 뒤 CDO 참조를 확인할 것 | P0-2 |
| **W15** | **P4V 체크아웃 미처리 — `Config/DefaultEngine.ini` · `Source/SoldierLab/SoldierLab.Build.cs`** | `DefaultEngine.ini`는 **Perforce 읽기전용 플래그를 사용자 1회 허가로 지우고** 편집했고 **`DefaultEngine.ini.bak` 백업이 남아 있다.** 체크아웃하지 않으면 다음 동기화에서 조용히 되돌아갈 수 있다. ⚠ 되돌아가면 **`PhysicalSurfaces` 순서와 `Cover`/`Sight` 채널이 통째로 사라지고**, 재질별 명중 반응이 이유 없이 기본값으로 떨어진다 | **즉시** |
| ~~W16~~ | ~~★ **총알 휘파람의 최근접 판정을 주변 병사로 확장 — AI 제압(suppression) 신호 생산자**~~ | `ASoldierProjectile::Tick()`의 휘즈 블록이 이미 `ClosestPointOnSegment`로 **이번 프레임 이동 선분과 청취자의 최근접 거리**를 구한다 — 제압 신호가 필요로 하는 기하 **그 자체**다. 지금은 **로컬 카메라 하나만** 시험하고 소리만 낸다. `WhizBroadPhaseRadiusCm 1500` 광역 컬링이 비용 구조도 이미 갖고 있다. **L2(인지·위협평가)가 착수될 때 여기가 입력 단이 된다** — 초안의 "피격/근접탄 인지"를 **추측이 아니라 실제 탄도에서** 얻을 수 있다 → `weapons/2026-09-12_projectile_port.md` 3.1절  ★ **✅ 해결 (2026-09-13)** — `ASoldierProjectile::ApplySuppressionAlongSegment()`가 휘즈와 같은 최근접 판정을 **병사 등록부**에 대해 한 번 더 한다. ⚠ 그 과정에서 **기존 버그 1건**을 고쳤다: `PreviousLocationForWhiz` 갱신이 `if (!bWhizPlayed && WhizSound)` **블록 안**에 있어 휘즈가 한 번 울리면 구간 추적이 멈췄다. `SuppressedThisFlight`가 **한 발 = 한 번의 놀람**을 보장한다. `ai/2026-09-13_perception_stack.md` 7.4절  | **✅ 완료 (2026-09-13)** |
| **W17** | **끊어낸 의존 4종의 복원 지점** | ① 진영(`bHitEnemy` → **[R7]**) ② 명중 방송(`ReportHitToInstigator`의 3분기 Multicast) ③ 도탄 방송(`ReportRicochetToInstigator`) ④ 바람(`WindVectorCms` 에 0). **네 군데 전부 호출 지점에 "무엇이 있었고 무엇을 되돌리면 복구되는가"가 주석으로 남아 있다.** ②③은 **리플리케이션이 생길 때** 한 벌로 돌아온다(P5) — 세 핸들러가 전부 `PlayImpactEffect`로 되돌아왔으므로 복원은 "직접 호출을 Multicast로 되돌리는 것"이다 → `weapons/2026-09-12_projectile_port.md` 2절 | 리플리케이션 착수 |
| **W18** | ★ **데미지·체력·사망이 없다 — 병사는 죽지 않는다** | 그래서 **교전이 끝나지 않고**, "적이 줄어든다"가 인지에도 교전에도 없다. 피격 반응 클립 19개는 반입돼 있으나 배선이 없고(`IMPLEMENTED.md` 6절) 래그돌은 GASP에 이미 있다. ★ **여기까지 미룬 것은 의도다 [B]** — **위험 회피는 노출 점수만으로 이미 성립하므로**(맞을 수 있다는 사실이 아니라 *보인다*는 사실이 병사를 움직인다) 사망이 없어도 AI 층의 판단은 전부 돈다. **피격 반응은 합류 시점에 `titan_example` 에서 가져올 계획**이다. ⚠ 대가: 목표 **점령/소유권 변경**도 성립하지 않는다(→ [W25]), 그리고 검증 세션이 **끝나지 않는 교전**을 본다 |
| ~~W19~~ | ~~**린 축과 블라인드 파이어 축을 AI가 몰지 않는다 — 다리는 이미 놓여 있다**~~ | ✅ **해결 (2026-09-14)** — `USoldierEngagementComponent` 가 `GetDesiredLean` / `GetDesiredBlindFireH/V` 를 발행한다. ★ **아래에 [B]로 적어 둔 "계획된 형태"가 그대로 나왔다** — 엄폐/사선 기하가 "무기를 어느 쪽으로 빼야 넘는가"를 알고(조리개 `Over`→`BlindFireV`, `Left/Right`→`BlindFireH`/린), **칸을 고르는 것은 제압도**이며, 맹목사격의 큰 산포 배수(4.0)가 **절대 가치 게이트를 통해** "근거리 전용"을 규칙 없이 만든다. `ai/2026-09-14_exposure_ladder_and_corrections.md` 1절 · P81. **옛 기술**: | `AITargetLean` · `AITargetBlindFireH/V` 가 `BP_SoldierCharacter`에 있고 `SelectFloat` 배선까지 끝났다. **넣는 쪽이 없을 뿐**이다. `ai/2026-09-13_ai_bridge_and_scene.md` 1.2절. ★ **계획된 형태 [B]**: 판단의 재료를 새로 만들지 않는다 — **엄폐 기하가 이미 "무기를 어느 쪽으로 빼야 엄폐를 넘는가"를 안다**(낮은 담 위로 들어올리면 `BlindFireV`, 모퉁이에서 옆으로 밀면 `BlindFireH`). **린은 완전 노출과 맹목사격 사이의 사다리**이고 그 칸을 고르는 것은 **제압도**다. 맹목사격에는 **큰 산포 배수**를 물려 둘 것 — 그러면 "근거리에서만 쓸 만하다"가 규칙 없이 자동으로 성립한다 |
| **W20** | ~~경로의 노출을 안 본다~~ → **경로 표본이 직선 위에 있다. 네브메시 경로가 아니다** | ✅ **절반 해결 (2026-09-13)** — `EvaluateRoute` 가 두 자리 사이 **내부 표본 3점**을 **서 있는 가슴 높이**로 트레이스해 `RouteRisk` 를 만든다(P74: 기어가는 값으로 매기면 모든 회랑이 안전해 보인다). **남은 것**: 표본이 **직선** 위에 있어 **건물을 돌아가는 경로는 틀리게 평가된다.** 들어온 목적이던 **탁 트인 회랑**은 직선과 경로가 같으므로 그 자리에서는 맞는다. 고치려면 실제 경로 질의가 필요하고 그것은 곧 비용이다 → [C-83]. **2026-09-14 갱신**: 항의 *성질* 이 바뀌었다 — **건너는 거리로 스케일**하고(보이는 것은 사건이 아니라 비율이다) 가중치를 **1.0 → 0.6** 으로 내렸다. 같게 두었을 때 **병사들이 스폰에서 한 발짝도 안 움직였다**(P83). **표본이 직선이라는 것은 그대로 남아 있다** |
| **W21** | ~~엄폐 후보가 한 반경의 링 하나다~~ → **고정 반경 *둘* 이다** | ✅ **절반 해결 (2026-09-14)** — `InnerRingFraction 0.45` 로 슬롯이 **두 링을 번갈아** 쓴다(1200 / 540). 고친 것은 다양성이 아니라 **막다른 골목**이다: 한 반경뿐이면 **구석에 몰린 병사의 후보가 전부 지오메트리 안**이라 네브메시 투영에 전부 실패하고 **스윕이 아무것도 못 돌려준다** — 그 자리에서 남은 경기를 보냈다. **남은 것**: 반경이 여전히 **고정 둘**이라 "2m 앞의 완벽한 자리"는 아직 후보가 아니다. **옛 기술**: | `SearchRadiusCm 1200` 위의 `CandidateCount 12`. **2m 앞의 완벽한 자리는 후보가 된 적이 없다.** 링을 여러 개로 늘리는 것은 곧 트레이스 예산 문제가 된다 → [C-83]과 엮인다 |
| **W22** | **소리에 차폐(occlusion)가 없다** | 벽 너머 총성도 똑같이 들린다. `BroadcastGunshot`은 거리만 본다. 차폐를 넣으면 **트레이스가 발당 청취자 수만큼** 늘어난다 → [C-83] |
| **W23** | **투사체의 `bHitEnemy`를 `USoldierIdentityComponent::Faction`으로 갈아끼운다** | [R7]의 잔여분. 정식 소스는 2026-09-13에 생겼는데 투사체만 아직 `IsA<ACharacter>()` 대역이다. [W17]①과 같은 자리 |
| **W24** | **`Cover` 채널(`GameTraceChannel4`)을 아무도 안 쓴다** | 시야·엄폐·사선 **셋 다 `GameTraceChannel5`("Sight")** 를 쓴다. 엄폐가 시야 판정을 거꾸로 돌린 것이므로 **같은 채널이 맞다**. 다만 [Q21]이 채널을 둘 판 이유 — "유리·철망은 엄폐가 되면서 시야는 통과한다" — 는 **아직 실현되지 않았다.** 채널을 지울지 살릴지 결정해야 한다 |
| **W25** | ~~`ASoldierObjective` 가 레벨에 한 개도 놓여 있지 않다~~ → **목표가 `BeginPlay` 에 하나로 고정된다 · 소유권 변경이 없다** | ✅ **절반 해결 (2026-09-14)** — `Objective_AllyBase`(−2500, 0 · `Friendly`) **1개 배치**. ★ **그런데 놓기만 했으면 여전히 안 돌았을 것이다**: `ASoldierObjective` 가 맨 `AActor` 라 **루트 컴포넌트가 없었고**, `GetActorLocation` 이 **영원히 월드 원점**을 답했다 — 즉 **모든 목표 거리가 (0,0,0) 에서 재어지고 있었다**(P90). `USceneComponent` 를 루트로 넣어 고쳤고, 배치된 수비 엄폐가 전부 목표 비용 **0.00~0.05** 로 나오는 것을 확인했다. `ai/2026-09-14_exposure_ladder_and_corrections.md` 6.1절. **남은 것**: 목표는 여전히 `BeginPlay` 의 **가장 가까운 것 하나**로 고정이고 **점령·소유권 변경이 없다** — 국면 전환은 분대·명령 층이다. **옛 기술**: | `L_SoldierTest.umap`(2026-09-13 18:27 저장) 이름 테이블에 `SoldierObjective` 가 없다 — **클래스가 레벨보다 늦게 생겼다.** 그래서 `Objective == nullptr` 이고 **위치 점수의 셋째 항이 항상 0**이다. 즉 **[D10]이 해소됐다는 머리글과 달리 거동은 아직 예전 그대로**다(노출만 최적화 → 무모한 전진). **할 일** ① 목표 액터를 시험 레벨에 놓는다(아군 소유 1개면 공격·수비가 동시에 생긴다) ② 그 뒤에 **[C-88]** 을 관찰한다. ⚠ 같이 딸린 구조 한계: 목표는 `USoldierCoverComponent::BeginPlay` 에서 **가장 가까운 것 하나**로 고정되므로 **국면 전환("1차 확보 후 2차로")이 표현될 자리가 없다** — 그 자리는 분대·명령 층이다 |
| ~~W26~~ | ~~**반동 누적이 없다 — 30번째 탄이 첫 탄과 같은 정확도다**~~ | ✅ **해결 (2026-09-14)** — `RecoilSpread` 가 `EffectiveSpreadAtCm` 의 곱셈 항으로 들어갔다(예상대로 **한 함수**였다). 발사 검출은 **새 훅 없이** — 무기가 이미 보고하는 탄창이 줄면 한 발 쏜 것이다. `BlindRecoilScale 2.0`: 팔 끝으로 담 밖에 내민 총은 눌러 둘 수가 없다. 값은 **[C-92]**. ⚠ 아래의 "몸 쪽 반동과 같은 양이 아니다"는 **여전히 미결정**이다 — 지금 넣은 것은 **탄의 산포뿐**이고 포즈 쪽 교란은 없다. **옛 기술**: | 교전 게이트는 **탄착 반경**(`거리 × tan(산포)`) 위에 세워져 있는데(P66), 그 산포는 **이동 속도로만** 넓어진다. **지속 사격도 맹목사격도 산포를 넓히지 않는다.** 귀결 둘: ① **제압 사격이 공짜로 정확하다** — 탄창을 비우는 동안 벌점이 없다 ② **맹목사격의 "근거리 전용"이 자동으로 성립하지 않는다**(계획은 큰 산포 배수를 물리는 것 → [W19]). 붙일 자리는 이미 있다 — `EffectiveSpreadAtCm` 한 함수에 항을 더하면 **교전 거리·재장전 판단·제압 판단이 전부 따라온다.** ⚠ 몸 쪽 반동(`animation/2026-09-02_pose_pipeline_spec.md` 6.6절 "교란")과 **같은 양이 아니다** — 이쪽은 탄의 산포, 저쪽은 포즈다. 두 개를 하나의 값으로 묶을지 결정해야 한다 |
| **W27** | **`StanceStandZ / StanceCrouchZ` 를 BeginPlay 에서 메시로부터 자동 산출** | 메시별 실측값이라(P77) 캐릭터 메시가 올 때마다 HUD 역산(`새 값 = 옛 값 − PelvOff`)으로 재야 한다. 아군 77.1 / 36.4, 마네킹 89.7 / 39.4. 기립 클립의 골반 높이는 메시 레퍼런스 포즈와 클립의 비율로 산출 가능할 것 [B]. 디자인팀이 [Q42] 스펙(마네킹 비율 피팅)으로 납품하면 값이 수렴해 급하지 않다 | P1 |
| **W28** | **아군을 "마네킹 비율"로 움직이게 하는 런타임 근사 — 5분 시험** | 사용자 요구: soldier_T 가 마네킹 뼈 길이로 움직이면 골반 높이·그립·IK 튜닝이 전부 마네킹 값으로 맞는다. `SK_UEFN_Mannequin` 스켈레톤 트리 → root 우클릭 → Recursively Set Translation Retargeting → **`Animation`** (현재 몸통 75본 `OrientAndScale`). 스킨이 ~9% 압축된다(181→165cm). **마네킹 자신은 안 바뀌어야 정상** [B](우리 클립이 마네킹 비율로 저작/리타깃돼 있으므로). 결과가 좋으면 `BP_Soldier_Friendly` 의 stance 값을 마네킹 값으로 되돌리고 팔꿈치 폴도 재확인. 나쁘면 `OrientAndScale` 로 원복(미저장 시 원복). 텍스처만 마네킹에 입히는 것은 **불가**(UV/지오메트리 종속) | P0-2 |
| **W29** | **메시 교체의 뒷정리** — ① 고아 `soldier_T_Skeleton`(참조 0건) 삭제 ② `soldier_T_PhysicsAsset` 바디를 에디터에서 확인(`thigh_twist_02` 바디 포함, MCP 로는 바디 목록을 못 읽는다) ③ `MM_Rifle_LowReady` 처분은 [C-90] 뒤에 | | P0-2 |
| **W30** | ★ **1인칭 카메라 · 플레이어가 조종하는 병사의 1인칭 모드가 없다** | **요청됐고 만들지 않았다.** 지금은 3인칭 + 관전 폰뿐이다(`ai/2026-09-13_ai_bridge_and_scene.md` 7절 · `ai/2026-09-14_exposure_ladder_and_corrections.md` 10절). ⚠ 이것은 보기 좋으라는 항목이 아니다 — **총구 정렬·조준 오프셋·블라인드 파이어의 품질은 1인칭에서 판정 기준이 달라진다**(눈이 총 뒤에 온다). [C-73]·[C-76]·[C-78]이 전부 그 시점에서 다시 물어진다. 빙의(10.2절)가 이미 있으므로 **붙일 자리는 병사 쪽 카메라 하나** | P0-2 |
| **W31** | **관전 폰의 입력이 Input Action 이 아니라 직접 키 이벤트다** | **새 `InputAction` 에셋을 이 도구로 만들 수 없고** `Config/DefaultInput.ini` 는 **읽기 전용**이다(→ [W15] P4V). 그래서 빙의 키(F)를 **직접 키 이벤트**로 배선했다. 동작하지만 **프로젝트의 입력 규약 밖**이고, 다음 사람이 `IA_*` 를 찾다가 못 찾는다. 7.1절(`DefaultPawnClass` 를 못 써서 함수 오버라이드로 우회한 것)과 **같은 계열 — 도구 제약이 설계를 한 칸 옮긴 자리** | 입력 정리 시 |
| **W32** | **조리개 트레이스가 예산 밖이다 — 그리고 디버그를 켜면 한 번 더 돈다** | `FindAperture` 는 최대 **4발**(Direct/Over/Right/Left)을 쓰는데 **엄폐의 라운드로빈 예산에 들어 있지 않다.** 게다가 `SoldierLab.Debug.Engagement` 가 켜지면 **그리기 위해 한 번 더 호출**되어 최대 8발이 된다. 병사당 틱 트레이스가 **3(여기) + 6(후보) + 4~8(조리개)** 로 올랐고 **[C-83]은 여전히 안 재봤다.** 고칠 방향 둘: ① 조리개 결과를 캐시해 디버그가 재사용 ② 조리개를 같은 예산에 넣는다(단 **조준 결정은 *지금* 필요한 답**이라 낡은 값을 쓰기 어렵다 — 그 점에서 엄폐 후보와 성질이 다르다) | [C-83]과 함께 |
| **W33** | **`soldier_T` 가 titan 에서 이름이 겹친다** | titan 에 이미 `/Game/Soldiers/New_Soldiers/soldier_T`(스켈레톤 `soldier_T_Skeleton`)가 있고, 이관본은 `/Game/SoldierLab/Characters/Ally/soldier_T`(스켈레톤 **`SK_UEFN_Mannequin`**)다. **경로가 달라 공존하고 서로 간섭하지 않는다** — 1차 이관에서 titan 기존 병사를 하나도 안 건드리는 것은 의도다(나란히 비교·되돌리기). 다만 **이름이 같아** 디자인팀 안내 시 반드시 경로로 구분해야 하고, `SKM_Ally_Soldier` 로 개명하는 편이 낫다. ⚠ 개명하면 리다이렉터가 남으므로 **Fix Up Redirectors 후에 Migrate**(P96) | 이관 전 |
| **W34** | **GASP 가 켜 둔 플러그인 중 실제로 필요한 것 가려내기** | titan 에 없어 새로 켜야 하는 것이 **16종**이고 두 프로젝트 교집합은 `ModelContextProtocol`·`ModelingToolsEditorMode`·`EditorToolset` **3종뿐**이다. 이 중 `Mover`/`ChaosMover`/`NetworkPrediction`/`MoverExamples`/`MovieSceneAnimMixer` 는 **우리 병사가 CMC 계열**([Q11] 확정)이라 불필요할 수 있고, `RigLogic`/`HairStrands`/`LiveLink`/`LiveLinkControlRig` 는 MetaHuman·Echo 샘플용이라 **[W36] 을 처리하면 같이 불필요해진다.** ⚠ 플러그인이 빠지면 로드 실패가 아니라 **조용히 껍데기로** 열린다(선례: `weapons/2026-09-12_projectile_port.md` 의 12.8 KB 껍데기) | 이관 후 |
| **W35** | **[W6] 계측 잔해를 이관 *전에* 게이트한다** | `BP_SoldierCharacter` Tick 의 `DrawDebugCoordinateSystem` 이 **분기 밖**에 있어 **모든 병사**가 그린다. 45명 목표에서 그대로 넘어가면 **titan 쪽 성능 문제로 보인다** — 원인이 우리 것인 줄 모르게 된다. [W6]의 잔해 목록 그대로. ⚠ 단 **stance 7행은 떼지 말 것**(P44) | 이관 전 |
| **W36** | ★ **`GM_SoldierLab` 의 캐릭터 목록이 약 2.5 GB 를 끌고 온다** | `GM_Sandbox` 복제 때 딸려 온 GASP 샌드박스의 *캐릭터 바꿔가며 보기* 배열이다. `BP_Echo`·`BP_Manny`·`BP_Quinn`·`BP_Twinblast`·`BP_UE4_Mannequin` + `MetaHumans/Kellan` 을 참조하고, 참조 폐포로 **Echo 1.1 GB · Paragon 751 MB · UE5_Mannequins 350 MB · MetaHumans 301 MB · UE4_Mannequin 17 MB** 를 끌고 온다. **우리는 병사만 쓴다.** 배열 하나를 비우는 것이 이번 정리에서 가장 큰 한 수. 관전/빙의는 `GM_SoldierObserver` 가 하고 그쪽은 이 목록을 안 쓴다. `migration/2026-09-14_asset_cleanup.md` 1절 | **이관 전 · 최우선** |
| **W37** | **아군 메시가 적군 폴더의 머티리얼을 참조한다** | `SoldierLab/Characters/Ally/soldier_T` → `SoldierLab/Characters/Enemy/Materials/Eyelashes/Ch_49_eyelashes`. 적군을 새로 받아 `Characters/Enemy/` 를 갈아엎으면 **아군이 같이 깨진다.** 공용이면 `Characters/Shared/` 로 올린다. 같은 자리에서 `Ally/Materials/Ch_49_body`(참조 0건 중복본)와 `/Game/Characters/Soldier/Mat/soldier_re_*`(동명 중복본 6개) 중 어느 쪽이 실제로 쓰이는지도 정리한다. [Q42] 적군 납품 전에 끝내는 것이 맞다 | [Q42] 전 |
| **W38** | **`L_SoldierTest` 가 `/Game/NewLevelSequence` 를 참조한다** | 프로젝트 루트에 떠 있는 `NewLevelSequence`/`1`/`2` 중 하나를 시험 레벨이 물고 있다. **의도한 것일 가능성이 낮다** — 이름이 기본값 그대로다. 끊고 3개 다 삭제. 끊지 않으면 이관 폐포에 따라간다 | 이관 전 |
| **W39** | **`M_RifleTracer` 가 `EvolveStudio` 마스터 머티리얼을 상속한다** | 트레이서 머티리얼이 남의 샘플 팩 마스터를 부모로 쓴다. 같은 계열로 `BP_RifleProjectile` → `/Game/Vehicles/UGV_OLD/M_RCWSRound`(titan RCWS 이식 잔재)·`/Game/Gun_effect/Decal_Bullet/.../M_Decal_Bullet` 이 있다. 플랫하게 만들지, 해당 에셋만 `SoldierLab/` 안으로 옮길지 판단. 정리 문서 2절 M4·M5·M8 | 이관 전 |
| **W43** | ★ **titan 과 SoldierLab 이 이미 서로를 물고 있다** | `/Game/Soldiers/Weapons/BP_EnemyRifle`(titan 기존 적군 소총)이 **우리 `SK_AR4_X` 를 참조**하고, 동시에 `NiagaraExamples` 의 총소리·재장전 사운드 4개와 `NS_MuzzleFlash` 를 **우리와 공유**한다. 즉 이관 후 어느 쪽을 정리하든 상대가 깨진다. 사운드 4개를 SoldierLab 으로 **옮길 수 없고 복제해야 하는** 이유이기도 하다. **이관 전에 이 얽힘을 끊을지(우리 총을 복제해 titan 쪽이 자기 것을 쓰게) 그냥 둘지 정한다.** `migration/2026-09-14_asset_cleanup.md` 2.5c | 이관 전 |
| **W44** | **무기 메시 애니메이션을 원하면 `SK_AR4_X` 에 ABP 를 붙인다 — Lyra 가 아니라** | 노리쇠 왕복 · 탄창 낙하 같은 무기 자체 동작은 지금 **전혀 없다**(`SK_AR4_X` 에 ABP 가 없어 `Montage_Play` 가 Accessed None 이었고 노드를 지웠다). Lyra 의 `AN_PlayWeaponMontage` 가 하려던 것이 정확히 이것이고, 그것 때문에 `/Script/LyraGame` 의존이 생겼다 → 끊었다(P100). 정말 필요해지면 **ABP 를 하나 만들고 `BP_AR4Rifle` 에서 몰면 된다** — 그쪽은 이미 발사·재장전 시점을 안다(`SetWeaponState`). 프레임워크가 아니라 노드 열 개 수준 [B] | 필요해지면 |
