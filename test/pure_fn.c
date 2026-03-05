#include "c2pulse.h"
#include <stdbool.h>

_pure bool pure_not(bool x) {
    if (x) {
        return false;
    } else {
        return true;
    }
}

_pure bool pure_and(bool x, bool y) {
    if (x) {
        return y;
    } else {
        return false;
    }
}

_pure bool pure_or(bool x, bool y) {
    if (x) {
        return true;
    } else {
        return y;
    }
}

_pure bool pure_let(bool x) {
    bool y;
    y = x;
    return y;
}
