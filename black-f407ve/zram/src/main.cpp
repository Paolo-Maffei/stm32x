// Launch CP/M, saved inside emulator and copied to a RAM-based disk.

#include <jee.h>
#include <string.h>

extern "C" {
#include "zextest.h"
#include "z80emu.h"
#include "macros.h"
}

uint8_t ram [0xE000]; // TODO more causes problems with PlatformIO upload (?)

const uint8_t rom [] = {
#include "sys.h";
};

UartDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
	return 0;
}

ZEXTEST zex;

void SystemCall (ZEXTEST* z, int req) {

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
            for (uint16_t i = DE; z->memory[i] != 0; i++)
                console.putc(z->memory[i]);
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
            if (pos < sizeof ram)
                memcpy(z->memory + HL, ram + pos, 128 * B);
            else
                memset(z->memory + HL, 0xE5, 128 * B);
            A = 0;
            break;
        }
        case 5: { // write
            uint8_t e = DE, d = DE >> 8;
            int pos = 128 * (e + 26 * d);
            if (pos + 128 * B < sizeof ram) {
                memcpy(ram + pos, z->memory + HL, 128 * B);
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

    memset(ram, 0xE5, sizeof ram);
    memcpy(ram, rom, sizeof rom);

    // now emulate a boot loader which loads the first "disk" block at 0x0000
    memcpy(zex.memory, ram, 128);
    Z80Reset(&zex.state);

    while (!zex.is_done)
        Z80Emulate(&zex.state, 4000000, &zex);

    while (1) {}
}
