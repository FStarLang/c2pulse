.PHONY: all
all: translate

# All C test files across subdirectories
ALL_TESTS = $(wildcard general/*.c issue-related/*.c memory_bugs/toy_examples/*.c memory_bugs/kdb/*.c engine/*.c)

C2PULSE ?= ../../target/debug/c2pulse

CACHE_DIR ?= _cache

# c2pulse writes output files next to the input .c file.
# The .fst filename is the capitalised basename: foo.c -> Foo.fst
fst_of = $(dir $(1))$(shell echo '$(notdir $(1))' | sed 's/^\(.\)/\U\1/; s/\.c$$/.fst/')

ALL_FST_FILES = $(foreach t,$(ALL_TESTS),$(call fst_of,$(t)))

.PHONY: translate
translate: $(ALL_FST_FILES)

.SECONDEXPANSION:

define TRANSLATE_RULE
$(call fst_of,$(1)): $(1) $$(C2PULSE)
	$$(C2PULSE) $$< || true
endef
$(foreach t,$(ALL_TESTS),$(eval $(call TRANSLATE_RULE,$(t))))

.PHONY: clean
clean:
	rm -rf $(CACHE_DIR)
	find . -name '*.fst' -delete
	find . -name '*_diagnostics.json' -delete
	find . -name '*_source_range_info.json' -delete
