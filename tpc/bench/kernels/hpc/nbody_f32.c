// Gravitational accelerations of 1024 bodies by direct summation, the inner
// loop of an N-body step: each lane is a body i, and the loop over all bodies
// j reads their position and mass as scalars from global memory. Softening
// keeps r > 0 for i == j. An rsqrt and 13 flops per pair.
//
// BENCH: arch gaudi2
// BENCH: in  f32 1024 range=-1,1
// BENCH: in  f32 1024 range=-1,1
// BENCH: in  f32 1024 range=-1,1
// BENCH: in  f32 1024 range=0.1,1
// BENCH: out f32 1024
// BENCH: out f32 1024
// BENCH: out f32 1024
// BENCH: index-space 16

#define SOFTENING 1e-3f

void main(tensor px, tensor py, tensor pz, tensor mass, tensor ax, tensor ay, tensor az) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  const int n = get_dim_size(px, 0);
  int5 c = 0;
  int5 j5 = 0;

  for (int i = start[0] * 64; i < end[0] * 64; i += 64) {
    c[0] = i;
    const float64 xi = v_f32_ld_tnsr_b(c, px);
    const float64 yi = v_f32_ld_tnsr_b(c, py);
    const float64 zi = v_f32_ld_tnsr_b(c, pz);
    float64 axi = 0.0f, ayi = 0.0f, azi = 0.0f;
    for (int j = 0; j < n; j++) {
      j5[0] = j;
      const float xj = s_f32_ld_g(gen_addr(j5, px));
      const float yj = s_f32_ld_g(gen_addr(j5, py));
      const float zj = s_f32_ld_g(gen_addr(j5, pz));
      const float mj = s_f32_ld_g(gen_addr(j5, mass));
      const float64 dx = xj - xi, dy = yj - yi, dz = zj - zi;
      const float64 inv = v_rsqrt_f32(dx * dx + dy * dy + dz * dz + SOFTENING);
      const float64 f = mj * inv * inv * inv;
      axi += f * dx;
      ayi += f * dy;
      azi += f * dz;
    }
    v_f32_st_tnsr(c, ax, axi);
    v_f32_st_tnsr(c, ay, ayi);
    v_f32_st_tnsr(c, az, azi);
  }
}
