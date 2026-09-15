# 리눅스 패키지 — UGV Host 모드로 통제기 연동(UDP+RTSP) 확인 가이드

2026-09-02 / 완료 / 리눅스 패키지 1개만 넘겨서, 대기실에서 RC IP 입력 → UGV Host로 들어가 통제기와 UDP/RTSP가 붙는지 확인하는 절차.

## 0. 이 문서의 범위

**확인하려는 것 딱 하나**: 우리 리눅스 패키지를 통제기 옆에서 실행했을 때

- **UDP+JSON**(LIG ICD §3.1)이 통제기와 양방향으로 오가는가
- **RTSP 5스트림**(`ugv/*`)을 통제기가 받아가는가

**범위에서 뺀 것**: 자체방호축(Client 버튼), 데모 모드(Solo 버튼), 3단계 전투 시나리오 완주.
시나리오가 끝까지 도는지는 여기서 안 본다 — UGV가 스폰돼서 소켓/스트림이 열리고 통제기 명령에
반응하면 성공이다.

**공유 대상**: 리눅스 패키지 폴더 **하나**. `rc_mockup_tools/`(우리가 만든 RC GUI / UDP 클라이언트 /
RTSP 뷰어)는 내부 테스트 도구라 넘기지 않는다.

---

## 1. 구성

```
  [ 통제기 PC (LIG) ]                        [ 우리 리눅스 PC = UGV 시뮬레이션 SW ]
   RC IP = 192.168.10.20 (예시)               UGV IP = 192.168.10.10 (예시)
                                              titan_example.sh (리슨서버 / Axis=UGV)

   RC → UGV   UDP 8000(주기) / 8001(비주기)  ──▶  0.0.0.0:8000 / 0.0.0.0:8001 로 바인드
   UGV → RC   UDP 8010(주기) / 8011(비주기)  ◀──  RCIP:8010 / RCIP:8011 로 송신
   RTSP       TCP 8554 (interleaved 권장 / UDP도 됨*) ◀──  rtsp://<UGV IP>:8554/ugv/<stream>
```

- 포트/IP 원문은 `protocol/protocol_icd.md` §3.1. 코드 기본값도 같은 값
  (`Source/titan_example/Network/UGVRemoteControlSubsystem.h`).
- **우리 쪽 수신은 `0.0.0.0` 바인드**라 우리 PC의 IP를 앱에 알려줄 필요가 없다. 앱에 넣어야 하는
  건 **통제기 PC의 IP(RC IP) 하나뿐**이다.
- 통제기 쪽에는 우리 PC의 IP를 알려줘야 한다(UDP 목적지 + RTSP 접속 주소 양쪽).
- *(2026-09-15 정정) RTSP 전송은 **TCP interleaved / UDP 둘 다 된다.** 이 문서와 수신 가이드에
  있던 "TCP만 / UDP 미지원"은 오류 — `RtspServerSubsystem`에 `gst_rtsp_media_factory_set_protocols`
  같은 제한 호출이 없어 gst-rtsp-server 기본값(UDP·TCP 허용)이 그대로 적용된다(09-04 검증 때
  목업이 UDP로 붙은 흔적도 §7-1 4번의 `udpsrc`). 다만 저지연 수신 설정은 TCP 기준으로 검증했고,
  **클라이언트가 UDP를 고르면 RTP/RTCP 포트가 접속 시 동적으로 협상**되므로 방화벽이 있으면
  8554만 열면 되는 TCP를 권장한다(§3-3).
- IP가 ICD 기본값(192.168.10.x)이 아니어도 대기실 화면에서 바꾸면 된다(§4). Tailscale IP(100.x.x.x)로도
  그대로 동작한다(2PC 테스트 이력 있음).

### RTSP 마운트 5개

| 스트림 | URL |
|---|---|
| RCWS 조준경 | `rtsp://<UGV IP>:8554/ugv/rcws` |
| 전면 CCTV | `rtsp://<UGV IP>:8554/ugv/front_cctv` |
| 후면 CCTV | `rtsp://<UGV IP>:8554/ugv/rear_cctv` |
| 좌측 CCTV | `rtsp://<UGV IP>:8554/ugv/left_cctv` |
| 우측 CCTV | `rtsp://<UGV IP>:8554/ugv/right_cctv` |

기본 해상도는 RCWS 1920×1080 / CCTV 320×180(`UStreamResolutionSubsystem` 기본값), 대기실 화면에서
바꿀 수 있다. 수신 측 저지연 세팅은 `rtsp/rtsp_client_reception_guide.md`(LIG 공유용) 그대로.

---

## 2. 패키징 (Windows 에디터에서 Linux 크로스컴파일)

