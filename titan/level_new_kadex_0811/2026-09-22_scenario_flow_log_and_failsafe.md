# 시나리오 흐름 로그 + hard fail-safe (무인 반복 실행 1단계)

2026-09-22 (2026-09-28 갱신) / 진행중(무인 실행 **12시간 85사이클**(§7) + **2-PC 8.7시간 59사이클**(§9) 전부 완주 — 시나리오 정지 0건 · "적이 안 쏜다"의 원인은 병사의 차량 표적 판정 버그로 확정돼 수정됨(§8.2) · 2-PC 로그가 잡아낸 **RCWS 도탄 액터 누수 수정**(§9.1, 빌드 대기)) / 사이클·스텝 타임라인과 액터 이벤트를 `Saved/ScenarioLogs/` 별도 파일에 남기고, 사이클이 20분을 넘으면 스냅샷 남기고 강제 재시작하는 `UScenarioMonitorSubsystem` 신설. 세부 fail-safe(스텝별 예산, UGV 전복 복귀, 숨은 적 강제 제거)는 이 로그로 데이터가 쌓인 뒤.

선행: `2026-09-22_scenario_restart_implementation.md`(재시작 — 이 작업의 전제), 규칙은 `scenario_authoring_guide.md` §2.8.

---

## 1. 왜 이 순서인가 (2026-09-22 토론 결론)

원래 계획은 "대충 로그 → Chronicle 로 n 시간 녹화 → 막히는 데 수정"이었는데, **fail-safe 가 없으면 첫 정지에서 n 시간이 그대로 날아간다.** 그래서:

1. **흐름 로그**(이 문서) — 지금은 스텝별 소요가 얼마인지 데이터가 하나도 없다. 먼저 잰다.
2. **hard fail-safe** — 사이클 총 시간 하나만 본다. 스텝마다 걸리는 시간이 다르고 어떤 스텝을 단위로 잡을지도 애매해서, 넉넉한 총량(20분)이 처음엔 합리적이다. 조치는 `RequestScenarioRestart()` 직접 호출.
3. 데이터가 쌓이면 스텝별 예산·세부 fail-safe(UGV 전복/땅 꺼짐 → 안전 위치, 적 한 명이 안 죽고 남으면 강제 제거 등)·근본 원인 수정.

**적을 다 죽여 완료로 밀지 않는 이유**: `ScenarioComplete` 행의 Prereq 가 `EnemyEngage` 라 그 전에 멈추면(드론이 낙하산을 못 봄, UGV 가 못 감) 전멸시켜도 완료가 안 뜬다. 억지로 완료시키면 그 사이클이 "완주"로 기록돼 데이터가 오염된다. 재시작이 어차피 병사 전원을 파괴한다.

**발사/피격은 개별 기록 안 함**(사용자 확정) — 최신 로그에서 RCWS `발사!` 253줄이 10사이클 분량이고 병사 15명 자동사격이면 수천 줄. 위치·속도도 안 남긴다(Chronicle 몫). 남기는 건 "언제 무슨 일이" 뿐.

---

## 2. 만든 것

| 파일 | 내용 |
|---|---|
| **신규** `Source/titan_example/UI/ScenarioMonitorSubsystem.h/.cpp` | `UTickableWorldSubsystem`(Game/PIE, 서버 전용, `AScenarioConfig` 있는 레벨만). 파일 2개(`cycles.csv`, `events.log`, UTF-8 BOM, 줄마다 Flush — 크래시 대비). 0.25s 폴링으로 액터 전이 감지. fail-safe. 콘솔 6종. 병사 사망은 `USoldierHealthComponent::OnDeath`(다이내믹이라 병사마다 `UScenarioMonitorDeathHook` 하나), 사격은 `USoldierRegistrySubsystem::OnGunshot` |
| `UI/ScenarioStateSubsystem.cpp` | 훅 4곳: `BeginScenarioSteps` 끝(`OnCycleBegin(ScenarioCycleCount+1, DT)`), `FireScenarioStepInternal`(이펙트 실행 **전** `OnStepFired`), `RequestScenarioRestart` 수락 직후(`OnRestartRequested`), `FinishScenarioRestart`(`OnRestartFinished`) |
| `UI/ScenarioConfig.h` | `FailSafeMaxCycleSeconds = 1200`(`Scenario|Restart`) |
| `scenario_authoring_guide.md` | §2.8 신설, `ScenarioConfig` 필드 표에 1행 |

**건드리지 않은 것**: SoldierLab 전체, UGV/RCWS/드론 코드, 재시작 로직. 액터 상태는 공개 getter 폴링(`IsMoving`, `GetDriveMode`, `CurrentMode`/`bFireSystemActive`/`CurrentAutoAimTarget`/`GetShotsFiredCount`, `GetCurrentData().AmmoCurrent`, `Autopilot->GetState()`, `HasObservedParachute`, `GetDetectionPhase`, `GetGimbalFramingSet`, `GetGimbalReconPhase`, `GetManualControlMode`, 등록부 `GetAll()`, `Identity.SquadId`/`GetAssignment().Mode`, `GetSquadStatus().bAchieved`).

### 이벤트 목록 (events.log 태그별)

