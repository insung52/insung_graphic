# 엄폐 시스템 — 사용자 결정이 필요한 항목

2026-09-09 / 결정 대기 / `PLAN.md`를 쓰면서 **내가 결정할 수 없는 것**만 모았다. 기본안과 그렇게 판단한 근거를 붙였다.

> ID는 `OPEN_ITEMS.md`의 `Q` 접두 현재 최대(Q11) 다음부터 할당했다.
> **측정하면 알 수 있는 것은 여기 없다** — 그건 `PLAN.md` 11절의 `C-60`~`C-69`다.

---

## 먼저 봐야 할 것 — 세 건은 **P0-3 착수 전에** 답이 필요하다

| # | 질문 | 왜 급한가 |
|---|---|---|
| **Q12** | GameplayInteractions 플러그인을 쓸 것인가 | 설계 13절이 "안 쓴다"고 했는데 현재 방침(GASP 승계)은 사실상 쓰는 쪽이다. **P0-3 코드의 절반이 이 답에 달려 있다** |
| **Q13** | `Exposure`를 곱셈 인자로 나눌 것인가 | **실측 결과 인지 쪽 `Exposure`에 엄폐 항이 없다.** 합의 없이 각자 채우면 **엄폐가 두 번 곱해져 조용히 틀린다** |
| **Q14** | Prone을 범위에 넣을 것인가 | **인지 쪽은 이미 `ExposureProne`을 넣었고 엄폐 쪽은 뺐다.** 두 문서가 다른 전제 위에 있다 |

---

## Q12 — GameplayInteractions 플러그인을 쓸 것인가 ★

**충돌하는 두 문서:**

| 출처 | 내용 |
|---|---|
| 설계 13절 (채택하지 않은 것) | *"**GameplayInteractions 플러그인** — Experimental. Smart Object 자체는 정식이므로 **서브시스템 API를 직접 쓴다**"* |
| 설계 10.3절 | *"StateTree와 네이티브 통합된다(GameplayInteractions는 Experimental이므로, 우리는 SmartObject 서브시스템 API를 직접 쓰고 **StateTree 태스크는 자체 작성**한다)"* |
| `CURRENT_STATE.md` 4절 ⑤ / `upper_layer_plan.md` 13.2절 | *"GASP의 `FindSmartObject → ClaimSlot → UseSmartObject` 흐름을 **그대로 쓰고 탐색만 EQS로 교체**"* |
| `.uproject` 플러그인 감사 (2026-09-03) | `GameplayInteractions` **활성화돼 있다.** `NavCorridor`·`ContextualAnimation`을 끌어온 것도 이 플러그인이다 |

**내가 확인한 사실 [A]**:
`Content/Blueprints/AI/StateTree/TasksAndConditions/`에 있는 BP 태스크는 **6개뿐**이다 —
`STT_SetCharacterInputState` / `STT_FocusToTarget` / `STT_ClearFocus` / `STT_FindRandomLocation` /
`STT_FindSlotTransforms` / `STT_CharacterIgnoreCollisionsWithOtherActor`.

**`STT_FindSmartObject` · `STT_ClaimSlot` · `STT_UseSmartObject` · `STT_PlayAnimFromBestCost`는
이 폴더에 없다.** GASP 콘텐츠가 아니라 **엔진 제공 태스크**일 가능성이 매우 높다(→ R6).
엔진의 `GameplayInteractions` 플러그인은 `StateTreeTask_FindSlotEntranceLocation`,
`GameplayInteractionFindSlotTask`, `PlayMontageStateTreeTask` 등을 실제로 제공한다 [A, 엔진 소스].

**즉, "GASP 흐름을 그대로 쓴다"는 현재 방침은 사실상 "GameplayInteractions를 쓴다"와 같은 말일 수
있다.** 설계 13절과 정면으로 어긋난다.

| 선택지 | 결과 |
|---|---|
| **A. 쓴다 (권고)** | P0-3에서 만들 것이 `STT_QueryCoverSlot` 하나로 줄어든다. 진입점 계산(`FindSlotEntranceLocation`)·슬롯 엔트런스 어노테이션을 공짜로 얻는다. **대가**: Experimental API 변경 위험, 5.9 업그레이드 시 재작업 가능 |
| B. 안 쓴다 | 예약·사용·진입점 태스크를 전부 자체 작성. 설계 13절과 일치. **대가**: `upper_layer_plan.md` 14.3절의 "중첩 상태로 예약 누수가 구조적으로 해결된다"는 관찰이 **우리가 그 구조를 직접 재현했을 때만** 성립하게 된다 |
| C. 절충 | 엔진 태스크를 쓰되 **얇은 래퍼**로 감싸 교체 지점을 한 곳에 모은다 | 

