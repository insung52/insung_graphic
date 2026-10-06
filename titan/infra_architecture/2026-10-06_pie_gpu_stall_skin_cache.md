# 에디터 PIE GPU 급저하 — GPU 스킨 캐시 회피

2026-10-06 / 완료(회피 적용, 원인 미확정) / 에디터 PIE GPU 급저하(28→80~630 ms)의 범인은 GPU 스킨 캐시 compute 디스패치 — r.SkinCache.Mode=0(DefaultEngine.ini 공통)으로 회피, 바운드 아토믹 가설 기각, VRAM 압박 하 D3D12 상주 관리 추정[B]

> 신뢰도 표기: **[A]** = file:line·로그·실측 근거 있음, **[B]** = 추정.
> 선행 문서: [2026-10-06_s6_verification.md](2026-10-06_s6_verification.md) §3 F3 (GPU 급저하 최초 발견·시계열).

---

## 0. 요약

- 느린 프레임의 GPU 시간 거의 전부가 **GPU 스킨 캐시 skinning compute 디스패치**였다(633.8 ms 중 602.1 ms) [A]. 같은 작업이 20~30배 느려졌고, 나머지 패스는 정상 범위였다 [A].
- `r.SkinCache.Mode 0` → 즉시 정상, `1` → 다시 느려짐(오전 5/5 재현) [A].
- **조치**: `Config/DefaultEngine.ini` `[ConsoleVariables]`에 `r.SkinCache.Mode=0` 추가(전 플랫폼 공통, P4 체크아웃·미서브밋) [A]. 꺼도 되는 근거는 §5 (HWRT 꺼짐, RecomputeTangents 전부 Off, Mesh Deformer·클로스 없음). 리눅스 패키지에서 Mode 0/1 성능 차이 없음(사용자 실측) [A].
- **원인은 미확정**: 바운드 아토믹 가설은 실측으로 **기각** [A]. 유력 가설은 VRAM 압박(사용률 ~96%) 하에서 D3D12 상주 관리(residency)가 스킨 캐시 버퍼를 내쫓고/다시 올리는 과정의 대기 [B].
- 남은 것: 에디터 재시작 후 값 0 확인 + 장시간 PIE 무저하 검증, HWRT 도입 시 재검토.

---

## 1. 증상

- Windows 에디터 PIE(New_kadex_0811)에서 fps가 1~2까지 떨어짐. 세 가지 양상:
  1. 시작 직후부터 느림
  2. 30+ fps로 잘 돌다가 1분쯤에 갑자기 떨어짐
  3. ~50 ms 딥이 반복됨
- PIE를 재시작하면 일시적으로 회복된다.
- 사용자 진술: **리눅스 패키지 빌드에서는 한 번도 본 적 없음**.
- PC: RTX 5060 8 GB (드라이버 610.88), Core Ultra 5 225, RAM 31 GB.

---

## 2. 측정 · 근거 [A]

### 2-1. 스레드 분해 (`stat unit` / `stat gpu`)

| 항목 | 값 | 해석 |
|---|---|---|
| Frame / GPU | 665 / 663 ms | Frame ≈ GPU → GPU 바운드 |
| 렌더 스레드 `SceneRenderBuilder_Render` | Inclusive 664.89 / Exclusive 2.42 ms | 렌더 스레드 자체는 GPU 대기만 함 |
| `stat gpu` Graphics Queue Total | 183.55 ms | 보이는 행 합은 ~27 ms뿐, 나머지는 숨은 "24 more stats" |
| Compute 큐 `FXSystemPreRender`(Niagara GPU) | 157 ms 대기 | |

### 2-2. ProfileGPU — 오전 느림/빠름 비교

| 프레임 | 전체 | 스킨 캐시 skinning 디스패치 | 그 외 |
|---|---|---|---|
| 느린 프레임 | 633.8 ms | **602.1 ms** | 31.3 ms |
| 빠른 프레임 | 29.5 ms | 1.1 ms | 28.2 ms |

- 같은 작업이 20~30배 느려짐: `soldier_T` 25개 디스패치 평균 0.02 → 0.40–0.64 ms.
- `new_enemy_T` Chunk2(88,628 버텍스)는 15회 호출 합계 408 ms.

### 2-3. ProfileGPU 4회 덤프 — 오후 (`r.SkinCache.DynamicMeshBounds=0` 상태)

