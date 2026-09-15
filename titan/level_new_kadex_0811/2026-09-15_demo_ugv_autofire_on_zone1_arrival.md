# 데모 모드 UGV RCWS 자동사격(탐색 스윕) 시작 시점 — 레벨 시작 → 1차 목적지 도착

2026-09-15 / 진행중(코드·DT 완료, 빌드 후 EffectType 설정 + PIE 검증 대기) / 데모 모드에서 UGV RCWS를 레벨 시작 1초 뒤에 ARM+AutoFire로 강제하던 것을, 새 DT 행 `UGVArriveZone1`(ActorStopped)이 1차 목적지 도착 시 켜도록 이동. 이동형지휘소는 그대로 레벨 시작 시.

관련: `2026-09-01_scenario_run_modes_demo_fullsystem.md`(데모 모드 §3.2),
`scenario_authoring_guide.md`(DT 저작 — 이펙트/행 표 갱신됨),
`2026-09-10_scenario_auto_restart_design.md`(재시작 설계 — §8 참고).

---

## 1. 요청 / 증상

데모 모드(`RunMode=Demo`)로 켜면 UGV 포탑이 **시나리오 시작하자마자**(레벨 시작 1초 뒤) 좌우로
탐색 스윕(자동정찰)을 돌기 시작한다. 아직 출발도 안 한 UGV가 정찰 중인 것처럼 보이는 게 어색해서,
**UGV가 1차 목적지에 도착한 뒤에** 스윕이 시작되도록 바꿔 달라는 요청.

## 2. 원인

`AScenarioConfig::BeginPlay` → `RegisterScenarioConfig` → 1초 뒤 `ApplyDemoRunModeSetup()` →
`ApplyDemoRCWSAutoFire()`가 **UGV와 이동형지휘소 둘 다** ARM + `AutoFire` + `FireMode(DemoFireMode)`로
강제했다(`bDemoForceUGVAutoFire`/`bDemoForceCommandPostAutoFire`가 true일 때). `AutoFire` 모드는
타겟이 없으면 `UpdateSearchSweep`(탐색 스윕)을 돌리므로, UGV 포탑은 출발 전부터 스윕 중이었다.

DT의 `UGVSurveillance`/`UGVAutoFire` 행은 `bEnabled=false`(FullSystem에선 통제기 SW가 사격 주체)라
원인이 아니다 — 스윕은 전적으로 데모 강제 경로에서 왔다.

## 3. 변경

### 3.1 코드 (3파일, 전부 P4 체크아웃 완료)

| 파일 | 내용 |
|---|---|
| `UI/ScenarioStepTypes.h` | `EScenarioEffectType::SetDemoUGVAutoFire` 신설(`SetCommandPostAutoFire` 뒤). **데모 전용**: `IsDemoMode() && bDemoForceUGVAutoFire`일 때만 UGV RCWS를 ARM + AutoFire + FireMode(`DemoFireMode`)로 전환, 아니면 로그만 남기고 no-op |
| `UI/ScenarioStateSubsystem.h/.cpp` | `ApplyDemoRCWSAutoFire()` 안의 람다를 private 멤버 `ApplyDemoRCWSAutoFireTo(AActor* Owner, const TCHAR* Label) const`로 추출. `ApplyDemoRCWSAutoFire()`(레벨 시작)는 이제 **이동형지휘소만** 처리(UGV 분기 삭제). `ExecuteScenarioEffect`에 `case SetDemoUGVAutoFire` 추가 — 데모 게이트 통과 후 `ApplyDemoRCWSAutoFireTo(ResolveUGVPawn(GetWorld()), TEXT("UGV"))` |
| `UI/ScenarioConfig.h` | `bDemoForceUGVAutoFire` 주석 갱신 — 이제 on/off 게이트일 뿐이고 **시점은 DT 행이 정한다** |

데모 시작 로그 줄도 바뀜:
`RCWS 자동사격 강제(UGV=켬[1차 목적지 도착 시, SetDemoUGVAutoFire 행], 지휘소=켬[레벨 시작 시])`.

**왜 기존 `SetUGVAutoFire`를 안 쓰고 새 이펙트를 만들었나**: `SetUGVAutoFire`는 실행 모드를 안 보고
무조건 AutoFire로 바꾸므로, 그 행을 켜두면 FullSystem에서 통제기와 충돌한다. `SetDemoUGVAutoFire`는
게이트가 이펙트 안에 있어서 **FullSystem과 같은 DT를 공유해도 행을 켜둔 채 둘 수 있다**(재시작 설계의
`RestartScenario` 이펙트와 같은 원칙).

