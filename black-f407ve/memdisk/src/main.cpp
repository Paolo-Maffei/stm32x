// Launch CP/M, with virtual disks on SPI flash and SD card.

#include <jee.h>
#include <jee/spi-flash.h>
#include <jee/spi-sdcard.h>
#include <string.h>
#include "spi-wear.h"
#include "flashwear.h"
#include "drive.h"

UartBufDev< PinA<9>, PinA<10> > console;
PinE<4> key0;
PinE<3> key1;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

Drive drives [8];

void diskCopy (int from, int to, int kb) {
    uint32_t start = ticks;

    uint8_t buf [8][128];
    for (int i = 0; i < kb; ++i) {
        for (int j = 0; j < 8; ++j)
            drives[from].read(8*i + j, buf[j]);
        for (int j = 0; j < 8; ++j)
            drives[to].write(8*i + j, buf[j]);
    }
    reBlock128(); // flush

    uint32_t ms = ticks - start;
    printf("%5d KB: (%d -> %d) %4d ms = %4d KB/sec\n",
            kb, from, to, ms, (1000*kb)/ms);
}

int main() {
    console.init();
    console.baud(115200, fullSpeedClock()/2);
    printf("\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");

    spi2.init();
    if (sdCard.init()) {
        //printf("[sd card: sdhc %d]\n", sdCard.sdhc);

        uint32_t t1 = ticks;
        fatFs.init();
        printf("init %d ms\n", ticks - t1);
#if 0
        printf("base %d spc %d rdir %d rmax %d data %d clim %d\n",
                                    fatFs.base, fatFs.spc, fatFs.rdir,
                                    fatFs.rmax, fatFs.data, fatFs.clim);
#endif
        uint32_t t2 = ticks;
        listSdFiles();
        printf("list %d ms\n", ticks - t2);

        uint32_t t3 = ticks;
        drives[0].assign("        01 "); // A:
        drives[1].assign("CPMA    CPM"); // B:
        drives[2].assign("        1  "); // C:
        drives[3].assign("        02 "); // D:
        drives[4].assign("T2      DSK"); // E:
        drives[5].assign("T3      DSK"); // F:
        drives[6].assign("T4      DSK"); // G:
        drives[7].assign("T5      DSK"); // H:
        printf("open %d ms\n", ticks - t3);

        diskCopy(6, 7, 1);
        diskCopy(6, 7, 10);
        diskCopy(6, 7, 100);

        diskCopy(0, 3, 1);
        diskCopy(0, 3, 10);
        diskCopy(0, 3, 100);

        diskCopy(2, 2, 1);
        diskCopy(2, 2, 10);
        diskCopy(2, 2, 100);
    }

    PinB<1> backlight;
    backlight.mode(Pinmode::out);

    while (1) {}
}
