#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"
#include "TPCAsmInstCompress.h"
#include <algorithm>

using namespace llvm;

static StringRef MCOperandToString(const MCOperand &Operand) {
  if (Operand.isExpr()) {
    const MCExpr* Expr = Operand.getExpr();
    const MCSymbolRefExpr* SymbolExpr = dyn_cast<MCSymbolRefExpr>(Expr);
    assert(SymbolExpr != nullptr);
    return SymbolExpr->getSymbol().getName();
  } else
    return StringRef();
}

void TPCAsmInstCompress::EmitInstruction(MCInst &Inst, MCStreamer &Out,
                                         const MCSubtargetInfo &STI) {
  if (!compressEnabled || !STI.hasFeature(TPC::FeatureCompress)) {
    Out.emitInstruction(Inst, STI);
    return;
  }

  // If Inst is loop the instructions will store until end_loop is occurred.
  StringRef Label;
  if (isLoopMCInst(Inst, Label))
    IsStoreInBuffer = true;
  else if(isJmpMCInst(Inst, Label)) {
    JmpLabels.push_back(Label);
    if (!Buffer.empty() && any_isa<MCSymbol *>(Buffer.front())) {
      if(any_cast<MCSymbol *>(Buffer.front())->getName() == Label)
        IsStoreInBuffer = false;
    }
  }

  // Flush buffer before emit current instruction.
  if (!Buffer.empty() && !IsStoreInBuffer) {
    flushBuffer(Out, STI);
    // Not all instructions have emitted
    if (!Buffer.empty())
      IsStoreInBuffer = true;
  }

  if (IsStoreInBuffer) {
    Buffer.push_back(Inst);
    return;
  }
  
  if (STI.hasFeature(TPC::FeatureDoron1) && isJmpMCInst(Inst, Label)) {
    flushInstUncompressed(Inst, Out, STI);
    DoNotCompressNextInst = true;
  } else if (maybeCompressInstr(Inst, false, STI))
    flushInstCompressed(Inst, Out, STI);
  else {
    flushInstUncompressed(Inst, Out, STI);
    DoNotCompressNextInst = false;
  }
}

void TPCAsmInstCompress::flushPendingInstructions(MCStreamer &Out,
                                                  const MCSubtargetInfo &STI) {
  if (!compressEnabled) {
    return;
  }
  if (!Buffer.empty()) {
    flushBuffer(Out, STI, true);
  }
  if (pInst != nullptr) {
    Out.emitInstruction(prevInst, STI);
    pInst = nullptr;
  }
}

void TPCAsmInstCompress::onLabelEmited(MCSymbol *Symbol) {
  if (!compressEnabled) {
    return;
  }

  if (!IsStoreInBuffer) {
    // If the Label is a target adress of a jmp instruction
    // when next instruction can not be compressed.
    if (isJmpLabel(Symbol))
      DoNotCompressNextInst = true;
    // Otherwise we start storing instructions
    else
      IsStoreInBuffer = true;
  }

  if (IsStoreInBuffer)
    Buffer.push_back(Symbol);
  else
    PendingLabels.push_back(Symbol);

  // Loop ending label check
  if (!Buffer.empty() && any_isa<MCInst>(Buffer.front())) {
    StringRef LoopLabel;
    if (isLoopMCInst(any_cast<MCInst>(Buffer.front()), LoopLabel))
      if (LoopLabel == Symbol->getName())
        IsStoreInBuffer = false;
  }
}

void TPCAsmInstCompress::flushPendingLabels(MCStreamer &Out) {
  if (PendingLabels.empty()) {
    return;
  }
  for (MCSymbol *Sym : PendingLabels) {
    Sym->setRedefinable(true);
    Out.emitLabel(Sym);
  }
  PendingLabels.clear();
}

