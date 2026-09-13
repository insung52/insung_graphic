# P0-1 · AI가 Motion Matching을 구동하는가 — **증명됨**

2026-09-04 / **성공 (런타임 관측)** / GASP에 이미 구현돼 있었다. 다만 **AI 예제가 Mover 경로에만 붙어 있어** 이동 시스템 선택이 결정 항목으로 올라왔다 → **[Q11]**

관련: [C-1] 미측정 · **[Q11] 신규** / 관련 문서: `design/2026-09-01_architecture.md` · `CURRENT_STATE.md` 3.1절

---

## 1. 결론

**AI가 조종하는 NPC가 실제로 Pose Search를 돌린다.** Rewind Debugger 녹화로 직접 확인했다.

```
SandboxCharacter_Mover_ABP_C_0
└── Pose Search
    ├── PSD Locomotion Transitions      Search Time : 38.752
    ├── PSD Idle Transitions            Best Index  : 3
    ├── PSD Locomotion Loops            Search Cost : 0.648
    └── PSD_CHT_SmartObject_BenchA…
└── Blend Weights
    ├── M_Relaxed_Walk_Reface_Start_1…      ← 로코모션
    ├── M_interaction_bench_into_R_090…     ← 스마트오브젝트 진입
    ├── M_interaction_bench_idle_loop
    └── M_interaction_bench_out_to_sta…     ← 이탈
```

DB 4개를 동시에 질의하고 비용을 계산해 클립을 뽑으며, 로코모션과 벤치 상호작용을 오간다.

## 2. 어떻게 되어 있나 — 배선의 실체

`/Game/Blueprints/AI/` 에 전부 있다:

```
AIC_NPC_SmartObject                                          AI 컨트롤러
StateTree/ST_NPC_SandboxCharacter_Patrol_Subtree
StateTree/ST_NPC_SandboxCharacter_SmartObject
StateTree/TasksAndConditions/STT_SetCharacterInputState  ★   핵심
                             STT_FindRandomLocation
                             STT_FocusToTarget / STT_ClearFocus
                             STT_FindSlotTransforms
                             STT_CharacterIgnoreCollisionsWithOtherActor
```

**`STT_SetCharacterInputState`** [A] — MCP로 그래프를 읽은 실물:

```lisp
parent : /Script/StateTreeModule.StateTreeTaskBlueprintBase
vars   : Character, WantsToWalk

(event EventEnterState (Transition)
  (bind _pawn (Utilities|Casting|CastToBPI_SandboxCharacter_Pawn (Variables|Context|GetCharacter)))
  (Setters|Set_CharacterInputState _pawn false (Variables|Default|GetWantstoWalk) false false false))
```

**AI가 플레이어와 똑같은 인터페이스로 input state를 채운다.** 그 뒤는 동일 경로다:

```
StateTree → Set_CharacterInputState → 이동 컴포넌트 → 궤적 생성 → Pose Search → 포즈
```

설계 3.1절이 엔진 함수 시그니처만 보고 내린 "자체 궤적 소스 불필요" 결론이
**에셋·런타임 양쪽에서 확인됐다.**

### 2.1 `BPI_SandboxCharacter_Pawn` — L3↔L4 계약의 원형

이 인터페이스가 이미 "AI ↔ 애니메이션" 경계를 끊어놓고 있다. 우리 설계의 계층 간 계약을
새로 만들 게 아니라 **이걸 확장하는 방향이 맞다.**

| 함수 | 우리 설계에서의 쓸모 |
|---|---|
| `Set_CharacterInputState(Desired Input State)` | L3 → L4 구동 입력 |
| **`AddTarget` / `RemoveTarget`** | L2가 고른 표적을 내려보내는 창구 |
| `Get_PropertiesForAnimation` | ABP가 캐릭터 상태를 읽는 **단일 창구** — 설계 P3("L4는 Intent와 월드만 읽는다")와 같은 구조 |
| `Get_MMIResult` | Motion Matching Interaction 결과 |
| `Set_IsPlayingRootMotion` | 루트모션 재생 플래그 |

## 3. ★ 그런데 — AI 예제는 **Mover** 경로에만 붙어 있다 **[Q11]**

GASP는 캐릭터가 **두 벌**이다:

