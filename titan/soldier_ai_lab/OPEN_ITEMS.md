# 미해결 항목 추적 — soldier_ai_lab

2026-09-03 / 진행중 / 모든 문서의 미해결 항목을 ID로 통합 추적. 새 항목을 만들면 여기 등록하고, 해결되면 원 문서에 결과를 쓴 뒤 여기에 해결 표시.

> **ID는 전역으로 유일하다.** 새 항목은 각 접두의 최대 번호 +1을 쓴다.
> 접두 의미는 `CLAUDE.md` 4절 참고.

---

## 요약

| 접두 | 뜻 | 열림 | 해결 |
|---|---|---|---|
| **C** | 측정해야 아는 것 (실험 필요) | 31 | 18 |
| **D** | 설계 백로그 (해당 단계에서 채움) | 9 | 0 |
| **Q** | 사용자 결정 필요 | 3 | 8 |
| **R** | 추가 조사 | 3 | 2 |
| **U** | GASP 미확인 | 2 | 5 |
| **W** | 분석에서 파생된 작업 | 5 | 0 |

---

## C — 측정해야 아는 것

문서: `animation/2026-09-02_pose_pipeline_spec.md` 10절 · `ai/2026-09-02_upper_layer_plan.md` 12절 ·
`animation/2026-09-02_gasp_abp_analysis.md`

