# S6 감사 수정 묶음 2 — SoldierLab 게임 스레드 비용 (결과 동일 최적화 + A/B 스위치)

2026-10-06 / 완료(코드, 빌드·측정 대기) / SoldierLab 게임 스레드 비용 — 소켓/컴포넌트 캐시·원점 선컷·필드 광선 dedupe·포즈층 조회 캐시, 전부 결과 동일 + SoldierLab.Perf.* A/B 스위치

> 출발점: `../infra_architecture/2026-09-29_s6_code_audit.md`(titan 루트 기준) §1 후보 1·2 · §2-3 · §2-4② · §2-5 · §3-5 · §3-6 · §7 3·4·5·7 + P3 일부. 묶음 1(데칼·Niagara·프리필·로그·RTSP)은 `../infra_architecture/2026-10-06_s6_fix_batch1.md`.
> 사용자 방침(감사 §8 머리말) 그대로 — **현재 품질 유지, 결과가 바뀌는 최적화는 범위 밖**, 사전 측정 없이 수정 후 S0 절차로 재측정.
> 신뢰도: **[A]** = 프로젝트/엔진 소스 file:line · grep · 로그 · **[B]** = 잠정 · **[C]** = 미측정. 줄 번호는 이 세션 수정 후 기준 근사치(`~`).
> 상태: **코드 완료 · 빌드 안 함 · 측정 안 함** — 기대 효과(ms)는 전부 **[C]**.
>
> ⚠ ID: 이 문서는 **새 번호(C/W 등)를 매기지 않는다** — L절 "등록 후보"를 `OPEN_ITEMS.md` 담당 세션이 번호 부여.

---

## 0. 원칙 — "같은 입력 → 같은 결정"

이번 묶음에서 허용한 변경은 세 종류뿐이다.

1. **변하지 않는 것의 캐시** — 컴포넌트 포인터 · 해석된 소켓(로컬 변환 + 본 인덱스) · `FProperty*` · `UFunction*` · 정적 `FName`. **값(위치·상태)은 여전히 매 호출 새로 읽는다.**
2. **보수적 선컷** — 어떤 경우에도 원래 판정이 통과시킬 수 없는 후보만 미리 버린다(여유 마진 포함).
3. **멱등 쓰기의 중복 제거** — 다시 써도 결과가 같은 쓰기를 건너뛴다(삽입 순서도 보존).

**모든 새 경로에 cvar** — 기본 1, **0 = 같은 빌드 안의 옛 코드**. 즉 빌드 한 번으로 A/B 와 회귀 의심 시 즉시 되돌리기가 된다.

| cvar | 기본 | 덮는 것 | 절 |
|---|---|---|---|
| `SoldierLab.Perf.SocketCache` | 1 | 눈/표적/총구 소켓 해석 캐시 | A |
| `SoldierLab.Perf.ComponentCache` | 1 | Identity 가 자기 액터의 Engagement/Sight/Cover 포인터 보관 | B |
| `SoldierLab.Perf.DistancePrecut` | 1 | Sight 살아있는 후보 · 투사체 제압의 액터 원점 거리 선컷 | C |
| `SoldierLab.Perf.PrecutMarginCm` | 300 | 위 선컷의 여유 마진(cm) | C |
| `SoldierLab.Perf.FieldRayDedupe` | 1 | `MarkClearAlongRay` 의 셀/부모 키 중복 쓰기 생략 | D |
| `SoldierLab.Perf.PropertyCache` | 1 | PoseSmoother `FProperty*` · AIBridge `UFunction*` 캐시 | E |

---

## A. 소켓 캐시 — `SoldierLab.Perf.SocketCache` (감사 §2-3) [A]

### A.1 무엇

- `USoldierIdentityComponent` 의 눈/표적/총구 소켓 조회(`GetSocketOrFallback` 계열, `AI/SoldierIdentity.cpp`)가 **메시 컴포넌트 + 해석된 소켓**을 캐시한다:
  - 메시에 `USkeletalMeshSocket` 이 있으면 그 **로컬 변환 + 본 인덱스**,
  - 소켓 없이 본 이름이면 **본 인덱스만**.
