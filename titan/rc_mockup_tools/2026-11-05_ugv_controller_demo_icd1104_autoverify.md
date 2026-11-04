# ugv_controller_demo ICD 1104 반영 + 11/04 피드백 자동 검증(verify_1104)

2026-11-05 / 완료(코드·가짜 UE dry run · 실제 UGV축 PIE 실행 PASS 30/FAIL 0) / 데모를 ICD 1104·B4·B5에 맞추고, B1·키·B5·B4·제어권 정리·UE 로그를 한 번에 PASS/FAIL로 내는 `tools/verify_1104.py` 추가.

관련: 인계 `documents/feedback_1104/2026-11-05_handoff.md` §2(이 도구가 2~6번을 자동화), ICD 반영 `protocol/2026-11-04_icd_1104_and_selfdefense_report.md` §1·§7,
팀장 점검 `documents/feedback_1104/ICD_정합성_수정사항.md`. UE 정본 = `Source/titan_example/Network/UGVRemoteControlSubsystem.{h,cpp}`(이번 작업에서 수정 안 함).

---

## 0. 위치 / P4

- 코드: `C:\working\works\kadex\ugv_controller_demo` (git 저장소). **P4 depot에 없던 폴더**라 사용자 결정으로 이번에 `p4 add` — **Change 528**(`insung_desktop1`, 미제출) 에 폴더 전체 15개 파일이 add로 열려 있음(.p4ignore 적용: `.git/`, `__pycache__/`, `*.log`, 신규 `verify_reports/`).
  기본 changelist에 다른 작업(SoldierLab_RL)이 열려 있어서 섞지 않으려고 번호 CL을 따로 만들었다.
- 패키징 복사본 `C:\working\outputs\kadex_1015_packaging\ugv_controller_demo`는 안 건드림.

## 1. 데모 수정 (ICD 1104 반영)

| 파일 | 변경 |
|---|---|
| `net_client.py` | 수신 키 정규화 `normalize_icd_keys` — 옛 `ConrtrolRight`/`Batterry`/`Latter`가 오면 `ControlRight`/`Battery`/`Letter`로 옮기고 경고 1회(예전 UE 빌드 대비 폴백). `add_listener(fn(env, t))` — 수신 원본(정규화 전)+수신 시각을 넘김(검증기용). 머리 주석 갱신(옛 철자 설명 제거) |
| `joystick_control.py` | `RC_Movement`: 20Hz 매번 재송신 → **값 변화 시 즉시 + 1초마다 같은 값 재송신**(`MOVEMENT_KEEPALIVE_S`). UE가 속도 지령으로 적분하므로 재송신 주기는 속도에 영향 없음(B5). 재송신은 UDP로 "0" 1건이 유실되면 포탑이 계속 도는 것 방지용. 조이스틱 비활성화 시 중립 1건. 타이머 PreciseTimer |
| `keyboard_driving.py` | `RC_RemoteDriving` 20Hz 유지 + PreciseTimer(UE 0.5초 타임아웃, B4). 비활성화 시 정지 지령 1건. "UE엔 데드맨이 없다"는 옛 주석 정정 |
| `main_window.py` | `RC_Request_BIT` 1Hz 주기 송신(ICD 주기 메시지인데 예전엔 연결 버튼에서 1회만). **"제어권 반납 (STAY)" 버튼 + 창 닫을 때 자동 정리**(중립 → `OperationMode STAY` → `ControlRight 0`). RCWS 패널에 Azimuth/Elevation(보정 고각 같이)/TotalBullet 표시. "시동" 버튼 라벨 → "RCWS 조향 비활성화/활성화"(`RC_ActivateMovement`는 08-31부로 RCWS 조향 게이트) |
| `README.md` | 위 내용 + 자동 검증 사용법 |

새 필드(`Azimuth`/`Elevation`/`TotalBullet`/`Runtime`)는 상태 패널이 data 전체를 일반 표시하던 구조라 원래도 보였고, 수신 로그(INFO)에도 data 그대로 찍힌다.

## 2. 자동 검증 `tools/verify_1104.py`

### 실행

```
cd C:\working\works\kadex\ugv_controller_demo
python tools/verify_1104.py --ugv-ip 127.0.0.1                 # 전부, 약 1분 30초
python tools/verify_1104.py --ugv-ip 127.0.0.1 --skip b4       # 차를 안 움직임
python tools/verify_1104.py --ugv-ip 127.0.0.1 --only b1,keys  # 제어권 안 잡고 수신만
python tools/verify_1104.py --log-only --ue-log <경로>         # 자체방호 PC 로그(K05.1/F1)만
```