1. **`Config/DefaultGame.ini` 확인** — `[/Script/UnrealEd.ProjectPackagingSettings]`에 아래 줄들이
   살아 있어야 한다. 게임 레벨은 문자열 트래블(`open ...`)로만 도달해서 쿠커가 정적 분석으로 못 찾는다.
   ```ini
   +MapsToCook=(FilePath="/Game/kadex_lobby")
   +MapsToCook=(FilePath="/Game/New_kadex_0811")
   +DirectoriesToAlwaysCook=(Path="/Game/Input")
   ; 2026-09-15 추가 — 에디터의 MCP 서버(8000)가 떠 있는 채로 패키징할 때 쓰는 커스텀 빌드(3번 참고)
   +ProjectCustomBuilds=(Name="Package Linux (MCP 8000 회피)",HelpText="(생략 — ini 원문 참고)",SpecificPlatforms=("Linux"),BuildCookRunParams="-build -cook -stage -package -archive -pak -iostore -compressed -prereqs -nop4 -utf8output -installed -SkipCookingErrorSummary -JsonStdOut -project={Project} -platform={Platform} {ProjectPackagingSettings} -archivedirectory={BrowseForDir} -additionalcookeroptions=-ModelContextProtocolPort=8001")
   ```
   ⚠️ 에디터의 Project Settings ▸ Packaging 화면을 열고 저장하면 이 섹션이 덮어써질 수 있다.
   ini를 직접 고쳤으면 **에디터를 재시작한 뒤** 패키징할 것(`UProjectPackagingSettings`는 시작 시
   한 번만 읽는다 — 커스텀 빌드 항목도 재시작해야 메뉴에 나타난다).
   `AdditionalCookerOptions=...` 같은 ini 키는 **존재하지 않는다**(넣어도 조용히 무시됨) — 쿠커
   인자는 위 `ProjectCustomBuilds`의 `-additionalcookeroptions=`로만 전달된다.
2. **빌드 구성은 Development 권장** — 이번 목적이 "로그로 확인"이라서다(§6의 확인용 로그가 전부
   `titan_example.log`에 찍힌다). Shipping으로 뽑으면 §6 절차를 대부분 못 쓴다.
3. **패키징 실행** — 툴체인은 `C:\UnrealToolchains\v26_clang-20.1.8-rockylinux8`. 빌드 PC의 NVENC SDK는
   `C:\SDK\Video_Codec_SDK_13.0.37`(`RtspEncoder.Build.cs` 기본 경로) — **13.0.37이어야 드라이버 570+
   대응**이 된다(13.1은 610+ 요구, 2026-09-15 교체 — §3-1 추가 블록 참고). SDK가 없으면 빌드는 성공하고
   RTSP만 조용히 빠지므로(아래 soft-fail 주의) 패키징 전에 경로가 살아 있는지 볼 것.
   - 에디터에 **MCP 서버가 떠 있으면**(Editor Preferences ▸ Model Context Protocol ▸ Auto Start
     Server ON, 즉 Claude Code가 unreal-mcp로 붙어 있는 보통 상태):
     **Platforms ▸ Project Custom Builds ▸ "Package Linux (MCP 8000 회피)"** 서브메뉴 안의 같은
     이름 항목을 실행. 출력 폴더 대화상자(`{BrowseForDir}`)가 Package Project와 똑같이 뜨고,
     플랫폼은 Linux로 고정, Binary Configuration/Build Target 라디오는 그 서브메뉴 안에서 고른다.
     표준 Package Project 커맨드라인에 `-additionalcookeroptions=-ModelContextProtocolPort=8001`
     하나만 덧붙인 것이다.
   - MCP 서버를 **꺼둔 상태**(Auto Start Server OFF)거나 **에디터를 닫고** 커맨드라인으로 패키징할
     때는 종전대로 Platforms ▸ Linux ▸ Package Project를 써도 된다.
   - ⚠️ MCP 서버가 떠 있는데 기본 Package Project를 누르면 쿡이 끝까지 돌고도 **`Cook failed`로
     끝난다**(아래 문제 해결 블록).
4. ~~`titan_example_x11_fallback.sh`를 결과물 폴더에 손으로 복사한다~~ — **2026-09-15부로 불필요.**
   Wayland/X11 세션 자동 판별이 `Config/BootstrapPreamble.sh`를 통해 `titan_example.sh` 자체에
   들어간다. UAT(`LinuxPlatform.Automation.cs`의 `StageBootstrapExecutable`)가 리눅스 스테이징 때
   `<Project>/Config/BootstrapPreamble.sh`가 있으면 그 내용을 생성 스크립트 맨 앞에 끼워 넣는 엔진
   기능을 쓴 것. 동작은 `run_titan_example.sh`와 동일 — 사용자가 `-sdlvideodriver=`를 안 줬고
   `WAYLAND_DISPLAY`가 비어 있으면 `-sdlvideodriver=x11`을 인자 맨 앞에 끼워 넣고, `WAYLAND_DISPLAY`가
   있으면 네이티브 Wayland, 명시 인자는 항상 존중. POSIX sh만(생성 스크립트가 `#!/bin/sh`=dash).
   - **파일이 P4에 들어 있어야 한다.** 체크아웃에 빠져 있으면 패키징은 조용히 성공하고 블록 없는
     스크립트가 나온다 → 결과물의 `titan_example.sh`를 열어 `### Added from project Config
     BootstrapPreamble.sh` 마커 블록이 있는지 **반드시 확인**.
   - 2026-09-15 stage-only UAT(`BuildCookRun -skipbuild -skipcook -stage ...`)로 검증: 블록 삽입,
     LF 줄바꿈, `sh -n` OK, `sh`로 실행 시 `[titan_example] Wayland를 찾지 못했습니다 — X11로
     실행합니다 (-sdlvideodriver=x11).` 출력 후 리눅스 바이너리 도달.
   - 프로젝트 루트의 `run_titan_example.sh` / `titan_example_x11_fallback.sh`는 삭제하지 않고 레거시로
     둔다. 같이 넣어도 무해(둘 다 `-sdlvideodriver=x11`을 명시하므로 프리앰블은 건너뜀).

