#!/usr/bin/env python3
"""Microbenchmarks for tpc-clang: time kernels on Gaudi hardware.

  build   (local) compile every kernel in kernels/ with each compiler variant,
          run it on the TPC simulator, check that every variant's outputs equal
          the reference's, and write a self-contained bundle directory
  run     (on the Gaudi machine) run the bundle on the card with the Habana
          profiler, REPS times, and append one JSON line per run to
          <bundle>/results.jsonl
  report  (anywhere) simulator cycles and card times per kernel, with the
          speedup of each variant over the reference

The first variant is the reference, by default Habana's tpc-clang. Card time
is the sum over the TPCs of the time from TPC start to halt, as the profiler
records it; see README.md in tpc/.
"""

import argparse
import concurrent.futures
import hashlib
import json
import math
import os
import random
import re
import shlex
import shutil
import socket
import statistics
import subprocess
import sys
import tarfile
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parent.parent

TICK_NS = 20                      # profiler timestamp unit
CLOCK_GHZ = {"gaudi2": 1.8}       # TPC clock, to compare card time with cycles
FILL = 0xCD                       # sim_runner's fill byte for output tensors
ACCEL = Path("/dev/accel")        # Gaudi device nodes


def fatal(msg):
    sys.exit(f"bench.py: error: {msg}")


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def run(cmd, timeout, **kw):
    try:
        p = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout, **kw)
    except subprocess.TimeoutExpired:
        return "timeout", ""
    return ("ok" if p.returncode == 0 else f"exit {p.returncode}"), p.stdout + p.stderr


def build_runner(src, exe, lib):
    """Build sim_runner.cpp; it runs on the card when TPC_RUNNER=1 is set."""
    if exe.exists() and exe.stat().st_mtime >= src.stat().st_mtime:
        return exe
    cmd = [os.environ.get("CXX", "c++"), "-std=c++17", "-O2", f"-I{lib}/include", str(src),
           "-o", str(exe), f"-L{lib}", "-ltpc_tests_core_ext", "-ltpcsim_shared",
           f"-Wl,-rpath-link,{lib}", f"-Wl,--disable-new-dtags,-rpath,{lib}"]
    status, log = run(cmd, timeout=600)
    if status != "ok":
        fatal(f"building {exe.name} failed:\n{log}")
    return exe


# --------------------------------------------------------------------------
# build
# --------------------------------------------------------------------------

def description(path):
    """The kernel's leading comment, up to the first blank comment line."""
    lines = []
    for line in path.read_text().splitlines():
        m = re.match(r"\s*//\s?(.*)", line)
        if not m or not m.group(1).strip():
            break
        lines.append(m.group(1).strip())
    return " ".join(lines)


def kernel_id(path):
    """'<category>/<name>' for a kernel under kernels/, else the file's name."""
    try:
        return path.relative_to(HERE / "kernels").with_suffix("").as_posix()
    except ValueError:
        return path.stem


def category(kid):
    return kid.split("/")[0] if "/" in kid else "other"


def unwritten(data, dtype, dt):
    """Output elements still holding the fill pattern (2-byte types and wider)."""
    size = dt.DTYPES[dtype][0]
    if size < 2:
        return 0
    fill = bytes([FILL]) * size
    return sum(data[i:i + size] == fill for i in range(0, len(data), size))


