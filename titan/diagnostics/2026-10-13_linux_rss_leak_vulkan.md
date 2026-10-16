# 리눅스 호스트 메모리 누수 2건 (Vulkan RHI) — 원인·우회

2026-10-13 / 완료(10-16 갱신) / 리눅스 호스트 RSS ≈40 MB/분 증가는 UE 5.8 Vulkan RHI 엔진 버그 2건(병렬 렌더패스 UE-392081 + 스테이징 버퍼 추적 배열) — `LinuxEngine.ini` cvar 2줄로 40 → 0.8 MB/분; 잔여 0.5 MB/분 중 MetaSound 파라미터 라우터 누수를 프로젝트 플러그인 사본 패치로 제거해 0.21 MB/분(6절).

제출용 보고서: `diagnostics/2026-10-13_linux_memory_leak_report.md`.

관련: 블랙박스 설계 `diagnostics/2026-10-01_blackbox_health_recorder_design.md`(이 누수를 처음 잡은 `health.csv`), LIG 9/28 사고 `diagnostics/2026-10-01_lig_0928_incident_log_analysis.md`.

## 1. 증상

- 2026-10-12 2-PC 실행(`D:\linuxlog_1012_1`): **리눅스 호스트 RSS 4.4 → 9.2 GB, ≈40 MB/분 선형 증가.** UObject 수·액터 수·VRAM 은 일정.
- 윈도우(D3D12) 클라이언트는 증가 없음 → 리눅스/Vulkan 쪽.
- RTSP/GStreamer 는 무관 — 접속 클라이언트가 없으면 push 전에 return 한다(`RtspServerSubsystem.cpp:462-466`).
- 블랙박스 `health.csv`(RAM 열)로 처음 잡혔다. LIG "메모리 누수로 뻗음" 제보(9/28)의 유력한 설명.

| 단계 | 조건 | 증가율 |
|---|---|---|
| 원래 | 기본 설정 | ≈40 MB/분 |
| 누수 1 우회 후 | `r.Vulkan.RHIThread=1` | 11.5 MB/분 |
| 누수 2 우회 후 | + `r.Vulkan.MemoryBacktrace=0` | **0.8 MB/분**(60분) / 0.5 MB/분(15.5시간) |
| 누수 3 수정 후 | + `Plugins/Metasound/` 패치(6절) | **0.21 MB/분**(21.2시간, 6시간 이후) |

## 2. 누수 1 — Vulkan 병렬 렌더패스 (Epic UE-392081)

**원인(엔진 소스)**: 동적 렌더링(`VK_KHR_dynamic_rendering`) 활성 시 병렬 렌더패스를 열 때마다 `new FVulkanDynamicRenderingInfo()`(`Engine/Source/Runtime/VulkanRHI/Private/VulkanContext.cpp:590`) 하는데, 닫을 때는 바깥 `FVulkanParallelRenderPassInfo` 만 delete(`:667`) 하고, 그 구조체에 소멸자가 없다(`VulkanContext.h:49-55`) → 패스당 ≈0.9 KB 잔류.

- Epic 이슈 **UE-392081**(9/22 Ready for QA), **5.8.3(9/22 출시)에도 미포함**.
- 포럼: https://forums.unrealengine.com/t/ue-5-8-2-vulkan-memory-leak-consumed-114gb-ram-and-killed-by-oom/2764868

**우회**: `Config/Linux/LinuxEngine.ini` `[SystemSettings] r.Vulkan.RHIThread=1`
- RHI 스레드 1개 → `GRHIGlobals.ParallelRenderPasses.Supported=false`(`VulkanRHI.cpp:979-985`) → 누수 코드 자체가 실행되지 않음.
- RHI 초기화 때만 읽힘. `[SystemSettings]` 는 RHIInit 전에 적용된다(`LaunchEngineLoop.cpp:2814` `GSystemSettings.Initialize` → `:3171` RHIInit). 콘솔로는 못 바꿈.
- 일시 원복: 실행 인자 `-dpcvars=r.Vulkan.RHIThread=2`.
- 대안 `r.Vulkan.AllowDynamicRendering=0`(ReadOnly)은 렌더 이상 위험이 있어 **채택 안 함**.

