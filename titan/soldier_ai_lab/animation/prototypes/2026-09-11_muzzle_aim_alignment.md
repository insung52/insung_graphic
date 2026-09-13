# 총구 정렬 — AimOffset 근사 오차의 되먹임 보정

2026-09-11 / **성공** / 총구가 실제로 조준점을 향하도록 **성분별 누적 보정** 루프를 넣었다. 월드 −X를 포함한 전 방향에서 오차 ≈0.

관련 항목: **[C-72]** · **[C-73]** 신설 / 관련 문서: `IMPLEMENTED.md` 2.5c절 · `CLAUDE.md` **P32 · P33 · P34**

> ⚠ **이 문서의 3절(제어 루프)과 5.5절(게이트)은 작성 직후에 두 번 더 갈아엎혔다.**
> **최종 파라미터는 9절**, 실패한 게이트 설계 3종은 **10절**, 잔해 노드 현황은 **11절**이다.
> 본문 3·5.5절은 판단 이력으로 남겨 둔 것이니 **그대로 따라 만들지 말 것** (`CLAUDE.md` 3.3절).
>
> ⚠⚠ **9절도 최종이 아니다 → 13절 정정 (2026-09-12).** 게이트는 **카메라 각속도 단독이 아니며**,
> 게인 앞에 **`AOActive`(= `Enable_AO`) 안티 와인드업 게이트**가 붙었다. 그게 없으면 총을 내린
> 동안 적분기가 **0.42초 만에 ±25 클램프까지 포화**한다.
> 후속 문서: `animation/prototypes/2026-09-12_body_yaw_rate_and_aim_antiwindup.md`

---

## 1. 문제 — AimOffset은 근사다 [A]

`AO_Rifle_ADS`는 **5 × 3 = 15장**짜리 AimOffset이다.

```
Yaw    −180 / −90 /  0 / +90 / +180      5장
Pitch    −90 /   0 / +90                 3장   ★
```

**pitch 표본이 3장뿐이고 사이는 선형 보간이다.** 포즈를 선형으로 섞은 결과가 총구를
정확히 그 각도로 보내 준다는 보장이 없다. 그래서 **총구 방향이 의도한 조준 방향에서
벗어나고, 벗어나는 양이 조준 각도에 따라 달라진다.**

정지·정면 상태에서 실측:

```
pitch 오차   −11.4°
yaw   오차   +10.1°
```

**오차가 각도에 따라 변한다**는 것이 핵심이다 — 상수 오프셋 한 번으로는 못 없앤다(5.1절).
그래서 열린 보정이 아니라 **닫힌 되먹임**이 필요했다.

---

## 2. 계측 장치 [A]

`BP_SoldierCharacter` EventGraph, **`Parent: Tick` 직후**에 붙였다.

```
WeaponMesh → GetSocketTransform("Muzzle", RTS_World) → BreakTransform
                                    │
                                    ├ Location ┐
                                    │          ├→ DrawDebugCoordinateSystem (Scale 50)
                                    └ Rotation ┤
                                               └→ GetForwardVector → MakeRotFromX ─┐
                                                                                   ├→ Delta(Rotator) = ERR
GetController → GetControlRotation ────────────────────────────────────────────────┘
```

- `DrawDebugCoordinateSystem`을 총구에 그려 **총열이 소켓의 +X(빨강) 축을 따라간다**는 것을
  눈으로 확정했다. `GetForwardVector` → `MakeRotFromX` 경로를 쓰는 근거가 이것이다
- `ERR = Delta(Rotator)( GetControlRotation , MakeRotFromX(총구 전방벡터) )`
- 화면 출력 3줄 — `BuildString(Rotator)` / `BuildString(Boolean)`의 **Prefix**로
  `ERR ` · `ACC ` · `GATE `를 달고, 각각 `PrintString`의 **Key**를 `err` · `acc` · `gate`로 분리.
  전체를 **`IsLocallyControlled` Branch로 게이트**한다

> ⚠ 이 장치는 **아직 Tick에 물려 있다.** 출하 전에 제거하거나 디버그 플래그 뒤로 보내야 한다
> → `IMPLEMENTED.md` 6절.

### 2.1 ★ 첫 계측이 틀렸다 — 오차가 roll로 샜다 [A]

