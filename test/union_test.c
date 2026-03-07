#include "c2pulse.h"

union foo {
    int x;
    _Bool y;
};

void set_x(union foo *f) {
    f->x = 42;
    _assert(f->x == 42);
}

void set_y(union foo *f) {
    f->x = 67;
    f->y = 1;
    _assert(f->y);
}
