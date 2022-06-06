// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(int dest, int src1, int vpredp, _Bool pred) {
  volatile half128 __local *dest_ptr = (half128 __local *)dest;
  float64 __local *src_ptr = (float64 __local *)src1;
  bool256 __local *vpred_ptr = (bool256 __local *)vpredp;

  float128 x      = {*src_ptr++, 0};
  bool256  vpred  = *vpred_ptr++;
  half128  income = *dest_ptr;

// CHECK-DAG: ld_l_v   [[DEST:%V[0-9]+]], %S0
// CHECK-DAG: ld_l_v   %V[[SRC:[0-9]+]], %S1
// CHECK-DAG: ld_l_v   [[VPRED:%VP[0-9]+]], %S2
// CHECK-DAG: mov{{.*}}	[[PRED:%SP[0-9]+]], %S3

  // v_convert_f32_to_f16_all_b
  {
    half128 res = income;

    res = v_convert_f32_to_f16_all_b(x, 0, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK: convert.f32 all_lanes target_type=f16 rhne [[DEST]], %D[[SRC]], [[PRED]]

    res = v_convert_f32_to_f16_all_b(x, 0, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK: convert.f32 all_lanes target_type=f16 rhne [[DEST]], %D[[SRC]], [[PRED]]
    res = v_convert_f32_to_f16_all_b(x, 0, res, pred, 1);
    *dest_ptr++ = res;
    // CHECK: convert.f32 all_lanes target_type=f16 rhne [[DEST]], %D[[SRC]], ![[PRED]]

    res = v_convert_f32_to_f16_all_b(x, SW_RHNE, res, 1, 0);
    *dest_ptr++ = res;
    // CHECK: convert.f32 all_lanes target_type=f16 rhne [[DEST]], %D[[SRC]]

    res = v_convert_f32_to_f16_all_b(x, SW_SR, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK: convert.f32 all_lanes target_type=f16 sr [[DEST]], %D[[SRC]], [[PRED]]

    res = v_convert_f32_to_f16_all_b(x, SW_RHAZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK: convert.f32 all_lanes target_type=f16 rhaz [[DEST]], %D[[SRC]], [[PRED]]

    income = res;
  }

  // v_convert_f32_to_f16_all_vb
  {
    half128 res = income;

    res = v_convert_f32_to_f16_all_vb(x, 0, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.f32 all_lanes target_type=f16 rhne [[DEST]], %D[[SRC]], [[VPRED]]

    res = v_convert_f32_to_f16_all_vb(x, 0, res, to_bool128(vpred), 1);
    *dest_ptr++ = res;
// CHECK: convert.f32 all_lanes target_type=f16 rhne [[DEST]], %D[[SRC]], ![[VPRED]]

    res = v_convert_f32_to_f16_all_vb(x, SW_RHNE, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.f32 all_lanes target_type=f16 rhne [[DEST]], %D[[SRC]], [[VPRED]]

    res = v_convert_f32_to_f16_all_vb(x, SW_SR, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.f32 all_lanes target_type=f16 sr [[DEST]], %D[[SRC]], [[VPRED]]

    res = v_convert_f32_to_f16_all_vb(x, SW_RHAZ, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
    // CHECK: convert.f32 all_lanes target_type=f16 rhaz [[DEST]], %D[[SRC]], [[VPRED]]

    income = res;
  }
}


