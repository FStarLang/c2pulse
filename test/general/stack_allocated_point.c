#include <stdint.h>
#include <stdlib.h>
#include "../include/PulseMacros.h"

typedef struct _point {
  int px;
  int py;
} point;

void test_stack_allocator()
{
  point p;  
  p.px = 1;
  p.px = 2;
}