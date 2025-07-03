#include "../include/PulseMacros.h"

RETURNS(r:ref int32)
ENSURES(r |-> v)
ENSURES(freeable r)
int* new_heap_ref(int v)
{
    int *r = (int*)malloc(sizeof(int));
    *r = v;
    return r;
}