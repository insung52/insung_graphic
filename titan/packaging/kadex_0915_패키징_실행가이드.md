# UGV 시뮬레이터 실행 가이드 (Ubuntu) — 2026-09-15판

2026-09-15 / 완료 / 아무것도 설치되지 않은 Ubuntu에서 패키지 실행 → RC IP 입력 → UGV Host 시작까지. `kadex_0902_패키징_실행가이드.md`를 대체함 — 09-15 빌드 기준(`titan_example.sh`에 Wayland/X11 자동 판별 내장, NVIDIA 드라이버 최소 570, RTSP 전송 TCP/UDP 정정).

UGV 시뮬레이션 SW를 실행해서 **원격통제기와 UDP 통신 / RTSP 영상**을 연동하기 위한 문서입니다.
**설치가 전혀 안 된 새 Ubuntu 기준**으로, §0부터 순서대로 그대로 따라 하시면 됩니다.

---

## 0. 먼저 확인해 주세요

| 항목 | 요구 사항 | 확인 명령 |
|---|---|---|
| OS | **Ubuntu 20.04 이상** (22.04 / 24.04 권장) | `lsb_release -d` |
| GPU | **NVIDIA GPU 필수** (하드웨어 인코더 NVENC 탑재 모델 — GeForce/RTX/Quadro 계열은 전부 해당) | `lspci \| grep -i nvidia` |
| NVIDIA 드라이버 | **570 이상** (595.84로 실측 검증됨) | `nvidia-smi` 상단 `Driver Version` |
| 화면 | 모니터가 연결된 **데스크톱 세션**에서 실행 (Wayland / X11 어느 쪽이든 됨) | — |

> ⚠️ **NVIDIA GPU가 없으면 실행되지 않습니다.** 현재 빌드는 NVIDIA 전용 라이브러리
> (`libnvidia-encode.so.1`, `libcuda.so.1`)를 필수로 요구합니다. AMD/Intel GPU, 또는 NVIDIA GPU라도
> 기본 드라이버(nouveau)만 설치된 상태에서는 창이 뜨기 전에 종료됩니다.
>
> ⚠️ **드라이버는 570 이상이어야 RTSP 영상이 나옵니다.** 프로그램 자체는 더 낮은 버전에서도 뜨지만,
> 영상 인코더(NVENC) 초기화가 실패해서 **RTSP 접속이 안 됩니다**(이 빌드부터는 접속 시 즉시
> `404 Not Found`, 09-02 빌드는 20초 타임아웃). 이때 로그
> `titan_example/Saved/Logs/titan_example.log`에 `NVENC init failed: ... Current Driver Version does
> not support this NvEncodeAPI version` 가 남습니다. 570~609 대 및 그 이상 어느 버전이든 됩니다
> (2026-09-15 빌드 기준, **595.84에서 5스트림 정상 동작 확인**. 이전 09-02 빌드는 610 이상이
> 필요했음 — 그 빌드에서 영상이 안 나왔다면 이 문제입니다).
>
> ⚠️ **SSH 원격 접속만으로는 실행할 수 없습니다.** 화면 출력이 필요하므로 실제 모니터가 연결된
> PC 앞에서(또는 물리 화면에 연결된 원격 데스크톱 세션에서) 실행해 주세요.

---

## 1. 환경 설치 (최초 1회)

**§1-1 → §1-2 → §1-3 순서대로** 진행해 주세요. 재부팅은 §1-2에서 한 번만 하면 됩니다.

### 1-1. 필수 라이브러리 설치

```bash
sudo apt update

# (1) GStreamer + Vulkan — 없으면 프로그램이 시작조차 안 됨
sudo apt install -y \
  libgstreamer1.0-0 libgstreamer-plugins-base1.0-0 libgstrtspserver-1.0-0 \
  gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad \
  libvulkan1

# (2) 창 생성 / 입력 / 오디오 런타임 — 데스크톱 설치본이면 대부분 이미 있으나,
#     최소 설치본에서는 빠져 있어 창이 안 뜨는 원인이 됨
sudo apt install -y \
  libwayland-client0 libwayland-cursor0 libwayland-egl1 libxkbcommon0 \
  libx11-6 libx11-xcb1 libxext6 libxcursor1 libxi6 libxrandr2 libxfixes3 libxss1 libxtst6 \
  libasound2 libpulse0 libudev1 libdbus-1-3 libgl1 libstdc++6
```

