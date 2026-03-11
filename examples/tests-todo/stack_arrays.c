// Tests-todo: Stack-allocated arrays (fixed and variable-length)
//
// Missing features: local array declarations `int arr[N]`, VLAs `int arr[n]`
// Prototype tests: stack_allocated_array.c
//
// Stack-allocated arrays are fundamental C. Proposed syntax follows the
// existing _array attribute pattern for heap arrays.

#include "c2pulse.h"

void test_fixed_array() {
    int arr[10];
    arr[0] = 42;
}

void test_vla(int len)
    _requires(len > 0)
{
    int arr[len];
    arr[0] = 42;
}
