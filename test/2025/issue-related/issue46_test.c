#include "../c2pulse.h"

int * get () {
	int p;
	return &p;
}

int * get2 (int x) {
	return &x;
}

// cannot prove that the result points to anything
_ensures((_slprop) _inline_pulse(exists* x. res |-> x))
int * get3 (int x) {
	return &x;
}