**기본안: A(쓴다). 단 R6을 먼저 확인할 것** — "쓴다/안 쓴다"를 정하기 전에 **그 태스크들이
실제로 어느 모듈 것인지**부터 봐야 한다. 이건 에디터에서 StateTree를 열어 태스크 하나를 클릭하면
5분에 끝난다.

> ⚠ **이미 쓰고 있을 가능성이 있다.** `ST_Soldier_SmartObject`(우리가 만든 것)가 GASP StateTree를
> 복제한 것이라면, 그 안의 태스크가 엔진 것일 수 있다. **설계 13절이 이미 사실과 다를 수 있다.**

---

## Q13 — `Exposure`를 **곱셈 인자로 합의**할 것인가 ★

> **2026-09-09 갱신**: 같은 날 작성된 `drafts/soldier_ai/`를 실제로 읽고 대조했다.
> 이 질문은 "누가 소유하는가"가 아니라 **"엄폐를 두 번 곱하지 않으려면 어떻게 나눌 것인가"** 였다.

**실측한 사실 [A]** — `drafts/soldier_ai/Source/SoldierAIConfig.h:189-212`:

```
인지의 Exposure ← ExposureStanding/Crouched/Prone
                × ExposureStationary/Walking/Running
                × ExposureFiring(사격 후 1.5초)
```

**엄폐 항이 하나도 없다.** 그러니까 두 숫자는 애초에 같은 것이 아니었다:

| | 인지의 `Exposure` | `PLAN.md` 5.3절의 `Exposure` |
|---|---|---|
| 뜻 | **내 자세·이동·사격 때문에** 얼마나 눈에 띄는가 | **이 지점의 기하가** 얼마나 안 가려주는가 |
| 입력 | 자기 상태 | 슬롯 facing + 위협 배치 |
| 슬롯 없이도 계산되나 | 된다 | 안 된다 |

> ⚠ `upper_layer_plan.md` 5.4절의 *"같은 숫자를 두 시스템이 공유한다 — 이 설계의 경제성"* 은
> **정확히는 틀렸다.** 공유되는 것은 숫자가 아니라 **곱셈 인자 하나**다.
> 이 구분이 없으면 두 팀이 각자 "엄폐를 반영한 Exposure"를 만들어 **엄폐가 두 번 곱해진다.**

**제안하는 계약 (한 줄)**:
```
PerceivedExposure = 인지의 Exposure × (1 - CurrentCoverQuality)
```

- `CurrentCoverQuality`는 **내가 채운다.** `SoldierScoringInputs::CurrentCoverQuality`
  (`SoldierAITypes.h:460`)라는 자리가 이미 있다.
- 공식은 `USoldierCoverSubsystem::GetCurrentCoverQuality()` 한 곳에만 둔다.
- **인지 쪽은 곱하기 한 번만 추가하면 된다.**

| 선택지 | 결과 |
|---|---|
| **A. 위 계약을 채택 (권고)** | 각 층이 자기가 아는 것만 계산한다. 중복 없음 |
| B. 인지가 엄폐까지 계산 | 인지가 슬롯 기하를 알아야 한다 — 층 경계가 무너진다 |
| C. 그대로 둔다 | **엄폐가 두 번 곱해져 조용히 틀린다.** 가장 나쁜 결과 |

**기본안: A.** 결정 사항은 *"인지 담당자와 언제 이 한 줄을 합의할 것인가"* 다.
(참고: 인지 쪽에는 `GetPrimaryThreatWeighted(FVector&, float&)`가 이미 있어
`PLAN.md` 6.2절의 위협 컨텍스트는 **그대로 붙는다.** 이쪽 접점은 이미 맞다.)

---

## Q14 — Prone(엎드림)을 범위에 넣을 것인가

