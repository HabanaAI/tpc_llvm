// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  -target-cpu gaudi2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  -target-cpu doron1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int x0, int x1, int dest0)
{
    unsigned a = 1;
    minifloat256 __local *ptr_x0 = (minifloat256 __local *)x0;
    minifloat256 __local *ptr_x1 = (minifloat256 __local *)x1;
    bool256 pred2;
    pred2 =  v_i1_mov_i1_b(1, 0, (bool256){0}, 1, 0);
    
   minifloat256 __local *res0 = (minifloat256  __local *)dest0;
    minifloat256 temp_res0 = 0;
    temp_res0 = v_f8_max_vb(*ptr_x0, *ptr_x1, 0,temp_res0, pred2, 0);
    *res0 = temp_res0;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: max.f8_143 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
