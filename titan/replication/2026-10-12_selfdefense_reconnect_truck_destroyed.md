# 자체방호 클라 재접속/늦은 접속 버그 3종 (트럭 파괴 · UGV 정지 · 트럭 RCWS 수동 잔류)

2026-10-12 / 완료(CL 36·37 제출, 2-PC 재접속 검증 "이제 잘된다") / 클라 퇴장 시 엔진 기본 PawnLeavingGame이 트럭을 Destroy했고, 서버의 원격 PC BeginPlay가 주행 중 UGV를 Manual로 세웠으며, 퇴장자의 트럭 RCWS 수동 모드가 서버에 남았다 — 셋 다 수정.

## 요약

| # | 증상 | 원인 | 수정 | CL |
|---|---|---|---|---|
| 1 | 재접속 클라: 트럭 RCWS 뷰가 월드 원점, 전장 카메라·트럭 CCTV 4개 검정(드론만 정상) | 클라 퇴장 → 엔진 기본 `PawnLeavingGame()` 이 빙의 중이던 **레벨 배치 트럭을 Destroy** | `PawnLeavingGame` 오버라이드(레벨 배치 폰은 UnPossess만) | 36 |
| 1' | (잠재) 클라 비정상 종료 직후 재접속 시 트럭 빙의 실패 | `PossessAxisVehicle` 이 "다른 컨트롤러가 빙의 중"이면 건너뜀 — 타임아웃 전 옛 PC가 쥐고 있음 | 다른 PlayerController가 쥐고 있으면 새 접속자에게 인계 | 36 |
| 2 | 재접속(또는 늦은 접속) 직후 서버의 주행 중 UGV 정지 → 시나리오 진행 정지(드론도 다음 스텝 대기로 멈춤) | 서버에서 원격 PC의 BeginPlay가 PlayerAxis 결정 **전에** 돌아 "UGV축 기본값" 블록의 `SetUGVFromTankMode("Manual")` 실행 | 해당 블록을 로컬 PC로 한정, 서버 상태 변경은 권한 있는 로컬 PC(호스트/솔로)에서만 | 37 |
| 3 | 트럭 RCWS 자동정찰(좌우 스캔) 영구 정지 | 수동(TruckRCWS)을 켠 채 퇴장 → 서버에 `Remote` 잔류 → 다음 접속자의 수동 on/off가 그 `Remote` 로 "복귀" | 퇴장(`EndPlay` Destroyed) 시 서버가 수동 조종 대상을 None으로 복원 | 37 |

## 1 — 퇴장 시 트럭 파괴

**보고 증상**: host 가동 중 자체방호 client가 **두 번째로 접속**하면 트럭 RCWS 뷰(Monitor2 메인뷰) 카메라가 월드 원점, 전장
카메라 + 트럭 CCTV 4개 완전 검정, 드론 짐벌 화면은 정상.

**원인**
1. `GameMode::PostLogin → PossessAxisVehicle` 이 자체방호 PC에 `BP_TitanTruck` 을 Possess.
2. 클라가 나가면 서버에서 그 PC가 파괴되고, 엔진 `APlayerController::Destroyed()` 가 (`Player == NULL && Authority`)
   `PawnLeavingGame()` 호출(`UE_5.8/Engine/Source/Runtime/Engine/Private/PlayerController.cpp:1633-1646`).
3. 기본 `PawnLeavingGame()` = `GetPawn()->Destroy()`(같은 파일 `:1572-1579`). 프로젝트 PC가 오버라이드하지 않아
   **레벨 배치 공용 트럭이 서버 월드에서 영구 삭제**. 호스트 쪽 트럭 로직(RCWS·미니맵 마커 등)도 같이 사라짐.
4. 재접속 클라: 맵 로드로 `BP_TitanTruck_C_4` 가 생겨 BeginPlay(캡처·RT·RTSP)까지 돌지만 곧 서버의 "삭제된 스타트업 액터"
   정보로 파괴됨. 서버엔 트럭이 없어 빙의 실패 → 뷰타겟이 PC 자신(원점).
5. 드론은 `SetOwner` 만 하고 빙의하지 않으므로 무사 — 사용자 관찰과 일치.

**증거** — 클라 로그 `C:\working\outputs\kadex_1012_windows_2\Windows\titan_example\Saved\Logs`
("첫 프레임" = `LogRtspAsyncEncoder: [diag] '<mount>': worker claimed its first slot`)

