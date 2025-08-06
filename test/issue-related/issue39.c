#include "../include/PulseMacros.h"

RETURNS(res : Int32.t)
ENSURES(pure (Int32.v res == 1))
int test0(int x) {
	x = 1;
	return x;
}

RETURNS(res : Int32.t)
ENSURES(pure (Int32.v res == 1))
int test1(int x) {
	*(&x) = 1;
	return x;
}

RETURNS(res : Int32.t)
ENSURES(pure (Int32.v res == 1))
int test2(int x) {
	int *p = &x;
	*p = 1;
	return *p;
}


RETURNS(res : Int32.t)
ENSURES(pure (Int32.v res == 1))
int test3(int x) {
	int *p = &x;
	*p = 1;
	return *p;
}

RETURNS(res : Int32.t)
ENSURES(pure (Int32.v res == 1))
int test4(int x) {
	int *p = &x;
	*p = 1;
	return x;
}


RETURNS(res : Int32.t)
ENSURES(pure (Int32.v res == 1))
int test5(int x) {
	int *p = &x;
	if (1)
	{
		*p = 1;
	}
	return x;
}


// Fails due to pulse
// "Allocating a mutable local variable expects an annotated post-condition"
// RETURNS(res : Int32.t)
// ENSURES(pure (Int32.v res == 1))
// int test6(int x) {
// 	if (1)
// 	{
// 		int *p = &x;
// 		*p = 1;
// 	}
// 	return x;
// }