| 태그 | 이벤트 |
|---|---|
| `SYS` | 세션 시작(레벨·RunMode·fail-safe 설정) / 종료 |
| `SCN` | 사이클 시작(등록부 적/아군 수, 직전 재시작 ms) · 스텝 발동(직전 스텝 +s, 트리거, 이펙트, `IssueSquadOrder` 면 진영/동사/분대/존/속도/ROE/agg/quota) · 완료(적 전멸) · 재시작 요청 수락(사이클 결과) · 재시작 완료/실패 ms |
| `UGV` / `CP` | 자율주행 시작(경로 점수·길이)/종료(소요·직선거리) · 드라이브 모드 · RCWS 모드 · ARM/SAFE · 표적 획득/상실/전환 · **표적별 첫 사격**(누적 발수·탄) · 탄 소진 |
| `UAV` | 오토파일럿 상태 전이(경로·진행%) · 낙하산 관측 · 탐지 단계 · 프레이밍 · 짐벌 정찰 · 수동 조종 |
| `SQD` | 분대 명령 달성(명령 +s, 생존 a/b) |
| `SLD` | 분대 편입(1→2) · 작업 전이(Approach→Hold, 앵커까지 m, 명령 +s) · **명령 뒤 첫 사격** · 사망 ← 가해자(누적) · 등록부에서 사라짐(사망 아님 — 비정상 신호) · 사이클 도중 신규 등록(비정상) |
| `FAIL` | fail-safe 발동 사유 / `RequestScenarioRestart` 거부(30s 재시도) |
| `SNAP` | UGV(pos, up.z, 속도, 주행, 모드, RCWS 상태) · 지휘소 RCWS · 드론(pos, 오토파일럿, 진행, 관측/탐지/프레이밍/수동) · 등록부 수 · 병사 전원(진영/분대/리더, pos, hp, 작업, 앵커 거리, 속도, 분대 상태) |
| `MARK` | `titan.ScenarioLog.Mark <메모>` |

### 사이클 결과 판정 (`cycles.csv` result)

`RequestScenarioRestart` 수락 시점에 결정: 완료 행이 발동했으면 `Complete`, fail-safe 가 이 사이클에서 불렀으면 `FailSafe`, 그 외(확인창 [예]/콘솔로 완료 전 재시작)는 `Restart`. 월드가 닫히면 `SessionEnd`, 재시작 없이 스텝 평가가 다시 시작되면 `Superseded`(비정상).

---

## 3. fail-safe 동작

```
매 0.25s (재시작 진행 중이면 쉼)
  사이클 진행 중 && Mode≠0 && (Mode==2 || 자동 재시작 ON)
    미완료: 경과 > MaxCycleSeconds(cvar>0 ? cvar : Config 1200)
    완료:   마지막 스텝 뒤 > MaxWaitAfterCompleteSeconds(120) && 자동 재시작 ON   ← 확인창 행이 안 뜬 경우
  → FAIL 줄 + SNAP 스냅샷 + 엔진 로그 Warning → RequestScenarioRestart()
    거부되면(스냅샷 없음 등) 30s 뒤 재시도
```

Mode 기본 1(자동 재시작 ON 일 때만)인 이유: 사람이 관전·디버그 중 멈춘 장면을 보고 있는데 20분 뒤 판이 갈리면 안 된다.

---

## 4. 검증 체크리스트

헤더에 UCLASS 2개·UPROPERTY 가 새로 생기므로 **정식 빌드**([[feedback_live_coding_uproperty_missing_property]]) — 2026-09-22 통과(첫 빌드 오류 1건: 스냅샷 함수의 로컬 `Squads` 가 멤버를 가림 → `SquadSys`).

1. ✅ `New_kadex_0811` PIE → `Saved/ScenarioLogs/2026.09.22-12.25.14_New_kadex_0811/` 생성.
2. ✅ (215초, PIE 중단 → `result=SessionEnd`) 실측 타임라인: `UAVMission +3.2` · `UAVSpotted +65.8` · `UGVArriveZone1 +153.6`(주행 87.8s, 60점 663m) · `EnemyEngage +159.6` · `EnemyFleeToZone2 +202.0`. `UAV Idle→Takeoff(+3.2)→Following(+11.7)→Arrived(+60.4)`, `낙하산 관측 완료 +65.9`, `RCWS Remote→AutoFire`, 표적 획득/전환/사격 시작, 적 15명 `첫 사격(명령 +0~34s)`, `사망 ← BP_UGV_0901` 4명, `분대 편입 1→3/1→2`, `적군/1 명령 달성 Occupy 명령 +40.6s`. 152줄/215초 — 잡음 수준 양호(사이클 시작에 `작업 None→…` 40줄은 명령 적용이라 정상).
   - 고친 것: 등록부 아군 27 = 병사 25 + 차량 Identity 2 → 보병(체력 컴포넌트)만 세게 / 정원 편입된 병사의 "명령 +Ns" 기준을 편입 시점으로.
3. ✅ 자동 재시작 ON 1시간(`2026.09.22-12.30.06_New_kadex_0811/`, 12:30~13:33) — **7사이클 완주 + 8번째 중단**, 전부 `Complete`, 재시작 1.15~1.19s, fail-safe 미발동, `등록부에서 사라짐`/`도중 신규` 0건. 2,179줄(사이클당 ~270). 실측 분포(§4.1) 와 발견(§4.2) 은 아래.
4. fail-safe 실검: `titan.ScenarioFailSafe.MaxCycleSeconds 60` + `titan.ScenarioFailSafe.Mode 2` → 60초 뒤 `FAIL` + `SNAP` + 재시작 → `cycles.csv` 그 행 `result=FailSafe`. 끝나면 두 cvar 원복.
5. 잡음 점검: 병사 `작업 전이` 줄이 사이클당 몇 줄인지(수백 줄이면 Approach/Hold 외 전이는 빼기), RCWS `표적 전환` 이 스티키 표적 때문에 초당 여러 번 튀는지.
6. 밤새 실행 뒤 `cycles.csv` 를 열어 사이클 소요 분포 → `FailSafeMaxCycleSeconds` 조정 + 스텝별 예산 설계(다음 문서).

### 4.1 1시간 실측 분포 (7사이클, 사이클 내 +s)

