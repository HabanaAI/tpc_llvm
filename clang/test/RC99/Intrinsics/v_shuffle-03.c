// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2  -bfloat16 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 -bfloat16 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 -bfloat16 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int x0, int x1, int dest0) {
  {
    half128 __local *ptr_x0 = (half128 __local *)x0;
    uchar256 __local *ptr_x1 = (uchar256 __local *)x1;

    half128 __local *res0 = (half128 __local *)dest0;
    half128 temp_res0 = 0;
    temp_res0 = v_f16_shuffle_b(*ptr_x0, *ptr_x1, 0, *ptr_x0, 1, 0);
    *res0 = temp_res0;
    //CHECK-ASM-DAG: shuffle.f16 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
  }
}
