// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O1 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s
// GAUDI-1118
// XFAIL:*

void main(int dest) {
  volatile _BFloat16 __local *ptr = (_BFloat16 __local *) dest;

  *ptr = 0.0; ptr += 2;
// CHECK: mov.i16 {{.*}}, 0x0

  *ptr = 1.0; ptr += 2;
// CHECK: mov.i16 {{.*}}, 0x3f80

  *ptr = -1.0; ptr += 2;
  // 0xBF80 -> 49024 -> -49024 -> 4080
// CHECK: mov.i16 {{.*}}, -0x4080

  *ptr = 1.5; ptr += 2;
// CHECK: mov.i16 {{.*}}, 0x3fc0

  *ptr = -1.5; ptr += 2;
  // 0xH3FC0 -> 49088 -> -49088 -> 4040
// CHECK: mov.i16 {{.*}}, -0x4040

  *ptr = 1.75; ptr += 2;
// CHECK mov.i16 {{.*}}, 0x3fe0

  *ptr = -1.75; ptr += 2;
  // 0xBFE0 -> 49120 -> -49120 -> 4020
// CHECK: mov.i16 {{.*}}, -0x4020

  *ptr = 2.; ptr += 2;
// CHECK: mov.i16 {{.*}}, 0x4000

  *ptr = -2.; ptr += 2;
// CHECK: mov.i16 {{.*}}, -0x4000

  *ptr = (1.0 / 0.0); ptr += 2;
// CHECK: mov.i16 {{.*}}, 0x7f80

  *ptr = (-1.0 / 0.0); ptr += 2;
  // 0xff80 -> 65408 -> -65408 -> 0x0080
// CHECK: mov.i16 {{.*}}, -0x80

  *ptr++ = (0.0 / 0.0); ptr += 2;
// CHECK: mov.i16 {{.*}}, 0x7fc0
}
