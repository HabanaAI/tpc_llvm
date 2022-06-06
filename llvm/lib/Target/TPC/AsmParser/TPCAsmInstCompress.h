#ifndef LLVM_LIB_TARGET_TPC_ASMPARSER_TPCASMINSTCOMPRESS_H
#define LLVM_LIB_TARGET_TPC_ASMPARSER_TPCASMINSTCOMPRESS_H

#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/ADT/Any.h"
#include <vector>

namespace llvm {

class TPCAsmInstCompress {
public:
  TPCAsmInstCompress() = delete;
  TPCAsmInstCompress(const MCInstrInfo &MII):
    compressEnabled(false), MCII(MII), pInst(nullptr), IsStoreInBuffer(false),
    DoNotCompressNextInst(false)
  {}
  // If compressEnabled is true when simply emit instruction.
  // Otherwise instruction is stored if IsStoreInBuffer is true.
  void EmitInstruction(MCInst &Inst, MCStreamer &Out, const MCSubtargetInfo &STI);
  void flushPendingInstructions(MCStreamer &Out, const MCSubtargetInfo &STI);
  void onLabelEmited(MCSymbol *Symbol);
  void setCompressEnabled(bool val) { compressEnabled = val; }

private:
  bool compressEnabled;
  const MCInstrInfo &MCII;
  MCInst prevInst;
  MCInst * pInst;
  SmallVector<MCSymbol *, 2> PendingLabels;

  std::vector<StringRef> JmpLabels;
  std::vector<Any> Buffer;

  bool IsStoreInBuffer;
  bool DoNotCompressNextInst;
  
  bool IsFirstInstrProcessed = false;

  void flushPendingLabels(MCStreamer &Out);
  void flushBuffer(MCStreamer &Out, const MCSubtargetInfo &STI,
                   bool FlushAll = false);
  void flushInstCompressed(MCInst &Inst, MCStreamer &Out,
                           const MCSubtargetInfo &STI);
  void flushInstUncompressed(MCInst &Inst, MCStreamer &Out,
                           const MCSubtargetInfo &STI);

  bool isNopMCInst(const MCInst &MI) const;
  bool isVpuInstrWithSrcCD(const MCInst &MI) const;
  bool isVPUInstWithVPUExtSwitch(const MCInst &MI) const;
  bool isSrcCIsStoreSrcC(const MCInst &MI) const;
  bool isLoopMCInst(const MCInst &MI, StringRef &Label) const;
  bool isJmpMCInst(const MCInst &MI, StringRef &Label) const;
  bool isJmpLabel(const MCSymbol *Label) const;
  void flushLoopsInsts(std::vector<Any>::iterator &Iter, MCStreamer &Out,
                       const MCSubtargetInfo &STI);
  void rmOpcodeFromBundle(MCInst &MI, unsigned opcode) const;
  bool maybeCompressInstr(MCInst &MI, bool doCompress,
                          const MCSubtargetInfo &STI) const;
};
}

#endif // LLVM_LIB_TARGET_TPC_ASMPARSER_TPCASMINSTCOMPRESS_H