> ⚠ **2026-09-09: 두 문서가 이미 어긋나 있다.**
> `drafts/soldier_ai/Source/SoldierAIConfig.h:196`에 **`ExposureProne = 0.35`가 있다** — 인지 쪽은
> Prone이 있다는 전제로 썼다. 반면 `PLAN.md` 8.2절 A7은 **애니메이션이 0개**라는 이유로 범위 밖을
> 권고했다. **한 번에 결정해야 하는 항목이다.**

설계 10.2절은 차폐 판정 높이를 **40cm(엎드림) / 100cm(앉기) / 180cm(서기)** 세 가지로 적어 두었다.
`PLAN.md`는 **40cm를 뺐다.** 근거:

| 항목 | 현황 |
|---|---|
| 애니메이션 | Prone 클립이 **한 개도 없다**(에셋 실측). 필요량 ~15클립 — 8.2절 A7 |
| 조준 한계각 | 설계 5.5.6절이 *"엎드린 자세(Prone)는 상체 비틀기 여유가 거의 없어 `AimTwistMax`가 훨씬 작고, 재정렬 비용이 크다"* 고 이미 지적 |
| 캡슐 | CMC 캡슐 높이를 40cm까지 줄이면 이동/충돌 로직 전반을 다시 봐야 한다 |
| 슬롯 수 | 높이 3종이면 베이크 후보가 1.5배로 는다 |

| 선택지 | 결과 |
|---|---|
| **A. 뺀다 (권고)** | 클립 ~15개와 캡슐 작업이 사라진다. 낮은 엄폐물은 Crouch로 처리 |
| B. 넣되 "정지 사격 전용"으로 | 이동 없는 저격 자세만. 클립 3~4개 |
| C. 전면 지원 | 조달·이동·캡슐 전부 |

**기본안: A.** 데이터 구조(`ESoldierCoverHeight`)에 나중에 `Prone`을 **추가할 자리는 남겨 뒀다.**

---

## Q15 — 엄폐 lean을 **애디티브**로 갈 것인가

`PLAN.md` 8.3절의 판단이다. **이 프로젝트가 "상하체 분리"를 실패로 못박았기 때문에**(설계 5.5.1절)
사용자 확인이 필요하다.

**내 논지**: 5.5.1이 실패로 판정한 것은 **이동 중 상체를 다른 클립으로 교체**하는 것이고,
엄폐 lean은 **정지 중 베이스 포즈에 작은 편차를 얹는 것**이라 설계 5.5.3절의 **[3]층(Aim Offset)과
같은 계열**이다. 규칙으로 "lean은 정지 상태에서만 켠다"를 걸면 5.5.1의 실패 조건에 들어가지 않는다.

| 선택지 | 클립 요구 |
|---|---|
| **A. 애디티브 (권고)** | **2개** (`Cover_Lean_L/R`) |
| B. 풀바디 lean 포즈 | 자세(2) × 좌우(2) = **4개** + 전환 |
| C. 풀바디 + 전환 클립 | **8~12개** |

**기본안: A. 단 [C-68]로 실측 판정한다** — lean 25°/45°에서 총구가 몸을 뚫는지, 견착 포즈가
무너지는지. `CLAUDE.md` 3.1절의 규칙(확인하지 않은 것을 세밀하게 쓰지 않는다)에 따라
**A를 "권고"로만 두고 확정하지 않았다.**

---

## Q16 — 설계 문서 정정 3건을 반영할 것인가

`PLAN.md`를 쓰면서 **설계 문서가 사실과 다르거나 오독을 부르는 곳**을 셋 찾았다.
`CLAUDE.md` 3.3절 규칙(본문을 고치지 말고 문서 끝에 정정 절을 추가)에 따라 처리하려면 승인이 필요하다.

| # | 위치 | 문제 | 제안 |
|---|---|---|---|
| **(가)** | 설계 **10.4절** | *"Generator: 커스텀 Generator — Smart Object 서브시스템 조회"* → **불필요하다.** 엔진에 `UEnvQueryGenerator_SmartObjects`가 있고 `bOnlyClaimable=true`가 예약된 슬롯을 애초에 뺀다 [A, 엔진 소스] | 정정 절 추가. `upper_layer_plan.md` 13.2절도 함께 |
| **(나)** | 설계 **3.6절 vs 12.2절** | EQS가 3.6절엔 **3ms**, 12.2절엔 **0.7ms**로 적혀 있다. 둘은 "엔진 상한"과 "설계 목표"로 역할이 다른데 그 구분이 문서에 없다 → **3ms를 예산으로 착각하면 애님 2.5ms가 밀린다** | 12.2절 표에 각주 한 줄 |
| **(다)** | 설계 **13절** | GameplayInteractions "채택 안 함"이 현재 방침과 어긋날 수 있다 | **Q12 결정 후** 정정 |

