# LIG 머신 RTSP 접속 실패(DESCRIBE 타임아웃) 원인 분석

2026-09-15 / 완료(코드 수정·빌드·리눅스 595.84 실증 완료, LIG 재발송만 남음) / 스크린샷 대조 결과 IP 문제 아님 — 서버가 DESCRIBE를 받고도 20초간 인코딩 프레임이 0장이라 응답 못 함. 원인 확정: LIG PC 드라이버 595.84 vs NVENC SDK 13.1(610+ 필요). SDK 13.0.37로 내려서 대응, 사내 리눅스 PC를 595.84로 내려 RTSP 정상 동작 확인.

## 배경

- 2026-09-02 전달 패키지(`D:\kadex_0902_ugv_test_3`, 실행 가이드 `kadex_0902_패키징_실행가이드.md`)를 LIG(`lig@mumt-server`, Ubuntu noble/GStreamer 1.24.2)에서 실행 → `gst-launch-1.0 rtspsrc location=rtsp://192.168.10.15:8554/ugv/rcws … nvh264dec` 접속 실패.
- 2026-09-08 그래피카 김준영 팀장이 "IP를 실제 PC IP로 넣으면 된다"고 회신했으나, 이후 LIG에서 여전히 실패 보고(로그 비슷하다고만 함, 실제 로그 미수령).
- RTSP 관련 코드는 그 패키징 이후 변경 없음 → 현재 `titan_example` 코드로 분석.

## 스크린샷 시간순 대조 (LIG_.jpg, 2026-09-08)

| 시각(UTC) | 클라이언트(gst-launch, 왼쪽 터미널) | 서버(titan_example.log, 오른쪽 터미널) |
|---|---|---|
| (첫 시도) | `Could not open resource … Failed to connect. (Generic error)` | — (이것만 IP/포트 문제) |
| 05:55:58 | `Retrieving server options`(OPTIONS) → `Retrieving media info`(DESCRIBE) | `RTSP stream 'ugv/rcws': rate control disabled …` / `prepared for a client (media=…, appsrc=…)` |
| 20초간 | 대기 | `first encoded frame pushed` 없음, `appsrc push … returned` 없음 (조이스틱 로그는 찍힘 → 게임 스레드 정상) |
| 05:56:18 | `Could not receive message. (Timeout while waiting for server response)` | `'ugv/rcws' unprepared (was the active appsrc, cleared)` |

- `prepared for a client`는 `RtspServerSubsystem.cpp:199`의 `media-configure` 콜백 로그 = **DESCRIBE가 서버에 도달해야만** 찍힘. 두 번째 시도부터는 TCP/OPTIONS/DESCRIBE 전부 도달했음 → IP 문제 아님.
- gst-rtsp-server는 라이브(appsrc is-live) 파이프라인의 SDP를 완성하려면 첫 버퍼가 payloader까지 흘러야 함 → **appsrc에 프레임이 한 장도 안 들어오면 DESCRIBE 응답이 영원히 안 나감**. `rtsp_poc_findings.md` §1.14에서 실측된 현상과 동일. (`RegisterStream`의 "caps 미리 줘서 첫 버퍼 안 기다림" 주석은 실제와 다름.)
- rtspsrc 기본 tcp-timeout 20초 = `prepared`→`unprepared` 간격 20초. 정상이면 `prepared`→`first encoded frame pushed`가 ms 단위(§1.16).

→ **LIG 머신에서는 인코딩 프레임이 0장**. 네트워크가 아니라 프로세스 내부(인코더) 문제로 확정.

## 최유력 원인: NVIDIA 드라이버 버전 (NVENC SDK 13.1 → 드라이버 610 이상)

- vendoring 헤더 `Plugins/RtspEncoder/Source/RtspEncoder/ThirdParty/NvCodec/Interface/nvEncodeAPI.h:115-116` = **13.1** (SDK `C:\SDK\Video_Codec_SDK_13.1.15`, `RtspEncoder.Build.cs:109`).
- `NvEncoder::LoadNvEncApi()`(`NvEncoder.cpp:84-93`): 드라이버의 `NvEncodeAPIGetMaxSupportedVersion` < 13.1이면 throw → `FNvencVulkanEncoder::Initialize`가 `NVENC init failed: … Current Driver Version does not support this NvEncodeAPI version, please upgrade driver` 로그 후 실패.
- NVIDIA 공식: SDK 13.1 = Linux/Windows 드라이버 **610.00 이상**, SDK 13.0 = 570 이상.
  - https://forums.developer.nvidia.com/t/system-requirements-for-video-codec-sdk-v13-1/371379
  - https://forums.developer.nvidia.com/t/system-requirements-for-video-codec-sdk-13-0-37/363095
- 개발 PC(RTX 5060)는 610.88 → 여기선 당연히 됨. Ubuntu 24.04에서 가이드대로 `ubuntu-drivers install` 하면 보통 535/550/570 → 13.1 체크 탈락.
- **전달 가이드에 드라이버 최소 버전이 없고, 예시로 `nvidia-driver-550`을 써놓음** (`kadex_0902_패키징_실행가이드.md` §1-3). 가이드 결함.
- 부수: `cuCtxCreate`가 `cuda.h 13.3`의 `cuCtxCreate_v4`(`FNvencVulkanEncoder.cpp:303`) → CUDA 12.5 미만 드라이버(≈555 미만)면 그 전에 심볼 문제 가능.

### 그 외 후보(우선순위 낮음, 로그 받으면 바로 갈림)

