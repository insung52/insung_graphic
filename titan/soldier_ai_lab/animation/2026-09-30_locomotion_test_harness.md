# 로코모션 자동 테스터 + 발 미끄러짐·Loop 불안정 해결 (아군)

2026-09-30~10-01 / 진행중(아군·적군 적용 완료 · 접지 커브 재생성 대기 · 디자이너 이음새 수정 대기) / 자동 테스터·기록기로 원인을 측정하고, Loop 클립 루트 선형화 + MM 질의 궤적 4방향 스냅 + Starts/Pivots 편향으로 아군 조준 이동 24케이스 전부 Loop 100%·전환 112/112 Loop 도달을 만들었다.

> **요약(5.4~5.10)** — 원인 3개: ① 일부 Loop 클립 루트 속도가 보폭 안에서 ±53~74% 출렁임(→ `SoldierRootSpeedTool` 선형화, 8클립) ② 4방향 DB의 대각선 공백(→ `SnapTrajectoryToCardinal`, 앞 대각선 → 옆걸음 · 뒤 대각선 → 후진) ③ Start/Pivot 정속 구간이 Loop 대용(→ Starts·Pivots DB `baseCostBias` 0.1). 부차: 플레이어 속도 ×0.85(뛰기 선택 악화, [W125]).

관련: `animation/2026-09-30_diagonal_aim_stop_selection.md`(대각선 Stop 선택 — [W124]) · `ai/2026-09-30_ai_real_pitch_and_aim_smoothing.md`(기록기 `SoldierAimTrace` 원형) · `design/2026-09-01_architecture.md` 5.1·5.5.2절(MM 채택 이유, "4방향 + 오리엔테이션 워핑" 계획).

---

## 1. 문제 정의 [A · 사용자]

걷기/뛰기, 총듦/총내림 등에서 WASD 이동 시:
1. 같은 방향으로 유지해 안정돼도 **Loop가 안정적으로 뜨지 않는 방향·자세 조합이 많다.**
2. 방향을 바꿀 때(예: W → WA) **발이 꼬이며 움직이는** 경우가 있다. 1번이 나는 조합은 2번도 거의 나고, 2번만 단독으로 나는 전환도 있다.

범위(사용자 결정): **지금 있는 애셋만 사용** — Lyra 원본 재반입, 대각선 클립(`MF_Rifle_*`) 사용은 이번 범위 밖.

## 2. 맥락 — 왜 모션매칭이었나, 어디가 비었나 [A · 설계 문서]

- MM 채택(설계 5.1): 옛 `ABP_Ally_kadex2`(블렌드스페이스+상태머신)의 버그가 모두 "속도→애니 파라미터 수동 매핑"에서 나옴 · UE5.8 PoseSearch 정식 + GASP 레퍼런스 · TLOU2 품질. 레이어 블렌드(Lyra/ALS식)는 이미 실패로 배제.
- 대각선 계획(5.5.2): "8방향 불필요 — 소수 풀바디 클립 + Orientation Warping".
- **빈틈**: MM은 워핑이 뒤에서 각도를 메운다는 걸 모른다 — 45°에서 4방향 Loop 모두 궤적 비용이 높아 Stop/Start/Pivot이 이긴다. GASP는 방향 클립 18개로, Lyra는 상태머신의 명시적 4방향 선택(여유폭)+워핑으로 이를 피한다.
- 가설(측정 후 검증): **MM 질의 궤적을 가장 가까운 4방향으로 스냅해 넣고, 잔여 각도는 워핑이 메우게** 한다(Lyra의 방향 선택을 MM 앞단에).

## 3. 만든 것 (코드 — 헤더 신규, 에디터 재빌드 필요)

### 3.1 자동 테스터 `Source/SoldierLab/Debug/SoldierLocomotionTest.{h,cpp}`

`USoldierLocomotionTestSubsystem`(월드 서브시스템). 콘솔: **`SoldierLab.Test.Locomotion <quick|steady|turns|all|stop>`**.

- 로컬 플레이어 폰을 조종: 컨트롤 yaw 고정(시작 시 값), `CharacterInputState.WantsToAim/WantsToWalk`(작성 이름으로 찾음 — 실제 필드명은 GUID 접미사), `WantsToSprint=false`, `StanceAxis` 램프(`StanceInput` 0), `AddMovementInput`(방향 = 고정 yaw + 45°×k: 0 W · 1 WD · 2 D · 3 SD · 4 S · 5 SA · 6 A · 7 WA).
- 안정 케이스 `S_<aim|low>_<walk|jog>_<stand|crouch>_<dir>`: rest 1.5 → settle 1.2 → **measure 3.0** → stop 1.2 s.
- 전환 케이스 `T_<구성>_<A>><B>`: 8방향 모든 순서쌍 56개를 연속 — 각 쌍 A 1.5 s(`pre`) → B 1.5 s(**`post`**).
- 프리셋: `quick` = 조준 3구성(걷기·뛰기·앉아걷기) 안정 + 조준 2구성(걷기·뛰기 서기) 전환 ≈ 8분 · `all` = 총내림 포함 안정 6구성 + 전환 5구성 ≈ 20분.
- 시간 cvar: `SoldierLab.Test.Loco.{RestSeconds,SettleSeconds,MeasureSeconds,StopSeconds,TurnHoldSeconds,StanceRate}`.
- 기록기를 자동으로 켜고(필터 = 플레이어 병사 이름, 새 세션 폴더) 끝나면 끈다. **적 AI 없는 레벨에서 돌릴 것.**

