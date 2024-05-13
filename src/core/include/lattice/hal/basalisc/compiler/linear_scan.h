#pragma once

#include <map>
#include <unordered_map>

#include "constants.h"
#include "ssa.h"

inline
std::unordered_map<ValueId,ssize_t>
  find_endpoints(std::vector<SSAInst> const& ssa) {

  std::unordered_map<ValueId,ssize_t> endpoint;

  auto use = [&](ValueId v, size_t i) {
    if (v == UNDEF_VALUE_ID) return;
    endpoint[v] = -static_cast<ssize_t>(i) - 1;
  };

  for (size_t i = 0; i < ssa.size(); ++i) {
    auto& inst = ssa[i];
    if (inst.op == FREE) {
      auto& last = endpoint[inst.arg1];
      last = -(last + 1);
    } else {
      use(inst.arg1,i);
      use(inst.arg2,i);
      endpoint[inst.dest] = 0;
    }
  }

  return endpoint;
}

inline
void linear_scan
  ( std::vector<SSAInst> const& ssa,
    /* Instructions */

    std::unordered_map<ValueId,ssize_t> const& endpoint,
    /* Instruction index of last use. -ve means no last use. */

    Address stack_start,
    /* Spill in sequential addresses starting with this one. */

    std::unordered_map<ValueId,RegisterOrAddress>& value_location
    /* Place computed locations here */

  ) {

  std::array<bool, BASALISC_REGISTER_COUNT> used_regs{};

  // Reserve first two registers for bringing in spilled values.
  used_regs[0] = true;
  used_regs[1] = true;

  auto next_stack = RegisterOrAddress::from_address(stack_start);

  // Values currently in regigster.
  // Maps `last_use instruction` to`value.
  std::multimap<size_t, ValueId> active;


  for (size_t i = 0; i < ssa.size(); ++i) {

    // Remove active things whose last use is before this instruction.
    auto const last = active.lower_bound(i);
    for (auto j = active.begin(); j != last; ++j) {
      auto const r = value_location.at(j->second);
      used_regs[r.as_register()] = false;
    }
    active.erase(active.begin(),last);

    auto const& inst = ssa[i];
    if (inst.op == FREE) continue;

    auto const endpoint_for = [&](ValueId v) {
      ssize_t const signed_end = endpoint.at(v);
      return signed_end < 0? SIZE_MAX: signed_end;
    };

    auto const f = [&](ValueId v) {
      if (v == UNDEF_VALUE_ID) return;

      size_t v_endpoint = endpoint_for(v);
      if (value_location.find(v) == value_location.end()) // first use.
      {
        auto const rit = std::find(used_regs.begin(), used_regs.end(), false);
        if (rit != used_regs.end()) {
          // We have a register
          auto reg = rit - used_regs.begin();
          value_location.insert({v,RegisterOrAddress::from_register(reg)});
          used_regs[reg] = true;
          active.insert({v_endpoint,v});
        } else {
          // We don't have a register, we need to spill
          auto spill = --active.end();
          auto [spill_endpoint, spill_v] = *spill;
          if (spill_endpoint > v_endpoint) {
            // Plase last in memory, use its register for this value
            RegisterOrAddress r = value_location.at(spill_v);
            value_location.insert({spill_v,next_stack});
            next_stack = next_stack.next();
            value_location.insert({v,r});
            active.erase(spill);
            active.insert({v_endpoint,v});
          } else {
            // Place value in memory
            value_location.insert({v,next_stack});
            next_stack = next_stack.next();
          }
        }
      }
    };
    f(inst.arg1);
    f(inst.arg2);
  }
}


