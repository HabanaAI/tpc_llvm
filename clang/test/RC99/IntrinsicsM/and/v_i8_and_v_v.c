// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int x0, int x1, int dest0)
{
    
    char256 __local *ptr_x0 = (char256 __local *)x0;
    char256 __local *ptr_x1 = (char256 __local *)x1;
    
   char256 __local *res0 = (char256  __local *)dest0;
    char256 temp_res0 = 0;
    temp_res0 = v_i8_and_v_v(*ptr_x0, *ptr_x1);
    *res0 = temp_res0;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: and.i8 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
