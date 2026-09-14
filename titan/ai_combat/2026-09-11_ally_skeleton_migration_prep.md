# 아군 신규 스켈레톤 이관 — 완료

2026-09-11 / 완료(PIE 육안 검증 대기) / 아군을 새 스켈레톤(`soldier_T_Skeleton`)으로 전면 이관. 블렌드스페이스 2종 신설, `ABP_Ally_kadex_T` 배선(본 참조 16곳 + 시퀀스 8곳), `Rifle_Socket` 신설, `BP_Ally_kadex`와 레벨 인스턴스 25명 전환까지 완료. 적군도 `AS_Enemy/` 전용 애셋으로 정리해 아군 의존 제거.

선행 문서: `2026-09-10_ally_enemy_animation_split.md`(분리 사고 경위), 요청 메일 `2026-09-11_ally_pose_request_email.html`.

---

## 1. 입고된 아군 애니메이션 — 폴더가 둘, 최종본은 `KS_`

디자이너가 같은 세트를 **두 폴더에 중복 입고**했다.

| 폴더 | 접두 | 개수 | 시각 |
|---|---|---|---|
| `Soldiers/AnimationSequences/New_Animation_k_army/` | `AS_` | 30 | 09/10 08:53 |
| `Soldiers/AnimationSequences/New_k_army_Animation/` | `KS_` | 29 | 09/10 15:51 |

**`KS_`가 최종본이다.** 근거는 재생 길이다 — `KS_`는 적군 세트(`AS_Enemy/`, PIE 검증 완료)와
프레임 단위까지 일치하고, `AS_`는 짧게 잘려 있다.

| 애니메이션 | `KS_` | `AS_` | 적군 |
|---|---|---|---|
| Jog_Fwd | 0.667s / 21key | 0.5s / 16key | **0.667s / 21key** |
| Aim_Walk_Fwd | 1.367s / 42key | 1.0s / 31key | **1.367s / 42key** |
| Crouch_Aim_Walk_Fwd | 1.4s / 43key | 1.0s / 31key | **1.4s / 43key** |
| Crouch_Aim_Run_Forward | 0.767s / 24key | 0.767s / 24key | 0.767s / 24key |

> **발소리 노티파이는 `AS_` 쪽에만 있다**(27/30 vs `KS_` 1/29). 재익스포트에서 유실된 것으로
> 보이지만, **`AS_`의 노티파이를 그대로 옮길 수는 없다** — 길이가 달라 프레임이 안 맞는다.
> 다만 발소리는 `UFootstepAudioComponent`의 **타이머 방식이 기본값**이라 소리 자체는 나며,
> 적군은 애초에 노티파이가 0개인 채로 운용 중이다. 우선순위 낮음.

## 2. 블렌드스페이스 2종 구축 — 이동 구간 완료

사용자가 빈 애셋 생성, 축·샘플은 MCP로 채움(이 프로젝트는 새 애셋을 MCP로 만들지 않음).
축 설정은 적군과 동일 — `Speed 0~600 / grid 4`, `Direction -180~180 / grid 8`, 둘 다 스냅 켜짐.

- **`BS_ally_kadex_T`** — 18샘플. 걷기(300) 9칸, 뛰기(600) 9칸
- **`BS_ally_kadex_T_Crouch`** — 14샘플. 걷기(300) 9칸, 뛰기(600) 5칸(대각선 없음)

**속도 0(정지) 줄은 일부러 비워뒀다.** 샘플이 아예 없으면 로드 에러 없이 가장 가까운 값으로
클램프되므로, 정지 포즈가 들어오면 9칸씩만 추가하면 끝난다.

판단이 필요했던 두 가지:

- **숙여 뛰기 우측은 `AS_` 폴더의 `AS_Crouch_Aim_Run_Right_2`로 임시 사용.** `KS_` 세트에서만
  누락됐는데, 이 애셋은 길이가 적군 것과 정확히 일치(0.6s/19key)해서 임시 대체에 문제없다.
  `KS_` 버전이 들어오면 그 한 칸만 교체.
