# `FRtspAxisGate::ResolveLocalAxis` dangling 타이머 크래시 수정

2026-09-16 / 완료 / RTSP 축 게이트의 0.1초 폴링 타이머가 raw `this`를 캡처해, 5초 대기 중 오너가 파괴되면 크래시. `CreateWeakLambda(Owner, …)`로 수정, 호출부 5곳에 `Owner` 인자 추가. 리플레이 재생에서 발견됐지만 정상 플레이에도 잠재해 있던 버그. P4 체크아웃 완료, 빌드는 사용자.

> `FRtspAxisGate` 자체의 배경(왜 축 게이팅이 필요한가, `GetFirstPlayerController()` 함정)은
> `rtsp_integration_complete_0817.md` §5 참고. 이 문서는 그 위에 얹은 수정 1건만.

---

## 1. 크래시

- 덤프: `Saved/Crashes/UECC-Windows-B7DDF45542867B886DF7EE87DC4DA747_0000`
- `EXCEPTION_ACCESS_VIOLATION` @ `Vehicles/VehicleRtspBridgeComponent.cpp:29` — BeginPlay 람다의
  `GetOwner()->GetName()`, 호출자는 `Vehicles/RtspAxisGate.cpp:58`의 타이머 람다.

## 2. 원인

`ResolveLocalAxis`는 `Atitan_examplePlayerController::PlayerAxis`가 아직 리플리케이트 안 됐을
수 있어 **0.1초 간격, 최대 5초** 폴링 타이머를 건다. 이 타이머가 `this`(호출한 컴포넌트/폰)와
`&TimerHandle`을 **raw로 캡처**했다 — 오너가 5초 안에 파괴되면 타이머는 그대로 살아서 죽은
객체를 찌른다.

재현 조건(Chronicle 리플레이 재생, `replay_chronicle/2026-09-16_chronicle_replay_plugin.md` §3 함정 5):

1. 리플레이 재생 중 PC는 스펙테이터라 `PlayerAxis`가 영원히 `Unspecified` → 타이머가 5초 내내 돈다.
2. 그 5초 동안 슬라이더 무한 스크럽 루프가 데모 드라이버로 UGV를 반복 파괴/리스폰.
3. 파괴된 `VehicleRtspBridgeComponent`의 타이머가 발화 → dangling `this`.

**정상 플레이에서도 잠재** — BeginPlay 후 5초 안에 오너가 파괴되는 경우(레벨 전환, 시나리오
리셋, 클라이언트 접속 직후 액터 교체 등) 전부 같은 경로다.

## 3. 수정

`Source/titan_example/Vehicles/RtspAxisGate.h/.cpp`:

```cpp
// 전
static void ResolveLocalAxis(UWorld* World, FTimerHandle& TimerHandle, TFunction<void(EPlayerAxis)> OnResolved);
// 후
static void ResolveLocalAxis(UObject* Owner, UWorld* World, FTimerHandle& TimerHandle, TFunction<void(EPlayerAxis)> OnResolved);
```

타이머를 `FTimerDelegate::CreateWeakLambda(Owner, …)`로 바인딩 — `Owner`가 죽으면 타이머
람다가 조용히 스킵된다. `OnResolved`가 `[this]`를 캡처해도 `Owner`가 살아있을 때만 불리므로
안전. 헤더 주석에 2026-09-16 경위 기록.

호출부 5곳 전부 `this`를 `Owner`로 전달:

| 파일 | 오너 |
|---|---|
| `Vehicles/VehicleRtspBridgeComponent.cpp` | 컴포넌트(UGV) |
| `Drone/DronePawn.cpp` | 드론 폰 |
| `Environment/AmbientFXDirector.cpp` | 환경 FX 디렉터 |
| `Vehicles/TitanTruck.cpp` | 트럭 |
| `Vehicles/UAVPawn.cpp` | 구 UAV 폰 |

## 4. 상태

코드 수정 + P4 체크아웃 완료, **빌드/실행 확인은 사용자**. 리플레이 재생에서 재현되던 크래시는
이 수정으로 없어져야 하고, 정상 플레이 동작은 바뀌지 않는다(타이머 발화 조건·콜백 내용 동일).

## 관련 문서

- `rtsp_integration_complete_0817.md` §5 — `FRtspAxisGate` 도입 배경, `GetFirstPlayerController()` 함정.
- `replay_chronicle/2026-09-16_chronicle_replay_plugin.md` — 발견 경위(함정 5 스크럽 루프).
