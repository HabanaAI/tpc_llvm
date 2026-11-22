// RUN: %tpc_clang -march=gaudi2 -O2 -S %s -o - | FileCheck %s

// The vector spill slots start after the VLM the kernel reaches at constant
// addresses: they used to start after the __local__ data only, from 0 here,
// where they could overwrite the vector the kernel keeps at 0x400.
// 44 live vectors spill.

#define LD(i) c[1] = i; float64 v##i = v_f32_ld_tnsr_b(c, a);
#define MA(i, j) s = s * v##i + v##j;

void main(tensor a, tensor out, int slot) {
  int5 c = 0;
  v_f32_st_l_v(1024, v_f32_ld_tnsr_b(c, a) + 1.0f);
  LD(0) LD(1) LD(2) LD(3)
  LD(4) LD(5) LD(6) LD(7)
  LD(8) LD(9) LD(10) LD(11)
  LD(12) LD(13) LD(14) LD(15)
  LD(16) LD(17) LD(18) LD(19)
  LD(20) LD(21) LD(22) LD(23)
  LD(24) LD(25) LD(26) LD(27)
  LD(28) LD(29) LD(30) LD(31)
  LD(32) LD(33) LD(34) LD(35)
  LD(36) LD(37) LD(38) LD(39)
  LD(40) LD(41) LD(42) LD(43)
  float64 s = v2;
  MA(0, 3) MA(1, 10) MA(2, 17) MA(3, 24)
  MA(4, 31) MA(5, 38) MA(6, 1) MA(7, 8)
  MA(8, 15) MA(9, 22) MA(10, 29) MA(11, 36)
  MA(12, 43) MA(13, 6) MA(14, 13) MA(15, 20)
  MA(16, 27) MA(17, 34) MA(18, 41) MA(19, 4)
  MA(20, 11) MA(21, 18) MA(22, 25) MA(23, 32)
  MA(24, 39) MA(25, 2) MA(26, 9) MA(27, 16)
  MA(28, 23) MA(29, 30) MA(30, 37) MA(31, 0)
  MA(32, 7) MA(33, 14) MA(34, 21) MA(35, 28)
  MA(36, 35) MA(37, 42) MA(38, 5) MA(39, 12)
  MA(40, 19) MA(41, 26) MA(42, 33) MA(43, 40)
  c[1] = 0;
  v_f32_st_tnsr(c, out, s + v_f32_ld_l_v_b(slot * 256));
}

// CHECK-LABEL: main:
// CHECK-NOT: {{st_l_v|ld_l_v}} {{.*}}0x{{[0-3]}}00
// CHECK: st_l_v 0x400,
// CHECK-NOT: {{st_l_v|ld_l_v}} {{.*}}0x{{[0-4]}}00
// CHECK: halt
