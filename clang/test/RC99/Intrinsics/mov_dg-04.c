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
    minifloat256 *d_ptr = (minifloat256 __local *)dest_ptr;
    
    res = v_f8_mov_dual_group_b(xx, 9, 0, 3, SW_WR_LOWER_GROUP, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=0 dst=3 wr_lg=0x1 wr_ug=0x0 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x9, %SP{{[0-9]+}}

    res = v_f8_mov_dual_group_b(xx, 7, 1, 2, SW_WR_LOWER_GROUP | SW_WR_UPPER_GROUP, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=1 dst=2 wr_lg=0x1 wr_ug=0x1 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7, %SP{{[0-9]+}}

    res = v_f8_mov_dual_group_b(xx, 15, 3, 1, SW_WR_UPPER_GROUP, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=3 dst=1 wr_lg=0x0 wr_ug=0x1 %V{{[0-9]+}}, %V{{[0-9]+}}, 0xf, %SP{{[0-9]+}}

    res = v_f8_mov_dual_group_vb(xx, 1, 0, 3, SW_WR_LOWER_GROUP, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=0 dst=3 wr_lg=0x1 wr_ug=0x0 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x1, %VP{{[0-9]+}}

    res = v_f8_mov_dual_group_vb(xx, 7, 1, 2, SW_WR_LOWER_GROUP | SW_WR_UPPER_GROUP, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=1 dst=2 wr_lg=0x1 wr_ug=0x1 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7, %VP{{[0-9]+}}

    res = v_f8_mov_dual_group_vb(xx, 15, 3, 1, SW_WR_UPPER_GROUP, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=3 dst=1 wr_lg=0x0 wr_ug=0x1 %V{{[0-9]+}}, %V{{[0-9]+}}, 0xf, %VP{{[0-9]+}}
    dest_ptr = (float64 __local *)d_ptr;
  }

  {
    minihalf256 res = 1.0;
    minihalf256 xx  = 2.0;
    minihalf256 *d_ptr = (minihalf256 __local *)dest_ptr;
    
    res = v_h8_mov_dual_group_b(xx, 9, 0, 3, SW_WR_LOWER_GROUP, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=0 dst=3 wr_lg=0x1 wr_ug=0x0 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x9, %SP{{[0-9]+}}

    res = v_h8_mov_dual_group_b(xx, 7, 1, 2, SW_WR_LOWER_GROUP | SW_WR_UPPER_GROUP, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=1 dst=2 wr_lg=0x1 wr_ug=0x1 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7, %SP{{[0-9]+}}

    res = v_h8_mov_dual_group_b(xx, 15, 3, 1, SW_WR_UPPER_GROUP, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=3 dst=1 wr_lg=0x0 wr_ug=0x1 %V{{[0-9]+}}, %V{{[0-9]+}}, 0xf, %SP{{[0-9]+}}

    res = v_h8_mov_dual_group_vb(xx, 1, 0, 3, SW_WR_LOWER_GROUP, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=0 dst=3 wr_lg=0x1 wr_ug=0x0 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x1, %VP{{[0-9]+}}

    res = v_h8_mov_dual_group_vb(xx, 7, 1, 2, SW_WR_LOWER_GROUP | SW_WR_UPPER_GROUP, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=1 dst=2 wr_lg=0x1 wr_ug=0x1 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7, %VP{{[0-9]+}}

    res = v_h8_mov_dual_group_vb(xx, 15, 3, 1, SW_WR_UPPER_GROUP, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg src=3 dst=1 wr_lg=0x0 wr_ug=0x1 %V{{[0-9]+}}, %V{{[0-9]+}}, 0xf, %VP{{[0-9]+}}
    dest_ptr = (float64 __local *)d_ptr;
  }

}
