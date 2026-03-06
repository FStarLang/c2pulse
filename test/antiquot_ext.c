#include "c2pulse.h"

typedef struct {
  int a;
  int b;
} my_pair;

// Test $type in a top-level _include_pulse block
_include_pulse(
  let antiquot_type_test : Type0 = $type(int *)

  $declare(my_pair x)
  let test_access ($(x): $type(my_pair)) =
    $(x.a)
)

// Test $declare in a ghost_stmt
void test_declare(my_pair *x)
  _requires(x->a == 0)
  _ensures(x->a == 0)
{
  _ghost_stmt(assert pure (test_access $(*x) == 0l));
  return;
}
