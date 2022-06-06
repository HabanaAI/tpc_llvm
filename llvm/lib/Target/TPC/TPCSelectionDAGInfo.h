//===-- TPCSelectionDAGInfo.h - TPC SelectionDAG Info -----------*- C++ -*-===//
//
//===----------------------------------------------------------------------===//
//
// This file defines the TPC subclass for SelectionDAGTargetInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TPC_SELECTION_DAG_INFO_H
#define LLVM_TPC_SELECTION_DAG_INFO_H

#include "llvm/CodeGen/SelectionDAGTargetInfo.h"

namespace llvm {

/// Holds information about the TPC instruction selection DAG.
class TPCSelectionDAGInfo : public SelectionDAGTargetInfo {
public:
  SDValue EmitTargetCodeForMemset(SelectionDAG &DAG, const SDLoc &dl,
                                  SDValue Chain, SDValue Dst, SDValue Src,
                                  SDValue Size, Align Alignment, bool isVolatile,
                                  MachinePointerInfo DstPtrInfo) const override;
    SDValue EmitTargetCodeForMemcpy(SelectionDAG &DAG, const SDLoc &dl,
                                  SDValue Chain, SDValue Dst,
                                  SDValue Src, SDValue Size,
                                  Align Alignment, bool isVolatile,
                                  bool AlwaysInline,
                                  MachinePointerInfo DstPtrInfo,
                                  MachinePointerInfo SrcPtrInfo) const override;
};

} // end namespace llvm

#endif // LLVM_TPC_SELECTION_DAG_INFO_H
