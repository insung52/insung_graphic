# 아군 앉기 — 무릎이 안 굽고 발이 꼬이던 것 (제거된 ik_* 본 · 크라우치 idle 의 IK 본 원점)

2026-09-23 / 완료 (사용자 확인) / 아군만 V/B 로 자세를 낮출 때 **무릎이 안 굽고 몸이 위아래로 떠다니던** 문제 + 이어서 **앉은 자세에서 발이 꼬이던** 문제. 원인 둘 다 애님 로직이 아니라 **에셋**이었다 — ① 디자이너가 재임포트한 메시(#499) **LOD0 의 "Bones to Remove" 에 `ik_*` 본이 들어가 있던 것** ② `ALLY_MM_Rifle_Crouch_Idle` 만 `ik_foot_*` 가 원점에 박혀 있던 것.

★ **함정**: FBX/블렌더 비교로는 "본 동일" 로 **오진**했다. 정답은 **스켈레톤 트리의 아이콘**이었다 — 속이 빈 동그라미 = `NonRequiredBone`.

관련: `animation/prototypes/2026-09-13_ally_mesh_on_mannequin_skeleton.md`(아군 메시를 마네킹 스켈레톤에 얹은 기록) · `animation/prototypes/2026-09-17_ally_enemy_anim_set_split.md`(아군/적군 애님 2벌 — 이 건은 **ALLY 쪽에서만** 났다) · `animation/prototypes/2026-09-04_c34_clip_curve_mapping.md` 5.2절 + `animation/prototypes/2026-09-08_root_facing_56deg.md`([C-46] — `AM_Copy_IKFootRoot` 가 파이프라인에 들어온 경위) · `animation/prototypes/2026-09-12_continuous_stance_axis.md`(stance 축 — 무릎이 굽어야 하는 그 축).

---

## 1. 증상 ①과 오진

**증상** [A · 사용자 관측]: **아군만** V/B 로 자세를 낮추면 무릎이 굽지 않고 **캐릭터 전체가 위아래로 떠다닌다.** 적군은 정상. 골반은 내려가는데 다리가 따라오지 않으니 발이 땅을 뚫거나 공중에 뜬다.

**오진 경로** [A · 기록해 두는 이유]: 아군/적군 메시가 갈렸으니 스켈레톤 차이를 의심하고 **FBX 와 블렌더 씬을 비교**했다 → **"본 목록 동일"** 이라는 결론이 나왔다. 실제로 본 계층은 정말 동일했다. **그래서 한참 엉뚱한 곳(ABP·stance 축·PelvisDrop)을 봤다.**

**정답의 단서는 스켈레톤 트리의 아이콘이었다** [A]:

```
●  실선 채운 동그라미  = 일반 본 (스킨에 쓰인다)
○  속이 빈 동그라미    = NonRequiredBone  ← 이 LOD 에서 제거된 본
```

`ik_foot_root` / `ik_foot_l` / `ik_foot_r` / `ik_hand_*` 가 **속이 빈 동그라미**였다.

---

## 2. 원인 ① — LOD0 의 "Bones to Remove" [A · 확정]

디자이너가 재임포트한 아군 메시(**#499**)의 **LOD0 설정 `Bones to Remove`(LOD Reduction Settings)에 `ik_*` 본이 들어가 있었다.**

- 본 계층에는 존재하지만 **LOD0 에서 제거(`NonRequiredBone`)** 되므로 **런타임 포즈에 없다.**
- → `ik_foot_*` 를 읽는 모든 것(IK 노드·`AM_Copy_IKFootRoot` 로 채운 데이터·발 접지)이 **조용히 무효**가 된다. 컴파일 에러도 경고도 없다.
- → 다리 IK 가 죽으니 골반만 내려가고 **무릎이 안 굽는다**, 접지가 없으니 **몸이 떠다닌다**.
- **적군 메시는 같은 재임포트를 안 거쳤으므로** 멀쩡했다 — "아군만" 이 여기서 나온다.

**해결**: 메시 LOD0 의 `Bones to Remove` 에서 `ik_*` 를 빼고 재빌드.

> **함정으로 기록**: FBX/블렌더 대조는 **본의 존재**만 말해 준다. **LOD 가 그 본을 쓰는지는 언리얼 쪽 설정**이고, 그건 **스켈레톤 트리 아이콘**에만 드러난다. 다음에 "본은 같은데 IK 만 안 먹는다" 가 나오면 **FBX 를 열기 전에 트리 아이콘부터 볼 것.**

---

## 3. 증상 ② / 원인 ② — 앉은 자세에서 발이 꼬인다 [A · 확정]

①을 고치자 무릎은 굽었는데, 이번엔 **앉은 자세에서만** 발이 서로 꼬였다.

**원인**: **`ALLY_MM_Rifle_Crouch_Idle` 한 클립만** `ik_foot_root` / `ik_foot_l` / `ik_foot_r` 가 **원점(0,0,0)에 박혀** 있었다. 다른 클립들은 `AM_Copy_IKFootRoot` 를 거쳐 `ik_foot_*` 가 `foot_*` 를 따라가게 채워져 있는데, 이 한 장이 그 단계를 안 탄 것이다([C-46] 으로 파이프라인에 넣은 단계 — `animation/prototypes/2026-09-04_c34_clip_curve_mapping.md` 5.2절).

IK 타깃이 캐릭터 원점에 있으니 발이 그쪽으로 끌려가 꼬인다.

**해결**: 그 클립에 **Animation Data Modifier `AM_Copy_IKFootRoot`** 적용 —

```
attach   → ik_foot_root
foot_l   → ik_foot_l
foot_r   → ik_foot_r
```

---

## 4. 배운 것

1. **"본이 같다" 와 "그 본이 쓰인다" 는 다른 질문이다.** LOD 의 `Bones to Remove` 는 계층을 안 바꾸고 **필요 여부만** 바꾼다 — DCC 쪽 비교로는 절대 안 잡힌다.
2. **디자이너 재임포트는 LOD 설정을 같이 들고 온다.** 메시가 새로 들어온 뒤 IK/접지가 이상하면 LOD0 설정을 먼저 본다.
3. **애님 데이터 모디파이어는 클립 단위다.** 한 장만 빠져도 그 상태(여기서는 앉기)에서만 증상이 난다 — "특정 자세에서만" 이면 **그 자세의 클립 한 장**을 의심한다.