처음에는 컨트롤 로테이션을 **총구 소켓의 회전값과 직접** 비교했다. 무기가
`relRot yaw 90`으로 붙어 있어서 **pitch 차이가 roll로 새어 나왔다**:

```
위를 보면   roll  +120
아래를 보면 roll   −45
```

**`Delta(Rotator)`는 진짜 각도차가 아니라 성분별 뺄셈**(`NormalizedDeltaRotator`)이다.
따라서 **두 로테이터가 같은 프레임에 있을 때만** 의미가 있다. yaw 90 회전이 끼어 있으면
프레임이 다르므로 성분이 섞인다.

**해법**: 총구 *방향 벡터*를 뽑아 `MakeRotFromX`로 되돌린다. 이 노드는 **roll = 0인 로테이터**를
만들므로 두 값이 같은 프레임에 놓인다. 바꾼 뒤 **roll이 소수점 6자리까지 0으로 고정**됐고,
그때부터 **`ERR.Roll == 0`이 계측 전체의 유효성 검사**가 됐다.

> 이 교훈이 그대로 5.4절의 실패로 이어진다 — 회전은 **프레임을 맞춰야 뺄 수 있다.**

---

## 3. 제어 루프 [A]  → **정정: 9절 참고**

> 아래는 **1차 형태**다. 게인은 그 뒤 0.25 → **0.05**로, 게이트는 오차 크기 → **카메라 각속도**로 바뀌었다.

`BP_SoldierCharacter`, **매 Tick**. AI 병사에게도 똑같이 돈다 — **화면 출력만** `IsLocallyControlled`로 막는다.

```
누적 = CombineRotators( 이전 누적 , Lerp(Rotator)( (0,0,0) , ERR , 0.25 ) )
       → BreakRotator → NormalizeAxis(Pitch) / NormalizeAxis(Yaw)
       → Clamp(±25°) 각각
       → MakeRotator( Roll = 0 , Pitch , Yaw )
       → 캐릭터의 AimCorrection 에 저장
       → Mesh → GetAnimInstance → Cast to SoldierCharacter_ABP
       → ABP 의 AimCorrection 에 전달
```

| 값 | 이유 |
|---|---|
| ~~**게인 0.25**~~ → **0.05** | `Lerp(Rotator)`의 Alpha. 0.5는 눈에 보이는 링잉(울림)을 일으켰다. **0.25도 과도구간을 학습해 오버슛했다 → 9.2절** |
| **Clamp ±25°** | 안티 와인드업. 보정이 무한히 쌓여 자세가 무너지는 것을 막는다 |
| **Roll = 0 고정** | 2.1절의 프레임 규약을 누적값에도 유지한다 |

> `Lerp(Rotator)((0,0,0), ERR, 0.25)`는 **스케일링을 보간으로 대신한 것**이다.
> 블루프린트 팔레트에 곱셈 노드가 없다 → **P33**.

---

## 4. 주입 지점 — `SoldierCharacter_ABP` 의 `Get_AOValue` [A]

```
AimingRotation ──────────────┐
                             ├─→ Delta(Rotator) ─→ (Pitch, Yaw) ─→ Lerp(Vector) ─→ AimOffset 입력
RootRotation ─┐              │                                         ↑
              ├→ Delta(Rotator) ┘                          Alpha = GetCurveValue("Disable_AO")
AimCorrection ┘
```

실제 배선(MCP 실측): `Delta_18(A = RootTransform.Rotation, B = AimCorrection)` →
`Delta_2(A = AimingRotation, B = Delta_18)`.

**왜 이게 덧셈이 되는가**: `Delta(A, B)`는 `NormalizedDeltaRotator`, 즉 **성분별 뺄셈**이다. 그러므로

```
Delta( Aim , Delta( Root , Corr ) )  ==  (Aim − Root) + Corr      ← 성분별로
```

원래 식 `(Aim − Root)`를 건드리지 않고 **보정만 성분별로 더한 것**이 된다. 회전 합성이
아니므로 프레임이 얼마나 벌어져 있든 축이 섞이지 않는다.

### 4.1 ★ `AimingRotation`을 편향해도 안전한 이유 — 독자가 하나뿐이다 [A]

ABP 전체(그래프 약 100개 · **노드 1,200개**)를 MCP로 훑어 `AimingRotation`이라는 이름의
핀을 전수 검색한 결과:

```
연결된 AimingRotation 핀  =  1개
  Get_AOValue . BreakSCharacterPropertiesforAnimation → Delta(Rotator)
```

**`AimingRotation`은 `Get_AOValue` 한 곳에서만 읽힌다.** 몸통 facing(`Update_TargetRotation`,
`Get_DesiredFacing`)도 궤적(`Update_Trajectory`)도 이 값을 쓰지 않는다.
그래서 여기에 편향을 넣어도 **조준 오프셋 외에는 아무것도 흔들리지 않는다.**

> 이것이 5.2절(원천에서 편향)을 포기해도 괜찮았던 이유다 — 원천을 못 고쳐도
> **소비 지점이 하나면 결과가 같다.**

---

## 5. ★ 실패한 접근 네 가지 + 증상 패치 — 이 문서에서 가장 값진 부분

### 5.1 상수 오프셋만 넣는다 — 기각 [A]

1절의 계측이 **오차가 조준 각도에 따라 변한다**는 것을 보여줘서 시작하기도 전에 기각됐다.
(계측을 먼저 넣은 덕에 버린 시간이 0이었다 — **P10**)

### 5.2 캐릭터에서 `Get_PropertiesForAnimation`을 오버라이드해 원천에서 편향한다 — 포기 [A]

`AimingRotation`이 만들어지는 자리에서 고치는 것이 가장 깔끔해 보였다. 그러나

- 블루프린트 팔레트에 **`Parent:` 호출 노드가 나오지 않는다**
- 부모 함수는 **커다란 구조체를 통째로 조립**한다. 오버라이드하려면 그 조립을 **전부 다시 구현**해야 하고
  GASP 로직을 복제하게 된다

**부모를 부를 수 없는 오버라이드는 오버라이드가 아니라 재작성이다.** 포기.

### 5.3 Delta의 **출력**에 보정을 더한다 — 불가능 [A]

두 가지 이유로 막혔다:

1. `Delta(Rotator)` 노드의 출력이 **Roll/Pitch/Yaw 서브핀으로 분해**돼 있어 로테이터를 통으로 받을 데가 없다
2. **float 덧셈 노드가 없다** — 팔레트에도, `create_node`로도 만들 수 없다 → **P33**

### 5.4 ★★ `CombineRotators`로 Delta **앞에서** `AimingRotation`에 합성한다 — 앞에서는 되고 뒤에서는 깨졌다 [A]

**가장 오래 붙잡았고 가장 많이 배운 실패다.**

```
CombineRotators( AimingRotation , AimCorrection ) → Delta( ... , RootRotation ) → AimOffset
```

**정면에서는 완벽했다.** 오차가 **0.009°** 까지 수렴했다. 그래서 맞는 줄 알았다.

**월드 −X 쪽을 보자 캐릭터가 떨고 뒤집혔다.**

원인: **회전 합성은 성분별 덧셈이 아니다.** 두 프레임이 가까울 때만 근사적으로 같다.

```
정면      aim ≈ root     →  프레임이 가깝다  →  덧셈처럼 동작  →  수렴
월드 −X   aim ≠ root     →  프레임이 벌어짐  →  pitch 보정이 yaw로 새고 부호가 뒤집힘
                         →  되먹임이 음성이 아니라 **양성**이 된다  →  발산
```

**결정적 단서는 증상의 모양이었다** — 멀쩡한 구간이 **월드 +X 축을 중심으로 한 약 150° 원뿔**이었다.
원뿔이 **캐릭터가 아니라 월드에 고정**돼 있다는 것이 "합성이 월드 공간에서 일어나고 있다"는 신호다.
→ **P32**

해법이 4절의 성분별 항등식이다. **합성(Combine)에서 성분별 뺄셈의 중첩(Delta of Delta)으로 바꾼 것**이
이 작업의 전부라고 해도 된다.

### 5.5 ⚠ 5.4 위에 덧댄 증상 패치 3종 — 전부 잘못된 접근이었다 [A]  → **정정: 10·11절 참고**

원인을 모르는 채로 증상만 깎으려 한 것이다. **사용자가 이 점을 정확히 지적했고 그게 맞았다.**

