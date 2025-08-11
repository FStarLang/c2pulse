#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "../include/PulseMacros.h"


typedef struct _s1 {
    bool bytes[10];
    char charbytes[10];
    int matrix[10][10];
    int length;
    bool arr[]; 
} s1;

//PRESERVES(exists* s1_s. s1_pred x1 s1_s)
void foo(s1 *x1){
    LEMMA(s1_explode (!x1));
    LEMMA(pts_to_len ((!(!x1)).bytes));
    ISARRAY() bool *tempArr = x1->bytes;
    ISARRAY() char *tempArr1 = x1->charbytes;
    int x;
    int matrix[10][10];
    x++; 
    x = x + 2;
    x += 1;
    x /= 2;
    x *= 2; 
    x = x % 2;
    

    //TODO: implement x %= 2;
    LEMMA(admit());
    return;
}
