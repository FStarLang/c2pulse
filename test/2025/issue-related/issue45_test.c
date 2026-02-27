#include "../c2pulse.h"

_include_pulse(assume val res : slprop)

void bar(void);

void foo() {
	bar();
}

void bar()
{
	foo();
}