결과물은 `titan_example.sh`(프리앰블 블록 포함) + `titan_example/` + `Engine/`이 들어있는 폴더 하나.
이 폴더를 통째로 넘기면 된다. 외부 전달용 실행 가이드는 `kadex_0915_패키징_실행가이드.md`
(받는 쪽 절차만 담김 — 패키징 절차는 이 문서가 유일). 성공 확인은 §2-5.

> **문제 해결 — `Cook failed`인데 로그에는 `LogCook: Done!`이 있는 경우(2026-09-15)**
>
> - 증상: UBT는 `BUILD SUCCESSFUL`, 쿡도 `Cooked packages 2055 Packages Remain 0` → `LogCook: Done!`
>   까지 찍혔는데 UAT가 `Cook failed` / `AutomationTool exiting with ExitCode=25
>   (Error_UnknownCookFailure)`로 끝남.
> - 규칙: 쿡 커맨드릿의 `Main()`은 항상 0을 돌려주지만 `LaunchEngineLoop.cpp`(4221줄 부근)가
>   `GWarn->GetNumErrors() > 0`이면 종료 코드를 1로 바꾼다. 즉 **커맨드릿 로그에 `Error:` 줄이 한
>   줄이라도 있으면 쿡은 실패**다. 에셋 문제가 아니라도 그렇다.
> - 원인: 에디터가 MCP 플러그인용으로 `127.0.0.1:8000`을 잡고 있는데, 쿠커(별도
>   `UnrealEditor-Cmd.exe`)가 같은 `Saved/Config/WindowsEditor/EditorPerProjectUserSettings.ini`의
>   `bAutoStartServer=True`를 읽어 8000에 또 MCP 서버를 띄우려다
>   `LogHttpListener: Error: HttpListener unable to bind to 127.0.0.1:8000` 한 줄을 남긴다. 이게
>   유일하게 카운트되는 Error다.
> - **오진 주의**: 같은 로그에 매번 찍히는 `LogUObjectGlobals: Error: CDO Constructor (UGVChaosPawn):
>   Failed to find /Game/Vehicles/UGV/Chaos/SK_UGVChaos`는 원인이 **아니다**. `ConstructorHelpers`는
>   별도 피드백 컨텍스트(`UClass::GetDefaultPropertiesFeedbackContext()`)에 기록하고 그건 Warning으로
>   재출력될 뿐 `GWarn` 카운터에 안 들어간다. 몇 달 전 쿡 로그(2026-08-21)에도 있었고 그때는
>   패키징이 성공했다.
> - 성공 확인: 쿡 로그에 `LogModelContextProtocol: Starting MCP server on port 8001`(8000이 아님)이
>   있고 `HttpListener unable to bind` 줄이 없어야 한다.
> - 커맨드라인만 미리 확인(빌드 없이 ~2초): 에디터에서 **Ctrl을 누른 채** 커스텀 빌드 항목을 클릭하면
>   실행 대신 커맨드라인을 출력하고 클립보드에 복사한다. 셸에서는
>   ```
>   RunUAT.bat -utf8output -ScriptsForProject=<uproject> Turnkey -command=ExecuteBuild -build="Package Linux (MCP 8000 회피)" -platform=Linux -project=<uproject> -overridetarget=titan_example -overrideconfiguration=Development -PrintOnly -outputdir=<dir> -nocompile -nocompileuat
>   ```
>   → `RunUAT BuildCookRun ... -additionalcookeroptions=-ModelContextProtocolPort=8001`이 찍히면 됨.
> - 조사 전문: `packaging/2026-09-15_linux_cook_failed_mcp_port_clash.md`.

> RTSP가 실제로 들어갔는지 의심되면 패키징 로그에서 `[RtspEncoder]` 경고를 확인할 것 —
> NVENC SDK / CUDA / GStreamer 번들이 빌드 PC에 없으면 **빌드는 성공하고 RTSP만 조용히 빠진다**
> (의도된 soft-fail, `rtsp/rtsp_poc_findings.md` §10.3.2).

### 2-5. 패키징 성공 확인 체크리스트 (2026-09-15 정리)

넘기기 전에 아래 5개를 순서대로 본다.

1. **쿡 로그**에 `LogModelContextProtocol: Starting MCP server on port 8001`(8000이 아님)이 있고,
   `HttpListener unable to bind` 줄이 **없다**.
2. 패키징 로그에 `[RtspEncoder]` 경고가 **없다** — 있으면 NVENC SDK / CUDA / GStreamer 번들 누락으로
   RTSP가 빠진 패키지다(위 soft-fail 주의).