### 3.2 기록기 확장 `Debug/SoldierAimTrace.cpp`

새 열: `test.case/phase/dirA/dirB/aim/walk/stance` · `input.xyz` · `input.yawVsFacing` · `travel.yawVsFacing` · `speed2D` · 본 `foot_l/r` · `ball_l/r`(월드) · 커브 `curve.contact_l/r` · `curve.Enable_Warping` · `curve.MoveData_Speed`. (MM 선택 DB/클립/블렌드 스택, ABP 변수 전부는 기존대로.)

### 3.3 분석 `titan/soldier_ai_lab/tools/locomotion_report.py`

`python locomotion_report.py [세션폴더|csv] [--csv 요약.csv]` — 인자 없으면 최신 세션.
- 안정(measure): **loop%**(선택 DB가 Loops) · DB 구성 · 클립 교체/초 · **접지 발 미끄러짐**(contact > 0.5인 동안 발 본 수평 속도, cm/s 중앙·p90) · **발 교차%**(액터 기준 왼발이 오른발보다 오른쪽) · 이동-몸 각도 · 속도.
- 전환(post): **Loop 도달 시간** · 등장 클립 수 · 미끄러짐 최대/초반 0.6 s 평균 · 발 교차 프레임.

미포함(필요 시 2차): DB별 최선 비용(포즈 히스토리 연동 필요).

## 4. 첫 실행 — `Saved/AimTrace/20260930_170622` (`quick`, 507 s) [A]

- **fps 문제**: 처음 30초 51 fps, 이후 **정확히 3.0 fps**(dt 0.333) — 사용자 자리 비움 시점. 에디터 `bThrottleCPUWhenNotForeground`는 이미 false → **PC 화면 잠금/모니터 절전으로 렌더가 멈춘 것**으로 판단. 측정 중엔 PC를 깨어 있게 둘 것. 리포트는 dt > 0.05 프레임을 속도 지표에서 뺀다.
- 정상 구간(조준 걷기 서기 5케이스)만 유효:
  | case | loop% | 주 DB | rest spd | planted% |
  |---|---:|---|---:|---:|
  | W | 76 | Loops 75 · **Pivots 24** | **31** | 0 |
  | WD | 64 | Loops 63 · **Pivots 36** | 24 | 1 |
  | D | 100 | Loops | 9 | 0 |
  | SD | 53 | Loops 52 · **Pivots 47** | 12 | 0 |
  → 대각선에서 Stop이 아니라 **Pivot이 섞여 든다**(편향 조정 이후). 직진 W에도 Pivot 24%.
- ★ **접지 커브 결함 [A]**: ABP `RemapCurves_0`가 `contact_l/r = (1 − contact) × 100`(GASP 원본 방식, FootPlacement 입력)으로 변환한다 — 런타임 0 = 접지, 100 = 공중. 정상 fps·블렌드 가중치 100% 프레임에서: `ALLY_MM_Rifle_Idle_ADS` 양발 0(=접지 ✅), **`Walk_Fwd`·`Walk_Right`·`Walk_Fwd_Start`·`Walk_Right_Start` 양발 100 고정 = 원본 접지 커브가 걷는 내내 0** — 발이 한 번도 "접지"로 표시되지 않는다 → **FootPlacement가 걷는 동안 발을 고정하지 못한다.** 클립엔 `contact_l_CURVE_CONTROL`(컨트롤 릭 커브 컨트롤)이 있어 **디자이너 Sequencer 재베이크가 커브를 0으로 덮어쓴 것**이 유력(미확정 — 원본 커브 직접 판독은 MCP 불가). 적군 클립은 다음 측정에서 확인.
- 리포트 보강: 접지 판정을 리맵 기준(≤ 50)으로 · **커브와 무관한 `rest spd`**(0.5초 창마다 각 발의 최소 수평 속도의 중앙값 — 제대로 걸으면 ≈ 0) · `planted%`.

### 4.1 무인 실행 지원 (재빌드 후)

MCP엔 콘솔 명령 입력 도구가 없다 → `Saved/LocoTest.request` 파일에 스케줄 이름을 쓰면 플레이어 폰이 생길 때 테스터가 자동 시작하고 파일을 지운다(1초마다 확인). 끝나면 `Saved/LocoTest.done`에 결과. MCP `StartPIE`/`StopPIE`와 합쳐 무인 반복 가능.

## 5. 기준선 — `Saved/AimTrace/20260930_185958` (`quick`, 60 fps, 30,382행, 324/324 스텝) [A]

