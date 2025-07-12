#include <stdlib.h>


struct list {
    struct list *next;
};

int foo(struct list *list){
  list->next->next = list->next;
}





