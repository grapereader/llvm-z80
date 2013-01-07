//===-- Z80ISelLowering.cpp - X86 DAG Lowering Implementation -------------===//
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

#include "Z80ISelLowering.h"
#include "Z80.h"
#include "Z80TargetMachine.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

Z80TargetLowering::Z80TargetLowering(Z80TargetMachine &TM)
  : TargetLowering(TM, new TargetLoweringObjectFileELF())
{
  addRegisterClass(MVT::i8, &Z80::GR8RegClass);
  addRegisterClass(MVT::i16, &Z80::GR16RegClass);

  computeRegisterProperties();

  setOperationAction(ISD::ZERO_EXTEND, MVT::i16, Custom);
  setOperationAction(ISD::SIGN_EXTEND, MVT::i16, Custom);
}

//===----------------------------------------------------------------------===//
//                      Calling Convention Implementation
//===----------------------------------------------------------------------===//

#include "Z80GenCallingConv.inc"

SDValue Z80TargetLowering::LowerFormalArguments(SDValue Chain,
  CallingConv::ID CallConv, bool isVarArg,
  const SmallVectorImpl<ISD::InputArg> &Ins,
  DebugLoc dl, SelectionDAG &DAG,
  SmallVectorImpl<SDValue> &InVals) const
{
  MachineFunction &MF = DAG.getMachineFunction();
  MachineRegisterInfo &MRI = MF.getRegInfo();

  // CCValAssign - represent the assignment of
  // the arguments to a location
  SmallVector<CCValAssign, 16> ArgLocs;

  // CCState - info about the registers and stack slot.
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
    getTargetMachine(), ArgLocs, *DAG.getContext());

  // Analyze Formal Arguments
  CCInfo.AnalyzeFormalArguments(Ins, CC_Z80);

  assert(!isVarArg && "Varargs not supported yet!");

  for (unsigned i = 0, e = ArgLocs.size(); i != e; i++)
  {
    SDValue ArgValue;
    unsigned VReg;

    CCValAssign &VA = ArgLocs[i];
    if (VA.isRegLoc())
    { // Argument passed in registers
      EVT RegVT = VA.getLocVT();
      switch (RegVT.getSimpleVT().SimpleTy)
      {
      default:
        {
#ifndef NDEBUG
          errs() << "LowerFormalArguments Unhandled argument type: "
            << RegVT.getSimpleVT().SimpleTy << "\n";
#endif
          llvm_unreachable(0);
        }
      case MVT::i8:
        VReg = MRI.createVirtualRegister(&Z80::GR8RegClass);
        MRI.addLiveIn(VA.getLocReg(), VReg);
        ArgValue = DAG.getCopyFromReg(Chain, dl, VReg, RegVT);
        InVals.push_back(ArgValue);
        break;
      case MVT::i16:
        VReg = MRI.createVirtualRegister(&Z80::GR16RegClass);
        MRI.addLiveIn(VA.getLocReg(), VReg);
        ArgValue = DAG.getCopyFromReg(Chain, dl, VReg, RegVT);
        InVals.push_back(ArgValue);
        break;
      }
    }
    else assert(0 && "Not implemented yet!");
  }
  return Chain;
}

SDValue Z80TargetLowering::LowerReturn(SDValue Chain,
  CallingConv::ID CallConv, bool isVarArg,
  const SmallVectorImpl<ISD::OutputArg> &Outs,
  const SmallVectorImpl<SDValue> &OutVals,
  DebugLoc dl, SelectionDAG &DAG) const
{
  // CCValAssign - represent the assignment of
  // the return value to a location
  SmallVector<CCValAssign, 16> RVLocs;

  // CCState - Info about the registers and stack slot.
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
    getTargetMachine(), RVLocs, *DAG.getContext());

  // Analyze return values.
  CCInfo.AnalyzeReturn(Outs, RetCC_Z80);

  // If this is the first return lowered for this function, add
  // the regs to the liveout set for the function.
  if (DAG.getMachineFunction().getRegInfo().liveout_empty()) {
    for (unsigned i = 0; i != RVLocs.size(); i++)
      if (RVLocs[i].isRegLoc())
        DAG.getMachineFunction().getRegInfo().addLiveOut(RVLocs[i].getLocReg());
  }

  SDValue Flag;

  // Copy the result value into the output registers.
  for (unsigned i = 0; i != RVLocs.size(); i++)
  {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");

    Chain = DAG.getCopyToReg(Chain, dl, VA.getLocReg(), OutVals[i], Flag);

    // Guarantee the all emitted copies are stuck together,
    // avoiding something bad
    Flag = Chain.getValue(1);
  }
  if (Flag.getNode())
    return DAG.getNode(Z80ISD::RET, dl, MVT::Other, Chain, Flag);
  return DAG.getNode(Z80ISD::RET, dl, MVT::Other, Chain);
}

SDValue Z80TargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const
{
  switch (Op.getOpcode())
  {
  case ISD::ZERO_EXTEND: return LowerZExt(Op, DAG);
  case ISD::SIGN_EXTEND: return LowerSExt(Op, DAG);
  default:
    llvm_unreachable("unimplemented operation");
  }
}

const char *Z80TargetLowering::getTargetNodeName(unsigned Opcode) const
{
  switch (Opcode)
  {
  default: return NULL;
  case Z80ISD::RET: return "Z80ISD::RET";
  }
}

SDValue Z80TargetLowering::LowerZExt(SDValue Op, SelectionDAG &DAG) const
{
  DebugLoc dl = Op.getDebugLoc();
  SDValue Val = Op.getOperand(0);
  EVT VT      = Op.getValueType();

  assert(VT == MVT::i16 && "ZExt support only i16");

  SDValue Tmp = DAG.getConstant(0, VT.getHalfSizedIntegerVT(*DAG.getContext()));
  SDValue HI  = DAG.getTargetInsertSubreg(Z80::subreg_hi, dl, VT, DAG.getUNDEF(VT), Tmp);
  SDValue LO  = DAG.getTargetInsertSubreg(Z80::subreg_lo, dl, VT, HI, Val);
  return LO;
}

SDValue Z80TargetLowering::LowerSExt(SDValue Op, SelectionDAG &DAG) const
{
  DebugLoc dl = Op.getDebugLoc();
  SDValue Val = Op.getOperand(0);
  EVT VT      = Op.getValueType();
  EVT HalfVT  = VT.getHalfSizedIntegerVT(*DAG.getContext());

  assert(VT == MVT::i16 && "SExt support only i16");

  // Generating the next code:
  // LD L,A
  // ADD A,L - set carry flag if negative (7 bit is set)
  // SBC A,A - turn to -1 if carry flag set
  // LD H,A
  SDValue LO   = DAG.getTargetInsertSubreg(Z80::subreg_lo, dl, VT, DAG.getUNDEF(VT), Val);
  SDValue Add  = DAG.getNode(ISD::ADDC, dl, DAG.getVTList(HalfVT, MVT::Glue), Val, Val);
  SDValue Flag = Add.getValue(1);
  SDValue Sub  = DAG.getNode(ISD::SUBE, dl, HalfVT, Add, Add, Flag);
  SDValue HI   = DAG.getTargetInsertSubreg(Z80::subreg_hi, dl, VT, LO, Sub);
  return HI;
}