def build_kernel(dt, k, variants, args, runner, env):
    d = args.out / k.id
    d.mkdir(parents=True, exist_ok=True)
    rng = random.Random(f"{args.seed}:{k.id}")
    for i, t in enumerate(k.tensors):
        if not t.is_output:
            (d / f"in{i}.bin").write_bytes(dt.gen_input(t, rng))
    res, outs = {}, {}
    for name, cc, flags in variants:
        obj = d / f"{name}.o"
        cmd = [str(cc), f"-march={args.arch}", "-fno-crash-diagnostics", "-c", str(k.path),
               "-o", str(obj), *shlex.split(args.cflags), *k.cflags, *flags]
        status, log = run(cmd, timeout=600, env=env)
        if status != "ok":
            res[name] = {"error": f"compile {status}: {dt.last_line(log)}"}
            continue
        # Relative paths: the same spec runs here and on the Gaudi machine.
        lines = [f"device {args.arch}", f"elf {name}.o",
                 "index_space " + " ".join(map(str, k.index_space))]
        lines += [f"scalar {s:08x}" for s in k.scalars]
        for i, t in enumerate(k.tensors):
            size, gc_type, _ = dt.DTYPES[t.dtype]
            f = f"{name}.out{i}.bin" if t.is_output else f"in{i}.bin"
            lines.append(f"tensor {'out' if t.is_output else 'in'} {size} {gc_type} "
                         + " ".join(map(str, t.shape)) + f" {f}")
        (d / f"{name}.spec").write_text("\n".join(lines) + "\n")
        status, log = run([str(runner), f"{name}.spec"], timeout=1800, cwd=d, env=env)
        if status != "ok":
            res[name] = {"error": f"simulator {status}: {dt.last_line(log)}"}
            continue
        m = re.search(r"cycles (\d+)", log)
        _, text_size = dt.find_section(obj.read_bytes(), b".text")
        r = {"cycles": int(m.group(1)), "text_bytes": text_size, "outputs": []}
        o = []
        for i, t in enumerate(k.tensors):
            if t.is_output:
                f = d / f"{name}.out{i}.bin"
                data = f.read_bytes()
                f.unlink()
                o.append((t, data))
                r["outputs"].append(hashlib.sha256(data).hexdigest())
                n = unwritten(data, t.dtype, dt)
                if n:
                    r["error"] = (f"output {i}: {n} elements unwritten (the card fills them "
                                  "differently, so card and simulator outputs would differ)")
        outs[name] = o
        res[name] = r
    ref = variants[0][0]
    for name in outs:
        if name != ref and ref in outs:
            res[name]["match_ref"] = dt.outputs_match(outs[ref], outs[name], k)
    return res


def compiler_comment(dt, obj):
    """The .comment section of an object: the compiler's version string."""
    data = obj.read_bytes()
    try:
        off, size = dt.find_section(data, b".comment")
    except ValueError:
        return ""
    strings = (s.decode(errors="replace") for s in data[off:off + size].split(b"\0") if s)
    return " | ".join(dict.fromkeys(strings))


def git_head():
    status, out = run(["git", "-C", str(REPO), "describe", "--always", "--dirty", "--abbrev=12",
                       "--exclude=*"], timeout=60)
    return out.strip() if status == "ok" else "unknown"


def cmd_build(args):
    sys.path.insert(0, str(REPO / "tpc/difftest"))
    import run as dt

    if not args.tpc_home:
        fatal("set TPC_HOME (e.g. 'source ~/tpc-1.24.2/env.sh') or pass --tpc-home")
    tpc_home = Path(args.tpc_home)
    lib = tpc_home / "usr/lib/habanatools"
    variants = []
    for v in args.variant or [f"habana={tpc_home / 'usr/bin/tpc-clang'}", f"fork={args.fork}"]:
        name, sep, rest = v.partition("=")
        if not sep or not re.fullmatch(r"[A-Za-z0-9_-]+", name):
            fatal(f"bad --variant '{v}': expected NAME=COMPILER[@FLAGS]")
        cc, _, flags = rest.partition("@")
        cc = Path(cc).absolute()  # not resolve(): tpc-clang is a symlink, and the
                                  # driver picks TPC mode from the name it runs as
        if not cc.is_file():
            fatal(f"compiler for variant '{name}' not found: {cc}")
        variants.append((name, cc, shlex.split(flags)))
    if len({v[0] for v in variants}) != len(variants):
        fatal("variant names must be unique")

    files = args.kernels or sorted((HERE / "kernels").rglob("*.c"))
    kernels = [dt.parse_kernel(f.resolve(), tag="BENCH") for f in files]
    kernels = [k for k in kernels if args.arch in k.archs]
    if not kernels:
        fatal(f"no kernels for {args.arch}")
    for k in kernels:
        k.id = kernel_id(k.path)
    if len({k.id for k in kernels}) != len(kernels):
        fatal("kernel names must be unique")

    args.out = args.out.resolve()
    if args.out.exists():
        if not (args.out / "manifest.json").exists():
            fatal(f"{args.out} exists and is not a bench bundle; not overwriting it")
        if (args.out / "results.jsonl").exists():
            fatal(f"{args.out} holds card results (results.jsonl); pick another --out")
        shutil.rmtree(args.out)
    args.out.mkdir(parents=True)
    shutil.copy(Path(__file__), args.out / "bench.py")
    shutil.copy(REPO / "tpc/difftest/sim_runner.cpp", args.out / "sim_runner.cpp")
    env = dict(os.environ)
    env["LD_LIBRARY_PATH"] = str(lib) + os.pathsep + env.get("LD_LIBRARY_PATH", "")
    runner = build_runner(args.out / "sim_runner.cpp", args.out / "sim_runner", lib)

    with concurrent.futures.ThreadPoolExecutor(args.jobs) as pool:
        futs = {k.id: pool.submit(build_kernel, dt, k, variants, args, runner, env)
                for k in kernels}
        sim = {name: f.result() for name, f in futs.items()}
    (args.out / "sim_runner").unlink()  # built again on the Gaudi machine

    manifest = {
        "created": time.strftime("%Y-%m-%d %H:%M:%S %z"),
        "repo": git_head(), "arch": args.arch, "cflags": args.cflags, "seed": args.seed,
        "variants": [], "kernels": {},
    }
    for name, cc, flags in variants:
        obj = next((args.out / k.id / f"{name}.o" for k in kernels
                    if (args.out / k.id / f"{name}.o").exists()), None)
        manifest["variants"].append({
            "name": name, "compiler": str(cc), "flags": flags,
            "sha256": sha256(cc)[:16], "comment": compiler_comment(dt, obj) if obj else ""})
    for k in kernels:
        manifest["kernels"][k.id] = {
            "description": description(k.path), "source": f"{k.id}.c",
            "index_space": k.index_space,
            "tensors": [f"{'out' if t.is_output else 'in'} {t.dtype} "
                        + "x".join(map(str, t.shape)) for t in k.tensors]}
    (args.out / "manifest.json").write_text(json.dumps(manifest, indent=1) + "\n")
    (args.out / "sim.json").write_text(json.dumps(sim, indent=1) + "\n")
    print_report(args.out)
    bad = [(k, v) for k, r in sim.items() for v, x in r.items()
           if "error" in x or x.get("match_ref") is False]
    print(f"\nbundle: {args.out}")
    if bad:
        print(f"{len(bad)} kernel/variant pairs cannot be compared (see above); "
              "they are left out of the card run")
    return 0