| 구간 | 값 | 편차 원인 |
|---|---|---|
| 사이클 총 | **443 ~ 512 s**(7.4~8.5 분) | 아래 셋의 합 |
| `UAVSpotted` | 63.0~64.6 | 없음(드론 비행 고정) |
| `UGVArriveZone1` | 150.4~153.0(주행 87.2~88.3 s, 60점 663 m) | 없음 |
| `EnemyEngage` | 155.8~163.8 | 없음 |
| `EnemyFleeToZone2`(사망 ≥3) | 172.8~205.4 | UGV 교전 운 |
| **`UGVMoveZone2`**(적 전원 55 m 밖) | **217.6~367.6** — Flee2 뒤 23~**176** s | **1분대 잔존**: 1분대는 2차 존이 없어 zone 0 을 Occupy 한 채 UGV 가 다 죽여야 함. C02 는 `Hostile_1` 이 +256 에 시야 상실 뒤 +360 까지 104 s 숨어 있었다 → 사용자 예시 "적 한 명이 계속 숨어있음" 그대로 |
| `EnemyFleeToZone3`(사망 ≥7) | 221.2~330.6 | 위와 같은 운 |
| `Squad3Stand`(트럭 80 m 사격) | 369.0~464.0 | 3분대 도주 속도·트럭 시야 |
| `ScenarioComplete` | 428.6~496.8 — Squad3Stand 뒤 44~83 s | 트럭 교전 |
| UGV 2차 주행 | 71.3~103.0 s(28점 268 m) | `UGVSpeedLimitOn`(적 근접 감속) |
| 탄 | UGV 사이클당 80~110 발, 트럭 30~70 발, 매 사이클 600 리필 확인 | |

**hard fail-safe 20분(1200 s)은 최대치 512 s 의 2.3배 — 그대로 둔다.** 스텝별 예산의 1순위 후보는 `UGVMoveZone2`(Flee2 뒤 176 s 실측 → 예산 240 s 정도 + 초과 시 1분대 잔존 강제 제거).

### 4.2 발견 (로그가 아니면 못 봤을 것)

1. **아군 2명 사망 — 둘 다 아군 오사, 둘 다 3차 구간(+322.9 / +324.9)**: C05 `Friendly_4 ← Friendly_7`(1분대), C08 `Friendly_22 ← Friendly_21`(5분대). 아군은 무적이어야 하는데(재시작 설계 문서 §2 "아군 `bInvincible` 사용자 체크") **`New_kadex_0811.umap`·`BP_Soldier_Friendly.uasset`·`L_SoldierScenario.umap` 어디에도 `bInvincible` 이름이 없다**(FName 테이블 grep 0건 — 비기본값이면 반드시 직렬화됨) → 이 레벨의 아군은 지금 무적이 아니다. 적군 사격에는 8사이클 동안 한 명도 안 죽고 아군끼리만 죽은 건 아군이 일렬로 서서 서쪽으로 쏘는 배치라 뒤에서 앞을 맞히는 것. → **사용자 결정 필요**: 무적으로 할 거면 BP_Soldier_Friendly 의 AC_SoldierHealth.bInvincible 을 켜거나(클래스 기본값 — 델타 불필요) 인스턴스 25개에 체크.
2. **킬 분포**: UGV 10/사이클, 트럭 4~5, **아군 보병은 8사이클 동안 적 2명**(C01·C02 각 1). 아군 25명은 사실상 장식 — `AllyDefend ReturnFireOnly 60 m` → `AllyEngage Free 80 m` 인데 적이 아군 80 m 안에 오는 건 도주 분대뿐이고 그건 UGV/트럭이 먼저 잡는다. 의도라면 OK, 아니면 [C-164] 페이싱 항목에 붙일 것.
3. **C02 드론 프레이밍 역순** `Zone1 → Zone3 → Zone2`: 1번 때문에 `EnemyFleeToZone3`(+240) 가 `UGVMoveZone2`(+367) 보다 먼저 와서 `DroneFrameZone3` 가 먼저 발동, 그 뒤 `DroneFrameZone2` 로 되돌아감(데모 화면에서 드론이 지휘소를 보다 다시 2차로 옴). 1분대 잔존이 길어지면 항상 생긴다 — `DroneFrameZone2` 의 Prereq 가 `AllyEngage` 라서. 스텝 순서 보장은 DT 로: `DroneFrameZone3` Prereq 에 `UGVMoveZone2` 를 거는 식(저작 결정).
4. 트럭 `RCWS 사격 시작 → (표적 없음)` 1건(C04 +407.9) — 버스트 중 표적 상실, 무해.
5. 로그 잡음 수정 2건: 아군 분대 `명령 달성 +246s`(AllyDefend→AllyEngage 가 둘 다 Occupy 라 폴링이 새 명령을 못 봄 → 스텝이 직접 기준 시각을 잡게), 등록부 아군 27(차량 Identity 2 포함 → 보병만).

---

## 5. 스텝 fail-safe 사다리 (2026-09-22 오후 — 1시간 데이터 보고 설계, 코드 완료·빌드 대기)

토론 결론(사용자): 분대별로 사다리를 두되 **"정상 분포 밖에서만, 티 안 나는 수단부터"**. 사망 수 트리거(≥3/≥7)는 "n차 교전 시작 뒤 m초"를 OR 로 붙이고, 1분대는 [잔존에게 2차 존으로 BreakContact → UGV 강제 출발], 3분대는 [엄폐 없는 킬존(`Z3_S3_Kill`)으로 이동 → 전원 제거]. 2분대 사다리(아군이 2차 존 점령)는 OR 타임아웃으로 체인이 1·2분대 사망에 의존하지 않게 되면서 **선택**으로 내려감(데이터 보고 필요하면). UGV 1차 전투지 순찰안은 시나리오 이탈 + 숲 내비 리스크로 폐기. 이전 제안 "트리거를 억지로 발동시키지 않는다"는 최종 그물(`PruneAll`)이 잔존을 받아주는 구조라 철회.