- 위치는 여전히 **매 호출 계산**: `(SocketLocal * Mesh->GetBoneTransform(BoneIndex)).GetTranslation()`.
  - 엔진 `USkinnedMeshComponent::GetSocketTransform`(`Runtime/Engine/Private/Components/SkinnedMeshComponent.cpp:3525-3570`)과 **같은 산술** [A].
  - 옛 경로가 매번 하던 것: `DoesSocketExist`/`GetSocketBoneName`(`:3709-3737`) + 에디터 빌드의 **선형 소켓 탐색**(`SkeletalMesh.cpp:5281-5316` `FindSocketInfo`, `#if WITH_EDITOR`) [A].
- **무효화**: 메시 컴포넌트가 바뀌거나 그 스키닝 애셋(`GetSkinnedAsset`)이 바뀌면 다시 해석.
- **소켓 오버라이드**(`AddSocketOverride`)는 참조하지 않는다 — 프로젝트와 엔진 Runtime 어디에도 호출자가 없다 [A · grep]. 누가 쓰기 시작하면 이 캐시가 어긋난다 (추정 — 그때 cvar 0 으로 확인).

### A.2 메시 선택 — "첫 스켈레탈 메시"에서 `ACharacter::GetMesh()` 로 [A · 로그]

- 옛 코드는 `FindComponentByClass<USkeletalMeshComponent>` = **첫 번째** 스켈레탈 메시. 감사 §2-3 의 부수 결함 지적: `WeaponMesh` 를 집을 수 있다.
- 조사: 레벨에서 로드된 액터의 `OwnedComponents` 순서는 `ForEachObjectWithOuter` 로 재구성된다(엔진 `Actor.cpp:1183` → `:487-539`) → **생성 순서로 보장되지 않는다** [A].
- 실측 증거: `Saved/Logs/titan_example-backup-2026.10.05-21.52.44.log` 에서 병사 **40명 전원**(`Friendly_C_0..24`, `Hostile_C_0..14`)이 `[SoldierIdentity] ... heights measured` 를 찍었다. 이 로그는 고른 메시에 `spine_03` 과 `head` 가 있어야 나온다 → **매번 몸 메시가 골라졌다** [A].
- 그래서 캐릭터는 `ACharacter::GetMesh()` 로 바꿨다(결과 동일 — 위 로그 기준). 캐릭터가 아닌 액터(브리지 차량 등)는 기존대로 첫 스켈레탈 메시(캐시만 추가).
- ⚠ 혹시라도 옛 경로가 어떤 실행에서 `WeaponMesh` 를 골랐다면, 그 경우엔 새 경로가 **다른** 결과를 낸다(이쪽이 맞는 쪽). cvar 0 이 옛 거동을 복원한다.

### A.3 쿡 빌드

- 쿡 빌드의 소켓 조회는 `SocketMap.Find` 라(감사 §0) 이 캐시의 절감은 **패키지에서 더 작다** [A 메커니즘 · C 수치].

---

## B. 컴포넌트 포인터 캐시 — `SoldierLab.Perf.ComponentCache` (감사 §1 후보 1·2, §2-5) [A]

- `USoldierIdentityComponent::BeginPlay` 에서 **레지스트리 `Register` 전에** 자기 액터의 Engagement / Sight / Cover 를 해석해 약참조(`TWeakObjectPtr`)로 보관. 접근자 `GetEngagementComponent()` / `GetSightComponent()` / `GetCoverComponent()`.
- 사용처: Engagement `CountTeammatesOn`(레지스트리 전원 × `FindComponentByClass<USoldierEngagementComponent>`) · `IsTeammateFacing`(팀원마다 `FindComponentByClass<USoldierSightComponent>`) · Cover `SquadCost` / `IsEnemyCoveredByTeammate`(팀원마다 Engagement 조회).
- **브리지 차량**(`Soldiers/SoldierLabBridgeSubsystem.cpp:187-197` 은 Identity **만** 붙인다)은 세 포인터가 null → 옛 `FindComponentByClass` 실패와 **같은 결과** [A].
- 감사 §2-5 가 제안한 "프레임당 팀원 **스냅샷**(클레임·총구·조준점·접촉)"은 **하지 않았다** — 값 스냅샷은 같은 프레임 안에서 앞선 병사의 갱신을 못 보게 만들 수 있어 "값은 라이브" 원칙에 어긋난다(추정 — 실제 차이 유무는 미확인). **포인터만 캐시, 값은 매번 읽는다.**
- 덤: `USoldierScanTurnComponent` 가 매 틱 `FindComponentByClass<UCharacterMovementComponent>` 대신 `ACharacter::GetCharacterMovement()`(감사 §2 표 아래 참고 줄) — cvar 없음(같은 포인터) [A].

