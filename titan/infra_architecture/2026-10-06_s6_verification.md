# S6 검증 1차 — 로딩 화면 · S6 묶음 1·2·3 자동 검증

2026-10-06 / 부분 완료(기능 검증 일부 · 성능 A/B 보류) / 로딩 화면·S6 묶음 1·2·3 자동 검증 1차 — L1~L4·B1-3·B1-4·B3-1 통과, L8 RTSP 검정 유지 의문·-game 워밍업 생략 발견, 성능 측정은 GPU 급저하 현상으로 보류

> 대상 문서: 로딩 화면 [`../ui/2026-10-06_loading_screen_impl.md`](../ui/2026-10-06_loading_screen_impl.md) §7 · 묶음 1 [`2026-10-06_s6_fix_batch1.md`](2026-10-06_s6_fix_batch1.md) §4 · 묶음 2/3 Part B [`../soldier_ai_lab/ai/2026-10-06_s6_fix_batch2_game_thread.md`](../soldier_ai_lab/ai/2026-10-06_s6_fix_batch2_game_thread.md) J절·M.7 · 묶음 3 [`2026-10-06_s6_fix_batch3.md`](2026-10-06_s6_fix_batch3.md) §5 · 감사 [`2026-09-29_s6_code_audit.md`](2026-09-29_s6_code_audit.md).
> 신뢰도: **[A]** = 로그 줄·수치·file:line 근거 · **[B]** = 추정. 검증만 했고 **코드·에셋·ini는 바꾸지 않았다**(아래 "제안"은 전부 미적용).
> ID 규칙: L = 로딩 화면(§7 항목 대응), B1 = 묶음 1 §4, P = 묶음 2·3 성능 A/B, B3 = 묶음 3 §5, F = 이번에 발견한 현상.
> 로그 타임스탬프는 **UTC**(현지 = UTC+9). 예: 로그 `00:19:11` = 현지 09:19:11.

---

## 0. 한 장 요약

