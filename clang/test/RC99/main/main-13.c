// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -main-function main -target-cpu goya2 -verify %s

float main(int I, float f, half h) // expected-warning{{entry function must return 'void'}}
{
  return I + h + f; 
}
