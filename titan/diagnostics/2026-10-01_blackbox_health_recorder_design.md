# 블랙박스 상태 기록기 (Health Recorder) 설계

2026-10-01 / 진행중(1단계 코드 완료·빌드 전) / 매 프레임 상태를 mmap 링버퍼에, 1초 요약을 CSV에 남기고, 별도 감시 스레드가 멈춤·에러 구간까지 기록하는 자체 진단 시스템.

> 📌 1단계 구현 결과·설계와 달라진 점·검증 체크리스트는 맨 아래 **14절**. 미결 Q1~Q3 은 확정됨(13절).
> 📌 0절의 9/28 사고 분석 전문(환경 비교·오디오·드라이버·"메모리 누수" 제보와 탐지 코드 소수정)은 `2026-10-01_lig_0928_incident_log_analysis.md`.

## 0. 왜 만드나

LIG 현장 사고(2026-09-28, 로그 `C:\working\logs`)를 분석하면서 드러난 한계:

| 사고 | 로그에 남은 것 | 남지 않아서 원인을 못 가린 것 |
|---|---|---|
| MUM-T 11:24 실행 — 시작 2분 뒤 멈춤, 10분 정지, 종료 시 `VK_ERROR_DEVICE_LOST` | 프레임 번호가 `[615]`에서 고정, 오디오 h/w 타임아웃 | 멈추기 직전 무엇을 하고 있었나(조이스틱 버튼 17 연타 외엔 없음), 그때 RAM/VRAM, 어느 스레드가 멈췄나 |
| 자기방호 15:23 — 정상 동작 중 X11 끊김 + DEVICE_LOST | 마지막 줄 | 직전 수 초의 부하·메모리 추이 |
| 자기방호 11:18 — 시작부터 VRAM 예산 7.2/16.3 GB, 87 MB 할당 실패 | 시작 시 1회 예산표 | 그 뒤 VRAM 추이 |

그리고 LIG가 "메모리 누수"라고 전해 왔지만 **엔진 로그에는 메모리 수치가 한 줄도 없어서** 맞는지 틀린지 판정할 수 없었다. GPU 크래시 breadcrumb 도 `-gpucrashdebugging` 없이 실행해 비어 있었다.

목표: **다음에 무엇이 터지든, 그 순간 우리 프로그램이 무엇을 하고 있었고 자원 상태가 어땠는지를 시간축으로 맞춰 볼 수 있게 한다.**

## 1. 요구사항 (사용자 확정, 2026-10-01)

1. **매 프레임** 상태를 남긴다 — 갑자기 생긴 문제의 직전 상황(조이스틱 조작 중이었나, 교전이 격했나)을 프레임 단위로 대조할 수 있어야 한다.
2. **프로그램이 감지할 틈 없이 죽어도**(OOM killer·`kill -9`) 직전 기록이 남아야 한다.
3. **게임 스레드가 멈춘 동안에도** 기록이 계속돼야 한다(MUM-T 사고처럼 10분 정지).
4. **PC 전체가 굳어 전원 재부팅**해도 최대 약 1초만 잃는다.
5. 성능 영향은 측정 오차 수준.
6. 엔진 로그·시나리오 흐름 로그(`Saved/ScenarioLogs`)와 **같은 시각(UTC)·프레임 번호**로 맞춰 볼 수 있다.
7. 리눅스(Vulkan, LIG 현장)와 윈도우(개발) 둘 다, 모든 실행 형태(축 선택 화면·데모·풀 시스템·서버·클라이언트)에서 동작.

경고창(사전 경고)·다음 실행 시 사후 보고·오류코드는 **이 문서 범위 밖(2·3단계)**이다 — 12절.

## 2. 구성 요소

```
                ┌────────── 게임 스레드 ──────────┐
 IInputProcessor ─▶ 입력 카운터 ─┐                │
 OnGunshot 등 훅 ─▶ 게임 카운터 ─┼─▶ OnEndFrame ──┼─▶ ① blackbox.bin (mmap 링버퍼, 매 프레임 1레코드)
 RHIGetMemoryStats (캐시 복사) ──┘   프레임 수집기 │   └─▶ 초 요약 (lock-free 핸드오프)
                └─────────────────────────────────┘                    │
                ┌───────── 감시 스레드 (0.25 s) ────────────────────────┴──┐
                │ 프레임 진행 확인 → 멈춤 판정 / RAM(/proc) 조회 / 1 s 마다: │
                │  ② health.csv 1줄, msync(blackbox)                         │
                └────────────────────────────────────────────────────────────┘
 GLog 출력 장치(Error/Fatal + 지정 Warning), ensure/system error 델리게이트 ─▶ ③ events.log + 링버퍼 보존본
```