- GeForce NVENC 동시 세션 제한(구 드라이버 3개): `VehicleRtspBridgeComponent::WireRtspStreams`가 CCTV 4개 먼저, rcws가 **5번째** 세션 → rcws만 실패 가능.
- 데이터센터 GPU(A100/H100은 NVENC 없음, NVDEC만 있음 — nvh264dec은 됨).
- Vulkan external-memory 확장 미활성(`LoadVulkanFunctionPointers` 에러).
- 전부 `LogRtspEncoderNvencVk` / `LogRtspFrameEncoderFactory` Error로 남음.

## 우리 코드 결함 (원인은 아니지만 진단을 방해)

`URtspStreamComponent::SetupEncoderAndStream()`(`RtspStreamComponent.cpp:89-104`)이 **마운트 등록 → 인코더 생성** 순서라, 인코더 실패 시 마운트가 남아 클라이언트가 404 대신 **20초 타임아웃**을 받음 → 네트워크 문제와 구분 불가(이번 IP 오인의 직접 원인). 인코더 실패 시 `UnregisterStream` 또는 순서 교체 필요. → 아래 "대응" 2번으로 순서 교체 완료(인코더 먼저, 성공 시에만 `RegisterStream`).

## LIG 회신 (2026-09-15)

드라이버 **595.84**, 안정성 사유로 업데이트 불가(방산). 595는 NVENC API 13.0까지만 지원 → 13.1
헤더의 `LoadNvEncApi` 체크에서 확정적으로 throw. 로그 안 받아도 원인 확정.

## 대응 (2026-09-15 코드 수정 완료)

1. **NVENC SDK 13.1.15 → 13.0.37** (`Plugins/RtspEncoder/Source/RtspEncoder/ThirdParty/NvCodec/` 9개
   파일을 `C:\SDK\Video_Codec_SDK_13.0.37` 원본으로 교체 후 UE 통합 패치 재적용 —
   `#if RTSPENCODER_HAS_NVENC` 게이팅, include 경로, clang `-Werror=reorder` 생성자 순서,
   D3D12 헤더 `<windows.h>`/`<wrl/client.h>`, Logger/nvEncodeAPI 주석. 13.1 때 있던
   `m_iToSend.load()` 패치는 13.0이 `std::atomic`을 안 써서 불필요). 우리가 호출하는 API는 전부
   13.0에 같은 시그니처로 존재 → `FNvencVulkanEncoder.cpp`/`NvencD3D12Encoder.cpp` 무수정.
   `RtspEncoder.Build.cs` 기본 SDK 경로 13.0.37로. 13.0 = 드라이버 570+.
2. **`RtspStreamComponent::SetupEncoderAndStream` 순서 교체**: 인코더 먼저 → 성공 시에만 마운트
   등록. 인코더 실패 시 클라이언트는 20초 타임아웃 대신 즉시 404.
3. 가이드(`packaging/kadex_0902_패키징_실행가이드.md`) §0/§1-2/§1-3/§6에 드라이버 ≥570 요구·확인
   절차·증상표 추가, `nvidia-driver-550` 예시 → 570.

## 검증 결과 (2026-09-15, 완료)

- Windows 에디터 빌드 통과, 개발 PC(610.88) PIE에서 RTSP 회귀 없음.
- **LIG와 동일한 드라이버 595.84로 리눅스 실증**: 사내 리눅스 테스트 PC(Ubuntu 22.04, RTX 4070
  SUPER)의 드라이버를 NVIDIA 공식 `.run`(`NVIDIA-Linux-x86_64-595.84.run`, proprietary 커널 모듈,
  DKMS 등록, 32-bit compat yes, nvidia-xconfig no)으로 595.84까지 내린 뒤 새 빌드 리눅스 패키지
  실행 → **RTSP 영상 완전 정상 동작 확인**. 드라이버를 내리는 절차·검증·원복 방법은
  `packaging/2026-09-15_linux_nvidia_driver_595_run_install.md`.
- 부수 확인: `.run` 설치본은 Vulkan ICD를 `/usr/share/vulkan/icd.d/`가 아니라
  **`/etc/vulkan/icd.d/nvidia_icd.json`**에 넣는다(`vulkaninfo --summary`에 NVIDIA 잡힘). 실행
  가이드 §1-3의 ICD 확인 절차를 두 경로 다 보도록 보완함.
- 로그 확인 항목(정상 시): `NVENC Vulkan/CUDA encoder ready` 5줄, `prepared for a client` 직후
  `first encoded frame pushed`. 09-02 빌드였다면 595.84에서 `NVENC init failed: … Current Driver
  Version does not support this NvEncodeAPI version`가 났을 자리.
- rcws가 5번째 NVENC 세션인 문제(구 드라이버 GeForce 3세션 제한)는 595에서 해당 없음(8세션)으로
  실증됨 — 5스트림 전부 정상.

## 남은 일

- [ ] LIG에 새 리눅스 패키지 재발송 + 메일. 초안
      `C:\Users\user\Desktop\2026-09-15_LIG_RTSP_확인요청_메일초안.txt`는 원인 미확정 시점에 쓴
      것이라 **"드라이버를 업데이트해 달라"는 문구가 들어 있음 → 빼고 "595.84 대응 빌드 재전달"로
      고쳐서 보낼 것**(드라이버 업데이트는 LIG가 거부한 사안). 갱신된 실행 가이드
      (`packaging/kadex_0902_패키징_실행가이드.md`, 드라이버 ≥570 명시판)를 같이 첨부.
