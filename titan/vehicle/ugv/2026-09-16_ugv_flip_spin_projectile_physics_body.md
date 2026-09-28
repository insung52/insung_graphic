# UGV_0901 교전 중 "팡 뒤집혀 빙글빙글" 버그 — 투사체 물리 바디가 원인

2026-09-16 / 완료(빌드·PIE 검증 대기) / `ARCWSProjectile` 콜리전을 `QueryAndPhysics`→`QueryOnly`로. 서스펜션 무관, 자기 탄의 키네마틱 물리 바디가 물리 스레드에서 차체 `Hull1`을 치는 게 원인.

---

## 1. 증상 / 출발 가설

`BP_UGV_0901`이 적과 교전 중 갑자기 튕겨 뒤집히고 빙글빙글 돈다. 정확한 재현 조건 미상, "교전
중"이 유일한 힌트. 사용자 가설은 "Chaos 차량이니 적 총알이 서스펜션과 이상 반응". 결론부터:
**총알은 맞는데 경로가 서스펜션이 아니다.**

## 2. 기각한 후보 (엔진 소스 기준)

| 후보 | 판정 | 근거 |
|---|---|---|
| 총알 ↔ 서스펜션 | ✗ | 휠 스피어캐스트는 채널 `ECC_WorldDynamic`(`ChaosWheeledVehicleMovementComponent.cpp:333`), 응답은 기본 컨테이너에서 Vehicle만 Ignore(`:1142`). 투사체는 WorldDynamic 채널을 **Ignore**(`RCWSProjectile.cpp:95`)라 트레이스에 애초에 안 잡힘 |
| 서스펜션 하드스톱 | △ 2순위 | 하드스톱은 살아 있음(`PBDSuspensionConstraints.cpp:315~` 강성 1 하드 접촉 매니폴드; `ApplySingle`의 위치식 하드스톱은 주석). 적 캡슐/래그돌이 바퀴 구체 시작 영역(지면 47~110cm)에 겹치면 타깃이 튀지만 법선이 수평(캡슐)이거나 질량비로 흡수(래그돌) → 덜컹임 수준 |
| 데미지 임펄스 | ✗ | 엔진 기본 `bApplyImpulseOnDamage=true`(`PrimitiveComponent.cpp:416`) + `UDamageType::DamageImpulse=800` → 피격마다 800 kg·cm/s. 1500kg에 Δv 0.5cm/s. 포탑/바퀴 바디는 Kinematic이라 `IsSimulatingPhysics(Bone)` false → 아예 안 붙음 |
| 반동 임펄스 | ✗ | `RecoilImpulseMagnitude=1000`(BP 값) → 발당 0.7cm/s |
| 적 캡슐 = 이동 불가 기둥 | ✗ | 차량이 적을 들이받는 경우만, 스핀과 무관 |
| `UGVAIController` 속도 편집/텔레포트 | ✗ | 오프로드 감쇠는 전진 성분만 깎음, 텔레포트는 속도 0 리셋 |

## 3. ★ 원인 — 투사체의 물리 스레드 키네마틱 바디

세 층이 겹친다.

1. **투사체 구체가 물리 스레드에 존재했다.** `CollisionComponent`가 `QueryAndPhysics` + 비시뮬
   = **키네마틱 강체**. `ProjectileMovement`가 매 프레임 ~15m를 `ETeleportType::None`으로
   옮기므로 PT에서는 키네마틱 타깃으로 **V≈850m/s, 무한질량**인 물체다.
   (`BP_RCWSProjectile`, `BP_RifleProjectile` 둘 다 이 C++ 부모, MCP로 CDO 확인.)
2. **`MoveIgnoreActors`는 게임 스레드 전용이다.** `LaunchFrom`(`RCWSProjectile.cpp:186`)이
   발사 차량을 ignore하지만 이건 스윕 쿼리에만 적용. PT 충돌은 필터 데이터(오브젝트
   타입/채널)로만 결정되고, 투사체(WorldDynamic, Vehicle 채널 Block) ↔ `Hull1`(Vehicle
   프로파일, WorldDynamic Block)은 **PT에서 충돌하는 쌍**이다.
   - 적/트럭 탄: GT 스윕이 UGV를 맞춰 `OnHit`→`Deactivate()`(`:1037`, 콜리전 즉시 off)가
     물리 스텝 전(TG_PrePhysics)에 끝남 → 안전.
   - **UGV 자기 탄, 그리고 자기 탄의 도탄**(`ReportRicochetToInstigator`가 발사자 상속):
     GT는 차체를 무시하고 통과시키는데, 탄의 프레임 끝 위치가 `Hull1` 안/근처에 떨어지면
     PT가 접촉을 만들어 차체를 밀어낸다. `MaxDepenetrationVelocity=0`(무제한),
     `MaxAngularVelocity=3600°/s`(10회전/초) → 한 스텝에 튕기고 스핀.
