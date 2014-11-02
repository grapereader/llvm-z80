//===-- Z80AsmPrinter.cpp - Z80 LLVM Assembly Printer ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to Z80 machine code.
//
//===----------------------------------------------------------------------===//

#include "Z80AsmPrinter.h"
#include "Z80.h"
#include "Z80MCInstLower.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

void Z80AsmPrinter::EmitInstruction(const MachineInstr *MI)
{
  Z80MCInstLower MCInstLowering(Mang, *MF, *this);

  MCInst TmpInst;
  MCInstLowering.Lower(MI, TmpInst);
  OutStreamer.EmitInstruction(TmpInst);
}

//
// INLINE ASM SUPPORT
//

bool Z80AsmPrinter::PrintAsmOperand(const MachineInstr *MI, unsigned OpNo, unsigned AsmVariant, const char *ExtraCode, raw_ostream &O)
{
	if (ExtraCode && ExtraCode[0]) {
		if (ExtraCode[1] != 0) return true;

		const MachineOperand &MO = MI->getOperand(OpNo);

		switch (ExtraCode[0]) {
		default:
			return AsmPrinter::PrintAsmOperand(MI, OpNo, AsmVariant, ExtraCode, O);
		case 'X': //Hex const int
			if ((MO.getType()) != MachineOperand::MO_Immediate) return true;
			O << "$" << std::hex << MO.getImm();
			return false;
		case 'x': //Hex const int (LS 16 bits)
			if ((MO.getType()) != MachineOperand::MO_Immediate)
			return true;
			O << "$" << std::hex << (MO.getImm() & 0xffff);
			return false;
		case 'd': //Decimal const int
			if ((MO.getType()) != MachineOperand::MO_Immediate) return true;
			O << MO.getImm();
			return false;
		case 'm': //Decimal const int minus 1
			if ((MO.getType()) != MachineOperand::MO_Immediate) return true;
			O << MO.getImm() - 1;
			return false;
		case 'z':
			{
				// $0 if zero, regular printing otherwise
				if (MO.getType() != MachineOperand::MO_Immediate) return true;
				int64_t val = MO.getImm();
				if (val) {
					O << val;
				} else {
					O << "$0";
				}
				return false;
			}
		}
	}

	printOperand(MI, OpNo, O);
	return false;
}

bool Z80AsmPrinter::PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNum, unsigned AsmVariant, const char *ExtraCode, raw_ostream &O)
{
	int Offset = 0;

	if (ExtraCode) {
		return true;
	}

	const MachineOperand &MO = MI->getOperand(OpNum);
	assert(MO.isReg() && "unexpected inline asm memory operand");
	O << Offset << "(" << Z80InstPrinter::getRegisterName(MO.getReg()) << ")";

	return false;
}

void Z80AsmPrinter::printOperand(const MachineInstr *MI, int opNum, raw_ostream &O)
{
	const MachineOperand &MO = MI->getOperand(opNum);
	bool closeP = false;

	if (MO.getTargetFlags()) closeP = true;
	/* CPU0 remnants. I have no idea if these are applicable to Z80 at all, so that's a TODO.
	switch(MO.getTargetFlags()) {
		case Cpu0II::MO_GPREL:    O << "%gp_rel("; break;
		case Cpu0II::MO_GOT_CALL: O << "%call16("; break;
		case Cpu0II::MO_GOT16:    O << "%got16(";  break;
		case Cpu0II::MO_GOT:      O << "%got(";    break;
		case Cpu0II::MO_ABS_HI:   O << "%hi(";     break;
		case Cpu0II::MO_ABS_LO:   O << "%lo(";     break;
		case Cpu0II::MO_GOT_HI16: O << "%got_hi16("; break;
		case Cpu0II::MO_GOT_LO16: O << "%got_lo16("; break;
	}
	*/

	switch (MO.getType()) {
	case MachineOperand::MO_Register:
		O << StringRef(Z80InstPrinter::getRegisterName(MO.getReg())).lower();
		break;

	case MachineOperand::MO_Immediate:
		O << MO.getImm();
		break;

	case MachineOperand::MO_MachineBasicBlock:
		O << *MO.getMBB()->getSymbol();
		return;

	case MachineOperand::MO_GlobalAddress:
		O << *getSymbol(MO.getGlobal());
		break;

	case MachineOperand::MO_BlockAddress: {
		MCSymbol *BA = GetBlockAddressSymbol(MO.getBlockAddress());
		O << BA->getName();
		break;
	}

	case MachineOperand::MO_ExternalSymbol:
		O << *GetExternalSymbolSymbol(MO.getSymbolName());
		break;

	case MachineOperand::MO_JumpTableIndex:
		O << MAI->getPrivateGlobalPrefix() << "JTI" << getFunctionNumber() << '_' << MO.getIndex();
		break;

	case MachineOperand::MO_ConstantPoolIndex:
		O << MAI->getPrivateGlobalPrefix() << "CPI" << getFunctionNumber() << "_" << MO.getIndex();
		if (MO.getOffset()) O << "+" << MO.getOffset();
		break;

	default:
		llvm_unreachable("<unknown operand type>");
	}

	if (closeP) O << ")";
}

//===----------------------------------------------------------------------===//
// Target Registry Stuff
//===----------------------------------------------------------------------===//

extern "C" void LLVMInitializeZ80AsmPrinter()
{
  RegisterAsmPrinter<Z80AsmPrinter> X(TheZ80Target);
}
