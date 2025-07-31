#include "PulseMacros.h"

RETURNS(y:Int32.t)
ENSURES(pure (Int32.v y > 0))
int abs(int x);