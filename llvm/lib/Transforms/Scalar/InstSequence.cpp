//===---------------------------- InstSequence.cpp ---------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===-------------------------------------------------------------------------===//
// 
//===-------------------------------------------------------------------------===//

#include "InstSequence.h"

using namespace llvm;

#define DEBUG_TYPE "loop-swp"

void InstSequence::debugDump(std::string Indent) {
  LLVM_DEBUG(
      INSTSEQ_DEBUG(Indent << "[-- DumpBegin --]");
      INSTSEQ_DEBUG(Indent << "Load/Store : " << *PivotInst);
      INSTSEQ_DEBUG(Indent << "Pivot BlockType : "
                           << TPCOptUtils::getBasicBlockStr(PivotBT));
      INSTSEQ_DEBUG(
          Indent << "BasicBlock : "
                 << TPCOptUtils::getLoopBlock(WorkingLoop, PivotBT)->getName());
      INSTSEQ_DEBUG(Indent << "Sequence : {");

      if (isInsertionFrozen()) {
        for (auto I : InstSeqVec)
          INSTSEQ_DEBUG(Indent << "\t" << *I);
      } else {
        for (auto I : InstSeqSet)
          INSTSEQ_DEBUG(Indent << "\t" << *I);
      } INSTSEQ_DEBUG(Indent << "}");
      INSTSEQ_DEBUG(Indent << "Phi Insts : {");
      for (auto Phi
           : PhiInstsSet) INSTSEQ_DEBUG(Indent << "\t" << *Phi);
      INSTSEQ_DEBUG(Indent << "}");
      Instruction *InductionDerived = derivesOnlyInduction();
      if (InductionDerived) {
        INSTSEQ_DEBUG(Indent << "InductionDerived = " << *InductionDerived);
      } else {
        INSTSEQ_DEBUG(Indent << "InductionDerived = nullptr");
      }
      INSTSEQ_DEBUG(Indent << "IsEmpty : " << isEmpty());
      INSTSEQ_DEBUG(Indent << "HasAddMask : " << hasAddMask());
      INSTSEQ_DEBUG(Indent << "FreezeInsertion : " << isInsertionFrozen());
      INSTSEQ_DEBUG(Indent << "FreezeOrdering : " << isOrderingFrozen());
      INSTSEQ_DEBUG(Indent << "IsClone : " << isClone());
      INSTSEQ_DEBUG(Indent << "IsCoordSeq : " << isCoordSeq());
      INSTSEQ_DEBUG(Indent << "IsDerived : " << isDerived());
      INSTSEQ_DEBUG(Indent << "IsDerivedFromLoad : " << isDerivedFromLoad());
      INSTSEQ_DEBUG(Indent << "[--  DumpEnd  --]"));
}

void InstSequence::init(bool SkipToClone, std::string Indent) {
  std::string DebugTag = "<initInstSequence> " + Indent;
  if (isInitialized()) {
    INSTSEQ_DEBUG(DebugTag << "Already initialized, nothing to do ...")
    return;
  }

  auto It = InstSeqMap.find(getPivotInst());
  if (It != InstSeqMap.end()) {
    INSTSEQ_DEBUG(
        DebugTag << "Pivot Inst is already associated with an InstSeq :")
    It->second->debugDump(Indent + "\t");
    assert(It->second->isCoordSeq() && !isCoordSeq() &&
           "Pivot insts must be mapped to CoordSeq objects alone");
  } else {
    INSTSEQ_DEBUG(DebugTag << "Pivot Inst not associated with any InstSeq yet")
    assert(isCoordSeq() &&
           "CoordSeq objects must be populated before Income-Pred Seq");
    INSTSEQ_DEBUG(DebugTag << "Inserting into InstSeqMap ...")
    InstSeqMap.insert(std::make_pair(getPivotInst(), this));
  }

  // if the Sequence object is being initialized for cloning without population
  // and ordering
  if (SkipToClone) {
    freezeInsertion();
    freezeOrdering();
  }

  setIsInitialized();
}

