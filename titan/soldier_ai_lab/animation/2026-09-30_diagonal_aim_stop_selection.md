# 조준 대각선 이동 시 Stop 선택 → 발 끌림 (임시 편향 조정) + 맹목사격 임시 비활성

2026-09-30 / 진행중(원인 확정·임시 완화 적용·편향 튜닝 중단, 근본 수정(대각선 클립 추가) 대기 [W124]) / 보는 방향과 이동 방향이 ~45°면 MM이 Loop 대신 Stop을 고른다 — 우리 Loops DB가 4방향뿐이라서. Stops/Loops 편향으로 임시 완화, 맹목사격은 cvar로 잠시 끔.

관련: `animation/prototypes/2026-09-09_lyra_rifle_migration.md` 9절(09-11 P30/P31 — 방향별 속도 불일치, 이번과 서명이 같지만 원인은 다름) · `animation/2026-09-29_movement_policy_and_playrate_band.md`(이동 정책) · `animation/prototypes/2026-09-03_gasp_curve_manifest.md:129`(Stop엔 `Enable_Warping` 없음 = 설계).

---

## 1. 증상 [A · 사용자 관측]

- 조준 + 걷기 + 대각선(WA/WD/SA/SD)에서 발이 질질 끌린다. 단일 키나 마우스로 방향을 돌리면 복구, 다시 대각선이면 재발.
- 달리기·비조준에서도 방향 전환 중 45° 부근을 지날 때 ~0.2초 미끄러짐.
- 아군·적군 모두. GASP 원본 `SandboxCharacter_CMC`(총 없음)는 어떤 방향이든 Pivot 후 Loop로 안정.

## 2. 관측 (Rewind Debugger / Pose Search)

- Blend Weights: 대각선 이동 내내 `Walk_Left_Stop`이 클립 길이마다 재선택, Loop는 첫 0.3초뿐.
- 비용(조준 W+D, Walk DB): **Stops 최저 0.533**(Traj ~0.51, Group ~0.02) vs **Loops 최저 0.702**(Traj 0.55~0.60, Group 0.10~0.17). Loop가 궤적·포즈 **양쪽**에서 짐.
- 재생 중 Stop은 `continuingPoseCostBias −0.3`로 0.369 → 클립 끝까지 유지.
- Stop 재생 중 워핑 화살표가 사라지는 것은 Stop에 `Enable_Warping`이 없어서(설계) — 원인이 아니라 결과.
- 차단 A/B(모두 무변화): `a.AnimNode.OrientationWarping.Enable 0` · `a.AnimNode.FootPlacement.Enable(.Lock) 0` · `SoldierLab.Move.Enabled 0` · `SoldierLab.AIBridge.Native 0` · `SoldierLab.Pose.LeftHandIK/WalkLowered/JogLowered 0`.

⚠ 디버그 표시 켜기: **`a.AnimNode.MotionMatching.DebugDrawInfo 1`이 스위치**다. `…Verbose 1`만 치면 아무것도 안 나온다(P141 재확인).

## 3. 원인 [A · MCP 에셋 대조]

| | GASP `PSD_Dense_Stand_Walk_Loops` | 우리 `ALLY_PSD_Rifle_Stand_Walk_Loops` |
|---|---|---|
| 방향 | F·B·**FL·FR·BL·BR**·F_L·F_R·LL·LR·RL·RR + 셔플/전환 (18) | Fwd·Bwd·Left·Right (**4**) |

GASP Stops DB에도 대각선 Stop(FL/FR/BL/BR)과 제자리 회전이 있다. 우리 쪽은 45° 쿼리 궤적에 맞는 Loop가 없어 4방향 Loop 모두 45° 어긋난 궤적 비용을 받고, 미래 +1.0초 샘플이 없는 `PSS_Stop` 스키마의 Stop이 이긴다.

Lyra 원본에는 대각선이 있다 — `Characters/Mannequins/Anims/Rifle/{Walk,Jog}/MF_Rifle_*_{Fwd,Bwd}_{Left,Right}` 8개. 09-09 이관 때 `_MF`(여성 변형)라 "나중에"로 미뤘고(`lyra_rifle_migration.md:40`) 09-17 분리 때 사본에서 빠졌다.

