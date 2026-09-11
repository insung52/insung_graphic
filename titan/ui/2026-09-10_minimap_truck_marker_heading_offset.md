# 미니맵 트럭 방향 화살표 90도 회전 버그

2026-09-10 / 완료 / BP_TitanTruck 재구성으로 낡아버린 `TruckMeshForwardOffsetDeg=-90` 중복 보정을 제거.

## 증상

미니맵 오버레이에서 **RCWS 방향/시야각(FOV 콘) 마커는 정상**인데, **트럭 방향 화살표
(`MinimapTruckMarker`)만 반시계 방향으로 90도 돌아가 있음**.

## 원인

`Monitor1Widget::RefreshMinimap()`에 2026-08-02에 넣어둔 고정 보정이 남아 있었음:

```cpp
const float TruckMeshForwardOffsetDeg = -90.f;
const float ScreenAngle = WorldYawToMinimapScreenAngle(TitanTruckRef->GetActorRotation().Yaw + TruckMeshForwardOffsetDeg);
```

그런데 그 사이 **BP_TitanTruck의 컴포넌트 구성이 바뀌었다.** 2026-09-10에 라이브 프로퍼티로
재확인한 값(`BP_TitanTruck_C_4`, New_kadex_0811):

| 컴포넌트 | 상대 Yaw | 상대 위치 | 2026-08-02 당시 |
|---|---|---|---|
| `BodyMesh` | **270** (= -90) | (0, 0, 0) | (기록 없음) |
| `RCWSMount` | **0** | (70, 0, 327) | **-90** |
| `FrontCineCamera` | **0** | (**+X** 310, 0, 120) | **-90** |

즉 예전엔 마운트/카메라 쪽에 붙어 있던 -90이 지금은 `BodyMesh`로 옮겨가 있다. 메시 에셋
(`Titan_Truck`) 자체의 정면이 로컬 **+Y**라서, `BodyMesh`만 상대 Yaw 270으로 돌려 시각적 정면을
**액터 로컬 +X**에 맞춰둔 구조다. `FrontCineCamera`가 상대 Yaw 0으로 **+X 310** 위치에 붙어 있는
것이 "진짜 정면 = 액터 +X"의 교차 검증(정의상 트럭의 실제 정면을 보는 카메라).

따라서 `GetActorRotation().Yaw`가 이미 시각적 정면과 일치하고, 남아 있던 -90이 **중복 보정**이
되어 마커만 반시계로 90도 돌아간 것.

**FOV 콘이 멀쩡했던 이유**: `URCWSComponent::GetSightWorldRotation()`이 마운트의 부착 체인을
전부 풀어서 월드 회전을 구하므로, BP 구조가 어떻게 바뀌든 항상 올바른 값이 나온다. 트럭 마커는
틀렸는데 콘은 맞는 증상이 보이면 **항상 "액터 정면 기준점" 쪽부터 의심할 것.**

## 수정

세 파일에서 오프셋 변수 제거(동일 코드가 복사되어 있었음):

- `Source/titan_example/UI/Monitor1Widget.cpp` — `RefreshMinimap()`. 재발 방지용으로 위 경위를
  주석에 전부 기록("여기에 ±90 오프셋을 다시 넣지 말 것").
- `Source/titan_example/UI/SelfDefenseMonitor1Widget.cpp` — `RefreshMinimap()`.
- `Source/titan_example/UI/SelfDefenseDashboardWidget.cpp` — `RefreshMinimap()`.

UGV/UAV 마커는 원래부터 보정 없이 `GetActorRotation().Yaw`를 쓰고 있어서 변경 없음.

## 남은 것 / 주의

- **빌드·인게임 확인 미실시** — 사용자가 직접 빌드해서 확인 예정. 검증 방법은 2026-07-21 나침반
  버그 때와 동일: 트럭을 실제로 돌려보고 미니맵 마커가 같은 방향으로 도는지 확인.
- 같은 보정이 세 파일에 복사되어 있는 구조 자체는 그대로 뒀다. BP 구조가 또 바뀌면 세 곳을 다시
  같이 고쳐야 한다 — 공통 헬퍼로 뽑는 것은 별도 작업.
- `guide/ui_dev_guide.md`의 `MinimapTruckMarker` 행에 "보정 없이 `GetActorRotation().Yaw` 사용"을
  명시해 두었음(에버그린 레퍼런스 갱신).
