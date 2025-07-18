#include <stdlib.h>
#include "../include/PulseMacros.h"


struct list {
    struct list *next;
};


//Vidush there is a bug here
// if i write list *list then this will fail
int foo(struct list *lst){
  LEMMA(admit());
  lst->next->next = lst->next;
}