| # | 항목 | 판정 기준 | 시점 |
|---|---|---|---|
| ~~C-1~~ | ~~AI 구동 시 `TrajectoryGenerationData` 값~~ | **✅ 잠정 통과 [B] (2026-09-04)** — `STT_FocusToPlayer`로 플레이어를 조준시킨 채 순찰시키자 옆걸음·뒷걸음·급선회가 강제 발생했는데 **자세가 무너지지 않았다.** `TrajectoryGenerationData` 재튜닝 불필요 → **[W4] 해소**. ⚠ 단 GASP 비무장 DB는 8방향이 완비돼 있어 **데이터가 충분한 조건에서의 결과**다. 견착 DB로는 다시 봐야 한다 → [C-44] | — |
| **C-44** | **견착 DB(방향 커버리지 부족)에서도 급선회가 버티는가** | [C-1]은 8방향이 완비된 GASP 비무장 DB로 통과했다. 견착은 전방 위주 소수 클립뿐이라 **같은 조건이 아니다.** 이것이 [C-24](점진적 폴백)와 함께 P0-2의 본 판정 | **P0-2 ★** |
| C-2 | 자세 높이 블렌드 구간(0.4~0.7) 품질 | 높이 스윕하며 걸을 때 발 미끄러짐/다리 늘어남 | P1 |
| C-3 | 캡슐 연속 구동 안정성 | 바닥 스냅/충돌 해소가 흔들리지 않는가 | P1 |
| C-4 | `Phase` 커브 기반 DB 경계 위상 정합 | Stand↔Crouch DB 전환이 매끄러운가 | P1 |
| ~~C-5~~ | ~~견착 워핑 한계각~~ | **✅ 해결(형태가 바뀜)** — 워핑은 **직선 루프만** 커버, 전환은 못 함. `animation/..._gasp_abp_analysis.md` 16.2절 | — |
| C-6 | `AimDeadzone` / `AimTwistMax` 실제 값 | 재정렬이 과하지도 부족하지도 않은가. **GASP 기준값 60° 확인됨**(14.5절) | P1 |
| C-7 | `LowReady` 전용 조준 오프셋 필요 여부 | ADS 세트 축소 적용으로 충분한가 | P1 |
| C-8 | 맹목사격 IK 품질 하한 | 어느 각도부터 고무처럼 보이는가 | P1 |
| C-9 | 뱅킹 lean + 전술 lean 합성 방식 | 동시 적용 시 과회전하지 않는가 | P1 |
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
| ~~C-25~~ | ~~`Enable_Warping` 커브 자동 생성 가능 여부~~ | **✅ 해결(2026-09-03) — GASP에 `AM_WarpingAlpha` 모디파이어가 이미 있다.** 자체 제작 불필요. `prototypes/2026-09-03_enable_warping_curve_generation.md` | — |
| **C-26** | 리타깃 클립에서 `AM_WarpingAlpha` 임계값 **5°가 적절한가** | 직선 구간에서 커브가 깜빡이지 않는가 | **P0-4 ★** |
| **C-27** | 우리 클립에 걸었을 때 커브가 **전 구간 1이 아닌가** | 인플레이스 클립에 잘못 걸면 조용히 전부 1이 된다 | **P0-4 ★** |
| ~~C-28~~ | ~~`contact_l/r`의 실제 생성 경로~~ | **✅ 해결(2026-09-03) — 이진 사각파다.** 연속값인 `MotionExtractor`로는 만들 수 없다 → `AM_FootSpeed_*` 개명설 기각. `prototypes/2026-09-03_gasp_curve_manifest.md` 9절 | — |
| ~~C-33~~ | ~~`contact_l/r` 생성 수단이 없다~~ | **✅ 해결(2026-09-04)** — C++ 에디터 모듈 `SoldierLabEditor` 신설, `UFootContactCurveModifier` 자작. 높이+속도 판정, **발별 자동 높이 보정**이 핵심. `prototypes/2026-09-04_foot_contact_curve_modifier.md` | — |
| **C-35** | 리타깃이 **발 높이를 좌우 비대칭으로** 만든다 | 실측: `ball_l` 최저 2.79cm / `ball_r` 2.42cm, 평균 5.28 vs 6.13. 커브는 자동 보정으로 우회했지만 **발 IK와 접지 품질에는 여전히 영향**. 리타깃 포즈나 `FloorConstraintOp`로 잡을 수 있는지 | P1 |
| ~~C-36~~ | ~~`Turning_..._Anim`에 `contact_l/r`이 없다~~ | **✅ 해결(2026-09-04)** — 적용 완료. 우회전이라 오른발 축(87% 접지, 최고 4.27cm) / 왼발 스텝(73%, 6.40cm, 이동 2배)으로 물리적으로 정합 | — |
| ~~C-37~~ | ~~`enable_turninplacesteering` / `steeringtargettime` **생성 수단 없음**~~ | **✅ 확정 (2026-09-04)** — `/Game/Blueprints/AnimModifiers/`의 **18개를 전수 확인**했고 이 둘을 만드는 것은 **없다.** `contact_l/r`과 같은 상황(원본 저작물 또는 사내 툴). 분포: `steeringtargettime` 62클립 / `enable_turninplacesteering` 18클립. **어떻게 할지는 → [C-45]** | — |
| **C-38** | 발이 지면 근처에만 머무는 클립에서 **접지 판정이 속도 단일 조건**이 된다 | `Turning_...`에서 `below-Z`가 양발 100% — 자동 보정 임계가 전 구간을 통과시켜 높이 판정이 무력해졌다. 이번엔 결과가 맞았지만 애매한 클립에서 의심할 지점 | P1 |
| ~~C-41~~ | ~~CMC 캐릭터로 AI 스택을 옮겼을 때 도는가~~ | **✅ 해결 — 옮길 것이 없다.** `NPCLevel`에 `SandboxCharacter_CMC` 3기와 `SandboxCharacter_Mover` 2기가 **똑같은 `AIC_NPC_SmartObject_C` / `AutoPossessAI=PlacedInWorld`로 이미 나란히 배치**돼 있다. 남은 것은 Rewind Debugger에서 CMC 액터를 골라 `Pose Search` 트랙을 눈으로 확인하는 것뿐 | — |
| ~~C-39~~ | ~~AI가 채워야 할 축이 무엇인가~~ | **✅ 해결 — `S_PlayerInputState` 5개 불리언.** `wantsToWalk`(GASP가 쓰는 유일한 것) / `wantsToSprint` / `wantsToStrafe` / **`wantsToAim`** / **`wantsToCrouch`**. ABP는 이를 `rotationMode`(OrientToMovement/Strafe/**Aim**) · `stance`(Stand/**Crouch**) · `gait` · `movementDirection`(F/B/LL/LR/RL/RR)로 변환한다. **견착 모드가 이미 데이터 모델에 있다** → `prototypes/2026-09-04_p0-1_ai_drives_mm.md` 4절 | — |
| ~~C-42~~ | ~~`STT_SetCharacterInputState`를 5개 축 전부 노출하도록 확장~~ | **✅ 완료(2026-09-04)** — `/Game/SoldierLab/AI/STT_SetSoldierInputState`. MCP `write_graph_dsl`로 작성, 인터페이스 디스패치 유지. `prototypes/2026-09-04_p0-1_ai_drives_mm.md` 4.3절 | — |
| **C-43** | 새 태스크를 StateTree에 물려 **AI가 실제로 조준·앉기를 구동하는지** | 태스크만 만들었을 뿐 아직 어느 StateTree에도 안 붙였다. GASP StateTree를 복제해 태스크를 교체하고 PIE로 확인해야 한다 | **P0-1 ★ 다음** |
| ~~C-40~~ | ~~`PSS_Relaxed_Loops`가 CMC인가 Mover인가~~ | **✅ 해결 — Mover 쪽이다.** CMC는 `CHT_PoseSearchDatabases`→Dense/Sparse/ExtremeSparse(`PSS_Default`, `M_Neutral_*`), Mover는 `CHT_..._Relaxed`(`PSS_Relaxed_Loops`, `M_Relaxed_*`). CMC로 가면 우리 PSD의 스키마·정규화세트 **두 줄만** 바꾸면 된다 | — |
| ~~C-26/C-27~~ | ~~`Enable_Warping` 값 품질 / 전 구간 1 여부~~ | **✅ 판정 완료** — 직선 클립 전 구간 1로 규약에 맞음. 회전 클립은 애초에 커브를 만들지 않는 것이 정답이었다(8.2절). `prototypes/2026-09-04_p0-4_complete.md` | — |
| ~~C-34~~ | ~~클립 종류 ↔ 커브 세트 **매핑표 확정**~~ | **✅ 해결 (2026-09-04)** — 996클립 전수 실측. `prototypes/2026-09-04_c34_clip_curve_mapping.md` **4절이 확정표**(8종류 × 모디파이어 7단계). 핵심은 표가 아니라 **"Epic의 데이터에는 일관된 표가 없다"** 는 사실 — 같은 "걸으며 90° 회전"이 커브 3/4/6종으로 갈린다. 복제하지 말고 **소비하는 쪽 기준으로 균일 적용**한다 | — |
| ~~C-45~~ | ~~스티어링 커브를 어떻게 채우는가~~ | **✅ 해결 (2026-09-08)** — ~~손 저작~~ → **`UTurnInPlaceCurvesModifier` 자작**(빌드·등록 확인). 스티어링 커브 2종 + PoseSearch 노티파이 2종을 기본값 Add→Apply로 끝낸다. ⚠ **09-05에 적었던 "게이트 0.5초 고정"은 틀렸다** — GASP 두 클립이 회전을 앞부분에서 끝내서 그렇게 보였을 뿐이다. 우리 클립은 0.417s가 회전 10% 지점이라 0.5s면 **돌기도 전에 꺼진다.** 모디파이어가 루트 yaw 프로파일에서 자동으로 잡는다 → `prototypes/2026-09-08_turn_in_place_curves_modifier.md` | — |
| **C-45b** | GASP 회전 클립의 **yaw 프로파일** | 게이트 규칙이 "yaw 진행률 90%"인지 확정하려면 GASP `M_Relaxed_Stand_Turn_090_L`의 0.567s가 그 클립의 90% 지점인지 봐야 한다. 우리 클립에서는 **yaw 90%와 마지막 발 착지가 같은 프레임(41)** 으로 일치했다 [B]. 클립을 우리 폴더로 복제해 같은 모디파이어를 걸면 1분(원본 무손상) | P0-2 중 |
| ~~C-48~~ | ~~회전 클립의 PoseSearch 노티파이 2종 누락~~ | **✅ 해결 (2026-09-08)** — `UTurnInPlaceCurvesModifier`가 `Continuing Pose Cost Bias`(앞) + `Block Transition In`(뒤)을 자동 배치한다. 경계는 스티어링 게이트 시각을 따라간다. PoseSearch 플러그인에 링크하지 않고 `TSoftClassPtr` 경로 + 리플렉션으로 처리 | — |
| ~~C-31~~ | ~~루트 회전 축 = pelvis Y~~ | **❌ 판정 번복 (2026-09-08)** — 09-03의 근거 두 개가 **facing을 검증하지 않았다**("루트 전진"=위치만, "net yaw 88.8°"=회전량만). 실측하니 **루트가 진행 방향보다 56° 틀어져 있었고**(sd 8.3), 견착 골반 블레이드가 루트에 구워진 것이다. Orientation Warping이 그만큼 다리를 틀고 있었다. → **`SoldierRootFacingModifier` 신설**로 교정, 재실측 `mean 0.0 / sd 0.0`. `prototypes/2026-09-08_root_facing_56deg.md` | — |
| **C-59** | **Epic의 비용 편향을 그대로 복제하면 희소한 세트에서 반대로 작동한다** | `Stand_Idles` **+0.10 패널티** / `Stand_TurnInPlace` **−0.20 할인**은 클립 227개를 전제한 값이다. idle이 1개인 우리 세트에서는 idle이 이길 수 없어 **0.5~1초마다 회전 클립으로 튀었다**(움찔 + 왼쪽 회전). 둘 다 0으로 놓아 해결. **나머지 DB들의 편향도 같은 눈으로 재검토할 것** → `../CLAUDE.md` P18 | P0-2 |
| ~~C-49~~ | ~~소규모 DB에서 MM 파라미터 재튜닝이 필요한가~~ | **✅ 불필요 (2026-09-09)** — `blendTime 0.5` / `poseReselectHistory 0.3` / `maxActiveBlends 4` **GASP 기본값으로 전부 원복했고 정상 동작한다.** 우회가 필요했던 진짜 원인은 `bForceRootLock`이었다. **"클립이 짧아서 Epic 상수가 안 맞는다"는 추정은 틀렸다** | — |
| ~~C-52~~ | ~~`WalkSpeeds`가 속도를 정하지 않고 상한으로만 작동한다~~ | **✅ 해결 (2026-09-09)** — 상한이 아니었다. **캐릭터의 기본 `gait`가 `Run`**이라 `walkSpeeds`는 **한 번도 쓰인 적이 없었다.** 실제로 쓰이던 건 `runSpeeds`(500)이고 Lyra Jog 클립은 **582.62 cm/s**로 저작돼 있어 발이 14% 느렸다. 클립의 `movedata_speed`를 실측(매핑표 5.0b의 호버법)해 `walkSpeeds=(291.31,262.18,218.48)` · `runSpeeds=(582.62,407.83,349.57)`로 교체하자 **미끄러짐이 사라졌다** → `prototypes/2026-09-09_lyra_rifle_migration.md` 8.5절 | — |
| ~~C-51~~ | ~~idle ↔ walk 선택을 속도로 가르는 구조가 없다~~ | **✅ 해결 (2026-09-09)** — Lyra 61클립을 GASP `Dense` 구조대로 **PSD 16개**로 나누고 Chooser를 **3열(Stance·MovementState·Gait) × 16행**으로 지었다. 속도가 아니라 **`Gait` 열거형**으로 가르는 것이 GASP의 실제 방식이었다(`= Walk` / `≠ Walk`). 8절 | — |
| ~~C-58~~ | ~~옆·뒤·앜기 속도는 GASP 비율로 추정한 값~~ | **✅ 해결 (2026-09-11)** — 실측 결과 **Lyra 라이플 세트는 전 방향 단일 속도**였다(Walk 291.31 / Jog 582.62 / Crouch 291.31, 방향 간 오차 0.001). GASP 비율(1:0.9:0.75 / 1:0.7:0.6)을 옮겨온 탓에 옆 −30% · 뒤 −40% 어긋나 **Loop가 영영 선택되지 않고 Start/Stop이 매 걸음 재선택됐다.** 세 백터를 모두 균일하게 고쳐 해결 → `prototypes/2026-09-09_lyra_rifle_migration.md` 9절 · `../CLAUDE.md` P30·P31 | — |
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
| **C-30** | 루트모션 생성 경로 — **A) 리타깃 Root Motion op** vs **B) `EncodeRootBoneModifier`** | **보류.** A의 `bRotateWithPelvis=true`로 회전 우려가 해소될 수 있어 A가 탈락한 것은 아니다. 현재 B로 진행(A에 [C-32] 버그가 있어서). 판정은 turn/pivot 클립 확보 후. `prototypes/2026-09-03_mixamo_retarget_setup.md` 3절 | **P0-4** |
| **C-31** | `EncodeRootBone`의 **회전 설정** — 어느 본의 어느 축을 써야 루트 헤딩이 맞는가 | 현재 비워둠(회전 없음). `Yaw=Atan2(-Heading.X, Heading.Y)`라 축을 틀리면 옆을 보고 걷는다. **직선 클립으로는 검증 불가** | **P0-4** |
| **C-32** | ★ 리타깃 **Root Motion op을 켜면 타깃 골반이 바닥에 붙는다** — 원인 미규명 | `bPropagateToNonRetargetedChildren`을 꺼도 재현(가설 기각). 현재 op을 끄는 것으로 우회 중. **A안 채택의 전제** | **P0-4** |

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
| Q8 | 캐릭터 메시 (리스킨/신규/MetaHuman) | P1 후반 |
| ~~Q9~~ | 유료 팩 구매 | ✅ 배제 |
| Q10 | 견착 전환 동작 조달 (A/B/C안) | P0-2 결과 후 |
| ~~Q11~~ | ~~이동 시스템 — CMC vs Mover~~ | ✅ **CMC 확정 (2026-09-04)**. `PSD_Soldier_Walk_Test`를 `PSS_Default` + `PSN_Dense_All`로 전환 완료. 남은 일: `NPCLevel`의 배치 캐릭터를 CMC로 교체·재확인 → **[C-41]**. 아래는 판단 근거 |
| | (Q11 근거) | **권고: CMC.** 설계 3.1절 전제·`titan_example` 정합·Mover의 `NetworkPrediction`/`ChaosMover` 의존(P5와 얽힘) 때문. **이관 비용은 거의 0** — AI 스택이 `BPI_SandboxCharacter_Pawn` 인터페이스만 쓰고 두 캐릭터가 모두 구현한다. 우리 PSD는 스키마→`PSS_Default`, 정규화세트→`PSN_Dense_All` 두 줄만 변경. 남는 일은 `NPCLevel`의 배치 캐릭터 교체·재확인 → `prototypes/2026-09-04_p0-1_ai_drives_mm.md` 3절 |

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
| ~~U6~~ | ~~`Relaxed` 티어의 성격 — LOD인가 스타일인가~~ | **✅ 해결(2026-09-04) — 둘 다 아니다. `Mover` 경로의 DB 세트다.** LOD 티어는 Dense/Sparse/ExtremeSparse 셋(`PSS_Default` + `M_Neutral_*` 클립)이고, `Relaxed`는 Mover ABP 전용(`PSS_Relaxed_Loops` + `M_Relaxed_*` 클립). `prototypes/2026-09-04_p0-1_ai_drives_mm.md` 3.2절 |
| ~~U7~~ | `RemapCurves` 역할 | ✅ 해결 — 3.3b절. 발 접지 구동원 |

---

## W — 분석에서 파생된 작업

문서: `animation/2026-09-02_gasp_abp_analysis.md` 13.1절

| # | 항목 | 시점 |
|---|---|---|
| W1 | 자산 반입에 **접지 커브 생성 단계** 추가 | ✅ 문서 반영 완료 / 구현은 P0-4 |
| W2 | 교란을 **Control Rig 연산**으로 구현 | P1 |
| W3 | `AIController`가 컨트롤 로테이션으로 조준 구동 | P0-1 |
| ~~W4~~ | ~~`TrajectoryGenerationData` AI용 재튜닝~~ | **✅ 불필요 (2026-09-04)** — [C-1] 잠정 통과. 기본값으로 급선회가 버틴다. Epic이 Steering 노드를 "작업 중"으로 표기해 첫 의심 대상이었으나 문제가 나타나지 않았다 |
| W5 | `Disable_AO` 커브 패턴을 우리 무기 시스템에 채택 | P1 |
