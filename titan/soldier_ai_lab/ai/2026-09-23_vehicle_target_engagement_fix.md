# 차량 표적 교전 — 표적 자신이 벽으로 읽히던 것 · 차량 위협의 눈높이

2026-09-23 / 완료 (빌드 · `L_SoldierTest` + New_kadex_0811 PIE 확인, 사용자 "이제 트럭도 잘 쏜다") / New_kadex_0811 3차 전투지에서 적 3분대가 이동형지휘소 트럭(`BP_TitanTruck`)을 거의 안 쏘던 원인 둘과 수정 둘. ① **사격 레인 트레이스가 표적 자신(차량)을 벽으로 읽어** 모든 사격 자세가 탈락(`aperture 0`) → 표적 액터를 레인 판정까지 넘겨 **"표적을 맞혔으면 도달"** ② 엄폐 층이 차량 위협의 눈을 **차체 한가운데**(≈2 m)에 두고 계산 → 소켓 없는 표적에 한해 **눈높이 단차를 그 대상의 형상에서** 계산.

전편(같은 축): `ai/2026-09-17_threat_bonus_lane_denied_vehicle_heights.md` 4절(차량 표적점 = 바운즈 비율 — **이 문서가 그 결정의 뒷면을 고친다**) · `ai/2026-09-21_game_thread_structural_pool_rays_bridge.md` B절(`SoldierQuery::BodiesAreNotWalls()` · 레인 캐시).
원칙: **P189**(표적 자신은 벽이 아니다) · P130(세계가 대신 알려 주지 않는다 — 2절의 "형상은 위치가 아니다") · P10(계측 먼저) · P36(패치 두 개째가 신호).
값: **[C-166]~[C-168]**.

---

## 0. 증상과 확정

**증상** [A · 사용자 관측]: New_kadex_0811 3차 전투지에서 적 3분대가 트럭을 상대로 **거의 사격하지 않는다.** 멀뚱히 서 있다가 자리만 옮기기를 반복하고, 아주 가끔 한 발. **대인 전투는 정상**이었다 — 즉 인지·분대·ROE·무기 어느 것도 통째로 죽어 있지 않았다.

**확정 방법** [A · 로그]: 콘솔

```
SoldierLab.Debug.Engagement.Log 1
```

→ `LogSoldierAI` 의 `[Engage]` 전이 줄. 실측:

```
[Engage] Enemy_A2 t=11.71 hold -> blocked roe=hold/0 tgt=BP_TitanTruck dist=8942 know=43
         spread=409 err=411 c=1.00 age=0.0 | believed 1 worth 1 aperture 0(none/open) ...
```

읽는 법 — 사격 결정 게이트(`squad/2026-09-18_squad_layer_fixes_quota_engage_range.md` 8절)를 앞에서부터 하나씩 통과시켜 보면:

| 게이트 | 값 | 판정 |
|---|---|---|
| 표적을 잡았는가 | `tgt=BP_TitanTruck` | ✅ 트럭을 표적으로 들고 있다 |
| 믿는가 | `believed 1` · `c=1.00` | ✅ 확신 최대 |
| 쏠 가치가 있는가 | `worth 1` | ✅ |
| **총구 자세가 있는가** | **`aperture 0(none/open)`** | ❌ **여기서 떨어진다** |
| 결과 | `hold -> blocked` | `FireIntent = Blocked` |

**표적 인식도 확신도 가치도 전부 통과하는데 사격 자세를 하나도 못 찾는다.** 대인에서는 같은 줄이 `aperture 1(direct/open)` 로 나온다. 그러면 질문은 하나로 좁아진다 — **`PlanAperture` 가 왜 전부 떨어뜨리는가.**

> P10 대로 로그가 먼저 이름을 댔다. `aperture 0` 이 아니었다면 ROE·분대 배정·진영 판정을 먼저 뒤졌을 것이고(실제로 그쪽부터 의심했다), 그 셋은 **전부 정상**이었다(4절).

---

