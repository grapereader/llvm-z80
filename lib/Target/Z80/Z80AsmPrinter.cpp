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
#include "llvm/Support/TargetRegistry.h"
#include "llvm/MC/MCSection.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/Target/Mangler.h"


using namespace llvm;

void Z80AsmPrinter::EmitInstruction(const MachineInstr *MI)
{
  Z80MCInstLower MCInstLowering(Mang, *MF, *this);

  MCInst TmpInst;
  MCInstLowering.Lower(MI, TmpInst);
  OutStreamer.EmitInstruction(TmpInst);
}

//
// Butcher the hell out of the AsmPrinter output so Z80 assemblers
// can parse it directly.
//

bool Z80AsmPrinter::doInitialization(Module &M)
{
	OutStreamer.setAutoInitSections(false);
	SmallString<128> Str1;
	raw_svector_ostream OS1(Str1);

	MMI = getAnalysisIfAvailable<MachineModuleInfo>();
	MMI->AnalyzeModule(M);

	// We need to call the parent's one explicitly.
	// No we don't.
	//bool Result = AsmPrinter::doInitialization(M);

	// Initialize TargetLoweringObjectFile.
	const_cast<TargetLoweringObjectFile &>(getObjFileLowering()).Initialize(OutContext, TM);

	Mang = new Mangler(&TM);

	// Emit header before any dwarf directives are emitted below.
	emitHeader(M, OS1);
	OutStreamer.EmitRawText(OS1.str());

	// Emit module-level inline asm if it exists.
	if (!M.getModuleInlineAsm().empty()) {
		OutStreamer.AddComment("Start of file scope inline assembly");
		OutStreamer.AddBlankLine();
		OutStreamer.EmitRawText(StringRef(M.getModuleInlineAsm()));
		OutStreamer.AddBlankLine();
		OutStreamer.AddComment("End of file scope inline assembly");
		OutStreamer.AddBlankLine();
	}
    
	return false; //Apparently this is success... Fantastic.
}

void Z80AsmPrinter::emitHeader(Module &M, raw_ostream &os)
{
	//Because why not
	os << "; Compiled using LLVM Z80 backend\n";
}

bool Z80AsmPrinter::doFinalization(Module &M)
{
	bool res = AsmPrinter::doFinalization(M);
	//If I ever need anything at the end of the output...
	return res;
}

void Z80AsmPrinter::EmitFunctionEntryLabel()
{
	//OutStreamer.EmitRawText(".global");
	AsmPrinter::EmitFunctionEntryLabel();
}

void Z80AsmPrinter::EmitFunctionHeader() {
  // Print out constants referenced by the function
  EmitConstantPool();

  // Print the 'header' of function.
  const Function *F = MF->getFunction();

  OutStreamer.SwitchSectionNoChange(getObjFileLowering().SectionForGlobal(F, Mang, TM));

  // Emit the CurrentFnSym.
  EmitFunctionEntryLabel();

  // Emit the prefix data.
  if (F->hasPrefixData())
    EmitGlobalConstant(F->getPrefixData());
}

void Z80AsmPrinter::EmitFunctionBodyEnd()
{
	AsmPrinter::EmitFunctionBodyEnd();
	//OutStreamer.EmitRawText(".endglobal");
}

void Z80AsmPrinter::EmitGlobalVariable(const GlobalVariable *GV)
{
	if (GV->hasInitializer()) {
		// Check to see if this is a special global used by LLVM, if so, IGNORE IT.
		if (   GV->getName() == "llvm.used"
			|| GV->getSection() == "llvm.metadata"
			|| GV->hasAvailableExternallyLinkage()
			|| GV->getName() == "llvm.global_ctors"
			|| GV->getName() == "llvm.global_dtors") return;
	}

	MCSymbol *GVSym = getSymbol(GV);

	if (!GV->hasInitializer()) return;

	SectionKind GVKind = TargetLoweringObjectFile::getKindForGlobal(GV, TM);

	const MCSection *TheSection = getObjFileLowering().SectionForGlobal(GV, GVKind, Mang, TM);

	OutStreamer.SwitchSectionNoChange(TheSection);

	OutStreamer.EmitLabel(GVSym);
	EmitGlobalConstant(GV->getInitializer());
	OutStreamer.AddBlankLine();
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
