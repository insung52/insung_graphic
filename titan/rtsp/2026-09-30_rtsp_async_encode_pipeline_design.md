# RTSP 인코드 비동기 파이프라인 — 현황 분석 + 새 구조 설계

2026-09-30 / 설계단계 / 렌더 스레드는 GPU 복사만 기록하고 NVENC 일(대기·CUDA 복사·인코드·Lock·push)은 스트림별 워커 스레드로 옮긴다 — 리눅스(전시 PC) 우선. 렌더 스레드 동기 대기 제거 + 리눅스 서버 지연 ≈100 ms 감소가 목표.

> 선행: `camera_pipeline/2026-09-29_s1_capture_rtsp_cost_analysis.md`(S1, 2절). 이 문서는 그 F2 후보의 설계.
> **[A]** = 프로젝트/엔진 소스 file:line 확인. **[B]** = 추정 — 검증 방법 병기. 코드 변경 없음(설계만).
> 경로 약어: `RE/` = `Plugins/RtspEncoder/Source/RtspEncoder/`, 엔진 = `UE_5.8/Engine/Source/Runtime/`.

---

## 0. 요약

| | 지금 | 새 구조 |
|---|---|---|
| 렌더 스레드(RT)가 하는 일 | 복사 기록 + **GPU 대기** + CUDA 복사 + 인코드 + Lock + push | **복사 기록 + 펜스 기록만** |
| 리눅스 RT 블록 | 스트림마다 `ImmediateFlush` + `CopyFence->Wait()` = **GPU 큐 앞쪽 전부가 끝날 때까지** | 없음 |
| Windows RT 블록 | `nvEncLockBitstream(doNotWait=false)`로 N-1 프레임 인코드 완료 대기 | 없음 |
| 리눅스 서버 지연 | 출력 지연 **3 인코드 호출**(30 fps면 ≈100 ms) — D3D12만 1로 고쳤고 리눅스는 미이식 | GPU 복사 완료 + 인코드 시간(수 ms) |
| 스레드 | RT 하나 | 스트림당 워커 1개(UGV 5 / 자체방호 7) |

---

## 1. 현황 분석

### 1.1 리눅스(Vulkan+CUDA) 경로 — 스트림 1개 인코드당 [A]

`RtspStreamComponent.cpp:216` 렌더 커맨드 → `FNvencVulkanEncoder::EncodeFrame`(`RE/Private/FNvencVulkanEncoder.cpp:521`)

| 단계 | 코드 | 성격 |
|---|---|---|
| 1. 소스 → ExportableImage 복사 + 펜스 기록 | `:449-450` Transition, `:490` CopyTexture, `:491` WriteGPUFence | 기록만 |
| 2. **강제 제출** | `:507` `ImmediateFlush(DispatchToRHIThread)` | 프레임 중간에 커맨드 버퍼를 닫고 제출 — 스트림마다 |
| 3. **CPU가 GPU 대기** | `:516` `CopyFence->Wait()` → `FVulkanDynamicRHI::ProcessInterruptQueueUntil`(엔진 `VulkanRHI/Private/VulkanUtil.cpp:313-320`) | GPU 큐는 순서대로 실행 → **이 복사 앞에 쌓인 GPU 일 전부**(이전 프레임 나머지 + 이번 프레임 캡쳐)가 끝나야 풀림 |
| 4. CUDA 복사 | `:544-553` `cuMemcpy2D`(NULL 스트림 = 동기) | 동기 |
| 5. 인코드 + Lock | `NvEncoderCuda` → `NvEncoder::EncodeFrame`(`ThirdParty/NvCodec/NvEncoder/NvEncoder.cpp:488-505`) → `GetEncodedPacket(bOutputDelay=true)` | 비Windows는 동기 모드(`NvEncoder.cpp:140-145`), Lock 블로킹 |

- 3번이 핵심: RT가 GPU를 기다리는 동안 다음 일을 준비하지 못하고, 기다림이 끝난 순간 GPU는 큐가 비어 잠깐 논다 → **CPU/GPU 병렬성이 스트림 수만큼 끊긴다.** 헤더 주석 스스로 "Still a CPU stall"이라고 적어 둠(`RE/Public/FNvencVulkanEncoder.h:45-65`).
- 2번 이유: 기록만 된 펜스를 기다리면 영원히 안 풀려서(08-19 데드락) 넣은 것(`:493-506` 주석). 즉 **2번은 3번(CPU 대기) 때문에 존재**한다.
- 리눅스 실측은 없음 — 과거 `SubmitAndBlockUntilGPUIdle` 시절 ≈50 ms 기록만 있다(헤더 `:45-51`). 현재 비용 [B].

### 1.2 리눅스 출력 지연 3프레임 — 미이식 발견 [A]

