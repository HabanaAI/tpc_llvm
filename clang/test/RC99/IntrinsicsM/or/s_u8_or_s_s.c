// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s  -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi  %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(unsigned char x0, unsigned char x1, int dest0, int dest1)
{
    
    
    
   unsigned char __local *res0 = (unsigned char  __local *)dest0;
    unsigned char temp_res0 = 0;
    temp_res0 = s_u8_or(x0, x1, 0, 0, 1, 0);
    *res0 = temp_res0;
    
   unsigned char __local *res1 = (unsigned char  __local *)dest1;
    unsigned char temp_res1 = 0;
    temp_res1 = s_u8_or(x0, 1, 0, 0, 1, 0);
    *res1 = temp_res1;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: or.u8 %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
//CHECK-ASM-DAG: or.u8 %S{{[0-9]+}}, %S{{[0-9]+}}, 0x1
