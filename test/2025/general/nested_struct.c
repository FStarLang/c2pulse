#include <stdlib.h>
#include "../c2pulse.h"

struct list {
    struct list *next;
};

//Vidush there is a bug here
// if i write list *list then this will fail
int foo(struct list *lst){
  _assert((_slprop) _inline_pulse(admit()));
  lst->next->next = lst->next;
}

