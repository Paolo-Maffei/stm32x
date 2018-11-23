// Launch CP/M, saved inside emulator and copied to a RAM-based disk.

#include <jee.h>
#include <string.h>

extern "C" {
#include "context.h"
#include "z80emu.h"
#include "macros.h"
}

// use the extra 64K of CCM ram as ram disk for now
// (the F407's 192K memory is split into 128K main + 64K CCM ram)
auto ram = (uint8_t* const) 0x10000000;
constexpr int ramSize = 0x10000;

const uint8_t sys [] = {
#include "sys.h"
};

const uint8_t hexsave [] = {
#include "hexsave.h"
};

UartDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
	return 0;
}

Context zex;

void SystemCall (Context* z, int req) {

    Z80_STATE* state = &(z->state);
    switch (req) {
        case 0: // coninst
            A = console.readable() ? 0xFF : 0x00;
            break;
        case 1: // conin
            A = console.getc();
            break;
        case 2: // conout
            console.putc(C);
            break;
        case 3: // constr
            for (uint16_t i = DE; MAINMEM[i] != 0; i++)
                console.putc(MAINMEM[i]);
            break;
        case 4: { // read
#if 0
148 FEE7  3A 0004     read:   ld a,(usrdrv)
149 FEEA  06 01               ld b,1
150 FEEC  ED 5B FF88          ld de,(seksat)
151 FEF0  2A FF8A             ld hl,(dmaadr)
152 FEF3  DB 04               in a,(4)
153 FEF5  C9          	ret
#endif
            uint8_t e = DE, d = DE >> 8;
            int pos = 128 * (e + 26 * d);
            if (pos < ramSize)
                memcpy(MAINMEM + HL, ram + pos, 128 * B);
            else
                memset(MAINMEM + HL, 0xE5, 128 * B);
            A = 0;
            break;
        }
        case 5: { // write
            uint8_t e = DE, d = DE >> 8;
            int pos = 128 * (e + 26 * d);
            if (pos + 128 * B < ramSize) {
                memcpy(ram + pos, MAINMEM + HL, 128 * B);
                A = 0;
            } else
                A = 1;
            break;
        }
        default:
            printf("syscall %d @ %04x ?\n", req, state->pc);
            while (1) {}
    }
}

int main() {
    console.init();
    console.baud(115200, fullSpeedClock()/2);

    memset(ram, 0xE5, ramSize);
    memcpy(ram, sys, sizeof sys);

    // now emulate a boot loader which loads the first "disk" block at 0x0000
    memcpy(MAINMEM, ram, 128);
    // and leave a copy of HEXSAVE.COM in the TPA for saving in CP/M
    memcpy(MAINMEM + 0x0100, hexsave, sizeof hexsave);

    Z80Reset(&zex.state);

    while (!zex.done)
        Z80Emulate(&zex.state, 4000000, &zex);

    while (1) {}
}
