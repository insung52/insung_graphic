# 현재 상태 — soldier_ai_lab

2026-09-14 / **★ AI 층 동작 확인 · 아군 메시 교체 완료** / 병사가 스스로 보고·듣고·전달받고·제압당하고·쏘고·엄폐한다.
**아군은 이제 `soldier_T` 외형이다** — `Assign Skeleton` 으로 `SK_UEFN_Mannequin` 에 올려 애니메이션·PSD·ABP 를
**하나도 안 고치고** 돈다 (`IMPLEMENTED.md` 3.2절). 적군 외형은 **결정 대기 [Q42]** — 지금은 마네킹 그대로이고 AI 작업을 막지 않는다.
⚠ 메시를 바꾸면 `StanceStandZ/CrouchZ` 같은 **실측값을 메시별로 다시 재야 한다**(P77). 사용자가 원하는 다음 시험은
"아군을 마네킹 비율로"(translation retargeting `Animation`, 5분) → **[W28]**.
★ **2026-09-14 교정 라운드가 들어갔다** — 노출의 사다리(조리개·사격자세) · 반동 · 조준 선회 ·
`FightingCost` · 재장전 조건 · 추측항법 만료 · 정지 감지/RVO · **110m × 90m 레벨 재건** · 빙의.
**목표 층은 이번 라운드에야 처음 실제로 돌았다** — 레벨에 1개 놓았고, 더 큰 것은 `ASoldierObjective` 가
**루트 컴포넌트가 없어 영원히 월드 원점에 서 있었다**는 것이다(**P90**).
⚠⚠ **그러나 수비수가 정착해 쓰지 못하는 문제는 두 번의 시도로도 해결되지 않았다.**
**다음 세션의 첫 일은 세 번째 추측이 아니라 진단이다** → **[C-95]** · `ai/2026-09-14_exposure_ladder_and_corrections.md` 12절.


---

## ★ 2026-09-14 — `titan_example` 편입 결정

디자인팀과 합의: **GASP/Lyra 가 기존 `titan_example` 애니메이션 시스템보다 낫다**는 판단으로
`SoldierLab` 을 본체에 편입한다. **이 PC 에서의 작업은 이관까지고, 그 뒤로는 문서만 남는다.**

**실사 결과 — 걱정한 3건은 전부 무충돌** (`migration/2026-09-14_titan_example_migration.md`):
엔진 둘 다 5.8 · titan 은 **커스텀 트레이스 채널이 0개**라 `Cover`(4)/`Sight`(5) 번호 충돌 없음 ·
`PhysicalSurfaces` 5줄이 **글자까지 동일**(투사체가 원래 titan 것이라 설정을 같이 가져왔다) ·
titan 에 `UEFN` 에셋 0건이라 **2본 추가된 `SK_UEFN_Mannequin` 이 덮어쓸 것이 없다** · 모듈 이름 충돌 없음.

**대신 나온 것**: titan 에 GASP 가 한 조각도 없어 기반이 통째로 들어간다 → **[Q43]** (2.7 GB) ·
플러그인 **16종**을 켜야 한다(교집합 3종뿐) → **[W34]** · 빠지면 **조용히 껍데기**가 된다.

**정리 스캔** (`migration/2026-09-14_asset_cleanup.md`) — `Content/` 전체를 바이트 단위로 훑었다:
★ **`GM_SoldierLab` 의 캐릭터 목록 배열 하나가 약 2.5 GB 를 끌고 온다** → **[W36] 최우선** ·
참조 0건 **약 44 MB** 삭제 가능(백업 3 · Pistol/Shotgun/Death/HitReact · `_MF/` 39 · `_Extra/` 12) ·
`SoldierLab/` 밖에 남은 우리 것 **9건** · 이동 후 빈 폴더 **6개** · 이름 충돌 `soldier_T` → **[W33]**.
⚠ 스캔에는 오차가 둘 있고 **둘 다 실제로 물렸다**(P95 · 낡은 경로 문자열) — 0절을 먼저 읽을 것.

### 다음 순서

```
1. 에셋 정리        [W36] → 백업/미사용 삭제 → [W37] 폴더 교차 → 빈 폴더
2. 마이그레이션      Config 먼저 → 플러그인 → 모듈 → Migrate → 검증 8단계
3. 적군 스켈레톤 규격서   디자인팀. "soldier_T 와 같은 규격" 한 줄이면 된다 [Q42]
4. 디자인팀 핸드오프  애님 시퀀스 목록(266개, 손대도 되는 것 구분) + 기능 사용법
```

⚠ **[C-95](수비수 정착 실패)는 이관과 무관하게 열려 있다.** 이관 후에도 그대로 재현된다.

<details>
<summary>2026-09-11 시점의 한 줄 요약 (접힘)</summary>

2026-09-11 / **★ 애니메이션 층(L4) 마감** / 이동·조준·견착/총내림·사격·재장전·왼손 IK가
전부 동작하고, 전 방향 로코모션의 속도 불일치([C-58])와 웅크리기 회전 발작(P18 재발)까지 해결했다.
플레이어가 WASD로 직접 검증 가능하다. **다음은 AI 상위 계층이다.**

</details>

> **무엇이 존재하는지는 `IMPLEMENTED.md`** — 에셋 목록, 애님 그래프 구조, 확정된 튜닝값과 그 근거.
> 이 문서는 "다음에 뭘 할 것인가"만 다룬다.

> **2026-09-12 추가분**: **유한 몸통 각속도**(총을 내리면 빨리 돈다 — 90~720 °/s)와
> **조준 보정 안티 와인드업**(`Enable_AO` 게이트)이 들어갔다. 새 미해결 3건 —
> **[C-76]**(간헐적 무기 잠금, 방아쇠 미규명) · **[C-77]**(극단 상방 조준, **의도적 미해결**) ·
> **[R6]**(`AO_Blend_Curve` 미확인).
> 전문: `animation/prototypes/2026-09-12_body_yaw_rate_and_aim_antiwindup.md`