| # | 패치 | 결과 |
|---|---|---|
| 1 | 누적에 **±25° 클램프** | 발산 폭만 줄었다. 떨림은 남음 (이 하나는 안티 와인드업으로 살아남아 지금도 있다) |
| 2 | **도달 가능성 게이트** — `\|Delta(control, actorRotation).Yaw\| ≤ 90` 일 때만 보정 | **한 번도 발동하지 않았다** |
| 3 | **히스테리시스** 15° / 30° | 증상이 옅어졌을 뿐 |

**게이트가 왜 안 걸렸나**: 조준 중에는 **몸이 카메라를 따라 돈다.** 그래서
`control − actorRotation`의 yaw는 **구조적으로 늘 작다.** 판정하려던 상황이 애초에
그 식으로는 표현되지 않았다.

**이걸 1회 PIE로 밝혀낸 것이 `GATE` 화면 출력이다.** 불리언을 라벨과 함께 찍자
"항상 true"가 즉시 보였다. → **P34**

> 현재 그래프에는 이 세 패치의 잔해가 **연결이 끊긴 채** 남아 있다 —
> ~~`SelectRotator`(출력 미연결)~~ · `Lerp(Rotator) Alpha 0.15`(감쇠) ·
> `Max(|ERR.Pitch|, |ERR.Yaw|)` → `InRange(0 .. SelectFloat(30/15))` ·
> `Delta(control, actorRotation)`(출력 미연결) · 변수 `AimLoopActive`.
> **평가 경로 밖이라 동작에 영향이 없다.** 정리는 계측 장치 제거와 함께 한다.
>
> → **정정(11절)**: `SelectRotator`는 그 뒤 **속도 게이트의 선택기로 부활**했고(= 살아 있는 경로다),
> 오차 크기 체인은 **완전히 죽은 게 아니라 화면의 `GATE` 표시와 `AimLoopActive` 변수만 계속 구동**한다.
> 즉 **화면에 찍히는 `GATE`는 지금 실제로 쓰이는 게이트가 아니다.**

---

## 6. 결과 [A]

```
정면        오차 ≈ 0
월드 −X     오차 ≈ 0        ← 5.4가 깨지던 방향
전 방향     수렴, 링잉 없음
ERR.Roll    0 (소수점 6자리)  ← 계측 유효성 유지
```

**판정: 성공.** AI 병사에게도 같은 루프가 돌므로 조준 정확도는 플레이어와 동일하다.

---

## 7. 남은 것

| ID | 항목 |
|---|---|
| ~~**C-72**~~ | ~~`OffsetRootBone`의 **`maxRotationError`**~~ **✅ 해결 (2026-09-11)** — `−1` → **90**. PIE에서 뒤집힘이 완전히 사라졌다. → `animation/prototypes/2026-09-11_sharp_turn_while_aiming.md` |
| **C-73** | `AO_Rifle_ADS`의 pitch 표본이 3장뿐이라 중간 각도에 보간 오차가 남는다 |
| — | **계측 장치를 Tick에서 걷어내거나 디버그 플래그 뒤로 보낼 것** (2절 경고) |
| — | 5.5절 잔해 노드 정리 |

`sprintSpeeds` 건은 이미 **[C-60]** 으로 등록돼 있어 새로 만들지 않았다.

---

## 8. 이것이 바꾸는 것

| 문서 | 절 | 어떻게 |
|---|---|---|
| `CLAUDE.md` | 5절 | **P32**(성분별 회전 보정) · **P33**(산술 노드 부재) · **P34**(계측 라벨) 추가 |
| `IMPLEMENTED.md` | 2.5c 신설 · 3절 · 6절 | 보정 루프 · 새 변수 3개 · 계측 장치 잔존 경고 |
| `OPEN_ITEMS.md` | C절 | **C-72** · **C-73** 등록 |

---

## 9. ★★ 최종 형태 (2026-09-11 3차) — 여기만 보면 된다 [A]

> 3절(1차)·5.5절(2차 게이트)을 **대체한다.** 아래는 2026-09-11에 MCP로 **살아 있는 그래프를
> 배선 단위로 추적해 확인한 값**이다(`find_nodes` + `get_node_infos`, `read_graph_dsl` 아님 — 6.1f).

### 9.1 배선 — `BP_SoldierCharacter` EventGraph, `Parent: Tick` 직후

