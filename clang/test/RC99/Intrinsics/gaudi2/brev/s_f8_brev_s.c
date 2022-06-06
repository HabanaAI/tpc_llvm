// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu doron1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(minifloat x0, int dest0)
{
   minifloat __local *res0 = (minifloat  __local *)dest0;
    minifloat temp_res0 = 0;
    temp_res0 = s_f8_brev(x0, 0, 0, 1, 0);
    *res0 = temp_res0;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: brev.f8_143 %S{{[0-9]+}}, %S{{[0-9]+}}
