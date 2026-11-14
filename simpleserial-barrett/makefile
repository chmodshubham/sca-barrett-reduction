# Firmware build for the ChipWhisperer Husky (CW312T-SAM4S), SimpleSerial 1.1.
# Needs a ChipWhisperer clone: https://github.com/newaetech/chipwhisperer
# Usage: make   (add FIRMWAREPATH=<chipwhisperer clone>/firmware/mcu if the clone is elsewhere)

TARGET = simpleserial-barrett
SRC += simpleserial-barrett.c

PLATFORM ?= CWHUSKY
SS_VER ?= SS_VER_1_1
CRYPTO_TARGET ?= NONE

# Default: ChipWhisperer cloned next to this repository.
FIRMWAREPATH ?= ../../chipwhisperer/firmware/mcu

ifeq ($(wildcard $(FIRMWAREPATH)/Makefile.inc),)
$(error ChipWhisperer firmware not found at '$(FIRMWAREPATH)'. Pass FIRMWAREPATH=<chipwhisperer clone>/firmware/mcu)
endif

include $(FIRMWAREPATH)/simpleserial/Makefile.simpleserial
include $(FIRMWAREPATH)/Makefile.inc
