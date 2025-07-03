// EXPECT_FAILURE(19)
#include "../../include/PulseMacros.h"
RETURNS(s:_)
ENSURES(s |-> 0)
int* refs_are_scoped()
{
    int s = 0;
    return &s;
}
