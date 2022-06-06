// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck --check-prefixes=CHECK,CHECK-BF16 %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck --check-prefixes=CHECK,CHECK-BF16 %s

void main(int dest, int src1, int vpredp, _Bool pred) {
  volatile int64 __local *dest_ptr = (int64 __local *)dest;
  float64 __local *src_ptr = (float64 __local *)src1;
  bool256 __local *vpred_ptr = (bool256 __local *)vpredp;

  float64 x      = *src_ptr++;
  bool256  vpred  = *vpred_ptr++;
  int64  income = *dest_ptr;

// CHECK-DAG: ld_l_v   [[DEST:%V[0-9]+]], %S0
// CHECK-DAG: ld_l_v   [[SRC:%V[0-9]+]], %S1
// CHECK-DAG: ld_l_v   [[VPRED:%VP[0-9]+]], %S2
// CHECK-DAG: mov{{.*}}	[[PRED:%SP[0-9]+]], %S3

  // v_convert_f32_to_i32_b
  {
    int64 res = income;

    res = v_convert_f32_to_i32_b(x, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert.f32 target_type=int32 rhne [[DEST]], [[SRC]], [[PRED]]

    res = v_convert_f32_to_i32_b(x, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert.f32 target_type=int32 rhne [[DEST]], [[SRC]], [[PRED]]
    res = v_convert_f32_to_i32_b(x, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert.f32 target_type=int32 rhne [[DEST]], [[SRC]], [[PRED]]

    res = v_convert_f32_to_i32_b(x, SW_RHNE, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert.f32 target_type=int32 rhne [[DEST]], [[SRC]], %SP1

#if defined(__gaudi__) || defined(__goya2__)
    res = v_convert_f32_to_i32_b(x, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
// CHECK-BF16: convert.f32 target_type=int32 rz [[DEST]], [[SRC]], [[PRED]]
    res = v_convert_f32_to_i32_b(x, SW_SR, res, pred, 0);
    *dest_ptr++ = res;
// CHECK-BF16: convert.f32 target_type=int32 sr [[DEST]], [[SRC]], [[PRED]]
#endif

    res = v_convert_f32_to_i32_b(x, SW_RD, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert.f32 target_type=int32 rd [[DEST]], [[SRC]], [[PRED]]

    res = v_convert_f32_to_i32_b(x, SW_RU, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: convert.f32 target_type=int32 ru [[DEST]], [[SRC]], [[PRED]]

    income = res;
  }

  // v_convert_f32_to_i32_vb
  {
    int64 res = income;

    res = v_convert_f32_to_i32_vb(x, 0, res, to_bool64(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.f32 target_type=int32 rhne [[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_f32_to_i32_vb(x, 0, res, to_bool64(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.f32 target_type=int32 rhne [[DEST]], [[SRC]], [[VPRED]]
    res = v_convert_f32_to_i32_vb(x, 0, res, to_bool64(vpred), 0);
    *dest_ptr++ = res;
// CHECK: convert.f32 target_type=int32 rhne [[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_f32_to_i32_vb(x, 0, res, to_bool64(vpred), 1);
    *dest_ptr++ = res;
// CHECK: convert.f32 target_type=int32 rhne [[DEST]], [[SRC]], ![[VPRED]]

    income = res;
  }
}


