# 아군/적군 애니메이션 분리 — 적군 블렌드스페이스 신설, 아군은 대기

2026-09-10 / 적군 완료·아군 대기 / 디자이너가 적군 스켈레톤을 수정하고 애니메이션을 `AS_Enemy/`로 분리(CL 452·454·455)하면서 공유 블렌드스페이스 참조가 끊겨 에디터 로드 에러 발생. 적군용 블렌드스페이스 2종을 신설·배선해 적군은 복구 완료, 아군은 새 시퀀스 입고 대기.

선행 문서: `2026-09-01_animation_asset_inventory.md`(애셋 현황), `enemy_locomotion_animation_pipeline.md`(gait/AnimGraph 구조).

---

## 1. 무슨 일이 있었나

디자이너 작업 3건이다.

| CL | 날짜 | 내용 |
|---|---|---|
| 452 | 09/09 | 적군 스켈레톤(`Enemy.uasset`) 수정 + 적군 애니메이션 30개를 `AS_Enemy/`에 `AS_Enemy_*` 이름으로 생성. 새 바디 메시 3종(`Ch15_body` 등) |
| 454 | 09/10 | `AS_Enemy_` 접두를 떼어 `AS_Enemy/` 안의 원래 이름으로 정리. 총 메시 교체·소켓 위치 수정 포함. 113파일(add 54/delete 55/edit 4) |
| 455 | 09/10 | 적군 재장전 모션(`AS_Enemy/AS_Reloading`), `ARP_Ally`(리타겟 프로파일) |

**의도는 아군/적군 애니메이션의 실제 분리**다. 적군 스켈레톤 자체가 바뀌었으니 공유가 불가능해진 것.

## 2. 왜 깨졌나

접두를 떼는 이름 변경 과정에서 **리다이렉터가 루트에 남고 그 대상(`AS_Enemy_*`)이 삭제**되어 참조 사슬이 끊겼다.

```
Soldiers/AnimationSequences/AS_Aim_Walk_Bwd1  (1,476 bytes) = ObjectRedirector
   → AS_Enemy/AS_Enemy_Aim_Walk_Bwd   ← 삭제됨
```

루트 파일 크기에 단절이 뚜렷하다 — **22개가 1.4~1.6KB(껍데기)**, 실물은 108KB 이상.

그 결과:

```
Error: BlendSpace BS_ally_kadex has a sample with no/invalid animation.
Error: BlendSpace BS_ally_kadex_Crouch has a sample with no/invalid animation.
```

- `BS_ally_kadex` — 방향 샘플 27칸 중 **13개 유실**
- `BS_ally_kadex_Crouch` — 걷기·뛰기 **12개 전부 유실**
- `BS_ally_kadex_Lowered` — 무사(참조가 `Tutorial/` 폴더라 무관). 단 이 블렌드스페이스는 원래 아무도 안 씀

> **개수만 보면 줄지 않았다**(106 → 136). 새로 생긴 55개 중 22개가 껍데기라 그렇다. 실물을
> 스켈레톤별로 세야 실상이 보인다 — 아군 44 / 적군 40 / `soldier_T_Skeleton` 30 / 껍데기 22.

## 3. 아군 애셋은 "옮겨간" 것이 아니다

이름만 보면 이동처럼 보이지만, **아군 스켈레톤(`Rifle_Aiming_Idle_Skeleton`) 실물이 프로젝트 어디에도 그 개수만큼 없다**(전수 조사). 아군 로코모션에 남은 실물은 이것뿐이다.

- 서서 걷기 8방향 중 2개 / 서서 뛰기 8방향 중 1개
- 숙여 걷기 8방향 중 2개 / 숙여 뛰기 4방향 중 1개

**디자이너 확인 결과: 아군은 작업 진행 중.** `soldier_T_Skeleton`이 새 아군 스켈레톤이며, 아군 시퀀스는 이후에 입고 예정이다(CL 455의 `ARP_Ally`가 그 맥락). 따라서 **아군 쪽은 복구하지 않고 대기**한다.

## 4. 적군 복구 — 완료

적군 애셋 자체는 정상이었으나(`AS_Enemy/` 30개 전부 실물, `Enemy_Skeleton`), **아무도 쓰지 않는 상태**였다. 적군 ABP가 여전히 `BS_ally_*`(아군 스켈레톤 전용)를 물고 있었고 그게 깨져 있었기 때문. 블렌드스페이스는 스켈레톤 하나에 묶이므로 적군 애셋을 아군 블렌드스페이스에 넣을 수 없다.

### 4.1 적군 블렌드스페이스 2종 신설

사용자가 빈 애셋을 생성(이 프로젝트는 새 애셋을 MCP로 만들지 않음), 축·샘플 설정은 MCP로 처리.