# --------------------------------------------------------------------------
# run (on the Gaudi machine)
# --------------------------------------------------------------------------

def accel_holders():
    """Processes with a Gaudi compute device open, such as a serving job; a card
    opens in one process at a time. Control nodes (accel_controlD*) do not count.
    Only our own processes' descriptors are visible."""
    device = re.compile(rf"{re.escape(str(ACCEL))}/accel\d+$")
    holders = []
    for fd_dir in Path("/proc").glob("[0-9]*/fd"):
        pid = fd_dir.parent.name
        if pid == str(os.getpid()):
            continue
        try:
            if any(device.match(os.readlink(fd)) for fd in fd_dir.iterdir()):
                cmd = (fd_dir.parent / "cmdline").read_bytes().replace(b"\0", b" ").decode()
                holders.append(f"{pid} {cmd.strip()[:120]}")
        except OSError:
            continue
    return holders


def parse_trace(path):
    """TPC start-to-halt times from a Habana profiler capture (.hltv)."""
    with tarfile.open(path, "r:gz") as tf:
        m = next(m for m in tf.getmembers() if "/" not in m.name and m.name.endswith(".json"))
        events = json.load(tf.extractfile(m))["traceEvents"]
    procs, threads = {}, {}
    for e in events:
        if e.get("ph") == "M" and e.get("name") == "process_name":
            procs[e["pid"]] = e["args"]["name"]
        if e.get("ph") == "M" and e.get("name") == "thread_name":
            threads[(e["pid"], e["tid"])] = e["args"]["name"]
    runs = {}
    for e in events:
        if e.get("ph") not in ("B", "E"):
            continue
        proc = procs.get(e["pid"], "")
        if "TPC" not in proc.upper() or "Bus" in proc:
            continue
        thread = threads.get((e["pid"], e["tid"]), str(e["tid"]))
        raw = e["args"].get("args_internal", {}).get("HW raw timestamp")
        runs.setdefault(thread, []).append((e["ph"], e["ts"], raw))
    ticks, first, last = [], None, None
    for evs in runs.values():
        begin = None
        for ph, _, raw in sorted(evs, key=lambda x: x[1]):
            if ph == "B":
                begin = raw
                first = raw if first is None else min(first, raw)
            elif begin is not None:
                ticks.append(raw - begin)
                last = raw if last is None else max(last, raw)
                begin = None
    if not ticks:
        raise ValueError("no TPC start/halt pairs in the trace")
    return {"runs": len(ticks), "tpcs": len(runs), "sum_ticks": sum(ticks),
            "max_ticks": max(ticks), "span_ticks": last - first}


