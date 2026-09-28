# `slomo N>1`이 New_kadex_0811에서 안 먹는 이유 — 물리 dt 클램프 조사

2026-09-16 / 보류 / `slomo 5`를 쳐도 시뮬이 안 빨라지는 원인은 `MaxPhysicsDeltaTime`(엔진 기본 1/30) 클램프. New_kadex_0811은 20~30fps라 이미 클램프에 닿아 있어 배속 여유가 0. 해결 후보 3개 비교만 하고 구현 안 함 — 실제 요구(6분 시나리오 반복 확인)는 리플레이 툴(`replay_chronicle/`)로 대신 해결됨.

> **폴더 선택 이유**: 이 조사는 특정 차량이 아니라 프로젝트 전체 물리 틱(`UPhysicsSettings`,
> `DefaultEngine.ini`, `UWorld::Tick`) 얘기고, 결론이 "프로젝트 물리 설정을 바꿀 것인가"라는 결정
> 기록이라 `vehicle/ugv/`·`vehicle/drone/` 어느 한쪽이 아니라 `infra_architecture/`(전체
> 아키텍처/인프라 결정 기록)에 둔다. 차량별 영향은 각 폴더 문서를 링크로 가리킨다.

---

## 1. 증상

New_kadex_0811에서 시나리오를 빨리 돌려보려고 콘솔 `slomo 2`/`slomo 5`를 쳤는데 **체감 속도가
전혀 안 바뀜**. 반대로 `slomo 0.5`는 정상적으로 느려짐. 같은 명령이 `/Game/test` 레벨에서는
`slomo 2`까지는 먹고 `slomo 5`는 2배 정도만 빨라짐.

## 2. 원인 — 엔진 호출 사슬 (UE 5.8 소스로 확인)

1. `UWorld::Tick` (`Engine/Source/Runtime/Engine/Private/LevelTick.cpp:1596`) — 프레임
   `DeltaSeconds`에 **먼저 `TimeDilation`을 곱한다**. `slomo 5`면 33ms 프레임이 166ms 게임
   dt가 됨.
2. 그 dt가 `FChaosScene::SetUpForFrame`
   (`Engine/Source/Runtime/PhysicsCore/Private/ChaosScene.cpp:344`)으로 들어가는데, 서브스테핑이
   꺼진 분기에서 **`MDeltaTime = min(dt, MaxPhysicsDeltaTime)`** 으로 잘린다.
3. `UPhysicsSettings::MaxPhysicsDeltaTime` 엔진 기본값 = **1/30 = 33.3ms**. 즉 물리는 한 프레임에
   최대 33.3ms까지만 진행한다. 프레임이 33ms(30fps)면 배율을 얼마를 곱하든 물리는 33.3ms
   그대로 → **배속 0**.
4. `slomo < 1`은 dt가 클램프 아래로 내려가니 그대로 먹는다(그래서 느리게는 됨).

게임 로직(액터 Tick, 시나리오 타이머)은 166ms를 받아 5배로 돌지만 차량/드론은 Chaos 강체라
물리 dt를 따라간다 — 눈에 보이는 "시뮬 속도"는 물리 쪽이라 안 빨라진 것처럼 보인다.

### 2-1. 프로젝트 설정 실측

MCP로 `/Script/Engine.Default__PhysicsSettings`를 라이브로 읽은 값:

| 키 | 값 | 비고 |
|---|---|---|
| `MaxPhysicsDeltaTime` | 0.0333 | 엔진 기본 |
| `bSubstepping` | false | 엔진 기본 |
| `MaxSubstepDeltaTime` | 1/60 | 엔진 기본(서브스테핑 꺼져 있어 무효) |
| `MaxSubsteps` | 6 | 〃 |
| `bTickPhysicsAsync` | false | 엔진 기본 |

**Perforce 확인**: `Config/DefaultEngine.ini` 리비전 31개 전부에 `bSubstepping`/`MaxPhysicsDeltaTime`
키가 **한 번도 들어간 적 없음**. 즉 이 프로젝트는 물리 스텝 설정을 한 번도 손대지 않았고 전부
엔진 기본값이다.

### 2-2. 레벨별 실측 fps와 배속 여유

PIE 로그 프레임 카운터로 계산(측정법은 memory `feedback_unreal_fps_measure_via_log`):

| 레벨 | 실측 fps | 프레임 dt | 클램프(33.3ms)까지 여유 | `slomo 2` | `slomo 5` |
|---|---|---|---|---|---|
| `/Game/test` | ≈58 | 16.7ms | 2.0배 | 정상 2배 | **2배만** |
| `New_kadex_0811` | ≈20~30 (사용자 체감 ~30) | ≈33ms | **1.0배(없음)** | **0** | **0** |

`t.MaxFPS=60`(`DefaultEngine.ini` 7~15행)이 걸려 있어 상한이 60fps이므로, 어떤 레벨이든
배속 여유는 최대 2배다. New_kadex_0811은 그 여유마저 없다.

