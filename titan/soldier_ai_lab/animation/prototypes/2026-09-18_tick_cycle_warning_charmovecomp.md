# `LogTick … would form a cycle` 경고(초당 600줄) — `BP_SoldierCharacter` 틱 선행 조건이 엔진의 `bTickBeforeOwner` 와 순환하고 있었다 + 총구 `DrawDebugCoordinateSystem` 제거

2026-09-18 / 부분(경고 제거 확인 · **실제 틱 순서는 서명 시험 대기 [C-146]**) / 09-12 에 넣은 `CharacterMovement.AddTickPrerequisiteActor(self)` 가 엔진이 CMC 에 기본으로 거는 반대 방향 엣지(액터가 CMC 뒤에)와 **순환**을 만들어, 엔진이 우리 엣지를 **매 프레임 버리고** 경고를 찍고 있었다 — 즉 09-12 문서 5.3절이 보장한다고 적은 순서(우리 Tick → CMC)는 **실제로는 반대**였을 가능성이 크다. 수정 = CMC `Tick Before Owner = false`. 덤으로 모든 소총 총구에 붙어 있던 xyz 디버그 축을 뗐다.

관련 항목: **[C-146]**(신설) / 관련 문서: `animation/prototypes/2026-09-12_body_yaw_rate_and_aim_antiwindup.md` **5.3·5.4절**, `IMPLEMENTED.md` **2.5d-1** — 둘 다 이 문서로 정정.

> 신뢰도: **[A]** 엔진 소스·BP 에셋 문자열로 확인 · **[B]** 잠정 · **[C]** 미측정. ⚠ 09-12 문서는 5.3절 결과로 "`YawRate_Up = 20` 에서 `BodyErr` 최대 75" 를 [A] 로 기록했다 — 그 관측과 아래 분석이 **양립하지 않는다**(4절). 어느 쪽이 맞는지는 서명 시험으로만 판정된다.

---

## 1. 무엇을 확인하려 했나

`L_SoldierScenario` PIE 중 `LogTick: Warning: While processing prerequisites for CharMoveComp, could not use … because it would form a cycle.` 가 **병사 35명 × 매 프레임 ≈ 초당 600줄** 찍혔다. 판정 기준: (a) 경고 0줄, (b) **09-12 문서가 정한 순서(`AC_PreCMCTick` → 우리 Tick → CMC)가 실제로 성립** — 후자는 그 문서 5.1·5.2절의 서명 시험(`YawRate_Up = 20` → `BodyErr ≠ 0.000`)으로만 확인된다.

## 2. 원인 [A]

### 2.1 두 엣지가 서로 반대다
| 엣지 | 누가 | 어디 |
|---|---|---|
| **CMC → 액터**(액터 틱은 CMC 뒤) | 엔진. `UMovementComponent::bTickBeforeOwner` 기본 **true**(`MovementComponent.cpp:35`, `MovementComponent.h:144`) → `RegisterComponentTickFunctions` 에서 `Owner->PrimaryActorTick.AddPrerequisite(this, PrimaryComponentTick)`(`MovementComponent.cpp:186-188`) | 컴포넌트 등록 시 1회 |
| **액터 → CMC**(CMC 는 액터 뒤) | 우리. `BP_SoldierCharacter` BeginPlay `CharacterMovement.AddTickPrerequisiteActor(self)`(09-12 5.3절) | BeginPlay 1회 |

둘이 합쳐지면 A→B→A. `FTickTaskManager` 는 선행 조건을 풀 때 순환을 감지하면 **그 엣지를 쓰지 않고** 경고한다 — `TickTaskManager.cpp:2658` (`"While processing prerequisites for %ls, could not use %ls because it would form a cycle."`, 2747 에 같은 문구의 다른 경로). 이 처리는 **프레임마다** 반복되므로 경고도 프레임마다.

### 2.2 그러면 실제 순서는?
어느 엣지가 버려지는가는 처리 순서에 달렸다 [B]. 경고 문구의 주어가 **`CharMoveComp`** 이므로 "CMC 의 선행 조건(= 우리가 건 `self`)을 쓸 수 없다" — 즉 **우리 엣지가 버려지고 엔진 엣지(CMC → 액터)가 살아남았다**. 그러면 프레임 순서는

```
AC_PreCMCTick (−1 쓰기)  →  CMC (−1 로 캡슐 회전 = 즉시)  →  우리 Tick (UpdateBodyYawRate 가 ω 쓰기, 다음 프레임 PreCMCTick 이 다시 −1)
```

09-12 문서가 "보장된다"고 적은 순서의 **정반대**이고, 그 문서 5절이 "조용히 무시된다"고 진단한 바로 그 상태다.

