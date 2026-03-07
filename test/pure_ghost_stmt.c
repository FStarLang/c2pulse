#include "c2pulse.h"
#include <stdbool.h>
#include <stdint.h>

/* This test is not in pure_fn.c because it also needs shift operations.
   Thus, it is not purely a unit test */

_include_pulse(
  let lemma_shift_right_8 () : Lemma
    (forall (x: FStar.UInt.uint_t 64) . (FStar.UInt.shift_right #64 x 8 == x / 256))
  = let phi
      (x: FStar.UInt.uint_t 64)
    : Lemma (FStar.UInt.shift_right #64 x 8 == x / 256)
    = assert_norm (pow2 8 == 256)
    in
    Classical.forall_intro phi
)

uint64_t stt_shift_right_8(uint64_t x)
_ensures(return == x / 256)
{
  _ghost_stmt(lemma_shift_right_8());
  return (x >> 8);
}

_pure uint64_t pure_shift_right_8(uint64_t x)
_ensures(return == x / 256)
{
  _ghost_stmt(lemma_shift_right_8());
  return (x >> 8);
}
