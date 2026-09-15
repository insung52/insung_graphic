# 새 노트북 — 리눅스 패키징 환경 구성 (RTSP 포함)

2026-09-15 / 새 노트북(RTX 4060 Laptop, 드라이버 610.88)에 titan_example 리눅스 패키징 환경을
처음부터 구성하는 절차. **RTSP 5스트림까지 정상 동작하는 패키지**를 뽑는 것이 목표.

기존 PC 기준 절차는 `2026-09-02_linux_package_ugv_host_rc_test_guide.md` §2, 배경은
`rtsp/rtsp_poc_findings.md` §10에 있다. 이 문서는 "아무것도 안 깔린 머신에서 시작"하는 경우다.

---

## 0. 왜 이 문서가 필요한가 — RTSP는 조용히 빠진다

`RtspEncoder.Build.cs`는 2026-08-18부터 **soft-fail**이다(§10.3.2). 외부 SDK를 하나도 못 찾아도
`BuildException`을 던지지 않고 `Log.TraceWarning`만 찍은 뒤 `RTSPENCODER_HAS_NVENC=0` /
`RTSPENCODER_HAS_GSTREAMER=0`으로 그 기능만 끈 채 나머지를 전부 정상 컴파일한다.

즉 **환경 구성이 반쯤 된 상태에서도 패키징은 "성공"으로 끝나고, 실행해보기 전까지 RTSP가
빠졌다는 걸 모른다.** 그래서 아래 §7의 검증(패키징 로그의 `[RtspEncoder]` 경고 확인)을 반드시
할 것.

---

## 1. 현재 상태 점검 (2026-09-15 실측)

