# 성능 계측(`stat SoldierLab`)과 엄폐 틱 비용 — 35명에서 프레임의 1/3 이던 Cover 8.9 → 1.9 ms

2026-09-21 / 완료 (빌드·PIE 실측 — 사용자 `stat` 캡처, 이 층의 몫은 끝났고 나머지는 다른 층 [W98]~[W102]) / 빈 레벨 1.7 ms 이던 World Tick 이 병사 35명에서 29 ms. `stat game` 은 그 중 6 ms 만 이름을 댈 수 있었다 → **`STATGROUP_SoldierLab`** 을 심고 시스템별 off 스위치로 A/B 한 뒤, 계측이 이름을 댄 것(**Cover Tick 8.92 ms · 487 트레이스/프레임**)만 고쳤다 — 전부 **결정 보존**(같은 자리를 고른다). 결과 Cover 1.92 ms · 243 트레이스, World Tick 28.5 → 22.6 ms. 남은 게임 스레드는 애니메이션(≈ 7.3) · 이동/트랜스폼(≈ 4~7) · 캐릭터 BP 틱(≈ 2.5) · 투사체 스폰(0.65) — 소유자별로 [W] 로 넘김.

전편: `ai/2026-09-21_per_squad_field_edge_advance_fire_model.md`(같은 날 — 섀도우 비용 줄 `[Field] cost/tick` 이 이 층의 첫 계측이었다, P180) · `ai/2026-09-18_patrol_scan_and_move_robustness.md` 3.1절(`MoveGraceSeconds` · `StallSpeedCms` — 이번 "이동 중" 판정이 그대로 쓴다) · `ai/2026-09-13_engagement_and_cover.md`(스윕 구조 — 후보 한 개 / 틱, 예산 `MaxTracesPerTick`).
원칙: **신설 P182 · P183**(`CLAUDE.md` 5절) · P10(계측 먼저) · P114(예산은 허가가 아니다) · P180(트레이스는 변화에). 값 **[C-162]**, 작업 **[W98]~[W103]**.

> 신뢰도: **[A]** 코드로 확인(file:line, 2026-09-21 작업 트리 — `AI/SoldierLabLog.h` 60줄 / `.cpp` 34줄 · `AI/SoldierCover.h` 999줄 / `.cpp` 2491줄) · 숫자는 전부 **사용자 PIE 캡처**(`stat game` / `stat SoldierLab` / `stat anim` / `stat PoseSearch` 스크린샷, **InclusiveAvg**, 별도 표기 없으면). 캡처 조건: `L_SoldierScenario`, 병사 35(적 15 / 아군 20), UGV·트럭·UAV 있음, PCG 랜드스케이프 없음, **PIE**, "PROFILING WITH AI LOGGING ON" 배너 있음(= 로깅 켜진 채 — 절대값은 Standalone 보다 높다, 8절). 캡처 사이의 장면(교전 시점·카메라)이 완전히 같지는 않아 **±0.3 ms 는 잡음**으로 읽을 것.

---

## 0. 한 장 요약

| 무엇 | 어디 | 상태 |
|---|---|---|
| **`STATGROUP_SoldierLab`**(`stat SoldierLab`) — 시스템 틱마다 사이클 카운터(Sight · Perception · **Cover + 하위 7**(Here / Begin Sweep / Candidate / Route / Score / Finish (path) / Edge Advance) · Engagement · Suppression · Comms · Health · Objective · **Field 5**(Tick / Shadows / Bake / Evict / Overlay / Wedge Reads)) + DWORD 카운터 **Traces: Sight / Cover / Engagement / Field** · **Soldiers Ticked** | `SoldierLabLog.h:17-59` · `.cpp:7-34` | [A] · PIE ✅ |
| **off 스위치** `SoldierLab.Sight.Enabled` · `.Perception.Enabled` · `.Cover.Enabled` · `.Engagement.Enabled` · `.Suppression.Enabled` · `.Comms.Enabled` · `.Field.Enabled`(각 1) — 그 시스템의 틱을 첫 줄에서 반환. **`SoldierLab.Cover.Avoidance`**(1) — 전 병사 CMC 의 RVO 를 런타임에 켜고 끔(다음 틱 적용) | `SoldierSight.cpp:16-20` · `SoldierPerception.cpp:14-18` · `SoldierCover.cpp:27-38` · `SoldierEngagement.cpp:18-22` · `SoldierSuppression.cpp:15-19` · `SoldierComms.cpp:14-18` · `SoldierSituationField.cpp:17-22` · 적용 `SoldierCover.cpp:2181-2189` | [A] |
| **첫 계측** — Cover Tick **8.92 ms** incl(6.86 excl, 34 calls) · Traces: Cover **487**/프레임(max 624) · Sight 0.96(108 트레이스) · Engagement 0.35(57) · Field 0.08 · 나머지 ≈ 0 → **Cover 혼자 프레임의 1/3** | 3절 | 실측 |
| **Cover 수정 넷**(전부 결정 보존, 4절) — (a) 이동 중 스윕 **정지**(`bSweepSuspended`; 어차피 `FinishSweep` 이 버리던 스윕) + 볼 곳만 `UpdateWatchPoint()` 로 `WatchRefreshSeconds 0.25` 마다 · (b) 발밑(HERE) 재평가를 매 틱에서 **`HereEvalIntervalSeconds 0.1` ∨ 걸음 > `MicroStepCm` ∨ 새 스윕** 으로 · (c) **경로 가지치기** — 경로 비용 없이도 best 에 지는 후보는 경로 트레이스를 안 긋는다(경로 비용 ≥ 0 이라 정확) · (d) 눈 0 스윕은 틱당 **`CalmCandidatesPerTick 2`** 후보만 | `SoldierCover.cpp:2229-2244` · `:2257-2285` · `:2287-2294` · `:2345-2381` · `:2311-2316` · `.h:548-563` · `:984-995` | [A] · PIE ✅ |
| **결과** — Cover **1.92 ms**(Candidate 1.29 / Score 0.51 / Begin Sweep 0.43 / Route 0.35 / Here 0.15 / Finish 0.03 / Edge Advance 0.02) · Traces: Cover **243** · SoldierLab 합 ≈ **3.3 ms** · World Tick **28.5 → 22.6 ms** · 프레임 31 ms · GPU 6 ms(**게임 스레드 바운드**) | 5절 | 실측 |
| **RVO A/B** — 정상 상태 비용 0.3~0.5 ms 로 판정, **켜 둔다**. "켬" 캡처에 **58 ms 히치**(`DispatchBlockingHit` 1회) 가 섞여 있었다 → 별건, 무기 BP `OnHit` 의 동기 로드 의심 → [W101] | 6절 | 실측 · [B] 원인 |
| **소유자별 인계** — 포즈(ABP 스레드-세이프 · 키네마틱 본 · URO, ≈ 5.5 ms 어치) [W98] · 캐릭터 BP(컴포넌트 23개 · 오버랩 · 틱 로직 ≈ 2.5 ms) [W99] · 무기(투사체 풀링 0.65 ms) [W100] · 히치 [W101] · SoldierLab 자체 다음 몫(Sight 표적 트레이스 간격 · Cover 후보 트레이스 박자 · 틱 함수 솎기 ≈ 1 ms) [W102] · 분대 세션 `STAT_SoldierLab_Squad/Zone` 스코프 [W103] | 7절 | — |
| **측정 규약**(로깅 off · 오버레이 off · 같은 카메라 · 같은 교전 시점 · Standalone · **세 갈래 확인** — 자기 스코프 / `.Enabled 0` 의 차 / 병사 수 스케일링) | 8절 · **P182** | — |

