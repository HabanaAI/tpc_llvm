// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s
void main()
{
  int5 var;
   var.x = 5; //legal syntax
   var.yz = 10; //legal syntax
   var.xy = var.zq * 2; //legal syntax
   var.xyz = var.wq; // expected-error{{assigning to 'int __attribute__((ext_vector_type(3)))' (vector of 3 'int' values) from incompatible type 'int __attribute__((ext_vector_type(2)))' (vector of 2 'int' values)}}
   var.zxq = var.xxy; // legal syntax
   var.xx = 2;  // expected-error{{vector is not assignable (contains duplicate components)}}
   // illegal - 'x' used twice
}             