// An unsigned 8-to-16-bit conversion of a vector the kernel computed (here a
// shuffle), so that it is not folded into the load: two UNPACKs that keep the
// bytes they do not write, so their destinations must be zero. The fork left
// them undefined and the high bytes held other vectors.
//
// DIFFTEST: arch gaudi2
// DIFFTEST: in  u8 256 4
// DIFFTEST: in  u8 256 4
// DIFFTEST: out u16 128 2 4
// DIFFTEST: index-space 1

void main(tensor in, tensor ctl, tensor out) {
  for (int r = 0; r < 4; r++) {
    int5 c = {0, r, 0, 0, 0};
    uchar256 a = v_u8_ld_tnsr_b(c, in);
    uchar256 k = v_u8_ld_tnsr_b(c, ctl) | 0x80;
    uchar256 x = v_u8_shuffle_b(a, k, 0, a);
    ushort256 y = convert_uchar256_to_ushort256(x, SW_RHNE | SW_LINEAR);
    int5 o = {0, 0, r, 0, 0};
    v_u16_st_tnsr(o, out, y.v1);
    o[1] = 1;
    v_u16_st_tnsr(o, out, y.v2);
  }
}
