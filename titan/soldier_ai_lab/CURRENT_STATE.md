# 현재 상태 — soldier_ai_lab

2026-09-09 / **★ 견착 로코모션 동작** / Lyra 라이플 세트 61클립이 PSD 16개 + Chooser 16행으로
들어갔고 **출발·정지·피벗·제자리회전·앉기·달리기가 견착 상태로 동작한다.** 플레이어 직접 조작(WASD)도
붙었다. 조준 오프셋(상하좌우)까지 동작. 다음은 로우레디와 사격 배선.

> **오늘 한 일 전문: `prototypes/2026-09-09_lyra_rifle_migration.md`** — 반입·PSD·Chooser·
> 디버깅 전 과정. **다음 세션은 이 문서만 보면 로코모션 상태를 전부 안다.**

> **전문: `prototypes/2026-09-04_p0-4_complete.md`** — 확정된 11단계 파이프라인, 모디파이어
> 순서 규약, PSD 설정값이 여기 있다. **반입 작업을 하려면 이 문서만 보면 된다.**
>
> **★ 계획 대비 무엇이 달라졌는지: `2026-09-04_p0_checkpoint.md`** — 리스크 장부가 뒤집힌 것,
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
> 절차·설정값은 **`prototypes/2026-09-04_p0-4_complete.md`**. 아래는 진행 이력이다.

<details>
<summary>진행 이력 (접힘)</summary>

### ① P0-4 · 반입 파이프라인 검증 (1일) ★ 최우선

**이제 모든 것의 전제가 됐다.** 커브 3종을 만들 수 있는지가 조달 계획 전체를 좌우한다.

> **2026-09-03 진전**: 최대 리스크였던 `Enable_Warping` **생성 수단 문제는 끝났다.**
> GASP에 `AM_WarpingAlpha` 모디파이어가 이미 있다 → **[C-25] 해결**.
> `prototypes/2026-09-03_enable_warping_curve_generation.md`
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
      · `prototypes/2026-09-04_foot_contact_curve_modifier.md`

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
> 전문: `prototypes/2026-09-04_p0-1_ai_drives_mm.md`

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

> **`prototypes/2026-09-04_c34_clip_curve_mapping.md` 4절이 확정표다.** 996클립 전수 실측.
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
> 전체 기록: **`prototypes/2026-09-09_walk_quality_debugging.md`**
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
| 3 | **[C-44] 급선회 판정** | `rotationRate.yaw = 360` 상수가 GASP 기본값임을 확인했다. 올려보고 데이터 부족이 드러나는지 본다 |
| 4 | **[C-58]** 옆·뒤·앉기 속도 실측 | 커브 호버 5회 |
| 5 | **AI 상위 계층** — 인지·위협평가·엄폐·분대 | `drafts/` 의 초안 3건 검토 후 착수 |

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
| Q8 | 캐릭터 메시 (기존 리스킨 / 신규 / MetaHuman) | P1 후반으로 미룸 |
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
