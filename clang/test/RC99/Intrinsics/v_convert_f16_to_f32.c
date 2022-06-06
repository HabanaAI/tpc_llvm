// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck --check-prefixes=GEN3P %s

void main(int dest, int src1, int vpredp, _Bool pred) {
  volatile float64 __local *dest_ptr = (float64 __local *)dest;
  half128 __local *src_ptr = (half128 __local *)src1;
  bool256 __local *vpred_ptr = (bool256 __local *)vpredp;

  half128 x      = *src_ptr++;
  bool256  vpred  = *vpred_ptr++;
  float64  income = *dest_ptr;

// GEN3P-DAG: ld_l_v   [[DEST:%V[0-9]+]], %S0
// GEN3P-DAG: ld_l_v   [[SRC:%V[0-9]+]], %S1
// GEN3P-DAG: ld_l_v   [[VPRED:%VP[0-9]+]], %S2
// GEN3P-DAG: mov{{.*}}	[[PRED:%SP[0-9]+]], %S3

  // v_convert_f16_to_f32_b
  {
    float64 res = income;

    res = v_convert_f16_to_f32_b(x, 0, res, pred, 0);
    *dest_ptr++ = res;
    // GEN3P: convert.f16 target_type=fp32 rhne [[DEST]], [[SRC]], [[PRED]]

    res = v_convert_f16_to_f32_b(x, 0, res, pred, 0);
    *dest_ptr++ = res;
// GEN3P: convert.f16 target_type=fp32 rhne [[DEST]], [[SRC]], [[PRED]]
    res = v_convert_f16_to_f32_b(x, 0, res, pred, 0);
    *dest_ptr++ = res;
// GEN3P: convert.f16 target_type=fp32 rhne [[DEST]], [[SRC]], [[PRED]]

    res = v_convert_f16_to_f32_b(x, SW_RHNE, res, pred, 0);
    *dest_ptr++ = res;
// GEN3P: convert.f16 target_type=fp32 rhne [[DEST]], [[SRC]], %SP1

    res = v_convert_f16_to_f32_b(x, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
// GEN3P: convert.f16 target_type=fp32 rz [[DEST]], [[SRC]], [[PRED]]
    res = v_convert_f16_to_f32_b(x, SW_SR, res, pred, 0);
    *dest_ptr++ = res;
// GEN3P: convert.f16 target_type=fp32 sr [[DEST]], [[SRC]], [[PRED]]


    res = v_convert_f16_to_f32_b(x, SW_RD, res, pred, 0);
    *dest_ptr++ = res;
// GEN3P: convert.f16 target_type=fp32 rd [[DEST]], [[SRC]], [[PRED]]

    res = v_convert_f16_to_f32_b(x, SW_RU, res, pred, 0);
    *dest_ptr++ = res;
// GEN3P: convert.f16 target_type=fp32 ru [[DEST]], [[SRC]], [[PRED]]

    income = res;
  }

  // v_convert_f16_to_f32_vb
  {
    float64 res = income;

    res = v_convert_f16_to_f32_vb(x, 0, res, to_bool64(vpred), 0);
    *dest_ptr++ = res;
// GEN3P: convert.f16 target_type=fp32 rhne [[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_f16_to_f32_vb(x, 0, res, to_bool64(vpred), 0);
    *dest_ptr++ = res;
// GEN3P: convert.f16 target_type=fp32 rhne [[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_f16_to_f32_vb(x, 0, res, to_bool64(vpred), 0);
    *dest_ptr++ = res;
// GEN3P: convert.f16 target_type=fp32 rhne [[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_f16_to_f32_vb(x, 0, res, to_bool64(vpred), 1);
    *dest_ptr++ = res;
// GEN3P: convert.f16 target_type=fp32 rhne [[DEST]], [[SRC]], ![[VPRED]]

    income = res;
  }
}


