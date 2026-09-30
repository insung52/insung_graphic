# AI 조준에 진짜 피치 + 조준 보정 시간 기반화 + 선회 2단 속도

2026-09-30 / 완료(AI 조준 떨림 — 녹화 8회 후 사용자 "해결" 판정, 남은 것은 14절) / AI 조준 떨림의 원인을 프레임 단위 조준 녹화기(`SoldierLab.Debug.AimTrace`)로 하나씩 갈라 고쳤다 — ① AI 컨트롤 피치 0(엔진 규칙) → `ASoldierAIController`가 교전 AimRotation(피치 포함)을 컨트롤 회전으로 ② 선회 오차 비례 40~150°/s ③ 조준 보정 재설계(빠른 추종 10/s · 총이 조준에 있을 때만 적분 · 밖에선 누설 · 60°/s 상한) ④ 자세 축 리밋 사이클 + 웅크림 슈미트 트리거 ⑤ 서기/앉기 AO 관성 전환 ⑥ 뛰는 동안 조준 해제(히스테리시스 + 최소 유지 + 몸통 회전 상한) ⑦ 몽타주 BlendIn · 슬롯 그룹 분리 · 사격 금지 = 재장전 몽타주 길이.
~~2026-09-30 / 진행중(코드·에디터 빌드·`AIC_Soldier` 부모 변경 완료 — PIE 확인 대기) / AI 컨트롤 회전은 엔진 규칙상 피치가 항상 0이라 총 위아래를 틱당 5% 보정기가 떠맡고 있었다 — 자세 임계값·사격 자세 전환에서 총구가 1프레임에 튀던 원인. 컨트롤 회전 = 교전 AimRotation(피치 포함), 보정은 초 단위로 느리게, 선회는 오차 크기별 속도로.~~ (최초 헤더 — "보정은 느리게"는 6.2절에서 정정됐다)

관련: `animation/prototypes/2026-09-11_muzzle_aim_alignment.md`(조준 보정 원형) · `animation/prototypes/2026-09-12_body_yaw_rate_and_aim_antiwindup.md` · `ai/2026-09-14_exposure_ladder_and_corrections.md` 3.1절(AimSlew 240 도입, P86) · `ai/2026-09-14_cover_frame_fix_and_observer.md:201`("AI 컨트롤러에서 pitch 0" 기록) · 미결 C-93(선회 속도).

---

## 1. 증상 [A · 사용자, slomo 0.1 + `SoldierLab.Debug.Engagement 1`]

1. 표적이 바뀌면 조준이 너무 빨리 돈다(1프레임 스냅은 아님).
2. 조준선 색(사격 의도)이 바뀔 때 가끔 틱틱 튄다.
3. **AI가 자세를 올리며 앉기/서기 임계값을 지나는 순간 총구가 1프레임에 아래로 튄다.** 플레이어가 조준한 채 V/B로 같은 임계값을 넘으면 부드럽다.

## 2. 원인 [A · 엔진 코드 + 에셋 실측]

- `AAIController::UpdateControlRotation`(UE 5.8 `AIController.cpp`): 컨트롤 회전 = `(초점 − GetPawnViewLocation()).Rotation()`, 그리고 **초점이 폰이 아니면 피치 0**. 교전 층은 `SetFocalPoint`(점)로 조준하므로 **AI 컨트롤 피치는 항상 0**이었다. AO(조준 오프셋)는 위아래 입력을 못 받았다.
- 그래서 AI 총의 위아래는 **`AimCorrection`(±25°, 틱당 오차 5% 적분)** 이 전부 맞추고 있었다. 틱 단위라 slomo에서도 정상 속도로 움직였다(조준 체인에서 유일하게 게임 시간을 안 쓰는 값).
- 임계값에서는 `Crouch()/UnCrouch()` · 서기↔앉기 DB 교체 · 눈 높이 100→32 cm가 한 프레임에 일어난다. 이전 자세 기준으로 쌓인 보정이 새 자세에 얹혀 튀었다(3번). 사격 자세·조리개 전환도 같은 구조(2번, 추정).
- 선회는 240°/s 등속 — 30° 전환이 0.125초(1번). 4° 미만 조준점 흔들림은 60fps에서 한 프레임에 따라감.
- 기각: StateTree `STT_FocusToPlayer`(`AIC_Soldier.bStartLogicAutomatically = false`라 미실행) · 초점 해제(사망 시만) · 컴포넌트 틱 간격(전부 0) · 사격 반동(산포만) · 린/자세 축(PoseSmoother 초당 속도로 연속).

## 3. 수정 (코드)

