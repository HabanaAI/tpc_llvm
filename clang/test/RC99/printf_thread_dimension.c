
// RUN: tpc-clang -c -S -O1 -march=doron1 %s -o - | FileCheck %s

void main(int arg_int) {
// check printf of argument


 #pragma tpc_printf (enable)
    printf("%d\n", arg_int);
 }

//CHECK: mov  [[RES:%S[0-9]+]], %THREAD_ID
//CHECK: set_indx  %I2, b00100, [[RES]]
