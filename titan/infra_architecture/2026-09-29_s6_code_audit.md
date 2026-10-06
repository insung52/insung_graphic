# S6 프로젝트 C++ 전수 감사 — 병목·누수·구조

2026-09-29 / 완료(조사·보고 전용, 코드 변경 없음) / Engagement 10배는 잎 투과율이 아님(호출 경로 없음, [A]) — 비용은 교전 중 Self(비트레이스) 70~88%, 유력 원인은 레지스트리×`FindComponentByClass` O(R·N) 루프; 확정 누수는 데칼 캡 무효(2곳)·RTSP appsrc 이중 unref(크래시 위험), 최대 미귀속 GT 항목은 `AWindSource` 0.57~0.66 ms.

> 신뢰도: **[A]** = 소스/엔진 소스 file:line 또는 S0 로그(`Saved/Logs/titan_example-backup-2026.09.29-02.34.40.log`) 수치. **[B]** = 추정.
> 등급: **P1** = S0 실측 ≥0.3 ms(또는 그 부모 스코프가 실측) 또는 누수/크래시 확정 · **P2** = 규모(병사·재시작·스트림·시간)에 따라 커지는 구조 · **P3** = 부록.
> 방법: 묶음 A~F + Engagement 전담 = 서브에이전트 7개(읽기 전용) → 이 세션이 핵심 주장(데칼 캡, 임팩트 Niagara, appsrc ref, 소켓 선형탐색, `AlwaysTickPoseAndRefreshBones`, WindSource 수치)을 원본 file:line으로 재확인.

---

## 0. 전제 — S0 수치 해석 시 주의

- **S0는 비쿡 에디터 exe(`-game`) 측정**이다. 에디터 빌드에서 소켓 조회 `USkeletalMesh::FindSocketAndIndex`는 **메시+스켈레톤 소켓 선형 탐색**이고(엔진 `Runtime/Engine/Private/SkeletalMesh.cpp:5245-5262` `#if WITH_EDITOR`), 쿡 빌드는 `SocketMap.Find`(`:5265-`)다 [A]. 아래 소켓 기반 비용(§2-3·§2-4·§2-5)은 패키지에서 더 작다. 09-21 기준선도 PIE라 같은 조건이지만, 전시 빌드 수치는 쿡 빌드로 재측정해야 한다.
- S0 문서의 "Engagement 2.37 ms"는 **아군 25명 그룹만**이다. 적 11명 그룹이 0.65~1.56 ms 더 있어 **실제 합은 2.4~3.7 ms/frame** [A](로그 29920·30073·32447·32506·39095·39238행).

---

## 1. Engagement 10배 원인 판정

### 판정: 잎 투과율(09-28 `ForestCanopyRegistrar`) 가설은 **기각 [A]**

- `ComputeTransmittance` 호출자는 `USoldierSightComponent::FoliageTransmittance`(`SoldierLab/AI/SoldierSight.cpp:150-167`, 사용처 `:264 :392 :466 :516`)와 `titan_example/Detection/TargetDetectionComponent.cpp:461-476` 두 곳뿐. `SoldierEngagement.cpp`는 `SoldierFoliageOcclusion.h`를 include하지 않고 Sight에서 `SightRangeCm/SightHalfAngleDeg`만 읽는다(`:745-748`).
- S0 dumpframe에서 `Foliage Queries`(0.252 ms / 93회)는 `BP_Drone_C.TargetDetection` 밑에만 나온다. `Engagement Tick` 자식은 `Self`와 `SceneQueryTotal → RaycastSingle → SoldierShotLane` 둘뿐.
- `ForestCanopyRegistrar` 자체도 틱·델리게이트·타이머 없음, `OnWorldBeginPlay` 1회(크라운 39,482개 등록 ≈ 41 ms, 재시작 시 재실행 안 함, SourceId 교체라 누적 없음) [A] — `Environment/ForestCanopyRegistrar.cpp`, `SoldierLab/AI/SoldierFoliageOcclusion.cpp:51-61`.
- 확인 A/B(코드에 이미 있음): `SoldierLab.Foliage.Enabled 0`(`SoldierFoliageOcclusion.cpp:15`) — Engagement가 내려가지 않아야 한다(오히려 접촉이 늘어 오를 수 있음).

### 비용이 실제로 있는 곳 [A] (dumpframe 자식 분해)

| 프레임 | 그룹 | 합계 | Self | 레인 트레이스(회) |
|---|---|---|---|---|
| 835 | 아군×25 | 1.706 | 1.455 | 0.250 (39) |
| 68 | 아군×25 | 2.150 | 1.649 | 0.501 (80) |
| 224 | 아군×25 | 1.778 | 1.498 | 0.280 (39) |
| 452 | 아군×25 | 2.343 | 2.062 | 0.281 (53) |
| 836 | 적×11 | 0.653 | 0.529 | 0.123 (19) |
| 68 | 적×11 | 1.562 | ≈0.76 | 0.805 (28) |
| 130/576(접촉 전) | 아군×25 | 0.146/0.058 | — | — |

- **Self(비트레이스 C++)가 70~88%**, 병사당 58~82 µs(접촉 없을 때 2~6 µs) → **접촉(기록 수) 구동 비용**.
- 09-21은 Self ≈ 0.15 ms, 레인 23~35회 0.07~0.10 ms(`soldier_ai_lab/ai/2026-09-21_game_thread_structural_pool_rays_bridge.md:84-85`). 트레이스 1회 3 µs → 5~28 µs는 숲 콜리전 프록시를 지나는 긴 레인과 부합 [B].

### Self 원인 후보 (순위) [B — 샘플링 프로파일로 확정 필요]

> 후보 1·2 — **수정됨(2026-10-06, 검증 대기) → `soldier_ai_lab/ai/2026-10-06_s6_fix_batch2_game_thread.md`** B절(Identity가 Engagement/Sight 포인터 보관, `SoldierLab.Perf.ComponentCache`) + 하위 stat 스코프(`CountTeammatesOn` · `IsTeammateFacing` 등)로 이 순위의 [B] 판정 가능. — A/B 측정은 2026-10-06 보류(GPU 급저하, [s6_verification](2026-10-06_s6_verification.md) P절·F3), [B] 유지.

1. **`CountTeammatesOn` O(R×N)** — `SoldierEngagement.cpp:602-636`. `PickCandidate`가 자기를 위협하지 않는 기록마다 호출(`:690-692`), `SelectTarget`에서 한 번 더(`:898`). 매 호출이 레지스트리 전원을 돌며 `FindComponentByClass<USoldierEngagementComponent>`(엔진 `Actor.cpp:4018-4035` 선형 IsA 스캔). 아군 ≈ 11기록×26명 ≈ 286, 적 ≈ 27×10 ≈ 270 스캔/병사/틱 — **두 진영의 병사당 Self가 거의 같다는 로그와 일치**.
2. **`ComputeLookValue → IsTeammateFacing`** — `:772-831 → :718-769`. 팀원마다 `FindComponentByClass<USoldierSightComponent>` + `GetEyeLocation`(소켓 조회, §2-3).
3. **브리지 차량 2대(09-23 이후 titan 레벨 전용)** — `Soldiers/SoldierLabBridgeSubsystem.cpp:160-199`가 `BP_TitanTruck4`·`BP_UGV_0901`에 아군 Identity를 붙인다. 모든 팀원 루프·적의 기록 목록에 들어가며, 차량에서는 `FindComponentByClass`가 실패할 때까지 큰 컴포넌트 목록 전체를 훑고 `GetEyeLocation/GetTargetLocation`이 `GetActorBounds(true)`(전 프리미티브 순회, `SoldierIdentity.cpp:47-56`, 엔진 `Actor.cpp:2265-2279`)로 떨어진다. **09-21 L_SoldierScenario엔 없던 요인**.
4. **레인 캐시 미스** — 허용치 15 cm / 0.15 s(`SoldierEngagement.h:899,902`), 38 fps에서 조깅 병사는 프레임당 ≈15 cm라 거의 매 프레임 미스, 미스마다 `FCollisionQueryParams` + `GetAttachedActors`(`:399-406`). 무엄폐 분기는 `PlanAperture`(최대 7트레이스) 매 틱(`:1284`).
5. 사소: 타깃 이름 FString(`:1146`), `IsFriendlyInLineOfFire`(`:1541`).