3. **출력 폴더 구성**이 아래와 같다.
   ```
   titan_example.sh     ← 실행 스크립트 (프리앰블 블록 포함 — 4번)
   titan_example/       ← 게임 데이터 + Binaries/Linux/titan_example
   Engine/
   ```
4. **`titan_example.sh`를 열어** 맨 앞에 `### Added from project Config BootstrapPreamble.sh` 마커로
   감싼 블록(`WAYLAND_DISPLAY` 검사 → `-sdlvideodriver=x11` 삽입)이 있다. 없으면 체크아웃에
   `Config/BootstrapPreamble.sh`가 빠진 것 → P4에서 받아 다시 패키징(§2-4).
5. 폴더를 통째로 압축하고 `kadex_0915_패키징_실행가이드.md`를 같이 보낸다. `run_titan_example.sh` /
   `titan_example_x11_fallback.sh`는 **넣지 않아도 된다**(레거시, 들어가도 무해).

### 2-6. 패키징 절차 변경 이력

| 날짜 | 변경 |
|---|---|
| 2026-09-02 | 최초 절차. Platforms ▸ Linux ▸ Package Project, `titan_example_x11_fallback.sh` 수동 복사. |
| 2026-09-04 | `run_titan_example.sh`(세션 자동 판별 래퍼) 신설, 역시 수동 복사(§7-1). |
| 2026-09-15 | 에디터 MCP 서버(8000)가 떠 있으면 `Cook failed` → `ProjectCustomBuilds` "Package Linux (MCP 8000 회피)" 도입(`2026-09-15_linux_cook_failed_mcp_port_clash.md`). |
| 2026-09-15 | NVENC SDK 13.1.15 → 13.0.37(드라이버 570+ 대응, `rtsp/2026-09-15_lig_rtsp_describe_timeout_analysis.md`). |
| 2026-09-15 | `Config/BootstrapPreamble.sh` 추가 — 세션 자동 판별이 `titan_example.sh`에 자동 삽입, 래퍼 복사 절차 폐지(§2-4). 커스텀 빌드로 전체 패키징 성공 확인. |

---

## 3. 대상 리눅스 머신 준비

Ubuntu 22.04 / 24.04 + NVIDIA GPU 기준(둘 다 실행 이력 있음).

### 3-1. 런타임 의존성

```bash
# GStreamer는 패키지에 번들되지 않는다 — 타겟 머신의 시스템 GStreamer를 그대로 쓴다.
sudo apt install -y \
  libgstreamer1.0-0 libgstreamer-plugins-base1.0-0 libgstrtspserver-1.0-0 \
  gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad \
  gstreamer1.0-tools
```

- 서버 파이프라인이 쓰는 요소: `appsrc`(plugins-base), `h264parse`(plugins-bad), `rtph264pay`(plugins-good).
- 링크는 `libgstreamer-1.0.so.0` 같은 SONAME 기준이라 배포판 버전이 정확히 1.24가 아니어도 되지만
  1.x 계열이어야 한다(빌드 시 참조한 번들은 Ubuntu 24.04 / GStreamer 1.24.2).
- Vulkan 런타임(`libvulkan1`)도 필요 — UE 리눅스 클라이언트는 Vulkan RHI로 뜬다.
- glibc 하한은 **2.28**(툴체인 sysroot=RockyLinux8) → **Ubuntu 20.04 이상**이면 됨.
- 자가 진단: `ldd titan_example/Binaries/Linux/titan_example | grep "not found"`

