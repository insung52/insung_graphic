# 상황 필드(Situation Field) — 위험도는 저장하지 않고 "조명"으로 파생한다

2026-09-17 → **2026-09-18 갱신** / 진행중 (1·2·**3단계**(호라이즌/앰비언트) + 오버레이 + **LOD 링** 빌드·PIE 확인 — 사용자 "딱 내가 원하는 그림이 이제 나옴" · "기능적으로는 아주 잘 작동" · **09-18 후반부(대칭 캡·라이트 시각 v2·부정 증거·헤더 2줄·파랑 미굽기)는 빌드 전 [B]** · 값은 전부 [C]) / 진영별 **위험 지도**(`SoldierDangerMap`, 09-14)를 **폐기**하고 `USoldierSituationFieldSubsystem`으로 교체했다. 저장하는 것은 **경계도(presence) 하나**, 정적으로 굽는 것은 **호라이즌 맵 하나**, **위험도(exposure)는 저장하지 않고 파생한다** — 목격(sighting)이 **점광원**이고 엄폐물이 **그림자**를 드리우며, 안 본 땅의 사전 확률이 **앰비언트**다. 렌더링 방정식 그대로. **09-18**: 레벨 0 위에 **밉(아래→위 집계) + 다중 앵커 퇴거**(병사에게서 먼 디테일을 부모에 **잔여물**로 접고 해제 — 메모리가 "지나간 자리"가 아니라 "병사가 있는 자리"에 묶인다) · 오버레이 v2(자체 배처, 클립맵 링, 불투명도 = 신선도) · 라이트 **부정 증거**(보고 있는데 없으면 반감기 하나 더) → **16·17·18절**. **09-18 오후**: **순찰**(`GetStaleVantage` — 앰비언트 적분의 방사체를 "안 본 지 얼마나"로) · **부채꼴 스캔**(`GetMostExposedDirection`에 편향·arc) · **solid 셀**(굽기가 기하 안에서 시작 = "여기 없음") · **미지 = 열림**(`GetExposureByStance` → bool) · **콘 스윕 띠 넓히기** · W83 해결 → **20~23절**(거동 쪽은 `ai/2026-09-18_patrol_scan_and_move_robustness.md`). **09-18 밤**: `GetExposure` 미지 = 사전값(0 이 아니라 `UnknownPresence × AmbientWeight`) → **24절**(정정). ★ **09-21**: 필드가 **진영별 → 분대별**(`FScope`), `GetWedgePresence`, 섀도우 재캐스트 문턱 + riders → **25절 포인터** → `ai/2026-09-21_per_squad_field_edge_advance_fire_model.md`(이 문서의 "진영별" 은 그 뒤로 "분대별" 로 읽을 것).

> ⚠ **줄 번호 주의**: 아래 1~15절의 `SoldierSituationField.cpp:NNN` 참조는 **09-17 판(1488줄)** 기준이다. 09-18에 파일이 **2332줄로 재작성**돼 옛 번호는 전부 어긋난다 — 함수 이름으로 찾을 것. 16~18절의 번호는 09-18 오전 판(2332줄), **20~23절은 09-18 저녁 판(2491줄)** 기준 — 셋이 서로 다르다.

전편: `ai/2026-09-16_squad_terms_and_learned_death.md`(분대 항·배운 죽음·두 점 시야). 같은 날 오전의 선행 수정(위협 보너스·사선 거부·차량 높이)은 `ai/2026-09-17_threat_bonus_lane_denied_vehicle_heights.md`, 이 필드가 **왜** 필요했는가(잠입 요구)와 병사 거동 쪽은 `ai/2026-09-17_infiltration_and_unknown_ground.md`.
폐기된 전신: `ai/2026-09-14_danger_map_and_position_commitment.md`(**superseded** — 상단 배너 참고).
원칙: **신설 P143~P151**(`CLAUDE.md` 5절) · **09-18 신설 P152~P157**(LOD·퇴거·오버레이·부정 증거) · P63(감쇠는 질의 시) · P130(세계가 대신 알려 주지 않는다) · P133(부정 증거) · P7(디버그 표시는 1급 시민) · P10(계측 먼저).

> **읽는 법**: 2절(모델)이 이 문서의 핵심이고 나머지는 그 모델의 구현·계측·값이다. 코드를 읽을 사람은 `SoldierSituationField.h:36-104`의 클래스 주석이 설계 선언문이니 그것부터(09-18에 RESOLUTION·EVICTION 문단 `:75-88`이 추가됐다). ~~이 문서의 거동 서술 중 3단계(7·9·10절 일부)는 [B]다 — 빌드 0회.~~ → **09-18: 3단계 빌드·PIE 확인됨.** 지금 [B]인 것은 17.4절의 "미빌드 목록"뿐.

---

## 0. 한 장 요약

| 무엇 | 어디 | 상태 |
|---|---|---|
| **상황 필드** — 진영별 XY 해시 격자, 2.5D(셀당 지면 Z 캐시), 희소 레벨 피라미드 | `AI/SoldierSituationField.{h,cpp}` **신규** `USoldierSituationFieldSubsystem : UTickableWorldSubsystem` | [A] 코드 · 1·2·3단계 PIE 확인 |
| **09-18 밉 + 다중 앵커 퇴거** — 레벨 1+는 레벨 0의 **집계**(`FCoarseCell`/`FCoarseHorizon`), 병사에게서 `DetailRadiusCm × LevelScale^L` 밖 디테일은 부모에 **잔여물**로 접고 해제 | `SoldierSituationField.cpp` `RefreshCoarse` · `Evict` · `EvictHorizons` · `FoldInto` · `SampleFinest` — **16절** | [A] 코드 · LOD 링 PIE 확인("기능적으로는 아주 잘 작동") |
| **09-18 오버레이 v2** — 자체 `ULineBatchComponent`, 0.1 s마다 flush+refill, 색별 메시 1개, 클립맵 링, 불투명도 = 신선도, 대칭 캡, 헤더 2줄, 라이트 시각 v2, 볼 곳 화살표 | `DrawDebug` · `DescribeGround` · `GetDebugBatcher` — **17절** | 링·배처 [A] PIE · **대칭 캡·라이트 v2·헤더 2줄·파랑·화살표 길이 [B] 빌드 전** |
| **09-18 라이트 부정 증거** — 얼린 목격을 빈 채로 보고 있으면 `ClearViewHalfLifeSeconds 4`로 한 번 더 감쇠(삭제 아님) | `FLight::ClearViewSeconds` · `ContradictLightsAlong` · `LightBrightnessNow` — **18절** | ~~[B] 빌드 전~~ → 오후 빌드 [A] · 들은 라이트 제외([W83] 해결) |
| **09-18 오후 순찰·스캔·solid·미지** — `GetStaleVantage`(낡은 조망) · `GetMostExposedDirection` 편향/arc · `FHorizon::bSolid` · `GetExposureByStance` → bool · `MarkClearAlongRay` 띠 | **20~23절** | [A] · PIE ✅ ("이제 정상적이다") |
| **프로젝트 설정** — 해상도·수명·라이트·섀도우·앰비언트·디버그 값 전부 | `AI/SoldierFieldSettings.h` **신규** `USoldierFieldSettings : UDeveloperSettings`, Project Settings → Game → **SoldierLab Situation Field** · 09-18 `SoldierLab.Field.CellSizeCm` cvar(런타임 셀 크기 오버라이드) | [A] · 값 [C-130]~[C-133] [C-139] · 09-18 **[C-140]~[C-143]** |
| **로그 카테고리 이사** | `AI/SoldierLabLog.{h,cpp}` **신규** — `LogSoldierAI`가 위험 지도 헤더에서 여기로 | [A] |
| **위험 지도 삭제** | `AI/SoldierDangerMap.{h,cpp}` **`p4 delete`** · `SoldierLab.Debug.Danger` cvar 소멸 · `SoldierCover::DangerHalfLifeSeconds` 소멸 | [A] |
| **쓰는 쪽** — 눈(목격·빈 땅) · 귀(총성) | `SoldierSight::SweepCone` / `TickComponent` · `SoldierPerception::ReportGunshot` | [A] |
| **안 쓰는 쪽** — 엄폐 부채꼴·몸 밴드·경로 표본 · 제압 근접탄 | `SoldierCover::AddFanCandidates` 주석 · `SoldierSuppression::ApplyNearMiss` 주석 | [A] (의도적) |
| **읽는 쪽** — 자리 Danger 항 · 경로 위험 · 눈 0일 때 자세/은폐 · 어두운 셀 후보 · 볼 곳 | `SoldierCover::ScorePosition / EvaluateRoute / EvaluatePosition / AddFieldCandidates / BeginSweep` → `SoldierEngagement`(watch point) | 앞 둘 [A] PIE · 뒤 셋 **[B] 빌드 전** |
| **오버레이** — 월드당 1회, 셀 채움 사각형(~~크기=신선도~~ → 09-18 **불투명도=신선도**, 크기 고정) + 라이트 핀/링 | `SoldierSituationField::Tick / DrawDebug` · `SoldierDebugDraw::GetObservedSoldier` **신규** | [A] PIE 확인 · v2는 17절 |
| `Build.cs` | `DeveloperSettings` 의존 추가 | [A] |
| 폐기한 접근 | 위험 지도(누적 버퍼) · **가상 관찰자**(`AddUnknownWatchers`, 같은 날 도입·철회) | 12절 |

---

## 1. 왜 — 문제와 증상

### 1.1 위험 지도가 답할 수 없던 질문

09-14의 `USoldierDangerMapSubsystem`은 진영별 2 m 격자에 셀당 세 시각(보였음/공터였음/총알 지나감)을 찍고 30 s 반감기로 낡히는 **땅의 기억**이었다. 잘 돌았다(09-15 "지금까지는 가장 좋네"). 그러나 09-17 오후에 **잠입 요구**(적군이 지식 0으로 첫 목표까지 들어간다 — `ai/2026-09-17_infiltration_and_unknown_ground.md` 1절)가 오자 구조적 한계가 드러났다:

- **노출이 "믿는 적"에 대해서만 정의돼 있었다.** 기록(`FSoldierEnemyRecord`)이 0이면 엄폐 층의 `EvaluatePosition`은 `Eyes.Num()==0`에서 즉시 `CanHide=true / Exposure=0`으로 빠졌다. 즉 **아무도 모른다 = 아무도 나를 못 본다**. 그 결과가 광장 한복판을 전력 질주하는 분대였다.
- 위험 지도는 **눈이 0일 때만** 자리 점수에 들어갔고(P116), 그때 셀에 든 것은 "공터였음" 시각뿐 — **안 본 곳**에 대해 아무 말도 안 했다.

### 1.2 1차 필드(복셀 이식)와 "붉은 얼룩"

그래서 먼저 위험 지도를 **복셀 필드**로 넓혔다 — 채널을 경계도/위험도/신선도로 나누고, 셀에 지면 Z를 캐시하고, 안 본 셀의 사전값(`UnknownPresence 0.5`)을 넣고, 가상 관찰자를 세웠다(12.2절). 사용자가 오버레이를 켜고 본 것:

> 병사 주변이 **전부 붉다**. 엄폐물 뒤와 공터가 거의 구별이 안 된다. 위험도가 **내려오질 않는다**. "확인된 적이 여기 조준할 수 있다"와 "총성/오래된 정보"가 **같은 색**이다.

사용자의 머릿속 그림은 달랐다: *적이 시야에 들어오면 그 자리가 **초록**으로 켜지고, 거기서 붉음이 **점광원처럼** 퍼지되 엄폐물이 **그림자**를 만들어야 한다.* 지금 그림은 "**글로벌 일루미네이션**"처럼 보인다고 했다.

결함 셋을 찾았다: ① 가상 관찰자의 부채꼴 광선이 **가설을 사실로** 필드에 썼다 ② 관찰자가 `FanRadiusCm` 안에 있으면 부채꼴이 **360°로 퇴화**해 사방을 칠했다 ③ 몸 밴드 ±200 cm 안의 셀을 전부 1.0으로 찍었다. 그런데 셋을 고쳐도 남는 **구조적 원인**이 있었다:

> **위험도가 누적 버퍼(accumulation buffer)였다.** 병사 45명 × 눈 3개 × 광선 48줄이 매 스윕 max-merge로 셀에 쌓이고, **누가 썼는지(출처) 기록이 없고**, 30 s 타이머로만 지워진다. 이건 우연히 만든 GI다 — 모든 병사의 추측을 30초 노출로 겹쳐 찍은 사진.

이것이 P147이다: **그림이 GI처럼 보이면 출처 없는 누적 버퍼가 있는 것이다.**

---

## 2. 모델 — 렌더링 방정식 (핵심)

### 2.1 위험도 = ∫ 경계도 × 가시성

한 점 Y의 위험도는 **"적이 있을 수 있는 모든 곳 X에 대해 (X에 적이 있을 확률) × (X에서 Y가 보이는가)"의 적분**이다.

```
위험도(Y) = ∫ 경계도(X) · 가시성(X → Y) dX
```

이건 비유가 아니라 **문자 그대로 렌더링 방정식**이다 — 경계도가 방사체(emitter), 가시성이 visibility term. 그리고 렌더러가 푸는 방식 그대로 푼다:

| 경계도의 분포 | 렌더링에서의 이름 | 여기서의 이름 |
|---|---|---|
| **한 점에 집중**(누군가를 봤다) | 점광원 + 섀도우 | **라이트(light) + 직접 노출(direct)** |
| **균일**(어디에나 있을 수 있다 — 안 본 땅) | 앰비언트 오클루전 | **앰비언트(ambient) + 호라이즌 맵** |

따라서 **저장 1 + 정적 1 + 파생 1**:

- **저장** — **경계도(presence)**. 진영의 실제 지식. 유일하게 낡는 것(→ 2.4절). 유일하게 눈만 낮출 수 있다.
- **정적** — **호라이즌 맵(horizon)**. 셀에서 8방향으로 첫 장애물까지의 거리, 두 높이. 순수 기하라 진영 무관·영원.
- **파생** — **위험도(exposure)**. 저장하지 않는다. `직접(라이트의 그림자) + 앰비언트(호라이즌 × 확산 경계도)`.

### 2.2 위험도에 타이머가 없는 이유 (P143)

