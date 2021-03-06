// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu  goya2 %s -o - | FileCheck %s
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
    half128 res = as_half128(income);
    half128 xx = as_half128(x);
    half128 __local *d_ptr = (half128 __local *)dest_ptr;
    
    res = v_f16_mov_dual_group_all_b(xx, 1, 0, 1, 2, 3, SW_WR_LOWER_GROUP0, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=1 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x1, [[PRED]]

    res = v_f16_mov_dual_group_all_b(xx, 7, 1, 2, 3, 0, SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0, res, pred, 0);
    *d_ptr++ = res;
    
// CHECK: mov_dg all sdg0=1 sdg1=2 sdg2=3 sdg3=0 weg0=3 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x7, [[PRED]]
    res = v_f16_mov_dual_group_all_b(xx, 15, 2, 3, 0, 1, SW_WR_UPPER_GROUP0, res, pred, 0);

    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=2 sdg1=3 sdg2=0 sdg3=1 weg0=2 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0xf, [[PRED]]

    res = v_f16_mov_dual_group_all_vb(xx, 1, 0, 1, 2, 3, SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=0 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x1, [[VPRED]]

    res = v_f16_mov_dual_group_all_vb(xx, 7, 1, 2, 3, 0, SW_WR_LOWER_GROUP1 | SW_WR_UPPER_GROUP1, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=1 sdg1=2 sdg2=3 sdg3=0 weg0=0 weg1=3 weg2=0 weg3=0 [[DEST]], [[SRC]], 0x7, [[VPRED]]

    res = v_f16_mov_dual_group_all_vb(xx, 15, 3, 0, 1, 2, SW_WR_UPPER_GROUP0 | SW_WR_UPPER_GROUP1, res, to_bool128(vpred), 0);
    *d_ptr++ = res;
// CHECK: mov_dg all sdg0=3 sdg1=0 sdg2=1 sdg3=2 weg0=2 weg1=2 weg2=0 weg3=0 [[DEST]], [[SRC]], 0xf, [[VPRED]]

    income = as_float64(res);
  }
}