---

## Q17 — P0-3의 범위를 어디까지로 볼 것인가

`CURRENT_STATE.md`는 P0-3을 **1~2일**로 잡았다. `PLAN.md` 10절의 단계 중 어디까지가 그 1~2일인가.

| 범위 | 산출물 | 예상 |
|---|---|---|
| **A. 최소 (권고)** | 수동 슬롯 8개 + `USoldierCoverSubsystem` + `CoverQuality` 테스트 + `ST_Intent_TakeCover`(S1~S4) + 사망 시 해제 확인 = **P0-3a~d** | 1.5~2일 |
| B. + 사격 | A + `CoverFiringPosition` 테스트 + S5b PeekFire | +1.5일 |
| C. + 베이크 툴 | B + NavMesh 경계 베이크 | +3일 이상 |

**기본안: A.** 근거는 설계 14절 P0-3의 판정 기준이
*"병사 3명이 서로 다른 슬롯을 예약해서 들어가는가(중복 없음)"* 하나뿐이라는 점이다.
**슬롯이 어떻게 생성됐는지도, 거기서 쏠 수 있는지도 이 판정과 무관하다.**
베이크 툴을 먼저 만들면 "슬롯 생성이 틀린 건지 예약 루프가 틀린 건지"를 구분할 수 없게 된다 —
`CLAUDE.md` P16(대조군 없이 측정을 신뢰하지 말 것)과 같은 함정이다.

> ⚠ **다만 A로 가면 P0-3 완료 시점에도 "임의 레벨에서 엄폐한다"는 못 한다.**
> 그건 P1 검수 기준(설계 P1 "마커 하나 없는 임의의 레벨")이며, P0-3의 기준이 아니다.
> 이 기대치 차이를 미리 합의해 두는 것이 이 질문의 목적이다.

---

---

## Q18 — 분대 soft-claim의 슬롯 식별자 타입

분대 초안이 **나에게 명시적으로 물어본 것**이다 —
`drafts/squad/Source/Squad/SquadTypes.h:288`:
> `/** SmartObject 슬롯 식별자. 실제 타입은 엄폐 담당자의 스키마에 맞춘다 [Q-12 인접] */`
> `FGuid SlotId;`

**답: `FSmartObjectSlotHandle`로 통일한다** (`PLAN.md` 12.3절에 근거 표).

핵심 근거 하나: **soft-claim은 `ExpiresAtSeconds`를 가진 몇 초짜리 휘발성 데이터라 영속될 필요가
없다.** 세션을 넘어 살아남지 않으므로 런타임 핸들이 옳고, GUID↔핸들 변환표를 유지할 이유가 없다.
변환표를 두면 **실제 SmartObject 예약과 어긋날 여지**가 생기는데, 그건 이 시스템에서 가장 피하고
싶은 종류의 버그다(중복 점유).

**결정 사항**: 분대 담당과 이 타입을 확정할 것. 내 쪽 코드는 이미 `FSmartObjectSlotHandle`이다.

---

## 결정하면 할 일

| Q | 결정되면 |
|---|---|
| Q12 | `PLAN.md` 6.3절 신규 클래스 목록이 확정된다. 설계 13절 정정 여부가 정해진다 |
| Q13 | `PerceivedExposure = Exposure × (1 - CurrentCoverQuality)` 한 줄을 인지 쪽에 반영 |
| Q14 | `ESoldierCoverHeight`에 `Prone` 추가 여부, 베이크 `ProbeHeightsCm` 기본값 |
| Q15 | 애니메이션 조달 요청서에 A1의 형식(애디티브/풀바디)을 적을 수 있다 |
| Q16 | 설계 문서에 정정 절 3개 추가 + `OPEN_ITEMS.md` 갱신 |
| Q17 | P0-3 작업 목록 확정 |
| Q18 | 분대 `FSlotSoftClaim::SlotId` 타입 교체 (1줄) |