---

## 1. 기준선 — 무엇이 문제였나 [실측]

빈 레벨(병사 0): **World Tick 1.71 ms**, 60 fps. 병사 35 명을 넣으면:

| `stat game` 항목 | 값 | 비고 |
|---|---|---|
| World Tick | **29.2 ms**(max 35.6) | fps 20~30 |
| Blueprint Time | 6.7 ms(386 calls) | ABP 의 `BlueprintUpdateAnimation` 을 **포함**한 값 — 캐릭터 BP 몫만 따로는 못 읽는다 |
| Char Movement | 1.4 | |
| Transform or RenderData | 0.82(752 calls) | 752 / 35 ≈ **병사당 부착 컴포넌트 21~23개** 의 트랜스폼 갱신 |
| Ticks Queued | **968**(빈 레벨 82) | (968 − 82) / 35 ≈ **병사당 틱 함수 25개** |
| SpawnActor + SpawnTime | 0.7 | 투사체 1 발 스폰 |
| 배너 | "PROFILING WITH AI LOGGING ON" | 로깅이 켜진 채 잰 값 |

여기서 `stat game` 이 **이름을 댈 수 있는 것은 ≈ 6 ms**(Blueprint · CharMovement · Transform · Spawn). 나머지 20 ms 는 "컴포넌트 틱" 안에 있었고 SoldierLab 의 11쌍 컴포넌트에는 카운터가 하나도 없었다.

**사용자의 지시 — "무식하게 바로 하지 말고" 계획부터.** 채택한 순서: ① 계측을 심는다 ② 의심 대상을 A/B 로 확인한다 ③ **계측이 ms 로 이름을 댄 것만** 고친다, 전후 stat 줄을 붙여서. 추측으로 고친 것은 하나도 없다(P10 · P182).

---

## 2. Phase 0 — 계측 [A]

### 2.1 `STATGROUP_SoldierLab`

`AI/SoldierLabLog.h:28`(`DECLARE_STATS_GROUP`, `STATCAT_Advanced`) — `stat SoldierLab` 으로 켠다. 헤더 주석(`:17-27`) 이 이유를 적어 뒀다: "Nothing gets optimised on a guess: a system is touched when this group says it costs, and the line before and after is the proof."

