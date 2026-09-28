# 순찰 · 부채꼴 스캔 · 이동 강건성 · 얇은 엄폐 — "적이 죽은 뒤 가만히 서 있는 아군"에서 시작한 하루

2026-09-18 (오후 → **밤 4차 갱신, 12~17절**) / 진행중 (①②③④ 묶음은 빌드·PIE — 사용자 "잘 되는 거 같음. 이제 정상적이다": 적군이 존으로 돌아오고, REJECT 루프 없고, 순찰이 움직인다 · ~~**A/B/C(활동도 가중 은폐 · 미세 위치 · 코너 멈춤/미리 보기)는 코드만, 빌드 전 [B]**~~ → **밤: A/B/C 빌드·PIE, 사용자 "잘됨"** · ~~**ScanTurn 연동 미확인**([C-152])~~ → 포즈 세션 PIE 확인 · 값 [C-148]~[C-153] + **밤 [C-155]~[C-156]**) / **밤 4차(12~17절)**: 빌드 뒤 로그가 잡은 **코너 멈춤 루프**(굽이는 인덱스가 아니라 **자리**, 재개된 이동은 방금 발행한 이동) + 눈이 나타나면 즉시 재개 · `RejectedCandidates` 정리([W90]) · `IsScanning()` 섹터만 있어도 true([W89]) · `GetExposure` 미지 ≠ 0 → **긴장도(`GetTension`)와 걸음(`GetDesiredGait` Walk/Jog/Sprint — 조용한 경비는 걷는다)** → **포즈 급박도(`GetPoseUrgency`) — AI 는 목표 + 숫자 하나만 내고 움직임은 포즈 층의 것** → 스무더 뒤 1프레임 점프(AI 층은 축을 안 쓴다 — 원인은 포즈 세션이 P167 로 확정) → 잠입 접근 현황 답. 원칙 **P172~P175**. / 사용자 보고 "적이 다 죽은 뒤 아군이 가만히 서 있고, 보라 화살표가 몸과 반대를 가리키며, 5분쯤 뒤에 화살표가 뒤집힌다"에서 출발해 **(1) 섹터 = 부채꼴(arc)** · **(2) 콘 스윕의 띠 넓히기** · **(3) 볼 곳은 항상 계산, 쓸지는 교전 층이 결정** · **(4) 교전 층이 `GetAimPoint()`/`IsScanning()`을 발행 → 포즈 세션의 `SoldierScanTurnComponent`** 로 이어졌고, 이어서 **순찰**(경로가 아니라 **낡은 조망(stale vantage)에 대한 비용**), 로그로 잡은 **이동 결함 셋**(큐브 꼭대기 셀 · 60 Hz MOVE 재발행 · 밴드 밖 평평한 Hold 비용), **CQB "눈이 발을 이끈다"**, 그리고 저녁에 **얇은 엄폐 셋**(A 활동도 가중 은폐 · B 미세 위치 · C 코너 멈춤 + 미리 보기)을 넣었다. 진짜 파이 자르기(경로 모양)는 **안 했다.** ★ **09-21 정정(18절): C 코너 멈춤은 삭제되고 엣지 전진으로 대체됐다** — [W85] 해결, `ai/2026-09-21_per_squad_field_edge_advance_fire_model.md` 2절.

전편: `ai/2026-09-17_situation_field_lighting_model.md`(필드 — 이 문서가 **20~23절**을 추가) · `ai/2026-09-17_infiltration_and_unknown_ground.md`(잠입 거동 — **13절** 추가) · `squad/2026-09-17_command_layer_design.md`(명령 계약 — **10절** 추가). 같은 날 오전은 필드 LOD/오버레이 v2(시스템 문서 16~18절), 병행 세션은 분대 2일차(`squad/2026-09-18_squad_layer_fixes_quota_engage_range.md`).
원칙: **신설 P158~P166**(`CLAUDE.md` 5절) · **밤 P172~P175**(12~14절) · P84(천장 없음) · P89(규칙이 아니라 선호) · P10(계측 먼저) · P117(활동도) · P149(모름 ≠ 없음) · P150(속도는 노출로 산다). 짝 문서(소비 측): `animation/2026-09-18_ai_pose_layer_scanturn_gait_smoother.md`(포즈 세션 — ScanTurn · GaitBridge · PoseSmoother, P167~P171).

> 신뢰도: **[A]** 코드로 확인(file:line) · **[B]** 잠정(코드는 있으나 빌드 전) · **[C]** 미측정. 줄 번호는 1~11절이 **2026-09-18 저녁 판**(`SoldierSituationField.cpp` 2491줄 · `SoldierCover.cpp` 1909줄 / `.h` 832줄 · `SoldierEngagement.cpp` 1502줄), **12~17절이 밤 판**(`SoldierCover.cpp` 1931줄 · `SoldierEngagement.cpp` 1586줄 / `.h` 913줄 · `SoldierSituationField.cpp` 2501줄) — 1~11절의 `SoldierCover.cpp`/`SoldierEngagement.cpp` 줄 번호는 밤 판에서 조금씩 밀렸다. 시스템 문서 16~18절의 줄 번호(2332줄 판)는 이 판에서 또 어긋난다 — 함수 이름으로 찾을 것.

---

## 0. 한 장 요약

| 무엇 | 어디 | 상태 |
|---|---|---|
| **섹터 = 부채꼴** — 명령의 섹터 안에서 필드가 가장 안 훑은 방위를 고른다, 중심은 필드가 할 말이 없을 때만 | `SoldierSituationField::AmbientFromOpenness`(`ArcYawDeg/ArcHalfWidthDeg`, `.cpp:1764-1768`) · `SoldierCover::BeginSweep`(`.cpp:1489-1495`) · `SoldierEngagement`(`.cpp:1289-1303`) | [A] · PIE ✅ |
| **콘 스윕 띠 넓히기** — 눈에서 8셀(16 m) 넘는 곳부터 선 양옆 이웃 셀도 비운다 | `MarkClearAlongRay`(`.cpp:629-654`) | [A] · PIE ✅ |
| **볼 곳은 항상 계산** — 스윕에 눈이 있어도 계산, 쓸지는 교전 층의 접촉 판정 | `BeginSweep`(`.cpp:1423-1433`) | [A] · PIE ✅ |
| **교전 층이 `GetAimPoint()`/`IsScanning()` 발행** — 무접촉 분기에서도 조준점을 채움. **밤: 섹터만 있어도 `bScanning = true`**(12.4절, [W89] 해결) | `SoldierEngagement.cpp:1315-1322` · `.h:236-254` (밤 판 `.cpp:1298-1325`) | [A] · ~~소비자(ScanTurn) 연동 [C-152]~~ 포즈 세션 PIE ✅ |
| **`SoldierScanTurnComponent`** — 총 내리고 발 멈춘 AI의 캡슐을 조준점 쪽으로 돌림 (**포즈 세션 작성, 여기선 계약만**) | `Pose/SoldierScanTurnComponent.{h,cpp}` — 게이트 `.cpp:50-77` | [A] 코드 · 동작 ✅ 포즈 세션 PIE(`animation/2026-09-18_ai_pose_layer_scanturn_gait_smoother.md` 6절) |
| **순찰 = 낡은 조망 비용** — `GetStaleVantage(Foot)`, Hold/수비 반경 안 `+ PatrolWeight × (1 − vantage)`, 눈 0일 때만 | `GetStaleVantage`(`SoldierSituationField.cpp:1916-1956`) · `SoldierCover::PatrolCost`(`.cpp:203-220`) · `TaskCost`(`.cpp:164-167` · `:193-200`) | [A] · PIE ✅ ("순찰이 움직인다") |
| **solid 셀** — 굽기가 기하 안에서 시작하면 "여기 없음": 전방향 트임 255 + `FindDarkestCells` 제외 | `BakeHorizons`(`.cpp:1641-1681`) · `FHorizon::bSolid`(`.h:281-287`) · `FindDarkestCells`(`.cpp:1998-2002`) | [A] · PIE ✅ |
| **미지 = 열림** — `GetExposureByStance`가 **bool**을 돌려주고, false면 엄폐 층이 노출 1·은폐 불가로 읽는다 | `.cpp:1823-1867` · `SoldierCover::EvaluatePosition`(`.cpp:858-868`) | [A] · PIE ✅ |
| **도달성 검사 + 거부 목록** — MoveTo 전에 `FindPathSync`(부분 경로 불허), 실패·무진전이면 30 s 거부 | `FinishSweep`(`.cpp:1590-1627`) · `RejectCandidate/IsRejected`(`.cpp:479-504`) · `AddCandidate`(`.cpp:522-531`) | [A] · PIE ✅ (REJECT 루프 없음) |
| **이동 유예** — 발행 0.75 s 안은 "정지"로 안 읽는다 | `MoveGraceSeconds`(`.h:385`) · `.cpp:1546-1551` | [A] · PIE ✅ |
| **Hold/수비 비용 밴드 밖 기울기** — 밴드 밖에서도 `ApproachScaleCm` 비율로 계속 오른다 | `TaskCost`(`.cpp:146-169`) · `ASoldierObjective::GetPositionCost`(`.cpp:54-72`) | [A] · PIE ✅ (적군이 존으로 돌아옴) |
| **CQB — 눈이 발을 이끈다** — 이동 중 볼 곳을 진행 방향으로 편향(`WatchTravelBias 1`), 도착 후 눈 0이면 2 s 머무름 | `BeginSweep`(`.cpp:1447-1475`) · `ScanDwellSeconds`(`.cpp:1557-1563`) | [A] · [C-149] |
| **`MinStance`** — 명령의 자세 바닥값(Rush가 풀음) | `FSoldierAssignment::MinStance`(`SoldierOrderTypes.h:139-140`) · `SoldierEngagement.cpp:795-800` | [A] 코드 · titan DT/콘솔 미연결(**W84 절반**) |
| 들은 라이트는 부정 증거에서 제외 | `ContradictLightsAlong`(`.cpp:1187`) | [A] · **[W83] 해결** |
| **A. 활동도 가중 은폐 판정** — 자세마다 **모든 눈**에 묻고, 보는 눈의 활동도 합 ÷ 전체 ≤ `HiddenGazeFraction 0.5`면 은폐 | `EvaluatePosition`(`.cpp:965-1041`) · `.h:242-254` | ~~**[B] 빌드 전**~~ → **[A] · 밤 빌드·PIE ✅ "잘됨"** |
| **B. 미세 위치** — 정지 ∧ 눈 있음일 때 발 주변 8점 × `MicroStepCm 30`을 후보에(병합 거리 우회) · 엄폐 이동은 수용 반경 `CoverAcceptanceRadiusCm 20`, `bStopOnOverlap=false` | `AddMicroCandidates`(`.cpp:792-822`) · `AddCandidate(…, bMicro)`(`.cpp:506-520`) · `MoveToLocation`(`.cpp:1630-1635`) · 무진전 판정 `SameSpotCm = max(10, 15)`(`.cpp:1600-1606`) | ~~**[B] 빌드 전**~~ → **[A] · 밤 빌드·PIE ✅ "잘됨"** |
| **C. 코너 멈춤 + 미리 보기** — 도달성 검사의 경로를 보관, 다음 굽이(`CornerAngleDeg 35`, `CornerLookAheadCm 500` 안) `CornerStopCm 150` 앞에서 `PauseMove` `CornerPauseSeconds 0.8`(눈 0 ∧ Rush 아님, 굽이당 1회) · 볼 곳 편향은 속도 대신 **굽이 너머 방향**. **밤: 첫 빌드 로그가 멈춤 루프를 드러냄 → 굽이 기억은 자리로 · 재개 = 재발행 · 눈 나타나면 즉시 재개**(12.1~12.2절) | `FindNextCorner`(`.cpp:689-736`) · `UpdateCornerPause`(`.cpp:743-790` → 밤 판 `:750-813`) · `IsPausedAtCorner`(`.cpp:738-741`) · `BeginSweep`(`.cpp:1457-1469`) · `Tick`(`.cpp:1714-1727`) · `.h:272-296` | ~~**[B] 빌드 전**~~ → **[A] · 밤 빌드·PIE ✅ "잘됨"** — **진짜 파이 자르기(경로 모양)는 아님** |
| **밤 — 긴장도 · 걸음** — `GetTension()`(알람 = 접촉 ∥ 제압 ∥ 사선 거부 ∥ 1 s 안 총성, `0.5^(age/20 s)`) · `GetDesiredGait()` Walk/Jog/Sprint(Sprint = `WantsToSprint`, Jog = 접촉 ∨ 긴장 ≥ 0.3, **Cautious ∧ 무접촉 = Walk**) → 포즈 세션 `SoldierGaitBridgeComponent`(Walk → GASP `WantsToWalk`) | `SoldierEngagement.cpp:1391-1416` · `:1441-1454` · `.h:65-71` · `:190-203` · `:455-465` | [A] · **PIE ✅ "잘됨"** · 값 [C-155] |
| **밤 — 포즈 급박도** — `GetPoseUrgency()` 0..1: 상황별 `Urgency*` 7값의 max(제압은 그대로 더함). AI 는 **목표 + 숫자 하나**만 내고 축의 움직임(가속·순항·정착, 축별·비대칭)은 포즈 세션 `SoldierPoseSmootherComponent` 몫 | `SoldierEngagement.cpp:1467-1502` · `.h:205-214` · `:467-497` | [A] · **PIE ✅**("평상시 괜찮음", 1프레임 점프는 포즈 측 해결) · 값 [C-156] |
| **밤 — `GetExposure` 미지 ≠ 0** — 안 구운 셀은 `UnknownPresence × AmbientWeight`(라이트 몫이 있으면 그것과 max) | `SoldierSituationField.cpp:1869-1889` | [A] · 시스템 문서 24절 |
| **밤 — `RejectedCandidates` 만료 정리** — 추가 시 `RemoveAll` | `RejectCandidate`(`SoldierCover.cpp:479-498`) | [A] · [W90] 해결 |

