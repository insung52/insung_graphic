# LIG 9/28 현장 사고 로그 분석

2026-10-01 / 완료(분석) — 원인 미확정 / MUM-T 게임 스레드 정지 2건·자기방호 GPU/X11 종료 2건을 로그로 정리, 오디오·드라이버·"메모리 누수" 제보를 판정한 기록. 원인은 로그만으로 못 가림 → 블랙박스 기록기를 만든 계기.

> 관련 문서: 블랙박스 설계·구현 `2026-10-01_blackbox_health_recorder_design.md`(§0이 이 분석의 요약판), LIG 회신 메일 초안 `2026-10-01_lig_test_guide_mail.md`(머리 노트에 드라이버 웹 조사 출처), 로그 3종 비교 `2026-10-01_log_sources_comparison.md`.
>
> 시각은 전부 **한국 시각(KST)**. 엔진 로그 원문은 UTC(+9시간)다.

## 0. 자료

- LIG가 보낸 로그: `C:\working\logs\mumt-server-logs`(UGV/MUM-T PC), `C:\working\logs\defense-server-logs`(자기방호 PC).
  (2026-10-01 이 문서 작성 시점에 `C:\working\logs`는 비어 있었다 — 원본을 다른 곳으로 옮겼을 수 있음.)
- 엔진 로그만 있다. 메모리 수치·멈춘 동안의 상태·GPU 크래시 breadcrumb(`-gpucrashdebugging` 없이 실행)는 없다.

## 1. 두 PC 환경

| 항목 | MUM-T(UGV) PC | 자기방호 PC |
|---|---|---|
| GPU | RTX 5070 Ti | RTX 5070 Ti |
| RAM | 15 GB | 15 GB |
| NVIDIA 드라이버 | **610.43** | 595.84 (LIG 기준 버전) |
| 엔진 CL | 56702186 | 56057345 |
| 패키지 | kadex_0902 (9월 초) | kadex_0814_2 (8월 14일) |
| 비디오 드라이버 | x11 강제 — 인자 없으면 `InitSDL` 실패 | — |
| 오디오 출력 장치 | **VelocityOne Flightstick**(조이스틱의 USB 오디오 — PulseAudio 기본 출력이 그리로 잡힘) | HDMI |

두 PC는 드라이버·엔진 CL·패키지가 모두 다르다 — 어느 한 요인으로 두 PC의 증상을 묶어 설명할 수 없다.

## 2. MUM-T(UGV) PC

### 2.1 9/28 11:24 실행 — 게임 스레드 정지 → 종료 시 DEVICE_LOST

- 시작 약 2분 뒤 **게임 스레드 정지**: 로그 줄의 프레임 번호가 `[615]`에서 고정된 채 약 10분.
- 그동안 오디오 `waiting for h/w` 타임아웃 **28회**.
- 창을 닫을 때 `VK_ERROR_DEVICE_LOST`. breadcrumb 없음.
- 직전 입력: **Device1 버튼 17 연타**. 단, **바로 전 실행에서도 같은 버튼을 28회 눌렀는데 정상**이었다 → 이 버튼이 원인이라고 단정할 수 없다(LIG에 버튼 17 기능을 문의 — 메일 §6).

### 2.2 9/15 실행 — 같은 형태

- 시작 10초 후 프레임 번호 `[560]` 고정 + 오디오 타임아웃.
- 이 실행에서는 **조이스틱 조작 0건** — 버튼 연타 가설과 맞지 않는 사례.

## 3. 자기방호 PC

| 실행 | 증상 | 로그 |
|---|---|---|
| 9/28 15:23 | 정상 동작 중 종료 | `Fatal X11 IO Error` + `VK_ERROR_DEVICE_LOST` 동시 |
| 9/28 11:18 | 시작 직후 종료 | 시작부터 VRAM 예산 **7.2 / 16.3 GB**(직전 실행이 아직 VRAM을 잡고 있던 것으로 추정) → `Failed to allocate Device Memory` → X11 끊김 |

11:18 건은 "닫은 직후 재실행" 가능성이 있어 메일에서 "닫고 10초 대기 + `nvidia-smi` 프로세스 목록 확인"을 요청했다.

## 4. 오디오 h/w 타임아웃 — 원인인가 동반 증상인가

- 관찰: 오디오 `waiting for h/w` 타임아웃은 **MUM-T 멈춤 2건(9/15, 9/28 11:24)에만** 동반된다. 그 PC의 출력 장치는 조이스틱 USB 오디오(1절).
- 엔진 소스상 게임 스레드는 오디오 장치를 무한정 기다리지 않는다:
  - `AudioMixerSourceManager.cpp:4384-4388` — 1초 타임아웃 후 경고 로그를 찍고 진행한다. **그 경고는 LIG 로그에 없다.**
  - `AudioMixer.cpp:880-915` — 리눅스 SDL 오디오는 장치 교체를 지원하지 않아 타임아웃만 반복된다.
