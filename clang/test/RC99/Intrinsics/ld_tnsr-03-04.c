// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 -bfloat16 %s -o - | FileCheck %s

void main(tensor input, int dest, _Bool pred, int vpredp,
          signed char partial_size, signed char partial_offset) {
  int5 index = {0};
  int64 __local *vector_ptr = (int64 __local *)dest;
  bool128 __local *vpred_ptr128 = (bool128 __local *)vpredp;
  bool128 vpred128 = *vpred_ptr128++;

  {
    half128 __local *dest_ptr = (half128 __local *)vector_ptr;
    half128 res = 0;
    
    res = v_f16_ld_tnsr_b(index, input, 0, res, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr %V{{[0-9]+}}, 0x0, %I{{[0-9]+}}, %SP{{[0-9]+}}
  }
  {
    half128 __local *dest_ptr = (half128 __local *)vector_ptr;
    half128 res = 0;
    
    res = v_f16_ld_tnsr_vb(index, input, 0, res, vpred128, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr %V{{[0-9]+}}, 0x0, %I{{[0-9]+}}, %VP{{[0-9]+}}
  }

  {
    int64 __local *dest_ptr = (int64 __local *)vector_ptr;
    int64 res = 0;

    res = v_i32_ld_tnsr_b(index, input, SW_UNPCK_8_TO_32, res, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr UNPCK_8_TO_32 %V{{[0-9]+}}, 0x0, %I{{[0-9]+}}, %SP{{[0-9]+}}
  }
  {
    int64 __local *dest_ptr = (int64 __local *)vector_ptr;
    int64 res = 0;

    res = v_i32_ld_tnsr_b(index, input, SW_UNPACK, res, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr unpack %V{{[0-9]+}}, 0x0, %I{{[0-9]+}}, %SP{{[0-9]+}}
  }
  {
    int64 __local *dest_ptr = (int64 __local *)vector_ptr;
    int64 res = 0;

    res = v_i32_ld_tnsr_b(index, input, SW_L0CS, res, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr l0cs %V{{[0-9]+}}, 0x0, %I{{[0-9]+}}, %SP{{[0-9]+}}
  }

  {
    int64 __local *dest_ptr = (int64 __local *)vector_ptr;
    int64 res = 0;

    res = v_i32_ld_tnsr_partial_b(index, input, partial_size, partial_offset,
                                  SW_UNPCK_8_TO_32 | SW_UNPACK | SW_L0CS, res, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr partial UNPCK_8_TO_32 unpack l0cs %V{{[0-9]+}}, 0x0, %I{{[0-9]+}}, %LD_PARTIAL_REG, %SP{{[0-9]+}}
  }
}
