//===---- TPCAsmParser.cpp --- Parse TPC assembly instructions ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/InstructionDB.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"
#include "MCTargetDesc/TPCInstPrinter.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/SmallBitVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/MC/MCFragment.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstBuilder.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TPCMetadataSection.h"
#include "llvm/Target/TPCKernelInfo.h"
#include "TPCAsmInstCompress.h"
#include <algorithm>
#include <deque>
#include <bitset>
#include <map>
#include <unordered_set>
#include <set>


using namespace llvm;
using TPCII::SlotParser;

#define DEBUG_TYPE "tpc-asm"

namespace {

// Names of parsers, needed for diagnostics only.
const char * SlotName[] = {
  "special",
  "load",
  "scalar",
  "vector",
  "store"
};


enum class LongRegisterKind {
  ARF,
  DRF,
  ZRF
};


enum class LoopCompare {
  GT  = 0x00,
  GE  = 0x01,
  LT  = 0x02,
  LE  = 0x03,
  EQ  = 0x04,
  NE  = 0x05
};

static const StringRef LoopCompareText[] = {
  ">", ">=", "<", "<=", "==", "!="
};

static const char *IndexMapSectionName = ".IndexMap";

static StringRef getStringFromLoc(SMLoc Start, SMLoc End) {
  assert(End.getPointer() >= Start.getPointer());
  return StringRef(Start.getPointer(), End.getPointer() - Start.getPointer());
}


class TPCOperand : public MCParsedAsmOperand {
  // Enum for operand types. SwitchSet is a special case:
  // We have a lot of switches that determine signature and encoding format
  // So we have a general switch mask, but it must have different type
  // if some specific switches are present in the set.
  // That's why we desgnate a base for a base SwitchSet that does not
  // change signature or encoding of instruction, and other switches are added to it
  // bitwise. Because of that single switches should not intersect bitwise.
public:
  enum class OpKind : uint64_t {
    Invalid,
    Token,
    Reg,
    Imm,
    DataType,
    DimMask,
    Comparison,
    Accumulator,
    MemRI,
    ABCinNormSel,
    Predicate,
    RoundMode,
    Rhu,
    RhazRs,
    Biased,
    GroupDesc,
    DualGroup,
    WR,
    SDG,
    WEG,
    RegSelFrom,
    RegSelTo,
    BothDivMod,
    X2,
    MovDGAll,
    MovDGPack,
    MovDGUnpack,
    SwitchSet = 32768,
    SwitchLutPtr = SwitchSet | 1,
    SwitchAll = SwitchSet | 8,
  };

private:
  OpKind Kind;
  SMLoc StartLoc, EndLoc;

  struct RegOp {
    int Class;      // Like TPC::SRFRegClass
    unsigned Num;
  };

  struct ImmOp {
    const MCExpr *Expr = nullptr;
    int64_t Val;
  };

  struct PredicateOp {
    unsigned RegNum;
    bool Polarity;
    bool IsVector;
  };

  struct MemRIInfo {
    unsigned Reg;
    unsigned Offset;
  };

  struct I8Const {
    unsigned Val;
  };

  struct MDGOp {
    unsigned Val;
    StringRef Name;
  };

  struct RhuOp {
    unsigned Val;
  };

  struct RhazRsOp {
    unsigned Val;
  };

  struct RoundModeOp {
    StringRef Name;
    unsigned Val;
  };

  struct RegSelFromOp {
    StringRef Name;
    unsigned Val;
  };

  struct RegSelToOp {
    StringRef Name;
    unsigned Val;
  };

  struct BothDivModOp {
    unsigned Val;
  };

  struct X2Op {
    unsigned Val;
  };

  struct MovDGAllOp {
    unsigned Val;
  };

  struct MovDGPackOp {
    unsigned Val;
  };

  struct MovDGUnpackOp {
    unsigned Val;
  };

  struct TokenOp {
    const char *Data;
    unsigned Length;
  };

  struct BinSwitch {
    unsigned Val;
  };

  union {
    TokenOp Token;
    RegOp Reg;
    ImmOp Imm;
    TPCII::OpType DataType;
    PredicateOp Pred;
    unsigned SwitchSet;
    unsigned DimMask;
    LoopCompare CompInfo;
    MemRIInfo MemRI;
    RhuOp Rhu;
    RhazRsOp RhazRs;
    I8Const Biased;
    I8Const GroupDesc;
    RoundModeOp RoundMode;
    MDGOp   MovDualGroup;
    RegSelFromOp RegSelFrom;
    RegSelToOp RegSelTo;
    BothDivModOp BothDivMod;
    X2Op X2;
    MovDGAllOp MovDGAll;
    MovDGPackOp MovDGPack;
    MovDGUnpackOp MovDGUnpack;
  };

  std::vector<std::string> SwitchNames;

public:
  TPCOperand(OpKind K, SMLoc S, SMLoc E)
    : Kind(K), StartLoc(S), EndLoc(E) {
  }

  //------ Register operand

  bool isReg() const override { return Kind == OpKind::Reg; }

  unsigned getReg() const override {
    assert(Kind == OpKind::Reg && "Invalid access!");
    return Reg.Num;
  }

  int getRegClass() const {
    assert(Kind == OpKind::Reg && "Invalid access!");
    return Reg.Class;
  }

  static std::unique_ptr<TPCOperand>
    CreateReg(unsigned RegNo, int RegClassNo, SMLoc StartLoc, SMLoc EndLoc) {
    auto Res = std::make_unique<TPCOperand>(OpKind::Reg, StartLoc, EndLoc);
    Res->Reg.Num = RegNo;
    Res->Reg.Class = RegClassNo;
    return Res;
  }

  //------ Token operand

  bool isToken() const override { return Kind == OpKind::Token; }

  StringRef getToken() const {
    assert(Kind == OpKind::Token && "Invalid access!");
    return StringRef(Token.Data, Token.Length);
  }

  void truncateToken(unsigned NewLen) {
    assert(Kind == OpKind::Token && "Invalid access!");
    assert(Token.Length >= NewLen);
    Token.Length = NewLen;
    EndLoc = SMLoc::getFromPointer(StartLoc.getPointer() + NewLen);
  }

  static std::unique_ptr<TPCOperand> CreateToken(StringRef Str, SMLoc S) {
    auto Op = std::make_unique<TPCOperand>(OpKind::Token, S, S);
    Op->Token.Data = Str.data();
    Op->Token.Length = Str.size();
    Op->StartLoc = S;
    Op->EndLoc = SMLoc::getFromPointer(S.getPointer() + Str.size());
    return Op;
  }

  //------ Immediate operand

  bool isImm() const override {
    return Kind == OpKind::Imm;
  }

  int64_t getImm() const {
    assert(Kind == OpKind::Imm && "Not an immediate");
    assert(Imm.Expr == nullptr && "Immediate is an expression");
    return Imm.Val;
  }


  static std::unique_ptr<TPCOperand> CreateImm(const MCExpr *EVal, SMLoc S,
                                               SMLoc E) {
    if (EVal->getKind() == MCExpr::Constant) {
      // If the expression is a constant, resolve it immediately.
      int64_t Val;
      if (EVal->evaluateAsAbsolute(Val)) {
        auto Op = std::make_unique<TPCOperand>(OpKind::Imm, S, E);
        Op->Imm.Expr = nullptr;
        Op->Imm.Val = Val;
        Op->StartLoc = S;
        Op->EndLoc = E;
        return Op;
      }
    } else {
      auto Op = std::make_unique<TPCOperand>(OpKind::Imm, S, E);
      Op->Imm.Expr = EVal;
      Op->StartLoc = S;
      Op->EndLoc = E;
      return Op;
    }
    return std::unique_ptr<TPCOperand>();
  }

  static std::unique_ptr<TPCOperand> CreateImm(int64_t Val, SMLoc S, SMLoc E) {
    auto Op = std::make_unique<TPCOperand>(OpKind::Imm, S, E);
    Op->Imm.Val = Val;
    Op->Imm.Expr = nullptr;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  //------ DataType operand

  bool isDataType() const { return Kind == OpKind::DataType; }

  TPCII::OpType getDataType() const {
    assert(Kind == OpKind::DataType && "Not a data type");
    return DataType;
  }

  static std::unique_ptr<TPCOperand> CreateDataType(TPCII::OpType Value,
                                                    SMLoc StartLoc, SMLoc EndLoc) {
    auto Res = std::make_unique<TPCOperand>(OpKind::DataType, StartLoc, EndLoc);
    Res->DataType = Value;
    return Res;
  }

  //------ Switch set operand

  bool isSwitchSet() const { return Kind == OpKind::SwitchSet; }
  bool isSwitchLutPtr() const { return Kind == OpKind::SwitchLutPtr; }
  bool isSwitchAll() const { return Kind == OpKind::SwitchAll; }

  unsigned getSwitchSet() const {
    assert(Kind >= OpKind::SwitchSet && "Not a switch set");
    return SwitchSet;
  }

  void setSwitchSet(unsigned X) {
    assert(Kind >= OpKind::SwitchSet && "Not a switch set");
    SwitchSet = X;
  }

  std::vector<std::string> &getSwitchNames() { return SwitchNames; }

  static std::unique_ptr<TPCOperand> CreateSwitchSet(SMLoc StartLoc, SMLoc EndLoc) {
    auto Res = std::make_unique<TPCOperand>(OpKind::SwitchSet, StartLoc, EndLoc);
    Res->SwitchSet = 0;
    return Res;
  }

  //------ DimMask operand

  bool isDimMask() const { return Kind == OpKind::DimMask; }

  unsigned getDimMask() const {
    assert(Kind == OpKind::DimMask && "Not a dimmask");
    return DimMask;
  }

  static std::unique_ptr<TPCOperand> CreateDimMask(unsigned Value,
                                                   SMLoc StartLoc, SMLoc EndLoc) {
    auto Res = std::make_unique<TPCOperand>(OpKind::DimMask, StartLoc, EndLoc);
    Res->DimMask = Value;
    return Res;
  }

  //------ Comparison operand

  bool isComparison() const { return Kind == OpKind::Comparison; }

  LoopCompare getComparison() const {
    assert(Kind == OpKind::Comparison && "Not a comparison");
    return CompInfo;
  }

  static std::unique_ptr<TPCOperand> CreateComparison(LoopCompare Value,
                                                      SMLoc StartLoc, SMLoc EndLoc) {
    auto Res = std::make_unique<TPCOperand>(OpKind::Comparison, StartLoc, EndLoc);
    Res->CompInfo = Value;
    return Res;
  }

  //------ MemRI operand

  bool isMemRI() const { return Kind == OpKind::MemRI; }

  static std::unique_ptr<TPCOperand> CreateMemRI(unsigned Reg, unsigned Offset,
                                                 SMLoc StartLoc, SMLoc EndLoc) {
    auto Res = std::make_unique<TPCOperand>(OpKind::MemRI, StartLoc, EndLoc);
    Res->MemRI.Reg = Reg;
    Res->MemRI.Offset = Offset;
    return Res;
  }

  //------ Accumulator operand

  bool isAccumulator() const { return Kind == OpKind::Accumulator; }

  static std::unique_ptr<TPCOperand> CreateAccumulator(SMLoc StartLoc, SMLoc EndLoc) {
    auto Res = std::make_unique<TPCOperand>(OpKind::Accumulator, StartLoc, EndLoc);
    return Res;
  }

  //------ BothDivMod operand

  bool isBothDivMod() const { return Kind == OpKind::BothDivMod; }

  static std::unique_ptr<TPCOperand> CreateBothDivMod(
      unsigned Val, SMLoc StartLoc, SMLoc EndLoc) {
    auto Res = std::make_unique<TPCOperand>(OpKind::BothDivMod, StartLoc, EndLoc);
    Res->BothDivMod.Val = Val;
    return Res;
  }

  //------ X2 operand

  bool isX2() const { return Kind == OpKind::X2; }

  static std::unique_ptr<TPCOperand> CreateX2(
      unsigned Val, SMLoc StartLoc, SMLoc EndLoc) {
    auto Res = std::make_unique<TPCOperand>(OpKind::X2, StartLoc, EndLoc);
    Res->X2.Val = Val;
    return Res;
  }

  //------ MovDGAll

  bool isMovDGAll() const { return Kind == OpKind::MovDGAll; }

  static std::unique_ptr<TPCOperand> CreateMovDGAll(
      unsigned Val, SMLoc StartLoc, SMLoc EndLoc) {
    auto Res = std::make_unique<TPCOperand>(OpKind::MovDGAll, StartLoc, EndLoc);
    Res->MovDGAll.Val = Val;
    return Res;
  }

  //------ MovDGPack

  bool isMovDGPack() const { return Kind == OpKind::MovDGPack; }

  static std::unique_ptr<TPCOperand> CreateMovDGPack(
      unsigned Val, SMLoc StartLoc, SMLoc EndLoc) {
    auto Res = std::make_unique<TPCOperand>(OpKind::MovDGPack, StartLoc, EndLoc);
    Res->MovDGPack.Val = Val;
    return Res;
  }

  //------ MovDGUnpack

  bool isMovDGUnpack() const { return Kind == OpKind::MovDGUnpack; }

  static std::unique_ptr<TPCOperand> CreateMovDGUnpack(
      unsigned Val, SMLoc StartLoc, SMLoc EndLoc) {
    auto Res = std::make_unique<TPCOperand>(OpKind::MovDGUnpack, StartLoc, EndLoc);
    Res->MovDGUnpack.Val = Val;
    return Res;
  }

  //------ Memory operand

  bool isMem() const override { return false; }

  //------ Predicate operand

  bool isPredicate() const { return Kind == OpKind::Predicate; }
  bool isSPredicate() const { return Kind == OpKind::Predicate && !Pred.IsVector; }
  bool isVPredicate() const { return Kind == OpKind::Predicate && Pred.IsVector; }

  PredicateOp getPredicate() const {
    assert(isPredicate());
    return Pred;
  }

  static std::unique_ptr<TPCOperand> CreatePredicate(int RegNum, bool Polarity, bool IsVector,
                                                     SMLoc StartLoc, SMLoc EndLoc) {
    auto Res = std::make_unique<TPCOperand>(OpKind::Predicate, StartLoc, EndLoc);
    Res->Pred.RegNum = RegNum;
    Res->Pred.Polarity = Polarity;
    Res->Pred.IsVector = IsVector;
    return Res;
  }

  //------

  // Used by the TableGen code to add particular types of operand
  // to an instruction.
  void addRegOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands");
    Inst.addOperand(MCOperand::createReg(getReg()));
  }

  void addImmOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands");
    assert(isImm());
    if (Imm.Expr)
      Inst.addOperand(MCOperand::createExpr(Imm.Expr));
    else
      Inst.addOperand(MCOperand::createImm(Imm.Val));
  }

