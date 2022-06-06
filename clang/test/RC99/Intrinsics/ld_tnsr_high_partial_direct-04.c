// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck %s

void main(unsigned addr1, unsigned addr2, int dest, _Bool pred, int vpredp, int partial_size, int partial_offset) {
  int64 __local *vector_ptr = (int64 __local *)dest;
  bool128 __local *vpred_ptr128 = (bool128 __local *)vpredp;
  bool128 vpred128 = *vpred_ptr128++;

  {
    float64 __local *dest_ptr = (float64 __local *)vector_ptr;
    float64 res = 0;
    
    res = v_f32_ld_tnsr_high_partial_direct_b(addr1, addr2, partial_size, partial_offset, 0, res, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr_high partial direct %V{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}, %LD_PARTIAL_REG, %SP{{[0-9]+}}
  }

  {
    int64 __local *dest_ptr = (int64 __local *)vector_ptr;
    int64 res = 0;

    res = v_i32_ld_tnsr_high_partial_direct_b(addr1, addr2, partial_size, partial_offset, 0, res, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr_high partial direct %V{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}, %LD_PARTIAL_REG, %SP{{[0-9]+}}
  }

  {
    uint64 __local *dest_ptr = (uint64 __local *)vector_ptr;
    uint64 res = 0;

    res = v_u32_ld_tnsr_high_partial_direct_b(addr1 + 1, addr2, partial_size, partial_offset, 0, res, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr_high partial direct %V{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}, %LD_PARTIAL_REG, %SP{{[0-9]+}}
  }

  {
    short128 __local *dest_ptr = (short128 __local *)vector_ptr;
    short128 res = 0;

    res = v_i16_ld_tnsr_high_partial_direct_b(addr1, addr2, partial_size, partial_offset, 0, res, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr_high partial direct %V{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}, %LD_PARTIAL_REG, %SP{{[0-9]+}}
  }

  {
    ushort128 __local *dest_ptr = (ushort128 __local *)vector_ptr;
    ushort128 res = 0;

    res = v_u16_ld_tnsr_high_partial_direct_b(addr1 + 1, addr2, partial_size, partial_offset, 0, res, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr_high partial direct %V{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}, %LD_PARTIAL_REG, %SP{{[0-9]+}}
  }

  {
    char256 __local *dest_ptr = (char256 __local *)vector_ptr;
    char256 res = 0;

    res = v_i8_ld_tnsr_high_partial_direct_b(addr1, addr2, partial_size, partial_offset, 0, res, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr_high partial direct %V{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}, %LD_PARTIAL_REG, %SP{{[0-9]+}}
  }

  {
    uchar256 __local *dest_ptr  = (uchar256 __local *)vector_ptr;
    uchar256 res  = 0;

    res = v_u8_ld_tnsr_high_partial_direct_b(addr1 + 1, addr2, partial_size, partial_offset, 0, res, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr_high partial direct %V{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}, %LD_PARTIAL_REG, %SP{{[0-9]+}}
  }

  {
    bool256 __local *dest_ptr = (bool256 __local *)vector_ptr;
    bool256 res = 0;

    res = v_i1_ld_tnsr_high_partial_direct_b(addr1, addr2, partial_size, partial_offset, 0, res, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr_high partial direct %VP{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}, %LD_PARTIAL_REG, %SP{{[0-9]+}}
  }


  {
    half128 __local *dest_ptr = (half128 __local *)vector_ptr;
    half128 res = 0;
    
    res = v_f16_ld_tnsr_high_partial_direct_b(addr1, addr2, partial_size, partial_offset, 0, res, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr_high partial direct %V{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}, %LD_PARTIAL_REG, %SP{{[0-9]+}}
  }

  {
    half128 __local *dest_ptr = (half128 __local *)vector_ptr;
    half128 res = 0;
    
    res = v_f16_ld_tnsr_high_partial_direct_vb(addr1, addr2, partial_size, partial_offset, 0, res, vpred128, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr_high partial direct %V{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}, %LD_PARTIAL_REG, %VP{{[0-9]+}}
  }

  {
    int64 __local *dest_ptr = (int64 __local *)vector_ptr;
    int64 res = 0;

    res = v_i32_ld_tnsr_high_partial_direct_b(addr1, addr2, partial_size, partial_offset, SW_UNPCK_8_TO_32, res, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr_high partial UNPCK_8_TO_32 direct %V{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}, %LD_PARTIAL_REG, %SP{{[0-9]+}}
  }

  {
    int64 __local *dest_ptr = (int64 __local *)vector_ptr;
    int64 res = 0;

    res = v_i32_ld_tnsr_high_partial_direct_b(addr1, addr2, partial_size, partial_offset, SW_UNPACK, res, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr_high partial unpack direct %V{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}, %LD_PARTIAL_REG, %SP{{[0-9]+}}
  }

  {
    int64 __local *dest_ptr = (int64 __local *)vector_ptr;
    int64 res = 0;

    res = v_i32_ld_tnsr_high_partial_direct_b(addr1, addr2, partial_size, partial_offset, SW_L0CS, res, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr_high partial l0cs direct %V{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}, %LD_PARTIAL_REG, %SP{{[0-9]+}}
  }

}
