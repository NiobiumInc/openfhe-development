#pragma once

#include "constants.h"
#include "symbolic.h"
#include "ssa.h"
#include "instruction.h"
#include "instruction_buffer.h"
#include "instruction_analysis.h"
#include "alloc_table.h"
#include "modulus_table.h"

struct ValueLoc {
  AllocationTable memory { BASALISC_MEMORY_SIZE_BLOCKS };
  AllocationTable registers { BASALISC_REGISTER_COUNT };
};


// Result of instruction generation.
struct Result {
  InstructionBuffer instructions;      // the instructions
  std::map<ValueId, size_t> input_map; // a map of the inputs to addresses
  size_t gen_count;                    // number of instructions generated

  Result(InstructionBuffer&& inst, std::map<ValueId, size_t> imap, size_t gen_count)
  : instructions { inst }, input_map { imap }, gen_count { gen_count } 
  {

  }
};


class InstructionGenerator {
public:

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

  // Remove a value from the registers and memory.
  void free_value(ValueId const& v) {
    vloc.registers.free_val(v);
    vloc.memory.free_val(v);
  }

  // Store the value, which is in the given register somewhere in memory.
  // Returns `false` if we do not have space.
  bool store(InstructionBuffer& buf, ValueId id, Register r) {
    Address a;

    // if it's already there, then use it as-is
    if (vloc.memory.get_loc(id, a)) return true;

    // find a new free address to push it
    if (!vloc.memory.alloc_val(id, a)) return false;

    buf.push_inst(Instruction { }.into_store(a, r));
    return true;
  }

  // Allocate a register for a value.  Will evict stuff, if we need a new
  // register and there are none free.
  // Preloaded is an output argument indicating if the value was already
  // in a register.
  bool allocate_reg(InstructionBuffer& buf, ValueId v, size_t ssa_idx, Register& r,
                      bool& preloaded, ValueId avoid = UNDEF_VALUE_ID) {

    preloaded = false;

    // if it's already in a register, return the register
    if (vloc.registers.get_loc(v, r)) { preloaded = true; return true; }

    // try to allocate a new register
    if (vloc.registers.alloc_val(v,r)) return true;

    // try to evict something to memeory.
    auto e = analysis.find_eviction_candidate(vloc.registers.get_slot_map(), ssa_idx, avoid);
    if (e.freed != UNDEF_VALUE_ID) {
      r = e.slot;
      vloc.memory.free_val(e.freed);
      vloc.registers.replace_val(r, e.freed, v);
      return true;
    }

    if (!store(buf, v, e.slot)) return false;
    r = e.slot;
    vloc.registers.replace_val(r, e.freed, v);
    return true;
  }

  // Allocate a register for reading.  Read it in from memory, if needed.
  bool src_reg(InstructionBuffer& buf, ValueId v, size_t ssa_idx, Register& rloc, ValueId avoid = UNDEF_VALUE_ID) {
    bool preloaded;
    if(!allocate_reg(buf, v, ssa_idx, rloc, preloaded, avoid)) return false;
    if (preloaded) return true;

    // load value from memory
    size_t mloc;
    if (vloc.memory.get_loc(v, mloc) || allocate_input(v, mloc)) {
      buf.push_inst(Instruction {}.into_load(rloc, mloc));
      return true;
    }

    return false;
  }

    // if we don't have a value memory, this must be an input
    // TODO: maybe we could allocate this sooner - we could do a pass at the beginning
    // to figure out all the inputs and allocate space for them but we don't know
    // how many instructions we'll be generating
  bool allocate_input(ValueId v, size_t& mloc) {
    if (!vloc.memory.alloc_val(v, mloc)) return false;
    input_map[v] = mloc;
    return true;
  }

  // copy all registers to memory (presumably every register is live)
  void complete(InstructionBuffer& buf) {
    auto reg_map = vloc.registers.get_slot_map();
    auto reg_num = reg_map.size();
    for (Register r = 0; r < reg_num; ++r) {
      auto v = reg_map[r];
      if (v == UNDEF_VALUE_ID) continue;
      if (!store(buf, v, r)) panic("BUG not enough memory to complete?");
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

    if(ssa.arg2 != UNDEF_VALUE_ID && !src_reg(insts, ssa.arg2, ssa_idx, rs2, ssa.arg1)) {
      return false;
    }

    bool preload;
    if(ssa.dest != UNDEF_VALUE_ID && !allocate_reg(insts, ssa.dest, ssa_idx, rd, preload)) {
      return false;
    }

    // generate instructions an immediates
    switch(ssa.op) {
      case TODO:
        panic("TODO found while generating instructions");
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
        panic("BUG: FREE should have been handled earlier");
        break;
      default:
        not_implemented("instruction not implemented!");
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


