// RUN: not %clang -cc1as -triple tpc-none-none -target-cpu dali -filetype obj %s -o /dev/null 2>&1 | FileCheck %s

NOP; NOP; NOP; ASO DEC
// CHECK: switch is not supported by the current processor