| 세션(접속 시각) | 직전 상황 | 로딩 워밍업 뷰타겟 | 첫 프레임 송출 스트림 |
|---|---|---|---|
| 00:18 | 호스트 새로 띄운 뒤 첫 접속 | `BP_TitanTruck_C_4` | 7개(트럭 6 + 드론) |
| 00:25 | 00:18 세션 종료 직후 재접속 | **PC 자신, V(0)** | **드론 1개뿐** |
| 00:28 | (호스트 재시작 추정) | `BP_TitanTruck_C_4` | 7개 |
| 01:02 | 00:28 세션 종료 직후 재접속 | **PC 자신, V(0)** | **드론 1개뿐** |

실패 세션은 트럭 마운트 6개가 Registered까지는 되지만 프레임이 한 번도 안 나간다 = 트럭 액터가 BeginPlay 직후 사라짐.

**수정 (CL 36)**
- `titan_examplePlayerController.h/.cpp` — `PawnLeavingGame()` 오버라이드: 빙의 폰이 레벨 배치 액터(`IsNetStartupActor()`)면
  `UnPossess()` 만 하고 파괴 안 함(로그 `PawnLeavingGame: … 파괴하지 않고 빙의만 해제`). 런타임 스폰 폰은 엔진 기본(Destroy) 유지.
  트럭을 하드코딩하지 않은 이유: 레벨 배치 공용 차량 전부에 해당하는 문제이고, 접속자 전용 스폰 폰의 정리 동작은 보존하기 위해.
- `titan_exampleGameMode.cpp` `PossessAxisVehicle` — 차량을 다른 **PlayerController** 가 쥐고 있으면 건너뛰지 않고 새 접속자에게
  인계(로그 `handing over to …`). 비정상 종료한 클라의 옛 연결은 타임아웃 전까지 열린 것처럼 보여 살아있는 접속자와 구분 불가 →
  축당 접속자 1명 전제로 **최신 접속 우선**. 엔진 `OnPossess` 가 옛 컨트롤러를 `UnPossess` 하므로(`PlayerController.cpp:885-887`)
  옛 PC가 나중에 파괴돼도 폰이 없어 트럭은 안전. AI 등 비플레이어 컨트롤러는 예전처럼 건너뜀.

## 2 — 접속할 때마다 서버가 주행 중 UGV를 세움

**보고 증상**(CL 36 적용 후, 호스트=에디터 PIE): 2번째 접속 직후 서버에서 주행 중이던 UGV 정지, 드론 정지, 트럭 RCWS 자동정찰
정지. 서버 로그엔 에러·ensure 없음(평균 32 fps, 틱 정상), `RevealEnemies` 이후 시나리오 로그가 끊김 — 서버 정지가 아니라
**시나리오 진행 조건 대기**. (CL 36 자체는 정상: 서버 `PawnLeavingGame: … 빙의만 해제`, 클라 2차 접속 뷰타겟=트럭·스트림 7/7.)

**원인**: 엔진 `AGameModeBase::Login` 은 `SpawnPlayerController`(→ `FinishSpawningActor` → **BeginPlay**, `GameModeBase.cpp:756-766`)
를 먼저 하고 `InitNewPlayer`(PlayerAxis 결정) 를 나중에 한다. 그래서 **서버에서 원격 PC의 BeginPlay 는 항상 PlayerAxis=Unspecified**
이고, `Atitan_examplePlayerController::BeginPlay` 의 "UGV축 기본값" 블록(`PlayerAxis != SelfDefense`)이 실행돼
`SetUGVFromTankMode("Manual")`(HasAuthority만 확인)가 서버의 UGV AI를 Manual로 바꿈 → `DispatchSetManualControl(0,0)` 로 자율주행 정지.

- **자체방호 접속이 일어날 때마다** 생기던 일. 첫 접속은 UGV가 출발 전(UAVSpotted 이전)이라 안 보였을 뿐.
- 처음 보고된 "호스트가 한참 진행된 뒤 늦게 접속" 조건도 같은 원인으로 시나리오가 선다.
- 드론 정지는 이미 경로 끝(`uavpath` 도착 → 낙하산 관찰 완료)이라 UGV 도착 이후 스텝을 기다리는 것 — UGV 정지의 결과.

