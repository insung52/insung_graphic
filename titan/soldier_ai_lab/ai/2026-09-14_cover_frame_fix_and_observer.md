# 엄폐 기준면 버그 수정 · 관전 폰 재작성 · 1인칭 · 머리 조준 추종

2026-09-14 / 완료(관전·1인칭·머리 추종은 사용자 확인 "잘됨") · [C-95] 는 빌드됐으나 정착 미확인 / [C-95] 진단 결과 = 후보와 HERE 를 **다른 높이 기준**으로 재고 있었다(액터 원점 vs 발) → 발 기준 통일. 관전 폰을 빙의 대신 **AI 를 살려 둔 추적 카메라**로 C++ 재작성. 같은 날 오후~저녁에 **직접 조작 1인칭**(3c)과 **머리·목 조준 추종**(3d)까지 붙었다 — 후자의 최종 설계는 `animation/2026-09-14_sight_alignment_plan.md`.

`2026-09-14_exposure_ladder_and_corrections.md` 12절의 후속. 그 문서가 "세 번째 추측이 아니라 진단"을 요구했고, 이 문서는 그 진단과 결과다.

---

## 0'. 최종 상태 (2026-09-14 저녁) — 아래 시간순 절들과 어긋나면 이쪽이 맞다 [A]

이 문서는 한나절 동안 시간순으로 쌓였고, 3c·3d 절 안의 "1차/2차/3차" 정정들이 서로를 뒤집는다. 최종 상태만 추리면:

| 항목 | 최종 |
|---|---|
| [C-95] 기준면 수정 | **빌드됨**(14:36, 사용자 CL 469 에 포함). 수비수 정착 여부는 **아무도 보지 않았다** — 1.6절 기준으로 확인할 것 |
| 이관 누락 DDCvar | titan `DefaultEngine.ini` 에 추가, 에디터 재시작 후 유효 ([W40] 해결). 원본 SoldierLab 프로젝트는 이 PC `C:\working\works\kadex\anim_test\SoldierLab` 에 있다 |
| 관전 폰 | `Observer/SoldierObserverPawn`. **F** 추적/해제(AI 계속) · **T** 1/3인칭(V 에서 통일) · **Tab** · 휠. `PickConeDegrees` **15**. `BP_ObserverPawn` 부모 교체 완료. ✅ **2026-09-15 확인**: 1인칭이면 그 병사의 `USoldierFirstPersonComponent` 에 뷰 위임(`BeginExternalView/EndExternalView`, 빙의 없음, `bUseSoldierFirstPersonComponent`) · **H** = 그 병사의 머리 추종 토글(AI 는 `bApplyToAI`) · HUD `headaim:ON/off · / soldier eyes` |
| 1인칭 | `Camera/SoldierFirstPersonComponent`. **뷰타겟 교환**(GASP 카메라 Deactivate 는 폐기 — P107). `Anchor` Body/Weapon · `CameraSocket=eyes`(사용자 소켓) · `LocationOffset` 은 **0** 이어야 눈 소켓에 정확히 · `RotationMode=FollowSocket` + `bAlignToAimOnEnter`(머리 실제 시선 기준) · `bFollowSocketOnlyWhileHeadAims` · `bHideBodyFromOwner`=`PC->HiddenPrimitiveComponents`. 키 **T** |
| 머리 추종 | `Pose/SoldierHeadAimComponent`, **기본 OFF**, **H**. 닫힌 루프·월드 Additive(몸 프레임 = spine_03 에 저장), 2단(둘러보기 / weld 가중치+래치), 조준선은 정지 게이트에서 학습·작은 변화는 추적, 눈은 **목 굽힘 60° + 스트레치 5 cm** 로 선 위에, 맹목사격 시 전체 off. **10차까지 사용자 확인 "완벽"(21:30)** — 이 문서 3d 절은 초기 판이고 **최종 설계는 `animation/2026-09-14_sight_alignment_plan.md` 0' 절** |
| ABP | 변수 `HeadAimRotation`·`NeckAimRotation`·`Neck2AimRotation`·`HeadAimAlpha`·`HeadAimLocation`(항상 0). 체인 `TwoBoneIK_0 → ModifyBone_6(neck_01) → ModifyBone_9(neck_02) → ModifyBone_7(head) → ModifyBone_8(head, Ignore·사용자 잔여) → ComponentToLocalSpace_2`. 회전 **Add to Existing · World Space**(사용자 수동, P108) |
| 검증 대기 | ~~[W47] · [W46]~~ → 21:30 사용자 확인으로 해결. ~~[W50]~~ → 2026-09-15 확인으로 해결(몸 회전 P119 도 같이). 남은 것: **[C-95]** 수비수 정착 실측(AI 세션이 진행 중) · [W54] ABP 잔여 노드 삭제 |
| Perforce | CL 469 제출 후 변경분(Observer.h 픽 각도 · 1인칭 컴포넌트 · Pose 재작업) **미제출** |

