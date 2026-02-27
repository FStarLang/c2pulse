#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "../include/PulseMacros.h"

REQUIRES(pure (length a1 == SizeT.v len))
REQUIRES(pure (length a2 == SizeT.v len))
REQUIRES(exists* v. (a1 |-> v))
REQUIRES(exists* v. (a2 |-> v))
RETURNS(b:bool)
ENSURES(exists* v. (a1 |-> v))
ENSURES(exists* v. (a2 |-> v))
bool compare_elt(ISARRAY(len)int *a1, ISARRAY(len)int *a2, size_t i, size_t len)
{
    if (i < len)
    {   
        LEMMA(pts_to_len !a1);
        LEMMA(pts_to_len !a2);
        return (a1[i] == a2[i]);
    }
    else
    {
        return false;
    }
}
