// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  -target-cpu gaudi2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  -target-cpu doron1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

typedef _Bool bool;

void main(minifloat x0, minifloat x1, int dest0, int dest1)
{
    bool res0 = 1; 
    bool* pd1=(bool*)dest0;
    bool* pd2=(bool*)dest1;   
    *pd1 = s_f8_cmp_eq(x0,x1, 0, res0, res0, 0);
    *pd2 = s_f8_cmp_eq(x0, (minifloat)8,0,0 ,1,0);
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: cmp_eq.f8_143 %SP{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
//CHECK-ASM-DAG: cmp_eq.f8_143 %SP{{[0-9]+}}, %S{{[0-9]+}}, 0x50
