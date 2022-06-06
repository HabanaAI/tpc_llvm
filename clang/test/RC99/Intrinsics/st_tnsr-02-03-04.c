// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck %s

void main(tensor input, int dest, _Bool pred) {
  int5 index = {0};
  int64 __local *vector_ptr = (int64 __local *)dest;

  {
    bfloat128 __local *dest_ptr = (bfloat128 __local *)vector_ptr;
    bfloat128 res = 0;
    
    v_bf16_st_tnsr(index, input, res, 0, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr 0x0, %I{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
  }
}
