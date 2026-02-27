#include "../c2pulse.h"
#include <stdint.h>

int neg(uint32_t x) { 
    return -x;
}

long neg_2(uint64_t x) { 
    return -x;
}

//Vidush : Will fail since minus is not there for Int32.
//// int neg_3(int x) { 
//     return -x;
// }

int not(int x) { 
    return !x;
}
