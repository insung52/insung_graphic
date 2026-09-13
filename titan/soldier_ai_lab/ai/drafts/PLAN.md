# 개별 병사의 인지·위협평가·판단 — 구현 계획 초안

2026-09-09 / 초안(검토 대기) / 병사 **한 명**의 지각·위협 평가·개인 의사결정·StateTree 분해. 설계 문서 7·8절과 `ai/2026-09-02_upper_layer_plan.md` 4~5절을 근거로 하고, 문서가 정하지 않은 부분만 제안한다.

> **담당 범위 밖**: 분대 조율(L1) · 명령(L0) · 엄폐 슬롯 **생성**과 EQS 질의 · 무기/사격 구현 ·
> 애니메이션 층(L4). 이 문서는 그 경계를 **접합면(8절)** 으로만 다룬다.
>
> 신뢰도 표기는 `CLAUDE.md` 3.1절과 동일: **[A] 확정 / [B] 잠정 / [C] 미측정**

---

## 0. 한 줄 요약

**설계 7·8절은 이미 충분히 구체적이다. 이 초안이 실제로 더한 것은 세 가지뿐이다** —
① 위협을 "나에게 위험한가"와 "쏠 값어치가 있는가" **두 축으로 분리**한 것,
② 인지 후보 수집·틱 분산을 **월드 서브시스템**으로 묶어 45명 예산을 상한으로 만든 것,
③ 다른 담당자(분대·엄폐·무기)와의 경계를 **`SetScoringInput` 이름 하나**로 좁힌 것.

나머지는 설계 문서를 코드로 옮긴 것이다.

---

## 1. 근거 문서 대응표 [A]

| 이 초안의 항목 | 근거 |
|---|---|
| 인지 = 시간이 걸리는 누적 과정, 3구간 해석 | `design/2026-09-01_architecture.md` **7.1** |
| 시야 각도/거리/차폐/부분가시, 시선과 조준의 분리 | 같은 문서 **7.2** |
| 청각은 방향만 준다, 차폐로 유효거리 축소 | 같은 문서 **7.3** |
| `FThreatMemory` 필드 구성, Confidence 감쇠 → PeekCheck | 같은 문서 **7.4** |
| `Exposure` 를 인지 입력과 유틸리티 축이 **공유** | 같은 문서 **7.5** |
| 유틸리티(판단) + StateTree(실행) 2계층 | 같은 문서 **8.1~8.2** |
| Intent 목록과 축, 히스테리시스, 최소 지속시간, 인터럽트 | 같은 문서 **8.3** |
| `FConsideration` / `FIntentDefinition` / 곱셈 + make-up value | 같은 문서 **8.5** |
| 표적 전환 시 조준 수렴 타이머 리셋 | 같은 문서 **6.2** |
| 사망 정리 체크리스트 (표적 레지스트리·타 병사 기억 무효화) | 같은 문서 **6.5.3** |
| 서버 권위 게이트를 처음부터 넣는다 | 같은 문서 **4.3.2 / 4.3.6**, `CLAUDE.md` **P5** |
| 인지 0.7ms · 판단 0.3ms, 10Hz 라운드로빈, 3티어 LOD | 같은 문서 **12.2 / 12.3** |
| 아군/적군 코드 한 벌, 진영은 데이터 | 같은 문서 **3.7.1**, `CLAUDE.md` **P4** |
| 튜닝 대상은 전부 DataAsset | `upper_layer_plan` **9절**, `CLAUDE.md` **P6** |
| "왜 그랬는지"는 층의 필수 출력 | `upper_layer_plan` **2.3** |
| 자원 보유 상태를 **부모**로 두는 StateTree 작성 규칙 | `upper_layer_plan` **14.3** |
| L3 태스크 라이브러리 승계 목록 | `upper_layer_plan` **3.2 / 4.2** |
| `S_PlayerInputState` 5축 = AI→애니메이션 다리 | `ai/prototypes/2026-09-04_p0-1_ai_drives_mm.md` **4절** |

---

## 2. 재발명 금지 목록 — 이미 있는 것 [A]

