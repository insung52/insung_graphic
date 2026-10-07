# 사격 콘 배선 누락 · 총구 방향 사격 게이트

2026-10-07 / 완료·PIE 실측 검증 / 무기가 AI 사격 콘을 쓰지 않던 배선 누락 수정 + 총구 방향 사격 게이트(급박도 완화) 신설

전편(같은 축): `ai/2026-09-21_per_squad_field_edge_advance_fire_model.md` 3절(**AI 가 소유하는 사격 콘** `GetShotSpreadDegrees` — 그 문서가 "무기 BP 배선은 [W93]" 으로 남긴 것이 이 문서의 1절이다) · `ai/2026-09-30_ai_real_pitch_and_aim_smoothing.md` 14절(조준 보정 적분기 = 3절에서 틈의 정체로 드러나는 것) · `squad/2026-09-18_squad_layer_fixes_quota_engage_range.md` 8절(사격 결정 게이트 목록 — 4절이 그 목록에 한 항을 더한다).
원칙: **P206**(조준 의도 ≠ 총구 방향) · **P207**(바닥이 0 이 아닌 축에 선형 완화는 상시 완화다) · P10(계측 먼저) · P195~P199(조준 층).
값: **[C-181]~[C-182]**, 작업 **[W135]~[W136]**. **[W93] 해결**(무기 BP 콘 배선).

---

## 0. 두 줄 요약

1. **무기가 AI 의 사격 콘을 읽지 않고 고정 3° 원뿔로 탄을 뿌리고 있었다.** 9월에 만든 `GetShotSpreadDegrees` 는 계산만 되고 소비자가 없었다 — BP 노드 하나(+ C++ 헬퍼 하나)로 연결했다.
2. **사격 허가가 "AI 가 생각하는 조준 방위"만 보고 총구가 어디를 향하는지는 아무도 보지 않았다.** 달리다 멈춘 첫 수 발이 엉뚱한 방향으로 나가던 원인 — 총구 forward 를 보는 게이트를 넣고, 급박할 때만 완화하게 했다.

---

## 1. 계기와 1차 발견 — 무기가 AI 콘을 안 쓰고 있었다 [A · 코드]

**제보**[사용자]: "적군 총 정확도가 너무 낮다."

무기 BP 를 열어 확인한 것:

- `BP_AR4Rifle` EventGraph 가 탄 방향을 `RandomUnitVectorInConeInDegrees` 로 뽑는다.
  - `ConeDir` = `MuzzlePoint` 컴포넌트의 Forward
  - `ConeHalfAngleInDegrees` = BP 변수 **`BulletSpreadDegrees`**
- `BulletSpreadDegrees = 3.0` **고정**. 적군 `BP_AK47Rifle` 은 `BP_AR4Rifle` 의 자식이고 같은 3.0 이다.

즉 **조준 상태·정착·자세·이동과 무관한 고정 반각 3° 원뿔**이었다. 정지해서 완전히 조준을 끝낸 병사와 달리면서 난사하는 병사의 탄이 정확히 같이 퍼진다.

### 1.1 고정 3° 가 무슨 뜻인가 [A · 기하]

탄착 반경 = `거리 × tan(반각)`. 사람을 반경 45 cm 원으로, 탄을 원뿔 안 **균일 분포**로 근사하면 명중 확률 = `(45 / 반경)²`:

| 거리 | 탄착 반경(3°) | 명중 확률(근사) |
|---|---|---|
| 25 m | **1.31 m** | 약 **11 %** |
| 50 m | **2.62 m** | 약 **3 %** |
| 100 m | **5.24 m** | 약 **0.8 %** |

실제 사격 분포는 중심이 두꺼우니 이 근사는 하한에 가깝다 — 그래도 **100 m 에서 사실상 안 맞는다**는 결론은 바뀌지 않는다. 사용자 제보는 정확했다.

### 1.2 교전 컴포넌트는 콘을 제대로 만들고 있었다 [A · 코드]

`USoldierEngagementComponent` 는 09-21 부터 콘을 직접 계산해서 내보낸다(`GetShotSpreadDegrees()`, `SoldierEngagement.h:446`). 입력:

