/*
    Board code for the power test of HQC-1's Barrett reduction.

    The computer sends 'b' and 3 bytes: the number x (lowest byte first).
    The board replies 'r' and 4 bytes: x mod PARAM_N (lowest byte first).

    The trigger goes high just before barrett_reduce and low just after it,
    so the recording starts at the reduction.
*/

#include <stdint.h>
#include "hal.h"
#include "simpleserial.h"

// Copied from HQC v5.1.0, src/ref/hqc-1/parameters.h
#define PARAM_N 17669
#define PARAM_N_MU 243079ULL

// Stop the build if PARAM_N_MU does not match PARAM_N.
_Static_assert(PARAM_N_MU == (1ULL << 32) / PARAM_N, "PARAM_N_MU must be floor(2^32 / PARAM_N)");

/**
 * @brief Constant-time Barrett reduction modulo PARAM_N.
 *
 * Reduces \p x modulo PARAM_N using the precomputed value PARAM_N_MU = ⌊2^32 / PARAM_N⌋.
 *
 * @param[in] x Input value to reduce.
 * @return x mod PARAM_N in constant time.
 */
static inline uint32_t barrett_reduce(uint32_t x)
{
    uint64_t q = ((uint64_t)x * PARAM_N_MU) >> 32;
    uint32_t r = x - (uint32_t)(q * PARAM_N);

    uint32_t reduce_flag = (((r - PARAM_N) >> 31) ^ 1);
    uint32_t mask = -reduce_flag;
    r -= mask & PARAM_N;
    return r;
}

// volatile makes the board read x and write the result while the trigger is high.
volatile uint32_t x_in;
volatile uint32_t r_out;

/*
    Runs when the computer sends 'b'. Reads x, runs barrett_reduce with the
    trigger high, and sends the result back. The trigger marks where the
    reduction starts and ends.
*/
uint8_t handle_barrett(uint8_t *buf, uint8_t len)
{
    x_in = buf[0] | (buf[1] << 8) | (buf[2] << 16);

    trigger_high();
    r_out = barrett_reduce(x_in);
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
