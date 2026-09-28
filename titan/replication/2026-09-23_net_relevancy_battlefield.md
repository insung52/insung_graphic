# 네트워크 관련성 — 클라이언트 드론 화면에 전장이 통째로 없던 문제

2026-09-23 / 완료 / 자체방호 클라이언트에서만 드론 화면의 전장이 "다른 씬"처럼 비어 있던 원인은 거리 기반 네트워크 관련성(150m) — 기준점이 트럭이고 전장은 923m 밖이라 병사·멀티캐스트가 통째로 안 갔다. 탐지 대상 액터를 always relevant로. 같은 축의 치트 매니저(ToggleDebugCamera) 건도 함께.

선행: `2026-09-15_drone_two_pc_validation.md`(2 PC 첫 검증·버그 3건). 현재 동작은
`vehicle/drone/drone_flight_dev_guide.md` 15절. 같은 날 드론 쪽 작업 2건은
`vehicle/drone/2026-09-23_drone_remote_rotor_and_squad_tracking.md`.

---

## 1. 증상 (사용자 리포트)

자체방호축 **클라이언트로 접속했을 때만**:

1. 드론 타겟 디텍션 결과가 **낙하산·트럭 2개뿐**. 적군/아군/UGV는 아예 안 잡힘.
2. 교전 장면의 **총구 화염·사격음이 안 보이고 안 들림**. 드론 시점으로 청취를 켜도 조용.
3. 병사들이 그 자리에 얼어붙어 있음.

사용자 표현: *"뭔가 혼자 다른 씬에 있는 거 같은 느낌"*. 09-15에 고친 드론 본체의 위치/짐벌
리플리케이션은 멀쩡했다 — **드론만 멀쩡하고 나머지 세계가 없었다.**

Solo(자체방호 단독)와 호스트(UGV축) 자기 화면에서는 전부 정상이라, 오래 "PIE 특유의 문제"로
오해하고 넘겨왔던 건이다.

---

## 2. 원인 — 관련성 기준점은 씬캡쳐가 아니라 ViewTarget

드론 짐벌은 `USceneCaptureComponent2D` 하나가 날아다니는 것이고, **엔진은 그걸 네트워크
뷰어로 치지 않는다.** 관련성을 재는 기준은 그 연결의 `ViewTarget` — 자체방호 클라에선
**트럭**이다(`Client_OnAxisResolved`의 `SetViewTarget(Truck)`).

실측:

| | 값 | 출처 |
|---|---|---|
| 트럭(클라 ViewTarget) | (57330, 12280, -3920) | 레벨 인스턴스 |
| 낙하산/전장 | (-34870, 13550, 1030) | 〃 |
| 거리 | **약 923 m** | |
| `NetCullDistanceSquared`(드론·트럭·UGV·`BP_Enemy_kadex` 전부) | 225,000,000 → **150 m** | 엔진 기본값 |
| `bAlwaysRelevant` | 전부 false | |
| `bUseDistanceBasedRelevancy` | true | `GameNetworkManager.cpp:54`, 프로젝트 ini 오버라이드 없음 |

전장이 관련성 반경의 **6.2배 밖**이다.

`AActor::IsNetRelevantFor`(`ActorReplication.cpp:388`)에서 살아남는 건 둘뿐이었다:

- **드론** — `Atitan_exampleGameMode::PostLogin`의 `SetOwner(자체방호 PC)` 덕에
  `IsOwnedBy(RealViewer)`로 통과. 그래서 드론만 멀쩡했다.
- **트럭** — 자기가 `ViewTarget`.

나머지는 전부 거리 판정으로 떨어졌고, 그 결과가 증상 그대로다:

- **병사(적군/아군)**: 관련성 밖이라 갱신이 끊겨 얼어붙는다.
- **UGV**: 레벨 배치 + 복제라 클라에 존재는 하지만 **출발 위치에 얼어붙은 채**. 드론이 전장을
  봐도 거기 없으니 안 잡힌다.
