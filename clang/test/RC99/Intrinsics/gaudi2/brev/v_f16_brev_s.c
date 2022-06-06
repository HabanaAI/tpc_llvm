// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu doron1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(half x0, int dest0)
{
   ushort128 __local *res0 = (ushort128  __local *)dest0;
    ushort128 temp_res0 = 0;
    temp_res0 = v_f16_brev_b(x0, 0, 0, 1, 0);
    *res0 = temp_res0;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: brev.f16 %V{{[0-9]+}}, %S{{[0-9]+}}
