# Perforce 서버 이전

2026-10-01 / 완료(확인, ★ 2026-10-07 재확인·보강) / P4 서버·워크스페이스가 바뀜(`ssl:p4d.grapicar.com:1666`, `grapicar4`, `insung_desktop1`, `//dev/main`). 이전 직후 로컬 수정 파일이 "안 열린 채 변경 + read-only"가 된 것을 `p4 diff -se` → `p4 edit`로 복구. **10-07 보강: 옛 주소를 `-p`/`-c` 로 강제하면 라이선스/세션 에러로 오진하게 된다 — 오버라이드 없이 `p4 edit` 할 것**(마지막 절).

## 현재 접속 정보 (2026-10-01 확인)

| 항목 | 값 |
|---|---|
| `P4PORT` | `ssl:p4d.grapicar.com:1666` |
| `P4USER` | `grapicar4` |
| `P4CLIENT`(워크스페이스) | `insung_desktop1` |
| 스트림 | `//dev/main` |

옛 워크스페이스 **`user4_DESKTOP-81S78B2_4340`은 무효**다. 2026-10-01 이전 devlog(예: `level_new_kadex_0811/2026-09-22_scenario_restart_implementation.md`, `soldier_ai_lab/` 일부)에 "이 워크스페이스에 체크아웃·미제출"로 적힌 내용은 그 시점 기록이다 — devlog는 고치지 않으므로, 지금 기준 열린 파일은 새 워크스페이스에서 다시 확인할 것.

## 이전 직후 증상과 처리

- 증상: 이전 전에 로컬에서 고치던 파일이 새 워크스페이스에서 **열리지 않은 채(not opened) 내용만 바뀐 상태 + read-only 속성**이 됨 — 이 상태로는 Edit 툴 쓰기가 막히고, 제출 대상에도 안 잡힌다.
- 처리: `p4 diff -se`로 "열리지 않았는데 depot과 다른" 파일을 찾아 `p4 edit`. 로컬 내용은 보존된다(`p4 edit`는 파일을 덮어쓰지 않고 열기만 함).
- 프로젝트 규칙대로 read-only를 셸로 풀지 말고 P4 체크아웃으로 해결할 것(`titan_example/CLAUDE.md`).

## ★ 2026-10-07 재확인 — `-p`/`-c` 오버라이드를 붙이지 말 것

옛 주소를 명령줄로 강제하면 **옛 서버에 붙는다.** 두 실패 모드가 전혀 다른 문제처럼 보여서 오진하기 쉽다:

| 호출 | 결과 | 실제 원인 |
|---|---|---|
| `p4 -p 192.168.0.99:1666 -c user4_DESKTOP-81S78B2_4340 edit …` | `Can't create a new user - over license quota` | 옛 서버에 `grapicar4` 가 없어 **유저 생성을 시도**한다 |
| 위 + `-u user4` | `session has expired` | 옛 서버의 그 유저 세션이 만료됨 |

**해법은 아무것도 붙이지 않는 것이다** — 현재 설정이 `p4 set` 에 이미 잡혀 있으므로 `p4 edit <경로>` 로 바로 열린다.

보강된 접속 정보(2026-10-07 `p4 info` 실측):

| 항목 | 값 |
|---|---|
| `P4PORT` | `ssl:p4d.grapicar.com:1666` |
| **`p4 info` 의 `Server address`** | **`p4dtest:1666`** — 서버가 자기 내부 주소를 보고하는 것이라 `P4PORT` 와 달라 보이는 게 **정상** |
| 서버 버전 | P4D/LINUX 2026.1 |
| `P4USER` | `grapicar4` |
| `P4CLIENT` | `insung_desktop1` |
| **클라이언트 루트** | **`C:\working\works`** |
| **depot 경로** | **`//dev/main/kadex/titan_example/...`** |

확인 경위: `soldier_ai_lab/ai/2026-10-07_shot_spread_wiring_and_muzzle_gate.md` 6.1절(SoldierLab 소스·BP 체크아웃 중 메모리에 남아 있던 옛 주소로 먼저 시도했다가 위 두 에러를 봤다).