> ⚠️ **2026-09-03 정정 — GStreamer/NVIDIA는 "없어도 게임은 뜬다"가 아니다.**
> `RtspEncoder.Build.cs`의 Linux 분기가 `PublicAdditionalLibraries`로 평범한 직접 링크를 하므로
> 실행 파일에 `DT_NEEDED` 엔트리가 박힌다. 하나라도 없으면 **동적 로더 단계에서 프로세스가
> 즉시 죽는다**(`error while loading shared libraries: ...`, `Saved/Logs`도 안 생김).
> 실제 빌드 산출물(`Binaries/Linux/titan_example`)을 `readelf -d`로 확인한 결과:
>
> ```
> libSDL2-2.0.so.0                      ← 패키지 동봉(Binaries/Linux/, RPATH $ORIGIN)
> libglib-2.0.so.0 / libgobject-2.0.so.0 ← libglib2.0-0
> libgstreamer-1.0.so.0                 ← libgstreamer1.0-0
> libgstapp-1.0.so.0                    ← libgstreamer-plugins-base1.0-0
> libgstrtspserver-1.0.so.0             ← libgstrtspserver-1.0-0
> libnvidia-encode.so.1                 ← NVIDIA 독점 드라이버
> libcuda.so.1                          ← NVIDIA 독점 드라이버
> ```
>
> Build.cs가 링크를 거는 .so는 11개지만 실제 `DT_NEEDED`로 남는 건 위 8개뿐이다(나머지는
> 미사용이라 링커가 뺌 — `libgio`/`libgmodule`/`libgstbase`/`libgstrtsp`/`libgstsdp`/`libgstnet`).
>
> **NVIDIA GPU가 없거나 nouveau/Mesa를 쓰는 머신에서는 지금 빌드가 아예 실행되지 않는다.**
> "Vulkan은 다 설정했다"는 Mesa Vulkan으로도 성립하므로 이 실패와 완벽히 양립한다 — 실행 실패
> 신고를 받으면 `nvidia-smi`와 `ldconfig -p | grep libnvidia-encode`부터 확인할 것.
>
> 런타임 스위치로는 못 피한다(로더 단계라 우리 코드가 실행되기도 전). 근본적으로 없애려면
> NVENC/GStreamer를 링크 대신 `dlopen`으로 바꿔야 하는데 아직 안 했다 — SDK 없는 PC에서 빌드가
> 막히던 문제(`rtsp_poc_findings.md` §10.3.2)는 **빌드 타임** soft-fail만 해결한 것이고,
> **런타임** soft-fail(= 라이브러리 없는 머신에서 RTSP만 꺼진 채 실행)은 별개 미해결 사안이다.
>
> **[2026-09-15 추가] 라이브러리가 있어도 드라이버 버전이 낮으면 RTSP만 죽는다.** NVENC SDK
> 메이저.마이너 = 최소 드라이버 버전(13.1 → 610+, 13.0 → 570+). 09-02 패키지는 13.1이라 LIG PC
> (595.84)에서 인코더 초기화가 실패했고, 마운트는 먼저 등록돼 있어서 클라이언트는 404 대신 20초
> DESCRIBE 타임아웃을 받았다(IP 문제로 오인). 09-15부터 SDK 13.0.37 + 인코더 성공 후에만 마운트
> 등록(실패 시 즉시 404). 실행 가이드 §0에 드라이버 ≥570 명시. 상세
> `rtsp/2026-09-15_lig_rtsp_describe_timeout_analysis.md`, 특정 버전 재현 절차
> `2026-09-15_linux_nvidia_driver_595_run_install.md`.

### 3-2. 세션 종류 (Wayland / X11)

- `Config/Linux/LinuxEngine.ini`가 **`VideoDriver=wayland`를 강제**한다. Xwayland 경유 X11 경로에서
  풀스크린 QHD가 11fps까지 떨어지는 문제 때문이다(`rtsp/linux_wayland_x11_present_bottleneck.md`).
- **[2026-09-15부터] 어느 세션이든 그냥 `./titan_example.sh`.** 스크립트 맨 앞의 프리앰블
  (`Config/BootstrapPreamble.sh`, §2-4)이 `WAYLAND_DISPLAY`를 보고 없으면 `-sdlvideodriver=x11`을
  끼워 넣는다. 터미널 첫 줄에 어느 쪽으로 가는지 찍힌다(`[titan_example] Wayland 세션 감지 …` /
  `… Wayland를 찾지 못했습니다 — X11로 실행합니다`).
- 강제하려면 `./titan_example.sh -sdlvideodriver=x11` / `=wayland`(명시 인자는 프리앰블이 건드리지
  않음). 09-15 이전 패키지(프리앰블 없음)는 순수 X11 머신에서 폴백 없이 죽으므로 이 명시 인자나
  레거시 `titan_example_x11_fallback.sh`가 필요했다.
- GNOME/Wayland에서 창 테두리·타이틀바가 없는 건 정상이다(SDL에 libdecor가 없고 Mutter가
  서버사이드 데코레이션을 안 함). `Super` + 드래그로 창을 옮길 수 있다.

### 3-3. 방화벽

```bash
sudo ufw allow 8000/udp   # RC → UGV 주기
sudo ufw allow 8001/udp   # RC → UGV 비주기
sudo ufw allow 8554/tcp   # RTSP (TCP interleaved — 권장. 이 포트 하나로 RTSP 제어+RTP 영상이 다 나감)
```

> (2026-09-15 정정) UDP 전송도 동작하지만, 그 경우 RTP/RTCP용 UDP 포트가 **클라이언트 접속 시마다
> 동적으로 협상**돼서 고정 포트로 열 수 없다. 방화벽이 있으면 수신측이 `protocols=tcp`를 쓰게
> 하는 것이 답이다(8554/tcp만 필요).

### 3-4. 실행

```bash
chmod +x titan_example.sh
./titan_example.sh -fullsystem                 # 창모드 (§4-2 참고 — -fullsystem 권장, 세션은 자동 판별)
./titan_example.sh -fullsystem -fullscreen     # 풀스크린
```

---

## 4. ⭐ 대기실(kadex_lobby) — 여기가 핵심

실행하면 `kadex_lobby`의 축 선택 화면(`WBP_AxisSelection2`)이 뜬다. **UGV Host 버튼을 누르기 전에**
아래 두 가지를 반드시 한다.

### 4-1. RC IP 입력 — 입력 후 반드시 **Enter**

| 입력 필드 | 넣을 값 | 기본값 |
|---|---|---|
| **RC IP** | **통제기가 돌아갈 PC의 IP** | `192.168.10.20` |
| RC Periodic Port | 보통 그대로 | `8010` |
| RC Event Port | 보통 그대로 | `8011` |
| UGV Listen Periodic Port | 보통 그대로 | `8000` |
| UGV Listen Event Port | 보통 그대로 | `8001` |
| RCWS / CCTV 해상도 | 필요하면 | 1920×1080 / 320×180 |

