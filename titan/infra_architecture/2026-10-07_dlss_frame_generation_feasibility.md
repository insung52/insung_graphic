# DLSS · 프레임 생성(FG) 도입 가능성 조사

2026-10-07 / 완료(조사 전용, 코드 변경 없음) / DLSS·FG는 전시 PC(리눅스)에서 UE 플러그인이 지원되지 않고, 씬캡쳐(RTSP 12스트림)에는 엔진 구조상 업스케일·FG 모두 효과가 없어 도입 비권장 — 대안은 기존 TSR 스크린 퍼센티지/품질을 그래픽 탭 옵션으로. (오후 추가: 공개된 UE+리눅스 통합 사례는 없음. 단 NGX 직접 FG SDK는 리눅스 .so 제공(공식 OS 요구사항은 Windows만 표기) — 7절. 저녁 추가: 사용자 요구사항(캡쳐 포함·화면=RTSP·리눅스·플러그인) 기준 판정 — DLSS SR/DLAA는 플러그인으로 가능성 높음, FG는 RTSP만 쉽고 화면 동일 출력은 대형 R&D, PoC 순서 — 8절. DLAA vs TSR 메모 — 9절)

> 근거 표기: 프로젝트 `file:line`은 `C:\working\works\kadex\titan_example` 기준, 엔진 `file:line`은
> `C:\Program Files\Epic Games\UE_5.8\Engine\Source\Runtime` 기준. 웹 근거는 6절 URL.
> [B] = 추정, [미확인] = 확인 못 함. 프로젝트 소스·Config·에셋은 하나도 바꾸지 않았다.

---

## 0. 요약

| 질문 | 답 |
|---|---|
| 지금 프로젝트에 DLSS/FG가 구현돼 있나 | **아니다.** DLSS·Streamline·FSR·XeSS 플러그인·설정 전부 없음. AA/업스케일은 엔진 기본 TSR |
| 여러 씬캡쳐 환경에서 업스케일러가 도나 | 캡쳐에도 플러그인이 붙을 수는 있지만 **캡쳐는 엔진상 항상 100% 해상도** → 렌더 비용을 못 줄임(AA 대체가 최대) |
| FG가 RTSP 화면에 효과가 있나 | **없다.** FG는 스왑체인 Present 단계에서 프레임을 끼워 넣음 → 로컬 창 표시 fps만 오름 |
| 전시 PC(리눅스)에서 쓸 수 있나 | **못 쓴다.** NVIDIA UE 플러그인은 Win64 전용(2026-04 기준 변화 없음) |
| 권고 | **도입 비권장.** 대안 = TSR 스크린 퍼센티지·TSR 품질을 그래픽 탭 옵션으로 |

---

## 1. 프로젝트 현황 [A]

- **플러그인 없음** — `titan_example.uproject` 플러그인 목록, `Config/*.ini`, 소스·플러그인 `Build.cs` 검색에서
  DLSS/Streamline/NGX/FSR/XeSS 흔적 없음(`SoldierCover` 검색 히트는 문자열 우연 일치). 엔진 5.8 `Engine/Plugins`
  (Marketplace 포함)에도 해당 플러그인 없음.
- **AA/업스케일 = TSR** — `Config/DefaultEngine.ini:46` `r.AntiAliasingMethod=4`. S0 기준선에서 메인 뷰는
  `TemporalSuperResolution 1400x788 -> 1920x1080`(`infra_architecture/2026-09-29_perf_baseline_s0.md` 5절).
- **RHI** — Windows `DefaultGraphicsRHI_DX12` + `PCD3D_SM6`(`Config/DefaultEngine.ini:156-159`),
  Linux `SF_VULKAN_SM6`(`Config/DefaultEngine.ini:193`).
- **엔진에 프레임 생성 내장 기능 없음** — `Engine/Source/Runtime` 헤더에서 FrameGeneration/FrameInterpolation 검색 시
  LiveLink 보간만 걸림.
- **서드파티 업스케일러 훅** = `ITemporalUpscaler`(`Renderer/Public/TemporalUpscaler.h:12`) — 뷰 패밀리에
  `SetTemporalUpscalerInterface`(`Engine/Public/SceneView.h:2688`)로 꽂으면 `GetMainTAAPassConfig`가
  `EMainTAAPassConfig::ThirdParty`로 분기(`Renderer/Private/PostProcess/TemporalAA.cpp:1180`).