---

## 1. 사용자 보고와 원인 넷 — "가만히 서 있는 아군"

시험 레벨에서 적이 전멸한 뒤 사용자가 본 것:

> 아군이 그 자리에 **가만히 서 있다.** 보라 화살표(볼 곳)가 **몸과 반대**를 가리킨다. 화살표 쪽 사각형만 보인다. 5분쯤 지나자 화살표가 **뒤집혔다.**

원인은 겹쳐 있었고 순서대로 넷을 찾았다. [A]

### 1.1 (a) 섹터를 고정 방위로 읽었다 → 명령받은 병사는 스캔이 영영 안 돌았다

`ASoldierZone::bUseSector`는 **기본 true**(`SoldierZone.h:86`)라 존으로 발행된 모든 명령이 섹터를 준다. 그런데 교전 층의 무접촉 조준은 "섹터 > 볼 곳"(잠입 문서 6절, 09-17)이었다 — 섹터가 있으면 `SectorYawDeg` **한 줄을 응시**. 볼 곳 루프("훑으면 그 방향이 최악이 아니게 되고 다음 방향으로")가 명령받은 병사에게는 **한 번도 안 돌았고**, 섹터 옆 땅은 경기 내내 안 훑였다.

**수정 — 섹터는 부채꼴(arc)이다** (P163):

| 층 | 무엇 | 어디 |
|---|---|---|
| 필드 | `AmbientFromOpenness`가 `ArcYawDeg/ArcHalfWidthDeg`를 받아 **최악 방위 선택만** 부채꼴 안으로 제한 — **적분 자체는 안 거른다**("부채꼴 밖도 나를 노출시킨다, 그건 남에게 맡기라는 뜻일 뿐") | `SoldierSituationField.cpp:1760-1768` · `GetMostExposedDirection` 시그니처 `.h:165-167` |
| 엄폐 | `BeginSweep`이 배정의 `bHasSector`면 arc를 넘긴다(**이동 중엔 안 넘김** — 4절) | `SoldierCover.cpp:1489-1495` |
| 교전 | `AimYaw = bHaveWatch ? (볼 곳 방위) : SectorYawDeg` — 볼 곳이 있으면 그것이 이미 "명령의 부채꼴을 필드가 좁힌 것"이고, **부채꼴 중심은 필드가 여기 대해 할 말이 없을 때만** | `SoldierEngagement.cpp:1289-1303` |
| 계약 주석 | `FSoldierAssignment::bHasSector` 주석이 뜻을 바꿔 적음 | `SoldierOrderTypes.h:169-176` |

### 1.2 (b) 콘 스윕 광선 사이의 셀이 영영 안 비워졌다 → 한 방위가 "가장 안 훑음"으로 고정

`ConeSweepRays 21`을 120°에 펴면 광선 간격 ≈ 6°. 20 m 밖에서는 이웃 광선 사이가 셀(2 m)보다 넓어 **두 광선 사이 셀은 한 번도 `WriteClear`를 못 받는다.** 그 셀을 지나는 방위는 앰비언트 적분에서 계속 사전값 0.5를 읽고, `GetMostExposedDirection`이 **같은 방위를 계속 고른다** — 화살표가 안 움직이는 이유. 5분 뒤 뒤집힌 것은 다른 방위의 `PresenceHalfLifeSeconds 8`로 낡은 셀들이 0.5로 돌아와 동률이 바뀐 것 **(추정)**.

**수정 — 사선은 실이 아니라 띠다**: `MarkClearAlongRay`가 눈에서 `WidenFromCm = 셀 × 8`(16 m, `SoldierSituationField.cpp:638`) 넘는 표본부터 **선에 직교한 이웃 셀 둘**(`Across = ±셀`)도 비운다(`:649-653`). 같은 몸 밴드 조건(`Above ∈ ±BodyBandCm`) 안에서만. 8셀인 이유: 6° × 16 m ≈ 1.7 m < 셀 2 m — 그 안쪽은 광선 자체가 이웃 셀에 떨어진다. → 시스템 문서 23절.

### 1.3 (c) 볼 곳이 엄폐 층의 눈에 게이트돼 있었다 → 두 문턱 사이의 틈

`BeginSweep`은 `SweepEyes.Num() == 0`일 때만 볼 곳을 계산했다(09-17). 그런데 죽은 적의 기록은 `MinThreatCertaintyForCover 0.05`까지 **≈ 90 s** 눈으로 남고(`GatherThreatEyes`), 교전 층의 접촉 판정(`bHasContact`)은 다른 문턱을 쓴다. 그 사이 — 교전은 "접촉 없음"이라 볼 곳을 쓰려 하고, 엄폐는 "눈 있음"이라 볼 곳을 안 만든 — **아무도 조준을 안 몰았다.** 병사는 마지막 전투 방향을 보고 서 있고 필드 화살표(오버레이는 필드에 직접 물으니)는 반대를 가리킨다 — 사용자가 본 그림 그대로.

**수정 — 볼 곳은 항상 계산, 쓸지는 소비자가 결정** (P165): `BeginSweep`이 눈 유무와 무관하게 `GetMostExposedDirection`을 부르고(`SoldierCover.cpp:1423-1501`, Rush만 예외), 교전 층이 자기 접촉 판정으로 쓸지 정한다.

### 1.4 (d) 눈은 도는데 몸이 안 돈다 → `GetAimPoint()`/`IsScanning()` 계약

(a)~(c)를 고쳐도 사용자가 본 것: 총 내린 idle에서 **몸이 안 돈다.** 시야 콘은 `SetFocalPoint`가 모는 컨트롤 회전을 읽으니 **인지는 돈다**(콘 스윕이 새 방위를 훑는다) — 그러나 GASP의 orient-to-movement는 움직일 때만, aim 모드는 견착일 때만 캡슐을 돌린다. [W69]가 "(추정)"으로 적어 둔 바로 그 상태.

