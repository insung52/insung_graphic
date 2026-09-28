# 로우레디(총 내림) 상체 레이어

2026-09-23 / 완료 (빌드 · PIE 확인 — 정지/걷기 총 내림, 조깅 올림, 전환 부드러움) / 정지·걷기에서 총을 내린 자세(LowReady)를 상체에만 얹고 조깅에서 올리는 레이어를 `SoldierCharacter_ABP` 에 신설. 왼손 그립 IK 는 쓰지 않는다. **터진 문제 다섯을 원인까지 적었다** — 그중 둘(**가중치 0 이면 `Layered blend per bone` 노드가 통째로 스킵** · **레이어가 베이스 커브를 덮어쓴다**)은 이 프로젝트에서 재발하기 쉬운 엔진 함정이다.

대상: `Content/SoldierLab/Animation/SoldierCharacter_ABP` · `Source/SoldierLab/Pose/SoldierAIBridgeComponent.cpp` · `BP_SoldierCharacter` 클래스 디폴트 · 스켈레톤 `SK_UEFN_Mannequin` 의 블렌드 마스크 2개.

관련: `animation/2026-09-18_ai_pose_layer_scanturn_gait_smoother.md`(같은 포즈 층 · `USoldierAIBridgeComponent` 의 다른 축) · `animation/prototypes/2026-09-15_sharp_turn_pop_bf_head_scale_and_weapon_socket.md`(**총내림 결론** — 그때의 `WeaponLowered` 는 각도 기반, 이 문서가 속도 축을 더한다) · `ai/2026-09-21_game_thread_structural_pool_rays_bridge.md` ④(`USoldierAIBridgeComponent` 가 BP EventTick 본문을 돈다 — **틱 본문은 C++ 쪽을 고친다**) · `animation/prototypes/2026-09-17_ally_enemy_anim_set_split.md`(아군/적군 2벌).
원칙: **P190**(가중치 0 = 노드 스킵 · 커브 Override) · **P191**(속도 램프는 애님그래프가 아니라 C++) · P167(`RampAxisTo` rate ≤ 0 = 스냅).
값: **[C-169]**(블렌드 마스크 가중치) · **[C-170]**(조깅 raise 구간). 작업: **[W117]**(cvar 5개 UPROPERTY 승격).

---

## 1. 목표

- **정지·걷기** = 총을 내린 자세(LowReady).
- **조깅** = 총을 올림.
- 상태 전환은 **부드럽게**(속도가 두 프레임에 0 이 돼도 자세가 두 프레임에 안 바뀐다).
- **왼손 그립 IK 는 쓰지 않는다** — `TwoBoneIK_0` 은 그대로 두되 이 레이어가 그 뒤를 건드리지 않는다.

---

## 2. 그래프 구조 (데이터 흐름 순서)

`SoldierCharacter_ABP` 애님그래프. 번호는 흐르는 순서대로다.

```
① 소스 포즈 6개 (Sequence Evaluator)
     ALLY / Enemy  ×  Rifle/Poses/*_MM_Rifle_LowReady
                      Rifle/Idles/*_MM_Rifle_Idle_ADS
                      Rifle/Idles/*_MM_Rifle_Idle_Hipfire
        └→ 진영 선택: Blend Poses by bool (UseAllyAnimSet, blend time 0) ×3
                                          │
② 로우레디 포즈 = Apply Mesh Space Additive
        Base     = Idle_ADS
        Additive = LowReady          Alpha 1
                                          │
③ 걷기 흔들림 additive = Make Dynamic Additive (bMeshSpaceAdditive)
        Base     = Idle_Hipfire (정지 자세)
        Additive = Use cached pose 'AimedPose'   ← 지금의 로코모션 포즈
        = "지금 로코모션 포즈 − 정지 힙파이어 자세"
                                          │
④ 흔들림에서 팔·목을 지운다 = Layered blend per bone
        Base  = ③
        Layer = Additive Identity Pose
        마스크: clavicle_l / clavicle_r / neck_01 이하 = 흔들림 0
                                          │
⑤ 로우레디 + 흔들림 = Apply Mesh Space Additive
        Base     = ②
        Additive = ④
        Alpha    = Map Range Clamped(Speed2D, 20 → 150) × 0.6
                                          │
⑥ 최종 레이어 = Layered blend per bone
        Base   = Use cached pose 'AimedPose'      ← 로코모션 전신
        Layer  = ⑤                                 (Mesh Space Rotation Blend)
        가중치 = FInterp Ease in Out(WeaponLowered, Exponent 2)
        Curve Blend Option = UseBasePose           ★ 5.3절
                                          │
⑦                            Slot 'UpperBody' 의 Source      ★ 5.2절
```