bool InstSequence::insertIntoSequence(Instruction *I, std::string Indent) {
  std::string DebugTag = "<insertIntoSequence> " + Indent;
  INSTSEQ_DEBUG(DebugTag << "inserting into sequence : ")
  INSTSEQ_DEBUG(DebugTag << *I)

  if (isOrderingFrozen()) {
    INSTSEQ_DEBUG(DebugTag << "Insertion not allowed now, exiting ...")
    return false;
  }

  auto It = InstSeqMap.find(I);
  if (It != InstSeqMap.end()) {
    INSTSEQ_DEBUG(
        DebugTag << "Instruction already belongs to a INSTSEQ, exiting ...")
    return false;
  }

  if (isa<PHINode>(I)) {
    INSTSEQ_DEBUG(DebugTag << "(Phi node collected)")
    PhiInstsSet.insert(I);
  }

  if (HasAddMask ||
      TPCOptUtils::isIntrinsicOfType(I, Intrinsic::tpc_add_mask)) {
    INSTSEQ_DEBUG(DebugTag << "AddMask type instruction inserted.")
    HasAddMask = true;
  }
  InstSeqSet.insert(I);
  setNotEmpty();

  InstSeqMap.insert(std::make_pair(I, this));

  return true;
}

Instruction *InstSequence::getRefInst() {
  Instruction *RefInst = nullptr;
  Instruction *Pivot = getPivotInst();
  if (isCoordSeq()) {
    RefInst = cast<Instruction>(Pivot->getOperand(0));
  } else {
    for (unsigned OpIdx = 0; OpIdx < Pivot->getNumOperands(); OpIdx++) {
      if (auto OpDef = dyn_cast<Instruction>(Pivot->getOperand(OpIdx))) {
        if (OpDef->getParent() != getPivotBlock())
          continue;
        RefInst = OpDef;
        break;
      }
    }
  }

  return RefInst;
}

bool InstSequence::crawlOperands(Instruction *I, InstInstMapType &InstUpdateMap,
                                 std::string Indent) {
  std::string DebugTag = "<crawlOperands> " + Indent;
  INSTSEQ_DEBUG(DebugTag << "Crawling over operands of instruction :")
  INSTSEQ_DEBUG(DebugTag << *I)

  if (I != getPivotInst()) {
    auto It = InstSeqMap.find(I);
    if (It != InstSeqMap.end()) {
      auto OtherInstSeq = It->second;
      if (OtherInstSeq == this) {
        INSTSEQ_DEBUG(
            DebugTag
            << "Nothing to do, instruction already visited, continuing ...")
        return true;
      } else {
        setDerived(OtherInstSeq->getPivotInst());

        // boundary conflict: CoordSeq has to be standalone and not dependant on
        // Income-Predicate sequence
        // this restriction is imposed for now to avoid cycle in the DAG
        if (isCoordSeq() && !OtherInstSeq->isCoordSeq()) {
          INSTSEQ_DEBUG(DebugTag
                        << "Pattern not supported : instruction belongs"
                           " to a Coord sequence as well as an "
                           "Income-Predicate sequence, exiting ...")
          return false;
        }

        if (OtherInstSeq->isLoadPivot() || OtherInstSeq->isDerivedFromLoad())
          setDerivedFromLoad(OtherInstSeq->getPivotInst());

        // a pivot instruction can have two sequences - coord and income-pred
        // if two seq.s belong to the same pivot inst, then both cannot be
        // coord sequences or income-pred sequences together
        if (OtherInstSeq->getPivotInst() == getPivotInst() &&
            (OtherInstSeq->isCoordSeq() == isCoordSeq())) {
          INSTSEQ_DEBUG(DebugTag
                        << "Duplicate Coord/Income-Pred sequences, exiting ...")
          return false;
        } else {
          if (OtherInstSeq->isLoadPivot()) {
            DerivesOnlyInduction &= (InductionInsts.find(I) != InductionInsts.end());
            if (DerivesOnlyInduction) {
              // if an Induction Inst is visited already
              if (DerivedInductionStep) {
                // second Induction derivation not supported
                if (I != DerivedInductionStep) {
                  DerivedInductionStep = nullptr;
                  DerivesOnlyInduction = false;
                }
              } else {
                // remember the newly discovered derived Induction
                DerivedInductionStep = I;
              }
            } else {
              // ensure the derived Induction is reset in case it was set earlier
              DerivedInductionStep = nullptr;
            }
          }

          INSTSEQ_DEBUG(DebugTag
                        << "Nothing to do, instruction already belongs to "
                           "another INSTSEQ, continuing ...")
          return true;
        }
      }
    }

    if (!insertIntoSequence(I, Indent)) {
      INSTSEQ_DEBUG(DebugTag << "Inst insertion failed, exiting ...")
      return false;
    }

    // dont crawl over the operands of the phi nodes
    if (isa<PHINode>(I))
      return true;
  }

  // crawl over instruction's operands
  unsigned StartOpIdx = 0;
  unsigned EndOpIdx = I->getNumOperands();
  if (I == getPivotInst()) {
    if (isCoordSeq())
      EndOpIdx = 1;
    else
      StartOpIdx = 1;
  }
  for (unsigned OpIdx = StartOpIdx; OpIdx < EndOpIdx; OpIdx++) {
    if (auto OpDef = dyn_cast<Instruction>(I->getOperand(OpIdx))) {
      INSTSEQ_DEBUG(DebugTag << "OpIdx = " << OpIdx)
      INSTSEQ_DEBUG(DebugTag << "OpDef = " << *OpDef)
      // values defined outside the pivot basic block
      if (OpDef->getParent() != I->getParent()) {
        INSTSEQ_DEBUG(DebugTag
                      << "Reached a different BasicBlock, continuing ...")
        continue;
      }
      if (!crawlOperands(OpDef, InstUpdateMap, Indent + "\t")) {
        INSTSEQ_DEBUG(DebugTag << "Crawling over operands failed, exiting ...")
        return false;
      }
    }
  }

  return true;
}