| 스코프 | 어디서 재나 | 비고 |
|---|---|---|
| `Sight Tick` | `USoldierSightComponent::TickComponent` 전체(`SoldierSight.cpp:217`) | **`Soldiers Ticked` 도 여기서 센다**(`:218`) — `Enabled`·권한 검사 **앞**이라 "시야 컴포넌트가 틱한 액터 수"지 "판단한 병사 수"가 아니다(클라 사본·꺼진 상태도 센다) |
| `Perception Tick` | `SoldierPerception.cpp:696` | |
| `Cover Tick` | `SoldierCover.cpp:2162` — 틱 전체 | 아래 7 하위 스코프는 **이 안에 중첩**(inclusive). `Score` 는 `Here` 안과 `Candidate` 안 양쪽에서, `Route` 는 `Candidate` 안에서 열린다 → **하위 합 > Cover Tick 이 정상**(5절 표 참고) |
| `  Cover: Here` | 발밑 `EvaluatePosition` + `ScorePosition`(`:2264`) | |
| `  Cover: Begin Sweep` | `BeginSweep()`(눈 수집 · 볼 곳 · 후보 생성 · 부채꼴 트레이스)(`:2246` · `:2395`) + 이동 중 `UpdateWatchPoint()`(`:2291`) | 부채꼴은 `MaxTracesPerTick` 예산에 안 잡히므로 트레이스 수는 `Traces: Cover` 로만 보인다 |
| `  Cover: Candidate` | 후보 한 개의 스냅 + `EvaluatePosition` + 점수 + 경로(`:2322`) | Score · Route 를 **포함** |
| `  Cover: Route` | `EvaluateRoute`(`:2362`) | |
| `  Cover: Score` | `ScorePosition`(필드 읽기 · `GetStaleVantage` 적분 · 분대 항)(`:2278` · `:2354` · `:2368`) | |
| `  Cover: Finish (path)` | `FinishSweep`(`FindPathSync` + `MoveToLocation`)(`:2392`) | |
| `  Cover: Edge Advance` | `UpdateEdgeAdvance`(`:2195`) | |
| `Engagement Tick` | `SoldierEngagement.cpp:760` | |
| `Suppression Tick` / `Comms Tick` / `Health Tick` / `Objective Tick` | `:154` / `:228` / `SoldierHealth.cpp:738` / `SoldierObjective.cpp:86` | Health · Objective 는 off 스위치 없음(디버그 표시뿐인 틱) |
| `Field Tick` · `Field Shadows` · `Field Bake` · `Field Evict` · `Field Overlay` · `Field Wedge Reads` | `SoldierSituationField.cpp:171` · `:262` · `:316` · `:324` · `:424` · `:2533` | Wedge Reads 는 서브시스템 틱 밖(엄폐 층이 부른다) — Cover: Edge Advance 와 겹친다 |
| `Squad Tick` · `Zone Tick` | **선언만**(`SoldierLabLog.h:52-54`) | 분대 세션이 `Squad/` 틱에 `SCOPE_CYCLE_COUNTER` 를 걸 것 → [W103] |
| `Traces: Sight / Cover / Engagement / Field` | 동기 트레이스마다 `INC_DWORD_STAT`(Sight `:121` `:187` `:405` · Cover `:617` `:1392` `:1466` `:1567` `:1843` · Engagement `:251` · Field `:836` `:1939` `:2125`) | 프레임당 합 — "이 시스템이 이번 프레임에 물리 쿼리를 몇 번 했나" |

### 2.2 off 스위치 — 카운터가 틀릴 수 없는 유일한 측정

`SoldierLab.<System>.Enabled 0` 은 그 시스템의 `TickComponent` 첫 줄에서 반환한다(`GSoldier*Enabled`, 위 0절 표의 줄). 스코프 카운터는 스코프 **바깥**의 비용(캐시 미스 전파 · 그 시스템이 만든 이동 · 물리 씬 오염)을 못 보지만, 스위치를 끈 프레임과 켠 프레임의 **World Tick 차**는 그것까지 포함한다. 헤더 주석(`SoldierLabLog.h:25-27`): "the one measurement a counter cannot be wrong about."

⚠ 끄면 **거동이 바뀐다** — `Cover.Enabled 0` 이면 아무도 안 움직이고 안 훑는다, `Sight.Enabled 0` 이면 장님. A/B 는 **같은 장면 몇 초** 안에 토글하고 다시 켜야 비교가 된다.

`SoldierLab.Cover.Avoidance 0`(`SoldierCover.cpp:33-38`) 은 다르다 — 시스템을 끄는 게 아니라 병사 전원의 `UCharacterMovementComponent::SetAvoidanceEnabled(false)` 를 다음 틱에 적용(`:2181-2189`, `bAvoidanceApplied` 래치). 서로 통과하지만 판단은 그대로라 **RVO 만의 비용**이 `Char Movement` · `MoveComponent` 에서 읽힌다(6절).

---

## 3. 첫 계측 — 교전 중 `stat SoldierLab` [실측]

| 항목 | InclusiveAvg | 비고 |
|---|---|---|
| **Cover Tick** | **8.92 ms**(excl 6.86, 34 calls) | 하위 스코프는 아직 없던 판 |
| **Traces: Cover** | **487** / 프레임(max 624) | 병사당 ≈ 14 트레이스 / 틱 |
| Sight Tick | 0.96 | Traces: Sight 108 |
| Engagement Tick | 0.35 | Traces: Engagement 57 |
| Field Tick | 0.08 | |
| Perception · Suppression · Comms · Health · Objective | ≈ 0 | |

같은 시점 `stat game`: World Tick 28.5 · Blueprint 5.95 · CharMovement 1.59 · PostTick 1.25 · EndScopedMove 0.94 · MoveComponent 1.19 · Transform 0.84(877 calls).
`stat anim`: **PerformAnimEvaluation_WorkerThread 19.7 ms**(워커 스레드 — 게임 스레드 아님) · AnimGameThreadTime 3.85 · BlueprintUpdateAnimation 3.36(66 calls) · SkinnedMeshComp Tick 1.84 · UpdateKinematicBonesToAnim 1.2 · FinalizeAnimationUpdate 1.37.
`stat PoseSearch`: PCA/KNN 2.09 ms(101 calls, 워커).

**읽기**: SoldierLab 합 ≈ 10.3 ms 중 Cover 8.9. Cover 하나가 World Tick 의 **31 %**. Sight 의 108 트레이스가 0.96 ms 인데 Cover 의 487 트레이스가 8.92 ms 라 트레이스만의 문제가 아니라(트레이스당 ≈ 9 µs 로 잡으면 4.4 ms) **트레이스 없는 계산**(후보 점수 · 필드 읽기 · 적분)이 절반이다 — 이것이 (d) 의 근거가 됐다.

---

## 4. Cover 수정 넷 — 왜 각각이 결정을 보존하는가 [A]

원칙은 하나다: **같은 눈 · 같은 후보 · 같은 비용 함수 → 같은 자리**. 바꾼 것은 "언제 재느냐"와 "이미 진 후보에 얼마를 더 쓰느냐"뿐이다. 하위 스코프(`Here` / `Begin Sweep` / `Candidate` / `Route` / `Score` / `Finish` / `Edge Advance`)는 이 수정과 같이 들어갔다(2.1절).

