// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int x0, unsigned x1, int dest0, int dest1)
{
    unsigned a = 1;
    uint64 __local *ptr_x0 = (uint64 __local *)x0;
    bool256 pred2;
    pred2 =  v_i1_mov_i1_b(1, 0, (bool256){0}, 1, 0);
    
   uint64 __local *res0 = (uint64  __local *)dest0;
    uint64 temp_res0 = 0;
    temp_res0 = v_u32_or_vb(*ptr_x0, 1, 0, temp_res0, to_bool64(pred2), 0);
    *res0 = temp_res0;
    
   uint64 __local *res1 = (uint64  __local *)dest1;
    uint64 temp_res1 = 0;
    temp_res1 = v_u32_or_vb(*ptr_x0, x1, 0, temp_res1, to_bool64(pred2), 0);
    *res1 = temp_res1;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: or.u32 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x1, %VP{{[0-9]+}}
//CHECK-ASM-DAG: or.u32 %V{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, %VP{{[0-9]+}}