**용어**: 사이클 상한 → 재시작은 fail-safe(안전 상태로 복귀), 스텝 타임아웃/가지치기는 watchdog + fallback 인데 실무에선 통틀어 fail-safe 로 부른다.

**DT 부품 3개** (코드, `UI/ScenarioStepTypes.h` · `UI/ScenarioStateSubsystem.cpp`):

| 부품 | 동작 |
|---|---|
| `FScenarioStepRow.TimeoutSeconds` | Prereq 발동 뒤 N초면 트리거 무시하고 발동(= 조건 OR 시간). `TickScenarioSteps` 스위치 뒤 한 블록. `FireScenarioStepInternal(…, bTimedOut)` → Warning 로그 + 모니터 `FAIL` + `cycles.csv` `timeouts` |
| `FScenarioStepRow.SkipIfStepFired` | 지정 스텝이 발동했으면 이 행 평가 안 함(루프 맨 앞 게이트) |
| `EScenarioEffectType::KillSquads` | `KillSquadsForScenario(Spec, Label)`: 등록부에서 진영/분대 일치 + 체력 컴포넌트 보유(차량 Identity 제외) + 살아있음 → 먼저 모아서 `USoldierHealthComponent::Kill(UGV)`. Warning 로그(이름 목록) + 모니터 `OnPruned` → `pruned` 열 |

`cycles.csv` 열 추가: `timeouts`, `pruned`. `result` 에 **`CompleteAssisted`**(완료됐지만 타임아웃/가지치기 개입) 신설 — 자연 완주와 분리해야 근본 원인이 안 묻힌다.

**행 저작 표·레벨 데이터**는 `scenario_authoring_guide.md` §2.8(정본). 빌드 뒤 DT 에 넣는다(새 열/enum 은 빌드 전엔 없음).

### 진행 (2026-09-22 17시)
1. ✅ 빌드 → DT 8행(MCP `set_rows`; 구조체는 통째) → `Z3_S3_Kill`(=`SoldierZone_8`, (55000,13200,−3742), 반경 500, Squad3 필터 — `trace_world` 로 평지·트럭 시야 확인) → `SquadZones` 적군 1/3 갱신(MCP `set_properties`: `zones` 를 **`{"refPath":…}` 객체**로 넘겨야 배열 갱신이 받아들여짐, 문자열 경로는 "elements changed alongside size change" 거부) → 맵·`BP_Soldier_Friendly` Invincible 저장(사용자).
2. ✅ **강제 검증 1사이클**(`2026.09.22-17.03.44`, 임시값 UGVMoveZone2 Timeout 10 / Squad3Expose 5 / PruneAll 20): `FAIL 스텝 UGVMoveZone2 타임아웃 발동`(Flee2+10.0) → UGV 출발, 잔존 있어도 `EnemyFleeToZone3` 자연 발동(+245) / `Squad3Expose` → `zone=3` 4명 → **트럭이 12 s 안에 3명 사살**(킬존 성립) / `PruneAll` → `KillSquads 2명 제거: Hostile_4, Hostile_11` → 0.2 s 뒤 `ScenarioComplete` / `cycles.csv` `result=CompleteAssisted, timeouts=1, pruned=2`. 원복 완료·저장.
   - **발견**: `Squad1LateWithdraw` 의 `BreakContact` 는 **존을 안 바꾼다**(SoldierLab 설계: "the zone stays") — 1분대 잔존 `Hostile_4` 가 zone 0 에 그대로 서 있다가 PruneAll 에 제거됨. → `Withdraw` zone 1 + `Squad1LateRun`(BreakContact, +2 s) 2행으로 수정(DT 저장).
   - 로그 잡음: 트럭 사살의 가해자가 `BP_RCWSProjectile8` 로 찍힘 → 모니터가 투사체를 소유 차량으로 치환(코드, 다음 빌드).
3. ✅ **사다리 적용 1시간**(`2026.09.22-17.17.35`, 17:17~18:48, 9사이클 완주): 타임아웃 0, 사이클 fail-safe 0, `CompleteAssisted` 1(C8: Expose + Prune 3), C5 는 Expose 만(트럭이 49 s 안에 마무리). 사이클 434~645 s.
   - **1분대 사다리 실증**: C2 `Squad1LateWithdraw`(+269) → 잔존 `Hostile_4` Hold→Approach → 3 s 뒤 UGV 표적 획득 → +275 사망 → 0.3 s 뒤 UGV 자연 출발. C5 동일.
   - **새 병목 = 2분대 잔존**: C8 `Hostile_6/7/9` 가 2차 존 도달(+261~288) 뒤 **PruneAll(+612)까지 340 s 교착** — UGV(ugvpoint2, 엄폐라 못 봄)·아군(70~95 m, `AllyEngage` 80 m 밖)·ROE(ReturnFireOnly, 아무도 안 쏨). `Squad3Expose` 는 3분대 전용이라 무관. Stand→완료 꼬리가 44~169 s 로 길어진 원인. → 사용자 원안의 **2분대 1차** 복원: `AllyClearZone2`(Squad3Stand+60, Skip ScenarioComplete, 아군 전원 Occupy zone 1 Rush Free 0.8) + 아군 존 `ZF_Zone2_Clear`(`SoldierZone_9`) + 아군 `SquadZones` index 1. DT·맵 저장(18:52).
4. ✅ **12시간 무인 실행**(`2026.09.22-18.56.36`, 18:56~06:57, **85사이클 전부 완주**) — §7.

## 7. 12시간 무인 실행 결과 (2026-09-23)

`Saved/ScenarioLogs/2026.09.22-18.56.36_New_kadex_0811/`(events.log 24,324줄 / 3.0 MB, cycles.csv 86행).

