#include "c2pulse.h"

void test_rvalue_antiquot(int *x)
    _ensures((_slprop) _inline_pulse(pure (Int32.v $(*x) > 0)))
{
    _assert((_slprop) _inline_pulse($&(x) |-> $(x) ** $&(*x) |-> $(*x)));
    *x = 6 + 7;
}

void test_ghost_stmt(int *x)
    _requires(*x == 0)
    _ensures(*x == 0)
{
    _ghost_stmt(rewrite (pure ($(*x) = $(*x))) as (pure ($(*x) = $(*x))));
    return;
}

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

  $declare(my_pair x)
  let test_access_2 ($(x): $type(my_pair)) =
    $(x).$field(my_pair::a)
)

// Test $declare in a ghost_stmt
void test_declare(my_pair *x)
  _requires(x->a == 0)
  _ensures(x->a == 0)
{
  _ghost_stmt(assert pure (test_access $(*x) == 0l));
  _ghost_stmt(assert pure (test_access_2 $(*x) == 0l));
  return;
}

typedef union {
  int a;
  char b;
} my_union;

_include_pulse(
  $declare(my_union x)

  let my_fun =
    $field(my_union::a)?
)