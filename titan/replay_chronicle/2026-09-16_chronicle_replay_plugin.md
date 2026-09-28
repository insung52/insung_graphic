# Chronicle — PIE 리플레이 녹화/재생 에디터 플러그인

2026-09-16 / 완료 / 엔진 Replay System(DemoNetDriver)을 디자이너가 버튼으로 쓰게 만든 에디터 전용 플러그인 `Plugins/Chronicle`. PIE 자동 녹화, 목록/재생, 타임라인 스크럽, 배속, 카메라 헬퍼. 만들면서 밟은 엔진 함정 12건과 리플리케이션 사각지대 1건 기록.

> **2026-09-17 추기**: 후속 문서 `2026-09-17_replay_respawn_and_physics_proxy_fixes.md` — 체크포인트
> 재스폰이 CDO를 드러내는 문제(적군 AnimClass), 물리 프록시 지터/스크럽 뒤 정지 수정, UI 변경
> (드롭다운=텔레포트, Go/Follow, Free=현재 시점). §4-2의 "리플레이 = 클라이언트 하나 더" 가설로
> 찾은 게임 코드 리플리케이션 결함 4건은 `replication/2026-09-17_enemy_anim_death_replication_gaps.md`.
> 낙하산 건은 여전히 미확인.

> **사용법은 여기 없다.** 사용자 가이드는 플러그인 안에 있음 —
> `Plugins/Chronicle/README.md`(마크다운) / `Plugins/Chronicle/Chronicle_Guide.html`(같은 내용,
> 브라우저용). 이 문서는 "왜 이렇게 만들었고 엔진의 어디에 걸려 넘어졌는가"만 다룬다.
>
> 이 폴더(`replay_chronicle/`)는 "시스템 하나당 폴더 하나" 규칙에 따라 오늘 신설. 리플레이 툴은
> 리플리케이션(`replication/`)을 소비하지만 별개 시스템이고, 게임 모듈 의존성도 없다.

---

## 1. 배경 — 왜 만들었나

New_kadex_0811 시나리오는 한 바퀴가 6분이라 "그 장면"을 확인하려면 매번 처음부터 기다려야 했다.
처음엔 `slomo 5`로 빨리 돌리려 했는데 물리 dt 클램프 때문에 New_kadex_0811에서는 배속이 전혀
안 먹는다(`infra_architecture/2026-09-16_slomo_physics_dt_clamp_investigation.md`). 사용자의 실제
요구는 "빨리 돌리기"가 아니라 "반복해서 보기"였으므로 녹화·재생으로 방향을 바꿨다.

### 1-1. Rewind Debugger가 아니라 Replay System인 이유

| | Rewind Debugger | Replay System (DemoNetDriver) |
|---|---|---|
| 방식 | 기록된 트랜스폼/애님 포즈를 덮어씀 | 리플리케이션 스트림으로 **게임 로직을 다시 돌림** |
| Niagara/사운드/UI | **안 나옴** | Multicast로 재생되는 발사/피격 이펙트 그대로 나옴 |
| 재생 환경 | 에디터 뷰포트 | **살아있는 게임 월드(PIE) 필요** |

이 프로젝트의 발사/피격 이펙트는 전부 Multicast라 Replay System이어야 의미가 있다. 대신 재생에도
PIE가 필요해서 툴이 PIE를 대신 띄운다.

## 2. 구조

```
Plugins/Chronicle/                       에디터 전용, 게임 모듈 의존성 없음 (insung52 작성)
  Chronicle.uplugin
  Source/Chronicle/
    Chronicle.Build.cs
    Public/ChronicleSubsystem.h          UEditorSubsystem — 리모컨 본체
    Public/ChroniclePanelWidget.h        UEditorUtilityWidget 파생, 패널 로직(BindWidget)
    Public/ChronicleReplayRowWidget.h    목록 행 위젯
    Private/ChronicleSubsystem.cpp
    Private/ChroniclePanelWidget.cpp
    Private/ChronicleReplayRowWidget.cpp
    Private/ChronicleModule.cpp          Tools ▸ Chronicle (Replay Panel) 메뉴, bIsEnabledInPIE 강제
  Content/EUW_ChroniclePanel.uasset      사용자가 만든 EUW, 부모를 C++ 위젯으로 재지정 — 그래프 없음
  Content/EUW_ChronicleReplayRow.uasset  〃
```

- **`UChronicleSubsystem`** (`Config=EditorPerProjectUserSettings`): 설정(`bAutoRecordOnPIE`,
  `AutoRecordPrefix`, `RecordHz=30`, `bStartPaused`), 녹화(`StartPIEAndRecord`/`RecordNow`/
  `StopRecording`), 재생(`PlayReplay`/`StopPIE`/`Scrub`/`SetSpeed`/`SetPaused`), 상태 조회, 파일
  (`Saved/Demos/*.replay` 목록·삭제), 카메라(`GoToActorByName`=액터 뒤 8m·위 3m로 자유 카메라
  텔레포트, `ViewActorByName`=`SetViewTarget`, `ViewSelf`=현재 시점에서 분리). 전부 BP 노출.
