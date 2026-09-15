# 리눅스 테스트 PC NVIDIA 드라이버를 특정 버전(595.84)으로 맞추기 — 공식 `.run` 설치 절차

2026-09-15 / 완료 / 고객(LIG) PC와 동일한 드라이버 595.84로 사내 리눅스 테스트 PC를 내려서 RTSP(NVENC) 동작을 실증한 절차. 검증 명령·원복 방법 포함.

## 목적

**고객 환경 재현용 절차.** LIG PC는 드라이버 **595.84**이고 안정성 사유로 업데이트를 거부했다
(`rtsp/2026-09-15_lig_rtsp_describe_timeout_analysis.md`). 개발 PC는 610.88이라 "595에서
NVENC 인코더가 뜨는가"를 로컬에서 재현할 수 없었고, 배포판 apt 저장소에는 595 패키지가 없어
NVIDIA 공식 `.run` 설치본으로 버전을 고정했다. 앞으로도 고객이 특정 드라이버 버전을 알려오면 이
절차로 같은 버전을 맞춘 뒤 패키지를 검증하고 보내는 것을 권장한다.

- 실증 환경: Ubuntu 22.04, RTX 4070 SUPER, 커널 기본(`uname -r`), 595.84 `.run`.
- 결과: SDK 13.0.37 빌드 리눅스 패키지에서 RTSP 5스트림 영상 정상. 09-02 빌드(SDK 13.1)였다면
  `NVENC init failed … please upgrade driver`로 실패했을 조합.

## 사전 확인

1. **Secure Boot** — `.run`이 빌드하는 커널 모듈은 서명이 없어서 Secure Boot가 켜져 있으면 로드가
   안 된다(설치는 성공한 것처럼 끝나고 재부팅 후 `nvidia-smi`만 실패).
   ```bash
   mokutil --sb-state        # "SecureBoot disabled" 여야 함. enabled면 BIOS에서 끄고 시작
   ```
   (apt 설치본은 DKMS가 MOK 서명을 걸어주지만 `.run`은 그런 절차가 없다. 켜진 채로 진행하려면
   직접 MOK 키를 만들어 서명해야 하는데, 테스트 PC라면 그냥 끄는 게 빠르다.)
2. 설치본 다운로드(공식 아카이브, 버전 숫자만 바꾸면 다른 버전도 동일):
   ```bash
   wget https://download.nvidia.com/XFree86/Linux-x86_64/595.84/NVIDIA-Linux-x86_64-595.84.run
   chmod +x NVIDIA-Linux-x86_64-595.84.run
   ```
3. 커널 모듈 빌드 도구:
   ```bash
   sudo apt install -y build-essential dkms linux-headers-$(uname -r) pkg-config libglvnd-dev
   ```

## 설치 절차

```bash
# 1) 기존 apt 드라이버 완전 제거 — 섞이면 유저스페이스 라이브러리 버전이 안 맞아 Vulkan/NVENC가 깨짐
sudo apt purge -y 'nvidia-*' 'libnvidia-*'
sudo apt autoremove -y

# 2) nouveau 블랙리스트 (없으면 .run 설치기가 거부함)
printf 'blacklist nouveau\noptions nouveau modeset=0\n' | sudo tee /etc/modprobe.d/blacklist-nouveau.conf
sudo update-initramfs -u
sudo reboot
```

재부팅 후 화면이 저해상도로 뜨는 것이 정상(드라이버가 없는 상태). 이어서:

```bash
# 3) 그래픽 세션을 내린다 — .run은 X/Wayland가 떠 있으면 설치를 거부한다
#    Ctrl+Alt+F3 으로 텍스트 콘솔에 로그인한 뒤
sudo systemctl isolate multi-user.target

# 4) 설치 (DKMS 등록 옵션 필수 — 커널 업데이트 시 자동 재빌드)
cd ~/Downloads   # .run을 받은 곳
sudo ./NVIDIA-Linux-x86_64-595.84.run --dkms
```

설치기 질문에 대한 선택(실증 시 사용한 값):

| 질문 | 선택 | 이유 |
|---|---|---|
| 커널 모듈 종류 (proprietary / open) | **proprietary** | LIG 환경과 동일하게. RTX 40계열은 open도 되지만 변수 줄이기 |
| DKMS 등록 | **yes** (`--dkms`로 이미 지정) | 커널 업데이트 후 모듈 자동 재빌드 |
| 32-bit compatibility libraries | **yes** | 무해, 일부 도구가 요구 |
| `nvidia-xconfig` 실행 | **no** | X 설정을 건드리면 Wayland/X11 자동 판별 테스트에 영향 |