- `NvEncoderCuda`를 `nExtraOutputDelay` 인자 없이 생성(`FNvencVulkanEncoder.cpp:324`) → 기본값 3(`ThirdParty/.../Cuda/NvEncoderCuda.h:64`).
- `m_nEncoderBuffer = frameIntervalP(1) + lookahead(0) + 3 = 4`(`NvEncoder.cpp:346`), `m_nOutputDelay = 4 - 1 = 3`(`:358`) → 프레임 N의 결과는 **N+3번째 인코드 호출 때** 나온다. CCTV·자체방호 RCWS(30 fps) ≈100 ms, UGV RCWS(60 fps) ≈50 ms.
- 08-19 지연 조사에서 D3D12는 `nExtraOutputDelay=1`로 고쳤다(`RE/Private/Windows/NvencD3D12Encoder.cpp:70`, `rtsp/rtsp_latency_investigation.md` §3.15, 112 → 24~28 ms). **리눅스 인코더는 그 수정이 안 들어갔다.** 코드 주석은 "Same encoder config as ... NvencD3D12Encoder ... nExtraOutputDelay left at the SDK default"(`FNvencVulkanEncoder.cpp:320-322`)라서 D3D12 쪽 변경을 못 따라간 흔적.

### 1.3 Windows(D3D12) 경로 [A] (S1 문서 2.1절 요약)

- 복사 + `RHISignalManualFence`는 기록만(`NvencD3D12Encoder.cpp:238-262`), NVENC가 입력 펜스를 **GPU 측**에서 대기.
- RT 블록은 출력 쪽: `NvEncoderD3D12.cpp:409-410` `nvEncLockBitstream(doNotWait=false)` — 직전(N-1) 프레임 완료 대기. GPU 바운드면 "GPU 따라잡기 + 인코드" 시간(S0 M1 12 ms / 4~5회, max 18.6).

### 1.4 과거 데드락의 정확한 원인 — 새 구조의 전제 [A]

- `nExtraOutputDelay=0` 시도가 데드락난 이유(`NvencD3D12Encoder.cpp:53-69` 주석, `rtsp_latency_investigation.md:559-561`): **RT가 Lock으로 "아직 GPU 큐에 제출도 안 된" 이번 프레임의 완료를 기다림** → 제출은 RT가 해야 하는데 RT가 멈춰 있음.
- 리눅스 08-19 데드락도 같은 형태(기록만 된 펜스를 RT가 대기).
- 즉 위험의 본질은 "**제출 주체(RT)가 스스로 완료를 기다린다**"는 것이지 버퍼 깊이 자체가 아니다. **대기를 RT 밖(워커)으로 옮기고, 워커는 GPU 완료가 확인된 뒤에만 NVENC를 부르면** 이 경로는 구조적으로 생길 수 없다.

### 1.5 새 구조에 쓸 수 있는 엔진 기능 [A]

- `FVulkanGPUFence::Poll()`(엔진 `VulkanRHI/Private/VulkanUtil.cpp:298-311`): 블록 없음. `SubmittedSyncPoint->IsComplete()`(제출됐는가) + `vkGetEventStatus(Event)`(GPU가 지나갔는가)만 본다 → **강제 제출도, CPU 대기도 없이 "끝났는지"만 물을 수 있다.**
- 08-18/19에 기각된 "외부 세마포어" 방식(`FNvencVulkanEncoder.h:55-65`: 같은 큐 제출 순서가 스펙상 보장 안 됨)은 **쓰지 않는다** — 이 설계는 UE 자신의 펜스(이미 검증된 `WriteGPUFence`)를 그대로 쓰고, 기다리는 방식만 "블록"에서 "폴링"으로 바꾼다.
- `PushEncodedFrame`은 `Handle.State`(TSharedPtr)와 `State.AppSrcLock`만 쓴다(`RE/Private/RtspServerSubsystem.cpp:443-500`) → 서브시스템 인스턴스 없이 워커에서 호출 가능하게 만들 수 있다.

---

## 2. 새 구조

### 2.1 역할 분리

```
GT  TickComponent ──(TargetFps 게이트, 기존 그대로)──► ENQUEUE_RENDER_COMMAND
RT  ① 빈 슬롯 확보(없으면 이 프레임 드롭)
    ② (스케일 필요 시) AddDrawTexturePass — 기존 그대로
    ③ Transition + CopyTexture(소스 → 슬롯 k) + 펜스 k 기록   ← 기록만, 제출 강제 없음
    ④ 슬롯 k를 워커 큐에 넣음 (키프레임 요청·타임스탬프 동봉) → 즉시 리턴
         (RT가 평소대로 프레임 끝에 제출)
워커 ⑤ 펜스 k Poll()이 true가 될 때까지 대기(짧은 sleep 폴링)
    ⑥ [Vulkan] cuMemcpy2DAsync(슬롯 k CUarray → NVENC 입력) + 스트림 동기
       [D3D12] (⑤ 없이) nvEncEncodePicture에 입력 펜스 값 전달 — NVENC가 GPU에서 대기
    ⑦ nvEncEncodePicture → nvEncLockBitstream(블로킹 — 워커라 무해) → Unlock
    ⑧ 키프레임 폐기 로직(bWaitingForRequestedKeyframe) → PushEncodedFrame
    ⑨ 슬롯 k 반납(Free)
```

