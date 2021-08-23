// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck --check-prefixes=CHECK,GEN2P %s

void main(int dest, int src1, int src2, int vpredp, char sh1, _Bool pred) {
  volatile char256 __local *dest_ptr = (char256 __local *)dest;
  short128 __local *src_ptr = (short128 __local *)src1;
  char256 __local *shift_ptr = (char256 __local *)src2;
  bool256 __local *vpred_ptr = (bool256 __local *)vpredp;
  
  short128 x      = *src_ptr++;
  char256  shift  = *shift_ptr++;
  bool256  vpred  = *vpred_ptr++;
  char256  income = *dest_ptr;

// CHECK-DAG: ld_l_v   [[DEST:%V[0-9]+]], %S0
// CHECK-DAG: ld_l_v   [[SRC:%V[0-9]+]], %S1
// CHECK-DAG: ld_l_v   [[SHIFT:%V[0-9]+]], %S2
// CHECK-DAG: ld_l_v   [[VPRED:%VP[0-9]+]], %S3
// CHECK-DAG: mov{{.*}}	[[PRED:%SP[0-9]+]], %S5

  // v_convert_int16_to_i8_b
  {
    char256 res = income;
    
    res = v_convert_int16_to_i8_b(x, shift, 0, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 lane_sel=0 rhne [[DEST]], [[SRC]], [[SHIFT]], [[PRED]]

    res = v_convert_int16_to_i8_b(x, 4, 0, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 lane_sel=0 rhne [[DEST]], [[SRC]], 0x4, [[PRED]]

    res = v_convert_int16_to_i8_b(x, sh1, 0, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 lane_sel=0 rhne [[DEST]], [[SRC]], %S4, [[PRED]]

    res = v_convert_int16_to_i8_b(x, sh1, 0, SW_RHNE, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 lane_sel=0 rhne [[DEST]], [[SRC]], %S4, %SP1

#if defined(__gaudi__)
    res = v_convert_int16_to_i8_b(x, sh1, 0, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
// GEN2P: convert_int16 lane_sel=0 rz [[DEST]], [[SRC]], %S4, [[PRED]]
#endif

    res = v_convert_int16_to_i8_b(x, sh1, 0, SW_RD, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 lane_sel=0 rd [[DEST]], [[SRC]], %S4, [[PRED]]

    res = v_convert_int16_to_i8_b(x, sh1, 0, SW_RU, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 lane_sel=0 ru [[DEST]], [[SRC]], %S4, [[PRED]]

    res = v_convert_int16_to_i8_b(x, sh1, 0, SW_SR, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 lane_sel=0 sr [[DEST]], [[SRC]], %S4, [[PRED]]

    res = v_convert_int16_to_i8_b(x, sh1, 1, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 lane_sel=1 rhne [[DEST]], [[SRC]], %S4, [[PRED]]

    res = v_convert_int16_to_i8_b(x, sh1, 1, SW_RHNE, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 lane_sel=1 rhne [[DEST]], [[SRC]], %S4, [[PRED]]

    res = v_convert_int16_to_i8_b(x, sh1, 1, SW_RD, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 lane_sel=1 rd [[DEST]], [[SRC]], %S4, [[PRED]]

    res = v_convert_int16_to_i8_b(x, sh1, 1, SW_RU, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 lane_sel=1 ru [[DEST]], [[SRC]], %S4, [[PRED]]

    res = v_convert_int16_to_i8_b(x, sh1, 1, SW_SR, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 lane_sel=1 sr [[DEST]], [[SRC]], %S4, [[PRED]]

    income = res;
  }

  // v_convert_int16_to_i8_vb
  {
    char256 res = income;
    
    res = v_convert_int16_to_i8_vb(x, shift, 0, 0, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 lane_sel=0 rhne [[DEST]], [[SRC]], [[SHIFT]], [[VPRED]]

    res = v_convert_int16_to_i8_vb(x, 4, 0, 0, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 lane_sel=0 rhne [[DEST]], [[SRC]], 0x4, [[VPRED]]

    res = v_convert_int16_to_i8_vb(x, sh1, 0, 0, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 lane_sel=0 rhne [[DEST]], [[SRC]], %S4, [[VPRED]]

    res = v_convert_int16_to_i8_vb(x, shift, 0, 0, res, to_bool128(vpred), 1);
    *dest_ptr++ = res;
// CHECK: convert_int16 lane_sel=0 rhne [[DEST]], [[SRC]], [[SHIFT]], ![[VPRED]]

    res = v_convert_int16_to_i8_vb(x, shift, 1, 0, res, to_bool128(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 lane_sel=1 rhne [[DEST]], [[SRC]], [[SHIFT]], [[VPRED]]

    income = res;
  }
}