> **2026-09-12 추가분 (2)**: **블라인드 파이어 축**이 들어갔다 — `BlindFireH`(−1~+1) ·
> `BlindFireV`(0~+1) 두 연속 축이 **저작 포즈 3장을 마스크 애디티브로** 구동한다(키 1·2·3·4).
> 총구 정렬 · 린에 이은 **세 번째 연속 자세 축**이고 사용자 확인 완료.
> 새 미해결 3건 — **[C-78]**(BF와 총구 보정 적분기의 상호작용 미측정) ·
> **[W7]**(린 축이 문서에 없다) · **[W8]**(마스크 규약 두 벌). **[C-8] 해결**(형태가 바뀜).
> 전문: `animation/prototypes/2026-09-12_blind_fire_axis.md`
> **포즈 저작 경로 자체는 `CLAUDE.md` 6.3절에 규약으로 적었다** — 막다른 길 4종 포함.

> **2026-09-12 추가분 (3)**: **연속 stance 축**이 들어갔다 — `StanceAxis`(0 기립 ~ 1 웅크림, 키 **V/B**)
> 하나가 **골반 높이 · 이동 속도 상한 · `Gait` 라벨 · MM 데이터베이스**를 함께 몬다.
> **네 번째이자 마지막 연속 자세 축**이고 사용자가 단계마다 확인했다.
> 골반 높이는 `FootPlacement`와 `LegIK` **사이**의 `ModifyBone(pelvis)`를 **매 프레임 역산**해서 잡는다
> (새 C++ 2함수). **같은 증상을 네 번 오진한 기록이 이 문서의 본체다** → **P44**(과도구간을 계측하라) ·
> **P45**(통한 패턴을 반사적으로 옮기지 말라) · **P46**(이산 라벨은 연속량과 함께 바꾼다).
> 새 미해결 — **[C-79]**(움찔 해결의 기구 미증명) · **[C-80]**(`maxRotationError 90` 회귀 후보) ·
> **[W9]**(★ 캡슐이 아직 이진 — **엄폐 판단이 읽는 높이**) · **[W10]**(카메라) · **[W11]**(AO 연속) ·
> **[W12]**(중간 포즈 3점 블렌드) · **[W13]**(`IA_Crouch` 무력화). **[Q41] 결정**: 웅크린 채 달리면
> **속도를 묶는다** — 자세는 AI가 소유하는 결정이고 속도가 결과다.
> 덤으로 **AI에게 치명적이던 `GetControlRotation(GetController())` null 버그**를 두 호출부에서 고쳤다.
> 전문: `animation/prototypes/2026-09-12_continuous_stance_axis.md`

> **2026-09-12 추가분 (4)**: **무기·투사체가 붙었다** — `titan_example`의 `ARCWSProjectile`을
> **의존 4종을 끊어** `Source/SoldierLab/Weapons/SoldierProjectile`로 이식하고,
> `BP_RifleProjectile`(마이그레이션본이 껍데기여서 **0부터 재작성**) · `BP_AR4Rifle`(순수 BP) ·
> `BP_SoldierCharacter` 배선까지 마쳤다. **실제로 총알이 날아가고 재질별로 터진다.**
> 포물선 탄도 · 도탄 · 재질별 명중 세트 · 데칼 · 지면 혈흔 · **총알 휘파람**이 살아 있다.
> ★ **휘즈의 최근접 판정이 AI 제압 신호의 자리다** — 기하는 이미 있고 지금은 카메라만 본다 → **[W16]**.
> `Config/DefaultEngine.ini`에 `PhysicalSurfaces` 5종(**선언 순서가 의미를 갖는다**)과
> 트레이스 채널 `Cover`/`Sight`를 팠다. **이식 전 "채널 인덱스가 충돌한다"는 예고는 틀렸다** —
> `titan_example`은 커스텀 채널을 쓰지 않는다 → **P52**.
> 배선 중 버그 6건에서 **P47~P51**이 나왔다(원인이 둘인 컴파일 오류 · 액터 단위 숨김 ·
> 자기 총열 명중 · **조건 앞에서 낸 부작용이 재장전 몽타주를 잘라먹은 것** ·
> 메시 교체 후의 `GetAnimInstance` None · 노드 이동 후 남은 exec 링크의 무한 재귀).
> 새 미해결 — **[C-81]**(디버그 궤적 라인만 안 보인다) · **[C-82]**(이식 튜닝값 미검증) ·
> **[R7]**(진영 판정의 정식 소스) · **[W14]**(에셋 경로) · **[W15]**(**P4V 체크아웃 미처리**) ·
> **[W16]** · **[W17]**(끊어낸 의존의 복원 지점).
> 전문: `weapons/2026-09-12_projectile_port.md`

> **2026-09-13 — AI 층이 들어갔다 (이 문서의 현재 머리글)**: `Source/SoldierLab/AI/` **C++ 8쌍**과
> `BP_SoldierCharacter` 컴포넌트 7개, `BP_Soldier_Friendly`/`_Hostile`, 관전 폰, 시가지 시험 레벨.
> **AI가 GASP 몸을 실제로 운전한다.**
>
> 구조적으로 새로 정한 것 — **신선도와 해상도를 합치지 않는다**(P62) · **감쇠는 질의 함수에**(P63) ·
> **불확실은 라벨이 아니라 지터**(P64) · **융합은 역분산 가중이라 우선순위 규칙이 없다**(P65) ·
> **게이트는 이미 있는 두 양의 비교**(P66, "앎이 무기보다 나쁜가") · **비율 게이트에는 절대 게이트를**(P67) ·
> **술어는 긍정 열거로**(P68) · **0 비용 반복이 가능한 루프는 횟수로도 막는다**(P69) ·
> **오버레이의 한 채널은 한 뜻**(P70) · **AI는 플레이어와 같은 레이트 상수를 쓴다**(P71).
> 도구 함정 9건이 **P53~P61**로 올라갔다 — 그중 **[P53] `set_properties`가 이 환경에서 쓰기를 못 한다**가
> 가장 아프다(모든 기본값을 사용자가 직접 넣어야 한다).
>
> ★ **"AI가 총을 안 들고 몸도 안 돈다"의 근본 원인**은 `S_PlayerInputState.WantsToAim` 이었다.
> 파생 플래그 `AOActive`를 만지던 이전 패치는 증상 처치였으므로 **되돌렸다** → **P36**.
>
> 해결 — **[W3]**(컨트롤러 조준 구동) · **[W16]**(제압 신호) · **[C-43]**(형태가 바뀜: StateTree 아님) ·
> **[R7]**(정식 진영 소스 생김, 투사체만 잔여).
> 새 미해결 — **[C-83]**(45명 성능) · **[C-84]**(제압 상수) · **[C-85]**(인지 상수) ·
> **[C-86]**(교전 거리) · **[C-87]**(교전/엄폐 튜닝값) · **[D10]**(★ 목표·임무 개념) ·
> **[W18]**(사망 없음) · **[W19]**(린/BF를 AI가 안 몬다) · **[W20]**(경로 노출) ·
> **[W21]**(엄폐 후보가 링 하나) · **[W22]**(소리 차폐) · **[W23]**(투사체 진영) · **[W24]**(`Cover` 채널 미사용).
> 전문 3부작: **`ai/2026-09-13_perception_stack.md`** · **`ai/2026-09-13_engagement_and_cover.md`** ·
> **`ai/2026-09-13_ai_bridge_and_scene.md`**