bool InstSequence::crawlOperands(InstInstMapType &InstUpdateMap,
                                 std::string Indent) {
  std::string DebugTag = "<crawlOperands> " + Indent;

  if (!crawlOperands(getPivotInst(), InstUpdateMap, Indent)) {
    INSTSEQ_DEBUG(DebugTag << "Recursive crawling failed, exiting ...")
    return false;
  }

  return true;
}

bool InstSequence::populateSequence(InstInstMapType &InstUpdateMap,
                                    std::string Indent) {
  std::string DebugTag = "<populateInstSeq> ";

  if (!isInitialized()) {
    INSTSEQ_DEBUG(
        DebugTag << "Sequence object not yet initialized, exiting ...")
    return false;
  }

  INSTSEQ_DEBUG(DebugTag << "----")
  INSTSEQ_DEBUG(DebugTag << "populating Instruction Sequence for Pivot :")
  INSTSEQ_DEBUG(DebugTag << *getPivotInst())
  INSTSEQ_DEBUG(DebugTag << "IsCoordSeq = " << isCoordSeq())

  if (isInsertionFrozen()) {
    INSTSEQ_DEBUG(
        DebugTag
        << "Cannot populate sequence : Insertion is frozen, exiting ...")
    return false;
  }

  if (!crawlOperands(InstUpdateMap, Indent + "\t")) {
    INSTSEQ_DEBUG(DebugTag << "Crawling over operands failed, exiting ...")
    return false;
  }

  freezeInsertion();

  if (!createOrderedSequence(Indent + "\t")) {
    INSTSEQ_DEBUG(DebugTag << "Sequence ordering failed, exiting ...")
    return false;
  }

  INSTSEQ_DEBUG(DebugTag << "----")

  return true;
}

bool InstSequence::createOrderedSequence(std::string Indent) {
  std::string DebugTag = "<createOrderedSequence> " + Indent;
  INSTSEQ_DEBUG(DebugTag << "Begin")

  if (!isInsertionFrozen()) {
    INSTSEQ_DEBUG(
        DebugTag
        << "Cannot order sequence : Insertion is not frozen yet, exiting ...")
    return false;
  }
  if (isOrderingFrozen()) {
    INSTSEQ_DEBUG(
        DebugTag << "Nothing to do : ordering already done, exiting ...")
    return true;
  }

  // copy to vec
  InstSeqVec.resize(InstSeqSet.size());
  std::copy(InstSeqSet.begin(), InstSeqSet.end(), InstSeqVec.begin());

  if (InstSeqSet.size() == 1) {
    freezeOrdering();
    INSTSEQ_DEBUG(DebugTag << "End")
    return true;
  }

  // level order the instructions in the set
  INSTSEQ_DEBUG(DebugTag << "Level ordering the Pre-LoadStore sequence")
  //
  // init levels
  InstNumMapType InstLevelMap;
  for (auto I : InstSeqVec)
    InstLevelMap.insert(std::make_pair(I, 0));
  //
  bool Change = true;
  while (Change) {
    Change = false;
    for (auto I : InstSeqVec) {
      if (isa<PHINode>(I))
        continue;
      unsigned PrevLevel = InstLevelMap[I];
      unsigned MaxLevel = 0;
      // calculate the new level using the operands' level info
      for (unsigned OpIdx = 0; OpIdx < I->getNumOperands(); OpIdx++) {
        if (auto OpDef = dyn_cast<Instruction>(I->getOperand(OpIdx))) {
          // if operand is outisde pivot BB
          if (OpDef->getParent() != getPivotBlock())
            continue;
          // if operand not in this sequence
          if (InstSeqSet.find(OpDef) == InstSeqSet.end())
            continue;

          // graceful exit on unforeseen case
          auto It = InstLevelMap.find(OpDef);
          if (It == InstLevelMap.end()) {
            INSTSEQ_DEBUG(DebugTag << "\t"
                                   << "An OpDef : ")
            INSTSEQ_DEBUG(DebugTag << "\t" << *OpDef)
            INSTSEQ_DEBUG(DebugTag << "\t"
                                   << "not found in LevelMap, exiting ...")
            return false;
          }
          MaxLevel = (MaxLevel > It->second) ? MaxLevel : It->second;
        }
      }
      InstLevelMap[I] = MaxLevel + 1;
      Change |= (PrevLevel != InstLevelMap[I]);
    }
  }

  // sort the vec in level order
  std::sort(InstSeqVec.begin(), InstSeqVec.end(),
            [&](Instruction *A, Instruction *B) -> bool {
              return (InstLevelMap[A] > InstLevelMap[B]);
            });

  freezeOrdering();

  INSTSEQ_DEBUG(DebugTag << "End")
  return true;
}

