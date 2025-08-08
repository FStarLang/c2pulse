#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "../include/PulseMacros.h"


typedef struct _s1 {
    bool bytes[10];
    int length; 
    bool arr[]; 
} s1;

PRESERVES(exists* s1_s. s1_pred x1 s1_s)
void foo(s1 *x1){
    LEMMA(s1_explode (!x1));
    LEMMA(pts_to_len ((!(!x1)).bytes));
    ISARRAY() bool *tempArr = x1->bytes;
    LEMMA(admit());
    return;
}
