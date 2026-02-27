.PHONY: all
all: translate

DIRS = general issue-related memory_bugs/toy_examples memory_bugs/kdb engine

# All C test files across subdirectories
ALL_TESTS = $(wildcard $(addsuffix /*.c,$(DIRS)))

C2PULSE_HOME ?= ../..
C2PULSE ?= $(C2PULSE_HOME)/target/debug/c2pulse

# c2pulse writes output files next to the input .c file.
# The .fst filename is the capitalised basename: foo.c -> Foo.fst
fst_of = $(dir $(1))$(shell echo '$(notdir $(1))' | sed 's/^\(.\)/\U\1/; s/\.c$$/.fst/')

ALL_FST_FILES = $(foreach t,$(ALL_TESTS),$(call fst_of,$(t)))

.PHONY: translate
translate: $(ALL_FST_FILES)

.SECONDEXPANSION:

define TRANSLATE_RULE
$(call fst_of,$(1)): $(1) $$(C2PULSE)
	$$(C2PULSE) $$<
endef
$(foreach t,$(ALL_TESTS),$(eval $(call TRANSLATE_RULE,$(t))))

ALL_CHECKED_FILES = $(addsuffix .checked,$(ALL_FST_FILES))

FSTAR_EXE ?= $(C2PULSE_HOME)/opt/run-fstar.sh
FSTAR = $(FSTAR_EXE) \
	--cache_checked_modules	\
	--already_cached Prims,FStar,Pulse.Nolib,Pulse.Class,Pulse.Lib,PulseCore

.PHONY: verify
verify: $(ALL_CHECKED_FILES)

.PHONY: clean
clean:
	find . -name '*.checked' -delete
	find . -name '*.fst' -delete
	find . -name '*_diagnostics.json' -delete
	find . -name '*_source_range_info.json' -delete

%.checked: %
	$(FSTAR) --force $<
	touch $@
