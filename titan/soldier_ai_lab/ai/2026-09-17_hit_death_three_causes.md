# 피격·사망이 안 나오던 이유 — 하나의 증상, 세 개의 원인

2026-09-17 / 완료(사망 몽타주는 의도적으로 끔) / "AI가 맞아도 반응이 없다" 하나에 **독립된 원인이 셋**이었다 — ① C++ 생성자의 콘텐츠 경로 하드코딩(몽타주 배열이 전부 빈 채였고, 같은 로드가 옛 폴더를 루트셋에 박아 이동·삭제까지 막았다) ② 레벨 배치 인스턴스가 **빈 배열을 직렬화해 들고** 있었다 ③ 사망 몽타주의 슬롯 `FullBody` 가 **ABP 에 없었다**. 그 뒤 이중 낙하 → 사망 몽타주 비활성(사용자 결정), 래그돌 품질 → 피직스 에셋 교체.

관련 항목: 신규 **[W71]** **[W72]** **[C-127]** **[C-128]** / 해결·정정 대상: `ai/2026-09-15_health_hit_death_implementation.md` 4.4·6.1절 · [C-117] / 관련 문서: `animation/prototypes/2026-09-17_ally_enemy_anim_set_split.md`(이 사고의 방아쇠), `IMPLEMENTED.md` 5절 · 3.2절, 원칙 **P134~P140**

---

## 0. ★ 이 문서의 뼈대 — 하나의 증상에 원인이 셋이었다

**P47** 의 실사례다. "피격/사망 애니메이션이 안 나온다"는 증상 하나를 놓고 고칠 때마다 조금씩 나아졌는데, **고친 셋은 서로 아무 관계가 없었다**:

```
증상       AI 가 맞아도 움찔하지 않고, 죽을 때도 1초쯤 마비된 뒤 갑자기 래그돌
원인 1     C++ 생성자가 하드코딩한 경로가 폴더 분리로 죽어 몽타주 배열이 전부 비었다   (모두에게)
원인 2     레벨에 배치된 인스턴스가 배열이 비어 있던 시절의 값을 직렬화해 들고 있었다   (AI 에게만)
원인 3     Death 몽타주의 슬롯 FullBody 가 ABP 에 없어 포즈가 출력에 안 실렸다        (사망에만)
```

원인 1을 고치면 **플레이어만** 반응하고, 2를 고치면 **피격은 나오는데 사망이 이상하고**, 3까지 고쳐야 끝났다. 중간마다 "고쳤다"고 말할 수 있는 상태가 있었다는 것이 이 유형의 위험이다.

그리고 **원인 1이 원인 0(옛 폴더를 지울 수 없던 것)의 정체이기도 했다** — 1절이 먼저 일어난 사건이고 그 원인은 2절에서야 밝혀졌다.

---

## 1. 발단 — 옛 애니메이션 폴더를 지울 수 없었다 [A]

세트 분리(`animation/prototypes/2026-09-17_ally_enemy_anim_set_split.md`) 뒤 옛 `Animations/` 에 **40개가 리다이렉터가 아니라 전체 사본으로** 남았다. 크기 대조로 확인 — 254,195 B vs 254,229 B(리다이렉터라면 수백 바이트다).

정체: **HitReact 13 + Death 6 몽타주 + 그 소스 클립 19 + `MM_Rifle_Idle_ADS` / `MM_Rifle_Idle_Hipfire` 2.**

삭제하려 하면:

```
is in use
External referencers: /Engine/Transient.AnimSequencerController_0 (root)
                      /Engine/Transient.AnimDataController_16 (root)
```

- `obj gc` 도, 에디터 재시작도 **부분적으로만** 통했다 — 클립 21개는 풀렸고 **몽타주 19개는 계속 잡혔다.**
- **Force Delete 도 거부**됐다. 참조자가 `(root)` 면 force delete 가 참조를 null 로 바꿀 수 없다.
- `/Game` 안의 참조는 디스크 전수 스캔으로 **0건** 확인.

최종 해결은 **에디터를 끄고 파일째 삭제**. 에러 메시지의 `-NoLoadStartupPackages` 힌트가 단서였는데, **왜 루트셋에 박혔는지의 진짜 원인은 2절에서 밝혀졌다.**

---

## 2. 원인 1 — C++ 생성자의 콘텐츠 경로 하드코딩 [A]

`USoldierHealthComponent` 생성자가 `ConstructorHelpers::FObjectFinder` 로
`"/Game/SoldierLab/Animations/Actions/%s.%s"` **19개**를 로드하고 있었다(2026-09-15 구현 시의 의도적 선택 — `ai/2026-09-15_health_hit_death_implementation.md` 4.4절).

