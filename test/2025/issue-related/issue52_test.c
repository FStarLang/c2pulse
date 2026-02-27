#include "../include/PulseMacros.h"
#include <stdlib.h>	
	
REQUIRES(exists* v. (x |-> v))
RETURNS(Int32.t)
ENSURES(exists* v. (x |-> v))
int fake_arr(ISARRAY(5) int* x) {
	LEMMA(pts_to_len !x);
	return x[0];
}