### 4.1 (a) 이동 중 스윕 정지 — `bSweepSuspended`

**전**: 이동 중에도 매 틱 후보 하나를 눈 셋에 대해 전부 트레이스했다(후보당 `(3 + 2·heights + routes) × eyes`). 그런데 `FinishSweep` 은 `bAlreadyGoing`(`:2013-2016` — `Moving ∧ (진전 ∨ 유예 안)` ∨ 엣지 전진 관찰 중)이면 **아무 결정도 안 하고 버렸고**(`:2029-2035`), 이동 중 걸은 스윕은 `bSweepWalkedWhileMoving` 으로 **또 버렸다**(`:2036-2043` — "움직이는 링에서 뽑은 후보 vs 방금 정착한 HERE 는 비교가 아니다", 09-14). 즉 이동 중인 병사 전원이 **결과가 버려질 스윕을 매 틱 걸었다.**

**후**(`:2229-2244`): `bMoveUnderWay = Moving ∧ (Now − LastMoveIssuedSeconds < MoveGraceSeconds 0.75 ∨ |v|² > StallSpeedCms² 20)` — `FinishSweep` 의 `bAlreadyGoing` 과 **같은 식**(P162 · P87 의 정지 판정 그대로). 이것 ∨ `IsAdvanceLooking()` 이면 `bSweepSuspended = true`, 후보 루프(`:2296-2399`)를 통째로 건너뛴다. 풀리는 순간 `bSweepActive = false` → 다음 줄에서 **새 스윕이 지금 발·지금 눈으로 시작**(`:2250-2255`).

- **결정 보존**: 정지 뒤 첫 결정은 옛날에도 "정지 뒤 걸은 스윕"이었다(이동 중 스윕은 폐기). 지금도 그렇다. 차이는 폐기될 스윕을 **안 걷는 것**뿐. 도착 뒤 첫 결정까지의 지연은 같다(한 스윕 길이).
- **정지 판정이 "막힘"을 놓치지 않는가**: `Moving` 인데 속도 0(둘이 마주 서서 밀기)은 유예 0.75 s 뒤 `bMoveUnderWay = false` → 스윕 재개 → `FinishSweep` 이 옛 규칙대로 재결정. P87 그대로.
- **볼 곳(watch point)은 계속 필요하다** — 이동 중 조준(`GetAimPoint()`, ScanTurn · 굽이 너머 미리 보기 `WatchTravelBias`)이 `BeginSweep` 안의 볼 곳 계산에 매달려 있었다. 그래서 그 부분을 **`UpdateWatchPoint()`** 로 떼어(`:1887-1975`, `BeginSweep` 도 `:1875` 에서 부른다) 정지 중에는 **`WatchRefreshSeconds 0.25`**(바닥 0.05, `:2289`) 마다만 부른다. `GetMostExposedDirection` 은 필드 적분이라 트레이스 0, 09-18 의 "이동 중 스캔" 거동은 그대로다([C-149] 판정 재료 불변).
- **HERE 는 정지 중에도 갱신된다**(4.2 의 조건 — 걸음 > 30 cm 마다) — 자세(`RequiredStance`)·`bCanFight` 는 이동 중에도 교전 층이 읽기 때문. 위치 재평가가 아니라 자세 답이다.

### 4.2 (b) 발밑(HERE) 재평가 박자 — `HereEvalIntervalSeconds 0.1`

**전**: 매 틱 `EvaluatePosition(Here, SweepEyes, ∞)` — 눈 셋이면 **틱당 27 트레이스**(`(3 + 2·3) × 3`), 예산 밖(`TNumericLimits<int32>::Max()`), 병사 전원, 서 있든 말든. 487 트레이스 / 34 명 ≈ 14 인데 그 중 태반이 여기였다.

**후**(`:2257-2285`): `LastHereEvalSeconds < 0`(새 스윕 — `BeginSweep` 직후 `:2254` · `:2397` 에서 −1 로 리셋) ∨ `Now − Last ≥ HereEvalIntervalSeconds` ∨ `DistXY(Here, LastHereEvalFoot) > MicroStepCm 30` 일 때만.

- **결정 보존**: HERE 는 스윕 **끝**에 `BestCost + Margin < HereCost` 로 비교된다(`:2044`). 스윕은 후보 하나 / 틱이라 20+ 틱 걸리고, 그 사이 HERE 는 0.1 s 마다 최소 3~6 번 갱신된다 — 비교 시점의 HERE 는 여전히 ≤ 0.1 s 전 값. 눈은 스윕 동안 **얼려 있으므로**(09-14 "위협 추정은 한 바퀴 동안 얼린다") 같은 눈에 대한 같은 자리의 답은 0.1 s 안에 안 바뀐다 — 바뀌는 것은 발이 옮겼을 때뿐이고 그건 `MicroStepCm` 조건이 잡는다.
- **자세 지연**: `RequiredStance` 가 최대 0.1 s 늦을 수 있다. 포즈 스무더의 자세 축 속도(0.9~1.6 /s, [C-154])가 이보다 훨씬 느리므로 화면에서 구별되지 않는다(추정 → **[C-162]**).

### 4.3 (c) 경로 가지치기 — 정확한 최적화

**전**: 모든 후보에 `EvaluatePosition` + `EvaluateRoute`(경로 표본 ≤ 8 × 눈) 를 다 긋고 점수를 매겼다.

**후**(`:2345-2381`): 경로 없이 먼저 `ScorePosition(Foot, Fighting, RouteRisk = 0)` → **이 값이 이미 `BestCost` 보다 크면 경로를 안 긋는다.** 경로가 있으면 `RouteRisk > 0` 일 때만 다시 점수.

