# 리눅스 패키징 `Cook failed` — 쿠커의 MCP 서버가 에디터의 8000 포트와 충돌

2026-09-15 / 완료 / 쿡이 `Done!`까지 돌고도 `Cook failed`로 끝나던 원인은 에셋이 아니라 쿠커 프로세스가 에디터와 같은 127.0.0.1:8000에 MCP 서버를 띄우려다 남긴 Error 로그 1줄. `ProjectCustomBuilds`로 쿠커 포트만 8001로 비켜 해결.

## 1. 증상

에디터에서 Platforms ▸ Linux ▸ Package Project를 실행하면 오늘 3회 연속 `Cook failed` /
`AutomationTool exiting with ExitCode=25 (Error_UnknownCookFailure)`로 끝났다. 그런데 로그를 보면

- UBT 리눅스 바이너리 빌드는 `BUILD SUCCESSFUL`.
- 쿡도 끝까지 돌았다: `Cooked packages 2055 Packages Remain 0 Total 2055` → `LogCook: Done!`.
- 에셋 관련 에러는 0건. 이번에 새로 들어온 GASP/Lyra 애니메이션 에셋 등이 원인이 아닐까
  의심했지만 **아니었다**.

마지막으로 패키징이 성공한 건 2026-09-02(`2026-09-02_linux_package_ugv_host_rc_test_guide.md`).

## 2. 원인 추적

### 2-1. 쿡 커맨드릿의 실패 판정 규칙

쿡 커맨드릿의 `Main()`은 항상 0을 돌려주지만, `LaunchEngineLoop.cpp`(4221줄 부근)가 커맨드릿 종료 시
`GWarn->GetNumErrors() > 0`이면 종료 코드를 1로 바꾼다. 즉 **커맨드릿 실행 중 `Error:`로 찍힌 로그가 한
줄이라도 있으면 쿡은 실패**다. 쿡 결과(패키지 수)와 무관하다. UAT는 이 종료 코드를 보고
`Error_UnknownCookFailure`(25)로 올린다.

### 2-2. 쿡 로그의 `Error:` 줄은 딱 두 개였다

전체 쿡 로그에서 `Error:`를 찾으면 두 줄뿐이다.

1. `LogUObjectGlobals: Error: CDO Constructor (UGVChaosPawn): Failed to find
   /Game/Vehicles/UGV/Chaos/SK_UGVChaos.SK_UGVChaos`
   — **카운트되지 않는다.** `ConstructorHelpers::FailedToFind`(`UObjectGlobals.cpp`)는 `GWarn`이 아니라
   `UClass::GetDefaultPropertiesFeedbackContext()`라는 별도 피드백 컨텍스트에 기록하고, 그 컨텍스트의
   카운터는 비워진 뒤 Warning으로 다시 출력된다(`-------------- Default Property warnings and
   errors:` 블록). 2026-08-21 쿡 로그 등 몇 달 전부터 매번 찍혔고 그때는 패키징이 성공했다.
   (에셋은 2026-07-24에 `Content/Vehicles/UGV_OLD/Chaos/`로 옮겨졌는데 C++ 경로가 그대로 남은 것.
   이번 세션에 `UGVChaosPawn.cpp`를 `FObjectFinderOptional`로 바꿔 다음 빌드부터 메시지는
   사라지지만, 이건 정리일 뿐 **해결책이 아니다**.)
2. `LogHttpListener: Error: HttpListener unable to bind to 127.0.0.1:8000`
   — **이게 쿡을 실패시킨 한 줄.** 평범한 `UE_LOG(Error)`라 `GWarn`에 카운트된다.

### 2-3. 왜 바인드가 실패하나

- 에디터 프로세스가 Epic `ModelContextProtocol` 플러그인(unreal-mcp, `http://127.0.0.1:8000/mcp`)용으로
  이미 `127.0.0.1:8000`을 잡고 있다(에디터 로그: `LogHttpListener: Created new HttpListener on
  127.0.0.1:8000`).
