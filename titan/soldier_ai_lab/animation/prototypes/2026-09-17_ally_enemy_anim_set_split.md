# 아군/적군 애니메이션 세트 분리 — 스켈레톤 1벌 · 애니메이션 2벌

2026-09-17 / 완료(런타임 분기 5축 컴파일·저장까지) / 스켈레톤을 가르자는 안을 기각하고 **애니메이션 247개를 통째로 복제해 `Animations_Enemy/`(`Enemy_`) · `Animations_Ally/`(`ALLY_`) 두 벌**로 갈랐다. ABP·캐릭터에 `UseAllyAnimSet` 하나를 심어 **로코모션·AO·총내림 델타·블라인드파이어·몽타주 5축**을 진영별로 분기.

관련 항목: **[C-120]** · 신규 **[C-129]** **[W73]** / 관련 문서: `IMPLEMENTED.md` 2.1~2.5e · 3절, `assets/2026-09-14_designer_guide.html`(경로 전면 갱신), `assets/2026-09-14_design_team_animation_handoff.md`, 같은 세션의 `ai/2026-09-17_hit_death_three_causes.md`(이 분리가 일으킨 사고 3건), 원칙 **P134~P140**

---

## 1. 무엇을 하려 했나

아군(`soldier_T`)과 적군(`new_enemy_T`)은 **본 길이가 다르고 총기도 갈라질 예정**이다. 한 벌의 애니메이션을 두 체형이 같이 쓰면 어느 한쪽에 맞춘 손 위치·팔꿈치가 다른 쪽에서 어긋난다.

**판정 기준**: ① 두 진영이 서로 다른 클립을 재생하고 ② 한쪽 클립을 고쳐도 다른 쪽이 안 바뀌며 ③ ABP·PSD·Chooser·AO·몽타주가 **한 벌씩 더 생기되 배선 작업이 두 배가 되지는 않을 것**.

## 2. ★ 스켈레톤을 가르지 않은 이유 (기각 근거) [A]

먼저 나온 안은 **스켈레톤 에셋을 둘로 나누는 것**이었다. 기각했다 — 스켈레톤이 갈리면 그 위의 거의 전부가 두 벌이 된다:

| 갈리는 것 | 규모 |
|---|---|
| `SoldierCharacter_ABP` | 노드 약 1200개. 이후 모든 애님 그래프 수정이 **두 번** |
| PSD 17 + PSN 1 | 스키마·정규화 세트까지 |
| Chooser | 4열 × 17행 |
| AO 블렌드스페이스 2 · 몽타주 21 · 블렌드 마스크 | |

게다가 **P76**(같은 스켈레톤 에셋이면 리매핑이 항등)의 이점이 사라진다 — 스켈레톤이 다르면 엔진이 레스트 포즈 델타로 회전을 보존하므로 A-포즈↔T-포즈 차이만큼 팔이 들린다. 이 프로젝트가 아군 메시를 `Assign Skeleton` 으로 올린 것이 정확히 그 이유였다(`animation/prototypes/2026-09-13_ally_mesh_on_mannequin_skeleton.md`).

**결론: 스켈레톤 1벌 유지 + 애니메이션 세트만 2벌.** 갈라지는 것은 클립·PSD·Chooser뿐이고 ABP는 한 벌로 남는다(그 대신 ABP 안에 진영 분기가 들어간다 — 4절).

**작업 순서에 대한 사용자 전략** [A]: 적군 시나리오가 최우선이므로 **모든 애니메이션을 적군 기준으로 먼저 맞추고**, 아군 세트는 그 복제본을 나중에 재피팅한다. 그래서 디자인팀 검수 기준도 적군 세트다(→ 6절). 작업량은 2배가 아니라 **1.2~1.4배** 로 본다 [B].

## 3. 어떻게 했나

### 3.1 최종 폴더 구조 [A] (디스크 실측)

```
/Game/SoldierLab/Animations_Enemy/
    Animations/   Rifle/{Idles,Loops,Starts,Stops,Pivots,Poses,TurnInPlace,_Extra} · Actions/
    PoseSearch/   Enemy_CHT_Soldier_Databases · Enemy_PSD_Soldier_Walk_Test · Rifle/(PSD 17 + PSN 1)
                                                                        = 247개, 전부 Enemy_ 접두사
/Game/SoldierLab/Animations_Ally/    같은 구조, 247개, 전부 ALLY_ 접두사

/Game/SoldierLab/Animation/   (단수, 그대로) SoldierCharacter_ABP · CHT_Soldier_CharacterAnimations

소멸: /Game/SoldierLab/Animations/ · /Game/SoldierLab/PoseSearch/
```

