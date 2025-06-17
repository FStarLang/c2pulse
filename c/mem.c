int box(int x)
{
  int *r = malloc(sizeof(int));
  *r = *r + *r;
  int v = *r;
  free(r);
  return v;
}

int local(int x)
{
  int r = x;
  r = r + r;
  int v = r;
  return v;
}

int heap_arr(int x)
{
  int *a = malloc(sizeof(int) * 10);
  a[0] = x;
  a[0] = a[0] + a[0];
  int v = a[0];
  free(a);
  return v;
}

int local_arr(int x)
{
  int a[10];
  a[0] = x;
  a[0] = a[0] + a[0];
  int v = a[0];
  return v;
}