| 요소 | 클래스(가칭) | 스레드 |
|---|---|---|
| 수명 관리·설정 | `UTitanHealthRecorderSubsystem : UEngineSubsystem` | 게임 |
| mmap 링버퍼 | `FTitanBlackboxRing` (리눅스 `mmap(MAP_SHARED)` / 윈도우 `CreateFileMapping`) | 게임(쓰기), 감시(msync) |
| 프레임 수집 | `FCoreDelegates::OnEndFrame` 핸들러 | 게임 |
| 입력 관찰 | `FTitanInputObserver : IInputProcessor` (`FSlateApplication::RegisterInputPreProcessor`) | 게임 |
| 감시 스레드 | `FTitanHealthWatchdog : FRunnable` | 전용 |
| 에러 수집 | `FTitanHealthLogSink : FOutputDevice` (`GLog->AddOutputDevice`) + `FCoreDelegates::OnHandleSystemError/Ensure` | 호출한 스레드 |

**`UEngineSubsystem` 인 이유**: 프로세스 전체 수명. 기존 `UScenarioMonitorSubsystem` 은 월드 서브시스템이라 레벨 전환·축 선택 화면·레벨 로딩 구간이 비는데, 사고는 그런 구간에서도 난다(MUM-T 사고는 레벨 진입 약 2분 뒤). 월드 의존 값(병사 수 등)은 매 프레임 현재 게임 월드에서 읽는다.

**다른 시스템을 건드리지 않는다** — `ScenarioMonitorSubsystem` 과 같은 원칙. 입력은 Slate 전처리기로 관찰만, 사격은 기존 공개 델리게이트(`USoldierRegistrySubsystem::OnGunshot`), RCWS 는 컴포넌트 공개 값 폴링. 예외는 5.3절(LIG UDP·RTSP 카운터 — 2단계).

## 3. 매 프레임 레코드

고정 크기 바이너리(128 B 목표). 텍스트로 쓰지 않는 이유: 매 프레임 포맷팅 비용과 파일 크기. 해석은 디코더(9절)가 한다.

| 필드 | 출처 | 비고 |
|---|---|---|
| `FrameCounter` (u64) | `GFrameCounter` | 엔진 로그의 `[nnn]` 은 이 값 % 1000 |
| `UtcTicks` (i64) | `FDateTime::UtcNow()` | 엔진 로그 시각과 같은 UTC |
| `WorldSeconds` | 게임 월드 `GetTimeSeconds()` | 없으면 -1 |
| `DeltaMs` | `FApp::GetDeltaTime()` | |
| `GameMs` / `RenderMs` / `RHIMs` / `GPUMs` | `GGameThreadTime`/`GRenderThreadTime`/`GRHIThreadTime`/`RHIGetGPUFrameCycles()` | `stat unit` 과 같은 값(`RenderCore/Public/RenderTimer.h`) |
| `GameWaitMs` | `GGameThreadWaitTime` | 렌더/GPU 대기 — GPU 행 직전에 커진다 |
| `KeysDown` / `KeysUp` | 입력 관찰자 | 이 프레임 눌림/뗌 수(키보드·마우스·조이스틱 전부) |
| `LastKey` (u16) | 입력 관찰자 | 이 프레임 마지막 키의 인덱스 — 키 이름 표는 세션 시작 시 events.log 에 한 번 |
| `JoyAxisMax` | 입력 관찰자 | 이 프레임 조이스틱 축 절댓값 최대(조작 중인가) |
| `ShotsSoldier` / `ShotsRCWS` | `OnGunshot` / RCWS 발사 수 증분 | 교전 강도 |
| `SoldiersH` / `SoldiersF` | SoldierLab 등록부 | 매 프레임 세면 비싸면 초 단위 값 복사 |
| `ScenarioCycle` / `ScenarioStep` | `UScenarioMonitorSubsystem` 게터 | 스텝은 DT 행 인덱스 |
| `NetConnections` | 월드 NetDriver `ClientConnections.Num()` / 클라 연결 여부 | 2-PC 구성 |
| `RamAvailMB` / `RamProcMB` | 감시 스레드의 최근 샘플 복사 | 1 s 갱신값을 매 레코드에 싣는다 — 레코드 하나만 봐도 상황이 나온다 |
| `VramUsedMB` / `VramBudgetMB` | `RHIGetMemoryStats()` → `UsedLocal`/`BudgetLocal` | 아래 참고 |
| `Flags` | | 히치 / 에러 줄 / 레벨 전환 중 / GPU 크래시(`GIsGPUCrashed`) 비트 |