- **GUI 데모를 끄고** 돌릴 것(둘 다 8010/8011을 SO_REUSEADDR로 잡으면 Windows가 패킷을 한쪽에만 줌).
- UE는 RCIP(로비/Game.ini, 기본 192.168.10.20)로 송신 — 이 PC가 그 IP이거나 RCIP를 이 PC로.
- 출력: 콘솔 + `verify_reports/<시각>_verify_1104.txt/.json`(항목별 결과·판정 기준). FAIL이 있으면 종료 코드 1.
- 순서: 연결(RC_Connection, BIT 1Hz 시작) → B1(제어권 없이 15초) → 제어권+REMOTE → B5 → B4 → 정리 → KEYS(전 구간) → LOG.
  Ctrl+C로 중단해도 `finally`에서 정리(중립 → STAY → 반납)는 한다.
- 단계 선택 `--only`/`--skip` (b1,keys,b5,b4,log). 주요 인자: `--b1-seconds 15`, `--rate-tol 0.10`, `--expected-rate 60`, `--rcws-tol 0.15`, `--link-timeout 3`,
  `--drive-accel 40 --drive-seconds 3 --drive-gear FRONT`, `--b4-window 8`, `--b4-decel-max 1.5`, `--ue-log`(기본 `..\titan_example\Saved\Logs\titan_example.log`, 여러 번 가능).

### 검사 항목과 판정 기준

| ID | 내용 | 판정 |
|---|---|---|
| CONN | 연결 | 5초 안에 수신 1건 이상(없으면 FAIL + IP/RCIP/포트 안내). `UGV_Response_Connection` 없으면 WARN |
| B1 | `UGV_RCWS_Status`(13) 20 / `Basicinfo`(14) 10 / `NavigationInformation`(12) 10 / `BasicInformation`(12) 1 / `ObjectDetectionResult`(13·12) 20 Hz | 15초 창에서 (건수−1)/(마지막−첫 수신). 오차 ≤10% PASS, ≤25% WARN, 그 밖 FAIL. 최대 수신 간격도 표시. **UE fps가 목표 Hz보다 낮으면 그 fps가 상한(설계)** — 60fps에서 돌릴 것 |
| CTRL | 제어권 획득 | 3초 안에 Basicinfo `ControlRight=Remote` + `OperationMode=Remote` |
| B5-1 | `RC_Movement` X=100 **1건만** 송신 | 0.3~3.3초 Azimuth(언랩) 최소제곱 기울기 60±15 °/s, R²≥0.95. 5°/s 미만이면 "옛 동작(1건당 3°)" 표시. 부호(＋X→방위 증가/감소)는 정보로만 |
| B5-2 | X=50 1건 | B5-1 속도의 50±20 % |
| B5-3 | X=0 1건 | 0.25초 뒤 1.5초간 \|rate\| ≤ 2°/s |
| B5-4 | X=100 + `BrakeButton PRESSED` → RELEASE | PRESSED 동안 ≤2°/s, RELEASE 후 B5-1의 70% 이상 |
| B5-5 | `RC_ActivateMovement PRESSED` → RELEASE | PRESSED 동안 ≤2°/s. RELEASE 후 마지막 X=100으로 다시 도는 것은 UE 설계(값을 안 지움) — 정보 |
| B5-6 | 회전 중 **RC 송신 전부 중단**(BIT·RemoteDriving 포함) | 마지막 송신 → 마지막 Azimuth 변화가 2.6~3.6초, 끊긴 직후 2초는 B5-1의 70% 이상으로 계속 회전 |
| B5-7 | 송신 재개(BIT+RemoteDriving, RC_Movement 없음) | \|rate\| ≤ 2°/s (두절 때 저장값을 지웠는지) |
| B4-1 | Acceleration 40으로 3초 주행 후 `RC_RemoteDriving`만 중단(BIT는 계속) | 가속 중 최고 ≥3 km/h가 전제(못 움직이면 FAIL "판정 불가"). Basicinfo Speed가 끊을 때보다 1 km/h 떨어진 시각 ≤1.5초, 8초 안에 Speed 0이 0.5초 유지. Navigation Velocity 표시 |
| B4-2 | 재개 2초 | 최고 ≥3 km/h. 이후 Brake로 세움 |
| CLEAN | 중립 → STAY → `ControlRight 0` | 1.5초 뒤 Basicinfo `ControlRight=None`, `OperationMode=Stay` |
| KEYS | 전 구간 수신 메시지 전부 | Basicinfo: `ControlRight`∈{Remote,None}, `Battery`/`Speed` 정수 0~100, `ConrtrolRight`/`Batterry` 없음 · RCWS_Status: `RCWSStatus`∈{ON,OFF}, `Azimuth` 0~360, `Elevation` 0~180, `TotalBullet` 0~200 정수 · ObjectDetection(13·12): `TotalObject` 0~10 = len(Objects), 원소 `ObjectID` 1~65535, 박스 4좌표 0~65535, `Letter` 문자열, `Latter` 없음 · Navigation: 정수 6필드+`Letter` · BasicInformation: `Odometer`, `Runtime` `^\d{2,}-[0-5]\d-[0-5]\d$`. 위반 1건이라도 FAIL(규칙별 건수+첫 예시). 정수가 `12.0`처럼 오면 WARN |
| KEYS | Runtime 진행 | 역행 0회, \|Runtime 증가 − 수신 경과\| ≤ 2초 |
| LOG-B | 이번 실행 구간에 UE 로그가 새로 찍은 줄(시작 시 파일 크기 기억) | B5 "속도 지령 정지", B4 "원격주행 정지·브레이크", "원격주행 복구" 각 ≥1건. 없으면 WARN(다른 PC UE이거나 플러시 전) |
| LOG-K051 | 로그 전체의 `[SelfDefenseReport] K05.1` 줄 시각 | 평균 1.00±0.05초, 1.5초 초과 0회. 5초 넘는 간격 = 세션 경계로 제외, **프레임 번호가 ≤2 증가하며 1.5초 넘은 간격 = 게임 스레드 히치로 제외**(따로 표시) |
| LOG-F1 | `[TargetDetectionComponent]` + `렌더 화면비` 줄 | 0건 PASS, 있으면 FAIL(첫 줄 표시) |

