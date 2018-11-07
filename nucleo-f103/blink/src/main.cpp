#include <jee.h>

UartBufDev< PinA<2>, PinA<3> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
	return 0;
}

PinA<5> led;

int main() {
    console.init();
	enableSysTick(); // no HSE crystal
	led.mode(Pinmode::out);

    while (1) {
        printf("%d\n", ticks);
        led = 1;
        wait_ms(100);
        led = 0;
        wait_ms(400);
    }
}
