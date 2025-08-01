#include <stdlib.h>
#include <stdint.h>
#include "../include/PulseMacros.h"

struct foo_struct {
int a; 
int b;
};

REQUIRES(pure (SizeT.v length < 100))
RETURNS(Int32.t)
int foo(size_t length){
  //ISARRAY(10)int *a = (int*) malloc(sizeof(int) * 10);
  //This should pass since the call allocates int of sizeof(int)
  int *b0 = (int*) malloc(sizeof(int));
  //int64_t *b3 = (int64_t*) malloc(sizeof(int64_t));
  //This should Fail since the call allocates int of sizeof(int)
  //int *b1 = (int*) malloc(sizeof(int) * 10);
  //This should Fail since the call allocates int of non constant size
  //Mark this with ISARRAY to tell the compiler its an array 
  //ISARRAY(length)int *b2 = (int*) malloc(sizeof(int) * (length * 100));
  ISARRAY(length) int *b = (int*) malloc(sizeof(int) * length);
  //ISARRAY(length)int *c = (struct foo_struct*) malloc(sizeof(struct foo_struct) * length);
  
  //free(a);
  free(b0);
  //free(b3);
  //free(b1); 
  //free(b2);
  free(b);
  //free(c);
  return length;
}
