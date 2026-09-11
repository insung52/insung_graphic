# soldier_ai_lab — 세션 공통 규칙

**이 폴더에서 작업하는 모든 세션은 이 문서를 먼저 읽는다.** 상위 `../CLAUDE.md`의 규칙을
따르되, 이 프로젝트 고유의 규칙을 아래에 추가한다(2026-09-03 도입).

---

## 0. 이 폴더가 무엇인가

`titan_example`과 **별개의 언리얼 프로젝트**(`C:\working\works\kadex\anim_test\SoldierLab`,
UE5.8, GASP 기반)에서 진행하는 고사실감 병사 AI/애니메이션 R&D의 문서 저장소.

코드는 다른 프로젝트에 있지만 문서는 titan 문서 체계 안에 둔다(`genesis/`와 동일 패턴).

**최종 목표**: 병사 개개인이 환경·적·아군을 실시간으로 판단해 엄폐/사격/기동하고 분대로
협동하는 시스템. 상위 시나리오는 고수준 "명령"만 내린다.

---

## 1. 시작 지점 — 무엇부터 읽을 것인가

| 상황 | 읽을 것 |
|---|---|
| **처음 오는 세션** | `README.md` → `CURRENT_STATE.md` → 작업할 영역의 폴더 문서 |
| 지금 뭘 해야 하나 | `CURRENT_STATE.md` |
| 미해결 항목이 뭐가 있나 | `OPEN_ITEMS.md` |
| 전체 설계가 궁금 | `design/2026-09-01_architecture.md` |
| 애니메이션 작업 | `animation/` 두 문서 **둘 다** |
| AI/분대 작업 | `ai/2026-09-02_upper_layer_plan.md` |
| 에셋/리깅/디자인팀 | `assets/2026-09-02_asset_supply_and_collaboration.md` |

---

## 2. 폴더 구조

| 폴더 | 담는 것 |
|---|---|
| **(최상위)** | 메타 문서만 — `README` / `CURRENT_STATE` / `OPEN_ITEMS` / `CLAUDE`. **내용 문서를 최상위에 두지 말 것** |
| `design/` | 전체 시스템 설계 — 층 구조, 계층 간 계약, 성능/스케일, 채택·미채택 결정 |
| `animation/` | **L4 모션** — 포즈 파이프라인 명세, GASP ABP 분석, 리그/커브/워핑 |
| `ai/` | **L2 판단 · L3 실행** — 인지, 유틸리티, StateTree |
| `squad/` | **L0 명령 · L1 분대** — 분대 조율, 사기, 토큰, 명령 스키마 *(내용이 생기면 `ai/`에서 분리)* |
| `assets/` | 자산 조달, 스켈레톤/리깅, 디자인팀 협업 |
| `prototypes/` | P0/P1 실험 기록 — 무엇을 시도했고 결과가 무엇이었나. **`prototypes/TEMPLATE.md`를 복사해서 쓸 것**(이 파일만 날짜 규칙 예외) |
| `tools/` | MCP 스크립트, 애니메이션 모디파이어, 베이크 툴 *(생기면 신설)* |

**맞는 폴더가 없으면 신설하고 이 표에 추가한다.** 스테이징 폴더는 두지 않는다.

---

## 3. 문서 작성 규칙

상위 `../CLAUDE.md`와 동일:

1. 파일명 `YYYY-MM-DD_짧은-주제.md`
2. 문서 맨 위 1줄 헤더: `날짜 / 상태 / 한줄요약`
3. 폴더는 위 2절 표에서 고른다

### 3.1 ★ 신뢰도 표기 — 이 프로젝트 고유 규칙

**모든 설계·분석 항목에 신뢰도를 붙인다.**

| 표기 | 뜻 |
|---|---|
| **[A] 확정** | 근거를 확인했다(에셋 실측·엔진 소스·공식 문서). 바뀌면 구조가 바뀐다 |
| **[B] 잠정** | 합리적이나 실측 전. 바뀔 수 있다 |
| **[C] 미측정** | 지금은 모른다. **측정 항목과 판정 기준만** 적는다 |

**왜 필요한가**: 이 프로젝트 초기에 "확인하지 않은 것을 세밀하게 쓴" 항목이 **네 번 틀렸다** —
상하체 분리 / 워핑이 8방향을 줄여준다 / IK는 항상 마지막 / 워핑이 실험 경로에만 있다.
넷 다 재료를 직접 만진 뒤에 바로잡혔다. **세밀하게 쓰는 게 문제가 아니라 확인하지 않은 것을
세밀하게 쓰는 게 문제다.**

**[C]가 줄어드는 것이 곧 진척도다.**

### 3.2 추정과 사실을 섞지 않는다

에셋/코드를 직접 읽어 확인한 것과 이름·패턴으로 추론한 것을 **문장 안에서 구분**한다.
추론에는 **(추정)** 을 붙인다. 나중에 뒤집히면 정정 절을 추가하되 **원래 서술을 지우지 말고
취소선으로 남긴다** — 왜 그렇게 판단했는지가 다음 세션에 필요하다.

### 3.3 정정은 문서 끝에 절을 추가한다

본문을 통째로 고쳐쓰면 판단의 이력이 사라진다. 본문에는 `→ 정정: N절 참고`만 달고,
문서 끝에 정정 절을 붙인다. (예: `animation/2026-09-02_gasp_abp_analysis.md`의 14~16절)

---

## 4. 미해결 항목 관리

**모든 미해결 항목은 ID를 갖고 `OPEN_ITEMS.md`에 등록된다.**

| 접두 | 뜻 | 어디서 |
|---|---|---|
| `C-n` | 측정해야 아는 것 (실험 필요) | 각 명세의 [C] 항목 |
| `D-n` | 설계 백로그 (해당 단계에서 채움) | design 문서 |
| `Q-n` | 사용자 결정 필요 | design 문서 |
| `R-n` | 추가 조사 필요 | 각 문서 |
| `U-n` | GASP 미확인 항목 | animation 분석 문서 |
| `W-n` | 분석에서 파생된 작업 | animation 분석 문서 |

