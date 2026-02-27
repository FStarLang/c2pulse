#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "../c2pulse.h"

bool foo(size_t i, size_t j){

	if (i < j){
          return true;
	}
	else {
	 return false;
	}

}

bool bar(uint64_t i, uint64_t j){

	if (i < j){
          return true;
	}
	else {
	 return false;
	}

}

bool baz(uint32_t i, uint64_t j){

        if (i < j){
          return true;
        }
        else {
         return false;
        }

}

bool boo(uint64_t i, uint32_t j){

        if (i < j){
          return true;
        }
        else {
         return false;
        }

}