  void addDataTypeOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands");
    Inst.addOperand(MCOperand::createImm(getDataType()));
  }

  void addDimMaskOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands");
    Inst.addOperand(MCOperand::createImm(getDimMask()));
  }

  void addSwitchSetOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands");
    Inst.addOperand(MCOperand::createImm(getSwitchSet()));
  }

  void addComparisonOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands");
    Inst.addOperand(MCOperand::createImm(static_cast<int>(getComparison())));
  }

  void addAccumulatorOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands");
    Inst.addOperand(MCOperand::createImm(0));
  }

  void addMemRIOperands(MCInst &Inst, unsigned N) const {
    assert(N == 2 && "Invalid number of operands");
    assert(isMemRI());
    Inst.addOperand(MCOperand::createReg(MemRI.Reg));
    Inst.addOperand(MCOperand::createImm(MemRI.Offset));
  }

  void addSPredicateOperands(MCInst &Inst, unsigned N) const {
    assert(N == 2 && "Invalid number of operands");
    PredicateOp Pred = getPredicate();
    assert(!Pred.IsVector);
    Inst.addOperand(MCOperand::createReg(Pred.RegNum));
    Inst.addOperand(MCOperand::createImm(Pred.Polarity));
  }

  void addVPredicateOperands(MCInst &Inst, unsigned N) const {
    assert(N == 2 && "Invalid number of operands");
    PredicateOp Pred = getPredicate();
    assert(Pred.IsVector);
    Inst.addOperand(MCOperand::createReg(Pred.RegNum));
    Inst.addOperand(MCOperand::createImm(Pred.Polarity));
  }

  void addDualGroupOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands");
    assert(isDualGroup() && "Invalid operand type");
    Inst.addOperand(MCOperand::createImm(MovDualGroup.Val));
  }

  void addWROperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands");
    assert(isWR() && "Invalid operand type");
    Inst.addOperand(MCOperand::createImm(MovDualGroup.Val));
  }

  void addSDGOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands");
    assert(isSDG() && "Invalid operand type");
    Inst.addOperand(MCOperand::createImm(MovDualGroup.Val));
  }

  void addWEGOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands");
    assert(isWEG() && "Invalid operand type");
    Inst.addOperand(MCOperand::createImm(MovDualGroup.Val));
  }

  bool isRoundMode() const {
    return Kind == OpKind::RoundMode;
  }

  bool isRhu() const {
    return Kind == OpKind::Rhu;
  }

  bool isRhazRs() const {
    return Kind == OpKind::RhazRs;
  }

  bool isBiased() const {
    return Kind == OpKind::Biased;
  }

  bool isGroupDesc() const {
    return Kind == OpKind::GroupDesc;
  }

  bool isRegSelFrom() const {
    return Kind == OpKind::RegSelFrom;
  }

  bool isRegSelTo() const {
      return Kind == OpKind::RegSelTo;
    }


  static std::unique_ptr<TPCOperand> CreateRhu(unsigned Val, SMLoc StartLoc, SMLoc EndLoc) {
    auto Res = std::make_unique<TPCOperand>(OpKind::Rhu, StartLoc, EndLoc);
    Res->Rhu.Val = Val;
    return Res;
  }

  static std::unique_ptr<TPCOperand> CreateRhazRs(unsigned Val, SMLoc StartLoc, SMLoc EndLoc) {
    auto Res = std::make_unique<TPCOperand>(OpKind::RhazRs, StartLoc, EndLoc);
    Res->RhazRs.Val = Val;
    return Res;
  }

  static std::unique_ptr<TPCOperand> CreateBiased(unsigned Val, SMLoc StartLoc, SMLoc EndLoc) {
    auto Res = std::make_unique<TPCOperand>(OpKind::Biased, StartLoc, EndLoc);
    Res->Biased.Val = Val;
    return Res;
  }

  static std::unique_ptr<TPCOperand> CreateRoundMode(StringRef Name, unsigned Val, SMLoc StartLoc, SMLoc EndLoc) {
    auto Res = std::make_unique<TPCOperand>(OpKind::RoundMode, StartLoc, EndLoc);
    Res->RoundMode.Name = Name;
    Res->RoundMode.Val = Val;
    return Res;
  }

  static std::unique_ptr<TPCOperand> CreateGroupDesc(unsigned Val, SMLoc StartLoc, SMLoc EndLoc) {
    auto Res = std::make_unique<TPCOperand>(OpKind::GroupDesc, StartLoc, EndLoc);
    Res->GroupDesc.Val = Val;
    return Res;
  }

  static std::unique_ptr<TPCOperand> CreateRegSelFrom(StringRef Name, unsigned Val, SMLoc StartLoc, SMLoc EndLoc) {
    auto Res = std::make_unique<TPCOperand>(OpKind::RegSelFrom, StartLoc, EndLoc);
    Res->RegSelFrom.Name = Name;
    Res->RegSelFrom.Val = Val;
    return Res;
  }

  static std::unique_ptr<TPCOperand> CreateRegSelTo(StringRef Name, unsigned Val, SMLoc StartLoc, SMLoc EndLoc) {
    auto Res = std::make_unique<TPCOperand>(OpKind::RegSelTo, StartLoc, EndLoc);
    Res->RegSelTo.Name = Name;
    Res->RegSelTo.Val = Val;
    return Res;
  }

  void addRegSelFromOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands");
    assert(isRegSelFrom() && "Invalid operand type");
    Inst.addOperand(MCOperand::createImm(RegSelFrom.Val));
  }

  void addRegSelToOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands");
    assert(isRegSelTo() && "Invalid operand type");
    Inst.addOperand(MCOperand::createImm(RegSelTo.Val));
  }

  void addRhuOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands");
    assert(isRhu() && "Invalid operand type");
    Inst.addOperand(MCOperand::createImm(Rhu.Val));
  }

  void addRhazRsOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands");
    assert(isRhazRs() && "Invalid operand type");
    Inst.addOperand(MCOperand::createImm(RhazRs.Val));
  }

  void addBiasedOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands");
    assert(isBiased() && "Invalid operand type");
    Inst.addOperand(MCOperand::createImm(Biased.Val));
  }

  void addGroupDescOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands");
    assert(isGroupDesc() && "Invalid operand type");
    Inst.addOperand(MCOperand::createImm(GroupDesc.Val));
  }

  void addRoundModeOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands");
    assert(isRoundMode() && "Invalid operand type");
    Inst.addOperand(MCOperand::createImm(RoundMode.Val));
  }

  void addBothDivModOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands");
    assert(isBothDivMod() && "Invalid operand type");
    Inst.addOperand(MCOperand::createImm(BothDivMod.Val));
  }

  void addX2Operands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands");
    assert(isX2() && "Invalid operand type");
    Inst.addOperand(MCOperand::createImm(X2.Val));
  }

  void addMovDGAllOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands");
    assert(isMovDGAll() && "Invalid operand type");
    Inst.addOperand(MCOperand::createImm(MovDGAll.Val));
  }

  void addMovDGPackOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands");
    assert(isMovDGPack() && "Invalid operand type");
    Inst.addOperand(MCOperand::createImm(MovDGPack.Val));
  }

  void addMovDGUnpackOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands");
    assert(isMovDGUnpack() && "Invalid operand type");
    Inst.addOperand(MCOperand::createImm(MovDGUnpack.Val));
  }

  bool isDualGroup() const {
    return Kind == OpKind::DualGroup;
  }

  static std::unique_ptr<TPCOperand> CreateDualGroup(unsigned Val, StringRef Name, SMLoc StartLoc, SMLoc EndLoc) {
    auto Res = std::make_unique<TPCOperand>(OpKind::DualGroup, StartLoc, EndLoc);
    Res->MovDualGroup.Val = Val;
    Res->MovDualGroup.Name = Name;
    return Res;
  }

  bool isWR() const {
    return Kind == OpKind::WR;
  }

  static std::unique_ptr<TPCOperand> CreateWR(unsigned Val, StringRef Name, SMLoc StartLoc, SMLoc EndLoc) {
    auto Res = std::make_unique<TPCOperand>(OpKind::WR, StartLoc, EndLoc);
    Res->MovDualGroup.Val = Val;
    Res->MovDualGroup.Name = Name;
    return Res;
  }

  bool isSDG() const {
    return Kind == OpKind::SDG;
  }

  static std::unique_ptr<TPCOperand> CreateSDG(unsigned Val, StringRef Name, SMLoc StartLoc, SMLoc EndLoc) {
    auto Res = std::make_unique<TPCOperand>(OpKind::SDG, StartLoc, EndLoc);
    Res->MovDualGroup.Val = Val;
    Res->MovDualGroup.Name = Name;
    return Res;
  }

  bool isWEG() const {
    return Kind == OpKind::WEG;
  }

  static std::unique_ptr<TPCOperand> CreateWEG(unsigned Val, StringRef Name, SMLoc StartLoc, SMLoc EndLoc) {
    auto Res = std::make_unique<TPCOperand>(OpKind::WEG, StartLoc, EndLoc);
    Res->MovDualGroup.Val = Val;
    Res->MovDualGroup.Name = Name;
    return Res;
  }

  /// getStartLoc - Get the location of the first token of this operand.
  SMLoc getStartLoc() const override { return StartLoc; }

  /// getEndLoc - Get the location of the last token of this operand.
  SMLoc getEndLoc() const override { return EndLoc; }

  /// getLocRange - Get the range between the first and last token of this
  /// operand.
  SMRange getLocRange() const { return SMRange(StartLoc, EndLoc); }

  void print(raw_ostream &OS) const override {
    switch (Kind) {
    case OpKind::Token:
      OS << "Tok: " << Token.Data;
      break;
    case OpKind::Reg:
      OS << "Reg: " << TPCInstPrinter::getRegisterName(Reg.Num);
      break;
    case OpKind::Imm:
      if (Imm.Expr)
        OS << "ImmExpr: " << Imm.Expr;
      else
        OS << "Imm: " << Imm.Val;
      break;
    case OpKind::MemRI:
      OS << "Offset: " << MemRI.Offset << ", Reg: " << TPCInstPrinter::getRegisterName(MemRI.Reg);
      break;
    case OpKind::DataType: {
      static const StringRef OpTypes[] = {
        "f32", "bf16", "i32", "u32", "i8", "u8", "b", "i16", "u16", "f16", "f8_143", "f8_152", "i64"
      };
      assert(DataType <= TPCII::OpType::Max);
      OS << "DataType: " << OpTypes[DataType];
      break;
    }
    case OpKind::DimMask:
      assert(DimMask < 32);
      OS << "DimMask: " << std::bitset<5>(DimMask).to_string();
      break;
    case OpKind::SwitchSet:
      OS << "SwitchSet: " << SwitchSet;
      break;
    case OpKind::Comparison:
      OS << LoopCompareText[static_cast<unsigned>(CompInfo)];
      break;
    case OpKind::Accumulator:
      OS << "Acc ";
      break;
    case OpKind::SwitchLutPtr:
      OS << "SwitchSet with LutPtr: " << SwitchSet;
      break;
    case OpKind::SwitchAll:
      OS << "SwitchSet with All: " << SwitchSet;
      break;
    case OpKind::Predicate:
      OS << "Pred: ";
      if (Pred.Polarity)
        OS << "!";
      OS << TPCInstPrinter::getRegisterName(Pred.RegNum);
      break;
    case OpKind::GroupDesc:
      OS << "GroupDesc: " << GroupDesc.Val;
      break;
    case OpKind::RoundMode:
      OS << "RoundMode: " << RoundMode.Name;
      break;
    case OpKind::Rhu:
      OS << "RHU: "<< Rhu.Val;
      break;
    case OpKind::RhazRs:
      OS << "RHAZ_RS: "<< RhazRs.Val;
      break;
    case OpKind::DualGroup:
    case OpKind::WR:
    case OpKind::SDG:
    case OpKind::WEG:
      OS << MovDualGroup.Name << "=" << MovDualGroup.Val;
      break;
    case OpKind::RegSelFrom:
      OS << "RegSelFrom: " << RegSelFrom.Name;
      break;
    case OpKind::RegSelTo:
      OS << "RegSelTo: " << RegSelTo.Name;
      break;
    case OpKind::BothDivMod:
      OS << "BothDivMod: " << BothDivMod.Val;
      break;
    case OpKind::X2:
      OS << "X2: " << X2.Val;
      break;
    case OpKind::MovDGAll:
      OS << "MovDGAll: " << MovDGAll.Val;
      break;
    case OpKind::MovDGPack:
      OS << "MovDGPack: " << MovDGPack.Val;
      break;
    case OpKind::MovDGUnpack:
      OS << "MovDGUnpack: " << MovDGUnpack.Val;
      break;
    default:
      llvm_unreachable("Invalid operand kind");
    }
  }
};

class TPCAsmParser : public MCTargetAsmParser {
#define GET_ASSEMBLER_HEADER
#define GET_REGISTER_MATCHER
#include "TPCGenAsmMatcher.inc"

private:
  MCAsmParser &Parser;
  MCAsmLexer &Lexer;
  TPCAsmInstCompress AC;
  const MCRegisterInfo *MRI;
  const MCRegisterClass &SPRegClass;
  const MCRegisterClass &VPRegClass;

  MCInst Bundle;
  std::deque<MCInst> BundledSubInstructions;
  SlotParser CurrentSlot = SlotParser::Unknown;
  bool IsLastInstrInBundle = false;
  bool NewAsmFormat = false;

  std::set<StringRef> OperatorLabels;
  std::set<StringRef> FreeLabels;

  const char *TPCHeaderParseBaseError =
    "Unexpected token during parse tpc metadata: '";
  Optional<TPCMetadataSection> TPCHeader;
  Optional<std::string> IndexMap;

public:
  TPCAsmParser(const MCSubtargetInfo &sti, MCAsmParser &Parser,
               const MCInstrInfo &MII, const MCTargetOptions &Options)
    : MCTargetAsmParser(Options, sti, MII)
    , Parser(Parser)
    , Lexer(Parser.getLexer())
    , AC(MII)
    , MRI(Parser.getContext().getRegisterInfo())
    , SPRegClass(MRI->getRegClass(TPC::SPRFRegClassID))
    , VPRegClass(MRI->getRegClass(TPC::VPRFRegClassID)) {
    AC.setCompressEnabled(Options.MCCompressInst);
    MCAsmParserExtension::Initialize(Parser);
    TPCII::setSubTargetInfo(&sti);
    TPCII::setInstrInfo(&MII);

    // Initialize the set of available features.
    setAvailableFeatures(ComputeAvailableFeatures(getSTI().getFeatureBits()));
  }

  void onLabelParsed(MCSymbol *Symbol) override {
    AC.onLabelEmited(Symbol);

    FreeLabels.insert(Symbol->getName());
    LLVM_DEBUG(dbgs() << "Label detected: " << Symbol->getName() << "\n");
  }

  void onEndOfFile() override {
    for (const StringRef &Label : OperatorLabels)
      if (FreeLabels.find(Label) == FreeLabels.end())
        Error(SMLoc::getFromPointer(Label.begin()), "missing label",
              SMRange(SMLoc::getFromPointer(Label.begin()),
                      SMLoc::getFromPointer(Label.end())));
  }

private:

  SMLoc consumeToken() {
    SMLoc Result = Parser.getTok().getLoc();
    Parser.Lex();
    return Result;
  }

  enum PredicateKind {
    NotAPredicate,
    ScalarPredicate,
    VectorPredicate
  };

  PredicateKind getPredicateKind(unsigned RegNo) {
    if (SPRegClass.contains(RegNo))
      return ScalarPredicate;
    if (VPRegClass.contains(RegNo))
      return VectorPredicate;
    return NotAPredicate;
  }

  enum class SwitchParseState {
    Ok,
    Unknown,
    Error
  };

  SwitchParseState processSwitch(StringRef Switch, SlotParser Slot,
                                 StringRef Mnemonic, StringRef Value,
                                 OperandVector &Operands, SMLoc Start, SMLoc End);

  OperandMatchResultTy parseOperand(OperandVector &Operands, StringRef Mnemonic);
  OperandMatchResultTy parsePredicate(OperandVector &Operands);
  OperandMatchResultTy parseComparison(OperandVector &Operands);
  OperandMatchResultTy parseRegister(OperandVector &Operands);
  OperandMatchResultTy parseNumber(OperandVector &Operands);
  OperandMatchResultTy parseImmediate(OperandVector &Operands);
  OperandMatchResultTy parseMemRI(OperandVector &Operands);
  bool parseAsSeparateSwitch(StringRef Switch, StringRef Mnemonic,
                             SMRange Range, OperandVector &Operands,
                             unsigned SwitchValue, TPCOperand *SpecialOperand);

  OperandMatchResultTy parseRhu(OperandVector &Operands) {
    SMLoc S = Parser.getTok().getLoc();
    const AsmToken &Tok = Parser.getTok();
    StringRef RhuFlagStr = Tok.getString();

    unsigned RHU = StringSwitch<unsigned>(RhuFlagStr.lower())
        .Case("rhu", 2)
        .Default(~0U);

    if (RHU != ~0U) {
      Parser.Lex(); // Eat identifier token.
      Operands.push_back(TPCOperand::CreateRhu(int64_t(RHU), S, S));
      return MatchOperand_Success;
    } else {
      return MatchOperand_NoMatch;
    }
  }

  OperandMatchResultTy parseRhazRs(OperandVector &Operands) {
    SMLoc S = Parser.getTok().getLoc();
    const AsmToken &Tok = Parser.getTok();
    StringRef RhazRsFlagStr = Tok.getString();

    unsigned RhazRs = StringSwitch<unsigned>(RhazRsFlagStr.lower())
        .Case("rhaz_rs", 4)
        .Default(~0U);

    if (RhazRs != ~0U) {
      Parser.Lex(); // Eat identifier token.
      Operands.push_back(TPCOperand::CreateRhazRs(int64_t(RhazRs), S, S));
      return MatchOperand_Success;
    } else {
      return MatchOperand_NoMatch;
    }
  }

  OperandMatchResultTy parseBiased(OperandVector &Operands) {
    SMLoc S = Parser.getTok().getLoc();
    const AsmToken &Tok = Parser.getTok();
    StringRef BiasedFlagStr = Tok.getString();

    unsigned Biased = StringSwitch<unsigned>(BiasedFlagStr.lower())
      .Case("biased", 1)
      .Default(~0U);

    if (Biased != ~0U) {
      Parser.Lex(); // Eat identifier token.
      Operands.push_back(TPCOperand::CreateBiased(int64_t(Biased), S, S));
      return MatchOperand_Success;
    }
    else {
      return MatchOperand_NoMatch;
    }
  }

