# ★ 루트 facing이 56° 틀어져 있었다 — 워핑이 아니라 기준값이 문제였다

2026-09-08 / **성공 (실측→교정→재실측)** / `AM_EncodeRootBone`이 pelvis에서 루트 yaw를 역산하면서 **견착 자세의 골반 블레이드가 루트에 통째로 구워졌다.** Orientation Warping이 그만큼 다리를 틀고 있었다.

관련: **[C-31] 판정 번복** · [C-46] · 신규 [C-49] / 관련 문서: `2026-09-04_c34_clip_curve_mapping.md` · `2026-09-06_p0-2_wiring_plan.md`

---

## 1. 증상

AI 병사의 다리가 **진행 방향과 어긋난 채** 움직였다. 사용자 관측:

> 상체가 보는 방향이랑 다리가 가려하는 방향이랑 전혀 싱크가 안 맞아서 발이 이상하게 움직이고 있어.

`wantsToAim`을 꺼서 몸통이 이동 방향을 향하게 해도 **여전히 틀어졌다.** 즉 조준/스트레이프 문제가 아니었다.

## 2. 원인 — 엔진 코드

`EncodeRootBoneModifier.cpp:109-111`:

```cpp
const double Yaw = FMath::Atan2(-WeightedBoneHeading.X, WeightedBoneHeading.Y) * (180.f / UE_PI);
const FRotator Rotator(0.f, Yaw, 0.f);
RootTransformNew.SetRotation(Rotator.Quaternion());
```

**매 키마다 지정한 본(우리는 pelvis)의 축 yaw를 그대로 루트 yaw로 쓴다.** 그래서 루트에 두 가지가 구워진다:

1. 걸음마다의 **골반 좌우 흔들림**
2. 견착 블레이드 스탠스의 **상시 오프셋**

`FAnimNode_OrientationWarping`은 **"원하는 이동 방향 vs 클립의 루트모션 방향"** 의 차이만큼 다리를 회전시킨다
(설계 5.5.2절). 기준인 루트 facing이 틀어져 있으면 **똑바로 걸으라고 해도 그만큼 다리가 돌아간다.**

> GASP 출하 클립에는 이 문제가 없다 — Epic이 루트모션을 저작해서 넣었다. **우리만 pelvis에서 역산했다.**

## 3. 실측 — `USoldierRootFacingModifier`

추측으로 축을 바꾸지 않고 **먼저 쟀다**(`../CLAUDE.md` P10).
`Source/SoldierLabEditor/AnimModifiers/SoldierRootFacingModifier.{h,cpp}`

```
DiagnoseOnly              키마다 (루트 yaw) − (평활화된 진행 방향) 을 평균·표준편차·범위로 로그
EncodeFromTravelDirection 루트 yaw를 진행 방향으로 재인코딩
RemoveConstantOffset      루트 yaw의 모양은 두고 평균 오프셋만 제거
```

진행 방향은 **± SmoothingWindow(기본 0.2s) 창의 루트 이동**에서 뽑는다 — 한 프레임 델타는
걸음 흔들림에 묻혀 방향이 안 나온다. `MinSpeedForDirection`(10cm/s) 미만이면 직전 방향을 유지한다.

### 3.1 교정 전

```
[Walking_Anim] keys=42 len=1.367s window=+-6 keys
  root yaw - travel:   mean -56.0 deg, sd 8.3, range -69.0 .. -41.4
  pelvis(Y) - travel:  mean -56.0 deg, sd 8.3   ← 루트가 pelvis에서 나왔으니 동일
```

**루트가 진행 방향보다 56° 틀어져 있고, 걸음마다 ±8.3° 흔들린다.**

계산: 클립의 루트모션 방향(루트 기준) = +56° → 런타임에 직진 요청(0°) → **워핑이 다리를 −56° 회전**.

### 3.2 교정 후

```
[Walking_Anim]
  root yaw - travel:   mean 0.0 deg, sd 0.0, range -0.0 .. 0.0    ★
  pelvis(Y) - travel:  mean -56.0 deg, sd 8.3                     ← 골반은 그대로
```

**루트만 정렬되고 골반은 여전히 56° 블레이드다.** 그게 견착 자세이므로 **그대로여야 맞다.**
보이는 포즈는 하나도 안 바뀐다 — 루트 프레임만 돌리고 **직계 자식 본 6개를 역보정**했다
(`EncodeRootBoneModifier.cpp:117-121`과 같은 처리).

