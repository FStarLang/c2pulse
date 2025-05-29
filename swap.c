//@requires ptr r1
//@requires ptr r2
//@ensures ptr r1
//@ensures ptr r2
void ref_swap(int* r1, int* r2) 
{
  //bb
  int tmp = *r1;
  //cc
  *r1 = *r2;
  //aaaaa
  *r2 = tmp;
}