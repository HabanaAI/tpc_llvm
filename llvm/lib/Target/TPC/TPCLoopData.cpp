//===- TPCLoopData.cpp ----------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#include "TPCLoopData.h"
#include "llvm/IR/IntrinsicsTPC.h"

static unsigned getUnrollCountFromMetadata(MDNode *LoopMD) {
  if (!LoopMD || LoopMD->getNumOperands() == 0)
    return 1;

  MDNode *MD = nullptr;

  for (unsigned i = 1, e = LoopMD->getNumOperands(); i < e; ++i) {
    MD = dyn_cast<MDNode>(LoopMD->getOperand(i));
    if (!MD)
      continue;

    MDString *S = dyn_cast<MDString>(MD->getOperand(0));
    if (!S)
      continue;

    if (S->getString().equals("llvm.loop.machine.unroll.count")) {
      assert(MD->getNumOperands() == 2 &&
             "Unroll hint metadata should have two operands.");
      unsigned Count =
          mdconst::extract<ConstantInt>(MD->getOperand(1))->getZExtValue();
      assert(Count >= 1 && "Unroll count must be positive.");
      return Count;
    }
  }

  return 1;
}

// function fullmap gets root instruction (in instToExp) and fills a vector (fill Vector)
// of all users of this root instruction.
static void fullmap(Instruction *instToExp,
                    vector<const Instruction *> *fillVector) {
  for (auto II : instToExp->users()) {
    // Phi is a user that we don't want to save and explore.
    if (auto a = dyn_cast<PHINode>(II))
      continue;
    // Check if this instruction allready intrduce in the pass or not.
    std::vector<const Instruction *>::iterator it = std::find(
        fillVector->begin(), fillVector->end(),dyn_cast<Instruction>(II));
    // in the case that instruction was introduced in the past don't explore him.
    if (it != fillVector->end())
      continue;
    fillVector->push_back(dyn_cast<Instruction>(II));
    fullmap(dyn_cast<Instruction>(II), fillVector);
  }
}

// Fucntion find the location (instruction) of offset and size.
std::pair<IntrinsicInst *, IntrinsicInst *>
LoopData::findOffsetAndSizeIntrinsics() {
    std::pair<IntrinsicInst *, IntrinsicInst *> indexSpaceOffsetSize;
    bool exit = false;
    for (auto FF = p_MD->begin(); FF != p_MD->end(); FF++) {
        for (auto BB = FF->begin(); BB != FF->end(); BB++) {
            for (auto II = BB->begin(); II != BB->end(); II++) {
                if (IntrinsicInst *check = dyn_cast<IntrinsicInst>(II)) {
                    if (check->getIntrinsicID() == Intrinsic::tpc_get_index_space_offset)
                        indexSpaceOffsetSize.first = check;
                    else if (check->getIntrinsicID() ==
                        Intrinsic::tpc_get_index_space_size)
                        indexSpaceOffsetSize.second = check;
                    if (indexSpaceOffsetSize.first && indexSpaceOffsetSize.second)
                        exit = true;
                }
                if (exit)
                    break;
            }
            if (exit)
                break;
        }
        if (exit)
            break;
    }
    return indexSpaceOffsetSize;
}

typedef enum indexOffsetSize { offset, size } indexOffsetSize;