---

## C. 액터 원점 거리 선컷 — `SoldierLab.Perf.DistancePrecut` + `PrecutMarginCm 300` (감사 §2-3) [A 코드 · B 마진 충분성]

### C.1 적용 위치

| 어디 | 기준 반경 | 옛 순서 |
|---|---|---|
| `USoldierSightComponent` 살아있는 후보 루프 | `SightRangeCm` + 마진 | `GetTargetLocation`(소켓) → 거리 컷 |
| `ASoldierProjectile::ApplySuppressionAlongSegment` | `WhizBroadPhaseRadiusCm` + 마진 | 레지스트리 전원 소켓 위치 → 브로드페이즈 |

- **캐릭터만** 선컷한다. 차량 표적은 표적점이 바운즈의 비율이라 원점 기준 상한이 없다 → 옛 경로 그대로.
- **Sight 의 시체 루프에는 적용 안 함** — 래그돌 메시가 캡슐에서 떠내려가 원점과 메시가 멀어질 수 있다.

### C.2 마진 300 cm 근거

- 머리 ≈ 캡슐 중심 위 80 cm, 린 ≈ 70 cm, 소켓 없는 폴백 110/160 cm → 원점과 실제 판정점의 차는 이보다 작다 [A 코드 · 수치는 코드 상수].
- 레지스트리에는 **살아있는 병사만** 있다: `USoldierHealthComponent` 가 `SetSimulatePhysics`(래그돌) **전에** 등록 해제한다(`SoldierHealth.cpp` ~604-610, 래그돌은 그 뒤) [A 코드] → 선컷 대상에 래그돌이 섞이지 않는다.

### C.3 클라이언트 쪽 조사 (코드 변경 없음) [A 코드]

- `ASoldierProjectile::Tick` 에는 권한 게이트가 없다 → **클라의 코스메틱 탄도** `ApplySuppressionAlongSegment` → `USoldierSuppressionComponent::ApplyNearMiss` → `ApplyImpulse` + `Perception::ReportThreatenedBy` 를 클라 사본에 돈다.
- 그 클라 측 값의 소비자:
  - Engagement · Cover — 권한 게이트가 있어 클라에선 안 씀.
  - **`USoldierMovementProfileComponent::TickComponent` 는 권한 게이트가 없고** `GetSuppression()` 을 읽는다(`SoldierMovementProfile.cpp` ~268) → `ChooseSituation` → `WriteGaitSpeeds` 로 **클라 CMC 의 gait 속도**를 쓴다.
  - `SoldierLab.Debug.Suppression` 오버레이.
- 따라서 감사 §2-3 이 말한 "투사체 틱 권한 게이트"를 넣으면 **클라의 gait 속도 값이 바뀐다** → 이번 묶음 범위 밖, 그대로 두고 등록 후보로(L절 3번).

---

## D. 필드 광선 dedupe — `SoldierLab.Perf.FieldRayDedupe` (감사 §2-4 ②) [A]

- `USoldierSituationFieldSubsystem::MarkClearAlongRay(int32, ...)`:
  - 한 광선 안에서 **이 광선이 이미 클리어한 레벨 0 셀**(최근 키 8개 링)은 건너뜀.
  - 거친 레벨마다 **마지막으로 표시한 부모 키**와 같으면 건너뜀.
- **멱등성 확인** [A]:
  - `WriteClear` = `Presence=0`, `PresenceTime=Now` — 같은 광선 안 반복은 같은 값.
  - `MarkDirtyUp` = `bDirty=true` 만 세운다.
  - `bDirty` 를 지우는 것은 **거친 셀 읽기**뿐(`SoldierSituationField.cpp` ~1173-1177) — 광선 진행 중에는 거친 셀을 읽지 않는다.
  - **반복만** 건너뛰므로 맵 삽입 순서가 바뀌지 않는다.