| | 이동 시스템 | ABP | Chooser |
|---|---|---|---|
| `SandboxCharacter_CMC` | CharacterMovementComponent | `SandboxCharacter_CMC_ABP` | `CHT_PoseSearchDatabases` · `CHT_CMCCharacterAnimations` |
| **`SandboxCharacter_Mover`** | **Mover 플러그인** | `SandboxCharacter_Mover_ABP` | `CHT_PoseSearchDatabases_Relaxed` · `CHT_MoverCharacterAnimations_PoseMatch` |

Rewind Debugger의 트랙 이름이 **`SandboxCharacter_Mover_ABP_C_0`** 이었다.
→ **`NPCLevel`의 AI NPC는 Mover 쪽이다.**

### 3.1 왜 문제인가

- **설계 3.1절의 전제가 CMC였다** — `PoseSearchGenerateTrajectory(forCharacter)`가 `ACharacter`를
  받는다는 점이 "AI가 CMC를 구동하면 궤적이 같은 경로로 나온다"의 근거였다
- **`titan_example` 현행도 CMC** — 표준 UE 캐릭터 AI + NavMesh
- **Mover는 `NetworkPrediction` / `ChaosMover` 의존을 끌고 온다** — 설계 P5(리슨서버 멀티플레이
  권한 게이트)와 얽힌다
- 반대로 **CMC로 가면 AI 예제를 그대로 못 쓴다.** `AIC_NPC_SmartObject`와 StateTree 3종을
  CMC 캐릭터로 옮기는 작업이 생긴다

### 3.2 두 경로는 **DB 세트가 완전히 갈린다** [A] — [C-40] 해결

```
CMC   : SandboxCharacter_CMC_ABP   → CHT_PoseSearchDatabases
                                     └→ CHT_..._Dense / _Sparse / _ExtremeSparse
Mover : SandboxCharacter_Mover_ABP → CHT_PoseSearchDatabases_Relaxed  (PSD 71개)
```

`PSD_Relaxed_*`를 참조하는 것은 `CHT_PoseSearchDatabases_Relaxed`와 `PSN_Relaxed_All`뿐이다.

| DB | 스키마 | 정규화 세트 | 클립 스타일 |
|---|---|---|---|
| `PSD_Dense_Stand_Walk_Loops` (18개) | **`PSS_Default`** | `PSN_Dense_All` | `M_Neutral_*` |
| `PSD_Sparse_Stand_Walk_Loops` (11개) | **`PSS_Default`** | `PSN_Sparse_All` | `M_Neutral_*` |
| `PSD_Relaxed_Stand_Walk_F_Loops` (5개) | **`PSS_Relaxed_Loops`** | `PSN_Relaxed_All` | `M_Relaxed_*` |

> **[U6] 해결**: `Relaxed`는 LOD 티어가 아니다. **Mover 경로의 DB 세트이자 `M_Relaxed_*`
> 스타일 클립군**이다. LOD 티어는 Dense / Sparse / ExtremeSparse 셋이고 그쪽은
> `M_Neutral_*` 클립에 `PSS_Default` 스키마를 쓴다.

**→ 우리 `PSD_Soldier_Walk_Test`는 `PSS_Relaxed_Loops`를 썼으므로 Mover 쪽 규격이다.**
CMC로 가면 스키마를 **`PSS_Default`**, 정규화 세트를 `PSN_Dense_All`(또는 Sparse)로 바꿔야 한다.
클립 자체(커브 5종·마커·루트모션)는 그대로 쓸 수 있다 — **바뀌는 것은 PSD 설정 두 줄뿐이다.**

### 3.3 ★ AI 스택은 이동 시스템과 무관하다 [A]

```
BPI_SandboxCharacter_Pawn 를 구현하는 것:
  SandboxCharacter_CMC     ✅
  SandboxCharacter_Mover   ✅
  (그 외 AC_TraversalLogic, TargetDummy, 카메라 디렉터 등)

STT_SetCharacterInputState 는 이 인터페이스로만 접근한다.
```

**두 캐릭터가 모두 같은 인터페이스를 구현한다.** StateTree 태스크는 인터페이스에만 의존하므로
**AI 스택(AIController + StateTree 3종 + 태스크 6종)은 CMC 캐릭터에 그대로 붙는다.**

→ **[Q11]의 이관 비용이 사실상 0이다.**

### 3.4 권고 — CMC