옛 위험 지도의 30 s 반감기는 **가짜**였다. 위험도는 캐시이고, 캐시는 **시간이 지나서** 만료되는 것이 아니라 **입력이 바뀌어서** 만료된다. 여기서 입력은 라이트다 — 라이트가 움직이거나 죽으면 그 라이트가 칠한 셀은 전부 무효여야 하고, 라이트가 그대로면 셀도 그대로여야 한다. 그래서 셀에는 **값이 아니라 태그**(`FLit{Light, Generation, Stance}`)가 들어가고, 읽을 때 태그의 세대가 라이트의 현재 세대와 같을 때만 유효하다(`IsLitValid`, `SoldierSituationField.cpp:551-555`). 라이트 해제는 세대를 1 올릴 뿐 셀을 걷지 않는다(`ReleaseLight`, `:572-582`).

> 타이머를 주면 이것은 방금 대체한 것이 된다 — "모든 병사의 추측을 30초 노출로 찍은 사진". (`SoldierSituationField.h:64-69`)

### 2.3 라이트의 주인은 "사람"이 아니라 "목격"이다 (사용자 지적, P145)

첫 설계는 라이트를 적 액터에 달려 했다. 사용자가 두 번 바로잡았다:

1. **적 액터에 달면 전지적이다.** 적이 벽 뒤로 가도 라이트가 따라간다 — 그건 우리가 모르는 정보다.
2. **"어떤 사람에 대한 믿음"도 아니다.** 라이트는 **"t초에 저 자리에 누군가 있었다"**는 **목격(sighting)**이다. 이름이 없다. 그래서 왼쪽 벽 뒤로 숨은 사람과 오른쪽에서 나온 사람이 같은 사람인지 필드는 **묻지 않는다** — 병사도 모르니까.

데이터는 여전히 복셀(셀)에 있다. 라이트가 **소유하는 것은 저장이 아니라 무효화**(세대 태그)다.

### 2.4 경계도는 0이 아니라 "모름"을 향해 낡는다 (P149)

`AgedPresence`(`:320-332`): `Lerp(Prior, Cell.Presence, 0.5^(age/PresenceHalfLifeSeconds))`. 한 번도 관측 안 된 셀은 `Prior = UnknownPresence 0.5`를 읽는다. 낡는 것은 적이 아니라 **관측**이다 — "봤다"도 "비었다"도 시간이 지나면 같은 곳, **모름**에 도달한다. 0을 향해 낡히면 한 번 훑은 복도가 경기 끝까지 "비어 있음"으로 남는다.

귀결 하나: **양의 증거는 사전값보다 커야 한다.** 총성 라이트의 밝기 `HeardPresence 0.75`는 0.5보다 크게 잡아야 "들었다"가 "안 봤다"보다 뭔가를 바꾼다(`SoldierPerception.h:425-435`).

### 2.5 사용자가 분리해 달라고 한 숫자 셋

| 숫자 | 뜻 | 값 |
|---|---|---|
| **라이트 수** | 목격 수만큼, 상한 | `MaxLights 16`/진영, 초과 시 가장 어두운 것 퇴출(추적 중이면 얼리고 나감) |
| **틱당 다시 그리는 라이트** | 섀도우 예산 | `ShadowRaysPerTick 96` = 48방향 × 2줄 → 새 라이트 하나가 예산 한 틱 분량(컴포넌트 틱 순서에 따라 나타난 뒤 **1~2틱**, `SoldierFieldSettings.h:186-192`)에 완성. 무효한 것 중 **밝은 순, 동률이면 추적 중 > 얼림** |
| **셀당 슬롯** | 한 셀을 비추는 라이트 기록 수 | `LitSlots 3` — 읽기가 max라 **무손실**(더 어두운 라이트는 답을 못 바꾼다, `LightCell` `:861-880`) |

---

## 3. 데이터 구조 [A]

전부 `SoldierSituationField.h:189-320`(09-18 판), `private`. 크기는 **계산값**(패딩 포함, `FVector`는 double 3개 = 24 B).

| 구조체 | 필드 | 크기(대략) | 비고 |
|---|---|---|---|
| **`FLit`** | `uint16 Light`, `uint16 Generation`, `uint8 Stance`(0 없음 / 1 기립 보임 / 2 웅크려도 보임) | 5 B → 6 B 정렬 | 한 라이트가 한 셀에 대한 주장. 값이 아니라 **태그** |
| **`FCell`** (레벨 0) | `float Presence`, `float PresenceTime`(−1e9 = 없음), `FLit Lit[3]`, `float GroundZ` + `bool bGroundResolved`, `float CornerZ[4]` + `bool bCornersResolved` | ≈ 56 B (+`TMap` 키 8 B·해시 ≈ 80 B 실효) | 지면 Z는 **셀이 한 번 정하고 유지**(지형처럼 정적). 모서리 4점은 오버레이가 처음 칠할 때. `.h:202-217` |
| **`FCoarseCell`** (레벨 1+, **09-18 신규**) | 집계 `Presence/PresenceTime`(자식 중 **지금 가장 강한 것의 원값+원시각** — 한 번만 낡게) · `LitStand/LitCrouch`(자식 **평균**) · `LitTime` · `RefreshedTime` · `bDirty` / 잔여물 `ResPresence/ResPresenceTime/ResLitStand/ResLitCrouch/ResLitTime/ResChildren` / `GroundZ`(오버레이용) | ≈ 64 B | **아무도 직접 쓰지 않는다** — 레벨 0 쓰기가 `bDirty`만 세우고 다음 읽기가 재집계. 잔여물 = 접혀 사라진 자식들이 알던 것. `.h:224-245` · 16절 |
| **`FLight`** | `FVector Eye`(지면+`LightEyeHeightCm`), `float GroundZ/Brightness/RadiusCm/BornTime/LastSeenTime`, **09-18 `float ClearViewSeconds/LastClearTime`**, `uint16 Generation`, `bool bAlive/bFrozen/bShadowValid`, `int32 ShadowCursor` | ≈ 64 B | 세대는 **이동·해제 때만** 증가. `ShadowCursor`로 섀도우 패스가 틱을 넘긴다. `ClearView*`는 18절 |
| **`FLevel`** | `float CellSizeCm`, `TMap<FIntPoint, FCell> Cells`, **`TMap<FIntPoint, FCoarseCell> Coarse`** | — | 레벨 0은 `Cells`만, 레벨 1+는 `Coarse`만 쓴다. 레벨 0 = `CellSizeCm`, 이후 `× LevelScale` |
| **`FHorizon`** (레벨 0) | `uint8 Stand[8]`, `uint8 Crouch[8]`(장애물까지 **셀 수**, 0~255), `bool bBaked/bQueued` | 18 B | **진영 공유**(`Horizons` 하나, 레벨 0 키), 절대 안 낡고 안 다시 굽는다. 09-18부터 **퇴거는 된다**(16.4절) |
| **`FCoarseHorizon`** (레벨 1+, **09-18 신규**) | `float OpenStand[8]/OpenCrouch[8]`(자식 **평균 트임** 0..1) · `int32 Count` · `bool bDirty` / 잔여물 `ResOpenStand[8]/ResOpenCrouch[8]/ResCount` | ≈ 140 B | 기하라 **진영 공유**(`CoarseHorizons[L]`). `.h:261-271` |
| **`FSample`** (**09-18 신규**, 읽기 전용 모양) | `Presence/PresenceTime/LitStand/LitCrouch/LitTime/bHasOpen/OpenStand[8]/OpenCrouch[8]` | 스택 | "어느 레벨이든 땅에 대해 말할 수 있는 것"을 **한 모양**으로 — 읽는 쪽이 레벨을 몰라도 된다. `.h:310-320` |
| 컨테이너 | `Levels[3]`(진영 인덱스, Neutral은 빈 채) · `Lights[3]` · `Horizons` · `CoarseHorizons[L]` · `PendingBakes` · **09-18** `Anchors[3]`/`AllAnchors`(이번 틱 병사 발 위치) · `EvictionQueue[3][5]`/`HorizonEvictionQueue[5]`(키 스냅샷) · `DebugBatcher` · ~~`LastWriterLocation`~~(삭제) | | 전부 `mutable` — 읽기가 셀을 만들고 굽기 큐에 넣는다(단 `SampleLevel0`는 예외, P157) |

**희소 피라미드 vs 클립맵**: 처음엔 관찰자 중심 클립맵을 검토했으나 진영 하나에 관찰자가 여럿이라 중심이 없다. 대신 **레벨별 희소 해시**로 갔고 `LevelCount`/`LevelScale`/~~`DetailRangeCm`~~를 프로젝트 설정으로 뺐다(사용자 요청 — 에디터에서 조정). ~~`FirstLevelFor`(`:334-342`): 마지막 쓴 병사에서 `DetailRangeCm 3000` 안이면 레벨 0부터, 밖이면 레벨 1부터 쓴다.~~ → **09-18 정정: `FirstLevelFor`·`LastWriterLocation`·`DetailRangeCm`는 삭제됐다.** 쓰기는 **항상 레벨 0**이고 거친 레벨은 **아래에서 위로 집계**(밉)되며, 병사에게서 먼 레벨 0은 **퇴거**로 접힌다 — 16절. **라이트 섀도우·어두운 셀 탐색·직접 노출은 레벨 0에만** 있다(변함없음). ~~기본 `LevelCount 1`이라 지금 실제로 도는 것은 단일 격자다 — 다층은 미검증 [C-130].~~ → 기본 **`LevelCount 3`**(200/800/3200), LOD 링을 오버레이로 PIE 확인.

**지면 Z** (`GroundZAt`, `:344-374`): 내비메시 투영(`ProjectPointToNavigation`, 반높이 `GroundSearchCm 20000`) → 실패하면 `ECC_WorldStatic` 하향 트레이스 → 그것도 실패하면 점 자체의 Z.

**해상도가 바뀌면 전부 비운다** (`EnsureLevels`, `:254-297`): 키가 다른 땅을 뜻하게 되므로 셀·라이트·호라이즌 전부 리셋, `[Field] resolution changed` 경고 1줄. 1~2초 안에 다시 찬다.

---

## 4. 라이트의 생애 — 목격 → 추적 → 얼어붙기 → 강등 → 해제 [A]

`ReportSighting(Faction, Location, Certainty, RadiusCm, Now)` (`:584-741`)가 유일한 입구. 세 단계로 판정한다:

### 4.1 연속성 — 눈이 떠나지 않은 동안만 같은 사람 (P145)

```
후보: bAlive && !bFrozen && (Now − LastSeenTime) ≤ ContinuityWindowSeconds 0.5
도달 가능: DistXY(light, new) ≤ MaxTrackedSpeedCms 600 × gap + TrackToleranceCm 150
→ 가장 가까운 것을 Refresh
```

창이 0.5 s로 **짧은 것이 의도**다 — 트레이스 예산 때문에 몇 프레임 빠진 것을 메우는 창이지, **벽 뒤에서의 몇 초를 메우는 창이 아니다**. 창을 넘기면 멈춘 목격은 멈춘 자리에 **얼고**, 다음에 나타난 것은 **새 라이트**다 — 진짜 누구였든.

**왼쪽 벽 / 오른쪽 벽 예**: 적이 왼쪽 벽 뒤로 들어간다 → 0.5 s 뒤 라이트 A가 왼쪽 벽 앞에서 언다. 3 s 뒤 오른쪽 벽에서 누가 나온다 → 라이트 B 신설. **왼쪽 벽은 여전히 켜져 있다.** 병사는 둘 다에서 숨는다. 라이트가 사람을 따라갔다면 왼쪽 벽의 위험이 지워졌을 것이고, 그게 이 규칙이 막는 실수다(`SoldierFieldSettings.h:106-118`).

`Refresh`(`:604-630`): 반 셀(`HalfCell`) 넘게 움직였을 때만 세대 증가 + 섀도우 무효(몇 cm 옆은 같은 그림자, 다시 긋는 건 낭비). 밝기는 `max(현재 감쇠값, 새 확신)`, 반경은 새 확신이 기존 이상일 때만 갱신(더 흐린 목격이 이미 선명한 것을 흐리지 않는다). 흐린 라이트(`RadiusCm > SharpRadiusCm`)가 선명해지면 섀도우를 다시 긋는다.

### 4.2 병합 — 같은 창의 얼린 라이트 (`MergeRadiusCm 300`)

얼린 라이트에서 3 m 안의 새 목격은 그 라이트를 **제자리에서 다시 켠다**(`bFrozen=false` → `Refresh`). 그 창에 있던 놈이 다시 나온 걸 가능성이 높고, 아니라도 1 m 옆 라이트 둘은 같은 그림자를 두 번 긋는다.

### 4.3 신설 — 빈 슬롯 → 상한 아래 추가 → 가장 어두운 것 퇴출

`MaxLights 16`을 넘으면 `LightBrightnessNow` 최소를 고르고, **추적 중이었으면 얼려서 기억을 쓰고** 해제한다("퇴출이지 망각이 아니다", `:719-724`). 새 라이트는 **이전 점유자의 세대 + 1**을 물려받는다 — 슬롯 재사용 때 옛 태그가 새 라이트 것으로 읽히지 않게(`:728-733`).

### 4.4 얼어붙기 (`FreezeLight`, `:557-570`) — 눈이 떨어지는 순간 기억을 쓴다

`Tick`(`:94-101`)이 매 틱 창을 넘긴 추적 라이트를 얼린다. 그 순간 **한 번** 확산 경계도에 원반을 쓴다: `WritePresenceDisc(지면, max(RadiusCm, 셀), FrozenPresence 0.6 × 현재 밝기)`. 이것이 **긴 기억**이다 — 라이트 자체는 `LightHalfLifeSeconds 20`으로 수십 초 안에 꺼지고, 남긴 확산 흔적은 `PresenceHalfLifeSeconds 8`로 "모름"을 향해 낡는다. 둘 다 옳다: **"정확히 저기 있었다"가 "저 근처에 누가 있었다"보다 먼저 잊힌다.**

### 4.5 강등·해제

