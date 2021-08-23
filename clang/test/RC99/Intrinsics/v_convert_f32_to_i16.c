// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -mllvm -tpc-hwwa-conv-maxint=0 %s -o - | FileCheck  %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 -mllvm -tpc-hwwa-conv-maxint=0  %s -o - | FileCheck --check-prefixes=CHECK,GEN23 %s


void main(int dest, int src1, int vpredp, _Bool pred) {
  volatile short128 __local *dest_ptr = (short128 __local *)dest;
  float64 __local *src_ptr = (float64 __local *)src1;
  bool256 __local *vpred_ptr = (bool256 __local *)vpredp;

  float64 x      = *src_ptr++;
  bool256  vpred  = *vpred_ptr++;
  short128  income = *dest_ptr;

// CHECK-DAG: ld_l_v   [[DEST:%V[0-9]+]], %S0
// CHECK-DAG: ld_l_v   [[SRC:%V[0-9]+]], %S1
// CHECK-DAG: ld_l_v   [[VPRED:%VP[0-9]+]], %S2
// CHECK-DAG: mov{{.*}}	[[PRED:%SP[0-9]+]], %S3

  // v_convert_f32_to_i16_b
  {
    short128 res = income;

    res = v_convert_f32_to_i16_b(x, 1, 0, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK: convert.f32 lane_sel=1 target_type=int16 rhne [[DEST]], [[SRC]], [[PRED]]

    res = v_convert_f32_to_i16_b(x, 1, 0, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK: convert.f32 lane_sel=1 target_type=int16 rhne [[DEST]], [[SRC]], [[PRED]]
    res = v_convert_f32_to_i16_b(x, 1, 0, res, pred, 1);
    *dest_ptr++ = res;
    // CHECK: convert.f32 lane_sel=1 target_type=int16 rhne [[DEST]], [[SRC]], ![[PRED]]

    res = v_convert_f32_to_i16_b(x, 1, SW_RHNE, res, 1, 0);
    *dest_ptr++ = res;
    // CHECK: convert.f32 lane_sel=1 target_type=int16 rhne [[DEST]], [[SRC]], %SP0

    res = v_convert_f32_to_i16_b(x, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK: convert.f32 lane_sel=1 target_type=int16 rz [[DEST]], [[SRC]], [[PRED]]


#if defined(__gaudi__)
    res = v_convert_f32_to_i16_b(x, 1, SW_RD, res, pred, 0);
    *dest_ptr++ = res;
    // GEN23: convert.f32 lane_sel=1 target_type=int16 rd [[DEST]], [[SRC]], [[PRED]]

    res = v_convert_f32_to_i16_b(x, 1, SW_RU, res, pred, 0);
    *dest_ptr++ = res;
    // GEN23: convert.f32 lane_sel=1 target_type=int16 ru [[DEST]], [[SRC]], [[PRED]]
#endif
    income = res;
  }

  // v_convert_f32_to_i16_vb
  {
    short128 res = income;

    res = v_convert_f32_to_i16_vb(x, 1, 0, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.f32 lane_sel=1 target_type=int16 rhne [[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_f32_to_i16_vb(x, 1, 0, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.f32 lane_sel=1 target_type=int16 rhne [[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_f32_to_i16_vb(x, 2, 0, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.f32 lane_sel=2 target_type=int16 rhne [[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_f32_to_i16_vb(x, 3, SW_RZ, res, to_bool128(vpred), 1);
    *dest_ptr++ = res;
// CHECK: convert.f32 lane_sel=3 target_type=int16 rz [[DEST]], [[SRC]], ![[VPRED]]

#if defined(__gaudi__)
    res = v_convert_f32_to_i16_vb(x, 1, SW_RD, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
    // GEN23: convert.f32 lane_sel=1 target_type=int16 rd [[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_f32_to_i16_vb(x, 1, SW_RU, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
    // GEN23: convert.f32 lane_sel=1 target_type=int16 ru [[DEST]], [[SRC]], [[VPRED]]
#endif

    income = res;
  }
}


