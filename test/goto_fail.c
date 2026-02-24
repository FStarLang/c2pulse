#include "c2pulse.h"

void cleanup_on_error(int *p)
{
    *p = 0;
    if (*p != 0)
        goto fail;
    *p = 1;
    if (*p != 1)
        goto fail;
    *p = 42;
    return;
fail:
    *p = 99;
}
