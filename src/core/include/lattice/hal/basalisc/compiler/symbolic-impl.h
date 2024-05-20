#include "symbolic.h"


SymVal::~SymVal()
{
  if (value == UNDEF_VALUE_ID) return;
  Basalisc.free_sym_val(value);
}


