#include "../pulse_macros.h"
	
ERASED_ARG(#w:_)
REQUIRES(r |-> w)
RETURNS(v:Int32.t)
ENSURES(r |-> w)
ENSURES(pure (v == w))
int value_of_explicit(int *r)
{
    return *r;
}