- 축: `Speed 0~600 / grid 4`, `Direction -180~180 / grid 8`, 둘 다 스냅 켜짐 (아군과 동일)
- **`BS_enemy_kadex`** — 27샘플. 정지 9칸 `AS_Enemy_Rifle_Aiming_Idle`, 걷기(300) 8방향 `AS_Enemy/AS_Aim_Walk_*`, 뛰기(600) 8방향 `AS_Enemy/AS_Jog_*` + `Run_Left`
- **`BS_enemy_kadex_Crouch`** — 22샘플. 걷기(300) 8방향, 뛰기(600) 4방향

판단이 필요했던 두 가지:

- **숙인 정지(속도 0) 9칸은 아군 `AS_Crouching`을 그대로 사용.** 적군 전용이 없는데
  `Enemy_Skeleton`의 `CompatibleSkeletons`에 아군 스켈레톤이 등록돼 있어 재생된다. 적군 전용이
  들어오면 그 자리만 교체하면 됨.
- **숙여 뛰기 전진 샘플에 `rateScale 1.3`을 아군과 동일하게 적용.** 같은 원본 모션의 리타겟이라
  같은 보정이 필요할 것으로 판단. 안 맞으면 이 값만 조정.

### 4.2 배선

- `ABP_Enemy_kadex2`: `AimLocomotion`·`LoweredLocomotion` → `BS_enemy_kadex`, `Knee` → `BS_enemy_kadex_Crouch`
- `BP_Enemy_Base` 슬롯: `FireRecoil` → `AS_Enemy/Firing_Rifle_Anim_2`, `ReloadSlot` → `AS_Enemy/AS_Reloading`

**PIE 확인 완료 — 적군 정상 동작.**

## 5. 남은 작업

> **2026-09-11 갱신** — 아군 1차분 입고됨(CL 457). `soldier_T_Skeleton`용으로
> `Soldiers/AnimationSequences/New_Animation_k_army/`(`AS_` 30개, 09/10 08:53)와
> `New_k_army_Animation/`(`KS_` 29개, 09/10 15:51) **두 폴더에 중복** 입고됐다(후자가 재익스포트본,
> `Crouch_Aim_Run_Right_2`만 빠짐). **이동 사이클은 사실상 완비, 정지 자세는 5종 전부 없음**
> (서서 조준/숙여/총 내림/엎드림/엎드리기→무릎). 정지 자세가 없으면 교체 즉시 T포즈가 되므로
> 그것부터 받아야 한다. 요청 메일: `2026-09-11_ally_pose_request_email.html`.
>
> 또한 **드롭인 교체가 불가능하다** — 새 스켈레톤은 UE5 마네킹 본 이름(`pelvis`/`spine_01`/
> `upperarm_l`/`thigh_l`…)이라 기존 Mixamo 이름(`Hips`/`Spine`/`LeftArm`/`LeftUpLeg`…)과
> **겹치는 본이 하나도 없다.** 교체 시 `ABP_Ally_kadex2`의 `Transform (Modify) Bone` 8개,
> `LayeredBoneBlend`의 `Spine` 브랜치 필터, 왼손 `TwoBoneIK`를 전부 재지정해야 하고,
> 소총 부착 소켓도 새로 만들어야 한다(새 메시엔 `hand_rSocket`뿐). 아군 C++ 쪽은 본 이름을
> 직접 참조하지 않아 수정 불필요. 참고로 새 스켈레톤엔 `root` 본이 있어, 허리가 루트라서 생기던
> 발 미끄러짐 문제는 구조적으로 해소된다.

- **아군 시퀀스 입고 대기.** 들어오면 `BS_ally_kadex`/`_Crouch`의 빈 샘플을 채우고, 루트의
  껍데기 리다이렉터 22개를 정리한다. 그전까지 에디터 로드 에러는 계속 뜬다.
- **`BP_Enemy_Base` 저장 미완료** — 슬롯 교체가 에디터 메모리에만 있고 디스크 반영이 안 됐다
  (P4 체크아웃 필요). 그동안 적군은 아군 재장전·반동 애셋을 재생한다(호환 스켈레톤이라 동작은 함).
- 적군 전용이 아직 없는 것: **숙인 정지**, **엎드리기 계열**(`AS_ProneIdle`/`AS_ProneToKneel`/
  `AS_StandtoKnee`/`AS_Knee`). 현재는 호환 스켈레톤으로 아군 것을 재생 중이라 동작에 문제는 없다.

## 6. 재발 방지 메모

- **애셋 이름 변경/이동은 리다이렉터를 남긴다. 그 대상까지 지우면 참조가 끊긴다.** 이번 사고의
  직접 원인. 이동 후에는 "Fix Up Redirectors"를 돌리고 리다이렉터를 정리해야 한다.
- **파일 개수로 손실을 판단하면 안 된다.** 껍데기 리다이렉터가 개수를 채워버린다. `.uasset`
  크기(1~2KB면 리다이렉터) 또는 스켈레톤 참조로 실물 여부를 판별할 것.
- 블렌드스페이스가 비었을 때 원인 추적은 `Saved/Logs`의 `LoadErrors`가 가장 빠르다 —
  어느 블렌드스페이스가 어느 패키지를 못 찾는지 그대로 찍힌다.
