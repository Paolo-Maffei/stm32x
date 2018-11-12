// Very basic test of the SPI-flash driver in the JeeH library.

#include <jee.h>
#include <jee/spi-flash.h>

UartBufDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
	return 0;
}

PinA<6> led;

// chip select needs a minute slowdown to work at 168 MHz
SpiGpio< PinB<5>, PinB<4>, PinB<3>, SlowPin< PinB<0>, 0 > > spi;
SpiFlash< decltype(spi) > spif;

int main() {
    console.init();
    console.baud(115200, fullSpeedClock()/2);
    printf("\n-------------------------------------------------------------\n");
    led.mode(Pinmode::out);

    spi.init();
    spif.init();

    printf("spif %x, %d kB\n", spif.devId(), spif.size());

    static uint8_t buf [4000];

    while (1) {
        spif.read(0, buf, sizeof buf);
        int sum = 0;
        for (int i = 0; i < sizeof buf; ++i)
            sum += buf[i];
        printf("%d sum %d\n", ticks, sum);

        spif.write(0, buf, sizeof buf);

        led = 0;
        wait_ms(100);
        led = 1;
        wait_ms(400);
    }
}
