// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int x2, int dest0)
{
    unsigned a = 1;
    bool256 pred0;
    pred0 =  v_i1_mov_i1_b(1, 0, (bool256){0}, 1, 0);
    bool256 pred1;
    pred1 =  v_i1_mov_i1_b(1, 0, (bool256){0}, 1, 0);
    bool256 res0 = 0; 
   int64 __local *pred_res0 = (int64  __local *)dest0;

    res0 = v_i1_and_b(pred0, pred1, 0, res0, x2, 0);
    *pred_res0 = v_i32_add_vb(*pred_res0 , 1, 1, *pred_res0 , to_bool64(res0), 0);
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: and.b %VP{{[0-9]+}}, %VP{{[0-9]+}}, %VP{{[0-9]+}}, %SP{{[0-9]+}}
