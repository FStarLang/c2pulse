#include "../include/PulseMacros.h"

INCLUDE(assume val res : slprop)

REQUIRES(res)
void bar(void);

REQUIRES(res)
void foo() {
	bar();
}

void bar()
{
	foo();
}
