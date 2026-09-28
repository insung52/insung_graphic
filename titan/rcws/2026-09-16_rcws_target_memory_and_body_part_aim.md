# RCWS 성능 업그레이드 — 타겟 기억(경계도) + 마지막 위치 응시 + 부위 기반 탐지/조준

2026-09-16 / 진행중(코드·빌드·인스턴스 설정 완료, PIE 1차 확인 — 머리만 나온 적 획득/재출현/전방우선 세부 검증·튜닝 대기) / 타겟을 놓치면 마지막 조준점을 4초(초기 7초) 응시하고 재출현 시 즉시 재획득, 탐지 샘플을 뼈(머리/가슴/골반)로 바꿔 머리만 내놓은 적도 잡고 보이는 부위를 조준.

## 요청 / 배경

사용자 요청: "UGV/TitanTruck RCWS 성능 업그레이드". 현재 상태를 분석해 보니 문제는 두 갈래였다.

**1. 기억/경계도가 없다.** 타겟을 시야에서 놓치면 `TargetRetentionGraceSeconds`(1초) 뒤 곧장
탐색 스윕으로 복귀한다. 적이 2~3초만 엄폐해도 카메라가 딴 데 가 있고, 다시 나타나면 처음부터
탐색·획득·슬루·락온을 반복한다 — 1대1 교전에서 치명적. `LockOnGaugeDischargeSeconds`는 "발사
허가" 축이라 별개 문제(그쪽은 2026-09-01 `rcws/2026-09-01_target_retention_grace.md`에서 다룸).

**2. 머리만 내놓은 적을 영영 못 잡는다.** 탐지(`UTargetDetectionComponent`)는 콜리전 박스 중심 XY의
수직선 위 3점(높이 20/50/80%)을 트레이스하고, 3점 중 2점(Acquire 0.6 / Lose 0.5)이 보여야 획득한다.
머리만 보이면 1/3 = 0.33이라 획득 불가. 조준점도 메시 바운드 중심(몸통)이라 엄폐물에 박힌다.
Lean(옆으로 기댐)은 캡슐이 안 따라가서 샘플이 엄폐물 안이고, 엎드림(캡슐은 서 있음)은 샘플이 허공.

## 설계

### 탐지 절반 (`UTargetDetectionComponent`)

- **샘플 위치를 수직선 → 뼈로.** `BodyParts` 배열(기본 Head/Chest/Pelvis)의 뼈 위치에서 트레이스.
  뼈는 애니메이션 포즈를 따라가므로 Lean/엎드림에서도 실제 몸 위치를 본다. 스켈레탈 메시가
  없거나 후보 뼈가 전부 없으면 기존 수직선 폴백(`SamplePointCount`는 이제 폴백에만 적용).
- **획득 규칙 `AcquireRule`.** `VisibleFraction`(기존, 기본값 — 드론/CCTV 동작 유지) /
  `AnyVisibleSample`(RCWS용). 후자는 Confidence가 "1점이라도 보임"의 0/1을 쫓는다. Acquire/Lose
  임계값과 `ConfidenceLerpSpeed`는 그대로 남아 시간 게이트 역할 — 기본값 기준 획득 0.4초, 상실 0.33초.
- **재획득 유예 `ReacquireGraceSeconds`(10초).** 최근 이 시간 안에 `DetectedTargets`에 있던 대상이
  다시 1점이라도 보이면 Confidence를 `AcquireConfidenceThreshold`로 즉시 올려 재획득(lerp 대기 없음).
- **부위별 결과 노출.** `FDetectedTarget`에 `VisibleFraction`(원시 가시 비율)과
  `Parts`(`TArray<FDetectedTargetPart>` — Label/Bone/TowardBone/TowardFraction/WorldLocation/bVisible)
  추가. 파이어컨트롤은 `FindDetectedTarget()`으로 항목을 찾고, static `ResolvePartLocation()`으로
  **지금** 뼈 위치를 다시 계산한다(스캔 위치는 최대 0.1초 묵음 — 움직이는 적의 리드가 어긋남).

### 조준 절반 (`URCWSFireControlComponent`)