  OperandMatchResultTy parseGroupDesc(OperandVector &Operands) {
    static StringRef incorrectGroupDesc = "Unexpected group description.";

    static std::map<StringRef, unsigned> maxValues {
      { "wr_lower_group", 1 },
      { "wr_upper_group", 1 },
      { "src_dual_group", 3 },
      { "dst_dual_group", 3 },
      { "group_en",       3 },
      { "lane_sel",       3 },
      { "dual_group_en",  15 }
    };

    SMLoc S = Parser.getTok().getLoc();
    const AsmToken &Tok = Parser.getTok();
    StringRef groupDesc = Tok.getString();

    if (maxValues.count(groupDesc.lower()) > 0) {
      Parser.Lex(); // Eat identifier token.
      if (Tok.getString() != "=") {
        Error(S, incorrectGroupDesc);
        return MatchOperand_ParseFail;
      }

      Parser.Lex();
      if (!Tok.is(AsmToken::Integer)) {
        Error(S, incorrectGroupDesc);
        return MatchOperand_ParseFail;
      }

      int64_t val = Tok.getIntVal();
      Parser.Lex(); // Eat value.
      if (val > maxValues[groupDesc.lower()]) {
        Error(S, incorrectGroupDesc);
        return MatchOperand_ParseFail;
      }

      Operands.push_back(TPCOperand::CreateGroupDesc(int64_t(val), S, S));
      return MatchOperand_Success;
    }

    return MatchOperand_NoMatch;
  }


  OperandMatchResultTy parseRegSelFrom(OperandVector &Operands) {
    SMLoc S = Parser.getTok().getLoc();
    const AsmToken &Tok = Parser.getTok();
    StringRef opName = Tok.getString();

    StringRef val = Tok.getString();
    unsigned regSel = StringSwitch<unsigned>(val.upper())
      .Case("PC", 0)
      .Case("S_CARRY", 2)
      .Case("V_CARRY", 3)
      .Default(~0U);

    if (regSel != ~0U) {
      Parser.Lex();
      Operands.push_back(TPCOperand::CreateRegSelFrom(opName, int64_t(regSel), S, S));
      return MatchOperand_Success;
    }
    else {
      return MatchOperand_NoMatch;
    }
  }
  OperandMatchResultTy parseRegSelTo(OperandVector &Operands) {
    SMLoc S = Parser.getTok().getLoc();
    const AsmToken &Tok = Parser.getTok();
    StringRef opName = Tok.getString();

#if 0
    StringRef val = Tok.getString();
    // OLD RegSelTo. Now we use Reg class for HW regs
    unsigned regSel = StringSwitch<unsigned>(val.lower())
      .Case("div_step", 1)
      .Case("s_carry",  2)
      .Case("v_carry",  3)
      .Case("irf_dim_mask0",  4)
      .Case("irf_dim_mask1",  5)
      .Case("irf_dim_mask2",  6)
      .Case("irf_dim_mask3",  7)
      .Case("ld_tnsr_id_reg", 8)
      .Case("st_tnsr_id_reg", 9)
      .Case("st_rmw_reg",    10)
      .Case("ld_partial_reg",  11)
      .Case("st_partial_reg",  12)
      .Default(~0U);
#else
    unsigned regSel = ~0U;
#endif

    if (regSel != ~0U) {
      Parser.Lex();
      Operands.push_back(TPCOperand::CreateRegSelTo(opName, int64_t(regSel), S, S));
      return MatchOperand_Success;
    }
    else {
      return MatchOperand_NoMatch;
    }
  }

  OperandMatchResultTy parseBothDivMod(OperandVector &Operands) {
    SMLoc S = Parser.getTok().getLoc();
    const AsmToken &Tok = Parser.getTok();
    StringRef OpName = Tok.getString();

    unsigned BothDivMod = StringSwitch<unsigned>(OpName.lower())
        .Case("both_div_mod", 2)
        .Default(~0U);

    if (BothDivMod != ~0U) {
      Parser.Lex(); // Eat identifier token.
      Operands.push_back(TPCOperand::CreateBothDivMod(
                           TPCII::SW_DIV_MODE_BOTH,S, S));
      return MatchOperand_Success;
    } else {
      return MatchOperand_NoMatch;
    }
  }

  std::unique_ptr<TPCOperand> defaultSPredicateOperands() {
    return TPCOperand::CreatePredicate(TPC::SPRF_TRUE, false, false, SMLoc(),
                                       SMLoc());
  }

  static std::unique_ptr<TPCOperand> defaultRhuOperands() {
    return TPCOperand::CreateRhu(0, SMLoc(), SMLoc());
  }

  static std::unique_ptr<TPCOperand> defaultRhazRsOperands() {
    return TPCOperand::CreateRhazRs(0, SMLoc(), SMLoc());
  }

  static std::unique_ptr<TPCOperand> defaultBiasedOperands() {
    return TPCOperand::CreateBiased(0, SMLoc(), SMLoc());
  }

  enum class MatchCode {
    Ok,           // Mnemonic matched successfully.
    Unrecognized, // Mnemonic is not recognized.
    Failed        // Error detected.
  };

  MatchCode MatchSlotInstruction(SMLoc IDLoc,
                                 OperandVector &Operands,
                                 MCInst &Inst,
                                 uint64_t &ErrorInfo,
                                 bool MatchingInlineAsm,
                                 MCStreamer &Out,
                                 SlotParser Slot);


  bool Error(SMLoc L, const Twine &Msg, SMRange Range = None,
             bool MatchingInlineAsm = false) {
    MCAsmParser &Parser = getParser();
    if (MatchingInlineAsm) {
      if (!getLexer().isAtStartOfStatement())
        eatToEndOfLine();
      return false;
    }
    bool Result = Parser.Error(L, Msg, Range);
    eatToEndOfLine();
    CurrentSlot = SlotParser::Unknown;
    // Avoid unfinished bundles.
    Bundle.clear();
    Bundle.setOpcode(0);
    Bundle.setLoc(SMLoc());

    return Result;
  }

  MCInst *getNOP(SlotParser Slot, SMLoc Loc);
  MCInst *getHALT(SlotParser Slot, SMLoc Loc);

  bool canLastArgBeAPredicate(StringRef Mnemonic, const OperandVector &Operands,
                              bool AlreadyParsed = false);
  void eatToEndOfLine();
  void onEndOfStatement() override {
    if (!NewAsmFormat) {
      CurrentSlot = SlotParser::Unknown;
    }
  }

  bool adjustLongRegister(OperandVector &Operands);
  unsigned convertToLongReg(unsigned RegNo, LongRegisterKind Kind);
  bool replaceRegisterWithLong(OperandVector &Operands, unsigned RegOpNum,
                               LongRegisterKind Kind);
  void initBundle(SMLoc Loc);
  void outputBundle(MCStreamer &Out, SMLoc IDLoc, bool UseStreamer);

  bool parseTPCMetadata();
  bool parseTPCMetadataArrayField(std::bitset<TPCNumTensors>& SetIndexes,
                                  const TPCMetadataFieldInfo& FieldInfo);
  bool parseTPCMetadataTypeDirective(unsigned ExpectedSize);

  bool parseIndexMap();
public:
  bool ParseRegister(unsigned &RegNo, SMLoc &StartLoc, SMLoc &EndLoc) override;
  OperandMatchResultTy tryParseRegister(unsigned &RegNo, SMLoc &StartLoc, SMLoc &EndLoc) override;
  bool ParseImmediate(const MCExpr *&Val, SMLoc &StartLoc, SMLoc &EndLoc);
  bool ParseInstruction(ParseInstructionInfo &Info, StringRef Name,
                        SMLoc NameLoc, OperandVector &Operands) override;
  bool ParseDirective(AsmToken DirectiveID) override;
  bool MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                               OperandVector &Operands, MCStreamer &Out,
                               uint64_t &ErrorInfo,
                               bool MatchingInlineAsm) override;
  // May call several times in some situation.
  // See RC99/asm/compare/aso-01.s
  void flushPendingInstructions(MCStreamer &Out) override;

};
} // end anonymous namespace


#define GET_REGISTER_MATCHER
#define GET_SUBTARGET_FEATURE_NAME
#define GET_MATCHER_IMPLEMENTATION
#include "TPCGenAsmMatcher.inc"


static bool isDimMask(StringRef Identifier) {
  if (Identifier.startswith("b")) {
    StringRef Remainder = Identifier.drop_front(1);
    if (Remainder.find_first_not_of("01") == StringRef::npos)
      return true;
  }
  return false;
}

bool TPCAsmParser::ParseRegister(unsigned &RegNo, SMLoc &StartLoc, SMLoc &EndLoc) {
  OperandMatchResultTy Res = tryParseRegister(RegNo, StartLoc, EndLoc);
  if (Res == MatchOperand_Success) {
    Parser.Lex();
    return false;
  }
  return Error(StartLoc, "invalid register name", SMRange(StartLoc, EndLoc));
}

OperandMatchResultTy TPCAsmParser::tryParseRegister(unsigned &RegNo, SMLoc &StartLoc, SMLoc &EndLoc) {
  MCAsmParser &Parser = getParser();
  const AsmToken &Tok = Parser.getTok();

  // Registers in TPC assembler are always identifiers.
  if (Tok.isNot(AsmToken::Identifier))
    return MatchOperand_NoMatch;

  StartLoc = Tok.getLoc();
  EndLoc = Tok.getEndLoc();
  StringRef TokStr = Tok.getString();
  RegNo = MatchRegisterName(TokStr);

  // If the match failed, try the register name as lowercase.
  if (RegNo == 0)
    RegNo = MatchRegisterName(TokStr.lower());

  return RegNo ? MatchOperand_Success : MatchOperand_NoMatch;
}

bool TPCAsmParser::ParseImmediate(const MCExpr *&Val, SMLoc &StartLoc, SMLoc &EndLoc) {
  MCAsmParser &Parser = getParser();
  Val = 0;
  const AsmToken &PercentTok = Parser.getTok();
  StartLoc = PercentTok.getLoc();
  const AsmToken &Tok = Parser.getTok();
  EndLoc = Tok.getEndLoc();

  if (!getParser().parseExpression(Val, StartLoc))
    return false;

  return Error(StartLoc, "invalid immediate value", SMRange(StartLoc, EndLoc));
}


OperandMatchResultTy TPCAsmParser::parsePredicate(OperandVector &Operands) {
  SMLoc StartLoc = Lexer.getTok().getLoc();

  // Optional '!'.
  bool Inversion = false;
  if (Lexer.getTok().is(AsmToken::Exclaim)) {
    Inversion = true;
    Parser.Lex();
  }

  // If '!' is encountered, it must be a predicate, otherwise it is an error.
  OperandMatchResultTy FailResult = Inversion ? MatchOperand_ParseFail
                                              : MatchOperand_NoMatch;

  // Register name.
  AsmToken RegisterTok = Lexer.getTok();
  SMLoc EndLoc = RegisterTok.getEndLoc();

  if (RegisterTok.isNot(AsmToken::Identifier)) {
    if (FailResult == MatchOperand_ParseFail) {
      Parser.Lex();
      Error(StartLoc, "expected predicate register", SMRange(StartLoc, EndLoc));
    }
    return FailResult;
  }

  unsigned RegNo = MatchRegisterName(RegisterTok.getString());
  // If the match failed, try the register name as lowercase.
  if (RegNo == 0)
    RegNo = MatchRegisterName(RegisterTok.getString().lower());

  if (RegNo == 0) {
    if (FailResult == MatchOperand_ParseFail) {
      Parser.Lex();
      Error(StartLoc, "expected predicate register", SMRange(StartLoc, EndLoc));
    }
    return FailResult;
  }

  bool IsVectopPred = VPRegClass.contains(RegNo);
  if (!SPRegClass.contains(RegNo) && !IsVectopPred) {
    if (FailResult == MatchOperand_ParseFail) {
      Parser.Lex();
      Error(StartLoc, "expected predicate register", SMRange(StartLoc, EndLoc));
    }
    return FailResult;
  }

  // If the last operand is a predicate register, it can define a predicate or
  // be a source.
  if (!Inversion) {
    TPCOperand &Op = static_cast<TPCOperand &>(*Operands[0]);
    assert(Op.isToken() && "Leading operand should always be a mnemonic!");
    StringRef Mnemonic = Op.getToken();
    if (!canLastArgBeAPredicate(Mnemonic, Operands))
      return MatchOperand_NoMatch;
  }

  Parser.Lex();
  Operands.push_back(
    TPCOperand::CreatePredicate(RegNo, Inversion,
                                getPredicateKind(RegNo) == VectorPredicate,
                                StartLoc, EndLoc));
  return MatchOperand_Success;
}


OperandMatchResultTy TPCAsmParser::parseComparison(OperandVector &Operands) {
  const AsmToken &Tok = Lexer.getTok();

  LoopCompare Result;
  if (Tok.is(AsmToken::Greater))
    Result = LoopCompare::GT;
  else if (Tok.is(AsmToken::GreaterEqual))
    Result = LoopCompare::GE;
  else if (Tok.is(AsmToken::Less))
    Result = LoopCompare::LT;
  else if (Tok.is(AsmToken::LessEqual))
    Result = LoopCompare::LE;
  else if (Tok.is(AsmToken::EqualEqual))
    Result = LoopCompare::EQ;
  else if (Tok.is(AsmToken::ExclaimEqual))
    Result = LoopCompare::NE;
  else
    return MatchOperand_NoMatch;

  SMLoc StartLoc = Tok.getLoc();
  SMLoc EndLoc = Tok.getEndLoc();
  Parser.Lex();
  Operands.push_back(TPCOperand::CreateComparison(Result, StartLoc, EndLoc));
  return MatchOperand_Success;
}


OperandMatchResultTy TPCAsmParser::parseRegister(OperandVector &Operands) {
  MCAsmParser &Parser = getParser();
  const AsmToken &Tok = Parser.getTok();
  if (Tok.isNot(AsmToken::Identifier))
    return MatchOperand_NoMatch;

  StringRef IdValue = Tok.getString();
  unsigned RegNo = MatchRegisterName(IdValue.upper());
  if (RegNo == 0) {
    if (IdValue.startswith_lower("d") || IdValue.startswith_lower("a")) {
      StringRef Number = IdValue.drop_front(1);
      if (Number.consumeInteger(10, RegNo))
        return MatchOperand_NoMatch;
      if (!Number.empty())
        return MatchOperand_NoMatch;
      if (IdValue.startswith_lower("d")) {
        if (RegNo % 2) {
          Error(Tok.getLoc(), "Pair of registers must start with register of even number", SMRange(Tok.getLoc(), Tok.getEndLoc()));
          Parser.Lex();
          return MatchOperand_ParseFail;
        }
      } else {
        if (RegNo % 4) {
          Error(Tok.getLoc(), "Quad of registers must start with register which number is a multiple of 4", SMRange(Tok.getLoc(), Tok.getEndLoc()));
          Parser.Lex();
          return MatchOperand_ParseFail;
        }
      }
    }
    return MatchOperand_NoMatch;
  }

  SMLoc StartLoc = Tok.getLoc();
  SMLoc EndLoc = Tok.getEndLoc();
  int RegClassNo = -1;
  for (int ClsId : { TPC::SRFRegClassID, TPC::SPRFRegClassID, TPC::MRFRegClassID,
                     TPC::VRFRegClassID, TPC::VPRFRegClassID, TPC::ADRFRegClassID, TPC::IRFRegClassID,
                     TPC::DRFRegClassID, TPC::ARFRegClassID, TPC::ZRFRegClassID,
                     // HW reg classes:
		     TPC::HWTnsrRegLdRegClassID, TPC::HWTnsrRegStRegClassID, TPC::HWOffsSizeRegLdRegClassID,
		     TPC::HWOffsSizeRegStRegClassID, TPC::HWRMWRegRegClassID, TPC::HWDivStepRegClassID,
		     TPC::HWSCarryRegClassID, TPC::HWVCarryRegClassID, TPC::HWZPRegRegClassID })
    if (MRI->getRegClass(ClsId).contains(RegNo)) {
      RegClassNo = ClsId;
      break;
    }
  Parser.Lex();
  Operands.push_back(TPCOperand::CreateReg(RegNo, RegClassNo, StartLoc, EndLoc));
  return MatchOperand_Success;
}


