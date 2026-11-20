#!/usr/bin/env python3
"""Differential functional tests for tpc-clang.

Each kernel in kernels/ is compiled by Habana's closed-source tpc-clang (the
reference), the candidate fork compiler and, optionally, a baseline fork
compiler. Every object runs on the TPC simulator with identical inputs and the
output tensors are compared.

  PASS   candidate output == reference output
  KNOWN  candidate differs from reference exactly like the baseline does
  FAIL   anything else (including candidate metadata differing from baseline)
  SKIP   reference cannot compile the kernel for that arch

The candidate runs exactly as compiled. A baseline from before the fork emitted
.tpc_metadata v20 cannot run on current habanatools (see README.md), so it runs
with the reference object's metadata section instead. A baseline with the
candidate's metadata version has its metadata checked byte-for-byte against the
candidate's.

Compile-parity mode (--compile-parity) compiles C sources (default: the RC99
lit tests) with each compiler and compares only whether compilation succeeds.
"""

import argparse
import concurrent.futures
import hashlib
import math
import os
import random
import re
import shlex
import struct
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parent.parent

# name: (element bytes, tpc_lib_api::TensorDataType, struct format or None)
DTYPES = {
    "i8": (1, 1 << 2, "b"), "u8": (1, 1 << 3, "B"),
    "f8_152": (1, 1 << 4, None), "f8_143": (1, 1 << 5, None),
    "i16": (2, 1 << 6, "h"), "u16": (2, 1 << 7, "H"),
    "bf16": (2, 1 << 8, None), "f16": (2, 1 << 9, "e"),
    "i32": (4, 1 << 10, "i"), "u32": (4, 1 << 11, "I"),
    "f32": (4, 1 << 12, "f"),
}
FLOAT_DTYPES = {"f32", "bf16", "f16"}
SIM_ARCHS = {"gaudi", "gaudi2", "gaudi3"}
PARITY_ARCHS = ["gaudi", "gaudib", "greco", "goya2", "gaudi2", "gaudi3"]
# The names in RUN lines that select a source for each parity arch. The lit tests
# call gaudi3 doron1, the fork's name for it, which Habana's compiler rejects.
PARITY_RUN_NAMES = {"gaudi3": {"gaudi3", "doron1"}}
# Known compile-parity differences on archs the baseline cannot build (the
# LLVM 12 baseline has no gaudi3), with the (reference, candidate) statuses they
# have. Reported as KNOWN, like a difference the baseline shares.
KNOWN_PARITY = {
    # Habana does not declare s_bf16_st_tnsr_s and the fork does; on gaudi2
    # the baseline shares it.
    ("clang/test/RC99/Intrinsics/st_tnsr_s-04.c", "gaudi3"): ("error", "ok"),
    # 512-lane f8 conversions: Habana's compiler crashes, the fork converts
    # each half (the baseline asserted).
    **{(f"clang/test/RC99/Intrinsics/convert_to/{name}.c", arch): ("crash", "ok")
       for name in ("convert_minifloat512_to_minihalf512",
                    "convert_minihalf512_to_minifloat512")
       for arch in ("gaudi2", "gaudi3")},
}
METADATA = b".tpc_metadata"
METADATA_VERSION = 20


@dataclass
class Tensor:
    is_output: bool
    dtype: str
    shape: list
    lo: float = None
    hi: float = None

    @property
    def count(self):
        return math.prod(self.shape)


@dataclass
class Kernel:
    path: Path
    archs: list = field(default_factory=list)
    tensors: list = field(default_factory=list)
    index_space: list = field(default_factory=lambda: [1] * 5)
    scalars: list = field(default_factory=list)  # u32 bit patterns
    ulp: int = 0
    cflags: list = field(default_factory=list)

    @property
    def name(self):
        return self.path.stem


def fatal(msg):
    sys.exit(f"run.py: error: {msg}")


# --------------------------------------------------------------------------
# Kernel spec parsing: lines of the form "// DIFFTEST: <directive> <args...>"
# --------------------------------------------------------------------------

def scalar_bits(kind, text):
    if kind == "f32":
        return struct.unpack("<I", struct.pack("<f", float(text)))[0]
    if kind in ("i32", "u32"):
        return int(text, 0) & 0xFFFFFFFF
    raise ValueError(f"unsupported scalar type '{kind}'")


