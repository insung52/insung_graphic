# 오전 수정 넷 — 위협 보너스 · 사선 거부 → 재배치 · 가치 히스테리시스 · 차량 표적 높이

2026-09-17 / 완료 (빌드 · PIE, 사용자 "이제 잘 작동하는거 같아. 이상하게 멈춰있는 애들은 없긴했음" · 값 [C-136] [C-126 갱신]) / 같은 날 오후의 상황 필드(`ai/2026-09-17_situation_field_lighting_model.md`) **이전**에 들어간 교전 층 수정 넷. ① 맞으면서 엉뚱한 곳을 보던 것 → **"나를 쏘는 놈"에 가산점** ② 어떤 총구 자세로도 안 뚫리는 자리에 15 s 앉아 있던 것 → **사선 거부를 자리 비용으로** ③ 방아쇠가 30 Hz로 떨리던 것 → **가치 게이트 히스테리시스** ④ 병사가 UGV의 바퀴/땅을 겨누던 것 → **비-캐릭터는 바운즈 비율**.

전편: `ai/2026-09-16_squad_terms_and_learned_death.md`. 후속(같은 날): `ai/2026-09-17_infiltration_and_unknown_ground.md` · `ai/2026-09-17_situation_field_lighting_model.md`.
원칙: P36(패치 두 개째가 신호) · P89(규칙이 아니라 선호) · P10(계측 먼저).

---

## 1. 위협 보너스 — 맞으면서 엉뚱한 곳을 본다 [A · 코드 · PIE]

**증상**: 표적 선택이 확신(certainty)만으로 순위를 매기니 "가장 잘 아는 놈" = "가장 최근에 **본** 놈"이 이겼다. 시야 반감기 20 s, 소리 반감기 8 s라 **10 s 전에 흘끗 본 것**이 지금 내 엄폐물에 탄을 박는 **(귀로만 들리는) 차량**보다 높았다.

**수정**:

- `USoldierPerceptionComponent::ReportThreatenedBy(Shooter)`(`SoldierPerception.cpp:216-232`, `.h:350-360`) — 그 액터의 기록에 `LastThreatenedTimeSeconds`(`.h:115`, 융합은 max `:122-123`)를 찍는다. **기록이 없으면 만들지 않는다** — 안 보고 안 들은 사수를 여기서 발명하면 전지성. 총성 방송이 그 몫을 (귀의 흐림으로) 한다.
- 호출: 제압 근접탄 `ApplyNearMiss`(`SoldierSuppression.cpp:121-133`) · 피격 `USoldierHealthComponent`(`SoldierHealth.cpp:193-204`).
- 점수 (`SoldierEngagement.cpp:476-486`, 잠금 표적도 같은 식 `:683-691`):

```
Threatened = clamp(1 − SinceThreatened / ThreatenedMemorySeconds 5, 0, 1)
Crowd      = Threatened > 0 ? 0 : 아군이 이미 잡은 수        ← 나를 쏘는 놈은 분담 감점 면제
Score      = Certainty − TargetCrowdingPenalty 0.15 × Crowd + ThreatenedBonus 0.6 × Threatened
```

"맞고 있다"는 세계가 아니라 **이 병사**에 대한 유일한 증거라 그렇게 취급한다(`SoldierEngagement.h:529-543`). 시간이 지나면 사라지는 **돌아볼 이유**이지 잠금이 아니다.

---

## 2. 사선 거부 → 재배치 — 15 s 동안 전이 0 [A · 로그 · PIE]

**증상**: 능선 뒤·깊은 벽 뒤에서 병사가 완벽히 숨은 채 **어떤 총구 배치로도**(기립·린·블라인드) 표적에 안 닿는데, 점수 어디에도 그것을 벌하는 항이 없어 앉아 있었다. 로그 실측: **15 s 동안 병사 15명이 상태를 한 번도 안 바꿈**(`SoldierEngagement.h:219-231`). 엄폐 스윕의 `FightingCost`는 "서면 보이니 싸울 수 있다"고 했고 총구는 전부 언덕을 쳤다 — 후보에 대한 **추측**과 실제 선 자리에서 **총열에게 물은 답**이 어긋난 것.

**수정**:

- `SoldierEngagement.cpp:1335-1351`: `bDeniedNow = bHasContact && bGateBelieved && (FireIntent == Blocked || Aperture == None)`. 이것이 `LaneDeniedSeconds 2`(`.h:395-399`) 이상 지속되면 `bLaneDenied` 래치(`IsLaneDenied()`). 잠깐 숙인 표적은 자리 탓이 아니라 타이머.
- `SoldierCover::ScorePosition`(`SoldierCover.cpp:1104-1109`): **현재 자리에만** `Cost.Fighting += LaneDeniedCost 1.0`(`.h:461-472`). 후보에는 못 묻는 질문(총열이 거기 없다)이라 HERE만.
- 그리고 스프린트 규칙의 `bUrgent`에 `bLaneDenied`가 들어갔다(후속 문서 5절) — 안 통하는 자리를 떠나는 이동은 급하다.

