#include "c2pulse.h"

// Forward declaration only — opaque type
struct opaque;

// Forward declaration with later definition
struct point;

struct point {
    int x;
    int y;
};

void use_point(struct point *p) {
    p->x = 1;
    p->y = 2;
}