---

## 0. 한 장 요약

| | 전 | 후 |
|---|---|---|
| [C-95] 수비수 재배치 반복 | `HERE` 가 거의 항상 **1.0**, 가끔 0.5 | **원인 확정** — 후보는 발(네비메시) 기준, HERE 는 **캡슐 중심(+90 cm)** 기준. 발로 통일 |
| 위협의 눈 높이 | 기록 위치(**이미 가슴**) + 135 = **2.6 m** | 기록 + `ThreatEyeAboveContactCm` 20 |
| 엄폐 판정에서 아군 몸 | 시야 채널을 막으므로 **벽으로 셌다** | 등록부 전원 무시 |
| 웅크린 총구 높이 | 원점 − 90 고정 → 웅크리면 **45 cm** | 발 + 95 |
| 관전 폰 | Visibility 트레이스 → **병사를 못 맞힘**, F = **Possess**(AI 정지) | 등록부+시야각 픽, F = **추적**(AI 그대로), V 1/3인칭, Tab 다음, 휠 거리 |

**하나도 플레이에서 재지 않았다** — 사용자가 빌드한 뒤 1절의 판정 기준으로 본다.

---

## 1. [C-95] 진단 — HERE 가 1.0 인 이유 [A · 코드 실측]

사용자 관측: `SoldierLab.Debug.Cover 1` 오버레이에서 **`HERE` 가 거의 언제나 1.0, 가끔 0.5.** 12절의 세 갈래(ⓐ 이동 발행 ⓑ 위협 진동 ⓒ 가중치) **어느 것도 아니었다.** 네 번째 경우다: **HERE 와 후보가 같은 자로 재어지지 않았다.**

### 1.1 기준면 불일치

```
후보  GetCandidate()  →  ProjectPointToNavigation  →  Foot = 지면          샘플 높이 135 / 107 / 80
HERE  OwnerPawn->GetActorLocation()                →  캡슐 중심 = 지면+90   샘플 높이 225 / 197 / 170
```

`EvaluatePosition(Foot, …)` 은 `Foot + StandChestHeightCm…CrouchChestHeightCm` 에 트레이스한다. 후보는 발이고 HERE 는 캡슐 중심이니 **같은 자리가 90 cm 차이로 두 번 평가된다.** 1.5 m 벽 뒤:

- 후보로서: 135 막힘 → `FirstHidden 0` → hide, `bCanFight false` → 비용 0.5 (또는 107 부터 막히면 0)
- 도착해서 HERE 로서: 170 부터 시작 → 아무것도 안 막힘 → `exposure 1.0`, `bCanHide false` → **`NoCoverCost 1.0`**

그래서 어디에 서든 HERE = 1.0, 다음 후보가 항상 `1.0 − 0.3` 보다 싸고, **도착 즉시 또 떠난다.** 가끔 0.5 는 4~5 m 건물 모서리 — 225 cm 도 막히는 곳뿐이다. 사용자 관측과 정확히 맞는다.

같은 이유로 **`RequiredStance` 가 항상 0** 이었다("아무것도 못 가려 주면 자세도 안 요구" 분기). `USoldierEngagementComponent::DesiredStance = max(제압, RequiredStance)` 이므로 **제압 없이는 절대 웅크리지 않았다** — 사용자가 본 "압박 높을 때만 숙인다"가 이것이다. 엄폐 로직이 없는 게 아니라 **입력이 항상 0 이었다.**

### 1.2 위협의 눈이 2.6 m

`ThreatEye = ThreatLocation + StandChestHeightCm(135)`. 그런데 `ThreatLocation` 은 발이 아니다 — 시야 기록은 `GetTargetLocation()` = **`spine_03`**(가슴, ~125 cm), 총성 기록은 **총구**(~140 cm). 135 를 더하면 **지면 2.6 m** 에서 내려다본다. 1 m 담은 바로 뒤에 붙어야 겨우 가리고 3 m 만 떨어져도 소용없다. 후보 쪽 판정까지 같이 후해졌다.

