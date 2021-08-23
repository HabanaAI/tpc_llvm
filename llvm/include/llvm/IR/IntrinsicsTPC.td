//===- IntrinsicsTPC.td - Defines TPC intrinsics ---*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines all of the TPC-specific intrinsics.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Definitions for all TPC intrinsics.
//

class GlobalPointerType<LLVMType elty>
  : LLVMQualPointerType<elty, 3>;

def global_ptr_ty   : GlobalPointerType<llvm_i8_ty>;             // i8*


let TargetPrefix = "tpc" in {  // All intrinsics start with "llvm.tpc.".

def int_tpc_get_index_space_offset : Intrinsic<[llvm_v5i32_ty], [], [IntrNoMem]>;
def int_tpc_get_index_space_size   : Intrinsic<[llvm_v5i32_ty], [], [IntrNoMem]>;

def int_tpc_read_lfsr              : Intrinsic<[llvm_v256i8_ty], [], []>;
def int_tpc_write_lfsr             : Intrinsic<[], [llvm_v256i8_ty], []>;
def int_tpc_read_lfsrnc            : Intrinsic<[llvm_v256i8_ty], [], [IntrNoMem]>;

def int_tpc_s_read_lfsr              : Intrinsic<[llvm_i32_ty], [], []>;
def int_tpc_write_lfsr_s             : Intrinsic<[], [llvm_i32_ty], []>;
def int_tpc_s_read_lfsrnc            : Intrinsic<[llvm_i32_ty], [], [IntrNoMem]>;


//------ MOVs ------------------------------------------------------------------

def int_tpc_mov_flavor    : Intrinsic<[llvm_v256i1_ty],
                                      [ llvm_i32_ty,        // X
                                        llvm_i32_ty,        // Flavor
                                        llvm_i32_ty,        // Switches
                                        llvm_v256i1_ty,     // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_mov_mask      : Intrinsic<[llvm_v5i32_ty],
                                      [ llvm_any_ty,        // X, may be v5i32 or i32
                                        llvm_i32_ty,        // DimMask
                                        llvm_i32_ty,        // Switches
                                        llvm_v5i32_ty,      // Income
                                        llvm_i1_ty,         // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_mov           : Intrinsic<[llvm_any_ty],
                                      [ llvm_any_ty,        // X
                                        llvm_i8_ty,         // Data Type
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;

def int_tpc_get_addr      : Intrinsic<[llvm_v2i32_ty],
                                      [ global_ptr_ty,      // Ptr
                                        llvm_i32_ty,        // Switches
                                        llvm_v2i32_ty,      // Income
                                        llvm_i1_ty,         // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrReadMem]>;

def int_tpc_update_addr   : Intrinsic<[global_ptr_ty],
                                      [ global_ptr_ty,      // Ptr
                                        llvm_v2i32_ty,      // Addr
                                        llvm_i32_ty,        // Switches
                                        llvm_i1_ty,         // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrWriteMem]>;

//------ Load slot -------------------------------------------------------------

def int_tpc_gen_addr      : Intrinsic<[global_ptr_ty],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_i32_ty, global_ptr_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_event         : Intrinsic<[],
                                      [llvm_i16_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      []>;
def int_tpc_prmt_indx     : Intrinsic<[llvm_v5i32_ty],
                                      [llvm_v5i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_v5i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_set_indx      : Intrinsic<[llvm_v5i32_ty],
                                      [llvm_v5i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_ld_l          : Intrinsic<[llvm_any_ty],
                                      [llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_i1_ty, llvm_i1_ty],
                                      [IntrReadMem]>;
def int_tpc_ld_g          : Intrinsic<[llvm_any_ty],
                                      [global_ptr_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrReadMem]>;
def int_tpc_ld_g_inc      : Intrinsic<[llvm_any_ty, llvm_any_ty],
                                      [LLVMMatchType<1>, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_i1_ty, llvm_i1_ty],
                                      [IntrReadMem, IntrArgMemOnly]>;
def int_tpc_ld_g_int5     : Intrinsic<[llvm_v5i32_ty],
                                      [global_ptr_ty, llvm_i32_ty, llvm_i32_ty, llvm_v5i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrReadMem]>;
def int_tpc_ld_g_int5_inc : Intrinsic<[llvm_v5i32_ty, global_ptr_ty],
                                      [global_ptr_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_v5i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrReadMem, IntrArgMemOnly]>;
def int_tpc_ld_g_partial  : Intrinsic<[llvm_anyvector_ty],
                                      [global_ptr_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_ld_g_partial_inc : Intrinsic<[llvm_any_ty, llvm_any_ty],
                                         [LLVMMatchType<1>, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                         [IntrReadMem, IntrArgMemOnly]>;
def int_tpc_prefetch      : Intrinsic<[],
                                      [ global_ptr_ty,      // Address
                                        llvm_i32_ty,        // Switches
                                        llvm_i1_ty,         // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrInaccessibleMemOrArgMemOnly]>;
def int_tpc_ld_l_v        : Intrinsic<[llvm_anyvector_ty],
                                      [ llvm_i32_ty,        // Address
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrReadMem]>;
def int_tpc_ld_l_v_low    : Intrinsic<[llvm_anyvector_ty],
                                      [ llvm_i32_ty,        // Address
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrReadMem]>;
def int_tpc_ld_l_v_high   : Intrinsic<[llvm_anyvector_ty],
                                      [ llvm_i32_ty,        // Address
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrReadMem]>;
def int_tpc_ld_tnsr       : Intrinsic<[llvm_anyvector_ty],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_ld_tnsr_direct : Intrinsic<[llvm_anyvector_ty],
                                      [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_ld_tnsr_partial : Intrinsic<[llvm_anyvector_ty],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_ld_tnsr_partial_direct : Intrinsic<[llvm_anyvector_ty],
                                      [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_ld_tnsr_low   : Intrinsic<[llvm_anyvector_ty],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_ld_tnsr_low_direct   : Intrinsic<[llvm_anyvector_ty],
                                      [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_ld_tnsr_low_partial : Intrinsic<[llvm_anyvector_ty],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_ld_tnsr_low_partial_direct : Intrinsic<[llvm_anyvector_ty],
                                      [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_ld_tnsr_high  : Intrinsic<[llvm_anyvector_ty],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_ld_tnsr_high_direct  : Intrinsic<[llvm_anyvector_ty],
                                      [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_ld_tnsr_high_partial : Intrinsic<[llvm_anyvector_ty],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_ld_tnsr_high_partial_direct : Intrinsic<[llvm_anyvector_ty],
                                      [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_lookup        : Intrinsic<[llvm_anyvector_ty],
                                      [llvm_any_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_i1_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_lookup_c0     : Intrinsic<[llvm_anyvector_ty],
                                      [llvm_any_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_i1_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_lookup_c1c2   : Intrinsic<[llvm_anyvector_ty],
                                      [llvm_any_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_i1_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_lookup_1c     : Intrinsic<[llvm_anyvector_ty],
                                      [llvm_any_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_i1_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_lookup_2c     : Intrinsic<[llvm_anyvector_ty],
                                      [llvm_any_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_i1_ty, llvm_i1_ty],
                                      [IntrNoMem]>;

//------ Scalar slot, also instructions common for scalar and vector slots.

def int_tpc_mac           : Intrinsic<[llvm_any_ty],
                                      [llvm_any_ty, LLVMMatchType<1>, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;

def int_tpc_mac_zp        : Intrinsic<[llvm_any_ty],
                                      [llvm_any_ty,         // srcA
                                       LLVMMatchType<1>,    // srcB
                                       LLVMMatchType<1>,    // ZP
                                       llvm_i8_ty,          // Data Type
                                       llvm_i32_ty,         // Switches
                                       LLVMMatchType<0>,    // Income
                                       llvm_any_ty,         // Predicate (i1/v256i1)
                                       llvm_i1_ty           // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_mac_x2        : Intrinsic<[llvm_any_ty],
                                      [llvm_any_ty,         // srcA
                                       llvm_any_ty,         // srcB
                                       LLVMMatchType<1>,    // srcC or StoreSrcC
                                       llvm_any_ty,         // srcD
                                       llvm_i8_ty,          // Data Type
                                       llvm_i32_ty,         // Switches
                                       LLVMMatchType<0>,    // Income
                                       llvm_any_ty,         // Predicate (i1/v256i1)
                                       llvm_i1_ty           // Polarity
                                      ],
                                      [IntrNoMem]>;

def int_tpc_mac_x2_f32    : Intrinsic<[llvm_any_ty],
                                      [llvm_any_ty,         // srcA
                                       llvm_any_ty,         // srcB
                                       llvm_any_ty,         // srcD
                                       llvm_i8_ty,          // Data Type
                                       llvm_i32_ty,         // Switches
                                       LLVMMatchType<0>,    // Income
                                       llvm_any_ty,         // Predicate (i1/v256i1)
                                       llvm_i1_ty           // Polarity
                                      ],
                                      [IntrNoMem]>;

def int_tpc_mac_x2_zp      : Intrinsic<[llvm_any_ty],
                                       [llvm_any_ty,         // srcA
                                        LLVMMatchType<1>,    // srcB
                                        LLVMMatchType<1>,    // srcC
                                        LLVMMatchType<1>,    // srcD
                                        LLVMMatchType<1>,    // ZP
                                        llvm_i8_ty,          // Data Type
                                        llvm_i32_ty,         // Switches
                                        LLVMMatchType<0>,    // Income
                                        llvm_any_ty,         // Predicate (i1/v256i1)
                                        llvm_i1_ty           // Polarity
                                       ],
                                       [IntrNoMem]>;

def int_tpc_madd           : Intrinsic<[llvm_any_ty],
                                      [llvm_any_ty,         // srcA
                                       LLVMMatchType<1>,    // srcB
                                       LLVMMatchType<0>,    // srcC
                                       llvm_i8_ty,          // Data Type
                                       llvm_i32_ty,         // Switches
                                       LLVMMatchType<0>,    // Income
                                       llvm_any_ty,         // Predicate (i1/v256i1)
                                       llvm_i1_ty           // Polarity
                                      ],
                                      [IntrNoMem]>;

def int_tpc_madd_zp        : Intrinsic<[llvm_any_ty],
                                      [llvm_any_ty,         // srcA
                                       LLVMMatchType<1>,    // srcB
                                       LLVMMatchType<0>,    // srcC
                                       LLVMMatchType<1>,    // ZP
                                       llvm_i8_ty,          // Data Type
                                       llvm_i32_ty,         // Switches
                                       LLVMMatchType<0>,    // Income
                                       llvm_any_ty,         // Predicate (i1/v256i1)
                                       llvm_i1_ty           // Polarity
                                      ],
                                      [IntrNoMem]>;

def int_tpc_madd_x2        : Intrinsic<[llvm_any_ty],
                                       [ llvm_any_ty,        // srcA
                                         llvm_any_ty,        // srcB
                                         llvm_any_ty,        // srcC
                                         llvm_any_ty,        // srcD
                                         llvm_i8_ty,         // Data Type
                                         llvm_i32_ty,        // Switches
                                         LLVMMatchType<0>,   // Income
                                         llvm_any_ty,        // Predicate (i1/v256i1)
                                         llvm_i1_ty          // Polarity
                                       ],
                                       [IntrNoMem]>;

// For instructions that support operations over IRF values we need to introduce
// new intrinsics to support Gen3. These are 'int_tpc_*_mask'. In the case of
// Gen1 and Gen2 dimmask is a constant, so it can be combined with switches. It
// is not the case for Gen3 anymore, where dimmask may be an expression, so
// previous versions of intrinsics were not able to represent non-constant
// dimmask.

def int_tpc_mul           : Intrinsic<[llvm_any_ty],
                                      [ llvm_any_ty,        // X
                                        llvm_any_ty,        // Y, may differ from X, in V+S variants
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_mul_x2_f32    : Intrinsic<[llvm_any_ty],
                                      [ llvm_any_ty,         // srcA
                                        llvm_any_ty,         // srcB
                                        llvm_any_ty,         // srcD
                                        llvm_i8_ty,          // Data Type
                                        llvm_i32_ty,         // Switches
                                        LLVMMatchType<0>,    // Income
                                        llvm_any_ty,         // Predicate (i1/v256i1)
                                        llvm_i1_ty           // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_mul_mask      : Intrinsic<[llvm_v5i32_ty],
                                      [ llvm_any_ty,        // X
                                        llvm_any_ty,        // Y, may differ from X, in V+S variants
                                        llvm_i32_ty,        // DimMask
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        llvm_v5i32_ty,      // Income
                                        llvm_i1_ty,         // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_add           : Intrinsic<[llvm_any_ty],
                                      [ llvm_any_ty,        // X
                                        llvm_any_ty,        // Y, may differ from X, in V+S variants
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_add_x2        : Intrinsic<[llvm_any_ty],
                                      [ LLVMMatchType<0>,   // SrcA
                                        llvm_any_ty,        // SrcB
                                        llvm_any_ty,        // SrcD
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_add_mask      : Intrinsic<[llvm_v5i32_ty],
                                      [ llvm_any_ty,        // X, may be scalar (including imm)
                                        llvm_any_ty,        // Y, may be scalar (including imm)
                                        llvm_i32_ty,        // DimMask
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        llvm_v5i32_ty,      // Income
                                        llvm_i1_ty,         // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_sub           : Intrinsic<[llvm_any_ty],        // VRF, IRF or SRF type
                                      [ llvm_any_ty,        // X, may differ from result in IRF operations
                                        llvm_any_ty,        // Y, may differ from X, in V+S variants
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_sub_x2        : Intrinsic<[llvm_any_ty],
                                      [ LLVMMatchType<0>,   // SrcA
                                        llvm_any_ty,        // SrcB
                                        llvm_any_ty,        // SrcD
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_sub_mask      : Intrinsic<[llvm_v5i32_ty],
                                      [ llvm_any_ty,        // X, may be scalar (including imm)
                                        llvm_any_ty,        // Y
                                        llvm_i32_ty,        // DimMask
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        llvm_v5i32_ty,      // Income
                                        llvm_i1_ty,         // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_max           : Intrinsic<[llvm_any_ty],
                                      [ llvm_any_ty,        // X
                                        llvm_any_ty,        // Y, may differ from X, in V+S variants
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_max_mask      : Intrinsic<[llvm_v5i32_ty],
                                      [ llvm_any_ty,        // X, may be scalar (including imm)
                                        llvm_any_ty,        // Y, may be scalar (including imm)
                                        llvm_i32_ty,        // DimMask
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        llvm_v5i32_ty,      // Income
                                        llvm_i1_ty,         // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_min           : Intrinsic<[llvm_any_ty],
                                      [ llvm_any_ty,        // X
                                        llvm_any_ty,        // Y, may differ from X, in V+S variants
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_min_mask      : Intrinsic<[llvm_v5i32_ty],
                                      [ llvm_any_ty,        // X, may be scalar (including imm)
                                        llvm_any_ty,        // Y, may be scalar (including imm)
                                        llvm_i32_ty,        // DimMask
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        llvm_v5i32_ty,      // Income
                                        llvm_i1_ty,         // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_abs           : Intrinsic<[llvm_any_ty],        // VRF, IRF or SRF type
                                      [ llvm_any_ty,        // X, may differ from result in IRF operations
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_abs_mask      : Intrinsic<[llvm_v5i32_ty],
                                      [ llvm_any_ty,        // X, may differ from result in IRF operations
                                        llvm_i32_ty,        // DimMask
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        llvm_v5i32_ty,      // Income
                                        llvm_i1_ty,         // Predicate
                                        llvm_i1_ty],        // Polarity
                                      [IntrNoMem]>;
def int_tpc_cmp_eq        : Intrinsic<[llvm_any_ty],        // v256i1 or i1
                                      [ llvm_any_ty,        // X, may be scalar (including imm)
                                        llvm_any_ty,        // Y, may be scalar (including imm)
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1 or v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_cmp_eq_mask   : Intrinsic<[llvm_i1_ty],
                                      [ llvm_any_ty,        // X, may be scalar (including imm)
                                        llvm_any_ty,        // Y, may be scalar (including imm)
                                        llvm_i32_ty,        // DimMask
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        llvm_i1_ty,         // Income
                                        llvm_i1_ty,         // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_cmp_neq       : Intrinsic<[llvm_any_ty],        // v256i1 or i1
                                      [ llvm_any_ty,        // X, may be scalar (including imm)
                                        llvm_any_ty,        // Y, may be scalar (including imm)
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1 or v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_cmp_neq_mask  : Intrinsic<[llvm_i1_ty],
                                      [ llvm_any_ty,        // X, may be scalar (including imm)
                                        llvm_any_ty,        // Y, may be scalar (including imm)
                                        llvm_i32_ty,        // DimMask
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        llvm_i1_ty,         // Income
                                        llvm_i1_ty,         // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_cmp_less      : Intrinsic<[llvm_any_ty],        // v256i1 or i1
                                      [ llvm_any_ty,        // X, may be scalar (including imm)
                                        llvm_any_ty,        // Y, may be scalar (including imm)
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1 or v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_cmp_less_mask : Intrinsic<[llvm_i1_ty],
                                      [ llvm_any_ty,        // X, may be scalar (including imm)
                                        llvm_any_ty,        // Y, may be scalar (including imm)
                                        llvm_i32_ty,        // DimMask
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        llvm_i1_ty,         // Income
                                        llvm_i1_ty,         // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_cmp_leq       : Intrinsic<[llvm_any_ty],        // v256i1 or i1
                                      [ llvm_any_ty,        // X, may be scalar (including imm)
                                        llvm_any_ty,        // Y, may be scalar (including imm)
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1 or v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_cmp_leq_mask  : Intrinsic<[llvm_i1_ty],
                                      [ llvm_any_ty,        // X, may be scalar (including imm)
                                        llvm_any_ty,        // Y, may be scalar (including imm)
                                        llvm_i32_ty,        // DimMask
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        llvm_i1_ty,         // Income
                                        llvm_i1_ty,         // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_cmp_grt       : Intrinsic<[llvm_any_ty],        // v256i1 or i1
                                      [ llvm_any_ty,        // X, may be scalar (including imm)
                                        llvm_any_ty,        // Y, may be scalar (including imm)
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1 or v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_cmp_grt_mask  : Intrinsic<[llvm_i1_ty],
                                      [ llvm_any_ty,        // X, may be scalar (including imm)
                                        llvm_any_ty,        // Y, may be scalar (including imm)
                                        llvm_i32_ty,        // DimMask
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        llvm_i1_ty,         // Income
                                        llvm_i1_ty,         // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_cmp_geq       : Intrinsic<[llvm_any_ty],        // v256i1 or i1
                                      [ llvm_any_ty,        // X, may be scalar (including imm)
                                        llvm_any_ty,        // Y, may be scalar (including imm)
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1 or v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_cmp_geq_mask  : Intrinsic<[llvm_i1_ty],
                                      [ llvm_any_ty,        // X, may be scalar (including imm)
                                        llvm_any_ty,        // Y, may be scalar (including imm)
                                        llvm_i32_ty,        // DimMask
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        llvm_i1_ty,         // Income
                                        llvm_i1_ty,         // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_or            : Intrinsic<[llvm_any_ty],
                                      [ llvm_any_ty,        // X
                                        llvm_any_ty,        // Y, may differ from X, in V+S variants
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_or_mask       : Intrinsic<[llvm_v5i32_ty],
                                      [ llvm_any_ty,        // X, may be scalar (including imm)
                                        llvm_any_ty,        // Y, may be scalar (including imm)
                                        llvm_i32_ty,        // DimMask
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        llvm_v5i32_ty,      // Income
                                        llvm_i1_ty,         // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_and           : Intrinsic<[llvm_any_ty],
                                      [ llvm_any_ty,        // X
                                        llvm_any_ty,        // Y, may differ from X, in V+S variants
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_and_mask      : Intrinsic<[llvm_v5i32_ty],
                                      [ llvm_any_ty,        // X, may be scalar (including imm)
                                        llvm_any_ty,        // Y, may be scalar (including imm)
                                        llvm_i32_ty,        // DimMask
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        llvm_v5i32_ty,      // Income
                                        llvm_i1_ty,         // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_xor           : Intrinsic<[llvm_any_ty],
                                      [ llvm_any_ty,        // X
                                        llvm_any_ty,        // Y, may differ from X, in V+S variants
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_xor_mask      : Intrinsic<[llvm_v5i32_ty],
                                      [ llvm_any_ty,        // X, may be scalar (including imm)
                                        llvm_any_ty,        // Y, may be scalar (including imm)
                                        llvm_i32_ty,        // DimMask
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        llvm_v5i32_ty,      // Income
                                        llvm_i1_ty,         // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_not           : Intrinsic<[llvm_any_ty],
                                      [ llvm_any_ty,        // X, may differ from the result in IRF operations
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_not_mask      : Intrinsic<[llvm_v5i32_ty],
                                      [ llvm_any_ty,        // X, may be v5i32 or i32
                                        llvm_i32_ty,        // DimMask
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        llvm_v5i32_ty,      // Income
                                        llvm_i1_ty,         // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_shr           : Intrinsic<[llvm_any_ty],
                                      [ LLVMMatchType<0>,   // Value to shift
                                        llvm_any_ty,        // Number of bits, may be vector or scalar
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_shr_mask      : Intrinsic<[llvm_v5i32_ty],
                                      [ llvm_v5i32_ty,      // Value to shift
                                        llvm_any_ty,        // Number of bits, may be i8/v5i32
                                        llvm_i32_ty,        // DimMask
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        llvm_v5i32_ty,      // Income
                                        llvm_i1_ty,         // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_shl           : Intrinsic<[llvm_any_ty],
                                      [ LLVMMatchType<0>,   // Value to shift
                                        llvm_any_ty,        // Number of bits, may be vector or scalar
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_shl_mask      : Intrinsic<[llvm_v5i32_ty],
                                      [ llvm_v5i32_ty,      // Value to shift
                                        llvm_any_ty,        // Number of bits, may be i8/v5i32
                                        llvm_i32_ty,        // DimMask
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        llvm_v5i32_ty,      // Income
                                        llvm_i1_ty,         // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;

def int_tpc_ash           : Intrinsic<[llvm_anyint_ty],
                                      [ llvm_anyint_ty,    // Value to shift
                                        llvm_anyint_ty,    // Shift value (i8/v256i8)
                                        llvm_i8_ty,        // DataType
                                        llvm_i32_ty,       // Switches
                                        LLVMMatchType<0>,  // Income
                                        llvm_any_ty,       // Predicate (i1/v256i1)
                                        llvm_i1_ty         // Polarity
                                      ],
                                      [IntrNoMem]>;

def int_tpc_convert_int64   : Intrinsic<[llvm_any_ty],
                                      [ llvm_any_ty,       // half1
                                        llvm_any_ty,       // half2
                                        llvm_i32_ty,       // Switches
                                        LLVMMatchType<0>,  // Income
                                        llvm_any_ty,       // Predicate
                                        llvm_i1_ty         // Polarity
                                      ],
                                      [IntrNoMem]>;

def int_tpc_convert       : Intrinsic<[llvm_any_ty],
                                      [ llvm_any_ty,       // Value to convert
                                        llvm_i8_ty,        // DataType
                                        llvm_i32_ty,       // Switches
                                        LLVMMatchType<0>,  // Income
                                        llvm_any_ty,       // Predicate (i1/v256i1)
                                        llvm_i1_ty         // Polarity
                                      ],
                                      [IntrNoMem]>;

def int_tpc_convert_int   : Intrinsic<[llvm_any_ty],
                                      [ llvm_any_ty,       // Value to convert
                                        llvm_any_ty,       // Shift value (char/char256)
                                        llvm_i32_ty,       // Switches
                                        LLVMMatchType<0>,  // Income
                                        llvm_any_ty,       // Predicate (i1/v256i1)
                                        llvm_i1_ty         // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_convert_uint  : Intrinsic<[llvm_any_ty],
                                      [ llvm_any_ty,       // Value to convert
                                        llvm_any_ty,       // Shift value (char/char256)
                                        llvm_i32_ty,       // Switches
                                        LLVMMatchType<0>,  // Income
                                        llvm_any_ty,       // Predicate (i1/v256i1)
                                        llvm_i1_ty         // Polarity
                                      ],
                                      [IntrNoMem]>;

// Floating point rounding mode intrinsics
def int_tpc_fptrunc_swch  : Intrinsic<[llvm_anyfloat_ty],
                                      [ llvm_anyfloat_ty,       // Value to convert
                                        llvm_i32_ty,            // Switches
                                      ],
                                      [IntrNoMem]>;

def int_tpc_fpext_swch  : Intrinsic<[llvm_anyfloat_ty],
                                      [ llvm_anyfloat_ty,       // Value to convert
                                        llvm_i32_ty,            // Switches
                                      ],
                                      [IntrNoMem]>;

def int_tpc_sitofp_swch  : Intrinsic<[llvm_anyfloat_ty],
                                      [ llvm_anyint_ty,       // Value to convert
                                        llvm_i32_ty,          // Switches
                                      ],
                                      [IntrNoMem]>;

def int_tpc_fptosi_swch  : Intrinsic<[llvm_anyint_ty],
                                      [ llvm_anyfloat_ty,      // Value to convert
                                        llvm_i32_ty,           // Switches
                                      ],
                                      [IntrNoMem]>;

def int_tpc_uitofp_swch  : Intrinsic<[llvm_anyfloat_ty],
                                      [ llvm_anyint_ty,       // Value to convert
                                        llvm_i32_ty,          // Switches
                                      ],
                                      [IntrNoMem]>;

def int_tpc_fptoui_swch  : Intrinsic<[llvm_anyint_ty],
                                      [ llvm_anyfloat_ty,     // Value to convert
                                        llvm_i32_ty,          // Switches
                                      ],
                                      [IntrNoMem]>;

def int_tpc_popcnt        : Intrinsic<[llvm_any_ty],
                                      [llvm_any_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_find_first    : Intrinsic<[llvm_any_ty],
                                      [llvm_any_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_nearbyint     : Intrinsic<[llvm_any_ty],
                                      [ llvm_any_ty,       // Value to convert
                                        llvm_i8_ty,        // DataType
                                        llvm_i32_ty,       // Switches
                                        LLVMMatchType<0>,  // Income
                                        llvm_any_ty,       // Predicate (i1/v256i1)
                                        llvm_i1_ty         // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_fclass        : Intrinsic<[llvm_any_ty],
                                      [ LLVMMatchType<0>,
                                        llvm_i8_ty,        // DataType
                                        llvm_i32_ty,       // Switches
                                        LLVMMatchType<0>,  // Income
                                        llvm_any_ty,       // Predicate (i1/v256i1)
                                        llvm_i1_ty         // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_fclass_limit  : Intrinsic<[llvm_any_ty],
                                      [ LLVMMatchType<0>,
                                        LLVMMatchType<0>,
                                        LLVMMatchType<0>,
                                        llvm_i8_ty,        // DataType
                                        llvm_i32_ty,       // Switches
                                        LLVMMatchType<0>,  // Income
                                        llvm_any_ty,       // Predicate (i1/v256i1)
                                        llvm_i1_ty         // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_extract_exp   : Intrinsic<[llvm_any_ty],
                                      [llvm_any_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_brev          : Intrinsic<[llvm_any_ty],
                                      [llvm_any_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_mov_irf_dim   : Intrinsic<[llvm_any_ty],
                                      [ llvm_v5i32_ty,      // IRF
                                        llvm_i8_ty,         // Dim
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_i1_ty,         // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_udiv_step     : Intrinsic<[llvm_any_ty],
                                      [llvm_anyint_ty, llvm_i32_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_i1_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_udiv          : Intrinsic<[llvm_any_ty],
                                      [llvm_anyint_ty, LLVMMatchType<1>, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_i1_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_calc_fp_special : Intrinsic<[llvm_any_ty],
                                      [LLVMMatchType<0>, LLVMMatchType<0>, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;

def int_tpc_convert_fp_flex  : Intrinsic<[llvm_any_ty],
                                         [ llvm_any_ty,
                                           llvm_any_ty,       // i16 or v256i16
                                           llvm_i8_ty,        // DataType
                                           llvm_i32_ty,       // Switches
                                           LLVMMatchType<0>,  // Income
                                           llvm_any_ty,       // Predicate (i1/v256i1)
                                           llvm_i1_ty         // Polarity
                                         ],
                                          [IntrNoMem]>;

//------ Vector slot

def int_tpc_shuffle       : Intrinsic<[llvm_any_ty],
                                      [LLVMMatchType<0>, llvm_v256i8_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_pack          : Intrinsic<[llvm_any_ty],
                                      [LLVMMatchType<0>, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_unpack        : Intrinsic<[llvm_any_ty],
                                      [LLVMMatchType<0>, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_get_lut_entry : Intrinsic<[llvm_any_ty],
                                      [llvm_any_ty, llvm_i8_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_form_fp_num   : Intrinsic<[llvm_any_ty],
                                      [llvm_any_ty, LLVMMatchType<0>, LLVMMatchType<0>, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_mov_group     : Intrinsic<[llvm_any_ty],
                                      [llvm_any_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_mov_dual_group : Intrinsic<[llvm_anyvector_ty],
                                      [ LLVMMatchType<0>,  // Source
                                        llvm_i32_ty,       // ByteEn (encoded in immediate)
                                        llvm_i32_ty,       // Switches
                                        LLVMMatchType<0>,  // Income value
                                        llvm_any_ty,       // Predicate (i1/v256i1)
                                        llvm_i1_ty         // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_mov_dual_group_all : Intrinsic<[llvm_anyvector_ty],
                                      [ LLVMMatchType<0>,  // Source
                                        llvm_i32_ty,       // ByteEn (encoded in immediate)
                                        llvm_i32_ty,       // Switches
                                        LLVMMatchType<0>,  // Income value
                                        llvm_any_ty,       // Predicate (i1/v256i1)
                                        llvm_i1_ty         // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_mov_dual_group_pack : Intrinsic<[llvm_anyvector_ty],
                                      [ LLVMMatchType<0>,  // Source
                                        llvm_i32_ty,       // Switches
                                        LLVMMatchType<0>,  // Income value
                                        llvm_any_ty,       // Predicate (i1/v256i1)
                                        llvm_i1_ty         // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_mov_dual_group_unpack : Intrinsic<[llvm_anyvector_ty],
                                      [ LLVMMatchType<0>,  // Source
                                        llvm_i32_ty,       // ByteEn (encoded in immediate)
                                        llvm_i32_ty,       // Switches
                                        LLVMMatchType<0>,  // Income value
                                        llvm_any_ty,       // Predicate (i1/v256i1)
                                        llvm_i1_ty         // Polarity
                                      ],
                                      [IntrNoMem]>;
                          // v64i32 SEL_EQ(v64f32:a, [v64f32, f32]:b, v64i32:c, [v64i32, i32]:d, int8 dt, int sw, v64i32 income, [v256i1, i1]:pred, i1 polarity)
def int_tpc_sel_eq        : Intrinsic<[llvm_any_ty],
                                      [llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_sel_neq       : Intrinsic<[llvm_any_ty],
                                      [llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_sel_less      : Intrinsic<[llvm_any_ty],
                                      [llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_sel_leq       : Intrinsic<[llvm_any_ty],
                                      [llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_sel_grt       : Intrinsic<[llvm_any_ty],
                                      [llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_sel_geq       : Intrinsic<[llvm_any_ty],
                                      [llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;

                          // (2 x v64i32) SEL2_LESS(v64f32:a, [v64f32, f32]:b, v64i32:c, [v64i32, i32]:d, int8 dt, int sw, v64i32 income, [v256i1, i1]:pred, i1 polarity)
def int_tpc_sel2_less     : Intrinsic<[llvm_any_ty],
                                      [llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_sel2_leq      : Intrinsic<[llvm_any_ty],
                                      [llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_sel2_grt      : Intrinsic<[llvm_any_ty],
                                      [llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_sel2_geq      : Intrinsic<[llvm_any_ty],
                                      [llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_msac          : Intrinsic<[llvm_anyvector_ty],
                                      [ LLVMMatchType<0>,   // A
                                        llvm_any_ty,        // B - may be a vector or a scalar
                                        llvm_v256i8_ty,     // C
                                        llvm_any_ty,        // D - may v256i8 or i8
                                        llvm_i8_ty,         // Datatype
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;

//------ Store slot

def int_tpc_st_l          : Intrinsic<[],
                                      [llvm_i32_ty, llvm_any_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_g          : Intrinsic<[],
                                      [global_ptr_ty, llvm_any_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem, IntrArgMemOnly]>;
def int_tpc_st_g_inc      : Intrinsic<[llvm_any_ty],
                                      [LLVMMatchType<0>, llvm_any_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem, IntrArgMemOnly]>;
def int_tpc_st_l_v        : Intrinsic<[],
                                      [ llvm_i32_ty,        // Address
                                        llvm_anyvector_ty,  // Value
                                        llvm_i32_ty,        // Switches
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrWriteMem]>;
def int_tpc_st_l_v_ofs    : Intrinsic<[],
                                      [ llvm_i32_ty,        // Base
                                        llvm_i32_ty,        // Offset
                                        llvm_anyvector_ty,  // Value
                                        llvm_i32_ty,        // Switches
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrWriteMem]>;
def int_tpc_st_l_v_low    : Intrinsic<[],
                                      [ llvm_i32_ty,        // Address
                                        llvm_anyvector_ty,  // Value
                                        llvm_i32_ty,        // Switches
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrWriteMem]>;
def int_tpc_st_l_v_low_ofs : Intrinsic<[],
                                      [ llvm_i32_ty,        // Base
                                        llvm_i32_ty,        // Offset
                                        llvm_anyvector_ty,  // Value
                                        llvm_i32_ty,        // Switches
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrWriteMem]>;
def int_tpc_st_l_v_high   : Intrinsic<[],
                                      [ llvm_i32_ty,        // Address
                                        llvm_anyvector_ty,  // Value
                                        llvm_i32_ty,        // Switches
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrWriteMem]>;
def int_tpc_st_l_v_high_ofs : Intrinsic<[],
                                      [ llvm_i32_ty,        // Base
                                        llvm_i32_ty,        // Offset
                                        llvm_anyvector_ty,  // Value
                                        llvm_i32_ty,        // Switches
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrWriteMem]>;
def int_tpc_aso           : Intrinsic<[],
                                      [llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      []>;
def int_tpc_st_tnsr       : Intrinsic<[],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_tnsr_direct : Intrinsic<[],
                                      [llvm_i32_ty, llvm_i32_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_tnsr_partial : Intrinsic<[],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_tnsr_partial_direct : Intrinsic<[],
                                      [llvm_i32_ty, llvm_i32_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_tnsr_rmw   : Intrinsic<[],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_tnsr_rmw_direct   : Intrinsic<[],
                                      [llvm_i32_ty, llvm_i32_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_tnsr_partial_rmw : Intrinsic<[],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_tnsr_partial_rmw_direct : Intrinsic<[],
                                      [llvm_i32_ty, llvm_i32_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_tnsr_low   : Intrinsic<[],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_tnsr_low_direct   : Intrinsic<[],
                                      [llvm_i32_ty, llvm_i32_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_tnsr_low_rmw : Intrinsic<[],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_tnsr_low_rmw_direct : Intrinsic<[],
                                      [llvm_i32_ty, llvm_i32_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_tnsr_high  : Intrinsic<[],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_tnsr_high_direct  : Intrinsic<[],
                                      [llvm_i32_ty, llvm_i32_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_tnsr_high_rmw : Intrinsic<[],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_tnsr_high_rmw_direct : Intrinsic<[],
                                      [llvm_i32_ty, llvm_i32_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;

def int_tpc_st_tnsr_sqz       : Intrinsic<[llvm_i32_ty],
                                          [llvm_v5i32_ty, llvm_i8_ty, llvm_i32_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_v256i1_ty, llvm_i1_ty],
                                          [IntrWriteMem]>;
def int_tpc_st_tnsr_sqz_rmw   : Intrinsic<[llvm_i32_ty],
                                          [llvm_v5i32_ty, llvm_i8_ty, llvm_i32_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i32_ty, llvm_v256i1_ty, llvm_i1_ty],
                                          [IntrWriteMem]>;

def int_tpc_st_tnsr_s         : Intrinsic<[],
                                          [llvm_v5i32_ty, llvm_i8_ty, llvm_any_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                          [IntrWriteMem]>;
def int_tpc_st_tnsr_s_rmw     : Intrinsic<[],
                                          [llvm_v5i32_ty, llvm_i8_ty, llvm_any_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                          [IntrWriteMem]>;
def int_tpc_st_tnsr_s_hwr     : Intrinsic<[],
                                          [llvm_v5i32_ty, llvm_i8_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                          [IntrWriteMem]>;
def int_tpc_st_tnsr_s_hwr_rmw : Intrinsic<[],
                                          [llvm_v5i32_ty, llvm_i8_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                          [IntrWriteMem]>;

def int_tpc_cache_flush       : Intrinsic<[], [llvm_i32_ty, llvm_i1_ty, llvm_i1_ty], []>;
def int_tpc_cache_invalidate  : Intrinsic<[], [llvm_i32_ty, llvm_i1_ty, llvm_i1_ty], []>;

def int_tpc_tanh : Intrinsic<[llvm_anyfloat_ty], [LLVMMatchType<0>]>;
def int_tpc_reciprocal : Intrinsic<[llvm_anyfloat_ty], [LLVMMatchType<0>]>;
def int_tpc_rsqrt : Intrinsic<[llvm_anyfloat_ty], [LLVMMatchType<0>]>;
}

