// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -emit-llvm -disable-llvm-passes -bfloat16 -target-cpu gaudi %s -o - | FileCheck %s

_BFloat16 BF16_zero_01 = 0;
// CHECK: @BF16_zero_01 = {{.*}} bfloat 0xR0000

_BFloat16 BF16_zero_02 = 0.0;
// CHECK: @BF16_zero_02 = {{.*}} bfloat 0xR0000

_BFloat16 BF16_zero_03 = 0.0bf;
// CHECK: @BF16_zero_03 = {{.*}} bfloat 0xR0000



_BFloat16 BF16_one_01 = 1;
// CHECK: @BF16_one_01 = {{.*}} bfloat 0xR3F80

_BFloat16 BF16_one_02 = 1.0;
// CHECK: @BF16_one_02 = {{.*}} bfloat 0xR3F80

_BFloat16 BF16_one_03 = 1.0bf;
// CHECK: @BF16_one_03 = {{.*}} bfloat 0xR3F80



_BFloat16 BF16_two = 2.bf;
// CHECK: @BF16_two = {{.*}} bfloat 0xR4000

_BFloat16 BF16_minus_one = -1.bf;
// CHECK: @BF16_minus_one = {{.*}} bfloat 0xRBF80

_BFloat16 BF16_one_and_half = 1.5bf;
// CHECK: @BF16_one_and_half = {{.*}} bfloat 0xR3FC0

_BFloat16 BF16_1_75 = 1.75bf;
// CHECK: @BF16_1_75 = {{.*}} bfloat 0xR3FE0

_BFloat16 BF16_minus_1_75 = -1.75bf;
// CHECK: @BF16_minus_1_75 = {{.*}} bfloat 0xRBFE0


_BFloat16 BF16_inf = 1.bf / 0.bf;
// CHECK: @BF16_inf = {{.*}} bfloat 0xR7F80

_BFloat16 BF16_minus_inf = -1.bf / 0.bf;
// CHECK: @BF16_minus_inf = {{.*}} bfloat 0xRFF80

_BFloat16 BF16_qnan = 0.bf / 0.bf;
// CHECK: @BF16_qnan = {{.*}} bfloat 0xR7FC0


void main() {
}

