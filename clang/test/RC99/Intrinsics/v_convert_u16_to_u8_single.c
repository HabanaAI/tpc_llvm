// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(int dest, int src1, int src2, int vpredp, short sh1, _Bool pred) {
  volatile uchar256 __local *dest_ptr = (uchar256 __local *)dest;
  ushort128 __local *src_ptr = (ushort128 __local *)src1;
  short128 __local *shift_ptr = (short128 __local *)src2;
  bool128 __local *vpred_ptr = (bool128 __local *)vpredp;
  
  ushort128 x      = *src_ptr++;
  short128  shift  = *shift_ptr++;
  bool128   vpred  = *vpred_ptr++;
  uchar256  income = *dest_ptr;

// CHECK-DAG: ld_l_v   [[DEST:%V[0-9]+]], %S0
// CHECK-DAG: ld_l_v   [[SRC:%V[0-9]+]], %S1
// CHECK-DAG: ld_l_v   [[SHIFT:%V[0-9]+]], %S2
// CHECK-DAG: ld_l_v   [[VPRED:%VP[0-9]+]], %S3
// CHECK-DAG: mov{{.*}}	[[PRED:%SP[0-9]+]], %S5

  // v_convert_uint16_to_u8_single_b
  {
    uchar256 res = income;
    
    res = v_convert_uint16_to_u8_single_b(x, shift, 0, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_uint16 lane_sel=0 single_lane rhne [[DEST]], [[SRC]], [[SHIFT]], [[PRED]]

    res = v_convert_uint16_to_u8_single_b(x, 4, 0, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_uint16 lane_sel=0 single_lane rhne [[DEST]], [[SRC]], 0x4, [[PRED]]

    res = v_convert_uint16_to_u8_single_b(x, sh1, 0, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_uint16 lane_sel=0 single_lane rhne [[DEST]], [[SRC]], %S4, [[PRED]]
    
    res = v_convert_uint16_to_u8_single_b(x, shift, 0, SW_RHNE, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_uint16 lane_sel=0 single_lane rhne [[DEST]], [[SRC]], [[SHIFT]], [[PRED]]
    
    res = v_convert_uint16_to_u8_single_b(x, shift, 0, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_uint16 lane_sel=0 single_lane rz [[DEST]], [[SRC]], [[SHIFT]], [[PRED]]
    
    res = v_convert_uint16_to_u8_single_b(x, shift, 0, SW_RU, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_uint16 lane_sel=0 single_lane ru [[DEST]], [[SRC]], [[SHIFT]], [[PRED]]
    
    res = v_convert_uint16_to_u8_single_b(x, shift, 0, SW_RD, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_uint16 lane_sel=0 single_lane rd [[DEST]], [[SRC]], [[SHIFT]], [[PRED]]
    
    res = v_convert_uint16_to_u8_single_b(x, shift, 0, SW_SR, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_uint16 lane_sel=0 single_lane sr [[DEST]], [[SRC]], [[SHIFT]], [[PRED]]
    
    res = v_convert_uint16_to_u8_single_b(x, shift, 1, SW_RU, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_uint16 lane_sel=1 single_lane ru [[DEST]], [[SRC]], [[SHIFT]], [[PRED]]

    res = v_convert_uint16_to_u8_single_b(x, shift, 0, 0, res, pred, 1);
    *dest_ptr++ = res;
// CHECK: convert_uint16 lane_sel=0 single_lane rhne [[DEST]], [[SRC]], [[SHIFT]], ![[PRED]]

    res = v_convert_uint16_to_u8_single_b(x, shift, 1, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_uint16 lane_sel=1 single_lane rhne [[DEST]], [[SRC]], [[SHIFT]], [[PRED]]

    income = res;
  }

  // v_convert_uint16_to_u8_single_vb
  {
    uchar256 res = income;
    
    res = v_convert_uint16_to_u8_single_vb(x, shift, 0, 0, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_uint16 lane_sel=0 single_lane rhne [[DEST]], [[SRC]], [[SHIFT]], [[VPRED]]

    res = v_convert_uint16_to_u8_single_vb(x, 4, 0, 0, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_uint16 lane_sel=0 single_lane rhne [[DEST]], [[SRC]], 0x4, [[VPRED]]

    res = v_convert_uint16_to_u8_single_vb(x, sh1, 0, 0, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_uint16 lane_sel=0 single_lane rhne [[DEST]], [[SRC]], %S4, [[VPRED]]
    
    res = v_convert_uint16_to_u8_single_vb(x, shift, 0, SW_RHNE, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_uint16 lane_sel=0 single_lane rhne [[DEST]], [[SRC]], [[SHIFT]], [[VPRED]]
    
    res = v_convert_uint16_to_u8_single_vb(x, shift, 0, SW_RZ, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_uint16 lane_sel=0 single_lane rz [[DEST]], [[SRC]], [[SHIFT]], [[VPRED]]
    
    res = v_convert_uint16_to_u8_single_vb(x, shift, 0, SW_RU, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_uint16 lane_sel=0 single_lane ru [[DEST]], [[SRC]], [[SHIFT]], [[VPRED]]
    
    res = v_convert_uint16_to_u8_single_vb(x, shift, 0, SW_RD, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_uint16 lane_sel=0 single_lane rd [[DEST]], [[SRC]], [[SHIFT]], [[VPRED]]
    
    res = v_convert_uint16_to_u8_single_vb(x, shift, 0, SW_SR, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_uint16 lane_sel=0 single_lane sr [[DEST]], [[SRC]], [[SHIFT]], [[VPRED]]
    
    res = v_convert_uint16_to_u8_single_vb(x, shift, 1, SW_RU, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_uint16 lane_sel=1 single_lane ru [[DEST]], [[SRC]], [[SHIFT]], [[VPRED]]

    res = v_convert_uint16_to_u8_single_vb(x, shift, 0, 0, res, vpred, 1);
    *dest_ptr++ = res;
// CHECK: convert_uint16 lane_sel=0 single_lane rhne [[DEST]], [[SRC]], [[SHIFT]], ![[VPRED]]

    res = v_convert_uint16_to_u8_single_vb(x, shift, 1, 0, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_uint16 lane_sel=1 single_lane rhne [[DEST]], [[SRC]], [[SHIFT]], [[VPRED]]
  }
}


