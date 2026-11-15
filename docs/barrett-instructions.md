# Barrett reduction instructions

This page explains each instruction of `handle_barrett`, the board function that runs the Barrett reduction in `tvla_barrett.ipynb`. Use it to match the leaking clock cycles from the notebook to the code.

## Print the listing

The notebook prints the listing in its "Where it leaks" section. To print it from a terminal, first build the firmware as in step 2 of the [README](../README.md). The path below is the one the README uses; change `~/chipwhisperer-setup` if your clone is elsewhere.

```bash
# Print the board instructions for the 'b' command
conda activate cw
cd ~/chipwhisperer-setup/sca-barrett-reduction
arm-none-eabi-objdump --disassemble=handle_barrett simpleserial-barrett/simpleserial-barrett-CWHUSKY.elf
```

The compiler puts `barrett_reduce` inside `handle_barrett`, so it has no separate listing. The tables below follow the listing from top to bottom.

## Before the trigger

These instructions are not recorded.

| Instruction | What it does | C code |
| --- | --- | --- |
| `push {r3, r4, r5, lr}` | Saves the registers this function uses and the return address. | start of `handle_barrett` |
| `ldrb r3, [r0, #2]`, `ldrb r2, [r0, #1]` | Read the top and middle input bytes from `buf`. | `buf[2]`, `buf[1]` |
| `ldr r5, [pc, #88]` | Loads the address of `x_in`. | |
| `lsls r3, r3, #16` | Moves the top byte to bits 16 to 23. | `buf[2] << 16` |
| `orr.w r3, r3, r2, lsl #8` | Adds the middle byte at bits 8 to 15. | OR with `buf[1] << 8` |
| `ldrb r2, [r0, #0]` | Reads the low input byte from `buf`. | `buf[0]` |
| `orrs r3, r2` | Adds the low byte. | OR with `buf[0]` |
| `str r3, [r5, #0]` | Writes `x` to `x_in` in memory. | `x_in = ...` |
| `ldr r3, [pc, #76]`, `mov r4, r0` | Load the address of `trigger_high`, and keep the `buf` address for later. | |
| `blx r3` | Calls `trigger_high`. It sets the trigger pin high, which starts the recording, then runs `pio_configure_pin` inside the window. | `trigger_high()` |

## Inside the trigger window

These instructions run the Barrett reduction.

| Instruction | What it does | C code |
| --- | --- | --- |
| `ldr r2, [r5, #0]` | Reads `x` back from `x_in`. | `barrett_reduce(x_in)` |
| `ldr r3, [pc, #72]` | Loads `PARAM_N_MU` = 243079 (`0x0003b587`). | |
| `ldr r5, [pc, #76]` | Loads the address of `r_out`. | |
| `umull r1, r3, r3, r2` | Multiplies `x` by `PARAM_N_MU` into a 64-bit result. The upper 32 bits, in `r3`, are `q`. | `q = (x * PARAM_N_MU) >> 32` |
| `movw r1, #17669` | Loads `PARAM_N`. | |
| `mls r2, r1, r3, r2` | Computes `x - q * PARAM_N`. | `r = x - q * PARAM_N` |
| `sub.w r3, r2, #17664`, `subs r3, #5` | Compute `r - PARAM_N` in two steps, because 17669 does not fit in one instruction's constant. | `r - PARAM_N` |
| `mvns r3, r3` | Flips every bit. Together with the next instruction, this gives the `^ 1`. | `^ 1` |
| `lsrs r3, r3, #31` | Keeps only the top bit: 1 when `r` is at least `PARAM_N`, 0 otherwise. | `reduce_flag = ((r - PARAM_N) >> 31) ^ 1` |
| `negs r3, r3` | Turns 1 into all ones and keeps 0 as 0. | `mask = -reduce_flag` |
| `ands r3, r1` | Gives `PARAM_N` when the mask is all ones, otherwise 0. | `mask & PARAM_N` |
| `subs r2, r2, r3` | Subtracts it from `r`. This is the result. | `r -= mask & PARAM_N` |
| `ldr r3, [pc, #48]` | Loads the address of `trigger_low`. | |
| `str r2, [r5, #0]` | Writes the result to `r_out` in memory. | `r_out = ...` |
| `blx r3` | Calls `trigger_low`. It sets the trigger pin low, which ends the trigger window, then runs `pio_configure_pin`. | `trigger_low()` |

## After the trigger

These instructions send the reply.

| Instruction | What it does | C code |
| --- | --- | --- |
| `ldr r3, [r5, #0]` | Reads the result back from `r_out`. | `uint32_t r = r_out` |
| `strb` and `lsrs` lines | Write the 4 bytes of the result into `buf`, lowest byte first. | `buf[0] = r` to `buf[3] = r >> 24` |
| `mov r2, r4`, `ldr r3, [pc, #28]`, `movs r1, #4`, `movs r0, #114` | Prepare the call: `buf`, the address of `simpleserial_put`, length 4, and 114, which is `'r'`. | |
| `blx r3` | Sends the reply to the computer. | `simpleserial_put('r', 4, buf)` |
| `movs r0, #0`, `pop {r3, r4, r5, pc}` | Return 0 and go back to the caller. | `return 0` |
| `.word` lines | Constants the `ldr ... [pc, ...]` lines load: the addresses of `x_in`, `trigger_high`, `r_out`, `trigger_low`, `simpleserial_put`, and `PARAM_N_MU`. Code addresses end in 1, which marks Thumb code. | |

> [!NOTE]
> These tables match the firmware built from `simpleserial-barrett/simpleserial-barrett.c` with the project makefile. If the C code, the compiler or its flags change, the instructions can change too, so compare the tables with the printed listing.
