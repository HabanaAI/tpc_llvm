// gaudi-197
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O1 %s -o - | FileCheck %s

// CHECK: set_indx [[coords:%I[0-9]+]], b11111, 0x0
// CHECK: ld_tnsr [[v0:%V[0-9]+]], 0x0, [[coords]]
// CHECK: ld_tnsr [[v1:%V[0-9]+]], 0x1, [[coords]]
// CHECK: cmp_grt.i32 [[mask:%VP[0-9]+]], [[v1]], [[v0]]
// CHECK: mov [[tmp:%V[0-9]+]], [[v1]], [[mask]]
// CHECK: mov [[o1:%V[0-9]+]], [[v0]], [[mask]]
// CHECK: mov [[o2:%V[0-9]+]], [[tmp]], [[mask]]
// CHECK: st_tnsr 0x2, [[coords]], [[o2]]
// CHECK: st_tnsr 0x3, [[coords]], [[o1]]

void main(tensor t0,
          tensor t1,
          tensor t2,
          tensor t3,
          int num)
{
    int5 coords = {0};
    int64 v0;
    int64 v1;
    v0 = v_i32_ld_tnsr_b(coords, t0, 0, v0, 1, 0);
    v1 = v_i32_ld_tnsr_b(coords, t1, 0, v1, 1, 0);

    bool256 mask;mask = from_bool64(v_i32_cmp_grt_b(v1, v0, 0, to_bool64(mask), 1, 0));
     // switch places
    int64 tmp;tmp =  v_i32_mov_vb(v1, 0, tmp, to_bool64(mask), 0);
    v1 = v_i32_mov_vb(v0, 0, v1, to_bool64(mask), 0);
    v0 = v_i32_mov_vb(tmp, 0, v0, to_bool64(mask), 0);

    // write result externally
    v_i32_st_tnsr(coords, t2, v0, 0, 1, 0);
    v_i32_st_tnsr(coords, t3, v1, 0, 1, 0);
}