| 항목 | 기본 경로 / env var | 상태 |
|---|---|---|
| UE 5.8 (uproject `EngineAssociation`) | `C:\Program Files\Epic Games\UE_5.8` | ✅ 있음 (5.7도 있음) |
| **리눅스 크로스컴파일 툴체인** | `C:\UnrealToolchains\` / `LINUX_MULTIARCH_ROOT` | ❌ **없음** |
| **NVIDIA Video Codec SDK 13.0.37** | `C:\SDK\Video_Codec_SDK_13.0.37` / `NVIDIA_VIDEO_CODEC_SDK_DIR` | ❌ **13.1.15만 있음** |
| Windows GStreamer (MSVC x86_64) | `C:\Program Files\gstreamer\1.0\msvc_x86_64` / `GSTREAMER_1_0_ROOT_MSVC_X86_64` | ✅ 있음 (env var도 설정됨) |
| Windows CUDA Toolkit | `C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.3` / `CUDA_PATH` | ✅ v13.3 (env var도 설정됨) |
| **Linux GStreamer 벤더링 번들** | `C:\SDK\gstreamer-1.24.2-linux-x86_64` / `GSTREAMER_1_0_ROOT_LINUX_X86_64` | ❌ **없음** |
| **Linux CUDA 벤더링 번들** | `C:\SDK\cuda-13.3-linux-x86_64` / `CUDA_LINUX_X86_64_DIR` | ❌ **없음** |
| WSL Ubuntu | — | ✅ 있음 (번들 생성에 필요) |

→ **해야 할 일은 4개: §2 툴체인, §3 Video Codec SDK, §5 Linux GStreamer 번들, §6 Linux CUDA 번들.**

경로들은 전부 `RtspEncoder.Build.cs`에 하드코딩된 기본값이다. 기본 경로에 그대로 깔면 환경변수를
따로 만들 필요가 없다(§4는 다른 위치에 깔았을 때만).

---

## 2. UE 리눅스 크로스컴파일 툴체인 (필수 — 이게 없으면 패키징 자체가 불가)

### ⚠️ 런처의 "Target Platforms ▸ Linux"는 툴체인이 아니다

에픽 런처에서 받는 Linux 타겟 플랫폼 컴포넌트는 엔진의 리눅스용 라이브러리/타겟 지원만 깐다.
실제로 컴파일을 수행하는 **clang 크로스컴파일 툴체인은 완전히 별개의 설치 파일**이고, 이게 없으면
에디터 Platforms 메뉴에서 Linux가 "SDK not installed"로 회색 처리된다.

### 버전은 반드시 `v26_clang-20.1.8-rockylinux8`

엔진이 요구하는 정확한 값은 `Engine/Config/Linux/Linux_SDK.json`에 박혀 있다. UE 5.8 / 5.7
**둘 다 동일**함을 확인:

```json
{
  "MainVersion" : "v26_clang-20.1.8-rockylinux8",
  "MinVersion"  : "v26_clang-20.1.8-rockylinux8",
  "MaxVersion"  : "v26_clang-20.1.8-rockylinux8",
  "AutoSDKPlatform" : "Linux_x64"
}
```

Min == Max이므로 다른 버전을 깔면 UBT가 그냥 거부한다. 기존 PC에서 쓰던 것과 같은 버전이다.

### 설치

1. Epic 공식 문서 **"Linux Development Requirements for Unreal Engine"** 페이지에서
   `native-linux-v26_clang-20.1.8-rockylinux8.exe`(1GB대)를 받는다.
   (CDN 직링크는 브라우저 외 요청에 403을 주므로 문서 페이지의 링크를 쓸 것.)
2. 실행 → 기본 경로 `C:\UnrealToolchains\v26_clang-20.1.8-rockylinux8`에 설치되고,
   설치 프로그램이 `LINUX_MULTIARCH_ROOT`를 시스템 환경변수로 자동 등록한다.
3. **에디터와 Epic Launcher를 완전히 종료 후 재시작.** 환경변수라서 실행 중인 프로세스엔 반영이
   안 된다. (Live Coding 세션이 떠 있으면 그것도 같이 내릴 것.)

### 확인

```powershell
[Environment]::GetEnvironmentVariable('LINUX_MULTIARCH_ROOT','Machine')
# → C:\UnrealToolchains\v26_clang-20.1.8-rockylinux8\
```
에디터 **Platforms** 메뉴에 **Linux**가 활성화되면 성공.

---

## 3. NVIDIA Video Codec SDK — **13.1.15가 아니라 13.0.37**

### 왜 버전을 내렸나

`RtspEncoder.Build.cs`의 주석(2026-09-15)에 기록된 이유:

> SDK 메이저.마이너가 곧 **최소 NVIDIA 드라이버 버전**이다(13.1 = 610+, 13.0 = 570+).
> `NvEncoder::LoadNvEncApi`가 드라이버의 `NvEncodeAPIGetMaxSupportedVersion`과 비교해서 낮으면
> throw한다. 고객(LIG) PC가 **595.84 고정**이라 13.1 빌드는 인코더 초기화에서 무조건 실패했다.

이 노트북 드라이버는 610.88이라 13.1도 로컬에선 돌지만, **배포 대상이 595.84라 13.0.37로 맞춰야
한다.** 리포의 벤더링 헤더도 이미 13.0으로 내려가 있다 —
`ThirdParty/NvCodec/Interface/nvEncodeAPI.h`가 `NVENCAPI_MAJOR_VERSION 13` /
`NVENCAPI_MINOR_VERSION 0`.

### ⚠️ `NVIDIA_VIDEO_CODEC_SDK_DIR`로 13.1.15를 가리키는 우회는 금지

Build.cs 주석 그대로: *"ThirdParty/NvCodec의 벤더링 헤더/샘플도 같은 버전이어야 하므로 여기만
올리지 말 것."* 헤더(13.0)와 lib(13.1)가 어긋나면 그 자리에서 안 터지고 런타임 인코더 초기화
실패로 나타난다 — 진단이 오래 걸리는 형태다.

### 설치

1. NVIDIA Developer 사이트에서 **Video Codec SDK 13.0.37** 아카이브를 받는다(로그인 필요).
2. `C:\SDK\Video_Codec_SDK_13.0.37\`에 압축을 푼다. 기존 `13.1.15` 폴더는 지울 필요 없다
   (Build.cs가 13.0.37만 보므로 공존해도 무해).
3. Build.cs가 실제로 확인하는 파일 **2개**가 그 자리에 있는지 확인:

```powershell
Test-Path 'C:\SDK\Video_Codec_SDK_13.0.37\Lib\win\x64\nvencodeapi.lib'                 # Windows용
Test-Path 'C:\SDK\Video_Codec_SDK_13.0.37\Lib\linux\stubs\x86_64\libnvidia-encode.so'  # Linux용
```
둘 다 `True`여야 한다. 하나라도 없으면 그 플랫폼의 NVENC가 꺼진다.
→ Linux 크로스컴파일은 이 **stub** `.so`에 링크하고, 실제 `libnvidia-encode.so.1`은 타겟 머신의
NVIDIA 독점 드라이버가 채워준다.

---

## 4. 환경변수 (기본 경로에 깔았으면 건드릴 것 없음)

Build.cs가 보는 오버라이드 변수. **기본 경로를 쓰면 전부 불필요하다.**

| env var | 기본값 | 현재 이 노트북 |
|---|---|---|
| `LINUX_MULTIARCH_ROOT` | (없음, 필수) | 툴체인 설치 시 자동 등록 |
| `NVIDIA_VIDEO_CODEC_SDK_DIR` | `C:\SDK\Video_Codec_SDK_13.0.37` | 미설정 = 기본값 사용 |
| `CUDA_PATH` | `C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.3` | ✅ 설정됨 (동일 값) |
| `CUDA_LINUX_X86_64_DIR` | `C:\SDK\cuda-13.3-linux-x86_64` | 미설정 = 기본값 사용 |
| `GSTREAMER_1_0_ROOT_MSVC_X86_64` | `C:\Program Files\gstreamer\1.0\msvc_x86_64` | ✅ 설정됨 (동일 값) |
| `GSTREAMER_1_0_ROOT_LINUX_X86_64` | `C:\SDK\gstreamer-1.24.2-linux-x86_64` | 미설정 = 기본값 사용 |

> Windows GStreamer는 **Complete 설치**여야 한다(Typical은 `gst-rtsp-server`가 빠진다).
> 확인: `Test-Path 'C:\Program Files\gstreamer\1.0\msvc_x86_64\lib\gstrtspserver-1.0.lib'`

---

## 5. Linux GStreamer 벤더링 번들 만들기

### 왜 번들인가

리눅스 패키징은 진짜 우분투에서 빌드하는 게 아니라 **이 윈도우 머신에서 크로스컴파일**한다
(§2 툴체인, RockyLinux8 sysroot, glibc 2.28). 그 sysroot엔 GStreamer가 없고, Build.cs가 도는
시점(윈도우)엔 진짜 리눅스 `pkg-config`를 돌릴 방법이 없다. 그래서 **실제 우분투에서 뽑아낸
헤더 + `.so` 사본**을 윈도우 디스크에 두고 링커에게 먹인다(§10.2).

**이 번들은 배포되지 않는다.** 링크 시점에만 쓰이고, 최종 바이너리는 `libgstreamer-1.0.so.0` 같은
SONAME으로 연결되어 실행 시점엔 타겟 머신에 apt로 깔린 진짜 GStreamer가 채워준다. 그래서
**번들 버전과 타겟 머신 버전이 호환돼야 한다** — 현재 양쪽 다 Ubuntu 24.04 / GStreamer 1.24.2 기준.

### 5-1. WSL Ubuntu에 dev 패키지 설치

WSL 배포판이 **Ubuntu 24.04**인지 먼저 확인(`lsb_release -d`). 22.04면 GStreamer가 1.20대라
번들 버전이 달라지므로, 24.04 배포판을 따로 설치해서 쓰는 걸 권장한다.

```bash
sudo apt update
sudo apt install -y \
  libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
  libgstrtspserver-1.0-dev libglib2.0-dev