```bash
sudo reboot
```

## 검증

```bash
nvidia-smi                                   # 상단 Driver Version: 595.84
vulkaninfo --summary | head -30              # driverName: NVIDIA, deviceName에 GPU 이름
ls /etc/vulkan/icd.d/ /usr/share/vulkan/icd.d/ 2>/dev/null   # .run 설치본은 /etc 쪽에 nvidia_icd.json
ldconfig -p | grep -E 'libnvidia-encode|libcuda\.so'         # 둘 다 잡혀야 함 (패키지가 DT_NEEDED로 링크)
gst-inspect-1.0 nvh264dec | head -5          # 수신 테스트용 NVDEC 플러그인이 드라이버를 잡는지
```

> ⚠️ **`.run` 설치본은 Vulkan ICD를 `/etc/vulkan/icd.d/nvidia_icd.json`에 넣는다.**
> `/usr/share/vulkan/icd.d/`는 비어 있어도 정상이다(apt 설치본만 거기 넣음). 실행 가이드
> `kadex_0902_패키징_실행가이드.md` §1-3은 이 두 경로를 모두 보도록 2026-09-15에 보완했다.
> 최종 판정은 항상 `vulkaninfo --summary`.

이후 시뮬레이터 패키지를 실행해서 로그(`titan_example/Saved/Logs/titan_example.log`)에서
`NVENC Vulkan/CUDA encoder ready` 5줄, 클라이언트 접속 시 `prepared for a client` 직후
`first encoded frame pushed`가 찍히면 드라이버-인코더 조합이 정상이다.

## 커널 업데이트 방지 (권장)

테스트 PC를 "고객 환경 재현" 용도로 유지할 거면 커널이 올라가면서 DKMS 재빌드가 실패하거나
(구 드라이버 + 신 커널 조합) 드라이버가 통째로 풀리는 일을 막기 위해 커널 패키지를 hold한다.

```bash
sudo apt-mark hold linux-image-generic linux-headers-generic linux-generic
sudo apt-mark hold "linux-image-$(uname -r)" "linux-headers-$(uname -r)"
apt-mark showhold
```

`unattended-upgrades`가 켜져 있으면 드라이버 관련 apt 패키지가 다시 깔릴 수 있으니
`apt purge` 상태가 유지되는지(`dpkg -l | grep nvidia`) 가끔 확인.

## 원복 (배포판 권장 드라이버로 돌아가기)

```bash
# 그래픽 세션 내린 상태(Ctrl+Alt+F3 → systemctl isolate multi-user.target)에서
sudo ./NVIDIA-Linux-x86_64-595.84.run --uninstall
sudo rm /etc/modprobe.d/blacklist-nouveau.conf     # apt 드라이버가 자체 블랙리스트를 넣으므로 제거해도 됨
sudo update-initramfs -u
sudo apt-mark unhold linux-image-generic linux-headers-generic linux-generic   # hold 했었다면
sudo apt install -y ubuntu-drivers-common
sudo ubuntu-drivers install
sudo reboot
```

원복 후 `nvidia-smi` / `vulkaninfo --summary`로 다시 확인. ICD는 이번엔
`/usr/share/vulkan/icd.d/`에 들어간다.

## 주의사항 모음

- **Secure Boot 켜진 채 `.run` 설치 금지** — 위 사전 확인 1번. 설치가 끝나도 모듈이 안 올라온다.
- **apt 드라이버와 `.run`을 섞지 말 것** — 반드시 `apt purge 'nvidia-*' 'libnvidia-*'` 후 진행.
  섞이면 `libnvidia-encode.so`/`libcuda.so` 버전이 커널 모듈과 달라져 NVENC 초기화가 이상한
  에러로 실패한다.
- **그래픽 세션에서 실행 불가** — `.run`은 X/Wayland가 살아 있으면 중단한다. 텍스트 콘솔 +
  `systemctl isolate multi-user.target`.
- `--dkms` 없이 설치하면 커널 업데이트 한 번에 드라이버가 사라진다.
- 다른 버전을 맞출 때는 그 버전이 GPU를 지원하는지 먼저 확인(예: RTX 50계열은 570 미만 불가).
  드라이버 버전 ↔ NVENC SDK 요구 관계: SDK 13.0 = 570+, SDK 13.1 = 610+ (현재 플러그인은 13.0.37,
  `rtsp/2026-09-15_lig_rtsp_describe_timeout_analysis.md`).
