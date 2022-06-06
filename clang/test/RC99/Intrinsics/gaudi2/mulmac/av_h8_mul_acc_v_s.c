// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  -target-cpu gaudi2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  -target-cpu doron1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int x0, minihalf x1, int dest0, int dest1)
{
    
    minihalf256 __local *ptr_x0 = (minihalf256 __local *)x0;
    
   float64 __local *res0 = (float64  __local *)dest0;
    float256 temp_res0 = {0.0, 0.0};
    temp_res0 = v_h8_mul_acc32_b(*ptr_x0, (minihalf)8, 0, (float256){0}, 1, 0);
    *res0 = temp_res0.v1;
   float64 __local *res1 = (float64  __local *)dest1;
    float256 temp_res1 = {0.0, 0.0};
    temp_res1 = v_h8_mul_acc32_b(*ptr_x0, x1, 0, (float256){0}, 1, 0);
    *res1 = temp_res1.v1;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: mul.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x48
//CHECK-ASM-DAG: mul.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}
