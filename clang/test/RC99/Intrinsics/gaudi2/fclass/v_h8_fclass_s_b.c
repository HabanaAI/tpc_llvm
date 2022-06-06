// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu doron1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(bf16 x0, int x1, int dest0)
{
    
    
    
   minihalf256 __local *res0 = (minihalf256  __local *)dest0;
    minihalf256 temp_res0 = 0;
    temp_res0 = v_h8_fclass_b(x0, 0, temp_res0, x1, 0);
    *res0 = temp_res0;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: fclass.f8_152 %V{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
