// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 %s -o - | FileCheck --check-prefixes=CHECK,CHECK-G3 %s

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

#if defined(__goya2__) || defined(__gaudi2__)

  // v_i1_mov_u32_b
  {
    bool256 res = income;
    volatile uint64 __local *val_ptr = (uint64 __local *)dest_ptr;
    uint64 val = *val_ptr;
    dest_ptr++;

    res = v_i1_mov_u32_b(val, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK-G3: mov    [[DEST]], %V{{.*}}, [[PRED]]

    income = res;
  }

  // v_i1_mov_u32_vb
  {
    bool256 res = income;
    volatile uint64 __local *val_ptr = (uint64 __local *)dest_ptr;
    uint64 val = *val_ptr;
    dest_ptr++;

    res = v_i1_mov_u32_vb(val, 0, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK-G3: mov    [[DEST]], %V{{.*}}, [[VPRED]]

    income = res;
  }

  // v_u32_mov_i1_b
  {
    volatile uint64 __local *dptr = (uint64 __local *)dest_ptr;
    uint64 res = *dptr++;

    res = v_u32_mov_i1_b(income, 0, res, pred, 0);
    *dptr++ = res;
// CHECK-G3: mov    %V{{.*}}, %VP{{.*}}, [[PRED]]
  }

  // v_u32_mov_i1_vb
  {
    volatile uint64 __local *dptr = (uint64 __local *)dest_ptr;
    uint64 res = *dptr++;

    res = v_u32_mov_i1_vb(income, 0, res, vpred, 0);
    *dptr++ = res;
// CHECK-G3: mov    %V{{.*}}, %VP{{.*}}, [[VPRED]]
  }
#endif
}
