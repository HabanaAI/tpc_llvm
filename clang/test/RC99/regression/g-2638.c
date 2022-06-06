// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu greco   %s  -o -  | FileCheck --check-prefixes=CHECK %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2  %s  -o -  | FileCheck --check-prefixes=CHECK %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1  %s  -o -  | FileCheck --check-prefixes=CHECK %s
#define ANYCAST_TGT 1

short128_pair_t cast_i16_to_32bits_lin_order(short128 x)
{
    short128_pair_t t, y;
    t.v1 = v_i16_mov_group_b(x, 0xFFFFFFFF, 63, t.v1);
    t.v2 = v_i16_mov_dual_group_all_b(x, 0xFFFFFFFF, 0, 2, 1, 3, MkWrA(0b11, 0b10, 0b01, 0b11), t.v2);
    t.v2 = v_i16_mov_dual_group_all_b(t.v1, 0xFFFFFFFF, 2, 0, 3, 1, MkWrA(0b10, 0b01, 0b10, 0b01), t.v2);
    y.v1 = v_i16_unpack_b(t.v2, SW_GROUP_0 | SW_STRIDE_2 | SW_GROUP_HALF_0, 0/*y.v1*/);
    y.v2 = v_i16_unpack_b(t.v2, SW_GROUP_1 | SW_STRIDE_2 | SW_GROUP_HALF_0, 0/*y.v1*/);
    return y;
}

int64_pair_t cast_i16_to_i32_lin_order(short128 x, int shiftToI32)
{
    short128_pair_t  t = cast_i16_to_32bits_lin_order(x);

    int64_pair_t     y;
    y.v1 = v_convert_i16_to_i32_b(t.v1);
    y.v2 = v_convert_i16_to_i32_b(t.v2);
    y.v1 = v_i32_ash_b(y.v1, shiftToI32, 2);
    y.v2 = v_i32_ash_b(y.v2, shiftToI32, 2);
    return y;
}

int64_pair_t cast_i16_to_i32_lin_order_no_shift(short128 x)
{
    short128_pair_t t = cast_i16_to_32bits_lin_order(x);

    int64_pair_t     y;
    y.v1 = v_convert_i16_to_i32_b(t.v1);
    y.v2 = v_convert_i16_to_i32_b(t.v2);
    return y;
}

int128 cast(short128 x, int shift) {
#ifdef ANYCAST_REF
  return cast_i16_to_i32_lin_order(x, shift);
#endif
#ifdef ANYCAST_TGT
  int128 r = convert_short128_to_int128(x, SW_RHAZ | SW_LINEAR);
  r.v1 = v_i32_ash_b(r.v1, shift, 2);
  r.v2 = v_i32_ash_b(r.v2, shift, 2);
  return r;
#endif
}

void main(int mem_inp, int mem_out, int shift) {
  short128 *p_inp = (short128 *)mem_inp;
  int128 *p_out = (int128 *)mem_out;

  *p_out = cast(*p_inp, shift);
}

// CHECK-DAG: ld_l_v   %V[[DES0:[0-9]+]], %S
// CHECK: convert.i16 target_type=int32 rhaz  %V{{[0-9]+}}, %V[[DES0]]

// CHECK-NOT: convert.i16 all_lanes
