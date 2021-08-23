//===- SCEVParser.cpp ----------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//
#include "SCEVParser.h"

void SCEVParser::searchForValue(const SCEV *s, vector<Instruction *> &InsVecUpdate) {
    if (auto ptr = dyn_cast<SCEVNAryExpr>(s)) {
        for (unsigned i = 0; i < ptr->getNumOperands(); i++) {
            searchForValue(ptr->getOperand(i), InsVecUpdate);
        }
    } else if (auto ptr = dyn_cast<SCEVUnknown>(s)) {
        if (Value *stepValue = m_SCE.visit(ptr))
            if (Instruction *ins = dyn_cast<Instruction>(stepValue))
                InsVecUpdate.push_back(ins);
    }
}

const SCEV *SCEVParser::findCoefficient(const SCEV *Expr,
                                        const Loop *TargetLoop) {
    const SCEVAddRecExpr *AddRec = dyn_cast<SCEVAddRecExpr>(Expr);
    if (!AddRec)
        return p_SEL->getZero(Expr->getType());
    if (AddRec->getLoop() == TargetLoop)
        return AddRec->getStepRecurrence(*p_SEL);
    return findCoefficient(AddRec->getStart(), TargetLoop);
}

void SCEVParser::computeStride() {
    if (const SCEVAddRecExpr *AddRec = dyn_cast<SCEVAddRecExpr>(p_SCEVP)) {
        // Compute the step inducation
        const SCEV *StepRec = AddRec->getStepRecurrence(*p_SEL);
        if (StepRec->getSCEVType() == SCEVTypes::scConstant && AddRec->isAffine()) {
            m_StepInducation = p_SEL->getUnsignedRangeMax(StepRec).getZExtValue();
            const SCEV *scev1 = AddRec->getStart();
            searchForValue(scev1, m_InsVec);
        }
    } else {
        m_StepInducation = 0;
    }
}

void SCEVParser::computerStepInstruction(const SCEV *scev) {
    if (const SCEVAddRecExpr *AddRec = dyn_cast<SCEVAddRecExpr>(scev)) {
        const SCEV *ptr = AddRec->getStepRecurrence(*p_SEL);
        if (const SCEVSignExtendExpr *ptr2 = dyn_cast<SCEVSignExtendExpr>(ptr))
            ptr = ptr2->getOperand();
        m_stepValue.push_back(std::make_pair(AddRec->getLoop(), m_SCE.visit(ptr)));
        computerStepInstruction(AddRec->getStart());
    }
}

const SCEV *SCEVParser::treeRunner(const SCEV *s) {
    const SCEV *LH;
    const SCEV *RH;
    const SCEV *Result = NULL;
    if (const SCEVAddExpr *ptr = dyn_cast<SCEVAddExpr>(s)) {
        LH = treeRunner(ptr->getOperand(0));
        RH = treeRunner(ptr->getOperand(1));
        return p_SEL->getAddExpr(LH, RH);
    }
    if (const SCEVMulExpr *ptr = dyn_cast<SCEVMulExpr>(s)) {
        LH = treeRunner(ptr->getOperand(0));
        RH = treeRunner(ptr->getOperand(1));
        return p_SEL->getMulExpr(LH, RH);
    }
    if (const SCEVUDivExpr *ptr = dyn_cast<SCEVUDivExpr>(s)) {
        LH = treeRunner(ptr->getLHS());
        RH = treeRunner(ptr->getRHS());
        return p_SEL->getUDivExpr(LH, RH);
    }
    if (const SCEVZeroExtendExpr *ptr = dyn_cast<SCEVZeroExtendExpr>(s)) {
        return p_SEL->getZeroExtendExpr(treeRunner(ptr->getOperand()), ptr->getType());
    }
    if (const SCEVTruncateExpr *ptr = dyn_cast<SCEVTruncateExpr>(s)) {
        return p_SEL->getTruncateExpr(treeRunner(ptr->getOperand()), ptr->getType());
    }
    if (const SCEVCommutativeExpr *ptr = dyn_cast<SCEVCommutativeExpr>(s)) {
        LH = treeRunner(ptr->getOperand(0));
        RH = treeRunner(ptr->getOperand(1));
        if (isa<SCEVSMaxExpr>(ptr))
            return p_SEL->getSMaxExpr(LH, RH);
        if (isa<SCEVUMaxExpr>(ptr))
            return p_SEL->getUMaxExpr(LH, RH);
    }
    if (const SCEVConstant *ptr = dyn_cast<SCEVConstant>(s))
        return ptr;
    if (const SCEVSignExtendExpr *ptr = dyn_cast<SCEVSignExtendExpr>(s)) {
        // todo support ungisng support
        dbgs() << "Sign/ungsined doesn't supportd by the anylsis\n";
        return ptr;
    }
    if (const SCEVUnknown *ptr = dyn_cast<SCEVUnknown>(s)) {
        if (Instruction *ins = dyn_cast<Instruction>(m_SCE.visit(ptr))) {

            if (m_InsVec2.size() > 1 && ins == m_InsVec2[m_InsVec2.size() - 1])
                return p_SEL->getZero(Type::getInt32Ty(ins->getContext()));
            else
                return ptr;
        } else
            return p_SEL->getZero(s->getType());
    }
    if (!Result)
        errs() << "empty";
    return Result;
}