> ⚠️ **가장 흔한 실수**: 값을 타이핑만 하고 바로 Host를 누르는 것. 이 필드들은 `OnTextCommitted`
> (= **Enter를 치거나, 다른 곳을 클릭해서 포커스가 빠질 때**)에만 반영된다. Host 버튼 핸들러는
> 텍스트 필드를 다시 읽지 않는다(`UAxisSelectionWidget::HandleHostClicked`).
> **필드마다 Enter를 한 번씩 칠 것.**
>
> 반영됐는지는 로그로 확인된다 — 커밋할 때마다
> `[AxisSelectionWidget] ApplyNetworkFieldsToSubsystem — RCIPValue='...'` 줄이 찍힌다.

입력값은 `UUGVRemoteControlSubsystem`(GameInstance 서브시스템)에 들어가므로 **레벨 트래블 후에도
유지**된다. 소켓도 커밋 즉시 재오픈된다.

### 4-2. 데모 모드 체크박스는 **반드시 해제**

⚠️ **이걸 놓치면 통제기 연동이 통째로 안 붙는다.** 지금 `New_kadex_0811`의 `ScenarioConfig_1`은
레벨에 **`RunMode=Demo`로 저장**돼 있고(전시용 1PC 데모 세팅), 데모 모드에서는
`UUGVRemoteControlSubsystem::ShouldBeActive()`가 **UDP 소켓을 아예 안 연다**(수신도 송신도 없음).

- 대기실에서 **체크박스를 해제한 채로** Host를 누르면 `?Demo=0`이 실려서 레벨 저장값을 덮어쓴다 → 풀 시스템.
- 체크박스가 WBP에 없으면 Host의 기본값이 "해제(=풀 시스템)"다.
- 더 확실하게 하려면 실행 인자에 `-fullsystem`을 주면 된다(커맨드라인이 URL·레벨값을 모두 이긴다):
  ```bash
  ./titan_example.sh -fullsystem
  ```
  **넘기는 쪽에는 이 인자를 기본으로 안내하는 게 안전하다.**

우선순위: `커맨드라인 -demo/-fullsystem` > `접속 URL ?Demo=` > `레벨 AScenarioConfig::RunMode`.

### 4-3. Host 버튼

`HostListenServer("UGV", false)` → `open New_kadex_0811?Listen?Axis=UGV?Demo=0`.
이 프로세스가 **리슨서버 + UGV축**이 되고, **그 조합에서만** 통제기 연동 서브시스템이 활성화된다.

- **Client 버튼**(자체방호축 접속), **호스트 없이 시작(Solo) 버튼**은 이번 범위가 아니다. 누르지 말 것.
  - 특히 Solo는 자체방호축 standalone이라 통제기 연동이 **영영 안 붙는다**.
- 콘솔로도 같다: `` ` `` 키 → `HostListenServer UGV 0`

---

## 5. 통제기 쪽에 알려줄 값

| 항목 | 값 |
|---|---|
| UGV 시뮬레이터 IP | 우리 리눅스 PC의 IP |
| UGV 수신 포트 | UDP 8000(주기) / 8001(비주기) |
| 통제기 수신 포트 | UDP 8010(주기) / 8011(비주기) — 우리가 여기로 쏜다 |
| RTSP | `rtsp://<우리 PC IP>:8554/ugv/{rcws,front_cctv,rear_cctv,left_cctv,right_cctv}` |
| RTSP 전송 | **TCP interleaved 권장**(`protocols=tcp`), H.264 High, B프레임 없음. (2026-09-15 정정: UDP도 됨 — 코드에 프로토콜 제한 없음, gst-rtsp-server 기본값. TCP는 저지연 검증 기준이고 방화벽 시 8554만 열면 됨; UDP는 RTP 포트가 동적 협상) |

---

## 6. 확인 절차

로그 위치: `<패키지폴더>/titan_example/Saved/Logs/titan_example.log` (Development 빌드 기준)

### 6-1. 풀 시스템으로 떴는지

```bash
grep "데모 실행 모드" titan_example.log
```
→ **이 줄이 나오면 실패다**(데모로 뜬 것). §4-2로 돌아갈 것. **안 나오는 게 정상.**

### 6-2. UDP 소켓이 열렸는지

```bash
grep "UGVRemoteControlSubsystem" titan_example.log
```
기대되는 줄:
```
UGVRemoteControlSubsystem: UDP 소켓 시작 — recv 8000(주기)/8001(비주기), send-to <RC IP>:8010(주기)/8011(비주기)
```
- `send-to`의 IP가 **입력한 RC IP인지 반드시 눈으로 확인**할 것. 여기가 `192.168.10.20`으로 남아
  있으면 §4-1의 Enter를 안 친 것이다.
- `UDP 소켓 바인딩 실패` → 8000/8001을 다른 프로세스가 쓰고 있음.
- `RCIP '...' 파싱 실패` → IP 문자열 오타.

소켓 확인:
```bash
ss -lunp | grep -E '8000|8001'
```

### 6-3. RTSP 마운트가 등록됐는지

