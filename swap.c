//@requires ptr r1
//@requires ptr r2
//@ensures  ptr r1
//@ensures  ptr r2
void ref_swap(int* r1, int* r2) 
{
  int tmp = *r1;
  *r1 = *r2;
  *r2 = tmp;
}
