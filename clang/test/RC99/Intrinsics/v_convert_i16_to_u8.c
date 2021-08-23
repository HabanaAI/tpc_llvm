// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  %s -o - | FileCheck  %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck  %s


void main(int dest, int src1, int vpredp, _Bool pred) {
  volatile uchar256 __local *dest_ptr = (uchar256 __local *)dest;
  short128 __local *src_ptr = (short128 __local *)src1;
  bool256 __local *vpred_ptr = (bool256 __local *)vpredp;

  short128 x      = *src_ptr++;
  bool256  vpred  = *vpred_ptr++;
  uchar256  income = *dest_ptr;

// CHECK-DAG: ld_l_v   [[DEST:%V[0-9]+]], %S0
// CHECK-DAG: ld_l_v   [[SRC:%V[0-9]+]], %S1
// CHECK-DAG: ld_l_v   [[VPRED:%VP[0-9]+]], %S2
// CHECK-DAG: mov{{.*}}	[[PRED:%SP[0-9]+]], %S3

  // v_convert_i16_to_u8_b
  {
    uchar256 res = income;

    res = v_convert_i16_to_u8_b(x, 1, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert.i16 lane_sel=1 target_type=uint8 rhne [[DEST]], [[SRC]], [[PRED]]

    res = v_convert_i16_to_u8_b(x, 2, 0, res, pred, 1);
    *dest_ptr++ = res;
// CHECK: convert.i16 lane_sel=2 target_type=uint8 rhne [[DEST]], [[SRC]], ![[PRED]]

    res = v_convert_i16_to_u8_b(x, 3, SW_RHNE, res, 1, 0);
    *dest_ptr++ = res;
// CHECK: convert.i16 lane_sel=3 target_type=uint8 rhne [[DEST]], [[SRC]], %SP0

    res = v_convert_i16_to_u8_b(x, 1, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert.i16 lane_sel=1 target_type=uint8 rhne [[DEST]], [[SRC]], [[PRED]]

    income = res;
  }

  // v_convert_i16_to_u8_vb
  {
    uchar256 res = income;

    res = v_convert_i16_to_u8_vb(x, 2, 0, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.i16 lane_sel=2 target_type=uint8 rhne [[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_i16_to_u8_vb(x, 3, 0, res, to_bool128(vpred), 1);
    *dest_ptr++ = res;
// CHECK: convert.i16 lane_sel=3 target_type=uint8 rhne [[DEST]], [[SRC]], ![[VPRED]]

    res = v_convert_i16_to_u8_vb(x, 1, 0, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.i16 lane_sel=1 target_type=uint8 rhne [[DEST]], [[SRC]], [[VPRED]]
    income = res;
  }
}


