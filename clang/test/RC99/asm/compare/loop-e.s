// RUN: not %clang -cc1as -triple tpc-none-none %s -o /dev/null 2>&1 1>/dev/null | FileCheck %s
// RUN: not %clang -cc1as -triple tpc-none-none -target-cpu gaudi %s -o /dev/null 2>&1 1>/dev/null | FileCheck %s


loop S1, S2, S3, <, End1
NOP
End2:
NOP
// CHECK: [[@LINE-4]]:21: error: missing label
