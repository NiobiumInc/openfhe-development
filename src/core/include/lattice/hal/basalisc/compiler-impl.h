#include"lattice/hal/basalisc/compiler.h"

namespace lbcrypto {

SymbolicValue::SymbolicValue(uint64_t value): value {value} {
  Basalisc.increment_refcount(value);
}

SymbolicValue::SymbolicValue(SymbolicValue const& s) {
  Basalisc.increment_refcount(s.value);
  value = s.value;
}

SymbolicValue::SymbolicValue(SymbolicValue&& s) noexcept {
  value = s.value;
}

SymbolicValue& SymbolicValue::operator=(SymbolicValue const& other)
{
  return *this = SymbolicValue(other);
}

SymbolicValue& SymbolicValue::operator=(SymbolicValue&& other) noexcept
{
  value = other.value;
  return *this;
}

SymbolicValue::~SymbolicValue() {
  Basalisc.decrement_refcount(value);
}

SSAInst::SSAInst(SSAInstOp op, SymbolicValue const& arg1, SymbolicValue const& arg2, NativeInteger const& m): op {op}, arg1 { arg1.value }, arg2 { arg2.value } { 
  modulus = Basalisc.modulus_index(m);
}

SSAInst::SSAInst(SSAInstOp op, SymbolicValue const& arg, Immediate i, NativeInteger const& m): op {op}, arg1 { arg.value } { 
  modulus = Basalisc.modulus_index(m);
}

SSAInst::SSAInst(SSAInstOp op, SymbolicValue const& arg1, SymbolicValue const& arg2, Immediate i, NativeInteger const& m): op {op}, arg1 { arg1.value }, arg2 { arg2.value }, imm { i } { 
  modulus = Basalisc.modulus_index(m);
}

SSAInst::SSAInst(SSAInstOp op, SymbolicValue& arg, NativeInteger const& m): op {op}, arg1 { arg.value } { 
  modulus = Basalisc.modulus_index(m);
}

} // namespace lbcrypto