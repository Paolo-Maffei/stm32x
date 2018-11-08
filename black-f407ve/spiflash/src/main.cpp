#include <jee.h>
#include <jee/spi-flash.h>

UartBufDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
	return 0;
}

//PinC<2> led;
PinA<6> led;

SpiGpio< PinB<5>, PinB<4>, PinB<3>, PinB<0> > spi;
SpiFlash< decltype(spi) > smem;

int main() {
    console.init();
	console.baud(115200, fullSpeedClock()/2);
    wait_ms(1000);
    printf("\n-------------------------------------------------------------\n");

	led.mode(Pinmode::out);

    spi.init();
    smem.init();

    printf("smem %x, %d kB\n", smem.devId(), smem.size());

    while (1) {
        printf("%d\n", ticks);
        led = 0;
        wait_ms(100);
        led = 1;
        wait_ms(400);
    }
}