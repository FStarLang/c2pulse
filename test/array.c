#include "c2pulse.h"

void foo(unsigned a[])
  _requires(a._length == 2)
  _ensures(a._length == 2 && a[0] == 42)
{
  a[0] = 42;
  a[1] = a[0] + 67;
}

void ptr_attr(_array unsigned *a)
  _requires(a._length == 2)
  _ensures(a._length == 2 && a[0] == 42)
{
  a[0] = 42;
  a[1] = a[0] + 67;
}

typedef unsigned *uptr _array;
void tydef_array(uptr a)
  _requires(a._length == 2)
  _ensures(a._length == 2 && a[0] == 42)
{
  a[0] = 42;
  a[1] = a[0] + 67;
}

typedef struct {
  _array unsigned *x;
} uptr_struct;
void struct_arr(uptr_struct a)
  _requires(a.x._length == 2)
  _ensures(a.x._length == 2 && a.x[0] == 42)
{
  a.x[0] = 42;
  a.x[1] = a.x[0] + 67;
}