| 항목 | 값 |
|---|---|
| 사이클 | **85 완주 + 86번째 중단**(사용자 정지). 시나리오 정지 **0건**, 사이클 fail-safe(20분) **0건**, 타임아웃 **0건**, 가지치기 **0건** |
| 사이클 소요 | 426 / **495**(중앙) / 636 s |
| 재시작 | 1146 / 1188 / 1256 ms — 85회 내내 평평(누수 징후 없음) |
| 이상 이벤트 | `등록부에서 사라짐`·`사이클 도중 신규`·탄 소진·수동 조종 **전부 0**. 아군 사망 **0**(무적 적용 확인) |
| 사살 | UGV 836 · 트럭 410 · **아군 보병 18** · 적 오사 11 |
| 사다리 발동 | `Squad1LateWithdraw`/`Run` 85/85(단 **74회는 대상 0명**, 21회만 1~2명) · `AllyClearZone2` **38/85** · `Squad3Expose` 10/85 · `PruneAll` **0** |

### 7.1 fps 하락 → 물리 클램프 → 시나리오가 느려진다 (진짜 발견)

같은 경로·같은 행동인데 밤이 갈수록 느려졌다(사이클 중앙값 495→539 s):

| 구간(사이클 10개 묶음) | 1–10 | 21–30 | 41–50 | 61–70 | 81–85 |
|---|---|---|---|---|---|
| UGV 1차 주행 663 m (s) | 87.2 | 87.8 | 90.8 | 94.3 | **96.8** |
| 드론 Takeoff→Arrived (s) | 54.6 | 56.9 | 58.5 | 61.0 | 61.0 |
| 적 보병 1차 접근 280 m (s) | 263.6 | 264.8 | 273.9 | 275.7 | 278.6 |

- **벽시계 = 월드 시간(비 1.000, 85사이클 내내)** — 시간 팽창이 아니다. 즉 "월드 1초당 이동 거리"가 줄었다.
- 엔진 로그 프레임 카운터로 잰 fps: **25.5 → 21~23**(30분 창 기준, 단조 감소).
- 기제: `MaxPhysicsDeltaTime` 1/30 클램프([[project_slomo_physics_dt_clamp]]) — 프레임 dt(40→47 ms)가 클램프(33 ms)보다 커지면 물리는 월드 시간의 83%→70%만 진행한다. 물리 기반(UGV Chaos·드론 강체)이 11~12%, 캐릭터 이동인 보병이 5.7% 느려진 비율 차이도 이 설명과 맞는다.
- fps 가 왜 내려가는지는 **미확정**. GAS 에러(`SendGameplayEventToActor: Invalid ability system component`)가 10,616건이지만 시간당 700~900 으로 **일정**해서 누수 신호는 아니다. → 다음 실행을 위해 `cycles.csv` 에 `avg_fps`·`actors`·`projectiles` 열 추가(코드 완료, 빌드 대기). 액터 수가 사이클마다 늘면 누수, 평평한데 fps 만 내려가면 GPU/드라이버·에디터 쪽.

### 7.2 사다리 실측 — 값 조정 1건

- `AllyClearZone2`(Stand+60)가 **38/85** 로 너무 자주 걸렸다. 그런데 그 38사이클의 **마지막 사살자는 33회가 트럭(3분대)** 이고 2분대가 마지막인 경우는 **5회뿐** — 즉 대부분은 "2분대 교착"이 아니라 그냥 3차 교전이 60초보다 오래 걸린 정상 사이클이었다(사다리 없는 사이클의 Stand→완료는 최대 58 s). → **150 s 로 올림**(DT 저장). 3분대용 `Squad3Expose`(120)가 먼저 받고, 그래도 안 끝나면 아군 소탕(150), 마지막이 `PruneAll`(Expose+90 = 210).
- `Squad3Expose` 10회, 발동 뒤 완료까지 **중앙 6 s**(최대 27) — 킬존이 정확히 의도대로 동작.
- `Squad1LateWithdraw`/`Run` 은 매 사이클 발동하지만 74회는 대상 0명(1분대가 이미 전멸) — 무해한 no-op, 필요할 때(21회) 1~2명을 움직였다.
- **로그 구멍**: `IssueSquadOrder` 사다리는 `timeouts`/`pruned` 에 안 잡혀서 85사이클이 전부 `Complete` 로 보였다(실제로는 38사이클이 사다리를 탔다). → 행에 `bFailSafeStep` 플래그 + `assists` 열 신설, `CompleteAssisted` 판정에 포함(코드 완료, 빌드 뒤 DT 5행에 체크).

### 7.3 새 열 첫 실행 + `AllyClearZone2` 150 검증 (2026-09-23 07:39~08:27, 5사이클)

빌드 뒤 `2026.09.23-07.39.00_New_kadex_0811`. 새 열이 나왔고(`assists`/`timeouts`/`pruned`/`avg_fps`/`actors`/`projectiles`), `bFailSafeStep` 은 아직 DT 에 없어 `assists=0`(예상대로) → 이 실행 뒤 DT 5행에 체크·저장(08:28).

| 항목 | 값 | 의미 |
|---|---|---|
| `actors` | **794**(5사이클 내내, 마지막만 795·798) | **액터 누수 없음** — 85사이클 뒤 fps 하락은 액터 증가가 원인이 아니다 |
| `projectiles` | **142** 고정 | 풀링 정상 |
| `avg_fps` | **30.4~33.6** | 12시간 실행의 22~26 과 비교 — **에디터 세션이 길어질수록 내려간다**(재시작·액터가 아니라 세션 수명). 다음 확인 대상은 PIE 세션/에디터 쪽 누적 |
| `AllyClearZone2`(150) | 5사이클 중 **2회**, 둘 다 진짜 교착 | C01: 트럭이 +420 에 3분대를 끝낸 뒤 **138 s 정지** → AllyClear(+529.8) → 아군이 `Hostile_5`(1분대)·`Hostile_7`·`Hostile_10`(2분대)를 +558~571 에 사살하고 완료. C04 도 동일(마지막 사살 `Friendly_4`). **2분대 사다리가 설계대로, 그리고 아군이 실제로 끝냈다** |
| `Squad3Expose`(120) | 같은 2사이클 | 이번엔 Expose 만으로는 안 끝났다(그 뒤 72~76 s) — 남은 게 3분대가 아니라 1·2분대였기 때문. 사다리 순서(Expose 120 → AllyClear 150)가 맞게 작동 |

