# 자체방호 환경카메라에서 멀리 있는 트럭 환기구가 번쩍이는 문제 (TSR 지터)

2026-09-29 / 완료 / 환경카메라(BattlefieldCapture) 뷰에서만 TSR 서브픽셀 지터를 꺼서 원거리 트럭 루버의 ~1초 주기 번쩍임 해결.

관련 파일: `Source/titan_example/Vehicles/TitanTruck.cpp`, `TitanTruck.h`(수정).
선행 문서: `camera_pipeline/rtsp_postprocess_parity_0820.md`(씬 캡쳐가 메인뷰와 다른 렌더 경로라는 전제).

---

## 0. 증상

- 자체방호축 환경카메라(`ATitanTruck::BattlefieldCapture`, 640x360 SceneCapture, RTSP `selfdefense/env_camera`)에서
  **멀리 보이는 Titan truck 뒷면 환기구(루버)가 ~1초 간격으로 전등처럼 번쩍임.**
- 가까이서 보면 그냥 환기구. 발광체가 아님.
- **2026-09-28 Nanite/VSM 활성화 이전부터 있던 문제**(사용자 확인) — 숲 Nanite 교체 작업과 무관.

## 1. 캡쳐 구성 (조사 시점)

`TitanTruck.cpp` BeginPlay:

- 수동 `CaptureScene()`을 Tick에서 라운드로빈 — **2프레임에 1회** 캡쳐.
- `bAlwaysPersistRenderingState = true` — 자체 ViewState(= TSR 히스토리) 유지.
- `ShowFlags.TemporalAA = true`, AA 방식은 프로젝트 전역 TSR(`r.AntiAliasingMethod=4`).

## 2. 조사 과정

1. **액터/재질 쪽 원인 배제** — `BP_TitanTruck`에 라이트/타이머/타임라인 없음. 재질 7슬롯 중 발광 없음
   (`Mat_Glass_light`만 Translucent). → 렌더링 문제로 판단.
2. **메시** — `Titan_Truck`: 983,577 tris, LOD 1개, Nanite off. 루버가 실제 지오메트리라 640x360에서 **서브픽셀**.
3. **A/B cvar 테스트**(PIE, 지터 켠 상태):

   | cvar / 변경 | 결과 |
   |---|---|
   | `r.TemporalAA.Debug.OverrideTemporalIndex 0` | **완전히 해결** |
   | `r.Shadow.Virtual.Enable 0` | 효과 없음 |
   | `r.SSR.Quality 0` | 효과 없음 |
   | `r.Lumen.DiffuseIndirect.Allow 0` | 효과 없음 |
   | `r.TSR.ShadingRejection.Flickering.Period 12` | 효과 없음 |
   | 트럭 Nanite ON + `r.Nanite.MaxPixelsPerEdge 4` | 효과 없음 |

   메인 화면과의 비교는 원인 판별엔 결정적이지 않았음(해상도·경로가 달라서).
4. **`OverrideTemporalIndex`가 무엇을 바꾸는지 확인** — `View.TemporalJitterIndex`와 투영 서브픽셀 오프셋뿐
   (`SceneVisibility.cpp:5445-5450`, UE 5.8). 이 인덱스를 읽는 건 TSR/TAA/DOF/디버그뿐이다. → 다른 시스템이
   우연히 같이 멈춘 게 아니라 **지터 자체가 원인**으로 확정.

## 3. 원인

- TSR 지터 시퀀스는 8 샘플을 **다음 소수 11로 올린다**(`SceneVisibility.cpp:5256-5296`).
- 서브픽셀 루버에서 11개 지터 위치 중 일부만 밝은 면에 맞음 → **11캡쳐 주기로 밝기가 튄다.**
- 캡쳐가 2프레임에 1회라 주기 = **22프레임** → fps 20대에서 **~1초**. 증상 주기와 일치.
- TSR 깜빡임 감지(`r.TSR.ShadingRejection.Flickering.Period`, 기본 2프레임, `TemporalSuperResolution.cpp:189`)는
  이렇게 긴 주기를 못 잡는다(Period 12로 올려도 효과 없었음).