- **§2-4 ① (`ReportSighting` 의 400×400×40000 cm `ProjectPointToNavigation`) 은 고치지 않았다** — 셀 캐시 GroundZ 로 바꾸면 값이 달라질 수 있다(결과 변경). 대신 하위 스코프 `Field: ReportSighting GroundZ` 로 **계측만** 추가 → 측정값으로 별도 결정(L절 2번).

---

## E. 포즈층 조회 캐시 — `SoldierLab.Perf.PropertyCache` (감사 §3-5, §3-6) [A]

- **`USoldierPoseSmootherComponent`**: 틱당 13회 `FindPropertyByName`(엔진 `Property.cpp:2482-2492` PropertyLink 선형) → **오너 클래스별 `FProperty*` 캐시**. 못 찾은 이름도 기억(miss 캐시), 오너 클래스가 바뀌면 비움.
- **`USoldierAIBridgeComponent::CallEvent`**: 소총 `Shoot` / `StartReload` 의 `UFunction` 을 소총 클래스별로 캐시(`TWeakObjectPtr<UFunction>`). **호출 자체와 BP 의 발사율 게이트는 그대로.**
- 감사 §3-6 의 "C++ 에서 발사 간격 게이트"는 **의도적으로 안 했다** — 발사 타이밍이 달라질 위험(L절 5번).

---

## F. cvar 없는 변경 (자명하게 동일) [A]

| 어디 | 변경 |
|---|---|
| `Pose/SoldierAIBridgeComponent.cpp` | `"Muzzle"` · `"pelvis"` · `"Shoot"` · `"StartReload"` → 정적 `FName` |
| 같은 파일 DebugAxes 6블록 | `if (DebugAxes && DebugAxes->IsAxisDisplayEnabled())` 로 감쌈 — `SetAxis*` 는 꺼져 있을 때 원래 no-op |
| `Debug/SoldierDebugAxes.h/.cpp` | 틱이 `SoldierLab.Debug.Axes` 를 따른다: `BeginPlay` 에서 틱 활성 = cvar · cvar→0 뒤 첫 틱이 행을 비우고 틱을 끔 · cvar→1 콜백이 `TObjectIterator` 로 전원 재활성 (감사 §7-8 P3) |
| `AI/SoldierMovementProfile.cpp` `SituationRowName` | 정적 256칸 `FName` 표(DataTable `FindRow` 와 간격은 그대로) (감사 §7-8 P3) |
| `Pose/SoldierScanTurnComponent.cpp` | `GetCharacterMovement()`(B절) |

---

## G. 새 stat (`STATGROUP_SoldierLab`, 선언 `AI/SoldierLabLog.h` · 정의 `.cpp`)

| 시스템 | 스코프 |
|---|---|
| Engagement | `SelectTarget` · `PickCandidate` · `CountTeammatesOn` · `ComputeLookValue` · `IsTeammateFacing` · `PlanAperture` · `IsFriendlyInLineOfFire` |
| Sight | `Candidates`(살아있는 후보 루프 — 트레이스 · MarkClear · ReportSighting 포함) |
| Field | `MarkClearAlongRay` · `ReportSighting` · `ReportSighting GroundZ`(nav 투영, 변경 없음) |
| Cover | `SquadCost` · `IsEnemyCoveredByTeammate` |
| 투사체 | `Projectile Tick` · `Projectile: Suppression` |
| 포즈층 | `PoseSmoother Tick` · `AIBridge TickBridge` |
| 카운터 | DWORD `Calls: CountTeammatesOn` |

화면 라벨은 `Engagement: SelectTarget`처럼 `<시스템>: <이름>` 형식(`SoldierLabLog.h:71-89`, 하위 스코프는 앞 공백 2칸 들여쓰기), `Projectile Tick` · `PoseSmoother Tick` · `AIBridge TickBridge` 는 최상위. 중첩 스코프라 하위 합 > 부모가 정상(09-21 Cover 하위 스코프와 같은 규약, `ai/2026-09-21_perf_instrumentation_and_cover_cost.md`).

