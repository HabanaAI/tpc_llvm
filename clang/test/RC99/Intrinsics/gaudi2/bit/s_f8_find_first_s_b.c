// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu doron1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(minifloat x0, int x3, int dest0)
{
    
    
    
   unsigned char __local *res0 = (unsigned char  __local *)dest0;
    unsigned char temp_res0 = 0;
    temp_res0 = s_f8_find_first(x0, ((1) | ((1) << 1)), temp_res0, x3, 0);
    *res0 = temp_res0;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: find_first.f8_143 set=1 start_msb %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
