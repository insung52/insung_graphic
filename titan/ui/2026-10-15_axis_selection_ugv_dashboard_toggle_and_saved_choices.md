# 축 선택 화면 — UGV 테스트 화면 체크박스 + 선택값 영구 저장

2026-10-15 / 진행중(C++ 완료, 빌드·WBP 배치 대기) / 축 선택 화면에 "UGV 테스트 화면 표시" 체크박스를 추가하고, 데모 모드·UGV 테스트 화면·접속 IP를 패키지 종료 후에도 남게 저장.

선행 문서: [`2026-09-10_axis_selection_operator_ux.md`](2026-09-10_axis_selection_operator_ux.md)
(축 선택 화면 운용자 UX 개편 — RCIP/포트/해상도 저장은 그때 이미 들어감).

---

## 1. 요청 (사용자, 2026-10-15)

1. 축 선택 화면(`WBP_AxisSelection2`, 부모 `UAxisSelectionWidget`)에 **데모 모드 체크박스처럼
   체크박스 하나**를 추가해서, UGV 모드에서 테스트 WBP(`UGVTestDashboardWidgetClass`)를
   **없이 실행(현재 기본값)할지 / 띄울지**를 화면에서 고를 수 있게.
   - 지금까지는 `BP_TestPlayerController`의 `bCreateUGVTestDashboard`를 **에디터에서만** 바꿀 수 있었음.
2. 축 선택 화면의 **선택·입력값이 패키징 프로그램 종료 후에도 유지**되게.

---

## 2. 무엇이 이미 저장되고 있었나 / 무엇이 안 됐나

| 값 | 위젯 | 10-15 이전 | 저장 주체 |
|---|---|---|---|
| RC IP | `RCIPText` | 저장됨(09-10) | `UGVRemoteControlSubsystem::SaveConfig()` |
| 포트 4개 | 포트 4칸 | 저장됨(09-10) | `UGVRemoteControlSubsystem::SaveConfig()` |
| 해상도 12칸 | 해상도 칸 | 저장됨(09-10) | `StreamResolutionSubsystem::SaveConfig()` |
| 접속 IP | `EditableText_ip` | **저장 안 됨**(09-10 코드에 "저장하지 않는다" 주석) | → 이번에 추가 |
| 데모 모드 | `DemoModeCheckBox` | **저장 안 됨** | → 이번에 추가 |
| UGV 테스트 화면 | (없었음) | — | → 이번에 신설 |

**SaveConfig 동작 근거(엔진 소스 확인)**: UE 5.8
`Engine/Source/Runtime/CoreUObject/Private/UObject/Obj.cpp`의 `UObject::SaveConfig`(3689~3945행)는
끝에서 `Config->Flush`를 하므로 **호출 즉시 파일에 기록**되고, 인스턴스 값을 CDO에도 복사한다.

> ⚠️ 다만 에디터 `Saved/Config/WindowsEditor/Game.ini`에는 아직 RCIP 섹션이 없다 — 09-10 저장이
> 실제로 동작하는지는 **실측 미검증** 상태. 이번 빌드 확인 때 같이 볼 것(§5).

---

## 3. 구현

### 3-1. 신규 `UI/AxisLaunchOptionsSubsystem.h/.cpp`

`UCLASS(Config=Game)` `UGameInstanceSubsystem`. 역할 두 가지:

**① 영구 저장 (Config)**

| 프로퍼티 | 의미 |
|---|---|
| `bHasSavedDemoMode` / `bSavedDemoMode` | 데모 모드 체크 상태 |
| `bHasSavedUGVTestDashboard` / `bSavedUGVTestDashboard` | UGV 테스트 화면 체크 상태 |
| `SavedServerIP` | 클라이언트 접속 IP |

- bool은 "저장된 적 없음"과 `false`를 구분 못 해서 `bHas…` 플래그를 따로 둠.
- 저장 위치: `Saved/Config/<Platform>/Game.ini`의 `[/Script/titan_example.AxisLaunchOptionsSubsystem]`
  (에디터는 `WindowsEditor`, Windows 패키지는 `Windows`, 리눅스 패키지는 `Linux`).

**② 트랜지언트 전달값 `TOptional<bool> LaunchUGVTestDashboard`**

- 로비 → 게임 레벨 `open` 트래블에서 PlayerController는 새로 생기지만 **GameInstance는 살아남는다** —
  그래서 "이번 실행에서 UGV 테스트 화면을 띄울지"를 GameInstance 서브시스템에 실어 보낸다.
- `SetLaunchUGVTestDashboard(bool)` / `GetLaunchUGVTestDashboard(bool& Out)`(설정된 적 없으면 false 반환).
- **URL 옵션(`?Demo=` 방식)을 안 쓴 이유**: 이건 서버 권위 상태가 아니라 "이 PC 화면에 무엇을
  띄울지"이고, 소비자가 같은 프로세스의 로컬 PC다. 서버를 거칠 이유가 없음.

### 3-2. `UAxisSelectionWidget` (`UI/AxisSelectionWidget.h/.cpp`)

- 새 `BindWidgetOptional` **`UGVTestDashboardCheckBox`** (타입 Check Box).
- `NativeConstruct` → **`RestoreSavedLaunchOptions()`** (저장값 → 화면):
  - 데모: 저장된 게 있으면 그 값, 없으면 WBP의 초기 체크 상태 유지.
  - UGV 테스트 화면: 저장된 게 있으면 그 값, 없으면 **owning PC의 `bCreateUGVTestDashboard`**
    (로비와 게임 레벨이 같은 PC BP를 쓴다는 전제).
  - 접속 IP: 저장된 게 있을 때만 `EditableText_ip`를 덮어씀.
