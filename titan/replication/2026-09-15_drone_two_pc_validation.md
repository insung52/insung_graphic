# 드론 리플리케이션 — 2 PC 실환경 검증과 버그 3건

2026-09-15 / 완료 / 2대 PC 실환경 첫 검증. 풀·데모 양쪽에서 "전혀 리플리케이션 안 됨" → 버그 3건(데모 이중 주체, AI 자동 빙의로 Server RPC 조용히 폐기, 서버 주체일 때 Rep* 미게시) 수정 후 양쪽 모두 정상 확인.

선행: `2026-09-01_drone_client_authoritative.md`(설계 배경·배선). 현재 동작은
`vehicle/drone/drone_flight_dev_guide.md` 15절.

---

## 1. 증상 (사용자 리포트)

1. 서버에서 먼저 드론 자율비행이 출발한 뒤 클라가 들어오면 — 서버 드론은 이미 날아갔는데 클라
   드론은 **출발 위치에서 뒤늦게 새로 출발**.
2. 풀 시스템에서 클라가 수동 조종하면 — 클라에서는 잘 움직이는데 **서버에서는 그냥 가만히**.

구성: 서버 = UGV축 PC(로비 → Host, PIE), 클라 = 자체방호축 PC(로비 → Client). 이 조합은
2026-09-01 이후 한 번도 실환경에서 돌려본 적이 없었다.

---

## 2. 버그 (a) — 데모 모드에서 서버·클라 **둘 다** 시뮬 주체

서버 로그(`Host … Demo=1`): `시뮬레이션 주체=이 프로세스 (물리 켬)` → 데모 자동 시작 → 자율비행.
3초 뒤 클라 접속 → 클라 로그: `시뮬레이션 주체=이 프로세스` + 출발점부터 `자율비행 시작`.

`ResolveShouldSimulateDrone`의 판정 순서가 문제였다:

```
② SelfDefense/Unspecified 면 true     ← 클라가 여기서 true
③ 리슨서버 + 데모 면 true              ← 서버가 여기서 true
```

데모 서버에 자체방호 클라가 붙으면 서버는 ③, 클라는 ②로 **각자 물리를 돌린다.** 09-01 문서에는
"양쪽이 같은 `bDemoRunMode`를 보므로 판정이 안 갈린다"고 써 있었지만 코드가 그렇게 돼 있지
않았다 — 데모 검사가 리슨서버에서만, 그것도 ② 뒤에서 돌았다.

**수정**: 데모 검사를 먼저 하고 `데모면 서버만 주체, 클라는 원격`.

```
① Standalone → true
② 데모 모드   → NetMode == ListenServer   (클라는 false)
③ 풀 시스템   → SelfDefense/Unspecified → true
```

클라의 데모 판정은 서버가 GameState에 리플리케이트한 `bDemoRunMode`에 달려 있는데, 축 판정
콜백 시점에 GameState가 아직 안 왔을 수 있다(둘 다 접속 직후 첫 리플리케이션에 실리지만 순서
보장 없음). `ResolveSimulationAuthorityForAxis`가 클라에서 GameState가 없으면 0.1초 × 최대 50회
기다렸다가 판정한다. 잘못 판정하면 두 프로세스가 동시에 물리를 돌리는 상황이라 몇 프레임
기다리는 쪽이 싸다.

---

## 3. 버그 (b) — 풀 시스템에서 `Server_ReportState`가 **조용히** 폐기됨

가장 오래 걸린 것. 서버 로그(`Host … Demo=0`)는 전부 정상으로 보였다:

```
[Drone] 시뮬레이션 주체=원격 (물리 끔). 서버권한=예
PostLogin: 드론 소유권을 자체방호축 PC로 지정 — Server RPC 경로 확보.
[Drone] 수동 조종 모드 0 → 2            ← 클라의 PC Server RPC는 통과
```

클라 로그에도 경고가 하나도 없었다. 그런데 `Server_ReportState`는 서버에서 한 번도 실행되지
않았다.

### 3.1 원인 사슬 (엔진 소스로 확인)

