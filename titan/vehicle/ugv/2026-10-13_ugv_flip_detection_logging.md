# UGV 전복 감지·로깅 (시나리오 모니터 + 블랙박스)

2026-10-13 / 완료(빌드·패키징됨, 실전 발동 미관측) / 시나리오 모니터가 UGV 기울기·회전속도를 0.25 s 로 보고 급격한 자세 변화·전복·회복을 주변 물체 목록과 함께 `events.log` 에 남기고, 전복 시 블랙박스 `ugv_flip` 보존본(A 등급)을 요청한다. 감지·기록만 — 원인 수정은 아님.

## 1. 계기

- 10/12 2-PC 15사이클 실행에서 UGV 가 **2차 목적지 도착·정렬 후 정지 상태에서** 13:18:33~13:19:07 사이에 뒤집혔다. 주변 적 C_12 가 4~7 m, 아군 집중사격 중, UGV 는 미사격.
- 그때 UGV 자세가 어디에도 기록되지 않아 시점·원인을 가릴 수 없었다. **원인은 미확정.**
- 원인 쪽(병사 캡슐 키네마틱 밀림 차단·자가 복원·무게중심)은 다른 세션 작업 — `vehicle/ugv/2026-10-12_ugv_flip_pawn_collision_and_self_righting.md`. 이 문서는 **감지·로깅만** 다룬다.

## 2. 시나리오 모니터 — `UScenarioMonitorSubsystem`

파일: `Source/titan_example/UI/ScenarioMonitorSubsystem.h/.cpp` — `PollUGVPosture()`, `DescribeUGVSurroundings()`. 기존 UGV 폴링(0.25 s) 끝에서 호출. 출력은 `Saved/ScenarioLogs/<세션>/events.log`.

| 항목 | 값(코드 상수, cpp 익명 네임스페이스) |
|---|---|
| 기울기 | 차체 위쪽 벡터와 월드 위쪽의 각(0 = 똑바로, 180 = 완전히 뒤집힘) |
| 회전 속도 | 직전 샘플 대비 쿼터니언 각거리 ÷ 경과 — 물리 각속도가 아님(클라에선 UGV 가 시뮬레이션되지 않아 0 이므로) |
| 급격한 자세 변화 | 회전 ≥ 45°/s **그리고** 기울기 ≥ 15°, 5 s 쿨다운, 전복 상태가 아닐 때만 |
| 전복 | 기울기 ≥ 60° 가 1.5 s 지속(이 레벨 최대 경사 ≈ 25° 와 안 겹치게) |
| 회복 | 전복 뒤 기울기 ≤ 30° 가 2 s 지속 |
| 자세 이력 | 최근 40 샘플(0.25 s × 40 = 10 s) 링 |
| 주변 물체 | 반경 8 m 구 오버랩(Pawn·PhysicsBody·WorldDynamic), UGV 자신·부착물 제외, 가까운 순 12개 |

### 남기는 줄

- **급격한 자세 변화**(`UGV`): `급격한 자세 변화 — 기울기, 회전 °/s, 속도 cm/s, 위치 | 주변: …` — 넘어가기 시작하는 순간이라 원인 물체가 전복 확정(1.5 s 뒤)보다 이 줄에 더 가까이 잡힌다.
- **전복 확정**:
  1. `FAIL` `UGV 전복 감지 — 기울기 (지속 s), 위치, 자율주행 중/아님, 드라이브 모드`
  2. `UGV` `자세 이력(최근 N샘플, 기울기/회전속도/속도): -9.8s:3°/0°s/0cm …`
  3. `UGV` `전복 시점 주변: …`
  4. `WriteWorldSnapshot("UGV 전복")` — 시나리오 로그의 월드 스냅샷 블록
  5. 블랙박스: `LogEvent("UGV", "UGV 전복 감지 — 기울기 N°")` + `RequestSnapshot("ugv_flip")`
