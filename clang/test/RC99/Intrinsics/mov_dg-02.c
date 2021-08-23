// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s  -o - | FileCheck %s



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
    bfloat128 res = as_bfloat128(income);
    bfloat128 xx = as_bfloat128(x);
    bfloat128 *d_ptr = (bfloat128 __local *)dest_ptr;
    
    res = v_bf16_mov_dual_group_b(xx, 2, 0, 3, SW_WR_LOWER_GROUP, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=0 dst=3 wr_lg=0x1 wr_ug=0x0 [[DEST]], [[SRC]], 0x2, [[PRED]]

    res = v_bf16_mov_dual_group_b(xx, 7, 1, 2, SW_WR_LOWER_GROUP | SW_WR_UPPER_GROUP, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=1 dst=2 wr_lg=0x1 wr_ug=0x1 [[DEST]], [[SRC]], 0x7, [[PRED]]

    res = v_bf16_mov_dual_group_b(xx, 15, 3, 1, SW_WR_UPPER_GROUP, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=3 dst=1 wr_lg=0x0 wr_ug=0x1 [[DEST]], [[SRC]], 0xf, [[PRED]]

    res = v_bf16_mov_dual_group_vb(xx, 1, 0, 3, SW_WR_LOWER_GROUP, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=0 dst=3 wr_lg=0x1 wr_ug=0x0 [[DEST]], [[SRC]], 0x1, [[VPRED]]

    res = v_bf16_mov_dual_group_vb(xx, 7, 1, 2, SW_WR_LOWER_GROUP | SW_WR_UPPER_GROUP, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=1 dst=2 wr_lg=0x1 wr_ug=0x1 [[DEST]], [[SRC]], 0x7, [[VPRED]]

    res = v_bf16_mov_dual_group_vb(xx, 15, 3, 1, SW_WR_UPPER_GROUP, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=3 dst=1 wr_lg=0x0 wr_ug=0x1 [[DEST]], [[SRC]], 0xf, [[VPRED]]

    income = as_float64(res);
  }

}
