# AI↔몸 배선과 시험 무대 — 블루프린트·에셋·레벨, 그리고 도구가 가르친 것

2026-09-13 / **동작 확인** / C++ 판단층이 실제로 GASP 몸을 운전한다. 축 다리(bridge) 5개,
컴포넌트 7개, Tick 접합, 조준 상태의 진짜 출처, 관전 폰과 시가지 레벨.
그리고 **이 세션이 도구에서 배운 것 9건**.

관련: **[C-43]** · **[C-75]** · **[W3]** · **[W19]** / 원칙: **P53~P61 · P71**
관련 문서: `ai/2026-09-13_perception_stack.md` · `ai/2026-09-13_engagement_and_cover.md` ·
`IMPLEMENTED.md` 2.4·2.5b·2.5e·2.5f절 · `ai/prototypes/2026-09-04_p0-1_ai_drives_mm.md`

---

## 1. ★ AI→애니메이션 다리 — 축마다 **같은 모양** [A]

`BP_SoldierCharacter` 에 변수 5개. 전부 **인스턴스 편집 가능**, 카테고리 `SoldierLab|AI Bridge`.

```
AIPoseDriven         (bool)   이 몸을 AI가 모는가
AITargetLean         (float)
AITargetStance       (float)
AITargetBlindFireH   (float)
AITargetBlindFireV   (float)
```

연속 축은 **전부 아래 한 모양**으로 배선됐다.

```
SetX(  SelectFloat(
           A      = RampAxisTo( X, clamp(AITarget), Rate, DeltaSeconds ),
           B      = <기존 플레이어 식 그대로>,
           bPickA = AIPoseDriven ) )
```

### 1.1 의도한 성질 둘

**① AI는 플레이어의 키 스텝이 쓰는 것과 *같은 Rate 변수*로 램프한다** (P71).

**AI가 사람보다 축을 빠르게 움직일 수 없다.** 별도의 "AI용 속도"를 두면 그 순간부터
**AI의 몸은 플레이어의 몸이 아니다** — 같은 애니메이션 층을 쓰면서 다른 물리를 갖게 되고,
플레이어로 검증한 모든 것이 AI에 대해서는 다시 미검증이 된다.

**② 목표값은 `MapRangeClamped` 로 클램프한다.**
`RampAxisTo`(`Math/SoldierAxisLibrary`)에는 **클램프가 없다.** 등속 램프라서 목표가 범위를
벗어나면 축도 따라 나간다. 플레이어 경로는 키 스텝이 클램프를 품고 있었으므로 이 문제가
없었다 — **AI 경로에서 처음 생긴 문제**다.

> 블루프린트 팔레트에 산술 노드가 없어(P33) 클램프에 `MapRangeClamped` 를 쓴다.
> `IMPLEMENTED.md` 4절의 "반파 정류기" 우회와 같은 계열이다.

### 1.2 지금 AI가 **실제로 모는 축** [A]

| 축 | AI가 모는가 |
|---|---|
| `AITargetStance` | ✅ `SoldierEngagement::GetDesiredStance()` |
| `AITargetLean` | ⬜ **변수와 배선만 있고 아무도 안 넣는다** → **[W19]** |
| `AITargetBlindFireH/V` | ⬜ 〃 |

**린과 블라인드 파이어는 몸에 이미 있다**(`IMPLEMENTED.md` 2.5e · [W7]). AI가 그것을
쓸 판단을 아직 갖고 있지 않을 뿐이다.

---

## 2. 컴포넌트와 진영 [A]

`BP_SoldierCharacter` 에 7개:

```
AC_SoldierIdentity      AC_SoldierPerception   AC_SoldierSight     AC_SoldierComms
AC_SoldierSuppression   AC_SoldierEngagement   AC_SoldierCover
```

자식 둘 — **`Faction` 기본값 하나만 다르다.**

```
BP_Soldier_Friendly   Faction = Friendly
BP_Soldier_Hostile    Faction = Hostile
```

**P4("아군/적군은 코드 한 벌. 진영은 데이터")가 지켜진 자리다.** 그리고 **[P53]** 때문에
그 기본값조차 도구로 쓸 수 없어서 **사용자가 에디터에서 넣었다.**

---

## 3. Tick 접합 — 순서가 전부다 [A]

```
SetAIPoseDriven( NOT IsPlayerControlled )
    ↓
SetAITargetStance( Engagement.GetDesiredStance )
    ↓
SetActualStance( StanceAxis )              ← 몸이 실제로 도달한 자세를 되돌려준다
    ↓
<기존 축 갱신 체인 그대로>
    ↓
Branch( AIPoseDriven )
    ↓ true
Branch( Engagement.WantsToFire )
    ├ true  → Shoot()
    └ false → Branch( Engagement.WantsToReload ) → StartReload()
```

