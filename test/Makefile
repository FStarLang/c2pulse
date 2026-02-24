.PHONY: all
all: verify

TESTS = $(wildcard *.c)

CACHE_DIR ?= _cache

$(CACHE_DIR)/.depend: $(TESTS) Makefile
	mkdir -p $(CACHE_DIR)
	echo >$@
	echo -n ALL_FST_FILES= >>$@
	echo $(TESTS) | sed 's/ /\n/g' | sed 's/^\(.\)\(.*\)\..*/\U\1\E\2.fst/' | tr '\n' ' ' >>$@
	echo >>$@
	echo $(TESTS) | sed 's/ /\n/g' | sed 's;^\(.\)\(.*\)\..*;\U\1\E\2.fst: & $$(C2PULSE)\n;' >>$@

C2PULSE ?= ../target/debug/c2pulse

include $(CACHE_DIR)/.depend

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
