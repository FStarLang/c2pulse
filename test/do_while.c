#include "c2pulse.h"
#include <stdint.h>

/* do-while with user-named flag variable for first-iteration invariant */
uint32_t simple_do(uint32_t n)
    _requires(n >= 1 && n <= 100)
{
    uint32_t i = 0;
    do
        _do_while_first(first)
        _invariant(_live(i) && _live(n) && _live(first))
        _invariant(first ==> (_specint) i < n)
        _invariant((_specint) i <= n)
    {
        i = i + 1;
    } while (i < n);
    return i;
}

/* do-while(0): execute body exactly once (no flag needed) */
uint32_t run_once(uint32_t x)
    _requires(x <= 100)
{
    uint32_t r = 0;
    do
        _invariant(_live(r))
    {
        r = x;
    } while (0);
    return r;
}
