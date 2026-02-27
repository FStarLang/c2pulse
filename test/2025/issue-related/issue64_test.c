#include <stdlib.h>
#include <stdint.h>
#include "../c2pulse.h"

void foo(){
   uint8_t *bytes = (uint8_t*)malloc(32*sizeof(uint8_t));
   uint8_t *bytes1 = (uint8_t*)malloc(sizeof(uint8_t)*32);
   free(bytes);
   free(bytes1);
}