레벨 `L_Basic` 세팅: WorldSettings GameMode = `GM_SoldierLab`(기본 폰 `BP_Soldier_Friendly`) · `PlayerStart_2` (0,0,120) · 바닥 Plane 스케일 400(±200 m). 기록 필터는 이름 대신 **`@test`**(테스터가 조종 중인 폰 객체로 판정 — 스폰 후 라벨이 `BP_Soldier_Friendly_C_0`→`BP_Soldier_Friendly0`로 바뀌어 이름 필터가 빈 기록을 냄).

### 5.1 안정(measure 3 s) — Loop 비율

| 방향 | 조준 걷기 서기 | 조준 뛰기 서기 | 조준 걷기 앉기 |
|---|---|---|---|
| W | 76 (Pivots 24) | **0** (Starts 100) | 83 |
| WD | 64 | **~20** (Pivots 78) | 55 |
| D | 100 | **0** (Starts 100) | **18** (Starts 81) |
| SD | 49 | **0** (Starts 62 · Pivots 37) | 37 |
| S | 100 | 100 | **12** (Starts 75) |
| SA | 100 | **0** (Starts 100) | 37 |
| A | 100 | 98 | 100 |
| WA | 100 | **0** (Starts 100) | 40 |

- rest spd(커브 무관 미끄러짐): 걷기 6–30 cm/s, **뛰기 최대 95 cm/s**.
- planted% ≈ 0 전부 — 4절의 접지 커브 결함 그대로(적군 클립 확인은 아직).
- cross%(발 교차) 측면·대각선 40–58%.

### 5.2 전환(post 1.5 s, 56쌍)

| 구성 | Loop 도달 | t_loop 중앙 | 클립 수 중앙/최대 | 교차 프레임 중앙/최대 |
|---|---|---:|---|---|
| 조준 걷기 서기 | 55/56 | 0.70 s | 2 / 5 (S>WD, A>WD, WA>WD, W>WD, W>SD) | 30 / 48 |
| 조준 뛰기 서기 | **21/56** (35쌍 끝내 못 감) | 0.80 s | — | 33 / 52 |

### 5.3 해석 [B]

1. ★ **플레이어도 속도 ×0.85** — `SoldierMovementProfile`이 플레이어 병사에도 붙어 걷기 248 / 뛰기 495 cm/s(작성 속도의 0.85)로 달린다([W125]). **뛰기에서 Starts가 100% 이기는 패턴은 P31 서명**(속도가 작성 속도와 다르면 가속 구간인 Start/Pivot 클립 궤적이 더 잘 맞는다). 즉 "플레이레이트 밴드 0.75–1.25면 선택엔 무해"라는 [P193] 가정이 **선택 측면에선 틀렸을 가능성** — AI에도 똑같이 적용된다.
2. 접지 커브 결함(4절) → FootPlacement가 발을 고정 못 함 → 선택이 맞아도 미끄러짐이 남는다. 1번과 독립.
3. 4방향 DB의 대각선 공백(2절 가설) — 걷기에서도 대각선만 Loop가 흔들림(WD 64 · SD 49). 1번을 걷어낸 뒤에 봐야 순수 기여를 알 수 있다.

### 5.4 A/B — 속도 배율 끔 `Saved/AimTrace/20260930_191458` [A]

request 파일(`quick` + `exec SoldierLab.Move.Enabled 0`)로 무인 실행. 걷기 291 / 뛰기 583 cm/s(작성 속도).

| 구성 | Loop% 평균 (기준선 → A/B) | Loop ≥95% 방향 수 | rest spd 중앙 | 전환 Loop 도달 |
|---|---|---|---|---|
| 조준 걷기 서기 | 86 → 83 | 5 → 5 | 11 → 16 | 55 → 55 /56 |
| 조준 뛰기 서기 | **25 → 65** | **2 → 5** | **33 → 19** | **21 → 45** /56 |
| 조준 걷기 앉기 | 48 → 43 | 1 → 2 | 20 → 20 | — |

- 작성 속도에서 **Loop 클립 재생 속도가 거의 전부 1.00**(기준선은 0.85 하한에 붙어 있었음) → 속도 배율이 뛰기 선택을 망가뜨린 건 사실([P193] 가정은 선택 측면에서 틀림).
- 그러나 걷기·앉기는 개선 없음 → **남은 주원인은 따로 있다**:

### 5.5 ★ 원인 2 — Start 클립의 정속 구간이 Loop 대용으로 반복 선택된다 [A]