**`bFailSafeStep` 첫 실행(08:29, 3사이클)**: `assists` 열은 나왔지만 **3사이클 모두 `assists=2` / `CompleteAssisted`** — `Squad1LateWithdraw`/`Run` 이 대상 0명이어도 "발동"만으로 세졌기 때문. 신호가 죽으므로 **카운트 지점을 "명령이 실제로 대상을 가졌을 때"로 이동**(`ExecuteSquadOrder` → `IssueSquadOrderSpec` 반환값 > 0 이면 `Monitor->OnAssist(라벨, 인원)`; `KillSquads` 는 기존 `pruned` 로 별도). 코드 완료·빌드 대기. ⚠ 이때 소스가 read-only 로 돌아가 있었다(P4 체크아웃 필요) — 다른 세션이 제출/리버트한 듯.

## 8. 클라이언트 로그 (2026-09-23, 코드 완료·빌드 대기)

계기: 2-PC(이 PC = 자체방호 **클라이언트**, 호스트 = 다른 PC) 플레이에서 사용자가 "적군이 총을 별로 안 쏘는 것 같다, 특히 이동형 지휘소와 싸울 때". 확인해 보니 **클라이언트에는 흐름 로그가 아예 안 남고 있었다**(`OnWorldBeginPlay` 가 `NM_Client` 면 리턴) — 그 판의 로그는 호스트에만 있다.

클라 엔진 로그(687줄)에서 건진 것:
- 적군 15명 액터는 클라에도 **있다**(레벨 배치라 `bNetLoadOnClient`). 문제는 `BP_Soldier_Hostile.NetCullDistanceSquared = 225,000,000` = **150 m** — 시점에서 150 m 밖 병사는 업데이트가 끊기고 사격 멀티캐스트(Unreliable)도 안 온다. 1차 전투지는 트럭에서 ~600 m, 2차는 ~250 m.
- 클라에서 **병사 총격 이펙트가 처음 재생된 시각이 세션 +8분**(`NS_MuzzleFlash`/`NS_Rifle_Tracer` 첫 컴파일). 그 전까지 클라에선 병사 사격 코스메틱이 0회 — 위 150 m 컷과 일치.
- 시나리오 설계상으로도 3차 구간은 원래 조용하다: `EnemyFleeToZone3`(ReturnFireOnly) → `Squad3Run`(BreakContact **HoldFire**) → `ExcludeFleeingEnemies`(SetTargetable false — 아군·UGV 가 안 쏘니 응사 조건도 안 열림) → 트럭이 80 m 안에서 쏴야 `Squad3Stand` 가 걸려 Free 로 바뀐다. 실측 Flee3→Stand **126~151 s**.
- 무관: `NS_MuzzleFlash_01 찾기 실패` 경고는 구 UGV BP(`BP_UGV_Vehicle_Genesis`/`BP_UGVFromTank`)의 죽은 경로 참조.

그래서 **클라이언트 모드**를 넣었다(`bClientMode`, 폴더 `…_client`). 적는 것은 §2.8 표 참고 — 사격 멀티캐스트 수신·복제 정지(거리 포함)·사망 코스메틱·차량 복제값(탄약 감소로 발사 판정)·드론(클라가 시뮬 주체)·사이클 경계(재시작 멀티캐스트 `RunLocalRestartBegin/End` 훅). 서버 로그와 벽시계로 대조하면 다음 두 가지가 갈린다:
- 서버에 `첫 사격` 이 있고 클라에 `사격 수신` 이 없다 → **복제/릴리번시 문제**(그 줄의 `시점까지 Nm` 이 컷 거리를 준다).
- 서버에도 사격이 없다 → **AI/시나리오 문제**.

구현 메모: 훅 클래스는 `UScenarioMonitorDeathHook` → **`UScenarioMonitorSoldierHook`**(사망 + 사격 수신 둘 다). 클라 폴링은 분대 배정(`Assignment`)이 복제되지 않으므로 작업 전이를 안 보고 위치·사격만 본다. 클라에는 `AUGVAIController` 가 없어 자율주행 줄이 없다.

### 8.1 클라 로그 첫 실전 (2026-09-23 11:14, 2-PC 1사이클 598 s) — **릴리번시 가설 기각**

| 열 | 값 |
|---|---|
| `shots_seen` / `shooters_seen` | **3,925 발 / 39 명**(병사 40 명 중 39 명) |
| `deaths_seen` | 15 |
| `frozen_soldiers` | 117 |
| `avg_fps` | **18.0**(같은 시각 호스트 30~33) |
| actors / projectiles | 754 / 149 (호스트 794~798 — 차이 ≈ 서버 전용 AI 컨트롤러 40) |

- **클라는 거의 모든 병사의 사격 멀티캐스트를 받고 있었다.** 첫 수신 거리도 323~629 m(중앙 355)로 150 m 컷 밖에서 멀쩡히 온다 → §8 의 "NetCullDistance 150 m 때문에 못 본다" 가설은 **기각**. (Unreliable 멀티캐스트라도 채널이 열려 있으면 온다.)
- 남은 후보는 ① 클라 18 fps(총구화염·트레이서는 1~2프레임이라 눈에 덜 띈다) ② 실제 AI 문제 — **②가 정답이었다**(8.2).

