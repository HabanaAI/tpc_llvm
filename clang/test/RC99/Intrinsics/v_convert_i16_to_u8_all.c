// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(int dest, int src1, int vpredp, _Bool pred) {
  volatile uchar256 __local *dest_ptr = (uchar256 __local *)dest;
  short128 __local *src_ptr = (short128 __local *)src1;
  bool256 __local *vpred_ptr = (bool256 __local *)vpredp;

  short256 x      = {*src_ptr++, 0};
  bool256  vpred  = *vpred_ptr++;
  uchar256  income = *dest_ptr;

// CHECK-DAG: ld_l_v   [[DEST:%V[0-9]+]], %S0
// CHECK-DAG: ld_l_v   %V[[SRC:[0-9]+]], %S1
// CHECK-DAG: ld_l_v   [[VPRED:%VP[0-9]+]], %S2
// CHECK-DAG: mov{{.*}}	[[PRED:%SP[0-9]+]], %S3

  // v_convert_i16_to_u8_all_b
  {
    uchar256 res = income;

    res = v_convert_i16_to_u8_all_b(x, 0, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK: convert.i16 all_lanes target_type=uint8 rhne [[DEST]], %D[[SRC]], [[PRED]]

    res = v_convert_i16_to_u8_all_b(x, 0, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK: convert.i16 all_lanes target_type=uint8 rhne [[DEST]], %D[[SRC]], [[PRED]]
    res = v_convert_i16_to_u8_all_b(x, 0, res, pred, 1);
    *dest_ptr++ = res;
    // CHECK: convert.i16 all_lanes target_type=uint8 rhne [[DEST]], %D[[SRC]], ![[PRED]]

    res = v_convert_i16_to_u8_all_b(x, SW_RHNE, res, 1, 0);
    *dest_ptr++ = res;
    // CHECK: convert.i16 all_lanes target_type=uint8 rhne [[DEST]], %D[[SRC]]

    income = res;
  }

  // v_convert_i16_to_u8_all_vb
  {
    uchar256 res = income;

    res = v_convert_i16_to_u8_all_vb(x, 0, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert.i16 all_lanes target_type=uint8 rhne [[DEST]], %D[[SRC]], [[VPRED]]

    res = v_convert_i16_to_u8_all_vb(x, 0, res, vpred, 1);
    *dest_ptr++ = res;
// CHECK: convert.i16 all_lanes target_type=uint8 rhne [[DEST]], %D[[SRC]], ![[VPRED]]

    res = v_convert_i16_to_u8_all_vb(x, SW_RHNE, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert.i16 all_lanes target_type=uint8 rhne [[DEST]], %D[[SRC]], [[VPRED]]

    income = res;
  }
}