- **숙여 뛰기 전진에 `rateScale 1.3` 유지.** 적군과 길이가 동일하므로 같은 보정이 필요하다고
  판단. 실측 후 조정 가능.

## 3. `ABP_Ally_kadex2` 본 참조 전수 조사 — 16곳

새 스켈레톤은 UE5 마네킹 이름이라 구 Mixamo 이름과 **겹치는 본이 하나도 없다.** 스켈레톤을
바꾸는 순간 아래가 전부 무효가 된다.

| 노드 | 현재 본 | → 새 본 |
|---|---|---|
| `ModifyBone_1` | `Spine` | `spine_01` |
| `ModifyBone_3` | `Spine1` | `spine_03` |
| `ModifyBone_0`, `ModifyBone_2` | `Spine2` | `spine_04` |
| `TwoBoneIK_0` (IK 본 / 조인트) | `LeftHand` / `LeftForeArm` | `hand_l` / `lowerarm_l` |
| `LayeredBoneBlend_1` | `Spine2` | `spine_04` |
| `LayeredBoneBlend_2`, `_4` | `Spine` | `spine_01` |
| `LayeredBoneBlend_3` | `LeftShoulder` | `clavicle_l` |
| `LayeredBoneBlend_5` | `LeftShoulder`/`RightShoulder`/`Neck` | `clavicle_l`/`clavicle_r`/`neck_01` |
| `LayeredBoneBlend_6` | `Neck`/`LeftShoulder`/`RightShoulder` (깊이 **-1**) | 동일 + 깊이 문제 별도 |

> **척추 매핑만 확정이 아니다.** 구 스켈레톤은 척추가 3개, 새 것은 5개(`spine_01`~`spine_05`)다.
> 위 값은 제안이며 기울임·조준각이 어색하면 한 칸씩 조정해야 한다. 눈으로 잡는 수밖에 없다.
>
> **`LayeredBoneBlend_6`의 깊이 -1은 기존 결함이다.** 엔진 규칙상 깊이가 음수면 모든 본
> 가중치가 0이라 그 블렌드 포즈가 최종 결과에 반영되지 않는다(`2026-09-01_animation_asset_inventory.md`
> 2절에서 지적한 죽은 경로). 이관하면서 같이 고칠 것.

## 4. AnimGraph 오버레이 5종 — 실사용 여부 확정

블렌드스페이스 밖에서 직접 재생되는 시퀀스가 5개 있고, 그중 **3개는 코드가 켜는 플래그로
게이팅**된다. 이관 전에 어느 것이 살아있는지 확인했다.

| 노드 | 애셋 | 게이팅 플래그 | 판정 |
|---|---|---|---|
| `SequencePlayer_4` | `AS_RifleIdle` (조준 정지) | 상시 | **사용 중** |
| `SequencePlayer_1` | `AS_RifleAim` (총 내린 정지) | 상시 | **사용 중** |
| `SequenceEvaluator_0` | `AS_Stopsign` (정지 수신호) | `IsLeftArmBlending` | **사용 중** |
| `SequenceEvaluator_2` | `AS_Inspecting` | `IsInspecting` | **아군에선 죽음** |
| `SequenceEvaluator_1` | `AS_Enemy_Looking_Around` | `IsLookingAround` | **아군에선 죽음** |

- **`AS_Stopsign`이 살아있다는 것이 이번 조사의 수확.** `ScenarioStateSubsystem.cpp:1224`에서
  분대장이 UGV 정차 시 `SetStopsignRaised(true)`를 부르고, 그게
  `AllyFormationComponent.cpp:488`의 리플렉션으로 `IsLeftArmBlending`을 세팅한다.
  **요청 목록에 빠져 있었으므로 추가**(메일 갱신함).
- `IsInspecting`은 **프로젝트 전체에서 세팅하는 코드가 없다**(BP_ThirdPersonCharacter의 플레이어
  입력용 변수). `IsLookingAround`는 `EnemyCombatComponent`만 세팅한다 — 적군 전용.
  둘 다 아군 ABP에는 복제 과정에서 딸려온 잔재다.
