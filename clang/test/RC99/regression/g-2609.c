// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi   %s  -o -  | FileCheck --check-prefixes=CHECK %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudib   %s  -o -  | FileCheck --check-prefixes=CHECK %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu greco   %s  -o -  | FileCheck --check-prefixes=CHECK %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2  %s  -o -  | FileCheck --check-prefixes=CHECK %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1  %s  -o -  | FileCheck --check-prefixes=CHECK %s
#define ANYCAST_TGT 1

#define ROUND_MODE SW_CSR

char256 cast_16bits_to_i8_lin_order(char256 x0, char256 x1)
{
    char256 t = 0, y = 0;

    y = v_i8_pack_b(x0, SW_GROUP_0 | SW_STRIDE_2, y);
    y = v_i8_pack_b(x0, SW_GROUP_1 | SW_STRIDE_2, y);

    // 0..15, 16..31, 32..47,
    y = v_i8_mov_dual_group_all_b(y, 0xFFFFFFFF, 1, 2, 3, 0, MkWrA(0b10, 0b01, 0b00, 0b00), y);
    // 0..15, 16..31, 32..47, 48..63
    y = v_i8_mov_dual_group_b(y, 0xFFFFFFFF, 3, 1, MkWr(0, 1), y);

    t = v_i8_pack_b(x1, SW_GROUP_0 | SW_STRIDE_2, t);
    t = v_i8_pack_b(x1, SW_GROUP_1 | SW_STRIDE_2, t);

    // 0..15, 16..31, 32..47, 48..63, 64..79, 96..111
    y = v_i8_mov_dual_group_all_b(t, 0xFFFFFFFF, 1, 3, 0, 2, MkWrA(0b00, 0b00, 0b01, 0b01), y);
    // 0..15, 16..31, 32..47, 48..63, 64..79, 80..95, 96..111, 112..127
    y = v_i8_mov_dual_group_all_b(t, 0xFFFFFFFF, 0, 2, 1, 3, MkWrA(0b00, 0b00, 0b10, 0b10), y);

    return y;
}

 char256 cast_lin_bf16_to_i8_swizzle_order(bfloat128 x0, bfloat128 x1)
 {
#ifdef ANYCAST_REF
    bfloat256 t;

    t.v1 = x0;
    t.v2 = x1;

    return v_convert_bf16_to_i8_all_b(t, (ROUND_MODE));
#endif  // ANYCAST_REF
#ifdef ANYCAST_TGT
     bfloat256 v;
     v.v1 = x0;
     v.v2 = x1;
     return convert_bfloat256_to_char256(v, ROUND_MODE);
#endif  // ANYCAST_TGT
 }

 char256 cast_bf16_to_i8_lin_order(bfloat128 x0, bfloat128 x1)
 {
#ifdef ANYCAST_REF
    char256 t0 = 0, t1 = 0;

#if defined(__greco_plus__)
    // 0..127
    t0 = v_convert_bf16_to_i8_b(x0, 0, (ROUND_MODE), 0);
    // 128..255
    t1 = v_convert_bf16_to_i8_b(x1, 0, (ROUND_MODE), 0);
#elif defined(__dali__) || defined(__gaudi__) || defined(__gaudib__)
    short128 t_i16;
    // 0..127
    t_i16 = v_convert_bf16_to_i16_b(x0, ROUND_MODE); // default ROUND_MODE is SW_RHNE
    t0 = v_convert_int16_to_i8_b(t_i16, 0, 0, ROUND_MODE, t0, 1, 0);
    // 128..255
    t_i16 = v_convert_bf16_to_i16_b(x1, ROUND_MODE);
    t1 = v_convert_int16_to_i8_b(t_i16, 0, 0, ROUND_MODE, t1, 1, 0);
#else
    #error "cast_bf16_to_i8_lin_order() is not defined"
#endif

    return cast_16bits_to_i8_lin_order(t0, t1);
#endif  // ANYCAST_REF

#ifdef ANYCAST_TGT
    bfloat256 v;
    v.v1 = x0;
    v.v2 = x1;
    return convert_bfloat256_to_char256(v, ROUND_MODE | SW_LINEAR);
#endif  // ANYCAST_TGT
 }

void main(int inp_mem, int out_mem) {
  bfloat128 *i_ptr = (bfloat128 *)inp_mem;
  bfloat128 x1 = *i_ptr++;
  bfloat128 x2 = *i_ptr;

  char256 c = cast_bf16_to_i8_lin_order(x1, x2);
  
  char256* o_ptr = (char256 *)out_mem;
  *o_ptr = c;
}
// CHECK: convert.bf16
// CHECK-NOT: all_lanes
