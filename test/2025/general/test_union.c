#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "../include/PulseMacros.h"


// struct abcd {
//          unsigned int a;
//          bool b;
//          uint8_t c[10];
// };

union ab {
  unsigned int a;
  bool b;      
};

INCLUDE(open Pulse.Lib.WithPure)

ERASED_ARG(s:_)
REQUIRES(with_pure (Case_ab_a? s))
REQUIRES(ab_pred x s)
REQUIRES(pure (UInt32.fits (UInt32.v (Case_ab_a?._0 s) + 1)))
ENSURES(exists* s'. ab_pred x s')
void incr_a(union ab *x) {
  LEMMA(ab_is_a (!x));
  LEMMA(ab_explode !x);
  x->a = x->a + 1;
  LEMMA(ab_recover !x #(Case_ab_a _));
}

REQUIRES(exists* s. ab_pred x s)
ENSURES(ab_pred x (Case_ab_a v))
void set_case_a(union ab *x, unsigned int v) {
  LEMMA(ab_change_a !x);
  LEMMA(ab_explode !x);
  x->a = v;
  LEMMA(ab_recover !x #(Case_ab_a _));
}

struct stru {
        int8_t tag;
        union ab payload;
};

INCLUDE(
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


PRESERVES(exists* s. stru_ok foo s)
void test_union(struct stru * foo){
  LEMMA(stru_explode (!foo));
  if (foo->tag == 0){
    LEMMA(ab_is_a (! (! foo)).payload);
    LEMMA(ab_explode (! (!foo)).payload);
    foo->payload.a = 1;
    LEMMA(ab_recover (! (!foo)).payload #(Case_ab_a _));
    LEMMA(stru_recover (!foo));
  }
  else if (foo->tag == 1) {
    LEMMA(ab_is_b (! (! foo)).payload);
    LEMMA(ab_explode (! (!foo)).payload);
    foo->payload.b = false;
    LEMMA(ab_recover (! (!foo)).payload #(Case_ab_b _));
    LEMMA(stru_recover (!foo));    
  }
  else {
    LEMMA(unreachable());
  }
}