void TPCAsmInstCompress::flushBuffer(MCStreamer &Out,
                                     const MCSubtargetInfo &STI,
                                     bool FlushAll) {
  assert(compressEnabled);
  auto It = Buffer.begin();
  for(; It != Buffer.end(); ++It) {
    if (any_isa<MCInst>(*It)) {
      auto& Inst = any_cast<MCInst &>(*It);
      StringRef Looplabel;
      if (!isLoopMCInst(Inst, Looplabel)) {
        StringRef JmpLabel;
        if (STI.hasFeature(TPC::FeatureDoron1) && isJmpMCInst(Inst, JmpLabel)) {
          flushInstUncompressed(Inst, Out, STI);
          DoNotCompressNextInst = true;
        } else if(!DoNotCompressNextInst &&
                  maybeCompressInstr(Inst, false, STI)) {
         flushInstCompressed(Inst, Out, STI);
        } else {
          flushInstUncompressed(Inst, Out, STI);
          DoNotCompressNextInst = false;
        }
      } else {
          // For loop we have special algorithm.
          // It will move to next instruction after loop end
        flushLoopsInsts(It, Out, STI);
      }
    } else {
      MCSymbol *Label = any_cast<MCSymbol *>(*It);
      // If the Label is a target adress of a jmp instruction
      // when next instruction can not be compressed.
      if (std::find(JmpLabels.begin(), JmpLabels.end(),Label->getName())
          != JmpLabels.end()) {
        DoNotCompressNextInst = true;
        PendingLabels.push_back(Label);
      } else {
        // Otherwise if FlushAll is true the Label is unused label.
        // If FlushAll is false algorithm stop and
        // we continue search jmp instruction.
        if(FlushAll)
          PendingLabels.push_back(Label);
        else
          break;
      }
    }
  }

  Buffer.erase(Buffer.begin(), It);
}



void TPCAsmInstCompress::flushInstCompressed(MCInst &Inst, MCStreamer &Out,
                                             const MCSubtargetInfo &STI) {
  assert(IsFirstInstrProcessed);
  if(pInst != nullptr) {
    maybeCompressInstr(prevInst, true, STI);
    Out.emitInstruction(prevInst, STI);
    flushPendingLabels(Out);
    maybeCompressInstr(Inst, true, STI);
    Out.emitInstruction(Inst, STI);
    pInst = nullptr;
  } else {
    flushPendingLabels(Out);
    prevInst = Inst;
    pInst = &prevInst;
  }
}

void TPCAsmInstCompress::flushInstUncompressed(MCInst &Inst, MCStreamer &Out,
                                               const MCSubtargetInfo &STI) {
  if (!IsFirstInstrProcessed)
    IsFirstInstrProcessed = true;

  if (pInst != nullptr) {
    Out.emitInstruction(prevInst, STI);
    flushPendingLabels(Out);
    Out.emitInstruction(Inst, STI);
    pInst = nullptr;
  } else {
    flushPendingLabels(Out);
    Out.emitInstruction(Inst, STI);
  }
}

bool TPCAsmInstCompress::isNopMCInst(const MCInst &MI) const {
  if (MI.getOpcode() == TPC::BUNDLE) {
    for (auto &I : TPCMCInstrInfo::bundleInstructions(MI)) {
         MCInst &BMI = const_cast<MCInst &>(*I.getInst());
      if (!isNopMCInst(BMI)) {
        return false;
      }
    }
    return true;
  }
  else {
    if (MI.getOpcode() == TPC::NOPv  ||
        MI.getOpcode() == TPC::NOPs  ||
        MI.getOpcode() == TPC::NOPld ||
        MI.getOpcode() == TPC::NOPst) {
      return true;
    }
  }
  return false;
}

bool TPCAsmInstCompress::isVpuInstrWithSrcCD(const MCInst &MI) const {
  const MCInstrDesc &MC = MCII.get(MI.getOpcode());
  if (!TPCII::isVPUInst(MC))
    return false;
  if (TPCII::getHasSrcC(MC) || TPCII::getHasSrcD(MC))
    return true;

  return false;
}

bool TPCAsmInstCompress::isVPUInstWithVPUExtSwitch(const MCInst &MI) const {
  const MCInstrDesc &Desc = MCII.get(MI.getOpcode());
  if (!TPCII::isVPUInst(Desc))
    return false;

  const auto &GetSwitch =[&Desc](const MCInst MI){
    for (unsigned I = Desc.getNumOperands() - 1; I >= 0; --I) {
      assert(Desc.OpInfo);
      const MCOperandInfo OpInfo = Desc.OpInfo[I];
      if (OpInfo.OperandType == TPC::OperandType::OPERAND_SWITCH) {
        const MCOperand &MCOperand = MI.getOperand(I);
        assert(MCOperand.isImm());
        return static_cast<uint32_t>(MCOperand.getImm());
      }
    }

    assert(false && "Switch was not found.");
    return 0U;
  };

  unsigned SlotOpcode = TPCII::getSlotOpCode(Desc);
  if (SlotOpcode == TPCII::vpuMAC ||
      SlotOpcode == TPCII::vpuMADD) {
    unsigned SwitchVal = GetSwitch(MI);
    if (SwitchVal & TPCII::SW_ZP ||
        SwitchVal & TPCII::SW_NEG_ZP)
      return true;
  }

  return false;
}

