# 루프 클립 이음새 불량 목록 (디자이너 수정 요청)

2026-10-01 / 진행중(디자이너 수정 대기) / 루프 클립의 마지막 프레임과 첫 프레임 포즈가 달라 반복될 때마다 발·손이 한 프레임에 튄다. 아군 8 · 적군 12 클립. 원본 애셋 자체의 문제(우리 코드·가공과 무관함을 확인).

관련: `animation/2026-09-30_locomotion_test_harness.md`(5.11) · 측정 도구 `Source/SoldierLabEditor/AnimModifiers/SoldierRootSpeedTool.cpp`(`seam` 명령).

---

## 1. 증상

조준하고 뛸 때(`Jog_Fwd`) 루프가 처음으로 돌아가는 순간 총이 뚝 끊긴다(사용자 보고). 측정하면 같은 순간에 발도 10~21 cm 튄다. 걷기·앉아걷기 루프도 마찬가지.

## 2. 무엇을 쟀나

루프 애니는 마지막 키에서 바로 첫 키로 넘어가므로, **루트 기준으로 본 포즈가 마지막 키와 첫 키에서 같아야** 끊김이 없다. 각 본에 대해 "첫 키 vs 마지막 키"의 위치(cm)·회전(°) 차이를 쟀다(애님 프리뷰는 병사가 전진해서 눈으로 비교가 어렵다 — 애님 시퀀스의 **Asset Details ▸ Root Motion ▸ Enable Root Motion 체크 해제**로 제자리 재생돼 육안 확인 가능. **확인 후 반드시 다시 체크**). 디자이너 전달본: `2026-10-01_loop_seam_fix_request.html`.

- 정상 기준: 디자이너가 정리한 아군 `Jog_Bwd` · `Jog_Left` · `Jog_Right` · `Walk_Left`, 모든 Idle = **0.0~0.2 cm / 0.5° 이하**.
- 게임 내 확인: 같은 클립들이 루프 경계에서 발 10~21 cm(평소 프레임 상위 5%는 2~6 cm), `Jog_Fwd`는 왼손 17.5 cm · 오른손 9.3 cm 튐. 정상 4클립은 1 cm 안팎.

## 3. 우리 작업과 무관함 확인

