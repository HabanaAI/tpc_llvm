// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int x0, bf16 x1, int x2, int dest0, int dest1)
{
    
    bfloat128 __local *ptr_x0 = (bfloat128 __local *)x0;
    
   bfloat128 __local *res0 = (bfloat128  __local *)dest0;
    bfloat128 temp_res0 = 0;
    temp_res0 = v_bf16_xor_v_s_b(*ptr_x0, 8.bf, temp_res0, x2, 0);
    *res0 = temp_res0;
    
   bfloat128 __local *res1 = (bfloat128  __local *)dest1;
    bfloat128 temp_res1 = 0;
    temp_res1 = v_bf16_xor_v_s_b(*ptr_x0, x1, temp_res1, x2, 0);
    *res1 = temp_res1;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: xor.bf16 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x4100, %SP{{[0-9]+}}
//CHECK-ASM-DAG: xor.bf16 %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
