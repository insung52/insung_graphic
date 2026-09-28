# 분대 스코프 상황 필드 · 엣지 전진(파이 자르기의 창발) · AI 가 소유하는 사격 콘 · 섀도우 수요 감축

2026-09-21 / 진행중 (① 분대 필드 · ③ 사격 모델 = **빌드·PIE 확인**(Perforce CL 498 · 500, 사용자 보고) · ② 엣지 전진 = **CL 500 에 포함돼 빌드됨, PIE 판정은 대기**([W95]) · ④ 섀도우 감축 + 비용 줄 = ~~작업 트리, 빌드 전 [B]([W96])~~ → **같은 날 늦게 빌드·PIE ✅ "아주 잘됨", [W96] 해결 — 10절** · 값은 전부 [C-157]~[C-161]; **같은 세션 후반의 오버레이 노출 보정은 별도 문서 `ai/2026-09-21_debug_overlay_exposure.md`(12절 포인터, P181, [W97])**) / 09-18 밤 이후의 AI 세션 묶음 넷. **(1)** 상황 필드가 진영 하나가 아니라 **분대 하나당 하나**가 됐다(`FScope` = 진영 × 분대 슬롯, 무전으로 들은 것만 남의 분대 필드에 들어간다, 명령 안 받는 Identity(UGV)는 스코프도 앵커도 아니다 — 오버레이가 UGV 를 중심으로 잡던 원인). **(2)** 09-18 저녁의 코너 멈춤(+ 잠깐 있었던 손으로 그린 파이 호)을 **엣지 전진**으로 대체 — 콘 스윕의 광선에서 **엣지**(옆 광선은 멀리 가는데 짧게 멈춘 광선)를 읽고, 한 걸음이 **무엇을 여는가**를 필드의 쐐기 적분(`GetWedgePresence`, 구운 호라이즌으로 가림, 트레이스 0)으로 값을 매겨 **예산 안에서 가장 멀리 가는 걸음**을 딛고, 연 조각이 눈에 익을 때까지 본다. 호도 타이머도 없다 — 파이 자르기는 **정보 굶주림 대 위험**에서 창발한다. **(3)** 사격 콘을 무기의 고정 3° 에서 **AI 가 소유하는 콘**으로: 정착 0.8° × 이동 × 자세 × (1+반동) + **조준 흔들림**(선회 뒤 2.5° 지수 정착 0.4 s, 발마다 +0.6°, 이동 중 바닥 1.5°) → 새 의도 `Settling`(정착하면 맞는다, 기다린다) · **버스트**(2~5발, 0.5 s × 지터 0.35, 병사 이름으로 시드) → `Pacing`. **(4)** 따라가는 라이트의 그림자 재캐스트를 **한 셀 이동 ∧ 0.5 s** 문턱으로, 같은 자리의 다른 분대 라이트는 **한 벌의 트레이스에 동승**(riders), 디버그 헤더 3줄째 **비용 줄**. 원칙 **P176~P180**.

전편: `ai/2026-09-18_patrol_scan_and_move_robustness.md`(7.3절 코너 멈춤 C — **이 문서 2절이 대체**) · `ai/2026-09-17_situation_field_lighting_model.md`(필드 시스템 문서 — 스코프·쐐기·섀도우는 **이 문서 1·2.4·4절이 최신**, 그쪽 25절은 포인터만) · `ai/2026-09-13_engagement_and_cover.md`(사격 결정 원형 — 콘·의도는 **이 문서 3절이 최신**) · `squad/2026-09-18_squad_layer_fixes_quota_engage_range.md`(`bTakesSquadOrders` 도입).
원칙: **신설 P176~P180**(`CLAUDE.md` 5절) · P143(출처 태그) · P144(눈과 귀만 쓴다) · P145(라이트의 주인은 목격) · P149(모름 ≠ 없음) · P150(속도는 노출로 산다) · P162(유예) · P164(경로가 아니라 비용) · P172(결정 이산 · 움직임 연속) · P10(계측 먼저) · P7(디버그 1급 시민). 짝: [W74](분대 필드, **해결**) · [W85](진짜 파이 자르기, **해결**).

> 신뢰도: **[A]** 코드로 확인(file:line, **2026-09-21 작업 트리 판** — `SoldierSituationField.cpp` 3036줄 / `.h` 610줄 · `SoldierCover.cpp` 2374줄 / `.h` 968줄 · `SoldierEngagement.cpp` 1712줄 / `.h` 1006줄 · `SoldierSight.cpp` 410줄 / `.h` 170줄) · **[B]** 잠정(코드는 있으나 빌드 전, 또는 계산값) · **[C]** 미측정. ⚠ 코드 주석의 날짜는 ①②③이 **2026-09-18**, ④가 **2026-09-21** 이다 — 09-18 밤 문서 뒤에 이어진 같은 세션의 작업이고 CL 498/500 은 그 사이다(CL 내용은 이 세션이 p4 로 확인하지 않았다, 사용자 보고).

---

## 0. 한 장 요약