const SCEV *LoopData::relaxSCEV(const SCEV *EV,
                                vector<const Instruction *> index,
                                string name) {
    const SCEV *Left, *Rigth, *temp[10];
    const Loop *LoopVal;
    if (auto value = dyn_cast<SCEVCouldNotCompute>(EV))
        return p_SEL->getConstant(
            ConstantInt::get(Type::getInt32Ty(p_Latch->getContext()), 0));
    if (auto value = dyn_cast<SCEVMulExpr>(EV)) {
        Left = relaxSCEV(value->getOperand(0), index, name);
        Rigth = relaxSCEV(value->getOperand(1), index, name);
        return p_SEL->getMulExpr(Left, Rigth);
    }
    else if (auto value = dyn_cast<SCEVAddExpr>(EV)) {
        int start = 0;
        if (value->getNumOperands() > 2)
            start = 1;
        for (unsigned i = start; i < value->getNumOperands(); i++) {
            temp[i] = relaxSCEV(value->getOperand(i), index, name);
        }
        for (unsigned i = start + 1; i < value->getNumOperands(); i++) {
            temp[start] = p_SEL->getAddExpr(temp[i], temp[start]);
        }
        return temp[start];
    }
    else if (auto value = dyn_cast<SCEVUDivExpr>(EV)) {
        Left = relaxSCEV(value->getLHS(), index, name);
        Rigth = relaxSCEV(value->getRHS(), index, name);
        return p_SEL->getUDivExpr(Left, Rigth);
    }
    else if (auto value = dyn_cast<SCEVCastExpr>(EV)) {
        if (isa<SCEVTruncateExpr>(value))
            return p_SEL->getTruncateExpr(relaxSCEV(value->getOperand(), index, name),
                                        value->getType());
        else if (isa<SCEVZeroExtendExpr>(value))
            return p_SEL->getZeroExtendExpr(
                    relaxSCEV(value->getOperand(), index, name), value->getType());
        else if (isa<SCEVSignExtendExpr>(value))
            return p_SEL->getSignExtendExpr(
                    relaxSCEV(value->getOperand(), index, name), value->getType());
    }
    else if (const SCEVCommutativeExpr *ptr =
        dyn_cast<SCEVCommutativeExpr>(EV)) {
        Left = relaxSCEV(ptr->getOperand(0), index, name);
        Rigth = relaxSCEV(ptr->getOperand(1), index, name);
        if (isa<SCEVSMaxExpr>(ptr))
            return p_SEL->getSMaxExpr(Left, Rigth);
        if (isa<SCEVUMaxExpr>(ptr))
            return p_SEL->getUMaxExpr(Left, Rigth);
    }
    else if (const SCEVAddRecExpr *ptr = dyn_cast<SCEVAddRecExpr>(EV)) {
        Left = relaxSCEV(ptr->getOperand(0), index, name);
        Rigth = relaxSCEV(ptr->getOperand(1), index, name);
        LoopVal = ptr->getLoop();
        SCEV::NoWrapFlags FlagVal = ptr->getNoWrapFlags();
        return p_SEL->getAddRecExpr(Left, Rigth, LoopVal, FlagVal);
    }
    else if (const SCEVUnknown *ptr = dyn_cast<SCEVUnknown>(EV)) {
        Value *valToSearch = ptr->getValue();
        if (std::find(index.begin(), index.end(),
            dyn_cast<Instruction>(valToSearch)) != index.end()) {
            if (valToSearch->getName().find("TPC" + name) == std::string::npos) {
                valToSearch->setName("TPC" + name + "." + to_string(m_DIM));
            }
        }
        else if (IntrinsicInst *val = dyn_cast<IntrinsicInst>(valToSearch)) {
            if (val->getIntrinsicID() == Intrinsic::tpc_ld_l) {
                if (ConstantInt *valConst = dyn_cast<ConstantInt>(val->getOperand(0))) {
                    valToSearch->setName("TPCLoadL" +
                        to_string(valConst->getValue().getZExtValue()));
                }
            }
            else if (val->getIntrinsicID() == Intrinsic::tpc_ld_g) {
                valToSearch->setName("TPCLoadG.0");
            }
            else if (val->getIntrinsicID() == Intrinsic::tpc_gen_addr) {
                valToSearch->setName("TPCGenA.0");
            }
        }
        return p_SEL->getSCEV(valToSearch);
    }
    else if (const SCEVConstant *ptr = dyn_cast<SCEVConstant>(EV)) {
        return ptr;
    }
    return NULL;
}

