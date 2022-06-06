// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck %s

void main(int dest, int src, int vpredp, _Bool pred) {
  float64 __local *dest_ptr = (float64 __local *)dest;
  float64 __local *src_ptr = (float64  __local *)src;
  bool256 __local *vpred_ptr = (bool256 __local *)vpredp;

  float64 x      = *src_ptr++;
  bool256 vpred  = *vpred_ptr++;
  float64 income = *dest_ptr;

  {
    minifloat256 res = 1.0;
    minifloat256 xx  = 2.0;
    minifloat256 __local *d_ptr = (minifloat256 __local *)dest_ptr;

    res = v_f8_mov_dual_group_pack_b(xx, 0, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

    res = v_f8_mov_dual_group_pack_b(xx, SW_PACK21, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

    res = v_f8_mov_dual_group_pack_b(xx, SW_PACK41, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=1 %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

    res = v_f8_mov_dual_group_pack_vb(xx, 0, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

    res = v_f8_mov_dual_group_pack_vb(xx, SW_PACK21, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

    res = v_f8_mov_dual_group_pack_vb(xx, SW_PACK41, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=1 %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
    dest_ptr = (float64 __local *)d_ptr;
  }

  {
    minihalf256 res = 1.0;
    minihalf256 xx  = 2.0;
    minihalf256 __local *d_ptr = (minihalf256 __local *)dest_ptr;

    res = v_h8_mov_dual_group_pack_b(xx, 0, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

    res = v_h8_mov_dual_group_pack_b(xx, SW_PACK21, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

    res = v_h8_mov_dual_group_pack_b(xx, SW_PACK41, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=1 %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

    res = v_h8_mov_dual_group_pack_vb(xx, 0, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

    res = v_h8_mov_dual_group_pack_vb(xx, SW_PACK21, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=0 %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

    res = v_h8_mov_dual_group_pack_vb(xx, SW_PACK41, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg pack pack_type=1 %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }
}