- **결정 보존, 근사 아님**: `RouteRiskWeight × RouteRisk ≥ 0` 이므로 `Cost(경로 없음) ≤ Cost(경로 있음)`. 경로 없이도 best 에 지는 후보는 경로를 더해도 진다 → 스윕의 best 는 **동일**(주석 `:2347-2350` "Exact - the same best comes out"). 후보 대부분이 지므로 경로 트레이스 대부분이 사라진다.
- 부수: `bBestValid` 가 아직 없을 때(첫 후보)는 항상 경로를 긋는다. 순서 의존 없음 — 어느 순서로 걸어도 min 은 같다.

### 4.4 (d) 눈 0 스윕의 후보 수 — `CalmCandidatesPerTick 2`

**전**: 루프의 상한은 트레이스 예산(`Budget ≥ SamplesPerCandidate`)과 `Attempts = 후보 수` 였다. 눈이 없으면 후보당 트레이스가 **0**(`EvaluatePosition` 눈 0 분기는 필드만 읽는다, 09-17) → 예산이 안 줄고 → **후보 전체(부채꼴 없이도 링 2겹 + 필드 후보 6 + 미세 8 ≈ 30~40개)를 한 틱에** → 매 틱 → 조용한 병사 전원. 후보 하나가 필드 읽기 몇 번 + `GetStaleVantage` 적분(순찰, 09-18) + 분대 항이라 트레이스 없이도 비싸다. 3절의 "트레이스 없는 절반"이 이것.

**후**(`:2311-2316`): `SweepEyes.Num() == 0` 이면 `Attempts = Clamp(CalmCandidatesPerTick, 1, 후보 수)`.

- **결정 보존**: 스윕은 후보 전부를 다 걷고 나서야 `FinishSweep` 한다(`:2389`). 틱당 2개면 40개 스윕이 ≈ 20 틱 = **0.33~0.67 s** — 눈 0 은 "전투 사이의 일"(순찰 · 도착 머무름 `ScanDwellSeconds 2`)이라 그 시간 척도가 초 단위다. 눈이 생기면 `BeginSweep` 이 다음 스윕부터 예산 규칙으로 돌아간다(눈이 있는 스윕은 원래 후보 1개 / 틱이었다 — 이쪽이 오히려 더 빨라진 셈).
- ⚠ 순찰 주기([C-148] "≈ 30 s 마다 옮겨 가는가")에 최대 0.7 s 가 더해진다 — 판정에 영향 없음(추정 → **[C-162]**).

### 4.5 바꾸지 않은 것

- `MaxTracesPerTick 36` · 후보 생성(부채꼴 · 링 · 필드 · 미세) · 비용 함수 · `FinishSweep` 의 규칙 전부 그대로.
- 부채꼴(`BeginSweep`) 트레이스는 안 줄였다 — 스윕당 1회라 이미 분할 상환됐고, 5절에서 `Begin Sweep 0.43 ms` 로 확인.
- 엣지 전진(`UpdateEdgeAdvance`)은 정지 여부와 무관하게 매 틱 돈다(`:2193-2197`) — 0.02 ms 라 손대지 않음.

---

## 5. 결과 [실측]

같은 레벨 · 같은 인원 · 교전 중 · 로깅 on(기준선과 같은 조건).

| `stat SoldierLab` | 전 | 후 | 비고 |
|---|---|---|---|
| **Cover Tick** | **8.92** | **1.92** | −7.0 ms |
| ↳ Candidate | — | 1.29 | Score · Route **포함** |
| ↳ Score | — | 0.51 | Here 안 + Candidate 안 합 |
| ↳ Begin Sweep | — | 0.43 | 부채꼴 + 볼 곳 |
| ↳ Route | — | 0.35 | 가지치기 뒤 남은 것 |
| ↳ Here | — | 0.15 | 0.1 s 박자 |
| ↳ Finish (path) | — | 0.03 | |
| ↳ Edge Advance | — | 0.02 | |
| **Traces: Cover** | **487** | **243** | −50 % |
| Sight Tick | 0.96 | 0.9 | 무변경 |
| Engagement Tick | 0.35 | 0.3 | 무변경 |
| Field Tick | 0.08 | 0.1 | 무변경 |
| **SoldierLab 합** | ≈ 10.3 | **≈ 3.3** | |

(하위 합 2.78 > 1.92 는 중첩 때문 — 2.1절. Candidate 1.29 안에 Score 대부분과 Route 0.35 가 들어 있다.)

| `stat game` | 전 | 후 |
|---|---|---|
| **World Tick** | 28.5 | **22.6** |
| 프레임 | — | 31 ms · **GPU 6 ms** → 게임 스레드 바운드 |

### 5.1 남은 게임 스레드 — 어디에 있나 (수정 뒤 캡처)

| 덩어리 | ms | 세부(InclusiveAvg) | 소유 |
|---|---|---|---|
| **애니메이션 게임 스레드** | **≈ 7.3** | `BlueprintUpdateAnimation` **3.47**(68 calls = 병사당 ABP **2개**) · `AnimGameThreadTime` 3.82 · `SkinnedMeshComp Tick` 1.85 · `FinalizeAnimationUpdate` 1.22 · `UpdateKinematicBonesToAnim` 0.97 | 포즈 세션 → [W98] |
| **이동 · 트랜스폼** | **≈ 4~7** | `Char Movement` 1.3~2.3 · `EndScopedMovementUpdate` 0.7~1.5 · `PostTick` 1.15 · `MoveComponent` 0.85~1.0 · `Transform or RenderData` 0.75~0.83(**≈ 800 calls = 병사당 부착 컴포넌트 ≈ 23**) · `UpdateOverlaps` ≈ 0.17(50~65 calls) | 캐릭터 BP → [W99] (RVO 몫 0.3~0.5 는 6절) |
| **캐릭터 · 무기 BP 틱** | **≈ 2.5** | `Blueprint Time` 5.7~7.1 − ABP 몫 3.47 | 캐릭터 BP → [W99] |
| **SoldierLab** | ≈ 3.3 | 위 표 | 이 층 → [W102] (뒤로) |
| **투사체 스폰** | ≈ 0.65 | `SpawnActor + SpawnTime` | 무기 → [W100] |
| Ticks Queued | ~960~1000 | 병사당 ≈ 25 틱 함수 | 전 층 |