const SCEV *SCEVParser::computeInit(const SCEV *scev) {
    while (const SCEVAddRecExpr *temp = dyn_cast<SCEVAddRecExpr>(scev))
        scev = temp->getStart();
    return treeRunner(scev);
}

template<typename T, unsigned N>
static SmallVector<T *, N> get_list_of(BasicBlock *const *BB) {
    // Retrun a vector of all element correspond to type <T>
    // indise the basicblock <BB>.
    SmallVector<T *, N> list;
    for (BasicBlock::iterator II = (*BB)->begin(); II != (*BB)->end(); II++)
        if (auto Intrin = dyn_cast<T>(II))
            list.push_back(Intrin);
    return list;
}

const SCEV *SCEVParser::computeInitIter(const SCEV *scev, status range) {
    if (const SCEVAddRecExpr *AddRec = dyn_cast<SCEVAddRecExpr>(scev)) {
        if (const SCEVAddRecExpr *ptr1 =
                dyn_cast<SCEVAddRecExpr>(AddRec->getOperand(0))) {
            const SCEV *induction =
                    p_SEL->getSCEV(AddRec->getLoop()->getCanonicalInductionVariable());
            if (range == status::min) {
                induction = p_SEL->getConstant(p_SEL->getSignedRangeMin(induction));
            }
            const SCEV *Var = AddRec->getOperand(1);
            return p_SEL->getAddExpr(p_SEL->getMulExpr(induction, Var),
                                     computeInitIter(ptr1, range));
        }
        if (const SCEVConstant *conValue =
                dyn_cast<SCEVConstant>(AddRec->getOperand(1))) {
            if (range == status::min)
                return p_SEL->getZero(scev->getType());
            return p_SEL->getMinusSCEV(conValue, p_SEL->getConstant(scev->getType(), 1));
        }
        if (const SCEVUnknown *unknown =
                dyn_cast<SCEVUnknown>(AddRec->getOperand(1))) {
            if (range == status::min)
                return p_SEL->getZero(scev->getType());
            return p_SEL->getMinusSCEV(unknown, p_SEL->getConstant(scev->getType(), 1));
        }
    }
    return p_SEL->getZero(scev->getType());
}

std::string SCEVParser::printH(const SCEV *scev) {
    string result;
    if (const SCEVAddRecExpr *AddRec = dyn_cast<SCEVAddRecExpr>(scev)) {
        if (!AddRec->getLoop()->getCanonicalInductionVariable()) {
            result = "NOINDUCATION";
        } else {
            result = string(
                    AddRec->getLoop()->getCanonicalInductionVariable()->getName());
        }
        result =
                "(" + result +
                string("*") + printH(AddRec->getOperand(1)) + string("+") +
                printH(AddRec->getOperand(0)) + ")";
    }

    if (const SCEVAddExpr *AddRec = dyn_cast<SCEVAddExpr>(scev)) {
        string resultStr[10];
        unsigned i = 0;
        for (i = 0; i < AddRec->getNumOperands(); i++) {
            resultStr[i] = printH(AddRec->getOperand(i));
        }
        for (i = 0; i < AddRec->getNumOperands() - 1; i++) {
            result += resultStr[i] + string("+");
        }

        result += resultStr[i];

        return "(" + result + ")";
    }
    if (const SCEVMulExpr *AddRec = dyn_cast<SCEVMulExpr>(scev)) {
        string resultStr[10];
        unsigned i;
        for (i = 0; i < AddRec->getNumOperands(); i++) {
            resultStr[i] = printH(AddRec->getOperand(i));
        }
        for (i = 0; i < AddRec->getNumOperands() - 1; i++) {
            result += resultStr[i] + string("*");
        }
        result += resultStr[i];
        return "(" + result + ")";
    }
    if (const SCEVUDivExpr *divExp = dyn_cast<SCEVUDivExpr>(scev)) {
        string resultStr[10];
        unsigned i = 0;
        resultStr[0] = printH(divExp->getLHS());
        resultStr[1] = printH(divExp->getRHS());
        result += resultStr[i] + string("/") + resultStr[1];
        return "(" + result + ")";
    }

    if (const SCEVConstant *conValue = dyn_cast<SCEVConstant>(scev))
        return std::to_string(
                dyn_cast<ConstantInt>(m_SCE.visit(conValue))->getSExtValue());
    if (const SCEVUnknown *unknown = dyn_cast<SCEVUnknown>(scev))
        return m_SCE.visit(unknown)->getName();
    return result;
}

