// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 -mllvm -tpc-hwwa-goya2=0  %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 -bfloat16 -mllvm -tpc-hwwa-goya2=0  %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 -bfloat16 -mllvm -tpc-hwwa-goya2=0  %s -o - 

void main(int dest, int src1, int src2, int vpredp, int sh1, _Bool pred) {
  volatile char256 __local *dest_ptr = (char256 __local *)dest;
  short256 __local *src_ptr = (short256 __local *)src1;
  char256 __local *shift_ptr = (char256 __local *)src2;
  bool256 __local *vpred_ptr = (bool256 __local *)vpredp;
  
  short256 x      = *src_ptr++;
  char256 shift   = *shift_ptr++;
  bool256  vpred  = *vpred_ptr++;
  char256  income = *dest_ptr;

// CHECK-DAG: ld_l_v   [[DEST:%V[0-9]+]], %S0
// CHECK-DAG: ld_l_v   %V[[SRC:[0-9]+]], %S1
// CHECK-DAG: ld_l_v   [[SHIFT:%V[0-9]+]], %S2
// CHECK-DAG: ld_l_v   [[VPRED:%VP[0-9]+]], %S3
// CHECK-DAG: mov{{.*}}	[[PRED:%SP[0-9]+]], %S5

  // v_convert_int16_to_i8_all_b
  {
    char256 res = income;
    
    res = v_convert_int16_to_i8_all_b(x, shift, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 all_lanes rhne [[DEST]], %D[[SRC]], [[SHIFT]], [[PRED]]

    res = v_convert_int16_to_i8_all_b(x, 4, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 all_lanes rhne [[DEST]], %D[[SRC]], 0x4, [[PRED]]

    res = v_convert_int16_to_i8_all_b(x, sh1, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 all_lanes rhne [[DEST]], %D[[SRC]], %S4, [[PRED]]
    
    res = v_convert_int16_to_i8_all_b(x, shift, SW_RHNE, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 all_lanes rhne [[DEST]], %D[[SRC]], [[SHIFT]], [[PRED]]
    
    res = v_convert_int16_to_i8_all_b(x, shift, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 all_lanes rz [[DEST]], %D[[SRC]], [[SHIFT]], [[PRED]]
    
    res = v_convert_int16_to_i8_all_b(x, shift, SW_RU, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 all_lanes ru [[DEST]], %D[[SRC]], [[SHIFT]], [[PRED]]
    
    res = v_convert_int16_to_i8_all_b(x, shift, SW_RD, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 all_lanes rd [[DEST]], %D[[SRC]], [[SHIFT]], [[PRED]]
    
    res = v_convert_int16_to_i8_all_b(x, shift, SW_SR, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 all_lanes sr [[DEST]], %D[[SRC]], [[SHIFT]], [[PRED]]
    
    res = v_convert_int16_to_i8_all_b(x, shift, SW_RU, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 all_lanes ru [[DEST]], %D[[SRC]], [[SHIFT]], [[PRED]]

    res = v_convert_int16_to_i8_all_b(x, shift, 0, res, pred, 1);
    *dest_ptr++ = res;
// CHECK: convert_int16 all_lanes rhne [[DEST]], %D[[SRC]], [[SHIFT]], ![[PRED]]

    res = v_convert_int16_to_i8_all_b(x, shift, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 all_lanes rhne [[DEST]], %D[[SRC]], [[SHIFT]], [[PRED]]

    income = res;
  }

  // v_convert_int16_to_i8_all_vb
  {
    char256 res = income;
    
    res = v_convert_int16_to_i8_all_vb(x, shift, 0, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 all_lanes rhne [[DEST]], %D[[SRC]], [[SHIFT]], [[VPRED]]

    res = v_convert_int16_to_i8_all_vb(x, 4, 0, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 all_lanes rhne [[DEST]], %D[[SRC]], 0x4, [[VPRED]]

    res = v_convert_int16_to_i8_all_vb(x, sh1, 0, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 all_lanes rhne [[DEST]], %D[[SRC]], %S4, [[VPRED]]
    
    res = v_convert_int16_to_i8_all_vb(x, shift, SW_RHNE, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 all_lanes rhne [[DEST]], %D[[SRC]], [[SHIFT]], [[VPRED]]
    
    res = v_convert_int16_to_i8_all_vb(x, shift, SW_RZ, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 all_lanes rz [[DEST]], %D[[SRC]], [[SHIFT]], [[VPRED]]
    
    res = v_convert_int16_to_i8_all_vb(x, shift, SW_RU, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 all_lanes ru [[DEST]], %D[[SRC]], [[SHIFT]], [[VPRED]]
    
    res = v_convert_int16_to_i8_all_vb(x, shift, SW_RD, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 all_lanes rd [[DEST]], %D[[SRC]], [[SHIFT]], [[VPRED]]
    
    res = v_convert_int16_to_i8_all_vb(x, shift, SW_SR, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 all_lanes sr [[DEST]], %D[[SRC]], [[SHIFT]], [[VPRED]]
    
    res = v_convert_int16_to_i8_all_vb(x, shift, SW_RU, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 all_lanes ru [[DEST]], %D[[SRC]], [[SHIFT]], [[VPRED]]

    res = v_convert_int16_to_i8_all_vb(x, shift, 0, res, vpred, 1);
    *dest_ptr++ = res;
// CHECK: convert_int16 all_lanes rhne [[DEST]], %D[[SRC]], [[SHIFT]], ![[VPRED]]

    res = v_convert_int16_to_i8_all_vb(x, shift, 0, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int16 all_lanes rhne [[DEST]], %D[[SRC]], [[SHIFT]], [[VPRED]]
  }
}