- 두 체크박스 `OnCheckStateChanged` → **`HandleLaunchOptionCheckChanged`**: 저장 + 요약문 갱신.
  (예전엔 데모 체크를 바꿔도 요약문이 안 갱신되던 것도 같이 해결.)
- `EditableText_ip` 커밋 시 저장. **빈 IP는 저장 안 함.**
- Host / Solo / Client 버튼: 입력 검증 통과 직후 **`RecordLaunchOptionsForTravel()`** —
  저장 + (체크박스가 있으면) 이번 실행 UGV 테스트 화면 값을 서브시스템에 기록.
- **WBP에 없는 위젯의 값은 저장을 건드리지 않음**(옛 WBP로 실행해도 저장값이 지워지지 않게).
- 툴팁 추가, 요약문 UGV 줄 끝에 `, 테스트 화면 표시` / `, 테스트 화면 없음`.

### 3-3. `Atitan_examplePlayerController::Client_OnAxisResolved`

- 서브시스템에 이번 실행 값이 있으면 **그 값**, 없으면(PIE로 게임 레벨 직접 열기, 콘솔
  `HostListenServer` 등 축 선택 화면을 안 거친 실행) **기존대로 BP의 `bCreateUGVTestDashboard`**.
- 로그: `Client_OnAxisResolved: UGV test dashboard=%d (축 선택 화면 값, BP 기본값=%d)`
- `bCreateUGVTestDashboard`를 꺼도 SetViewTarget/카메라·오디오 타겟 설정은 그대로 실행됨
  (메인 뷰포트가 RCWS 시점을 렌더해야 RTSP가 가져갈 수 있음 — 기존 동작 유지).
- 헤더의 `bCreateUGVTestDashboard` 주석에 역할 변경 기록: 이제 "축 선택 화면을 안 거친 실행의
  기본값 + 축 선택 화면에 저장값이 없을 때의 초기 체크 상태".

### 3-4. 바뀐 파일 (전부 Perforce 미제출)

| 파일 | 상태 |
|---|---|
| `Source/titan_example/UI/AxisLaunchOptionsSubsystem.h` | 신규, `p4 add` |
| `Source/titan_example/UI/AxisLaunchOptionsSubsystem.cpp` | 신규, `p4 add` |
| `Source/titan_example/UI/AxisSelectionWidget.h/.cpp` | `p4 opened` |
| `Source/titan_example/titan_examplePlayerController.h/.cpp` | `p4 opened` |

`.uasset` 변경 없음.

---

## 4. 사용자 남은 작업

1. **빌드** — 새 `UCLASS`/`UPROPERTY` 추가라 **Live Coding 말고 에디터 닫고 정식 빌드**
   (Live Coding으로 하면 BP에서 missing property가 나는 함정).
2. **WBP 배치** — `WBP_AxisSelection2`의 `AdvancedPanel` 안(데모 체크박스 옆)에 Check Box를 놓고
   이름을 정확히 **`UGVTestDashboardCheckBox`**로.
   - 같은 WidgetTree 안에 둘 것(자식 UserWidget으로 빼면 바인딩 안 됨).
   - 부모 클래스 `AxisSelectionWidget` 유지.
   - 라벨 TextBlock은 자유(예: "UGV 테스트 화면 표시").

---

## 5. 확인 방법

로그:
- `[AxisSelectionWidget] NativeConstruct — … UGVTestDashboardCheckBox=BOUND`
- `[AxisSelectionWidget] RestoreSavedLaunchOptions — Demo=… UGVTestDashboard=… ServerIP='…'`
  (처음엔 `(미저장)`)
- Host 클릭 시 `[AxisSelectionWidget] RecordLaunchOptionsForTravel — UGVTestDashboard=1/0`
- 게임 레벨에서 `Client_OnAxisResolved: UGV test dashboard=… (축 선택 화면 값, BP 기본값=…)`

파일 (종료 후):
- 에디터: `Saved/Config/WindowsEditor/Game.ini`
- Windows 패키지: 패키지의 `Saved/Config/Windows/Game.ini`
- 리눅스 패키지: 패키지의 `Saved/Config/Linux/Game.ini`
- 위 파일에 `[/Script/titan_example.AxisLaunchOptionsSubsystem]` 섹션과 **RCIP 섹션이 같이**
  생겼는지 볼 것(RCIP 쪽은 09-10 저장의 첫 실측 검증을 겸함).

> ⚠️ **주의**: 패키지의 `Saved` 폴더를 지우거나 새 패키지로 덮어써서 `Saved`가 날아가면 저장값도
> 같이 사라진다(전시장 배포 시 패키지 교체 절차에서 주의).

---

## 6. 보류

- **실행가이드(`packaging/kadex_0902_패키징_실행가이드.md`) §4 갱신** — 09-10 문서 방침대로
  **빌드·실측 후**에 고친다(검증 안 된 동작을 고객 문서에 먼저 쓰지 않음).
- `guide/`에 축 선택 화면 문서는 여전히 없음(09-10 문서 §7-4 — 화면 확정 후 신설 검토).