- **② 가 두 장을 합치는 이유**: `*_MM_Rifle_LowReady` 애님은 **ADS 기준 MeshSpace additive** 다. 단독으로는 포즈가 아니라 차이값이라, ADS 위에 얹어야 "총 내린 자세"가 완성된다 [A].
- **③ 이 additive 인 이유**: 로우레디 포즈는 정지 한 장뿐이다. 걸을 때의 몸 흔들림은 로코모션 포즈에서 **빼서** 가져온다 — "지금 포즈 − 정지 포즈" 가 곧 흔들림이다.
- **⑦ 이 슬롯 Source 인 이유**: 5.2절.
- **ABP 변수 `WeaponLowered`** 는 C++ 브리지가 매 틱 써 준다(3절). 애님그래프 안에 속도 매핑이 **없다** — 5.4절이 그 이유다.

---

## 3. C++ 쪽 — 포즈 전용 램프

`USoldierAIBridgeComponent::UpdateBodyYawRate` 말미(`Pose/SoldierAIBridgeComponent.cpp:429-445`).

```cpp
const float Speed2D = Movement->Velocity.Size2D();
const float LoweredBySpeed = Speed2D < GSoldierPoseIdleSpeed ? 1.f
    : MapClamped(Speed2D, GSoldierPoseRaiseSpeedStart, GSoldierPoseRaiseSpeedFull,
                 GSoldierPoseWalkLowered, GSoldierPoseJogLowered);
const float PoseTarget  = WeaponTarget * LoweredBySpeed;
const float PoseLowered = ABP_WeaponLowered.ReadFloat(AnimBP);      // 이전 값은 ABP 가 든다
const float NewPoseLowered = FMath::Max(
    RampAxisTo(PoseLowered, PoseTarget, WeaponRaiseRate, WorldDelta),
    RampAxisTo(PoseLowered, PoseTarget, WeaponLowerRate, WorldDelta));
ABP_WeaponLowered.WriteFloat(AnimBP, NewPoseLowered);
```

★ **캐릭터의 `WeaponLowered` 와 ABP 의 `WeaponLowered` 는 서로 다른 값이다** [A · `SoldierAIBridgeComponent.h:107-112`]. 같은 목표(`WeaponTarget`)·같은 레이트를 쓰지만:

| | 캐릭터 `WeaponLowered` | ABP `WeaponLowered`(`ABP_WeaponLowered`) |
|---|---|---|
| 쓰는 곳 | 몸통 요 회전속도(`YawRate_Up/Down` 보간) · 조준 보정 게인(`:304`) | **로우레디 레이어 가중치만** |
| 속도 축 | **없다** — 조준 오프셋이 꺼지면 항상 1 | `LoweredBySpeed` 가 곱해진다 |
| 상태 보관 | 캐릭터 BP 변수 | **ABP 변수**(C++ 은 상태를 안 든다 — 매 틱 읽고 쓴다) |

**이 분리가 핵심이다.** 캐릭터 값에 속도를 곱하면 조깅할 때 몸통 회전속도와 조준 보정이 같이 바뀐다 — 총을 올리는 것과 몸이 도는 속도는 다른 이야기다.

---

## 4. 커스터마이즈 지점

| 무엇을 | 어디서 | 현재 값 |
|---|---|---|
| **내림/올림 시간** | `BP_SoldierCharacter` 클래스 디폴트 `WeaponRaiseRate` / `WeaponLowerRate` | **2.5** / **3** (이전 2 / 8) |
| **속도별 내림 정도** | cvar `SoldierLab.Pose.WalkLowered` / `.JogLowered` | **1.0** / **0.0** (0 = 사격자세, 1 = 완전 로우레디) |
| 정지 판정 속도 | cvar `SoldierLab.Pose.IdleSpeed` | **20** (아래면 무조건 1) |
| 올리기 시작/완료 속도 | cvar `SoldierLab.Pose.RaiseSpeedStart` / `.RaiseSpeedFull` | **250** / **400** |
| **전환 곡선** | 애님그래프 `FInterp Ease in Out` 의 Exponent | **2** (1 이면 선형) |
| 걷기 흔들림 양 | 애님그래프 `Multiply` 의 B | **0.6** |
| 흔들림 켜지는 속도 구간 | 애님그래프 `Map Range Clamped` | **20 / 150** |
| 로우레디 자세 자체 | LowReady `Sequence Evaluator` 의 Explicit Time | **2.5** |