- 밝기 `Brightness × 0.5^((Now−LastSeen)/20)`가 `LightDeathThreshold 0.05` 아래 → `ReleaseLight`. 세대 +1이 전부고 셀은 안 걷는다.
- **`SharpRadiusCm 400`**: 이보다 흐린 라이트(귀로 들은 것 — 반경은 `max(300, 거리×0.25)`)는 **그림자를 안 긋는다**. 원 중심에서 그리면 아무도 안 서 있는 곳에서 선명한 벽 경계를 그리게 된다. 대신 경계도 채널에 **글로우**만(`GetPresence`, `:1221-1247` — 가우시안 σ = 반경) — "그쪽을 볼 이유"까지만이고, 위험은 앰비언트 항의 몫.

---

## 5. 섀도우 패스 — 라이트가 땅에 대해 증명하는 것 [A]

`CastShadow`(`:743-830`), `Tick`이 `:113-155`에서 예산 안에서 반복 호출.

- **48방향 × 2줄** (`ShadowRays 48`). 줄 하나는 **눈높이 수평**(평지에서 기립 몸 높이를 통과하다 뭔가에 막힌다), 하나는 **먼 끝에서 `GroundZ + CrouchTop/2`로 내려가는** 줄(낮은 벽에는 막히고 공터에는 안 막힌다). 라이트마다 `LightIndex × 0.37 rad` 각도 오프셋 — 인접 라이트 둘이 같은 방위에 같은 틈을 남기지 않게.
- 트레이스는 `ShadowChannel`(GameTraceChannel5 "Sight"), `ShadowRangeCm 6000`, **등록부의 모든 병사 몸을 무시**(몸은 엄폐가 아니다 — 아군이 사이에 서면 벽으로 읽고 물러난다).
- 막힌 곳까지만 `MarchRay`(`:402-428`, 셀 크기 보폭, `MaxMarchCellsPerRay 32`)로 걷고, **셀마다 그 광선이 그 셀 지면 위 몇 cm를 지나는가로 자세를 정한다** (P148):

```
Above = RayZ − Cell.GroundZ
Above < −GroundSlackCm(40) →  땅속: 아무것도 아님          ← 09-18 정정 (옛 −CrouchTop 110)
Above ≤ CrouchTop(110)     →  Stance 2 (웅크려도 보인다)
Above ≤ StandTop(175)      →  Stance 1 (서면 보인다)
Above > StandTop           →  아무것도 아님 (뒤 언덕으로 가는 선이지 이 셀의 몸에 대한 말이 아니다)
```

  같은 두 줄이 경사에서는 길이를 따라 **다르게 읽힌다** — 그래야 맞다. ~~⚠ 브리핑의 "지면 아래는 없음"과 달리 코드는 `−CrouchTop`까지는 Stance 2로 친다(`:813-816`) — 셀 지면 캐시와 실제 지형의 오차 흡수로 읽힌다 **(추정)**.~~ → **09-18 정정**: 이 문서 작성 세션의 지적대로 "셀 지면보다 110 cm **아래**를 지나는 광선 = 웅크려도 보임"은 틀린 판정이었다. 이제 `GroundSlackCm 40`(`SoldierSituationField.cpp:67`, 상수)까지만 허용 — 셀 지면은 **중심**의 값이라 경사에서 표본이 조금 낮을 수 있는 만큼만. 그보다 아래는 트레이스가 지형에 막혔을 자리라 **캐시 오차지 사선이 아니다**(`:1467-1477`).
- `LightCell`(`:832-881`): 같은 라이트·같은 세대면 자세만 위로 갱신 → 무효 슬롯 재사용 → 셋 다 유효하면 **더 어두운 것만** 교체.
- 패스가 틱을 넘긴다: `ShadowCursor`에 방향 인덱스가 남고, 다 돌면 `bShadowValid=true`. **얼린 라이트의 그림자는 한 번 긋고 해제 때까지 유효** — 조용한 순간엔 이 루프가 할 일이 없다.
- **거리 감쇠** (`LightFalloff`, `:537-549`): `LightFullRangeCm 3000`까지 1, `LightMaxRangeCm 9000`에서 0으로 선형. 300 m 밖 사선과 30 m 사선은 같은 위험이 아니다.

**직접 노출 읽기** (`DirectAt`, `:1056-1090`): 셀의 유효 태그 중 `밝기 × 감쇠`의 **max**. Stance ≥ 1이면 Standing에, ≥ 2면 Crouched에도.

---

## 6. 앰비언트 — 호라이즌 맵과 안 본 땅 [B, 빌드 전]

균일한 사전값(안 본 땅 = 0.5)에 대한 적분은 정확히 **"이 땅이 얼마나 트여 있는가"**다 — 앰비언트 오클루전. 광장은 사방에서 밝고, 동굴은 어둡고, 복도는 길이 방향으로 밝고 가로로는 어둡다. 그리고 **내 눈으로 한 방향을 훑으면 그 방향만 어두워진다.**

### 6.1 호라이즌 굽기 (`BakeHorizons`, `:908-974`)

- 셀당 **8방향 × 2높이**(`HorizonStandHeightCm 135` / `HorizonCrouchHeightCm 70`) = 16 트레이스, `HorizonRangeCm 4000`. 결과는 **막힌 거리를 셀 수로**(uint8). 몸 무시, `ShadowChannel`.
- **지연 굽기**: `HorizonFor`(`:885-906`)가 **읽기마다** 안 구운 셀을 큐에 넣는다. 큐는 **최신 우선**(`Pop`) — 병사 발밑이 지나간 땅보다 먼저. `HorizonBakesPerTick 8`셀 = 128 트레이스/틱, **한 번 구우면 영원**(기하라 진영 공유, 안 낡음). → **09-18 단서**: "영원"은 **안 다시 굽는다**는 뜻이고, **모든 병사에게서 먼 호라이즌은 퇴거된다**(부모 `FCoarseHorizon`에 평균 트임으로 접힘, 16.4절). 그 땅에 누가 돌아오면 다시 굽는다. 굽기 큐에 있는 동안은 안 접힌다.
- 오버레이가 시야의 모든 셀을 `TouchCell`하고 `AmbientAt`를 부르므로 **오버레이를 켜면 관찰자 주변이 굽기 속도로 채워진다**(`:1341-1345`).

### 6.2 앰비언트 적분 (`AmbientAt`, `:992-1054`)

```
방향 d마다:
  OpenStand  = min(1, Stand[d] / RangeCells)      ← 4000 cm 트이면 1
  OpenCrouch = min(1, Crouch[d] / RangeCells)
  Presence   = max(확산경계도(Foot + d × Open/2), 확산경계도(Foot + d × Open))   ← 반·끝 두 표본
  StandSum  += Presence × OpenStand ;  CrouchSum += Presence × OpenCrouch
Standing = clamp(AmbientWeight 1 × StandSum / 8) ;  Crouched = 동
OutDirection = argmax(Presence × OpenStand)
```

- **확산 경계도만**(`DiffusePresenceAt`, 라이트 글로우 제외). 라이트는 이미 그림자로 직접 기여하므로 여기에 또 넣으면 같은 목격이 같은 땅을 **두 번** 비춘다.
- 트인 **길이**가 중요한 이유: 40 m 트인 복도에는 광장만큼의 적이 들어갈 수 있고, 2 m 트인 방향에는 거의 없다.
- `AmbientWeight 1`이면 "안 훑은 광장(사전값 0.5, 사방 트임)에 서기"가 "확인된 적의 정면"의 **절반**쯤 — 한 번도 안 본 건물에 들어갈 때의 느낌이 그 정도라는 판단(`SoldierFieldSettings.h:242-249`).

### 6.3 합성

`GetExposureByStance`(`:1094-1113`): `Standing = clamp(직접 + 앰비언트)`, `Crouched = 동`, `Standing = max(Standing, Crouched)`. 렌더러가 직접광과 앰비언트를 더하듯 더한다 — 출처가 서로소라 이중 계산이 없다.
`GetExposure`(`:1115-1125`) = `max(Crouched, 0.5 × Standing)` — 엄폐 스윕이 자기 트레이스로 매기는 것과 같은 눈금(웅크려도 보이면 1, 서야만 보이면 0.5).

---

## 7. 읽기 API — 누가 무엇을 읽는가

| API | 반환 | 읽는 쪽 | 상태 |
|---|---|---|---|
| `GetExposure(F, Foot, Now)` | 0..1 자세 눈금 | `SoldierCover::ScorePosition` Danger 항(`SoldierCover.cpp:1093-1095`, **눈 0일 때만**, `DangerWeight 0.8`) · `EvaluateRoute` 경로 표본(`:875-881`, `× SweepActivity`, live와 max) · `FindDarkestCells` 내부 | [A] PIE |
| `GetExposureByStance(F, Foot, Now, Standing, Crouched)` → **`bool`** (09-18 오후) | 자세별 · **false = 아는 것이 없음**(셀 없음 ∥ 레벨 0 호라이즌 미굽기 — 그때 둘 다 0 또는 라이트 몫만) | `SoldierCover::EvaluatePosition` **눈 0 분기**(09-18 판 `SoldierCover.cpp:844-893`) → **false면 노출 1·은폐 불가**(22절) / true면 `HiddenThreshold 0.25`로 셋으로 가른다: Standing<thr → 은폐·자세 0 / Crouched<thr → 낮은 벽·자세 1·Exposure 1 / 그 외 → 공터·CanHide=false | [A] PIE |
| `GetMostExposedDirection(F, Foot, Now, OutPoint, PreferDir=null, PreferWeight=0, ArcYawDeg=null, ArcHalfWidthDeg=180)` (09-18 오후 파라미터 추가) | 트인 끝점 · **편향**(`Score ×= 1 + w·dot`)과 **부채꼴 제한**(선택만, 적분은 그대로) | `SoldierCover::BeginSweep`(09-18 판 `:1423-1501` — **눈 유무와 무관하게 항상**, Rush만 예외; 이동 중 진행/굽이 방향 편향 · 정지 Approach 앵커 편향 · 정지 섹터 arc) → `WatchPoint` → `SoldierEngagement`(`SoldierEngagement.cpp:1275-1328`, **볼 곳 > 섹터 중심**, `GetAimPoint()`/`IsScanning()` 발행) · 오버레이 화살표(`:2414-2447`, arc만) | [A] PIE (21절) |
| **`GetStaleVantage(F, Foot, Now, StaleSeconds)`** (09-18 오후 신설) | 0..1 — 기립 높이로 내려다보는 **낡은** 땅의 비율 | `SoldierCover::PatrolCost`(`:203-220`) → `TaskCost` Hold 반경 안 · `ASoldierObjective` 수비 반경 안, **눈 0일 때만** | [A] PIE (20절) |
| `FindDarkestCells(F, Centre, Radius, Now, Max, Out)` | 어두운 셀 중심(지면 Z) | `SoldierCover::AddFieldCandidates`(09-18 판 `:390-421`, `FieldCandidateCount 6`, 내비메시 투영 후 후보) — **키 하나 걸러**(2셀 = 4 m 간격), 안 구운 셀은 **큐에 넣고 건너뜀**(모르는 땅을 "어둡다"고 보고하면 정반대 효과) · **09-18 오후: `bSolid` 셀도 건너뜀**(`:1998-2002`, 22절) | [A] PIE |
| `GetPresence(F, Loc, Now)` | 확산 ∨ 글로우 | (오버레이 외 소비자 없음) | [A] |
| `GetSecondsSinceObserved` | 초 | (미사용) — 09-18 오후 `StalenessAt`(private, `:1704-1713`)이 같은 질문을 `GetStaleVantage` 안에서 한다 | [A] |
| `MarkClear / MarkClearAlongRay` | — | 쓰기(8절) | |
| `GetDebugFaction / GetCellCount / GetLightCount` | | 오버레이 | |

**09-18 — 읽기는 `SampleFinest`를 걷는다** (`SoldierSituationField.cpp:884-902`): `GetExposureByStance` · `GetMostExposedDirection` · `DiffusePresenceAt` · `GetSecondsSinceObserved` · `DescribeGround`가 전부 레벨 0 → 1 → 2 순으로 "이 땅에 대해 뭔가 아는 가장 미세한 레벨"의 `FSample`을 받는다. 레벨 0 셀이 퇴거로 접혔으면 부모의 집계+잔여물이 답한다(16.3절). 3단계 표의 [B]는 전부 **09-18 PIE 확인**으로 [A]가 됐다(단 `FindDarkestCells`·눈 0 자세는 오버레이가 아니라 거동으로 판정해야 하므로 [C-133]·[C-135] 판정은 그대로 열려 있다).

**`IsRushing()`** (`SoldierCover.cpp:332-341`, 배정 `Speed == Rush`)이면 **필드 후보·필드 자세·볼 곳을 전부 건너뛴다** — 명령은 "있을지도 모르는 것"에 대한 조심을 끄는 것이지 사실(기억 속 적)까지 지우진 않는다. 경로 위험의 필드 읽기(`EvaluateRoute`)는 Rush여도 그대로다.

**서버 전용**: 쓰는 쪽(`SoldierSight` `:201`, `SoldierCover` `:1321`)이 P5 `HasAuthority` 게이트 뒤에 있어 필드는 서버 지식이다. 서브시스템 자체는 클라이언트에서도 틱하지만 빈 채로 돈다 **(추정)**.

---

## 8. 쓰는 쪽 — 그리고 일부러 안 쓰는 쪽 [A]

| 출처 | 호출 | 쓰는 것 | 위치 |
|---|---|---|---|
| **눈 — 목격** | `MarkClearAlongRay(눈→보인 점)` **먼저**, 그 다음 `ReportSighting(발, 1.0, SightingRadiusCm 100)` | 사선의 셀을 비움 + 선명한 라이트 | `SoldierSight.cpp:288-301` |
| **눈 — 콘 스윕** | `MarkClearAlongRay(눈→막힌 점)` | 비움(**사전값을 물리는 유일한 것**) | `SweepCone` `:142-198`, 2줄/틱 × 21줄 라운드로빈, 120°, `ConeSweepRangeCm 4000` |
| **눈 — 빈 땅 확인** | `MarkClearAlongRay(눈→기록 예상점)` | 비움 | `:380-395` (부정 증거 P133과 같은 트레이스) |
| **귀 — 총성** | `ReportSighting(추정점, HeardPresence 0.75, 반경)` | 흐린 라이트(그림자 없음, 글로우만) | `SoldierPerception.cpp:355-368` |
| 얼어붙는 라이트 | `WritePresenceDisc` | 확산 경계도 원반 | `FreezeLight` (내부) |

