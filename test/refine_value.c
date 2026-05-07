// Test: _refine_value — custom existential bindings on pointer types.
//
// Covers:
//   1. Defining a custom predicate binding on a typedef via _refine_value
//   2. The binding appears as a parameter in the generated pred
//   3. The predicate body references both this and the custom binding
#include "pal.h"
#include <stdint.h>
#include <stddef.h>

typedef struct node {
    int data;
    struct node *next;
} node;

_type(spec_list, list Int32.t)

_include_pulse(
  let rec is_list ([@@@mkey] head: ref (struct_node)) (elements: ty_spec_list)
    : Tot slprop (decreases elements)
  = match elements with
    | [] -> pure (is_null head)
    | hd :: tl ->
      exists* (nd: struct_node).
        pts_to head nd **
        freeable head **
        pure (nd.struct_node__data == hd) **
        is_list nd.struct_node__next tl
)

_refine_value(spec_list elements, (_slprop) _inline_pulse(is_list $(this) elements))
_plain typedef struct node * list;
