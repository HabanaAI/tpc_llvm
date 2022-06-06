// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck --check-prefixes=CHECK %s

void main(int dest, int src1, int vpredp, _Bool pred) {
  volatile float64 __local *dest_ptr = (float64 __local *)dest;
  int64 __local *src_ptr = (int64 __local *)src1;
  bool256 __local *vpred_ptr = (bool256 __local *)vpredp;

  int128 x      = {*src_ptr++, 0, 0, 0};
  bool256  vpred  = *vpred_ptr++;
  float128  income = {*dest_ptr, 0, 0, 0};

  // CHECK-DAG: ld_l_v   %V[[DEST:[0-9]+]], %S0
  // CHECK-DAG: ld_l_v   %V[[SRC:[0-9]+]], %S1
  // CHECK-DAG: ld_l_v   [[VPRED:%VP[0-9]+]], %S2
  // CHECK-DAG: mov{{.*}}	[[PRED:%SP[0-9]+]], %S3

  // v_convert_i32_to_f32_x2_b
  {
    float128 res = income;

    res = v_convert_i32_to_f32_x2_b(x, 0, res, pred, 0);
    *dest_ptr++ = res.v1;
// CHECK: convert.i32 x2 target_type=fp32 rhne %D[[DEST]], %D[[SRC]], [[PRED]]

    res = v_convert_i32_to_f32_x2_b(x, 0, res, pred, 0);
    *dest_ptr++ = res.v1;
// CHECK: convert.i32 x2 target_type=fp32 rhne %D[[DEST]], %D[[SRC]], [[PRED]]
    res = v_convert_i32_to_f32_x2_b(x, 0, res, pred, 1);
    *dest_ptr++ = res.v1;
// CHECK: convert.i32 x2 target_type=fp32 rhne %D[[DEST]], %D[[SRC]], ![[PRED]]

    res = v_convert_i32_to_f32_x2_b(x, SW_RHNE, res, 1, 0);
    *dest_ptr++ = res.v1;
// CHECK: convert.i32 x2 target_type=fp32 rhne %D[[DEST]], %D[[SRC]]
 income = res;
  }

  // v_convert_i32_to_f32_x2_vb
  {
    float128 res = income;

    res = v_convert_i32_to_f32_x2_vb(x, 0, res, to_bool128(vpred), 0);
    *dest_ptr++ = res.v1;
// CHECK: convert.i32 x2 target_type=fp32 rhne %D[[DEST]], %D[[SRC]], [[VPRED]]

    res = v_convert_i32_to_f32_x2_vb(x, SW_RHNE, res, to_bool128(vpred), 0);
    *dest_ptr++ = res.v1;
// CHECK: convert.i32 x2 target_type=fp32 rhne %D[[DEST]], %D[[SRC]], [[VPRED]]

    res = v_convert_i32_to_f32_x2_vb(x, 0, res, to_bool128(vpred), 1);
    *dest_ptr++ = res.v1;
// CHECK: convert.i32 x2 target_type=fp32 rhne %D[[DEST]], %D[[SRC]], ![[VPRED]]

    income = res;
  }
}