**수정 — 교전 층이 신호를 발행한다**: 무접촉 분기에서 `AimPoint = BaseMuzzle + AimRotation × 3000`을 채우고(`SoldierEngagement.cpp:1321`), `bScanning = bHaveWatch`(`:1322`). `GetAimPoint()`는 "접촉이 있으면 예측 위치, 없으면 스캔 중인 점 — **항상 AI 컨트롤러 초점과 같은 점**"(`.h:236-243`), `IsScanning()`은 "접촉 없이 필드가 눈을 돌리는 중"(`.h:245-254`). **몸을 돌릴지는 포즈 층의 결정**이고 이것은 그 결정의 재료다.

~~⚠ `bScanning`은 **볼 곳이 있을 때만** true다. 섹터만 있고 필드가 여기 대해 아직 할 말이 없는(굽기 전) 순간은 조준은 섹터 중심으로 가지만 `IsScanning() == false` — `SoldierScanTurnComponent`의 기본값 `bOnlyWhileScanningOrInContact = true`(`.h:46`)에서는 그 순간 몸이 안 돈다 [A·코드]. 굽히면 볼 곳이 생기므로 짧은 틈이지만 [C-152] 판정 때 볼 것.~~ → **정정: 17.1절** — 밤에 `bScanning = true`를 "무접촉 조준점이 있을 때"(섹터 중심이든 볼 곳이든)로 넓혔다([W89] 해결).

### 1.5 포즈 세션의 답 — `SoldierScanTurnComponent` (계약만 서술)

`Source/SoldierLab/Pose/SoldierScanTurnComponent.{h,cpp}`는 **다른 세션(포즈)이 썼다.** 이 문서는 소비하는 계약만 적는다 [A·코드]:

- 틱 순서: `AddTickPrerequisiteComponent(Engagement)`(`.cpp:39`) — **이번 프레임의** 조준점을 읽는다. `TG_PrePhysics`(`:26`).
- 게이트(`.cpp:50-77`): ① `bEnabled` ∧ 폰 ∧ 교전 컴포넌트 ∧ **플레이어 조종 아님**(`:50`) ② **`!WantsToAim()`**(견착이면 GASP aim 모드 몫, `:60`) ∧ (`bOnlyWhileScanningOrInContact`면 `IsScanning() || HasContact()`, `:58-59`) ③ **정지**: `IsMovingOnGround` ∧ 가속 0 ∧ 속도 ≤ `MaxSpeedCms 10`(`:68-72` — 이동 중엔 CMC가 캡슐 주인).
- 동작: `GetAimPoint()` 방위와 캡슐 yaw 차가 `StartDegrees 20` 넘으면 회전 시작, `StopDegrees 5` 안이면 끝(히스테리시스, `:93-102`), `TurnRateDegPerSec 180`(교전 `AimSlewDegreesPerSecond 240`보다 느리게 — 몸이 눈을 뒤따른다, `.h:22-25`)으로 `SetActorRotation`(`:104-111`). 캡슐만 — GASP `OffsetRootBone`이 메시를 제자리에 두고 MM이 turn-in-place를 고른다.
- 디버그 `SoldierLab.Debug.ScanTurn 1`: 시안 = 조준 방위, 흰색 = 캡슐 전방, 라벨 `scanturn SCAN|contact d=… TURN`.

`[Cover]` 로그에 **`watch=1|0`** 을 추가해(`SoldierCover.cpp:1669-1690`) ScanTurn이 안 돌 때 "볼 곳이 없었나, 게이트에 막혔나"를 가르게 했다.

---

## 2. 순찰 — 경로가 아니라 비용 (P164) [A · PIE ✅]

### 2.1 요구

사용자: **"적 정보가 없는 존 방어는 계속 순찰해야 한다 — 밖에서 접근은 언제나 가능하니까."** 존 반경 안은 Hold 비용이 평평한 0이라(P73 — 수비대를 중심점으로 붕괴시키지 않으려고), 아는 위협이 없는 홀더는 **도착한 자리에 경기 끝까지** 서 있었다.

### 2.2 설계 — 왜 경로(waypoint)가 아닌가

경로를 쓰면 ① 저작이 필요하고 ② 엄폐·노출과 **다른 통화**라 합칠 수 없고(P89 — 규칙이 아니라 선호) ③ "이미 본 곳"을 다시 도는 낭비를 막을 수 없다. 대신 **"낡은 땅을 얼마나 내려다보는가"를 비용으로** 매긴다:

```
GetStaleVantage(Foot) = clamp( (1/8) Σ_d  Open_stand[d] × clamp( max(staleness(Foot + d·Open/2), staleness(Foot + d·Open)) / PatrolStaleSeconds ) )
staleness(X) = Now − PresenceTime(X)   (한 번도 관측 안 됐으면 ∞ → 1)
PatrolCost(Foot) = PatrolWeight × (1 − vantage)          단, 눈 0 ∧ Weight > 0 ∧ 필드 있음일 때만
```

`SoldierSituationField.cpp:1916-1956` · `StalenessAt` `:1704-1713` · `SoldierCover::PatrolCost` `:203-220`. **앰비언트 적분에서 방사체를 경계도 대신 "안 본 지 얼마나"로 바꾼 것**(`.h:169-180`). 기립 높이만(순찰은 서서 본다).

거동: 낡은 땅을 많이 내려다보는 자리가 **싸다** → 거기 서서 본다 → 콘 스윕이 그 땅을 비운다(`PresenceTime` 갱신) → staleness 0 → 그 자리가 **비싸진다** → 다음 낡은 조망으로. **경로는 어디에도 없다.** 노출 비용은 그대로 살아 있으므로 순찰이 살상 지대를 건너 모퉁이를 확인하러 가진 않는다(`SoldierOrderTypes.h:184-194`).

### 2.3 어디에 붙는가

| 자리 | 조건 | 어디 |
|---|---|---|
| 배정 Hold | `DistanceCm ≤ Task.RadiusCm`(반경 안)일 때 `Hold += PatrolCost(Foot, Task.PatrolWeight, Task.PatrolStaleSeconds)` | `SoldierCover.cpp:164-167` |
| 레벨 `ASoldierObjective` 수비 | `GetRoleFor == Defend ∧ DistXY ≤ RadiusCm` | `:193-200` |

두 번째가 있는 이유: **`L_SoldierTest`에는 명령이 없다** — 첫 순찰 시험을 그 레벨에서 돌렸는데 로그에 `[Squad]` 줄이 **0줄**이었다. 배정에만 붙였으면 순찰이 돌 수 없었다. `ASoldierObjective::PatrolWeight 1 / PatrolStaleSeconds 30`(`SoldierObjective.h:98-108`).

값의 흐름: `ASoldierZone::PatrolWeight 1.0 / PatrolStaleSeconds 30`(`SoldierZone.h:91-100`) → `ApplyToOrder`(`.cpp:123-124`) → `FSoldierSquadOrder`(`SoldierOrderTypes.h:290-295`, **기본 0**) → `BuildAssignment`(`SoldierSquadSubsystem.cpp:270-271`) → `FSoldierAssignment`(`.h:184-200`, **기본 0**). 즉 **존을 거치지 않은 명령은 순찰 0**이다 — 의도(땅의 값은 존이 준다).

### 2.4 눈 0 조건의 뜻

`SweepEyes.Num() > 0`이면 0(`SoldierCover.cpp:211`). 순찰은 **전투 사이**에 하는 것이지 전투 중에 하는 것이 아니다. 단 1.3절과 같은 이유로 죽은 적의 기록이 ≈ 90 s 눈으로 남으므로 전멸 직후 순찰 재개까지 그만큼 걸린다 [A·코드, 체감 미측정].

---

## 3. 로그로 잡은 이동 결함 셋 [A · PIE ✅]

`titan_example.log` 08:52 · 09:51 세션(`L_SoldierTest`, `SoldierLab.Debug.Cover.Log 1`). P10 — 추측 대신 로그.

### 3.1 큐브 꼭대기 셀 — Friendly가 230 s 동안 초당 60번 `MOVE @(+4,−492)`

**증상**: 한 아군이 매 스윕(눈 0이라 스윕이 매 틱 끝난다) 같은 목적지로 `MOVE`를 재발행, 230 s. 목적지는 큐브(장애물) **꼭대기** 셀.

**원인 세 겹**:
1. **호라이즌 굽기가 큐브 안에서 시작했다.** 셀 중심이 기하 안이면 16 트레이스 전부 `bStartPenetrating`(Distance 0) → 8방향 "0셀 트임" = **지도에서 가장 어두운 셀** → `FindDarkestCells`가 1순위 후보로 보고 → 내비메시 투영(`NavProjectExtentCm`)이 **큐브 위 폴리곤**에 떨어졌다.
2. **`MoveTo`가 부분 경로로 성공했다.** 큐브 위 폴리곤은 내비메시에 있으나 병사 폴리곤과 안 이어진다 → 경로가 가장 가까운 벽까지 → 병사가 벽에 붙어 4분.
3. **매 틱 재발행.** 벽에 붙어 속도 0 → `bMakingProgress false` → "정지" → 같은 최선 후보로 다시 `MoveTo`(`FinishSweep`)… 가속할 틈도 없이.

**수정 셋**:

