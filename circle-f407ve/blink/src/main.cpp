// Simple on-board LED blink demo, with periodic messages to the serial port.

#include <jee.h>

UartBufDev< PinA<9>, PinA<10> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
	return 0;
}

PinB<9> led;

int main() {
    console.init();
    console.baud(115200, fullSpeedClock()/2);
    printf("\r\n");

#if 0
    printf("flash %d kb\n", MMIO16(0x1FFF7A22));
    printf("h/w id %08x %08x %08x\n",
        MMIO32(0x1FFF7A10), MMIO32(0x1FFF7A14), MMIO32(0x1FFF7A18));
#endif

    led.mode(Pinmode::out);

    while (1) {
        printf("%d\n", ticks);
        led = 0;
        wait_ms(100);
        led = 1;
        wait_ms(400);
    }
}