- **부위 조준 `AimPartPriority`** = [Chest, Pelvis, Head]. 탐지 `Parts` 중 **보이는** 부위를 이
  순서로 골라 조준. 가슴이 먼저인 이유는 탄 퍼짐 대비 명중 확률이고, 머리는 다른 부위가 안 보일
  때(머리만 내놓은 적)만 자연히 선택된다. 우선순위: `AimTargetBoneName` 지정(기존, 최우선) →
  보이는 부위 → 기억의 `LastAimPart`(유예 구간) → 바운드 중심(기존).
- **타겟 기억 `FRCWSTargetMemory`** {LastAimLocation, LastAimPart, LastSeenTime, Alertness} —
  `TargetMemories` 맵, 서버 전용. 보이는 타겟은 경계도가 `AlertnessRiseSeconds`(0.5)로 1까지, 나머지는
  `AlertnessDecaySeconds`(15)로 0까지(0이면 삭제). `LastAimLocation`은 **보이는 동안만** 갱신 —
  유예 구간에 엄폐물 뒤 실제 위치를 몰래 따라가면 안 되니까.
- **표적 선정 거리 가중.** 유효거리 = 실거리 × (1 − `AlertTargetDistanceBias`(0.5) × 경계도). 방금
  놓친 적이 새로 나타난 더 가까운 적에게 밀리지 않는다. 전방 원뿔 우선은 상위 규칙 그대로.
- **락온 충전 단축.** `LockOnGaugeChargeSeconds × Lerp(1, AlertLockOnChargeMultiplier(0.5), 경계도)`.
- **마지막 위치 응시.** 살아있는 타겟을 유예 후 놓쳤고 대체 타겟이 없으면 `WatchPoint =
  Memory.LastAimLocation`을 `WatchLastKnownSeconds`(4 — 초기 7, PIE 확인 후 사용자 지정) 동안 겨눈다. 응시 중에도 매 틱 표적 탐색 →
  누구든 보이면 즉시 Tracking. 타겟 **사망**(`UDetectableTargetComponent::IsIncapacitated`)이면
  응시 없이 즉시 놓는다(부수효과: 죽은 적을 유예 1초 동안 물고 있던 기존 동작도 사라짐).
- **스윕 블렌드 아웃.** 응시 만료(또는 추적 종료) 시 `ResumeSearchSweepFromCurrentAim()` —
  스윕 오프셋 = 현재 조준 방위 − 차체 heading(범위 클램프, 진행 방향은 중앙 쪽) → 큰 되돌림 없이
  `SearchSweepSpeedDegPerSec`로 자연스럽게 이어진다.

### 상태 흐름 (`ERCWSAutoAimPhase`, `AutoAimPhase`로 리플리케이트)

```
Searching ──(DetectedTargets에 적)──▶ Tracking
   ▲                                    │ 매 틱: 보이는 부위 조준, 경계도↑, LastAimLocation 갱신
   │                                    │ 탐지 유실 → 유예 TargetRetentionGraceSeconds(1s)
   │                                    │   (유예 중엔 LastAimPart를 계속 겨눔, 위치 갱신 없음)
   │                                    ▼
   │                      ┌── 대체 타겟 있음 ──▶ Tracking(새 타겟)
   │                      ├── 타겟 사망 ───────▶ Searching (ResumeSearchSweepFromCurrentAim)
   │                      └── 살아있고 대체 없음 ▼
   │                                WatchingLastKnown  (WatchPoint 응시, WatchLastKnownSeconds=4s)
   │                                    │ 매 틱 SelectNearestEnemyTarget → 누구든 보이면 Tracking
   └──(만료, ResumeSearchSweepFromCurrentAim)┘
```

Remote/AutoSurveillance 분기에서는 항상 `Searching`으로 리셋. 전환 로그:
`타겟 X 시야 상실(경계도 N) → 마지막 조준점 7.0초 응시` / `응시 종료 → 탐색 스윕 복귀` /
`추적 종료 → 탐색 스윕 복귀`. `bLogAutoFireReadiness` 로그에 `Phase=/Watch=/Alert=` 추가.

