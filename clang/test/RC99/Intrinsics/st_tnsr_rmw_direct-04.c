// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck %s

void main(unsigned addr1, unsigned addr2, int dest, _Bool pred, int rmw) {
  minihalf256 __local *vector_ptr_1 = (minihalf256 __local *)dest;

  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)vector_ptr_1;
    minihalf256 res = 0;

    v_h8_st_tnsr_rmw_direct(addr1, addr2, res, rmw, 0, pred, 0);
    *dest_ptr++ = res;
    vector_ptr_1 = (minihalf256 __local *)dest_ptr;
    // CHECK-DAG: st_tnsr rmw_sel direct %S{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
  }
}