**VRAM 조회가 싼 이유 (엔진 소스 확인)**: Vulkan RHI 는 매 프레임 `RHIEndFrame` 에서 `FDeviceMemoryManager::UpdateMemoryProperties()` 로 `VK_EXT_memory_budget` 을 이미 읽어 캐시한다(`VulkanRHI.cpp:1230-1235`, `VulkanMemory.cpp:576-663`). `RHIGetMemoryStats()` 는 그 캐시 구조체를 복사할 뿐이다(`VulkanRHI.cpp:1693-1699`). `RHIGetTextureMemoryStats()` 는 예산이 아니라 총량만 채우므로(`VulkanTexture.cpp:1043-1057`) 쓰지 않는다. ~~`UsedLocal` 은 GPU 전체 사용량이다~~ → **정정(2026-10-01 리눅스 실측)**: `UsedLocal`(heapUsage)은 **이 프로세스의 사용량**이고(시작 직후 16 MB 로 찍힘 — 데스크톱만으로도 그보다 크다), `BudgetLocal`(heapBudget)은 다른 프로세스 사용분을 고려한 **이 프로세스의 가용 예산**이다. 11:18 사고처럼 다른 프로세스가 VRAM 을 쥔 상황은 **예산이 평소보다 작은 것**(그 사고: 7.2 GB vs 평소 15.3 GB)으로 보인다.

**링 크기**: 기본 120 초 분량(60 fps 기준 7,200 레코드 ≈ 0.9 MB). cvar 로 조정.

**헤더**: 매직 `TBBX`, 포맷 버전, 레코드 크기, 용량, 쓰기 인덱스(원자적), 세션 ID, 프로세스 시작 UTC, 빌드 정보(엔진 CL·프로젝트 버전·패키지 경로), 호스트명. 디코더가 헤더만 보고 해석할 수 있게.

## 4. 1초 요약과 이벤트

**`health.csv`** — 감시 스레드가 1초마다 한 줄. 12시간 ≈ 43,200 줄, 약 10 MB.

```
utc, frame, world_s, fps_min, fps_avg, fps_max, frame_ms_max, game_ms_max, render_ms_max, gpu_ms_max,
ram_avail_mb, ram_proc_mb, ram_proc_peak_mb, vram_used_mb, vram_budget_mb,
keys, joy_active_frames, shots_soldier, shots_rcws, soldiers_h, soldiers_f, actors, projectiles,
scenario_cycle, scenario_step, net_conns, stalled
```

- 최대/최소를 남긴다 — 평균만 있으면 1프레임 히치가 묻힌다.
- `actors`/`projectiles` 는 액터 순회가 필요해 게임 스레드에서 1초에 한 번만 센다(시나리오 로그의 `CountActors` 와 같은 방식).
- **게임 스레드가 멈춰도 이 파일은 계속 쓰인다.** 그때 프레임 지표 열은 비고 `stalled=<경과초>`, RAM 열은 계속 갱신 — "멈춘 동안 메모리가 어땠나"가 여기서 나온다.

**`events.log`** — 이산 이벤트. 태그는 시나리오 로그와 같은 형식.

| 태그 | 내용 |
|---|---|
| `SYS` | 세션 시작(빌드, 명령줄, GPU/드라이버, RAM, 비디오 드라이버, 키 이름 표), 레벨 전환, 정상 종료 |
| `HITCH` | 프레임 시간 기준 초과 — 그 프레임의 게임/렌더/GPU 분해 |
| `STALL` | 멈춤 시작/지속/해제(5절) |
| `MEM` | RAM·VRAM 문턱 통과(오르내림 둘 다) |
| `ERR` | 엔진 Error/Fatal 로그 줄 원문, ensure, system error, `GIsGPUCrashed` 전이 |
| `SNAP` | 링버퍼 보존본 생성(사유·파일명) |

## 5. 이상 감지와 보존

### 5.1 트리거

| 트리거 | 감지 | 초기 기준(cvar) |
|---|---|---|
| 히치 | 게임 스레드, 프레임 끝 | 프레임 > 250 ms |
| 멈춤 | 감시 스레드: `GFrameCounter` 가 진행 안 함 | 2 s 무진행 → `STALL` 시작, 이후 5 s 마다 지속 줄 |
| RAM 부족 | 감시 스레드 | 시스템 가용 < 1.5 GB 또는 < 10 % |
| VRAM 부족 | 게임 스레드(캐시값) | 사용 > 예산의 90 % |
| 엔진 에러 | `FTitanHealthLogSink` | `Error`/`Fatal` 전부 + 지정 Warning(`Failed to allocate Device Memory`, `X11`, `VK_ERROR`) |
| 크래시·ensure | `FCoreDelegates::OnHandleSystemError` / `OnHandleSystemEnsure` (`CoreDelegates.h:170,173`) | |
| GPU 크래시 | `GIsGPUCrashed` (감시 스레드 폴링) | Vulkan 은 `TerminateOnGPUCrash` 가 이 값을 세운 뒤 `RequestExit(true)` — 델리게이트가 없어서 폴링(`VulkanUtil.cpp:678-705`) |

### 5.2 보존 방식

링버퍼는 **평소에도 항상 디스크에 있다**(mmap). 트리거는 "지금 링을 `snapshots/blackbox_<utc>_<사유>.bin` 으로 복사해 덮어쓰기에서 지킨다"는 뜻이다. 복사는 감시 스레드가 한다(게임 스레드가 멈춘 경우 포함). 같은 사유는 최소 30 s 간격(폭주 방지).