폴더 분리로 그 경로가 죽자:

```
LogUObjectGlobals: Error: CDO Constructor (SoldierHealthComponent):
    Failed to find /Game/SoldierLab/Animations/Actions/AM_MM_Death_Front_01...      × 19건
```

귀결 셋:

1. **몽타주 배열이 전부 빈 채로** 남아 피격·사망 애니메이션이 **한 번도 재생된 적이 없었다.**
2. 그리고 **이 생성자가 시작 시 몽타주를 로드해 루트셋에 박은 것이 1절의 이동·삭제 거부의 원인**이었다. 폴더를 못 지운 것과 피격이 안 나온 것은 **같은 한 줄**이었다.
3. 에디터 로그의 `CDO Constructor` 에러는 쿡 실패 카운트에 안 잡히는 종류라(메모리: "Cook failed 인데 쿡은 Done!") 조용히 넘어가기 쉽다.

### 조치 — B안 채택: 걷어내고 데이터로 [A]

생성자의 `FObjectFinder` 블록과 `#include "UObject/ConstructorHelpers.h"` 를 **삭제**하고, 왜 걷어냈는지를 소스 주석으로 남겼다(`SoldierHealth.cpp:92-100`). 몽타주는 블루프린트 데이터로 이관:

| 어디 | 무엇 |
|---|---|
| `BP_SoldierCharacter` 의 `AC_SoldierHealth` 템플릿 | **`Enemy_` 19개** — Front L4/M2/H1 · Back·Left·Right 각 L1/M1 · Death F3/B1/L1/R1 |
| `BP_Soldier_Friendly` 의 같은 템플릿 | **`ALLY_` 19개** 오버라이드 |
| `BP_Soldier_Hostile` | 자체 컴포넌트 템플릿이 없어 **부모(Enemy) 상속** |

사용자가 빌드했고 `Failed to find` 에러는 사라졌다.

> **원칙**: 콘텐츠 경로를 C++ 생성자에 하드코딩하지 않는다 → **P134**. 폴더 이동에 조용히 깨지고, 게다가 **루트셋에 박아 이동·삭제까지 막는다**.

---

## 3. 원인 2 — 레벨 배치 인스턴스가 빈 배열을 들고 있었다 [A]

빌드 후에도 **AI 만** 피격 반응이 없었다. 플레이어는 나왔다.

처음 의심한 것은 **"조준/사격 중이라 막히나"** 였다 — 사격 몽타주가 같은 슬롯 그룹의 몽타주를 정지시킨다는 가설(5절). **그런데 전투 전에 가만히 서 있는 AI 도 반응이 없었다.** 그래서 가설을 버리고 PIE 인스턴스를 직접 읽었다:

```
BP_Soldier_Friendly_C_0 / BP_Soldier_Hostile_C_0 의 AC_SoldierHealth
    DeathFront 0개 · HitReactFront{ Light:0, Medium:0, Heavy:0 }
```

**템플릿은 채워져 있는데 배치 인스턴스는 비어 있었다.** 배열이 비어 있던 시절에 레벨에 배치돼 **그 상태가 직렬화**됐고, 플레이어 폰만 런타임 스폰이라 템플릿 값을 받았다. 메모리의 "MCP CDO 쓰기는 레벨 인스턴스에 전파 안 됨" · [C-50] 과 같은 계열이다.

**해결**: 레벨 액터를 새로 배치(또는 Details 에서 Reset to Default). 사용자가 새로 배치해 해결됐다.

> **원칙**: 배치 인스턴스는 템플릿 갱신을 안 따라온다 → **P139**. "플레이어는 되는데 AI 만 안 된다"는 **스폰 경로 차이**를 먼저 의심한다.

---

## 4. 원인 3 — 사망 몽타주의 슬롯이 ABP 에 없었다 [A]

`AM_MM_Death_*` 의 슬롯은 **`FullBody`** 인데, ABP 의 슬롯 노드는 다섯뿐이다:

```
DefaultSlot · FullBodyAdditivePreAim · UpperBody · UpperBodyAdditive · AdditiveHitReact
```

`FullBody` **가 없다.** 스켈레톤에는 슬롯 이름이 등록돼 있으므로 몽타주는 유효하고 **재생도 된다** — 다만 그 포즈를 받는 노드가 그래프에 없어 **출력에 안 실린다.** 경고도 없다.

