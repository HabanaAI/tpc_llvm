// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck %s

void main() {
	thread_sync();
}

// CHECK: thread_sync
