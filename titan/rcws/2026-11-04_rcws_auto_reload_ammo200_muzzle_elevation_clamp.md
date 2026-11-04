# RCWS 자동 재장전 · 탄약 200발 · 총구 고각 제한

2026-11-04 / 코드 완료, 빌드·검증 대기(사용자) / LIG 11/04 피드백 S4(R1~R3). 근거: `documents/feedback_1104/email.md`
(6. 기타), ICD 1104 시트2 D13. 레퍼런스: `guide/rcws_fire_control_dev_guide.md` §15.

## 요청 원문

- 6-1 `[UGV] RCWS의 상/하 각도 제어 시 각도의 제한 필요 (현재는 각도 제한이 없음)`
- 6-2 `[공통] RCWS의 모든 탄 소모시 자동으로 재장전(애니메이션이나 사용자의 입력이 없이 그냥 잔탄수량만 업데이트)`
- ICD 1104 `UGV_RCWS_Status.TotalBullet` (Int, "MAX 200발", 20Hz 상태 필드)

## 조사 결과 (코드 수정 전)

- **각도 제한은 이미 있었고 UDP 경로도 우회하지 않음.** `Handle_RC_Movement`(UGVRemoteControlSubsystem.cpp)
  → `URCWSComponent::AddPanTiltInput` → 마운트 relative pitch clamp(-20~60). 마운트를 돌리는 모든 호출자
  (조이스틱·안정화·자동조준·탐색 스윕·반동·재시작)가 같은 함수를 거친다. 실사용 UGV `BP_UGV_0901`·`BP_TitanTruck`
  uasset 이름 테이블에 `Min/MaxElevationDegrees` 없음 → C++ 기본값 그대로(에디터 확인은 미완).
- LIG가 "제한 없음"으로 본 원인 후보: ① 표시 고각은 수평 기준이라 경사에서 범위 밖 값 ② **총구는 탄도해로
  월드 회전을 따로 받아 제한 밖으로 쏠 수 있음** ③ +60°가 넉넉해서 영상상 무제한처럼 보임. 이 중 ②만 코드 결함.
- 탄약: `AmmoMax`는 UGV·트럭 모두 600이었음(주석의 "UGV 1200"은 발사속도 1200rpm과 섞인 낡은 기록).
  재장전 경로는 없었고 재시작 리셋만이 유일한 보충.
- 시나리오 재시작 소요 시간(R4, 코드 수정 없음): `Saved/ScenarioLogs` 657회 — 1160~1360 ms, 중앙값 1237 ms
  (페이드 0.3+0.5 s 포함).

## 변경

| 파일 | 내용 |
|---|---|
| `Vehicles/RCWSComponent.h/.cpp` | `AmmoMax` 600→200. `bAutoReload`(true)·`AutoReloadDelaySeconds`(1.5)·`IsAutoReloadPending()` 추가. `ConsumeAmmo`가 0에 닿으면 서버에서 타이머 → `CompleteAutoReload`(AmmoCurrent=AmmoMax). `ResetForScenarioRestart`·신규 `EndPlay`에서 타이머 해제. 장전/사격준비 상태 불변. 낡은 주석 정리 |
| `Vehicles/RCWSTypes.h` | `FRCWSStatusData::AmmoMax` 기본값 200, 주석 정리 |
| `Vehicles/UGVPawn.cpp`, `Vehicles/TitanTruck.cpp` | 생성자 `RCWS->AmmoMax = 200` |
| `Vehicles/RCWSFireControlComponent.h/.cpp` | `bManualRetriggerRequired` — 수동 방아쇠를 쥔 채 소진되면 놓았다 다시 눌러야 격발(자동사격 제외). 재장전 대기 중 `CanFire()` 실패 경고 억제. `UpdateMuzzleBallisticAim`: 목표 회전을 차체 기준으로 바꿔 pitch를 Min/MaxElevationDegrees로 clamp 후 월드로 복귀 |

설계 메모:
- 600발 기준 시나리오 의존은 `ResetForScenarioRestart` 주석의 "몇 사이클이면 탄이 말라 EnemyEngage 트리거가
  안 걸림" 하나뿐이었고, 자동 재장전으로 오히려 해소된다. 200발(연사 10 s)이라 한 사이클 안에서 재장전이
  일어날 수 있다 — 데모 자동사격은 1.5 s 끊긴 뒤 재개.
- UI·진단(`MissionDashboardWidget`, `Monitor1/2Widget`, `SelfDefense*`, `ScenarioMonitorSubsystem` "RCWS 탄 소진",
  `TitanHealthRecorderSubsystem` 발사 수 = 잔탄 감소분)은 값만 읽으므로 보충(증가)에 영향 없음.
- 총구 clamp로 제한 밖 표적은 `MuzzleBallisticElevationErrorDegrees`가 남아 락온 불가 — 자동사격이 제한 밖
  표적을 노리고 계속 대기할 수 있다(진단 로그 `MuzErr`로 보임).
- `CurrentData.ElevationDegrees`(수평 기준) 계산은 건드리지 않음(S1 `Elevation` 송신이 이 값+90 사용).

## 빌드 후 확인할 것

1. UGV·트럭 RCWS 탄약 표시 200/200(대시보드·Monitor2).
2. 연사로 200발 소진 → 로그 `탄 소진 — 1.5s 뒤 자동 재장전` → 1.5 s 후 `자동 재장전 완료 — 탄약 200/200`, 표시 갱신
   (2-PC면 클라 화면도). 장전 상태 표시 변화 없음.
3. 방아쇠를 쥔 채 소진 → 재장전 후에도 안 나감 → 놓았다 누르면 격발. LIG 통제기(UDP `RC_FireWeapon`)도 동일.
4. 데모 자동사격 중 소진 → 1.5 s 후 자동 재개.
5. 소진 직후(1.5 s 안) 시나리오 재시작 → 만탄, 이후 중복 재장전 로그 없음.
6. 경사에서 최저/최고 각으로 조준 후 사격 → 예광탄이 포신 방향(차체 기준 -20~60) 밖으로 안 나감.
   제한 밖 적에게 자동사격이 락온 안 하는지.
7. 통제기 `TotalBullet`(S1 구현 후) 0 → 200 복귀.

## 미룬 에디터 작업

- `BP_UGV_0901`·`BP_TitanTruck`(그리고 남아 있다면 `BP_UGV_Vehicle_new`)의 RCWS 컴포넌트 Details에서
  `AmmoMax`·`Min/MaxElevationDegrees`·`bAutoReload`/`AutoReloadDelaySeconds` 인스턴스 오버라이드 없는지 확인
  (있으면 200/-20/60/true/1.5로 맞추거나 리셋). uasset 문자열상 `BP_UGV_Vehicle*` 계열엔 `AmmoMax` 오버라이드 흔적 있음.
- 고각 범위는 디자인팀 답 오면 위 BP 값(또는 C++ 기본값)만 변경.
