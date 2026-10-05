# RTSP 인코드 비동기 파이프라인 — 현황 분석 + 새 구조 설계

2026-09-30 / 진행중(2026-10-01 리눅스·Windows 동시 구현 완료, 빌드·실기 검증 대기 — 7절) / 렌더 스레드는 GPU 복사만 기록하고 NVENC 일(대기·CUDA 복사·인코드·Lock·push)은 스트림별 워커 스레드로 옮긴다 — 리눅스(전시 PC) 우선. 렌더 스레드 동기 대기 제거 + 리눅스 서버 지연 ≈100 ms 감소가 목표.

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
워커 ⑤ 슬롯 k CAS 점유 → 펜스 k Poll()이 true가 될 때까지 50 µs sleep 폴링(2.6절)
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
- **최신 화면 우선(2026-10-01 사용자 결정)**. 원칙: 스트림은 프레임을 건너뛸지언정 게임 프레임을 절대 막지 않고, 건너뛸 땐 항상 오래된 쪽을 버린다.
  - 슬롯 상태: `Free → Queued(RT가 복사 기록) → Claimed(워커가 CAS로 점유, 이때부터 Poll) → Free`.
  - **워커**: 큐에서 꺼낼 때 뒤에 더 새로운 Queued 슬롯이 있으면 오래된 것들을 Free로 돌리고 가장 새 것만 처리.
  - **RT**: Free 슬롯이 없으면 아직 워커가 점유 안 한 **가장 오래된 Queued 슬롯을 CAS로 회수**해 덮어쓴다(Claimed는 절대 안 건드림). 회수한 슬롯의 GPU 복사가 아직 끝나지 않았어도 무해 — 같은 큐에 나중에 기록된 복사가 뒤에 실행되고, CUDA는 Claimed 전엔 그 메모리를 안 읽는다. 펜스는 `Clear()` 후 다시 기록(이전 기록의 RHI 람다는 자기 싱크포인트 사본을 들고 있어 무해, `VulkanUtil.cpp:343-357`).
  - **키프레임 요청 승계**: 버려지는/덮어쓰이는 슬롯이 `bForceKeyframe=true`였으면 그 플래그를 살아남는 슬롯으로 OR로 넘긴다(안 하면 새 클라이언트가 IDR을 못 받아 GOP 끝(≈2초)까지 화면이 안 나옴).
  - 버린 프레임 수는 카운터 + 주기적 로그(스트림 건강 확인용).
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

### 2.6 완료 확인은 워커가 한다 — 안전성 근거 (2026-10-01 추가 조사) [A]

결론: **워커 스레드가 `FRHIGPUFence::Poll()`을 sleep 폴링**(안 A). 엔진 소스와 엔진 자체 플러그인 선례로 안전성이 확인돼 RT 폴링(안 B, +1프레임)은 필요 없다.

**① 리눅스 기본 구성에서 `Poll()`이 실제로 하는 일**
- `GAllowTimelineSemaphores = 1` 기본(엔진 `VulkanRHI/Private/VulkanQueue.cpp:23`) → 펜스 생성 시 `VkEvent`를 **만들지 않는다**(`VulkanUtil.cpp:269`, `!GAllowTimelineSemaphores`일 때만 생성).
- `r.Vulkan.UseMinimalSubmits = 0` 기본(`VulkanDevice.cpp:71-75`) → 펜스의 완료 싱크포인트 타입은 `GPUAndCPU`(`VulkanUtil.cpp:335`).
- 따라서 `Poll()`(`VulkanUtil.cpp:298-311`)은 사실상 `CompletedSyncPoint->IsComplete()` = `FGraphEvent::IsComplete()` 한 줄. `FGraphEvent`는 태스크 그래프의 스레드 간 동기화 원시 객체라 어느 스레드에서 읽어도 된다.
- 엔진 헤더가 이 타입을 "IsComplete() ... can be used to poll for completion from the CPU"로 정의(`VulkanSubmission.h:31-33`).

**② 누가 완료를 알려 주나 — 폴링이 늦지 않은 이유**
- 타임라인 세마포어 + 태스크 워커 7개 이상이면 엔진이 전용 **`RHIInterruptThread`**(TPri_Highest)를 띄워 GPU 타임라인 진행을 기다렸다가 싱크포인트의 `FGraphEvent`를 완료시킨다(`VulkanSubmission.cpp:148-160`, `r.Vulkan.Submission.UseInterruptThread=1` 기본 `:134-139`).
- 즉 GPU가 복사를 끝내면 인터럽트 스레드가 곧바로 표시 → 워커의 다음 폴링(≤50 µs~0.5 ms)에서 잡힌다. 강제 제출·CPU 대기 없음.
- 인터럽트 스레드가 없는 구성(워커 6개 이하 CPU 등)이면 RHI 스레드가 완료를 처리 — `Poll()`은 여전히 정확하고 **늦어질 뿐**(최대 ≈1프레임). 정확성 문제는 없다. 초기화 때 `GetNumWorkerThreads()`·타임라인 지원 여부를 1회 로그로 남겨 전시 PC가 어느 쪽인지 기록.

