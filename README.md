# TPC-CLANG compiler
TPC-CLANG compiler is based on the widely used [LLVM](https://llvm.org/) open-source compiler infrastructure project. The tpc-clang compiler compiles a  TPC C programming language (also referred to as TPC C). TPC-C is based on the ISO/IEC 9899:1999 C language specification with TPC-specific extensions and restrictions.

<img style="float: right;
   height:200px;" src=TPC-compiler.jpg >

## Table of Contents
- [TPC-CLANG compiler](#tpc-clang-compiler)
  - [Table of Contents](#table-of-contents)
  - [Performance](#performance)
  - [Clone tpc-llvm Repository](#clone-tpc-llvm-repository)
  - [Building the Compiler](#building-the-compiler)
    - [Requirements](#requirements)
    - [Build LLVM from source](#build-llvm-from-source)
    - [Using a Build Script](#using-a-build-script)
  - [Working with the Compiler](#working-with-the-compiler)
  - [Specific Compiler Options](#specific-compiler-options)
  - [Pragmas](#pragmas)
  - [Macros](#macros)
  - [Intrinsics](#intrinsics)
  - [Disassembler](#disassembler)
    - [Typical Invocation:](#typical-invocation)
    - [Other TPC-Specific Options:](#other-tpc-specific-options)
    - [How to Disassemble Binary: Wrap Binary to Elf](#how-to-disassemble-binary-wrap-binary-to-elf)
  - [Assembler](#assembler)

## Performance

The code this fork generates, against that of Habana's own tpc-clang from the
habanalabs-tools 1.24.2 SDK, timed on a Gaudi2 card (HL-225, driver 1.24.2):
33 microbenchmarks from [`tpc/bench/kernels/`](tpc/bench/kernels), compiled
for gaudi2 at -O2. Taken together the two compilers are even (geometric mean
1.00×).

* **Faster:** RMSNorm (1.84×), LayerNorm (1.36×) and the fused residual add
  and RMSNorm (1.34×). Each reduces over the hidden dimension in a hardware
  loop that the fork unrolls automatically. With that unrolling turned off,
  they run at the speed of Habana's code on the simulator, except RMSNorm,
  which keeps 1.10×.
* **Slower:** staging data in vector local memory (0.45×), whose accesses
  Habana's compiler keeps in registers and the fork does not yet; the 3-D
  stencil (0.90×); and kernels that spend most of their time in tpclib math
  functions, such as the plasma effect (0.87×) and Black-Scholes (0.95×).
* **Same code:** five benchmarks compile to identical code with both
  compilers. They differ by at most 0.2%, which is the measurement noise.

| Kernel | Habana (TPC-µs) | Fork (TPC-µs) | Fork speedup |
|---|---:|---:|---:|
| `general/add_f32` | 37.2 | 37.3 | 1.00× |
| `general/add_sat_u8` | 37.2 | 37.3 | 1.00× |
| `general/axpy_bf16` | 37.3 | 37.1 | 1.01× |
| `general/cast_f32_bf16` | 18.7 | 18.7 | 1.00× |
| `general/local_mem_f32` | 18.6 | 41.3 | 0.45× |
| `general/prefix_sum_f32` | 32.0 | 32.0 | 1.00× |
| `general/reduce_sum_f32` | 27.5 | 27.5 | 1.00× |
| `general/sdiv_i32` | 703.4 | 726.3 | 0.97× |
| `general/short_rows_f32` | 59.8 | 59.8 | 1.00× |
| `general/where_f32` | 59.8 | 59.8 | 1.00× |
| `hpc/blackscholes_f32` | 1420.3 | 1496.5 | 0.95× |
| `hpc/fft_radix4_f32` | 49.1 | 49.0 | 1.00× |
| `hpc/jacobi2d_f32` | 136.9 | 136.9 | 1.00× |
| `hpc/laplace3d_f32` | 240.8 | 268.2 | 0.90× |
| `hpc/montecarlo_pi_f32` | 701.4 | 701.4 | 1.00× |
| `hpc/nbody_f32` | 1312.3 | 1303.2 | 1.01× |
| `hpc/rk4_duffing_f32` | 460.0 | 460.2 | 1.00× |
| `misc/game_of_life_i32` | 200.6 | 200.5 | 1.00× |
| `misc/mandelbrot_f32` | 1256.4 | 1256.3 | 1.00× |
| `misc/plasma_rgb_f32` | 348.2 | 401.0 | 0.87× |
| `misc/raytrace_spheres_f32` | 722.9 | 745.1 | 0.97× |
| `ml/add_rmsnorm_bf16` | 132.2 | 98.7 | 1.34× |
| `ml/causal_conv1d_bf16` | 423.7 | 441.9 | 0.96× |
| `ml/decode_attention_bf16` | 934.2 | 925.3 | 1.01× |
| `ml/dequant_int4_f32` | 52.6 | 51.5 | 1.02× |
| `ml/fp8_quant_bf16` | 159.7 | 166.6 | 0.96× |
| `ml/gelu_f32` | 396.6 | 401.2 | 0.99× |
| `ml/layernorm_f32` | 129.9 | 95.7 | 1.36× |
| `ml/moe_router_f32` | 343.4 | 343.4 | 1.00× |
| `ml/rmsnorm_bf16` | 76.2 | 41.3 | 1.84× |
| `ml/rope_neox_bf16` | 112.0 | 112.8 | 0.99× |
| `ml/softmax_f32` | 1094.6 | 1103.7 | 0.99× |
| `ml/swiglu_bf16` | 99.5 | 103.0 | 0.97× |
| geomean, general |  |  | 0.92× |
| geomean, hpc |  |  | 0.98× |
| geomean, misc |  |  | 0.96× |
| geomean, ml |  |  | 1.10× |
| **geomean, all** |  |  | **1.00×** |

Time is in TPC-microseconds: the Habana profiler's start-to-halt time of each
TPC that ran the kernel (2 to 8 per launch), summed, so host and launch
overhead are left out. Each time is the median of 10 runs, which spread by at
most ±0.9%. Speedup is Habana's time divided by the fork's, so above 1× the
fork is faster. Every run's outputs equal those of the other compiler and of
the TPC simulator. Measured on 2026-11-20 with the fork at e23536e7f779; the
method, and how to rerun it, is in [`tpc/README.md`](tpc/README.md#microbenchmarks-bench).

## Clone tpc-llvm Repository

Create a working directory, and clone the repository. For example the folder '~/habanaTools'.

```sh
cd ~/habanaTools
git clone https://github.com/HabanaAI/tpc_llvm.git tpc-llvm
```

From here on, we assume the root of the source tree is '~/habanaTools/tpc-llvm'.

## Building the Compiler

### Requirements
1. [cmake](https://cmake.org/) must be installed on the build machine.
2. cmake version 3.4.3 or higher is required.
3. [gcc 5.1 | clang 6.0] or higher 
4. Python 3.6.9 or higher

Create a build directory, for examples '~/habanaTools/build'  (the folder can be located anywhere), and enter the folder:

```sh
cd ~/habanaTools
mkdir build
cd build
```

### Build LLVM from source
```sh
cmake -G "Unix Makefiles"  \
      -DLLVM_ENABLE_PROJECTS=clang \
      -DLLVM_TARGETS_TO_BUILD="TPC" \
      -DLLVM_BUILD_EXAMPLES=OFF \
      -DLLVM_INCLUDE_EXAMPLES=OFF \
      -DCLANG_ENABLE_ARCMT=OFF \
      -DCLANG_BUILD_EXAMPLES=OFF \
      ../tpc-llvm/llvm
```
Run the build:
```sh
make clang llc opt llvm-objdump
```
The build must finish successfully.

### Using a Build Script
From build directory run:
```sh
../tpc-llvm/buildTPC_CLANG.sh
```

```
usage: buildTPC_CLANG.sh [options]

options:

  -r,  --release              Build only release build
  -j,  --jobs <val>           Overwrite number of jobs
  -h,  --help                 Prints this help
```

## Working with the Compiler

The build must create compiler executable 'tpc-clang' in the directory ~/habanaTools/build/bin'. It is used as usually, for instance, the next invocations may be used to compile source file to an object, ASM or llvm IR respectively:

```sh
~/habanaTools/build/bin/tpc-clang ~/habanaTools/tpc-llvm/codeExample/leakyrelu_f32.c -c
~/habanaTools/build/bin/tpc-clang ~/habanaTools/tpc-llvm/codeExample/leakyrelu_f32.c -S
~/habanaTools/build/bin/tpc-clang ~/habanaTools/tpc-llvm/codeExample/leakyrelu_f32.c -S -emit-llvm
```

## Specific Compiler Options


* **Optimization levels:** -O2 and -O0 are maximum and minimum levels supported, -O2 is the default level.
  * **-O1** turns off HW loops, loop pipelining, and few other optimizations.
  * **-O0** turns off instruction scheduling and bundling plus pads all instructions with 6 NOPs to ensure results queue is committed to register file before the next instruction executed.
* **-march=\<name\>**     Architecture switch, currently supported names are "goya", "gaudi"
* **-max-tensors \<n\>**  Tensor limit, n is a number in range 0..16. Default is architecture-dependent (8 for goya and 16 for others).
* **-vlm \<n\>**    Vector local memory limit, n is the amount of memory in KB. Default is 80KB if the program does not use LOOKUP instruction and 16KB otherwise. Beware that the compiler can need some VLM space for register spilling.
* **-slm \<n\>**    Scalar local memory limit, n is the amount of memory in KB. Default is architecture-dependent (1 for goya/gaudi). Beware that the compiler can need some SLM space for register spilling.
* **-spill-vlm N**    Limit VLM amount available for spills, abort compilation if this limit gets exceeded. If not set, the whole VLM can be used.
* **-main-function \<main_entry_name\>**  Name of entry function. Default is "main".
* **-all-loops-taken**  Enable global elimination of loop end padding as loops are always taken.
* **-use-printf**   Reserve tensor for printf, regardless of compiled code contents.
* **-reg-mem-count**    Output usage of registers and local memory.
* **-disable-lut-warn** Suppress performance warning when used LUT size exceeds LUT cache
* **-mllvm -ignore-mem-overflow=1**     Do not abort compilation if local memory limit is exceeded. A warning message is still issued.
* **-mllvm -loop-unswitch-threshold=N** Enable transformation of loops that contain branches on loop-invariant conditions to multiple loops. This can increase the size of the code exponentially (doubling it every time a loop is unswitched), so we only unswitch if the resultant code will be smaller than a threshold. Working values usually start from 500.
* **-mllvm -tpc-default-latency=N**   Latency for operands undefined in latency DB. Default is 7, suboptimal but guaranteed to be working.
* **--tpcversion**    Print TPC version information (git revision hash).

## Pragmas

* **\#pragma unroll(UnrollCount)**  enables standard llvm unrolling optimization for the loop.
* **\#pragma loop_unroll(UnrollCount)**   enables tpc backend unrolling optimization. The number of iterations of the loop should be a multiple of UnrollCount. To also pipeline, the loop option 'pipelined' should be added. In this case the number of iteration of the loop should also be at least two times larger than UnrollCount. Option 'taken' tells the compiler that the loop always executes at least one time. 
* **\#pragma loop_taken**   tells compiler that the loop always executes at least 1 time. 
Examples:
* **\#pragma unroll(2)** - unroll two iterations using high-level llvm algorithm
* **\#pragma loop_unroll(4)** - unroll 4 iterations using tpc backend algorithm
* **\#pragma loop_unroll(3) pipelined taken** - pipeline 3 iterations, assume loop is always entered and has at least 6 iterations.

## Macros

* **\_\_TPC\_\_**                 common architecture marker
* **\_\_goya\_\_**                architecture generation 1
* **\_\_gaudi\_\_**               architecture generation 2
* **\_\_X_TENSORS=\<n\>**       reflects actual tensor limit
* **MAX_SLM=\<n\>**           reflects architecture scalar memory limit (in bytes)
* **MAX_VLM=\<n\>**           reflects architecture vector memory limit (in bytes)
* **\_\_HABANA_TOOL_VERSION**   Public software version (e.g. recent was 0.10.0)
* **\_\_TPC_DROP_VERSION**      Internal drop version (e.g. recent was 18.0.1)
* **VERSION2DEC(a,b,c)**      Macro to express an expected version (as "a.b.c" triple)

Typical check for compiler version is like this:
```C++
#if __TPC_DROP_VERSION >= VERSION2DEC(16, 0, 0)
    //use some recent feature
#else
    //fall back to legacy way
#endif
```

## Intrinsics
See **tpc-intrinsics.h** and **tpc-defs.h** header files (NB: these headers are documentation only, not intended for real inclusion into source code).

## Disassembler
### Typical Invocation:

```sh
llvm-objdump --triple tpc -d -no-show-raw-insn -no-leading-addr -mcpu=gaudi mytest.o
```

### Other TPC-Specific Options:
* **-tpc-encoding-info**  output human-readable decomposition of VLIW instruction fields. It may be used with or without -d.
* **-no-common-header** suppress output of non-assembly text. Useful to stream disassembled sources back to the assembler.

### How to Disassemble Binary: Wrap Binary to Elf
1) Create an empty ELF container:
```sh
echo "" |tpc-clang -o empty.o -c -xassembler -
```

2) Embed the binary to ELF as .text section (and strip irrelevant sections away):
```sh
objcopy --update-section .text=path/to/file.bin -j .text empty.o result.o
```

3) Disassemble the result ELF into a file name 'result.s':
```sh
llvm-objdump --triple tpc -mcpu=gaudi -d -no-show-raw-insn -no-leading-addr -no-common-header result.o > result.s
```
## Assembler
Typically compiler handles assembler source files (*.tpcasm and *.s) the same way as c/cpp sources. No special care is needed. E.g.:
```sh
tpc-clang -c -march=goya my.s
```
