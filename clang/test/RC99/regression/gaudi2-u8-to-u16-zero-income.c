// RUN: %tpc_clang -march=gaudi2 -O2 -S %s -o - | FileCheck %s

// An unsigned 8-to-16-bit conversion with no convert after it is two UNPACKs,
// which write every second byte and keep the others: their destinations must
// be zero, as Habana's compiler makes them. With an undefined income the high
// bytes were whatever the register held (here the shuffle's control).

void main(tensor in, tensor ctl, tensor out) {
  int5 c = 0;
  uchar256 a = v_u8_ld_tnsr_b(c, in);
  uchar256 k = v_u8_ld_tnsr_b(c, ctl) | 0x80;
  uchar256 x = v_u8_shuffle_b(a, k, 0, a);
  ushort256 y = convert_uchar256_to_ushort256(x, SW_RHNE | SW_LINEAR);
  v_u16_st_tnsr(c, out, y.v1);
  c[1] = 1;
  v_u16_st_tnsr(c, out, y.v2);
}

// CHECK:     mov.i8 [[ZA:%V[0-9]+]], 0x0
// CHECK:     mov.i8 [[ZB:%V[0-9]+]], 0x0
// CHECK-DAG: unpack.u8 source_group=0 element_stride=2 group_half=0 {{.*}} [[ZB]], %V
// CHECK-DAG: unpack.u8 source_group=1 element_stride=2 group_half=0 {{.*}} [[ZA]], %V
