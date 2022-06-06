// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck %s

void main(unsigned addr1, unsigned addr2, int dest, _Bool pred, int vpredp) {
  int64 __local *vector_ptr = (int64 __local *)dest;
  bool256 __local *vpred_ptr256 = (bool256 __local *)vpredp;
  bool256 vpred256 = *vpred_ptr256++;


  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)vector_ptr;
    minihalf256 res = 0;
    
    res = v_h8_ld_tnsr_high_direct_b(addr1, addr2, 0, res, pred, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr_high direct %V{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)vector_ptr;
    minihalf256 res = 0;
    
    res = v_h8_ld_tnsr_high_direct_vb(addr1, addr2, 0, res, vpred256, 0);
    *dest_ptr++ = res;
    vector_ptr = (int64 __local *)dest_ptr;
    // CHECK-DAG: ld_tnsr_high direct %V{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}, %VP{{[0-9]+}}
  }
}