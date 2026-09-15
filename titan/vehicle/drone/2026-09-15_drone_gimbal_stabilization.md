# 드론 짐벌 2축 안정화 + 니어플레인 캡쳐 동기화

2026-09-15 / 완료 / 짐벌 각도의 기준 프레임을 기체 → 수평 프레임으로 바꿔 가감속 기울기가 카메라에 안 실리게 함(요·피치 2축, 롤은 상쇄 안 함). CineCamera의 Custom Near Clipping Plane이 씬캡쳐에도 복사되게 수정.

작업 경과 기록. "지금 어떻게 동작하는가"는 같은 폴더 `drone_flight_dev_guide.md` 12.4절 참고.

선행: `2026-09-10_drone_manual_control_split.md`(짐벌 기본 자세 복귀).

---

## 1. 요구

두 가지.

1. **니어플레인 / 기본 내려다보는 각도를 에디터에서 조절할 수 있나** — 각도는 `Gimbal|Home ▸
   Gimbal Home Pitch Deg`(지난주 추가)로 이미 됐고, 니어플레인은 반만 됐다(아래 5절).
2. **짐벌 안정화** — "처음 낙하산 위치로 날아갈 때 가속/감속 시 기체가 기울어지는 게 카메라에
   너무 잘 보인다." 실제 짐벌 리그는 `CamYaw`/`CamPitch` 본 둘뿐이라 **롤 본이 없다** — 사용자
   확인: 요·피치만 상쇄하면 되고 롤은 안 해도 된다. 항상 켜짐(토글 없음).

조건: 자동 추적과 충돌하지 않을 것 — "막 덜덜 떨리고 그러면 안 됨".

---

## 2. 기존 코드 분석 — 충돌 지점은 두 곳뿐

구현 전에 자동 추적/스윕/홈 복귀/수동 입력/리플리케이션이 짐벌 각도를 어떻게 쓰는지 훑었다.
전부 **`GimbalYawDeg`/`GimbalPitchDeg`라는 추상 각도 두 개만 읽고 쓴다.** 본에 실제로 넣는 건
`ApplyGimbalRotation()` 한 곳.

월드 방향 → 각도 변환은 딱 두 군데였다:

- `SlewGimbalTowardWorldLocation` — `GetActorQuat().UnrotateVector(ToTarget)`
- `TickGimbalCosmeticSweep` — 같은 패턴

여기서 **기체 좌표**로 바꾸기 때문에 각도가 "기체 기준"이었던 것이다. 즉 **각도의 기준 프레임을
바꾸면** 변환 2곳 + `ApplyGimbalRotation` 만 고치면 되고, 추적 로직은 코드 한 줄 안 바뀐다.

### 2.1 지금 화면이 출렁이는 원리

기체가 기울면 목표의 기체 기준 각도가 바뀐다 → 추적기가 슬루 속도 제한(`GimbalTrackSlewRateDegPerSec`)
안에서 뒤쫓는다 → 그 뒤처짐이 그대로 화면 흔들림. 수평 프레임이 되면 기체가 기울어도 목표 각도
자체가 안 변하므로 **추적기가 할 일이 줄어든다** — 데드밴드·감속 구간이 기울기 때문에 깨질 일이
없어진다. 새로 도입되는 떨림 원인은 없다.

---

## 3. 설계

### 3.1 기준 프레임 = 수평 프레임

```cpp
FQuat ADronePawn::GetGimbalReferenceQuat() const
{
    // 기체 요만 남기고 피치/롤을 뺀 "수평 프레임"
    return FRotator(0.f, GetActorRotation().Yaw, 0.f).Quaternion();
}
```

`GimbalYawDeg`/`GimbalPitchDeg`는 이제 이 프레임 기준이다: **요 0 = 기체 진행 방향, 피치 0 =
수평선.** 실제 짐벌의 "yaw follow" 모드와 같다(팬은 기체 헤딩을 따르고, 틸트는 수평선 기준).

### 3.2 본 각도는 매 틱 역변환해서 새로 뽑는다

```cpp
// ApplyGimbalRotation
const FVector DirLevel = FRotator(GimbalPitchDeg, GimbalYawDeg, 0.f).Vector();
const FVector DirBody  = GetActorQuat().UnrotateVector(GetGimbalReferenceQuat().RotateVector(DirLevel));
const float BoneYawDeg   = RadiansToDegrees(Atan2(DirBody.Y, DirBody.X));
const float BonePitchDeg = RadiansToDegrees(Asin(DirBody.Z));
// 이후는 기존과 동일 — Yaw*Rest, Yaw*Pitch*Rest 를 CS 절대값으로 SetBoneRotationByName
```