pkg-config --modversion gstreamer-1.0   # → 1.24.x 확인
```

### 5-2. 번들 생성 스크립트

WSL 안에서 아래를 `~/make_bundle.sh`로 저장하고 실행한다. (원본은 기존 PC의
`C:\SDK\gstreamer-1.24.2-linux-x86_64\make_bundle.sh`에 있었다 — 새 노트북엔 없어서 §10.2의
번들 명세대로 재작성한 것.)

```bash
#!/bin/bash
# make_bundle.sh — 크로스컴파일용 GStreamer Linux 번들 생성 (WSL Ubuntu 24.04에서 실행)
set -euo pipefail

VER=$(pkg-config --modversion gstreamer-1.0)
OUT="/mnt/c/SDK/gstreamer-${VER}-linux-x86_64"
ARCH_DIR="/usr/lib/x86_64-linux-gnu"

echo "[make_bundle] GStreamer ${VER} -> ${OUT}"
mkdir -p "${OUT}/include" "${OUT}/lib"

# --- 헤더 (Build.cs의 PrivateIncludePaths와 이름이 정확히 일치해야 함) ---
cp -rL /usr/include/gstreamer-1.0   "${OUT}/include/gstreamer-1.0"
cp -rL /usr/include/glib-2.0        "${OUT}/include/glib-2.0"
cp -rL /usr/include/gio-unix-2.0    "${OUT}/include/gio-unix-2.0"
# glibconfig.h는 아키텍처별이라 /usr/include가 아니라 lib 밑에 있다.
# Build.cs는 이걸 include/glib-2.0-arch 라는 이름으로 찾는다.
mkdir -p "${OUT}/include/glib-2.0-arch"
cp -L "${ARCH_DIR}/glib-2.0/include/glibconfig.h" "${OUT}/include/glib-2.0-arch/"

