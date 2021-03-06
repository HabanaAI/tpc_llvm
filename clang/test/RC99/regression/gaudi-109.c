// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -O1 %s -o - | FileCheck %s

void main(int ifm)
{    
    int5 idx = get_index_space_size();
    int t = idx[0];
    s_i32_st_l(ifm, t, 0, 1, 0);
}

// CHECK: mov_irf_dim  0x0 [[REGS:%S[0-9]+]], [[I:%I[0-9]+]]
// CHECK: st_l         [[S:%S[0-9]+]], [[REGS]]