**실측**(`D:\linuxlog_1012_2`, 85분): 증가 40 → 11.5 MB/분, fps 57.7 → 58.0(변화 없음), RHI 스레드 최대 7.1 → 12.4 ms.

## 3. 누수 2 — 스테이징 버퍼 추적 배열 (Development 빌드 전용)

### 3.1 분석 과정

1. **LLM**(`-LLMCSV`, `D:\linuxlog_1013_1`, 143분): `RHIMisc` +8.2 MB/분(하위 태그 0), `SceneRender` 평평, `Untracked` +0.3 → RHI 쪽 할당이지만 LLM 만으로는 어디인지 모름.
2. **메모리 트레이스**(`-trace=memory`, `Desktop\linuxlog_1013_2`, utrace 16.6 GB, 16분51초 지점부터 패킷 손상): 교전 중 7~16 MB/분 vs 재시작 확인창 대기 중 ≈2 MB/분 → **교전 부하에 비례.**
3. **Insights ExportAllocs**(Rule Growth A\*B\*, 390~990초): 남은 할당 113 MB/10분 = 11.3 MB/분(실측 11.5 와 일치). 1위 = LLM 태그 `RHIMisc`, **80 바이트, 약 21만 개**, 할당 스레드 Foreground Worker #0/#1.
4. **콜스택**(Insights GUI 주소 → llvm-symbolizer, 4절):
   ```
   FScheduler::WorkerMain → FTaskBase::TryExecuteTask
   → FRDGBuilder::Execute (CommandListSetupTask) → FRDGBuilder::SubmitBufferUploads
   → FVulkanDynamicRHI::RHILockBuffer → FVulkanBuffer::Lock
   → VulkanRHI::FStagingManager::AcquireBuffer → operator new[]
   ```
   반환주소 `0xD1EAF1E` = `0xd1eaf19` 의 `call operator new[]` 직후임을 디스어셈블로 확인 — `new FStagingBuffer`(`_Znwm`, `0xd1ead20`)가 **아니다**.

### 3.2 원인(엔진 소스)

- `AcquireBuffer` 가 `VULKAN_FILL_TRACK_INFO(Track, ...)` 를 부르고, `VulkanTrackFillInfo`(`VulkanMemory.cpp:358-370`)는 `r.Vulkan.MemoryBacktrace`(기본 10, `VulkanMemory.cpp:128`, `ECVF_ReadOnly`) > 0 이면 `new uint64[10]`(80 B, `:362`) 로 호출 스택을 저장한다.
- `~FStagingBuffer`(`:4361-4364`)와 `Destroy`(`:4393-4399`)에 `VULKAN_FREE_TRACK_INFO` 가 없다 → 스테이징 버퍼 생성·삭제마다 80 B 가 남는다. 렌더 그래프 버퍼 업로드량(교전 이펙트)에 비례.
- `VULKAN_MEMORY_TRACK` 은 `!UE_BUILD_SHIPPING` 에서만 1(`VulkanMemory.h:18-22`) → **Shipping 빌드는 해당 없음.** 현재 배포는 Development 라 해당.

**확인**: Development 콘솔 `r.Vulkan.DumpStagingMemory`(`D:\linuxlog_1013_3`) 두 번 → Used 71 / Pending 3~4 / Free 224~238 로 일정 → 버퍼 객체는 정상 삭제되고 **추적 배열만 남는다.**

### 3.3 우회와 검증

- `LinuxEngine.ini` `[SystemSettings] r.Vulkan.MemoryBacktrace=0` — 호출 스택 대신 파일명/줄번호 포인터만 저장(할당 없음). 잃는 것: Vulkan 메모리 덤프 출력의 호출 스택뿐.
- 2026-10-13 ini 반영, **P4 체크아웃 상태(서밋은 사용자)**. ReadOnly cvar 라 ini 또는 `-dpcvars=` 로만 바뀐다.
- **검증**(`D:\linuxlog_1013_4`, `-dpcvars=r.Vulkan.MemoryBacktrace=0`, 약 60분, 교전 7사이클 Complete, 57.8 fps): 워밍업 후 `rss_anon` 3181 → 3224 MB, **회귀 기울기 0.80 MB/분**, 스왑 0, UObject 8.4~8.8만 일정.

### 3.4 남은 일

