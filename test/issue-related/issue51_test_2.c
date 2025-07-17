#include "../include/PulseMacros.h"
#include <stdint.h>

RETURNS(Int32.t)
EXPECT_FAILURE()
int neg(uint32_t x) { 
    return -x;
}

RETURNS(Int64.t)
EXPECT_FAILURE()
long neg_2(uint64_t x) { 
    return -x;
}

//Vidush : Will fail since minus is not there for Int32.
// RETURNS(Int32.t)
// int neg_3(int x) { 
//     return -x;
// }

RETURNS(Int32.t)
int not(int x) { 
    return !x;
}
