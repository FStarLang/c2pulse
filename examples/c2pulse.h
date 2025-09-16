#ifdef C2PULSE

#define __c2pulse_concat_ind(x, y) x ## y
#define __c2pulse_concat(x, y) __c2pulse_concat_ind(x, y)
#define _include_pulse(...) [[clang::annotate("c2pulse-includes", __COUNTER__)]] \
    void __c2pulse_concat(__c2pulse_include_anchor_, __COUNTER__) (void) {}

#define _requires(p) __attribute__((annotate("c2pulse-requires", __COUNTER__)))
#define _ensures(p) __attribute__((annotate("c2pulse-ensures", __COUNTER__)))

#define _plain [[clang::annotate("c2pulse-plain")]]
#define _consumes [[clang::annotate("c2pulse-consumes")]]

#define __capture_args(args...) __COUNTER__
#define _inline_pulse(args...) _inline_pulse(_capture_args(args...))

#else

#define _include_pulse(...)
#define _requires(p)
#define _ensures(p)

#define _plain
#define _consumes

#endif

#define _preserves(p) _requires(p) _ensures(p)
#define _allocated _preserves((_slprop) _inline_pulse(freeable this))