OperandMatchResultTy TPCAsmParser::parseNumber(OperandVector &Operands) {
  AsmToken Sign = Lexer.getTok();
  SMLoc StartLoc = Sign.getLoc();

  bool Negative = false;
  if (Sign.is(AsmToken::Minus)) {
    Negative = true;
    consumeToken();
  } else if(Sign.is(AsmToken::Plus)) {
    consumeToken();
  }

  AsmToken Number = Lexer.getTok();
  SMLoc EndLoc = Number.getEndLoc();
  if (Number.isNot(AsmToken::Integer) && Number.isNot(AsmToken::Real)) {
    if (Negative)
      Lexer.UnLex(Sign);
    return MatchOperand_NoMatch;
  }
  consumeToken();

  StringRef Digits = Number.getString();
  if (Number.is(AsmToken::Integer)) {
    long long Value;
    if (consumeSignedInteger(Digits, 0, Value)) {
      Error(StartLoc, "invalid number", SMRange(StartLoc, EndLoc));
      Operands.push_back(std::unique_ptr<TPCOperand>());
      return MatchOperand_ParseFail;
    } else {
      // Parse the suffix for integer type.
      // 'u' and 'U' supported
      const AsmToken &Suffix = Lexer.getTok();
      if (Suffix.is(AsmToken::Identifier) &&
          Suffix.getString().compare_lower("u") == 0 &&
          (Suffix.getLoc().getPointer() - EndLoc.getPointer()) == 0) {
        consumeToken();
        EndLoc = Suffix.getEndLoc();
        if(Negative){
          Error(Suffix.getLoc(), "the invalid suffix for a negative integer",
                SMRange(StartLoc, EndLoc));
          Operands.push_back(std::unique_ptr<TPCOperand>());
          return MatchOperand_ParseFail;
        }
      }
      Operands.push_back(TPCOperand::CreateImm(Negative ? -Value : Value,
                                                 StartLoc, EndLoc));
    }
  } else {
    // Parse the suffix for a floating type
    const AsmToken Suffix = Lexer.getTok();
    const fltSemantics *Semantics = &APFloat::IEEEsingle();
    if (Suffix.is(AsmToken::Identifier) &&
        Suffix.getLoc().getPointer() - EndLoc.getPointer() == 0) {
      EndLoc = Suffix.getEndLoc();
      consumeToken();
      const StringRef& SuffixStr = Suffix.getString();
      if(SuffixStr.compare_lower("f") == 0) {
      } else if(SuffixStr.compare_lower("h") == 0) {
        Semantics = &APFloat::IEEEhalf();
      } else if(SuffixStr.compare_lower("bf") == 0) {
        Semantics = &APFloat::BFloat();
      } else {
        Error(Suffix.getLoc(), "invalid suffix for real number",
              SMRange(StartLoc, EndLoc));
        Operands.push_back(std::unique_ptr<TPCOperand>());
        return MatchOperand_ParseFail;
      }
    }

    APFloat BinValue(APFloat::IEEEsingle());
    Expected<APFloat::opStatus> CvtStatus =
        BinValue.convertFromString(Digits, APFloat::rmNearestTiesToEven);
    if (!CvtStatus)
      Error(StartLoc, "Error on converting number from the string",
            SMRange(StartLoc, EndLoc));
    APFloat::opStatus Status = *CvtStatus;
    if (Status == APFloat::opOK || Status == APFloat::opInexact) {
      if (Negative)
        BinValue.changeSign();
      bool LoosesInfo;
      Status = BinValue.convert(*Semantics,
                                APFloat::rmNearestTiesToEven,
                                &LoosesInfo);
      if (Status == APFloat::opOK || Status == APFloat::opInexact)
        Operands.push_back(
          TPCOperand::CreateImm(BinValue.bitcastToAPInt().getLimitedValue(),
                                StartLoc, EndLoc));
    }

    if (Status & APFloat::opUnderflow) {
      Error(StartLoc, "Type underflow", SMRange(StartLoc, EndLoc));
      Operands.push_back(std::unique_ptr<TPCOperand>());
      return MatchOperand_ParseFail;
    } else if (Status & APFloat::opOverflow) {
      Error(StartLoc, "Type overflow", SMRange(StartLoc, EndLoc));
      Operands.push_back(std::unique_ptr<TPCOperand>());
      return MatchOperand_ParseFail;
    } else if (Status & APFloat::opInvalidOp) {
      assert(false);
    }
  }

  return MatchOperand_Success;
}


OperandMatchResultTy TPCAsmParser::parseImmediate(OperandVector &Operands) {
  const AsmToken &Tok = Lexer.getTok();
  SMLoc StartLoc = Tok.getLoc();
  SMLoc EndLoc = Tok.getEndLoc();

  if (Tok.is(AsmToken::Integer) || Tok.is(AsmToken::Real) ||
      Tok.is(AsmToken::Minus) || Tok.is(AsmToken::Plus))
    return parseNumber(Operands);

  if (Tok.isNot(AsmToken::Identifier))
    return MatchOperand_NoMatch;

  StringRef Identifier = Tok.getString();

  // u32_12, i32_-11, u16_, i16_, i8_, u8_, f32_, f16_
  bool IsUnsigned = Identifier.startswith_lower("u");
  bool IsSigned = Identifier.startswith_lower("i");
  bool IsFloat = Identifier.startswith_lower("f");
  bool IsBFloat16 = Identifier.startswith_lower("bf");
  if (!IsUnsigned && !IsSigned && !IsFloat && !IsBFloat16)
    return MatchOperand_NoMatch;

  StringRef Remainder = Identifier.drop_front(IsBFloat16 ? 2 : 1);
  unsigned BitWidth = 0;
  if (Remainder.startswith("32_")) {
    if (IsBFloat16)
      return MatchOperand_NoMatch;
    BitWidth = 32;
    Remainder = Remainder.drop_front(3);
  } else if (Remainder.startswith("16_")) {
    if (IsFloat)
      return MatchOperand_NoMatch;
    BitWidth = 16;
    Remainder = Remainder.drop_front(3);
  } else if (Remainder.startswith("8_")) {
    if (IsFloat || IsBFloat16)
      return MatchOperand_NoMatch;
    BitWidth = 8;
    Remainder = Remainder.drop_front(2);
  }
  if (!BitWidth)
    return MatchOperand_NoMatch;

  if (IsFloat || IsBFloat16) {
    // TODO: ammend identifier with fractional/exponential part.
    double Value;
    if (Remainder.getAsDouble(Value))
      return MatchOperand_NoMatch;
    consumeToken();   // No rollbacks anymore.
    const fltSemantics &FSem =
        (BitWidth == 32)
            ? APFloat::IEEEsingle()
        : (IsBFloat16 ? APFloat::BFloat() : APFloat::IEEEhalf());
    APFloat BinValue(Value);
    bool LoosesInfo;
    auto Status = BinValue.convert(FSem,
                                   APFloat::rmNearestTiesToEven, &LoosesInfo);
    if (Status != APFloat::opOK && Status != APFloat::opInexact) {
      Error(StartLoc, "invalid number", SMRange(StartLoc, EndLoc));
      Operands.push_back(std::unique_ptr<TPCOperand>());
      return MatchOperand_ParseFail;
    }
    APInt IntVal = BinValue.bitcastToAPInt();
    assert(IntVal.getBitWidth() == BitWidth);
    Operands.push_back(
      TPCOperand::CreateImm(IntVal.getLimitedValue(),
                            Tok.getLoc(), Tok.getEndLoc()));
    return MatchOperand_Success;
  }
  if (IsUnsigned) {
    // Unsigned constants may be specified by hexadecimal notation.
    unsigned Radix = 10;
    if (Remainder.front() == 'x' || Remainder.front() == 'X') {
      Radix = 16;
      Remainder = Remainder.drop_front(1);
    }
    unsigned Value;
    if (Remainder.consumeInteger(Radix, Value))
      return MatchOperand_NoMatch;
    consumeToken(); // No rollbacks anymore.
    Operands.push_back(
        TPCOperand::CreateImm(Value, Tok.getLoc(), Tok.getEndLoc()));
    return MatchOperand_Success;
  }

  // Signed number is allowed to have minus sign, like i32_-12. Such constants
  // are composed of several tokens.
  if (Remainder.empty()) {
    consumeToken(); // No rollbacks anymore.
    return parseNumber(Operands);
  }

  // Signed constants may be specified by hexadecimal notation, if they are not
  // started with minus.
  unsigned Radix = 10;
  if (Remainder.front() == 'x' || Remainder.front() == 'X') {
    Radix = 16;
    Remainder = Remainder.drop_front(1);
  }
  long long Value;
  if (consumeSignedInteger(Remainder, Radix, Value))
    return MatchOperand_NoMatch;
  consumeToken(); // No rollbacks anymore.

  Operands.push_back(TPCOperand::CreateImm(Value, Tok.getLoc(), Tok.getEndLoc()));
  return MatchOperand_Success;
}


OperandMatchResultTy TPCAsmParser::parseMemRI(OperandVector &Operands) {
  // TODO: check expression?
  OperandMatchResultTy Result = parseImmediate(Operands);
  if (Result != MatchOperand_Success)
    return Result;

  // TODO: to make this method a checker, we must implement token stream rollback.
  AsmToken CommaTok = Lexer.getTok();
  if (CommaTok.isNot(AsmToken::Comma))
    return MatchOperand_ParseFail;
  consumeToken();

  Result = parseRegister(Operands);
  if (Result != MatchOperand_Success)
    return MatchOperand_ParseFail;

  // Swap the last two operands.
  unsigned Size = Operands.size();
  assert(Size >= 2);
  auto RegOperand = std::move(Operands[Size - 1]);
  auto ImmOperand = std::move(Operands[Size - 2]);
  Operands.pop_back();
  Operands.pop_back();
  Operands.push_back(TPCOperand::CreateMemRI(
      static_cast<TPCOperand *>(RegOperand.get())->getReg(),
      static_cast<TPCOperand *>(ImmOperand.get())->getImm(),
      ImmOperand->getStartLoc(),
      RegOperand->getEndLoc()));

  return MatchOperand_Success;
}


TPCAsmParser::SwitchParseState TPCAsmParser::processSwitch(
    StringRef Switch, SlotParser Slot, StringRef Mnemonic, StringRef Value,
    OperandVector &Operands, SMLoc Start, SMLoc End) {
  // Skip dimmask operand. Although it has a record in switch database, that
  // record is only for disassembler.
  if (isDimMask(Switch))
    return SwitchParseState::Unknown;

  TPCII::OpType Type = TPCII::OpType::Invalid;
  if (Operands.size() > 1) {
    const TPCOperand *DT = static_cast<const TPCOperand *>(Operands[1].get());
    if (DT->isDataType())
      Type = DT->getDataType();
  }
  unsigned SwitchPos = (Type == TPCII::OpType::Invalid) ? 1 : 2;
  if (SwitchPos >= Operands.size() ||
      !static_cast<const TPCOperand *>(Operands[SwitchPos].get())->isSwitchSet())
    return SwitchParseState::Unknown;
  TPCOperand &SwitchSetOp = static_cast<TPCOperand &>(*Operands[SwitchPos]);
  unsigned ExistingSwitches = SwitchSetOp.getSwitchSet();
  std::vector<std::string> &SwitchNames = SwitchSetOp.getSwitchNames();

  bool IsUnknown;
  std::string Msg = TPCII::incorporateSwitch(Switch, Value, Mnemonic, Slot, Type,
                                             /*IsSuffix*/false, IsUnknown, ExistingSwitches,
                                             SwitchNames);
  if (!Msg.empty()) {
    Error(Start, Msg, SMRange(Start, End));
    return SwitchParseState::Error;
  }

  SwitchSetOp.setSwitchSet(ExistingSwitches);
  return SwitchParseState::Ok;
}


static StringRef getSwitchForIntegerOperand(OperandVector &Operands, StringRef Mnemonic) {
  if (Mnemonic.equals("popcnt") && Operands.size() == 3)  // text, datatype, switch
    return "set";
  if (Mnemonic.equals("find_first") && Operands.size() == 3)  // text, datatype, switch
    return "set";
  if (Mnemonic.equals("get_lut_entry_and_interval_start") && Operands.size() == 6)  // text, datatype, dest, src, shift, switch
    return "funcid";
  return StringRef();
}

OperandMatchResultTy TPCAsmParser::parseOperand(OperandVector &Operands, StringRef Mnemonic) {
  const AsmToken &Tok = Lexer.getTok();
  SMLoc StartLoc = Tok.getLoc();
  SMLoc EndLoc = Tok.getEndLoc();

  OperandMatchResultTy ResTy = MatchOperandParserImpl(Operands, Mnemonic);

  // If there wasn't a custom match, try the generic matcher below. Otherwise,
  // there was a match, but an error occurred, in which case, just return that
  // the operand parsing failed.
  if (ResTy == MatchOperand_Success || ResTy == MatchOperand_ParseFail)
    return ResTy;

  if(Tok.is(AsmToken::BigNum)) {
    Error(StartLoc, "Type overflow", SMRange(StartLoc, EndLoc));
    Operands.push_back(std::unique_ptr<TPCOperand>());
    return MatchOperand_ParseFail;
  }

  if (Tok.is(AsmToken::Integer) || Tok.is(AsmToken::Real)) {
    StringRef SwitchGroup = getSwitchForIntegerOperand(Operands, Mnemonic);
    if (!SwitchGroup.empty()) {
      StringRef Identifier = Tok.getString();
      switch (processSwitch(SwitchGroup, CurrentSlot, Mnemonic, Identifier, Operands, StartLoc, EndLoc)) {
      case SwitchParseState::Ok:
        consumeToken();
        return MatchOperand_Success;
      case SwitchParseState::Error:
        return MatchOperand_ParseFail;
      case SwitchParseState::Unknown:
        return MatchOperand_NoMatch;
      }
    }
  }

  ResTy = parseImmediate(Operands);
  if (ResTy == MatchOperand_Success || ResTy == MatchOperand_ParseFail)
    return ResTy;

  // !SP1
  if (Tok.is(AsmToken::Exclaim)) {
    consumeToken();
    unsigned RegNo;
    if (!ParseRegister(RegNo, StartLoc, EndLoc)) {
      PredicateKind PredKind = getPredicateKind(RegNo);
      if (getPredicateKind(RegNo) == NotAPredicate) {
        Error(StartLoc, "expected predicate register", SMRange(StartLoc, EndLoc));
        Operands.push_back(std::unique_ptr<TPCOperand>());
        return MatchOperand_ParseFail;
      }
      Operands.push_back(TPCOperand::CreatePredicate(RegNo, true, PredKind == VectorPredicate, StartLoc, EndLoc));
      return MatchOperand_Success;
    } else {
      Error(StartLoc, "expected predicate register", SMRange(StartLoc, EndLoc));
      Operands.push_back(std::unique_ptr<TPCOperand>());
      return MatchOperand_ParseFail;
    }
  }

  if (Tok.is(AsmToken::Identifier)) {
    StringRef Identifier = Tok.getString();
    StringRef SwitchValue;

    // Register.
    if (parseRegister(Operands) == MatchOperand_Success)
      return MatchOperand_Success;

    // If identifier is followed by '=' sign, it is a switch.
    AsmToken IdentifierTok = Tok;
    consumeToken();
    if (Tok.is(AsmToken::Equal)) {
      consumeToken();
      if (Tok.is(AsmToken::Identifier) || Tok.is(AsmToken::Integer)) {
        SwitchValue = Tok.getString();
        EndLoc = Tok.getEndLoc();
        consumeToken();
      } else {
        Error(StartLoc, "expected integer number or identifier", SMRange(Tok.getLoc(), Tok.getEndLoc()));
        Operands.push_back(std::unique_ptr<TPCOperand>());
        return MatchOperand_ParseFail;
      }
    }

    // Try parsing identifier as switch.
    switch (processSwitch(Identifier, CurrentSlot, Mnemonic, SwitchValue, Operands, StartLoc, EndLoc)) {
    case SwitchParseState::Ok:
      return MatchOperand_Success;
    case SwitchParseState::Error:
      Operands.push_back(std::unique_ptr<TPCOperand>());
      return MatchOperand_ParseFail;
    default:
      break;
    }
    if (!SwitchValue.empty()) {
      Error(StartLoc, "cannot parse as a switch", SMRange(StartLoc, EndLoc));
      Operands.push_back(std::unique_ptr<TPCOperand>());
      return MatchOperand_ParseFail;
    }

    Lexer.UnLex(IdentifierTok);

    // b11001
    if (isDimMask(Identifier)) {
      consumeToken();
      StringRef Remainder = Identifier.drop_front(1);
      if (Remainder.size() > 5) {
        Error(StartLoc, "too long dimmask", SMRange(StartLoc, EndLoc));
        Operands.push_back(std::unique_ptr<TPCOperand>());
        return MatchOperand_ParseFail;
      }
      if (Remainder.size() < 5) {
        Error(StartLoc, "too short dimmask", SMRange(StartLoc, EndLoc));
        Operands.push_back(std::unique_ptr<TPCOperand>());
        return MatchOperand_ParseFail;
      }
      unsigned Value;
      if (Remainder.consumeInteger(2, Value)) {
        Error(StartLoc, "invalid number", SMRange(StartLoc, EndLoc));
        Operands.push_back(std::unique_ptr<TPCOperand>());
        return MatchOperand_ParseFail;
      }
      Operands.push_back(TPCOperand::CreateDimMask(Value, StartLoc, EndLoc));
      return MatchOperand_Success;
    }

    // Generic integer expression.
    const MCExpr *Expr;
    SMLoc NewEndLoc;
    if (getParser().parseExpression(Expr, NewEndLoc))
      return MatchOperand_ParseFail;
    Operands.push_back(TPCOperand::CreateImm(Expr, StartLoc, NewEndLoc));
    return MatchOperand_Success;
  }

  // Immediate
  if (getLexer().is(AsmToken::Integer)) {
    const MCExpr *Val;
    if (!ParseImmediate(Val, StartLoc, EndLoc))
      Operands.push_back(TPCOperand::CreateImm(Val, StartLoc, EndLoc));
    return MatchOperand_Success;
  }

  Error(StartLoc, "unrecognized operand", SMRange(StartLoc, EndLoc));
  Operands.push_back(std::unique_ptr<TPCOperand>());
  return MatchOperand_ParseFail;
}


