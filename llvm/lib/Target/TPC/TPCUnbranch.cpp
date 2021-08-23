//===- TPCUnbranch.cpp ------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This pass try to remove condinal on statements with predicate
// if (p) {s1;s2,..}   {s1(p);s2(p)
//===----------------------------------------------------------------------===//
#ifdef LLVM_TPC_COMPILER

#include "TPCTargetMachine.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicsTPC.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Pass.h"
#include "llvm/PassRegistry.h"

#define DEBUG_TYPE "TPCUnbranch"

using namespace llvm;

namespace llvm {
  FunctionPass* createTPCUnbranchPass();
  void initializeTPCUnbranchPass(PassRegistry&);
}

static const char PassDescription[] = "TPC unbranch optimization";
static const char PassName[] = "tpc-unbranch";

static cl::opt<bool> EnableTPCUnbranch(PassName, cl::Hidden, cl::init(true));
static cl::opt<bool> TPCEnableConformity("tpc-unbranch-cnf", cl::Hidden, cl::init(true));

namespace {
  class TPCUnbranch : public FunctionPass {
    Function* F = nullptr;
    unsigned NumTransformed = 0;
  public:
    static char ID;

    StringRef getPassName() const override { return PassDescription; }

    TPCUnbranch() : FunctionPass(ID) {
      initializeTPCUnbranchPass(*PassRegistry::getPassRegistry());
    }
    bool runOnFunction(Function& F) override;
  };//class
}//namespace

char TPCUnbranch::ID = 0;
INITIALIZE_PASS(TPCUnbranch, PassName, PassDescription, false, false)

FunctionPass* llvm::createTPCUnbranchPass() {
  return new TPCUnbranch();
}

#define TOO_MANY_INSTR 256

enum KindPred { FalsePred = 0, TruePred = 1, NoPred = 2 };
enum KindBB { ThenBB, ElseBB, EndBB };
enum KindDecomposingContext { Analysis, Synthesis };
static std::map<BasicBlock*, KindBB> bbk;
enum KindPhi { good, DontMove, StopShow };
SmallSetVector<BasicBlock*, 4> notmovebb;
typedef struct {
  Value *predic;
  Value *polar;
} tPredicPart;
std::map<PHINode *, tPredicPart> PHIConform;
    // returns true if conformed and operands which different
static bool PHIOperandsAreConform(PHINode *phi, Value **PtrOpnd0,
                                  Value **PtrOpnd1,
                                  KindDecomposingContext kdc) {
  if (!TPCEnableConformity)
    return false;
  Instruction *i0 = dyn_cast<Instruction>(phi->getIncomingValue(0));
  Instruction *i1 = dyn_cast<Instruction> (phi->getIncomingValue(1));
  if (i0 == nullptr || i1 == nullptr) {
    return false;
  }
  IntrinsicInst *ii = dyn_cast<IntrinsicInst>(i0);     
  if (!ii)
    return false;
  Intrinsic::ID inid = ii->getIntrinsicID();
  if (inid < Intrinsic::tpc_abs)
    return false;
  int numop = i0->getNumOperands();
  int numop1 = i1->getNumOperands();
  if (numop != numop1) {
    return false;
  }
  int different_operand = 0;
  if (kdc == Synthesis) {
    if (PHIConform.find(phi) != PHIConform.end()) {
      *PtrOpnd0 = i0->getOperand(different_operand);
      *PtrOpnd1 = i1->getOperand(different_operand);
      return true;
    }
    return false;
  }
  int cntdiff = 0;
  for (int i = 0; i < numop; i++) {
    if (i0->getOperand(i) != i1->getOperand(i)) {
      cntdiff++;
      different_operand = i;
    }
  }
  if (cntdiff == 1 && different_operand == 0) {
    // other operands are posible but must be considered individually
    *PtrOpnd0 = i0->getOperand(different_operand);
    *PtrOpnd1 = i1->getOperand(different_operand);
    return true;
  }
  return false;
}