- **총구 화염·사격음·피격**: 전부 Multicast인데, **멀티캐스트도 같은 관련성 검사를 거쳐
  송신 단계에서 폐기된다**(`NetDriver.cpp:8243`). 비신뢰 멀티캐스트는 예외도 없다.
- **적군이 숨은 채로 남음**: `bIsRevealed`의 OnRep이 안 오니 클라에선 영영 미발견 →
  `UTargetDetectionComponent::ScanTargets`의 `!IsRevealed() → continue`로 탐지에서도 빠진다.
- **낙하산**: `bReplicates=false`. 복제를 안 하니 프로세스마다 독립된 로컬 사본이라 혼자
  멀쩡했다 — 트럭과 함께 둘만 보이던 이유.

### 2.1 왜 Solo·호스트는 멀쩡했나

**관련성 판정은 원격 연결에만 존재한다.** 그래서 같은 코드가 셋으로 갈린다:

| 실행 형태 | 넷드라이버 | 드론 화면의 전장 |
|---|---|---|
| Solo (`StartSoloAxis` → `?Listen` 없이 open = `NM_Standalone`) | 없음 | ✅ 정상 — `ServerReplicateActors`가 안 돌고 `IsNetRelevantFor`는 호출조차 안 된다. 멀티캐스트도 라우팅 없이 **로컬 함수 호출**로 즉시 실행 |
| 호스트(UGV축 리슨서버) 자기 화면 | 있음 | ✅ 정상 — 관련성은 연결마다 계산되고, 서버 자신의 로컬 플레이어는 연결이 아니다 |
| **클라이언트 접속** | 있음 | ❌ 150 m 밖 전부 얼어붙음 + 멀티캐스트 유실 |

"옛날부터 클라로 들어가면 이상했다"가 이걸로 설명된다. 그리고 **패키징 2 PC 전시에서만
터지는 구조**다.

---

## 3. 수정 — 탐지 대상 액터를 always relevant로

훅은 `UDetectableTargetComponent::BeginPlay`. 서버에서, 복제되는 소유 액터에 한해
`bAlwaysRelevant = true`.

```cpp
if (bForceAlwaysNetRelevant && Owner->GetIsReplicated() && Owner->HasAuthority())
{
    Owner->bAlwaysRelevant = true;
}
```

`bForceAlwaysNetRelevant`(기본 켬)로 끌 수 있다 — 끄면 예전 거리 기반으로 돌아간다.

**왜 여기가 맞는 자리인가**

- 이 컴포넌트를 단 액터의 집합 = **드론/RCWS가 봐야 하는 대상의 집합**이다. 더 넓지도 좁지도
  않다. 낙하산은 `bReplicates=false`라 자동으로 건너뛴다.
- 병사 BP(`BP_Enemy_kadex` 등)는 `ACharacter` 직속이라 **생성자 기본값을 둘 C++ 자리가 없다.**
  BP CDO를 일일이 고치는 대신 코드 한 곳에서 끝난다.
- SoldierLab 병사는 이 컴포넌트가 브리지에 의해 **런타임에 붙으므로**, 새 병사가 추가돼도
  자동으로 따라간다(`USoldierLabBridgeSubsystem`).

`bAlwaysRelevant`는 `IsNetRelevantFor`에서 **가장 먼저** 검사되므로, 숨김(`bIsRevealed=false`)
상태의 적도 확실히 복제된다.

### 3.1 성능 — 거의 안 든다 (사용자 질문)

| 축 | 평가 |
|---|---|
| 대역폭 | `NetServerMaxTickRate=30`(BaseEngine.ini)이 서버 복제를 30 Hz로 묶는다 — 병사 `NetUpdateFrequency=100`은 사실상 못 쓴다. 25명 × 30 Hz × ~40 B ≈ **30 KB/s**. 클라 상한 100 KB/s(`Client netspeed is 100000`) 안이고, 포화해도 엔진이 거리 우선순위(`FActorPriority`)로 늦출 뿐 깨지지 않는다. 1 Gbps LAN에선 무의미 |
| 서버 CPU | 관련성 판정은 어차피 전 액터를 돌고 있었다. 늘어나는 건 액터 채널 ~25개 분의 복제뿐 |
| 클라 CPU | 유일한 실비용 — 클라가 이제 병사 25명의 보간+애니메이션을 실제로 돌린다. **원하던 것**이고, solo가 이미 한 프로세스에서 전부 돌리며 잘 도는 걸 확인한 부하다 |
| 클라 AI | 없음 — `Pawn.cpp:145`가 `NM_Client`에서 AI 컨트롤러를 안 붙인다 |

