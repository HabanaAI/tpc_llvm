// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int x0, int x1, int x2, int dest0)
{
    
    bfloat128 __local *ptr_x0 = (bfloat128 __local *)x0;
    bfloat128 __local *ptr_x1 = (bfloat128 __local *)x1;
    
   bfloat128 __local *res0 = (bfloat128  __local *)dest0;
    bfloat128 temp_res0 = 0;
    temp_res0 = v_bf16_and_b(*ptr_x0, *ptr_x1, 0, temp_res0, x2, 0);
    *res0 = temp_res0;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: and.bf16 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
