// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int x0, int x1, int dest0)
{
    unsigned a = 1;
    bfloat128 __local *ptr_x0 = (bfloat128 __local *)x0;
    bfloat128 __local *ptr_x1 = (bfloat128 __local *)x1;
    bool256 pred2;
    pred2 =  bv_mov_b(1);
    
   bfloat128 __local *res0 = (bfloat128  __local *)dest0;
    bfloat128 temp_res0 = 0;
    temp_res0 = v_bf16_xor_v_v_vb(*ptr_x0, *ptr_x1, temp_res0, pred2, 0);
    *res0 = temp_res0;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: xor.bf16 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