예: `Rifle/Idles/MM_Rifle_Idle_ADS` → `Animations_Enemy/Animations/Rifle/Idles/Enemy_MM_Rifle_Idle_ADS` / `Animations_Ally/Animations/Rifle/Idles/ALLY_MM_Rifle_Idle_ADS`.

⚠ 옛 `Rifle/_MF/` 39개(미사용 보관물)는 사본에 따라오지 않았다.

### 3.2 복제 — Advanced Copy [A]

**UE 5.8 콘텐츠 브라우저에는 Advanced Copy 메뉴가 없다.** 엔진 소스로 확인했다 — `BeginAdvancedCopyPackages` 는 `Developer/AssetTools/` 에만 있고 ContentBrowser 어디서도 호출하지 않는다(UE4 시절의 드래그드롭 메뉴에서 제거됐다). 파이썬으로만 부를 수 있고 **3번째 인자(완료 델리게이트)가 필수**다:

```python
at = unreal.AssetToolsHelpers.get_asset_tools()
evt = unreal.AdvancedCopyCompletedEvent(); evt.bind_callable(fn)
at.begin_advanced_copy_packages([unreal.Name(p) for p in src], '/Game/SoldierLab/Ally', evt)
```

★ **`Animations` 와 `PoseSearch` 를 한 번의 호출로 넘겨야** PSD↔PSN↔클립 참조가 **사본끼리** remap 된다. 나눠 부르면 사본 PSD가 원본 클립을 가리킨다.

확인 대화상자에서 **공유 에셋은 자동으로 체크 해제**돼 나온다 — `SK_UEFN_Mannequin` · `PSS_*` 스키마 · `E_Gait` 등 열거형 · `AM_Copy_IKFootRoot` 모디파이어 · `SoldierCharacter_ABP`. 그대로 두면 사본이 원본을 공유한다(원하는 결과다).

★ **`CHT_Soldier_Databases` 는 체크된 채로 복사되는데 이게 이득이다** — Chooser가 두 벌이 되어 "열 추가 + 17행 복제(에디터 수작업)"가 통째로 불필요해졌다.

### 3.3 이름 변경 [A]

MCP `AssetTools.move` 를 에셋마다 호출해 **폴더 이동 + 접두사를 한 번에**(목적 경로를 최종형으로 지정). 아군 247 · 적군 247 **전부 실패 0건**.

- 아군 쪽은 참조자가 없어 리다이렉터가 안 남았다.
- 적군 쪽은 ABP/BP가 참조 중이었으나 이름 변경이 참조를 갱신해 **리다이렉터 잔존 0건**.

## 4. 결과 — 검증 [A]

### 4.1 참조 검증

디스크 바이트 스캔으로 사본 247개 중 **원본 폴더를 가리키는 참조 0건**. 개별 확인도 전부 통과:

| 확인 | 결과 |
|---|---|
| AO 블렌드스페이스 → 포즈 15장 | 사본 |
| AO 애디티브 기준(`ALLY_MM_Rifle_Idle_ADS`) | 사본 |
| 총내림 델타(`AO_CD` → 기준 `AO_CC`) | 사본 |
| HitReact/Reload 몽타주 → 클립 | 사본 |
| **PSD → 클립** | 사본 ★ 걱정했던 `TArray<FInstancedStruct>` remap 이 성공했다 |
| PSN ↔ PSD · Chooser → PSD | 사본 |

⚠ **`PreviewSkeletalMesh` 는 `duplicatetransient` 라 사본에 안 따라온다** — 다시 지정해야 한다(5절).

### 4.2 프리뷰 메시 — 에셋별 지정 [A]

`PreviewSkeletalMesh` 는 `EditAnywhere` 가 아니라(`AnimationAsset.h:1177`) **프로퍼티 매트릭스에 안 뜨고 MCP `get/set_properties` 로도 못 읽는다.** 그러나 **`SetPreviewSkeletalMesh` 가 BlueprintCallable 로 열려 있어**(`AnimationAsset.h:1100`) 파이썬으로 일괄 지정이 된다:

```python
a.set_preview_skeletal_mesh(mesh)
EAL.checkout_loaded_asset(a); EAL.save_loaded_asset(a, only_if_is_dirty=False)
```

적군 세트 **227개에 `new_enemy_T` 지정 완료**(`saved 227, skipped 0, failed 0`). 아군 세트용 같은 스크립트는 사용자가 실행 → **[C-129]**.

⚠ 저장이 **P4 체크아웃이 아니라 Uncontrolled Changelist 로 들어갔다** — 에디터 Changelists 에서 정식 체크아웃으로 올려야 submit 에 잡힌다 → **[W73]**.

### 4.3 ★ 프리뷰에 총이 안 보이던 문제 [A]

애님 에디터의 부착물은 `AddPreviewAttachedObjects()`(`AnimationEditorPreviewScene.cpp:504`)가 **① PersonaToolkit 의 Mesh ② 스켈레톤의 컨테이너** 두 곳에서만 읽는다. 그런데 ①은 **에디터를 열 때 애님 에셋 자체의 Preview Mesh 로 한 번 정해지고 그 뒤 안 바뀐다**(`PersonaToolkit.cpp:42`).

→ Preview Scene Settings 에서 **메시만** 바꾸면 그 메시의 **메시 소켓 부착물이 안 읽힌다.** 해결은 **에셋별 Preview Mesh 를 박아 두는 것**(4.2 스크립트). 부착물 자체는 메시 소켓 `weapon_r` 에 Add Preview Asset.

## 5. 런타임 분기 — 진영별 애님 세트 [A]

구동 변수는 **하나**다:

```
ABP 변수      UseAllyAnimSet (bool, 기본 false = 적군)
캐릭터 변수    UseAllyAnimSet (bool, Instance Editable)
              → BeginPlay 의 Cast(SoldierCharacter_ABP) 뒤에 Set 노드 삽입
BP_Soldier_Friendly = true  /  BP_SoldierCharacter · BP_Soldier_Hostile = false
```

분기 5축 (전부 컴파일·저장 완료):

| 축 | 방법 | 노드 |
|---|---|---|
| **로코모션 61클립** | `Update_MotionMatching` 에 Branch + **두 번째 EvaluateChooser** | `K2Node_IfThenElse_0` · `K2Node_EvaluateChooser2_0`(ALLY_CHT) · `K2Node_VariableSet_1`(SetValidDatabases) · `K2Node_VariableGet_1`(GetValidDatabases → `SetDatabasesToSearch.Databases`) |
| **조준 AO 2** | Select 3단 (스탠스 Select ×2 → 진영 Select) → `BlendSpacePlayer_1.BlendSpace` | `K2Node_Select_1`(ally) · `K2Node_Select_2`(faction) · `K2Node_VariableGet_15` |
| **총내림 델타 1** | 노드 복제 + `BlendPosesByBool`(blend time 0) | `AnimGraphNode_SequencePlayer_1` · `AnimGraphNode_BlendListByBool_1` |
| **블라인드파이어 3** | 동 | `SequenceEvaluator_3/4/5` · `BlendListByBool_2/3/4` |
| **사격/재장전 몽타주 2** | 캐릭터 변수 `FireMontage` · `ReloadMontage`(Instance Editable) → `PlayAnimMontage` 핀 | `K2Node_VariableGet_35/36` |

(같은 세션에서 **체력 컴포넌트의 몽타주 19개**도 같은 방식으로 데이터화됐다 — 그쪽은 `ai/2026-09-17_hit_death_three_causes.md` 2절.)

## 6. 판정

**성공.** 1절 기준 ①②③ 충족. 두 진영이 서로 다른 PSD/Chooser/AO/포즈/몽타주를 쓰고, ABP·PSD·Chooser 배선 작업은 Advanced Copy 가 Chooser 사본까지 만들어 준 덕에 두 배가 되지 않았다.

⚠ **런타임 육안 검증은 아직 부분적이다** — 분기 자체는 PIE 에서 "아군이 적군 PSD 를 쓴다"는 증상을 통해 **역으로** 확인됐고(7절 두 번째 함정), 값을 고친 뒤 정상 동작을 사용자가 확인했다. 두 세트가 **시각적으로** 어떻게 다른지는 아직 아무 의미가 없다(내용이 동일한 복제본이므로).

