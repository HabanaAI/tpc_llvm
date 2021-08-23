// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi  %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(unsigned short x0, unsigned short x1, int dest0, int dest1)
{
    
    
    
   unsigned short __local *res0 = (unsigned short  __local *)dest0;
    unsigned short temp_res0 = 0;
    temp_res0 = s_u16_xor_s_s(x0, x1);
    *res0 = temp_res0;
    
   unsigned short __local *res1 = (unsigned short  __local *)dest1;
    unsigned short temp_res1 = 0;
    temp_res1 = s_u16_xor_s_s(x0, 1);
    *res1 = temp_res1;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: xor.u16 %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
//CHECK-ASM-DAG: xor.u16 %S{{[0-9]+}}, %S{{[0-9]+}}, 0x1
