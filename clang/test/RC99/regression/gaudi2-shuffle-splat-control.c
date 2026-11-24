// RUN: %tpc_clang -march=gaudi2 -O2 -S %s -o - | FileCheck %s

// A shuffle whose control is one byte for every element can take it as an
// immediate or a scalar. The hardware gives each 32-bit lane that 32-bit value
// and each element reads its own byte, so the byte must fill all four bytes;
// zero-extended, the source-group bit (0x20) of 32-bit elements and every
// control of 16- and 8-bit elements were lost.

void main(tensor in, tensor out, tensor outb, int k) {
  int5 c = 0;
  float64 x = v_f32_ld_tnsr_b(c, in);
  bfloat128 b = v_bf16_ld_tnsr_b(c, in);
  uchar256 ki = 0xa1, ks = (unsigned char)k;
  v_f32_st_tnsr(c, out, v_f32_shuffle_b(x, ki, 0, x));
  v_bf16_st_tnsr(c, outb, v_bf16_shuffle_b(b, ki, 0, b));
  c[1] = 1;
  v_f32_st_tnsr(c, out, v_f32_shuffle_b(x, ks, 0, x));
}

// 0xa1a1a1a1, printed signed.
// CHECK-DAG: shuffle.f32 {{.*}}, -0x5e5e5e5f
// CHECK-DAG: shuffle.bf16 {{.*}}, -0x5e5e5e5f
// CHECK-DAG: and.i32 [[BYTE:%S[0-9]+]], %S{{[0-9]+}}, 0xff
// CHECK-DAG: mul.i32 [[CTL:%S[0-9]+]], [[BYTE]], 0x1010101
// CHECK-DAG: shuffle.f32 {{.*}}, [[CTL]]