| ID | 내용 | 결과 | 근거(요약) |
|---|---|---|---|
| L1 | 데모 로그 순서(MinHold 30 / Timeout 60) + 자동사격이 게이트 해제 뒤 | ✅ 통과 [A] | R01: 연기 → 게이트 해제 → 데모 세팅 적용(같은 ms) → 자동사격 +1.0 s → 자동 시작 +3.0 s |
| L2 | 오버레이 중 콘솔 명령 5종 거부 | ✅ 통과 [A] | R02: 5종 모두 `… 거부 — 호스트 로딩 중` · 대기열 없음. `?Demo=0` 풀시스템은 미실시 |
| L3 | 로비 없이 직접 시작에서 게이트 해제 | ✅ 통과 [A] | -game R01/R02 · PIE#1 · PIE `Enabled=0` 전부 해제 |
| L4 | 준비 후 `titan.ScenarioRestart` 3회 | ✅ 통과 [A] | 재시작 #1~#3 완료 2113/1078/1126 ms, 거부 0줄 |
| L5 | 2-PC | ⏸ 미실시 | 같은 PC 2프로세스는 검증 중단 → 수동 |
| L6 | `ForceUnmet` 타임아웃 | ⏸ 미실시 | — |
| L7 | 종료 사유·시간·마지막 미충족 | ◐ 부분 [A] | 타임아웃 원인 = **프레임 조건(MaxFrameMs 60)**, 스트리밍/PSO 아님 |
| L8 | RTSP 메인뷰가 오버레이 동안 검정 | ❌ 의문(미통과) [A 수치 · B 원인] | 0~5.65 s YAVG 16(검정) → 이후 2.4 s YAVG≈68(검정 아님), 로딩은 그 뒤 ≈7 s 더 |
| L9 | 첫 발사 히치 Enabled 1 vs 0 | ◐ 부분 [A 수치 · B 해석] | PIE만, 1샘플씩: 69.2 ms vs 82.6 ms — 차이 미확정 |
| L10 | -game MoviePlayer | ◐ 부분 [A] | `LoadMap(…) — MoviePlayer 로딩 화면.` 확인, `UseMoviePlayer 0` 비교 미실시 |
| **F1** | -game 워밍업 이펙트 생략(경합) | ⚠ 발견 [A 로그 · B 메커니즘] | R01 `10s 동안 로컬 카메라가 없어 워밍업 이펙트 생략` / R02는 스폰 성공 |
| B1-1 | 데칼 캡 | ◐ 부분 [A] | 교전 중 220, 재시작 후 1. 피크 미측정 |
| B1-2 | Niagara 평탄 | ⏸ 미완 | 290 → 325(다른 사이클·시점) — 평탄 판정 불가 |
| B1 #4 | 투사체 풀 32 유지 | ✅ 통과 [A] | 교전 중 `BP_RifleProjectile_C` = 32 |
| B1-3 | RTSP 마운트 접속/해제 ×10 | ✅ 통과 [A] | 5 마운트 × 10 = 50/50 성공, `still-set AppSrc` 0 |
| B1-4 | `t.MaxFPS 10` 후 회복 | ✅ 통과 [A] | 21–24 → 10.0 → 20–24 fps, 멈춤·끊김 없음 |
| P1~P3 | 묶음 2·3 Perf A/B | ⏸ 보류 | R01 1회 시도 무효(교전 강도 변동) + F3 |
| B3-1 | 시나리오 로그 크래시 내성 | ✅ 통과 [A] | `events.log` 마지막 줄 = `MARK | 크래시직전` |
| B3-2 | 스냅샷 히치 DiskSync 0 vs 2 | ◐ 참고치 [A 수치 · B 해석] | GT 최대 63.4/147.8/93.4 → 27.8/53.3/26.1 ms (GPU 바운드 중) |
| B3-3 | 자체방호 로비→레벨 트래블 | ⏸ 미실시 | — |
| B3-4 | AmbientFX 첫 틱 히치 | ⏸ 미실시 | — |
| M1 | (M1 장면 측정) | ⏸ 미실시 | — |
| **F3** | GPU 급저하(이산 단계 점프) | ⚠ 발견 [A 수치 · B 원인] | GPU ≈28 ↔ ≈165–176 ↔ ≈385–455 ms, 같은 PIE 안에서 저절로 전환 |

---

## 1. 테스트 환경 · 자동화 방법

### 1-1. 환경 [A]

- PC: Intel Core Ultra 5 225, RAM 31 GB, NVIDIA RTX 5060 8 GB. **S0과 다른 PC·조건**(S0 PC 사양은 기록 없음) → **S0과의 절대 비교는 무효**.
- 테스트 내내 **에디터(PID 19636)가 켜져 있었다**(-game 실행도 그 옆에서). 에디터 유휴 상태에서 GPU 사용률 86% · VRAM 4.3 GB. Parsec · Chrome · Discord도 실행 중.
- 빌드: titan_example DLL 08:37 · SoldierLab DLL 08:36 > 최신 소스 08:32(`SoldierCover.cpp`) → 현재 빌드 확인.
- 로비 경로 없이 맵 직접 실행.

### 1-2. 실행 회차 (현지 시각)

| 회차 | 시각 | 방식 | 옵션 | 종료 |
|---|---|---|---|---|
| R01 | 09:18–09:31 | -game, PID 17464 | `New_kadex_0811?Listen?Axis=UGV?Demo=1` · `-ExecCmds="titan.Loading.MinHoldSeconds 30,titan.Loading.TimeoutSeconds 60"` | `debug crash`(B3-1) |
| R02 | 09:32–09:38 | -game, PID 12472 | R01과 동일 | quit |
| PIE#1 | 09:39–09:43 | 에디터 PIE(MCP StartPIE) | `titan.Loading.Enabled=1`(기본 MinHold) | — |
| PIE#2 | 09:43–09:55 | 에디터 PIE | `Enabled=0` — ≈1–2.5 fps 저하 지속 | — |
| (사용자 수동) | 09:55:52 · 56:46 · 57:35 · 58:02 | PIE 재시작 여러 번 | `Enabled=0` | 마지막 09:58:02 회차를 `Enabled=0` 표본으로 사용 |

