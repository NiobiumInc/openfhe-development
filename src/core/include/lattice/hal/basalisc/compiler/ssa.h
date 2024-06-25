#pragma once

#include "constants.h"
#include "symbolic.h"
#include <sstream>
#include <map>

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
  NOP
};

struct SSAInst {
    SSAInst(SSAInstOp op, SymbolicValue const& arg1, SymbolicValue const& arg2, NativeInteger const& m);
    SSAInst(SSAInstOp op, SymbolicValue const& arg, NativeInteger const& i, NativeInteger const& m);
    SSAInst(SSAInstOp op, SymbolicValue const& arg1, SymbolicValue const& arg2, NativeInteger const& i, NativeInteger const& m);
    SSAInst(SSAInstOp op, SymbolicValue const& arg, NativeInteger const& m);
    SSAInst(SSAInstOp op, SymbolicValue const& arg, AutomorphismNumber n, NativeInteger const& m);
    SSAInst(SSAInstOp op, SymbolicValue const& arg);
    SSAInst(SSAInstOp op, ValueId const& arg)
    : op { op }, arg1 { arg } { }

    SSAInstOp op = SSAInstOp::TODO;
    ValueId dest = UNDEF_VALUE_ID;
    ValueId arg1 = UNDEF_VALUE_ID;
    ValueId arg2 = UNDEF_VALUE_ID; 
    NativeInteger imm = 0; // immediate or or AutomorphismNumber if it is MORPH
    PrimeModulus modulus = 0;

    void into_nop() {
      op = NOP;
      dest = UNDEF_VALUE_ID;
      arg1 = UNDEF_VALUE_ID;
      arg2 = UNDEF_VALUE_ID;
      imm  = 0;
      modulus = 0;
    }

private:
    void display_val(std::ostream& os, ValueId x) const
    { 
      os << "x" << x; 
    }

    void display_mod(std::ostream& os) const {
      os << " mod " << uint64_t{modulus};
    }
    void display_assign(std::ostream& os) const {
      display_val(os, dest); os << " = ";
    }

    void display_bin(std::ostream& os, const char* op) const {
      display_assign(os);
      os << op;
      display_val(os, arg1);
      os << " ";
      display_val(os, arg2);
      display_mod(os);
    }

    void display_bin_imm(std::ostream& os, const char* op) const {
      display_assign(os);
      os << op;
      display_val(os, arg1);
      os << " ";
      os << imm;
      display_mod(os);
    }


public:
    void display(std::ostream& os = std::cout) const {
      switch (op) {
        case TODO:    os << "TODO"; break;
        case ADD:     display_bin(os,"ADD "); break;
        case ADDI:    display_bin_imm(os,"ADDI "); break;
        case SUB:     display_bin(os,"SUB "); break;
        case SUBI:    display_bin_imm(os,"SUBI "); break;
        case MUL:     display_bin(os,"MUL "); break;
        case MULI:    display_bin_imm(os,"MULI "); break;
        case ADDMULI:
          display_assign(os);
          os << "ADDMULI ";
          display_val(os,arg1);
          os << " ";
          display_val(os,arg2);
          os << " " << imm;
          display_mod(os);
          break;

        case NTT:
          display_assign(os);
          os << "NTT ";
          display_val(os,arg1);
          display_mod(os);
          break;

        case INTT:
          display_assign(os);
          os << "NTT ";
          display_val(os,arg1);
          display_mod(os);
          break;

        case MORPH:
          display_assign(os);
          os << "MORPH " << uint64_t{imm} << " ";
          display_val(os,arg1);
          break;

        case FREE:
          os << "FREE "; display_val(os,arg1);
          break;

        case NOP:
          os << "NOP";
          break;
      }
    }
};

inline std::vector<SSAInst> ssa_subtree(std::unordered_set<ValueId> const& values_of_interest, std::vector<SSAInst> const& program) {
  std::vector<SSAInst> output;
  std::unordered_set<ValueId> vi = values_of_interest;
  for(int idx = program.size() - 1; idx >= 0; idx--) {
    SSAInst const& inst = program[idx];
    if(inst.dest != UNDEF_VALUE_ID && vi.find(inst.dest) != vi.end()) {
      if(inst.arg1 != UNDEF_VALUE_ID)
        vi.insert(inst.arg1);
      if(inst.arg2 != UNDEF_VALUE_ID)
        vi.insert(inst.arg2);
    }
  }

  for(auto const& inst : program) {
    if(inst.dest != UNDEF_VALUE_ID && vi.find(inst.dest) != vi.end()) {
      output.push_back(inst);
    }
  }

  return output;
}

