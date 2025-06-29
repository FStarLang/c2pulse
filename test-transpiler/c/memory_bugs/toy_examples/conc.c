#include <pthread.h>

int shared = 0;

void* thread_func(void *arg) {
    shared++;  // Not synchronized: potential data race.
    return NULL;
}
