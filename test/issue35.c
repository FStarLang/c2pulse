#include "c2pulse.h"

void empty_branch()
{
  int x = 1;
  if (x == 1) {
    return;
  } else {
    return;
  }
}

int non_final_if_else() {
  if (1) {
    return 1;
  } else {
    return 2;
  }

  return 3;
}

int non_final_if () {
  if (1) {
    return 1;
  }

  return 3;
}
