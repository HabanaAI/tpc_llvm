<h1><span style="color: blue;">TPC LLVM PROJECT</span></h1>
===========================================================
Getting sources
---------------

Create working directory, below we use '~/arbeit/habana' as an example.

cd ~/arbeit/habana
git clone  <location of delivered llvm repository> tpc-llvm

Further we assume the root of source tree is '~/arbeit/habana/tpc-llvm'.

Building compiler
-----------------
**Requirement**
+ cmake must be installed on build machine.<br/>
+ cmake version 3.4.3 or higher is required.<br/>
+ gcc 5.1 or higher is required.<br/>

Create build directory, in the examples we use '~/arbeit/habana/build' but it can be anywhere.<br/>

+ cd ~/arbeit/habana<br/>
+ mkdir build<br/>
+ cd build<br/>
+ cmake -G "Unix Makefiles"  \
      -DLLVM_ENABLE_PROJECTS=clang \
      -DLLVM_TARGETS_TO_BUILD="TPC" \
      -DLLVM_BUILD_EXAMPLES=OFF \
      -DLLVM_INCLUDE_EXAMPLES=OFF \
      -DCLANG_ENABLE_ARCMT=OFF \
      -DCLANG_BUILD_EXAMPLES=OFF \
      ../tpc-llvm/llvm<br/>
      
**Run the build:**
+ make clang llc opt llvm-objdump<br/>
The build must finish successfully.<br/>

Working with compiler
---------------------

The build must create compiler executable 'tpc-clang' in the directory ~/arbeit/habana/build/bin'. It is used as usually, for instance, the next invocations may
be used to compile source file to object, asm or llvm ir respectively:<br/>

~/arbeit/habana/build/bin/tpc-clang ~/arbeit/habana/tpc-llvm/clang/test/RC99/misc/reduction.c -c<br/>
~/arbeit/habana/build/bin/tpc-clang ~/arbeit/habana/tpc-llvm/clang/test/RC99/misc/reduction.c -S<br/>
~/arbeit/habana/build/bin/tpc-clang ~/arbeit/habana/tpc-llvm/clang/test/RC99/misc/reduction.c -S -emit-llvm<br/>


Specific compiler options
-------------------------
Optimization levels: -O2 and -O0 are max and min levels supported, -O2 is the default level.<br/>
-O1 turns off HW loops and few other optimizations.<br/>
-O0 turns off instruction scheduling and bundling plus pads all instructions with 6 NOPs to ensure results queue is committed to register file before the next instruction executed.<br/>
**-march=\<name\> Architecture switch, currently supported names are "goya", "gaudi", "gaudib" "greco", "gaudi2"<br/>
**-max-tensors \<n\> Tensor limit, n is a number in range 0..16. Default is architecture-dependent (8 for goya and 16 for others).<br/>
**-vlm \<n\>** Vector local memory limit, n is amount of memory in KB. Default is 80KB if the program does not use LOOKUP instruction and 16KB otherwise. Beware that the compiler can need some VLM space for register spilling.<br/>
**-slm \<n\>** Scalar local memory limit, n is amount of memory in KB. Default is architecture-dependent (1 for goya/gaudi, 2 for greco, 16 for gaudi2). Beware that the compiler can need some SLM space for register spilling.<br/>
**-spill-vlm N** Limit VLM amount available for spills, abort compilation if this limit gets exceeded. If not set, whole VLM can be used.<br/>
**-main-function \<main_entry_name\>**	Name of entry function. Default is "main".<br/>
**-all-loops-taken** Enable global elimination of loop end padding as loops are always taken.<br/>
**-use-printf**	Reserve tensor for printf, regardless of compiled code contents.<br/>
**-reg-mem-count** Output usage of registers and local memory.<br/>
**-disable-lut-warn** Suppress performance warning when used LUT size exceeds LUT cache<br/>
**-mllvm -ignore-mem-overflow=1** 		Do not abort compilation if local memory limit is exceeded. Warning message is still issued.<br/>
**-mllvm -loop-unswitch-threshold=N**	Enable transformation of loops that contain branches on loop-invariant conditions to multiple loops. This can increase the size of the code exponentially (doubling it every time a loop is unswitched) so we only unswitch if the resultant code will be smaller than a threshold. Working values usually start from 500.<br/>
**-instr-compress**		Force instruction compression (Greco and later architectures). Enabled for -O2 level by default and disabled for -O1/-O0.<br/>
**-no-instr-compress**	Disable instruction compression.<br/>
**--tpcversion**		Print TPC version information (git revision hash).<br/>

