// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -emit-llvm -O1 -main-function main_entry %s -o - | FileCheck %s
void main_entry(int arg0, tensor ifm, tensor ofm, int dest, tensor t1) {
}
// CHECK: define void @main_entry(i32 %arg0, i32 %dest) local_unnamed_addr #0 {