### 3.2 DataTable (`/Game/Scenario/DT_ScenarioSteps_ThreeStage`, 체크아웃·저장 완료)

| RowName | Prereq | Trigger | 값 | Effect | bEnabled |
|---|---|---|---|---|---|
| `UGVArriveZone1` | `UAVSpotted` | `ActorStopped` | 0 | **`None`(⚠️ 임시)** → 빌드 후 `SetDemoUGVAutoFire`로 | true |

DebugLabel: "UGV 1차 목적지 도착 -> (데모) RCWS ARM+자동사격, 탐색 스윕(자동정찰) 시작".

> ⚠️ **EffectType이 아직 `None`이다.** 새 enum 값은 C++ 재빌드 전엔 에디터에 존재하지 않아서
> 넣을 수 없었다. 빌드 뒤 반드시 설정할 것(§6). 그리고 이 DT는 작업 당시
> `user2@user2_jiseong`도 동시에 열어두고 있었으니 P4 제출 전 충돌 확인.

## 4. 왜 `ActorStopped`로 되는가

`UAVSpotted` 행의 이펙트 `MoveUGVToZone1Destination`은 `FormUpLeader=UGV`로 잡고
`AUGVAIController::MoveToDestination`을 부른다. 경로를 찾으면 **그 호출 안에서 동기적으로
`bIsMoving=true`**가 되고, `HandleMoveCompleted`에서 false로 내려간다. 따라서 `UAVSpotted` 발동
이후 처음으로 `!IsMoving()`인 틱 = 1차 목적지 도착. Prereq 충족 이전 상태(출발 전 정지)는 평가되지
않으므로 즉시 발동하지 않는다.

엣지 케이스: 경로 탐색 실패 시 `bIsMoving`이 false로 남아 행이 즉시 발동한다 — RCWS가 어쨌든
ARM되므로 허용 가능한 폴백.

## 5. 동작 변화

- 도착 전 UGV RCWS는 기본 `Remote` 모드(안정화만, 스윕·자동사격 없음). **1차 목적지로 주행 중엔
  적을 쏘지 않는다.**
- 그에 따라 `EnemyEngage`(`UGVFiredNearEnemy`)와 `DroneSeeEnemies → DroneWideView` 체인이 전부
  도착 이후로 밀린다. 시나리오 순서 자체는 그대로.
- 이동형지휘소는 변화 없음(레벨 시작 시 ARM). 어차피 탐지 사거리 400m 밖이라 3차 전까지 안 쏜다
  (`2026-09-01_axis_rcws_autofire_investigation.md`).
- 예전 동작(레벨 시작 즉시)으로 되돌리려면 코드 수정 없이 `UGVArriveZone1` 행을
  **Prereq 없음 + `TimerOnly` 0초**로 바꾸면 된다.

## 6. 남은 작업

1. 사용자 빌드(에디터 재시작).
2. `DT_ScenarioSteps_ThreeStage` ▸ `UGVArriveZone1` 행 ▸ `EffectType = SetDemoUGVAutoFire` (DT 에디터 또는 MCP).
3. DT 저장 (+ P4 충돌 확인).
4. PIE 로그 순서 확인:
   ```
   데모 실행 모드 — ... UGV=켬[1차 목적지 도착 시 ...]
   데모 자동사격: 이동형지휘소(...) RCWS를 ARM + AutoFire...      ← 시작 시 UGV 줄이 없어야 함
   시나리오 스텝 발동: UAVSpotted
   [UGVAIController] Move completed: Success
   시나리오 스텝 발동: UGVArriveZone1
   데모 자동사격: UGV(...) RCWS를 ARM + AutoFire + FireMode=...로 전환.
   ```
   그리고 UGV 포탑이 도착 전엔 가만히 있고 도착 후 스윕을 시작하는지 육안 확인.

## 7. 재시작 설계와의 관계

`2026-09-10_scenario_auto_restart_design.md`는 "재시작 후 `ApplyDemoRunModeSetup()` 재실행만 하면
RCWS 복구"를 전제로 썼다. 이번 변경 후엔:

- 지휘소 절반은 여전히 그 재실행으로 복구된다.
- UGV 절반은 `FiredScenarioSteps`가 비워지므로 다음 사이클에 `UGVArriveZone1` 행이 다시 발동해
  복구된다 — 단, **재시작이 UGV RCWS를 명시적으로 `Remote`로 되돌리지 않으면** 이전 사이클의
  AutoFire 상태가 그대로 남아 2회차에는 출발 전부터 스윕한다(1회차와 다른 연출). 재시작 구현 시
  UGV 리셋(§4d)에 "모드를 `Remote`로" 항목을 넣을 것. 해당 문서에 메모 추가함.
