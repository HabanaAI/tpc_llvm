# TPC-LLVM compiler

## Table of Contents
- [TPC-LLVM compiler](#tpc-llvm-compiler)
  - [Table of Contents](#table-of-contents)
  - [Getting Sources](#getting-sources)
  - [Building Compiler](#building-compiler)
    - [Requirements](#requirements)
    - [Build LLVM From Source](#build-llvm-from-source)
    - [Using a build script](#using-a-build-script)
  - [Working with compiler](#working-with-compiler)
  - [Specific compiler options](#specific-compiler-options)
  - [Pragmas](#pragmas)
  - [Macros](#macros)
  - [Intrinsics](#intrinsics)
  - [Disassembler](#disassembler)
    - [Typical Invocation:](#typical-invocation)
    - [Other Tpc-Specific Options:](#other-tpc-specific-options)
    - [How to Disassemble Binary: Wrap Binary to Elf](#how-to-disassemble-binary-wrap-binary-to-elf)
  - [Assembler](#assembler)

## Getting Sources

Create a working directory. Below we use '~/habanaTools' as an example.

```
cd ~/habanaTools
git clone  <location of delivered llvm repository> tpc-llvm
```

Further, we assume the root of the source tree is '~/habanaTools/tpc-llvm'.

## Building Compiler

### Requirements
1. cmake must be installed on the build machine.
2. cmake version 3.4.3 or higher is required.
3. [gcc 5.1|clang 6.0] or higher  

Create a build directory, in the examples, we use '~/habanaTools/build', but it can be anywhere.

```
cd ~/habanaTools
mkdir build
cd build
```

### Build LLVM From Source
```
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
```
make clang llc opt llvm-objdump
```
The build must finish successfully.

### Using a build script
From build dircotry run:
```
../buildTPC_LLVM.sh
```

## Working with compiler

The build must create compiler executable 'tpc-clang' in the directory ~/habanaTools/build/bin'. For instance, the next invocations may be used to compile source file to an object, ASM or llvm IR respectively:

```
$~/habanaTools/build/bin/tpc-clang ~/habanaTools/tpc-llvm/codeExample/leakyrelu_f32.c -c  
$~/habanaTools/build/bin/tpc-clang ~/habanaTools/tpc-llvm/codeExample/leakyrelu_f32.c -S  
$~/habanaTools/build/bin/tpc-clang ~/habanaTools/tpc-llvm/codeExample/leakyrelu_f32.c -S -emit-llvm
```

## Specific compiler options


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

* **__TPC__**     common architecture marker
* **__goya__**                architecture generation 1
* **__gaudi__**               architecture generation 2
* **__X_TENSORS=\<n\>**   reflects actual tensor limit
* **MAX_SLM=\<n\>**             reflects architecture scalar memory limit (in bytes)
* **MAX_VLM=\<n\>**             reflects architecture vector memory limit (in bytes)
* **__LONG_IRF__**    marker for extended IRF mode
* **__HABANA_TOOL_VERSION**   Public software version (e.g. recent was 0.10.0)
* **__TPC_DROP_VERSION**      Internal drop version (e.g. recent was 18.0.1)
* **VERSION2DEC(a,b,c)**      Macro to express an expected version (as "a.b.c" triple)

Typical check for compiler version is like this:
```
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

```
llvm-objdump --triple tpc -d -no-show-raw-insn -no-leading-addr -mcpu=gaudi mytest.o
```

### Other Tpc-Specific Options:
* **-tpc-encoding-info**  output human-readable decomposition of VLIW instruction fields. It may be used with or without -d.
* **-no-common-header** suppress output of non-assembly text. Useful to stream disassembled sources back to the assembler.

### How to Disassemble Binary: Wrap Binary to Elf
1) Create an empty ELF container:
```
echo "" |tpc-clang -o empty.o -c -xassembler -
```

2) Embed the binary to ELF as �.text� section (and strip irrelevant sections away):
```
objcopy --update-section .text=path/to/file.bin -j .text empty.o result.o
```

3) Disassemble the result ELF:
```
llvm-objdump --triple tpc -mcpu=gaudi -d -no-show-raw-insn -no-leading-addr -no-common-header result.o > result.s
```
## Assembler
Typically compiler handles assembler source files (*.tpcasm and *.s) the same way as c/cpp sources. No special care is needed. E.g.:
```
tpc-clang -c -march=goya my.s
```