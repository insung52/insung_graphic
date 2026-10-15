# UGV 탐지가 교전 제외 대상(도주 3분대)을 감추기 + 3초 페이드아웃

2026-10-15 / 진행중(C++ 완료, 빌드·실측 대기) / UGV RCWS가 "안 쏘는" 적(SetTargetable false)은 UGV 탐지에서도 빼서(LIG UDP 포함) "보이는데 안 쏨" 모순을 없앰 — 사격 스위치 `bRespectEnemyTargetingExclusion`을 그대로 따르므로 트럭은 무관, 화면 박스만 약 3초 페이드.

관련 문서:
- 같은 날 UGV 테스트 대시보드: [`2026-10-15_ugv_test_dashboard_stretch_with_fixed_viewport.md`](2026-10-15_ugv_test_dashboard_stretch_with_fixed_viewport.md) · [`2026-10-15_axis_selection_ugv_dashboard_toggle_and_saved_choices.md`](2026-10-15_axis_selection_ugv_dashboard_toggle_and_saved_choices.md)
- 탐지 박스 마커/추적 선행 작업: [`2026-10-13_detection_marker_wbp.md`](2026-10-13_detection_marker_wbp.md) · [`2026-10-14_detection_box_tracking.md`](2026-10-14_detection_box_tracking.md)
- `SetTargetable` 동사 도입: [`../level_new_kadex_0811/2026-09-17_soldierlab_squad_scenario_link.md`](../level_new_kadex_0811/2026-09-17_soldierlab_squad_scenario_link.md) → UGV RCWS 스위치를 켜게 된 경위: [`../level_new_kadex_0811/2026-09-21_soldierlab_migration_new_kadex_0811.md`](../level_new_kadex_0811/2026-09-21_soldierlab_migration_new_kadex_0811.md) 5절 · [`../soldier_ai_lab/squad/2026-09-21_break_contact_and_targeting_exclusion.md`](../soldier_ai_lab/squad/2026-09-21_break_contact_and_targeting_exclusion.md)
- 에버그린: `guide/detection_dev_guide.md` §2·§3.6·§4.1, `guide/rcws_fire_control_dev_guide.md` §3.3, `guide/ui_dev_guide.md`(DetectionOverlayWidget 절), `level_new_kadex_0811/scenario_authoring_guide.md`(`bTargetable` 행)

> 폴더 선택: 탐지(`UTargetDetectionComponent`) 전용 devlog 폴더는 없고, 최근 탐지 작업(마커 WBP·박스 추적)과 같은 날 UGV 대시보드 문서가 전부 `ui/`에 있어 여기에 둠.

## 1. 계기 (사용자, 2026-10-15)

UGV 테스트 대시보드를 띄우자 **3차 전투지로 도주하는 적 3분대가 UGV TargetDetection에 박스로 잡히는데 UGV는 안 쏜다** — 전시 관람객 눈에는 논리 모순.

요구:
- 그 분대는 **UGV 탐지에 면역** — LIG 원격통제기 UDP 탐지 보고에서도 감지 안 됨.
- **트럭(이동형지휘소) RCWS는 무관** — 도주해 오는 3분대를 보고 쏴야 한다(그게 3차 전투지 시나리오).
- 사라질 때 **약 3초 페이드**.

## 2. 기존 구조 — 왜 "보이는데 안 쏨"이 됐나

- 시나리오 `IssueSquadOrder`의 `SetTargetable(false)`(`UI/ScenarioStateSubsystem.cpp` ~2732) / 구 이펙트 `ExcludeFleeingEnemiesFromAllyTargeting`(~2230)가
  ① 대상 플래그를 끄고 — `UDetectableTargetComponent::bTargetableByFriendlyForces`(SoldierLab), 구 레벨은 `UEnemyCombatComponent::bTargetableByAlliesAndUGV` —
  ② **UGV의** `URCWSFireControlComponent::bRespectEnemyTargetingExclusion`만 true로 켠다(트럭은 false 유지).
