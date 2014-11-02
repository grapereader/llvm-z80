//===-- Z80ISelLowering.h - X86 DAG Lowering Interface ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that Z80 uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef Z80ISELLOWERING_H
#define Z80ISELLOWERING_H

#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/Target/TargetLowering.h"

namespace llvm {
  class Z80TargetMachine;

  namespace Z80ISD {
    enum NodeType {
      FIRST_NUMBER = ISD::BUILTIN_OP_END,
      WRAPPER,
      SCF, CCF,
      RLC, RRC, RL, RR, SLA, SRA, SLL, SRL,
      SHL, LSHR, ASHR,
      CP,
      SELECT_CC,
      BR_CC,
      CALL, RET
    }; // end NodeType
  } // end namespace Z80ISD

  class Z80TargetLowering : public TargetLowering {
  public:
    explicit Z80TargetLowering(Z80TargetMachine &TM);

    // LowerOperation - Provide custom lowering hooks for some operations.
    virtual SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const;

    // getTargetNodeName - This method returns the name of a target specific
    // DAG node.
    virtual const char *getTargetNodeName(unsigned Opcode) const;

    // getScalarShiftAmountTy - Return the value type to use for amount in shifts
    virtual MVT getScalarShiftAmountTy(EVT LHSTy) const { return MVT::i8; }

    SDValue LowerZExt(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerSExt(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerSUB(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerShifts(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerBinaryOp(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerSelectCC(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerBrCC(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerStore(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerLoad(SDValue Op, SelectionDAG &DAG) const;

    MachineBasicBlock* EmitInstrWithCustomInserter(MachineInstr *MI,
      MachineBasicBlock *MBB) const;
    MachineBasicBlock* EmitSelectInstr(MachineInstr *MI,
      MachineBasicBlock *MBB) const;
    MachineBasicBlock* EmitShiftInstr(MachineInstr *MI,
      MachineBasicBlock *MBB) const;

	// Inline asm support
	ConstraintType getConstraintType(const std::string &Constraint) const override;
	ConstraintWeight getSingleConstraintMatchWeight(AsmOperandInfo &info, const char *constraint) const override;
	std::pair<unsigned, const TargetRegisterClass*> parseRegForInlineAsmConstraint(const StringRef &C, MVT VT) const;
	std::pair<unsigned, const TargetRegisterClass*> getRegForInlineAsmConstraint(const std::string &Constraint, MVT VT) const override;
	void LowerAsmOperandForConstraint(SDValue Op, std::string &Constraint, std::vector<SDValue> &Ops, SelectionDAG &DAG) const override;

	bool isLegalAddressingMode(const AddrMode &AM, Type *Ty) const override;
  private:
    SDValue
      LowerCallResult(SDValue Chain, SDValue Flag,
        CallingConv::ID CallConv, bool isVarArg,
        const SmallVectorImpl<ISD::InputArg> &Ins,
        SDLoc dl, SelectionDAG &DAG,
        SmallVectorImpl<SDValue> &InVals) const;
    virtual SDValue
      LowerFormalArguments(SDValue Chain,
        CallingConv::ID CallConv, bool isVarArg,
        const SmallVectorImpl<ISD::InputArg> &Ins,
        SDLoc dl, SelectionDAG &DAG,
        SmallVectorImpl<SDValue> &InVals) const;
    virtual SDValue
      LowerReturn(SDValue Chain,
        CallingConv::ID CallConv, bool isVarArg,
        const SmallVectorImpl<ISD::OutputArg> &Outs,
        const SmallVectorImpl<SDValue> &OutVals,
        SDLoc dl, SelectionDAG &DAG) const;
    virtual SDValue
      LowerCall(TargetLowering::CallLoweringInfo &CLI,
        SmallVectorImpl<SDValue> &InVals) const;

    // Emit nodes that will be selected as "cp Op0, Op1", or something
    // equivalent, for use with given LLVM condition code and return
    // equivalent Z80 condition code.
    SDValue EmitCMP(SDValue &LHS, SDValue &RHS, SDValue &Z80CC,
      ISD::CondCode CC, SDLoc dl, SelectionDAG &DAG) const;
  }; // end class Z80TargetLowering
} // end namespace llvm

#endif
