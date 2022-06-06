// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -float8 -target-cpu gaudi2 %s -o - | FileCheck --check-prefixes=GEN %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -float8 -target-cpu doron1 %s -o - | FileCheck --check-prefixes=GEN %s

void main(unsigned dest, _Bool pred, minifloat mfloat, minihalf mhalf) {
  s_f8_st_l(dest, mfloat, 0, pred, 0);
  s_h8_st_l(dest + 4, mhalf, 0, pred, 1);
}

// GEN: mov     [[PRED:%SP[0-9]+]], %S{{[0-9]+}}
// GEN-DAG: st_l    %S{{[0-9]+}}, %S{{[0-9]+}}, [[PRED]]
// GEN-DAG: st_l    %S{{[0-9]+}}, %S{{[0-9]+}}, ![[PRED]]
