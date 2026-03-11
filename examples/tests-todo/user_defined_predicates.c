// Tests-todo: Complex ghost proofs and user-defined predicates
//
// Missing features:
//   - LEMMA(...) for explicit proof steps (c2pulse has _ghost_stmt but
//     the prototype's LEMMA was more integrated with struct predicates)
//   - Automatic struct explode/recover in proofs
//   - User-defined SL predicates with fold/unfold
// Prototype tests: pulse_user_defined_predicates1.c, pulse_tutorial_loops.c,
//   pulse_tutorial_existentials.c, issue18_test.c, issue26_test.c
//
// The new c2pulse handles struct predicates more automatically than the
// prototype (no explicit explode/recover needed for simple cases).
// For complex user-defined predicates, _include_pulse + _ghost_stmt
// should suffice.
//
// Example: user-defined diagonal predicate (from prototype)

#include "c2pulse.h"

_include_pulse(
    let pts_to_diag
        (#a: Type0)
        (r0 r1: ref a)
        (v: a)
    : slprop
    = (pts_to r0 #1.0R v) ** (pts_to r1 #1.0R v)
)

// Proposed idiomatic syntax for working with custom predicates:
// void double_int(int *r0, int *r1)
//     _requires((_slprop) _inline_pulse(pts_to_diag $&(r0) $&(r1) $(*r0)))
//     _requires((_specint) *r0 * 2 <= INT32_MAX)
//     _ensures((_slprop) _inline_pulse(
//         exists* w. pts_to_diag $&(r0) $&(r1) w ** pure (Int32.v w == Int32.v $(_old(*r0)) * 2)
//     ))
// {
//     _ghost_stmt(unfold (pts_to_diag $&(r0) $&(r1) $(*r0)));
//     *r0 = *r0 + *r1;
//     *r1 = *r0;
//     _ghost_stmt(fold (pts_to_diag $&(r0) $&(r1) $(*r0)));
// }

// Example: point with custom is_point predicate
typedef struct {
    int px;
    int py;
} point;

_include_pulse(
    $declare(point p)
    let is_point (p: ref $type(point)) (xy: (int & int)) : slprop =
        exists* v. point__pred p v ** pure (Int32.v $(v.px) == fst xy) ** pure (Int32.v $(v.py) == snd xy)
)

// void move(point *p, int dx, int dy)
//     _requires((_slprop) _inline_pulse(is_point $&(p) ...))
// {
//     _ghost_stmt(unfold is_point; point__explode $&(p));
//     p->px = p->px + dx;
//     p->py = p->py + dy;
//     _ghost_stmt(point__recover $&(p); fold is_point);
// }