## 7. 교훈 / 함정 [A]

- **EvaluateChooser 는 Chooser 에셋이 노드 타입에 구워져 있다** — `Animation|EvaluateChooser:CHT_Soldier_Databases`. 새로 만들 땐 제네릭 `Animation|EvaluateChooser` 로 만든 뒤 `chooser` 프로퍼티를 set 한다. 그리고 **`mode` / `structOutputMode` 를 원본과 맞춰야 한다** — 기본값(`FirstResult` / `Default`)이면 Result 가 단일 객체라 **에디터가 MakeArray 노드를 자동 삽입**한다(원본은 `AllResults` / `SingleOutputStruct`).
- **변수 setter 노드의 `type_id` 는 변수 카테고리 경로를 탄다**(P60): `Variables|MotionMatching|SetValidDatabases` 이지 `Variables|Default|...` 가 아니다. 틀리면 `does not exist` 로 스크립트가 중단되고 **그 전까지 생성된 노드는 남는다**(P59).
- **BP 에 변수를 추가한 직후에는 CDO 에 그 프로퍼티가 없다** — `compile_blueprint` 를 한 번 부른 뒤에야 `set_properties` 로 기본값을 넣을 수 있다 → **P138**.
- **`BlendListByBool` 은 true 가 `BlendPose_0`** (P24 계열의 기존 기록과 일치).
- ⚠⚠ **자식 BP 의 CDO 는 부모 CDO 의 기본값을 안 물려받는다** — `BP_SoldierCharacter` 에 `FireMontage` 기본값을 넣어도 `BP_Soldier_Hostile` 은 `None` 으로 남았다. 자식마다 명시 지정이 필요하다 → **P137**(P127 과 같은 계열).
- ⚠⚠ **읽기 전용(P4 미체크아웃) 상태에서 `set_properties` 는 성공하지만 `save_assets` 가 false 를 반환하고, 그 값은 에디터 재시작 때 유실된다.** 실제로 `BP_Soldier_Friendly.UseAllyAnimSet = true` 가 이렇게 한 번 날아가 **"아군이 적군 PSD 를 쓴다"** 는 증상으로 나타났다. **set 후 반드시 `is_dirty` / `save_assets` 결과와 디스크를 확인할 것** → **P136**.

## 8. 이것이 바꾸는 것

| 문서 | 절 | 어떻게 |
|---|---|---|
| `IMPLEMENTED.md` | 2.1 · 2.2 · 2.3 · 2.4 · 2.5e · 3 | 클립/PSD/Chooser 경로 전면 교체, 분기 노드·변수 추가 |
| `assets/2026-09-14_designer_guide.html` | 0 · 2 · 4.1 · 5 · 부록 | 103개 경로 전부 `Animations_Enemy/Animations/` + `Enemy_` 로. 검수 기준 = 적군 세트 |
| `assets/2026-09-14_design_team_animation_handoff.md` | 2 · 7 | 같은 경로 + 세트 2벌 구조 |
| `CLAUDE.md` | 5 · 6.1 | P134~P140 · Advanced Copy 부재 · `SetPreviewSkeletalMesh` 우회 · EvaluateChooser mode |

- [x] 원 문서에 결과 반영
- [x] `OPEN_ITEMS.md` 등록 ([C-129] · [W73], [C-120] 갱신)
- [x] `CURRENT_STATE.md` 갱신

## 9. 막힌 것 / 다음에 확인할 것

- **[C-129]** 아군 세트 227개의 프리뷰 메시(`soldier_T`) 지정 — 사용자가 스크립트 실행 예정, 완료 확인 필요
- **[W73]** Uncontrolled Changelist 에 들어간 저장분을 정식 체크아웃으로 올리기
- **[C-120]** `CHT_Soldier_CharacterAnimations` 실사용 여부 — 이제 **`Enemy_CHT`/`ALLY_CHT` 와 별개**인 GASP 비무장 chooser 복제본이 `Animation/`(단수)에 그대로 남아 있다. 미사용이 확정되면 UEFN 클립 387개를 폐포에서 뺄 수 있다
- 세트가 **아직 내용이 같은 복제본**이라는 것 — 실제 분리의 이득은 디자인팀이 적군 세트를 고치기 시작해야 나온다
- **미저장/체크아웃 대기** 목록은 `CURRENT_STATE.md` 2026-09-17 저녁 블록
