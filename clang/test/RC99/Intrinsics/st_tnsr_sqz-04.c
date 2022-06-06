// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck %s

void main(tensor input, int other_input, int dest, int vpredp) {
  int5 index = {0};
  int64 __local *vector_ptr = (int64 __local *)dest;

  bool256 __local *vpred_ptr = (bool256 __local *)vpredp;
  bool256 vpred = *vpred_ptr;
  
  reset_sqz_cntr(0, 0);
  // CHECK-DAG: mov.to_hw_reg 0x10, %S{{[0-9]+}}
  // CHECK-DAG: mov.to_hw_reg 0x11, %S{{[0-9]+}}

  {
    float64 __local *dest_ptr = (float64 __local *)vector_ptr;
    float64 res = {0};

    v_f32_st_tnsr_sqz(index, input, 0, res, 0, vpred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr_sqz 0x0, 0x0, %I{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }
  {
    int64 __local *dest_ptr = (int64 __local *)vector_ptr;
    int64 res = {0};

    v_i32_st_tnsr_sqz(index, input, 0, res, 0, vpred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr_sqz 0x0, 0x0, %I{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }
  {
    uint64 __local *dest_ptr = (uint64 __local *)vector_ptr;
    uint64 res = {0};

    v_u32_st_tnsr_sqz(index, input, 0, res, 0, vpred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr_sqz 0x0, 0x0, %I{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }

  {
    float64 __local *dest_ptr = (float64 __local *)vector_ptr;
    float64 res = {0};

    v_f32_st_tnsr_sqz(index, other_input, 0,  res, 0, vpred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr_sqz %ST_TNSR_ID_REG, 0x0, %I{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }
  {
    int64 __local *dest_ptr = (int64 __local *)vector_ptr;
    int64 res = {0};

    v_i32_st_tnsr_sqz(index, other_input, 0, res, 0, vpred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr_sqz %ST_TNSR_ID_REG, 0x0, %I{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }
  {
    uint64 __local *dest_ptr = (uint64 __local *)vector_ptr;
    uint64 res = {0};

    v_u32_st_tnsr_sqz(index, other_input, 0, res, 0, vpred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr_sqz %ST_TNSR_ID_REG, 0x0, %I{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }
}