- **회복**(`UGV`): `자세 회복 — 기울기 (전복 감지 N s 뒤, 재시작 리셋이면 순간이동 결과)`

주변 물체 항목 형식: `라벨/컴포넌트(거리 m, 생존|사망, 물리, v=속도)` — 병사면 `USoldierHealthComponent` 로 생존/사망, 시뮬레이션 중이면 `물리`(래그돌·떨어진 총 구분용).

### 재시작과의 관계

- `ResetPollState()` 에서 `bUGVPostureValid=false` — 재시작 리셋의 순간이동(`AUGV0901Pawn::ResetForScenarioRestart_Implementation`, `UGV0901Pawn.cpp:328-344`) 자세 차이를 급회전으로 읽지 않게 기준만 버린다.
- `bUGVFlipped` 는 남긴다 → 리셋으로 바로 서면 다음 폴링들에서 "자세 회복" 줄이 나온다.

## 3. 블랙박스 — `UTitanHealthRecorderSubsystem`

- `Diagnostics/TitanHealthRecorderSubsystem.cpp` `IsCriticalSnapshotReason()`(`:105-107`)에 **`ugv_flip` 을 A 등급(절대 삭제 안 함) 사유로 추가**. 보존본 = `HealthLogs/<세션>/snapshots/blackbox_<utc>_ugv_flip.bin` — 전복 직전 약 2분의 프레임 단위 입력·사격·부하.
- `health.csv` 맨 끝 UGV 자세 6열 `ugv_tilt_max, ugv_tilt, ugv_pitch, ugv_roll, ugv_angvel_max, ugv_speed_kmh`(매 프레임 샘플·1초 최대, UGV 없는 레벨·멈춤 줄은 빈칸)은 자가 복원 세션이 같은 날 추가한 것 — 위 10-12 문서 (d)절. 이벤트 기반 감지(이 문서)와 연속 기록(그쪽)이 보완 관계.
- 같은 시기 리눅스 RSS 분해 열도 추가됨(`diagnostics/2026-10-13_linux_rss_leak_vulkan.md`). 현재 헤더 끝부분(`TitanHealthRecorderSubsystem.cpp:511-513`):
  ```
  …,stalled_s,swap_used_mb,uobjects,proc_cpu_cores,load_avg1,rss_anon_mb,rss_file_mb,rss_shmem_mb,threads,
  ugv_tilt_max,ugv_tilt,ugv_pitch,ugv_roll,ugv_angvel_max,ugv_speed_kmh
  ```

## 4. 자가 복원과의 관계

이 작업 착수 시점(10/12)엔 전복 복구 fail-safe 가 없었으나(재시작 순간이동만), 같은 날 다른 세션이 `AUGV0901Pawn::UpdateSelfRighting()`(서버 전용, 기본 트리거 `SelfRightTriggerTiltDeg=45` + 바퀴 2/3 이상 비접지)을 넣었다. 따라서:

- 자가 복원 트리거(45°)가 이 감지(60°·1.5 s)보다 낮다 — 굴러 넘어간 경우엔 둘 다 나오고, 복원 성공 시 이 모니터의 "자세 회복" 줄이 뒤따른다. 회복 줄의 원인이 재시작 순간이동인지 자가 복원인지는 같은 `events.log` 의 `자가 복원 시작/완료(토크)/순간이동` 줄과 대조할 것.
- 이 모니터 자체는 차량을 건드리지 않는다(기록만).

## 5. 확인 방법 / 남은 것

- 장시간 실행 뒤: `events.log` 에서 `FAIL` `UGV 전복` 과 `급격한 자세 변화` 검색 → 주변 물체 목록으로 원인 후보 확인 → 같은 UTC 의 `snapshots/*_ugv_flip.bin` 을 `titan/diagnostics/tools/blackbox_decode.py` 로 풀어 직전 프레임 대조.
- 실전 발동은 아직 관측되지 않았다(빌드·패키징만 됨). 전복 재발 시 이 기록으로 원인 확정이 다음 단계.