- 종료 후 `titan.Loading.Enabled`를 1로 되돌리고 MCP로 재확인.

### 1-3. 콘솔 명령 자동화 — 키보드 주입 (재사용 가능)

unreal-mcp엔 콘솔 명령 툴이 없고 Remote Control도 꺼져 있어 PowerShell 키보드 주입으로 했다(`Saved/S6Verify/2026-10-06/tools/ue_console.ps1`).

안 되는 것 [A]:
- 문자 타이핑(SendInput / WM_CHAR)은 **한글 IME 때문에** 콘솔에 글자가 안 들어간다.
- 콘솔 키(VK_OEM_3)를 PostMessage로 보내거나 스캔코드 0으로 보내면 무시된다.

**동작하는 프로토콜**:
1. AppActivate(+ALT 트릭)로 대상 창 활성화.
2. `keybd_event(0xC0, scan 0x29)`로 콘솔 키.
3. 명령을 클립보드에 넣고 Ctrl+V(keybd_event)로 붙여넣기.
4. Enter — 실행되고 콘솔이 닫힘.

- **안전장치**: 매 전송 전에 포그라운드 창이 대상 창인지 확인, 아니면 중단(에디터로 잘못 보내는 것 방지).
- 부작용: 사용자 클립보드를 덮어쓴다.
- 검증: Mark 명령 3/3 성공. 한글 인자(`크래시직전` 등)도 붙여넣기라 동작.
- **-game에선 실행한 명령이 로그에 `Cmd:`로 안 찍힌다**(에디터에선 찍힘) → 효과로 확인(Mark 줄, cvar `= "1"` 에코 등).
- -game 창은 1920x1080 창 모드라 클라이언트 영역이 화면 전체.
- R01 초반엔 이 방법이 아직 안 잡혀 L2 명령이 전달되지 않았다 → L2는 R02에서 다시 함.

---

## 2. ID별 결과

### L — 로딩 화면 (`../ui/2026-10-06_loading_screen_impl.md` §7)

#### L1 데모 로그 순서 — ✅ 통과 [A] (§7 1-①·1-③)

R01(MinHold 30 / Timeout 60), `R01/game.log`:

```
00:19:11.059 데모 세팅(지휘소 자동사격·자동 시작) 연기
00:19:11.270 대상 월드 판정: New_kadex_0811(리슨서버) — 로딩 오버레이 + 워밍업 시작
00:20:11.261 로딩 종료 — 사유=타임아웃 (60.1s, 연속 0프레임, 마지막 미충족=[프레임=90.6ms])
00:20:11.262 호스트 로딩 완료 — 시나리오 게이트 해제
00:20:11.262 연기했던 데모 세팅 적용
00:20:12.220 데모 자동사격: 이동형지휘소          (+1.0 s)
00:20:12.303 페이드인 완료
00:20:14.248 데모 자동 시작                       (+3.0 s) → 스텝 평가 시작
```

- 해제 전에 스텝·알림 없음. 자동사격은 해제 뒤에만.

#### L2 오버레이 중 콘솔 명령 거부 — ✅ 통과 [A] (§7 1-②)

R02, 오버레이 중 00:33:24–00:33:30. 5종 모두 `… 거부 — 호스트 로딩 중(완료 후 다시 입력). 대기열에 넣지 않음.`:

