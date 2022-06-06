// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int x0, int x1, int dest0)
{
    unsigned a = 1;
    short128 __local *ptr_x0 = (short128 __local *)x0;
    short128 __local *ptr_x1 = (short128 __local *)x1;
    bool256 pred2;
    pred2 =  v_i1_mov_i1_b(1, 0, (bool256){0}, 1, 0);
    
   short128 __local *res0 = (short128  __local *)dest0;
    short128 temp_res0 = 0;
    temp_res0 = v_i16_and_vb(*ptr_x0, *ptr_x1, 0, temp_res0, to_bool128(pred2), 0);
    *res0 = temp_res0;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: and.i16 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