- 내부 동작은 엔진 콘솔 명령과 동일 경로: `demorec`/`demoplay`/`demostop`/`demoscrub <sec>`/
  `demopause`/`demospeed <x>` (`World.cpp:5699-5719`), 배속은 `demo.TimeDilation` cvar.
- **재생 상태 유지 루프** `TickPlaybackState`(FTSTicker, 매 프레임): 원하는 일시정지 상태, 따라갈
  액터(이름), 마지막 스펙테이터 트랜스폼을 매 프레임 다시 적용한다. 이유는 아래 함정 6.
- 위젯은 로직 0줄 — `BindWidget`으로 위젯 이름만 고정하고 C++가 전부 처리.

## 3. 엔진 함정 12건 (UE 5.8 소스 기준, 실측)

1. **Standalone PIE로 녹화하면 Multicast RPC가 통째로 빠진다.** 게임 NetDriver가 없으면
   `UWorld::InternalGetNetMode()`(`World.cpp:9607`)가 데모 드라이버의 넷모드를 **데모 드라이버
   틱 안에서만** 돌려주고, 그 밖에서는 NM_Standalone → `AActor::GetFunctionCallspace`가 RPC를
   Local로만 처리해 DemoNetDriver에 안 닿는다. 증상: 리플레이에 적/아군/UGV/트럭 RCWS 발사
   이펙트 전부 없음. **해결: PIE Net Mode = Play As Listen Server.** 툴은 강제하지 않고
   `GetWarningText()`로 빨간 경고만 띄운다.
2. **`demo.RecordHz` 엔진 기본값이 5.8에서 8Hz**(`DemoNetDriver.cpp:51`) → 차량이 뚝뚝 끊김.
   툴이 녹화 직전에 30으로 세팅.
3. 콘솔 명령 `demorec/demoplay/demostop/demoscrub <sec>/demopause/demospeed <x>`가 전부 존재
   (`World.cpp:5699-5719`), 배속 cvar는 `demo.TimeDilation`. 툴은 이 경로를 C++로 직접 부른다.
4. **`UEditorUtilityWidgetBlueprint::bIsEnabledInPIE` 기본 false** → PIE가 뜨는 순간 EUW 탭
   전체가 회색(`EditorUtilityWidgetBlueprint.cpp:146 IsWidgetEnabled`). 툴은 Tools 메뉴로 열 때
   리플렉션으로 강제 on, 에셋 Class Settings에도 켜 둠(콘텐츠 브라우저에서 직접 Run하는 경우 대비).
5. **`USlider::SetValue()`가 `OnValueChanged`를 브로드캐스트**(`Slider.cpp:162`). 재생 시간
   표시로 슬라이더를 갱신하면 그게 다시 스크럽 콜백을 부르고 → `GotoTimeInSeconds` → 액터 전부
   리스폰 → 다시 SetValue… **약 0.5초마다 스크럽되는 무한 루프.** 스크럽은 `OnMouseCaptureEnd`에만
   물릴 것. (이 루프가 아래 `rtsp/` 크래시의 방아쇠였다.)
6. **재생 시작 후 첫 스크럽이 심리스 트래블(레벨 재로드)을 일으킨다** — 데모 드라이버가 패킷의
   LevelIndex 불일치를 보고 `ProcessSeamlessTravel`을 탐(`DemoNetDriver.cpp:2584`). 이때
   WorldSettings(→ 일시정지 상태), 스펙테이터 폰(→ 위치), 뷰 타겟이 전부 새로 만들어져 툴이
   세팅한 게 날아간다. 그래서 툴은 "한 번 세팅"이 아니라 **매 프레임 원하는 상태를 다시 강제**
   (`TickPlaybackState`: 일시정지 여부, 따라갈 액터 이름, 마지막 스펙테이터 트랜스폼).
7. **일시정지 중엔 스펙테이터가 안 움직인다.** `APlayerController::TickActor`가
   LEVELTICK_PauseTick에서 `bShouldPerformFullTickWhenPaused`가 꺼져 있으면 입력만 읽고
   RotationInput을 0으로 밀고 리턴(UpdateRotation/PlayerTick 안 돎). 게다가 ADefaultPawn 축
   바인딩은 `bExecuteWhenPaused=false`라 UPlayerInput이 일시정지 중 무시(`PlayerInput.cpp:1234`).
   툴이 `bShouldPerformFullTickWhenPaused`(protected → 리플렉션), 스펙테이터 AxisBindings의
   `bExecuteWhenPaused`, 폰/무브먼트의 `bTickEvenWhenPaused`를 켠다. 엔진
   `DemoNetDriver::TickDemoPlayback`(~1479행)도 스펙테이터 `CustomTimeDilation=1/DemoPlayTimeDilation`과
   폰/무브먼트 `bTickEvenWhenPaused`는 이미 켜 준다(그래서 관찰자 이동 속도는 배속과 무관).