- ~~ini 반영본으로 패키징 → 8~12시간 밤샘 실행으로 최종 확인.~~ → 완료(2026-10-13~14, 15.5시간·113사이클, ini만으로 `Set CVar [[r.Vulkan.MemoryBacktrace:0]]` 적용 확인, ≈0.5 MB/분, 스왑 0). 잔여분 분석은 6절.
- 엔진 업그레이드 시 두 우회 모두 재검토(UE-392081 수정 포함 여부, 스테이징 추적 해제 추가 여부). 수정 버전이면 ini 줄 제거.

## 4. 분석 방법 기록 (재사용용)

### 4.1 메모리 트레이스 수집

- 리눅스 실행 인자: `-trace=memory -tracefile=/home/grapicar/titan_mem.utrace`
- 대가: 7 fps 로 느려지고 40분에 16 GB. 길게 돌리면 패킷 손상(이번엔 16분51초부터)이 생길 수 있다.

### 4.2 심볼 — 시간 낭비하지 말 것

- **런처 엔진은 DWARF 가 줄정보 없는 빈 CU 뿐이라 Insights(symslib)가 엔진 함수 이름을 못 푼다.** 프로젝트/플러그인 코드만 풀린다.
- 실패한 우회(재시도 금지): 심볼 경로(`UE_INSIGHTS_SYMBOL_PATH`), `.debug` 사본, strip, 빈 debug_info, UNC 경로 지연(분석 스레드까지 막혀 58분 걸림).
- **해결 경로**: Insights GUI 는 해석 실패 프레임을 `titan_example!0x주소 (Not loaded/NotFound)` 로 보여 준다 → 그 주소를 직접 푼다.
  ```
  C:\UnrealToolchains\v26_clang-20.1.8-rockylinux8\x86_64-unknown-linux-gnu\bin\llvm-symbolizer.exe ^
      --obj=Binaries\Linux\titan_example.debug -C -f 0x주소
  ```
  엔진 함수도 정상으로 풀린다. 디스어셈블은 같은 폴더 `x86_64-unknown-linux-gnu-objdump.exe -d --start-address=… --stop-address=…`.

### 4.3 Insights GUI 절차

1. Memory Insights 탭 → Investigation 패널 Rule = **Growth (A\*B\*)**(Active Allocs 는 마커 1개뿐이라 증가분을 못 봄).
2. A/B 시간 마커 지정 → Run Query.
3. Allocs Table 상단 "Click to add or edit groupings" 에서 **Unique Values - LLM Tag → Unique Values - Size → By Alloc Callstack** 순서로 추가.
4. All > 태그 > 크기 > 콜스택 트리를 Count 큰 쪽으로 펼친다(루트 = 스레드 진입점, 아래로 내려갈수록 할당 지점).

### 4.4 자동 내보내기(심볼은 안 풀리지만 태그·스레드·크기 집계에 유용)

