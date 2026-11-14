/*
    Single-word Barrett reduction target: r = x mod N.

    'b' + 3 bytes : x, little-endian (24-bit)
    'r' + 4 bytes : x mod N, little-endian
*/

#include <stdint.h>
#include "hal.h"
#include "simpleserial.h"

#define N 17669               // HQC-1 modulus
#define MU ((1ULL << 32) / N) // floor(2^32 / N)

// volatile keeps the load and store inside the trigger window.
volatile uint32_t x_in;
volatile uint32_t r_out;

/*
    Returns x mod N without branches.
    q is x / N or one too small, so r needs at most one subtraction of N.
*/
uint32_t barrett_reduce(uint32_t x)
{
    uint32_t q = ((uint64_t)x * MU) >> 32;
    uint32_t r = x - q * N;

    uint32_t mask = -(((r - N) >> 31) ^ 1); // all ones if r >= N, else 0
    r -= mask & N;

    return r;
}

uint8_t reduce(uint8_t *buf, uint8_t len)
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
    Sets up the board, registers the 'b' command,
    then serves SimpleSerial commands forever.
*/
int main(void)
{
    platform_init();
    init_uart();
    trigger_setup();

    simpleserial_init();
    simpleserial_addcmd('b', 3, reduce);

    while (1)
        simpleserial_get();
}