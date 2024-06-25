SSAInst::SSAInst(SSAInstOp op, SymbolicValue const& arg1, SymbolicValue const& arg2, NativeInteger m)
  : op {op}, arg1 { arg1.value() }, arg2 { arg2.value() }, modulus { m }
{
  assert(m != 0);
  Basalisc.freeze_value(arg1);
  Basalisc.freeze_value(arg2);
}

SSAInst::SSAInst(SSAInstOp op, SymbolicValue const& arg, NativeInteger i, NativeInteger m)
  : op {op}, arg1 { arg.value() }, imm{i}, modulus { m }
{
  assert(m != 0);
  Basalisc.freeze_value(arg);
}

SSAInst::SSAInst(SSAInstOp op, SymbolicValue const& arg1, SymbolicValue const& arg2, NativeInteger i, NativeInteger m)
  : op {op}, arg1 { arg1.value() }, arg2 { arg2.value() }, imm { i }, modulus { m }
{
  assert(m != 0);
  Basalisc.freeze_value(arg1);
  Basalisc.freeze_value(arg2);
}

SSAInst::SSAInst(SSAInstOp op, SymbolicValue const& arg, NativeInteger m)
  : op {op}, arg1 { arg.value() }, modulus { m }
{
  assert(m != 0);
  Basalisc.freeze_value(arg);
}

SSAInst::SSAInst(SSAInstOp op, SymbolicValue const& arg)
  : op {op}, arg1 {arg.value()}
{
  Basalisc.freeze_value(arg);
}

SSAInst::SSAInst(SSAInstOp op, SymbolicValue const& arg, AutomorphismNumber n, NativeInteger m)
  : op { op }, arg1 { arg.value() }, imm { NativeInteger { n } }, modulus { m }
{
  assert(m != 0);
  Basalisc.freeze_value(arg);
}