bool InstSequence::clone(InstSequence *CloneSource, InstCloneMapType &CloneMap,
                         InstructionSetType &InsertedInsts,
                         BasicBlock::iterator &InsertIt, bool UseInsertIt,
                         BlockType Bt, std::string Indent) {
  std::string DebugTag = "<cloneSequence> " + Indent;

  INSTSEQ_DEBUG(DebugTag << "cloning from the sequence : ")
  CloneSource->debugDump();

  if (!isOrderingFrozen()) {
    INSTSEQ_DEBUG(
        DebugTag
        << "Cloning not allowed until sequence ordering is frozen, exiting ...")
    return false;
  }

  if (Bt == getPivotBT() && isLoadPivot()) {
    INSTSEQ_DEBUG(
        DebugTag
        << "Cloning not allowed for non-store pivot within the same BasicBlock")
    return false;
  }

  if (CloneSource->isLoadPivot()) {
    unfreezeInsertion();
    unfreezeOrdering();
    // clone only the Induction Inst if 'this' seq has store pivot and it
    // derives only the Induction Inst from the Source
    Instruction *CloneOnlyInductionInst = derivesOnlyInduction();
    CloneOnlyInductionInst = !isLoadPivot() ? CloneOnlyInductionInst : nullptr;
    if (!cloneInsts(CloneSource, CloneMap, InsertedInsts, InsertIt, UseInsertIt,
                    Bt, CloneOnlyInductionInst, Indent)) {
      INSTSEQ_DEBUG(DebugTag << "Load - Load/Store cloning failed, exiting ...")
      freezeInsertion();
      freezeOrdering();
      return false;
    }
  } else if (!isLoadPivot() && !CloneSource->isLoadPivot()) {
    if (Bt != BlockType::Exit) {
      INSTSEQ_DEBUG(DebugTag << "Store sequences will be cloned only if "
                                "derived from Load, exiting ...")
      return false;
    }
    unfreezeInsertion();
    unfreezeOrdering();
    if (!cloneInsts(CloneSource, CloneMap, InsertedInsts, InsertIt, UseInsertIt,
                    Bt, nullptr, Indent)) {
      INSTSEQ_DEBUG(DebugTag << "Store - Store cloning failed, exiting ...")
      freezeInsertion();
      freezeOrdering();
      return false;
    }
  } else {
    INSTSEQ_DEBUG(DebugTag << "Derivation from Store to Load sequence is "
                              "prohibited by design, exiting ...")
    return false;
  }

  if (CloneSource->hasAddMask())
    setHasAddMask();

  // both loads and stores can derive from loads
  if (CloneSource->isDerivedFromLoad()) {
    // after cloning, stores will no longer derive from loads
    setDerivedFromLoad(CloneSource->getPivotInst(), isLoadPivot());
    DerivesOnlyInduction = false;
    DerivedInductionStep = nullptr;
  } else { // only stores can derive from stores
    if (CloneSource->isDerived())
      setDerived(CloneSource->getPivotInst());
  }
  setClone();

  freezeInsertion();
  createOrderedSequence();

  return true;
}

Instruction *InstSequence::createExitPhi(Instruction *Phi,
                                         InstCloneMapType &InstCloneMap,
                                         InstructionSetType &InsertedInsts,
                                         std::string DebugTag) {
  INSTSEQ_DEBUG(DebugTag << "Creating Exit phi clone ...")

  Instruction *PhiClone = nullptr;
  auto It = InstCloneMap.find(Phi);
  if (It != InstCloneMap.end()) {
    INSTSEQ_DEBUG(DebugTag << "LCSSA Phi already exists, not cloning ...")
    InsertedInsts.insert(It->second);
    PhiClone = It->second;
  } else {
    // create Exit Phi for the given Phi of the Header
    PhiClone = Phi->clone();
  }

  return PhiClone;
}