### 2.2 슬롯 링(스트림당 3개)

- 슬롯 = 복사 대상 텍스처 + 완료 펜스 + 상태(`Free → Queued → Encoding → Free`, 원자 변수).
  - Vulkan: 슬롯마다 ExportableImage + CUDA 외부 메모리 import + `FGPUFenceRHIRef` — 지금 1개인 것을 3개로(`CreateExportableImageAndImportToCuda` 반복).
  - D3D12: 슬롯마다 NVENC 등록 입력 버퍼 + 펜스 값.
- 슬롯이 전부 차 있으면 RT는 **기다리지 않고 그 프레임을 드롭**(카운터 + 1회 로그). 원칙: 스트림이 늦어질지언정 게임 프레임은 절대 안 막는다.
- 메모리: 1080p BGRA 8 MB × 3 = 24 MB(UGV RCWS), CCTV 320x180은 무시할 수준.

### 2.3 NVENC 세션 소유권

- NVENC 세션·CUDA 컨텍스트는 **워커 스레드 하나만** 만진다(초기화는 GT, 이후 호출 전부 워커). 샘플 클래스의 `m_iToSend/m_iGot/GetNextInputFrame`은 단일 스레드 전제라 RT와 공유 금지.
- 워커는 한 번에 한 프레임만 처리하므로 **출력 지연 0**(제출 직후 Lock). 샘플 클래스를 계속 쓴다면 `nExtraOutputDelay=0`으로 생성하거나, 초기화·등록만 샘플을 쓰고 인코드/Lock은 API 테이블을 직접 호출하는 얇은 래퍼로.
  - `NvEncoder.h:553`의 "그래픽과 인코드 병렬엔 ≥1 필요"는 **같은 스레드에서 파이프라이닝할 때** 얘기 — 워커 분리 구조엔 해당 없음 [B, 구현 후 실기 확인]. 문제 시 1로 두고 워커 안에서 한 칸 늦게 꺼내도 RT 영향은 없다.
- 스레드 수: 스트림당 `FRunnable` 1개. 공용 1개로 하면 한 스트림의 블로킹 Lock이 다른 스트림을 막는다. 태스크 그래프 워커에서 블로킹 대기 금지(엔진 워커 고갈).

### 2.4 무엇이 바뀌고 무엇이 그대로인가

| 항목 | 변경 |
|---|---|
| GT 틱·TargetFps 게이트·축 게이트·해상도 고정 | 그대로 |
| 스케일 복사(`AddDrawTexturePass`) | 그대로(RT) |
| 리눅스 `ImmediateFlush` + `CopyFence->Wait()` | **삭제**(폴링으로 대체) |
| 리눅스 `cuMemcpy2D` 동기 | 워커의 `cuMemcpy2DAsync` + 전용 CUstream |
| D3D12 Lock | 워커로 이동 |
| `PendingSubmitWallClockSeconds` FIFO 큐 + 새 연결 시 비우기(`RtspStreamComponent.cpp:264-268`) | **불필요** — 슬롯이 자기 타임스탬프를 들고 다님(출력 지연 0이라 FIFO 매칭 자체가 없음) |
| 새 연결 IDR(`ConsumeNewConnectionFlag` → `bForceKeyframe`) | 슬롯에 실어 워커로 — 동작 동일 |
| `PushEncodedFrame` | `Handle.State`만 쓰는 정적 함수로(워커가 서브시스템 raw 포인터를 들지 않게) |
| `IRtspFrameEncoder::EncodeFrame` 시그니처 | `OutAccessUnits` 반환이 사라짐 → `SubmitFrame(RHICmdList, Source, bForceKeyframe, Pts, Handle)` 형태로 |

### 2.5 수명·종료

- `EndPlay`: 지금처럼 `FlushRenderingCommands()`(`RtspStreamComponent.cpp:134`) 뒤 `Encoder->Shutdown()`.
- `Shutdown` 순서: ① 워커에 정지 신호 → ② 큐에 남은 슬롯 처리 또는 폐기 → ③ join → ④ **RT에서** Queued 상태 슬롯의 펜스를 `Wait()`(종료 시 1회라 블록 허용) → ⑤ ExportableImage/CUDA import/NVENC 세션 파괴. ④를 빼면 GPU가 아직 쓰는 VkImage를 지우게 된다(지금은 매 프레임 대기라 이 문제가 없었음).
- 레벨 전환·시나리오 재시작에서 스트림 재생성 경로 재검증 필요(`rtsp/2026-09-16_rtsp_axis_gate_dangling_timer_fix.md`의 타이머 수명 이슈와 같은 류).

