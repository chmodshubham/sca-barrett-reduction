/*
    Board code that sets the trigger low at a chosen point of HQC-1's
    Barrett reduction. TRIGGER_STAGE picks the point (make TRIGGER_STAGE=<n>,
    the makefile sets 1 if it is not given):

    1   after r_out = barrett_reduce(x_in), the normal position
    2   after r = r - sel
    3   after sel = mask & N
    4   after mask = -reduce_flag
    5   after reduce_flag = sign ^ 1
    6   after sign = t >> 31
    7   after t = r - N
    8   after r = x - qn
    9   after qn = q * N
    10  after q = (x * mu) >> 32
    11  after reading x

    Each stage ends the window one operation earlier than the one before.

    The trigger always goes high at the same place, just before barrett_reduce.

    The computer sends 'b' and 3 bytes: the number x (lowest byte first).
    The board replies 'r' and 4 bytes: x mod PARAM_N (lowest byte first).
*/

#include <stdint.h>
#include "hal.h"
#include "simpleserial.h"

_Static_assert(TRIGGER_STAGE >= 1 && TRIGGER_STAGE <= 11, "TRIGGER_STAGE must be 1 to 11");

// Copied from HQC v5.1.0, src/ref/hqc-1/parameters.h
#define PARAM_N 17669
#define PARAM_N_MU 243079ULL

_Static_assert(PARAM_N_MU == (1ULL << 32) / PARAM_N, "PARAM_N_MU must be floor(2^32 / PARAM_N)");

/*
    Sets the trigger low here if TRIGGER_STAGE is stage. Without the two empty
    asm lines, the compiler could move the maths across the trigger_low() call.
    The first makes value ready before the call, the second keeps the code that
    uses value after it. They emit no instructions themselves, but can change
    which registers the compiler uses.
*/
#define TRIGGER_LOW_AT(stage, value)                          \
    do {                                                      \
        if (TRIGGER_STAGE == (stage)) {                       \
            __asm__ volatile("" : "+r"(value) : : "memory");  \
            trigger_low();                                    \
            __asm__ volatile("" : "+r"(value) : : "memory");  \
        }                                                     \
    } while (0)

/*
    HQC-1's Barrett reduction (x mod PARAM_N in constant time), written with
    one operation per line and a possible trigger low after each. The result
    is the same as HQC's code. The product and the shift stay on one line
    because one umull instruction gives both halves of the product. q is
    kept in 32 bits: x and mu are below 2^32, so q always fits.
*/
static inline uint32_t barrett_reduce(uint32_t x)
{
    TRIGGER_LOW_AT(11, x);
    uint32_t q = (uint32_t)(((uint64_t)x * PARAM_N_MU) >> 32);
    TRIGGER_LOW_AT(10, q);
    uint32_t qn = q * PARAM_N;
    TRIGGER_LOW_AT(9, qn);
    uint32_t r = x - qn;
    TRIGGER_LOW_AT(8, r);

    uint32_t t = r - PARAM_N;
    TRIGGER_LOW_AT(7, t);
    uint32_t sign = t >> 31;
    TRIGGER_LOW_AT(6, sign);
    uint32_t reduce_flag = sign ^ 1;
    TRIGGER_LOW_AT(5, reduce_flag);
    uint32_t mask = -reduce_flag;
    TRIGGER_LOW_AT(4, mask);
    uint32_t sel = mask & PARAM_N;
    TRIGGER_LOW_AT(3, sel);
    r = r - sel;
    TRIGGER_LOW_AT(2, r);
    return r;
}

// volatile keeps the read of x and the write of the result in place, so the
// compiler cannot move them across the trigger calls.
volatile uint32_t x_in;
volatile uint32_t r_out;

/*
    Runs when the computer sends 'b'. Reads x, runs barrett_reduce with the
    trigger high until the chosen point, and sends the result back.
*/
uint8_t handle_barrett(uint8_t *buf, uint8_t len)
{
    x_in = buf[0] | (buf[1] << 8) | (buf[2] << 16);

    trigger_high();
    r_out = barrett_reduce(x_in);
    if (TRIGGER_STAGE == 1)
        trigger_low();

    uint32_t r = r_out;
    buf[0] = r;
    buf[1] = r >> 8;
    buf[2] = r >> 16;
    buf[3] = r >> 24;
    simpleserial_put('r', 4, buf);
    return 0;
}

/*
    Sets up the board, links the 'b' command to handle_barrett,
    then waits for commands from the computer forever.
*/
int main(void)
{
    platform_init();
    init_uart();
    trigger_setup();

    simpleserial_init();
    simpleserial_addcmd('b', 3, handle_barrett);

    while (1)
        simpleserial_get();
}
