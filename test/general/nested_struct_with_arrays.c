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
[@@pulse_unfold] 
let owns_s2 (x:ref s2) (y:Seq.seq UInt8.t) =
exists* s2_spec s1_spec.
  s2_pred x s2_spec **
  freeable x **
  s1_pred s2_spec.field_s1 s1_spec **
  freeable s2_spec.field_s1 ** 
  (s1_spec.bytes |-> y) **
  freeable_array s1_spec.bytes

ghost
fn intro_owns_s2 (x:ref s2) s2_spec s1_spec fs1 bytes (y:_)
requires
    s2_pred x s2_spec **
    s1_pred fs1 s1_spec **
    (bytes |-> y) **
    freeable x **
    freeable fs1 ** 
    freeable_array bytes **
    pure (s2_spec.field_s1 == fs1) **
    pure (s1_spec.bytes == bytes)
ensures owns_s2 x y
{
  rewrite each fs1 as s2_spec.field_s1;
  rewrite each bytes as s1_spec.bytes;
}
)

void test()
{
    ISARRAY(32) uint8_t *bytes = (uint8_t*)malloc(sizeof(uint8_t)*32);
    s1 x1;
    LEMMA(s1_pack x1);
    LEMMA(s1_explode x1);
    x1.bytes = bytes;
    LEMMA(s1_recover x1);

    LEMMA(s1_explode x1);
    x1.bytes = bytes;
    LEMMA(s1_recover x1);

    LEMMA(admit());
}


RETURNS(x: ref s2)
ENSURES(exists* y. owns_s2 x y)
s2* mk_s2()
{
    ISARRAY(32) uint8_t *bytes = (uint8_t*)malloc(sizeof(uint8_t)*32);
    s1 *x1 = (s1*)malloc(sizeof(s1));
    LEMMA(s1_explode (!x1));
    x1->bytes = bytes;
    LEMMA(s1_recover (!x1));
    s2 *x2 = (s2*)malloc(sizeof(s2));
    LEMMA(s2_explode (!x2));
    x2->field_s1 = x1;
    LEMMA(s2_recover (!x2));
    LEMMA(intro_owns_s2 (!x2) _ _ _ _ _);
    return x2;
}

PRESERVES(exists* y. owns_s2 x2 y)
uint8_t read_byte0(s2 *x2)
{
    LEMMA(s2_explode(!x2));
    s1 *x1 = x2->field_s1;
    LEMMA(s1_explode(!x1));
    LEMMA(pts_to_len (!(!(!x1)).bytes));
    // uint8_t res = x1->bytes[0];
    // uint8_t *v = x1->bytes;
    LEMMA(admit());
    // uint8_t res = v[0];
    // LEMMA(s1_recover(!x1));
    // LEMMA(s2_recover(!x2));
    return 0;
}