- 이 스위치를 본 곳은 사격 표적 선정(`Vehicles/RCWSFireControlComponent.cpp` ~538-545)과 총성 조사(~914)뿐. **탐지 스캔은 안 봤다** → 제외된 적도 `DetectedTargets`·HUD 박스·UDP에 그대로 있었다(guide에도 "탐지는 된다"고 명시돼 있었음).

## 3. 구현

### 3.1 `UTargetDetectionComponent` (`Detection/TargetDetectionComponent.h/.cpp`)

| 신규 | 기본값 | 내용 |
|---|---|---|
| `bHideTargetsExcludedByFireControl` (UPROPERTY) | true | 같은 액터의 FireControl이 안 쏘는 대상을 감지에서도 뺀다 |
| `ExcludedTargetFadeOutSeconds` (UPROPERTY) | 3 | 직전까지 감지돼 있던 대상의 화면 페이드 시간. 0 이하면 즉시 사라짐 |
| `FadingOutTargets` (배열) | — | **표시 전용.** 막 빠진 대상의 박스. UDP/사격/트리거는 안 읽음 |
| `IsHiddenByFireControlExclusion()` | — | 판정 함수(아래) |
| `ExclusionFadeStartTime` (TMap, private) / `CachedFireControl` | — | 페이드 시작 시각, 같은 액터의 FireControl(첫 스캔에 한 번 캐시) |

**판정** (`IsHiddenByFireControlExclusion`, `.cpp:154`): `bHideTargetsExcludedByFireControl` && `CachedFireControl` 있음 && 그 `bRespectEnemyTargetingExclusion == true` 이고, 대상이 `IsTargetableByFriendlyForces()==false` 또는 `UEnemyCombatComponent::IsTargetableByAlliesAndUGV()==false` → 숨김. 즉 **사격 표적 선정과 같은 두 플래그·같은 스위치.**

**`ScanTargets` 처리** (`.cpp:226-266`, `bIgnoreBreakingContactTargets` 바로 뒤):
1. 제외 대상은 `DetectedTargets`에 넣지 않고 `ActivelyDetected`/`TargetConfidence`/`LastActiveTime`를 지운다 → 제외가 풀리면 **처음부터 재획득**(히스테리시스·재획득 유예 없이).
2. 직전까지 감지(`ActivelyDetected`)였거나 이미 페이드 중인 대상만 `ExclusionFadeStartTime`에 시작 시각을 기록(`FindOrAdd`)하고, 그 시간 동안 `EvaluateTarget`으로 박스를 계속 계산해 화면 안(`bInView`)이면 `FadingOutTargets`에 넣는다(`Confidence=1`, `FadeOutStartTime`/`FadeOutDuration` 채움).
3. 페이드가 끝나도 제외가 유지되는 동안엔 시작 시각을 **보존** — 다음 스캔에 페이드가 다시 시작되지 않게.
4. 제외가 풀리면(판정 false) 시작 시각 삭제. 스캔 끝에서 죽은 키 정리(`.cpp:363`).
5. **처음부터 제외된 대상**(감지된 적 없음)은 아예 나타나지 않는다.

### 3.2 데이터 / 표시

- `FDetectedTarget`(`Detection/DetectionTypes.h:144-151`)에 `FadeOutStartTime`(-1 = 페이드 아님) / `FadeOutDuration` 추가. `FadingOutTargets` 항목만 채운다.
- `UDetectionOverlayWidget`(`UI/DetectionOverlayWidget.cpp`):
  - `SetDetectionsFrom`이 `LastDetections`에 `FadingOutTargets`를 append(`:186`).
  - `UpdateBoxTracks`가 `Detector->DetectedTargets`가 아니라 `LastDetections`를 순회(`:251`) → 페이드 대상도 칼만 추적이 유지돼 사라지는 동안 박스가 부드럽게 따라간다.
  - `BuildInfos`가 매 프레임 불투명도 `1 - (now - start) / duration` 계산(`:622-630`, ≤0이면 목록에서 제외) → `FDetectionMarkerInfo::DisplayOpacity`(신규, `UI/DetectionMarkerWidget.h:89`, 기본 1).
  - 선 박스는 색 알파에 곱(`:805`), WBP 마커는 `SetRenderOpacity`(`:984`).
