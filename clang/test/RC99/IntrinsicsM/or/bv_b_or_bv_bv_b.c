// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int x2, int dest0)
{
    unsigned a = 1;
    bool256 pred0;
    pred0 =  bv_mov_b(1);
    bool256 pred1;
    pred1 =  bv_mov_b(1);
    bool256 res0 = 0; 
   int64 __local *pred_res0 = (int64  __local *)dest0;

    res0 = bv_b_or_bv_bv_b(pred0, pred1, res0, x2, 0);
    *pred_res0 = v_i32_add_v_s_vb(*pred_res0 , 1, *pred_res0 , 1, res0, 0);
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: or.b %VP{{[0-9]+}}, %VP{{[0-9]+}}, %VP{{[0-9]+}}, %SP{{[0-9]+}}
