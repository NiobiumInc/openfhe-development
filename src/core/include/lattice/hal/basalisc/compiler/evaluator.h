
#include"lattice/hal/default/poly.h"
#include "constants.h"
#include "modulus_table.h"
#include "instruction.h"

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
          lbcrypto::PolyImpl<NativeVector> poly;
          poly.SetValues(v1, Format::EVALUATION);
          poly.AutomorphismTransform(i.automorphism());
          storevec(regs[i.rd()], poly.GetValues());
          break;
        }
        case Instruction::Opcode::NTT1:
          break;
        case Instruction::Opcode::NTT2: {
          loadvec(v1, regs[i.rs1()], mt.get_modulus(i.mod_index()));
          lbcrypto::PolyImpl<NativeVector> poly;
          poly.SetValues(v1, Format::COEFFICIENT);
          poly.SwitchFormat();
          storevec(regs[i.rd()], poly.GetValues());
          break;
        }
        case Instruction::Opcode::INTT1:
          break;
        case Instruction::Opcode::INTT2: {
          loadvec(v1, regs[i.rs1()], mt.get_modulus(i.mod_index()));
          lbcrypto::PolyImpl<NativeVector> poly;
          poly.SetValues(v1, Format::EVALUATION);
          poly.SwitchFormat();
          storevec(regs[i.rd()], poly.GetValues());
          break;
        }
        case Instruction::Opcode::FENCE:
          break;
        case Instruction::Opcode::RNGCONFIG:
          not_implemented("rng ops not implemenred");
        case Instruction::Opcode::RNGSETUP:
          not_implemented("rng ops not implemenred");
        case Instruction::Opcode::RNGGENERATE:
          not_implemented("rng ops not implemenred");
        default:
          not_implemented("BUG: missing instruction");
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