Pragmas
-------

**\#pragma unroll(UnrollCount)** 	enables standard llvm unrolling optimization for the loop.<br/>
**\#pragma loop_unroll(UnrollCount)** 	enables tpc backend unrolling optimization. By default, if 'unaligned_trip_count' option is not specified, then the number of iterations of the loop should be a multiple of UnrollCount. The 'unaligned_trip_count' option forces compiler to generate epilogue loop. So, 'unaligned_trip_count' option should be omitted iff there is a guarantee that iterations count is a multiple of 'UnrollCount' for faster and smaller code. To also pipeline the loop option 'pipelined' should be added. In this case the number of iteration of the loop should also be at least two times larger than UnrollCount. Option 'taken' tells compiler that the loop always executes at least 1 time. <br/>
**\#pragma loop_taken** 	tells compiler that the loop always executes at least 1 time. <br/>
**Examples:**
\#pragma unroll(2) - unroll two iterations using high-level llvm algorithm<br/>
\#pragma loop_unroll(4) - unroll 4 iterations using tpc backend algorithm<br/>
\#pragma loop_unroll(3) pipelined taken - pipeline 3 iterations, assume loop is always entered and has at least 6 iterations.<br/>
\#pragma loop_unroll(4) unaligned_trip_count - unroll 4 iterations using tpc backend algorithm. No assumption on iterations count. Extra epilogue loop is generated.<br />


Macros
------

+ **__'TPC'__'** 		       common architecture marker<br/>
+ **__goya__**               architecture generation 1<br/>
+ **__gaudi__**              architecture generation 2<br/>
+ **__gaudib__**             architecture generation 2b<br/>
+ **__greco__**              architecture generation 3<br/>
+ **__gaudi2__**             architecture generation 4<br/>
+ **MAX_TENSORS=\<n\>** 	       reflects actual tensor limit<br/>
+ **MAX_SLM=\<n\>**             reflects architecture scalar memory limit (in bytes)<br/>
+ **MAX_VLM=\<n\>**             reflects architecture vector memory limit (in bytes)<br/>

+ **__HABANA_TOOL_VERSION**   Public software version (e.g. recent was 0.10.0)
+ **__TPC_DROP_VERSION**      Internal drop version (e.g. recent was 18.0.1)
+ **VERSION2DEC(a,b,c)**      Macro to express an expected version (as "a.b.c" triple)

Typical check for compiler version is like this:
#if __TPC_DROP_VERSION >= VERSION2DEC(16, 0, 0)
    //use some recent feature
#else
    //fall back to legacy way
#endif

Intrinsics
----------
See tpc-intrinsics.h and tpc-defs.h header files (NB: these headers are documentation only, not intended for real inclusion into source code).


Extracting binary code for simulator
------------------------------------

objcopy -O binary --only-section=.text reduction.o reduction.bin 


Disassembler
------------
Typical invocation:
llvm-objdump --triple tpc -d --no-show-raw-insn --no-leading-addr --mcpu=greco mytest.o
Other TPC-specific options:
+ --tpc-compress-info	output compression map (used for VLSI testing). It is possible to re-direct this output to a file via -tpc-compress-info-file option.
+ --tpc-encoding-info	output human-readable decomposition of VLIW instruction fields. May be used with or without -d.
+ --no-common-header	suppress output of non-assembly text. Useful to stream disassembled sources back to assembler.

How to disassemble binary: wrap binary to elf<br/>  
1) Create an empy ELF container:
echo "" |tpc-clang -o empty.o -c -xassembler -

2) Embed the binary to ELF as .text section (and strip irrelevant sections away):
objcopy --update-section .text=path/to/file.bin -j .text empty.o result.o

3) Disassemble the result ELF:
llvm-objdump --triple tpc --mcpu=gaudi -d --no-show-raw-insn --no-leading-addr --no-common-header result.o > result.s

Assembler
---------
Normally compiler handles assembler source files (*.tpcasm and *.s) the same way as c/cpp sources, no special care is needed. E.g.:<br/>
tpc-clang -c -march=goya my.s<br/>
In case you need to bypass common compilation driver and invoke assembler with extra options, here is an example:<br/>
clang -cc1as -triple tpc-none-none -target-cpu goya -filetype obj mytest.s<br/>
