#include"lattice/hal/basalisc/compiler.h"

namespace lbcrypto {


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

// -----------------------------------------------------------------------------
// SSAInst
// -----------------------------------------------------------------------------
SSAInst::SSAInst(SSAInstOp op, SymbolicValue const& arg1, SymbolicValue const& arg2, NativeInteger const& m)
  : op {op}, arg1 { arg1.value }, arg2 { arg2.value }
{
  modulus = Basalisc.modulus_index(m);
  Basalisc.freeze_value(arg1.value);
  Basalisc.freeze_value(arg2.value);
}

SSAInst::SSAInst(SSAInstOp op, SymbolicValue const& arg, NativeInteger const& i, NativeInteger const& m)
  : op {op}, arg1 { arg.value }
{
  modulus = Basalisc.modulus_index(m);
}

SSAInst::SSAInst(SSAInstOp op, SymbolicValue const& arg1, SymbolicValue const& arg2, NativeInteger const& i, NativeInteger const& m)
  : op {op}, arg1 { arg1.value }, arg2 { arg2.value }, imm { i }
{
  modulus = Basalisc.modulus_index(m);
}

SSAInst::SSAInst(SSAInstOp op, SymbolicValue& arg, NativeInteger const& m)
  : op {op}, arg1 { arg.value }
{
  modulus = Basalisc.modulus_index(m);
}

} // namespace lbcrypto
