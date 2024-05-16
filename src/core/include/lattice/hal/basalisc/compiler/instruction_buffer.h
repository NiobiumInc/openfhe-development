#pragma once

#include "instruction.h"

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

  void display(std::ostream& os = std::cout) {
    for(size_t i = 0; i < inst_buf.size(); i++) {
      if(!inst_buf_is_inst[i])
        continue;

      Instruction inst = Instruction { inst_buf[i] };
      uint64_t imm = inst.has_imm() ? inst_buf[i + 1]: 0;
      switch(inst.opcode()) {
        case Instruction::Opcode::LOAD:
          display_instruction(os, inst.opcode(), 
            { pp_reg(inst.rd()), 
              pp_addr(inst.load_addr()) });
          break;

        case Instruction::Opcode::STORE:
          display_instruction(os, inst.opcode(), 
            { pp_addr(inst.load_addr()),
              pp_reg(inst.rs1())});
          break;

        case Instruction::Opcode::ADD:
        case Instruction::Opcode::SUB:
        case Instruction::Opcode::MUL:
          display_instruction(os, inst.opcode(), 
            { pp_reg(inst.rd()), 
              pp_reg(inst.rs1()), 
              pp_reg(inst.rs2()), 
              pp_mod_index(inst.mod_index())});
          break;

        case Instruction::Opcode::ADDI:
        case Instruction::Opcode::SUBI:
        case Instruction::Opcode::MULI:
          display_instruction(os, inst.opcode(), 
            { pp_reg(inst.rd()), 
              pp_reg(inst.rs1()), 
              pp_imm(imm), 
              pp_mod_index(inst.mod_index())});
          break;

        case Instruction::Opcode::ADDMULI:
          display_instruction(os, inst.opcode(), 
            { pp_reg(inst.rd()), 
              pp_reg(inst.rs1()), 
              pp_reg(inst.rs2()), 
              pp_imm(imm), 
              pp_mod_index(inst.mod_index())});
          break;

        case Instruction::Opcode::NTT1:
        case Instruction::Opcode::NTT2:
        case Instruction::Opcode::INTT1:
        case Instruction::Opcode::INTT2:
          display_instruction(os, inst.opcode(), 
            { pp_reg(inst.rd()), 
              pp_reg(inst.rs1()), 
              pp_mod_index(inst.mod_index())});
          break;

        case Instruction::Opcode::MORPH1:
        case Instruction::Opcode::MORPH2:
          display_instruction(os, inst.opcode(), 
            { pp_reg(inst.rd()), 
              pp_reg(inst.rs1()), 
              pp_autonum(inst.automorphism())});
          break;

        case Instruction::Opcode::FENCE:
          display_instruction(os, inst.opcode(), {});
          break;

        case Instruction::Opcode::RNGCONFIG:
          display_instruction(os, inst.opcode(), 
            { pp_rng_sram(inst.rd()),
              pp_imm(imm),
              pp_mod_index(inst.mod_index())});
          break;
        case Instruction::Opcode::RNGSETUP:
          display_instruction(os, inst.opcode(),
            { pp_mod_index(inst.mod_index())});
          break;
        case Instruction::Opcode::RNGGENERATE:
          display_instruction(os, inst.opcode(),
            { pp_reg(inst.rd())});
          break;

        default:
          not_implemented("InstructionBuffer::display_instruction not implemented for opcode: " + std::to_string(inst.opcode()));
      }
    }
  }
  
private:
  void display_instruction(std::ostream& os, Instruction::Opcode const& c, std::initializer_list<std::string> const& args) {
    bool first = true;
    os << Instruction::pp_opcode(c) << " ";
    for(auto const& arg: args) {
      if(!first) {
        os << ", ";
      } else {
        first = false;
      }

      os << arg;
    }

    os << std::endl;
  }
};


