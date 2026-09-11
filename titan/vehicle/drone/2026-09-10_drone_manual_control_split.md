# 드론 수동 조종 — 비행/짐벌 축 분리

2026-09-10 / 완료 / 수동 조종을 비행·짐벌 두 축으로 분리해 "카메라만 수동" 모드 신설. 그 과정에서 수동 해제 시 출발 위치로 576km/h 역주행하던 버그와 교전 관측 상태 유실 버그를 잡음.

작업 경과 기록. "지금 어떻게 동작하는가"는 같은 폴더 `drone_flight_dev_guide.md` 17절 참고.

선행: `2026-09-05_drone_engagement_observation.md`(교전 관측 이동).

---

## 1. 요구

기존 자체방호축 수동 조종은 `ESelfDefenseManualTarget::UAV` 하나로 **비행 + 짐벌을 한 덩어리로**
넘겨받았다. 사용자 요구는 "**드론 짐벌 카메라만** 수동 on/off 하는 버튼을 하나 더" — 기체는
자율비행/교전 관측을 그대로 계속하고 사람은 카메라만 돌려보는 조합이 전시에서 가장 많이 쓰인다.

상호배타(다른 걸 켜면 꺼짐)는 기존 토글들과 동일하게.

---

## 2. 설계

폰의 수동 상태를 bool 하나에서 **모드 enum**으로 바꿨다. 상호배타가 enum이라 공짜이면서도,
효과를 거는 쪽에서는 **두 질문으로 나눠서** 본다.

```cpp
enum class EDroneManualControlMode { None, GimbalOnly, Full };

bool IsFlightManuallyControlled() const { return Mode == Full; }            // 자율비행을 놓았나
bool IsGimbalManuallyControlled() const { return Mode != None; }            // 자동 조준이 짐벌을 안 잡나
```

| | 자율비행 | 교전 관측 이동 | 짐벌 자동 조준 | 비행 스틱 | 짐벌 입력 |
|---|---|---|---|---|---|
| `None` | ● | ● | ● | ✕ | ✕ |
| `GimbalOnly` | ● | ● | ✕ | ✕ | ● |
| `Full` | ✕ | ✕ | ✕ | ● | ● |

컨트롤러 쪽은 `ESelfDefenseManualTarget`에 `UAVGimbal`을 추가했다.

---

## 3. 겪은 함정

### 3.1 짐벌 자동 로직 안에 비행 로직이 들어 있었다

`UpdateGimbalWideEngagementView`가 짐벌 조준뿐 아니라 **관측 지점 선정(비행)** 까지 한다. 기존
코드처럼 "수동이면 UpdateGimbalRecon을 통째로 건너뛴다"를 그대로 두면, 카메라만 넘겨받았는데
**기체가 관측 지점 갱신을 멈춰 제자리에 굳는다.**

함수 중간에서 갈랐다 — 비행 몫은 항상 돌고, 짐벌을 실제로 돌리는 부분부터 `return`.

### 3.2 짐벌 입력이 `GimbalReconPhase == Idle`일 때만 통과

`ApplyUAVGimbalPanTiltInput`의 오래된 가드다. 원래 목적은 "자동 슬루와 스틱이 같은 각도를
다투면 카메라가 떨린다"(AUAVPawn 실측)인데, 교전 중에는 항상 `WideEngagementView`라
**정작 짐벌을 돌려보고 싶은 구간에서 입력이 통째로 버려진다.**

수동 모드에선 자동 쪽이 짐벌을 아예 안 건드리므로 다툴 상대가 없다 →
`IsGimbalManuallyControlled()`면 단계와 무관하게 통과.

### 3.3 짐벌 버튼이 비행 IMC 안에 있었다 — "버튼 켰는데 아무 반응 없음"

가장 오래 헤맨 것. 짐벌 4방향 버튼(`DroneGimbalUp/Down/Left/Right`)이
`DroneManualMappingContext` 안에 있는데, 그 IMC를 **전체 수동일 때만** 올리고 있었다.

```cpp
ApplyDroneManualMappingContext(NewTarget == ESelfDefenseManualTarget::UAV);  // GimbalOnly → false
```

IMC가 안 올라가니 액션 자체가 발화하지 않는다. 로그상 모드 전환은 정상이라
(`[Drone] 수동 조종 모드 0 → 1 (비행 자동, 짐벌 수동)`) 상태 기계만 보면 멀쩡해 보였다.

짐벌만 수동일 때도 IMC를 올리도록 바꿨다. 같이 올라오는 비행 액션은
`SendDroneManualFlightInput`이 `!= UAV`면 버리므로 기체는 안 움직인다 — 결과적으로 **두
모드에서 짐벌 조작 방법이 동일**하고 비행만 빠진다.

### 3.4 그런데 IMC를 올리면 메인 스틱이 죽는다

IMC가 우선순위로 메인 스틱을 `DroneCyclicAction`에 묶어버려서 `DoCameraLook`이 막힌다. 비행
입력은 위에서 버려지고 짐벌로도 안 가니 **스틱이 통째로 무반응.** `SendDroneManualFlightInput`
에서 GimbalOnly일 때 그 스틱을 짐벌 pan/tilt로 직접 돌렸다 — 이 모드에선 스틱과 4방향 버튼이
둘 다 카메라를 움직인다.

---

## 4. 부호 사슬 — 같은 축을 두 용도로 쓰는 대가

