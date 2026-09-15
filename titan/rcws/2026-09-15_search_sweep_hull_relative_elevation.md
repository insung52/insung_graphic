# 탐색 스윕 고각을 차체 기준으로 — 내리막에서 포탑이 하늘 보던 문제

2026-09-15 / 완료(빌드·실차 확인 대기) / 자동정찰 스윕의 고각 목표를 "월드 수평 0°"에서 "차체 기준 `SearchSweepElevationDegrees`(-3°)"로 바꿈.

## 현장 피드백

UGV RCWS 자동정찰(탐색 스윕) 중 **내리막길에서 포탑이 너무 위를 본다**. 사용자 결정:
고각도 좌우 스윕처럼 **차체 기준**으로 통일하고, 약간 아래를 보게(처음 -5°로 넣었다가 같은 날
**-3°**로 확정).

## 원인

`URCWSFireControlComponent::UpdateSearchSweep`(`Source/titan_example/Vehicles/RCWSFireControlComponent.cpp`)이
고각 목표를 `CurrentData.ElevationDegrees == 0`으로 잡고 있었다. 그런데 이 값은 2026-07-20부터
`URCWSComponent::RefreshAzimuthElevation`이 `SightCamera->GetComponentRotation().Pitch`, 즉
**월드 수평 기준** pitch로 계산한다. 내리막에서 차체가 앞으로 기울면 스윕이 월드 수평을
유지하려고 마운트를 차체 대비 위로 들어 올리고, 결과적으로 하늘을 본다.

좌우 스윕은 원래부터 차체 기준이었다(`Owner->GetActorRotation().Yaw + SearchSweepOffsetDegrees`) —
상하만 기준이 달랐던 것.

## 변경

`RCWSFireControlComponent.h` — 새 프로퍼티:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Control|Search Sweep")
float SearchSweepElevationDegrees = -3.f;   // 차체(마운트 베이스) 기준 도, 음수 = 아래
```

`RCWSFireControlComponent.cpp` `UpdateSearchSweep` — 고각 오차를
`SearchSweepElevationDegrees - HullRelativeSightPitch`로 계산. `HullRelativeSightPitch`는
**조준 카메라의 월드 회전을 차체 회전으로 되돌린 값**:
`(Owner->GetActorQuat().Inverse() * RCWS->GetSightWorldRotation().Quaternion()).Rotator().Pitch`.
레이트 제한은 기존대로 `SearchSweepSpeedDegPerSec`.

> ⚠️ **1차 구현(CL 476)은 마운트의 relative pitch**(`RCWS->GetMount()->GetRelativeRotation().Pitch`)를
> 기준으로 썼는데, **TitanTruck에서 카메라가 너무 아래를 보는 문제**가 나왔다. 원인은 트럭 BP의
> `RCWSSightCineCamera`가 `RCWSMount` 아래에 **자체 relative pitch -6.1°**를 갖고 있어서(UGV는 0),
> 마운트를 -3°에 맞추면 카메라는 -9.1°를 보게 된 것. 그래서 같은 날 기준을 "마운트"에서
> "카메라 조준선 자체"로 바꿨다 — 마운트/카메라 사이 오프셋이나 마운트 yaw 오프셋(구 UGV 180°)에
> 상관없이 "카메라가 차체 대비 몇 도를 보는가"가 곧 목표값. 조작은 그대로 `AddPanTiltInput`
> (마운트 pitch에 델타 누적)이고 카메라가 마운트 자식이라 1:1로 따라오므로 오차→델타 관계는
> 변함없음.

`UpdateSearchSweep` 호출처 둘(AutoSurveillance 모드, AutoAim/AutoFire 무표적 시) 모두에
적용되며, UGV와 이동형지휘소(트럭) RCWS에 똑같이 영향.

## 튜닝값

- `SearchSweepElevationDegrees` 기본 **-3°**. 경사와 무관하게 차체 앞쪽 지면을 약간 내려다봄.
- 반드시 `MinElevationDegrees(-20)` / `MaxElevationDegrees(60)` 안쪽이어야 함 — 마운트 기계적
  한계이고 이것도 차체 기준. 단 이 클램프는 **마운트** relative pitch에 걸리므로, 트럭처럼
  카메라 오프셋(-6.1°)이 있는 구조에선 조준선 기준으로는 그만큼 어긋나 보인다(-3° 목표면
  마운트는 +3.1° — 한계와는 멀어서 실질 문제 없음).
- 새 프로퍼티라 레벨 인스턴스에는 C++ 기본값이 자동 반영됨. 차량별로 다르게 두려면
  BP/인스턴스에서 오버라이드.

## 알려진 것 / 안 건드린 것

- RCWS 안정화(`SetRCWSStabilization`, 기본 OFF)는 매 틱 차체 pitch를 상쇄하므로 켜면 이 목표와
  서로 싸운다 — 하지만 좌우 스윕도 원래 같은 식으로 싸우던 기존 구조라 이번엔 손대지 않음.
- 같은 두 파일이 Perforce에서 `user2@user2_jiseong`에게도 동시에 체크아웃되어 있음 — 서브밋 시
  머지 가능성.

## 남은 확인

- **빌드**(사용자 직접) 후 실차 경사로에서 내리막/오르막 모두 포탑이 차체 기준 -3°를 유지하는지
  확인.
- 관련 가이드: `guide/rcws_fire_control_dev_guide.md` §8.4 갱신함.
