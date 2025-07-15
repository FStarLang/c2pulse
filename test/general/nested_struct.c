#include <stdlib.h>
#include "../include/PulseMacros.h"


struct list {
    struct list *next;
};

int foo(struct list *list){
  LEMMA(admit());
  list->next->next = list->next;
}





