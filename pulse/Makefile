.PHONY: all
all: verify

include ../opt/env.mk

ADMIT ?=
MAYBE_ADMIT = $(if $(ADMIT),--admit_smt_queries true)

CACHE_DIR ?= _cache

FSTAR_EXE = fstar.exe
FSTAR = $(FSTAR_EXE) \
	--cache_checked_modules	\
	--cache_dir $(CACHE_DIR) \
	--already_cached Prims,FStar,Pulse.Nolib,Pulse.Class,Pulse.Lib,PulseCore	\
	 $(MAYBE_ADMIT)

FSTAR_FILES ?= $(wildcard *.fst) $(wildcard *.fsti)
FSTAR_FILES := $(strip $(FSTAR_FILES))

.depend: $(FSTAR_FILES)
	$(FSTAR) --dep full $(FSTAR_FILES) --output_deps_to $@
include .depend

%.checked:
	$(FSTAR) $<
	touch -c $@

.PHONY: verify
verify: $(ALL_CHECKED_FILES)

.PHONY: clean
clean:
	rm -rf $(CACHE_DIR) .depend