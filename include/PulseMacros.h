#define STR(...) #__VA_ARGS__
#define REQUIRES(s) [[clang::pulse("requires:"#s"|END")]] 
#define ENSURES(s) [[clang::pulse("ensures:" STR(s)"|END")]]
#define RETURNS(s) [[clang::pulse("returns:" STR(s)"|END")]]
#define ISARRAY(len) [[clang::pulse("array:" STR(len)"|END")]]
#define INVARIANTS(...) [[clang::pulse("invariants:" STR(__VA_ARGS__)"|END")]]
#define LEMMA(l) [[clang::pulse("lemma:" STR(l)"|END")]]
#define ERASED_ARG(arg) [[clang::pulse("erased_arg:" STR(arg)"|END")]]
#define ASSERT(message) [[clang::pulse("assert:" STR(message) "|END")]]
#define HEAPALLOCATED(message) [[clang::pulse("heap_allocated:" STR(message) "|END")]]

#define CONCAT_IND(x, y) x ## y
#define CONCAT(x, y) CONCAT_IND(x, y)
#define INCLUDE(...) [[clang::pulse("includes:" STR(__VA_ARGS__) "|END")]] void CONCAT(__pulse_include_anchor_, __COUNTER__) (void) {}
