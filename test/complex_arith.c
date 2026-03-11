#include "c2pulse.h"
#include <stdint.h>

int triple(int x)
    _requires((_specint) x * 3 <= INT32_MAX && (_specint) x * 3 >= INT32_MIN)
    _ensures(return == x * 3)
{
    return x * 3;
}

int square(int x)
    _requires((_specint) x * x <= INT32_MAX && (_specint) x * x >= INT32_MIN)
    _ensures(return == x * x)
{
    return x * x;
}

int double_value(int x)
    _requires((_specint) x * 2 <= INT32_MAX && (_specint) x * 2 >= INT32_MIN)
    _ensures(return == x * 2)
{
    return x * 2;
}

int sum(int x, int y)
    _requires((_specint) x + y <= INT32_MAX && (_specint) x + y >= INT32_MIN)
    _ensures(return == x + y)
{
    return x + y;
}