| # | 무엇 | 어디 | 원칙 |
|---|---|---|---|
| ① | **solid 셀** — 굽기의 첫 트레이스가 `bStartPenetrating ∥ Distance ≤ ε`면 `bSolid = true`, 8방향 **255(완전 트임)** 로 채워 어떤 비용도 선호 못 하게, `FindDarkestCells`는 **아예 건너뜀** | `BakeHorizons` `SoldierSituationField.cpp:1641-1681` · `FHorizon::bSolid` `.h:281-287` · `FindDarkestCells` `:1998-2002` | **P160** — 안에서 시작한 트레이스는 "0 m에서 막힘"이 아니라 "여기가 없음" |
| ② | **동기 경로 검사** — `MoveToLocation` 전에 `FindPathSync`(`SetAllowPartialPaths(false)`, 배정의 `NavFilterClass` 적용), `!IsSuccessful ∥ !Path ∥ IsPartial`이면 **`RejectCandidate` 30 s** + `DISCARD`. 이동 결정 한 번당 경로 탐색 1회 — "대안은 4분이었다" | `FinishSweep` `SoldierCover.cpp:1590-1627` · `RejectCandidate/IsRejected` `:479-504` · `AddCandidate`가 거부 목록 검사 `:522-531`(`CandidateMergeCm 150` 안이면 같은 후보) | **P161** — 내비메시 위의 점은 도달 가능한 점이 아니다, 점이 아니라 경로를 시험 |
| ②' | **무진전 거부** — 같은 목적지(`LastIssuedFoot`)로 같은 출발점(`LastIssuedFrom`)에서 유예 지난 뒤 또 고르면 "경로는 됐다는데 세계가 반대했다" → 거부. "같은 자리"의 반경은 저녁 B 뒤 `SameSpotCm = max(10, MicroStepCm × 0.5) = 15`(미세 후보가 150 안에 여럿이라) | `:1600-1606` · 상태 `.h:757-760` | 〃 |
| ③ | **`MoveGraceSeconds 0.75`** — 발행 뒤 이 시간 안은 `bJustIssued`로 "진행 중"으로 친다 | `.h:379-385` · `.cpp:1546-1551` | **P162** — 방금 발행한 이동은 정지한 이동이 아니다 |

~~⚠ `RejectedCandidates`는 **만료된 항목을 지우지 않는다**(`RejectCandidate` `:479-491`은 `Add`만, `IsRejected` `:493-504`는 시간으로 거를 뿐) — 긴 경기에서 배열이 자란다. 거동엔 무해, [W90].~~ → **정정: 17.2절** — 밤에 `RejectCandidate`가 추가 전에 만료분을 `RemoveAll`(`SoldierCover.cpp:483-486`), [W90] 해결.

### 3.2 x = −1824 벽 꼭대기 셀 — 두 번째 세션의 REJECT 35건

①②③ 뒤 두 번째 로그: `REJECT` 35줄이 전부 `x=−1824` 근처 — **벽 꼭대기** 셀. solid가 아니다(벽 위 셀 중심은 공중). 원인: **안 구운 셀**을 `GetExposureByStance`가 `(0, 0)`으로 답했고 엄폐 층이 그것을 "노출 0 = 완벽한 은폐"로 읽었다. 거부 목록이 30 s마다 같은 셀을 다시 열어 주니 35번.

**수정**: `GetExposureByStance`가 **`bool`을 반환** — 어느 레벨에도 셀이 없거나(`:1830-1837`) 레벨 0 셀의 호라이즌이 안 구워졌으면(`:1842-1854`, 라이트 몫만 답하고) **false**. 엄폐 층 눈 0 분기는 false를 **노출 1 · 자세 0 · 은폐 불가**로 읽는다(`SoldierCover.cpp:858-868`) — "모르는 땅은 숨은 땅이 아니라 모르는 땅이고, 읽기가 굽기를 큐에 넣었으니 다음 스윕은 안다." **P159** — "데이터 없음"이 최선값으로 읽혀선 안 된다(P149의 읽는 쪽 판).

### 3.3 Enemy_A `hold@z1`, 후보 전부 `o3.20` 200 s — 밴드 밖 평평한 Hold 비용

**증상**: 적군 한 명이 존 1 홀드 배정으로 200 s 동안 모든 후보의 목표 비용이 **3.20 동일**. 어디로도 안 움직임.

**원인**: 엄폐가 병사를 반경+밴드 밖으로 끌어냈는데(전투 중엔 정상), Hold 비용이 밴드 끝에서 **1.0에 멈췄다**(`Hold = min(1, Beyond/Band)`). 밴드 밖 후보는 전부 1.0 × `ObjectiveWeight 3.2` = 3.20 → **기울기 0 → 집으로 가는 이유가 없다.** 접근 비용에서 P84로 걷어냈던 천장이 홀드 밴드에는 남아 있었다.

**수정** (P158 — 오르기를 멈춘 비용은 당기기를 멈춘다):

```
Beyond = Distance − Radius
Hold   = Beyond ≤ Band ? Beyond / Band : 1 + (Beyond − Band) / ApproachScaleCm
```

`TaskCost` Hold `SoldierCover.cpp:146-169` · `ASoldierObjective::GetPositionCost` 수비 `SoldierObjective.cpp:54-72`(같은 모양). 사용자 확인: "적군이 존으로 돌아온다."

---

## 4. CQB — 눈이 발을 이끈다 (look before you move) [A · 값 C-149]

이동 중 무접촉이면 볼 곳을 **진행 방향으로 편향**한다:

| 편향 | 언제 | 값 | 어디 |
|---|---|---|---|
| **`WatchTravelBias 1.0`** — 굽이 너머 | (이동 ∨ 코너 멈춤 중) ∧ `FindNextCorner` 성공 → `Prefer = Beyond`(굽이 나가는 방향) — **저녁 C** | `Score ×= 1 + 1.0 × dot(Dir, Prefer)` → 정후방 0, 정면 ×2 | `BeginSweep` `SoldierCover.cpp:1457-1469` [B] |
| **`WatchTravelBias 1.0`** — 진행 방향 | 이동 중(속도 > `StallSpeedCms 20`) ∧ 굽이 없음 → `Prefer = 속도 방향` | 〃 | `:1470-1474` · `.h:361-368` · 필드 `:1769-1773` |
| `WatchApproachBias 0.5` | 정지 ∧ Approach 배정 → `Prefer = Anchor − Feet` | 정면 ×1.5 | `:1477-1488` · `.h:353-359` |
| 섹터 arc | **편향이 없을 때만**(정지 ∧ Approach 아님) | | `:1489-1495` |

"모퉁이를 돌면서 측면을 보는 병사는 중요한 것을 하나도 안 훑은 것이다." 여행 편향이 **가장 세다** — 이동을 눈먼 이동이 아니라 조심스러운 이동으로 만드는 것이 이것이라서. **이동 중엔 섹터 arc를 안 적용한다** — 가는 곳을 본다.

**도착 머무름 `ScanDwellSeconds 2`** (`.h:370-377` · `FinishSweep` `:1557-1563`): 눈 0이면 엄폐 유무와 **무관하게** 도착 후 2 s는 `DwellBreakMargin 1.0`으로 잠긴다 — 다음 홉은 콘 스윕이 새 땅을 훑은 뒤에 결정한다. `MinDwellSeconds 3`은 "엄폐를 안 떠남", 이것은 "보고 나서 감".

⚠ 오버레이의 보라 화살표(`SoldierSituationField.cpp:2414-2447`)는 **배정의 arc만** 넘기고 여행/접근/굽이 편향은 안 넘긴다(`:2420-2434`) — 이동 중엔 화살표와 병사의 실제 볼 곳이 다를 수 있다 [A·코드]. 정지 상태에서만 "병사가 보는 곳"으로 읽을 것.

---

## 5. 분대 세션 목록에 대한 답

| 항목 | 답 |
|---|---|
| **"숙여서 침투 없음"** | 이미 있다 — 필드 자세(잠입 문서 8.2절): 낮은 벽에서는 `RequiredStance 1`, 공터에서는 웅크려도 안 숨으니 0(웅크림은 속도만 잃는다). "**어디서나 더 낮게**"는 다른 요구라 **`FSoldierAssignment::MinStance`**(`SoldierOrderTypes.h:129-140`)를 자세 바닥값으로 추가 — `DesiredStance = max(DesiredStance, MinStance)`, **Rush면 안 적용**(`SoldierEngagement.cpp:795-800`). `FSoldierSquadOrder::MinStance`(`.h:267-269`)가 존 동사(MoveTo/Occupy/Withdraw)와 함께 흐른다(`SoldierSquadSubsystem.cpp:263`). ⚠ **titan 쪽 `FScenarioSquadOrderSpec`·`titan.SquadOrder` 콘솔·`ASoldierZone`에는 아직 없다**(`Source/titan_example` grep 0건) — DT에서 못 준다 → **[W84] 절반** |
| **"은폐 경로 보류"** | 이미 있다 — 필드 후보(`AddFieldCandidates`, 문간→문간)와 경로 표본의 앰비언트 가격(`EvaluateRoute`, `SoldierCover.cpp:1138`). 별도 "은폐 경로 계획"은 안 만든다 |
| 스프린트/Rush | **불변** — `bUrgent = 접촉 ∥ 제압 ∥ 사선 거부`(`SoldierEngagement.cpp:1384-1386`), `Cautious` 절대/`Rush` 항상(`:1398-1401`). Rush는 볼 곳·필드 후보·필드 자세·**MinStance**·**코너 멈춤** 다섯을 끈다(`SoldierCover::IsRushing` `:379-388` · `UpdateCornerPause` `:768`) |

---

## 6. 들은 라이트 — [W83] 해결

`ContradictLightsAlong`의 필터가 `!bAlive ∥ !bFrozen ∥ RadiusCm > SharpRadiusCm`(`SoldierSituationField.cpp:1187`)로 바뀌어 **들은(흐린) 라이트는 부정 증거를 안 받는다.** 주석(`:1183-1186`)에 "처음엔 안 걸렀고 넓은 반경 때문에 **가장 부정하기 쉬운** 라이트가 됐다(2026-09-18)"를 남김 — 문서 세션의 [W83] 발견이 코드에 반영된 것. 시스템 문서 18절 표의 "들은 라이트" 행 정정.

---

## 7. 저녁 — 얇은 엄폐 셋 A · B · C [B · 빌드 전]

오후 빌드 뒤 남은 문제: **시험 레벨의 큐브가 아니라 실제 레벨의 나무.** 2 m 셀·1.5 m 후보 병합·"눈 전원에게서 숨어야 은폐"로는 굵기 30~50 cm의 나무 뒤가 후보로 잡히지도, 잡혀도 정확히 뒤에 서지도 못한다. 셋을 같은 저녁에 넣었고 **빌드는 안 했다.**

### 7.1 A — 활동도 가중 은폐 판정 (`HiddenGazeFraction 0.5`, P166)