> **`WeaponRaiseRate` / `WeaponLowerRate` 를 다시 튜닝한 이유** [A]: 예전 레이어는 **spine_05 부터만** 덮어서 총이 거의 안 움직였고, 그래서 이 두 값을 바꿔도 체감이 없었다(2 / 8 은 "체감 없는 상태"에서 고른 값). 현재 레이어는 **spine_01 기준**으로 내려와 값이 실제로 보인다 → 2.5 / 3 으로 재조정. 정확히 어느 본을 얼마나 덮는지는 이제 블렌드 마스크가 정한다(6절).

---

## 5. 터졌던 문제 다섯 — 증상 · 원인 · 해결

### 5.1 걷기 발이 2배속 [A · 원인 확정]

**증상**: 레이어를 붙이자 걷기 발놀림이 두 배 빨라졌다.

**원인**: **로코모션 출력의 포즈 fan-out.** 같은 로코모션 포즈 노드 출력을 두 군데(③의 Additive 와 ⑥의 Base)에 연결하면 **그 서브그래프가 두 번 평가된다** — 모션매칭/시퀀스의 시간이 프레임당 두 번 전진한다. titan 쪽에서 이미 한 번 겪은 함정이다(`feedback_animgraph_pose_fanout_double_speed`: "애니메이션이 빠르면 재생배율보다 포즈 fan-out 부터 확인").

**해결**: 기존 **`AimedPose` 캐시를 두 번 읽는다**(`Use cached pose 'AimedPose'` ×2). 캐시는 프레임당 한 번만 평가되고 읽기는 공짜다.

⚠ **새 `SaveCachedPose` 는 MCP 로 만들 수 없었다** — 기존 캐시를 재사용한 것은 그 제약 때문이기도 하다. 새 캐시가 필요하면 에디터에서 손으로.

### 5.2 플레이어가 빙의하면 로우레디가 전혀 안 보인다 [A · 원인 확정]

**증상**: AI 병사에서는 보이는데, 플레이어가 빙의하면 총이 전혀 안 내려간다.

**원인**: 레이어 출력을 `LayeredBoneBlend_1.Base` 에 먹였는데, **그 위에 가중치 1 짜리 상체 슬롯이 있었고** 그 슬롯이 (몽타주가 없을 때 흘리는) 캐시 포즈로 **통째로 덮어썼다.** 레이어는 계산되고 있었고 결과만 버려졌다. AI 에서 보였던 건 그쪽 경로가 그 슬롯을 안 타서다.

**해결**: 레이어 출력을 **`Slot 'UpperBody'` 의 Source** 로 옮겼다(②의 ⑦). 슬롯은 "몽타주가 없으면 Source 를 그대로 통과"이므로, 몽타주가 오면 자연스럽게 몽타주가 이긴다.

> 같은 함정의 일반형은 이미 기록돼 있다 — "애니메이션 진단은 3층을 동시에(C++ 값 / ABP 변수 / **슬롯 가중치**)". **전신 몽타주 슬롯이 이동 포즈를 덮는** 그 함정의 상체판이다.

### 5.3 ★ 총이 거의 다 올라왔을 때 오른손이 턱 하고 튄다 [A · 엔진 소스 확정]

**증상**: `WeaponLowered` 가 0 에 **가까워질 때**(총이 거의 다 올라온 순간) 오른손/상체가 한 번 툭 튄다. 0 에 도달한 뒤엔 멀쩡하다.

**원인 — `Layered blend per bone` 의 `Curve Blend Option` 기본값이 `Override` 다** [A · 엔진 소스]:

1. 엔진 구현상 **레이어 가중치가 0 보다 크기만 하면 레이어 쪽 커브가 베이스 커브를 통째로 덮는다**(`AnimationRuntime.cpp` `BlendCurves` 의 기본 분기 — Override 는 가중치와 무관하게 "있으면 덮는다").
2. 그런데 **가중치가 정확히 0 이 되면 노드 자체가 스킵되어**(`AnimNode_LayeredBoneBlend.cpp:249` — 모든 블렌드 가중치가 0 이면 `BasePose` 를 그대로 출력하고 반환) **베이스 커브가 그대로 나온다.**

