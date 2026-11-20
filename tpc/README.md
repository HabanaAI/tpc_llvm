# TPC fork tooling

Downstream-only tooling for maintaining this TPC fork of LLVM. Nothing in this
directory exists upstream, so upstream merges never touch it, and it is not part
of the LLVM CMake build.

## Differential tests (`difftest/`)

`difftest/run.py` checks the fork compiler for functional correctness against
Habana's closed-source `tpc-clang`. It compiles every kernel in
`difftest/kernels/` with both compilers, runs the objects on the TPC simulator
with identical, fixed-seed inputs, and compares the output tensors bit for bit.

### Setup

It needs an extracted `habanalabs-tools` package (closed compiler + simulator),
located through `TPC_HOME`:

```sh
source ~/tpc-1.24.2/env.sh        # sets TPC_HOME, PATH, LD_LIBRARY_PATH
```

The habanatools headers and libraries are Intel-confidential; they are used in
place and must never be copied into this repository. `run.py` builds its small
simulator driver (`sim_runner.cpp`) against them on first use.

### Running

```sh
# Candidate = build/bin/tpc-clang by default.
tpc/difftest/run.py

# During the upstream port: compare a new build against the last good one, so
# pre-existing differences from Habana's compiler are not reported as failures.
tpc/difftest/run.py --candidate build-port/bin/tpc-clang --baseline build/bin/tpc-clang

# A/B the same compiler with different flags.
tpc/difftest/run.py --candidate-flags=-fno-unroll-loops

# Only some kernels or architectures.
tpc/difftest/run.py --arch gaudi2 tpc/difftest/kernels/add_f32.c

# Another optimization level (the default is -O2). -O0 and -O1 take different
# paths through the pipeline, so check them when changing it.
tpc/difftest/run.py --cflags=-O0

# Compile parity: does every RC99 lit-test source still compile (or still fail
# to compile) the same way Habana's compiler and the baseline do?
tpc/difftest/run.py --compile-parity --baseline build/bin/tpc-clang
```

Results, one line per kernel and architecture:

| Verdict | Meaning |
|---------|---------|
| `PASS`  | Candidate output equals the reference output. |
| `KNOWN` | Candidate differs from the reference exactly like the baseline does (a pre-existing difference), or, in compile parity, as `KNOWN_PARITY` records. |
| `FAIL`  | Anything else, including a change in the candidate's `.tpc_metadata` relative to a baseline with the same metadata version. |
| `SKIP`  | The reference compiler cannot build the kernel for that architecture. |

`run.py` exits non-zero on any `FAIL`. Work files (objects, inputs, outputs) are
kept under `build/tpc-difftest/<kernel>/<arch>/` for debugging.

### Writing a kernel

A kernel is a TPC-C file whose tensors and launch geometry are declared in
`// DIFFTEST:` comments, much like lit `RUN:` lines:

```c
// DIFFTEST: arch gaudi gaudi2             simulated archs: gaudi, gaudi2, gaudi3
// DIFFTEST: in  f32 256 6 3 range=-4,4    input tensor: dtype, dims (dim 0 first), value range
// DIFFTEST: out f32 256 6 3               output tensor
// DIFFTEST: index-space 4 6 3             index-space geometry (missing dims = 1)
// DIFFTEST: scalar f32 0.5                scalar kernel parameter (f32, i32, u32), in order
// DIFFTEST: tolerance ulp 2               optional: allow small float differences
// DIFFTEST: cflags -O1                    optional: extra flags for every compiler
```

As with the graph compiler, inputs come before outputs: list all `in` lines
before any `out` line, in the same order as the kernel's `tensor` parameters.
Data types: `f32 bf16 f16 i32 u32 i16 u16 i8 u8 f8_152 f8_143`. Unwritten
output elements keep the fill byte `0xCD`.

Keep each kernel focused on one feature, so a failure points at a cause.

Habana's compiler is the reference for all behaviour, including where it differs
from C. For example, with a non-constant divisor both compilers take the absolute
value with `abs.i32`, which saturates, so `INT_MIN % 3` gives -1 rather than the
-2 that C expects. `abs_i32`, `sdiv_vec_edge_i32` and `srem_vec_edge_i32` cover
such inputs.

### Known issues

* **Old baselines.** Fork compilers older than the version 20 `.tpc_metadata`
  change write version 12, which current habanatools cannot run: the missing
  `irf32Mode` makes the simulator assume irf44 mode. `run.py` runs such a baseline
  with the reference object's metadata section, and does not compare its metadata
  with the candidate's.
* **Metadata the fork does not compute.** `targetSpecific`, `regIDUsed`,
  `duplicateTensors`, `exclusiveSwitch` and `unsetSmallVLM` stay 0, and
  `hashedISA` does not match Habana's values. The simulator accepts this, but
  nothing has checked it against the graph compiler or hardware.
* **Unsigned vector `/` and `%` with an operand of 2^31 or more.** Both compilers
  take the absolute value of the operands with `abs.i32`, as they do for signed
  values, so these operands act as negative numbers and the results differ from C.
  Scalar unsigned `/` and `%` are correct. `udiv_vec_u32` and `urem_vec_u32` record
  Habana's behaviour.
* **`gaudi3`.** The fork's name for it is `doron1`, and it accepts both. The
  kernels run as `gaudi3`; compile parity builds the sources whose RUN lines name
  `doron1` or `gaudi3` with `-march=gaudi3`, since Habana's compiler has no
  `doron1`. The LLVM 12 baseline cannot build `gaudi3`, so differences from
  Habana that predate the fork's `gaudi3` support are listed in `KNOWN_PARITY`
  in `run.py`. Habana's compiler uses V40-V47 as ordinary vector registers on
  `gaudi3`; the fork does not yet.