정상으로 확인된 것(GASP 원본과 대조): DB 16개 설정 · 스키마 2종 · PSN 17개 · Chooser 필터 열 · ABP CDO(공유 변수 차이 0) · 캐릭터 CDO(의도된 차이 + `wantsToStrafe` 기본값 true→false) · CMC(차이 3개: `bAlwaysCheckFloor`·`bEnablePhysicsInteraction` false(09-21), `bTickBeforeOwner` false(09-18)) · 클립 커브/노티파이/모디파이어 스택(09-09 문서 5절 사양과 일치).

## 4. "왜 지금" — 미확정

- 캡처 당시 플레이어 `MaxWalkSpeed = 247.61 = 291.31 × 0.85` — **09-29 이동 정책이 플레이어에게도 적용**되고 있었다(`SoldierMovementProfile`에 `IsPlayerControlled` 게이트 없음). Loop(291)와 속도까지 어긋나 대각선 균형이 무너졌을 가능성. `Enabled 0` 상태에서 291.31 확인 후 비용 재측정은 미실시.
- 거의 모든 로코모션 클립이 `Driving_<클립명>` 레벨 시퀀스에 링크돼 있다(디자이너 컨트롤 릭 재베이크). 커브·노티파이 이름은 유지, 모션 값 변화는 원본이 없어 비교 불가.

## 5. 임시 완화 — 적용·저장됨 (P4 체크아웃, 미제출)

| DB (아군·적군 각각) | 값 |
|---|---|
| `*_PSD_Rifle_Stand_Walk_Stops` · `*_Stand_Jog_Stops` | `baseCostBias` 0 → **0.2** (인덱싱 시 구워짐 — `PoseSearchAssetIndexer.cpp:199`) |
| `*_PSD_Rifle_Stand_Walk_Loops` · `*_Stand_Jog_Loops` | `continuingPoseCostBias` −0.01 → **−0.05** (런타임 전용, 재인덱싱 불필요) |

사용자 평가: 안정 구간은 좋아졌으나 전환 시 버벅임·대각선 전환 Stop은 남음, 달리기 등 다른 동작에서도 Loop가 안 나오는 경우가 많음 → **편향 튜닝은 중단.** 값은 임시방편으로 유지.

## 6. 맹목사격 임시 비활성 (코드)

실전 시뮬 피드백("부자연스럽다")으로 잠시 끔. `Source/SoldierLab/AI/SoldierEngagement.cpp`:

- 신규 cvar **`SoldierLab.Engagement.BlindFire`**(기본 **0**). `PlanAperture`의 Blind 후보 3종(위 · 좌 · 우)을 이 값이 1일 때만 제시.
- 0이면 Lean·일어서 쏘기만 남고, 그것도 막히면 엄폐 뒤에 머문다. 맹목사격 코드 경로는 지우지 않았다.
- **다시 켜기**: 콘솔 `SoldierLab.Engagement.BlindFire 1`(세션 한정) / 영구는 `DefaultEngine.ini [ConsoleVariables]`에 `SoldierLab.Engagement.BlindFire=1` 또는 코드 기본값을 1로.

## 7. 남은 것

1. **근본 수정**: GASP 기준으로 처음부터 재점검 — 대각선 Loop/Stop 추가(Lyra `MF_` 8개 리타깃 + 모디파이어 스택 A/B, 아군·적군) 또는 동등 구조. 추가 후 5절 편향은 원복 검토.
2. `SoldierMovementProfile`을 플레이어 조작 캐릭터에서 제외.
3. 적군 클립 `contact_l/r` 누락: `Enemy_MM_Rifle_Jog_Fwd/Bwd/Left`(양쪽), `Jog_Right`·`Walk_Right`·`Crouch_Walk_Fwd`·Jog Pivot/Start 일부(한쪽). 아군은 정상.
4. Crouch Walk Stops/Loops에는 5절 편향 미적용.

## 8. 등록된 항목 (2026-09-30, 문서 정리)

`OPEN_ITEMS.md` 에 등록: **[W124]** 7절 1·4(대각선 Loop/Stop 추가 + 편향 원복 검토, Crouch 포함) · **[W125]** 7절 2(`SoldierMovementProfile` 플레이어 제외) · **[W126]** 7절 3(적군 `contact_l/r` 커브) · **[C-177]** 4절("왜 지금" — `SoldierLab.Move.Enabled 0` 상태 비용 재측정) · **[Q52]** 6절(맹목사격 다시 켤지) · **[W129]** P4 제출(5절 PSD 8개 포함). 원칙 **P200**(MM DB 방향 커버리지 — 쿼리 궤적에 맞는 Loop 가 없으면 Stop 이 이긴다, 편향은 임시방편) → `CLAUDE.md` 5절.
