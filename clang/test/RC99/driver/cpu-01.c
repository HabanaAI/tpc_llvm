// RUN: not %clang -target tpc -std=rc99 -c -march=qqq %s -o - 2>&1 | FileCheck %s
// RUN: not %clang -target tpc -std=rc99 -c -mcpu=qqq %s -o - 2>&1 | FileCheck %s
// RUN: not %clang -target tpc -std=rc99 -c -mtune=qqq %s -o - 2>&1 | FileCheck %s
// RUN: not %clang_cc1 -triple tpc -std=rc99 -S -target-cpu qqq %s -o - 2>&1 | FileCheck %s

void main() {
}

// CHECK: error: unknown target CPU 'qqq'
// CHECK: note: valid target CPU values are: goya, gaudi, gaudib, greco, gaudi2