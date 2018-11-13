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
FatFS< decltype(sdCard) > fatFs;

// TODO yucky init
typedef FileMap< decltype(fatFs), 9 > DiskMap;
DiskMap disks [] = {fatFs,fatFs};
DiskMap* currDisk;
int currSect;
bool currDirty;
uint8_t currBuf [512];

void* reBlock128 (DiskMap* dmp =0, int blk =0, bool dirty =false) {
    int sect = blk / 4;  // CP/M blocks are 128b, SD card sectors are 512b
    if (dmp != currDisk || sect != currSect) {
        if (currDisk != 0 && currDirty)
            if (!currDisk->writeSect(currSect, currBuf))
                printf("WERR: disk %d sect %d blk %d\n", dmp-disks, sect, blk);
        currDisk = dmp;
        currSect = sect;
        currDirty = false;
        if (currDisk != 0)
            if (!currDisk->readSect(currSect, currBuf))
                printf("RERR: disk %d sect %d blk %d\n", dmp-disks, sect, blk);
    }
    currDirty |= dirty;
    return currBuf + (blk*128) % 512;
}

const uint8_t skewMap [] = {
    1,7,13,19,25,5,11,17,23,3,9,15,21,2,8,14,20,26,6,12,18,24,4,10,16,22
};

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
read:   ld a,(sekdrv)
        ld b,1
        ld de,(seksat)
        ld hl,(dmaadr)
        in a,(4)
	ret
#endif
            uint8_t sec = DE, trk = DE >> 8, dsk = A;
            //printf("r128: cnt %d dsk %d trk %d sec %d -> %d\n",
            //        B, dsk, trk, sec, skewMap[sec]);
            if (dsk > 0)
                sec = skewMap[sec]-1;
            int blk = 26 * trk + sec;
            for (int i = 0; i < B; ++i) {
                void* ptr = z->memory + HL + 128 * i;
                if (dsk > 0)
                    memcpy(ptr, reBlock128(&disks[dsk-1], blk+i, false), 128);
                else
                    spiWear.read128((256*1024/128)*dsk + blk + i, ptr);
            }
            if (dsk > 10) {
                uint8_t* ptr = (uint8_t*) reBlock128(&disks[dsk-1], blk, false);
                for (int i = 0; i < 128; ++i) {
                    printf("%c", ' ' <= ptr[i] && ptr[i] <= '~' ? ptr[i] : '.');
                    if (i % 32 == 31)
                        printf("\n");
                }
            }
            A = 0;
            break;
        }
        case 5: { // write
            uint8_t sec = DE, trk = DE >> 8, dsk = A;
            if (dsk > 0)
                sec = skewMap[sec]-1;
            int blk = 26 * trk + sec;
            for (int i = 0; i < B; ++i) {
                void* ptr = z->memory + HL + 128 * i;
                if (dsk > 0)
                    memcpy(reBlock128(&disks[dsk-1], blk+i, true), ptr, 128);
                else
                    spiWear.write128((256*1024/128)*dsk + blk + i, ptr);
            }
            A = 0;
            break;
        }
        default:
            printf("syscall %d @ %04x ?\n", req, state->pc);
            while (1) {}
    }
}

ZEXTEST zex;

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
#if 0
        printf("base %d spc %d rdir %d rmax %d data %d clim %d\n",
                                    fatFs.base, fatFs.spc, fatFs.rdir,
                                    fatFs.rmax, fatFs.data, fatFs.clim);
#endif
        printf("\n");
        for (int i = 0; i < fatFs.rmax; ++i) {
            int off = (i*32) % 512;
            if (off == 0)
                sdCard.read512(fatFs.rdir + i/16, fatFs.buf);
            int length = *(int32_t*) (fatFs.buf+off+28);
            if (length >= 0 && '!' < fatFs.buf[off] &&
                    fatFs.buf[off] <= '~' && fatFs.buf[off+6] != '~') {
                uint8_t attr = fatFs.buf[off+11];
                printf("   %s\t", attr & 8 ? "vol:" : attr & 16 ? "dir:" : "");
                for (int j = 0; j < 11; ++j) {
                    int c = fatFs.buf[off+j];
                    if (j == 8)
                        printf(".");
                    printf("%c", c);
                }
                printf(" %7d b\n", length);
            }
        }
        printf("\n");

        int len;
        len = disks[0].open("CPMA    CPM");
        if (len > 0)
            printf("  cpma = %db\n", len);
        len = disks[1].open("ZORK1   CPM");
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

    do {
        Z80Emulate(&zex.state, 4000000, &zex);

        static uint32_t last;
        if (last != ticks/3000) {
            last = ticks/3000; // approx every 3s ...
            reBlock128();      // ... flush pending changes to SD card
        }
    } while (!zex.is_done);

    while (1) {}
}