| 무엇 | 어디 | 상태 |
|---|---|---|
| **분대 스코프** — `FScope{Faction, SquadId, Levels, Lights, Anchors, EvictionQueue}`, `Scopes.SetNum(3 × MaxSquadsPerFaction)`(인덱스 = 진영 × N + 슬롯, Neutral 몫은 안 씀), 슬롯은 `ScopeFor(Faction, SquadId)` 가 **처음 말하는 순서**로, 넘치면 슬롯 0 공유 + 분대당 1회 경고 | `SoldierSituationField.h:546-564` · `.cpp:499-559`(`ScopeFor`) · `:656-700`(`EnsureLevels`) · `SoldierFieldSettings.h:94-101`(`MaxSquadsPerFaction 3`) | [A] · PIE ✅ |
| **공개 API 전부 `const USoldierIdentityComponent* Who`** — 스코프 −1 이면 no-op / 0 / false / `Max` | `.h:137-233` · `.cpp:561-652` | [A] · PIE ✅ |
| **호라이즌은 계속 세계 공유** — `Horizons`/`CoarseHorizons`/`AllAnchors` 는 스코프 밖 | `.h:566-575` · `.cpp:153-168` | [A] |
| **무전 → 필드**: `ReceiveSharedRecord` 가 받는 쪽 Identity 로 `ReportSighting(…, ObservedTimeSeconds)` — 관측 시각 그대로, 반경 부풀림·확신 감쇠 뒤 | `SoldierPerception.cpp:400-434` · `Refresh` 의 `LastSeenTime = max`(`SoldierSituationField.cpp:1618-1620`) | [A] · PIE ✅ |
| **`bTakesSquadOrders=false` = 병사 아님** — 스코프 −1 · 스코프 앵커 아님(호라이즌 앵커엔 들어감) · `GetObservedSoldier` 가 조종 폰/뷰타겟으로 안 잡음 | `.cpp:548-559` · `:156-167` · `SoldierDebugDraw.cpp:98-120` | [A] · PIE ✅ (오버레이가 UGV 중심으로 잡히던 것 해결) |
| **cvar** `SoldierLab.Debug.Field.Squad −1`(슬롯 고정) · `.Centre 0|1`(1 = 그린 분대의 병사 전원 주위에 링) · 헤더 `[Field] HOSTILE/1 | …  (following|pinned)  centred on the squad (N)` | `.cpp:43-71` · `:359-370` · `:444-492` | [A] · PIE ✅ |
| **`GetWedgePresence(Who, Apex, DirA, DirB, RangeCm, Now)`** — 쐐기 안 경계도 × m², 표본 셀의 **자기 호라이즌**(apex 쪽 bin)과 solid 로 가림, 트레이스 0, 안 구운 셀은 사전값으로 세고 큐에 | `.h:217-229` · `.cpp:2473-2548` | [A] · 빌드됨(CL 500) · PIE [W95] |
| **`FSoldierSweepRay` / `GetSweepRays()`** — 콘 스윕 광선을 슬롯별로 보관(눈·방향·멈춘 점·막힘·시각) | `SoldierSight.h:13-25` · `:122-127` · `.cpp:186-196` | [A] · 빌드됨 |
| **엣지 전진** `USoldierCoverComponent` — `FindViewEdges` → `UpdateEdgeAdvance` → `IssueAdvanceStep`/`OpenedByStep`/`GlowInWedges` → `EndAdvance`/`BreakAdvanceForContact`, `GetAdvanceView`/`IsAdvancing`/`IsAdvanceLooking` | `SoldierCover.h:285-374` · `:861-920` · `.cpp:766-1226` · `:2152-2168`(Tick) · `:1985-1987`(bAlreadyGoing) · `:2075-2078`(새 이동이 전진 취소) · `:694-711`(후퇴 후보) | [A] 코드 · **PIE 대기 [W95]** · 값 [C-158] |
| **교전 연동** — `GetAdvanceView` → 조준점 + `DesiredLean` · `bWantsToAim = 접촉 ∥ 전진` · 전진 ∧ 무접촉 = **Walk**(스프린트 해제) · 급박도 `UrgencyLookPeek` | `SoldierEngagement.cpp:1391-1403` · `:1532-1535` · `:1554-1561` · `:1611-1615` | [A] |
| **사격 콘 `GetShotSpreadDegrees()`** = `WeaponSpreadDegrees 0.8` × `(1 + MovementSpreadScale 6 × v/600)` × 자세(`Lean 1.5` / `Blind 15`) × `(1 + RecoilSpread)` + `AimSettleDeg` | `SoldierEngagement.h:352-406` · `.cpp:807-822`(무접촉) · `:1221-1236`(접촉) | [A] · PIE ✅ (무기 BP 는 아직 안 읽음 [W93]) |
| **조준 흔들림 `AimSettleDeg`** — 선회 중 `max(·, AimSettleInitialDeg 2.5)`, 매 틱 `×exp(−dt/AimSettleSeconds 0.4)`, 발마다 `+RecoilKickDeg 0.6`, 바닥 `MoveWobbleDeg 1.5 × min(1.5, v/600)`, 상한 30 | `.cpp:791-800` · `:807-819` · `:1262-1267` | [A] · PIE ✅ |
| **조준 게이트** — 기록 반경 ≤ `TargetRadiusCm 45 × AimedHitTolerance 2`(= 90 cm, 사격 중 ×1.2) 이면: 지금 콘이 들어가면 `Aimed` / 정착 콘(이동 흔들림 포함)이 들어가면 **`Settling`** / 아니면 예비 있을 때 `Suppressive` | `.cpp:1278-1319` | [A] · PIE ✅ |
| **버스트** — `WantsToFire()` 뒤 `Now < NextBurstSeconds` → **`Pacing`**, 아니고 남은 발 0 → `BurstRoundsMin 2..Max 5` 딜(제압은 Max), 발마다 감산, 0 이 되면 `NextBurst = Now + BurstPauseSeconds 0.5 × (1 ± RhythmJitter 0.35)`; `FRandomStream Rhythm` 은 `GetTypeHash(Owner->GetFName())` 시드 | `.cpp:124` · `:791-800` · `:1336-1353` | [A] · PIE ✅ · 값 [C-160] |
| `[Engage]` 로그 꼬리 `cone %.2f wobble %.2f burst %d next %+.2f` | `.cpp:1629-1649` | [A] |
| `KnowledgeToSpreadRatio` **삭제** | grep 0건 | [A] |
| **섀도우 재캐스트 문턱** — `FLight::ShadowEye/ShadowCastTime`, `ShadowRecastMoveCm 0`(= 한 셀) ∧ `ShadowRecastSeconds 0.5`; 얼면 낡은 그림자는 즉시 재캐스트; 한 패스는 `ShadowEye` 에서 끝까지 | `SoldierSituationField.h:374-380` · `SoldierFieldSettings.h:247-260` · `.cpp:1591-1614`(`Refresh`) · `:1537-1550`(`FreezeLight`) · `:1767-1775` | ~~**[B] 빌드 전** · [W96]~~ → **PIE ✅ (10절, 줄 번호는 그쪽이 최신)** · 값 [C-161] |
| **동승(riders)** — 다른 스코프의 그림자 없는 선명 라이트가 반 셀 안이면 같은 트레이스로 각자의 필드에 씀; 틱을 넘는 패스는 커서·시각·`ShadowEye` 일치로 다시 모음 | `.cpp:1777-1834` · `:1869-1872` · `:1889-1909` · `:1913-1921` | ~~**[B] 빌드 전**~~ → **PIE ✅ (10절)** |
| **비용 줄** 헤더 3줄째 `[Field] cost/tick: shadows x ms (N rays/tick, N waiting of M alive, all squads)  bake  evict  |  overlay` · 넘침 경고 분대당 1회 | `.cpp:204-222` · `:287-298` · `:417-431` · `:529-537` | 비용 줄 **PIE ✅ 측정됨**(감축 전: shadows 0.03 ms · 0 waiting · **96 alive**) · 감축은 [B] |

---

## 1. 분대 스코프 상황 필드 [A · PIE ✅]

### 1.1 왜

[W74] 그대로: 진영 하나에 필드 하나면 **한 명이 본 목격을 무전 없이 전원이 그 즉시 안다** — 전지(omniscient). 분대 명령 층(09-17~18)이 `SquadId` 를 갖게 됐고 무전(`USoldierCommsComponent`)이 이미 접촉 보고 단위로 목격을 나르고 있으니, 필드를 **분대의 지식**으로 내리면 "우리 분대는 그걸 모른다"가 필드가 말할 수 있는 문장이 된다(`SoldierSituationField.h:93-97` · `:125-136`).

### 1.2 구조

- `FScope`(`.h:546-561`) — 진영 · `SquadId` · `bUsed` · `Levels`(밉) · `Lights` · `Anchors`(이 틱의 분대원 발) · `EvictionQueue[5]`. **분대 하나의 지식 전부**가 이 안에 있고, 둘 사이를 건너는 것은 무전뿐.
- `Scopes.SetNum(3 × PerFaction)`(`.cpp:678-690`) — 진영(Friendly/Hostile/Neutral) × `MaxSquadsPerFaction`(설정, 1~4, 기본 **3**). Neutral 몫은 만들어지지만 `ScopeFor` 가 −1 을 돌려줘 영영 안 쓴다. `BuiltSquadsPerFaction` 이 바뀌면 필드 전체를 비운다(셀 크기 변경과 같은 취급).
- `ScopeFor(Faction, SquadId)`(`.cpp:499-546`) — 그 진영의 슬롯 중 같은 `SquadId` 가 있으면 그것, 없으면 첫 빈 슬롯을 **지금** 이름 붙여 준다, 빈 슬롯이 없으면 **슬롯 0 을 공유**하고 `WarnedOverflowSquads` 로 **분대당 한 번만** 경고(`[Field] faction N has more squads than MaxSquadsPerFaction (3): squad 'X' shares the first field …` — 이 함수는 매 틱 병사마다 불리므로 호출당 경고면 로그가 잠긴다). 이름 없는 분대(`SquadId` None)는 "이름 없는 분대" 하나로 묶인다.
- `ScopeFor(Who)`(`.cpp:548-559`) — `Who == nullptr ∥ !Who->bTakesSquadOrders` → **−1**. titan 브리지가 UGV 에 표적용 Identity 를 다는데, UGV 가 있는 레벨에서는 그것이 **첫 번째로 말하는 Friendly "분대"** 라 슬롯을 하나 가져가고, 플레이어가 UGV 를 몰면 오버레이가 그 뒤를 따라갔다(주석 `:550-553`). 이제 스코프도 없고 앵커도 아니다(`Tick` `:161-166` — 단 `AllAnchors` 에는 들어가서 **호라이즌**은 UGV 주위에도 남는다 · `:166`).
- **공개 API 전부** `Who` 를 받는다(`.h:137-233`): `ReportSighting` · `MarkClear(AlongRay)` · `GetExposure(ByStance)` · `GetMostExposedDirection` · `GetStaleVantage` · `FindDarkestCells` · `GetPresence` · `GetWedgePresence` · `GetSecondsSinceObserved`. 스코프 −1 이면 쓰기는 무시, 읽기는 0 / false / `TNumericLimits<float>::Max()`(`GetSecondsSinceObserved`). 내부는 전부 `int32 Scope` 오버로드(`.h:412-428`).
- **호라이즌(기하)만 세계 공유** — `Horizons` · `CoarseHorizons` · `PendingBakes` · `AllAnchors` · `HorizonEvictionQueue` 는 스코프 밖(`.h:566-575`). 굽기 예산도 하나.
- **섀도우 예산은 세계에 하나**(`ShadowRaysPerTick 96`, `Tick` `:200-268`): 모든 스코프의 그림자 없는 선명 라이트 중 **가장 밝은 것, 같으면 따라가는 것 우선**, 단 **그 스코프의 앵커에서 `DetailRadiusCm` 안**인 것만(아무도 없는 땅에 그은 그림자는 퇴거가 도로 접는다). `LastShadowsPending` = 그 조건의 대기 수(비용 줄의 `waiting`).

