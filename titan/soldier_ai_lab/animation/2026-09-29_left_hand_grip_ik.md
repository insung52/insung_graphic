# 왼손 그립 IK 재가동 (A안 — 총 소켓 위치+회전 · 알파 3층)

2026-09-29 / 진행중 (ABP 저장 · C++ 게임 타깃 컴파일 성공 — 에디터 타깃 빌드 · 소켓 회전 캡처 · PIE 확인 대기) / 디자이너가 FK 로 맞춘 왼손이 GASP 블렌딩(MM 크로스페이드 · AO · 뱅킹)에서 총을 뚫고 손목이 돌아가서, 꺼 두었던 왼손 IK 를 다시 켰다. 목표 = **들고 있는 총 메시의 `LeftHandGrip` 소켓(위치+회전, 총마다 따로)**, 해제 = `DisableLHandIK` 커브 · 사망 · 총 없음 · cvar.

관련: `assets/2026-09-14_design_team_animation_handoff.md` 4절 ⑦ · 5절(IK 를 끈 이유) · `animation/prototypes/2026-09-15_sharp_turn_pop_bf_head_scale_and_weapon_socket.md` 4절(그립 소켓 런타임 산출) · `animation/2026-09-23_low_ready_upper_body_layer.md`("왼손 IK 미사용" → 이 문서로 대체) · `IMPLEMENTED.md` 2.5.
원칙 **P194** · 값 **[C-175]** · 작업 **[W123]**.

---

## 1. 왜 다시 켰나 — FK 만으로는 양손 그립이 블렌딩을 못 버틴다 [A]

- 디자이너가 총 든 클립을 전부 FK 로 맞췄지만 **걷기·뛰기에서 왼손이 총을 뚫거나 손목이 돌아갔다**(사용자 관측). 시퀀스 단독 재생에서는 맞는다.
- 원인은 본별 회전 블렌딩이다. 척추를 통째로 돌리는 층(린 · 오리엔테이션 워핑)은 두 팔을 함께 돌리니 무해하지만, **MM 블렌드 스택 크로스페이드 · AO 메시공간 additive(기준 `Idle_ADS` f0 를 팔 자세가 다른 로코 클립에 얹음) · 뱅킹 `BS1D_Additive_Lean_Run`(GASP 원본, 팔 포함)** 은 왼팔 체인과 오른팔 체인을 따로 보간한다 → 손 사이 거리가 유지되지 않는다.
- 총은 오른손(`weapon_r`)에 붙어 있으므로 **왼손만** 틀어진다. 슈터들이 그래프 마지막에 왼손 IK 를 두는 이유 그대로.

## 2. 옛 IK 를 그냥 켜면 안 됐던 이유 [A · MCP 로 노드 실측]

| 항목 | 옛 `TwoBoneIK_0` | 결과 |
|---|---|---|
| 회전 | `bTakeRotationFromEffectorSpace` / `bMaintainEffectorRelRot` 둘 다 false | **위치만 잡는다 — 손목 돌아감은 그대로** |
| 목표 | 소켓 `weapon_r` + 변수 `LeftHandGripOffset`(BeginPlay 에서 한 번 산출) | 위치만 표현 가능, 회전 오프셋을 넣을 자리가 없다 |
| 알파 스무딩 | `bInterpResult` false | 켜고 끌 때 한 프레임에 튄다 |
| 해제 신호 | `DisableLHandIK` 커브뿐 | 사망 · 총 드롭에 반응 없음 |

## 3. 새 구조

### 3.1 ABP (`SoldierCharacter_ABP`, MCP 로 편집 · 컴파일 · 저장)

```
… → FootPlacement_0 → ModifyBone pelvis → LegIK_1
  → CopyBone_0        hand_r → ik_hand_gun  (Component Space, 위치+회전, 스케일 X)
                      ⚠ bCopyTranslation/Rotation 은 **핀**으로 노출돼 있어 핀 값 true 로 맞췄다 (P25)
  → ModifyBone_10     ik_hand_l  ← Translation = LeftHandGripLocation, Rotation = LeftHandGripRotation
                      Replace · Parent Bone Space (= ik_hand_gun = 지금 프레임의 hand_r 기준)
  → TwoBoneIK_0       hand_l → effector **본 ik_hand_l** (Bone Space, 위치 0)
                      bTakeRotationFromEffectorSpace = **true** (손목까지)
                      alphaScaleBiasClamp: scale −1 / bias +1 (그대로) · bInterpResult = **true** (속도 10/10)
                      EffectorLocation 핀 ← (옛 LeftHandGripOffset 연결 해제, 0)
  → ModifyBone_6 (neck_01) → … (머리 추종 이하 변화 없음)

Alpha = 1 − SelectFloat( A = Max( GetCurveValue("DisableLHandIK"), LeftHandIKBlock ),
                         B = 1.0, bPickA = LeftHandIKEnabled )
```