static KindPhi DecomposePHIAccordingBranch(BranchInst *bi, PHINode *phi,
                                           Value **ptv, Value **pfv,
                                           Value **cond,
                                           KindDecomposingContext kdc = Analysis
                                           ) {
  if (phi->getNumIncomingValues() > 2) { // not able now
    return StopShow;
  } 
  Value *ConfOpnd0, *ConfOpnd1;
  bool PhiSimilar;
  PhiSimilar = PHIOperandsAreConform(phi, &ConfOpnd0, &ConfOpnd1, kdc);
  Type* PhiType = phi->getType();
  if (PhiSimilar) {
    if (kdc == Analysis) {
      if (PHIConform.find(phi) == PHIConform.end()) {
        IntrinsicInst* ii = dyn_cast<IntrinsicInst>(phi->getIncomingValue(0));     
        tPredicPart PP = {nullptr, nullptr};
        if (ii) {
          Intrinsic::ID inid= ii->getIntrinsicID();
          if (inid >= Intrinsic::tpc_abs) {
            int no = ii->getNumOperands();
            int num_polar = no - 2;
            int num_predic = num_polar - 1;
            PP.polar = ii->getOperand(num_polar);
            PP.predic = ii->getOperand(num_predic);
          }
        }
        PHIConform.insert(std::make_pair(phi, PP));
      }
    }
    PhiType = ConfOpnd0->getType();
  }
  PointerType *ppt = dyn_cast<PointerType>(PhiType);
  if (ppt) {
    //unsigned pps = ppt->getScalarSizeInBits();
    // for pointer type it is equal 0. May be llvm error.
    return StopShow;
  }
  unsigned PhiSize = PhiType->getScalarSizeInBits();
  if (PhiSize == 64) {
    // it is not supported in instructions
    return StopShow;
  }
  BasicBlock *s0 = bi->getSuccessor(0);
  BasicBlock *s1 = bi->getSuccessor(1);
  BasicBlock *pb = bi->getParent();
  BasicBlock *ib0 = phi->getIncomingBlock(0);
  BasicBlock *ib1 = phi->getIncomingBlock(1);

  if (ib0!=pb && ib0 != s1 && ib0!=s0 &&
      ib1!=pb && ib1 != s1 && ib1!=s0) {
    // absolutely not our phi
    return DontMove;
  }
  if (ib0 == s0 && ib1 != s1 && bi->getParent() != ib1) {
    return DontMove;
  }
  Value *iv0 = phi->getIncomingValue(0);
  Value *iv1 = phi->getIncomingValue(1);
  if (s0 == ib1 && ib0 == bi->getParent()) {
    *cond = bi->getCondition();
    *ptv = iv1;
    *pfv = iv0;
    if (PhiSimilar) {
      *ptv = ConfOpnd1;
      *pfv = ConfOpnd0;
    }
    return good;
  }
  if (ib0 == s1) {//need revert
    if (bbk[s0] == EndBB) {
      if (bi->getParent() == ib1) {
        *ptv = iv1;
        *pfv = iv0;
        *cond = bi->getCondition();
        if (PhiSimilar) {
          *ptv = ConfOpnd1;
          *pfv = ConfOpnd0;
        }
        return good;
      }
    }
  }
  if (ib0 != s0) {
    Value *pomv = iv0;
    BasicBlock *pom = ib0;
    ib0 = ib1;
    iv0 = iv1;
    ib1 = pom;
    iv1 = pomv;
    if (PhiSimilar) {
      pomv = ConfOpnd0;
      ConfOpnd0 = ConfOpnd1;
      ConfOpnd1 = pomv;
    }
  } else {
    if (bbk[s1] == EndBB) {
      if (ib1 == bi->getParent()) {
        *ptv = iv0;
        *pfv = iv1;
        *cond = bi->getCondition();
        if (PhiSimilar) {
          *ptv = ConfOpnd0;
          *pfv = ConfOpnd1;
        }
        return good;
      }
    }
  }
  if (ib0 != s0 || ib1 != s1) { // not our phi
    return StopShow;
  }

  *ptv = iv0;
  *pfv = iv1;
  *cond = bi->getCondition();
  if (PhiSimilar) {
    *ptv = ConfOpnd0;
    *pfv = ConfOpnd1;
  }
  return good;
}