# --- .so (Build.cs의 GstLibs 배열과 정확히 같은 11개) ---
# cp -L 로 심볼릭 링크 체인을 전부 풀어서 실제 파일로 뽑는다 —
# NTFS/WSL<->Windows 경계에서 다단계 symlink가 깨지는 문제가 있었다(§10.2).
GST_LIBS="libglib-2.0.so libgobject-2.0.so libgio-2.0.so libgmodule-2.0.so \
libgstreamer-1.0.so libgstbase-1.0.so libgstapp-1.0.so \
libgstrtsp-1.0.so libgstrtspserver-1.0.so libgstsdp-1.0.so libgstnet-1.0.so"

for so in ${GST_LIBS}; do
  cp -L "${ARCH_DIR}/${so}" "${OUT}/lib/${so}"
done

# 자체 검증 — 파일명이 하나라도 다르면 Build.cs가 조용히 GStreamer를 끈다
for so in ${GST_LIBS}; do
  [ -f "${OUT}/lib/${so}" ] || { echo "MISSING: ${so}"; exit 1; }
done
[ -f "${OUT}/include/glib-2.0-arch/glibconfig.h" ] || { echo "MISSING: glibconfig.h"; exit 1; }
[ -f "${OUT}/include/gstreamer-1.0/gst/gst.h" ]    || { echo "MISSING: gst.h"; exit 1; }

