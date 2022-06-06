// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudib -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(int dest, int src1, int vpredp, _Bool pred) {
  volatile half128 __local *dest_ptr = (half128 __local *)dest;
  float64 __local *src_ptr = (float64 __local *)src1;
  bool256 __local *vpred_ptr = (bool256 __local *)vpredp;

  float64 x      = *src_ptr++;
  bool256  vpred  = *vpred_ptr++;
  half128  income = *dest_ptr;

// CHECK-DAG: ld_l_v   [[DEST:%V[0-9]+]], %S0
// CHECK-DAG: ld_l_v   [[SRC:%V[0-9]+]], %S1
// CHECK-DAG: ld_l_v   [[VPRED:%VP[0-9]+]], %S2
// CHECK-DAG: mov{{.*}}	[[PRED:%SP[0-9]+]], %S3

  // v_convert_f32_to_f16_b
  {
    half128 res = income;

    res = v_convert_f32_to_f16_b(x, 2, 0, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK: convert.f32 lane_sel=2 target_type=f16 rhne [[DEST]], [[SRC]], [[PRED]]

    res = v_convert_f32_to_f16_b(x, 1, 0, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK: convert.f32 lane_sel=1 target_type=f16 rhne [[DEST]], [[SRC]], [[PRED]]
    res = v_convert_f32_to_f16_b(x, 1, SW_RHAZ, res, pred, 1);
    *dest_ptr++ = res;
    // CHECK: convert.f32 lane_sel=1 target_type=f16 rhaz [[DEST]], [[SRC]], ![[PRED]]

    res = v_convert_f32_to_f16_b(x, 1, SW_RHNE, res, 1, 0);
    *dest_ptr++ = res;
    // CHECK: convert.f32 lane_sel=1 target_type=f16 rhne [[DEST]], [[SRC]]

    res = v_convert_f32_to_f16_b(x, 1, SW_SR, res, pred, 0);
    *dest_ptr++ = res;
    income = res;
    // CHECK: convert.f32 lane_sel=1 target_type=f16 sr [[DEST]], [[SRC]], [[PRED]]
  }

  // v_convert_f32_to_f16_vb
  {
    half128 res = income;

    res = v_convert_f32_to_f16_vb(x, 1, SW_RHAZ, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
    // CHECK: convert.f32 lane_sel=1 target_type=f16 rhaz [[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_f32_to_f16_vb(x, 1, 0, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
    // CHECK: convert.f32 lane_sel=1 target_type=f16 rhne [[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_f32_to_f16_vb(x, 3, 0, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
    // CHECK: convert.f32 lane_sel=3 target_type=f16 rhne [[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_f32_to_f16_vb(x, 2, SW_RZ, res, to_bool128(vpred), 1);
    *dest_ptr++ = res;
    // CHECK: convert.f32 lane_sel=2 target_type=f16 rz [[DEST]], [[SRC]], ![[VPRED]]

    income = res;
  }
}