## 1. 원인 1 (주원인) — 사격 레인 트레이스가 표적 자신을 벽으로 읽는다 [A · 코드 · 실측]

### 1.1 세 조각이 맞물린다

**(가) 차량의 조준점은 바운드박스 비율이다** [A · `AI/SoldierIdentity.cpp:20-58`]

차량에는 `head`/`spine_03` 소켓이 없다. `USoldierIdentityComponent::GetSocketOrFallback` 은 소켓이 없고 소유자가 `ACharacter` 가 아니면 **액터 바운드박스의 비율**을 돌려준다:

```
GetTargetLocation() = 바운즈 하단 + 2 × ExtentZ × 0.5   ← 차체 한가운데
GetEyeLocation()    = 바운즈 하단 + 2 × ExtentZ × 0.8   ← 포탑/캡 높이쯤
```

이건 09-17 오전에 의도적으로 넣은 것이다(`ai/2026-09-17_threat_bonus_lane_denied_vehicle_heights.md` 4절 — 병사가 UGV의 **바퀴/땅**을 겨누던 것을 고친 수정). 조준점으로서는 맞다. **문제는 그 점이 차체 표면이 아니라 차체 속 깊은 곳에 있다는 것**이고, 그 사실이 레인 판정에서 뒤통수를 쳤다.

**(나) 레인 판정은 "조준점 근처에 떨어졌는가"로 막힘을 정한다** [A · `AI/SoldierEngagement.cpp:370-440`]

`USoldierEngagementComponent::IsShotBlockedByWorld` 는 총구→조준점 라인 트레이스를 쏘고:

```cpp
bBlocked = FVector::Dist(Hit.ImpactPoint, Target) > LaneToleranceCm;   // 기본 200 cm
```

**"조준점에서 2 m 보다 멀리서 뭔가에 맞았으면 벽에 던진 것"** 이라는 규칙이다. 원래 의도는 "적의 엄폐물 **먼쪽 입술**에 맞은 탄은 레인을 다 지난 것이니 막힘이 아니다" — 사람 크기(반지름 ≈ 45 cm)를 상정한 여유값이다.

**(다) 차량은 그 트레이스가 무시하지 않는다** [A · `AI/SoldierQuery.h` · `Config/DefaultEngine.ini`]

09-21 부터 병사 무시는 액터 목록이 아니라 **Pawn 오브젝트 타입의 채널 응답**이다(`SoldierQuery::BodiesAreNotWalls()`, `ai/2026-09-21_game_thread_structural_pool_rays_bridge.md` B절). 무시되는 건 자기 부착물(총 액터)과 **Pawn** 뿐인데 — **차량은 Vehicle/WorldDynamic 이고 Sight 채널(`ECC_GameTraceChannel5`)을 Block 한다**(`DefaultEngine.ini` 기본 응답 Block).

### 1.2 합치면

총구에서 트럭 중심을 향해 쏜 트레이스는 **트럭 껍데기에 맞는다.** 그 충돌점에서 조준점(차체 중심)까지의 거리가 200 cm 를 넘으면 → `bBlocked = true`. **표적 자신이 벽으로 읽힌다.**

**실측(트럭 본체 메시 `Titan_Truck` 로컬 바운드)** [A · 에셋 실측]:

| 축 | 범위 | 중심에서 표면까지 |
|---|---|---|
| X | ±133 cm | **133 cm** |
| Y | −340 … +310 cm (전장 6.5 m) | **310~340 cm** |
| Z | 0 … 328 cm | — |

따라서:

| 사격 방향 | 충돌점 → 조준점 | vs `LaneToleranceCm 200` | 결과 |
|---|---|---|---|
| **측면** | ≈ 133 cm | < 200 | ✅ 통과 |
| **정면 / 후면** | 310~340 cm | **> 200** | ❌ 막힘 |