**결론**: 10배는 "새 시스템 하나"가 아니라 **본 레벨의 교전 밀도(기록 수 R) × 팀원 수 N × 루프 안 컴포넌트/소켓 조회**의 곱이 09-21보다 커진 것 + 차량 Identity 추가 + 숲 콜리전으로 비싸진 레인 트레이스 [B]. 담당: **SoldierLab AI 소유 세션**(S3는 애니 쪽이라 무관).

---

## 2. P1 목록

| # | 원인 file:line | 내용 | 예상 비용 | 근거 | 담당 | 수정 방향 · 위험 |
|---|---|---|---|---|---|---|
| **2-1** — **수정됨(2026-10-06, 검증 대기) → `2026-10-06_s6_fix_batch1.md`** — 부분 검증(2026-10-06 교전 중 220 · 재시작 후 1, 피크 미측정 → [s6_verification](2026-10-06_s6_verification.md) B1-1) | `SoldierLab/Weapons/SoldierProjectile.cpp:1159-1170` · `titan_example/Vehicles/RCWSProjectile.cpp:1060-1071` | **데칼 FIFO 캡(`MaxActiveImpactDecals` 200)이 아무것도 안 지움.** `SpawnDecalAtLocation`이 데칼 Owner를 `World->GetWorldSettings()`로 만들고(엔진 `GameplayStatics.cpp:2256`), 캡은 `OldestOwner->Destroy()` → `UWorld::DestroyActor`가 WorldSettings 파괴를 거부(`LevelActor.cpp:862-866`, return false). 목록에서만 빠지고 데칼은 수명(60 s 표면/적, 30 s 지면 혈흔 — `SoldierProjectile.h:98,284,352`)까지 산다. 정적 배열 `RemoveAt(0)`은 O(n). | 명중 20~40/s × 60 s ≈ **1,200~2,400개 상주**(설계 200) — GT 컴포넌트 등록 + GPU 디퍼드 데칼(숲 정면은 GPU 바운드) | 코드 [A] · 개수 [B] | SoldierLab AI(무기) · titan RCWS | `Oldest->DestroyComponent()`. 위험 낮음. [C-82]가 200 캡을 동작하는 것처럼 기술 — 정정 필요 |
| **2-2** — **수정됨(2026-10-06, 검증 대기) → `2026-10-06_s6_fix_batch1.md`** — 평탄 확인 미완(290→325, 다른 시점 → [s6_verification](2026-10-06_s6_verification.md) B1-2) | `SoldierProjectile.cpp:1077-1079` | 임팩트 FX `SpawnSystemAtLocation(..., bAutoDestroy=true, bAutoActivate=false)` 기본 `ENCPoolMethod::None`(엔진 `Plugins/FX/Niagara/.../NiagaraFunctionLibrary.h:93`) → **명중마다 새 `UNiagaraComponent`**. 언리라이어블 멀티캐스트라 모든 프로세스에서 발생. | S0 dumpframe `Multicast_PlayImpactEffect` **0.26~0.96 ms/frame(1~2회)**, 로그 30388·32622·32704·35630행. S0 "Niagara GT 0.46"의 상당 부분 [B] | [A] | SoldierLab AI(무기) | `ENCPoolMethod::AutoRelease`(User.Hit* 파라미터는 Activate 전에 매번 다시 씀). 위험: 시스템 MaxPoolSize, 새 컴포넌트 상태 의존 NS |
| **2-3** — **수정됨(2026-10-06, 검증 대기) → `soldier_ai_lab/ai/2026-10-06_s6_fix_batch2_game_thread.md`** A·C절(소켓 해석 캐시 `SoldierLab.Perf.SocketCache` · 원점 선컷 `SoldierLab.Perf.DistancePrecut` · 캐릭터는 `GetMesh()`; 프레임 스탬프 캐시 대신 소켓 해석만 캐시, 위치는 매 호출 계산. "첫 스켈레탈 메시가 WeaponMesh일 수 있음"은 확인함 — 2026-10-05 로그에서 병사 40명 전원 heights 로그 = 매번 몸 메시 [A]. 투사체 틱 권한 게이트는 클라 gait 값이 바뀌어 미적용) | `SoldierLab/AI/SoldierIdentity.cpp:20-73`(`GetSocketOrFallback`) ← `SoldierSight.cpp:68,115` · `SoldierProjectile.cpp:384-400` · `SoldierSuppression.cpp:87` · `SoldierEngagement.cpp:582` · `SoldierCover.cpp`(§2-5) | **`GetTargetLocation/GetEyeLocation` 무캐시.** 매 호출 `FindComponentByClass<USkeletalMeshComponent>`(≈38 컴포넌트 선형) + `DoesSocketExist` + `GetSocketLocation`(에디터 빌드 선형 소켓 탐색, §0). Sight는 **거리 컷 전에**(`PassesCheapRejects`), 투사체 제압(`ApplySuppressionAlongSegment`)은 **브로드페이즈 컷 전에** 레지스트리 전원에 호출(이 세션이 `SoldierProjectile.cpp:384-400` 재확인). 투사체 틱엔 권한 게이트 없음 → 클라 코스메틱 탄도 같은 비용. 부수 결함: "첫 스켈레탈 메시"라 `WeaponMesh`를 집을 수 있음. | Sight 40×~20 ≈ 800/스캔 + 비행 탄 ~21×40 ≈ 840/frame, 0.5~1 µs → **Sight 안 0.4~0.8 + 투사체 틱 0.4~0.8 ms**(투사체 틱은 stat 스코프 없음) | 엔진 [A] · 비용 [B] | SoldierLab AI | `GetActorLocation()`+마진으로 선컷, Identity에 `GFrameCounter` 스탬프 프레임 캐시, `ACharacter::GetMesh()` 사용. Engagement·Cover·Sight·투사체를 한 번에 줄이는 **공통 해법**. 위험 낮음 |
| **2-4** — **②만 수정됨(2026-10-06, 검증 대기) → `soldier_ai_lab/ai/2026-10-06_s6_fix_batch2_game_thread.md`** D절(`SoldierLab.Perf.FieldRayDedupe`) · **①nav 투영은 미변경 — 하위 스코프 `Field: ReportSighting GroundZ`로 계측만** | `SoldierSituationField.cpp:1681 → GroundZAt :851-881`(extent `:866`, `GroundSearchCm=20000 :86`) · `MarkClearAlongRay :1019-1057`(+`WriteClear :1008`, `MarkDirtyUp :910-920`) ← `SoldierSight.cpp:424-425` | **Sight 1.35 ms(실측 부모) 내부 후보 2개.** ① `ReportSighting`이 목격마다 400×400×40000 cm `ProjectPointToNavigation`(실패 시 300 m 트레이스) — 호출자가 이미 발 위치를 넘김. ② 광선 2 m 스텝마다 `TouchCell` + 최대 3 `WriteClear`(각 `MarkDirtyUp` 2 `FindOrAdd`), 이웃 셀이 같은 8 m 부모라 중복 다수. | ① ≈100 투영/스캔 × 2~5 µs ≈ 0.2~0.5 ms ② ≈16k 해시/스캔 ≈ 0.3~0.6 ms | [B] | SoldierLab AI | ① 셀 캐시 GroundZ 사용 ② 이번 스캔 이미 클리어한 셀 스킵·마지막 부모키 dedupe. **먼저 Sight 하위 스코프 추가로 분해** |
| **2-5** — **수정됨(2026-10-06, 검증 대기) → `soldier_ai_lab/ai/2026-10-06_s6_fix_batch2_game_thread.md`** B절(`SoldierLab.Perf.ComponentCache`) — **아래 제안한 스냅샷 없이**: 값은 라이브로 읽고 컴포넌트 포인터만 Identity에 캐시 | `SoldierLab/AI/SoldierCover.cpp:1722-1815`(`SquadCost` ← `ScorePosition :1840`) · `:1669-1720`(`IsEnemyCoveredByTeammate`) | **Cover Score의 O(N²)**: 후보 평가마다 레지스트리 전원 × `FindComponentByClass<USoldierEngagementComponent>` + 접촉 팀원은 `GetMuzzleLocation`(소켓) + `GetAimPoint`. | ≈1,600 반복/frame × 0.3~0.5 µs ≈ 0.5~0.8 ms. Score 서브스탯 09-21 35명 **0.51 ms** [A], S0 Cover 2.40(max 4.9) | [A] 부모 · [B] 비중 | SoldierLab AI | 레지스트리에 프레임당 팀원 스냅샷(클레임·총구·조준점·접촉) 1회 구축 → 40회 소켓 읽기로. 위험 낮음. [W102] 범위 밖(신규) |
| **2-6** — **스코프만 추가(2026-10-06), 등록 방식은 보류 → `2026-10-06_s6_fix_batch3.md`** A2(`stat TitanWind`; "Wind" 태그가 `BP_UGV_0901` 등 에셋에도 있어 생산자 전수 열거 불가) — **실측 후 종결(2026-10-06, 같은 문서 7-3): 현재 평균 ≈0.1 ms/프레임(Rescan 1 Hz ≈0.7 ms 스파이크 · Push 0.02 · MPC 0.01 · 대상 Niagara 3개), 등록 방식 전환 불필요.** 아래 S0 0.57–0.66 ms 빈도는 현재 코드로 재현 안 됨(09-30 DynamicWind 폐기 영향 추정) | `titan_example/Environment/WindSource.cpp:75-100`(Rescan `:230-247`, Push `:92-160,250-268`) | **`AWindSource` Self 0.574/0.633/0.655 ms**(5개 dumpframe 중 3개, 나머지 0.083/0.088) — 로그 21358·32638·43205행(이 세션 재확인). TickInterval 없음, Push 0.05 s(≈2프레임에 1번 = 비싼 프레임 비율과 일치), Rescan 1 s(`TActorIterator` ≈755 액터 × `GetComponents`로 "Wind" 태그 Niagara 수집). MPC(엔진 `ParameterCollection.cpp:969-991` 변화 시만 dirty)·DynamicWind(`DynamicWindSubsystem.cpp:99-110` ENQUEUE 1회)는 싸므로 **태그 Niagara 개별 `SetVectorParameter` × N 또는 Rescan**이 유력 [B]. | 평균 ≈0.3 ms/frame, **양 PC** | 비용 [A] · 원인 [B] | **S2(숲/환경)** | Rescan·각 Push에 스코프 → Niagara Parameter Collection 1개로 전달, 컴포넌트는 등록 방식으로. 위험 낮음 |
| **2-7** — **수정됨(2026-10-06) → `2026-10-06_s6_fix_batch1.md`** — 검증됨(2026-10-06, 5 마운트 × 접속/해제 10 = 50/50, `still-set AppSrc` 0 → [s6_verification](2026-10-06_s6_verification.md) B1-3) | `Plugins/RtspEncoder/Source/RtspEncoder/Private/RtspServerSubsystem.cpp:140-150` · `:109-112` | **GstAppSrc 이중 unref(크래시 위험).** `gst_bin_get_by_name`(transfer full) ref 1개를 `State.AppSrc`와 `MediaData->OwnAppSrc` 두 곳에 저장. "superseded" 경로(새 미디어 configure가 옛 미디어 unprepared보다 먼저 — 주석 `:74-88`이 실제로 일어난다고 명시)에서 `:147`이 unref, 나중에 옛 미디어 `OnMediaUnprepared`가 `:111`에서 같은 포인터를 또 unref → bin 소유 ref까지 소진 → use-after-free. 이 세션이 코드 재확인. 로컬 `Saved/Logs`에 `:144` 경고 문자열 0건(발생 빈도 미측정, LIG/리눅스 PC 로그는 미확인). | 경합 1회당 프로세스 크래시 가능. 12 h 무인 + RC 클라 재접속 × 5~7 마운트 | 로직 [A] · 발생 빈도 [B] | **S1(RTSP)** — 성능이 아니라 수명 버그 | `:147`의 unref 제거(소유자는 `OwnAppSrc` 하나), 또는 `State.AppSrc`용 `gst_object_ref` 추가. 위험 낮음 |

