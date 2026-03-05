#include "c2pulse.h"
#include <stddef.h>

struct point {
  int x;
  int y;
};

void set_x(_array struct point *pts, size_t i, int val)
  _requires(i < pts._length)
  _preserves_value(pts._length)
{
  pts[i].x = val;
}

void set_y(_array struct point *pts, size_t i, int val)
  _requires(i < pts._length)
  _preserves_value(pts._length)
{
  pts[i].y = val;
}

void set_both(_array struct point *pts, size_t i, int vx, int vy)
  _requires(i < pts._length)
  _preserves_value(pts._length)
{
  pts[i].x = vx;
  pts[i].y = vy;
}
