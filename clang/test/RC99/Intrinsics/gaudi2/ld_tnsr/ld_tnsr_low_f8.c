// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 -bfloat16 %s -o - | FileCheck %s

void main(tensor input, int dest, _Bool pred, int vpredp) {
  int5 index = {0};
  int64 __local *vector_ptr = (int64 __local *)dest;
  bool256 __local *vpred_ptr256 = (bool256 __local *)vpredp;
  bool256 vpred256 = *vpred_ptr256++;
  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)vector_ptr;
    minifloat256 res = 0;
    
    res = v_f8_ld_tnsr_low_b(index, input, 0, res, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr_low %V{{[0-9]+}}, 0x0, %I{{[0-9]+}}, %SP{{[0-9]+}}
  }
  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)vector_ptr;
    minifloat256 res = 0;
    
    res = v_f8_ld_tnsr_low_vb(index, input + 1, 0, res, vpred256, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr_low %V{{[0-9]+}}, 0x1, %I{{[0-9]+}}, %VP{{[0-9]+}}
  }
}