static TPCII::OpType parseDataTypeSuffix(StringRef Suffix) {
  return StringSwitch<TPCII::OpType>(Suffix)
    .CaseLower("f32", TPCII::OpType::FP32)
    .CaseLower("bf16", TPCII::OpType::BF16)
    .CaseLower("i32", TPCII::OpType::INT32)
    .CaseLower("u32", TPCII::OpType::UINT32)
    .CaseLower("i8", TPCII::OpType::INT8)
    .CaseLower("u8", TPCII::OpType::UINT8)
    .CaseLower("b", TPCII::OpType::BOOL)
    .CaseLower("i16", TPCII::OpType::INT16)
    .CaseLower("u16", TPCII::OpType::UINT16)
    .CaseLower("i4", TPCII::OpType::INT4)
    .CaseLower("u4", TPCII::OpType::UINT4)
    .CaseLower("f16", TPCII::OpType::FP16)
    .CaseLower("f8_143", TPCII::OpType::FP8_143)
    .CaseLower("f8_152", TPCII::OpType::FP8_152)
    .Default(TPCII::OpType::Invalid);
}


// Checks if the instruction represented by the given mnemonic needs datatype
// as an operand.
static bool needDataTypeAsOperand(StringRef Mnemonic, SlotParser Slot, const FeatureBitset& Features) {
  if (Mnemonic.startswith_lower("gen_addr"))
    return false;
  return true;
}


bool TPCAsmParser::parseAsSeparateSwitch(
    StringRef Switch, StringRef Mnemonic, SMRange Range,
    OperandVector &Operands, unsigned SwitchValue,
    TPCOperand *SpecialOperand) {
  TPCOperand *TempOperand = nullptr;

  bool Result = false;
  if (CurrentSlot == SlotParser::Scalar) {
    if (Switch.equals_lower("div_mod") &&
        (SwitchValue & TPCII::SW_DIV_MODE_BOTH)) {
      auto BothDivMode = TPCOperand::CreateBothDivMod(
        TPCII::SW_DIV_MODE_BOTH, Range.Start, Range.End);
      TempOperand = BothDivMode.get();
      Operands.insert(Operands.begin() + 3, std::move(BothDivMode));
      Result = true;
    }
  } else if (CurrentSlot == SlotParser::Vector) {
    if ((Mnemonic.equals_lower("mac") || Mnemonic.equals_lower("add") ||
         Mnemonic.equals_lower("sub") || Mnemonic.equals_lower("madd") ||
         Mnemonic.equals_lower("mul")) &&
        (Switch.equals_lower("acc_fp32") || Switch.equals_lower("acc_i16"))) {
      auto Accumulator = TPCOperand::CreateAccumulator(Range.Start, Range.End);
      TempOperand = Accumulator.get();
      Operands.insert(Operands.begin() + 3, std::move(Accumulator));
      Result = true;
    } else if (Switch.equals_lower("rhaz_rs")) {
      auto Rhaz = TPCOperand::CreateRhazRs(1, Range.Start, Range.End);
      TempOperand = Rhaz.get();
      Operands.insert(Operands.begin() + 3, std::move(Rhaz));
      Result = true;
    } else if (Switch.equals_lower("x2") && (
                 Mnemonic.equals_lower("add") ||
                 Mnemonic.equals_lower("sub") ||
                 Mnemonic.equals_lower("madd"))) {
      auto X2 = TPCOperand::CreateX2(TPCII::SW_X2_ARITHMETIC,
                                     Range.Start, Range.End);
      TempOperand = X2.get();
      Operands.insert(Operands.begin() + 3, std::move(X2));
      Result = true;
    } else if (((Switch.equals_lower("mdg_type") &&
                 (SwitchValue & TPCII::SW_MDG_TYPE_MASK) ==
                 TPCII::SW_MDG_TYPE_ALL) ||
                Switch.equals_lower("all")) &&
               Mnemonic.equals_lower("mov_dg")) {
      auto MovDGAll = TPCOperand::CreateMovDGAll(TPCII::SW_MDG_TYPE_ALL,
                                                 Range.Start, Range.End);
      TempOperand = MovDGAll.get();
      Operands.insert(Operands.begin() + 1, std::move(MovDGAll));
    } else if (((Switch.equals_lower("mdg_type") &&
                 (SwitchValue & TPCII::SW_MDG_TYPE_MASK) ==
                 TPCII::SW_MDG_TYPE_PACK) ||
                Switch.equals_lower("pack")) &&
               Mnemonic.equals_lower("mov_dg")){
      auto MovDGPack = TPCOperand::CreateMovDGPack(TPCII::SW_MDG_TYPE_PACK,
                                                 Range.Start, Range.End);
      TempOperand = MovDGPack.get();
      Operands.insert(Operands.begin() + 1, std::move(MovDGPack));
    } else if (Switch.equals_lower("mdg_type") &&
               (SwitchValue & TPCII::SW_MDG_TYPE_MASK) ==
                 TPCII::SW_MDG_TYPE_UNPACK &&
               Mnemonic.equals_lower("mov_dg")) {
      auto MovDGUnpack = TPCOperand::CreateMovDGUnpack(
            TPCII::SW_MDG_TYPE_UNPACK, Range.Start, Range.End);
      TempOperand = MovDGUnpack.get();
      Operands.insert(Operands.begin() + 1, std::move(MovDGUnpack));
    }
  }

  if (SpecialOperand && TempOperand) {
    Error(Range.Start, "the switch conflicts with another", Range);
    Error(SpecialOperand->getStartLoc(), "conflicting switch is here",
          SpecialOperand->getLocRange());
    return false;
  }

  SpecialOperand = TempOperand;
  return Result;
}


static bool shallSuffixBeAPartOfMnemonic(StringRef Mnemonic, StringRef Suffix,
                                         SlotParser Slot, bool &NeedAsSwitch) {
  // TODO: Initially Slot is Unknown.
  if (Mnemonic.equals_lower("mov") /*&& Slot == SlotParser::Load*/)
    return Suffix.equals_lower("from_hw_reg") || Suffix.equals_lower("to_hw_reg");

  if (Mnemonic.equals_lower("convert") && Suffix.equals_lower("i64"))
    return true;

  if (StringSwitch<bool>(Suffix)
      // MSVC has an issue with compilation of CasesLower.
      .CasesLower("i8", "i16", "i32", true)
      .CasesLower("u8", "u16", "u32", true)
      .CasesLower("f32", "bf16", "f16", true)
      .CasesLower("f8_143", "f8_152", true)
      .Default(false)) {
    if (Mnemonic.equals_lower("mac") && Slot == SlotParser::Vector) {
      NeedAsSwitch = true;
      return true;
    }
    if (Mnemonic.equals_lower("madd") && Slot == SlotParser::Vector) {
      NeedAsSwitch = true;
      return true;
    }
    if (Mnemonic.equals_lower("mul") && Slot == SlotParser::Vector) {
      NeedAsSwitch = true;
      return true;
    }
  }

  return false;
}


static bool isSoloInstruction(StringRef Mnemonic) {
  return StringSwitch<bool>(Mnemonic)
    .Case("loop", true)
    .Default(false);
}

bool TPCAsmParser::ParseInstruction(ParseInstructionInfo &Info,
                                    StringRef Mnemonic, SMLoc MnemonicLoc,
                                    OperandVector &Operands) {
  if (Mnemonic.equals("{") || Mnemonic.equals("}")) {
    NewAsmFormat = !Mnemonic.equals("}");
    return true;
  }

  MCAsmParser &Parser = getParser();
  MCAsmLexer &Lexer = getLexer();

  // If the current slot is undefined, check if we are parsing a solo
  // instruction. If not, change slot from Unknown to Load.
  if (CurrentSlot == SlotParser::Unknown) {
    if (!isSoloInstruction(Mnemonic))
      initBundle(MnemonicLoc);
    else
      CurrentSlot = SlotParser::Special;
  }

  // If we need to make a message referring to the instruction mnemonic or a
  // part of it, 'Mnemonic' cannot be used, because it is allocated aside of
  // input buffer and SMLoc does not point to it. So recreate the real name,
  // which resides in the input buffer. This name however is not canonicalized
  // to lower case.
  StringRef FullName = StringRef(MnemonicLoc.getPointer(), Mnemonic.size());

  // Many instructions may have suffixes in their opcode, like 'ADD.I32.ST.CARRY'.
  // In this case we split such opcode into real opcode and operand(s) that
  // represent these suffixes.
  SmallVector<StringRef, 4> Suffixes;
  StringRef                 BareMnemonic;   // Mnemonic without suffixes.
  if (FullName.contains('.')) {
    FullName.split(Suffixes, '.');
    BareMnemonic = Mnemonic.take_front(Suffixes.front().size());
    Suffixes.erase(Suffixes.begin());
  } else {
    BareMnemonic = Mnemonic;
  }

  // If data type suffix (like '.I32') presents, it must be the first one.
  TPCII::OpType DataType = TPCII::OpType::Invalid;
  SMLoc DataTypeStartLoc;
  SMLoc DataTypeEndLoc;
  StringRef DataTypeSuffix;
  SMRange DataTypeRange;
  for (unsigned I = 0, E = Suffixes.size(); I < E; ++I) {
    StringRef Suffix = Suffixes[I];
    SMLoc SuffixStartLoc = SMLoc::getFromPointer(Suffix.begin());
    SMLoc SuffixEndLoc = SMLoc::getFromPointer(Suffix.end());
    TPCII::OpType DT = parseDataTypeSuffix(Suffix);
    if (DT != TPCII::OpType::Invalid) {
      if (I != 0) {
        SMRange SuffixRange(SuffixStartLoc, SuffixEndLoc);
        Error(SuffixStartLoc, "Data type must be specified first", SuffixRange);
        return true;
      }
      DataType = DT;
      DataTypeSuffix = Suffix;
      DataTypeRange = SMRange(SuffixStartLoc, SuffixEndLoc);
    }
  }

  // The first suffix may be a part of mnemonic.
  if (!Suffixes.empty()) {
    bool NeedAsSwitch = false;
    StringRef FirstSuffix = Suffixes.front();
    if (shallSuffixBeAPartOfMnemonic(BareMnemonic, FirstSuffix, CurrentSlot, NeedAsSwitch)) {
      Mnemonic = Mnemonic.take_front(BareMnemonic.size() + FirstSuffix.size() + 1);
      if (!NeedAsSwitch) {
        Suffixes.erase(Suffixes.begin());
      }
    } else {
      Mnemonic = BareMnemonic;
    }
  }

  // Make datatype argument. Data type may be represented by a separate argument
  // of type 'DataType', be incorporated into instruction mnemonic or treated as
  // switch set.
  std::unique_ptr<TPCOperand> DataTypeOp;
  bool DataTypeAsSwitch = false;
  if (!Suffixes.empty()) {
    if (DataType != TPCII::OpType::Invalid) {
      if (needDataTypeAsOperand(Mnemonic, CurrentSlot, getSTI().getFeatureBits()))
        DataTypeOp = TPCOperand::CreateDataType(DataType, DataTypeStartLoc, DataTypeEndLoc);
      else
        DataTypeAsSwitch = true;
      Suffixes.erase(Suffixes.begin());
    }
  }

  // Split bare mnemonic and switch for same instructions
  if (Mnemonic.equals_lower("mov_group"))
    Mnemonic = BareMnemonic = "mov_g";
  else if (Mnemonic.startswith_lower("mov_dual_group"))
    Mnemonic = BareMnemonic = "mov_dg";
  else if (Mnemonic.startswith_lower("mov_dg"))
    Mnemonic = Mnemonic.substr(0, 6);

  // The first operand is the instruction name.
  Operands.push_back(TPCOperand::CreateToken(Mnemonic, MnemonicLoc));

  // The second operand is DataType operand if it presents.
  if (DataTypeOp)
    Operands.push_back(std::move(DataTypeOp));

  // If the instruction can have switches, let it always have switch operand,
  // even if it is zero. It follows datatype operand if the latter presents.
  unsigned SwitchSetPos = 0;
  TPCOperand *Sw = nullptr;
  unsigned AllSwFlags = 0;
  if (TPCII::doesInstructionHasASwitch(BareMnemonic, CurrentSlot)) {
    SMLoc SwitchSetStartLoc;
    SMLoc SwitchSetEndLoc;
    if (!Suffixes.empty()) {
      SwitchSetStartLoc = SMLoc::getFromPointer(Suffixes.front().begin());
      SwitchSetEndLoc = SMLoc::getFromPointer(Suffixes.back().end());
    } else {
      SwitchSetStartLoc = MnemonicLoc;
      SwitchSetEndLoc = MnemonicLoc;
    }
    SwitchSetPos = Operands.size();
    Operands.push_back(TPCOperand::CreateSwitchSet(SwitchSetStartLoc,
                                                   SwitchSetEndLoc));
    Sw = static_cast<TPCOperand *>(Operands.back().get());
  }

  // In non arithmetic slots the data type, if it does not become an operand, is
  // turned into switch.
  if (DataTypeAsSwitch && SwitchSetPos)
    if (CurrentSlot == SlotParser::Load || CurrentSlot == SlotParser::Store) {
      auto &Sw = *static_cast<TPCOperand *>(Operands.back().get());
      bool IsUnknown;
      std::vector<std::string> SwNames = Sw.getSwitchNames();
      std::string Msg = TPCII::incorporateSwitch(DataTypeSuffix, "", BareMnemonic,
          CurrentSlot, DataType, /*IsSuffix*/true, IsUnknown, AllSwFlags, SwNames);
      if (!Msg.empty()) {
        Error(DataTypeRange.Start, Msg, DataTypeRange);
        return true;
      }
      Sw.setSwitchSet(AllSwFlags);
    }

  // Other suffixes may be in any order, they have integer values corresponding
  // to the bits in switch field.
  for (auto &Suffix : Suffixes) {
    Lexer.UnLex(AsmToken(AsmToken::Identifier, Suffix));
  }

  // Instruction is followed by list of operands, which is limited by
  // end-of-statement token (semicolon, line feed or end-of line).
  // The operand list may start with arguments separated by spaces (like
  // 'ADD.I32 b11011 I1, I2, I3'), which can be followed by comma-separated
  // operands.
  bool CommaSeen = false;
  if (Lexer.isNot(AsmToken::EndOfStatement) && Lexer.isNot(AsmToken::Eof)) {
    // Read the operands.
    while (true) {
      if (parseOperand(Operands, BareMnemonic) != MatchOperand_Success)
        return true;
      TPCOperand &LastOp = static_cast<TPCOperand &>(*Operands.back());
      // Check for comma and eat it.
      if (Lexer.is(AsmToken::Comma)) {
        Parser.Lex();
        //Processing switch set separated by comma
        if (!LastOp.isSwitchSet()){
          CommaSeen = true;
        }
      } else if (CommaSeen || Lexer.is(AsmToken::EndOfStatement)) {
        break;
      } else {
        // The last operand is not separated by a comma from the next operand.
        // It is allowed only for limited kinds of operands.
        if (!LastOp.isDimMask() &&        // ADD.I32 b11011 I5, I4, I2
            !(LastOp.isReg() &&           // MUL.I32 M1 I5, I4, I2
              (LastOp.getRegClass() == TPC::MRFRegClassID ||
               LastOp.getRegClass() == TPC::HWTnsrRegLdRegClassID ||
               LastOp.getRegClass() == TPC::HWTnsrRegStRegClassID ||
               LastOp.getRegClass() == TPC::HWOffsSizeRegLdRegClassID ||
               LastOp.getRegClass() == TPC::HWOffsSizeRegStRegClassID ||
               LastOp.getRegClass() == TPC::HWRMWRegRegClassID ||
               LastOp.getRegClass() == TPC::HWDivStepRegClassID ||
               LastOp.getRegClass() == TPC::HWSCarryRegClassID ||
               LastOp.getRegClass() == TPC::HWVCarryRegClassID ||
               LastOp.getRegClass() == TPC::HWZPRegRegClassID )) &&
            !LastOp.isRegSelTo() &&
            !LastOp.isAccumulator() &&
            !LastOp.isImm() &&            // MOV_IRF_DIM 0 S2, I4
            !LastOp.isSwitchSet())
          break;
      }
    }

    if (Lexer.isNot(AsmToken::EndOfStatement))
      return TokError("unexpected token in argument list");
  }

  const MCParsedAsmOperand *LabelOperand = nullptr;
  if (Mnemonic.compare_lower("jmpr") == 0) {
    assert(Operands.size() >= 2);
    if (Operands[1]->isImm()) {
      LabelOperand = Operands[1].get();
      LLVM_DEBUG(dbgs() << "JMP instuction with label:" <<
                 getStringFromLoc(LabelOperand->getStartLoc(),
                                  LabelOperand->getEndLoc()) << " detected\n");
    }
  } else if (Mnemonic.compare_lower("loop") == 0) {
     assert(Operands.size() >= 6);
     LabelOperand = Operands[5].get();
     LLVM_DEBUG(dbgs() << "LOOP instuction with label: " <<
                getStringFromLoc(LabelOperand->getStartLoc(),
                                 LabelOperand->getEndLoc()) << " detected\n");
  }
  if (LabelOperand)
    OperatorLabels.insert(getStringFromLoc(LabelOperand->getStartLoc(),
                                           LabelOperand->getEndLoc()));

  // Add AUTO_INC for ld_g for goya2
  if (BareMnemonic.equals_lower("ld_g") &&
      getSTI().getFeatureBits()[TPC::FeatureGreco]){
    TPCOperand &SwitchSetOp = static_cast<TPCOperand &>(*Operands[SwitchSetPos]);
    unsigned SwitchValue = SwitchSetOp.getSwitchSet();
    if (SwitchValue & TPCII::SW_INC_VAL_G3)
      SwitchValue = SwitchValue | TPCII::SW_AUTO_INC_G3;

    SwitchSetOp.setSwitchSet(SwitchValue);
  }

  // If the instruction has a switchset operand, set proper default values
  // switches that were not specified and have non-zero default value.
  if (SwitchSetPos != 0) {
    TPCOperand &SwitchSetOp = static_cast<TPCOperand &>(*Operands[SwitchSetPos]);
    unsigned CurrentSwitchSet = SwitchSetOp.getSwitchSet();
    const std::vector<std::string> &SwNames = SwitchSetOp.getSwitchNames();
    if (TPCII::getDefaultSwitches(Mnemonic, CurrentSlot, DataType,
                                  CurrentSwitchSet, SwNames))
      SwitchSetOp.setSwitchSet(CurrentSwitchSet);

    TPCOperand *SpecialOperand = nullptr;
    for (const std::string &Switch : SwNames){
      parseAsSeparateSwitch(Switch, BareMnemonic, SwitchSetOp.getLocRange(),
                            Operands, CurrentSwitchSet, SpecialOperand);
    }
  }

  auto IsX2Switch = [&Operands]() -> bool {
    const TPCOperand &SwitchOp =
      static_cast<const TPCOperand &>(*Operands[2]);
    assert(SwitchOp.isSwitchSet() && "Must be SwitchSet");
    return SwitchOp.isSwitchSet() &&
      (SwitchOp.getSwitchSet() & TPCII::SW_X2_ARITHMETIC);
  };
  auto IsFP32Type = [&Operands]() -> bool {
    const TPCOperand &DataTypeOp =
      static_cast<const TPCOperand &>(*Operands[1]);
    assert(DataTypeOp.isDataType() && "Must be DataType");
    return DataTypeOp.isDataType() &&
      (DataTypeOp.getDataType() == TPCII::OpType::FP32);
  };
  // Check if arguments SRF of Imm
  auto IsBothImm = [](const TPCOperand &Op1,
                      const TPCOperand &Op2) -> bool {
    return Op1.isImm() && Op2.isImm();
  };
  auto IsBothSrf = [](const TPCOperand &Op1,
                      const TPCOperand &Op2) -> bool {
    return Op1.isReg() && Op2.isReg() &&
        Op1.getRegClass() == TPC::SRFRegClassID &&
        Op2.getRegClass() == TPC::SRFRegClassID;
  };
  // Check instruction with two immediate
  if (BareMnemonic.compare_lower("add") == 0 ||
      BareMnemonic.compare_lower("sub") == 0) {
    // Skip if not x2 and if not FP32
    if (IsX2Switch() && IsFP32Type()) {
      const TPCOperand &SrcBOp =
          static_cast<const TPCOperand &>(*Operands[5]);
      const TPCOperand &SrcDOp =
          static_cast<const TPCOperand &>(*Operands[6]);
      if ((IsBothImm(SrcBOp, SrcDOp) && (SrcBOp.getImm() != SrcDOp.getImm())) ||
          (IsBothSrf(SrcBOp, SrcDOp) && (SrcBOp.getReg() != SrcDOp.getReg()))) {
        Error(SrcDOp.getStartLoc(),
              "The second and the third sources must be equal in the case both are SRFs or immediates",
              SrcDOp.getLocRange());
        return true;
      }
    }
  } else if (BareMnemonic.compare_lower("mac") == 0 ||
             BareMnemonic.compare_lower("mul") == 0) {
    // Skip if not x2 and if not FP32
    if (IsX2Switch() && IsFP32Type()) {
      const TPCOperand &SrcBOp =
          static_cast<const TPCOperand &>(*Operands[5]);
      const TPCOperand &SrcDOp =
          static_cast<const TPCOperand &>(*Operands[6]);
      if ((IsBothImm(SrcBOp, SrcDOp) && (SrcBOp.getImm() != SrcDOp.getImm())) ||
          (IsBothSrf(SrcBOp, SrcDOp) && (SrcBOp.getReg() != SrcDOp.getReg()))) {
        Error(SrcDOp.getStartLoc(),
              "The second and the fourth sources must be equal in the case both are SRFs or immediates",
              SrcDOp.getLocRange());
        return true;
      }
    }
  }

  LLVM_DEBUG({
    dbgs() << "Operands: ";
    for (unsigned I = 0; I < Operands.size(); ++I)
      Operands[I]->dump();
    dbgs() << '\n';
  });

  // To properly parse instruction we need to recognize end-of-line, which
  // ends the current bundle.
  StringRef Delimiter;
  IsLastInstrInBundle = Lexer.is(AsmToken::Eof);
  if (!IsLastInstrInBundle) {
    Delimiter = Lexer.getTok().getString();
    // Recognize sequences of end-of-statement. Trailing comment is recognized
    // as EndOfStatement.
    while (Delimiter.equals(";") || NewAsmFormat) {
      Parser.Lex();
      IsLastInstrInBundle = Lexer.is(AsmToken::Eof);
      if (!Lexer.is(AsmToken::EndOfStatement) || IsLastInstrInBundle || NewAsmFormat)
        return false;
      Delimiter = Lexer.getTok().getString();
    }
    if (Delimiter.equals("\r\n"))
      Delimiter = Delimiter.drop_front();
    IsLastInstrInBundle = Delimiter.equals("\n") ||
        (Delimiter.size() > 1 && Delimiter.substr(0, 2) == "//");

    // If we're missing a newline at EOF, make sure we still get an
    // EndOfStatement token before the Eof token.
    if (Lexer.is(AsmToken::EndOfStatement)) {
      AsmToken Current = Lexer.getTok();
      Lexer.Lex();
      if (Lexer.isNot(AsmToken::Eof)) {
        Lexer.UnLex(Current);
      } else {
        IsLastInstrInBundle = true;
        return false;
      }
    }
  }

  return false;
}


