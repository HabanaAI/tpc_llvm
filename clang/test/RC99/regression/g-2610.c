// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu greco   %s  -o -  | FileCheck --check-prefixes=CHECK %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2  %s  -o -  | FileCheck --check-prefixes=CHECK %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1  %s  -o -  | FileCheck --check-prefixes=CHECK %s

#define ROUND_MODE SW_CSR
#define ANYCAST_TGT 1
char256 cast_32bits_to_i8_lin_order(char256 x0, char256 x1, char256 x2, char256 x3)
{
    char256 t = 0, y = 0;

    // 0..7
    y = v_i8_pack_b(x0, SW_GROUP_0 | SW_STRIDE_4, y);
    // 0..15
    y = v_i8_pack_b(x0, SW_GROUP_1 | SW_STRIDE_4, y);
    // 0..15, 16..31
    y = v_i8_mov_dual_group_b(y, 0xFFFF0000, 1, 0, MkWr(1, 0), y);
    // 0..15, 16..31, 32..47
    y = v_i8_mov_dual_group_b(y, 0x0000FFFF, 2, 0, MkWr(0, 1), y);
    // 0..15, 16..31, 32..47, 48..63
    y = v_i8_mov_dual_group_b(y, 0xFFFF0000, 3, 0, MkWr(0, 1), y);

    // 64..71
    t = v_i8_pack_b(x1, SW_GROUP_0 | SW_STRIDE_4, t);
    // 64..79
    t = v_i8_pack_b(x1, SW_GROUP_1 | SW_STRIDE_4, t);
    // 0..15, 16..31, 32..47, 48..63, 64..79
    y = v_i8_mov_dual_group_b(t, 0x0000FFFF, 0, 1, MkWr(1, 0), y);
    // 0..15, 16..31, 32..47, 48..63, 64..79, 80..95
    y = v_i8_mov_dual_group_b(t, 0xFFFF0000, 1, 1, MkWr(1, 0), y);
    // 0..15, 16..31, 32..47, 48..63, 64..79, 80..95, 96..111
    y = v_i8_mov_dual_group_b(t, 0x0000FFFF, 2, 1, MkWr(0, 1), y);
    // 0..15, 16..31, 32..47, 48..63, 64..79, 80..95, 96..111, 112..127
    y = v_i8_mov_dual_group_b(t, 0xFFFF0000, 3, 1, MkWr(0, 1), y);

    // 128..137
    t = v_i8_pack_b(x2, SW_GROUP_0 | SW_STRIDE_4, t);
    // 128..143
    t = v_i8_pack_b(x2, SW_GROUP_1 | SW_STRIDE_4, t);
    // 0..143
    y = v_i8_mov_dual_group_b(t, 0x0000FFFF, 0, 2, MkWr(1, 0), y);
    // 0..159
    y = v_i8_mov_dual_group_b(t, 0xFFFF0000, 1, 2, MkWr(1, 0), y);
    // 0..175
    y = v_i8_mov_dual_group_b(t, 0x0000FFFF, 2, 2, MkWr(0, 1), y);
    // 0..191
    y = v_i8_mov_dual_group_b(t, 0xFFFF0000, 3, 2, MkWr(0, 1), y);

    // 192..199
    t = v_i8_pack_b(x3, SW_GROUP_0 | SW_STRIDE_4, t);
    // 192..207
    t = v_i8_pack_b(x3, SW_GROUP_1 | SW_STRIDE_4, t);
    // 0..207
    y = v_i8_mov_dual_group_b(t, 0x0000FFFF, 0, 3, MkWr(1, 0), y);
    // 0..223
    y = v_i8_mov_dual_group_b(t, 0xFFFF0000, 1, 3, MkWr(1, 0), y);
    // 0..239
    y = v_i8_mov_dual_group_b(t, 0x0000FFFF, 2, 3, MkWr(0, 1), y);
    // 0..255
    y = v_i8_mov_dual_group_b(t, 0xFFFF0000, 3, 3, MkWr(0, 1), y);

    return y;
}

char256 cast_f32_to_i8_lin_order(float64 x0, float64 x1, float64 x2, float64 x3)
{
#ifdef ANYCAST_REF
    char256 t0 = 0, t1 = 0, t2 = 0, t3 = 0;
#if defined(__greco_plus__)
    // 0..63
    t0 = v_convert_f32_to_i8_b(x0, 0, ROUND_MODE, t0);
    // 64..127
    t1 = v_convert_f32_to_i8_b(x1, 0, ROUND_MODE, t1);
    // 128..192
    t2 = v_convert_f32_to_i8_b(x2, 0, ROUND_MODE, t2);
    // 192..255
    t3 = v_convert_f32_to_i8_b(x3, 0, ROUND_MODE, t3);
#elif defined(__dali__) || defined(__gaudi__) || defined(__gaudib__)
    int64 t_i32;

    // 0..63
    t_i32 = v_convert_f32_to_i32_b(x0, ROUND_MODE);
    t0 = v_convert_int32_to_i8_b(t_i32, 0, 0, ROUND_MODE, t0);

    // 64..127
    t_i32 = v_convert_f32_to_i32_b(x1, ROUND_MODE);
    t1 = v_convert_int32_to_i8_b(t_i32, 0, 0, ROUND_MODE, t1);

    // 128..192
    t_i32 = v_convert_f32_to_i32_b(x2, ROUND_MODE);
    t2 = v_convert_int32_to_i8_b(t_i32, 0, 0, ROUND_MODE, t2);

    // 192..255
    t_i32 = v_convert_f32_to_i32_b(x3, ROUND_MODE);
    t3 = v_convert_int32_to_i8_b(t_i32, 0, 0, ROUND_MODE, t3);
#else
    #error cast_f32_to_i8_lin_order is not implemented
#endif

    return cast_32bits_to_i8_lin_order(t0, t1, t2, t3);
#endif  // ANYCAST_REF

#ifdef ANYCAST_TGT
    float256 x;
    x.v1 = x0;
    x.v2 = x1;
    x.v3 = x2;
    x.v4 = x3;
    return convert_float256_to_char256(x, ROUND_MODE | SW_LINEAR);
#endif  // ANYCAST_TGT
}

void main(int inp_mem, int out_mem) {
  float64 *i_ptr = (float64 *)inp_mem;
  float64 x1 = *i_ptr++;
  float64 x2 = *i_ptr++;
  float64 x3 = *i_ptr++;
  float64 x4 = *i_ptr++;

  char256 c = cast_f32_to_i8_lin_order(x1, x2, x3, x4);
  
  char256* o_ptr = (char256 *)out_mem;
  *o_ptr = c;
}

// CHECK-DAG: ld_l_v   %V[[DES0:[0-9]+]], %S
// CHECK-DAG: ld_l_v   %V[[DES1:[0-9]+]], %S
// CHECK-DAG: ld_l_v   %V[[DES2:[0-9]+]], %S
// CHECK-DAG: ld_l_v   %V[[DES3:[0-9]+]], %S


// CHECK: convert.f32 target_type=int8  %V{{[0-9]+}}, %V[[DES0]]
// CHECK: convert.f32 target_type=int8  %V{{[0-9]+}}, %V[[DES1]]
// CHECK: convert.f32 target_type=int8  %V{{[0-9]+}}, %V[[DES2]]
// CHECK: convert.f32 target_type=int8  %V{{[0-9]+}}, %V[[DES3]]

// CHECK-NOT: convert.f32 all_lanes