| 종료 형태 | 결과 |
|---|---|
| 크래시·ensure·GPU 크래시(엔진이 우리를 부를 틈 있음) | 링 + 보존본 + `ERR` 줄 |
| OOM killer·`kill -9` (틈 없음) | **링은 남는다** — mmap 페이지는 커널 소유 |
| 게임 멈춤 후 사람이 창 닫음 | 멈춘 동안 health.csv 계속 + `STALL` 보존본 |
| PC 전체 정지 → 전원 재부팅 | 감시 스레드가 1 s 마다 `msync` 하므로 **마지막 약 1 s 만 유실** |

### 5.3 엔진 행 감지기(`FThreadHeartBeat`)와의 관계

엔진에도 행 감지기가 있지만 `[Core.System] HangDuration` 이 0 이면 스레드 자체를 안 만든다(`ThreadHeartBeat.cpp:92-107`). 켜면 멈춘 스레드의 콜스택을 로그에 남기는 게 큰 장점(`:186-219`)이나, 빌드 설정(`MINIMAL_FATAL_HANG_DETECTION`/`UE_ASSERT_ON_HANG`)에 따라 **일부러 크래시시킨다.** → 우리 감시 스레드는 기록만 하고, 엔진 행 감지기는 켜되 비치명(Error 로그) 동작인지 패키지 빌드에서 먼저 확인한 뒤 결정한다(미결 사항 Q3).

## 6. 파일 위치와 보존 기간

```
Saved/HealthLogs/<UTC yyyy.MM.dd-HH.mm.ss>_<호스트>/
    blackbox.bin        mmap 링(최근 120 s)
    health.csv          1 s 요약
    events.log          이벤트
    snapshots/          트리거 보존본
```

- 최근 30 세션 또는 합계 2 GB 초과 시 오래된 세션부터 삭제(세션 시작 시).
- LIG 에는 "`Saved/HealthLogs` 폴더째 보내 달라" 한 줄로 안내 가능.

## 7. 시간 맞추기

- 모든 줄/레코드에 **UTC** + `GFrameCounter`. 엔진 로그(`[2026.09.28-02.25.39:099][326]`)도 UTC 이고 뒤 숫자는 프레임 % 1000 이라(9/28 로그로 확인: 로그 시각 02:17 ↔ "Log file closed 11:17" 로컬) 그대로 대조된다.
- 시나리오 로그(`ScenarioMonitorSubsystem`)는 월드 초 기준이므로, `events.log` 의 `SYS` 레벨 진입 줄에 "월드 초 0 = UTC …" 를 남겨 변환 기준으로 쓴다.

## 8. 성능 예산

| 항목 | 주기 | 예상 |
|---|---|---|
| 프레임 레코드 채우기(원자 카운터 읽기 + 128 B 쓰기) | 매 프레임 | 수 µs |
| 입력 관찰자 | 입력마다 | 카운터 증가뿐 |
| `RHIGetMemoryStats()` 캐시 복사 | 매 프레임 | 구조체 복사 |
| 액터/투사체 수 | 1 s | 시나리오 로그 `CountActors` 와 동급 |
| RAM 조회(`FPlatformMemory::GetStats`, 리눅스 `/proc`) | 1 s, 감시 스레드 | 게임 스레드 영향 없음 |
| CSV/이벤트 쓰기·`msync` | 1 s, 감시 스레드 | 〃 |

검증: `STATGROUP_TitanHealth` 로 수집기 비용 계측 + `titan.Health.Enabled 0/1` A/B, 기준은 성능 기준선 S0 (`infra_architecture/2026-09-29_perf_baseline_s0.md`).

## 9. 디코더

- `titan/diagnostics/tools/blackbox_decode.py` — `blackbox.bin`/보존본 → CSV(헤더로 레이아웃 자동 해석, 링 순서 복원). 현장에서 파이썬만 있으면 됨.
- 인게임 콘솔 `titan.Health.DumpBlackbox` — 현재 링을 CSV 로(개발 중 확인용).

## 10. 콘솔 변수 (가칭)

| cvar | 기본 | 뜻 |
|---|---|---|
| `titan.Health.Enabled` | 1 | 전체 on/off(A/B) |
| `titan.Health.RingSeconds` | 120 | 링 길이(재시작 시 적용) |
| `titan.Health.HitchMs` | 250 | 히치 기준 |
| `titan.Health.StallSeconds` | 2 | 멈춤 기준 |
| `titan.Health.RamWarnMB` | 1536 | RAM 경고 |
| `titan.Health.VramWarnPct` | 90 | VRAM 경고 |
| `titan.Health.Snapshot` | — | 즉시 보존본(수동) |

## 11. 플랫폼 주의

