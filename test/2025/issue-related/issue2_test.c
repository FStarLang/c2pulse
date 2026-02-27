#include "../c2pulse.h"

_ensures(return == _old(*r))
int value_of_explicit(int *r)
{
    return *r;
}
