#include "TPCLoopData.h"

#define GET_INSTRINFO_ENUM
#include "TPCGenInstrInfo.inc"

#include "MCTargetDesc/TPCMCInstrInfo.h"
#include "TPCSubtarget.h"

#include "llvm/ADT/Optional.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/IR/IntrinsicsTPC.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

static bool isLoop(const MachineInstr &MI) {
  return TPCII::isLoopInst(MI.getDesc()) && MI.getOpcode() != TPC::LOOPEND;
}

static Optional<unsigned> findOptionMDIndex(const MDNode &LoopMD,
                                            const StringRef Name) {
  for (unsigned I = 1, E = LoopMD.getNumOperands(); I < E; ++I) {
    MDNode *MD = dyn_cast<MDNode>(LoopMD.getOperand(I));
    if (!MD)
      continue;

    MDString *S = dyn_cast<MDString>(MD->getOperand(0));
    if (!S)
      continue;

    if (Name.equals(S->getString()))
      return I;
  }
  return None;
}

static MachineOperand *getMachineLoopMDOperand(MachineInstr &LoopEndMI) {
  assert(LoopEndMI.getOpcode() == TPC::LOOPEND);
  for (MachineOperand &MO : LoopEndMI.operands()) {
    if (MO.isMetadata())
      return &MO;
  }
  return nullptr;
}

namespace llvm {

bool hasLoopTakenMD(const MDNode &LoopMD) {
  const Optional<unsigned> OpIx = findOptionMDIndex(LoopMD, "llvm.loop.taken");

  if (!OpIx.hasValue())
    return false;

  MDNode &MD = *dyn_cast<MDNode>(LoopMD.getOperand(OpIx.getValue()));
  assert(MD.getNumOperands() == 2); // requirement on llvm.loop.taken metadata
  return !mdconst::extract<ConstantInt>(MD.getOperand(1))->isZero();
}

void removeLoopTakenMD(MachineInstr &LoopEndMI) {
  MachineOperand *MO = getMachineLoopMDOperand(LoopEndMI);
  if (!MO)
    return;

  const MDNode *LoopMD = MO->getMetadata();
  if (!LoopMD)
    return;

  const Optional<unsigned> LoopTakenMDIndex =
      findOptionMDIndex(*LoopMD, "llvm.loop.taken");
  if (!LoopTakenMDIndex.hasValue())
    return;

  SmallVector<Metadata *, 8> MDs;
  for (const auto& X : enumerate(LoopMD->operands())) {
    MDNode *MD = dyn_cast<MDNode>(X.value());
    if (MD && X.index() != LoopTakenMDIndex.getValue())
      MDs.push_back(MD);
  }
  if (!MDs.empty())
    MO->setMetadata(MDNode::get(LoopMD->getContext(), MDs));
  else
    MO->setMetadata(nullptr);
}

const MDNode *getMachineLoopMDNode(MachineInstr &LoopEndMI) {
  MachineOperand *MO = getMachineLoopMDOperand(LoopEndMI);
  return MO ? MO->getMetadata() : nullptr;
}

const MDNode *findOptionMDForLoopMD(const MDNode &LoopMD,
                                    const StringRef Name) {
  const Optional<unsigned> Ix = findOptionMDIndex(LoopMD, Name);
  return Ix.hasValue() ? dyn_cast<MDNode>(LoopMD.getOperand(Ix.getValue()))
                       : nullptr;
}

Optional<unsigned> getUnrollCountFromMetadata(const MDNode *LoopMD) {
  if (!LoopMD)
    return None;

  const MDNode *MD =
      findOptionMDForLoopMD(*LoopMD, "llvm.loop.machine.unroll.count");
  if (!MD)
    return None;

  assert(MD->getNumOperands() == 2 &&
         "Unroll hint metadata should have two operands.");
  unsigned Count =
      mdconst::extract<ConstantInt>(MD->getOperand(1))->getZExtValue();
  assert(Count >= 1 && "Unroll count must be positive.");
  return Count;
}

Register getCounterRegister(const MachineLoop& L) {
  const TPCSubtarget &ST =
      L.getHeader()->getParent()->getSubtarget<TPCSubtarget>();
  const unsigned HWLoopStartReg = ST.getHWLoopStartReg().id();
  const unsigned HWLoopFinalReg = ST.getHWLoopFinalReg().id();
  assert(HWLoopStartReg <= HWLoopFinalReg);

  unsigned Counter = HWLoopStartReg;
  for (MachineLoop *Parent = L.getParentLoop();
       Parent && Counter < HWLoopFinalReg; Parent = Parent->getParentLoop()) {
    MachineBasicBlock *const ParentLatch = Parent->getLoopLatch();
    MachineBasicBlock *const ParentHeader = Parent->getHeader();
    if (!ParentLatch || !ParentHeader)
      continue;

    const auto PreHeaderIt = llvm::find_if(
        ParentHeader->predecessors(),
        [ParentLatch](MachineBasicBlock *BB) { return BB != ParentLatch; });
    if (PreHeaderIt == ParentHeader->predecessors().end())
      continue;

    if (any_of((*PreHeaderIt)->instrs(), isLoop))
      Counter++;
  }
  return Register(Counter);
}

Optional<HWLoopCounterInfo> getLoopCounterInfo(const MachineLoop &ML) {
  MachineBasicBlock *const PredMBB = ML.getLoopPredecessor();
  if (!PredMBB || PredMBB->empty())
    return None;

  MachineInstr &LoopInst = PredMBB->back();
  if (!isLoop(LoopInst))
    return None;

  HWLoopCounterInfo Info;
  Info.LoopInstr = &LoopInst;
  Info.CounterReg = getCounterRegister(ML);
  return Info;
}

} // namespace llvm

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
    m_backendUnroll = getUnrollCountFromMetadata(L->getLoopID()).getValueOr(1);
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