| 덤프 | 전체 | skinning | 주요 항목 |
|---|---|---|---|
| `pg2_a_107ms` | 107.8 ms | 69.3 ms | `SK_UGV_0901` 64.1 ms / 7회 (적 숨김 상태) |
| `pg2_b_20ms` | 20.2 ms | 0.2 ms | — (정상) |
| `pg2_c_83ms` | 83.5 ms | 47.4 ms | `soldier_T` 41.6 ms |
| `pg2_d_330ms` | 329.9 ms | 275.7 ms | new_enemy_T 102 / soldier_T 87 / UGV 56 / 소총류 29 ms |

- 그 외 패스는 전 덤프에서 정상 범위 20–54 ms 유지.
- → 특정 메시가 아니라 **스킨 캐시를 쓰는 모든 스켈레탈 메시**가 같이 느려진다.

### 2-4. nvidia-smi 1초 로그 · D3D 메모리

| 항목 | 느린 상태 | 정상 상태 |
|---|---|---|
| 그래픽 클럭 | 2880–2895 MHz (최대) | — |
| 사용률 | 100% | — |
| **전력** | **46–58 W** | 95–105 W |
| 카드 전체 VRAM | 7.4–7.7 / 8.15 GB | — |

- 클럭 최대 + 사용률 100%인데 전력이 절반 → 연산이 아니라 **스톨/대기** 상태.
- `stat D3DMemory`(느린 상태): Demoted Video Memory 0, Used Video Memory 6210 MB / Budget 6479 MB.
- PCIe 송수신(`nvidia-smi dmon`)은 느림/정상을 구분하지 못함(오히려 느린 구간이 더 낮음) → 판별 지표로 못 씀.

### 2-5. 상관관계 · 토글 재현

- 여러 세션에서 `RevealEnemies` 스텝(적 표시) 후 5 s 안에 저하(예: 11:15:52 reveal → 11:15:57 느려짐) [A].
- 그러나 11:30 PIE에서는 적이 숨김 상태인데도 3분 넘게 느렸다 → reveal은 **트리거 중 하나**일 뿐 [A].
- 에디터 재시작 직후 첫 PIE는 대체로 괜찮고, PIE를 반복할수록 나빠지는 경향 [B].
- `r.SkinCache.Mode` 토글:
  - 오전: 0 → 즉시 정상(전력 95–110 W 복귀), 1 → 다시 느려짐 (5/5) [A].
  - 오후 교전 중: 1 → 느려졌다가 한참 뒤 회복되는 경우도 있음, 0 → 항상 즉시 정상 [A].
- `r.SkinCache.PrintMemorySummary`: 스킨 캐시 총 ~111 MB, 전 엔트리 RecomputeTangent=Off, new_enemy_T 1구당 ~4.3 MB(15구 ~64 MB) [A].

---

## 3. 가설과 기각 · 현황

### H-바운드 아토믹 — 기각 [A]

- 내용: 스킨 캐시 dynamic mesh bounds가 웨이브마다 공유 버퍼의 같은 주소에 아토믹 6회
  (`GpuSkinCacheComputeShader.usf:372-381`, `WaveOpUtil.ush:104-108`).
  cvar는 `GPUSkinCache.cpp:205-213`, 기본값 2 = Nanite 플랫폼에서 ON, ReadOnly.
- Mode 토글은 엔트리 버퍼는 재생성하지만 `DynamicMeshBoundsBuffer`는 재생성하지 않음
  (`GPUSkinCache.cpp:987-989, 1098, 2509-2531`) — "토글하면 나아진다"는 관찰과 부분적으로 맞아 보였음.
- 시험: `Config/Windows/WindowsEngine.ini`에 `r.SkinCache.DynamicMeshBounds=0` 추가.
- 결과: 그 상태에서도 재발(§2-3 오후 덤프 4개) → **기각**. 파일은 **p4 revert로 원복**(원본과 동일).

### H-VRAM 압박 / D3D12 상주 관리 — 유력, 미확정 [B]

엔진 소스로 확인한 사실 [A]:

- 엔진의 Demoted 값은 단지 `Used − Budget`이고 Used > Budget일 때만 셈
  (`RHICore/Private/DXGIUtilities.cpp:115-124`) → **"Demoted 0"은 전부 VRAM에 있다는 증명이 아니다.**
- 상주 관리자는 기본 ON (`D3D12RHI.h:29`, `D3D12Adapter.cpp:30-35` `D3D12.ResidencyManagement=1`),
  리소스는 내쫓긴 상태로 시작 (`:37-44` `D3D12.ResourcesStartResident=0`).
- 압박도(local 사용량/예산) > 70%면 eviction 유예 시간이 60 s → 1 s 쪽으로 줄어듦
  (`ThirdParty/Windows/D3DX12/include/d3dx12residency.h:1630-1649`, 상수 `:693-695`) —
  실측 96%면 약 9 s.