### 2.6 폴링을 워커가 하나, RT가 하나

| 안 | 방식 | 지연 | 검증 필요 |
|---|---|---|---|
| **A(권장)** | 워커가 `Poll()`을 0.5 ms sleep 간격으로 | 최소(GPU 완료 + ≤0.5 ms) | `FVulkanGPUFence::Poll()`을 RT 외 스레드에서 불러도 되는가 [B] — 구현이 `const`, `FGraphEvent::IsComplete` + `vkGetEventStatus`(Vulkan 스펙상 외부 동기화 불필요)만 쓰므로 가능성 높음. 단 `Clear()`(`vkResetEvent`)는 슬롯이 Free일 때 RT만 호출해야 함 |
| B(폴백) | RT가 매 틱 가벼운 렌더 커맨드로 `Poll()`, 끝난 슬롯만 워커로 넘김 | +최대 1프레임(≈16~25 ms) | 없음(RT 전용 호출) |

B여도 현재 리눅스(출력 지연 ≈100 ms + RT 블록)보다 양쪽 다 낫다.

---

## 3. 예상 효과

| 항목 | 예상 | 근거 |
|---|---|---|
| RT 인코드 대기 | Windows M1 12 ms(max 18.6) → ≈0, 리눅스는 더 큼(스트림마다 GPU 큐 전체 대기) | [A] 구조 / 리눅스 수치 [B] |
| 프레임 시간(Windows M1) | 23.9 → ≈22 ms(GPU 21.8이 새 상한) | [B] S1 대화 추정 |
| 프레임 시간(리눅스) | Windows보다 개선 폭이 클 가능성 — 스트림마다 CPU/GPU 직렬화가 끊겨 있으므로 | [B] 리눅스 계측 필요 |
| 리눅스 서버 지연 | 출력 지연 3호출(≈100 ms @30fps) 제거 | [A] 1.2절 |
| 교전(GT 바운드) 시점 | 효과 작음 | S0 M2 인코드 2.2 ms |

---

## 4. 단계

| 단계 | 내용 | 위험 |
|---|---|---|
| **P0**(선택, 1줄) | 리눅스 `NvEncoderCuda(..., nExtraOutputDelay=1)` — D3D12와 맞춤. 현재 리눅스는 인코드 전에 CPU가 복사 완료를 이미 기다리므로 1.4절 데드락 조건(미제출 작업 대기)이 성립하지 않는다 | 낮음. P1이 들어가면 대체됨. 지연 ≈67 ms 감소 효과만 먼저 |
| **P1** | 리눅스 Vulkan 비동기 파이프라인(2절 전체) | 스레딩·수명. 리눅스 실기 검증 필수(`-vulkandebug` 검증 레이어 포함 — 08-18 사고 이력) |
| **P2** | D3D12를 같은 구조로(개발 PC 체감·Windows 측정 일치) | P1 추상화 재사용 |
| P3 | CCTV 중복 인코드(내용 ≈10 fps, 송출 30 fps) 재검토 — 슬롯 구조에선 "새 캡쳐가 없으면 복사 생략"이 쉬워짐 | SDP fps 미달 시 클라이언트 멈춤 이력(`StreamResolutionSubsystem.h:51-69`) |

## 5. 검증 계획

- 리눅스 전시 PC(또는 사내 리눅스 PC, 드라이버 595.84 맞춤 완료)에서 **적용 전/후 같은 시점 1회씩**: `stat dumpave -num=120 -ms=0.1`(`RtspStreamEncodeFrame`·`Frame Sync Time`), `stat unit`.
- 기능: VLC로 5스트림(UGV) / 7스트림(자체방호) 동시 수신, 재연결 반복 시 IDR 즉시 복귀, 검은/깨진 프레임 없음(`-vulkandebug` 로그 0건), 시나리오 재시작 후 스트림 복구.
- 지연: 기존 `capture_to_push_ms` 계측(`rtsp_latency_investigation.md` §3.14 방식) 재사용.

## 6. 미결 질문

1. 안 A/B 중 어느 쪽으로 시작할지(권장 A, 막히면 B).
2. 드롭 정책(슬롯 부족 시 드롭) 수용 여부 — 대안은 "가장 오래된 대기 슬롯 덮어쓰기"(최신성 우선).
3. P0를 P1과 별개로 먼저 넣을지.