## Microbenchmarks (`bench/`)

`bench/bench.py` times kernels on a Gaudi card; the performance figures in the
top-level `README.md` come from it. It is separate from the differential tests:
those check outputs on small inputs, while the benchmarks run larger inputs
through kernels picked for timing, and check outputs only so that every
comparison is between kernels that compute the same thing.

A benchmark is a TPC-C file in `bench/kernels/<category>/` whose tensors and
launch geometry are declared in `// BENCH:` lines, with the same directives as
`// DIFFTEST:` (see "Writing a kernel" above). Its leading comment says what it
measures and, for model kernels, where the operation comes from. Benchmarks are
named `<category>/<name>`, and the report gives a geometric mean per category.

| Category | Kernels |
|----------|---------|
| `general/` | Basic patterns: streaming elementwise ops, select, 8-bit lanes, conversion, reductions, scans, integer division, short loops, vector local memory. |
| `ml/` | Inference kernels, many modeled on operations of DeepSeek-V3 and Qwen inference (vLLM, DeepSeek's reference code): RMSNorm with and without the fused residual add, LayerNorm, softmax, GELU, SwiGLU, RoPE, decode attention, MoE routing, FP8 quantization, int4 dequantization, causal conv1d. |
| `hpc/` | Scientific workloads: 2-D and 3-D stencils, an FFT stage, an ODE integrator, Monte Carlo, N-body, Black-Scholes. |
| `misc/` | Anything else, for now graphics: Mandelbrot, ray tracing, Game of Life, a plasma effect. |

### Running

There are three steps: build a bundle where the compilers are, run it on the
Gaudi machine, then print the results anywhere.

```sh
# 1. Compile every kernel with Habana's compiler (the reference) and the fork,
#    run both on the simulator, check the outputs are equal, write a bundle.
source ~/tpc-1.24.2/env.sh
tpc/bench/bench.py build --fork build/bin/tpc-clang --out build/tpc-bench

# 2. Copy the bundle to the Gaudi machine and time it there. The machine needs
#    python3, a C++ compiler and habanatools (/usr/lib/habanatools, or --lib).
rsync -a build/tpc-bench/ gaudi:tpc-bench/
ssh gaudi 'cd tpc-bench && setsid nohup python3 bench.py run --reps 10 > run.log 2>&1 < /dev/null &'

# 3. Copy the results back and print them; --markdown prints the README table.
rsync -a gaudi:tpc-bench/results.jsonl gaudi:tpc-bench/hw_env.json build/tpc-bench/
tpc/bench/bench.py report build/tpc-bench
```

`--variant NAME=COMPILER[@FLAGS]` (repeatable) replaces the default pair. The
first variant is the reference. For example, this times a fork change against
the build before it:

```sh
tpc/bench/bench.py build --out build/tpc-bench-unroll \
    --variant before=/path/to/old/tpc-clang --variant after=build/bin/tpc-clang
```

`run --filter REGEX` times only the benchmarks whose name matches, for example
`--filter '^ml/'`. `run` appends to `results.jsonl`, so another `run` adds
repetitions to the ones already there, and the report pools them. It prints
each run as it goes; `bench.py report` works at any point.

### What is measured

* **Simulator cycles** (`build`): the TPC simulator runs every index-space
  member on one TPC and reports total cycles. It models no memory stalls and
  no taken-jump penalty (about 7 cycles per taken `jmpr` on Gaudi2), so
  memory-bound kernels and branchy loops take longer on the card than the
  cycle count says.
* **Card time** (`run`): the Habana profiler (`HABANA_PROFILE=1`) records when
  each TPC starts and halts; a TPC starts once per launch and runs all of its
  index-space members before it halts. A kernel's card time is the sum of
  those start-to-halt times over the TPCs, in TPC-microseconds. This leaves out
  host and launch overhead, which the compiler does not control. The profiler
  counts in 20 ns ticks, quantized to 4 ticks. The report gives the
  median over repetitions, and ± is half the min-max range. Variants of a
  kernel run back to back, in an order that rotates every repetition.
* **Speedup** is reference time / variant time, so a value above 1 means the
  variant is faster. The geometric mean is over the kernels with results.
* **Validity:** every variant's simulator outputs must equal the reference's
  (bit for bit, or within the kernel's `tolerance`), and the card's outputs
  must equal the simulator's on every run. A kernel that fails either check is
  reported and left out of the speedups. A kernel that both compilers build to
  identical code shows the card's run-to-run noise.

### Safety on the card

* A kernel spec with no tensors hangs the card: the driver resets it after
  30 s. Every benchmark declares an output tensor, and `run` refuses a spec
  without tensors.
* `run` stops at the first failed or traceless run.
* A card opens in one process at a time. `run` refuses to start while another
  process holds a `/dev/accel/accelN` device (a serving job, for example), and
  never stops such processes. `--force` tries anyway.
* The card returns 0xFF in output elements a kernel does not write; the
  simulator returns 0xCD. Their outputs could not be compared, so `build`
  rejects a kernel that leaves output elements unwritten.

### Writing a benchmark

Keep each kernel to one kind of work, so a speedup points at a cause. Give it
enough work for the reference to take at least ~20000 simulator cycles, and
keep each tensor to a few MB, since the bundle is copied to the Gaudi machine.
Write every output element. Name the code paths it exercises in the leading
comment.
