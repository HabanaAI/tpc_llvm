// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -ast-dump %s -o - | FileCheck %s

struct New {
__local__ float * local_var_ptr;   // CHECK: FieldDecl {{.*}} local_var_ptr '__attribute__((address_space(1))) float *'
__local__ float64 * local_vec_ptr; // CHECK: FieldDecl {{.*}} local_vec_ptr '__attribute__((address_space(2))) float64 *'
};

__local__  struct New local_struc; // CHECK: VarDecl {{.*}} local_struc '__attribute__((address_space(1))) struct New':'__attribute__((address_space(1))) struct New'

__local__ int * __local locvar_locptr[4]; // CHECK: VarDecl {{.*}} locvar_locptr '__attribute__((address_space(1))) int *__attribute__((address_space(1))) [4]'
__local__ int64 * __local locvar_locvecptr[4]; // CHECK: VarDecl {{.*}} locvar_locvecptr '__attribute__((address_space(2))) int64 *__attribute__((address_space(1))) [4]'

void main() {
}

