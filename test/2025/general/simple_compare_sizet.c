#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "../include/PulseMacros.h"

RETURNS(bool)
bool foo(size_t i, size_t j){

	if (i < j){
          return true;
	}
	else {
	 return false;
	}

}

RETURNS(bool)
bool bar(uint64_t i, uint64_t j){

	if (i < j){
          return true;
	}
	else {
	 return false;
	}

}

RETURNS(bool)
bool baz(uint32_t i, uint64_t j){

        if (i < j){
          return true;
        }
        else {
         return false;
        }

}

RETURNS(bool)
bool boo(uint64_t i, uint32_t j){

        if (i < j){
          return true;
        }
        else {
         return false;
        }

}
