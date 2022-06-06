// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 -bfloat16 %s -o - | FileCheck %s

void main(tensor input, int dest, _Bool pred, int vpredp,
          signed char partial_size, signed char partial_offset) {
  int5 index = {0};
  int64 __local *vector_ptr = (int64 __local *)dest;
  bool256 __local *vpred_ptr256 = (bool256 __local *)vpredp;
  bool128 __local *vpred_ptr128 = (bool128 __local *)vpredp;
  bool64 __local *vpred_ptr64 = (bool64 __local *)vpredp;
  bool256 vpred256 = *vpred_ptr256++;
  bool128 vpred128 = *vpred_ptr128++;
  bool64 vpred64 = *vpred_ptr64++;

  {
    float64 __local *dest_ptr = (float64 __local *)vector_ptr;
    float64 res = 0;
    
    res = v_f32_ld_tnsr_vb(index, input, 0, res, vpred64, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr %V{{[0-9]+}}, 0x0, %I{{[0-9]+}}, %VP{{[0-9]+}}
  }
  {
    bfloat128 __local *dest_ptr = (bfloat128 __local *)vector_ptr;
    bfloat128 res = 0;
    
    res = v_bf16_ld_tnsr_b(index, input, 0, res, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr %V{{[0-9]+}}, 0x0, %I{{[0-9]+}}, %SP{{[0-9]+}}
  }
  {
    bfloat128 __local *dest_ptr = (bfloat128 __local *)vector_ptr;
    bfloat128 res = 0;
    
    res = v_bf16_ld_tnsr_vb(index, input + 1, 0, 0, vpred128, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr %V{{[0-9]+}}, 0x1, %I{{[0-9]+}}, %VP{{[0-9]+}}
  }
  {
    int64 __local *dest_ptr = (int64 __local *)vector_ptr;
    int64 res = 0;

    res = v_i32_ld_tnsr_vb(index, input, 0, res, vpred64, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr %V{{[0-9]+}}, 0x0, %I{{[0-9]+}}, %VP{{[0-9]+}}
  }
  {
    uint64 __local *dest_ptr = (uint64 __local *)vector_ptr;
    uint64 res = 0;

    res = v_u32_ld_tnsr_vb(index, input + 1, 0, res, vpred64, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr %V{{[0-9]+}}, 0x1, %I{{[0-9]+}}, %VP{{[0-9]+}}
  }
  {
    short128 __local *dest_ptr = (short128 __local *)vector_ptr;
    short128 res = 0;

    res = v_i16_ld_tnsr_vb(index, input, 0, res, vpred128, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr %V{{[0-9]+}}, 0x0, %I{{[0-9]+}}, %VP{{[0-9]+}}
  }
  {
    ushort128 __local *dest_ptr = (ushort128 __local *)vector_ptr;
    ushort128 res = 0;

    res = v_u16_ld_tnsr_vb(index, input + 1, 0, res, vpred128, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr %V{{[0-9]+}}, 0x1, %I{{[0-9]+}}, %VP{{[0-9]+}}
  }
  {
    char256 __local *dest_ptr = (char256 __local *)vector_ptr;
    char256 res = 0;

    res = v_i8_ld_tnsr_vb(index, input, 0, res, vpred256, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr %V{{[0-9]+}}, 0x0, %I{{[0-9]+}}, %VP{{[0-9]+}}
  }
  {
    uchar256 __local *dest_ptr  = (uchar256 __local *)vector_ptr;
    uchar256 res  = 0;

    res = v_u8_ld_tnsr_vb(index, input + 1, 0, res, vpred256, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr %V{{[0-9]+}}, 0x1, %I{{[0-9]+}}, %VP{{[0-9]+}}
  }
  {
    bool256 __local *dest_ptr = (bool256 __local *)vector_ptr;
    bool256 res = 0;

    res = v_i1_ld_tnsr_vb(index, input, 0, res, vpred256, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr %VP{{[0-9]+}}, 0x0, %I{{[0-9]+}}, %VP{{[0-9]+}}
  }

  {
    float64 __local *dest_ptr = (float64 __local *)vector_ptr;
    float64 res = 0;
    
    res = v_f32_ld_tnsr_partial_b(index, input, partial_size, partial_offset,
                                  0, res, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr partial %V{{[0-9]+}}, 0x0, %I{{[0-9]+}}, {{%S30|%LD_PARTIAL_REG}}, %SP{{[0-9]+}}
  }
}
