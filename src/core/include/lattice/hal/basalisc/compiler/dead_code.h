#pragma once

#include <vector>
#include "ssa.h"
#include "linear_scan.h"

inline void remove_dead(std::vector<SSAInst> &ssa) {
  auto n = ssa.size();
  if (n == 0) return;

  std::unordered_map<ValueId, size_t> dead;
  auto use = [&](ValueId v) {
    if (v == UNDEF_VALUE_ID) return;
    dead.erase(v);
  };

  for (size_t i = n; i > 0; --i) {
    auto j = i - 1;
    auto& inst = ssa[j];
    if (inst.op == FREE) {
      dead.insert({inst.arg1, j});
    } else {
      if (inst.dest == UNDEF_VALUE_ID) continue;
      auto found = dead.find(inst.dest);
      if (found == dead.end()) {
        // not dead
        use(inst.arg1);
        use(inst.arg2);
      } else {
        // dead
        ssa[found->second].into_nop();
        dead.erase(found);
        inst.into_nop();
      }
    }
  }

  // dead constants (should probably also delete)
  for (auto& v : dead) {
    ssa[v.second].into_nop();
  }
}
