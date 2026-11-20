// A ray tracer's first bounce: one primary ray per pixel of a 512 x 256 image
// against three spheres, the nearest hit shaded by a directional light
// (Lambert plus ambient), sky gradient elsewhere. Square roots, an rsqrt and
// compare-selects; pixel directions come from the lane id.
//
// BENCH: arch gaudi2
// BENCH: out f32 512 256
// BENCH: index-space 8 8

#define ROWS 32
#define FAR 1.0e30f

// The light direction, a unit vector.
#define LX 0.57735f
#define LY 0.57735f
#define LZ 0.57735f

// Nearest hit of the ray (dx, dy, dz) from the origin with sphere (cx, cy, cz)
// of radius r: updates tmin and the hit sphere's centre and 1/r.
#define SPHERE(cx, cy, cz, r)                                                  \
  {                                                                            \
    const float64 b = cx * dx + cy * dy + cz * dz;                            \
    const float64 disc = b * b - (cx * cx + cy * cy + cz * cz - r * r);       \
    float64 t = b - v_sqrt_f32(v_f32_max_b(disc, 0.0f));                      \
    t = v_f32_sel_grt_f32_b(disc, 0.0f, t, FAR);                               \
    t = v_f32_sel_grt_f32_b(t, 0.0f, t, FAR);                                  \
    hx = v_f32_sel_less_f32_b(t, tmin, cx, hx);                                \
    hy = v_f32_sel_less_f32_b(t, tmin, cy, hy);                                \
    hz = v_f32_sel_less_f32_b(t, tmin, cz, hz);                                \
    hinv = v_f32_sel_less_f32_b(t, tmin, 1.0f / r, hinv);                      \
    tmin = v_f32_min_b(t, tmin);                                               \
  }

void main(tensor out) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  const float64 lane = convert_int64_to_float64((int64)read_lane_id_4b_b(), SW_RHNE);
  int5 c = 0;

  for (int x = start[0] * 64; x < end[0] * 64; x += 64) {
    c[0] = x;
    const float64 u = (lane + (float)x - 256.0f) * (1.0f / 256.0f);
    for (int y = start[1] * ROWS; y < end[1] * ROWS; y++) {
      c[1] = y;
      const float v = (128.0f - (float)y) * (1.0f / 256.0f);
      const float64 norm = v_rsqrt_f32(u * u + v * v + 1.0f);
      const float64 dx = u * norm, dy = v * norm, dz = -norm;
      float64 tmin = FAR, hx = 0.0f, hy = 0.0f, hz = 0.0f, hinv = 0.0f;
      SPHERE(0.0f, 0.0f, -3.0f, 1.0f)
      SPHERE(1.5f, 0.5f, -4.0f, 0.7f)
      SPHERE(-1.5f, -0.3f, -3.5f, 0.6f)
      const float64 nx = (tmin * dx - hx) * hinv;
      const float64 ny = (tmin * dy - hy) * hinv;
      const float64 nz = (tmin * dz - hz) * hinv;
      const float64 lambert = v_f32_max_b(nx * LX + ny * LY + nz * LZ, 0.0f);
      const float64 sky = 0.2f + 0.3f * dy;
      v_f32_st_tnsr(c, out, v_f32_sel_less_f32_b(tmin, FAR, 0.1f + 0.9f * lambert, sky));
    }
  }
}
