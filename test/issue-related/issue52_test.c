#include "../include/PulseMacros.h"
	
	
REQUIRES(exists* v. (x |-> v))
REQUIRES(pure (length x == 5))
RETURNS(Int32.t)
ENSURES(exists* v. (x |-> v))
int fake_arr(ISARRAY(5) int* x) {
	LEMMA(pts_to_len !x);
	return x[0];
}
