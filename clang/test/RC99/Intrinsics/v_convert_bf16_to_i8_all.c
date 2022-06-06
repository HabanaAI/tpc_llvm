// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(int dest, int src1, int vpredp, _Bool pred) {
  volatile char256 __local *dest_ptr = (char256 __local *)dest;
  bfloat128 __local *src_ptr = (bfloat128 __local *)src1;
  bool256 __local *vpred_ptr = (bool256 __local *)vpredp;

  bfloat256 x      = {*src_ptr++, 0};
  bool256  vpred  = *vpred_ptr++;
  char256  income = *dest_ptr;

// CHECK-DAG: ld_l_v   [[DEST:%V[0-9]+]], %S0
// CHECK-DAG: ld_l_v   %V[[SRC:[0-9]+]], %S1
// CHECK-DAG: ld_l_v   [[VPRED:%VP[0-9]+]], %S2
// CHECK-DAG: mov{{.*}}	[[PRED:%SP[0-9]+]], %S3

  // v_convert_bf16_to_i8_all_b
  {
    char256 res = income;

    res = v_convert_bf16_to_i8_all_b(x, 0, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK: convert.bf16 all_lanes target_type=int8 rhne [[DEST]], %D[[SRC]], [[PRED]]

    res = v_convert_bf16_to_i8_all_b(x, 0, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK: convert.bf16 all_lanes target_type=int8 rhne [[DEST]], %D[[SRC]], [[PRED]]
    res = v_convert_bf16_to_i8_all_b(x, 0, res, pred, 1);
    *dest_ptr++ = res;
    // CHECK: convert.bf16 all_lanes target_type=int8 rhne [[DEST]], %D[[SRC]], ![[PRED]]

    res = v_convert_bf16_to_i8_all_b(x, SW_RHNE, res, 1, 0);
    *dest_ptr++ = res;
    // CHECK: convert.bf16 all_lanes target_type=int8 rhne [[DEST]], %D[[SRC]]

    res = v_convert_bf16_to_i8_all_b(x, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK: convert.bf16 all_lanes target_type=int8 rz [[DEST]], %D[[SRC]], [[PRED]]

    res = v_convert_bf16_to_i8_all_b(x, SW_RU, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK: convert.bf16 all_lanes target_type=int8 ru [[DEST]], %D[[SRC]], [[PRED]]
    
    res = v_convert_bf16_to_i8_all_b(x, SW_RD, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK: convert.bf16 all_lanes target_type=int8 rd [[DEST]], %D[[SRC]], [[PRED]]
    income = res;
  }

  // v_convert_bf16_to_i8_all_vb
  {
    char256 res = income;

    res = v_convert_bf16_to_i8_all_vb(x, 0, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert.bf16 all_lanes target_type=int8 rhne [[DEST]], %D[[SRC]], [[VPRED]]

    res = v_convert_bf16_to_i8_all_vb(x, 0, res, vpred, 1);
    *dest_ptr++ = res;
// CHECK: convert.bf16 all_lanes target_type=int8 rhne [[DEST]], %D[[SRC]], ![[VPRED]]

    res = v_convert_bf16_to_i8_all_vb(x, SW_RHNE, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert.bf16 all_lanes target_type=int8 rhne [[DEST]], %D[[SRC]], [[VPRED]]

    res = v_convert_bf16_to_i8_all_vb(x, SW_RZ, res, vpred, 0);
    *dest_ptr++ = res;
// CHECK: convert.bf16 all_lanes target_type=int8 rz [[DEST]], %D[[SRC]], [[VPRED]]

    res = v_convert_bf16_to_i8_all_vb(x, SW_RU, res, vpred, 0);
    *dest_ptr++ = res;
    // CHECK: convert.bf16 all_lanes target_type=int8 ru [[DEST]], %D[[SRC]], [[VPRED]]

    res = v_convert_bf16_to_i8_all_vb(x, SW_RD, res, vpred, 0);
    *dest_ptr++ = res;
    // CHECK: convert.bf16 all_lanes target_type=int8 rd [[DEST]], %D[[SRC]], [[VPRED]]

    income = res;
  }
}


