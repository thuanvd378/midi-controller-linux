# ──────────────────────────────────────────────────────────────────
# Root Makefile for Custom MIDI Driver Project
# ──────────────────────────────────────────────────────────────────

.PHONY: all driver load unload clean

# Default rule builds the kernel module
all: driver

driver:
	$(MAKE) -C driver

# Quick loading and unloading scripts
load:
	sudo insmod driver/custom_midi.ko
	@echo "Module loaded. Check 'dmesg | tail' and 'ls -l /dev/custom_midi'."

unload:
	sudo rmmod custom_midi
	@echo "Module unloaded."

clean:
	$(MAKE) -C driver clean