Instruction *InstSequence::createPreheaderPhi(Instruction *Phi,
                                              BasicBlock *PreheaderBlock,
                                              InstructionSetType &InsertedInsts,
                                              std::string DebugTag) {
  INSTSEQ_DEBUG(DebugTag << "Creating preheader phi clone ...")
  // create Preheader Phi for the given Phi of the Header
  Instruction *PhiClone = nullptr;
  // get the Phi's incoming value from Preheader
  Value *PreheaderValue =
      (cast<PHINode>(Phi))->getIncomingValueForBlock(PreheaderBlock);
  Instruction *PreheaderInst = cast<Instruction>(PreheaderValue);
  if (PreheaderInst->getParent() != PreheaderBlock) {
    // create the Preheader clone
    INSTSEQ_DEBUG(DebugTag << "Preheader clone phi does not exist, cloning ...")
    auto *InstClonePN = PHINode::Create(PreheaderValue->getType(), 1);
    // TODO: assuming unnecessary PHI nodes will be eliminated later
    // for each predecessor of PreheaderBlock, add the same incoming value
    for (BasicBlock *Pred : predecessors(PreheaderBlock))
      InstClonePN->addIncoming(PreheaderValue, Pred);
    PhiClone = cast<Instruction>(InstClonePN);
  } else {
    INSTSEQ_DEBUG(
        DebugTag << "Preheader Income Inst already exists, linking ...")
    PhiClone = PreheaderInst;
    InsertedInsts.insert(PhiClone);
  }

  return PhiClone;
}

void InstSequence::insertCloneIntoBlock(Instruction *I,
                                        InstCloneMapType &InstCloneMap,
                                        InstructionSetType &InsertedInsts,
                                        BasicBlock::iterator &InsertIt,
                                        bool UseInsertIt, BlockType Bt,
                                        std::string DebugTag) {
  BasicBlock *BB = TPCOptUtils::getLoopBlock(WorkingLoop, Bt);
  BasicBlock::iterator PhiInsertIt = BB->begin();

  // not cloned yet
  auto CloneIt = InstCloneMap.find(I);
  assert(CloneIt != InstCloneMap.end() && "Cannot insert : no clone found");
  INSTSEQ_DEBUG(DebugTag << "IClone = " << *CloneIt->second)

  // already inserted
  auto InsertedInstIt = InsertedInsts.find(CloneIt->second);
  if (InsertedInstIt != InsertedInsts.end())
    return;

  bool InsertionFlag = false;
  if (UseInsertIt) { // use caller's custom insertion pt
    InsertionFlag = TPCOptUtils::insertIntoBlock(
        getWorkingLoop(), Bt, CloneIt->second, InsertedInsts, InsertIt,
        PhiInsertIt, DebugTag);
  } else { // use I's position as insertion pt
    BasicBlock::iterator InstInsertIt = I->getIterator();
    InsertionFlag = TPCOptUtils::insertIntoBlock(
        getWorkingLoop(), Bt, CloneIt->second, InsertedInsts, InstInsertIt,
        PhiInsertIt, DebugTag);
  }
  assert(InsertionFlag && "Incorrect insertion into BB");

  return;
}

