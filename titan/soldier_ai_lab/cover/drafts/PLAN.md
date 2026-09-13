# 엄폐(Cover) 시스템 — 설계·구현 초안

2026-09-09 / 초안(리뷰 대기) / 엄폐 지점의 생성·평가·사용. 설계 문서 10절을 구현 수준으로 내리고, 45명@60fps 예산 안에 들어가는 EQS 파이프라인과 필요한 애니메이션 목록을 확정한다.

> **담당 범위**: 엄폐 지점과 그 사용. **개별 병사의 지각·위협평가(7절)와 분대 협동(9절)은 다른 담당자**이며
> 이 문서는 그 두 곳과의 **계약(입력 인터페이스)만** 정의한다.
>
> **신뢰도**: `[A]` 확정(엔진 소스·에셋 실측 확인) / `[B]` 잠정(합리적이나 실측 전) / `[C]` 미측정
>
> **근거 문서**: `design/2026-09-01_architecture.md` **10절**(3층 구조·질의 설계) · 5.4 · 5.5.5 · 5.5.6 ·
> 12.2~12.3(예산·LOD) · 3.6(프로젝트 설정) · 3.7(모듈) · 15.3 **D2** ·
> `ai/2026-09-02_upper_layer_plan.md` **14.3절**(중첩 상태 = 예약 누수 구조적 해결) · 4.2 · 6.2 ·
> `CURRENT_STATE.md` 4절 ⑤(P0-3 방침)

---

## 0. 요약 — 이 문서가 정한 것 8가지

| # | 결정 | 신뢰도 | 근거 |
|---|---|---|---|
| 1 | 슬롯 생성은 **오프라인 베이크 + 동적 런타임 등록** 하이브리드. 런타임 전면 생성은 채택하지 않는다 | [A] 설계 10.2 확정 | 3절 |
| 2 | 베이크 산출물은 **레벨 액터가 아니라 `UCoverBakeData` DataAsset 1개**. 수동 저작 슬롯은 레벨 액터로 분리 | [B] | **4절 — D2의 답** |
| 3 | **커스텀 EQS Generator를 만들지 않는다.** 엔진 `UEnvQueryGenerator_SmartObjects`가 이미 있다 | **[A] 엔진 소스 확인** | 6.1절 |
| 4 | **"어느 방향을 막는가"는 베이크가 답한다.** 런타임은 각도 계산만 하고, **트레이스는 "쏠 수 있는가"에만 쓴다** | [B] | 5절 |
| 5 | 신규 C++ 클래스는 **EQS 테스트 3개 + 컨텍스트 2개 + 서브시스템 1개 + 액터/컴포넌트 2개**. 나머지는 엔진 재사용 | [B] | 6절 |
| 6 | 상태 흐름은 GASP 중첩 패턴 그대로. **예약은 `ClaimSlot` 상태가 보유**하고 이동·진입·점유는 전부 그 자식 | [A] upper_layer 14.3 | 7절 |
| 7 | **엄폐 퇴출 전용 애니메이션을 만들지 않는다** — 퇴출은 다음 Intent의 진입부가 흡수한다 | [B] | 7.4절 |
| 8 | 질의는 **이벤트 구동 + 전역 동시 상한(토큰)**. 주기 재질의를 두지 않는다 | [B] | 9절 |
| 9 | **"엄폐가 있는가"는 EQS 없이 답한다** — 그래야 L2가 `TakeCover`를 고르기 전에 알 수 있다 | [B] | 6.4절 |
| 10 | 인지의 `Exposure`와 우리 `Exposure`는 **같은 숫자가 아니다.** 곱셈 인자로 나눈다 | **[A] 타 초안 실측** | **12.2절 — 합의 안 하면 엄폐가 두 번 곱해진다** |

**사용자 결정이 필요한 것은 `OPEN_QUESTIONS.md`에 있다.**

---

## 1. 이 시스템이 답해야 하는 질문

설계 10.1절의 3층을 그대로 쓰되, 각 층이 **어떤 질문에 답하는가**로 다시 적는다.

```
[층 1] 생성   "이 레벨 어디에 엄폐가 가능한 지점이 있고, 각 지점은 어느 방향을 막는가"
              → 오프라인. 방향·높이·사격점이 이때 다 정해진다.
[층 2] 평가   "지금 이 위협 배치에서, 나에게 가장 좋은 지점은 어디인가"
              → 런타임 EQS. 각도 계산이 주(主), 트레이스는 최소.
[층 3] 점유   "그 지점을 내가 쓴다고 선언하고, 들어가서, 쏘고, 나온다"
              → StateTree + SmartObject 예약. GASP 승계.
```

**층 1이 무거울수록 층 2가 가벼워진다.** 45명 예산의 열쇠가 여기 있다 — 9절.

---

## 2. 용어와 좌표 규약 (먼저 못박는다)

이 규약이 틀리면 전부가 조용히 틀린다. `CLAUDE.md` P10(계측 먼저) · P16(대조군) 정신을 따라
**베이크 툴에 규약 검증 시각화를 먼저 넣는다**(9.6절).

| 용어 | 정의 | 비고 |
|---|---|---|
| **슬롯 트랜스폼** | SmartObject 슬롯의 월드 트랜스폼. 위치는 **병사의 캡슐 중심이 설 지점(발밑 아님)** | `USmartObjectSubsystem::GetSlotTransform(SlotHandle)` [A] |
| **`CoverFacing`** | 슬롯 트랜스폼의 **+X축**. **"이 슬롯이 막아주는 방향의 중심"** = 엄폐물이 있는 쪽 | ★ 병사는 이 방향을 바라보고 서며, 그 앞에 엄폐물이 있다 |
| **`ProtectionHalfAngle`** | `CoverFacing` 기준 ±각도. 이 부채꼴 안의 위협만 막아준다 | Inner(완전 차폐) / Outer(0으로 감쇠) 두 값 |
| **`CoverHeight`** | `Low`(≈100cm, 웅크려야 가려짐) / `High`(≈180cm, 서서 가려짐) | 설계 10.2절의 40/100/180 3높이 중 40(Prone)은 **범위 밖 권고** — 8.2절 A7 / `OPEN_QUESTIONS.md` **Q36** |
| **사격점(Firing Point)** | 그 슬롯에서 사격 가능한 **총구 위치**의 슬롯 로컬 오프셋 | LeanL / LeanR / OverTop / Blind |
| **Protection** | `[0,1]`. 특정 위협·특정 자세에 대해 이 슬롯이 막아주는 정도 | 5.2절 공식 |
| **Exposure** | `1 - Σ(가중 Protection)`. **7.5절 인지의 `Exposure`와 같은 정의여야 한다** | 5.3절 — 인지 담당자와의 계약 |

> ⚠ **`UEnvQueryItemType_SmartObject`는 `UEnvQueryItemType_VectorBase` 파생이라 위치만 준다.
> 아이템에서 슬롯의 *회전*을 못 읽는다** [A, 엔진 소스]. 그래서 `CoverFacing`은 EQS 테스트 안에서
> 서브시스템 캐시로 조회한다(6.3절). 이걸 모르면 "왜 GetItemRotation이 0인가"에서 하루를 잃는다.

---

## 3. 층 1 — 엄폐 지점을 어떻게 만들 것인가

### 3.1 세 방식 비교

