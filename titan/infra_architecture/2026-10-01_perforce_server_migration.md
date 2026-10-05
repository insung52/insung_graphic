# Perforce 서버 이전

2026-10-01 / 완료(확인) / P4 서버·워크스페이스가 바뀜(`ssl:p4d.grapicar.com:1666`, `grapicar4`, `insung_desktop1`, `//dev/main`). 이전 직후 로컬 수정 파일이 "안 열린 채 변경 + read-only"가 된 것을 `p4 diff -se` → `p4 edit`로 복구.

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
