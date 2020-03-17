#include <jee.h>

UartBufDev< PinA<2>, PinA<3> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

PinA<1> led;

int main() {
    console.init();
    //console.baud(115200, fullSpeedClock());
    enableSysTick();
    led.mode(Pinmode::out);

    while (true) {
        printf("%d\n", ticks);
        led = 1;
        wait_ms(100);
        led = 0;
        wait_ms(400);
    }
}
