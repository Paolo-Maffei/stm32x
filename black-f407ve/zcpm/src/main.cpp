// Launch CP/M, saved inside emulator and copied to a RAM-based disk.

#include <jee.h>
#include <jee/spi-flash.h>
#include <jee/spi-sdcard.h>
#include <string.h>
#include "spi-wear.h"

extern "C" {
#include "zextest.h"
#include "z80emu.h"
#include "macros.h"
}

const uint8_t sys [] = {
#include "sys.h"
};

const uint8_t hexsave [] = {
#include "hexsave.h"
};

UartDev< PinA<9>, PinA<10> > console;
PinE<4> key0;
PinE<3> key1;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
	return 0;
}

// chip select needs a minute slowdown to work at 168 MHz
SpiGpio< PinB<5>, PinB<4>, PinB<3>, SlowPin< PinB<0>, 2 > > spi1;
SpiFlash< decltype(spi1) > spiFlash;
SpiWear< decltype(spiFlash), PinA<6> > spiWear;

SpiGpio< PinD<2>, PinC<8>, PinC<12>, PinC<11> > spi2;
SdCard< decltype(spi2) > sdCard;
FatFS< decltype(sdCard), 3 > fatFs;
FileMap< decltype(fatFs), 9 > cpma (fatFs);
FileMap< decltype(fatFs), 9 > zork1 (fatFs);

ZEXTEST zex;

void SystemCall (ZEXTEST* z, int req) {

    Z80_STATE* state = &(z->state);
    //printf("req %d A %d\n", req, A);
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
            uint8_t sec = DE, trk = DE >> 8, dsk = A;
            int blk = (256*1024/128) * dsk + 26 * trk + sec;
            for (int i = 0; i < B; ++i)
                spiWear.read128(blk + i, z->memory + HL + 128 * i);
            A = 0;
            break;
        }
        case 5: { // write
            uint8_t sec = DE, trk = DE >> 8, dsk = A;
            int blk = (256*1024/128) * dsk + 26 * trk + sec;
            for (int i = 0; i < B; ++i)
                spiWear.write128(blk + i, z->memory + HL + 128 * i);
            A = 0;
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
    printf("\r\n");

    key0.mode(Pinmode::in_pullup); // inverted logic
    key1.mode(Pinmode::in_pullup); // inverted logic

    spi1.init();
    spiWear.init();

    spi2.init();
    if (sdCard.init()) {
        printf("[sd card: sdhc %d]\n", sdCard.sdhc);

        fatFs.init();

        printf("base %d spc %d rdir %d rmax %d data %d clim %d\n\n",
                                    fatFs.base, fatFs.spc, fatFs.rdir,
                                    fatFs.rmax, fatFs.data, fatFs.clim);

        for (int i = 0; i < fatFs.rmax; ++i) {
            int o = (i*32) % 512;
            if (o == 0)
                sdCard.read512(fatFs.rdir + i/16, fatFs.buf);
            int length = *(int32_t*) (fatFs.buf+o+28);
            if (length >= 0 && '!' < fatFs.buf[o] &&
                    fatFs.buf[o] <= '~' && fatFs.buf[o+6] != '~') {
                uint8_t attr = fatFs.buf[o+11];
                printf("   %s\t", attr & 8 ? "vol:" : attr & 16 ? "dir:" : "");
                for (int j = 0; j < 11; ++j) {
                    int c = fatFs.buf[o+j];
                    if (c != ' ') {
                        if (j == 8 && (attr & 0x08) == 0)
                            printf(".");
                        printf("%c", c);
                    }
                }
                printf(" (%db)\n", length);
            }
        }
        printf("\n");

        int len;
        len = cpma.open("CPMA    CPM");
        if (len > 0)
            printf("  cpma = %db\n", len);
        len = zork1.open("ZORK1   CPM");
        if (len > 0)
            printf("  zork1 = %db\n", len);
    }

    // The "K0" and "K1" buttons are checked on power-up and reset:
    //  - both pressed: wipe all, re-install system and clear directory on 1
    //  - left "K0" button pressed: clear directory on 1
    //  - right "K1" button pressed: re-install system on 1
    // Disk 1 is the first 256 KB of spi flash, normally mounted as A:

    if (!key0 && !key1) { // wipe entire spi flash
        printf("[erasing SPI flash] ");
        spiFlash.wipe();
    }

    if (!key1) { // set up system tracks on disk 1
        printf("[updating system tracks] ");
        for (uint32_t i = 0; i < sizeof sys; i += 128)
            spiWear.write128(i / 128, sys + i);
    }

    if (!key0) { // set up empty directory blocks on disk 1
        printf("[clearing boot directory] ");
        for (int i = 0; i < 16; ++i) {
            uint8_t buf [128];
            memset(buf, 0xE5, sizeof buf);
            spiWear.write128(2*26 + i, buf);
        }
    }

    // wait for both keys to be released
    while (!key0 || !key1)
        wait_ms(100); // debounce

    // emulate a boot loader which loads the first block of "disk" A: at 0x0000
    spiWear.read128(0, zex.memory);
    // and leave a copy of HEXSAVE.COM in the TPA for saving in CP/M
    memcpy(zex.memory + 0x0100, hexsave, sizeof hexsave);

    Z80Reset(&zex.state);

    while (!zex.is_done)
        Z80Emulate(&zex.state, 4000000, &zex);

    while (1) {}
}
