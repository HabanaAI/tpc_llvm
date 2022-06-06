// RUN: %clang -cc1 -triple tpc -emit-obj --mrelax-relocations -fembed-bitcode=bitcode -disable-free -main-file-name tpc_compiler_args.c -slm 1 -mllvm -simplifycfg-sink-common=false -mllvm -enable-load-pre=false -max-tensors 8 -tpc-special -mllvm -loop-unswitch-threshold=0 -no-instr-compress -mllvm -dontAnalysis=1 -mrelocation-model static -mframe-pointer=all -fmath-errno -fno-rounding-math -mconstructor-aliases -target-cpu goya -fno-split-dwarf-inlining -debugger-tuning=gdb -O2 -Wfloat-conversion -Wmissing-braces -Wuninitialized -std=rc99 -fdebug-compilation-dir %T -ferror-limit 19 -fgnuc-version=4.2.1 -fcolor-diagnostics -faddrsig -o %t.o -x c %s
// RUN: %llvm-objdump --triple tpc -s -j .tpc_compiler %t.o | FileCheck %s

void main() {
	return;
}

// CHECK: Contents of section .tpc_compiler:
// CHECK-NEXT:  0000 222d6363 3122{{[a-f0-9]+}} {{[a-f0-9]+}} {{[a-f0-9]+}}  "-cc1"


