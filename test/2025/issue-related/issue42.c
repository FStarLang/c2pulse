#include "../c2pulse.h"

struct _foo;
typedef struct _foo foo;
struct _foo {
	foo *next;
	int v;
};

_requires((_slprop) _inline_pulse(exists* v vn. foo_pred p v ** foo_pred v.next vn))
_ensures((_slprop) _inline_pulse(exists* v vn. foo_pred v.next vn ** foo_pred p v ** pure (vn.v == 0l)))
void set_next_zero(foo *p)
{
	foo *pn = p->next;
	pn->v = 0;
	_assert((_slprop) _inline_pulse(foo_recover (!pn)));
}

_requires((_slprop) _inline_pulse(exists* v vn vnn. foo_pred p v ** foo_pred v.next vn ** foo_pred vn.next vnn))
_ensures((_slprop) _inline_pulse(exists* v vn vnn. foo_pred vn.next vnn ** foo_pred v.next vn ** foo_pred p v ** pure (vnn.v == 0l)))
void set_next_next_zero(foo *p)
{
	foo *pn = p->next;
	foo *pnn = pn->next;
	pnn->v = 0;
	_assert((_slprop) _inline_pulse(foo_recover (!pnn)));
}

int get_next(foo *p)
{
	_assert((_slprop) _inline_pulse(admit())); // just checking we translate to something sensible
	int res = p->next->v;
	return res;
}
