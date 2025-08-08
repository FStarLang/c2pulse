#include <stdlib.h>
#include <stdint.h>
#include "../include/PulseMacros.h"

typedef struct _s1 {
    ISARRAY(32) uint8_t *bytes;
} s1;

typedef struct _s2 {
    s1 *field_s1;
} s2;

INCLUDE (
[@@pulse_unfold] let owns_s2 (x:ref s2) (y:Seq.seq UInt8.t) = 
exists* s2_spec s1_spec. 
  s2_pred x s2_spec **
  s1_pred s2_spec.field_s1 s1_spec **
  (s1_spec.bytes |-> y) **
  freeable x ** 
  freeable s2_spec.field_s1 ** freeable_array s1_spec.bytes 

ghost fn intro_owns_s2 (#x:ref s2) (#y:ref s1) (#z:array UInt8.t) (#bytes:Seq.seq UInt8.t) ()
requires 
    s2_pred x ({field_s1=y}) **
    s1_pred y ({bytes=z}) **
    (z |-> bytes) **
    freeable x **
    freeable y **
    freeable_array z
ensures owns_s2 x bytes { rewrite each z as ({bytes=z}).bytes }
)

void test()
{
    ISARRAY(32) uint8_t *bytes = (uint8_t*)malloc(sizeof(uint8_t)*32);
    s1 x1;
    LEMMA(s1_pack x1);
    x1.bytes = bytes;
    x1.bytes = bytes;
    LEMMA(admit());
}


RETURNS(x: _)
ENSURES(exists* y. owns_s2 x y)
s2* mk_s2()
{
    ISARRAY(32) uint8_t *bytes = (uint8_t*)malloc(sizeof(uint8_t)*32);
    s1 *x1 = (s1*)malloc(sizeof(s1));
    x1->bytes = bytes;
    s2 *x2 = (s2*)malloc(sizeof(s2));
    x2->field_s1 = x1;
    LEMMA(intro_owns_s2 ());
    return x2;
}

PRESERVES(exists* y. owns_s2 x2 y)
uint8_t read_byte0(s2 *x2)
{
    s1 *x1 = x2->field_s1;
    LEMMA(pts_to_len (!(!(!x1)).bytes));
    uint8_t res = x1->bytes[0];
    return res;
}
