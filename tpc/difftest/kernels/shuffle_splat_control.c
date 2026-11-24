// Shuffles whose control is one byte for every element, each of the 256
// bytes: as a constant (an immediate) and from a scalar parameter, on 32-, 16-
// and 8-bit elements. The fork passed the byte zero-extended; the hardware
// reads a byte per element of the 32-bit value, so the source group bit of
// 32-bit elements and all of the control of 16- and 8-bit elements were lost.
//
// DIFFTEST: arch gaudi2
// DIFFTEST: in  f32 64 2
// DIFFTEST: in  bf16 128 2
// DIFFTEST: in  u8 256 2
// DIFFTEST: out f32 64 256
// DIFFTEST: out f32 64 256
// DIFFTEST: out bf16 128 256
// DIFFTEST: out bf16 128 256
// DIFFTEST: out u8 256 256
// DIFFTEST: out u8 256 256
// DIFFTEST: index-space 1
// DIFFTEST: scalar i32 0

void main(tensor in32, tensor in16, tensor in8, tensor f_imm, tensor f_srf,
          tensor b_imm, tensor b_srf, tensor u_imm, tensor u_srf, int base) {
  int5 c = 0;
  float64 f = v_f32_ld_tnsr_b(c, in32);
  bfloat128 b = v_bf16_ld_tnsr_b(c, in16);
  uchar256 u = v_u8_ld_tnsr_b(c, in8);
  c[1] = 1;
  float64 fi = v_f32_ld_tnsr_b(c, in32);
  bfloat128 bi = v_bf16_ld_tnsr_b(c, in16);
  uchar256 ui = v_u8_ld_tnsr_b(c, in8);
  #pragma unroll
  for (int k = 0; k < 256; k++) {
    c[1] = k;
    const uchar256 imm = (unsigned char)k;
    const uchar256 srf = (unsigned char)(k + base);
    v_f32_st_tnsr(c, f_imm, v_f32_shuffle_b(f, imm, 0, fi));
    v_f32_st_tnsr(c, f_srf, v_f32_shuffle_b(f, srf, 0, fi));
    v_bf16_st_tnsr(c, b_imm, v_bf16_shuffle_b(b, imm, 0, bi));
    v_bf16_st_tnsr(c, b_srf, v_bf16_shuffle_b(b, srf, 0, bi));
    v_u8_st_tnsr(c, u_imm, v_u8_shuffle_b(u, imm, 0, ui));
    v_u8_st_tnsr(c, u_srf, v_u8_shuffle_b(u, srf, 0, ui));
  }
}
