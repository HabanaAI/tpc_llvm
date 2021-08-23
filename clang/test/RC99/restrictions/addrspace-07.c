// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -ast-dump %s -o - | FileCheck %s

__local__ int5 dcl_local_int5;
// CHECK: VarDecl {{.*}} dcl_local_int5 '__attribute__((address_space(1))) int5':'int __attribute__((address_space(1))) __attribute__((ext_vector_type(5)))'

int5 default_local_int5;
// CHECK: VarDecl {{.*}} default_local_int5 '__attribute__((address_space(1))) int5':'int __attribute__((address_space(1))) __attribute__((ext_vector_type(5)))'

void main() {
}
