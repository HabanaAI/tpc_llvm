// RUN: not %clang -cc1as -triple tpc-none-none %s -o /dev/null 2>&1 1>/dev/null | FileCheck %s
// RUN: not %clang -cc1as -triple tpc-none-none -target-cpu gaudi %s -o /dev/null 2>&1 1>/dev/null | FileCheck %s
// RUN: not %clang -cc1as -triple tpc-none-none -target-cpu goya2 %s -o /dev/null 2>&1 1>/dev/null | FileCheck %s

nop;jmpr END_LABEL, SP1; nop; nop
END_LABEL1:
// CHECK: [[@LINE-2]]:10: error: missing label
