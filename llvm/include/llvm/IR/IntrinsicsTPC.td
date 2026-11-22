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

def tpc_global_ptr_ty   : LLVMQualPointerType<3>;             // ptr addrspace(3)


// Every TPC intrinsic is one instruction and returns. LLVM 14 assumed that of
// an intrinsic that only reads memory; later LLVM needs the attribute, and
// without it a call whose result is unused stays and SCEV cannot use an
// induction variable's no-wrap flags past a store. A `let IntrProperties`
// block replaces the list, so it must name IntrWillReturn itself.
class TPCIntrin<list<LLVMType> ret_types,
                list<LLVMType> param_types = [],
                list<IntrinsicProperty> intr_properties = [],
                string name = "",
                list<SDNodeProperty> sd_properties = []>
  : Intrinsic<ret_types, param_types, intr_properties # [IntrWillReturn],
              name, sd_properties>;

let TargetPrefix = "tpc" in {  // All intrinsics start with "llvm.tpc.".

def int_tpc_get_index_space_offset : TPCIntrin<[llvm_v5i32_ty], [], [IntrNoMem]>;
def int_tpc_get_index_space_size   : TPCIntrin<[llvm_v5i32_ty], [], [IntrNoMem]>;

def int_tpc_read_lfsr              : TPCIntrin<[llvm_v256i8_ty],
                                               [ llvm_v256i8_ty,     // Income
                                                 llvm_any_ty,        // Predicate (i1/v256i1)
                                                 llvm_i1_ty          // Polarity
                                               ],
                                               []>;
def int_tpc_write_lfsr             : TPCIntrin<[],
                                               [ llvm_v256i8_ty,     // Src
                                                 llvm_any_ty,        // Predicate (i1/v256i1)
                                                 llvm_i1_ty          // Polarity
                                               ],
                                               [IntrWriteMem, IntrInaccessibleMemOnly]>;
def int_tpc_read_lfsrnc            : TPCIntrin<[llvm_v256i8_ty],
                                               [ llvm_v256i8_ty,     // Income
                                                 llvm_any_ty,        // Predicate (i1/v256i1)
                                                 llvm_i1_ty          // Polarity
                                               ],
                                               [IntrReadMem, IntrWillReturn, IntrInaccessibleMemOnly]>;

def int_tpc_s_read_lfsr              : TPCIntrin<[llvm_i32_ty],
                                                 [ llvm_i32_ty,        // Income
                                                   llvm_i1_ty,         // Predicate
                                                   llvm_i1_ty          // Polarity
                                                 ],
                                                 []>;
def int_tpc_write_lfsr_s             : TPCIntrin<[],
                                                 [ llvm_i32_ty,        // Src
                                                   llvm_i1_ty,         // Predicate
                                                   llvm_i1_ty          // Polarity
                                                 ],
                                                 [IntrWriteMem, IntrInaccessibleMemOnly]>;
def int_tpc_s_read_lfsrnc            : TPCIntrin<[llvm_i32_ty],
                                                 [ llvm_i32_ty,        // Income
                                                   llvm_i1_ty,         // Predicate
                                                   llvm_i1_ty          // Polarity
                                                 ],
                                                 [IntrReadMem, IntrWillReturn, IntrInaccessibleMemOnly]>;


def int_tpc_read_lane_id             : TPCIntrin<[llvm_any_ty],
                                                 [ LLVMMatchType<0>,   // Income
                                                   llvm_any_ty,        // Predicate
                                                   llvm_i1_ty          // Polarity
                                                 ],
                                                 [IntrNoMem]>;

//------ MOVs ------------------------------------------------------------------

def int_tpc_mov_flavor    : TPCIntrin<[llvm_v256i1_ty],
                                      [ llvm_i32_ty,        // X
                                        llvm_i32_ty,        // Flavor
                                        llvm_i32_ty,        // Switches
                                        llvm_v256i1_ty,     // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_mov_mask      : TPCIntrin<[llvm_v5i32_ty],
                                      [ llvm_any_ty,        // X, may be v5i32 or i32
                                        llvm_i32_ty,        // DimMask
                                        llvm_i32_ty,        // Switches
                                        llvm_v5i32_ty,      // Income
                                        llvm_i1_ty,         // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_mov           : TPCIntrin<[llvm_any_ty],
                                      [ llvm_any_ty,        // X
                                        llvm_i8_ty,         // Data Type
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;

def int_tpc_get_addr      : TPCIntrin<[llvm_v2i32_ty],
                                      [ tpc_global_ptr_ty,      // Ptr
                                        llvm_i32_ty,        // Switches
                                        llvm_v2i32_ty,      // Income
                                        llvm_i1_ty,         // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrReadMem, IntrWillReturn]>;

def int_tpc_update_addr   : TPCIntrin<[tpc_global_ptr_ty],
                                      [ tpc_global_ptr_ty,      // Ptr
                                        llvm_v2i32_ty,      // Addr
                                        llvm_i32_ty,        // Switches
                                        llvm_i1_ty,         // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrWriteMem]>;

//------ Load slot -------------------------------------------------------------

def int_tpc_gen_addr      : TPCIntrin<[tpc_global_ptr_ty],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_i32_ty, tpc_global_ptr_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_event         : TPCIntrin<[],
                                      [llvm_i16_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      []>;
def int_tpc_prmt_indx     : TPCIntrin<[llvm_v5i32_ty],
                                      [llvm_v5i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_v5i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_set_indx      : TPCIntrin<[llvm_v5i32_ty],
                                      [llvm_v5i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_ld_l          : TPCIntrin<[llvm_any_ty],
                                      [llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_i1_ty, llvm_i1_ty],
                                      [IntrReadMem, IntrWillReturn]>;
def int_tpc_ld_g          : TPCIntrin<[llvm_any_ty],
                                      [tpc_global_ptr_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrReadMem, IntrWillReturn]>;
def int_tpc_ld_g_inc      : TPCIntrin<[llvm_any_ty, llvm_any_ty],
                                      [LLVMMatchType<1>, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_i1_ty, llvm_i1_ty],
                                      [IntrReadMem, IntrWillReturn, IntrArgMemOnly]>;
def int_tpc_ld_g_int5     : TPCIntrin<[llvm_v5i32_ty],
                                      [tpc_global_ptr_ty, llvm_i32_ty, llvm_i32_ty, llvm_v5i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrReadMem, IntrWillReturn]>;
def int_tpc_ld_g_int5_inc : TPCIntrin<[llvm_v5i32_ty, tpc_global_ptr_ty],
                                      [tpc_global_ptr_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_v5i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrReadMem, IntrWillReturn, IntrArgMemOnly]>;
def int_tpc_ld_g_partial  : TPCIntrin<[llvm_anyvector_ty],
                                      [tpc_global_ptr_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_ld_g_partial_inc : TPCIntrin<[llvm_any_ty, llvm_any_ty],
                                         [LLVMMatchType<1>, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                         [IntrReadMem, IntrWillReturn, IntrArgMemOnly]>;
def int_tpc_prefetch      : TPCIntrin<[],
                                      [ tpc_global_ptr_ty,      // Address
                                        llvm_i32_ty,        // Switches
                                        llvm_i1_ty,         // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrInaccessibleMemOrArgMemOnly]>;
def int_tpc_ld_l_v        : TPCIntrin<[llvm_anyvector_ty],
                                      [ llvm_i32_ty,        // Address
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrReadMem, IntrWillReturn]>;
def int_tpc_ld_l_v_low    : TPCIntrin<[llvm_anyvector_ty],
                                      [ llvm_i32_ty,        // Address
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrReadMem, IntrWillReturn]>;
def int_tpc_ld_l_v_high   : TPCIntrin<[llvm_anyvector_ty],
                                      [ llvm_i32_ty,        // Address
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrReadMem, IntrWillReturn]>;
def int_tpc_ld_tnsr       : TPCIntrin<[llvm_anyvector_ty],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_ld_tnsr_direct : TPCIntrin<[llvm_anyvector_ty],
                                      [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_ld_tnsr_partial : TPCIntrin<[llvm_anyvector_ty],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_ld_tnsr_partial_direct : TPCIntrin<[llvm_anyvector_ty],
                                      [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_ld_tnsr_low   : TPCIntrin<[llvm_anyvector_ty],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_ld_tnsr_low_direct   : TPCIntrin<[llvm_anyvector_ty],
                                      [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_ld_tnsr_low_partial : TPCIntrin<[llvm_anyvector_ty],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_ld_tnsr_low_partial_direct : TPCIntrin<[llvm_anyvector_ty],
                                      [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_ld_tnsr_high  : TPCIntrin<[llvm_anyvector_ty],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_ld_tnsr_high_direct  : TPCIntrin<[llvm_anyvector_ty],
                                      [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_ld_tnsr_high_partial : TPCIntrin<[llvm_anyvector_ty],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_ld_tnsr_high_partial_direct : TPCIntrin<[llvm_anyvector_ty],
                                      [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_lookup        : TPCIntrin<[llvm_anyvector_ty],
                                      [llvm_any_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_i1_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_lookup_c0     : TPCIntrin<[llvm_anyvector_ty],
                                      [llvm_any_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_i1_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_lookup_c1c2   : TPCIntrin<[llvm_anyvector_ty],
                                      [llvm_any_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_i1_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_lookup_1c     : TPCIntrin<[llvm_anyvector_ty],
                                      [llvm_any_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_i1_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_lookup_2c     : TPCIntrin<[llvm_anyvector_ty],
                                      [llvm_any_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_i1_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_ld_tnsr_cnvrt : TPCIntrin<[llvm_anyvector_ty],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_ld_tnsr_cnvrt_partial : TPCIntrin<[llvm_anyvector_ty],
                                              [llvm_v5i32_ty, llvm_i8_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                              [IntrNoMem]>;
def int_tpc_ld_tnsr_cnvrt_direct : TPCIntrin<[llvm_anyvector_ty],
                                             [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                             [IntrNoMem]>;
def int_tpc_ld_tnsr_cnvrt_partial_direct : TPCIntrin<[llvm_anyvector_ty],
                                                     [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                                     [IntrNoMem]>;

//------ Scalar slot, also instructions common for scalar and vector slots.

def int_tpc_mac           : TPCIntrin<[llvm_any_ty],
                                      [llvm_any_ty, LLVMMatchType<1>, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;

def int_tpc_mac_zp        : TPCIntrin<[llvm_any_ty],
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
def int_tpc_mac_x2        : TPCIntrin<[llvm_any_ty],
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

def int_tpc_mac_x2_f32    : TPCIntrin<[llvm_any_ty],
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

def int_tpc_mac_x2_zp      : TPCIntrin<[llvm_any_ty],
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

def int_tpc_madd           : TPCIntrin<[llvm_any_ty],
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

def int_tpc_madd_zp        : TPCIntrin<[llvm_any_ty],
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

def int_tpc_madd_x2        : TPCIntrin<[llvm_any_ty],
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

def int_tpc_mul           : TPCIntrin<[llvm_any_ty],
                                      [ llvm_any_ty,        // X
                                        llvm_any_ty,        // Y, may differ from X, in V+S variants
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_mul_x2_f32    : TPCIntrin<[llvm_any_ty],
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
def int_tpc_mul_mask      : TPCIntrin<[llvm_v5i32_ty],
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
def int_tpc_add           : TPCIntrin<[llvm_any_ty],
                                      [ llvm_any_ty,        // X
                                        llvm_any_ty,        // Y, may differ from X, in V+S variants
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_add_x2        : TPCIntrin<[llvm_any_ty],
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
def int_tpc_add_mask      : TPCIntrin<[llvm_v5i32_ty],
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
def int_tpc_sub           : TPCIntrin<[llvm_any_ty],        // VRF, IRF or SRF type
                                      [ llvm_any_ty,        // X, may differ from result in IRF operations
                                        llvm_any_ty,        // Y, may differ from X, in V+S variants
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
// An integer ADD or SUB followed by an ADD with CARRY or a SUB with BORROW
// that reads the carry the first one writes (a 64-bit add from two 32-bit
// ones). The carry is not a value in the IR, so the two are kept as one call
// (TPCCarryPairs) and emitted next to each other. Operands: Kind (bit 0: the
// first is a SUB, bit 1: the second is a SUB), then the operands of tpc_add
// or tpc_sub for the first and for the second. Returns both results.
def int_tpc_carry_pair    : TPCIntrin<[llvm_any_ty, llvm_any_ty],
                                      [ llvm_i32_ty,        // Kind
                                        llvm_any_ty,        // X of the first
                                        llvm_any_ty,        // Y of the first
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty,         // Polarity
                                        llvm_any_ty,        // X of the second
                                        llvm_any_ty,        // Y of the second
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<1>,   // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem, ImmArg<ArgIndex<0>>]>;
def int_tpc_sub_x2        : TPCIntrin<[llvm_any_ty],
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
def int_tpc_sub_mask      : TPCIntrin<[llvm_v5i32_ty],
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
def int_tpc_max           : TPCIntrin<[llvm_any_ty],
                                      [ llvm_any_ty,        // X
                                        llvm_any_ty,        // Y, may differ from X, in V+S variants
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_max_mask      : TPCIntrin<[llvm_v5i32_ty],
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
def int_tpc_min           : TPCIntrin<[llvm_any_ty],
                                      [ llvm_any_ty,        // X
                                        llvm_any_ty,        // Y, may differ from X, in V+S variants
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_min_mask      : TPCIntrin<[llvm_v5i32_ty],
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
def int_tpc_abs           : TPCIntrin<[llvm_any_ty],        // VRF, IRF or SRF type
                                      [ llvm_any_ty,        // X, may differ from result in IRF operations
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_abs_mask      : TPCIntrin<[llvm_v5i32_ty],
                                      [ llvm_any_ty,        // X, may differ from result in IRF operations
                                        llvm_i32_ty,        // DimMask
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        llvm_v5i32_ty,      // Income
                                        llvm_i1_ty,         // Predicate
                                        llvm_i1_ty],        // Polarity
                                      [IntrNoMem]>;
def int_tpc_cmp_eq        : TPCIntrin<[llvm_any_ty],        // v256i1 or i1
                                      [ llvm_any_ty,        // X, may be scalar (including imm)
                                        llvm_any_ty,        // Y, may be scalar (including imm)
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1 or v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_cmp_eq_mask   : TPCIntrin<[llvm_i1_ty],
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
def int_tpc_cmp_neq       : TPCIntrin<[llvm_any_ty],        // v256i1 or i1
                                      [ llvm_any_ty,        // X, may be scalar (including imm)
                                        llvm_any_ty,        // Y, may be scalar (including imm)
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1 or v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_cmp_neq_mask  : TPCIntrin<[llvm_i1_ty],
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
def int_tpc_cmp_less      : TPCIntrin<[llvm_any_ty],        // v256i1 or i1
                                      [ llvm_any_ty,        // X, may be scalar (including imm)
                                        llvm_any_ty,        // Y, may be scalar (including imm)
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1 or v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_cmp_less_mask : TPCIntrin<[llvm_i1_ty],
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
def int_tpc_cmp_leq       : TPCIntrin<[llvm_any_ty],        // v256i1 or i1
                                      [ llvm_any_ty,        // X, may be scalar (including imm)
                                        llvm_any_ty,        // Y, may be scalar (including imm)
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1 or v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_cmp_leq_mask  : TPCIntrin<[llvm_i1_ty],
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
def int_tpc_cmp_grt       : TPCIntrin<[llvm_any_ty],        // v256i1 or i1
                                      [ llvm_any_ty,        // X, may be scalar (including imm)
                                        llvm_any_ty,        // Y, may be scalar (including imm)
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1 or v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_cmp_grt_mask  : TPCIntrin<[llvm_i1_ty],
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
def int_tpc_cmp_geq       : TPCIntrin<[llvm_any_ty],        // v256i1 or i1
                                      [ llvm_any_ty,        // X, may be scalar (including imm)
                                        llvm_any_ty,        // Y, may be scalar (including imm)
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1 or v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_cmp_geq_mask  : TPCIntrin<[llvm_i1_ty],
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
def int_tpc_or            : TPCIntrin<[llvm_any_ty],
                                      [ llvm_any_ty,        // X
                                        llvm_any_ty,        // Y, may differ from X, in V+S variants
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_or_mask       : TPCIntrin<[llvm_v5i32_ty],
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
def int_tpc_and           : TPCIntrin<[llvm_any_ty],
                                      [ llvm_any_ty,        // X
                                        llvm_any_ty,        // Y, may differ from X, in V+S variants
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_and_mask      : TPCIntrin<[llvm_v5i32_ty],
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
def int_tpc_xor           : TPCIntrin<[llvm_any_ty],
                                      [ llvm_any_ty,        // X
                                        llvm_any_ty,        // Y, may differ from X, in V+S variants
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_xor_mask      : TPCIntrin<[llvm_v5i32_ty],
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
def int_tpc_not           : TPCIntrin<[llvm_any_ty],
                                      [ llvm_any_ty,        // X, may differ from the result in IRF operations
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_not_mask      : TPCIntrin<[llvm_v5i32_ty],
                                      [ llvm_any_ty,        // X, may be v5i32 or i32
                                        llvm_i32_ty,        // DimMask
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        llvm_v5i32_ty,      // Income
                                        llvm_i1_ty,         // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_shr           : TPCIntrin<[llvm_any_ty],
                                      [ LLVMMatchType<0>,   // Value to shift
                                        llvm_any_ty,        // Number of bits, may be vector or scalar
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_shr_mask      : TPCIntrin<[llvm_v5i32_ty],
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
def int_tpc_shl           : TPCIntrin<[llvm_any_ty],
                                      [ LLVMMatchType<0>,   // Value to shift
                                        llvm_any_ty,        // Number of bits, may be vector or scalar
                                        llvm_i8_ty,         // DataType
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_shl_mask      : TPCIntrin<[llvm_v5i32_ty],
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

def int_tpc_ash           : TPCIntrin<[llvm_anyint_ty],
                                      [ llvm_anyint_ty,    // Value to shift
                                        llvm_anyint_ty,    // Shift value (i8/v256i8)
                                        llvm_i8_ty,        // DataType
                                        llvm_i32_ty,       // Switches
                                        LLVMMatchType<0>,  // Income
                                        llvm_any_ty,       // Predicate (i1/v256i1)
                                        llvm_i1_ty         // Polarity
                                      ],
                                      [IntrNoMem]>;

def int_tpc_convert_int64   : TPCIntrin<[llvm_any_ty],
                                      [ llvm_any_ty,       // half1
                                        llvm_any_ty,       // half2
                                        llvm_i32_ty,       // Switches
                                        LLVMMatchType<0>,  // Income
                                        llvm_any_ty,       // Predicate
                                        llvm_i1_ty         // Polarity
                                      ],
                                      [IntrNoMem]>;

// Universal routine to convert any vector types (u/int, fp) and keep order of elements (no swizzling).
// A chain of intermediate conversions will be generated automatically if target architecture does not 
// support direct hardware conversion. 
// Supports native TPC rounding modes, saturation and clipping where applicable. 
// The rounding mode switches are the same as for TPC-C API (tpc-defs.h):
//         \li SW_RHNE - Round half to nearest even.
//         \li SW_RZ - Round zero.
//         \li SW_RU - Round up.
//         \li SW_RD - Round down.
//         \li SW_SR - Stochastic Rounding.
//         \li SW_RHAZ - Round half away from zero.
//         \li SW_CSR - Take rounding mode from CSR register 
//         \li SW_SR_RNE - Stochastic Rounding, but use RNE when rounding denormals.
//         \li SW_CLIP_FP - Convert FP with clipping.
// Source and destination DT should be specified explicitly, this allows to distinguish 
// between signed vs unsigned types. 
// The source DT is passed via dedicated arg, values follow PRM encoding of optype.
// The dest DT is passed via switch operand, values have optype encoding shifted left by 8 (see InstructionDB.h):
//    SW_TO_FP32            = OpType::FP32 << 8,
//    SW_TO_BF16            = OpType::BF16 << 8,
//    SW_TO_INT32           = OpType::INT32 << 8,
//    SW_TO_UINT32          = OpType::UINT32 << 8,
//    SW_TO_INT8            = OpType::INT8 << 8,
//    SW_TO_UINT8           = OpType::UINT8 << 8,
//    SW_TO_INT16           = OpType::INT16 << 8,
//    SW_TO_UINT16          = OpType::UINT16 << 8,
//    SW_TO_FP16            = OpType::FP16 << 8,
//    SW_TO_FP8_152         = OpType::FP8_152 << 8,
//    SW_TO_FP8_143         = OpType::FP8_143 << 8,
def int_tpc_convert_linear       : TPCIntrin<[llvm_any_ty],
                                      [ llvm_any_ty,       // Value to convert
                                        llvm_i8_ty,        // Src DataType
                                        llvm_i32_ty,       // Switches (Dest datatype, rounding mode)
                                        LLVMMatchType<0>,  // Income
                                        llvm_any_ty,       // Predicate (i1/v256i1)
                                        llvm_i1_ty         // Polarity
                                      ],
                                      [IntrNoMem]>;

// Universal routine to convert any vector types (u/int, fp) w/ swizzling 
// (i.e. to perform native TPC behaviour for up- or down-converts).
// A chain of intermediate conversions will be generated automatically if 
// target architecture does not support direct hardware conversion.
// Switches and DT follow the same convention as int_tpc_convert_linear().
def int_tpc_convert       : TPCIntrin<[llvm_any_ty],
                                      [ llvm_any_ty,       // Value to convert
                                        llvm_i8_ty,        // Src DataType
                                        llvm_i32_ty,       // Switches (Dest datatype, rounding mode)
                                        LLVMMatchType<0>,  // Income
                                        llvm_any_ty,       // Predicate (i1/v256i1)
                                        llvm_i1_ty         // Polarity
                                      ],
                                      [IntrNoMem]>;

def int_tpc_convert_int   : TPCIntrin<[llvm_any_ty],
                                      [ llvm_any_ty,       // Value to convert
                                        llvm_any_ty,       // Shift value (char/char256)
                                        llvm_i32_ty,       // Switches
                                        LLVMMatchType<0>,  // Income
                                        llvm_any_ty,       // Predicate (i1/v256i1)
                                        llvm_i1_ty         // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_convert_uint  : TPCIntrin<[llvm_any_ty],
                                      [ llvm_any_ty,       // Value to convert
                                        llvm_any_ty,       // Shift value (char/char256)
                                        llvm_i32_ty,       // Switches
                                        LLVMMatchType<0>,  // Income
                                        llvm_any_ty,       // Predicate (i1/v256i1)
                                        llvm_i1_ty         // Polarity
                                      ],
                                      [IntrNoMem]>;

def int_tpc_convert_int_linear   : TPCIntrin<[llvm_any_ty],
                                      [ llvm_any_ty,       // Value to convert
                                        llvm_any_ty,       // Shift value (char/char256)
                                        llvm_i32_ty,       // Switches
                                        LLVMMatchType<0>,  // Income
                                        llvm_any_ty,       // Predicate (i1/v256i1)
                                        llvm_i1_ty         // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_convert_uint_linear  : TPCIntrin<[llvm_any_ty],
                                      [ llvm_any_ty,       // Value to convert
                                        llvm_any_ty,       // Shift value (char/char256)
                                        llvm_i32_ty,       // Switches
                                        LLVMMatchType<0>,  // Income
                                        llvm_any_ty,       // Predicate (i1/v256i1)
                                        llvm_i1_ty         // Polarity
                                      ],
                                      [IntrNoMem]>;



// Floating point rounding mode intrinsics (deprecated)
def int_tpc_fptrunc_swch  : TPCIntrin<[llvm_anyfloat_ty],
                                      [ llvm_anyfloat_ty,       // Value to convert
                                        llvm_i32_ty,            // Switches
                                      ],
                                      [IntrNoMem]>;

def int_tpc_fpext_swch  : TPCIntrin<[llvm_anyfloat_ty],
                                      [ llvm_anyfloat_ty,       // Value to convert
                                        llvm_i32_ty,            // Switches
                                      ],
                                      [IntrNoMem]>;

def int_tpc_sitofp_swch  : TPCIntrin<[llvm_anyfloat_ty],
                                      [ llvm_anyint_ty,       // Value to convert
                                        llvm_i32_ty,          // Switches
                                      ],
                                      [IntrNoMem]>;

def int_tpc_fptosi_swch  : TPCIntrin<[llvm_anyint_ty],
                                      [ llvm_anyfloat_ty,      // Value to convert
                                        llvm_i32_ty,           // Switches
                                      ],
                                      [IntrNoMem]>;

def int_tpc_uitofp_swch  : TPCIntrin<[llvm_anyfloat_ty],
                                      [ llvm_anyint_ty,       // Value to convert
                                        llvm_i32_ty,          // Switches
                                      ],
                                      [IntrNoMem]>;

def int_tpc_fptoui_swch  : TPCIntrin<[llvm_anyint_ty],
                                      [ llvm_anyfloat_ty,     // Value to convert
                                        llvm_i32_ty,          // Switches
                                      ],
                                      [IntrNoMem]>;

def int_tpc_popcnt        : TPCIntrin<[llvm_any_ty],
                                      [llvm_any_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_find_first    : TPCIntrin<[llvm_any_ty],
                                      [llvm_any_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_nearbyint     : TPCIntrin<[llvm_any_ty],
                                      [ llvm_any_ty,       // Value to convert
                                        llvm_i8_ty,        // DataType
                                        llvm_i32_ty,       // Switches
                                        LLVMMatchType<0>,  // Income
                                        llvm_any_ty,       // Predicate (i1/v256i1)
                                        llvm_i1_ty         // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_fclass        : TPCIntrin<[llvm_any_ty],
                                      [ LLVMMatchType<0>,
                                        llvm_i8_ty,        // DataType
                                        llvm_i32_ty,       // Switches
                                        LLVMMatchType<0>,  // Income
                                        llvm_any_ty,       // Predicate (i1/v256i1)
                                        llvm_i1_ty         // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_fclass_limit  : TPCIntrin<[llvm_any_ty],
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
def int_tpc_extract_exp   : TPCIntrin<[llvm_any_ty],
                                      [llvm_any_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_brev          : TPCIntrin<[llvm_any_ty],
                                      [llvm_any_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_mov_irf_dim   : TPCIntrin<[llvm_any_ty],
                                      [ llvm_v5i32_ty,      // IRF
                                        llvm_i8_ty,         // Dim
                                        llvm_i32_ty,        // Switches
                                        LLVMMatchType<0>,   // Income
                                        llvm_i1_ty,         // Predicate
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_udiv_step     : TPCIntrin<[llvm_any_ty],
                                      [llvm_anyint_ty, llvm_i32_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_i1_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_udiv          : TPCIntrin<[llvm_any_ty],
                                      [llvm_anyint_ty, LLVMMatchType<1>, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_i1_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_calc_fp_special : TPCIntrin<[llvm_any_ty],
                                      [LLVMMatchType<0>, LLVMMatchType<0>, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;

def int_tpc_thread_sync  : TPCIntrin<[], [], []>;

def int_tpc_convert_fp_flex  : TPCIntrin<[llvm_any_ty],
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

def int_tpc_shuffle       : TPCIntrin<[llvm_any_ty],
                                      [LLVMMatchType<0>, llvm_v256i8_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_pack          : TPCIntrin<[llvm_any_ty],
                                      [LLVMMatchType<0>, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_unpack        : TPCIntrin<[llvm_any_ty],
                                      [LLVMMatchType<0>, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_get_lut_entry : TPCIntrin<[llvm_any_ty],
                                      [llvm_any_ty, llvm_i8_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_form_fp_num   : TPCIntrin<[llvm_any_ty],
                                      [llvm_any_ty, LLVMMatchType<0>, LLVMMatchType<0>, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_mov_group     : TPCIntrin<[llvm_any_ty],
                                      [llvm_any_ty, llvm_i32_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_mov_dual_group : TPCIntrin<[llvm_anyvector_ty],
                                      [ LLVMMatchType<0>,  // Source
                                        llvm_i32_ty,       // ByteEn (encoded in immediate)
                                        llvm_i32_ty,       // CTRL value (encoded in imm or scalar (since doron1))
                                        llvm_i32_ty,       // Switches
                                        LLVMMatchType<0>,  // Income value
                                        llvm_any_ty,       // Predicate (i1/v256i1)
                                        llvm_i1_ty         // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_mov_dual_group_all : TPCIntrin<[llvm_anyvector_ty],
                                      [ LLVMMatchType<0>,  // Source
                                        llvm_i32_ty,       // ByteEn (encoded in immediate)
                                        llvm_i32_ty,       // CTRL value (encoded in imm or scalar (since doron1))
                                        llvm_i32_ty,       // Switches
                                        LLVMMatchType<0>,  // Income value
                                        llvm_any_ty,       // Predicate (i1/v256i1)
                                        llvm_i1_ty         // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_mov_dual_group_pack : TPCIntrin<[llvm_anyvector_ty],
                                      [ LLVMMatchType<0>,  // Source
                                        llvm_i32_ty,       // Switches
                                        LLVMMatchType<0>,  // Income value
                                        llvm_any_ty,       // Predicate (i1/v256i1)
                                        llvm_i1_ty         // Polarity
                                      ],
                                      [IntrNoMem]>;
def int_tpc_mov_dual_group_unpack : TPCIntrin<[llvm_anyvector_ty],
                                      [ LLVMMatchType<0>,  // Source
                                        llvm_i32_ty,       // ByteEn (encoded in immediate)
                                        llvm_i32_ty,       // Switches
                                        LLVMMatchType<0>,  // Income value
                                        llvm_any_ty,       // Predicate (i1/v256i1)
                                        llvm_i1_ty         // Polarity
                                      ],
                                      [IntrNoMem]>;
                          // v64i32 SEL_EQ(v64f32:a, [v64f32, f32]:b, v64i32:c, [v64i32, i32]:d, int8 dt, int sw, v64i32 income, [v256i1, i1]:pred, i1 polarity)
def int_tpc_sel_eq        : TPCIntrin<[llvm_any_ty],
                                      [llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_sel_neq       : TPCIntrin<[llvm_any_ty],
                                      [llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_sel_less      : TPCIntrin<[llvm_any_ty],
                                      [llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_sel_leq       : TPCIntrin<[llvm_any_ty],
                                      [llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_sel_grt       : TPCIntrin<[llvm_any_ty],
                                      [llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_sel_geq       : TPCIntrin<[llvm_any_ty],
                                      [llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;

                          // (2 x v64i32) SEL2_LESS(v64f32:a, [v64f32, f32]:b, v64i32:c, [v64i32, i32]:d, int8 dt, int sw, v64i32 income, [v256i1, i1]:pred, i1 polarity)
def int_tpc_sel2_less     : TPCIntrin<[llvm_any_ty],
                                      [llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_sel2_leq      : TPCIntrin<[llvm_any_ty],
                                      [llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_sel2_grt      : TPCIntrin<[llvm_any_ty],
                                      [llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_sel2_geq      : TPCIntrin<[llvm_any_ty],
                                      [llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_any_ty, llvm_i8_ty, llvm_i32_ty, LLVMMatchType<0>, llvm_any_ty, llvm_i1_ty],
                                      [IntrNoMem]>;
def int_tpc_msac          : TPCIntrin<[llvm_anyvector_ty],
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

def int_tpc_st_l          : TPCIntrin<[],
                                      [llvm_i32_ty, llvm_any_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_g          : TPCIntrin<[],
                                      [tpc_global_ptr_ty, llvm_any_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem, IntrArgMemOnly]>;
def int_tpc_st_g_inc      : TPCIntrin<[llvm_any_ty],
                                      [LLVMMatchType<0>, llvm_any_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem, IntrArgMemOnly]>;
def int_tpc_st_l_v        : TPCIntrin<[],
                                      [ llvm_i32_ty,        // Address
                                        llvm_anyvector_ty,  // Value
                                        llvm_i32_ty,        // Switches
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrWriteMem]>;
def int_tpc_st_l_v_ofs    : TPCIntrin<[],
                                      [ llvm_i32_ty,        // Base
                                        llvm_i32_ty,        // Offset
                                        llvm_anyvector_ty,  // Value
                                        llvm_i32_ty,        // Switches
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrWriteMem]>;
def int_tpc_st_l_v_low    : TPCIntrin<[],
                                      [ llvm_i32_ty,        // Address
                                        llvm_anyvector_ty,  // Value
                                        llvm_i32_ty,        // Switches
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrWriteMem]>;
def int_tpc_st_l_v_low_ofs : TPCIntrin<[],
                                      [ llvm_i32_ty,        // Base
                                        llvm_i32_ty,        // Offset
                                        llvm_anyvector_ty,  // Value
                                        llvm_i32_ty,        // Switches
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrWriteMem]>;
def int_tpc_st_l_v_high   : TPCIntrin<[],
                                      [ llvm_i32_ty,        // Address
                                        llvm_anyvector_ty,  // Value
                                        llvm_i32_ty,        // Switches
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrWriteMem]>;
def int_tpc_st_l_v_high_ofs : TPCIntrin<[],
                                      [ llvm_i32_ty,        // Base
                                        llvm_i32_ty,        // Offset
                                        llvm_anyvector_ty,  // Value
                                        llvm_i32_ty,        // Switches
                                        llvm_any_ty,        // Predicate (i1/v256i1)
                                        llvm_i1_ty          // Polarity
                                      ],
                                      [IntrWriteMem]>;
def int_tpc_aso           : TPCIntrin<[],
                                      [llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      []>;
def int_tpc_st_tnsr       : TPCIntrin<[],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_tnsr_direct : TPCIntrin<[],
                                      [llvm_i32_ty, llvm_i32_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_tnsr_partial : TPCIntrin<[],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_tnsr_partial_direct : TPCIntrin<[],
                                      [llvm_i32_ty, llvm_i32_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
// The RMW stores read the value they update, and the squeeze stores depend on
// the squeeze counter and partial buffer, so they are not idempotent, but they
// stay write-only: as anything else TableGen would make their instructions
// loads or give them side effects. On TPC, EarlyCSE does not merge identical
// write-only calls and LICM does not move them out of loops.
def int_tpc_st_tnsr_rmw   : TPCIntrin<[],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_tnsr_rmw_direct   : TPCIntrin<[],
                                      [llvm_i32_ty, llvm_i32_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_tnsr_partial_rmw : TPCIntrin<[],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_tnsr_partial_rmw_direct : TPCIntrin<[],
                                      [llvm_i32_ty, llvm_i32_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_tnsr_low   : TPCIntrin<[],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_tnsr_low_direct   : TPCIntrin<[],
                                      [llvm_i32_ty, llvm_i32_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_tnsr_low_rmw : TPCIntrin<[],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_tnsr_low_rmw_direct : TPCIntrin<[],
                                      [llvm_i32_ty, llvm_i32_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_tnsr_high  : TPCIntrin<[],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_tnsr_high_direct  : TPCIntrin<[],
                                      [llvm_i32_ty, llvm_i32_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_tnsr_high_rmw : TPCIntrin<[],
                                      [llvm_v5i32_ty, llvm_i8_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;
def int_tpc_st_tnsr_high_rmw_direct : TPCIntrin<[],
                                      [llvm_i32_ty, llvm_i32_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                      [IntrWriteMem]>;

def int_tpc_st_tnsr_sqz       : TPCIntrin<[],
                                          [llvm_v5i32_ty, llvm_i8_ty, llvm_i8_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_v256i1_ty, llvm_i1_ty],
                                          [IntrWriteMem]>;
def int_tpc_st_tnsr_sqz_rmw   : TPCIntrin<[],
                                          [llvm_v5i32_ty, llvm_i8_ty, llvm_i8_ty, llvm_anyvector_ty, llvm_i32_ty, llvm_i32_ty, llvm_v256i1_ty, llvm_i1_ty],
                                          [IntrWriteMem]>;

def int_tpc_st_tnsr_s         : TPCIntrin<[],
                                          [llvm_v5i32_ty, llvm_i8_ty, llvm_any_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                          [IntrWriteMem]>;
def int_tpc_st_tnsr_s_rmw     : TPCIntrin<[],
                                          [llvm_v5i32_ty, llvm_i8_ty, llvm_any_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                          [IntrWriteMem]>;
def int_tpc_st_tnsr_s_hwr     : TPCIntrin<[],
                                          [llvm_v5i32_ty, llvm_i8_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                          [IntrWriteMem]>;
def int_tpc_st_tnsr_s_hwr_rmw : TPCIntrin<[],
                                          [llvm_v5i32_ty, llvm_i8_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty],
                                          [IntrWriteMem]>;

def int_tpc_reset_sqz_cntr    : TPCIntrin<[], [llvm_i8_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty], [IntrHasSideEffects]>;

def int_tpc_cache_flush       : TPCIntrin<[], [llvm_i32_ty, llvm_i1_ty, llvm_i1_ty], []>;
def int_tpc_cache_invalidate  : TPCIntrin<[], [llvm_i32_ty, llvm_i1_ty, llvm_i1_ty], []>;

def int_tpc_cache_flush_addr      : TPCIntrin<[], [tpc_global_ptr_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty], []>;
def int_tpc_cache_invalidate_addr : TPCIntrin<[], [tpc_global_ptr_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty], []>;

def int_tpc_tanh : TPCIntrin<[llvm_anyfloat_ty], [LLVMMatchType<0>]>;
def int_tpc_reciprocal : TPCIntrin<[llvm_anyfloat_ty], [LLVMMatchType<0>]>;
def int_tpc_rsqrt : TPCIntrin<[llvm_anyfloat_ty], [LLVMMatchType<0>]>;
def int_tpc_swizzle : TPCIntrin<[llvm_anyvector_ty], [LLVMMatchType<0>]>;
def int_tpc_gen_stage1_seed_v64f32 : TPCIntrin<[llvm_v64f32_ty], [llvm_i32_ty]>;

//===------------------------ Cast LTO Intrinsics -------------------------===//
let IntrProperties = [IntrNoMem, IntrWillReturn] in {
  def int_tpc_trunc_i32_to_i8 : TPCIntrin<[llvm_v256i8_ty], [llvm_v64i32_ty, llvm_v64i32_ty, llvm_v64i32_ty, llvm_v64i32_ty]>;
}

//===------------------------ Reduction Intrinsics ------------------------===//
//
let IntrProperties = [IntrNoMem, IntrWillReturn] in {
  def int_tpc_vector_reduce_v2_fadd  : TPCIntrin<[llvm_anyfloat_ty], [LLVMMatchType<0>]>;
  def int_tpc_vector_reduce_v2_fmul  : TPCIntrin<[llvm_anyfloat_ty], [LLVMMatchType<0>]>;
  def int_tpc_vector_reduce_add  : TPCIntrin<[llvm_anyvector_ty], [LLVMMatchType<0>]>;
  def int_tpc_vector_reduce_mul  : TPCIntrin<[llvm_anyvector_ty], [LLVMMatchType<0>]>;
  def int_tpc_vector_reduce_and  : TPCIntrin<[llvm_anyvector_ty], [LLVMMatchType<0>]>;
  def int_tpc_vector_reduce_or  : TPCIntrin<[llvm_anyvector_ty], [LLVMMatchType<0>]>;
  def int_tpc_vector_reduce_xor  : TPCIntrin<[llvm_anyvector_ty], [LLVMMatchType<0>]>;
  def int_tpc_vector_reduce_smax  : TPCIntrin<[llvm_anyvector_ty], [LLVMMatchType<0>]>;
  def int_tpc_vector_reduce_smin  : TPCIntrin<[llvm_anyvector_ty], [LLVMMatchType<0>]>;
  def int_tpc_vector_reduce_umax  : TPCIntrin<[llvm_anyvector_ty], [LLVMMatchType<0>]>;
  def int_tpc_vector_reduce_umin  : TPCIntrin<[llvm_anyvector_ty], [LLVMMatchType<0>]>;
  def int_tpc_vector_reduce_fmax  : TPCIntrin<[llvm_anyvector_ty], [LLVMMatchType<0>]>;
  def int_tpc_vector_reduce_fmin  : TPCIntrin<[llvm_anyvector_ty], [LLVMMatchType<0>]>;
}

//===------------------------ MMIO access Intrinsics ----------------------===//
let IntrProperties = [IntrReadMem, IntrWillReturn] in {
  def int_tpc_get_dim_size   : TPCIntrin<[llvm_i32_ty], [llvm_i32_ty, llvm_i32_ty]>;
  def int_tpc_get_dim_stride : TPCIntrin<[llvm_i32_ty], [llvm_i32_ty, llvm_i32_ty]>;

  def int_tpc_get_tensor_hwpref_stride : TPCIntrin<[llvm_i32_ty], [llvm_i32_ty]>;

  def int_tpc_get_pad_value : TPCIntrin<[llvm_any_ty],   [llvm_i32_ty]>;
}

let IntrProperties = [IntrWriteMem, IntrWillReturn] in {
  def int_tpc_set_dim_size   : TPCIntrin<[], [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty]>;
  def int_tpc_set_dim_stride : TPCIntrin<[], [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty]>;

  def int_tpc_set_tensor_hwpref_stride : TPCIntrin<[], [llvm_i32_ty, llvm_i32_ty]>;

  def int_tpc_set_pad_value : TPCIntrin<[], [llvm_i32_ty, llvm_any_ty]>;
}

let IntrProperties = [IntrWillReturn] in {
  def int_tpc_get_semaphore_value : TPCIntrin<[llvm_i32_ty]>;
  def int_tpc_set_semaphore_value : TPCIntrin<[], [llvm_i32_ty]>;

  def int_tpc_get_csr_value : TPCIntrin<[llvm_i32_ty]>;
  def int_tpc_set_csr_value : TPCIntrin<[], [llvm_i32_ty]>;

  def int_tpc_get_convert_csr_value : TPCIntrin<[llvm_i32_ty]>;
  def int_tpc_set_convert_csr_value : TPCIntrin<[], [llvm_i32_ty]>;
}

}