| 파일 | 내용 |
|---|---|
| **신규** `AI/SoldierAIController.{h,cpp}` | `ASoldierAIController : AAIController` — `UpdateControlRotation` 오버라이드. 초점이 유효하고 교전이 조준한 적이 있으면 **컨트롤 회전 = `Engagement->GetAimRotation()`(선회 후, 피치 포함, roll 0)**. 아니면 엔진 원래 동작. cvar **`SoldierLab.Aim.RealPitch`**(1, 0 = 엔진 규칙 A/B) |
| `AI/SoldierEngagement.h/.cpp` | `GetAimRotation()` 게터 · **`SlewAim()`** — 선회 속도를 오차에 비례: 3° 이하 **40°/s**, 30° 이상 **`AimSlewDegreesPerSecond` 150°/s**(240→150), 사이는 선형. 두 `RInterpConstantTo` 호출(교전 · 수색 방향) 교체. UPROPERTY `AimSlewNearDegreesPerSecond`/`AimSlewNearErrorDeg`/`AimSlewFarErrorDeg` 신설 |
| `Pose/SoldierAIBridgeComponent.cpp` | 조준 보정 시간 기반화: 게인 = `1 − exp(−rate·dt)`(~~**`SoldierLab.Aim.CorrectionRate` 1.5/s**~~ → 정정: 6.2·7절, **10/s**, 기존 ≈3/s) · 데드밴드 **`SoldierLab.Aim.CorrectionDeadbandDeg` 0.3°**(대기 흔들림 추종 방지) · 게이트 틱당 2° → **`SoldierLab.Aim.CorrectionGateDegPerSec` 120°/s**. 플레이어에게도 같이 적용 |

같은 날 별건: `SoldierLab.Engagement.BlindFire`(기본 0) — 맹목사격 임시 비활성(`animation/2026-09-30_diagonal_aim_stop_selection.md` 6절).

## 4. 남은 절차

1. ~~에디터 타깃 빌드~~ ✅ (사용자)
2. ~~`AIC_Soldier` 부모 변경~~ ✅ MCP `set_parent` → 컴파일 → 저장(P4 체크아웃). 실제 옛 부모는 GASP BP가 아니라 **`AIController`(C++)** 였다 — `AIC_Soldier`는 GASP `AIC_NPC_SmartObject`의 복제본이라 변수·함수를 자체 보유, 잃은 것 없음. `bStartLogicAutomatically = false` 유지 확인. `BP_Soldier_Friendly`/`_Hostile` 모두 `AIC_Soldier_C` 사용.
3. ~~교전 컴포넌트 값~~ ✅ `AimSlewDegreesPerSecond 150 · Near 40 · NearErr 3 · FarErr 30`으로 따라옴.
4. ~~PIE: 임계값 통과 시 총구 · 표적 전환 속도 · 조준선 색 전환 시 튐 · 높은/낮은 표적에 총이 실제로 위아래를 향하는지~~ ✅ 5~13절(녹화 8회로 측정 — 떨림 잔존 → 원인별 수정 → 사용자 "해결" 판정). · 2-PC 클라(원격 피치는 `RemoteViewPitch`로 복제됨) — **이 문서에 확인 기록 없음** → [C-178].
5. ~~값 조정: 선회 `AimSlewNear*`/`AimSlewDegreesPerSecond`(C-93) · 보정 `SoldierLab.Aim.Correction*`.~~ ✅ 보정은 7·10절에서 재설계(`CorrectionRate` 10 · `LeakRate` 6 · `MaxDegPerSec` 60 신설), 선회는 3절 값(150 · Near 40 · 3°/30°) 그대로 사용자 판정 통과 → [C-93] 닫음, 남은 값은 [C-178].

## 5. PIE 결과 → 떨림 잔존 → 프레임 단위 조준 녹화기 [W — 2026-09-30]

사용자: 수정 후에도 **엄폐 뒤에서 자세를 낮출 때 프레임 단위로 도리도리하며 떨린다.** 조준 체인에 쓰는 곳이 7~8군데라 눈·추정으로는 발원지를 못 가른다 → 측정으로 전환(P10).

**`Source/SoldierLab/Debug/SoldierAimTrace.{h,cpp}`** — `USoldierAimTraceSubsystem`(월드 서브시스템, BP 변경 없음, 꺼져 있으면 비용 0). `SoldierLab.Build.cs`에 `PoseSearch`·`BlendStack` 의존성 추가. `USoldierEngagementComponent`에 `friend` 선언(내부 판단 상태 읽기).

| cvar | 뜻 |
|---|---|
| `SoldierLab.Debug.AimTrace 1` / `0` | 기록 시작 / 중지(파일 경로가 출력 로그에 찍힘) |
| `SoldierLab.Debug.AimTrace.Filter <이름 일부>` | 비우면 전원(기본). 교전 컴포넌트를 가진 모든 캐릭터 |