카메라 **전방 벡터**만 수평 프레임 목표에 정확히 맞추고, 롤은 기체 롤이 그대로 보인다 — 축이 둘인
실제 짐벌과 같다. 기체가 수평이면 `BoneYaw/Pitch == GimbalYaw/PitchDeg`라 기존 동작과 완전히 같다.

`CamYaw` 본은 기체에 그대로 얹혀 있고 `CamPitch` 본만 역보정된다 → 모델도 "헤드만 수평 유지"로
보인다.

### 3.3 자동 추적과의 통일 (핵심)

`SlewGimbalTowardWorldLocation` / `TickGimbalCosmeticSweep`의 `GetActorQuat()` →
`GetGimbalReferenceQuat()`. **이 둘을 안 맞추면 추적기가 기울기만큼의 가짜 오차를 쫓아서 오히려
더 떨린다.** 짐벌 각도를 만들거나 쓰는 곳은 전부 같은 프레임이어야 한다.

### 3.4 매 틱 적용

기체 자세가 바뀌면 명령 각도가 안 바뀌어도 본이 갱신돼야 한다. 주체 Tick의
`RefreshBoneTransforms()` 직전에 `ApplyGimbalRotation()`을 추가했다(원격 프로세스는
`TickRemoteInterpolation`이 이미 매 틱 부른다). Tick에서 읽는 자세·본 확정·캡쳐가 한 프레임 안에서
같은 값을 쓰므로 프레임 간 불일치 없음.

---

## 4. 의미가 바뀐 값 / 주의

전부 **수평선 기준**이 됐고, 오히려 의도에 맞는다:

| 값 | 전 | 후 |
|---|---|---|
| `GimbalHomePitchDeg` (-15) | 기체 기준 | 수평선 기준 — 기체가 기울어도 -15° 유지 |
| `Min/MaxGimbalPitchDegrees` (-80/45) | 기체 기준 | 수평선 기준 |
| `GimbalReconMaxPitchDegrees` ("수평선 위는 안 본다") | 기체 기준 | 진짜 수평선 |

> ⚠ **근수직 요 특이점.** 축이 둘이라 카메라가 거의 수직 아래(-80°)를 볼 때 기체가 크게 기울면
> 본 요가 수학적으로 크게 돌 수 있다(실물 2축 짐벌의 짐벌락과 같은 현상). 카메라 방향 자체는
> 맞아서 화면엔 문제 없고 모델의 짐벌 헤드가 홱 도는 정도. 지금 비행 프로파일(순항 기울기 십수 도,
> 관측 -45~-60°)에선 안 걸린다.

---

## 5. 니어플레인 — 캡쳐에 복사가 안 되고 있었다

BP `GimbalCineCamera` ▸ `Current Camera Settings ▸ (Advanced) Custom Near Clipping Plane` 은
**C키 Gimbal 뷰포트에만** 먹고 있었다. 자체방호 모니터/RTSP에 나가는 화면은 `GimbalCapture`
(SceneCapture2D)가 렌더하는데, `SyncGimbalLensFromCineCamera()`가 FOV/포스트프로세스만 복사하고
니어플레인은 안 넘겨서 캡쳐는 전역값(프로젝트 설정 `Near Clip Plane`, 기본 10cm)을 썼다.

```cpp
GimbalCapture->bOverride_CustomNearClippingPlane = ViewInfo.PerspectiveNearClipPlane > 0.f;
GimbalCapture->CustomNearClippingPlane = FMath::Max(ViewInfo.PerspectiveNearClipPlane, 0.f);
```

`UCineCameraComponent::GetCameraView`가 오버라이드를 켰을 때만 양수를 넣어주므로(안 켜면 -1),
CineCamera 값 하나로 뷰포트·위젯·RTSP가 전부 조절된다. 체크 안 하면 지금처럼 전역값.

---

## 6. 검증

2026-09-15 사용자 확인 — 낙하산 순항 가감속 구간에서 수평선 고정, 낙하산 관찰·교전 관측 전환 때
떨림 없음. 2 PC 실환경(`replication/2026-09-15_drone_two_pc_validation.md`)에서도 원격 프로세스가
같은 역변환으로 본을 그려 두 화면이 일치.