echo "[make_bundle] OK"
cp "$0" "${OUT}/make_bundle.sh"   # 다음에 다시 뽑을 때를 위해 스크립트도 같이 보관
```

```bash
chmod +x ~/make_bundle.sh && ~/make_bundle.sh
```

### 5-3. 결과 확인 (윈도우에서)

```powershell
Get-ChildItem C:\SDK\gstreamer-1.24.2-linux-x86_64\lib      # .so 11개
Get-ChildItem C:\SDK\gstreamer-1.24.2-linux-x86_64\include  # 폴더 4개
```

> 버전이 1.24.2가 아니면 폴더 이름이 달라진다 → 그 경우 `GSTREAMER_1_0_ROOT_LINUX_X86_64`를
> 실제 폴더로 설정하거나 폴더명을 바꿀 것. Build.cs 기본값은 `1.24.2` 고정이다.

---

## 6. Linux CUDA 벤더링 번들 만들기

리눅스에서 도는 인코더는 `FNvencVulkanEncoder`(NvEncoderCuda + Vulkan `VK_KHR_external_memory`
브릿지)뿐이라(§10.3, SDK에 `NvEncoderVulkan`은 없다) **리눅스 빌드에도 CUDA 드라이버 API가
필요하다.** Build.cs가 확인하는 건 딱 2개:

- `C:\SDK\cuda-13.3-linux-x86_64\include\cuda.h`
- `C:\SDK\cuda-13.3-linux-x86_64\lib\libcuda.so`  ← 툴킷의 **stub**

### 6-1. WSL에 CUDA Toolkit 13.3 설치

NVIDIA CUDA apt 저장소(ubuntu2404/x86_64)를 등록한 뒤:

```bash
sudo apt install -y cuda-toolkit-13-3
# 최소 구성으로 가려면 드라이버 API 헤더 + stub만 있으면 된다:
#   sudo apt install -y cuda-driver-dev-13-3 cuda-cudart-dev-13-3
```

> WSL에는 **NVIDIA 드라이버를 설치하지 말 것** (`cuda-drivers` 계열 금지). WSL2는 윈도우 호스트
> 드라이버를 `/usr/lib/wsl/lib`로 패스스루받으므로 게스트에 드라이버를 깔면 깨진다. 툴킷만 깐다.

설치 후 실제 경로를 확인한다(버전에 따라 `targets/x86_64-linux/` 밑일 수 있다):

```bash
find /usr/local/cuda-13.3 -name cuda.h
find /usr/local/cuda-13.3 -name 'libcuda.so' -path '*stubs*'
```

### 6-2. 번들 생성

```bash
#!/bin/bash
set -euo pipefail
OUT=/mnt/c/SDK/cuda-13.3-linux-x86_64
CUDA_H=$(find /usr/local/cuda-13.3 -name cuda.h | head -1)
CUDA_SO=$(find /usr/local/cuda-13.3 -name 'libcuda.so' -path '*stubs*' | head -1)
[ -n "$CUDA_H" ] && [ -n "$CUDA_SO" ] || { echo "cuda.h 또는 stub libcuda.so 없음"; exit 1; }

mkdir -p "$OUT/include" "$OUT/lib"
# cuda.h와 같은 디렉토리의 헤더를 통째로 가져간다(cuda.h가 다른 헤더를 끌어올 수 있음)
cp -rL "$(dirname "$CUDA_H")/." "$OUT/include/"
cp -L "$CUDA_SO" "$OUT/lib/libcuda.so"

