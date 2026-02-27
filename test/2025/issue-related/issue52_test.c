#include "../c2pulse.h"
#include <stdlib.h>	
	
_requires((_slprop) _inline_pulse(exists* v. (x |-> v)))
_ensures((_slprop) _inline_pulse(exists* v. (x |-> v)))
int fake_arr(int* x) {
	_assert((_slprop) _inline_pulse(pts_to_len !x));
	return x[0];
}
