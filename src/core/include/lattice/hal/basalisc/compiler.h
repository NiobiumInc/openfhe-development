#ifndef LBCRYPTO_INC_LATTICE_HAL_BASALISC_COMPILER_H
#define LBCRYPTO_INC_LATTICE_HAL_BASALISC_COMPILER_H

#include<variant>
#include<stdint.h>
#include<assert.h>
#include<optional>
#include"utils/exception.h"
#include"nativeintbackend.h"
#include"lattice/hal/default/poly.h"

namespace lbcrypto {

using Register = uint64_t;
using Address = uint64_t;
using Immediate = int64_t;
using PrimeModulusIndex = uint8_t;
using Instruction = uint64_t;
using AutomorphismNumber = uint64_t;



// Encoding for BASALISC instructions
struct Instruction {
  enum Opcode {
    LOAD = 1,
    STORE = 2,
    ADD = 3,
    ADDI = 4,
    SUB = 5,
    SUBI = 6,
    MUL = 7,
    MULI = 8,
    ADDMULI = 9,
    MORPH1 = 10,
    MORPH2 = 11,
    NTT1 = 12,
    NTT2 = 13,
    INTT1 = 14,
    INTT2 = 15,
    FENCE = 16,
    RNGCONFIG = 17,
    RNGSETUP = 18,
    RNGGENERATE = 19,
  };

  Instruction() {
    encoded = 0;
  }

  // opcode
  Instruction& opcode(Opcode op) {
    setbits(0, 4, op);
    return *this;
  }

  // destination register
  Instruction& rd(Register reg) {
    setbits(5, 11, reg);
    return *this;
  }

  // first register operand
  Instruction& rs1(Register reg) {
    setbits(12, 18, reg);
    return *this;
  }

  // second register operand
  Instruction& rs2(Register reg) {
    setbits(19, 25, reg);
    return *this;
  }

  // modulus index
  Instruction& mod_index(PrimeModulusIndex idx) {
    setbits(26, 30, static_cast<uint64_t>(idx));
    return *this;
  }

  // automorphism number
  Instruction& automorphism(uint64_t anum) {
    anum >>= 1;
    setbits(19, 33, anum);
    return *this;
  }

  // address for LOAD instructions
  Instruction& load_addr(uint64_t addr) {
    setbits(12, 63, addr);
    return *this;
  }

  // address for STORE instructions
  Instruction& store_addr(uint64_t addr) {
    setbits(19, 63, addr);
    return *this;
  }

  Instruction& immediate(Immediate i) {
    imm = i;
    return *this;
  }

  uint64_t encoded;
  std::optional<Immediate> imm;

private:
  uint64_t setbits(uint64_t initial, uint64_t lo, uint64_t hi, uint64_t val) {
    uint64_t mask = (UINT64_MAX >> (UINT64_WIDTH - hi - 1)) & (UINT64_MAX << lo);
    return (initial & ~mask) | ((val << lo) & mask);
  }
};

// Program that does nothing
class ProgramNull {
  inline uint64_t PolyValues(NativeVector& v, uint64_t modulus) {

  }

  inline uint64_t Add(uint64_t a1, uint64_t a2, uint64_t modulus) {
    return 0;
  }

  inline uint64_t AddI(uint64_t a1, uint64_t a2, Immediate i, uint64_t modulus) {
    return 0;
  }

  inline uint64_t Sub(uint64_t a1, uint64_t a2, uint64_t modulus) {
    return 0;
  }

  inline uint64_t SubI(uint64_t a1, uint64_t a2, Immediate i, uint64_t modulus) {
    return 0;
  }

  inline uint64_t AddMulI(uint64_t a1, uint64_t a2, Immediate i, uint64_t modulus) {
    return 0;
  }

  inline uint64_t Morph(uint64_t a1, AutomorphismNumber n, uint64_t modulus) {
    return 0;
  }

  inline uint64_t NTT(uint64_t a1, uint64_t modulus) {
    return 0;
  }

  inline uint64_t INTT(uint64_t a1, uint64_t modulus) {
    return 0;
  }

  // inform program that an address is being used
  inline void increment_refcount(uint64_t a) {

  }

  // inform program that an address is no longer live
  inline void decrement_refcount(uint64_t a) {

  }

  // force computation
  inline void force() {
    OPENFHE_THROW(not_implemented_error, "force() called on ProgramNull");
  }
};

// SSA form instructions
struct SSAInst {
  enum Op {
    ADD,
    ADDI,
    SUB,
    SUBI,
    ADDMULI,
    NTT,
    INTT,
    MORPH,
  };

  uint64_t arg1;
  uint64_t arg2;
  uint64_t imm;
  uint64_t modulus;
};


// **************** COMPILER GLOBAL VARIABLE **********************************
inline ProgramNull compiler;

} // namespace lbcrypto

#endif