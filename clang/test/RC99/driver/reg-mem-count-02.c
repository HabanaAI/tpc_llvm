// RUN: %tpc_clang -reg-mem-count %s -S -o - 2>&1 | FileCheck %s

void main(tensor tensor0, int src) {

}
//CHECK: Total VLM used: 0 bytes
