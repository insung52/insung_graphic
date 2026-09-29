# 성능 기준선(S0) — 실제 전시 구성 New_kadex_0811 첫 통합 계측

2026-09-29 / 완료 / 숲 정면은 GPU 바운드(21.8 ms), 교전은 게임 스레드 바운드(28.9 ms); 최대 단일 항목은 수동 `CaptureScene()`의 월드 렌더 상태 플러시 3.6 ms와 메인 뷰 Velocity 3.35 ms — 세션 S1~S5 인계표 포함.

> 신뢰도: **[A]** = 이 측정의 `stat dumpave/dumpmax/dumpframe`·`ProfileGPU` 로그 파싱 + 엔진/프로젝트 소스 file:line. **[B]** = 추정, 후속 세션이 판정.
> 이 문서는 최적화 세션 분리(S0 기준선 → S1 캡쳐/RTSP · S2 숲/렌더 · S3 병사 · S4 titan 게임플레이 · S5 메모리)의 출발점이다. 코드·에셋 변경 없음.

---

## 0. 한 장 요약

| 무엇 | 결과 | 근거 |
|---|---|---|
| M1 숲 정면(교전 전) | 프레임 **23.9 ms**, **GPU 바운드**(GPU 21.8 · GT 작업 ≈ 20 · RT 23.9 중 RTSP 인코드 12.0) | [A] 2절 |
| M2 교전 피크 | 프레임 **28.9 ms**, **게임 스레드 바운드**(RT 는 13.8 ms 대기) | [A] 2절 |
| M3 재시작 직후 | 프레임 **20.0 ms**(GT idle 2.3) | [A] 2절 |
| GT 최대 덩어리 | 병사 **7.3(M1) / 12.5(M2)** · UGV `QuadCam` 틱 **3.6~3.9** · 숲 스키닝 틱 1.6 | [A] 3절 |
| 발견 A | 수동 `CaptureScene()` 이 `World->SendAllEndOfFrameUpdates()` 를 불러 **프레임 첫 캡쳐가 월드 전체 렌더 상태 갱신을 틱 중간에 떠맡음**(3.4~3.7 ms) | [A] 소스 · 순수 추가비용은 [B] · 4절 |
| 발견 B | 메인 뷰 GPU 17.4 ms 중 **Velocity 3.35 · 비Nanite BasePass 3.4 · VSM 2.7 · DynamicWind 스키닝 1.43** — 09-28 Nanite/스키닝 숲 이후 첫 측정 | [A] 5절 |
| 병사 변화 | 09-21 `L_SoldierScenario` 대비 **Engagement 0.22 → 2.37 ms(≈ 10배)** | [A] 수치 · 원인 [B] 3절 |
| 메모리 | 재시작 1회로는 누수 판정 불가(M1 이 교전 전이라 워밍업 섞임) → 1회차 vs 5회차 재측정 필요 | 7절 |

---

## 1. 측정 조건

| 항목 | 값 |
|---|---|
| 실행 | `UnrealEditor.exe titan_example.uproject -game -log -noailogging`(비쿡 Standalone, 에디터 닫음) — `-noailogging` 은 "PROFILING WITH AI LOGGING ON!" 배너 조건 `GEngine->bDisableAILogging`(엔진 `UnrealEngine.cpp:3304` · `:13672`)을 끈다 |
| 진입 | `kadex_lobby` → `New_kadex_0811?Listen?Axis=UGV?Demo=1`(UGV 호스트, 데모) |
| 시각 | 2026-09-29 11:16~11:30 |
| 병사 | 40(아군 25 / 적 15) |
| 시점 | M1 숲 정면·교전 전 → M2 교전 피크 → M3 재시작 직후 |
| 명령 | 시점마다 `stat dumpave -num=120 -ms=0.1` · `stat dumpmax -num=120 -ms=0.5` · `stat dumpframe -ms=0.05` ×2, M1 만 `ProfileGPU`, M1·M3 `memreport -full` |
| 원본 | 로그 `Saved/Logs/titan_example-backup-2026.09.29-02.34.40.log`(세션 스크래치패드에 `s0.log` 사본) · memreport 2개 `Saved/Profiling/MemReports/New_kadex_0811-WindowsEditor-09.29-11.18.51/` |

**누락**: `stat unit` 스크린샷 · M2/M3 `ProfileGPU` · PC 사양 · 2-PC(자체방호축) 측정. ⚠ GPU 프로파일의 메인 뷰가 **1920x1080 한 개**라 평소 듀얼 모니터 구성이 아니었을 수 있다(확인 필요).

---

## 2. 시점별 스레드 판정 [A]

| 시점 | 프레임 | GT | RT | GPU | 판정 |
|---|---|---|---|---|---|
| M1 숲 정면 | 23.9 | 작업 ≈ 20(idle 3.9) | 23.9 = `RtspStreamEncodeFrame` 12.0(4~5회) + `SceneRenderBuilder_Render` 11.35 | 21.8 | **GPU 바운드** |
| M2 교전 | 28.9 | ≈ 28 | 13.8 ms 대기 · RTSP 인코드 2.2 | (미측정) | **GT 바운드** |
| M3 재시작 후 | 20.0 | idle 2.3 | RTSP 인코드 7.5 | (미측정) | GPU 쪽 [B] |

