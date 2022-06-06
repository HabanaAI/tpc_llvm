// RUN: %tpc_clang -S -march=goya2 -emit-llvm     %s -o - -### 2>&1 | FileCheck -check-prefix DEFAULT  %s
// RUN: %tpc_clang -S -march=goya2 -emit-llvm -O0 %s -o - -### 2>&1 | FileCheck -check-prefix LEVEL_O0 %s
// RUN: %tpc_clang -S -march=goya2 -emit-llvm -O1 %s -o - -### 2>&1 | FileCheck -check-prefix LEVEL_O1 %s
// RUN: %tpc_clang -S -march=goya2 -emit-llvm -O2 %s -o - -### 2>&1 | FileCheck -check-prefix LEVEL_O2 %s
// RUN: %tpc_clang -S -march=goya2 -emit-llvm -O3 %s -o - -### 2>&1 | FileCheck -check-prefix LEVEL_O2 %s
// RUN: %tpc_clang -S -march=goya2 -emit-llvm -O4 %s -o - -### 2>&1 | FileCheck -check-prefix LEVEL_O2 %s
// RUN: %tpc_clang -S -march=gaudi -emit-llvm -O2 %s -o - -### 2>&1 | FileCheck -check-prefix LEVEL_O2_GAUDI %s
// RUN: %tpc_clang -S -march=goya2 -emit-llvm -O1 %s -o - -instr-compress -### 2>&1 | FileCheck -check-prefix LEVEL_O1_COMPRESS %s
// RUN: %tpc_clang -S -march=goya2 -emit-llvm -O2 %s -o - -no-instr-compress -### 2>&1 | FileCheck -check-prefix LEVEL_O2_NO_COMPRESS %s

void main() {
}

// DEFAULT: -instr-compress
// LEVEL_O0: -no-instr-compress
// LEVEL_O1: -no-instr-compress
// LEVEL_O2: -instr-compress
// LEVEL_O2_GAUDI: -no-instr-compress
// LEVEL_O1_COMPRESS: -instr-compress
// LEVEL_O2_NO_COMPRESS: -no-instr-compress
