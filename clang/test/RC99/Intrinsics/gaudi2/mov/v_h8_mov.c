// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi2 %s -o - | FileCheck --check-prefixes=GEN3 %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu doron1 %s -o - | FileCheck --check-prefixes=GEN3 %s


void main(int dest0, int dest1, int xi, short xs, char xc) {
  _Bool pred = xi < dest1;
  // CHECK-DAG: cmp_less.i32 [[PRED:%SP[0-9]+]], %S2, %S1

  volatile int64 __local *dptr = (int64 __local *)dest0;
  volatile int64 __local *sptr = (int64 __local *)dest1;


  // v_h8_mov_b
  {
    volatile minihalf256 __local *dfptr = (minihalf256 __local *)dptr;
    volatile minihalf256 __local *sfptr = (minihalf256 __local *)sptr;
    minihalf256 res = *sfptr++;
    minihalf256 x = *sfptr++;
    minihalf xscal = (minihalf)(xs);
    
    res = v_h8_mov_b(x, 0, res, pred, 0);
    // GEN3: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
    *dfptr++ = res;
    
    res = v_h8_mov_b(xscal, 0, res, pred, 0);
    // GEN3: mov.f8_152  %V{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
    *dfptr++ = res;
    
    res = v_h8_mov_b((minihalf)1.0, 0, res, pred, 0);
    // GEN3: mov.f8_152  %V{{[0-9]+}}, 0x3c, %SP{{[0-9]+}}
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_h8_mov_vb
  {
    volatile minihalf256 __local *dfptr = (minihalf256 __local *)dptr;
    volatile minihalf256 __local *sfptr = (minihalf256 __local *)sptr;
    bool256 vpred = v_h8_cmp_grt_b(*sfptr++, 0.0, 0, (bool256){0}, 1, 0);

    minihalf256 res = *sfptr++;
    minihalf256 x = *sfptr++;
    minihalf xscal = (minihalf)(xs);
   
    res = v_h8_mov_vb(x, 0, res, vpred, 0);
    // GEN3: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
    *dfptr++ = res;
   
    res = v_h8_mov_vb(xscal, 0, res, vpred, 0);
    // GEN3: mov.f8_152  %V{{[0-9]+}}, %S{{[0-9]+}}, %VP{{[0-9]+}}
    *dfptr++ = res;
   
    res = v_h8_mov_vb((minihalf)1.0, 0, res, vpred, 0);
    // GEN3: mov.f8_152  %V{{[0-9]+}}, 0x3c, %VP{{[0-9]+}}
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }
}