`SetActualStance` 가 **목표를 넣은 다음, 축 갱신 앞**에 오는 것이 중요하다 — 교전 컴포넌트의
총구 높이(`..._engagement_and_cover.md` 3.1절)는 **희망 자세가 아니라 도달한 자세**로
계산돼야 한다. 아직 일어서는 중인 병사의 총열은 아직 담 아래다.

---

## 4. ★ 조준 상태 — 증상이 아니라 상태를 고쳤다 [A]

**증상**: AI가 **무기를 영영 안 들고, 몸도 안 돈다.**

### 4.1 인과 사슬

```
S_PlayerInputState.WantsToAim
    → RotationMode 가 aim 으로 파생
        → Enable_AO() 가 RotationMode == aim 을 요구
            → AimOffset · 총구 정렬 · 몸통 회전이 전부 그 뒤에 달려 있다
```

AI는 `WantsToAim` 을 **아무도 안 넣고 있었다.**

### 4.2 고친 방법 — 프로젝트 자신의 기존 경로를 읽어서 찾았다

`STT_SetSoldierInputState`([C-42]에서 만든 P0-1 실험 자산)가 **이미 같은 일을 하고 있었다.**
그 경로를 그대로 쓴다 —

```
GASP 입력 상태 구조체를 Break → Aim 만 덮어쓰고 나머지 필드는 전부 보존 → Make
    → UpdateInputStateServer
```

### 4.3 ⚠ 되돌린 패치 — P36의 사례가 하나 더 [A]

이전에 **파생 플래그인 `AOActive` 를 직접 건드리는 패치**를 넣었었다. 그것은 **증상을 다룬
것이고 상태를 다룬 것이 아니다** — 값의 출처가 둘이 되고, 그 뒤로는 어느 쪽이 맞는지
아무도 모른다.

**되돌렸다.** 값의 출처는 `S_PlayerInputState.WantsToAim` 하나다.

> P36("증상마다 패치를 덧대지 말고 근본 원인으로 돌아간다. 패치 두 개째가 신호다")이
> **또 한 번 맞았다.**

---

## 5. 무기가 자기 상태를 보고한다 [A]

`BP_AR4Rifle` 의 Tick이 캐릭터의 교전 컴포넌트를 `GetComponentByClass` 로 찾아
**직접 밀어 넣는다.**

```
Engagement.SetWeaponState( AmmoInMag, MagSize, IsReloading )
```

**무기가 보고하는 이유는 무기가 아는 쪽이기 때문**이다. 그리고 실무적으로도 반대가 불가능했다
— **캐릭터 그래프에서는 라이플의 변수 게터를 아예 만들 수 없다**(P55).

---

## 6. `AIC_Soldier` — `StartLogic` 노드를 **지웠다** [A]

**증상**: 병사들이 **벤치로 걸어가 앉았다가 순찰을 돌았다.**

**원인**: GASP 샌드박스에서 상속된 StateTree `ST_Soldier_SmartObject` —
`ST_NPC_SandboxCharacter_SmartObject` 의 복제본이고 `Root / SmartObject / Patrol` 상태를 갖는다.
그것이 **이동을 소유**하고 있었다.

**결정**: `StartLogic` 노드를 지워 StateTree를 아예 시작하지 않는다.

> **[C-43]("새 태스크를 StateTree에 물려 AI가 실제로 조준·앉기를 구동하는지")은
> 질문의 형태가 바뀌어 해결됐다.** 판단은 **StateTree가 아니라 컴포넌트 + Tick 접합**이
> 하고 있다. 설계 문서의 "유틸리티(판단) + StateTree(실행) 2계층"은 **아직 그 형태로
> 만들어지지 않았다** — 지금 있는 것은 그보다 얇다.

---

## 7. 관전 폰 — `BP_ObserverPawn` · `GM_SoldierObserver` [A]

AI끼리 싸우는 것을 보려면 **플레이어가 병사가 아니어야** 한다.

```
BP_ObserverPawn      부모 DefaultPawn
GM_SoldierObserver   GetDefaultPawnClassForController 를 오버라이드
```

### 7.1 `DefaultPawnClass` 를 **안 쓴** 이유

**[P53] — `set_properties` 가 이 환경에서 쓰기를 못 한다.** 게임모드 CDO의
`DefaultPawnClass` 를 쓸 수 없어서, **함수 오버라이드라는 그래프 편집으로 우회**했다.
도구 제약이 설계를 한 칸 옮긴 자리이므로 적어 둔다.

### 7.2 입력 — `Config/DefaultInput.ini` 를 **안 고쳤다** [A]

프로젝트에 이미 있던 **`FlyCam_*` 레거시 축 매핑**을 그대로 쓴다. 새 입력을 추가하면
설정 파일이 바뀌고, 그것은 [W15](P4V 체크아웃)와 엮인다.

### 7.3 ⚠ 이동 방향은 `GetControlRotation` 에서 온다 — 액터 벡터가 아니라