1. 레벨에 놓인 `BP_Drone`은 `AutoPossessAI = PlacedInWorld`(APawn **엔진 기본값**) → 서버가 레벨
   로드 때 `AAIController`를 만들어 **빙의**시킨다. 드론은 AI 컨트롤러를 전혀 안 쓰는데도.
2. `APawn::GetNetConnection()`은 Owner보다 **Controller를 우선** 본다:
   ```cpp
   if (GetController()) return GetController()->GetNetConnection();   // AI → Owner 없음 → nullptr
   return Super::GetNetConnection();                                   // 여기까지 안 옴
   ```
   PostLogin의 `SetOwner(자체방호 PC)`는 무력화.
3. 서버는 받은 Server RPC를 `UNetDriver::ShouldCallRemoteFunction`에서
   `RepFlags.bNetOwner`(= `Actor->GetNetConnection() == 보낸 연결`)일 때만 실행한다.
   불일치 → `LogRep Verbose "Rejected unwanted function"` — **기본 로그 레벨에선 안 보인다.**

PC 자신의 Server RPC(수동 모드 전환)는 PC가 연결 소유자라 통과했고, 드론 폰의 RPC만 버려졌다.
09-01 문서 §4 "소유권 함정"은 **Owner만** 신경 썼는데, 폰은 Controller가 먼저다.

### 3.2 수정

- 생성자 `AutoPossessAI = EAutoPossessAI::Disabled`.
- `ADronePawn::GetNetConnection()` 오버라이드 → `AActor::GetNetConnection()`(Owner 사슬만).
  BP가 AutoPossessAI를 덮어썼거나 나중에 뭔가 빙의하더라도 안전. 빙의로 조종하는 `L_DroneTest`
  경로는 `PossessedBy`가 Owner도 그 PC로 잡아주니 결과가 같다.

> 확인: 빌드 후 `BP_Drone ▸ Pawn ▸ Auto Possess AI`가 Disabled로 보이는지. BP가 직접 값을
> 덮어썼으면 C++ 기본값이 안 먹는다(오버라이드가 있으니 동작엔 지장 없음).

---

## 4. 버그 (c) — 데모 + 클라: 서버가 주체일 때 Rep*를 아무도 안 채움

(a)·(b) 수정 후 재검증. 풀은 완전 정상. 데모는 클라가 이제 정상적으로 `원격`으로 판정됐는데,
**클라 드론이 출발 위치에 그대로 굳어 있음**(속도/고도 숫자만 변함).

`RepLocation`/`RepRotation`/`RepVelocity`/짐벌/줌을 채우는 곳이 `Server_ReportState_Implementation`
**하나뿐**이었고, 그 RPC는 클라만 보낸다(`!HasAuthority()`). 리슨서버 자신이 날면 아무도 Rep*를
안 써서 클라의 원격 보간은 초기값(출발 위치)에 머문다. 데모 + 클라 조합은 원래 코드에서 한 번도
안 만들어진 경로다.

**수정**: Tick 끝의 게시 경로를 갈랐다.

```cpp
if (보고 주기 && HasAuthority())        // 리슨서버가 주체(데모) — RPC 없이 Rep*에 직접 씀
    RepLocation = …; RepRotation = …; RepVelocity = …; RepGimbal…; RepZoomLevel = …;
else if (보고 주기)                      // 시뮬 클라(풀) — 기존 RPC
    Server_ReportState(…);
```

같이: 원격 프로세스의 상태 패널(고도/속도/방위)이 `RepVelocity`로 채워지게
`UpdateStatusHUDFlightData(VelocityMS)` 인자화 — 전엔 원격에서 아예 안 불렀다.

---

## 5. 추가 수정 / 진단

### 5.1 늦게 붙은 클라의 경로 명령 레이스

서버가 이미 경로를 명령해 둔 뒤 접속하면 `CommandedPathId`가 초기 리플리케이션으로 오는데, 그
OnRep이 축 판정보다 먼저 도착하면(순서 보장 없음) `bSimulationAuthority`가 아직 false라 명령이
버려진다. `ApplySimulationAuthority`에서 주체가 된 시점에 대기 중인 명령을 집행한다
(`Autopilot->IsEngaged()`가 false이고 `CommandedPathId`가 있으면).

