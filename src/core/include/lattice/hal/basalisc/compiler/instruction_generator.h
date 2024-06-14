#pragma once

#include "constants.h"
#include "symbolic.h"
#include "ssa.h"
#include "instruction.h"
#include "instruction_buffer.h"
#include "instruction_analysis.h"
#include "alloc_table.h"
#include "modulus_table.h"
#include <cassert>


struct ValueLoc {
  AllocationTable memory { BASALISC_MEMORY_SIZE_BLOCKS - BASALISC_RESERVED_SIZE };
  AllocationTable registers { BASALISC_REGISTER_COUNT };

  // values stored in the reserved space
  std::unordered_map<ValueId, Address> reserved_space_values; 
};

// Result of instruction generation.
struct Epoch {
  InstructionBuffer instructions;                 // the instructions
  std::unordered_map<ValueId, Address> input_map;  // a map of the inputs to addresses - values must be moved 
                                                  // here before executing the epoch
  std::unordered_map<ValueId, Address> output_map; // map from values to where they will live after the epoch
  ModulusTable modulus_table;                     // the modulus table
  std::string end_reason;                         // what ended the epoch
  Address gen_idx;                                 // where in the SSA vector we stopped generating instructions
};

class InstructionGenerator {
public:

  static std::vector<Epoch> generate_epochs(std::vector<SSAInst> const& input) {
    Address idx = 0;
    std::vector<Epoch> epochs;
    InstructionAnalysis analysis { input };
    while(idx < input.size()) {
      Epoch e = generate_instructions(input, analysis, idx);
      assert(e.gen_idx > idx);
      idx = e.gen_idx;
      epochs.push_back(std::move(e));
    }

    return epochs;
  }

  // Generate instructions into `dest` until we exceed some limit
  // Transforms `mt` and `vloc` accordingly
  static Epoch generate_instructions(std::vector<SSAInst> const& input, InstructionAnalysis const& analysis, Address gen_start = 0) {
    Epoch epoch;
    ValueLoc vloc;
    InstructionGenerator ig { input, epoch.modulus_table, vloc, analysis };
    InstructionBuffer dest;
    Address idx = gen_start;
    for(idx = gen_start; idx < input.size(); idx++) {
      if(!ig.generate_instruction(epoch.instructions, input[idx], idx, epoch.end_reason)) {
        break;
      }
    }
    epoch.gen_idx = idx;

    if(idx == input.size()) {
      epoch.end_reason = "execution complete";
    }

    // move anything live from registers to memory
    ig.complete(epoch.instructions);

    // compute input/output maps
    epoch.input_map = std::move(ig.input_map);
    vloc.memory.extract_and_clear(epoch.output_map);
    epoch.output_map.merge(vloc.reserved_space_values);

    return epoch;
  }

private:
  InstructionGenerator(std::vector<SSAInst> const& ssa, ModulusTable& mt, ValueLoc& loc, InstructionAnalysis const& analysis)
  : modulus_table { mt }, vloc { loc }, analysis { analysis } 
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
  bool allocate_reg(InstructionBuffer& buf, ValueId v, Address ssa_idx, Register& r,
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
  bool src_reg(InstructionBuffer& buf, ValueId v, Address ssa_idx, Register& rloc, ValueId avoid = UNDEF_VALUE_ID) {
    bool preloaded;
    if(!allocate_reg(buf, v, ssa_idx, rloc, preloaded, avoid)) return false;
    if (preloaded) return true;

    // load value from memory
    Address mloc;
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
  bool allocate_input(ValueId v, Address& mloc) {
    if (!vloc.memory.alloc_val_fresh(v, mloc)) return false;
    input_map[v] = mloc;
    return true;
  }

  // copy all registers to memory (presumably every register is live)
  void complete(InstructionBuffer& buf) {
    auto reg_map = vloc.registers.get_slot_map();
    auto reg_num = reg_map.size();
    Address mloc;
    Address reserved_loc = BASALISC_RESERVED_REGISTER_ADDRESS;
    for (Register r = 0; r < reg_num; ++r) {
      if(reg_map[r] != UNDEF_VALUE_ID && !vloc.memory.get_loc(reg_map[r], mloc)) {
        buf.push_inst(Instruction {}.into_store(reserved_loc++, r));
      }
    }
  }

  bool generate_instruction(InstructionBuffer& buf, SSAInst const& ssa, Address ssa_idx, std::string& failure_reason) {
    Instruction inst;  // A temporary instruction
    
    // Temporary buffer for instructions -- 
    // we either want to output a complete set of instructions (including loads and stores)
    // to `buf` or none at all - so we stage instructions here
    InstructionBuffer insts;

    Register rs1;
    Register rs2;
    Register rd;
    PrimeModulusIndex modidx = 0;
    
    // get index for modulus or 
    if(ssa.modulus != 0 && !modulus_table.try_get_modulus_index(ssa.modulus, modidx)) {
      failure_reason = "at limit for modulus table size";
      return false;
    }

    // special case for FREE
    if(ssa.op == SSAInstOp::FREE) {
      free_value(ssa.arg1);
      return true;
    }

    // allocate any needed registers
    if(ssa.arg1 != UNDEF_VALUE_ID && !src_reg(insts, ssa.arg1, ssa_idx, rs1)) {
      failure_reason = "at memory limit (src_reg returned false)";
      return false;
    }

    if(ssa.arg2 != UNDEF_VALUE_ID && !src_reg(insts, ssa.arg2, ssa_idx, rs2, ssa.arg1)) {
      failure_reason = "at memory limit (src_reg returned false)";
      return false;
    }

    bool preload;
    if(ssa.dest != UNDEF_VALUE_ID && !allocate_reg(insts, ssa.dest, ssa_idx, rd, preload)) {
      failure_reason = "at memory limit (allocate_reg returned false)";
      return false;
    }

    // generate instructions an immediates
    switch(ssa.op) {
      case NOP:
        break;

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

    // append temp instructions to buf - or fail if we're out of space for instructions
    if(buf.size_bytes() + insts.size_bytes() > MAX_PROGRAM_SIZE_BYTES) {
      failure_reason = "at limit for program space";
      return false;
    }
    buf.append(insts);
    return true;
  }

  // the largest program we can create in the reserved program space, minus the amount we need to write any stores needed at the end
  const Address MAX_PROGRAM_SIZE_BYTES = (BASALISC_RESERVED_PROGRAM_SIZE * BASALISC_BLOCK_SIZE) - 
                                        (BASALISC_REGISTER_COUNT * BASALISC_INSTRUCTION_SIZE_BYTES);
  ModulusTable& modulus_table;
  ValueLoc& vloc;
  InstructionAnalysis const& analysis;
  std::unordered_map<ValueId, Address> input_map;
};