각도로 옮기면 — 트럭 **측면 축 기준 ±48° 안에서만** 사격이 허용되고, **정면·후면 각 ±42° 부채꼴이 사각**이다. "아주 가끔 쏜다"는 관측이 정확히 이 그림이다: 병사가 자리를 옮겨 우연히 측면 부채꼴에 들어간 순간에만 한 발이 나간다.

### 1.3 왜 모든 자세가 한꺼번에 떨어지는가

`PlanAperture`(`SoldierEngagement.cpp:456-540`)는 후보 자세 — Direct · 좌/우 린 · 기립 · 블라인드 3종 — 를 순회하면서 **전부 같은 레인 테스트 하나**(`:526` `IsShotBlockedByWorld(Option.OptionMuzzle, Target, ...)`)로 검사한다. 총구 위치만 수십 cm 씩 다를 뿐 표적점은 같으니, **트럭 껍데기가 벽이면 일곱 개가 동시에 탈락한다.**

그 뒤의 연쇄:

```
aperture 0  →  FireIntent = Blocked
            →  (2 s 지속) bLaneDenied 래치  (ai/2026-09-17_… 2절)
            →  엄폐 층이 HERE 에 LaneDeniedCost 1.0
            →  "여기선 못 쏨" → 자리를 옮긴다 → 새 자리도 같은 사각 → 반복
```

**관측된 "서 있다 이동" 행동이 여기서 나온다.** 09-17 에 넣은 사선 거부 → 재배치는 정상 동작했다 — 거짓 입력을 정직하게 소화했을 뿐이다.

### 1.4 ⚠ 레벨에서 방향을 읽을 때 주의

`BP_TitanTruck` 은 `BodyMesh` 의 `RelativeRotation` yaw 가 **270°** 다 — 즉 **장축(전장 6.5 m)이 액터 기준 X축**이다. 위 표의 "Y 가 장축" 은 **메시 로컬** 기준이고, 레벨에서 "어느 방향이 정면 사각인가"를 따질 때 액터 로테이션과 한 번 더 곱해야 한다. (이걸 안 보고 액터 축으로 바로 읽으면 사각의 방향이 90° 어긋난다.)

---

## 2. 수정 1 — 표적 액터를 레인 판정까지 넘긴다 [A · 코드]

**"표적을 맞혔으면 그 탄은 도달한 것이다."** 거리 규칙은 *무엇에* 맞았는지를 안 보고 *어디서* 맞았는지만 봤다. 답은 문턱을 키우는 것(트럭에 맞추면 사람에게 헐거워진다)이 아니라 **맞은 것이 표적 자신인지 묻는 것**이다.

### 2.1 API — 인자 하나가 세 함수를 타고 내려간다

```cpp
// AI/SoldierEngagement.h:980-1024
bool IsShotBlockedByWorld(const FVector& Muzzle, const FVector& Target,
                          const AActor* TargetActor = nullptr) const;
ESoldierAperture FindAperture(const FVector& Target, FVector& OutMuzzle,
                          const AActor* TargetActor = nullptr) const;
bool PlanAperture(const FVector& Target, bool bTooDearToShow, ESoldierAperture& OutAperture,
                  ..., const AActor* TargetActor = nullptr) const;
```

전부 **기본값 `nullptr`** 이라 기존 호출부는 그대로 컴파일되고, 표적 액터를 아는 자리에서만 넘긴다.

### 2.2 판정

```cpp
// AI/SoldierEngagement.cpp:421-429
const AActor* HitActor = Hit.GetActor();
const bool bStruckTarget = TargetActor != nullptr && HitActor != nullptr
    && (HitActor == TargetActor || HitActor->GetAttachParentActor() == TargetActor);

bBlocked = !bStruckTarget && FVector::Dist(Hit.ImpactPoint, Target) > LaneToleranceCm;
```

- **부착 액터까지 본다** — 차량의 RCWS/포탑처럼 별도 액터로 붙은 것을 맞혀도 도달이다.
- **기존 200 cm 규칙은 그대로 살아 있다.** `bStruckTarget` 이 아닐 때만 쓰이므로 **대인 동작은 한 글자도 안 바뀐다**(병사는 Pawn 이라 트레이스에서 애초에 무시된다 — 1.1(다)).