### 1.3 무전이 필드로 들어오는 자리

`USoldierPerceptionComponent::ReceiveSharedRecord`(`SoldierPerception.cpp:400-434`): 받은 기록을 `SharedRadiusInflationCm` 만큼 부풀리고 `SharedCertaintyFactor` 로 깎아 `IntegrateRecord` 한 **뒤**, 받는 병사의 Identity 로 `Field->ReportSighting(OwnIdentity, Location, ObservedCertainty, ObservedRadiusCm, ObservedTimeSeconds)`. 시각은 **관측 시각**이라(무전이 걸린 만큼 과거) 새 라이트면 `BornTime/LastSeenTime` 이 과거로 찍혀 태어나면서 이미 낡아 있고, 같은 분대 안(보낸 사람의 분대 = 라이트가 이미 있음)이면 `Refresh` 가 `LastSeenTime = max(기존, 보고)`(`SoldierSituationField.cpp:1618-1620`)라 **자기 눈이 보고 있는 라이트의 시계를 되돌리지 않는다**. 즉 남의 분대의 목격은 "나중에, 흐리게, 무전이 닿았을 때만" 들어온다 — 무전 모델(말하는 시간 = 낡음)이 그대로 필드의 낡음이 된다.

### 1.4 오버레이

- `GetDebugScope()`(`.cpp:444-479`) — 관찰 중인 병사(`GetObservedSoldier`)의 진영과 **슬롯**을 따라간다(Tab 으로 다음 병사 → 그 분대의 필드). `Debug.Field.Faction` 만 고정하면 **같은 슬롯의 반대 진영**("우리 맞은편 적 분대는 뭘 아나"), `Debug.Field.Squad` 로 슬롯 고정(0..N−1 clamp). 그 스코프가 `bUsed` 아니면 −1 → `[Field] no field to show yet - follow a soldier, or pin Debug.Field.Faction/.Squad`.
- `Debug.Field.Centre`(`.cpp:65-71` · `:359-370`) — 0 = 관찰자(병사 없으면 카메라, 차량이면 차량) 주위 링, **1 = 그린 스코프의 `Anchors` 전원** 주위 링. UGV 에서 반 지도 밖의 분대를 볼 때 1. 헤더 끝에 `centred on the squad (N)`.
- `DescribeScope`(`.cpp:481-492`) → `HOSTILE/1` · `FRIENDLY/(no squad)`. 헤더 1줄 `[Field] HOSTILE/1  |  채널  |  lights …  |  cells …  |  horizons N  |  rings, drawn N/6000  (following|pinned)`. `(following)` 은 Faction·Squad 둘 다 −1 일 때만.
- `GetObservedSoldier`(`SoldierDebugDraw.cpp:57-124`) — 필터 이름 → 관전 폰 추적 → **조종 폰/뷰타겟 중 `bTakesSquadOrders` 인 것만**. UGV 조종은 "병사를 보고 있는 것"이 아니다.

### 1.5 레벨 — `L_SoldierScenario` 재편 (사용자, 09-21)

아군이 **4개 분대(5/5/5/5) → 3개(7/7/6)**: `Friendly_16`·`_17` → 분대 1, `_18`·`_19` → 2, `_20` → 3, `_16` 의 리더 플래그 해제. 적군 5/5/5 불변. `MaxSquadsPerFaction 3` 에 맞춘 것 — 넘치면 슬롯 0 공유(경고)라 4번째 분대는 1번 분대의 지식을 봤을 것이다. ⚠ **`DT_ScenarioSteps_SquadThreeStage` 가 아직 분대 '4' 를 참조할 수 있다** → [W94](분대 세션 몫, 이 세션은 DT 를 열지 않았다).

### 1.6 안 한 것

- 분대 사이 **자리 주장·엄호**(S1~S4)는 여전히 진영 단위(등록부) — 필드만 분대다.
- Neutral 필드 없음(그대로).
- 분대 `SquadId` 가 **바뀌는** 병사(`ReinforceSquads` 영구 편입)는 다음 틱부터 새 스코프의 앵커·쓰기가 된다 — 옛 분대의 필드에 그가 쓴 것은 그대로 남는다(옳다: 그 분대가 알았던 것이다).

---

## 2. 엣지 전진 — 파이 자르기를 그리지 않고 창발시키기 [A 코드 · PIE 대기]

### 2.1 무엇이 문제였나

09-18 저녁 C(코너 멈춤 + 굽이 너머 미리 보기, 같은 날 문서 7.3절)는 "멈춰서 본다" 반쪽이었고, 진짜 파이 자르기(모퉁이를 비스듬히 한 조각씩 여는 **경로 모양**)는 [W85] 로 남겼다. 그 뒤 잠깐 **손으로 그린 파이 호 + 머무름**(굽이 앞에서 바깥으로 비켜선 호를 따라 걷고 각 지점에서 잠시 멈춤)이 작업 트리에 있었다가 이 세션에서 **기각·삭제**됐다(코드에 흔적 없음 — `CornerStopCm`/`CornerPauseSeconds`/`UpdateCornerPause` 전부 소멸, grep 0건). 남은 것은 `FindNextCorner`/`CornerAngleDeg 35`/`CornerLookAheadCm 500` 뿐이고 **볼 곳 미리 보기**(`BeginSweep` 의 굽이 너머 편향)에만 쓰인다(`SoldierCover.h:272-277` · `.cpp:717-764`).

### 2.2 설계 논의 — 기각한 둘과 고른 것

사용자의 요구는 하나였다: **파이 자르기는 스크립트된 모양이 아니라 "정보 굶주림 대 위험"에서 나와야 한다.** 판정 기준:

- 팀원이 이미 본 땅(분대 필드에서 어두운 곳)은 **그냥 걸어 지나간다** — 의식(ritual)이 아니다.
- 넓은 방은 걸음이 **짧고**, 복도는 **길다** — 같은 각도에 열리는 면적이 다르니까.
- 호의 반경은 **예산에서** 나온다 — 누가 그리지 않는다.
- 새 레이 트레이스를 만들지 않는다 — 인지 광선과 구운 호라이즌을 **재사용**한다.
- 타이머 없음 — 걸음의 박자는 **인지의 박자**다.

| 안 | 무엇 | 왜 기각 |
|---|---|---|
| **(a) 손으로 그린 호 + 머무름** (잠깐 있었음) | 굽이 앞에서 바깥쪽으로 비켜선 호(반경·점 수 상수)를 중간 목적지로 끼우고 점마다 N 초 멈춤 | 반경·간격·머무름이 전부 **물리적 뜻 없는 상수**(09-18 밤에 코너 멈춤 0.8 s 가 유예 0.75 s 와 부딪혀 루프를 만든 것과 같은 종류). 팀원이 치운 복도에서도 같은 호를 그린다. 방과 복도가 같은 호. 굽이(경로 각 35° 초과)가 없는 **열린 방 입구**는 코너로 안 잡힌다 — 그런데 위험은 거기 있다 |
| **(b) 후보마다 광선 부채꼴** | 걸음 후보마다 그 자리에서 광선 부채꼴을 그어 "무엇이 보이게 되나"를 잰다 | 후보 7방향 × 4길이 × 부채꼴 = 걸음 결정 하나에 트레이스 수백. 그리고 **이미 있는 정보를 다시 재는 것** — 콘 스윕이 방금 그은 광선이 "눈이 어디서 멈추나"를 알고, 호라이즌 맵이 "저 셀이 여기를 볼 수 있나"를 구워 두었다. P177 |
| **(c) 채택 — 엣지 + 쐐기 적분** | 스윕 광선에서 **엣지**를 읽고, 걸음이 **눈-엣지 선을 얼마나 돌리나**로 열리는 쐐기를 정의, 그 쐐기 안의 **경계도 × 면적**을 필드에서 적분(호라이즌으로 가림) | 트레이스 0(전부 재사용). 어두운 땅은 0 을 열어 곧장 지나간다. 방은 각도당 면적이 커서 걸음이 짧아진다. 엣지에 가까울수록 같은 걸음이 선을 크게 돌려 많이 열리니 발이 **밀려나** 호가 된다 — 예산이 만드는 호. 걸음 뒤 "본다"는 열린 쐐기의 **글로우가 스윕에 의해 내려가는 것**이라 타이머가 아니다 |

