#ifdef C2PULSE
#define STR(...) #__VA_ARGS__
#define REQUIRES(s) [[clang::pulse("requires:" STR(s) "|END")]]
#define ENSURES(...) [[clang::pulse("ensures:" STR(__VA_ARGS__) "|END")]]
#define PRESERVES(...) [[clang::pulse("preserves:" STR(__VA_ARGS__) "|END")]]
#define RETURNS(s) [[clang::pulse("returns:" STR(s)"|END")]]
#define ISARRAY(len) [[clang::pulse("array:" STR(len)"|END")]]
#define INVARIANTS(...) [[clang::pulse("invariants:" STR(__VA_ARGS__)"|END")]]
#define LEMMA(l) [[clang::pulse("lemma:" STR(l)"|END")]]0
#define ERASED_ARG(arg) [[clang::pulse("erased_arg:" STR(arg)"|END")]]
#define GHOST_ARG(arg) [[clang::pulse("erased_arg:" STR(arg)"|END")]]
#define ASSERT(message) [[clang::pulse("assert:" STR(message) "|END")]]0
#define HEAPALLOCATED(message) [[clang::pulse("heap_allocated:" STR(message) "|END")]]
#define EXPECT_FAILURE(...)                                                    \
  [[clang::pulse("expect_failure:" STR(__VA_ARGS__) "|END")]]

#define CONCAT_IND(x, y) x ## y
#define CONCAT(x, y) CONCAT_IND(x, y)
#define INCLUDE(...) [[clang::pulse("includes:" STR(__VA_ARGS__) "|END")]] void CONCAT(__pulse_include_anchor_, __COUNTER__) (void) {}
//ISPURE could take additional args but these are not used anywhere at the moment. 
//For now, this attribute is used to tell if a function is pure.
//Vidush: TODO: This attribute is not parsed in the compiler atm. 
#define ISPURE(...) [[clang::pulse("ispure:" STR(__VA_ARGS__)"|END")]]
#else 
#define STR(...)
#define REQUIRES(s)
#define ENSURES(...)
#define ISPURE(...)
#define PRESERVES(...)
#define RETURNS(s)
#define ISARRAY(len)
#define INVARIANTS(...)
#define LEMMA(l)
#define ERASED_ARG(arg)
#define GHOST_ARG(arg)
#define ASSERT(message)
#define HEAPALLOCATED(message)
#define CONCAT_IND(x, y)
#define CONCAT(x, y)
#define INCLUDE(...)
#define EXPECT_FAILURE(...)
#endif