**문제**: `EvaluatePosition`의 은폐 판정은 "가슴·머리 둘 다 **모든 눈**에게서 막힘"이었다 — 하나라도 보면 그 자세는 노출. 나무는 **한 방위**만 가린다. 적 둘이 두 방위에 있으면 어떤 나무도 둘 다에게서 안 가리고 → 나무는 전부 공터 가격 → **나무가 엄폐인 레벨에서 아무도 엄폐를 안 쓴다.**

**수정** (`SoldierCover.cpp:965-1041` · `.h:242-254`): 자세 표본마다 **모든 눈**에 묻고(눈 안에서는 가슴 막히면 머리, 09-16 두 점 그대로), 보는 눈의 **활동도**(`FThreatEye::Activity` — 방금 쏜 총 1.0, 조용하면 `QuietEyeWeight 0.3`, P117) 합을 전체 활동도 합으로 나눈 **보는 시선 비율**이 `HiddenGazeFraction 0.5` **이하**면 그 자세는 은폐(`SeenGaze ≤ 0 ∥ SeenFraction ≤ 0.5`, `:1035-1040`). 첫 은폐 자세가 `RequiredStance`, 노출은 그 자세 인덱스 비율(변함없음, `:1055-1057`).

| 눈 | 판정 |
|---|---|
| 1개 | 비율은 0 또는 1 → **옛 판정과 동일** |
| 2개 활동도 같음 | 하나에게서만 숨어도 0.5 ≤ 0.5 → **은폐** — 나무가 엄폐가 된다 |
| 2개: 쏘는 놈 1.0 · 조용한 놈 0.3 | 쏘는 놈에게서 숨으면 0.3/1.3 = 0.23 → 은폐 / 조용한 놈에게서만 숨으면 1.0/1.3 = 0.77 → **노출** — "쏘는 소총을 막는 나무는 엄폐, 조용한 소총만 막는 나무는 아니다" |
| 3개 같음 | 둘에게서 숨어야(1/3 ≤ 0.5) |

비용은 늘지 않는다 — 예산 하한 `(2·Samples + 3) × Eyes`(`:897`)는 원래 눈마다 두 점을 셈. 사격 가능(`OutCanFight`) 판정(`:911-963`)은 그대로 "누구 하나라도 보이면".

### 7.2 B — 미세 위치 (`MicroStepCm 30` · `CoverAcceptanceRadiusCm 20`)

**문제**: 후보가 2 m 셀 중심·부채꼴 광선·링에서 오고 `CandidateMergeCm 150`으로 병합되니 나무 그림자(폭 30~50 cm)에 **정확히** 떨어질 확률이 낮고, 떨어져도 `MoveToLocation`의 기본 수용 반경(에이전트 반폭) + `bStopOnOverlap=true`가 **반 몸 앞에서** 멈춰 나무 **옆**에 선다.

**기각한 안 — 병사별 고해상도 복셀**: 병사마다 유지하는 격자는 비싸고 **눈이 움직이면 낡는다**(P143의 반대편). 요구 시 국소 재탐색이 같은 일을 더 싸게 한다.

**수정**:

| 무엇 | 어디 |
|---|---|
| `AddMicroCandidates` — **정지**(속도 ≤ `StallSpeedCms`) ∧ **눈 ≥ 1**일 때만, 발 주변 8방향 × `MicroStepCm 30`을 내비메시에 투영해 후보에. `BuildCandidates` 마지막(`:686`) | `.cpp:792-822` · `.h:264-270` |
| `AddCandidate(Foot, bFromFan, bMicro)` — 미세 후보는 **병합 거리 검사를 건너뛴다**(정의상 150 안에 여럿). 거부 목록 검사는 그대로 | `.cpp:506-520` · `.h:644` |
| 엄폐 이동 `MoveToLocation(…, AcceptanceRadius = max(5, CoverAcceptanceRadiusCm 20), bStopOnOverlap = false, …)` — "목표는 엄폐 **뒤**의 점이고 반 몸 앞에서 멈추면 나무 옆이다" | `.cpp:1630-1635` · `.h:256-262` |
| 무진전 거부의 "같은 자리" = `max(10, MicroStepCm × 0.5)` = 15 cm(옛 `CandidateMergeCm 150`) — 미세 후보끼리 서로 거부하지 않게 | `.cpp:1600-1606` |

⚠ 작은 수용 반경은 `MoveImprovementMargin 0.3`과 맞물려 **30 cm 이동을 반복**할 수 있다(미세 후보가 HERE보다 0.3 싸면 옮기고, 옮긴 자리에서 또…). 판정 [C-153].

### 7.3 C — 코너 멈춤 + 미리 보기 (파이 자르기의 **반쪽**)

**문제**: 4절의 여행 편향은 "가는 방향"이지 "**곧 열릴** 방향"이 아니다. 벽을 따라 걷다 모퉁이를 도는 순간 눈은 아직 벽을 보고 있다.

**수정** — 이동 결정 때 도달성 검사가 만든 경로를 **보관**하고(`CurrentPath = Result.Path`, `:1617-1618`; 도착 시 `Reset`, `:1725`) 그 굽이를 쓴다:

| 무엇 | 규칙 | 어디 |
|---|---|---|
| `FindNextCorner(Feet)` | 경로 점 `i`에서 들어오는 방향과 나가는 방향의 각이 `CornerAngleDeg 35` 초과면 굽이. **발 앞**(들어오는 방향으로 양수) ∧ `CornerLookAheadCm 500` 안인 첫 굽이를 돌려줌(인덱스 · 점 · **나가는 방향 `Beyond`**). 더 먼 굽이면 false | `.cpp:689-736` |
| `UpdateCornerPause` (`Tick` 매 틱 `:1717`) | 조건: `CornerPauseSeconds > 0` ∧ **눈 0** ∧ **Rush 아님** ∧ 상태 `Moving`(`:768-769` — "총 맞고 있으면 모퉁이에서 안 멈춘다, 돈다"). 다음 굽이가 `CornerStopCm 150` 안이고 아직 안 멈춘 굽이(~~`Index ≠ LastPausedCornerIndex`~~ → **정정: 12.1절 — 자리로 기억**, 굽이당 1회)면 **`AAIController::PauseMove`**, `CornerResumeSeconds = Now + 0.8`. 시간이 되면 ~~`ResumeMove`(`:751-760`)~~ → **정정: 12.1~12.2절 — 재개 시 유예 재시작, 눈이 나타나면 즉시 재개** | `.cpp:743-790`(밤 판 `:750-813`) · `.h:272-296` |
| `IsPausedAtCorner` | 멈춤 중은 **"아직 이동 중"** — `FinishSweep`의 `bAlreadyGoing`(`:1549-1551`, 재결정 안 함)과 `Tick`의 도착 판정(`:1720-1721`, 도착으로 안 셈) 둘 다 | `.cpp:738-741` |
| 볼 곳 편향 | (이동 ∨ 멈춤 중) ∧ 굽이 있음 → `Prefer = Beyond`(굽이 **너머**), 없으면 속도 방향 | `BeginSweep` `:1457-1474` |
| 새 이동 발행 시 | `PausedCornerIndex = LastPausedCornerIndex = −1` 리셋 | `:1639-1640` |

거동: 굽이 5 m 앞부터 눈이 굽이 너머로 기울고 → 1.5 m 앞에서 0.8 s 멈춰 콘 스윕이 그 너머를 훑고(경계도가 떨어진다) → 다시 간다. 개인이 할 수 있는 "모퉁이 확인"의 몫.

**진짜 파이 자르기 — 모퉁이를 비스듬히, 한 조각씩 여는 것 — 는 경로 모양(path shaping)이고 안 했다**(`.h:281-283`). 이것은 그 중 "멈춰서 본다" 반쪽이다 → [W85] 유지.

---

## 8. 검증 상태

| 무엇 | 상태 |
|---|---|
| ①②③④ 묶음(섹터 arc · 띠 넓히기 · 볼 곳 항상 · AimPoint/IsScanning · 순찰 · solid · 미지=열림 · 도달성/거부 · 유예 · Hold 기울기 · CQB · MinStance · W83) | ✅ 사용자 빌드·PIE — **"잘 되는 거 같음. 이제 정상적이다"**: 적군이 존으로 돌아옴 · REJECT 루프 없음 · 순찰이 움직임 |
| **A · B · C** (7절) | ~~**[B] 빌드 전** — 다음 빌드 뒤 [C-153]~~ → ✅ **밤 빌드·PIE, 사용자 "잘됨"**(첫 빌드 로그의 코너 멈춤 루프는 12.1절로 고친 뒤). [C-153]의 숫자 판정(미세 이동 반복 · 0.5의 후함)은 여전히 미측정 |
| `SoldierScanTurnComponent`가 실제로 도는가 | ~~**미확인**~~ → ✅ **포즈 세션 PIE**(`animation/2026-09-18_ai_pose_layer_scanturn_gait_smoother.md` 6절) — 몸이 돌고 TIP 이 나온다. [W89] 틈은 밤에 코드로 닫음(12.4절) |
| **코너 멈춤 루프**(12.1절) | ✅ 수정 뒤 PIE — 스폰 옆 굽이에서 60 s 도는 것 사라짐(사용자 "잘됨"에 포함) |
| **긴장도 · 걸음**(13절) | ✅ **PIE "잘됨"** — 조용한 병사가 걷고, 접촉·총성 뒤 조깅, 20 s 반감으로 다시 걷기. 포즈 세션 `GaitBridge`가 소비 |
| **포즈 급박도 · 스무더**(14~15절) | ✅ 평상시 "괜찮아 보임" · 급할 때 1프레임 점프 → **포즈 측 해결**("구현 완료", P167) |
| 값 | [C-148]~[C-153] 하나도 안 잼 · 밤 [C-155]~[C-156] 추가(기본값 그대로, "잘됨"은 거동 확인이지 숫자 판정이 아님) |
| **레벨** — `L_SoldierTest`에는 큐브뿐이라 린·미세 위치를 시험할 **얇은 수직 엄폐(원기둥)** 가 필요 · 실제 레벨(New_kadex_0811)의 **나무 콜리전이 Sight 채널을 Block하는가** — 안 막으면 호라이즌·섀도우·콘 스윕·엄폐 트레이스 전부 숲을 광장으로 읽는다 | **미확인** → [W88] |