- `UDetectionMaskCaptureComponent::RebuildShowOnlyList`(`Detection/DetectionMaskCaptureComponent.cpp:130-135`)가 `FadingOutTargets`도 실루엣 대상에 포함 — 마커가 사라지는 동안 윤곽이 먼저 꺼지지 않게.

### 3.3 설계 이유

- **별도 스위치를 만들지 않고 사격 스위치를 그대로 따른다** → 감지와 사격이 같은 순간·같은 조건으로 바뀌므로 "보이는데 안 쏨"이 구조적으로 불가.
- 시나리오 재시작: `URCWSFireControlComponent::ResetForScenarioRestart`가 스위치를 초기값(false)으로 되돌리므로 **추가 배선 없음**.
- 트럭은 스위치가 영영 false → 영향 없음. 드론 등 FireControl이 없는 액터도 `CachedFireControl==nullptr`라 영향 없음.
- 시나리오가 나중에 `SetTargetable(true)`(또는 구역 동사 `Occupy`가 배정을 초기화)로 대상 플래그를 되돌리면 UGV가 다시 감지·사격한다(일관).

## 4. 영향 범위

`DetectedTargets`에서 **즉시** 빠지므로(페이드는 화면 전용) 다음에서도 빠진다:
- LIG UDP 탐지 보고 — `Network/UGVRemoteControlSubsystem.cpp` ~869가 `DetectedTargets`를 순회.
- UGV RCWS 표적 후보.
- 시나리오 `EnemyDetected` 트리거(UGV 탐지기 기준인 것).

부수효과(코드 읽기 추정, 미실측): RCWS 스티키 표적 유지 조건이 "`DetectedTargets`에 있는가"(`RCWSFireControlComponent.cpp:672-699`)라, 제외 순간 이미 물고 있던 3분대원도 `TargetRetentionGraceSeconds`(1 s) 뒤 놓아준다 — `guide/rcws_fire_control_dev_guide.md` §3.3의 스티키 표적 틈([W106], "시야를 1 s 넘게 잃을 때까지 계속 쏜다")이 UGV에서는 최대 1 s로 줄어드는 셈. 실측으로 확인할 것.

## 5. 한계 / 주의

- **FireControl 스위치는 서버에서만 켜진다(복제 안 됨).** UGV 화면·UDP는 서버(Host)에서 돌아 문제없지만, 만약 원격 클라이언트에서 UGV 탐지를 돌리면 거기선 안 걸러진다.
- 페이드 중 박스는 화면 안이면 가림 여부와 무관하게 계속 그려진다(판정이 아니라 연출).

## 6. 바뀐 파일 (전부 p4 opened, 미제출)

- `Source/titan_example/Detection/TargetDetectionComponent.h` / `.cpp`
- `Source/titan_example/Detection/DetectionTypes.h`
- `Source/titan_example/Detection/DetectionMaskCaptureComponent.cpp`
- `Source/titan_example/UI/DetectionOverlayWidget.cpp` (같은 날 마스크 화면비 수정도 이 파일 — `2026-10-15_ugv_test_dashboard_stretch_with_fixed_viewport.md`)
- `Source/titan_example/UI/DetectionMarkerWidget.h`

## 7. 남은 작업 (사용자)

새 UPROPERTY/USTRUCT 필드라 **에디터 닫고 정식 빌드**(Live Coding 금지). 그 뒤 New_kadex_0811 데모로:

- [ ] 3차 Withdraw(`ExcludeFleeingEnemies`, Flee3 +4 s) 시점에 UGV 화면에서 3분대 박스가 약 3초 페이드 후 사라지고, 이후 다시 안 나타나는지
- [ ] 트럭 화면에서는 3분대가 계속 잡히는지(그리고 트럭이 쏴서 `Squad3Stand`가 걸리는지)
- [ ] LIG UDP(`ugv_rc_gui` 등)에서 3분대 객체가 빠지는지
- [ ] 시나리오 재시작 후 2회차에 UGV가 다시 정상 감지하는지
- [ ] (부수효과) 제외 순간 물고 있던 3분대원을 UGV가 1 s 안에 놓는지