로그 자체의 결함 2건(다음에 고칠 것):
1. **`frozen_soldiers` 117 은 대부분 오탐** — "15 s 위치 불변 = 복제 끊김" 인데 SoldierLab 병사는 엄폐 사격 중 실제로 그만큼 제자리다. 복제 시각 기반으로 바꾸거나 "거리 200 m 이상일 때만" 조건을 붙여야 신호가 된다.
2. **사이클 1 의 시점(View Target)이 `PlayerController_0`** — 월드 시작 때는 폰이 없어 PC 자신이 시점이라 초반 거리값이 트럭 기준이 아니다(사이클 2 는 `BP_TitanTruck4`). 시점이 바뀔 때 한 줄 남기게 할 것.

### 8.2 진짜 원인 — 병사가 차량을 표적으로 제대로 못 봤다 (다른 세션 수정, CL 510)

사용자 증상 "3분대가 이동형 지휘소와 싸울 때 총을 많이 안 쏜다"의 절반은 **설계된 침묵 구간**(§8.3), 나머지 절반은 **버그**였다. 2026-09-23 12:28 CL 510 "전투 ai ugv/truck 도 정상적으로 인식하게 수정 / 드론 3분대 트래킹 관련 수정"(`SoldierEngagement.h/.cpp` · `SoldierCover.cpp` · `SoldierIdentity.h` · `DronePawn` · `SoldierLabBridgeSubsystem`):

- **사선 판정** — 차량은 `WorldDynamic` 이라 총알을 막는데 조준점이 **차체 한가운데**다. 트럭을 정면에서 쏘면 3 m 앞 범퍼에 맞는 것으로 계산돼 "막혔다"로 읽혔고, 그래서 **사격 구멍이 영영 계획되지 않았다**(`IsShotBlockedByWorld` 가 표적 액터 자체에 맞는 것은 도달로 치게 수정).
- **엄폐 가격** — 위협이 차량이면 기록점(차체 중앙)과 실제로 쏘는 것(지붕 포탑, 트럭은 1.5 m 위)의 높이가 달라서, 지붕과 차체 사이 벽이 전부 "엄폐"로 읽히고 "여기서 응사 가능한가" 프로브가 **차량 콜리전 안쪽에서** 쏘아졌다. 표적의 형상(자기 눈점 − 자기 표적점)으로 높이 단차를 얹도록 수정.

즉 3분대가 트럭 앞에서 Free 였는데도 조용했던 것은 ROE 가 아니라 "쏠 수 있는 자리가 하나도 없다"는 판정이었다.

### 8.3 3분대는 설계상 130~180 s 동안 조용하다 (버그 아님)

`Flee3` → `Squad3Stand` 사이(실측 130~180 s)는 의도된 침묵 구간이고, 두 겹으로 막혀 있다:
1. `Squad3Run`(Flee3 +6 s)의 **`BreakContact` 가 ROE 를 HoldFire 로 덮는다** — 엄폐 가격 0, dwell 없음, 서서 스프린트. 360 m 를 "도주"로 보이게 하려고 만든 동사.
2. `ExcludeFleeingEnemies`(Flee3 +4 s)의 `SetTargetable(false)` 로 **아군·UGV 가 3분대를 안 쏘니** `ReturnFireOnly` 의 조건("3 s 안에 나를 쏜 접촉")도 안 열린다. 트럭만 예외(`bRespectEnemyTargetingExclusion=false` — 도망쳐 오는 분대와 싸우는 게 트럭 역할).

`Squad3Stand`(트럭이 80 m 안에서 사격, 또는 Flee3 +240 s 타임아웃)가 걸리면 `Occupy` 가 **구역 동사라 배정을 통째로 초기화**하므로 `bBreakContact` 와 `SetTargetable(false)` 가 둘 다 풀리고 Free/0.8 이 된다. 이 구간을 바꾸려면 손댈 곳은 `Squad3Run` 의 ROE · `ExcludeFleeingEnemies` 유무 · `Squad3Stand` 의 80 m 셋뿐이다(전부 DT 값).

## 9. 8.7시간 2-PC 무인 실행 (2026-09-23 17:16 ~ 09-24 01:59, 59사이클)

호스트 `2026.09.23-17.16.34_New_kadex_0811` · 클라 `2026.09.23-17.16.47_..._client`.

| | 호스트 | 클라 |
|---|---|---|
| 사이클 | **58 완주 + 1 중단**, 452 / **515** / 672 s | 59 |
| 타임아웃 · 아군 사망 | **0 · 0** | — |
| 사다리(`assists`) | 33건 / **20사이클**(나머지 38 은 자연 완주) | — |
| `AllyClearZone2` | **2회**(60→150 s 조정이 정확히 먹혔다 — 이전 38/85) | — |
| `Squad3Expose` / `PruneAll` | 11 / 1 | — |
| 재시작 | 1238~1288 ms, 59회 평평 | — |
| `avg_fps` | 26.2 → 26.0 (**하락 없음**) | 32.5 → 29.6 |
| 사격·사망 수신 | — | 매 사이클 **37~40명 전원**, 사망 **15/15** |

**전투 수정(CL 510) 확인** — 클라가 매 사이클 사실상 전원의 사격을 받고 사망 15명을 다 본다. §8 의 "3분대가 안 쏜다" 는 재현되지 않는다. 20분 사이클 fail-safe 는 한 번도 안 걸렸다.

### 9.1 실제 누수 — RCWS 도탄 액터 (고침, 2026-09-28)

`projectiles` 열이 **59사이클 내내 단조 증가**하고 `actors` 가 1:1 로 따라갔다(풀이 피크에 닿은 것이면 평평해져야 한다):

| | 시작 → 끝 | 사이클당 |
|---|---|---|
| 호스트 | 141 → **193** | +0.9 |
| 클라 | 142 → **242** | +1.7 |

