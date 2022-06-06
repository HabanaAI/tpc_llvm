// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu doron1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(minihalf x0, signed char x1, int dest0, int dest1)
{
    
    
    
   minihalf __local *res0 = (minihalf  __local *)dest0;
    minihalf temp_res0 = 0;
    temp_res0 = s_h8_shr(x0, x1, 0, 0, 1, 0);
    *res0 = temp_res0;
    
   minihalf __local *res1 = (minihalf  __local *)dest1;
    minihalf temp_res1 = 0;
    temp_res1 = s_h8_shr(x0, 1, 0, 0, 1, 0);
    *res1 = temp_res1;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: shr.f8_152 %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
//CHECK-ASM-DAG: shr.f8_152 %S{{[0-9]+}}, %S{{[0-9]+}}, 0x1