| 방식 | 장점 | 단점 | 판정 |
|---|---|---|---|
| **A. 레벨 배치 SmartObject (수동 저작)** | 저작 통제 100%. 연출 지점 보장. **구현 비용 0** — 지금 당장 된다. 기존 `FiringPose.Marker`/`CoverPose.Marker` 이관 경로(설계 10.2 하위호환) | **임의 레벨 대응 불가** → P1 검수 기준 "마커 하나 없는 임의의 레벨"을 원천적으로 못 만족. 저작 비용이 레벨 크기에 비례 | **보조로 유지.** 전면 채택 안 함 |
| **B. 런타임 생성** (매 질의마다 주변을 트레이스해 지점 생성) | 임의 레벨·동적 지형 완전 대응. 저작 0 | 45명 × 반경 15m × 방향 16 × 높이 3 = **질의당 수백 트레이스.** 12.2절 EQS 예산 0.7ms를 한 명이 다 먹는다. 결과가 프레임마다 달라져 **디버깅이 불가능**(P7 "값이 안 보이면 튜닝 불가"의 최악형) | **채택 안 함** |
| **C. 오프라인 베이크 + 동적 런타임 등록** | 트레이스 비용을 **에디터로 밀어낸다.** "NavMesh 경계 = 장애물이 있는 곳"이라는 무료 정보를 쓴다. 동적 엄폐물(UGV·잔해)은 컴포넌트가 자기 슬롯을 등록/해제 | 베이크 툴 제작 비용. 재베이크 정책 필요(→ D2). 지오메트리 변경 추적 필요 | ✅ **채택 (설계 10.2절 확정)** |

**권고: C. 단 단계를 나눈다.**

| 단계 | 하는 일 | 이유 |
|---|---|---|
| **P0-3 (지금)** | **A로만 간다.** 테스트 레벨에 `ASoldierCoverPoint` 5~8개를 손으로 놓고 EQS→Claim→진입 루프를 검증 | P0-3의 판정 기준은 *"병사 3명이 서로 다른 슬롯을 예약해서 들어가는가"*(설계 14절)다. **슬롯이 어떻게 생겼는지는 이 판정과 무관하다.** 베이크 툴을 먼저 만들면 두 미지수를 동시에 푸는 것 |
| **P1** | 베이크 툴(C) 정식화 + A를 "수동 저작 슬롯"으로 흡수 | 설계 P1 시스템 트랙 "엄폐 슬롯 생성 툴 정식화" |
| **P1 후반** | `UCoverProviderComponent`(동적 엄폐물) | UGV는 SoldierLab에 아직 없다(D8) |

### 3.2 베이크 알고리즘 (P1) [B]

```
1) 대상 영역 = 레벨의 ASoldierCoverVolume(없으면 NavMesh 경계 전체)
2) NavMesh 경계 에지 추출
      ARecastNavMesh::GetDebugGeometry(FRecastDebugGeometry&, TileIndex)
      → FRecastDebugGeometry::NavMeshEdges                        ← [C-63] 접근 가능성 확인 필요
3) 에지를 75cm 간격으로 샘플 → 후보점 = 에지에서 **안쪽으로** (캡슐반경 42 + 여유 10)cm
4) 각 후보점에서 바깥 방향으로 3높이(180 / 100 / 40cm) 트레이스   ← Cover 채널(설계 3.6절)
      전부 미충돌 → 폐기 (엄폐물이 없다)
5) 방향 스윕: CoverFacing 후보를 ±90°, 15° 간격으로 돌려 각 방향의 차폐 높이를 기록
      → 최대 차폐 방향 = CoverFacing
      → 그 방향에서 좌우로 차폐가 유지되는 범위 = HalfAngleInner / Outer
      → 그 부채꼴에서 **보장되는 최소 차폐 높이** = CoverHeight
6) 사격점 판정 (슬롯 로컬)
      LeanL/R : ±55cm 옆, 높이 130cm 에서 CoverFacing 반구가 보이는가
      OverTop : 엄폐물 상단 +15cm 에서 보이는가  (Low 전용)
      Blind   : 위 둘 중 하나가 기하적으로만 가능하고 시야는 막힌 경우
7) 중복 제거: 같은 벽 위 유사 슬롯 병합 (간격 < 150cm 이고 facing 차 < 20°)
8) UCoverBakeData 에 기록 + 지오메트리 해시 저장
```

> **왜 방향 스윕을 베이크에서 하는가**: 이 한 번의 계산이 런타임 트레이스를 전부 없앤다.
> 후보점 1개당 16방향 × 3높이 = 48 트레이스가 **에디터에서 한 번** 돌고, 런타임은
> `Dot(CoverFacing, ThreatDir)` 한 번으로 답한다. **이것이 이 설계의 경제성의 전부다.**

### 3.3 동적 엄폐물

| 소스 | 처리 |
|---|---|
| UGV·차량 | `UCoverProviderComponent` — 액터에 붙이면 자기 슬롯 정의를 `USmartObjectSubsystem::CreateSmartObject()`로 등록/해제 [A, 엔진 API 확인] |
| 파괴·변형 | 슬롯 무효화 → `SetSlotEnabled(SlotHandle, false)` [A] + 점유 중인 병사에게 `CoverInvalidated` 이벤트 → L2 재평가 |
| 이동하는 엄폐물 | 슬롯 트랜스폼 갱신은 서브시스템이 지원하나(`Transform/Location` 설정 가능 [A]), **이동 중 예약을 유지할지는 [C-64]** |

---

## 4. 베이크 산출물의 저장·버전관리 — **D2의 답** [B]

설계 백로그 D2가 P0-3 시점 항목으로 걸려 있다. 답:

| 질문 | 답 | 근거 |
|---|---|---|
| 레벨 액터인가 별도 DataAsset인가 | **`UCoverBakeData` DataAsset 1개 / 레벨 1개.** `/Game/SoldierLab/Cover/CoverBake_<LevelName>` | 아래 |
| 런타임에 어떻게 실체화하나 | 레벨의 `ASoldierCoverManager`가 BeginPlay에 읽어 `USmartObjectSubsystem::CreateSmartObject()`로 등록 | **컴포넌트 없이 SmartObject 인스턴스를 만드는 API가 존재한다** [A] |
| 재베이크가 수동 저작을 날리지 않는 보장은 | **파일이 다르다.** 수동 슬롯은 레벨의 `ASoldierCoverPoint` 액터, 베이크 슬롯은 DataAsset. 베이크는 DataAsset만 덮어쓴다 | **보장을 정책이 아니라 구조로 만든다** — 정책은 지켜지지 않는다 |
| 지오메트리 변경 시 재생성 정책 | 베이크 시 **지오메트리 해시**(볼륨 내 스태틱 액터의 트랜스폼 + 메시 에셋 GUID 집합)를 DataAsset에 저장. 로드 시 불일치면 **에디터 경고만.** 자동 재베이크는 하지 않는다 | 자동 재베이크는 비결정적 결과를 조용히 밀어넣는다 |
| Perforce | 재베이크 = **에셋 1개 체크아웃**. 레벨을 건드리지 않으므로 다른 사람의 레벨 작업과 충돌하지 않는다 | 레벨에 수천 액터를 스폰하는 방식의 가장 큰 실무 비용이 이것 |

> **왜 레벨 액터가 아닌가 (추가 근거)**: 슬롯 수백~수천 개를 레벨 액터로 두면 World Partition
> 로딩·액터 초기화 비용이 붙고, 레벨 diff가 거대해지며, "수동 저작인지 베이크 산출물인지"를
> 액터 목록만 봐서는 구분할 수 없게 된다.

---

## 5. 엄폐 품질을 어떻게 평가하는가

### 5.1 무엇을 평가하는가 — 3가지가 서로 다른 질문이다