```bash
grep -E "RTSP server listening|Registered RTSP mount" titan_example.log
```
기대:
```
RTSP server listening on port 8554
Registered RTSP mount 'ugv/rcws' (1920x1080 @ ... fps) -> rtsp://<host>:8554/ugv/rcws
Registered RTSP mount 'ugv/front_cctv' ...        (총 5줄)
```
- RTSP 서버 자체는 프로세스 시작 시(= 대기실에서 이미) 뜨고, **마운트 5개는 레벨에 들어가 UGV가
  스폰된 다음**에 등록된다. 대기실 상태에서 마운트가 없는 건 정상.
- `GStreamer failed to initialize` / `RtspEncoder built without GStreamer support` → §2-4 / §3-1 확인.

### 6-4. 실제 영상 (우리 쪽에서 먼저 확인)

같은 리눅스 머신이나 옆 PC에서:
```bash
gst-launch-1.0 rtspsrc location=rtsp://<UGV IP>:8554/ugv/rcws latency=0 drop-on-latency=true protocols=tcp \
  ! rtph264depay ! h264parse ! avdec_h264 ! autovideosink sync=true qos=true
```
NVIDIA가 있으면 `avdec_h264` 대신 `nvh264dec max-display-delay=0`. VLC로도 열리지만 지연이 크다
(수신측 튜닝 근거는 `rtsp/rtsp_client_reception_guide.md`).

### 6-5. 통제기와 실제로 주고받는지

- **UGV→RC**: 통제기 화면에 속도/기어/배터리(`UGV_Period_Basicinfo`, 10Hz)와 RCWS 상태
  (`UGV_RCWS_Status`, 20Hz)가 갱신되면 송신 OK.
- **RC→UGV**: 통제기에서 제어권 획득(`RC_Control_Right`) → 운용모드 REMOTE(`RC_OperationMode`) →
  주행(`RC_RemoteDriving`) 순으로 주면 화면의 UGV가 움직여야 한다. 이 순서가 아니면 Idle이라 안 움직인다.
  - `RC_OperationMode=REMOTE` **AND** 제어권 보유 → Manual(움직임)
  - 둘 중 하나라도 아니면 Idle
  - 비상정지 래치가 걸려 있으면 무조건 Idle (해제해도 STAY로 떨어짐 — REMOTE로 자동 복귀 안 함)
- **RCWS 조준**: `RC_ActivateMovement`=RELEASE(활성) **AND** `RC_Movement.BrakeButton`=RELEASE일 때만
  pan/tilt가 먹는다(AND 조건, 2026-08-31 재매핑). 하나만 줘서는 안 돈다.
- 파싱 문제는 로그에 `JSON 파싱 실패` / `처리되지 않는 cmd '...'`(Verbose)로 남는다.

---

## 7. 자주 걸리는 것

| 증상 | 원인 / 조치 |
|---|---|
| UDP가 한 방향도 안 감, `UDP 소켓 시작` 로그 없음 | 데모 모드로 떴음 → `-fullsystem` 또는 체크박스 해제(§4-2) |
| 로그의 `send-to`가 `192.168.10.20`으로 남음 | RC IP 입력 후 Enter를 안 침(§4-1) |
| UGV→RC는 가는데 RC→UGV가 안 옴 | 통제기가 우리 PC IP의 8000/8001로 쏘고 있는지 + 방화벽(§3-3) |
| 창이 아예 안 뜸 (`wayland not available`) | 09-15 이후 패키지면 정상적으로는 안 나옴 — `titan_example.sh`에 프리앰블 블록이 있는지 확인(§2-4). 임시로 `./titan_example.sh -sdlvideodriver=x11`(§3-2) |
| 풀스크린에서 10fps대 | X11/Xwayland 경로로 뜬 것 → Wayland 세션에서 실행(§3-2) |
| RTSP 접속 거부 | 마운트가 아직 등록 안 됨(대기실 상태) / 8554 방화벽 / GStreamer 미설치 |
| RTSP는 붙는데 영상이 안 나옴 | (2026-09-15 정정) UDP 자체는 되지만, 방화벽/NAT가 있으면 동적 협상된 UDP RTP 포트가 막혀 이 모양이 된다 → `protocols=tcp`로 바꿔서 재시도. 그래도 안 나오면 §6-4 로그(`first encoded frame pushed`)와 §7-1 4번(수신 파이프라인 `videoconvert` 누락) 확인 |
| RTSP 마운트 로그가 아예 없음 | 빌드 PC에 NVENC SDK/CUDA/GStreamer가 없어 RTSP가 빠진 채 패키징됨(§2) |
| UGV가 통제기 명령에 반응 안 함 | 제어권 → REMOTE 순서, 비상정지 래치 확인(§6-5) |

---

## 7-1. 2026-09-04 실측 검증 기록

**검증 완료**: Ubuntu 22.04.5 / RTX 4070 SUPER / NVIDIA Open Kernel Module 610.43.02에서
**Wayland 세션과 Xorg 세션 양쪽 모두** 시뮬레이터 실행 + 파이썬 통제기 목업 UDP 연동 + RTSP 영상
수신까지 확인. 자동 판별 스크립트(`run_titan_example.sh`)가 두 세션에서 올바른 드라이버를 고르는
것도 확인.