| 근거 | |
|---|---|
| 설계 3.1절 전제 | `PoseSearchGenerateTrajectory(forCharacter)`가 `ACharacter`를 받는다 |
| `titan_example` 정합 | 현행이 표준 UE 캐릭터 AI + NavMesh |
| 의존성 | Mover는 `NetworkPrediction`/`ChaosMover`를 끌고 오고 P5(리슨서버 권한 게이트)와 얽힌다 |
| 이관 비용 | **AI 스택은 인터페이스 기반이라 그대로 이식**(3.3절). PSD 설정 두 줄만 변경(3.2절) |

### 3.5 ★ 이관 비용이 0인 것을 넘어, **이미 배치돼 있다** [A]

`NPCLevel`의 액터를 조회한 결과:

```
SandboxCharacter_CMC_C_1 / _3 / _5      AIControllerClass: AIC_NPC_SmartObject_C   AutoPossessAI: PlacedInWorld
SandboxCharacter_Mover_C_1 / _2         AIControllerClass: AIC_NPC_SmartObject_C   AutoPossessAI: PlacedInWorld
```

**CMC 3기와 Mover 2기가 똑같은 AI 컨트롤러로 나란히 돌고 있다.**
3.3절에서 인터페이스 구조로 추론한 "그대로 붙는다"가 **Epic의 실제 배치로 확인**됐다.

2026-09-04의 Rewind Debugger 관측이 Mover 캐릭터였던 것은 **단순히 그 액터를 골랐기 때문**이다.
→ CMC 액터를 선택하면 같은 검증을 CMC 경로에서 그대로 할 수 있다. **교체 작업 없음.**

**[Q11] 결론: CMC 확정.** 우리 `PSD_Soldier_Walk_Test`는 `PSS_Default` + `PSN_Dense_All`로 전환 완료.

## 3.6 CMC 경로 실측 확인 [A]

Rewind Debugger에서 `SandboxCharacter_CMC_C_1`을 선택한 결과:

```
SandboxCharacter_CMC_ABP_C_0
└── Pose Search
    ├── PSD_Dense_Stand_Walk_Starts
    ├── PSD_Dense_Stand_Idles
    ├── PSD_Dense_Stand_Walk_Loops
    ├── PSD_Dense_Stand_Walk_Stops
    └── PSD_CHT_SmartObject_BenchAr…
```

**예측대로 `Dense` 계열**이고 트랙에 비용 곡선이 차 있다. → **[C-41] 실측 완료.**
우리 PSD를 `PSS_Default` + `PSN_Dense_All`로 바꾼 것이 맞는 선택임도 같이 확인됐다.

---

## 4. ★ [C-39] — AI가 채워야 할 축이 무엇인가

`S_PlayerInputState` (인터페이스로 넘어가는 구조체) [A]:

```
wantsToSprint  : bool
wantsToWalk    : bool     ← STT_SetCharacterInputState 가 유일하게 세팅하는 것
wantsToStrafe  : bool
wantsToAim     : bool     ★ 견착
wantsToCrouch  : bool     ★ 엄폐 자세
```

`S_CharacterPropertiesForAnimation` (ABP가 읽는 창구) [A]:

```
inputState        : S_PlayerInputState
movementMode      : OnGround / InAir / Sliding / Traversing / Ragdoll / Flying
stance            : Stand / Crouch
rotationMode      : OrientToMovement / Strafe / Aim        ★★ 견착 모드가 이미 있다
gait              : Walk / Run / Sprint
movementDirection : F / B / LL / LR / RL / RR              ← DB 방향 버킷과 1:1
actorTransform    : ...
```

### 4.1 이것이 뜻하는 것

**우리 설계가 필요로 하는 축이 전부 이미 데이터 모델에 있다.**

| 우리 설계 요구 | GASP 대응 |
|---|---|
| 견착 / 총내림 | `wantsToAim` → `rotationMode = Aim` |
| 엄폐 자세(앉기) | `wantsToCrouch` → `stance = Crouch` |
| 스트레이프 이동 | `wantsToStrafe` → `rotationMode = Strafe` |
| 보행/구보/전력질주 | `wantsToWalk` / `wantsToSprint` → `gait` |
| 8방향 이동 | `movementDirection` F/B/LL/LR/RL/RR |

**`rotationMode = Aim`은 "진행 방향이 아니라 조준 방향을 향한다"는 뜻이고, 이것이 정확히
견착 이동이다.** 아키텍처를 새로 만들 필요가 없다.