### 2.3 재료 1 — 스윕 광선의 기억 (`SoldierSight`)

`FSoldierSweepRay{Eye, Dir, Stop, bBlocked, TimeSeconds}`(`SoldierSight.h:18-25`) 를 슬롯별로 보관(`SweepRays[Slot]`, `.cpp:186-196`), `GetSweepRays()`. 슬롯 순서 = 콘 왼쪽 끝부터 각도순(`ConeSweepRays 21` × `SightHalfAngleDeg 60` → 6° 간격, `ConeSweepTracesPerTick 2` 라운드로빈이라 한 바퀴 ≈ 10.5 틱). 이웃 슬롯은 **다른 틱**의 광선이므로 읽는 쪽이 신선도를 짝지어 본다. 트레이스는 **새로 안 는다** — 있던 콘 스윕이 결과를 남길 뿐.

### 2.4 재료 2 — 쐐기 적분 (`GetWedgePresence`, `SoldierSituationField.cpp:2473-2548`)

`Apex` 에서 `DirA`→`DirB`(짧은 쪽) 쐐기, `RangeCm` 까지:
- 광선 `ceil(span/4°)`(1..12) 갈래, 갈래마다 `ceil(Range/셀)`(1..48) 표본, 표본은 셀 하나 깊이 × `r·dθ` 폭의 조각을 대표(`Area = R × RayRad × Cell × 1e-4` m²).
- 표본마다 **그 셀의 호라이즌**(`HorizonFor(P)`)을 apex 를 **되돌아보는 bin** 으로 읽어, `bSolid` 면 그 갈래 끝, 트임 `Stand[bin] × 셀` 이 `R − 셀` 보다 짧으면(굽기 상한 근처 제외) **그 셀은 apex 를 못 본다** → 갈래 끝. 안 구운 셀은 **세고**(사전값) 큐에 넣는다 — 다음 물음에 답이 날카로워진다.
- 합 = Σ `GetPresence(Scope, P) × Area` — 경계도 × m². 라이트 글로우도 `GetPresence` 가 max 로 넣으니 목격이 있는 쐐기는 밝다.

두 가지 물음에 같은 함수: "이 걸음이 **무엇을 열까**"(Before→After) 와 "연 조각에 **아직 뭐가 남았나**"(같은 쐐기를 다시 — 콘 스윕이 `MarkClearAlongRay` 로 내리는 걸 읽는다).

### 2.5 기계 (`SoldierCover`)

**엣지 찾기** `FindViewEdges(Eye, Toward, Now, Out)`(`.cpp:768-844`): 이웃 광선 쌍(둘 다 `FreshSeconds 0.75` 안)에서 **한쪽이 막혀 `EdgeRangeCm 1000` 안에 멈추고 옆 광선이 `max(2×, +300 cm)` 이상 더 간** 곳. 엣지 apex = 가까운 광선의 멈춘 점과 먼 광선 선 위 그 투영의 중점(눈 높이). 발이 가는 쪽 ±(dot ≥ −0.2) 인 것만. 그리고 **숨은 쪽 10° 탐침 쐐기**를 `GetWedgePresence` 로 재서 `EdgeIgnorePresence 0.5` 미만이면 버림 — **팀원이 본 땅이면 여기서 이미 어둡다**, 그 엣지는 그냥 벽이다.

**걸음의 가격** `OpenedByStep(Here, There, Edges, Now, OutWedges)`(`.cpp:846-882`): 엣지마다 `Before = (Apex−Here)`, `After = (Apex−There)` 의 선회가 **숨은 쪽으로** 도는 경우만 `GetWedgePresence(Apex, Before, After, WedgeRangeCm 1500)` 를 더한다(반대로 돌면 벽을 더 끼우는 것 = 0). 연 쐐기 `FOpenedWedge{Apex, DirBefore, DirAfter, NearDir}` 를 남긴다.

**걸음 고르기** `IssueAdvanceStep`(`.cpp:988-1123`): 목표 방향에서 `{0, ±35, ±70, ±105}°` × `AdvanceStepCm 60 × {1..AdvanceStepCount 4}` — 내비 투영(40 cm 안), 뒤로 가는 것(진전 < −½걸음) 제외. 예산 `StepPresenceBudget 6`(**Cautious 명령이면 절반**). 예산 안에서 `Score = 진전 − 0.5 × 걸음 × (열림/예산)` 최대(같은 진전이면 덜 여는 쪽). 예산 안이 없으면 **1걸음 후보 중 가장 적게 여는 것**(진전 ≥ 0) — "볼 여유가 없어도 언젠가는 봐야 한다". `MoveToLocation(Point, 20 cm, 경로탐색, strafe)`, `AdvanceWedges` = 연 쐐기, `AdvanceRestSeconds = −1`(걷는 중), 발 자리를 `AdvanceRetreatPoints`(최대 8)에. 로그 `[Cover] X t=… ADVANCE step 60 cm to (x,y) opens 2.31 of budget 6.00 [(over budget, smallest)] edges N`.

**한 틱** `UpdateEdgeAdvance`(`.cpp:1125-1226`, `Tick` 첫 줄 `:2155`):
1. 전진 중 ∧ `SweepEyes > 0` → `BreakAdvanceForContact`(`:968-986`): **그 자리에서 정지**(`StopMovement`), `bMovingToCover=false`, 경로 리셋 — 다음 스윕이 **이 틱에** 결정한다. 09-18 에 "접촉이면 목적지로 계속"이 병사를 모퉁이 밖 공터로 내보냈다(주석 `:1132-1134`). 걸어온 자리들은 `AdvanceRetreatSeconds 10` 동안 **엄폐 후보**로 제공(`BuildCandidates` `:694-711`, 눈 있을 때만, 다른 후보와 똑같이 값 매김 — 명령이 아니라 제안).
2. 전진 아님: **눈 0 ∧ Rush 아님 ∧ 엄폐 이동 중 ∧ 경로 추종 `Moving`** 일 때만, `AdvanceCheckSeconds 0.1` 마다 `FindViewEdges`. 엣지 ≥ 1 → `AdvanceGoal = LastIssuedFoot`, **`StopMovement`**, `bAdvancing=true`, 쐐기 없음, `AdvanceRestSeconds = Now`(이미 보이는 것부터 본다). 로그 `ADVANCE begins: N edge(s) ahead, goal (x,y)`.
3. 걷는 중(`AdvanceRestSeconds < 0`) → 대기. 발이 멈추면 `Tick`(`:2159-2168`)이 `AdvanceRestSeconds = Now` 로 찍고 **도착 판정에서는 "아직 이동 중"** 으로 센다(한 전진 = 한 이동).
4. 보는 중: `AdvanceGlow = GlowInWedges()`; `쐐기 없음 ∥ glow ≤ AnalyzedPresence 1 ∥ 본 지 ≥ MaxLookSeconds 3` 이면 다음. 아니면 대기(**박자 = 스윕이 쐐기를 내리는 속도**).
5. 다음: 엣지 재탐색(`Toward` = 목표 방향) → 없거나 걸음을 못 내면 `EndAdvance`(`:945-966`): 원래 목적지로 `MoveToLocation` 재발행, `CurrentPath` 재보관(볼 곳 미리 보기용), `LastMoveIssuedSeconds = Now`(P174). 로그 `ADVANCE ends: N edge(s), on to goal`.

**스윕과의 관계**: `IsAdvanceLooking()`(멈춰서 보는 중) 은 `FinishSweep` 의 `bAlreadyGoing`(`:1985-1987`)에 들어가 **재결정을 막는다**. 새 엄폐 이동을 발행하면(`:2075-2078`) 전진 상태를 지운다 — 전진은 그 이동의 것.

