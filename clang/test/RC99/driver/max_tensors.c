// RUN: %tpc_clang -S -emit-llvm %s -o - -### 2>&1 | FileCheck -check-prefix DEFAULT %s
// RUN: %tpc_clang -S -emit-llvm -march=dali %s -o - -### 2>&1 | FileCheck -check-prefix DALI %s
// RUN: %tpc_clang -S -emit-llvm -march=gaudi %s -o - -### 2>&1 | FileCheck -check-prefix GAUDI %s

void main(int x) {
}

// DEFAULT: -max-tensors{{.+}}8
// DALI: -max-tensors{{.+}}8
// GAUDI: -max-tensors{{.+}}16
