// The demoscene plasma effect, one frame of 512 x 256 pixels:
// v = sin(x/16) + sin(y/8 + t) + sin((x + y)/32) + sin(r/8), r the distance
// from the centre, and red, green, blue = 0.5 + 0.5 sin(pi v + phase). Seven
// tpclib sines and a square root per pixel, three output planes.
//
// BENCH: arch gaudi2
// BENCH: out f32 512 256
// BENCH: out f32 512 256
// BENCH: out f32 512 256
// BENCH: index-space 8 8
// BENCH: scalar f32 1.5

#define ROWS 32
#define PI 3.14159265f

void main(tensor red, tensor green, tensor blue, float t) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  const float64 lane = convert_int64_to_float64((int64)read_lane_id_4b_b(), SW_RHNE);
  int5 c = 0;

  for (int x = start[0] * 64; x < end[0] * 64; x += 64) {
    c[0] = x;
    const float64 px = lane + (float)x;
    for (int y = start[1] * ROWS; y < end[1] * ROWS; y++) {
      c[1] = y;
      const float py = (float)y;
      const float64 cx = px - 256.0f;
      const float cy = py - 128.0f;
      const float64 r = v_sqrt_f32(cx * cx + cy * cy);
      const float64 v = v_sin_f32(px * (1.0f / 16.0f)) + v_sin_f32(py * (1.0f / 8.0f) + t) +
                        v_sin_f32((px + py) * (1.0f / 32.0f)) + v_sin_f32(r * (1.0f / 8.0f));
      v_f32_st_tnsr(c, red, 0.5f + 0.5f * v_sin_f32(PI * v));
      v_f32_st_tnsr(c, green, 0.5f + 0.5f * v_sin_f32(PI * v + 2.0943951f));
      v_f32_st_tnsr(c, blue, 0.5f + 0.5f * v_sin_f32(PI * v + 4.1887902f));
    }
  }
}
