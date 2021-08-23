//===- TPCSchedule.td ---- TPC Scheduling Definitions ------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//===----------------------------------------------------------------------===//

// There are four SLOTS in TPC machine.
// This file describes that machine information.
//
//    |===========|
//    | LD_SLOT   |
//    |-----------|
//    | ST_SLOT   |
//    |-----------|
//    | VPU_SLOT  |
//    |-----------|
//    | SPU_SLOT  |
//    |===========|

//------------------------------------------------------------------------------
// Functional Units.
//------------------------------------------------------------------------------

def LD_SLOT       : FuncUnit;
def SPU_SLOT      : FuncUnit;
def VPU_SLOT      : FuncUnit;
def ST_SLOT       : FuncUnit;

// Field SrcD of VPU instruction is also SrcB in Load slot.
def SRC_BD        : FuncUnit;

// Field SrcC of VPU instruction is also SrcC in Store slot.
def SRC_C         : FuncUnit;

// Local memory address.
//
// According to PRM 1.3.4:
// LOAD and STORE issue slots share the same resource (spill RAM), and cannot
// access it simultaneously. On the LOAD issue slot, the LD_L* and LOOKUP*
// instructions access the spill RAM. On the STORE issue slot, all ST_L*
// instructions access this SRAM. The compiler should avoid scheduling both the
// stated instruction on the LOAD issue slot and the stated insruction on the
// STORE issue slot in the same VLIW instruction.
//
def LOCAL_MEM     : FuncUnit;

// Global memory address.
//
// PRM 1.3.4:
// ST_G and LD_G aren’t allowed to be scheduled in the same VLIW instruction.
//
def GLOBAL_MEM    : FuncUnit;

//------------------------------------------------------------------------------
// Instruction Itinerary Classes.
//------------------------------------------------------------------------------

// Load classes.
//--------------

def IIC_LoadOp                : InstrItinClass;
def IIC_LoadComplexOp         : InstrItinClass;     // MOV(ld) I1, I2
def IIC_LoadScalLocal         : InstrItinClass;     // LD_L  Dest, S0
def IIC_LoadVectLocal         : InstrItinClass;     // LD_L_V  Dest, S0, S1
def IIC_LoadShortVectLocal    : InstrItinClass;     // LD_L_V  Dest, S0 (Gaudi)
def IIC_LoadScalGlobal        : InstrItinClass;     // LD_G  Dest, A0
def IIC_LoadScalGlobalMask    : InstrItinClass;     // LD_G  b11111 I5, AD0
def IIC_LoadTensor            : InstrItinClass;     // LD_TNSR  V0, 4, I2

// Store classes.
//---------------

def IIC_StoreOp               : InstrItinClass;
def IIC_StoreComplexOp        : InstrItinClass;
def IIC_StoreScalLocal        : InstrItinClass;     // ST_L  S0, S1
def IIC_StoreVectLocal        : InstrItinClass;     // ST_L_V  S0, S1, V1
def IIC_StoreScalGlobal       : InstrItinClass;     // ST_G  S0, S2
def IIC_StoreTensor           : InstrItinClass;     // ST_TNSR  4, I2, V0

// Scalar classes.
//----------------

def IIC_ScalarOp              : InstrItinClass;

// Vector classes.
//----------------

def IIC_VectorOp              : InstrItinClass;     // ADD  V2, V1, V3
def IIC_VectorComplexOp       : InstrItinClass;

// Special classes.
//-----------------

def IIC_Loop                  : InstrItinClass;


//------------------------------------------------------------------------------
// Processor Itineraries Definition.
//------------------------------------------------------------------------------