| 질문 | 언제 답하나 | 비용 |
|---|---|---|
| **어느 방향의 사격을 막는가** | 베이크(3.2절 5단계) → `CoverFacing` + `HalfAngle` + `CoverHeight` | 런타임 0 |
| **서서/웅크려 쏠 수 있는가** | 베이크(3.2절 6단계) → 사격점 플래그 + 로컬 오프셋 | 런타임 0 |
| **지금 이 위협 배치에서 유효한가 / 실제로 표적이 보이는가** | 런타임 EQS | 각도 계산 + **트레이스 1~3회** |

### 5.2 Protection 공식 [B]

```
AngularFactor(threat) :
    ToThreat = Normalize(ThreatLoc - SlotLoc)            // 수평 성분만 사용
    Cos      = Dot(CoverFacing, ToThreat)
    Angle    = Acos(Cos)
    = 1                                   , Angle <= HalfAngleInner
    = smoothstep(Outer→Inner 로 1→0 감쇠)  , Inner < Angle < Outer
    = 0                                   , Angle >= HalfAngleOuter

HeightFactor(stance) :
    CoverHeight == High  →  Stand 1.0 / Crouch 1.0
    CoverHeight == Low   →  Stand 0.25 / Crouch 1.0      ← Low에서 서 있으면 상체가 노출된다
                                                            0.25는 [C-65] 튜닝 대상

Protection(threat, stance) = AngularFactor(threat) * HeightFactor(stance)
```

- **수평 성분만 쓰는 이유**: 고저차가 있는 위협(옥상)은 각도만으로 판정하면 틀린다.
  → 고저차 보정은 [C-66]로 남긴다. **지금은 평지 가정임을 명시**하고, 짐 레벨(D3)에
  높은 지점을 넣어 실측할 것.
- **`smoothstep`을 쓰는 이유**: 계단 함수면 위협이 경계를 오갈 때 슬롯 점수가 튀고,
  히스테리시스가 없는 EQS에서 병사가 두 슬롯 사이를 왕복한다.

### 5.3 Exposure — 인지(7절)와 공유하는 숫자

```
Exposure(slot, stance) = 1 - Σ_i ( w_i · Protection(slot, stance, threat_i) )   , clamp[0,1]
        w_i = 위협 i의 (신뢰도 × 위험도) 를 Σw=1 로 정규화
```

> ★ **계약**: 이 정의는 설계 7.5절의 `Exposure`(내가 남에게 얼마나 보이는가)와 **같아야 한다.**
> `ai/2026-09-02_upper_layer_plan.md` 5.4절이 "같은 숫자를 두 시스템이 공유한다 — 이 설계의 경제성"이라고
> 못박은 지점이다. **인지 담당자와 이 공식을 맞추는 것이 통합 시점의 첫 작업이다** → `OPEN_QUESTIONS.md` Q35.

`w_i`(신뢰도·위험도)는 **인지 담당 범위**다. 나는 `TArray<FCoverThreatInfo{Location, Weight}>`를
입력으로 받는 인터페이스만 정의한다(6.2절 컨텍스트).

### 5.4 사격 가능성 — 트레이스가 필요한 유일한 축

```
FiringViability(slot, target) =
    max over 활성 사격점 p :
        ( LOS(p.WorldMuzzleLoc → target) ? 1 : 0 ) × ModeWeight(p.Mode)

ModeWeight :  OverTop 1.0 / LeanL·LeanR 0.9 / Blind 0.3
```

- `Blind`는 맹목사격이라 명중을 기대하지 않는다. 0.3은 "제압사격은 되지만 조준사격은 안 된다"의 표현.
- **트레이스는 사격점→표적 방향으로 쏜다**(표적→사격점이 아니라). 사격점 주변 지오메트리에
  파묻히는 오탐을 줄인다. 엔진 `EnvQueryTest_Trace`도 `bTraceFromItem` 축을 갖고 있다 [A].
- **비용 상한**: 아이템당 트레이스 **최대 1회**가 기본값. 기하적으로 가장 유망한 사격점
  (표적 방향과 사격점 오프셋의 내적이 최대인 것) 하나만 검증한다 → 9.2절.

### 5.5 "완벽한 엄폐"만 찾지 않는다 — 엔진 기능으로 공짜 [A]

설계 10.5절의 요구(상위 N 가중 랜덤)는 **EQS `RunMode`로 이미 제공된다**:

```
EEnvQueryRunMode::RandomBest25Pct   "Pick random item with score 75% .. 100% of max"
EEnvQueryRunMode::RandomBest5Pct    "... 95% .. 100% of max"
```
(`EnvQueryTypes.h:183-192` 실측)