> ⚠️ (1)의 라이브러리들은 **영상 기능용 선택 사항이 아니라 실행 필수 조건**입니다. 하나라도 없으면
> `error while loading shared libraries: ...` 메시지와 함께 프로그램이 시작되지 않습니다.
> (2)는 프로그램이 실행 중에 찾는 것들이라 없으면 창이 안 뜨거나 소리·입력이 동작하지 않습니다.

### 1-2. NVIDIA 독점 드라이버 + 재부팅

이미 `nvidia-smi`가 정상 동작하고 **`Driver Version`이 570 이상**이라면 재부팅 없이 §1-3으로 넘어가도 됩니다.

```bash
sudo apt install -y ubuntu-drivers-common
sudo ubuntu-drivers install        # 권장 드라이버 자동 선택·설치
sudo reboot                        # 재부팅 필수
```

> ⚠️ 설치 후 `nvidia-smi`의 `Driver Version`이 **570 미만**이면 §0의 요구사항 미달입니다.
> `ubuntu-drivers list`로 570 이상 버전이 있는지 확인해서 `sudo apt install nvidia-driver-570`처럼
> 버전을 지정해 설치하거나, 배포판 저장소에 없으면 NVIDIA 공식 저장소/설치본을 사용하세요.

> ⚠️ **Secure Boot가 켜져 있으면** 드라이버 설치 중에 비밀번호(MOK) 등록 화면이 나오고,
> **재부팅 시 파란 화면에서 "Enroll MOK"를 선택해 그 비밀번호를 입력해야** 드라이버가 활성화됩니다.
> 이 과정을 놓치면 재부팅 후에도 `nvidia-smi`가 실패합니다 — 그 경우 BIOS에서 Secure Boot를 끄고
> `sudo apt install --reinstall nvidia-driver-<버전>` 후 다시 재부팅하는 게 가장 간단합니다.

> ⚠️ **`nvidia-smi`가 나온다고 Vulkan까지 되는 건 아닙니다.** `nvidia-smi`는 커널 모듈만 확인하는데,
> 이 프로그램은 Vulkan으로 렌더링하므로 드라이버의 유저스페이스 부분(Vulkan ICD)도 필요합니다.
> §1-3의 Vulkan 확인을 반드시 거치세요.

<details>
<summary>NVIDIA 공식 <code>.run</code> 설치본으로 특정 버전을 맞추는 경우 (예: 595.84)</summary>

배포판 저장소에 원하는 버전이 없어 `NVIDIA-Linux-x86_64-<버전>.run`을 쓰실 때 걸리는 점만
요약합니다(사내에서 595.84로 실증한 절차입니다).

- **Secure Boot는 꺼야 합니다**(`mokutil --sb-state` → `SecureBoot disabled`). `.run`이 빌드하는
  커널 모듈은 서명이 없어서 켜진 채로는 설치가 끝난 것처럼 보여도 `nvidia-smi`가 실패합니다.
- **기존 apt 드라이버를 먼저 완전히 제거**하세요(`sudo apt purge 'nvidia-*' 'libnvidia-*'`).
  섞이면 `libnvidia-encode.so`/`libcuda.so` 버전이 커널 모듈과 달라져 NVENC 초기화가 이상한
  에러로 실패합니다.
- nouveau 블랙리스트 + `update-initramfs -u` + 재부팅 후, **텍스트 콘솔**(Ctrl+Alt+F3 →
  `sudo systemctl isolate multi-user.target`)에서 `sudo ./NVIDIA-Linux-x86_64-<버전>.run --dkms`.
  설치기 질문은 proprietary 커널 모듈 / DKMS yes / 32-bit yes / `nvidia-xconfig` **no**.
- `.run` 설치본은 Vulkan ICD를 `/etc/vulkan/icd.d/nvidia_icd.json`에 넣습니다
  (`/usr/share/vulkan/icd.d/`는 비어 있어도 정상). §1-3의 확인은 두 경로를 다 봅니다.

