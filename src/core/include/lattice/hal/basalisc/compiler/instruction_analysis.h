#pragma once

#include "ssa.h"

class InstructionAnalysis {
public:
  struct Eviction {
    Eviction() {
      freed = UNDEF_VALUE_ID;
      slot = 0;
    }

    Eviction(size_t slot, ValueId fr): slot { slot }, freed { fr } {

    }

    size_t slot;
    ValueId freed;
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

  // For a set of values, beginning at SSA 
  // If `avoid` is defined, then we can't evict it.
  Eviction find_eviction_candidate(std::vector<ValueId> const& vals, size_t ssa_idx, ValueId avoid) const {
    bool first = true;
    size_t candidate_slot = 0;
    size_t candidate_next_use = 0;
    int candidate_uses = 0;

    auto reg_num = vals.size();
    for(size_t slot = 0; slot < reg_num; ++slot) {
      ValueId v = vals[slot];
      if (v == UNDEF_VALUE_ID || v == avoid) continue;

      auto uses = val_uses.find(v);
      auto freed = val_free.find(v);
      if(uses == val_uses.end()) {
        // We found something that is allocated, but never used, but
        // also not freed so it won't be considered to be dead code.
        // Seems like a good candidate for evication.
        return Eviction { slot, v };
      }

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
        return Eviction { slot, v };
      }
      
      if(first) {
        candidate_slot = slot;
        candidate_uses = use_count;
        candidate_next_use = next_use;
        first = false;
      } else if(candidate_next_use < ssa_idx + 20 && candidate_next_use < next_use ) {
        // if something is going to be used imminently, try to find an alternative to evicting it
        candidate_slot = slot;
        candidate_uses = use_count;
        candidate_next_use = next_use;
      } else if(use_count < candidate_uses) {
        // prefer the value that's used the least frequently for eviction
        candidate_slot = slot;
        candidate_uses = use_count;
        candidate_next_use = next_use;
      } else if(use_count == candidate_uses && next_use < candidate_next_use) {
        // prefer the value that will be used later for eviction
        candidate_slot = slot;
        candidate_uses = use_count;
        candidate_next_use = next_use;
      }
    }

    if(first) {
      panic("BUG: empty vals");
    }

    return Eviction { candidate_slot, UNDEF_VALUE_ID };
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