## 2. 씬캡쳐와의 관계 (엔진 소스) [A]

- **캡쳐도 뷰 익스텐션을 받는다** — `SceneCaptureRendering.cpp:915-916`
  `GEngine->ViewExtensions->GatherActiveExtensions(...)`, `:833-841` `SetupSceneViewExtensionsForSceneCapture`가
  `SetupViewFamily`/`SetupView` 호출. 즉 DLSS 같은 플러그인이 캡쳐 뷰 패밀리에 붙는 것 자체는 가능하다
  (실제로 붙는지는 플러그인 구현 몫 — [미확인], 플러그인 소스 미열람).
- **그러나 캡쳐는 스크린 퍼센티지를 지원하지 않는다** — `SceneCaptureRendering.cpp:950-955`:
  `// Screen percentage is still not supported in scene capture.` →
  `EngineShowFlags.ScreenPercentage = false` + `FLegacyScreenPercentageDriver(..., 1.0f)`. 캡쳐는 **항상 100% 해상도**로 렌더.
- **예외는 메인 뷰 해상도 모드뿐** — `bMainViewResolution`(`Engine/Classes/Components/SceneCaptureComponent2D.h:178-180`,
  판정 `:215-223`)이 켜져 있고 `MainViewFamily`가 있을 때만 메인 뷰의 스크린 퍼센티지 인터페이스를 포크한다
  (`SceneCaptureRendering.cpp:892-894, 945-948`). `MainViewFamily`는 엔진 지연 캡쳐 경로에서만 채워진다
  (`Engine/Private/Components/SceneCaptureComponent.cpp:536-593`).
- **우리 캡쳐는 전부 이 예외 밖** — QuadCam/RCWS/트럭/드론 모두 수동 `CaptureScene()`(S1 문서), RCWS는
  `bMainViewResolution`이 기본 off이고 수동 캡쳐에선 효과도 없다는 것이 이미 주석으로 정리돼 있음
  (`Source/titan_example/Vehicles/RCWSComponent.cpp:119-140`).
- **결론** — 업스케일러는 캡쳐의 렌더 해상도를 낮추지 못하므로 **캡쳐 렌더 비용을 줄이지 못한다**(캡쳐에 붙어도 AA 대체가 최대).
  S1이 찾은 캡쳐당 **해상도 무관 고정비 ≈2.4 ms**(VSM·Nanite·Lumen, `camera_pipeline/2026-09-29_s1_capture_rtsp_cost_analysis.md`)도
  업스케일로는 줄지 않는다.

## 3. 프레임 생성(FG)

- **동작 위치** — FG는 스왑체인 Present 단계에서 보간 프레임을 끼워 넣는다(Streamline 문서: DLSS-G가 로드되면
  호스트를 오프스크린 렌더로 강제하고 추가 그래픽 큐를 만든다). 따라서:
  - **로컬 창의 표시 fps만 오른다.**
  - 렌더 타깃 → NVENC로 가는 **RTSP 12스트림(LIG가 실제로 보는 화면)에는 효과 0.**
  - **게임 스레드 시뮬 속도는 그대로** — 교전 장면은 GT 바운드(S0 M2 28.9 ms)라 체감 개선이 없다.
  - **입력 지연 증가** — 조이스틱 조준 조작에 불리하다 [B: 정량 미측정].
- **Streamline DLSS-G 요구사항**(6절 Streamline 문서):
  - D3D12·Vulkan 지원(Dynamic MFG는 D3D12 전용), OS는 **Windows 10 20H1(19041)+**, 하드웨어 가속 GPU 스케줄링(HAGS) 필요.
  - **Reflex 통합 필수**, 화질을 위해 Hudless(UI 전) 버퍼와 UI 버퍼 제공 필요.
  - "does **not currently support multiple swap chains**".
  - 해상도 변경·풀스크린/창 모드 전환 시 FG를 꺼야 하고, 켜고 끌 때 스왑체인 재생성.