- 뛰기 W: `Jog_Fwd_Start`(길이 2.17 s)의 **0.68~1.13 s 구간만** 반복 — `mm.time` 1.12 → 0.94 → 0.75 → 1.06…, 같은 클립 안 역점프 3초에 6회(두 실행 모두). `Jog_Bwd_Start`(SA), `Crouch_Walk_Right_Start`(D, 1.18~1.84 s)도 같은 패턴.
- 기록의 `curve.MoveData_Speed`로 보면 **모든 Start 클립이 ~0.7 s에 작성 속도 도달**(뛰기 587~617, 걷기 ~295~330) — 그 뒤는 가속이 끝난 정속 보행 = 사실상 Loop. MM이 그 구간을 Loop 대신 고르고, 구간 끝에서 같은 클립 앞쪽으로 되감으며 블렌드 → 버벅임.
- DB 편향은 무관: Stand Jog Starts/Loops/Pivots 모두 continuing −0.05 · base 0 · looping −0.005 · exclude (0, −0.3) 동일.
- 같은 원리로 Pivot 클립 꼬리(반대 방향 정속 구간)도 의심 — 걷기 W Pivots 50%, WA 72%.
- **해법 후보**: Start(·Pivot) 클립의 **클립별 샘플링 범위를 가속 구간(~0–0.8 s)으로 제한** → 범위 끝에서 강제 검색 → Loop로 인계(GASP의 Start→Loop 흐름). 클립 길이가 1.63~2.30 s로 제각각이라 DB 단위 `excludeFromDatabaseParameters` 한 값으로는 불가. 클립별 범위(`AnimationAssets[].SamplingRange`)는 MCP로 읽기/쓰기 불가 → DB 에디터 수작업.

### 5.6 Start 샘플링 범위 (0, 0.8) 적용 — `Saved/AimTrace/20261001_050606` [A]

사용자가 아군 Start DB 3개 12클립에 Sampling Range (0, 0.8) 입력. 조건은 5.4와 동일(`Move.Enabled 0`).
- 적용 확인: Start 클립 선택 시간 최대 0.82 s(이전 1.15~1.84). DB exclude (0, −0.3)은 중복 적용되지 않음.
- 결과는 엇갈림: 앉아걷기 D 0 → 100, WD 42 → 80 · 그러나 뛰기 WD/SD 20/100 → 0(Starts 0~0.8 + Pivots), 뛰기 W는 Starts 대신 **Pivots 78%**, 걷기 SD/SA 100 → 40(Pivots). **Start 꼬리를 막자 Pivot 꼬리가 그 자리를 차지** → Start/Pivot은 증상이지 원인이 아니다.

### 5.7 ★★ 근본 원인 — 일부 Loop 클립의 루트 이동 속도가 보폭 안에서 크게 출렁인다 [A]

걷기 W 프레임 추적: `Walk_Fwd`(길이 2.0 s, 루프)가 **0.69 → 0.94 s만 재생** → `Walk_Bwd_Pivot` 1.0 → 1.13 s → 다시 `Walk_Fwd` 0.69 … 0.4 s 주기로 정확히 반복. 보폭 위상이 매번 끊기므로 **발 꼬임·미끄러짐의 직접 원인**.

클립 시간별 MM 희망 재생속도(`mm.wantedRate` = 캡슐 속도 ÷ 그 포즈의 클립 루트 속도, 밴드 0.85~1.15에서 잘림):

| Loop 클립 | 클립 전 구간 wantedRate | 해당 방향 Loop% |
|---|---|---|
| `Jog_Bwd` · `Jog_Left` · `Jog_Right` · `Walk_Left` | **1.00 일정** | 뛰기 S·A·D, 걷기 A = **100%** |
| `Walk_Fwd` · `Walk_Bwd` · `Walk_Right` · `Jog_Fwd` · `Crouch_Walk_*` 4종 | **0.85 ↔ 1.15 (밴드 양끝에 붙음)** | 흔들림 |

→ 문제 클립은 **루트 속도가 한 보폭 안에서 ±15% 이상 변한다.** 캡슐은 일정 속도라 MM 궤적 비용이 보폭 대부분에서 커지고, 궤적이 맞는 짧은 조각만 골라 다른 클립(Start/Pivot 정속부)과 오간다. 깨끗한 4클립은 루트가 선형이라 문제가 없다. (P4 이력 대조는 로그인 만료로 못 함 — 원인이 Lyra 원본인지 이후 가공인지 미확정.)

**해법 후보(기존 애셋 가공)**: 문제 Loop 8클립의 **루트 이동을 선형(평균 속도 × t)으로 재베이크하고 골반을 그만큼 보상**하는 애니메이션 모디파이어. 월드 발 궤적은 원본과 같게 유지되면서 루트가 캡슐처럼 일정 속도가 된다 → 궤적 비용 평탄 + 캡슐 등속 재생 시 미끄러짐 제거. Start 범위 제한은 이 수정 후 재평가.

### 5.8 ★ 루트 선형화 적용 — `Saved/AimTrace/20261001_071342` [A]

도구 `Source/SoldierLabEditor/AnimModifiers/SoldierRootSpeedTool.cpp`(요청 파일 `Saved/AnimTool.request` → 결과 `Saved/AnimTool.done`, 동사 `diagnose|linearize|revert|reindex`).