- 쿠커는 별도 `UnrealEditor-Cmd.exe` 프로세스인데 같은
  `Saved/Config/WindowsEditor/EditorPerProjectUserSettings.ini`를 읽는다. 거기에
  `[/Script/ModelContextProtocolEngine.ModelContextProtocolSettings] bAutoStartServer=True`
  (Editor Preferences ▸ Model Context Protocol ▸ Auto Start Server, 플러그인 기본값은 False)가 있어서
  쿠커도 8000에 MCP 서버를 띄우려 한다 → 바인드 실패 → Error 1줄 → 쿡 "실패".

### 2-4. 왜 전에는 안 났나

`bAutoStartServer`는 프로젝트 Config가 아니라 **사용자별 에디터 환경설정**이다. 마지막 성공(09-02)
이후에 켜진 것으로 보이며, 정확한 날짜는 에디터 로그가 오늘 것밖에 없어 확인 불가.

## 3. 오진/헛수고 기록

- **CDO Constructor 에러가 원인이라는 오진** — 로그에서 `Error:`로 검색하면 제일 먼저 눈에 띄고
  이름도 프로젝트 클래스라 자연스럽게 의심하게 되는데, 위 2-2처럼 카운트 대상이 아니다. 앞으로
  같은 증상이 나면 이 줄은 건너뛸 것.
- **`AdditionalCookerOptions` ini 키** — 첫 시도로 `DefaultGame.ini`의
  `[/Script/UnrealEd.ProjectPackagingSettings]`에 `AdditionalCookerOptions=-ModelContextProtocolPort=8001`을
  넣었는데, `UProjectPackagingSettings`에 **그런 프로퍼티가 없다**. 조용히 무시되어 7분짜리 패키징
  1회를 날렸다. 제거함. (`2026-09-02_..._guide.md` §2에 잠깐 이 줄이 들어갔다가 같은 날 정정.)

## 4. 해결

### 4-1. 선택지 검토

| 방법 | 결과 |
|---|---|
| 쿠커에 `-ModelContextProtocolPort=N`(플러그인 공식 커맨드라인 오버라이드) | **채택.** 단, 쿠커 커맨드라인은 UAT `-additionalcookeroptions=`로만 전달되고 기본 Package Project 메뉴는 이를 붙일 수단이 없음 → 커스텀 빌드 필요 |
| 프로젝트 Config에서 `bAutoStartServer=False` 강제 | 불가 — 사용자 레이어(`Saved/.../EditorPerProjectUserSettings.ini`)가 최우선이라 덮어씀 |
| 에디터 실행 인자 `-ModelContextProtocolStartServer` 류 | 기각 — 에디터는 .uproject 더블클릭(인자 없음)으로 띄움 |
| `[Core.Log] LogHttpListener=NoLogging` | 기각 — 진짜 에러도 어디서나 다 숨김 |
| HttpServer `bReuseAddressAndPort` | 기각 — 리스너 2개가 같은 포트를 두고 싸움 |
| Auto Start Server 끄고 세션마다 `ModelContextProtocol.StartServer` 콘솔 명령 | 동작은 하지만 매번 수동 |
| 에디터 닫고 패키징 | 동작(설정 0), 대신 패키징 동안 MCP를 못 씀 |

### 4-2. 적용: Project Custom Build

`Config/DefaultGame.ini`의 `[/Script/UnrealEd.ProjectPackagingSettings]`에 추가:

```ini
+ProjectCustomBuilds=(Name="Package Linux (MCP 8000 회피)",HelpText="기본 Package Project와 동일하지만 쿠커의 MCP 서버 포트를 8001로 비켜서, 에디터(MCP 8000)를 켜둔 채 패키징해도 Cook failed가 나지 않는다.",SpecificPlatforms=("Linux"),BuildCookRunParams="-build -cook -stage -package -archive -pak -iostore -compressed -prereqs -nop4 -utf8output -installed -SkipCookingErrorSummary -JsonStdOut -project={Project} -platform={Platform} {ProjectPackagingSettings} -archivedirectory={BrowseForDir} -additionalcookeroptions=-ModelContextProtocolPort=8001")
```

- 에디터 Platforms 드롭다운의 **"Project Custom Builds"** 섹션에 서브메뉴 "Package Linux (MCP 8000
  회피)"로 나타난다. 서브메뉴 안에 같은 이름의 실행 항목 + Platform / Binary Configuration / Build
  Target 라디오가 있고, 플랫폼은 `SpecificPlatforms`로 Linux 고정.