UE 로그 줄 접두 `[YYYY.MM.DD-HH.MM.SS:mmm][프레임%1000]`의 시각은 **UTC**(오늘 07:39 KST 세션이 `2026.11.04-22.39`로 찍힘).

### 실행해 본 것 / 못 한 것

- ~~실제 UE(PIE/패키지) 대상으로는 한 번도 안 돌림~~ → **[2026-11-05 추가] 실제 UGV축 PIE에서 실행: PASS 30 / FAIL 0**
  (`python tools/verify_1104.py --ugv-ip 127.0.0.1`, 리포트 `C:\working\works\kadex\ugv_controller_demo\verify_reports\2026-11-05_075400_verify_1104.txt`).
  인계 `documents/feedback_1104/2026-11-05_handoff.md` §2 남은 검증 2~6번이 이것으로 끝났다. 기준값은 인자 조정 없이 기본값으로 통과.
  - B1: RCWS_Status·ObjectDetection 20.01Hz, Basicinfo·Navigation 10.01Hz, BasicInformation 1.00Hz.
  - B5: X=100에서 58.2°/s(R²=0.996), X=50에서 51%, 중립·Brake·ActivateMovement 정지, 송신이 끊기면 3.00초 뒤 정지.
  - B4: 끊기면 0.67초 뒤 감속 시작, 0.84초에 0 km/h. 재개하면 다시 주행.
  - 제어권 반납 정리, ICD 1104 키/범위(전 구간), Runtime 진행 PASS.
  - UE 로그: K05.1 평균 1.001초(최대 1.062초) — 같은 로그 파일에 남아 있던 자체방호 PIE 구간, F1 화면비 경고 0건.
  - (이전 기록) 첫 실행 때 기준값(특히 B4 감속 시작 1.5초, 최소 이동 3 km/h, B5-6 2.6~3.6초 창)이 실차 동역학/fps와 맞는지 확인 필요하다고 적었던 부분 — 위 결과로 확인됨.