(참고: ScanTurn 0.35 ms(M1)는 `SetActorRotation`의 ≈23 부착 컴포넌트 전파로 이미 [W99]/[W113]/[W121] — 재보고 안 함. `SoldierScanTurnComponent.cpp:77,118`의 매 틱 `FindComponentByClass<UCharacterMovementComponent>`만 사소 추가. — **수정됨(2026-10-06 묶음 2)**: `GetCharacterMovement()`)

---

## 3. P2 요약

| # | file:line | 내용 | 규모 요인 | 근거 | 담당 | 방향 · 위험 |
|---|---|---|---|---|---|---|
| 3-1 | `titan_example/Detection/DetectableTargetComponent.cpp:124-132` | 탐지대상 액터의 **모든 스켈레탈 메시를 `AlwaysTickPoseAndRefreshBones` 강제**(09-21 주석: "적 15명 규모면 무시"). SoldierLab 병사 40명에게도 적용 → 화면 밖 병사도 풀 본 리프레시. (Character 기본은 `AlwaysTickPose`, 엔진 `Character.cpp:125`) | 병사 수 × 화면 밖 비율 | [A] 코드 · 비용 [B] | **S3(병사 애니)**와 협의 · titan 탐지 | `!bIsRevealed` 동안만 강제, 공개 후 기본 복원. 위험: 미렌더 대상 본 기반 조준(캡쳐 뷰는 렌더로 침) — `stat anim` A/B |
| 3-2 — **행렬만 수정됨(2026-10-06, 검증 대기) → `2026-10-06_s6_fix_batch3.md`** A1 · **캐시 `Bounds`·넷모드 게이트는 의도적으로 안 함** | `Detection/TargetDetectionComponent.cpp:562-589`(`:337`에서 호출) · `:26-43,:310` | 코너 8개마다 view×proj 행렬 재구성, 대상마다 전 컴포넌트 `CalcBounds`(스켈레탈은 PhysicsAsset `CalcAABB`, 엔진 `SkinnedMeshComponent.cpp:2271`). 넷모드 게이트 없어 2-PC 양쪽에서 3개 탐지기 모두 실행 | 대상 × 탐지기 × 프로세스. S0 TargetDetection 0.23 ms [A] | [A] 경로 · [B] µs | titan 드론/RCWS | 행렬 스캔당 1회, 캐시된 `Bounds` 사용, 결과를 안 읽는 프로세스에선 스킵 |
| 3-3 — **수정·PIE 검증됨(2026-10-06) → `2026-10-06_s6_fix_batch1.md`** | `Vehicles/RCWSFireControlComponent.cpp:1623` | 발사마다 `UE_LOG(Log)` + `FVector::ToString`×2 | 12 h ≈ 1.7M줄 ≈ 250 MB 로그 | [A] | titan RCWS | Verbose로. 위험 없음 |
| 3-4 | `RCWSProjectile.cpp:263-360`, `.h:175` · `RCWSFireControlComponent.h:628` | RCWS당 풀 64발이 동시에 틱 가능, `MaxFlightTimeSeconds=5`(≈4 km) | 연사 중 64×2×5~10 µs ≈ 0.6~1.3 ms 피크 | [B] | titan RCWS | MaxFlightTime ≈2 s, 투사체 틱 stat 확인 |
| 3-5 — **수정됨(2026-10-06, 검증 대기) → `soldier_ai_lab/ai/2026-10-06_s6_fix_batch2_game_thread.md`** E절(`SoldierLab.Perf.PropertyCache`) | `SoldierLab/Pose/SoldierPoseSmootherComponent.cpp:98,115,134`(호출 `:157,:194,:219-226`) | `FindPropertyByName` 병사당 틱당 13회 무캐시(엔진 `CoreUObject/.../Property.cpp:2482-2492` PropertyLink 선형) | 40×13 ≈ 0.1~0.3 ms, stat 스코프 없음 | 엔진 [A] · ms [B] | SoldierLab AI(포즈층) | BeginPlay에 FProperty* 캐시(AIBridge FVar 패턴). 위험 없음 |
| 3-6 — **UFunction 캐시만 수정됨(2026-10-06, 검증 대기) → `soldier_ai_lab/ai/2026-10-06_s6_fix_batch2_game_thread.md`** E절 · **C++ 발사 간격 게이트는 의도적으로 안 함**(발사 타이밍 변경 위험) | `Pose/SoldierAIBridgeComponent.cpp:420-429,303-312` | `WantsToFire()` 동안 **매 프레임** `FindFunction(FName(TEXT))` + BP `Shoot` ProcessEvent(발사율 게이트는 BP 안) — BP ReceiveTick 0.91 안에 포함 | 사격 병사 10~25 × 3~10 µs ≈ 0.05~0.2 ms | [B] | SoldierLab AI | UFunction 캐시 + C++에서 발사 간격 게이트. [W108] 잔여 |
| 3-7 | `SoldierCover.cpp:2355-2368`(`CalmCandidatesPerTick=2`, `.h:567`) | "calm" 병사 스윕이 프레임 구동 → fps에 비례해 재시작(후보 ≈20 × nav 투영 + FindDarkestCells + `TActorIterator :129`) | fps × calm 병사, M1(전원 calm)에 최대 | [B] | SoldierLab AI | `CalmCandidateIntervalSeconds` 추가. 위험: 순찰 재결정 지연([C-149]/[C-153]류 재확인). [W102]⑤ 확장 |
| 3-8 — **수정됨(2026-10-06) → `2026-10-06_s6_fix_batch3.md`** A3(`titan.ScenarioLog.DiskSyncSeconds`) — PIE로 형식 동일·실행 중 기록·세션 종료 기록 확인(7-2), 강제 크래시 테스트는 미실시 → 강제 크래시 내성 검증됨(2026-10-06), 스냅샷 히치는 GT 참고치 → [s6_verification](2026-10-06_s6_verification.md) B3-1·B3-2 | `titan_example/UI/ScenarioMonitorSubsystem.cpp:307`(및 `:333,342,367,391`) | 로그 줄마다 `EventWriter->Flush()` = OS 디스크 싱크(엔진 `FileManagerGeneric.cpp:1000-1005` → Win `FlushFileBuffers` `WindowsPlatformFile.cpp:933-937`, Linux `fdatasync` `UnixPlatformFile.cpp:323-328`). `WriteWorldSnapshot`(`:722-797`)은 한 프레임에 ≈45 싱크 | 평균 작음, fail-safe 스냅샷·사망 폭주 시 5~100 ms 히치 | [A] 메커니즘 · [B] ms | titan 시나리오 | 싱크는 SNAP 끝/FAIL/SessionEnd/N초마다만. 위험 낮음 |
| 3-9 — **수정됨(2026-10-06) → `2026-10-06_s6_fix_batch3.md`** A4(`RemoveFromParent`는 no-op이라 창 `SetContent(Null)`로 대체) — 자체방호 축 PIE 3회: 실행 중 창 1개·종료 후 0개 확인(7-4), 트래블 경로는 미실시(2026-10-06 검증 회차에서도 미실시 → [s6_verification](2026-10-06_s6_verification.md) B3-3) | `titan_examplePlayerController.cpp:312-341`, `.h:114` | 자체방호 Monitor1 `SWindow`를 `FSlateApplication::AddWindow` 후 파괴 경로 없음(PC에 EndPlay 없음, 소스 전체 `RequestDestroyWindow` 0건). `SObjectWidget`은 FGCObject(UMG `SObjectWidget.h:30,50`)라 위젯·UPROPERTY 참조 유지, 재호출 시 창 2개 | 레벨 세션당 1세트. 12 h 런(트래블 없음)에선 0, PIE 종료·로비↔레벨 이동 시 발생 | [A] 미파괴 · 월드 유지 [B] | titan UI | EndPlay에서 `RequestDestroyWindow`+`RemoveFromParent`, 재생성 가드 |
| 3-10 — **수정됨(2026-10-06) → `2026-10-06_s6_fix_batch1.md`** — 검증됨(2026-10-06, `t.MaxFPS 10` → 10.0 fps → 원복 후 20–24 fps, 멈춤 없음 → [s6_verification](2026-10-06_s6_verification.md) B1-4) | `Plugins/RtspEncoder/.../RtspStreamComponent.cpp:180-188` | 프레임 페이싱 누산기 무클램프 → fps 저하 뒤 회복 시 매 틱 제출(SDP 초과 = 헤더 `:383-389`가 경고하는 클라 멈춤), PTS가 벽시계와 괴리 | 스트림 5/7 × 누적 저하 시간(12 h 누적) | [A] | S1 | `Min(acc - Interval, Interval)`. 위험 낮음 |
| 3-11 — **이미 해소 — 10-01 비동기 인코드 파이프라인이 FIFO 제거, `RtspStreamComponent.cpp:266` 주석**(2026-10-06 확인, `2026-10-06_s6_fix_batch1.md`) | `RtspStreamComponent.cpp:264-283` + `NvencD3D12Encoder.cpp:584-596,313-316` | `PendingSubmitWallClockSeconds` 무상한 — `EncodeFrame` false(크기 불일치·예외) 때 pop 없음. UGV CCTV는 OutputResolution 없음(`Vehicles/VehicleRtspBridgeComponent.cpp:105-111`) → 대시보드 리사이즈 시 영구 이 경로 | 30/s/스트림, 12 h ≈10 MB/스트림, 이후 pop마다 O(n) memmove(렌더 스레드) | [A] | S1 | false 반환 시 비우기 또는 ≈8개 캡, 링버퍼 |
| 3-12 | `Plugins/RtspEncoder/.../FNvencVulkanEncoder.cpp:303` | 인코더마다 `cuCtxCreate`(리눅스 자체방호 7개) | 1회성 VRAM ≈100~300 MB × 7 | [B] | S1(VRAM 예산) | `cuDevicePrimaryCtxRetain` 공유. 위험 중(리눅스 재검증) |
| 3-13 | `RtspServerSubsystem.cpp:163,477` | appsrc 기본 `block=FALSE`·leaky 없음 → 느린/정지 클라에서 큐 증가 가능 | 정지 스트림당 ≈0.5 MB/s | [B](GStreamer 소스 미확인) | S1 | `max-buffers`/`leaky-type=downstream` |

