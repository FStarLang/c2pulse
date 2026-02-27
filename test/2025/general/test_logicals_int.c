#include <stdint.h>
#include <stdio.h>
#include "../c2pulse.h"

int f1(){
   
  int x = 11; 
  int y = 10;  
  int z = x && y; 
  return z;
}

int f2(){

  int x = 10; 
  int y = 11; 
  int z = x || y; 
  return z;
}

int f3(){

 int x = 10; 
 int y = 20; 
 return !!x + !!y;
}

int f4(){
 int x = -10;
 int y = 10; 
 int z = !(x && y);
 return z;
}

int f5(){
 int x = -100; 
 int y = 100; 
 int z = x && y;
 return z;
}

int f6(){
 uint32_t x = 100; 
 int y = -x; 
 int z = x && y;
 return z;
}

/*int main(){
  int a = f1(); 
  int b = f2(); 
  int c = f3();
  int d = f4();
  int e = f5(); 
  printf("a: %d, b: %d, c: %d, d: %d, e: %d\n", a, b, c, d, e);
  return 0;
}*/