**워커 스레드 애니메이션 19 ms 는 게임 스레드 병목이 아니다.** `PerformAnimEvaluation_WorkerThread` 19.7 ms 와 PoseSearch 2.09 ms 는 병렬로 돌고, 게임 스레드에는 그 **대기**가 `Post Tick Component Update ≈ 1.15 ms` 로만 나타난다. 게임 스레드에서 실제로 돈 애니메이션 일(≈ 7.3)이 문제다 — 그 중 3.47 은 ABP 이벤트 그래프(스레드-세이프 업데이트로 옮길 수 있는 것), 0.97 은 래그돌 전까지 필요 없는 키네마틱 본 갱신.

---

## 6. RVO A/B 와 히치 [실측 · 원인은 [B]]

`SoldierLab.Cover.Avoidance 0` 토글 전후, 교전 중:

| `stat game` | RVO 켬 | RVO 끔 | 차 |
|---|---|---|---|
| Char Movement | 1.58 | 1.32 | 0.26 |
| MoveComponent | **2.20** | 1.01 | 1.19 ⚠ |
| EndScopedMovementUpdate | 0.93 | 0.73 | 0.20 |

⚠ "켬" 캡처에 **히치가 섞여 있었다** — World Tick max **84 ms**, MoveComponent max **59.9**, `PrimComp DispatchBlockingHit` max **58.8**. 한 프레임의 `OnHit` 디스패치 하나가 58 ms — 평균(2.20)을 끌어올린 것은 RVO 가 아니라 이것이다. 히치를 빼면 RVO 의 정상 상태 비용은 **0.3~0.5 ms**(Char Movement + EndScoped 의 차 + MoveComponent 의 히치 제외분) → **회피는 켜 둔다**(서로 통과하는 것보다 싸다).

**히치 자체는 별건이다.** `DispatchBlockingHit` 58 ms 는 물리가 아니라 **그 히트의 BP 이벤트 핸들러 안**에서 난 시간이다 — 투사체 명중 이벤트(`OnHit` → 이펙트 · 데칼 · 사운드 스폰)에서 **동기 에셋 로드**(처음 쓰는 나이아가라/머티리얼/사운드를 그 자리에서 로드)가 나는 전형적 모양(추정 — 60 ms 는 디스크 로드 규모지 계산 규모가 아니다). 확인·수정은 무기/BP 소유자 몫 → **[W101]**: `stat game` max 열 + `LogStreaming`/`stat streaming` 으로 그 프레임의 로드를 잡고, 명중 에셋을 미리 로드하거나 `AsyncLoad` 로.

---

## 7. 소유자별 인계 표 — 재 본 ms 가 이유다

각 항목은 **측정된 값**이 근거이고, 그 값 이하로 내려가면 끝난 것이다. 순서는 ms 큰 순.

