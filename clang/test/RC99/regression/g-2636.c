// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi   %s  -o -  | FileCheck --check-prefixes=CHECK %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudib   %s  -o -  | FileCheck --check-prefixes=CHECK %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu greco   %s  -o -  | FileCheck --check-prefixes=CHECK %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2  %s  -o -  | FileCheck --check-prefixes=CHECK %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1  %s  -o -  | FileCheck --check-prefixes=CHECK %s
#define ANYCAST_TGT 1

short128_pair_t cast_i8_to_i16_lin_order(char256 x)
{
    char256 group0, group1;
    group0 = v_i8_unpack_b(x, SW_GROUP_0 | SW_STRIDE_2 | SW_GROUP_HALF_0, 0);
    group1 = v_i8_unpack_b(x, SW_GROUP_1 | SW_STRIDE_2 | SW_GROUP_HALF_0, 0);

    short128_pair_t y;
    y.v1 = v_convert_i8_to_i16_b(group0);
    y.v2 = v_convert_i8_to_i16_b(group1);

    short128 vec0 = y.v1;
    y.v1 = v_i16_mov_dual_group_b(y.v2, 0xFFFFFFFF, 0, 1, MkWr(1, 1), y.v1);
    y.v1 = v_i16_mov_dual_group_b(vec0, 0xFFFFFFFF, 1, 2, MkWr(1, 1), y.v1);
    y.v1 = v_i16_mov_dual_group_b(y.v2, 0xFFFFFFFF, 1, 3, MkWr(1, 1), y.v1);
    y.v2 = v_i16_mov_dual_group_b(vec0, 0xFFFFFFFF, 2, 0, MkWr(1, 1), y.v2);
    y.v2 = v_i16_mov_dual_group_b(y.v2, 0xFFFFFFFF, 2, 1, MkWr(1, 1), y.v2);
    y.v2 = v_i16_mov_dual_group_b(vec0, 0xFFFFFFFF, 3, 2, MkWr(1, 1), y.v2);
    return y;
}

short256 cast(char256 x) {
#ifdef ANYCAST_REF
  return cast_i8_to_i16_lin_order(x);
#endif
#ifdef ANYCAST_TGT
  return convert_char256_to_short256(x, SW_LINEAR);
#endif
}

void main(int mem_inp, int mem_out) {
  char256 *p_inp = (char256 *)mem_inp;
  short256 *p_out = (short256 *)mem_out;

  *p_out = cast(*p_inp);
}
// CHECK: convert.i8 target_type=int16 rhne %V
// CHECK-NOT: all_lanes