출력: `Saved/AimTrace/<시각>/<병사>.csv` — 병사당 파일 하나, 프레임당 한 줄. 열(체인 순서):
frame·time·dt·dilation → 교전(접촉·표적·잠금·전환 수·확신도·의도·조리개·자세·peek·scan·장전·자세/린/BF 목표·실제·aimPoint·기준 총구·원하는 조준·선회 후 조준) → 컨트롤러(회전·초점)·`GetBaseAimRotation` → 몸(위치·회전·속도·crouch·MaxWalkSpeed·yaw rate·가속·메시 상대 위치) → **모션매칭(선택 DB·클립·시간·비용·continuing) + 블렌드 스택 5칸(클립·시간·blend-in·유효 가중치·재생배율)** · 몽타주 → **본 월드 변환**(pelvis·spine_05·neck_01·head·clavicle_l/r·hand_l/r·weapon_r·ik_hand_gun) → **총구 소켓 변환·총열 방향·조준/컨트롤 대비 오차·액터 대비 방향** → **ABP·캐릭터 BP의 숫자/bool/enum/벡터/회전 변수 전부 자동 덤프**(`abp.*`, `chr.*`).

판정 방법: 떨림 구간에서 **가장 먼저 진동을 시작하는 열이 발원지**. 특히 `muzzle.*`가 튄 프레임에 `stack*.anim`이 바뀌었으면 "시퀀스 간 총구 방향 차이", 클립이 그대로인데 튀면 "애니메이션 시스템(보정·AO·자세 레이어 등) 자체".

빌드: 게임 타깃 컴파일 성공(Build.cs·헤더 변경 → 에디터 재빌드 필요).

## 6. 녹화 분석 결과 — `Saved/AimTrace/20260930_150415` (3대3, 63초, 6명 × 3804프레임) [A]

기준: `muzzle.errVsCtrl`(총열 − 컨트롤 회전)의 프레임 간 변화. 컨트롤 회전은 프레임당 최대 2.5~2.8°로 부드러웠고, 총열만 ±10°/프레임으로 부호가 번갈아 튀었다 → **조준 판단 쪽 무죄, 애니메이션 쪽 발원.**

### 6.1 원인 ① — 자세축이 임계값 위에서 리밋 사이클 → 웅크림 매 프레임 토글 (적군 떨림의 주범)

- AI 목표 자세(`AITargetStance`/교전 `DesiredStance`) = **0.5000**, `StanceThreshold` = **0.5000**.
- PoseSmoother의 사다리꼴 운동이 `√(2ad)` 제동을 이산 프레임에 그대로 써서 목표를 넘고 되돌아오기를 반복 — `StanceAxis` 0.4995 ↔ 0.5006, 속도가 스냅 기준(0.02)보다 항상 커서 착지도 안 함.
- 브리지 `UpdateStance`의 `InRange(Stance, Threshold, 2)`가 매 프레임 뒤집힘 → `Crouch()/UnCrouch()` → ABP `Stance` → 모션매칭 DB 서기↔앉기 교체 → 블렌드 스택 맨 위 클립이 `Enemy_MM_Rifle_TurnLeft_90`(서기) ↔ `Crouch_TurnLeft_180`로 매 프레임 교체 → 총구 ±10°.
- 수치: 큰 튐(>5°/프레임) 중 웅크림 토글과 같은 프레임 — Hostile 68건 중 54, Hostile2 55 중 23(+클립 교체 9), Hostile3 23 중 10(+6). 토글 횟수 49~121회/분.
- 플레이어는 입력을 떼면 축이 멈추고 임계값 위에 정확히 머물지 않아 무관(사용자 관찰과 일치).
- **"시퀀스마다 총구 방향이 달라서"가 아니라 시스템 문제** — 클립 교체는 결과.

### 6.2 원인 ② — 조준 보정 적분기 와인드업 + 느린 추종