**③ 엔진 자체 선례 — 같은 패턴을 Epic이 이미 쓴다**
- **MediaIO `FSyncPointWatcher`**(`Plugins/Media/MediaIOFramework/Source/MediaIOCore/Private/MediaCaptureSyncPointWatcher.cpp:17-78`): 전용 `FRunnable` 스레드(TPri_Highest)가 `RHIFence->Poll()`을 **50 µs `SleepNoStats`** 간격으로 돌린다. 심지어 `Clear()`도 그 스레드에서 호출(`:80`). 이 설계(안 A)와 구조가 같다.
- **PixelCapture(Pixel Streaming 캡처)**(`Plugins/Media/PixelCapture/Source/PixelCapture/Private/PixelCaptureCapturerRHI.cpp:78-94`): `AnyBackgroundHiPriTask`에서 `Fence->Poll()`, 안 끝났으면 태스크 재예약.

**④ 이 설계가 지킬 규칙(선례보다 보수적으로)**
- `Clear()`와 `WriteGPUFence()`는 **RT만** 호출(MediaIO는 워커에서 Clear하지만 따라 하지 않음).
- 워커는 슬롯을 **CAS로 점유한 뒤에만** 그 슬롯의 펜스를 `Poll()` — RT와 같은 펜스를 동시에 만지는 순간이 없다.
- 펜스 필드(`CompletedSyncPoint`)는 RT의 `RHIWriteGPUFence_TopOfPipe`(`VulkanUtil.cpp:327-336`)에서 써지고, 그 뒤 RT가 큐에 넣는 동작(잠금/원자 연산 = release)이 워커 쪽 읽기보다 앞선다.
- `FGPUFenceRHIRef`의 마지막 참조는 RT(`Shutdown`)에서 놓는다.

**기각된 대안 참고**: `FRHIGPUFence::Wait()`를 워커에서 부르는 것 — Vulkan 구현은 `RHICmdList`를 안 쓰고 `ProcessInterruptQueueUntil`도 여러 스레드를 고려해 `InterruptCS.TryLock`으로 짜여 있지만(`VulkanSubmission.cpp:500-530`), API가 `FRHICommandListImmediate&`를 요구해 RT 전용 계약으로 읽히므로 쓰지 않는다.

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

## 7. 구현 기록 (2026-10-01, 빌드 전)

`RE/` 기준. 리눅스(Vulkan) 먼저, 같은 세션에서 Windows(D3D12)까지.

| 파일 | 내용 |
|---|---|
| `Public/IRtspFrameEncoder.h` | 계약 변경: `EncodeFrame(..., OutAccessUnits)` → `SubmitFrame(RHICmdList, Source, FRtspFrameInfo)` + `SetOutputSink(StreamName, Sink)`. `FRtspFrameInfo`(Pts·제출 시각·키프레임 요청) |
| `Public/RtspAsyncEncoderBase.h` · `Private/RtspAsyncEncoderBase.cpp` (신규, p4 add) | 슬롯 3개 상태기계(Free/Writing/Queued/Claimed, CAS), 스트림당 `FRunnable` 워커(`RtspEncode_<mount>`), 최신 화면 우선(RT 회수 + 워커 스킵), 키프레임 요청 승계(RT 쪽·워커 쪽 둘 다), 10초마다 드롭 통계 로그(`LogRtspAsyncEncoder`) |
| `FNvencVulkanEncoder.h/.cpp` | ExportableImage/CUDA import/펜스를 `FStagingSlot` ×3으로. RT: 복사 + `WriteGPUFence`만(`ImmediateFlush`·`Wait` 삭제). 워커: `NumPendingWriteCommands==0 && Poll()` 100 µs 폴링 → `cuMemcpy2D` → 인코드. `NvEncoderCuda(..., nExtraOutputDelay=0)`. 이전 쓰기가 안 끝난 펜스는 Clear 대신 새로 생성(타임라인 off 구성의 VkEvent 리셋 경쟁 방지). Shutdown: 워커 join → RT에서 기록된 펜스 대기 → 파괴 |
| `Windows/NvencD3D12Encoder.h/.cpp` | 슬롯 k = NVENC 입력 버퍼 k(`GetNextInputFrame(k)`), `nExtraOutputDelay=NumSlots-1`은 버퍼 개수용. RT: 기존 네이티브 복사 + `RHISignalManualFence`만, 슬롯별 펜스 값 기록. 워커: `ID3D12Fence::GetCompletedValue()` 폴링(정지 요청을 볼 수 있게) → `EncodeFrameAtIndex`. Shutdown: 워커 join → 마지막 펜스 값까지 최대 5초 대기 → 파괴 |
| `ThirdParty/.../Windows/NvEncoderD3D12.h/.cpp` | **벤더 코드에 함수 1개 추가**: `EncodeFrameAtIndex(bfrIdx, waitValue, ...)` — 인덱스 고정 인코드 + 즉시 Lock(출력 지연 0). 샘플의 `MapResources`/`GetEncodedPacket`이 private이라 서브클래스로는 불가. `[titan_example RtspEncoder 추가]` 주석으로 표시 — SDK 교체 시 다시 넣어야 함 |
| `RtspStreamComponent.h/.cpp` | 렌더 커맨드는 스케일 패스 + `SubmitFrame`만. `PendingSubmitWallClockSeconds` FIFO 삭제. 싱크는 `FRtspStreamHandle` 사본만 캡처 |
| `RtspServerSubsystem.h` | `PushEncodedFrame`을 `static`으로(본문이 `Handle.State`만 씀 — .cpp 변경 없음) |