**순서가 중요하다** (P151): 목격에서 비우기를 **나중에** 하면 사선의 마지막 셀 — 적이 서 있는 셀 — 이 지워져 방금 켠 라이트의 근거를 없앤다(`SoldierSight.cpp:294-297`).

**`MarkClearAlongRay`**(`:504-523`): 광선 표본의 Z가 셀 지면 **±`BodyBandCm 200`** 안일 때만 그 셀을 비운다 — 언덕에서 계곡 5 m 위를 지나간 광선이 "계곡을 확인했다"가 되지 않게. ~~`WriteClear`(`:486-502`)는 **레벨 0만 0으로**, 거친 레벨은 시각만 찍는다(거친 셀 하나에 미세 셀 여럿 — 하나 비었다고 나머지가 빈 게 아니다).~~ → **09-18 정정**: `WriteClear`(09-18 판 `:610-619`)는 레벨 0만 쓰고 **`MarkDirtyUp`으로 부모를 더럽힐 뿐** 거친 레벨에 아무것도 직접 쓰지 않는다 — 거친 레벨은 다음 읽기에서 자식들의 **max**로 재집계되므로 "하나 비었다고 나머지가 빈 게 아니다"는 집계 규칙이 보장한다(16.2절). **09-18 추가**: 같은 함수가 끝에 **`ContradictLightsAlong`**(`:641`)을 부른다 — 뚫린 선이 지나간 **얼린 라이트**에 "봤는데 없었다" 시간을 적립(18절).

### 일부러 안 쓰는 쪽

| 안 쓰는 곳 | 이유 | 주석 위치 |
|---|---|---|
| **엄폐 부채꼴 광선**(`AddFanCandidates`) · 몸 밴드 · 경로 표본 | 이 광선은 "이 병사가 어디 설 수 있는가"에 대한 것이지 땅의 사실이 아니다. 45명이 매 스윕 자기 동네를 max-merge로 쓰던 것이 1.2절의 사진이다. **필드는 목격에서, 목격당 한 번** 땅을 비춘다 | `SoldierCover.cpp:506-510` |
| **제압 근접탄**(`ApplyNearMiss`) | 지나간 탄은 **사수가 어디 있는가**의 증거지 이 땅의 증거가 아니다. 사수는 귀(총성)나 눈으로 필드에 들어오고 필드가 거기서 이 땅을 비춘다 — 같은 사실을 정직하게 도달. "이 자리는 노출"을 직접 쓰는 것은 **목격에 귀속 못 하는 유일한 노출 쓰기**라 영영 못 지운다 | `SoldierSuppression.cpp:135-139` |
| 총성 → `MarkFiredUpon` | 위험 지도와 함께 소멸 | — |

---

## 9. 디버그 오버레이 [A, PIE 확인] — ⚠ 09-18 v2로 대체된 부분 있음 → 17절

> 아래는 **09-17 v1** 서술이다. 09-18에 그리기 경로(`DrawDebugMesh` → 자체 배처) · 신선도 표현(크기 → **불투명도**) · 링(`Level -1`) · 기본 반경(4000 → **12000**) · 셀 상한(4000 → **6000**) · 라이트 색(밝기 → **출처**)이 바뀌었다. 채널 뜻·진영 추적·`GetObservedSoldier`는 그대로. **17절이 최신.**

**월드당 1회**다 — 병사당이 아니라(P146). 서브시스템이 `UTickableWorldSubsystem`이 된 이유. 진영의 그림이라 "누구 편을 보여줄까"를 병사 없이 답해야 했고, 그 답이 `SoldierDebug::GetObservedSoldier`(`SoldierDebugDraw.cpp:57-114`): **필터 이름**(`SoldierLab.Debug.AI.Filter`) → **관전 폰이 따라가는 병사**(`ASoldierObserverPawn::GetFollowedSoldier()`, 빙의가 아니라 추적이라 "로컬 조종 폰"으로 물으면 영영 관전 폰만 나온다) → **조종 중/뷰타겟 병사**.

| cvar | 기본 | 뜻 |
|---|---|---|
| `SoldierLab.Debug.Field` | 0 | 1이면 칠한다 |
| `SoldierLab.Debug.Field.Channel` | **3** | 3 = **R 위험도 / G 경계도** 합성 · 0 위험도(빨강) · 1 경계도(초록) · 2 신선도(파랑) · 4 **앰비언트만(보라)** |
| `SoldierLab.Debug.Field.Faction` | **−1** | −1 관찰 중인 병사의 진영을 따라감(Tab으로 다음 병사 → 그림도 따라감) · 0 Friendly · 1 Hostile |
| `SoldierLab.Debug.Field.Level` | 0 | 어느 해상도 |
| `SoldierLab.Debug.Field.RadiusCm` | 4000 | 관찰자 주변 얼마나 |

**읽는 법** (채널 3):

- **빨강** = 여기 서면 맞는다(직접+앰비언트). **초록** = 저기 누가 있을지 모른다(사전값 0.5면 중간 초록). **노랑** = 누가 보고 있을 살상 지대. **거의 검정** = 이쪽이 훑어서 안전한 땅.
- **크기 = 신선도**: 방금 관측한 셀은 `DebugFillMax 0.8`, `DebugStaleSeconds 30` 지나면 `DebugFillMin 0.1`로 **줄어든다**. 페이드가 아니라 축소인 이유: 낡은 셀도 **거기 있다는 것이 보여야** 한다 — "한참 전에 누가 있었다"는 정보지 잡음이 아니다. `DebugAlpha 0.35` 고정(색이 값, 불투명도는 가독성).
- `DrawDebugMesh` 채움 사각형, 네 모서리는 **각자의 지면**(`CornerZ`, 처음 칠할 때 셀이 한 번 정함) — 언덕에서 평평한 사각형은 묻히거나 뜬다. `DebugLiftCm 12`.
- **라이트**: 눈 위치 **구**(18 cm) + 지면까지 **핀**(굵으면 추적 중 2.5, 얇으면 얼림 0.5) + 지면 **링**(반경 = 목격 반경, 최소 40). 색은 밝기에 따라 주황→노랑.
- **화면 헤더** `[Field] HOSTILE | R 위험도 / G 경계도 | lights 2 followed + 5 frozen | 1834 cells | level 0 (following|pinned)`.
- 셀 상한 `MaxDebugCells 4000`. 오버레이는 시야의 셀을 **만든다**(굽기 큐 포함) — 관찰 중 셀 수가 오르는 것은 정상.

---

## 10. 프로젝트 설정 — 값 표 (전부 [C], `SoldierFieldSettings.h`)

Project Settings → Game → **SoldierLab Situation Field** (`config=Game, defaultconfig` → `DefaultGame.ini`). **하나도 재지 않았다.** 사용자는 오버레이를 보며 잡을 생각이고, 오버레이는 그러라고 만들었다(`:20-21`).

| 분류 | 값 | 기본 | 한 줄 | [C] |
|---|---|---|---|---|
| Resolution | `CellSizeCm` | 200 | 몸 + 한 걸음. 바꾸면 필드 비움. **09-18** cvar `SoldierLab.Field.CellSizeCm`(0 = 설정값)이 런타임 오버라이드 | C-130 |
| | `LevelCount` | ~~1~~ **3** (09-18) | 레벨 0 = 진실, 1+는 집계(밉). 클램프 1~5 | C-130 → **C-140** |
| | `LevelScale` | 4 | 200→800→3200 | C-140 |
| | ~~`DetailRangeCm`~~ | ~~3000~~ | **09-18 삭제** — 쓰기 레벨 선택은 없어졌다 | — |
| | **`DetailRadiusCm`** (09-18) | **8000** | 진영 병사 **아무나**에게서 이 안이면 레벨 0 유지, 밖이면 부모에 접고 해제. 레벨 L은 `× LevelScale^L`(80 m / 320 m / 코스트는 안 접음). **콘 스윕 40 m·섀도우 60 m보다 커야** 쓰는 중에 안 접힌다(`SoldierFieldSettings.h:58-72`) | **C-140** |
| | **`EvictionCellsPerTick`** (09-18) | **1024** | 진영당 틱당 거리 검사 수. 접힘 지연 = 셀 수 ÷ 이 값(틱) + 간격 | C-140 |
| | **`EvictionIntervalSeconds`** (09-18) | **2** | 키 스냅샷 주기(큐가 빈 뒤) | C-140 |
| | **`CoarseRefreshSeconds`** (09-18) | **1** | 거친 셀 재집계 TTL — 자식이 안 바뀌어도 라이트는 낡으니 이만큼마다 | C-140 |
| Presence | `PresenceHalfLifeSeconds` | 8 | 사람에 대한 것이라 빨리 낡는다 | C-130 |
| | `UnknownPresence` | 0.5 | **문을 열고 들어가느냐 박차고 들어가느냐의 차이** | C-130 |
| | `BodyBandCm` | 200 | 사선이 셀 지면 ±이 안일 때만 "봤다" | C-130 |
| Lights | `LightHalfLifeSeconds` | 20 | | C-131 |
| | `LightDeathThreshold` | 0.05 | | C-131 |
| | **`ClearViewHalfLifeSeconds`** (09-18) | **4** | 얼린 목격을 **빈 채로 보고 있는 시간**에 대한 두 번째 반감기. 삭제가 아니다 — 18절 | **C-142** |
| | `ContinuityWindowSeconds` | 0.5 | 이보다 긴 공백 뒤는 다른 사람 | C-131 |
| | `MaxTrackedSpeedCms` | 600 | | C-131 |
| | `TrackToleranceCm` | 150 | | C-131 |
| | `MergeRadiusCm` | 300 | 얼린 라이트 재점등 | C-131 |
| | `MaxLights` | 16 | 진영당. 오랜 전투가 유령 50개로 지도를 붉게 하는 것 방지 | C-131 |
| | `SharpRadiusCm` | 400 | 이보다 흐리면 그림자 없음 | C-131 |
| | `FrozenPresence` | 0.6 | 얼 때 확산 원반에 쓰는 밝기 비율 | C-131 |
| Shadow | `LightEyeHeightCm` | 160 | 목격 지면 + 이 값 = 라이트 눈 | C-132 |
| | `CrouchTopCm` / `StandTopCm` | 110 / 175 | 셀별 자세 판정 문턱 | C-132 |
| | `ShadowRays` | 48 | 방향 수(×2줄) | C-132 |
| | `ShadowRaysPerTick` | 96 | 월드 전체 예산 | C-132 |
| | `ShadowRangeCm` | 6000 | 그 밖은 앰비언트만 | C-132 |
| | `LightFullRangeCm` / `LightMaxRangeCm` | 3000 / 9000 | 거리 감쇠 | C-132 |
| | `ShadowChannel` | GameTraceChannel5 | "Sight" | — |
| | `MaxMarchCellsPerRay` | 32 | 긴 광선이 짧은 것보다 비싸지 않게 | C-132 |
| | `GroundSlackCm` (**상수**, `.cpp:67`) | 40 | 셀 지면보다 이만큼 아래까지는 경사 오차로 봐 준다(옛 −110) | C-140 |
| Ambient | `HorizonRangeCm` | 4000 | 트임 탐색 거리 = 완전 트임 기준 | C-133 |
| | `HorizonStandHeightCm` / `HorizonCrouchHeightCm` | 135 / 70 | 굽는 두 높이 | C-133 |
| | `HorizonBakesPerTick` | 8 | ×16 트레이스 | C-133 |
| | `AmbientWeight` | 1 | 직접 대비 앰비언트 비중 | C-133 |
| | `HiddenThreshold` | 0.25 | 엄폐 층이 눈 0일 때 "숨음"으로 읽는 문턱 | C-133 |
| Debug | ~~`MaxDebugCells` / `DebugAlpha` / `DebugFillMin` / `DebugFillMax` / `DebugStaleSeconds`~~ | ~~4000 / 0.35 / 0.1 / 0.8 / 30~~ | 09-17 v1 — `DebugAlpha`·`DebugFillMin/Max`는 **삭제** | ~~C-139~~ → C-141 |
| Debug (09-18) | `MaxDebugCells` | **6000** | 리프레시당 셀 상한. 링이 넘치면 **대칭으로 줄이고** 헤더 CAPPED. 50 cm 셀 × 25 m 링만 8천 | **C-141** |
| | `DebugRefreshSeconds` | 0.1 | 오버레이 재그리기 주기 — 그 사이는 정적 | C-141 |
| | `DebugDetailRadiusCm` | 2500 | 오버레이 클립맵: 레벨 0은 이 안, 레벨 L은 `× LevelScale^L` | C-141 |
| | `DebugFill` | 0.7 | 사각형이 셀을 채우는 비율 — **고정**(링이 있으니 크기가 곧 레벨) | C-141 |
| | `DebugAlphaMin` / `DebugAlphaMax` | 0.1 / 0.8 | **불투명도 = 신선도**, 0에는 절대 안 간다 | C-141 |
| | `DebugStaleSeconds` | 30 | 이 나이에 `AlphaMin` 도달 | C-141 |
| | `DebugLiftCm` (상수, `.cpp:58`) | 12 | 지형 위 띄우기 | C-141 |
| cvar | `SoldierLab.Debug.Field.RadiusCm` | ~~4000~~ **12000** | 관찰자 주변 그리는 반경 | C-141 |
| | `SoldierLab.Debug.Field.Level` | ~~0~~ **−1** | −1 = 링(클립맵) · 0+ = 그 레벨만 | — |

컴포넌트 쪽 신설·변경값(`SoldierSight` 콘 스윕 · `SoldierPerception::HeardPresence` · `SoldierCover::FieldCandidateCount` · 골든앵글)은 `ai/2026-09-17_infiltration_and_unknown_ground.md` 7절 → [C-134] [C-135].

**09-18 오후 — `USoldierFieldSettings`에 새 값은 없다.** 이날 오후의 값(`PatrolWeight`/`PatrolStaleSeconds`, `WatchTravelBias` 등)은 전부 **배정·존·목표·엄폐 컴포넌트** 쪽이다 → `ai/2026-09-18_patrol_scan_and_move_robustness.md` 10절 · [C-148]~[C-153]. 필드 안의 상수 둘: `MarkClearAlongRay`의 `WidenFromCm = 셀 × 8`(`.cpp:638`) · 오버레이 화살표 250 cm.