→ `ThreatEyeAboveContactCm = 20`(신설, [C]). 기록 위치가 이미 가슴이라는 사실을 값 이름에 박아 둔다.

### 1.3 아군 몸이 벽으로 셌다

`Sight` 채널은 `DefaultResponse=Block` 이고 캡슐/메시 모두 막는다(시야 자체가 그래야 한다). 엄폐 트레이스는 소유자만 무시했으므로 **위협과 후보 사이에 아군이 서 있으면 그 후보는 엄폐**였고, 둘 중 하나가 움직이면 사라졌다. `IgnoreBodies()` 로 등록부 전원 무시. 엄폐는 지형이다.

### 1.4 총구도 같은 버그

`SoldierEngagement::MuzzleAtStance`: `원점 + (높이 − 90)`. `SetCrouchMaintainsBaseLocation(true)` 라 웅크리면 원점이 ~50 cm 내려가므로 **웅크린 총구 = 발 + 45 cm** — 담에 파묻혀 조리개 탐색이 매번 `Over` 로 갔다. 발 + 높이로 고침.

### 1.5 한 곳에 모음 — `USoldierIdentityComponent::GetFeetLocation()` (P103 — 처음 P100 으로 적었다가 번호 충돌로 개명)

```cpp
static FVector GetFeetLocation(const AActor*);   // 원점 − 현재 캡슐 반높이
static float   GetCapsuleHalfHeight(const AActor*);
```

AI 층에서 높이를 더하는 자리는 **전부 이것 위에** 더한다. 엄폐 HERE · 경로 시작점 · 총구 · 관전 1인칭 폴백 — 넷이 이제 같은 자를 쓴다.

### 1.6 판정 기준 (빌드 후 볼 것)

`SoldierLab.Debug.Cover 1` + `SoldierLab.Debug.AI.Filter Friendly`:

- 오버레이에 **`st`**(RequiredStance)가 추가됐다: `exp 0.50 st 0.50 hide+fight obj 0.00 | HERE 0.00 best …`
- 낮은 담 뒤 수비수: **`HERE 0.00`**, `hide+fight`, `st 0.5~1.0` → **stay** 가 유지되는가
- 그 상태에서 사격할 때만 일어서고(`Over/open`), 재장전(`RELOADING`)은 웅크린 채인가
- 여전히 옮긴다면 12절의 ⓐ/ⓑ/ⓒ 로 돌아간다 — **이번엔 그 셋이 유효한 갈래다**

---

## 2. 관전 폰 — `ASoldierObserverPawn` (C++) [A]

### 2.1 왜 BP 가 안 됐나

| 증상 | 원인 |
|---|---|
| `[F] 빙의` 가 안 뜬다 | Tick 의 `LineTraceByChannel` 이 **Visibility** 채널. 병사 캡슐(`Pawn` 프로필)도 메시(`Custom`: Visibility=Ignore)도 **Visibility 를 무시**한다. 아무것도 안 맞는다 |
| F 가 안 된다 | 위와 같음 — `HoverSoldier` 가 영원히 null |
| 됐더라도 원하는 게 아니다 | F 가 **`Possess`** 였다. AI 컨트롤러가 떨어져 나가 **병사가 멈춘다.** 사용자 요구는 "AI 는 계속 돌고 나는 보기만" |
| 플라이캠이 움직인 이유 | `FlyCam_*` 축 이벤트는 titan 에 매핑이 없어 죽어 있었고, **`DefaultPawn` 의 엔진 정의 바인딩**(WASD/QE/마우스)이 대신 돌고 있었다 |

### 2.2 설계

`Source/SoldierLab/Observer/SoldierObserverPawn.{h,cpp}` — `ADefaultPawn` 자식.

