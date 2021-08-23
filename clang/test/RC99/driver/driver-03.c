// RUN: %tpc_clang -S -emit-llvm %s -o - -### 2>&1 | FileCheck -check-prefixes DEFAULT,COMMON %s
// RUN: %tpc_clang -S -emit-llvm -O0 %s -o - -### 2>&1 | FileCheck -check-prefixes LEVEL_O0,COMMON %s
// RUN: %tpc_clang -S -emit-llvm -O1 %s -o - -### 2>&1 | FileCheck -check-prefixes LEVEL_O1,COMMON %s
// RUN: %tpc_clang -S -emit-llvm -O2 %s -o - -### 2>&1 | FileCheck -check-prefixes LEVEL_O2,COMMON %s
// RUN: %tpc_clang -S -emit-llvm -O3 %s -o - -### 2>&1 | FileCheck -check-prefixes LEVEL_O3,COMMON %s

void main() {
}

// COMMON-NOT: -vectorize
// DEFAULT: -O2
// LEVEL_O0: -O0
// LEVEL_O1: -O1
// LEVEL_O2: -O2
// LEVEL_O3: -O2
