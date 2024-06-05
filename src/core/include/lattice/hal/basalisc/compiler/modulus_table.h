#pragma once

#include <iostream>
#include "constants.h"
#include <cassert>

struct ModulusTable {
  std::vector<uint64_t> modulus_table;

  // PrimeModulusIndex get_modulus_index(uint64_t modulus) {
  //   for(PrimeModulusIndex i = 0; i < modulus_table.size(); i++) {
  //     if(modulus == modulus_table[i]) {
  //       return i;
  //     }
  //   }

  //   // XXX: Do not limit the table size, temporarily.
  //   if(true || modulus_table.size() < BASALISC_MODULUS_TABLE_SIZE) {
  //     modulus_table.push_back(modulus);
  //     return modulus_table.size() - 1;
  //   }

  //   panic("Modulus table size exceeded");
  // }

  bool try_get_modulus_index(uint64_t modulus, PrimeModulusIndex& idx) {
    assert(modulus != 0);
    for(PrimeModulusIndex i = 0; i < modulus_table.size(); i++) {
      if(modulus == modulus_table[i]) {
        idx = i;
        return true;
      }
    }

    if(modulus_table.size() < BASALISC_MODULUS_TABLE_SIZE) {
      modulus_table.push_back(modulus);
      idx = modulus_table.size() - 1;
      return true;
    }

    return false;
  }

  uint64_t get_modulus(PrimeModulusIndex const& idx) const {
    if(idx >= modulus_table.size())
      panic("index exceeds modulus table size");

    return modulus_table[idx];
  }

  size_t size() const {
    return modulus_table.size();
  }

  bool operator==(ModulusTable const& other) const {
    return modulus_table == other.modulus_table;
  }

  void display(std::ostream& os = std::cout) const {
    for(size_t i = 0; i < modulus_table.size(); i++) {
      os << "modulus_table[" << i << "] = " << modulus_table[i] << std::endl;
    }
  }
};


