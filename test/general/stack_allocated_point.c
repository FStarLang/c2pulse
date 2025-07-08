#include <stdint.h>
#include <stdlib.h>
#include "../include/PulseMacros.h"

typedef struct _point {
  int px;
  int py;
} point;


REQUIRES(emp)
ENSURES(emp)
void test_stack_allocator()
{
  point p;
  ///TODO: Vidush: remove point pack and unpack calls since these 
  ///should be added by the c2pulse frontend.
  LEMMA(point_pack p);
  LEMMA(point_explode p);
  p.px = 1;
  p.py = 2;
  LEMMA(point_recover p);
  LEMMA(point_unpack p);
}