inline void display_ssa_graph(std::vector<SSAInst> const& program, std::ostream& out = std::cout) {
  std::set<ValueId> computed;
  std::set<ValueId> input;
  std::unordered_map<PrimeModulus, size_t> modulus_colors {};

  std::vector<std::string> colors = {
    "#696969","#a9a9a9","#dcdcdc","#2f4f4f","#556b2f","#8b4513","#19ff70","#006400",
    "#8b0000","#808000","#483d8b","#3cb371","#bc8f8f","#663399","#008080","#bdb76b",
    "#cd853f","#4682b4","#00ff80","#d2691e","#9acd32","#cd5c5c","#32cd32","#daa520",
    "#8fbc8f","#8b008b","#b03060","#66cdaa","#ff4500","#00ced1","#ffa500","#ffd700",
    "#c71585","#0000cd","#deb887","#00ff00","#ba55d3","#00fa9a","#4169e1","#e9967a",
    "#dc143c","#00ffff","#00bfff","#9370db","#0000ff","#a020f0","#adff2f","#ff6347",
    "#d8bfd8","#ff00ff","#db7093","#f0e68c","#ffff54","#6495ed","#dda0dd","#90ee90",
    "#87ceeb","#ff1493","#afeeee","#ee82ee","#7fffd4","#ff69b4","#ffe4c4","#ffb6c1",
  };

  out << "digraph {\n";

  // nodes
  for(SSAInst const& inst: program) {
    std::stringstream inst_display;
    inst.display(inst_display);

    std::string modulus_color = "#ffffff";
    if(inst.modulus != 0) {
      auto mc = modulus_colors.find(inst.modulus);
      if(mc == modulus_colors.end()) {
        modulus_colors[inst.modulus] = modulus_colors.size();
      }
      modulus_color = colors[modulus_colors[inst.modulus]];
    } 

    if(inst.arg1 != UNDEF_VALUE_ID && computed.find(inst.arg1) == computed.end() && input.find(inst.arg1) == input.end()) {
      input.insert(inst.arg1);
      out << "  " << inst.arg1 << " [label=\"" << "x" << inst.arg1 << " input" << "\"]" << std::endl;
    }

    if(inst.arg2 != UNDEF_VALUE_ID && computed.find(inst.arg2) == computed.end() && input.find(inst.arg2) == input.end()) {
      input.insert(inst.arg2);
      out << "  " << inst.arg2 << " [label=\"" << "x" << inst.arg1 << " input" << "\"]" << std::endl;
    }

    if(inst.dest != UNDEF_VALUE_ID) {
      computed.insert(inst.dest);
      out << "  " << inst.dest << " [style=\"filled\", label=\"" << inst_display.str() << "\", fillcolor=\"" << modulus_color << "\"]" << std::endl;
    }


  }
  out << std::endl << std::endl;

  // edges
  for(SSAInst const& inst: program) {
    if(inst.dest != UNDEF_VALUE_ID) {
      if(inst.arg1 != UNDEF_VALUE_ID) {
        out << "  " << inst.arg1 << " -> " << inst.dest << std::endl;
      }

      if(inst.arg2 != UNDEF_VALUE_ID) {
        out << "  " << inst.arg2 << " -> " << inst.dest << std::endl;
      }
    }
  }

  out << "}\n";
}

inline void collect_ssa_moduli(std::vector<SSAInst> const& program, std::unordered_map<ValueId, size_t> const& value_map, size_t idx, std::unordered_map<PrimeModulus, ValueId>& moduli) {
  auto const& ssa = program[idx];
  for(ValueId arg : {ssa.arg1, ssa.arg2}) {
    if(arg != UNDEF_VALUE_ID) {
      auto const& value = value_map.find(arg);
      if(value != value_map.end()) {
        collect_ssa_moduli(program, value_map, value->second, moduli);
      }
    }
  }

  if(ssa.modulus != 0) {
    moduli[ssa.modulus] = ssa.dest;
  }
}

inline void display_ssa_with_modulus_depth(std::vector<SSAInst> const& program, std::ostream& out = std::cout, bool skip_free = true) {
  std::unordered_map<ValueId, size_t> inst_map;

  for(size_t idx = 0; idx < program.size(); idx++) {
    auto const& ssa = program[idx];
    if(ssa.op == SSAInstOp::FREE && skip_free)
      continue;

    std::unordered_map<PrimeModulus, ValueId> moduli;
    collect_ssa_moduli(program, inst_map, idx, moduli);

    ssa.display(out);
    out << " [depth: " << moduli.size() << "]"; 
    for(auto const& mod_val : moduli) {
      out << " " << mod_val.second << "(" << mod_val.first << ")";
    }
    out << "\n";

    if(ssa.dest != UNDEF_VALUE_ID)
      inst_map[ssa.dest] = idx;

    out << std::flush;
  }
}