</details>

### 1-3. 설치 검증 (재부팅 후)

먼저 드라이버부터 확인합니다. 표 형태로 GPU 정보가 나오고, **상단 `Driver Version`이 570 이상**이면 정상입니다.

```bash
nvidia-smi
```

이어서 아래를 그대로 복사해서 실행하세요. **10줄 모두 `OK`** 여야 합니다.

```bash
for lib in libnvidia-encode.so.1 libcuda.so.1 libvulkan.so.1 \
           libglib-2.0.so.0 libgobject-2.0.so.0 \
           libgstreamer-1.0.so.0 libgstapp-1.0.so.0 libgstrtspserver-1.0.so.0 \
           libwayland-client.so.0 libxkbcommon.so.0; do
  if ldconfig -p | grep -q "$lib"; then echo "OK    $lib"; else echo "없음  $lib"; fi
done
```

`없음`이 나오면:

| 없는 라이브러리 | 해결 |
|---|---|
| `libnvidia-encode.so.1`, `libcuda.so.1` | §1-2 (NVIDIA 드라이버). `nvidia-smi`부터 확인 |
| `libvulkan.so.1` | §1-1 (1) |
| `libgstreamer-*`, `libgstapp-*`, `libgstrtspserver-*`, `libglib-*`, `libgobject-*` | §1-1 (1) |
| `libwayland-client.so.0`, `libxkbcommon.so.0` | §1-1 (2) |

#### ⚠️ Vulkan 드라이버(ICD) 확인 — 위 검사만으로는 부족합니다

`libvulkan.so.1`은 **로더**일 뿐이고, 실제 그림을 그리는 건 GPU 벤더가 제공하는 **드라이버 등록
파일(ICD)** 입니다. 로더만 있고 ICD가 없으면 위 검사는 전부 `OK`인데 실행 시
**"Failed to load Vulkan Driver which is required to run the engine."** 로 종료됩니다.
`nvidia-smi`도 이 경우 정상 동작합니다(커널 모듈만 보기 때문) — 실제로 이 조합으로 실행이 막힌
사례가 있었습니다.

```bash
sudo apt install -y vulkan-tools

# ① ICD 파일이 있어야 함 — NVIDIA면 nvidia_icd.json. 설치 방식에 따라 위치가 다르므로 두 곳 다 확인
ls /usr/share/vulkan/icd.d/ /etc/vulkan/icd.d/ 2>/dev/null
#   apt(ubuntu-drivers) 설치본 → /usr/share/vulkan/icd.d/nvidia_icd.json
#   NVIDIA 공식 .run 설치본     → /etc/vulkan/icd.d/nvidia_icd.json

# ② 실제로 GPU가 잡히는지 (driverName / deviceName 이 나와야 정상) — 이게 최종 판정
vulkaninfo --summary | head -30
```

두 경로 모두에 `nvidia_icd.json`이 없거나 `vulkaninfo`가 장치를 못 찾으면 드라이버의 유저스페이스
부분이 빠진 것입니다. 드라이버를 재설치하세요.

```bash
sudo apt install --reinstall nvidia-driver-570    # 설치된 버전에 맞게(570 이상). nvidia-smi 우측 상단에 표시됨
sudo reboot
```

> `nvidia-headless-*` / `nvidia-driver-*-server` 계열이나 `.run` 설치본을 옵션 없이 설치하면
> 커널 모듈만 깔리고 Vulkan ICD(`libnvidia-gl-<버전>`)가 빠질 수 있습니다.
> NVIDIA 공식 `.run` 설치본을 쓰신 경우 ICD는 `/etc/vulkan/icd.d/`에 들어가는 것이 정상이며
> (`/usr/share/vulkan/icd.d/`는 비어 있어도 됨), `vulkaninfo`에 NVIDIA가 잡히면 문제 없습니다.

### 1-4. 방화벽 (사용 중일 때만)

```bash
sudo ufw allow 8000/udp   # 통제기 → UGV (주기)
sudo ufw allow 8001/udp   # 통제기 → UGV (비주기)
sudo ufw allow 8554/tcp   # RTSP (TCP 전송이면 이 포트 하나로 제어+영상이 다 나감)
```