- 출력 폴더는 `{BrowseForDir}` 덕에 Package Project와 똑같이 대화상자로 묻는다.
- `{ProjectPackagingSettings}`가 프로젝트 설정/메뉴 선택값에서 `-clientconfig=Development
  -target=titan_example -zenstore -pak`을 채운다.
- **에디터 재시작 필요** — `UProjectPackagingSettings`는 시작 시 읽는다.
- 기본 Package Project 항목은 에디터 MCP 서버가 떠 있는 한 여전히 실패한다. 커스텀 빌드를 쓰거나
  에디터를 닫고 패키징할 것.

### 4-3. 검증

UAT 드라이런(빌드 없음, ~2초):

```
RunUAT.bat -utf8output -ScriptsForProject=<uproject> Turnkey -command=ExecuteBuild -build="Package Linux (MCP 8000 회피)" -platform=Linux -project=<uproject> -overridetarget=titan_example -overrideconfiguration=Development -PrintOnly -outputdir=<dir> -nocompile -nocompileuat
```

출력:

```
RunUAT BuildCookRun -build -cook -stage -package -archive -pak -iostore -compressed -prereqs -nop4 -utf8output -installed -SkipCookingErrorSummary -JsonStdOut -project="..." -platform=Linux -clientconfig=Development -target=titan_example -zenstore -pak -archivedirectory={BrowseForDir} -additionalcookeroptions=-ModelContextProtocolPort=8001
```

표준 Package Project 커맨드와 동일하고 포트 오버라이드만 붙었다. 에디터에서는 **Ctrl을 누른 채**
커스텀 빌드 항목을 클릭하면 같은 print-only를 하고 커맨드라인을 클립보드에 복사한다.

실제 패키징에서의 성공 기준: 쿡 로그에 `LogModelContextProtocol: Starting MCP server on port 8001`
(8000이 아님)이 있고, `HttpListener unable to bind` 줄이 없어야 한다.

## 5. 앞으로 "Cook failed인데 Done!" 진단 절차

1. 쿡 로그에서 `LogCook: Done!`과 `Packages Remain 0`을 확인 — 있으면 에셋 문제가 아니라 "Error
   로그 카운트" 문제다.
2. 로그 전체에서 `Error:`를 검색. 아래는 **제외**:
   - `LogUObjectGlobals: Error: CDO Constructor (...)` — 별도 피드백 컨텍스트, 카운트 안 됨.
   - `Default Property warnings and errors:` 블록 안의 재출력분.
3. 남는 `Error:` 줄 하나하나가 후보다. 이번엔 `LogHttpListener ... unable to bind`였고, 다음번엔
   다른 플러그인/서브시스템일 수 있다 — 프로세스 시작 직후(엔진 초기화 단계)에 찍힌 Error를 특히
   의심할 것(쿡 결과와 무관하게 실패시키는 부류).
4. 원인이 환경(포트, 경로, 권한)이면 쿠커 커맨드라인(`-additionalcookeroptions=`)이나 커스텀 빌드로
   우회할 수 있는지 먼저 본다. ini 키를 지어내지 말 것 — `UProjectPackagingSettings.h`에 있는
   프로퍼티만 유효하다.

## 6. 관련 파일

- `titan_example/Config/DefaultGame.ini` — `ProjectCustomBuilds` 항목 + 주석(원인 요약).
- `titan_example/Source/titan_example/Vehicles/UGVChaosPawn.cpp` — `FObjectFinderOptional`로 변경(정리용).
- `packaging/2026-09-02_linux_package_ugv_host_rc_test_guide.md` §2 — 패키징 절차, 같은 날 정정.
- `guide/mcp/unreal-mcp-claude-code.md` — 주의사항에 한 줄 추가.
- 엔진 소스: `Engine/Source/Runtime/Launch/Private/LaunchEngineLoop.cpp`(커맨드릿 종료 코드),
  `Engine/Source/Runtime/CoreUObject/Private/UObject/UObjectGlobals.cpp`(`ConstructorHelpers::FailedToFind`),
  `Engine/Plugins/.../ModelContextProtocol`(`-ModelContextProtocolPort` 파싱, `bAutoStartServer`).