bool TPCAsmInstCompress::isSrcCIsStoreSrcC(const MCInst &MI) const {
  const MCInstrDesc &MC = MCII.get(MI.getOpcode());
  if (!TPCII::isVPUInst(MC)) {
    return false;
  }
  if (TPCII::getSrcCIsStoreSrcC(MC))
    return true;

  return false;
}

bool TPCAsmInstCompress::isLoopMCInst(const MCInst &MI, StringRef &Label) const {
  const MCInstrDesc &MIDesc = MCII.get(MI.getOpcode());
  bool IsLoop = TPCII::isLoopInst(MIDesc);

  if (IsLoop) {
    assert(MI.getNumOperands() >= 5);
    const MCOperand& ValueOperand = MI.getOperand(4);
    assert(ValueOperand.isExpr());
    Label = MCOperandToString(ValueOperand);
  }

  return IsLoop;
}

bool TPCAsmInstCompress::isJmpMCInst(const MCInst &MI, StringRef &Label) const {
  if(MI.getOpcode() == TPC::BUNDLE) {
    for (auto &I : TPCMCInstrInfo::bundleInstructions(MI)) {
      MCInst &BMI = const_cast<MCInst &>(*I.getInst());
      if (isJmpMCInst(BMI, Label)) {
        return true;
      }
    }

    return false;
  } else {
    bool IsJmp = false;
    switch(MI.getOpcode()) {
    case TPC::JMPA:
    case TPC::JMPAr:
    case TPC::JMPR:
    case TPC::JMPR_u:
    case TPC::JMPRr:
      IsJmp = true;
      break;
    default:
      return false;
    }
    if(IsJmp) {
      assert(MI.getNumOperands() >= 1);
      const MCOperand& ValueOperand = MI.getOperand(0);
      if(ValueOperand.isExpr()) {
        Label = MCOperandToString(ValueOperand);
      }
      return true;
    }

    return false;
  }
}

bool TPCAsmInstCompress::isJmpLabel(const MCSymbol* Label) const {
  return std::find(JmpLabels.begin(), JmpLabels.end(),
                   Label->getName()) != JmpLabels.end();
}

void TPCAsmInstCompress::flushLoopsInsts(std::vector<Any>::iterator &Iter,
                                         MCStreamer &Out,
                                         const MCSubtargetInfo &STI) {
  SmallVector<StringRef, 4> LoopLabels;
  struct {
    SmallVector<Any, 4> buffer;
    unsigned instCount;
  } LastFourInstrs = {{}, 0};

  // Emit instructions from LastFourInstrs
  auto FlushLastFourInstrs = [&LastFourInstrs, &Out, &STI, this]
      (bool IsCompressed) {
    for (Any &Value : LastFourInstrs.buffer) {
      if (any_isa<MCInst>(Value)) {
        MCInst Inst = any_cast<MCInst &>(Value);
        if (IsCompressed && !DoNotCompressNextInst &&
            maybeCompressInstr(Inst, false, STI))
          flushInstCompressed(Inst, Out, STI);
        else {
          flushInstUncompressed(Inst, Out, STI);
          DoNotCompressNextInst = false;
        }
      } else { //It is a label
        MCSymbol *Label = any_cast<MCSymbol *>(Value);
        bool IsJmpDest = isJmpLabel(Label);
        if (IsJmpDest)
          DoNotCompressNextInst = true;
        PendingLabels.push_back(Label);
      }
    }
    LastFourInstrs.buffer.clear();
    LastFourInstrs.instCount = 0;
  };
  // Get the first instruction and push labels to PendingLabels
  auto PopFirstInst = [&LastFourInstrs, this]() -> MCInst {
    assert(LastFourInstrs.instCount > 0);
    for (auto It = LastFourInstrs.buffer.begin();
         It != LastFourInstrs.buffer.end(); ++It) {
      if(any_isa<MCInst>(*It)) {
        MCInst Result = any_cast<MCInst &>(*It);
        LastFourInstrs.buffer.erase(LastFourInstrs.buffer.begin(), It + 1);
        --LastFourInstrs.instCount;
        return Result;
      } else { // It is a label
        MCSymbol *Label =  any_cast<MCSymbol *>(*It);
        PendingLabels.push_back(Label);
        if (isJmpLabel(Label))
          DoNotCompressNextInst = true;
      }
    }
    llvm_unreachable("An unhandled case occurred");
  };

  for (;;) {
    if (any_isa<MCInst>(*Iter)) {
      MCInst Inst = any_cast<MCInst &>(*Iter);
      StringRef LoopLabel;
      if (isLoopMCInst(Inst, LoopLabel)) {
        FlushLastFourInstrs(true);
        DoNotCompressNextInst = true;
        LoopLabels.emplace_back(std::move(LoopLabel));
        flushInstUncompressed(Inst, Out, STI);
      } else {
        LastFourInstrs.buffer.emplace_back(std::move(Inst));
        ++LastFourInstrs.instCount;
        if (LastFourInstrs.instCount > 4) {
          Inst = PopFirstInst();
          if (!DoNotCompressNextInst && maybeCompressInstr(Inst, false, STI))
            flushInstCompressed(Inst, Out, STI);
          else {
            flushInstUncompressed(Inst, Out, STI);
            DoNotCompressNextInst = false;
          }
        }
      }
    } else { // It is a label
      MCSymbol *Label = any_cast<MCSymbol *>(*Iter);
      assert(Label);

      if (Label->getName() == LoopLabels.back()) {
        // Emit instructions from LastFourInstrs
        FlushLastFourInstrs(false);

        bool IsJmpDest = isJmpLabel(Label);
        if (IsJmpDest)
          DoNotCompressNextInst = true;

        if (LoopLabels.size() != 1 || IsJmpDest)
          PendingLabels.push_back(Label);
        else
          // Do not emit the label because it may be a jmp destination
          // The label will be first element in buffer
          --Iter;
        LoopLabels.pop_back();
        if (LoopLabels.empty())
          break;
      } else
        LastFourInstrs.buffer.push_back(Label);
    }

    ++Iter;
  }
}


