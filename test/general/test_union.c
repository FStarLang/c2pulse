#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "../include/PulseMacros.h"


union ab {
         unsigned int a;
         bool b;
};

ERASED_ARG(s:_)
REQUIRES(pure (Case_ab_a? s))
REQUIRES(ab_pred x s)
REQUIRES(pure (UInt32.fits (UInt32.v (Case_ab_a?._0 s) + 1)))
ENSURES(exists* s'. ab_pred x s')
void incr_a(union ab *x) {
  LEMMA(ab_explode !x);
  LEMMA(rewrite each s as Case_ab_a (Case_ab_a?._0 s));
  x->a = x->a + 1;
  LEMMA(ab_recover !x #(Case_ab_a _));
}

struct stru {
        int8_t tag;
        union ab payload;
};

INCLUDE(
"let stru_ok (u : ref stru) (s : stru_spec) : slprop =\
stru_pred u s **\
pure (\
match s.tag with\
| 0y -> Case_ab_a? s.payload\
| 1y -> Case_ab_b? s.payload\
| _ -> false\
)"
)                                          

