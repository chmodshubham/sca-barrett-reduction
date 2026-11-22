/*
    Board code for the assembly trigger sweep of HQC-1's Barrett reduction.
    The reduction and both trigger edges are in barrett.S.

    The computer sends 'b' and 3 bytes: the number x (lowest byte first).
    The board replies 'r' and 4 bytes: x mod PARAM_N (lowest byte first).
*/

#include <stdint.h>
#include "hal.h"
#include "simpleserial.h"

// Read and written by barrett_window in barrett.S.
volatile uint32_t x_in;
volatile uint32_t r_out;

void barrett_window(void);

/*
    Runs when the computer sends 'b'. Stores x for barrett_window, runs it,
    and sends the result back.
*/
uint8_t handle_barrett(uint8_t *buf, uint8_t len)
{
    x_in = buf[0] | (buf[1] << 8) | (buf[2] << 16);

    barrett_window();

    uint32_t r = r_out;
    buf[0] = r;
    buf[1] = r >> 8;
    buf[2] = r >> 16;
    buf[3] = r >> 24;
    simpleserial_put('r', 4, buf);
    return 0;
}

/*
    Sets up the board and the trigger pin, links the 'b' command to
    handle_barrett, then waits for commands from the computer forever.
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