| 변수 | 기본값 | 누가 쓰나 |
|---|---|---|
| `LeftHandIKEnabled` | false → **true** | 마스터 스위치 (그대로) |
| `LeftHandIKBlock` (신규 float) | **1** | C++ 가 매 틱 0/1. **기본 1 = C++ 가 안 돌면 IK 꺼짐**(그립이 0 이면 왼손이 오른손으로 끌려가므로 안전 쪽 기본값) |
| `LeftHandGripLocation` / `Rotation` (신규) | 0 | C++ 가 매 틱 |
| `LeftHandGripOffset` (옛) | — | **이제 아무도 안 읽는다.** BP BeginPlay 가 여전히 써 넣지만 무해 |

**왜 `ik_hand_gun` 체인을 거치나**: TwoBoneIK 는 회전을 **본/소켓 공간에서만** 가져온다(값으로 회전을 못 준다). 그립 소켓은 캐릭터 메시가 아니라 **총 메시**에 있어서 직접 가리킬 수 없다. 그래서 지금 프레임의 `hand_r` 을 `ik_hand_gun` 에 복사하고 그 자식 `ik_hand_l` 을 그립 자리에 놓아 **effector 본을 그래프 안에서 만든다.** 컴포넌트 공간 값을 C++ 에서 바로 넣으면 한 프레임 늦어 손이 총을 따라 늦게 붙는다 — `hand_r` 기준 상대값은 총이 손에 고정이라 상수라서 지연이 없다.

`ik_hand_gun` / `ik_hand_l` 을 읽는 곳은 이 체인 말고 없다(클립의 `ik_hand_*` 트랙은 어디서도 안 쓰였다 — 이 체인이 매 프레임 덮어쓴다). 메시에 두 본이 **필수 본으로** 있어야 한다: `new_enemy_T` 부모관계 확인(`ik_hand_l → ik_hand_gun`), 아군 `soldier_T` 는 09-23 에 LOD0 `Bones to Remove` 에서 `ik_*` 를 빼서 복구됨(`animation/prototypes/2026-09-23_ally_crouch_ik_bones_removed.md`).

### 3.2 C++ (`Pose/SoldierAIBridgeComponent`)

`UpdateLeftHandIK()` — `TickBridge` 의 `UpdateStance` 다음:

```
bHeld  = WeaponMesh 가 캐릭터 메시에 붙어 있고(IsAttachedTo) LeftHandGrip 소켓이 있다
if bHeld:  Grip(월드) 을 hand_r(월드) 기준으로 → ABP.LeftHandGripLocation / Rotation
Block  = LeftHandIK cvar 0  ||  !bHeld  ||  (IsDead && LeftHandReleaseOnDeath)
ABP.LeftHandIKBlock = Block ? 1 : 0          (완만함은 ABP 의 bInterpResult 가 맡는다)
```

- 바인딩은 **선택적**이다(`bLeftHandIKBound`) — `BindAll` 의 `bBound &=` 에 넣으면 변수 하나 없을 때 **틱 본문 전체가 꺼진다.**
- 매 틱 소켓을 다시 읽으므로 **PIE 중 소켓을 고치면 바로 보인다.**
- 디버그 축 `LHandIK`(bool).

| 콘솔 | 기본 | 뜻 |
|---|---|---|
| `SoldierLab.Pose.LeftHandIK` | 1 | 0 = IK 끄고 FK 왼손 (A/B · 캡처용) |
| `SoldierLab.Pose.LeftHandReleaseOnDeath` | 1 | 1 = 사망 클립 시작 시 손을 놓는다 / 0 = 래그돌까지 쥔다 |
| `SoldierLab.Pose.CaptureLeftHandGrip [0\|1\|2]` | 0 | 들고 있는 총마다 **지금 왼손이 있는 자리**를 그 총의 `LeftHandGrip` 소켓 값으로 환산. 0 출력 · 1 회전만 소켓에 씀 · 2 위치+회전. `LeftHandIK 0` 상태에서(= 디자이너 FK 손) 가만히 서서 쓴다 |

## 4. 왼손을 떼는 상황 — 전수 [A]

| 상황 | IK | 누가 | 근거 |
|---|---|---|---|
| 재장전 | 끔 | 클립 커브 `DisableLHandIK` | `MM_Rifle_Reload`(적/아군) — 실사용 클립 중 이 커브를 가진 건 이것과 아래 TurnLeft_180 뿐(바이트 스캔 전수) |
| `MM_Rifle_TurnLeft_180` | **켬** | 커브 **삭제**(사용자, 09-29) | 1→0 으로 앞 절반 손을 놓던 값. `TurnRight_180` 엔 없어 Lyra 원본 잔재로 판단. ⚠ FBX 에서 온 커브(`disablelhandik_CURVE_CONTROL`)라 **FBX 리임포트 시 되살아날 수 있다** → 원본 DCC 에서도 지울 것 |
| 사망 몽타주 | 끔 | C++ `IsDead`(리플리케이트됨) | 사용자 미결정 → 추천안(놓기) 적용, cvar 로 뒤집을 수 있음 |
| 래그돌 이후 | 무관 | 물리 | `RagdollRootBone = pelvis` → 팔까지 시뮬레이션. 총도 같은 순간 드롭(`SoldierHealth.cpp` `DropCarriedWeapons`) |
| 총이 손에 없음 | 끔 | C++ `!bHeld` | 드롭 후 · 스폰 직후 등 |
| 이동 · 조준 · 사격 반동 · 린 · BF · 앉기 · **로우레디** | 켬 | — | 전부 손이 총에 있는 자세. 09-23 로우레디 문서의 "IK 미사용" 은 이 문서로 대체 |
| 피격 13 | 켬 | — | 커브 없음. **손이 튀어야 하는 클립이 있는지 PIE 육안 확인 대기 → [C-175]** — 있으면 그 클립에 커브 추가 |
| Equip · 근접 · 수류탄 · 권총/샷건 | — | — | 미사용. 배선할 때 커브 확인 (`DisableRHandIK` 는 ABP 가 안 읽는다 — 오른손 IK 없음) |

