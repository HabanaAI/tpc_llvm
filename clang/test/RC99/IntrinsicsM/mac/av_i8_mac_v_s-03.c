// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int x0, signed char x1, int dest0)
{
    char256 __local *ptr_x0 = (char256 __local *)x0;
    int256 __local *res0 = (int256 __local *)dest0;
    int256 temp_res0 = {0,0,0,0};
    temp_res0 = av_i8_mac_v_s(*ptr_x0, x1, temp_res0, 1);
    *res0 = temp_res0;
}

// CHECK-ASM:    .globl main
// CHECK-ASM:     main:
// CHECK-ASM-DAG: ld_l_v    %V{{[0-9]+}}, %S0, 0x0
// CHECK-ASM-DAG: mov.i32   %V{{[0-9]+}}, 0x0
// CHECK-ASM:     mac.i8 st %A{{[0-9]+}}, %V{{[0-9]+}}, %S1, %SP0
// CHECK-ASM-DAG: st_l_v %S2, 0x300, %V{{[0-9]+}}
// CHECK-ASM-DAG: st_l_v %S2, 0x200, %V{{[0-9]+}}
// CHECK-ASM-DAG: st_l_v %S2, 0x100, %V{{[0-9]+}}
// CHECK-ASM-DAG: st_l_v %S2, 0x0, %V{{[0-9]+}}
