// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 -bfloat16 %s -o - | FileCheck %s

void main(tensor input, int dest, _Bool pred, int vpredp,
          signed char partial_size, signed char partial_offset) {
  int5 index = {0};
  int64 __local *vector_ptr = (int64 __local *)dest;
  bool256 __local *vpred_ptr256 = (bool256 __local *)vpredp;
  bool64 __local *vpred_ptr64 = (bool64 __local *)vpredp;
  bool256 vpred256 = *vpred_ptr256++;
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)vector_ptr;
    minihalf256 res = 0;
    
    res = v_h8_ld_tnsr_partial_b(index, input, 16,6,0, res, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr partial %V{{[0-9]+}}, 0x0, %I{{[0-9]+}}, %LD_PARTIAL_REG, %SP{{[0-9]+}}
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)vector_ptr;
    minihalf256 res = 0;
    
    res = v_h8_ld_tnsr_partial_vb(index, input + 1,16,6, 0, 0, vpred256, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr partial %V{{[0-9]+}}, 0x1, %I{{[0-9]+}}, %LD_PARTIAL_REG, %VP{{[0-9]+}}
  }

}