def cmd_run(args):
    b = args.bundle.resolve()
    manifest = json.loads((b / "manifest.json").read_text())
    sim = json.loads((b / "sim.json").read_text())
    lib = Path(args.lib)
    if not (lib / "include").is_dir():
        fatal(f"habanatools not found at {lib} (pass --lib)")
    if not ACCEL.is_dir():
        fatal(f"no {ACCEL}: not a Gaudi machine")
    holders = accel_holders()
    if holders and not args.force:
        fatal("these processes hold Gaudi devices; a busy card cannot run the benchmarks "
              "(--force to try anyway):\n  " + "\n  ".join(holders))
    runner = build_runner(b / "sim_runner.cpp", b / "hw_runner", lib)

    cases = []
    for kname in sorted(sim):
        if args.filter and not re.search(args.filter, kname):
            continue
        ok = [v["name"] for v in manifest["variants"]
              if "error" not in sim[kname].get(v["name"], {"error": 1})
              and sim[kname][v["name"]].get("match_ref", True)]
        if len(ok) >= 2 or (ok and len(manifest["variants"]) == 1):
            cases.append((kname, ok))
    for kname, _ in cases:
        for v in manifest["variants"]:
            spec = b / kname / f"{v['name']}.spec"
            if spec.exists() and not re.search(r"^tensor ", spec.read_text(), re.M):
                fatal(f"{spec}: no tensors; such a kernel hangs the card")  # see README
    if not cases:
        fatal("nothing to run")

    (b / "hw_env.json").write_text(json.dumps({
        "host": socket.gethostname(), "date": time.strftime("%Y-%m-%d %H:%M:%S %z"),
        "driver": Path("/sys/module/habanalabs/version").read_text().strip()
        if Path("/sys/module/habanalabs/version").exists() else "unknown",
        "lib": str(lib)}, indent=1) + "\n")
    env = dict(os.environ, HABANA_PROFILE="1", TPC_RUNNER="1")
    env["LD_LIBRARY_PATH"] = str(lib) + os.pathsep + env.get("LD_LIBRARY_PATH", "")
    total = sum(len(v) for _, v in cases) * args.reps
    done = 0
    with open(b / "results.jsonl", "a") as results:
        for rep in range(1, args.reps + 1):
            for kname, names in cases:
                d = b / kname
                # Rotate the order of the variants every rep, so that neither
                # always runs first after another kernel.
                order = names[rep % len(names):] + names[:rep % len(names)]
                for name in order:
                    for old in d.glob("default_profiling_*.hltv"):
                        old.unlink()
                    t0 = time.time()
                    status, log = run([str(runner), f"{name}.spec"], timeout=120, cwd=d, env=env,
                                      stdin=subprocess.DEVNULL)
                    wall = time.time() - t0
                    (d / f"{name}.hw.log").write_text(log)
                    traces = sorted(d.glob("default_profiling_*.hltv"), key=os.path.getmtime)
                    if status != "ok" or not traces:
                        why = status if status != "ok" else "no profiler trace"
                        fatal(f"{kname} {name} rep {rep}: {why}; stopping (a hung kernel "
                              f"resets the card). Log: {d / (name + '.hw.log')}")
                    try:
                        t = parse_trace(traces[-1])
                    except (ValueError, KeyError, OSError, tarfile.TarError) as e:
                        fatal(f"{kname} {name} rep {rep}: bad trace {traces[-1]}: {e}")
                    traces[-1].unlink()
                    outs = sorted(d.glob(f"{name}.out*.bin"), key=lambda p: p.name)
                    same = [sha256(p) for p in outs] == sim[kname][name]["outputs"]
                    for p in outs:
                        p.unlink()
                    rec = {"kernel": kname, "variant": name, "rep": rep, "same_as_sim": same,
                           "wall_s": round(wall, 3), **t}
                    results.write(json.dumps(rec) + "\n")
                    results.flush()
                    done += 1
                    print(f"[{done}/{total}] rep {rep} {kname:28} {name:10} "
                          f"{t['sum_ticks'] * TICK_NS / 1000:10.1f} TPC-us"
                          + ("" if same else "  CARD OUTPUT != SIMULATOR"), flush=True)
    print_report(b)
    return 0


# --------------------------------------------------------------------------
# report
# --------------------------------------------------------------------------

def geomean(xs):
    xs = [x for x in xs if x and x > 0]
    return math.exp(sum(map(math.log, xs)) / len(xs)) if xs else float("nan")


