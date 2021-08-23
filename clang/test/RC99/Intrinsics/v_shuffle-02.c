// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi  -bfloat16 %s -o - | FileCheck --check-prefix=CHECK-ASM %s



void main(int x0, int x1, int dest0) {
  {
    bfloat128 __local *ptr_x0 = (bfloat128 __local *)x0;
    uchar256 __local *ptr_x1 = (uchar256 __local *)x1;

    bfloat128 __local *res0 = (bfloat128 __local *)dest0;
    bfloat128 temp_res0 = 0;
    temp_res0 = v_bf16_shuffle_v_v(*ptr_x0, *ptr_x1);
    *res0 = temp_res0;
    //CHECK-ASM-DAG: shuffle.bf16 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
  }
}