### 5.2 진단 로그 2종 — 다음에 또 "안 움직임"이 나오면 이것부터

| 어디 | 로그 | 의미 |
|---|---|---|
| 시뮬 클라, 첫 송신 1회 | `[Drone] Server_ReportState 첫 송신 — Owner=…, Controller=…, NetConnection=…, 액터채널=…, NetReady=…` | 엔진이 Server RPC를 **로그 없이** 버리는 두 조건(Owner 사슬에 연결 없음 / 클라에 액터 채널 없음)을 찍는다 |
| 서버, 첫 도착 + 5초마다 | `[Drone] Server_ReportState 수신 N회 — 위치 …, 시뮬주체=…` | 도착 여부 + 서버가 주체로 잘못 잡혔는지(`!! 이중 시뮬`) |

읽는 법: 서버에 `수신` 줄이 없으면 송신 단계(클라 첫 송신 줄의 `없음(!!)` 확인), 있는데
`이중 시뮬`이면 판정 문제, 있고 원격인데도 안 움직이면 서버 쪽 적용 문제.

`시뮬레이션 주체=…` 줄에 `Owner=`도 같이 찍는다.

---

## 6. 왜 주체가 모드별로 다른가 (설계 정리)

네트워크 구조(서버=UGV축 리슨서버, 클라=자체방호축)는 데모든 풀이든 똑같다. 다른 건 **"드론
물리를 어느 프로세스가 돌리느냐"** 하나뿐이고, 제약 두 개가 충돌해서 갈린다.

| 구성 | 주체 | 이유 | Rep* 채우는 곳 |
|---|---|---|---|
| 풀 (`Demo=0`) | **자체방호 클라** | 클라가 드론을 수동 조종한다. 서버 권위면 입력이 서버 왕복 후 반영 → 조종 지연. 엔진 예측/롤백은 5.8 WIP + async physics 요구(RTSP 지연·UGV 거동 영향) | `Server_ReportState` RPC → 서버 |
| 데모 (`Demo=1`) | **리슨서버** | 자체방호 클라가 없는 1 PC 구성이 정상 상태 — 클라 주체 규칙이면 물리를 돌릴 프로세스가 없어 드론이 영영 안 뜸(09-01 버그). 전 구간 자율비행이라 조종 지연 문제 없음 | 서버 Tick에서 직접 |

"자체방호 클라가 붙어 있느냐"가 아니라 "데모 모드냐"로 판정하는 이유: 접속 여부로 주체를 바꾸면
클라가 늦게 붙거나 떨어질 때 **물리 주체를 실시간으로 넘기는 핸드오버**가 필요해진다. 모드는 세션
시작 때 고정된 값이라 판정이 정적이고 양쪽이 항상 같은 답을 낸다. 주체는 세상에 **정확히 하나**.

데모에 클라가 붙은 경우 클라의 수동 조종은 PC Server RPC로 서버 드론에 들어가고 결과가
되돌아온다 — 동작은 하되 약간 지연. 데모에선 그 정도면 된다.

---

## 7. 검증 결과 (2026-09-15, 2대 PC 실환경)

| 구성 | 서버 로그 | 클라 로그 | 결과 |
|---|---|---|---|
| 풀 (`Demo=0`) 수동 조종 | `Server_ReportState 수신 191회 — … 시뮬주체=원격` | `첫 송신 — Owner=BP_TestPlayerController_C_0, Controller=None, NetConnection=있음, 액터채널=있음, NetReady=예` | ✅ 서버 화면이 클라 조종을 따라감 |
| 데모 (`Demo=1`) 늦은 접속 | `시뮬레이션 주체=이 프로세스` + 자율비행 | `시뮬레이션 주체=원격 (물리 끔)` | ✅ (c) 수정 후 클라 드론이 서버를 따라감 |

09-01 문서 §5 "2대 PC 실환경 ❌ 미검증" 항목 해소.

남은 것: `RemoteInterpSpeed`(12/s) 보간 품질은 아직 튜닝 안 함 — 화면상 문제 보고 없음.