| 항목 | 값 | 자리 |
|---|---|---|
| 정착 완료 시 기본 콘 | `WeaponSpreadDegrees` **0.8** | `.h:530` |
| 조준 정착(선회 직후 얹히는 값 / 시간상수) | `AimSettleInitialDeg` **2.5** / `AimSettleSeconds` **0.4** | `.h:540` · `:543` |
| 발당 반동 | `RecoilKickDeg` **0.6** | `.h:546` |
| 이동 흔들림 바닥 | `MoveWobbleDeg` **1.5** | `.h:549` |
| 이동 배율(조깅 기준) | `MovementSpreadScale` **6** | `.h:795` |
| 린 배율 | `LeanSpreadScale` **1.5** | `.h:953` |
| 맹목사격 배율 | `BlindSpreadScale` **15** | `.h:964` |

**이 값들이 전부 아무 효과가 없었다.** 소비자가 없었기 때문이다 — 09-21 문서가 "무기 BP·`WantsToAim` 배선은 [W93]" 으로 미완 표시해 둔 그 항목이고, 사용자는 이것을 **"적용 안 되어 있던 게 버그"** 로 판단했다.

### 1.3 손해가 이중이었다 [A]

| | 증상 |
|---|---|
| ① | **조준을 잘해도 탄은 3°** — AI 층의 정착·버스트·자세 판단 전체가 탄도에 반영되지 않는다 |
| ② | **AI 가 자기 콘을 0.8° 라고 믿고 사격 판정을 내린다** — `TargetRadiusCm 45`(`.h:559`) × `AimedHitTolerance 2`(`.h:562`) 로 "이 산포면 맞는다"를 계산해 방아쇠를 당겼는데, 실제로 나가는 탄은 3° 짜리였다 |

②가 더 나쁘다. **실제보다 정확하다고 착각한 채** 쏘므로, 탄약·사격 리듬·제압 모델 전부가 틀린 전제 위에 있었다.

---

## 2. 수정 1 — 콘 배선 (C++ 헬퍼 1개 + BP 노드 1개) [A · 코드]

### 2.1 왜 BP 만으로 깔끔하지 않았는가 [A · 도구 제약]

무기 BP 에서 `OwningCharacter` 의 교전 컴포넌트 게터를 직접 부르려고 했으나:

1. `Actor|GetComponentByClass` 는 **`ActorComponent` 를 돌려준다** → `GetShotSpreadDegrees` 의 `self`(= `SoldierEngagementComponent`)에 타입 불일치로 못 꽂힌다.
2. 캐스트 노드가 필요한데 **MCP 로는 pure 캐스트 전환이 불가**(`bIsPureCast` 쓰기 거부).
3. 콘 각도는 **pure 값 체인**이다 — exec 이 필요한 캐스트를 사격 흐름에 끼우려면 EventGraph 를 크게 건드려야 한다(그래프 재배선 = 다음 세션이 읽기 어려워지는 쪽).

### 2.2 그래서 null-safe 정적 헬퍼 하나 [A · `SoldierEngagement.h:461` · `.cpp:624-631`]

```cpp
static float USoldierEngagementComponent::GetShotSpreadDegreesFor(
    const AActor* Owner, float FallbackDegrees = 3.f);
```

```cpp
const USoldierEngagementComponent* Engagement =
    Owner != nullptr ? Owner->FindComponentByClass<USoldierEngagementComponent>() : nullptr;
return Engagement != nullptr ? Engagement->GetShotSpreadDegrees() : FallbackDegrees;
```

**Fallback 이 0 이 아니라 인자인 것이 핵심이다.** 교전 컴포넌트가 없는 소유자(테스트용 액터, 플레이어가 든 총)에게 0° 를 주면 **레이저처럼 정확한 소총**이 되고, 그건 아무도 버그로 신고하지 않는 종류의 실패다. 소유자 쪽 값(= 그 무기의 `BulletSpreadDegrees`)을 그대로 쓴다.

### 2.3 BP 배선 — 노드 1개 [A]

`BP_AR4Rifle` EventGraph:

```
OwningCharacter        ──→ Owner
BulletSpreadDegrees    ──→ Fallback Degrees
            GetShotSpreadDegreesFor.ReturnValue ──→ RandomUnitVectorInConeInDegrees.ConeHalfAngleInDegrees
```

기존 `BulletSpreadDegrees → ConeHalfAngleInDegrees` **직결은 끊었다**. `BP_AK47Rifle` 은 자식이라 자동 적용.

### 2.4 효과 [A · 기하 / B · 기대치]