| 소유 | 항목 | 실측 근거 | 제안(소유자가 정한다) | ID |
|---|---|---|---|---|
| **포즈 세션**(ABP · `Pose/`) | ABP 이벤트 그래프 → 스레드-세이프 업데이트(`BlueprintThreadSafeUpdateAnimation` / 프로퍼티 액세스) | `BlueprintUpdateAnimation` **3.47 ms**, 68 calls(병사당 ABP 2 — 메인 + 포스트프로세스 `ABP_UEFN_Mannequin_PostProcess`) | 이벤트 그래프의 변수 읽기를 스레드-세이프로, PP ABP 가 병사에 정말 필요한지(P122 의 head/thigh 스케일이 거기 있다) | [W98] |
| 포즈 세션 | `KinematicBonesUpdateType = SkipAllBones` 를 사망(래그돌) 전까지 | `UpdateKinematicBonesToAnim` **0.97~1.2 ms** — 살아 있는 병사의 피직스 바디는 QueryOnly 라 본을 따라갈 이유가 없다 | `USoldierHealthComponent` 의 사망 경로가 이미 메시 콜리전을 바꾼다(`SoldierHealth.cpp:566-569`) — 거기서 `SkipSimulatingBones` 로 되돌리면 된다 | [W98] |
| 포즈 세션 | URO(`bEnableUpdateRateOptimizations`) / 거리별 애님 레이트 | `SkinnedMeshComp Tick` 1.85 · `FinalizeAnimationUpdate` 1.22 · 워커 19.7 ms(카메라에서 먼 병사 대부분) | 관전 카메라 기준 거리 티어. ⚠ MM 트래젝토리 · 발 IK 가 프레임 스킵에 민감 — 눈에 띄면 티어를 좁게 | [W98] |
| **캐릭터 BP**(`BP_SoldierCharacter`) | 부착 컴포넌트 수 | `Transform or RenderData` 0.75~0.83 ms · **≈ 800 calls / 35 = 병사당 ≈ 23** | 어느 23개인지 목록부터(P0-1 실험 잔재 · 디버그 화살표 · 카메라 컴포넌트가 AI 에도 붙어 있는지). AI 병사에 안 쓰는 것은 자식 BP 에서 제거 또는 `bAutoActivate=false` | [W99] |
| 캐릭터 BP | 오버랩 생성 컴포넌트 | `UpdateOverlaps` ≈ 0.17 ms, 50~65 calls | `Generate Overlap Events` 가 켜진 컴포넌트가 병사당 1~2개 — 필요한 게 아니면 끈다 | [W99] |
| 캐릭터 BP | 매 프레임 BP 틱 로직 | `Blueprint Time` 5.7~7.1 − ABP 3.47 = **≈ 2.5 ms**(386 calls) | 틱에서 매 프레임 도는 것(축 램프 · 브리지 · 무기 소켓 갱신)을 세고 C++ 컴포넌트가 이미 하는 것(스무더 · 게이트브리지)과 겹치는 옛 경로를 뺀다 | [W99] |
| **무기**(`BP_AR4Rifle` · `ASoldierProjectile`) | 투사체 풀링 | `SpawnActor + SpawnTime` **≈ 0.65 ms** / 프레임(35명 사격 중 한 발꼴) | 풀 + `Reset`; `bReplicates=false` 라 풀링에 복제 함정 없음 | [W100] |
| 무기/BP | `OnHit` 히치 | `DispatchBlockingHit` max **58.8 ms**, World Tick max 84 | 6절 — 그 프레임의 로드를 잡고 명중 에셋 preload / async | [W101] |
| **SoldierLab**(이 층, **다른 층 뒤에**) | Sight 표적 트레이스 간격 | Sight 0.9 ms · 108 트레이스 — 두 점 시야(가슴 → 머리)가 매 틱 라운드로빈 5 | 보이는 표적은 `ContinuityWindowSeconds` 안에서 격틱으로 (≈ 0.4 ms) | [W102] |
| SoldierLab | Cover 후보 트레이스 박자 | Candidate 1.29 · Traces 243 | 눈 있는 스윕도 후보 하나를 2틱에 걸쳐(≈ 0.5 ms) — 결정 지연 ×2 라 [C-149] · [C-153] 과 같이 볼 것 | [W102] |
| SoldierLab | 틱 함수 솎기 | Ticks Queued ≈ 25 / 병사 중 이 층 11 | Health · Suppression · Comms · Perception 은 `PrimaryComponentTick.TickInterval`(0.1~0.2 s) 로 — 각각 ≈ 0 ms 라 ms 가 아니라 큐 길이 몫 | [W102] |
| **분대 세션** | `SCOPE_CYCLE_COUNTER(STAT_SoldierLab_Squad / _Zone)` | 선언만 있음(`SoldierLabLog.h:52-54`) — 분대 서브시스템·존 틱이 `stat SoldierLab` 에 안 보인다 | 두 줄 | [W103] |

이 층은 **다른 세 층이 끝난 뒤에** 다시 잰다 — 지금 3.3 ms 는 남은 22.6 의 15 % 라 순서가 뒤다(P182 의 "이름을 댄 것부터").

---

## 8. 측정 규약 — 다음에 잴 때 (P182)

이번에 배운 것을 규칙으로:

1. **조건을 고정한다** — `SoldierLab.Debug.* 0` 전부(오버레이는 자체 ms 가 있다 — `Field Overlay` 스코프 · P155) · **AI 로깅 off**("PROFILING WITH AI LOGGING ON" 배너가 없어야 한다) · 같은 카메라 위치 · 같은 교전 시점(시나리오 단계로 맞춘다) · **Standalone**(PIE 는 에디터 틱 · 디테일 패널 · 언두 버퍼가 섞인다). 이번 숫자는 PIE + 로깅 on 이라 **절대값은 높고 차이만 믿을 것**.
2. **의심 대상은 세 갈래로 확인한다** — ① 자기 스코프의 ms(`stat SoldierLab`) ② `.Enabled 0` 의 World Tick 차(스코프가 못 보는 바깥 비용 포함) ③ **병사 수로 스케일링**(15 → 35 에서 비례하는가 — 비례하지 않으면 병사 몫이 아니다). 셋이 같은 말을 해야 이름이 붙은 것이다.
3. **계측이 이름을 댄 것만 고친다** — 그리고 전후 stat 줄을 문서에 붙인다(5절 표가 형식). 추측으로 고친 최적화는 거동을 바꾸고도 ms 를 안 준다.
4. **평균과 max 를 같이 읽는다** — 6절의 RVO 처럼 max 가 평균을 끌어올린 캡처는 A/B 가 아니다. max 가 튀면 그 프레임을 따로 잡는다.
5. **워커 스레드 ms 는 게임 스레드 ms 가 아니다** — `stat anim` 의 19 ms 는 대기(`Post Tick Component Update`)로만 게임 스레드에 닿는다. GPU 도 마찬가지(6 ms). 병목은 `stat unit` 의 Game 열이 정한다.
6. **하위 스코프는 중첩**이다 — 합이 부모보다 커도 정상(2.1절). Exclusive 가 필요하면 `stat SoldierLab -exclusive`.

---

## 9. 브리핑과 코드가 어긋난 자리

- `Soldiers Ticked` 는 "판단한 병사"가 아니라 **시야 컴포넌트가 틱한 액터**(`Enabled`·권한 검사 앞, `SoldierSight.cpp:218`) — 리슨서버 클라에서는 사본도 센다. 병사 수 스케일링(8절 ②)의 분모로 쓸 때 주의.
- HERE 재평가(4.2)는 **정지 중에도** 돈다(`bSweepSuspended` 분기 **앞**, `:2257`) — 이동 중 30 cm 마다 = 걷기 속도에서 ≈ 0.1 s 마다. "이동 중 스윕 정지"가 HERE 까지 멈추는 것은 아니다(자세 답이 필요하다). 브리핑의 "(b)" 는 이 뜻으로 읽을 것.
- `WatchRefreshSeconds` 의 바닥은 **0.05 s**(`:2289`) — 0 을 줘도 매 틱은 아니다.
- `CalmCandidatesPerTick` 은 `ClampMin 1`(`.h:558-559`) — 0 이면 1.
- Health · Objective 는 off 스위치가 **없다**(디버그 표시뿐인 틱 — 스코프만).
- 첫 계측(3절) 시점에는 Cover 하위 스코프가 **없었다** — 수정과 같이 들어갔으므로 "전" 열의 하위값은 없다.

