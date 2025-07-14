#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "../include/PulseMacros.h"

RETURNS(b:bool)
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
