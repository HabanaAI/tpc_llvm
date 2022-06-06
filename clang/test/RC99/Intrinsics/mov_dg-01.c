// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o -  | FileCheck --check-prefixes=CHECK,G2P %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 %s -o -  | FileCheck --check-prefixes=CHECK,G2P,G3 %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK,G2P,G3 %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck --check-prefixes=CHECK,G2P,G3 %s

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
    res = v_f32_mov_dual_group_b(x, 1, 0, 3, SW_WR_LOWER_GROUP, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: mov_dg src=0 dst=3 wr_lg=0x1 wr_ug=0x0 [[DEST]], [[SRC]], 0x1, [[PRED]]

    res = v_f32_mov_dual_group_b(x, 7, 1, 2, SW_WR_LOWER_GROUP | SW_WR_UPPER_GROUP, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: mov_dg src=1 dst=2 wr_lg=0x1 wr_ug=0x1 [[DEST]], [[SRC]], 0x7, [[PRED]]

    res = v_f32_mov_dual_group_b(x, 15, 3, 1, SW_WR_UPPER_GROUP, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: mov_dg src=3 dst=1 wr_lg=0x0 wr_ug=0x1 [[DEST]], [[SRC]], 0xf, [[PRED]]

    res = v_f32_mov_dual_group_vb(x, 1, 0, 3, SW_WR_LOWER_GROUP, res, to_bool64(vpred), 0);
    *dest_ptr++ = res;
// CHECK: mov_dg src=0 dst=3 wr_lg=0x1 wr_ug=0x0 [[DEST]], [[SRC]], 0x1, [[VPRED]]

    res = v_f32_mov_dual_group_vb(x, 7, 1, 2, SW_WR_LOWER_GROUP | SW_WR_UPPER_GROUP, res, to_bool64(vpred), 0);
    *dest_ptr++ = res;
// CHECK: mov_dg src=1 dst=2 wr_lg=0x1 wr_ug=0x1 [[DEST]], [[SRC]], 0x7, [[VPRED]]

    res = v_f32_mov_dual_group_vb(x, 15, 3, 1, SW_WR_UPPER_GROUP, res, to_bool64(vpred), 0);
    *dest_ptr++ = res;
// CHECK: mov_dg src=3 dst=1 wr_lg=0x0 wr_ug=0x1 [[DEST]], [[SRC]], 0xf, [[VPRED]]

    income = res;
  }


  {
    int64 res = as_int64(income);
    int64 xx = as_int64(x);
    int64 *d_ptr = (int64 __local *)dest_ptr;
    
    res = v_i32_mov_dual_group_b(xx, 3, 0, 3, SW_WR_LOWER_GROUP, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=0 dst=3 wr_lg=0x1 wr_ug=0x0 [[DEST]], [[SRC]], 0x3, [[PRED]]

    res = v_i32_mov_dual_group_b(xx, 7, 1, 2, SW_WR_LOWER_GROUP | SW_WR_UPPER_GROUP, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=1 dst=2 wr_lg=0x1 wr_ug=0x1 [[DEST]], [[SRC]], 0x7, [[PRED]]

    res = v_i32_mov_dual_group_b(xx, 15, 3, 1, SW_WR_UPPER_GROUP, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=3 dst=1 wr_lg=0x0 wr_ug=0x1 [[DEST]], [[SRC]], 0xf, [[PRED]]

    res = v_i32_mov_dual_group_vb(xx, 1, 0, 3, SW_WR_LOWER_GROUP, res, to_bool64(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=0 dst=3 wr_lg=0x1 wr_ug=0x0 [[DEST]], [[SRC]], 0x1, [[VPRED]]

    res = v_i32_mov_dual_group_vb(xx, 7, 1, 2, SW_WR_LOWER_GROUP | SW_WR_UPPER_GROUP, res, to_bool64(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=1 dst=2 wr_lg=0x1 wr_ug=0x1 [[DEST]], [[SRC]], 0x7, [[VPRED]]

    res = v_i32_mov_dual_group_vb(xx, 15, 3, 1, SW_WR_UPPER_GROUP, res, to_bool64(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=3 dst=1 wr_lg=0x0 wr_ug=0x1 [[DEST]], [[SRC]], 0xf, [[VPRED]]

    income = as_float64(res);
  }

  {
    uint64 res = as_uint64(income);
    uint64 xx = as_uint64(x);
    uint64 *d_ptr = (uint64 __local *)dest_ptr;
    
    res = v_u32_mov_dual_group_b(xx, 4, 0, 3, SW_WR_LOWER_GROUP, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=0 dst=3 wr_lg=0x1 wr_ug=0x0 [[DEST]], [[SRC]], 0x4, [[PRED]]

    res = v_u32_mov_dual_group_b(xx, 7, 1, 2, SW_WR_LOWER_GROUP | SW_WR_UPPER_GROUP, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=1 dst=2 wr_lg=0x1 wr_ug=0x1 [[DEST]], [[SRC]], 0x7, [[PRED]]

    res = v_u32_mov_dual_group_b(xx, 15, 3, 1, SW_WR_UPPER_GROUP, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=3 dst=1 wr_lg=0x0 wr_ug=0x1 [[DEST]], [[SRC]], 0xf, [[PRED]]

    res = v_u32_mov_dual_group_vb(xx, 1, 0, 3, SW_WR_LOWER_GROUP, res, to_bool64(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=0 dst=3 wr_lg=0x1 wr_ug=0x0 [[DEST]], [[SRC]], 0x1, [[VPRED]]

    res = v_u32_mov_dual_group_vb(xx, 7, 1, 2, SW_WR_LOWER_GROUP | SW_WR_UPPER_GROUP, res, to_bool64(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=1 dst=2 wr_lg=0x1 wr_ug=0x1 [[DEST]], [[SRC]], 0x7, [[VPRED]]

    res = v_u32_mov_dual_group_vb(xx, 15, 3, 1, SW_WR_UPPER_GROUP, res, to_bool64(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=3 dst=1 wr_lg=0x0 wr_ug=0x1 [[DEST]], [[SRC]], 0xf, [[VPRED]]

    income = as_float64(res);
  }

  {
    short128 res = as_short128(income);
    short128 xx = as_short128(x);
    short128 *d_ptr = (short128 __local *)dest_ptr;
    
    res = v_i16_mov_dual_group_b(xx, 5, 0, 3, SW_WR_LOWER_GROUP, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=0 dst=3 wr_lg=0x1 wr_ug=0x0 [[DEST]], [[SRC]], 0x5, [[PRED]]

    res = v_i16_mov_dual_group_b(xx, 7, 1, 2, SW_WR_LOWER_GROUP | SW_WR_UPPER_GROUP, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=1 dst=2 wr_lg=0x1 wr_ug=0x1 [[DEST]], [[SRC]], 0x7, [[PRED]]

    res = v_i16_mov_dual_group_b(xx, 15, 3, 1, SW_WR_UPPER_GROUP, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=3 dst=1 wr_lg=0x0 wr_ug=0x1 [[DEST]], [[SRC]], 0xf, [[PRED]]

    res = v_i16_mov_dual_group_vb(xx, 1, 0, 3, SW_WR_LOWER_GROUP, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=0 dst=3 wr_lg=0x1 wr_ug=0x0 [[DEST]], [[SRC]], 0x1, [[VPRED]]

    res = v_i16_mov_dual_group_vb(xx, 7, 1, 2, SW_WR_LOWER_GROUP | SW_WR_UPPER_GROUP, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=1 dst=2 wr_lg=0x1 wr_ug=0x1 [[DEST]], [[SRC]], 0x7, [[VPRED]]

    res = v_i16_mov_dual_group_vb(xx, 15, 3, 1, SW_WR_UPPER_GROUP, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=3 dst=1 wr_lg=0x0 wr_ug=0x1 [[DEST]], [[SRC]], 0xf, [[VPRED]]

    income = as_float64(res);
  }

  {
    ushort128 res = as_ushort128(income);
    ushort128 xx = as_ushort128(x);
    ushort128 *d_ptr = (ushort128 __local *)dest_ptr;
    
    res = v_u16_mov_dual_group_b(xx, 6, 0, 3, SW_WR_LOWER_GROUP, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=0 dst=3 wr_lg=0x1 wr_ug=0x0 [[DEST]], [[SRC]], 0x6, [[PRED]]

    res = v_u16_mov_dual_group_b(xx, 7, 1, 2, SW_WR_LOWER_GROUP | SW_WR_UPPER_GROUP, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=1 dst=2 wr_lg=0x1 wr_ug=0x1 [[DEST]], [[SRC]], 0x7, [[PRED]]

    res = v_u16_mov_dual_group_b(xx, 15, 3, 1, SW_WR_UPPER_GROUP, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=3 dst=1 wr_lg=0x0 wr_ug=0x1 [[DEST]], [[SRC]], 0xf, [[PRED]]

    res = v_u16_mov_dual_group_vb(xx, 1, 0, 3, SW_WR_LOWER_GROUP, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=0 dst=3 wr_lg=0x1 wr_ug=0x0 [[DEST]], [[SRC]], 0x1, [[VPRED]]

    res = v_u16_mov_dual_group_vb(xx, 7, 1, 2, SW_WR_LOWER_GROUP | SW_WR_UPPER_GROUP, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=1 dst=2 wr_lg=0x1 wr_ug=0x1 [[DEST]], [[SRC]], 0x7, [[VPRED]]

    res = v_u16_mov_dual_group_vb(xx, 15, 3, 1, SW_WR_UPPER_GROUP, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=3 dst=1 wr_lg=0x0 wr_ug=0x1 [[DEST]], [[SRC]], 0xf, [[VPRED]]

    income = as_float64(res);
  }

  {
    char256 res = as_char256(income);
    char256 xx = as_char256(x);
    char256 *d_ptr = (char256 __local *)dest_ptr;
    
    res = v_i8_mov_dual_group_b(xx, 8, 0, 3, SW_WR_LOWER_GROUP, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=0 dst=3 wr_lg=0x1 wr_ug=0x0 [[DEST]], [[SRC]], 0x8, [[PRED]]

    res = v_i8_mov_dual_group_b(xx, 7, 1, 2, SW_WR_LOWER_GROUP | SW_WR_UPPER_GROUP, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=1 dst=2 wr_lg=0x1 wr_ug=0x1 [[DEST]], [[SRC]], 0x7, [[PRED]]

    res = v_i8_mov_dual_group_b(xx, 15, 3, 1, SW_WR_UPPER_GROUP, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=3 dst=1 wr_lg=0x0 wr_ug=0x1 [[DEST]], [[SRC]], 0xf, [[PRED]]

    res = v_i8_mov_dual_group_vb(xx, 1, 0, 3, SW_WR_LOWER_GROUP, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=0 dst=3 wr_lg=0x1 wr_ug=0x0 [[DEST]], [[SRC]], 0x1, [[VPRED]]

    res = v_i8_mov_dual_group_vb(xx, 7, 1, 2, SW_WR_LOWER_GROUP | SW_WR_UPPER_GROUP, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=1 dst=2 wr_lg=0x1 wr_ug=0x1 [[DEST]], [[SRC]], 0x7, [[VPRED]]

    res = v_i8_mov_dual_group_vb(xx, 15, 3, 1, SW_WR_UPPER_GROUP, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=3 dst=1 wr_lg=0x0 wr_ug=0x1 [[DEST]], [[SRC]], 0xf, [[VPRED]]

    income = as_float64(res);
  }

  {
    uchar256 res = as_uchar256(income);
    uchar256 xx = as_uchar256(x);
    uchar256 *d_ptr = (uchar256 __local *)dest_ptr;
    
    res = v_u8_mov_dual_group_b(xx, 9, 0, 3, SW_WR_LOWER_GROUP, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=0 dst=3 wr_lg=0x1 wr_ug=0x0 [[DEST]], [[SRC]], 0x9, [[PRED]]

    res = v_u8_mov_dual_group_b(xx, 7, 1, 2, SW_WR_LOWER_GROUP | SW_WR_UPPER_GROUP, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=1 dst=2 wr_lg=0x1 wr_ug=0x1 [[DEST]], [[SRC]], 0x7, [[PRED]]

    res = v_u8_mov_dual_group_b(xx, 15, 3, 1, SW_WR_UPPER_GROUP, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=3 dst=1 wr_lg=0x0 wr_ug=0x1 [[DEST]], [[SRC]], 0xf, [[PRED]]

    res = v_u8_mov_dual_group_vb(xx, 1, 0, 3, SW_WR_LOWER_GROUP, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=0 dst=3 wr_lg=0x1 wr_ug=0x0 [[DEST]], [[SRC]], 0x1, [[VPRED]]

    res = v_u8_mov_dual_group_vb(xx, 7, 1, 2, SW_WR_LOWER_GROUP | SW_WR_UPPER_GROUP, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=1 dst=2 wr_lg=0x1 wr_ug=0x1 [[DEST]], [[SRC]], 0x7, [[VPRED]]

    res = v_u8_mov_dual_group_vb(xx, 15, 3, 1, SW_WR_UPPER_GROUP, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=3 dst=1 wr_lg=0x0 wr_ug=0x1 [[DEST]], [[SRC]], 0xf, [[VPRED]]

    income = as_float64(res);
  }

}