3. **왜 "가끔 갑자기"인가 — CCD가 없다.** `VehicleMesh` 컴포넌트의 `bUseCCD=true`는 스켈레탈
   바디로 **복사되지 않는다**(`SkeletalMeshComponentPhysics.cpp:1029~1060` — 복사되는 건
   iteration count/DOF/COMNudge뿐). `Hull1` 바디 셋업은 `bUseCCD=false`. 그래서 이산 충돌만
   일어나고, 15m 스텝 중 하나가 차체 두께(1~2.8m) 안에 떨어질 때만(스텝당 10~20%) 터진다.
   도탄은 느려서 스텝이 짧아 확률이 더 높다.
   (CCD가 켜져 있었다면 `CCDUtilities.cpp:448~`의 fast-moving-kinematic 처리가 매번 잡아
   `ClipParticleP(차체, V·(1−TOI)·Dt)`로 100% 터졌을 것.)

### 어떤 조준이 자기 차체를 지나는가

`RCWSMount` (50, 0, 105), `Muzzle`은 마운트 +123cm(z +6). `Hull1` 컨벡스 x −137~+147,
z 최대 ~82, 후방 코너에 z 137까지 솟은 기둥(x −111~−73, y ±51~62). 앙각 −20°~+60°.

- 전방/측방: 총구가 차체 밖 → 안전.
- **후방(180°) −20°: 총구 z≈69 < 지붕 z≈70 → 총구가 차체 안.** 후방 −10°도 1m 안에 지붕
  관통. 후방 ±25°는 기둥 관통.
- 근거리 지면/바위 도탄이 차체로 되돌아오는 경우.

즉 "적이 UGV 뒤/옆으로 붙어서 포탑이 뒤로 돌아 내려쏠 때"가 유력한 재현 조건.

## 4. 수정

`Source/titan_example/Vehicles/RCWSProjectile.cpp` 생성자:

```cpp
CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);   // was QueryAndPhysics
```

BP 두 개(`BP_RCWSProjectile`, `BP_RifleProjectile`)는 `.uasset`에 `CollisionEnabled` 문자열이
없음 = 오버라이드 없음 → C++ 기본값이 그대로 흐른다. 빌드 후 MCP `get_properties`로
`CollisionComponent.BodyInstance.collisionEnabled == QueryOnly` 재확인 권장.

### 기존 기능이 안 깨지는 이유

- 스윕은 `IsQueryCollisionEnabled()`만 본다(`PrimitiveComponent.cpp:3325`).
- 블로킹 히트 → `DispatchBlockingHit` → `OnComponentHit.Broadcast`에 물리 콜리전 조건 없음
  (`:3541~3550`, `Actor.cpp:3525 InternalDispatchBlockingHit`). `bNotifyRigidBodyCollision`은
  시뮬레이션 접촉 콜백 전용.
- `OnHit`는 `Hit` 결과만 쓴다. `NormalImpulse`는 파라미터로만 존재, `Hit.PhysicsObject` 미사용.
- UGV의 포탑/바퀴 바디는 원래부터 QueryOnly인데 지금도 잘 맞고 있다 = QueryOnly 대상도
  스윕에 잡힌다는 실증.
- 따라서 적→UGV 피격효과, 트럭 RCWS→UGV 피격효과, 전 발사→적 피격/데미지, 도탄, 휘즈
  사운드 전부 동일. 없어지는 건 아무도 안 쓰던 PT 키네마틱 바디뿐.

## 5. 검증 체크리스트 (빌드 후 PIE)

1. RCWS를 후방으로 돌려 −20°로 연사 → 뒤집힘 재현 안 됨.
2. 적 사격 → UGV 피격 이펙트/사운드.
3. 트럭 RCWS → UGV 피격 이펙트.
4. UGV/트럭/아군 사격 → 적 피격 이펙트 + 데미지(사망).
5. 지면/바위 도탄 정상.

## 6. 함정 요약

- **`MoveIgnoreActors` / `IgnoreActorWhenMoving`은 GT 스윕 전용.** 물리 스레드 충돌은 채널/
  오브젝트 타입 필터로만 걸러진다. "자기 자신 무시" 목적으로 이걸 믿고 QueryAndPhysics
  바디를 초고속으로 움직이면 PT에서 자기 차체를 친다.
- **컴포넌트 `BodyInstance.bUseCCD`는 스켈레탈 메시 바디에 전파되지 않는다.** 스켈레탈
  차량에 CCD를 주려면 피직스 에셋 바디 셋업에서 켜거나 `SetAllUseCCD()`.
- 투사체·트레이서 같은 "쿼리만 필요한 초고속 물체"는 QueryOnly가 기본이어야 한다.
- `SoldierLab/Weapons/SoldierProjectile.cpp`(2026-09-12 포팅본)은 아직 `QueryAndPhysics`(`:91`).
  실험 모듈이라 이번엔 안 건드렸다 — 그쪽에서 같은 증상이 나면 동일 수정.

## 관련 문서

- `guide/rcws_fire_control_dev_guide.md` §7.1 — 콜리전 구성(이번에 갱신).
- `2026-09-02_ugv_0901_new_model_rig.md` §2 — 피직스 에셋 바디 구성(Hull1 시뮬 + 나머지 Kinematic/QueryOnly).
- `2026-09-10_ugv_0901_suspension_tuning.md` — 서스펜션 구조(하드스톱 주석 부분은 §3 참고: `GatherInput` 쪽 매니폴드 하드스톱은 살아 있음).
