#pragma once

#include <memory>

using ValueId = uint64_t;
const ValueId UNDEF_VALUE_ID = 0;
const ValueId ZERO_VALUE_ID = 1;

struct SymVal {
  ValueId value;
  constexpr SymVal(): value{UNDEF_VALUE_ID} {};
  explicit SymVal(ValueId value): value{value} {};
  ~SymVal();
};

class SymbolicValue {
  std::shared_ptr<SymVal> ref;
  inline static std::shared_ptr<SymVal> undef_ref;
public:
  SymbolicValue(ValueId value) : ref{std::make_shared<SymVal>(value)} {}
  SymbolicValue() {
    if (!undef_ref) undef_ref = std::make_shared<SymVal>(UNDEF_VALUE_ID);
    ref = undef_ref;
  }
  ValueId value() const { return ref->value; }
};


