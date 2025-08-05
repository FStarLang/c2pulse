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



EXPECT_FAILURE()
ERASED_ARG(#s : stru_spec)
REQUIRES(stru_pred foo s)
ERASED_ARG(s:_)
REQUIRES(pure (Case_ab_a? s))
REQUIRES(ab_pred foo.payload s)
REQUIRES(pure (UInt32.fits (UInt32.v (Case_ab_a?._0 s) + 1)))
ENSURES(exists* s'. ab_pred foo.payload s')
ENSURES("exists* (s':stru_spec). stru_pred foo s' ** pure (s' == {tag = s.tag; payload = s.payload})")
void test_union(struct stru * foo){

  if (foo->tag == 0){
    foo->payload.a += 1; 
  }
  else if (foo->tag == 1){
    foo->payload.b = !(foo->payload.b);
  }
}






