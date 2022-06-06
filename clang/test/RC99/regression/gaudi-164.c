// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O1 %s -o -

void main(tensor input, tensor output) {
  float64 v1 = 3.3;
  uint64_float64_pair_t pairv2v3 = { 0, 0 };
  pairv2v3 = v_f32_get_lut_entry_and_interval_start_b(v1, 0, (0) << 13, pairv2v3, 1, 0);
  pairv2v3.v2 = v_f32_mac_b(pairv2v3.v2, v1, pairv2v3.v2, (0) << 1, 1, 0);
}