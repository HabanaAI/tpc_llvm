// RUN: %tpc_clang -c -march=gaudi %s -o - -### 2>&1 | FileCheck -check-prefix GAUDI %s
// RUN: %tpc_clang -c -march=dali %s -o - -### 2>&1 | FileCheck -check-prefix DALI %s
// RUN: %tpc_clang -c %s -o - -### 2>&1 | FileCheck -check-prefix DEFAULT %s

void main() {
}

// GAUDI: -target-cpu
// GAUDI-SAME: gaudi
// DALI: -target-cpu
// DALI-SAME: goya
// DEFAULT: -target-cpu
// DEFAULT-SAME: goya