bool TPCAsmParser::ParseDirective(AsmToken DirectiveID) {
  AsmToken SectionNameToken =
      DirectiveID.getString().equals(".section") ?
        getTok() : DirectiveID;
  const char *MultipleDefErrorMessage = "Multi definition {0} is denied";
  bool ParseResult = true;
  if (SectionNameToken.getString()
      .equals_lower(BinaryTPCMetadataSectionName)) {
    // Check TPC header exists
    if (TPCHeader) {
      Error(DirectiveID.getLoc(),
            formatv(MultipleDefErrorMessage,
                    BinaryTPCMetadataSectionName),
            DirectiveID.getLocRange());
      return false;
    }

    Lexer.Lex();
    ParseResult = parseTPCMetadata();
  } else if (SectionNameToken.getString().
             equals_lower(IndexMapSectionName)) {
    if (IndexMap) {
      Error(DirectiveID.getLoc(),
            formatv(MultipleDefErrorMessage,
                    IndexMapSectionName),
            DirectiveID.getLocRange());
      return false;
    }

    Lexer.Lex();
    ParseResult = parseIndexMap();
  }

  if (!ParseResult)
    Error(SectionNameToken.getLoc(),
          formatv("Error occured during parse section {0}",
                  SectionNameToken.getString()),
          SectionNameToken.getLocRange());

  return ParseResult;
}


void TPCAsmParser::eatToEndOfLine() {
  while (Lexer.isNot(AsmToken::Eof)) {
    if (Lexer.is(AsmToken::EndOfStatement)) {
      StringRef Delimiter = Lexer.getTok().getString();
      if (Delimiter.equals("\r\n"))
        Delimiter = Delimiter.drop_front();
      if (Delimiter == "\n" || (Delimiter.size() > 1 && Delimiter.substr(0, 2) == "//"))
        break;
    }
    Lexer.Lex();
  }

  // Eat EOL.
  if (Lexer.is(AsmToken::EndOfStatement))
    Lexer.Lex();
}


MCInst *TPCAsmParser::getNOP(SlotParser Slot, SMLoc Loc) {
  BundledSubInstructions.emplace_back();
  MCInst &Result = BundledSubInstructions.back();
  Result.setLoc(Loc);
  switch (Slot) {
  case SlotParser::Load:
    Result.setOpcode(TPC::NOPld);
    break;
  case SlotParser::Scalar:
    Result.setOpcode(TPC::NOPs);
    break;
  case SlotParser::Vector:
    Result.setOpcode(TPC::NOPv);
    break;
  case SlotParser::Store:
    Result.setOpcode(TPC::NOPst);
    break;
  default:
    llvm_unreachable("Invalid slot");
  }
  return &Result;
}


MCInst *TPCAsmParser::getHALT(SlotParser Slot, SMLoc Loc) {
  BundledSubInstructions.emplace_back();
  MCInst &Result = BundledSubInstructions.back();
  Result.setLoc(Loc);
  switch (Slot) {
  case SlotParser::Scalar:
    Result.setOpcode(TPC::HALTs);
    break;
  case SlotParser::Vector:
    Result.setOpcode(TPC::HALTv);
    break;
  default:
    llvm_unreachable("Invalid slot");
  }
  return &Result;
}


// Check if the instruction allows a predicate register as the last
// non-predicate operand.
bool TPCAsmParser::canLastArgBeAPredicate(StringRef Mnemonic,
                                          const OperandVector &Operands,
                                          bool AlreadyParsed) {
  unsigned NumNonPredOperand = ~0U;
  switch (CurrentSlot) {
  case SlotParser::Load:
    NumNonPredOperand = StringSwitch<unsigned>(Mnemonic)
      .Case("mov", 2)       // MOV SP1, SP2
      .Case("mov.from_hw_reg", 2)       // MOV.HW_REG 0, SP2
      .Case("mov.to_hw_reg", 2)       // MOV.HW_REG 0, SP2
      .Default(~0U);
    break;
  case SlotParser::Scalar:
    NumNonPredOperand = StringSwitch<unsigned>(Mnemonic)
      .Case("mov", 2)       // MOV.I32 SP1, SP2
      .Case("mov.from_hw_reg", 2)       // MOV.HW_REG 0, SP2
      .Case("mov.to_hw_reg", 2)       // MOV.HW_REG 0, SP2
      .Case("not", 2)
      .Case("and", 3)
      .Case("or", 3)
      .Case("xor", 3)
      .Default(~0U);
    break;
  case SlotParser::Vector:
    NumNonPredOperand = StringSwitch<unsigned>(Mnemonic)
      .Case("mov", 2)       // MOV.i32 VP1, VP2
      .Case("mov.from_hw_reg", 2)       // MOV.HW_REG 0, SP2
      .Case("mov.to_hw_reg", 2)       // MOV.HW_REG 0, SP2
      .Case("not", 2)
      .Case("and", 3)
      .Case("or", 3)
      .Case("xor", 3)
      .Default(~0U);
    break;
  case SlotParser::Store:
    if (Mnemonic.startswith_lower("st_l_v")) {
      if(getSTI().getFeatureBits()[TPC::FeatureAddr2]) {
        auto &MayBeMemRIOp = static_cast<TPCOperand &>(*Operands[2].get());
        NumNonPredOperand = MayBeMemRIOp.isMemRI() ? 2U   // STL_L_V 100, S2, VP1
                                                   : 3U;  // STL_L_V S3, S2, VP1
      } else {
        NumNonPredOperand = 2;  // ST_L_V S1, VP1
      }
    } else if (Mnemonic.startswith_lower("st_tnsr")) {
      if (Mnemonic.startswith_lower("st_tnsr_sqz")) {
        NumNonPredOperand = ~0U; // st_tnsr_sqz can not use predicates as use regiters
      } else {
        NumNonPredOperand = 3;    // ST_TNSR 1, I3, VP4
      }
    } else {
      NumNonPredOperand = StringSwitch<unsigned>(Mnemonic)
        .Case("st_g", 2)          // ST_G AD1, SP1
        .Case("st_l", 2)          // ST_L S1, SP1
        .Default(~0U);
    }
    break;
  default:
    return true;
  }
  if (NumNonPredOperand == ~0U)
    return true;

  // Skip leading pseudo operands (DataType, Switches).
  unsigned Shift = 0;
  for (unsigned I = 1, E = Operands.size(); I < E; ++I)
    if (static_cast<TPCOperand&>(*Operands[I]).isDataType() ||
        static_cast<TPCOperand&>(*Operands[I]).isSwitchSet())
      ++Shift;
    else
      break;
  assert(Shift <= 2);

  return Operands.size() - (AlreadyParsed ? 1 : 2) > NumNonPredOperand + Shift;
}


unsigned TPCAsmParser::convertToLongReg(unsigned RegNo, LongRegisterKind Kind) {
  struct TransPair {
    unsigned Src;
    unsigned Dst;
  };

  static const TransPair TableARF[] = {
    { TPC::V0, TPC::A0 },
    { TPC::V4, TPC::A4 },
    { TPC::V8, TPC::A8 },
    { TPC::V12, TPC::A12 },
    { TPC::V16, TPC::A16 },
    { TPC::V20, TPC::A20 },
    { TPC::V24, TPC::A24 },
    { TPC::V28, TPC::A28 },
    { TPC::V32, TPC::A32 },
    { TPC::V36, TPC::A36 },
    { 0, 0 }
  };
  static const TransPair TableDRF[] = {
    { TPC::V0, TPC::D0 },
    { TPC::V2, TPC::D2 },
    { TPC::V4, TPC::D4 },
    { TPC::V6, TPC::D6 },
    { TPC::V8, TPC::D8 },
    { TPC::V10, TPC::D10 },
    { TPC::V12, TPC::D12 },
    { TPC::V14, TPC::D14 },
    { TPC::V16, TPC::D16 },
    { TPC::V18, TPC::D18 },
    { TPC::V20, TPC::D20 },
    { TPC::V22, TPC::D22 },
    { TPC::V24, TPC::D24 },
    { TPC::V26, TPC::D26 },
    { TPC::V28, TPC::D28 },
    { TPC::V30, TPC::D30 },
    { TPC::V32, TPC::D32 },
    { TPC::V34, TPC::D34 },
    { TPC::V36, TPC::D36 },
    { TPC::V38, TPC::D38 },
    { 0, 0 }
  };
  static const TransPair TableZRF[] = {
    { TPC::S0, TPC::Z0 },
    { TPC::S2, TPC::Z2 },
    { TPC::S4, TPC::Z4 },
    { TPC::S6, TPC::Z6 },
    { TPC::S8, TPC::Z8 },
    { TPC::S10, TPC::Z10 },
    { TPC::S12, TPC::Z12 },
    { TPC::S14, TPC::Z14 },
    { TPC::S16, TPC::Z16 },
    { TPC::S18, TPC::Z18 },
    { TPC::S20, TPC::Z20 },
    { TPC::S22, TPC::Z22 },
    { TPC::S24, TPC::Z24 },
    { TPC::S26, TPC::Z26 },
    { TPC::S28, TPC::Z28 },
    { TPC::S30, TPC::Z30 },
    { TPC::S32, TPC::Z32 },
    { TPC::S34, TPC::Z34 },
    { 0, 0 }
  };

  const TransPair *TablePtr;
  switch (Kind) {
  case LongRegisterKind::ARF:
    TablePtr = TableARF;
    break;
  case LongRegisterKind::DRF:
    TablePtr = TableDRF;
    break;
  case LongRegisterKind::ZRF:
    TablePtr = TableZRF;
    break;
  }

  for (; TablePtr->Src; ++TablePtr) {
    if (TablePtr->Src == RegNo)
      return TablePtr->Dst;
  }
  return 0;
}