- **다만 노드를 지우지는 않았다.** 레이어드 블렌드 사슬 중간에 있어 삭제하면 포즈 배선을
  다시 이어야 한다. 이관 시점에 **알파가 항상 0이므로, 시퀀스만 새 스켈레톤의 아무 애셋으로
  바꿔치면 충분**하다(특히 `AS_Enemy_Looking_Around`는 적군 스켈레톤이라 그대로 두면 깨진다).

## 5. `BP_Enemy_Base` 슬롯 배선 — 완료 (저장 함정 1건)

적군 슬롯이 아직 아군 애셋을 물고 있던 것을 교체했다. **완료, 디스크 반영 확인.**

- `FireRecoil` → `AS_Enemy/Firing_Rifle_Anim_2`
- `ReloadSlot` → `AS_Enemy/AS_Reloading`

**`set_pin_value`는 패키지를 dirty로 표시하지 않는다.** 그래서 첫 시도에서 `save_assets`가
아무 일도 하지 않고 `true`를 반환했고(`is_dirty`가 계속 false), 편집 내용은 패키지가 리로드되면서
**통째로 유실**됐다.

해결: 핀 값을 다시 넣은 뒤 **`set_node_position`으로 노드를 1픽셀 밀었다 되돌려 dirty를 강제**하고
컴파일·저장했다. `611957 → 612476 bytes`로 변한 것까지 확인.

```python
bp_tool('set_pin_value', {...})
bp_tool('set_node_position', {'node': ref, 'pos': {'x': x + 1, 'y': y}})  # dirty 강제
bp_tool('set_node_position', {'node': ref, 'pos': {'x': x,     'y': y}})
```

> **오진 기록**: 처음엔 `p4 fstat`에 `otherOpen0 user2@user2_jiseong`가 찍혀 있어 타 사용자
> 체크아웃 점유를 원인으로 지목했으나 **틀렸다.** 로컬 read-only 속성만 풀려 있으면 저장은 된다.
> 실제 원인은 위의 dirty 플래그였다.
>
> 재발 방지: `save_assets`의 반환값을 믿지 말고 **디스크 mtime/크기로 검증**할 것. 저장이 안 되면
> `is_dirty`부터 확인하고, false면 P4를 의심하기 전에 dirty를 강제할 것.

## 6. `ABP_Ally_kadex_T` 배선 — 완료

사용자가 `ABP_Ally_kadex2`를 복제·리타겟해 `/Game/Soldiers/ABP_Ally_kadex_T`(타겟 스켈레톤
`soldier_T_Skeleton`)를 만들었고, 그래프 전체를 훑어 **애셋·본을 참조하는 노드 22개를 전수
교체**했다. 컴파일·저장 완료, 에러 없음.

### 6.1 블렌드스페이스 3곳

| 상태 | 이전 | 이후 |
|---|---|---|
| `AimLocomotion` | `BS_ally_kadex` | `BS_ally_kadex_T` |
| `LoweredLocomotion` | `BS_ally_kadex` | `BS_ally_kadex_T` |
| `Knee` | `BS_ally_kadex_Crouch` | `BS_ally_kadex_T_Crouch` |

### 6.2 본 참조 16곳 — 3절 매핑표대로 적용

`ModifyBone` 4개(`spine_04`×2 / `spine_01` / `spine_03`), `TwoBoneIK`(`hand_l`/`lowerarm_l`),
`LayeredBoneBlend` 6개의 브랜치 필터 10개 항목 전부.

> **`LayeredBoneBlend_6`의 깊이 -1은 일부러 그대로 뒀다.** 이름만 `neck_01`/`clavicle_l`/
> `clavicle_r`로 바꿨다. 깊이를 고치면 지금 화면에 안 나오던 오버레이가 켜지는 **동작 변경**이라,
> 이관과 분리해서 별도로 판단하는 게 맞다.

### 6.3 시퀀스 7곳 — 임시 플레이스홀더