---

## 4. 이미 알려진 것 / 다른 세션 몫 (표시만)

- **→S1**: 수동 `CaptureScene()` 4곳(`QuadCamComponent.cpp:309`, `RCWSComponent.cpp:366`, `TitanTruck.cpp:376`, `DronePawn.cpp:1018`) · 무클라이언트 상태에서도 인코드(`RtspStreamComponent.cpp:216-286`, push에서야 버림 `RtspServerSubsystem.cpp:456-460`) · 리눅스 `ImmediateFlush`+`CopyFence->Wait()` 스트림당 매 프레임(`FNvencVulkanEncoder.cpp:507,516`) · `bDisableSightCapture`여도 `AddViewInformation` 스트리밍 뷰 등록(`RCWSComponent.cpp:313-343,377`).
- **캡쳐 수명은 깨끗**: RT·캡쳐 컴포넌트 전부 BeginPlay 1회 생성 UPROPERTY(Transient), 재시작 재생성 없음; 인코더 세션도 컴포넌트당 1회(스트림 컴포넌트 생성 지점 `VehicleRtspBridgeComponent.cpp:105/149`, `TitanTruck.cpp:249/295`, `DronePawn.cpp:292`). RTSP 위험은 **재시작 횟수가 아니라 시간·재접속·fps 저하**에 비례.
- **메모리 M1→M3 +17(PMC/Sphere/StaticMesh/MID)** = 투사체 풀 17발 성장(발당 생성자에서 각 1개, `SoldierProjectile.cpp:101-178`, MID `:191`) — 정상, 클래스당 96 상한(`SoldierProjectilePool.cpp:11-15,51-69`). 단 투사체 BP가 **2종**(`Content/SoldierLab/Weapons/Blueprints/BP_RifleProjectile`, `Content/Soldiers/Weapons/BP_RifleProjectile`)이라 상한 2×96. Niagara +33 = 트레이서 17 + 미귀속 16(임팩트 FX 순간값 추정, §2-2) [B].
- **09-23 클라 로그 액터 수 747→844 / 59사이클**(`Saved/ScenarioLogs/2026.09.23-17.16.47_New_kadex_0811_client/cycles.csv`) — 코드 주석(`ScenarioMonitorSubsystem.cpp:1327-1329`)은 풀 성장으로 봄. UI/시나리오 코드는 사이클당 영속 액터를 만들지 않음 [A]. 데칼(§2-1)은 컴포넌트라 액터 수엔 안 잡힘 — 다음 장기 런에서 `rcws_rounds/soldier_rounds` 열로 확인.

---

## 5. 구조·레거시 판정 (묶음 E)