| 상황 | 전 | 후 |
|---|---|---|
| 정지·조준 완료 | 3° | **0.8°**(바닥) |
| 이동 중 | 3° | 넓어진다(`MovementSpreadScale 6`) |
| 연사 중 | 3° | 발마다 +0.6° |
| 린 / 맹목사격 | 3° | ×1.5 / ×15 |

50 m 탄착 반경은 0.8° 에서 **0.70 m** 다(근사 명중률 약 40 %). 다만 **교전 중에 콘이 0.8° 로 내려가는 일은 드물다** — 5절 PIE 로그의 실제 `cone` 은 **4.38 ~ 6.31°** 였다(정착 잔량 + 이동 흔들림). 그래서 현실적인 기대치는 "50 m 명중률 3 % → **25 % 내외**" [B] 이고, 진짜 변화는 숫자 하나가 아니라 **콘이 상황에 반응하기 시작했다**는 것이다.

---

## 3. 2차 발견 — 총구가 표적을 향하는지 아무도 보지 않았다 [A · 코드 · 실측]

**사용자 관찰**: "달리기 애니메이션이 안 끝났는데 쏜다."

### 3.1 사격 허가 게이트를 다시 읽어 보면

```
bBelieved && bWorthTheRound && bHaveAperture && bOnTarget && !bReloading && Ammo > 0
```

그중 `bOnTarget` 은(`SoldierEngagement.cpp:1515-1520`):

```cpp
AimErrorDeg = angle(AimRotation.Vector(), DesiredAim.Vector());
bOnTarget   = AimErrorDeg <= ConeDeg * OnTargetConeRatio;   // OnTargetConeRatio 1.0, .h:855
```

**`AimRotation` 은 AI 가 슬루 중인 자기 조준 방위이고 `DesiredAim` 은 표적 방위다 — 둘 다 AI 내부의 숫자다.** 실제 총구가 어디를 보는지는 이 식에 없다.

함께 걸려 있는 `bHaveAperture`(= `PlanAperture`)도 총구 **위치**에서의 사격 레인 판정이다 — **방향이 아니다**(`ai/2026-09-23_vehicle_target_engagement_fix.md` 2절).

### 3.2 탄은 총구 방향으로 나간다 — 그 둘을 메우는 것이 적분기다 [A · `Pose/SoldierAIBridgeComponent.cpp:443-501`]

조준 보정 적분기(09-30 재설계)가 "AI 가 원하는 방위"와 "몸·총이 실제로 향한 방향"의 차이를 매 프레임 줄인다. 조건:

```cpp
bWeaponOnAim = bAOActiveNow && !bMontage && !bHitReacting
               && WeaponLowered.ReadFloat(Self) < 0.05f;        // :449
```

| 항목 | 값 | cvar |
|---|---|---|
| 보정 비율 | 10 /s | `SoldierLab.Aim.CorrectionRate` |
| 데드밴드 | 0.3° | `SoldierLab.Aim.CorrectionDeadbandDeg` |
| 최대 속도 | 60 °/s | `SoldierLab.Aim.CorrectionMaxDegPerSec` |
| 누출(조준 밖일 때) | 6 /s | `SoldierLab.Aim.CorrectionLeakRate` |

**총을 올리는 동안(`WeaponLowered ≥ 0.05`)에는 `bWeaponOnAim` 이 false 라 보정이 0 이고, 이미 쌓인 보정은 누출로 풀린다.** 이것은 설계대로다(총이 내려가 있는데 조준 보정을 걸면 포즈가 깨진다 — P191).

### 3.3 합치면 [A · 실측]

```
달리다 멈춤
 → AI 내부 방위(AimRotation)는 선회 40~150°/s 로 금방 표적에 도달 → bOnTarget = true
 → 그런데 총은 아직 로우레디에서 올라오는 중 → 보정 0, 총구는 스윙 중
 → 게이트는 열려 있고 탄은 총구 forward 로 나간다 → 첫 수 발이 엉뚱한 방향
```

PIE 로그에서 `aimErr 0.0` 인데 총구 오차가 **74 ~ 131°** 인 줄들이 확인됐다. 즉 **"조준 완료"라고 판정한 순간에 총구는 표적의 등 뒤를 보고 있을 수 있다.**

---

## 4. 수정 2 — 총구 방향 게이트 + 급박도 완화 [A · 코드]