정지 포즈가 아직 없어서, 구 스켈레톤 애셋 7개를 전부 `KS_Aim_Walk_Fwd_2`로 임시 대체했다
(**컴파일을 통과시키기 위한 조치**). 포즈가 들어오면 아래 표대로 교체한다.

| 노드 | 위치 | 들어갈 포즈 |
|---|---|---|
| `SequencePlayer_4` | AnimGraph | 서서 조준 정지 |
| `SequencePlayer_1` | AnimGraph | 총 내린 정지 |
| `SequenceEvaluator_0` | AnimGraph | 정지 수신호 |
| `SequencePlayer_5` | `ProneIdle` 상태 | 엎드린 정지 |
| `SequencePlayer_7` | `ToProne` 상태 | 엎드리기→무릎 |
| `SequencePlayer_6` | `FromProne` 상태 | 엎드리기→무릎 |
| `SequenceEvaluator_1`, `_2` | AnimGraph | **교체 불필요**(4절의 죽은 경로) |

**의존성 확인 결과 구 스켈레톤 참조가 0개다** — `KS_Aim_Walk_Fwd_2`, `BS_ally_kadex_T`,
`BS_ally_kadex_T_Crouch`만 남았다.

`BP_Ally_kadex`는 아직 `ABP_Ally_kadex2`를 물고 있다(원본도 무손상). **인게임 아군은 그대로
구버전으로 동작 중**이며, 전환은 정지 포즈 입고 후에 한다.

## 7. 정지 포즈 입고(CL 461) → 이관 완료

디자이너가 **아군 정지 포즈 5종**을 입고(CL 461). 전부 `soldier_T_Skeleton`.

| 애셋 | 길이 | 용도 |
|---|---|---|
| `KS_RifleIdle` | 7.70s / 232key | 서서 조준 정지 |
| `KS_RifleAim` | 3.10s / 94key | 총 내린 정지 |
| `KS_Crouching` | 2.10s / 64key | 숙여 정지 |
| `KS_ProneIdle` | 4.97s / 150key | 엎드린 정지 |
| `KS_ProneToKneel` | 1.97s / 60key | 엎드리기↔무릎 전환 |

> 정지 수신호(`AS_Stopsign`)와 `AS_Inspecting`은 **사용자 판단으로 요청에서 제외**했다.
> 해당 오버레이 노드는 `KS_RifleIdle`(중립 포즈)을 물려뒀다. `ScenarioStateSubsystem.cpp:1224`의
> `SetStopsignRaised()` 경로는 코드에 남아 있으므로, 시나리오가 이를 호출하면 팔 부분에
> 중립 포즈가 얹힌다(깨지지는 않음).

### 7.1 적용 내역

- **블렌드스페이스** — 속도 0 줄 9칸씩 추가. `BS_ally_kadex_T` **27샘플**(`KS_RifleIdle`),
  `BS_ally_kadex_T_Crouch` **23샘플**(`KS_Crouching`). 적군과 구조 동일.
- **CL 460 반영** — `KS_Crouch_Aim_Run_Right_2`·`KS_Run_Left`가 들어와, 임시로 쓰던 구 폴더
  애셋과 접두 없는 `Run_Left`를 교체. **두 블렌드스페이스 모두 100% `KS_` 애셋.**
- **`ABP_Ally_kadex_T`** — 플레이스홀더 8곳을 실제 포즈로 교체. 의존성에 구 스켈레톤 애셋 0개.
- **`Rifle_Socket` 신설** — `soldier_T`의 `hand_r` 본에 추가. 트랜스폼은 디자이너가 잡아둔
  `hand_rSocket` 값을 복사(위치 `-4.23, 2.63, -0.42` / 회전 `-13.9, 179.8, 0`). **추정치이므로
  총 위치 육안 확인 필요.** 기존 `hand_rSocket`은 그대로 뒀다.
- **`BP_Ally_kadex`** — 메시 `soldier_T`, AnimClass `ABP_Ally_kadex_T_C`. 상대 트랜스폼
  (`Z -89`, `yaw -90`)은 유지.

