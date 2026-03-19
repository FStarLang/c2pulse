.PHONY: all
all: rust lib

.PHONY: rust
rust:
	cargo build

ifeq ($(C2PULSE_OPT),0)

opt/opam-available.done:
	touch $@

else

opt/opam-available.done: opt/opam-init.done
	touch $@

opt/opam-init.done:
	opam update
	opam init --no-setup --root=opt/opam --compiler=5.3.0
	touch $@

endif

opt/env.mk opt/env.sh: opt/opam-available.done
	./opt/configure

include opt/env.mk

ifeq ($(C2PULSE_OPT),0)

.PHONY: build-pulse
build-pulse:

else

.PHONY: build-opam
build-opam: opt/opam.done

opt/opam.done: opt/fstar/fstar.opam
	opam install --deps-only $<
	touch $@

.PHONY: build-fstar
build-fstar: opt/opam.done
	$(MAKE) -C opt/fstar ADMIT=1

.PHONY: build-pulse
build-pulse: build-fstar

endif

.PHONY: lib
lib: build-pulse
	$(MAKE) -C pulse

.PHONY: -testsuite
-testsuite: rust lib
	$(MAKE) -C test

.PHONY: format-check
format-check:
	cargo fmt --check
	clang-format --dry-run --Werror cpp/impl.cpp

.PHONY: test
test: rust lib -testsuite
# Only run formatting checks when tests succeed
	$(MAKE) format-check