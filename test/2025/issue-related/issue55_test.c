#include "../c2pulse.h"
#include <stddef.h>
#include <stdbool.h>

bool compare(int *a1, int *a2, size_t l)
{
    size_t i = 0;
    while( (i < l) ? a1[i] == a2[i] : false )
        _invariant((_slprop) _inline_pulse(emp))
    {
        i = i + 1;
    }
    return (i == l);
}