**교전 연동**(`SoldierEngagement.cpp`):
- `GetAdvanceView(Eye, OutAim, OutLean)`(`SoldierCover.cpp:899-943`): 조준점 = 마지막 연 쐐기의 **이등분 방향 8 m**(쐐기 없으면 첫 엣지 방향), 린 = `AdvanceLean 0.7` × 벽(`NearDir`) 쪽 부호. 교전 무접촉 분기(`:1391-1403`)가 볼 곳 대신 이걸 쓰고 `DesiredLean` 을 넣는다 — 머리가 어깨보다 먼저 엣지를 넘는다.
- `bWantsToAim = bHasContact ∥ IsAdvancing()`(`:1532-1535`) — 걸음마다 총구가 새 조각 위에 있어야 하고 린은 총 든 어깨가 필요. ⚠ **`BP_SoldierCharacter` 는 아직 `HasContact()` 를 aim/strafe 모드에 먹인다** → 전진 중 몸이 진행 방향을 본다 → [W93]. (`SoldierScanTurnComponent` 는 `WantsToAim()` 게이트라 전진 중엔 안 돈다 — 발이 움직이니 무관.)
- 걸음: 전진 ∧ 무접촉이면 **Walk**(`bWantsToSprint=false`, `:1554-1561`) — 조각이 눈에 익기 전에 다음이 열리면 안 된다. 급박도 `UrgencyLookPeek 0.3`(`:1611-1615`).

**디버그**: `SoldierLab.Debug.Cover 1` 에 **시안 핀**(엣지, 지면 −120~+40 cm) + **시안 두 줄**(마지막 쐐기 apex 에서 `DirBefore` 얇게·`DirAfter` 굵게, 글로우 > `AnalyzedPresence` 면 밝고 아니면 어둡게, `.cpp:2321-2337`). `SoldierLab.Debug.Cover.Log 1` 에 `ADVANCE begins/step/ends` 세 줄.

### 2.6 왜 이게 파이 자르기인가 (창발의 산수, [B] 계산)

엣지에서 `d` 떨어진 발이 옆으로 `s` 옮기면 눈-엣지 선이 `≈ s/d` rad 돈다. 열리는 면적은 `≈ ½ × (선회각) × R²`(R = 쐐기 범위 1500 → 최대 112 m²/rad). 사전값 0.5 인 방에서 예산 6 (presence·m²) 는 **선회 ≈ 0.107 rad ≈ 6°** 어치. `d = 1 m` 이면 옆 걸음 10 cm 가 6° — 60 cm 걸음은 전부 예산 초과 → **가장 적게 여는 1걸음** 즉 엣지에서 **멀어지는** 쪽(정면 대신 ±105°까지 부채꼴이 있는 이유). `d = 3 m` 면 30 cm 가 6°. 발이 예산이 허락하는 거리까지 밀려나면 그때부터 정면 걸음이 예산 안이 되고, 걸음마다 선이 6° 씩 돌아 **조각이 열린다** — 그게 호다. 복도(쐐기 안 셀이 호라이즌에 잘려 면적이 작음)는 같은 6° 에 면적이 작아 걸음이 커지고, 팀원이 치운 방(경계도 ≈ 0)은 열림 0 → 240 cm 걸음이 최대 진전으로 뽑혀 곧장 간다. **정확한 숫자는 [C-158]** — 위는 왜 값들이 이 크기여야 하는지의 근거일 뿐.

### 2.7 상태

코드 [A] · 주석 날짜 09-18 · **CL 500 에 포함돼 빌드됨**(사용자) · **PIE 판정 미보고** → [W95]. 판정 재료: `[Cover] … ADVANCE begins … step … opens x of budget 6 … ends` 가 모퉁이/문간에서 나오는가 · 오버레이 시안 핀이 실제 모퉁이에 서는가(6° 광선 간격이라 apex 는 ±0.5 m 오차) · 팀원이 치운 복도에서는 `begins` 가 **안** 나오는가(`EdgeIgnorePresence 0.5`) · 접촉 시 그 자리 정지 뒤 스윕이 걸어온 자리 중 하나를 고르는가 · `MaxLookSeconds 3` 에 걸려 넘어가는 비율(자주면 `AnalyzedPresence` 가 너무 낮거나 스윕이 못 내리는 셀).

---

## 3. 사격 모델 — AI 가 콘을 소유한다 [A · PIE ✅]

### 3.1 왜

09-13 의 방아쇠 질문 "앎이 무기보다 나쁜가"(P66)에서 무기 쪽은 **소총의 고정 3°** 였다. 그 결과: 40 m 에서 완벽히 위치를 아는 표적에 2% 명중이라 조준 게이트가 무의미했고, 선회 직후·버스트 중·달리는 중이 **전부 같은 콘**이라 "멈춰서 쏜다"도 "겨누고 잠깐 기다린다"도 나올 수 없었다. 이제 **콘은 AI 가 계산해 발행**하고(`GetShotSpreadDegrees()`, `SoldierEngagement.h:352-362`) 무기는 그것으로 쏴야 한다(아직 안 읽는다 → [W93]: 그때까지 결정은 **탄이 따르지 않는 콘**에 대해 내려진다).

### 3.2 콘

```
SettledConeDeg = WeaponSpreadDegrees 0.8 × (1 + MovementSpreadScale 6 × min(1.5, v/ReferenceSpeedCms 600))
                                     × PostureScale(Open 1 · Lean 1.5 · Blind 15) × (1 + RecoilSpread)
ConeDeg        = min(89, SettledConeDeg + AimSettleDeg)                      ← GetShotSpreadDegrees (접촉 분기 .cpp:1221-1233)
무접촉         = WeaponSpreadDegrees + AimSettleDeg                            (.cpp:820-822)
```

재스케일 근거(헤더 주석 `.h:366-374` · `:628-639` · `:766-775`): 기준을 3° → **0.8°**(제대로 든 소총) 로 내리고 배율을 올려 **가치 게이트(`SuppressiveRadiusCm 500`)의 사거리 범위는 그대로** — 조깅 = `0.8 × 7 + 1.5 wobble ≈ 7.1°` → 500 cm 안은 ≈ 40 m(옛 2 × 3 과 같음), 스프린트 ≈ 28 m, 블라인드 `15 × 0.8 = 12°` → ≈ 24 m(옛 4 × 3 과 같음). 정지 상태에서는 가치 게이트가 거의 안 막고 **조준 허용치**가 결정한다.

### 3.3 흔들림 `AimSettleDeg` (`.cpp:785-819` · `:1262-1267`)

| 사건 | 효과 |
|---|---|
| 선회 중(`!bOnTarget`) | `AimSettleDeg = max(·, AimSettleInitialDeg 2.5)` — 선회가 남기는 흔들림, 첫 발이 기다리는 것 |
| 매 틱 | `×= exp(−dt / AimSettleSeconds 0.4)` — 대부분은 빨리, 마지막은 천천히(정착하는 조준경) |
| 발마다(탄창 감소로 감지) | `+= RecoilKickDeg 0.6` |
| 이동 중 | `= max(·, MoveWobbleDeg 1.5 × min(1.5, v/600))` — 발이 들고 있는 바닥, 기다려도 안 내려간다 |
| 상한 | 30 |

### 3.4 조준 게이트 (`.cpp:1278-1319`)

앞 게이트(믿음 · 가치 `SpreadCm ≤ 500 (×1.2 사격 중)` · 조리개 · `bOnTarget = aimErr ≤ ConeDeg × OnTargetConeRatio` · 재장전/탄약) 통과 뒤:

```
HitRadiusCm   = TargetRadiusCm 45 × AimedHitTolerance 2 (× WorthHysteresis 1.2 사격 중) = 90 (108)
RestSpreadCm  = dist × tan(SettledConeDeg + MoveWobbleNow)        ← 기다리면 닿는 콘 (이동 흔들림은 남는다)
RadiusCm ≤ HitRadius (표적 위치를 충분히 안다):
    SpreadCm ≤ HitRadius       → Aimed
    RestSpreadCm ≤ HitRadius   → Settling   (새 의도 — 정착하면 맞는다, 기다린다)
    예비 > SuppressiveReserve  → Suppressive (달리며·내밀며·너무 멀리: 맞진 않아도 가까이 간다, 가치 게이트가 보증)
아니면 RadiusCm ≤ 1000 ∧ 예비 → Suppressive
```