- 가짜 UE(스크래치패드 스크립트, UE `TickPoll`/`TickRCWSMovement`/`TickRemoteDrivingWatchdog` 로직을 파이썬으로 흉내, 별도 포트 18000대)로 전체 흐름 dry run: **PASS 25 / FAIL 0 / SKIP 3(로그 없음)**, B5-6 정지 3.03초, B4 감속 0.63초. 옛 오타 키를 보내게 바꾼 가짜 UE에선 KEYS Basicinfo·ObjectDetection(13·12) 3건 FAIL로 정확히 잡음.
- `--log-only`를 이 PC의 현재 `titan_example.log`(오늘 아침 자체방호 PIE 2세션)에 돌린 결과: **K05.1 PASS**(165건, 평균 1.001초, 최대 1.183초, 히치 1건 3.6초 — 프레임 860→861, PIE 시작 직후), **F1 PASS**(0건).
- GUI 데모는 창을 띄워 보진 않음. 컨트롤러만 offscreen Qt로 스모크 테스트: RC_Movement 변화 시 1건 → 1.05/2.05초 keepalive → 비활성화 시 X=0, RemoteDriving 평균 49.4ms 간격.
- 탐지 대상이 화면에 없으면 Objects가 빈 배열이라 원소 키(`Letter` 등) 검사가 실질적으로 안 된다 — 리포트에 "(참고) 탐지 객체가 한 번도 없었음"으로 표시. RCWS 시야에 적/아군이 있을 때 돌릴 것.

## 3. UE ↔ 데모 차이 / 관찰 (UE는 안 고침, ICD에 없는 안전로직은 안 넣음)

1. **제어권 반납 시 UE는 DriveMode를 Idle로 둔다**(`ApplyEffectiveDriveMode`) — 예전 모드(Auto/로컬 Manual)로 되돌리지 않는다. 데모가 이제 반납은 하지만, 로컬 키보드 주행은 UE에서 다시 Manual로 바꿔야 함(PIE 재시작은 불필요).
2. **B5 링크 생존 = 어떤 RC 메시지든**(`PollSocket`이 `LastRCPacketSeconds` 갱신). 데모가 BIT 1Hz·RemoteDriving 20Hz를 보내는 동안엔 3초 정지가 걸리지 않으므로, 데모가 스틱 0을 안 보내면 포탑은 계속 돈다 → 데모가 비활성화/종료 시 중립을 보내도록 한 이유. 3초 정지는 RC 송신이 **전부** 끊길 때만(B5-6이 그렇게 검사).
3. **B4 워치독은 그 문맥에서 RemoteDriving을 한 번이라도 받은 뒤에만** 판정(`LastRemoteDrivingSeconds <= 0`이면 skip). 제어권만 잡고 RemoteDriving을 안 보내는 RC는 대상 밖 — 설계 그대로.
4. `RC_ActivateMovement`: 데모 버튼 라벨이 "시동"이었음(08-16 해석). UE는 08-31부로 RCWS 조향 게이트 → 라벨만 정정.
5. `RC_RemoteDriving.Brake`: ICD 범위 `-0~-100`, UE는 `!= 0`이면 on/off(정합성 C3 보류). 데모 키보드는 항상 0, 검증기는 세울 때 -100.
6. `Gear`: ICD 타입 Int인데 값은 `FRONT`/`BACK` 문자열(C4) — 데모·UE 모두 문자열. Basicinfo 보고값은 `Front`/`Back`(대소문자 다름, C5).
7. `recv` 장치 코드: 데모는 `RC_Movement`→13, `RC_ActivateMovement`→14로 보냄. UE는 recv를 검사하지 않음(D3).
8. `RC_FireMode`: 데모는 ICD 원문 `BRUST`/`CONTINUS` 유지(ICD에 그대로 남아 있음), UE는 정상 철자도 수용.
9. `RC_Request_BIT`: ICD 1Hz 주기인데 데모가 1회만 보냈음 → 수정. UE는 BIT 수신 유무로 아무것도 판정하지 않음(B3 보류).
10. ObjectDetectionResult는 UE 20Hz(S2 결정) — SQ 다이어그램은 10Hz. 검증기는 20Hz 기준.
11. UE 로그 시각은 UTC — 로그 판정에서 KST 시각과 비교하지 말 것.

## 4. 남은 일

- ~~실제 UE에서 첫 실행 → 기준값 확인(위 §2 "못 한 것").~~ 11/05 완료(PASS 30 / FAIL 0).
- §3-2(스틱 중립 시 `RC_Movement` 0 미송신 → 포탑 계속 회전)는 LIG 질문 후보 Q6으로 추가됨(`documents/feedback_1104/2026-11-04_lig_questions_feedback_1104.md` §A).
- CL 528 제출(사용자).
- 패키징 복사본(`kadex_1015_packaging`)에 반영할지는 별도 결정.