def collect(b):
    manifest = json.loads((b / "manifest.json").read_text())
    sim = json.loads((b / "sim.json").read_text())
    hw = {}
    if (b / "results.jsonl").exists():
        for line in (b / "results.jsonl").read_text().splitlines():
            r = json.loads(line)
            hw.setdefault((r["kernel"], r["variant"]), []).append(r)
    return manifest, sim, hw


def card_time(runs):
    """Median card time in microseconds, its spread and the run count."""
    us = [r["sum_ticks"] * TICK_NS / 1000 for r in runs]
    med = statistics.median(us)
    return med, (max(us) - min(us)) / med if med else 0.0, len(us)


def speedup(row, key, ref, v):
    """Reference time / variant time in row, from 'cycles' or 'card'."""
    a, b = row[key].get(ref), row[key].get(v)
    if a is None or b is None:
        return None
    if key == "card":
        a, b = a[0], b[0]
    return a / b if b else None


def geomeans(rows, key, ref, v):
    """{category: (geomean, kernel count)}, plus 'all', over rows with a speedup."""
    groups = {}
    for r in rows:
        s = speedup(r, key, ref, v)
        if s:
            groups.setdefault(category(r["kernel"]), []).append(s)
            groups.setdefault("all", []).append(s)
    return {c: (geomean(xs), len(xs)) for c, xs in groups.items()}


def print_report(b, markdown=False):
    manifest, sim, hw = collect(b)
    names = [v["name"] for v in manifest["variants"]]
    ref, others = names[0], names[1:]
    clock = CLOCK_GHZ.get(manifest["arch"])
    rows, notes = [], []
    for k in sorted(sim):
        s = sim[k]
        for v in names:
            if "error" in s.get(v, {}):
                notes.append(f"{k} [{v}]: {s[v]['error']}")
            elif s.get(v, {}).get("match_ref") is False:
                notes.append(f"{k} [{v}]: outputs differ from {ref}'s; not compared")
        cyc = {v: s[v]["cycles"] for v in names
               if v in s and "error" not in s[v] and s[v].get("match_ref", True)}
        card = {}
        for v in names:
            runs = hw.get((k, v), [])
            if runs and not all(r["same_as_sim"] for r in runs):
                notes.append(f"{k} [{v}]: card outputs differ from the simulator's; not compared")
            elif runs and v in cyc:
                card[v] = card_time(runs)
        rows.append({"kernel": k, "cycles": cyc, "card": card})
    categories = list(dict.fromkeys(category(r["kernel"]) for r in rows)) + ["all"]

    if markdown:
        print_markdown(manifest, b, rows, ref, others, categories)
        return

    print(f"arch {manifest['arch']}, {manifest['cflags']}, built {manifest['created']} "
          f"from {manifest['repo']}")
    for v in manifest["variants"]:
        print(f"  {v['name']:10} {v['compiler']} {' '.join(v['flags'])}  [{v['comment']}]")
    env = b / "hw_env.json"
    if env.exists():
        e = json.loads(env.read_text())
        print(f"  card runs on {e['host']}, driver {e['driver']}, last started {e['date']}")
    w = max(len(r["kernel"]) for r in rows) + 2
    head = f"{'kernel':{w}}" + "".join(f"{'sim:' + v:>12}" for v in names)
    head += "".join(f"{'card us:' + v:>15}" for v in names)
    head += "".join(f"{'sim x':>8}{'card x':>8}" for v in others)
    if clock:
        head += f"{'card/sim':>10}"
    print("\n" + head)
    for r in rows:
        line = f"{r['kernel']:{w}}"
        line += "".join(f"{r['cycles'].get(v, '-'):>12}" for v in names)
        for v in names:
            c = r["card"].get(v)
            line += f"{c[0]:>9.1f} ±{c[1] * 50:3.1f}%" if c else f"{'-':>15}"
        for v in others:
            for key in ("cycles", "card"):
                s = speedup(r, key, ref, v)
                line += f"{s:>8.3f}" if s else f"{'-':>8}"
        if clock:
            c, cyc = r["card"].get(ref), r["cycles"].get(ref)
            line += f"{c[0] * 1000 * clock / cyc:>10.2f}" if c and cyc else f"{'-':>10}"
        print(line)
    for v in others:
        sims, cards = geomeans(rows, "cycles", ref, v), geomeans(rows, "card", ref, v)
        print(f"\ngeomean speedup of {v} over {ref}:{'simulator':>13}{'card':>16}")
        for c in categories:
            cell = [f"{g[0]:.3f} ({g[1]:2})" if (g := d.get(c)) else "-" for d in (sims, cards)]
            print(f"  {c:28}{cell[0]:>13}{cell[1]:>16}")
    if not hw:
        print("no card results yet (bench.py run on the Gaudi machine)")
    print("x = reference time / variant time (> 1: the variant is faster); "
          "± = half the min-max range over reps; card/sim = reference card cycles / "
          "simulator cycles")
    for n in notes:
        print("NOTE", n)