- **`Source/titan_example/Soldiers/`는 통째로 죽은 코드가 아니다.** 본 레벨에서 LIVE: `USoldierLabBridgeSubsystem`(틱 서브시스템, 0.25 s 스캔, `Soldiers/SoldierLabBridgeSubsystem.cpp:15-18`), `UNavQueryFilter_EnemySquad1/2/3`(레벨 존/오더의 `NavFilterClass`, 0811.umap 각 1회), `UNavArea_EnemyPath/EnemySquad1~3Path`(내비메시에 베이크, 0811.umap 2/3/3/3회). **폴더 삭제 시 게임이 깨진다** — 먼저 이 클래스들을 이름 유지한 채 이동.
- 구 `UEnemyCombatComponent`·`UAllyFormationComponent`: 0811/lobby 인스턴스 0 → 틱 없음, 다른 모듈의 이중 조회만 남음(항상 null): `RCWSFireControlComponent.cpp:462,837`, `RCWSProjectile.cpp:791-800,817-826`, `DronePawn.cpp:1299-1303`, `ScenarioStateSubsystem.cpp:1820-1826,242-263,2144,3131-3252`. 전부 µs — P3.
- 구 폰(BP_UAV, BP_UGV_Vehicle*, AUAVPawn, AUGVPawn 등) 0811.umap 0건 — 폴백 `GetActorOfClass`는 이벤트성·클래스 해시라 무해.
- `Variant_Combat/Platforming/SideScrolling`, `TestSceneCapture`, `TestCaptureWidget`, `RCWSPreviewActor`: 두 레벨 배치 0, 서브시스템·틱커 없음 → DEAD. `RtspPocTestActor`는 콘솔 명령 `RtspPoc.SpawnTestActors`로만 스폰(`RtspPocCommands.cpp:26,43`).
- `Plugins/Chronicle`은 `"Type": "Editor"` 모듈 — 패키지 게임에 비용 0.
- 무조건 생성 서브시스템: `SoldierSituationField`/`SoldierRegistry`(`SoldierSituationField.h:111`, `SoldierIdentity.h:346`)와 `USoldierProjectilePoolSubsystem`에 `DoesSupportWorldType` 없음 → 로비에서도 생성(엔진 기본 Game/Editor/PIE, `WorldSubsystem.cpp:29-32`), 비용 µs — P3.
- `AUGVAIController`가 PlayerController라 UGV마다 유령 `APlayerState`(GameState PlayerArray에 복제) + 카메라 매니저 생성(엔진 `PlayerController.cpp:1076,1079`) — P3, PlayerArray 순회 코드 주의.

---

## 6. 요청 측정 (한 번에 모음)

| # | 시점 | 명령 | 볼 것 | 판정 대상 |
|---|---|---|---|---|
| 1 | M2 교전 피크 | `obj list class=DecalComponent` · `obj list class=NiagaraComponent` | 데칼 >200이면 §2-1 확정, Niagara 수 | 2-1, 2-2 |
| 2 | M2 | `SoldierLab.Foliage.Enabled 0` ↔ `1` 각각 `stat dumpave -num=120 -ms=0.1` | `Engagement Tick` 변화 없음(또는 상승) | §1 기각 재확인 |
| 3 | M2 | **샘플링 프로파일러**(VS Performance Profiler / Superluminal) 10 s | `USoldierEngagementComponent::TickComponent` 하위의 `CountTeammatesOn`/`IsTeammateFacing`/`FindComponentByClass`/`GetSocketOrFallback` 비중, `USoldierSightComponent`·`ASoldierProjectile::Tick` 하위도 | §1 후보 순위, 2-3~2-5 |
| 4 | M2 | `stat SoldierLab` 라이브 스크린샷 | `Traces: Engagement` 등 DWORD 카운터(dumpframe에 안 나옴) | §1-4 |
| 5 | 재시작 1회차 vs 5회차 같은 시점 | `memreport -full` + `obj list class=SoldierProjectile` / `DecalComponent` / `NiagaraComponent` | 풀 상한 도달 후 평탄한지 | 누수(S5 겸용) |
| 6 | 전시 전 | 쿡 패키지로 M2 재측정 | 에디터 선형 소켓 탐색 제거 후 Sight/Cover/Engagement 수치 | §0 |
| 7 | RC 클라 반복 접속/해제 | 로그 `media-configure found a still-set AppSrc` 검색(LIG·리눅스 PC 포함) | 발생 여부 | 2-7 |

---

## 7. OPEN_ITEMS 등록 후보 (SoldierLab — 번호는 담당 세션이 부여)

1. 데칼 캡 무효(§2-1) — [C-82] 정정 포함.
2. 임팩트 Niagara 비풀링(§2-2).
3. Identity 소켓 포인트 프레임 캐시 + `GetMesh()` 사용(§2-3) — Sight 선컷·투사체 제압 선컷·투사체 틱 권한 게이트·투사체 틱 stat 스코프 포함. — **수정됨(2026-10-06 묶음 2)**: 소켓 해석 캐시·`GetMesh()`·Sight/제압 선컷·투사체 stat 스코프. 권한 게이트는 미적용(클라 gait 값 변경) → `soldier_ai_lab/ai/2026-10-06_s6_fix_batch2_game_thread.md`
4. Engagement Self: `CountTeammatesOn`/`IsTeammateFacing` 컴포넌트 포인터 Identity 보관 + 하위 stat 스코프(`SelectTarget :1084`, `ComputeLookValue :1091`, `PlanAperture`, `IsFriendlyInLineOfFire`) + 브리지 차량 Identity 비용(§1). — **수정됨(2026-10-06 묶음 2)**: 포인터 보관 + 하위 스코프(브리지 차량은 포인터 null = 이전과 같음)
5. Sight 하위 스코프 + `ReportSighting` nav 투영 + `MarkClearAlongRay` dedupe(§2-4). — **수정됨(2026-10-06 묶음 2)**: 스코프 + dedupe. nav 투영은 계측만(결과 변경이라 미수정)
6. Cover SquadCost 팀원 스냅샷(§2-5), calm 스윕 간격(§3-7).
7. PoseSmoother FProperty 캐시(§3-5), AIBridge `Shoot` 게이트(§3-6). — **수정됨(2026-10-06 묶음 2)**: FProperty 캐시 + `Shoot`/`StartReload` UFunction 캐시. C++ 발사 게이트는 의도적으로 안 함
8. P3 묶음: `SoldierDebugAxes` 무의미 틱+FName 17개(`Debug/SoldierDebugAxes.cpp:28-30,122-126`) **[수정됨(2026-10-06 묶음 2): 틱이 `SoldierLab.Debug.Axes`를 따름 + AIBridge 쪽 호출 게이트]**, `SoldierMovementProfile` 스코프·FName·간격(`AI/SoldierMovementProfile.cpp:336-374`) **[FName만 수정됨(2026-10-06 묶음 2): 정적 256칸 표, 간격은 그대로]**, 런타임 `USoundAttenuation` 생성(`SoldierProjectile.cpp:341,1181`, `SoldierAudioLibrary.cpp:96-115` — TStrongObjectPtr 쓰지 말 것), 필드 예산 프레임 기반 + 재시작 후 호라이즌 재베이크(`SoldierFieldSettings.h:80,245,308`).

---

## 8. 후속(2026-10-01) — 품질 유지 수정 목록 · 로그 · 선로딩 · 로딩 화면

> 사용자 방침: **현재 품질을 유지하면서 성능 향상**(품질을 깎는 최적화는 범위 밖), 사전 측정 없이 수정 후 S0 절차로 재측정. 데칼 캡은 **(a) 설계대로 200개 복원**으로 결정.

### 8-1. 남은 플레이 중 로그 (`Saved/Logs/titan_example.log` 3348–7426행, PIE 12.1분, New_kadex_0811 교전 포함) [A]