`RtspStreamEncodeFrame` 은 렌더 스레드의 CPU 일이 아니라 **NVENC 출력 대기 = GPU 대기**로 판단한다 **[B]** — 근거: 같은 스코프가 GT 바운드인 M2 에서 2.2 ms 로 줄었다(GPU 여유가 생기면 대기가 사라짐). 인코더는 `Private/Windows/NvencD3D12Encoder.cpp:280/284` 에서 NVENC 샘플 `Encoder->EncodeFrame` 을 렌더 스레드에서 동기 호출한다.

---

## 3. 게임 스레드 — 소유자별 (120프레임 평균) [A]

| 소유 | M1 | M2 | 세부 |
|---|---|---|---|
| **병사(SoldierLab + GASP)** | **7.27** | **12.52** | Cover 1.60/2.40 · Engagement –/2.37 · CMC 1.11/2.06 · 애니 GT 1.70/1.58 · Sight 0.50/1.35 · 메시 틱 0.95/1.03 · `AC_PreCMCTick` 0.63/0.71 · BP ReceiveTick 0.41/0.91 · ScanTurn 0.35(M1) |
| **UGV `QuadCamComponent`** | **3.59** | **3.92** | 거의 전부 `Post Tick Component Update` — 4절 |
| 숲 `InstancedSkinnedMeshComponent` 틱 | 1.64 | 1.59 | `ABM_Tree_Silver_Birch_01_D` 0.53 최대, 나머지 수종 0.13~0.19 |
| Landscape Grass Update | 0.66 | — | `ULandscapeSubsystem` |
| EndPhysics | 0.72 | 0.76 | |
| Niagara 매니저 GT | — | 0.46 | |
| 병사 투사체 | — | 0.45 | `ProjectileMovement` |
| 드론 `TargetDetection` | — | 0.23 | |
| 트럭 | 0.23 | 0.10 | |

**09-21 `L_SoldierScenario`(숲 없음, PIE, 35명) 대비**: Engagement **0.22 → 2.37**(≈ 10배) · Sight 0.9 → 1.35 · BP ReceiveTick 0.22 → 0.9. 병사 수(35 → 40)와 교전 밀도 차이를 감안해도 Engagement 는 설명이 안 된다. 원인 후보: **09-28 잎 투과율 시스템**(잎이 병사 시야/탐지를 가림, `nanite/2026-09-28_forest_nanite_foliage_migration.md` E절) 추가 **[B]** — S3 가 `SoldierLab.<System>.Enabled` A/B 로 판정.

---

## 4. 발견 A — 수동 `CaptureScene()` 의 월드 렌더 상태 플러시 [A 소스 / B 순비용]

- 관측: UGV `QuadCam` 틱 3.6~3.9 ms(max 6.9) 중 **3.4~3.7 ms 가 `Post Tick Component Update`**.
- 원인(엔진 `Runtime/Engine/Private/Components/SceneCaptureComponent.cpp:823-839`): `USceneCaptureComponent2D::CaptureScene()` 이 렌더 전에 **`World->SendAllEndOfFrameUpdates()`** 를 호출한다. 그래서 그 프레임의 **첫 수동 캡쳐**가 월드 전체의 더티 렌더 상태(병사 스키닝·트랜스폼 등)를 틱 한가운데서 게임 스레드로 직렬 처리하고, 그 뒤에 틱한 컴포넌트는 프레임 끝에서 다시 갱신된다. QuadCam 이 `TG_DuringPhysics` 앞쪽에서 틱해서 매번 이 비용을 떠맡는다.
- 같은 패턴: `Plugins/QuadCamModule/.../QuadCamComponent.cpp:309` · `Vehicles/RCWSComponent.cpp:366` · `Vehicles/TitanTruck.cpp:376` · `Drone/DronePawn.cpp:1018`(전부 `bCaptureEveryFrame=false` + 틱에서 라운드로빈 수동 호출).
- 후보 해법: **`CaptureSceneDeferred()`**(엔진 `:811`, 프레임 끝 일괄 갱신 뒤 렌더). 3.6 ms 중 **순수 추가비용이 얼마인지**(원래 프레임 끝에서 어차피 낼 비용과 겹치는 몫)는 **[B]** — S1 이 A/B 로 판정. ⚠ 수동 호출을 택한 이력(동적 생성 캡쳐에서 `bCaptureEveryFrame` 이 첫 프레임에 얼던 문제, 라운드로빈·뷰 셰이크 보정 구간)은 각 호출 지점 주석에 있다 — 보존 조건 확인 필요.

---

## 5. 발견 B — GPU (M1 단일 프레임, `ProfileGPU` Frame 4869) [A]

