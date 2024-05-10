#pragma once

#include "ssa.h"

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
        panic("BUG: InstructionAnalysis is not complete");

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
      panic("BUG: empty vals");
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