// return true if it was inverted
static bool DecomposeBranchInstWithNorm(BranchInst *bi, Value **cond,
                                        BasicBlock **suc0,
                                        BasicBlock **suc1) {
  if (bi->getNumSuccessors() == 2) {
    BasicBlock *s0 = bi->getSuccessor(0);
    BasicBlock *s1 = bi->getSuccessor(1);
    Value *co = bi->getCondition();
    *suc0 = s0;
    *suc1 = s1;
    *cond = co;
    return false;
  }
  return true;
}

static bool IsMac(Intrinsic::ID inid) {
  switch (inid) {
        case Intrinsic ::tpc_mac:                         // llvm.tpc.mac
        case Intrinsic ::tpc_mac_x2:                      // llvm.tpc.mac.x2
        case Intrinsic ::tpc_mac_x2_f32:                  // llvm.tpc.mac.x2.f32
        case Intrinsic ::tpc_mac_x2_zp:                   // llvm.tpc.mac.x2.zp
        case Intrinsic ::tpc_mac_zp:                      // llvm.tpc.mac.zp
          return true;

  }
  return false;
}

static bool IsSt(Intrinsic::ID inid) {
  switch (inid) {
  case Intrinsic ::tpc_st_g:                // llvm.tpc.st.g
  case Intrinsic ::tpc_st_g_inc:            // llvm.tpc.st.g.inc
  case Intrinsic ::tpc_st_l:                // llvm.tpc.st.l
  case Intrinsic ::tpc_st_l_v:              // llvm.tpc.st.l.v
  case Intrinsic ::tpc_st_l_v_high:         // llvm.tpc.st.l.v.high
  case Intrinsic ::tpc_st_l_v_high_ofs:     // llvm.tpc.st.l.v.high.ofs
  case Intrinsic ::tpc_st_l_v_low:          // llvm.tpc.st.l.v.low
  case Intrinsic ::tpc_st_l_v_low_ofs:      // llvm.tpc.st.l.v.low.ofs
  case Intrinsic ::tpc_st_l_v_ofs:          // llvm.tpc.st.l.v.ofs
  case Intrinsic ::tpc_st_tnsr:             // llvm.tpc.st.tnsr
  case Intrinsic ::tpc_st_tnsr_direct:      // llvm.tpc.st.tnsr.direct
  case Intrinsic ::tpc_st_tnsr_high:        // llvm.tpc.st.tnsr.high
  case Intrinsic ::tpc_st_tnsr_high_direct: // llvm.tpc.st.tnsr.high.direct
  case Intrinsic ::tpc_st_tnsr_high_rmw:    // llvm.tpc.st.tnsr.high.rmw
  case Intrinsic ::
      tpc_st_tnsr_high_rmw_direct:         // llvm.tpc.st.tnsr.high.rmw.direct
  case Intrinsic ::tpc_st_tnsr_low:        // llvm.tpc.st.tnsr.low
  case Intrinsic ::tpc_st_tnsr_low_direct: // llvm.tpc.st.tnsr.low.direct
  case Intrinsic ::tpc_st_tnsr_low_rmw:    // llvm.tpc.st.tnsr.low.rmw
  case Intrinsic ::
      tpc_st_tnsr_low_rmw_direct:       // llvm.tpc.st.tnsr.low.rmw.direct
  case Intrinsic ::tpc_st_tnsr_partial: // llvm.tpc.st.tnsr.partial
  case Intrinsic ::
      tpc_st_tnsr_partial_direct:           // llvm.tpc.st.tnsr.partial.direct
  case Intrinsic ::tpc_st_tnsr_partial_rmw: // llvm.tpc.st.tnsr.partial.rmw
  case Intrinsic ::
      tpc_st_tnsr_partial_rmw_direct: // llvm.tpc.st.tnsr.partial.rmw.direct
  case Intrinsic ::tpc_st_tnsr_rmw:   // llvm.tpc.st.tnsr.rmw
  case Intrinsic ::tpc_st_tnsr_rmw_direct: // llvm.tpc.st.tnsr.rmw.direct
  case Intrinsic ::tpc_st_tnsr_s:          // llvm.tpc.st.tnsr.s
  case Intrinsic ::tpc_st_tnsr_s_hwr:      // llvm.tpc.st.tnsr.s.hwr
  case Intrinsic ::tpc_st_tnsr_s_hwr_rmw:  // llvm.tpc.st.tnsr.s.hwr.rmw
  case Intrinsic ::tpc_st_tnsr_s_rmw:      // llvm.tpc.st.tnsr.s.rmw
  case Intrinsic ::tpc_st_tnsr_sqz:        // llvm.tpc.st.tnsr.sqz
  case Intrinsic ::tpc_st_tnsr_sqz_rmw:    // llvm.tpc.st.tnsr.sqz.rmw
   return true;
  }
  return false;
}


