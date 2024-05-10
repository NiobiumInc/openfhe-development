#pragma once

#include "constants.h"
#include "symbolic.h"

enum SSAInstOp {
  TODO,
  ADD,
  ADDI,
  SUB,
  SUBI,
  MUL,
  MULI,
  ADDMULI,
  NTT,
  INTT,
  MORPH,
  FREE,
  SWITCHMODULUS,
};

struct SSAInst {
    SSAInst(SSAInstOp op, SymbolicValue const& arg1, SymbolicValue const& arg2, NativeInteger const& m);
    SSAInst(SSAInstOp op, SymbolicValue const& arg, NativeInteger const& i, NativeInteger const& m);
    SSAInst(SSAInstOp op, SymbolicValue const& arg1, SymbolicValue const& arg2, NativeInteger const& i, NativeInteger const& m);
    SSAInst(SSAInstOp op, SymbolicValue const& arg, NativeInteger const& m);
    SSAInst(SSAInstOp op, SymbolicValue const& arg, AutomorphismNumber n, NativeInteger const& m);
    SSAInst(SSAInstOp op, SymbolicValue const& arg);
    SSAInst(SSAInstOp op, ValueId const& arg)
    : op { op }, arg1 { arg } {

    }

    SSAInstOp op = SSAInstOp::TODO;
    ValueId dest = UNDEF_VALUE_ID;
    ValueId arg1 = UNDEF_VALUE_ID;
    ValueId arg2 = UNDEF_VALUE_ID; 
    NativeInteger imm = 0; // immediate or or AutomorphismNumber if it is MORPH
    PrimeModulusIndex modulus = 0;
};


