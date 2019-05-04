// Very basic test of the SD-card driver in the JeeH library.

#include <jee.h>
#include <jee/spi-sdcard.h>

UartBufDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
	return 0;
}

#if CIRCLE407
PinB<9> led;
#elif BLACK407
PinA<6> led;
#endif

SpiGpio< PinD<2>, PinC<8>, PinC<12>, PinC<11> > spi;
SdCard< decltype(spi) > sd;
FatFS< decltype(sd) > fat;

int main() {
    console.init();
    console.baud(115200, fullSpeedClock()/2);
    wait_ms(100);
    printf("\n");
    led.mode(Pinmode::out);

    spi.init();
    if (!sd.init()) {
        printf("no SD card detected\n");
        return 1;
    }
    printf("sdhc %d\n", sd.sdhc);

    fat.init();

    printf("base %d spc %d rdir %d rmax %d data %d clim %d\n",
            fat.base, fat.spc, fat.rdir, fat.rmax, fat.data, fat.clim);

    uint32_t t = ticks;
    for (int i = 1; i <= 1000; ++i)
        sd.read512(fat.rdir + 1000 - i, fat.buf);
    printf("read: %d µs/blk\n\n", ticks - t);

    printf("Files:\n");
    //fat.dumpHex(32);
    for (int i = 0; i < 512; i += 32) {
        int length = *(int32_t*) (fat.buf+i+28);
        if (length >= 0 && '!' < fat.buf[i] && fat.buf[i] <= '~') {
            uint8_t attr = fat.buf[i+11];
            printf("   %s\t", attr & 8 ? "vol:" : attr & 16 ? "dir:" : "");
            for (int j = 0; j < 11; ++j) {
                int c = fat.buf[i+j];
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

#if 1  // this WRITES to the SD card, disable if that's not acceptable
    t = ticks;
    for (int i = 1; i <= 1000; ++i)
        sd.write512(120 * 1024 + i%10, fat.buf); // will wipe out above 60 MB!
    printf("write: %d µs/blk\n\n", ticks - t);
#endif

    while (1) {
        printf("%d", ticks % 10);
        led = 0;
        wait_ms(100);
        led = 1;
        wait_ms(9900);
    }
}
