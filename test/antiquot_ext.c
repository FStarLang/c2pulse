#include "c2pulse.h"

struct my_pair {
  int a;
  int b;
};

// Test $type in a top-level _include_pulse block
_include_pulse(
  let antiquot_type_test : Type0 = $type(int *)

  $declare(struct my_pair x)
  let test_access ($(x): $type(struct my_pair)) =
    $(x.a)
)

// Test $declare in a ghost_stmt
void test_declare(struct my_pair *x)
  _requires(x->a == 0)
  _ensures(x->a == 0)
{
  _ghost_stmt(assert pure (test_access $(*x) == 0l));
  return;
}
