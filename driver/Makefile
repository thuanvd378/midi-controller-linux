# ──────────────────────────────────────────────────────────────────
# driver/Makefile — Kbuild for the modular custom_midi LKM
# ──────────────────────────────────────────────────────────────────

obj-m += custom_midi.o
custom_midi-y := main.o fops.o hw_sim.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

.PHONY: modules clean

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
