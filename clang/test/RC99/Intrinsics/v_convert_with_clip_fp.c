// RUN: %tpc_clang -c -O1 %s -S -march=gaudi2 -mllvm -enable-misched=0 -o - | FileCheck %s
// RUN: %tpc_clang -c -O1 %s -o %t.2 -march=gaudi2
// lvm-objdump --mcpu=gaudi2 --tpc-for-assembler --triple tpc -d -j .text -no-show-raw-insn -no-leading-addr %t2.o | FileCheck --check-prefix=DIS %s

// RUN: %tpc_clang -c -O1 %s -S -march=doron1 -mllvm -enable-misched=0 -o - | FileCheck %s
// RUN: %tpc_clang -c -O1 %s -o %t.2 -march=doron1
// lvm-objdump --mcpu=doron1 --tpc-for-assembler --triple tpc -d -j .text -no-show-raw-insn -no-leading-addr %t2.o | FileCheck --check-prefix=DIS %s

void main(int dest, int src1, int vpredp, _Bool pred) {
  volatile float64 __local  *dest_ptr = (float64 __local *)dest;
  uchar256  __local * src_ptr = (uchar256 __local *)src1;
  bool256  __local * vpred_ptr = (bool256 __local *)vpredp;

  uchar256 x = *src_ptr++;
  bool256 vpred = *vpred_ptr++;
  float256 income = {*dest_ptr, 0, 0, 0};

  // CHECK-DAG: ld_l_v   %V[[DEST:[0-9]+]], %S0
  // CHECK-DAG: ld_l_v   [[SRC:%V[0-9]+]], %S1
  // CHECK-DAG: ld_l_v   [[VPRED:%VP[0-9]+]], %S2
  // CHECK-DAG: mov{{.*}}	[[PRED:%SP[0-9]+]], %S3

  // v_convert_u8_to_f32_all_b
  {
    float256 res = income;

    res = v_convert_u8_to_f32_all_b(x, SW_CLIP_FP|SW_RZ, res, pred, 0);
    *dest_ptr++ = res.v1;
    // CHECK: convert.u8 all_lanes target_type=fp32 rz clip_fp %A[[DEST]], [[SRC]], [[PRED]]

    res = v_convert_u8_to_f32_all_b(x, SW_CLIP_FP, res, pred, 1);
    *dest_ptr++ = res.v1;
    // CHECK: convert.u8 all_lanes target_type=fp32 rhne clip_fp %A[[DEST]], [[SRC]], ![[PRED]]

    res = v_convert_u8_to_f32_all_b(x, SW_CLIP_FP, res, 1, 0);
    *dest_ptr++ = res.v1;
    // CHECK: convert.u8 all_lanes target_type=fp32 rhne clip_fp %A[[DEST]], [[SRC]]

    res = v_convert_u8_to_f32_all_b(x, SW_RHNE|SW_CLIP_FP, res, pred, 0);
    *dest_ptr++ = res.v1;
    // CHECK: convert.u8 all_lanes target_type=fp32 rhne clip_fp %A[[DEST]], [[SRC]], [[PRED]]
    
    income = res;
  }

  // v_convert_u8_to_f32_all_vb
  {
    float256 res = income;

    res = v_convert_u8_to_f32_all_vb(x, SW_CLIP_FP|SW_RD, res, vpred, 0);
    *dest_ptr++ = res.v1;
    // CHECK: convert.u8 all_lanes target_type=fp32 rd clip_fp %A[[DEST]], [[SRC]], [[VPRED]]

    res = v_convert_u8_to_f32_all_vb(x,SW_CLIP_FP|SW_SR, res, vpred, 1);
    *dest_ptr++ = res.v1;
    // CHECK: convert.u8 all_lanes target_type=fp32 sr clip_fp %A[[DEST]], [[SRC]], ![[VPRED]]

    income = res;
  }
}
//DIS: convert.u8 all_lanes target_type=fp32 rhne clip_fp
//DIS: convert.u8 all_lanes target_type=fp32 rz clip_fp
//DIS: convert.u8 all_lanes target_type=fp32 rd clip_fp

