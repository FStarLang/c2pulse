#include <stdint.h>
#include <stdlib.h>
#include "../c2pulse.h"

typedef struct _point {
  int px;
  int py;
} point;

void test_stack_allocator()
{
  point p;
  ///TODO: Vidush: remove point pack and unpack calls since these 
  ///should be added by the c2pulse frontend.
  _assert((_slprop) _inline_pulse(point_pack p));
  _assert((_slprop) _inline_pulse(point_explode p));
  p.px = 1;
  p.py = 2;
  _assert((_slprop) _inline_pulse(point_recover p));
  _assert((_slprop) _inline_pulse(point_unpack p));
}