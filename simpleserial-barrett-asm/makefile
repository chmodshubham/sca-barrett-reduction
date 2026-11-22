# Firmware build for the assembly trigger sweep on the ChipWhisperer Husky (CW312T-SAM4S), SimpleSerial 1.1.
# Usage: make clean && make TRIGGER_STAGE=<1 to 15>   (1, after the last instruction, if not given)
# (add FIRMWAREPATH=<chipwhisperer clone>/firmware/mcu if the clone is elsewhere)

TARGET = simpleserial-barrett-asm
SRC += barrett_asm.c
ASRC += barrett.S

PLATFORM ?= CWHUSKY
SS_VER ?= SS_VER_1_1
CRYPTO_TARGET ?= NONE

# The instruction after which the trigger goes low, see barrett.S
TRIGGER_STAGE ?= 1
ADEFS += -DTRIGGER_STAGE=$(TRIGGER_STAGE)

# Default: ChipWhisperer cloned next to this repository.
FIRMWAREPATH ?= ../../chipwhisperer/firmware/mcu

ifeq ($(wildcard $(FIRMWAREPATH)/Makefile.inc),)
$(error ChipWhisperer firmware not found at '$(FIRMWAREPATH)'. Pass FIRMWAREPATH=<chipwhisperer clone>/firmware/mcu)
endif

include $(FIRMWAREPATH)/simpleserial/Makefile.simpleserial
include $(FIRMWAREPATH)/Makefile.inc