### 2.3 호출부 — 교전 틱이 표적 액터를 뽑아 넘긴다

```cpp
// AI/SoldierEngagement.cpp:1188-1192
const AActor* const LaneTarget = Contact.Enemy.Get();
const bool bActualLaneClear = !IsShotBlockedByWorld(ActualMuzzle, AimPoint, LaneTarget);
```

같은 `LaneTarget` 이 `:1272`(웅크린 총구) · `:1281` · `:1315-1316`(계획 자세)로도 간다.

**청각처럼 액터를 못 대는 기록은 `nullptr` 이 되고, 그때는 기존의 "추정점 근처에 떨어졌는가" 규칙이 그대로 쓰인다** [A]. 소리로만 아는 적에게 액터를 발명하지 않는다 — P130.

### 2.4 레인 캐시도 표적을 기억한다

09-21 에 넣은 `FLaneAnswer` 캐시(`LaneCacheMoveCm 15` · `LaneCacheSeconds 0.15`)에 `TargetActor` 를 추가하고 **캐시 키로도 비교**한다(`.h:996` · `.cpp:388` · `:436`).

```cpp
if (... && Answer.TargetActor == TargetActor && Dist(Answer.Muzzle, Muzzle) <= Move && ...)
```

없으면 **같은 좌표에 다른 표적이 왔을 때** 앞 표적의 답을 돌려준다 — 병사가 쓰러진 자리로 차량이 굴러오는 상황이 딱 그것이다. (캐시를 넣은 세션이 표적 액터를 안 썼으니 그때는 키가 아니어도 맞았다.)

---

## 3. 원인 2 / 수정 2 — 차량 위협의 눈높이 [A · 코드 · 정성 확인]

### 3.1 원인

`USoldierCoverComponent::GatherThreatEyes`(`AI/SoldierCover.cpp:260-300`)는 위협의 **눈**을

```
눈 = 지각 기록 위치 + ThreatEyeAboveContactCm (20 cm)
```

로 둔다. 기록은 가슴 높이(표적 소켓 또는 총구)에 있으니 "가슴 → 눈" 한 뼘만 얹는다는 뜻이고, **사람에 대해서는 맞다.**

차량은 아니다. 기록은 **차체 한가운데**(≈2 m)에 있고(2절의 바운즈 50%), 실제로 쏘는 것은 **지붕의 RCWS 포탑**(≈3.3 m+)이다. **1 m 이상 낮은 눈**으로 엄폐·그림자를 계산하면:

- 지붕과 차체 중심 사이를 지나는 모든 벽이 **없는 엄폐로 계산된다** → "숨었다고 판단한 자리"가 실제로는 뚫려 있다.
- 더 나쁜 것: 그 눈이 **차량 콜리전 내부**라, 거기서 출발하는 "여기서 반격 가능한가"(fight probe) 트레이스가 **설계에 없던 케이스**가 된다(출발점이 기하 안).

### 3.2 수정 — 형상에서 단차를 읽는다 (소켓 없는 표적에 한해)

```cpp
// AI/SoldierCover.cpp:296-311
float EyeAboveCm = ThreatEyeAboveContactCm;
if (const AActor* ThreatActor = Record.Enemy.Get())
  if (const USoldierIdentityComponent* Id = ThreatActor->FindComponentByClass<...>())
    if (!Id->HasSocket(Id->TargetSocket))          // ← 바운드박스 폴백 = 차량
    {
      const float ShapeStep = Id->GetEyeLocation().Z - Id->GetTargetLocation().Z;  // 80% − 50%
      EyeAboveCm = FMath::Max(EyeAboveCm, ShapeStep);
    }
```