---

## H. 변경 파일

```
Source/SoldierLab/AI/SoldierLabLog.h/.cpp                    G(stat 선언·정의)
Source/SoldierLab/AI/SoldierIdentity.h/.cpp                  A(소켓 캐시 · GetMesh) · B(컴포넌트 포인터)  ← 비-UPROPERTY 멤버 추가
Source/SoldierLab/AI/SoldierSight.cpp                        A · B · C(선컷) · G
Source/SoldierLab/AI/SoldierEngagement.cpp                   B · G
Source/SoldierLab/AI/SoldierCover.cpp                        B · G
Source/SoldierLab/AI/SoldierSituationField.cpp               D · G
Source/SoldierLab/AI/SoldierMovementProfile.cpp              F(FName 표)
Source/SoldierLab/Weapons/SoldierProjectile.cpp              C(제압 선컷) · G   (묶음 1 수정 보존)
Source/SoldierLab/Pose/SoldierPoseSmootherComponent.h/.cpp   E(FProperty 캐시) · G  ← 멤버 추가
Source/SoldierLab/Pose/SoldierAIBridgeComponent.h/.cpp       E(UFunction 캐시) · F · G  ← 멤버 추가
Source/SoldierLab/Pose/SoldierScanTurnComponent.cpp          B(GetCharacterMovement)
Source/SoldierLab/Debug/SoldierDebugAxes.h/.cpp              F(cvar 따르는 틱)  ← BeginPlay 오버라이드 추가
```

## I. 빌드 메모

- 헤더 레이아웃 변경(Identity · PoseSmoother · AIBridge 에 비-UPROPERTY 멤버, DebugAxes 에 `BeginPlay` 오버라이드) → **에디터 닫고 정식 빌드**. Live Coding 금지.
- 새 `UPROPERTY`/`UFUNCTION` 없음.
- `SoldierIdentity.h` 는 **titan_example 모듈(브리지)도 include** → 두 모듈 모두 재컴파일.
- URO 는 건드리지 않았다(P186).

---

## J. 측정 절차 (사용자 실행 — S0 와 같은 조건, `../infra_architecture/2026-09-29_perf_baseline_s0.md`)

1. New_kadex_0811 데모, 교전 피크 **M2** 에서 `stat dumpave -num=120 -ms=0.1` ×2 + `stat SoldierLab` 스크린샷.
2. 한 번은 Perf cvar 전부 1(기본), 한 번은 전부 0:
   ```
   SoldierLab.Perf.SocketCache 0
   SoldierLab.Perf.ComponentCache 0
   SoldierLab.Perf.DistancePrecut 0
   SoldierLab.Perf.FieldRayDedupe 0
   SoldierLab.Perf.PropertyCache 0
   ```
   가능하면 **같은 교전 안에서** 토글. 절감을 항목별로 귀속하려면 개별 토글.
3. S0 대비: Cover 2.40 · Engagement 아군 2.37 + 적 0.65–1.56 · Sight 1.35 · BP ReceiveTick 0.91 ms.

### J.1 판정 기준 [C]

- Perf=1 에서 Engagement / Cover / Sight 의 Self 가 내려가는가.
- 새 하위 스코프로 `CountTeammatesOn` / `IsTeammateFacing` 이 Engagement 를 지배했는지 → 감사 §1 의 **[B] → [A] 또는 기각**.
- Field: `ReportSighting GroundZ` 값이 nav 투영의 (결과가 바뀌는) 후속 최적화를 할 가치가 있는지 결정.

### J.2 거동 동일성 확인

실행이 비트 단위로 결정적이지 않다(프레임 타이밍·물리) → 다음 세 가지로 비교:

- **(a) 교전 중 즉시 토글**: `SoldierLab.Debug.Engagement.Log` 의도 스트림 / `Debug.Sight` 에 눈에 띄는 불연속이 없어야.
- **(b) 시나리오 흐름 로그**(`Saved/ScenarioLogs` `cycles.csv`: 사이클 길이 · 진영별 사망 · 발사 수)를 Perf=1 vs 0 여러 사이클 — **분포가 겹쳐야**.
- **(c) 쿡 빌드 재측정**(나중): 에디터는 소켓 탐색이 선형, 쿡은 `SocketMap` — SocketCache 절감은 패키지에서 더 작다.