### 4.2 그래서 진짜 구멍은 아키텍처가 아니라 **데이터**다

GASP는 축을 다 갖고 있지만 **그 축을 채울 견착 클립이 없다**(비무장 로코모션 1,879개, 견착 0개).
`AimOffset` 42개는 시선/상체 오프셋이지 무기 조준이 아니다.

→ **`CURRENT_STATE.md` 3.4절("조달 요청의 성격이 바뀌었다")의 결론이 재확인된다.**
우리가 만들 것은 시스템이 아니라 **데이터와, 그 축을 AI가 채우는 얇은 배선**이다.

### 4.3 ✅ `STT_SetSoldierInputState` — 5개 축 전부 노출 (2026-09-04 작성 완료)

```
/Game/SoldierLab/AI/STT_SetSoldierInputState     (GASP 태스크 복제 + 확장)
vars : Character · WantsToWalk · WantsToSprint · WantsToStrafe · WantsToAim · WantsToCrouch
```

```lisp
(event EventEnterState (Transition)
  (bind _pawn (CastToBPI_SandboxCharacter_Pawn (Variables|Context|GetCharacter)))
  (Setters|Set_CharacterInputState (Utilities|ToObject(Interface) _pawn)
    (Utilities|Struct|MakeSPlayerInputState
      (GetWantstoSprint) (GetWantstoWalk) (GetWantstoStrafe) (GetWantstoAim) (GetWantstoCrouch))))
```

**MCP `write_graph_dsl`로 작성했다.** 컴파일·저장 완료.

#### 작성 시 걸린 것 — DSL 읽기/쓰기 이름이 다르다 ★

| | 읽기(`read_graph_dsl`) 출력 | 쓰기에 필요한 실제 `type_id` |
|---|---|---|
| 세터 | `Setters\|Set_CharacterInputState` | **`Setters\|SetCharacterInputState`** (밑줄 없음) |
| 구조체 생성 | — | **`Utilities\|Struct\|MakeSPlayerInputState`** (`S_` 아님) |

**읽기 출력을 그대로 되먹이면 "node does not exist"로 실패한다.**
반드시 `find_node_types(graph, type_id_filter, context_pins=[])`로 실제 id를 찾고,
`get_node_type_pins`로 핀 이름을 확인할 것. 구조체 Make 노드의 핀 이름에는
**GUID 접미사**가 붙는다(`WantsToAim_7_3A4AA0AA4DE529A3B213E68BECD246F8`).

#### 인터페이스 호출을 강제해야 한다 ★

`Setters|SetCharacterInputState`(일반 함수 버전)로 쓰면 컴파일은 되지만
**`CastToSandboxCharacter_CMC` 하드 캐스트가 자동으로 끼어든다** — Mover 캐릭터에서 깨진다.
**`Setters|SetCharacterInputState(Message)`** 를 쓰면 `Utilities|ToObject(Interface)`를 거치는
인터페이스 디스패치가 되어 원본과 동일해진다.

---

## 5. 아직 안 잰 것

| ID | 항목 |
|---|---|
| **C-1** | AI 구동 시 `TrajectoryGenerationData` 값 — 급선회에서 포즈가 튀는가. **아직 급선회 상황을 못 봤다**(NPC가 순찰·벤치만 한다) |
| **C-39** | GASP NPC는 `WantsToWalk` 외 4개 축을 **전부 `false`로 둔다.** 조준·앉기·달리기를 AI가 구동하려면 우리가 채워야 한다 |

## 5. 재현 방법 — Rewind Debugger

```
Tools → Debug → Rewind Debugger  (+ Rewind Debugger Details)
1. "Auto Record" 켜기 → PIE 재시작           (Automatically start recording when PIE is started)
2. NPCLevel 20~30초 실행 → "Stop Recording"
3. 왼쪽 액터 트리에서 NPC 선택 → Pose Search 트랙 펼치기
4. "Previous Frame" / "Next Frame" 으로 한 프레임씩 검사
```

`Pose Search` 트랙이 값을 채우고 있으면 **AI가 MM을 구동한다는 직접 증거**다.
곁들여 `DDCvar.DrawCharacterDebugShapes 1`로 궤적·접지를 뷰포트에 그릴 수 있다.