산수([B]): 90 cm 는 60 m 에서 0.86°. 정착 0.8° 가 겨우 들어가니 **정지 · Open 자세**에서만 60 m 조준사격이 있고, 선회 뒤 2.5° 흔들림이 0.06° 로 내려오려면 `0.4 × ln(2.5/0.06) ≈ 1.5 s`; 40 m 에서는 허용 1.29° → 흔들림 0.49° 까지 → ≈ 0.65 s; 10 m 에서는 허용 5.1° > 3.3° 라 **절대 기다리지 않는다**("가까우면 상관없다"가 산수에서 나온다). 린(×1.5 = 1.2°)은 60 m 조준 불가 → 제압. ⚠ 헤더 주석 "at sixty for most of a second"(`.h:114-115`) 는 위 계산보다 후하다 — 60 m 는 ≈ 1.5 s, "most of a second" 는 40 m 쪽 이야기다 [B].

### 3.5 버스트 (`.cpp:124` · `:791-800` · `:1336-1353`)

`WantsToFire()` 인 채로 마지막에: `Now < NextBurstSeconds` → **`Pacing`**(방아쇠 뗌). 아니고 `BurstRoundsLeft ≤ 0` → 딜: 제압사격 = `BurstRoundsMax 5`, 조준사격 = `Rhythm.RandRange(2, 5)`. 발마다 `--BurstRoundsLeft`, 0 이면 `NextBurstSeconds = Now + BurstPauseSeconds 0.5 × (1 + RhythmJitter 0.35 × U(−1,1))`. `Rhythm` 은 `BeginPlay` 에서 **`GetTypeHash(Owner->GetFName())`** 시드 → 병사마다 다른 박자, 같은 병사는 재현 가능. 정착(3.3)이 그 위에 얹혀 **사거리가 멀수록 버스트 사이가 길어진다**(마지막 버스트가 남긴 `+0.6 × N` 이 내려와야 다시 `Aimed`).

`[Engage]` 로그 꼬리(`:1629-1649`): `… cone %.2f wobble %.2f burst %d next %+.2f` — `cone` = 발행 콘(°), `wobble` = `AimSettleDeg`, `burst` = 남은 발, `next` = 다음 버스트까지(음수면 지났음). 의도 색: `Settling` 주황(255,160,60) · `Pacing` 붉은 회색(200,80,80)(`:1672-1673`).

### 3.6 열린 배선 (BP, [W93])

1. **무기 BP** 가 산포를 `GetShotSpreadDegrees()` 로 읽어야 한다 — 지금은 소총의 고정 콘. 그때까지 `Settling`·`Pacing` 은 **탄이 따르지 않는 콘**으로 내리는 결정(정착을 기다렸는데 탄은 3° 로 나감).
2. **`BP_SoldierCharacter`** 가 aim/strafe 모드에 `HasContact()` 대신 **`WantsToAim()`** 을 읽어야 한다(`.h:238-244` 주석) — 아니면 엣지 전진 중 몸이 진행 방향을 본다(2.5절).

---

## 4. 섀도우 수요 감축 + 비용 줄 [~~B 빌드 전 · 비용 줄만 PIE 측정~~ → 같은 날 빌드·PIE ✅, **10절**]

### 4.1 측정 (감축 전, `L_SoldierScenario`, 사용자)

헤더 3줄째 `[Field] cost/tick: shadows 0.03 ms (96 rays/tick, 0 light(s) waiting of 96 alive, all squads)  bake … evict … | overlay …`. **0.03 ms** 는 문제가 아니다. 문제는 **96 alive = 6 스코프 × `MaxLights 16`** — 양 진영 3분대 전부가 **상한에 붙어** 있다. 상한에 붙으면 새 목격이 **가장 어두운 라이트를 밀어낸다**(`ReportSighting` 3 `.cpp:1711-1735`, 밀려나는 따라가는 라이트는 얼리며 기억을 쓴다) → 오래된 얼린 목격이 먼저 사라지는 건 맞지만, 16 이 "한 분대가 한 번에 날카롭게 아는 것보다 많다"는 전제(`SoldierFieldSettings.h:186-192`)가 **15 v 20 에서 깨졌다** → [W92]. 감축 자체의 동기: 따라가는 라이트가 **반 셀 옮길 때마다** 96 트레이스를 다시 긋고, 같은 적을 본 분대 셋이 그걸 **세 번** 했다(주석 `.cpp:1591-1596`) — 지금 0 waiting 인 것은 30 fps 예산이 넉넉해서고, 예산이 모자라면 waiting 이 쌓인다.

### 4.2 코드 (작업 트리)

- `FLight::ShadowEye` · `ShadowCastTime`(`.h:374-380`) — **지금 셀에 있는 그림자**가 어디서 언제 그어졌나. `Eye` 는 목격마다 움직이지만 그림자는 아니다.
- `Refresh`(`.cpp:1597-1614`): `Eye` 는 반 셀 넘게 움직였을 때만 갱신(그대로). 재캐스트(`++Generation`, `bShadowValid=false`)는 `bShadowValid ∧ DistXY(Eye, ShadowEye) > ShadowRecastMoveCm(0 → 한 셀 = 200) ∧ Now − ShadowCastTime ≥ ShadowRecastSeconds 0.5` 일 때만. 그 사이 셀은 옛 세대 = **그림자가 라이트보다 조금 뒤처진다** — 보이는 적은 어차피 엄폐 스윕이 실제 눈으로 재고, 필드의 그림자는 **눈이 떠난 뒤**의 것.
- `FreezeLight`(`.cpp:1537-1550`): 얼 때 `ShadowEye` 가 `Eye` 에서 반 셀 넘게 떨어져 있으면 **즉시** 재캐스트 — 여기서부터 그림자가 라이트의 전부다.
- `CastShadow`(`.cpp:1753-1924`): 커서 0 에서 `ShadowEye = Eye, ShadowCastTime = Now` 를 찍고 **패스 끝까지 `From = ShadowEye`**(틱을 넘어도 시작점 고정 — 옛 코드는 매 틱 `Light.Eye` 라 움직이는 적의 그림자가 방향마다 다른 눈에서 그어졌다).
- **동승(riders)**(`:1777-1834`): 패스 시작 시 **다른 스코프**의 alive · 그림자 없음 · 커서 0 · 선명 · `From` 에서 반 셀 안인 라이트를 모아 `ShadowEye/CastTime` 을 같이 찍는다. 틱을 넘는 패스는 `ShadowCursor == 우리 커서 ∧ ShadowCastTime 동일 ∧ ShadowEye 일치` 로 다시 모은다(패스 도중 새로 온 라이트는 안 탄다 — 다음 패스). 방향마다 rider 전원의 커서를 전진(`:1869-1872`), 셀마다 rider 마다 `TouchCell(rider 스코프) + LightCell(rider 스코프, rider 인덱스)`(`:1889-1909` — **트레이스는 한 번, 쓰기는 필드마다**), 끝나면 전원 `bShadowValid`(`:1913-1921`). 같은 스코프 안의 이웃 라이트는 안 탄다(그건 `MergeRadiusCm 300` 의 일).
- **비용 줄**(`:287-298` · `:417-431`): 패스별 ms 를 `Lerp(·, ·, 0.1)` 로 평활, `waiting` = `LastShadowsPending`(선명 ∧ 앵커 근처 ∧ 그림자 없음), `alive` = 모든 스코프 합. `waiting` 이 0 위에 머물면 예산이 싸움에 뒤처지는 것이고 ms 가 예산을 올릴 값이다.
- 넘침 경고 분대당 1회(1.2절).

### 4.3 판정 → [W96] · [C-161]

빌드 뒤 같은 장면에서: 비용 줄의 `shadows` ms 와 `waiting` 이 감축 전과 같거나 낮은가 · 오버레이에서 **움직이는 적의 붉은 그림자가 한 셀(2 m)·0.5 s 단위로 끊겨 따라오는 것**이 보이는가(설계 — 거슬리면 `ShadowRecastSeconds` 를 내린다, `MoveCm` 은 셀 아래로 내려도 뜻 없음) · 적이 사라진 순간(얼림) 그림자가 **즉시** 마지막 자리로 오는가(안 오면 `FreezeLight` 조건) · 두 분대가 같은 적을 볼 때 흰 점(그림자 미완)이 **동시에** 꺼지는가(riders) · `Debug.Field.Squad` 로 두 스코프를 번갈아 봤을 때 같은 자리에 같은 그림자인가.

