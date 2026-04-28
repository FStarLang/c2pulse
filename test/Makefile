.PHONY: all
all: verify

TESTS := $(filter-out $(EXPECTED_FAIL),$(wildcard *.c))

CACHE_DIR ?= _cache

C2PULSE ?= ../target/debug/pal

# Tests that are expected to fail (known unsupported features)
EXPECTED_FAIL :=

# Per-test pal options (e.g., OPTS_include = -I include)
OPTS_include = -I include

# Convert foo.c -> Foo.fst (capitalize first letter, change extension)
c_to_fst = $(shell echo '$(1)' | sed 's/^\(.\)/\U\1/;s/\.c$$/.fst/')

ALL_FST_FILES := $(foreach t,$(TESTS),$(call c_to_fst,$(t)))

# Generate per-file rules: Foo.fst: foo.c $(C2PULSE) pal.h
define make_fst_rule
$(call c_to_fst,$(1)): $(1) $$(C2PULSE) pal.h
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
	$(C2PULSE) $(OPTS_$(basename $<)) $<

$(CACHE_DIR)/%.fst.checked: %.fst
	$(FSTAR) --force $<
	touch $@

# ── Expected-fail targets ──────────────────────────────────────────
# Run with: make test-expected-fail
.PHONY: test-expected-fail
test-expected-fail: $(patsubst %.c,test-xfail-%,$(EXPECTED_FAIL))

test-xfail-%: %.c $(C2PULSE)
	@echo "=== Expected failure: $< ==="
	@output=$$($(C2PULSE) $< 2>&1); \
	if echo "$$output" | grep -q "^error:"; then \
	  echo "$$output"; \
	  echo "OK — got expected error"; \
	else \
	  echo "$$output"; \
	  echo "UNEXPECTED PASS — $< should have failed"; exit 1; \
	fi
