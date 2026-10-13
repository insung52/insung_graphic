# UGV_0901 장시간 실행 중 전복 — 병사 캡슐 물리 접촉 차단 + 자가 복원

2026-10-12 / 완료(빌드·PIE 검증 완료, 장시간 2-PC 재실행 검증 대기) / 정차 중 적 병사가 붙어 있다가 뒤집힌 사건. 병사 캡슐(무한질량 키네마틱)이 유력 후보 → UGV는 Pawn/PhysicsBody와 물리 접촉 안 함 + 전용 차단 박스로 병사를 밀어냄 + 전복 시 자가 복원(토크→순간이동) + health.csv에 자세 컬럼 추가.

---

## 1. 계기 / 로그 분석 (2026-10-12 장시간 2-PC 실행)

로그 위치:

- 리눅스 호스트: `D:\linuxlog_1012_1` (`HealthLogs/`, `Logs/`, `ScenarioLogs/`)
- 윈도우 클라(자체방호): `C:\working\outputs\kadex_1012_windows_3\Windows\titan_example\Saved`

14사이클 정상 완주, 15사이클이 13:12:44(현지)에 시작. 사용자가 UGV가 뒤집혀 있는 걸 발견.

### 15사이클 타임라인 (현지 시각)

| 시각 | 사건 |
|---|---|
| 13:16:44 | UGV 2구역 목적지로 출발(경로 270m) |
| 13:17:31 | 윈도우 클라 사용자가 정상 종료(SessionEnd) |
| 13:18:33 | 서버 UGV 2구역 도착·정렬 완료 — `Facing: aligned (-0.0 deg off)`, 주행 230m → **이 시점까지 서버 UGV는 정상 자세**. 같은 순간 적 2분대 생존자가 2구역 도달(`적군/2 명령 달성 ... 생존 1/5`) |
| 13:18:37, 13:18:44 | RCWS가 `BP_Soldier_Hostile_C_12`를 획득했다가 **한 발도 안 쏘고 상실** — 15사이클 전체에서 "획득 후 미사격"은 이 두 번뿐 |
| 13:18:50~13:19:14 | `근처 적 총성(7m/5m/4m)` — 적이 UGV에 약 30초 붙어 있음(전례 없음) |
| 13:19:07 | 호스트 창 포커스 잃음/복귀(사용자가 호스트 앞에 옴) |
| 13:19:20 | C_12 사망 |
| 13:19:27 | 호스트 종료 |

15사이클 2구역 주행은 109.5초(다른 사이클 55~95초).

**결론**: 서버에서 13:18:34~13:19:27 사이, **정차 상태에서 적이 차체에 붙어 있는 동안** 뒤집혔을
가능성이 가장 높다. 증명은 아님 — 그 시점 UGV 자세를 기록한 로그가 없었다(→ 3(d)에서 추가).

### 이번 사건에서 기각한 후보

| 후보 | 판정 | 근거 |
|---|---|---|
| fps/메모리 | ✗ | 15사이클 내내 호스트 60fps. 유일한 231ms 프레임(13:17:29)은 클라 접속 종료 순간이고 UGV는 그 뒤 정상 주행. 프로세스 RAM 2.0→9.2GB(누수는 다른 세션이 처리 중)지만 여유 52GB, 스왑 0 |
| UGV 자기 탄 물리 바디(09-16 버그) | ✗ | 수정이 들어가 있음(`RCWSProjectile.cpp:127` QueryOnly), 도착 후 UGV 발사 0발 |

## 2. 원인 — 엔진 소스로 확인한 메커니즘 (유력 후보, 확정 아님)

1. **병사 캡슐은 물리 스레드의 무한질량 키네마틱 바디다.** `BP_Soldier_*`의 `CollisionCylinder`
   (MCP 확인): `QueryAndPhysics`, ObjectType `Pawn`, Vehicle 채널 Block, 반지름 30 / 반높이 86.
   시뮬레이션 안 하는 QueryAndPhysics 바디는 `SetKinematicTarget`으로 옮겨지는 키네마틱
   파티클이다(엔진 `BodyInstance.cpp:2812`). PT 입장에서는 걷는 속도로 움직이는 무한질량 물체 →
   1.5t UGV가 일방적으로 밀린다(사용자 관찰: "적이 부딪히면 UGV가 공기처럼 밀린다"). 접촉점이
   무게중심보다 높으면 전복 토크. **09-16 투사체 버그와 같은 구조**(키네마틱 + 무한질량).