static bool HasIncome(Intrinsic::ID inid) {
  return !(IsSt(inid) || IsMac(inid));
 
}

// check instructions are good to move BB
static bool BBBad(BasicBlock *bb_accept, BasicBlock *rbb) {
  int instr_count = 0;
  BranchInst *bia = dyn_cast<BranchInst>(bb_accept->getTerminator());

  for (auto It = rbb->begin(), E = rbb->end(); It != E;) {
    Instruction &I = *It;
    ++It;
    if (PHINode *phi = dyn_cast<PHINode>(&I)) {
      Value *tv, *fv, *cond;
      if (DecomposePHIAccordingBranch(bia, phi, &tv, &fv, &cond) == StopShow)
        return true;
    }
  }

  if (bbk[rbb] == EndBB) {
    return false;
  }
  for (auto It = rbb->begin(), E = rbb->end(); It != E;) {
    Instruction &I = *It;
    ++It;
    instr_count++;
    if (const IntrinsicInst *intrins = dyn_cast<IntrinsicInst>(&I)) {
      Intrinsic::ID inid = intrins->getIntrinsicID();
      if (inid < Intrinsic::tpc_abs) {
        return true;
      }
      int no = intrins->getNumOperands();
      int num_polar = no - 2;
      int num_predic = num_polar - 1;
      int num_income = num_predic - 1;
      Value *polar = intrins->getOperand(num_polar);
      Value *predic = intrins->getOperand(num_predic);
      Type *predic_type = predic->getType();
      if (predic_type->isVectorTy()) {
        return true;
      }
      ConstantInt *CPre = dyn_cast<ConstantInt>(predic);
      ConstantInt *CPol = cast<ConstantInt>(polar);
      unsigned vpola = CPol->getZExtValue();
      if (HasIncome(inid)) {
        Value *income = intrins->getOperand(num_income);
        Type *ti = income->getType();
        if ((!(CPre && CPre->getZExtValue() == 1) || vpola == 1 ||
             ti->isVectorTy()) &&
            !isa<UndefValue>(income)) {
          // let's try
          // return true;
        }
      }
    } else if (dyn_cast<StoreInst>(&I)) {
      // not able now how
      return true;
    } else if (dyn_cast<CallInst>(&I)) {
      return true;
    } else if (bbk[rbb] != EndBB && dyn_cast<ReturnInst>(&I)) {
      return true;
    } else if (dyn_cast<SwitchInst>(&I)) {
      return true;
    } else { // start with expr which used only inside this BB
      BranchInst *bi = dyn_cast<BranchInst>(&I);
      // need to check there is no phi
      if (bi) {
        int nuo = bi->getNumSuccessors();
        if (nuo > 1) {
          // conditions must be merged
          return true;
        }
      }
      for (auto Usr : I.users()) {
        Instruction *instr = dyn_cast<Instruction>(Usr);
        BasicBlock *users_bb = instr->getParent();
        if (users_bb != rbb) {
          if (PHINode *phi = dyn_cast<PHINode>(instr)) {
            Value *tv, *fv, *cond;
            if (DecomposePHIAccordingBranch(bia, phi, &tv, &fv, &cond) ==
                StopShow)
              return true;
            else
              continue;
          }
          return true;
        }
      }
    }
  }
  return instr_count > TOO_MANY_INSTR;
}