bool TPCAsmParser::replaceRegisterWithLong(OperandVector &Operands,
                                           unsigned RegOpNum,
                                           LongRegisterKind Kind) {
  auto *LongRegOp = static_cast<TPCOperand *>(Operands[RegOpNum].get());
  unsigned OldReg = LongRegOp->getReg();

  // Already long register.
  if (Kind == LongRegisterKind::ARF &&
      MRI->getRegClass(TPC::ARFRegClassID).contains(OldReg))
    return true;
  if (Kind == LongRegisterKind::DRF &&
      MRI->getRegClass(TPC::DRFRegClassID).contains(OldReg))
    return true;
  if (Kind == LongRegisterKind::ZRF &&
      MRI->getRegClass(TPC::ZRFRegClassID).contains(OldReg))
    return true;

  // Scalar variant.
  if (Kind == LongRegisterKind::ARF || Kind == LongRegisterKind::DRF)
    if (MRI->getRegClass(TPC::SRFRegClassID).contains(OldReg))
      return true;

  unsigned NewReg = convertToLongReg(OldReg, Kind);
  static const int RegClass[3] = { TPC::ARFRegClassID,
                                   TPC::DRFRegClassID,
                                   TPC::ZRFRegClassID };
  if (NewReg == 0) {
    // Invalid register.
    static const StringRef RegName[3] = { "ARF", "DRF", "ZRF" };
    static const StringRef RegAlign[3] = { "4", "2", "2" };
    Error(LongRegOp->getStartLoc(),
          "Register '" + StringRef(TPCInstPrinter::getRegisterName(OldReg)) +
          "' Cannot be a start of " + RegName[static_cast<unsigned>(Kind)] +
          " register, the number must be a multiple of " + RegAlign[static_cast<unsigned>(Kind)],
          LongRegOp->getLocRange());
    return false;
  }

  auto NewRegOp = TPCOperand::CreateReg(NewReg, RegClass[static_cast<unsigned>(Kind)],
                                        LongRegOp->getStartLoc(), LongRegOp->getEndLoc());
  Operands[RegOpNum] = std::move(NewRegOp);
  return true;
}


bool TPCAsmParser::adjustLongRegister(OperandVector &Operands) {
  TPCOperand &Op = static_cast<TPCOperand &>(*Operands[0]);
  assert(Op.isToken() && "Leading operand should always be a mnemonic!");
  StringRef Mnemonic = Op.getToken();

  TPCOperand *LongRegOp = nullptr;
  unsigned DestRegOpNum = 1;
  unsigned AccRegNo = 0;
  LongRegisterKind Kind;

  if (Mnemonic.startswith_lower("mac") || Mnemonic.startswith_lower("mul")) {
    if (Operands.size() < 4)
      return true;

    TPCOperand &DataTypeOp = static_cast<TPCOperand &>(*Operands[1]);
    TPCOperand &SwitchOp = static_cast<TPCOperand &>(*Operands[2]);
    TPCII::OpType DataType = DataTypeOp.getDataType();
    unsigned Switches = SwitchOp.getSwitchSet();
    bool HasAccFlag = false;

    // In MAC and MUL instructions, the destination register comes after datatype,
    // switch set and accumulator type:
    DestRegOpNum = 3;
    LongRegOp = static_cast<TPCOperand*>(Operands[DestRegOpNum].get());
    if (LongRegOp->isAccumulator()) {
      HasAccFlag = true;
      LongRegOp = static_cast<TPCOperand*>(Operands[++DestRegOpNum].get());
      AccRegNo = LongRegOp->getReg();
      if (MRI->getRegClass(TPC::HWZPRegRegClassID).contains(AccRegNo)) {
        LongRegOp = static_cast<TPCOperand *>(Operands[++DestRegOpNum].get());
      }
    } else if (LongRegOp->isReg()) {
      AccRegNo = LongRegOp->getReg();
      if (MRI->getRegClass(TPC::HWZPRegRegClassID).contains(AccRegNo)) {
        LongRegOp = static_cast<TPCOperand *>(Operands[++DestRegOpNum].get());
      }
    }

    if (!LongRegOp->isReg())
      return true;
    AccRegNo = LongRegOp->getReg();
    if (!MRI->getRegClass(TPC::VRFRegClassID).contains(AccRegNo))
      return true;

    if (Mnemonic.startswith_lower("mac")) {
      // If MAC has suffix ACC_I16 or ACC_FP32, it uses double registers.
      if (DataType == TPCII::OpType::INT8 || DataType == TPCII::OpType::UINT8) {
        if (HasAccFlag) {
          // MAC.I8.ACC_I16 V0, V5, V6 -> MAC.I8.ACC_I16 D0, V5, V6
          Kind = LongRegisterKind::DRF;
        } else {
          // MAC.I8 V0, V5, V6 -> MAC.I8 A0, V5, V6
          Kind = LongRegisterKind::ARF;
        }
      } else if (DataType == TPCII::OpType::INT16 || DataType == TPCII::OpType::UINT16) {
        // MAC.I16 V8, V1, V2 -> MAC.I16 D8, V1, V2
        Kind = LongRegisterKind::DRF;
      } else if (DataType == TPCII::OpType::BF16) {
        if (HasAccFlag) {
          // MAC.BF16.ACC_FP32 V6, V1, V2 -> MAC.BF16.ACC_FP32 D6, V1, V2
          Kind = LongRegisterKind::DRF;
        } else {
          return true;
        }
      } else {
        return true;
      }
    } else {
      if (DataType == TPCII::OpType::BF16) {
        // In MUL.BF16.ACC_FP32 V0, V1, V2: V0 ->D0
        if (HasAccFlag)
          Kind = LongRegisterKind::DRF;
        else
          return true;
      } else if (DataType == TPCII::OpType::INT32 || DataType == TPCII::OpType::UINT32) {
        if ((Switches & TPCII::SW_GROUP_RND32) == TPCII::SW_RND32_NO_ROUND) {
          //assert(!HasAccFlag);
          //auto AccOp = TPCOperand::CreateAccumulator(SwitchOp.getStartLoc(), SwitchOp.getEndLoc());
          //Operands.insert(Operands.begin() + 3, std::move(AccOp));
          //++DestRegOpNum;
          Kind = LongRegisterKind::DRF;
        } else {
          assert(!HasAccFlag);
          auto AccOp = TPCOperand::CreateAccumulator(SwitchOp.getStartLoc(), SwitchOp.getEndLoc());
          Operands.insert(Operands.begin() + 3, std::move(AccOp));
          return true;
        }
      } else if (DataType == TPCII::OpType::INT16 || DataType == TPCII::OpType::UINT16) {
        // In MUL.I16 V0, V1, V2: V0 -> D0
        Kind = LongRegisterKind::DRF;
      } else if (DataType == TPCII::OpType::INT8 || DataType == TPCII::OpType::UINT8) {
        Kind = LongRegisterKind::ARF;
      } else {
        return true;
      }
    }
  } else if (Mnemonic.startswith_lower("ash")) {
    if (Operands.size() < 8) {
      return true;
    }
    DestRegOpNum = 5;
    unsigned RhazRsNo = 3;

    TPCOperand *RhazRsOp = static_cast<TPCOperand*>(Operands[RhazRsNo].get());
    if (RhazRsOp->isRhazRs()) {
      LongRegOp = static_cast<TPCOperand*>(Operands[DestRegOpNum].get());
      Kind = LongRegisterKind::DRF;
      AccRegNo = LongRegOp->getReg();
      if (!MRI->getRegClass(TPC::VRFRegClassID).contains(AccRegNo))
        return true;
    } else {
      Error(LongRegOp->getStartLoc(), "Too many operands and no rhaz_rs switch");
      return false;
    }

  } else if (Mnemonic.startswith_lower("sel2_")) {
    const unsigned DestRegOpNum = 3;
    return replaceRegisterWithLong(Operands, DestRegOpNum, LongRegisterKind::DRF);
  } else if (Mnemonic.startswith_lower("get_lut_entry_and_interval_start")) {
    const unsigned DestRegOpNum = 3;
    return replaceRegisterWithLong(Operands, DestRegOpNum, LongRegisterKind::DRF);
  } else if (Mnemonic.startswith_lower("lookup_c1c2") ||
             Mnemonic.startswith_lower("lookup_c2")) {
    const unsigned DestRegOpNum = 2;
    return replaceRegisterWithLong(Operands, DestRegOpNum, LongRegisterKind::DRF);
  } else if (Mnemonic.startswith_lower("udiv_")) {
    DestRegOpNum = 2;
    LongRegOp = static_cast<TPCOperand*>(Operands[DestRegOpNum].get());
    if (LongRegOp->isReg())
        AccRegNo = LongRegOp->getReg();
    if (!LongRegOp->isReg() || MRI->getRegClass(TPC::HWDivStepRegClassID).contains(AccRegNo)) {
      LongRegOp = static_cast<TPCOperand*>(Operands[++DestRegOpNum].get());
      if (LongRegOp->isReg())
        AccRegNo = LongRegOp->getReg();
      if (!LongRegOp->isReg() || MRI->getRegClass(TPC::HWDivStepRegClassID).contains(AccRegNo)) {
        LongRegOp = static_cast<TPCOperand*>(Operands[++DestRegOpNum].get());
      }
    }
    assert(LongRegOp->isReg() && "Incorrect argument in udiv_step instruction");

    AccRegNo = LongRegOp->getReg();

    if (!MRI->getRegClass(TPC::SRFRegClassID).contains(AccRegNo))
      return true;
    Kind = LongRegisterKind::ZRF;
  } else if (Mnemonic.startswith_lower("convert_int8") ||
             Mnemonic.startswith_lower("convert_uint8")) {
    const unsigned DestRegOpNum = 3;
    return replaceRegisterWithLong(Operands, DestRegOpNum, LongRegisterKind::DRF);
  } else if (Mnemonic.equals_lower("mov")) {
    const unsigned SwitchOpNo = 1;
    TPCOperand *SwitchOp = static_cast<TPCOperand *>(Operands[SwitchOpNo].get());
    if (SwitchOp->isDataType())
      SwitchOp = static_cast<TPCOperand *>(Operands[SwitchOpNo + 1].get());
    unsigned Switches = SwitchOp->getSwitchSet();
    if (Switches & TPCII::SW_X2_MOV) {
      return replaceRegisterWithLong(Operands, 2, LongRegisterKind::DRF) &&
             replaceRegisterWithLong(Operands, 3, LongRegisterKind::DRF);
    }
    return true;
  } else if (Mnemonic.startswith_lower("nearbyint")) {
    const unsigned SwitchOpNo = 2;
    TPCOperand *SwitchOp = static_cast<TPCOperand *>(Operands[SwitchOpNo].get());
    unsigned Switches = SwitchOp->getSwitchSet();
    if (Switches & TPCII::SW_CNVRT)
      return replaceRegisterWithLong(Operands, 3, LongRegisterKind::DRF);
    return true;
  } else {
    return true;
  }

  unsigned NewRegNo = convertToLongReg(AccRegNo, Kind);
  static const int RegClass[3] = { TPC::ARFRegClassID, TPC::DRFRegClassID, TPC::ZRFRegClassID };
  if (NewRegNo == 0) {
    static const StringRef RegName[3] = { "ARF", "DRF", "ZRF" };
    static const StringRef RegAlign[3] = { "4", "2", "2" };
    Error(LongRegOp->getStartLoc(),
          "Register '" + StringRef(TPCInstPrinter::getRegisterName(AccRegNo)) +
          "' Cannot be a start of " + RegName[static_cast<unsigned>(Kind)] +
          " register, the number must be a multiple of " + RegAlign[static_cast<unsigned>(Kind)],
          LongRegOp->getLocRange());
    return false;
  }
  auto NewReg = TPCOperand::CreateReg(NewRegNo, RegClass[static_cast<unsigned>(Kind)],
                                      LongRegOp->getStartLoc(), LongRegOp->getEndLoc());
  Operands[DestRegOpNum] = std::move(NewReg);
  return true;
}

static bool checkHALT(OperandVector &Operands) {
  TPCOperand &Op = static_cast<TPCOperand &>(*Operands[0]);
  assert(Op.isToken() && "Leading operand should always be a mnemonic!");
  StringRef Mnemonic = Op.getToken();
  return Mnemonic.equals_lower("halt") && Operands.size() == 1;
}


bool TPCAsmParser::MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                                           OperandVector &Operands,
                                           MCStreamer &Out,
                                           uint64_t &ErrorInfo,
                                           bool MatchingInlineAsm) {
  MatchCode MatchResult;

  // Process start of instruction.
  if (CurrentSlot == SlotParser::Special) {
    MatchResult = MatchSlotInstruction(IDLoc, Operands, Bundle,
                         ErrorInfo, MatchingInlineAsm, Out, SlotParser::Special);
    if (MatchResult == MatchCode::Ok) {
      // All-bundle instruction just parsed.
      Bundle.setLoc(IDLoc);
      if (!MatchingInlineAsm)
        AC.EmitInstruction(Bundle, Out, getSTI());
      Bundle.clear();
      Bundle.setOpcode(0);
      Bundle.setLoc(SMLoc());
      CurrentSlot = SlotParser::Unknown;
      Opcode = Bundle.getOpcode();
      return false;
    }
    return true;
  } else if (CurrentSlot == SlotParser::Load) {
    // Assume we have 4 slot instructions.
    initBundle(IDLoc);
    // If this instruction mnemonic is HALT, expand it to "NOP; HALT; HALT; NOP".
    if (checkHALT(Operands)) {
      Bundle.addOperand(MCOperand::createInst(getNOP(SlotParser::Load, IDLoc)));
      Bundle.addOperand(MCOperand::createInst(getHALT(SlotParser::Scalar, IDLoc)));
      Bundle.addOperand(MCOperand::createInst(getHALT(SlotParser::Vector, IDLoc)));
      Bundle.addOperand(MCOperand::createInst(getNOP(SlotParser::Store, IDLoc)));
      outputBundle(Out, IDLoc, !MatchingInlineAsm);
      Opcode = Bundle.getOpcode();
      return false;
    }
  }

  // If the last operand is a predicate register, convert it into the
  // predicate operand if possible.
  MCParsedAsmOperand *LastArg = Operands.back().get();
  if (LastArg && LastArg->isReg()) {
    unsigned RegNo = LastArg->getReg();
    PredicateKind PredKind = getPredicateKind(RegNo);
    if (PredKind != PredicateKind::NotAPredicate) {
      TPCOperand &Op = static_cast<TPCOperand &>(*Operands[0]);
      assert(Op.isToken() && "Leading operand should always be a mnemonic!");
      StringRef Mnemonic = Op.getToken();
      if (canLastArgBeAPredicate(Mnemonic, Operands, true)) {
        MCParsedAsmOperand *LastArg = Operands.back().get();
        unsigned RegNo = LastArg->getReg();
        SMLoc StartLoc = LastArg->getStartLoc();
        SMLoc EndLoc = LastArg->getEndLoc();
        Operands.pop_back();
        Operands.push_back(TPCOperand::CreatePredicate(RegNo, false, PredKind == VectorPredicate, StartLoc, EndLoc));
      }
    }
  }

  if (!adjustLongRegister(Operands))
    return true;

  if (CurrentSlot == SlotParser::Unknown) {
    TPCOperand &Op = static_cast<TPCOperand &>(*Operands[0]);
    Error(IDLoc, "Too many slots", Op.getLocRange(), MatchingInlineAsm);
    return true;
  }

  // Parse current slot instruction.
  BundledSubInstructions.emplace_back();
  MCInst &SubInst = BundledSubInstructions.back();
  MatchResult = MatchSlotInstruction(IDLoc, Operands, SubInst,
                                ErrorInfo, MatchingInlineAsm, Out, CurrentSlot);
  if (MatchResult != MatchCode::Ok)
    return true;


  Bundle.addOperand(MCOperand::createInst(&SubInst));
  Opcode = SubInst.getOpcode();

  if (NewAsmFormat && CurrentSlot == SlotParser::Store) {
    IsLastInstrInBundle = true;
  }

  if (IsLastInstrInBundle) {
    outputBundle(Out, IDLoc, !MatchingInlineAsm);
  } else {
    // Otherwise proceed to the next slot.
    switch (CurrentSlot) {
    case SlotParser::Load: CurrentSlot = SlotParser::Scalar; break;
    case SlotParser::Scalar: CurrentSlot = SlotParser::Vector; break;
    case SlotParser::Vector: CurrentSlot = SlotParser::Store; break;
    case SlotParser::Store: CurrentSlot = SlotParser::Unknown; break;
    default:
      llvm_unreachable("Wrong current parser");
    }
  }

  return false;
}


