//===-- TPCSelectionDAGInfo.cpp ---- TPC SelectionDAG Info ----------------===//
//
//
//===----------------------------------------------------------------------===//
//
// This file implements the TPCSelectionDAGInfo class.
//
//===----------------------------------------------------------------------===//

#include "TPCTargetMachine.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
using namespace llvm;

#define DEBUG_TYPE "tpc-selectiondag-info"

SDValue TPCSelectionDAGInfo::EmitTargetCodeForMemset(
    SelectionDAG &DAG, const SDLoc &dl, SDValue Chain, SDValue Dst, SDValue Src,
    SDValue Size, Align Alignment, bool isVolatile,
    MachinePointerInfo DstPtrInfo) const {
  ConstantSDNode *V = dyn_cast<ConstantSDNode>(Src);
  assert(V && "Fill value of memset must be a compile time constant");
  ConstantSDNode *SizeValue = dyn_cast<ConstantSDNode>(Size);
  assert(SizeValue && "Size in memset call must be a compile time constant");
  int AS = DstPtrInfo.getAddrSpace();
  assert((AS == 1 || AS == 2) && "memset may be called only for local memory");
  unsigned Sz = SizeValue->getZExtValue();
  unsigned UnitSz = (AS == 1) ? 4 : 256;
  assert((Sz % UnitSz == 0) && "Size in memset must be a multiple of space unit");
  MVT VT = (AS == 1) ? MVT::i32 : MVT::v64i32;

  const TPCSubtarget &STI = DAG.getMachineFunction().getSubtarget<TPCSubtarget>();
  const TPCTargetLowering &TLI = *STI.getTargetLowering();
  const TargetRegisterClass *RC = TLI.getRegClassFor(VT);
  MachineFunction &MF = DAG.getMachineFunction();

  // Build i32 value of four i8 values
  uint64_t Val = V->getZExtValue() & 255;
  assert(V->getZExtValue() == Val && "Non-char value in memset?");
  Val = (Val << 8)  | Val;
  Val = (Val << 16) | Val;

  unsigned ValueReg = MF.getRegInfo().createVirtualRegister(RC);
  Chain = DAG.getCopyToReg(Chain, dl, ValueReg, DAG.getConstant(Val, dl, VT));
  SDValue ZeroV = DAG.getCopyFromReg(Chain, dl, ValueReg, VT);
  Chain = ZeroV.getValue(1);
  unsigned DstOffs = 0;

  SmallVector<SDValue, 8> Chains;
  for (unsigned I = 0, SZ = Sz / UnitSz; I != SZ; ++I) {
    SDValue NewChain = DAG.getStore(Chain, dl, ZeroV,
                                    DAG.getMemBasePlusOffset(Dst, TypeSize::Fixed(DstOffs), dl),
                                    DstPtrInfo.getWithOffset(DstOffs), Alignment);
    Chains.push_back(NewChain);
    DstOffs += UnitSz;
  }

  return DAG.getNode(ISD::TokenFactor, dl, MVT::Other, Chains);
}

SDValue TPCSelectionDAGInfo::EmitTargetCodeForMemcpy(
    SelectionDAG &DAG, const SDLoc &dl, SDValue Chain,
    SDValue Dst, SDValue Src, SDValue Size,
    Align Alignment, bool isVolatile, bool AlwaysInline,
    MachinePointerInfo DstPtrInfo, MachinePointerInfo SrcPtrInfo) const {
  // Prepare arguments making sanity checks.
  ConstantSDNode *SizeValue = dyn_cast<ConstantSDNode>(Size);
  assert(SizeValue && "Size in memset call must be a compile time constant");
  assert(SrcPtrInfo.getAddrSpace() == DstPtrInfo.getAddrSpace() &&
         "Cannot memcpy between different address spaces");
  int AS = DstPtrInfo.getAddrSpace();
  assert((AS == 1 || AS == 2) && "memcpy may be called only for local memory");
  unsigned Sz = SizeValue->getZExtValue();
  unsigned UnitSz = (AS == 1) ? 4 : 256;
  assert((Sz % UnitSz == 0) && "Size in memcpy must be a multiple of space unit");

  // Prepare working variables.
  MVT VT = (AS == 1) ? MVT::i32 : MVT::v64i32;

  // Move data in a loop.
  SmallVector<SDValue, 8> Chains;
  for (unsigned I = 0, SZ = Sz / UnitSz, Offs = 0; I != SZ; ++I, Offs += UnitSz) {
    SDValue Value = DAG.getLoad(VT, dl, Chain,
                                DAG.getMemBasePlusOffset(Src, TypeSize::Fixed(Offs), dl, SDNodeFlags()),
                                SrcPtrInfo, Alignment);
    SDValue NewChain = DAG.getStore(Value.getValue(1), dl, Value,
                                    DAG.getMemBasePlusOffset(Dst, TypeSize::Fixed(Offs), dl),
                                    DstPtrInfo.getWithOffset(Offs), Alignment);
    Chains.push_back(NewChain);
  }

  return DAG.getNode(ISD::TokenFactor, dl, MVT::Other, Chains);
}
