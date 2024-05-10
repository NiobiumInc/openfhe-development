#ifndef LBCRYPTO_INC_LATTICE_HAL_BASALISC_COMPILER_H
#define LBCRYPTO_INC_LATTICE_HAL_BASALISC_COMPILER_H


#include <filesystem>
#include <sstream>
#include <fstream>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>

#include "compiler/constants.h"
#include "compiler/instruction.h"
#include "compiler/symbolic.h"
#include "compiler/ssa.h"
#include "compiler/modulus_table.h"
#include "compiler/evaluator.h"
#include "compiler/instruction_generator.h"
#include "compiler/linear_scan.h"


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
      not_available("get_values() called on symbolic value");
    }
    return p->second;
  }

  NativeVector& get_values_mut(SymbolicValue& s) {
    bool is_mod = m_modifiable_concrete.find(s.value) != m_modifiable_concrete.end();
    if(is_mod) {
      return m_concrete_polys[s.value];
    } else {
      not_available("get_values_mut() called on symbolic or frozen value");
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

    void display(std::ostream& os = std::cout) {
      os << "host_to_basalisc_bytes: " << host_to_basalisc_bytes << std::endl;
      os << "basalisc_to_host_bytes: " << basalisc_to_host_bytes << std::endl;
      os << "epoch_count: " << epoch_count << std::endl;

      os << std::endl;
      os << "instruction freq:" << std::endl;
      for(auto const& opfreq : opcode_freq) {
        os << Instruction::pp_opcode(opfreq.first) << ": " <<  opfreq.second << std::endl;
      }

      os << "modulus_table size: " << modulus_table_size << std::endl;

      os << std::endl;
    }
  };

  void write_program(std::filesystem::path const& path) {
    std::ofstream f { path };
    if(!f) {
      not_available("could not open file: '" + path.string() + "' " );
    }
    display_program(f);
    f.close();
  }

  void display_program(std::ostream& os) const {
    // make copies of context stuff
    std::vector<SSAInst> inst = m_inst;
    ModulusTable mt { modulus_table };
    ValueLoc vl { vloc };

    // generate instructions
    auto epoch = InstructionGenerator::generate_instructions(inst, mt, vl);

    modulus_table.display(os);
    os << std::endl << std::endl;
    epoch.instructions.display(os);
  }

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


#endif
