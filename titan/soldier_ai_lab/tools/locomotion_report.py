# -*- coding: utf-8 -*-
"""
Locomotion test report (2026-09-30).

Reads the CSV the aim recorder writes while `SoldierLab.Test.Locomotion` drives the player's
soldier (Saved/AimTrace/<session>/<soldier>.csv) and prints, per test case:

  steady cases (phase "measure"):
    loop%      share of frames whose selected MM database is a *Loops* database
    db mix     the databases seen, most frequent first
    sw/s       blend-stack top clip changes per second
    slide      horizontal speed of a foot while its contact curve says it is planted
               (contact_l/contact_r > 0.5), cm/s - median and p90 over both feet
    cross%     frames where the left foot is to the right of the right foot (actor frame)
    travel     mean travel direction relative to facing (deg) - what warping has to make up
  turns (phase "post", the second half of each direction pair):
    t_loop     seconds after the change until a Loops database is selected (- = never)
    clips      distinct top clips during the window
    slide max / slide mean (first 0.6 s)
    cross      crossed-feet frames

Usage:
  python locomotion_report.py                      # newest session under the default Saved/AimTrace
  python locomotion_report.py <session dir or csv> [--csv out.csv]
"""
import csv, glob, math, os, sys, statistics, collections

DEFAULT_ROOT = r"C:\working\works\kadex\titan_example\Saved\AimTrace"
# The recorder reads the anim instance's curves AFTER the ABP's RemapCurves node, which rewrites
# contact_l/r = (1 - contact) * 100 for FootPlacement. So a planted foot reads 0 and an airborne
# one 100: planted <=> value <= PLANTED_MAX.
PLANTED_MAX = 50.0
# Curve-free slide measure: a foot that walks properly comes to (near) rest once per step. The
# minimum horizontal foot speed inside each window this long is taken; its median is "rest speed".
REST_WINDOW_S = 0.5
MAX_DT = 0.05          # frames slower than this (throttled editor) are left out of speed measures
CROSS_MARGIN_CM = 0.0  # left foot lateral > right foot lateral + margin => crossed


def find_csv(arg):
    if arg is None:
        sessions = sorted(glob.glob(os.path.join(DEFAULT_ROOT, "*")), key=os.path.getmtime)
        if not sessions:
            sys.exit("no sessions under " + DEFAULT_ROOT)
        arg = sessions[-1]
    files = [arg] if arg.lower().endswith(".csv") else glob.glob(os.path.join(arg, "*.csv"))
    for f in files:
        with open(f, newline="", encoding="utf-8") as fh:
            r = csv.reader(fh)
            h = next(r)
            if "test.case" not in h:
                continue
            ix = h.index("test.case")
            for row in r:
                if row and not row[0].startswith("#") and len(row) > ix and row[ix]:
                    return f
    sys.exit("no CSV with locomotion test labels in " + arg)


def load(path):
    with open(path, newline="", encoding="utf-8") as fh:
        r = csv.reader(fh)
        h = next(r)
        rows = [x for x in r if x and not x[0].startswith("#") and len(x) == len(h)]
    return {k: i for i, k in enumerate(h)}, rows


def fnum(s, default=0.0):
    try:
        return float(s)
    except (TypeError, ValueError):
        return default


def pct(v, q):
    if not v:
        return float("nan")
    v = sorted(v)
    return v[min(len(v) - 1, int(len(v) * q))]