> 사용자 지적이 문제를 열었다: *"루트가 바라보는 방향이 진행방향이랑 똑같으면 안 되지 —
> 견착 자세에서는 아예 틀어져 있을 테니까."*
> **틀어져 있어야 하는 것은 골반이고, 루트가 아니었다.**

## 4. [C-31] 판정 번복

09-03에 "루트 회전 축 = pelvis Y"로 닫았는데, 그 근거 두 개가 **facing을 검증하지 않는다**:

| 당시 근거 | 실제로 검증한 것 |
|---|---|
| "루트 전진 잘됨" | **위치**뿐. 위치는 pelvis translation이라 축과 무관하다 |
| 회전 클립 `net yaw 88.8°` | **회전량**뿐. 절대 facing이 아니다 |

**"루트가 바라보는 방향 = 진행 방향인가"는 한 번도 재지 않았다.** 워핑이 정확히 그 값을 기준으로 삼는데도.
`../CLAUDE.md` 3.1절의 실패 패턴이 다시 나온 것이다 — 확인하지 않은 것을 확정으로 적었다.

## 5. 반입 파이프라인이 바뀐다

```
[3]  AM_EncodeRootBone            pelvis 1.0 / orientation pelvis 축 Y
[3b] SoldierRootFacingModifier    ★ 신규 · Mode = EncodeFromTravelDirection
[4]  AM_Copy_IKFootRoot           ★ [C-46]에서 추가된 단계 (워핑의 전제)
[5]  AM_FootSteps_*
...
```

**순서 제약**: `[3b]`는 `AM_EncodeRootBone` **바로 다음**이어야 한다. `AM_WarpingAlpha`가 루트
회전을 읽어 `enable_warping`을 판정하므로 그보다 먼저 교정돼야 한다.

### 5.1 클립 종류별 적용 여부

| 종류 | 적용 | 이유 |
|---|---|---|
| 이동 루프 · 출발 · 정지 · 피벗 | ✅ `EncodeFromTravelDirection` | 진행 방향이 정의된다 |
| **제자리 회전** | ❌ **걸지 말 것** | 이동 거리가 없어 진행 방향이 무의미하다. 실측 `sd 65.4` — 값이 난수다. 어차피 워핑은 `minRootMotionSpeedThreshold=10`으로 꺼진다 |
| Idle · 정지 포즈 | ❌ | 동일 |

### 5.2 재적용 시 주의

`Apply All Modifiers`를 누르면 `AM_EncodeRootBone`이 먼저 돌아 **루트를 다시 pelvis에서 만든다.**
그 직후 `[3b]`가 고치므로 결과는 같지만, **진단 로그는 계속 −56°로 찍힌다**(교정 직전 값이므로).
교정된 최종 상태를 보려면 **그 모디파이어 줄만 개별 Apply** 해야 한다.

> ⚠ 그래서 모드를 `DiagnoseOnly`로 바꿔 확인한 뒤에는 **반드시 `EncodeFromTravelDirection`으로
> 되돌려야 한다.** 안 그러면 다음 Apply All에서 루트가 다시 망가진다.

## 6. 이것으로 설명되는 것 / 설명되지 않는 것

**설명된다**: 똑바로 걸어도 다리가 틀어지던 것.

**설명되지 않는다** — 아래는 별개이고 **클립 수의 문제**다:

| 증상 | 원인 |
|---|---|
| 정지 중 0.4~0.5초 주기 버벅거림 | `poseReselectHistory = 0.3`(최근 0.3초 포즈 재선택 금지) + `blendTime = 0.5`. DB에 정지 포즈가 없으면 MM이 걷기 클립 안을 계속 뛰어다닌다 |
| 스텝 꼬임 | `maxActiveBlends = 4` — 위상이 다른 클립들이 동시에 평균된다 |
| 정지 중 발 미끄러짐 | 걷기 클립엔 루트모션이 있는데 캐릭터는 안 움직인다 |

**이 파라미터들은 전부 227개짜리 DB를 전제로 Epic이 튜닝한 값이다.** 클립이 1~3개면
오히려 품질을 파괴한다. → **[C-49]**

## 7. 남은 것

| ID | 항목 |
|---|---|
| **C-49** | 소규모 DB에서 `poseReselectHistory` / `blendTime` / `maxActiveBlends` 재튜닝이 필요한가, 아니면 데이터를 채우는 것이 정답인가 |
| C-46 | `AM_Copy_IKFootRoot` 파이프라인 정식 편입 (이미 적용은 함) |