def TPCItineraries :
  ProcessorItineraries<
    [LD_SLOT, ST_SLOT, VPU_SLOT, SPU_SLOT,
     SRC_BD, SRC_C,
     LOCAL_MEM, GLOBAL_MEM],
    [],
    [
      // Load classes.
      //--------------

      InstrItinData<IIC_LoadOp,
          [InstrStage<1, [LD_SLOT]>]>,

      InstrItinData<IIC_LoadComplexOp,
          [InstrStage<1, [LD_SLOT]>,
           InstrStage<1, [SRC_BD], 0>]>,

      InstrItinData<IIC_LoadScalLocal,
          [InstrStage<1, [SRC_BD], 0>,
           InstrStage<1, [LD_SLOT], 0>,
           InstrStage<1, [LOCAL_MEM]>]>,

      InstrItinData<IIC_LoadShortVectLocal,
          [InstrStage<1, [LD_SLOT], 0>,
           InstrStage<1, [LOCAL_MEM]>]>,

      InstrItinData<IIC_LoadVectLocal,
          [InstrStage<1, [SRC_BD], 0>,
           InstrStage<1, [LD_SLOT], 0>,
           InstrStage<1, [LOCAL_MEM]>]>,

      InstrItinData<IIC_LoadScalGlobal,
          [InstrStage<1, [LD_SLOT], 0>,
           InstrStage<1, [GLOBAL_MEM]>]>,

      InstrItinData<IIC_LoadScalGlobalMask,
          [InstrStage<1, [LD_SLOT], 0>,
           InstrStage<1, [SRC_BD], 0>,
           InstrStage<1, [GLOBAL_MEM]>]>,

      InstrItinData<IIC_LoadTensor,
          [InstrStage<1, [SRC_BD], 0>,
           InstrStage<1, [LD_SLOT], 0>]>,

      // Store classes.
      //---------------

      InstrItinData<IIC_StoreOp,
          [InstrStage<1, [ST_SLOT ]>]>,

      InstrItinData<IIC_StoreComplexOp,
          [InstrStage<1, [SRC_C], 0>,
           InstrStage<1, [ST_SLOT], 0>]>,

      InstrItinData<IIC_StoreScalLocal,
          [InstrStage<1, [SRC_C], 0>,
           InstrStage<1, [ST_SLOT], 0>,
           InstrStage<1, [LOCAL_MEM]>]>,

      InstrItinData<IIC_StoreVectLocal,
          [InstrStage<1, [SRC_C], 0>,
           InstrStage<1, [ST_SLOT], 0>,
           InstrStage<1, [LOCAL_MEM]>]>,

      InstrItinData<IIC_StoreScalGlobal,
          [InstrStage<1, [SRC_C], 0>,
           InstrStage<1, [ST_SLOT], 0>,
           InstrStage<1, [GLOBAL_MEM]>]>,

      InstrItinData<IIC_StoreTensor,
          [InstrStage<1, [SRC_C], 0>,
           InstrStage<1, [ST_SLOT], 0>]>,

      // Scalar classes.
      //----------------

      InstrItinData<IIC_ScalarOp,
          [InstrStage<1, [SPU_SLOT]>]>,

      // Vector classes.
      //----------------

      InstrItinData<IIC_VectorOp,
          [InstrStage<1, [VPU_SLOT]>]>,

      InstrItinData<IIC_VectorComplexOp,
          [InstrStage<1, [SRC_BD], 0>,
           InstrStage<1, [SRC_C], 0>,
           InstrStage<1, [VPU_SLOT]>]>,

      // Special classes.
      //-----------------

      InstrItinData<IIC_Loop,
          [InstrStage<1, [LD_SLOT ], 0>,
           InstrStage<1, [SPU_SLOT], 0>,
           InstrStage<1, [VPU_SLOT], 0>,
           InstrStage<1, [ST_SLOT ]>]>
    ]>;

//------------------------------------------------------------------------------
// Scheduler Model.
//------------------------------------------------------------------------------

def TPCModel : SchedMachineModel {
  // Max issue per cycle == bundle width.
  let IssueWidth = 4;
  let Itineraries = TPCItineraries;
  let LoadLatency = 1;
  let CompleteModel = 0;
  let MicroOpBufferSize = 0; // In-order execution, always hide latency.
  let PostRAScheduler = 1;
}
