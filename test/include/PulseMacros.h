#ifdef C2PULSE
#define STR(...) #__VA_ARGS__
#define REQUIRES(s) [[clang::annotate("requires:" STR(s) "|END")]]
#define ENSURES(...) [[clang::annotate("ensures:" STR(__VA_ARGS__) "|END")]]
#define PRESERVES(...) [[clang::annotate("preserves:" STR(__VA_ARGS__) "|END")]]
#define RETURNS(s) [[clang::annotate("returns:" STR(s)"|END")]]
#define ISARRAY(len) [[clang::annotate("array:" STR(len)"|END")]]
#define INVARIANTS(...) [[clang::annotate("invariants:" STR(__VA_ARGS__)"|END")]]
#define LEMMA(l) [[clang::annotate("lemma:" STR(l)"|END")]]0
#define ERASED_ARG(arg) [[clang::annotate("erased_arg:" STR(arg)"|END")]]
#define GHOST_ARG(arg) [[clang::annotate("erased_arg:" STR(arg)"|END")]]
#define ASSERT(message) [[clang::annotate("assert:" STR(message) "|END")]]0
#define HEAPALLOCATED(message) [[clang::annotate("heap_allocated:" STR(message) "|END")]]
#define EXPECT_FAILURE(...)                                                    \
  [[clang::annotate("expect_failure:" STR(__VA_ARGS__) "|END")]]

#define CONCAT_IND(x, y) x ## y
#define CONCAT(x, y) CONCAT_IND(x, y)
#define INCLUDE(...) [[clang::annotate("includes", __COUNTER__)]] void CONCAT(__pulse_include_anchor_, __COUNTER__) (void) {}
//ISPURE could take additional args but these are not used anywhere at the moment. 
//For now, this attribute is used to tell if a function is pure.
//Vidush: TODO: This attribute is not parsed in the compiler atm. 
#define ISPURE(...) [[clang::annotate("ispure:" STR(__VA_ARGS__)"|END")]]
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