- **새 항목을 만들면 `OPEN_ITEMS.md`에 반드시 등록한다.** ID는 전역으로 유일해야 한다
- **해결되면 원 문서에 결과를 쓰고 `OPEN_ITEMS.md`에서 해결 표시**한다. 지우지 않는다

---

## 5. 작업 원칙 (설계 결정에서 파생된 것)

이건 문서 규칙이 아니라 **구현할 때 지켜야 하는 규칙**이다. 어겼을 때 되돌리기 어렵다.

| # | 원칙 | 근거 |
|---|---|---|
| P1 | **GASP ABP를 복제해서 확장한다. 애님 그래프를 새로 짜지 않는다** | MM은 부분적 정확성이 부분적 품질을 주지 않는다. `animation/..._pose_pipeline_spec.md` 8.1절 |
| P2 | **원본 GASP 캐릭터를 지우지 않는다** — 같은 레벨에서 A/B 비교 기준으로 유지 | 위와 동일 |
| P3 | **L4는 `FSoldierPoseIntent`와 월드 말고 아무것도 읽지 않는다** | 액터 변수 직접 참조가 `titan_example` 버그의 근원 |
| P4 | **아군/적군은 코드 한 벌.** 진영은 데이터 | 현행이 두 벌이라 같은 버그를 두 번 고쳤다 |
| P5 | **모든 L0~L3 Tick 진입점에 `HasAuthority()` 게이트** | `titan_example`은 리슨서버 멀티플레이 |
| P6 | **튜닝 대상은 전부 데이터**(DataAsset/DataTable). 코드에 상수를 박지 않는다 | |
| P7 | **디버그 표시는 1급 시민.** 축을 구현하기 전에 HUD 골격부터 | 값이 안 보이면 튜닝이 불가능 |
| P8 | ~~커브 3종 필수~~ ~~5종~~ ~~전 클립 공통은 `contact_l/r` 뿐~~ → **커브는 클립 종류별 세트다. 확정 매핑표를 보고 건다** | **`prototypes/2026-09-04_c34_clip_curve_mapping.md` 4절** ← 유일한 기준. [C-34] 해결 |
| P8c | **Epic의 출하 데이터를 복제하려 하지 말 것 — 일관돼 있지 않다.** 같은 "걸으며 90° 회전"이 커브 3종/4종/6종으로 갈린다. 티어를 따라가는 것도 답이 아니다(Neutral이 더 들쭉날쭉). **소비하는 쪽(PSD 스키마·ABP)이 뭘 읽는지로 규칙을 정해 균일하게 적용한다** | 996클립 전수 실측. 위 문서 3절 |
| P8e | **PSS 스키마가 읽는 커브는 `Phase` 하나뿐이고, 그것도 `PSS_Relaxed_Loops`(루프 전용) 단 하나다.** 나머지 스키마는 전부 Trajectory + Group(Position/Velocity/Heading)만 본다. **우리가 쓰는 `PSS_Default`는 커브를 안 읽는다** | 커브의 주 소비자는 PSD가 아니라 **ABP**다. 위 문서 3.2절 |
| P8d | **애매하면 커브를 뺀다.** 누락 = 그 기능 OFF(`Get Curve Value`가 0을 반환). 잘못 넣으면 기능이 **켜진다** — 정지 클립에 `enable_warping`이 들어가면 발이 미끄러진다. **단 `contact_l/r`만은 예외 없이 만든다**(없으면 발 IK가 조용히 오작동) | 위 문서 3.2절 |
| P8b | **`contact_l/r`은 이진 사각파다.** 엔진 기본 모디파이어로는 못 만든다 → **`UFootContactCurveModifier`**(자작, `Source/SoldierLabEditor/`)를 쓴다 | `MotionExtractor`는 연속값, `FootstepAnimEvents`는 걸음당 1개. `prototypes/2026-09-04_foot_contact_curve_modifier.md` |
| P10 | **임계값·판정 기준을 다룰 땐 조정 전에 계측부터 넣는다** | 접지 임계를 추측으로 세 번 고치다 `UE_LOG`로 분포를 찍자 원인(발 높이 좌우 비대칭)이 즉시 드러났다. 로그는 MCP `LogsToolset.GetLogEntries`로 읽힌다 |
| P11 | **모디파이어 CDO를 바꿔도 이미 애님에 추가된 인스턴스에는 반영되지 않는다** | 추가 시점에 값이 복사된다. 고치려면 Details에서 직접 바꾸거나 **Remove → 재Add**. 2026-09-04에 이걸로 두 번 헛돌았다 |
| P12 | **모디파이어 목록 순서 = "Apply All"의 실행 순서.** 개별 Apply는 무관하지만 목록은 맞춰 둔다 | `EncodeRootBone`이 contact·MoveData·Warping보다 먼저, `FootSteps`가 `BakePhase`보다 먼저. 틀린 채로 Apply All을 누르면 **조용히 망가진다** |
| P15 | **반입 클립은 `bForceRootLock = true`** — GASP 클립이 전부 그렇다 | 루트가 잠기는 조건은 `(bExtractRootMotion && bEnableRootMotion) \|\| bForceRootLock` (`AnimSequence.cpp:1862`). GASP는 루트모션으로 이동하지 않아 **앞 조건이 false**이므로 이 플래그가 유일한 잠금 수단이다. false면 **포즈에 루트 이동이 남아 루프 시 원점으로 순간이동**한다 |
| P16 | **"정답을 아는 대조군" 없이 측정을 신뢰하지 말 것** | 우리 클립만 재서 −56°를 "블레이드"로 단정했으나, GASP 클립을 재니 −90.0°/**sd 0.0** — 데이터가 아니라 **공식이 90° 틀린 것**이었다. `sd = 0`은 공식 오류의 서명 |
| P17 | **에셋을 비교할 땐 항목을 눈으로 고르지 말고 전 프로퍼티를 기계적으로 diff한다** | `bForceRootLock`도 `curveCompressionSettings`도 diff가 뱉어준 것이다. 커브 **이름**만 대조하고 "동일"로 판정했던 것이 오래 헤맨 원인 |
| P24 | **몽타주의 슬롯이 서로 다른 슬롯 그룹에 걸쳐 있으면 몽타주가 통째로 무효가 된다** | `UAnimMontage::HasValidSlotSetup()`이 `All slots must belong to the same group`으로 **false**를 반환하고 재생 자체가 안 된다(`AnimMontage.cpp:989-1031`). 2026-09-10에 `AM_MM_Rifle_Reload`가 `UpperBody`(Partials) + `UpperBodyAdditive`(DefaultGroup)로 갈라져 캐릭터만 미동도 안 했다 — **마이그레이션이 슬롯 그룹을 갈라놓은 것**이다. **`LogAnimMontage` 경고에 정확한 원인이 찍히므로 몽타주가 안 나오면 로그부터 볼 것**(P10과 같은 교훈) |
| P28 | **스켈레탈 컨트롤 노드의 `AlphaInputType = Curve`는 이 프로젝트에서 동작하지 않는다. `Get Curve Value` 노드를 Alpha 핀에 직결할 것** | 노드 내부 구현이 `Cast<UAnimInstance>(proxy)->GetCurveValue()`인데(`AnimNode_SkeletalControlBase.cpp:91`) **항상 0을 반환한다** — `contact_l`(61클립 전부 보유, 매 걸음 0↔1)로 시험해도 무반응이었다. 그래프의 `Animation|Curves|GetCurveValue` 노드를 만들어 `Alpha` 핀에 연결하면 정상 동작한다. GASP가 `Get_AOValue`에서 쓰는 방식이 이것이다 |
| P29 | **효과가 눈에 잘 안 보이는 기능은, 값을 극단으로 몰아 먼저 "작동 자체"를 확인한다** | 왼손 IK가 켜졌는지 꺼졌는지 육안 구별이 안 돼 커브 쪽을 네 번 잘못 뒤졌다. IK 목표를 **120cm 위**로 보내자 한 번에 판정됐고, 그제서야 "IK는 되는데 커브만 안 온다"가 확정됐다. **관찰이 불확실하면 관찰을 개선하는 것이 먼저다** (P10·P16과 같은 계열) |
| P26 | **애님 노드의 알파는 `AlphaInputType`마다 *다른 구조체*를 쓴다. 엉뚱한 것을 고쳐도 경고가 없다** | `Float → AlphaScaleBias` · `Bool → AlphaBoolBlend` · **`Curve → AlphaScaleBiasClamp`** (`AnimNode_SkeletalControlBase.cpp:91`, `AnimNode_ApplyAdditive.cpp:54`). 2026-09-10에 커브 구동 IK를 만들며 `AlphaScaleBias`에 반전(−1,+1)을 넣었는데 커브 경로가 안 읽어 **평상시 IK가 꺼져 있었다.** 커브 게이트를 반전하려면 `AlphaScaleBiasClamp`의 `scale/bias`를 쓸 것 |
| P27 | **"이 구간에는 IK/AO를 끄라"는 판정은 코드가 아니라 애니메이션 커브가 한다** | Lyra는 `disablelhandik`(재장전 등 4클립) · `disablelegik`(5클립), GASP는 `disable_ao` · `enable_warping`을 쓴다. **상황을 열거하지 말고 클립에 커브를 찍는다** — 커브가 없으면 `GetCurveValue`가 0을 반환해 자동으로 기본 동작이 된다. 새 클립을 추가해도 배선을 안 고쳐도 된다 |
| P25 | **핀으로 노출된 애님 노드 프로퍼티는 핀 리터럴이 `node.<prop>`를 이긴다** | `Alpha`·`BlendWeights` 등은 `PinShownByDefault`라 입력 핀으로 나온다. `set_properties`로 `node.alpha=0`을 써도 **핀 값이 1.0으로 남아 있으면 1.0이 쓰인다.** 2026-09-10에 재장전 애디티브를 껐다고 믿고 다른 원인을 세 번 뒤졌다. **핀이 있는 값은 `set_pin_value`로 바꿀 것**이고, `get_node_infos`의 `input_pins[].value`가 진실이다 |
| P23 | **MCP로 AnimGraph 노드를 고친 뒤에는 반드시 `compile_blueprint`를 호출한다. 저장은 컴파일이 아니다** | ABP에는 두 벌의 데이터가 있다 — 디테일 패널이 보여주는 **에디터 노드**(`UAnimGraphNode_*`)와 런타임이 읽는 **생성된 클래스의 `FAnimNode_*`**. 컴파일이 전자를 후자로 굽는데 `set_properties`는 전자만 고친다. → **패널에는 새 값이 보이는데 실행은 옛 값으로 돈다.** 2026-09-09~10에 조준 오프셋이 두 번 "갑자기 고쳐졌는데", 두 번 다 원인은 그 직전의 컴파일이었다. 눈으로 검증해도 안 잡히는 종류다 |
| P20 | **리타깃은 애디티브의 기준 포즈(`refPoseSeq`)를 끊는다 — 그리고 소스 에셋까지 고친다** | `Duplicate and Retarget`이 참조를 새 사본으로 바꿔치기하는데 `ABPT_AnimFrame` **자기 참조**는 끊긴다(AO 포즈 14개 · Actions 9개 실측). 게다가 **소스의 참조도 새 사본을 가리키게 바뀌므로, 리타깃 결과를 지우면 소스가 망가진다.** 다시 리타깃하려면 원본 프로젝트에서 재마이그레이션할 것. 원본 값은 `.uasset` 바이너리의 참조 테이블을 문자열로 읽어 복원할 수 있다 |
| P21 | **`ABPT_LocalAnimFrame`은 `refPoseSeq`가 비어 있는 것이 정상이다** | "**이 애니메이션 자신의** N번 프레임"을 기준으로 쓴다는 뜻이다. 끊김 판정은 **`ABPT_AnimFrame`에만** 적용할 것. 2026-09-09에 29개를 "끊김"으로 오판했다 |
| P22 | **몽타주에는 `AnimationModifiersAssetUserData`가 없다** | 애님시퀀스와 섞어 순회하면 `is not valid Object for property 'instance'`로 **배치 전체가 오염된다**(6.1b). `get_asset_class == "AnimSequence"`로 먼저 거를 것 |
| P18 | **Epic의 비용 편향(`baseCostBias` 등)은 "클립이 많다"를 전제로 튜닝된 값이다. 그대로 복제하면 희소한 세트에서 반대로 작동한다** | GASP는 `Stand_Idles`에 **+0.10 패널티**, `Stand_TurnInPlace`에 **−0.20 할인**을 준다. 클립이 227개라 "idle보다 잘 맞는 게 늘 있다"는 뜻이라 정상이다. 우리는 idle이 **1개**라 이길 방법이 없었고 → **0.5~1초마다 회전 클립으로 튀어 움찔거렸다.** 둘 다 0으로 놓자 해결. **스키마·정규화·검색모드는 복제하되 편향은 우리 밀도로 다시 잡을 것** |
| P30 | **캐릭터의 이동 속도 설정은 클립의 저작 속도를 실측해서 맞춘다. Epic의 속도 "비율"을 옮겨오면 안 된다** | GASP는 `walkSpeeds (200,180,150)` · `runSpeeds (500,350,300)` = 1:0.9:0.75 / 1:0.7:0.6 인데, 이는 **GASP 클립이 방향별로 다른 속도로 저작돼 있기 때문**이다. **Lyra 라이플 세트는 전 방향 단일 속도다** — Walk 291.31 / Jog 582.62 / Crouch 291.31, 방향 간 오차 0.001 (2026-09-11 실측). 비율만 옮겨오자 옆 **−30%**, 뒤 **−40%** 어긋났고 재생속도 클램프(`playRate 0.85–1.15`)로는 흡수가 불가능해 **Loop가 영영 선택되지 않았다.** P18과 같은 계열 — **Epic의 숫자는 Epic의 데이터에 대해서만 옳다** |
| P31 | **발 미끄러짐이 없다는 것은 이동 속도가 맞다는 증거가 아니다** | 속도가 어긋나면 MM은 Loop 대신 **Start/Stop/Pivot을 고른다.** 이 클립들은 가감속 구간이라 **0~최고속의 모든 속도 프레임을 갖고 있어** 어떤 요구 속도에도 맞는 프레임이 하나는 있다 — 일종의 "속도 뷔페"다. 그래서 미끄러짐은 사라지고 대신 **매 걸음 클립이 재선택되는 떨림**으로 나온다. 2026-09-11에 이 오판정 때문에 [C-58]을 "넘어가도 된다"고 잘못 안내했고, 워핑·발배치·연속포즈 편향을 차례로 헛짚었다. **속도 검증은 `a.AnimNode.MotionMatching.DebugDrawInfoVerbose 1`로 Loop가 실제로 뽑히는지 볼 것** |
| P19 | **PSD를 복제할 때 `normalizationSet`도 같이 딸려온다 — 우리 DB는 그 세트의 멤버가 아니다** | 정규화 세트는 **등록된 DB들의 통계**로 특징 척도를 정해 비용을 비교 가능하게 만든다. Epic 것을 가리키면 GASP 25개 DB의 척도로 우리 클립을 재게 된다. **우리 DB만 담은 PSN을 따로 만들 것** (2026-09-09 `PSN_Rifle_All`) |
| P13 | **새 `UCLASS`는 Live Coding으로 안 들어간다.** 에디터를 닫고 빌드해야 한다 | `Unable to build while Live Coding is active`. 단 **UHT는 먼저 돌므로 에디터를 안 닫아도 리플렉션 문법 오류는 잡힌다**(`Build.bat`가 `N generated files written`까지 진행) |
| P14 | **자작 모디파이어에서 루트모션을 읽을 땐 `bIncorporateRootMotionIntoPose = true`** (= `bIgnoreRootLock`) | 빠뜨리면 루트가 고정돼 **접지 속도가 전부 이동속도만큼 뜨고, yaw 프로파일은 평평하게 나온다.** 2026-09-04와 09-08에 같은 함정을 두 번 만났다 |
| P9 | **모디파이어 적용 순서: `EncodeRootBone` → `AM_WarpingAlpha`.** 그리고 생성된 커브를 **눈으로 확인**한다 | `AM_WarpingAlpha`는 루트모션을 읽어 판정한다. 루트모션이 없으면 **에러 없이 전 구간 1**이 나온다. `prototypes/2026-09-03_enable_warping_curve_generation.md` 3.4절 |

---

## 6. 언리얼 작업 환경

| 항목 | 값 |
|---|---|
| 프로젝트 | `C:\working\kadex\anim_test\SoldierLab` (UE5.8, GASP 기반) — **2026-09-03 PC 이관으로 `works\` 가 빠졌다** |
| MCP 포트 | **8000** (2026-09-03 변경. 이전 8001. 언리얼 쪽 `ServerPortNumber`가 8000이라 `.mcp.json`을 맞췄다. `titan_example`을 같은 PC에서 동시에 띄우면 충돌하니 그때 다시 분리할 것) |
| `.mcp.json` | `anim_test/SoldierLab/` 에만 있음 (`anim_test/` 쪽은 이관 후 없음) |
| MCP 사전 조건 | Project Settings → Model Context Protocol → **Auto Start Server 켜기** (프로젝트별 설정) |
| 유용한 툴셋 | `editor_toolset.*`, `state_tree_toolset.*`, `animation_toolset.*`, `ProgrammaticToolset`(배치 실행) |

### 6.1 MCP로 **접근이 안 되는 것** (에디터에서 수동 확인 필요)

- ~~애님 스테이트머신 내부 그래프~~ · ~~AnimGraph 노드~~ → **정정(2026-09-08): 읽힌다.**
  `read_graph_dsl`은 K2 전용이라 AnimGraph에서 **빈 문자열**을 주지만,
  **`find_nodes(graph, title="")`가 AnimGraph 노드 목록을 그대로 준다.** 그 다음
  `ObjectTools.get_properties(<노드경로>, ["node"])`로 **노드 설정 전체가 읽힌다**
  (워핑 파라미터·스파인 본 목록·알파·링크 연결 여부까지).
  중첩 그래프는 경로로 구분된다 — `:AnimGraph.AnimGraphNode_MotionMatching_0.AnimationBlendStackGraph_0`
  vs `...AnimGraphNode_BlendStack_3.AnimationBlendStackGraph_0`.
  `linkId = -1`이면 **연결 안 된 노드**다(평가 경로 밖).
  ⚠ 다만 `AnimGraphNodeBinding_Base_0`(프로퍼티 바인딩 대상)은 여전히 안 읽힌다
- ~~애님 모디파이어 BP의 그래프 본문~~ → **정정(2026-09-03): 읽힌다.** `AM_WarpingAlpha`의
  EventGraph가 3,964자로 읽혔다. 앞서 `AM_OrientationWarpingAlpha`에서 빈 문자열이 나온 것은
  그 에셋 개별 사정으로 보인다. **그래프를 못 읽는다고 단정하지 말 것**
- ⚠ **단 `read_graph_dsl` 출력은 손실이 있다.** 구조체 핀 분해(break) 노드가
  `(ToolMenus|Get)` 으로 오역되어 나온다 — `AM_WarpingAlpha`에서 벡터 location 3곳이 그렇다.
  **읽기는 참고용으로만 쓰고, 복잡한 그래프를 `write_graph_dsl`로 왕복시키지 말 것**
- ⚠⚠ **읽기 출력의 노드 이름을 그대로 쓰기에 되먹이면 실패한다.** 읽기는 `Set_CharacterInputState`로
  보여주지만 실제 `type_id`는 `SetCharacterInputState`다(밑줄 없음). 쓰기 전에는 **반드시**
  `find_node_types(graph, type_id_filter, context_pins=[])` → `get_node_type_pins` 로 실제 id와
  핀 이름을 확인할 것. 구조체 Make 노드의 핀에는 **GUID 접미사**가 붙는다
- ⚠ **인터페이스 함수는 `(Message)` 접미 버전을 쓸 것.** 일반 버전으로 쓰면 컴파일은 되지만
  **하드 캐스트가 자동 삽입**되어 다른 구현 클래스에서 깨진다 (2026-09-04 `SetCharacterInputState` 사례)
- ⚠ **`add_variable`로 만든 변수는 기본이 비공개**(Instance Editable = false)라 **StateTree 태스크
  파라미터로 노출되지 않는다.** 컴파일도 되고 그래프도 맞는데 에디터에서 값을 못 넣는다.
  **`set_variable_instance_editable`을 반드시 같이 호출할 것** (파라미터명은 `variable_name`)
- ~~FBX 임포트~~ → **정정: 된다.** `AssetTools`에는 없지만
  **`SkeletalMeshTools.import_file`** 이 있다(`folder_path`, `asset_name`, `source_file`,
  `skeleton`, `import_materials/textures/animations`, `create_physics_asset`).
  **클립 반입을 MCP로 자동화할 수 있다** — 수십 개 일괄 반입 시 중요
- **애님시퀀스의 커브 *값*** — `get_asset_tags`의 `CurveNameList`로 **이름은** 읽히지만 값은 못 읽는다.
  `animationTrackNames`/`dataModel`은 UE5.8에서 비어 있는 구식 프로퍼티고, 실 데이터는
  `dataModelInterface`(`IAnimationDataModel`) 뒤라 노출되지 않는다 → **커브 모양 검수는 에디터에서**
- ⚠⚠ **레벨에 배치된 액터의 프로퍼티 오버라이드** — MCP로 **CDO(클래스 기본값)를 바꿔도
  배치된 인스턴스가 그 프로퍼티를 오버라이드하고 있으면 반영되지 않는다.** 오버라이드 여부는
  **레벨을 열어야만** 보이고 에셋만 봐서는 알 수 없다. 2026-09-09에 `WalkSpeeds`를 네 번 바꿨는데
  전부 무시됐고, 그 사이 측정이 전부 오염됐다 → **[C-50]**. 값을 바꾼 뒤 거동이 안 변하면
  **가장 먼저 인스턴스 오버라이드를 의심할 것**
- **애님시퀀스의 노티파이 목록** — `get_asset_tags`의 `AnimNotifyList`는 **BP 노티파이를 안 잡는다.**
  발자국 노티파이가 실제로 있는 클립(`Walking_Anim`)에서도 빈 값(`";"`)이 나왔다.
  **"노티파이가 없다"는 판정에 이 태그를 쓰지 말 것** (2026-09-04에 이걸로 오판할 뻔했다)
- **`IKRigDefinition`의 체인·리타깃 정의** — `list_properties`가 `drawGoals`/`goalSize` 같은
  표시용 프로퍼티만 준다. 체인 구성은 에디터에서 볼 것
- **BlendStack 내부 그래프**(`AnimationBlendStackGraph`) — MM 노드 더블클릭으로 열림
- 노드의 프로퍼티 **바인딩** 대상
- ⚠⚠ **PSD의 클립 목록 `animationAssets`** — `TArray<FInstancedStruct>`라 **읽기·쓰기 둘 다 실패**한다.
  DB 자산 생성·스키마·비용 편향·태그는 전부 MCP로 되지만 **클립을 넣는 것만은 에디터**다 (2026-09-09)
- ⚠⚠ **Chooser 테이블의 Result(행 결과) `ResultsStructs`** — `#if WITH_EDITORONLY_DATA` 안에 있어
  `list_properties`에 아예 안 나온다(`Chooser.h:156-159`). 반면 **`columnsStructs`는 런타임
  프로퍼티라 읽기·쓰기 둘 다 된다.** → **열은 자동화, 행 결과는 수동**이 원칙이다.
  일반화: **에디터 전용 프로퍼티는 MCP에 노출되지 않는다** — 못 읽으면 헤더에서 `WITH_EDITORONLY_DATA`를 의심할 것
- StateTree의 `get_node_description` (구조는 읽히나 설명은 실패)

### 6.1b MCP 요령 — 잘 되는데 경로가 까다로운 것

- **BP 모디파이어의 CDO 값**: `<에셋경로>.Default__<이름>_C` 로 읽는다.
  `<에셋경로>.<이름>_C`(클래스 자체)는 `list_properties`는 되지만 `get_properties`가 전부 실패한다
- **애님시퀀스에 붙은 모디파이어 인스턴스 목록**:
  `<경로>.<이름>:AnimationModifiersAssetUserData_0` 의 `animationModifierInstances`.
  **어떤 클립에 뭘 걸었는지 원격으로 검수할 수 있다.**
  단 GASP 출하 클립은 전부 비어 있다(적용 후 제거됨) — **우리 클립 검수용**이다
- ⚠ **`execute_tool_script`는 `try/except`로 감싸도 소용없다.** 안에서 툴 호출이 한 번이라도
  실패하면 **배치 전체가 실패로 반환**된다. 긴 배치가 도는 중에 다른 MCP 호출을 섞지 말 것
- `find_assets`의 `name`은 **접두사 매칭**이다. `""`(빈 문자열)이 전체 검색, `"*"`는 0건
- `get_asset_tags`의 `CurveNameList`·`AnimNotifyList`는 **세미콜론 구분**이다. 쉼표로 파싱하면
  전 항목이 "누락"으로 오판된다 (2026-09-09에 61클립을 전부 실패로 오독했다)
- `execute_tool_script` 안에서 **`collections` 등은 import 금지**다. 허용: `copy datetime json time re math`.
  `dict.get(k, default)`도 막혀 있다(`_StrictDict`) — `k in d` 로 분기할 것. `run()`은 **dict만** 반환 가능

### 6.1d ★ Chooser 테이블의 의미론 (2026-09-09, 엔진 소스 확인)

| 사실 | 근거 |
|---|---|
| **첫 매치에서 멈추지 않는다.** 모든 열 필터를 통과한 **모든 행**의 결과가 수집된다 | `Chooser.cpp:712-713` "of the rows that passed all column filters, iterate through them calling the callback until it returns Stop" |
| 행 결과 타입은 **`Asset`(자산 1개) · `Nested Chooser` · `Evaluate Chooser`** 뿐 — **목록 타입은 없다** | `ObjectChooser_Asset.h:11,28` · `Chooser.h:220,240` |
| 한 행에서 여러 자산을 내려면 결과를 `Nested Chooser`로 두고 **하위 표에 필터 열 없이** 행마다 자산 하나씩 | GASP `CHT_..._Dense`: PSD 35개 참조 · 외부 챙터 0개 |
| 행 개수는 **에디터 전용** `ResultsStructs`가 정하고, `SetNumRows`가 열의 초과분을 **잘라낸다** | `IChooserColumn.h:138-141` |

> ★ **설계 함의**: "위 행이 먼저 먹으니 아래는 ANY로 둬도 된다"는 **틀렸다.** 둘 다 통과한다.
> 행은 **상호배타로 설계**할 것. `MatchNotEqual`(`EnumColumn.h:20`)이 그 도구다.
>
> 뒤집어 보면, **DB 하나당 행 하나를 두고 같은 조건의 행들을 함께 통과시키면
> Nested Chooser 없이도 여러 자산을 낼 수 있다** — 에디터 작업이 훨씬 단순해진다.
>
> ★ **작업 순서**: 열 값을 MCP로 주입하려면 **에디터에서 행 개수를 먼저 확정**해야 한다.
> 행이 적은 상태에서 미리 넣으면 조용히 잘린다.
>
> ★ **행 개수는 `columnsStructs`의 `rowValues` 길이로 원격 확인된다.** `ResultsStructs`를 못 읽어도
> 표가 갱신될 때 `SetNumRows(결과개수)`가 열을 맞춰주기 때문이다(`SChooserTableWidget.cpp:161-166`).
> **어떤 행에 어떤 자산이 있는지는 여전히 못 읽으므로**, 사용자에게 **조건이 같은 것끼리 묶어
> 순서대로 넣게** 하면 묶음 내부 순서와 무관하게 조건을 주입할 수 있다

- **에셋을 여러 개 끌어다 놓으면 Chooser에 행이 에셋당 하나씩 생긴다** — 드롭다운을 N번 누를
  필요가 없다 (`SChooserTableRow.cpp:526-551`, 트랜잭션명 "Drag and Drop Assets into Chooser").
  `ResultType == ObjectResult`인 표에서만 동작한다

> ### ⚠⚠ 에디터에 **열려 있는 에셋**의 배열 프로퍼티를 MCP로 갈면 **에디터가 죽는다**
>
> 2026-09-09 실측. `CHT_Soldier_Databases`를 연 채로 `columnsStructs`를 `[]`로 비웠더니
> **`UnrealEditor_ChooserEditor` 안에서 Slate `Prepass` 중 크래시**했다
> (`UECC-Windows-E635...`, `Slate.txt`: `Context:'Prepass' Type:'SWindow'`).
> 표 위젯이 열 배열을 순회하는 중에 배열이 통째로 사라진 것이다.
> 같은 쓰기를 **닫힌 상태**로는 세 번 문제없이 했다.
>
> **규칙: 배열을 통째로 교체하기 전에 "그 에셋을 에디터에서 닫아 달라"고 먼저 요청할 것.**
> 단일 스칼라 프로퍼티 쓰기는 열려 있어도 괜찮았다 — 위험한 것은 **크기가 바뀌는 배열 쓰기**다.
>
> 부수 교훈: `save_assets([])`(전체 저장)은 느리고 크래시 시 원인 추적을 흐린다.
> **해당 에셋만 지정해 저장할 것.**
>
> 크래시 후 복구는 **로그로 판단한다** — `Saved/Crashes/<...>/SoldierLab.log`에서
> `LogFileHelpers: Saving Package: <에셋>` 시각과 크래시 시각을 비교하면 무엇이 디스크에
> 남았는지 알 수 있다. 위 사고에서는 크래시 56초 전에 저장이 끝나 **사용자 작업이 보존**됐다

### 6.1c ★ 배열 프로퍼티 쓰기 규칙 (2026-09-09 확립)

`set_properties`로 `TArray`를 쓸 때 세 가지 제약이 있다. 챙터 열을 만들며 전부 밟았다:

| 제약 | 증상 | 대응 |
|---|---|---|
| **크기 변경과 내용 변경을 동시에 못 한다** | `ArrayAdd: elements changed alongside the size change; insertion points are ambiguous` | 두 번에 나눠 호출 |
| **인스턴스 구조체의 _타입_은 제자리에서 못 바꾼다** | `_structType`이 옛것으로 남고 이름이 겹치는 필드만 반영된다 → `FloatRangeColumn`인데 바인딩만 `Stance`인 잡종이 생긴다 | 아래 |
| **축소는 "끝에서" 일어난다** — 인덱스 기준이지 동일성 기준이 아니다 | 0번을 빼려고 나머지만 써 보내면, **0번이 남고 뒷것이 지워진 뒤 0번 내용만 덮어써진다** | 아래 |

> ★ **해법은 하나다 — `[]`로 완전히 비우고 나서 원하는 배열을 통째로 쓴다.**
> 빈 배열에서 출발하면 전부 순수 add라 세 제약이 모두 비껴간다.
> ```python
> setcols([])      # 1) 비우고
> setcols(WANT)    # 2) 통째로
> ```
> 되먹임도 주의: **쓰기 후 엔진이 재직렬화**하므로 직전에 보낸 사본을 그대로 다시 보내면
> "elements changed"로 거부된다. 이어서 쓰려면 **반드시 다시 읽을 것**
>
> ⚠⚠ **단 "비우기"가 안전하지 않은 배열이 있다.** 엔진 코드가 원소 존재를 전제하고 무조건
> 0번을 인덱싱하면 **에디터가 어서션으로 죽는다.** 2026-09-09 실측 — 블렌드스페이스의
> `sampleData`를 `[]`로 만들자 즉사했다:
> ```
> Assertion failed: (Index >= 0) & (Index < ArrayNum)
> Array index out of bounds: 0 into an array of size 0
> ```
> **처음 보는 자산 타입의 배열을 비우기 전에는 사본으로 먼저 시험할 것.**
> 크기를 유지한 채 내용만 바꾸는 것은 안전하다.

- ⚠ **`skeleton`은 쓰기 불가다**(`AimOffsetBlendSpace` 실측). 즉 **MCP로 다른 스켈레톤용 애니메이션
  자산을 만들 수 없다** — 복제하면 원본 스켈레톤에 묶인 채로 온다.
  **자산 생성만 에디터에서 하고, 내용은 MCP로 채우는** 분업이 필요하다

### 6.2 GASP 콘솔 변수

```
DDCvar.DrawCharacterDebugShapes 1        디버그 도형 (궤적·접지)
DDCVar.LocomotionSetupCMC <int>          로코모션 경로 전환 ★ 실제 스위치
DDCvar.MMDatabaseLOD 0|1|2               데이터 밀도 티어
a.animnode.offsetrootbone.enable 0|1     ⚠ 끄면 조준/발이 깨진다 (규약 문제)
DDCVar.ThreadSafeAnimationUpdate.Enable  45명 규모에서 필수
DDCVar.ExperimentalStateMachine.Enable   ❌ 덮어써져서 효과 없음
```

### 6.2b ★ 애니메이션 이분법 디버깅 키트 (엔진 CVar, 2026-09-11)

**전부 런타임 CVar다** — PIE 중 `~` 콘솔에서 켜고 끄면 즉시 반영된다.
에셋 수정도 `compile_blueprint`도 재시작도 필요 없다. 증상의 범인을 **한 번의 PIE로 이분한다.**

```
관찰 ─ 무엇이 재생 중인가
a.AnimNode.MotionMatching.DebugDrawInfo 1          현재 DB · 현재 클립을 머리 위에 표시
a.AnimNode.MotionMatching.DebugDrawInfoVerbose 1   + 검색 대상 DB 목록 + 블렌드 스택 전체
                                                     (클립명 · time · playrate) ★ 가장 유용
a.AnimNode.OrientationWarping.Debug 1              빨강=목표방향 파랑=클립 루트모션 초록=워핑 결과

차단 ─ 끄고 증상이 사라지는지 본다
a.AnimNode.OrientationWarping.Enable 0             오리엔테이션 워핑
a.AnimNode.FootPlacement.Enable 0                  발 배치 전체
a.AnimNode.FootPlacement.Enable.Lock 0             발 고정만
```

`DebugDrawInfoVerbose`가 특히 결정적이다. **Loop가 아니라 Start/Stop/Pivot이 매 걸음
재선택되고 있으면 그것은 곧 이동 속도 불일치다** (P30·P31). 소스는
`AnimNode_MotionMatching.cpp:121-139` · `AnimNode_OrientationWarping.cpp:20-32` ·
`AnimNode_FootPlacement.cpp:20-24`.

---

## 7. 새 환경(다른 PC)에서 시작하기

> 2026-09-03 추가. 프로젝트가 Perforce(P4V)로 관리되며, 작업 PC가 바뀔 수 있다.
> **아래 순서를 건너뛰면 에디터가 안 열리거나 MCP가 안 붙는다.**

### 7.1 순서

```
1. P4에서 SoldierLab 워크스페이스 동기화
2. ★ C++ 빌드          ← Binaries/ 가 P4에서 제외돼 있으므로 필수
3. 에디터 최초 실행     ← 셰이더 컴파일 + DDC 빌드로 오래 걸림 (에셋 5.5GB)
4. MCP Auto Start Server 켜기   ← 프로젝트별 설정이라 PC마다 다시 켜야 함
5. Claude Code 재시작 → /mcp 로 연결 확인
6. 플러그인 활성화 확인 (7.3)
7. PIE 동작 확인 → CURRENT_STATE.md 읽고 작업 시작
```

> **2026-09-04 이후 C++ 빌드는 선택이 아니다.** 자작 모디파이어
> `UFootContactCurveModifier`가 **에디터 모듈 `Source/SoldierLabEditor/`** 에 있다.
> 빌드하지 않으면 `contact_l/r`을 만들 수 없고, 모디파이어 목록에 아예 안 나타난다.

### 7.2 ★ 가장 흔한 함정 — C++ 빌드를 안 하고 여는 것

`.p4ignore`가 `Binaries/`·`Intermediate/`를 제외하므로, **동기화 직후에는 컴파일된 모듈이
없다.** 그냥 `.uproject`를 더블클릭하면 "모듈이 없습니다. 다시 빌드하시겠습니까?"가 뜨거나
실패한다.

```
1) SoldierLab.uproject 우클릭 → Generate Visual Studio project files
2) 생성된 .sln 열어서 Development Editor / Win64 로 빌드
3) 그 다음에 에디터 실행
```

### 7.3 플러그인 확인 — `.uproject`가 진실이다

플러그인 활성화는 `SoldierLab.uproject`에 기록되고 P4로 추적되므로 **동기화하면 따라온다.**
다만 아래는 아직 켜지지 않았을 수 있으니 확인할 것(2026-09-03 기준 미확인):

```
SmartObjects, GameplayBehaviorSmartObjects, StateTree, GameplayStateTree,
FullBodyIK, NavCorridor, AnimationModifierLibrary,
AnimationBudgetAllocator, SignificanceManager, AnimationSharing,
GameplayInsights(=Animation Insights, Rewind Debugger)
```

**⚠ 이름에 `UAF`가 붙은 것은 켜지 말 것** — 차세대 프레임워크(AnimNext)용 별도 모듈이고
우리는 기존 Animation Blueprint 경로를 쓴다. `UAF Chooser` / `UAF Pose Search` 등이 검색에
같이 나오니 주의.

### 7.4 MCP 재설정

`.mcp.json`(포트 8000)은 P4로 추적되므로 따라온다. **하지만 언리얼 쪽 서버는 프로젝트별
사용자 설정이라 PC마다 다시 켜야 한다:**

```
Project Settings → Model Context Protocol → Auto Start Server  체크
(포트 확인 → 에디터 재시작)
```

**⚠ 2026-09-03 실제로 걸린 함정**: 언리얼 쪽 설정은 PC마다 다시 잡히는데(`Saved/Config/
WindowsEditor/EditorPerProjectUserSettings.ini` → `[/Script/ModelContextProtocolEngine.
ModelContextProtocolSettings] ServerPortNumber`), `.mcp.json`은 P4로 따라오므로 **둘이
어긋난다.** 이관 PC에서 언리얼은 8000, `.mcp.json`은 8001이라 연결이 거부됐다.

**진단 순서** — 붙지 않으면 Auto Start를 의심하기 전에 포트부터 맞춰볼 것:

```powershell
# 1) 언리얼이 실제로 어느 포트를 여는가
Get-NetTCPConnection -State Listen | Where-Object { $_.LocalPort -in 8000,8001 }
# 2) .mcp.json 의 url 과 같은가
```

붙었는지 확인: Claude Code에서 `/mcp`, 또는 레벨 액터 조회를 한 번 시켜볼 것.

### 7.5 경로에 대하여

문서에 나오는 절대경로는 **최초 작업 PC 기준의 참고값**이다. 새 환경에서는 다를 수 있다.

| 항목 | 최초 PC 경로 | 새 환경 |
|---|---|---|
| UE 프로젝트 | `C:\working\works\kadex\anim_test\SoldierLab` | **현 PC: `C:\working\kadex\anim_test\SoldierLab`** |
| 엔진 | `C:\Program Files\Epic Games\UE_5.8` | 현 PC 동일 |
| 문서(이 폴더) | `C:\working\insung_grapic\titan\soldier_ai_lab` | **현 PC: `C:\mine\insung_grapic\titan\soldier_ai_lab`** |
| `titan_example` | `C:\working\works\kadex\titan_example` | **없을 수도 있다** — 참조용이며 필수 아님 |

`../../ai_combat/` 같은 상위 참조는 **titan 문서 저장소가 함께 있을 때만** 열린다. 없어도
soldier_ai_lab 문서만으로 작업할 수 있게 써 두었다.

### 7.6 첫 세션 체크리스트

- [ ] C++ 빌드 성공, 에디터 열림
- [ ] `Content/Levels/DefaultLevel` PIE — 캐릭터 조작 정상
- [ ] `Content/Levels/NPCLevel` PIE — AI NPC 3기가 순찰·벤치 상호작용
- [ ] MCP 연결 (`/mcp`)
- [ ] `DDCvar.DrawCharacterDebugShapes 1` 동작
- [ ] `CURRENT_STATE.md` 4절의 "다음 작업" 확인