def parse_kernel(path, tag="DIFFTEST"):
    """Read the '// <tag>:' lines of a kernel (tpc/bench uses tag BENCH)."""
    k = Kernel(path)
    for lineno, line in enumerate(path.read_text().splitlines(), 1):
        m = re.match(rf"\s*//\s*{tag}:\s*(\S+)\s*(.*)", line)
        if not m:
            continue
        key, args = m.group(1), m.group(2).split()
        try:
            if key == "arch":
                k.archs = args
            elif key in ("in", "out"):
                t = Tensor(key == "out", args[0], [int(a) for a in args[1:] if "=" not in a])
                for a in args[1:]:
                    if a.startswith("range="):
                        t.lo, t.hi = (float(v) for v in a[6:].split(","))
                if t.dtype not in DTYPES or not 1 <= len(t.shape) <= 5:
                    raise ValueError("bad dtype or rank")
                if not t.is_output and any(o.is_output for o in k.tensors):
                    raise ValueError("'in' tensors must all come before 'out' tensors")
                t.shape += [1] * (5 - len(t.shape))
                k.tensors.append(t)
            elif key == "index-space":
                k.index_space = [int(a) for a in args] + [1] * (5 - len(args))
            elif key == "scalar":
                k.scalars.append(scalar_bits(args[0], args[1]))
            elif key == "tolerance":
                if args[0] != "ulp":
                    raise ValueError("only 'tolerance ulp N' is supported")
                k.ulp = int(args[1])
            elif key == "cflags":
                k.cflags = args
            else:
                raise ValueError(f"unknown directive '{key}'")
        except (ValueError, IndexError) as e:
            fatal(f"{path}:{lineno}: {e}")
    if not k.archs or not any(t.is_output for t in k.tensors):
        fatal(f"{path}: needs a {tag} 'arch' line and at least one 'out' tensor")
    return k


# --------------------------------------------------------------------------
# Data generation and comparison
# --------------------------------------------------------------------------

def gen_input(t, rng):
    size, _, fmt = DTYPES[t.dtype]
    if t.dtype in ("f8_152", "f8_143"):
        return bytes(rng.getrandbits(8) for _ in range(t.count))
    if t.dtype in FLOAT_DTYPES:
        lo = -10.0 if t.lo is None else t.lo
        hi = 10.0 if t.hi is None else t.hi
        vals = [rng.uniform(lo, hi) for _ in range(t.count)]
        if t.dtype == "bf16":
            return b"".join(struct.pack("<f", v)[2:] for v in vals)
        return struct.pack(f"<{t.count}{fmt}", *vals)
    bits = size * 8
    signed = t.dtype.startswith("i")
    lo = int(t.lo) if t.lo is not None else (-(1 << (bits - 1)) if signed else 0)
    hi = int(t.hi) if t.hi is not None else ((1 << (bits - 1)) - 1 if signed else (1 << bits) - 1)
    return struct.pack(f"<{t.count}{fmt}", *(rng.randint(lo, hi) for _ in range(t.count)))


def float_ordinals(data, dtype):
    """Map float bit patterns to integers that are monotonic in value."""
    if dtype == "f32":
        raw, sign = struct.unpack(f"<{len(data) // 4}I", data), 1 << 31
    else:
        raw, sign = struct.unpack(f"<{len(data) // 2}H", data), 1 << 15
    return [(-(r & (sign - 1)) if r & sign else r) for r in raw]


def is_nan(bits, dtype):
    exp_mask, man_mask = {"f32": (0x7F800000, 0x7FFFFF), "bf16": (0x7F80, 0x7F),
                          "f16": (0x7C00, 0x3FF)}[dtype]
    return (bits & exp_mask) == exp_mask and (bits & man_mask) != 0


def max_ulp_diff(a, b, dtype):
    oa, ob = float_ordinals(a, dtype), float_ordinals(b, dtype)
    width = 4 if dtype == "f32" else 2
    fmt = "<I" if width == 4 else "<H"
    worst = 0
    for i, (x, y) in enumerate(zip(oa, ob)):
        if x == y:
            continue
        bx = struct.unpack_from(fmt, a, i * width)[0]
        by = struct.unpack_from(fmt, b, i * width)[0]
        if is_nan(bx, dtype) and is_nan(by, dtype):
            continue
        worst = max(worst, abs(x - y))
    return worst


# --------------------------------------------------------------------------
# ELF helpers (TPC objects are ELF32 little-endian)
# --------------------------------------------------------------------------

