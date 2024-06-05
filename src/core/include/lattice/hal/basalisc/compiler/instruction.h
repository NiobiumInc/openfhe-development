#pragma once

#include <cstdint>
#include "constants.h"

template<uint64_t LO, uint64_t HI> 
struct Bits {
  // XXX: should we use a literal 64 here?
  const uint64_t MASK = (UINT64_MAX >> (64 - HI - 1)) & (UINT64_MAX << LO);

  void set(uint64_t& dest, uint64_t val) const {
    dest = (dest & ~MASK) | ((val << LO) & MASK);
  }

  uint64_t get(uint64_t const& src) const {
    return (src & MASK) >> LO;
  }
};

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

  Instruction(uint64_t val) {
    encoded = val;
  }

  Instruction& into_load(Register dest, Address src_addr) {
    return opcode(LOAD).rd(dest).load_addr(src_addr);
  }

  Instruction& into_store(Address dest_addr, Register src) {
    return opcode(STORE).rs1(src).store_addr(dest_addr);
  }

  Instruction& into_add(Register dest, Register src1, Register src2, PrimeModulusIndex m) {
    return rtype(ADD, dest, src1, src2, m);
  }

  Instruction& into_addi(Register dest, Register src, PrimeModulusIndex m) {
    return rtype(ADDI, dest, src, 0, m);
  }

  Instruction& into_sub(Register dest, Register src1, Register src2, PrimeModulusIndex m) {
    return rtype(SUB, dest, src1, src2, m);
  }

  Instruction& into_subi(Register dest, Register src, PrimeModulusIndex m) {
    return rtype(SUBI, dest, src, 0, m);
  }

  Instruction& into_mul(Register dest, Register src1, Register src2, PrimeModulusIndex m) {
    return rtype(MUL, dest, src1, src2, m);
  }

  Instruction& into_muli(Register dest, Register src, PrimeModulusIndex m) {
    return rtype(MULI, dest, src, 0, m);
  }

  Instruction& into_addmuli(Register dest, Register src1, Register src2, PrimeModulusIndex m) {
    return rtype(ADDMULI, dest, src1, src2, m);
  }

  Instruction& into_morph1(Register dest, Register src, AutomorphismNumber num) {
    return opcode(MORPH1).rd(dest).rs1(src).automorphism(num);
  }

  Instruction& into_morph2(Register dest, Register src, AutomorphismNumber num) {
    return opcode(MORPH2).rd(dest).rs1(src).automorphism(num);
  }

  Instruction& into_ntt1(Register dest, Register src, PrimeModulusIndex m) {
    return rtype(NTT1, dest, src, 0, m);
  }

  Instruction& into_ntt2(Register dest, Register src, PrimeModulusIndex m) {
    return rtype(NTT2, dest, src, 0, m);
  }

  Instruction& into_intt1(Register dest, Register src, PrimeModulusIndex m) {
    return rtype(INTT1, dest, src, 0, m);
  }

  Instruction& into_intt2(Register dest, Register src, PrimeModulusIndex m) {
    return rtype(INTT2, dest, src, 0, m);
  }

  Instruction& into_fence() {
    return opcode(FENCE);
  }

  Instruction& into_rngconfig(RngConfig dest, PrimeModulusIndex idx) {
    return opcode(RNGCONFIG).rd(dest).mod_index(idx);
  }

  Instruction& into_rngsetup(PrimeModulusIndex idx) {
    return opcode(RNGSETUP).mod_index(idx);
  }

  Instruction& into_rnggenerate(Register dest) {
    return opcode(RNGGENERATE).rd(dest);
  }

  Opcode opcode() const {
    return static_cast<Opcode>(OPCODE.get(encoded));
  }

  Register rd() const {
    return RD.get(encoded);
  }

  Register rs1() const {
    return RS1.get(encoded);
  }

  Register rs2() {
    return RS2.get(encoded);
  }

  PrimeModulusIndex mod_index() {
    return static_cast<uint8_t>(MOD_INDEX.get(encoded));
  }

  AutomorphismNumber automorphism() {
    return AUTOMORPHISM.get(encoded) << 1;
  }

  Address load_addr() const {
    return LOAD_ADDR.get(encoded);
  }

  Address store_addr() const {
    return STORE_ADDR1.get(encoded) | STORE_ADDR2.get(encoded) << 7;
  }

  Instruction& rtype(Opcode op, Register dest, Register src1, Register src2, PrimeModulusIndex midx) {
    opcode(op).rd(dest).rs1(src1).rs2(src2).mod_index(midx);
    return *this;
  }

  // opcode
  Instruction& opcode(Opcode op) {
    OPCODE.set(encoded, op);
    return *this;
  }

  // destination register
  Instruction& rd(Register reg) {
    RD.set(encoded, reg);
    return *this;
  }

  // first register operand
  Instruction& rs1(Register reg) {
    RS1.set(encoded, reg);
    return *this;
  }

  // second register operand
  Instruction& rs2(Register reg) {
    RS2.set(encoded, reg);
    return *this;
  }

  // modulus index
  Instruction& mod_index(PrimeModulusIndex idx) {
    MOD_INDEX.set(encoded, static_cast<uint64_t>(idx));
    return *this;
  }

  // automorphism number
  Instruction& automorphism(AutomorphismNumber anum) {
    AUTOMORPHISM.set(encoded, anum >> 1);
    return *this;
  }

  // address for LOAD instructions
  Instruction& load_addr(uint64_t addr) {
    LOAD_ADDR.set(encoded, addr);
    return *this;
  }

  // address for STORE instructions
  Instruction& store_addr(uint64_t addr) {
    STORE_ADDR1.set(encoded, addr);
    STORE_ADDR2.set(encoded, addr << 7);
    return *this;
  }

  uint64_t encode() const {
    return encoded;
  }

  static std::string pp_opcode(Instruction::Opcode const& op) {
    switch(op) {
      case LOAD: return "LOAD";
      case STORE: return "STORE";
      case ADD: return "ADD";
      case SUB: return "SUB";
      case MUL: return "MUL";
      case MORPH1: return "MORPH1";
      case MORPH2: return "MORPH2";
      case NTT1: return "NTT1";
      case NTT2: return "NTT2";
      case INTT1: return "INTT1";
      case INTT2: return "INTT2";
      case FENCE: return "FENCE";
      case RNGCONFIG: return "RNGCONFIG";
      case RNGSETUP: return "RNGSETUP";
      case RNGGENERATE: return "RNGGENERATE";
      case ADDI: return "ADDI";
      case SUBI: return "SUBI";
      case MULI: return "MULI";
      case ADDMULI: return "ADDMULI";
    }


    not_implemented("pp_opcode() not implemented for this opcode: " +
                                          std::to_string(uint64_t { op }));
  }

  bool has_imm() const {
    switch(opcode()) {
      case LOAD:
      case STORE:
      case ADD:
      case SUB:
      case MUL:
      case MORPH1:
      case MORPH2:
      case NTT1:
      case NTT2:
      case INTT1:
      case INTT2:
      case FENCE:
      case RNGCONFIG:
      case RNGSETUP:
      case RNGGENERATE:
        return false;

      case ADDI:
      case SUBI:
      case MULI:
      case ADDMULI:
        return true;
    }

    not_implemented("has_imm() not implemented for this opcode: " +
                      std::to_string(uint64_t { opcode() }));
  }

private:
  const Bits<0,4> OPCODE;
  const Bits<5, 11> RD;
  const Bits<12, 18> RS1;
  const Bits<19, 25> RS2;
  const Bits<26, 30> MOD_INDEX;
  const Bits<19, 33> AUTOMORPHISM;
  const Bits<12, 63> LOAD_ADDR;
  const Bits<5,11> STORE_ADDR1;
  const Bits<19, 63> STORE_ADDR2;


  uint64_t encoded;
};


