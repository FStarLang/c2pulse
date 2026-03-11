#include "c2pulse.h"
#include <stdbool.h>
#include <stddef.h>

bool compare_sizet(size_t i, size_t j) {
    if (i < j) {
        return true;
    } else {
        return false;
    }
}