### 2.3 09-12 문서와의 모순
09-12 5.3절은 이 배선 직후 `YawRate_Up = 20` 에서 `BodyErr` 가 **최대 75** 까지 올라갔다고 기록했다(유한 각속도가 먹는다는 서명). 2.2 가 맞다면 그 값이 나올 수 없다. 가능한 설명 [B]:
- 순환 처리 순서가 상황(등록 순서·`AC_PreCMCTick` 엣지 존재 여부)에 따라 달라 그때는 반대 엣지가 버려졌다.
- 또는 `BodyErr` 75 가 다른 경로(카메라/컨트롤러 회전 지연)에서 나온 것이었다.
어느 쪽이든 **"보장"은 아니었다.** 판정은 4절.

## 3. 수정 [A]

BP 노드는 **그대로 두고**(우리 엣지 = 의도), 엔진 엣지를 껐다: `BP_SoldierCharacter` 의 `CharMoveComp` **`Tick Before Owner = false`** (`bTickBeforeOwner`). 이제 엣지는 우리 것 하나뿐이라 순환이 없다. 09-12 5.4절의 부모 배선(`CMC.AddTickPrerequisiteComponent(AC_PreCMCTick)`)은 방향이 같아 영향 없음.

적용 범위: `BP_SoldierCharacter` CDO(디스크 `Blueprints/BP_SoldierCharacter.uasset` 09-18 08:47 저장본에 `bTickBeforeOwner` 문자열 있음 [A]) + 자식 `BP_Soldier_Friendly`/`BP_Soldier_Hostile` CDO + `L_SoldierScenario` 배치 인스턴스에 MCP `set_properties` 로 같은 값. ⚠ 자식 BP(09-17 14:15 저장) · 레벨(09-17 16:47 저장) 파일에는 그 문자열이 **없다** — 부모와 같은 값이면 오버라이드로 직렬화되지 않으므로 정상일 수 있으나, 인스턴스가 **true 를 오버라이드하고 있었다면** 레벨 재저장이 필요하다(`soldier_ai_lab/CLAUDE.md` 6.1 "레벨 인스턴스 오버라이드" 항목) [B]. PIE 로 경고 0줄은 확인했다 [A].

## 4. 판정 — **부분**. 서명 시험이 남았다 → [C-146]

09-12 문서 5.1·5.2절 그대로:
```
BP_SoldierCharacter 인스턴스 YawRate_Up = 20  (정상 90)
PIE → 조준 상태에서 90° 돌기
통과: BodyErr 가 0 이 아닌 값(수십 °)으로 올라간다 · 90° 에 ~4.5 s
실패: BodyErr == 0.000 고정 → 순서가 여전히 CMC → 우리 Tick. 그때는 AddTickPrerequisiteActor 대신
      CMC.PrimaryComponentTick.AddPrerequisite(self, PrimaryActorTick) 방향/대상을 get_node_infos 로 재확인(5.4절 표)
```
경고가 없어진 것은 순환이 없어졌다는 뜻이지 **순서가 의도대로라는 뜻이 아니다** — P29·P39(값의 서명을 본다).

## 5. 이것이 바꾸는 것

| 문서 | 절 | 어떻게 바뀌나 |
|---|---|---|
| `animation/prototypes/2026-09-12_body_yaw_rate_and_aim_antiwindup.md` | 5.3 | "보장되는 순서"는 엔진 기본 엣지와 순환해 **보장이 아니었다**. 정정 절 추가(이 문서 링크) |
| `IMPLEMENTED.md` | 2.5d-1 | 틱 선행 조건 2줄 + **CMC `bTickBeforeOwner=false`** 가 전제. 서명 시험 [C-146] |
| `OPEN_ITEMS.md` | C | [C-146] 등록 |

- [x] 원 문서에 정정 절
- [x] `OPEN_ITEMS.md` 등록
- [x] `CURRENT_STATE.md`

## 6. 덤 — 총구의 xyz 디버그 축 제거 [A]

`BP_SoldierCharacter` Event Tick 에 `Parent: Tick` 직후 **무조건** 실행되는 `DrawDebugCoordinateSystem`(Muzzle 소켓 위치)이 있었다 — 모든 병사 소총 끝에 붙어 있던 빨강/초록/파랑 막대. 이 세션이 **exec 핀만 끊었다**(노드는 그래프에 남김, `Parent: Tick` → 다음 노드 직결). 문자열 `DrawDebugCoordinateSystem` 은 에셋에 그대로 있다(노드 잔존) [A]. 이유가 있어 넣었던 것이라면(09-11 총구 정렬 세션의 계측?) 다시 잇기만 하면 된다 — 아무 문서에도 이 노드가 적혀 있지 않아 출처는 모른다 [C].

## 7. 막힌 것 / 다음에 확인할 것

- **[C-146]** 4절 서명 시험.
- 같은 패턴이 다른 BP 에도 있는가 — `AddTickPrerequisiteActor(self)` 를 CMC 에 거는 곳(GASP 부모 `CBP_SandboxCharacter` 포함) grep. 이번엔 `BP_SoldierCharacter` 만 봤다.
- `LogTick` 경고가 없어졌으니 로그 노이즈가 사라져 `[Squad]`/`[Engage]` 판독이 쉬워졌다 — MCP `LogsToolset` 필터 없이도 읽힌다.