2. **CMC `bEnablePhysicsInteraction=false`로는 안 막힌다.** 병사 BP에 이미 false(MCP 확인,
   09-21 설정)지만 이 플래그는 CMC가 명시적으로 가하는 임펄스만 끈다 — 위 키네마틱 접촉과는 무관.
   옛 titan 병사 컴포넌트(`AllyFormationComponent.cpp:253`, `EnemyCombatComponent.cpp:188`)도
   같은 오해를 하고 있었다.
3. **바퀴 서스펜션 트레이스가 병사를 땅으로 봤다.** 09-23 수정은 `ECC_PhysicsBody`만 Ignore로
   바꿨고 `WheelTraceCollisionResponses`에는 여전히 Pawn=Block(MCP 확인) → 바퀴 옆에 선 살아 있는
   병사가 그 바퀴의 "지면"이 됨.
4. 참고: 병사 메시는 09-21부터 QueryOnly(PT 바디 없음). 스켈레탈/스킨드 메시 기본 콜리전은
   NoCollision(`SkinnedMeshComponent.cpp:476`)이라 손에 든 `WeaponMesh`도 아마 안전 —
   **WeaponMesh MCP 확인은 미완**.

## 3. 조치 (코드, 사용자 빌드·PIE 검증 완료)

### (a) 바퀴 트레이스가 Pawn 무시

`Source/titan_example/Vehicles/UGVWheeledVehicleMovementComponent.cpp` 생성자, 09-23 PhysicsBody
줄 옆에:

```cpp
WheelTraceCollisionResponses.SetResponse(ECC_Pawn, ECR_Ignore);
```

### (b) `AUGV0901Pawn` — 충돌 정책 / 병사 밀어내기 / 자가 복원

`Source/titan_example/Vehicles/UGV0901Pawn.h/.cpp`.

**`PawnBlocker` (새 기본 서브오브젝트, `UBoxComponent`)** — `VehicleMesh`에 부착.

- QueryOnly, ObjectType Vehicle, 모든 채널 Ignore + **Pawn만 Block**, `bAutoWeld=false`,
  `CanCharacterStepUpOn=No`, 게임 중 숨김, 내비 영향 없음, 셰이프 색 주황.
- 기본 extent (150,100,80) @ (5,0,50) — `SK_UGV_0901` 바운드(중심 (5,0,53), extent (141,90,84))에서
  산출. BP 뷰포트에서 조절 가능, BeginPlay에서 콜리전 설정은 다시 강제.

**`ApplyPawnCollisionPolicy()`** — BeginPlay, 모든 넷모드.

- UGV의 콜리전 켜진 프리미티브 전부(`PawnBlocker` 제외)에 `ECC_Pawn`, `ECC_PhysicsBody` = Ignore.
  스켈레탈 바디의 실효 응답은 min(바디, 컴포넌트)(`BodyInstance.cpp:4816`)라 Hull/바퀴/포탑 바디
  전부 커버.
- **PhysicsBody도 끈 이유(사용자 결정)**: 시체 래그돌과 떨어진 총(둘 다 `ECC_PhysicsBody` —
  `SoldierHealth.cpp:652`, `:767`, 코드상 이 타입의 유일한 사용처)이 UGV에 닿지 않게 — "후보 원인을
  남기지 말 것". 부작용: UGV가 시체를 그냥 통과. 레벨에 PhysicsActor 소품이 있으면 그것도 통과
  (레벨 확인 안 함).
- 시작 로그: `병사 충돌 정책 — 차단 박스 ok (...), 물리접촉 끔: [VehicleMesh/Pawn VehicleMesh/PhysicsBody], 바퀴 트레이스 Pawn/PhysicsBody 무시 ok`

**`PushOverlappingPawns()`** — 서버 전용, 매 틱.

- `OverlapMultiByObjectType(box, ECC_Pawn)` 1회(비용 = 브로드페이즈 쿼리 1번, 병사 수 비례 아님).
- 실제 관통 > 1cm인 캐릭터 캡슐마다: 박스 로컬 공간에서 **옆면 4개 방향 탈출 거리**(수평만) 계산 →
  거리순 정렬 → 각 면을 캡슐 스윕(캡슐 자신의 채널/응답, UGV와 그 병사 무시)으로 검사 → 가장 가까운
  막히지 않은 면(없으면 가장 많이 나간 면) 선택. 경사 고려(바닥 법선으로 투영).
