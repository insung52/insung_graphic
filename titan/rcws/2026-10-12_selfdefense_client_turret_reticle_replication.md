# 자체방호 클라 RCWS 포신 레티클 고정 — 서버 총구 상태 복제

2026-10-12 / 완료(빌드·2-PC 검증 완료, 사용자 확인 2026-10-12) / 클라에서 포신 레티클이 화면에 고정되던 원인(총구 탄도 상태 미복제)을 서버 총구 상태 복제로 수정.

관련 코드: `Source/titan_example/Vehicles/RCWSFireControlComponent.h/.cpp`
(`UpdateMuzzleBallisticAim`, `UpdateTurretReticleForUI`, `OnRep_MuzzleRelativeRotation`, `GetLifetimeReplicatedProps`).
레퍼런스: `guide/rcws_fire_control_dev_guide.md` §6.1.

## 1. 증상

2-PC 풀 시스템(UGV PC = 리슨 서버 호스트, 자체방호 PC = 클라)에서 자체방호 통제기 RCWS 화면
(`USelfDefenseMonitor2Widget`의 `MainViewTurretReticleImage`)의 **포신 레티클이 화면 중심보다 살짝 위에
정적인 위젯처럼 고정**되어 전혀 움직이지 않음.

## 2. 의도된 동작

1. 카메라 중심 방향으로 레이트레이스(락온 중이면 타겟 위치) → 조준 목표 지점.
2. 그 지점에 맞도록 포탑 총구(`Muzzle`)의 상하각을 탄도(낙차)에 맞춰 슬루.
3. 레티클은 **"현재 총구 각도로 쏘면 어디에 맞는가"**의 예측점을 표시 — 화면 중심 고정이 아니라 슬루 중에는
   움직이다가 수렴한다.

## 3. 원인

`TickComponent`에서:

- `UpdateMuzzleBallisticAim` — `HasAuthority()`일 때만(서버) 실행.
- `UpdateTurretReticleForUI` — 모든 프로세스에서 실행(화면을 보는 쪽이 투영해야 하므로).

클라에서는:

1. `MuzzleAimTargetWorldLocation`이 복제되지 않아 항상 `ZeroVector` → 탄도 낙하 계산 대신
   `TurretReticleProjectionDistanceCm`(50000 cm = 500 m) 직선 투영 폴백으로 떨어짐.
2. `Muzzle` 컴포넌트 자체도 복제되지 않음(`BP_TitanTruck`에 컴포넌트 복제 플래그 없음) → 탄도 앙각이
   전혀 반영되지 않고 BP 기본 상대자세 그대로 마운트만 따라 돎.

결과적으로 카메라와 총구의 상대각이 상수 → 레티클이 화면상 한 점에 고정. 그 점이 중심보다 약간 위인
것은 Muzzle/카메라 배치 시차(총구가 사이트와 다른 위치에 있음) 때문.

## 4. 설계 판단

사용자 질문: "화면을 보는 클라가 계산해야 하지 않나?"

- **투영은 이미 클라가 한다**(`UpdateTurretReticleForUI`는 모든 프로세스에서 실행).
- 그러나 **발사는 서버**다 — `Fire` → `Multicast_FireEffects`가 서버의 총구 방향을 뿌리고, 클라 트레이서도 그
  방향으로 그려진다. 클라가 총구각을 따로 계산하면 "그려지는 총구 ≠ 쏘는 총구"로 갈라져 레티클과 트레이서
  사이에 오차가 생긴다.
- 트럭 RCWS 카메라도 클라 예측 없이 서버 값을 복제(`OnRep_MountRelativeRotation`)받는 구조다. 총구 상태를
  같은 경로로 복제하면 **카메라·레티클·트레이서가 모두 같은 서버 시점으로 일치**한다.

→ **방안 A: 서버 총구 상태 복제** 채택.

조작 지연 자체가 문제라면 그건 트럭 RCWS를 드론처럼 클라 권위로 바꾸는 별개의 큰 작업이다(이번엔 미채택).

## 5. 수정 내용

- `MuzzleAimTargetWorldLocation` → `UPROPERTY(Transient, Replicated)`. 클라에서도 탄도 낙하 시뮬레이션
  (`SimulateBallisticLandingPoint`)에 필요한 수평거리를 얻을 수 있게 됨.
- 신규 `ReplicatedMuzzleRelativeRotation` — `UPROPERTY(Transient, ReplicatedUsing = OnRep_MuzzleRelativeRotation)`.
  - 서버: `UpdateMuzzleBallisticAim`에서 `SetWorldRotation` 직후 Muzzle의 **상대** 회전을 기록.
  - 클라: `OnRep_MuzzleRelativeRotation`에서 `MuzzleComponent->SetRelativeRotation`. 서버는 덮어쓰지 않음
    (`OnRep_MountRelativeRotation`과 같은 방어).
  - 월드가 아니라 상대회전인 이유: 마운트 회전과 차체 자세가 각자 따로 복제되므로, 월드값을 보내면 도착
    시점이 어긋날 때 총구가 튄다. 상대값이면 부모 계층이 갱신되는 대로 자연스럽게 따라간다.
- `BeginPlay`: 초기 번들의 OnRep이 `MuzzleComponent` 해석보다 먼저 도착해 버려진 경우를 대비해, 권한 없는
  쪽에서 값이 있으면 1회 적용.
- `GetLifetimeReplicatedProps`에 두 프로퍼티 `DOREPLIFETIME` 추가.
- 자체방호 solo/데모(권한 있는 쪽이 직접 계산)는 기존 동작 그대로.

FRotator 복제 양자화(~0.0055°)는 1 km에서 약 10 cm 수준이라 무시 가능하다고 판단.

## 6. 남은 것

- [x] **정식 빌드** — 헤더에 UPROPERTY 추가라 Live Coding 불가, 에디터 닫고 빌드. (사용자 빌드, 2026-10-12)
- [x] **2-PC 검증** — 자체방호 클라에서 레티클이 슬루 중 움직이고 정지 시 화면 중심에 수렴하는지,
      트레이서 탄착과 일치하는지. (사용자 2-PC 확인 "잘 된다", 2026-10-12)
- [ ] 포신 메시가 Muzzle을 따라 움직이는 구조라면 클라 포신 비주얼에도 이제 앙각이 반영될 수 있음 — 미확인.
- [ ] P4 서브밋(`RCWSFireControlComponent.h/.cpp`).
