#include <stdlib.h>
#include <stdint.h>
#include "../c2pulse.h"

_requires((_slprop) _inline_pulse(pure FStar.Int32.(fits (v a + 1))))
int bar(int a);

int test(){
	int a = 1;
	a++;
	++a;
	a--;
	--a;
	a += 1;
	a -= 1;
	a *= 2; 
	a /= 1;
	return a;
}

_requires((_slprop) _inline_pulse(pure FStar.SizeT.(fits (v a + 1))))
size_t test2(size_t a){
	a++; 
	a--; 
	++a; 
	--a;
	return a;
}

_requires((_slprop) _inline_pulse(pure FStar.Int64.(fits (v a + 1))))
long long test3(long long a){
	a++; 
	a--; 
	++a; 
	--a;
	return a;
}

_requires((_slprop) _inline_pulse(pure FStar.Int64.(fits (v a + 1))))
long test4(long a){
	a++; 
	a--; 
	++a; 
	--a;
	return a;
}

_requires((_slprop) _inline_pulse(pure FStar.UInt64.(fits (v a + 1))))
ulong test5(ulong a){
	a++; 
	a--; 
	++a; 
	--a;
	return a;
}

_requires((_slprop) _inline_pulse(pure FStar.UInt16.(fits (v a + 1))))
uint16_t test6(uint16_t a){
	a++; 
	a--; 
	++a; 
	--a;
	return a;
}

_requires((_slprop) _inline_pulse(pure FStar.UInt8.(fits (v a + 1))))
uint8_t test7(uint8_t a){
	a++; 
	a--; 
	++a; 
	--a;
	return a;
}

_requires((_slprop) _inline_pulse(pure FStar.UInt32.(fits (v a + 1))))
uint32_t test8(uint32_t a){
	a++; 
	a--; 
	++a; 
	--a;
	return a;
}

int bar(int a){
	a++;
	return a;
}