- 이동은 CMC `SafeMoveUpdatedComponent`, 그동안 캡슐이 `PawnBlocker`만 무시. 이후
  `bForceNextFloorCheck=true`(병사는 `bAlwaysCheckFloor=false`라 필요).
- 4면 모두 막힘: 이동 안 함, 경고 로그(최대 5초에 1번).

> **리뷰 중 잡은 함정**: 첫 버전은 MTD + 엔진 `ResolvePenetration`을 썼다. `ResolvePenetrationImpl`
> (`MovementComponent.cpp`)은 겹침이 없으면 MTD 방향으로 **스윕 없이 순간이동**한다 — 관통이 깊으면
> MTD가 위를 향해서 병사가 UGV 지붕에 올라간다. 그래서 옆면 기반으로 바꿈. 이동이 스윕+수평이라
> 지형 뚫림 불가. fps 하락 시에도 물리 dt 클램프 1/30이 UGV의 스텝당 이동(25km/h에서 ~23cm)을
> 제한하므로 박스를 뚫고 지나갈 일 없음.

**넷모드**: UGV 호스트 단독 / 호스트 + 자체방호 클라 / 자체방호 단독(NM_Standalone) 모두 —
충돌 정책은 서버·클라 양쪽 적용, 밀어내기는 권한(서버)만. 밀린 위치는 일반 캐릭터 이동 복제로
전파. 클라의 시뮬레이티드 병사 프록시는 스윕을 안 하므로 박스는 클라에서 효과 없음.

**`UpdateSelfRighting()` (안전장치)** — 서버 전용.

- 트리거: 기울기 ≥60°가 2초 지속 + 거의 정지(≤150cm/s, ≤60°/s; 5초 지나면 조건 무시하고 시작).
- 토크 단계: 축 `Cross(BodyUp, WorldUp)`(거의 뒤집혔으면 전방축, 부호 래치) 각속도 제어. 목표 각속도
  min(90°/s, 1.5×기울기), 가속 상한 2500°/s²(`bAccelChange` — 질량/관성 무관), 교차축 감쇠,
  400cm/s² 상승. ≤20°이면 완료, **또는 바퀴 6개 중 4개 이상 접지 + 기울기 <60°이면 완료**(경사면 대응 —
  아래 주의).
- **경사면 함정(사후 수정, PIE 미검증)**: 처음엔 완료 조건이 "월드 수직 기준 ≤20°"뿐이라, 25° 경사에 바퀴로
  제대로 내려앉아도 완료가 안 되고 토크가 오르막 쪽 바퀴를 계속 들어 올리다 6초 뒤 불필요하게 순간이동했다
  (평지 PIE에선 안 드러남). 접지 판정(`GetWheelState(i).bInContact`)을 추가했고, 대기 단계에서도 바퀴가
  대부분 접지면 "급경사 주행 중"으로 보고 취소한다. 주행 중에는 원래도 트리거(≥60° 2초 + 거의 정지)를 못
  넘으므로 토크가 들어가지 않는다.
- 6초 타임아웃 → `TeleportUpright()`: 지면 트레이스(WorldStatic, 나무 프록시 피하려고 원점 1.5m 위에서),
  요 유지한 채 똑바로, 바퀴 바닥이 지면 30cm 위. `PawnBlocker` 크기 박스로 메시 채널/응답 겹침 검사,
  겹치면 30cm씩 최대 4회 상승.
- 시나리오 재시작 시 리셋. 로그: 엔진 로그 + `events.log` 태그 `UGV`
  (`자가 복원 시작` / `완료(토크)` / `순간이동`). 튜닝값은 BP 카테고리 `UGV|Self Righting`.
- **판정 개정(최신 빌드 PIE 후)**: test 레벨에서 150°·180°로 놓고 두 번 시험 → 둘 다 대기 2.4~2.7초 중
  **스스로 굴러 섰다**("대기 취소 — 스스로 섰음", 59~60°에서). 토크 경로는 아직 미검증. 이 로그로 틈 발견:
  문턱 바로 아래(예: 55°)로 기대 멈추면 영원히 감지 안 됨 → 넘어짐 = "바퀴 2/3 이상 비접지 + 기울기 ≥45°"
  (기본 문턱 60→45), 대기 취소 = "바퀴 대부분 접지 또는 ≤20°"로 변경. 대기 진입/취소 로그 추가(PIE에선
  health.csv가 꺼져 있어 그 외엔 자세를 알 수 없음).