void TPCAsmInstCompress::rmOpcodeFromBundle(MCInst &MI, unsigned opcode) const {
  for (auto &I : TPCMCInstrInfo::bundleInstructions(MI)) {
    MCInst &BMI = const_cast<MCInst &>(*I.getInst());

    if (BMI.getOpcode() == opcode) {
      MI.erase(const_cast<MCOperand*>(&I));
      return;
    }
  }
}

bool TPCAsmInstCompress::maybeCompressInstr(MCInst &MI, bool doCompress,
                                            const MCSubtargetInfo &STI) const {
  if (!IsFirstInstrProcessed)
    return false;

  if (MI.getOpcode() != TPC::BUNDLE) {
    return false;
  }
  bool hasVPU = false;
  bool hasSPU = false;
  bool hasLD = false;
  bool hasST = false;
  for (auto &I : TPCMCInstrInfo::bundleInstructions(MI)) {
    MCInst &BMI = const_cast<MCInst &>(*I.getInst());
    const MCInstrDesc &MC = MCII.get(BMI.getOpcode());

    // Check for cross-slot instructions
    if (isVpuInstrWithSrcCD(BMI))
      return false;
    if (isSrcCIsStoreSrcC(BMI))
      return false;
    if (STI.hasFeature(TPC::FeatureDoron1) && isVPUInstWithVPUExtSwitch(BMI))
      return false;

    // Instructions that can be compressed (2 instructions in a single VLIW):
    // Instructions which are not JMPR/JMPA
    if (MC.isTerminator()) {
      return false;
    }

    if (!isNopMCInst(BMI)) {
      if (TPCII::isVPUInst(MC))
        hasVPU = true;
      else if (TPCII::isSPUInst(MC))
        hasSPU = true;
      else if (TPCII::isLoadInst(MC))
        hasLD = true;
      else if (TPCII::isStoreInst(MC))
        hasST = true;
    }
  }
  if ((hasVPU || hasSPU) && (hasLD || hasST)) {
      // Cannot compress
      return false;
  }
  
  //
  // TODO: check for other extra bits?
  //


  //
  // Now we know that the bundle instruction is compressible, so we remove
  // two NOPs from this bundle leaving only two other instructions there.
  // The code emitter will know to compress a bundle with only two instructions.
  //
  if (doCompress) {
    if (hasVPU || hasSPU) {
      rmOpcodeFromBundle(MI, TPC::NOPld);
      rmOpcodeFromBundle(MI, TPC::NOPst);
    }
    else {
      rmOpcodeFromBundle(MI, TPC::NOPs);
      rmOpcodeFromBundle(MI, TPC::NOPv);
    }
  }
  
  return true;
}
