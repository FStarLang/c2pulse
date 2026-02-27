#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "../c2pulse.h"

bool test_bool(int a, int b)
{
    if (a < b)
    {
        return true;
    }
    else
    {
        return false;
    }
}