- 측정 시점의 아군 클립은 **P4 원본(#1)과 내용 동일**(`p4 diff -sa` 결과 없음), 적군은 체크아웃조차 안 됨.
- 우리가 아군 8클립에 하는 루트 선형화는 루트 기준 포즈를 바꾸지 않는다 — 선형화 전후 이음새 수치가 **8클립 모두 완전히 동일**함을 확인.

## 4. 수정 요청 목록

수치 = 첫 키 vs 마지막 키 차이(루트 기준). "다리 최대"는 발·볼 본.

### 4.1 아군 (`Content/SoldierLab/Animations_Ally/Animations/Rifle/Loops/`)

| 클립 | 다리 최대 | 상체 최대 | 비고 |
|---|---|---|---|
| **`ALLY_MM_Rifle_Jog_Fwd`** | ball_l 22.4 cm / 28.4° | **hand_l 17.4 cm / 28.5°** (왼손 손가락 18~20 cm / 30°) | ★ 총 끊김의 직접 원인. 같은 클립의 적군판은 상체 3~4 cm — 아군판에서 왼손이 달라짐 |
| **`ALLY_MM_Rifle_Crouch_Walk_Bwd`** | ball_r 16.2 cm / 20.0° | **lowerarm_l 0.8 cm / 45.2°** (아래팔 비틀림) | ★ 상체 회전 불량 |
| `ALLY_MM_Rifle_Walk_Fwd` | ball_r 15.2 cm / 19.3° | lowerarm_r 0.9 cm / 1.2° | |
| `ALLY_MM_Rifle_Walk_Bwd` | ball_r 13.4 cm / 14.2° | lowerarm_l 0.7 cm / 2.5° | |
| `ALLY_MM_Rifle_Walk_Right` | ball_l 16.1 cm / 20.5° | pelvis 0.5 cm / 2.8° | |
| `ALLY_MM_Rifle_Crouch_Walk_Fwd` | ball_r 15.2 cm / 20.7° | pelvis 0.4 cm / 2.9° | |
| `ALLY_MM_Rifle_Crouch_Walk_Left` | ball_r 16.8 cm / 17.6° | pelvis 1.0 cm / 3.5° | |
| `ALLY_MM_Rifle_Crouch_Walk_Right` | ball_r 16.6 cm / 22.1° | pelvis 0.9 cm / 2.8° | |

정상: `Jog_Bwd` · `Jog_Left` · `Jog_Right` · `Walk_Left` · `Idle_ADS` · `Idle_Hipfire` · `Crouch_Idle`.

### 4.2 적군 (`Content/SoldierLab/Animations_Enemy/Animations/Rifle/Loops/`) — 루프 12개 전부

| 클립 | 다리 최대 | 상체 최대 |
|---|---|---|
| `Enemy_MM_Rifle_Walk_Fwd` | ball_r 15.2 cm / 19.3° | pelvis 0.6 cm / 1.5° |
| `Enemy_MM_Rifle_Walk_Bwd` | ball_r 13.4 cm / 14.2° | hand_l 1.2 cm / 2.1° |
| `Enemy_MM_Rifle_Walk_Left` | ball_r 18.5 cm / 19.1° | lowerarm_l 1.5 cm / 1.2° |
| `Enemy_MM_Rifle_Walk_Right` | ball_l 16.1 cm / 20.5° | pelvis 0.5 cm / 2.8° |
| `Enemy_MM_Rifle_Jog_Fwd` | ball_l 22.4 cm / 28.4° | lowerarm_r 3.4 cm / 4.0° |
| `Enemy_MM_Rifle_Jog_Bwd` | ball_l 20.0 cm / 18.3° | lowerarm_l 4.1 cm / 4.6° |
| `Enemy_MM_Rifle_Jog_Left` | ball_r 22.0 cm / 16.3° | lowerarm_r 2.0 cm / 2.8° |
| `Enemy_MM_Rifle_Jog_Right` | ball_l 27.6 cm / 29.3° | lowerarm_r 3.0 cm / 4.1° |
| `Enemy_MM_Rifle_Crouch_Walk_Fwd` | ball_r 15.2 cm / 20.7° | lowerarm_r 1.6 cm / 2.0° |
| `Enemy_MM_Rifle_Crouch_Walk_Bwd` | ball_r 16.2 cm / 20.0° | lowerarm_r 1.0 cm / 1.5° |
| `Enemy_MM_Rifle_Crouch_Walk_Left` | ball_r 16.8 cm / 17.6° | pelvis 1.0 cm / 3.5° |
| `Enemy_MM_Rifle_Crouch_Walk_Right` | ball_r 16.6 cm / 22.1° | lowerarm_r 1.4 cm / 2.1° |

정상: 적군 Idle 3개.

## 5. 참고 (수정 시)

- 아군 정상 4클립(`Jog_Bwd`/`Left`/`Right`, `Walk_Left`)은 이음새 0 + **루트 이동이 완전 등속**이다. 불량 클립은 루트 속도도 보폭 안에서 53~74% 출렁인다 → 같은 방식으로 정리해 주면 두 문제가 함께 해결된다. 우리는 그동안 아군 8클립 루트를 코드로 등속화해 쓰고 있다(포즈는 불변) — **새 버전을 받으면 다시 적용**한다.
- 아군 불량 8클립은 `ik_hand_root`(+ `ik_hand_gun`/`_l`/`_r`)가 루트를 따라가지 않고 제자리에 고정돼 있어, 루트 기준으로 이동 거리만큼(`Jog_Fwd` 971 cm) 밀려난다. 적군판·정상 4클립에는 없다. 현재 ABP가 이 본들을 직접 쓰지 않는 것으로 보여 끊김 원인은 아니지만, 다시 굽을 때 같이 바로잡는 것이 좋다.
- **적군 뛰기 루프는 발 미끄러짐도 크다**(2026-10-01, 루트 등속화·스냅 적용 후 자동 테스트, 미끄러짐 지표 cm/s): 적군 뛰기 중앙 34.9 / 최대 63(W 43 · SD 63 · SA 46 · WA 48) vs 아군 뛰기 17.7 / 20. 아군은 뛰기 루프 4개 중 3개(`Jog_Bwd`/`Left`/`Right`)가 정리된 버전이다 → 적군 뛰기 루프도 아군 정리본과 같은 수준으로 정리 요청. 총내림 상태 재측정(`all`, 적군 BP): 총내림 뛰기 35.0 / 43.5(아군 20.1 / 20.7), 총내림 걷기 서기 19.2 / 20.9(아군 8.2 / 9.9) — 총내림 클립도 같은 경향.
- ~~참고: 적군 클립은 접지 커브(`contact_l/r`)가 살아 있다~~ → **정정(2026-10-01, 클립별 측정)**: 아군·적군 Loop 모두 접지 커브가 대부분 망가져 있다(뛰기 0~4%, 걷기 0~17%). 원인은 우리 모디파이어 임계값 — **디자이너 몫이 아니라 우리가 재생성**한다(C-179). 발 미끄러짐도 그 뒤 재측정해 남는 것만 요청.
- 재측정: 에디터에서 `Saved/AnimTool.request`에 `seam <애셋경로>` 한 줄 → `Saved/AnimTool.done`.