- P4: `RtspServerSubsystem.cpp`는 변경 없이 열려 있음 — `p4 diff`가 "Translation of file content failed near line 8"로 실패해 `revert -a`가 판정을 못 함(워크스페이스 charset 문제로 보임, 이 작업과 무관). 수동 revert 가능.
- stat 스코프 `RtspStreamEncodeFrame`은 이름 유지(이제 복사 기록만 — 전후 비교용). 워커 인코드는 Insights `RtspEncodeWorker_EncodeSlot`.

### Windows PIE 검증 (2026-10-06) — 통과

- **1차: 실패(RTSP 수신 화면 검정)**. 진단 로그로 원인 확정: 벤더에 추가한 `EncodeFrameAtIndex`가 `WaitForCompletionEvent(bfrIdx)`를 불렀는데, `DoEncode`는 완료 이벤트를 `GetCompletionEvent(m_iToSend % m_nEncoderBuffer)`로 붙인다(`NvEncoder.cpp` DoEncode). `m_iToSend`를 안 움직였으므로 NVENC는 항상 0번 이벤트에 신호 → 슬롯 1·2 인코드가 20초 타임아웃(`NvEncoder.cpp:850`)으로만 끝남(로그 실측 ≈20000 ms, 스트림당 20초에 1프레임). D3D12 세션은 async(`enableEncodeAsync`=caps, 이벤트는 등록 없이 picParams로 전달)라 이 대기가 실제로 돈다.
- 수정: `WaitForCompletionEvent(m_iToSend % m_nEncoderBuffer)` 후 `m_iToSend++; m_iGot++` (이벤트 인덱스·frameIdx 정상 순환, `EndEncode` 루프는 항상 비어 무해). 리눅스 CUDA 경로는 샘플 `EncodeFrame` 그대로라 해당 없음.
- **2차: 통과** — 사용자 파이썬 UGV RC GUI로 5스트림 수신 정상. 로그: 접속 직후 첫 프레임 즉시 push(IDR 13~32 KB), 드롭 통계 0건(10초 로그 미출력 = 드롭 없음), 100 ms 넘는 인코드는 레벨 시작 직후 1.5초(시작 히치 구간, 168~232 ms)뿐. `stat dumpave` 수치 비교는 아직 안 함.
- 진단 로그(`[diag]`)는 리눅스 실기 검증까지 유지(Vulkan 경로에도 같은 진단 추가), 그 뒤 제거.

### 검증 체크리스트(빌드 후)
1. Windows 에디터/`-game`: 스트림 생성 로그 "(async, 3 staging slots)", VLC 5스트림 수신, 재연결 시 즉시 화면(IDR 승계), 레벨 종료/시나리오 재시작 시 hang·크래시 없음.
2. `stat dumpave -num=120 -ms=0.1`로 `RtspStreamEncodeFrame` ≈0 확인(S0 M1 12 ms 대비).
3. 리눅스 패키지: 위 1·2 + `-vulkandebug` 검증 레이어 에러 0, 지연(`capture_to_push_ms` 방식) 비교, 초기화 로그의 `task workers=` 값(7 이상이면 RHIInterruptThread 있음).
4. `LogRtspAsyncEncoder` 드롭 로그가 정상 운용 중 안 찍히는지(찍히면 워커가 프레임 간격보다 느림).

## 6. 결정 기록 / 미결

- ✅ 완료 확인 주체 = 워커 폴링(안 A) — 2.6절 근거(2026-10-01).
- ✅ 슬롯 부족 시 = 최신 화면 우선(오래된 Queued 슬롯 회수, 키프레임 플래그 승계) — 사용자 결정(2026-10-01).
- ✅ P0 생략, P1 직행 — P1의 워커는 제출 직후 Lock하므로 출력 지연 설정 자체가 무의미해져 P0의 1줄은 그대로 대체됨. 일정상 P1 전에 전시할 일도 없음(2026-10-01 사용자 결정).