- **우리 구성과 충돌** — 자체방호 축은 **OS 창 2개**: 메인 창 + `SelfDefenseMonitor1Window`
  (`Source/titan_example/titan_examplePlayerController.cpp:342-367`, `SNew(SWindow)` + `AddWindow`) = 스왑체인 2개 →
  "다중 스왑체인 미지원" 제약에 걸린다. F11 풀스크린 전환도 운용에 포함돼 있다(같은 파일 주석).

## 4. 플랫폼 지원

- **NVIDIA DLSS 4.5 UE 플러그인 8.8.0(2026-09)** — UE 5.5/5.6/5.7/5.8 지원, 기능: Multi Frame Generation, Dynamic MFG,
  Ray Reconstruction, Super Resolution, DLAA, Reflex, NIS(NVIDIA 개발자 페이지).
- **그러나 UE 플러그인은 Win64 전용** — `.uplugin` Supported Platforms가 Win64뿐. NVIDIA 포럼 2025-03-26 공식 답변
  "Not at this time, but the team is aware of people asking for it.", 2026-04-01까지 변화 없음.
- **전시 PC는 리눅스(거의 확정 — S1 후속 메모)** → **DLSS SR·FG 모두 전시 환경에서 사용 불가.**
- 안정성: UE 5.8.2 + D3D12 + Streamline DLSS-G에서 `ResizeBuffers` 크래시 보고(NVIDIA·Epic 포럼).
- 타사: AMD FSR 3.1 SDK는 Vulkan을 지원하지만 **UE 플러그인의 리눅스 지원은 [미확인]**. Intel XeSS는 Windows 우선,
  리눅스 미지원(커뮤니티 요청 단계).

## 5. 결론 · 권고

**DLSS·FG 도입 비권장.** 이유 네 가지가 독립적으로 성립한다:
1. 전시 PC(리눅스)에서 UE 플러그인이 동작하지 않는다.
2. RTSP 12스트림에 FG는 효과 0.
3. 캡쳐는 엔진상 스크린 퍼센티지 미지원 → 업스케일로 캡쳐 비용을 못 줄인다.
4. 교전 장면은 게임 스레드 바운드 → GPU 측 기법의 체감 이득이 작다.

**대안** — 이미 쓰고 있는 **TSR의 스크린 퍼센티지·TSR 품질을 그래픽 설정 탭(`UTitanGraphicsSettings`) 옵션으로** 노출.
엔진 구조상 캡쳐는 이 값의 영향을 받지 않는다(2절). 과거 그래픽 탭에서 "렌더 스케일"을 보류한 사유가 "축/RTSP와
얽힘"이었는데(`CURRENT_STATE.md` §5), 실제로 얽히는 것이 메인 뷰 스트림(`MainViewStreamComponent`)뿐인지는
확인 필요 [B].

Windows 시연 PC 한정으로 DLSS SR/DLAA는 가능하지만, TSR 대비 이득은 성능보다 화질 쪽이다 [B].

## 6. 출처

- NVIDIA 포럼 — Linux Support for UE5 DLSS Plugin: https://forums.developer.nvidia.com/t/linux-support-for-ue5-dlss-plugin/328014
- NVIDIA DLSS 시작 페이지(UE 플러그인 버전·기능): https://developer.nvidia.com/rtx/dlss/get-started
- Streamline DLSS-G 프로그래밍 가이드: https://github.com/NVIDIAGameWorks/Streamline/blob/main/docs/ProgrammingGuideDLSS_G.md
- UE 5.8.2 Streamline DLSS-G ResizeBuffers 크래시: https://forums.developer.nvidia.com/t/ue-5-8-2-d3d12-streamline-dlss-g-editor-crashes-in-resizebuffers-with-dxgi-error-invalid-call-when-minimized/381531
- AMD FSR 3.1 Vulkan 지원: https://www.gamingonlinux.com/2024/03/amd-fsr-31-announced-with-vulkan-support-upscaling-quality-improvements/comment_id=260005
- Intel XeSS on Linux: https://community.intel.com/t5/Graphics/XeSS-on-Linux/td-p/1741330

---

## 7. 추가 조사 (2026-10-07 오후) — GitHub/Fab 사례 · NGX 직접 FG SDK

사용자 질문: "GitHub나 Fab에 우리가 원하는 걸(리눅스 UE에서 업스케일/FG) 시도한 흔적이 있나?"