> **2026-09-13 추가분 (2) — 목표(objective)와 위치 비용**: `ASoldierObjective`(9번째 파일 쌍)와
> **세 비용 위치 스코어러**가 들어갔다 —
> `Cost = Exposure + RouteRiskWeight×RouteRisk + ObjectiveWeight×ObjectiveCost`, **전부 0..1 한 통화**.
> 목표 액터는 **로직이 없는 레벨 마커**이고 **한 개가 쥔 쪽에겐 수비·나머지 전부에겐 공격**이 된다.
> 수비 비용은 반경 안이 **평평한 0**(거리로 매기면 수비대가 중심점으로 붕괴한다), 공격은 **평지 없는 비례**.
> 경로 노출 표본이 함께 들어와 **[W20]이 절반 해결**됐다(직선 표본 — 네브메시 경로가 아니다).
> 새 원칙 — **무모함은 값이 안 매겨진 항이다**(P72) · **지킬 것이 구역이면 비용도 구역**(P73) ·
> **비용은 실제로 수행되는 방식으로 매긴다**(P74, 경로는 *서 있는 높이*로) ·
> **엔진 베이스가 쓰는 이름을 지역변수로 쓰지 말 것**(P75, `Role` → C4458).
> 해결 — **[D10]**. 새 미해결 — **[C-88]**(목표 층 값과 거동) · **[W25]**(★ **레벨 배치 0개 · 목표가
> `BeginPlay` 에 하나로 고정**) · **[W26]**(반동 누적이 없다 — 30번째 탄이 첫 탄과 같다).
> ⚠ **이 조각은 아직 한 번도 돌아 본 적이 없다** — 레벨에 목표가 없으면 셋째 항은 0이다.
> 전문: **`ai/2026-09-13_objective_and_position_cost.md`**

> ★ **2026-09-14 — 교정 라운드 (이 문서의 현재 머리글)**: 목표 층이 들어온 뒤의 모든 것.
>
> **교전** — 총구가 막히면 거절하는 대신 **조리개**(Direct/Over/Right/Left)를 찾고, 그 다음
> **몸을 얼마나 같이 내보낼지**를 고른다(Open / Lean / Blind — 칸을 고르는 것은 **제압도**).
> 조리개의 답이 **위 아니면 옆**이고 그것이 포즈 파이프라인이 이미 갖고 있던 두 축이었다 → **[W19] 해결**.
> **반동**은 탄창이 줄어드는 것을 보고 센다(새 훅 없음) → **[W26] 해결**.
> **조준이 240 °/s 로 선회**하고 시야 콘도 그 회전을 읽는다(머리가 안 돌아간 표적은 보지 못한다) → 새 의도 `Traversing`.
> 재장전은 오지 않는 소강상태를 기다리는 대신 **숨을 곳이 있는 순간**을 산다.
>
> **위치 비용** — 첫 항이 노출에서 **`FightingCost`** 로 바뀌었다: 노출만 최소화하면
> **깊은 구석이 만점**이고 거기 있는 병사는 **맞을 수도 쏸 수도 없다** — 실제로 그러고 있었다.
> 경로 위험은 **건너는 거리로 스케일**하고 가중치를 1.0→0.6 으로 내렸다
> (같게 두었더니 **아무도 스폰에서 안 움직였다**). 후보는 **두 겹 링**, 위협 추정은 **한 바퀴 동안 얼린다**,
> 위협이 없으면 `bCanHide` 는 **true** 다(아무도 안 쏭면 엄폐가 없는 자리란 없다).
>
> **목표** — ★ `ASoldierObjective` 가 **맨 `AActor`** 라 루트 컴포넌트가 없었고, 그래서
> **모든 목표 거리가 (0,0,0) 에서 재어지고 있었다**(P90). 공격 비용의 **clamp 제거**(평평한 비용에는 기울기가 없다),
> 환율 6000→3000, 반경 900→1500 · 밴드 900→1200(**수비 사격 위치가 1077cm 로 옛 반경 바로 밖**이었다).
> 레벨에 `Objective_AllyBase` **1개 배치** → **[W25] 절반 해결**.
>
> **그 밖** — 추측항법 만료(`VelocityTrustSeconds 1.5` — 병사들이 **환영을 향해** 자리를 옮기고 있었다) ·
> **정지 감지**(보고된 프리즈의 정체 — 마주 보고 걸어간 둘이 경로추종기를 영원히 `Moving` 으로 두었다) ·
> **RVO 회피** · **스프린트**(규칙이 아니라 **느릴 이유의 부재**) · **레벨 재건 110m × 90m** · 관전 **빙의**.
>
> 새 원칙 **P81~P94** — 막힘의 답은 다른 총구 위치(P81) · **완벽한 엄폐와 사격 위치는 반대말**(P82) ·
> 시간 규모가 다른 비용을 같은 가중치로 더하지 말 것(P83) · 비교에만 쓰는 비용에 천장 금지(P84) ·
> 구역 반경은 싸울 엄폐를 품어야 한다(P85) · 레이트 제한은 **양을 만드는 자리에서 한 번**(P86) ·
> 진행 중은 상태가 아니라 **진척**으로 묻는다(P87) · 믿음의 반감기는 양마다 다르다(P88) ·
> ★ **거동은 규칙이 아니라 선호로 쓴다**(P89, **사용자 명시 요구**) · 루트 없는 `AActor` 는 위치를 못 갖는다(P90) ·
> 도구 함정 3건(P91~P93) · 화면의 예고와 키의 실행은 같은 계산에서 나와야 한다(P94).
>
> 해결 — **[W19]** · **[W26]**. 절반 — **[W20]** · **[W21]** · **[W25]**.
> 새 미해결 — **[C-91]**(사다리 상수) · **[C-92]**(반동) · **[C-93]**(조준 선회) · **[C-94]**(위치 비용 재교정값) ·
> **★★ [C-95]**(수비수가 정착하지 못한다 — **세 갈래 진단**) · **[C-96]**(속도 신뢰) ·
> **[C-97]**(레벨 규모·정지 판정·회피) · **[W30]**(★ **1인칭 카메라가 없다 — 요청됐고 안 만들었다**) ·
> **[W31]**(관전 입력이 InputAction 이 아니다) · **[W32]**(조리개 트레이스가 예산 밖).
> 전문: **`ai/2026-09-14_exposure_ladder_and_corrections.md`**