grep -n 'CUDA_VERSION' "$OUT/include/cuda.h" | head -3   # 13030 계열인지 확인
echo "[cuda bundle] OK -> $OUT"
```

> **stub인 게 중요하다.** `lib64/stubs/libcuda.so`는 정확히 이런 링크 전용 용도로 존재한다 —
> 실제 `libcuda.so.1`은 타겟 머신의 NVIDIA 드라이버가 제공한다.
> WSL의 `/usr/lib/wsl/lib/libcuda.so.1`을 대신 쓰지 말 것(WSL 전용 빌드다).

> `cuda.h` 13.3에서 `cuCtxCreate`가 `cuCtxCreate_v4`로 리다이렉트되며 시그니처가 바뀌었고,
> `FNvencVulkanEncoder.cpp:301` 근처가 이미 그걸 전제로 작성돼 있다(§10.3.1). **버전을 13.3에서
> 임의로 올리지 말 것.**

---

## 7. 검증 — "성공"을 믿지 말고 로그를 볼 것

패키징 로그(또는 Output Log)에서 `[RtspEncoder]`를 검색한다.

**경고가 한 줄도 없으면 정상** = NVENC + GStreamer 둘 다 붙은 것.

다음 중 하나라도 보이면 그 기능이 빠진 채로 패키징된 것이다:

| 경고 | 원인 |
|---|---|
| `NVIDIA Video Codec SDK not found at 'C:\SDK\Video_Codec_SDK_13.0.37'` | §3 |
| `libnvidia-encode.so stub not found under ...` | §3-3 (SDK는 있는데 linux stub 경로가 없음) |
| `CUDA Linux dev bundle not found at 'C:\SDK\cuda-13.3-linux-x86_64'` | §6 |
| `libcuda.so not found under ...` | §6 (stub 복사 누락) |
| `GStreamer Linux dev bundle not found at ...` | §5 |
| `Expected GStreamer .so not found in bundle: ...` | §5-2 (파일명 오타 / 11개 중 일부 누락) |
| `GStreamer dev package not found at 'C:\Program Files\gstreamer\...'` | Windows GStreamer (Complete 설치 확인) |

패키지에 플러그인이 실제로 들어갔는지는 산출물 매니페스트에 `RtspEncoder.uplugin`이 있는지로
확인할 수 있다(§10.2.1에서 쓴 방법).

---

## 8. 패키징 실행 — 기본 Package Project 메뉴를 쓰면 안 된다

### 8-1. `Config/DefaultGame.ini` 확인 (현재 리포에 살아 있음 ✅)

```ini
+MapsToCook=(FilePath="/Game/kadex_lobby")
+MapsToCook=(FilePath="/Game/New_kadex_0811")
+DirectoriesToAlwaysCook=(Path="/Game/Input")
```
게임 레벨은 문자열 트래블(`open ...`)로만 도달해서 쿠커가 정적 분석으로 못 찾고,
`DA_TitanInputSchema`도 `LoadObject()` 하드코딩 경로라 같은 이유로 빠진다.

> ⚠️ **에디터에서 Project Settings ▸ Packaging 화면을 열고 저장하면 이 섹션이 덮어써진다.**
> ini를 직접 고쳤으면 에디터를 재시작한 뒤 패키징할 것.

### 8-2. **Platforms ▸ Project Custom Builds ▸ "Package Linux (MCP 8000 회피)"**

2026-09-15자로 `DefaultGame.ini`에 추가된 항목. 기본 `Package Project` 메뉴를 쓰면:

- 쿠커는 별도 `UnrealEditor-Cmd` 프로세스인데 `EditorPerProjectUserSettings`의
  ModelContextProtocol `bAutoStartServer=True`(개인 설정이라 프로젝트 Config로 못 덮음)를 읽어
  MCP 서버를 8000에 띄우려다 에디터가 이미 잡고 있는 포트에 부딪힌다
  → `LogHttpListener: Error: HttpListener unable to bind to 127.0.0.1:8000`
- 쿡 자체가 2055/2055 전부 성공해도 **Error 로그 1줄이면 커맨드릿이 ExitCode=1**을 돌려서
  (`GWarn->GetNumErrors()>0`) UAT가 실패로 판정한다 → `Cook failed`

커스텀 빌드는 쿠커에만 `-additionalcookeroptions=-ModelContextProtocolPort=8001`을 넘겨 포트를
비킨다. 그 외 UAT 커맨드와 출력 폴더 대화창은 기본 Package Project와 동일하다.

### 8-3. 빌드 구성은 Development

확인용 로그가 전부 `titan_example.log`에 찍혀야 하기 때문. Shipping으로 뽑으면
`2026-09-02_...md` §6의 검증 절차를 대부분 못 쓴다.

### 8-4. 산출물에 `run_titan_example.sh` 수동 복사

프로젝트 루트에 있고 **UE 패키징이 자동으로 넣어주지 않는다.** 세션 종류(Wayland/X11)를 자동
판별해서 SDL 드라이버를 고르는 래퍼다. `LinuxEngine.ini`가 `VideoDriver=wayland`를 **폴백 없이**
강제하므로, 순수 X11 머신에서 이게 없으면 `Could not initialize SDL: wayland not available`로
0.04초 만에 죽는다(외부 테스터가 이걸로 4회 연속 실패한 이력).

`titan_example_x11_fallback.sh`는 X11 강제용 구버전 래퍼로, 같이 복사해두면 무해하다.

---

## 9. 타겟 리눅스 머신 (배포 받는 쪽)

`2026-09-02_linux_package_ugv_host_rc_test_guide.md` §3 전문 참고. 핵심만:

```bash
sudo apt install -y \
  libgstreamer1.0-0 libgstreamer-plugins-base1.0-0 libgstrtspserver-1.0-0 \
  gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad \
  gstreamer1.0-tools libvulkan1