극성 문제로 두 번 왕복했다. 원인은 **같은 물리 축(Extreme 3D Pro `Axis_1`)을 비행과 카메라
두 용도로 나눠 쓰는데, 둘이 원하는 극성이 반대**라는 것.

기존 `bInvertPitchAxis`(BP_Drone)로는 못 푼다 — 그건 빙의 경로와 라우팅 경로에 **공통으로**
걸리는데, 두 경로가 서로 다른 IMC를 쓰고 그 IMC들의 `Axis_1` 극성이 다르다
(`IMC_DroneTest`에는 Negate가 있고 `DroneManualMappingContext`에는 없다).

그래서 라우팅 경로 전용 노브 둘을 뒀다. 최종 부호 사슬(앞으로 밀면 raw = **-1**, IMC Negate 없음):

```
비행 : raw(-1) × bInvertDroneManualCyclicPitch(-1) × bInvertPitchAxis(-1) = Pitch -1
       → 기수 내림 = 앞으로 기울기                                          ✔
짐벌 : raw(-1) × bInvertDroneGimbalOnlyTilt(+1) = -1
       → GimbalPitchDeg 감소 = 아래를 봄                                    ✔
```

> ⚠ **IMC에 Negate를 추가/제거하면 두 노브를 같이 뒤집어야 한다.** 한쪽만 맞추면 다른 쪽이
> 반대가 된다 — 실제로 비행을 맞춘 뒤 짐벌이 반대가 되어 한 번 더 왕복했다.

---

## 5. 수동 해제 시 576km/h 역주행 (별건 버그)

수동으로 날리다가 끄면 **뒤로 100km/h 넘게 가속해 출발 위치로 되돌아갔다가** 다시 자율비행을
시작했다.

원인: 수동 진입 때 `Autopilot->Disengage()`로 `State`가 `Idle`이 되는데, 해제할 때
`BeginPathFollowingById`가 그 `Idle`을 보고 **"지상에서 새로 시작"으로 판정**해 이륙 단계로
들어간다. 이륙은 스플라인 **첫 포인트**를 목표로 잡고 속도 상한을 거기까지의 거리에서 역산한다:

```
첫 포인트까지 800m ÷ TakeoffDurationSeconds 5초 = 16000cm/s = 576km/h
```

`bInFlightReroute`를 "자율비행 중이었나"(State != Idle)로 판정한 게 잘못이었다 — 실제로 알아야
하는 건 **"지금 공중에 떠 있나"** 인데, 수동 조종이 그 둘을 갈라놓는다.

`ResumeAfterManualControl(PathId, bResumeObserving)`을 신설했다. 이륙 단계를 건너뛰고 현재
위치를 경로에 투영해 가장 가까운 지점부터 이어받는다.

### 5.1 교전 트래킹에는 같은 버그가 **두 겹**이었다

사용자가 "교전 상황 트래킹에도 같은 버그 있을 수 있으니 확인"이라고 짚어준 대로, 확인해보니:

1. 위와 같은 이륙 폭주
2. 그에 더해 **`Observing` 상태를 잃고 경로 추종으로 돌아가** 경로 끝(낙하산 자리)까지
   주행한다 — 교전을 두고 반대쪽으로 날아간다

수동 진입 시 `Autopilot->IsObserving()`을 스냅샷해뒀다가(`Disengage` **전에** 잡아야 한다 —
그 뒤엔 Idle이라 알 수 없다) 복귀 시 관측으로 되돌린다.

---

## 6. 짐벌 기본 자세 복귀

사용자 요청: 수동으로 돌려놓은 카메라가 자동으로 넘어간 뒤 기본 방향(기체 진행 방향에서 살짝
아래)으로 천천히 돌아오게.

**사용자가 먼저 제안한 "스켈레탈 메시의 CamPitch 본을 돌려 기본값을 바꾼다"는 방법은 문제를
안 푼다.** `GimbalPitchDeg`는 바인드 포즈로부터의 **누적 오프셋**이라, 본을 돌리면 "0이 어디냐"만
바뀌고 사용자가 돌려놓은 값은 그대로 남는다(= 복귀가 안 된다). 게다가 `Min/MaxGimbalPitchDegrees`,
`GimbalScanPitchDegrees`, `GimbalReconMaxPitchDegrees`가 전부 rest 기준이라 같이 어긋난다.

"천천히 복귀"는 어차피 코드가 필요하고, 그게 생기면 기본 각도는 자연히 그 로직의 파라미터다.
`GimbalHome*` 프로퍼티로 넣고, **정찰 단계가 `Idle`일 때만** 돈다 — 자율비행 중이고 아무도
짐벌을 안 잡는 구간이 정확히 거기 하나뿐이다. 다른 단계는 각자 조준하므로 복귀가 끼어들면 다툰다.

---

## 7. 남은 것

- [ ] `DroneManualMappingContext`의 `Axis_1`에 Negate를 추가하고 두 반전 노브를 끄면 양쪽 IMC
      극성이 같아져 더 깔끔하다(4절). 코드 노브로 이미 동작하므로 급하지 않고, MCP로 IMC를
      건드렸다가 헤맨 이력이 있어 에디터에서 직접 하는 게 안전하다.
- [ ] 수동 해제 직후 짐벌 복귀가 **즉시** 시작된다. 조작 실수처럼 보이면 지연 시간을 넣을 것.
