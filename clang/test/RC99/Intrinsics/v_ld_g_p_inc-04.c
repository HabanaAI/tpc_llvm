// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck %s

void main(tensor in, char size, char offset, int desta, int vpreda) {
  int5 indx = { 0, 0, 0, 0, 0 };
  void __global *addr = gen_addr(indx, in, 0, 0, 1, 0);
  bool256 __local *vpredp = (bool256 __local *)vpreda;
  bool256 vpred = *vpredp;

  int64 __local *dest_ptr = (int64 __local *)desta;

  {
    float64 __local *dest = (float64 __local *)dest_ptr;
    float64 res = {0};
    res = v_f32_ld_g_inc_partial_vb(&addr, size, offset, 0, res, to_bool64(vpred), 0);
    // CHECK: ld_g inc_4 %V{{[0-9]+}}, %AD{{[0-9]+}}, %LD_PARTIAL_REG, %VP{{[0-9]+}}
    *dest++ = res;
  }

  {
    bfloat128 __local *dest = (bfloat128 __local *)dest_ptr;
    bfloat128 res = {0};
    res = v_bf16_ld_g_inc_partial_vb(&addr, size, offset, 0, res, to_bool128(vpred), 0);
    // CHECK: ld_g inc_2 %V{{[0-9]+}}, %AD{{[0-9]+}}, %LD_PARTIAL_REG, %VP{{[0-9]+}}
    *dest++ = res;
  }

  {
    half128 __local *dest = (half128 __local *)dest_ptr;
    half128 res = {0};
    res = v_f16_ld_g_inc_partial_vb(&addr, size, offset, 0, res, to_bool128(vpred), 0);
    // CHECK: ld_g inc_2 %V{{[0-9]+}}, %AD{{[0-9]+}}, %LD_PARTIAL_REG, %VP{{[0-9]+}}
    *dest++ = res;
  }

  {
    minifloat256 __local *dest = (minifloat256 __local *)dest_ptr;
    minifloat256 res = {0};
    res = v_f8_ld_g_inc_partial_vb(&addr, size, offset, 0, res, vpred, 0);
    // CHECK: ld_g inc_1 %V{{[0-9]+}}, %AD{{[0-9]+}}, %LD_PARTIAL_REG, %VP{{[0-9]+}}
    *dest++ = res;
  }

  {
    minihalf256 __local *dest = (minihalf256 __local *)dest_ptr;
    minihalf256 res = {0};
    res = v_h8_ld_g_inc_partial_vb(&addr, size, offset, 0, res, vpred, 0);
    // CHECK: ld_g inc_1 %V{{[0-9]+}}, %AD{{[0-9]+}}, %LD_PARTIAL_REG, %VP{{[0-9]+}}
    *dest++ = res;
  }

  {
    int64 __local *dest = (int64 __local *)dest_ptr;
    int64 res = {0};
    res = v_i32_ld_g_inc_partial_vb(&addr, size, offset, 0, res, to_bool64(vpred), 0);
    // CHECK: ld_g inc_4 %V{{[0-9]+}}, %AD{{[0-9]+}}, %LD_PARTIAL_REG, %VP{{[0-9]+}}
    *dest++ = res;
  }

  {
    uint64 __local *dest = (uint64 __local *)dest_ptr;
    uint64 res = {0};
    res = v_u32_ld_g_inc_partial_vb(&addr, size, offset, 0, res, to_bool64(vpred), 0);
    // CHECK: ld_g inc_4 %V{{[0-9]+}}, %AD{{[0-9]+}}, %LD_PARTIAL_REG, %VP{{[0-9]+}}
    *dest++ = res;
  }

  {
    short128 __local *dest = (short128 __local *)dest_ptr;
    short128 res = {0};
    res = v_i16_ld_g_inc_partial_vb(&addr, size, offset, 0, res, to_bool128(vpred), 0);
    // CHECK: ld_g inc_2 %V{{[0-9]+}}, %AD{{[0-9]+}}, %LD_PARTIAL_REG, %VP{{[0-9]+}}
    *dest++ = res;
  }

  {
    ushort128 __local *dest = (ushort128 __local *)dest_ptr;
    ushort128 res = {0};
    res = v_u16_ld_g_inc_partial_vb(&addr, size, offset, 0, res, to_bool128(vpred), 0);
    // CHECK: ld_g inc_2 %V{{[0-9]+}}, %AD{{[0-9]+}}, %LD_PARTIAL_REG, %VP{{[0-9]+}}
    *dest++ = res;
  }

  {
    char256 __local *dest = (char256 __local *)dest_ptr;
    char256 res = {0};
    res = v_i8_ld_g_inc_partial_vb(&addr, size, offset, 0, res, vpred, 0);
    // CHECK: ld_g inc_1 %V{{[0-9]+}}, %AD{{[0-9]+}}, %LD_PARTIAL_REG, %VP{{[0-9]+}}
    *dest++ = res;
  }

  {
    uchar256 __local *dest = (uchar256 __local *)dest_ptr;
    uchar256 res = {0};
    res = v_u8_ld_g_inc_partial_vb(&addr, size, offset, 0, res, vpred, 0);
    // CHECK: ld_g inc_1 %V{{[0-9]+}}, %AD{{[0-9]+}}, %LD_PARTIAL_REG, %VP{{[0-9]+}}
    *dest++ = res;
  }
}
