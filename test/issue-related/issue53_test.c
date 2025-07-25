#include <stdlib.h>
#include "../include/PulseMacros.h"


RETURNS(res : ref Int32.t)
ENSURES(exists* v. res |-> v)
int* foo(int x) {
	LEMMA(admit());
	return &x; // bogus
}

EXPECT_FAILURE()
RETURNS(Int32.t)
int bar(int x) {
	free(foo(x));
	return 42;
}
