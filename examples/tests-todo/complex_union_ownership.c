// Tests-todo: Complex union ownership and nested struct/union with pointers
//
// GABRIEL: the only thing missing here is managing the implicit ownership
// predicate of the fields.  Right now if you have a union field `unsigned int
// *a` then you can read/write it, but you don't get liveness of the pointer.
// See examples/dpe for how the automatically generated code could look like.
//
// Missing features:
//   - Unions with pointer fields (ownership transfer)
//   - Nested struct/union with LEMMA-based ownership proofs
//   - Tagged union patterns (tag field + union payload)
// Prototype tests: test_union.c, test_union_nested_owns.c,
//   test_union_nested_owns_with_functional_spec.c,
//   test_union_nested_owns_with_functional_spec_maybe_freeable.c,
//   nested_struct_with_arrays.c, issue65_test.c
//
// The existing c2pulse handles simple unions well (union_test.c, union_test2.c).
// These tests exercise more complex ownership patterns.
//
// Proposed: idiomatic syntax for tagged unions

#include "c2pulse.h"
#include <stdbool.h>
#include <stdint.h>

// Simple tagged union (existing syntax works for the types)
union ab {
    unsigned int *a;
    bool *b;
};

struct tagged_ab {
    int8_t tag;
    union ab payload;
};

// Proposed: accessing through tagged union with ownership
// void incr_a(union ab *x)
//     _requires(x->a._active)
//     _ensures(x->a._active)
// {
//     *x->a = *x->a + 1;
// }

// Proposed: tag-checked dispatch
// void process_tagged(struct tagged_ab *t) {
//     if (t->tag == 0) {
//         // t->payload.a._active is implied by tag
//         *t->payload.a = 1;
//     } else if (t->tag == 1) {
//         *t->payload.b = false;
//     }
// }

// Nested struct with array ownership
typedef struct {
    _array uint8_t *bytes _refine(this._length == 32);
} s1;

typedef struct {
    s1 *field_s1;
} s2;

// Proposed: nested ownership spec
// s2 *mk_s2()
//     _ensures(return->field_s1->bytes._length == 32)
//     _allocated
// {
//     uint8_t *bytes = (uint8_t *) malloc(sizeof(uint8_t) * 32);
//     s1 *x1 = (s1 *) malloc(sizeof(s1));
//     x1->bytes = bytes;
//     s2 *x2 = (s2 *) malloc(sizeof(s2));
//     x2->field_s1 = x1;
//     return x2;
// }