---

## 11. 성능 — 비용 비교 [B, 계산값]

| | 옛(위험 지도 + 가상 관찰자 시절, 병사당) | 새(월드 전체) |
|---|---|---|
| 노출 쓰기 | 부채꼴 24줄 × 2높이 × 눈 3(가상 포함) + 몸 밴드 + 경로 표본 → 스윕 라운드당 **≈ 6,500 광선** 상당의 셀 쓰기, 45명 각자 | **0** (엄폐 광선은 필드를 안 쓴다) |
| 그림자 | — | `ShadowRaysPerTick` **96** 트레이스/틱, 진영 둘 합쳐서. 조용하면 0 |
| 호라이즌 | — | `HorizonBakesPerTick 8` × 16 = **128** 트레이스/틱, 셀당 **한 번뿐** |
| 콘 스윕 | — | 병사당 **2** 트레이스/틱(`ConeSweepTracesPerTick`) → 45명 = 90 |
| 읽기 | 해시 조회 | 해시 조회 + 앰비언트 8방향 × 2표본 = 16 조회 (09-18: 레벨 0에 없으면 레벨 1·2 조회 + 더러우면 재집계 `Scale²` = 16 자식) |
| 오버레이 | 병사당 그리기 | 월드당 1회, ~~`MaxDebugCells 4000` 사각형~~ → **09-18** 0.1 s마다 ≤ 6000 사각형을 **색별 메시 수십 개**로, 그 사이 0 |
| **09-18 퇴거** | — | 진영당 틱당 `EvictionCellsPerTick 1024` × 거리 비교(앵커 수 ≤ 15) + 호라이즌 1024. 접히는 셀당 `SampleLevel0` 1회 |
| **09-18 메모리** | 지나간 면적 전부(해제 없음) | **병사가 있는 자리에 묶임**: 진영당 레벨 0 ≈ π·(80 m)²/(2 m)² ≈ **5천 셀/병사 상한**(겹치면 덜), ≈ 80 B 실효 → 병사 15명이 완전히 흩어져도 **≈ 6 MB**. 레벨 1 ≈ 1/16, 레벨 2는 안 접혀 맵 전체 누적(3200 cm 셀이라 1 km² = 100개) |

**왜 메모리가 문제였고 계산은 아니었나** (09-18 사용자 질문 "복셀 GI처럼 클립맵 레벨을 두면?"): 이 필드는 **요구 주도·희소**다 — 매 틱 맵 전체를 도는 루프가 **하나도 없다**(섀도우는 라이트 예산, 굽기는 큐, 읽기는 병사가 묻는 점). 그래서 **계산은 레벨 크기와 무관**하다. 커지는 것은 **메모리 ∝ 지나간 면적**(09-17까지 셀을 해제하는 코드가 없었다)과 **호라이즌 굽기 ∝ 경로 길이**뿐. 사용자 시나리오가 "적군이 큰 레벨을 가로질러 후퇴하며 싸운다"라 **꼬리(trail)**가 진짜 문제였고, 답은 계산 LOD가 아니라 **퇴거**다(P152). **실측 없음** → [C-138] 갱신 · 오버레이 fps 차이는 09-18 v2에서 사용자 체감 해결(4000 cm에서 깜빡임+fps 저하 → 12000 cm에서 정상, 수치 없음).

---

## 12. 폐기한 접근들

### 12.1 위험 지도 (`SoldierDangerMap`, 09-14 → 09-17 `p4 delete`)

셀당 세 시각(`MarkSeenStanding` / `MarkOpenGround` / `MarkFiredUpon`), 30 s 반감기, max-only. **증상**: 안 본 땅에 대해 침묵(1.1절) · 출처 없는 누적(1.2절) · 자리 항이 2 m 셀 때문에 좋은 엄폐를 옆 공터 값으로 벌함(P116 — 지금은 눈 0일 때만 읽어 회피, `SoldierCover.cpp:1089-1095`). 문서 `ai/2026-09-14_danger_map_and_position_commitment.md`는 **superseded** — 그 문서의 나머지(모든 눈·그림자 스냅·머무름·표적 잠금)는 살아 있다.

### 12.2 가상 관찰자 (`AddUnknownWatchers`, 같은 날 도입 → 3단계에서 철회)

안 본 땅에 **상상의 눈**을 세우는 안: 8개 월드 방향을 20→30 m로 표본해 경계도가 가장 높은 땅에 최대 2개, 목표 쪽으로 편향, 가중 0.25, 기존 눈 기반 엄폐 기계에 먹였다. **거동은 됐다.** 그런데:

1. 적분을 **표본 2개의 몬테카를로**로 푸는 것이다 — 앰비언트가 같은 적분을 제대로 푼다.
2. 접촉이 없는 내내 **눈 3개 비용**으로 돌았다.
3. **최악**: 그 부채꼴 광선이 노출을 **공유 필드에 썼다** → 가설이 공유 사실이 되고, max-only 30 s 기억에 남아 "모든 병사 주위가 전부 붉음"(1.2절). **P144.**

3단계의 앰비언트(6절)가 대체 — "같은 적분을, 두 점 대신 제대로, 병사마다 추측하는 대신 진영이 공유"(`SoldierSituationField.h:80-82`).

---

## 13. 열린 문제 → `OPEN_ITEMS.md`

| # | 무엇 | 왜 열려 있나 |
|---|---|---|
| **[W74]** | **분대 스코프 필드** — 진영 공유는 근사. 정직한 버전은 "그 분대가 실제로 들은 목격만" | 라이트가 정확히 무전이 이미 나르는 단위(접촉 보고)라 구조는 준비됨(`SoldierSituationField.h:84-86`). L1이 생기면(W55) 붙인다 |
| **[W75]** | **내리막 과소 보고** — 두 줄 섀도우가 라이트보다 낮은 땅을 못 비춘다 | 수평 줄과 `CrouchTop/2`로 내려가는 줄뿐이라, 눈 아래로 크게 꺼진 계곡의 셀은 `Above > StandTop`으로 잘린다. 사면 아래 몸이 뻔히 보여도 안 켜진다 |
| **[W76]** | **선명 라이트의 페넘브라 없음** — 반경 100~400의 라이트가 점에서 그림자를 긋는다 | 목격 반경만큼 벽 경계가 흐려져야 맞다. 지금은 `SharpRadiusCm` 이분법 |
| **[W77]** | **앰비언트가 높이 한 쌍(135/70)·8방향·표본 2** — Identity의 실측 높이(P115)와 무관, 재굽기 없음 | 문 열림·차량 이동 같은 기하 변화에 호라이즌이 안 따라온다(등록부 몸은 무시하니 병사·차량은 문제 없음, 정적 배치물만) |
| ~~**[W78]**~~ | ~~잔존 주석 정리~~ | ✅ **09-18 해결** — `SoldierCover.h:92`는 상황 필드 설명으로, `SoldierSuppression.h:101-105`는 과거형("used to be written into the side's danger map")으로 정리 |
| **[W79]** (09-18) | **거친 셀 잔여물의 lit이 라이트와 끊겨 있다** | 접힐 때 `ResLitStand/Crouch`는 그 순간의 직접 노출 **값**만 남고 세대 태그가 없다. 라이트가 죽거나 움직여도 잔여물은 `LightHalfLifeSeconds 20`으로만 꺼진다(`FoldInto` `:911-914` · `RefreshCoarse` `:828-830`). 레벨 0에서 P143으로 없앤 "타이머"가 코스 레벨엔 남아 있는 셈 — 경로 판단 해상도라 허용 |
| **[W80]** (09-18) | **접힘 지연과 "쐈다가 접히는" 셀** | 지연 = 큐 셀 수 ÷ `EvictionCellsPerTick` 틱 + `EvictionIntervalSeconds`. 그리고 병사에게서 70 m 떨어진 라이트는 `DetailRadiusCm 80 m` 안이라 그림자를 긋는데 `ShadowRangeCm 60 m`라 130 m까지 셀을 쓰고, 80 m 밖 셀은 **다음 패스에 접힌다** — 의도된 것(쓰기는 항상 레벨 0, 접기가 정리). 오버레이에서 깜빡임으로 보이면 [C-143] |
| **[W81]** (09-18) | **오버레이가 필드 메모리를 바꾼다** | `DrawDebug`가 시야의 모든 레벨 0 셀을 `TouchCell` + `HorizonFor`(굽기 큐), 거친 셀을 `FindOrAdd`한다(`:2087-2114`). 관찰자가 병사와 멀면 만든 셀은 퇴거가 도로 접는다. "디버그가 상태를 만든다"는 P7의 대가 — 헤더의 셀 수는 오버레이 켠 채로는 필드의 진짜 크기가 아니다 |
| **[W82]** (09-18) | **`FoldInto` 경계도 = max 의미론(선택)** | 잔여물 경계도는 자식과 기존 잔여물 중 **지금 더 강한 쪽**(`:916-924`) — 평균이 아니다. 자식 15개가 "비었음"이고 1개가 목격이면 블록은 목격을 대표한다. 집계(`RefreshCoarse`)도 max라 일관되지만, "블록 대부분이 확인됐다"는 정보는 잃는다. 필요해지면 count 가중 |
| ~~**[W83]**~~ (09-18) | ~~**`ContradictLightsAlong`이 들은 라이트를 안 거른다** — 주석과 코드 불일치~~ | ✅ **09-18 오후 해결** — 필터에 `RadiusCm > SharpRadiusCm` 제외 추가(저녁 판 `:1187`), 주석(`:1183-1186`)이 "처음엔 안 걸렀고 넓은 반경 때문에 가장 부정하기 쉬운 라이트가 됐다"를 남김. 옛 서술: ~~`:1164-1165` 주석은 "a heard one is too vague…"이라 하지만 필터는 `bAlive && bFrozen`뿐(`:1166`). 들은 라이트는 반경이 크니 `Reach = max(RadiusCm, 셀)`이 더 넓어져 오히려 더 잘 걸린다~~ |
| **[W88]** (09-18 오후) | **실제 레벨의 나무 콜리전이 `ShadowChannel`(Sight)을 Block하는가** | 안 막으면 호라이즌·섀도우·콘 스윕이 숲을 광장으로 읽는다(엄폐 트레이스 `CoverChannel`도 같은 문제). 시험 레벨은 큐브뿐이라 드러나지 않음 — `ai/2026-09-18_patrol_scan_and_move_robustness.md` 8절 |
| [C-130]~[C-133] [C-138] · **[C-140]~[C-143]** | 값·성능 | 10·11·16~18절 |

그 외 알려진 근사: ~~다층(`LevelCount > 1`)은 미검증이고~~ 라이트·후보 탐색은 레벨 0 전용 · `MarchRay`는 DDA가 아니라 셀 보폭 표본(대각선에서 셀을 건너뛸 수 있음) · ~~`FirstLevelFor`의 기준점이 "마지막 쓴 병사"라 여러 병사가 멀리 흩어지면 레벨 선택이 요동한다 **(추정)**~~ → 09-18 `FirstLevelFor` 삭제로 소멸 · 코스트 레벨(`LevelCount−1`)은 절대 안 접히므로 맵 전체에 누적된다(3200 cm 셀이라 무시 가능) · 오버레이의 거친 셀 지면 Z(`FCoarseCell.GroundZ`)는 중심 1점이라 언덕에선 평평한 사각형이 묻히거나 뜬다(의도 — "블록의 요약은 평평한 사각형이 정직하다", `:2210-2219`).

---

## 14. 검증 상태

| 단계 | 내용 | 상태 |
|---|---|---|
| 1 | 필드 자료구조 · 경계도 · 사전값 · 콘 스윕 · 스프린트 규칙 | ✅ 빌드 · PIE (사용자 "잘되는거같아") |
| 2 | 라이트(목격→추적→얼림→해제) · 섀도우 패스 · 세대 무효화 · 쓰기 정리(부채꼴·제압 제거) · `LogSoldierAI` 이사 | ✅ 빌드 · PIE (동) |
| 오버레이 | 월드당 1회 · 채널 · 진영 추적 · 라이트 핀/링 · 크기=신선도 | ✅ 빌드 · PIE (동) |
| **3** | **호라이즌/앰비언트 · `FindDarkestCells` 필드 후보 · 눈 0 필드 자세 · 볼 곳 · 가상 관찰자 철회 · Rush 우회** | ~~[B] 코드만 — 빌드 대기~~ → ✅ **09-18 빌드 · PIE** (사용자 "잘된다", "딱 내가 원하는 그림이 이제 나옴") — 단 거동 판정([C-133]·[C-135])은 오버레이가 아니라 로그로 봐야 하므로 열려 있음 |
| **09-18 LOD** | 밉(`FCoarseCell`/`FCoarseHorizon`) · 다중 앵커 퇴거 · `LevelCount 3` · `SampleFinest` 읽기 · 섀도우 앵커 게이트 · `GroundSlackCm` · `SoldierLab.Field.CellSizeCm` | ✅ **빌드 · PIE** — 링이 거리에 따라 눈에 띄게 거칠어짐, "기능적으로는 아주 잘 작동" |
| **09-18 오버레이 v2 (1차)** | 자체 배처 flush+refill · 색별 메시 · 클립맵 링 · 불투명도=신선도 · 반경 12000 | ✅ **빌드 · PIE** — 깜빡임·fps 저하 해소(체감) |
| **09-18 오버레이 v2 (2차)** | **50 cm 셀 시험이 드러낸 캡 버그(−X 반쪽만 그림) 수정 = 대칭 캡** · `MaxDebugCells 6000` · 라이트 시각 v2(불투명도=밝기·색=출처·흰 점) · 미굽기 파랑 · 헤더 2줄(`DescribeGround`) · 화살표 2.5 m | **[B] 빌드 전** |
| **09-18 부정 증거** | `ClearViewSeconds` · `ContradictLightsAlong` · `ClearViewHalfLifeSeconds 4` | ~~[B] 빌드 전~~ → ✅ **09-18 오후 빌드**(들은 라이트 제외 포함, 오전 미빌드분과 같은 묶음) — 핀이 빨리 흐려지는지의 판정 [C-142]는 열림 |
| **09-18 오후** | `GetStaleVantage` 순찰 · `GetMostExposedDirection` 편향/arc · `bSolid` · `GetExposureByStance` bool · `MarkClearAlongRay` 띠 · 오버레이 링 annulus/화살표 arc | ✅ **빌드 · PIE** — "잘 되는 거 같음. 이제 정상적이다"(적군 존 복귀 · REJECT 루프 없음 · 순찰 이동). 오전 미빌드분(대칭 캡·라이트 v2·헤더 2줄·파랑끼)도 같은 빌드에 포함 **(추정 — 사용자가 항목별로 확인하진 않음)** |
| 값 | `USoldierFieldSettings` 전부 + 컴포넌트 신설값 | [C] 하나도 안 잼 |