- **픽**: 트레이스가 아니라 **등록부 + 시야각.** 시선과 병사 가슴 사이 각이 `PickConeDegrees 6°` 안이고 가장 가까운 것. `bPickNeedsLineOfSight` 로 벽 뒤 픽만 막는다(Visibility, 몸은 무시). Tick 에서 한 번 판정해 `Hovered` 에 저장, 라벨 `[F] follow <이름>` 과 F 키가 그 값을 읽는다(P94 유지)
- **추적**: 병사를 건드리지 않는다 — 뷰타겟 교체도 컨트롤러 교체도 없다. **관전 폰 자신을 매 프레임 카메라 자리로 옮긴다**(`TG_PostPhysics`, 병사가 움직인 뒤). 풀면 그 자리에 남는다
- **3인칭**: 발 + `PivotHeightCm 140` 을 축으로 컨트롤 회전(마우스)으로 궤도, `ShoulderOffsetCm 60`, 거리 `ThirdPersonDistanceCm 350`(휠 ±50, 100~1500). 벽은 `Camera` 채널 스피어 스윕으로 당긴다
- **1인칭**: `head` 소켓 + 조준 방향 18 cm 앞. `bFirstPersonFollowsAim`(기본 true) 이면 컨트롤 회전을 병사 `GetBaseAimRotation()` 으로 덮어 **병사가 보는 곳을 본다.** 메시는 안 숨긴다 — 총이 메시에 달려 있고, 총구가 담을 넘는지가 관전의 요점이다
- **충돌 없음**: 기본 구체가 `Pawn` 프로필이라 병사를 물리적으로 막는다. 유령으로 둔다
- **키**: 레거시 `BindKey` — IA 에셋도 ini 도 필요 없다([W31] 의 제약을 정면으로 받아들인 것). `FKey` UPROPERTY 라 BP 기본값에서 바꿀 수 있다

| 키 | 동작 |
|---|---|
| **F** | 조준선 아래 병사 추적 / 해제 |
| **T** | 1인칭 ↔ 3인칭 (추적 중) — ~~V~~ → 17:10 병사 조작(T)과 통일 |
| **Tab** | 등록부 다음 병사 |
| **휠** | 3인칭 거리 |
| WASD / Q E / 마우스 | DefaultPawn 그대로 (추적 중엔 무시됨) |

⚠ `C` 는 쓰지 않는다 — `DefaultPawn` 이 `MoveUp −1` 로 잡고 있다.

### 2.3 BP_ObserverPawn — 그래프를 비웠다

옛 Tick/F/FlyCam 노드 38개를 전부 삭제·컴파일·저장했다(151 KB → 26 KB, 디스크 확인). **아직 부모는 `DefaultPawn` 이다** — C++ 클래스가 빌드되기 전엔 `set_parent` 를 걸 수 없다. `GM_SoldierObserver` 는 `GetDefaultPawnClassForController` 오버라이드가 `BP_ObserverPawn` 을 돌려주므로(P53 우회) **BP 의 부모를 바꾸는 것**이 맞는 길이다:

```
빌드  →  BP_ObserverPawn 열기  →  Class Settings  →  Parent Class = SoldierObserverPawn  →  컴파일·저장
```

### 2.4 1인칭/3인칭 — 두 모드의 현황

| 모드 | 게임모드 | 카메라 | 1/3인칭 |
|---|---|---|---|
| 관전 | `GM_SoldierObserver` → `BP_ObserverPawn` | 이 문서 | **V 로 전환 (신규)** |
| 직접 조작 | `GM_SoldierLab` → `BP_Soldier_Friendly` | GASP **Gameplay Camera**(`CameraAsset_SandboxCharacter`, 리그 `Close/Medium/Far × Aim/Freecam/Strafe`) | **1인칭 없음.** 마우스 휠 = `CameraStyle` Close/Medium/Far 순환뿐 → **[W30] 그대로 열려 있다** |

직접 조작의 1인칭은 카메라 리그(`CameraRig_*`)를 하나 더 만들고 디렉터에 분기를 넣는 일이라 이번 범위 밖. 필요하면 별건.

---

## 3. 변경 파일

```
Source/SoldierLab/AI/SoldierIdentity.{h,cpp}     GetFeetLocation / GetCapsuleHalfHeight
Source/SoldierLab/AI/SoldierCover.{h,cpp}        발 기준 HERE · ThreatEyeAboveContactCm · IgnoreBodies · 오버레이 st
Source/SoldierLab/AI/SoldierEngagement.cpp       MuzzleAtStance 발 기준 (상수 90 삭제)
Source/SoldierLab/Observer/SoldierObserverPawn.{h,cpp}   신규
Content/SoldierLab/Blueprints/BP_ObserverPawn    그래프 전부 삭제 (부모 교체는 빌드 후)
```

전부 Perforce 체크아웃 상태(`user4_DESKTOP-81S78B2_4340`), 미제출.

---

