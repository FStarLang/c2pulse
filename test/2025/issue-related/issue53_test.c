#include <stdlib.h>
#include "../include/PulseMacros.h"


RETURNS(res : ref Int32.t)
ENSURES(exists* v. (res |-> v) ** freeable res)
int* foo(int x) {
	return malloc(sizeof(int));
}

RETURNS(Int32.t)
int bar(int x) {
	free(foo(x));
	return 42;
}
