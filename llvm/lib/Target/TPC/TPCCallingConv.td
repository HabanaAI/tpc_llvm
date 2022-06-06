//----------------------------------------------------------------------------//
//
// This describes the calling conventions for TPC architectures.
//
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// Return Value Calling Conventions
//----------------------------------------------------------------------------//

// No returns

def CC_TPC_PRIOR_DORON1 : CallingConv<[
  // Promote i1 arguments to i32.
  CCIfType<[i1], CCPromoteToType<i32>>,

  CCIfType<[i8, i16, i32, f8_143, f8_152, f16, bf16, f32],
           CCAssignToReg<[S0, S1, S2, S3, S4, S5, S6, S7,
                          S8, S9, S10, S11, S12, S13, S14, S15,
                          S16, S17, S18, S19, S20, S21, S22, S23,
                          S24, S25, S26, S27, S28, S29, S30]>>,

  CCIfType<[i8, i16, i32, f8_143, f8_152, f16, bf16, f32],
           CCCustom<"CC_TPC_EXTRA_ARGS">>
]>;

def CC_TPC_DORON1 : CallingConv<[
  // Promote i1 arguments to i32.
  CCIfType<[i1], CCPromoteToType<i32>>,
  
  CCIfType<[i8, i16, i32, f8_143, f8_152, f16, bf16, f32],
           CCAssignToReg<[S0, S1, S2, S3, S4, S5, S6, S7,
                          S8, S9, S10, S11, S16, S17, S18, S19,
                          S20, S21, S22, S23, S24, S25, S26, S27,
                          S28, S29, S30, S31, S32, S33, S34]>>,

  CCIfType<[i8, i16, i32, f8_143, f8_152, f16, bf16, f32],
           CCCustom<"CC_TPC_EXTRA_ARGS">>
]>;

def CC_TPC : CallingConv<[
  CCIf<"!static_cast<const TPCSubtarget&>"
       "(State.getMachineFunction().getSubtarget()).hasDoron1()",
       CCDelegateTo<CC_TPC_PRIOR_DORON1>>,
  CCIf<"static_cast<const TPCSubtarget&>"
       "(State.getMachineFunction().getSubtarget()).hasDoron1()",
       CCDelegateTo<CC_TPC_DORON1>>
]>;
