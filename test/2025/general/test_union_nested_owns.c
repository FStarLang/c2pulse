#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "../c2pulse.h"

_include_pulse(
  module U32 = Pulse.Lib.C.UInt32
  open Pulse.Lib.C.UInt32
)
// struct abcd {
//          unsigned int a;
//          bool b;
//          uint8_t c[10];
// };

union ab {
  unsigned int *a;
  bool *b;      
};

_preserves((_slprop) _inline_pulse(ab_pred x (Case_ab_a a)))
_requires((_slprop) _inline_pulse(reveal a |-> v))
_requires((_slprop) _inline_pulse(pure (U32.fits ( + ) (UInt32.v v) 1)))
_ensures((_slprop) _inline_pulse(exists* (u:uint32). (reveal a |-> u) ** pure (U32.as_int u == U32.as_int v + 1)))
void incr_a(union ab *x) {
  _assert((_slprop) _inline_pulse(ab_is_a (!x)));
  _assert((_slprop) _inline_pulse(ab_explode !x));
  *x->a = *x->a + 1;
  _assert((_slprop) _inline_pulse(ab_recover !x #(Case_ab_a _)));
}

_requires((_slprop) _inline_pulse(exists* s. ab_pred x s))
_preserves((_slprop) _inline_pulse(a |-> v))
_ensures((_slprop) _inline_pulse(ab_pred x (Case_ab_a a)))
void set_case_a(union ab *x, unsigned int *a) {
  _assert((_slprop) _inline_pulse(ab_change_a !x));
  _assert((_slprop) _inline_pulse(ab_explode !x));
  x->a = a;
  _assert((_slprop) _inline_pulse(ab_recover !x #(Case_ab_a _)));
}

struct stru {
        int8_t tag;
        union ab payload;
};

_include_pulse(
let stru_payload (a:ab_spec) : slprop = 
  match a with 
  | Case_ab_a a -> exists* v. (a |-> v)
  | Case_ab_b b -> exists* v. (b |-> v)

[@@pulse_unfold]
let stru_ok (u : ref stru) (s : stru_spec) : slprop =
    stru_pred u s **
    stru_payload s.payload **
    pure ( match s.tag with | 0y -> Case_ab_a? s.payload | 1y -> Case_ab_b? s.payload | _ -> False )

ghost
fn elim_stru_payload_a (a:ab_spec { Case_ab_a? a })
requires stru_payload a
ensures exists* v. (Case_ab_a?._0 a |-> v)
{
  rewrite each a as (Case_ab_a (Case_ab_a?._0 a));
  unfold stru_payload;
}

ghost
fn elim_stru_payload_b (a:ab_spec { Case_ab_b? a })
requires stru_payload a
ensures exists* v. (Case_ab_b?._0 a |-> v)
{
  rewrite each a as (Case_ab_b (Case_ab_b?._0 a));
  unfold stru_payload;
}

ghost
fn intro_stru_payload_a (a:ab_spec { Case_ab_a? a })
requires exists* v. (Case_ab_a?._0 a |-> v)
ensures stru_payload (Case_ab_a (Case_ab_a?._0 a))
{
  fold stru_payload (Case_ab_a (Case_ab_a?._0 a))
}

ghost
fn intro_stru_payload_b (a:ab_spec { Case_ab_b? a })
requires exists* v. (Case_ab_b?._0 a |-> v)
ensures stru_payload (Case_ab_b (Case_ab_b?._0 a))
{
  fold stru_payload (Case_ab_b (Case_ab_b?._0 a))
}

ghost
fn intro_stru_ok (u:ref stru) (#s:stru_spec) (#pl:ab_spec)
requires
    stru_pred u s **
    stru_payload pl **
    pure (s.payload == pl) **
    pure ( match s.tag with | 0y -> Case_ab_a? s.payload | 1y -> Case_ab_b? s.payload | _ -> False )
ensures stru_ok u s
{
  rewrite each pl as s.payload;
}
)

_preserves((_slprop) _inline_pulse(exists* s. stru_ok foo s))
void test_union(struct stru * foo){
  _assert((_slprop) _inline_pulse(stru_explode (!foo)));
  if (foo->tag == 0){
    _assert((_slprop) _inline_pulse(ab_is_a (! (! foo)).payload; ab_explode (! (!foo)).payload; elim_stru_payload_a _));
    *foo->payload.a = 1;
    _assert((_slprop) _inline_pulse(intro_stru_payload_a _; ab_recover (! (!foo)).payload #(Case_ab_a _);
          stru_recover (!foo); intro_stru_ok (!foo)));
  }
  else if (foo->tag == 1) {
    _assert((_slprop) _inline_pulse(ab_is_b (! (! foo)).payload; ab_explode (! (!foo)).payload; elim_stru_payload_b _));
    *foo->payload.b = false;
    _assert((_slprop) _inline_pulse(intro_stru_payload_b _; ab_recover (! (!foo)).payload #(Case_ab_b _);
          stru_recover (!foo); intro_stru_ok (!foo)));    
  }
  else {
    _assert((_slprop) _inline_pulse(unreachable()));
  }
}