- 리눅스 빌드는 clang `-Werror` — 도달 불가 코드 경고가 에러가 된다(`TargetDetectionComponent.cpp` `#if 0` 전례). 플랫폼 분기는 `#if PLATFORM_LINUX / PLATFORM_WINDOWS` 로 파일을 나눈다.
- mmap 파일은 세션 시작 시 `ftruncate` 로 크기 고정, 쓰기 인덱스는 헤더의 원자 변수.
- 윈도우: `CreateFileMapping` + `FlushViewOfFile`.

## 12. 단계

| 단계 | 내용 | 비고 |
|---|---|---|
| **1 (이 문서)** | 링버퍼·프레임 수집·입력 관찰·감시 스레드·health.csv·events.log·보존본·디코더 | 다음 LIG 방문 전 패키지에 포함 목표 |
| 2 | 사전 경고 토스트(운영자 화면 전용 옵션, 기존 `NotificationSubsystem`), LIG UDP 송수신 카운터(`UGVRemoteControlSubsystem`), RTSP 클라이언트·푸시 실패 카운터(`RtspEncoder` 플러그인) | 다른 시스템 소폭 수정 |
| 3 | 실행 중 표식 파일 → 다음 실행 시 "이전 실행 비정상 종료(오류코드)" 창 + 로그 묶음 폴더, 오류코드 표(`KDX-GPU-01` 등), 실행 스크립트 `-gpucrashdebugging` 기본화 검토 | |

## 13. 미결 사항 → 확정 (2026-10-01)

- **Q1 확정: 추가 안 함.** RCWS 모드·표적, UGV 주행 모드, 드론 상태 등은 시나리오 로그 `events.log` 가 이미 전이 단위로 남긴다 — 블랙박스는 UTC 로 그쪽과 맞춰 본다(사용자).
- **Q2 확정: 최근 30 세션.** 용량 상한(2 GB)은 두지 않음.
- **Q3 확정: 엔진 행 감지기는 끈 채로.** 빌드 설정에 따라 멈춤 감지 시 일부러 크래시시키므로. 우리 감시 스레드만 쓴다.
- **Q4 (그대로)** 감시 스레드는 커널 로그(`dmesg`)를 못 읽는다(root 필요) — NVIDIA Xid 는 여전히 현장 요청이 필요. 시작 시 GPU·드라이버 버전을 남기는 것까지만.

## 14. 1단계 구현 (2026-10-01, 코드 완료·빌드 전)

### 14.1 파일

| 파일 | 내용 |
|---|---|
| `Source/titan_example/Diagnostics/TitanBlackboxTypes.h` | 헤더(256 B)·레코드(**104 B**) 구조, 1바이트 패킹 + `static_assert` |
| `Diagnostics/TitanBlackboxRing.h/.cpp` | mmap 링(리눅스 `mmap(MAP_SHARED)`+`msync(MS_SYNC)` / 윈도우 `CreateFileMapping`+`FlushViewOfFile`, 그 외 메모리 폴백) |
| `Diagnostics/TitanHealthRecorderSubsystem.h/.cpp` | `UEngineSubsystem` 본체 + 공유 상태·감시 스레드·로그 싱크·입력 관찰자(cpp 내부 클래스) |
| `titan_example.Build.cs` | Private 의존성 `RHI`, `RenderCore` 추가 |
| `titan/diagnostics/tools/blackbox_decode.py` | 디코더 — 가짜 파일로 링 순환 복원·KEY 이름·플래그 해석 확인함 |

다른 시스템 코드는 **수정하지 않았다**(시나리오 모니터 `GetCycleNumber()`·등록부 `OnGunshot`·`URCWSComponent::GetCurrentData()` 공개 API 만 읽음).

### 14.2 설계와 달라진 점

- 레코드 **104 B**(설계 128 B 목표). 필드는 3절 그대로 + `Actors`(1초값). 링 용량 = `RingSeconds × 100` 레코드(기본 12,000 ≈ 1.2 MB) — 40 fps 현장이면 약 5분, 100 fps 면 120 초.
- **시나리오 스텝은 레코드에 안 싣는다**(사이클 번호만) — Q1 결정과 같은 이유로 시나리오 로그와 시각으로 대조.
- **병사 사격(`shots_soldier`)은 서버/단독 실행만** — `USoldierRegistrySubsystem::OnGunshot` 이 서버에서만 발동한다. 클라이언트(풀 시스템 자체방호 PC)는 0. RCWS 사격은 **복제되는 탄약 감소**로 세서 클라에서도 잡힌다.
- `health.csv` 열에 `frames`, `rhi_ms_max`, `wait_ms_max`, `rcws_rounds`/`soldier_rounds`(투사체를 두 풀로 나눠서), `net_mode` 추가.
- 엔진 로그 싱크의 "지정 Warning" = `Failed to allocate` / `VK_ERROR` / `DEVICE_LOST` / `X11` / `out of memory` / `hang detected`(9/28 LIG 로그에서 사고 직전에 찍힌 것들). 1초 20줄 상한.
- **에디터(PIE 포함)에서는 기본 꺼짐** — 명령줄 `-TitanHealthInEditor` 로 켠다. 커맨드릿(쿡)·데디서버는 항상 끔.
- 콘솔 추가: `titan.Health.Snapshot`(수동 보존본), `titan.Health.Mark <메모>`(events.log 에 한 줄 — 현장에서 "지금 이상함" 표시용).
- VRAM: 윈도우 D3D12 도 `RHIGetMemoryStats` 를 구현하고 있어 개발 PC 에서도 값이 나온다.