- 진단: 깨끗한 4클립 swing **0.0%**(완전 등속) vs 문제 8클립 **53~74%**(예: `Walk_Fwd` 평균 287, 최소 79 @1.93 s, 최대 383). 가설 확정.
- 선형화 8클립: swing 0.0%, 루트 직계 자식(3개) 월드 이동 **0.000 cm**(보이는 동작 불변). P4 체크아웃 후 저장.
- ⚠ 첫 시도는 효과 0 — `Modify()` 없이 편집해 PoseSearch DB가 인덱스를 다시 안 만들었다(MM은 인덱스의 옛 궤적으로 비교). `Modify()` 추가 + `reindex` 동사(`FAsyncPoseSearchDatabasesManagement::RequestAsyncBuildIndex(NewRequest|WaitForCompletion)`)로 해결. **클립을 코드로 고치면 반드시 DB 재인덱스.**

| 실행 | 걷기 서기 Loop%/100%방향 | 뛰기 서기 | 앉아걷기 | 전환 Loop 도달 (걷기/뛰기) |
|---|---|---|---|---|
| 기준선(×0.85) | 86 / 5 | 25 / 2 | 48 / 1 | 55 / 21 |
| 속도 1.0 | 83 / 5 | 65 / 5 | 43 / 2 | 55 / 45 |
| + Start 범위 0~0.8 | 68 / 2 | 43 / 3 | 60 / 3 | 56 / 48 |
| **+ 루트 선형화** | 75 / **6** | 50 / 4 | **100 / 8** | 53 / 40 |

- **정방향 W/A/S/D는 걷기·뛰기 모두 Loop 100%, 앉아걷기는 8방향 전부 100%**, rest spd 중앙 앉기 20 → 8.4, 걷기 → 9.3.
- **남은 실패 = 서기 대각선뿐**: 걷기 SD·SA(Pivots 72 · Starts 27), 뛰기 WD·SD·SA·WA(Starts/Pivots). 4방향 DB의 대각선 공백(2절 가설) 그대로 → 다음은 질의 궤적 4방향 스냅.

### 5.9 ★ 질의 궤적 4방향 스냅 — `Saved/AimTrace/20261001_073840` [A]

- C++ `USoldierAnimLibrary::SnapTrajectoryToCardinal`(BlueprintPure·ThreadSafe): 궤적 샘플 **위치만** 현재 샘플 중심으로 회전(Facing 불변). 몸 정면 = 샘플 Facing(메시 회전)의 +Y. 앞·뒤 우선 무상태 판정: |상대각| ≤ 55° 전진, ≥ 125° 후진, 그 사이 옆. 예측 이동 < 10 cm면 스냅 안 함. cvar `SoldierLab.Anim.TrajectorySnap`(기본 1).
- ABP `Update_Trajectory`(MCP로 노드 추가·재배선): 새 변수 **`RawTrajectory`** = 생성기의 이전 프레임 입력 + `Trj_Past/Current/FutureVelocity` 원천. 기존 **`Trajectory` = Snap(충돌 처리된 궤적)** — PoseHistory 노드의 `TransformTrajectory` 바인딩이 그대로 받아 MM 질의에만 스냅이 들어간다. (스냅된 값을 생성기 입력으로 되먹이면 회전이 매 프레임 누적되므로 분리 필수.)

| | 서기 걷기 | 서기 뛰기 | 앉아걷기 | 전환 Loop 도달 걷기 / 뛰기 |
|---|---|---|---|---|
| 루트 선형화까지 | 6/8 | 4/8 | 8/8 | 53 / 40 |
| **+ 스냅** | **8/8** | **8/8**(WD 96%) | **8/8** | 53 / **56** |

- 대각선은 기대대로 전진·후진 Loop(`Walk_Fwd`, `Jog_Fwd`, `Jog_Bwd`, 재생속도 1.00~1.02) + 워핑. `Enable_Warping` 전 케이스 1.0.
- **남은 것**: ① 뛰기 전진 대각선 WD·WA의 rest spd 69·45(같은 방식의 후진 대각선 SD·SA는 17.6, 정방향 W 19) → 전진 루프에서 워핑이 45°를 다 채우지 못하는 듯(워핑 각도 상한·보간 속도 확인 필요). ② 걷기 전환 중 도착 방향 SA 3쌍(W>SA, D>SA, WA>SA) 미도달.

### 5.10 남은 2문제 — 전진 대각선 미끄러짐 · 걷기 SA 전환 [A]

**① 뛰기 전진 대각선 미끄러짐** — 발 최소속도 분해 결과 **옆 방향 미끄러짐**(|lateral| 33·22 cm/s, 직진 8). 선택은 깨끗(되감기 0). 비교: 스냅 전 걷기 WD는 **`Walk_Right` −45° 워핑 → rest 2.7**, 스냅(앞뒤 우선) 후 **`Walk_Fwd` +45° → 17.9**, 뛰기 `Jog_Fwd` +45° → 45~69, 후진 `Jog_Bwd` ±45° → 17.6. **전진 루프는 워핑에 약하다.** → 스냅 창을 전진·후진 분리 cvar로: `SoldierLab.Anim.TrajectorySnap.ForwardHalfAngle` **40**(앞 대각선 → 옆걸음) · `.BackwardHalfAngle` **55**(뒤 대각선 → 후진). 워핑 노드 설정(Graph 모드, 상한 180°, 목표 = `LastNonZeroVelocity`)과 스티어링(`Get_DesiredFacing` = 궤적 Facing, 스냅 무관)은 무죄.

