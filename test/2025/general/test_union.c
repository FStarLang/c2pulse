#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "../c2pulse.h"

// struct abcd {
//          unsigned int a;
//          bool b;
//          uint8_t c[10];
// };

union ab {
  unsigned int a;
  bool b;      
};

_include_pulse(open Pulse.Lib.WithPure)

_requires((_slprop) _inline_pulse(with_pure (Case_ab_a? s)))
_requires((_slprop) _inline_pulse(ab_pred x s))
_requires((_slprop) _inline_pulse(pure (UInt32.fits (UInt32.v (Case_ab_a?._0 s) + 1))))
_ensures((_slprop) _inline_pulse(exists* s'. ab_pred x s'))
void incr_a(union ab *x) {
  _assert((_slprop) _inline_pulse(ab_is_a (!x)));
  _assert((_slprop) _inline_pulse(ab_explode !x));
  x->a = x->a + 1;
  _assert((_slprop) _inline_pulse(ab_recover !x #(Case_ab_a _)));
}

_requires((_slprop) _inline_pulse(exists* s. ab_pred x s))
_ensures((_slprop) _inline_pulse(ab_pred x (Case_ab_a v)))
void set_case_a(union ab *x, unsigned int v) {
  _assert((_slprop) _inline_pulse(ab_change_a !x));
  _assert((_slprop) _inline_pulse(ab_explode !x));
  x->a = v;
  _assert((_slprop) _inline_pulse(ab_recover !x #(Case_ab_a _)));
}

struct stru {
        int8_t tag;
        union ab payload;
};

_include_pulse(
[@@pulse_unfold]
let stru_ok (u : ref stru) (s : stru_spec) : slprop =
stru_pred u s **
pure (
match s.tag with
| 0y -> Case_ab_a? s.payload
| 1y -> Case_ab_b? s.payload
| _ -> False
)
)

_preserves((_slprop) _inline_pulse(exists* s. stru_ok foo s))
void test_union(struct stru * foo){
  _assert((_slprop) _inline_pulse(stru_explode (!foo)));
  if (foo->tag == 0){
    _assert((_slprop) _inline_pulse(ab_is_a (! (! foo)).payload));
    _assert((_slprop) _inline_pulse(ab_explode (! (!foo)).payload));
    foo->payload.a = 1;
    _assert((_slprop) _inline_pulse(ab_recover (! (!foo)).payload #(Case_ab_a _)));
    _assert((_slprop) _inline_pulse(stru_recover (!foo)));
  }
  else if (foo->tag == 1) {
    _assert((_slprop) _inline_pulse(ab_is_b (! (! foo)).payload));
    _assert((_slprop) _inline_pulse(ab_explode (! (!foo)).payload));
    foo->payload.b = false;
    _assert((_slprop) _inline_pulse(ab_recover (! (!foo)).payload #(Case_ab_b _)));
    _assert((_slprop) _inline_pulse(stru_recover (!foo)));    
  }
  else {
    _assert((_slprop) _inline_pulse(unreachable()));
  }
}