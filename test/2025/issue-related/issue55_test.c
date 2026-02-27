#include "../include/PulseMacros.h"
#include <stddef.h>
#include <stdbool.h>

EXPECT_FAILURE()
bool compare(ISARRAY(l) int *a1, ISARRAY(l) int *a2, size_t l)
{
    size_t i = 0;
    while( (i < l) ? a1[i] == a2[i] : false )
    INVARIANTS( invariant c. emp)
    {
        i = i + 1;
    }
    return (i == l);
}