~~**다음 빌드 뒤 볼 것**: 채널 4(보라)로 앰비언트가 광장에서 밝고 골목에서 어두운가 · 접촉 없는 병사가 문간→문간으로 가는가(`[Cover]` 로그 `eyes=0`에서 `tried`에 필드 후보가 섞이는가) · 콘 스윕 뒤 초록이 검게 꺼지고 8 s 뒤 다시 중간 초록으로 돌아오는가 · 왼쪽 벽 라이트가 오른쪽 벽 라이트 생성 후에도 남는가(핀 2개).~~ → 앞 셋 중 그림은 09-18 확인, 거동(문간→문간)·핀 2개는 아직 명시 확인 없음.

**09-18 미빌드분 빌드 뒤 볼 것**: `SoldierLab.Field.CellSizeCm 50`으로 링이 **양쪽 대칭**으로 줄고 헤더에 `CAPPED`가 뜨는가 · 채널 3에서 관찰자 먼 곳(안 구운 셀)이 **파랑끼**로 보이다 굽히면 검정/색으로 바뀌는가 · 얼린 라이트를 병사가 정면으로 보고 있을 때 핀이 20 s 반감보다 **눈에 띄게 빨리** 흐려지는가(4 s 반감 추가) · 헤더 2줄째 `exposure … = direct + ambient`가 관찰 병사 발밑 셀 색과 맞는가 · 보라 화살표가 훑은 뒤 다른 방향으로 넘어가는가.

---

## 15. 변경 파일

```
Source/SoldierLab/AI/SoldierSituationField.h/.cpp   신규 (323 / 1488줄)
Source/SoldierLab/AI/SoldierFieldSettings.h          신규 (284줄)
Source/SoldierLab/AI/SoldierLabLog.h/.cpp            신규 — LogSoldierAI
Source/SoldierLab/AI/SoldierDangerMap.h/.cpp         p4 delete
Source/SoldierLab/AI/SoldierCover.h/.cpp             Field 포인터 · FieldCandidateCount · LaneDeniedCost · IsRushing · AddFieldCandidates ·
                                                     CandidateRotationRad · WatchPoint · EvaluatePosition 눈 0 분기 · 부채꼴 필드 쓰기 제거 ·
                                                     DangerHalfLifeSeconds 제거
Source/SoldierLab/AI/SoldierSight.h/.cpp             SweepCone · ConeSweepTracesPerTick/Rays/RangeCm · SightingRadiusCm · MarkClear/ReportSighting 배선
Source/SoldierLab/AI/SoldierEngagement.h/.cpp        bUrgent 스프린트 규칙 · Rush/Cautious · 무접촉 watch point 조준 (+ 오전분은 별도 문서)
Source/SoldierLab/AI/SoldierPerception.h/.cpp        HeardPresence · ReportGunshot → Field->ReportSighting
Source/SoldierLab/AI/SoldierSuppression.cpp          ApplyNearMiss 필드 쓰기 제거
Source/SoldierLab/AI/SoldierDebugDraw.h/.cpp         GetObservedSoldier
Source/SoldierLab/SoldierLab.Build.cs                DeveloperSettings
```

---

## 16. LOD — 밉과 다중 앵커 퇴거 (2026-09-18) [A 코드 · 링 PIE 확인]

### 16.1 왜 클립맵이 아닌가 — 사용자 질문과 분석

09-18 아침, 3단계 빌드가 "딱 내가 원하는 그림"을 낸 뒤 사용자가 물었다: **복셀 GI처럼 클립맵 레벨을 두면 안 되나?** 큰 레벨에서 적군이 후퇴하며 싸우는 시나리오가 걱정이었다.

분석의 출발은 "무엇이 커지는가"였다. 이 필드는 **요구 주도(demand-driven)·희소**다:

| 비용 | 레벨 크기와의 관계 | 근거 |
|---|---|---|
| 섀도우 | 무관 — 라이트 수 × 예산 | `ShadowRaysPerTick` 고정 |
| 굽기 | 무관 — 큐 길이 × 예산 | `HorizonBakesPerTick` 고정, 큐는 **읽은** 셀만 |
| 읽기 | 무관 — 병사가 묻는 점 | 해시 조회 |
| 오버레이 | 무관 — 관찰자 반경 | `MaxDebugCells` |
| **메모리** | **∝ 지나간 면적** | 09-17까지 셀을 **해제하는 코드가 없었다** |
| **굽기 총량** | **∝ 경로 길이** | 셀당 한 번이지만 지나간 셀 전부 |

즉 **계산은 레벨 크기에 전혀 비례하지 않는다** — 맵 전체를 도는 틱 루프가 하나도 없다. 커지는 것은 **꼬리(trail)**다: 후퇴하는 적군이 남긴 레벨 0 셀과 호라이즌이 경기 내내 쌓인다. 그러니 답은 계산 LOD(클립맵의 목적)가 아니라 **메모리 상한**이다(P152).

그리고 **클립맵은 이 필드에 맞지 않는다**. 클립맵은 **한 관찰자**를 중심에 두고 링을 짜는데, 진영 하나에는 관찰자가 **15명**이고 중심이 없다(09-17에 이미 한 번 기각한 이유, 3절). 맞는 것은 둘이다:

- **(A) 밉(mip)** — 거친 레벨은 레벨 0에서 **아래→위로 집계**한 것. 라이트가 없는 먼 땅에 대해 "그 블록 어딘가에 누가 있을지도"를 답한다.
- **(B) 다중 앵커 퇴거** — 진영의 **모든 병사**에게서 `DetailRadiusCm × LevelScale^L`보다 먼 레벨 L 디테일을 부모에 **잔여물(residue)**로 접고 해제한다. 병사 각자가 중심이다.

**오버레이는 반대로 단일 관찰자가 있으므로 진짜 클립맵을 받았다**(17.2절). 한 문장으로: **필드는 밉 + 다중 앵커 퇴거, 오버레이는 클립맵**(P153).

### 16.2 밉 — 거친 셀은 아무도 직접 쓰지 않는다

| 무엇 | 어디 | 규칙 |
|---|---|---|
| 더럽히기 | `MarkDirtyUp`(`:511-521`) · `MarkHorizonDirtyUp`(`:523-530`) | **레벨 0 쓰기**(`WritePresence`·`WriteClear`·`LightCell`·`BakeHorizons`)만 부른다. 위 모든 레벨의 부모 키를 `FindOrAdd`하고 `bDirty = true`. 거친 셀을 직접 쓰는 코드는 **없다** |
| 재집계 | `RefreshCoarse`(`:750-839`) | `bDirty`이거나 `RefreshedTime`이 `CoarseRefreshSeconds 1`보다 오래됐을 때(`:758`) — 라이트는 아무것도 안 쓰고 낡으므로 TTL이 필요. 자식 `Scale²`개를 `SampleLevel0`/`SampleCoarse`로 읽는다 |
| 경계도 집계 | `:797-806` | **max** — "블록 어딘가에 누가 있을지도"는 자식 하나만 그래도 참. 이긴 자식의 **원값 + 원시각**을 그대로 보관해 **한 번만 낡는다**(집계 시각으로 찍으면 낡음이 리셋된다) |
| lit 집계 | `:808-813` | **평균** — "블록의 몇 %가 누군가의 시야에 있나". 경로에는 충분하고 벽 고르기에는 무용(그건 레벨 0 몫) |
| 잔여물 합산 | `:817-832` | `ResChildren`개의 자식으로 쳐서 더한다. 경계도는 max 비교, lit은 `LightHalfLife` 감쇠 × `ResChildren` 가중 |
| 읽을 때 | `SampleCoarse`(`:841-882`) | 집계 lit에 `RefreshedTime`부터의 `LightHalfLife` 감쇠를 한 번 더 곱한다(`:861-864`) — 자식은 집계 순간 값이고 라이트는 그 뒤로 낡았다 |
| 호라이즌 | `RefreshCoarseHorizon`(`:679-748`) | 자식 트임(0..1)의 **평균**, 진영 무관. 레벨 2+는 아래 레벨의 (집계 × Count + 잔여물 × ResCount) / 합 |
| 읽기 진입 | `SampleFinest`(`:884-902`) | 레벨 0 → 1 → … 순으로 첫 답. 7절의 모든 읽기가 이걸 탄다 |

### 16.3 퇴거 — 접기(fold)와 잔여물(residue)

`Evict(Faction)`(`:957-1027`), `Tick`에서 진영마다 매 틱(`:232-238`).

```
앵커 = 이번 틱 등록부의 그 진영 병사 발 위치 전부 (Tick :133-151)
스냅샷: 큐가 전부 비고 EvictionIntervalSeconds 2 지났으면 레벨 0..(Count−2)의 모든 키를 큐에 (:967-983)
틱마다 EvictionCellsPerTick 1024개 Pop:
  반경 R_L = DetailRadiusCm 8000 × LevelScale^L  (:991-992)
  앵커 아무나에게서 R_L 안이면 → 그대로 (IsNearAny :943-955)
  아니면 → SampleLevel0/SampleCoarse로 한 번 읽고 → 셀 Remove → 부모 FoldInto (:1005-1024)
```

- **잔여물이 없으면 접기는 망각이다.** `FoldInto`(`:904-928`): lit은 접힌 자식 수로 가중한 **누적 평균**(옛 잔여물은 `LightHalfLife`로 감쇠시킨 뒤), 경계도는 **지금 더 강한 쪽**(`:916-924` — 자식의 원시각을 보존해 "언제 관측했나"부터 계속 낡게). `ResChildren` 증가, 부모 `bDirty`. → 선택의 결과는 [W82].
- **코스트 레벨(마지막)은 절대 안 접힌다** — 루프가 `Set.Num() − 1`까지. 3200 cm 셀이라 맵 전체여도 무시 가능.
- **스냅샷을 걷는 동안 지워진 키**는 `Find` 실패로 그냥 넘어간다(`:967-969`).
- **섀도우 게이트**(`Tick :179-203`): 그림자는 **앵커에게서 `DetailRadiusCm` 안의 라이트만** 긋는다. 아니면 셀을 써 놓고 다음 패스가 도로 접는 낭비 — 그리고 접힌 잔여물에 "직접 노출"이 남아 [W79]를 키운다. 단 80 m 안 라이트도 60 m를 쏘므로 80 m 밖 셀은 여전히 생겼다 접힌다([W80], 의도).
- **접힘 지연** = 스냅샷 크기 ÷ 1024 틱 + 2 s. 진영당 레벨 0이 2만 셀이면 ≈ 20틱 + 2 s.

### 16.4 호라이즌 퇴거 — 기하는 누구 것도 아니다

`EvictHorizons`(`:1029-1126`): 같은 구조지만 앵커가 **`AllAnchors`(양 진영 전원)**(`:1070`) — 호라이즌은 진영 공유라 **아무나** 가까우면 남는다. 접을 때 `Fine->Stand/Crouch`를 트임 0..1로 바꿔 부모 `FCoarseHorizon`에 `FoldOpenInto`(`:930-941`, 누적 평균). **굽기 큐에 있는 셀(`bQueued && !bBaked`)은 안 접는다**(`:1083-1087`) — 접으면 그 굽기가 허공에 떨어진다. 반대로 `BakeHorizons`는 큐에서 뽑았는데 이미 퇴거된 키면 건너뛴다(`:1606-1610`).

### 16.5 읽기에 부수효과가 있으면 퇴거가 되돌린다 (P157)

`SampleLevel0`(`:646-677`)의 호라이즌 읽기는 `HorizonFor`(굽기 큐에 넣음)가 아니라 **`Horizons.Find`**(`:662-664`, 주석 "A look, not a request")다. 퇴거 패스가 접으려고 셀을 읽는데 그 읽기가 굽기를 **요청**하면, 방금 접은 땅을 다시 굽는 루프가 된다. 병사의 읽기(`GetExposureByStance` 등)는 `SampleFinest`가 −1이거나 `bHasOpen`이 false일 때 **따로** `HorizonFor`를 부른다(`:1767-1779`) — 요청은 묻는 쪽이 한다.

### 16.6 `SoldierLab.Field.CellSizeCm` — 런타임 셀 크기

`:16-21` cvar. 0이면 설정값, 아니면 레벨 0 셀 크기 오버라이드(최소 25). `EnsureLevels`(`:365-411`)가 매 진입마다 `BuiltCellSizeCm/LevelCount/LevelScale`과 비교해 다르면 **셀·라이트·호라이즌·거친 레벨 전부** 비우고 `[Field] resolution changed` 경고. 09-18에 50 cm로 시험한 것이 오버레이 캡 버그(17.3절)를 드러냈다.

---

## 17. 오버레이 v2 (2026-09-18) [링·배처 A PIE · 나머지 B 빌드 전]

### 17.1 깜빡임과 fps 저하의 원인 — 수명 있는 디버그 그리기

사용자 보고: 반경 4000 cm에서 오버레이가 **깜빡이고 fps가 떨어진다**. 원인 둘, 둘 다 `DrawDebugMesh`에 **수명**을 준 것에서 나왔다:

1. 수명이 있는 요소는 월드의 **`PersistentLineBatcher`**로 간다. 그 배처는 **매 프레임 모든 요소를 늙히고 렌더 상태를 다시 만든다** — 4천 개 사각형이면 매 프레임 4천 개.
2. 재그리기는 "≥ 0.1 s"였는데 30 fps에서 실제 간격은 0.133 s, 사각형 수명은 0.12 s → **빈 프레임 하나**가 깜빡임.