| 보낸 명령 | 로그의 거부 함수 |
|---|---|
| BeginEnemyContact 류 | `BeginEnemyContactScenario 거부` |
| `titan.ScenarioRestart` | `RequestScenarioRestart 거부` |
| `BeginAllyFormUpAndAdvance (X=0,Y=0,Z=0)` | `BeginAllyFormUp 거부` |
| `BeginEnemyFlee` | `BeginEnemyFleeToZone 거부` |
| `titan.SquadOrder Hostile * MoveTo 0` | `IssueSquadOrderSpec 거부` |

- 이후 로딩 정상 종료(00:33:35.620, 38.9 s).
- "페이드인 후 정상 시작"은 데모 자동 시작(`BeginEnemyContactScenario() 호출(콘솔 명령과 동일)`)으로 확인. **`?Demo=0` 풀시스템 회차는 안 했다.**

#### L3 직접 시작에서 게이트 해제 — ✅ 통과 [A] (§7 2)

- -game R01/R02: `대상 월드 판정: New_kadex_0811(리슨서버) — 로딩 오버레이 + 워밍업 시작` → 종료 사유 타임아웃(R01) / 정상(R02) → 게이트 해제.
- PIE#1(에디터, New_kadex_0811, MCP StartPIE): `로딩 종료 — 사유=타임아웃 (30.1s, 연속 0프레임, 마지막 미충족=[프레임=89.6ms])` + 게이트 해제.
- PIE `Enabled=0`: `대상 월드 UEDPIE_0_New_kadex_0811(단독) — titan.Loading.Enabled=0, 워밍업 없이 즉시 준비.` → 즉시 게이트 해제.

#### L4 재시작 3회 — ✅ 통과 [A] (§7 3)

R01, 준비 후 `titan.ScenarioRestart` 30 s 간격 3회:

| 회 | 시작 → 완료(페이드 포함) | 재시작 검증 |
|---|---|---|
| #1 | 2113 ms | 재스폰 40, 등록부 적 15 / 아군 25 |
| #2 | 1078 ms | 동일 |
| #3 | 1126 ms | 동일 |

- 매번 `데모 자동 시작` 재발생. 로그 전체에 **거부 줄 0**.

#### L5 2-PC — ⏸ 미실시 (§7 4)
같은 PC에서 프로세스 2개로 하는 검증은 중단. 실 2-PC는 수동 목록(§4).

#### L6 `ForceUnmet` — ⏸ 미실시 (§7 7)

#### L7 종료 사유·시간·마지막 미충족 — ◐ 부분 [A]

| 회차 | 사유 | 시간 | 마지막 미충족 | MinHold |
|---|---|---|---|---|
| R01 | 타임아웃 | 60.1 s | 프레임=90.6 ms | 30 |
| R02 | 정상 | 38.9 s | (직전) 프레임=60.8 ms | 30 |
| PIE#1 | 타임아웃 | 30.1 s | 프레임=89.6 ms | 기본 1 |

- 진행 로그상 스트리밍/PSO는 한 번도 미충족이 아니었다. R01은 프레임 [0]에서 `[비동기로딩]`만 나오고, 그 뒤로는 `[프레임=64~91ms]` 또는 `[없음]`뿐(예: `로딩 중 20.4s — 연속 2/30프레임, 미충족=[없음]` → `40.5s — 연속 0/30프레임, 미충족=[프레임=61.3ms]`).
- **결론 [A]: 타임아웃의 주원인은 프레임 시간 조건(`titan.Loading.MaxFrameMs` 60, `TitanLoadingScreenSubsystem.cpp:56`, 판정 `:687`)이지 `MaxStreamingWanting=0`이 아니다.** 이 환경(에디터 동시 실행 + GPU 저하, F3)에선 60 ms를 자주 넘어 30프레임 연속(`StableFrames`, `:51`)을 못 채운다. 다른 세션의 이전 PIE는 11~20 s에 정상 종료했다(loading_screen_impl §8-4).
- **제안(미변경)**: 프레임 조건을 연속 프레임 대신 창(window) 중앙값으로 판정, 또는 MaxFrameMs/StableFrames 완화 — **전시 PC/패키지 환경에서 재측정 후 결정**.