### 7-1. 공개된 UE + 리눅스 업스케일/FG 통합 사례 — 없음
| 대상 | 플랫폼·상태 | 출처 |
|---|---|---|
| NVIDIA DLSS UE 플러그인 | Win64 전용(4절) | NVIDIA 포럼 |
| AMD FSR 3 UE 플러그인 v3.1.4a | UE 5.6까지. FG는 "requires Windows using Direct3D 12" | https://gpuopen.com/learn/ue-fsr3/ |
| AMD FSR(Redstone 4.1.1) UE 5.8 플러그인 | Windows 10/11. ML FG는 Windows 11 + RX 9000 이상 | https://videocardz.com/newz/amd-updates-fsr-unreal-engine-plugin-with-rx-7000-support-and-ue-5-8-compatibility · https://gpuopen.com/learn/ue-fsr/ |
| Intel XeSS UE 플러그인 | Windows 우선, 리눅스 미지원 | 4절 |
| GitHub `scragnog/DLSS5ForUE5` | 비공식 UE 플러그인, Windows + D3D12 전용 | https://github.com/scragnog/DLSS5ForUE5 |
| GitHub `FireBurn/FSR-Vulkan` | 네이티브 Vulkan FSR + 프레임 보간 라이브러리(MIT, 리눅스 가능). **UE 통합 없음, 스타 0 — 실험 단계** | https://github.com/FireBurn/FSR-Vulkan |
| `dlssg-linux` · `DLSS5VKLayer` · `lsfg-vk` | 게임 대상 해킹/Vulkan 레이어(Present 가로채기). 전시 장비용 아님 | https://github.com/aethelisdev/dlssg-linux · https://www.phoronix.com/news/DLSS5VKLayer |

### 7-2. 핵심 발견 — NVIDIA DLSS SDK에 리눅스 FG 라이브러리가 있다
- **`github.com/NVIDIA/DLSS` `lib/Linux_x86_64/rel`** 에 `libnvidia-ngx-dlss.so.310.9.1`(SR) · `libnvidia-ngx-dlssd.so.310.9.1`(RR) ·
  **`libnvidia-ngx-dlssg.so.310.9.1`(FG)** 존재. 정적 라이브러리 `lib/Linux_x86_64/libnvsdk_ngx.a`.
- **DLSS-FG Programming Guide 310.7.0**(`doc/DLSS-FG Programming Guide.pdf`, 텍스트 추출 기준 줄 번호):
  - 리눅스 링크법: "Linux projects must link against lib/Linux_x86_64/libsdk_nvngx.a, libstdc++.so, and libdl.so"(≈248행),
    런타임 "On Linux: libnvidia-ngx-dlssg.so.<ver>"(≈261행), 리눅스 디버그는 환경변수 `__NGX_LOG_LEVEL` 등(≈276·291행).
  - API: "D3D12 or Vulkan"(≈181행), Vulkan 1.1 이상(≈216행).
  - **그러나 System Requirements에는 "Operating System: Windows 10 20H1 (build 19041) or higher."만 표기**(≈67행) →
    **리눅스 공식 지원 여부 [B]**(라이브러리·링크 안내는 있는데 요구사항 표에는 없음).
  - GPU: **RTX 40 시리즈 이상**(MFG는 50 시리즈)(≈64-66행). 드라이버 520+(≈68행).
  - "NVIDIA recommends integration of DLSS-FG via the NVIDIA Streamline SDK."(≈61행) — 아래 NGX 직접 방식은 비권장 경로다.
- **NGX 직접 방식의 구조**(가이드 6·7절, ≈674-935행):
  - 앱이 `NVSDK_NGX_EvaluateFeature`를 부를 때마다 **보간 프레임 1장이 앱이 준 텍스처로 나온다**(2x = 프레임당 1회, MFG는 배수-1회).
  - **Present와 페이싱은 앱 책임** — "The DLSS-FG feature itself does not handle timing or presentation". 생성 프레임을 즉시
    출력하고, 실제 프레임은 `OutputReal`로 보류했다가 같은 간격에 맞춰 출력(보통 렌더 스레드와 별도로 비동기 Present).
  - **다중 인스턴스·여러 창·한 백버퍼의 서브렉트 지원**("Multiple Instances/Viewports", ≈1391-1560행). 인스턴스 간 동기화는 앱 책임.
  - 창 크기·렌더 해상도가 바뀌면 인스턴스를 재생성(≈662-667행).
  - 즉 Streamline(스왑체인 후킹, 다중 스왑체인 미지원 — 3절)과 달리 **스왑체인에 묶이지 않는다.**

