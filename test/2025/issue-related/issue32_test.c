#include "../c2pulse.h"
#include <stdlib.h>

int check_null(int *r) {
  if (r != NULL) {
    return 1;
  } else {
    return 0;
  }
}