### 14.2a 윈도우 PIE 검증 결과와 후속 수정 (2026-10-01)

에디터 `-TitanHealthInEditor` + PIE, 세션 `Saved/HealthLogs/2026.10.01-00.50.00_DESKTOP-81S78B2`.

| 항목 | 결과 |
|---|---|
| 파일 4종·1초 CSV·하드웨어 줄 | ✅ (RTX 5060 / D3D12 / 610.88) |
| 조이스틱 | ✅ `KEY 2 = GenericUSBController_Button1`, `3 = Joystick_Extreme_3D_pro_Button_0` — 한 번 누르면 엔진 원시 HID 키와 JoystickPlugin 키가 **둘 다** 온다(정상) |
| `debug stall 10` | ✅ `STALL 시작(2.0 s)/지속 7 s/해제 10.0 s`, 멈춘 동안 CSV 가 1초마다 RAM 만 채워 계속 기록(`stalled_s` 2.5→9.5), 이어서 `HITCH 10033 ms` |
| `debug crash` | ✅ `ERR` 원문 + `crash` 보존본, 본 `blackbox.bin` 디코딩 시 마지막 레코드 = 크래시 프레임(F17177) |
| MEM | ✅ RAM 가용 1 GB대 경고(개발 PC 에디터 18 GB — 실제값), VRAM 6134/6590 MB 경고 |

발견 → 수정(빌드 필요):
- `joy_frames` 가 손을 안 대도 매초 프레임 수와 같았다 — 스로틀처럼 중립이 0 이 아닌 축이 매 프레임 큰 값으로 와서. **레코드 `JoyAxisMax` 를 "축 값 절댓값" → "직전 값 대비 움직인 양"으로 변경**, `joy_frames` 문턱 0.01.
- PIE 시작은 `PreLoadMap/PostLoadMapWithWorld` 를 안 거쳐 "레벨 진입" 줄이 없었다 → 게임 월드가 바뀔 때 `SYS 게임 월드 → <맵> (PIE|Game, NetMode)` 한 줄.
- 크래시 델리게이트가 2초 간격으로 **두 번** 불려 보존본이 둘 생겼다 → 한 번만 처리, 이후 크래시 리포터가 만드는 멈춤은 `STALL … — 크래시 처리 중` 줄만 남기고 보존본 생략.
- 크래시 리포트 연결: `FGenericCrashContext::SetGameData("TitanHealthSession", <세션 폴더>)` — `Saved/Crashes/*/CrashContext.runtime-xml` 에 블랙박스 경로가 실린다.
- 오디오: `Buffer Underrun`/`waiting for h/w` 줄을 `AUDIO` 태그로(9/15·9/28 MUM-T 멈춤 두 건에 모두 동반 — 선후 판정용).

알려진 한계(수정 안 함): **에디터에서는 `render_ms` 가 거의 0** — 엔진이 렌더 스레드 시간을 창마다 계산하는데(`SlateRHIRenderer.cpp:1327-1359`) 에디터는 창이 여럿이라 마지막 창 몫만 남는다. Standalone/패키지에서는 정상(앞 세션 15 ms 대). 콘솔에 친 글자도 `KEY` 줄로 남는다(무해).

### 14.2b 리눅스 패키지 검증 결과 (2026-10-01)

사내 리눅스 PC(RTX 4070 SUPER / 595.84 / Vulkan / `-sdlvideodriver=x11`), 로그 `D:\log_1001`(3 세션).

| 항목 | 결과 |
|---|---|
| 리눅스 빌드(clang) | ✅ |
| mmap | ✅ 3 세션 모두 `(mmap)` |
| 레벨 전환·`게임 월드 →` 줄 | ✅ |
| `joy_frames`(수정 후) | ✅ 평소 0, 조작한 초만 값 |
| `debug stall 10` ×2 | ✅ 시작 2 s / 지속 / 해제 10.0 s, 멈춘 동안 CSV 계속 |
| `debug gpucrash` | ✅ `appError … Simulating a GPU crash` + `crash` 보존본, 링 마지막 = 크래시 프레임 F1505 |
| 즉사 보존 | ✅ 세션 1: **엔진 로그는 04:21:15.904 에서 종료 기록 없이 끊겼는데 링은 04:21:16.431 까지** — 엔진 로그가 잃은 마지막 0.5 s 를 링이 갖고 있다 |