def find_section(data, name):
    if data[:4] != b"\x7fELF" or data[4] != 1 or data[5] != 1:
        raise ValueError("not an ELF32 little-endian object")
    shoff = struct.unpack_from("<I", data, 0x20)[0]
    shentsize, shnum, shstrndx = struct.unpack_from("<HHH", data, 0x2E)
    strtab = struct.unpack_from("<I", data, shoff + shstrndx * shentsize + 0x10)[0]
    for i in range(shnum):
        sh_name, _, _, _, sh_offset, sh_size = struct.unpack_from("<6I", data, shoff + i * shentsize)
        start = strtab + sh_name
        if data[start:data.index(b"\0", start)] == name:
            return sh_offset, sh_size
    raise ValueError(f"section {name.decode()} not found")


def section_bytes(path, name=METADATA):
    data = path.read_bytes()
    off, size = find_section(data, name)
    return data[off:off + size]


def metadata_version(path):
    return struct.unpack_from("<I", section_bytes(path))[0]


def transplant_metadata(fork_obj, ref_obj, out_obj):
    data = bytearray(fork_obj.read_bytes())
    off, size = find_section(data, METADATA)
    ref_meta = section_bytes(ref_obj)
    if len(ref_meta) != size:
        raise ValueError(f"metadata size differs (fork {size}, reference {len(ref_meta)})")
    data[off:off + size] = ref_meta
    out_obj.write_bytes(data)


# --------------------------------------------------------------------------
# Tool invocation
# --------------------------------------------------------------------------

@dataclass
class Compiler:
    role: str  # "ref", "baseline" or "candidate"
    path: Path
    flags: list


def run(cmd, timeout, env=None):
    try:
        p = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout, env=env)
    except subprocess.TimeoutExpired:
        return "timeout", ""
    if p.returncode == 0:
        return "ok", p.stdout + p.stderr
    status = "crash" if p.returncode < 0 or p.returncode >= 128 else "error"
    return status, p.stdout + p.stderr


def compile_kernel(cc, src, arch, obj, extra):
    # -fno-crash-diagnostics: crashes are expected for known bugs; don't write
    # reproducer files into /tmp on every run.
    cmd = [str(cc.path), f"-march={arch}", "-fno-crash-diagnostics", "-c", str(src),
           "-o", str(obj), *cc.flags, *extra]
    return run(cmd, timeout=600)


def build_runner(tpc_home, out_dir):
    lib = tpc_home / "usr/lib/habanatools"
    src = HERE / "sim_runner.cpp"
    exe = out_dir / "sim_runner"
    if exe.exists() and exe.stat().st_mtime >= src.stat().st_mtime:
        return exe
    cxx = os.environ.get("CXX", "c++")
    cmd = [cxx, "-std=c++17", "-O2", f"-I{lib}/include", str(src), "-o", str(exe),
           f"-L{lib}", "-ltpc_tests_core_ext", "-ltpcsim_shared",
           # DT_RPATH (not RUNPATH) so the libraries' own dependencies resolve too.
           f"-Wl,-rpath-link,{lib}", f"-Wl,--disable-new-dtags,-rpath,{lib}"]
    status, log = run(cmd, timeout=600)
    if status != "ok":
        fatal(f"building sim_runner failed:\n{log}")
    return exe


# --------------------------------------------------------------------------
# Functional mode
# --------------------------------------------------------------------------

def simulate(k, arch, obj, work, tag, runner, env):
    outs = []
    lines = [f"device {arch}", f"elf {obj}", "index_space " + " ".join(map(str, k.index_space))]
    lines += [f"scalar {s:08x}" for s in k.scalars]
    for i, t in enumerate(k.tensors):
        size, gc_type, _ = DTYPES[t.dtype]
        f = work / (f"{tag}.out{i}.bin" if t.is_output else f"in{i}.bin")
        if t.is_output:
            outs.append((t, f))
        dims = " ".join(map(str, t.shape))
        lines.append(f"tensor {'out' if t.is_output else 'in'} {size} {gc_type} {dims} {f}")
    spec = work / f"{tag}.spec"
    spec.write_text("\n".join(lines) + "\n")
    status, log = run([str(runner), str(spec)], timeout=900, env=env)
    if status != "ok":
        return status, log, None
    return "ok", log, [(t, f.read_bytes()) for t, f in outs]


def outputs_match(a, b, k):
    for (t, x), (_, y) in zip(a, b):
        if x == y:
            continue
        if k.ulp and t.dtype in FLOAT_DTYPES and max_ulp_diff(x, y, t.dtype) <= k.ulp:
            continue
        return False
    return True


