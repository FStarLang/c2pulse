#include <stdlib.h>

struct foo {
	int left;
};

void test(struct foo *p)
{
        p->left = 123;
}
