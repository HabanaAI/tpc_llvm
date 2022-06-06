// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck --check-prefixes=CHECK,GEN2P %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck --check-prefixes=CHECK,GEN3P %s


// GEN2P check for VP was removed due to found hw issue GAUDI-1732

void main(int dest, int src1, int vpredp, _Bool pred) {
  volatile float64 __local  *dest_ptr = (float64 __local *)dest;
  bfloat128  __local * src_ptr = (bfloat128 __local *)src1;
  bool256  __local * vpred_ptr = (bool256 __local *)vpredp;

  bfloat128 x = *src_ptr++;
  bool256 vpred = *vpred_ptr++;
  float128 income = {*dest_ptr, 0};

  // CHECK-DAG: ld_l_v   %V[[DEST:[0-9]+]], %S0
  // CHECK-DAG: ld_l_v   [[SRC:%V[0-9]+]], %S1
  // CHECK-DAG: ld_l_v   [[VPRED:%VP[0-9]+]], %S2
  // CHECK-DAG: mov{{.*}}	[[PRED:%SP[0-9]+]], %S3

  // v_convert_bf16_to_f32_all_b
  {
    float128 res = income;

    res = v_convert_bf16_to_f32_all_b(x, 0, res, pred, 0);
    *dest_ptr++ = res.v1;
    // GEN2P: convert.bf16 all_lanes target_type=fp32 rhne %D[[DEST]], [[SRC]], [[PRED]]
    // GEN3P: convert.bf16 all_lanes target_type=fp32 rhne %D[[DEST]], [[SRC]], [[PRED]]

    res = v_convert_bf16_to_f32_all_b(x, 0, res, pred, 1);
    *dest_ptr++ = res.v1;
    // GEN2P: convert.bf16 all_lanes target_type=fp32 rhne %D[[DEST]], [[SRC]], ![[PRED]]
    // GEN3P: convert.bf16 all_lanes target_type=fp32 rhne %D[[DEST]], [[SRC]], ![[PRED]]

    res = v_convert_bf16_to_f32_all_b(x, 0, res, 1, 0);
    *dest_ptr++ = res.v1;
    // GEN2P: convert.bf16 all_lanes target_type=fp32 rhne %D[[DEST]], [[SRC]]
    // GEN3P: convert.bf16 all_lanes target_type=fp32 rhne %D[[DEST]], [[SRC]]

    res = v_convert_bf16_to_f32_all_b(x, SW_RHNE, res, pred, 0);
    *dest_ptr++ = res.v1;
    // GEN2P: convert.bf16 all_lanes target_type=fp32 rhne %D[[DEST]], [[SRC]], [[PRED]]
    // GEN3P: convert.bf16 all_lanes target_type=fp32 rhne %D[[DEST]], [[SRC]], [[PRED]]
    
    income = res;
  }

  // v_convert_bf16_to_f32_all_vb
  {
    float128 res = income;

    res = v_convert_bf16_to_f32_all_vb(x, 0, res, to_bool128(vpred), 0);
    *dest_ptr++ = res.v1;
    // GEN3P: convert.bf16 all_lanes target_type=fp32 rhne %D[[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_bf16_to_f32_all_vb(x, 0, res, to_bool128(vpred), 1);
    *dest_ptr++ = res.v1;
    // GEN3P: convert.bf16 all_lanes target_type=fp32 rhne %D[[DEST]], [[SRC]], ![[VPRED]]

    income = res;
  }
}