## 변경 파일

| 파일 | 내용 |
|---|---|
| `Detection/DetectionTypes.h` | `FDetectionBodyPart`{Label, BoneCandidates, TowardBoneCandidates, TowardFraction}, `FDetectedTargetPart`, `EDetectionAcquireRule` 신규. `FDetectedTarget`에 `VisibleFraction`/`Parts` 추가 |
| `Detection/TargetDetectionComponent.h/.cpp` | `BodyParts`(생성자 기본 3부위)/`AcquireRule`/`ReacquireGraceSeconds` 프로퍼티, `FindDetectedTarget()`, static `ResolvePartLocation()`, `LastActiveTime` 맵. `EvaluateTarget`이 뼈 샘플 → 수직선 폴백 두 갈래, `ScanTargets`가 획득 규칙·재획득 처리 |
| `Vehicles/RCWSFireControlComponent.h/.cpp` | `ERCWSAutoAimPhase`, `FRCWSTargetMemory`, 프로퍼티 7개(아래 표), `GetTargetAlertness()`. `SelectVisibleAimPart`/`SlewSightTowardWorldPoint`(추적·응시 공용 슬루 분리)/`UpdateTargetMemories`/`ResumeSearchSweepFromCurrentAim` 신규. `UpdateAutoAim` 상태 흐름 재구성, `GetTargetAimWorldLocation` 부위 우선, `SelectNearestEnemyTarget` 거리 가중, `UpdateFireReadinessGauges` 충전 단축. `AutoAimPhase` 리플리케이션 등록 |

기본 부위 정의(생성자): Head(`Head`,`head`; toward `HeadTop_End` 0.5 — Mixamo `Head` 뼈 원점이 턱
높이라 두개골 중심으로 옮김), Chest(`Spine2`,`spine_04`,`spine_03`), Pelvis(`Hips`,`pelvis`). 후보를
여러 개 두는 이유: 적군 스켈레톤 `/Game/Tutorial/Blueprints/Enemy/Enemy`는 Mixamo 이름, 아군은
UE5 마네킹 이름 — 대상 메시에 존재하는 첫 후보를 쓴다.

## 튜닝값 (신규 프로퍼티 전부)

| 프로퍼티 | 기본값 | 의미 |
|---|---|---|
| `TargetDetection.BodyParts` | Head/Chest/Pelvis | 뼈 샘플 부위. 비우면 항상 수직선 폴백 |
| `TargetDetection.AcquireRule` | `VisibleFraction` | `AnyVisibleSample`이면 1점만 보여도 획득. **UGV/트럭은 BP 기본값+레벨 인스턴스 모두 `AnyVisibleSample`로 설정 완료(2026-09-16, 아래 §인스턴스 설정 참고)** |
| `TargetDetection.ReacquireGraceSeconds` | 10 | 이 시간 안에 잡혔던 대상은 다시 보이면 즉시 재획득. 0=끔 |
| `FireControl.AimPartPriority` | [Chest, Pelvis, Head] | 보이는 부위 중 조준 순서. 없는 라벨은 제외 |
| `FireControl.WatchLastKnownSeconds` | 4 (초기 7) | 마지막 조준점 응시 시간. 0=끔(예전 동작) |
| `FireControl.AlertnessRiseSeconds` | 0.5 | 경계도 0→1(보는 동안) |
| `FireControl.AlertnessDecaySeconds` | 15 | 경계도 1→0(안 보는 동안), 0이면 기억 삭제 |
| `FireControl.AlertTargetDistanceBias` | 0.5 | 유효거리 = 실거리×(1−Bias×경계도). 0=끔, 최대 0.9 |
| `FireControl.AlertLockOnChargeMultiplier` | 0.5 | 경계도 1일 때 락온 충전 시간 배율. 1=끔 |
| `FireControl.AutoAimPhase` / `WatchRemainingSeconds` | (읽기 전용) | UI/디버그. Phase는 Replicated |

