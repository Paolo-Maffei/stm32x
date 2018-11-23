// Launch CP/M, saved inside emulator and copied to internal flash.

#include <jee.h>
#include <string.h>

const uint8_t* FlashBase = (const uint8_t*) 0x40000;

extern "C" {
#include "context.h"
#include "z80emu.h"
#include "macros.h"
}

const uint8_t rom [] = {
#include "sys.h";
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
            if (pos < sizeof rom)
                memcpy(MAINMEM + HL, rom + pos, 128 * B);
            else
                memset(MAINMEM + HL, 0xE5, 128 * B);
            A = 0;
            break;
        }
        case 5: // write
            A = 1;
            break;
        default:
            printf("syscall %d @ %04x ?\n", req, state->pc);
            while (1) {}
    }
}

int main() {
    console.init();
    console.baud(115200, fullSpeedClock()/2);

    // erase boot block and store some example code in it
    printf("erasing...\n");
    Flash::erasePage(FlashBase);

    printf("%08x %08x\n", ((const uint32_t*) FlashBase)[0],
                          ((const uint32_t*) FlashBase)[1]);

    printf("writing...\n");
    for (int i = 0; i < sizeof rom; i += 4)
        Flash::write32(FlashBase + i, ((const uint32_t*) rom)[i>>2]);

    printf("%08x %08x\n", ((const uint32_t*) FlashBase)[0],
                          ((const uint32_t*) FlashBase)[1]);

    // now emulate a boot loader which loads the first "disk" block at 0x0000
    memcpy(MAINMEM, FlashBase, 128);
    Z80Reset(&zex.state);

    while (!zex.done)
        Z80Emulate(&zex.state, 4000000, &zex);

    while (1) {}
}