/// This function is a for the cost model
/// BB - BasicBlock of the loop's latch
void LoopData::findNumberOfIterations(BasicBlock *BB) {
    std::pair<IntrinsicInst *, IntrinsicInst *> indexSpaceOffsetSize =
        findOffsetAndSizeIntrinsics();

    if ((!indexSpaceOffsetSize.first) || (!indexSpaceOffsetSize.second))
        return;

    vector<const Instruction *> indexSpaceSize[2];
    fullmap(indexSpaceOffsetSize.first, &indexSpaceSize[0]);
    fullmap(indexSpaceOffsetSize.second, &indexSpaceSize[1]);

    for (int i = 0; i < 2; i++) {
        string Intrin = i == 0 ? "offset" : "size";
        p_LoopSCEV = relaxSCEV(p_LoopSCEV, indexSpaceSize[i], Intrin);
    }
}

LoopData::LoopData(Loop *L, ScalarEvolution *SE, bool costModel)
    : p_LH(L), p_SEL(SE) {
    m_backendUnroll = getUnrollCountFromMetadata(L->getLoopID());
    p_Prev = L->getParentLoop();
    SmallVector<BasicBlock *, 8> Latches;
    L->getLoopLatches(Latches);
    //TODO: Check what to do when there is more then one latch
    p_Latch = Latches[0];
    p_MD = L->getBlocksVector().at(0)->getParent()->getParent();
    p_Nested = L->getLoopPredecessor();
    SCEVUnionPredicate Pred;
    p_LoopSCEV = p_SEL->getPredicatedBackedgeTakenCount(L, Pred);
    // since this is pollynom we want to multiply with the netural number.
    vector<BasicBlock *> bb = L->getBlocks();
    InductionDescriptor ID;
    const SCEV *scev = NULL;
    for (auto II = bb[0]->begin(), IE = bb[0]->end(); II != IE; ++II) {
        Value *ptr = dyn_cast<Value>(II);
        if (p_SEL->isSCEVable(ptr->getType())) {
            scev = p_SEL->getSCEV(ptr);
            p_SEL->getLoopDisposition(scev, p_LH);
            SCEVParser SCVECP(scev, p_SEL);

            if (Instruction *temp = SCVECP.getValueInducation())
                p_Inducation = temp;
            m_STEP = SCVECP.get_step();
            if (p_Inducation != nullptr)
                break;
        }
    }
    if (p_Inducation) {
        if (auto val = dyn_cast<SCEVCouldNotCompute>(p_LoopSCEV)) {
            p_LoopSCEV = scev;
            m_SCEVNotValid = false;
        }
        if (p_Inducation->getOpcode() == Instruction::ExtractElement) {
            llvm::ConstantInt *CI =
                dyn_cast<llvm::ConstantInt>(p_Inducation->getOperand(1));
            m_DIM = CI->getZExtValue();
            m_Valid = true;
        }
    }
    if (auto val = dyn_cast<SCEVCouldNotCompute>(p_LoopSCEV)) {
        m_SCEVNotValid = false;
        p_LoopSCEV = tryFindSCEV();
    }
    if (costModel) {
      if (p_LoopSCEV->getType()->isPointerTy()) {
        m_SCEVNotValid = false;
        return;
      }
      findNumberOfIterations(p_Latch);
      const SCEV *divFactor = SE->getConstant(ConstantInt::get(
          Type::getIntNTy(p_Latch->getContext(),
                          p_LoopSCEV->getType()->getIntegerBitWidth()),
          m_backendUnroll));
      p_LoopSCEV = SE->getUDivExpr(p_LoopSCEV, divFactor);
    }
}

const SCEV *LoopData::tryFindSCEV() {
    std::vector<const SCEV *> Canidate;
    std::vector<IntrinsicInst *> getLocalList;
    BasicBlock *BB = p_LH->getBlocks()[0];
    for (auto II = BB->begin(), IE = BB->end(); II != IE; ++II) {
        if (p_SEL->isSCEVable(II->getType())) {
            Value *ptr = dyn_cast<Value>(II);
            if (const SCEVAddRecExpr *val =
                dyn_cast<SCEVAddRecExpr>(p_SEL->getSCEV(ptr))) {
                Canidate.push_back(val);
                m_SCEVNotValid = true;
                return val;
            }
        }
    }
    return p_SEL->getConstant(
        ConstantInt::get(Type::getInt32Ty(p_Latch->getContext()), 1));
}