def short_db(name):
    for key in ("Loops", "Starts", "Stops", "Pivots", "Idles", "TurnInPlace"):
        if key in name:
            stance = "Crouch" if "Crouch" in name else "Stand"
            gait = "Jog" if "Jog" in name else ("Walk" if "Walk" in name else "")
            return f"{stance}{gait}.{key}" if gait else f"{stance}.{key}"
    return name or "-"


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    out_csv = None
    if "--csv" in sys.argv:
        out_csv = sys.argv[sys.argv.index("--csv") + 1]
        args = [a for a in args if a != out_csv]
    path = find_csv(args[0] if args else None)
    ix, rows = load(path)
    g = lambda r, k: fnum(r[ix[k]]) if k in ix else 0.0
    s = lambda r, k: r[ix[k]] if k in ix else ""
    print("# Locomotion report -", path)

    # Per-frame derived values.
    n = len(rows)
    slide_l = [None] * n
    slide_r = [None] * n
    speed_l = [None] * n
    speed_r = [None] * n
    crossed = [False] * n
    for i in range(n):
        yaw = math.radians(g(rows[i], "actor.rot.yaw"))
        right = (-math.sin(yaw), math.cos(yaw))
        ax, ay = g(rows[i], "actor.loc.x"), g(rows[i], "actor.loc.y")
        lat = {}
        for side in ("l", "r"):
            fx, fy = g(rows[i], f"bone.foot_{side}.loc.x"), g(rows[i], f"bone.foot_{side}.loc.y")
            lat[side] = (fx - ax) * right[0] + (fy - ay) * right[1]
        crossed[i] = lat["l"] > lat["r"] + CROSS_MARGIN_CM
        if i == 0:
            continue
        dt = g(rows[i], "dt")
        if dt <= 0 or dt > MAX_DT:
            continue
        for side, store, spd in (("l", slide_l, speed_l), ("r", slide_r, speed_r)):
            dx = g(rows[i], f"bone.foot_{side}.loc.x") - g(rows[i - 1], f"bone.foot_{side}.loc.x")
            dy = g(rows[i], f"bone.foot_{side}.loc.y") - g(rows[i - 1], f"bone.foot_{side}.loc.y")
            v = math.hypot(dx, dy) / dt
            spd[i] = v
            if g(rows[i], f"curve.contact_{side}") <= PLANTED_MAX and g(rows[i - 1], f"curve.contact_{side}") <= PLANTED_MAX:
                store[i] = v

    def rest_speed(idx):
        """Median over REST_WINDOW_S windows of each foot's minimum horizontal speed (cm/s)."""
        mins = []
        for spd in (speed_l, speed_r):
            win, t_start = [], None
            for i in idx:
                if spd[i] is None:
                    continue
                t = g(rows[i], "time")
                if t_start is None:
                    t_start = t
                if t - t_start >= REST_WINDOW_S:
                    if win:
                        mins.append(min(win))
                    win, t_start = [], t
                win.append(spd[i])
            if win and len(win) > 3:
                mins.append(min(win))
        return statistics.median(mins) if mins else float("nan")

    def planted_share(idx):
        """Share of frames where the contact curve says at least one foot is planted (%)."""
        ok = [i for i in idx if g(rows[i], "dt") <= MAX_DT]
        if not ok:
            return float("nan")
        return sum(1 for i in ok if g(rows[i], "curve.contact_l") <= PLANTED_MAX or g(rows[i], "curve.contact_r") <= PLANTED_MAX) / len(ok) * 100

    # Group consecutive frames by (case, phase).
    groups = collections.OrderedDict()
    for i, r in enumerate(rows):
        c, p = s(r, "test.case"), s(r, "test.phase")
        if not c:
            continue
        groups.setdefault((c, p), []).append(i)

    summary = []
    steady = [(k, v) for k, v in groups.items() if k[1] == "measure"]
    if steady:
        print("\n## Steady (measure phase)\n")
        print("| case | loop% | db mix | sw/s | rest spd | planted% | slide med | slide p90 | cross% | travel | speed |")
        print("|---|---:|---|---:|---:|---:|---:|---:|---:|---:|---:|")
        for (case, _), idx in steady:
            dur = sum(g(rows[i], "dt") for i in idx) or 1e-6
            dbs = collections.Counter(short_db(s(rows[i], "mm.db")) for i in idx)
            loop = sum(v for k, v in dbs.items() if k.endswith("Loops")) / len(idx) * 100
            mix = " ".join(f"{k}:{v * 100 // len(idx)}" for k, v in dbs.most_common(3))
            sw = sum(1 for a, b in zip(idx, idx[1:]) if s(rows[a], "stack0.anim") != s(rows[b], "stack0.anim")) / dur
            sl = [x for i in idx for x in (slide_l[i], slide_r[i]) if x is not None]
            cr = sum(1 for i in idx if crossed[i]) / len(idx) * 100
            tv = [fnum(s(rows[i], "travel.yawVsFacing"), float("nan")) for i in idx]
            tv = [x for x in tv if not math.isnan(x)]
            sp = statistics.mean(g(rows[i], "speed2D") for i in idx)
            med, p90 = pct(sl, 0.5), pct(sl, 0.9)
            rs, ps = rest_speed(idx), planted_share(idx)
            print(f"| {case} | {loop:.0f} | {mix} | {sw:.1f} | {rs:.1f} | {ps:.0f} | {med:.1f} | {p90:.1f} | {cr:.0f} | {statistics.mean(tv) if tv else float('nan'):.0f} | {sp:.0f} |")
            summary.append(dict(kind="steady", case=case, loop_pct=round(loop, 1), db_mix=mix, switches_per_s=round(sw, 2),
                                rest_speed=round(rs, 1), planted_pct=round(ps, 1),
                                slide_med=round(med, 1), slide_p90=round(p90, 1), cross_pct=round(cr, 1)))

    turns = [(k, v) for k, v in groups.items() if k[1] == "post"]
    if turns:
        print("\n## Turns (post phase: after the direction change)\n")
        print("| case | t_loop s | clips | slide max | slide mean 0.6s | cross frames |")
        print("|---|---:|---:|---:|---:|---:|")
        for (case, _), idx in turns:
            t0 = g(rows[idx[0]], "time")
            t_loop = next((g(rows[i], "time") - t0 for i in idx if short_db(s(rows[i], "mm.db")).endswith("Loops")), None)
            clips = len(set(s(rows[i], "stack0.anim") for i in idx))
            sl_all = [x for i in idx for x in (slide_l[i], slide_r[i]) if x is not None]
            sl_early = [x for i in idx if g(rows[i], "time") - t0 <= 0.6 for x in (slide_l[i], slide_r[i]) if x is not None]
            crf = sum(1 for i in idx if crossed[i])
            print(f"| {case} | {('%.2f' % t_loop) if t_loop is not None else '-'} | {clips} | "
                  f"{max(sl_all) if sl_all else float('nan'):.0f} | {statistics.mean(sl_early) if sl_early else float('nan'):.0f} | {crf} |")
            summary.append(dict(kind="turn", case=case, t_loop=round(t_loop, 2) if t_loop is not None else "",
                                clips=clips, slide_max=round(max(sl_all), 1) if sl_all else "",
                                slide_mean_early=round(statistics.mean(sl_early), 1) if sl_early else "", cross_frames=crf))

    if out_csv and summary:
        keys = sorted({k for d in summary for k in d})
        with open(out_csv, "w", newline="", encoding="utf-8") as fh:
            w = csv.DictWriter(fh, fieldnames=keys)
            w.writeheader()
            w.writerows(summary)
        print("\nsummary written to", out_csv)


if __name__ == "__main__":
    main()