변경 전 MCP로 읽은 인스턴스 값(참고): UGV/트럭 TargetDetection — SamplePointCount 3, inset 0.2,
lerp 1.5, Acquire 0.6, Lose 0.5, MinScreenSizePixels 10/12, 400m, 0.1s. FireControl — UGV slew 20°/s,
락온 충전 0.6/방전 1.0, retention 1.0, burst 2/3s, sweep 7°/s, spread 1°; 트럭 slew 15, 0.6/2.5,
1.0, 2/3s, sweep 2.5, spread 0. 이 값들은 이번에 안 건드렸다.

## 기존 시스템과의 연동

- **전방 45° 우선**(`bPrioritizeForwardArcTargets`/`ForwardPriorityHalfAngleDegrees`) — 상위 규칙
  그대로. 경계도 가중은 원뿔 안 후보끼리 / 전체 후보끼리 각각 적용된다.
- **도주 분대 제외**(`bRespectEnemyTargetingExclusion` → `UEnemyCombatComponent::IsTargetableByAlliesAndUGV`)
  — `SelectNearestEnemyTarget` 안에서 가중 계산 전에 필터되므로 그대로 동작.
- **락온 게이지** — 타겟 identity 변경 시 0 리셋 규칙 유지. 응시 중엔 `CurrentAutoAimTarget=null`
  이라 `bModeWantsLock=false` → 게이지 0, 발사 없음. 재획득 시 경계도가 남아 있어 충전만 빨라진다.
- **시나리오 `EnemyDetected` 트리거** = `HasDetectedEnemyTarget()` = `SelectNearestEnemyTarget()!=null`
  그대로 — 응시 상태는 "탐지"가 아니므로 트리거에 영향 없음. 단 `AcquireRule=AnyVisibleSample`로
  바꾸면 트리거가 조금 더 일찍(머리만 보여도) 발동한다.
- **탐지 유실 유예**(`TargetRetentionGraceSeconds`, 2026-09-01) — 그대로 Tracking 안에 포함. 유예
  구간의 조준점이 바운드 중심으로 튀지 않도록 `LastAimPart`를 겨눈다.
- **엎드린 적 조준**(`AimTargetBoneName`, 2026-09-02) — 지정돼 있으면 여전히 최우선.

## 인스턴스 설정 (2026-09-16, MCP로 완료)

`TargetDetection.AcquireRule = AnyVisibleSample`을 네 곳에 썼고 `BP_UGV_0901`, `BP_TitanTruck`,
`New_kadex_0811` 저장:

| 대상 | MCP 경로 | 비고 |
|---|---|---|
| UGV 레벨 인스턴스 | `BP_UGV_0901_C_1.TargetDetection` | |
| 트럭 레벨 인스턴스 | `BP_TitanTruck_C_4.TargetDetection` | |
| UGV BP 기본값 | `BP_UGV_0901.BP_UGV_0901_C:TargetDetection_GEN_VARIABLE` | **BP에서 추가한 컴포넌트** → `_GEN_VARIABLE` 아키타입 |
| 트럭 BP 기본값 | `BP_TitanTruck.Default__BP_TitanTruck_C:TargetDetection` | **C++ `CreateDefaultSubobject`** → CDO 서브오브젝트 |

- **컴포넌트 아키타입 경로가 둘로 갈린다** — 컴포넌트를 BP에서 붙였으면 `<BP>.<BP>_C:<이름>_GEN_VARIABLE`,
  C++ 생성자에서 붙였으면 `<BP>.Default__<BP>_C:<이름>`. 앞으로 MCP로 BP 기본값을 만질 때 참고.
- **`.umap`에 값이 안 보이는 건 정상.** BP 기본값을 먼저 맞추면 레벨 인스턴스의 값이 아키타입과
  같아져 델타 직렬화에서 빠진다 — `grep AnyVisibleSample New_kadex_0811.umap`이 0건이어도 실패가
  아니다(두 BP `.uasset`에는 들어 있음). 인스턴스 값은 MCP `get_properties`로 확인할 것.
- 뼈 기반 `BodyParts` 기본값(Head/Chest/Pelvis)은 빌드 후 인스턴스에 생성자 기본값이 그대로
  전파된 것을 확인했다 — 별도 설정 불필요.