static void NormalizeIntrinPredic(IntrinsicInst *intrins, bool invert,
                                    Instruction *BuildInstr,
                                    LLVMContext &Ctx) {
  // do intrinsic with normal(0) polarity if invert - 0 and in turn
  // do polarity as invert
  int no = intrins->getNumOperands();
  int num_polar = no - 2;
  int num_predic = num_polar - 1;
  Value *polar = intrins->getOperand(num_polar);
  Value *predic = intrins->getOperand(num_predic);
  // Start with trivial cases
  Constant *cv = dyn_cast<Constant>(polar);
  unsigned polarity = cv->isZeroValue() ? 0 : 1;
  if (polarity == invert) {
    return;
  } else {
    unsigned neg_polar = ~polarity & 1;
    IRBuilder<> Builder(BuildInstr);
 
    intrins->setOperand(num_polar,
                        ConstantInt::get(Type::getInt1Ty(Ctx), neg_polar));
    Value *NewPred = Builder.CreateNot(predic);
    intrins->setOperand(num_predic, NewPred);
  }
}

static Value* ReplacePHI(PHINode* phino, Instruction* build_instr,
                          Instruction *bi,Function &Func) {
  IRBuilder<> Builder(build_instr);
  LLVMContext &Ctx = Func.getContext();
  Value *truval, *falsval;
  Value *cond;
  KindPhi kphi = DecomposePHIAccordingBranch(cast<BranchInst>(bi), phino, &truval,
                                 &falsval, &cond, Synthesis);
  if (kphi == StopShow)
    llvm_unreachable("decompose_phi failed");
  // let'c create mov instr
  Module *Mod = Func.getParent();
  Type *tbool = IntegerType::get(Ctx, 1);
  Type *Int32Ty = Type::getInt32Ty(Ctx);
  VectorType *Int5Ty = VectorType::get(Int32Ty, 5);
  Type *tt = truval->getType();
  Value *ExtF;
  Value *NewIns;
  if (tt == Int5Ty) {
    ExtF = Intrinsic::getDeclaration(Mod, Intrinsic::tpc_mov_mask, tt);
    NewIns =
        Builder.CreateCall(ExtF, {truval, ConstantInt::get(Int32Ty, 31), // mask
                                  ConstantInt::get(Int32Ty, 0), // switch
                                  falsval, cond, ConstantInt::get(tbool, 0)});
  } else {
    // here can be types, which is not supported in arch, so better use
    // select
    NewIns = Builder.CreateSelect(cond, truval, falsval);
  }

  if (PHIConform.find(phino) != PHIConform.end()) { // need to repeat by with new argument
    tPredicPart PP = PHIConform[phino];
    Instruction*orig = cast<Instruction>(phino->getIncomingValue(0));
    Instruction *cloni = orig->clone();
    if (const IntrinsicInst *intrins = dyn_cast<IntrinsicInst>(orig)) {
      assert(intrins->getIntrinsicID() >= Intrinsic::tpc_abs);
      int no = intrins->getNumOperands();
      int num_polar = no - 2;
      int num_predic = num_polar - 1;
      cloni->setOperand(num_polar, PP.polar);
      cloni->setOperand(num_predic, PP.predic);
    }
    cloni->setOperand(0, NewIns);
    cloni->insertBefore(build_instr);
    NewIns = cloni;
  }
  return NewIns;
}

