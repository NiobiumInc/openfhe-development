#pragma once

#include <map>
#include <set>

#include "symbolic.h"

class AllocationTable {
public:
  AllocationTable(size_t size)
    : loc_to_value(size, UNDEF_VALUE_ID), free_list(size) {
    for(size_t i = 0; i < size; i++) {
      free_list[i] = i;
    }
  }

  // Total number of slots
  size_t total_slots() const { return loc_to_value.size(); }

  // How many slots are allocated at the moment.
  size_t allocated_slots() const { return total_slots() - free_list.size(); }

  // Get the location for the given value.
  // Returns `true` and updates `rloc` if we have it,
  // otherwise return `false`.
  bool get_loc(ValueId id, size_t& rloc) const {
    auto l = value_to_loc.find(id);
    if (l != value_to_loc.end()) {
      rloc = l->second;
      return true;
    }

    return false;
  }

  // Get the slot allocation map.  Slots that are allocated but not
  // occupied by values (e.g., code) contain UNDEF_VALUE_ID
  std::vector<ValueId> const& get_slot_map() const {
    return loc_to_value;
  }

  // Free the location for the given value.
  void free_val(ValueId val) {
    auto loci = value_to_loc.find(val);
    if (loci == value_to_loc.end()) return;
    loc_to_value[loci->second] = UNDEF_VALUE_ID;
    value_to_loc.erase(loci);
  }

  // Free a location and update value maps, if needed.
  void free_slot(size_t loc) {
    free_list.push_back(loc);
    auto v = loc_to_value[loc];
    if (v != UNDEF_VALUE_ID) {
      loc_to_value[loc] = UNDEF_VALUE_ID;
      value_to_loc.erase(v);
    }
  }

  // Allocate a slot.  If one is available return true and set `loc`.
  // This does not update the value indexes, so the slot may be used
  // for a value or for code.
  bool alloc_slot(size_t& loc) {
    if (free_list.size() == 0) return false;
    loc = free_list.back();
    free_list.pop_back();
    return true;
  }

  // Allocate a slot for a value.  If one is availabe return true and set `loc`,
  // and update the value maps.
  bool alloc_val(ValueId val, size_t& loc) {
    if (!alloc_slot(loc)) return false;
    loc_to_value[loc] = val;
    value_to_loc.insert({val,loc});
    return true;
  }


private:
  std::unordered_map<ValueId, size_t> value_to_loc;

  // disjoint:
  std::vector<ValueId> loc_to_value;          // allocated
  std::vector<size_t> free_list;              // unallocated
};