> RTSP 영상은 TCP(interleaved) / UDP 둘 다 받을 수 있지만, **수신측이 UDP를 고르면 영상(RTP/RTCP)용
> UDP 포트가 접속할 때마다 동적으로 정해져서** 고정 포트로 열어둘 수 없습니다. 방화벽을 쓰신다면
> 수신측에서 TCP 전송(`protocols=tcp`)을 지정해 주세요 — 그러면 위 8554/tcp만 열면 됩니다(§5).

---

## 2. 패키지 준비

전달받은 압축을 풀면 아래 구조입니다.

```
titan_example.sh                  ← ★ 이걸로 실행하세요 (Wayland/X11 세션 자동 판별 내장)
titan_example/                    ← 게임 데이터
Engine/
```

> 이전(09-02) 패키지에 있던 `run_titan_example.sh` / `titan_example_x11_fallback.sh`는 **더 이상
> 필요 없습니다.** 세션 자동 판별이 `titan_example.sh` 자체에 들어갔습니다. 만약 폴더에 그 파일들이
> 같이 들어 있더라도 그냥 `titan_example.sh`를 쓰시면 되고, 그 스크립트들로 실행해도 동작합니다.

압축 방식에 따라 실행 권한이 사라지므로, 압축을 푼 폴더에서 아래를 실행해 주세요.

```bash
chmod +x titan_example.sh
chmod +x titan_example/Binaries/Linux/titan_example
```

이어서 실행 전 최종 점검입니다. **아무것도 출력되지 않아야** 정상입니다.

```bash
ldd titan_example/Binaries/Linux/titan_example | grep "not found"
```

`not found`가 나오면 그 라이브러리가 빠진 것이므로 §1-3의 표를 참고해 주세요.

---

## 3. 실행

```bash
./titan_example.sh
```

이 스크립트가 **현재 세션이 Wayland인지 X11인지 자동으로 판별해서** 알맞은 옵션으로 실행합니다.
세션 종류를 미리 확인하실 필요 없습니다. 실행하면 어느 쪽으로 뜨는지 한 줄이 먼저 출력됩니다.

| 세션 | 터미널 첫 줄 | 결과 |
|---|---|---|
| Wayland | `[titan_example] Wayland 세션 감지 — 네이티브 Wayland로 실행합니다.` | 네이티브 Wayland |
| X11(Xorg) 또는 Wayland 없는 PC | `[titan_example] Wayland를 찾지 못했습니다 — X11로 실행합니다 (-sdlvideodriver=x11).` | 네이티브 X11 |

**양쪽 모두 Ubuntu 22.04에서 실측 확인했습니다**(2026-09-04 Wayland/Xorg 실행 검증, 2026-09-15
내장 스크립트 검증). XWayland 경유로 잘못 뜨는 경우는 없습니다(Wayland 세션이면 네이티브 Wayland로
보내기 때문).

제대로 떴는지는 로그로도 확인할 수 있습니다.

```bash
grep "SDL video driver" titan_example/Saved/Logs/titan_example.log
# X11  : Command line override: SDL video driver set to 'x11'  / Using SDL video driver 'x11'
# Wayland: INI override: SDL video driver set to 'wayland'     / Using SDL video driver 'wayland'
```

전체화면으로 띄우려면 뒤에 `-fullscreen`을 붙입니다(그 외 인자도 그대로 전달됩니다).

```bash
./titan_example.sh -fullscreen
```

<details>
<summary>수동으로 지정하고 싶을 때</summary>

`-sdlvideodriver=...`를 직접 주면 자동 판별을 건너뛰고 그 값을 그대로 씁니다.

```bash
./titan_example.sh -sdlvideodriver=x11        # X11 강제
./titan_example.sh -sdlvideodriver=wayland    # Wayland 강제
```

⚠️ Wayland 컴포지터가 없는 PC에서 `-sdlvideodriver=wayland`를 주면
`Could not initialize SDL: wayland not available`로 **즉시 종료**됩니다. 옵션 없이 실행하면 자동
판별이 X11로 보내므로 이 문제가 없습니다.

