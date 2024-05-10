#pragma once


using ValueId = uint64_t;
const ValueId UNDEF_VALUE_ID = 0;
const ValueId ZERO_VALUE_ID = 1;

struct SymbolicValue {
  constexpr SymbolicValue(): value{UNDEF_VALUE_ID} {};
  explicit SymbolicValue(ValueId value);
  SymbolicValue(SymbolicValue const& s);
  SymbolicValue(SymbolicValue&& s) noexcept;
  SymbolicValue& operator=(SymbolicValue const& other);
  SymbolicValue& operator=(SymbolicValue&& other) noexcept;
  ~SymbolicValue();

  NativeInteger const& operator[](usint i) const;

  ValueId value;
};