```
GetController → GetControlRotation ──┬──────────────────────────────┐
                                     │                              │
WeaponMesh.GetSocketTransform         │                              │
   ("Muzzle", RTS_World) → Break      │                              │
        → GetForwardVector            │                              │
        → MakeRotFromX ───────────────┴→ Delta(Rotator) = ERR        │
                                                │                    │
                        Lerp(Rotator)((0,0,0), ERR, Alpha = 0.05)    │
                                                │                    │
        AimCorrection(이전) ──→ CombineRotators(A=이전, B=위) ─┐      │
                                                             │      │
  ┌──────────── 게이트 (카메라 각속도) ──────────────┐        │      │
  │ Delta(Rotator)(GetControlRotation, PrevAimRot)  │        │      │
  │   → Break → |Pitch| , |Yaw| → Max               │        │      │
  │   → InRange(0 .. 2.0)  = GATE                   │        │      │
  └─────────────────────┬───────────────────────────┘        │      │
                        │                                    │      │
        SelectRotator( A = 누적갱신 , B = AimCorrection(이전) , bPickA = GATE )
                        │                                            │
        → BreakRotator → NormalizeAxis(Pitch/Yaw) → Clamp(±25°) 각각 │
        → MakeRotator(Roll = 0)                                      │
        → SetAimCorrection (캐릭터)                                  │
        → Mesh.GetAnimInstance → Cast SoldierCharacter_ABP           │
        → SetAimCorrection (ABP)                                     │
                                                                     │
        ... Branch(IsLocallyControlled) → PrintString ×3 → SetPrevAimRot ←┘
```

### 9.2 파라미터 한 곳 정리 [A]

| 항목 | 값 | 이유 |
|---|---|---|
| **오차** `ERR` | `Delta(Rotator)(GetControlRotation, MakeRotFromX(총구 전방벡터))` | 2.1절 — `MakeRotFromX`가 roll을 0으로 만들어 두 로테이터의 프레임을 맞춘다 |
| **게이트** | `max(\|Δ카메라.Pitch\|, \|Δ카메라.Yaw\|) ≤ **2.0°/프레임**` | **입력(카메라 각속도)** 으로 건다. 오차로 걸면 자기 잠금 → 10.2절 |
| 게이트 상태 변수 | **`PrevAimRot` (Rotator)** — 틱 끝에서 `GetControlRotation`을 대입 | 각속도를 만들려면 직전 프레임 값이 필요하다 |
| 게이트 거짓일 때 | **이전 누적을 그대로 유지**(감쇠 아님) | 0으로 감쇠시키면 카메라를 움직이는 동안 보정이 사라져 총구가 다시 벌어진다 |
| **게인** | `Lerp(Rotator)`의 Alpha = **0.05** (이력: 0.5 → 0.25 → 0.05) | **게인이 곧 학습 시간상수** → 9.3절 |
| **Clamp** | ±25° (Pitch/Yaw 각각, `NormalizeAxis` 뒤) | 안티 와인드업 |
| **Roll** | `MakeRotator(Roll = 0)` 고정 | 2.1절의 프레임 규약을 누적값에도 유지 |
| **주입** | `SoldierCharacter_ABP.Get_AOValue`: `Delta(AimingRotation, Delta(RootRotation, AimCorrection))` | **4절에서 바뀌지 않았다** — MCP 재확인 |

### 9.3 ★ 게인은 필터가 아니라 **학습 시간상수**다 [A]

게이트를 각속도로 바꾸자 "마우스를 멈추는 순간" 게이트가 열렸다. 그런데 그 순간 **몸은 아직
정착 중**이다 — `OffsetRootBone.rotationHalfLife = 0.1`이라 메시가 캡슐을 따라잡는 데
**약 0.3초**가 걸린다(실측 프로퍼티 기반 추정 [B]). 게인 0.25에서는 적분기가 그 **추적 지연**을
약 4프레임 만에 통째로 삼켜 버리고, 몸이 따라잡은 뒤에는 그만큼 **오버슛**한다.

> 사용자가 보고한 증상: **"왼쪽으로 빠르게 돌다 마우스를 멈추면 총이 더 왼쪽 아래로 쳐졌다가
> 되돌아온다."** 회전 관성처럼 보였지만 **적분 오버슛**이었다.