**수정 (CL 37, `titan_examplePlayerController.cpp`)**: BeginPlay "UGV축 기본값" 블록에 `IsLocalController()` 조건 추가, 그 안의
서버 상태 변경(`SetRCWSStabilization(false)`, `SetUGVFromTankMode("Manual")`)은 `HasAuthority()`(= 호스트/솔로 자기 PC)에서만.
부수 효과로 자체방호 클라가 자기 BeginPlay에서 `Server_SetRCWSStabilization`(UGV RCWS 안정화 끄기)을 보내던 것도 사라짐.
⚠ UGV축이 **원격 클라**인 구성(현재 없음 — 호스트=UGV축 전제)이 생기면, 그 클라는 시작 시 UGV RCWS 안정화 off 강제를 안 보낸다.

## 3 — 퇴장자의 트럭 RCWS 수동 모드 잔류

**원인**: 서버 로그 1차 접속에서 `[자체방호] 수동 조종 대상 0 -> 1`(1=TruckRCWS, Monitor2 `TruckManualControlCheckBox`) 후
**해제 없이 퇴장** → 트럭 RCWS가 `Remote` 로 서버에 남음. 2차 접속의 `0 -> 1` 이 `TruckModeBeforeManual` 에 그 `Remote` 를
"원래 모드"로 저장, `1 -> 0` 이 `Remote` 로 "복귀" → 자동정찰 영구 정지. 드론 수동 모드(UAV/UAVGimbal)도 같은 구조로 남을 수 있었다.

**수정 (CL 37)**: `EndPlay(EEndPlayReason::Destroyed)` 에서 서버(`HasAuthority`)가 퇴장 PC의 `SelfDefenseManualTarget` 을 `None` 으로
되돌림(`ApplySelfDefenseManualTarget(Old, None)` — 트럭 RCWS 원래 모드 복원, 드론 수동 해제). 레벨/PIE 종료는 다른
EndPlayReason이고 월드째 정리되므로 제외.

## 검증

- CL 36: 에디터 타깃 빌드 성공. 2-PC(호스트=에디터)에서 재접속 시 트럭 유지·뷰타겟 정상·스트림 7/7 확인.
- CL 37: 게임 타깃(`titan_example Win64 Development`) 컴파일 성공(작업 PC 에디터가 Live Coding 중이라 에디터 타깃은 빌드 불가).
  2-PC 재접속에서 시나리오 정상 진행 — 사용자 확인 "이제 잘된다".
- 아직 따로 확인하지 않은 경로: 클라를 작업관리자로 강제 종료 후 즉시 재접속(`handing over` 로그, 1' 경로). 패키징 빌드에서의 재확인.

## 열린 항목

- 두 접속 모두 접속 직후 `수동 조종 대상 0 -> 1`(트럭 RCWS 수동)이 찍혔다. 사용자 클릭인지, 게임패드 버튼(클라 로그에
  `JoystickButton Device=0 Button=0` 다수)이 포커스된 체크박스를 토글한 건지 미확인. 의도치 않은 토글이면 별도 수정 필요.
- (보류) 위젯 재탐색 견고화 — `USelfDefenseMonitor1Widget`/`Monitor2` 는 `NativeConstruct` 에서 `GetActorOfClass` 한 번뿐이다.
  클라에선 `Client_OnAxisResolved`(위젯 생성)가 항상 트럭/드론 BeginPlay보다 먼저 와서 첫 바인딩 때 RT가 null이고,
  `RefreshResolutions` 의 "크기 바뀌면 재바인딩"으로 복구되는 구조다. RT 기본 크기(QuadCam 240×135, 전장 640×360, 드론 1280×720)와
  패널 픽셀 크기가 우연히 같으면 영영 안 붙는 잠재 버그가 있으나 실측 보고 없음 — 증상이 보이면 그때 넣는다.

## 조사 중 기각한 가설

- `FRtspAxisGate` 5초 타임아웃 → 캡처 영구 차단: 실패 로그에서 `axis callback fired, LocalAxis=2` 정상.
  (로컬 로그의 타임아웃 4건은 축 시스템 없는 `L_SoldierTest` PIE.)
- 라운드로빈 Slot ≥ Count(`TitanGraphicsSettings::ApplyCaptureRates`): 항상 Slot < Count.
- 시나리오 재시작 재스폰(`ScenarioRespawnSubsystem`): 트럭/드론은 재스폰이 아니라 제자리 부활, `RespawnActors` 빈 목록.
- 로딩 화면 서브시스템: 워밍업 이펙트만 캡처에서 숨김, 캡처 자체는 안 건드림.
- 2차 정지가 서버 일시정지/행: 프로젝트에 `SetGamePaused`/시간 배율 변경 없음, 서버 로그상 틱·fps 정상.