#### L8 RTSP 메인뷰 검정 — ❌ 의문·미통과 [A 수치 · B 원인] (§7 5)

R02, 오버레이 동안 ffmpeg로 `rtsp://127.0.0.1:8554/ugv/rcws` 8 s 수신(482프레임, 1920x1080 h264 60 fps), signalstats:
- 0–5.65 s: YAVG = **16**(검정, limited range).
- 이후 2.4 s: YAVG ≈ **68**(검정 아님).
- 로딩은 그로부터 약 7 s 뒤(00:33:35.6)에야 끝났다 → **오버레이 중인데 송출이 검정이 아니게 된 구간이 있다.**

원인 미확정:
- 같은 시간대에 L2 콘솔 명령을 보내고 있었지만, 이 명령들은 카메라를 안 건드린다(PC Exec 구현 확인 — `Server_*_Implementation` → 시나리오 서브시스템 호출뿐).
- `HoldLocalCamerasBlack`(`TitanLoadingScreenSubsystem.cpp:179-193`)이 로컬 PC(`GetLocalPlayer()!=nullptr`)에 매 틱 `SetManualCameraFade(1)`을 다시 건다.
- **재확인 필요 [B]**: 명령을 보내지 않고 로딩 전 구간을 녹화해 프레임 추출. 확인 포인트 — `UMainViewStreamComponent`의 캡쳐 지점이 카메라 페이드를 포함하는지, 뷰어(뷰 타겟/카메라)가 다른지 등.

#### L9 첫 발사 히치 Enabled 1 vs 0 — ◐ 부분 [A 수치 · B 해석] (§7 6)

PIE만. 첫 사격 ±5 s 안 최대 프레임:

