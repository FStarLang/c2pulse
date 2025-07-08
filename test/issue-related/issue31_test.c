#include "../include/PulseMacros.h"
#include <stdlib.h>

REQUIRES(emp)
RETURNS(i:int32)
ENSURES(emp)
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