발견 → 수정(빌드 필요): **리눅스는 창 닫기도 GPU 크래시도 `RequestExit(force)` → `_exit(1)` 즉시 종료**라(`UnixPlatformMisc.cpp:337-355`) `Deinitialize` 의 "정상 종료" 줄이 안 남는다(세션 2: 창 닫기인데 종료 줄 없음). 로그 싱크가 `RequestExit(` / `SDL_EVENT_QUIT` 줄을 잡아 `SYS 종료 요청 — <원문>` + 링 flush. 원문의 CallSite 로 창 닫기 / GPU 크래시(`FVulkanDynamicRHI.TerminateOnGPUCrash`) / 흔적 없음(= OOM·kill)을 구분한다.

남은 제안: 레벨 진입 직후 로딩 히치(약 2 s)마다 보존본이 생긴다 — 보존본 개수 상한을 넣을 때 "레벨 진입 후 N 초 히치 보존본 생략"도 같이.

### 14.2c 보존본 등급·스왑·자원 요약 (2026-10-01, 코드 완료·빌드 전)

- **보존본 등급(사용자 확정)** — 개수 상한은 장시간 실행 끝의 진짜 사고를 놓치므로 기각. **A(crash·fatal·gpu_crash·stall·manual) = 절대 삭제 안 함**, **B(hitch·error·ram_low·vram_high·swap_grow·ensure) = 세션당 총 용량 `titan.Health.SnapshotBudgetMB`(기본 300) 초과 시 오래된 것부터 삭제, 단 사유별 첫 3개 보호**. 삭제해도 `SNAP` 줄은 남고 `SNAP 정리 — … 삭제` 줄이 추가된다. 마지막 사고 직전은 보존본과 무관하게 `blackbox.bin` 본체에 있다.
- 레벨 진입 후 10 s 동안 끊김 보존본 생략, **레벨 로딩 중 멈춤은 줄만 남기고 A 보존본 생략**(리눅스 테스트에서 로딩 2.5 s 가 지울 수 없는 STALL 보존본이 됐다).
- **리눅스 스왑**: 감시 스레드가 1 s 마다 `/proc/meminfo`(`SwapTotal-SwapFree`) — `health.csv` 맨 끝 열 `swap_used_mb`(리눅스 외 -1), 시작 시 `MEM 세션 시작 시 스왑 사용`, 시작 대비 `titan.Health.SwapWarnMB`(512) 증가 시 `MEM 경고` + `swap_grow` 보존본. RAM 이 차면 스왑으로 넘어가며 PC 전체가 굳는 단계를 잡으려는 것.
- **자원 요약 줄**: 시작 시 `SYS 시작 시 자원 — VRAM 사용/예산, RAM 가용/프로세스, 스왑, UObject`, 게임 월드 변경 줄에도 같은 요약.
- **UObject 수**(`health.csv` `uobjects`, 게임 스레드 1 s): 계속 늘면 우리(엔진 객체) 쪽 누수, RAM 만 늘고 이건 그대로면 드라이버·네이티브 쪽 — "메모리 누수" 제보를 가르는 열. 멈춘 동안엔 마지막 값.
- **CPU**(리눅스, 감시 스레드 1 s — 멈춘 동안에도 갱신): `proc_cpu_cores`(이 프로세스 CPU 사용, 코어 단위 — `getrusage` 차분 ÷ 실제 경과), `load_avg1`(`/proc/loadavg` 1분 부하). 엔진 `FPlatformTime::GetCPUTime()` 은 리눅스에서 경과를 250 ms 로 고정해 나눠(`UnixPlatformTime.cpp:113`) 1 s 간격 호출 시 4배로 부풀어 쓰지 않음. 리눅스 외 -1.
- `health.csv` 끝 열 순서: `…,stalled_s,swap_used_mb,uobjects,proc_cpu_cores,load_avg1`.
- **장치·연결(`DEV` 태그, 2026-10-01 추가 — 리눅스 확인 완료 `D:\log_1001\3`: 시작 시 오디오 장치(HDMI)·조이스틱 Ready, RTSP 인코더 5개 생성, 조이스틱 USB 뽑기/꽂기 2회 `Device removed`→`Device Ready`, Alt+Tab 포커스 잃음/얻음, 화면 `2560x1440 창 전체화면 / 모니터 2대`, 시작 줄 UObject 47205(수정 확인). RTSP 클라이언트 접속 줄은 접속 클라이언트가 없어 미확인)**: 엔진/플러그인이 이미 남기는 줄을 로그 싱크가 문구로 잡아 같은 시간축에 — 조이스틱 연결·해제·준비(JoystickPlugin `JoystickSubsystem.cpp:744/844`, `JoystickInputDevice.cpp:489`), 오디오 출력 장치 선택·교체 시도(LogAudioMixer/SDL), RTSP 클라이언트 접속·해제·인코더 생성(LogRtspEncoder*). 카테고리는 FName 비교(모든 로그 줄이 지나가는 자리라 할당 없음). 기록기 초기화가 오디오·조이스틱 초기화보다 먼저라(리눅스 엔진 로그 1356 → 1378 → 1546줄) **시작 시 잡힌 장치도 남는다**. 추가로 창 포커스 얻음/잃음(`OnApplicationActivationStateChanged`)과 시작 시 화면 줄(게임 해상도·창 모드·주 모니터·모니터 수).
- **리눅스 확인(2026-10-01, `D:\log_1001\2`)**: 종료 사유 ✅(창 닫기 → `종료 요청 — Received SDL_EVENT_QUIT` + `RequestExit(bForce=true, ReturnCode=0)`), 새 열 ✅(스왑 0, UObject 47k→84k(로비→본 레벨), CPU 0.8~5.8 코어, 부하 2~3), 시작 줄 ✅. 시작 줄의 `UObject 0`(1초값 비어 있음)은 즉시 읽기로 수정, VRAM 문구 정정(위 3절). 로딩 중 멈춤 경로는 이번 실행에서 로딩이 2 s 미만이라 미발동 — 미검증.