## 3b. 빌드 후 (2026-09-14 15:00) — 재부모 완료 · 이관 누락 1건 발견 [A]

- `BP_ObserverPawn` 부모를 `SoldierObserverPawn` 으로 교체·컴파일·저장(디스크 확인, 26 KB). PIE 기동 시 에러 0건.
- ⚠ **PIE 로그가 매 틱 `Failed to find console variable 'DDCvar.*'` 로 도배됐다.** GASP 의 데이터 구동 콘솔 변수
  (`[/Script/Engine.DataDrivenConsoleVariableSettings]` 27줄)가 **이관에서 빠져 있었다** — 이관 문서 6.1절이
  콜리전 채널만 옮겼다. 없으면 값이 전부 0 으로 읽힌다: `ThreadSafeAnimationUpdate.Enable`(기본 True) ·
  `FootPlacementMode`(기본 1) · `NewGameplayCameraSystem.Enable`(기본 True) 이 **꺼진 채로 돌고 있었다.**
  SoldierLab 원본(`anim_test/SoldierLab/Config/DefaultEngine.ini`, 이 PC 에 있다 — `works\` 아래)에서 그대로 복사해
  titan `DefaultEngine.ini` 끝에 붙였고 `PoseSearchSettings.AvailabilitiesBufferSize=230` 도 같이. **에디터 재시작이 필요하다**
  (DDCvar 는 기동 시 등록). 플러그인 추가는 불필요(클래스가 Engine 모듈).
- `LogAnimation: Warning: weapon_r: socket doesn't exist` ×3 / PIE — 병사 3명의 메시에 무기 소켓이 없다. 별건 → [W45](처음 [W39] 로 등록했다가 번호 충돌로 개명)

## 3c. 직접 조작 모드 1인칭 — `USoldierFirstPersonComponent` (2026-09-14 15:30, 빌드 대기) [A]

[W30] 착수. `Source/SoldierLab/Camera/SoldierFirstPersonComponent.{h,cpp}` — `BP_SoldierCharacter` 에 붙이는 액터 컴포넌트.

| 요소 | 방식 |
|---|---|
| 카메라 | 런타임 생성 `UCameraComponent`, 메시의 **`CameraSocket`(기본 `head`, 에디터에서 변경 가능)** 에 부착. `bUsePawnControlRotation` — 마우스가 돌리고 본은 위치만 준다 |
| 오프셋 | `LocationOffset (12, 0, 10)` — **본 공간이 아니라 폰 공간**(X 시선 앞·Y 오른쪽·Z 위). 실측: **`head` 본은 이 스켈레톤에서 입 높이**다. Z 로 올리거나, 정확히 하려면 스켈레톤에 눈 소켓(예 `eyes`)을 저작하고 `CameraSocket` 에 넣는다 — 소켓도 본과 똑같이 받는다 |
| 소켓 없음 | 경고 로그 + 루트에 `FallbackEyeHeightCm 165` 고정 높이 |
| 3인칭 ↔ | ~~GASP `GameplayCamera` 를 `Deactivate()` / `ActivateCameraForPlayerController()`~~ ⚠ **실측 실패(15:40)** — 재활성화하면 카메라 시스템(평가 컨텍스트·호스트)이 새로 만들어지며 **머리 위에 떠 있는 채로** 돌아왔다. → **2차 설계: GASP 카메라는 아예 안 건드린다.** 1인칭 = 별도 뷰타겟 액터 `ASoldierFirstPersonViewTarget`(`CalcCamera` 오버라이드가 컴포넌트의 `ComputeView()` 를 호출), `PC->SetViewTarget(ViewTarget)`. 3인칭 = `SetViewTarget(캐릭터)` — GASP 출력 카메라는 그동안 계속 평가되고 있었으므로 **떠난 적 없는 상태로** 돌아온다. `CalcCamera` 는 카메라 매니저가 답을 원할 때(그 프레임 애니메이션 확정 후) 불리므로 본↔눈 틱 순서 지연도 없다. `GameplayCameras` 링크 불필요 |
| 회전 모드 | `RotationMode` — **`HeadBone`(기본)**: 카메라 = 머리 본 회전. 진입 순간 `BoneToView = 본회전⁻¹ × 조준회전` 을 잡아 두므로 스켈레톤 본 축을 몰라도 되고, 그 뒤로는 **머리가 움직인 만큼만** 시선이 따라간다(AO 지연·반동·걸음 흔들림 포함). `SocketRotationOffset` 은 그 위의 트림. **`ControlRotation`**: 마우스 그대로, 본은 위치만 — 흔들림 없음 |
| 재빙의 | GASP `Possessed → SetupCamera` 가 3인칭을 다시 켜므로 `ReceiveRestartedDelegate` 에서 1인칭이면 다시 가져온다 |
| 입력 | **런타임 생성 `UInputAction` + `UInputMappingContext`** (`MapKey(ToggleKey)`) → Enhanced Input 경로 그대로. 에셋도 ini 도 없음. 기본 **T**, `FKey` 프로퍼티라 BP 기본값에서 변경 |
| 옵션 | `bHideBodyFromOwner`(기본 off — 총이 메시에 달려 있어 팔까지 사라짐) · `FieldOfView 90` · `bStartInFirstPerson` |

Build.cs 에 `EnhancedInput` · `GameplayCameras`(엔진 플러그인, 기본 활성) 추가.

**빌드 후 할 일**: `BP_SoldierCharacter` 에 컴포넌트 추가(`add_component` — `.uasset` 체크아웃해 둠) → `GM_SoldierLab` 로 PIE → **T**.
관전 폰의 1인칭(V)과는 별개 구현이다 — 관전은 병사를 안 건드리는 게 요점이라 카메라를 병사에 붙이지 않는다.

## 3d. 머리·목 조준 추종 — `USoldierHeadAimComponent` (2026-09-14 16:00, 빌드·수동 마무리 대기) [A]

**왜**: 총은 AO + 총구 되먹임(2.5c)으로 조준을 닫힌 루프로 따르는데 **머리는 어디에도 정렬 목표가 없다.** 1인칭 `FollowSocket` 카메라가
크로스헤어를 안 보는 이유이고, 관전에서 병사가 쏘는 곳을 고개로 안 보는 이유다. 사용자 요구: **켜고 끌 수 있게**(왼손 IK처럼).

**방식** — `LookAt` 노드가 아니라 **`Modify Bone`(회전 · World Space · Replace) 2개 + C++ 계산**:
- `LookAt` 은 "머리 본의 어느 축이 앞인가"를 스켈레톤마다 알아야 한다. 대신 **레퍼런스 포즈에서 본의 "facing 대비 상대 회전"을 한 번 재고**
  (`FAnimationRuntime::GetComponentSpaceTransformRefPose`, facing = 메시 상대회전의 역), 원하는 머리 = **조준 회전 × 그 상대 회전**. 메시가 바뀌어도 그대로.
- 조준은 `RInterpTo(AimInterpSpeed 10)` 로 지연 → 급선회 시 **총이 먼저, 머리가 한 박자 뒤** (사용자가 원한 과도). 몸 기준 yaw ±75 / pitch ±55 클램프.
- 목은 `NeckShare 0.35` 만큼 Slerp, 머리는 전부. 알파는 `AlphaRampPerSecond 4` 로 램프 → 토글이 안 튄다. `Strength` 로 부분 적용 가능.
- AI: `GetBaseAimRotation` 은 AI 컨트롤러에서 pitch 0 이라, 접촉 중이면 `Engagement->GetAimPoint()` 방향을 쓴다. `bApplyToAI` 로 끌 수 있다.
- ABP 변수 3개(`HeadAimRotation`·`NeckAimRotation` Rotator, `HeadAimAlpha` Float)에 **이름으로 리플렉션 쓰기** — BP float 은 double 이므로 둘 다 처리.
  메시 틱 선행 조건으로 등록(P39 계열: 순서 안 잡으면 한 프레임 늦은 머리).
- 키 **H** (런타임 IA/IMC, 1인칭 T 와 같은 방식).

**ABP 배선 (MCP 로 완료·저장, 16:05)**: `TwoBoneIK_0 → ModifyBone_6(neck) → ModifyBone_7(head) → ComponentToLocalSpace_2`.
Rotation 핀 ← 변수 게터, Alpha 핀 ← `HeadAimAlpha` 게터(노드별 하나씩).
⚠ **`set_properties` 가 애님 노드 프로퍼티를 못 쓴다(P53 재확인 — 점 경로·중첩·PascalCase 전부 false)**. 따라서 **노드 2개의 세 필드는 에디터에서 수동**:
`Bone to Modify` = neck_01 / head · `Rotation Mode` = **Replace Existing** · `Rotation Space` = **World Space** (Translation/Scale 은 Ignore 그대로). 위치 (1650,300)·(1900,300).

⚠ **2차 (16:40) — Replace 는 틀렸다.** 실측: 머리 추종 on 이면 **QE 린 롤·블라인드파이어·걸음 흔들림이 전부 사라졌다**(Replace 가 애니메이션 머리 회전을 통째로 덮음). 그리고 조준을 `GetBaseAimRotation()` 으로 읽은 것이 **1인칭에서 되먹임 루프**였다 — 그 함수는 컨트롤 회전이 아니라 **카메라 매니저의 뷰 회전**을 주고, 1인칭 뷰 = 머리 소켓이라 머리가 자기 자신을 목표로 삼아 마우스와 끊겼다(→ `Controller->GetControlRotation()` 으로).
**해법 = 총구 보정(2.5c)과 같은 닫힌 루프**: 지난 프레임 최종 머리의 실제 시선(`GetSocketQuaternion(head) × HeadRel⁻¹`)과 조준의 오차(롤 제외)를 재서 `CorrectionGain 8`/s 만큼 **월드 공간 Additive** 회전에 누적. 느린 오차(AO 잔여 편차·조준)는 잡히고 빠른 흔들림은 통과, 롤은 손대지 않는다. 안티 와인드업(off 면 풀어 둠, 총량 상한) 포함. **ABP 노드 두 개는 `Rotation Mode = Add to Existing` 으로 바꿔야 한다**(World Space 유지). 1인칭 `bHideBodyFromOwner` 는 `SetOwnerNoSee`(뷰 액터=소유자 조건이라 별도 뷰타겟에선 불성립) → `PC->HiddenPrimitiveComponents` 로.

⚠ **3차 (17:00) — 롤 누적·게이트.** 실측 "좌우로 돌리면 머리가 기울고 원복 안 됨": 서로 다른 축의 swing 을 쿼터니언으로 누적하면 시선축 twist(롤)가 부산물로 생기고 롤은 측정에서 빼놓아 영영 남았다. → 오차는 `FindBetweenNormals(현재 시선, 목표)` 의 **순수 swing**, 누적 보정은 매 프레임 `ToSwingTwist(애니메이션 시선축)` 로 **twist 제거**. 사용자 확인: 성공.
**게이트 추가**: `bOnlyWhileAiming`(기본 on) — ABP `AOActive`(bool) 을 리플렉션으로 읽어 견착 중에만. 총내림·달리기에서는 알파가 내려간다. 1인칭 `FollowSocket` 은 그때 카메라가 몸에 잠기지 않도록 **머리 추종 알파만큼만 소켓을 따르고 나머지는 컨트롤 회전**(`bFollowSocketOnlyWhileHeadAims`).
**다음**: ~~눈–조준선 정렬(총을 눈으로 IK)~~ → 방향이 거꾸로였다(머리가 총에 간다). 같은 날 밤 10차까지 가서 완성 → `animation/2026-09-14_sight_alignment_plan.md` 0' 절.

**1인칭 컴포넌트 추가분(같은 시각)**: `Anchor` = BodyMesh / **WeaponMesh**(`WeaponMeshComponentName`, 부착 액터까지 탐색) ·
`bLocationOffsetInSocketSpace`(조준경 소켓 뒤로 고정 거리) · `RotationOffset` 양 모드 공통 · `bAlignToAimOnEnter`(끄면 소켓 축 그대로 = 조준경용).
`HeadBone` 모드는 **`FollowSocket`** 으로 개명. 총 조준경 소켓에 카메라를 얹는 조합: `Anchor=WeaponMesh · CameraSocket=<scope> · FollowSocket · bAlignToAimOnEnter=false · 오프셋은 소켓 공간`.

## 4. 이 라운드가 하지 않은 것

- **위협이 없을 때의 엄폐** — 기록이 하나도 없으면 `EvaluatePosition` 이 `bCanHide=true, stance 0` 으로 바로 돌아온다. 접촉 없는 수비수는 서 있는다. "적이 멀어도 엄폐는 필수"는 **보이는 적**에 대해선 이제 성립하고, **한 번도 못 본** 적에 대해선 위협 방향(목표의 접근축 등)이 필요하다 → 별건
- 관전 1인칭에서 얼굴/헬멧 클리핑 — `FirstPersonForwardCm` 로 다이얼
- [W32] 조리개 트레이스 예산 · [C-83] 성능 — 그대로
