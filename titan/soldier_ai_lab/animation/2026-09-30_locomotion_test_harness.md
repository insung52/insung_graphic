# 로코모션 자동 테스터 + 기록기 확장 (발 미끄러짐·방향 전환 꼬임 측정용)

2026-09-30 / 진행중(기준선 측정 완료 · 속도 배율 A/B 대기) / 플레이어 병사를 조준/총내림 × 걷기/뛰기 × 서기/앉기 × 8방향 안정·전환 스케줄로 자동 조종하며 프레임별로 기록하고, 케이스별 Loop 비율·접지 발 미끄러짐·발 교차를 표로 뽑는 도구.

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

## 6. 다음

1. **A/B: 속도 배율 끄고 같은 스케줄** — request 파일 2번째 줄부터 `exec <콘솔명령>`을 받도록 테스터 확장(라이브 코딩 필요). `SoldierLab.Move.Enabled 0`은 런타임에 작성 속도로 되돌린다(`WriteGaitSpeeds(1.f)`).
   ```
   quick
   exec SoldierLab.Move.Enabled 0
   ```
2. 결과에 따라: 속도 정책을 플레이어에서 빼기([W125]) · AI 쪽 정책 재검토([P193], 사용자 결정 사항).
3. 질의 궤적 4방향 스냅 구현 → 같은 스케줄로 전후 비교.
4. 접지 커브: 클립 원본 커브 확인 경로 찾기(적군 클립 포함).
