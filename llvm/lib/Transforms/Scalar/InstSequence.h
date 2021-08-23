//===---------------------------- InstSequence.h ---------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===-----------------------------------------------------------------------===//
// 
//===-------------------------------------------------------------------------===//


#ifndef LLVM_TRANSFORM_SCALAR_INSTSEQ_H
#define LLVM_TRANSFORM_SCALAR_INSTSEQ_H

#include "TPCOptUtils.h"

namespace llvm {

// TODO: change this if this class later gets associated with ClusterInfo pass
#define DEBUG_TYPE "loop-swp"

#define INSTSEQ_DEBUG(x) LLVM_DEBUG(dbgs() << "[InstSeq] " << x << "\n");

class LoadStoreSequence;

class InstSequence {
public:
  // A Pivot is an instruction that is used as an identifier for any
  // InstSequence object. The instruction sequence is formed/populated by
  // crawling over the DAG of definitions reaching this Pivot.

  // The LoadStore instruction argument is the pivot used for crawling over
  // the pre-load or pre-store instructions
  //
  // - L is the WorkingLoop
  // - PivotBT is the type of the Loop Block the PivotInst instruction belongs
  //   to
  InstSequence(Instruction *LoadStore, Loop *L, BlockType Bt,
               InstSequenceMapType &InstSeqMap, LoadStoreSequence *LoadStoreSeq,
               InstructionSetType &InductionInsts, bool IsCoordSeq)
      : PivotInst(LoadStore), WorkingLoop(L), PivotBT(Bt),
        InstSeqMap(InstSeqMap), LoadStoreSeq(LoadStoreSeq),
        InductionInsts(InductionInsts), IsCoordSeq(IsCoordSeq) {
    assert(LoadStore && "Invalid Pivot LoadStore");
    assert(isValidPivot() && "Invalid pivot instruction");
    assert(L && "Invalid working loop");
    if (!isLoadPivot())
      assert(IsCoordSeq &&
             "Store Pivot's Income-Pred sequence not supported as of now");
    assert(LoadStoreSeq && "LoadStoreSequence object ptr cannot be null");
    assert(Bt < BlockType::NumBlocks && "Invald BlockType");
    INSTSEQ_DEBUG("InstSequence Constructed.")
  }

  void init(bool SkipToClone = false, std::string Indent = "\t");

  // Populate the pre load/store sequence with pivot instruction as the
  // starting point
  bool populateSequence(InstInstMapType &InstUpdateMap,
                        std::string Indent = "\t");

  // get the pivot load/store instruction
  Instruction *getPivotInst() { return PivotInst; }

  Instruction *getDerivedFrom() { return DerivedFrom; }

  // get the working loop
  Loop *getWorkingLoop() { return WorkingLoop; }

  // get the pivot instruction's block type
  BlockType getPivotBT() { return PivotBT; }

  // get the pivot basic block ptr
  BasicBlock *getPivotBlock() {
    return TPCOptUtils::getLoopBlock(WorkingLoop, getPivotBT());
  }

  // get the LoadStoreSequence to which this InstSeq belongs to
  LoadStoreSequence *getLoadStoreSequence() { return LoadStoreSeq; }

  // get a reference instruction from the sequence
  Instruction *getRefInst();

  // check add mask presence
  bool hasAddMask() { return HasAddMask; }

  // check if the object is initialized
  bool isInitialized() { return IsInitialized; }

  // get insertion freeze status
  bool isInsertionFrozen() { return FreezeInsertion; }

  // get sequence freeze status
  bool isOrderingFrozen() { return FreezeOrdering; }

  // check if the object is a clone
  bool isClone() { return IsClone; }

  // check if the pivot is a load
  bool isLoadPivot() { return TPCOptUtils::isLoadTensor(PivotInst); }

  // check if the sequence originated from a coord operand
  bool isCoordSeq() { return IsCoordSeq; }

  // get whether the sequence is derived from another
  bool isDerived() { return IsDerived; }

  // get whether the sequence is derived from another load sequence
  bool isDerivedFromLoad() { return IsDerivedFromLoad; }

  // Check whether the sequence derives from other sequence only through an
  // Induction step. If true, return the Induction step Instruction, otherwise
  // return nullptr
  Instruction *derivesOnlyInduction() {
    return (DerivesOnlyInduction ? DerivedInductionStep : nullptr);
  }

  // get whether the sequence is empty
  bool isEmpty() { return IsEmpty; }

  // check whether the sequence is shared with / duplicate of another
  bool isShared() { return isEmpty() && isDerived(); }

  // get the set of phis collected
  InstructionSetType getPhis() { return PhiInstsSet; }

