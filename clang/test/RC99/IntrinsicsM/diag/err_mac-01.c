// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 %s -verify

void main(signed char x0, _Bool pred) {
  float fres = 0.0;
  fres = s_f32_mac(x0, 1, fres, SW_SAT, 1, 0);      // expected-error{{type 'float' is incompatible with swicth SW_SAT}}
  fres = s_f32_mac(x0, 1, fres, 18, 1, 0);          // expected-error{{type 'float' is incompatible with swicth SW_X2_MAC}}

  int ires = 0;

  // Target dependent diagnostic.
  ires = s_i8_mac(x0, 1, ires, SW_NEG, 1, 0);       // todo: incompatible
}