### 7-3. 우리 프로젝트에의 의미 (전부 [B] — 코드·실측 없음)
- **(a) DLSS SR을 리눅스에서** — NVIDIA UE 플러그인 소스의 Vulkan(NGX) 백엔드를 리눅스로 이식하고 리눅스 NGX를 링크.
  중간 난이도. 이득은 메인 뷰 화질 위주(TSR 대비) — 캡쳐 비용은 2절 이유로 그대로.
- **(b) FG를 리눅스에서** — UE 통합이 없으므로 직접 만들어야 한다: RHI에서 Vulkan 핸들 획득, depth/velocity/hudless 공급,
  **엔진 Present 경로에 생성 프레임을 끼우고 페이싱**(엔진 RHI·Slate 수정급). **대형 R&D.** 메인 창 표시 fps만 오르고
  GT 바운드 장면의 시뮬 속도는 그대로(3절).
- **(c) RTSP 스트림에 FG** — NGX 직접 FG는 텍스처 입출력이라 **이론상 캡쳐 → NVENC 경로에도 끼울 수 있다**(스트림 fps 2배).
  단 캡쳐에서 depth·모션벡터를 따로 뽑아야 하고, 실제 프레임을 1장 보류하므로 **지연이 늘어난다**(현재 RTSP 종단 68 ms 기준).
  스트림 12개 × 인스턴스 GPU 비용도 미지수.
- **(d) 하드웨어 조건** — DLSS-FG는 RTX 40 이상. **전시 PC GPU 세대 확인 필요.**
- 결론 갱신: 5절의 "비권장"은 **기성 플러그인 기준**으로 유지. 리눅스 FG는 "불가능"이 아니라 "**기성품 없음 + 공식 OS 표기 불명확 +
  직접 통합 대형 R&D**"로 정정한다.

### 7-4. 출처 (추가분)
- NVIDIA DLSS SDK: https://github.com/NVIDIA/DLSS (`lib/Linux_x86_64/rel`, `doc/DLSS-FG Programming Guide.pdf` 310.7.0)
- AMD FSR 3 UE 플러그인 가이드: https://gpuopen.com/learn/ue-fsr3/
- AMD FSR UE 플러그인 가이드(최신): https://gpuopen.com/learn/ue-fsr/
- AMD FSR UE 5.8 업데이트: https://videocardz.com/newz/amd-updates-fsr-unreal-engine-plugin-with-rx-7000-support-and-ue-5-8-compatibility
- DLSS5ForUE5: https://github.com/scragnog/DLSS5ForUE5
- FSR-Vulkan: https://github.com/FireBurn/FSR-Vulkan
- dlssg-linux: https://github.com/aethelisdev/dlssg-linux
- DLSS5VKLayer: https://www.phoronix.com/news/DLSS5VKLayer

---

## 8. 사용자 요구사항 기준 실현 가능성 판정 (2026-10-07 저녁)

> 실제 구현은 별도 세션에서 진행(사용자). 이 절은 **구현 세션의 출발점**이다 — 판정, 근거, 리스크, PoC 순서.
> 근거 표기: [A] = 엔진/프로젝트 소스로 확인, [B] = 추론·미검증.

### 8-1. 요구사항 (사용자 원문 요약)
- **DLSS**: 메인 뷰(RCWS 뷰)뿐 아니라 **씬캡쳐에도** 적용. **언리얼 화면 표시 결과와 RTSP 송출 영상 둘 다** 적용돼야 함(하나만 되면 안 됨).
- **프레임 생성(FG)**: 위와 동일.
- **공통**: 리눅스 지원 · 언리얼 엔진 코드 수정은 되도록 기피(유지보수) · RTSP 플러그인처럼 **새 플러그인**으로 구현.