// if I is connected to an Instruction that was cloned earlier, patch those
// operands
//
// e.g: I :
// %vecins = insertelement(ifmCoords, ...)
// where, ifmCoords has a clone ifmCoordsClone, we complete the clone to
// clone connection
//
// (Before) :
//  %ifmCoords       ----->  %vecins
// [%ifmCoordsClone]
//
// (Clone I = %vecins) :
//  %ifmCoords       ----->  %vecins
// [%ifmCoordsClone]   '--> [%vecinsClone]
//
// (Connect clones) :
//  %ifmCoords       ----->  %vecins
// [%ifmCoordsClone] -----> [%vecinsClone]
void InstSequence::patchOperandsWithClones(Instruction *I,
                                           BasicBlock *SourceBlock,
                                           InstCloneMapType &InstCloneMap,
                                           std::string DebugTag,
                                           bool StrictPatch) {
  unsigned StartIdx = 0;
  unsigned EndIdx = I->getNumOperands();
  if (I == getPivotInst()) {
    // For a pivot instruction, the patching is required in either the Coord
    // operand or in the Income-Pred operands
    StartIdx = isCoordSeq() ? StartIdx : 1;
    EndIdx = isCoordSeq() ? 1 : EndIdx;
  }

  for (unsigned OpIdx = StartIdx; OpIdx < EndIdx; OpIdx++) {
    if (auto OpDef = dyn_cast<Instruction>(I->getOperand(OpIdx))) {
      // before
      INSTSEQ_DEBUG(DebugTag << "\tOp = " << *OpDef)

      // skip the operands outside the block
      if (OpDef->getParent() != SourceBlock)
        continue;

      auto CloneIt = InstCloneMap.find(OpDef);
      if (CloneIt == InstCloneMap.end()) {
        if (!StrictPatch) {
          INSTSEQ_DEBUG(
              DebugTag
              << "Clone not found, patching for this operand skipped ...")
          INSTSEQ_DEBUG(DebugTag << "\tOp' = " << *I->getOperand(OpIdx))
          continue;
        }
        assert(false && "Cannot patch the operand without a clone");
      }

      I->setOperand(OpIdx, cast<Value>(CloneIt->second));
      // after
      INSTSEQ_DEBUG(DebugTag << "\tOp' = " << *I->getOperand(OpIdx))
    }
  }

  return;
}

bool InstSequence::cloneInst(Instruction *I, BasicBlock *SourceBlock,
                             InstCloneMapType &InstCloneMap,
                             InstructionSetType &InsertedInsts, BlockType Bt,
                             bool ShouldCloneOnlyIndStep, std::string Indent) {
  std::string DebugTag = "<cloneInst> " + Indent;
  INSTSEQ_DEBUG(DebugTag << "Cloning the Instruction : ")
  INSTSEQ_DEBUG(DebugTag << "I = " << *I)
  INSTSEQ_DEBUG(
      DebugTag << "(ShouldCloneOnlyIndStep = " << ShouldCloneOnlyIndStep << ")")
  BasicBlock *BB = TPCOptUtils::getLoopBlock(WorkingLoop, Bt);

  auto It = InstCloneMap.find(I);
  if (It != InstCloneMap.end()) {
    INSTSEQ_DEBUG(
        DebugTag << "Already cloned to another sequence, continuing ...")
    // not good when we want to clone only the Induction Steps
    return !ShouldCloneOnlyIndStep;
  }

  // create a clone
  Instruction *IClone = nullptr;
  if (ShouldCloneOnlyIndStep) {
    if (InductionInsts.find(I) == InductionInsts.end()) {
      INSTSEQ_DEBUG(DebugTag
                    << "Instruction is not an Induction step, continuing ...")
      return false;
    }
    // we need to split only the Induction var connection between the already
    // split load coords and store coords
    IClone = I->clone();
  } else if (isa<PHINode>(I)) {
    if (Bt == BlockType::Preheader)
      IClone = createPreheaderPhi(I, BB, InsertedInsts, DebugTag);
    else if (Bt == BlockType::Exit)
      IClone = createExitPhi(I, InstCloneMap, InsertedInsts, DebugTag);
    else
      IClone = I->clone();
  } else {
    IClone = I->clone();
  }

  InstCloneMap.insert(std::make_pair(I, IClone));
  insertIntoSequence(IClone, Indent + "\t");

  if (!isa<PHINode>(I)) {
    INSTSEQ_DEBUG(DebugTag << "Patching the operands of IClone ...")
    // strict patching is not needed if only Induction Inst is being cloned
    patchOperandsWithClones(IClone, SourceBlock, InstCloneMap, DebugTag,
                            !ShouldCloneOnlyIndStep);
  }

  return true;
}

