#include "c2pulse.h"
#include <stdlib.h>
#include <stdint.h>

void test_malloc_free(void) {
    int *p = (int *) malloc(sizeof(int));
    *p = 42;
    free(p);
}

typedef struct {
    int x;
    int y;
} point;

_allocated typedef point *point_ptr;

point_ptr mk_point()
    _ensures((_specint) return->x + return->y == 13)
{
    point *p = malloc(sizeof(point));
    *p = (point) { .x = 6, .y = 7 };
    return p;
}

// TODO: > INT32_MIN
#define INT32_FITS(x) ((x) >= 0 && (x) < INT32_MAX)

int sum_and_free_point(_consumes point_ptr p)
    _requires(INT32_FITS((_specint) p->x + p->y))
    _ensures(return == _old(p->x + p->y))
{
    int sum = p->x + p->y;
    free(p);
    return sum;
}