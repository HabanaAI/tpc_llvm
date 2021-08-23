// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck --check-prefixes=CHECK %s


void main(int dest, int src1, int vpredp, _Bool pred) {
  volatile float64 __local *dest_ptr = (int64 __local *)dest;
  int64 __local *src_ptr = (int64 __local *)src1;
  bool256 __local *vpred_ptr = (bool256 __local *)vpredp;

  int64 x      = *src_ptr++;
  bool256  vpred  = *vpred_ptr++;
  float64  income = *dest_ptr;

// CHECK-DAG: ld_l_v   [[DEST:%V[0-9]+]], %S0
// CHECK-DAG: ld_l_v   [[SRC:%V[0-9]+]], %S1
// CHECK-DAG: ld_l_v   [[VPRED:%VP[0-9]+]], %S2
// CHECK-DAG: mov{{.*}}	[[PRED:%SP[0-9]+]], %S3

  // v_convert_i32_to_f32_b
  {
    float64 res = income;

    res = v_convert_i32_to_f32_b(x, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert.i32 target_type=fp32 rhne [[DEST]], [[SRC]], [[PRED]]

    res = v_convert_i32_to_f32_b(x, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert.i32 target_type=fp32 rhne [[DEST]], [[SRC]], [[PRED]]
    res = v_convert_i32_to_f32_b(x, 0, res, pred, 1);
    *dest_ptr++ = res;
// CHECK: convert.i32 target_type=fp32 rhne [[DEST]], [[SRC]], ![[PRED]]

    res = v_convert_i32_to_f32_b(x, SW_RHNE, res, 1, 0);
    *dest_ptr++ = res;
// CHECK: convert.i32 target_type=fp32 rhne [[DEST]], [[SRC]], %SP0
 income = res;
  }

  // v_convert_i32_to_f32_vb
  {
    float64 res = income;

    res = v_convert_i32_to_f32_vb(x, 0, res, to_bool64(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.i32 target_type=fp32 rhne [[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_i32_to_f32_vb(x, SW_RHNE, res, to_bool64(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.i32 target_type=fp32 rhne [[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_i32_to_f32_vb(x, 0, res, to_bool64(vpred), 1);
    *dest_ptr++ = res;
// CHECK: convert.i32 target_type=fp32 rhne [[DEST]], [[SRC]], ![[VPRED]]

    income = res;
  }
}


