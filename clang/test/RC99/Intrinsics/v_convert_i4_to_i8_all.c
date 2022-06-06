// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck --check-prefixes=GEN3P %s

void main(int dest, int src1, int vpredp, _Bool pred) {
  volatile char256 __local *dest_ptr = (char256 __local *)dest;
  nibble512 __local *src_ptr = (nibble512 __local *)src1;
  bool256 __local *vpred_ptr = (bool256 __local *)vpredp;

  nibble512 x      = *src_ptr++;
  bool256  vpred  = *vpred_ptr++;
  char512  income = {*dest_ptr, 0};

  // GEN3P-DAG: ld_l_v   %V[[DEST:[0-9]+]], %S0
  // GEN3P-DAG: ld_l_v   [[SRC:%V[0-9]+]], %S1
  // GEN3P-DAG: ld_l_v   [[VPRED:%VP[0-9]+]], %S2
  // GEN3P-DAG: mov{{.*}}	[[PRED:%SP[0-9]+]], %S3

  // v_convert_i4_to_i8_all_b
  {
    char512 res = income;

    res = v_convert_i4_to_i8_all_b(x, 0, res, pred, 0);
    *dest_ptr++ = res.v1;
    // GEN3P: convert.i4 all_lanes target_type=int8 rhne %D[[DEST]], [[SRC]], [[PRED]]


    res = v_convert_i4_to_i8_all_b(x, 0, res, pred, 0);
    *dest_ptr++ = res.v1;
    // GEN3P: convert.i4 all_lanes target_type=int8 rhne %D[[DEST]], [[SRC]], [[PRED]]


    res = v_convert_i4_to_i8_all_b(x, 0, res, pred, 0);
    *dest_ptr++ = res.v1;
    // GEN3P: convert.i4 all_lanes target_type=int8 rhne %D[[DEST]], [[SRC]], [[PRED]]


    res = v_convert_i4_to_i8_all_b(x, SW_RHNE, res, pred, 0);
    *dest_ptr++ = res.v1;
    // GEN3P: convert.i4 all_lanes target_type=int8 rhne %D[[DEST]], [[SRC]], %SP1


    res = v_convert_i4_to_i8_all_b(x, SW_RZ, res, pred, 0);
    *dest_ptr++ = res.v1;
    // GEN3P: convert.i4 all_lanes target_type=int8 rz %D[[DEST]], [[SRC]], [[PRED]]


    res = v_convert_i4_to_i8_all_b(x, SW_SR, res, pred, 0);
    *dest_ptr++ = res.v1;
    // GEN3P: convert.i4 all_lanes target_type=int8 sr %D[[DEST]], [[SRC]], [[PRED]]



    res = v_convert_i4_to_i8_all_b(x, SW_RD, res, pred, 0);
    *dest_ptr++ = res.v1;
    // GEN3P: convert.i4 all_lanes target_type=int8 rd %D[[DEST]], [[SRC]], [[PRED]]


    res = v_convert_i4_to_i8_all_b(x, SW_RU, res, pred, 0);
    *dest_ptr++ = res.v1;
    // GEN3P: convert.i4 all_lanes target_type=int8 ru %D[[DEST]], [[SRC]], [[PRED]]


    income = res;
  }

  // v_convert_i4_to_i8_all_vb
  {
    char512 res = income;

    res = v_convert_i4_to_i8_all_vb(x, 0, res, vpred, 0);
    *dest_ptr++ = res.v1;
    // GEN3P: convert.i4 all_lanes target_type=int8 rhne %D[[DEST]], [[SRC]], [[VPRED]]


    res = v_convert_i4_to_i8_all_vb(x, 0, res, vpred, 0);
    *dest_ptr++ = res.v1;
    // GEN3P: convert.i4 all_lanes target_type=int8 rhne %D[[DEST]], [[SRC]], [[VPRED]]


    res = v_convert_i4_to_i8_all_vb(x, 0, res, vpred, 0);
    *dest_ptr++ = res.v1;
    // GEN3P: convert.i4 all_lanes target_type=int8 rhne %D[[DEST]], [[SRC]], [[VPRED]]


    res = v_convert_i4_to_i8_all_vb(x, 0, res, vpred, 1);
    *dest_ptr++ = res.v1;
    // GEN3P: convert.i4 all_lanes target_type=int8 rhne %D[[DEST]], [[SRC]], ![[VPRED]]


    income = res;
  }
}


