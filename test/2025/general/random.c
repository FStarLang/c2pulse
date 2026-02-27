#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "../c2pulse.h"

typedef struct _s1 {
    bool bytes[10];
    char charbytes[10];
    int matrix[10][10];
    int length;
    bool arr[]; 
} s1;

_preserves((_slprop) _inline_pulse(exists* s1_s. s1_pred x1 s1_s))
void foo(s1 *x1){
    _assert((_slprop) _inline_pulse(s1_explode (!x1)));
    _assert((_slprop) _inline_pulse(pts_to_len ((!(!x1)).bytes)));
    bool *tempArr = x1->bytes;
    char *tempArr1 = x1->charbytes;
    int x;
    int matrix[10][10];
    x++; 
    x = x + 2;
    x += 1;
    x /= 2;
    x *= 2; 
    x = x % 2;
    

    //TODO: implement x %= 2;
    _assert((_slprop) _inline_pulse(admit()));
    return;
}
