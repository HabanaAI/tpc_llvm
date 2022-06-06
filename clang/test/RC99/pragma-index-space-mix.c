// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -emit-llvm -O1 %s -o - | FileCheck %s

// CHECK: define {{.*}}void @main() {{.*}} !index_space_b [[METADATA:.*]] {
// CHECK: [[METADATA]] = !{i32 0, i32 1, !"x * y", !"1", !"1", !"8"}

void main(tensor Tensor0_T, tensor Tensor1_T) {
#pragma index_space_b(0, 1)("x * y", "1", "1", "8")
#pragma index_space(0, 1)("64", "4", "1", "1")
  int5 index_space_start = get_index_space_offset();
  int5 index_space_end = index_space_start + get_index_space_size();
  for (int dim2 = index_space_start[2] * 1; dim2 < index_space_end[2] * 1; dim2 = dim2 + 1) {
    for (int dim0 = index_space_start[0] * 64; dim0 < index_space_end[0] * 64; dim0 = dim0 + 64) {
#pragma loop_unroll(4)
      for (int dim1 = index_space_start[1] * 4; dim1 < index_space_end[1] * 4; dim1 = dim1 + 1) {
        int5 coords = {dim0, dim1, dim2};
        float64 Tensor0 = v_f32_ld_tnsr_b(coords, Tensor0_T, 0, 0, 1, 0);
        int64 cast_f32_to_i32_ID_0 = (int64)(Tensor0);
        v_i32_st_tnsr(coords, Tensor1_T, cast_f32_to_i32_ID_0, 0, 1, 0);
      }
    }
  }
}