해결은 게이트를 하나 더 다는 것이 **아니었다**:

```
게인 0.05  ≈  1/0.05 = 20프레임  ≈  0.33초  (60fps 기준)
                                  ↑
           OffsetRootBone 의 정착 시간과 의도적으로 맞춘 값
```

**학습 시간상수를 학습 대상이 정착하는 시간보다 느리게 잡으면 과도구간은 그냥 걸러진다.**
게이트는 "언제 배울지"를 정하고, 게인은 "얼마나 빨리 배울지"를 정한다. **둘 다 필요하다.**
→ `CLAUDE.md` **P35**

### 9.4 결과 [A]

- 전 방향 오차 ≈ 0 (6절 그대로 유지)
- 급선회 후 정지 시 **총구가 쳐졌다 돌아오는 오버슛이 사라졌다** (PIE 확인)

---

## 10. ★★ 실패한 게이트 설계 3종 — 이 문서에서 두 번째로 값진 부분 [A]

**게이트를 세 번 잘못 걸었다. 셋 다 원인이 다르다.**

### 10.1 도달 가능성 게이트 — `|Delta(controlRotation, GetActorRotation).Yaw| ≤ 90`

**한 번도 발동하지 않았다.** 조준 모드에서는 `bUseControllerDesiredRotation = true`라
**몸이 카메라를 따라 돈다.** 그래서 `control − actorRotation`의 yaw는 **구조적으로 늘 작다** —
판정하려던 상황이 애초에 그 식으로 표현되지 않았다.

**밝혀낸 수단**: `GATE ` 라벨을 붙여 화면에 불리언을 찍은 것. PIE 1회로 "항상 true"가 보였다 → **P34**.

### 10.2 ★★ 오차 크기 게이트 + 히스테리시스 (15/30 → 20/25) — **자기 잠금**

```
게이트 =  max(|ERR.Pitch|, |ERR.Yaw|)  ≤  SelectFloat(25 / 20, AimLoopActive)
```

**구조적으로 틀렸다.**

```
누적값이 틀어진다
   → 오차가 커진다
      → 게이트가 닫힌다
         → 누적값을 고칠 수 없다
            → 오차가 계속 크다  ──┐
   ↑                              │
   └──────────────────────────────┘   자기 잠금(self-locking)
```

사용자는 **마우스를 마구 흔들어 오차가 우연히 임계 아래로 떨어지기를 기다리는 것** 말고는
빠져나올 방법이 없었다. 히스테리시스(15/30 → 20/25)를 조여도 잠기는 방향만 바뀔 뿐이다.

> **일반화**: **제어기의 게이트를 그 제어기 자신의 출력(=오차)으로 걸었다.**
> 게이트는 **입력**으로 걸어야 한다. → `CLAUDE.md` **P35**

### 10.3 속도 게이트 + 게인 0.25 — **과도구간을 학습했다**

게이트 자체는 옳았다(입력으로 걸었다). 그런데 **게이트가 열리는 시점**과 **몸이 정착하는 시점**이
달랐다. 마우스를 멈추는 순간 게이트가 열리지만 메시는 아직 캡슐을 쫓는 중이고, 그 **추적 지연**이
적분기에는 그냥 "오차"로 보인다.

**이번엔 게이트를 고치는 것이 답이 아니었다** — 9.3절의 게인 재해석이 답이었다.
세 번째 실패가 중요한 이유가 이것이다: **게이트를 옳게 걸어도, 게인이 빠르면 여전히 틀린다.**

---

## 11. 잔해 노드 현황 (2026-09-11 실측) [A]

5.5절의 서술을 **정정한다** — 잔해의 구성이 달라졌다.

| 노드 | 지금 상태 |
|---|---|
| `SelectRotator` | ❌ 잔해 아님 — **속도 게이트의 선택기로 살아 있다**(9.1절) |
| `AimLoopActive` (Boolean) | ⚠ **평가 경로 밖이지만 계속 쓰인다** — `SelectFloat(25/20)`의 `bPickA`로 들어가고, `InRange` 결과가 다시 이 변수에 써진다. 조준 보정에는 **영향 없음** |
| `Max(\|ERR.Pitch\|,\|ERR.Yaw\|)` → `InRange(0 .. SelectFloat(25/20))` | ⚠ **화면의 `GATE` 표시와 `AimLoopActive`만 구동한다.** 보정 경로와 무관 |
| `Delta(controlRotation, GetActorRotation)` | ✅ 출력 미연결 — 완전한 죽은 노드 (10.1절 잔해) |
| `Lerp(Rotator)(A = AimCorrection, Alpha 0.15)` | ✅ 출력 미연결 — 감쇠 패치 잔해 |
| `GetControlRotation` 한 개 · `ToString(Rotator)` 두 개 | ✅ 출력 미연결 |

