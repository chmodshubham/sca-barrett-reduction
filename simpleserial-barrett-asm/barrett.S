/*
    HQC-1's Barrett reduction in assembly, with the trigger pin set directly.

    The instructions are the ones the compiler makes for HQC's barrett_reduce
    (see simpleserial-barrett). The trigger is one store to the pin register,
    not a function call, so the window holds only the instructions between
    the two stores. TRIGGER_STAGE (make TRIGGER_STAGE=<n>) picks where the
    trigger goes low: 1 after the last instruction, then one instruction
    earlier per stage, up to 15, right after trigger high (an empty window).
*/

    .syntax unified
    .cpu cortex-m4
    .thumb

// Copied from HQC v5.1.0, src/ref/hqc-1/parameters.h
#define PARAM_N 17669
#define PARAM_N_MU 243079

#if PARAM_N_MU != 4294967296 / PARAM_N
#error "PARAM_N_MU must be floor(2^32 / PARAM_N)"
#endif

#if TRIGGER_STAGE < 1 || TRIGGER_STAGE > 15
#error "TRIGGER_STAGE must be 1 to 15"
#endif

// The trigger is pin PA7: writing bit 7 to SODR sets it high, to CODR sets it low
#define PIOA_SODR 0x400E0E30
#define PIOA_CODR 0x400E0E34
#define TRIGGER_PIN 0x80

// Sets the trigger low here if TRIGGER_STAGE is n (r6 = CODR address, r7 = pin bit).
    .macro trigger_low_at n
    .if TRIGGER_STAGE == \n
    str     r7, [r6]
    .endif
    .endm

/*
    void barrett_window(void)
    Reads x from x_in and writes x mod PARAM_N to r_out. Sets the trigger high
    before the first Barrett instruction and low at TRIGGER_STAGE.
*/
    .text
    .global barrett_window
    .type   barrett_window, %function
    .thumb_func
barrett_window:
    push    {r4, r5, r6, r7, lr}
    ldr     r4, =PIOA_SODR
    ldr     r6, =PIOA_CODR
    movs    r7, #TRIGGER_PIN
    ldr     r5, =x_in

    str     r7, [r4]                @ trigger high
    trigger_low_at 15
    ldr     r2, [r5]                @ x = x_in
    trigger_low_at 14
    ldr     r3, =PARAM_N_MU         @ mu
    trigger_low_at 13
    ldr     r5, =r_out              @ address of r_out
    trigger_low_at 12
    umull   r1, r3, r3, r2          @ q = (x * mu) >> 32, in r3
    trigger_low_at 11
    movw    r1, #PARAM_N            @ N
    trigger_low_at 10
    mls     r2, r1, r3, r2          @ r = x - q * N
    trigger_low_at 9
    sub.w   r3, r2, #(PARAM_N - 5)  @ t = r - N, first step
    trigger_low_at 8
    subs    r3, #5                  @ t = r - N, second step
    trigger_low_at 7
    mvns    r3, r3                  @ ~t
    trigger_low_at 6
    lsrs    r3, r3, #31             @ reduce_flag = (~t) >> 31
    trigger_low_at 5
    negs    r3, r3                  @ mask = -reduce_flag
    trigger_low_at 4
    ands    r3, r1                  @ sel = mask & N
    trigger_low_at 3
    subs    r2, r2, r3              @ r = r - sel
    trigger_low_at 2
    str     r2, [r5]                @ r_out = r
    trigger_low_at 1

    pop     {r4, r5, r6, r7, pc}
    .size   barrett_window, . - barrett_window
    .ltorg