여기의 `DefaultPawn` 은 **`bUseControllerRotationPitch/Yaw = false`** 다. 즉
**액터가 영영 회전하지 않는다.** `GetActorForwardVector` 로 몰면 **시선과 무관하게
언제나 같은 방향으로 난다.**

---

## 8. 시험 레벨 — `/Game/SoldierLab/Levels/L_SoldierTest` [A]

```
폴더 SoldierLab_Urban    블록 19개 + LowWall_A
                         건물  7개   4~5m
                         중간  5개   1.5~1.7m
                         낮은  7개   1m
폴더 SoldierLab_Test     적대 3명 · 아군 2명
```

**높이 셋으로 나눈 것이 의도다.** 엄폐 층은 **막힌 첫 높이**로 필요 자세를 정하므로
(`..._engagement_and_cover.md` 7.2절), **선 채로 가려지는 것 / 웅크려야 가려지는 것 /
아무것도 못 가려 주는 것**이 한 레벨에 다 있어야 그 축이 눈에 보인다.
`LowWall_A` 는 **총구가 담 아래로 내려가는 것**을 보는 자리다(3.1절).

> [D3]("테스트 레벨 사양 + 성능 측정 방법")의 **앞쪽 절반이 이것으로 채워졌다.**
> 성능 측정은 여전히 없다 → **[C-83]**.

---

## 9. ★ 도구가 가르친 것 — 이 세션의 함정 9건

**전부 `CLAUDE.md` 5절에 P번호로 올렸다.** 여기서는 무엇을 하다 만났는지만 남긴다.

| P | 만난 자리 |
|---|---|
| **P53** | `ObjectTools.set_properties` 가 **쓰기를 못 한다** — 컴포넌트 템플릿 · 배치 인스턴스의 컴포넌트 · CDO 전부, float·FName·enum 가리지 않고 `false`. **읽기는 된다.** 이 문서의 모든 기본값은 **사용자가 직접 넣었거나 그래프 편집으로 우회**한 것이다(7.1절) |
| **P54** | `find_node_types` 가 `context_pins` 없이는 **아무것도 안 준다.** 맞는 타입의 출력 핀을 넘겨야 컨텍스트 노드가 나온다. **도구가 불안정한 것으로 오독했다** |
| **P55** | 발견은 되는데 **생성이 안 되는 노드**가 있다 — `CallFunction\|Shoot`, 다른 블루프린트 그래프에서 본 라이플 변수 게터. 우회: **exec 입력 핀은 연결을 여러 개 받는다** → 기존 호출 노드를 두 호출자가 공유 (3절의 `Shoot()`) |
| **P56** | `get_node_type_pins` 가 **트랜지언트 노드를 만들고 호출이 끝나면 부순다.** 반환된 참조를 재사용하면 **조용히 실패하고 끊긴 가지를 남긴다** → 엉뚱한 컴파일 ICE(`SetVariableOnPersistentFrame - No property found. Delta Seconds`) |
| **P57** | `UCLASS()` 매크로와 클래스 선언 **사이에** 무엇을 끼우면 UHT가 매크로를 **그 끼어든 것에** 적용한다 (`Found ';' when expecting '{'`) |
| **P58** | `Pawn\|GetControlRotation` 이 **APawn에도 AController에도 있다.** `create_node` 는 Controller 쪽을 고르고 블루프린트는 `This blueprint (self) is not a Controller` 로 죽는다 → `declaring_class` 로 못박는다 (7.3절) |
| **P59** | **툴 에러를 파이썬에서 잡아도 스크립트는 프레임워크 레벨에서 중단되고 반환값이 버려진다.** 이미 수행된 부작용은 **보이지 않게 남는다** — **컴포넌트 5개가 한 벌 더 생겼다**(2절) |
| **P60** | 블루프린트 변수의 **카테고리를 바꾸면 노드 type_id 경로가 바뀐다** — `Variables\|SoldierLab\|AIBridge\|Get...` 이지 `Variables\|Default\|Get...` 이 아니다 (1절) |
| **P61** | `write_graph_dsl` 은 **ABP 세터 노드가 든 그래프를 다시 쓰지 못한다**(읽기 전용 형태 `\|SetBF_AlphaL`). 다만 **쓰기 전에 검증하므로 그래프는 망가지지 않는다** — [6.1f]("`read_graph_dsl`은 무손실이 아니다")의 구체적 실패 지점 하나 |

---

## 10. 남은 것 [A]

| | ID |
|---|---|
| 린·블라인드 파이어 축을 **AI가 몰지 않는다** — 다리는 놓여 있다 | **[W19]** |
| 총구 보정 속도 게이트가 AI 병사에서 실제로 도는가 | **[C-75]** |
| 45명 규모 성능 · 시험 레벨의 성능 측정 절차 | **[C-83]** · [D3] |
| 목표·임무 개념 — **다음 큰 조각** | **[D10]** |
