// RUN: %clang_cc1 -S -O2 -mllvm --emit-index-factors=false -triple tpc-none-none -std=rc99 -target-cpu gaudi2 -long-irf %s -o - | FileCheck %s
// RUN: %clang_cc1 -S -O2 -mllvm --emit-index-factors=false -triple tpc-none-none -std=rc99 -target-cpu doron1 -long-irf %s -o - | FileCheck %s

void main(tensor input, tensor output) {
  const int depth = 0;
  const int width = 1;
  const int height = 2;
  const int batch = 3;

  const int5 index_delta = { 64, 4, 1, 1, 0 };
  const int5 depthStep   = { 64, 0, 0, 0, 0 };
  const int5 widthStep   = { 0,  4, 0, 0, 0 };
  const int5 heightStep  = { 0,  0, 1, 0, 0 };
  const int5 batchStep   = { 0,  0, 0, 1, 0 };
  
  const int5 index_space_start = get_index_space_offset() * index_delta;
  const int5 index_space_end = (get_index_space_size() + get_index_space_offset()) * index_delta;
  int5 ifmCoords = index_space_start;

  float64 in;
  float64 out;

  for (; ifmCoords[depth] < index_space_end[depth]; ifmCoords += depthStep) {
    for (; ifmCoords[batch] < index_space_end[batch]; ifmCoords += batchStep) {
      for (; ifmCoords[height] < index_space_end[height]; ifmCoords += heightStep) {
        for (; ifmCoords[width] < index_space_end[width]; ifmCoords += widthStep) {
          in = v_f32_ld_tnsr_b(ifmCoords, input, 0, 0, 1, 0);
          out = v_f32_abs_b(in, 0, 0, 1, 0);
          v_f32_st_tnsr(ifmCoords, output, out, 0, 1, 0);
        }
      }
    }
  }
}

// CHECK: cmp_less.i32  b00001  %SP{{[0-9]+}}, [[NDX1:%I[0-9]+]], [[NDX2:%I[0-9]+]]
// CHECK: cmp_less.i32  b01000  %SP{{[0-9]+}}, [[NDX1]], [[NDX2]]
// CHECK: cmp_less.i32  b00100  %SP{{[0-9]+}}, [[NDX1]], [[NDX2]]
// CHECK: cmp_less.i32  b00010  %SP{{[0-9]+}}, [[NDX1]], [[NDX2]]