해결(P155): 서브시스템이 **`ULineBatchComponent`를 직접 소유**한다(`DebugBatcher`, `.h:462-471`). 월드가 자기 것을 만드는 방식 그대로 — 액터 없이 `NewObject` + `RegisterComponentWithWorld`, `DefaultLifeTime 0` = "flush할 때까지"(`GetDebugBatcher` `:95-116`). 재그리기는 **한 틱에 `Flush` + 채우기**(`DrawDebug :1976`), 그 사이는 정적 — 늙힐 수명도 다시 만들 렌더 상태도 없다. 주기 `DebugRefreshSeconds 0.1`(`Tick :253-259`). 오버레이를 끄면 배처를 flush(`:242-251`), `Deinitialize`에서 등록 해제(`:82-93`).

그리고 **색별 메시 하나**: 배치 메시는 색이 하나라, 색(알파 포함)을 **채널당 8단계**로 양자화해(`/32 × 32 + 16`, `:2000-2003`) 같은 색의 사각형을 한 정점 배열에 모은다(`:1988-2015`). 천 개 셀이 수십 개 `DrawMesh`(`:2228-2231`).

### 17.2 오버레이 클립맵 — 여기엔 관찰자가 하나다

`SoldierLab.Debug.Field.Level −1`(기본, `:43-49`): 레벨 0은 관찰자에서 `DebugDetailRadiusCm 2500` 안, 레벨 L은 `2500 × LevelScale^L`까지, 마지막 레벨은 `Debug.Field.RadiusCm`(기본 **12000**)까지(`:2023-2049`). 0 이상이면 그 레벨만 반경 전체에. 필드 자체엔 없는 "중심"이 오버레이엔 있으므로 여기서만 클립맵이 옳다(P153).

- **거친 셀은 평평한 사각형**(`:2210-2219`) — 블록의 요약이 정직하게 주장할 수 있는 건 그뿐. 레벨 0은 v1처럼 모서리 4점 지면.
- 오버레이가 거친 셀을 `FindOrAdd`하고 레벨 0 셀을 `TouchCell` + `HorizonFor`한다(`:2087-2114`) → [W81].

### 17.3 캡은 대칭으로 잘라야 한다 — 50 cm 셀이 드러낸 버그

사용자가 `SoldierLab.Field.CellSizeCm 50`으로 시험하자 **−X 반쪽만** 그려졌다. 원인: 키를 X 순서로 걷다 `MaxDebugCells`에 닿으면 멈추는 구조라, 링 하나가 캡보다 크면 걷기 순서 앞쪽(−X)만 남는다(25 m 링에 50 cm 셀 = 8천 개 > 4000).

수정(`:2054-2068`, **[B] 빌드 전**): 링을 걷기 **전에** `RingCells = π(Outer² − Inner²)/Size²`를 남은 예산과 비교해 넘치면 **`Outer`를 예산에 맞게 줄인다**(`Outer = √(Inner² + Remaining·Size²/π)`) — 링이 사방으로 고르게 작아진다. 헤더에 `CAPPED - rings shrunk, raise MaxDebugCells`. `MaxDebugCells` 4000 → **6000**. (P154: 상한 있는 순회는 **대칭으로** 잘라야지 순회 순서로 잘라선 안 된다.)

### 17.4 신선도 = 불투명도, 크기 고정 (사용자 결정)

v1의 "크기 = 신선도"는 링과 충돌한다 — 링에서는 **크기가 곧 레벨**이다. 사용자 결정: 크기는 `DebugFill 0.7` 고정, 신선도는 **불투명도**(`DebugAlphaMax 0.8` → `DebugAlphaMin 0.1`, `DebugStaleSeconds 30`에 걸쳐, `:2180-2184`). 0에는 안 간다 — "한참 전에 누가 있었다"는 정보지 잡음이 아니다(v1과 같은 이유).

### 17.5 미굽기 파랑 · 헤더 2줄 · 화살표 · 라이트 v2 [B 빌드 전]

| 무엇 | 어디 | 뜻 |
|---|---|---|
| **파랑끼** | 채널 3에서 `bHasOpen == false`면 B = 140(`:2169-2177`) | 안 구운 셀은 앰비언트가 **0으로 읽혀 "안전"으로 오해**된다. 파랑은 "모름"이지 "안전"이 아니다. 굽히면 B = 0 |
| **헤더 1줄** | `Tick :318-329` | `[Field] HOSTILE \| 채널 \| lights N followed + M frozen \| cells L0/L1/L2 (cell 200 cm x4) \| horizons N \| rings\|level N, drawn N/6000 [CAPPED…] (following\|pinned)` |
| **헤더 2줄** | `DescribeGround`(`:2291-2332`), `Tick :331-338` | 관찰 병사 **자기 발밑 셀**을 AI가 읽는 그대로: `ground L0: exposure 0.43 (stand 0.61 = direct 0.40 + ambient 0.21 / crouch 0.43 = 0.30 + 0.13) presence 0.50 open (unbaked) 5/8 seen 12s ago`. Project Settings 값을 잡을 때 볼 줄 |
| **보라 화살표** | `:2269-2288` | 관찰 병사 발 + 30 cm에서 `GetMostExposedDirection` 방위로 **2.5 m** — 목적지가 아니라 방위(끝점은 40 m까지 가서 경로로 읽힌다). 무접촉 스캔 루프가 이 화살표가 돌아가는 것으로 보인다 |
| **라이트** | `:2235-2267` | **불투명도 = 밝기**(`LightBrightnessNow`, 문턱에서 툭 사라지지 않고 흐려짐) · **색 = 출처**: 초록 = 봄, 호박색 = 들음(`RadiusCm > SharpRadiusCm`로 판정; `SoldierDebugDraw.cpp:123-125`의 인지 오버레이 색과 같음) · 구 18 cm · 핀 굵음 2.5 = 추적 / 얇음 0.5 = 얼림 · 링 반경 = 목격 반경(최소 40) · **흰 점**(눈 +40 cm) = 그림자 아직 안 그음 |

### 17.6 채널 4(앰비언트)는 왜 따로 있나 — 사용자 질문

"앰비언트를 RGB에서 왜 분리했나?" → 분리한 게 아니다. **앰비언트는 노출에 포함돼 있고**(`Exposure = max(LitCrouch + AmbientCrouch, 0.5 × (LitStand + AmbientStand))`, `:2126-2128`) 채널 3의 빨강에 들어 있다. 채널 4는 그 **절반만 따로 보는 디버그 뷰**(`:2129`) — 값을 잡을 때 "이 빨강이 라이트 몫인가 앰비언트 몫인가"를 가르려고.

---

## 18. 라이트의 부정 증거 — "직접 가서 봤는데 없으면 사라지나?" (2026-09-18) [B 빌드 전]

사용자 질문에 대한 답은 **"아니오"**였다 — 09-17까지 얼린 라이트는 `LightHalfLifeSeconds 20`으로만 꺼졌고, 병사가 그 자리를 뚫린 시선으로 몇 초를 보고 있어도 아무 차이가 없었다. 09-16의 부정 증거(P133, `ReportClearView`)는 **기록**에만 있고 **라이트**엔 없었다.

설계(P156): **삭제가 아니라 두 번째 반감기.** 사용자 말대로 그 사람은 웅크리고 있거나 같은 벽 뒤 2 m 옆에 있을 수 있어 한 번 흘끗 본 것으로 결론 낼 수 없다 — 하지만 보고 있는 매 초는 증거다. "봤는데 없는 라이트는 신뢰도를 기본 상태보다 더 빠르게 감소."

| 무엇 | 어디 | 규칙 |
|---|---|---|
| 적립 | `ContradictLightsAlong`(`:1145-1188`), `MarkClearAlongRay` 끝에서(`:641`) — 콘 스윕·목격 사선·빈 땅 확인 셋 다 | **얼린** 라이트만(`:1166` — 추적 중인 것은 눈이 매 틱 갱신하니 제외). 라이트 눈이 뚫린 선분에서 `max(RadiusCm, 셀)` 안이면(`:1171-1179`) **시간**을 적립: 지난 적립과의 간격이 1 s 미만이면 그 간격, 아니면 0.1 s(`:1181-1186`) — 한 병사의 스윕이 같은 방위로 1/3 s마다 돌아오므로 "몇 번 봤나"가 아니라 "얼마나 봤나"를 세려고 |
| 감쇠 | `LightBrightnessNow`(`:1130-1143`) | `Brightness × 0.5^(age/LightHalfLife 20) × 0.5^(ClearViewSeconds/ClearViewHalfLife 4)`. 4 s 정면으로 보면 반, 8 s면 1/4 — 자연 감쇠 위에 |
| 리셋 | `ReportSighting`의 `Refresh`(`:1273-1275`) | 다시 보이면 `ClearViewSeconds 0` — 적립은 무효 |
| 들은 라이트 | `:1187`(저녁 판) | ~~⚠ 브리핑은 "반경 때문에 사실상 제외"라 했으나 코드는 **거르지 않고 오히려 더 넓게 걸린다** → [W83]~~ → **09-18 오후 해결**: `RadiusCm > SharpRadiusCm`면 건너뜀 — 들은 라이트는 부정 증거를 안 받는다(20 s 반감뿐) |

`FLight::ClearViewSeconds/LastClearTime`(`.h:284-289`). 여러 병사가 보면 초가 **합산**된다(`SoldierFieldSettings.h:136-146`).

---

## 19. 09-18 변경 파일

```
Source/SoldierLab/AI/SoldierSituationField.h/.cpp   재작성 (323 → 473 / 1488 → 2332줄)
    + FCoarseCell · FCoarseHorizon · FSample · FLight::ClearView* · DebugBatcher
    + SampleLevel0/SampleCoarse/SampleFinest · RefreshCoarse(Horizon) · MarkDirtyUp/MarkHorizonDirtyUp
    + FoldInto/FoldOpenInto · Evict/EvictHorizons · IsNearAny · ContradictLightsAlong · DescribeGround
    + SoldierLab.Field.CellSizeCm cvar · Debug.Field.Level 기본 −1 · Debug.Field.RadiusCm 기본 12000
    − FirstLevelFor · LastWriterLocation · DetailRangeCm
    GroundSlackCm 40 (섀도우 −CrouchTop → −40)
Source/SoldierLab/AI/SoldierFieldSettings.h          + DetailRadiusCm · EvictionCellsPerTick · EvictionIntervalSeconds · CoarseRefreshSeconds ·
                                                       ClearViewHalfLifeSeconds · DebugRefreshSeconds · DebugDetailRadiusCm · DebugFill · DebugAlphaMin/Max
                                                     − DetailRangeCm · DebugAlpha · DebugFillMin/Max   LevelCount 1→3 · MaxDebugCells 4000→6000
Source/SoldierLab/AI/SoldierCover.h · SoldierSuppression.h   주석 정리 (W78)
SoldierDebugDraw · SoldierSight · SoldierPerception   호출자로서만 (변경 없음)
```

---

## 20. 순찰 — `GetStaleVantage` (2026-09-18 오후) [A · PIE ✅]

> 줄 번호는 **09-18 저녁 판(2491줄)**. 거동(왜 경로가 아니라 비용인가, 어디에 붙는가, 값의 흐름)은 `ai/2026-09-18_patrol_scan_and_move_robustness.md` 2절 — 여기는 필드가 답하는 부분만.

사용자 요구 "적 정보가 없는 존 방어는 계속 순찰해야 한다"에 대한 필드의 답은 **앰비언트 적분을 한 번 더 쓰는 것**이다. 6.2절의 적분에서 방사체(emitter)를 경계도 대신 **"안 본 지 얼마나"(staleness)** 로 바꾸면 "이 자리에 서면 낡은 땅을 얼마나 내려다보는가"가 나온다:

```
GetStaleVantage(F, Foot, Now, StaleSeconds)   .cpp:1916-1956  ·  .h:169-180
  방향 d마다 (Open_stand[d] > 0):
    OpenCm    = Open × HorizonRangeCm
    Staleness = clamp( max(StalenessAt(Foot + d·Open/2), StalenessAt(Foot + d·Open)) / max(1, StaleSeconds) )
    Sum      += Staleness × Open
  return clamp(Sum / 8)
StalenessAt(F, X, Now)                          .cpp:1704-1713
  = SampleFinest 실패 ∥ PresenceTime ≤ NeverStamp → ∞(float max)   ← 한 번도 안 본 땅은 최대로 낡았다
  = Now − PresenceTime
```

- **기립 높이만**(`OpenStand`) — 순찰은 서서 본다.
- 표본 두 점(반·끝)의 **max** — 6.2절과 같은 표본이라 추가 조회 비용은 앰비언트 한 번 분.
- 미굽기(`!bHasOpen`)면 `HorizonFor`로 큐에 넣고 **0**(= "조망 없음" = 순찰 비용 최대)을 돌려준다(`:1922-1926`). 22절의 "미지 = 열림"과 같은 방향 — 모르는 자리가 좋은 자리로 읽히지 않게.
- `PresenceTime`을 갱신하는 것은 `WritePresence`(라이트 얼림 원반)와 `WriteClear`(눈) 둘이다. 그러니 **"본다"가 곧 "덜 낡게 한다"** 이고, 순찰의 되먹임은 콘 스윕(8절)이 닫는다: 낡은 조망이 싸다 → 서서 본다 → 콘이 비운다 → 조망이 비싸진다 → 다음 자리. 경로는 없다(P164).
- 엄폐 층 `PatrolCost = PatrolWeight × (1 − vantage)`, **눈 0일 때만**(`SoldierCover.cpp:203-220`). 기본 `PatrolWeight`는 존/목표 **1.0**, 배정/명령 **0**(존을 거쳐야 켜진다). `PatrolStaleSeconds 30` = 이 나이에 완전히 낡음 → [C-148].

## 21. 부채꼴 스캔과 주시 편향 — `AmbientFromOpenness`의 새 파라미터 (2026-09-18 오후) [A · PIE ✅]

