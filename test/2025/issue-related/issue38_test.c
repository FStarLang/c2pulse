#include <stdlib.h>

struct foo {
	int left;
};

/* This is a funny corner case, that works.
   The internal struct declaration does not
   defined any field for foo2, but defines a new
   struct type. */
struct foo2 {
	int left;
	struct xy {
		int x;
		int y;
	};
};

struct xy bar(void);
