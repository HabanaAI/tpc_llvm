// RUN: %tpc_clang -c -O1 %s -S -march=gaudi2 -mllvm -enable-misched=0 -o - | FileCheck %s

void main(int dest, int src1, int vpredp, _Bool pred) {
  volatile float64 __local *dest_ptr = (float64 __local *)dest;
  int64 __local *src_ptr = (int64 __local *)src1;
  bool256 __local *vpred_ptr = (bool256 __local *)vpredp;

  int256 x      = {*src_ptr++, 0, 0, 0};
  bool256  vpred  = *vpred_ptr++;
  float256  income = {*dest_ptr, 0, 0, 0};

  // CHECK-DAG: ld_l_v   %V[[DEST:[0-9]+]], %S0
  // CHECK-DAG: ld_l_v   %V[[SRC:[0-9]+]], %S1
  // CHECK-DAG: ld_l_v   [[VPRED:%VP[0-9]+]], %S2
  // CHECK-DAG: mov{{.*}}	[[PRED:%SP[0-9]+]], %S3

  // v_convert_i32_to_f32_x4_b
  {
    float256 res = income;

    res = v_convert_i32_to_f32_x4_b(x, 0, res, pred, 0);
    *dest_ptr++ = res.v1;
// CHECK: convert.i32 target_type=fp32 rhne x4 %A[[DEST]], %A[[SRC]], [[PRED]]

    res = v_convert_i32_to_f32_x4_b(x, 0, res, pred, 0);
    *dest_ptr++ = res.v1;
// CHECK: convert.i32 target_type=fp32 rhne x4 %A[[DEST]], %A[[SRC]], [[PRED]]
    res = v_convert_i32_to_f32_x4_b(x, 0, res, pred, 1);
    *dest_ptr++ = res.v1;
// CHECK: convert.i32 target_type=fp32 rhne x4  %A[[DEST]], %A[[SRC]], ![[PRED]]

    res = v_convert_i32_to_f32_x4_b(x, SW_RHNE, res, 1, 0);
    *dest_ptr++ = res.v1;
// CHECK: convert.i32 target_type=fp32 rhne x4  %A[[DEST]], %A[[SRC]]
 income = res;
  }

  // v_convert_i32_to_f32_x4_vb
  {
    float256 res = income;

    res = v_convert_i32_to_f32_x4_vb(x, 0, res, vpred, 0);
    *dest_ptr++ = res.v1;
// CHECK: convert.i32 target_type=fp32 rhne x4  %A[[DEST]], %A[[SRC]], [[VPRED]]

    res = v_convert_i32_to_f32_x4_vb(x, SW_RHNE, res, vpred, 0);
    *dest_ptr++ = res.v1;
// CHECK: convert.i32 target_type=fp32 rhne x4 %A[[DEST]], %A[[SRC]], [[VPRED]]

    res = v_convert_i32_to_f32_x4_vb(x, 0, res, vpred, 1);
    *dest_ptr++ = res.v1;
// CHECK: convert.i32 target_type=fp32 rhne x4 %A[[DEST]], %A[[SRC]], ![[VPRED]]

    income = res;
  }
}


