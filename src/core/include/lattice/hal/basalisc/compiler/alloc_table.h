#pragma once

#include <map>
#include <set>

#include "symbolic.h"

class AllocationTable {
public:
  AllocationTable(size_t size) {
    for(size_t i = 0; i < size; i++) {
      free_list.insert(i);
    }
    wm = size;
  }

  // Is the value at the given location in this table?
  bool get_loc(ValueId id, size_t& rloc) const {
    auto l = value_to_loc.find(id);
    if(l != value_to_loc.end()) {
      rloc = l->second;
      return true;
    }

    return false;
  }

  void set_loc(ValueId id, size_t rloc) {
    free_loc(rloc);
    value_to_loc[id] = rloc;
    loc_to_value[rloc] = id;
    free_list.erase(rloc);
    wm = std::min(rloc, wm);
  }

  bool find_highest_free(size_t& rloc) const {
    auto elt = free_list.lower_bound(size());
    if(elt == free_list.end())
      return false;

    rloc = *elt;
    return true;
  }

  bool find_free(size_t& rloc) const {
    if(free_list.begin() == free_list.end()) {
      return false;
    }

    rloc = *free_list.begin();
    return true;
  }

  // Deallocate a previously allocated location.
  void free_loc(size_t t) {
    auto i = loc_to_value.find(t);
    if(i != loc_to_value.end()) {
      value_to_loc.erase(i->second);
      loc_to_value.erase(t);
    }
    free_list.insert(t);
  }

  // Deallocate a location, if it was allocated.
  void clear_loc(size_t rloc) {
    auto v = loc_to_value.find(rloc);
    if(v != loc_to_value.end()) {
      value_to_loc.erase(v->second);
      loc_to_value.erase(rloc);
      free_list.insert(rloc);
    }
  }

  // Remove the value from a location (if any),
  // and reserve it (i.e., it is not free).
  void burn_loc(size_t rloc) {
    clear_loc(rloc);
    free_list.erase(rloc);
  }

  void free_val(ValueId const& v) {
    size_t loc;
    while(get_loc(v, loc)) {
      free_loc(loc);
    }
  }

  size_t size() const {
    return free_list.size() + loc_to_value.size();
  }

  size_t watermark() const {
    return wm;
  }

  std::map<size_t, ValueId> const& get_slot_map() const {
    return loc_to_value;
  }

  bool allocate(ValueId const& val, size_t& loc) {
    if(!find_free(loc))
      return false;

    set_loc(val, loc);
    return true;
  }

private:
  size_t wm;
  std::map<ValueId, size_t> value_to_loc;
  std::map<size_t, ValueId> loc_to_value;
  std::set<size_t> free_list;
};