로그 판정 재료: `[Cover] … eyes=N watch=0|1 … | HERE … o… | best … o… @(dx,dy) … MOVE|stay|DISCARD|MOVING … (dwell)` · `[Cover] … REJECT (x,y,z) unreachable for 30s` · 순찰은 눈 0 홀더의 `MOVE` 목적지가 존 반경 안에서 30 s 단위로 옮겨 가는가(`o` 항이 후보마다 다른가) · A는 눈 2 이상일 때 `hide` 후보가 나무 뒤에 생기는가 · B는 `MOVE … 30cm` 짧은 이동이 나오고 **반복되지 않는가** · C는 `MOVING`이 이어지는 중에 0.8 s 정지 + 보라 화살표가 굽이 너머로 도는가.

---

## 9. 남은 것

| # | 무엇 | ID |
|---|---|---|
| 진짜 파이 자르기(경로 모양) | 7.3절 — 멈춰서 보기만 있다(밤에도 **변함없음**) | [W85] |
| ~~A/B/C 빌드 + PIE 판정~~ → A/B/C **숫자** 판정 | 거동은 "잘됨"; 미세 이동 반복 · `HiddenGazeFraction 0.5`의 후함 · 코너 멈춤 0.8 s 체감 | [C-153] |
| ~~ScanTurn 연동 확인 · 섹터만 있을 때 `bScanning=false` 틈~~ | ~~1.4~1.5절~~ → 포즈 세션 확인 · 12.4절 | ~~[C-152] · [W89]~~ 둘 다 닫힘 |
| **긴장도·걸음 값** — `TensionHalfLifeSeconds 20` · `JogTension 0.3` | 13절 | [C-155] |
| **급박도 값** — `Urgency*` 7개 | 14절 | [C-156] |
| `MinStance`를 titan DT/콘솔/존에 | 5절 · 16절(잠입 현황) | [W84] 절반 |
| 얇은 원기둥 배치(시험 레벨) · 나무 Sight 채널 확인(실제 레벨) | 8절 · 16절 | [W88] |
| ~~`RejectedCandidates` 만료 정리~~ | ~~3.1절~~ → 12.3절 | ~~[W90]~~ 해결 |
| 잠입 접근에서 **우리 몫이 아닌 것** — 내비 가중 경로 · 걸음 매핑(GASP) · `MinStance` titan DT · 나무 Sight 콜리전 | 16절 | [W70] · [W84] · [W88] |

---

## 10. 값 — 신설 (전부 [C])

```
FSoldierAssignment / FSoldierSquadOrder   PatrolWeight 0 (기본 — 존이 1.0을 준다) · PatrolStaleSeconds 30 · MinStance 0
ASoldierZone                              PatrolWeight 1.0 · PatrolStaleSeconds 30   (bUseSector true · SectorHalfWidthDeg 45 — 뜻이 arc로)
ASoldierObjective                         PatrolWeight 1.0 · PatrolStaleSeconds 30   (신설)
USoldierCoverComponent                    WatchApproachBias 0.5 · WatchTravelBias 1.0 · ScanDwellSeconds 2 · MoveGraceSeconds 0.75 · CandidateRejectSeconds 30
                                          [B] HiddenGazeFraction 0.5 · CoverAcceptanceRadiusCm 20 · MicroStepCm 30 ·
                                          [B] CornerAngleDeg 35 · CornerLookAheadCm 500 · CornerStopCm 150 · CornerPauseSeconds 0.8
                                          (기존 StallSpeedCms 20 · CandidateMergeCm 150 · MinDwellSeconds 3 · DwellBreakMargin 1.0 · QuietEyeWeight 0.3 이 판정에 같이 쓰임)
USoldierSituationFieldSubsystem           WidenFromCm = 셀 × 8 (상수, MarkClearAlongRay) · 볼 곳 화살표 250 cm (상수)
USoldierScanTurnComponent (포즈 세션)     StartDegrees 20 · StopDegrees 5 · TurnRateDegPerSec 180 · MaxSpeedCms 10 · bOnlyWhileScanningOrInContact true

★ 밤 4차 (12~14절)
USoldierEngagementComponent  [Gait]      TensionHalfLifeSeconds 20 · JogTension 0.3                                            → [C-155]
                             [Pose]      UrgencyIdle 0.15 · UrgencyContactIdle 0.35 · UrgencyLookPeek 0.3 · UrgencyShootPeek 0.6 ·
                                         UrgencyRetreat 0.7 · RetreatUrgencySeconds 0.6 · UrgencyReload 1.0  (제압은 자기 값 그대로 max)  → [C-156]
                                         알람 창 = 총성 1 s (상수, .cpp:1402) · 조준점 거리 3000 cm (상수, .cpp:1321)
USoldierSituationFieldSubsystem           GetExposure 미지 = clamp(UnknownPresence 0.5 × AmbientWeight 1) = 0.5 (Project Settings 값의 곱, 새 값 없음)
USoldierCoverComponent                    코너 멈춤 "같은 굽이" 반경 = CornerStopCm 150 재사용 (새 값 없음)
```

→ [C-148] 순찰 · [C-149] CQB 볼 곳 · [C-150] 이동 강건성 · [C-151] MinStance · ~~[C-152] ScanTurn~~(포즈 측 확인) · [C-153] 얇은 엄폐 A/B/C · **[C-155] 긴장도·걸음 · [C-156] 급박도**. 포즈 층의 축별 속도(`ScaleAtCalm 0.5`/`ScaleAtUrgent 1.6`, Stance 1.6/0.9/5 …)는 [C-154](포즈 세션).

---

## 11. 변경 파일 (이 세션)

```
Source/SoldierLab/AI/SoldierSituationField.h/.cpp   GetStaleVantage · StalenessAt · AmbientFromOpenness(PreferDir/PreferWeight/ArcYawDeg/ArcHalfWidthDeg) ·
                                                     GetMostExposedDirection(동 파라미터) · GetExposureByStance → bool · FHorizon::bSolid(BakeHorizons · FindDarkestCells) ·
                                                     MarkClearAlongRay 띠 넓히기 · ContradictLightsAlong SharpRadius 필터 · DrawDebug 링 annulus 메시 + 화살표 arc
Source/SoldierLab/AI/SoldierCover.h/.cpp             PatrolCost · TaskCost Hold 기울기 + 순찰 · Objective 수비 순찰 · BeginSweep 볼 곳 항상 + 편향/arc/굽이 ·
                                                     EvaluatePosition 미지=열림 · 활동도 가중 은폐(A) · AddMicroCandidates/AddCandidate bMicro(B) ·
                                                     FinishSweep 유예/도달성/무진전/거부/ScanDwell/CurrentPath/수용 반경 · RejectCandidate/IsRejected ·
                                                     FindNextCorner/UpdateCornerPause/IsPausedAtCorner(C) · 값 12개 · [Cover] watch=
Source/SoldierLab/AI/SoldierObjective.h/.cpp         PatrolWeight · PatrolStaleSeconds · 수비 비용 밴드 밖 기울기
Source/SoldierLab/AI/SoldierEngagement.h/.cpp        IsScanning/bScanning · 무접촉 분기 AimPoint · 섹터 arc 우선순위(볼 곳 > 섹터 중심) · MinStance 바닥
Source/SoldierLab/Squad/SoldierOrderTypes.h          PatrolWeight · PatrolStaleSeconds · MinStance (배정·명령) · bHasSector 주석(arc)
Source/SoldierLab/Squad/SoldierSquadSubsystem.cpp    BuildAssignment 복사 3필드
Source/SoldierLab/Squad/SoldierZone.h/.cpp           PatrolWeight · PatrolStaleSeconds · ApplyToOrder
Source/SoldierLab/Pose/SoldierScanTurnComponent.h/.cpp   (포즈 세션 — 읽기 전용)

★ 밤 4차 (12~17절)
Source/SoldierLab/AI/SoldierCover.h/.cpp             UpdateCornerPause: LastPausedCornerLocation(자리 기억) · 재개 시 LastMoveIssuedSeconds = Now · 눈 나타나면 즉시 재개 ·
                                                     RejectCandidate 만료 RemoveAll                                   (1909 → 1931줄)
Source/SoldierLab/AI/SoldierEngagement.h/.cpp        ESoldierGait · GetDesiredGait/GetTension(TensionHalfLifeSeconds · JogTension · LastAlarmSeconds) ·
                                                     GetPoseUrgency(Urgency* 7 · RetreatUrgencySeconds · bWasPeekingLastTick · RetreatUntilSeconds) ·
                                                     bScanning = 무접촉 조준점 있음 · [Engage] 로그 `tension gait urg`     (1502 → 1586줄)
Source/SoldierLab/AI/SoldierSituationField.cpp       GetExposure 미지 = UnknownPresence × AmbientWeight (max)               (2491 → 2501줄)
Source/SoldierLab/Pose/SoldierGaitBridgeComponent.h/.cpp · SoldierPoseSmootherComponent.h/.cpp   (포즈 세션 — 읽기 전용, 소비 측)
```

빌드: 사용자가 함(P13/메모리). ~~A/B/C는 빌드 전.~~ → 밤에 A/B/C + 4차분 빌드·PIE "잘됨". Perforce 상태는 이 세션에서 확인 안 함.

---

## 12. 밤 — 빌드 뒤 로그가 잡은 코너 멈춤 루프, 그리고 문서 세션이 짚은 넷 [A · PIE ✅]

7절 A/B/C를 빌드해 13:15 세션(`L_SoldierTest`, 명령 있음, `SoldierLab.Debug.Cover.Log 1`)을 돌렸다. P10 — 다시 로그.

### 12.1 Enemy_A `HERE o4.21` 60 s 동일, `MOVE` 0.82 s 간격 — 굽이는 인덱스가 아니라 자리다 (P173 · P174)