  // create a clone of the current PivotInst sequence object
  bool clone(
      InstSequence *CloneSource,      // reference for cloning
      InstCloneMapType &InstCloneMap, // map used to search/add clones of insts
      InstructionSetType &InsertedInsts, // set of insts inserted into BB
      BasicBlock::iterator &InsertIt,    // insertion point
      bool UseInsertIt, // flag indicating if InsertIt should be used as
                        // insertion point
      BlockType Bt,     // block type of the new INSTSEQ clone's Pivot
      std::string Indent = "\t");

  // debug dump
  void debugDump(std::string Indent = "\t");

private:
  // get the InstSeq vector
  InstructionVecType &getInstSeqVec() { return InstSeqVec; }

  // set whether the inserted instructions contain an add.mask instruction
  void setHasAddMask() { HasAddMask = true; }

  // set whether object is initialized
  void setIsInitialized() { IsInitialized = true; }

  // freeze insertion of instructions
  void freezeInsertion() { FreezeInsertion = true; }

  // freeze sequencing of instructions
  void freezeOrdering() { FreezeOrdering = true; }

  // unfreeze insertion
  void unfreezeInsertion() { FreezeInsertion = false; }

  // unfreeze ordering
  void unfreezeOrdering() { FreezeOrdering = false; }

  // set whether the object is a clone
  void setClone() { IsClone = true; }

  // set whether the sequence is derived from another
  void setDerived(Instruction *DerivedFromPivot) {
    IsDerived = true;
    DerivedFrom = DerivedFromPivot;
  }

  // set whether the sequence is derived from another load sequence
  void setDerivedFromLoad(Instruction *DerivedFromPivot, bool Flag = true) {
    if (Flag) {
      setDerived(DerivedFromPivot);
    }
    IsDerivedFromLoad = Flag;
  }

  // set whether the sequence is empty
  void setNotEmpty() { IsEmpty = false; }

  // analyze whether the pivot LoadStore instruction is valid
  bool isValidPivot() {
    return (TPCOptUtils::isLoadTensor(PivotInst) ||
            TPCOptUtils::isStoreTensor(PivotInst));
  }

  // insert the given instruction into the vector of instruction sequence
  bool insertIntoSequence(Instruction *I, std::string Indent = "\t");

  // once the insertion is frozen, convert the seq set into a vector of
  // instructions
  bool createOrderedSequence(std::string Indent = "\t");

  // visit the operands of the pivot instruction and include them in the
  // Sequence set
  bool crawlOperands(InstInstMapType &InstUpdateMap, std::string Indent = "\t");

  // util recursive method to crawl over operands of the given instruction
  bool crawlOperands(Instruction *I, InstInstMapType &InstUpdateMap,
                     std::string Indent = "\t");

  // create/get Phi node in Exit block
  Instruction *createExitPhi(Instruction *Phi, InstCloneMapType &InstCloneMap,
                             InstructionSetType &InsertedInsts,
                             std::string DebugTag = "\t");

  // create Phi node in Preheader block
  Instruction *createPreheaderPhi(Instruction *Phi, BasicBlock *PreheaderBlock,
                                  InstructionSetType &InsertedInsts,
                                  std::string DebugTag = "\t");

  // if the given Instruction I is connected to any Instruction that was cloned
  // earlier, patch those operands
  void patchOperandsWithClones(Instruction *I, BasicBlock *SourceBlock,
                               InstCloneMapType &InstCloneMap,
                               std::string DebugTag = "\t",
                               bool StrictPatch = true);

  // insert the clone of the given Instruction into the BasicBlock at an
  // appropriate insertion point
  void insertCloneIntoBlock(Instruction *I, InstCloneMapType &InstCloneMap,
                            InstructionSetType &InsertedInsts,
                            BasicBlock::iterator &InsertIt, bool UseInsertIt,
                            BlockType Bt, std::string DebugTag = "\t");

  // clone the given Instruction, and patch it's operands with their clones
  bool cloneInst(Instruction *I, BasicBlock *SourceBlock,
                 InstCloneMapType &InstCloneMap,
                 InstructionSetType &InsertedInsts, BlockType Bt,
                 bool ShouldCloneOnlyIndStep, std::string Indent = "\t");

  // create clones of insts and replace uses of old insts with clones
  bool cloneInsts(
      InstSequence *CloneSource,      // reference for cloning
      InstCloneMapType &InstCloneMap, // map used to search/add clones of insts
      InstructionSetType &InsertedInsts, // set of insts inserted into BB
      BasicBlock::iterator &InsertIt,    // insertion point
      bool UseInsertIt, // flag indicating if InsertIt should be used as
                        // insertion point
      BlockType Bt,     // block type of the new INSTSEQ clone's Pivot
      Instruction
          *CloneOnlyInductionInst, // if not nullptr, clone only Induction Insts
      std::string Indent = "\t");

  // DATA :

  Instruction *PivotInst = nullptr; // pivot instruction
  Loop *WorkingLoop = nullptr;      // working loop
  BlockType PivotBT;      // the basic block type the pivot belongs to

