# (세션 프롬프트) 병사 피격 · 사망 · 체력/데미지 — 구조 파악 및 방식 추천

2026-09-14 / ~~초안~~ **소진(2026-09-15)** / 다른 세션에 붙여넣을 프롬프트. 조사 → 보고 → 추천까지. **구현은 하지 않는다.**

> **후일담 (2026-09-15)**: 이 프롬프트의 산출물 `2026-09-14_hit_death_health_recommendation.md` 는 구현이 끝나 **`ai/` 로 올라갔다**(아래 본문의 `ai/drafts/...` 경로는 당시 지시문 그대로 둔다). 구현 기록은 `ai/2026-09-15_health_hit_death_implementation.md`.

---

```
프로젝트 CLAUDE.md(C:\working\works\kadex\titan_example\CLAUDE.md)와
C:\working\insung_grapic\titan\soldier_ai_lab\CLAUDE.md 를 먼저 읽어. 그 다음
soldier_ai_lab\IMPLEMENTED.md 0절·3절·5절·6절, CURRENT_STATE.md 머리글,
assets\2026-09-14_design_team_animation_handoff.md 를 읽고 시작해.

## 배경
SoldierLab(GASP 모션매칭 기반 병사)이 2026-09-14 에 titan_example 에 편입됐다
(migration/2026-09-14_titan_example_migration.md). 새 병사(BP_SoldierCharacter, C++ 모듈
Source/SoldierLab/)는 AI 로 서로 쏘지만 **피격 반응 · 사망 · 체력 · 데미지가 하나도 없다**
(IMPLEMENTED.md 6절 첫 줄, OPEN_ITEMS [W18]). 병사가 안 죽으니 교전이 끝나지 않는다.

지금 두 갈래 후보가 있다:
  (A) GASP/Lyra 에서 반입된 클립 — SoldierLab/Animations/Actions/ 의 MM_HitReact 13장
      (Front/Back/Left/Right × Lgt/Med/Hvy) + MM_Death 6장 + 각각의 AM_MM_* 몽타주.
      반입만 됐고 배선 0. 래그돌은 GASP 캐릭터에 이미 뭔가 있다고 기록돼 있다(확인 필요).
  (B) titan_example 의 **기존 적군**(구 시스템)이 쓰던 물리 기반 피격/사망:
      Source/titan_example/Soldiers/EnemyCombatComponent.h/.cpp 의
      TriggerHitReactionPhysics / TickHitReactionSpring(순수 계산 감쇠조화진동자 —
      물리 엔진 안 씀, ABP 의 Transform(Modify) Bone 에 오프셋을 얹는다),
      BP_Enemy_Base 의 Event PointDamage/AnyDamage, 사망 시 UPhysicalAnimationComponent
      액티브 랙돌 + DeathImpulse. 문서: C:\working\insung_grapic\titan\ai_combat\
      enemy_hit_reaction_physics_system.md, enemy_ai_combat_system_status.md,
      2026-09-01_enemy_spin_on_hit_investigation.md. 데미지는 표준 엔진 경로
      (ApplyPointDamage → TakeDamage)로 들어온다.
  체력/데미지 시스템도 같이 정해야 한다. 새 병사의 투사체는
  Source/SoldierLab/Weapons/SoldierProjectile.cpp (titan 의 ARCWSProjectile 이식본,
  weapons/2026-09-12_projectile_port.md) 이고, 진영은 USoldierIdentityComponent::Faction
  (Source/SoldierLab/AI/), 제압·명중 판정은 USoldierEngagementComponent 쪽에 있다.
  titan 본체의 UGV/RCWS 도 병사를 쏜다(ARCWSProjectile → ApplyPointDamage).

## 해 줄 것 — 읽기 전용 조사. 파일을 고치지 말고 보고해
1. 구조 파악 (전부 [A]로, 파일:줄 인용):
   a. 새 병사가 데미지를 **받는** 경로가 지금 있나 — BP_SoldierCharacter / C++ 에 TakeDamage,
      OnTakeAnyDamage, OnTakePointDamage 바인딩이 있는지. SoldierProjectile 이 명중 시
      무엇을 호출하는지(ApplyPointDamage? 자체 이벤트?). 제압(Suppression)은 어디서
      "맞았다/스쳤다"를 판정하는지 — 그 판정을 데미지와 공유할 수 있는지.
   b. GASP 캐릭터(SandboxCharacter_CMC / SoldierCharacter_ABP)에 래그돌·피직스 에셋·
      PhysicalAnimation 관련 배선이 있는지. SK_UEFN_Mannequin 의 PA_UEFN_Mannequin,
      soldier_T_PhysicsAsset, enemy_T_PhysicsAsset 바디 구성(에디터에서 봐야 함 —
      MCP 로 바디 목록은 못 읽는다. 못 보면 [C] 로 남겨).
   c. HitReact 13 · Death 6 클립/몽타주의 실체 — 슬롯 이름, 슬롯 그룹(P24: 슬롯 그룹이
      갈리면 몽타주가 통째로 무효), 루트모션 유무, 애디티브인지, 길이. 우리 ABP 의 슬롯
      노드(FullBodyAdditivePreAim / UpperBody / UpperBodyAdditive / DefaultSlot)와 맞는지.
      MM 이 도는 채로 전신 몽타주가 재생되면 무슨 일이 나는지(DefaultSlot 은 OffsetRootBone
      앞에 있다 — IMPLEMENTED 2.4 체인).
   d. titan 기존 적군의 (B) 시스템이 새 병사에 **이식 가능한 형태**인지 — EnemyCombatComponent
      가 얼마나 BP_Enemy_Base/ABP_Enemy_kadex2 에 결합돼 있는지, 스프링 부분만 떼어낼 수
      있는지, 부호값(hit_reaction_sign_convention 메모리: Pitch=-1/Yaw=0.3/Roll=1.0 는
      **Mesh 트랜스폼 기준**)이 마네킹 리그에서도 성립하는지.
   e. 새 병사의 절차 층과의 충돌 — 피격 오프셋을 어디에 넣어야 린(ModifyBone spine_01..05),
      stance(ModifyBone pelvis), 왼손 IK(TwoBoneIK_0), 총구 보정 적분기(AimCorrection —
      피격으로 총구가 흔들리면 그걸 "오차"로 학습할 위험, P40/[C-78] 과 같은 기구)와
      안 싸우는지. 사망 시 AI 컴포넌트들(인지/교전/엄폐)·MM·Tick 을 어떻게 멈춰야 하는지.
   f. 리플리케이션 — titan_example 은 리슨서버 멀티(P5: L0~L3 Tick 에 HasAuthority 게이트).
      체력/사망은 서버 권위여야 한다. 지금 새 병사 코드에 권위 게이트가 어디까지 있는지.

2. 방식 비교 — 표로. 각 항목에 [A]/[B]/[C] 붙여:
   후보:  (A) 클립 몽타주(HitReact/Death) 만
          (B) titan 스프링 피격 + 액티브 랙돌 사망 이식
          (C) 혼합 — 피격은 스프링(방향·부위·세기가 연속으로 나옴), 사망은 Death 클립 →
              끝에서 래그돌 전환, 또는 그 반대
          (D) 순수 래그돌 사망 + 피격은 애디티브 몽타주
   축:  ① 이동 중 피격의 자연스러움(MM 위에 얹힐 때 발 미끄러짐/포즈 팝)
        ② 45명 성능(몽타주 vs 매 프레임 스프링 vs 물리 바디)
        ③ 디자인팀이 시퀀스로 고칠 수 있는 범위(핸드오프 문서 4절 — "시퀀스로 못 고치는 층"
           이 늘어나는가)
        ④ 방향·부위·세기 정보의 활용(13장 이산 vs 연속)
        ⑤ 구현 비용과 되돌리기 쉬움
        ⑥ 멀티플레이(몽타주 RPC vs 물리 동기화)
        ⑦ 아군/적군 코드 한 벌(P4) 유지

3. 체력/데미지 시스템 제안 — 최소 사양:
   컴포넌트 하나(USoldierHealthComponent 같은)에 MaxHealth/Health/OnDamaged/OnDeath,
   데미지 입력은 표준 TakeDamage 경로(titan 의 UGV/RCWS 도 그 경로로 쏜다), 부위 배율
   (본 이름 → 배율, 데이터로 — P6), 진영 판정은 USoldierIdentityComponent 재사용,
   서버 권위. GAS 는 쓰지 않는다(migration 문서 3.4: LyraGame/GAS 를 들이지 않기로 했다).
   기존 titan 의 BP_Enemy_Base 체력 로직이 있으면 그것과 대조해 무엇을 가져올지 말해.

4. 최종 보고 형식:
   - 추천 1개 + 그 이유 3줄, 차선 1개
   - 추천안의 구현 순서(단계별, 각 단계의 검증 방법 — P7 "디버그 표시 먼저", P10 "계측부터")
   - 새로 생기는 미해결 항목은 ID 를 붙여 OPEN_ITEMS.md 형식으로(등록은 하지 말고 목록만)
   - 문서는 soldier_ai_lab/ai/drafts/2026-09-14_hit_death_health_recommendation.md 로 저장
     (헤더 규칙: 날짜 / 상태 / 한줄요약. drafts 에 두는 이유: 아직 미구현)

## 지킬 것
- 코드·에셋 수정 금지. MCP 는 읽기만. PIE 를 MCP 로 켜지 말 것(P79).
- 추정과 사실을 섞지 말 것(soldier_ai_lab CLAUDE.md 3.2). 못 본 것은 [C] 로 남긴다.
- "이름이 그럴듯해서"로 판단하지 말 것(P41) — 클립·몽타주는 실제로 열어 슬롯/애디티브를 확인.
- titan 기존 적군 코드는 참고용이지 정답이 아니다 — memo.md 는 근거로 인용 금지.
```
