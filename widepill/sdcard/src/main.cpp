#include <jee.h>
#include <jee/spi-sdcard.h>

PinA<1> led;

SpiGpio< PinA<7>, PinA<6>, PinA<5>, PinA<4> > spi;
SdCard< decltype(spi) > sd;
FatFS< decltype(sd) > fat;

UartBufDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

int main () {
    console.init();
    // start slow for SD card init, increase the clock to full speed later on
    //console.baud(115200, fullSpeedClock());
    enableSysTick();
    led.mode(Pinmode::out);

    wait_ms(500);
    printf("\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");

    // disable JTAG in AFIO-MAPR to release PB3, PB4, and PA15
    constexpr uint32_t afio = 0x40010000;
    MMIO32(afio + 0x04) |= 1 << 25;

    spi.init();
    if (sd.init()) {
        printf("sdhc %d\n", sd.sdhc);
        wait_ms(10);
        console.baud(115200, fullSpeedClock());

        fat.init();

        printf("base %d spc %d rdir %d rmax %d data %d\n",
            fat.base, fat.spc, fat.rdir, fat.rmax, fat.data);
#if 0
        for (int i = 0; i < 5; ++i) {
            printf("%3d:", i);
            sd.read512(fat.rdir+i, fat.buf);
            for (int j = 0; j < 20; ++j)
                printf(" %02x", fat.buf[j]);
            printf("\n");
        }
#endif
        sd.read512(fat.rdir, fat.buf);
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

        uint32_t start = ticks;
        for (int i = 0; i < 100; ++i)
            sd.read512(i, fat.buf);
        printf("read512 %d us\n", (ticks - start) * 10);

        start = ticks;
        for (int i = 0; i < 100; ++i)
            sd.write512(1, fat.buf);
        printf("write512 %d us\n", (ticks - start) * 10);
    }

    while (true) {}
}