```
UnrealInsights.exe -OpenTraceFile="…" -AutoQuit ^
  -ExecOnAnalysisCompleteCmd="MemoryInsights.ExportAllocs -Rule=AaBf -TimeA=390 -TimeB=990 -Output=… -Columns=Size,Tag,AllocThread,AllocCallstack"
```
- **PowerShell `Start-Process` 로 `-X="…"` 형태로 넘길 것.** Git Bash 는 인자 전체를 따옴표로 감싸서 FParse 가 무시한다.
- 명령은 분석 완료 즉시 실행되고 심볼 해석을 기다리지 않는다. Insights 가 RAM 을 20 GB 가까이 쓴다.
- `TimingInsights.ExportThreads` 로 스레드 id → 이름(이번 트레이스: 6·10 = Foreground Worker #0·#1, 62 = RenderThread).

### 4.5 LLM

- `-LLMCSV` → `Saved/LLM/*.csv`.
- RHI 스레드 `Run()` 전체가 `LLM_SCOPE(ELLMTag::RHIMisc)`(`RenderingThread.cpp:200`)이고 태스크는 띄운 쪽 태그를 상속 → Foreground Worker 의 할당도 `RHIMisc` 로 잡힌다. Vulkan 자체 스코프도 대부분 `RHIMisc`(`VulkanLLM.h:52-62`). 즉 `RHIMisc` 증가만으로는 범인을 못 좁힌다 — 콜스택이 필요.

### 4.6 기각한 가설

- GPU 프로파일러 `ProcessEvents`(EOP 태스크, `VulkanRHI.cpp:1238-1246`) — 콜스택으로 기각.

## 5. 리눅스에서 메모리가 한계에 닿으면 (사용자 질문 답변 요약)

1. 스왑(Ubuntu 기본 약 2 GB) 소진 → **스래싱**: 수 초~수 분 멈춤, 엔진 로그 공백(LIG 9/28 "10분 멈춤"과 같은 패턴).
2. 이어서 systemd-oomd 또는 커널 OOM killer 가 **SIGKILL** — 크래시 리포터·종료 로그 없음.
3. 확인 방법:
   - `journalctl -k | grep -i -E "out of memory|oom"`, `journalctl -u systemd-oomd`
   - 블랙박스 `health.csv` 의 `rss_anon_mb` / `swap_used_mb` / `stalled_s`
   - `blackbox.bin` 은 mmap 이라 SIGKILL 에도 마지막 프레임까지 보존된다.
- 윈도우는 페이지파일을 키운 뒤 커밋 한도에 닿으면 언리얼 OOM 크래시(크래시 리포트 남음).

## 6. 2026-10-14~16 잔여 0.5 MB/분 분해와 MetaSound 수정

### 6.1 출발점

- 누수 1·2 우회 반영 패키지 밤샘(2026-10-13~14): 15.5시간·113 시나리오 사이클, `rss_anon` 기울기 **≈0.5 MB/분** 잔여.
- 증가는 교전 중에 일어나고 재시작 구간이 아님: 108사이클 평균 재시작 창 0.36 MB vs 교전 중 4.5 MB/사이클.

### 6.2 방법

1. **memreport bin 비교** — Development 콘솔 `memreport -full` 을 19분·3.6시간(같은 프로세스), 17시간(다른 프로세스) 시점에 찍고 FMallocBinned2 bin별 "Total Allocated Mem" 을 비교.
   - **64 B bin 만 선형**: 9.5 MB(19분) → 37.8 MB(3.6h) → 155 MB(17h), ≈8.7~9.1 MB/h. 나머지 small bin 은 2~36 MB.
   - Large pool 도 같은 구간 +108 MB — 아래 맵 배열 추정치(약 240만 항목 × 40 B ≈ 96 MB)와 맞음.
   - 콜스택 없이 크기만으로 범위를 좁히는 데 가장 싸고 빨랐다(트레이스 없이 정상 속도 실행).
2. **LLM 태그 곡선** — `-LLMCSV` 3.5시간: `Audio/MetaSound` 12 → 51 MB(≈0.2 MB/분), `Meshes/SkeletalMesh` 38 → 60, `Niagara` 11 → 27, `Shaders` 261 → 275, 나머지 평평.
3. **DumpLLM** — 콘솔 `DumpLLM`(인자 없음)은 정상 동작, 태그별 현재값 확인용.
4. **`-LLMTAGSETS=Assets,AssetClasses` 는 쓸모없음(재시도 금지)**: 런처 엔진 CSV writer 가 태그셋 열을 쓰지 않고, `DumpLLM -Assets` / `-AssetClasses` 는 헤더만 출력. 에셋 단위 귀속은 이 경로로 안 된다.
5. 메모리 트레이스(`Audio/MetaSound` 태그 잔존 할당 확인)와 Insights GUI 분석 절차는 4.1~4.4 그대로.

### 6.3 누수 3 — MetaSound 파라미터 라우터 (엔진 소스)

`Engine/Plugins/Runtime/Metasound/Source/MetasoundEngine/Private/MetasoundSource.cpp`
- `UMetaSoundSource::CreateSoundGenerator`(`:1257-1258`)와 `CreateParameterTransmitter`(`:1730-1731`)가 `FParameterRouter::FindOrCreateDataChannel`(`:165-201`)을 호출 → `TSortedMap DataChannels` 에 키 (AudioDeviceID, InstanceID) 로 `Add`. **InstanceID 는 재생마다 새로 발급**되고 플러그인 전체에 `Remove` 가 없다.
- 라우터는 함수 static(`GetParameterRouter`, `:1940-1945`, private) → 레벨 리로드로도 안 비워짐.
- 재생 1회당 맵 항목 40 B + `MakeShared` 블록 56 B(TWeakPtr 가 컨트롤 블록을 붙잡음, 64 B bin 행) 잔류.
- 메모리 트레이스 확인: `Audio/MetaSound` 태그에 배열 318,400 B(= 7,960 항목 × 40 B 정확히) + 56 B 블록 수천 개.
- 공개된 Epic 보고 없음.

### 6.4 누수 아님으로 판정

- **Niagara**: AutoRelease 컴포넌트 풀. NiagaraComponent 수 309 → 611 → 804, 증가 속도 92/h → 15/h 로 감속. 풀은 180초 미사용 컴포넌트를 정리(`NiagaraComponentPool.cpp:17`).
- 그 외 UObject 클래스별 개수 일정, SkeletalMeshComponent 125개 고정.

### 6.5 수정 (2026-10-15) — 프로젝트 플러그인 사본

- 엔진 MetaSound 플러그인(`Metasound.uplugin`, `Config`, `Content`, `Resources`, `Source`; `Binaries`/`Intermediate` 제외)을 프로젝트 `Plugins/Metasound/` 로 복사.
- 같은 이름이면 프로젝트 플러그인이 엔진 것보다 우선: `Engine/Source/Runtime/Projects/Private/PluginManager.cpp:1280`, UBT `Plugins.cs:275-276`.
- 변경은 `MetasoundSource.cpp` 의 `[TITAN PATCH 2026-10-15]` 1곳뿐: `Add` 1,024회마다 TWeakPtr 가 만료된 항목을 제거. 만료 항목은 원래도 새 채널로 덮어써질 뿐이라 동작 동일. 헤더 무변경.
- MetaSound 의존 엔진 플러그인(Harmonix, AudioModulation, MetasoundExperimental, Subsonic, TechAudioTools)은 이 프로젝트 빌드에 없음.
- 재적용 절차: `Plugins/Metasound/TITAN_PATCH.md` — 엔진 업그레이드 시 수정 여부 확인 → 고쳐졌으면 폴더 삭제, 아니면 새 엔진 플러그인 재복사 + 패치 재적용.

### 6.6 검증 (`D:\Saved_1015`, `D:\Saved_1016`, 2026-10-15~16, 21.2시간, 스왑 0)

- 로그 `Mounting Project plugin Metasound` 확인.
- 64 B bin: 7.0 MB(8분) → 7.8 MB(3.3h) → 9.4 MB(19h) — 15.7시간 +1.6 MB(이전 ≈+140 MB).
- LLM `Audio/MetaSound` 2.5 MB 평평.
- `rss_anon` 기울기(이전 → 이후): 10~90분 0.65 → 0.45, 90~180분 0.69 → 0.36, **6시간 이후 0.47 → 0.21 MB/분**(360~600, 600~900, 900~끝 모두 ≈0.21로 안정). rss 3200 MB(2h) → 3486 MB(20h).
- 전체 경과: 40 → 11.5(RHIThread=1) → 0.8(MemoryBacktrace=0, 1시간 측정) / 0.5(15.5시간) → **0.21 MB/분**. 24시간 ≈0.3 GB, 1주일 ≈2.1 GB.

### 6.7 남은 ≈0.21 MB/분 (미식별, 낮은 우선순위)

- 밤샘 small bin 증가 합계 +182 MB/15.7h: 5,952 B bin +49 MB(≈3.1 MB/h), 9,360 B bin +39 MB(≈2.5 MB/h), 700~3,000 B bin 들 각 ≈0.7~0.8 MB/h.
- LLM 상 계속 오르는 태그: `Meshes/SkeletalMesh`, `Shaders`.
- 7 fps 메모리 트레이스(41 GB, 앞 18 GB 분석, 480~900초 창)는 신호가 너무 약해 위 증가를 귀속하지 못함 (추정: 7 fps 면 정상 ≈58 fps 대비 약 1/8 속도라 교전 비례 증가도 같이 줄어듦).
- 다음 후보: 정상 속도 + LLM 으로 기능 A/B(끄고 켜서 기울기 비교). 크기상 우선순위 낮음.
