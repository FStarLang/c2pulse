// Tests-todo: NULL pointer expressions
//
// Missing features: NULL as an rvalue, NULL comparisons (==, !=)
// Prototype tests: issue31_test.c, issue32_test.c, issue43_test.c
//
// NULL is a fundamental C concept. c2pulse should support:
// - Using NULL as an rvalue in assignments and arguments
// - Comparing pointers against NULL with == and !=
// The existing `issue50.c` test uses `&p != NULL` and `&p != 0` which
// works because &p is never null; full NULL support needs the general case.

#include "c2pulse.h"

int check_null(int *r) {
    if (r == NULL) {
        return 0;
    } else {
        return 1;
    }
}

void pass_null(int *p) {
}

void call_with_null() {
    pass_null(NULL);
}

void assign_null() {
    int *p = NULL;
}
