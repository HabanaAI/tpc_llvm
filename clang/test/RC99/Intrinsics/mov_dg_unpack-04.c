// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck  %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck  %s

void main(int dest, int src, int vpredp, _Bool pred) {
  float64 __local *dest_ptr  = (float64 __local *)dest;
  float64 __local *src_ptr   = (float64  __local *)src;
  bool256 __local *vpred_ptr = (bool256 __local *)vpredp;

  float64 x      = *src_ptr++;
  bool256 vpred  = *vpred_ptr++;
  float64 income = *dest_ptr;

  {
    minifloat256 res = 1.0;
    minifloat256 xx  = 2.0;
    minifloat256 __local *d_ptr = (minifloat256 __local *)dest_ptr;
    
    res = v_f8_mov_dual_group_unpack_b(xx, 1, 0, 1, 2, 3, SW_WR_LOWER_GROUP0, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg unpack unpack_type=0 sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=1 weg1=0 weg2=0 weg3=0 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x1, %SP{{[0-9]+}}

    res = v_f8_mov_dual_group_unpack_b(xx, 7, 1, 2, 3, 0, SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0, res, pred, 0);
    *d_ptr++ = res;
    
// CHECK: mov_dg unpack unpack_type=0 sdg0=1 sdg1=2 sdg2=3 sdg3=0 weg0=3 weg1=0 weg2=0 weg3=0 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7, %SP{{[0-9]+}}
    res = v_f8_mov_dual_group_unpack_b(xx, 15, 2, 3, 0, 1, SW_WR_UPPER_GROUP0, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg unpack unpack_type=0 sdg0=2 sdg1=3 sdg2=0 sdg3=1 weg0=2 weg1=0 weg2=0 weg3=0 %V{{[0-9]+}}, %V{{[0-9]+}}, 0xf, %SP{{[0-9]+}}

    res = v_f8_mov_dual_group_unpack_vb(xx, 1, 0, 1, 2, 3, SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg unpack unpack_type=0 sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=0 weg2=0 weg3=0 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x1, %VP{{[0-9]+}}

    res = v_f8_mov_dual_group_unpack_vb(xx, 7, 1, 2, 3, 0, SW_WR_LOWER_GROUP1 | SW_WR_UPPER_GROUP1, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg unpack unpack_type=0 sdg0=1 sdg1=2 sdg2=3 sdg3=0 weg0=0 weg1=3 weg2=0 weg3=0 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7, %VP{{[0-9]+}}

    res = v_f8_mov_dual_group_unpack_vb(xx, 15, 3, 0, 1, 2, SW_WR_UPPER_GROUP0 | SW_WR_UPPER_GROUP1, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg unpack unpack_type=0 sdg0=3 sdg1=0 sdg2=1 sdg3=2 weg0=2 weg1=2 weg2=0 weg3=0 %V{{[0-9]+}}, %V{{[0-9]+}}, 0xf, %VP{{[0-9]+}}
    dest_ptr = (float64 __local *)d_ptr;
  }

  {
    minihalf256 res = 1.0;
    minihalf256 xx  = 2.0;
    minihalf256 __local *d_ptr = (minihalf256 __local *)dest_ptr;
    
    res = v_h8_mov_dual_group_unpack_b(xx, 1, 0, 1, 2, 3, SW_WR_LOWER_GROUP0, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg unpack unpack_type=0 sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=1 weg1=0 weg2=0 weg3=0 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x1, %SP{{[0-9]+}}

    res = v_h8_mov_dual_group_unpack_b(xx, 7, 1, 2, 3, 0, SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0, res, pred, 0);
    *d_ptr++ = res;
    
// CHECK: mov_dg unpack unpack_type=0 sdg0=1 sdg1=2 sdg2=3 sdg3=0 weg0=3 weg1=0 weg2=0 weg3=0 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7, %SP{{[0-9]+}}
    res = v_h8_mov_dual_group_unpack_b(xx, 15, 2, 3, 0, 1, SW_WR_UPPER_GROUP0, res, pred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg unpack unpack_type=0 sdg0=2 sdg1=3 sdg2=0 sdg3=1 weg0=2 weg1=0 weg2=0 weg3=0 %V{{[0-9]+}}, %V{{[0-9]+}}, 0xf, %SP{{[0-9]+}}

    res = v_h8_mov_dual_group_unpack_vb(xx, 1, 0, 1, 2, 3, SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg unpack unpack_type=0 sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=0 weg2=0 weg3=0 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x1, %VP{{[0-9]+}}

    res = v_h8_mov_dual_group_unpack_vb(xx, 7, 1, 2, 3, 0, SW_WR_LOWER_GROUP1 | SW_WR_UPPER_GROUP1, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg unpack unpack_type=0 sdg0=1 sdg1=2 sdg2=3 sdg3=0 weg0=0 weg1=3 weg2=0 weg3=0 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7, %VP{{[0-9]+}}

    res = v_h8_mov_dual_group_unpack_vb(xx, 15, 3, 0, 1, 2, SW_WR_UPPER_GROUP0 | SW_WR_UPPER_GROUP1, res, vpred, 0);
    *d_ptr++ = res;
// CHECK: mov_dg unpack unpack_type=0 sdg0=3 sdg1=0 sdg2=1 sdg3=2 weg0=2 weg1=2 weg2=0 weg3=0 %V{{[0-9]+}}, %V{{[0-9]+}}, 0xf, %VP{{[0-9]+}}
  }

}
