// Tests-todo: Forward-declared / self-referential structs
//
// Missing features: forward declarations of structs, self-referential
//   struct types via forward decl + typedef
// Prototype tests: issue38_test_4.c, issue38_test_5.c, issue38_test_6.c,
//   nested_struct.c (linked list), issue42.c
//
// C allows forward-declaring struct types and using pointers to them
// before the full definition. This is essential for linked data structures.

#include "c2pulse.h"

// Forward declaration pattern
struct foo;
typedef struct foo *foo_ptr;
struct foo {
    int left;
    foo_ptr next;
};

// Direct self-reference pattern
struct bar {
    int v;
    struct bar *next;
};

// Linked list traversal
void set_next_zero(struct foo *p) {
    struct foo *pn = p->next;
    pn->v = 0;
}
