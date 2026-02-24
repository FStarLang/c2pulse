#include "c2pulse.h"

void cleanup_on_error(int *p)
{
    *p = 0;
    if (*p != 0)
        goto fail;
    *p = 42;
fail:;
}