→ **코드 0줄.** 기본은 `RandomBest25Pct`, 정예 프로파일은 `RandomBest5Pct`.
병사별 성향 노이즈(±10%)와 "최근 점유 감점"만 우리가 넣는다(6.1절 테스트 #6).

---

## 6. 층 2 — EQS 질의 설계

### 6.1 질의 `EQS_FindCover` 파이프라인

설계 10.4절을 EQS 에셋 구성으로 내린 것. **테스트는 저작 순서대로 실행되므로 값싼 필터를 위에 둔다** [B].

| # | 테스트 | 구현 | 목적 | 필터/점수 | 가중 | 비용 |
|---|---|---|---|---|---|---|
| 0 | **Generator: Smart Objects** | **엔진 `UEnvQueryGenerator_SmartObjects`** `bOnlyClaimable=true`, `QueryBoxExtent=(1500,1500,400)`, `ActivityRequirements = SO.Cover` | 후보 슬롯 | — | — | 저 (해시그리드) |
| 1 | Distance (Querier) | 엔진 `EnvQueryTest_Distance` | 필터 ≤1500cm + "가까울수록 좋음" | 둘 다 | **2.0** | 저 |
| 2 | **Cover Quality (주 위협)** | **신규 `UEnvQueryTest_CoverQuality`** | `Protection ≥ 0.5` 필터 + 점수 | 둘 다 | **3.0** | 저 (트레이스 0) |
| 3 | **Cover Exposure (전체 위협)** | **신규** — 같은 클래스, 컨텍스트=`KnownThreats`, **`MultipleContextScoreOp = MinScore`** | 다른 위협에 대한 노출 감점 | 점수 | **2.0** | 저 |
| 4 | Corridor | 엔진 `EnvQueryTest_Volume` 또는 분대 회랑 박스 테스트 | 분대가 준 영역 밖 제외 | 필터 | — | 저 |
| 5 | **Squad Spacing** | **엔진 `EnvQueryTest_Distance`** + 컨텍스트 `EnvQueryContext_SquadMates` + **`bDefineReferenceValue=true`, `ReferenceValue=400cm`** | 적정 간격. **너무 붙어도, 너무 멀어도 감점** | 점수 | **1.0** | 중 |
| 6 | **Cover Recency** | **신규 `UEnvQueryTest_CoverRecency`** | 최근에 있던 자리 감점 | 점수 | **0.5** | 저 |
| 7 | Pathfinding (PathExist) | 엔진 `EnvQueryTest_Pathfinding` | 도달 가능성 | 필터 | — | 고 |
| 8 | **Firing Position** | **신규 `UEnvQueryTest_CoverFiringPosition`** | "여기서 표적을 쏠 수 있는가" | 둘 다 | **2.5** | **최고** |

> ★ **#3의 집계를 우리가 코딩하지 않는다.** 컨텍스트가 위협 N개를 주면 `SetScore`가 N번
> 호출되고, 집계는 엔진의 `MultipleContextScoreOp`(Average / Min / Max)가 한다 [A].
> `MinScore`로 두면 **"가장 안 막아주는 위협" 기준**이 되어 설계 10.4절의 "노출 감점"과 같은
> 효과가 나온다 — 별도의 `InverseLinear` 설정도, 합산 코드도 필요 없다.
> ⚠ 대신 **위협별 가중치(신뢰도×위험도)는 컨텍스트로 전달되지 않는다**(컨텍스트는 위치 배열만
> 준다). 가중이 꼭 필요해지면 테스트가 `ISoldierThreatProvider`를 직접 조회해야 한다 → [C-71].

> ★ **#5가 이 표에서 가장 중요한 재사용이다.** `UEnvQueryTest`의 `ReferenceValue` 필드
> (`EnvQueryTest.h:135-143`)는 *"값이 ReferenceValue에 가까울수록 높은 점수"* 로 정규화한다 [A].
> 설계 10.4절의 **"분대 응집 — 적정 간격, 너무 붙어도 감점"** 이 **엔진 필드 하나로 끝난다.**
> 커스텀 테스트를 쓰지 말 것.

> ★ **#0**: 설계 10.4절이 *"커스텀 Generator — Smart Object 서브시스템 조회"* 로 적어둔 것은
> **불필요하다.** `UEnvQueryGenerator_SmartObjects`가 엔진에 이미 있고, `bOnlyClaimable=true`가
> **예약된 슬롯을 애초에 후보에서 뺀다** [A, `EnvQueryGenerator_SmartObjects.h:37-39`].
> `ai/2026-09-02_upper_layer_plan.md` 13.2절의 "커스텀 Generator는 C++로 만든다"도 이 발견으로 갱신된다 → **R2 갱신**.

### 6.2 질의 변종

| 에셋 | 쓰는 곳 | 표 대비 차이 | 이유 |
|---|---|---|---|
| `EQS_FindCover` | `TakeCover` Intent | 위 전체 | |
| `EQS_FindCover_Fast` | **T1 티어** | #7 Pathfinding, #8 Firing Position 제거. #1 거리에 NavMesh 투영만 | 12.3절 "EQS 간소화 질의" |
| `EQS_FindFiringPosition` | `Reposition` / `AimedFire` 재배치 | #8 가중을 **4.0**으로, #2를 1.5로 | 공세적 성향 = 차폐보다 사격 우선(설계 10.4 "핵심 트레이드오프") |
| `EQS_FindOverwatchSlot` | `Overwatch` / `Hold` | #2의 컨텍스트를 "감시 방향"으로, #8 제거 | 적이 아직 없다 |

> **`FOrderConstraints::Aggression`(0~1)이 #2와 #8의 가중 비율에 직접 매핑된다**(설계 10.4 · 11.2).
> 질의 에셋을 4벌 두는 대신, `ScoringFactor`를 `FAIDataProviderFloatValue`로 두고 블랙보드에서
> 주입할 수 있는지는 [C-67].

### 6.3 신규 C++ 클래스 목록

`Source/SoldierLab/Tactical/` (설계 3.7절 모듈 레이아웃)

| 클래스 | 종류 | 책임 |
|---|---|---|
| `FSoldierCoverSlotData` | `USTRUCT : FSmartObjectDefinitionData` | **슬롯 정의에 붙는 엄폐 메타데이터.** 에디터에서 슬롯에 직접 얹힌다 |
| `FSoldierCoverRuntimeInfo` | `USTRUCT` | 월드 공간으로 미리 푼 평면 캐시(서브시스템 보유) |
| `FSoldierCoverBakePoint` / `UCoverBakeData` | `USTRUCT` / `UDataAsset` | 베이크 산출물(4절) |
| **`USoldierCoverSubsystem`** | `UWorldSubsystem` | 캐시·베이크 실체화·최근 점유 기록·**질의 동시 상한 토큰** |
| **`UEnvQueryTest_CoverQuality`** | `UEnvQueryTest` | 5.2절 Protection. 트레이스 0 |
| **`UEnvQueryTest_CoverRecency`** | `UEnvQueryTest` | 최근 점유 감점 |
| **`UEnvQueryTest_CoverFiringPosition`** | `UEnvQueryTest` | 5.4절. 트레이스 |
| `UEnvQueryContext_PrimaryThreat` / `UEnvQueryContext_KnownThreats` | `UEnvQueryContext` | 위협 위치 공급. **인지 담당과의 접점** |
| `ASoldierCoverPoint` / `ASoldierCoverVolume` | `AActor` | 수동 저작 슬롯 / 베이크 영역 |
| `UCoverProviderComponent` | `UActorComponent` | 동적 엄폐물 등록/해제 |

> **왜 EQS 테스트에서 `USmartObjectSubsystem::ReadSlotData()`를 직접 부르지 않는가**:
> `ReadSlotData`는 `TFunctionRef` + 락이다 [A, `SmartObjectSubsystem.h:759-767`]. 아이템 40개 ×
> 테스트 3개 = 120회 락을 매 질의마다 잡게 된다. **등록 시점에 한 번만 읽어 평면 `TMap`으로
> 캐시**하고, 테스트는 그걸 읽는다 → `USoldierCoverSubsystem`. [B] — 실측은 [C-68].

**Build.cs 추가 의존성**: `AIModule`, `NavigationSystem`, `SmartObjectsModule`, `GameplayTags`,
`StateTreeModule`, `GameplayStateTreeModule`.
(설계 3.5절이 `SmartObjectsModule`을 이미 나열해 두었다.)

### 6.4 ★ EQS 질의 **없이** 답해야 하는 값 두 개 — 닭과 달걀

L2 유틸리티(`ai/drafts/`)가 `TakeCover` Intent를 고르려면 **"쓸 만한 엄폐가 있는가"를
먼저 알아야 한다.** 그런데 EQS 질의는 `TakeCover`가 선택된 *뒤에* 발행된다(설계 8.4절).
**질의 결과로 질의 여부를 결정할 수는 없다.**

→ `USoldierCoverSubsystem`이 **질의 없이** 답하는 값 두 개를 따로 둔다.

| 값 | 계산 | 비용 |
|---|---|---|
| `CoverSlotAvailable` (0/1) | 반경 R 내에 **예약 안 된** 엄폐 슬롯이 있는가 | 해시그리드 조회 1회 |
| `BestCoverSlotScore` (0~1) | 그중 최고 **기하 Protection**. **트레이스·경로탐색 없음** | 후보당 내적 1~2회 |

- **갱신 주기 2Hz**, 병사 인덱스로 위상 분산. 전체 EQS 파이프라인의 축소판이 아니라 **#2 테스트만** 돌린 것.
- 이 값이 낮으면 L2가 `TakeCover`를 아예 안 고르므로 **EQS 질의 자체가 발행되지 않는다.**
  9.3절의 세 겹 방어에 **"질의를 아예 안 하게 만드는 0번째 겹"** 이 하나 더 붙는 셈이다.
- ⚠ **여기서 경로탐색을 하면 안 된다.** "도달 가능한가"는 비싸고, 그건 실제 질의(#7)가 답한다.
  여기서는 **낙관적으로** 답하고, 질의가 실패하면 L3가 `TaskFailed`를 보고한다(4.3절 규칙).

### 6.5 초안 코드

`code/` 폴더에 있다. **컴파일 검증은 불가능하므로 보수적으로 썼다** — 확인한 엔진 API만 쓰고,
불확실한 것은 주석 `// TODO(verify)` 로 표시했다.

| 파일 | 내용 |
|---|---|
| `code/SoldierCoverTypes.h` | 열거형 · `FSoldierCoverSlotData` · `FSoldierCoverRuntimeInfo` · `UCoverBakeData` |
| `code/SoldierCoverSubsystem.h` / `.cpp` | 캐시 · 실체화 · 최근 점유 · 질의 토큰 · **질의 없이 답하는 값 2개**(6.4절) |
| `code/EnvQueryContext_CoverThreats.h` / `.cpp` | 위협 컨텍스트 2종 |
| `code/EnvQueryTest_CoverQuality.h` / `.cpp` | Protection 테스트 + Recency 테스트 |
| `code/EnvQueryTest_CoverFiringPosition.h` / `.cpp` | 사격 가능성 테스트 |
| `code/SoldierCoverActors.h` / `.cpp` | `ASoldierCoverPoint` · `ASoldierCoverVolume` · `ASoldierCoverManager` · `UCoverProviderComponent`. **cpp는 Manager만 구현** — 여기가 유일하게 새 엔진 API(`CreateSmartObject`/`GetAllSlots`/`DestroySmartObject`)를 쓰는 지점이라 먼저 확정해 둔다 |

---

## 7. 층 3 — 진입 / 사격 / 퇴출의 상태 흐름

### 7.1 골격 — GASP 중첩 패턴 그대로 [A]

`ai/2026-09-02_upper_layer_plan.md` 14.3절의 규칙: **자원은 그것을 보유하는 상태가 부모여야 한다.**

```
ST_Intent_TakeCover                                    (LinkedAsset, L2가 진입시킴)
│
├─[S1] SelectCover          tasks: STT_SetSoldierInputState(Aim=true)
│  │                               STT_QueryCoverSlot        ← EQS 발행·결과 보유 (신규)
│  │   ★ 부모가 "후보 슬롯"을 보유
│  │
│  └─[S2] ClaimCover        tasks: STT_ClaimSlot             ← GASP/엔진 승계
│     │   ★★ 부모가 "예약"을 보유 → 이 상태를 벗어나면 예약이 자동 해제된다
│     │      = 사망·피격 인터럽트·명령 변경 어느 쪽으로 나가도 누수가 없다  (설계 6.5.3의 최대 위험 항목)
│     │
│     ├─[S3] ApproachCover  tasks: StateTreeTask_FindSlotEntranceLocation  ← 엔진(GameplayInteractions)
│     │                            StateTreeMoveToTask
│     │                            STT_SetSoldierInputState(Crouch = 경로 노출 시)
│     │
│     ├─[S4] SettleInCover  tasks: STT_SetCoverPosture(Behind)  (신규)
│     │                            [선택] STT_PlayAnimFromBestCost — 정렬 동작
│     │
│     └─[S5] OccupyCover    tasks: STT_SetCoverPosture(유지)
│           ├─[S5a] Hunker    cond: 피제압도 > 임계        → 완전 은폐, 사격 안 함
│           ├─[S5b] PeekFire  cond: L2가 AimedFire 요청     → 사격점으로 노출 → 버스트 → 복귀
│           └─[S5c] Reload    cond: 잔탄 낮음 + 엄폐 중
```

- **S3의 진입점 계산은 엔진이 준다** — `StateTreeTask_FindSlotEntranceLocation`
  (`GameplayInteractions` 플러그인 Public) + `FSmartObjectSlotEntranceAnnotation`
  (`bIsEntry`/`bIsExit`/`bTraceGroundLocation`/`bCheckTransitionTrajectory`) [A, 엔진 소스].
  GASP의 `STT_FindSlotTransforms`가 하던 일이 여기 대응한다.
  → **엄폐물 진입 지점을 우리가 계산할 필요가 없다.**
- S3에는 `STT_CharacterIgnoreCollisionsWithOtherActor`(GASP 승계)를 붙인다 — 엄폐물에 밀착할 때 필요.

### 7.2 사격 — 슬롯을 떠나지 않는다

> **규칙: lean / over-top / blind 는 캡슐을 옮기지 않는다. 포즈만 바뀐다.**

이유가 두 개다.
1. 캡슐이 움직이면 SmartObject 슬롯 위치와 실제 위치가 어긋나 예약의 의미가 사라진다.
2. lean 도중 다른 병사가 그 공간을 지나가면 겹침 밀어내기가 캐릭터를 슬롯 밖으로 밀어낸다.

따라서 사격은 `FSoldierPoseIntent`의 축으로 표현된다 → 7.5절.

**총구가 엄폐물을 뚫는 문제** — 이건 반드시 다뤄야 한다:
```
매 사격 판정 전에 총구 소켓에서 조준 방향으로 40cm Cover 채널 트레이스
  걸림  → 사격 억제 + 자세를 한 단계 올림(Crouch→Stand, Behind→Lean)
  계속 걸림 → L3가 TaskFailed 보고 → L2가 Reposition 재평가
```
이건 6.2절 "조준 수렴 조건"과 같은 계열의 게이트다 — **"몸이 준비 안 됐으면 못 쏜다"**.

### 7.3 퇴출

| 경로 | 처리 | 예약 해제 |
|---|---|---|
| L2가 다른 Intent 선택 | S1 이탈 | **자동** (S2 부모 상태 종료) [A] |
| 사망 | 상태 트리 이탈 | **자동** [A] — 설계 6.5.3 최대 위험 항목이 구조로 해결 |
| 슬롯 무효화(파괴) | `CoverInvalidated` 이벤트 → S2 실패 전이 | 자동 |
| `Reposition` | **한 Intent 안에서** 이탈→이동→진입. 새 슬롯을 먼저 예약한 뒤 옛 슬롯을 놓는다 | 순서 주의 → [C-69] |

### 7.4 ★ 엄폐 퇴출 전용 애니메이션을 만들지 않는다 [B]

중첩 상태 구조의 대가(代價)다: **S2를 벗어나는 순간 예약이 풀리므로, 그 뒤에 "퇴출 동작"을
재생할 상태가 없다.** 퇴출 동작을 재생하려고 상태를 하나 더 두면 14.3절의 이점이 사라진다.

**대신**: 퇴출은 **다음 Intent의 진입부가 흡수한다.**
- `Advance`/`Retreat`의 첫 프레임에서 자세가 Crouch→Stand로 보간되고, MM이 start 클립을 고른다.
- `MM_Rifle_Crouch_Exit`가 **이미 있다**(에셋 실측 — 8절).
- 즉 "엄폐에서 뛰쳐나가는 동작"은 **기존 crouch exit + walk start의 MM 전환**으로 나온다.

> 이 결정이 조달 규모를 눈에 띄게 줄인다. 뒤집으려면 14.3절의 자원 소유 규칙을 먼저 재검토해야 한다.

### 7.5 L3 → L4 계약 추가 요구 (`FSoldierPoseIntent`)

애니메이션 담당에게 요청할 필드. **의미만 담고 숫자는 L4가 만든다**(P3 원칙).

| 필드 | 값 | 소비처 |
|---|---|---|
| `CoverPosture` | `None / Behind / PeekLeft / PeekRight / OverTop / Hunker` | 자세·lean·조준 억제 |
| `CoverLeanAlpha` | 0~1 (전이 중 보간값) | 상체 lean additive |
| `bSuppressAimOffset` | Hunker 중 true | AO 게이트 (`disable_ao` 커브와 같은 계열, [C-47]) |
| `CoverHeightHint` | `Low / High` | 자세 높이 목표 |

---

## 8. 애니메이션 요구 목록 — 조달 계획 입력

### 8.1 지금 있는 것 (에셋 실측, 2026-09-09)

`Content/SoldierLab/Animations/Rifle/` 전수 확인:

| 카테고리 | 보유 | 엄폐에 쓰이는가 |
|---|---|---|
| Stand/Crouch × Walk/Jog × 4방향 **loops / starts / stops / pivots** | ✅ 48클립 | 접근·이탈 전부 커버 |
| TurnInPlace 90/180 L/R (Stand + Crouch) | ✅ 8클립 | 슬롯에서 몸통 재정렬(5.5.6절) |
| **`MM_Rifle_Crouch_Entry` / `MM_Rifle_Crouch_Exit`** | ✅ | **★ 엄폐 자세 전환의 절반이 이미 있다** |
| `MM_Rifle_Idle_ADS` / `Idle_Hipfire` / `Crouch_Idle` / `IdleBreak_Scan` | ✅ | 슬롯 대기 자세 · Overwatch 스캔 |
| Fire / DryFire / Reload / GrenadeToss 몽타주 | ✅ | S5b / S5c |
| HitReact 12 / Death 6 | ✅ | 피격 인터럽트 |
| AO Hipfire 15포즈 + `AO_Rifle_Aim` | ✅ | 조준 편차 |
| `MM_Rifle_Jog_Leans_L/Center/R` | ✅ | ⚠ **뱅킹(선회 관성)용이다. 전술 lean이 아니다** |

### 8.2 없는 것 — 요청 목록

| # | 클립 | 왜 필요 | 없을 때의 대안 | 우선 |
|---|---|---|---|---|
| **A1** | `Cover_Lean_L` / `Cover_Lean_R` **(애디티브 2개)** | 높은 엄폐에서 옆으로 몸을 내밀어 조준 | **상체 additive 기울임으로 임시 대체 가능** — 8.3절 | ★★ |
| **A2** | `Cover_Hunker_Low` / `Cover_Hunker_High` (2) | 피제압 시 완전 은폐(총 내리고 웅크림). **연출상 가장 눈에 띄는 상태** | `Crouch_Idle` + 조준 억제 | ★★ |
| **A3** | `Cover_OverTop_Fire` 정지 포즈 (1) | 낮은 엄폐에서 일어서 넘겨쏘기 | `Idle_ADS` + 자세 높이 보간 | ★ |
| **A4** | `Cover_Blind_OverTop` / `_SideL` / `_SideR` (3) | 맹목사격(고개 안 내밀고 총만) | ~~Control Rig IK로 총만 올림 → 품질 하한이 [C-8]~~ → ★ **2026-09-12: 이미 존재한다.** `MM_Rifle_BlindFire_L/R/U` 저작 포즈 3장 + 연속 마스크 애디티브(`BlindFireH`/`BlindFireV`)가 **구현돼 동작 중**이다. **IK 경로는 기각됐다** — 팔 IK는 척추를 돌리지 못한다. [C-8] 해결 → `IMPLEMENTED.md` 2.5e절 | ☆ → **대체됨** |
| **A5** | `Cover_Peek_In` / `_Out` 전환 (2~4) | lean 진입/복귀의 전신 전환 | 포즈 보간(A1이 additive면 자연히 해결) | ☆ |
| **A6** | `Cover_Enter_Front/L/R` (3) | 엄폐물에 정렬해 붙는 동작 | **Motion Warping + 기존 stop 클립** — 5.5.5절이 이 경로를 이미 지정 | ☆ |
| **A7** | `Prone_*` 전체 세트 (~15) | 엎드림 엄폐 | — | **범위 밖 권고** → `OPEN_QUESTIONS.md` **Q36** (인지 초안은 이미 Prone을 전제한다 — 12.4절) |

**★★ 4개(A1×2 + A2×2)가 최소 집합이다.** 이것만 있으면 엄폐가 "동작하는 것처럼" 보인다.

### 8.3 ★ 설계 판단 — 엄폐 lean은 **애디티브로 간다** [B]

5.5.1절이 "상하체 분리"를 이 프로젝트의 실증적 실패로 못박았다. 그럼 lean도 안 되는가? **아니다.
구분해야 한다.**

| | 5.5.1이 실패로 판정한 것 | 엄폐 lean |
|---|---|---|
| 언제 | **이동 중** (발맞춤에 동기화된 전신 움직임) | **정지 중** (슬롯에 서 있다) |
| 무엇을 | 상체를 **다른 클립으로 교체** | 베이스 포즈에 **작은 편차를 얹기** |
| 대응하는 층 | — | **5.5.3의 [3]층 (Aim Offset과 같은 계열)** |

5.5.3절이 *"조준각 적응 Aim Offset(애디티브) — 풀바디 베이스에 대한 '작은 편차'라 레이어
블렌드와 달리 포즈를 무너뜨리지 않는다"* 라고 이미 쓴 그 자리에 lean이 들어간다.

**규칙: lean은 정지 상태에서만 켠다.** 이동 중 lean 요청은 무시하고, 슬롯에 도착한 뒤 켠다.
→ 클립 요구가 **전신 세트가 아니라 애디티브 2개**로 줄어든다. **[C-70]로 실측 판정.**

### 8.4 조달 경로

| 경로 | 대상 | 비고 |
|---|---|---|
| Lyra | 이미 반입한 61클립에 엄폐 전용은 **없음**(실측) | |
| Mixamo | "cover crouch", "cover peek" 계열 존재 (추정) | P0-4 파이프라인이 검증돼 있어 반입 비용은 낮다 |
| 자체 제작 | A2 hunker 2개는 **정지 포즈**라 난이도가 낮다(3.4절 "정지 포즈는 루트모션도 접지 커브도 필요 없다") | 디자인팀 요청 후보 |

**반입 시 커브 세트**: 정지 포즈이므로 `animation/prototypes/2026-09-04_c34_clip_curve_mapping.md` 4절의
**"정지 클립" 행**을 따른다 — `enable_warping` **만들지 말 것**(P8d).

---

## 9. 성능 — 45명 @ 60fps

### 9.1 예산의 출처와, 문서 간 불일치 하나 [A]

| 출처 | 값 | 성격 |
|---|---|---|
| 설계 3.6절 (프로젝트 설정) | EQS `Max Allowed Time Per Frame` = **0.003 (3ms)** | **엔진에 거는 상한(안전판)** |
| 설계 12.2절 (예산 분해) | EQS = **0.7 ms / frame** | **우리가 지켜야 하는 목표** |

> ⚠ **둘은 모순이 아니라 역할이 다르다.** 3ms는 "폭주해도 여기서 끊긴다"이고 0.7ms가 설계 목표다.
> **3ms를 예산으로 착각하면 12.2절의 나머지 항목(애님 2.5ms)이 밀린다.**
> 문서에 이 구분이 없어 오독 위험이 있다 → 설계 문서 정정 제안(`OPEN_QUESTIONS.md` Q38).

### 9.2 질의 하나의 비용 산정 [B] → 판정 [C-68]

| 항목 | 값 | 근거 |
|---|---|---|
| 후보 슬롯 (반경 15m, 시가지 밀도) | ~40 | [B] 짐 레벨 만들어 실측 |
| #1 거리 필터 통과 | ~30 | |
| #2 차폐 필터(Protection ≥ 0.5) 통과 | ~10 | 위협이 한 방향이면 절반 이상 탈락 |
| #7 경로 존재 필터 통과 | ~8 | |
| **#8 트레이스 도달** | **~8회** | 아이템당 1 트레이스 상한 |
| 목표 질의 비용 | **≤ 0.5 ms** | **[C-68] 판정 기준** |

### 9.3 빈도 — 여기가 진짜 설계 지점

```
45명 × 0.5회/초  =  22.5 질의/초
22.5 × 0.5ms     =  11.25 ms/초  =  0.19 ms/frame (평균)     ← 0.7ms 예산의 27%
```

평균은 여유롭다. **문제는 피크다** — 분대가 동시에 피격당하면 45개 질의가 한 프레임에 들어온다.
그래서 세 겹으로 막는다:

| 장치 | 내용 | 어디에 |
|---|---|---|
| **① 이벤트 구동** | **주기 재질의를 두지 않는다.** `TakeCover`/`Reposition` Intent 진입 시 1회. 그 외엔 `CoverInvalidated`·`ThreatDirectionChanged(>60°)`·피격 시에만 | L2/L3 |
| **② 병사별 쿨다운** | 재질의 최소 간격 **2.0초**. GASP `STC_CheckCooldown` / `STT_AddCooldown` 승계 | StateTree |
| **③ 전역 동시 상한(토큰)** | `USoldierCoverSubsystem`이 in-flight 질의를 **N=4**로 제한. 초과분은 우선순위 큐(체력 낮은 순) | 서브시스템 |

> ③이 필요한 이유: EQS 매니저의 3ms 타임슬라이스는 **각 질의를 늦출 뿐 거절하지 않는다.**
> 45개가 큐에 쌓이면 마지막 병사는 몇 초 뒤에 답을 받는다 — "엄폐하러 가라"고 했는데 3초 뒤에
> 움직이는 병사가 된다. **지연은 프레임 문제가 아니라 행동 품질 문제로 나타난다.** [B]

### 9.4 LOD 티어별 (설계 12.3절)

| 티어 | 질의 | 상한 |
|---|---|---|
| **T0** (≤8명) | `EQS_FindCover` 전체 | 쿨다운 2.0s |
| **T1** (~20명) | `EQS_FindCover_Fast` (트레이스·경로 테스트 제거) | 쿨다운 4.0s |
| **T2** (나머지) | **질의하지 않는다.** 서브시스템이 준 "가장 가까운 유효 슬롯"을 그대로 쓴다 | — |

12.3절이 T2를 *"엄폐는 캐시된 슬롯 사용"* 으로 이미 지정해 두었다.

### 9.5 캐싱 전략

| 무엇 | 어디에 | 무효화 |
|---|---|---|
| 슬롯 메타데이터(월드 공간 facing·사격점) | `USoldierCoverSubsystem`의 평면 `TMap` | 슬롯 등록/해제/이동 시 |
| 최근 점유 이력 | 병사당 링버퍼 4개 + 타임스탬프 | 30초 경과 |
| 위협 요약(`FCoverThreatInfo[]`) | **분대당 1벌** — 개인마다 다시 만들지 않는다 | 분대 blackboard 갱신 주기(2~4Hz) |
| 최근 질의 결과 상위 N | 병사당 3개 | 쿨다운과 동일 |

> **위협 요약을 분대 단위로 두는 것**이 두 번째로 큰 절약이다. 5.3절의 Exposure 계산이
> `위협 수 × 슬롯 수`인데, 위협 목록을 5명이 각자 만들면 그 앞단이 5배가 된다.
> `FSquadBlackboard::KnownThreats`(9.3절)가 이미 그 자리다 → **분대 담당과의 계약**.

### 9.6 관찰 가능성 — 축을 구현하기 전에 넣는다 (P7)

`ai/2026-09-02_upper_layer_plan.md` 10절이 *"엄폐: EQS 후보 슬롯의 점수 히트맵"* 을 이미 요구한다. 추가로:

| 표시 | 왜 |
|---|---|
| 슬롯의 `CoverFacing` 화살표 + `HalfAngle` 부채꼴 | **2절 좌표 규약이 맞는지가 눈으로 보여야 한다.** P16 "대조군 없이 측정을 신뢰하지 말 것" |
| 사격점 3종 위치 + 활성 여부 | 베이크 6단계 검증 |
| 아이템별 **테스트별 점수 분해** (EQS 디버거가 이미 준다) | 왜 이 슬롯이 이겼는가 |
| 예약 상태 색상 (자유/예약됨/사용중) | P0-3 판정 기준 "3명이 서로 다른 슬롯" |
| 질의 큐 길이 · 평균 대기시간 | 9.3절 ③이 작동하는지 |

---

## 10. 구현 순서

| 단계 | 하는 일 | 판정 기준 |
|---|---|---|
| **P0-3a** | 테스트 레벨 + `ASoldierCoverPoint` 수동 8개 + `SOD_Cover` 슬롯 정의 | 에디터에서 슬롯 시각화가 보인다 |
| **P0-3b** | `USoldierCoverSubsystem` + `UEnvQueryTest_CoverQuality` + 컨텍스트 | EQS 테스팅 폰으로 점수 히트맵이 위협 방향에 반응한다 |
| **P0-3c** | `ST_Intent_TakeCover` (S1~S4까지) | **병사 3명이 서로 다른 슬롯을 예약해서 들어간다** ← 설계 14절 P0-3 판정 |
| **P0-3d** | 사망/이탈 시 예약 해제 확인 | 병사를 죽였을 때 슬롯이 즉시 다시 잡힌다 (14.3절 구조 검증) |
| P1-a | `UEnvQueryTest_CoverFiringPosition` + S5b PeekFire | 병사가 엄폐 뒤에서 lean으로 쏜다 |
| P1-b | 베이크 툴 + `UCoverBakeData` | **마커 하나 없는 임의 레벨에서 엄폐한다** ← P1 검수 기준 |
| P1-c | 애니메이션 A1/A2 반입 | 엄폐 자세가 "엄폐로 보인다" |
| P3 | 45명 성능 측정 · LOD 티어 튜닝 | 0.7ms 예산 내 |

---

## 11. 이 문서가 만든 미해결 항목

`OPEN_ITEMS.md`에 등록해야 한다. ID는 각 접두의 현재 최대 +1부터 할당했다.

| # | 항목 | 판정 기준 | 시점 |
|---|---|---|---|
| **C-62** | 슬롯 정의를 **에셋 1개로 재사용**할지 슬롯마다 만들지 | `USmartObjectDefinition` 하나에 슬롯 N개를 두는 구조 vs 슬롯 1개짜리 정의를 N번 인스턴스화. 후자가 베이크에 맞지만 메모리·등록 비용 미측정 | P0-3 |
| **C-63** | `ARecastNavMesh::GetDebugGeometry`로 **NavMesh 경계 에지를 에디터 툴에서 읽을 수 있는가** | 못 읽으면 베이크 3.2절 2단계를 "볼륨 내 스태틱 메시 바운드 스윕"으로 대체 | P1 |
| **C-64** | **이동하는 엄폐물**의 슬롯 예약 유지 정책 | UGV가 움직이는 동안 예약을 유지하는가, 매번 끊는가. 유지하면 병사가 끌려가야 한다 | P1~P3 |
| **C-65** | `HeightFactor(Low, Stand) = 0.25` 가 맞는가 | 낮은 엄폐 뒤에 서 있을 때 실제 피탄 면적 비율. 사격선 트레이스로 실측 | P1 |
| **C-66** | **고저차 있는 위협**에서 수평 각도 판정이 틀리는 정도 | 옥상의 적에 대해 Protection이 과대평가되는가. 짐 레벨(D3)에 고지대 필수 | P1 |
| **C-67** | EQS `ScoringFactor`를 **블랙보드에서 주입**할 수 있는가 | 되면 `Aggression`으로 질의 1개를 재사용, 안 되면 질의 에셋 4벌 | P1 |
| **C-68** | **질의 1회 비용 ≤ 0.5ms** 인가 | EQS 프로파일러. 초과하면 #8 트레이스 상한을 0으로(기하 판정만) | P0-3 |
| **C-69** | `Reposition`에서 **새 슬롯 예약 → 옛 슬롯 해제** 순서가 안전한가 | 한 병사가 슬롯 2개를 잠깐 점유한다. 슬롯이 희소하면 교착 가능 | P1 |
| **C-70** | **엄폐 lean을 애디티브로 만들면 견착 포즈가 무너지는가** | 8.3절 판단의 본 판정. lean 25°/45°에서 총구가 몸을 뚫는지 | P1 |
| **C-71** | 위협별 **가중치(신뢰도×위험도)를 EQS에 전달**할 필요가 실제로 있는가 | 컨텍스트는 위치 배열만 준다. `MinScore` 집계(최악 위협 기준)로 충분한지, 아니면 테스트가 `ISoldierThreatProvider`를 직접 조회해야 하는지 | P1 |
| **D10** | 엄폐 슬롯의 **GameplayTag 체계** | `SO.Cover.Low` / `.High` / `.Firing` / `.Overwatch`. 질의 필터가 이걸 쓴다 | P0-3 |
| **D11** | **엄폐 중 아군 사격선** 처리 | 설계 9.5의 `FireLaneAllyMarginCm` 이식이 엄폐 슬롯 평가에도 들어가야 하는가 | P2 |
| **R6** | `STT_FindSmartObject`/`ClaimSlot`/`UseSmartObject`가 **어느 모듈 제공인가** | `Content/Blueprints/AI/StateTree/TasksAndConditions/`에 **없다**(실측 — BP 6개뿐). 엔진 `GameplayInteractions`/`SmartObjects` 제공일 가능성이 높다 → 설계 13절 "GameplayInteractions는 Experimental이라 안 쓴다"와 충돌 | **P0-3 착수 즉시** |
| **R7** | `EQS_FindCover`를 **StateTree에서 발행하는 표준 경로** | `StateTreeRunEnvQueryTask`가 5.8에 있는가, 아니면 자체 태스크가 필요한가 | P0-3 |

---

## 12. 다른 담당 문서와의 접점 — **실측 대조 (2026-09-09)**

같은 날 `ai/drafts/`(인지·판단)와 `squad/drafts/`(분대)가 함께 작성됐다.
**추측이 아니라 그 문서의 실제 심볼과 대조한 결과다** [A].

### 12.1 인지·판단 쪽이 **나에게 채워 달라고 남긴 자리**

`ai/drafts/Source/SoldierAITypes.h:478-480` —
> `// Supplied by the cover / EQS layer (not owned by this module)`
> `SoldierScoringInputs::BestCoverSlotScore` · `SoldierScoringInputs::CoverSlotAvailable`

**이 두 개가 내 층의 공식 출력이다.** 6.4절이 그 계산을 정의했다. 채우는 주체는
`USoldierCoverSubsystem`(2Hz) → `USoldierBrainComponent`의 스코어링 입력.

추가로 `SoldierScoringInputs::CurrentCoverQuality`(같은 파일 460행)도 사실상 내 층이 답해야 한다:

```
현재 슬롯을 점유 중  →  Protection(현재 슬롯, 현재 자세, 주 위협)
슬롯 밖              →  0        (엄폐하지 않은 것으로 친다)
```

### 12.2 ★ `Exposure`는 **두 개의 다른 숫자다** — Q13의 답이 실측으로 확정됐다

`ai/drafts/Source/SoldierAIConfig.h:189-212`의 실제 구성:

```
Exposure ← ExposureStanding/Crouched/Prone × ExposureStationary/Walking/Running × ExposureFiring
```

**엄폐 항(項)이 하나도 없다.** 즉 인지의 `Exposure`는 *"내 자세·이동·사격 때문에 얼마나
눈에 띄는가"* 이고, 내 5.3절 `ComputeExposure(slot, threats, stance)`는 *"이 지점의 기하가
얼마나 안 가려주는가"* 다. **둘은 곱해져야 하는 서로 다른 인자이지, 같은 숫자가 아니다.**

→ **제안하는 계약 (한 줄)**:
```
PerceivedExposure = 인지의 Exposure × (1 - CurrentCoverQuality)
```
`CurrentCoverQuality`는 12.1절대로 내가 채운다. **인지 쪽은 곱하기 한 번만 추가하면 된다.**

> ⚠ `ai/2026-09-02_upper_layer_plan.md` 5.4절의 *"같은 숫자를 두 시스템이 공유한다"* 는 **정확히는 틀렸다.**
> 공유되는 것은 숫자가 아니라 **곱셈 인자 하나**다. 이 구분이 없으면 두 팀이 각자
> "엄폐를 반영한 Exposure"를 만들어 **엄폐가 두 번 곱해진다.** → `OPEN_QUESTIONS.md` Q35

### 12.3 분대 쪽이 나에게 물어본 것 — 슬롯 식별자 타입

`squad/drafts/Source/Squad/SquadTypes.h:284-290`:
```cpp
struct FSlotSoftClaim
{
    /** SmartObject 슬롯 식별자. 실제 타입은 엄폐 담당자의 스키마에 맞춘다 [Q23 인접] */
    FGuid SlotId;
    ...
    float ExpiresAtSeconds;
};
```

**답: `FSmartObjectSlotHandle`로 간다.** 근거:

| | `FGuid`(현재) | **`FSmartObjectSlotHandle`(권고)** |
|---|---|---|
| soft-claim의 수명 | `ExpiresAtSeconds` — **몇 초짜리 휘발성** | 같음 |
| 저장 필요성 | 없음 | 없음 |
| 조회 | 핸들 ↔ GUID 변환표를 내가 유지해야 한다 | **변환 없음.** EQS 결과·`ClaimSlot`·내 캐시가 전부 이 타입 |
| 실제 예약과의 정합 | 두 식별자가 어긋날 여지 | 하나뿐 |

> soft-claim이 **영속 데이터가 아니라는 점**이 결정적이다. 세션을 넘어 살아남을 필요가
> 없으므로 런타임 핸들이 옳다. 분대 문서가 *"최적화이지 정확성 장치가 아니다"* 라고 스스로
> 적어 둔 것과 정확히 같은 이유다.
>
> ⚠ 단 `FSmartObjectSlotHandle`에 `GetTypeHash`가 있는지 확인해야 한다 → 코드의 `TODO(verify)`.

`FSquadBlackboard::AssignedCorridor`(`FBox`, 511행)는 6.1절 테스트 #4가 그대로 쓴다.

### 12.4 Prone 충돌

인지 쪽에 **`ExposureProne = 0.35`가 이미 있다**(`SoldierAIConfig.h:196`).
반면 `PLAN.md`는 8.2절 A7에서 Prone을 범위 밖으로 권고했다 — **애니메이션이 0개**이기 때문이다.
**두 문서가 서로 다른 전제 위에 있다** → `OPEN_QUESTIONS.md` **Q36**에서 한 번에 결정할 것.

### 12.5 여전히 내 범위가 **아닌** 것

| 항목 | 담당 |
|---|---|
| 위협의 신뢰도·위험도(`w_i`) 계산 | 인지 — `GetPrimaryThreatWeighted()`가 이미 있다 |
| `TakeCover` Intent를 언제 고르는가 | L2 유틸리티 |
| 회랑·토큰·역할 배분 | 분대 |
| 사격 타이밍·조준 수렴 | 무기(설계 6절) |
| lean 애디티브의 실제 제작 | 애니메이션(L4) |
