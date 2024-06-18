#pragma once

#include <map>
#include <set>

#include "symbolic.h"
#include <cassert>

class AllocationTable {
public:
  AllocationTable(AllocationTableSize size)
    : loc_to_value(size, UNDEF_VALUE_ID) {
      unallocated = size;
  }

  // Total number of slots
  AllocationTableSize total_slots() const { return loc_to_value.size(); }

  // How many slots are allocated at the moment.
  AllocationTableSize allocated_slots() const { return total_slots() - (free_list.size() + unallocated); }

  // Get the location for the given value.
  // Returns `true` and updates `rloc` if we have it,
  // otherwise return `false`.
  bool get_loc(ValueId id, Location & rloc) const {
    auto l = value_to_loc.find(id);
    if (l != value_to_loc.end()) {
      rloc = l->second;
      return true;
    }

    return false;
  }


  // Replace the value in a given slot.
  void replace_val(Location slot, ValueId val_old, ValueId val_new) {
    value_to_loc.erase(val_old);
    value_to_loc.insert({val_new,slot});
    assert(slot >= 0 && slot < loc_to_value.size());
    loc_to_value[slot] = val_new;
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
    auto slot = loci->second;
    loc_to_value[slot] = UNDEF_VALUE_ID;
    value_to_loc.erase(loci);
    free_list.push_back(slot);
  }

  // Free a location and update value maps, if needed.
  void free_slot(Location loc) {
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
  bool alloc_slot(Location& loc) {
    if (free_list.size() != 0) {
      loc = free_list.back();
      free_list.pop_back();
      return true;
    } else if(alloc_fresh(loc)) {
      return true;
    }
    return false;
  }

  // Allocate a slot for a value.  If one is available return true and set `loc`,
  // and update the value maps.
  bool alloc_val(ValueId val, Location& loc) {
    if (!alloc_slot(loc)) return false;
    loc_to_value[loc] = val;
    value_to_loc.insert({val,loc});
    return true;
  }

  // Allocate a slot that has never been allocated before
  // If a fresh slot is available, set `loc` and return true
  bool alloc_fresh(Location& loc) {
    if(unallocated <= 0) {
      return false;
    }
    loc = --unallocated;
    return true;
  }

  // Allocate a slot to a value that has never been allocated before
  // If a fresh slot is available, set `loc` and return true, updating the value
  // maps accordingly
  bool alloc_val_fresh(ValueId val, Location& loc) {
    if(alloc_fresh(loc)) {
      loc_to_value[loc] = val;
      value_to_loc[val] = loc;
      return true;
    }

    return false;
  }

  // clear this allocation table, merging its value map with the argument
  void extract_and_clear(std::unordered_map<ValueId, Location>& map) {
    AllocationTableSize size = total_slots();
    map.merge(std::move(value_to_loc));
    reset(size);
  }

  // clear this allocation table
  void clear() {
    reset(total_slots());
  }

private:
  void reset(AllocationTableSize size) {
    value_to_loc = {};
    free_list = {};
    unallocated = size;
  }

  std::unordered_map<ValueId, Location> value_to_loc;

  // disjoint:
  std::vector<ValueId> loc_to_value;          // allocated
  std::vector<Location> free_list;              // unallocated
  Location unallocated;                         // slots lower than this have never been allocated before
};


