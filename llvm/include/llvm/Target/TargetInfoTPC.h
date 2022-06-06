#ifndef LLVM_TARGET_TARGETINFOTPC_H
#define LLVM_TARGET_TARGETINFOTPC_H

static const char *const DataLayoutStringTPC =
    "e"                 // TPC is little-endian CPU
    "-p0:32:32:32"      // Default address space is local, 32-bit address
    "-p1:32:32:32"      // Local scalar address space requires 32-bit address
    "-p2:32:32:32"      // Local vector address space requires 32-bit address
    "-p3:64:64:64"      // Global address space requires 64-bit address
    "-i32:32:32"        // Native integer is 32 bits
    "-n8:16:32"         // Native integer sizes
    "-f16:16:16"        // Half is 16 bits aligned at 32bit boundary (TODO: fix it)
    "-f32:32:32"        // Float is 32 bits
    "-v160:32:32"       // v5i32 are aligned on 32 bit boundary
    "-v256:2048:2048"   // 256 bit vectors (bool256) are aligned at 256 byte boundary
    "-v2048:2048:2048"  // 2048 bit vectors are aligned at 256 bytes
    "-v4096:2048:2048"  // double vectors (DRF) are aligned at 256 bytes
    "-v8192:2048:2048"  // quad vectors (ARF) are aligned at 256 bytes
  ;

#endif