### 7.2 ★ 레벨 인스턴스 25명 — CDO만 고치면 안 바뀐다

`BP_Ally_kadex`의 CDO를 바꿔도 **`New_kadex_0811`에 배치된 아군 25명은 구 메시/구 ABP를 그대로
들고 있었다.** PIE는 인스턴스 값을 쓰므로 BP만 고치면 게임에서는 아무 변화가 없다.

25명 전부에 메시·AnimClass를 직접 써넣고 전수 검증(25/25 `soldier_T | ABP_Ally_kadex_T`,
상대 트랜스폼 이상 0건) 후 레벨 저장. 이 레벨은 **월드파티션이 아니라** 액터 개별 저장이
불가능하다(`save_actor`가 "not an external actor asset"로 거부) — 레벨 통째로 저장해야 한다.

**적군은 해당 없음**(15명 전수 확인, `Enemy | ABP_Enemy_kadex2`로 CDO와 일치). 적군은 메시·ABP를
바꾼 게 아니라 ABP 내부가 참조하는 시퀀스만 교체했으므로 인스턴스에 자동 반영된다.

## 8. 적군 정리 — `AS_Enemy/` 단일화 완료

CL 459에서 user1이 **적군 전용 `AS_ProneIdle`/`AS_ProneToKneel`/`AS_RifleIdle`을 추가**해,
5절에서 지적한 "아군 애셋 빌려쓰기"가 해소됐다. `AS_Enemy/`는 35개가 됐다.

`ABP_Enemy_kadex2` 6곳 교체:

| 노드 | 이전(아군) | 이후 |
|---|---|---|
| `SequencePlayer_4` | `AS_RifleIdle` | `AS_Enemy/AS_RifleIdle` |
| `ProneIdle` 상태 | `AS_ProneIdle` | `AS_Enemy/AS_ProneIdle` |
| `ToProne`/`FromProne` | `AS_ProneToKneel` | `AS_Enemy/AS_ProneToKneel` |
| `SequenceEvaluator_0`/`_2`(죽은 경로) | `AS_Stopsign`/`AS_Inspecting` | `AS_Enemy/AS_RifleIdle` |

`BS_enemy_kadex` 속도 0 줄 9칸도 `AS_Enemy_Rifle_Aiming_Idle` → `AS_Enemy/AS_RifleIdle`로 교체.
**두 애셋은 서로 다른 모션이다**(3.1s/94key vs 7.7s/232key) — 육안 확인 대상.

적군 ABP 의존성에서 아군 애니메이션이 0개가 됐다(남은 `Rifle_Aiming_Idle_Skeleton`은
`Enemy_Skeleton`의 호환 목록에서 오는 간접 참조).

## 9. 남은 작업

**PIE 육안 검증(최우선):**

1. **총 위치** — `Rifle_Socket` 트랜스폼이 추정치다. 어긋나면 스켈레톤 에디터에서 조정.
2. **척추 매핑** — `Spine`→`spine_01`, `Spine1`→`spine_03`, `Spine2`→`spine_04`가 제안값이다.
   기울임·조준각이 어색하면 한 칸씩 올리거나 내린다(3절).
3. **이동 속도** — `200/600/200/400` 기준값 재검증. 발 미끄러짐이 보이면 조정.
4. **적군 서서 정지 자세** — 8절의 모션 교체 결과 확인.

**정리:**

- 구 `BS_ally_kadex` 3종 + `ABP_Ally_kadex2` + 루트의 껍데기 리다이렉터 22개 삭제
  → **에디터 로드 에러 소멸**. 적군이 더 이상 아군 애셋을 안 쓰므로 이제 안전하다.
- 미참조 폴더 `New_Animation_k_army/`(30개)와 `New_k_army_Animation/Run_Left` 정리.
- `LayeredBoneBlend_6` 깊이 -1 결함 처리 방침 결정(6.2 참고).
- 발소리 노티파이 — `KS_` 세트는 1/29뿐이다(1절). 필요하면 다시 찍어야 한다.
