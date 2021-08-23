// RUN: %tpc_clang -c -main-function main_relaxed %s -### 2>&1 | FileCheck -check-prefix=CHECK-DRV %s

// CHECK-DRV: clang{{.*}}-cc1{{.*}}-main-function{{.*}}main_relaxed

void main_relaxed(int I) {
}