| file:line | 내용 | 빈도 | 조치 | 담당 |
|---|---|---|---|---|
| `Vehicles/UGVAIController.cpp:1599`(+`:1714`) | `[UGVPursuit]` 18인자 Printf | **211줄/분**(12 h ≈ 30 MB) — `bLogPursuitDiagnostics`(C++ 기본 false, `.h:540`)가 `BP_UGVAIController_new`에서 켜진 것으로 보임 [B] | BP에서 끄거나 Verbose — **2026-10-06: `BP_UGVAIController_new` CDO `bLogPursuitDiagnostics=true` 확인(MCP) → BP CDO에서 끔(코드 무변경) — 수정·PIE 검증됨(2026-10-06, UGV 주행 중 `[UGVPursuit]` 0건) → `2026-10-06_s6_fix_batch1.md`** | UGV |
| `UGVAIController.cpp:575` | 경로 시작마다 멀티라인 JSON 덤프(≈220줄/회, `:587-603`) | ≈36줄/분 | Verbose 또는 점 개수만 — **수정·PIE 검증됨(2026-10-06) → `2026-10-06_s6_fix_batch1.md`** | UGV |
| `RCWSFireControlComponent.cpp:1623` | 발사마다 Log | 9~29줄/분 | Verbose — **수정·PIE 검증됨(2026-10-06) → `2026-10-06_s6_fix_batch1.md`** | RCWS |
| 에셋 `AN_Reload`(Heroes 템플릿 GAS 노티파이) on `ALLY_/Enemy_AM_MM_Rifle_Reload(_Emote_MW)` 4개 몽타주 | `LogAbilitySystem: Error … GameplayEvent.ReloadDone` | 재장전마다(7.4줄/분) — **버그**: SoldierLab 병사엔 ASC 없음 | 몽타주에서 노티파이 제거(에셋 편집) | SoldierLab |
| 엔진 GameplayCameras(병사 스폰마다) | `LogCameraSystem` 2줄/병사/재시작 | 6.6줄/분 | `[Core.Log] LogCameraSystem=Warning` | SoldierLab |
| `Soldiers/SoldierLabBridgeSubsystem.cpp:147` · `SoldierLab/AI/SoldierIdentity.cpp:176` | 병사 스폰마다 1줄 | 재시작당 ≈80줄 | Verbose — **수정·PIE 검증됨(2026-10-06) → `2026-10-06_s6_fix_batch1.md`** | SoldierLab |
| `Drone/DronePawn.cpp:2555-2605`(호출 `:834,:985`) | 화면 디버그 6~7개 Printf **매 프레임** — `bShowFlightDebug` C++ 기본 **true**(`.h:736`), BP 오버라이드 값 미확인 [B] | 매 프레임 | C++ 기본 false — **2026-10-06 확인: `BP_Drone` CDO가 false로 오버라이드, New_kadex_0811 인스턴스 오버라이드 없음 → 화면에 안 나옴, C++ 기본값은 그대로 둠** | 드론 |

### 8-2. 선로딩·워밍업 후보 (결과 동일)

핵심 사실 [A]:
- **PIE 첫 발사 버벅임 = Niagara 런타임 컴파일.** 로그 15건 `LogNiagara: Compiling System … took`(첫 RCWS 발사 06.28.21 프레임 284~286에 NS_Blood_RCWS·NS_MuzzleFlash 등 연속). `fx.Niagara.OnDemandCompileEnabled`가 스크립트 동기 안 맞은 에셋을 첫 활성화로 미룸(엔진 `Plugins/FX/Niagara/.../NiagaraSystem.cpp:198-203,966-970`, `NiagaraComponent.cpp:1352-1360`). NS_BarrelSmoke·NS_AmbientDust·NS_FallingLeaves는 **매 세션** 재컴파일(10.97 s 등) — 저장된 컴파일 ID가 무효. **쿡 빌드엔 없음.**
- **에디터는 컴포넌트 PSO 프리캐싱 꺼짐**: `IsComponentPSOPrecachingEnabled() … && !GIsEditor`(엔진 `Runtime/Engine/Private/PSOPrecache.cpp:141`). 쿡 빌드는 켜짐 → 첫 이펙트가 버벅이는 대신 PSO 준비 전엔 **안 그려질** 수 있음(Niagara 기본 `DelayRendering`).
- **진입 직후 없는 에셋 동기 로드 2건**: `NS_MuzzleFlash_01`(EvolveStudio, 프레임 641), `SC_Gun_shots`(프레임 684) `Failed to find object` — 경로는 `S_WeaponTank`/`S_WeaponWheeledVehicle`에만 있고, 이를 끌어오는 건 `RCWSFireControlComponent.cpp:156-160` `LoadClass(AC_WeaponTank)`(현 차량 미사용) [연결은 B].

