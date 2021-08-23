// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s 

void main(int I, int* a) // expected-error{{parameter 'a' of entry function has wrong type}}
{
}