- 재장전 몽타주(총을 크게 기울이는 게 정상) 동안에도 보정이 적분 → **±25° 한계에 들러붙음**(병사당 157~554프레임). 끝난 뒤 최대 **5~12초** 동안 총이 조준과 어긋남.
- 보정을 넣은 상태의 총구 오차(몽타주 제외, AO 켜짐): 앉아 정지 중앙 5.8°/p90 14.0° · 서서 정지 5.3/13.4 · 앉아 이동 7.3/17.9 · **서서 이동 12.5/32.9** · 서서 이동+린 13.2/**42.7**.
- **정정**: 4절에서 보정을 "현실적으로 느리게"(1.5/s) 만든 것은 개념이 틀렸다. 총의 **움직임** 속도는 선회(`AimRotation` 40~150°/s)가 정한다. 보정은 "그 조준에 총구를 맞추는 것"이라 빨라야 하고, 느리면 총이 부드러워지는 게 아니라 **어긋난 채로 남는다.**

## 7. 근본 수정 (코드, ~~미빌드 — cpp만 변경이라 Live Coding 가능~~ → 적용 후 8절에서 재녹화 확인)

| 파일 | 내용 |
|---|---|
| `Pose/SoldierPoseSmootherComponent.cpp` `StepAxis` | 원하는 속도에 **"이번 프레임에 목표에 도달하는 속도"(`|d|/dt`) 상한** 추가 + **한 스텝에 목표를 넘으면 목표에 착지·정지.** 자세·린·맹목사격 축 전부의 리밋 사이클 제거 |
| `Pose/SoldierAIBridgeComponent.cpp` `UpdateStance` | 웅크림 판정을 **슈미트 트리거**로: 현재 `bWantsToCrouch`를 기억, 앉기는 `임계값 + 여유폭`, 일어서기는 `임계값 − 여유폭`을 넘어야 전환. cvar **`SoldierLab.Pose.CrouchHysteresis`**(0.05). 플레이어 V/B에도 적용 |
| 같은 파일, 조준 보정 | 재설계 — ① **총구가 조준을 향해야 할 때만 적분**: AO 켜짐 · 몽타주 없음(`IsAnyMontagePlaying`) · 피격 반응 없음 · 총 내림 < 0.05. ② 그 밖에는 **0으로 누설**(기존: 붙잡기) — `SoldierLab.Aim.CorrectionLeakRate` 6/s. ③ 추종을 빠르게 — `SoldierLab.Aim.CorrectionRate` 1.5 → **10/s**(시간상수 ≈0.1 s). ④ 조준이 게이트보다 빨리 돌 땐 유지(기존 안티 와인드업 규칙 그대로) |

`SoldierMovementProfile`의 `Crouched` 상황 판정은 **목표 자세**(흔들리지 않는 입력)를 보므로 수정 대상 아님. BP 복사 경로(`SoldierLab.AIBridge.Native 0`)의 옛 판정은 그대로.

### 검증 절차

Live Coding(또는 빌드) → 같은 3대3 장면을 `SoldierLab.Debug.AimTrace 1`로 다시 녹화 → 이번 수치와 비교: **웅크림 토글 횟수**(목표: 자세 변경 시에만) · 큰 튐 건수 · **보정 포화 프레임**(몽타주 중 0 목표) · 재장전 후 복귀 시간 · 자세별 총구 오차 중앙값/p90.

## 8. 재녹화 — `Saved/AimTrace/20260930_152730` (69초) [A]

| 지표 | 이전 | 수정 후 |
|---|---|---|
| 앉기/서기 토글 | 38~121회 | **7~22회** |
| 토글에서 생긴 큰 튐(Hostile) | 54 | **2** |
| 재장전 중 보정 포화 | 85~382프레임 | **0~3** |
| 재장전 후 복귀 2초 초과 | 3~12회 | **0** |

남은 재장전 튐 — 보정·컨트롤·몸통은 조용했고, 둘 다 몽타주 애셋:
- **시작 0~0.15초**(적군 30건/12회, 최대 16°/프레임): 몽타주 `BlendIn` **0.1초**가 짧아 조준→재장전 상체 전환(`spine_05` yaw ~70°)이 0.1초에 끝남. → **두 재장전 몽타주 `BlendIn` 0.1 → 0.25초**(HermiteCubic 유지, 저장, P4 체크아웃). 재생은 캐릭터 BP `PlayAnimMontage` — 에셋 블렌드 값을 쓴다.
- **1.6초**(아군만, 55건/11회, 최대 12°/프레임, 적군 5건): `hand_r`이 0.1초에 ~55° 회전 — **아군 클립에 저작된 동작.** 아군 몽타주 `UpperBodyAdditive` 트랙은 09-29 디자이너가 추가한 `ALLY_MM_Rifle_Reload_Additive1`, 적군은 원본 `Enemy_MM_Rifle_Reload_Additive`. → **디자이너에게 1.5~1.7초 `hand_r` 키 정리 요청**(미조치).
- Hostile2는 몽타주 밖 보정 포화 398프레임 — 특정 자세에서 총구 오차가 ±25°를 넘는다(아래 과제).

## 9. 3차 녹화 — `Saved/AimTrace/20260930_153415` — 일어설 때 튐 [A]

전체 튐은 병사당 4~38건(대부분 사격·재장전 몽타주)으로 더 줄었다. 사용자 관찰("앉아 있다 일어날 때 튄다")을 전환만 떼어 측정:

| 전환(몽타주 없는 구간) | 건수 | 전환 ±10프레임 총구 최대변화 중앙 / p90 / max |
|---|---|---|
| **일어나기 1→0** | 25 | 1.1 / **11.4** / **12.8°** — 전부 전환 그 프레임 |
| 앉기 0→1 | 22 | 1.7 / 3.1 / 3.6° |

전환 프레임(Hostile 21.98 s): 캡슐 z·메시 상대 z는 상쇄돼 골반 월드 높이 연속 ✅, 새 서기 클립 가중치 0.00(블렌드 무관), 보정 변화 0.5° — 그런데 **`pelvis` yaw −1.9 → −61.0°(59°), `hand_r` yaw 9.3 → 23.1°(14°)가 한 프레임에**, `spine_05`·`clavicle_r`은 연속.

**원인 [A · MCP 그래프 추적]**: ABP가 AO 애셋을 **`Select(Stance == Crouch)`로 `BlendSpacePlayer_1`의 BlendSpace 핀에 직접 꽂는다**(아군 `ALLY_AO_Rifle_ADS`/`_Crouch`, 적군 `Enemy_AO_Rifle_ADS`/`_Crouch`, `UseAllyAnimSet`로 선택). 애셋 핀이 바뀌면 플레이어는 그 프레임에 새 AO로 즉시 전환한다. 아래쪽 `BlendListByBool_0`(`Enable_AO`) → `DeadBlending_0`이 있지만 **관성 블렌드는 요청이 있어야** 동작한다. 09-02 설계 문서의 미결 **[W11]**("아직 이진 Select")이 이것.

**수정**:
- **신규 `Pose/SoldierAnimLibrary.{h,cpp}`** — `USoldierAnimLibrary::UpdateStanceAimOffset(UpdateContext, Node, AllyStand, AllyCrouch, EnemyStand, EnemyCrouch, BlendTime=0.25)`(BlueprintThreadSafe). 애님 인스턴스의 `Stance`(1 = 웅크림)와 `UseAllyAnimSet`으로 원하는 AO를 고르고, 현재와 다르면 **`UBlendSpacePlayerLibrary::SetBlendSpaceWithInertialBlending`** — 아래쪽 DeadBlending이 차이를 0.25초에 걸쳐 흘려보낸다. 첫 업데이트는 즉시 설정. cvar **`SoldierLab.Pose.AimOffsetInertialSwitch`**(1, 0 = 옛 즉시 교체 A/B).
- `SoldierLab.Build.cs` — `AnimGraphRuntime` 의존성.
- **ABP 배선 ✅ (재빌드 후 MCP + 사용자 체크박스 3개)**: 새 함수 **`OnUpdate_StanceAimOffset`**(`Context`·`Node` by-ref, **Thread Safe**) → `UpdateStanceAimOffset` 호출(AO 4개 · BlendTime 0.25 핀 지정). `BlendSpacePlayer_1`: `Select_2 → BlendSpace` 연결 해제, 기본 애셋 `ALLY_AO_Rifle_ADS`(런타임에 함수가 덮어씀), **On Update = `OnUpdate_StanceAimOffset`**. 컴파일·저장 확인. 옛 Select 체인(`K2Node_Select_0/1/2`·`EnumEquality_0`)은 연결 없이 남아 있음(순수 노드라 무해, 정리 대상).
  - ⚠ MCP 한계: 함수 파라미터의 **Pass-by-Reference**와 함수의 **Thread Safe** 플래그는 MCP로 설정 불가(`K2Node_FunctionEntry` 메타데이터 비노출) — 애님 노드 함수를 MCP로 만들 땐 이 3개를 사용자가 체크해야 컴파일된다("Update function's signature is not compatible" / "not thread safe").

## 10. 4차 녹화 — `Saved/AimTrace/20260930_154726` (84초) — 종합 점검 [A]

(사망 후 프레임 — 초점 무효 — 은 통계에서 제외. Hostile2의 컨트롤 8.9° 튐은 사망 순간의 `ClearFocus`였다.)

| 지표 | 최초 15:04 | 지금 |
|---|---|---|
| 앉기/서기 토글 튐 | 75 | **2** |
| 일어나기 전환 p90 / max | 10.7 / 13.3° | **3.4 / 6.8°** |
| 정지 총구 오차 중앙 | 5~6° | **1.7°** |
| 재장전 시작 적군 최대 | 16.3°/f | **7.4°/f** |

남은 것:
1. **뛰는 중 총구 오차**(몽타주 제외, AO 켜짐): 정지 1.7/9.7° · 걷기 4.3/18.4° · 중간 13.5/38.1° · **뛰기 13.0/50.4°, 보정 포화 48%**. 뛰기 클립 위의 AO는 총을 70~80° 비켜 든다 — 뛰면서 조준하는 애셋이 없다.
2. **보정 재가동 튐**: 피격 반응 등으로 누설된 뒤 다시 켜질 때 오차 70~80°를 오차 비율로 따라가 한 프레임 10~12°.
3. 아군 재장전 1.6초 손목(51건/18회) — 사용자가 애셋 확인.
4. 사격(42)·피격(26) 몽타주 튐은 의도된 동작.

**수정 (브리지 cpp, 헤더 무변경 → Live Coding)**:
- **뛰는 동안 조준 안 함(사용자 결정 a)**: AI `WantsToAim = HasContact && !달리기`. 지면 속도 > **`SoldierLab.Aim.JogAimOffSpeed` 400**이면 조준 해제, < **`JogAimOnSpeed` 340**이면 다시 조준(현재 입력 상태를 래치로 쓰는 여유폭). 뛰는 동안은 클립이 저작된 대로 총을 든다. 사격 중엔 이동 정책이 Walk(≤291)로 묶으니 조준 유지. 플레이어 무관.
- **보정 속도 상한**: 적분·누설 모두 축당 **`SoldierLab.Aim.CorrectionMaxDegPerSec` 60°/s**를 넘지 않는다.

## 11. 5차 녹화 — `Saved/AimTrace/20260930_155624` (42초) [A]

- 뛰기 구간은 AO가 꺼져 통계에서 빠짐 ✅. 일어나기 p90 3.4 / max **3.7°** ✅. 보정 포화 대부분 0~56프레임(Hostile2만 195).
- **새 문제 — 조준 켜기/끄기 전환**: 몽타주 무관 튐 136건 중 **120건이 회전 모드·AO 전환 ±15프레임**. 전환 순간 **몸통 회전 속도 중앙 561~719, 최대 795°/s**.
  - 원인 ①: 몸통 회전 속도 = `Lerp(YawRate_Up 90, YawRate_Down 720, WeaponLowered)`. 뛰다 멈춰 **조준이 다시 켜지는 순간 총은 아직 완전히 내려가 있어(1.0) 720°/s** — 두 프레임에 조준 방향으로 스냅(실측 ~1000°/s).
  - 원인 ②: 뛰다 멈추다를 반복하면 속도가 400↔340 구간을 0.2초 만에 오가서 조준이 켜졌다 꺼졌다 한다(예: 56.20 해제 → 56.40 재개).

**수정 (브리지 — 헤더 멤버 추가, 에디터 재빌드 필요)**:
- 조준 전환 **최소 유지 시간** `SoldierLab.Aim.JogAimMinHoldSeconds` **0.8초**(멤버 `LastAimToggleSeconds`). 접촉을 잃으면 즉시 해제는 유지.
- **AI 몸통 회전 속도 상한** `SoldierLab.Aim.AIMaxBodyYawRate` **360°/s**(0 = 제한 없음). 플레이어 무관.

## 12. 6차 녹화 — `Saved/AimTrace/20260930_160242` (65초) — 확인 [A]

- 조준 전환 시 몸통 회전 **최대 795 → 365~406°/s** ✅(상한 360). 전환 순간 몸 기준 총 움직임 중앙 ~7°/프레임 = AO 블렌드(0.25~0.375초)로 총을 올리고 내리는 속도 — 정상.
- 일어나기 max 3.9°, 앉기 max 2.4° ✅. 보정 포화 Hostile2 195 → 24프레임.
- 컨트롤 5~6.6°/프레임 2건은 프레임 지연(dt 0.063/0.035 s × 150°/s) — 정상.
- 총구 오차(몽타주·사망·전환 ±0.5초 제외, 중앙/p90): 서서 정지 1.4/7.5 · 앉아 정지 1.7/10.3 · 서서 걷기 2.6/14.4 · **앉아 걷기 4.5/23.9(p99 50)** · 뛰기 사실상 없음(조준 안 함).
- 남은 것: 앉아 걷기 조준 오차(웅크려 걷기 클립 팔 자세 vs `AO_Rifle_Crouch` — 같은 부류), 아군 재장전 1.6초 손목(사용자 확인 중), 사격·피격 몽타주(의도).

## 13. 피격 → 사격 전환 시 머리·상체 끊김 [A]

사용자: "피격 몽타주가 재생 중인데 다른 몽타주가 재생되면 기존 것이 끊기는 것 아닌가." 6차 녹화에서 몽타주→다른 몽타주 직행 전환을 전수:

| 전환 | n | 머리 °/프레임 중앙/최대 | 척추 최대 | 끊긴 위치 |
|---|---|---|---|---|
| **피격 → 사격** | 9 | **76.5 / 148.4** | 163.8 | 피격 0.12 s |
| 사격 → 피격 | 11 | 13.5 / 105.1 | 168.3 | 사격 0.13 s |
| 사격 → 종료(정상) | 122 | 1.1 / 6.8 | 21.8 | — |

예: Hostile 71.37 s — 피격이 `spine_05`를 178→52°로 비트는 중 사격 몽타주 시작 → 한 프레임에 52→−174°(≈130°). HeadAim 알파 0(무관).

**원인**: 스켈레톤 `SK_UEFN_Mannequin`의 슬롯 그룹이 **`DefaultGroup` 하나** — `AdditiveHitReact`(피격)·`FullBodyAdditivePreAim`(사격)·`UpperBody`(재장전)·`DefaultSlot`(사망)이 전부 같은 그룹. 언리얼은 몽타주 재생 시 **같은 그룹의 기존 몽타주를 새 몽타주의 BlendIn 시간으로 멈춘다** — 사격 `BlendIn = 0` → 피격이 한 프레임에 사라짐. 피격 Med/Hvy도 `BlendIn = 0`.

**수정**:
1. **슬롯 그룹 분리** — `AdditiveHitReact`를 새 그룹(`HitReact`)으로. 피격(애디티브)이 사격·재장전과 서로 끊지 않고 겹친다. 사망은 `bStopAllMontages=true`라 영향 없음, `IsHitReacting`은 슬롯 이름 기준이라 그대로. ⚠ MCP로 불가(`USkeleton::SlotGroups` 비노출 — API `AddSlotGroupName`/`SetSlotGroupName`은 있으나 MCP가 호출 못 함) → **에디터 Anim Slot Manager에서 수동**(스켈레톤 체크아웃 필요). ✅ 사용자 적용(13.1절).
2. ✅ **피격 Med/Hvy 몽타주 12개 `BlendIn` 0 → 0.05초**(Linear 유지, 저장) — 연속 피격 시 이전 피격이 끊기는 것 완화. Lgt(0.06)보다 짧게 두어 첫 타격감 유지. 재생은 `SoldierHealth` `Montage_Play` — 에셋 블렌드 값 사용.

### 13.1 7차 녹화 `20260930_161008` — 슬롯 분리 후 + 재장전 직후 사격 [A]

- 슬롯 그룹 분리(사용자 적용) 후 **피격 → 사격 척추 튐 중앙 27.0 → 7.3°, 최대 163.8 → 24.6°** ✅.
- 사용자 관찰 "재장전 끝나자마자 쏘면 끊긴다" — 사격 시작 시점별 척추 튐: **재장전 종료 0.35초 이내 15건 중앙 56.9 / 최대 74.7°/프레임**(전부 종료 0.12초 뒤), 그 외 1.3°.
- **원인**: `BP_AR4Rifle` `StartReload`의 사격 금지 = Delay **0.6 + 0.7 + 0.7 = 2.0초**(재장전 소리 타이밍), 재장전 몽타주는 **2.2초**(1.9초부터 0.3초 블렌드 아웃으로 총을 올림) → 총을 올리는 도중에 사격이 풀리고, BlendIn 0인 사격 몽타주가 같은 그룹의 재장전 블렌드 아웃을 끊음.
- **수정(사용자 제안)**: 마지막 Delay **0.7 → 0.9초** — 사격 금지 2.2초 = 몽타주 길이. 소리 타이밍(0.6, 1.3초)은 그대로. `BP_AK47Rifle`은 상속. 컴파일·저장, P4 체크아웃. ⚠ 재장전 몽타주 길이를 바꾸면 이 Delay 합도 같이 맞춰야 한다.

### 13.2 8차 녹화 `20260930_161651` — 확인 [A]

- 재장전 종료 0.35초 이내 사격: 척추 **56.9/74.7 → 0.5/2.5°/프레임** ✅ — 첫 발이 몽타주 블렌드 아웃이 끝난 뒤에 나간다.
- ⚠ 측정 정정: 척추·머리 피치가 75~80°라 **오일러 차는 짐벌락으로 부풀려진다**. 이후 본 회전 변화는 **쿼터니언 각도 차**로 잰다(`tmp/quat.py` 방식).
- 쿼터니언 기준 몽타주 전환(머리/척추 °/프레임, 중앙/최대): 피격→사격 **27.0/47.9 · 18.7/31.0 → 15.8/22.5 · 7.5/11.1**, 사격→피격 13.3/33.6 · 8.1/18.4, 재장전→종료 1.5/3.1 · 2.6/4.2.
- **기준(몽타주 없이 피격 시작)**: 머리 13.0/20.8 · 척추 7.4/9.1 — 남은 값은 **피격 동작 자체의 충격 움직임**과 같은 수준. 전환 끊김은 해소.

남는 과제(측정 후 판단): 이동 중·린 상태의 총구 오차가 빠른 보정으로도 크게 남으면, 1프레임 늦은 피드백 대신 **애님 그래프 안에서 그 프레임 포즈로 총구를 조준에 맞추는 해석적 보정 노드**(척추 체인 회전)로 바꾸는 것이 다음 단계.

## 14. 마무리 (2026-09-30, 문서 정리) — 떨림 해결 판정 · 바뀐 것 · 남은 것

8차 녹화(13.2절) 뒤 사용자가 **AI 조준 떨림을 해결로 판정**했다. 이 문서의 상태를 완료로 바꾼다.

**최종 조준 체인** (위 절들의 결과만 모음 — 근거는 각 절):

| 층 | 지금 동작 | 절 |
|---|---|---|
| 컨트롤 회전 | `ASoldierAIController::UpdateControlRotation` = 교전 `GetAimRotation()`(피치 포함, roll 0). 초점 없음/교전 조준 전이면 엔진 원래 동작 | 2·3 |
| 선회 | `SlewAim` — 오차 ≤ 3° 40°/s · ≥ 30° 150°/s · 사이 선형(교전·수색 두 호출부) | 3 |
| 조준 보정 | 총이 조준에 있어야 할 때(AO 켜짐 ∧ 몽타주 없음 ∧ 피격 반응 없음 ∧ 총 내림 < 0.05)만 `1−exp(−10·dt)` 적분, 데드밴드 0.3°, 게이트 120°/s(넘으면 유지) · 그 밖엔 6/s 로 0 누설 · 축당 60°/s 상한 · ±25° 클램프 | 6.2·7·10 |
| 자세 | PoseSmoother `StepAxis` 한 프레임 도달 속도 상한 + 넘으면 착지 · 브리지 `UpdateStance` 슈미트 트리거 ±0.05 | 6.1·7 |
| AO 자산 | 서기↔앉기 AO 교체를 `USoldierAnimLibrary::UpdateStanceAimOffset`(관성 0.25 s)로 — ABP `BlendSpacePlayer_1` On Update | 9 |
| 조준 on/off | AI `WantsToAim` = 접촉 ∧ 달리지 않음(400 해제 / 340 재개) · 전환 최소 0.8 s · AI 몸통 회전 ≤ 360°/s | 10·11 |
| 몽타주 | 재장전 BlendIn 0.25 · 피격 Med/Hvy BlendIn 0.05 · 스켈레톤 슬롯 그룹 `HitReact` 분리 · `BP_AR4Rifle` 사격 금지 2.2 s = 재장전 몽타주 길이 | 8·13 |

**새 cvar** (전부 런타임): `SoldierLab.Aim.RealPitch` · `SoldierLab.Aim.CorrectionRate`/`.CorrectionLeakRate`/`.CorrectionDeadbandDeg`/`.CorrectionGateDegPerSec`/`.CorrectionMaxDegPerSec` · `SoldierLab.Aim.JogAimOffSpeed`/`.JogAimOnSpeed`/`.JogAimMinHoldSeconds`/`.AIMaxBodyYawRate` · `SoldierLab.Pose.CrouchHysteresis` · `SoldierLab.Pose.AimOffsetInertialSwitch` · `SoldierLab.Debug.AimTrace`/`.AimTrace.Filter`. (같은 날 별건 `SoldierLab.Engagement.BlindFire` — `animation/2026-09-30_diagonal_aim_stop_selection.md` 6절.)

**바뀐 것**: 코드 — 신규 `AI/SoldierAIController.{h,cpp}` · `Pose/SoldierAnimLibrary.{h,cpp}` · `Debug/SoldierAimTrace.{h,cpp}`, 수정 `AI/SoldierEngagement.{h,cpp}` · `Pose/SoldierAIBridgeComponent.{h,cpp}`(헤더: `LastAimToggleSeconds`) · `Pose/SoldierPoseSmootherComponent.cpp` · `SoldierLab.Build.cs`(`PoseSearch` · `BlendStack` · `AnimGraphRuntime`). 에셋 — `AIC_Soldier`(부모 `AIController` → `SoldierAIController`) · `SoldierCharacter_ABP`(함수 `OnUpdate_StanceAimOffset`, `Select_2 → BlendSpace` 연결 해제) · 재장전 몽타주 2 · 피격 몽타주 12 · 스켈레톤 `SK_UEFN_Mannequin` 슬롯 그룹(사용자) · `BP_AR4Rifle`.

**남은 것** → `OPEN_ITEMS.md`:
- **[C-176]** 앉아 걷기 조준 오차(중앙 4.5° / p90 23.9° / p99 50°, 12절) — 웅크려 걷기 클립 팔 자세 vs `AO_Rifle_Crouch`. 위 "남는 과제"(해석적 보정 노드)는 이것을 측정한 뒤 판단.
- **[W127]** 아군 재장전 1.6 초 손목(`ALLY_MM_Rifle_Reload_Additive1` 의 `hand_r` 키, 디자이너 애셋 — 사용자 처리 중, 8·10·12절).
- **[W128]** ABP 옛 Select 체인(`K2Node_Select_0/1/2` · `EnumEquality_0`) 잔해 정리(9절).
- **[C-178]** 새 값들(위 cvar · 선회 40/150 · 3°/30°)의 넓은 장면 검증 · 2-PC 클라 원격 피치(4절 ④).
- 대각선 조준 이동의 발 끌림은 별건 — `animation/2026-09-30_diagonal_aim_stop_selection.md`([W124]).
- **해결 처리**: [W11](AO 이진 Select — 9절 관성 전환) · [C-93](선회 상수 — 3절 값으로 사용자 판정 통과).

**원칙**(`CLAUDE.md` 5절): **P195**(연속 축 → 이산 상태는 히스테리시스, 6.1) · **P196**(보정 속도 ≠ 움직임 속도, 6.2) · **P197**(몽타주 슬롯 그룹 · 사격 금지 = 몽타주 길이, 13) · **P198**(AI 컨트롤 피치 0 — 엔진 규칙, 2) · **P199**(측정 — 프레임 녹화기로 발원지, 본 회전은 쿼터니언 각도, 5·13.2).