- 안 쓰인 오브젝트는 evict(`:1317-1328`); 다시 상주시킬 때 여유 공간을 local+non-local 합산으로
  계산하고(`:1338-1347`) 실제 배치 위치는 OS(VidMm)가 결정. 공간이 모자라면 CPU가 GPU를 기다리고
  더 evict(`:1430-1446`).
- 스킨 캐시·엔진 어디에도 VRAM 상태에 따라 성능을 스로틀하는 코드 없음: `GPUSkinCache.cpp`에
  예산 조회 없음, 엔진은 예산 초과 시 경고만 표시(`UnrealClient.cpp:850`). 텍스처 풀은 시작 시
  예산의 90%로 설정(`WindowsD3D12Device.cpp:1849-1866`).

미증명 [B]:

- 스킨 캐시 버퍼가 실제로 VRAM 밖에 배치됐는지 여부.
- 미실시 시험: `D3D12.ResidencyManagement=0`(ini, ReadOnly), 에디터 닫은 -game vs 열린 상태 A/B,
  WPR(GPU 프로파일) VidMm 트레이스(`wpr.exe`/WPA 이 PC에 있음).
- 정황: 검증 R01 등에서 에디터가 열린 상태의 -game(별도 프로세스)에서도 저하 발생(GPU 165 ms) —
  VRAM 압박 가설과 부합 [B].

### 소스로 기각한 것 [A]

- 업로드 힙 배치: 스킨 캐시 버퍼는 DEFAULT 힙 (`GPUSkinCache.cpp:350,358`, `D3D12Buffer.cpp:193-221`).
- 매 프레임 재할당: 할당은 엔트리 생성/LOD 변경 때만 (`GPUSkinCache.cpp:2050-2066, 2129-2155, 2404-2428`).

### "적을 처음부터 보이게 하면 해결된다" — 가능성 낮음 [A 관찰]

- 적 숨김 상태에서도 발생하므로(§2-5) 해결책이 아니다. (적 가시성은 사용자가 다른 세션에서 별도 논의 중.)

---

## 4. 엔진 코드 조사 결과 (file:line)

| 주제 | 위치 | 내용 |
|---|---|---|
| 바운드 아토믹 | `GpuSkinCacheComputeShader.usf:372-381`, `WaveOpUtil.ush:104-108` | 웨이브당 같은 주소 아토믹 6회 |
| DynamicMeshBounds cvar | `GPUSkinCache.cpp:205-213` | 기본 2(Nanite 플랫폼 ON), ReadOnly |
| Mode 토글 시 재생성 범위 | `GPUSkinCache.cpp:987-989, 1098, 2509-2531` | 엔트리 버퍼만, `DynamicMeshBoundsBuffer`는 유지 |
| 버퍼 힙 | `GPUSkinCache.cpp:350,358`, `D3D12Buffer.cpp:193-221` | DEFAULT 힙 |
| 버퍼 할당 시점 | `GPUSkinCache.cpp:2050-2066, 2129-2155, 2404-2428` | 엔트리 생성/LOD 변경 시만 |
| Demoted 계산 | `RHICore/Private/DXGIUtilities.cpp:115-124` | Used−Budget, Used>Budget일 때만 |
| 상주 관리 기본값 | `D3D12RHI.h:29`, `D3D12Adapter.cpp:30-35`, `:37-44` | ResidencyManagement=1, ResourcesStartResident=0 |
| eviction 유예 | `d3dx12residency.h:1630-1649`, `:693-695` | 압박 >70%에서 60 s→1 s |
| evict / 재상주 / 대기 | `d3dx12residency.h:1317-1328`, `:1338-1347`, `:1430-1446` | 미사용 evict, local+non-local 합산, 부족 시 CPU가 GPU 대기 |
| VRAM 기반 스로틀 부재 | `GPUSkinCache.cpp`(예산 조회 없음), `UnrealClient.cpp:850` | 경고만 |
| 텍스처 풀 | `WindowsD3D12Device.cpp:1849-1866` | 예산의 90% |
| RT 경로의 스킨 캐시 강제 | `SkinnedMeshComponent.cpp:2620` | `IsRayTracingEnabled()`일 때만 |
| 스킨 캐시 OFF 시 스키닝 | `SkeletalRenderGPUSkin.cpp:161-163` | 버텍스 셰이더에서 패스·뷰마다 재계산 |

---

## 5. 적용한 변경 (ini)

