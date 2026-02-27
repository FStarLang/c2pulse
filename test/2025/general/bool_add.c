#include <stdbool.h>
#include "../c2pulse.h"  

_ensures((_slprop) _inline_pulse(pure (v r == (if a then 1 else 0) + (if b then 1 else 0))))
int foo(bool a, bool b){
	return a + b;
}