### 8-2. 전제 — "화면 = RTSP" 조건은 현재 구조가 이미 만족한다 [A]
출력마다 원본이 하나다. 원본(렌더 결과)에 적용하면 두 출력은 자동으로 같아진다.
- **씬캡쳐**: 캡쳐 RT 하나를 인게임 위젯과 RTSP가 같이 읽는다 — `Plugins/RtspEncoder/.../RtspStreamComponent.cpp:28`(`SourceCapture->TextureTarget`).
- **RCWS 메인 뷰**: 메인 뷰가 렌더를 마친 결과를 뷰 익스텐션이 RTSP용 RT로 복사한다 — `Plugins/RtspEncoder/Public/MainViewFrameSource.h`(2026-08-20 설계, 메인 뷰의 TSR 히스토리·스크린 퍼센티지가 그대로 반영됨).
- 이 전제는 **업스케일러(DLSS/DLAA)에는 그대로 성립**하고, **FG에는 성립하지 않는다**(8-4).

### 8-3. DLSS 업스케일링 / DLAA — 이론상 가능, 플러그인만으로 될 가능성 높음
업스케일러는 "렌더 뒤에 덧씌우는 필터"가 아니라 **렌더 파이프라인 안에서 TSR 자리를 대체**하는 단계다(저해상도 씬 컬러 + depth + 모션벡터 + 지터 필요). 그래도 결과가 곧 RT이므로 두 출력은 같아진다.

엔진 수정 없이 붙일 수 있는 근거 [A] (엔진 `Runtime/` 기준):
| 필요한 것 | 엔진 쪽 근거 |
|---|---|
| 외부 업스케일러 꽂는 자리 | `Renderer/Public/TemporalUpscaler.h:12` `ITemporalUpscaler`, `Engine/Public/SceneView.h:2688` `SetTemporalUpscalerInterface`, `Renderer/Private/PostProcess/TemporalAA.cpp:1180` ThirdParty 분기 (NVIDIA 공식 플러그인도 이 방식) |
| 캡쳐 뷰에도 적용 | `Renderer/Private/SceneRenderBuilder.cpp:511` — `BeginRenderViewFamily`가 캡쳐 포함 **모든** 뷰 패밀리에 호출. 캡쳐는 활성 익스텐션을 모으고 `SetupViewFamily`/`SetupView` 호출(`SceneCaptureRendering.cpp:915-916, 833-846`). `SceneRendering.cpp:5460` "upscalers set no earlier than BeginRenderViewFamily" |
| NGX용 Vulkan 확장 켜기 | `VulkanRHI/Public/IVulkanDynamicRHI.h:173-182` `AddEnabledInstanceExtensionsAndLayers` / `AddEnabledDeviceExtensionsAndLayers`(정적, RHI 초기화 전 플러그인 로드 단계에서 호출) |
| Vulkan 핸들·이미지 | 같은 헤더 `RHIGetVkInstance/RHIGetVkDevice`(:115-116), `RHIGetVkImage`(:165), 커맨드버퍼는 `RHIGetVkCommandBuffer(FRHICommandListBase&)`(:127 — 5.8에서 `RHIGetActiveVkCommandBuffer`는 deprecated) |
| 프로젝트 선례 | RTSP 플러그인이 이미 리눅스 Vulkan + CUDA 연동을 플러그인으로 구현(`FNvencVulkanEncoder`) |
| 리눅스 | DLSS SR SDK는 2021년부터 리눅스 공식 지원(드라이버 470+), SDK `lib/Linux_x86_64/rel/libnvidia-ngx-dlss.so` |