| 조건 | 최대 프레임 | GT / GPU | 시점 | 중앙값 (그 창 / 앞 25 s) |
|---|---|---|---|---|
| Enabled=1 (PIE#1) | **69.2 ms** | 36.4 / 22.0 | +4.7 s | 27.8 / 26.0 |
| Enabled=0 (09:58 PIE) | **82.6 ms** | 33.2 / 31.9 | +1.9 s | 31.2 / 31.1 |

- 1샘플씩이고, Enabled=0 회차는 같은 세션 앞쪽에 GPU 저하 에피소드(F3)가 있었다 → **차이 확정 안 됨**. -game 비교는 미실시.

#### L10 -game MoviePlayer — ◐ 부분 [A] (§7 추가)

- -game R01/R02에서 `LoadMap(/Game/New_kadex_0811) — MoviePlayer 로딩 화면.` 확인.
- `titan.Loading.UseMoviePlayer 0` 비교는 미실시.

### B1 — 묶음 1 (`2026-10-06_s6_fix_batch1.md` §4)

| ID(§4 #) | 결과 | 근거 |
|---|---|---|
| B1-1 (#1 데칼 캡) | ◐ 부분 [A] | R01 1사이클 초반 교전(00:26:00) `obj list class=DecalComponent` = **220**. 전부 `WorldSettings.DecalComponent_N` 아래라 이름으로 병사/RCWS 분리 불가 — 합계 ≤400(200+200)은 성립. **피크는 미측정**. 재시작 3회 후(00:29:59) = **1**(재시작이 비움) |
| B1-2 (#2 Niagara 평탄) | ⏸ 미완 | `NiagaraComponent` 290(1사이클 교전, 00:26:02) → 325(3번째 재시작 직후, 00:29:57). 다른 사이클·시점이라 평탄 판정이 아님 — 같은 교전 안 주기 측정 필요 |
| #4 (투사체 풀) | ✅ 통과 [A] | 교전 중 `obj list class=BP_RifleProjectile_C` = **32**(프리필 32 유지) |
| B1-3 (#7 RTSP 반복 접속) | ✅ 통과 [A] | R02, `ugv/rcws` · `front_cctv` · `rear_cctv` · `left_cctv` · `right_cctv` 5 마운트 × ffprobe 접속/해제 10회 = **50/50 성공**, `media-configure found a still-set AppSrc` **0**, `LogRtspEncoder` Warning/Error 0, 프로세스 생존 |
| B1-4 (#8 fps 저하 회복) | ✅ 통과 [A] | R02, `/ugv/rcws` 연속 수신(벽시계 타임스탬프). `t.MaxFPS 10`(09:36:51) → `t.MaxFPS 0`(09:37:23). 도착률 21–24 fps → 제한 중 **10.0 fps**(최대 간격 0.28 s) → 원복 후 20–24 fps(최대 간격 0.19 s). 멈춤·끊김 없음 |

### P — 묶음 2·3 성능 A/B — ⏸ 보류

- R01(에디터 켜진 상태)에서 1회만 시도: Perf 전부 1 → 전부 0(`IdentityMap` 포함 6개 cvar, `= "0"` 에코로 확인) → 다시 1, 각각 `stat dumpave -num=120 -ms=0.1`.
- 세 덤프 사이 교전 강도가 크게 달랐다 — FrameTime 42.9 / 41.1 / 38.3 ms, Engagement Tick 0.23 / 1.20 / 0.63 ms → **토글 효과로 귀속 불가, 무효**. 파서 `tools/parse_dump.py`는 재사용 가능.
- 보류 이유: ① 에디터 동시 실행 ② **GPU 급저하 에피소드(F3)** — 원인이 풀리기 전엔 성능 수치가 의미 없다. P1~P3 · M1은 fps 문제 해결 후 깨끗한 환경에서 재실시. OPEN_ITEMS 후보(묶음 2 L-1/L-2, 묶음 3 M.8-1)는 **[C] 미측정 그대로**.
- 참고(A/B 아님, R01 덤프 1의 하위 스코프, 120프레임 평균 · 포함 시간 ms): Engagement: `CountTeammatesOn` 0.313 · `SelectTarget` 0.385 · `PickCandidate` 0.345 · Field: `ReportSighting` 0.259 · Field: `MarkClearAlongRay` 0.419 · Sight: `Candidates` 0.774 · Cover Tick 2.568 · AIBridge `TickBridge` 0.774. `ReportSighting GroundZ`는 이 문턱(0.1 ms)에서 따로 안 나왔다.

### B3 — 묶음 3 (`2026-10-06_s6_fix_batch3.md` §5)

#### B3-1 크래시 내성(§5 #3) — ✅ 통과 [A]

- R01: `titan.ScenarioLog.Mark 크래시직전` → `debug crash` → 프로세스 종료(`LogWindows: FPlatformMisc::RequestExit(1, LaunchWindowsStartup.ExceptionHandler)`, `Saved/Crashes` 생성).
- `Saved/ScenarioLogs/2026.10.06-09.19.11_New_kadex_0811/events.log`의 **마지막 줄**: `2026-10-06 09:31:39.498 | C04  +145.7s | MARK | 크래시직전`.

#### B3-2 스냅샷 히치(§5 #4) — ◐ 참고치 [A 수치 · B 해석]

R01 csvprofile 중 `titan.ScenarioLog.Snapshot`을 `DiskSyncSeconds 0`에서 3회, `2`에서 3회. 각 스냅샷 −1.0~+0.6 s 창의 GameThreadTime 최대:

| DiskSyncSeconds | 1회 | 2회 | 3회 |
|---|---|---|---|
| 0 (줄마다 싱크) | 63.4 ms | 147.8 ms | 93.4 ms |
| 2 (주기 싱크) | 27.8 ms | 53.3 ms | 26.1 ms |

- 단 캡쳐 동안 프레임이 GPU 바운드(중앙값 176.7 ms, GPU 165 / GT 16.8)라 프레임 시간 스파이크는 묻힌다 → **GT 기준 참고치**. 방향은 기대와 같다(감소).

#### B3-3 자체방호 로비→레벨 트래블(§5 #5) — ⏸ 미실시
#### B3-4 AmbientFX 첫 틱 히치(§5 #7) — ⏸ 미실시
#### M1 — ⏸ 미실시

---

## 3. 발견 · 이상 현상

### F1. -game 워밍업 이펙트 생략 (경합) [A 로그 · B 메커니즘]

- R01: `[LoadingScreen] 10s 동안 로컬 카메라가 없어 워밍업 이펙트 생략.`(00:19:22.016). 워밍업 시작(00:19:11.27)부터 이 줄까지 로그 프레임 번호가 계속 **[0]** — 약 10.7 s 동안 게임 프레임이 진행되지 않았다.
- `NoCameraSkipSeconds`(10 s, `TitanLoadingScreenSubsystem.cpp:70`)는 **실시간**(`FPlatformTime::Seconds() - PhaseStartRealSeconds`, `TickWarmup` `:620-622`)으로 잰다 → 첫 프레임이 돌기 전에 폴백 시간이 만료됐다 [B — 첫 `TickWarmup` 호출이 10 s 뒤였다는 해석].
- R02는 성공: `워밍업 카메라 …` → `워밍업 이펙트 스폰 — Niagara 23, 데칼 1 @ V(X=58768,Y=13500,Z=-3741)`(00:33:05.867, 프레임 [1], 시작 후 9.1 s) → `3프레임 후 제거`.
- 즉 -game에서 워밍업이 되느냐는 **첫 프레임까지 걸린 시간**에 달려 있다(9.1 s면 됨, 10 s 넘으면 생략).
- **제안(미변경)**: NoCameraSkipSeconds를 첫 틱 프레임부터 재거나 프레임 수로 판정. 위치: `TickWarmup`의 10 s 분기(`TitanLoadingScreenSubsystem.cpp:639`, `:648-651` — 로그 문자열 `%.0fs 동안 로컬 카메라가 없어 워밍업 이펙트 생략`).

### F2. L7 · L8 — 위 §2 참고
- L7: 타임아웃은 프레임 조건 탓(제안 있음).
- L8: 오버레이 중 RTSP 송출이 검정에서 풀리는 구간 — 원인 미확정.

### F3. GPU 급저하 (모든 측정에 영향) [A 수치 · B 원인]

- CSV `PIE2-6_enabled0_slowdown.csv`(09:43:37 – 10:03:45, 9387프레임)를 20 s 창으로: 전 구간 GPU 바운드(RenderThread ≈ GPUTime, GameThread 30 ms).
- GPU 시간이 **이산 단계로 점프**: ≈28 ms ↔ ≈165–176 ms ↔ ≈385–455 ms.
- **같은 PIE 안에서도 저절로 전환**된다 — 예: 946→966 s 구간 32 → 166 ms, 1127→1147 s 구간 172 → 31 ms, PIE 재시작 없음. 사용자의 "또 느려짐/또 빨라짐" 보고와 시점이 맞는다.
- VRAM은 느릴 때/빠를 때 거의 같다(6156/6509 MB vs 6160/6533 MB), SystemUsedMB도 평탄 → **단순 VRAM 예산 초과로는 설명이 약하다**.
- R01 -game 스냅샷 창도 같은 시그니처(GPU 165 ms, GT 16.8). PIE#1은 로딩 후 26–33 ms.
- 에디터 프로세스: 커밋 24.8 GB vs 워킹셋 8.0 GB(10:0x), Pages Input/sec 2 → 그 시점 하드 페이징 없음. 에디터 백그라운드 스로틀 `bThrottleCPUWhenNotForeground=False` → 스로틀 원인 아님.
- 사용자 진술: 리눅스 패키지에선 전혀 없다, PIE 재시작하면 한동안 낫다.
- **미해결 — 별도 조사**. 다음 단계: 1 s 간격 GPU 클럭/전력/스로틀 사유/인코더 로그 + 느린 상태에서 `ProfileGPU`로 어느 패스가 커지는지.
- → 후속: 원인 범위 좁힘·회피 적용 — [2026-10-06_pie_gpu_stall_skin_cache.md](2026-10-06_pie_gpu_stall_skin_cache.md)

---

## 4. 미완 · 불가 · 수동 목록

**사용자 수동만 가능**:
- 실 2-PC(LIG 구성) 로딩 + 늦은 클라 접속, LIG 수신 화면 확인.
- 로비 UI 버튼 → 페이드아웃 → 로딩 화면(육안).
- 쿡 패키지(Windows/Linux)에서 첫 발사 히치·성능 재측정.

**자동화 실패/미실시** (다음 회차 후보):
- L5(2-PC) · L6(`ForceUnmet`) · L8 재확인(명령 없이 로딩 전 구간 녹화) · L10 `UseMoviePlayer 0` 비교.
- B1-1 피크 · B1-2 같은 교전 내 평탄 확인.
- P1~P3 · M1(F3 해결 후 깨끗한 환경).
- B3-3(자체방호 트래블) · B3-4(AmbientFX 첫 틱).
- L9 -game 비교 · L2 `?Demo=0` 회차.

---

## 5. 원본 데이터 경로

모두 `C:\working\works\kadex\titan_example\Saved\S6Verify\2026-10-06\` 아래.

| 파일 | 용도 |
|---|---|
| `R01/game.log` | L1 · L3 · L4 · L7 · B1-1 · B1-2 · #4 · P 시도 · B3-1 · B3-2 |
| `R01/snapshot_ab.csv` | B3-2 csvprofile |
| `R02/game.log` | L2 · L7 · L8 · B1-3 · B1-4 |
| `R02/yavg_loading.txt`, `R02/ffmpeg_loading.txt` | L8 |
| `R02/b1-3_rtsp_reconnect.txt` | B1-3 |
| `R02/b1-4_ffmpeg_showinfo.txt` | B1-4 |
| `PIE1_enabled1.csv` | L9 Enabled=1 |
| `PIE2-6_enabled0_slowdown.csv` | L9 Enabled=0 + F3 |
| `editor_PIE_titan_example.log` | 에디터 PIE 로그 사본 |
| `tools/ue_console.ps1` · `tools/parse_dump.py` · `tools/shot_bottom.ps1` | 콘솔 주입 · dumpave 파서 · 화면 하단 캡쳐 |

시나리오 로그(`Saved/ScenarioLogs/`): `2026.10.06-09.19.11_New_kadex_0811`(R01) · `…09.32.56…`(R02) · `…09.39.29…`(PIE#1) · `…09.43.28…`(PIE#2) · `…09.58.02…`(Enabled=0 표본).

---

## 6. OPEN_ITEMS 등록 후보 · 후속

번호는 매기지 않는다. SoldierLab 몫은 `../soldier_ai_lab/OPEN_ITEMS.md` 끝 "등록 후보" 절에 한 줄 추가함. titan 쪽은 번호 체계가 없어 여기 기록.

**titan 쪽 후속**:
1. **F3 GPU 급저하 조사** — 모든 성능 측정의 선행 조건. 다음 단계는 §3 F3.
2. **F1 -game 워밍업 생략** — NoCameraSkipSeconds 기준 변경 제안(미적용).
3. **L7 로딩 타임아웃 = 프레임 조건** — 판정 방식/문턱 조정 제안, 전시 PC·패키지 재측정 후 결정.
4. **L8 RTSP 메인뷰 오버레이 중 검정 해제** — 명령 없이 재녹화로 원인 확정.
5. 미실시 항목(§4) 재실시.

**SoldierLab 쪽 등록 후보**:
- 묶음 2 L-1 · 묶음 3 M.8-1(Perf A/B)은 이번 회차에서 **측정 무효 · [C] 유지** — F3 해결 후 재측정. 새 후보는 없다(상태 갱신만).