여유가 더 필요하면 병사 `NetUpdateFrequency`를 100 → 30으로 낮추는 게 무해한 다음 수다
(5.8은 `SetNetUpdateFrequency()`로만 변경 가능). 이번엔 일부러 안 건드렸다.

---

## 4. 같은 축의 별건 — 클라에서 `ToggleDebugCamera`가 "Command not recognized"

UGV 호스트와 자체방호 Solo에서는 되는데 **클라에서만** 콘솔 명령이 없는 것으로 나왔다.

`APlayerController::AddCheats`는 치트 매니저를 이 조건에서만 만든다:

```cpp
if ((World->GetAuthGameMode() && World->GetAuthGameMode()->AllowCheats(this)) || bForce)
```

그리고 `AGameModeBase::AllowCheats`(`GameModeBase.cpp:1413`)는

```cpp
return (GetNetMode() == NM_Standalone || GIsEditor);
```

**클라이언트에는 AuthGameMode 자체가 없다** → 조건이 통째로 false → `UCheatManager`가 안 생기고,
그 exec 명령(`ToggleDebugCamera` 등)이 존재하지 않는 명령이 된다. Solo는 `NM_Standalone`,
PIE 호스트는 `GIsEditor`라 둘 다 통과했던 것.

**수정**: `Atitan_examplePlayerController::BeginPlay`에서 로컬 컨트롤러면 `EnableCheats()`
(비-쉬핑 한정. 쉬핑 빌드는 엔진이 `UE_WITH_CHEAT_MANAGER`로 치트 매니저를 통째로 뺀다).

> **덤**: 패키징 **호스트**도 Standalone도 에디터도 아니라 원래 같이 막혀 있었다. 이 수정으로
> 같이 풀린다.

---

## 5. 검증 (2026-09-23, 2대 PC 실환경)

풀·데모 양쪽에서 클라 접속 — 드론 화면에 교전·총구 화염·사격음·탐지 박스가 모두 정상
표시됨을 사용자 확인("해결. 아주 잘됨"). `ToggleDebugCamera`도 클라에서 동작.

관련 관찰 하나: 3분대가 이동형지휘소 근처(150 m 안)까지 오면 관련성 안으로 들어와 **그
구간만 멀쩡히 보였다** — 진단을 확인해 준 증상이다. 2차 전투지(~250 m)는 경계 바로 밖이라
애매하게 보였다.

---

## 6. 남은 것 / 미해결

**PIE 로비 경유 시 드론 화면의 나무 라이팅·팝핑** — 로비에서 Client/Solo로 들어가 레벨이
뒤늦게 로드되면 드론이 빠르게 이동할 때 나무/지형이 로딩되는 것처럼 불안정해 보인다. 패키징
빌드에서는 안 난다.

이건 **관련성과 무관하다**(식생은 복제 대상이 아니다). 다만 성질은 같다 — "씬캡쳐는 엔진의
여러 뷰 의존 시스템이 아는 시점이 아니다". 텍스처 스트리밍은 이미 캡쳐를 `IStreamingManager`에
뷰어로 등록해 막아뒀고, 남는 후보는 Lumen 서피스 캐시/VSM 워밍업, PIE 한정이라면 셰이더(PSO)
컴파일·에셋 로드다. **원인 미확정 — 계측 전이라 단정하지 않는다.** 전장 액터가 통째로 없던
것과 겹쳐 보였을 수 있으니, 이번 수정 뒤 남는 증상으로 다시 판단할 것.
