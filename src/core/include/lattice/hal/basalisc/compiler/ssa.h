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

private:
    void display_val(std::ostream& os, ValueId x) { os << "x" << x; }
    void display_mod(std::ostream& os) {
      os << " mod " << uint64_t{modulus};
    }
    void display_assign(std::ostream& os) {
      display_val(os, dest); os << " = ";
    }

    void display_bin(std::ostream& os, const char* op) {
      display_assign(os);
      os << op;
      display_val(os, arg1);
      os << " ";
      display_val(os, arg2);
      display_mod(os);
    }

    void display_bin_imm(std::ostream& os, const char* op) {
      display_assign(os);
      os << op;
      display_val(os, arg1);
      os << " ";
      os << imm;
      display_mod(os);
    }


public:
    void display(std::ostream& os = std::cout) {
      switch (op) {
        case TODO:    os << "TODO"; break;
        case ADD:     display_bin(os,"ADD "); break;
        case ADDI:    display_bin_imm(os,"ADDI "); break;
        case SUB:     display_bin(os,"SUB "); break;
        case SUBI:    display_bin_imm(os,"SUBI "); break;
        case MUL:     display_bin(os,"MUL "); break;
        case MULI:    display_bin_imm(os,"MULI "); break;
        case ADDMULI:
          display_assign(os);
          os << "ADDMULI ";
          display_val(os,arg1);
          os << " ";
          display_val(os,arg2);
          os << " " << imm;
          display_mod(os);
          break;

        case NTT:
          display_assign(os);
          os << "NTT ";
          display_val(os,arg1);
          display_mod(os);
          break;

        case INTT:
          display_assign(os);
          os << "NTT ";
          display_val(os,arg1);
          display_mod(os);
          break;

        case MORPH:
          display_assign(os);
          os << "MORPH " << uint64_t{imm} << " ";
          display_val(os,arg1);
          break;

        case FREE:
          os << "FREE "; display_val(os,arg1);
          break;
      }
    }
};