> **2026-10-06 측정 보류 → [s6_verification](../../infra_architecture/2026-10-06_s6_verification.md) P절**: -game 1회 시도(1→0→1)는 덤프 사이 교전 강도 변동으로 무효, 그리고 GPU 급저하 현상(같은 문서 F3) 해결 전까지 A/B 보류. J.1 판정은 여전히 [C].

---

## K. 이번 범위 밖 (같은 패턴, 손대지 않음)

> → 묶음 3(2026-10-06)에서 처리, **M절** — 아래 목록 중 §3-7(calm 스윕 간격)만 여전히 미처리(결과 변경).

- Engagement `IsContactExcluded` 의 기록마다 `FindComponentByClass<USoldierIdentityComponent>`(`PickCandidate`/`SelectTarget`).
- Cover `GatherThreatEyes` 의 `FindComponentByClass<Identity>`(~`SoldierCover.cpp:300`).
- Cover 틱의 `FindComponentByClass<UCharacterMovementComponent>`(~`:2227`).
- Sight 시체 루프의 `FindComponentByClass<Identity>`.
- Suppression `ApplyNearMiss` 의 `FindComponentByClass<Perception>`.
- AIBridge 매 틱 이름 기반 소켓 조회 — `WeaponMesh` `"Muzzle"` · 몸 `"pelvis"`(에디터 빌드 선형 소켓 탐색).
- 감사 §3-7(calm 스윕 간격) — 결과가 바뀜(재결정 지연).

---

## L. OPEN_ITEMS 등록 후보 (번호는 담당 세션이 부여)

`soldier_ai_lab/OPEN_ITEMS.md` 담당 세션이 등록할 것 — 이 문서는 번호를 매기지 않는다(같은 목록을 `OPEN_ITEMS.md` 끝 "등록 후보" 절에도 적어 둠).

1. **[C] S6 묶음 2 측정** — M2 에서 Perf=1/0 A/B, 하위 스코프 분해(J.1 판정 기준).
2. **[C] `ReportSighting GroundZ`(nav 투영 400×400×40000) 비용** — 크면 셀 캐시 GroundZ 사용 같은 **결과가 바뀌는** 최적화를 별도 결정.
3. **클라 코스메틱 탄의 제압이 클라 MovementProfile gait 에 들어간다**(MovementProfile 에 권한 게이트 없음) — 의도인지 결정. 바꾸면 클라 gait 값이 바뀐다(C.3).
4. **같은 패턴의 남은 조회**(K절 목록) — 묶음 2 범위 밖, 손대지 않음.
5. **감사 §3-6 잔여: `Shoot` 의 C++ 발사 간격 게이트** — 발사 타이밍이 바뀌어 의도적으로 안 함.
6. **감사 §2-5 스냅샷 안은 "값은 라이브" 규칙으로 기각** — 현재 수정은 포인터만 캐시.

---

## M. 후속 — 묶음 3(2026-10-06): K절 조회 꼬리

> 상태: **코드 완료 · 빌드 안 함 · 측정 안 함**. titan 쪽(Part A — 탐지 행렬 · 시나리오 로그 싱크 · Monitor1 창 · UGV/RCWS 조회 캐시 · AmbientFX 선로드 · RCWS 풀 정리 · WindSource 스코프)은 `../infra_architecture/2026-10-06_s6_fix_batch3.md`(titan 루트 기준).
> 원칙은 0절 그대로 — 포인터/해석 결과만 캐시, 값은 매번 읽음, 새 경로마다 cvar(0 = 옛 코드).

### M.1 새 cvar `SoldierLab.Perf.IdentityMap` (기본 1) [A]

- `AI/SoldierLabLog.h` ~103-109 선언(`GSoldierPerfIdentityMap`), `.cpp` ~91-93 정의. **0 = 매번 `FindComponentByClass<USoldierIdentityComponent>`**.

