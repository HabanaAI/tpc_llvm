// RUN: not %tpc_clang -march=dali -c -o - %s 2>&1 | FileCheck %s

_BFloat16 BF16_zero_01 = 0;
// CHECK: error: type _Bfloat16 is not supported

void main() {
}