- 판단: **원인보다 동반 증상일 가능성이 높다. 단 배제는 못 한다.** 분리를 위해 MUM-T PC 출력 장치를 HDMI/스피커로 바꿔 달라고 요청(메일 §3-(2)). 블랙박스에도 해당 줄을 `AUDIO` 태그로 남기게 했다(설계 문서 §14.2a — 빌드 후 재확인 대기).

## 5. 드라이버 610.43

웹 조사 결론: **610.43 단독 원인 가능성은 낮음~중간.**

- 595.84인 자기방호 PC에서도 GPU 사고(3절)가 났다.
- RTX 50(Blackwell) + open 커널 모듈의 GPU 정지 보고가 580~610 전반에 있다.
- UE 5.7~5.8 리눅스 Vulkan 자체의 DEVICE_LOST 보고도 있다 — 엔진 쪽 원인도 같은 무게.

상세 항목과 출처는 메일 문서 머리 노트. 권장(메일 §3-(1)): MUM-T PC도 595.84로 맞추기.

## 6. 기타 — 텐트 체크 무늬

로그에 `tent_01`/`tent_08` 셰이더맵 쿡 누락이 있다(렌더 시 체크 무늬). 옛 레벨 리소스라 무시. **단 LIG가 말한 "자기방호 화면 오류"가 이것인지는 모른다** — 스크린샷을 요청했다(메일 §1).

## 7. "메모리 누수" 제보

### 7.1 로그로는 판정 불가

엔진 로그에 **메모리 수치가 한 줄도 없다**(11:18 건의 시작 시 VRAM 예산표 1회 제외). 누수가 맞는지 틀린지 판정할 근거가 없다 → 매 프레임·1초 단위로 RAM/VRAM을 남기는 블랙박스 기록기를 만들었다(설계 문서). LIG에는 누수로 판단한 근거를 문의(메일 §6).

### 7.2 탐지/바운딩박스 코드 조사 + 소수정 (2026-09-29, 코드 완료·빌드됨)

LIG가 "타겟디텍션/바운딩박스 쪽 메모리 누수"라고 짚어서 해당 경로를 조사했다.

- 조사 범위: 탐지 `UTargetDetectionComponent`, 박스 그리기 `UI/DetectionOverlayWidget.cpp`(`SDetectionOverlay::OnPaint`), LIG 송신 `UUGVRemoteControlSubsystem::SendObjectDetectionResult`.
- 결과: **프레임 단위로 쌓이는 누수는 없다.**
- 다만 정리하는 곳이 없던 맵 2곳에 **죽은 액터 키 정리**를 추가했다(시나리오 재시작 = 병사 Destroy→재스폰마다 죽은 키가 남았음):

| 위치 | 변경 |
|---|---|
| `Detection/TargetDetectionComponent.cpp` `ScanTargets` 끝 (기존 `LastActiveTime` 정리 루프 옆) | `TargetConfidence`·`ActivelyDetected`에서 무효(죽은) 액터 키 제거. 살아 있는 대상의 Confidence는 유지(엄폐 후 복귀 기억). |
| `Network/UGVRemoteControlSubsystem.cpp` `SendObjectDetectionResult` 앞부분 | `ObjectIDMap`에서 무효 액터 키 제거. `NextObjectID`는 그대로 증가(`GetOrAssignObjectID`, 1..65535 순환)해서 ID 재사용 없음 → **LIG가 보는 ObjectID 부여 동작은 불변.** |

- 양: 재시작마다 수십 항목 수준 → **사고의 주원인은 아니다.**
- PIE/현장 검증: 이 수정 자체를 따로 검증한 기록은 없음(빌드만 됨).

## 8. 결론과 다음 단계

- 원인 확정 없음. 남은 후보: 드라이버(610.43 / Blackwell open 모듈), UE 리눅스 Vulkan, 오디오(동반 증상 쪽으로 기울지만 미배제), 직전 실행의 VRAM 잔여(11:18 건).
- 다음:
  1. 블랙박스 기록기를 넣은 패키지로 재테스트 — 리눅스 패키지 빌드는 **미검증**(설계 문서 §14.3-7).
  2. LIG 요청 사항(드라이버 통일, 오디오 출력 장치 변경, `-gpucrashdebugging`, 문제 발생 시 `nvidia-bug-report`/`dmesg`/`Saved` 전달) — 메일 초안, **발송 전**.