bool InstSequence::cloneInsts(InstSequence *CloneSource,
                              InstCloneMapType &InstCloneMap,
                              InstructionSetType &InsertedInsts,
                              BasicBlock::iterator &InsertIt, bool UseInsertIt,
                              BlockType Bt, Instruction *CloneOnlyInductionInst,
                              std::string Indent) {
  std::string DebugTag = "<cloneInsts> " + Indent;
  bool IsLoadToStoreClone = (!isLoadPivot() && CloneSource->isLoadPivot());
  if (CloneOnlyInductionInst &&
      (Bt != BlockType::Header || !IsLoadToStoreClone)) {
    INSTSEQ_DEBUG(
        DebugTag << "Cloning of only InductionInst is allowed to be used only "
                    "for HeaderBlock's Load to Store clone (coord split)")
    return false;
  }

  auto &InstVec = CloneSource->getInstSeqVec();

  INSTSEQ_DEBUG(DebugTag << "Cloning ... [Begin]")

  if (CloneOnlyInductionInst) {
    if (!cloneInst(CloneOnlyInductionInst, CloneSource->getPivotBlock(),
                   InstCloneMap, InsertedInsts, Bt, true, Indent + "\t")) {
      INSTSEQ_DEBUG(DebugTag
                    << "Could not clone the Inst Sequence, exiting ...")
      return false;
    }

    INSTSEQ_DEBUG(DebugTag << "Patching the users of DerivedInductionStep ...")
    // Induction Inst's users in this sequence need to be patched up
    for (auto User : CloneOnlyInductionInst->users()) {
      Instruction *U = cast<Instruction>(User);
      INSTSEQ_DEBUG(DebugTag << "\tUser = " << *User)
      auto It = InstSeqMap.find(U);
      // if InstSeq of this User is not found or is not this sequence, there is
      // nothing to do
      if (It == InstSeqMap.end() || It->second != this)
        continue;
      patchOperandsWithClones(U, CloneSource->getPivotBlock(), InstCloneMap,
                              DebugTag, false);
    }

    insertCloneIntoBlock(CloneOnlyInductionInst, InstCloneMap, InsertedInsts,
                         InsertIt, UseInsertIt, Bt, DebugTag);

    INSTSEQ_DEBUG(DebugTag << "Cloning ... [End]")
    return true;
  }

  bool IsClonedNow = false;
  for (auto InstIt = InstVec.rbegin(); InstIt != InstVec.rend(); InstIt++) {
    Instruction *I = *InstIt;
    if (!cloneInst(I, CloneSource->getPivotBlock(), InstCloneMap, InsertedInsts,
                   Bt, false, Indent + "\t")) {
      // partial cloning not supported yet
      INSTSEQ_DEBUG(DebugTag
                    << "Could not fully clone the Inst Sequence, exiting ...")
      return false;
    }
    IsClonedNow = true;
  }

  INSTSEQ_DEBUG(DebugTag << "Patching operands of Pivot Inst ...")
  INSTSEQ_DEBUG(DebugTag << "Pivot : " << *getPivotInst())
  patchOperandsWithClones(getPivotInst(), CloneSource->getPivotBlock(),
                          InstCloneMap, DebugTag, false);

  // if the insts were cloned now
  if (IsClonedNow) {
    // insert all into block
    INSTSEQ_DEBUG(
        DebugTag << "Inserting cloned instructions into Basic Block ...")
    for (auto InstIt = InstVec.rbegin(); InstIt != InstVec.rend(); InstIt++) {
      Instruction *I = *InstIt;
      INSTSEQ_DEBUG(DebugTag << "\tI = " << *I)
      insertCloneIntoBlock(I, InstCloneMap, InsertedInsts, InsertIt,
                           UseInsertIt, Bt, DebugTag);
    }
  }

  INSTSEQ_DEBUG(DebugTag << "Cloning ... [End]")
  return true;
}

void LoadStoreSequence::debugDump(std::string Indent) {
  INSTSEQ_DEBUG(Indent << "[-- DumpBegin --]")
  INSTSEQ_DEBUG(Indent << "Pivot = " << *getPivotInst())

  INSTSEQ_DEBUG(Indent << "CoordSeq :")
  getCoordSequence()->debugDump(Indent + "\t");

  if (isLoadPivot()) {
    if (hasIncomePredSeq()) {
      INSTSEQ_DEBUG(Indent << "IncomePredSeq :")
      getIncomePredSequence()->debugDump(Indent + "\t");
    } else {
      INSTSEQ_DEBUG(Indent << "IncomePredSeq : null")
    }
  }

  INSTSEQ_DEBUG(Indent << "[--  DumpEnd  --]")
}