### 4.1 총구 forward 를 물을 수단 [A · `SoldierIdentity.h:197` · `.cpp:412-431`]

```cpp
UFUNCTION(BlueprintPure, Category = "SoldierLab|Identity")
bool GetMuzzleForward(FVector& OutForward) const;
```

- **`GetMuzzleLocation` 을 먼저 호출**해서 같은 소켓 탐색·같은 캐시 경로(S6 소켓 캐시, BeginPlay 때 없던 무기를 1 Hz 로 다시 찾는 것까지)를 그대로 쓴다. 두 함수가 서로 다른 총을 가리킬 여지를 없앴다.
- 그 뒤 `MuzzleComponent->GetSocketQuaternion(MuzzleSocket).GetForwardVector()` = 소켓의 X 축.
- **총구를 못 찾으면 `false`** → 게이트는 통과(기존 동작 유지). 메시 없는 시험 액터나 무기 스폰 전 프레임에서 사격이 통째로 멈추지 않게 하는 쪽을 골랐다.

### 4.2 게이트 [A · `SoldierEngagement.cpp:1528-1574`]

```
muzErr    = angle( 총구 forward , (AimPoint - ActualMuzzle) )
허용치    = ConeDeg × MuzzleOnTargetConeRatio × Lerp(1, MuzzleUrgentToleranceScale, 유효급박도)
bAimReady = bOnTarget && (muzErr <= 허용치)
```

`bAimReady` 는 **사격 허가(`:1580`)와 `Traversing` 분기(`:1657`) 양쪽**에서 옛 `bOnTarget` 자리를 그대로 대체한다 — 즉 "조준됐다"의 정의 자체가 바뀌었다. 로그의 `onTarget` 도 이 값이다.

| 프로퍼티 | 값 | 자리 |
|---|---|---|
| `MuzzleOnTargetConeRatio` | **1.5** | `.h:980` |
| `MuzzleUrgentToleranceScale` | **12** | `.h:994` |
| `AimRushThreshold` | **0.5** | `.h:1007` |
| `AimRushCloseRangeCm` | **400** | `.h:1019` |
| `AimRushFadeCm` | **600** | `.h:1023` |

### 4.3 급박도 [A · 사용자 설계 · `:1535-1544`]

```cpp
CloseUrgency = 1 - clamp((DistanceCm - AimRushCloseRangeCm) / AimRushFadeCm, 0, 1);
RawUrgency   = clamp(max3(CloseUrgency, SuppressionNow, PoseUrgency), 0, 1);
AimRushUrgency = clamp((RawUrgency - AimRushThreshold) / (1 - AimRushThreshold), 0, 1);
```

- 근접도: **400 cm 이내 1.0 → 600 cm 더해 1000 cm 에서 0**(선형).
- 제압(`SuppressionNow`) · 자세 급박도(`PoseUrgency`) 와 **최댓값**을 취한다.
- **`GetTension()` 은 일부러 제외했다** [사용자 설계] — 긴장도는 반감기 20 초라 교전 내내 깔려 있어서, 넣으면 완화가 상시화된다.

### 4.4 ★ 문턱이 필요했던 이유 (실측 근거) [A · 로그]

1차 적용에서는 **문턱 없이** `MuzzleUrgentToleranceScale 200` + `raw` 를 그대로 Lerp 에 넣었다. 결과:

- `PoseUrgency` 의 **바닥이 0 이 아니다** — 접촉만 있어도 `UrgencyContactIdle` **0.35**(`.h:719`), 평상시 `UrgencyIdle` **0.15**(`.h:715`).
- 따라서 허용치가 상시 콘의 **70 ~ 200 배** = 사실상 무제한.
- 로그에 `muzErr 94` 인데 `onTarget 1` — **게이트가 한 발도 막지 못했다.**

수정: `AimRushThreshold 0.5` 를 추가해 `유효 = clamp((raw − 0.5) / 0.5)` 로 바꾸고 배율을 **200 → 12** 로 내렸다. 이제 raw 0.5 이하(= 접촉만, 평상시)는 **완화가 정확히 0** 이다.

→ 이것이 **P207** 이다.

### 4.5 최종 거동 (콘 3° 기준) [A · 계산]

