// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -main-function main_entry -verify %s 

void main_entry(int I, int* a) // expected-error{{parameter 'a' of entry function has wrong type}}
{
}
