#include "../include/PulseMacros.h"

struct _foo;
typedef struct _foo foo;
struct _foo {
	foo *next;
	int v;
};

REQUIRES(exists* v vn. foo_pred p v ** foo_pred v.next vn)
ENSURES (exists* v vn. foo_pred p v ** foo_pred v.next vn)
void set_next_zero(foo *p)
{
	LEMMA(foo_explode (!p));
	foo *pn = p->next;
	LEMMA(foo_explode (!pn));
	pn->v = 0;
	LEMMA(foo_recover (!pn));
	LEMMA(foo_recover (!p));
}

EXPECT_FAILURE() // ambiguous...
REQUIRES(exists* v vn vnn. foo_pred p v ** foo_pred v.next vn ** foo_pred vn.next vnn)
ENSURES (exists* v vn vnn. foo_pred p v ** foo_pred v.next vn ** foo_pred vn.next vnn)
void set_next_next_zero(foo *p)
{
	LEMMA(foo_explode (!p));
	foo *pn = p->next;
	LEMMA(foo_explode (!pn));
	foo *pnn = pn->next;
	LEMMA(foo_explode (!pnn));
	pnn->v = 0;
	LEMMA(foo_recover (!pnn));
	LEMMA(foo_recover (!pn));
	LEMMA(foo_recover (!p));
}


int get_next(foo *p)
{
	LEMMA(admit()); // just checking we translate to something sensible
	int res = p->next->v;
	return res;
}
