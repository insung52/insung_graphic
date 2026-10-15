# RCWS 거리측정 트레이스가 SoldierLab 병사를 관통 — TraceSightLine 도입

2026-10-15 / 완료(2026-10-15 사용자 빌드·실측 확인 — RNG 거리) / RNG 거리·무락온 조준점이 병사 뒤 배경 기준으로 나오던 버그를 Visibility+Pawn 몸 2단 트레이스(`URCWSComponent::TraceSightLine`)로 수정.

관련 코드: `Source/titan_example/Vehicles/RCWSComponent.h/.cpp`, `Vehicles/RCWSFireControlComponent.cpp`
레퍼런스: `guide/rcws_fire_control_dev_guide.md` §6.1

## 증상 (사용자 리포트)

UGV 대시보드 `RNG 90 m` 거리 표시가 적을 조준하고 있어도 **적까지가 아니라 그 뒤 배경까지** 거리로 나옴.

## 원인

- `URCWSComponent::UpdateRangeTrace`(`RCWSComponent.cpp`)는 조준경 카메라 정면으로 `ECC_Visibility`
  라인트레이스를 쏘고 첫 히트 거리를 `RangeMeters`로 쓴다.
- New_kadex_0811의 적은 SoldierLab 병사 `BP_Soldier_Hostile`(아군 `BP_Soldier_Friendly`도 동일).
  MCP로 CDO 확인 결과 두 컴포넌트 모두 **Visibility = Ignore**:
  - `CollisionCylinder`(캡슐, Custom) — Visibility / Pawn / Camera / PhysicsBody Ignore
  - `CharacterMesh0`(Custom) — Pawn / Visibility / Vehicle / Camera Ignore
  - 엔진 기본 `Pawn`/`CharacterMesh` 프로필(`BaseEngine.ini:3110`/`:3112`)과 같은 응답.
- 그래서 Visibility 트레이스가 병사를 그대로 관통해 뒤 지형/배경에 맞았다.

### 이력 — 같은 버그의 재발

2026-08-11에 구 적군에서 똑같은 리포트("거리탐지 레이트레이싱에서 적군 제외, 뒤 배경 조준")가 있었고,
`Soldiers/EnemyCombatComponent.cpp`(~203-209)에서 캡슐을 `ECC_Visibility = ECR_Block`으로 바꿔 고쳤다
(별도 문서 없이 코드 주석으로만 남아 있음). 적이 SoldierLab 병사로 교체되면서 이 처리가 없어졌다 —
New_kadex_0811에서 `EnemyCombatComponent` 기제는 죽은 코드.

### 같은 원인의 동반 버그

- `URCWSFireControlComponent`의 **무락온 탄도 조준점**(`RCWSFireControlComponent.cpp` ~981-988,
  Remote 조작/탐색 중)도 같은 Visibility 트레이스 → 병사를 겨눠도 뒤 배경 기준 앙각으로 슬루.
  락온 중엔 대상 위치를 직접 쓰므로 무관.
- 트럭 RCWS도 같은 `URCWSComponent`라 동일 버그였음.

## 기각안

**병사 콜리전을 Visibility Block으로 바꾸기** — SoldierLab이 Visibility 트레이스를 엄폐 판정·탄 피격 재질
판정 등에 쓰므로 AI 동작이 바뀔 위험. 병사 쪽 설정은 손대지 않는다.

## 수정

신규 static `URCWSComponent::TraceSightLine(World, Start, End, IgnoreActor, OutHit)` (`RCWSComponent.h:305`, `.cpp:382`):

1. 기존과 같은 `ECC_Visibility` 단일 트레이스.
2. Start → (Visibility 히트 지점, 없으면 End)까지 `LineTraceMultiByObjectType(ECC_Pawn)` 후 필터:
   - 컴포넌트가 히트 액터의 **루트 컴포넌트(캐릭터 캡슐)** 이거나 **`USkeletalMeshComponent`** 일 때만 채택.
     오브젝트 쿼리는 채널 응답과 무관하게 Pawn 타입이면 다 맞으므로, 아군 `EnemyDetectRange` 같은 큰 감지
     스피어를 거르기 위함.
   - `"NoBounds"` 태그 컴포넌트 제외.
   - `IgnoreActor`(자기 차량)와 그 부착 액터 제외.
3. Pawn 몸 히트가 있으면 그것(구간이 Visibility 히트까지라 반드시 그보다 앞), 없으면 Visibility 히트.
   잎 등 Visibility를 막는 게 병사보다 앞이면 그게 이김 — 실제 레이저 측정기와 같은 동작.

교체한 호출부:
- `URCWSComponent::UpdateRangeTrace` (`RCWSComponent.cpp:463`) — RNG 거리.
- `URCWSFireControlComponent` 무락온 조준점 (`RCWSFireControlComponent.cpp:984-987`) — `MuzzleAimTargetWorldLocation`.

영향 없음: 투사체(`SoldierProjectile`/`RCWSProjectile`)는 WorldDynamic이라 이 쿼리와 무관.

### 변경 파일 (p4 opened, 미제출)

- `Vehicles/RCWSComponent.h` / `.cpp`
- `Vehicles/RCWSFireControlComponent.cpp`

UPROPERTY 변경 없음(Live Coding 가능하나, 헤더에 static 함수 추가라 정식 빌드 권장).

## 검증 항목 (사용자)

> **2026-10-15 사용자 실측 확인** — 빌드 후 RNG 거리 "잘 나와". 사용자가 RNG 표시만 언급했으므로
> 첫 항목만 확인으로 닫고, 나머지(트럭 개별 확인·숲 뒤 병사·무락온 조준 앙각)는 미확인으로 둔다.

- [x] RNG — 병사를 조준하면 그 병사까지 거리(2026-10-15 사용자 확인. UGV/트럭 중 어느 쪽인지, 적·아군 둘 다인지는 언급 없음).
- [ ] 병사 없는 곳은 지형 거리(기존과 동일).
- [ ] 숲 뒤 병사는 잎까지 거리.
- [ ] 원격 조작 중 병사를 겨눴을 때 조준 앙각이 그 거리 기준(탄착이 병사 근처).
