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
  InstructionBuffer instructions;                  // the instructions
  std::unordered_map<ValueId, Address> input_map;  // a map of the inputs to addresses - values must be moved 
                                                   // here before executing the epoch
  std::unordered_map<ValueId, Address> output_map; // map from values to where they will live after the epoch
  ModulusTable modulus_table;                      // the modulus table
  std::string end_reason;                          // what ended the epoch

  void reset() {
    instructions = {};
    input_map = {};
    output_map = {};
    modulus_table = {};
    end_reason = {};
  }
};

struct GenerationIndex {

  std::vector<bool> generated;
  size_t start;
  size_t current;

  GenerationIndex(size_t size) {
    generated = std::vector<bool>(size, false);
    start = 0;
    current = 0;
  }

  size_t current_idx() {
    return current;
  }

  size_t next_idx() {
    while(current < generated.size() && generated[current]) {
      if(current == start)
        start++;
      current++;
    }
    
    return current;
  }

  void record_gen() {
    if(current == start)
      start++;

    generated[current++] = true;
  }

  void record_skip() {
    current++;
  }

  bool complete() {
    return start == generated.size();
  }

  void reset() {
    current = start;
  }
};

enum InstructionGenerationResult {
  CONTINUE,
  SKIP,
  STOP,
};


class InstructionGenerator {
public:
  InstructionGenerator(std::vector<SSAInst> const& program)
  : program { program }, analysis { program }, gen_idx {program.size()}
  {
  }

  static std::vector<Epoch> generate_epochs(std::vector<SSAInst> const& program) {
    Epoch e;
    std::vector<Epoch> epochs;
    InstructionGenerator gen { program };
    while(gen.generate_epoch(e)) {
      epochs.push_back(e);
    }

    return epochs;
  }

  bool generate_epoch(Epoch& epoch) {
    if(gen_idx.complete())
      return false;
    
    InstructionBuffer buf;
    size_t idx;
    std::string stop_reason;
    while((idx = gen_idx.next_idx()) < program.size()) {
      switch(generate_instruction(buf, idx, stop_reason)) {
        case InstructionGenerationResult::CONTINUE:
          gen_idx.record_gen();
          break;
        case InstructionGenerationResult::SKIP:
          gen_idx.record_skip();
          break;
        case InstructionGenerationResult::STOP:
          goto end;

      }
    }
  end:
    complete_epoch(buf);

    if(gen_idx.complete()) {
      stop_reason = "generation complete.";
    }

    epoch.end_reason = stop_reason;
    epoch.modulus_table = modulus_table;
    epoch.instructions = std::move(buf);
    epoch.input_map = std::move(input_map);
    vloc.memory.extract_and_clear(epoch.output_map);
    vloc.reserved_space_values.merge(epoch.output_map);
    
    epoch_reset();
    return true;
  }

private:

  // reset state between epochs
  void epoch_reset() {
    modulus_table = {};
    vloc = {};
    input_map = {};
    outside_modulus_table = {};
    gen_idx.reset();
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
    Address mloc;
    if (vloc.memory.get_loc(v, mloc) || allocate_input(v, mloc)) {
      buf.push_inst(Instruction {}.into_load(rloc, mloc));
      return true;
    }

    return false;
  }

    // if we don't have a value in memory, this must be an input
    // TODO: maybe we could allocate this sooner - we could do a pass at the beginning
    // to figure out all the inputs and allocate space for them but we don't know
    // how many instructions we'll be generating
  bool allocate_input(ValueId v, Address& mloc) {
    if (!vloc.memory.alloc_val_fresh(v, mloc)) return false;
    input_map[v] = mloc;
    return true;
  }

  // copy all registers to memory (presumably every register is live)
  void complete_epoch(InstructionBuffer& buf) {
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

  InstructionGenerationResult generate_instruction(InstructionBuffer& buf, size_t ssa_idx, std::string& failure_reason) {
    SSAInst const& ssa = program[ssa_idx];
    Instruction inst;  // A temporary instruction
    
    // Temporary buffer for instructions -- 
    // we either want to output a complete set of instructions (including loads and stores)
    // to `buf` or none at all - so we stage instructions here
    InstructionBuffer insts;

    Register rs1;
    Register rs2;
    Register rd;
    PrimeModulusIndex modidx = 0;
    
    if(ssa.modulus != 0 && !modulus_table.try_get_modulus_index(ssa.modulus, modidx)) {
      outside_modulus_table.insert(ssa.dest);
      return InstructionGenerationResult::STOP;
    }

    if(outside_modulus_table.find(ssa.arg1) != outside_modulus_table.end() ||
       outside_modulus_table.find(ssa.arg2) != outside_modulus_table.end()) {
      outside_modulus_table.insert(ssa.dest);
      return InstructionGenerationResult::STOP;
    }

    // special case for FREE
    if(ssa.op == SSAInstOp::FREE) {
      free_value(ssa.arg1);
      return InstructionGenerationResult::CONTINUE;
    }

    // allocate any needed registers
    if(ssa.arg1 != UNDEF_VALUE_ID && !src_reg(insts, ssa.arg1, ssa_idx, rs1)) {
      failure_reason = "at memory limit (src_reg returned false)";
      return InstructionGenerationResult::STOP;
    }

    if(ssa.arg2 != UNDEF_VALUE_ID && !src_reg(insts, ssa.arg2, ssa_idx, rs2, ssa.arg1)) {
      failure_reason = "at memory limit (src_reg returned false)";
      return InstructionGenerationResult::STOP;
    }

    bool preload;
    if(ssa.dest != UNDEF_VALUE_ID && !allocate_reg(insts, ssa.dest, ssa_idx, rd, preload)) {
      failure_reason = "at memory limit (allocate_reg returned false)";
      return InstructionGenerationResult::STOP;
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
      return InstructionGenerationResult::STOP;
    }
    buf.append(insts);
    return InstructionGenerationResult::CONTINUE;
  }

  // the largest program we can create in the reserved program space, minus the amount we need to write any stores needed at the end
  const uint64_t MAX_PROGRAM_SIZE_BYTES = (BASALISC_RESERVED_PROGRAM_SIZE * BASALISC_BLOCK_SIZE) - 
                                          (BASALISC_REGISTER_COUNT * BASALISC_INSTRUCTION_SIZE_BYTES);

  
  std::vector<SSAInst> const& program;
  InstructionAnalysis analysis;
  GenerationIndex gen_idx;

  ModulusTable modulus_table;
  ValueLoc vloc;
  std::unordered_map<ValueId, Address> input_map;
  std::set<ValueId> outside_modulus_table;
};


