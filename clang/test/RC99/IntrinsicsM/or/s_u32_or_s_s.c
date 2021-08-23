// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi  %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(unsigned x0, unsigned x1, int dest0, int dest1)
{
    
    
    
   unsigned __local *res0 = (unsigned  __local *)dest0;
    unsigned temp_res0 = 0;
    temp_res0 = s_u32_or_s_s(x0, x1);
    *res0 = temp_res0;
    
   unsigned __local *res1 = (unsigned  __local *)dest1;
    unsigned temp_res1 = 0;
    temp_res1 = s_u32_or_s_s(x0, 1);
    *res1 = temp_res1;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: or.u32 %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
//CHECK-ASM-DAG: or.u32 %S{{[0-9]+}}, %S{{[0-9]+}}, 0x1