걸리는 점:
- **캡쳐 해상도 비율이 100%로 고정** [A] — `SceneCaptureRendering.cpp:950-955`("Screen percentage is still not supported in scene capture", `FLegacyScreenPercentageDriver(1.0)` + `ShowFlags.ScreenPercentage=false`). 그래서 캡쳐는 **DLAA(100% 해상도, AA만 대체)는 바로 가능**, **SR(저해상도 렌더 → 업스케일)은 플러그인이 고정값을 덮어써야 함**. 덮어쓰는 함수 `SetScreenPercentageInterface_Unchecked`는 공개(`SceneView.h:2646`, 일반 setter는 `:2642`에서 이미 설정돼 있으면 checkf)지만, 렌더러가 비-메인뷰 캡쳐에서 그 경로를 제대로 처리하는지는 **PoC 필요 [B]**. (예외 경로 `bMainViewResolution`은 엔진 지연 캡쳐에서만 동작 — 우리 캡쳐는 전부 수동 `CaptureScene()`, `RCWSComponent.cpp:119-140`.)
- **캡쳐 성능 이득은 작다** — S1에서 캡쳐 1개당 ≈2.4 ms가 해상도 무관 고정비(VSM·Nanite·Lumen)로 나옴(`camera_pipeline/2026-09-29_s1_capture_rtsp_cost_analysis.md`). 캡쳐 쪽 효과는 **화질 위주**.
- **CCTV 라운드로빈** — QuadCam은 카메라 4개를 1틱에 1개씩 찍으므로 같은 카메라 히스토리 간격이 4틱. 업스케일러 히스토리가 약해져 잔상 가능 [B].
- **인스턴스 수** — 뷰(캡쳐)마다 NGX 인스턴스 1개 → VRAM 증가 [B]. 스킨 캐시 급저하 건(`2026-10-06_pie_gpu_stall_skin_cache.md`)에서 VRAM 7.4~7.7/8.15 GB였던 점 유의.
- 리눅스 NGX 패키징 — NGX가 설정 경로·`/usr/share` 쓰기 권한을 요구한다는 포럼 글(2026-01)이 검색에 보였음 [미확인].

### 8-4. 프레임 생성 — RTSP 쪽은 가능, "화면까지 동일"은 매우 어려움
**가능한 부분**: NGX 직접 FG(7-2)는 텍스처 입력 → 생성 프레임 텍스처 출력, 다중 인스턴스 지원. 캡쳐 RT마다 FG를 돌려 **RTSP를 2배 fps로 송출**하는 것은 플러그인으로 가능해 보인다(인코더는 우리 코드) [B].