//move instruction from bb to it's predecessor. Predicate is taken into account
static void MoveInstructionsUp(BasicBlock *bb_donor, BasicBlock *bb_accept,
                                 int kind_pred,
                                 Function &Func) {
  LLVMContext &Ctx = Func.getContext();
  Instruction *lasti = bb_accept->getTerminator();
  assert(lasti);
  IRBuilder<> Builder(lasti);
  for (auto It = bb_donor->begin(), E = bb_donor->end(); It != E;) {
    Instruction &I = *It;
    ++It;
    if (IntrinsicInst *intrins = dyn_cast<IntrinsicInst>(&I)) {
      if (kind_pred == NoPred) {
        I.moveBefore(lasti);
      } else {
        NormalizeIntrinPredic(intrins, kind_pred == FalsePred, lasti, Ctx);
        Value *NewPred;
        BranchInst *bi = dyn_cast<BranchInst>(lasti);
        assert(bi && bi->isConditional());
        Value *cond = bi->getCondition();
        int no = intrins->getNumOperands();
        int num_polar = no - 2;
        int num_predic = num_polar - 1;
        int num_income = num_predic - 1;
        Value *polar = intrins->getOperand(num_polar);
        Value *predic = intrins->getOperand(num_predic);
        Type *predic_type = predic->getType();
        if (predic_type->isVectorTy()) {
          llvm_unreachable("not yet");
        }

        ConstantInt *CPre = dyn_cast<ConstantInt>(predic);
        ConstantInt *CPol = cast<ConstantInt>(polar);
        unsigned vpola = CPol->getZExtValue();
        Intrinsic::ID inid = intrins->getIntrinsicID();
        bool via_select = false;
        Value *income = intrins->getOperand(num_income);       
        if (IsMac(inid)) {
          if (CPre == nullptr) {
            via_select = true;
          }
        }
        else if (HasIncome(inid)) {
          Type *ti = income->getType();
          Value *vu = UndefValue::get(ti);
          if (CPre && CPre->getZExtValue() == 1 && !isa<UndefValue>(income) &&
              !ti->isVectorTy()) { // transform to undef
            intrins->setOperand(num_income, vu);
            income = intrins->getOperand(num_income);
          }
          if ((!(CPre && CPre->getZExtValue() == 1) || vpola == 1 ||
               ti->isVectorTy()) &&
              !isa<UndefValue>(income)) {
            // let's try
            via_select = true;
          }
        }

        if (via_select) {
          Type *ity = intrins->getType();
          Value *vu = UndefValue::get(ity);
          SelectInst *SelIns;
          intrins->moveBefore(lasti);
          SelIns = cast <SelectInst>(Builder.CreateSelect(cond, vu, vu));
          intrins->replaceAllUsesWith(SelIns);
          if (kind_pred == TruePred) {
            SelIns->setTrueValue(intrins);
          } else {
            SelIns->setFalseValue(intrins);
          }         
        } else {
          if (kind_pred == TruePred) {
            NewPred=(CPre && CPre->getZExtValue() == 1)
                ? cond
                : Builder.CreateAnd(predic, cond);
          } else {
            NewPred = (CPre && CPre->getZExtValue() == 0)
                           ? cond
                           : Builder.CreateOr(predic, cond);
          }
          intrins->removeFromParent();
          intrins->setOperand(num_predic, NewPred);
          intrins->insertBefore(lasti);
        }
      }
    } else if (PHINode *phino = dyn_cast<PHINode>(&I)) {
      assert(kind_pred == NoPred);     
      Value *NewIns = ReplacePHI(phino, lasti, lasti, Func);
      phino->replaceAllUsesWith(NewIns);
      phino->eraseFromParent();         
    } else {
      I.moveBefore(lasti);
    }
  }
}

static bool CmpBranches(BranchInst *bi1,BranchInst *bi2) {
  unsigned int ns1 = bi1->getNumSuccessors();
  if (ns1 == bi2->getNumSuccessors()) {
    for (unsigned i = 0; i < ns1; i++) {
      if (bi1->getSuccessor(i) != bi2->getSuccessor(i))
        return false;
    }
  } else
    return false;
  return true;
}

static bool CloseBB(BasicBlock* bthen, BasicBlock* belse) {
  return bthen->getNextNode() == belse || belse->getNextNode() == bthen;
}