> **오늘 한 일 전문: `animation/prototypes/2026-09-09_lyra_rifle_migration.md`** — 반입·PSD·Chooser·
> 디버깅 전 과정. **다음 세션은 이 문서만 보면 로코모션 상태를 전부 안다.**

> **전문: `animation/prototypes/2026-09-04_p0-4_complete.md`** — 확정된 11단계 파이프라인, 모디파이어
> 순서 규약, PSD 설정값이 여기 있다. **반입 작업을 하려면 이 문서만 보면 된다.**
>
> **★ 계획 대비 무엇이 달라졌는지: `design/2026-09-04_p0_checkpoint.md`** — 리스크 장부가 뒤집힌 것,
> 인플레이스 전제 오류, CMC/Mover 분기, 반복된 실패 패턴, 남은 계획 조정.
> **새 세션은 `CLAUDE.md` → `CURRENT_STATE.md` → 이 점검 문서 순으로 볼 것.**

> **환경 (2026-09-03 갱신)**: 프로젝트는 `C:\working\kadex\anim_test\SoldierLab`(`works\` 없음),
> 문서는 `C:\mine\insung_grapic\titan\soldier_ai_lab`, **MCP 포트 8000**. → `CLAUDE.md` 6·7절

---

## 1. 한 줄 요약

**"GASP 정도의 움직임 + 총을 든 상태"가 처음으로 성립했다. 이제 전투 동작(사격·재장전·피격)과
로우레디를 얹을 차례다.**

### 1.1 현재 보유 재고 (2026-09-09)

| 구분 | 내용 | 상태 |
|---|---|---|
| **로코모션** | Lyra 라이플 61클립 — Walk·Jog·Crouch × 4방향 × Loop/Start/Stop/Pivot + TurnInPlace + Idle | ✅ PIE 동작 |
| **PSD / Chooser** | PSD 16개(Epic Dense 설정 복제) · `PSN_Rifle_All` · Chooser 3열 16행 | ✅ |
| **조준 오프셋** | `AO_Rifle_Aim` (Yaw 5 × Pitch 3) — GASP AO 배관 재사용 | ✅ 동작 |
| **액션** | 95개 — 사격·재장전·피격 13·사망 6·근접·수류탄·대시 | ✅ 사격·재장전 배선 완료 |
| **무기** | `SK_Rifle` → `weapon_r` 부착 + `ABP_Weap_Rifle`(볼트·탄창) | ✅ |
| **로우레디** | `AO_CD` + 뼈 마스크 | ✅ (완벽하진 않음) |
| **조준 포즈** | AO 포즈 15 + 오버라이드 2 | ✅ 애디티브 복구됨 |
| **플레이어 조작** | `GM_SoldierLab` — WASD·조준·앉기·달리기 | ✅ |


### 1.2 오늘 규명한 원인 세 가지

전부 "우리 설정이 Epic 전제와 안 맞았던 것"이지 GASP/Lyra 결함이 아니었다.

| 증상 | 원인 |
|---|---|
| 이동 중 발 끌림 | 기본 `gait`가 `Run`이라 `walkSpeeds`가 **한 번도 안 쓰였다.** 583cm/s 클립을 500으로 재생 → **[C-52] 해결** |
| Idle 0.5~1초 주기 움찔 + 좌회전 | **Epic의 비용 편향은 클립 227개를 전제**한다. idle 1개인 우리 세트에선 반대로 작동 → `../CLAUDE.md` P18 |
| 조준 상하좌우 무반응 | 데이터·배선 모두 정상이었고 **컴파일/저장 반영 문제**였다 |

---

## 2. 프로젝트 현황

| 항목 | 상태 |
|---|---|
| UE 프로젝트 | ✅ 생성 완료 (UE5.8, GASP 기반, C++ 전환됨) |
| **소스컨트롤** | ✅ **Perforce(P4V)로 확정.** `.p4ignore` 작성 완료 (프로젝트 루트) |
| 플러그인 | ⚠️ GASP 기본 + StateTree 툴셋. **AI 계열(SmartObjects·NavCorridor·FullBodyIK 등) 미확인** → `CLAUDE.md` 7.3절 |
| GASP 동작 | ✅ PIE 확인 — 관성 이동·정지·제자리회전·벤치 상호작용 전부 자연스러움 |
| MCP | 포트 8001. **PC를 옮기면 Auto Start Server를 다시 켜야 함** → `CLAUDE.md` 7.4절 |

> **다른 PC에서 처음 시작한다면 `CLAUDE.md` 7절을 먼저 볼 것.**
> 특히 `Binaries/`가 P4에서 제외돼 있어 **C++ 빌드를 먼저 해야 에디터가 열린다.**

---

## 3. 이번 세션(2026-09-01~03)에 확정된 것

### 3.1 P0-1은 사실상 통과했다

**궤적이 플레이어 입력이 아니라 캐릭터 이동 상태에서 생성된다.**
`PoseSearchGenerateTrajectory(forCharacter)`가 캐릭터를 통째로 받는 엔진 함수라, AI가 CMC를
구동하면 동일 경로로 궤적이 나온다. 설계에서 "최대 리스크"로 잡았던 `USoldierTrajectorySource`
자체 구현이 **불필요**하다.

AI가 손댈 것은 정확히 2개: `PreviousDesiredControllerYaw`(위협 방향) + `TrajectoryGenerationData`
재튜닝.

### 3.2 워핑이 무엇을 커버하는지 확정 — 조달 계획이 정밀해졌다

Epic이 GASP 그래프에 남긴 주석으로 확정:

| 카테고리 | Orientation Warping | 결론 |
|---|---|---|
| **정상상태 루프**(직선 이동) | ✅ 커버 — "전진 보행 하나로 여러 각도 스트레이프" | 견착 루프는 **전방 위주 소수 클립으로 충분** |
| **start / stop / pivot / turn** | ❌ 구조적으로 못 함 (직선 구간에서만 동작) | **데이터 필수 — 여기가 진짜 비용** |

### 3.3 반입 클립 필수 커브 3종

```
contact_l / contact_r   발 심기 타이밍     없으면 발 IK 오작동
Enable_Warping          워핑 허용 구간     없으면 방향 커버 0
Disable_AO (선택)       조준 억제 구간
```

**이게 디자인팀 요청 스펙의 핵심 항목이 된다.**

### 3.4 조달 요청의 성격이 바뀌었다

> ~~"라이플 견착 로코모션 클립 수백 개"~~
> → **"라이플 견착 정지 포즈 42개 + 무기 자세 앵커 3개 + 전환 클립 소수"**

조준 공간 전체가 **7 yaw × 3 pitch × 2 자세 = 42 정지 포즈**로 정의된다(로코모션 1,450개의 3%).
정지 포즈는 루트모션도 접지 커브도 필요 없어 **제작 난이도가 완전히 다르다.**

### 3.5 GASP는 AI 샘플이기도 하다

`STT_FindSmartObject → STT_ClaimSlot → STT_UseSmartObject` 흐름이 **중첩 상태**로 구현돼 있다.
부모 상태가 자원을 보유하므로 **상태를 벗어나면 예약이 자동 해제**된다 — 설계에서 "사망 시
가장 위험한 항목"으로 표시했던 SmartObject 예약 누수가 **구조로 해결**된다.

---

## 4. 다음 작업 — 우선순위 순

### ✅ P0-4 · 반입 파이프라인 검증 — **완료 (2026-09-04)**

> **결과**: `Walking_Anim`이 `enable_warping`·`contact_l`·`contact_r`·`MoveData_Speed`·`Phase`
> + `L`/`R` 싱크마커를 갖고 `SK_UEFN_Mannequin` 위에서 루트모션으로 재생된다.
> `PSD_Soldier_Walk_Test` 인덱싱 성공(경고 0). **GASP `M_Relaxed_Walk_Loop_F`와 구성이 동일하다.**
>
> 절차·설정값은 **`animation/prototypes/2026-09-04_p0-4_complete.md`**. 아래는 진행 이력이다.

<details>
<summary>진행 이력 (접힘)</summary>

### ① P0-4 · 반입 파이프라인 검증 (1일) ★ 최우선

**이제 모든 것의 전제가 됐다.** 커브 3종을 만들 수 있는지가 조달 계획 전체를 좌우한다.

> **2026-09-03 진전**: 최대 리스크였던 `Enable_Warping` **생성 수단 문제는 끝났다.**
> GASP에 `AM_WarpingAlpha` 모디파이어가 이미 있다 → **[C-25] 해결**.
> `animation/prototypes/2026-09-03_enable_warping_curve_generation.md`
> 남은 것은 "수단이 있는가"가 아니라 **"우리 클립에서 쓸 만한 값이 나오는가"**([C-26]/[C-27]).

**순서를 지킬 것 — 바꾸면 커브가 조용히 망가진다:**

- [x] **클립 확보·반입 완료 (2026-09-03)** — `Walking.fbx` (비인플레이스 확인: 골반 순이동
      136.6cm / 1.367s / 99.9cm·s⁻¹ / 완전 순환 루프). ★ **"In Place" 체크 해제**가 필수
      · ~~인플레이스~~ 는 틀린 지시였다 → `assets/2026-09-02_...md` 14절
      · `Rifle Aiming Idle.fbx`는 순이동 0 → **커브 검증엔 못 쓴다**(견착 앵커용)
      · 반입 위치 `/Game/SoldierLab/Source/Mixamo/` → `Walking_Anim` / `Walking_Skeleton` / `Walking`
        (42키 / 1.366667s — 원본과 일치, 리샘플 없음)
- [x] `Walking_Anim` 전진 확인 (임포트에서 Translation 살아있음)
- [x] **IK Rig `IK_Mixamo`** — auto generate. 체인·손가락 정상, **수정 불필요**
      · IK Goal이 일부 `None`이어도 무관 — **소스 리그에서는 안 읽힌다**(전수 확인)
      · `Root Motion: Hips`도 그대로 둘 것 — 비우면 op이 초기화에 실패한다
- [x] **IK Retargeter `RTG_Mixamo_to_UEFN`** — 타깃은 기존 `IK_UEFN_Mannequin` 재사용
      · ⚠⚠ 새 리타기터는 **op 스택이 비어 있다.** `Op Stack` 탭 → **`Add Default Ops`** 를
        먼저 눌러야 한다. **안 누르면 Auto Align이 아무것도 안 한다**(체인 매핑이 없어서)
      · 리타깃 포즈: `Source` 선택 → `Auto Align ▼ → Align All Bones`(방식 **`Default`**) → T→A 정렬
      · ⚠ **Root Motion op은 꺼둠** — 켜면 골반이 바닥에 붙는다 **[C-32] 원인 미규명**
- [x] **`AM_EncodeRootBone` → 루트모션 합성** ← ★ 워핑 커브보다 먼저
      · `/Game/SoldierLab/AnimModifiers/AM_EncodeRootBone` (pelvis 1.0 / **회전 비움 [C-31]**)
      · **루트 전진 확인, 발 미끄러짐 없음**
- [x] `bEnableRootMotion = true`
- [x] 산출물 `/Game/SoldierLab/Animations/Walking_Anim` — **`SK_UEFN_Mannequin`**, 42키, 1.366667s
- [x] `AM_WarpingAlpha_Soldier` 적용 → `Walking_Anim`은 **전 구간 1** ✅ 규약대로
      · GASP 원본을 복제해 우리 폴더에 둠(임계값 튜닝 대비)
- [x] **커브 세트가 클립 종류별로 다르다는 것 확인** → **[C-34]** 매핑표 필요
      · 정지·제자리회전 클립에는 `Enable_Warping`을 **만들지 않는 것이 정답**
      · `Turning_Right_90_Degrees_Anim`에 실수로 걸었음 → **Revert 필요**
- [x] **[C-28] 해결** — `contact_l/r`은 **이진 사각파**. `MotionExtractor`로는 못 만든다

- [x] **[C-33] 해결 — `contact_l`/`contact_r` 생성** (2026-09-04)
      · **C++ 에디터 모듈 `SoldierLabEditor` 신설** + `UFootContactCurveModifier` 자작
      · 엔진 기본 모디파이어로는 불가능했다(연속값 / 걸음당 노티파이 1개)
      · ★ 핵심은 **발별 자동 높이 보정** — 리타깃이 좌우 발 높이를 다르게 만든다 **[C-35]**
      · `animation/prototypes/2026-09-04_foot_contact_curve_modifier.md`

**현재 `Walking_Anim`: `enable_warping` · `contact_l` · `contact_r` (5종 중 3종)**

- [ ] `AM_FootSteps_Walk` → 발자국 노티파이 (`phase`의 선행 조건)
      · ⚠ 싱크마커도 필요하면 `bShouldGenerateSyncMarkers`를 켜야 한다 (GASP 기본은 꺼짐)
- [ ] `AM_MoveData_Speed` → `MoveData_Speed`
- [ ] `AM_BakePhaseCurveFromFootstepNotifies` → `phase` (루프·전환 클립만)
- [ ] PSD 편입 → MM으로 재생

**판정**: 발 미끄러짐 없이 재생되고, 워핑이 실제로 켜지는가
→ **전반부(반입·리타깃·루트모션) 통과. 후반부(커브 5종)가 남았다.**

#### 확보한 클립 (2026-09-03)

| 파일 | 내용 | 쓸 곳 |
|---|---|---|
| `~/Downloads/Rifle Aiming Idle.fbx` | Mixamo 표준 리그(65본, 루트=`mixamorig:Hips`, Root 없음), 스킨 포함, FBX 7700 | **리타깃 경로 검증 전용** + 3.4절 **42개 정지 포즈의 첫 앵커**(Stand·정면·0°) |
| (미확보) 라이플 견착 walk, **비인플레이스** | | **커브 검증([C-26]/[C-27])에 필수** |

> **왜 idle로는 커브 검증이 안 되나**: 이동이 없으면 ① `EncodeRootBone`이 옮길 이동이 없고
> ② 발 접지 구간이 없어 `contact_l/r`이 평평하며 ③ 회전·이동 오차가 0이라
> `AM_WarpingAlpha`가 **전 구간 1**을 뱉는다. 이게 바로 [C-27]의 고장 신호와 **같은 값**이라
> 정상인지 고장인지 구분할 수 없다. **판정력이 0이다.**

</details>

### ② P0-0 잔여 셋업 (반나절)

- [x] **플러그인 감사 완료 (2026-09-03, 이관 PC)** — `.uproject` + 엔진 `.uplugin` 대조

  | 상태 | 플러그인 |
  |---|---|
  | ✅ `.uproject`에 명시 | SmartObjects, GameplayBehaviorSmartObjects, AnimationBudgetAllocator, AnimationWarping, PoseSearch, Chooser, MotionTrajectory, GameplayInteractions |
  | ✅ 엔진 기본 활성 | **AnimationModifierLibrary**, AnimationSharing |
  | ✅ 의존성으로 따라옴 | StateTree, GameplayStateTree, **NavCorridor**, ContextualAnimation ← `GameplayInteractions`가 끌어온다 |
  | ❌ **꺼져 있음 — 켤 것** | **FullBodyIK**, **SignificanceManager**, **GameplayInsights**(Rewind Debugger) |

- [x] **플러그인 항목 완료 (2026-09-04)**
      · `FullBodyIK` — `IKRig`/`ControlRigModules` **의존성으로 이미 활성**
      · `SignificanceManager` — `AnimationSharing`(엔진 기본 활성) 의존성으로 이미 활성
      · `GameplayInsights` — `.uproject`에 추가. **Rewind Debugger 동작 확인됨**
      · ⚠ 브라우저 검색어는 `GameplayInsights`가 아니라 **"Animation Insights"**(FriendlyName)
- [ ] 프로젝트 설정 — NavMesh Dynamic, EQS 프레임 예산 3ms, Cover 트레이스 채널
- [ ] P4 워크스페이스 등록 + `p4 set P4IGNORE=.p4ignore` 후 초기 제출
- [ ] Rewind Debugger / Pose Search Debugger 켜서 동작 확인

### ✅ P0-1 · AI가 MM 구동 — **완료 (2026-09-04)**

> **판정**: `SandboxCharacter_CMC_C_1` + `AIC_Soldier` + `ST_Soldier_SmartObject`로
> **AI가 조준 방향을 유지한 채 이동**하고, 그 과정의 옆걸음·뒷걸음·급선회에서 **자세가 무너지지 않는다.**
> → **[C-1] 잠정 통과, [W4](TrajectoryGenerationData 재튜닝) 불필요.**
>
> **우리가 만든 것**(`/Game/SoldierLab/AI/`):
> `STT_SetSoldierInputState`(5개 축 노출) · `STT_FocusToPlayer` ·
> `ST_Soldier_Patrol_Subtree` · `ST_Soldier_SmartObject` · `AIC_Soldier`
>
> ⚠ **과대해석 금지**: GASP 비무장 DB는 8방향이 완비돼 있다. **데이터가 충분한 조건에서의 결과**이며,
> 전방 위주 소수 클립뿐인 견착 DB에서는 다시 봐야 한다 → **[C-44]**, P0-2의 본 판정.
>
> 전문: `ai/prototypes/2026-09-04_p0-1_ai_drives_mm.md`

<details>
<summary>진행 이력 (접힘)</summary>

### ③ P0-1 · AI가 MM 구동 (2~3일)

> **2026-09-04 조사: GASP에 이미 구현돼 있다.** 배선조차 대부분 불필요하다.
>
> `/Game/Blueprints/AI/` 에 `AIC_NPC_SmartObject` + StateTree 3종 + 태스크 6종이 있고,
> 그중 **`STT_SetCharacterInputState`** 가 핵심이다:
>
> ```lisp
> parent : StateTreeTaskBlueprintBase        vars : Character, WantsToWalk
> (event EventEnterState
>   (bind _pawn (CastToBPI_SandboxCharacter_Pawn (GetCharacter)))
>   (Setters|Set_CharacterInputState _pawn false (GetWantstoWalk) false false false))
> ```
>
> **AI가 플레이어와 똑같은 인터페이스(`BPI_SandboxCharacter_Pawn::Set_CharacterInputState`)로
> input state를 채운다.** 즉 AI → inputState → CMC → 궤적 → MM 경로가 **이미 하나로 통일돼 있다.**
> 3.1절의 "`USoldierTrajectorySource` 자체 구현 불필요" 결론이 에셋 수준에서도 확인됐다.
>
> **`BPI_SandboxCharacter_Pawn`의 다른 함수들도 우리 설계와 맞물린다:**
> `AddTarget` / `RemoveTarget`(표적 지정) · `Get_MMIResult` · `Get_PropertiesForAnimation`(ABP가 읽는 창구)
>
> → **P0-1은 "만드는 일"이 아니라 "확인하고 한계를 재는 일"이다.**

궤적 문제가 해결됐으므로 **배선 작업**이다.
- [ ] GASP 캐릭터에 AIController 부착, `inputState`를 AI가 채우도록 교체
- [ ] `SetFocus`/컨트롤 로테이션으로 조준 구동
- [ ] `PreviousDesiredControllerYaw` 공급
- [ ] 급선회 품질 확인 → `TrajectoryGenerationData` 재튜닝 **[C-1]**
  - ⚠️ Epic이 Steering 노드를 "작업 중, 원치 않는 거동 있음"으로 표기 — 첫 번째 의심 대상

</details>

### ✅ [C-34] 반입 매핑표 — **완료 (2026-09-04)**

> **`animation/prototypes/2026-09-04_c34_clip_curve_mapping.md` 4절이 확정표다.** 996클립 전수 실측.
> 클립 8종류 × 모디파이어 7단계. **대량 반입 전에 굳혀야 했던 항목이 닫혔다.**
>
> 핵심은 표가 아니라 **"Epic의 데이터에는 일관된 표가 없다"** 는 사실이다 — 같은 "걸으며 90° 회전"이
> 커브 3종/4종/6종으로 갈린다. 복제하지 말고 소비하는 쪽 기준으로 균일 적용한다.
>
> 부수 확정: **[C-36] 해결**(회전 클립에 `contact_l/r` 생성됨) · **[C-37] 확정**(스티어링 커브
> 생성기가 모디파이어 18개 어디에도 없음) · **[C-45] 해결**(→ 아래) ·
> 신규 **[C-46]**(`AM_Copy_IKFootRoot` 누락) · **[C-47]** · **[C-48]**
>
> **[C-45] 스티어링 커브 = 손 저작.** 클립 길이·회전각과 무관하게 항상 같다 —
> `steeringtargettime` **1.0 상수**, `enable_turninplacesteering` **1.0 → 0.0 @ 0.5s 계단**.
> 자작 모디파이어가 필요 없다. 레시피는 위 문서 5.0c절.
>
> **[C-48] ★ 회전이 안 끊기는 장치는 커브가 아니라 노티파이였다.** GASP 회전 클립에는
> `Pose Search: Block Transition In`(뒷구간 — MM이 **새로 진입 못 함**)과
> `Override Continuing Pose Cost Bias`(앞구간 — 유지 편향)가 붙어 있다. 우리 클립엔 없다.
> **P0-2에서 [C-44](급선회 품질)를 판정할 때 이게 교란 변수다** — 노티파이 없이 나온 결과를
> "견착 데이터 부족 탓"으로 오독할 수 있다.
>
> ⚠ 문서 두 곳을 정정했다: 매니페스트 8절의 "전 클립 공통 `contact_l/r`"과 "루프는 L/R 마커"가
> 둘 다 틀렸고(커브 0개 클립 179개 / 마커 보유 클립 4개), P0-4 문서의 PSD 설정은 [Q11] 전환 전 값이었다.

### ✅ 견착 전진 보행 — **정상 작동 (2026-09-09)**

> AI 병사가 견착 클립으로 **발 미끄러짐 없이 걷는다.** 원인이 하나가 아니라 **여섯 개가 겹쳐**
> 있었고, 전부 우리 쪽 에셋·설정 결함이었다. **GASP 시스템은 한 줄도 안 바꿨다.**
> 전체 기록: **`animation/prototypes/2026-09-09_walk_quality_debugging.md`**
>
> | # | 원인 |
> |---|---|
> | 1 | `ik_foot_*` 미채움 → Orientation Warping 무효 **[C-46]** |
> | 2 | Stride Warping 노드 부재 (설계 [2]층 요구사항, GASP엔 없음) |
> | 3 | **`bForceRootLock = false`** → 포즈에 루트 이동이 남아 루프 시 순간이동 ★최대 |
> | 4 | 루트 facing 34° 오차 (pelvis 블레이드가 루트에 구워짐) |
> | 5 | 진단 공식 90° 오류 — GASP 대조군이 `sd 0.0`으로 뱉어 발각 |
> | 6 | **레벨 인스턴스의 `WalkSpeeds` 오버라이드**가 CDO 변경 4회를 전부 가림 **[C-50]** ★ |
>
> **반입 파이프라인에 3단계가 추가됐다** — `SoldierRootFacingModifier`, `AM_Copy_IKFootRoot`,
> 그리고 클립 설정 2종(`bForceRootLock`, 커브 압축). 매핑표 4절에 반영.
>
> ⚠ **아직 우회책이 남아 있다** — MM 파라미터 3개와 PSD `Disable Reselection`을
> GASP 기본값으로 되돌릴 수 있는지 확인해야 한다 → **[C-49]**

### ★ 다음 작업 (2026-09-09 갱신)

| 순 | 항목 | 사용자 손이 필요한 부분 |
|---|---|---|
| 1 | **로우레디** — `AO_CD` 델타를 뼈 마스크로 상체에만 적용해 `BlendListByBool_0`의 비조준 분기에 꽂는다 | `BM_LowReady` 블렌드 마스크 생성 + 노드 3개 배치 (설정은 MCP) |
| 2 | **사격·재장전 배선** — 몽타주 95개가 이미 들어와 있다 | **소총 메시를 `ik_hand_gun`에 부착**하는 것이 선행 조건 |
| 3 | **[C-44] 급선회 판정** | ~~`rotationRate.yaw = 360` 상수가 GASP 기본값임을 확인했다~~ → **정정**: GASP 부모는 접지 시 **`(0, −1, 0)` = 즉시 회전**, 공중 200이다. 그리고 **2026-09-12부터 우리가 이 값을 매 틱 덮어쓴다**(무기 자세에 따라 90~720 °/s) → `IMPLEMENTED.md` 2.5d |
| 4 | **[C-58]** 옆·뒤·앉기 속도 실측 | 커브 호버 5회 |
| ~~5~~ | ~~**AI 상위 계층** — 인지·위협평가·엄폐·분대~~ | **✅ 절반 완료 (2026-09-13)** — 인지·무전·제압·교전·엄폐가 **초안이 아닌 다른 형태로** 들어갔다. **분대·명령은 그대로 남았다** |
| ~~6~~ | ~~★★ **목표(objective) 층**~~ | **✅ 코드 완료 (2026-09-13)** — `ASoldierObjective` + 세 비용 스코어러 → [D10] |
| **6b** | ★★ **목표 액터를 `L_SoldierTest` 에 놓는다** — 아군 소유 1개면 공격·수비가 동시에 생긴다. 그 전까지 6번은 **코드로만 존재한다** | **에디터 배치 1회** (⚠ [P53] — 기본값은 도구로 못 넣는다) → **[W25]**, 그 뒤 **[C-88]** |
| 7 | **데미지·체력·사망** — 병사가 죽지 않아 교전이 끝나지 않는다 | 클립 19개는 이미 반입돼 있다 → **[W18]** |
| 8 | **45명 성능 측정** | 시험 레벨은 있다(`L_SoldierTest`). 측정 절차가 없다 → **[C-83]** · [D3] |

> **[C-56] Lyra 원본 삭제는 취소됐다.** `Actions`에 사격·재장전·피격·사망이 들어 있었고,
> 앞으로도 조달처로 남는다. 컴파일 에러(`FootstepEffectTagModifier`)는 쿠킹 전에만 정리하면 된다.

<details>
<summary>이전 P0-2 정의 (접힘)</summary>

### ④ P0-2 재정의 · 견착 이동 품질 (2일)

> **선행 조건이 하나 붙었다 (2026-09-04)**: 견착 클립을 재생시키려면 **ABP/챙터 배선**이 필요하다.
> `rotationMode == Aim`은 상태값일 뿐이고, 어떤 DB를 조회할지는 `CHT_PoseSearchDatabases`와
> ABP가 정한다. **클립을 DB에 넣어도 챙터가 안 가리키면 안 뽑힌다.** 계획에 없던 작업량이다.
>
> **재료는 준비됨**: `Walking_Anim`(견착 walk, 커브 5종) · `Rifle_Aiming_Idle_Anim`(견착 정지,
> 리타깃 완료) · `Turning_Right_90_Degrees_Anim`
>
> **본 판정은 [C-44] + [C-24]** — 방향 커버리지가 부족한 견착 DB에서 급선회가 버티는가.

"워핑 각도 한계"가 아니라 **"전환 데이터 없이 견착 이동이 얼마나 버티는가"** 로 바뀌었다.
- [ ] 소총 메시를 `ik_hand_gun`에 부착, 임시 견착 포즈
- [x] 루프는 워핑으로 커버되는지 확인 — **Lyra 4방향 루프 확보로 워핑 의존이 줄었다**
- [x] **전환(출발/정지/급선회)에서 얼마나 무너지는지 측정** — Lyra가 Start/Stop/Pivot을 방향별로
      제공해 **조달 규모 문제 자체가 해소**됐다. 남은 것은 급선회(Spin) 클립뿐이다

</details>

### ⑤ P0-3 · 엄폐 슬롯 루프 (1~2일)

GASP의 `FindSmartObject → ClaimSlot → UseSmartObject` 중첩 패턴을 그대로 쓰고 **탐색만 EQS로 교체**.

---

## 5. 막혀 있는 것 / 결정 대기

| # | 항목 | 상태 |
|---|---|---|
| ~~Q4~~ | ~~소스컨트롤~~ | ✅ **Perforce(P4V) 확정** (2026-09-03) |
| ~~Q8~~ | ~~캐릭터 메시 (기존 리스킨 / 신규 / MetaHuman)~~ | ✅ **아군 확정 (2026-09-13)** — 기존 리스킨 `soldier_T` 를 `SK_UEFN_Mannequin` 에 Assign Skeleton. 적군은 **[Q42]** |
| **Q42** | **적군 메시 경로** — A) 디자인팀에 "UEFN 마네킹 리그 · 마네킹 비율 피팅 · LOD" 스펙 (권장, 아군도 같은 스펙으로 재납품) / B) Blender 리스킨 / C) 당분간 마네킹 | **결정 대기.** 참조 리그는 `SKM_UEFN_Mannequin` FBX export |
| Q10 | 견착 전환 동작 조달 방안 (A/B/C안) | P0-2 결과 후 |
| Q7 | 디자인팀 공지 시점 | P0 완료 후 비교 영상과 함께 |

---

## 6. 협업 상태

- 디자인팀은 현재 **기존 22종 시퀀스 정리 + 기존 스켈레톤 스킨 웨이팅 수정** 중
- 그 작업은 **낭비가 아니다** — `titan_example` 현재 빌드의 품질을 올리는 작업
- 리그 교체 제안은 **한 번 보류된 상태**. P0 검증 + 비교 영상을 만든 뒤 재제안
- 타임박스: **1주**. 안 되면 접고 현행 유지 (`assets/...` 11절)

---

## 7. 리스크

| 리스크 | 완화 |
|---|---|
| ~~`Enable_Warping` 커브를 자동 생성 못 하면 방향 커버가 0~~ | ✅ **해소** — `AM_WarpingAlpha` 확보 |
| ~~`contact_l/r`을 만들 수단이 없다~~ | ✅ **해소** — `UFootContactCurveModifier` 자작 **[C-33]** |
| 리타깃이 **좌우 발 높이를 다르게** 만든다 | 커브는 자동 보정으로 우회했으나 **발 IK 품질에는 남아 있다** **[C-35]** |
| 클립 종류별 커브 세트를 잘못 적용하면 조용히 틀린다 | **[C-34]** 매핑표를 대량 반입 **전에** 확정 |
| 견착 전환 데이터를 못 구하면 이동 품질이 무너짐 | 복수 DB 반환을 이용한 **점진적 폴백**(총내림 클립 혼합) — `animation/..._gasp_abp_analysis.md` 15.3절 |
| 리그 교체가 무산될 수 있음 | **설계 7~12절(AI 시스템)은 스켈레톤과 무관** — 전체의 75%가 생존 |
| Steering 노드가 실험적 | P0-1에서 급선회 품질 관찰 시 첫 의심 대상 |