</details>

- **반드시 터미널에서 실행해 주세요.** 실행에 실패하면 원인이 터미널 출력에만 표시됩니다
  (라이브러리 문제로 못 뜨는 경우에는 로그 파일도 생성되지 않습니다).
- **`-vulkan` / `-dx12` / `-opengl` / `-sm5` 같은 렌더러 인자는 주지 마세요.** 리눅스는 Vulkan이
  기본이고 다른 값은 지원하지 않습니다. 실행이 안 될 때 이런 인자로 우회를 시도하면
  `Trying to force specific Vulkan feature level but it is not supported.` 같은 **다른 에러로
  바뀌기만 해서** 원인 파악이 더 어려워집니다. 인자 없이 실행하고 §6을 보세요.
- GNOME/Wayland에서는 **창 테두리와 타이틀바가 없는 것이 정상**입니다. `Super` 키를 누른 채
  드래그하면 창을 옮길 수 있습니다.
- 첫 실행은 셰이더 준비 때문에 시간이 조금 걸릴 수 있습니다.

---

## 4. 축 선택 화면 — 입력 항목

실행하면 축 선택 화면이 먼저 뜹니다. **UGV Host 버튼을 누르기 전에** 아래 값을 입력합니다.

### 필수 — RC IP

| 항목 | 넣을 값 | 기본값 |
|---|---|---|
| **RC IP** | **원격통제기 SW가 실행되는 PC의 IP** | `192.168.10.20` |

> ⚠️ **입력 후 반드시 Enter를 눌러주세요.** 입력값은 Enter를 누르거나 다른 곳을 클릭해서 커서가
> 빠져나갈 때 반영됩니다. 타이핑만 하고 바로 Host 버튼을 누르면 기본값이 그대로 사용됩니다.

### 선택 — 포트

기본값이 ICD 규격값이라 보통 바꿀 필요가 없습니다. 바꿀 경우 각 칸마다 Enter를 눌러주세요.

| 항목 | 기본값 |
|---|---|
| RC Periodic Port | `8010` |
| RC Event Port | `8011` |
| UGV Listen Periodic Port | `8000` |
| UGV Listen Event Port | `8001` |

### 선택 — RTSP 송출 해상도

바꾸지 않으면 기본값으로 송출됩니다. 역시 각 칸마다 Enter가 필요합니다.

| 항목 | 기본값 |
|---|---|
| RCWS 조준경 (가로 × 세로) | `1920` × `1080` |
| CCTV 4방 공통 (가로 × 세로) | `320` × `180` |

### 데모 모드 체크박스

기본값(해제) 그대로 두세요. 체크하면 통제기 없이 혼자 돌아가는 데모로 실행되어 UDP 통신이 되지
않습니다.

### UGV Host 버튼

입력을 마쳤으면 **UGV Host** 버튼을 누릅니다. 시나리오 레벨로 이동하면서 UDP 소켓과 RTSP 스트림이
열립니다.

> 옆에 있는 **Client** / **호스트 없이 시작** 버튼은 다른 용도(이동형지휘소 축, 단독 데모)입니다.
> 통제기 연동 확인에는 사용하지 않습니다.

---

## 5. 통제기 쪽 접속 정보

`<UGV IP>` = 이 프로그램을 실행한 PC의 IP (`ip addr`로 확인).

### UDP (JSON)

| 방향 | 주소 |
|---|---|
| 통제기 → UGV | `<UGV IP>` : **8000**(주기) / **8001**(비주기) |
| UGV → 통제기 | 축 선택 화면에 입력한 **RC IP** : **8010**(주기) / **8011**(비주기) |

### RTSP (5개 스트림)

| 스트림 | URL |
|---|---|
| RCWS 조준경 | `rtsp://<UGV IP>:8554/ugv/rcws` |
| 전면 CCTV | `rtsp://<UGV IP>:8554/ugv/front_cctv` |
| 후면 CCTV | `rtsp://<UGV IP>:8554/ugv/rear_cctv` |
| 좌측 CCTV | `rtsp://<UGV IP>:8554/ugv/left_cctv` |
| 우측 CCTV | `rtsp://<UGV IP>:8554/ugv/right_cctv` |