→ 흔들림 additive(③)를 타고 **로코모션 커브**(`Enable_Warping`, `MoveData_Speed`, `contact_l`/`contact_r`, `Disable_AO` …)가 레이어 쪽으로 따라 들어왔고, 그것이 0.6 → 0 으로 **줄어들다가 마지막 프레임에 1 로 튄다**(스킵되면서 베이스 커브 복귀). `Enable_Warping` 이 0.x 에서 1 로 한 번에 오르면 **오리엔테이션 워핑이 한 번에 돌아가고**, 그게 손이 튀는 모습이다.

**해결**: **`Curve Blend Option = UseBasePose`.** 레이어는 뼈만 얹고 커브는 항상 베이스(로코모션) 것을 쓴다. 이 레이어는 포즈를 바꾸는 것이지 워핑/접지 신호를 바꾸는 것이 아니므로 의미상으로도 맞다.

### 5.4 ★ 멈추면 1~2프레임 만에 총이 역순으로 팍 내려간다 [A · 원인 확정]

**증상**: 조깅하다 멈추면 총이 **부드럽게** 내려가는 게 아니라 한두 프레임 만에 역재생하듯 뚝 떨어진다.

**원인**: 처음엔 조깅 올리기를 **애님그래프에서 `Speed2D` 에 직결**했다(`Map Range Clamped` 250/400). 그런데 CMC 감속은 **2프레임 안에 400 → 0** 이다 — 입력이 계단이면 출력도 계단이다. `WeaponRaiseRate`/`LowerRate` 램프는 캐릭터 값에만 걸려 있었고 이 경로는 램프를 안 탔다.

**해결**: **속도 기반 올리기를 C++ 램프로 옮겼다**(3절). 같은 목표·같은 레이트를 쓰되 **캐릭터 `WeaponLowered` 와 분리된 별도 값**으로 램프하고, 이전 값은 ABP 에서 읽어 온다. 캐릭터 쪽 값은 몸통 요 회전속도와 조준 보정 게인을 계속 쓰므로 안 건드린다.

**애님그래프의 250/400 노드는 연결을 끊어 무효화했다**(Keep = 1 고정). ⚠ **노드가 그래프에 남아 있으니** 다음 사람이 "여기서 속도를 매핑하는구나" 하고 읽을 수 있다 — 값을 만질 곳은 cvar 다(4절).

### 5.5 총을 앞으로 내미는 것처럼 보인다 [A · 원인 확정]

**증상**: walk 와 jog 를 둘 다 완전 내림으로 뒀더니, 걸을 때 총이 내려간 게 아니라 **앞으로 내민** 모양이 됐다.

**원인**: 흔들림 additive(③ = `AimedPose` − 정지 힙파이어)에는 **움직임만 들어 있지 않다.** 로코모션 애님의 팔 자세와 정지 힙파이어의 팔 자세 사이에는 **정적 오프셋**이 있고, 그 차이도 통째로 additive 에 실린다. 그것이 × 0.6 으로 얹히면서 총을 앞으로 밀었다.

**해결**: 구조 ④ — 흔들림 additive 를 `Layered blend per bone` 에 통과시켜 **clavicle_l/r · neck_01 이하에서 흔들림을 0 으로 지운다**(반대 입력 = `Additive Identity Pose`). 몸통/골반의 흔들림만 남고 팔·목·머리는 로우레디 포즈가 그대로 정한다.

---

## 6. 블렌드 마스크 2개

스켈레톤 `SK_UEFN_Mannequin` 에 신설:

| 마스크 | 쓰이는 곳 | 하는 일 |
|---|---|---|
| `BM_LowReady_Layer` | ⑥ 최종 레이어 | 레이어가 **어느 본을 얼마나** 덮는지 |
| `BM_LowReady_Sway` | ④ 흔들림 제거 | 흔들림을 **어느 본에서 지우는지** |

두 `Layered blend per bone` 노드를 **`Blend Mode = Blend Mask`** 로 바꿔 연결했다.

**왜 Branch Filter 가 아닌가** [A]: Branch Filter 는 "이 본부터 자식 전부 가중치 1" 이라 **본별 가중치를 못 준다.** 척추를 따라 0.3 → 0.7 → 1.0 처럼 점증시키려면 마스크여야 한다. (4절의 "예전엔 spine_05 부터만 덮었다"가 Branch Filter 시절의 한계다.)

