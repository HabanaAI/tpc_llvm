// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int x0, signed char x1, int x3, int dest0, int dest1)
{
    char256 __local *ptr_x0 = (char256 __local *)x0;
    
    int64 __local *res0 = (int64 __local *)dest0;
    int256 temp_res0 = {0,0,0,0};
    temp_res0 = av_i8_mac_v_s_b(*ptr_x0, 123, temp_res0, 1, x3, 0);
    *res0 = temp_res0.v1;
    
    int64 __local *res1 = (int64 __local *)dest1;
    int256 temp_res1 = {0,0,0,0};
    temp_res1 = av_i8_mac_v_s_b(*ptr_x0, x1, temp_res1, 1, x3, 0);
    *res1 = temp_res1.v1;
}

// CHECK-ASM:    .globl main
// CHECK-ASM:    mac.i8 st %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %SP{{[0-9]+}}
// CHECK-ASM:    mac.i8 st %A{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