- 코덱은 H.264 High Profile, B프레임 없음. RTP 전송은 TCP(interleaved) / UDP 둘 다 됩니다.
  **TCP를 권장합니다** — 저지연 수신 설정을 TCP 기준으로 검증했고, UDP는 영상 포트가 접속 시
  동적으로 협상되어 방화벽이 있으면 막히기 쉽습니다(§1-4). GStreamer면 `rtspsrc`에 `protocols=tcp`.
- 스트림은 UGV Host로 레벨에 진입한 뒤에 열립니다. 축 선택 화면 상태에서는 아직 접속되지 않습니다.
- 각 스트림은 **영상 인코더가 정상 시작된 경우에만** 등록됩니다. 드라이버 버전 미달(§0) 등으로
  인코더가 실패하면 해당 URL은 `404 Not Found`를 돌려줍니다.

연결 확인용 예시입니다. 이 명령에만 필요한 패키지를 먼저 설치해 주세요
(`avdec_h264`는 §1-1에서 설치한 패키지들에 들어있지 않고 `gstreamer1.0-libav`에 있습니다).

```bash
sudo apt install -y gstreamer1.0-tools gstreamer1.0-libav

gst-launch-1.0 rtspsrc location=rtsp://<UGV IP>:8554/ugv/rcws latency=0 \
  ! rtph264depay ! h264parse ! avdec_h264 ! autovideosink
```

- `no element "avdec_h264"` → 위 `gstreamer1.0-libav` 설치 누락
- `autovideosink`에서 오류가 나면 `ximagesink`로 바꿔서 다시 시도
- **창이 뜨긴 하는데 계속 검은 화면이면 그대로 두지 마시고 알려주세요.** 접속은 됐는데 영상
  데이터가 안 오는 상태이며, 시뮬레이터 쪽 로그를 같이 봐야 합니다.

---

## 6. 문제 해결

### 실행이 안 될 때

| 터미널에 나오는 메시지 / 증상 | 원인과 조치 |
|---|---|
| **`Failed to load Vulkan Driver which is required to run the engine.`** | Vulkan ICD 없음. `nvidia-smi`가 되더라도 발생합니다 → §1-3의 Vulkan 확인 |
| **`Could not initialize SDL: wayland not available`** + `InitSDL() failed` | Wayland가 없는 환경인데 Wayland로 뜨려 한 것. 자동 판별을 건너뛰는 `-sdlvideodriver=wayland`를 주셨다면 빼고 실행. 옵션 없이 실행했는데도 이 메시지가 나오면 (`WAYLAND_DISPLAY` 변수가 남아 있거나) 스크립트에 자동 판별 블록이 빠진 패키지일 수 있습니다 → `./titan_example.sh -sdlvideodriver=x11`로 실행하고 알려주세요 |
| `Vulkan Driver is required to run the engine.` (`-vulkan` 지정 시) | 위와 동일 원인 |
| `Trying to force specific Vulkan feature level but it is not supported.` | `-sm5` 같은 RHI 인자를 준 경우 → §3 참고, 인자를 빼고 실행 |
| `error while loading shared libraries: libnvidia-encode.so.1` 또는 `libcuda.so.1` | NVIDIA 독점 드라이버 미설치 → §1-2 |
| `error while loading shared libraries: libgst...` / `libglib...` | GStreamer 미설치 → §1-1 (1) |
| `Permission denied` | 실행 권한 없음 → §2의 `chmod +x` |
| 창이 안 뜨고 SDL / video driver 관련 메시지 | `./titan_example.sh -sdlvideodriver=x11`로 실행해 보세요. 그래도 안 되면 §1-1 (2) 설치 확인 |
| Vulkan / RHI 관련 오류 | `libvulkan1`(§1-1)과 `nvidia-smi`(§1-2) 확인. `sudo apt install -y vulkan-tools` 후 `vulkaninfo --summary`로 NVIDIA 드라이버가 잡히는지 추가 확인 가능 |
| 소리가 안 남 / 오디오 장치 오류 | §1-1 (2)의 `libasound2`, `libpulse0` 설치 확인 (실행 자체에는 지장 없음) |
| 전체화면에서 프레임이 매우 낮음 | X11 세션에서는 정상입니다(알려진 제약). 창모드로 쓰시거나, Wayland 세션이 있는 PC라면 그쪽에서 실행하세요 |

