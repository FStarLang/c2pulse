#include "../include/PulseMacros.h"

EXPECT_FAILURE(19,20,21)
RETURNS(s:_)
ENSURES(s |-> 0l)
int* refs_are_scoped()
{
    int s = 0;
    return &s;
}