```

> ⚠️ **GStreamer/NVIDIA는 "없어도 게임은 뜬다"가 아니다.** Linux 분기가 `PublicAdditionalLibraries`로
> 평범한 직접 링크를 하므로 실행 파일에 `DT_NEEDED`가 박힌다. 하나라도 없으면 **동적 로더
> 단계에서 프로세스가 즉사**한다(`Saved/Logs`도 안 생김). NVIDIA GPU가 없거나 nouveau/Mesa인
> 머신에서는 지금 빌드가 아예 실행되지 않는다.
>
> 실행 실패 신고를 받으면 `nvidia-smi` → `ldconfig -p | grep libnvidia-encode` →
> `vulkaninfo --summary` 순으로 확인할 것. `libvulkan1`은 로더일 뿐이고 ICD 등록 파일
> (`/usr/share/vulkan/icd.d/nvidia_icd.json`)이 따로 필요하다 — 라이브러리 존재 확인만으론 못 잡는다.

자가 진단:
```bash
ldd titan_example/Binaries/Linux/titan_example | grep "not found"
```

**드라이버 최소 버전**: Video Codec SDK 13.0 → NVIDIA **570 이상**. (LIG PC 595.84 ✅)

RTSP 실제 수신 확인:
```bash
gst-launch-1.0 rtspsrc location=rtsp://<UGV IP>:8554/ugv/rcws latency=0 drop-on-latency=true protocols=tcp \
  ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! autovideosink sync=true qos=true
```
> `videoconvert`를 빼면 `avdec_h264`(I420)와 `ximagesink`(RGB) 사이 caps 협상이 실패하고,
> GStreamer가 그걸 소스까지 거슬러 `not-negotiated (-4)`로 보고해서 네트워크 문제로 오해하기 쉽다
> (2026-09-04에 실제로 겪음).

---

## 10. 요약 체크리스트

- [ ] `C:\UnrealToolchains\v26_clang-20.1.8-rockylinux8` 설치 + `LINUX_MULTIARCH_ROOT` 확인 + 에디터 재시작
- [ ] `C:\SDK\Video_Codec_SDK_13.0.37` (13.1.15 아님) — win lib + linux stub 2개 파일 확인
- [ ] `C:\SDK\gstreamer-1.24.2-linux-x86_64` — include 4폴더 + lib `.so` 11개
- [ ] `C:\SDK\cuda-13.3-linux-x86_64` — `include/cuda.h` + `lib/libcuda.so`(stub)
- [ ] Windows GStreamer Complete 설치 확인 (`gstrtspserver-1.0.lib` 존재) ✅ 이미 됨
- [ ] Windows CUDA v13.3 ✅ 이미 됨
- [ ] 패키징 로그에 `[RtspEncoder]` 경고 **0줄**
- [ ] Platforms ▸ Project Custom Builds ▸ "Package Linux (MCP 8000 회피)" / Development
- [ ] 산출물에 `run_titan_example.sh` 복사

---

## 11. 참고 문서

- `packaging/2026-09-02_linux_package_ugv_host_rc_test_guide.md` — 뽑은 패키지로 통제기 연동 확인하는 절차
- `rtsp/rtsp_poc_findings.md` §10 — 크로스컴파일/번들 설계의 배경과 겪은 문제 전량
  (§10.2 Phase 1 빌드 시스템, §10.2.1 UBT 플랫폼 파일 제외 규칙, §10.3 Phase 2 Vulkan/CUDA 인코더,
  §10.3.2 soft-fail 전환)
- `rtsp/linux_wayland_x11_present_bottleneck.md` — `VideoDriver=wayland` 강제의 이유
- `rtsp/rtsp_client_reception_guide.md` — 수신측 저지연 세팅 (LIG 공유용)