증상은 **"약 1초 마비된 뒤 갑자기 래그돌"** 이었다. `PlayDeath` 가 몽타주 길이 − `RagdollLeadSeconds` 만큼 기다렸다가 래그돌을 켜기 때문에, 그 대기 시간 동안 아무 포즈도 안 바뀐 것이다.

### 조치 — 슬롯 노드를 만들지 않고 몽타주 쪽을 고쳤다 [A]

사망 몽타주 **12개(적군 6 + 아군 6)** 의 슬롯을 **`DefaultSlot`** 으로 변경.

`FullBody` 슬롯 노드를 새로 만드는 대신 이쪽을 고른 이유: **`Enable_AO` 가 몽타주 중 조준을 끄는 판정에 `GetSlotLocalWeight("DefaultSlot")` 만 본다.** `DefaultSlot` 을 쓰면 **쓰러지면서 조준 오프셋이 남는 문제가 자동으로 해결**된다. (그리고 2026-09-15 문서가 이미 "Death 6 → `DefaultSlot`" 으로 적고 있었다 — 문서가 맞고 에셋이 달랐던 셈이다.)

> **원칙**: 애디티브/전신 몽타주가 안 보이면 **슬롯 노드가 그래프에 존재하는지부터** 본다 → **P140**. 슬롯 이름이 스켈레톤에 등록돼 있어도 ABP 에 노드가 없으면 조용히 버려진다.

---

## 5. 슬롯 그룹 — 조사했으나 원인은 아니었다 [A]

3절에서 버린 가설을 그래도 끝까지 확인했다.

스켈레톤 `SK_UEFN_Mannequin` 의 슬롯 그룹은 **`DefaultGroup` 하나뿐**이고 슬롯 6개(`DefaultSlot` · `FullBody` · `FullBodyAdditivePreAim` · `UpperBody` · `UpperBodyAdditive` · `AdditiveHitReact`)가 전부 거기 속한다.

- 피격은 `Montage_Play(..., bStopAllMontages = false)` 라 남을 안 끊는다.
- 그러나 **사격/재장전은 `PlayAnimMontage` → `Montage_Play` 기본값 `bStopAllMontages = true`**(`AnimInstance.h:626` · `Character.cpp`)라 **같은 그룹의 몽타주를 전부 정지시킨다.**

교전 중 AI 는 계속 쏘므로 **피격 몽타주를 밀어낼 수 있다.** — **이번 증상의 원인은 아니었지만(원인 2가 진짜였다) 구조적 위험은 그대로 남아 있다.**

정석 해결은 `AdditiveHitReact` 를 **별도 슬롯 그룹**으로 빼는 것(스켈레톤의 Anim Slot Manager, **MCP 불가**). 미착수 → **[W71]**.

참고로 **애디티브 몽타주가 보이기 위한 3조건**은 이번에 전부 확인됐다 [A]: 클립이 `AAT_LocalSpaceBase` / `ABPT_LocalAnimFrame` · 슬롯이 `ApplyAdditive_0.Additive` 입력(Base 아님) · 소스가 `EIT_Additive` 항등 포즈.

---

## 6. 두 번 넘어짐 → 사망 몽타주 비활성 (사용자 결정) [A]

슬롯을 고치자 이번엔 **1차 애니메이션 낙하 → 0.75초 뒤 순간이동하듯 튀며 2차 래그돌 낙하**.

원인은 `RagdollLeadSeconds = 0.1` 이라 **몽타주가 거의 끝까지 재생된 뒤**(이미 바닥에 누운 뒤) 물리가 새로 시작되는 것이다 — "애니메이션이 눕히고 물리가 다시 눕히기". [C-117] 이 물었던 "전환 프레임에 튀는 프레임이 있는가"의 답이 **있다** 로 나온 셈이고, 다만 원인은 MM 포즈가 아니라 **누운 자세에서의 물리 재시작**이었다.

선택지:

| | 안 | 내용 |
|---|---|---|
| (A) | `RagdollLeadSeconds` 0.1 → **0.6** | 몽타주 앞 0.25초만 쓰고 넘긴다 |
| (B) | **`bPlayDeathMontage = false`** | 순수 래그돌 |

**사용자 선택 = (B).** 피직스 에셋을 마네킹 것으로 바꾼 뒤라(7절) 순수 래그돌 품질이 충분하다는 판단.

적용: `BP_SoldierCharacter` + `BP_Soldier_Friendly` 템플릿에 `bPlayDeathMontage = false`.

**사망 시 흐름 (현행)** [A]:

```
StopDriving
 → 즉시 StartRagdoll (캡슐 충돌 해제, pelvis 이하 시뮬)
 → bInheritVelocity = true 로 직전 속도 승계
 → 다음 틱에 AddImpulseAtLocation(피격방향 × DeathImpulseMagnitude 1500, 피격위치, 맞은 본)
 → 8초 뒤 FreezeCorpse
```

⚠ **사망 몽타주 12개의 `DefaultSlot` 변경은 되돌리지 않고 유지한다** — 나중에 (A)안을 시험할 때 필요하다 → **[C-128]**.

---

## 7. 래그돌 품질 — 피직스 에셋 [A / C]

`soldier_T` 와 `new_enemy_T` 는 FBX 임포트 때 자동 생성된 `soldier_T_PhysicsAsset` / `new_enemy_T_PhysicsAsset` 을 쓰고 있었다(마네킹만 Epic 이 튜닝한 `PA_UEFN_Mannequin`). 자동 생성본은 관절 제한이 기본값이라 **무릎·팔꿈치가 반대로 꺾였다.**

**A안 적용** [A]: 두 메시의 피직스 에셋을 **`PA_UEFN_Mannequin` 으로 교체**(MCP `assign_physics_asset`). 스켈레톤이 같아 본 이름으로 매칭된다.
**대가**: 바디 모양이 마네킹 체형 기준이라 아군(181 cm) · 적군(178.7 cm)에서 캡슐이 메시와 약간 어긋난다.

**장기안(C)**: `PA_UEFN_Mannequin` 을 복제해 진영별로 두고 **제한값은 유지, 바디 크기·위치만 메시에 맞춤** → **[W72]**.
⚠ MCP 는 피직스 에셋의 **바디·콘스트레인트를 읽지도 쓰지도 못한다**(`CLAUDE.md` 6.1) — 에디터 수작업이다.

개선 효과는 사용자 육안으로 *"두 번째 낙하는 제대로 사망"* 수준까지 확인됐으나 **정량 판정은 안 했다** [C] → **[C-127]**. 이것은 [C-111](바디 실체 확인)과 짝이다.

---

## 8. 판정

**성공(피격·사망 동작).** 적군이 맞으면 움찔하고, 죽으면 그 자리에서 래그돌로 쓰러진다. 아군은 무적이라 움찔만 한다.

**부분(품질).** 사망 연출은 지금 **몽타주 없는 순수 래그돌**이다 — 의도한 선택이지만 애니메이션이 있는 쪽이 더 나은지는 [C-128] 에서 다시 본다. 래그돌 품질 자체는 미측정 [C-127].

## 9. 이것이 바꾸는 것

| 문서 | 절 | 어떻게 |
|---|---|---|
| `IMPLEMENTED.md` | 5 · 3.2 · 6 | 체력 컴포넌트가 **BP 데이터**(C++ 하드코딩 제거) · 피직스 에셋 교체 · 사망 몽타주 OFF |
| `ai/2026-09-15_health_hit_death_implementation.md` | 8(신설 정정 절) | 4.4절(생성자 하드코딩)이 폐기됐다 |
| `assets/2026-09-14_design_team_animation_handoff.md` | 2.8 | 사망 몽타주는 현재 재생되지 않는다(슬롯은 `DefaultSlot` 로 바뀜) |
| `CLAUDE.md` | 5 | P134 · P136~P140 |

- [x] 원 문서에 결과 반영
- [x] `OPEN_ITEMS.md` 등록 ([W71] [W72] [C-127] [C-128])
- [x] `CURRENT_STATE.md` 갱신

## 10. 막힌 것 / 다음에 확인할 것

- **[W71]** `AdditiveHitReact` 를 별도 슬롯 그룹으로 — 5절의 구조적 위험(사격이 피격 몽타주를 끊는다)은 **그대로 남아 있다**
- **[W72]** 피직스 에셋 진영별 복제 + 바디 피팅(에디터 수작업)
- **[C-127]** 래그돌 품질 정량 판정 — 짝은 [C-111]
- **[C-128]** 사망 몽타주 재도입 시 `RagdollLeadSeconds` 튜닝(0.6 부근). 슬롯 변경은 유지해 뒀다
- [C-112] · [C-113] · [C-118] 은 **여전히 미측정**이고, 사망 몽타주를 껐으므로 [C-112](Death 클립 루트모션) · [C-117](전환 프레임)은 **당장은 무의미**해졌다 — (A)안을 시험할 때 함께 되살아난다
- **미저장/체크아웃 대기** — `CURRENT_STATE.md` 2026-09-17 저녁 블록