def print_markdown(manifest, b, rows, ref, others, categories):
    """The table for the README: card times when there are any, else cycles."""
    env = json.loads((b / "hw_env.json").read_text()) if (b / "hw_env.json").exists() else None
    key = "card" if any(r["card"] for r in rows) else "cycles"
    cols = ["Kernel"] + [f"{v} ({'TPC-µs' if key == 'card' else 'cycles'})"
                         for v in [ref] + others]
    cols += [f"{v} vs {ref}" for v in others]
    print("| " + " | ".join(cols) + " |")
    print("|" + "|".join(["---"] + ["---:"] * (len(cols) - 1)) + "|")
    for r in rows:
        vals = []
        for v in [ref] + others:
            x = r[key].get(v)
            vals.append("—" if x is None else f"{x[0]:.1f}" if key == "card" else str(x))
        sp = [f"{s:.2f}×" if (s := speedup(r, key, ref, v)) else "—" for v in others]
        print(f"| `{r['kernel']}` | " + " | ".join(vals + sp) + " |")
    means = {v: geomeans(rows, key, ref, v) for v in others}
    for c in categories:
        label = "**geomean, all**" if c == "all" else f"geomean, {c}"
        cells = []
        for v in others:
            g = means[v].get(c)
            cells.append("—" if not g else f"**{g[0]:.2f}×**" if c == "all" else f"{g[0]:.2f}×")
        print(f"| {label} | " + " | ".join([""] * (len(others) + 1)) + " | "
              + " | ".join(cells) + " |")
    where = (f"Gaudi2 card, driver {env['driver']}" if key == "card" and env
             else "TPC simulator")
    print(f"\n{where}, {manifest['arch']} {manifest['cflags']}, fork {manifest['repo']}.")


def cmd_report(args):
    print_report(args.bundle.resolve(), markdown=args.markdown)
    return 0


# --------------------------------------------------------------------------

def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)

    p = sub.add_parser("build", help="compile, simulate and write a bundle")
    p.add_argument("kernels", nargs="*", type=Path,
                   help="kernel files (default: every kernels/<category>/*.c)")
    p.add_argument("--out", type=Path, default=REPO / "build/tpc-bench",
                   help="bundle directory (default: build/tpc-bench)")
    p.add_argument("--tpc-home", default=os.environ.get("TPC_HOME"),
                   help="habanatools root (default: $TPC_HOME)")
    p.add_argument("--fork", type=Path, default=REPO / "build/bin/tpc-clang",
                   help="fork compiler for the default variants (default: build/bin/tpc-clang)")
    p.add_argument("--variant", action="append", metavar="NAME=COMPILER[@FLAGS]",
                   help="a compiler to time, repeatable; the first is the reference "
                        "(default: habana=$TPC_HOME/usr/bin/tpc-clang fork=--fork)")
    p.add_argument("--arch", default="gaudi2")
    p.add_argument("--cflags", default="-O2", help="flags for every variant (default: -O2)")
    p.add_argument("--seed", type=int, default=1)
    p.add_argument("-j", "--jobs", type=int, default=8)
    p.set_defaults(func=cmd_build)

    p = sub.add_parser("run", help="time a bundle on the card (Gaudi machine)")
    p.add_argument("bundle", type=Path, nargs="?", default=HERE)
    p.add_argument("--reps", type=int, default=10)
    p.add_argument("--filter", help="regex: only kernels whose '<category>/<name>' matches")
    p.add_argument("--lib", default=str(Path(os.environ["TPC_HOME"]) / "usr/lib/habanatools")
                   if os.environ.get("TPC_HOME") else "/usr/lib/habanatools",
                   help="habanatools library directory (default: /usr/lib/habanatools)")
    p.add_argument("--force", action="store_true", help="run although devices look busy")
    p.set_defaults(func=cmd_run)

    p = sub.add_parser("report", help="print the results")
    p.add_argument("bundle", type=Path, nargs="?", default=HERE)
    p.add_argument("--markdown", action="store_true", help="a table for README.md")
    p.set_defaults(func=cmd_report)

    args = ap.parse_args()
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
