#ifndef LBCRYPTO_INC_LATTICE_HAL_BASALISC_COMPILER_H
#define LBCRYPTO_INC_LATTICE_HAL_BASALISC_COMPILER_H

#include<variant>
#include<stdint.h>
#include<assert.h>
#include<optional>
#include"utils/exception.h"
#include"math/hal/nativeintbackend.h"
#include"lattice/hal/default/poly.h"

namespace lbcrypto {

using Register = uint64_t;
using Address = uint64_t;
using Immediate = int64_t;
using PrimeModulusIndex = uint8_t;
using AutomorphismNumber = uint64_t;
using RngConfig = uint64_t;

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
    imm = 0;
  }

  void into_load(Register dest, Address src_addr) {
    opcode(LOAD).rd(dest).load_addr(src_addr);
  }

  void into_store(Address dest_addr, Register src) {
    opcode(STORE).rs1(src).store_addr(dest_addr);
  }

  void into_add(Register dest, Register src1, Register src2, PrimeModulusIndex m) {
    rtype(ADD, dest, src1, src2, m);
  }

  void into_addi(Register dest, Register src, Immediate i, PrimeModulusIndex m) {
    rtype(ADDI, dest, src, 0, m).immediate(i);
  }

  void into_sub(Register dest, Register src1, Register src2, PrimeModulusIndex m) {
    rtype(SUB, dest, src1, src2, m);
  }

  void into_subi(Register dest, Register src, Immediate i, PrimeModulusIndex m) {
    rtype(SUBI, dest, src, 0, m).immediate(i);
  }

  void into_mul(Register dest, Register src1, Register src2, PrimeModulusIndex m) {
    rtype(MUL, dest, src1, src2, m);
  }

  void into_muli(Register dest, Register src, Immediate i, PrimeModulusIndex m) {
    rtype(MULI, dest, src, 0, m).immediate(i);
  }

  void into_addmuli(Register dest, Register src1, Register src2, Immediate i, PrimeModulusIndex m) {
    rtype(ADDMULI, dest, src1, src2, m).immediate(i);
  }

  void into_morph1(Register dest, Register src, AutomorphismNumber num) {
    opcode(MORPH1).rd(dest).rs1(src).automorphism(num);
  }

  void into_morph2(Register dest, Register src, AutomorphismNumber num) {
    opcode(MORPH2).rd(dest).rs1(src).automorphism(num);
  }

  void into_ntt1(Register dest, Register src, PrimeModulusIndex m) {
    rtype(NTT1, dest, src, 0, m);
  }

  void into_ntt2(Register dest, Register src, PrimeModulusIndex m) {
    rtype(NTT2, dest, src, 0, m);
  }

  void into_intt1(Register dest, Register src, PrimeModulusIndex m) {
    rtype(INTT1, dest, src, 0, m);
  }

  void into_intt2(Register dest, Register src, PrimeModulusIndex m) {
    rtype(INTT2, dest, src, 0, m);
  }

  void into_fence() {
    opcode(FENCE);
  }

  void into_rngconfig(RngConfig dest, Immediate i, PrimeModulusIndex idx) {
    opcode(RNGCONFIG).rd(dest).immediate(i).mod_index(idx);
  }

  void into_rngsetup(PrimeModulusIndex idx) {
    opcode(RNGSETUP).mod_index(idx);
  }

  void into_rnggenerate(Register dest) {
    opcode(RNGGENERATE).rd(dest);
  }


private:
  Instruction& rtype(Opcode op, Register dest, Register src1, Register src2, PrimeModulusIndex midx) {
    opcode(op).rd(dest).rs1(src1).rs2(src2).mod_index(midx);
    return *this;
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
  Instruction& automorphism(AutomorphismNumber anum) {
    setbits(19, 33, anum >> 1);
    return *this;
  }

  // address for LOAD instructions
  Instruction& load_addr(uint64_t addr) {
    setbits(12, 63, addr);
    return *this;
  }

  // address for STORE instructions
  Instruction& store_addr(uint64_t addr) {
    rd(addr);
    setbits(19, 63, addr << 7);
    return *this;
  }

  Instruction& immediate(Immediate i) {
    imm = i;
    return *this;
  }

  inline uint64_t compute_mask(uint64_t lo, uint64_t hi) {
    return (UINT64_MAX >> (UINT64_WIDTH - hi - 1)) & (UINT64_MAX << lo);
  }

  void setbits(uint64_t lo, uint64_t hi, uint64_t val) {
    uint64_t mask = compute_mask(lo, hi);
    encoded = (encoded & ~mask) | ((val << lo) & mask);
  }

  uint64_t getbits(uint64_t lo, uint64_t hi) {
    return (encoded & compute_mask(lo, hi)) >> lo;
  }

  uint64_t encoded;
  uint64_t imm;       
};

using ValueId = uint64_t;

// Program that does nothing
class ProgramNull {
public:
  using SymbolicValue = uint64_t;

  inline SymbolicValue PolyValues(NativeVector& v, NativeInteger modulus) {
    return 0;
  }

  inline SymbolicValue Add(SymbolicValue a1, SymbolicValue a2, NativeInteger modulus) {
    return 0;
  }

  inline SymbolicValue AddI(SymbolicValue a1, SymbolicValue a2, Immediate i, NativeInteger modulus) {
    return 0;
  }

  inline SymbolicValue Sub(SymbolicValue a1, SymbolicValue a2, NativeInteger modulus) {
    return 0;
  }

  inline SymbolicValue SubI(SymbolicValue a1, SymbolicValue a2, Immediate i, NativeInteger modulus) {
    return 0;
  }

  inline SymbolicValue AddMulI(SymbolicValue a1, SymbolicValue a2, Immediate i, NativeInteger modulus) {
    return 0;
  }

  inline SymbolicValue Morph(SymbolicValue a1, AutomorphismNumber n, NativeInteger modulus) {
    return 0;
  }

  inline SymbolicValue NTT(SymbolicValue a1, NativeInteger modulus) {
    return 0;
  }

  inline SymbolicValue INTT(SymbolicValue a1, NativeInteger modulus) {
    return 0;
  }

  inline void increment_refcount(ValueId val) {

  }

  inline void decrement_refcount(ValueId val) {

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
inline ProgramNull Basalisc;

// struct SymbolicValue {
//   SymbolicValue(uint64_t value): value {value} {
//     Basalisc.increment_refcount(value);
//   }

//   SymbolicValue(SymbolicValue const& s) {
//     Basalisc.increment_refcount(s.value);
//     value = s.value;
//   }

//   SymbolicValue(SymbolicValue&& s) noexcept {
//     value = s.value;
//   }

//   SymbolicValue& operator=(SymbolicValue const& other)
//   {
//     return *this = SymbolicValue(other);
//   }
 
//   SymbolicValue& operator=(SymbolicValue&& other) noexcept
//   {
//     value = other.value;
//     return *this;
//   }

//   ~SymbolicValue() {
//     Basalisc.decrement_refcount(value);
//   }

//   ValueId value;
// };


} // namespace lbcrypto

#endif