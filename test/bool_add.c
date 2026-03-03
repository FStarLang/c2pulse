#include "c2pulse.h"
#include <stdbool.h>

int foo(bool a, bool b)
    _ensures(a != false || b != false || (_specint) return == 0)
    _ensures(a != true || b != false || (_specint) return == 1)
    _ensures(a != true || b != true || (_specint) return == 2)
    _ensures(return == (_specint) a + b)
{
    return a + b;
}