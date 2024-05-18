#include "symbolic.h"


// -----------------------------------------------------------------------------
// SymbolicValue
// -----------------------------------------------------------------------------

SymbolicValue::SymbolicValue(ValueId value): value {value} {}

SymbolicValue::SymbolicValue(SymbolicValue const& s) : value {s.value}
{
  Basalisc.increment_refcount(s.value);
}

SymbolicValue::SymbolicValue(SymbolicValue&& s) noexcept
  : value(s.value)
{
  s.value = UNDEF_VALUE_ID;
}

SymbolicValue& SymbolicValue::operator=(SymbolicValue const& other)
{
  Basalisc.decrement_refcount(value);
  Basalisc.increment_refcount(other.value);
  value = other.value;
  return *this;
}

SymbolicValue& SymbolicValue::operator=(SymbolicValue&& other) noexcept
{
  Basalisc.decrement_refcount(value);
  value = other.value;
  other.value = UNDEF_VALUE_ID;
  return *this;
}

SymbolicValue::~SymbolicValue()
{
  Basalisc.decrement_refcount(value);
}