| cvar | 기본 | 덮는 것 | 절 |
|---|---|---|---|
| `SoldierLab.Perf.IdentityMap` | 1 | 레지스트리의 액터→Identity 맵 조회 | M.2 |
| (기존) `SoldierLab.Perf.ComponentCache` | 1 | + Identity의 Perception 포인터 · Cover 틱 CMC | M.3 |
| (기존) `SoldierLab.Perf.SocketCache` | 1 | + AIBridge `Muzzle`/`pelvis` 소켓 해석 | M.4 |

### M.2 액터→Identity 맵 — `USoldierRegistrySubsystem` (`AI/SoldierIdentity.h/.cpp`) [A]

- 맵: `TMap<TObjectKey<AActor>, TWeakObjectPtr<USoldierIdentityComponent>> IdentityByActor`(`.h` ~491-492).
- `NoteIdentity`(Identity `BeginPlay`, `Register` **전**, `.cpp` ~636 · ~656-663) / `ForgetIdentity`(Identity `EndPlay`, `Unregister` **뒤**, ~649 · ~667-680 — 값이 자기일 때만 제거).
- `FindIdentityOf(actor)`(~685-700): 맵 hit + 그 Identity의 오너 == actor면 반환, 아니면 `FindComponentByClass` 폴백. 정적 `FindIdentityOfActor(actor)`(~705-713)는 액터의 월드 레지스트리를 거침(레지스트리 없으면 탐색).
- **⚠ 설계 편차 — 맵은 "등록(Register/Unregister)"이 아니라 Identity *컴포넌트 수명*과 동기** [A 코드]:
  - 지시는 "맵은 Register/Unregister와 동기"였지만, `USoldierHealthComponent`는 **사망 시 등록 해제**하고(`SoldierHealth.cpp` ~608) 시체는 Identity를 그대로 가진다.
  - Sight의 **시체 루프가 묻는 것이 바로 등록 해제된 몸**이다 → 등록 동기 맵이면 그 루프에선 항상 miss(= 절감 0, 폴백만).
  - 액터당 Identity 1개 보장: 브리지는 없을 때만 붙인다(titan `Soldiers/SoldierLabBridgeSubsystem.cpp` ~185-190) → **맵의 답 == `FindComponentByClass`의 답**.
- 호출처:
  | 어디 | 변경 |
  |---|---|
  | Engagement `IsContactExcluded`(~628) | `USoldierRegistrySubsystem::FindIdentityOfActor(Enemy)` |
  | Cover `GatherThreatEyes`(~300-302) | `FindIdentityOfActor(ThreatActor)` |
  | Sight 시체 루프(~471) | `Registry->FindIdentityOf(Body)` |

### M.3 컴포넌트 포인터 추가 — `SoldierLab.Perf.ComponentCache` [A]

- Identity에 `CachedPerception`(`.h` ~370, BeginPlay ~629) + `GetPerceptionComponent()`(~595-600; cvar 0 = 탐색).
- Suppression `ApplyNearMiss`(~136-137): `Identity->GetPerceptionComponent()` 사용(Identity가 null이면 탐색 폴백).
- Cover 틱 회피 스위치(~2227-2233): `ACharacter::GetCharacterMovement()`(cvar 0 = `FindComponentByClass`).
  - **참고**: 이 블록은 **첫 틱과 `GSoldierCoverAvoidance` 값이 바뀔 때만** 돈다 — 매 틱 아님. 절감은 무시할 수준이고 K절 목록을 비우려고 함.

### M.4 AIBridge 소켓 해석 캐시 — `SoldierLab.Perf.SocketCache` [A]

- `Pose/SoldierAIBridgeComponent.h` ~164-177: private `FBridgeSocketLookup MuzzleSocketLookup` / `PelvisSocketLookup` + 정적 `GetSocketTransformCached(InMesh, Socket, Lookup)`(`.cpp` ~757-). 사용: 총구 ~458(`WeaponMesh` `"Muzzle"`), 골반 ~720(몸 `"pelvis"`). cvar 0 = `GetSocketTransform`.
- 메시 컴포넌트 + 스키닝 애셋당 1회 해석: `USkinnedAsset::FindSocketInfo` → 없으면 `GetBoneIndex`.
- 매 호출 계산은 `SocketLocal * GetBoneTransform(BoneIndex)` / `GetBoneTransform` / 컴포넌트 변환 — 엔진 `USkinnedMeshComponent::GetSocketTransform` RTS_World와 같은 산술(`Runtime/Engine/Private/Components/SkinnedMeshComponent.cpp:3525-3570`, `GetSocketInfoByName` `:3591-3616`) [A].
- 차이:
  - 소켓 오버라이드는 참조하지 않음(호출자 없음 — A.1과 같음).
  - 메시에 스키닝 애셋이 없으면 엔진은 호출마다 경고 로그, 캐시 경로는 안 남김(무기/몸은 항상 애셋 있음).