- **게이트가 `HasSocket(TargetSocket)` 인 것이 핵심이다.** 소켓이 없다 = `GetSocketOrFallback` 이 바운드박스 비율을 쓰는 경로 = 차량. **병사는 튜닝된 +20 cm 를 그대로 유지한다.**
- 단차는 **그 대상의 형상**(자기 눈점 − 자기 표적점 = 바운즈 80% − 50%)에서 나온다.

### 3.3 이게 지각 규칙(P130) 위반이 아닌 이유

P130 은 "세계가 대신 알려 주지 않는다" — **위치**는 본 만큼만 안다. 여기서 쓰는 것은 위치가 아니라 **형상**이다: 트럭을 보는 사람은 누구나 "저건 지붕에 포탑이 있는 큰 것"임을 안다. 기록의 위치(어디 있는가, 얼마나 확신하는가)는 손대지 않았고, **그 위치 위에 얹는 높이만** 대상의 크기에서 나온다. `Location` 은 여전히 지각이 준 값이다.

### 3.4 곁가지 — `HasSocket()` public 노출

`USoldierIdentityComponent::HasSocket()`(`AI/SoldierIdentity.h:156` · `.cpp:75-86`)을 public 으로 올렸다. **`UFUNCTION` 이 아니다 = 순수 C++ 이라 리플렉션이 안 바뀌고, 따라서 Live Coding 으로 빌드된다** — 헤더에 새 `UPROPERTY`/`UFUNCTION` 을 넣었으면 에디터를 닫는 정식 빌드가 필요했을 것이다(`feedback_live_coding_uproperty_missing_property` 함정).

---

## 4. 검증

| 단계 | 내용 | 결과 |
|---|---|---|
| ① 재현 | 전용 시험 레벨 `L_SoldierTest` 에 `BP_TitanTruck` 배치, 적 3명 96 m, **트럭 정면 각도**(1.2절의 최악 조건) | 수정 전 `aperture 0` 재현 ✅ |
| ② 수정 후 | 같은 배치 | **사격 정상** ✅ |
| ③ 본 레벨 | **New_kadex_0811 3차 전투지** — 3분대 vs 트럭 | **사격 정상, 사용자 확인(2026-09-23)** ✅ |

②만으로 닫지 않은 이유: 시험 레벨은 트럭 하나와 평지뿐이라 원인 2(엄폐 눈높이)가 거의 안 드러난다. 벽·능선이 있는 본 레벨에서 확인해야 "숨었다고 판단한 자리가 뚫려 있다"가 사라졌는지 보인다 — 다만 **정성 확인이고 수치는 안 쟀다** → [C-168].

### 4.1 시험 레벨 세팅 메모 (재현용) [A · 중요]

`L_SoldierTest` 에서 **처음에 적이 ROE `hold` 상태여서 원인을 한 번 헛짚었다.** 재현하려면 알아야 할 것:

- **`ScenarioConfig_0.bDemoAutoStartScenario = false` 로 바꿔 뒀다.** 자동 시작이 켜져 있으면 `DT_ScenarioSteps_SoldierTest1v1` 의 `EnemyInfiltrate` 행이 걸려 ROE **HoldFire** 가 된다 → 로그의 `roe=hold` 가 그것이다.
- **분대 명령이 없으면 기본 배정이 ROE `Free`** 다 — 즉 "보이면 쏨". 자동 시작을 끄는 것이 곧 "자유 사격으로 두는 것".
- 트럭은 `RCWSFireControl.CurrentMode = Remote`, `bDemoForceCommandPostAutoFire = false` 라 **반격하지 않는다**(한쪽만 보는 실험).
- **진영 판정 자체는 원래부터 정상이었다** [A]: 트럭의 `DetectableTarget`(Friendly) → `USoldierLabBridgeSubsystem` 이 `USoldierIdentityComponent`(Friendly)를 붙임 → 적 보병이 표적으로 잡음. 로그의 `tgt=BP_TitanTruck` 이 그 증거다. 브리지·진영·탐지 어느 것도 안 고쳤다.

---

## 5. 남은 것 — [C]

