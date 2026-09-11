# 초안 검증 기록

2026-09-10 / 초안 3건이 "컴파일 검증 불가"로 남긴 항목을 **엔진 소스로** 대조한 결과.
에디터 없이 확인 가능한 것만 다룬다.

---

## ✅ [C-63] UE5.8 StateTree 노드 API — **초안이 맞다**

`drafts/squad/Source/Squad/SquadStateTreeNodes.h`가 5.3~5.5 계열 API를 가정하고
"엔진 대조 필요"로 표시했던 항목이다. 대조 결과 **전부 유효하다** [A].

### 베이스 클래스 (전부 존재)

```
FStateTreeTaskCommonBase       : FStateTreeTaskBase      : FStateTreeNodeBase
FStateTreeEvaluatorCommonBase  : FStateTreeEvaluatorBase : FStateTreeNodeBase
FStateTreeConditionCommonBase  : FStateTreeConditionBase : FStateTreeNodeBase
```

### ⚠ 모듈 경로가 다르다

```
StateTree는 Runtime 모듈이 아니라 플러그인이다.
  Engine/Plugins/Runtime/StateTree/Source/StateTreeModule/Public/
```

헤더 include 자체는 `#include "StateTreeTaskBase.h"`로 동일하지만,
**`Build.cs`의 `PublicDependencyModuleNames`에 `StateTreeModule`을 넣어야 한다.**
(`GameplayStateTree`도 별도 플러그인이다)

### 가상함수 시그니처 — 초안과 일치

| 노드 | 시그니처 |
|---|---|
| Task | `EStateTreeRunStatus EnterState(FStateTreeExecutionContext&, const FStateTreeTransitionResult&) const` |
| Task | `void ExitState(FStateTreeExecutionContext&, const FStateTreeTransitionResult&) const` |
| Task | `EStateTreeRunStatus Tick(FStateTreeExecutionContext&, const float) const` |
| Task | `void StateCompleted(FStateTreeExecutionContext&, const EStateTreeRunStatus, const FStateTreeActiveStates&) const` |
| Evaluator | `void TreeStart / TreeStop(FStateTreeExecutionContext&) const` · `void Tick(..., const float) const` |
| Condition | `bool TestCondition(FStateTreeExecutionContext&) const` |

> ★ **전부 `const`다.** StateTree 노드는 **무상태**이고 상태는 InstanceData에 산다.
> 노드 멤버에 런타임 상태를 두면 여러 인스턴스가 공유해 조용히 망가진다.
> 초안은 이 규약을 지켰다.

### InstanceData 관례 — 초안과 일치

`FStateTreeNodeBase`(`StateTreeNodeBase.h:75-94`)가 정의한다:

```cpp
using FInstanceDataType = FNoInstanceDataType;              // :82
virtual const UStruct* GetInstanceDataType() const          // :94
```

초안이 쓴 형태가 그대로 관례다:

```cpp
using FInstanceDataType = FSTT_AcquireSquadTokenInstanceData;
virtual const UStruct* GetInstanceDataType() const override
    { return FInstanceDataType::StaticStruct(); }
```

## ✅ 엠폐 EQS — 커스텀 Generator가 불필요하다는 판정은 **맞다** [A]

```
Engine/Plugins/Runtime/SmartObjects/Source/SmartObjectsModule/Public/
    EnvQueryGenerator_SmartObjects.h   :12  class UEnvQueryGenerator_SmartObjects : public UEnvQueryGenerator
                                       :39  bool bOnlyClaimable = true;   ← 기본값
    EnvQueryItemType_SmartObject.h
```

예약된 슬롯을 후보에서 빼는 것이 **기본 동작**이다. 모듈은 `SmartObjectsModule`.

→ **설계 10.4절 · `upper_layer_plan` 13.2절의 "커스텀 Generator를 C++로 만든다"는 갱신 대상이다.**
   작업량이 줄어든다.

---

## 아직 검증 못 한 것

에디터나 빌드가 필요해 이번에 못 본 것들. 착수 전 확인 대상이다.

| 항목 | 출처 | 확인 방법 |
|---|---|---|
| `UAISense::GetSenseID` 사용법 | `soldier_ai` | 엔진 소스 대조 가능 — 아직 안 함 |
| `UTickableWorldSubsystem` 사용법 | `soldier_ai` | 〃 |
| 본 이름(`spine_01`/`neck_01` 등) 실재 | `soldier_ai` | 스켈레톤 확인 (에디터) |
| EQS Generator/Test 클래스 시그니처 | `cover` | 엔진 소스 대조 가능 |
| **[R6] GameplayInteractions 실제 모듈 확인** | `cover` Q12 | `STT_FindSmartObject` 등이 엔진 태스크인지 (에디터 5분) |
| 전체 컴파일 | 셋 다 | 에디터 닫고 빌드 (P13) |
