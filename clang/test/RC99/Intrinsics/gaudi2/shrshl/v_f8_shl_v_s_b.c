// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu doron1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int x0, signed char x1, int x2, int dest0, int dest1)
{
    
    minifloat256 __local *ptr_x0 = (minifloat256 __local *)x0;
    
   minifloat256 __local *res0 = (minifloat256  __local *)dest0;
    minifloat256 temp_res0 = 0;
    temp_res0 = v_f8_shl_b(*ptr_x0, 1, 0, temp_res0, x2, 0);
    *res0 = temp_res0;
    
   minifloat256 __local *res1 = (minifloat256  __local *)dest1;
    minifloat256 temp_res1 = 0;
    temp_res1 = v_f8_shl_b(*ptr_x0, x1, 0, temp_res1, x2, 0);
    *res1 = temp_res1;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: shl.f8_143 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x1, %SP{{[0-9]+}}
//CHECK-ASM-DAG: shl.f8_143 %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
