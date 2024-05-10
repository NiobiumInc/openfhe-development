#pragma once

#include <string>
#include "utils/exception.h"

const size_t BASALISC_MEMORY_SIZE_GB      = 128 * 100; // XXX
const size_t BASALISC_REGISTER_COUNT      = 128;
const size_t BASALISC_MODULUS_TABLE_SIZE  = 32;
const size_t BASALISC_POLY_LENGTH         = (1 << 16);

// number of vector-sized blocks the memory contains
const size_t BASALISC_BLOCK_SIZE  = 8 * BASALISC_POLY_LENGTH;
const size_t BYTES_PER_GB         = 1024 * 1024 * 1024;
const size_t BASALISC_MEMORY_SIZE_BLOCKS =
  (BASALISC_MEMORY_SIZE_GB * BYTES_PER_GB) / BASALISC_BLOCK_SIZE;



using Register = uint64_t;
using Address = uint64_t;
using Immediate = uint64_t;
using PrimeModulusIndex = uint8_t;
using AutomorphismNumber = uint32_t;
using RngConfig = uint64_t;

class RegisterOrAddress {
  int64_t value;
  RegisterOrAddress(int64_t x) : value(x) {}

public:
  static RegisterOrAddress from_register(Register x) {
    return RegisterOrAddress(static_cast<int64_t>(x));
  }

  static RegisterOrAddress from_address(Address x) {
    return RegisterOrAddress(- static_cast<int64_t>(x) - 1);
  }

  RegisterOrAddress next() const {
    return RegisterOrAddress(is_register() ? (value+1) : (value-1));
  }

  bool is_register() const { return value >= 0; }
  bool is_address()  const { return value < 0; }

  Register as_register() const { return value; }
  Address  as_address()  const { return -value-1; }

};




/* Exceptions */

[[noreturn]]
inline void not_implemented(std::string err) {
  OPENFHE_THROW(lbcrypto::not_implemented_error, err);
}

[[noreturn]]
inline void not_available(std::string err) {
  OPENFHE_THROW(lbcrypto::not_available_error, err);
}

[[noreturn]]
inline void panic(std::string err) {
  OPENFHE_THROW(lbcrypto::openfhe_error, err);
}




/* Pretty Printing */

inline std::string pp_reg(Register const& c) {
  return "r" + std::to_string(c);
}

inline std::string pp_addr(Address const& a) {
  std::stringstream stream;
  stream << "$" << std::setfill('0') << std::setw(16) << std::hex << a;
  return stream.str();
}

inline std::string pp_mod_index(PrimeModulusIndex const& i) {
  return "mod" + std::to_string(i);
}

inline std::string pp_imm(uint64_t i) {
  return "imm" + std::to_string(i);
}

inline std::string pp_autonum(AutomorphismNumber const& n) {
  return "auto" + std::to_string(n);
}

inline std::string pp_rng_sram(uint64_t i) {
  return "o" + std::to_string(i);
}


