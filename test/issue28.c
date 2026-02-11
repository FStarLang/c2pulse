#include "c2pulse.h"

typedef struct _point {
  int px;
  int py;
} point;

void use_point(point *p) {
  p->px = 17;
}

void test(void) {
  point p = {.px=0, .py=0};
  use_point(&p);
}