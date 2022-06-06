// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu doron1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(minifloat x0, signed char x1, int dest0, int dest1)
{
    
    
    
   minifloat __local *res0 = (minifloat  __local *)dest0;
    minifloat temp_res0 = 0;
    temp_res0 = s_f8_shl(x0, x1, 0, 0, 1, 0);
    *res0 = temp_res0;
    
   minifloat __local *res1 = (minifloat  __local *)dest1;
    minifloat temp_res1 = 0;
    temp_res1 = s_f8_shl(x0, 1, 0, 0, 1, 0);
    *res1 = temp_res1;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: shl.f8_143 %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
//CHECK-ASM-DAG: shl.f8_143 %S{{[0-9]+}}, %S{{[0-9]+}}, 0x1
