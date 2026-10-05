#!/usr/bin/env python3
"""블랙박스(blackbox.bin / snapshots/*.bin) → CSV 디코더.

형식 정본: titan_example/Source/titan_example/Diagnostics/TitanBlackboxTypes.h (Version 1).
구조가 바뀌면 HEADER_FMT / RECORD_FMT / RECORD_FIELDS 를 같이 고칠 것.

사용:
    python blackbox_decode.py <blackbox.bin> [-o out.csv] [--events events.log]

--events 를 안 주면 같은 세션 폴더의 events.log 를 찾아 KEY 번호를 키 이름으로 바꾼다
(snapshots/ 안의 파일이면 한 단계 위 폴더). 레코드는 프레임 번호 순으로 정렬해 쓴다 —
링은 순환 기록이라 파일 순서 ≠ 시간 순서.
"""
import argparse
import csv
import datetime as dt
import os
import re
import struct
import sys

MAGIC = 0x58424254  # "TBBX"
HEADER_FMT = "<IIIIIIQqiI64s104s40s"
RECORD_FMT = "<QqffffffffIIIIIIHHHHHHHHBB14s"
RECORD_FIELDS = [
    "frame", "utc_ticks", "world_s", "delta_ms", "game_ms", "render_ms", "rhi_ms", "gpu_ms", "game_wait_ms",
    "joy_axis_delta", "ram_avail_mb", "ram_proc_mb", "vram_used_mb", "vram_budget_mb", "flags", "actors",
    "keys_down", "keys_up", "last_key", "shots_soldier", "shots_rcws", "soldiers_h", "soldiers_f",
    "scenario_cycle", "net_mode", "net_conns", "_reserved",
]
FLAG_NAMES = [
    (1 << 0, "HITCH"), (1 << 1, "ERRLINE"), (1 << 2, "LOADING"), (1 << 3, "GPU_CRASHED"),
    (1 << 4, "VRAM_WARN"), (1 << 5, "RAM_WARN"), (1 << 6, "NO_WORLD"),
]
NET_MODES = {0: "Standalone", 1: "DedicatedServer", 2: "ListenServer", 3: "Client"}
TICKS_EPOCH = dt.datetime(1, 1, 1)

assert struct.calcsize(HEADER_FMT) == 256
assert struct.calcsize(RECORD_FMT) == 104


def ticks_to_utc(ticks):
    return TICKS_EPOCH + dt.timedelta(microseconds=ticks // 10)


def cstr(raw):
    return raw.split(b"\0", 1)[0].decode("utf-8", "replace")


def load_key_names(events_path):
    names = {}
    if not events_path or not os.path.exists(events_path):
        return names
    pattern = re.compile(r"\|\s*KEY\s*\|\s*(\d+)\s*=\s*(\S+)")
    with open(events_path, encoding="utf-8-sig", errors="replace") as f:
        for line in f:
            m = pattern.search(line)
            if m:
                names[int(m.group(1))] = m.group(2)
    return names


def guess_events_path(bin_path):
    folder = os.path.dirname(os.path.abspath(bin_path))
    for candidate in (folder, os.path.dirname(folder)):
        path = os.path.join(candidate, "events.log")
        if os.path.exists(path):
            return path
    return None


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("bin")
    ap.add_argument("-o", "--out")
    ap.add_argument("--events")
    args = ap.parse_args()

    with open(args.bin, "rb") as f:
        data = f.read()
    if len(data) < 256:
        sys.exit("파일이 너무 작다(헤더 없음)")

    (magic, version, header_size, record_size, capacity, _r0, write_count, start_ticks, utc_offset_min,
     _r1, session_id, build, _r2) = struct.unpack_from(HEADER_FMT, data, 0)
    if magic != MAGIC:
        sys.exit("블랙박스 파일이 아니다(매직 불일치)")
    if version != 1 or record_size != struct.calcsize(RECORD_FMT):
        sys.exit(f"지원하지 않는 버전/레코드 크기: v{version}, {record_size} B — 디코더 갱신 필요")

    records = []
    available = (len(data) - header_size) // record_size
    for i in range(min(capacity, available)):
        values = struct.unpack_from(RECORD_FMT, data, header_size + i * record_size)
        rec = dict(zip(RECORD_FIELDS, values))
        if rec["frame"] == 0 and rec["utc_ticks"] == 0:
            continue  # 아직 안 쓴 칸
        records.append(rec)
    records.sort(key=lambda r: r["frame"])

    key_names = load_key_names(args.events or guess_events_path(args.bin))
    out_path = args.out or os.path.splitext(args.bin)[0] + ".csv"

    print(f"세션 {cstr(session_id)} / {cstr(build)}")
    print(f"시작 UTC {ticks_to_utc(start_ticks)} (로컬 = UTC {utc_offset_min:+d}분), 용량 {capacity}, 누적 기록 {write_count}")
    if records:
        first, last = records[0], records[-1]
        print(f"담긴 구간 F{first['frame']} {ticks_to_utc(first['utc_ticks'])} ~ F{last['frame']} {ticks_to_utc(last['utc_ticks'])} ({len(records)} 프레임)")

    columns = ["utc", "local", "frame", "frame_mod1000"] + [c for c in RECORD_FIELDS if c not in ("frame", "utc_ticks", "_reserved")] + ["last_key_name", "flag_names", "net_mode_name"]
    with open(out_path, "w", newline="", encoding="utf-8-sig") as f:
        w = csv.writer(f)
        w.writerow(columns)
        for r in records:
            utc = ticks_to_utc(r["utc_ticks"])
            local = utc + dt.timedelta(minutes=utc_offset_min)
            row = {
                "utc": utc.strftime("%Y.%m.%d-%H.%M.%S:") + f"{utc.microsecond // 1000:03d}",
                "local": local.strftime("%H:%M:%S.") + f"{local.microsecond // 1000:03d}",
                "frame": r["frame"],
                "frame_mod1000": r["frame"] % 1000,  # 엔진 로그 [nnn]
                "last_key_name": key_names.get(r["last_key"], "") if r["last_key"] else "",
                "flag_names": "|".join(name for bit, name in FLAG_NAMES if r["flags"] & bit),
                "net_mode_name": NET_MODES.get(r["net_mode"], str(r["net_mode"])),
            }
            for c in RECORD_FIELDS:
                if c not in row and c not in ("utc_ticks", "_reserved"):
                    v = r[c]
                    row[c] = round(v, 2) if isinstance(v, float) else v
            w.writerow([row[c] for c in columns])
    print(f"→ {out_path}")


if __name__ == "__main__":
    main()
