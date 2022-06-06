// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(minihalf x0, int x2, int dest0)
{
    
    
    
   unsigned char __local *res0 = (unsigned char  __local *)dest0;
    unsigned char temp_res0 = 0;
    temp_res0 = s_h8_popcnt(x0, 1, temp_res0, x2, 0);
    *res0 = temp_res0;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: popcnt.f8_152 set=1 %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