void TPCAsmParser::initBundle(SMLoc Loc) {
  if (CurrentSlot == SlotParser::Unknown) {
    assert(Bundle.getOpcode() == 0);
    CurrentSlot = SlotParser::Load;
    Bundle.setLoc(Loc);
    Bundle.setOpcode(TPC::BUNDLE);
    Bundle.addOperand(MCOperand::createImm(0));
  }
}


void TPCAsmParser::outputBundle(MCStreamer &Out, SMLoc IDLoc, bool UseStreamer) {
  if (Bundle.getOpcode() == TPC::BUNDLE) {
    assert(Bundle.getOperand(0).isImm());
    assert(Bundle.getOperand(0).getImm() == 0);
    // Add missing slot instructions.
    for (unsigned I = Bundle.getNumOperands() - 1; I < 4; ++I) {
      switch (I) {
      case 0:
        Bundle.addOperand(MCOperand::createInst(getNOP(SlotParser::Load, IDLoc)));
        break;
      case 1:
        Bundle.addOperand(MCOperand::createInst(getNOP(SlotParser::Scalar, IDLoc)));
        break;
      case 2:
        Bundle.addOperand(MCOperand::createInst(getNOP(SlotParser::Vector, IDLoc)));
        break;
      case 3:
        Bundle.addOperand(MCOperand::createInst(getNOP(SlotParser::Store, IDLoc)));
        break;
      default:
        llvm_unreachable("Wrong bundle");
      }
    }
  }

  // Output to streamer.
  LLVM_DEBUG(dbgs() << "Bundle:");
  LLVM_DEBUG(Bundle.dump_pretty(dbgs()));
  LLVM_DEBUG(dbgs() << "--\n");

  if (UseStreamer)
    AC.EmitInstruction(Bundle, Out, getSTI());

  // Clear current bundle.
  Bundle.clear();
  Bundle.setOpcode(0);
  Bundle.setLoc(SMLoc());
  CurrentSlot = SlotParser::Unknown;
}

bool TPCAsmParser::parseTPCMetadata() {
  const AsmToken &Tok = Lexer.getTok();

  TPCHeader = TPCMetadataSection();
  std::unordered_set<const TPCMetadataFieldInfo *> SetField;

  std::string RedefinitionField = formatv("Redefintion of {0} field: ",
                                          BinaryTPCMetadataSectionName);

  // scalarLds set indexes.
  std::bitset<TPCNumTensors> ScalarIdSetIndexes;
  // rmwStore set indexes.
  std::bitset<TPCNumTensors> RMWStoreIndexes;
  // Parse field by field

  for (;;) {
    while (Tok.getKind() == AsmToken::EndOfStatement)
      Lexer.Lex();

    if (Tok.getKind() == AsmToken::Eof)
      break;

    const TPCMetadataFieldInfo *CurrentFieldInfo =
        getTPCMetadataFieldInfo(Tok.getString());
    if (!CurrentFieldInfo)
      break;  // Possible, if it is start of an assembler program
    if (StringRef(CurrentFieldInfo->fieldName).compare(TPCScalarLdName) == 0) {
      if (!parseTPCMetadataArrayField(ScalarIdSetIndexes, *CurrentFieldInfo))
        return false;
      continue;
    }
    if (StringRef(CurrentFieldInfo->fieldName).compare(TPCRMWStoreName) == 0) {
      if (!parseTPCMetadataArrayField(RMWStoreIndexes, *CurrentFieldInfo))
        return false;
      continue;
    }
    if (SetField.find(CurrentFieldInfo) != SetField.end()) {
      Error(Tok.getLoc(), Twine(RedefinitionField.data(), Tok.getString()),
            Tok.getLocRange());
      return false;
    }

    SetField.insert(CurrentFieldInfo);
    Lexer.Lex();

   // Parse ':'
   if (Tok.getKind() == AsmToken::Colon) {
     Lexer.Lex();
   } else {
     Error(Tok.getLoc(), Twine(TPCHeaderParseBaseError, Tok.getString()) +
           "'. Expected ':'.", Tok.getLocRange());
     return false;
   }

   // Parse type directive
   if (!parseTPCMetadataTypeDirective(CurrentFieldInfo->elementSize))
     return false;

   // Parse value
   if (Tok.getKind() == AsmToken::Integer) {
     std::string ErrorMessage;
     if (!setTpcMetadataValue(Tok.getAPIntVal().getLimitedValue(),
                             *CurrentFieldInfo,
                             *TPCHeader,
                             ErrorMessage)) {
       Error(Tok.getLoc(), ErrorMessage, Tok.getLocRange());
       return false;
     }

     // Check arch
     if (StringRef(CurrentFieldInfo->fieldName).compare(TPCMarchName) == 0) {
       StringRef CPU = getSTI().getCPU();
       bool WrongArch = false;
       switch (TPCHeader.getValue().march) {
       case 1:
         WrongArch = CPU.compare_lower("goya") != 0;
         break;
       case 2:
         WrongArch = CPU.compare_lower("gaudi") != 0;
         break;
       case 3:
         WrongArch = CPU.compare_lower("greco") != 0;
         break;
       case 4:
         WrongArch = CPU.compare_lower("gaudi2") != 0;
         break;
       case 5:
         WrongArch = CPU.compare_lower("gaudib") != 0;
         break;
       case 6:
         WrongArch = CPU.compare_lower("doron1") != 0;
         break;
       default:
         llvm_unreachable("Unknown arch");
       }

       if (WrongArch) {
         Error(Tok.getLoc(),
               formatv("Specified TPC_METADATA architecture is different from actual target: {0}", CPU),
               Tok.getLocRange());
         return false;
       }
     }

     Lexer.Lex();
   } else {
     Error(Tok.getLoc(), Twine(TPCHeaderParseBaseError, Tok.getString()) +
           ". Expected a number value.",
           Tok.getLocRange());
     return false;
   }
  }

  return true;
}


bool TPCAsmParser::parseTPCMetadataArrayField(
    std::bitset<TPCNumTensors> &SetIndexes, const TPCMetadataFieldInfo &FieldInfo) {
  const AsmToken &Tok = Lexer.getTok();

  // For more accuracy error message
  AsmToken IndexTok;
  AsmToken FieldTok;

  const std::string RedefinitionMessage = formatv("Redefintion of {0} field.",
                                                  BinaryTPCMetadataSectionName);

  // Parse field by field
  for (;;) {
    // Eat end of lines and end of statement
    while (Tok.getKind() == AsmToken::EndOfStatement)
     Lexer.Lex();

    if (Tok.getKind() == AsmToken::Eof)
      return true;

    // Possible it is other TPC_METADATA field
    if (Tok.getKind() != AsmToken::Identifier)
      return true;

    const StringRef &CurrentFieldName = Tok.getString();
    // Parse field name
    if (CurrentFieldName.compare_lower(FieldInfo.fieldName) == 0)
      Lexer.Lex();
    else // Possible it is other TPC_METADATA field
      return true;

    // Optional, parse '[NUMBER]'
    int Index = -1;
    if (Tok.getKind() == AsmToken::LBrac) {
      Lexer.Lex();

      if (Tok.getKind() == AsmToken::Integer) {
        uint64_t TempIndex = Tok.getAPIntVal().getLimitedValue();
        if (TempIndex > FieldInfo.length) {
          Error(Tok.getLoc(),
                "Tensor index out of range, expected from 0 to 15.",
                Tok.getLocRange());
          return false;
        }
        Index = TempIndex;
        IndexTok = Tok;
        Lexer.Lex();
      } else {
        Error(Tok.getLoc(), Twine(TPCHeaderParseBaseError, Tok.getString()) +
              "'. Expected INTEGER token.", Tok.getLocRange());
        return false;
      }

      if(Tok.getKind() == AsmToken::RBrac)
        Lexer.Lex();
      else {
        Error(Tok.getLoc(), Twine(TPCHeaderParseBaseError, Tok.getString()) +
              "'. Expected ']'.", Tok.getLocRange());
        return false;
      }
    }

    if (Index >= 0 && SetIndexes[Index]) {
      Error(IndexTok.getLoc(), RedefinitionMessage,
            IndexTok.getLocRange());
      return false;
    } else if (Index < 0 && SetIndexes.any()){
      Error(FieldTok.getLoc(), RedefinitionMessage,
            FieldTok.getLocRange());
      return false;
    } else if (Index >= 0)
      SetIndexes[Index] = true;
    else
      SetIndexes.flip();

    // Parse ':'
    if (Tok.getKind() == AsmToken::Colon)
      Lexer.Lex();
    else {
      Error(Tok.getLoc(), Twine(TPCHeaderParseBaseError, Tok.getString()) +
            "'. Expected ':'.", Tok.getLocRange());
      return false;
    }

    // Parse type directive
    if (!parseTPCMetadataTypeDirective(
          Index < 0 ?
          FieldInfo.elementSize * FieldInfo.length : FieldInfo.elementSize))
      return false;

    // Parse value
    std::string ErrorMessage;
    if (Index >= 0 && !setTpcMetadataArrayValue(Tok.getAPIntVal().getLimitedValue(), Index, FieldInfo, *TPCHeader, ErrorMessage)) {
      Error(Tok.getLoc(), ErrorMessage, Tok.getLocRange());
      return false;
    } else if (Index < 0 && !setTpcMetadataArrayValue(Tok.getString(), FieldInfo, *TPCHeader, ErrorMessage)) {
      Error(Tok.getLoc(), ErrorMessage, Tok.getLocRange());
      return false;
    }

    Lexer.Lex();
  }
}


bool TPCAsmParser::parseTPCMetadataTypeDirective(unsigned ExpectedSize) {
  const AsmToken &Tok = Lexer.getTok();
  bool IsTypeCorrect = false;
  unsigned CurrentSize = 0;
  for (auto Type : TPCMetadataTypeDirectives) {
    if (Tok.getString().compare_lower(Type.second) == 0) {
      CurrentSize = Type.first;
      IsTypeCorrect = true;
      break;
    }
  }

  if (!IsTypeCorrect) {
     std::string SuffixMsg = "'. Expected one of ";
     for (auto It = TPCMetadataTypeDirectives.begin();
          It != TPCMetadataTypeDirectives.end(); ++It) {
       SuffixMsg += formatv("'{0}'", It->second);
       if (std::next(It) != TPCMetadataTypeDirectives.end())
         SuffixMsg += ", ";
       else
         SuffixMsg += '.';
     }

     Error(Tok.getLoc(), Twine(TPCHeaderParseBaseError,Tok.getString())
           .concat(SuffixMsg), Tok.getLocRange());
     return false;
  }

  if (CurrentSize != ExpectedSize) {
    assert(TPCMetadataTypeDirectives.find(ExpectedSize) !=
        TPCMetadataTypeDirectives.end());
    std::string SuffixMsg = formatv(
          "'. Expected '{0}'.", TPCMetadataTypeDirectives.at(ExpectedSize));
    Error(Tok.getLoc(), Twine(TPCHeaderParseBaseError, Tok.getString())
          .concat(SuffixMsg), Tok.getLocRange());
    return false;
  }

  Lexer.Lex();
  return true;
}


bool TPCAsmParser::parseIndexMap() {
  Lexer.setSkipSpace(false);

  const AsmToken &Tok = Lexer.getTok();
  SMLoc Start;
  SMLoc End;
  for (;;) {
    StringRef TokStr = Tok.getString();
    if (TokStr.equals("SCEVBEGIN"))
      Start = Tok.getLoc();
    else if (TokStr.equals("SCEVEND")) {
      End = Tok.getEndLoc();
      break;
    }

    if (Tok.getKind() == AsmToken::Eof)
      return false;

    Lexer.Lex();
  }

  IndexMap = std::string(Start.getPointer(),
                         End.getPointer());
  Lexer.setSkipSpace(true);
  return true;
}


void TPCAsmParser::flushPendingInstructions(MCStreamer &Out) {
  if (CurrentSlot != SlotParser::Unknown)
    outputBundle(Out, SMLoc(), true);
  AC.flushPendingInstructions(Out, getSTI());

  if (TPCHeader) {
    Out.PushSection();
    MCSectionELF *MetadataSection = getContext().getELFSection(
          BinaryTPCMetadataSectionName,
          ELF::SHT_PROGBITS, ELF::SHF_WRITE | ELF::SHF_ALLOC);
    MetadataSection->getFragmentList().clear();
    MCDataFragment* Fragment = new MCDataFragment(MetadataSection);

    const TPCMetadataSection &Header = TPCHeader.getValue();
    std::vector<uint8_t> BinaryValue = binarySerializeTPCProgramHeader(Header);
    Fragment->getContents().append(BinaryValue.begin(), BinaryValue.end());
    Fragment->setAlignToBundleEnd(true);
    Out.SwitchSection(MetadataSection);
    Out.PopSection();
  }

  std::string RootFileName = getContext().getMainFileName();

  if (!RootFileName.empty()) {
    Out.PushSection();
    MCSectionELF *KernelInfoSection = getContext().getELFSection(
          KernelInfoSectionName,
          ELF::SHT_PROGBITS, ELF::SHF_WRITE | ELF::SHF_ALLOC);
    KernelInfoSection->getFragmentList().clear();
    MCDataFragment* Fragment = new MCDataFragment(KernelInfoSection);

    StringRef BareFileName = llvm::sys::path::stem(RootFileName);
    std::string KernelInfo = GenerateKernelInfo(BareFileName);
    Fragment->getContents().append(KernelInfo.begin(), KernelInfo.end());
    Fragment->setAlignToBundleEnd(true);
    Out.SwitchSection(KernelInfoSection);
    Out.PopSection();
  }

  if (IndexMap) {
    Out.PushSection();
    MCSectionELF *IndexMapSection = getContext().getELFSection(
          IndexMapSectionName,
          ELF::SHT_PROGBITS, ELF::SHF_WRITE | ELF::SHF_ALLOC);
    IndexMapSection->getFragmentList().clear();
    MCDataFragment* Fragment = new MCDataFragment(IndexMapSection);

    Fragment->getContents().append(IndexMap->begin(), IndexMap->end());
    Fragment->setAlignToBundleEnd(true);
    Out.SwitchSection(IndexMapSection);
    Out.PopSection();
  }
}


TPCAsmParser::MatchCode TPCAsmParser::MatchSlotInstruction(SMLoc IDLoc,
    OperandVector &Operands, MCInst &Inst, uint64_t &ErrorInfo,
    bool MatchingInlineAsm, MCStreamer &Out, SlotParser Slot) {
  assert(!Operands.empty() && "Unexpect empty operand list!");
  TPCOperand &Op = static_cast<TPCOperand &>(*Operands[0]);
  assert(Op.isToken() && "Leading operand should always be a mnemonic!");
  StringRef Mnemonic = Op.getToken();

  unsigned MatchResult = MatchInstructionImpl(Operands, Inst, ErrorInfo,
                                              MatchingInlineAsm, Slot);
  switch (MatchResult) {
  case Match_Success:
    break;
  case Match_MnemonicFail:
    if (CurrentSlot == SlotParser::Unknown)
      return MatchCode::Unrecognized;
    Error(IDLoc, "Instruction mnemonic '" + Mnemonic + "' is invalid in " + SlotName[Slot] + " slot",
          Op.getLocRange(), MatchingInlineAsm);
    return MatchCode::Failed;
  case Match_MissingFeature:
    Error(IDLoc, "Missing feature for instruction '" + Mnemonic + "'",
          Op.getLocRange(), MatchingInlineAsm);
    return MatchCode::Failed;
  case Match_InvalidOperand:
    Error(IDLoc, "Invalid operand in the instruction '" + Mnemonic + "'",
          Op.getLocRange(), MatchingInlineAsm);
    return MatchCode::Failed;
  case Match_InvalidTiedOperand:
    Error(IDLoc, "Invalid tied operand in the instruction '" + Mnemonic + "'",
          Op.getLocRange(), MatchingInlineAsm);
    return MatchCode::Failed;
  default:
    Error(IDLoc, "Error while parsing instruction '" + Mnemonic + "'",
          Op.getLocRange(), MatchingInlineAsm);
    return MatchCode::Failed;
  }

  return MatchCode::Ok;
}


// Force static initialization.
extern "C" void LLVMInitializeTPCAsmParser() {
  RegisterMCAsmParser<TPCAsmParser> X(getTheTPCTarget());
}
