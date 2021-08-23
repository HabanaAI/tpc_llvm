// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck %s


void main(int dest, int x, int vpredp, _Bool pred) {
  volatile bool256 __local *dest_ptr = (bool256 __local *)dest;
  bool256 __local *vpred_ptr = (bool256 __local *)vpredp;

  bool256  vpred  = *vpred_ptr++;
  bool256  income = *dest_ptr++;

// CHECK-DAG: ld_l_v   [[DEST:%VP[0-9]+]], %S0
// CHECK-DAG: ld_l_v   [[VPRED:%VP[0-9]+]], %S2
// CHECK-DAG: mov{{.*}}	[[PRED:%SP[0-9]+]], %S3

  // v_i1_mov_flavor_b
  {
    bool256 res = income;

    res = v_i1_mov_flavor_b(x, 0, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: mov 0x0 [[DEST]], %S1, [[PRED]]

    res = v_i1_mov_flavor_b(x, 1, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: mov 0x1 [[DEST]], %S1, [[PRED]]

    res = v_i1_mov_flavor_b(0x55, 6, 0, res, pred, 1);
    *dest_ptr++ = res;
// CHECK: mov 0x6 [[DEST]], 0x55, ![[PRED]]

    res = v_i1_mov_flavor_b(0xAA, 7, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: mov 0x7 [[DEST]], 0xaa, [[PRED]]

    income = res;
  }

  // v_i1_mov_flavor_vb
  {
    bool256 res = income;

    res = v_i1_mov_flavor_vb(x, 0, 0, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: mov 0x0 [[DEST]], %S1, [[VPRED]]

    res = v_i1_mov_flavor_vb(x, 1, 0, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: mov 0x1 [[DEST]], %S1, [[VPRED]]

    res = v_i1_mov_flavor_vb(0x55, 6, 0, res, vpred, 1);
    *dest_ptr++ = res;
// CHECK: mov 0x6 [[DEST]], 0x55, ![[VPRED]]

    res = v_i1_mov_flavor_vb(0xAA, 7, 0, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: mov 0x7 [[DEST]], 0xaa, [[VPRED]]

    income = res;
  }

  // v_i1_mov_b
  {
    bool256 res = income;
    bool256 val = *vpred_ptr++;
// CHECK: ld_l_v [[VAL:%VP[0-9]+]], {{%S[0-9]+}}    

    res = v_i1_mov_b(val, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: mov    [[DEST]], [[VAL]], [[PRED]]

    income = res;
  }

  // v_i1_mov_vb
  {
    bool256 res = income;
    bool256 val = *vpred_ptr++;
// CHECK: ld_l_v [[VAL:%VP[0-9]+]], {{%S[0-9]+}}    

    res = v_i1_mov_vb(val, 0, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: mov    [[DEST]], [[VAL]], [[VPRED]]

    income = res;
  }

  // v_i1_mov_i1_b
  {
    bool256 res = income;

    res = v_i1_mov_i1_b(pred, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: mov    [[DEST]], [[PRED]], [[PRED]]

    income = res;
  }

  // v_i1_mov_i1_vb
  {
    bool256 res = income;

    res = v_i1_mov_i1_vb(pred, 0, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: mov    [[DEST]], [[PRED]], [[VPRED]]

    income = res;
  }
}