**어려운 부분 — 인게임 화면은 창의 Present 횟수에 묶인다**:
- 창은 엔진 프레임당 1회 Present. 캡쳐 패널은 대시보드 UI(Slate) 안에 그려지므로, 생성된 중간 프레임을 화면에 보이려면 **프레임 사이에 창 전체를 한 번 더 그려 Present**해야 한다.
- Present 가로채기 훅은 Vulkan에 있다 [A] — `VulkanRHI/Private/VulkanViewport.cpp:812-863` `FRHICustomPresent::Present()`가 false면 네이티브 Present 생략(VR 플러그인 방식). AMD FSR3 UE 플러그인의 RHI 백엔드도 "Slate Redraw" UI 모드로 보간 프레임용 Slate 재그리기를 쓴 선례가 있다(https://gpuopen.com/learn/ue-fsr3/). 하지만 우리 환경 적용은 [B]이고 해야 할 일이 크다:
  생성된 캡쳐 프레임으로 대시보드 Slate 재그리기 · 출력 간격 페이싱 · **자체방호 축은 OS 창 2개**(`titan_examplePlayerController.cpp:342-367`) 동기화.
- 창 전체 백버퍼에 FG를 거는 방식은 캡쳐 패널에 모션벡터가 없어 UI로 취급됨 → 패널은 보간되지 않음 → "화면 = RTSP" 조건 위반 [B].

추가 리스크 [B]:
- **CCTV 라운드로빈** — 같은 카메라 두 프레임 간격 ≈80 ms(50 fps 기준 4틱) 사이를 생성 → 화질 위험 큼.
- **비용** — 스트림 12개 × FG 인스턴스 GPU 비용, NVENC 처리량 2배.
- **지연** — 실제 프레임 1장 보류 → RTSP(현재 종단 68 ms)·조준 지연 증가.
- **입력 데이터** — 캡쳐마다 depth·모션벡터 필요. 뷰 익스텐션 `SubscribeToPostProcessingPass`/`PrePostProcessPass_RenderThread`(`Engine/Public/SceneViewExtension.h:210, 222`)로 씬 텍스처 접근 가능해 보이나 확인 필요.
- **리눅스 공식 지원 불명** — SDK에 `libnvidia-ngx-dlssg.so` 있으나 FG 가이드 System Requirements는 "Windows 10 20H1"만 표기(7-2). GPU RTX 40+ 필요.
- 시뮬레이션 속도·게임 스레드는 그대로(교전 장면 GT 바운드 — S0).

### 8-5. 판정표
| 항목 | 메인 뷰 | 씬캡쳐 | 화면 = RTSP | 리눅스 | 플러그인만으로 |
|---|---|---|---|---|---|
| DLSS SR / DLAA | ○ | DLAA ○ / SR은 PoC 필요 | ○ (구조상 자동) | ○ 공식 | ○ 가능성 높음 |
| 프레임 생성 | △ | △ | **✗에 가까움** (RTSP만 쉬움) | ? 공식 표기 불명 | △ (Present 가로채기 R&D) |

### 8-6. 권장 PoC 순서 (각 단계 실패 시 중단)
0. **사전 확인(반나절~1일)** — 전시 PC GPU가 RTX 40 이상인지 · 리눅스 드라이버 595.84에서 DLSS SDK 샘플로 SR과 FG(`dlssg`)가 초기화되는지.
1. **Windows 화질 비교(1~2시간)** — Windows 시연 PC에서 NVIDIA 공식 UE 플러그인으로 메인 뷰 TSR(현재 73%) vs DLSS Quality vs DLAA를 **RTSP 수신 화면 기준**으로 비교. 리눅스 이식 투자 가치 판단용(9절).
2. 메인 뷰 DLSS SR 플러그인 리눅스 이식(NVIDIA UE 플러그인 Vulkan 경로 참고).
3. 캡쳐 1개에 DLAA → 이어서 캡쳐 SR(스크린 퍼센티지 덮어쓰기) PoC.
4. FG는 **RTSP 스트림 1개**에 먼저 — 화질·지연·GPU 비용 측정.
5. 4의 결과가 좋을 때만 인게임 Present 가로채기 검토.

---

## 9. DLAA vs TSR — 화질 비교 메모 (2026-10-07)

> 이 프로젝트에서 **직접 비교한 적은 없다.** 아래는 업계 일반 평가 + 이 프로젝트 조건을 합친 판단 [B].

### 9-1. 일반적으로 DLAA가 나은 점
- **얇은 디테일** — 나뭇잎·잔가지·전선·원거리 작은 물체의 깜빡임/뭉개짐이 적다. 이 프로젝트의 Nanite 숲, 원거리 병사 식별에 유리.
- **움직임 잔상** — 카메라 회전·이동 물체의 고스팅이 대체로 적다(DLSS 4 이후 트랜스포머 모델에서 개선).
- **연산 장치** — 텐서 코어에서 돌아 셰이더 연산 부담이 상대적으로 적다.

### 9-2. 비교 시 주의
- **메인 뷰는 지금 TSR 73%**(1400×788 → 1920×1080, S0 `ProfileGPU`). DLAA는 100% 해상도라 그대로 바꾸면 화질↑·GPU 비용↑. 공정 비교는 **DLSS Quality(≈67%) vs 현재 TSR**.
- **씬캡쳐는 이미 100%**라 DLAA vs TSR을 같은 조건으로 비교 가능. 단:
  - **환경카메라**(`BattlefieldCapture`)는 09-29 번쩍임 때문에 지터를 껐다(`camera_pipeline/2026-09-29_battlefield_capture_tsr_jitter_blink.md`) → 지터 없는 TSR은 AA가 약해 DLAA와 차이가 클 수 있음. 반대로 DLAA도 지터가 필요하므로 번쩍임 재발 위험.
  - **CCTV 라운드로빈** — 4틱 간격이라 둘 다 히스토리가 약함.
- **RTSP 인코딩 후 화질이 기준** — 압축이 미세한 차이를 상당 부분 덮는다. CCTV 320×180급 스트림은 차이가 거의 안 보일 수 있다.

### 9-3. 판단
- 화질 상승 폭은 **RCWS 메인 뷰와 숲을 비추는 고해상도 캡쳐**에서 가장 클 것, CCTV 저해상도 스트림은 기대치 낮음.
- 확정은 8-6의 1단계(Windows에서 공식 플러그인으로 같은 장면 나란히 비교, RTSP 수신 화면 기준)로.