def describe_mismatch(a, b, k):
    for i, ((t, x), (_, y)) in enumerate(zip(a, b)):
        if x == y:
            continue
        size = DTYPES[t.dtype][0]
        bad = sum(x[j:j + size] != y[j:j + size] for j in range(0, len(x), size))
        detail = f"output {i} ({t.dtype}): {bad}/{t.count} elements differ"
        if t.dtype in FLOAT_DTYPES:
            detail += f", max {max_ulp_diff(x, y, t.dtype)} ulp"
        return detail
    return "outputs differ"


def run_functional_case(k, arch, compilers, args, runner, env):
    work = args.out / k.name / arch
    work.mkdir(parents=True, exist_ok=True)
    rng = random.Random(f"{args.seed}:{k.name}:{arch}")
    for i, t in enumerate(k.tensors):
        if not t.is_output:
            (work / f"in{i}.bin").write_bytes(gen_input(t, rng))

    objs, results = {}, {}
    for cc in compilers:
        obj = work / f"{cc.role}.o"
        status, log = compile_kernel(cc, k.path, arch, obj, k.cflags)
        if status != "ok":
            results[cc.role] = (f"compile-{status}", log)
        else:
            objs[cc.role] = obj
    if "ref" not in objs:
        return "SKIP", "reference compiler cannot build this kernel/arch"

    notes = []
    if "baseline" in objs and "candidate" in objs:
        if (metadata_version(objs["baseline"]) == metadata_version(objs["candidate"]) and
                section_bytes(objs["baseline"]) != section_bytes(objs["candidate"])):
            notes.append("candidate .tpc_metadata differs from baseline")

    for role, obj in objs.items():
        if role == "baseline" and metadata_version(obj) < METADATA_VERSION:
            patched = work / f"{role}.sim.o"
            try:
                transplant_metadata(obj, objs["ref"], patched)
            except ValueError as e:
                results[role] = ("metadata-error", str(e))
                continue
            obj = patched
        status, log, outs = simulate(k, arch, obj, work, role, runner, env)
        results[role] = ("ok", outs) if status == "ok" else (f"sim-{status}", log)

    ref = results["ref"]
    if ref[0] != "ok":
        return "FAIL", f"reference failed to simulate: {ref[1].strip().splitlines()[-1:]}"

    def same(role_a, role_b):
        a, b = results.get(role_a), results.get(role_b)
        if a is None or b is None or a[0] != b[0]:
            return False
        return a[0] != "ok" or outputs_match(a[1], b[1], k)

    cand = results["candidate"]
    if cand[0] == "ok":
        difference = describe_mismatch(ref[1], cand[1], k)
    else:
        difference = f"candidate {cand[0]}: {last_line(cand[1])}"
    if same("candidate", "ref"):
        verdict, detail = "PASS", ""
    elif "baseline" in results and same("candidate", "baseline"):
        verdict, detail = "KNOWN", f"same as baseline; {difference}"
    else:
        verdict, detail = "FAIL", difference
    if notes:
        verdict = "FAIL"
        detail = "; ".join(filter(None, [detail] + notes))
    return verdict, detail


def last_line(text):
    """The most informative line of a tool's output: an assertion or error if any."""
    lines = [l.strip() for l in str(text).splitlines() if l.strip()]
    for pattern in ("Assertion", "error:", "Error"):
        for l in lines:
            if pattern in l:
                return l[:200]
    return lines[-1][:200] if lines else ""


# --------------------------------------------------------------------------
# Compile-parity mode
# --------------------------------------------------------------------------

def parity_cases(paths, archs):
    for p in paths:
        files = sorted(p.rglob("*.c")) if p.is_dir() else [p]
        for f in files:
            text = f.read_text(errors="replace")
            marchs = set(re.findall(r"RUN:.*?(?:-march=|-mcpu=|-target-cpu\s+)([a-z0-9]+)", text))
            for arch in archs:
                if marchs & PARITY_RUN_NAMES.get(arch, {arch}):
                    yield f, arch


def run_parity_case(src, arch, compilers):
    status = {}
    for cc in compilers:
        status[cc.role], _ = compile_kernel(cc, src, arch, "/dev/null", ["-O2"])
    if status["candidate"] == status["ref"]:
        return "PASS", ""
    rel = Path(os.path.relpath(src.resolve(), REPO)).as_posix()
    if (status.get("baseline") == status["candidate"] or
            KNOWN_PARITY.get((rel, arch)) == (status["ref"], status["candidate"])):
        return "KNOWN", f"reference {status['ref']}, fork {status['candidate']}"
    detail = ", ".join(f"{r} {s}" for r, s in status.items())
    return "FAIL", detail


