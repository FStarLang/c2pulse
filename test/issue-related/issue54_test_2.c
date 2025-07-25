#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "../include/PulseMacros.h"

RETURNS(Int32.t)
int add(){
    int x = 1; 
    int y = 0;
    return !!x + !!y;
}

REQUIRES(pure (length a1 == UInt64.v len))
REQUIRES(pure (length a2 == UInt64.v len))
REQUIRES(exists* v. (a1 |-> v))
REQUIRES(exists* v. (a2 |-> v))
RETURNS(Int32.t)
ENSURES(exists* v. (a1 |-> v))
ENSURES(exists* v. (a2 |-> v))
int compare_elt(ISARRAY(len)int *a1, ISARRAY(len)int *a2, size_t i, size_t len)
{
    if (i < len)
    {   
        LEMMA(pts_to_len !a1);
        LEMMA(pts_to_len !a2);
        return (a1[i] == a2[i]);
    }
    else
    {
        return 0l;
    }
}
