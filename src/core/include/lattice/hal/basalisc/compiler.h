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

const size_t BASALISC_REGISTER_COUNT = 128;

// number of vector-sized blocks the memory contains
const size_t BASALISC_MEMORY_SIZE_GB = 128 * 100;
const size_t BASALISC_POLY_LENGTH = (1 << 16);
const size_t BASALISC_BLOCK_SIZE = 8 * BASALISC_POLY_LENGTH;
const size_t BYTES_PER_GB = 1024 * 1024 * 1024;
const size_t BASALISC_MEMORY_SIZE_BLOCKS = (BASALISC_MEMORY_SIZE_GB * BYTES_PER_GB) / BASALISC_BLOCK_SIZE;


const size_t BASALISC_MODULUS_TABLE_SIZE = 32;

// ----------------------------------------------------------------------------
// -- Instructions

template<uint64_t LO, uint64_t HI> 
struct Bits {
  const uint64_t MASK = (UINT64_MAX >> (UINT64_WIDTH - HI - 1)) & (UINT64_MAX << LO);

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

  static std::string opcode_string(Instruction::Opcode const& op) {
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
    OPENFHE_THROW(not_implemented_error, "opcode_string() not implemented for this opcode: " + std::to_string(uint64_t { op }));
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

    OPENFHE_THROW(not_implemented_error, "has_imm() not implemented for this opcode: " + std::to_string(uint64_t { opcode() }));
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

using ValueId = uint64_t;
const ValueId UNDEF_VALUE_ID = 0;
const ValueId ZERO_VALUE_ID = 1;


// ----------------------------------------------------------------------------
// -- SymbolicValue

struct SymbolicValue {
  constexpr SymbolicValue(): value{UNDEF_VALUE_ID} {};
  explicit SymbolicValue(ValueId value);
  SymbolicValue(SymbolicValue const& s);
  SymbolicValue(SymbolicValue&& s) noexcept;
  SymbolicValue& operator=(SymbolicValue const& other);
  SymbolicValue& operator=(SymbolicValue&& other) noexcept;
  ~SymbolicValue();

  NativeInteger const& operator[](usint i) const;

  ValueId value;
};

// ----------------------------------------------------------------------------
// -- SSA instructions

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


class AllocationTable {
public:
  AllocationTable(size_t size) {
    for(size_t i = 0; i < size; i++) {
      free_list.insert(i);
    }
    wm = size;
  }

  bool get_loc(ValueId id, size_t& rloc) const {
    auto l = value_to_loc.find(id);
    if(l != value_to_loc.end()) {
      rloc = l->second;
      return true;
    }

    return false;
  }

  void set_loc(ValueId id, size_t rloc) {
    free_loc(rloc);
    value_to_loc[id] = rloc;
    loc_to_value[rloc] = id;
    free_list.erase(rloc);
    wm = std::min(rloc, wm);
  }

  bool find_highest_free(size_t& rloc) const {
    auto elt = free_list.lower_bound(size());
    if(elt == free_list.end())
      return false;

    rloc = *elt;
    return true;
  }

  bool find_free(size_t& rloc) const {
    if(free_list.begin() == free_list.end()) {
      return false;
    }

    rloc = *free_list.begin();
    return true;
  }

  void free_loc(size_t t) {
    auto i = loc_to_value.find(t);
    if(i != loc_to_value.end()) {
      value_to_loc.erase(i->second);
      loc_to_value.erase(t);
    }
    free_list.insert(t);
  }

  void clear_loc(size_t rloc) {
    auto v = loc_to_value.find(rloc);
    if(v != loc_to_value.end()) {
      value_to_loc.erase(v->second);
      loc_to_value.erase(rloc);
      free_list.insert(rloc);
    }
  }

  void burn_loc(size_t rloc) {
    clear_loc(rloc);
    free_list.erase(rloc);
  }

  void free_val(ValueId const& v) {
    size_t loc;
    while(get_loc(v, loc)) {
      free_loc(loc);
    }
  }

  size_t size() const {
    return free_list.size() + loc_to_value.size();
  }

  size_t watermark() const {
    return wm;
  }

  std::map<size_t, ValueId> const& get_slot_map() const {
    return loc_to_value;
  }

  bool allocate(ValueId const& val, size_t& loc) {
    if(!find_free(loc))
      return false;
    
    set_loc(val, loc);
    return true;
  }

private:
  size_t wm;
  std::map<ValueId, size_t> value_to_loc;
  std::map<size_t, ValueId> loc_to_value;
  std::set<size_t> free_list;
};

class InstructionAnalysis {
public:
  struct Eviction {
    Eviction() {
      freed = true;
      slot = 0;
    }

    Eviction(size_t slot, bool fr): slot { slot }, freed { fr } {

    }

    size_t slot;
    bool freed;
  };

  InstructionAnalysis(std::vector<SSAInst> const& inst) {
    analyze(inst);
  }

  void analyze(std::vector<SSAInst> const& inst) {
    val_free.clear();
    val_uses.clear();
    for(size_t i = 0; i < inst.size(); i++) {
      if(inst[i].op == SSAInstOp::FREE) {
        val_free[inst[i].arg1] = i;
        continue;
      }

      if(inst[i].arg1 != UNDEF_VALUE_ID) {
        use(inst[i].arg1, i);
      }

      if(inst[i].arg2 != UNDEF_VALUE_ID) {
        use(inst[i].arg2, i);
      }
    }
  }

  // for a set of values, beginning at SSA 
  Eviction find_eviction_candidate(std::map<size_t, ValueId> const& vals, size_t ssa_idx) {
    bool first = true;
    size_t candidate_slot = 0;
    size_t candidate_next_use = 0;
    int candidate_uses = 0;

    for(auto const& vs : vals) {
      ValueId const& slot = vs.first;
      size_t const& v = vs.second;

      auto uses = val_uses.find(v);
      auto freed = val_free.find(v);
      if(uses == val_uses.end())
        OPENFHE_THROW(openfhe_error, "BUG: InstructionAnalysis is not complete");

      // count the uses in the future, and determine the closest use in the future
      int use_count = 0;
      size_t next_use = 0;
      for(size_t use_idx : uses->second) {
        if(ssa_idx < use_idx) {
          if(use_count == 0) {
            next_use = use_idx;
          } else {
            next_use = std::min(next_use, use_idx);
          }
          use_count++;
        }
      }

      // if a value is not used and is eventually freed, free it now
      if(freed != val_free.end() && use_count == 0) {
        return Eviction { slot, true };
      }
      
      if(first) {
        candidate_slot = v;
        candidate_uses = use_count;
        candidate_next_use = next_use;
        first = false;
      } else if(candidate_next_use < ssa_idx + 20 && candidate_next_use < next_use ) {
        // if something is going to be used imminently, try to find an alternative to evicting it
        candidate_slot = v;
        candidate_uses = use_count;
        candidate_next_use = next_use;
      } else if(use_count < candidate_uses) {
        // prefer the value that's used the least frequently for eviction
        candidate_slot = v;
        candidate_uses = use_count;
        candidate_next_use = next_use;
      } else if(use_count == candidate_uses && next_use < candidate_next_use) {
        // prefer the value that will be used later for eviction
        candidate_slot = v;
        candidate_uses = use_count;
        candidate_next_use = next_use;
      }
    }

    if(first) {
      OPENFHE_THROW(openfhe_error, "BUG: empty vals");
    }

    return Eviction { candidate_slot, false };
  }

private:
  void use(ValueId const& v, size_t idx) {
    auto uses = val_uses.find(v);
    if(uses == val_uses.end()) {
      val_uses[v] = std::vector<size_t>{};
    }

    val_uses[v].push_back(idx);
  }

  std::map<ValueId, std::vector<size_t>> val_uses;
  std::map<ValueId, size_t> val_free;
};

struct ModulusTable {
  std::vector<uint64_t> modulus_table;

  PrimeModulusIndex get_modulus_index(uint64_t modulus) {
    for(PrimeModulusIndex i = 0; i < modulus_table.size(); i++) {
      if(modulus == modulus_table[i]) {
        return i;
      }
    }

    if(true || modulus_table.size() < BASALISC_MODULUS_TABLE_SIZE) {
      modulus_table.push_back(modulus);
      return modulus_table.size() - 1;
    }

    OPENFHE_THROW(openfhe_error, "Modulus table size exceeded");
  }

  uint64_t get_modulus(PrimeModulusIndex const& idx) const {
    if(idx >= modulus_table.size())
      OPENFHE_THROW(openfhe_error, "index exceeds modulus table size");

    return modulus_table[idx];
  }

  size_t size() const {
    return modulus_table.size();
  }

  void display() {
    for(size_t i = 0; i < modulus_table.size(); i++) {
      std::cout << "modulus_table[" << i << "] = " << modulus_table[i] << std::endl;
    }
  }
};

struct InstructionBuffer {
  void clear() {
    inst_buf.clear();
    inst_buf_is_inst.clear();
  }

  void push_inst(Instruction const& i) {
    inst_buf.push_back(i.encode());
    inst_buf_is_inst.push_back(true);
  }

  void push_imm(NativeInteger const& c) {
    inst_buf.push_back(uint64_t { c });
    inst_buf_is_inst.push_back(false);
  }

  void append(InstructionBuffer const& other) {
    inst_buf.insert(inst_buf.end(), other.inst_buf.begin(), other.inst_buf.end());
    inst_buf_is_inst.insert(inst_buf_is_inst.end(), other.inst_buf_is_inst.begin(), other.inst_buf_is_inst.end());
  }

  std::vector<uint64_t> inst_buf;
  std::vector<bool> inst_buf_is_inst;
};

struct ValueLoc {
  AllocationTable memory { BASALISC_MEMORY_SIZE_BLOCKS };
  AllocationTable registers { BASALISC_REGISTER_COUNT };
};

class InstructionGenerator {
public:
  struct Result {
    InstructionBuffer instructions;      // the instructions
    std::map<ValueId, size_t> input_map; // a map of the inputs to addresses
    size_t gen_count;                    // number of instructions generated

    Result(InstructionBuffer&& inst, std::map<ValueId, size_t> imap, size_t gen_count)
    : instructions { inst }, input_map { imap }, gen_count { gen_count } 
    {

    }
  };

  // Generate instructions into `dest` until we exceed some limit
  // Transforms `mt` and `vloc` accordingly
  static Result generate_instructions(std::vector<SSAInst> const& input, ModulusTable& mt, ValueLoc& vloc) {
    InstructionGenerator ig { input, mt, vloc };
    InstructionBuffer dest;
    for(size_t idx = 0; idx < input.size(); idx++) {
      if(!ig.generate_instruction(dest, input[idx], idx)) {
        return { std::move(dest), std::move(ig.input_map), idx };
      }
    }

    ig.complete(dest);
    return { std::move(dest), std::move(ig.input_map), input.size() };
  }

private:
  InstructionGenerator(std::vector<SSAInst> const& ssa, ModulusTable& mt, ValueLoc& loc)
  : modulus_table { mt }, vloc { loc }, analysis { ssa } 
  {

  }

  void free_value(ValueId const& v) {
    vloc.registers.free_val(v);
    vloc.memory.free_val(v);
  }

  bool store(InstructionBuffer& buf, ValueId id, Register r, Address& a) {
    size_t mloc;
    // if it's already there, then use it as-is
    if(vloc.memory.get_loc(id, mloc)) {
      a = mloc;
      return true;
    }

    // find a new free address to push it
    if(vloc.memory.find_free(mloc)) {
      Instruction i;
      buf.push_inst(Instruction { }.into_store(mloc, r));
      a = mloc;
      return true;
    }

    return false;
  }

  bool allocate_reg(InstructionBuffer& buf, ValueId const& v, size_t ssa_idx, Register& r) {
    size_t rloc = 0;
    // if it's already in a register, return the register
    if(vloc.registers.get_loc(v, rloc)) {
      r = rloc;
      return true;
    }

    // if there are no free registers, evict something to memory - otherwise use the free register
    if(!vloc.registers.find_free(rloc)) {
      auto e = analysis.find_eviction_candidate(vloc.registers.get_slot_map(), ssa_idx);
      if(!e.freed) {
        Address addr;
        if(!store(buf, v, e.slot, addr)) {
          return false;
        }
      }
      rloc = e.slot;
    } 

    r = rloc;
    return true;
  }

  bool src_reg(InstructionBuffer& buf, ValueId const& v, size_t ssa_idx, Register& rloc) {
    if(!allocate_reg(buf, v, ssa_idx, rloc)) {
      return false;
    }

    // load value from memory
    size_t mloc;
    if(vloc.memory.get_loc(v, mloc) || allocate_input(v, mloc)) {
      vloc.registers.set_loc(v, rloc);
      buf.push_inst(Instruction {}.into_load(rloc, mloc));
      return true;
    }

    return false;
  }

    // if we don't have a value memory, this must be an input
    // TODO: maybe we could allocate this sooner - we could do a pass at the beginning
    // to figure out all the inputs and allocate space for them but we don't know
    // how many instructions we'll be generating
  bool allocate_input(ValueId const& v, size_t& mloc) {
    if(vloc.memory.allocate(v, mloc)) {
      input_map[v] = mloc;
      return true;
    }

    return false;
  }

  // copy all registers to memory (presumably every register is live)
  void complete(InstructionBuffer& buf) {
    Address _a;
    for(auto const& reg_slot : vloc.registers.get_slot_map()) {
      if(!store(buf, reg_slot.first, reg_slot.second, _a)) {
        OPENFHE_THROW(openfhe_error, "BUG not enough memory to complete?");
      }
    }
  }

  bool generate_instruction(InstructionBuffer& buf, SSAInst const& ssa, size_t ssa_idx) {
    Instruction inst;  // A temporary instruction
    
    // Temporary buffer for instructions -- 
    // we either want to output a complete set of instructions (including loads and stores)
    // to `buf` or none at all - so we stage instructions here
    InstructionBuffer insts;

    Register rs1;
    Register rs2;
    Register rd;
    PrimeModulusIndex modidx = ssa.modulus != 0 ? modulus_table.get_modulus_index(ssa.modulus) : 0;

    // special case for FREE
    if(ssa.op == SSAInstOp::FREE) {
      free_value(ssa.arg1);
      return true;
    }

    // allocate any needed registers
    if(ssa.arg1 != UNDEF_VALUE_ID && !src_reg(insts, ssa.arg1, ssa_idx, rs1)) {
      return false;
    }

    if(ssa.arg2 != UNDEF_VALUE_ID && !src_reg(insts, ssa.arg2, ssa_idx, rs2)) {
      return false;
    }

    if(ssa.dest != UNDEF_VALUE_ID && !allocate_reg(insts, ssa.dest, ssa_idx, rd)) {
      return false;
    }
    vloc.registers.set_loc(ssa.dest, rd);

    // generate instructions an immediates
    switch(ssa.op) {
      case TODO:
        OPENFHE_THROW(not_available_error, "TODO found while generating instructions");
        break;

      case ADD:
        insts.push_inst(inst.into_add(rd, rs1, rs2, modidx)); 
        break;

      case SUB:
        insts.push_inst(inst.into_sub(rd, rs1, rs2, modidx)); 
        break;

      case MUL:
        insts.push_inst(inst.into_mul(rd, rs1, rs2, modidx)); 
        break;

      case ADDI:
        insts.push_inst(inst.into_addi(rd, rs1, modidx));
        insts.push_imm(ssa.imm);
        break;

      case SUBI:
        insts.push_inst(inst.into_subi(rd, rs1, modidx));
        insts.push_imm(ssa.imm);
        break;

      case MULI:
        insts.push_inst(inst.into_muli(rd, rs1, modidx));
        insts.push_imm(ssa.imm);
        break;

      case ADDMULI:
        insts.push_inst(inst.into_addmuli(rd, rs1, rs2, modidx));
        insts.push_imm(ssa.imm);
        break;

      case NTT: 
        insts.push_inst(inst.into_ntt1(rd, rs1, modidx));
        insts.push_inst(inst.into_ntt2(rd, rd, modidx));
        break;

      case INTT: {
        insts.push_inst(inst.into_intt1(rd, rs1, modidx));
        insts.push_inst(inst.into_intt2(rd, rd, modidx));
        break;
      }
      case MORPH: {
        insts.push_inst(inst.into_morph1(rd, rs1, uint64_t { ssa.imm }));
        insts.push_inst(inst.into_morph2(rd, rd, uint64_t { ssa.imm }));
        break;
      }
      case FREE: 
        OPENFHE_THROW(not_implemented_error, "BUG: should have been heandled earlier");
        break;
      case SWITCHMODULUS: {
        /// XXX: implement me
        break;
      }
      default:
        OPENFHE_THROW(not_implemented_error, "instruction not implemented!");
    }


    // append temp instructions to buf
    buf.append(insts);
    return true;
  }

  ModulusTable& modulus_table;
  ValueLoc& vloc;
  InstructionAnalysis analysis;
  std::map<ValueId, size_t> input_map;
};

class OpenFHEEvaluator {
public:


  void loadvec(NativeVector& v, std::array<uint64_t, BASALISC_POLY_LENGTH> const& reg, NativeInteger const& modulus) {
    for(size_t i = 0; i < BASALISC_POLY_LENGTH; i++) {
      v[i] = reg[i];
    }
    v.SetModulus(modulus);
  }

  void storevec(std::array<uint64_t, BASALISC_POLY_LENGTH>& reg, NativeVector const& v) {
    for(size_t i = 0; i < BASALISC_POLY_LENGTH; i++) {
      reg[i] = uint64_t { v[i] };
    }
  }

  void evaluate(size_t start, size_t len, ModulusTable const& mt) {
    size_t ip = start;
    size_t end = start + len;
    std::array<std::array<uint64_t,BASALISC_POLY_LENGTH>, BASALISC_REGISTER_COUNT> regs;
    NativeVector v1 { BASALISC_POLY_LENGTH };
    NativeVector v2 { BASALISC_POLY_LENGTH };

    while(ip < end) {
      Instruction i { memory[ip] };
      uint64_t const& imm = 
        i.has_imm() && memory[ip+1] ? memory[ip+1] : 0;

      switch(i.opcode()) {
        case Instruction::Opcode::LOAD:
          std::copy(memory.begin() + i.load_addr(), memory.begin() + i.load_addr() + regs[i.rd()].size(), regs[i.rd()].begin());
          break;

        case Instruction::Opcode::STORE:
          std::copy(regs[i.rs1()].begin(), regs[i.rs1()].end(), memory.begin() + i.store_addr());
          break;

        case Instruction::Opcode::ADD:
          loadvec(v1, regs[i.rs1()], mt.get_modulus(i.mod_index()));
          loadvec(v2, regs[i.rs2()], mt.get_modulus(i.mod_index()));
          v1.ModAddNoCheckEq(v2);
          storevec(regs[i.rd()], v1);
          break;

        case Instruction::Opcode::ADDI:
          loadvec(v1, regs[i.rs1()], mt.get_modulus(i.mod_index()));
          v1.ModAddEq(NativeInteger {imm});
          storevec(regs[i.rd()], v1);
          break;

        case Instruction::Opcode::SUB:
          loadvec(v1, regs[i.rs1()], mt.get_modulus(i.mod_index()));
          loadvec(v2, regs[i.rs2()], mt.get_modulus(i.mod_index()));
          v1.ModSubEq(v2);
          storevec(regs[i.rd()], v1);
          break;

        case Instruction::Opcode::SUBI:
          loadvec(v1, regs[i.rs1()], mt.get_modulus(i.mod_index()));
          v1.ModSubEq(NativeInteger {imm});
          storevec(regs[i.rd()], v1);
          break;

        case Instruction::Opcode::MUL:
          loadvec(v1, regs[i.rs1()], mt.get_modulus(i.mod_index()));
          loadvec(v2, regs[i.rs2()], mt.get_modulus(i.mod_index()));
          v1.ModAddNoCheckEq(v2);
          storevec(regs[i.rd()], v1);
          break;

        case Instruction::Opcode::MULI:
          loadvec(v1, regs[i.rs1()], mt.get_modulus(i.mod_index()));
          v1.ModMulEq(NativeInteger {imm});
          storevec(regs[i.rd()], v1);
          break;

        case Instruction::Opcode::ADDMULI:
          loadvec(v1, regs[i.rs1()], mt.get_modulus(i.mod_index()));
          loadvec(v2, regs[i.rs2()], mt.get_modulus(i.mod_index()));
          v2.ModMulEq(NativeInteger { imm });
          v1.ModAddNoCheckEq(v2);
          storevec(regs[i.rd()], v1);
          break;

        case Instruction::Opcode::MORPH1:
          // morph1 does nothing
          break;
        case Instruction::Opcode::MORPH2: {
          loadvec(v1, regs[i.rs1()], mt.get_modulus(i.mod_index()));
          PolyImpl<NativeVector> poly;
          poly.SetValues(v1, Format::EVALUATION);
          poly.AutomorphismTransform(i.automorphism());
          storevec(regs[i.rd()], poly.GetValues());
          break;
        }
        case Instruction::Opcode::NTT1:
          break;
        case Instruction::Opcode::NTT2: {
          loadvec(v1, regs[i.rs1()], mt.get_modulus(i.mod_index()));
          PolyImpl<NativeVector> poly;
          poly.SetValues(v1, Format::COEFFICIENT);
          poly.SwitchFormat();
          storevec(regs[i.rd()], poly.GetValues());
          break;
        }
        case Instruction::Opcode::INTT1:
          break;
        case Instruction::Opcode::INTT2: {
          loadvec(v1, regs[i.rs1()], mt.get_modulus(i.mod_index()));
          PolyImpl<NativeVector> poly;
          poly.SetValues(v1, Format::EVALUATION);
          poly.SwitchFormat();
          storevec(regs[i.rd()], poly.GetValues());
          break;
        }
        case Instruction::Opcode::FENCE:
          break;
        case Instruction::Opcode::RNGCONFIG:
          OPENFHE_THROW(not_implemented_error, "rng ops not implemenred");
        case Instruction::Opcode::RNGSETUP:
          OPENFHE_THROW(not_implemented_error, "rng ops not implemenred");
        case Instruction::Opcode::RNGGENERATE:
          OPENFHE_THROW(not_implemented_error, "rng ops not implemenred");
        default:
          OPENFHE_THROW(not_implemented_error, "BUG: missing instruction");
      }
      

      // advance ip, skip immediate if it's there
      if(i.has_imm())
        ip+= 2;
      else
        ip++;
    }
  }
private:

  size_t ip = 0;
  std::vector<uint64_t> memory;
};

// ----------------------------------------------------------------------------
// -- Program
class Program {
public:
  SymbolicValue ConcretePoly(NativeVector&& v) {
    auto value = new_value();
    modulus_index(v.GetModulus());
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
    return emit_instruction({SSAInstOp::ADDI, a1, i, m});
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

  SymbolicValue MulI(SymbolicValue const& a1, NativeInteger const& i, NativeInteger const& m) {
    return emit_instruction({SSAInstOp::MULI, a1, i, m});
  }

  SymbolicValue Morph(SymbolicValue const& a1, AutomorphismNumber n, NativeInteger const& m) {
    return emit_instruction({SSAInstOp::MORPH, a1, n, m});
  }

  SymbolicValue NTT(SymbolicValue const& a1, NativeInteger const& m) {
    return emit_instruction({SSAInstOp::NTT, a1, m});
  }

  SymbolicValue INTT(SymbolicValue const& a1, NativeInteger const& m) {
    return emit_instruction({SSAInstOp::INTT, a1, m});
  }

  SymbolicValue SwitchModulus(SymbolicValue const& a, NativeInteger rootOfUnity, NativeInteger m) {
    return emit_instruction({SSAInstOp::SWITCHMODULUS, a, rootOfUnity, m});
  }

  SymbolicValue emit_instruction(SSAInst const& ssa) {
    auto v = new_value();
    m_inst.push_back(ssa);
    m_inst.back().dest = v.value;
    return v;
  }

  bool is_zero_vec(NativeVector const& nv) {
    for(size_t i = 0; i < nv.GetLength(); i++) {
      if(nv[i] != 0)
        return false;
    }

    return true;
  }

  void increment_refcount(ValueId val) {
    if (val == UNDEF_VALUE_ID) return;

    auto res = m_symbolic_refcount.find(val);
    if(res != m_symbolic_refcount.end()) {
      res->second++;
    } else {
      m_symbolic_refcount[val] = 1;
    }
  }

  void decrement_refcount(ValueId val) {
    if (val == UNDEF_VALUE_ID) return;

    auto res = m_symbolic_refcount.find(val);
    if(res != m_symbolic_refcount.end()) {
      res->second--;
      if(res->second <= 0) {
        m_inst.push_back({FREE, val});  
        m_symbolic_refcount.erase(val);
      }
    }
  }

  bool is_used(ValueId const& v) {
    for(auto const& ssa_inst: m_inst) {
      if(ssa_inst.arg1 == v || ssa_inst.arg2 == v) {
        return true;
      }
    }
    return false;
  }

  size_t modulus_index(NativeInteger const& m) {
    return modulus_table.get_modulus_index(uint64_t { m });
  }

  SymbolicValue new_value() {
    return SymbolicValue {m_next_value_name++};
  }

  bool is_concrete(SymbolicValue const& s) const {
    return m_concrete_polys.find(s.value) != m_concrete_polys.end();
  }

  void freeze_value(SymbolicValue const& v) {
    m_modifiable_concrete.erase(v.value);
    m_concrete_polys.erase(v.value);
  }

  NativeVector const& get_values(SymbolicValue const& s) {
    auto p = m_concrete_polys.find(s.value);
    if(p == m_concrete_polys.end()) {
      OPENFHE_THROW(not_available_error, "get_values() called on symbolic value");
    }
    return p->second;
  }

  NativeVector& get_values_mut(SymbolicValue& s) {
    bool is_mod = m_modifiable_concrete.find(s.value) != m_modifiable_concrete.end();
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

  struct InstructionStats {
    std::map<Instruction::Opcode, size_t> opcode_freq;
    size_t epoch_count = 0;
    size_t host_to_basalisc_bytes = 0;
    size_t basalisc_to_host_bytes = 0;
    size_t modulus_table_size = 0;

    void display() {
      std::cout << "host_to_basalisc_bytes: " << host_to_basalisc_bytes << std::endl;
      std::cout << "basalisc_to_host_bytes: " << basalisc_to_host_bytes << std::endl;
      std::cout << "epoch_count: " << epoch_count << std::endl;

      std::cout << std::endl;
      std::cout << "instruction freq:" << std::endl;
      for(auto const& opfreq : opcode_freq) {
        std::cout << Instruction::opcode_string(opfreq.first) << ": " <<  opfreq.second << std::endl;
      }

      std::cout << "modulus_table size: " << modulus_table_size << std::endl;

      std::cout << std::endl;
    }
  };

  InstructionStats instruction_stats() const {
    InstructionStats stats;

    // make copies of context stuff
    std::vector<SSAInst> inst = m_inst;
    ModulusTable mt { modulus_table };
    ValueLoc vl { vloc };

    // generate instructions
    auto epoch = InstructionGenerator::generate_instructions(inst, mt, vl);

    std::cout << "epoch: " << epoch.gen_count << " ssa: " << inst.size() << "\n";

    // track epoch
    stats.epoch_count++;

    // track copies
    stats.host_to_basalisc_bytes += epoch.input_map.size() * BASALISC_BLOCK_SIZE;
    stats.host_to_basalisc_bytes += epoch.instructions.inst_buf.size() * sizeof(u_int64_t);

    stats.basalisc_to_host_bytes += vl.memory.size() * BASALISC_BLOCK_SIZE;

    // count ops
    for(size_t i = 0; i < epoch.instructions.inst_buf.size(); i++) {
      if(!epoch.instructions.inst_buf_is_inst[i])
        continue;

      auto opcode = Instruction { epoch.instructions.inst_buf[i] }.opcode();
      auto op_count = stats.opcode_freq.find(opcode);
      if(op_count == stats.opcode_freq.end()) {
        stats.opcode_freq[opcode] = 1;
      } else {
        op_count->second += 1;
      }
    }

    // modulus table size
    stats.modulus_table_size = mt.size();

    return stats;
  }

  void software_computation() {
    OpenFHEEvaluator eval;
    while(m_inst.size() > 0) {
      // generate instructions
      auto epoch = InstructionGenerator::generate_instructions(m_inst, modulus_table, vloc);
      m_inst.erase(m_inst.begin(), m_inst.begin() + epoch.gen_count);

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
  ValueLoc vloc;

  // modulus table
  ModulusTable modulus_table;
  
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