8. 스펙테이터 폰은 `APlayerController::GetSpectatorPawn()`에 있고 `GetPawn()`은 null —
   `GetPawnOrSpectator()`를 쓸 것. 리플레이 스펙테이터는 **월드 원점**에 스폰되는데
   New_kadex_0811에선 원점이 랜드스케이프 아래라 처음엔 아무것도 안 보임. Spectator 콜리전
   프로필은 WorldStatic만 막는다(지형/건물 통과).
9. 에디터 기본 폰트(Roboto)에 ▶(U+25B6)/⏸(U+23F8) 글리프가 없어 `?`로 나옴 → 버튼 라벨은 ASCII.
10. 리플레이 중 쓸 만한 치트: `viewclass <Class>`, `viewactor <Name>`, `teleport`, `viewself`
    (`CheatManager.cpp:165` Teleport는 현재 시점에서 트레이스). `ghost`는 ACharacter 전용이라
    DefaultPawn 스펙테이터엔 안 됨.
11. EUW 탭 표시 이름은 CDO `TabDisplayName`에서 오고 탭 스포너는 **에디터 세션당 한 번만 등록**
    (`EditorUtilitySubsystem.cpp:425`) → 바꾸면 Compile+Save+**에디터 재시작**.
12. 위젯 레이아웃: ScrollBox는 높이가 제한돼야만 스크롤됨; 팝업 Border는 루트 Overlay의 **직계
    자식**이어야 하고 VAlign Fill, ScrollBox 슬롯도 Fill.

## 4. 리플레이로 드러난 것들

### 4-1. RtspAxisGate dangling 타이머 크래시 (게임 코드 수정, 별도 문서)

재생 중 스크럽 루프(함정 5)가 UGV를 반복 파괴/리스폰하는 동안
`FRtspAxisGate::ResolveLocalAxis`의 0.1초 폴링 타이머가 죽은 `this`를 찔러 크래시.
리플레이 재생에선 PC가 스펙테이터라 축이 영원히 안 정해져 5초 내내 폴링하는 게 재현 조건이었지만,
정상 플레이에서도 BeginPlay 후 5초 안에 오너가 파괴되면 같은 크래시가 잠재해 있었다.
→ **`rtsp/2026-09-16_rtsp_axis_gate_dangling_timer_fix.md`** (WeakLambda로 수정, 5개 호출부).

### 4-2. 적 낙하산이 리플레이에서 안 사라짐 — 리플리케이션 사각지대 후보 (미수정)

착지 후에 녹화를 시작하면 리플레이의 적군에 `SM_Parachute`(`BP_Enemy_kadex` 컴포넌트)가 계속
붙어 있다. 숨김이 서버 로컬(리플리케이트 안 됨)이라 리플레이 스트림에 없기 때문. BeginPlay부터
자동 녹화하면 정상(낙하 → 착지 → 숨김이 순서대로 기록되진 않지만 초기 상태가 맞음).

**리플레이는 클라이언트 하나가 더 붙은 것과 같으므로, 이건 2대 PC 풀 시스템에서 자체방호축
클라이언트가 늦게 접속하면(또는 관련성 재진입 시) 낙하산이 안 사라지는 것과 같은 결함일 가능성이
높다.** 확인·수정은 `ai_combat/` 또는 `replication/` 쪽 작업으로 남김.

## 5. 남은 것

- 낙하산 숨김 리플리케이션 확인(§4-2).
- 재생 배속은 `demo.TimeDilation`이라 물리 클램프와 무관하게 5배까지 정상 — 다만 5배에서 30Hz
  녹화 스트림이 6프레임/초로 보이는 건 당연한 것.
- 리플레이 파일은 녹화 당시 코드/에셋 기준이라 C++ 리플리케이션 구조가 바뀌면 옛 파일이 안
  열릴 수 있음(가이드 §7).

## 관련 문서

- `Plugins/Chronicle/README.md`, `Plugins/Chronicle/Chronicle_Guide.html` — **사용자 가이드**.
- `infra_architecture/2026-09-16_slomo_physics_dt_clamp_investigation.md` — 이 툴을 만들게 된 선행 조사.
- `rtsp/2026-09-16_rtsp_axis_gate_dangling_timer_fix.md` — 리플레이로 잡은 크래시.
- `replication/2026-09-15_drone_two_pc_validation.md` — 2 PC 리플리케이션 검증(리플레이가 대신 볼 수 있는 영역).