**② 걷기 → SA 전환 미도달** — `Walk_Bwd_Start` 0.4~0.8 s(범위 0~0.8 안이지만 이미 정속)가 반복 선택. → 아군 **Starts DB 3개 `baseCostBias` 0.1**. 그러자 **정지→출발 0.4 s 구간이 Starts 83% → 28%(Pivots 65%)** 부작용 → **Pivots DB 3개도 0.1**.

| 실행 | 걷기 rest 중앙/최대 | 뛰기 rest 중앙/최대 | 전환 도달 걷기/뛰기 · t_loop | 출발 0.4 s DB |
|---|---|---|---|---|
| 스냅(5.9) `073840` | 11.2 / 20 | 17.9 / **69** | 53 / 56 · 0.57 / 0.69 | Starts 83 · Pivots 17 |
| + 창 40/55 + Starts 0.1 `081139` | 6.4 / 13 | 17.7 / **20** | **56 / 56** · 0.44 / 0.51 | Starts 28 · **Pivots 65** |
| + Pivots 0.1 `082101` | 8.6 / 17 | 17.7 / 20 | 56 / 56 · **0.12 / 0.32** | Starts 60 · Loops 25 · Pivots 14 |

- 대각선 클립: WD→`*_Right`, WA→`*_Left`, SD/SA→`*_Bwd`(걷기·뛰기·앉기 모두), Loop% 24/24 = 100.
- ⚠ 트레이드오프: 편향 0.1은 **전환이 매우 빨리 Loop로 감**(Pivot 동작 생략 경향, t_loop 0.12 s) · **출발 25%가 Loop로 바로**. 눈으로 확인 후 0.05 등으로 조정 여지.

### 5.11 조준 뛰기 총 끊김 = 루프 이음새 불량(원본 애셋) [A]

