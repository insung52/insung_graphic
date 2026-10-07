# RCWS RTSP 특정 방향 끊김 = 비트레이트 기아

2026-10-07 / 완료(빌드·PIE 검증, 미서브밋) / RCWS 1080p 스트림이 CCTV용 기본값 4000 kbps + 실제보다 높은 선언 fps(60)로 인코딩돼 복잡한 방향에서 끊겨 보임 → `RcwsBitrateKbps`(UGV 10000·트럭 6000) 추가 + `RcwsTargetFps=30`으로 해결.

> 관련: `rtsp/2026-09-30_rtsp_async_encode_pipeline_design.md`(10-01 비동기 인코드 — 이번 원인 아님, 2절).

## 1. 증상 (사용자, 2026-10-07)

- 에디터 PIE, kadex 로비 레벨, **UGV 호스트 단독**(클라 없음), 데모 모드 off.
- Python `ugv_controller_demo`(GStreamer `rtspsrc latency=0` + `nvh264dec` + `d3d11videosink sync=true qos=true`,
  `video_panel.py:157-160`)에서 **RCWS 메인뷰가 특정 방향을 볼 때만 fps가 심하게 떨어짐**.
- PIE fps는 그대로, **CCTV 4스트림은 모든 방향에서 일정**.

## 2. 서버 측은 정상 송출 (로그 근거)

`Saved/Logs/titan_example.log`, UTC 10-06 22:45–22:51(= KST 10-07 07:45–07:51):

- `ugv/rcws` push: 8248프레임/≈277 s(**≈29.8 fps**), 1385프레임/≈46.6 s. CCTV ≈29 fps.
- `LogRtspAsyncEncoder` 드롭 통계 줄이 ugv에 대해 **0줄**(드롭>0일 때만 출력) → 비동기 인코더는 드롭 없음.
- 즉 **송출 프레임 수는 일정** — 문제는 프레임 수가 아니라 프레임 내용.
- 10-01 비동기화 무관: P4 `diff2 NvencD3D12Encoder.cpp #1→#2`(CL 23)에서 rcParams/preset/tuning/bitrate 줄 변경 없음.

## 3. 원인 — 프레임당 비트 예산 부족

- `ugv/rcws` 등록값: **`1920x1080 @ 60 fps, 4000 kbps`**.
  - `RcwsTargetFps` 기본 60(`VehicleRtspBridgeComponent.h`), `BitrateKbps` 기본 4000은 `URtspStreamComponent`
    기본값(원래 CCTV용). 게임은 실제 ≈30 fps라 **선언 60 / 실제 push 30** 불일치도 있음.
- 인코더 설정(`NvencD3D12Encoder.cpp:89-105`): preset P3, tuning ULTRA_LOW_LATENCY,
  **CBR averageBitRate = maxBitRate = BitrateKbps**, frameRateNum = Fps.
  → 프레임당 예산 ≈ 4000k/60 ≈ **8 KB @ 1080p**.
- 실측: 새 접속 시 RCWS 1080p IDR이 **15–18 KB**(P프레임 3.8 KB)인데, CCTV 320x180 IDR은 **30–32 KB**.
- 메커니즘: 비트가 모자라면 복잡한 장면(수풀 등)에서 대부분 매크로블록이 **skip(이전 프레임 복사)** →
  스트림은 30 fps로 오지만 **그림이 거의 안 바뀌어 저fps처럼 보임**. 단순한 방향은 잘 압축돼 멀쩡.
- CCTV는 320x180에 4000 kbps/30 fps ≈ 16 KB/프레임 — 픽셀 수 1/36이라 기아 없음.

## 4. 사용자 테스트

- `RcwsTargetFps=30`(프레임당 예산 2배 + 선언/실제 불일치 제거) → **눈에 띄게 개선, 약한 끊김 잔존**.
- 방향 의존성이 있다는 점이 fps 불일치보다 **비트레이트 가설**을 지지.

## 5. 수정 (코드, 2026-10-07 — 빌드는 사용자)

- `UVehicleRtspBridgeComponent`(`VehicleRtspBridgeComponent.h`, `RcwsTargetFps` 옆):
  `UPROPERTY(EditAnywhere) int32 RcwsBitrateKbps = 10000;`
- `VehicleRtspBridgeComponent.cpp`: 메인뷰 RCWS 경로와 씬캡쳐 폴백 경로 **둘 다** `Stream->BitrateKbps` 설정
  (`MakeCaptureStream`에 선택 인자 `InBitrateKbps` 추가). **≤0이면 기존 4000 기본값**.
