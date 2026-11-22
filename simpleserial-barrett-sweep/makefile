# Firmware build for the trigger sweep on the ChipWhisperer Husky (CW312T-SAM4S), SimpleSerial 1.1.
# Usage: make clean && make TRIGGER_STAGE=<1 to 11>   (1, the normal position, if not given)
# (add FIRMWAREPATH=<chipwhisperer clone>/firmware/mcu if the clone is elsewhere)

TARGET = simpleserial-barrett-sweep
SRC += barrett_sweep.c

PLATFORM ?= CWHUSKY
SS_VER ?= SS_VER_1_1
CRYPTO_TARGET ?= NONE

# The point where the trigger goes low (1 = normal position), see barrett_sweep.c
TRIGGER_STAGE ?= 1
CDEFS += -DTRIGGER_STAGE=$(TRIGGER_STAGE)

# Default: ChipWhisperer cloned next to this repository.
FIRMWAREPATH ?= ../../chipwhisperer/firmware/mcu

ifeq ($(wildcard $(FIRMWAREPATH)/Makefile.inc),)
$(error ChipWhisperer firmware not found at '$(FIRMWAREPATH)'. Pass FIRMWAREPATH=<chipwhisperer clone>/firmware/mcu)
endif

include $(FIRMWAREPATH)/simpleserial/Makefile.simpleserial
include $(FIRMWAREPATH)/Makefile.inc
