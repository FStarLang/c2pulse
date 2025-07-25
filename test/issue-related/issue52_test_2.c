#include "../include/PulseMacros.h"


REQUIRES(pure (length x == 5))
REQUIRES(exists* v. (x |-> v))
RETURNS(Int32.t)
ENSURES(exists* v. (x |-> v))
int fake_arr(ISARRAY(5) int* x) {
        int idx = 0;
        LEMMA(pts_to_len !x);
        return x[idx];
}