### 통신이 안 될 때

로그 파일: `titan_example/Saved/Logs/titan_example.log`

| 확인할 내용 | 로그에서 찾을 문자열 |
|---|---|
| UDP 소켓이 열렸는지, 목적지 IP가 맞는지 | `UDP 소켓 시작` |
| RTSP 스트림 5개가 열렸는지 | `Registered RTSP mount` (5줄) |
| 영상 인코더가 정상 시작됐는지 | `NVENC Vulkan/CUDA encoder ready` (5줄). 대신 `NVENC init failed`가 있으면 드라이버 버전(§0) |
| 통제기가 붙었을 때 실제로 영상이 나가는지 | `prepared for a client` 직후 `first encoded frame pushed` |

| 증상 | 확인할 것 |
|---|---|
| UDP가 전혀 오가지 않음 | 데모 모드 체크박스가 해제된 상태로 시작했는지(§4) |
| UGV → 통제기 방향만 안 감 | 로그의 `UDP 소켓 시작` 줄에 찍힌 목적지 IP가 입력한 RC IP인지 (§4의 Enter) |
| 통제기 → UGV 방향만 안 옴 | 통제기가 `<UGV IP>`의 8000/8001로 보내고 있는지, 방화벽(§1-4) |
| RTSP 접속 자체가 안 됨 (`Failed to connect`) | UGV Host로 레벨에 들어갔는지, `<UGV IP>`가 맞는지(`ip addr`), 8554 방화벽(§1-4) |
| RTSP 접속은 되는데 `404 Not Found` | 인코더 초기화 실패 — 로그에서 `NVENC init failed` 확인, 거의 항상 드라이버 버전(§0, 570 이상) |
| RTSP 접속 후 `Timeout while waiting for server response` (약 20초) | 09-02 빌드에서의 인코더 실패 증상(위와 같은 원인 — 그 빌드는 드라이버 610 이상 필요). 09-15 이후 빌드에선 대신 404가 나옵니다. 09-15 빌드에서 이 증상이 나오면 알려주세요 |

---

## 변경 이력 (09-02 가이드 대비)

| 날짜 | 변경 |
|---|---|
| 2026-09-15 | **실행 방법 단순화(§2·§3)**: Wayland/X11 세션 자동 판별이 `titan_example.sh` 자체에 들어감. `run_titan_example.sh` / `titan_example_x11_fallback.sh`는 더 이상 패키지에 포함되지 않음(들어 있어도 동작). 명시 지정은 `-sdlvideodriver=x11` / `=wayland` 그대로. |
| 2026-09-15 | **NVIDIA 드라이버 최소 570 명시(§0·§1-2·§1-3·§6)**: 이 빌드부터 영상 인코더(NVENC) 요구 드라이버가 610+ → 570+로 낮아짐(595.84에서 실측 확인). 09-02 빌드에서 RTSP 접속이 20초 타임아웃으로 실패하던 원인이 이것이며, 이 빌드부터는 인코더 실패 시 즉시 404로 표시됨. |
| 2026-09-15 | Vulkan ICD 확인을 `/usr/share/vulkan/icd.d/`·`/etc/vulkan/icd.d/` 두 경로로(`.run` 설치본은 후자). `.run`으로 특정 버전을 맞출 때의 주의사항을 §1-2에 접이식으로 추가. |
| 2026-09-15 | **RTSP 전송 방식 정정(§1-4·§5)**: 09-02 가이드의 "TCP/UDP 둘 다"가 맞음(다른 내부 문서의 "TCP만" 표기가 오류). TCP 권장 이유(저지연 검증 기준, UDP는 영상 포트 동적 협상 → 방화벽)를 명시. |
| 2026-09-04 | (09-02 가이드에 반영됐던 것) Wayland·Xorg 양쪽 실측 검증, Vulkan ICD 누락 증상, X11 전용 머신 즉시 종료 대응(당시는 래퍼 스크립트로). |
