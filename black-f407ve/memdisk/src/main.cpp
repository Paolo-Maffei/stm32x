// Launch CP/M, with virtual disks on SPI flash and SD card.

#include <jee.h>
#include <jee/spi-flash.h>
#include <jee/spi-sdcard.h>
#include <string.h>
#include "spi-wear.h"

UartBufDev< PinA<9>, PinA<10> > console;
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
typedef FileMap< decltype(fatFs), 257 > DiskMap; // 8M = 256 fat entries x 32K
DiskMap disks [] = {fatFs,fatFs,fatFs,fatFs,fatFs,fatFs,fatFs,fatFs};
DiskMap* currDisk;

static void* reBlock128 (DiskMap* dmp =0, int blk =0, bool dirty =false) {
    static int currSect;
    static bool currDirty;
    static uint8_t currBuf [512];

    int sect = blk / 4;  // CP/M blocks are 128b, SD card sectors are 512b
    if (dmp != currDisk || sect != currSect) {
        if (currDirty && !currDisk->ioSect(true, currSect, currBuf))
            printf("WR-ERR: disk %d sect %d blk %d\n", dmp-disks, sect, blk);
        currDirty = false;
        if (dmp == 0)
            return 0; // it's a flush request, return value vill be ignored
        currDisk = dmp;
        currSect = sect;
        if (currDisk != 0 && !currDisk->ioSect(false, currSect, currBuf))
            printf("RD-ERR: disk %d sect %d blk %d\n", dmp-disks, sect, blk);
    }
    currDirty |= dirty;
    return currBuf + (blk*128) % 512;
}

void listSdFiles () {
    //printf("\n");
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
    //printf("\n");
}

void diskCopy (int from, int to, int num) {
    DiskMap* df = disks + from;
    DiskMap* dt = disks + to;

    uint32_t start = ticks;

    uint8_t buf [128];
    for (int i = 0; i < num; ++i) {
        void* pf = reBlock128(df, i, false);
        memcpy(buf, pf, 128);
        void* pt = reBlock128(dt, i, true);
        memcpy(pt, buf, 128);
    }
    reBlock128(); // flush

    uint32_t ms = ticks - start;
    printf("%5d KB: (%d -> %d) = %4d ms = %3d KB/sec\n",
            num/8, from, to, ms, (125*num)/ms);
}

int main() {
    console.init();
    console.baud(115200, fullSpeedClock()/2);
    printf("\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");

    spi1.init();
    spiWear.init();

    spi2.init();
    if (sdCard.init()) {
        //printf("[sd card: sdhc %d]\n", sdCard.sdhc);

        fatFs.init();

        printf("base %d spc %d rdir %d rmax %d data %d clim %d\n\n",
                                    fatFs.base, fatFs.spc, fatFs.rdir,
                                    fatFs.rmax, fatFs.data, fatFs.clim);
        listSdFiles();
        printf("\n");

        disks[0].open("CPMA    CPM"); // B:
        disks[1].open("ZORK1   CPM"); // C:
        disks[2].open("T1      DSK"); // D:
        disks[3].open("T2      DSK"); // E:
        disks[4].open("T3      DSK"); // F:
        disks[5].open("T4      DSK"); // G:
        disks[6].open("T5      DSK"); // H:

        diskCopy(5, 6, 8 * 10);
        diskCopy(5, 6, 8 * 100);
    }

    PinB<1> backlight;
    backlight.mode(Pinmode::out);

    while (1) {}
}