- **미특정**: "밝은 값"의 정확한 원천(스페큘러/그림자 경계 등)은 A/B로 특정되지 않았다(VSM/SSR/Lumen 끄기 모두
  무효). 지터가 트리거라는 것까지만 확정.

## 4. 기각한 해결책

- **트럭 메시 Nanite** — Nanite는 설계상 ~1px 디테일을 남긴다(`MaxPixelsPerEdge=1`, LOD 선택은 캡쳐 해상도 기준)
  → 서브픽셀 루버가 그대로 남아 효과 없음. 부수 교훈:
  - Position Precision을 `1/2cm`로 두면 작은 부품·타이어 트레드가 뭉개진다(분모가 클수록 정밀). **Auto 권장.**
  - Nanite 켠 메시에서 `Mat_Glass_light`(Translucent)는 `r.Nanite.AllowTranslucency=0`(기본, ReadOnly)이라 미지원.
  - 결국 **P4 revert.**
- **캡쳐의 TAA 쇼플래그 끄기** — FXAA로 폴백(`SceneView.cpp:1188-1194`)되어 나뭇잎 디더링 누적까지 사라짐 →
  **사용자 거부.**
- **디버그 cvar를 그대로 쓰기** — 전역이라 메인뷰 AA도 약화되고, `#if !UE_BUILD_SHIPPING`이라 Shipping에서 컴파일 아웃.

## 5. 적용한 해결책 — 이 캡쳐 뷰만 지터 끄기 (2026-09-29)

`Source/titan_example/Vehicles/TitanTruck.cpp/.h`:

- `TitanTruck.cpp` 상단 익명 네임스페이스에 `FBattlefieldNoJitterExtension : FSceneViewExtensionBase`.
  `SetupView()`에서 **`InView.State == Capture->GetViewState(0)`일 때만** `InView.bAllowTemporalJitter = false`.
  - TSR 누적은 유지되고 지터만 꺼진다(`SceneVisibility.cpp:5249` 조건). 엔진의 평면 반사
    (`PlanarReflectionRendering.cpp:584`)와 같은 방식.
- 확장은 전역 등록이지만 씬 캡쳐도 `SetupView`를 거치므로(`SceneCaptureRendering.cpp:916`, `833-849`)
  **ViewState 비교로 이 캡쳐만 골라낸다** → 메인뷰/CCTV/RCWS/드론 짐벌 영향 없음.
- BeginPlay에서 `FSceneViewExtensions::NewExtension<FBattlefieldNoJitterExtension>(BattlefieldCapture)`.
  헤더에 `TSharedPtr<ISceneViewExtension, ESPMode::ThreadSafe> BattlefieldNoJitterExtension;` — 액터 해제 시
  공유 포인터가 풀리면서 자동 등록 해제. 캡쳐는 `TWeakObjectPtr`로 잡아 캡쳐 소멸 후에도 안전.
- **검증**: 사용자 빌드 후 PIE에서 번쩍임 사라짐, 이상 없음 확인(2026-09-29).

## 6. 품질 트레이드오프

- 잃는 건 **서브픽셀 AA뿐**:
  - 정지 카메라에서 고대비 직선 모서리에 1px 계단(고정, 지글거림 없음).
  - 1px보다 가는 물체가 부분 커버리지 대신 보임/안보임으로 고정.
- 640x360 + H.264에선 체감 거의 없음(사용자도 차이 못 느낌).
- 노이즈 누적(디더링/Lumen/그림자)은 지터가 아니라 프레임 인덱스 기반이라 **유지된다.**

## 7. 남은 것

- **CCTV(QuadCam)/드론 짐벌/RCWS 조준경으로 확장은 보류** — 사용자 "일단 환경카메라만".
  확장 시 공용 헬퍼 `Plugins/QuadCamModule/.../SceneCaptureViewParity`에 올리는 방향.
- RCWS 조준경에 적용한다면 원거리 1px 목표의 부분 커버리지가 사라지는 영향 확인 필요.
  `TargetDetectionComponent`는 캡쳐 픽셀을 읽지 않으므로(기하 투영만) **탐지엔 영향 없음.**
