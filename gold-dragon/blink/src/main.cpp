#include <jee.h>

UartBufDev< PinC<10>, PinC<11> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

PinE<0> led;

int main() {
    console.init();
	enableSysTick();

    led.mode(Pinmode::out);

    while (1) {
        printf("%d\n", ticks);

        led = 0;
        wait_ms(100);
        led = 1;
        wait_ms(400);
    }
}