# --------------------------------------------------------------------------

def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("kernels", nargs="*", type=Path,
                    help="kernel files (default: all in kernels/), or sources/dirs with --compile-parity")
    ap.add_argument("--tpc-home", type=Path, default=os.environ.get("TPC_HOME"),
                    help="habanatools root (default: $TPC_HOME)")
    ap.add_argument("--ref", type=Path, help="reference tpc-clang (default: $TPC_HOME/usr/bin/tpc-clang)")
    ap.add_argument("--candidate", type=Path, default=REPO / "build/bin/tpc-clang",
                    help="fork compiler under test (default: build/bin/tpc-clang)")
    ap.add_argument("--baseline", type=Path, help="known-good fork compiler to tell regressions from pre-existing differences")
    for role in ("ref", "candidate", "baseline"):
        ap.add_argument(f"--{role}-flags", default="", help=f"extra flags for the {role} compiler")
    ap.add_argument("--cflags", default="-O2", help="flags for every compiler (default: -O2)")
    ap.add_argument("--arch", help="comma-separated archs to test (default: each kernel's list)")
    ap.add_argument("--out", type=Path, default=REPO / "build/tpc-difftest", help="work directory")
    ap.add_argument("--seed", type=int, default=1)
    ap.add_argument("-j", "--jobs", type=int, default=max(1, (os.cpu_count() or 2) // 2))
    ap.add_argument("--compile-parity", action="store_true", help="compare only whether sources compile")
    ap.add_argument("-v", "--verbose", action="store_true", help="also list passing cases")
    args = ap.parse_args()

    if args.tpc_home is None and args.ref is None:
        fatal("set TPC_HOME (e.g. 'source ~/tpc-1.24.2/env.sh') or pass --tpc-home/--ref")
    tpc_home = Path(args.tpc_home) if args.tpc_home else None
    ref = args.ref or tpc_home / "usr/bin/tpc-clang"
    common = shlex.split(args.cflags)
    compilers = [Compiler("ref", ref, common + shlex.split(args.ref_flags)),
                 Compiler("candidate", args.candidate, common + shlex.split(args.candidate_flags))]
    if args.baseline:
        compilers.append(Compiler("baseline", args.baseline, common + shlex.split(args.baseline_flags)))
    for cc in compilers:
        if not cc.path.is_file():
            fatal(f"{cc.role} compiler not found: {cc.path}")
    only_archs = args.arch.split(",") if args.arch else None
    args.out = args.out.resolve()
    args.out.mkdir(parents=True, exist_ok=True)

    jobs = []
    with concurrent.futures.ThreadPoolExecutor(args.jobs) as pool:
        if args.compile_parity:
            paths = args.kernels or [REPO / "clang/test/RC99"]
            for src, arch in parity_cases(paths, only_archs or PARITY_ARCHS):
                jobs.append((f"{src.relative_to(REPO) if src.is_relative_to(REPO) else src} [{arch}]",
                             pool.submit(run_parity_case, src, arch, compilers)))
        else:
            if tpc_home is None:
                fatal("functional mode needs --tpc-home or $TPC_HOME for the simulator")
            runner = build_runner(tpc_home, args.out)
            env = dict(os.environ)
            lib = str(tpc_home / "usr/lib/habanatools")
            env["LD_LIBRARY_PATH"] = lib + os.pathsep + env.get("LD_LIBRARY_PATH", "")
            files = args.kernels or sorted((HERE / "kernels").glob("*.c"))
            for f in files:
                k = parse_kernel(f.resolve())
                for arch in k.archs:
                    if arch not in SIM_ARCHS or (only_archs and arch not in only_archs):
                        continue
                    jobs.append((f"{k.name} [{arch}]",
                                 pool.submit(run_functional_case, k, arch, compilers, args, runner, env)))

        counts = {}
        for name, fut in jobs:
            verdict, detail = fut.result()
            counts[verdict] = counts.get(verdict, 0) + 1
            if verdict != "PASS" or args.verbose:
                print(f"{verdict:5}  {name}" + (f"  -- {detail}" if detail else ""), flush=True)

    summary = ", ".join(f"{counts.get(v, 0)} {v}" for v in ("PASS", "KNOWN", "FAIL", "SKIP"))
    print(f"\n{len(jobs)} cases: {summary}")
    return 1 if counts.get("FAIL") else 0


if __name__ == "__main__":
    sys.exit(main())
