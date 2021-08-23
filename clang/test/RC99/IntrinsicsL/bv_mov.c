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

  // bv_mov_bv_vb
  {
    bool256 res = income;
    bool256 val = *vpred_ptr++;
// CHECK-DAG: ld_l_v   [[VAL:%VP[0-9]+]], %S{{[0-9]+}}

    res = bv_mov_bv_vb(val, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: mov [[DEST]], [[VAL]], [[VPRED]]

    income = res;
  }

  // bv_mov_bv_b
  {
    bool256 res = income;
    bool256 val = *vpred_ptr++;
// CHECK-DAG: ld_l_v   [[VAL:%VP[0-9]+]], %S{{[0-9]+}}

    res = bv_mov_bv_b(val, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: mov [[DEST]], [[VAL]], [[PRED]]

    income = res;
  }

  // bv_mov_b_vb
  {
    bool256 res = income;

    res = bv_mov_b_vb(pred, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: mov [[DEST]], [[PRED]], [[VPRED]]

    income = res;
  }

  // bv_mov_b_b
  {
    bool256 res = income;

    res = bv_mov_b_b(pred, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: mov [[DEST]], [[PRED]], [[PRED]]

    income = res;
  }
}
