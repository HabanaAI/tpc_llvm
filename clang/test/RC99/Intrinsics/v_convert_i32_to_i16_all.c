// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -mllvm -tpc-hwwa-goya2=0  -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 -mllvm -tpc-hwwa-goya2=0  -bfloat16 %s -o - | FileCheck %s
// UN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 -mllvm -tpc-hwwa-goya2=0  -bfloat16 %s -o - | FileCheck %s

void main(int dest, int src1, int src2, int vpredp, int sh1, _Bool pred) {
  volatile short128 __local *dest_ptr = (short128 __local *)dest;
  int128   __local *src_ptr = (int128 __local *)src1;
  short128 __local *shift_ptr = (short128 __local *)src2;
  bool128  __local *vpred_ptr = (bool128 __local *)vpredp;
  
  int128 x      = *src_ptr++;
  short128 shift   = *shift_ptr++;
  bool128  vpred  = *vpred_ptr++;
  short128 income = *dest_ptr;

// CHECK-DAG: ld_l_v   [[DEST:%V[0-9]+]], %S0
// CHECK-DAG: ld_l_v   %V[[SRC:[0-9]+]], %S1
// CHECK-DAG: ld_l_v   [[SHIFT:%V[0-9]+]], %S2
// CHECK-DAG: ld_l_v   [[VPRED:%VP[0-9]+]], %S3
// CHECK-DAG: mov{{.*}}	[[PRED:%SP[0-9]+]], %S5

  // v_convert_int32_to_int16_all_b
  {
    short128 res = income;
    
    res = v_convert_int32_to_i16_all_b(x, shift, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int32 all_lanes rhne to_16 [[DEST]], %D[[SRC]], [[SHIFT]], [[PRED]]

    res = v_convert_int32_to_i16_all_b(x, 4, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int32 all_lanes rhne to_16 [[DEST]], %D[[SRC]], 0x4, [[PRED]]

    res = v_convert_int32_to_i16_all_b(x, sh1, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int32 all_lanes rhne to_16 [[DEST]], %D[[SRC]], %S4, [[PRED]]
    
    res = v_convert_int32_to_i16_all_b(x, shift, SW_RHNE, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int32 all_lanes rhne to_16 [[DEST]], %D[[SRC]], [[SHIFT]], [[PRED]]
    
    res = v_convert_int32_to_i16_all_b(x, shift, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int32 all_lanes rz to_16 [[DEST]], %D[[SRC]], [[SHIFT]], [[PRED]]
    
    res = v_convert_int32_to_i16_all_b(x, shift, SW_RU, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int32 all_lanes ru to_16 [[DEST]], %D[[SRC]], [[SHIFT]], [[PRED]]
    
    res = v_convert_int32_to_i16_all_b(x, shift, SW_RD, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int32 all_lanes rd to_16 [[DEST]], %D[[SRC]], [[SHIFT]], [[PRED]]
    
    res = v_convert_int32_to_i16_all_b(x, shift, SW_SR, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int32 all_lanes sr to_16 [[DEST]], %D[[SRC]], [[SHIFT]], [[PRED]]
    
    res = v_convert_int32_to_i16_all_b(x, shift, SW_RU, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int32 all_lanes ru to_16 [[DEST]], %D[[SRC]], [[SHIFT]], [[PRED]]

    res = v_convert_int32_to_i16_all_b(x, shift, 0, res, pred, 1);
    *dest_ptr++ = res;
// CHECK: convert_int32 all_lanes rhne to_16 [[DEST]], %D[[SRC]], [[SHIFT]], ![[PRED]]

    res = v_convert_int32_to_i16_all_b(x, shift, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int32 all_lanes rhne to_16 [[DEST]], %D[[SRC]], [[SHIFT]], [[PRED]]

    income = res;
  }

  // v_convert_int32_to_i16_all_vb
  {
    short128 res = income;
    
    res = v_convert_int32_to_i16_all_vb(x, shift, 0, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int32 all_lanes rhne to_16 [[DEST]], %D[[SRC]], [[SHIFT]], [[VPRED]]

    res = v_convert_int32_to_i16_all_vb(x, 4, 0, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int32 all_lanes rhne to_16 [[DEST]], %D[[SRC]], 0x4, [[VPRED]]

    res = v_convert_int32_to_i16_all_vb(x, sh1, 0, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int32 all_lanes rhne to_16 [[DEST]], %D[[SRC]], %S4, [[VPRED]]
    
    res = v_convert_int32_to_i16_all_vb(x, shift, SW_RHNE, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int32 all_lanes rhne to_16 [[DEST]], %D[[SRC]], [[SHIFT]], [[VPRED]]
    
    res = v_convert_int32_to_i16_all_vb(x, shift, SW_RZ, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int32 all_lanes rz to_16 [[DEST]], %D[[SRC]], [[SHIFT]], [[VPRED]]
    
    res = v_convert_int32_to_i16_all_vb(x, shift, SW_RU, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int32 all_lanes ru to_16 [[DEST]], %D[[SRC]], [[SHIFT]], [[VPRED]]
    
    res = v_convert_int32_to_i16_all_vb(x, shift, SW_RD, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int32 all_lanes rd to_16 [[DEST]], %D[[SRC]], [[SHIFT]], [[VPRED]]
    
    res = v_convert_int32_to_i16_all_vb(x, shift, SW_SR, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int32 all_lanes sr to_16 [[DEST]], %D[[SRC]], [[SHIFT]], [[VPRED]]
    
    res = v_convert_int32_to_i16_all_vb(x, shift, SW_RU, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int32 all_lanes ru to_16 [[DEST]], %D[[SRC]], [[SHIFT]], [[VPRED]]

    res = v_convert_int32_to_i16_all_vb(x, shift, 0, res, vpred, 1);
    *dest_ptr++ = res;
// CHECK: convert_int32 all_lanes rhne to_16 [[DEST]], %D[[SRC]], [[SHIFT]], ![[VPRED]]

    res = v_convert_int32_to_i16_all_vb(x, shift, 0, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert_int32 all_lanes rhne to_16 [[DEST]], %D[[SRC]], [[SHIFT]], [[VPRED]]
  }
}