`GetMostExposedDirection(F, Foot, Now, OutPoint, PreferDir, PreferWeight, ArcYawDeg, ArcHalfWidthDeg)`(`.h:165-167` · `.cpp:1881-1914`) → `AmbientFromOpenness(…)`(`.h:454-457` · `.cpp:1715-1784`). 둘 다 **선택(argmax)에만** 걸리고 **적분(StandSum/CrouchSum)에는 안 걸린다**(`:1760-1763` 주석: "부채꼴 밖도 나를 노출시킨다 — 그건 남에게 맡기라는 뜻일 뿐").

```
방향 d마다 (적분은 그대로 누적한 뒤):
  ArcYawDeg 있음 ∧ |Δ(angle_d, ArcYaw)| > ArcHalfWidthDeg  →  후보에서 제외          (:1764-1768)
  Score = Presence × Open_stand
  PreferDir 있음 ∧ PreferWeight > 0  →  Score ×= 1 + PreferWeight × dot(d, PreferDir)   (:1769-1773)
  argmax Score → OutDirection
```

- **arc = 명령의 섹터**. 09-17까지 교전 층이 섹터를 **고정 방위**로 읽어 명령받은 병사(존은 기본 `bUseSector true`)는 스캔이 영영 안 돌았다(거동 문서 1.1절, P163). 이제 필드가 부채꼴 **안에서** 가장 안 훑은 방위를 고르고, 부채꼴 중심은 필드가 여기 대해 할 말이 없을 때(미굽기)만 교전 층이 쓴다.
- **편향 = 이동 방향**(`WatchTravelBias 1`: 정후방 ×0, 정면 ×2 — CQB "눈이 발을 이끈다") 또는 **접근 앵커**(`WatchApproachBias 0.5`) 또는 **경로 굽이 너머**(저녁 C, [B]). 경기 시작 땐 모든 방위가 동률(전부 사전값 0.5)이라 편향 없이는 표본 순서가 방위를 정했다. 누가 어떤 편향을 넘기는가는 엄폐 층(`SoldierCover::BeginSweep` `:1423-1501`) — 거동 문서 4절.
- **볼 곳은 항상 계산된다**(눈이 있어도, Rush만 예외) — 09-17엔 눈 0일 때만이었고, 죽은 적 기록이 ≈ 90 s 눈으로 남는 동안 교전 층은 "접촉 없음"이라 **아무도 조준을 안 모는 틈**이 있었다(P165). 쓸지는 교전 층(`SoldierEngagement.cpp:1275-1328`)이 정하고, 그 결과를 `GetAimPoint()`/`IsScanning()`으로 발행해 포즈 층(`SoldierScanTurnComponent`, 다른 세션)이 몸을 돌린다.
- 오버레이 화살표(`:2414-2447`)는 관찰 병사 배정의 **arc만** 넘긴다(`:2420-2434`) — 이동 중 편향은 안 넘기므로 화살표는 "정지 시 병사가 보는 곳"이다.
- **`DrawDebug` 라이트 링**: 09-18 오전의 선 원(`DrawCircle`)이 **얇은 반투명 annulus 메시**(`:2382-2406`, 24분할, 폭 = 반경 × 0.08 ∈ [4, 25])로 — 배치 선은 알파를 무시해 페이드가 안 보였다. 17.5절 라이트 v2의 "링" 행에 해당.

## 22. solid 셀 · 미지 = 열림 (2026-09-18 오후) [A · PIE ✅]

로그가 드러낸 두 결함(거동 문서 3.1·3.2절)에 대한 필드 쪽 답.

### 22.1 `FHorizon::bSolid` — 안에서 시작한 트레이스는 "0 m에서 막힘"이 아니다 (P160)

큐브(장애물) 안에 셀 중심이 떨어지면 16 트레이스 전부 `bStartPenetrating`(Distance 0)이라 8방향 **0셀 트임** = 지도에서 **가장 어두운 셀**. `FindDarkestCells`가 1순위로 보고했고 내비메시 투영이 큐브 **위**에 떨어져 병사가 4분간 벽을 밀었다.

```
BakeHorizons   .cpp:1641-1681
  첫 트레이스가 bBlocked ∧ (bStartPenetrating ∥ Distance ≤ ε)  →  bSolid = true, 나머지 방향 중단
  bSolid면 Stand/Crouch 8방향 전부 255(완전 트임 = 가장 노출된 읽기)      ← 어떤 비용도 이 셀을 선호 못 한다
FHorizon::bSolid   .h:281-287
FindDarkestCells   .cpp:1998-2002   Horizon == nullptr(미굽기) ∥ bSolid  →  건너뜀
```

"여기가 없다"를 **가장 나쁜 값으로 표현**하는 것이 요점 — `GetExposure`·`GetStaleVantage`·앰비언트 전부 255를 "사방 트임"으로 읽으므로 별도 분기 없이 자연히 배제된다. 굽기가 셀당 한 번이라 판정도 한 번.

### 22.2 `GetExposureByStance` → bool — "데이터 없음"은 최선값이 아니다 (P159)

두 번째 로그: 벽 **꼭대기** 셀(solid 아님 — 셀 중심이 공중) 35건 REJECT. 안 구운 셀을 `(0, 0)`으로 답했고 엄폐 층이 "노출 0 = 완벽한 은폐"로 읽었다.

```
GetExposureByStance   .cpp:1823-1867  ·  .h:146-154
  SampleFinest < 0            →  (0, 0), HorizonFor(큐), return false      (:1830-1837)
  레벨 0 ∧ !bHasOpen          →  (LitStand, LitCrouch)만, HorizonFor(큐), return false   (:1842-1854)
  그 외                       →  직접 + 앰비언트, return true
```

`.h:149-151`: "zero there is absence of an answer, not an answer of zero, and a reader must not take it for cover." 읽는 쪽(`SoldierCover::EvaluatePosition` 눈 0 분기, `SoldierCover.cpp:858-868`)은 false를 **노출 1 · 자세 0 · 은폐 불가**로 — P149("모름 ≠ 없음")의 **읽는 쪽 판**이다. 쓰는 쪽은 사전값 0.5로 이미 그렇게 했지만, 호라이즌이 없는 셀은 앰비언트가 0이 되어 같은 구멍이 다른 층에 다시 났던 것. ~~`GetExposure`(`:1869-1879`)는 bool을 버리고 `max(Crouched, 0.5·Standing)`만 돌려주므로 **경로 위험·Danger 항은 여전히 미굽기 = 0**으로 읽는다 [A·코드] — 경로 표본이 미굽기 땅을 안전하게 볼 수 있다는 뜻, 필요해지면 같은 규칙을 거기도.~~ → **정정: 24절** — 09-18 밤에 닫았다. 미지 = `UnknownPresence × AmbientWeight`(기본 0.5)를 하한으로.

## 23. 콘 스윕의 띠 넓히기 — `MarkClearAlongRay` (2026-09-18 오후) [A · PIE ✅]

사용자 보고 "보라 화살표가 5분 동안 한 방위에 고정, 그 뒤 뒤집힘"의 필드 쪽 원인. `ConeSweepRays 21` / 120° = 이웃 광선 **≈ 6°**. 20 m 밖에서는 광선 간격이 셀(2 m)보다 넓어 **두 광선 사이 셀은 한 번도 `WriteClear`를 못 받는다** → 그 셀을 지나는 방위는 앰비언트에서 계속 0.5를 읽고 `GetMostExposedDirection`이 **같은 방위를 계속 고른다**(눈이 그쪽을 보고 있는데도).

```
MarkClearAlongRay   .cpp:621-659
  Across      = 선에 직교한 단위벡터 × 셀 크기                       (:634-637)
  WidenFromCm = 셀 × 8  (= 16 m)                                      (:638)
  MarchRay 표본마다 (몸 밴드 ±BodyBandCm 안일 때):
    WriteClear(표본)                                                  (:647)
    DistXY(표본, 눈) > WidenFromCm  →  WriteClear(표본 ± Across)      (:649-653)   ← 이웃 셀 둘
  끝에 ContradictLightsAlong                                          (:658)
```

8셀인 이유: 6° × 16 m ≈ 1.7 m < 2 m — 그 안쪽은 이웃 광선 자체가 이웃 셀에 떨어진다. 그 밖은 광선 하나가 **3셀 폭의 띠**를 비운다("사선은 실이 아니라 띠다", `:629-633`). 콘 스윕(40 m)에서 광선당 쓰기가 최대 ≈ 20 → 60으로 느는 셈이나 전부 해시 쓰기라 트레이스 비용은 그대로. 목격 사선·빈 땅 확인(8절)도 같은 함수를 타므로 같이 넓어진다.

**같은 함수의 `ContradictLightsAlong`은 이제 들은 라이트를 거른다**(`:1187`, [W83] 해결 — 18절 표 정정).

### 23.1 09-18 오후 변경 파일 (필드 몫)

```
Source/SoldierLab/AI/SoldierSituationField.h/.cpp   2332 → 2491줄
    + GetStaleVantage · StalenessAt · FHorizon::bSolid(BakeHorizons · FindDarkestCells)
    + AmbientFromOpenness / GetMostExposedDirection: PreferDir · PreferWeight · ArcYawDeg · ArcHalfWidthDeg
    ~ GetExposureByStance → bool (미지 = false)
    ~ MarkClearAlongRay 띠 넓히기(WidenFromCm = 셀 × 8) · ContradictLightsAlong SharpRadius 필터(W83)
    ~ DrawDebug: 라이트 링 annulus 메시 · 화살표에 배정 arc
Source/SoldierLab/AI/SoldierFieldSettings.h          변경 없음
```

호출자 쪽(`SoldierCover` · `SoldierObjective` · `SoldierEngagement` · `Squad/*`)은 `ai/2026-09-18_patrol_scan_and_move_robustness.md` 11절.

## 24. 정정 — `GetExposure`의 미지 = 사전값 (2026-09-18 밤) [A · PIE ✅]

22.2절 끝의 "경로 위험·Danger 항은 여전히 미굽기 = 0"을 닫았다. 같은 날 A/B/C 빌드에 포함, 사용자 "잘됨".

```
GetExposure   .cpp:1869-1889   (2501줄 판)
  GetExposureByStance == false                                     (:1874)
    Unknown = clamp( clamp(UnknownPresence, 0..1) × clamp(AmbientWeight, 0..4), 0..1 )   (:1880-1882)   기본 0.5 × 1 = 0.5
    return max( Unknown, max(Crouched, 0.5·Standing) )             (:1883)   ← 라이트 몫(있으면)과 max
  그 외                                                             (:1888)   max(Crouched, 0.5·Standing) 그대로
```

주석(`:1876-1879`): "숫자 하나만 원하는 읽기 — 경로 표본, 기억 항 — 에게 미지의 땅은 **사전값의 열린 땅**이지 0(= 안전)이 아니다. 셀이 있어 라이트가 말하는 것이 있으면 그것도 위에 얹힌다." `GetExposureByStance`의 두 false 분기(22.2절) 중 두 번째(레벨 0 셀은 있는데 호라이즌만 없음)는 `LitStand/LitCrouch`를 채워 돌려주므로 max가 의미 있다; 첫 번째(셀 없음)는 (0, 0)이라 사전값만 남는다. P159(읽는 쪽)와 P149(쓰는 쪽)가 이 함수에서도 만난다. 새 설정값 없음 — 이미 있는 `UnknownPresence`·`AmbientWeight`의 곱. 거동 쪽은 `ai/2026-09-18_patrol_scan_and_move_robustness.md` 12.5절.

`ContradictLightsAlong`의 들은 라이트 제외(23절 끝, [W83])는 이 판에서도 그대로(`:1187` 부근, 함수 이름으로 찾을 것).

---

## 25. 포인터 — 분대 스코프 · 쐐기 적분 · 섀도우 재캐스트 문턱 (2026-09-21) → 별도 문서

09-18 밤 이후의 필드 변경 셋은 **`ai/2026-09-21_per_squad_field_edge_advance_fire_model.md`** 에 있다(이 문서에는 절을 더 붙이지 않는다 — 스코프가 바뀌면서 1~24절의 "진영별" 서술이 전부 "분대별" 로 읽혀야 한다):

- **1절 — 필드는 분대 하나당 하나** — `FScope` = 진영 × 분대 슬롯(`MaxSquadsPerFaction 3`), 공개 API 전부 `const USoldierIdentityComponent* Who`, 호라이즌만 세계 공유, 무전 → 받는 분대 필드, `bTakesSquadOrders=false` 는 스코프 없음, cvar `Debug.Field.Squad`/`.Centre`, 헤더 `HOSTILE/1`. **[W74] 해결**. 이 문서 13절의 "분대 스코프는 L1 뒤" 는 그 뒤의 말로 대체됐다.
- **2.4절 — `GetWedgePresence`** — 쐐기 안 경계도 × m², 표본 셀의 자기 호라이즌으로 가림, 트레이스 0(엣지 전진의 재료).
- **4절 — 섀도우 재캐스트 문턱 + riders + 비용 줄**(~~빌드 전 [B]~~ → 같은 날 빌드·PIE ✅, 그 문서 10절, [W96] 해결) — `FLight::ShadowEye/ShadowCastTime`, `ShadowRecastMoveCm 0`/`ShadowRecastSeconds 0.5`, 다른 분대의 같은 자리 라이트는 한 벌의 트레이스에 동승, 헤더 3줄째 `[Field] cost/tick`. 실측 96 alive = `MaxLights 16` 상한 → [W92].
- **오버레이 노출 보정(09-21 늦게) → `ai/2026-09-21_debug_overlay_exposure.md`** — 이 문서 17절의 오버레이 v2 는 **사각형·라이트 링을 라인 배처의 `DrawMesh`** 로 그렸는데, 그것은 8-bit `FColor` 라 EV10 고정 노출 레벨에서 숯검정이었다. 지금은 **`USoldierDebugMeshComponent`**(선형 색, 같은 `DebugMeshMaterial`)에 그리고 선·점·구·화살표는 배처에 `HDR(…)`(뷰 노출의 역수 × 색)를 거쳐 넣는다. flush + refill 한 틱(P155)은 그대로. 원칙 P181.

⚠ 이 문서의 "진영별 필드" · "진영 전원이 하나의 필드" 서술(1·2·8·13절 등)은 **09-21 부터 틀리다** — 진영 → 분대. 줄 번호는 09-21 판(3036줄 → 늦게 3081줄)에서 또 밀렸다. 원칙 P179·P180·P181.
