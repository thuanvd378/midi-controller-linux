# ──────────────────────────────────────────────────────────────────
# Root Makefile for Custom MIDI Driver Project
# Assigned to Person 5 (Build Systems & Integration)
# ──────────────────────────────────────────────────────────────────

.PHONY: all driver tests load unload clean

# Default rule builds both kernel module and user-space app
all: driver tests

driver:
	$(MAKE) -C driver

tests:
	$(MAKE) -C tests

# Quick loading and unloading scripts
load:
	sudo insmod driver/custom_midi.ko
	@echo "Module loaded. Check 'dmesg | tail' and 'ls -l /dev/custom_midi'."

unload:
	sudo rmmod custom_midi
	@echo "Module unloaded."

clean:
	$(MAKE) -C driver clean
	$(MAKE) -C tests clean