| # | 수정 | 위치 | 효과 | 담당 |
|---|---|---|---|---|
| L1 | Niagara 에셋 열어서 컴파일·재저장(Physics Field 타입 경고 해결 포함) | SoldierLab/Effects/*, VFX/RCWS/*, NS_Blood_RCWS, NS_BarrelSmoke, NS_AmbientDust, NS_FallingLeaves | PIE 첫 발사 100~280 ms 히치 + 클라 17 s 백그라운드 컴파일 제거 | 콘텐츠(VFX) |
| L2 — **수정·PIE 검증됨(2026-10-06 — `Failed to find object` NS_MuzzleFlash_01/SC_Gun_shots 0건, [B] 연결 확정) → `2026-10-06_s6_fix_batch1.md`** | 죽은 `LoadClass(AC_WeaponTank)` 제거 | `RCWSFireControlComponent.cpp:156-160` | 진입 직후 동기 플러시 2회 제거 | RCWS |
| L3 — **수정·PIE 검증됨(2026-10-06 — `Prefill` API + `BP_AR4Rifle` BeginPlay 배선, 32발 프리필이 첫 교전 전 완료) → `2026-10-06_s6_fix_batch1.md`** — 교전 중 32 유지도 검증됨(2026-10-06 → [s6_verification](2026-10-06_s6_verification.md) B1 #4) | 소총 투사체 풀 레벨 로드 시 미리 채움(RCWS는 이미 64발 프리필 `:170-178`) | `SoldierProjectilePool.cpp:51-69` | 첫 교전 중 `SpawnActor` 제거 | SoldierLab |
| L4 — **코드 수정됨(2026-10-06, 검증 대기 — 자체 풀 쓰는 ImpactFire `:753`·`:832`는 의도적 제외) → `2026-10-06_s6_fix_batch1.md`** — 평탄 확인 미완(2026-10-06 → [s6_verification](2026-10-06_s6_verification.md) B1-2) | Niagara 풀링 `AutoRelease` + 에셋 `PoolPrimeSize`(첫 월드 틱에 프라임, 엔진 `NiagaraWorldManager.cpp:1722-1727`) — NS_MuzzleFlash·NS_Blood는 이미 PoolPrimeSize가 있는데 코드가 풀을 안 써서 낭비 중 | `RCWSFireControlComponent.cpp:1657`, `RCWSProjectile.cpp:753,981`, `SoldierProjectile.cpp:832,1077` | 피격마다 컴포넌트 생성 제거(§2-2 확장) | SoldierLab · RCWS |
| L5 — **구현됨(2026-10-06, 검증 대기) → `ui/2026-10-06_loading_screen_impl.md`** C단계(`UTitanLoadingScreenSubsystem`, 목록은 `DefaultGame.ini` `TitanLoadingScreenSettings`) — 2026-10-06 -game: 첫 프레임이 10 s 넘게 걸린 회차에서 워밍업 생략 발견, 히치 비교는 PIE 1샘플씩으로 미확정 → [s6_verification](2026-10-06_s6_verification.md) F1·L9 | Niagara·데칼 워밍업: 로딩 화면 아래에서 각 시스템/데칼 재질 1회 실제 그리기(PIE는 그려야 PSO 컴파일), 에디터는 `WaitForCompilationComplete` | 신규 워밍업(§8-3 Phase C) | 첫 발사/첫 피격 히치 | titan |
| L6 | 프로젝트 설정 Niagara `DefaultPSOPrecacheSettings.bCacheOnLoad=true`(엔진 `NiagaraSettings.h:301`) | ini | 쿡 빌드 첫 이펙트 미표시 방지 | 콘텐츠 |
| L7 — **수정됨(2026-10-06, 검증 대기) → `2026-10-06_s6_fix_batch3.md`** A6 — 첫 틱 히치 확인은 2026-10-06 검증 회차에서 미실시([s6_verification](2026-10-06_s6_verification.md) B3-4) | AmbientFX 소프트 참조를 BeginPlay에 로드 | `Environment/AmbientFXDirector.cpp:106` | 진입 직후 첫 틱 스톨 | titan |
| L8 | MetaSound 프리캐시 `UMetaSoundCacheSubsystem::PrecacheMetaSound`(`MetasoundOperatorCacheSubsystem.h:38`) | 총성·피격·휘바람 MetaSound | 첫 재생 지연 | SoldierLab |
| L9 | 무기 사운드웨이브 `LoadingBehavior=PrimeOnLoad`(기본 LoadOnDemand, `SoundWave.cpp:58-59`) | 에셋 | 첫 재생 청크 로드 | 콘텐츠(오디오) |
| L10 | (선택) 번들 PSO 캐시: 데모 1회 `-logPSO` 기록 → `.spc` 동봉 + `r.ShaderPipelineCache.Enabled=1`; 리눅스 Vulkan은 PC마다 배포 후 워밍업 1회 | 패키징 | 프리캐싱이 놓치는 전역/포스트 PSO | 패키징 |

제외(결과가 바뀜): SituationField 호라이즌 사전 베이크(첫 몇 초 AI 판단이 달라짐 — `SoldierSituationField.cpp:2073-2092`), Niagara DelayMode/ProxyCreationStrategy 변경(PSO 전 그리기 → 히치/대체 모습).

### 8-3. 로딩 화면 설계 (로비 → New_kadex_0811)

> **검증(2026-10-06)**: 게이트 ①②③·재시작 3회 통과, RTSP 메인뷰 검정 유지는 의문(오버레이 중 풀리는 구간), 2-PC ④ 미실시 → [s6_verification](2026-10-06_s6_verification.md) L1~L10.
>
> **최종 설계는 `ui/2026-10-06_loading_screen_design.md`로 분리·확정**(2026-10-06). 아래는 초안 기록 — 차이점: 준비 상태를 월드 키잉으로(재시작 공유 함수에 넣으면 재시작 후 영구 미시작), 카메라 페이드를 로딩 내내 유지(RTSP 메인뷰도 가려짐 [A]), 명령어 거부 정책, 클라 신호 없음.

현재 [A]: **하드 트래블**(시임리스·TransitionMap 없음). 호스트 `HostListenServer` → `open <map>?Listen?Axis=UGV?Demo=`(`titan_examplePlayerController.cpp:2433`), 솔로 `:2462`, 클라는 IP로 `open`(`:2478`) — 클라가 호스트와 **다른 시점에 따로** LoadMap. `UEngine::LoadMap`은 액터 BeginPlay(`UnrealEngine.cpp:16661`)가 `PostLoadMapWithWorld`(`:16665`)보다 먼저. 데모 자동 시작은 BeginPlay에서 3 s 타이머(`ScenarioStateSubsystem.cpp:650`, `ScenarioConfig.h:126`). 클라 준비 핸드셰이크 없음.

설계(`UGameInstanceSubsystem` 1개):
- **A. 로비 페이드아웃**: `open` 직전 `StartCameraFade` → 타이머 T 후 `ConsoleCommand`(즉시 open하면 게임 스레드가 막혀 페이드가 안 보임). 접속 실패 시 `OnNetworkFailure/OnTravelFailure`로 상태 해제.
- **B. LoadMap 동안 MoviePlayer Slate 화면**: `PreLoadMap`에서 `GetMoviePlayer()->SetupLoadingScreen`. **에디터(PIE)에선 비활성**(`IsMoviePlayerEnabled` = `!GIsEditor && …`, 엔진 `Runtime/MoviePlayer/Private/MoviePlayer.cpp:125`), Standalone/패키지 Windows·Linux 동작. Build.cs에 `MoviePlayer` 추가. 워밍업 단계까지 끌지 말 것(월드 뷰가 안 그려져 PSO/스트리밍이 실제 뷰로 안 데워짐).
- **C. 워밍업 오버레이(“몇 초 불안정”을 실제로 없애는 단계)**: `OnWorldBeginPlay`에서 `GameViewport->AddViewportWidgetContent`(PIE 포함 동작), 아래에서 월드는 계속 렌더. L5 워밍업 실행. 준비 조건(N프레임 연속 + 하드 타임아웃 20~30 s): `!IsAsyncLoading()`, `IStreamingManager::Get().GetNumWantingResources()==0`(엔진 `ContentStreaming.h:321`), `PipelineStateCache::NumActivePrecacheRequests()==0`(`RHI/Public/PipelineStateCache.h:286`, 동안 `PrecachePSOsBoostToHighestPriority(true)`), 프레임 시간 안정.
- **D. 페이드인 → 시나리오 시작**: 기존 `FadeLocalCameras`(`ScenarioStateSubsystem.cpp:1190-1203`) 재사용. 데모 자동 시작 타이머를 BeginPlay가 아니라 준비 완료 시점에 무장 → 시나리오 내용은 그대로.
- **2-PC (2026-10-06 사용자 결정)**: 클라 로딩은 **자기 화면 전용**, 호스트에 RPC·신호 일절 없음(핸드셰이크 만들지 않음). 클라는 지금처럼 접속해서 복제로 합류. **호스트 시나리오는 호스트 로딩 완료 전엔 절대 시작 불가.**

#### 호스트 시나리오 게이트 — 막아야 할 진입점 전수 [A]

모든 스텝 시작은 `UScenarioStateSubsystem::BeginScenarioSteps()`(`ScenarioStateSubsystem.cpp:1489`)로 모이고, 호출자는 둘뿐: `BeginEnemyContactScenario`(`:1241`)와 `StartUGVAdvance`(`:1486`). 이 둘로 들어오는 경로:

| 경로 | 위치 |
|---|---|
| 데모 자동 시작 타이머(BeginPlay 후 `DemoAutoStartDelaySeconds`) | `ApplyDemoRunModeSetup` `:648-653` → `DemoAutoStartScenario` `:716-727` |
| 데모 RCWS 자동사격 타이머(BeginPlay 후 1 s) | `:645` → `ApplyDemoRCWSAutoFire` |
| 콘솔/HUD `BeginEnemyContact` 류 Exec → Server RPC | `titan_examplePlayerController.cpp:2005-2027` |
| `BeginAllyFormUpAndAdvance` Exec → 서버 → 지연 타이머 `StartUGVAdvance` | `PlayerController.cpp:1775-1783`, `ScenarioStateSubsystem.cpp:1422-1451` |
| 재시작(로딩 중 발동 시 첫 시작과 같음): 확인창/카운트다운/`titan.ScenarioRestart`/ScenarioMonitor fail-safe | `ScenarioStateSubsystem.cpp:138, 799-807, 825`, `ScenarioMonitorSubsystem.cpp:713`, `PlayerController.cpp:2030-2060` |
| 풀시스템 모드 LIG 명령 | `Network/UGVRemoteControlSubsystem` — 시작 명령이 위 함수들을 부르는지 구현 시 재확인 [B] |

설계:
1. 게이트는 PC/RPC가 아니라 **`UScenarioStateSubsystem` 안의 진입 함수**(`BeginEnemyContactScenario`, `StartUGVAdvance`, `BeginScenarioSteps`, `RequestScenarioRestart`, `ApplyDemoRCWSAutoFire`)에 둔다 — 모든 경로가 여기로 모이므로 한 곳만 지키면 빠짐이 없다. 미준비 시 수동 명령은 **거부 + 경고 로그**, 데모 자동 시작은 거부가 아니라 **준비 시점으로 연기**.
2. 데모 타이머 2개는 BeginPlay가 아니라 **준비 완료 콜백에서 무장**(지연 3 s가 페이드인부터 셈). 준비 신호와 `RegisterScenarioConfig`(BeginPlay)는 순서가 뒤바뀌어도 되게 양쪽에서 확인.
3. 준비 플래그는 **월드 단위**(GameInstance 서브시스템이 트래블을 넘어 살아남으므로 `ResetForNewWorldIfNeeded`에서 반드시 false로). 재시작은 레벨 리로드가 아니라 로딩을 다시 안 거치므로, 한 번 준비된 월드는 계속 준비 상태.
4. 준비 신호는 **NM_Client가 아닌 프로세스(호스트/솔로)의 로딩 서브시스템만** 낸다. 클라 로딩 서브시스템은 자기 오버레이만 내린다(클라 `ApplyDemoRunModeSetup`은 이미 `NM_Client`면 리턴, `:621-624`).
5. "준비 전 시작 금지"는 구조로 보장되고, 타임아웃은 **로딩 종료 조건**이다(타임아웃 = 로딩 끝 → 그다음에 시나리오). 12 h 무인 운용에서 영구 대기를 막기 위해 타임아웃 필수, 사유를 로그로 남김.
6. 검증: 로딩 최소 유지 시간을 일부러 길게(예: 30 s) 둔 상태에서 ① 스텝 로그가 페이드인 전에 안 찍히는지 ② 로딩 중 콘솔 BeginEnemyContact·`titan.ScenarioRestart`가 거부되는지 ③ PIE 직접 시작(로비 없이)에서도 게이트가 풀리는지 ④ 클라 늦은 접속이 호스트 시나리오에 영향 없는지.

확인 필요 [B]: SoldierLab 분대가 시나리오 스텝과 무관하게 BeginPlay부터 스스로 움직이는지(오버레이 아래서 교전이 시작되면 안 됨) — 구현 세션이 확인.

위험: UMG 오버레이는 **RTSP 메인뷰 송출엔 안 가려짐**(포스트톤맵 복사) — 카메라 페이드는 씬에 적용돼 가려질 가능성 [B]; 자체방호 Monitor1은 별도 SWindow라 따로 처리; 스트리밍 대기는 숲 씬에서 0이 안 될 수 있어 타임아웃 필수; 데모 자동사격 타이머(`:645`)도 게이트 고려.

---

## 부록 A — P3 표

| file:line | 내용 | 담당 |
|---|---|---|
| `Vehicles/LatencyClockComponent.cpp:35` | BP_UGV_0901에서 매 프레임 `SetText`(FDateTime 문자열) → `MarkRenderStateDirty` 무조건(엔진 `TextRenderComponent.cpp:1441`) | titan UGV |
| `RCWSFireControlComponent.cpp:1283-1289,462-466,504,610` — **수정됨(2026-10-06, 검증 대기) → `2026-10-06_s6_fix_batch3.md`** A5(4) 표적 컴포넌트 캐시(본/소켓은 매 호출) | `UpdateAimPointForUI` 매 틱 전 프로세스, 대상마다 컴포넌트 조회 2회+소켓 | RCWS |
| `DetectableTargetComponent.cpp:207` — **수정됨(2026-10-06, 검증 대기) → `2026-10-06_s6_fix_batch3.md`** A5(1) 클래스별 캐시(miss 포함) | `FindFProperty(FName("IsDead"))` 0.3 s마다 대상마다, SoldierLab 병사엔 없어 영구 실패 | titan 탐지 |
| `UGVAIController.cpp:975,1047,1063,2329,2368` — **수정됨(2026-10-06, 검증 대기) → `2026-10-06_s6_fix_batch3.md`** A5(2)(0811은 RoadNavMod 볼륨 0개라 253 복사는 옛 레벨 얘기) | 매 틱 `FindFunction(FName(TEXT))`×2, `FindPropertyByName`×2, `FindComponentByClass`, 기어 FString, 무시 목록(253) 복사 | titan UGV |
| `UGVAvoidanceProxyComponent.cpp:33-36` — **수정됨(2026-10-06, 검증 대기) → `2026-10-06_s6_fix_batch3.md`** A5(3)(정적 FName + EndPlay `RemoveAvoidanceObject`) | `GetName()==TEXT(...)` FString 할당, `RegisterMovementComponent` 해제 없음 | titan UGV |
| `RCWSComponent.cpp:313-343` · `TitanTruck.cpp:347-363` · `UI/UGVStatusComponent.cpp:14,24-26` | 렌즈 동기화 매 틱(`FPostProcessSettings` 복사 2회), 더미 전압/온도 사인값이 복제 구조체를 매 넷업데이트 dirty | RCWS / UI |
| `RCWSFireControlComponent.cpp:156-160` — **수정·PIE 검증됨(2026-10-06) → `2026-10-06_s6_fix_batch1.md`** (§8-2 L2와 같은 항목) | BeginPlay마다 미사용 `AC_WeaponTank`(1.5 MB BP) `LoadClass` 동기 로드 | RCWS |
| `RCWSFireControlComponent.cpp:163-179` — **수정됨(2026-10-06, 검증 대기) → `2026-10-06_s6_fix_batch3.md`** A7(Destroyed/RemovedFromWorld에서 풀 파괴) | 풀 64발(Owner=차량) EndPlay 파괴 없음 — 현재는 차량 제자리 부활이라 미발동 | RCWS |
| 재시작 미복원 | `BarrelHeatValue`, 정적 `RCWSProjectile_ActiveImpactFires`(24개 bAutoDestroy=false 재사용) | 시나리오 |
| `DronePawn.h:736-740`, `.cpp:834,985,2547` | `bShowFlightDebug`/`bDrawRotorThrustVectors` C++ 기본 true(BP 오버라이드 값 미확인 [B]) | 드론 |
| `TargetDetectionComponent.cpp:141,245` | 카메라 미해결 시 매 틱 재시도, 배열 복사(MoveTemp 아님) | 드론/RCWS |
| `Plugins/QuadCamModule/.../QuadCamComponent.cpp:259,95-126` | `SyncLensFromCineCameras` 매 틱 4카메라(캡쳐 안 하는 틱 포함) | QuadCam |
| `QuadCamComponent.cpp:210-223` | 뷰포트 위젯 EndPlay 제거 없음 — 차량 영속이라 미발동 | QuadCam |
| `RtspServerSubsystem.cpp:492-497` | push 실패마다 Log 레벨 로그(해제 구간 ≈30줄/s/스트림) | S1 |
| `RtspServerSubsystem.cpp:300-342` · `RtspEncoderModule.cpp:124` | Deinitialize에서 클라 세션·소스·스레드풀 정리 없이 `gst_deinit` — 종료/PIE만 | S1 |
| `RtspStreamComponent.cpp:134` · `MainViewStreamComponent.cpp:372` | EndPlay마다 `FlushRenderingCommands`(레벨 종료만) | S1 |
| `FNvencVulkanEncoder.cpp:390-396` | `vkDestroyImage`이 RHI 지연 삭제보다 먼저 | S1 |
| `Plugins/Chronicle/.../ChronicleSubsystem.cpp:12-16` · `:368-376` | `demo.RecordHz` 복원 안 함, 재생 중 매 프레임 `TActorIterator`(에디터 전용) | Chronicle |
| `UI/ScenarioMonitorSubsystem.cpp:270-281` | 세션 로그 폴더 보존 정책 없음(현재 156개, 7.8 MB) | 시나리오 |
| `UI/ScenarioRespawnSubsystem.cpp:298-310,353-372,256-265` | 재시작 시 병사마다 전 월드 순회(≈60k 반복, 1~3 ms, 페이드 뒤) | 시나리오 |
| `UI/ScrollingRulerWidget.cpp:190,226,260,284,288` · `LineGraphWidget.cpp:168-227` | OnPaint마다 배열 할당·Printf·큐빅 재구성 | UI |
| `Soldiers/SoldierLabBridgeSubsystem.cpp:106-111,113-129,169-184` — **`GetActorOfClass` 부분만 수정됨(2026-10-06, 검증 대기) → `2026-10-06_s6_fix_batch3.md`** A5(5) · O(N·M) `ContainsByPredicate`는 미변경 | 0.25 s마다 `GetActorOfClass(AScenarioConfig)`, O(N·M) `ContainsByPredicate` | titan 시나리오 |
| `Squad/SoldierZone.cpp:26-27,129-137` · `Squad/SoldierSquadSubsystem.cpp:562-563` | 디버그 전용 상시 틱, 매 프레임 `GetMembers()` 힙 배열 | SoldierLab |

## 부록 B — 확인했고 깨끗한 것 (요약)

- `TStrongObjectPtr`/`AddToRoot`/정적 UObject: 전 범위에서 위험한 것 없음(RCWS 정적 `USoundConcurrency` AddToRoot는 월드 참조 없음).
- 타이머·람다: 약참조 또는 `CreateWeakLambda`(지면 혈흔 `SoldierProjectile.cpp:671-697`, `SoldierHealth.cpp:517`).
- 사망 무기 드롭: `DroppedWeapons` 추적 후 `SoldierHealth` EndPlay 파괴, [W116] 적용 확인.
- 재시작: `ScenarioRespawnSubsystem`이 병사 전원 파괴, 풀 투사체는 `RecallAll`로 주차(파괴 제외 `ScenarioRespawnSubsystem.cpp:283-287`), 타이머 ClearTimer(`ScenarioStateSubsystem.cpp:1692-1721`), UGV nav modifier 파괴(`:1345-1359`), 토스트/확인창 RemoveFromParent.
- 네트워크: 발사/임팩트/도탄 멀티캐스트 전부 Unreliable, 틱당 reliable RPC 없음, Engagement Mirror 변경 게이트+양자화. `DetectableTarget`의 `bAlwaysRelevant`는 드론 관련성 수정의 의도된 설정.
- LIG UDP: 논블로킹 소켓, GT의 FTSTicker 50 Hz 폴링, Demo 모드에선 완전 비활성(`Network/UGVRemoteControlSubsystem.cpp:185-190,283-284`).
- DrawDebug: 범위 내 전부 cvar 게이트 또는 `#if 0`.
- NVENC D3D12 렌더 람다: `this`가 아니라 ThreadSafe `TSharedPtr` 캡처, EndPlay 플러시로 보호. `ResizeTarget`은 리소스 객체 유지(엔진 `TextureRenderTarget2D.cpp:188-199`).
- GStreamer 루프 FRunnable은 Deinitialize에서 Stop/Join, `AppSrcLock`/`StreamsLock`로 스레드 간 보호.
