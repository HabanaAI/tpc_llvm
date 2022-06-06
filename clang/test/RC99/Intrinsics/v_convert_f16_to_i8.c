// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck  %s

void main(int dest, int src1, int vpredp, _Bool pred) {
  volatile char256 __local *dest_ptr = (char256 __local *)dest;
  half128 __local *src_ptr = (half128 __local *)src1;
  bool256 __local *vpred_ptr = (bool256 __local *)vpredp;

  half128 x      = *src_ptr++;
  bool256  vpred  = *vpred_ptr++;
  char256  income = *dest_ptr;

// CHECK-DAG: ld_l_v   [[DEST:%V[0-9]+]], %S0
// CHECK-DAG: ld_l_v   [[SRC:%V[0-9]+]], %S1
// CHECK-DAG: ld_l_v   [[VPRED:%VP[0-9]+]], %S2
// CHECK-DAG: mov{{.*}}	[[PRED:%SP[0-9]+]], %S3

  // v_convert_f16_to_i8_b
  {
    char256 res = income;

    res = v_convert_f16_to_i8_b(x, 1, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert.f16 lane_sel=1 target_type=int8 rhne [[DEST]], [[SRC]], [[PRED]]

    res = v_convert_f16_to_i8_b(x, 2, 0, res, pred, 1);
    *dest_ptr++ = res;
// CHECK: convert.f16 lane_sel=2 target_type=int8 rhne [[DEST]], [[SRC]], ![[PRED]]

    res = v_convert_f16_to_i8_b(x, 3, SW_RHNE, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert.f16 lane_sel=3 target_type=int8 rhne [[DEST]], [[SRC]], [[PRED]]

    res = v_convert_f16_to_i8_b(x, 1, SW_RHNE, res, 1, 0);
    *dest_ptr++ = res;
// CHECK: convert.f16 lane_sel=1 target_type=int8 rhne [[DEST]], [[SRC]]

    res = v_convert_f16_to_i8_b(x, 2, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert.f16 lane_sel=2 target_type=int8 rz [[DEST]], [[SRC]], [[PRED]]


    res = v_convert_f16_to_i8_b(x, 3, SW_RD, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert.f16 lane_sel=3 target_type=int8 rd [[DEST]], [[SRC]], [[PRED]]

    res = v_convert_f16_to_i8_b(x, 1, SW_RU, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert.f16 lane_sel=1 target_type=int8 ru [[DEST]], [[SRC]], [[PRED]]

    res = v_convert_f16_to_i8_b(x, 1, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert.f16 lane_sel=1 target_type=int8 rhne [[DEST]], [[SRC]], [[PRED]]

    income = res;
  }

  // v_convert_f16_to_i8_vb
  {
    char256 res = income;

    res = v_convert_f16_to_i8_vb(x, 2, 0, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.f16 lane_sel=2 target_type=int8 rhne [[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_f16_to_i8_vb(x, 3, 0, res, to_bool128(vpred), 1);
    *dest_ptr++ = res;
// CHECK: convert.f16 lane_sel=3 target_type=int8 rhne [[DEST]], [[SRC]], ![[VPRED]]

    res = v_convert_f16_to_i8_vb(x, 1, SW_RHNE, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.f16 lane_sel=1 target_type=int8 rhne [[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_f16_to_i8_vb(x, 2, SW_RZ, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.f16 lane_sel=2 target_type=int8 rz [[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_f16_to_i8_vb(x, 1, SW_RU, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.f16 lane_sel=1 target_type=int8 ru [[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_f16_to_i8_vb(x, 2, SW_RD, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.f16 lane_sel=2 target_type=int8 rd [[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_f16_to_i8_vb(x, 1, SW_RHNE, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.f16 lane_sel=1 target_type=int8 rhne [[DEST]], [[SRC]], [[VPRED]]
    income = res;
  }
}


