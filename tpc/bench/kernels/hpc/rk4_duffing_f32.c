// Fourth-order Runge-Kutta integration of the damped Duffing oscillator
// x'' = -delta x' - alpha x - beta x^3, one oscillator per lane from its own
// initial state, for 128 steps. No memory traffic inside the loop: the time
// goes to a chain of dependent multiply-adds.
//
// BENCH: arch gaudi2
// BENCH: in  f32 4096 range=-2,2
// BENCH: in  f32 4096 range=-2,2
// BENCH: out f32 4096
// BENCH: out f32 4096
// BENCH: index-space 64
// BENCH: scalar i32 128
// BENCH: scalar f32 0.01

#define DELTA 0.1f
#define ALPHA -1.0f
#define BETA 1.0f

static float64 accel(float64 x, float64 v) {
  return -DELTA * v - ALPHA * x - BETA * x * x * x;
}

void main(tensor x0, tensor v0, tensor x_out, tensor v_out, int steps, float dt) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  const float h = 0.5f * dt;
  const float sixth = dt / 6.0f;
  int5 c = 0;

  for (int i = start[0] * 64; i < end[0] * 64; i += 64) {
    c[0] = i;
    float64 x = v_f32_ld_tnsr_b(c, x0);
    float64 v = v_f32_ld_tnsr_b(c, v0);
    for (int s = 0; s < steps; s++) {
      const float64 k1x = v, k1v = accel(x, v);
      const float64 k2x = v + h * k1v, k2v = accel(x + h * k1x, v + h * k1v);
      const float64 k3x = v + h * k2v, k3v = accel(x + h * k2x, v + h * k2v);
      const float64 k4x = v + dt * k3v, k4v = accel(x + dt * k3x, v + dt * k3v);
      x += sixth * (k1x + 2.0f * k2x + 2.0f * k3x + k4x);
      v += sixth * (k1v + 2.0f * k2v + 2.0f * k3v + k4v);
    }
    v_f32_st_tnsr(c, x_out, x);
    v_f32_st_tnsr(c, v_out, v);
  }
}