원인은 `URCWSFireControlComponent::Multicast_LaunchRicochet_Implementation` 의 `SpawnActor<ARCWSProjectile>` 였다. `ARCWSProjectile` 은 `InitialLifeSpan=0` 이고 수명이 끝나면 `Deactivate()`(숨김·콜리전/틱 끔)만 한다 — **풀 소속 탄의 계약**이다. 풀 밖에서 태어난 도탄본은 아무도 파괴하지 않아 숨은 채 영구히 남았고, 재시작의 `Park()` 루프도 풀 기준이라 못 치웠다. 도탄은 멀티캐스트라 모든 프로세스에서 스폰되므로 클라에서도(오히려 더 빨리) 샌다.

**SoldierLab 이 2026-09-21 에 `ASoldierProjectile` 로 같은 버그를 이미 고쳤다**(`AI/SoldierEngagement.cpp` 주석: *"a spawned round was never destroyed and each bounce leaked one"*). RCWS 만 남아 있었다.

수정: 도탄도 **일반 발사와 같은 로컬 풀**에서 꺼낸다 — 새 헬퍼 `AcquirePooledRound()`(`ProjectilePool[NextPoolIndex]` 라운드로빈)를 `Multicast_FireEffects` 와 `Multicast_LaunchRicochet` 이 **같은 커서로** 공유. 근거 두 가지: ① 도탄 클래스는 튕긴 탄의 `GetClass()` 라 풀 클래스와 같다(다르면 스폰 + `SetLifeSpan(10s)` 폴백 — 영구 잔류만은 없게) ② 프로세스 간 슬롯 정렬이 어긋나도 무해하다(두 멀티캐스트 다 Unreliable 이라 원래 보장이 없고, 투사체는 `bReplicates=false` 로컬 비주얼이다). 소비량 증가는 실측 사이클당 1~2발이라 64슬롯에 무의미하다.

같이 넣은 로그 개선 2건:
- `projectiles` 열 → **`rcws_rounds` / `soldier_rounds`** 분리(합계만으론 어느 쪽인지 몰라 코드를 뒤져야 했다). RCWS 는 차량당 고정 64, SoldierLab 은 상한까지 자라는 풀이라 정상 곡선이 다르다.
- 클라 `복제 정지` 판정에 **거리 200 m 이상** 조건 추가(오탐 ~100/사이클 → 실제 릴리번시 컷만).

### 9.2 남은 것
1. 빌드 → 2-PC 한 판 → `rcws_rounds` 가 **평평한지**(호스트 128 근처 고정) 확인. 평평하면 누수 종결.
2. `soldier_rounds` 곡선 — SoldierLab 풀이 상한까지 자라고 멈추는지(정상) 계속 미는지(또 다른 누수).
3. 클라 fps 32.5→29.6 이 누수 때문이었는지(수정 후에도 내려가면 다른 원인).

### 8.4 남은 것 (클라 로그)
1. 진행 중: **멀티플레이 n시간 테스트**(2026-09-23 13:32~, 클라 로그 `2026.09.23-13.32.00_New_kadex_0811_client`). 끝나면 호스트/클라 `cycles.csv`·`events.log` 를 벽시계로 대조.
2. 8.1 의 로그 결함 2건 수정.
3. 병사별 수신 발수를 사이클 끝에 덤프(40줄/사이클) — "누가 몇 발 쐈나" 를 클라만으로 보려면 필요.
4. 클라 18 fps — 호스트(30~33) 대비 낮다. §7.1 의 fps 하락과 같은 줄기인지 볼 것(클라는 물리를 안 도는데도 낮다).

### 7.4 남은 것
1. 다시 밤새 → `assists`/`CompleteAssisted` 비율로 사다리 개입률을 정확히 보고, `avg_fps` 추이로 fps 하락을 재확인(액터는 이미 무죄).
2. fps 하락 원인 좁히기 — 액터 수가 아니면 PIE 세션 누적(렌더 리소스·GC·에디터). `stat memory`/`obj list` 를 사이클 경계에서 찍는 것도 한 방법.
3. 아군 보병 기여가 낮다(12시간 18킬) — 다만 7.3 에서 보듯 **교착 해소 역할**은 확실하다. 페이싱 [C-164] 와 함께 볼 것.

## 6. 다음 단계 (그 뒤)

- **사용자 메모(2026-09-22)**: 스텝이 트리거 하나에만 의존하는 곳(단일 실패점)을 DT 전체에서 정리해 확인 — 위 사다리는 1·3분대 병목만 다뤘고, `UAVSpotted`(낙하산 관측), `UGVArriveZone1`(ActorStopped), `AllyEngage`(80 m), `CommandPostFire` 등도 같은 눈으로 볼 것. 각 행에 "이게 안 걸리면 무엇이 멈추나 / 대체 조건은" 표를 만드는 작업.
- 세부 fail-safe: UGV/드론 전복·땅 꺼짐 감지(`up.z`, 지면 트레이스) → 안전 위치 재스폰 / 분대 명령 미달성 타임아웃.
- 드론 2차 프레이밍이 zone 0 잔존까지 화각에 넣는 문제(W104) — UGV 강제 출발 뒤 눈에 띄면.
- 오프라인 리포트 스크립트(`ScenarioLogs/` → 사이클 표·이상 목록·기준선 편차).
- Chronicle 을 **사이클 단위로 녹화 분할**(재시작마다 새 파일, 이름에 사이클 번호) — 10시간 단일 파일의 용량/스크럽 문제 회피, 로그의 `C37 +215s` 로 바로 여는 구조(선수과제 3 과 연동).
- `[ScenarioRespawn]` 컴포넌트 델타 Warning 스팸(10사이클에 2,800줄) 정리 — 별건.