---

## 5. 검증 상태

| 무엇 | 상태 |
|---|---|
| 분대 스코프 · cvar · `bTakesSquadOrders` 제외 · 무전 → 필드 | ✅ 빌드·PIE(CL 498/500) — 오버레이가 병사의 분대를 따라가고 UGV 를 중심으로 잡지 않음. **숫자·거동 판정은 [C-157]**(남의 분대 목격이 무전 뒤에만·흐리게 들어오는가, 넘침 경고) |
| `L_SoldierScenario` 아군 3분대 재편 | ✅ 사용자 (DT 의 분대 '4' 참조 미확인 [W94]) — **11절에서 재확인**(MCP 로 저장됨) |
| 엣지 전진 (`GetWedgePresence` · `GetSweepRays` · `SoldierCover` 전진) | **빌드됨(CL 500) · PIE 미보고** → [W95] · 값 [C-158] |
| 사격 콘 · 흔들림 · `Settling` · 버스트 · `Pacing` · 로그 꼬리 | ✅ 빌드·PIE — 거동 확인, 숫자 [C-159]·[C-160]. **무기 BP 미배선**이라 탄은 옛 콘 [W93] |
| 섀도우 재캐스트 문턱 · riders | ~~**[B] 빌드 전** → [W96]~~ → **✅ 빌드·PIE(같은 날 늦게, 사용자 "아주 잘됨") — [W96] 해결, 10절**. 숫자 판정은 [C-161] 그대로 |
| 비용 줄 | ✅ PIE 측정(0.03 ms · 0 waiting · 96 alive — 감축 **전** 판) → **감축 뒤 재측정도 0.03 ms · 0 waiting · 96 alive**(10절) |
| 오버레이 노출 보정(`SoldierDebug::Bright` · `USoldierDebugMeshComponent`) | ✅ 빌드·PIE — **별도 문서** `ai/2026-09-21_debug_overlay_exposure.md` |
| `MaxLights 16` 상한 도달 | ⚠ 측정으로 드러남 → [W92] |

---

## 6. 값 — 신설·변경 (전부 [C])

```
USoldierFieldSettings   Resolution  MaxSquadsPerFaction 3 (신설, 1~4)                                                                 → [C-157]
                        Shadow      ShadowRecastMoveCm 0 (= 한 셀) · ShadowRecastSeconds 0.5 (신설, 빌드 전)                             → [C-161]
                        (상수)      riders 동승 반경 = 반 셀 · 얼림 시 재캐스트 문턱 = 반 셀 · 비용 평활 Lerp 0.1
cvar                    SoldierLab.Debug.Field.Squad -1 · SoldierLab.Debug.Field.Centre 0   (신설)

USoldierCoverComponent  [Advance]   bEdgeAdvance true · StepPresenceBudget 6 (Cautious ×0.5) · AnalyzedPresence 1 · MaxLookSeconds 3 ·
                                    EdgeRangeCm 1000 · WedgeRangeCm 1500 · EdgeIgnorePresence 0.5 · AdvanceStepCm 60 · AdvanceStepCount 4 ·
                                    AdvanceLean 0.7 · AdvanceCheckSeconds 0.1 · AdvanceRetreatSeconds 10                                   → [C-158]
                        (상수)      FreshSeconds 0.75 (이웃 광선 짝짓기) · 엣지 조건 far ≥ max(2×near, near+300) · 탐침 쐐기 10° ·
                                    부채꼴 {0, ±35, ±70, ±105}° · 뒤로 = 진전 < −½걸음 · 조준점 apex + 8 m · 후퇴 자리 최대 8 · 내비 투영 40 cm
                        [Corners]   CornerAngleDeg 35 · CornerLookAheadCm 500 (유지 — 볼 곳 미리 보기만) · ~~CornerStopCm 150 · CornerPauseSeconds 0.8~~ 삭제
USoldierSituationField  GetWedgePresence 상수: 광선 4°/갈래(1..12) · 표본 셀 하나(1..48) · 가림 = Stand[bin]×셀 < R−셀 (굽기 상한 근처 제외)
USoldierSightComponent  ConeSweepRays 21 · ConeSweepTracesPerTick 2 · SightHalfAngleDeg 60 (기존 — 엣지 해상도 6° 를 정한다)

USoldierEngagementComponent  WeaponSpreadDegrees 0.8 (← 3) · MovementSpreadScale 6 (← 2) · ReferenceSpeedCms 600 (기존) ·
                             [Posture]  LeanSpreadScale 1.5 (← 1.4) · BlindSpreadScale 15 (← 4)
                             [Accuracy] AimSettleInitialDeg 2.5 · AimSettleSeconds 0.4 · RecoilKickDeg 0.6 · MoveWobbleDeg 1.5 ·
                                        TargetRadiusCm 45 · AimedHitTolerance 2  (신설)                                                  → [C-159]
                             [Rhythm]   BurstRoundsMin 2 · BurstRoundsMax 5 · BurstPauseSeconds 0.5 · RhythmJitter 0.35 (0..0.9) (신설)      → [C-160]
                             (상수)     흔들림 상한 30° · 이동 배율 min(1.5, v/600) · Rhythm 시드 = GetTypeHash(이름)
                             ~~KnowledgeToSpreadRatio~~ 삭제 · SuppressiveRadiusCm 500 · WorthHysteresis 1.2 · OnTargetConeRatio 1.0 (기존, 같이 쓰임)
```

---

## 7. 변경 파일 (이 묶음)

```
Source/SoldierLab/AI/SoldierSituationField.h/.cpp   FScope · ScopeFor ×2 · 공개 API 전부 Who · Tick 스코프 앵커/AllAnchors · 세계 섀도우 예산 · GetWedgePresence ·
                                                     GetDebugScope(Faction/Squad) · DescribeScope · Debug.Field.Squad/.Centre · 헤더 3줄(비용 줄) ·
                                                     [B] FLight::ShadowEye/ShadowCastTime · Refresh 재캐스트 문턱 · FreezeLight 즉시 재캐스트 · CastShadow riders ·
                                                     WarnedOverflowSquads                                                        (2501 → 3036줄)
Source/SoldierLab/AI/SoldierFieldSettings.h          MaxSquadsPerFaction · [B] ShadowRecastMoveCm · ShadowRecastSeconds · ShadowRaysPerTick 주석(세계 예산)
Source/SoldierLab/AI/SoldierSight.h/.cpp             FSoldierSweepRay · SweepRays · GetSweepRays
Source/SoldierLab/AI/SoldierCover.h/.cpp             엣지 전진 전부(2.5절) · 코너 멈춤 삭제 · BuildCandidates 후퇴 후보 · Tick/FinishSweep 연동 · 오버레이 시안  (1931 → 2374줄)
Source/SoldierLab/AI/SoldierEngagement.h/.cpp        ESoldierFireIntent Settling/Pacing · GetShotSpreadDegrees · 콘 재스케일 · AimSettleDeg · 조준 게이트 · 버스트/Rhythm ·
                                                     GetAdvanceView 연동 · bWantsToAim · Walk · UrgencyLookPeek · 로그 꼬리 · KnowledgeToSpreadRatio 삭제  (1586 → 1712줄)
Source/SoldierLab/AI/SoldierPerception.cpp           ReceiveSharedRecord → Field->ReportSighting(OwnIdentity, …, ObservedTimeSeconds)
Source/SoldierLab/AI/SoldierDebugDraw.cpp            GetObservedSoldier IsSoldier(bTakesSquadOrders)
Content/SoldierLab/Levels/L_SoldierScenario          아군 분대 4 → 3 (사용자, 에디터)
```

Perforce: CL 498 · 500 제출(사용자 보고, 내용은 미확인) + ④ 작업 트리. 빌드는 사용자가(P13/메모리).

---

## 8. 남은 것