GPU 21.8 ms = 메인 뷰 17.4 + 캡쳐 1개 3.7(그 프레임엔 UGV 캡쳐 하나 — 라운드로빈).

| 메인 뷰 패스 | ms |
|---|---|
| BasePass(`BasePassParallel` = 비Nanite 3.41 + Nanite 0.36) | **3.77** |
| **RenderVelocities** | **3.35** |
| ShadowDepths(VSM Nanite 1.59 · VSM Non-Nanite 0.92) | 2.72 |
| PostProcessing(TSR 1400x788 → 1920x1080 1.30) | 1.83 |
| Nanite VisBuffer | 1.48 |
| UpdateAllPrimitiveSceneInfos → Skinning → **DynamicWind** | 1.43 |
| RenderDeferredLighting | 0.83 |
| GPUSkinCache 배치 | 0.49 |

09-28 Nanite ON(→ VSM 첫 실동작) + Megaplants 스키닝 숲 도입 이후 첫 측정. Velocity·비Nanite BasePass 가 큰 것은 바람 스키닝 나무의 움직임 벡터·비Nanite 경로 몫일 가능성 **[B]** — S2 판정.

---

## 6. 히치 (120프레임 max) [A]

| 무엇 | max |
|---|---|
| GC `IncrementalPurgeGarbage` → `FMemory_Trim_GMalloc`(M1) | 4.2 ms |
| Cover 틱(M2) | 4.9 ms |
| `RtspStreamEncodeFrame`(M1) | 18.6 ms |
| QuadCam 틱(M1) | 6.9 ms |

(stat max 는 항목별 최댓값이라 같은 프레임이 아닐 수 있다.)

---

## 7. 메모리 (M1 vs M3 memreport) [A]

- 프로세스 물리 16.6 GB(M1) / 13.7 GB(M3, OS 트림) — **비쿡 `-game` 이라 에디터 데이터 포함**, 절대값은 패키지와 다르다.
- 오브젝트 182,500 → 182,826(+326). 증가 항목: `ProjectileMovementComponent`/`SphereComponent`/`StaticMeshComponent`/`MaterialInstanceDynamic` 각 +17(투사체 풀 성장일 가능성 — 풀 상한까지 크는 건 정상), `NiagaraComponent` +33 · Niagara DataInterface 다수 · `DebugCameraController`/`CheatManager`/`PlayerCameraManager` 등 +1(사용자가 디버그 카메라 토글). `Texture2D` 상주 +285 MB(텍스처 스트리밍).
- **결론: 누수 판정 불가** — M1 이 교전 전이라 첫 교전의 워밍업(풀 성장·Niagara 캐시)이 섞였다. S5 는 **재시작 1회차 vs 5회차**(같은 시점)로 다시 떠야 한다.

---

## 8. 세션 인계표

| 세션 | 넘기는 것 | 규모(M1/M2) |
|---|---|---|
| **S1 캡쳐/RTSP** | 발견 A(`CaptureScene` 플러시, 4개 호출 지점) · RTSP 인코드 동기 대기(렌더 스레드 12 ms, max 18.6) · 캡쳐 GPU 뷰당 ≈ 3.7 ms × 활성 캡쳐 수 | GT 3.6~3.9 + RT 대기 + GPU |
| **S2 숲/렌더** | Velocity 3.35 · 비Nanite BasePass 3.4 · VSM 2.7 · DynamicWind 스키닝 1.43 · 숲 GT 틱 1.6 · Grass 0.66 — **메인 뷰 GPU 의 대부분** | GPU ≈ 12 + GT 2.3 |
| **S3 병사** | M2 GT 12.5 ms · **Engagement 10배 원인**(잎 투과율 후보) · Cover/CMC · 09-21 수치는 숲 없는 레벨 기준이라 본 레벨 재측정 필요 | GT 7.3 / 12.5 |
| **S4 titan 게임플레이** | 드론 TargetDetection · 트럭 · 기타 합 < 1.5 ms — **우선순위 낮음** | < 1.5 |
| **S5 메모리** | 재시작 N회 memreport 재측정(1회차 vs 5회차) · GC `FMemory_Trim` 4.2 ms 히치 | — |

---

## 9. 다음 측정 때 보완할 것

- `stat unit` 스크린샷(시점별 Game/Draw/RHI/GPU 열) — 이번엔 로그로 역산했다.
- **M2·M3 `ProfileGPU`** — 교전 중 GPU(Niagara·총구·트레이서)와 재시작 후 GPU 비교가 없다.
- PC 사양(CPU 모델·코어 수·GPU·RAM) — 애니 워커 스레드가 병목이 되는지와 LIG PC 비교에 필요.
- 듀얼 모니터 구성 여부 확인(메인 뷰가 1080p 하나였음).
- 2-PC 자체방호축(캡쳐 7개 쪽) 측정.
- RTSP 수신 클라이언트가 붙은 상태 비교(인코드 비용 포함 여부).
- 메모리: 재시작 1회차 vs 5회차 같은 시점 memreport.
