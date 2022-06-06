// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -main-function main_entry -target-cpu goya2 -verify %s
struct TT {
  float64 vec;
} var;

void main_entry(int I, int arr[], struct TT stvar, int64 vec, int64* vecptr, char ch, char* chptr, half hf, half* hfptr) // expected-error{{parameter 'arr' of entry function has wrong type}}
// expected-error@-1{{parameter 'stvar' of entry function has wrong type}}
// expected-error@-2{{parameter 'vec' of entry function has wrong type}}
// expected-error@-3{{parameter 'vecptr' of entry function has wrong type}}
// expected-error@-4{{parameter 'chptr' of entry function has wrong type}}
// expected-error@-5{{parameter 'hfptr' of entry function has wrong type}}
{
}