### 5-1. 변경 내용 [A]

- `Config/DefaultEngine.ini` `[ConsoleVariables]` 끝(81행)에 `r.SkinCache.Mode=0` 추가(주석 포함).
- 전 플랫폼 공통. P4 체크아웃 상태(서브밋 안 함).
- `r.SkinCache.CompileShaders=True`(RendererSettings, ~120행)는 유지 → 콘솔 `r.SkinCache.Mode 1`로 언제든 되돌릴 수 있음.

### 5-2. 꺼도 되는 근거 (소스·에셋) [A]

- HW 레이 트레이싱 꺼짐(`r.RayTracing=False`) — 스켈레탈 메시 RT 경로가 스킨 캐시를 강제하는 건
  `IsRayTracingEnabled()`일 때뿐(`SkinnedMeshComponent.cpp:2620`).
- RecomputeTangents 전부 Off.
- Mesh Deformer(Optimus) 에셋 없음, 클로스 에셋 없음.
- Niagara 스켈레탈 메시 DI는 스킨 캐시를 참조하지 않음(Niagara 소스 grep). 이를 쓰는 에셋은 `NiagaraExamples` 샘플뿐.

### 5-3. 끄면 달라지는 것 · 리눅스 실측

- 스키닝이 버텍스 셰이더로 이동, 패스·뷰마다 재계산(`SkeletalRenderGPUSkin.cpp:161-163`) [A].
- 우려: 메인 뷰 + 씬 캡쳐 5~7개만큼 스키닝 반복 [B].
- → **사용자가 리눅스 패키지 빌드에서 실측: Mode 0/1 성능 차이 없음** [A].

### 5-4. 병사 메시 LOD (관련 정보)

- 작업 전 [A]:
  - `/Game/SoldierLab/Characters/Ally/soldier_T` (BP_Soldier_Friendly): LOD 1개 / 24,136 버텍스
  - `/Game/Soldiers/New_enemy_soldiers/NewFolder/new_enemy_T` (BP_Soldier_Hostile): LOD 1개 / 111,083 버텍스
  - 복제본 `/Game/Soldiers/New_Soldiers/new_enemy_T`도 있으나 미사용.
- **이후 사용자가 LOD 작업 완료** (작업 후 LOD 수는 미측정).

### 5-5. 남은 확인

- 에디터 재시작 후 값이 0인지 확인 + 장시간 PIE에서 저하 없음 검증 — **미실시**.

---

## 6. 미실시 시험 · 후속

- **하드웨어 레이 트레이싱(Lumen HWRT)** 을 켜면 스켈레탈 메시가 다시 스킨 캐시를 필요로 함
  (`SkinnedMeshComponent.cpp:2620`) → 이 설정 재검토 필요. 사용자가 HWRT 검토를 별도로 예정.
- 원인 확정이 필요하면 다음 중 하나:
  - `D3D12.ResidencyManagement=0` 시험(ini, ReadOnly cvar)
  - 에디터 닫은 -game vs 에디터 열린 상태 A/B
  - WPR(GPU 프로파일) VidMm 트레이스 (`wpr.exe`/WPA 이 PC에 있음)
- 부수 발견: 검증 R01 등에서 에디터가 열린 상태의 -game(별도 프로세스)에서도 저하 발생(GPU 165 ms) —
  VRAM 압박 가설과 부합 [B]. 선행 문서 F3 참고.

---

## 7. 원본 데이터 경로

기준 폴더: `C:\working\works\kadex\titan_example\Saved\S6Verify\2026-10-06\`

| 파일 | 내용 |
|---|---|
| `profilegpu_slow.txt`, `profilegpu_fast.txt` | 오전 ProfileGPU 느림/빠름 (§2-2) |
| `pg2_a_107ms.txt`, `pg2_b_20ms.txt`, `pg2_c_83ms.txt`, `pg2_d_330ms.txt` | 오후 ProfileGPU 4회 (§2-3) |
| `gpu_clock_log.csv` | nvidia-smi 1초 로그 10:23–11:23 |
| `gpu_clock_log_2.csv` | nvidia-smi 1초 로그 11:2x–13:2x |
| `pcie_log.csv` | PCIe 송수신 (`nvidia-smi dmon`) |
| `PIE2-6_enabled0_slowdown.csv` | csvprofile, 저하 20분 시계열 |

- 에디터 로그: `Saved/Logs/titan_example.log`, `titan_example-backup-2026.10.06-02.22.54.log`
- 도구: `tools/gpu_logger.ps1`, `tools/pcie_logger.ps1`