| 상황 | raw | 유효 | 배율 | 허용치 | 뜻 |
|---|---|---|---|---|---|
| 10~16 m, 접촉만 | 0.35 | **0.00** | ×1 | **4.5°** | 조준 강제 — 총구가 올 때까지 기다린다 |
| 7 m | 0.50 | **0.00** | ×1 | **4.5°** | 아직 엄격 |
| 5 m | 0.83 | 0.67 | ×8.3 | **≈ 37°** | 대부분 통과 |
| 4 m 이내 · 재장전 직후 · 강한 제압 | 1.00 | **1.00** | ×12 | **54°** | 즉시 발사 |

**각도 하한은 두지 않았다** [사용자 지시] — "등 뒤로 쏠 정도면 그냥 놀란 것"이라, 급박할 때는 프리패스를 허용한다. 54° 는 3° 콘일 때의 값이고 콘이 커지면 같이 커진다(교전 중 콘 5° 면 90°).

### 4.6 로그 진단 [A · `:1964-1975`]

```
onTarget %d(aimErr %.1f muzErr %.1f rush %.2f/%.2f)
```

`rush` 는 **raw / 유효** 두 개다. 이 한 줄에서 구분되는 것:

| 읽는 것 | 알 수 있는 것 |
|---|---|
| `aimErr` 작고 `muzErr` 큼 | AI 는 조준됐다고 믿지만 총구가 안 왔다 = 3절의 그 틈 |
| `rush` 왼쪽만 큼(`0.35/0.00`) | 입력은 올라왔지만 **문턱을 못 넘어 완화가 안 걸렸다** |
| `rush` 양쪽 큼 | 실제로 완화가 걸렸다 |

4.4 절의 사고를 다시 내지 않으려면 **왼쪽/오른쪽을 같이 봐야 한다** — 1차 적용에서 틀린 자리가 정확히 "입력은 낮은데 완화는 full" 이었다.

---

## 5. PIE 실측 검증 (2026-10-07, `L_SoldierTest`) [A · 로그]

세 구간이 모두 로그에 나왔다.

**① 근접 — 완화 full, 기다리지 않는다**
```
dist=356 onTarget 1(aimErr 4.4 muzErr 6.0 rush 1.00/1.00) cone 5.06
```
3.56 m → 근접도 1.0 → 유효 1.00. 허용치 = 5.06 × 1.5 × 12 ≈ 91°. `muzErr 6.0` 은 여유롭게 통과.

**② 중간 — 부분 완화**
```
dist=643 onTarget 1(aimErr 0.0 muzErr 18.6 rush 0.70/0.40) cone 4.38
```
6.43 m 의 근접도는 0.60 인데 raw 가 0.70 → 제압/자세 쪽이 올렸다. 유효 0.40 → 허용치 = 4.38 × 1.5 × 5.4 ≈ **35°**. `muzErr 18.6` 통과.

**③ 중거리 — 방위는 완벽한데 총구가 안 와서 보류**
```
dist=1281 onTarget 0(aimErr 0.0 muzErr 83.2 rush 0.35/0.00) cone 6.31
```
12.81 m → 근접도 0 → raw 0.35(접촉 바닥) → **유효 0.00** → 허용치 = 6.31 × 1.5 = **9.5°**. `muzErr 83.2` → **발사 보류.** `aimErr 0.0` 과 나란히 있는 이 줄이 3절의 틈을 그대로 보여 준다 — 수정 전이라면 여기서 83° 빗나간 탄이 나갔다.

**사용자 확인**: 근접에서 답답하지 않고, 중거리에서는 조준하고 쏜다.

---

## 6. ⚠ 작업 중 드러난 환경·도구 함정 (다음 세션용)

### 6.1 P4 서버가 또 바뀌었다 [A]

옛 설정으로 `-p`/`-c` 오버라이드를 붙이면 안 된다:

| 호출 | 결과 |
|---|---|
| `-p 192.168.0.99:1666 -c user4_DESKTOP-81S78B2_4340` | `Can't create a new user - over license quota`(옛 서버에 `grapicar4` 가 없어 생성을 시도) |
| 위 + `-u user4` | `session has expired` |

둘 다 "로그인/라이선스 문제"처럼 보여서 오진하기 쉽다. **현재 설정은 `p4 set` 에 이미 잡혀 있으므로 그냥 `p4 edit <경로>` 로 부르면 된다.**

