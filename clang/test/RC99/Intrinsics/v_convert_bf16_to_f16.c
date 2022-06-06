// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(int dest, int src1, int vpredp, _Bool pred) {
  volatile half128 __local *dest_ptr = (half128 __local *)dest;
  bfloat128 __local *src_ptr = (bfloat128 __local *)src1;
  bool256 __local *vpred_ptr = (bool256 __local *)vpredp;

  bfloat128 x      = *src_ptr++;
  bool256  vpred  = *vpred_ptr++;
  half128  income = *dest_ptr;

// CHECK-DAG: ld_l_v   [[DEST:%V[0-9]+]], %S0
// CHECK-DAG: ld_l_v   [[SRC:%V[0-9]+]], %S1
// CHECK-DAG: ld_l_v   [[VPRED:%VP[0-9]+]], %S2
// CHECK-DAG: mov{{.*}}	[[PRED:%SP[0-9]+]], %S3

  // v_convert_bf16_to_f16_b
  {
    half128 res = income;

    res = v_convert_bf16_to_f16_b(x, 0, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK: convert.bf16 target_type=f16 rhne [[DEST]], [[SRC]], [[PRED]]

    res = v_convert_bf16_to_f16_b(x, 0, res, pred, 1);
    *dest_ptr++ = res;
// CHECK: convert.bf16 target_type=f16 rhne [[DEST]], [[SRC]], ![[PRED]]

    res = v_convert_bf16_to_f16_b(x, SW_RHNE, res, 1, 0);
    *dest_ptr++ = res;
// CHECK: convert.bf16 target_type=f16 rhne [[DEST]], [[SRC]]

    res = v_convert_bf16_to_f16_b(x, SW_RHAZ, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert.bf16 target_type=f16 rhaz [[DEST]], [[SRC]], [[PRED]]

    res = v_convert_bf16_to_f16_b(x, SW_SR, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert.bf16 target_type=f16 sr [[DEST]], [[SRC]], [[PRED]]


    income = res;
  }

  // v_convert_bf16_to_f16_vb
  {
    half128 res = income;

    res = v_convert_bf16_to_f16_vb(x, 0, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.bf16 target_type=f16 rhne [[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_bf16_to_f16_vb(x, SW_RHNE, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.bf16 target_type=f16 rhne [[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_bf16_to_f16_vb(x, SW_RHAZ, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.bf16 target_type=f16 rhaz [[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_bf16_to_f16_vb(x, SW_SR, res, to_bool128(vpred), 1);
    *dest_ptr++ = res;
// CHECK: convert.bf16 target_type=f16 sr [[DEST]], [[SRC]], ![[VPRED]]

    income = res;
  }
}


