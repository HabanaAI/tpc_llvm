





// RUN: %tpc_clang -S -march=gaudi -bfloat16 -emit-llvm -O2 %s -o - -### 2>&1 | FileCheck -check-prefix LEVEL_O2_GAUDI %s



void main() {
}

// DEFAULT: -instr-compress
// LEVEL_O0: -no-instr-compress
// LEVEL_O1: -no-instr-compress
// LEVEL_O2: -instr-compress
// LEVEL_O2_GAUDI: -no-instr-compress
// LEVEL_O1_COMPRESS: -instr-compress
// LEVEL_O2_NO_COMPRESS: -no-instr-compress
