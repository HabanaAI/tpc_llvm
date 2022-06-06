// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu doron1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(bf16 x0, int dest0)
{
    unsigned a = 1;
    bool256 pred1;
    pred1 =  v_i1_mov_i1_b(1, 0, (bool256){0}, 1, 0);
    
   minifloat256 __local *res0 = (minifloat256  __local *)dest0;
    minifloat256 temp_res0 = 0;
    temp_res0 = v_f8_fclass_vb(x0, 0, temp_res0, (pred1), 0);
    *res0 = temp_res0;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: fclass.f8_143 %V{{[0-9]+}}, %S{{[0-9]+}}, %VP{{[0-9]+}}
