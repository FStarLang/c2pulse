#include "c2pulse.h"

struct my_pair {
  int a;
  int b;
};

// Test $type in a top-level _include_pulse block
_include_pulse(
  let antiquot_type_test : Type0 = $type(int *)
)

// Test $declare in a ghost_stmt
void test_declare(int *x)
  _requires(*x == 0)
  _ensures(*x == 0)
{
  _ghost_stmt(assume_ (pure ($(*x) == 0l)));
  return;
}