bool LoadStoreSequence::clone(LoadStoreSequence *CloneSource,
                              InstCloneMapType &CloneMap,
                              InstructionSetType &InsertedInsts,
                              BasicBlock::iterator &InsertIt, bool UseInsertIt,
                              BlockType Bt, std::string Indent) {
  std::string DebugTag = "<cloneLoadStoreSeq> ";
  INSTSEQ_DEBUG(DebugTag << "====")

  // TODO : clone source validation

  INSTSEQ_DEBUG(DebugTag << "cloning the Coord Sequence of Load/Store ...")
  CoordSeq = new InstSequence(getPivotInst(), getWorkingLoop(), getPivotBT(),
                              InstSeqMap, this, InductionInsts, true);
  CoordSeq->init(true, Indent + "\t");
  if (!CoordSeq->clone(CloneSource->getCoordSequence(), CloneMap, InsertedInsts,
                       InsertIt, UseInsertIt, Bt, Indent + "\t")) {
    INSTSEQ_DEBUG(
        DebugTag << "Coord Seq cloning for Load Pivot failed, exiting ...")
    return false;
  }

  // TODO: Income-Predicate sequence support is limited to Load Pivots for now
  // TODO: how to detect inadvertent access of Store's Income-Pred Seq?
  if (!isLoadPivot())
    return true;

  // TODO : extract method
  // check if there is a need for income-pred sequence
  bool HasNonConstOperands = false;
  Instruction *SrcPivotInst = CloneSource->getPivotInst();
  for (unsigned OpIdx = 1;
       OpIdx < SrcPivotInst->getNumOperands() && !HasNonConstOperands;
       OpIdx++) {
    if (auto OpDef = dyn_cast<Instruction>(SrcPivotInst->getOperand(OpIdx))) {
      if (OpDef->getParent() != SrcPivotInst->getParent())
        continue;
      HasNonConstOperands = true;
    }
  }
  if (!HasNonConstOperands) {
    INSTSEQ_DEBUG(
        DebugTag << "Income-Predicate Sequence does not exist, continuing ...")
    INSTSEQ_DEBUG(DebugTag << "====")
    return true;
  }

  INSTSEQ_DEBUG(
      DebugTag << "cloning the Income-Predicate Sequence of Load/Store ...")
  IncomePredSeq =
      new InstSequence(getPivotInst(), getWorkingLoop(), getPivotBT(),
                       InstSeqMap, this, InductionInsts, false);
  IncomePredSeq->init(true, Indent + "\t");
  if (!IncomePredSeq->clone(CloneSource->getIncomePredSequence(), CloneMap,
                            InsertedInsts, InsertIt, UseInsertIt, Bt,
                            Indent + "\t")) {
    INSTSEQ_DEBUG(
        DebugTag
        << "Income-Predicate Seq cloning for Load Pivot failed, exiting ...")
    return false;
  }

  INSTSEQ_DEBUG(DebugTag << "====")

  return true;
}

bool LoadStoreSequence::populateSequence(InstInstMapType &InstUpdateMap,
                                         std::string Indent) {
  std::string DebugTag = "<populateLoadStoreSeq> ";
  INSTSEQ_DEBUG(DebugTag << "====")

  INSTSEQ_DEBUG(DebugTag << "populating the Coord Sequence of Load/Store ...")
  CoordSeq = new InstSequence(getPivotInst(), getWorkingLoop(), getPivotBT(),
                              InstSeqMap, this, InductionInsts, true);
  CoordSeq->init(false, Indent + "\t");
  if (!CoordSeq->populateSequence(InstUpdateMap, Indent + "\t")) {
    INSTSEQ_DEBUG(
        DebugTag << "CoordSeq population for Load Pivot failed, exiting ...")
    return false;
  }

  // TODO: Income-Predicate sequence support is limited to Load Pivots for now
  // TODO: how to detect inadvertent access of Store's Income-Pred Seq?
  if (!isLoadPivot())
    return true;

  // check if there is a need for income-pred sequence
  bool HasNonConstOperands = false;
  for (unsigned OpIdx = 1;
       OpIdx < getPivotInst()->getNumOperands() && !HasNonConstOperands;
       OpIdx++) {
    if (auto OpDef = dyn_cast<Instruction>(getPivotInst()->getOperand(OpIdx))) {
      if (OpDef->getParent() != getPivotInst()->getParent())
        continue;
      HasNonConstOperands = true;
    }
  }
  if (!HasNonConstOperands) {
    INSTSEQ_DEBUG(
        DebugTag << "Income-Predicate Sequence does not exist, continuing ...")
    INSTSEQ_DEBUG(DebugTag << "====")
    return true;
  }

  INSTSEQ_DEBUG(
      DebugTag << "populating the Income-Predicate Sequence of Load/Store ...")
  IncomePredSeq =
      new InstSequence(getPivotInst(), getWorkingLoop(), getPivotBT(),
                       InstSeqMap, this, InductionInsts, false);
  IncomePredSeq->init(false, Indent + "\t");
  if (!IncomePredSeq->populateSequence(InstUpdateMap, Indent + "\t")) {
    INSTSEQ_DEBUG(
        DebugTag
        << "Income-Predicate Seq population for Load Pivot failed, exiting ...")
    return false;
  }

  INSTSEQ_DEBUG(DebugTag << "====")

  return true;
}

#undef DEBUG_TYPE