> ⚠⚠ **화면에 찍히는 `GATE`는 지금 실제로 쓰이는 게이트가 아니다.**
> 죽은 오차 크기 게이트를 표시하고 있다. 이 상태로 튜닝하면 **또 속는다** —
> 계측을 정리할 때 `GATE`를 속도 게이트(`InRange(0..2.0)`)에 다시 물리거나 아예 뗄 것 → **[W6]**

### 11.1 ⚠ `SetPrevAimRot`이 `IsLocallyControlled` 분기 **안**에 있다 [A]

틱의 실행 순서는 이렇다:

```
EventTick → Parent:Tick → DrawDebugCoordinateSystem
   → SetAimCorrection(캐릭터) → Cast → SetAimCorrection(ABP)
   → Branch(IsLocallyControlled)
        └ true → PrintString(err) → PrintString(acc) → PrintString(gate)
                 → SetAimLoopActive → **SetPrevAimRot**
```

`AimCorrection` 자체는 분기 **앞**에서 갱신되므로 AI 병사도 보정을 받는다. 그러나
**`PrevAimRot`은 분기 뒤라 AI 병사에게는 영원히 초기값으로 남는다.** 그러면
`Delta(GetControlRotation, PrevAimRot)`이 늘 크게 나와 **게이트가 계속 닫히고, AI의 누적
보정은 사실상 얼어붙는다** [B] — PIE로 확인하지 않았다. → **[C-75]**

`DrawDebugCoordinateSystem`도 분기 앞이라 **모든 병사에게 그려진다**는 점도 같이 정리 대상이다.

---

## 12. 이것이 바꾸는 것 (추가분, 2026-09-11 3차)

| 문서 | 절 | 어떻게 |
|---|---|---|
| `CLAUDE.md` | 5절 | **P35**(게이트는 입력으로, 게인은 시간상수) · **P36**(증상 패치 대신 근본 원인) 추가 |
| `IMPLEMENTED.md` | 2.5c · 3절 · 6절 | 속도 게이트 · 게인 0.05 · 유지(비감쇠) · `PrevAimRot` · 잔해 현황 |
| `OPEN_ITEMS.md` | C · W절 | **C-72 해결** / **C-75**(AI `PrevAimRot`) · **W6**(계측·잔해 정리) 신설 |
| 형제 문서 | — | `animation/prototypes/2026-09-11_sharp_turn_while_aiming.md` — `maxRotationError` · `Enable_AO` |

---

## 13. 정정 (2026-09-12) — 게이트가 하나 더 붙었다 [A]

9절을 **대체하지는 않고 보강한다.** 9.1절의 배선은 그대로이고, **게인이 상수 0.05가 아니게 됐다.**

### 13.1 빠져 있던 조건 — 적분기가 **열린 루프로 돌 수 있다**

9절의 게이트는 **카메라 각속도 하나뿐**이었다. 그런데 `Enable_AO`가 false면
(= 급선회나 우클릭 해제로 총이 내려가면) AO 갈래가 블렌드 아웃되어
**`AimCorrection`이 포즈에 아무 영향도 주지 않는다.** 그동안에도 적분은 계속 돌았다.

```
닫힌 루프:  보정 → 총구가 움직임 → ERR 감소 → 수렴
열린 루프:  보정 → (아무 일도 안 일어남) → ERR 그대로 → acc 가 **선형으로** 증가

ERR ≈ 20° · 60fps · 게인 0.05   →   1°/프레임   →   ±25° 클램프까지 25프레임 = **0.42초**
```

**증상**: 총을 다시 들면 총구가 한참 위로 솟구쳤다가 1초쯤에 걸쳐 내려온다.
화면의 `AimCorr`는 총이 내려가 있는 동안 **언제나 25.000**이었다.

