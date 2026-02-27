#include "../c2pulse.h"
#include <stdlib.h>

_ensures((_slprop) _inline_pulse(r |-> v))
_ensures((_slprop) _inline_pulse(freeable r))
int* new_heap_ref(int v)
{
    int *r = (int*)malloc(sizeof(int));
    *r = v;
    return r;
}