### 이번에 잡힌 것 4건

1. **패키지가 GStreamer/NVIDIA .so를 `DT_NEEDED`로 직접 링크** — 없으면 RTSP만 꺼지는 게 아니라
   프로세스가 로더 단계에서 즉사. §3-1의 정정 블록 참고.
2. **`VideoDriver=wayland` 강제에 폴백이 없어 X11 전용 머신에서 즉사** — 외부 테스터가 이걸로
   4회 연속 실패(`Could not initialize SDL: wayland not available`). 대응으로 세션을 자동 판별하는
   `run_titan_example.sh`를 프로젝트 루트에 신설. 배포본에 같이 복사해야 함(패키징이 자동 포함 안 함).
   → **2026-09-15: 같은 로직을 `Config/BootstrapPreamble.sh`로 옮겨 `titan_example.sh`에 자동
   삽입되게 함. 복사 절차 폐지(§2-4).**
3. **Vulkan ICD 없음 → `Failed to load Vulkan Driver`** — `nvidia-smi`가 정상이고
   `ldconfig -p | grep libvulkan.so.1`도 통과하는데 실행이 막힌다. `libvulkan1`은 로더일 뿐이고
   드라이버 등록 파일(`/usr/share/vulkan/icd.d/nvidia_icd.json` — apt 설치본 기준; NVIDIA 공식
   `.run` 설치본은 `/etc/vulkan/icd.d/nvidia_icd.json`, 2026-09-15 확인)이 따로 필요. 검증은
   `vulkaninfo --summary`로 해야 하며, 라이브러리 존재 확인만으로는 못 잡는다.
4. **통제기 목업(리눅스)에서 영상이 검은 화면** — `avdec_h264`(I420 출력)와 `ximagesink`(RGB만 수용)
   사이에 `videoconvert`가 없어서 caps 협상 실패. GStreamer가 이걸
   `Internal data stream error ... not-negotiated (-4)`로 소스(udpsrc)까지 거슬러 올려 보고해서
   네트워크 문제로 오해하기 쉬웠다. RTSP 접속·SETUP·PLAY는 전부 성공한 뒤에 터진다.

### 진단이 오래 걸린 구조적 이유 (고침)

- `video_panel.py`가 GStreamer 오류를 **읽지도 않고 버렸다**(`has_error_or_eos()`가 메시지를 pop만
  하고 폐기, `play()`는 반환값 무시). 그래서 클라이언트 쪽에는 아무 단서가 안 남아 서버 로그와
  대조해야만 좁힐 수 있었다. 2026-09-04에 실패 경로 전체를 로깅하도록 수정 — 이제 파이썬 로그만
  보면 원인이 바로 나온다.
- `RtspStreamComponent::SetupEncoderAndStream()`이 **마운트 등록을 인코더 생성보다 먼저** 한다.
  인코더가 실패해도 클라이언트는 접속에 성공하므로, "접속은 되는데 영상만 안 나오는" 모양이 된다.
  판정하려면 `first encoded frame pushed to appsrc` 로그 유무를 봐야 한다.

### 우리 테스트 PC에서만 났던 문제 (배포물과 무관)

Xorg 세션 전환 시 검은 화면으로 멈췄는데, 원인은 **로컬에 남아 있던 깨진 `/etc/X11/xorg.conf`**
였다(`Parse error on line 43 ... "Monitor" is not a valid keyword` → `no screens found`).
`nvidia-settings`가 남긴 잔재로 보이며, 파일을 치우니 정상 동작. 배포 대상 PC와는 무관한 사안이다.

> 곁다리로 확인된 것: 이 PC는 **자동 로그인이 켜져 있어** 로그인 화면 자체를 건너뛰고 있었다.
> "세션 선택 톱니바퀴가 없다"의 원인이 이것이었고, `AutomaticLoginEnable=false`로 바꾸면 나온다.
> 저널도 volatile이라 `journalctl -b -1`로 이전 부팅 로그를 볼 수 없었다 —
> `sudo mkdir -p /var/log/journal` 로 영구화해두면 다음 사고 때 유리하다.

---

## 8. 참고 문서

- `protocol/protocol_icd.md` §3 — UDP/JSON ICD(IP·포트·메시지 전량), §3.3 RTSP 마운트
- `protocol/lig_icd_ugv_rc_full.md` — LIG ICD 원문
- `rtsp/rtsp_client_reception_guide.md` — **LIG 공유용** 수신측 저지연 가이드
- `rtsp/linux_wayland_x11_present_bottleneck.md` — Wayland/X11 프레임 폭락 원인·해결
- `level_new_kadex_0811/2026-09-01_scenario_run_modes_demo_fullsystem.md` — 데모/풀 시스템 스위치,
  대기실 버튼 계약, 패키징 대상 레벨
- `ui/kadex_test_dashboard_wbp_spec.md` — 대시보드 / 축 선택 위젯 구조
- `rtsp/rtsp_poc_findings.md` §10 — 리눅스 크로스컴파일 / GStreamer 번들 배경