// return true if met bad phi, which cannt be processed
static KindPhi CheckOutEdges(BranchInst *bia, BasicBlock *babl) {
  SmallSetVector<PHINode *, 16> phiset;

  for (auto *bb : successors(babl)) {
    for (auto It = bb->begin(), E = bb->end(); It != E;) {
      Instruction &I = *It;
      ++It;
      if (PHINode *phino = dyn_cast<PHINode>(&I)) {
        phiset.insert(phino);
      }
    }
  }

  // phi must be good
  bool was_dont_move = false;
  for (PHINode *phii : phiset) {
    Value *tv, *fv, *cond;
    KindPhi kphi = DecomposePHIAccordingBranch(bia, phii, &tv, &fv, &cond);
    if (kphi==StopShow)
      return StopShow;
    if (kphi == DontMove) {
      was_dont_move = true;
      notmovebb.insert(phii->getParent());
    }  
    continue;
  }
    
   return was_dont_move ? DontMove : good;
}

bool TPCUnbranch::runOnFunction(Function &Func) {

  if (!EnableTPCUnbranch) {
    return false;
  }

  if (skipFunction(Func))
    return false;
  F = &Func;
  NumTransformed = 0;
  SmallSetVector<BranchInst *, 16> iflist;
  for (auto BBIt = Func.begin(), BBEnd = Func.end(); BBIt != BBEnd;) {
    BasicBlock *BB = &*BBIt;
    ++BBIt;
    if (dyn_cast<BranchInst>(BB->getFirstNonPHI())) {
      continue;
    }
    BasicBlock *predeb = BB->getSinglePredecessor();
    if (predeb) {
      if (dyn_cast<BranchInst>(predeb->getFirstNonPHI())) {
        continue;
      }
      Instruction *lasti = predeb->getTerminator();
      if (lasti) {
        BranchInst *bi = dyn_cast<BranchInst>(lasti);
        if (bi && bi->isConditional()) {
          BasicBlock *theno = bi->getSuccessor(0);
          BasicBlock *elseo = bi->getSuccessor(1);
          if (dyn_cast<BranchInst>(theno->getFirstNonPHI())) {
            continue;
          }
          if (dyn_cast<BranchInst>(elseo->getFirstNonPHI())) {
            continue;
          }
          bool looping = false;
          for (auto *bb : successors(theno)) {
            if (bb == predeb) {
              looping = true;
              break;
            }
          }
          if (looping)
            continue;
          for (auto *bb : successors(elseo)) {
            if (bb == predeb) {
              looping = true;
              break;
            }
          }
          if (looping)
            continue;
          int nuo = bi->getNumSuccessors();
          if (theno != predeb && elseo != predeb && 
            CloseBB(theno,elseo) && (theno == BB || elseo == BB) && nuo == 2) {
            iflist.insert(bi);
          }
        }
      }
    }
  }

  for (auto *bi : make_range(iflist.rbegin(), iflist.rend())) {
    BasicBlock *accept = bi->getParent();
    BasicBlock *suc_true, *suc_false;
    Value *condition;
    bbk.clear();
    notmovebb.clear();
    PHIConform.clear();
    //bi->dump();
    if (DecomposeBranchInstWithNorm(bi, &condition, &suc_true,
                                        &suc_false)) {
      continue;
    }
    BranchInst *term_true = dyn_cast<BranchInst>(suc_true->getTerminator());
    BranchInst *term_false = dyn_cast<BranchInst> (suc_false->getTerminator());
    if (!term_true) {
      if (!term_false)
        continue;
      if (term_false->getSuccessor(0) != suc_true) {
        // it is not just else/then revert, mangled cfg
        continue;
      }
      bbk[suc_true] = EndBB;
      bbk[suc_false] = ElseBB;
    } else if(accept->getNextNode() == suc_true){
      bbk[suc_true] = ThenBB;
      bbk[suc_false] = EndBB;
      if (term_true && term_false && CmpBranches(term_true, term_false)) {
        bbk[suc_false] = ElseBB;
      }
    } else if (accept->getNextNode() == suc_false) {
      bbk[suc_true] = EndBB;
      bbk[suc_false] = ElseBB;
      if (term_true && term_false && CmpBranches(term_true, term_false)) {
        bbk[suc_true] = ThenBB;
      }
    } else {
      continue;
    }
    BasicBlock *predecessor_true;
    if (bbk[suc_true] == ThenBB) {
      predecessor_true = suc_true->getSinglePredecessor();
      if (!predecessor_true)
        continue;
    }
    BasicBlock *predecessor_false;

    if (bbk[suc_false]==ElseBB) {
      predecessor_false = suc_false->getSinglePredecessor();
      if (!predecessor_false)
        continue;
    }

    if (bbk[suc_false] == EndBB) {
      if (suc_true->getSingleSuccessor() != suc_false)
        continue;
    }
    
    if (BBBad(accept, suc_true))
      continue;
    if (BBBad(accept, suc_false))
      continue;
    BasicBlock *bb_end;   
    if (bbk[suc_true] == ThenBB && bbk[suc_false] == ElseBB) {
      bb_end = term_false->getSuccessor(0);
      bbk[bb_end] = EndBB;
      if (suc_true->getSingleSuccessor() != bb_end ||
          suc_false->getSingleSuccessor() != bb_end) {
        continue;
      }
      if (CheckOutEdges(bi,suc_true) == StopShow) {
        continue;
      }
      if (CheckOutEdges(bi, suc_false) == StopShow) {
        continue;
      }
    } else if (bbk[suc_true] == ThenBB && bbk[suc_false] == EndBB) {
      bb_end = suc_false;
      if (suc_true->getSingleSuccessor() != bb_end) {
        continue;
      }
    } else if (bbk[suc_true] == EndBB && bbk[suc_false] == ElseBB) {
      bb_end = suc_true;
      if (suc_false->getSingleSuccessor() != bb_end) {
        continue;
      }
    } else {
      llvm_unreachable("bad combination");
    }
 
    if (suc_false != bb_end && BBBad(accept, bb_end)) {
      continue;
    }
    if (CheckOutEdges(bi,bb_end) == StopShow) {
      continue;
    }

    if (bbk[suc_true]==ThenBB && bbk[suc_false]==ElseBB) {
      MoveInstructionsUp(suc_true, accept, TruePred, Func);
      suc_true->dropAllReferences();
      suc_true->removeFromParent();
      MoveInstructionsUp(suc_false, accept, FalsePred, Func);
      suc_false->dropAllReferences();
      suc_false->removeFromParent();
      assert(bb_end == term_false->getSuccessor(0));
      } else if (bbk[suc_true] == ThenBB && bbk[suc_false] == EndBB) {
        MoveInstructionsUp(suc_true, accept, TruePred, Func);
        suc_true->dropAllReferences();
        suc_true->removeFromParent();
        assert(bb_end == suc_false);
      } else if (bbk[suc_true] == EndBB && bbk[suc_false] == ElseBB) {
        MoveInstructionsUp(suc_false, accept, FalsePred, Func);
        suc_false->dropAllReferences();
        suc_false->removeFromParent();
        assert(bb_end == suc_true);
      } else {
        llvm_unreachable("bad combination");
      }
        // may end block dragged inside?
    BasicBlock *predecessor_end = nullptr;
    // must be single after dragging then/else
    // and all preds must be accept
    for (auto *bb : predecessors(bb_end)) {
      if (bb != accept) {
        predecessor_end = nullptr;
        break;
      } else {
        predecessor_end = accept;
      }
    }
    bool mov_end = notmovebb.empty();
    
    if (predecessor_end == accept) {
      // need to check no ref beside branches (phi) in ref BB
      if (!mov_end) { 
        // can not move,but suc_true already moved
        // so we must transform phi with such block
        for (auto It = bb_end->begin(), E = bb_end->end(); It != E;) {
          Instruction &I = *It;
          ++It;
          if (PHINode *phino = dyn_cast<PHINode>(&I)) {
            Value *NewIns = ReplacePHI(phino, phino, bi, Func);
            phino->replaceAllUsesWith(NewIns);
            phino->eraseFromParent();
          }
        }
      }
      else { // tail can be moved
        MoveInstructionsUp(bb_end, accept, NoPred, Func);
        bb_end->dropAllReferences();
        bb_end->removeFromParent();
      }
    }
    bi->replaceAllUsesWith(UndefValue::get(bi->getType()));
    bi->eraseFromParent();
    NumTransformed++;
  }

 //Func.dump();
  return NumTransformed > 0;
}
#endif // LLVM_TPC_COMPILER