---

## 10. 정정 (2026-09-21 후속 세션 — `ai/2026-09-21_game_thread_batch_cameras_abp_muzzle.md`)

후속 세션이 `stat dumpframe -ms=0.05` 로그 블록을 파싱해(그 문서 1절) 틱 함수별 ms 를 얻은 뒤 이 문서의 읽기 셋이 틀린 것으로 드러났다. **본문 숫자는 그대로 두고 여기서만 정정한다**(3.3절 규칙).

| 자리 | 본문 | 정정 [A] |
|---|---|---|
| 5.1절 · 7절 [W98] ① | ~~`BlueprintUpdateAnimation` 3.47 ms, 68 calls = 병사당 ABP **2개**(메인 + PP)~~ | `STAT_BlueprintUpdateAnimation` 은 GT 이벤트 그래프(`AnimInstance.cpp:795`)와 **워커 스레드의 `BlueprintThreadSafeUpdateAnimation`**(`AnimInstanceProxy.cpp:1354`)이 **같은 스코프** → 68 = **34 GT + 34 워커**, ms 는 두 스레드 합산. 실제 GT 이벤트 그래프는 병사당 **29~40 µs**(37명 ≈ 1.2 ms). PP ABP 는 `soldier_T` · `new_enemy_T` 모두 **None**(MCP 확인). `obj list class=SoldierCharacter_ABP_C` = 82 는 PIE 40 + 에디터 월드 40(틱 안 함, `AreActorsInitialized` false) + 프리뷰 2. → [W98] ① 의 기대치는 **≤ 1 ms 로 하향**(후속 세션이 cvar 폴링 1 Hz 로 29 → 22 µs) |
| 7절 [W98] ② | ~~`KinematicBonesUpdateType = SkipAllBones` — 피직스 바디는 QueryOnly 라 본을 따라갈 이유가 없다~~ | **채택 불가** — 피격 부위 판정이 **메시 바디 트레이스**(`AI/SoldierHealth.cpp:289-305 ResolveBoneByTrace` → `BodyPartScaleFor`)라 QueryOnly 바디가 **포즈를 따라가야** 머리/가슴이 갈린다. `UpdateKinematicBonesToAnim` 0.97 은 정당한 비용. **② 폐기** |
| 7절 [W98] ③ | URO / 거리별 애님 레이트(⚠ MM 이 프레임 스킵에 민감) | 후속 세션이 `bEnableUpdateRateOptimizations=true` 로 켜자 **약 2분 만에 에디터 크래시**(`BonePose.h:645` 포즈 NaN). "민감" 이 아니라 **불가** → 폐기, **P186** · 되돌리기 [W107] · 원인 [C-165] |
| 7절 [W99] "병사당 ≈ 23 컴포넌트" | Transform calls 역산 | 실제 런타임 목록 **38개** = CDO 29 + PIE 카메라 프록시/프러스텀 6 + OutputCamera + GameplayTasks + SoldierLabDetectable. 이 문서가 못 본 큰 덩어리가 그 안에 있었다 — **AI 병사 전원의 GASP GameplayCamera 1.60 + SpringArm 0.38 = 2.0 ms**(`bAutoActivate` + `bRunStandaloneCameraSystem` CDO 상속 → `GameplayCameraComponentBase.cpp:552-568` · `:622-657`). BeginPlay 에서 비플레이어면 틱 off → World Tick **22.6 → 19.4 / 18.4**. [W99] 카메라 항목 **완료**, BP 틱 C++ 이관은 [W108] |
| 5.1절 · 7절 [W100] "투사체 스폰 ≈ 0.65 ms" | 프레임 평균 | **발당 ≈ 1.1 ms**(`BeginDeferredActorSpawnFromClass` 0.45 + `FinishSpawningActor` 0.12 + 총구 `SpawnSystemAttached` 0.38 + `LaunchFrom` 0.08), 프레임 2발 = 2.25. 총구 `SpawnSystemAttached` 는 `bAutoDestroy=false` 라 **컴포넌트 영구 누적 누수**(8정에 106개) → 상주 `MuzzleFlashFX` + `Activate` 로 해결. 풀링은 [W109] |
| 7절 [W103] | 선언만 | **완료**(`Squad/SoldierSquadSubsystem.cpp` · `Squad/SoldierZone.cpp` 에 `SCOPE_CYCLE_COUNTER`, 빌드 대기) |
| 1절 "Ticks Queued 968 → 병사당 틱 함수 25개" | 역산 | 후속 세션 실측 틱 함수 ≈ 850(35명) — 틱 오버헤드 0.95 ms. Health · Comms `TickInterval 0.1`(빌드 대기), Suppression · Perception 은 DeltaTime 의존이라 그대로([W102] ③ 절반) |

계측법 자체도 하나 는다: `stat SoldierLab` 은 **우리 코드**만 이름을 대고, 엔진·BP 를 포함한 전체 틱 함수는 **`stat dumpframe -ms=0.05` → `Saved/Logs/titan_example.log` 의 `LogStats:` 블록** 을 파싱해 얻는다(MCP 에 콘솔 툴이 없어 사용자가 친다). 8절 규약에 이 방법을 더한다 — `CLAUDE.md` 6.2c.