**증상**: 한 적군의 `[Cover]` 줄이 60 s 동안 `HERE … o4.21`로 한 글자도 안 바뀌고, `MOVE`가 **0.82 s마다** 찍힌다(= `CornerPauseSeconds 0.8` + 틱 하나). 스폰 옆 1 m.

**루프** (전부 코드에서 재구성 [A]):

```
FinishSweep → MoveToLocation, LastMoveIssuedSeconds = Now, PausedCornerIndex = -1
  → 경로의 첫 굽이가 스폰에서 CornerStopCm 150 안                   (UpdateCornerPause: FindNextCorner ∧ DistXY ≤ 150)
  → PauseMove 0.8 s                                                   (LastPausedCornerIndex = i)
  → 0.8 s 뒤 ResumeMove                                               ← 폰은 아직 속도 0
  → 다음 틱 FinishSweep: Moving ∧ !bMakingProgress ∧ !bJustIssued     (유예 0.75 s는 멈춤 0.8 s 동안 이미 만료)
  → "정지"로 읽혀 재결정 → 같은 후보로 새 MoveToLocation
  → 새 경로, 새 인덱스 → 같은 굽이가 인덱스 i' ≠ i 로 "아직 안 멈춘 굽이"
  → PauseMove 0.8 s … (반복)
```

두 결함이 맞물렸다. ① **굽이 기억이 경로 인덱스였다** — 재탐색은 같은 자리에 새 번호를 준다. 경로 인덱스에 키를 건 기억은 전부 재탐색에서 죽는다. ② **재개된 이동이 "방금 발행한 이동"으로 안 읽혔다** — 유예(P162)는 발행 시각에서 재는데 멈춤이 유예보다 길어(0.8 > 0.75) 재개 순간 이미 만료, 속도 0 = 정지.

**수정** (`SoldierCover.cpp:750-813` · `.h:776-779`):

| # | 무엇 | 어디 | 원칙 |
|---|---|---|---|
| ① | `LastPausedCornerIndex` → **`LastPausedCornerLocation`**(`FVector`, 초기값 `(1e9, 1e9, 0)`). 다음 굽이가 이 자리에서 `CornerStopCm 150` 안이면 "이미 멈춘 굽이" → 안 멈춤. `PausedCornerIndex`는 "지금 멈춰 있나"에만 쓴다 | `.cpp:802-807` · `:810-811` · `.h:777-778` | **P173** — 굽이는 자리다 |
| ② | 재개 시 **`LastMoveIssuedSeconds = NowSeconds`** — 재개된 이동은 방금 발행한 이동이다. 다음 0.75 s는 속도 0이어도 "진행 중" | `.cpp:768-774` | **P174** |

새 이동 발행 시 `PausedCornerIndex = -1`만 리셋(`:1662`) — `LastPausedCornerLocation`은 **안 지운다**. 새 목적지의 경로가 같은 굽이를 지나면 거기서 또 멈추지 않는다(굽이당 1회의 뜻이 "이동당"이 아니라 "자리당"으로 바뀜). 다른 굽이면 150 cm 밖이라 멈춘다.

### 12.2 눈이 나타나면 즉시 재개

[C-153] 판정 항목에 "눈이 생기면 멈춤이 즉시 풀리는가 — `SweepEyes` 게이트는 **시작**만 막고 진행 중 멈춤은 0.8 s를 채운다"고 적었던 것(문서 세션 발견). 밤에 고쳤다: 재개 조건이 `NowSeconds >= CornerResumeSeconds || SweepEyes.Num() > 0`(`.cpp:759`) — "모퉁이에 서서 아무것도 안 하는 것은 적이 나타났을 때 가장 나쁜 자리". 재개 뒤 `ResumeMove`는 상태가 `Paused`일 때만(`:762-765`).

### 12.3 `RejectedCandidates` 만료 정리 — [W90] 해결

`RejectCandidate`가 추가 **전에** `RemoveAll(UntilSeconds <= Now)`(`.cpp:483-486`). 스윕마다 후보 × 항목 거리 비교를 하는 목록이라 "지금 거부 중인 것"만 들고 있으면 된다. `IsRejected`(`:500-511`)의 시간 검사는 그대로(추가가 없으면 만료분이 남아 있어도 걸러진다).

### 12.4 `IsScanning()` — 섹터만 있어도 true — [W89] 해결

1.4절의 ⚠(섹터만 있고 굽기 전이면 `bScanning=false` → ScanTurn 이 그 순간 안 돈다). 밤에 `bScanning = true`를 `if (Task.bHasSector || bHaveWatch)` 블록 안으로(`SoldierEngagement.cpp:1298` · `:1322-1325`) — "스캔 중 = 접촉 없이 눈이 갈 곳이 있다"이고, 그 곳이 필드의 방위든 명령의 부채꼴 중심이든 몸이 그리로 돌아야 한다. 볼 곳만 세자 섹터 + 미굽기 땅의 병사가 마지막 전투 쪽을 보고 서 있었다(주석 `:1322-1324`). 접촉 분기와 리셋(`:809`)은 그대로.

### 12.5 `GetExposure` 미지 ≠ 0

시스템 문서 22.2절 끝의 [A·코드] — "`GetExposure`는 bool을 버리므로 경로 위험·Danger 항은 여전히 미굽기 = 0". 밤에 닫았다(`SoldierSituationField.cpp:1874-1884`): `GetExposureByStance`가 false면 `Unknown = clamp(UnknownPresence × AmbientWeight)`(기본 0.5 × 1 = 0.5)를 **하한**으로, 라이트 몫(레벨 0 셀은 있는데 호라이즌만 없으면 `LitStand/LitCrouch`는 채워져 돌아온다)과 max. "숫자 하나만 원하는 읽기(경로 표본 · 기억 항)에게 미지의 땅은 사전값의 열린 땅이지 0이 아니다." P159의 마지막 구멍. → 시스템 문서 24절.

---

## 13. 밤 — 긴장도와 걸음: 조용한 경비는 걷는다 (P175) [A · PIE ✅ "잘됨"]

### 13.1 왜

빌드 뒤 그림: 접촉 0인 병사들이 **어디를 가든 조깅**한다. GASP 의 기본 gait 가 Run 이고 교전 층은 `WantsToSprint`(P150)만 냈으니 "안 뛴다 = 조깅"이었다. 스프린트 규칙이 "속도는 노출로 산다"를 맞게 했는데도 조용한 장면이 틀려 보인 유일한 이유.

### 13.2 긴장도 `GetTension()` (`SoldierEngagement.cpp:1391-1416`)

```
bAlarm  = bUrgent (접촉 ∥ 제압 > 0 ∥ 사선 거부)                                  (:1397, bUrgent :1387)
        ∥ 기록 중 LastGunshotTimeSeconds 가 1 s 안                                (:1398-1407 — 들은/본 총성)
bAlarm → LastAlarmSeconds = Now
Tension = 0.5 ^ ((Now − LastAlarmSeconds) / TensionHalfLifeSeconds 20)           (:1413-1415, 알람 전이면 0)
```

"방금 무슨 일이 있었나"의 반감기. 1분 전에 총 맞은 병사는 1/8 남아 아직 조깅, 조용한 경계는 2~3분 뒤 걷는다(`.h:455-461`). 알람 창의 총성 1 s 는 상수.

### 13.3 걸음 `GetDesiredGait()` (`:1441-1454` · `.h:65-71` · `:190-196`)

```
Sprint  ← bWantsToSprint (명령의 Cautious/Rush 가 이미 손댄 뒤, :1427-1439)
Jog     ← bHasContact ∥ Tension ≥ JogTension 0.3        단, Task.IsActive ∧ Cautious ∧ !bHasContact → Walk
Walk    ← 그 밖
```

Cautious 는 "아는 것이 없으면 긴장돼도 걷는다" — 그것이 cautious 의 뜻. 접촉이 있으면 Cautious 도 조깅(뛰지는 않음). 이 컴포넌트는 **어느 걸음인지만** 말하고 몸에 어떻게 닿는지는 모른다.

### 13.4 소비자 — 포즈 세션 `SoldierGaitBridgeComponent` (계약만, 읽기 전용)

`Pose/SoldierGaitBridgeComponent.cpp:92-97`: 매 틱 `CharacterInputState.WantsToWalk = (GetDesiredGait() == Walk)` 를 리플렉션으로. Sprint 는 `bDriveSprint false`(`.h:37-39`) — 옛 BP 브리지가 `WantsToSprint` 를 계속 소유. Jog 는 GASP 기본 Run 이라 **아무것도 안 쓴다**. AI 전용(`:72`), Engagement 틱 선행(`:25`).

**`GetTension()` 은 소비되지 않는다** — GASP 의 gait 는 이산(Walk/Run/Sprint)이고 AI 입력 크기는 항상 1 이라 "긴장도로 속도를 섞는" 자리가 없다. 긴장도는 걸음의 **재료**이고 디버그 줄에 찍힌다(15절 로그). 사용자 확인 "잘됨"(걷기 나옴 — 포즈 세션 6절과 같은 관찰).

---

## 14. 밤 — 포즈 급박도: AI 는 목표와 숫자 하나만 낸다 (P172) [A · PIE ✅]

### 14.1 왜

Q/E 린 · V/B 앉기 · 맹목사격 축이 AI 에서 **기계적**으로 보였다 — BP 가 플레이어 키 스텝 rate 로 등속 램프하니 딱 시작하고 딱 멈추고, 목표가 도중에 바뀌면 그 자리에서 다시 등속. 논의의 결론:

**AI 층은 목표(TARGET) 4개 + 급박도 숫자 하나를 내고, 움직임(가속 · 순항 · 정착, 축별, 앉기는 내려갈 때 더 빠르게, 목표가 바뀌면 현재 속도에서 이어감)은 포즈 층이 소유한다.** 이유 넷:

1. 교전 층은 이미 **실제 자세를 되읽는다**(`ReadActualPoseAxes` — 노출 회계 · 자세별 총구 학습, 09-15 P115~P116). 몸이 목표에 늦는 것은 설계상 **허용돼 있다** — 회계는 실제 노출로 오르고 총구는 실제 자세에서 나간다. 그러니 AI 가 프레임마다 값을 밀 이유가 없다.
2. 축별 물리(속도 상한 · 가속 · 비대칭)는 **몸의 성질**이지 판단의 성질이 아니다. 판단 층에 두면 판단이 애니메이션 튜닝값을 들고 다닌다.
3. 리플리케이션 — 목표와 급박도는 **바뀔 때만** 나가면 되지만 프레임별 값은 매 틱이다(P5, 리슨서버).
4. **결정은 이산이고 움직임은 연속이다** — 둘은 선의 양쪽에 있어야 한다(`.cpp:1467-1472` 주석).

에디터 분할도 같은 선: **상황별 급박도**(`Urgency*`)는 교전 컴포넌트에, **축별 속도**(`FSoldierPoseAxisMotion`, `ScaleAtCalm/Urgent`)는 포즈 스무더에 — 둘 다 `EditAnywhere`, 디자이너가 "총 맞을 때 앉기"를 고치려면 앞을, "앉는 몸의 무게"를 고치려면 뒤를.

### 14.2 `GetPoseUrgency()` (`SoldierEngagement.cpp:1467-1502` · `.h:205-214` · `:467-497`)

```
Urgency = bHasContact ? UrgencyContactIdle 0.35 : UrgencyIdle 0.15
bPeeking                       → max(·, WantsToFire ∥ Traversing ? UrgencyShootPeek 0.6 : UrgencyLookPeek 0.3)
!bPeeking ∧ Now < RetreatUntil → max(·, UrgencyRetreat 0.7)        RetreatUntil = 내밈이 끝난 순간 + RetreatUrgencySeconds 0.6
(재장전 의도 ∨ 재장전 중) ∧ 엄폐 → max(·, UrgencyReload 1.0)
                                 max(·, SuppressionNow)              제압은 이미 0..1 — 자기 값 그대로
PoseUrgency = clamp(0..1)
```

읽는 법: 총 맞으며 내려가기(제압 1.0 · 재장전 1.0)는 **뚝**, 아무도 안 쏘는데 내다보기(0.3)는 **천천히**, 들어오기(0.7)는 나가기(0.6)보다 급하다 — "밖은 머물 곳이 아니다". 7값 전부 [C-156]; 사용자 "평상시 괜찮아 보임" 뒤 숫자 조정 없음.

### 14.3 소비자 — 포즈 세션 `SoldierPoseSmootherComponent` (계약만, 읽기 전용)

`Pose/SoldierPoseSmootherComponent.cpp:209-215`: `Scale = max(0.05, lerp(ScaleAtCalm 0.5, ScaleAtUrgent 1.6, GetPoseUrgency()))`, 축 4개(`GetDesiredStance/Lean/BlindFireH/BlindFireV`)를 각각 사다리꼴로(`StepAxis`). AI 전용(`:150`), Engagement 뒤 · **액터 틱 앞**(`:45` · `:50`). BP 램프는 `RateVariablesToFreeze {StanceRate, BlindFireRate}` 를 `FrozenRate 0.0001`(`.h:151-155` · `.cpp:163-175`, **0 금지** — 15절)로 얼리고 린 램프 리터럴 핀은 BP 에서 0.0001. 상세는 포즈 세션 문서 3절.

---

## 15. 밤 — 스무더 뒤 "급할 때만 1프레임 점프": AI 층은 축을 쓰지 않는다 [A]

스무더가 들어간 뒤 사용자 관찰: 평상시는 부드러운데 **급한 경우에만** 한 프레임에 목표로 뛴다. 이 세션 쪽에서 확인한 것:

- **AI 층은 어떤 축도 쓰지 않는다** — `ReadActualPoseAxes` 는 리플렉션 **읽기**뿐이고, 교전 층이 BP 변수에 쓰는 것은 없다 [A·코드]. 그러니 "제3의 손"은 이쪽이 아니다.
- **목표는 설계상 한 틱에 0→1 로 뛴다** — 제압 스파이크, 재장전 덕(`DesiredStance = 1`, `:1458-1465`), 조리개 전환(Lean 좌↔우). 목표가 뛰는 것은 결정이 이산이라서고(P172), 그 사이를 메우는 것이 스무더의 일이다. 목표가 안 뛰게 만드는 것은 답이 아니다.
- 그래서 원인은 BP/포즈 쪽에 있어야 했고, 후보 셋을 냈다: ① BP 램프(`FInterpTo`류)의 rate 0 = **즉시 스냅**(램프 함수의 "0"이 정지가 아니라 무한 속도인 경우) ② 옛 BP 브리지가 어떤 조건에서 축을 **직접** 덮어씀 ③ GASP `Crouch()/UnCrouch()` 를 `DesiredStance` 문턱으로 토글 — 이산 캡슐/DB 전환이라 "급할 때만"(평상시 목표는 분수라 문턱을 안 넘음)과 맞는다.
- **이 세션은 어느 것인지 확정하지 못했다.** 사용자가 뒤에 "구현 완료"라 보고했고, 포즈 세션이 원인을 **①로 확정해 문서화했다** — `USoldierAxisLibrary::RampAxisTo` 가 `RatePerSecond <= 0` 이면 `Target` 을 그대로 반환(`Math/SoldierAxisLibrary.cpp:19-22`), `FrozenRate 0.0001` 로 해결, **P167**. `animation/2026-09-18_ai_pose_layer_scanturn_gait_smoother.md` 3.4절. ③은 남은 이산 전환으로 [W91](눈에 띄면).

이쪽의 흔적은 `[Engage]` 로그 끝의 `tension %.2f gait %d urg %.2f`(`SoldierEngagement.cpp:1513` · `:1523`) — 급박도가 뛰는 순간과 축이 뛰는 순간을 로그로 맞춰 볼 수 있게. `SoldierLab.Debug.PoseSmooth 1` 의 `u=`/`ext` 와 짝(포즈 세션 3.5절).

---

## 16. 잠입 접근 — 지금 어디까지 있나 (분대 세션 질문에 대한 답) [A]

"숙여서 은밀히 접근"의 **개인 층은 완성**이다. 있는 것:

| 조각 | 어디 |
|---|---|
| 안 본 땅의 사전값 0.5 · 앰비언트 · 필드 후보(문간→문간) | 잠입 문서 3·8절 · 시스템 문서 |
| 기하 자세(낮은 벽에서 `RequiredStance`) + `MinStance` 바닥 | 5절 · `SoldierEngagement.cpp:799` |
| 조용하면 안 뛰고 **걷는다**(스프린트 규칙 + 13절 Walk) | P150 · P175 |
| 눈이 발을 이끈다(여행 편향) · 코너 멈춤 + 미리 보기 · 도착 머무름 | 4절 · 7.3절 · 12절 |
| 콘 스윕(띠) · Rush 는 이 조심 전부를 푼다 | 1.2절 · 5절 |

시나리오는 이미 `MoveTo r1200 roe=hold spd=cautious agg=0.30` 을 발행한다(분대 2일차 DT). **우리 몫이 아닌 것** — 내비 가중 경로(은폐 경로 계획, [W70] 재저작 쪽) · 걸음 → GASP 매핑(포즈 세션, 됐음) · `MinStance` 를 titan DT/콘솔에([W84] 절반) · 실제 레벨 나무의 Sight 콜리전([W88] — 안 막으면 이 층 전체가 숲을 광장으로 읽는다).

---

## 17. 정정 (2026-09-18 밤)

### 17.1 1.4절 ⚠ — `bScanning` 은 이제 섹터만 있어도 true

12.4절. `SoldierEngagement.cpp:1298-1325`. [W89] 해결, ScanTurn 의 `bOnlyWhileScanningOrInContact true` 기본값 그대로 둬도 틈 없음.

### 17.2 3.1절 ⚠ — `RejectedCandidates` 는 정리된다

12.3절. [W90] 해결.

### 17.3 7.3절 표 — `LastPausedCornerIndex` 는 없다, `LastPausedCornerLocation` 이다

12.1절. 인덱스 기억은 첫 빌드에서 60 s 루프로 드러났다. 표의 `ResumeMove(:751-760)` 도 밤 판 `:759-775`(유예 재시작 + 눈 나타나면 즉시).

### 17.4 8절 A/B/C [B] · ScanTurn 미확인 — 둘 다 닫힘

A/B/C 밤 빌드·PIE "잘됨"(12.1 수정 뒤). ScanTurn 은 포즈 세션 PIE. [C-152] 닫힘, [C-153] 은 숫자 판정만 남음.

---

## 18. 정정 (2026-09-21) — 7.3절 C 코너 멈춤은 **엣지 전진**으로 대체됐다

`UpdateCornerPause` · `IsPausedAtCorner` · `CornerStopCm 150` · `CornerPauseSeconds 0.8` · `LastPausedCornerLocation`(12.1절) 은 **전부 삭제**됐다(grep 0건). 남은 것은 `FindNextCorner` · `CornerAngleDeg 35` · `CornerLookAheadCm 500` 이고 **볼 곳 미리 보기**(`BeginSweep` 의 굽이 너머 편향)에만 쓰인다. 9절의 [W85](진짜 파이 자르기)는 **경로를 휘지 않고 걸음을 값 매기는** 형태로 해결됐다 — 콘 스윕 광선의 엣지 + 필드 쐐기 적분(`GetWedgePresence`, 트레이스 0), 예산 `StepPresenceBudget 6`, 호도 타이머도 없음(P176·P177). 12.1~12.2절의 코너 멈춤 루프 수정(P173·P174)은 그 코드가 사라지면서 **원칙만** 남았다(P174 는 `EndAdvance` 의 `LastMoveIssuedSeconds = Now` 에서 다시 쓰인다). [C-153] 의 C 항목(0.8 s 멈춤 체감)은 판정 대상이 사라졌다 — A·B 만 남는다. → **`ai/2026-09-21_per_squad_field_edge_advance_fire_model.md` 2절**, 판정 [W95] · [C-158].