- 파일은 P4 체크아웃 상태, **미서브밋**.
- ⚠ 새 UPROPERTY → **에디터 닫고 정식 빌드**(Live Coding 금지 — BP missing property 함정).

## 6. 지연·대역폭 영향

- **지연 증가 없음**: NVENC 버퍼 깊이는 프레임 단위이고 비동기 워커가 submit 직후 Lock(출력 지연 0) —
  커지는 건 프레임 크기뿐. 추가 전송 시간도 미미(≈40 KB 프레임 @ 1 Gbps ≈ 0.3 ms, localhost는 무시 가능).
- **비용은 대역폭**: RCWS 스트림당 10 Mbps — LIG 현장 네트워크/Tailscale 링크 여유 확인 필요.
- IDR이 커져 **2초마다 버스트**(gopLength = Fps×2).

## 7. 빌드 후 검증 (2026-10-07, 사용자 빌드 + PIE)

**1차 — `BP_UGV_0901`의 `RcwsTargetFps=60` + 10000 kbps: 개선됐지만 특정 방향에서 약한 끊김 잔존.**
로그(UTC 10-06 23:24–23:26, PIE 레벨 New_kadex_0811):

- `ugv/rcws: streaming 1920x1080 @ 60 fps, 10000 kbps` — 설정 적용 확인. 새 접속 IDR **18 KB → 46 KB**.
- 실제 게임 fps **≈35**(로그 프레임 카운터 3구간: 34.4 / 35.6 / 34.7). `t.MaxFPS=60` 상한을 못 채움.
  CCTV는 96 s에 2864프레임(29.7 fps)으로 정상. RCWS 워커 드롭 10 s당 1건(무시 가능).
- 60이 끊김을 키운 이유:
  - PTS는 push마다 `1/Fps`씩 올리는 카운터(`RtspServerSubsystem.cpp` `PushEncodedFrame`) → 실제 35장/s인데
    PTS는 16.7 ms 간격 = 실시간의 ≈0.58배. 데모 클라는 `latency=0` + `sync=true qos=true`라 늦은 프레임 폐기·재동기.
  - NVENC가 60 fps 기준으로 비트를 나눠 프레임당 예산이 30 fps 대비 절반(≈21 KB).

**2차 — `RcwsTargetFps=30` + 10000 kbps: 사용자 확인 "잘 됨". → 해결.**

- 운용값: **`BP_UGV_0901`의 `RcwsTargetFps=30`**(BP 인스턴스 값 — C++ 기본값은 아직 60).
- 잠재 요인(현재는 체감 안 됨): 게임 35 fps → 스트림 30 fps면 ≈6틱 중 1틱을 건너뛰어 캡처 간격이 28.6/57 ms로
  불규칙한데 PTS는 33.3 ms 균일 → 디테일 많은 화면에서 미세한 울컥임이 생길 수 있다. 다시 보이면
  `t.MaxFPS 30`으로 판별 → 근본 해결은 게임 fps 고정 또는 PTS를 실제 캡처 시각 기반으로 바꾸는 것.

## 8. 남은 것

- [x] 빌드 후 로그 확인(`10000 kbps`) — 위 7절.
- [x] 문제 방향 재시험 — 60 fps는 약한 끊김, 30 fps 해결.
- [ ] P4 서브밋: `VehicleRtspBridgeComponent.h/.cpp`, `TitanTruck.h/.cpp`.
- [ ] 선택: C++ 기본값 `RcwsTargetFps`도 30으로(지금은 BP 값에만 의존 — 다른 UGV BP나 새 배치는 60으로 뜸).
- [ ] 자체방호 트럭 RCWS 6000 kbps 실기 확인(로그 `'selfdefense/rcws': streaming 1280x720 @ 30 fps, 6000 kbps.`).
- [x] **자체방호 트럭 RCWS**(`selfdefense/rcws`, 1280x720 @ 30fps) — 같은 날 `ATitanTruck::RcwsBitrateKbps`(기본 **6000** = UGV 1080p 10000과 같은 픽셀당 예산) 추가, 메인뷰 경로 + 씬 캡쳐 폴백 둘 다 적용(`TitanTruck.h/.cpp`, P4 체크아웃·미제출). 빌드 후 로그 `'selfdefense/rcws': streaming 1280x720 @ 30 fps, 6000 kbps.` 확인.
- [ ] 드론/UAV 짐벌 스트림도 기본값 사용 중 — 해상도 대비 점검.
- 규칙: **`RcwsTargetFps`는 실제 게임 fps 이하로**(선언 fps > 실제 송출이면 PTS가 늦어져 클라가 버린다).
