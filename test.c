#include <stdio.h>

//@requires r1 |-> n1
//@requires r2 |-> n2
//@ensures r1 |-> n2
//@ensures r2 |-> n1 
void ref_swap(int* r1, int* r2)
{

    int tmp = *r1;
    *r1 = *r2;
    *r2 = tmp;
}

int main() {
    int a = 5, b = 10;
    
    printf("Before swap: a = %d, b = %d\n", a, b);
    ref_swap(&a, &b);  
    printf("After swap: a = %d, b = %d\n", a, b);
    
    return 0;
} 