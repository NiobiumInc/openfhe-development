#ifndef LBCRYPTO_INC_LATTICE_HAL_BASALISC_COMPILER_H
#define LBCRYPTO_INC_LATTICE_HAL_BASALISC_COMPILER_H

#include<variant>
#include<stdint.h>
#include<assert.h>
#include<optional>
#include<map>
#include<set>
#include"utils/exception.h"
#include"math/hal/nativeintbackend.h"
#include"lattice/hal/default/poly.h"

namespace lbcrypto {

using Register = uint64_t;
using Address = uint64_t;
using Immediate = uint64_t;
using PrimeModulusIndex = uint8_t;
using AutomorphismNumber = uint32_t;
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
const ValueId undef_value_id = 0;
const ValueId zero_value_id = 1;

// SSA form instructions
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


// Symbolic value
struct SymbolicValue {
  constexpr SymbolicValue(): value{undef_value_id} {};
  SymbolicValue(ValueId value);
  SymbolicValue(SymbolicValue const& s);
  SymbolicValue(SymbolicValue&& s) noexcept;
  SymbolicValue& operator=(SymbolicValue const& other);
  SymbolicValue& operator=(SymbolicValue&& other) noexcept;
  ~SymbolicValue();

  NativeInteger const& operator[](usint i) const;

  ValueId value;
};

// Compiler state
struct SSAInst {
    // TODO:  nativeinteger const&
    SSAInst(SSAInstOp op, SymbolicValue const& arg1, SymbolicValue const& arg2, NativeInteger const& m);
    SSAInst(SSAInstOp op, SymbolicValue const& arg, NativeInteger const& i, NativeInteger const& m);
    SSAInst(SSAInstOp op, SymbolicValue const& arg1, SymbolicValue const& arg2, NativeInteger const& i, NativeInteger const& m);
    SSAInst(SSAInstOp op, SymbolicValue& arg, NativeInteger const& m);
    SSAInst(SSAInstOp op, SymbolicValue& arg, NativeInteger const& m);
    SSAInst(SSAInstOp op, SymbolicValue& arg, AutomorphismNumber n, NativeInteger const& m);

    SSAInstOp op;
    ValueId dest;
    ValueId arg1;
    ValueId arg2; 
    NativeInteger imm; // or AutomorphismNumber if it is MORPH
    PrimeModulusIndex modulus;
};


// Compiler state
class Program {
public:
  SymbolicValue ConcretePoly(NativeVector&& v) {
    auto value = new_value();
    auto midx = modulus_index(v.GetModulus());
    m_concrete_polys[value.value] = v;
    m_modifiable_concrete.insert(value.value);
    return value;
  }

  SymbolicValue ConcretePoly(std::unique_ptr<NativeVector> v) {
    return ConcretePoly(std::move(*v));
  }

  SymbolicValue Add(SymbolicValue const& a1, SymbolicValue const& a2, NativeInteger const& m) {
    return emit_instruction({SSAInstOp::ADD, a1, a2, m});
  }

  SymbolicValue AddI(SymbolicValue const& a1, NativeInteger const& i, NativeInteger const& m) {
    return emit_instruction({SSAInstOp::ADDI, a1.value, i, m});
  }

  SymbolicValue Sub(SymbolicValue const& a1, SymbolicValue const& a2, NativeInteger const& m) {
    return emit_instruction({SSAInstOp::SUB, a1, a2, m});
  }

  SymbolicValue SubI(SymbolicValue a1, NativeInteger const& i, NativeInteger const& m) {
    return emit_instruction({SSAInstOp::SUBI, a1, i, m});
  }

  SymbolicValue Mul(SymbolicValue const& a1, SymbolicValue const& a2, NativeInteger const& m) {
    return emit_instruction({SSAInstOp::MUL, a1, a2, m});
  }

  SymbolicValue MulI(SymbolicValue a1, NativeInteger const& i, NativeInteger const& m) {
    return emit_instruction({SSAInstOp::MULI, a1, i, m});
  }

  SymbolicValue Morph(SymbolicValue a1, AutomorphismNumber n, NativeInteger const& m) {
    return emit_instruction({SSAInstOp::MORPH, a1, n, m});
  }

  SymbolicValue NTT(SymbolicValue a1, NativeInteger const& m) {
    return emit_instruction({SSAInstOp::NTT, a1, m});
  }

