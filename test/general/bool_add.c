#include <stdbool.h>
#include "../include/PulseMacros.h"

RETURNS(r: Int32.t)
ENSURES(pure (v r == (if a then 1 else 0) + (if b then 1 else 0)))
int foo(bool a, bool b){
	return a + b;
}