- **의도적 미변경**: 왼손 그립 IK 소켓 조회(`UpdateLeftHandIK`) — K절 목록에 없었고, 주석상 "PIE에서 소켓 편집이 즉시 보이도록 매 틱 읽는다".

### M.5 안 한 것 (지시대로 — 결과 변경)

- 감사 §3-7 calm 스윕 간격 · §2-4① `ReportSighting` nav 투영 · 투사체 틱 권한 게이트(C.3) · C++ `Shoot` 발사 간격 게이트(E절).

### M.6 변경 파일

```
Source/SoldierLab/AI/SoldierLabLog.h/.cpp            M.1 cvar
Source/SoldierLab/AI/SoldierIdentity.h/.cpp          M.2 맵 · M.3 CachedPerception   ← 멤버 추가(titan_example도 include)
Source/SoldierLab/AI/SoldierEngagement.cpp           M.2
Source/SoldierLab/AI/SoldierCover.cpp                M.2 · M.3
Source/SoldierLab/AI/SoldierSight.cpp                M.2
Source/SoldierLab/AI/SoldierSuppression.cpp          M.3
Source/SoldierLab/Pose/SoldierAIBridgeComponent.h/.cpp  M.4                          ← 멤버 추가
```

빌드: 헤더 변경 → 에디터 닫고 정식 빌드, `SoldierIdentity.h` 때문에 **두 모듈 모두**. 새 UPROPERTY/UFUNCTION 없음(SoldierLab 쪽).

### M.7 측정 절차 [C]

J절과 같다 — M2 교전, `stat SoldierLab` + `stat dumpave -num=120 -ms=0.1`, Perf cvar 전부 1 vs 0에 **`SoldierLab.Perf.IdentityMap 0`** 추가. 같은 교전 토글로 J.2 (a)/(b)/(c) 거동 동일성 확인. 기대 효과는 Engagement `PickCandidate`/`SelectTarget`(IsContactExcluded) · Sight `Candidates`(시체 루프) · `AIBridge TickBridge` 의 Self 감소 — 크기 미측정.

> **2026-10-06 측정 보류 → [s6_verification](../../infra_architecture/2026-10-06_s6_verification.md) P절** — `IdentityMap` 포함 6개 토글 1회 시도는 무효(교전 강도 변동), GPU 급저하(F3) 해결 후 재실시. [C] 유지.

### M.8 OPEN_ITEMS 등록 후보 (번호는 담당 세션이 부여)

같은 목록을 `OPEN_ITEMS.md` 끝 "등록 후보 — 2026-10-06 S6 묶음 3" 절에도 적어 둠.

1. **[C] S6 묶음 3 측정** — IdentityMap / ComponentCache / SocketCache A/B(M.7).
2. **설계 메모: Identity 맵은 등록이 아니라 컴포넌트 수명과 동기** — 시체 루프가 등록 해제된 몸을 묻기 때문(M.2). 나중에 "등록 동기"로 바꾸면 Sight 시체 루프가 항상 폴백.
3. **AIBridge 왼손 그립 IK 소켓 조회(`UpdateLeftHandIK`, 매 틱 이름 조회)** — 같은 패턴 잔여, PIE 소켓 편집 즉시 반영 의도라 미변경.
4. **WindSource 등록 방식 전환 보류**(titan 쪽 후보지만 기록) — 에셋 생산자(`BP_UGV_0901` 등)의 "Wind" 태그 확인 필요, `stat TitanWind` 측정 후 결정(`../infra_architecture/2026-10-06_s6_fix_batch3.md` A2).
