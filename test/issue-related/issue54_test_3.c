#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "../include/PulseMacros.h"

RETURNS(Int32.t)
int test_ampamp(){
    int a; 
    int b;
    return a && b;
}

RETURNS(Int32.t)
int test_barbar(){
    int a; 
    int b;
    return a || b;

}

REQUIRES(exists* v. (a1 |-> v))
REQUIRES(exists* v. (a2 |-> v))
RETURNS(bool)
ENSURES(exists* v. (a1 |-> v))
ENSURES(exists* v. (a2 |-> v))
bool compare_elt(ISARRAY(len)int *a1, ISARRAY(len)int *a2, size_t len)
{   
    size_t i = 0;
    if (i < len)
    {   
        LEMMA(pts_to_len !a1);
        LEMMA(pts_to_len !a2);
        return ((i < len) && a1[i] == a2[i]);
    }
    else
    {
        return false;
    }
}