## 3. 문서 정정 — `vehicle/drone/drone_flight_dev_guide.md` §10.2

§10.2가 "물리 서브스테핑이 꺼져 있고 … **UGV 거동 편차 때문에 의도적으로 설정된 것**"이라고
적혀 있었는데, 이건 `DefaultEngine.ini` 7~15행 주석을 잘못 읽은 것이다. 그 주석은
**`t.MaxFPS=60`을 추가한 이유**(프레임률에 따라 차량 거동이 달라지던 문제, 2026-08-25)를
설명하는 것이고, 서브스테핑은 위 §2-1대로 **건드린 적이 없다**(엔진 기본 false).

다만 우려 자체는 여전히 유효하다 — Chaos 차량 서스펜션은 PBD 컨스트레인트라 강성이 dt²에
비례하므로(`vehicle/ugv/2026-09-10_ugv_0901_suspension_tuning.md` §7 "프레임률 의존성"),
서브스텝 dt를 바꾸면 승차감 튜닝이 같이 바뀐다. §10.2는 오늘 이 내용으로 고쳤다.

## 4. 해결 후보 비교 (구현 안 함)

| # | 방법 | 30fps × `slomo 5`일 때 | 기각/보류 사유 |
|---|---|---|---|
| 1 | `MaxPhysicsDeltaTime`을 올린다 (예: 0.2) | 물리 스텝 1개 = **167ms** | **기각.** 서스펜션 강성 (167/33)² ≈ **25배**, 드론 제어 루프가 프레임당 1회라 시뮬 시간 기준 **6Hz**, 고속 탄/차량 터널링. 물리가 아예 다른 물건이 됨 |
| 2 | 서브스테핑 ON, `MaxSubstepDeltaTime`을 1/60보다 살짝 크게 (예: **1/55 = 18.2ms**), `MaxSubsteps` 충분히 | 33ms×5 = 167ms → **10 × 16.7ms**, 정확히 5배 | **보류.** 60fps에서는 16.7 < 18.2라 여전히 1스텝(=현재와 동일), 30fps는 2×16.7ms로 오히려 60fps와 같은 스텝 크기가 돼 승차감 편차도 사라짐. **위험**: 게임스레드 컨트롤러(UGV AI 속도 PI, 드론 오토파일럿 유도 — 물리 콜백이 아니라 프레임 Tick)는 프레임당 1회라 시뮬 시간 기준 6Hz가 됨, 미검증. 런타임 토글 가능(`PhysLevel.cpp`가 매 프레임 `UPhysicsSettings::Get()`을 읽음)이라 실험 비용은 낮음 |
| 3 | 별도 "데모 배속" 메커니즘 — 물리 dt는 그대로 두고 차량 목표속도/시나리오 타이머 등 세트포인트를 배율로 스케일 | 물리는 정상, 시나리오 진행만 빨라짐 | **보류.** 스코프가 가장 넓음(속도 제한·제동 곡선·탐지 타이머·애니메이션까지 전부 손대야 함). 물리적으로 "5배 빠른 차"가 되는 거라 거동 검증 목적엔 무의미 |

드론 쪽 참고: 드론 커스텀 물리 콜백은 서브스테핑이 켜지면 자동으로 서브스텝마다 돈다
(`drone_flight_dev_guide.md` §10.2) — 후보 2를 실험한다면 드론 PID는 그대로 두고 오토파일럿
유도(프레임 Tick)만 확인하면 된다.

## 5. 결론 / 상태

**보류.** 사용자의 실제 요구는 "6분짜리 시나리오를 매번 처음부터 안 기다리고 특정 장면을
반복해서 보고 싶다"였고, 이건 배속이 아니라 **녹화·재생**으로 푸는 게 맞다 — 같은 날 만든
Chronicle 리플레이 툴(`replay_chronicle/2026-09-16_chronicle_replay_plugin.md`)이 그 역할을
한다(타임라인 스크럽 + 0.5/1/2/5배 재생, 재생 배속은 `demo.TimeDilation`이라 물리 클램프와
무관).

나중에 "실시간 시뮬 자체를 빨리 돌리고 싶다"가 다시 필요해지면 **후보 2**부터 실험할 것.
콘솔에서 `p.` 계열이 아니라 `UPhysicsSettings` 프로퍼티라 MCP `set_properties`로
`/Script/Engine.Default__PhysicsSettings`에 써서 PIE 중 바로 비교할 수 있다.

## 관련 문서

- `vehicle/ugv/2026-09-10_ugv_0901_suspension_tuning.md` §7 — 서스펜션 강성 ∝ dt², 프레임률 의존성.
- `vehicle/drone/drone_flight_dev_guide.md` §10.2 — 드론 제어 루프 60Hz(오늘 정정).
- `replay_chronicle/2026-09-16_chronicle_replay_plugin.md` — 이 요구를 실제로 해결한 툴.
- `Config/DefaultEngine.ini` 7~15행 — `t.MaxFPS=60` 도입 배경(2026-08-25).
