//===-- Z80AsmPrinter.h - Z80 LLVM Assembly Printer -------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Z80 assembly code printer class.
//
//===----------------------------------------------------------------------===//

#ifndef Z80ASMPRINTER_H
#define Z80ASMPRINTER_H

#include "Z80TargetMachine.h"
#include "../Z80/InstPrinter/Z80InstPrinter.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCStreamer.h"

namespace llvm {

  class Z80AsmPrinter : public AsmPrinter {
  public:
    explicit Z80AsmPrinter(TargetMachine &TM, MCStreamer &Streamer)
		: AsmPrinter(TM, Streamer) {
			OutStreamer.setAutoInitSections(false);
	}
    virtual const char *getPassName() const {
      return "Z80 Assembly Printer";
    }
	bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo, unsigned AsmVariant, const char *ExtraCode, raw_ostream &O);
	bool PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNo, unsigned AsmVariant, const char *ExtraCode, raw_ostream &O);
	void printOperand(const MachineInstr *MI, int opNum, raw_ostream &O);

	void EmitGlobalVariable(const GlobalVariable *GV);
	void EmitFunctionEntryLabel();
	void EmitFunctionHeader();
	void EmitFunctionBodyEnd();

	bool doInitialization(Module &M);
	bool doFinalization(Module &M);

	void emitHeader(Module &M, raw_ostream &os);

	void EmitInstruction(const MachineInstr *MI);
  }; // end class Z80AsmPrinter
} // end namespace llvm

#endif
