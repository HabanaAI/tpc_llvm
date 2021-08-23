// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o -  | FileCheck %s



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
    
    res = v_f32_mov_dual_group_all_b(x, 1, 0, 1, 2, 3, SW_WR_LOWER_GROUP0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: mov_dg all sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=1 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x1, [[PRED]]

    res = v_f32_mov_dual_group_all_b(x, 7, 1, 2, 3, 0, SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: mov_dg all sdg0=1 sdg1=2 sdg2=3 sdg3=0 weg0=3 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x7, [[PRED]]

    res = v_f32_mov_dual_group_all_b(x, 15, 2, 3, 0, 1, SW_WR_UPPER_GROUP0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: mov_dg all sdg0=2 sdg1=3 sdg2=0 sdg3=1 weg0=2 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0xf, [[PRED]]

    res = v_f32_mov_dual_group_all_b(x, 15, 3, 0, 1, 2, SW_WR_UPPER_GROUP0 | SW_WR_UPPER_GROUP1, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: mov_dg all sdg0=3 sdg1=0 sdg2=1 sdg3=2 weg0=2 weg1=2 weg2=0 weg3=0 [[DEST]], [[SRC]], 0xf, [[PRED]]

    res = v_f32_mov_dual_group_all_b(x, 15, 3, 3, 3, 3, SW_WR_UPPER_GROUP0 | SW_WR_UPPER_GROUP1 | SW_WR_UPPER_GROUP2, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: mov_dg all sdg0=3 sdg1=3 sdg2=3 sdg3=3 weg0=2 weg1=2 weg2=2 weg3=0 [[DEST]], [[SRC]], 0xf, [[PRED]]

    res = v_f32_mov_dual_group_all_b(x, 15, 2, 2, 2, 2, SW_WR_UPPER_GROUP0 | SW_WR_UPPER_GROUP1 | SW_WR_UPPER_GROUP2 | SW_WR_UPPER_GROUP3, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: mov_dg all sdg0=2 sdg1=2 sdg2=2 sdg3=2 weg0=2 weg1=2 weg2=2 weg3=2 [[DEST]], [[SRC]], 0xf, [[PRED]]

    res = v_f32_mov_dual_group_all_b(x, 15, 1, 1, 1, 1, SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP1 | SW_WR_UPPER_GROUP2 | SW_WR_UPPER_GROUP3, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: mov_dg all sdg0=1 sdg1=1 sdg2=1 sdg3=1 weg0=1 weg1=2 weg2=2 weg3=2 [[DEST]], [[SRC]], 0xf, [[PRED]]

    res = v_f32_mov_dual_group_all_b(x, 15, 0, 0, 0, 0, SW_WR_LOWER_GROUP0 | SW_WR_LOWER_GROUP1 | SW_WR_UPPER_GROUP2 | SW_WR_UPPER_GROUP3, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: mov_dg all sdg0=0 sdg1=0 sdg2=0 sdg3=0 weg0=1 weg1=1 weg2=2 weg3=2 [[DEST]], [[SRC]], 0xf, [[PRED]]

    res = v_f32_mov_dual_group_all_b(x, 15, 0, 0, 1, 1, SW_WR_LOWER_GROUP0 | SW_WR_LOWER_GROUP1 | SW_WR_LOWER_GROUP2 | SW_WR_UPPER_GROUP3, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: mov_dg all sdg0=0 sdg1=0 sdg2=1 sdg3=1 weg0=1 weg1=1 weg2=1 weg3=2 [[DEST]], [[SRC]], 0xf, [[PRED]]

    res = v_f32_mov_dual_group_all_b(x, 15, 1, 1, 0, 0, SW_WR_LOWER_GROUP0 | SW_WR_LOWER_GROUP1 | SW_WR_LOWER_GROUP2 | SW_WR_LOWER_GROUP3, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: mov_dg all sdg0=1 sdg1=1 sdg2=0 sdg3=0 weg0=1 weg1=1 weg2=1 weg3=1 [[DEST]], [[SRC]], 0xf, [[PRED]]


    res = v_f32_mov_dual_group_all_vb(x, 1, 0, 1, 2, 3, SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0, res, to_bool64(vpred), 0);
    *dest_ptr++ = res;
// CHECK: mov_dg all sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x1, [[VPRED]]

    res = v_f32_mov_dual_group_all_vb(x, 7, 1, 2, 3, 0, SW_WR_LOWER_GROUP1 | SW_WR_UPPER_GROUP1, res, to_bool64(vpred), 0);
    *dest_ptr++ = res;
// CHECK: mov_dg all sdg0=1 sdg1=2 sdg2=3 sdg3=0 weg0=0 weg1=3 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x7, [[VPRED]]

    res = v_f32_mov_dual_group_all_vb(x, 15, 3, 0, 1, 2, SW_WR_UPPER_GROUP0 | SW_WR_UPPER_GROUP1, res, to_bool64(vpred), 0);
    *dest_ptr++ = res;
// CHECK: mov_dg all sdg0=3 sdg1=0 sdg2=1 sdg3=2 weg0=2 weg1=2 weg2=0 weg3=0 [[DEST]], [[SRC]], 0xf, [[VPRED]]

    income = res;
  }

  {
    bfloat128 res = as_bfloat128(income);
    bfloat128 xx = as_bfloat128(x);
    bfloat128 __local *d_ptr = (bfloat128 __local *)dest_ptr;
    
    res = v_bf16_mov_dual_group_all_b(xx, 1, 0, 1, 2, 3, SW_WR_LOWER_GROUP0, res, pred, 0);
    *d_ptr++ = res;
// G2P: mov_dg all sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=1 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x1, [[PRED]]

    res = v_bf16_mov_dual_group_all_b(xx, 7, 1, 2, 3, 0, SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0, res, pred, 0);
    *d_ptr++ = res;
    
// G2P: mov_dg all sdg0=1 sdg1=2 sdg2=3 sdg3=0 weg0=3 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x7, [[PRED]]
    res = v_bf16_mov_dual_group_all_b(xx, 15, 2, 3, 0, 1, SW_WR_UPPER_GROUP0, res, pred, 0);
    *d_ptr++ = res;
// G2P: mov_dg all sdg0=2 sdg1=3 sdg2=0 sdg3=1 weg0=2 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0xf, [[PRED]]


    res = v_bf16_mov_dual_group_all_vb(xx, 1, 0, 1, 2, 3, SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// G2P: mov_dg all sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x1, [[VPRED]]

    res = v_bf16_mov_dual_group_all_vb(xx, 7, 1, 2, 3, 0, SW_WR_LOWER_GROUP1 | SW_WR_UPPER_GROUP1, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// G2P: mov_dg all sdg0=1 sdg1=2 sdg2=3 sdg3=0 weg0=0 weg1=3 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x7, [[VPRED]]

    res = v_bf16_mov_dual_group_all_vb(xx, 15, 3, 0, 1, 2, SW_WR_UPPER_GROUP0 | SW_WR_UPPER_GROUP1, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// G2P: mov_dg all sdg0=3 sdg1=0 sdg2=1 sdg3=2 weg0=2 weg1=2 weg2=0 weg3=0 [[DEST]], [[SRC]], 0xf, [[VPRED]]

    income = as_float64(res);
  }

  {
    int64 res = as_int64(income);
    int64 xx = as_int64(x);
    int64 __local *d_ptr = (int64 __local *)dest_ptr;
    
    res = v_i32_mov_dual_group_all_b(xx, 1, 0, 1, 2, 3, SW_WR_LOWER_GROUP0, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=1 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x1, [[PRED]]

    res = v_i32_mov_dual_group_all_b(xx, 7, 1, 2, 3, 0, SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0, res, pred, 0);
    *d_ptr++ = res;
    
// CHECK: mov_dg all sdg0=1 sdg1=2 sdg2=3 sdg3=0 weg0=3 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x7, [[PRED]]
    res = v_i32_mov_dual_group_all_b(xx, 15, 2, 3, 0, 1, SW_WR_UPPER_GROUP0, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=2 sdg1=3 sdg2=0 sdg3=1 weg0=2 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0xf, [[PRED]]


    res = v_i32_mov_dual_group_all_vb(xx, 1, 0, 1, 2, 3, SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0, res, to_bool64(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x1, [[VPRED]]

    res = v_i32_mov_dual_group_all_vb(xx, 7, 1, 2, 3, 0, SW_WR_LOWER_GROUP1 | SW_WR_UPPER_GROUP1, res, to_bool64(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=1 sdg1=2 sdg2=3 sdg3=0 weg0=0 weg1=3 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x7, [[VPRED]]

    res = v_i32_mov_dual_group_all_vb(xx, 15, 3, 0, 1, 2, SW_WR_UPPER_GROUP0 | SW_WR_UPPER_GROUP1, res, to_bool64(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=3 sdg1=0 sdg2=1 sdg3=2 weg0=2 weg1=2 weg2=0 weg3=0 [[DEST]], [[SRC]], 0xf, [[VPRED]]

    income = as_float64(res);
  }

  {
    uint64 res = as_uint64(income);
    uint64 xx = as_uint64(x);
    uint64 __local *d_ptr = (uint64 __local *)dest_ptr;
    
    res = v_u32_mov_dual_group_all_b(xx, 1, 0, 1, 2, 3, SW_WR_LOWER_GROUP0, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=1 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x1, [[PRED]]

    res = v_u32_mov_dual_group_all_b(xx, 7, 1, 2, 3, 0, SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0, res, pred, 0);
    *d_ptr++ = res;
    
// CHECK: mov_dg all sdg0=1 sdg1=2 sdg2=3 sdg3=0 weg0=3 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x7, [[PRED]]
    res = v_u32_mov_dual_group_all_b(xx, 15, 2, 3, 0, 1, SW_WR_UPPER_GROUP0, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=2 sdg1=3 sdg2=0 sdg3=1 weg0=2 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0xf, [[PRED]]


    res = v_u32_mov_dual_group_all_vb(xx, 1, 0, 1, 2, 3, SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0, res, to_bool64(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x1, [[VPRED]]

    res = v_u32_mov_dual_group_all_vb(xx, 7, 1, 2, 3, 0, SW_WR_LOWER_GROUP1 | SW_WR_UPPER_GROUP1, res, to_bool64(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=1 sdg1=2 sdg2=3 sdg3=0 weg0=0 weg1=3 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x7, [[VPRED]]

    res = v_u32_mov_dual_group_all_vb(xx, 15, 3, 0, 1, 2, SW_WR_UPPER_GROUP0 | SW_WR_UPPER_GROUP1, res, to_bool64(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=3 sdg1=0 sdg2=1 sdg3=2 weg0=2 weg1=2 weg2=0 weg3=0 [[DEST]], [[SRC]], 0xf, [[VPRED]]

    income = as_float64(res);
  }

  {
    short128 res = as_short128(income);
    short128 xx = as_short128(x);
    short128 __local *d_ptr = (short128 __local *)dest_ptr;
    
    res = v_i16_mov_dual_group_all_b(xx, 1, 0, 1, 2, 3, SW_WR_LOWER_GROUP0, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=1 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x1, [[PRED]]

    res = v_i16_mov_dual_group_all_b(xx, 7, 1, 2, 3, 0, SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0, res, pred, 0);
    *d_ptr++ = res;
    
// CHECK: mov_dg all sdg0=1 sdg1=2 sdg2=3 sdg3=0 weg0=3 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x7, [[PRED]]
    res = v_i16_mov_dual_group_all_b(xx, 15, 2, 3, 0, 1, SW_WR_UPPER_GROUP0, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=2 sdg1=3 sdg2=0 sdg3=1 weg0=2 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0xf, [[PRED]]


    res = v_i16_mov_dual_group_all_vb(xx, 1, 0, 1, 2, 3, SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x1, [[VPRED]]

    res = v_i16_mov_dual_group_all_vb(xx, 7, 1, 2, 3, 0, SW_WR_LOWER_GROUP1 | SW_WR_UPPER_GROUP1, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=1 sdg1=2 sdg2=3 sdg3=0 weg0=0 weg1=3 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x7, [[VPRED]]

    res = v_i16_mov_dual_group_all_vb(xx, 15, 3, 0, 1, 2, SW_WR_UPPER_GROUP0 | SW_WR_UPPER_GROUP1, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=3 sdg1=0 sdg2=1 sdg3=2 weg0=2 weg1=2 weg2=0 weg3=0 [[DEST]], [[SRC]], 0xf, [[VPRED]]

    income = as_float64(res);
  }

  {
    ushort128 res = as_ushort128(income);
    ushort128 xx = as_ushort128(x);
    ushort128 __local *d_ptr = (ushort128 __local *)dest_ptr;
    
    res = v_u16_mov_dual_group_all_b(xx, 1, 0, 1, 2, 3, SW_WR_LOWER_GROUP0, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=1 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x1, [[PRED]]

    res = v_u16_mov_dual_group_all_b(xx, 7, 1, 2, 3, 0, SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0, res, pred, 0);
    *d_ptr++ = res;
    
// CHECK: mov_dg all sdg0=1 sdg1=2 sdg2=3 sdg3=0 weg0=3 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x7, [[PRED]]
    res = v_u16_mov_dual_group_all_b(xx, 15, 2, 3, 0, 1, SW_WR_UPPER_GROUP0, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=2 sdg1=3 sdg2=0 sdg3=1 weg0=2 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0xf, [[PRED]]


    res = v_u16_mov_dual_group_all_vb(xx, 1, 0, 1, 2, 3, SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x1, [[VPRED]]

    res = v_u16_mov_dual_group_all_vb(xx, 7, 1, 2, 3, 0, SW_WR_LOWER_GROUP1 | SW_WR_UPPER_GROUP1, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=1 sdg1=2 sdg2=3 sdg3=0 weg0=0 weg1=3 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x7, [[VPRED]]

    res = v_u16_mov_dual_group_all_vb(xx, 15, 3, 0, 1, 2, SW_WR_UPPER_GROUP0 | SW_WR_UPPER_GROUP1, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=3 sdg1=0 sdg2=1 sdg3=2 weg0=2 weg1=2 weg2=0 weg3=0 [[DEST]], [[SRC]], 0xf, [[VPRED]]

    income = as_float64(res);
  }

  {
    char256 res = as_char256(income);
    char256 xx = as_char256(x);
    char256 __local *d_ptr = (char256 __local *)dest_ptr;
    
    res = v_i8_mov_dual_group_all_b(xx, 1, 0, 1, 2, 3, SW_WR_LOWER_GROUP0, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=1 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x1, [[PRED]]

    res = v_i8_mov_dual_group_all_b(xx, 7, 1, 2, 3, 0, SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0, res, pred, 0);
    *d_ptr++ = res;
    
// CHECK: mov_dg all sdg0=1 sdg1=2 sdg2=3 sdg3=0 weg0=3 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x7, [[PRED]]
    res = v_i8_mov_dual_group_all_b(xx, 15, 2, 3, 0, 1, SW_WR_UPPER_GROUP0, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=2 sdg1=3 sdg2=0 sdg3=1 weg0=2 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0xf, [[PRED]]


    res = v_i8_mov_dual_group_all_vb(xx, 1, 0, 1, 2, 3, SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x1, [[VPRED]]

    res = v_i8_mov_dual_group_all_vb(xx, 7, 1, 2, 3, 0, SW_WR_LOWER_GROUP1 | SW_WR_UPPER_GROUP1, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=1 sdg1=2 sdg2=3 sdg3=0 weg0=0 weg1=3 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x7, [[VPRED]]

    res = v_i8_mov_dual_group_all_vb(xx, 15, 3, 0, 1, 2, SW_WR_UPPER_GROUP0 | SW_WR_UPPER_GROUP1, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=3 sdg1=0 sdg2=1 sdg3=2 weg0=2 weg1=2 weg2=0 weg3=0 [[DEST]], [[SRC]], 0xf, [[VPRED]]

    income = as_float64(res);
  }

  {
    uchar256 res = as_uchar256(income);
    uchar256 xx = as_uchar256(x);
    uchar256 __local *d_ptr = (uchar256 __local *)dest_ptr;
    
    res = v_u8_mov_dual_group_all_b(xx, 1, 0, 1, 2, 3, SW_WR_LOWER_GROUP0, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=1 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x1, [[PRED]]

    res = v_u8_mov_dual_group_all_b(xx, 7, 1, 2, 3, 0, SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0, res, pred, 0);
    *d_ptr++ = res;
    
// CHECK: mov_dg all sdg0=1 sdg1=2 sdg2=3 sdg3=0 weg0=3 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x7, [[PRED]]
    res = v_u8_mov_dual_group_all_b(xx, 15, 2, 3, 0, 1, SW_WR_UPPER_GROUP0, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=2 sdg1=3 sdg2=0 sdg3=1 weg0=2 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0xf, [[PRED]]


    res = v_u8_mov_dual_group_all_vb(xx, 1, 0, 1, 2, 3, SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x1, [[VPRED]]

    res = v_u8_mov_dual_group_all_vb(xx, 7, 1, 2, 3, 0, SW_WR_LOWER_GROUP1 | SW_WR_UPPER_GROUP1, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=1 sdg1=2 sdg2=3 sdg3=0 weg0=0 weg1=3 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x7, [[VPRED]]

    res = v_u8_mov_dual_group_all_vb(xx, 15, 3, 0, 1, 2, SW_WR_UPPER_GROUP0 | SW_WR_UPPER_GROUP1, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=3 sdg1=0 sdg2=1 sdg3=2 weg0=2 weg1=2 weg2=0 weg3=0 [[DEST]], [[SRC]], 0xf, [[VPRED]]

    income = as_float64(res);
  }
}
