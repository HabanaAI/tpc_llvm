// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck  %s


void main(int dest, int src1, int vpredp, _Bool pred) {
  volatile bfloat128 __local *dest_ptr = (bfloat128 __local *)dest;
  short128 __local *src_ptr = (short128 __local *)src1;
  bool256 __local *vpred_ptr = (bool256 __local *)vpredp;

  short128 x      = *src_ptr++;
  bool256  vpred  = *vpred_ptr++;
  bfloat128  income = *dest_ptr;

// CHECK-DAG: ld_l_v   [[DEST:%V[0-9]+]], %S0
// CHECK-DAG: ld_l_v   [[SRC:%V[0-9]+]], %S1
// CHECK-DAG: ld_l_v   [[VPRED:%VP[0-9]+]], %S2
// CHECK-DAG: mov{{.*}}	[[PRED:%SP[0-9]+]], %S3

  // v_convert_i16_to_bf16_b
  {
    bfloat128 res = income;

    res = v_convert_i16_to_bf16_b(x, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert.i16 target_type=bf16 rhne [[DEST]], [[SRC]], [[PRED]]

    res = v_convert_i16_to_bf16_b(x, 0, res, pred, 1);
    *dest_ptr++ = res;
// CHECK: convert.i16 target_type=bf16 rhne [[DEST]], [[SRC]], ![[PRED]]

    res = v_convert_i16_to_bf16_b(x, SW_RHNE, res, 1, 0);
    *dest_ptr++ = res;
// CHECK: convert.i16 target_type=bf16 rhne [[DEST]], [[SRC]], %SP0

    income = res;
  }

  // v_convert_i16_to_bf16_vb
  {
    bfloat128 res = income;

    res = v_convert_i16_to_bf16_vb(x, 0, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.i16 target_type=bf16 rhne [[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_i16_to_bf16_vb(x, 0, res, to_bool128(vpred), 1);
    *dest_ptr++ = res;
// CHECK: convert.i16 target_type=bf16 rhne [[DEST]], [[SRC]], ![[VPRED]]

    income = res;
  }
}


