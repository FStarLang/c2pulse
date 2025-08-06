#include "../include/PulseMacros.h"

int * get () {
	int p;
	return &p;
}

int * get2 (int x) {
	return &x;
}

EXPECT_FAILURE() // cannot prove that the result points to anything
RETURNS(res : ref Int32.t)
ENSURES(exists* x. res |-> x)
int * get3 (int x) {
	return &x;
}