- **PIE 결과(에디터 로그, 이전 빌드·New_kadex_0811)**: 05:46:51 168°·흔들리는 중(127cm/s) → 토크로 3.6초 만에 복원(17°).
  05:42:31 171°·**완전 정지**(3cm/s) → 6초 동안 1°도 안 움직이고(→172°) 순간이동 폴백.
  - 원인은 토크 크기가 아니라 **컨트롤러 결함**: 비례항만 있어 정지 상태 명령이 (목표 1.57 rad/s × 6)
    ≈ 9.4 rad/s²에서 막혔다. 지붕 모서리 축으로 들어 올리는 데 ≈ g×90cm/회전반경²(~4100cm²) ≈ 21 rad/s²
    (리프트 0.4g 감안 ≈ 13) 필요 → 상한 2500°/s²(≈43)에 닿지도 못함. 흔들리던 경우는 회전 관성으로 넘어감.
  - 수정(PIE 미검증): **적분항**(게인 30/s², 상한 = MaxAccel) — 안 돌면 ~1초 안에 상한까지 쌓이고, 목표
    속도를 넘는 순간 0으로 비워 반대편으로 넘어가지 않게.

### (c) 무게중심(COM)

- Chaos 차량은 `VehicleMovementComp`의 `bEnableCenterOfMassOverride` + `CenterOfMassOverride`(루트
  바디 로컬, `ChaosVehicleMovementComponent.cpp:1638-1642`)를 쓴다. 기존 (0,0,10) → 사용자가 **Z=0**으로
  낮춤(핸들링 유지 위해 더 낮추진 않음).
- 물리 에셋에 COM용 바디 추가 **금지** — 시뮬레이션하는 건 `Hull1`뿐이고 바퀴/포탑 바디는 키네마틱
  (피격 이펙트용 콜리전).
- 대략적 정적 전복각 atan(반 트랙폭≈75 / COM 높이): Z=10 → ~61°, Z=0 → ~67°.
  시각화: `p.Vehicle.ShowCOM 1`.

### (d) 로그 — health.csv에 UGV 자세 컬럼

`Source/titan_example/Diagnostics/TitanHealthRecorderSubsystem.h/.cpp` — `health.csv` 맨 끝에 6컬럼 추가:
`ugv_tilt_max`, `ugv_tilt`, `ugv_pitch`, `ugv_roll`, `ugv_angvel_max`, `ugv_speed_kmh`
(매 프레임 샘플, 1초 구간 최대값; UGV 없거나 stalled 행이면 빈칸). 블랙박스 바이너리는 그대로
(바꾸려면 Version 올리고 디코더 수정 필요).

다른 세션의 `ScenarioMonitorSubsystem` 이벤트 기반 전복 감지(60°/1.5초 FAIL 줄, 주변 상황 포함
충격(jolt) 줄, `ugv_flip` 스냅샷)와 보완 관계 — 그쪽은 다른 세션 작업, 여기선 참조만.

## 4. 검증 방법 / 남은 것

- **장시간 확인**: `health.csv`의 `ugv_tilt_max` — 정상이면 ≲25°(레벨 최대 경사). ≥60° 행이 있으면
  `events.log`의 FAIL 줄 + `자가 복원` 줄과 대조.
- 대기:
  - 다사이클 장시간 2-PC 재실행 — 전복이 재발하는지(원인 확정).
  - `WeaponMesh` 콜리전, 레벨 PhysicsActor 소품 MCP 확인.
  - 복원 중에도 AI 컨트롤러가 주행 입력을 계속 넣음(문제로 관찰되진 않음).
  - 밀려난 병사가 내비메시 살짝 밖에 설 수 있음(도로 위라 위험 낮음).
- 시나리오의 "적 거리별 속도 제한"(15/7/3 km/h, `2026-08-31_ugv_enemy_distance_speed_limit.md`)은
  설계이며 여전히 적을 고려 — 의도적으로 손대지 않음.

## 관련 문서

- `vehicle/ugv/2026-09-16_ugv_flip_spin_projectile_physics_body.md` — 같은 구조(키네마틱 무한질량)의 이전 전복 버그
- `vehicle/ugv/2026-09-10_ugv_0901_suspension_tuning.md`
- `guide/rcws_fire_control_dev_guide.md` 7.1절 — 투사체 콜리전 + 이번 UGV 충돌 정책 메모