- 사용자: 조준+`Jog_Fwd`에서만 루프 경계마다 총이 끊김. 기록: 경계 프레임 손 9.3 cm / 22°(평소 1.6 cm / 2.7°).
- `SoldierRootSpeedTool`에 `seam`(첫 키 vs 마지막 키, 루트 기준) 추가. **원본(P4 #1, `p4 diff -sa` 무변경) 상태에서** 아군 Loop 8 · 적군 Loop 12가 발 13~28 cm 불량, 아군 `Jog_Fwd`는 왼손 17.4 cm / 28.5°, 아군 `Crouch_Walk_Bwd`는 왼 아래팔 45°. 정상 = 아군 `Jog_Bwd`/`Left`/`Right`·`Walk_Left`·Idle 전부. 게임 기록에서도 같은 8클립만 경계에서 발 10~21 cm 튐.
- **우리 작업 무관 증명**: 원본으로 되돌린 8클립을 다시 선형화 — 이음새 수치 전후 완전 동일.
- 처리: 사용자 결정 — **디자이너 수정 요청**(우리가 고치지 않음). 목록 = `assets/2026-10-01_loop_seam_defects.md`. (`fixseam` 명령은 만들어 두었으나 쓰지 않음.)
- 선형화 코드는 트랙 키 직접 계산으로 교체(포즈 평가 의존 제거). 한때 `ik_hand_root` 971 cm 오차를 선형화 버그로 오판했으나 **원본 아군 클립 자체의 상태**였다(적군판엔 없음).

### 5.12 적군 적용 — `Saved/AimTrace/20261001_110056` [A]

- 적군 Loop 12 전부 선형화(swing 52~74% → 0, 이음새 전후 동일, 자식 `pelvis`·`ik_foot_root`·`ik_hand_root` drift 0) · Loops DB 3 재인덱스 · Starts·Pivots DB 6 `baseCostBias` 0 → 0.1 · Starts DB 3 Sampling Range (0, 0.8)(사용자 입력) · 스냅은 ABP 공용.
- 테스터 cvar `SoldierLab.Test.Loco.EnemyAnimSet 1` — 플레이어(아군 BP)의 ABP `UseAllyAnimSet`을 false로 덮어 적군 클립을 측정(겉모습은 아군 그대로, MM DB 100% `Enemy_`).
- 결과: 정상 24/24 Loop 100%, 전환 112/112 도달, 출발 0.4 s Starts 65%. **뛰기 미끄러짐만 아군보다 큼**(중앙 34.9 / 최대 63 vs 17.7 / 20) → 클립 품질(아군은 뛰기 루프 3개가 디자이너 정리본) — 디자이너 목록에 추가. ~~적군은 접지 커브 정상(C-179는 아군만)~~ → **정정(5.14 이후 측정)**: 적군도 Loop 접지 커브가 대부분 망가져 있다(뛰기 0~4%, 걷기 0~17%) — [C-179] 확장.

### 5.13 병렬 테스터 — 구성 블록마다 병사 1명 [A]

- `SoldierLab.Test.Loco.Parallel 1`(기본): 구성 블록(한 구성의 정상 상태 8방향, 또는 전환 56쌍)마다 병사 1명, 동시 실행. `all` 11명 · `quick` 5명, 소요 = 가장 긴 블록(전환 ≈ 3분). 추가 병사는 **로컬 플레이어**(`UGameplayStatics::CreatePlayer`, 분할화면 강제 끔) — AI 컨트롤러면 `!IsPlayerControlled()` 분기 컴포넌트(AIBridge·GaitBridge·PoseSmoother·ScanTurn·HeadAim)가 입력을 덮어씀. 60 m 격자(`LaneSpacing`), 캡슐끼리 충돌 무시, 종료 시 제거. 리포트는 세션의 병사별 CSV를 모두 합친다.
- 검증 `Saved/AimTrace/20261001_112316`(적군 클립, 병사 4명, 사용자가 189/324에서 중단): 적군 순차 실행 `110056`과 같은 케이스 비교 — 정상 24/24 Loop% 동일, rest spd 대부분 ±1 cm/s(최대 차 63.1 vs 68.2), 전환 46/46 동일. **59.9 fps, dt > 0.05 프레임 0**.
- 발견: ① `MaxSplitscreenPlayers`(기본 4) 때문에 5번째 플레이어 생성 실패 → 실행 중에만 리플렉션으로 블록 수까지 올리고 복원(코드 수정, 라이브 코딩 대기) ② `GM_SoldierLab` 기본 폰이 `BP_Soldier_Hostile`로 바뀌어 있었다(이 세션 아님, default CL에 열림) → `EnemyAnimSet`을 "0 = 아군 강제 · 1 = 적군"으로 바꿔 기본 폰과 무관하게 함, 종료 시 캐릭터 자신의 값으로 복원(코드 수정, 라이브 코딩 대기).

### 5.14 `all` 병렬 — 아군 BP / 적군 BP 각자 클립 [A]

사용자 지시: **아군 BP면 아군 클립, 적군 BP면 적군 클립**으로 테스트(클립 세트 강제 방식 폐기). 테스터 cvar `SoldierLab.Test.Loco.Side` 0 = `BP_Soldier_Friendly` · 1 = `BP_Soldier_Hostile` — GameMode가 다른 BP를 냈으면 그 자리에 원하는 BP를 생성해 플레이어가 빙의(AI 자동 빙의 끔), 클립 세트는 캐릭터 BP 자신이 정함. 첫 플레이어의 원래 폰은 숨겼다가 종료 시 복원. `MaxSplitscreenPlayers` 리플렉션 상향으로 11명 생성.

| 구성 | 아군 `113055` 정상 Loop · rest 중앙/최대 · 전환 도달 | 적군 `113357` |
|---|---|---|
| 조준 걷기 서기 | 8/8 · 7.6 / 17.2 · 56/56 | 8/8 · 11.1 / 17.2 · 56/56 |
| 조준 뛰기 서기 | 8/8 · 17.7 / 20.0 · 56/56 | 8/8 · **32.8 / 68.4** · 56/56 |
| 조준 걷기 앉기 | 8/8 · 6.3 / 21.8 · 56/56 | 8/8 · 5.7 / 9.8 · 56/56 |
| 총내림 걷기 서기 | 8/8 · 8.2 / 9.9 · 56/56 | 8/8 · **19.2 / 20.9** · 56/56 |
| 총내림 뛰기 서기 | 8/8 · 20.1 / 20.7 · 56/56 | 8/8 · **35.0 / 43.5** · 56/56 |
| 총내림 걷기 앉기 | 8/8 · 7.3 / 9.5 · — | 8/8 · 5.9 / 5.9 · — |

- **양 진영 정상 48/48 Loop 100%, 전환 280/280 도달.** 총내림 구성(C-180)도 처음 측정 — 문제없음.
- 적군 뛰기(조준·총내림)와 총내림 걷기 서기의 미끄러짐이 아군보다 큼 → 클립 품질, 디자이너 목록에 반영.
- 11명 동시: 59.9 fps, dt > 0.05 프레임 0 / 26 (7만 프레임 중).

### 5.15 디자이너 수정본(Change 20 아군 8 · 21 적군 12) 후처리 재적용 — 2026-10-06 [A]

- 진단(`seam`·`diagnose`·`ikcheck`): FK 이음새 — 아군 완벽(`Jog_Fwd` 왼손·`Crouch_Walk_Bwd` 아래팔 0), 적군은 몸 전체 2~4° 회전 잔여(최대 `Jog_Right` 4.06°, 말단 3~6 cm). **Sequencer 재베이크로 우리 후처리가 전부 사라짐**: 루트 swing 31~74% 복귀 · `ik_foot_l/r`이 발에서 8~21 cm(정상 0) · 접지 커브 일부 소실.
- 처리: ① `linearize` 20개(이음새 전후 변화 0) ② 각 클립 자신의 모디파이어 스택 A를 도구 `applystack`으로 **두 번** 적용 — `AM_Copy_IKFootRoot`(CopyBones, World: `attach`→`ik_foot_root`, `foot_l/r`→`ik_foot_l/r`)가 부모(`ik_foot_root`)를 옮기기 전 값으로 자식을 계산해 1회차엔 54.75 cm 어긋나고, 2회차에 0.00 cm로 수렴 ③ 저장 · Loops DB 6개 재인덱스. 20개 모두 P4 체크아웃(미제출).
- MCP `SelectAssets`는 성공을 반환하지만 실제 선택이 안 됨 → 콘텐츠 브라우저 선택 대신 도구 명령으로 직접 적용.
- 접지 커브: 임계값 20/30/40/60 cm/s 스윕 — 뛰기 0~5%, 걷기 ~20%, 정상 `Walk_Left`도 36%. 진단 로그상 뛰기는 **높이 조건(최저+3 cm) 통과 19~24%, 속도 조건 통과 3~10%** — 디딘 발볼이 클립 안에서 실제로 움직여 이 판정 방식으로는 못 잡는다 → 판정 방식 재설계 필요([C-179]).

| `all` 병렬 | 아군 전 `113055` → 후 `110352` | 적군 전 `113357` → 후 `110654` |
|---|---|---|
| 정상 Loop · 전환 도달 | 48/48 · 280/280 → 동일 | 48/48 · 280/280 → 동일 |
| 조준 뛰기 rest 중앙/최대 | 17.7/20.0 → 17.2/18.2 | **32.8/68.4 → 22.2/41.0** |
| 총내림 뛰기 | 20.1/20.7 → 17.6/17.7 | **35.0/43.5 → 21.7/22.1** |
| 총내림 걷기 서기 | 8.2/9.9 → 8.1/9.0 | **19.2/20.9 → 7.1/9.8** |
| 루프 경계 튐(최악 부위, cm) | 걷기·앉기 12~23 → 2~6 · `Jog_Fwd` 24 → 8.7(손 튐 소멸) | 14~28 → 3~9 |

- 남은 것: 루프 경계의 **속도 불연속**(자세는 같지만 발 속도가 끊김, 경계 튐 3~9 cm vs 평소 2~5) · 적군 2~4° 몸 회전 잔여 · 접지 커브 판정 재설계.

## 6. 다음

완료(아군): ~~A/B 속도 배율~~(5.4) · ~~질의 궤적 스냅~~(5.9) · ~~남은 2문제~~(5.10) = [W130]. 편향 0.1은 사용자가 육안 확인 후 유지(2026-10-01). 함정 기록: [P201] 코드로 클립 수정 → DB 재인덱스 · [P202] 전진 루프 ±45° 워핑 약함.

OPEN_ITEMS 연결: ~~적군 적용 [W131]~~(5.12 완료) · ~~P4 제출 [W132]~~(Change 17) · 속도 밴드 가정 [Q53]/[W125] · 접지 커브 [C-179] · ~~총내림 미측정 [C-180]~~(5.14 해결) · 대각선 Stop [W124](스냅으로 해소 추정) · 디자이너 이음새 수정 [W133] · 미제출 3파일 [W134].

1. **접지 커브 재생성(우리 몫, [C-179])** — 아군·적군 Loop 대부분이 접지 0~17%(정상 걷기 ~60%, 뛰기 30~40%). `UFootContactCurveModifier` 속도 임계 10 cm/s가 클립의 디딘 발 미끄러짐(18~60 cm/s)보다 낮아서. `Saved/AnimTool.request`에 `contact <클립> <임계값>`으로 재생성(메모리) → 병렬 `all` 아군·적군 재측정 → 개선 확인 후 저장. 그래도 크게 미끄러지는 클립만 디자이너 목록에.
2. **디자이너 루프 이음새 수정 대기([W133])** — 전달본 `assets/2026-10-01_loop_seam_fix_request.html`. 수정본 오면 `seam` → `linearize` → `reindex` → (`contact`) → 병렬 `all` 양측.
3. 플레이어 속도 배율 제외([W125]) · AI 속도 정책 재검토([P193]) — 사용자 결정 사항. 테스트는 계속 `exec SoldierLab.Move.Enabled 0`.
4. P4 미제출([W134]): `SoldierLocomotionTest.{h,cpp}` · `SoldierRootSpeedTool.cpp` — **제출은 파일 목록 제시 후 사용자 명시 승인 필수.**

### 도구 사용법 (무인 반복)

```
Saved/LocoTest.request   1행 = 스케줄(quick|steady|turns|all), 2행~ = "exec <콘솔명령>"
                         → 플레이어 폰 생기면 자동 시작, 결과 Saved/LocoTest.done
Saved/AnimTool.request   "diagnose|linearize|revert|reindex <에셋경로>" 줄 단위
                         → 에디터가 1초마다 처리, 결과 Saved/AnimTool.done
리포트                    PYTHONIOENCODING=utf-8 python tools/locomotion_report.py [세션] --csv out.csv
```
- 클립을 코드로 고치면 **반드시 해당 DB `reindex`**(안 하면 MM이 옛 인덱스로 비교 — 5.8).
- MCP `StartPIE`/`StopPIE` + 위 요청 파일로 사람 없이 측정 가능. **레벨은 `L_Basic`**(다른 레벨이면 다른 폰을 조종).
