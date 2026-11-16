// Instruction compression follows Habana's driver: on by default and at -O2,
// off at every other -O level. gaudi and gaudib compress only when no -O is
// given.

// RUN: %tpc_clang -S -march=goya2 -emit-llvm     %s -o - -### 2>&1 | FileCheck -check-prefix COMPRESS %s
// RUN: %tpc_clang -S -march=goya2 -emit-llvm -O0 %s -o - -### 2>&1 | FileCheck -check-prefix NO_COMPRESS %s
// RUN: %tpc_clang -S -march=goya2 -emit-llvm -O1 %s -o - -### 2>&1 | FileCheck -check-prefix NO_COMPRESS %s
// RUN: %tpc_clang -S -march=goya2 -emit-llvm -O  %s -o - -### 2>&1 | FileCheck -check-prefix NO_COMPRESS %s
// RUN: %tpc_clang -S -march=goya2 -emit-llvm -O2 %s -o - -### 2>&1 | FileCheck -check-prefix COMPRESS %s
// RUN: %tpc_clang -S -march=goya2 -emit-llvm -O3 %s -o - -### 2>&1 | FileCheck -check-prefix NO_COMPRESS %s
// RUN: %tpc_clang -S -march=goya2 -emit-llvm -O4 %s -o - -### 2>&1 | FileCheck -check-prefix NO_COMPRESS %s
// RUN: %tpc_clang -S -march=goya2 -emit-llvm -Os %s -o - -### 2>&1 | FileCheck -check-prefix NO_COMPRESS %s
// RUN: %tpc_clang -S -march=goya2 -emit-llvm -Oz %s -o - -### 2>&1 | FileCheck -check-prefix NO_COMPRESS %s
// RUN: %tpc_clang -S -march=goya2 -emit-llvm -Og %s -o - -### 2>&1 | FileCheck -check-prefix NO_COMPRESS %s
// RUN: %tpc_clang -S -march=goya2 -emit-llvm -Ofast %s -o - -### 2>&1 | FileCheck -check-prefix NO_COMPRESS %s
// RUN: %tpc_clang -S -march=goya2 -emit-llvm -O3 -O2 %s -o - -### 2>&1 | FileCheck -check-prefix COMPRESS %s
// RUN: %tpc_clang -S -march=goya2 -emit-llvm -O2 -O3 %s -o - -### 2>&1 | FileCheck -check-prefix NO_COMPRESS %s
// RUN: %tpc_clang -S -march=gaudi2 -emit-llvm -O2 %s -o - -### 2>&1 | FileCheck -check-prefix COMPRESS %s
// RUN: %tpc_clang -S -march=gaudi2 -emit-llvm -O3 %s -o - -### 2>&1 | FileCheck -check-prefix NO_COMPRESS %s
// RUN: %tpc_clang -S -march=gaudi -emit-llvm     %s -o - -### 2>&1 | FileCheck -check-prefix COMPRESS %s
// RUN: %tpc_clang -S -march=gaudi -emit-llvm -O2 %s -o - -### 2>&1 | FileCheck -check-prefix NO_COMPRESS %s
// RUN: %tpc_clang -S -march=goya2 -emit-llvm -O1 %s -o - -instr-compress -### 2>&1 | FileCheck -check-prefix COMPRESS %s
// RUN: %tpc_clang -S -march=goya2 -emit-llvm -O3 %s -o - -instr-compress -### 2>&1 | FileCheck -check-prefix COMPRESS %s
// RUN: %tpc_clang -S -march=goya2 -emit-llvm -O2 %s -o - -no-instr-compress -### 2>&1 | FileCheck -check-prefix NO_COMPRESS %s

void main() {
}

// COMPRESS: "-cc1"
// COMPRESS-SAME: "-instr-compress"
// COMPRESS-NOT: "-no-instr-compress"
// NO_COMPRESS: "-cc1"
// NO_COMPRESS-SAME: "-no-instr-compress"
// NO_COMPRESS-NOT: "-instr-compress"
