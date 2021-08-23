// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s


void main(int dest, int x, int vpredp, _Bool pred) {
  volatile bool256 __local *dest_ptr = (bool256 __local *)dest;
  bool256 __local *vpred_ptr = (bool256 __local *)vpredp;

  bool256  vpred  = *vpred_ptr++;
  bool256  income = *dest_ptr++;
// CHECK-DAG: ld_l_v   [[DEST:%VP[0-9]+]], %S0
// CHECK-DAG: ld_l_v   [[VPRED:%VP[0-9]+]], %S2
// CHECK-DAG: mov{{.*}}	[[PRED:%SP[0-9]+]], %S3


  // bv_i32_mov_flavor_s_b
  {
    bool256 res = income;

    res = bv_i32_mov_flavor_s_b(0x55, res, 1, pred, 0);
    *dest_ptr++ = res;
// CHECK: mov 0x1 [[DEST]], 0x55, [[PRED]]

    res = bv_i32_mov_flavor_s_b(x, res, 2, pred, 1);
    *dest_ptr++ = res;
// CHECK: mov 0x2 [[DEST]], %S1, ![[PRED]]

    income = res;
  }

  // bv_i32_mov_flavor_s_vb
  {
    bool256 res = income;

    res = bv_i32_mov_flavor_s_vb(0x55, res, 1, vpred, 0);
    *dest_ptr++ = res;
// CHECK: mov 0x1 [[DEST]], 0x55, [[VPRED]]

    res = bv_i32_mov_flavor_s_vb(x, res, 2, vpred, 1);
    *dest_ptr++ = res;
// CHECK: mov 0x2 [[DEST]], %S1, ![[VPRED]]

    income = res;
  }

  // bv_i32_mov_flavor_s
  {
    bool256 res = income;

    res = bv_i32_mov_flavor_s(0x55, res, 1);
    *dest_ptr++ = res;
// CHECK: mov 0x1 [[DEST]], 0x55

    res = bv_i32_mov_flavor_s(x, res, 2);
    *dest_ptr++ = res;
// CHECK: mov 0x2 [[DEST]], %S1

    income = res;
  }

  // bv_u32_mov_flavor_s_b
  {
    bool256 res = income;

    res = bv_u32_mov_flavor_s_b(0x55, res, 1, pred, 0);
    *dest_ptr++ = res;
// CHECK: mov 0x1 [[DEST]], 0x55, [[PRED]]

    res = bv_u32_mov_flavor_s_b(x, res, 2, pred, 1);
    *dest_ptr++ = res;
// CHECK: mov 0x2 [[DEST]], %S1, ![[PRED]]

    income = res;
  }

  // bv_u32_mov_flavor_s_vb
  {
    bool256 res = income;

    res = bv_u32_mov_flavor_s_vb(0x55, res, 1, vpred, 0);
    *dest_ptr++ = res;
// CHECK: mov 0x1 [[DEST]], 0x55, [[VPRED]]

    res = bv_u32_mov_flavor_s_vb(x, res, 2, vpred, 1);
    *dest_ptr++ = res;
// CHECK: mov 0x2 [[DEST]], %S1, ![[VPRED]]

    income = res;
  }

  // bv_u32_mov_flavor_s
  {
    bool256 res = income;

    res = bv_u32_mov_flavor_s(0x55, res, 1);
    *dest_ptr++ = res;
// CHECK: mov 0x1 %VP{{[1-2]}}, 0x55

    res = bv_u32_mov_flavor_s(x, res, 2);
    *dest_ptr++ = res;
// CHECK: mov 0x2 %VP{{[1-2]}}, %S1

    income = res;
  }
}
