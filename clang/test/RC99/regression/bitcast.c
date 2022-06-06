// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -O1 %s -o -

void main(tensor in, tensor out, float ff) {

  int5 space = {0, 0, 0, 0, 0};

  float64 f = v_f32_ld_tnsr_b(space, in, 0, 0 /*source*/, 1, 1);
  space.x = space.x + 1;
  int64 i = v_i32_ld_tnsr_b(space, in, 0, 0 /*source*/, 1, 1);

  int64 cast_f = i + *(int64*)(&f);
  v_i32_st_tnsr(space, out, cast_f, 0, 1, 1);
  space.x = space.x + 1;
  f = f + 1.0f;
  v_f32_st_tnsr(space, out, f, 0, 1, 1);

  //float x = ff;
  int ii = *((int*)(&ff));

  if (ii < 10) {
    ff = ff * 4.0f;
  }
  v_f32_st_tnsr(space, out, f + ff, 0, 1, 1);

}