**아래는 만들지 않는다.** 하나라도 새로 짜면 이 프로젝트가 반복해 배운 교훈("Epic이 이미 만든
것을 찾아 쓰는 것이 자작보다 낫다")을 어기는 것이다.

| 이미 있는 것 | 위치 | 이 초안에서의 취급 |
|---|---|---|
| `STT_SetSoldierInputState` (5축 전부 노출) | `/Game/SoldierLab/AI/` | **모든 Intent 서브트리의 첫 태스크로 그대로 사용** |
| `STT_FocusToPlayer` | 같은 곳 | **`STT_FocusToThreat`로 일반화**(표적을 인지 컴포넌트에서 받게) |
| `AIC_Soldier` | 같은 곳 | 브레인/인지 컴포넌트 부착 지점. 컨트롤러 로직은 계속 비워둔다 |
| `ST_Soldier_Patrol_Subtree` | 같은 곳 | `Intent.Idle/Overwatch` 서브트리의 출발점 |
| `STT_FindSmartObject` / `STT_ClaimSlot` / `STT_UseSmartObject` / `STT_ClearHandle` | GASP | **엄폐 담당자 영역.** 개인 판단은 Intent만 내고 손대지 않는다 |
| `StateTreeMoveToTask` / `StateTreeDelayTask` | 엔진 | 그대로 |
| `STC_CheckCooldown` / `STT_AddCooldown` | GASP | PeekCheck 재시도 제한에 사용 |
| `STE_GetAIData` | GASP | 우리 `STE_SoldierPerception` Evaluator의 원형 (**R4**) |
| `UAIPerceptionComponent` (Hearing/Damage) | 엔진 | **감지 이벤트 소스로만.** 시야는 우리가 누적 모델로 대체(설계 7.1) |
| `SignificanceManager` | 엔진(활성 확인됨) | LOD 티어 산출. 우리는 `LODTier` 정수만 받는다 |
| GASP Look-At POI 솔버 | GASP 5.8 | 시선 채널의 최종 구동원. **지금은 안 붙인다** → **Q19** |

---

## 3. 인지(Perception) — `USoldierPerceptionComponent`

### 3.1 왜 엔진 시야를 안 쓰는가 [A]

`UAIPerceptionComponent`의 Sight는 **표적당 불리언**을 준다. 설계 7.1이 없애려는 것이 정확히 그
불리언(현행 "스피어 오버랩 = 즉시 표적 확정")이고, 누적 모델은 그 불리언 **뒤에 있는 연속값**
(각도·거리·차폐 비율·대상 노출도)을 필요로 한다. 그래서 **시야만 자체 구현**하고 청각·피격은
엔진 이벤트를 받는다.

### 3.2 매 갱신마다 하는 일

```
1  자기 Exposure 갱신                        (설계 7.5)
2  후보 수집   서브시스템 → 진영 태그 + 거리 선필터
3  시야 평가   각도 → 거리 → 차폐 트레이스 순서 (트레이스가 마지막인 것이 요점)
4  Awareness 누적 / 미가시 감쇠              (설계 7.1)
5  Confidence·불확실 반경 감쇠, 수명 만료 정리 (설계 7.4)
6  구간(밴드) 판정 + 히스테리시스
7  위협 평가 → ThreatLevel / EngagementValue
8  주표적 선정 (전환 마진 적용)
```

**3의 순서가 예산의 전부다.** 각도·거리는 스칼라 연산이라 공짜고, 트레이스만 비싸다. 설계
12.2가 "후보 선필터로 트레이스 수 억제"라고 쓴 것을 코드 순서로 못박았다. 갱신당 트레이스는
`MaxSightTracesPerUpdate`로 상한이 걸린다 **[B]**.

### 3.3 설계 7절 대비 추가한 것 — 그리고 그 이유

| 추가 | 왜 | 신뢰도 |
|---|---|---|
| `AwarenessBandHysteresis` | 임계에 걸친 기억이 매 갱신 밴드를 오가며 분대 공유·Intent 변경을 도배한다. 설계 8.3이 Intent에 히스테리시스를 넣은 것과 같은 이유 | [B] |
| `LocationUncertaintyCm` | 설계 7.3이 "청각은 방향만"이라 했는데 위치를 점으로 기록하면 그 의도가 사라진다. 반경으로 기록해야 PeekCheck가 **구역을 수색**한다 | [B] |
| `VisibilityRatio`를 기억에 보존 | 설계 7.2의 "부분 가시 0~1"을 인지에만 쓰고 버리면 위협 평가가 "벽 뒤 적"을 구분할 수 없다 | [B] |
| `USoldierPerceptionSubsystem` | 45명이 각자 월드를 훑으면 예산이 안 나온다. 후보 등록 + 프레임 분산을 한 곳에 둔다 | [B] |
| `ReportDamageFrom` / `ReportNoise`를 **명시 호출**로 | 데미지 파이프라인이 아직 없다. `OnTakeAnyDamage`에 지금 묶으면 **조용히 틀린다** | [A] (판단 근거) |

### 3.4 후보 수집을 서브시스템으로 둔 이유 [B]

| 방식 | 문제 |
|---|---|
| `GetAllActorsOfClass` 매 갱신 | 45명 × 10Hz. 예산 밖 |
| 스피어 오버랩 | 현행 시스템이 하던 것. 물리 질의를 인지 후보에 쓰면 비용이 표적 수가 아니라 **월드 밀도**에 비례한다 |
| **등록 기반 레지스트리** | 병사만 후보다. 진영 태그 비교 + 거리 제곱 비교로 끝난다 |

현행 `titan_example`의 `UDetectableTargetSubsystem`과 같은 역할이고, 설계 6.5.3이 "표적
레지스트리 등록 해제"를 사망 체크리스트에 넣은 것도 이 물건을 전제한다. **이식할지 새로
만들지는 → Q15.**

### 3.5 미측정 항목

| ID(제안) | 항목 | 판정 기준 |
|---|---|---|
| **C-60** | `BaseAwarenessRatePerSec` / `AwarenessDecayPerSec` 초기값 | 정면·개활·정지 표적을 **거리별로** 확정하는 데 걸리는 시간이 사람 같은가. 10m 0.5초 / 50m 2~3초 / 100m 5초 이상이 출발 가설 [C] |
| **C-61** | 45명 × 10Hz에서 인지 0.7ms 충족 | `MaxUpdatesPerFrame` × `MaxSightTracesPerUpdate` 조합. 넘으면 트레이스 수부터 줄인다 |
| **C-62** | `VisibilitySampleBones` 이름이 `SK_UEFN_Mannequin`에 실제 존재하는가 | 하나도 안 맞으면 조용히 **바운드 중심 1점**으로 떨어져 부분 가시가 무의미해진다. 실측 필요 |
| **C-63** | `EyeBoneName = head` 위치가 실제 눈높이와 맞는가 | 어긋나면 벽 너머로 보거나 자기 어깨에 막힌다 |

---

## 4. 위협 평가 — `USoldierThreatAssessment`

### 4.1 ★ 핵심 결정 — 위협은 한 숫자가 아니다 [B]

설계 7.4의 `ThreatLevel`("나에게 얼마나 위험한가")만으로는 8.3의 `AimedFire` 축
(표적 신뢰도 × 표적 노출 × 탄약 × 사격선)을 만들 수 없다. **설계 문서 자체가 이미 두 종류의
숫자를 쓰고 있는데 이름이 하나뿐이다.** 그래서 분리한다:

| | `ThreatLevel` | `EngagementValue` |
|---|---|---|
| 뜻 | 저 적이 **나를** 얼마나 위험하게 하는가 | 저 적을 **쏘는 것이** 얼마나 값어치 있는가 |
| 축 | 근접도 · 신뢰도(하한 있음) · 무기 등급 · 나를 조준 중 · 최근 나를 맞힘 | 신뢰도(게이트) · 가시 비율 · 유효 사거리 · 내 준비상태(피제압·잔탄) |
| 소비하는 Intent | `TakeCover` · `Retreat` · `Reposition` | `AimedFire` · `SuppressiveFire` |
| 0이 되는 조건 | 사거리 밖 | 신뢰도/가시비율 게이트 미달 → **곱셈 스코어에서 완전 탈락** |

**분리를 정당화하는 사례**: 벽 뒤 기관총 사수는 `ThreatLevel` 높고 `EngagementValue` 낮다.
한 숫자로 합치면 "제일 위험하니 정조준한다"가 되어 벽을 쏜다. 나눠 두면 자연히
**엄폐 + 제압사격**이 이긴다.

### 4.2 신뢰도의 취급이 두 축에서 다르다 [B]

- `EngagementValue`: 신뢰도가 `MinConfidenceToEngage` 미만이면 **0**. 소문에 정조준하지 않는다.
- `ThreatLevel`: 신뢰도로 곱하되 **하한 `MinConfidenceThreatScale`** 을 둔다. 안 보이는 곳에서
  나를 쏜 적은 신뢰도가 낮아도 위험하다. 여기를 0으로 만들면 병사가 저격당하면서 태연히 서 있다.

### 4.3 주표적 전환 — 히스테리시스는 연출이 아니라 구조다 [A]

설계 6.2: **표적을 바꾸면 조준 수렴 타이머가 리셋된다.** 그러므로 매 갱신 표적을 재선정하는
병사는 **영원히 발사 조건을 못 채운다.** 이건 "우유부단해 보인다"가 아니라 **고장**이다.

```
전환 조건 = (현재 표적 보유 ≥ MinTargetHoldSec)  AND
            (도전자 점수 > 현재 점수 × (1 + TargetSwitchMargin))
현재 표적이 사라졌으면 마진 없이 즉시 재선정
```

`Unaware` 밴드의 기억은 주표적 후보에서 **제외**한다 — 인지하지 못한 적을 조준하는 것이
설계 7.1이 없애려는 에임봇 거동 그 자체다.

### 4.4 열린 것

- **Q12** — 주표적을 위협 기준으로 고를 것인가 교전가치 기준으로 고를 것인가.
  현재 `ThreatVsValueBias` 하나로 섞게 해뒀으나 **기본값을 정할 근거가 없다.**
- **Q13** — `bTargetAimingAtMe`를 무엇으로 판정하나. 지금 코드에는 채우는 곳이 없다.

---

## 5. 개인 의사결정 — `USoldierBrainComponent` (L2)

### 5.1 설계 8.5를 그대로 구현한다 [A]

```
Score = BaseWeight
for each Consideration:  Score *= Curve(정규화된 입력)
Modification = 1 - 1/축개수
Score = Score + (1 - Score) * Modification * Score      ← IAUS make-up value
```

추가한 것은 **`InputRange`(원시 입력의 0~1 정규화)** 와 **`bInvert`** 둘뿐이다. 거리(cm)·
시간(초)을 축마다 코드에서 정규화하면 P6("코드에 상수를 박지 않는다")을 어기게 된다 **[B]**.

### 5.2 Intent 목록 — 개인이 혼자 판단할 수 있는 것과 아닌 것 [A]

| Intent | 개인 판단만으로 가능? | 필요한 외부 입력 | 소유 |
|---|---|---|---|
| `Idle` / `Overwatch` | ✅ | — | 개인 |
| `AimedFire` | ✅ | `LineOfFireClear`(무기 층) | 개인 |
| `Reload` | ✅ | — | 개인 |
| `PeekCheck` | ✅ | — | 개인 |
| `TakeCover` | ⚠ 부분 | `CoverSlotAvailable` / `BestCoverSlotScore` (엄폐 담당) | 개인이 결정, 위치는 EQS |
| `Reposition` | ⚠ 부분 | 위와 동일 | 위와 동일 |
| `Retreat` | ⚠ 부분 | `SquadMorale` (분대 담당) | 개인 + 분대 변조 |
| `SuppressiveFire` | ❌ | `SquadHasSuppressionToken` | **분대 게이트** |
| `Advance` | ❌ | `SquadHasMovementToken` · `OrderAggression` | **분대 게이트** |
| `Regroup` | ❌ | `SquadDistanceCm` | **분대 게이트** |

**게이트가 안 채워지면 그 Intent 점수는 0이다.** 곱셈 스코어링의 성질을 그대로 쓴 것이고,
"분대가 없는 병사는 혼자 제압사격을 개시하지 않는다"가 **의도한 기본 거동**이다.
→ 이 설계 덕분에 **분대 담당자와 코드를 공유할 필요가 없다.** 접합면은 `SetScoringInput` 하나.

### 5.3 진동 방지 3종 [A]

| 장치 | 값 | 근거 |
|---|---|---|
| 현재 Intent 보너스 | +10~15% | 설계 8.3 |
| 최소 지속시간 | Intent별 0.5~2초 | 설계 8.3 |
| 생존 계열 인터럽트 | `Intent.Trait.Survival` 태그 보유 시 최소 지속시간 무시 | 설계 8.3 |

인터럽트를 **enum 하드코딩이 아니라 태그**로 둔 이유: 나중에 `Grenade`(회피)나 `Revive`가
인터럽트 자격을 얻을 때 코드를 안 고친다. `FIntentDefinition::Traits`에 태그만 추가한다 **[B]**.

### 5.4 "왜 이겼는가"는 부산물이 아니라 출력이다 [A]

`FIntentScoreBreakdown`이 **모든 후보의 축별 원시입력·커브출력**을 남긴다.
`upper_layer_plan` 2.3과 10절이 이것을 층의 필수 출력으로 못박았고, 11절이 **"A3의 글래스박스
UI가 선행되어야 한다"** 고 했다. 축이 0을 뱉어 탈락한 경우에도 **기록한 뒤에** 중단하도록 짜여
있다 — 아니면 "어느 축이 거부권을 썼는지"가 안 보인다.

> **D10(제안)**: 이 breakdown을 그리는 HUD 규격. 축을 늘리기 **전에** 만든다.

### 5.5 미측정

| ID(제안) | 항목 | 판정 기준 |
|---|---|---|
| **C-64** | 축 3~5개 곱셈의 실제 분별력 | 점수가 전부 0.01 근처로 뭉개져 `MinViableScore`만으로 갈리지 않는가. make-up value가 이걸 막으라고 있는 것이지만 실측 전이다 |
| **C-65** | `TargetSwitchMargin` × 조준 수렴의 합작 | 표적 둘이 비슷할 때 실제로 한쪽을 쏘고 끝내는가, 아니면 둘 다 못 쏘는가 |
| C-14 (기존) | 커브 초기값 | 글래스박스 UI 선행 |
| C-15 (기존) | 4~10Hz 체감 반응성 | 굼떠 보이지 않는가 |

---

## 6. L3 StateTree 구조

### 6.1 작성 규칙 — 어기면 사망 시 자원이 샌다 [A]

`upper_layer_plan` 14.3: **자원(슬롯 예약·토큰·포커스)은 그것을 보유하는 상태가 부모여야 한다.**
태스크를 한 상태에 나열하면 상태 이탈 시 정리가 안 된다. 설계 6.5.3이 "가장 위험"으로 표시한
SmartObject 예약 누수가 **이 구조를 지킬 때만** 자동 해결된다.

### 6.2 루트 트리 [B]

```
ST_Soldier_Root
 │  Evaluator: STE_SoldierPerception   (인지 결과 노출: 주표적, 밴드, 마지막 목격 위치)
 │  Evaluator: STE_SoldierIntent       (브레인의 CurrentIntent 태그 + 표적 노출)
 │
 ├ Incapacitated   [cond: HealthState ∈ {Downed, Dead}]        ← 최우선, 다른 모든 것을 차단
 │
 ├ Engage (Group)
 │   ├ TakeCover        [cond: Intent == Intent.TakeCover]      → LinkedAsset ST_Intent_TakeCover
 │   ├ AimedFire        [cond: Intent == Intent.AimedFire]      → LinkedAsset ST_Intent_AimedFire
 │   ├ SuppressiveFire  [cond: Intent == Intent.SuppressiveFire]→ (분대 담당자 영역)
 │   ├ Reposition       [cond: Intent == Intent.Reposition]     → (엄폐 담당자와 공동)
 │   ├ Reload           [cond: Intent == Intent.Reload]         → LinkedAsset ST_Intent_Reload
 │   ├ PeekCheck        [cond: Intent == Intent.PeekCheck]      → LinkedAsset ST_Intent_PeekCheck
 │   └ Retreat          [cond: Intent == Intent.Retreat]        → LinkedAsset ST_Intent_Retreat
 │
 └ Standby              [기본]                                   → ST_Soldier_Patrol_Subtree 확장
```

`LinkedAsset`으로 Intent당 에셋 하나를 다는 것은 **R1에서 이미 확인된 구성**이고, 서브트리의
`Tree Failed`가 **링크 지점까지만 전파**되는 성질이 "L3는 실패를 보고하고 L2가 다시 고른다"는
규칙(4.3절)을 엔진 기능으로 표현해 준다 **[A]**.

### 6.3 Intent 전환을 어떻게 트리거하나 — 두 안 [B]

| | A. 이벤트 | B. 매 틱 조건 재평가 |
|---|---|---|
| 방법 | 브레인의 `OnIntentChanged` → 컨트롤러가 StateTree 이벤트 발송 → 루트 전이 `Trigger = On Event` | 각 상태에 `Intent == X` 조건, 매 틱 상태 선택 재실행 |
| 장점 | 전환 시점이 명시적. 판단 주기(4~10Hz)와 실행 주기(30Hz)의 분리가 유지됨 | 배선이 단순 |
| 단점 | 이벤트 유실 시 상태가 굳는다 | 매 틱 전체 조건 평가. 45명 × 10 서브트리에서 **C-13** 그 자체 |

**권고: A** [B]. 단 **주기적 자기 교정**(예: 1초마다 태그 불일치 검사 후 강제 재선택)을 함께
둔다. → **Q18**

### 6.4 Intent 서브트리 예 — `ST_Intent_AimedFire` [B]

```
AimedFire  (부모 — 자원: 포커스, 견착 상태)
 │  tasks: STT_SetSoldierInputState(WantsToAim=true, WantsToStrafe=true)
 │         STT_FocusToThreat
 │
 ├ Settle      [cond: AimConvergence < 임계]
 │     tasks: STT_AimAtTarget      ← 수렴 대기. 완료 시 Succeeded
 │
 ├ Fire        [cond: AimConvergence ≥ 임계 AND STC_HasLineOfFire]
 │     tasks: STT_FireBurst(길이 = 상황)        (설계 6.4)
 │
 └ LostContact [cond: 주표적 TimeSinceLastSeen > 임계]
       → Tree Failed  → L2가 재선택 (PeekCheck가 이길 가능성이 높다)
```

**부모가 견착·포커스를 보유**하므로 어떤 이유로 상태를 벗어나도 조준이 풀린다. 6.1절 규칙의
적용례다.

### 6.5 `ST_Intent_PeekCheck` — 인지 설계가 행동으로 나오는 지점 [B]

설계 7.4가 **"Confidence 감쇠 → PeekCheck 점수 상승 → 확인하러 감. 이게 살아있는 행동의 핵심
부품"** 이라고 지목한 그 경로다.

```
PeekCheck (부모 — 자원: 쿨다운 핸들)
 │  tasks: STT_SetSoldierInputState(WantsToAim=true, WantsToWalk=true)
 │         STT_AddCooldown(같은 표적 재확인 억제)
 ├ MoveToVantage   tasks: StateTreeMoveToTask(마지막 목격 위치 근처, 불확실 반경 고려)
 ├ Scan            tasks: STT_ScanSector(불확실 반경을 부채꼴로) + StateTreeDelayTask
 └ Resolve         → 발견하면 Succeeded(인지가 Confirmed로 올라가 L2가 AimedFire로 넘어감)
                     못 찾으면 Failed
```

`STT_ScanSector`는 `USoldierPerceptionComponent::SetGazeOverride`를 구동한다.
**시선을 인지의 실제 입력으로 둔 것(설계 7.2)이 여기서 값을 한다** — 스캔이 연출이 아니라
실제로 발견 확률을 바꾼다.

### 6.6 신규 태스크·조건 — 최소 목록 [B]

| 에셋 | 종류 | 하는 일 | 대체 가능한 기존 것 |
|---|---|---|---|
| `STE_SoldierPerception` | Evaluator | 주표적/밴드/마지막 목격 위치/불확실 반경 노출 | `STE_GetAIData` 패턴 (R4) |
| `STE_SoldierIntent` | Evaluator | 브레인의 `CurrentIntent` 노출 | — |
| `STT_FocusToThreat` | Task | `STT_FocusToPlayer`의 일반화 | **기존 것 복제·수정** |
| `STT_AimAtTarget` | Task | 조준 수렴 대기 (설계 6.2) | 없음 |
| `STT_FireBurst` | Task | 버스트 길이를 상황이 정함 (설계 6.4) | 없음 (무기 담당 접합) |
| `STT_ScanSector` | Task | 시선 스캔 구동 | 없음 |
| `STC_HasLineOfFire` | Condition | 사격선에 아군 없음 (설계 9.5) | 없음 (무기 담당 접합) |
| `STC_IntentIs` | Condition | 태그 비교 | **엔진 기본 조건이 있는지 먼저 확인 → R6** |

> **⚠ MCP 주의**: StateTree 태스크를 `write_graph_dsl`로 만들 때 `CLAUDE.md` 6.1절의 함정
> 3종(읽기/쓰기 `type_id` 불일치 · 인터페이스 `(Message)` 버전 · `set_variable_instance_editable`)이
> 전부 적용된다. `STT_SetSoldierInputState` 작성 때 실제로 다 밟았다.

---

## 7. 클래스·파일 목록

`Source/SoldierLab/` 아래 배치. 설계 3.7절 모듈 레이아웃을 따른다.

| 파일 | 배치 | 내용 |
|---|---|---|
| `SoldierAITypes.h/.cpp` | `Soldier/` | Intent 네이티브 태그, `ESoldierAwareness`, `FThreatMemory`, `FSoldierSelfState`, `FConsideration`, `FIntentDefinition`, `FIntentSelection`, 스코어링 입력 이름 |
| `SoldierAIConfig.h` | `Soldier/` | `USoldierPerceptionConfig` · `USoldierThreatConfig` · `USoldierProfile` (전부 DataAsset) |
| `SoldierPerceptionComponent.h/.cpp` | `Perception/` | `USoldierPerceptionComponent` + `USoldierPerceptionSubsystem` |
| `SoldierThreatAssessment.h/.cpp` | `Perception/` | `USoldierThreatAssessment` |
| `SoldierBrainComponent.h/.cpp` | `Soldier/` | `USoldierBrainComponent` (L2 유틸리티) |

### 7.1 `SoldierLab.Build.cs` 에 추가해야 하는 것

```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "Core", "CoreUObject", "Engine", "InputCore",
    "GameplayTags",     // FGameplayTag, NativeGameplayTags.h
    "AIModule"          // UAIPerceptionComponent, UAISense_Hearing (청각/피격 이벤트 소스)
});
```

StateTree 태스크를 **C++로** 만들게 되면 `"StateTreeModule", "GameplayStateTree"` 가 추가된다.
**이 초안은 태스크를 BP로 두므로 아직 필요 없다** — 이유는 8.2절.

> ⚠ `CLAUDE.md` **P13**: 새 `UCLASS`는 Live Coding으로 안 들어간다. 에디터를 닫고 빌드해야 한다.
> 단 UHT는 먼저 도니 **에디터를 안 닫아도 리플렉션 문법 오류는 잡힌다.**

---

## 8. 접합면 — 다른 담당자와의 경계

### 8.1 개인 층이 **밖에서 받는** 것

| 입력 이름 | 채우는 쪽 | 안 채우면 |
|---|---|---|
| `BestCoverSlotScore` / `CoverSlotAvailable` | 엄폐/EQS 담당 | `TakeCover`·`Reposition` 점수 0 |
| `SquadHasSuppressionToken` / `SquadHasMovementToken` | 분대 담당 | `SuppressiveFire`·`Advance` 점수 0 |
| `SquadMorale` / `SquadDistanceCm` / `OrderAggression` / `AlliesManoeuvring` | 분대 담당 | `Retreat`·`Regroup`이 개인 축만으로 계산됨 |
| `LineOfFireClear` | 무기 담당 | **`AimedFire` 점수 0** ← 위험. Q20 |
| `FSoldierSelfState`의 탄약·체력·조준수렴 | 무기/체력 담당 | 기본값(만탄·건강·수렴 0)으로 동작 |

전부 `USoldierBrainComponent::SetScoringInput(FName, float)` 하나를 통과한다.
**헤더 의존이 없다** — 상대 모듈이 아직 없어도 이 코드는 빌드된다.

### 8.2 개인 층이 **밖으로 내보내는** 것

| 출력 | 받는 쪽 |
|---|---|
| `USoldierPerceptionComponent::OnAwarenessChanged` (Confirmed 도달) | 분대 blackboard 공유 (설계 9.3) |
| `ReceiveSharedThreat(...)` | 분대가 **호출하는** 수신구 |
| `USoldierBrainComponent::OnIntentChanged` + `GetCurrentIntent()` | L3 StateTree, 그리고 설계 4.3.3의 복제 대상 |
| `GetExposure()` | 다른 병사의 인지율 계산(설계 7.5) — 이미 컴포넌트끼리 직접 읽는다 |
| `ForgetTarget(...)` | 사망 정리(설계 6.5.3)에서 **모든** 병사에게 호출해야 함 |

### 8.3 StateTree 태스크를 C++가 아니라 BP로 두는 이유 [B]

1. 이미 있는 자산(`STT_SetSoldierInputState`)이 BP이고, 인터페이스 디스패치 방식이 검증돼 있다.
2. C++ StateTree 태스크(`FStateTreeTaskCommonBase` 계열)는 이 프로젝트에서 **한 번도 컴파일해
   본 적이 없다.** 컴파일 검증이 불가능한 초안 단계에서 새 리플렉션 문법을 도입하면 실패가
   조용히 쌓인다.
3. 비용이 문제가 되면(C-13) 그때 옮긴다. **옮기는 비용은 국소적이다** — 태스크는 얇다.

### 8.4 ★ 병행 초안이 요구한 인터페이스 — 수용 상태 [A]

같은 날 작성된 `cover/drafts/` · `squad/drafts/` 초안이 **인지 담당에게 두 개의 인터페이스를
요구**하고 있다. 둘 다 수용했고, 초안 코드가 그 계약을 지키도록 고쳤다.

| 요구자 | 인터페이스 | 우리 쪽 구현 | 상태 |
|---|---|---|---|
| `cover/drafts/.../EnvQueryContext_CoverThreats.h` | `ISoldierThreatProvider` — `GetPrimaryThreat` / `GetKnownThreats` (`FCoverThreatInfo{Location, Weight}`) | `GetPrimaryThreatWeighted` / `GetWeightedThreats`. **Weight = `Confidence × ThreatLevel`** (`FThreatMemory::GetThreatWeight`) | ✅ 값 계산까지 구현. 인터페이스 상속은 **상대 헤더가 프로젝트에 들어온 뒤** |
| `squad/drafts/.../SquadTypes.h` | `ISoldierThreatMemorySink::ReceiveSharedThreat(const FSharedThreat&)` + **병합 규칙 R1/R2/R3** | `ReceiveSharedThreat(Target, Loc, Vel, Sigma, ObservedAt, Confidence)` | ✅ **R1·R2·R3 전부 구현**. 어댑터 한 줄만 남음 |

**세 규칙을 코드가 어떻게 지키는가** (분대 초안의 [Q23] 합의 요청에 대한 답):

| 규칙 | 구현 |
|---|---|
| R1 직접 목격이 더 최신이면 보고를 버린다 | `bCurrentlyVisible \|\| LastSeenWorldTime >= ObservedAtSeconds` 이면 즉시 반환 |
| R2 이미 병합한 것보다 오래된 보고는 버린다 | `FThreatMemory::LastSharedReportObservedTime` **신설** — 이 필드가 없으면 R2를 지킬 수 없다 |
| R3 병합 결과 Confidence ≤ 0.6 | `FMath::Min(보고값, Config->SquadSharedConfidence)`. 상한이 **DataAsset**에 있으므로 0.6은 코드에 안 박힌다 |

추가로 **`LocationSigmaCm`을 200cm 하한으로 강제**한다. 분대 초안이 "0이면 직접 목격이라는
뜻이므로 공유본에 넣지 말 것"이라고 적었는데, 발신 측 실수로 0이 오면 **PeekCheck가 구역
수색이 아니라 한 점으로 걸어간다** — 조용히 틀리는 종류라 수신 측에서도 막는다.

#### 한 가지 이견 — "위협 목록을 분대 단위로 한 벌만" [B]

엄폐 초안이 `ISoldierThreatProvider`를 **분대당 하나로 공유**할 것을 권했다(5명이 각자 목록을
만들면 앞단이 5배). **동의하지 않는다**, 다만 조건부다:

- 설계 7.4·9.3의 핵심은 **"내가 직접 본 것"과 "남이 알려준 것"이 다르다**는 것이다. 목록을
  분대가 한 벌만 가지면 그 구분이 사라지고, `bSharedBySquad` · `Confidence 0.6` · PeekCheck의
  근거가 통째로 무너진다.
- 비용 주장은 **인지 계산**에는 맞지만 **EQS 컨텍스트**에는 해당하지 않는다. `GetWeightedThreats`는
  이미 만들어둔 배열을 복사할 뿐 트레이스를 돌지 않는다.
- **절충안**: EQS 질의가 "분대가 아는 전부"를 원하면 분대 blackboard의 `KnownThreats`를 쓰는
  별도 컨텍스트를 두고, 개인 인지 기반 컨텍스트와 **둘 다** 제공한다. 어느 쪽을 쓸지는 질의가
  정한다.

→ 엄폐·분대 담당자와 합의 필요. **Q22**로 등록 제안.

---

## 9. 구현 순서와 검수 기준 [B]

`upper_layer_plan` 11절의 A1·A3을 개인 범위로 쪼갠 것이다.

| # | 내용 | 검수 기준 | 선행 |
|---|---|---|---|
| **A1a** | 타입 + Config DataAsset + 인지 컴포넌트/서브시스템 | 컴파일. 병사 2기가 서로를 후보로 잡는다 | 없음 |
| **A1b** | **인지 디버그 표시** (밴드·Awareness 막대·마지막 목격 위치·불확실 반경·Exposure) | 값이 **보인다**. `CLAUDE.md` P7 | A1a |
| **A1c** | 누적/감쇠 튜닝 | 적을 **점진적으로** 인지하고, 시야를 벗어나면 서서히 잃는다 (**C-60**) | A1b |
| **A2a** | 위협 평가 + 주표적 선정 | 두 적 사이에서 표적이 **덜컹거리지 않는다** (**C-65**) | A1c |
| **A3a** | 브레인 + Intent 4종(`Idle`/`AimedFire`/`Reload`/`TakeCover`) | 점수 막대가 보이고, 축 하나를 만지면 행동이 바뀐다 | A2a |
| **A3b** | 루트 StateTree + Intent 서브트리 4종 | Intent가 바뀌면 서브트리가 바뀐다. 실패가 L2로 올라온다 | A3a |
| **A3c** | `PeekCheck` 추가 | **놓친 적을 확인하러 간다** — 설계 7.4가 지목한 "살아있는 행동" | A3b |
| **A3d** | 1 vs 1 교전 | 마커 없는 임의 레벨에서 자연스럽게 교전한다 (설계 P1 검수 기준) | A3c |

**A1b가 A1c보다 먼저인 것이 요점이다.** `CLAUDE.md` P7·P10 — 임계값을 다루기 전에 계측을
넣는다. 접지 임계를 추측으로 세 번 고치다 로그를 찍자 원인이 즉시 드러난 사례가 이 프로젝트에
이미 있다.

### 9.1 P0 진행 상황과의 관계

현재 `CURRENT_STATE.md` 4절의 다음 작업은 **P0-2(견착 이동 품질)** 이고 이 계획은 그와 **직교**
한다. A1a~A1c는 애니메이션 자산과 무관하게 진행할 수 있다 —
설계 문서도 "**7~12절(AI 시스템)은 스켈레톤과 무관**"이라고 리스크 표에 적어 두었다.

---

## 10. 성능 예산 대비 [B]

| 항목 | 설계 12.2 예산 | 이 초안의 대응 |
|---|---|---|
| 인지 0.7 ms | 10Hz 라운드로빈, 후보 선필터 | 서브시스템이 `MaxUpdatesPerFrame`으로 **프레임당 상한**을 건다. 평균이 아니라 상한인 것이 요점 |
| | | 트레이스는 각도·거리 통과 후에만. `MaxSightTracesPerUpdate`로 갱신당 상한 |
| 판단 0.3 ms | 4~10Hz | 브레인 틱은 카운트다운만. 스코어링은 주기당 1회 |
| LOD 3티어 | T0 10Hz / T1 5Hz / T2 2Hz | `LODTier` 정수 하나. `SignificanceManager`가 채운다(**아직 배선 없음**) |

**측정 전이다.** 위 숫자는 전부 [C]이고 **C-61**로 등록을 제안한다.

> ⚠ 프레임당 상한이 물리면 갱신 간격이 벌어진다. 그래서 `UpdatePerception`에 **실제 경과
> 시간**을 넘긴다 — 명목 간격을 넘기면 붐빌수록 인지가 느려져 **전투가 커질수록 AI가
> 멍청해진다.** 코드에 주석으로 남겨 두었다.

---

## 11. 신규 미해결 항목 — `OPEN_ITEMS.md`에 등록 필요

**이 초안은 `OPEN_ITEMS.md`를 수정하지 않았다.** 아래 ID는 제안이며, 채택 시 등록해야 한다.
기존 최대 번호는 C-59 / D9 / Q11 / R5 기준이다.

| ID | 유형 | 항목 |
|---|---|---|
| C-60 | 측정 | 인지 누적/감쇠 초기값 — 거리별 확정 시간 |
| C-61 | 측정 | 45명 인지 0.7ms 충족 (`MaxUpdatesPerFrame` × `MaxSightTracesPerUpdate`) |
| C-62 | 측정 | `VisibilitySampleBones` 본 이름이 `SK_UEFN_Mannequin`에 실재하는가 |
| C-63 | 측정 | `EyeBoneName` 위치가 실제 눈높이와 맞는가 |
| C-64 | 측정 | 축 3~5개 곱셈 스코어의 분별력 (make-up value가 실제로 상쇄하는가) |
| C-65 | 측정 | `TargetSwitchMargin` × 조준 수렴 합작 — 표적 둘일 때 실제로 쏘는가 |
| D10 | 설계 | 개인 판단 글래스박스 HUD 표시 규격 (A3 선행) |
| Q12~Q22 | 결정 | **`OPEN_QUESTIONS.md` 참조** |
| R6 | 조사 | StateTree에 GameplayTag 비교 조건이 기본 제공되는가 |
| R7 | 조사 | StateTree 이벤트로 상위 상태를 재선택시키는 정석 패턴 |

---

## 12. 이 초안이 **하지 않은** 것 — 정직하게

| 항목 | 왜 |
|---|---|
| 컴파일 검증 | 프로젝트 빌드가 금지된 작업이다. **문법 오류가 남아 있다고 가정할 것** |
| `Downed`/`Dead` 상태의 위협 0 처리 | 체력 컴포넌트가 없다. 인터페이스를 추측해 넣으면 **가장 안 보이는 곳에 틀린 가정**이 남는다 → 코드에 `TODO`로 명시 |
| `bTargetAimingAtMe` 채우기 | 데이터 원천 미정 → **Q13** |
| 사망 정리 진입점 `HandleDeath()` | 설계 6.5.3의 체크리스트 절반이 분대·명령 층 소유다. **개인 층 몫(`ForgetTarget`)만** 구현 |
| `Grenade` / `Revive` Intent | 설계가 "(2차)"로 표기 |
| Control Rig · 조준 IK · 사격 판정 | 무기/애니메이션 담당 |
| 엄폐 슬롯 생성·EQS 질의 | 담당 밖 |
| 유틸리티 커브 **초기값** | 글래스박스 UI 없이 정하는 것은 무의미하다 (설계 8.3, C-14) |
