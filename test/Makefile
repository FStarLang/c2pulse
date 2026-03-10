.PHONY: all
all: verify

TESTS := $(wildcard *.c)

CACHE_DIR ?= _cache

C2PULSE ?= ../target/debug/c2pulse

# Convert foo.c -> Foo.fst (capitalize first letter, change extension)
c_to_fst = $(shell echo '$(1)' | sed 's/^\(.\)/\U\1/;s/\.c$$/.fst/')

ALL_FST_FILES := $(foreach t,$(TESTS),$(call c_to_fst,$(t)))

# Generate per-file rules: Foo.fst: foo.c $(C2PULSE) c2pulse.h
define make_fst_rule
$(call c_to_fst,$(1)): $(1) $$(C2PULSE) c2pulse.h
endef
$(foreach t,$(TESTS),$(eval $(call make_fst_rule,$(t))))

ALL_CHECKED_FILES = $(addprefix $(CACHE_DIR)/,$(addsuffix .checked,$(ALL_FST_FILES)))

FSTAR_EXE ?= ../opt/run-fstar.sh
FSTAR = $(FSTAR_EXE) \
	--cache_checked_modules	\
	--cache_dir $(CACHE_DIR) \
	--already_cached Prims,FStar,Pulse.Nolib,Pulse.Class,Pulse.Lib,PulseCore

.PHONY: verify
verify: $(ALL_CHECKED_FILES)

%.fst:
	$(C2PULSE) $<

$(CACHE_DIR)/%.fst.checked: %.fst
	$(FSTAR) --force $<
	touch $@