## 남은 작업 / 검증 체크리스트

1. [x] **빌드 — 에디터 닫고 풀 빌드.** 새 UPROPERTY/USTRUCT/UENUM이 많아 Live Coding 금지.
   2026-09-15에 `BarrelSpinGaugeValue`가 "missing property"로 뜬 사고가 Live Coding 리로드에서
   났다(09-15 스윕 문서에도 함정으로 기록). 2026-09-16 풀 빌드 완료.
2. [x] **인스턴스 설정** — `TargetDetection.AcquireRule = AnyVisibleSample`을 UGV(`BP_UGV_0901` +
   레벨 인스턴스 `BP_UGV_0901_C_1`)와 트럭(`BP_TitanTruck` + `BP_TitanTruck_C_4`) **둘 다**에.
   CDO 쓰기는 레벨 인스턴스에 전파 안 됨(`feedback_mcp_cdo_write_skips_level_instances`) — 그래서
   네 곳 전부 직접 썼다(위 §인스턴스 설정).
3. **PIE 검증** — 1차 "잘 되는 것 같다"까지 확인, 아래 세부 항목은 개별 확인 대기
   - [ ] 머리만 내놓은 적: 획득되고(로그 `visible=0.33 active=true`) 조준점이 머리로 감
   - [ ] Lean/엎드림: 샘플이 몸을 따라가고 조준이 엄폐물에 안 박힘
   - [x] 엄폐 후: 1초 유예 → `시야 상실 → N초 응시` 로그, 카메라가 그 자리에 머묾 (PIE 1차 확인
         "잘 되는 것 같다", 이후 응시 7→4초로 조정)
   - [ ] 재출현: 즉시 Tracking(재획득 지연 없음), 락온 충전이 경계도만큼 빨라짐(경계도 1이면
         절반 — 4초 응시 뒤면 경계도 ≈0.73이라 UGV 기준 0.6s → 약 0.38s)
   - [ ] 응시 만료: `응시 종료 → 탐색 스윕 복귀`, 스윕이 현재 방향에서 이어짐(되돌림 없음)
   - [ ] 사망: 응시 없이 바로 스윕 복귀
   - [ ] 전방 우선 유지: 후방 경계 타겟보다 전방 신규 타겟이 먼저(원뿔 안 후보가 있을 때)
   - [ ] 트럭(`BP_TitanTruck_C_4`)에서도 위 항목 동일하게 동작
   - [ ] 드론(`ADronePawn`) 탐지 회귀 없음(샘플 위치만 뼈로 바뀌고 규칙은 `VisibleFraction`)
4. **튜닝** — 응시 4초(PIE 1회 확인 후 7→4로 조정됨)/경계도 감쇠 15초/거리 가중 0.5가 실제 교전에서 적절한지.
5. `guide/rcws_fire_control_dev_guide.md` §3/§3.1/§5, `guide/detection_dev_guide.md` §3.4에
   2026-09-16 단락 추가함.

## 알려진 한계

- 클라이언트의 `UpdateAimPointForUI`가 부르는 `SelectNearestEnemyTarget`은 `TargetMemories`가
  비어 있어 가중 없이 선정 — 화면 마커용이라 무해.
- 뼈 트레이스가 적이 든 무기 액터(별도 액터)에 막힐 수 있음 — 기존 수직선 샘플도 같았음.
- 드론도 같은 `UTargetDetectionComponent`라 샘플 위치가 뼈로 바뀐다(규칙은 기존 유지).
  회귀가 보이면 드론 인스턴스의 `BodyParts`를 비워 수직선 폴백으로 되돌릴 수 있다.
- `WatchPoint`는 유예 진입 전 마지막 **보이던** 조준점이라, 적이 엄폐 직전 이동 중이었다면
  실제 엄폐 지점과 조금 어긋난다(의도 — 엄폐물 뒤를 따라가지 않기 위함).
- `TargetMemories`/`WatchPoint`는 리플리케이트하지 않는다(`AutoAimPhase`만). 클라 UI가 응시
  지점을 그리려면 별도 노출 필요.
