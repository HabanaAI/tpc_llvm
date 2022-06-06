// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi   %s  -o -  | FileCheck --check-prefixes=CHECK %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudib   %s  -o -  | FileCheck --check-prefixes=CHECK %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu greco   %s  -o -  | FileCheck --check-prefixes=CHECK %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2  %s  -o -  | FileCheck --check-prefixes=CHECK %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1  %s  -o -  | FileCheck --check-prefixes=CHECK %s
#define ANYCAST_TGT 1

int256 cast_i8_to_32bits_lin_order(char256 x)
{
    int256 y;
    int64 t0, t1;
    y.v1 = (int64) v_i8_unpack_b(x, SW_GROUP_0 | SW_STRIDE_4 | SW_GROUP_HALF_0, 0/*x00*/);
    y.v2 = (int64) v_i8_unpack_b(x, SW_GROUP_0 | SW_STRIDE_4 | SW_GROUP_HALF_1, 0/*x01*/);
    y.v3 = (int64) v_i8_unpack_b(x, SW_GROUP_1 | SW_STRIDE_4 | SW_GROUP_HALF_0, 0/*x02*/);
    y.v4 = (int64) v_i8_unpack_b(x, SW_GROUP_1 | SW_STRIDE_4 | SW_GROUP_HALF_1, 0/*x03*/);
    t0 = y.v1;
    y.v1 = v_i32_mov_dual_group_all_b(y.v2, 0xFFFFFFFF, 1, 0, 3, 2, MkWrA(0b00, 0b11, 0b00, 0b11), y.v1);
    y.v2 = v_i32_mov_dual_group_all_b(t0, 0xFFFFFFFF, 1, 0, 3, 2, MkWrA(0b11, 0b00, 0b11, 0b00), y.v2);
    t1 = y.v3;
    y.v3 = v_i32_mov_dual_group_all_b(y.v4, 0xFFFFFFFF, 1, 0, 3, 2, MkWrA(0b00, 0b11, 0b00, 0b11), y.v3);
    y.v4 = v_i32_mov_dual_group_all_b(t1, 0xFFFFFFFF, 1, 0, 3, 2, MkWrA(0b11, 0b00, 0b11, 0b00), y.v4);
    t0 = y.v1;
    y.v1 = v_i32_mov_dual_group_all_b(y.v3, 0xFFFFFFFF, 2, 3, 0, 1, MkWrA(0b00, 0b00, 0b11, 0b11), y.v1);
    t1 = y.v2;
    y.v2 = v_i32_mov_dual_group_all_b(y.v4, 0xFFFFFFFF, 2, 3, 0, 1, MkWrA(0b00, 0b00, 0b11, 0b11), y.v2);
    y.v3 = v_i32_mov_dual_group_all_b(t0, 0xFFFFFFFF, 2, 3, 0, 1, MkWrA(0b11, 0b11, 0b00, 0b00), y.v3);
    y.v4 = v_i32_mov_dual_group_all_b(t1, 0xFFFFFFFF, 2, 3, 0, 1, MkWrA(0b11, 0b11, 0b00, 0b00), y.v4);
    return y;
}

uint256 cast_u8_to_32bits_lin_order(uchar256 x)
{
    int256 t = cast_i8_to_32bits_lin_order((char256)x);

    uint256 y;
    y.v1 = (uint64)t.v1;
    y.v2 = (uint64)t.v2;
    y.v3 = (uint64)t.v3;
    y.v4 = (uint64)t.v4;

    return y;
}

uint256 cast(uchar256 x) {
#ifdef ANYCAST_REF
  return cast_u8_to_32bits_lin_order(x);
#endif
#ifdef ANYCAST_TGT
  return convert_uchar256_to_uint256(x, SW_RHNE | SW_LINEAR);
#endif
}

void main(int mem_inp, int mem_out) {
  uchar256 *p_inp = (uchar256 *)mem_inp;
  uint256 *p_out = (uint256 *)mem_out;

  *p_out = cast(*p_inp);
}
// CHECK-NOT: convert.u8 
// CHECK-NOT: all_lanes
