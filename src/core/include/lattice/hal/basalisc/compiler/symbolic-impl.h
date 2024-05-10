#include "symbolic.h"


// -----------------------------------------------------------------------------
// SymbolicValue
// -----------------------------------------------------------------------------

SymbolicValue::SymbolicValue(ValueId value): value {value}
{
  Basalisc.increment_refcount(value);
}

SymbolicValue::SymbolicValue(SymbolicValue const& s)
{
  Basalisc.increment_refcount(s.value);
  value = s.value;
}

SymbolicValue::SymbolicValue(SymbolicValue&& s) noexcept
{
  value = s.value;
}

SymbolicValue& SymbolicValue::operator=(SymbolicValue const& other)
{
  if (other.value != value) {
    Basalisc.decrement_refcount(value);
    *this = SymbolicValue(other);
  }
  return *this;
}

SymbolicValue& SymbolicValue::operator=(SymbolicValue&& other) noexcept
{
  if (other.value != value) {
    Basalisc.decrement_refcount(value);
    value = other.value;
  }
  return *this;
}

SymbolicValue::~SymbolicValue()
{
  Basalisc.decrement_refcount(value);
}