### 13.2 최종 게인 체인

```
AimGain = SelectFloat( 0.0 ,
                       Lerp( 0.05 , 0.0 , WeaponLowered ) ,
                       bPickA = NOT AOActive )

ABP        : 새 bool `AOActive` ← `Update_Logic` 에서 `Update_States` **직후** `Enable_AO()`
캐릭터 Tick: ABP 의 `AOActive` 를 **캐시**(1프레임 지연).
             ⚠ 인라인으로 못 읽는다 — `CastToSoldierCharacter_ABP` 가 Tick 안에서
                `SetAimCorrection` **뒤에** 실행되어 순수 체인으로는 null 이다
`WeaponLowered` : `AOActive` 가 false 면 목표를 **1.0 으로 강제** (회복을 0.5초로 늘린다)
```

**게인 회복 0.5초 > AO 포즈 블렌드 0.375초** — 포즈가 먼저 서고 게인이 나중에 붙는다.
반대면 블렌드 구간에서 다시 포화한다(`CLAUDE.md` **P38**).

### 13.3 ★ 여기서도 게이트를 **세 번** 잘못 걸었다

10절의 3종과 **별개의** 3종이다. 전문은
`animation/prototypes/2026-09-12_body_yaw_rate_and_aim_antiwindup.md` 10절.

| # | 게이트 | 왜 틀렸나 |
|---|---|---|
| 1 | `bOrientRotationToMovement`(우클릭 여부) | **의도**에는 즉시 열리는데 포즈는 0.375초 블렌드를 거친다. 그 창에서 `0.05 × 20° × 22프레임 ≈ 22°` — 사실상 포화 그대로 |
| 2 | `WeaponLowered` / `BodyErr`(캡슐 기준) | 포즈 게이트는 `RootTransform`(= **OffsetRootBone 의 지연된 메시 회전**) 위에, 게인 게이트는 캡슐(**지연 없음**) 위에 있었다. **시간상수가 달라 어긋나는 창**이 곧 와인드업 구간이다. 증거: `BodyErr 0.000 / WpnLow 0.000 / AimGain 0.050` 인데 `AimErr P 73 Y 99` |
| 3 | ✅ `Enable_AO` 자체 | **포즈를 실제로 고르는 함수.** 다른 어떤 신호도 어딘가에서 `Enable_AO` 와 어긋난다 → **P37** |

> ⚠ 처음에 `Enable_AO` 게이트를 **"자기 참조"라며 기각했던 판단은 틀렸다.**
> `Get_AOValue` 안에 `AimCorrection` 이 덧셈으로 들어 있는 것은 맞지만,
> **제어기 자신의 오차로 거는 것**(= 10.2절의 자기 잠금)과
> **액추에이터가 결합돼 있는가로 거는 것**(= 포화 인지형 안티 와인드업)은 다른 것이다.
> 후자는 액추에이터의 결합 여부가 제어 출력에 부분적으로 의존해도 성립한다.

### 13.4 ⚠ 클램프에는 **두 번째 안정 평형점**이 있다 → [C-76]

`AimCorr ±25`로는 `AimErr 99°`를 줄일 수 없다. 그 상태에 한 번 들어가면
**시간이 지나도 풀리지 않고** 외부 입력(마우스 움직임)이 흔들어야 빠져나온다.
간헐적으로 관측됐고 **방아쇠는 미규명이다** → **[C-76]** · `CLAUDE.md` **P40**.

### 13.5 추가로 확인된 것

- **`AimCorrection` 의 소비자는 `Get_AOValue` 하나뿐**이다 — ABP **최상위 그래프 69개 전수 조사** [A].
  `Disable_AO` 커브도 마찬가지. (4.1절의 `AimingRotation` 전수 검색과 같은 결론)
- ⚠ `Update_TargetRotation` · `OnUpdate_TransitionToLocomotion` **두 그래프는 DSL 로 읽히지 않는다** —
  `RInterpTo` 의 데이터 흐름 순환에서 리더가 죽는다. `find_nodes` + `get_node_infos` 로 확인했고 둘 다 깨끗하다
- **`Enable_AO` 의 블루프린트 호출자는 0개**다. 유일한 소비자는 `BlendListByBool_0` 의 바인딩 [A]
