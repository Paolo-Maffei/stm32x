// Testing a wear-leveled wrapper around the SPI-flash driver.

#include <jee.h>
#include <jee/spi-flash.h>
#include <string.h>
#include <spi-wear.h>

UartBufDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
	return 0;
}

// chip select needs a minute slowdown to work at 168 MHz
SpiGpio< PinB<5>, PinB<4>, PinB<3>, SlowPin< PinB<0>, 2 > > spi;
SpiFlash< decltype(spi) > spif;
SpiWear< decltype(spif), PinA<6> > wear;

int main() {
    console.init();
    console.baud(115200, fullSpeedClock()/2);
    printf("\n-------------------------------------------------------------\n");

    spi.init();
    wear.init();
    //spif.wipe();

    printf("spif %x, %d kB\n", spif.devId(), spif.size());

    static uint8_t buf [128];

    // only run a few times, don't wear out flash while testing
    for (int n = 0; n < 30; ++n) {
        int offset = n < 15 ? 0 : n < 25 ? 2048 : 4096;

        wear.read128(offset + n % 5, buf);
        int sum = 0;
        for (uint32_t i = 0; i < sizeof buf; ++i)
            sum += buf[i];

        ++buf[0];
        wear.write128(offset + n % 5, buf);

        printf("block %d ticks %d sum %d buf %02x%02x...\n",
                offset + n % 5, ticks, sum, buf[0], buf[1]);

        wait_ms(100);
    }

    while (true) {}
}
