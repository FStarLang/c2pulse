#include "c2pulse.h"

union foo {
    int x;
    _Bool y;
};

void set_x(union foo *f) {
    f->x = 42;
}

void set_y(union foo *f) {
    f->y = 1;
}