vector<Constant *> SCEVParser::createAString(std::string input, Type *Int8Ty) {
    vector<Constant *> Init;
    for (unsigned i = 0; i < input.size(); i++) {
        Init.push_back(ConstantInt::get(Int8Ty, input[i]));
    }
    return Init;
}


Instruction *SCEVParser::getValueInducation() {
    if (m_InsVec.size() == 0)
        return nullptr;
    return m_InsVec[0];
}

void SCEVParser::parseExpr(int index, char LoadStore) {
    m_InsVec2.clear();
    m_stepValue.clear();
    searchForValue(p_SCEVP, m_InsVec2);
    computerStepInstruction(p_SCEVP);
    StringRef Name = "SCEV";
    const SCEV *init = computeInit(p_SCEVP);
    const SCEV *innerValueMin = computeInitIter(p_SCEVP, status::min);
    const SCEV *innerValueMax = computeInitIter(p_SCEVP, status::max);
    Type *Int8Ty = llvm::Type::getInt8Ty(p_M->getContext());
    ArrayType *ATy;
    std::string MINA = std::string("0");
    std::string MAXA = std::string("0");
    if (m_stepValue.size() != 0) {
        MINA = printH(p_SEL->getSCEV(m_stepValue.back().second));
        MAXA = printH(p_SEL->getSCEV(m_stepValue.back().second));
    }

    vector<Constant *> Init =
            createAString("MIN-A-" + std::to_string(index) + "-" + LoadStore +
                          "-0:" + MINA + std::string("#"),
                          Int8Ty);
    ATy = ArrayType::get(Int8Ty, Init.size());

    llvm::GlobalVariable *GV0 =
            new llvm::GlobalVariable(*p_M, ATy, false, GlobalValue::ExternalLinkage,
                                     ConstantArray::get(ATy, Init), Name, nullptr);

    Init = createAString("MAX-A-" + std::to_string(index) + "-" + LoadStore +
                         "-0:" + MAXA + "#",
                         Int8Ty);
    ATy = ArrayType::get(Int8Ty, Init.size());
    llvm::GlobalVariable *GV1 =
            new llvm::GlobalVariable(*p_M, ATy, false, GlobalValue::ExternalLinkage,
                                     ConstantArray::get(ATy, Init), Name, GV0);

    Init =
            createAString("MIN-B-" + std::to_string(index) + "-" + LoadStore + "-0:" +
                          printH(p_SEL->getAddExpr(init, innerValueMin)) + "#",
                          Int8Ty);
    ATy = ArrayType::get(Int8Ty, Init.size());

    llvm::GlobalVariable *GV2 =
            new llvm::GlobalVariable(*p_M, ATy, false, GlobalValue::ExternalLinkage,
                                     ConstantArray::get(ATy, Init), Name, GV1);

    Init =
            createAString("MAX-B-" + std::to_string(index) + "-" + LoadStore + "-0:" +
                          printH(p_SEL->getAddExpr(init, innerValueMax)) + "#",
                          Int8Ty);
    ATy = ArrayType::get(Int8Ty, Init.size());
    llvm::GlobalVariable *GV3 =
            new llvm::GlobalVariable(*p_M, ATy, false, GlobalValue::ExternalLinkage,
                                     ConstantArray::get(ATy, Init), Name, GV2);

    GV0->setSection(".SCEV");
    GV1->setSection(".SCEV");
    GV2->setSection(".SCEV");
    GV3->setSection(".SCEV");
    //errs() << "SCEV Was set\n";
}