| # | 항목 | 판정 기준 |
|---|---|---|
| **[C-166]** | **`TargetRadiusCm 45` 가 차량 크기를 반영해야 하는가** | `SoldierEngagement.h:526` 의 45 cm 는 **사람 가슴** 기준이다. 2.7 × 6.5 m 트럭도 반지름 45 cm 표적처럼 조준사격 게이트를 통과해야 해서, 원거리에서 산포가 45 cm 를 넘으면 `Aimed` 가 `Suppressive` 로 떨어진다(0절 로그의 `spread=409 err=411` — 4 m 산포에 4 m 오차). 사격은 이제 나가지만 **의도 등급이 실제보다 낮다.** 선택지: 표적의 `Identity` 에서 바운즈 반지름을 읽어 `max(TargetRadiusCm, 차량 반지름)` · 또는 차량 전용 값. ⚠ 반영하면 원거리 트럭 사격이 **전부 Aimed 로 올라가** 버스트/정착(`Settling`) 박자가 같이 바뀐다 — 값을 정하기 전에 그 부작용을 본다. 판정 = 트럭 상대 90 m 에서 `[Engage]` 의 의도 분포 |
| **[C-167]** | **차량의 조준점을 포탑으로 옮길 것인가** | 지금 조준점은 바운즈 50% = **차체 중앙**이고, "RCWS 를 노린다"는 표현은 코드 어디에도 없다. 수정 2 가 **위협의 눈**을 80% 로 올렸으므로 "쏘는 쪽은 지붕"이라는 정보는 이미 시스템 안에 있다 — 조준점만 안 따라갔다. 판정 = 포탑 조준이 전술적으로 읽히는가(명중 판정·데미지 모델과 같이 봐야 함) |
| **[C-168]** | **차량 위협에 대한 엄폐 품질(수정 2 이후)** | 본 레벨에서 **정성 확인만** 했다(4절 ③). 수치 미측정. 판정 = 트럭 상대 교전에서 `ExposureByStance` · 선택된 자리의 실제 피격률이 대인 교전의 그것과 같은 범위인가 |

---

## 6. 원칙

**P189 — 표적 자신은 벽이 아니다.** 사격 레인 트레이스는 *어디서* 맞았는지가 아니라 *무엇에* 맞았는지를 먼저 봐야 한다. 거리 문턱(`LaneToleranceCm`)은 **사람 크기**를 상정한 값이고, Pawn 이 아닌 표적(차량)은 트레이스가 무시하지도 않으면서 조준점이 자기 껍데기에서 수 m 안쪽에 있어 **정직한 명중이 전부 "벽에 던진 탄"으로 읽힌다.** 표적 액터를 판정까지 내려보내 `HitActor == TargetActor || 부착부모 == TargetActor` 면 도달로 처리한다. 새 표적 종류(장갑차·구조물·드론)를 붙일 때마다 같은 자리에서 같은 증상이 난다 — **`aperture 0` 인데 `believed/worth` 가 통과라면 레인부터 본다.**

---

## 7. 바뀐 파일

| 파일 | 내용 |
|---|---|
| `AI/SoldierEngagement.h` | `IsShotBlockedByWorld`/`FindAperture`/`PlanAperture` 에 `const AActor* TargetActor = nullptr` · `FLaneAnswer::TargetActor` |
| `AI/SoldierEngagement.cpp` | 레인 판정 `bStruckTarget`(`:421-429`) · 캐시 키(`:388`, `:436`) · 호출부 `LaneTarget`(`:1188-1192`, `:1272`, `:1281`, `:1315-1316`) |
| `AI/SoldierCover.cpp` | `GatherThreatEyes` 의 형상 기반 눈높이 단차(`:296-311`) |
| `AI/SoldierIdentity.h` / `.cpp` | `HasSocket()` public(비-`UFUNCTION`) |

레벨/에셋: `L_SoldierTest` 에 `BP_TitanTruck` 배치 + `ScenarioConfig_0.bDemoAutoStartScenario = false`(4.1절).