  Instruction *DerivedFrom =
      nullptr; // The pivot of the Seq from which this Seq is derived from
  Instruction *DerivedInductionStep =
      nullptr; // the only Induction var shared by this sequence with any other

  InstSequenceMapType
      &InstSeqMap; // mapping from instruction to INSTSEQ object ref
  LoadStoreSequence *LoadStoreSeq =
      nullptr; // the LoadStoreSequence the InstSeq belongs to
  InstructionSetType &InductionInsts; // set of Induction Insts

  InstructionVecType InstSeqVec; // vector of instruction sequence

  InstructionSetType
      InstSeqSet; // set of all pre load/store instructions inserted
  InstructionSetType
      PhiInstsSet; // set of all phi instructions discovered in the sequence

  bool IsCoordSeq = false; // is the sequence originating from a coord operand
  bool HasAddMask = false; // is true if there is at least one add.mask
                           // instruction in the sequence
  bool FreezeInsertion = false; // after this is changed to true, no more
                                // insertion into the sequence is allowed
  bool FreezeOrdering = false;  // after this is changed to true, no more
                                // insertion into the sequence is allowed
  bool IsClone = false; // is the object is created by cloning another one
  bool IsDerived =
      false; // is the sequence derived (due to coord update) from another
  bool IsDerivedFromLoad =
      false; // is the sequence derived from another Load coord sequence
  bool DerivesOnlyInduction = true; // is the sequence derived from another
                                    // sequence only through an Induction step
  bool IsInitialized = false; // is the sequence object initialized
  bool IsEmpty = true;        // is the Sequence empty
};

class LoadStoreSequence {
public:
  LoadStoreSequence(Instruction *Pivot, Loop *L, BlockType Bt,
                    InstSequenceMapType &InstSeqMap,
                    InstructionSetType &InductionInsts)
      : PivotInst(Pivot), WorkingLoop(L), PivotBT(Bt), InstSeqMap(InstSeqMap),
        InductionInsts(InductionInsts) {
    assert(Pivot && "Invalid Pivot LoadStore");
    assert(isValidPivot() && "Invalid pivot instruction");
    assert(L && "Invalid working loop");
    assert(Bt < BlockType::NumBlocks && "Invald BlockType");

    INSTSEQ_DEBUG("LoadStoreSequence Constructed.")
  }

  ~LoadStoreSequence() {
    if (CoordSeq)
      delete CoordSeq;
    if (IncomePredSeq)
      delete IncomePredSeq;
  }

  // get the pivot load instruction
  Instruction *getPivotInst() { return PivotInst; }

  // get the working loop
  Loop *getWorkingLoop() { return WorkingLoop; }

  // get the pivot instruction's block type
  BlockType getPivotBT() { return PivotBT; }

  // get the pivot basic block ptr
  BasicBlock *getPivotBlock() {
    return TPCOptUtils::getLoopBlock(WorkingLoop, getPivotBT());
  }

  // check if the pivot is a load
  bool isLoadPivot() { return TPCOptUtils::isLoadTensor(PivotInst); }

  // check if the income pred seq exists
  bool hasIncomePredSeq() { return (getIncomePredSequence() != nullptr); }

  // get the ptr to Coord Seq
  InstSequence *getCoordSequence() { return CoordSeq; }

  // get the ptr to IncomePred Seq
  InstSequence *getIncomePredSequence() { return IncomePredSeq; }

  // populate the coord sequence and if present, the income-pred sequence
  bool populateSequence(InstInstMapType &InstUpdateMap,
                        std::string Indent = "\t");

  // clone the coord sequence and if present, the income-pred sequence
  bool clone(LoadStoreSequence *CloneSource, InstCloneMapType &CloneMap,
             InstructionSetType &InsertedInsts,
             BasicBlock::iterator &InsertIt, // insertion point
             bool UseInsertIt, // flag indicating if InsertIt should be used as
                               // insertion point
             BlockType Bt, std::string Indent = "\t");

  // debug dump
  void debugDump(std::string Indent = "\t");

private:
  // analyze whether the pivot LoadStore instruction is valid
  bool isValidPivot() {
    return (TPCOptUtils::isLoadTensor(PivotInst) ||
            TPCOptUtils::isStoreTensor(PivotInst));
  }

  InstSequence *CoordSeq = nullptr; // sequence originating from Load Coord
  InstSequence *IncomePredSeq =
      nullptr; // sequence originating from Load Income and Predicate

  Instruction *PivotInst = nullptr; // pivot load instruction
  Loop *WorkingLoop = nullptr;      // working loop
  BlockType PivotBT;      // block type of the pivot instruction

  InstSequenceMapType
      &InstSeqMap; // mapping from instruction to INSTSEQ object ptr
  InstructionSetType &InductionInsts; // set of Induction Insts
};

} // end namespace llvm

#undef DEBUG_TYPE

#endif
