#include <stdlib.h>
#include "../c2pulse.h"

_ensures((_slprop) _inline_pulse(exists* v. (res |-> v) ** freeable res))
int* foo(int x) {
	return malloc(sizeof(int));
}

int bar(int x) {
	free(foo(x));
	return 42;
}
