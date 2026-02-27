#include "../c2pulse.h"
#include <stdlib.h>

int check_null(int *r)
{
  if (r == NULL)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}