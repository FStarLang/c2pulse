#include "../include/PulseMacros.h"

EXPECT_FAILURE(19)
RETURNS(s:ref int32)
ENSURES(s |-> 0l)
int* refs_are_scoped()
{
    int s = 0;
    return &s;
}