`DisableLegIK`(점프 5 · Dash) 도 ABP 가 안 읽는다 — 전부 미사용 클립이라 무해.

## 5. 총별 커스텀 — 소켓이 곧 손의 자리

| 진영 | 총 메시 (`WeaponMesh`) | `LeftHandGrip` |
|---|---|---|
| 아군 (`BP_SoldierCharacter` 기본) | `/Game/SoldierLab/Weapons/Meshes/SK_AR4_X` | 있음 — loc (7, −8, 21) · **rot yaw 180 (손 방향 기준 아님)** |
| 적군 (`BP_Soldier_Hostile` 오버라이드) | `/Game/FPS_Weapon_Bundle/Weapons/Meshes/KA74U/SK_KA74U_X` | 있음 (값 미확인) |

**소켓 트랜스폼 = `hand_l` 본(손목) 이 가야 할 트랜스폼.** 위치는 09-15 부터 같은 의미로 튜닝돼 왔고, **회전은 이번에 처음 의미를 갖는다** — 지금 회전값은 손 방향과 무관하므로 그대로 켜면 손목이 뒤집힌다. 그래서 캡처 명령으로 한 번 맞춘다(6절 ②).

## 6. 남은 절차 (사용자 · 다음 세션)

1. **에디터 닫고 에디터 타깃 빌드** — 헤더(클래스 레이아웃)가 바뀌어 Live Coding 불가(P13). 게임 타깃으로 컴파일 에러 없음은 확인했다.
2. **소켓 회전 캡처** — PIE, 아군·적군 병사가 한 명씩 서 있는 상태에서
   `SoldierLab.Pose.LeftHandIK 0` → (가만히 조준 자세) → `SoldierLab.Pose.CaptureLeftHandGrip 1` → `SoldierLab.Pose.LeftHandIK 1`.
   출력 로그에 총별 now/captured 값이 찍힌다. 위치도 디자이너 FK 로 맞추려면 `2`. **총 메시 두 개 P4 체크아웃 후 저장.**
3. **PIE 확인 [C-175]** — 걷기/조깅/선회에서 뚫림·손목 · 재장전에서 손이 떨어졌다 붙는지 · 피격 13 · 사망 시 손 놓음 · 2-PC 클라.

## 7. 바뀐 것

| 무엇 | 내용 |
|---|---|
| `Content/SoldierLab/Animation/SoldierCharacter_ABP` | 변수 3 신설 · `CopyBone_0` · `ModifyBone_10` · Max(Float) · 게터 3 · `TwoBoneIK_0` 설정 · 기본값 `LeftHandIKEnabled` true · `LeftHandIKBlock` 1 |
| `Source/SoldierLab/Pose/SoldierAIBridgeComponent.h/.cpp` | `FVar::WriteVector` · 선택 바인딩 3 · `UpdateLeftHandIK` · cvar 2 · 콘솔 `CaptureLeftHandGrip` |
| P4 | 위 3파일 checkout(default change, 미제출) |

## 8. 원칙 · 값 · 작업

- **P194** — **FK 로 맞춘 양손 그립은 블렌딩을 못 버틴다. 총에 붙은 쪽 손이 아닌 손은 그래프 마지막 IK 로 다시 붙이고, 목표 회전은 본 공간으로 만든다.** TwoBoneIK 는 회전을 값으로 못 받으므로(본/소켓 공간에서만) 다른 컴포넌트의 소켓을 목표로 할 땐 `CopyBone`(기준 본) + `ModifyBone`(부모 공간 상대값) 으로 effector 본을 그래프 안에서 만든다. C++ 에서 컴포넌트 공간 값을 넣으면 한 프레임 늦는다.
- **[C-175]** 피격 13 클립 중 왼손이 총에서 떨어져야 하는 것이 있는가 · 재장전 커브 구간이 디자이너 수정 타이밍과 맞는가 · 보간 속도 10 이 재장전 복귀에 적당한가.
- **[W123]** 에디터 빌드 → 캡처(총 2) → PIE 확인. 이후 필요하면 B안(클립별 그립 = `ik_hand_l` 굽기 모디파이어 + 상체 마스크에 `ik_hand_*` 포함).