| 항목 | 값 |
|---|---|
| 서버(자기 보고 주소) | **`p4dtest:1666`** (P4D/LINUX 2026.1) |
| 유저 | **`grapicar4`** |
| 클라이언트 | **`insung_desktop1`** |
| 클라이언트 루트 | **`C:\working\works`** |
| depot 경로 | **`//dev/main/kadex/titan_example/...`** |

`p4 info` 의 `Server address` 가 `P4PORT`(`ssl:p4d.grapicar.com:1666`) 와 달라 보이는 것은 **서버가 자기 내부 주소를 보고하는 것이라 정상**이다. 상세는 titan 루트의 `../infra_architecture/2026-10-01_perforce_server_migration.md`.

### 6.2 BP 컴포넌트 템플릿 값이 배치된 레벨 액터에 안 내려간다 [A · 실측]

`MuzzleUrgentToleranceScale` 을 **BP 컴포넌트 템플릿에만** 1.0 으로 바꿨을 때:

| 레벨 | 결과 |
|---|---|
| `New_kadex_0811` 인스턴스 | 따라갔다 |
| **`L_SoldierTest` 의 병사 6명** | **200 을 유지** → PIE 에서 게이트가 전혀 물리지 않았다 |

레벨에 배치된 액터의 컴포넌트는 자기 인스턴스 값을 들고 있다 — **`<액터>.AC_SoldierEngagement` 에 직접 써야 한다.** (titan 쪽 같은 함정: 메모리 `feedback_mcp_cdo_write_skips_level_instances`.)

### 6.3 Live Coding 빌드는 기존 프로퍼티의 메모리 값을 살려둔다 [A · 실측]

| 프로퍼티 | 빌드 후 상태 |
|---|---|
| `AimRushThreshold`(**신규**) | 코드 기본값 0.5 로 나타났다 |
| `MuzzleUrgentToleranceScale`(기존, 테스트로 1.0 심어 둠) | **1.0 이 템플릿·인스턴스에 그대로 남아 새 기본값 12 를 가렸다** |

**"빌드했는데 값이 안 바뀐 것처럼 보이면 코드를 다시 읽기 전에 템플릿·인스턴스 값을 먼저 읽을 것."** 기존 `UPROPERTY` 의 기본값을 코드에서 바꿔도, 이미 그 프로퍼티를 오버라이드한 에셋/인스턴스는 당연히 안 따라온다 — 여기서는 "테스트로 심은 값"이 그 오버라이드였다.

---

## 7. 바뀐 파일 / 상태

| 파일 | 내용 |
|---|---|
| `Source/SoldierLab/AI/SoldierEngagement.h` | `GetShotSpreadDegreesFor` 선언(`:461`) · `MuzzleOnTargetConeRatio 1.5`(`:980`) · `MuzzleUrgentToleranceScale 12`(`:994`) · `AimRushThreshold 0.5`(`:1007`) · `AimRushCloseRangeCm 400`(`:1019`) · `AimRushFadeCm 600`(`:1023`) |
| `Source/SoldierLab/AI/SoldierEngagement.cpp` | 헬퍼 본문(`:624-631`) · 급박도 + 총구 게이트 + `bAimReady`(`:1528-1574`) · 사격 허가·`Traversing` 분기가 `bAimReady` 사용(`:1580`, `:1657`) · 로그 확장(`:1964-1975`) |
| `Source/SoldierLab/AI/SoldierIdentity.h` / `.cpp` | `GetMuzzleForward`(`.h:197` · `.cpp:412-431`) |
| `Content/SoldierLab/Weapons/Blueprints/BP_AR4Rifle` | 콘 배선 교체(컴파일·저장 완료, **P4 체크아웃 상태**) |

**에디터 메모리에만 남은 것** ⚠: 테스트용으로 심었던 인스턴스/템플릿 오버라이드는 **12**(= 코드 기본값)로 되돌려 두었고 **저장하지 않았다.**

- 에디터를 재시작하면 오버라이드가 사라지고 코드 기본값을 쓴다 → 의도한 상태.
- **레벨을 저장하면 그 오버라이드가 박힌다** — 이후 코드 기본값을 바꿔도 `L_SoldierTest` 는 안 따라온다. 다른 이유로 레벨을 저장할 때 주의.

`BP_AK47Rifle`(적군)은 수정 없음 — 자식이라 자동 적용.

---

## 8. 남은 것 — [C] / [W]

