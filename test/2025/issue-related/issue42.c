#include "../include/PulseMacros.h"

struct _foo;
typedef struct _foo foo;
struct _foo {
	foo *next;
	int v;
};

REQUIRES(exists* v vn. foo_pred p v ** foo_pred v.next vn)
ENSURES (exists* v vn. foo_pred v.next vn ** foo_pred p v ** pure (vn.v == 0l))
void set_next_zero(foo *p)
{
	foo *pn = p->next;
	pn->v = 0;
	LEMMA(foo_recover (!pn));
}

REQUIRES(exists* v vn vnn. foo_pred p v ** foo_pred v.next vn ** foo_pred vn.next vnn)
ENSURES (exists* v vn vnn. foo_pred vn.next vnn ** foo_pred v.next vn ** foo_pred p v ** pure (vnn.v == 0l))
void set_next_next_zero(foo *p)
{
	foo *pn = p->next;
	foo *pnn = pn->next;
	pnn->v = 0;
	LEMMA(foo_recover (!pnn));
}


int get_next(foo *p)
{
	LEMMA(admit()); // just checking we translate to something sensible
	int res = p->next->v;
	return res;
}
