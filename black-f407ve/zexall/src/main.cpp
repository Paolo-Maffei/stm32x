/* Example program using z80emu to run the zexall tests. This will 
 * check if the Z80 is correctly emulated.
 *
 * Copyright (c) 2012, 2016 Lin Ke-Fong
 * Copyright (c) 2012 Chris Pressey
 * Ported to STM32 by jcw, Nov 2018.
 *
 * This code is free, do whatever you want with it.
 */

#include <jee.h>
#include <string.h>

extern "C" {
#include "context.h"
#include "z80emu.h"
}

const uint8_t rom [] = {
#include "zexall.h";
};

UartBufDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
	return 0;
}

PinA<6> led;

/* Emulate "zexdoc.com" or "zexall.com". */

static void emulate (const void* rom, int len)
{
    static Context context;
    memcpy(MAINMEM + 0x100, rom, len);

    /* Patch the memory of the program. Reset at 0x0000 is trapped by an
     * OUT which will stop emulation. CP/M bdos call 5 is trapped by an IN.
     * See Z80_INPUT_BYTE() and Z80_OUTPUT_BYTE() definitions in z80user.h.
     */

    MAINMEM[0] = 0xd3;       /* OUT N, A */
    MAINMEM[1] = 0x00;

    MAINMEM[5] = 0xdb;       /* IN A, N */
    MAINMEM[6] = 0x00;
    MAINMEM[7] = 0xc9;       /* RET */

    context.done = 0;

    /* Emulate. */

    Z80Reset(&context.state);
    context.state.pc = 0x100;
    do {
        led.toggle();
        Z80Emulate(&context.state, 4000000, &context);
    } while (!context.done);
    led = 0;
}

/* Emulate CP/M bdos call 5 functions 2 (output character on screen) and 9
 * (output $-terminated string to screen).
 */

void SystemCall (Context *zextest, int)
{
    if (zextest->state.registers.byte[Z80_C] == 2)
        printf("%c", zextest->state.registers.byte[Z80_E]);
    else if (zextest->state.registers.byte[Z80_C] == 9) {
        int     i, c;
        for (i = zextest->state.registers.word[Z80_DE], c = 0; 
                MAINMEM[i] != '$';
                i++) {

            printf("%c", MAINMEM[i & 0xffff]);
        }
    }
}

int main() {
    console.init();
    console.baud(115200, fullSpeedClock()/2);
    led.mode(Pinmode::out);

    uint32_t start = ticks;
    emulate(rom, sizeof rom);
    printf("Emulating zexall took %d ms.\n", ticks - start);

    while (1) {}
}
