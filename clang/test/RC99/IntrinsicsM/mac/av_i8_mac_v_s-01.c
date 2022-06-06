// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int x0, int dest0, int dest1)
{
    char256 __local *ptr_x0 = (char256 __local *)x0;
    int64 __local *res0 = (int64 __local *)dest0;
    int256 temp_res0 = {0,0,0,0};
    temp_res0 = v_i8_mac_b(*ptr_x0, 123, temp_res0, 1, 1, 0);
    *res0 = temp_res0.v1;
}

// CHECK: call <256 x i32> @llvm.tpc.av.i8.mac.v.s(<256 x i8> %2, i8 123, <256 x i32> zeroinitializer, i8 1)

// CHECK-ASM:    .globl main
// CHECK-ASM:     main:
// CHECK-ASM-DAG: ld_l_v    %V{{[0-9]+}}, %S0, 0x0
// CHECK-ASM-DAG: mov.i32   %V{{[0-9]+}}, 0x0
// CHECK-ASM:     mac.i8    st %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b
// CHECK-ASM-DAG: st_l_v    %S1, 0x0, %V{{[0-9]+}}
