// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(int dest, int src1, int src2, int vpredp, char sh1, _Bool pred) {
  volatile unibble512 __local *dest_ptr = (unibble512 __local *)dest;
  uchar512 __local *src_ptr = (uchar512 __local *)src1;
  char256  __local *shift_ptr = (char256 __local *)src2;
  bool256  __local *vpred_ptr = (bool256 __local *)vpredp;
  
  uchar512   x      = *src_ptr++;
  char256    shift  = *shift_ptr++;
  bool256    vpred  = *vpred_ptr++;
  unibble512 income = *dest_ptr;

// CHECK-DAG: ld_l_v   [[DEST:%V[0-9]+]], %S0
// CHECK-DAG: ld_l_v   %V[[SRC:[0-9]+]], %S1
// CHECK-DAG: ld_l_v   [[SHIFT:%V[0-9]+]], %S2
// CHECK-DAG: ld_l_v   [[VPRED:%VP[0-9]+]], %S3
// CHECK-DAG: mov{{.*}}	[[PRED:%SP[0-9]+]], %S5

  // v_convert_uint8_to_u4_all_b
  {
    unibble512 res = income;
    
    res = v_convert_uint8_to_u4_all_b(x, shift, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_uint8 rhne [[DEST]], %D[[SRC]], [[SHIFT]], [[PRED]]

    res = v_convert_uint8_to_u4_all_b(x, 4, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_uint8 rhne [[DEST]], %D[[SRC]], 0x4, [[PRED]]

    res = v_convert_uint8_to_u4_all_b(x, sh1, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_uint8 rhne [[DEST]], %D[[SRC]], %S4, [[PRED]]

    res = v_convert_uint8_to_u4_all_b(x, sh1, SW_RHNE, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_uint8 rhne [[DEST]], %D[[SRC]], %S4, %SP1

    res = v_convert_uint8_to_u4_all_b(x, sh1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
// GEN2P: convert_uint8 rz [[DEST]], %D[[SRC]], %S4, [[PRED]]

    res = v_convert_uint8_to_u4_all_b(x, sh1, SW_RU, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_uint8 ru [[DEST]], %D[[SRC]], %S4, [[PRED]]

    res = v_convert_uint8_to_u4_all_b(x, sh1, SW_RD, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_uint8 rd [[DEST]], %D[[SRC]], %S4, [[PRED]]

    res = v_convert_uint8_to_u4_all_b(x, sh1, SW_SR, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_uint8 sr [[DEST]], %D[[SRC]], %S4, [[PRED]]

    income = res;
  }

  // v_convert_uint8_to_u4_all_vb
  {
    unibble512 res = income;
    
    res = v_convert_uint8_to_u4_all_vb(x, shift, 0, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_uint8 rhne [[DEST]], %D[[SRC]], [[SHIFT]], [[VPRED]]

    res = v_convert_uint8_to_u4_all_vb(x, 4, 0, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_uint8 rhne [[DEST]], %D[[SRC]], 0x4, [[VPRED]]

    res = v_convert_uint8_to_u4_all_vb(x, sh1, 0, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_uint8 rhne [[DEST]], %D[[SRC]], %S4, [[VPRED]]

    res = v_convert_uint8_to_u4_all_vb(x, shift, 0, res, vpred, 1);
    *dest_ptr++ = res;
// CHECK: convert_uint8 rhne [[DEST]], %D[[SRC]], [[SHIFT]], ![[VPRED]]

    income = res;
  }
}


