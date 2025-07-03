#include <stdint.h>
#include <stdlib.h>
#include "../include/PulseMacros.h"


INCLUDE (

  let pts_to_diag 
        (#a:_)
        (r0 r1:ref a)
        (v:a)
  : slprop
  = (r0 |-> v) ** (r1 |-> v)

)

ERASED_ARG(#v:erased _ { fits ( * ) 2 (as_int v) })
REQUIRES(pts_to_diag r0 r1 v)
ENSURES(exists* w. pts_to_diag r0 r1 w ** pure (as_int w = 2 * as_int v))
void double_int(int *r0, int *r1)
{
    LEMMA(unfold (pts_to_diag r0 r1 v));
    *r0 = *r0 + *r1;
    *r1 = *r0;
    LEMMA(fold (pts_to_diag r0 r1));
}