---

## 3. 가치 히스테리시스 — 방아쇠 떨림 [A · 코드 · PIE]

**증상**: 교전 거리에서 산포 콘이 `SuppressiveRadiusCm 500` 한계 **몇 % 안쪽**에 걸려 있어, 한 발의 반동이 넘기고 다음 틱 회복이 되돌리며 **AIMED, hold, AIMED, hold**가 30 Hz로 — 매번 1~2발. 자기 출력으로 뒤집히는 결정(P35 계열).

**수정** (`SoldierEngagement.cpp:1202-1203`, `.h:401-411`):

```
bWorthTheRound = SpreadCm <= SuppressiveRadiusCm × (bWasFiring ? WorthHysteresis 1.2 : 1)
```

`bWasFiring = WantsToFire()`는 틱 끝에 래치(`:1332`). 이미 쏘는 중이면 한계를 20% 넘어도 계속 쏜다.

---

## 4. 차량 표적 높이 — 바운즈 비율 [A · 코드]

**증상**: 브리지가 UGV/트럭에 붙인 `USoldierIdentityComponent`는 소켓이 없어 눈 `+160`/표적 `+110`을 **액터 원점** 위에 더했다([C-126]). 차량 BP의 원점은 어디든 될 수 있어 그 점이 차체 안이나 **땅 밑**에 떨어졌고 시야 판정·사선이 실체 아닌 점을 두고 다퉜다 — 병사가 바퀴·지면을 겨눴다.

**수정** (`SoldierIdentity.cpp:20-73`): `GetSocketOrFallback`이 소켓 없음 ∧ `Cast<ACharacter>` 실패면 `GetActorBounds(true)`의 Z 범위에서 비율을 취한다 — **눈 0.8**(포탑/캐빈 높이쯤), **표적 0.5**(차체 한가운데, 쏘기에 가장 정직한 것). 바운즈가 0이면 옛 상수 폴백.

⚠ 0.8/0.5는 감이다 → [C-126]에 갱신 등록. 실제 UGV_0901 바운즈에서 눈/표적이 어디 찍히는지는 `SoldierLab.Debug.Sight 1`로 봐야 한다.

---

## 5. 값 (전부 [C]) → [C-136] · [C-126]

```
SoldierEngagement   ThreatenedBonus 0.6 · ThreatenedMemorySeconds 5   (신설)
                    LaneDeniedSeconds 2                                 (신설)
                    WorthHysteresis 1.2                                 (신설)
                    TargetCrowdingPenalty 0.15                          (그대로 — 위협 시 면제만 추가)
SoldierCover        LaneDeniedCost 1.0                                  (신설)
SoldierIdentity     비-캐릭터 폴백: 눈 = 바운즈 0.8 · 표적 = 0.5          (신설)
SoldierPerception   FSoldierEnemyRecord.LastThreatenedTimeSeconds       (신설 필드)
```

**판정**: [C-136] — `[Engage]` 로그에서 근접탄 뒤 표적 전환이 사수 쪽으로 가는가(`switch` 원인) · `-> hold`가 2 s 넘게 `blocked`로 지속된 병사가 `[Cover]` `MOVE`를 내는가(사선 거부 래치 → 이동) · `AIMED↔hold` 전이 간격이 0.03 s 단위로 반복되지 않는가.

---

## 6. 변경 파일

```
Source/SoldierLab/AI/SoldierEngagement.h/.cpp    ThreatenedBonus · ThreatenedMemorySeconds · LaneDeniedSeconds · bLaneDenied/IsLaneDenied ·
                                                 WorthHysteresis · bWasFiring
Source/SoldierLab/AI/SoldierPerception.h/.cpp    LastThreatenedTimeSeconds · ReportThreatenedBy · GetSecondsSinceThreatened
Source/SoldierLab/AI/SoldierSuppression.cpp      ApplyNearMiss → ReportThreatenedBy
Source/SoldierLab/AI/SoldierHealth.cpp           피격 → ReportThreatenedBy
Source/SoldierLab/AI/SoldierCover.h/.cpp         LaneDeniedCost · ScorePosition HERE 항
Source/SoldierLab/AI/SoldierIdentity.cpp         GetSocketOrFallback 바운즈 비율
```
