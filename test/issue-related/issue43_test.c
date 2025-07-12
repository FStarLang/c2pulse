#include <stdio.h>

void foo(int *p) {
}

void bar() {
	foo(NULL);
}

void baz() {
	int *p = NULL;
	foo(p);
}
