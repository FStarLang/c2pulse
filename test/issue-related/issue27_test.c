#include "../include/PulseMacros.h"

RETURNS(s:int32)
int refs_are_scoped()
{
    int s = 0;
    return &s;
}