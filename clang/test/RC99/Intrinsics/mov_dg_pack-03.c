// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 %s  -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck %s

void main(int dest, int src, int vpredp, _Bool pred) {
  float64 __local *dest_ptr = (float64 __local *)dest;
  float64 __local *src_ptr = (float64  __local *)src;
  bool256 __local *vpred_ptr = (bool256 __local *)vpredp;

  float64 x      = *src_ptr++;
  bool256 vpred  = *vpred_ptr++;
  float64 income = *dest_ptr;

// CHECK-DAG: ld_l_v   [[DEST:%V[0-9]+]], %S0
// CHECK-DAG: ld_l_v   [[SRC:%V[0-9]+]], %S1
// CHECK-DAG: ld_l_v   [[VPRED:%VP[0-9]+]], %S2
// CHECK-DAG: mov{{.*}}	[[PRED:%SP[0-9]+]], %S3

  {
    float64 res = income;
    res = v_f32_mov_dual_group_pack_b(x, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[PRED]]

    res = v_f32_mov_dual_group_pack_b(x, SW_PACK21, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[PRED]]

    res = v_f32_mov_dual_group_pack_b(x, SW_PACK41, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: mov_dg pack pack_type=1 [[DEST]], [[SRC]], [[PRED]]

    res = v_f32_mov_dual_group_pack_vb(x, 0, res, to_bool64(vpred), 0);
    *dest_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[VPRED]]

    res = v_f32_mov_dual_group_pack_vb(x, SW_PACK21, res, to_bool64(vpred), 0);
    *dest_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[VPRED]]

    res = v_f32_mov_dual_group_pack_vb(x, SW_PACK41, res, to_bool64(vpred), 0);
    *dest_ptr++ = res;
// CHECK: mov_dg pack pack_type=1 [[DEST]], [[SRC]], [[VPRED]]

    income = res;
  }

  {
    bfloat128 res = as_bfloat128(income);
    bfloat128 xx = as_bfloat128(x);
    bfloat128 __local *d_ptr = (bfloat128 __local *)dest_ptr;

    res = v_bf16_mov_dual_group_pack_b(xx, 0, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[PRED]]

    res = v_bf16_mov_dual_group_pack_b(xx, SW_PACK21, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[PRED]]

    res = v_bf16_mov_dual_group_pack_b(xx, SW_PACK41, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=1 [[DEST]], [[SRC]], [[PRED]]

    res = v_bf16_mov_dual_group_pack_vb(xx, 0, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[VPRED]]

    res = v_bf16_mov_dual_group_pack_vb(xx, SW_PACK21, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[VPRED]]

    res = v_bf16_mov_dual_group_pack_vb(xx, SW_PACK41, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=1 [[DEST]], [[SRC]], [[VPRED]]

    income = as_float64(res);
  }

  {
    half128 res = as_half128(income);
    half128 xx = as_half128(x);
    half128 __local *d_ptr = (half128 __local *)dest_ptr;

    res = v_f16_mov_dual_group_pack_b(xx, 0, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[PRED]]

    res = v_f16_mov_dual_group_pack_b(xx, SW_PACK21, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[PRED]]

    res = v_f16_mov_dual_group_pack_b(xx, SW_PACK41, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=1 [[DEST]], [[SRC]], [[PRED]]

    res = v_f16_mov_dual_group_pack_vb(xx, 0, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[VPRED]]

    res = v_f16_mov_dual_group_pack_vb(xx, SW_PACK21, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[VPRED]]

    res = v_f16_mov_dual_group_pack_vb(xx, SW_PACK41, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=1 [[DEST]], [[SRC]], [[VPRED]]

    income = as_float64(res);
  }

  {
    int64 res = as_int64(income);
    int64 xx = as_int64(x);
    int64 __local *d_ptr = (int64 __local *)dest_ptr;

    res = v_i32_mov_dual_group_pack_b(xx, 0, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[PRED]]

    res = v_i32_mov_dual_group_pack_b(xx, SW_PACK21, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[PRED]]

    res = v_i32_mov_dual_group_pack_b(xx, SW_PACK41, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=1 [[DEST]], [[SRC]], [[PRED]]

    res = v_i32_mov_dual_group_pack_vb(xx, 0, res, to_bool64(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[VPRED]]

    res = v_i32_mov_dual_group_pack_vb(xx, SW_PACK21, res, to_bool64(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[VPRED]]

    res = v_i32_mov_dual_group_pack_vb(xx, SW_PACK41, res, to_bool64(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=1 [[DEST]], [[SRC]], [[VPRED]]

    income = as_float64(res);
  }

  {
    uint64 res = as_uint64(income);
    uint64 xx = as_uint64(x);
    uint64 __local *d_ptr = (uint64 __local *)dest_ptr;

    res = v_u32_mov_dual_group_pack_b(xx, 0, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[PRED]]

    res = v_u32_mov_dual_group_pack_b(xx, SW_PACK21, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[PRED]]

    res = v_u32_mov_dual_group_pack_b(xx, SW_PACK41, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=1 [[DEST]], [[SRC]], [[PRED]]

    res = v_u32_mov_dual_group_pack_vb(xx, 0, res, to_bool64(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[VPRED]]

    res = v_u32_mov_dual_group_pack_vb(xx, SW_PACK21, res, to_bool64(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[VPRED]]

    res = v_u32_mov_dual_group_pack_vb(xx, SW_PACK41, res, to_bool64(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=1 [[DEST]], [[SRC]], [[VPRED]]

    income = as_float64(res);
  }

  {
    short128 res = as_short128(income);
    short128 xx = as_short128(x);
    short128 __local *d_ptr = (short128 __local *)dest_ptr;

    res = v_i16_mov_dual_group_pack_b(xx, 0, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[PRED]]

    res = v_i16_mov_dual_group_pack_b(xx, SW_PACK21, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[PRED]]

    res = v_i16_mov_dual_group_pack_b(xx, SW_PACK41, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=1 [[DEST]], [[SRC]], [[PRED]]

    res = v_i16_mov_dual_group_pack_vb(xx, 0, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[VPRED]]

    res = v_i16_mov_dual_group_pack_vb(xx, SW_PACK21, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[VPRED]]

    res = v_i16_mov_dual_group_pack_vb(xx, SW_PACK41, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=1 [[DEST]], [[SRC]], [[VPRED]]

    income = as_float64(res);
  }

  {
    ushort128 res = as_ushort128(income);
    ushort128 xx = as_ushort128(x);
    ushort128 __local *d_ptr = (ushort128 __local *)dest_ptr;

    res = v_u16_mov_dual_group_pack_b(xx, 0, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[PRED]]

    res = v_u16_mov_dual_group_pack_b(xx, SW_PACK21, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[PRED]]

    res = v_u16_mov_dual_group_pack_b(xx, SW_PACK41, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=1 [[DEST]], [[SRC]], [[PRED]]

    res = v_u16_mov_dual_group_pack_vb(xx, 0, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[VPRED]]

    res = v_u16_mov_dual_group_pack_vb(xx, SW_PACK21, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[VPRED]]

    res = v_u16_mov_dual_group_pack_vb(xx, SW_PACK41, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=1 [[DEST]], [[SRC]], [[VPRED]]

    income = as_float64(res);
  }

  {
    char256 res = as_char256(income);
    char256 xx = as_char256(x);
    char256 __local *d_ptr = (char256 __local *)dest_ptr;

    res = v_i8_mov_dual_group_pack_b(xx, 0, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[PRED]]

    res = v_i8_mov_dual_group_pack_b(xx, SW_PACK21, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[PRED]]

    res = v_i8_mov_dual_group_pack_b(xx, SW_PACK41, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=1 [[DEST]], [[SRC]], [[PRED]]

    res = v_i8_mov_dual_group_pack_vb(xx, 0, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[VPRED]]

    res = v_i8_mov_dual_group_pack_vb(xx, SW_PACK21, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[VPRED]]

    res = v_i8_mov_dual_group_pack_vb(xx, SW_PACK41, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=1 [[DEST]], [[SRC]], [[VPRED]]

    income = as_float64(res);
  }

  {
    uchar256 res = as_uchar256(income);
    uchar256 xx = as_uchar256(x);
    uchar256 __local *d_ptr = (uchar256 __local *)dest_ptr;

    res = v_u8_mov_dual_group_pack_b(xx, 0, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[PRED]]

    res = v_u8_mov_dual_group_pack_b(xx, SW_PACK21, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[PRED]]

    res = v_u8_mov_dual_group_pack_b(xx, SW_PACK41, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=1 [[DEST]], [[SRC]], [[PRED]]

    res = v_u8_mov_dual_group_pack_vb(xx, 0, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[VPRED]]

    res = v_u8_mov_dual_group_pack_vb(xx, SW_PACK21, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 [[DEST]], [[SRC]], [[VPRED]]

    res = v_u8_mov_dual_group_pack_vb(xx, SW_PACK41, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=1 [[DEST]], [[SRC]], [[VPRED]]

    income = as_float64(res);
  }

}
