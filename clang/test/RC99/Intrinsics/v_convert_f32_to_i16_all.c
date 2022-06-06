// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(int dest, int src1, int vpredp, _Bool pred) {
  volatile short128 __local *dest_ptr = (short128 __local *)dest;
  float64 __local *src_ptr = (float64 __local *)src1;
  bool256 __local *vpred_ptr = (bool256 __local *)vpredp;

  float128 x      = {*src_ptr++, 0};
  bool256  vpred  = *vpred_ptr++;
  short128  income = *dest_ptr;

// CHECK-DAG: ld_l_v   [[DEST:%V[0-9]+]], %S0
// CHECK-DAG: ld_l_v   %V[[SRC:[0-9]+]], %S1
// CHECK-DAG: ld_l_v   [[VPRED:%VP[0-9]+]], %S2
// CHECK-DAG: mov{{.*}}	[[PRED:%SP[0-9]+]], %S3

  // v_convert_f32_to_i16_all_b
  {
    short128 res = income;

    res = v_convert_f32_to_i16_all_b(x, 0, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK: convert.f32 all_lanes target_type=int16 rhne [[DEST]], %D[[SRC]], [[PRED]]

    res = v_convert_f32_to_i16_all_b(x, 0, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK: convert.f32 all_lanes target_type=int16 rhne [[DEST]], %D[[SRC]], [[PRED]]
    res = v_convert_f32_to_i16_all_b(x, 0, res, pred, 1);
    *dest_ptr++ = res;
    // CHECK: convert.f32 all_lanes target_type=int16 rhne [[DEST]], %D[[SRC]], ![[PRED]]

    res = v_convert_f32_to_i16_all_b(x, SW_RHNE, res, 1, 0);
    *dest_ptr++ = res;
    // CHECK: convert.f32 all_lanes target_type=int16 rhne [[DEST]], %D[[SRC]]

    res = v_convert_f32_to_i16_all_b(x, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK: convert.f32 all_lanes target_type=int16 rz [[DEST]], %D[[SRC]], [[PRED]]

    res = v_convert_f32_to_i16_all_b(x, SW_RD, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK: convert.f32 all_lanes target_type=int16 rd [[DEST]], %D[[SRC]], [[PRED]]

    res = v_convert_f32_to_i16_all_b(x, SW_RU, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK: convert.f32 all_lanes target_type=int16 ru [[DEST]], %D[[SRC]], [[PRED]]
    income = res;
  }

  // v_convert_f32_to_i16_all_vb
  {
    short128 res = income;

    res = v_convert_f32_to_i16_all_vb(x, 0, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.f32 all_lanes target_type=int16 rhne [[DEST]], %D[[SRC]], [[VPRED]]

    res = v_convert_f32_to_i16_all_vb(x, 0, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.f32 all_lanes target_type=int16 rhne [[DEST]], %D[[SRC]], [[VPRED]]

    res = v_convert_f32_to_i16_all_vb(x, 0, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.f32 all_lanes target_type=int16 rhne [[DEST]], %D[[SRC]], [[VPRED]]

    res = v_convert_f32_to_i16_all_vb(x, SW_RZ, res, to_bool128(vpred), 1);
    *dest_ptr++ = res;
// CHECK: convert.f32 all_lanes target_type=int16 rz [[DEST]], %D[[SRC]], ![[VPRED]]

    res = v_convert_f32_to_i16_all_vb(x, SW_RD, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
    // CHECK: convert.f32 all_lanes target_type=int16 rd [[DEST]], %D[[SRC]], [[VPRED]]

    res = v_convert_f32_to_i16_all_vb(x, SW_RU, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
    // CHECK: convert.f32 all_lanes target_type=int16 ru [[DEST]], %D[[SRC]], [[VPRED]]


    income = res;
  }
}