| # | 항목 | 판정 기준 |
|---|---|---|
| **[C-181]** | **총구 소켓과 `MuzzlePoint` 컴포넌트가 같은 방향인가** | 게이트가 보는 것은 **캐릭터 손에 붙은 무기 메시(`WeaponMesh`, `SK_KA74U_X`)의 `Muzzle` 소켓**이고, 탄이 실제로 나가는 것은 **무기 액터 `BP_AR4Rifle` 의 `MuzzlePoint` 컴포넌트** forward 다(이 두 총의 역할 분담은 `ai/2026-09-23_corpse_vehicle_interaction_and_weapon_drop.md` 의 함정 절). 둘 다 같은 손 소켓 계열이라 사실상 같을 것으로 보이지만 **미실측** — 각도 차이가 있으면 게이트가 **체계적으로 어긋난다**(한쪽으로 늘 muzErr 오프셋). 판정 = 같은 프레임에 `GetMuzzleForward` 와 `MuzzlePoint->GetForwardVector()` 를 찍어 각도차 측정, 1° 이내면 무시 |
| **[C-182]** | **본 레벨 `New_kadex_0811` 에서의 거동** | 검증은 `L_SoldierTest`(교전 3~13 m)에서만 했다. 본 레벨은 교전 거리가 수십 m 라 **4.5° 엄격 구간이 거의 전부**가 된다 — 완화가 안 걸리는 쪽의 거동(첫 발까지의 지연, 발사율 하락 여부)이 거기서 처음 드러난다. 판정 = 3단계 시나리오 완주에서 `[Engage]` 의 `onTarget 0(... muzErr ...)` 보류 빈도와 분대 교전 템포 |
| **[W135]** | **`WeaponSpreadDegrees` 진영별 분리** | 지금은 아군·적군 모두 **0.8**(`.h:530`)이다. "적군만 덜 정확하게"가 필요하면 **`BP_Soldier_Hostile` 의 교전 컴포넌트에서 값만 올리면 된다**(코드 변경 불필요). 1절의 표가 값↔명중률 환산이다 |
| **[W136]** | **이번 변경분 P4 제출** | 소스 4파일(`SoldierEngagement.{h,cpp}` · `SoldierIdentity.{h,cpp}`) + `BP_AR4Rifle` 이 체크아웃 상태. ⚠ **파일 목록을 보여 주고 사용자 명시 승인을 받은 뒤에만 제출**(`CLAUDE.md` 6절 · [W134] 와 같은 사유) |

---

## 9. 원칙

**P206 — AI 의 조준 의도와 실제 총구 방향은 다른 값이다.** 발사 판정은 **탄이 나가는 쪽**을 근거로 해야 한다. `bOnTarget` 은 AI 내부의 두 방위(`AimRotation` vs `DesiredAim`)를 비교한 것이고, 그 둘이 일치해도 총구는 수십 도 벗어나 있을 수 있다 — 둘을 메우는 조준 보정 적분기가 **총이 견착된 뒤에만** 돌기 때문이다(`bWeaponOnAim`). 같은 함정은 사격 레인(`bHaveAperture`)에도 있었다: 그건 총구 **위치**만 쓰고 방향은 안 본다. **"이 AI 는 X 를 향하고 있다"를 쓰는 모든 자리에서 "그 X 로 무엇이 나가는가"를 따로 물어야 한다.** 새 사격 주체(차량 RCWS, 드론)에도 같은 질문이 그대로 간다.

**P207 — 완화 계수를 바닥이 0 이 아닌 축에 선형으로 걸면 상시 완화가 된다.** `Lerp(1, Scale, x)` 에 넣는 `x` 의 **최솟값**을 먼저 확인할 것. `PoseUrgency` 는 접촉만으로 0.35, 평상시에도 0.15 였고 `GetTension()` 은 반감기 20 초로 교전 내내 깔린다 — 그런 축을 그대로 쓰면 게이트가 **한 번도 닫히지 않는다**(실측: 허용치가 콘의 70~200 배, `muzErr 94` 에 `onTarget 1`). 해법은 배율을 줄이는 것이 아니라 **문턱을 두어 바닥 구간을 정확히 0 으로 깎는 것**이다(`clamp((raw − t) / (1 − t))`). 그리고 **진단 로그에는 raw 와 유효 둘 다 찍는다** — 한쪽만 보면 "입력이 낮은데 완화는 full" 인 상태를 못 본다.