| # | 무엇 | ID |
|---|---|---|
| 엣지 전진 PIE 판정(2.7절 재료) | 새 | **[W95]** · [C-158] |
| ~~섀도우 감축 빌드 + 판정(4.3절)~~ — 같은 날 빌드·PIE ✅(10절), 숫자만 남음 | 해결 | ~~[W96]~~ · [C-161] |
| `Squad/` · `Pose/` · `Weapons/` 오버레이를 `SoldierDebug::*` 로(별도 문서 4.2절) | 새 · 다른 세션 | **[W97]** |
| `MaxLights 16` 상한 도달(6 × 16 = 96 alive) — 상한을 올릴지, 얼린 라이트를 더 빨리 놓을지(`LightHalfLifeSeconds 20`·`FrozenPresence`), 분대별로 달리 둘지 | 새 | **[W92]** |
| BP 배선 둘 — 무기 산포 = `GetShotSpreadDegrees()` · `BP_SoldierCharacter` aim 모드 = `WantsToAim()` | 새 | **[W93]** |
| `DT_ScenarioSteps_SquadThreeStage` 의 분대 '4' 참조(아군 3분대 재편 뒤) | 새 · 분대 세션 | **[W94]** |
| 분대 필드 거동·넘침 판정 | 새 | [C-157] |
| 콘·흔들림·정착 숫자 · 버스트 박자 숫자 | 새 | [C-159] · [C-160] |
| ~~분대 스코프 필드~~ | 해결 | ~~[W74]~~ |
| ~~진짜 파이 자르기(경로 모양)~~ — 경로를 휘는 대신 **걸음을 값 매기는** 형태로 | 해결 | ~~[W85]~~ |
| 얇은 엄폐 후속 · 원기둥/나무 Sight 채널 | 그대로 | [W86]~[W88] |

---

## 9. 브리핑과 코드가 어긋난 자리 (문서 세션 확인)

- 코드 주석 날짜: ①②③ **2026-09-18**, ④ **2026-09-21**. 이 문서 날짜는 세션 종료일(09-21).
- 헤더 문서 주석의 예 `"HOSTILE/Alpha"`(`SoldierSituationField.h:251`)는 형식 예시고 실제 시나리오는 `HOSTILE/1`(SquadId 가 숫자 이름).
- `Settling` 설명 "at sixty for most of a second"(`SoldierEngagement.h:114-115`)는 계산(3.4절)보다 후하다 — 60 m ≈ 1.5 s.
- 엣지 전진의 접촉 후퇴 후보는 `SweepEyes > 0` 일 때만 후보에 들어가고(눈 없이 끝난 전진은 제안하지 않음) 최대 8자리 — 브리핑의 "steps taken offered as cover candidates" 의 조건.
- `bTakesSquadOrders=false` Identity 는 스코프 앵커에서는 빠지지만 **`AllAnchors`(호라이즌 유지)에는 들어간다** — UGV 주위 기하는 굽힌 채 남는다(브리핑은 "not anchors" 라고만 했다).
- 잠깐 있었던 "손으로 그린 파이 호" 는 어떤 문서에도 기록된 적 없고(09-18 문서는 "안 했다"), 코드에도 흔적이 없다 — 2.2절 (a) 는 사용자 서술을 옮긴 것이다.
- CL 498/500 의 내용 경계(어느 파일이 어느 CL 인가)는 이 세션이 p4 로 확인하지 않았다.

---

## 10. ④ 섀도우 감축 — 빌드·PIE 확인 (같은 날 늦게, [W96] 해결) [A · PIE ✅]

4절의 [B] 가 **정식 빌드**(헤더 UPROPERTY 추가라 Live Coding 아님)를 거쳐 PIE 에서 확인됐다 — 사용자 "아주 잘됨". 코드는 4.2절 서술 그대로이고 줄 번호만 옮겨 갔다(오버레이 노출 보정이 같은 파일 앞쪽에 들어가서 — `SoldierSituationField.cpp` 3036 → **3081줄**): `FLight::ShadowEye/ShadowCastTime` `.h:374-380` · `Refresh` 재캐스트 문턱 `.cpp:1625-1648`(`RecastMove = ShadowRecastMoveCm > 0 ? … : BuiltCellSizeCm`, `bShadowValid ∧ DistXY(Eye, ShadowEye) > RecastMove ∧ Now − ShadowCastTime ≥ ShadowRecastSeconds`) · `FreezeLight` 반 셀 넘게 낡은 그림자 즉시 재캐스트 `.cpp:1577-1584` · `CastShadow` 커서 0 에서 `ShadowEye = Eye, ShadowCastTime = Now`, 패스 끝까지 `From = ShadowEye` `.cpp:1801-1808` · riders 수집(커서 0: 다른 스코프 · alive · 그림자 없음 · 커서 0 · 선명 · `From` 반 셀 안 → `ShadowEye/CastTime` 같이 찍음 / 틱 넘는 패스: 커서 · `ShadowCastTime` 동일 · `ShadowEye` 일치) `.cpp:1811-1868` · 방향마다 rider 전원 커서 전진 `.cpp:1903-1906` · 셀마다 rider 마다 `TouchCell` + `LightCell` `.cpp:1923-1943` · 끝나면 전원 `bShadowValid` `.cpp:1947-1955`. 넘침 경고 분대당 1회 `WarnedOverflowSquads`(`.h:591` · `.cpp:564-566`). 비용 줄 `.cpp:451-465`.

**측정(감축 뒤, `L_SoldierScenario`, 사용자)**: 헤더 3줄째 `[Field] cost/tick: shadows 0.03 ms (96 rays/tick, 0 light(s) waiting of 96 alive, all squads)` — **감축 전과 같은 숫자**. 뜻: 이 장면은 30 fps 예산에서 원래 `waiting 0` 이었으므로 ms 로는 차이가 안 보인다(4.1절이 예고한 대로 — 감축의 효과는 예산이 모자라는 장면에서 `waiting` 이 안 쌓이는 것으로 나타난다, [C-161] 그대로). ⚠ 사용자가 처음에 **"96" 을 waiting 으로 읽었다** — 96 은 `alive`(모든 분대의 살아 있는 라이트 합 = 6 스코프 × `MaxLights 16` **상한**, [W92] 그대로 열림)이고 waiting 은 **0** 이다. 헤더 문구가 `%d light(s) waiting of %d alive` 라 앞 숫자가 waiting, 뒤가 alive.

4.3절의 나머지 판정(그림자가 2 m/0.5 s 단위로 끊겨 따라오는 것이 거슬리는가 · 얼림 즉시 재캐스트 · 두 분대 흰 점 동시 소등 · 두 스코프 같은 그림자)은 사용자 총평 "아주 잘됨" 에 묻혀 **개별 보고는 없다** — [C-161] 은 "눈에 띄면" 으로 남긴다.

---

## 11. `L_SoldierScenario` 아군 3분대 재편 — 확인

1.5절 그대로: 아군 4개 분대(5/5/5/5) → **3개(7/7/6)**, `Friendly_16`·`_17` → 분대 1, `_18`·`_19` → 2, `_20` → 3, `_16` 리더 플래그 해제 — **MCP 로 쓰고 저장됨**(사용자, 09-21). 적군 5/5/5 불변. `DT_ScenarioSteps_SquadThreeStage` 의 분대 '4' 참조는 여전히 미확인 → [W94] 그대로(분대 세션 몫).

---

## 12. 같은 세션 후반 — 오버레이 노출 보정 (별도 문서)

EV10 고정 노출 레벨에서 **모든 오버레이가 숯검정**으로 나온 것을, 레벨 라이팅을 건드리지 않고 **뷰가 적용한 노출의 역수만큼 밝게 그려서** 해결했다 — `SoldierDebug::GetExposureScale/Bright` · 래퍼 `SoldierDebug::Line/Point/Sphere/Circle` · 새 `USoldierDebugMeshComponent`(이 문서의 필드 오버레이 사각형·라이트 링이 라인 배처의 `DrawMesh` 대신 이걸 쓴다) · cvar `SoldierLab.Debug.ExposureScale`. `AI/` 의 `DrawDebug*` 24곳 교체, `Squad/`·`Pose/`·`Weapons/` 는 다른 세션 몫 [W97]. **→ `ai/2026-09-21_debug_overlay_exposure.md`**, 원칙 **P181**. 그 문서 3.5절이 이 문서 1.4절의 오버레이가 지금 어떻게 그려지는가의 최신이다.