### 14.2d 2026-10-12/13 추가 — UGV 자세 열·`ugv_flip`·RSS 분해 열

- **A 등급 사유 `ugv_flip` 추가**(`IsCriticalSnapshotReason`, `TitanHealthRecorderSubsystem.cpp:105-107`) — 시나리오 모니터가 UGV 전복(기울기 ≥60° 1.5 s)을 확정하면 `LogEvent("UGV", …)` + `RequestSnapshot("ugv_flip")`. 상세 `vehicle/ugv/2026-10-13_ugv_flip_detection_logging.md`.
- **리눅스 RSS 분해 4열**: 감시 스레드가 `/proc/self/status` 의 `RssAnon`(힙·익명 매핑) / `RssFile`(.so·pak) / `RssShmem`(드라이버 공유 매핑) / `Threads` 를 1 s 마다 — 어느 층이 느는지로 누수 범인을 가른다(리눅스 외 -1). 이 열로 Vulkan 누수 2건을 잡음: `diagnostics/2026-10-13_linux_rss_leak_vulkan.md`.
- **UGV 자세 6열**(맨 끝): `ugv_tilt_max, ugv_tilt, ugv_pitch, ugv_roll, ugv_angvel_max, ugv_speed_kmh` — 게임 스레드 매 프레임 샘플·1초 최대, UGV 없는 레벨·멈춤 줄은 빈칸. 상세 `vehicle/ugv/2026-10-12_ugv_flip_pawn_collision_and_self_righting.md` (d)절. 블랙박스 바이너리 레코드는 그대로.
- `health.csv` 끝 열 순서(현재, `TitanHealthRecorderSubsystem.cpp:511-513`): `…,stalled_s,swap_used_mb,uobjects,proc_cpu_cores,load_avg1,rss_anon_mb,rss_file_mb,rss_shmem_mb,threads,ugv_tilt_max,ugv_tilt,ugv_pitch,ugv_roll,ugv_angvel_max,ugv_speed_kmh`.

### 14.3 빌드·검증 체크리스트

1. 윈도우 에디터 빌드 → `-TitanHealthInEditor` 로 PIE 또는 Standalone 실행 → `Saved/HealthLogs/<세션>/` 에 파일 4종 생성, 로그 `[Health] 블랙박스 기록 →`.
2. `health.csv` 1초 1줄, fps·VRAM·RAM 값이 `stat unit`/작업 관리자와 대략 맞는지.
3. 조이스틱 버튼 → `events.log` 에 `KEY n = GenericUSBController_ButtonNN`, 디코더 CSV 의 `last_key_name`.
4. `titan.Health.HitchMs 20` 으로 낮춰 HITCH + `snapshots/blackbox_*_hitch.bin` 생성 확인 → 원복.
5. 멈춤: 디버거 브레이크로 게임 스레드를 3초 이상 세웠다 풀기 → `STALL 시작/지속/해제` 줄, CSV 의 `stalled_s` 열이 멈춘 동안 계속 찍히는지. (`pause` 콘솔은 게임만 멈추고 프레임은 돌아서 테스트가 안 된다. 패키지에서는 `debug hang` — 영구 정지라 이어서 6번으로.)
6. 즉사: 작업 관리자/`kill -9` 로 강제 종료 → `blackbox.bin` 을 디코더로 열어 종료 직전 프레임까지 있는지.
7. **리눅스 패키지 빌드**(clang `-Werror`) 통과 확인 + 리눅스에서 5·6번 반복.
8. 성능: `stat TitanHealth` 로 `Frame record` 비용 확인, `titan.Health.Enabled 0/1` A/B.
9. 12시간 무인 실행에서 `health.csv` 크기(예상 ~10 MB)와 세션 30개 초과 시 삭제 동작.
