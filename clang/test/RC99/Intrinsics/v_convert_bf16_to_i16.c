// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck  %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck  %s

void main(int dest, int src1, int vpredp, _Bool pred) {
  volatile short128 __local *dest_ptr = (short128 __local *)dest;
  bfloat128 __local *src_ptr = (bfloat128 __local *)src1;
  bool256 __local *vpred_ptr = (bool256 __local *)vpredp;

  bfloat128 x      = *src_ptr++;
  bool256  vpred  = *vpred_ptr++;
  short128  income = *dest_ptr;

// CHECK-DAG: ld_l_v   [[DEST:%V[0-9]+]], %S0
// CHECK-DAG: ld_l_v   [[SRC:%V[0-9]+]], %S1
// CHECK-DAG: ld_l_v   [[VPRED:%VP[0-9]+]], %S2
// CHECK-DAG: mov{{.*}}	[[PRED:%SP[0-9]+]], %S3

  // v_convert_bf16_to_i16_b
  {
    short128 res = income;

    res = v_convert_bf16_to_i16_b(x, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert.bf16 target_type=int16 rhne [[DEST]], [[SRC]], [[PRED]]

    res = v_convert_bf16_to_i16_b(x, 0, res, pred, 1);
    *dest_ptr++ = res;
// CHECK: convert.bf16 target_type=int16 rhne [[DEST]], [[SRC]], ![[PRED]]

    res = v_convert_bf16_to_i16_b(x, SW_RHNE, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert.bf16 target_type=int16 rhne [[DEST]], [[SRC]], [[PRED]]

    res = v_convert_bf16_to_i16_b(x, SW_RHNE, res, 1, 0);
    *dest_ptr++ = res;
// CHECK: convert.bf16 target_type=int16 rhne [[DEST]], [[SRC]]

    res = v_convert_bf16_to_i16_b(x, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert.bf16 target_type=int16 rz [[DEST]], [[SRC]], [[PRED]]


    res = v_convert_bf16_to_i16_b(x, SW_RD, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert.bf16 target_type=int16 rd [[DEST]], [[SRC]], [[PRED]]

    res = v_convert_bf16_to_i16_b(x, SW_RU, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert.bf16 target_type=int16 ru [[DEST]], [[SRC]], [[PRED]]

    income = res;
  }

  // v_convert_bf16_to_i16_vb
  {
    short128 res = income;

    res = v_convert_bf16_to_i16_vb(x, 0, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.bf16 target_type=int16 rhne [[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_bf16_to_i16_vb(x, 0, res, to_bool128(vpred), 1);
    *dest_ptr++ = res;
// CHECK: convert.bf16 target_type=int16 rhne [[DEST]], [[SRC]], ![[VPRED]]

    res = v_convert_bf16_to_i16_vb(x, SW_RHNE, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.bf16 target_type=int16 rhne [[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_bf16_to_i16_vb(x, SW_RZ, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.bf16 target_type=int16 rz [[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_bf16_to_i16_vb(x, SW_RU, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.bf16 target_type=int16 ru [[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_bf16_to_i16_vb(x, SW_RD, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.bf16 target_type=int16 rd [[DEST]], [[SRC]], [[VPRED]]

    income = res;
  }
}