  SymbolicValue INTT(SymbolicValue a1, NativeInteger const& m) {
    return emit_instruction({SSAInstOp::INTT, a1, m});
  }

  SymbolicValue emit_instruction(SSAInst const& ssa) {
    auto v = new_value();
    m_inst.push_back(ssa);
    m_inst.back().dest = v.value;
    return v;
  }

  void increment_refcount(ValueId val) {
    if (val == undef_value_id) return;

    auto res = m_symbolic_refcount.find(val);
    if(res != m_symbolic_refcount.end()) {
      res->second++;
    } else {
      m_symbolic_refcount[val] = 1;
    }
  }

  void decrement_refcount(ValueId val) {
    if (val == undef_value_id) return;

    auto res = m_symbolic_refcount.find(val);
    if(res != m_symbolic_refcount.end()) {
      res->second--;
      if(res->second <= 0) {
        // emit hint to memory that this value is dead
        m_symbolic_refcount.erase(val);
      }
    }
  }

  size_t modulus_index(NativeInteger const& m) {
    auto midx = m_modulus_lookup.find(m);
    if(midx != m_modulus_lookup.end()) {
      return midx->second;
    }

    if(m_next_modulus_slot >= MODULUS_TABLE_SIZE) {
      OPENFHE_THROW(not_implemented_error, "program exceeds modulus table size");
    }

    m_modulus_table[m_next_modulus_slot] = m;
    m_modulus_lookup[m] = m_next_modulus_slot;
    return m_next_modulus_slot++;
  }

  SymbolicValue new_value() {
    return {m_next_value_name++};
  }

  bool is_concrete(SymbolicValue const& s) const {
    return m_concrete_polys.find(s.value) != m_concrete_polys.end();
  }

  void freeze_value(SymbolicValue const& v) {
    m_modifiable_concrete.erase(v.value);
  }

  NativeVector const& get_values(SymbolicValue const& s) {
    auto p = m_concrete_polys.find(s.value);
    if(p == m_concrete_polys.end()) {
      OPENFHE_THROW(not_available_error, "get_values() called on symbolic value");
    }
    return p->second;
  }

  NativeVector& get_values_mut(SymbolicValue& s) {
    bool is_mod = m_modifiable_concrete.find(s.value) == m_modifiable_concrete.end();
    if(is_mod) {
      return m_concrete_polys[s.value];
    } else {
      OPENFHE_THROW(not_available_error, "get_values_mut() called on symbolic or frozen value");
    }
  }

  // do a deep, modifiable copy of a polynomial if it is concrete
  // if it is symbolic, does a normal (by-reference) copy instead
  SymbolicValue deepcopy(SymbolicValue const& s) {
    auto poly = m_concrete_polys.find(s.value);
    if(poly == m_concrete_polys.end()) {
      return s;
    } else {
      auto data = std::make_unique<NativeVector>(poly->second);
      return ConcretePoly(std::move(data));
    }
  }

  // std::optional<NativeVector&> get_values_if_modifiable(SymbolicValue const& s) {
  //   auto poly = m_concrete_polys.find(s.value);
  //   auto refs = m_symbolic_refcount[s.value];

  //   if(poly == m_concrete_polys.end() || refs != 1) {
  //     return std::nullopt;
  //   } else {
  //     return *(poly->second.coeff);
  //   }
  // }

private:
  // modulus table
  static const size_t MODULUS_TABLE_SIZE = 32;
  size_t m_next_modulus_slot;
  NativeInteger m_modulus_table[MODULUS_TABLE_SIZE];
  std::map<NativeInteger, size_t> m_modulus_lookup;

  // symbolic value stuff
  std::unordered_map<ValueId, size_t> m_symbolic_refcount;
  ValueId m_next_value_name = 2;
      // 0 is reserved for uninitialized symbolic value
      // 1 is reserved for constant 0 polynomial
  std::unordered_set<ValueId> m_modifiable_concrete;

  // the program
  std::unordered_map<ValueId, NativeVector> m_concrete_polys; // mapping from symbolic value to index into m_concrete_polys;
  std::vector<SSAInst> m_inst;
};


// **************** COMPILER GLOBAL VARIABLE **********************************
inline Program Basalisc;

} // namespace lbcrypto

#endif
