// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s  -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s  -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(unsigned short x0, unsigned short x1, int dest0, int dest1)
{
    
    
    
   unsigned short __local *res0 = (unsigned short  __local *)dest0;
    unsigned short temp_res0 = 0;
    temp_res0 = s_u16_and(x0, x1, 0, 0, 1, 0);
    *res0 = temp_res0;
    
   unsigned short __local *res1 = (unsigned short  __local *)dest1;
    unsigned short temp_res1 = 0;
    temp_res1 = s_u16_and(x0, 1, 0, 0, 1, 0);
    *res1 = temp_res1;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: and.u16 %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
//CHECK-ASM-DAG: and.u16 %S{{[0-9]+}}, %S{{[0-9]+}}, 0x1
