// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(int dest, int src1, int vpredp, _Bool pred) {
  volatile uint64 __local  *dest_ptr = (uint64 __local *)dest;
  char256  __local * src_ptr = (char256 __local *)src1;
  bool256  __local * vpred_ptr = (bool256 __local *)vpredp;

  char256 x = *src_ptr++;
  bool256 vpred = *vpred_ptr++;
  uint256 income = {*dest_ptr, 0, 0, 0};

  // CHECK-DAG: ld_l_v   %V[[DEST:[0-9]+]], %S0
  // CHECK-DAG: ld_l_v   [[SRC:%V[0-9]+]], %S1
  // CHECK-DAG: ld_l_v   [[VPRED:%VP[0-9]+]], %S2
  // CHECK-DAG: mov{{.*}}	[[PRED:%SP[0-9]+]], %S3

  // v_convert_i8_to_u32_all_b
  {
    uint256 res = income;

    res = v_convert_i8_to_u32_all_b(x, 0, res, pred, 0);
    *dest_ptr++ = res.v1;
    // CHECK: convert.i8 all_lanes target_type=uint32 rhne %A[[DEST]], [[SRC]], [[PRED]]

    res = v_convert_i8_to_u32_all_b(x, 0, res, pred, 1);
    *dest_ptr++ = res.v1;
    // CHECK: convert.i8 all_lanes target_type=uint32 rhne %A[[DEST]], [[SRC]], ![[PRED]]

    res = v_convert_i8_to_u32_all_b(x, 0, res, 1, 0);
    *dest_ptr++ = res.v1;
    // CHECK: convert.i8 all_lanes target_type=uint32 rhne %A[[DEST]], [[SRC]]

    res = v_convert_i8_to_u32_all_b(x, SW_RHNE, res, pred, 0);
    *dest_ptr++ = res.v1;
    // CHECK: convert.i8 all_lanes target_type=uint32 rhne %A[[DEST]], [[SRC]], [[PRED]]
    
    income = res;
  }

  // v_convert_i8_to_u32_all_vb
  {
    uint256 res = income;

    res = v_convert_i8_to_u32_all_vb(x, 0, res, vpred, 0);
    *dest_ptr++ = res.v1;
    // CHECK: convert.i8 all_lanes target_type=uint32 rhne %A[[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_i8_to_u32_all_vb(x, 0, res, vpred, 1);
    *dest_ptr++ = res.v1;
    // CHECK: convert.i8 all_lanes target_type=uint32 rhne %A[[DEST]], [[SRC]], ![[VPRED]]

    income = res;
  }
}


