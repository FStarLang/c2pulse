#include "../c2pulse.h"

int refs_are_scoped()
{
    int s = 0;
    return &s;
}