**마스크 가중치 값 자체는 사용자가 스켈레톤 에디터에서 조정한다** → **[C-169]** 미측정.

---

## 7. ⚠ 재발하기 쉬운 함정 (굵게 적어 둔다)

1. **`Layered blend per bone` 은 모든 블렌드 가중치가 정확히 0 이면 노드가 통째로 스킵된다**(`AnimNode_LayeredBoneBlend.cpp:249`). 0 에 **도달하는 순간** 출력이 "레이어 결과" 에서 "베이스 그대로" 로 **불연속 전환**된다. 뼈는 어차피 같아서 안 보이지만 **커브는 보인다**(5.3절).
2. **`Layered blend per bone` 을 쓸 때 additive 경로에 로코모션 커브가 따라 들어온다.** `Curve Blend Option` 기본값이 `Override` 라 **레이어 가중치가 0 보다 크기만 하면** 레이어 커브가 베이스를 덮는다 — 워핑/접지 커브가 조용히 바뀌고, 증상은 커브가 아니라 **손이 튀는 모습**으로 나타난다. 상체 레이어에 커브 의도가 없으면 **`UseBasePose` 로 둘 것.**
3. 포즈 출력을 두 군데에 연결하면 그 서브그래프가 두 번 돈다 → **재생이 2배속**(5.1절). 배속을 의심하기 전에 fan-out 을 본다.
4. 슬롯이 위에 있으면 그 아래의 어떤 레이어도 안 보인다 — **슬롯 가중치는 애니메이션 진단 3층 중 하나다**(5.2절).

---

## 8. 남은 것

| # | 항목 | 판정 기준 |
|---|---|---|
| **[W117]** | **cvar 5개를 `UPROPERTY` 로 승격** — `SoldierLab.Pose.IdleSpeed` / `.WalkLowered` / `.JogLowered` / `.RaiseSpeedStart` / `.RaiseSpeedFull` | 지금은 `FAutoConsoleVariableRef`(`SoldierAIBridgeComponent.cpp:96-119`)라 PIE 마다 다시 쳐야 하고 디자이너가 못 만진다. **헤더 리플렉션이 바뀌므로 Live Coding 불가** — **다음에 에디터를 닫는 정식 빌드 때 같이** (`feedback_live_coding_uproperty_missing_property`). 어디에 둘지: 캐릭터 클래스 디폴트(`WeaponRaiseRate` 옆)가 자연스럽다 |
| **[C-169]** | **블렌드 마스크 가중치 미확정** — `BM_LowReady_Layer` / `BM_LowReady_Sway` | 본별 가중치를 사용자가 스켈레톤 에디터에서 조정 중. 판정 = 완전 내림(1.0)에서 상체만 내려가고 **골반/다리 로코모션이 안 상하는가**, 흔들림 제거가 **팔만** 잡고 몸통은 살리는가 |
| **[C-170]** | **조깅 raise 구간 값 미측정** — `RaiseSpeedStart 250` / `RaiseSpeedFull 400` | 지금 값은 걷기(≈200)와 조깅(≈450)의 사이를 눈대중으로 가른 것. 판정 = 걷기 최고속에서 총이 안 올라오고, 조깅 진입에서 **완전히** 올라오는가(중간에 어중간하게 걸리지 않는가). AI 의 `GetDesiredGait()`(Walk/Jog)와 속도 문턱이 어긋나면 "걸으라고 했는데 총은 올린" 상태가 난다 |

---

## 9. 바뀐 것

| 무엇 | 내용 |
|---|---|
| `Content/SoldierLab/Animation/SoldierCharacter_ABP` | 2절 구조 ①~⑦ 신설, `Slot 'UpperBody'` Source 재배선, 250/400 매핑 노드 연결 해제 |
| `Source/SoldierLab/Pose/SoldierAIBridgeComponent.h` | `FVar ABP_WeaponLowered`(`:107-112`) |
| `Source/SoldierLab/Pose/SoldierAIBridgeComponent.cpp` | cvar 5개(`:96-119`) · 바인딩(`:195`) · 포즈 전용 램프(`:429-445`) |
| `BP_SoldierCharacter` 클래스 디폴트 | `WeaponRaiseRate` 2 → **2.5**, `WeaponLowerRate` 8 → **3** |
| `SK_UEFN_Mannequin` | 블렌드 마스크 `BM_LowReady_Layer` · `BM_LowReady_Sway` 신설 |
